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

add_library(Jasmin     "src/AstNodes.cpp"
                       "src/Lexer.cpp"
                       "src/Parser.cpp")

target_include_directories(Jasmin PUBLIC "include")

target_link_libraries(Jasmin PRIVATE fmt)

if(EXISTS "${PROJECT_SOURCE_DIR}/spike/")
  add_executable(spike "spike/spike.cpp")
  target_link_libraries(spike PUBLIC Jasmin)
endif()

add_executable(spike "spike/spike.cpp")
target_link_libraries(spike PUBLIC Jasmin)

