#include <queue>
#include <mutex>
#include <condition_variable>

template<typename T>
class thread_queue {
private:
    std::queue<T> queue;

    std::mutex mutex;
    std::condition_variable condvar;

public:
    void push(T val) {
        std::unique_lock<std::mutex> lock(mutex);
        queue.push(val);

        condvar.notify_one();
    }

    int size() {
        return queue.size();
    }

    //blocking function
    void wait() {
        std::unique_lock<std::mutex> lock(mutex);
        condvar.wait(lock, [&] {
            return queue.size() != 0;
        });
    }

    T pop() {
        std::unique_lock<std::mutex> lock(mutex);

        T item = queue.front();
        queue.pop();

        return item;
    }
 
};
