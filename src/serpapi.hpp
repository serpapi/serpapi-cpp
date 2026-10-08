/***
 * Run search engine powered by SerpApi
 */
#ifndef SERPAPI_HPP
#define SERPAPI_HPP

#include "rapidjson/document.h"
#include <map>
#include <memory>
#include <stdexcept>
#include <string>

namespace serpapi {

/// Request parameters as key/value pairs (e.g. {"q", "coffee"}).
using Parameters = std::map<std::string, std::string>;

/**
 * @brief Raised when a request fails or SerpApi reports an error.
 *
 * Thrown whenever the response carries a SerpApi error, regardless of the HTTP
 * status: an HTTP error status (>= 400), or an HTTP 200 response whose body
 * contains an error (e.g. a search that returned no results). Also thrown for
 * client-side failures: transport error, CURL initialization, unparsable JSON.
 *
 * For server-reported errors, cause() is SerpApi's message (e.g. "Invalid API
 * key. ..."), http_code() the HTTP status and payload() the raw response body.
 * For client-side failures, http_code() is 0 and payload() is empty.
 *
 * cause() is the preferred accessor for the failure description; what() returns
 * the same text and remains available for generic std::exception handlers.
 */
class Error : public std::runtime_error {
public:
  explicit Error(const std::string &message, long http_code = 0, std::string payload = "");

  /// Failure description: SerpApi's error message, or e.g. "CURL error: ..." for client-side failures.
  const std::string &cause() const noexcept;

  /// HTTP status code, or 0 when the failure happened before a response was received.
  long http_code() const noexcept;

  /// Raw response body, empty when no response was received.
  const std::string &payload() const noexcept;

private:
  // Fields live behind a pointer so they can change without breaking the ABI;
  // shared ownership keeps copying the exception nothrow, as std::exception requires.
  struct Detail;
  std::shared_ptr<const Detail> detail_;
};

/**
 * @brief Client for the SerpApi HTTP API.
 *
 * Default parameters supplied at construction (e.g. api_key, engine) are merged
 * into every request; parameters passed to an individual call take precedence.
 *
 * All request methods are const and may be called concurrently from multiple
 * threads on the same instance. set_timeout() is not synchronized and must not
 * race with in-flight requests.
 *
 * Client is copyable and movable. A moved-from Client may only be assigned to
 * or destroyed.
 */
class Client {
public:
  /**
   * @brief Constructs a client with default parameters merged into every request.
   * @param parameter Default parameters (e.g. api_key, engine) applied to all calls.
   */
  explicit Client(Parameters parameter = {});
  ~Client();
  Client(const Client &other);
  Client &operator=(const Client &other);
  Client(Client &&other) noexcept;
  Client &operator=(Client &&other) noexcept;

  /// Sets the per-request timeout in seconds (default 60).
  void set_timeout(int seconds);

  /**
   * @brief Runs a search and returns the parsed/structured results as JSON.
   * @param parameter Search parameters (e.g. q, location) merged with the client defaults.
   * @return Parsed JSON response.
   * @throws Error on transport failure, unparsable JSON, or when the response
   *         contains an "error" member (any HTTP status).
   */
  rapidjson::Document search(const Parameters &parameter = {}) const;

  /**
   * @brief Runs a search and returns the raw search engine result page as HTML.
   * @param parameter Search parameters (e.g. q, location) merged with the client defaults.
   * @return Raw HTML response body.
   * @throws Error on transport failure or HTTP error status. A search that
   *         succeeds with no results returns HTTP 200 with Google's own page,
   *         which carries no SerpApi error, so it is returned rather than thrown.
   */
  std::string html(const Parameters &parameter = {}) const;

  /**
   * @brief Runs a search and returns the search engine result page as Markdown.
   * @param parameter Search parameters (e.g. q, location) merged with the client defaults.
   * @return Raw Markdown response body.
   * @throws Error on transport failure, HTTP error status, or when the response
   *         contains an "## Error" section (any HTTP status).
   */
  std::string markdown(const Parameters &parameter = {}) const;

  /// Alias for markdown().
  std::string md(const Parameters &parameter = {}) const { return markdown(parameter); }

  /**
   * @brief Retrieves a previously run search from the search archive.
   * @param search_id Search id, as returned in search_metadata.id from a prior search().
   * @return Parsed JSON response.
   * @throws Error on transport failure, unparsable JSON, or when the response
   *         contains an "error" member (any HTTP status).
   */
  rapidjson::Document search_archive(const std::string &search_id) const;

  /**
   * @brief Retrieves account information (e.g. plan, usage) for the given api_key.
   * @param parameter Request parameters (e.g. api_key) merged with the client defaults.
   * @return Parsed JSON response.
   * @throws Error on transport failure, unparsable JSON, or when the response
   *         contains an "error" member (any HTTP status).
   */
  rapidjson::Document account(const Parameters &parameter = {}) const;

  /**
   * @brief Looks up supported locations matching a query. No api_key required.
   * @param parameter Request parameters (e.g. q, limit) merged with the client defaults.
   * @return Parsed JSON response (an array of locations).
   * @throws Error on transport failure, unparsable JSON, or when the response
   *         contains an "error" member (any HTTP status).
   */
  rapidjson::Document location(const Parameters &parameter = {}) const;

private:
  // State lives behind a pointer so settings can be added without breaking the ABI.
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

} // namespace serpapi

#endif
