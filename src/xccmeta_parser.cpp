#include "xccmeta_parser.hpp"

namespace xccmeta {

  std::shared_ptr<node> parser::parse(const std::string& input, const compile_args& args) {
    return node::create(node::kind::translation_unit);
  }

}  // namespace xccmeta