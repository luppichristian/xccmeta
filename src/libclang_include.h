#pragma once

// Suppress MSVC warning C4291 from LLVM/Clang headers
#ifdef _MSC_VER
#  pragma warning(push)
#  pragma warning(disable : 4291)
#endif

// libclang C API for AST parsing
#include <clang-c/Index.h>

// Clang C++ API for preprocessing
#include <clang/Basic/Diagnostic.h>
#include <clang/Basic/DiagnosticOptions.h>
#include <clang/Basic/FileManager.h>
#include <clang/Basic/LangOptions.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Basic/TargetInfo.h>
#include <clang/Basic/TargetOptions.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/TextDiagnosticPrinter.h>
#include <clang/Lex/HeaderSearch.h>
#include <clang/Lex/HeaderSearchOptions.h>
#include <clang/Lex/ModuleLoader.h>
#include <clang/Lex/Preprocessor.h>
#include <clang/Lex/PreprocessorOptions.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/TargetParser/Host.h>

#ifdef _MSC_VER
#  pragma warning(pop)
#endif
