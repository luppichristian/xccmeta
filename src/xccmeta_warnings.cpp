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

}  // namespace xccmeta