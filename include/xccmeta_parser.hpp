#pragma once

#include "xccmeta_compile_args.hpp"
#include "xccmeta_node.hpp"

namespace xccmeta {

  class XCCMETA_API parser {
   public:
    parser() = default;
    ~parser() = default;

    // Non-copyable
    parser(const parser&) = delete;
    parser& operator=(const parser&) = delete;

    // Move constructor
    parser(parser&&) noexcept = default;

    // Move assignment
    parser& operator=(parser&&) noexcept = default;

    // Parse input source code with given compile arguments
    std::shared_ptr<node> parse(const std::string& input, const compile_args& args);

    // Merge two AST nodes (e.g., from multiple translation units)
    std::shared_ptr<node> merge(std::shared_ptr<node> a, std::shared_ptr<node> b, const compile_args& args);
  };

}  // namespace xccmeta