include("${PROJECT_SOURCE_DIR}/cmake/enumerate_all_sources.cmake")

find_program(CLANG_FORMAT clang-format HINTS ENV PATH REQUIRED)
message(STATUS "clang-format found: ${CLANG_FORMAT}")

add_custom_target(
  "clang-format" COMMAND "${CLANG_FORMAT}" -i --verbose ${ALL_SOURCE_FILES}
)
