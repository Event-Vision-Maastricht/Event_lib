#include "event_lib/processing/Window.hpp"

#if EVENT_LIB_WITH_OPENCV
#include <new>
#include <opencv2/core.hpp>
#include <opencv2/core/core_c.h>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/highgui/highgui_c.h>
#endif

#include <algorithm>
#include <cstdlib>
#include <stdexcept>
#include <utility>

namespace event_lib{
#if EVENT_LIB_WITH_OPENCV
    namespace {
        bool has_display_server() {
#if defined(_WIN32) || defined(__APPLE__)
            return true;
#else
            return std::getenv("DISPLAY") != nullptr || std::getenv("WAYLAND_DISPLAY") != nullptr;
#endif
        }

        bool is_window_visible(const std::string& window_name) {
            if (window_name.empty()) return false;
            try {
                return cvGetWindowProperty(window_name.c_str(), cv::WND_PROP_VISIBLE) >= 1;
            } catch (const cv::Exception&) {
                return false;
            }
        }

        void destroy_window_if_registered(const std::string& window_name) {
            if (window_name.empty()) return;
            try {
                if (is_window_visible(window_name)) {
                    cvDestroyWindow(window_name.c_str());
                }
            } catch (const cv::Exception&) {
                // Some Linux HighGUI backends throw if a user already closed the window apparently.
            }
        }
    }

    void Window::init_window(std::shared_ptr<Frame> frame, bool colorOn, const std::string& windowName, std::shared_ptr<std::atomic<bool>> stopFlag) {
        if (!frame || frame_ ) return;
        frame_ = std::move(frame);
        color_on_ = colorOn;
        window_name_ = windowName.empty() ? "Event_lib" : windowName;
        stopFlag_ = std::move(stopFlag);
        const auto& metadata = frame_->get_metadata();

        if (!has_display_server()) {
            if (stopFlag_) stopFlag_->store(true);
            frame_.reset();
            throw std::runtime_error("Window requires a Linux display server. Set DISPLAY or WAYLAND_DISPLAY, or build with EVENT_LIB_WITH_OPENCV=OFF for headless use.");
        }

        cvNamedWindow(window_name_.c_str(), CV_WINDOW_NORMAL);
        image_ = cv::Mat(metadata.height, metadata.width, color_on_ ? CV_8UC3 : CV_8UC1);
        image_.setTo(cv::Scalar::all(0));
        display_frame_.on_events.assign(metadata.height, std::vector<int>(metadata.width, 0));
        display_frame_.off_events.assign(metadata.height, std::vector<int>(metadata.width, 0));
        display_frame_.timestamp = 0;
        last_decay_ = std::chrono::steady_clock::now();
    }

    void Window::show_frame() {
        if (!frame_ || (stopFlag_ && stopFlag_->load())) return;

        if (has_shown_image_ && !is_window_visible(window_name_)) {
            finish();
            return;
        }

        const auto& metadata = frame_->get_metadata();
        if (frame_->consume_published_frame(display_frame_)) has_display_frame_ = true;
        if (!has_display_frame_) {
            cvWaitKey(1);
            return;
        }

        const FrameStr& current_frame = display_frame_;
        if (image_.empty()) {
            image_ = cv::Mat(metadata.height, metadata.width, color_on_ ? CV_8UC3 : CV_8UC1);
        }

        int max_value = 0;
        for (int y = 0; y < metadata.height; ++y) {
            for (int x = 0; x < metadata.width; ++x) {
                if (color_on_) {
                    max_value = std::max(max_value, current_frame.on_events[y][x]);
                    max_value = std::max(max_value, current_frame.off_events[y][x]);
                } else {
                    max_value = std::max(
                        max_value,
                        current_frame.on_events[y][x] + current_frame.off_events[y][x]
                    );
                }
            }
        }

        const double normalized_scale = max_value > 0 ? 255.0 / static_cast<double>(max_value) : 0.0;
        const double scale = std::max(12.0, normalized_scale);
        for (int y = 0; y < metadata.height; ++y) {
            for (int x = 0; x < metadata.width; ++x) {
                const int on_value = current_frame.on_events[y][x];
                const int off_value = current_frame.off_events[y][x];

                if (color_on_) {
                    const unsigned char blue = static_cast<unsigned char>(std::min(255.0, std::max(0.0, on_value * scale)));
                    const unsigned char red = static_cast<unsigned char>(std::min(255.0, std::max(0.0, off_value * scale)));
                    image_.at<cv::Vec3b>(y, x) = cv::Vec3b(blue, 0, red);
                } else {
                    const int combined = on_value + off_value;
                    const unsigned char gray = static_cast<unsigned char>(std::min(255.0, std::max(0.0, combined * scale)));
                    image_.at<unsigned char>(y, x) = gray;
                }
            }
        }

        IplImage image_header = cvIplImage(image_);
        cvShowImage(window_name_.c_str(), &image_header);
        has_shown_image_ = true;
        const int key = cvWaitKey(1);
        if (key == 27 || key == 'q' || key == 'Q') finish();
    }

    void Window::finish() {
        if (stopFlag_) stopFlag_->store(true);
        if (frame_) frame_->close();
        destroy_window_if_registered(window_name_);

        image_.release();
        display_frame_ = FrameStr{};
        has_display_frame_ = false;
        has_shown_image_ = false;
        frame_.reset();
        stopFlag_.reset();
    }
#else
    void Window::init_window(std::shared_ptr<Frame> frame, bool colorOn, const std::string& windowName, std::shared_ptr<std::atomic<bool>> stopFlag) {
        frame_ = std::move(frame);
        color_on_ = colorOn;
        window_name_ = windowName.empty() ? "Event_lib" : windowName;
        stopFlag_ = std::move(stopFlag);
        if (stopFlag_) stopFlag_->store(true);
        throw std::runtime_error("Window support is disabled. Reconfigure with EVENT_LIB_WITH_OPENCV=ON to use event_lib::Window.");
    }

    void Window::show_frame() {
        if (stopFlag_) stopFlag_->store(true);
    }

    void Window::finish() {
        if (stopFlag_) stopFlag_->store(true);
        if (frame_) frame_->close();
        display_frame_ = FrameStr{};
        has_display_frame_ = false;
        has_shown_image_ = false;
        frame_.reset();
        stopFlag_.reset();
    }
#endif
}
