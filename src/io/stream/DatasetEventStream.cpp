#include "event_lib/core/event_packet.hpp"
#include "event_lib/io/stream/DatasetEventStream.hpp"
#include "event_lib/io/parser/EventParserFactory.hpp"
#include <iostream>
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
        std::cerr << "DEBUG: DatasetEventStream constructor called with path: " << path << std::endl;
        parser_ = EventParserFactory::create_parser(path);
        std::cerr << "DEBUG: Parser created successfully" << std::endl;
        parser_->open(path);
        std::cerr << "DEBUG: Parser opened file successfully" << std::endl;
        metadata_ptr_ = &parser_->header();
        std::cerr << "DEBUG: After open - width=" << metadata_ptr_->width << ", height=" << metadata_ptr_->height
            << ", date='" << metadata_ptr_->date << "'" << std::endl;
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

    const SensorMetadata& DatasetEventStream::metadata() const {
        if (!metadata_ptr_) {
            throw std::runtime_error("DatasetEventStream metadata is not available.");
        }
        return *metadata_ptr_;
    }

    int DatasetEventStream::get_width(){
        return metadata().width;
    }

    int DatasetEventStream::get_height(){
        return metadata().height;
    }

    std::string DatasetEventStream::get_date(){
        return metadata().date;
    }

    std::string DatasetEventStream::get_init_recording_time(){
        return metadata().time;
    }

    std::string DatasetEventStream::get_version(){
        return metadata().version;
    }

    std::string DatasetEventStream::get_event_type(){
        return metadata().event_type;
    }
}