// Google search example:
//   1. run a search,
//   2. retrieve the same search again from the search archive by its id,
//   3. page through the results with the `start` offset.
//
// Uses up to max_pages searches: the first search is page 1.
#include <cstdlib>
#include <iostream>
#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>
#include <serpapi.hpp>
#include <string>

static void print_json(const rapidjson::Value &value) {
  rapidjson::StringBuffer buffer;
  rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
  value.Accept(writer);
  std::cout << buffer.GetString() << std::endl;
}

// Prints the organic result titles, numbered across pages.
static void print_titles(const rapidjson::Document &page, int &count) {
  if (!page.HasMember("organic_results") || !page["organic_results"].IsArray()) return;
  for (const auto &result : page["organic_results"].GetArray()) {
    if (result.HasMember("title") && result["title"].IsString()) {
      std::cout << "  " << ++count << ". " << result["title"].GetString() << std::endl;
    }
  }
}

// serpapi_pagination.next is present only while there is a next page.
static bool has_next_page(const rapidjson::Document &page) {
  return page.HasMember("serpapi_pagination") && page["serpapi_pagination"].IsObject() &&
         page["serpapi_pagination"].HasMember("next");
}

int main() {
  const char *env_p = std::getenv("SERPAPI_KEY");
  if (env_p == nullptr || *env_p == '\0') {
    std::cout << "SERPAPI_KEY not set, skipping example" << std::endl;
    return 0;
  }

  serpapi::Client client({{"api_key", env_p}, {"engine", "google"}});
  const serpapi::Parameters query = {{"q", "coffee"}, {"location", "Austin,TX"}};

  const int max_pages = 3;
  // Google's start is an offset into its ranking and advances by 10 per page,
  // even when a page shows fewer organic results.
  const int page_size = 10;

  try {
    // 1. Search
    rapidjson::Document page = client.search(query);
    if (!page.HasMember("search_metadata") || !page["search_metadata"].IsObject()) {
      std::cerr << "Unexpected response: no search_metadata" << std::endl;
      return 1;
    }
    const rapidjson::Value &metadata = page["search_metadata"];
    std::cout << "Search metadata:" << std::endl;
    print_json(metadata);

    // 2. Search archive
    if (metadata.HasMember("id") && metadata["id"].IsString()) {
      const std::string id = metadata["id"].GetString();
      rapidjson::Document archived = client.search_archive(id);
      const bool ok = archived.HasMember("search_metadata") &&
                      archived["search_metadata"].HasMember("status") &&
                      archived["search_metadata"]["status"] == "Success";
      std::cout << "\nRetrieved search " << id << " from the archive: "
                << (ok ? "Success" : "unexpected status") << std::endl;
    }

    // 3. Pagination: the first search is page 1. Requesting past the last page
    // fails or returns nothing and still uses a credit, so follow `next` only.
    int count = 0;
    std::cout << "\nPage 1 (start=0)" << std::endl;
    print_titles(page, count);

    for (int n = 1; n < max_pages && has_next_page(page); ++n) {
      serpapi::Parameters next = query;
      next["start"] = std::to_string(n * page_size);
      page = client.search(next);

      std::cout << "Page " << n + 1 << " (start=" << next["start"] << ")" << std::endl;
      print_titles(page, count);
    }
    std::cout << count << " results" << std::endl;
  } catch (const serpapi::Error &e) {
    // cause() is SerpApi's error message; http_code() is 0 for transport failures.
    std::cerr << "Request failed: " << e.cause() << " (HTTP " << e.http_code() << ")" << std::endl;
    return 1;
  }

  return 0;
}
