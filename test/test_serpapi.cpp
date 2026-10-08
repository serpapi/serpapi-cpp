#include "../src/callback.hpp"
#include "../src/serpapi.hpp"
#include <gtest/gtest.h>

using namespace std;

TEST(client, search) {
  const char *env_p = std::getenv("SERPAPI_KEY");
  if (env_p == nullptr || *env_p == '\0') {
    GTEST_SKIP() << "SERPAPI_KEY not set";
  }
  std::string apiKey(env_p);
  std::map<string, string> default_parameter;
  default_parameter["api_key"] = apiKey;
  default_parameter["engine"] = "google";

  // using namespace serpapi;
  serpapi::Client client(default_parameter);

  // execute search
  map<string, string> parameter;
  parameter["q"] = "coffee";
  parameter["location"] = "Austin,TX";

  rapidjson::Document d = client.search(parameter);

  ASSERT_FALSE(d.HasMember("error"));
  ASSERT_TRUE(d.HasMember("search_metadata"));
  ASSERT_EQ(d["search_metadata"]["status"], "Success");
}

TEST(client, location) {
  std::map<string, string> default_parameter = {};
  serpapi::Client client(default_parameter);

  map<string, string> parameter;
  parameter["limit"] = "3";
  parameter["q"] = "Austin";
  rapidjson::Document doc = client.location(parameter);
  ASSERT_TRUE(doc.IsArray());
  ASSERT_GT(doc.GetArray().Size(), 1u);
}

// fetch account information
TEST(client, account) {
  const char *env_p = std::getenv("SERPAPI_KEY");
  if (env_p == nullptr || *env_p == '\0') {
    GTEST_SKIP() << "SERPAPI_KEY not set";
  }
  std::string apiKey(env_p);
  std::map<string, string> default_parameter;
  default_parameter["api_key"] = apiKey;
  serpapi::Client client(default_parameter);

  map<string, string> parameter = {};
  rapidjson::Document d = client.account(parameter);

  ASSERT_FALSE(d.HasMember("error"));
  ASSERT_TRUE(d.HasMember("account_id"));
}

TEST(client, search_archive) {
  const char *env_p = std::getenv("SERPAPI_KEY");
  if (env_p == nullptr || *env_p == '\0') {
    GTEST_SKIP() << "SERPAPI_KEY not set";
  }
  std::string apiKey(env_p);
  std::map<string, string> default_parameter;
  default_parameter["api_key"] = apiKey;
  default_parameter["engine"] = "google";

  // using namespace serpapi;
  serpapi::Client client(default_parameter);

  // execute search
  map<string, string> parameter;
  parameter["q"] = "coffee";
  parameter["location"] = "Austin,TX";

  rapidjson::Document d = client.search(parameter);

  ASSERT_FALSE(d.HasMember("error"));
  ASSERT_TRUE(d.HasMember("search_metadata"));
  ASSERT_TRUE(d["search_metadata"].HasMember("id"));
  ASSERT_EQ(d["search_metadata"]["status"], "Success");

  ASSERT_TRUE(d["search_metadata"]["id"].IsString());

  std::string id = d["search_metadata"]["id"].GetString();
  rapidjson::Document d2 = client.search_archive(id);

  ASSERT_FALSE(d2.HasMember("error"));
  ASSERT_TRUE(d2.HasMember("search_metadata"));
  ASSERT_TRUE(d2["search_metadata"].HasMember("id"));
  std::string id_ = d2["search_metadata"]["id"].GetString();
  ASSERT_EQ(id_, id);
}

TEST(client, html) {
  const char *env_p = std::getenv("SERPAPI_KEY");
  if (env_p == nullptr || *env_p == '\0') {
    GTEST_SKIP() << "SERPAPI_KEY not set";
  }
  std::string apiKey(env_p);
  std::map<string, string> default_parameter;
  default_parameter["api_key"] = apiKey;
  default_parameter["engine"] = "google";

  serpapi::Client client(default_parameter);

  map<string, string> parameter;
  parameter["q"] = "coffee";

  std::string html = client.html(parameter);
  ASSERT_FALSE(html.empty());
  ASSERT_TRUE(html.find("coffee") != std::string::npos);
}

TEST(client, markdown) {
  const char *env_p = std::getenv("SERPAPI_KEY");
  if (env_p == nullptr || *env_p == '\0') {
    GTEST_SKIP() << "SERPAPI_KEY not set";
  }
  std::string apiKey(env_p);
  std::map<string, string> default_parameter;
  default_parameter["api_key"] = apiKey;
  default_parameter["engine"] = "google";

  serpapi::Client client(default_parameter);

  map<string, string> parameter;
  parameter["q"] = "coffee";

  std::string markdown = client.markdown(parameter);
  ASSERT_FALSE(markdown.empty());
  ASSERT_TRUE(markdown.find("coffee") != std::string::npos);
}

// A search with no results returns HTTP 200 with an error in the body; it must
// still throw, independent of the HTTP status.
static const char *kNoResultsQuery = "site:zxqvjkwplmrtnbhgf.invalid qqzzxxkkvvwwjjhh";

TEST(client, search_no_results_throws_with_http_200) {
  const char *env_p = std::getenv("SERPAPI_KEY");
  if (env_p == nullptr || *env_p == '\0') {
    GTEST_SKIP() << "SERPAPI_KEY not set";
  }
  serpapi::Client client({{"api_key", env_p}, {"engine", "google"}});
  try {
    client.search({{"q", kNoResultsQuery}});
    FAIL() << "expected serpapi::Error";
  } catch (const serpapi::Error &e) {
    EXPECT_EQ(e.http_code(), 200);
    EXPECT_EQ(e.cause(), "Google hasn't returned any results for this query.");
    EXPECT_NE(e.payload().find("search_metadata"), std::string::npos);
  }
}

TEST(client, markdown_no_results_throws_with_http_200) {
  const char *env_p = std::getenv("SERPAPI_KEY");
  if (env_p == nullptr || *env_p == '\0') {
    GTEST_SKIP() << "SERPAPI_KEY not set";
  }
  serpapi::Client client({{"api_key", env_p}, {"engine", "google"}});
  try {
    client.markdown({{"q", kNoResultsQuery}});
    FAIL() << "expected serpapi::Error";
  } catch (const serpapi::Error &e) {
    EXPECT_EQ(e.http_code(), 200);
    EXPECT_EQ(e.cause(), "Google hasn't returned any results for this query.");
  }
}
