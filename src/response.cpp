#include "response.hpp"

namespace serpapi {

namespace {

std::string trim(const std::string &s) {
  const char *ws = " \t\r\n";
  const auto begin = s.find_first_not_of(ws);
  if (begin == std::string::npos) return "";
  return s.substr(begin, s.find_last_not_of(ws) - begin + 1);
}

std::string stripCarriageReturn(std::string line) {
  if (!line.empty() && line.back() == '\r') line.pop_back();
  return line;
}

} // namespace

std::optional<std::string> jsonError(const rapidjson::Value &value) {
  if (!value.IsObject()) return std::nullopt;
  auto it = value.FindMember("error");
  if (it == value.MemberEnd()) return std::nullopt;
  if (it->value.IsString()) {
    return std::string(it->value.GetString(), it->value.GetStringLength());
  }
  return std::string("Unknown error");
}

std::optional<std::string> bodyError(const std::string &body) {
  rapidjson::Document d;
  d.Parse(body.data(), body.size());
  if (d.HasParseError()) return std::nullopt;
  return jsonError(d);
}

std::optional<std::string> markdownError(const std::string &body) {
  size_t pos = 0;
  bool inSection = false;
  std::string message;

  while (pos <= body.size()) {
    size_t end = body.find('\n', pos);
    if (end == std::string::npos) end = body.size();
    const std::string line = stripCarriageReturn(body.substr(pos, end - pos));
    pos = end + 1;

    if (line.rfind("## ", 0) == 0) {
      if (inSection) break;
      inSection = (line == "## Error");
      continue;
    }
    if (inSection) {
      if (!message.empty()) message += '\n';
      message += line;
    }
  }

  if (!inSection) return std::nullopt;
  message = trim(message);
  return message.empty() ? std::string("Unknown error") : message;
}

} // namespace serpapi
