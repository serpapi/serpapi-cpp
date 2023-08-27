#include <gtest/gtest.h>
#include "../src/callback.hpp"
#include "../src/serpapi.hpp"

TEST(client, search) {
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

    rapidjson::Document d = client.search(parameter);

    ASSERT_FALSE(d.HasMember("error"));
    ASSERT_TRUE(d.HasMember("search_metadata"));
    ASSERT_EQ(d["search_metadata"]["status"], "Success");
}

TEST(client, location) {
    std::map<string, string> default_parameter = {};
    serpapi::Client client(default_parameter);

    map<string, string> parameter;
    parameter["limit"] = "3";
    parameter["q"] = "Austin";
    rapidjson::Document doc = client.location(parameter);
    const rapidjson::Value& list = doc.GetArray();
    size_t arraySize = list.Size();
    ASSERT_TRUE(arraySize > 1);
}

// fetch account information
TEST(client, account) {
    const char* env_p = std::getenv("API_KEY");
    std::string apiKey(env_p);
    std::map<string, string> default_parameter;
    default_parameter["api_key"] = apiKey;
    serpapi::Client client(default_parameter);

    map<string, string> parameter = {};
    rapidjson::Document d = client.account(parameter);

    ASSERT_FALSE(d.HasMember("error"));
    ASSERT_TRUE(d.HasMember("account_id"));
}

TEST(client, searchArchive) {
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

    rapidjson::Document d = client.search(parameter);

    ASSERT_FALSE(d.HasMember("error"));
    ASSERT_TRUE(d.HasMember("search_metadata"));
    ASSERT_TRUE(d["search_metadata"].HasMember("id"));
    ASSERT_EQ(d["search_metadata"]["status"], "Success");

    ASSERT_TRUE(d["search_metadata"]["id"].IsString());

    std::string id = d["search_metadata"]["id"].GetString();
    rapidjson::Document d2 = client.searchArchive(id);

    ASSERT_FALSE(d2.HasMember("error"));
    ASSERT_TRUE(d2.HasMember("search_metadata"));
    ASSERT_TRUE(d2["search_metadata"].HasMember("id"));
    std::string id_ = d2["search_metadata"]["id"].GetString();
    ASSERT_EQ(id_, id);

}
