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

#include <fstream>
#include "xccmeta_node.hpp"
#include "xccmeta_warnings.hpp"

namespace xccmeta {

  // Generator for XCCMeta metadata
  class XCCMETA_API generator {
   public:
    generator(const std::string& output_file);
    ~generator();

    // Output a line of data (with newline)
    generator& out(const std::string& data);

    // Manage indentation
    generator& indent();
    generator& unindent();

    // Output a separator line
    generator& separator();

    // Output a named separator line
    generator& named_separator(const std::string& name);

    // Issue a custom warning message
    generator& warn(const std::string& message, source_location loc = {});
    generator& warn(const std::string& message, const node_ptr node);

    // Finalize generation and close the output file
    bool done();

   private:
    compile_warnings warnings;
    std::ofstream output_stream;
    int indent_level = 0;
  };

}  // namespace xccmeta