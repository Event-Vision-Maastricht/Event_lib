#pragma once

#include <string>
#include <vector>
#include "../../core/event_stream.hpp"

namespace event_lib {

class DatEventStream : public EventStream {
public:
    explicit DatEventStream(const std::string& path);

    bool has_next() const override;
    Event next() override;
    bool reset() override;
    long long get_event_count() const override;
    void close() override;

private:
    std::string path_;
    std::vector<Event> events_;
    std::size_t current_index_ = 0;

    void load_events();
};

}
