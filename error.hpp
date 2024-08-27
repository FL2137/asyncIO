#ifndef ERROR_HPP
#define ERROR_HPP

#include <string>

namespace asyncio {
     class error : std::exception {
        friend class socket;
    public:

        const char* what() {
            return "what?";
        }

        void set_error_message(const char* new_message) {
            this->message = std::string(new_message);
        }

        void set_error_message(std::string new_message) {
            this->message = new_message;
        }

        bool isError() {
            return !message.empty();
        }

    private:
        std::string message = "";    
    };
}

#endif 