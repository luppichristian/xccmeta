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

#include "xccmeta/xccmeta_tags.hpp"

namespace xccmeta {

  // Helper function to trim whitespace from both ends of a string
  static std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\n\r\f\v");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\n\r\f\v");
    return str.substr(first, last - first + 1);
  }

  tag::tag(const std::string& name, const std::vector<std::string>& args): name(name), args(args) {
  }

  tag tag::parse(const std::string& to_parse) {
    tag result;
    // Parsing logic to extract name and args from to_parse
    // Expected format: xccmeta::tag_name(arg1, arg2, "string, with, commas")
    auto paren_pos = to_parse.find('(');
    if (paren_pos == std::string::npos) {
      result.name = trim(to_parse);
      return result;
    }

    result.name = trim(to_parse.substr(0, paren_pos));
    auto args_str = to_parse.substr(paren_pos + 1, to_parse.length() - paren_pos - 2);  // Exclude closing ')'

    // Split args_str by commas, respecting quoted strings
    size_t start = 0;
    bool in_string = false;
    char quote_char = '\0';

    for (size_t i = 0; i < args_str.length(); ++i) {
      char c = args_str[i];

      // Handle quote characters
      if ((c == '"' || c == '\'') && (i == 0 || args_str[i - 1] != '\\')) {
        if (!in_string) {
          in_string = true;
          quote_char = c;
        } else if (c == quote_char) {
          in_string = false;
          quote_char = '\0';
        }
      }

      // Split on comma only if not inside a string
      if (c == ',' && !in_string) {
        std::string arg = args_str.substr(start, i - start);
        result.args.push_back(trim(arg));
        start = i + 1;
      }
    }

    // Add the last argument
    if (start < args_str.length()) {
      std::string arg = args_str.substr(start);
      result.args.push_back(trim(arg));
    }

    return result;
  }

  const std::string& tag::get_name() const {
    return name;
  }

  const std::vector<std::string>& tag::get_args() const {
    return args;
  }

  std::string tag::get_args_combined() const {
    std::string combined;
    for (size_t i = 0; i < args.size(); ++i) {
      combined += args[i];
      if (i < args.size() - 1) {
        combined += ", ";
      }
    }
    return combined;
  }

  std::string tag::get_full() const {
    std::string full = name + "(" + get_args_combined() + ")";
    return full;
  }

}  // namespace xccmeta