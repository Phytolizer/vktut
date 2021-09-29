include("${PROJECT_SOURCE_DIR}/cmake/enumerate_all_sources.cmake")

find_program(CPPCHECK cppcheck cppcheck.exe HINTS ENV PATH "C:/Program Files/Cppcheck" REQUIRED)

message(STATUS "cppcheck found: ${CPPCHECK}")
add_custom_target(
  cppcheck
  COMMAND
    ${CPPCHECK}
    --enable=all
    --std=c++20 --project="${PROJECT_BINARY_DIR}/compile_commands.json"
    --template="[{severity}][{id}] {message} {callstack} \(On {file}:{line}\)"
    --suppress=missingIncludeSystem
    --quiet 
    ${ALL_SOURCE_FILES}
)
