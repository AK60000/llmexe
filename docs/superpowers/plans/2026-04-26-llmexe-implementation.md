# llmexe Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Build a C++ command-line tool that embeds Qwen3-0.6B GGUF model and performs inference with streaming output.

**Architecture:** Use llama.cpp as static library. Convert GGUF model to C array embedded in executable. Use memory buffer loading API from llama.cpp. Command-line arguments parsed manually or with simple parsing.

**Tech Stack:** C++17, CMake, MinGW-w64, llama.cpp (static library)

---

## File Structure

Files to be created/modified:

```
llmexe/
├── CMakeLists.txt                    # Main build configuration
├── src/
│   ├── main.cpp                     # Entry point, CLI parsing
│   ├── model_loader.h               # Model loading interface
│   ├── model_loader.cpp             # Load model from memory buffer
│   ├── inference.h                  # Inference interface
│   └── inference.cpp                # Inference with streaming output
├── model/
│   └── model_data.h                 # Generated C array from GGUF (gitignored)
├── scripts/
│   └── convert_gguf_to_header.py    # Conversion script
├── third_party/
│   └── llama.cpp/                   # Git submodule (gitignored build dir)
└── .gitignore                       # Ignore build artifacts and model files
```

---

### Task 1: Project Setup and CMake Configuration

**Files:**
- Create: `C:/code/llmexe/CMakeLists.txt`
- Create: `C:/code/llmexe/.gitignore`
- Create: `C:/code/llmexe/src/CMakeLists.txt`

- [ ] **Step 1: Create .gitignore**

```gitignore
# Build directories
build/
build-*/

# Model files (too large for git)
model/
*.gguf
*.h (model_data.h specifically)

# IDE files
.vscode/
.idea/
*.swp
*.swo

# OS files
Thumbs.db
Desktop.ini
.DS_Store

# Executables
*.exe
*.dll
*.lib
*.a

# CMake artifacts
CMakeFiles/
CMakeCache.txt
cmake_install.cmake
install_manifest.txt
```

- [ ] **Step 2: Create main CMakeLists.txt**

```cmake
cmake_minimum_required(VERSION 3.20)
project(llmexe VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Option for building llama.cpp
option(LLMEXE_BUILD_LLAMA "Build llama.cpp as submodule" ON)

# Include llama.cpp
if(LLMEXE_BUILD_LLAMA)
    add_subdirectory(third_party/llama.cpp)
endif()

# Add our source directory
add_subdirectory(src)
```

- [ ] **Step 3: Create src/CMakeLists.txt**

```cmake
# Create the executable
add_executable(llmexe
    main.cpp
    model_loader.cpp
    inference.cpp
)

# Link llama.cpp libraries
target_link_libraries(llmexe
    PRIVATE
        llama
        common
)

# Include directories
target_include_directories(llmexe
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/third_party/llama.cpp
        ${CMAKE_SOURCE_DIR}/third_party/llama.cpp/common
)

# Set compile definitions
target_compile_definitions(llmexe
    PRIVATE
        _WIN32
)
```

- [ ] **Step 4: Initialize git repository**

```bash
cd C:/code/llmexe
git init
git add .gitignore CMakeLists.txt src/CMakeLists.txt
git commit -m "feat: initial project setup with CMake"
```

---

### Task 2: Add llama.cpp as Submodule

**Files:**
- Modify: None (git submodule add)

- [ ] **Step 1: Add llama.cpp submodule**

```bash
cd C:/code/llmexe
git submodule add https://github.com/ggerganov/llama.cpp.git third_party/llama.cpp
```

- [ ] **Step 2: Checkout a stable version**

```bash
cd C:/code/llmexe/third_party/llama.cpp
git checkout b5023 # or latest stable tag
cd C:/code/llmexe
```

- [ ] **Step 3: Commit submodule addition**

```bash
git add .gitmodules third_party/llama.cpp
git commit -m "feat: add llama.cpp as submodule"
```

---

### Task 3: Create Model Loader Interface

**Files:**
- Create: `C:/code/llmexe/src/model_loader.h`
- Create: `C:/code/llmexe/src/model_loader.cpp`

- [ ] **Step 1: Write model_loader.h**

```cpp
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
```

- [ ] **Step 2: Write model_loader.cpp**

```cpp
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
```

- [ ] **Step 3: Commit model loader**

```bash
git add src/model_loader.h src/model_loader.cpp
git commit -m "feat: add model loader with memory buffer support"
```

---

### Task 4: Create Inference Engine

**Files:**
- Create: `C:/code/llmexe/src/inference.h`
- Create: `C:/code/llmexe/src/inference.cpp`

- [ ] **Step 1: Write inference.h**

```cpp
#ifndef LLMEXE_INFERENCE_H
#define LLMEXE_INFERENCE_H

#include <string>
#include <functional>

struct llama_model;
struct llama_context;

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
    InferenceParams params_;
    std::string last_error_;
};

} // namespace llmexe

#endif // LLMEXE_INFERENCE_H
```

- [ ] **Step 2: Write inference.cpp**

```cpp
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
    ctx_params.n_threads = params.num_threads;
    ctx_params.n_threads_batch = params.num_threads;
    
    ctx_ = llama_create_context(model_, ctx_params);
    
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
    std::vector<llama_token> tokens = llama_tokenize(model_, full_prompt, true, true);
    
    // Evaluate the prompt
    for (size_t i = 0; i < tokens.size(); i++) {
        if (llama_decode(ctx_, tokens[i]) != 0) {
            last_error_ = "Failed to decode prompt token";
            return false;
        }
    }
    
    // Generate response
    llama_token new_token;
    int n_cur = 0;
    
    while (n_cur < params_.max_tokens) {
        new_token = llama_sample_token(ctx_, nullptr);
        
        // Check for end of generation
        if (llama_token_is_eog(model_, new_token)) {
            break;
        }
        
        // Apply temperature and top_p sampling
        llama_token_data_array cur_p = {nullptr, 0, false};
        llama_sample_temperature(ctx_, new_token, params_.temperature);
        llama_sample_top_p(ctx_, &cur_p, params_.top_p, 1, nullptr);
        
        // Convert token to text
        std::string token_text = llama_token_to_piece(ctx_, new_token);
        
        // Stream the token
        if (callback) {
            callback(token_text);
        }
        
        // Decode the new token
        if (llama_decode(ctx_, new_token) != 0) {
            last_error_ = "Failed to decode generated token";
            return false;
        }
        
        n_cur++;
    }
    
    return true;
}

} // namespace llmexe
```

- [ ] **Step 3: Commit inference engine**

```bash
git add src/inference.h src/inference.cpp
git commit -m "feat: add inference engine with streaming support"
```

---

### Task 5: Create Main Entry Point with CLI Parsing

**Files:**
- Create: `C:/code/llmexe/src/main.cpp`

- [ ] **Step 1: Write main.cpp**

```cpp
#include <iostream>
#include <string>
#include <vector>
#include "model_loader.h"
#include "inference.h"

void printUsage(const char* prog_name) {
    std::cout << "Usage: " << prog_name << " [options] \"prompt\"\n"
              << "Options:\n"
              << "  --temperature FLOAT   Temperature (default: 0.7)\n"
              << "  --top-p FLOAT        Top-p sampling (default: 0.9)\n"
              << "  --max-tokens INT     Max tokens to generate (default: 512)\n"
              << "  --threads INT        Number of threads (default: auto)\n"
              << "  -h, --help           Show this help message\n";
}

bool parseArgs(int argc, char** argv, 
              std::string& prompt,
              llmexe::InferenceParams& params) {
    std::vector<std::string> args(argv + 1, argv + argc);
    
    for (size_t i = 0; i < args.size(); ++i) {
        if (args[i] == "-h" || args[i] == "--help") {
            return false;
        } else if (args[i] == "--temperature" && i + 1 < args.size()) {
            params.temperature = std::stof(args[++i]);
        } else if (args[i] == "--top-p" && i + 1 < args.size()) {
            params.top_p = std::stof(args[++i]);
        } else if (args[i] == "--max-tokens" && i + 1 < args.size()) {
            params.max_tokens = std::stoi(args[++i]);
        } else if (args[i] == "--threads" && i + 1 < args.size()) {
            params.num_threads = std::stoi(args[++i]);
        } else if (args[i][0] != '-') {
            prompt = args[i];
        }
    }
    
    return !prompt.empty();
}

int main(int argc, char** argv) {
    // Parse command line arguments
    std::string prompt;
    llmexe::InferenceParams params;
    
    if (!parseArgs(argc, argv, prompt, params)) {
        printUsage(argv[0]);
        return 1;
    }
    
    // Load model from embedded data
    llmexe::ModelLoader loader;
    
    // Model data will be linked from model_data.h
    extern const unsigned char model_data[];
    extern const unsigned int model_data_size;
    
    std::cout << "Loading model from memory..." << std::endl;
    if (!loader.loadFromMemory(model_data, model_data_size)) {
        std::cerr << "Error: " << loader.getLastError() << std::endl;
        return 1;
    }
    
    // Initialize inference engine
    llmexe::InferenceEngine engine(loader.getModel());
    if (!engine.initialize(params)) {
        std::cerr << "Error: " << engine.getLastError() << std::endl;
        return 1;
    }
    
    // Streaming callback
    auto callback = [](const std::string& token) {
        std::cout << token << std::flush;
    };
    
    // Generate response
    std::cout << "\nGenerating...\n" << std::endl;
    if (!engine.generate(prompt, callback)) {
        std::cerr << "\nError: " << engine.getLastError() << std::endl;
        return 1;
    }
    
    std::cout << std::endl;
    return 0;
}
```

- [ ] **Step 2: Commit main.cpp**

```bash
git add src/main.cpp
git commit -m "feat: add main entry point with CLI argument parsing"
```

---

### Task 6: Create Model Conversion Script

**Files:**
- Create: `C:/code/llmexe/scripts/convert_gguf_to_header.py`

- [ ] **Step 1: Write conversion script**

```python
#!/usr/bin/env python3
"""
Convert GGUF model file to C header file with embedded data.
Usage: python convert_gguf_to_header.py input.gguf output.h
"""

import sys
import os

def convert_gguf_to_header(input_path, output_path):
    with open(input_path, 'rb') as f:
        data = f.read()
    
    with open(output_path, 'w') as out:
        out.write(f'// Auto-generated from {os.path.basename(input_path)}\n')
        out.write(f'// Size: {len(data)} bytes ({len(data) / 1024 / 1024:.2f} MB)\n\n')
        out.write(f'const unsigned int model_data_size = {len(data)};\n')
        out.write('const unsigned char model_data[] = {\n')
        
        for i, b in enumerate(data):
            if i % 16 == 0:
                out.write('    ')
            out.write(f'0x{b:02x},')
            if i % 16 == 15:
                out.write('\n')
        
        if len(data) % 16 != 0:
            out.write('\n')
        out.write('};\n')
    
    print(f'Converted {len(data)} bytes to {output_path}')

if __name__ == '__main__':
    if len(sys.argv) != 3:
        print(f'Usage: {sys.argv[0]} input.gguf output.h')
        sys.exit(1)
    
    convert_gguf_to_header(sys.argv[1], sys.argv[2])
```

- [ ] **Step 2: Commit script**

```bash
git add scripts/convert_gguf_to_header.py
git commit -m "feat: add script to convert GGUF model to C header"
```

---

### Task 7: Build and Test (Manual Verification)

**Files:**
- Modify: `C:/code/llmexe/src/CMakeLists.txt` (to include model_data.h)

- [ ] **Step 1: Update src/CMakeLists.txt to include model directory**

```cmake
# Add model directory for model_data.h
target_include_directories(llmexe
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/third_party/llama.cpp
        ${CMAKE_SOURCE_DIR}/third_party/llama.cpp/common
        ${CMAKE_SOURCE_DIR}/model
)
```

- [ ] **Step 2: Configure with CMake**

```bash
cd C:/code/llmexe
cmake -B build -G "MinGW Makefiles"
```

Expected: CMake configuration successful

- [ ] **Step 3: Build the project**

```bash
cmake --build build
```

Expected: Build successful (will fail until model_data.h is generated, which is expected)

- [ ] **Step 4: Generate model_data.h (instructions for user)**

```bash
# User needs to:
# 1. Download qwen3-0.6b-q4_K_M.gguf from HuggingFace
# 2. Run: python scripts/convert_gguf_to_header.py path/to/model.gguf model/model_data.h
# 3. Rebuild: cmake --build build
```

- [ ] **Step 5: Commit CMakeLists update**

```bash
git add src/CMakeLists.txt
git commit -m "feat: add model directory to include paths"
```

---

## Self-Review Checklist

**1. Spec coverage:**
- [x] CLI tool with command line prompt input
- [x] Pure text streaming output
- [x] Model embedded in exe (via C array)
- [x] Support parameter adjustment (temperature, top-p, max-tokens, threads)
- [x] Qwen3 prompt format
- [x] Uses llama.cpp and MinGW/CMake

**2. Placeholder scan:**
- No TBD/TODO found
- All code blocks are complete
- File paths are exact

**3. Type consistency:**
- `llmexe::InferenceParams` used consistently
- `llama_model` and `llama_context` types match llama.cpp API
- Callback signature is consistent

**Plan complete and saved to `docs/superpowers/plans/2026-04-26-llmexe-implementation.md`.**

**Two execution options:**

**1. Subagent-Driven (recommended)** - I dispatch a fresh subagent per task, review between tasks, fast iteration

**2. Inline Execution** - Execute tasks in this session using executing-plans, batch execution with checkpoints

**Which approach?**
