#include "xccmeta_parser.hpp"

namespace xccmeta {

  std::shared_ptr<node> parser::parse(const std::string& input, const compile_args& args) {
    return std::make_shared<node>();
  }

}  // namespace xccmeta