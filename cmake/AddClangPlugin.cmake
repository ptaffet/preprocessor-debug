find_package(Clang REQUIRED)

set(CLANG_LIBRARIES clangAST clangASTMatchers clangAnalysis clangBasic clangDriver clangEdit clangFrontend clangFrontendTool clangLex clangParse clangSema clangEdit clangRewrite clangRewriteFrontend clangStaticAnalyzerFrontend clangStaticAnalyzerCheckers clangStaticAnalyzerCore clangSerialization clangToolingCore clangTooling clangFormat)

macro(add_clang_plugin _name)
  add_library(${_name} SHARED "")
  set_target_properties(${_name}
    PROPERTIES
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED On
    CXX_EXTENSIONS Off)
  target_include_directories(${_name}
    PRIVATE
    ${LLVM_INCLUDE_DIR})
  target_link_libraries(${_name}
    libclang)
endmacro(add_clang_plugin)

macro(add_clang_executable _name)
  add_executable(${_name} "")
  set_target_properties(${_name}
    PROPERTIES
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED On
    CXX_EXTENSIONS Off)
  target_include_directories(${_name}
    PRIVATE
    ${LLVM_INCLUDE_DIR})
  target_link_libraries(${_name}
    ${CLANG_LIBRARIES} ${LLVM_AVAILABLE_LIBS})
endmacro(add_clang_executable)
