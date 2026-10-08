// Demonstrates Client::search() returning JSON search results.
// SerpApi Search API docs: https://serpapi.com/search-api
#include <cstdlib>
#include <iostream>
#include <rapidjson/document.h>
#include <serpapi.hpp>

int main() {
  const char *env_p = std::getenv("SERPAPI_KEY");
  if (env_p == nullptr || *env_p == '\0') {
    std::cout << "SERPAPI_KEY not set, skipping example" << std::endl;
    return 0;
  }

  serpapi::Client client({{"api_key", env_p}, {"engine", "google"}});

  try {
    rapidjson::Document results = client.search({{"q", "coffee"}, {"location", "Austin,TX"}});

    std::size_t count = 0;
    if (results.IsObject() && results.HasMember("organic_results") && results["organic_results"].IsArray()) {
      count = results["organic_results"].Size();
    }

    std::cout << "Organic results: " << count << std::endl;
    if (results.IsObject() && results.HasMember("organic_results") && results["organic_results"].IsArray()) {
      std::size_t printed = 0;
      for (const auto &result : results["organic_results"].GetArray()) {
        if (result.IsObject() && result.HasMember("title") && result["title"].IsString()) {
          std::cout << "  " << ++printed << ". " << result["title"].GetString() << std::endl;
          if (printed == 5) break;
        }
      }
    }
  } catch (const serpapi::Error &e) {
    std::cerr << "Request failed: " << e.cause() << " (HTTP " << e.http_code() << ")" << std::endl;
    return 1;
  }

  return 0;
}
