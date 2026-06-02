#pragma once

#include "event_lib/processing/Frame.hpp"

#ifndef EVENT_LIB_WITH_OPENCV
#define EVENT_LIB_WITH_OPENCV 1
#endif

#if EVENT_LIB_WITH_OPENCV
#include <new>
#include <opencv2/core.hpp>
#endif

#include <atomic>
#include <chrono>
#include <memory>
#include <string>

/**
 * @brief A window struct to display updated frames.
 * If no incoming frame decays the frame down slowly until new input.
 */
namespace event_lib{
    class Window{
        public:
            // Initializes the window and binds it to a shared frame source.
            void init_window(
                std::shared_ptr<Frame> frame,
                bool colorOn,
                const std::string& windowName = "Event_lib",
                std::shared_ptr<std::atomic<bool>> stopFlag = nullptr);
            // Shows the current frame and performs a small idle decay when no new frame was published.
            void show_frame();
            void finish();

        //TODO: save video
        // void show_and_save(bool colorOn = true, std::string name);
        // void save(bool colorOn = true, std::string name);

        private:
            std::chrono::steady_clock::time_point last_decay_;
            double idle_decay_amount_{0.85};
            std::string window_name_ = "Event_lib";
            bool color_on_{true};
#if EVENT_LIB_WITH_OPENCV
            cv::Mat image_;
#endif
            FrameStr display_frame_;
            bool has_display_frame_{false};
            bool has_shown_image_{false};
            std::shared_ptr<Frame> frame_;
            std::shared_ptr<std::atomic<bool>> stopFlag_;
    };
}
