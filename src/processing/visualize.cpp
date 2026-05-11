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
        // initialize queue with sensor dimensions
        frame_queue_ = FrameQueue(metadata);
        initialized = true;
        return true;
    }

    void visualize::timew_histogram(const EventPacket& packet, long time_window){
        if (!initialized) return;
        if(stop_requested_.load()) return;

        Frame frame(*metadata_);
        const auto& events = packet.get_events();
        if (events.empty()) return;

        long start_time = events[0].timestamp;
        long window_end = start_time + time_window;
        bool work_on_frame = false;
        FrameStr ready_frame;

        for (const auto& ev : events) {
            long ev_time = ev.timestamp;
            frame.add_event(ev);
            work_on_frame = true;

            // if ev_time exceeded, time for next frame
            if (ev_time >= window_end) {
                if (frame.finalize_frame(start_time, ready_frame)) {
                    frame_queue_.add_frame(std::move(ready_frame));
                }
                start_time = ev_time;

                // Move to next window
                work_on_frame = false;
                window_end = start_time + time_window;
            }
        }
        if (work_on_frame) {
            if (frame.finalize_frame(start_time, ready_frame)) {
                frame_queue_.add_frame(std::move(ready_frame));
            }
        }
    }

    void visualize::eventc_histogram(const EventPacket& packet, int event_count){
        if (!initialized) return;
        if (stop_requested_.load()) return;

        Frame frame(*metadata_);
        const auto& events = packet.get_events();

        if (events.empty()) return;

        int event_counter = 0;
        long frame_ts = events[0].timestamp;
        FrameStr ready_frame;

        for (const auto& ev : events) {
            frame.add_event(ev);
            event_counter++;

            // check if we should start new frame
            if (event_counter >= event_count) {
                if (frame.finalize_frame(frame_ts, ready_frame)) {
                    frame_queue_.add_frame(std::move(ready_frame));
                }
                frame_ts = ev.timestamp;

                event_counter = 0;
            }
        }
        // std::cerr << "DEBUG: added events from current package, finalized frame" << std::endl;
        // Save final frame if there are remaining events
        if (event_counter > 0) {
            if (frame.finalize_frame(frame_ts, ready_frame)) {
                frame_queue_.add_frame(std::move(ready_frame));
            }
        }

    }

    void visualize::make_bi(const EventPacket& packet){
        // TODO: Implement binary frame generation
    }

    void visualize::make_time_surface(const EventPacket& packet){
        // TODO: Implement time surface visualization
    }

    void visualize::finish(){
        frame_queue_.close();
    }


    void visualize::show(bool colorOn){
        if (!initialized || metadata_ == nullptr) return;

        std::string window_name = "Event lib";
        cv::namedWindow(window_name, cv::WINDOW_NORMAL);

        //cv::resizeWindow(window_name, metadata_->width * 2, metadata_->height * 2);
        cv::Mat image(metadata_->height, metadata_->width, colorOn ? CV_8UC3 : CV_8UC1, cv::Scalar::all(0));

        FrameStr frame;
        while (frame_queue_.wait_and_pop_frame(frame)) {
            int max_value = 1;
            for (int y = 0; y < metadata_->height; ++y) {
                for (int x = 0; x < metadata_->width; ++x) {
                    max_value = std::max(max_value, frame.on_events[y][x]);
                    max_value = std::max(max_value, frame.off_events[y][x]);
                }
            }

            const double gain = 1.5;
            const int minimum_visible = 35;

            for (int y = 0; y < metadata_->height; ++y) {
                for (int x = 0; x < metadata_->width; ++x) {
                    const int on_value = frame.on_events[y][x];
                    const int off_value = frame.off_events[y][x];

                    if (colorOn) {
                        // Map ON events to blue channel and OFF events to red channel
                        //const unsigned char blue = static_cast<unsigned char>(std::min(255, (on_value * 255) / max_value));
                        //const unsigned char red = static_cast<unsigned char>(std::min(255, (off_value * 255) / max_value));
                        const unsigned char blue = static_cast<unsigned char>(std::min(255.0, std::max(0.0, (on_value * 255.0 * gain) / max_value)));
                        const unsigned char red = static_cast<unsigned char>(std::min(255.0, std::max(0.0, (off_value * 255.0 * gain) / max_value)));
                        image.at<cv::Vec3b>(y, x) = cv::Vec3b(blue, 0, red);
                    } else {
                        const int combined = on_value + off_value;
                        const unsigned char gray = static_cast<unsigned char>(std::min(255, (combined * 255)/(2 * max_value)));
                        // const int scaled = static_cast<int>((combined * 255.0 * gain) / (2.0 * max_value));
                        // const unsigned char gray = static_cast<unsigned char>(combined > 0 ? std::clamp(std::max(minimum_visible, scaled), 0, 255) : 0);
                        image.at<unsigned char>(y, x) = gray;
                    }
                }
            }

            // cv::Mat display_image;
            // cv::resize(image, display_image, cv::Size(), 4.0, 4.0, cv::INTER_NEAREST);
            cv::imshow(window_name, image); //display_image);
            const int key = cv::waitKey(1);
            if (key == 27 || key == 'q' || key == 'Q') {
                stop_requested_ .store(true);
                frame_queue_.close();
                break;
            }
        }

        cv::destroyWindow(window_name);
    }

    // void show_and_save(bool colorOn = true, std::string name){
    //     //TODO: keep the frames without erasing and save as mp4
    //     return;
    // }
        
    // void save(bool colorOn = true, std::string name){
    //     //TODO: just create and save as mp4
    //     return;
    // }
}