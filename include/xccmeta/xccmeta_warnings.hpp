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

#include "xccmeta_base.hpp"
#include "xccmeta_source.hpp"

namespace xccmeta {

  // This is an utility to output custom compile warnings.
  // It generates a string that contains definitions to trigger
  // warnings when compiled with specific compilers.
  // On MSVC it uses #pragma message, on GCC/Clang it uses #warning.
  class XCCMETA_API compile_warnings {
   public:
    struct entry {
      std::string message;
      source_location loc;
    };

    compile_warnings() = default;

    compile_warnings& push(const std::string& message, source_location loc = {});
    std::string build() const;

    const std::vector<entry>& get_warnings() const;

   private:
    std::vector<entry> warnings;
  };

}  // namespace xccmeta