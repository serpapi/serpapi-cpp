#include "../src/callback.hpp"
#include "../src/query.hpp"
#include "../src/response.hpp"
#include "../src/serpapi.hpp"
#include <gtest/gtest.h>
#include <atomic>
#include <climits>
#include <cstdlib>
#include <functional>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>

// ── callback ──────────────────────────────────────────────────────────────────

TEST(callback, null_out_returns_zero) {
  EXPECT_EQ(serpapi::callback("data", 4, 1, nullptr), 0u);
}

TEST(callback, null_in_returns_zero) {
  std::string s;
  EXPECT_EQ(serpapi::callback(nullptr, 4, 1, &s), 0u);
  EXPECT_TRUE(s.empty());
}

TEST(callback, zero_size_returns_zero) {
  std::string s;
  EXPECT_EQ(serpapi::callback("data", 0, 1, &s), 0u);
  EXPECT_TRUE(s.empty());
}

TEST(callback, zero_num_returns_zero) {
  std::string s;
  EXPECT_EQ(serpapi::callback("data", 1, 0, &s), 0u);
  EXPECT_TRUE(s.empty());
}

TEST(callback, overflow_returns_zero) {
  std::string s;
  EXPECT_EQ(serpapi::callback("data", std::numeric_limits<size_t>::max(), 2, &s), 0u);
  EXPECT_TRUE(s.empty());
}

TEST(callback, appends_data) {
  std::string s;
  size_t n = serpapi::callback("hello", 5, 1, &s);
  EXPECT_EQ(n, 5u);
  EXPECT_EQ(s, "hello");
}

TEST(callback, size_times_num_appends_correctly) {
  std::string s;
  const char data[] = {'a', 'b', 'c', 'd', 'e', 'f'};
  size_t n = serpapi::callback(data, 2, 3, &s);
  EXPECT_EQ(n, 6u);
  EXPECT_EQ(s.size(), 6u);
}

TEST(callback, incremental_chunks_accumulate) {
  std::string s;
  serpapi::callback("foo", 3, 1, &s);
  serpapi::callback("bar", 3, 1, &s);
  EXPECT_EQ(s, "foobar");
}

// ── percentEncode ─────────────────────────────────────────────────────────────

TEST(percent_encode, leaves_unreserved_untouched) {
  EXPECT_EQ(serpapi::percentEncode("AZaz09-._~"), "AZaz09-._~");
}

TEST(percent_encode, encodes_reserved_and_space) {
  EXPECT_EQ(serpapi::percentEncode("a b&c=d/e?f#g+h"), "a%20b%26c%3Dd%2Fe%3Ff%23g%2Bh");
}

TEST(percent_encode, encodes_utf8_bytes) {
  EXPECT_EQ(serpapi::percentEncode("caf\xC3\xA9"), "caf%C3%A9");
}

TEST(percent_encode, empty_string) {
  EXPECT_EQ(serpapi::percentEncode(""), "");
}

// ── buildQuery ────────────────────────────────────────────────────────────────

TEST(build_query, appends_output_and_source) {
  EXPECT_EQ(serpapi::buildQuery({}, {}, "json", "serpapi-cpp:1.0"),
            "output=json&source=serpapi-cpp%3A1.0");
}

TEST(build_query, merges_defaults_and_call_params) {
  std::string q = serpapi::buildQuery({{"api_key", "k"}}, {{"q", "coffee"}}, "json", "s");
  EXPECT_EQ(q, "api_key=k&output=json&q=coffee&source=s");
}

TEST(build_query, call_params_override_defaults) {
  std::string q = serpapi::buildQuery({{"engine", "google"}}, {{"engine", "bing"}}, "json", "s");
  EXPECT_EQ(q, "engine=bing&output=json&source=s");
  EXPECT_EQ(q.find("google"), std::string::npos);
}

TEST(build_query, caller_cannot_override_output_or_source) {
  std::string q = serpapi::buildQuery({{"output", "xml"}}, {{"source", "evil"}}, "html", "s");
  EXPECT_EQ(q, "output=html&source=s");
}

TEST(build_query, encodes_keys_and_values) {
  std::string q = serpapi::buildQuery({}, {{"q", "Austin & Texas"}}, "json", "s");
  EXPECT_NE(q.find("q=Austin%20%26%20Texas"), std::string::npos);
}

// ── Error detection in responses ──────────────────────────────────────────────

static rapidjson::Document parse(const char *json) {
  rapidjson::Document d;
  d.Parse(json);
  return d;
}

TEST(json_error, returns_error_member) {
  auto d = parse(R"({"search_metadata":{"status":"Success"},"error":"No results."})");
  EXPECT_EQ(serpapi::jsonError(d), std::optional<std::string>("No results."));
}

TEST(json_error, absent_for_normal_object) {
  EXPECT_FALSE(serpapi::jsonError(parse(R"({"organic_results":[]})")));
}

TEST(json_error, ignores_nested_error_member) {
  EXPECT_FALSE(serpapi::jsonError(parse(R"({"answer_box":{"error":"x"}})")));
}

TEST(json_error, absent_for_array) {
  EXPECT_FALSE(serpapi::jsonError(parse(R"([{"error":"x"}])")));
}

TEST(json_error, non_string_error_still_detected) {
  EXPECT_EQ(serpapi::jsonError(parse(R"({"error":{"code":42}})")),
            std::optional<std::string>("Unknown error"));
}

TEST(body_error, extracts_from_json_body) {
  EXPECT_EQ(serpapi::bodyError("{\n  \"error\": \"Invalid API key.\"\n}"),
            std::optional<std::string>("Invalid API key."));
}

TEST(body_error, absent_for_non_json_body) {
  EXPECT_FALSE(serpapi::bodyError("<html>error</html>"));
  EXPECT_FALSE(serpapi::bodyError(""));
}

TEST(markdown_error, extracts_error_section) {
  const std::string md =
      "---\nsearch_metadata:\n  status: Success\n---\n\n"
      "## Search Information\n\n- Total Results: 0\n\n"
      "## Error\n\nGoogle hasn't returned any results for this query.\n\n\n";
  EXPECT_EQ(serpapi::markdownError(md),
            std::optional<std::string>("Google hasn't returned any results for this query."));
}

TEST(markdown_error, stops_at_next_section) {
  const std::string md = "## Error\r\n\r\nSomething failed.\r\n\r\n## Pagination\r\n\r\n- next\r\n";
  EXPECT_EQ(serpapi::markdownError(md), std::optional<std::string>("Something failed."));
}

TEST(markdown_error, absent_for_normal_results) {
  const std::string md =
      "## Organic Results\n\n### Error codes explained\n\nSome page about errors.\n\n"
      "## Related Searches\n\n- error\n";
  EXPECT_FALSE(serpapi::markdownError(md));
}

TEST(markdown_error, empty_section_still_detected) {
  EXPECT_EQ(serpapi::markdownError("## Error\n\n"), std::optional<std::string>("Unknown error"));
}

// ── Error ─────────────────────────────────────────────────────────────────────

TEST(error, carries_cause_code_and_payload) {
  serpapi::Error e("boom", 401, "{\"error\":\"x\"}");
  EXPECT_EQ(e.cause(), "boom");
  EXPECT_EQ(e.http_code(), 401);
  EXPECT_EQ(e.payload(), "{\"error\":\"x\"}");
}

TEST(error, what_matches_cause_for_std_exception_handlers) {
  serpapi::Error e("boom");
  const std::exception &base = e;
  EXPECT_EQ(std::string(base.what()), e.cause());
}

TEST(error, defaults_to_client_side_failure) {
  serpapi::Error e("boom");
  EXPECT_EQ(e.http_code(), 0);
  EXPECT_TRUE(e.payload().empty());
}

// Exceptions are copied while being thrown; a throwing copy would call std::terminate.
static_assert(std::is_nothrow_copy_constructible<serpapi::Error>::value,
              "serpapi::Error must be nothrow copy-constructible");

TEST(error, copies_share_cause_code_and_payload) {
  serpapi::Error original("boom", 503, "body");
  serpapi::Error copy = original;
  EXPECT_EQ(copy.cause(), "boom");
  EXPECT_EQ(copy.http_code(), 503);
  EXPECT_EQ(copy.payload(), "body");
  EXPECT_STREQ(copy.what(), "boom");
}

// ── Client construction ───────────────────────────────────────────────────────

TEST(client_unit, construct_empty_params) {
  EXPECT_NO_THROW(serpapi::Client client({}));
  EXPECT_NO_THROW(serpapi::Client client);
}

TEST(client_unit, construct_with_params) {
  EXPECT_NO_THROW(serpapi::Client client({{"api_key", "test"}, {"engine", "google"}}));
}

TEST(client_unit, set_timeout_positive) {
  serpapi::Client client({});
  EXPECT_NO_THROW(client.set_timeout(30));
}

TEST(client_unit, set_timeout_zero) {
  serpapi::Client client({});
  EXPECT_NO_THROW(client.set_timeout(0));
}

static_assert(std::is_copy_constructible<serpapi::Client>::value, "Client must be copyable");
static_assert(std::is_nothrow_move_constructible<serpapi::Client>::value &&
                  std::is_nothrow_move_assignable<serpapi::Client>::value,
              "Client moves must not throw");

TEST(client_unit, copies_are_independent) {
  // limit is observable through the location endpoint (free, no api_key).
  serpapi::Client original({{"limit", "1"}});
  serpapi::Client copy(original);
  original = serpapi::Client({{"limit", "3"}});

  EXPECT_EQ(copy.location({{"q", "Austin"}}).GetArray().Size(), 1u);
  EXPECT_GT(original.location({{"q", "Austin"}}).GetArray().Size(), 1u);
}

TEST(client_unit, copy_assignment_and_self_assignment) {
  serpapi::Client a({{"limit", "1"}});
  serpapi::Client b;
  b = a;
  const serpapi::Client &alias = b;
  b = alias;
  EXPECT_EQ(b.location({{"q", "Austin"}}).GetArray().Size(), 1u);
}

TEST(client_unit, move_and_reassign_moved_from) {
  serpapi::Client a({{"limit", "1"}});
  serpapi::Client b(std::move(a));
  EXPECT_EQ(b.location({{"q", "Austin"}}).GetArray().Size(), 1u);

  a = serpapi::Client({{"limit", "1"}});  // a moved-from Client may be assigned to
  EXPECT_EQ(a.location({{"q", "Austin"}}).GetArray().Size(), 1u);
}

// ── Edge cases (network required – uses location which needs no API key) ──────

TEST(client_unit, location_special_chars_in_query) {
  serpapi::Client client({});
  // & and = in the query value must be percent-encoded, not break the URL
  rapidjson::Document d = client.location({{"q", "Austin & Texas"}, {"limit", "1"}});
  EXPECT_FALSE(d.HasParseError());
  EXPECT_TRUE(d.IsArray());
}

TEST(client_unit, location_empty_query) {
  serpapi::Client client({});
  // Empty string value should not crash URL encoding
  rapidjson::Document d = client.location({{"q", ""}, {"limit", "1"}});
  EXPECT_FALSE(d.HasParseError());
}

TEST(client_unit, default_params_merged_with_call_params) {
  // Default params set at construction should be sent alongside call params
  serpapi::Client client({{"limit", "1"}});
  rapidjson::Document d = client.location({{"q", "Austin"}});
  EXPECT_FALSE(d.HasParseError());
  EXPECT_TRUE(d.IsArray());
  // Only 1 result because of default limit=1
  EXPECT_LE(d.GetArray().Size(), 1u);
}

// ── Error handling (network required) ─────────────────────────────────────────

TEST(client_unit, search_with_invalid_api_key_throws_http_error) {
  serpapi::Client client({{"engine", "google"}, {"api_key", "invalid_key"}});
  try {
    client.search({{"q", "coffee"}});
    FAIL() << "expected serpapi::Error";
  } catch (const serpapi::Error &e) {
    EXPECT_EQ(e.http_code(), 401);
    EXPECT_EQ(e.cause().rfind("Invalid API key", 0), 0u) << e.cause();
    rapidjson::Document d;
    d.Parse(e.payload().c_str());
    ASSERT_FALSE(d.HasParseError());
    EXPECT_TRUE(d.HasMember("error"));
  }
}

TEST(client_unit, html_with_invalid_api_key_throws_http_error) {
  serpapi::Client client({{"engine", "google"}, {"api_key", "invalid_key"}});
  EXPECT_THROW(client.html({{"q", "coffee"}}), serpapi::Error);
}

TEST(client_unit, search_archive_encodes_id) {
  // A path-traversal-looking id must be escaped, resulting in a 404 rather than
  // hitting a different endpoint.
  serpapi::Client client({});
  try {
    client.search_archive("../account");
    FAIL() << "expected serpapi::Error";
  } catch (const serpapi::Error &e) {
    EXPECT_GE(e.http_code(), 400);
  }
}

TEST(client_unit, error_is_a_runtime_error) {
  serpapi::Client client({{"engine", "google"}, {"api_key", "invalid_key"}});
  EXPECT_THROW(client.search({{"q", "coffee"}}), std::runtime_error);
}

// ── Transport failure (offline) ───────────────────────────────────────────────

// Routes libcurl through a proxy on a closed local port so every request fails
// before reaching the network, deterministically and without credentials.
class TransportFailure : public ::testing::Test {
protected:
  void SetUp() override {
    for (const char *name : kProxyVars) {
      const char *value = std::getenv(name);
      saved_.emplace_back(name, value ? std::string(value) : std::string());
      had_.push_back(value != nullptr);
      setenv(name, "http://127.0.0.1:1", 1);
    }
    unsetenv("no_proxy");
    unsetenv("NO_PROXY");
  }

  void TearDown() override {
    for (size_t i = 0; i < saved_.size(); ++i) {
      if (had_[i]) setenv(saved_[i].first.c_str(), saved_[i].second.c_str(), 1);
      else unsetenv(saved_[i].first.c_str());
    }
  }

  static void expectTransportError(const std::function<void()> &call) {
    try {
      call();
      FAIL() << "expected serpapi::Error";
    } catch (const serpapi::Error &e) {
      EXPECT_EQ(e.http_code(), 0);
      EXPECT_EQ(e.cause().rfind("CURL error: ", 0), 0u) << e.cause();
      EXPECT_TRUE(e.payload().empty());
    }
  }

private:
  static constexpr const char *kProxyVars[] = {"https_proxy", "HTTPS_PROXY"};
  std::vector<std::pair<std::string, std::string>> saved_;
  std::vector<bool> had_;
};

TEST_F(TransportFailure, every_method_throws_with_http_code_zero) {
  const serpapi::Client client({{"api_key", "unused"}});
  expectTransportError([&] { client.search({{"q", "coffee"}}); });
  expectTransportError([&] { client.html({{"q", "coffee"}}); });
  expectTransportError([&] { client.markdown({{"q", "coffee"}}); });
  expectTransportError([&] { client.md({{"q", "coffee"}}); });
  expectTransportError([&] { client.search_archive("abc"); });
  expectTransportError([&] { client.account(); });
  expectTransportError([&] { client.location({{"q", "Austin"}}); });
}

// ── Thread safety ─────────────────────────────────────────────────────────────

TEST(client_unit, concurrent_location_calls_are_safe) {
  // Verifies std::call_once CURL init and shared Client are race-free
  serpapi::Client client({});
  std::map<std::string, std::string> params = {{"q", "Austin"}, {"limit", "1"}};
  std::atomic<int> success{0};

  std::vector<std::thread> threads;
  threads.reserve(4);
  for (int i = 0; i < 4; ++i) {
    threads.emplace_back([&]() {
      rapidjson::Document d = client.location(params);
      if (!d.HasParseError() && d.IsArray())
        ++success;
    });
  }
  for (auto& t : threads) t.join();

  EXPECT_EQ(success.load(), 4);
}
