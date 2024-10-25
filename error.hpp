#ifndef ERROR_HPP
#define ERROR_HPP

#include <string>

namespace asyncio {
     class error : std::exception {
        friend class socket;
    public:

        const char* what() {
            return message.c_str();
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

        static void error_callback(asyncio::error error, int nbytes) {
            std::cout << error.what() << std::endl;
        }

        static error placeholder() {
            return error();
        }
        

    private:
        std::string message = "";    
    };
}

#endif 