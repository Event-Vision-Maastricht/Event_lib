#include "event_lib/core/event_packet.hpp"
#include "event_lib/io/stream/DatasetEventStream.hpp"
#include "event_lib/processing/DisplayMode.hpp"
#include "event_lib/processing/Window.hpp"

#include <atomic>
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <psapi.h>
#pragma comment(lib, "psapi.lib")
#else
#include <sys/resource.h>
#include <unistd.h>
#endif

namespace {

enum class RepresentationMode {
    EventCountHistogram,
    TimeWindowHistogram,
    TimeSurface
};

struct LoadedEvents {
    event_lib::SensorMetadata metadata;
    std::vector<event_lib::EventPacket> packets;
    std::size_t event_count = 0;
};

struct MemorySnapshot {
    std::uint64_t current_bytes = 0;
    std::uint64_t peak_bytes = 0;
};

MemorySnapshot process_memory() {
#ifdef _WIN32
    PROCESS_MEMORY_COUNTERS_EX counters{};
    if (GetProcessMemoryInfo(
            GetCurrentProcess(),
            reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&counters),
            sizeof(counters))) {
        return {
            static_cast<std::uint64_t>(counters.WorkingSetSize),
            static_cast<std::uint64_t>(counters.PeakWorkingSetSize)
        };
    }
    return {};
#else
    rusage usage{};
    getrusage(RUSAGE_SELF, &usage);
#if defined(__APPLE__)
    const auto peak_bytes = static_cast<std::uint64_t>(usage.ru_maxrss);
#else
    const auto peak_bytes = static_cast<std::uint64_t>(usage.ru_maxrss) * 1024ULL;
#endif
    return {peak_bytes, peak_bytes};
#endif
}

class MemorySampler {
public:
    void start() {
        running_.store(true);
        peak_bytes_.store(process_memory().current_bytes);
        worker_ = std::thread([this]() {
            while (running_.load()) {
                const auto snapshot = process_memory();
                const auto sampled_peak = std::max(snapshot.current_bytes, snapshot.peak_bytes);
                auto old_peak = peak_bytes_.load();
                while (sampled_peak > old_peak &&
                       !peak_bytes_.compare_exchange_weak(old_peak, sampled_peak)) {
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(5));
            }
        });
    }

    std::uint64_t stop() {
        running_.store(false);
        if (worker_.joinable()) worker_.join();
        const auto snapshot = process_memory();
        return std::max({peak_bytes_.load(), snapshot.current_bytes, snapshot.peak_bytes});
    }

private:
    std::atomic<bool> running_{false};
    std::atomic<std::uint64_t> peak_bytes_{0};
    std::thread worker_;
};

std::string xml_escape(const std::string& value) {
    std::string out;
    out.reserve(value.size());
    for (const char ch : value) {
        switch (ch) {
            case '&': out += "&amp;"; break;
            case '<': out += "&lt;"; break;
            case '>': out += "&gt;"; break;
            case '"': out += "&quot;"; break;
            case '\'': out += "&apos;"; break;
            default: out += ch; break;
        }
    }
    return out;
}

std::string extension_of(const std::string& path) {
    auto ext = std::filesystem::path(path).extension().string();
    if (!ext.empty() && ext.front() == '.') ext.erase(ext.begin());
    return ext.empty() ? "unknown" : ext;
}

std::uint64_t file_size_or_zero(const std::string& path) {
    std::error_code ec;
    const auto size = std::filesystem::file_size(path, ec);
    return ec ? 0 : static_cast<std::uint64_t>(size);
}

RepresentationMode parse_mode(const std::string& mode) {
    if (mode == "event_count" || mode == "eventc" || mode == "event_count_histogram") {
        return RepresentationMode::EventCountHistogram;
    }
    if (mode == "time_window" || mode == "timew" || mode == "time_window_histogram") {
        return RepresentationMode::TimeWindowHistogram;
    }
    if (mode == "time_surface" || mode == "surface") {
        return RepresentationMode::TimeSurface;
    }
    throw std::invalid_argument("Unknown representation mode: " + mode);
}

std::string mode_name(RepresentationMode mode) {
    switch (mode) {
        case RepresentationMode::EventCountHistogram: return "event_count_histogram";
        case RepresentationMode::TimeWindowHistogram: return "time_window_histogram";
        case RepresentationMode::TimeSurface: return "time_surface";
    }
    return "unknown";
}

LoadedEvents load_events_as_packets(
    const std::string& path,
    std::size_t requested_events,
    std::size_t packet_size) {
    LoadedEvents loaded;
    event_lib::DatasetEventStream stream(path);
    loaded.metadata = stream.metadata();

    const bool read_all = requested_events == 0;
    const auto target = read_all ? std::numeric_limits<std::size_t>::max() : requested_events;

    while (stream.has_next() && loaded.event_count < target) {
        const auto remaining = target - loaded.event_count;
        const auto next_count = read_all ? packet_size : std::min(packet_size, remaining);
        try {
            auto packet = stream.next_packet(next_count);
            if (packet.is_empty()) break;
            loaded.event_count += packet.size();
            loaded.packets.push_back(std::move(packet));
        } catch (const std::out_of_range&) {
            break;
        }
    }

    stream.close();
    return loaded;
}

struct RepresentationResult {
    int repeat = 0;
    std::string path;
    std::string format;
    std::string mode;
    std::uint64_t file_size_bytes = 0;
    std::size_t source_events = 0;
    std::size_t packet_count = 0;
    std::size_t packet_size = 0;
    std::int64_t mode_value = 0;
    int width = 0;
    int height = 0;
    bool visualization_enabled = false;
    std::uint64_t generated_frames = 0;
    double generation_runtime_ms = 0.0;
    double throughput_events_per_second = 0.0;
    std::uint64_t memory_before_bytes = 0;
    std::uint64_t peak_memory_bytes = 0;
};

RepresentationResult run_representation_benchmark(
    const std::string& path,
    const LoadedEvents& loaded,
    RepresentationMode mode,
    std::int64_t mode_value,
    std::size_t packet_size,
    bool show_window,
    int repeat) {
    RepresentationResult result;
    result.repeat = repeat;
    result.path = path;
    result.format = extension_of(path);
    result.file_size_bytes = file_size_or_zero(path);
    result.mode = mode_name(mode);
    result.source_events = loaded.event_count;
    result.packet_count = loaded.packets.size();
    result.packet_size = packet_size;
    result.mode_value = mode_value;
    result.width = loaded.metadata.width;
    result.height = loaded.metadata.height;
    result.visualization_enabled = show_window;
    result.memory_before_bytes = process_memory().current_bytes;

    event_lib::DisplayMode display_mode;
    if (!display_mode.init_metadata(loaded.metadata)) {
        throw std::runtime_error("DisplayMode metadata initialization failed.");
    }

    const auto frame = display_mode.get_frame();
    const auto stop_flag = display_mode.get_stop_flag();
    if (!frame || !stop_flag) {
        throw std::runtime_error("DisplayMode did not provide a frame or stop flag.");
    }

    std::unique_ptr<event_lib::Window> window;
    if (show_window) {
        window = std::make_unique<event_lib::Window>();
        window->init_window(frame, true, "event_lib representation benchmark", stop_flag);
    }

    std::atomic<bool> producer_done{false};
    std::atomic<std::uint64_t> frame_count{0};
    std::exception_ptr consumer_error;

    std::thread consumer([&]() {
        try {
            while (!producer_done.load() || frame->is_dirty()) {
                const bool ready = frame->wait_for_published_frame(std::chrono::milliseconds(2));
                if (!ready && producer_done.load()) break;

                if (frame->is_dirty()) {
                    if (show_window && window) {
                        window->show_frame();
                    } else {
                        event_lib::FrameStr consumed;
                        frame->consume_published_frame(consumed);
                    }
                    ++frame_count;
                }
            }
        } catch (...) {
            consumer_error = std::current_exception();
            if (stop_flag) stop_flag->store(true);
        }
    });

    MemorySampler sampler;
    sampler.start();
    const auto start = std::chrono::steady_clock::now();

    for (const auto& packet : loaded.packets) {
        if (stop_flag->load()) break;
        switch (mode) {
            case RepresentationMode::EventCountHistogram:
                display_mode.eventc_histogram(packet, static_cast<int>(mode_value));
                break;
            case RepresentationMode::TimeWindowHistogram:
                display_mode.timew_histogram(packet, static_cast<event_lib::EventTimestamp>(mode_value));
                break;
            case RepresentationMode::TimeSurface:
                display_mode.make_time_surface(packet);
                break;
        }
    }
    display_mode.flush_pending_frame();

    const auto end = std::chrono::steady_clock::now();
    result.peak_memory_bytes = sampler.stop();
    producer_done.store(true);
    if (consumer.joinable()) consumer.join();

    if (consumer_error) std::rethrow_exception(consumer_error);

    result.generated_frames = frame_count.load();
    result.generation_runtime_ms =
        std::chrono::duration<double, std::milli>(end - start).count();
    result.throughput_events_per_second =
        result.generation_runtime_ms > 0.0
            ? static_cast<double>(result.source_events) / (result.generation_runtime_ms / 1000.0)
            : 0.0;

    if (window) window->finish();
    display_mode.finish();
    return result;
}

void append_result(const std::string& output_path, const RepresentationResult& result) {
    std::ofstream out(output_path, std::ios::app);
    out << std::fixed << std::setprecision(3);
    out << "<representation_benchmark"
        << " repeat=\"" << result.repeat << "\""
        << " path=\"" << xml_escape(result.path) << "\""
        << " format=\"" << xml_escape(result.format) << "\""
        << " file_size_bytes=\"" << result.file_size_bytes << "\""
        << " mode=\"" << xml_escape(result.mode) << "\""
        << " mode_value=\"" << result.mode_value << "\""
        << " source_events=\"" << result.source_events << "\""
        << " packet_count=\"" << result.packet_count << "\""
        << " packet_size=\"" << result.packet_size << "\""
        << " width=\"" << result.width << "\""
        << " height=\"" << result.height << "\""
        << " generated_frames=\"" << result.generated_frames << "\""
        << " generation_runtime_ms=\"" << result.generation_runtime_ms << "\""
        << " throughput_events_per_second=\"" << result.throughput_events_per_second << "\""
        << " memory_before_bytes=\"" << result.memory_before_bytes << "\""
        << " peak_memory_bytes=\"" << result.peak_memory_bytes << "\""
        << " visualization_enabled=\"" << (result.visualization_enabled ? "true" : "false") << "\""
        << " />\n";
}

void print_usage(const char* exe) {
    std::cerr
        << "Usage:\n"
        << "  " << exe << " <dataset_path> <event_count> <mode> [mode_value] [packet_size] [output_file] [show_window] [repeats]\n\n"
        << "Modes:\n"
        << "  event_count    mode_value = events per frame. Default: 10000.\n"
        << "  time_window    mode_value = timestamp window size. Default: 16.\n"
        << "  time_surface   mode_value ignored. Default: 0.\n\n"
        << "Arguments:\n"
        << "  event_count    Number of events to load before generation. Use 0 to read until EOF.\n"
        << "  packet_size    Parser packet size used while preloading events. Default: 10000.\n"
        << "  output_file    XML-like result log. Default: representation_benchmark_results.xml.\n"
        << "  show_window    0 = drain frames headlessly, 1 = show with event_lib::Window. Default: 0.\n"
        << "  repeats        Number of repeated generation runs over the preloaded event set. Default: 1.\n";
}

} // namespace

int main(int argc, char** argv) {
    if (argc < 4) {
        print_usage(argv[0]);
        return 2;
    }

    const std::string dataset_path = argv[1];
    const auto event_count = static_cast<std::size_t>(std::stoull(argv[2]));
    const auto mode = parse_mode(argv[3]);

    const std::int64_t default_mode_value =
        mode == RepresentationMode::EventCountHistogram ? 10000 :
        mode == RepresentationMode::TimeWindowHistogram ? 16 : 0;

    const auto mode_value = argc >= 5 ? std::stoll(argv[4]) : default_mode_value;
    const auto packet_size = argc >= 6 ? static_cast<std::size_t>(std::stoull(argv[5])) : 10000;
    const std::string output_file = argc >= 7 ? argv[6] : "representation_benchmark_results.xml";
    const bool show_window = argc >= 8 ? std::stoi(argv[7]) != 0 : false;
    const int repeats = argc >= 9 ? std::stoi(argv[8]) : 1;

    try {
        const auto loaded = load_events_as_packets(dataset_path, event_count, packet_size);
        std::cout << "preloaded_events=" << loaded.event_count
                  << " packets=" << loaded.packets.size()
                  << " width=" << loaded.metadata.width
                  << " height=" << loaded.metadata.height
                  << '\n';

        for (int repeat = 1; repeat <= repeats; ++repeat) {
            const auto result = run_representation_benchmark(
                dataset_path,
                loaded,
                mode,
                mode_value,
                packet_size,
                show_window,
                repeat);
            append_result(output_file, result);
            std::cout << "repeat=" << repeat
                      << " mode=" << result.mode
                      << " frames=" << result.generated_frames
                      << " generation_runtime_ms=" << result.generation_runtime_ms
                      << " throughput_events_per_second=" << result.throughput_events_per_second
                      << " peak_memory_bytes=" << result.peak_memory_bytes
                      << '\n';
        }
    } catch (const std::exception& e) {
        std::cerr << "Benchmark failed: " << e.what() << '\n';
        return 1;
    }

    return 0;
}
