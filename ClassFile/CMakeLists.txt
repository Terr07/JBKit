cmake_minimum_required(VERSION 3.7)

project(ClassFile)
set(CMAKE_CXX_STANDARD 17)

add_compile_options("-pedantic")
add_compile_options("-Wall")

include(FetchContent)

FetchContent_Declare(
  fmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt
  GIT_TAG f5e54359df4c26b6230fc61d38aa294581393084 #v10.1.1
  )

FetchContent_MakeAvailable(fmt)


add_library(ClassFile "src/Parser.cpp"
                      "src/Serializer.cpp" 
                      "src/Instruction.cpp" 
                      "src/ConstantPool.cpp" 
                      "src/Attribute.cpp")

target_include_directories(ClassFile PRIVATE "src")
target_include_directories(ClassFile PUBLIC "include")

target_link_libraries(ClassFile PRIVATE fmt)

add_executable(readclass "example/readclass.cpp")
target_link_libraries(readclass PUBLIC ClassFile)

add_executable(dupeclass "example/dupeclass.cpp")
target_link_libraries(dupeclass PUBLIC ClassFile)
