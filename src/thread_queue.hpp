#include <queue>
#include <mutex>
#include <condition_variable>

template<typename T>
class thread_queue {
private:
    std::queue<T> queue;

    std::mutex m_mutex;
    std::condition_variable condvar;

public:
    void push(T val) {
        queue.push(val);
    }

    int size() {
        return queue.size();
    }

    
    T pop() {
        T item = queue.front();
        queue.pop();
        return item;
    }
};
