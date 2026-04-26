#include <iostream>
#include <string>
#include <vector>
#include "model_loader.h"
#include "inference.h"
#include "model_data.h"

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
