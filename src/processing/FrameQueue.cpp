#include <string>
#include <vector>
#include <fstream>
#include <cstring>

#include "event_lib/core/event.hpp"
#include "event_lib/processing/Frame.hpp"
#include "event_lib/processing/FrameQueue.hpp"

namespace event_lib{

    FrameQueue::FrameQueue()
        : width(0), height(0), start_timestamp(0), end_timestamp(0), closed(false) {}

    FrameQueue::FrameQueue(int w, int h)
        : width(w), height(h), start_timestamp(0), end_timestamp(0), closed(false) {}

    FrameQueue::FrameQueue(FrameQueue&& other) noexcept {
        std::lock_guard<std::mutex> lock(other.mutex_);
        frames = std::move(other.frames);
        width = other.width;
        height = other.height;
        start_timestamp = other.start_timestamp;
        end_timestamp = other.end_timestamp;
        closed = other.closed;
    }

    FrameQueue& FrameQueue::operator=(FrameQueue&& other) noexcept {
        if (this != &other) {
            std::lock_guard<std::mutex> lock_this(mutex_);
            std::lock_guard<std::mutex> lock_other(other.mutex_);
            frames = std::move(other.frames);
            width = other.width;
            height = other.height;
            start_timestamp = other.start_timestamp;
            end_timestamp = other.end_timestamp;
            closed = other.closed;
        }
        return *this;
    }

    void FrameQueue::add_frame(FrameStr frame) {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            frames.push_back(std::move(frame));
            if (frames.size() == 1) {
                start_timestamp = frames.front().timestamp;
            }
            end_timestamp = frames.back().timestamp;
            closed = false;
        }
        cv_.notify_one();
    }

    size_t FrameQueue::frame_count() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return frames.size();
    }

    bool FrameQueue::is_empty() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return frames.empty();
    }

    void FrameQueue::close() {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            closed = true;
        }
        cv_.notify_all();
    }

    void FrameQueue::clear() {
        std::lock_guard<std::mutex> lock(mutex_);
        frames.clear();
        start_timestamp = 0;
        end_timestamp = 0;
        closed = false;
    }

    bool FrameQueue::try_pop_frame(FrameStr& frame) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (frames.empty()) {
            return false;
        }

        frame = std::move(frames.front());
        frames.pop_front();
        return true;
    }

    bool FrameQueue::wait_and_pop_frame(FrameStr& frame) {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock, [this]() {
            return !frames.empty() || closed;
        });

        if (frames.empty()) {
            return false;
        }

        frame = std::move(frames.front());
        frames.pop_front();
        return true;
    }

} // namespace event_lib