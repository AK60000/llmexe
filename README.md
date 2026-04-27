# LLMExe

[English](README.md) | [中文](README_zh.md)

A minimal, generic tool for packaging any `.gguf` Large Language Model into a **single, zero-dependency Windows executable**. 

## Project Overview

LLMExe shifts the paradigm of distributing local LLMs. Instead of requiring users to download a model, install an inference runtime (like python or llama.cpp), and run complicated CLI commands, this project lets you **bundle any known GGUF model directly into a static C++ inference runner**.

The result? A single `.exe` file that can be double-clicked or run from the command line on any clean Windows machine, instantly executing your LLM with stream-based text generation.

## Features

- **Zero Dependencies**: The resulting `.exe` is fully statically linked. No external DLLs (not even Visual C++ Redistributables or OpenMP) are required.
- **Universal GGUF Support**: Works with models quantization formats supported by `llama.cpp` v0.10.0+ (Qwen, Llama, Mistral, etc.).
- **Self-Extracting Payload**: The executable securely unpacks its embedded model to `%TEMP%` at runtime for native memory-mapped loading, ensuring blazing fast startup times on subsequent runs.
- **C++ Native Performance**: Built directly on top of the `llama.cpp` inference engine for maximum CPU performance.

## Prerequisites

- CMake (>= 3.20)
- A C++17 compatible compiler (e.g., GCC/MinGW-w64)
- Any downloaded `.gguf` model

## Compilation

Build the base inference executable (`build\bin\llmexe.exe`). This base executable will be used by the packaging script later.

```bash
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

## Packaging a Model

Once you have the base executable built and a `.gguf` model downloaded, use the provided PowerShell script to bundle them together:

```powershell
.\scripts\package.ps1 -ModelPath .\path\to\your_model.gguf -OutputPath .\my_standalone_llm.exe
```

This creates `my_standalone_llm.exe` (which will be slightly larger than your `.gguf` file). 

## Usage

You can now distribute `my_standalone_llm.exe` to anyone. They can run it natively:

```powershell
.\my_standalone_llm.exe --prompt "Explain quantum computing in one sentence."
```

*(Note: They can still override parameters like `-n 1024` for max tokens or `--temp 0.8` for temperature. Run with `-h` for all options).*

## Command-Line Options

| Flag | Description | Default |
|------|-------------|---------|
| `-m, --model <path>` | Path to GGUF model file (only needed if no model is embedded) | *(embedded or error)* |
| `-p, --prompt <text>` | The prompt to generate from | *(required)* |
| `-n, --max-tokens <n>` | Maximum number of tokens to generate | `512` |
| `-t, --threads <n>` | Number of CPU threads to use | `4` |
| `--temp <f>` | Sampling temperature | `0.7` |
| `--top_p <f>` | Top-P sampling | `0.9` |
| `-h, --help` | Show help | |

## How It Works

LLMExe consists of two parts:

1. **A generic C++ inference runner** (`src/`) that uses `llama.cpp` to load and run any GGUF model with streaming text output.
2. **A self-extraction packaging system** (`src/self_extract.cpp` + `scripts/package.ps1`) that appends a GGUF model binary to the executable and registers a 16-byte footer (`[8-byte size][LLMEXE00]`) at the very end.

At runtime, the executable:
1. Reads its own last 16 bytes to check for the `LLMEXE00` magic.
2. If found, extracts the embedded payload to `%TEMP%` (with a size-cache so repeated runs skip re-extraction).
3. Feeds the extracted GGUF file path to `llama.cpp`'s native memory-mapped loader.

## Project Structure

```
.
├── src/
│   ├── main.cpp          # CLI entry point
│   ├── model_loader.cpp  # llama.cpp context initialization
│   ├── inference.cpp     # Tokenization & inference loop
│   ├── self_extract.cpp  # Embedded payload detection & extraction
│   └── *.h               # Corresponding headers
├── scripts/
│   └── package.ps1       # PowerShell packaging script
├── third_party/
│   └── llama.cpp/        # Git submodule (auto-fetched)
├── CMakeLists.txt
├── LICENSE               # MIT
├── README.md
└── README_zh.md
```

## Acknowledgments

**llama.cpp** — https://github.com/ggml-org/llama.cpp  
This project is built directly on top of the `llama.cpp` inference engine. All GGUF loading, tokenization, sampling, and model evaluation logic is provided by `llama.cpp` (MIT License). We gratefully acknowledge the `ggml-org` team for building and maintaining this exceptional open-source library.

## Related Projects & Alternatives

**[Mozilla-Ozone/llamafile](https://github.com/mozilla-ai/llamafile)**: The pioneer of the "single-file LLM" concept. While `llamafile` uses Cosmopolitan Libc to create a brilliant multi-platform executable (Windows/macOS/Linux in one file) with a built-in web server, **LLMExe** takes a different approach. LLMExe focuses on being a **Windows-native, radically simplified, and minimal CLI alternative** built with standard C++ toolchains (CMake/MinGW) without complex cross-compilation magic.
