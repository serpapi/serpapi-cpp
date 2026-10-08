/***
 * Scrape search powered by SerpApi
 */

#include "callback.hpp"
#include "query.hpp"
#include "response.hpp"
#include <serpapi.hpp>
#include <curl/curl.h>
#include <memory>
#include <mutex>
#include <optional>
#include <rapidjson/error/en.h>

namespace serpapi {

const static std::string HOST = "https://serpapi.com";
const static std::string NAME = "serpapi-cpp";
#ifndef SERPAPI_VERSION
#error "SERPAPI_VERSION must be defined by the build system"
#endif
const static std::string VERSION = SERPAPI_VERSION;

static std::once_flag curl_init_flag;

namespace {

struct Response {
  long http_code = 0;
  std::string payload;
};

using CurlHandle = std::unique_ptr<CURL, decltype(&curl_easy_cleanup)>;

/// Performs an HTTP GET; throws Error only on client-side (pre-response) failure.
Response httpGet(const std::string &url, long timeout) {
  std::call_once(curl_init_flag, []() { curl_global_init(CURL_GLOBAL_DEFAULT); });

  CurlHandle curl(curl_easy_init(), &curl_easy_cleanup);
  if (!curl) {
    throw Error("CURL initialization failed");
  }

  Response response;
  curl_easy_setopt(curl.get(), CURLOPT_URL, url.c_str());
  curl_easy_setopt(curl.get(), CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4);
  curl_easy_setopt(curl.get(), CURLOPT_TIMEOUT, timeout);
  curl_easy_setopt(curl.get(), CURLOPT_FOLLOWLOCATION, 1L);
  curl_easy_setopt(curl.get(), CURLOPT_SSL_VERIFYHOST, 2L);
  curl_easy_setopt(curl.get(), CURLOPT_SSL_VERIFYPEER, 1L);
  curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, callback);
  curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, &response.payload);

  CURLcode res = curl_easy_perform(curl.get());
  if (res != CURLE_OK) {
    throw Error(std::string("CURL error: ") + curl_easy_strerror(res));
  }

  curl_easy_getinfo(curl.get(), CURLINFO_RESPONSE_CODE, &response.http_code);
  return response;
}

} // namespace

struct Error::Detail {
  std::string cause;
  long http_code;
  std::string payload;
};

Error::Error(const std::string &message, long http_code, std::string payload)
    : std::runtime_error(message),
      detail_(std::make_shared<const Detail>(Detail{message, http_code, std::move(payload)})) {}

const std::string &Error::cause() const noexcept { return detail_->cause; }
long Error::http_code() const noexcept { return detail_->http_code; }
const std::string &Error::payload() const noexcept { return detail_->payload; }

namespace detail {

/// Per-client settings, shared by the request helpers below.
struct Settings {
  Parameters parameter;
  int timeout = 60;
};

} // namespace detail

namespace {

using detail::Settings;

/// Sends the request and returns the body; throws Error on transport failure or HTTP status >= 400.
std::string request(const Settings &impl, const std::string &uri, const std::string &output,
                    const Parameters &parameter, long *http_code = nullptr) {
  const std::string query = buildQuery(impl.parameter, parameter, output, NAME + ":" + VERSION);
  Response response = httpGet(HOST + uri + "?" + query, static_cast<long>(impl.timeout));

  if (response.http_code >= 400) {
    // SerpApi sends {"error": "..."} for every output format on HTTP errors.
    std::string message = bodyError(response.payload)
                              .value_or("HTTP error " + std::to_string(response.http_code));
    throw Error(message, response.http_code, std::move(response.payload));
  }
  if (http_code) *http_code = response.http_code;
  return response.payload;
}

rapidjson::Document json(const Settings &impl, const std::string &uri,
                         const Parameters &parameter) {
  long http_code = 0;
  std::string payload = request(impl, uri, "json", parameter, &http_code);

  rapidjson::Document d;
  d.Parse(payload.data(), payload.size());
  if (d.HasParseError()) {
    throw Error(std::string("JSON parse error: ") + rapidjson::GetParseError_En(d.GetParseError()),
                http_code, std::move(payload));
  }
  if (auto message = jsonError(d)) {
    throw Error(*message, http_code, std::move(payload));
  }
  return d;
}

} // namespace

struct Client::Impl : detail::Settings {};

Client::Client(Parameters parameter) : impl_(std::make_unique<Impl>()) {
  impl_->parameter = std::move(parameter);
}

Client::~Client() = default;
Client::Client(const Client &other) : impl_(std::make_unique<Impl>(*other.impl_)) {}
Client::Client(Client &&other) noexcept = default;
Client &Client::operator=(Client &&other) noexcept = default;

Client &Client::operator=(const Client &other) {
  if (this != &other) impl_ = std::make_unique<Impl>(*other.impl_);
  return *this;
}

void Client::set_timeout(int seconds) { impl_->timeout = seconds; }

std::string Client::html(const Parameters &parameter) const {
  return request(*impl_, "/search", "html", parameter);
}

std::string Client::markdown(const Parameters &parameter) const {
  long http_code = 0;
  std::string payload = request(*impl_, "/search", "md", parameter, &http_code);
  if (auto message = markdownError(payload)) {
    throw Error(*message, http_code, std::move(payload));
  }
  return payload;
}

rapidjson::Document Client::search(const Parameters &parameter) const {
  return json(*impl_, "/search", parameter);
}

rapidjson::Document Client::search_archive(const std::string &search_id) const {
  return json(*impl_, "/searches/" + percentEncode(search_id) + ".json", {});
}

rapidjson::Document Client::account(const Parameters &parameter) const {
  return json(*impl_, "/account.json", parameter);
}

rapidjson::Document Client::location(const Parameters &parameter) const {
  return json(*impl_, "/locations.json", parameter);
}

} // namespace serpapi
