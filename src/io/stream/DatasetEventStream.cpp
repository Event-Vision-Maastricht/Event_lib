#include "event_lib/core/event.hpp"
#include "event_lib/io/dataset/DatasetEventStream.hpp"
#include "event_lib/io/dataset/FileParser.hpp"
#include <vector>
#include <fstream>
#include <stdexcept>
///////////////////// Ideas for future, supporting bigger files easily //////////////////////
// - Lazy Loading: Instead of preloading, read events on-demand in next().
// Keep the file open in DatasetEventStream (revert to opening it), and read chunks or one event at a time.
// This uses minimal memory but requires more I/O.
// - Streaming Mode: Add a flag to choose eager vs. lazy. For now, keep eager loading as
// it's simpler, but document the limitation.
// - Future Optimization: If needed, implement a LazyDatasetEventStream subclass or add a load_mode parameter.



//ABSTRACT ANY OF THE FILES TO ONE TYPE, SO WHAT DO THEY ALL HAVE IN COMMON??
//---- header info (date, wtv idk)
//---- events
//---- do lazy loading, maybe read only a specific amount of events at a time???
namespace event_lib {
    DatasetEventStream::DatasetEventStream(const std::string& path) : path_(path), current_index_(0) {
        try {
            load_dataset();
        } catch (const std::exception& e) {
            throw std::runtime_error("Failed to load file: " + std::string(e.what()));
        }
    }

    bool DatasetEventStream::has_next() const {
        return current_index_ < events_.size();
    }

    Event DatasetEventStream::next() {
        if (!has_next()) {
            throw std::out_of_range("No more events in stream.");
        }
        return events_[current_index_++];
    }

    bool DatasetEventStream::reset() {
        current_index_ = 0;
        return true;
    }

    long long DatasetEventStream::get_event_count() const {
        return static_cast<long long>(events_.size());
    }

    void DatasetEventStream::load_dataset() {
        events_ = FileParser::load_events(path_);
    }
}