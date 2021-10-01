add_executable(vkguide_chapter0 WIN32 "main.cpp" "vk_initializers.cpp" "vk_engine.cpp" "vk_pipeline.h" "vk_pipeline.cpp")
set_target_properties(
  vkguide_chapter0 PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY
                              "${PROJECT_SOURCE_DIR}"
)
target_include_directories(
  vkguide_chapter0 PRIVATE "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/chapter-0>" "$<BUILD_INTERFACE:${PROJECT_BINARY_DIR}>"
)
target_link_libraries(
  vkguide_chapter0
  PRIVATE "SDL2-static"
          SDL2main
          imgui
          VulkanMemoryAllocator
          glm::glm
          stb
          tiny_obj_loader
          vk-bootstrap::vk-bootstrap
          Vulkan::Vulkan
)
target_compile_features(vkguide_chapter0 PRIVATE cxx_std_20)
add_dependencies(vkguide_chapter0 shaders)
