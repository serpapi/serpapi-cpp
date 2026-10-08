#include "query.hpp"

namespace serpapi {

static bool isUnreserved(unsigned char c) {
  return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
         (c >= '0' && c <= '9') || c == '-' || c == '.' || c == '_' || c == '~';
}

std::string percentEncode(const std::string &value) {
  static const char hex[] = "0123456789ABCDEF";
  std::string out;
  out.reserve(value.size() * 3);
  for (unsigned char c : value) {
    if (isUnreserved(c)) {
      out += static_cast<char>(c);
    } else {
      out += '%';
      out += hex[c >> 4];
      out += hex[c & 0x0F];
    }
  }
  return out;
}

std::string buildQuery(const Parameters &defaults, const Parameters &parameter,
                       const std::string &output, const std::string &source) {
  Parameters merged = defaults;
  for (const auto &entry : parameter) {
    merged[entry.first] = entry.second;
  }
  merged["output"] = output;
  merged["source"] = source;

  std::string query;
  for (const auto &entry : merged) {
    if (!query.empty()) query += '&';
    query += percentEncode(entry.first);
    query += '=';
    query += percentEncode(entry.second);
  }
  return query;
}

} // namespace serpapi
