#pragma once
#include <string>
#include <vector>
#include <fstream>
#include "../../core/event_stream.hpp"

namespace event_lib {

class DatasetEventStream : public EventStream {
public:
    explicit DatasetEventStream(const std::string& path);
    ~DatasetEventStream() override;

    bool has_next() const override;
    Event next() override;
    bool reset() override;
    void close() override;
    long long get_event_count() const override;

private:
    std::string path_;
    std::ifstream file_;
    std:: vector<Event> events_;
    std::size_t current_index = 0;

    void load_dataset();
};
}