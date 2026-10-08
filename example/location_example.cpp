// Demonstrates Client::location() for location autocomplete without an api_key.
// SerpApi Locations API docs: https://serpapi.com/locations-api
#include <iostream>
#include <rapidjson/document.h>
#include <serpapi.hpp>

int main() {
  serpapi::Client client;

  try {
    rapidjson::Document locations = client.location({{"q", "Austin"}, {"limit", "3"}});
    if (!locations.IsArray()) {
      std::cerr << "Unexpected response: locations is not an array" << std::endl;
      return 1;
    }

    for (const auto &location : locations.GetArray()) {
      if (!location.IsObject() || !location.HasMember("canonical_name") ||
          !location["canonical_name"].IsString()) {
        continue;
      }

      std::cout << location["canonical_name"].GetString();
      if (location.HasMember("google_id")) {
        const rapidjson::Value &google_id = location["google_id"];
        if (google_id.IsString()) {
          std::cout << " (" << google_id.GetString() << ")";
        } else if (google_id.IsInt64()) {
          std::cout << " (" << google_id.GetInt64() << ")";
        } else if (google_id.IsUint64()) {
          std::cout << " (" << google_id.GetUint64() << ")";
        }
      }
      std::cout << std::endl;
    }
  } catch (const serpapi::Error &e) {
    std::cerr << "Request failed: " << e.cause() << " (HTTP " << e.http_code() << ")" << std::endl;
    return 1;
  }

  return 0;
}
