#include <gtest/gtest.h>
#include <xccmeta.hpp>

// Example test fixture
class XccmetaTest : public ::testing::Test {
 protected:
  void SetUp() override {
    // Setup code here
  }

  void TearDown() override {
    // Cleanup code here
  }
};

// Example test case
TEST_F(XccmetaTest, BasicTest) {
  EXPECT_TRUE(true);
}