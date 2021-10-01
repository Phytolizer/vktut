cmake_minimum_required(VERSION 3.12...3.21 FATAL_ERROR)

project(
  vkguide
  LANGUAGES C CXX
  VERSION 0.1.0
  DESCRIPTION "Learning Vulkan from VkGuide"
  HOMEPAGE_URL "https://vkguide.dev/"
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" "${PROJECT_SOURCE_DIR}/cmake")

include(cmake/cppcheck.cmake)
include(cmake/clang_format.cmake)

find_package(Vulkan REQUIRED)

configure_file(cmake/config.hpp.cin "${PROJECT_BINARY_DIR}/config.hpp")

message("PROJECT SOURCE DIR: ${PROJECT_SOURCE_DIR}")
file(GLOB GLSL_SOURCE_FILES CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/shaders/*.glsl")
message("GLSL SOURCE FILES: ${GLSL_SOURCE_FILES}")
foreach(GLSL ${GLSL_SOURCE_FILES})
  get_filename_component(FILE_NAME "${GLSL}" NAME_WLE)
  get_filename_component(SHADER_STAGE "${FILE_NAME}" LAST_EXT)
  string(REPLACE "." "" SHADER_STAGE "${SHADER_STAGE}")
  set(SPIRV "${CMAKE_BINARY_DIR}/shaders/${FILE_NAME}.spv")
  message("BUILDING SHADER ${FILE_NAME} (${SHADER_STAGE}) ==> ${SPIRV}")
  add_custom_command(
    OUTPUT "${SPIRV}"
    COMMAND "${CMAKE_COMMAND}" -E make_directory "${CMAKE_BINARY_DIR}/shaders"
    COMMAND "${Vulkan_GLSLC_EXECUTABLE}" -fshader-stage="${SHADER_STAGE}"
            "${GLSL}" -o "${SPIRV}"
    DEPENDS "${GLSL}"
  )
  list(APPEND SPIRV_BINARY_FILES "${SPIRV}")
endforeach()

add_custom_target(shaders DEPENDS "${SPIRV_BINARY_FILES}")

set(LIBC ON CACHE BOOL "Use libc")

add_subdirectory("third-party/glm")
add_subdirectory("third-party/SDL2-2.0.16")
add_subdirectory("third-party/imgui-1.84.2")
add_subdirectory("third-party/stb-master")
add_subdirectory("third-party/tiny_obj_loader")
add_subdirectory("third-party/vk-bootstrap-0.4")
add_subdirectory("third-party/VulkanMemoryAllocator-master")

add_subdirectory("chapter-0")
