#ifndef REDISFORGE_RESP_VALUE_H
#define REDISFORGE_RESP_VALUE_H
#include <vector>
#include <string>
#include <variant>
#include <memory>


struct SimpleString {
    std::string value;
};

struct BulkString {
    std::string value;
};

struct RespError {
    std::string message;
};

struct RespValue {
    using Array = std::vector<RespValue>;

    using Data = std::variant<

        SimpleString,
        BulkString,
        long long,
        RespError,
        std::monostate,
        std::shared_ptr<Array>

    >;

    Data data;
};

#endif // REDISFORGE_RESP_VALUE_H