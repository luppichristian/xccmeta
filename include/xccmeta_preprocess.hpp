#pragma once

#include "xccmeta_import.hpp"

namespace xccmeta {

  // This class holds any state needed during preprocessing.
  //
  class XCCMETA_API preprocessor_context {
   public:
    preprocessor_context() = default;
    preprocessor_context(const std::string& input);

    std::string apply(const std::string& to_preprocess) const;

   private:
    struct internal_data;
    std::unique_ptr<internal_data> data;
  };

  // Preprocessor for files, it operates in two steps:
  // 1. Produces a "preprocessor_context" from the input files. This context holds
  //    any state needed for preprocessing (e.g., macros, includes, etc).
  // 2. Uses the context to produce preprocessed content for each file.
  class XCCMETA_API preprocessor {
   public:
    explicit preprocessor(const file& file);
    explicit preprocessor(const std::vector<file>& files);

    const std::vector<std::string>& get_preprocessed_content() const;
    const preprocessor_context& get_context() const;

   private:
    std::vector<std::string> content;
    preprocessor_context context;
  };

}  // namespace xccmeta