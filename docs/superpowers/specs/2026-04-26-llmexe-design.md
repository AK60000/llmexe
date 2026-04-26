# llmexe - LLM可执行文件设计文档

## 概述

将Qwen3-0.6B大模型通过llama.cpp集成到单一可执行文件中，提供命令行接口进行推理。

## 需求

- **用途**：命令行工具，通过参数接收提示词并输出推理结果
- **输入**：命令行参数传入提示词
- **输出**：纯文本流式输出（逐token输出）
- **模型**：Qwen3-0.6B-q4_K_M.gguf，嵌入exe中
- **参数**：支持调整temperature、top-p、max-tokens等生成参数

## 架构设计

### 整体架构

```
[用户输入] → [命令行解析] → [模型加载(内存)] → [推理引擎] → [流式输出]
```

### 核心组件

1. **main.cpp** - 程序入口，命令行参数解析
2. **model_loader** - 从嵌入的模型数据加载llama模型
3. **inference** - 处理推理逻辑和流式输出
4. **嵌入式模型** - 将qwen3-0.6b-q4_K_M.gguf转换为C数组

### 依赖

- llama.cpp (作为静态库编译)
- mingw-w64 (编译器)
- cmake (构建系统)

## 项目结构

```
llmexe/
├── CMakeLists.txt          # 构建配置
├── src/
│   ├── main.cpp           # 程序入口
│   ├── model_loader.h     # 模型加载接口
│   ├── model_loader.cpp   # 从内存加载模型
│   ├── inference.h        # 推理接口
│   └── inference.cpp      # 推理实现
├── model/
│   └── model_data.h       # 嵌入式模型数据(C数组)
└── third_party/
    └── llama.cpp/         # 子模块或源码
```

## 命令行参数设计

```
llmexe.exe [选项] "提示词"
  --temperature FLOAT   温度参数(默认0.7)
  --top-p FLOAT         top-p采样(默认0.9)
  --max-tokens INT      最大生成token数(默认512)
  --threads INT         线程数(默认系统核心数)
  -h, --help            显示帮助
```

## 模型嵌入方案

采用方案1：静态嵌入

- 使用xxd或python脚本将gguf转换为C数组
- 生成 `const unsigned char model_data[] = {...};`
- 在CMake中配置该文件参与编译
- 使用llama.cpp的`llama_model_load_from_buffer`从内存加载

## 核心逻辑

### 模型加载 (model_loader.cpp)

```cpp
// 模型数据在model_data.h中声明为外部变量
extern const unsigned char model_data[];
extern const unsigned int model_data_size;

// 使用llama.cpp的从缓冲区加载功能
llama_model * model = llama_model_load_from_buffer(
    model_data, 
    model_data_size, 
    llama_model_params
);
```

### 推理流程 (inference.cpp)

```cpp
// 1. 创建上下文
llama_context * ctx = llama_create_context(model, ctx_params);

// 2. 构建提示词（qwen3格式）
std::string prompt = build_qwen_prompt(user_input);

// 3. 流式生成
while (llama_decode(ctx, token) == 0) {
    // 将新token转为文本并输出
    std::string chunk = token_to_text(new_tokens);
    std::cout << chunk << std::flush;  // 流式输出
}
```

### 数据流

```
命令行参数 → 解析 → 加载模型(内存) → 构建prompt → 推理 → 逐token输出 → 清场
```

## 错误处理

- 模型加载失败：输出错误并退出
- 推理失败：输出错误信息
- 参数错误：显示帮助信息

## Qwen3提示词格式

使用Qwen3的聊天模板：
```
<|im_start|>user
{用户输入}<|im_end|>
<|im_start|>assistant

```

## 构建流程

1. 克隆llama.cpp到third_party/llama.cpp（使用git submodule add https://github.com/ggerganov/llama.cpp.git third_party/llama.cpp）
2. 下载qwen3-0.6b-q4_K_M.gguf模型（从HuggingFace或ModelScope）
3. 使用Python脚本将gguf转换为model_data.h：
   ```python
   # convert_gguf_to_header.py
   import sys
   with open(sys.argv[1], 'rb') as f:
       data = f.read()
   with open(sys.argv[2], 'w') as out:
       out.write(f'const unsigned int model_data_size = {len(data)};\n')
       out.write('const unsigned char model_data[] = {\n')
       for i, b in enumerate(data):
           if i % 16 == 0:
               out.write('    ')
           out.write(f'0x{b:02x},')
           if i % 16 == 15:
               out.write('\n')
       out.write('\n};\n')
   ```
4. 使用cmake配置：cmake -B build -G "MinGW Makefiles"
5. 编译：cmake --build build
