#ifndef LLMEXE_INFERENCE_H
#define LLMEXE_INFERENCE_H

#include <string>
#include <functional>
#include <vector>

struct llama_model;
struct llama_context;
struct llama_sampler;
struct llama_vocab;

namespace llmexe {

struct InferenceParams {
    float temperature = 0.7f;
    float top_p = 0.9f;
    int max_tokens = 512;
    int num_threads = 0; // 0 = auto-detect
};

using StreamCallback = std::function<void(const std::string&)>;

class InferenceEngine {
public:
    InferenceEngine(llama_model* model);
    ~InferenceEngine();

    bool initialize(const InferenceParams& params);
    bool generate(const std::string& prompt, const StreamCallback& callback);
    
    const std::string& getLastError() const { return last_error_; }

private:
    std::string buildQwenPrompt(const std::string& user_input);
    
    llama_model* model_;
    llama_context* ctx_;
    llama_sampler* sampler_;
    llama_vocab* vocab_;
    InferenceParams params_;
    std::string last_error_;
};

} // namespace llmexe

#endif // LLMEXE_INFERENCE_H
