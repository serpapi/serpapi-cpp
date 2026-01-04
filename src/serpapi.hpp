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
    int timeout = 60;
    
    public:
    explicit Client(const std::map<std::string, std::string>& parameter);
    ~Client();

    Document search(const std::map<std::string, std::string>& parameter = {});

    std::string html(const std::map<std::string, std::string>& parameter = {});

    Document search_archive(const std::string& search_id);

    Document account(const std::map<std::string, std::string>& parameter = {});

    Document location(const std::map<std::string, std::string>& parameter = {});

    private:
    Document json(const std::string& uri, const std::map<std::string, std::string>& parameter);

    std::string url(CURL* curl, const std::string& output, const std::map<std::string, std::string>& parameter);

    GetResponse get(const std::string& uri, const std::string& output, const std::map<std::string, std::string>& parameter);
};
}

#endif
