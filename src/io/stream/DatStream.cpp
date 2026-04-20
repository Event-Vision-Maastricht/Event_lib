#include "event_lib/io/stream/DatStream.hpp"
#include "event_lib/io/parser/FileParser.hpp"
#include <stdexcept>

namespace event_lib {

DatStream::DatStream(const std::string& path) : path_(path) {
    load_events();
}

bool DatStream::has_next() const {
    return current_index_ < events_.size();
}

Event DatStream::next() {
    if (!has_next()) {
        throw std::out_of_range("No more events in DAT stream.");
    }

    return events_[current_index_++];
}

bool DatStream::reset() {
    current_index_ = 0;
    return true;
}

long long DatStream::get_event_count() const {
    return static_cast<long long>(events_.size());
}

void DatStream::close() {
    events_.clear();
    current_index_ = 0;
}

void DatStream::load_events() {
    if (!FileParser::is_supported(path_)) {
        throw std::runtime_error("Only .dat files are supported for now: " + path_);
    }

    events_ = FileParser::load_events(path_);
}

}
