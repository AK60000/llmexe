#include "inference.h"
#include "llama.h"
#include <vector>
#include <iostream>

namespace llmexe {

InferenceEngine::InferenceEngine(llama_model* model)
    : model_(model), ctx_(nullptr) {}

InferenceEngine::~InferenceEngine() {
    if (ctx_) {
        llama_free(ctx_);
    }
}

bool InferenceEngine::initialize(const InferenceParams& params) {
    params_ = params;
    
    // Create context parameters
    llama_context_params ctx_params = llama_context_default_params();
    ctx_params.n_ctx = 2048; // Context window
    
    // Create context
    ctx_ = llama_new_context_with_model(model_, ctx_params);
    
    if (!ctx_) {
        last_error_ = "Failed to create llama context";
        return false;
    }
    
    return true;
}

std::string InferenceEngine::buildQwenPrompt(const std::string& user_input) {
    return "<|im_start|>user\n" + user_input + "<|im_end|>\n<|im_start|>assistant\n";
}

bool InferenceEngine::generate(const std::string& prompt, const StreamCallback& callback) {
    if (!ctx_) {
        last_error_ = "Inference engine not initialized";
        return false;
    }
    
    std::string full_prompt = buildQwenPrompt(prompt);
    
    // Tokenize the prompt
    std::vector<llama_token> tokens(2048);
    int n_tokens = llama_tokenize(
        ctx_,
        full_prompt.c_str(),
        tokens.data(),
        tokens.size(),
        true  // add_bos
    );
    
    if (n_tokens <= 0) {
        last_error_ = "Failed to tokenize prompt";
        return false;
    }
    tokens.resize(n_tokens);
    
    if (n_tokens <= 0) {
        last_error_ = "Failed to tokenize prompt";
        return false;
    }
    
    // Evaluate the prompt
    for (int i = 0; i < n_tokens; i++) {
        if (llama_eval(ctx_, &tokens[i], 1, i, params_.num_threads) != 0) {
            last_error_ = "Failed to evaluate prompt token";
            return false;
        }
    }
    
    // Generate response
    llama_token new_token;
    int n_cur = 0;
    
    while (n_cur < params_.max_tokens) {
        // Get logits
        float* logits = llama_get_logits(ctx_);
        
        // Apply sampling (temperature, top_p, etc.)
        // For now, just sample greedily
        new_token = llama_sample_token_greedy(ctx_, nullptr);
        
        // Check for end of generation
        if (new_token == llama_token_eos()) {
            break;
        }
        
        // Convert token to text
        const char* token_text = llama_token_to_str(ctx_, new_token);
        std::string chunk(token_text);
        
        // Stream the token
        if (callback) {
            callback(chunk);
        }
        
        // Evaluate the new token
        if (llama_eval(ctx_, &new_token, 1, n_cur + n_tokens, params_.num_threads) != 0) {
            last_error_ = "Failed to evaluate generated token";
            return false;
        }
        
        n_cur++;
    }
    
    return true;
}

} // namespace llmexe
