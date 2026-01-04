#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <curl/curl.h>

#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/prettywriter.h"

using namespace std;
using namespace rapidjson;

void info(const string& msg) {
    cout << "\nINFO: " << msg << endl;
}

void info(double msg) {
    cout << "\nINFO: " << msg << endl;
}

void info(const Document& document) {
    StringBuffer buffer;
    PrettyWriter<StringBuffer> writer(buffer);
    document.Accept(writer);
    cout << "\nINFO: " << buffer.GetString();
}