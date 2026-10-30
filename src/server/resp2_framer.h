#ifndef REDISFORGE_RESP2_FRAMER_H
#define REDISFORGE_RESP2_FRAMER_H
#include <cstddef>
#include <string>
#include <string_view>

// why we need frame_size?
//Because one TCP read can contain multiple requests.
//We need to know exactly how many bytes belong to the first request so we can process it and
//leave the remaining bytes in the buffer.


enum class FrameStatus {
    Complete,
    Incomplete,
    Malformed
    };

struct FrameResult {
    FrameStatus status;
    std::size_t frame_size;
};

class Resp2Framer {
public:
    FrameResult find_frame(std::string_view input) const;
};

#endif


