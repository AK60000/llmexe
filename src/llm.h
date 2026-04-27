#pragma once
#include <string>
#include <functional>

struct llama_model;
struct llama_context;
struct llama_sampler;
struct llama_vocab;

namespace llmexe {

struct InferenceParams {
    int max_tokens = 512;
    int num_threads = 4;
    float temperature = 0.7f;
    float top_p = 0.9f;
};

using StreamCallback = std::function<void(const std::string&)>;

class LLM {
public:
    LLM();
    ~LLM();

    // Load the model from file and initialize context/sampler
    bool load(const std::string& model_path, const InferenceParams& params);

    // Generate response from prompt, streaming tokens via callback
    bool generate(const std::string& prompt, const StreamCallback& callback);

    const std::string& getLastError() const { return last_error_; }

private:
    llama_model* model_ = nullptr;
    llama_context* ctx_ = nullptr;
    llama_sampler* sampler_ = nullptr;
    const llama_vocab* vocab_ = nullptr;
    
    InferenceParams params_;
    std::string last_error_;
};

} // namespace llmexe