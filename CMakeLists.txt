cmake_minimum_required(VERSION 3.12...3.21 FATAL_ERROR)

project(
  vktut
  LANGUAGES CXX
  VERSION 0.1.0
  DESCRIPTION "Learning Vulkan from the official tutorial"
)

find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)

configure_file(cmake/config.hpp.cin "${CMAKE_BINARY_DIR}/config.hpp")

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "Source/Library/*.cxx")
add_library(vktut_lib OBJECT ${SOURCES})
target_include_directories(
  vktut_lib PUBLIC "${PROJECT_SOURCE_DIR}/Include" "${CMAKE_BINARY_DIR}"
)
target_compile_features(vktut_lib PUBLIC cxx_std_20)
target_link_libraries(vktut_lib PUBLIC Vulkan::Vulkan glfw)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "Source/Executable/*.cxx")
add_executable(vktut_exe ${SOURCES})
target_link_libraries(vktut_exe PRIVATE vktut_lib)
