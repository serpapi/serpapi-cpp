// Tests against the live SerpApi service.
#include "../src/serpapi.hpp"
#include <gtest/gtest.h>
#include <cstdlib>
#include <string>

// Skips the test when SERPAPI_KEY is unset or empty; otherwise provides a
// Google client with that key.
class LiveApi : public ::testing::Test {
protected:
  void SetUp() override {
    const char *key = std::getenv("SERPAPI_KEY");
    if (key == nullptr || *key == '\0') {
      GTEST_SKIP() << "SERPAPI_KEY not set";
    }
    api_key_ = key;
  }

  serpapi::Client google() const { return serpapi::Client({{"api_key", api_key_}, {"engine", "google"}}); }

  std::string api_key_;
};

TEST_F(LiveApi, search) {
  rapidjson::Document d = google().search({{"q", "coffee"}, {"location", "Austin,TX"}});

  ASSERT_TRUE(d.HasMember("search_metadata"));
  ASSERT_EQ(d["search_metadata"]["status"], "Success");
}

// No api_key needed, so this test always runs.
TEST(client, location) {
  serpapi::Client client;
  rapidjson::Document doc = client.location({{"q", "Austin"}, {"limit", "3"}});
  ASSERT_TRUE(doc.IsArray());
  ASSERT_GT(doc.GetArray().Size(), 1u);
}

TEST_F(LiveApi, account) {
  rapidjson::Document d = serpapi::Client({{"api_key", api_key_}}).account();
  ASSERT_TRUE(d.HasMember("account_id"));
}

TEST_F(LiveApi, search_archive) {
  serpapi::Client client = google();
  rapidjson::Document d = client.search({{"q", "coffee"}, {"location", "Austin,TX"}});

  ASSERT_TRUE(d.HasMember("search_metadata"));
  ASSERT_TRUE(d["search_metadata"].HasMember("id"));
  ASSERT_TRUE(d["search_metadata"]["id"].IsString());
  ASSERT_EQ(d["search_metadata"]["status"], "Success");
  const std::string id = d["search_metadata"]["id"].GetString();

  rapidjson::Document archived = client.search_archive(id);
  ASSERT_TRUE(archived.HasMember("search_metadata"));
  ASSERT_TRUE(archived["search_metadata"].HasMember("id"));
  EXPECT_EQ(std::string(archived["search_metadata"]["id"].GetString()), id);
}

TEST_F(LiveApi, search_archive_md) {
  serpapi::Client client = google();
  rapidjson::Document d = client.search({{"q", "coffee"}, {"location", "Austin,TX"}});
  ASSERT_TRUE(d.HasMember("search_metadata"));
  const std::string id = d["search_metadata"]["id"].GetString();

  std::string markdown = client.search_archive_md(id);
  ASSERT_FALSE(markdown.empty());
  EXPECT_NE(markdown.find(id), std::string::npos);
}

TEST_F(LiveApi, html) {
  std::string html = google().html({{"q", "coffee"}});
  ASSERT_FALSE(html.empty());
  EXPECT_NE(html.find("coffee"), std::string::npos);
}

TEST_F(LiveApi, markdown) {
  std::string markdown = google().markdown({{"q", "coffee"}});
  ASSERT_FALSE(markdown.empty());
  EXPECT_NE(markdown.find("coffee"), std::string::npos);
}

// A search with no results returns HTTP 200 with an error in the body; it must
// still throw, independent of the HTTP status.
static const char *kNoResultsQuery = "site:zxqvjkwplmrtnbhgf.invalid qqzzxxkkvvwwjjhh";

TEST_F(LiveApi, search_no_results_throws_with_http_200) {
  try {
    google().search({{"q", kNoResultsQuery}});
    FAIL() << "expected serpapi::Error";
  } catch (const serpapi::Error &e) {
    EXPECT_EQ(e.http_code(), 200);
    EXPECT_EQ(e.cause(), "Google hasn't returned any results for this query.");
    EXPECT_NE(e.payload().find("search_metadata"), std::string::npos);
  }
}

TEST_F(LiveApi, markdown_no_results_throws_with_http_200) {
  try {
    google().markdown({{"q", kNoResultsQuery}});
    FAIL() << "expected serpapi::Error";
  } catch (const serpapi::Error &e) {
    EXPECT_EQ(e.http_code(), 200);
    EXPECT_EQ(e.cause(), "Google hasn't returned any results for this query.");
  }
}
