#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <curl/curl.h>
#include <serpapi.hpp>
#include <helper.hpp>

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/pointer.h"

using namespace rapidjson;
using namespace std;

// RapidJSON parser documentation is available: https://rapidjson.org
int main()
{
    // initialize a client
    const char* env_p = std::getenv("SERPAPI_KEY");
    if (env_p == nullptr) {
        cout << "SERPAPI_KEY not set, skipping example" << endl;
        return 0;
    }
    std::string apiKey(env_p);
    std::map<string, string> default_parameter;
    default_parameter["engine"] = "google";
    default_parameter["api_key"] = apiKey;
    
    // using namespace serpapi;
    serpapi::Client client(default_parameter);

    // execute search 
    std::map<string, string> parameter;
    parameter["q"] = "coffee";
    parameter["location"] = "Austin,TX";
    
    //  using namespace rapidjson;
    rapidjson::Document d = client.search(parameter);
    info("document loaded");
    assert(d.HasMember("search_metadata"));
    assert(d["search_metadata"]["status"] == "Success");
    assert(d["search_metadata"]["id"].IsString());

    string id = d["search_metadata"]["id"].GetString();
    client.search_archive(id);
    assert(d["search_metadata"]["status"] == "Success");
    info(d);
}
