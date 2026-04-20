#pragma once
#include <string>
#include <vector>
#include "../../core/event_stream.hpp"


namespace event_lib {

class DatasetEventStream : public EventStream {
public:
    explicit DatasetEventStream(const std::string& path);

    /**
     * @brief checks if there exists a next event to read
     */
    bool has_next() const override;

    /**
     * @brief reads the next event
     */
    Event next() override;

    /**
     * @brief 
     * @return 
     */
    bool reset() override;

    /**
     * @brief 
     * @return 
     */
    long long get_event_count() const override;

private:
    std::string path_;
    std:: vector<Event> events_;
    std::size_t current_index_ = 0;

    void load_dataset();
};
}