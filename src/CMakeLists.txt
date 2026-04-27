# Create the executable
add_executable(llmexe 
    main.cpp
    llm.cpp
    self_extract.cpp
)

# Enforce FULL static linking for MinGW to avoid DLL hell
if(MINGW)
    target_link_options(llmexe PRIVATE -static)
    # Sometimes OpenMP also needs static linking explicitly
    # The flag -static handles libstdc++, libgcc, and libgomp in most cases
    # We add -static to both C and CXX flags in root as well just in case.
endif()

# Link llama.cpp libraries (conditional)
if(LLMEXE_BUILD_LLAMA)
    target_link_libraries(llmexe
        PRIVATE
            llama
            ggml
            ggml-base
            ggml-cpu
    )
endif()

# Include directories
target_include_directories(llmexe
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/third_party/llama.cpp
)

# Set compile definitions (platform-specific)
if(WIN32)
    target_compile_definitions(llmexe
        PRIVATE
            _WIN32
    )
endif()
