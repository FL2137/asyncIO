#pragma once

#include <thread>
#include <mutex>
#include <shared_mutex>
#include <condition_variable>

namespace asyncio {
    
    class buffer {
        public:
            buffer(char *ptr) {

            }
        


        private:

            std::shared_mutex mutex;

    };

} // namespace asyncio