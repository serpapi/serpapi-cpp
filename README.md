<div align="center">
<h1 align="center">SerpApi C++ Library</h1>

[![Gem Version](https://badge.fury.io/rb/serpapi.svg)](https://badge.fury.io/rb/serpapi) [![serpapi-C++](https://github.com/serpapi/serpapi-C++/actions/workflows/ci.yml/badge.svg)](https://github.com/serpapi/serpapi-C++/actions/workflows/ci.yml)  [![serpapi-C++-alternative](https://github.com/serpapi/serpapi-C++/actions/workflows/sanity_alt.yml/badge.svg)](https://github.com/serpapi/serpapi-C++/actions/workflows/sanity_alt.yml) [![serpapi-C++-sanity-1](https://github.com/serpapi/serpapi-C++/actions/workflows/sanity_1.yml/badge.svg)](https://github.com/serpapi/serpapi-C++/actions/workflows/sanity_1.yml) [![serpapi-C++-sanity-2](https://github.com/serpapi/serpapi-C++/actions/workflows/sanity_2.yml/badge.svg)](https://github.com/serpapi/serpapi-C++/actions/workflows/sanity_2.yml)
</div>

Integrate search data into your C++ application. This library is the official wrapper for SerpApi (https://serpapi.com).

SerpApi supports Google, Google Maps, Google Shopping, Baidu, Yandex, Yahoo, eBay, App Stores, and more.

## Installation

C++ and meson are required.

### Meson
```meson
```

[C++](https://C++gems.org/gems/serpapi/)

## Simple Usage

```cpp
std::string apiKey(env_p);
std::map<string, string> default_parameter;
default_parameter["api_key"] = apiKey;
default_parameter["engine"] = "google";

// using namespace serpapi;
serpapi::Client client(default_parameter);

// execute search 
map<string, string> parameter;
parameter["q"] = "coffee";
parameter["location"] = "Austin,TX";

Document d = client.search(parameter);

StringBuffer buffer;
PrettyWriter<StringBuffer> writer(buffer);
document.Accept(writer);
cout << "\nINFO: " << buffer.GetString();
```

This example runs a search for "coffee" on Google. It then returns the results as a regular C++ Hash. See the [playground](https://serpapi.com/playground) to generate your own code.

## Advanced Usage
### Search API
```cpp
#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <curl/curl.h>
#include <serpapi.hpp>

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/pointer.h"

using namespace rapidjson;
using namespace std;

// serpapi client created with default parameters
client = SerpApi::Client.new(api_key: "secret_key", engine: "google")

// We recommend that you keep your keys safe.
// At least, don't commit them in plain text.
// More about configuration via environment variables: 
// https://hackernoon.com/all-the-secrets-of-encrypting-api-keys-in-C++-revealed-5qf3t5l

// search query overview (more fields available depending on search engine)
params = {
  // select the search engine (full list: https://serpapi.com/)
  engine: "google",
  // actual search query
  q: "Coffee",
  // then adds search engine specific options.
  // for example: google specific parameters: https://serpapi.com/search-api
  google_domain: "Google Domain",
  location: "Location Requested", // example: Portland,Oregon,United States [see: Location API](#Location-API)
  device: "desktop|mobile|tablet",
  hl: "Google UI Language",
  gl: "Google Country",
  safe: "Safe Search Flag",
  num: "Number of Results",
  start: "Pagination Offset",
  tbm: "nws|isch|shop",
  tbs: "custom to be client criteria",
  // tweak HTTP client behavior
  async: false, // true when async call enabled.
  timeout: 60, // HTTP timeout in seconds on the client side only.
}

// formated search results as a Hash
//  serpapi.com converts HTML -> JSON 
results = client.search(params)

// raw search engine html as a String
//  serpapi.com acts a proxy to provive high throughputs, no search limit and more.
raw_html = client.html(parameter)
```

[Google search documentation](https://serpapi.com/search-api).
More hands on examples are available below.

#### Documentations

 * [API documentation](https://C++doc.info/github/serpapi/serpapi-C++/master)
 * [Full documentation on SerpApi.com](https://serpapi.com)
 * [Library Github page](https://github.com/serpapi/serpapi-C++)
 * [Library GEM page](https://C++gems.org/gems/serpapi/)
 * [API health status](https://serpapi.com/status)

### Location API

```cpp
std::map<string, string> default_parameter = {};
serpapi::Client client(default_parameter);
map<string, string> parameter;
parameter["limit"] = "3";
parameter["q"] = "Austin";

rapidjson::Document doc = client.location(parameter);
const rapidjson::Value& list = doc.GetArray();
// list of city matching with Austin.
```

it prints the first 3 locations matching Austin (Texas, Texas, Rochester)
```cpp
[{
  :id=>"585069bdee19ad271e9bc072",
  :google_id=>200635,
  :google_parent_id=>21176,
  :name=>"Austin, TX",
  :canonical_name=>"Austin,TX,Texas,United States",
  :country_code=>"US",
  :target_type=>"DMA Region",
  :reach=>5560000,
  :gps=>[-97.7430608, 30.267153],
  :keys=>["austin", "tx", "texas", "united", "states"]
  }
  // ...
]
```

NOTE: api_key is not required for this endpoint.

### Search Archive API

This API allows retrieving previous search results.
To fetch earlier results from the search_id.

First, you need to run a search and save the search id.
```cpp
require 'serpapi'
client = SerpApi::Client.new(api_key: 'secret_api_key', engine: 'google')
results = client.search(q: "Coffee", location: "Portland")
search_id = results[:search_metadata][:id]
```

Now let's retrieve the previous search results from the archive.

```cpp
require 'serpapi'
client = SerpApi::Client.new(api_key: 'secret_api_key')
results = client.search_archive(search_id)
pp results
```

This code prints the search results from the archive. :)

### Account API
```cpp
require 'serpapi'
client = SerpApi::Client.new(api_key: 'secret_api_key')
pp client.account
```

It prints your account information.

## Migration quick guide

if you were already using (google-search-results-C++ gem)[https://github.com/serpapi/google-search-results-C++], here are the changes.

```
// load library
// old way 
require 'google_search_results'
// new way
require 'serpapi'

// define a search
// old way to describe the search
search = GoogleSearch.new(search_params)
// new way 
default_parameter = {api_key: "secret_key", engine: "google"}
client = SerpApi::Client.new(default_parameter)
// an instance of the serpapi client is created
// where the default parameters are stored in the client.
//   like api_key, engine
//  then each subsequent API call can be made with additional parameters.

// override an existing parameter
// old way
search.params[:location] = "Portland,Oregon,United States"
// new way
// just provided the search call with the parameters.
results = client.search({location: "Portland,Oregon,United States", q: "Coffe"})

// search format return as raw html
// old way
html_results = search.get_html
// new way
raw_html = client.html(params)
// where params is Hash containing additional key / value

// search format returns a Hash
// old way
hash_results = search.get_hash
// new way
results = client.search(params)
// where params is the search parameters (override the default search parameters in the constructor). 

// search as raw JSON format
// old way
json_results = search.get_json
// new way
results = client.search(params)

// The prefix get_ is removed from all other methods.
//  Because it's evident that a method returns something.
// old -> new way
search.get_search_archive -> client.search_archive
search.get_account -> client.account
search.get_location -> client.location
```

Most notable improvements:
 - Removing parameters check on the client side. (most of the bugs)
 - Reduce logic complexity in our implementation. (faster performance)
 - Better documentation.

## Advanced search API usage
### Highly scalable batching

Search API features non-blocking search using the option: `async=true`.
 - Non-blocking - async=true - a single parent process can handle unlimited concurrent searches.
 - Blocking - async=false - many processes must be forked and synchronized to handle concurrent searches. This strategy is I/O usage because each client would hold a network connection.

Search API enables `async` search.
 - Non-blocking (`async=true`) : the development is more complex, but this allows handling many simultaneous connections.
 - Blocking (`async=false`) : it's easy to write the code but more compute-intensive when the parent process needs to hold many connections.

Here is an example of asynchronous searches using C++ 
```C++
require 'serpapi'
#include <iostream>
#include <memory>
#include <string>
#include <curl/curl.h>
#include <serpapi.hpp>

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/pointer.h"

using namespace rapidjson;
using namespace std;

void info(string msg) {
    cout << "\nINFO: " << msg;
}

void info(double msg) {
    cout << "\nINFO: " << msg;
}

void info(const Document& document) {
    StringBuffer buffer;
    PrettyWriter<StringBuffer> writer(buffer);
    document.Accept(writer);
    cout << "\nINFO: " << buffer.GetString();
}

// RapidJSON parser documentation is available: https://rapidjson.org
int main()
{
    // initialize a client
    const char* env_p = std::getenv("API_KEY");
    std::string apiKey(env_p);
    std::map<string, string> default_parameter;
    default_parameter["api_key"] = apiKey;
    default_parameter["engine"] = "google";
    
    // using namespace serpapi;
    serpapi::Client client(default_parameter);

    // execute search 
    map<string, string> parameter;
    parameter["q"] = "coffee";
    parameter["location"] = "Austin,TX";

    //  using namespace rapidjson;
    Document d = client.search(parameter);
    info("document loaded");
    info(d);
    info("check content");

    assert(!d.HasMember("error"));
    assert(d.HasMember("search_metadata"));
    assert(d["search_metadata"]["status"] == "Success");
    info(" search_metadata:");
    string status = d["search_metadata"]["status"].GetString();
    info("   status: " + status);
    assert(d["search_metadata"]["id"].IsString());
    string id = d["search_metadata"]["id"].GetString();
    info("   id: " + id);

    info("search archive with id: " + id);
    client.searchArchive(id);
    assert(d["search_metadata"]["status"] == "Success");
    info(" search found in archive.");
    info(" test passed.\n");
    return 0;
}


```

 * source code: [oobt/main.cpp](https://github.com/serpapi/serpapi-C++/blob/master/oobt/main.cpp)

This code shows a simple solution to batch searches asynchronously into a [queue](https://en.wikipedia.org/wiki/Queue_(abstract_data_type)). 
Each search takes a few seconds before completion by SerpApi service and the search engine. By the time the first element pops out of the queue. The search result might be already available in the archive. If not, the `search_archive` method blocks until the search results are available. 

## Supported C++ version.
C++ versions validated by Github Actions:
 - 3.1
 - 2.6
see: [Github Actions.](https://github.com/serpapi/serpapi-C++/actions/workflows/ci.yml)

## Change logs
 * [2023-02-20] 1.0.0 Full API support

## Developer Guide
### Key goals
 - Brand centric instead of search engine based
   - No hard-coded logic per search engine
 - Simple HTTP client (lightweight, reduced dependency)
   - No magic default values
   - Thread safe
 - Easy extension
 - Defensive code style (raise a custom exception)
 - TDD
 - Best API coding practice per platform
 - KiSS principles

### Inspirations
This project source code and coding style was inspired by the most awesome C++ Gems:
 ..
 
### Code quality expectations
 - 0 lint offense: `rake lint`
 - 100% tests passing: `rake test`
 - 100% code coverage: `rake test` (simple-cov)

# Developer Guide
## Design : UML diagram
### Class diagram
```mermaid
classDiagram
  Application *-- serpapi 
  serpapi *-- Client
  class Client {
    engine String
    api_key String
    params Hash
    search() Hash
    html() String
    location() String
    search_archive() Hash
    account() Hash
  }
  openuri <.. Client
  json <.. Client
  C++ <.. openuri
  C++ <.. json
```
### search() : Sequence diagram
```mermaid
sequenceDiagram
    Client->>SerpApi.com: search() : http request 
    SerpApi.com-->>SerpApi.com: query search engine
    SerpApi.com-->>SerpApi.com: parse HTML into JSON
    SerpApi.com-->>Client: JSON string payload
    Client-->>Client: decode JSON into Hash
```
where:
  - The end user implements the application.
  - Client refers to SerpApi:Client.
  - SerpApi.com is the backend HTTP / REST service.
  - Engine refers to Google, Baidu, Bing, and more.

The SerpApi.com service (backend)
 - executes a scalable search on `engine: "google"` using the search query: `q: "coffee"`.
 - parses the messy HTML responses from Google on the backend.
 - returns a standardized JSON response.
The class SerpApi::Client (client side / C++):
 - Format the request to SerpApi.com server.
 - Execute HTTP Get request.
 - Parse JSON into C++ Hash using a standard JSON library.
Et voila!

## Continuous integration
We love "true open source" and "continuous integration", and Test Drive Development (TDD).
 We are using RSpec to test [our infrastructure around the clock]) using Github Action to achieve the best QoS (Quality Of Service).

The directory spec/ includes specification which serves the dual purposes of examples and functional tests.

Set your secret API key in your shell before running a test.
```bash
export API_KEY="your_secret_key"
```
Install testing dependency
```bash
$ bundle install
// or
$ rake dependency
```

Check code quality using Lint.
```bash
$ rake lint
```

Run regression.
```bash
$ rake test
```

To flush the flow.
```bash
$ rake
```

Open coverage report generated by `rake test`
```sh
open coverage/index.html
```

Open ./Rakefile for more information.

Contributions are welcome. Feel to submit a pull request!

// TODO
 - [] Release version 1.0.0


## Run with Docker

```bash
env API_KEY=API_KEY docker run --rm -it -e API_KEY --workdir /tmp/serpapi -v $PWD:/tmp/serpapi conanio/gcc10 make install_linux reset all
```

This will use https://hub.docker.com/layers/conanio/gcc10 image instead
of creating our own image for compilation on Linux.

## Get started.
 - Set an environment API_KEY=<secret>
 - Install dependency either apple or linux tested.
    - `rake install_apple`
    - `rake install_linux`
- Initialize the project: `rake reset`
- Compile `rake` or `rake build run`
- Edit main.cpp for testing.

#### To modify the *Search classes
 - edit sooglesearch.cpp/hpp
 - update all files using  `rake`

## TODO

 - [ ] Add unit test
 - [ ] Customize classes
 - [ ] Package as a library 
 - [ ] Create OOBT test
 - [ ] Publish library online
 - [ ] Add REPL.it example
 - [ ] Add integration documentation on serpapi.com
 - [ ] Write a full README to match  serpapi-search-results-dotnet

# Documentation
https://formulae.brew.sh/formula/googletest
https://rapidjson.org/md_doc_tutorial.html


# Links

https://github.com/mesonbuild/meson/blob/master/test%20cases/frameworks/2%20gtest/test_nomain.cc