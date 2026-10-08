// Runs a Yandex search with SerpApi and prints the first organic result titles.
// Docs: https://serpapi.com/yandex-search-api
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <rapidjson/document.h>
#include <serpapi.hpp>

static const char *summary_text(const rapidjson::Value &item) {
  if (!item.IsObject()) return "(no title)";

  const char *fields[] = {"title", "name", "snippet", "link"};
  const auto match = std::find_if(fields, fields + (sizeof(fields) / sizeof(fields[0])),
                                  [&item](const char *field) {
                                    return item.HasMember(field) && item[field].IsString();
                                  });
  if (match != fields + (sizeof(fields) / sizeof(fields[0]))) return item[*match].GetString();
  return "(no title)";
}

static int print_results(const rapidjson::Document &results) {
  if (!results.HasMember("organic_results") || !results["organic_results"].IsArray()) {
    std::cerr << "organic_results not found" << std::endl;
    return 1;
  }

  const rapidjson::Value &items = results["organic_results"];
  std::cout << items.Size() << " organic_results" << std::endl;
  const rapidjson::SizeType limit = std::min<rapidjson::SizeType>(items.Size(), 5);
  for (rapidjson::SizeType i = 0; i < limit; ++i) {
    std::cout << "  " << i + 1 << ". " << summary_text(items[i]) << std::endl;
  }
  return 0;
}

int main() {
  const char *key = std::getenv("SERPAPI_KEY");
  if (key == nullptr || *key == '\0') {
    std::cout << "SERPAPI_KEY not set, skipping example" << std::endl;
    return 0;
  }

  serpapi::Client client({{"api_key", key}, {"engine", "yandex"}});
  const serpapi::Parameters query = {{"text", "coffee"}};

  try {
    rapidjson::Document results = client.search(query);
    return print_results(results);
  } catch (const serpapi::Error &e) {
    std::cerr << "Request failed: " << e.cause() << " (HTTP " << e.http_code() << ")" << std::endl;
    return 1;
  }
}
