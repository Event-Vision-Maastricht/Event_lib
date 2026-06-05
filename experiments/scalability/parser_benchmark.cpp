#include "event_lib/io/stream/DatasetEventStream.hpp"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
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

struct MetricStats {
    double avg = 0.0;
    double mean = 0.0;
    double min = 0.0;
    double max = 0.0;
    double stddev = 0.0;
};

MetricStats calculate_stats(const std::vector<double>& values) {
    if (values.empty()) return {};

    MetricStats stats;
    stats.min = *std::min_element(values.begin(), values.end());
    stats.max = *std::max_element(values.begin(), values.end());

    double sum = 0.0;
    for (const auto value : values) sum += value;
    stats.mean = sum / static_cast<double>(values.size());
    stats.avg = stats.mean;

    double variance_sum = 0.0;
    for (const auto value : values) {
        const auto delta = value - stats.mean;
        variance_sum += delta * delta;
    }
    stats.stddev = std::sqrt(variance_sum / static_cast<double>(values.size()));
    return stats;
}

std::uint64_t rounded_u64(double value) {
    return static_cast<std::uint64_t>(std::llround(value));
}

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

void append_summary(const std::string& output_path, const std::vector<ParserResult>& results) {
    if (results.empty()) return;

    const auto& first = results.front();
    std::vector<double> runtimes;
    std::vector<double> throughputs;
    std::vector<double> memory_before;
    std::vector<double> peak_memory;
    runtimes.reserve(results.size());
    throughputs.reserve(results.size());
    memory_before.reserve(results.size());
    peak_memory.reserve(results.size());

    for (const auto& result : results) {
        runtimes.push_back(result.runtime_ms);
        throughputs.push_back(result.throughput_events_per_second);
        memory_before.push_back(static_cast<double>(result.memory_before_bytes));
        peak_memory.push_back(static_cast<double>(result.peak_memory_bytes));
    }

    const auto runtime = calculate_stats(runtimes);
    const auto throughput = calculate_stats(throughputs);
    const auto memory_before_stats = calculate_stats(memory_before);
    const auto peak_memory_stats = calculate_stats(peak_memory);

    std::ofstream out(output_path, std::ios::app);
    out << std::fixed << std::setprecision(3);
    out << "<entry"
        << " operation=\"load_parse\""
        << " warmup_runs=\"1\""
        << " measured_runs=\"" << results.size() << "\""
        << " path=\"" << xml_escape(first.path) << "\""
        << " format=\"" << xml_escape(first.format) << "\""
        << " file_size_bytes=\"" << first.file_size_bytes << "\""
        << " requested_events=\"" << first.requested_events << "\""
        << " processed_events=\"" << first.processed_events << "\""
        << " packet_size=\"" << first.packet_size << "\""
        << " width=\"" << first.width << "\""
        << " height=\"" << first.height << "\""
        << " runtime_mean_ms=\"" << runtime.mean << "\""
        << " runtime_min_ms=\"" << runtime.min << "\""
        << " runtime_max_ms=\"" << runtime.max << "\""
        << " runtime_stddev_ms=\"" << runtime.stddev << "\""
        << " throughput_events_per_second_mean=\"" << throughput.mean << "\""
        << " throughput_events_per_second_min=\"" << throughput.min << "\""
        << " throughput_events_per_second_max=\"" << throughput.max << "\""
        << " throughput_events_per_second_stddev=\"" << throughput.stddev << "\""
        << " memory_before_bytes_avg=\"" << rounded_u64(memory_before_stats.avg) << "\""
        << " memory_before_bytes_min=\"" << rounded_u64(memory_before_stats.min) << "\""
        << " memory_before_bytes_max=\"" << rounded_u64(memory_before_stats.max) << "\""
        << " peak_memory_bytes=\"" << rounded_u64(peak_memory_stats.max) << "\""
        << " peak_memory_bytes_avg=\"" << rounded_u64(peak_memory_stats.avg) << "\""
        << " peak_memory_bytes_min=\"" << rounded_u64(peak_memory_stats.min) << "\""
        << " peak_memory_bytes_max=\"" << rounded_u64(peak_memory_stats.max) << "\""
        << " peak_memory_bytes_stddev=\"" << peak_memory_stats.stddev << "\""
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
        << "  repeats      Number of measured runs after one warmup run. Default: 4.\n";
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
    const int repeats = argc >= 6 ? std::stoi(argv[5]) : 4;
    if (repeats < 1) {
        std::cerr << "repeats must be at least 1 measured run.\n";
        return 2;
    }

    try {
        std::cout << "warmup=1\n";
        const auto warmup = run_parser_benchmark(dataset_path, event_count, packet_size, 0);
        std::cout << "warmup events=" << warmup.processed_events
                  << " runtime_ms=" << warmup.runtime_ms
                  << " throughput_events_per_second=" << warmup.throughput_events_per_second
                  << " peak_memory_bytes=" << warmup.peak_memory_bytes
                  << '\n';

        std::vector<ParserResult> measured_results;
        measured_results.reserve(static_cast<std::size_t>(repeats));
        for (int repeat = 1; repeat <= repeats; ++repeat) {
            const auto result = run_parser_benchmark(dataset_path, event_count, packet_size, repeat);
            measured_results.push_back(result);
            std::cout << "repeat=" << repeat
                      << " events=" << result.processed_events
                      << " runtime_ms=" << result.runtime_ms
                      << " throughput_events_per_second=" << result.throughput_events_per_second
                      << " peak_memory_bytes=" << result.peak_memory_bytes
                      << '\n';
        }
        append_summary(output_file, measured_results);
        std::cout << "saved_summary=" << output_file << '\n';
    } catch (const std::exception& e) {
        std::cerr << "Benchmark failed: " << e.what() << '\n';
        return 1;
    }

    return 0;
}
