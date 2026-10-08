// Demonstrates Client::account() while printing only non-sensitive plan and usage fields.
// SerpApi Account API docs: https://serpapi.com/account-api
#include <cstdlib>
#include <iostream>
#include <rapidjson/document.h>
#include <serpapi.hpp>

static void print_field(const rapidjson::Value &account, const char *name) {
  std::cout << name << ": ";
  if (!account.IsObject() || !account.HasMember(name)) {
    std::cout << "(missing)" << std::endl;
    return;
  }

  const rapidjson::Value &value = account[name];
  if (value.IsString()) {
    std::cout << value.GetString();
  } else if (value.IsInt64()) {
    std::cout << value.GetInt64();
  } else if (value.IsUint64()) {
    std::cout << value.GetUint64();
  } else if (value.IsDouble()) {
    std::cout << value.GetDouble();
  } else if (value.IsBool()) {
    std::cout << (value.GetBool() ? "true" : "false");
  } else if (value.IsNull()) {
    std::cout << "null";
  } else {
    std::cout << "(unprintable)";
  }
  std::cout << std::endl;
}

int main() {
  const char *env_p = std::getenv("SERPAPI_KEY");
  if (env_p == nullptr || *env_p == '\0') {
    std::cout << "SERPAPI_KEY not set, skipping example" << std::endl;
    return 0;
  }

  serpapi::Client client({{"api_key", env_p}});

  try {
    rapidjson::Document account = client.account();
    print_field(account, "plan_name");
    print_field(account, "searches_per_month");
    print_field(account, "this_month_usage");
    print_field(account, "plan_searches_left");
    print_field(account, "total_searches_left");
  } catch (const serpapi::Error &e) {
    std::cerr << "Request failed: " << e.cause() << " (HTTP " << e.http_code() << ")" << std::endl;
    return 1;
  }

  return 0;
}
