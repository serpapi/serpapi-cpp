// Demonstrates Client::search_archive(); archive lookups do not use search credits.
// SerpApi Search Archive API docs: https://serpapi.com/search-archive-api
#include <cstdlib>
#include <iostream>
#include <rapidjson/document.h>
#include <serpapi.hpp>
#include <string>

static bool print_archive_status(const rapidjson::Document &archived) {
  if (!archived.IsObject() || !archived.HasMember("search_metadata") ||
      !archived["search_metadata"].IsObject()) {
    std::cerr << "Unexpected response: no search_metadata" << std::endl;
    return false;
  }

  const rapidjson::Value &metadata = archived["search_metadata"];
  if (!metadata.HasMember("id") || !metadata["id"].IsString() ||
      !metadata.HasMember("status") || !metadata["status"].IsString()) {
    std::cerr << "Unexpected response: incomplete search_metadata" << std::endl;
    return false;
  }

  std::cout << "Search id: " << metadata["id"].GetString() << std::endl;
  std::cout << "Status: " << metadata["status"].GetString() << std::endl;
  return true;
}

int main(int argc, const char *argv[]) {
  const char *env_p = std::getenv("SERPAPI_KEY");
  if (env_p == nullptr || *env_p == '\0') {
    std::cout << "SERPAPI_KEY not set, skipping example" << std::endl;
    return 0;
  }

  serpapi::Client client({{"api_key", env_p}, {"engine", "google"}});

  try {
    std::string search_id;
    if (argc > 1) {
      search_id = argv[1];
    } else {
      rapidjson::Document results = client.search({{"q", "coffee"}, {"location", "Austin,TX"}});
      if (!results.IsObject() || !results.HasMember("search_metadata") ||
          !results["search_metadata"].IsObject()) {
        std::cerr << "Unexpected response: no search_metadata" << std::endl;
        return 1;
      }

      const rapidjson::Value &metadata = results["search_metadata"];
      if (!metadata.HasMember("id") || !metadata["id"].IsString()) {
        std::cerr << "Unexpected response: no search id" << std::endl;
        return 1;
      }
      search_id = metadata["id"].GetString();
    }

    rapidjson::Document archived = client.search_archive(search_id);
    if (!print_archive_status(archived)) return 1;
  } catch (const serpapi::Error &e) {
    std::cerr << "Request failed: " << e.cause() << " (HTTP " << e.http_code() << ")" << std::endl;
    return 1;
  }

  return 0;
}
