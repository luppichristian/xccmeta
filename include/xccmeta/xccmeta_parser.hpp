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

#include "xccmeta_compile_args.hpp"
#include "xccmeta_node.hpp"

namespace xccmeta {

  // The parser converts C/C++ source code into an AST (Abstract Syntax Tree).
  //
  // PREPROCESSOR HANDLING:
  // The parser internally uses libclang which performs full C/C++ preprocessing
  // before parsing. This means:
  //   - #define macros are expanded
  //   - #ifdef/#ifndef/#if/#elif/#else/#endif conditionals are evaluated
  //   - #include directives are processed
  //   - Macros defined via compile_args (e.g., args.define("FOO")) are respected
  //
  // Therefore, you do NOT need to preprocess the input before calling parse().
  // The preprocessor module (xccmeta_preprocess.hpp) is completely optional and
  // only useful if you need the preprocessed source text itself.
  //
  class XCCMETA_API parser {
   public:
    parser() = default;
    ~parser() = default;

    // Non-copyable
    parser(const parser&) = delete;
    parser& operator=(const parser&) = delete;

    // Move constructor
    parser(parser&&) noexcept = default;

    // Move assignment
    parser& operator=(parser&&) noexcept = default;

    // Parse input source code with given compile arguments
    std::shared_ptr<node> parse(const std::string& input, const compile_args& args);

    // Merge two AST nodes (e.g., from multiple translation units)
    std::shared_ptr<node> merge(std::shared_ptr<node> a, std::shared_ptr<node> b, const compile_args& args);
  };

}  // namespace xccmeta