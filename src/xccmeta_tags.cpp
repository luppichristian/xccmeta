#include "xccmeta_tags.hpp"

namespace xccmeta {

  tag::tag() {
  }

  tag::tag(const std::string& name, const std::vector<std::string>& args): name(name), args(args) {
  }

  tag::tag(const std::string& to_parse) {
    // Parsing logic to extract name and args from to_parse
    // Expected format: xccmeta::tag_name(arg1, arg2)
    auto paren_pos = to_parse.find('(');
    if (paren_pos == std::string::npos) {
      name = to_parse;
      return;
    }

    name = to_parse.substr(0, paren_pos);
    auto args_str = to_parse.substr(paren_pos + 1, to_parse.length() - paren_pos - 2);  // Exclude closing ')'

    // Split args_str by commas
    size_t start = 0;
    size_t end = args_str.find(',');
    while (end != std::string::npos) {
      args.push_back(args_str.substr(start, end - start));
      start = end + 1;
      end = args_str.find(',', start);
    }
    // Add the last argument
    if (start < args_str.length()) {
      args.push_back(args_str.substr(start));
    }
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