# llmexe - LLM Executable

将Qwen3-0.6B大模型通过llama.cpp集成到单一可执行文件中。

## 构建步骤

### 1. 克隆项目（如果还没克隆）
```bash
git clone <repo-url>
cd llmexe
```

### 2. 初始化submodule
```bash
git submodule update --init --recursive
```

### 3. 配置和构建（使用假模型数据）
```bash
cmake -B build -G "MinGW Makefiles"
cmake --build build
```

这会生成一个带有假模型数据的exe，模型加载会失败（预期行为）。

### 4. 下载真实模型并转换

#### 4.1 下载Qwen3-0.6B GGUF模型
从HuggingFace或ModelScope下载 `qwen3-0.6b-q4_K_M.gguf` 模型文件。

例如：
```bash
# 使用huggingface_hub
huggingface-cli download Qwen/Qwen3-0.6B-GGUF qwen3-0.6b-q4_K_M.gguf --local-dir .
```

#### 4.2 转换为C头文件
```bash
python scripts/convert_gguf_to_header.py path/to/qwen3-0.6b-q4_K_M.gguf model/model_data.h
```

**警告：** 转换后的model_data.h文件会非常大（几百MB到几GB），编译时间会很长（可能几小时）。

#### 4.3 重新构建
```bash
Remove-Item -Recurse -Force build
cmake -B build -G "MinGW Makefiles"
cmake --build build
```

## 使用方法

### 显示帮助
```bash
./build/bin/llmexe.exe --help
```

### 生成文本
```bash
./build/bin/llmexe.exe "你好，请介绍一下自己"
```

### 调整参数
```bash
./build/bin/llmexe.exe --temperature 0.8 --top-p 0.95 --max-tokens 1024 "写一个故事"
```

## 参数说明

- `--temperature FLOAT` - 温度参数，控制随机性（默认0.7）
- `--top-p FLOAT` - Top-p采样（默认0.9）
- `--max-tokens INT` - 最大生成token数（默认512）
- `--threads INT` - 线程数（默认自动检测）
- `-h, --help` - 显示帮助

## 注意事项

1. **模型嵌入exe**：模型数据被转换为C数组并编译进exe，导致：
   - exe文件非常大（和模型一样大，几百MB到几GB）
   - 编译时间很长
   - 每次修改模型都需要重新编译

2. **替代方案**：如果不需要模型嵌入exe，可以修改`src/model_loader.cpp`，支持从外部文件加载模型（更快的迭代速度）。

3. **Qwen3提示词格式**：程序自动使用Qwen3的聊天模板：
   ```
   <|im_start|>user
   {用户输入}<|im_end|>
   <|im_start|>assistant
   ```

## 项目结构

```
llmexe/
├── CMakeLists.txt          # 主构建配置
├── src/                   # 源代码
│   ├── main.cpp          # 入口，CLI解析
│   ├── model_loader.cpp  # 模型加载
│   └── inference.cpp    # 推理引擎
├── model/                 # 模型数据（gitignore，不提交）
├── scripts/               # 工具脚本
│   └── convert_gguf_to_header.py
└── third_party/
    └── llama.cpp/        # submodule
```

## 许可证

本项目遵循相应许可证（llama.cpp遵循其自身许可证）。
