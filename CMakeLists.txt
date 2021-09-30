cmake_minimum_required(VERSION 3.12...3.21 FATAL_ERROR)

project(
  vktut
  LANGUAGES CXX
  VERSION 0.1.0
  DESCRIPTION "Learning Vulkan from the official tutorial"
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" "${PROJECT_SOURCE_DIR}/cmake")

include(cmake/cppcheck.cmake)
include(cmake/clang_format.cmake)

find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)
find_package(GLM REQUIRED)

configure_file(cmake/config.hpp.cin "${PROJECT_BINARY_DIR}/config.hpp")

find_program(GLSL_VALIDATOR glslc REQUIRED HINTS "$ENV{VULKAN_SDK}/Bin")

file(GLOB_RECURSE GLSL_SOURCE_FILES CONFIGURE_DEPENDS
     "Resources/Shaders/*.vert" "Resources/Shaders/*.frag"
)

foreach(GLSL_SOURCE_FILE ${GLSL_SOURCE_FILES})
  get_filename_component(FILE_NAME ${GLSL_SOURCE_FILE} NAME)
  set(SPIRV_OUTPUT_FILE
      "${PROJECT_BINARY_DIR}/Resources/Shaders/${FILE_NAME}.spv"
  )
  add_custom_command(
    OUTPUT ${SPIRV_OUTPUT_FILE}
    COMMAND ${CMAKE_COMMAND} -E make_directory
            "${PROJECT_BINARY_DIR}/Resources/Shaders"
    COMMAND ${GLSL_VALIDATOR} ${GLSL_SOURCE_FILE} -o ${SPIRV_OUTPUT_FILE}
    DEPENDS ${GLSL_SOURCE_FILE}
  )
  list(APPEND SPIRV_OUTPUT_FILES ${SPIRV_OUTPUT_FILE})
endforeach()

add_custom_target(vktut_shaders DEPENDS ${SPIRV_OUTPUT_FILES})

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "Source/Library/*.cpp")
add_library(vktut_lib OBJECT ${SOURCES})
target_include_directories(
	vktut_lib PUBLIC "${PROJECT_SOURCE_DIR}/Include" "${PROJECT_BINARY_DIR}" "${GLFW3_INCLUDE_DIRS}" "${GLM_INCLUDE_DIRS}"
)
target_compile_features(vktut_lib PUBLIC cxx_std_20)
target_link_libraries(vktut_lib PUBLIC "${GLFW3_LIBRARIES}")
target_link_libraries(vktut_lib PUBLIC Vulkan::Vulkan)
add_dependencies(vktut_lib vktut_shaders)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "Source/Executable/*.cpp")
add_executable(vktut_exe ${SOURCES})
target_link_libraries(vktut_exe PRIVATE vktut_lib)
set_target_properties(vktut_exe PROPERTIES OUTPUT_NAME vktut)

add_custom_command(
  TARGET vktut_exe
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E make_directory
          "$<TARGET_FILE_DIR:vktut_exe>/Resources/Shaders"
  COMMAND
    ${CMAKE_COMMAND} -E copy_directory
    "${PROJECT_BINARY_DIR}/Resources/Shaders"
    "$<TARGET_FILE_DIR:vktut_exe>/Resources/Shaders"
)
