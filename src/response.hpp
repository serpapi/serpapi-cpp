/***
 * Detection of SerpApi errors in response bodies (internal, not installed)
 */
#ifndef SERPAPI_RESPONSE_HPP
#define SERPAPI_RESPONSE_HPP

#include "rapidjson/document.h"
#include <optional>
#include <string>

namespace serpapi {

/**
 * @brief Returns the message of a top-level "error" member, if present.
 *
 * SerpApi reports errors as {"error": "..."}, both with HTTP error statuses
 * and with HTTP 200 (e.g. a search that returned no results).
 */
std::optional<std::string> jsonError(const rapidjson::Value &value);

/// Parses body as JSON and returns its top-level "error" message, if any.
std::optional<std::string> bodyError(const std::string &body);

/**
 * @brief Returns the text of the "## Error" section of a Markdown response, if present.
 *
 * The Markdown output (output=md) renders the JSON "error" member as a level-2
 * "Error" section; normal result sections never use that heading.
 */
std::optional<std::string> markdownError(const std::string &body);

} // namespace serpapi

#endif
