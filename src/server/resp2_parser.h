#ifndef REDISFORGE_RESP2_PARSER_H
#define REDISFORGE_RESP2_PARSER_H
#include <string>
#include <vector>


class Resp2Parser {
public:
    std::vector<std::string> parse(const std::string& input);
};


#endif //REDISFORGE_RESP2_PARSER_H