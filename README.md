# llmexe

[English](README.md) | [中文](README_zh.md)

A minimal, standalone C++ application for running local LLMs using the latest `llama.cpp` (v0.10.0+ API).

## Overview

This project provides a single executable (`llmexe`) that can load `.gguf` model files and perform stream-based text generation. It avoids Python dependencies by interacting directly with the native `llama.cpp` library.

## Prerequisites

- CMake (>= 3.10)
- A C++17 compatible compiler (e.g., GCC, Clang, MSVC)
- A downloaded `.gguf` model (e.g., Qwen3-0.6B-Q4_K_M.gguf)

## Build Instructions

```bash
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

The resulting executable will be available at `build/bin/llmexe` (or `build\bin\llmexe.exe` on Windows).

## Usage

```bash
./build/bin/llmexe --model path/to/your/model.gguf --prompt "Your prompt here" [options]
```

### Options

- `-m, --model <path>`: Path to the GGUF model file (required)
- `-p, --prompt <text>`: The initial prompt to start generating from (required)
- `-n, --max-tokens <n>`: Maximum number of tokens to generate (default: 512)
- `-t, --threads <n>`: Number of threads to use for generation (default: 4)
- `--temp <f>`: Temperature for sampling (default: 0.7)
- `--top_p <f>`: Top-P sampling parameter (default: 0.9)
- `-h, --help`: Show help message

## Example

```bash
./build/bin/llmexe -m ./models/Qwen3-0.6B-Q4_K_M.gguf -p "Explain quantum computing in one sentence." -n 100 --temp 0.6
```

## Architecture

- `src/main.cpp`: Entry point, CLI argument parsing.
- `src/model_loader.cpp`: Initializes the llama context, manages the model and backend lifetimes.
- `src/inference.cpp`: Handles tokenization, inference evaluation loops, and streaming output using the new `llama_sampler` API.
- `src/self_extract.cpp`: Enables standalone deployment by extracting an appended `.gguf` payload at runtime.
- `llama.cpp` integration: The project fetches and builds `llama.cpp` automatically via CMake `FetchContent`.

## Standalone Packaging

You can package the compiled executable and the `.gguf` model into a single, standalone executable that can be run anywhere on Windows without passing the `-m` flag.

Run the provided PowerShell packaging script:

```powershell
.\scripts\package.ps1
```

This will create `llmexe_standalone.exe` (approx. 400MB if using a 0.6B Q4 model). You can run this file directly:

```powershell
.\llmexe_standalone.exe --prompt "Hello! How are you?"
```

At runtime, the executable will automatically extract its embedded model to the system's temporary directory (`%TEMP%`) and load it. Subsequent runs are instantaneous because it checks for the extracted payload cache first.
