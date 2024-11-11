#pragma once

#include <string>
#include "json.hpp"


namespace asyncio {
namespace http {

enum METHOD {
    GET = 0,
    POST,
    DELETE,

};

class request {

public:

    request(const std::string &request_string){
        std::string line;
    }



private:
    nlohmann::json json;

};

class response {

};

class parser {

    bool parse(const std::string &message, std::string &result) {

    }

};

} 
}