cmake_minimum_required(VERSION 3.3)
project(ClangPluginExamples CXX)

include(cmake/AddClangPlugin.cmake)

execute_process(
  COMMAND ./find_sys_includes.sh
  OUTPUT_VARIABLE SYSTEM_HEADERS)

add_clang_executable(pp-step)

target_compile_definitions(pp-step
  PUBLIC SYSTEM_HEADERS=${SYSTEM_HEADERS})

target_sources(pp-step
  PUBLIC
  pp-step.cpp)
