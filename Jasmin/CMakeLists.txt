cmake_minimum_required(VERSION 3.16)

project(Jasmin)
set(CMAKE_CXX_STANDARD 17)

add_compile_options("-pedantic")
add_compile_options("-Wall")


include(FetchContent)

FetchContent_Declare(
  fmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt
  )

FetchContent_MakeAvailable(fmt)

add_library(Jasmin     "src/Lexer.cpp"
                       "src/Assembler.cpp"
                       "src/Disassembler.cpp")

target_include_directories(Jasmin PUBLIC "include")

target_link_libraries(Jasmin PRIVATE fmt)

add_subdirectory("${PROJECT_SOURCE_DIR}/../ClassFile" "ClassFile")
target_link_libraries(Jasmin PUBLIC ClassFile)

if(EXISTS "${PROJECT_SOURCE_DIR}/spike/")
  add_executable(spike "spike/spike.cpp")
  target_link_libraries(spike PUBLIC Jasmin)
endif()

