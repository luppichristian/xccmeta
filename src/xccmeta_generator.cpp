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

#include <xccmeta/xccmeta_generator.hpp>

#include <iostream>

namespace xccmeta {

  generator::generator(const std::string& output_file)
      : output_stream(output_file) {
    if (!output_stream.is_open()) {
      std::cerr << "Error: Could not open output file: " << output_file << std::endl;
    }
  }

  generator::~generator() {
    done();
  }

  generator& generator::out(const std::string& data) {
    if (output_stream.is_open()) {
      output_stream << data << '\n';
    }
    return *this;
  }

  generator& generator::separator() {
    if (output_stream.is_open()) {
      output_stream << "// ============================================================================\n";
    }
    return *this;
  }

  generator& generator::named_separator(const std::string& name) {
    separator();
    out("// === " + name);
    separator();
    return *this;
  }

  generator& generator::warn(const std::string& message, source_location loc) {
    warnings.push(message, loc);
    return *this;
  }

  generator& generator::warn(const std::string& message, const node_ptr node) {
    if (node) {
      warnings.push(message, node->get_location());
    } else {
      warnings.push(message, {});
    }
    return *this;
  }

  bool generator::done() {
    // Output any warnings to the generated file
    if (!warnings.get_warnings().empty()) {
      named_separator("Warnings");
      out(warnings.build());
    }

    // Close the output stream
    if (output_stream.is_open()) {
      output_stream.close();
      return !output_stream.fail();
    }

    return true;
  }

}  // namespace xccmeta