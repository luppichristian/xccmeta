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

#include <xccmeta/xccmeta_warnings.hpp>

namespace xccmeta {

  compile_warnings& compile_warnings::push(const std::string& message, source_location loc) {
    warnings.push_back({message, loc});
    return *this;
  }

  std::string compile_warnings::build() const {
    if (warnings.empty()) {
      return "";
    }

    std::string result = {};
    result += "#ifdef _MSC_VER\n";
    for (const auto& msg : warnings) {
      if (msg.loc.is_valid()) {
        result += "#pragma message(\"Warning" + msg.loc.to_string() + ": " + msg.message + "\")\n";
      } else {
        result += "#pragma message(\"Warning: " + msg.message + "\")\n";
      }
    }
    result += "#else\n";
    for (const auto& msg : warnings) {
      if (msg.loc.is_valid()) {
        result += "#warning \"" + msg.loc.to_string() + ": " + msg.message + "\"\n";
      } else {
        result += "#warning \"" + msg.message + "\"\n";
      }
    }
    result += "#endif\n";
    return result;
  }

  const std::vector<compile_warnings::entry>& compile_warnings::get_warnings() const {
    return warnings;
  }

}  // namespace xccmeta