#include "xccmeta/xccmeta_import.hpp"
#include <fstream>

namespace xccmeta {

  file::file(const path& path): file_path(path) {
  }

  const path& file::get_path() const {
    return file_path;
  }

  bool file::exists() const {
    return std::filesystem::exists(file_path);
  }

  std::string file::read() const {
    if (!exists()) {
      return "";
    }

    std::ifstream file_stream(file_path, std::ios::in | std::ios::binary);
    if (!file_stream) {
      return "";
    }

    std::string content((std::istreambuf_iterator<char>(file_stream)),
                        std::istreambuf_iterator<char>());
    return content;
  }

  bool file::write(const std::string& content) const {
    std::ofstream file_stream(file_path, std::ios::out | std::ios::binary);
    if (!file_stream) {
      return false;
    }

    file_stream.write(content.c_str(), content.size());
    return true;
  }

  importer::importer(const std::string& wildcard) {
    std::filesystem::path base_path = std::filesystem::path(wildcard).parent_path();
    std::string pattern = std::filesystem::path(wildcard).filename().string();

    for (const auto& entry : std::filesystem::directory_iterator(base_path)) {
      if (entry.is_regular_file()) {
        if (std::filesystem::path(entry.path()).filename().string() == pattern ||
            pattern == "*") {
          files.emplace_back(entry.path());
        }
      }
    }
  }

  const std::vector<file>& importer::get_files() const {
    return files;
  }

}  // namespace xccmeta