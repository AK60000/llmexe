#include "model_loader.h"
#include "llama.h"
#include <fstream>
#include <iostream>

namespace llmexe {

ModelLoader::ModelLoader() : model_(nullptr) {}

ModelLoader::~ModelLoader() {
    if (model_) {
        llama_free_model(model_);
        model_ = nullptr;
    }
    
    // Clean up temp file
    if (!temp_model_path_.empty()) {
        std::remove(temp_model_path_.c_str());
    }
}

bool ModelLoader::loadFromMemory(const void* data, size_t size) {
    // Initialize llama backend
    llama_backend_init(false);
    
    // Write model data to temp file
    temp_model_path_ = "model_temp.gguf";
    std::ofstream out(temp_model_path_, std::ios::binary);
    if (!out) {
        last_error_ = "Failed to create temp file: " + temp_model_path_;
        return false;
    }
    out.write(static_cast<const char*>(data), size);
    out.close();
    
    if (!out.good()) {
        last_error_ = "Failed to write model data to temp file";
        return false;
    }
    
    // Set up model parameters
    llama_context_params model_params = llama_context_default_params();
    
    // Load model from file
    model_ = llama_load_model_from_file(temp_model_path_.c_str(), model_params);
    
    if (!model_) {
        last_error_ = "Failed to load model from file: " + temp_model_path_;
        return false;
    }
    
    return true;
}

} // namespace llmexe
