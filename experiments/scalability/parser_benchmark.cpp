#include "event_lib/io/stream/DatasetEventStream.hpp"

#include <atomic>
#include <chrono>
#include <cstdint>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <thread>

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

struct ParserResult {
    int repeat = 0;
    std::string path;
    std::string format;
    std::uint64_t file_size_bytes = 0;
    std::size_t requested_events = 0;
    std::size_t processed_events = 0;
    std::size_t packet_size = 0;
    int width = 0;
    int height = 0;
    double runtime_ms = 0.0;
    double throughput_events_per_second = 0.0;
    std::uint64_t memory_before_bytes = 0;
    std::uint64_t peak_memory_bytes = 0;
};

ParserResult run_parser_benchmark(
    const std::string& path,
    std::size_t requested_events,
    std::size_t packet_size,
    int repeat) {
    ParserResult result;
    result.repeat = repeat;
    result.path = path;
    result.format = extension_of(path);
    result.file_size_bytes = file_size_or_zero(path);
    result.requested_events = requested_events;
    result.packet_size = packet_size;
    result.memory_before_bytes = process_memory().current_bytes;

    MemorySampler sampler;
    sampler.start();
    const auto start = std::chrono::steady_clock::now();

    event_lib::DatasetEventStream stream(path);
    result.width = stream.metadata().width;
    result.height = stream.metadata().height;

    const bool read_all = requested_events == 0;
    const auto target = read_all ? std::numeric_limits<std::size_t>::max() : requested_events;

    while (stream.has_next() && result.processed_events < target) {
        const auto remaining = target - result.processed_events;
        const auto next_count = read_all ? packet_size : std::min(packet_size, remaining);
        try {
            const auto packet = stream.next_packet(next_count);
            if (packet.is_empty()) break;
            result.processed_events += packet.size();
        } catch (const std::out_of_range&) {
            break;
        }
    }
    stream.close();

    const auto end = std::chrono::steady_clock::now();
    result.peak_memory_bytes = sampler.stop();
    result.runtime_ms = std::chrono::duration<double, std::milli>(end - start).count();
    result.throughput_events_per_second =
        result.runtime_ms > 0.0
            ? static_cast<double>(result.processed_events) / (result.runtime_ms / 1000.0)
            : 0.0;
    return result;
}

void append_result(const std::string& output_path, const ParserResult& result) {
    std::ofstream out(output_path, std::ios::app);
    out << std::fixed << std::setprecision(3);
    out << "<parser_benchmark"
        << " repeat=\"" << result.repeat << "\""
        << " path=\"" << xml_escape(result.path) << "\""
        << " format=\"" << xml_escape(result.format) << "\""
        << " file_size_bytes=\"" << result.file_size_bytes << "\""
        << " requested_events=\"" << result.requested_events << "\""
        << " processed_events=\"" << result.processed_events << "\""
        << " packet_size=\"" << result.packet_size << "\""
        << " width=\"" << result.width << "\""
        << " height=\"" << result.height << "\""
        << " runtime_ms=\"" << result.runtime_ms << "\""
        << " throughput_events_per_second=\"" << result.throughput_events_per_second << "\""
        << " memory_before_bytes=\"" << result.memory_before_bytes << "\""
        << " peak_memory_bytes=\"" << result.peak_memory_bytes << "\""
        << " />\n";
}

void print_usage(const char* exe) {
    std::cerr
        << "Usage:\n"
        << "  " << exe << " <dataset_path> <event_count> [packet_size] [output_file] [repeats]\n\n"
        << "Arguments:\n"
        << "  event_count  Number of events to read. Use 0 to read until EOF.\n"
        << "  packet_size  Events requested per parser packet. Default: 10000.\n"
        << "  output_file  XML-like result log. Default: parser_benchmark_results.xml.\n"
        << "  repeats      Number of repeated runs. Default: 1.\n";
}

} // namespace

int main(int argc, char** argv) {
    if (argc < 3) {
        print_usage(argv[0]);
        return 2;
    }

    const std::string dataset_path = argv[1];
    const auto event_count = static_cast<std::size_t>(std::stoull(argv[2]));
    const auto packet_size = argc >= 4 ? static_cast<std::size_t>(std::stoull(argv[3])) : 10000;
    const std::string output_file = argc >= 5 ? argv[4] : "parser_benchmark_results.xml";
    const int repeats = argc >= 6 ? std::stoi(argv[5]) : 1;

    try {
        for (int repeat = 1; repeat <= repeats; ++repeat) {
            const auto result = run_parser_benchmark(dataset_path, event_count, packet_size, repeat);
            append_result(output_file, result);
            std::cout << "repeat=" << repeat
                      << " events=" << result.processed_events
                      << " runtime_ms=" << result.runtime_ms
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
