/***
 * Query string construction (internal, not installed)
 */
#ifndef SERPAPI_QUERY_HPP
#define SERPAPI_QUERY_HPP

#include <map>
#include <string>

namespace serpapi {

using Parameters = std::map<std::string, std::string>;

/**
 * @brief Percent-encodes a string per RFC 3986.
 *
 * Every byte outside the unreserved set (ALPHA / DIGIT / "-" / "." / "_" / "~")
 * is encoded as %XX using uppercase hex digits.
 */
std::string percentEncode(const std::string &value);

/**
 * @brief Builds the query string for a request, without the leading '?'.
 *
 * Parameters are merged with call-specific values taking precedence over the
 * client defaults. The reserved "output" and "source" keys are always set by
 * the library and cannot be overridden by the caller.
 *
 * @param defaults Client-wide default parameters (e.g. api_key, engine).
 * @param parameter Call-specific parameters; win over defaults on conflict.
 * @param output Desired response format (e.g. "json", "html", "md").
 * @param source Value for the source tag identifying this library.
 */
std::string buildQuery(const Parameters &defaults, const Parameters &parameter,
                       const std::string &output, const std::string &source);

} // namespace serpapi

#endif
