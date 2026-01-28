#include <gtest/gtest.h>
#include <xccmeta_import.hpp>

#include <atomic>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>

namespace {

  // Counter to generate unique directory names for each test
  std::atomic<int> test_counter {0};

  // Helper class to create temporary test files and directories
  // Each instance creates a unique directory to avoid conflicts in parallel tests
  class TempTestEnvironment {
   public:
    TempTestEnvironment() {
      int id = test_counter.fetch_add(1);
      test_dir = std::filesystem::temp_directory_path() /
                 ("xccmeta_test_" + std::to_string(id) + "_" +
                  std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
      // Remove any leftover directory from previous runs (shouldn't happen with unique names)
      std::error_code ec;
      std::filesystem::remove_all(test_dir, ec);
      std::filesystem::create_directories(test_dir);
    }

    ~TempTestEnvironment() {
      std::error_code ec;
      std::filesystem::remove_all(test_dir, ec);
    }

    std::filesystem::path get_test_dir() const { return test_dir; }

    std::filesystem::path create_file(const std::string& name,
                                      const std::string& content = "") {
      auto file_path = test_dir / name;
      std::ofstream ofs(file_path, std::ios::binary);
      ofs << content;
      ofs.close();
      return file_path;
    }

    std::filesystem::path create_subdir(const std::string& name) {
      auto subdir_path = test_dir / name;
      std::filesystem::create_directories(subdir_path);
      return subdir_path;
    }

   private:
    std::filesystem::path test_dir;
  };

}  // namespace

// =============================================================================
// file class tests
// =============================================================================

// Test file constructor and get_path
TEST(FileTest, ConstructorAndGetPath) {
  std::filesystem::path test_path = "test/path/file.txt";
  xccmeta::file f(test_path);

  EXPECT_EQ(f.get_path(), test_path);
}

// Test file constructor with absolute path
TEST(FileTest, ConstructorWithAbsolutePath) {
  std::filesystem::path test_path = std::filesystem::temp_directory_path() / "test.txt";
  xccmeta::file f(test_path);

  EXPECT_EQ(f.get_path(), test_path);
}

// Test exists() returns false for non-existent file
TEST(FileTest, ExistsReturnsFalseForNonExistentFile) {
  xccmeta::file f("non_existent_file_12345.txt");

  EXPECT_FALSE(f.exists());
}

// Test exists() returns true for existing file
TEST(FileTest, ExistsReturnsTrueForExistingFile) {
  TempTestEnvironment env;
  auto file_path = env.create_file("existing_file.txt");

  xccmeta::file f(file_path);

  EXPECT_TRUE(f.exists());
}

// Test read() returns empty string for non-existent file
TEST(FileTest, ReadReturnsEmptyStringForNonExistentFile) {
  xccmeta::file f("non_existent_file_12345.txt");

  EXPECT_EQ(f.read(), "");
}

// Test read() returns file content for existing file
TEST(FileTest, ReadReturnsContentForExistingFile) {
  TempTestEnvironment env;
  std::string expected_content = "Hello, World!";
  auto file_path = env.create_file("readable_file.txt", expected_content);

  xccmeta::file f(file_path);

  EXPECT_EQ(f.read(), expected_content);
}

// Test read() handles empty file
TEST(FileTest, ReadHandlesEmptyFile) {
  TempTestEnvironment env;
  auto file_path = env.create_file("empty_file.txt", "");

  xccmeta::file f(file_path);

  EXPECT_EQ(f.read(), "");
}

// Test read() handles multiline content
TEST(FileTest, ReadHandlesMultilineContent) {
  TempTestEnvironment env;
  std::string expected_content = "Line 1\nLine 2\nLine 3\n";
  auto file_path = env.create_file("multiline_file.txt", expected_content);

  xccmeta::file f(file_path);

  EXPECT_EQ(f.read(), expected_content);
}

// Test read() handles binary content
TEST(FileTest, ReadHandlesBinaryContent) {
  TempTestEnvironment env;
  std::string binary_content = "test\x00\x01\x02\x03binary";
  binary_content.resize(17);  // Include null byte
  auto file_path = env.create_file("binary_file.bin", binary_content);

  xccmeta::file f(file_path);

  EXPECT_EQ(f.read(), binary_content);
}

// Test write() creates new file with content
TEST(FileTest, WriteCreatesNewFileWithContent) {
  TempTestEnvironment env;
  auto file_path = env.get_test_dir() / "new_write_file.txt";
  std::string content = "Written content";

  xccmeta::file f(file_path);
  bool result = f.write(content);

  EXPECT_TRUE(result);
  EXPECT_TRUE(std::filesystem::exists(file_path));

  // Verify content
  std::ifstream ifs(file_path, std::ios::binary);
  std::string actual_content((std::istreambuf_iterator<char>(ifs)),
                             std::istreambuf_iterator<char>());
  EXPECT_EQ(actual_content, content);
}

// Test write() overwrites existing file
TEST(FileTest, WriteOverwritesExistingFile) {
  TempTestEnvironment env;
  auto file_path = env.create_file("overwrite_file.txt", "Original content");
  std::string new_content = "New content";

  xccmeta::file f(file_path);
  bool result = f.write(new_content);

  EXPECT_TRUE(result);

  // Verify content
  std::ifstream ifs(file_path, std::ios::binary);
  std::string actual_content((std::istreambuf_iterator<char>(ifs)),
                             std::istreambuf_iterator<char>());
  EXPECT_EQ(actual_content, new_content);
}

// Test write() handles empty content
TEST(FileTest, WriteHandlesEmptyContent) {
  TempTestEnvironment env;
  auto file_path = env.get_test_dir() / "empty_write_file.txt";

  xccmeta::file f(file_path);
  bool result = f.write("");

  EXPECT_TRUE(result);
  EXPECT_TRUE(std::filesystem::exists(file_path));
  EXPECT_EQ(std::filesystem::file_size(file_path), 0);
}

// Test write() handles multiline content
TEST(FileTest, WriteHandlesMultilineContent) {
  TempTestEnvironment env;
  auto file_path = env.get_test_dir() / "multiline_write_file.txt";
  std::string content = "Line 1\nLine 2\nLine 3\n";

  xccmeta::file f(file_path);
  bool result = f.write(content);

  EXPECT_TRUE(result);

  // Verify content
  std::ifstream ifs(file_path, std::ios::binary);
  std::string actual_content((std::istreambuf_iterator<char>(ifs)),
                             std::istreambuf_iterator<char>());
  EXPECT_EQ(actual_content, content);
}

// Test round-trip: write then read
TEST(FileTest, WriteAndReadRoundTrip) {
  TempTestEnvironment env;
  auto file_path = env.get_test_dir() / "roundtrip_file.txt";
  std::string content = "Round-trip test content with special chars: \t\n\r";

  xccmeta::file f(file_path);
  EXPECT_TRUE(f.write(content));
  EXPECT_EQ(f.read(), content);
}

// Test file path with special characters
TEST(FileTest, PathWithSpecialCharacters) {
  TempTestEnvironment env;
  auto file_path = env.get_test_dir() / "file with spaces.txt";
  std::string content = "Content in file with spaces";

  xccmeta::file f(file_path);
  EXPECT_TRUE(f.write(content));
  EXPECT_TRUE(f.exists());
  EXPECT_EQ(f.read(), content);
}

// =============================================================================
// importer class tests
// =============================================================================

// Test default constructor creates empty importer
TEST(ImporterTest, DefaultConstructorCreatesEmptyImporter) {
  xccmeta::importer imp;

  EXPECT_TRUE(imp.get_files().empty());
}

// Test importer with non-existent directory
TEST(ImporterTest, NonExistentDirectoryThrowsOrEmpty) {
  // The implementation iterates over directory_iterator which may throw
  // or result in empty files vector depending on implementation
  try {
    xccmeta::importer imp("/non/existent/path/*");
    // If no exception, files should be empty or implementation handles it
    EXPECT_TRUE(imp.get_files().empty());
  } catch (const std::filesystem::filesystem_error&) {
    // Expected behavior - directory doesn't exist
    SUCCEED();
  }
}

// Test importer with wildcard "*" imports all files
TEST(ImporterTest, WildcardImportsAllFiles) {
  TempTestEnvironment env;
  env.create_file("file1.txt", "content1");
  env.create_file("file2.txt", "content2");
  env.create_file("file3.cpp", "content3");

  std::string wildcard = (env.get_test_dir() / "*").string();
  xccmeta::importer imp(wildcard);

  EXPECT_EQ(imp.get_files().size(), 3);
}

// Test importer with specific filename imports single file
TEST(ImporterTest, SpecificFilenameImportsSingleFile) {
  TempTestEnvironment env;
  env.create_file("target.txt", "target content");
  env.create_file("other.txt", "other content");

  std::string path = (env.get_test_dir() / "target.txt").string();
  xccmeta::importer imp(path);

  EXPECT_EQ(imp.get_files().size(), 1);
  EXPECT_EQ(imp.get_files()[0].get_path().filename(), "target.txt");
}

// Test importer ignores subdirectories
TEST(ImporterTest, IgnoresSubdirectories) {
  TempTestEnvironment env;
  env.create_file("file.txt", "content");
  env.create_subdir("subdir");

  std::string wildcard = (env.get_test_dir() / "*").string();
  xccmeta::importer imp(wildcard);

  EXPECT_EQ(imp.get_files().size(), 1);
  EXPECT_EQ(imp.get_files()[0].get_path().filename(), "file.txt");
}

// Test importer with empty directory
TEST(ImporterTest, EmptyDirectoryReturnsNoFiles) {
  TempTestEnvironment env;
  auto empty_subdir = env.create_subdir("empty");

  std::string wildcard = (empty_subdir / "*").string();
  xccmeta::importer imp(wildcard);

  EXPECT_TRUE(imp.get_files().empty());
}

// Test imported files are readable
TEST(ImporterTest, ImportedFilesAreReadable) {
  TempTestEnvironment env;
  std::string content1 = "Content of file 1";
  std::string content2 = "Content of file 2";
  env.create_file("file1.txt", content1);
  env.create_file("file2.txt", content2);

  std::string wildcard = (env.get_test_dir() / "*").string();
  xccmeta::importer imp(wildcard);

  EXPECT_EQ(imp.get_files().size(), 2);

  // Check that all imported files exist and are readable
  for (const auto& f : imp.get_files()) {
    EXPECT_TRUE(f.exists());
    std::string content = f.read();
    EXPECT_FALSE(content.empty());
  }
}

// Test importer with specific file that doesn't exist
TEST(ImporterTest, SpecificNonExistentFileReturnsEmpty) {
  TempTestEnvironment env;
  env.create_file("other.txt", "content");

  std::string path = (env.get_test_dir() / "nonexistent.txt").string();
  xccmeta::importer imp(path);

  EXPECT_TRUE(imp.get_files().empty());
}

// Test importer get_files returns const reference
TEST(ImporterTest, GetFilesReturnsConstReference) {
  TempTestEnvironment env;
  env.create_file("file.txt", "content");

  std::string wildcard = (env.get_test_dir() / "*").string();
  xccmeta::importer imp(wildcard);

  const std::vector<xccmeta::file>& files1 = imp.get_files();
  const std::vector<xccmeta::file>& files2 = imp.get_files();

  // Both references should point to the same vector
  EXPECT_EQ(&files1, &files2);
}

// Test importer with multiple files of same type
TEST(ImporterTest, MultipleFilesOfSameType) {
  TempTestEnvironment env;
  for (int i = 0; i < 5; ++i) {
    env.create_file("file" + std::to_string(i) + ".txt", "content" + std::to_string(i));
  }

  std::string wildcard = (env.get_test_dir() / "*").string();
  xccmeta::importer imp(wildcard);

  EXPECT_EQ(imp.get_files().size(), 5);
}

// Test importer with path containing spaces
TEST(ImporterTest, PathWithSpaces) {
  TempTestEnvironment env;
  auto subdir = env.create_subdir("dir with spaces");
  auto file_path = subdir / "file.txt";
  std::ofstream(file_path) << "content";

  std::string wildcard = (subdir / "*").string();
  xccmeta::importer imp(wildcard);

  EXPECT_EQ(imp.get_files().size(), 1);
}

// =============================================================================
// Integration tests
// =============================================================================

// Test importing and modifying files
TEST(IntegrationTest, ImportAndModifyFiles) {
  TempTestEnvironment env;
  env.create_file("config.txt", "original");

  std::string path = (env.get_test_dir() / "config.txt").string();
  xccmeta::importer imp(path);

  ASSERT_EQ(imp.get_files().size(), 1);

  const xccmeta::file& f = imp.get_files()[0];
  EXPECT_EQ(f.read(), "original");

  // Modify the file
  f.write("modified");
  EXPECT_EQ(f.read(), "modified");
}

// Test importing multiple files and reading their contents
TEST(IntegrationTest, ImportMultipleAndReadAll) {
  TempTestEnvironment env;
  std::vector<std::string> contents = {"alpha", "beta", "gamma"};
  for (size_t i = 0; i < contents.size(); ++i) {
    env.create_file("file" + std::to_string(i) + ".txt", contents[i]);
  }

  std::string wildcard = (env.get_test_dir() / "*").string();
  xccmeta::importer imp(wildcard);

  ASSERT_EQ(imp.get_files().size(), 3);

  // Collect all content
  std::vector<std::string> read_contents;
  for (const auto& f : imp.get_files()) {
    read_contents.push_back(f.read());
  }

  // Sort both vectors and compare (order may differ)
  std::sort(contents.begin(), contents.end());
  std::sort(read_contents.begin(), read_contents.end());
  EXPECT_EQ(read_contents, contents);
}
