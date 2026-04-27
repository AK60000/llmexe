cmake_minimum_required(VERSION 3.20)
project(llmexe VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enforce FULL static linking for MinGW
if(MINGW)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -static")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static")
endif()

# Turn off OpenMP for extreme portability (removes libgomp-1.dll dependency entirely)
set(GGML_OPENMP OFF CACHE BOOL "Disable OpenMP" FORCE)

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
