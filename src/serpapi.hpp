/***
 * Run search engine powered by SerpApi
 */
#ifndef SERPAPI
#define SERPAPI

#include <iostream>
#include <map>
#include <string>
#include <curl/curl.h>
#include "./callback.hpp"
#include "rapidjson/document.h"

namespace serpapi 
{
using namespace rapidjson;
using namespace std;

struct GetResponse;

class Client {

    map<string, string> parameter;
    CURL *curl;
    int timeout = 60;
    
    public:

    Client(map<string, string> parameter);

    Document search(map<string, string> parameter);

    string html(map<string, string> parameter);

    Document searchArchive(string searchID);

    Document account(map<string, string> parameter);

    Document location(map<string, string> parameter);

    Document json(string uri, map<string, string> parameter);

    string url(string output, map<string, string> parameter);

    GetResponse get(string uri, string output, map<string, string> parameter);
};
}

#endif
