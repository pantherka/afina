#include <afina/concurrency/Executor.h>
#include <iostream>

namespace Afina {
namespace Concurrency {

void perform(Executor *executor) {
    std::unique_lock<std::mutex> lock(executor->mutex);
    executor->_thread_count++;
    while (executor->state != Executor::State::kStopped) {
        if (executor->tasks.empty()) {
            if (executor->state == Executor::State::kStopping) {
                executor->state = Executor::State::kStopped;
            }
            auto responce = executor->empty_condition.wait_for(lock, std::chrono::milliseconds(executor->_idle_time));
            if (responce == std::cv_status::timeout) {
                if (executor->tasks.size() > executor->_low_watermark) {
                    break;
                }
                else {
                    continue;
                }
            }
        }
        if (executor->tasks.empty()) {
            continue;
        }

        auto task = executor->tasks.front();
        executor->tasks.pop_front();
        lock.unlock();
        task();
    }
    executor->_thread_count--;
    if (executor->_thread_count == 0) {
        executor->last_thread.notify_all();
    }
}

Executor::Executor(size_t low_watermark, size_t high_watermark, size_t max_queue_size, size_t idle_time) :
                    _low_watermark(low_watermark), _high_watermark(high_watermark), _max_queue_size(max_queue_size), _idle_time(idle_time) {
    
    for (size_t i = 0; i < _low_watermark; ++i) {
        std::thread(&perform, this).detach();
    }
}

void Executor::Stop(bool await) {
    std::unique_lock<std::mutex> lock(mutex);
    if (!await) {
        state = State::kStopped;
        return;
    }
    else {
        state = State::kStopping;
    }
    empty_condition.notify_all();
    while(_thread_count > 0) {
        last_thread.wait(lock);
    }
}

Executor::~Executor() {
    Stop(true);
}

}
} // namespace Afina
