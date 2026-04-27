#include <iostream>
#include <string>
#include <vector>
#include "llm.h"
#include "self_extract.h"

void printUsage(const char* prog) {
    std::cout << "Usage: " << prog << " [options]\n"
              << "Options:\n"
              << "  -m, --model <path>      Path to GGUF model file (not needed if model is embedded)\n"
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

    llmexe::InferenceParams params;
    
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
    
    if (model_path.empty()) {
        std::cerr << "Error: No embedded model found and no model specified via -m.\n"
                  << "Please package a model using the package.ps1 script, or specify one manually with -m.\n";
        printUsage(argv[0]);
        return 1;
    }

    llmexe::LLM runner;
    
    if (!runner.load(model_path, params)) {
        std::cerr << "Initialization failed: " << runner.getLastError() << "\n";
        return 1;
    }
    
    auto callback = [](const std::string& token) {
        std::cout << token << std::flush;
    };
    
    if (!runner.generate(prompt, callback)) {
        std::cerr << "\nGeneration failed: " << runner.getLastError() << "\n";
        return 1;
    }
    
    std::cout << std::endl;
    return 0;
}