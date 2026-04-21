#pragma once

#include <cstddef>
#include <memory>
#include <string>
#include "../../core/event_stream.hpp"
#include "event_lib/core/event_parser.hpp"
//////////////////////    owns the parser, take events send to user as event packet

namespace event_lib {

class DatasetEventStream : public EventStream {
public:
    explicit DatasetEventStream(const std::string& path);
    ~DatasetEventStream() override;

    bool has_next() const override;
    Event next() override;
    bool reset() override;
    void close() override;
    //    long long get_event_count() const override; //////expensive

    EventPacket next_packet(std::size_t max_events = 1024) override;

private:
    std::unique_ptr<EventParser> parser_;

};
}