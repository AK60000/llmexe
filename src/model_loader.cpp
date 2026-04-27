#include "model_loader.h"
#include "llama.h"
#include <fstream>
#include <iostream>
#include <windows.h>

namespace llmexe {

ModelLoader::ModelLoader() : model_(nullptr) {}

ModelLoader::~ModelLoader() {
    if (model_) {
        llama_model_free(model_);
        model_ = nullptr;
    }
}

bool ModelLoader::loadFromFile(const std::string& path) {
    if (model_) {
        llama_model_free(model_);
        model_ = nullptr;
    }
    
    llama_model_params model_params = llama_model_default_params();
    model_params.n_gpu_layers = 0; // Use CPU only
    
    model_ = llama_model_load_from_file(path.c_str(), model_params);
    
    if (!model_) {
        last_error_ = "Failed to load model from: " + path;
        return false;
    }
    
    return true;
}

// Load model from the end of the current executable (APE format)
bool ModelLoader::loadFromAttachedData() {
    // Get current executable path
    char exe_path[MAX_PATH];
    GetModuleFileNameA(NULL, exe_path, MAX_PATH);
    
    std::ifstream file(exe_path, std::ios::binary | std::ios::ate);
    if (!file) {
        last_error_ = "Failed to open executable: " + std::string(exe_path);
        return false;
    }
    
    // Get file size
    size_t file_size = file.tellg();
    file.seekg(0, std::ios::beg);
    
    // TODO: Parse APE format to find the ZIP data offset
    // For now, assume model data is appended at the end
    // We need to extract it or use memory mapping
    
    last_error_ = "loadFromAttachedData not fully implemented yet";
    return false;
}

bool ModelLoader::loadFromMemory(const void* data, size_t size) {
    // Initialize llama backend
    llama_backend_init();
    
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
    
    // Load from temp file
    return loadFromFile(temp_model_path_);
}

llama_model* ModelLoader::getModel() const {
    return model_;
}

const std::string& ModelLoader::getLastError() const {
    return last_error_;
}

} // namespace llmexe
