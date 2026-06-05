#include "event_lib/processing/Frame.hpp"
#include "event_lib/core/sensor_metadata.hpp"
#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <utility>

namespace event_lib {

    Frame::Frame(const SensorMetadata& metadata)
                : metadata_(&metadata),
          total_events_(0),
          on_events_count_(0),
          off_events_count_(0) {
        if (!metadata.is_valid()) throw std::runtime_error("Frame: SensorMetadata must have valid width and height > 0");
        ensure_frame_storage(current_frame_);
        ensure_frame_storage(published_frame_);
        current_frame_.timestamp = 0;
        published_frame_.timestamp = 0;
    }

    Frame::~Frame() {close();}

    void Frame::add_event(const Event& ev) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (metadata_ == nullptr || closed_) return;

        const int x = ev.x;
        const int y = ev.y;
        const bool polarity = ev.polarity;

        if (x < 0 || x >= metadata_->width || y < 0 || y >= metadata_->height) return;

        if (polarity) {
            current_frame_.on_events[y][x]++;
            ++on_events_count_;
        } else {
            current_frame_.off_events[y][x]++;
            ++off_events_count_;
        }
        ++total_events_;
    }

    void Frame::decay_frame(double amount) {
        std::lock_guard<std::mutex> lock(mutex_);
        decay_frame_unlocked(amount);
    }

    void Frame::decay_frame_unlocked(double amount) {
        if (metadata_ == nullptr) return;

        for (int y = 0; y < metadata_->height; ++y) {
            for (int x = 0; x < metadata_->width; ++x) {
                current_frame_.on_events[y][x] = static_cast<int>(
                    current_frame_.on_events[y][x] * amount
                );
                current_frame_.off_events[y][x] = static_cast<int>(
                    current_frame_.off_events[y][x] * amount
                );
            }
        }
    }

    bool Frame::finalize_frame(EventTimestamp timestamp, FrameStr& output_frame) {
        std::lock_guard<std::mutex> lock(mutex_);
        return finalize_frame_unlocked(timestamp, output_frame);
    }

    bool Frame::publish_frame(EventTimestamp timestamp, double decay_amount) {
        std::unique_lock<std::mutex> lock(mutex_);
        publish_slot_free_.wait(lock, [this]() {
            return closed_ || !has_published_frame_;
        });

        if (closed_) return false;

        if (!publish_frame_unlocked(timestamp, published_frame_)) return false;

        has_published_frame_ = true;
        is_dirty_ = true;
        lock.unlock();
        frame_ready_.notify_one();

        decay_frame(decay_amount);
        return true;
    }

    // bool Frame::publish_frame(const FrameStr& frame) {
    //     std::unique_lock<std::mutex> lock(mutex_);
    //     publish_slot_free_.wait(lock, [this]() {
    //         return closed_ || !has_published_frame_;
    //     });

    //     if (closed_ || metadata_ == nullptr) return false;

    //     ensure_frame_storage(published_frame_);
    //     for (int y = 0; y < metadata_->height; ++y) {
    //         std::copy(frame.on_events[y].begin(),
    //                   frame.on_events[y].end(),
    //                   published_frame_.on_events[y].begin());
    //         std::copy(frame.off_events[y].begin(),
    //                   frame.off_events[y].end(),
    //                   published_frame_.off_events[y].begin());
    //     }
    //     published_frame_.timestamp = frame.timestamp;
    //     has_published_frame_ = true;
    //     is_dirty_ = true;
    //     lock.unlock();
    //     frame_ready_.notify_one();

    //     return true;
    // }

    bool Frame::publish_frame_unlocked(EventTimestamp timestamp, FrameStr& output_frame) {
        if (metadata_ == nullptr || total_events_ == 0) {
            return false;
        }

        current_frame_.timestamp = timestamp;
        ensure_frame_storage(output_frame);

        for (int y = 0; y < metadata_->height; ++y) {
            std::copy(current_frame_.on_events[y].begin(),
                      current_frame_.on_events[y].end(),
                      output_frame.on_events[y].begin());
            std::copy(current_frame_.off_events[y].begin(),
                      current_frame_.off_events[y].end(),
                      output_frame.off_events[y].begin());
        }
        output_frame.timestamp = timestamp;

        total_events_ = 0;
        on_events_count_ = 0;
        off_events_count_ = 0;

        return true;
    }

    bool Frame::consume_published_frame(FrameStr& output_frame) {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (!has_published_frame_) return false;

            ensure_frame_storage(output_frame);
            for (int y = 0; y < metadata_->height; ++y) {
                std::copy(published_frame_.on_events[y].begin(),
                          published_frame_.on_events[y].end(),
                          output_frame.on_events[y].begin());
                std::copy(published_frame_.off_events[y].begin(),
                          published_frame_.off_events[y].end(),
                          output_frame.off_events[y].begin());
            }
            output_frame.timestamp = published_frame_.timestamp;
            has_published_frame_ = false;
            is_dirty_ = false;
        }
        publish_slot_free_.notify_one();
        return true;
    }

    bool Frame::wait_for_published_frame(std::chrono::milliseconds timeout) const {
        std::unique_lock<std::mutex> lock(mutex_);
        return frame_ready_.wait_for(lock, timeout, [this]() {
            return closed_ || has_published_frame_;
        });
    }

    void Frame::close() {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            closed_ = true;
        }
        frame_ready_.notify_all();
        publish_slot_free_.notify_all();
    }

    bool Frame::finalize_frame_unlocked(EventTimestamp timestamp, FrameStr& output_frame) {
        if (metadata_ == nullptr || total_events_ == 0) return false;

        current_frame_.timestamp = timestamp;
        ensure_frame_storage(output_frame);

        std::swap(output_frame.on_events, current_frame_.on_events);
        std::swap(output_frame.off_events, current_frame_.off_events);
        output_frame.timestamp = timestamp;

        current_frame_.timestamp = 0;
        for (int y = 0; y < metadata_->height; ++y) {
            std::fill(current_frame_.on_events[y].begin(), current_frame_.on_events[y].end(), 0);
            std::fill(current_frame_.off_events[y].begin(), current_frame_.off_events[y].end(), 0);
        }
        total_events_ = 0;
        on_events_count_ = 0;
        off_events_count_ = 0;

        return true;
    }

    void Frame::reset_frame() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (metadata_ == nullptr) return;

        for (int y = 0; y < metadata_->height; ++y) {
            std::fill(current_frame_.on_events[y].begin(), current_frame_.on_events[y].end(), 0);
            std::fill(current_frame_.off_events[y].begin(), current_frame_.off_events[y].end(), 0);
        }
        total_events_ = 0;
        on_events_count_ = 0;
        off_events_count_ = 0;
    }

    int Frame::get_total_events() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return total_events_;
    }

    int Frame::get_on_events() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return on_events_count_;
    }

    int Frame::get_off_events() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return off_events_count_;
    }

    bool Frame::is_dirty() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return has_published_frame_ || is_dirty_;
    }

    bool Frame::consume_dirty() {
        std::lock_guard<std::mutex> lock(mutex_);
        const bool was_dirty = is_dirty_;
        is_dirty_ = false;
        return was_dirty;
    }

    void Frame::set_dirty(bool newDirty) {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            is_dirty_ = newDirty;
        }
        if (newDirty) {
            frame_ready_.notify_one();
        }
    }

    EventTimestamp Frame::get_last_update() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return current_frame_.timestamp;
    }

    const SensorMetadata& Frame::get_metadata() const {
        return *metadata_;
    }

    void Frame::ensure_frame_storage(FrameStr& frame) {
        if (metadata_ == nullptr) {
            return;
        }

        if (static_cast<int>(frame.on_events.size()) != metadata_->height) {
            frame.on_events.assign(metadata_->height, std::vector<int>(metadata_->width, 0));
        } else {
            for (int y = 0; y < metadata_->height; ++y) {
                if (static_cast<int>(frame.on_events[y].size()) != metadata_->width) {
                    frame.on_events[y].assign(metadata_->width, 0);
                }
            }
        }

        if (static_cast<int>(frame.off_events.size()) != metadata_->height) {
            frame.off_events.assign(metadata_->height, std::vector<int>(metadata_->width, 0));
        } else {
            for (int y = 0; y < metadata_->height; ++y) {
                if (static_cast<int>(frame.off_events[y].size()) != metadata_->width) {
                    frame.off_events[y].assign(metadata_->width, 0);
                }
            }
        }
    }

}
