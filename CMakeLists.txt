cmake_minimum_required(VERSION 3.3)
project(ClangPluginExamples CXX)

include(cmake/AddClangPlugin.cmake)

execute_process(
  COMMAND ${CMAKE_CXX_COMPILER} -E -xc++ -v /dev/null
  ERROR_FILE compiler_info.txt)

execute_process(
  COMMAND awk "/^End of search list/ \
               { p=0 } \
               { \
                 if(p==1) { \
                   printf \"\\\"%s\\\",\", $1 \
                 } \
               } \
               /include <...> search starts here/ \
               { p=1 }" compiler_info.txt
  OUTPUT_VARIABLE SYSTEM_HEADERS)

add_clang_executable(pp-step)

target_compile_definitions(pp-step
  PUBLIC SYSTEM_HEADERS=${SYSTEM_HEADERS})

target_sources(pp-step
  PUBLIC
  pp-step.cpp)
