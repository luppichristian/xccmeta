#pragma once

#include "xccmeta_base.hpp"

#include <filesystem>
#include <string>
#include <vector>

namespace xccmeta {

  using path = std::filesystem::path;

  // File class to represent a single file.
  // Its just a wrapper around std::filesystem::path.
  class XCCMETA_API file {
   public:
    explicit file(const path& path);
    const path& get_path() const;

    bool exists() const;
    std::string read() const;
    bool write(const std::string& content) const;

   private:
    path file_path;
  };

  // Importer class to import multiple files based on a wildcard.
  // By passing in a direct filepath, it will import that single file.
  class XCCMETA_API importer {
   public:
    explicit importer(const std::string& wildcard);
    const std::vector<file>& get_files() const;

   private:
    std::vector<file> files;
  };

}  // namespace xccmeta