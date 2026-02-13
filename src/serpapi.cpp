/***
 * Scrape search powered by SerpApi
 */

#include "callback.hpp"
#include <serpapi.hpp>

namespace serpapi {

const static std::string HOST = "https://serpapi.com";
const static std::string NAME = "serpapi-cpp";
const static std::string VERSION = "0.2.0";

using namespace rapidjson;
using namespace std;

Client::Client(const map<string, string> &parameter) {
  this->parameter = parameter;
}

Client::~Client() {}

/***
 * Get HTML search results
 */
string Client::html(const map<string, string> &parameter) {
  GetResponse gr = Client::get("/search", "html", parameter);
  return gr.payload;
}

Document Client::search(const map<string, string> &parameter) {
  return Client::json("/search", parameter);
}

Document Client::search_archive(const string &id) {
  return Client::json("/searches/" + id + ".json", map<string, string>());
}

Document Client::account(const map<string, string> &parameter) {
  return Client::json("/account.json", parameter);
}

Document Client::location(const map<string, string> &parameter) {
  return Client::json("/locations.json", parameter);
}

Document Client::json(const string &uri, const map<string, string> &parameter) {
  GetResponse gr = get(uri, "json", parameter);
  const char *json_payload = gr.payload.c_str();
  Document d;
  d.Parse(json_payload);
  return d;
}

string encodeUrl(CURL *curl, const map<string, string> &parameter) {
  string s = "";
  map<string, string>::const_iterator it;
  for (it = parameter.begin(); it != parameter.end(); ++it) {
    if (s != "") {
      s += "&";
    }

    char *escaped_key =
        curl_easy_escape(curl, it->first.c_str(), it->first.length());
    char *escaped_value =
        curl_easy_escape(curl, it->second.c_str(), it->second.length());

    s += string(escaped_key) + "=" + string(escaped_value);

    curl_free(escaped_key);
    curl_free(escaped_value);
  }
  return s;
}

string Client::url(CURL *curl, const string &output,
                   const map<string, string> &parameter) {
  // encode parameter
  string url_str = encodeUrl(curl, parameter);
  if (this->parameter.size() > 0) {
    if (!url_str.empty())
      url_str += "&";
    url_str += encodeUrl(curl, this->parameter);
  }
  // append output format
  url_str += "&output=" + output;
  // append source language
  url_str += "&source=" + NAME + ":" + VERSION;
  return url_str;
}

GetResponse Client::get(const string &uri, const string &output,
                        const map<string, string> &parameter) {
  static bool initialized = false;
  if (!initialized) {
    curl_global_init(CURL_GLOBAL_DEFAULT);
    initialized = true;
  }
  CURL *curl = curl_easy_init();
  const string url_params = this->url(curl, output, parameter);
  const string full_url = HOST + uri + "?" + url_params;

  curl_easy_setopt(curl, CURLOPT_URL, full_url.c_str());
  curl_easy_setopt(curl, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4);
  curl_easy_setopt(curl, CURLOPT_TIMEOUT, (long)timeout);
  curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);

  long httpCode(0);
  string httpData;
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, callback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &httpData);

  // execute search
  curl_easy_perform(curl);
  curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
  curl_easy_cleanup(curl);

  GetResponse gr;
  gr.httpCode = httpCode;
  gr.payload = httpData;
  return gr;
}

} // namespace serpapi
