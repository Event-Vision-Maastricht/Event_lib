#include "event_lib/core/event_packet.hpp"
#include "event_lib/io/stream/DatasetEventStream.hpp"
#include "event_lib/io/parser/EventParserFactory.hpp"
#include <stdexcept>
///////////////////// Ideas for future, supporting bigger files easily //////////////////////
// - Lazy Loading: Instead of preloading, read events on-demand in next().
// Keep the file open in DatasetEventStream (revert to opening it), and read chunks or one event at a time.
// This uses minimal memory but requires more I/O.
// - Streaming Mode: Add a flag to choose eager vs. lazy. For now, keep eager loading as
// it's simpler, but document the limitation.
// - Future Optimization: If needed, implement a LazyDatasetEventStream subclass or add a load_mode parameter.

namespace event_lib {
    DatasetEventStream::DatasetEventStream(const std::string& path) {
        parser_ = EventParserFactory::create_parser(path);
        parser_->open(path);
    }

    DatasetEventStream::~DatasetEventStream(){
        close();
    }

    bool DatasetEventStream::has_next() const {
        return parser_ && parser_->has_more();
    }

    Event DatasetEventStream::next() {
        EventPacket packet = next_packet(1);
        if (packet.is_empty()) {
            throw std::out_of_range("No more events in DAT stream.");
        }

        return packet.get_events().front();
    }

    bool DatasetEventStream::reset() {
        if(!parser_) {
            return false;
        }
        return parser_->reset();
    }

    void DatasetEventStream::close(){
        if(parser_){
            parser_->close();
        }
    }

    EventPacket DatasetEventStream::next_packet(std::size_t max_events){
        if(!parser_){
            throw std::runtime_error("Parser not initialized.");
        }if(!has_next()){
            throw std::out_of_range("No more event packages in stream.");
        }
        return parser_->read_packet(max_events);
    }
}