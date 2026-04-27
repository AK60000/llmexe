# llmexe

[English](README.md) | [中文](README_zh.md)

一个极简的、独立的 C++ 应用程序，用于使用最新的 `llama.cpp` (v0.10.0+ API) 运行本地大语言模型 (LLM)。

## 项目概述

本项目提供了一个单一的可执行文件（`llmexe`），它可以加载 `.gguf` 格式的模型文件并执行基于流式输出的文本生成。通过直接与原生的 `llama.cpp` 库交互，本项目彻底避免了任何 Python 依赖。

## 运行环境要求

- CMake (>= 3.10)
- 支持 C++17 的编译器 (如 GCC、Clang、MSVC 等)
- 已下载好的 `.gguf` 模型 (例如 Qwen3-0.6B-Q4_K_M.gguf)

## 编译指南

```bash
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

编译产出的可执行文件将位于 `build/bin/llmexe` (Windows 下为 `build\bin\llmexe.exe`)。

## 基本用法

```bash
./build/bin/llmexe --model path/to/your/model.gguf --prompt "你的提示词" [options]
```

### 参数选项

- `-m, --model <path>`: 指定 GGUF 模型文件的路径 (必填)
- `-p, --prompt <text>`: 开始生成的初始提示词 prompt (必填)
- `-n, --max-tokens <n>`: 允许生成的最大 Token 数量 (默认: 512)
- `-t, --threads <n>`: 生成时使用的线程数 (默认: 4)
- `--temp <f>`: 采样温度 Temperature (默认: 0.7)
- `--top_p <f>`: Top-P 采样参数 (默认: 0.9)
- `-h, --help`: 显示帮助信息

## 示例

```bash
./build/bin/llmexe -m ./models/Qwen3-0.6B-Q4_K_M.gguf -p "用一句话解释量子计算。" -n 100 --temp 0.6
```

## 架构说明

- `src/main.cpp`: 入口程序，负责解析命令行参数。
- `src/model_loader.cpp`: 初始化 Llama 上下文，管理模型和后端的生命周期。
- `src/inference.cpp`: 处理分词 (Tokenization)、推理评估循环以及使用最新的 `llama_sampler` API 进行流式输出。
- `src/self_extract.cpp`: 通过在运行时自动提取追加在可执行文件尾部的 `.gguf` 负载内容，实现无需外部依赖的单文件独立部署功能。
- `llama.cpp` 集成: 项目通过 CMake 的 `FetchContent` 模块自动拉取并构建最新的 `llama.cpp`。

## 单文件独立打包 (Standalone)

您可以将编译出的可执行文件和 `.gguf` 模型合并打包成一个独立的单文件应用程序。这样便可在任何一台 Windows 电脑上直接运行，而无需附带模型文件或使用 `-m` 参数。

只需运行提供的 PowerShell 打包脚本：

```powershell
.\scripts\package.ps1
```

这将会生成 `llmexe_standalone.exe` (如果使用的是 0.6B Q4 模型，体积约 400MB)。然后您可以直接运行它：

```powershell
.\llmexe_standalone.exe --prompt "你好！最近怎么样？"
```

在运行时，程序会自动将内部包含的模型提取到系统的临时目录 (`%TEMP%`) 中并加载。由于程序会检查缓存的负载文件，因此之后的再次运行将会是瞬间启动的。
