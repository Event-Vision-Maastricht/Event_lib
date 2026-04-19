#include "event_lib/core/event.hpp"
#include "event_lib/io/dataset/DatasetEventStream.hpp"
#include <vector>
#include <fstream>
#include <stdexcept>

namespace event_lib {
    DatasetEventStream::DatasetEventStream(const std::string& path) : path_(path), current_index_(0) {
        try {
            file_.open(path_, std::ios::binary);
            if (!file_.is_open()) {
                throw std::runtime_error("Failed to open file: " + path_);
            }
            load_dataset();///////////////////////////////////////////
        } catch (const std::exception& e) {
            throw std::runtime_error("DatasetEventStream constructor failed: " + std::string(e.what()));
        }
    }

    DatasetEventStream::~DatasetEventStream() {
        close();
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
        try {
            if (!file_.is_open()) return false;
            current_index_ = 0;
            file_.clear();
            file_.seekg(0, std::ios::beg);
            if (file_.fail()) {
                return false;
            }
            return true;
        } catch (const std::exception& e) {
            throw std::runtime_error("DatasetEventStream reset failed: " + std::string(e.what()));
            return false;
        }
    }

    void DatasetEventStream::close() {
        if (file_.is_open()) {
            file_.close();
        }
    }

    long long DatasetEventStream::get_event_count() const {
        return static_cast<long long>(events_.size());
    }

    void DatasetEventStream::load_dataset() {
        // Placeholder: Implement actual dataset loading logic here
        // For now, assume events are loaded into events_ vector
        // Example: Read binary data and populate events_
        // This depends on your dataset format
    }
}