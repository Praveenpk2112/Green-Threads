cmake_minimum_required(VERSION 3.10)
project(GreenInjector)

# Find the LLVM and Clang libraries required to build a compiler tool
find_package(LLVM REQUIRED CONFIG)
include_directories(${LLVM_INCLUDE_DIRS})
link_directories(${LLVM_LIBRARY_DIRS})
add_definitions(${LLVM_DEFINITIONS})

find_package(Clang REQUIRED CONFIG)
include_directories(${CLANG_INCLUDE_DIRS})

# Create our executable
add_executable(loop_injector loop_injector.cpp)

# Link all the massive Clang/LLVM parsing libraries
target_link_libraries(loop_injector
    clangTooling
    clangBasic
    clangASTMatchers
    clangFormat
    clangFrontend
    clangLex
    clangParse
    clangSema
    clangAST
    clangRewrite
    LLVMSupport
)