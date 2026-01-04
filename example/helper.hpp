#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <curl/curl.h>

#include "rapidjson/document.h"

void info(const std::string& msg); 
void info(double msg);
void info(const rapidjson::Document& document);
