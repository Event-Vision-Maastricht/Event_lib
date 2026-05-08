#pragma once

#include <deque>
#include <string>
#include <vector>
#include <fstream>
#include <cstring>
#include <mutex>
#include <condition_variable>

#include "event_lib/core/event.hpp"
#include "event_lib/processing/Frame.hpp"

namespace event_lib{

    // Frame Packet: FIFO container for completed frames
    struct FrameQueue {
        std::deque<FrameStr> frames;
        int width;
        int height;
        long start_timestamp;
        long end_timestamp;
        bool closed;

        FrameQueue();
        FrameQueue(int w, int h);

        // Non-copyable, movable
        FrameQueue(const FrameQueue& other) = delete;
        FrameQueue& operator=(const FrameQueue& other) = delete;
        FrameQueue(FrameQueue&& other) noexcept;
        FrameQueue& operator=(FrameQueue&& other) noexcept;

        // Add a completed frame to the queue (by-value so we can move)
        void add_frame(FrameStr frame);

        // Get current frame count
        size_t frame_count() const;
        bool is_empty() const ;
        void close() ;
        void clear() ;
        bool try_pop_frame(FrameStr& frame);
        bool wait_and_pop_frame(FrameStr& frame);

    private:
        mutable std::mutex mutex_;
        std::condition_variable cv_;
    };
}