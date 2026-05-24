#include "event_lib/processing/Window.hpp"
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/imgcodecs.hpp>
#include <algorithm>
#include <utility>

namespace event_lib{
    void Window::init_window(std::shared_ptr<Frame> frame, bool colorOn, const std::string& windowName, std::shared_ptr<std::atomic<bool>> stopFlag) {
        if (!frame || frame_ ) return;
        frame_ = std::move(frame);
        color_on_ = colorOn;
        window_name_ = windowName;
        stopFlag_ = std::move(stopFlag);
        const auto& metadata = frame_->get_metadata();
        cv::namedWindow(window_name_, cv::WINDOW_NORMAL);
        image_ = cv::Mat(metadata.height, metadata.width, color_on_ ? CV_8UC3 : CV_8UC1);
        image_.setTo(cv::Scalar::all(0));
        last_decay_ = std::chrono::steady_clock::now();
    }

    void Window::show_frame() {
        if (!frame_ || (stopFlag_ && stopFlag_->load())) return;

        if (cv::getWindowProperty(window_name_, cv::WND_PROP_VISIBLE) < 1) {
            finish();
            return;
        }

        frame_->consume_dirty();
        frame_->decay_frame(idle_decay_amount_);

        const FrameStr& current_frame = frame_->get_current_frame();
        const auto& metadata = frame_->get_metadata();
        if (image_.empty()) {
            image_ = cv::Mat(metadata.height, metadata.width, color_on_ ? CV_8UC3 : CV_8UC1);
        }

        const double gain = 12.0;
        for (int y = 0; y < metadata.height; ++y) {
            for (int x = 0; x < metadata.width; ++x) {
                const int on_value = current_frame.on_events[y][x];
                const int off_value = current_frame.off_events[y][x];

                if (color_on_) {
                    const unsigned char blue = static_cast<unsigned char>(std::min(255.0, std::max(0.0, on_value * gain)));
                    const unsigned char red = static_cast<unsigned char>(std::min(255.0, std::max(0.0, off_value * gain)));
                    image_.at<cv::Vec3b>(y, x) = cv::Vec3b(blue, 0, red);
                } else {
                    const int combined = on_value + off_value;
                    const unsigned char gray = static_cast<unsigned char>(std::min(255.0, std::max(0.0, combined * gain)));
                    image_.at<unsigned char>(y, x) = gray;
                }
            }
        }

        cv::setWindowTitle(window_name_, window_name_ );
        cv::imshow(window_name_, image_);
        const int key = cv::waitKey(1);
        if (key == 27 || key == 'q' || key == 'Q') finish();
    }

    void Window::finish() {
        if (stopFlag_) stopFlag_->store(true);
        if (!window_name_.empty()) cv::destroyWindow(window_name_);

        image_.release();
        frame_.reset();
        stopFlag_.reset();
    }
}