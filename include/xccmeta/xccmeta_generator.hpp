#pragma once

#include <fstream>
#include "xccmeta_node.hpp"
#include "xccmeta_warnings.hpp"

namespace xccmeta {

  // Generator for XCCMeta metadata
  class XCCMETA_API generator {
   public:
    generator(const std::string& output_file);
    ~generator();

    // Output a line of data (with newline)
    generator& out(const std::string& data);

    // Manage indentation
    generator& indent();
    generator& unindent();

    // Output a separator line
    generator& separator();

    // Output a named separator line
    generator& named_separator(const std::string& name);

    // Issue a custom warning message
    generator& warn(const std::string& message, source_location loc = {});
    generator& warn(const std::string& message, const node_ptr node);

    // Finalize generation and close the output file
    bool done();

   private:
    compile_warnings warnings;
    std::ofstream output_stream;
  };

}  // namespace xccmeta