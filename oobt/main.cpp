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

