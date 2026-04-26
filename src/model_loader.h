#ifndef LLMEXE_MODEL_LOADER_H
#define LLMEXE_MODEL_LOADER_H

#include <string>

struct llama_model;
struct llama_context;

namespace llmexe {

class ModelLoader {
public:
    ModelLoader();
    ~ModelLoader();

    // Load model from embedded data
    bool loadFromMemory(const void* data, size_t size);
    
    // Get the loaded model
    llama_model* getModel() const { return model_; }
    
    // Get last error message
    const std::string& getLastError() const { return last_error_; }

private:
    llama_model* model_;
    std::string last_error_;
};

} // namespace llmexe

#endif // LLMEXE_MODEL_LOADER_H