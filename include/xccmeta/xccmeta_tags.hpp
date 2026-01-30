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

  // Represents a metadata tag extracted from source code
  // Tags can be defined in two ways: comment style or attribute style.
  // 1) Comment Style: Tags are embedded within comments using a specific syntax.
  //    ===============================================================================
  //    Example 1:
  //    /// @tag_name(arg1, arg2)
  //    struct MyStruct {};
  //    The tag is directly associated with the subsequent code element.
  //    ===============================================================================
  //    Example 2:
  //    They can also be defined in the same line:
  //    struct MyStruct {}; ///< @tag_name(arg1, arg2)
  //    ===============================================================================
  // 2) Attribute Style: Tags are defined using clang annotations.
  //    ===============================================================================
  //    Example:
  //    struct [[clang::annotate("example")]] MyStruct {};
  //    ===============================================================================
  // <!> NOTICE <!>: Comment style tags dont work for niche cases like template arguments or function parameters. (libclang limitation)
  // Its best to always tag the main declaration of a code element.
  // If you really need to tag such niche cases, use attribute style tags (but they are not guaranteed to be very portable).
  //    ===============================================================================
  // <!> WARNING <!>: Arguments should always be simple literals (e.g., numbers, strings) without nested structures.
  // Complex expressions or nested parentheses in arguments may lead to incorrect parsing.
  // They should be always specified as follows: tag_name(arg1, arg2, "string_arg", 42)
  // Named arguments are **NOT** supported: tag_name(arg1=value1, arg2=value2)
  //    ===============================================================================

  class XCCMETA_API tag {
   public:
    tag() = default;
    tag(const std::string& name, const std::vector<std::string>& args);

    static tag parse(const std::string& to_parse);  // must stricly be in this format: "tag(arg1, arg2)"

    std::string get_args_combined() const;             // Combined args as a single string, e.g., "arg1, arg2"
    std::string get_full() const;                      // Full representation excluding [[ and ]], e.g., xccmeta::tag_name(arg1, arg2)
    const std::string& get_name() const;               // e.g., xccmeta::tag_name
    const std::vector<std::string>& get_args() const;  // e.g., {arg1, arg2}

   private:
    std::string name;
    std::vector<std::string> args;
  };

}  // namespace xccmeta