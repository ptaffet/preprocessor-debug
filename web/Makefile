
CLANG_LIB_PATH=~/clang/llvm-build/lib/

EMSCRIPTEN_FLAGS=-s NO_EXIT_RUNTIME=1 -s EXPORTED_FUNCTIONS='["_main"]' 
CXXFLAGS=-g -std=c++11 -Oz

CLANG_BIN_PATH=$(shell which clang)
CLANG_INC_PATH="$(dir $(CLANG_BIN_PATH))/../include"

# All of these libraries need to be LLVM bitcode, not machine code. Building them is not fun.
RQD_LIBS=libclangAST.a  libclangASTMatchers.a  libclangAnalysis.a  libclangBasic.a  libclangDriver.a  libclangEdit.a  libclangFrontend.a  libclangFrontendTool.a  libclangLex.a  libclangParse.a  libclangSema.a  libclangEdit.a  libclangRewrite.a  libclangRewriteFrontend.a  libclangStaticAnalyzerFrontend.a  libclangStaticAnalyzerCheckers.a  libclangStaticAnalyzerCore.a  libclangSerialization.a  libclangToolingCore.a  libclangTooling.a  libclangFormat.a  libLLVMDemangle.a  libLLVMSupport.a  libLLVMTableGen.a  libLLVMCore.a  libLLVMIRReader.a  libLLVMCodeGen.a  libLLVMSelectionDAG.a  libLLVMAsmPrinter.a  libLLVMMIRParser.a  libLLVMGlobalISel.a  libLLVMBitReader.a  libLLVMBitWriter.a  libLLVMTransformUtils.a  libLLVMInstrumentation.a  libLLVMInstCombine.a  libLLVMScalarOpts.a  libLLVMipo.a  libLLVMVectorize.a  libLLVMObjCARCOpts.a  libLLVMCoroutines.a  libLLVMLinker.a  libLLVMAnalysis.a  libLLVMLTO.a  libLLVMMC.a  libLLVMMCParser.a  libLLVMMCDisassembler.a  libLLVMObject.a  libLLVMObjectYAML.a  libLLVMOption.a  libLLVMDebugInfoDWARF.a  libLLVMDebugInfoMSF.a  libLLVMDebugInfoCodeView.a  libLLVMDebugInfoPDB.a  libLLVMSymbolize.a  libLLVMExecutionEngine.a  libLLVMInterpreter.a  libLLVMMCJIT.a  libLLVMOrcJIT.a  libLLVMRuntimeDyld.a  libLLVMTarget.a  libLLVMLineEditor.a  libLLVMProfileData.a  libLLVMCoverage.a  libLLVMPasses.a  libLLVMLibDriver.a  libLLVMXRay.a  libLLVMCoroutines.a  libLLVMLTO.a  libLLVMObjCARCOpts.a  libLLVMCoverage.a  libLLVMPasses.a  libclangStaticAnalyzerCheckers.a  libclangStaticAnalyzerCore.a  libclangASTMatchers.a  libclangFrontend.a  libclangDriver.a  libclangParse.a  libclangSerialization.a  libclangSema.a  libclangAnalysis.a  libclangEdit.a  libclangToolingCore.a  libclangAST.a  libclangRewrite.a  libclangLex.a  libclangBasic.a  libLLVMDebugInfoDWARF.a  libLLVMDebugInfoPDB.a  libLLVMExecutionEngine.a  libLLVMRuntimeDyld.a  libLLVMipo.a  libLLVMIRReader.a  libLLVMAsmParser.a  libLLVMInstrumentation.a  libLLVMLinker.a  libLLVMVectorize.a  libLLVMGlobalISel.a  libLLVMSelectionDAG.a  libLLVMAsmPrinter.a  libLLVMCodeGen.a  libLLVMBitWriter.a  libLLVMScalarOpts.a  libLLVMInstCombine.a  libLLVMDebugInfoCodeView.a  libLLVMDebugInfoMSF.a  libLLVMTransformUtils.a  libLLVMTarget.a  libLLVMAnalysis.a  libLLVMProfileData.a  libLLVMMCDisassembler.a  libLLVMObject.a  libLLVMBitReader.a  libLLVMCore.a  libLLVMMCParser.a  libLLVMMC.a  libLLVMOption.a  libLLVMSupport.a  libLLVMDemangle.a    

FULL_LIB_PATHS=$(addprefix $(CLANG_LIB_PATH), $(RQD_LIBS))

web: pp-step.html

pp-step.bc: ../pp-step.cpp
	em++ $(EMSCRIPEN_FLAGS) -I $(CLANG_INC_PATH)  $(CXXFLAGS)  -DSYSTEM_HEADERS=$(shell ../find_sys_includes.sh) -o $@ $<

pp-step.html: pp-step.bc
	em++ $(EMSCRIPTEN_FLAGS) $(CXXFLAGS) -s DEMANGLE_SUPPORT=1 -o $@ $< -Wl,-rpath,$(CLANG_LIB_PATH) $(FULL_LIB_PATHS) -lrt -ldl -lpthread -lm 
	cp pp-step.html.bkp pp-step.html


