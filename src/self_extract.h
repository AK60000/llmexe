#pragma once
#include <string>

namespace llmexe {
    // Checks if the current executable has an embedded model.
    // If so, extracts it to the temp directory and returns the path to the temp file.
    // Returns an empty string if no embedded model is found or if extraction fails.
    std::string extractEmbeddedModel();
}
