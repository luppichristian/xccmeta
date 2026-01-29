#pragma once

#include <memory>
#include <vector>
#include "xccmeta_base.hpp"
#include "xccmeta_tags.hpp"

namespace xccmeta {

  class node {
   public:
    enum class type {
      unknown,
      root,
      // TODO: ...
    };

   private:
    type node_type;
    std::vector<std::shared_ptr<node>> children;
    std::shared_ptr<node> parent;
    std::vector<tag> tags;
  };

}  // namespace xccmeta