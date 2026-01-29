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
#include "xccmeta_import.hpp"

namespace xccmeta {

  // ============================================================================
  // IMPORTANT: This module is COMPLETELY OPTIONAL for most use cases.
  //
  // The parser (xccmeta_parser.hpp) internally uses libclang which already
  // performs full C/C++ preprocessing before parsing. You do NOT need to
  // preprocess input before calling parser::parse().
  //
  // This module is only useful when you need:
  //   - The preprocessed source text itself (e.g., for display or debugging)
  //   - To inspect macro expansions
  //   - To share preprocessing context across multiple files
  // ============================================================================

  // This class holds any state needed during preprocessing.
  //
  class XCCMETA_API preprocessor_context {
   public:
    preprocessor_context();
    ~preprocessor_context();
    preprocessor_context(const std::string& input, const compile_args& args = compile_args());

    // Move operations (needed for Pimpl with unique_ptr)
    preprocessor_context(preprocessor_context&&) noexcept;
    preprocessor_context& operator=(preprocessor_context&&) noexcept;

    // Delete copy operations
    preprocessor_context(const preprocessor_context&) = delete;
    preprocessor_context& operator=(const preprocessor_context&) = delete;

    std::string apply(const std::string& to_preprocess, const compile_args& args = compile_args()) const;

   private:
    struct internal_data;
    std::unique_ptr<internal_data> data;
  };

  // Preprocessor for files, it operates in two steps:
  // 1. Produces a "preprocessor_context" from the input files. This context holds
  //    any state needed for preprocessing (e.g., macros, includes, etc).
  // 2. Uses the context to produce preprocessed content for each file.
  class XCCMETA_API preprocessor {
   public:
    explicit preprocessor(const file& file, const compile_args& args = compile_args());
    explicit preprocessor(const std::vector<file>& files, const compile_args& args = compile_args());

    const std::vector<std::string>& get_preprocessed_content() const;
    const preprocessor_context& get_context() const;

   private:
    std::vector<std::string> content;
    preprocessor_context context;
  };

}  // namespace xccmeta