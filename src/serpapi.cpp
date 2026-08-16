/***
 * Scrape search powered by SerpApi
 */

#include "callback.hpp"
#include <serpapi.hpp>
#include <mutex>
#include <sstream>

namespace serpapi {

const static std::string HOST = "https://serpapi.com";
const static std::string NAME = "serpapi-cpp";
const static std::string VERSION = "0.5.0";

static std::once_flag curl_init_flag;

/**
 * @brief Constructs a client with default parameters merged into every request.
 * @param parameter Default parameters (e.g. api_key, engine) applied to all calls.
 */
Client::Client(const std::map<std::string, std::string> &parameter) {
  this->parameter = parameter;
}

Client::~Client() {}

/**
 * @brief Runs a search and returns the raw search engine result page as HTML.
 * @param parameter Search parameters (e.g. q, location) merged with the client defaults.
 * @return Raw HTML response body.
 */
std::string Client::html(const std::map<std::string, std::string> &parameter) {
  GetResponse gr = Client::get("/search", "html", parameter);
  return gr.payload;
}

/**
 * @brief Runs a search and returns the raw search engine result page as Markdown.
 * @param parameter Search parameters (e.g. q, location) merged with the client defaults.
 * @return Raw Markdown response body.
 */
std::string Client::markdown(const std::map<std::string, std::string> &parameter) {
  GetResponse gr = Client::get("/search", "md", parameter);
  return gr.payload;
}

/**
 * @brief Runs a search and returns the parsed/structured results as JSON.
 * @param parameter Search parameters (e.g. q, location) merged with the client defaults.
 * @return Parsed JSON response as a rapidjson::Document.
 */
rapidjson::Document Client::search(const std::map<std::string, std::string> &parameter) {
  return Client::json("/search", parameter);
}

/**
 * @brief Retrieves a previously run search from the search archive.
 * @param id Search id, as returned in search_metadata.id from a prior search().
 * @return Parsed JSON response as a rapidjson::Document.
 */
rapidjson::Document Client::search_archive(const std::string &id) {
  return Client::json("/searches/" + id + ".json", std::map<std::string, std::string>());
}

/**
 * @brief Retrieves account information (e.g. plan, usage) for the given api_key.
 * @param parameter Request parameters (e.g. api_key) merged with the client defaults.
 * @return Parsed JSON response as a rapidjson::Document.
 */
rapidjson::Document Client::account(const std::map<std::string, std::string> &parameter) {
  return Client::json("/account.json", parameter);
}

/**
 * @brief Looks up supported locations matching a query.
 * @param parameter Request parameters (e.g. q, limit) merged with the client defaults.
 * @return Parsed JSON response as a rapidjson::Document.
 */
rapidjson::Document Client::location(const std::map<std::string, std::string> &parameter) {
  return Client::json("/locations.json", parameter);
}

/**
 * @brief Fetches a JSON endpoint and parses it into a rapidjson::Document.
 * @param uri Endpoint path, relative to HOST, to request with output=json.
 * @param parameter Request parameters merged with the client defaults.
 * @return Parsed JSON response, or a Document with an "error" member if the
 *         payload could not be parsed.
 */
rapidjson::Document Client::json(const std::string &uri, const std::map<std::string, std::string> &parameter) {
  GetResponse gr = get(uri, "json", parameter);
  rapidjson::Document d;
  d.Parse(gr.payload.c_str());
  if (d.HasParseError()) {
    d.SetObject();
    d.AddMember("error", "JSON parse error", d.GetAllocator());
  }
  return d;
}

/**
 * @brief URL-encodes and joins a parameter map into a query string.
 * @param curl Initialized CURL handle used to escape keys and values.
 * @param parameter Parameters to encode.
 * @return "key=value&key=value" encoded string, or "" if escaping any entry failed.
 */
std::string encodeUrl(CURL *curl, const std::map<std::string, std::string> &parameter) {
  std::ostringstream oss;
  bool first = true;

  for (const auto& entry : parameter) {
    char *escaped_key = curl_easy_escape(curl, entry.first.c_str(), entry.first.length());
    char *escaped_value = curl_easy_escape(curl, entry.second.c_str(), entry.second.length());

    if (!escaped_key || !escaped_value) {
      if (escaped_key) curl_free(escaped_key);
      if (escaped_value) curl_free(escaped_value);
      return "";
    }

    if (!first) oss << "&";
    oss << escaped_key << "=" << escaped_value;
    first = false;

    curl_free(escaped_key);
    curl_free(escaped_value);
  }
  return oss.str();
}

/**
 * @brief Builds the full query string for a request.
 *
 * Concatenates call-specific parameters, then the client's default
 * parameters, then the output format and source tag.
 *
 * @param curl Initialized CURL handle used to escape parameters.
 * @param output Desired response format (e.g. "json", "html", "md").
 * @param parameter Call-specific parameters, merged with the client defaults.
 * @return Fully encoded query string, without the leading '?'.
 */
std::string Client::url(CURL *curl, const std::string &output,
                        const std::map<std::string, std::string> &parameter) {
  std::string url_str = encodeUrl(curl, parameter);
  if (this->parameter.size() > 0) {
    if (!url_str.empty()) url_str += "&";
    url_str += encodeUrl(curl, this->parameter);
  }
  url_str += "&output=" + output;
  url_str += "&source=" + NAME + ":" + VERSION;
  return url_str;
}

/**
 * @brief Performs the HTTP GET request against SerpApi.
 * @param uri Endpoint path, relative to HOST (e.g. "/search").
 * @param output Desired response format (e.g. "json", "html", "md").
 * @param parameter Call-specific parameters, merged with the client defaults.
 * @return HTTP status code and raw response body. httpCode is 0 on a
 *         client-side failure (CURL init or transport error), with the
 *         error message in payload.
 */
GetResponse Client::get(const std::string &uri, const std::string &output,
                        const std::map<std::string, std::string> &parameter) {
  std::call_once(curl_init_flag, []() { curl_global_init(CURL_GLOBAL_DEFAULT); });

  CURL *curl = curl_easy_init();
  if (!curl) {
    GetResponse gr;
    gr.httpCode = 0;
    gr.payload = "CURL initialization failed";
    return gr;
  }

  const std::string url_params = this->url(curl, output, parameter);
  const std::string full_url = HOST + uri + "?" + url_params;

  curl_easy_setopt(curl, CURLOPT_URL, full_url.c_str());
  curl_easy_setopt(curl, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4);
  curl_easy_setopt(curl, CURLOPT_TIMEOUT, (long)timeout);
  curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);

  long httpCode(0);
  std::string httpData;
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, callback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &httpData);

  CURLcode res = curl_easy_perform(curl);
  if (res != CURLE_OK) {
    curl_easy_cleanup(curl);
    GetResponse gr;
    gr.httpCode = 0;
    gr.payload = std::string("CURL error: ") + curl_easy_strerror(res);
    return gr;
  }

  curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
  curl_easy_cleanup(curl);

  GetResponse gr;
  gr.httpCode = httpCode;
  gr.payload = httpData;
  return gr;
}

} // namespace serpapi
