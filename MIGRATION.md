# Migrating from 0.5 to 1.0

1.0 is not source- or binary-compatible with 0.5. Rebuild your application
against the new header and library; a binary built against 0.5 will fail to
load against 1.0 with undefined symbols.

1.0 is the first stable release. From here on, the project follows
[Semantic Versioning](https://semver.org): 1.x releases add features and fix
bugs without breaking source or binary compatibility, and any breaking change
waits for 2.0. The shared library carries the ABI version in its file name
(`libserpapi.so.1` on Linux, `libserpapi.1.dylib` on macOS), so a future
incompatible library is a different file rather than a load-time failure.

## 1. Errors now throw `serpapi::Error`

In 0.5, errors were returned as data that looked like a normal response. In
1.0, every response that carries an error throws `serpapi::Error`, whatever
its HTTP status. The exception carries `cause()` (SerpApi's message),
`http_code()` and `payload()`.

| Situation | 0.5 | 1.0 |
|-----------|-----|-----|
| HTTP error from SerpApi (invalid `api_key`, bad parameter, plan exhausted, ...) | `Document` containing the server's `"error"` | throws; `cause()` is the server's message, `http_code()` the status (e.g. 401) |
| Search succeeded but found no results (HTTP 200) | `Document` containing `"error"` | `search()`, `markdown()`: throws; `http_code()` is 200<br>`html()`: returns Google's page (see [Error handling](README.md#error-handling)) |
| Network failure or timeout | `search()` etc.: `{"error": "JSON parse error"}`<br>`html()`/`markdown()`: the string `"CURL error: ..."` | throws; `http_code()` is 0, `cause()` is `"CURL error: ..."` |
| Response body is not valid JSON | `{"error": "JSON parse error"}` | throws; `cause()` is `"JSON parse error: ..."`, `payload()` is the raw body |

Before (0.5):

```cpp
rapidjson::Document d = client.search(params);
if (d.HasMember("error")) {
  std::cerr << d["error"].GetString() << std::endl;
  return 1;
}
```

After (1.0):

```cpp
try {
  rapidjson::Document d = client.search(params);
  // d never contains "error" here
} catch (const serpapi::Error &e) {
  std::cerr << e.cause() << " (HTTP " << e.http_code() << ")" << std::endl;
  return 1;
}
```

Remove your `HasMember("error")` checks: a returned `Document` never contains a
top-level `"error"` member. If you treated "no results" as a normal, empty
outcome, catch it explicitly. It is the `serpapi::Error` with `http_code() == 200`.

An uncaught exception terminates the program, so add a `try`/`catch` around
every call, or rely on an existing handler. `serpapi::Error` derives from
`std::runtime_error`, so `catch (const std::exception &e)` also catches it,
and `e.what()` returns the same text as `cause()`.

For `html()` and `markdown()`, delete any checks for a `"CURL error:"` prefix
or a JSON error body in the returned string. If the call returns, the string
holds the page content.

## 2. Per-call parameters override client defaults

In 0.5, a key set both in the `Client` defaults and in a call was sent twice
(e.g. `engine=google&engine=bing`), and you could not control which value
SerpApi used. In 1.0, the per-call value wins and each key is sent once.

```cpp
serpapi::Client client({{"api_key", key}, {"engine", "google"}});
client.search({{"engine", "bing"}, {"q", "coffee"}});  // 1.0: searches Bing
```

`output` and `source` are set by the library. Values you pass for them are
ignored; 0.5 sent them as duplicates instead.

## 3. `<serpapi.hpp>` no longer includes `<curl/curl.h>`

If your code calls libcurl directly and relied on getting its header through
`<serpapi.hpp>`, add `#include <curl/curl.h>` yourself. Compiling against
`<serpapi.hpp>` no longer requires the libcurl development headers.

## 4. Removed and renamed symbols

| 0.5 | 1.0 |
|-----|-----|
| `Client::setTimeout(int)` | Renamed to `Client::set_timeout(int)`; the public API uses snake_case throughout. |
| `serpapi::GetResponse` | Removed. Use `serpapi::Error::http_code()` and `payload()`. |
| `<serpapi/callback.hpp>` and `serpapi::callback()` | No longer installed. It was the internal libcurl write callback; there is no replacement. |
| Include guard macro `SERPAPI` | Renamed to `SERPAPI_HPP`. This only matters if your code tested `#ifdef SERPAPI`. |

## 5. RapidJSON is bundled

1.0 builds and installs a pinned, CVE-patched RapidJSON under
`<prefix>/include/serpapi/rapidjson/`, and `pkg-config --cflags serpapi`
puts it on your include path, so `#include <rapidjson/document.h>` in your
code resolves to the same copy the library was built with. You no longer need
`rapidjson-dev` / `brew install rapidjson`. If you package serpapi for a
distribution, use `-Dsystem_rapidjson=true` to keep the previous behaviour.

## 6. Additions (no action required)

 - `serpapi::Parameters` is an alias for `std::map<std::string, std::string>`; existing `std::map` code compiles unchanged.
 - `serpapi::Client client;` constructs a client with no default parameters.
 - `Client` is copyable and movable; copies are independent.
 - Request methods are `const`. They can be called on a `const Client &` and concurrently from several threads on one instance. `set_timeout()` is not synchronized and must not run while requests are in flight.
 - `md()` is an alias for `markdown()`.
 - `search_archive()` percent-encodes the search id. Ids taken from `search_metadata.id` are unaffected.

## Checklist

 - [ ] Rebuild your application against 1.0.
 - [ ] Catch `serpapi::Error` (or `std::exception`) around request calls.
 - [ ] Remove `HasMember("error")` checks on results; handle "no results" in the `catch` (`http_code() == 200`).
 - [ ] Remove error-string checks on `html()` / `markdown()` results.
 - [ ] Check calls that pass a key also set in the `Client` defaults; the per-call value now wins.
 - [ ] Add `#include <curl/curl.h>` if you use libcurl directly.
 - [ ] Rename `setTimeout(` to `set_timeout(`.
 - [ ] Replace any use of `serpapi::GetResponse` and remove `#include <serpapi/callback.hpp>`.
 - [ ] If you link by file name, use `libserpapi.so.1` / `libserpapi.1.dylib` (or link with `-lserpapi`).
