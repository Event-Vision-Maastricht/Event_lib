#include "event_lib/io/stream/Evk4CameraStream.hpp"

#include <stdexcept>

namespace event_lib {

    bool DVSCamera::has_next() const {
        return false;
    }

    Event DVSCamera::next() {
        throw std::runtime_error("DVSCamera is not implemented yet. DatasetEventStream is available for file-based event streams.");
    }

    EventPacket DVSCamera::next_packet(std::size_t max_events) {
        (void)max_events;
        throw std::runtime_error("DVSCamera is not implemented yet. Add a camera SDK-backed implementation before using this stream.");
    }

}
