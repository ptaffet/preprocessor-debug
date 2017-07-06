cmake_minimum_required(VERSION 3.3)
project(ClangPluginExamples CXX)

include(cmake/AddClangPlugin.cmake)


add_clang_executable(pp-step)
target_sources(pp-step
  PUBLIC
  pp-step.cpp)
