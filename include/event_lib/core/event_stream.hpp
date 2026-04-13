#include "event.hpp"

namespace event_lib{

    class EventStream{
        public:
            virtual ~EventStream() = default;
            virtual bool has_next() const = 0;
            virtual Event next() =0;
    };
}