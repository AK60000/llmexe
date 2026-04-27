#include <iostream>
#include <string>
#include <vector>
#include "model_loader.h"
#include "inference.h"
#include "self_extract.h"

void printUsage(const char* prog) {
    std::cout << "Usage: " << prog << " [options]\n"
              << "Options:\n"
              << "  -m, --model <path>      Path to GGUF model file\n"
              << "  -p, --prompt <text>     Prompt to generate from\n"
              << "  -n, --max-tokens <n>    Maximum number of tokens to generate (default: 512)\n"
              << "  -t, --threads <n>       Number of threads to use (default: 4)\n"
              << "  --temp <f>              Temperature (default: 0.7)\n"
              << "  --top_p <f>             Top-P (default: 0.9)\n"
              << "  -h, --help              Show this help message\n";
}

int main(int argc, char** argv) {
    std::string prompt = "Hello, how are you?";
    std::string model_path = llmexe::extractEmbeddedModel();
    if (model_path.empty()) {
        model_path = "Qwen3-0.6B-Q4_K_M.gguf"; // Default fallback
    }

    llmexe::InferenceParams params;
    params.max_tokens = 512;
    params.num_threads = 4;
    params.temperature = 0.7f;
    params.top_p = 0.9f;
    
    // Parse arguments
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "-h" || arg == "--help") {
            printUsage(argv[0]);
            return 0;
        } else if ((arg == "-m" || arg == "--model") && i + 1 < argc) {
            model_path = argv[++i];
        } else if ((arg == "-p" || arg == "--prompt") && i + 1 < argc) {
            prompt = argv[++i];
        } else if ((arg == "-n" || arg == "--max-tokens") && i + 1 < argc) {
            params.max_tokens = std::stoi(argv[++i]);
        } else if ((arg == "-t" || arg == "--threads") && i + 1 < argc) {
            params.num_threads = std::stoi(argv[++i]);
        } else if (arg == "--temp" && i + 1 < argc) {
            params.temperature = std::stof(argv[++i]);
        } else if (arg == "--top_p" && i + 1 < argc) {
            params.top_p = std::stof(argv[++i]);
        } else {
            std::cerr << "Unknown argument: " << arg << "\n";
            printUsage(argv[0]);
            return 1;
        }
    }
    
    // std::cout << "Loading model " << model_path << "..." << std::endl;
    
    llmexe::ModelLoader loader;
    
    // Turn off llama.cpp's stderr logging unless necessary
    // We keep it default here, users expect some loading info
    
    if (!loader.loadFromFile(model_path)) {
        std::cerr << "Failed to load model: " << loader.getLastError() << "\n";
        return 1;
    }
    
    llmexe::InferenceEngine engine(loader.getModel());
    
    if (!engine.initialize(params)) {
        std::cerr << "Failed to initialize inference engine: " << engine.getLastError() << "\n";
        return 1;
    }
    
    // std::cout << "\nGeneration started. Prompt: '" << prompt << "'\n\n";
    
    auto callback = [](const std::string& token) {
        std::cout << token << std::flush;
    };
    
    if (!engine.generate(prompt, callback)) {
        std::cerr << "\nGeneration failed: " << engine.getLastError() << "\n";
        return 1;
    }
    
    std::cout << std::endl;
    return 0;
}
