#ifndef REDISFORGE_RESP2_ENCODER_H
#define REDISFORGE_RESP2_ENCODER_H
#include <string>
#include "resp2_value.h"

class Resp2Encoder {
public:
    std::string encode(const RespValue& response) const;
};

#endif // REDISFORGE_RESP2_ENCODER_H
