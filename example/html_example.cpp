// Demonstrates Client::html() returning raw search engine HTML.
// SerpApi Search API docs: https://serpapi.com/search-api
#include <cstdlib>
#include <iostream>
#include <serpapi.hpp>
#include <string>

int main() {
  const char *env_p = std::getenv("SERPAPI_KEY");
  if (env_p == nullptr || *env_p == '\0') {
    std::cout << "SERPAPI_KEY not set, skipping example" << std::endl;
    return 0;
  }

  serpapi::Client client({{"api_key", env_p}, {"engine", "google"}});

  try {
    const std::string html = client.html({{"q", "coffee"}, {"location", "Austin,TX"}});
    std::cout << "HTML bytes: " << html.size() << std::endl;
    std::cout << "Preview:" << std::endl << html.substr(0, 300) << std::endl;
  } catch (const serpapi::Error &e) {
    std::cerr << "Request failed: " << e.cause() << " (HTTP " << e.http_code() << ")" << std::endl;
    return 1;
  }

  return 0;
}
