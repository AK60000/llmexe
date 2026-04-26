# Create the executable
add_executable(llmexe
    main.cpp
    model_loader.cpp
    inference.cpp
    ${CMAKE_SOURCE_DIR}/model/model_data.cpp
)

# Link llama.cpp libraries (conditional)
if(LLMEXE_BUILD_LLAMA)
    target_link_libraries(llmexe
        PRIVATE
            llama
            ggml_static
    )
endif()

# Include directories
target_include_directories(llmexe
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/third_party/llama.cpp
        ${CMAKE_SOURCE_DIR}/third_party/llama.cpp/common
        ${CMAKE_SOURCE_DIR}/model
)

# Set compile definitions (platform-specific)
if(WIN32)
    target_compile_definitions(llmexe
        PRIVATE
            _WIN32
    )
endif()
