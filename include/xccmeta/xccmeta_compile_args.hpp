/*
MIT License

Copyright (c) 2026 Christian Luppi

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once

#include <string>
#include <vector>

#include "xccmeta_base.hpp"

namespace xccmeta {

  // Language standards supported by the preprocessor
  enum class language_standard {
    c89,
    c99,
    c11,
    c17,
    c23,
    cxx98,
    cxx03,
    cxx11,
    cxx14,
    cxx17,
    cxx20,
    cxx23,
    cxx26
  };

  // Language modes
  enum class language_mode {
    c,
    cxx,
    objective_c,
    objective_cxx
  };

  class XCCMETA_API compile_args {
   public:
    compile_args();

    // Basic argument manipulation
    void add(const std::string& arg);
    void add_many(const std::vector<std::string>& args_to_add);
    void clear();
    const std::vector<std::string>& get_args() const;

    // ===== Language Standard and Mode =====

    // Set language standard (e.g., c++17, c++20, c11, etc.)
    compile_args& set_standard(language_standard std);

    // Set language mode (C, C++, Objective-C, etc.)
    compile_args& set_language(language_mode lang);

    // ===== Include Paths =====

    // Add an include path (-I<path>)
    compile_args& add_include_path(const std::string& path);

    // Add multiple include paths
    compile_args& add_include_paths(const std::vector<std::string>& paths);

    // ===== Preprocessor Definitions =====

    // Define a macro (-D<name>)
    compile_args& define(const std::string& name);

    // Define a macro with value (-D<name>=<value>)
    compile_args& define(const std::string& name, const std::string& value);

    // Define a macro with integer value
    compile_args& define(const std::string& name, int value);

    // Undefine a macro (-U<name>)
    compile_args& undefine(const std::string& name);

    // ===== Target Configuration =====

    // Set target triple (--target=<triple>) - affects sizeof, predefined macros
    compile_args& set_target(const std::string& triple);

    // Set pointer size for cross-compilation (-m32 or -m64)
    compile_args& set_pointer_size(int bits);

    // ===== Common Presets =====

    // Configure for modern C++ development
    static compile_args modern_cxx(language_standard std = language_standard::cxx20);

    // Configure for C development
    static compile_args modern_c(language_standard std = language_standard::c17);

    // Configure minimal args (no defaults)
    static compile_args minimal();

   private:
    std::vector<std::string> args;

    // Helper to convert enums to strings
    static const char* standard_to_string(language_standard std);
    static const char* language_to_string(language_mode lang);
  };

}  // namespace xccmeta