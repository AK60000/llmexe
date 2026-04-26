#include "model_loader.h"
#include "llama.h"

namespace llmexe {

ModelLoader::ModelLoader() : model_(nullptr) {}

ModelLoader::~ModelLoader() {
    if (model_) {
        llama_model_free(model_);
    }
}

bool ModelLoader::loadFromMemory(const void* data, size_t size) {
    // Initialize llama backend if not already done
    llama_backend_init();
    
    // Set up model parameters
    llama_model_params model_params = llama_model_default_params();
    
    // Load model from buffer
    model_ = llama_model_load_from_buffer(data, size, model_params);
    
    if (!model_) {
        last_error_ = "Failed to load model from memory buffer";
        return false;
    }
    
    return true;
}

} // namespace llmexe