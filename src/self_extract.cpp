#include "self_extract.h"
#include <fstream>
#include <iostream>
#include <vector>
#include <cstring>
#include <algorithm>
#include <cstdint>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

namespace llmexe {

std::string getExecutablePath() {
#ifdef _WIN32
    char buffer[MAX_PATH];
    GetModuleFileNameA(NULL, buffer, MAX_PATH);
    return std::string(buffer);
#else
    char buffer[1024];
    ssize_t len = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
    if (len != -1) {
        buffer[len] = '\0';
        return std::string(buffer);
    }
    return "";
#endif
}

std::string getTempDirectory() {
#ifdef _WIN32
    char buffer[MAX_PATH];
    GetTempPathA(MAX_PATH, buffer);
    return std::string(buffer);
#else
    const char* tmp = getenv("TMPDIR");
    return tmp ? std::string(tmp) + "/" : "/tmp/";
#endif
}

std::string extractEmbeddedModel() {
    std::string exe_path = getExecutablePath();
    if (exe_path.empty()) return "";

    std::ifstream file(exe_path, std::ios::binary | std::ios::ate);
    if (!file) return "";

    std::streampos size = file.tellg();
    if (size < 16) return "";

    file.seekg(-16, std::ios::end);
    char footer[16];
    file.read(footer, 16);

    // Check magic "LLMEXE00"
    if (std::string(footer + 8, 8) != "LLMEXE00") {
        return ""; // No embedded model
    }

    uint64_t payload_size = 0;
    std::memcpy(&payload_size, footer, sizeof(uint64_t));

    if (payload_size == 0 || (uint64_t)size < payload_size + 16) {
        std::cerr << "Invalid embedded payload size." << std::endl;
        return "";
    }

    uint64_t payload_offset = (uint64_t)size - 16 - payload_size;

    std::string temp_path = getTempDirectory() + "llmexe_embedded_model.gguf";

    // Fast check: if temp file exists and size matches, skip extraction
    std::ifstream temp_check(temp_path, std::ios::binary | std::ios::ate);
    if (temp_check) {
        if ((uint64_t)temp_check.tellg() == payload_size) {
            // Already extracted
            return temp_path;
        }
    }
    temp_check.close();

    std::cout << "Extracting embedded model to " << temp_path << "..." << std::endl;

    file.seekg(payload_offset, std::ios::beg);
    std::ofstream out_file(temp_path, std::ios::binary);
    if (!out_file) {
        std::cerr << "Failed to open temporary file for extraction." << std::endl;
        return "";
    }

    const size_t buffer_size = 1024 * 1024 * 4; // 4MB chunks
    std::vector<char> buffer(buffer_size);
    uint64_t bytes_left = payload_size;

    while (bytes_left > 0) {
        size_t to_read = std::min((uint64_t)buffer_size, bytes_left);
        if (!file.read(buffer.data(), to_read)) break;
        out_file.write(buffer.data(), to_read);
        bytes_left -= to_read;
    }

    return temp_path;
}

} // namespace llmexe
