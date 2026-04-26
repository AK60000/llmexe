cmake_minimum_required(VERSION 3.20)
project(llmexe VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Option for building llama.cpp
option(LLMEXE_BUILD_LLAMA "Build llama.cpp as submodule" OFF)

# Include llama.cpp
if(LLMEXE_BUILD_LLAMA)
    add_subdirectory(third_party/llama.cpp)
endif()

# Add our source directory
add_subdirectory(src)
