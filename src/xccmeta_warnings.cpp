#include <xccmeta/xccmeta_warnings.hpp>

namespace xccmeta {

  compile_warnings& compile_warnings::push(const std::string& message) {
    warnings.push_back(message);
    return *this;
  }

  std::string compile_warnings::build() const {
    if (warnings.empty()) {
      return "";
    }

    std::string result;
    result += "#ifdef _MSC_VER\n";
    for (const auto& msg : warnings) {
      result += "#pragma message(\"Warning: " + msg + "\")\n";
    }
    result += "#else\n";
    for (const auto& msg : warnings) {
      result += "#warning \"" + msg + "\"\n";
    }
    result += "#endif\n";
    return result;
  }

}  // namespace xccmeta