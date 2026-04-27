#include "inference.h"
#include "llama.h"
#include <vector>
#include <iostream>
#include <cstring>
#include <fstream>

namespace llmexe {

InferenceEngine::InferenceEngine(llama_model* model)
    : model_(model), ctx_(nullptr), sampler_(nullptr), vocab_(nullptr) {
    vocab_ = const_cast<llama_vocab*>(llama_model_get_vocab(model_));
}

InferenceEngine::~InferenceEngine() {
    if (sampler_) {
        llama_sampler_free(sampler_);
    }
    if (ctx_) {
        llama_free(ctx_);
        ctx_ = nullptr;
    }
}

bool InferenceEngine::initialize(const InferenceParams& params) {
    params_ = params;
    
    // Create context parameters
    llama_context_params ctx_params = llama_context_default_params();
    ctx_params.n_ctx = 2048; // Context window
    ctx_params.n_threads = params_.num_threads;
    ctx_params.n_threads_batch = params_.num_threads;
    
    // Create context
    ctx_ = llama_init_from_model(model_, ctx_params);
    
    if (!ctx_) {
        last_error_ = "Failed to create llama context";
        return false;
    }
    
    // Create sampler
    llama_sampler_chain_params smpl_params = llama_sampler_chain_default_params();
    sampler_ = llama_sampler_chain_init(smpl_params);
    
    if (params_.temperature > 0) {
        llama_sampler_chain_add(sampler_, llama_sampler_init_temp(params_.temperature));
        llama_sampler_chain_add(sampler_, llama_sampler_init_top_p(params_.top_p, 1));
        llama_sampler_chain_add(sampler_, llama_sampler_init_dist(LLAMA_DEFAULT_SEED));
    } else {
        llama_sampler_chain_add(sampler_, llama_sampler_init_greedy());
    }
    
    return true;
}

std::string InferenceEngine::buildQwenPrompt(const std::string& user_input) {
    return "<|im_start|>user\n" + user_input + "<|im_end|>\n<|im_start|>assistant\n";
}

bool InferenceEngine::generate(const std::string& prompt, const StreamCallback& callback) {
    if (!ctx_ || !sampler_) {
        last_error_ = "Inference engine not initialized";
        return false;
    }
    
    std::string full_prompt = buildQwenPrompt(prompt);
    
    // Tokenize the prompt
    std::vector<llama_token> tokens(2048);
    int n_tokens = llama_tokenize(
        vocab_,
        full_prompt.c_str(),
        full_prompt.size(),
        tokens.data(),
        tokens.size(),
        true,  // add_special
        true   // parse_special
    );
    
    if (n_tokens <= 0) {
        last_error_ = "Failed to tokenize prompt";
        return false;
    }
    
    // Prepare batch for prompt
    llama_batch batch = llama_batch_init(n_tokens, 0, 1);
    if (batch.token == nullptr) {
        last_error_ = "Failed to init batch";
        return false;
    }
    
    for (int i = 0; i < n_tokens; i++) {
        batch.token[i] = tokens[i];
        batch.pos[i] = i;
        batch.n_seq_id[i] = 1;
        batch.seq_id[i][0] = 0;
        batch.logits[i] = (i == n_tokens - 1) ? 1 : 0;
    }
    batch.n_tokens = n_tokens;
    
    // Decode the prompt
    if (llama_decode(ctx_, batch) != 0) {
        last_error_ = "Failed to decode prompt";
        llama_batch_free(batch);
        return false;
    }
    llama_batch_free(batch);
    
    // Generate response
    int n_cur = 0;
    std::vector<char> piece_buf(256);
    int32_t n_eval = n_tokens; // number of tokens evaluated (prompt + generated)
    
    while (n_cur < params_.max_tokens) {
        // Sample token
        llama_token new_token = llama_sampler_sample(sampler_, ctx_, -1);
        
        // Check for end of generation
        if (llama_vocab_is_eog(vocab_, new_token)) {
            break;
        }
        
        // Convert token to text
        int n_chars = llama_token_to_piece(
            vocab_,
            new_token,
            piece_buf.data(),
            piece_buf.size(),
            0,  // lstrip
            true   // special
        );
        
        if (n_chars > 0) {
            std::string chunk(piece_buf.data(), n_chars);
            if (callback) {
                callback(chunk);
            }
        } else if (n_chars < 0) {
            // buffer too small, resize and retry
            piece_buf.resize(-n_chars);
            n_chars = llama_token_to_piece(
                vocab_,
                new_token,
                piece_buf.data(),
                piece_buf.size(),
                true,
                true
            );
            if (n_chars > 0) {
                std::string chunk(piece_buf.data(), n_chars);
                if (callback) {
                    callback(chunk);
                }
            }
        }
        
        // Prepare batch for the new token
        batch = llama_batch_init(1, 0, 1);
        if (batch.token == nullptr) {
            last_error_ = "Failed to init batch for new token";
            return false;
        }
        batch.token[0] = new_token;
        batch.pos[0] = n_eval;
        batch.n_seq_id[0] = 1;
        batch.seq_id[0][0] = 0;
        batch.logits[0] = 1;
        batch.n_tokens = 1;
        
        // Decode the new token
        if (llama_decode(ctx_, batch) != 0) {
            last_error_ = "Failed to decode token";
            llama_batch_free(batch);
            return false;
        }
        llama_batch_free(batch);
        
        n_cur++;
        n_eval++;
    }
    
    return true;
}

} // namespace llmexe
