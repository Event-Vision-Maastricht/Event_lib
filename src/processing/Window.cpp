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
        display_frame_.on_events.assign(metadata.height, std::vector<int>(metadata.width, 0));
        display_frame_.off_events.assign(metadata.height, std::vector<int>(metadata.width, 0));
        display_frame_.timestamp = 0;
        last_decay_ = std::chrono::steady_clock::now();
    }

    void Window::show_frame() {
        if (!frame_ || (stopFlag_ && stopFlag_->load())) return;

        if (has_shown_image_ && cv::getWindowProperty(window_name_, cv::WND_PROP_VISIBLE) < 1) {
            finish();
            return;
        }

        const auto& metadata = frame_->get_metadata();
        if (frame_->consume_published_frame(display_frame_)) has_display_frame_ = true;
        if (!has_display_frame_) {
            cv::waitKey(1);
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

        cv::setWindowTitle(window_name_, window_name_ );
        cv::imshow(window_name_, image_);
        has_shown_image_ = true;
        const int key = cv::waitKey(1);
        if (key == 27 || key == 'q' || key == 'Q') finish();
    }

    void Window::finish() {
        if (stopFlag_) stopFlag_->store(true);
        if (frame_) frame_->close();
        if (!window_name_.empty()) cv::destroyWindow(window_name_);

        image_.release();
        display_frame_ = FrameStr{};
        has_display_frame_ = false;
        has_shown_image_ = false;
        frame_.reset();
        stopFlag_.reset();
    }
}
