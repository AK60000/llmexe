# LLMExe

[English](README.md) | [中文](README_zh.md)

一个极简工具，用于将任意 `.gguf` 大语言模型打包成**单个、零依赖的 Windows 可执行文件**。

## 项目概述

LLMExe 改变了分发本地大模型的方式。不再需要用户自行下载模型、安装推理运行时（Python 或 llama.cpp）并运行复杂的命令行，本项目允许你**将任意 GGUF 模型直接绑定到一个静态 C++ 推理运行程序中**。

最终生成什么？一个`.exe` 文件，在任何全新的 Windows 电脑上双击或通过命令行即可运行，立刻执行你的大模型并以流式方式输出文本。

## 核心特性

- **零依赖**：生成的可执行文件完全静态链接，无需任何外部 DLL（甚至不需要 Visual C++ 运行库或 OpenMP）。
- **通用 GGUF 支持**：支持 `llama.cpp` v0.10.0+ 支持的所有量化格式和模型架构（Qwen、Llama、Mistral 等）。
- **自解压负载**：可执行文件在运行时将内置模型安全释放到 `%TEMP%` 目录，供原生内存映射加载，后续运行瞬间启动。
- **C++ 原生性能**：直接基于 `llama.cpp` 推理引擎构建，CPU 推理性能最优。

## 运行环境要求

- CMake (>= 3.20)
- 支持 C++17 的编译器 (如 GCC/MinGW-w64)
- 任意已下载的 `.gguf` 模型文件

## 编译

构建基础推理可执行文件（`build\bin\llmexe.exe`）。该可执行文件将在后续打包步骤中使用。

```bash
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

## 打包模型

构建完成后，准备好你的 `.gguf` 模型，使用 PowerShell 脚本将两者绑定为单文件：

```powershell
.\scripts\package.ps1 -ModelPath .\path\to\your_model.gguf -OutputPath .\my_standalone_llm.exe
```

这将创建 `my_standalone_llm.exe`（体积略大于原始 `.gguf` 文件）。

## 运行

现在你可以将 `my_standalone_llm.exe` 分发给任何人。他们可以直接运行：

```powershell
.\my_standalone_llm.exe --prompt "用一句话解释量子计算。"
```

*（提示：用户仍可通过 `-n 1024` 调整最大 Token 数、`--temp 0.8` 调整温度等参数。运行 `-h` 查看所有选项。）*

## 命令行参数

| 参数 | 说明 | 默认值 |
|------|------|--------|
| `-m, --model <path>` | GGUF 模型文件路径（仅在无内置模型时需要） | *（内置或报错）* |
| `-p, --prompt <text>` | 起始提示词 | *（必填）* |
| `-n, --max-tokens <n>` | 最大生成 Token 数 | `512` |
| `-t, --threads <n>` | 使用的 CPU 线程数 | `4` |
| `--temp <f>` | 采样温度 | `0.7` |
| `--top_p <f>` | Top-P 采样参数 | `0.9` |
| `-h, --help` | 显示帮助信息 | |

## 工作原理

LLMExe 由两部分组成：

1. **通用 C++ 推理运行程序**（`src/`）：使用 `llama.cpp` 加载并运行任意 GGUF 模型，以流式方式输出文本。
2. **自解压打包系统**（`src/self_extract.cpp` + `scripts/package.ps1`）：将 GGUF 模型二进制追加到可执行文件末尾，并在最末端附加 16 字节的尾部标记（`[8字节大小][LLMEXE00]`）。

运行时，可执行文件执行以下步骤：
1. 读取自身最后 16 字节，检查 `LLMEXE00` 魔术标记。
2. 若找到，则将内置的模型负载释放到 `%TEMP%`（带有大小缓存，重复运行跳过再次释放）。
3. 将提取出的 GGUF 文件路径交给 `llama.cpp` 的原生内存映射加载器。

## 项目结构

```
.
├── src/
│   ├── main.cpp          # CLI 入口
│   ├── model_loader.cpp  # llama.cpp 上下文初始化
│   ├── inference.cpp     # 分词与推理循环
│   ├── self_extract.cpp  # 内置模型检测与释放
│   └── *.h               # 对应头文件
├── scripts/
│   └── package.ps1       # PowerShell 打包脚本
├── third_party/
│   └── llama.cpp/        # Git 子模块（自动获取）
├── CMakeLists.txt
├── LICENSE               # MIT 许可证
├── README.md
└── README_zh.md
```

## 致谢

**llama.cpp** — https://github.com/ggml-org/llama.cpp  
本项目直接基于 `llama.cpp` 推理引擎构建，所有 GGUF 加载、分词、采样和模型评估逻辑均由 `llama.cpp`（MIT 许可证）提供。衷心感谢 `ggml-org` 团队构建并维护这一杰出的开源库。

## 相关项目与替代方案

**[Mozilla-Ozone/llamafile](https://github.com/mozilla-ai/llamafile)**：“单文件大模型”概念的先驱。`llamafile` 使用了 Cosmopolitan Libc 技术来实现令人惊叹的跨平台特性（同一个文件在 Windows/macOS/Linux 均可运行），并内置了 Web UI。而 **LLMExe** 选择了另一条路线：我们专注于提供一个**原生的、极致精简的 Windows 纯命令行替代方案**，使用最标准的 C++ 工具链（CMake/MinGW）构建，没有复杂的跨平台编译魔法，代码极易理解和二次开发。
