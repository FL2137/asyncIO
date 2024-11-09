#pragma once

#include <string>
#include "json.hpp"


namespace asyncio {
namespace HTTP {

enum METHOD {
    GET = 0,
    POST,
    DELETE,

};

class request {




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