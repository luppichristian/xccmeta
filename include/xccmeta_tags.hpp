#pragma once

#include <string>
#include <vector>
#include "xccmeta_base.hpp"

namespace xccmeta {

  // Represents a metadata tag extracted from source code
  // e.g., [[xccmeta::tag_name(arg1, arg2)]]
  // C/C++ allows for this kind of attribute syntax
  class XCCMETA_API tag {
   public:
    tag() = default;
    tag(const std::string& name, const std::vector<std::string>& args);

    static tag parse(const std::string& to_parse);  // Excluding [[ and ]]

    std::string get_args_combined() const;             // Combined args as a single string, e.g., "arg1, arg2"
    std::string get_full() const;                      // Full representation excluding [[ and ]], e.g., xccmeta::tag_name(arg1, arg2)
    const std::string& get_name() const;               // e.g., xccmeta::tag_name
    const std::vector<std::string>& get_args() const;  // e.g., {arg1, arg2}

   private:
    std::string name;
    std::vector<std::string> args;
  };

}  // namespace xccmeta