#define NOMINMAX
#include "event_lib/processing/visualize.hpp"
#include <algorithm>
#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/imgcodecs.hpp>

////////////////LAB2B HAS THE TIME WINDOW HISTOGRAM IMPLEMENTATION
namespace event_lib {

    bool visualize::init_metadata(const SensorMetadata& metadata){
        metadata_ = &metadata;
        frame_ = std::make_unique<Frame>(*metadata_);
        window_initialized_ = false;
        initialized = true;
        return true;
    }

    void visualize::init_show(const std::string& window_name, bool colorOn) {
        if (!initialized || metadata_ == nullptr || window_initialized_) return;
        window_name_ = window_name;
        color_on_ = colorOn;
        cv::namedWindow(window_name_, cv::WINDOW_NORMAL);
        // allocate persistent display buffer
        image_ = cv::Mat(metadata_->height, metadata_->width, color_on_ ? CV_8UC3 : CV_8UC1);
        image_.setTo(cv::Scalar::all(0));
        window_initialized_ = true;
    }

    void visualize::timew_histogram(const EventPacket& packet, long time_window, bool colorOn){
        if (!initialized || stop_requested_.load() || !frame_) return;

        init_show(window_name_, colorOn);
        const auto& events = packet.get_events();
        if (events.empty()) return;

        long window_start = events[0].timestamp;
        long window_end = window_start + time_window;
        bool work_on_frame = false;

        //going through all events
        for (const auto& ev : events) {
            if (stop_requested_.load()) return;

            long ev_time = ev.timestamp;
            frame_->add_event(ev);
            work_on_frame = true;

            // if ev_time exceeded, time for next frame
            if (ev_time >= window_end) {
                show_frame(window_start);
                if (stop_requested_.load()) return;

                // Move to next window
                window_start = ev_time;
                work_on_frame = false;
                window_end = window_start + time_window;
                frame_->decay_frame(0.9);
            }
        }
        //handle if theres events left to be updated
        //TODO:Find a way to handle: if there is a new packet incoming then dont show, 
        // wait until frame updated completely
        if (work_on_frame) {
            show_frame(window_start);
        }
    }

    
    void visualize::eventc_histogram(const EventPacket& packet, int event_count, bool colorOn){
        if (!initialized || stop_requested_.load() || !frame_) return;

        init_show(window_name_, colorOn);
        const auto& events = packet.get_events();
        if (events.empty()) return;

        int event_counter = 0;
        long frame_ts = events[0].timestamp;

        for (const auto& ev : events) {
            if (stop_requested_.load()) return;

            frame_->add_event(ev);
            event_counter++;

            // check if we should start new frame
            if (event_counter >= event_count) {
                show_frame(frame_ts);
                if (stop_requested_.load()) return;

                frame_ts = ev.timestamp; //new timestamp of the next frame 
                event_counter = 0; // ready for updating frame
                frame_->decay_frame(0.9);

            }
        }
        // Save final frame if there are remaining events
        //TODO: if incoming package wait until event count limit is filled
        if (event_counter > 0) {
            show_frame(frame_ts);
        }
    }

    void visualize::show_frame(long frame_time){
        if (!initialized || metadata_ == nullptr || !frame_) {
            return;
        }
        if (image_.empty()) {
            init_show(window_name_, color_on_);
            if (image_.empty()) return;
        }

        const FrameStr& current_frame = frame_->get_current_frame();

        int max_value = 1;
        for (int y = 0; y < metadata_->height; ++y) {
            for (int x = 0; x < metadata_->width; ++x) {
                max_value = std::max(max_value, current_frame.on_events[y][x]);
                max_value = std::max(max_value, current_frame.off_events[y][x]);
            }
        }

        const double gain = 1.5;
        for (int y = 0; y < metadata_->height; ++y) {
            for (int x = 0; x < metadata_->width; ++x) {
                const int on_value = current_frame.on_events[y][x];
                const int off_value = current_frame.off_events[y][x];

                if (color_on_) {
                    const unsigned char blue = static_cast<unsigned char>(std::min(255.0, std::max(0.0, (on_value * 255.0 * gain) / max_value)));
                    const unsigned char red = static_cast<unsigned char>(std::min(255.0, std::max(0.0, (off_value * 255.0 * gain) / max_value)));
                    image_.at<cv::Vec3b>(y, x) = cv::Vec3b(blue, 0, red);
                } else {
                    const int combined = on_value + off_value;
                    const unsigned char gray = static_cast<unsigned char>(std::min(255, (combined * 255) / (2 * max_value)));
                    image_.at<unsigned char>(y, x) = gray;
                }
            }
        }

        cv::setWindowTitle(window_name_, window_name_ + " - " + std::to_string(frame_time));
        cv::imshow(window_name_, image_);
        const int key = cv::waitKey(1);
        if (key == 27 || key == 'q' || key == 'Q') {
            finish();
        }
    }

    // void show_and_save(bool colorOn = true, std::string name){
    //     //TODO: keep the frames without erasing and save as mp4
    //     return;
    // }
        
    // void save(bool colorOn = true, std::string name){
    //     //TODO: just create and save as mp4
    //     return;
    // }

    
    void visualize::make_bi(const EventPacket& packet){
        // TODO: Implement binary frame generation
    }

    void visualize::make_time_surface(const EventPacket& packet){
        // TODO: Implement time surface visualization
    }

    void visualize::finish(){
        stop_requested_.store(true);
        if (window_initialized_) {
            cv::destroyWindow(window_name_);
            window_initialized_ = false;
        }
        image_.release();
    }
}
