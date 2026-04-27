#include "llm.h"
#include "llama.h"
#include <vector>

namespace llmexe {

LLM::LLM() = default;

LLM::~LLM() {
    if (sampler_) llama_sampler_free(sampler_);
    if (ctx_) llama_free(ctx_);
    if (model_) llama_model_free(model_);
}

bool LLM::load(const std::string& model_path, const InferenceParams& params) {
    params_ = params;

    // Load model
    llama_model_params model_params = llama_model_default_params();
    model_params.n_gpu_layers = 0; // CPU only
    model_ = llama_model_load_from_file(model_path.c_str(), model_params);
    if (!model_) {
        last_error_ = "Failed to load model from: " + model_path;
        return false;
    }

    vocab_ = llama_model_get_vocab(model_);

    // Init context
    llama_context_params ctx_params = llama_context_default_params();
    ctx_params.n_ctx = 2048;
    ctx_params.n_threads = params_.num_threads;
    ctx_params.n_threads_batch = params_.num_threads;
    ctx_ = llama_init_from_model(model_, ctx_params);
    if (!ctx_) {
        last_error_ = "Failed to create context";
        return false;
    }

    // Init sampler
    llama_sampler_chain_params smpl_params = llama_sampler_chain_default_params();
    sampler_ = llama_sampler_chain_init(smpl_params);
    if (params_.temperature > 0.0f) {
        llama_sampler_chain_add(sampler_, llama_sampler_init_temp(params_.temperature));
        llama_sampler_chain_add(sampler_, llama_sampler_init_top_p(params_.top_p, 1));
        llama_sampler_chain_add(sampler_, llama_sampler_init_dist(LLAMA_DEFAULT_SEED));
    } else {
        llama_sampler_chain_add(sampler_, llama_sampler_init_greedy());
    }

    return true;
}

bool LLM::generate(const std::string& prompt, const StreamCallback& callback) {
    if (!ctx_ || !sampler_) {
        last_error_ = "LLM not initialized";
        return false;
    }

    // Simple ChatML formatting (works for Qwen, etc.)
    std::string full_prompt = "<|im_start|>user\n" + prompt + "<|im_end|>\n<|im_start|>assistant\n";

    // Tokenize
    std::vector<llama_token> tokens(2048);
    int n_tokens = llama_tokenize(vocab_, full_prompt.c_str(), full_prompt.size(), tokens.data(), tokens.size(), true, true);
    if (n_tokens <= 0) {
        last_error_ = "Failed to tokenize prompt";
        return false;
    }

    // Decode prompt
    llama_batch batch = llama_batch_init(n_tokens, 0, 1);
    for (int i = 0; i < n_tokens; i++) {
        batch.token[i] = tokens[i];
        batch.pos[i] = i;
        batch.n_seq_id[i] = 1;
        batch.seq_id[i][0] = 0;
        batch.logits[i] = (i == n_tokens - 1) ? 1 : 0;
    }
    batch.n_tokens = n_tokens;

    if (llama_decode(ctx_, batch) != 0) {
        last_error_ = "Failed to decode prompt";
        llama_batch_free(batch);
        return false;
    }
    llama_batch_free(batch);

    // Generation loop
    int n_eval = n_tokens;
    std::vector<char> piece_buf(256);

    for (int i = 0; i < params_.max_tokens; i++) {
        llama_token new_token = llama_sampler_sample(sampler_, ctx_, -1);
        if (llama_vocab_is_eog(vocab_, new_token)) break;

        // Convert token to piece
        int n_chars = llama_token_to_piece(vocab_, new_token, piece_buf.data(), piece_buf.size(), 0, true);
        if (n_chars < 0) {
            piece_buf.resize(-n_chars);
            n_chars = llama_token_to_piece(vocab_, new_token, piece_buf.data(), piece_buf.size(), 0, true);
        }
        if (n_chars > 0 && callback) {
            callback(std::string(piece_buf.data(), n_chars));
        }

        // Decode next token
        batch = llama_batch_init(1, 0, 1);
        batch.token[0] = new_token;
        batch.pos[0] = n_eval++;
        batch.n_seq_id[0] = 1;
        batch.seq_id[0][0] = 0;
        batch.logits[0] = 1;
        batch.n_tokens = 1;

        if (llama_decode(ctx_, batch) != 0) {
            last_error_ = "Failed to decode token";
            llama_batch_free(batch);
            return false;
        }
        llama_batch_free(batch);
    }

    return true;
}

} // namespace llmexe