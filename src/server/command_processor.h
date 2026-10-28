#ifndef REDISFORGE_COMMAND_PROCESSOR_H
#define REDISFORGE_COMMAND_PROCESSOR_H

#include "command_executor.h"
#include "resp2_encoder.h"
#include "resp2_parser.h"

#include <string>


class CommandProcessor {
private:
    Resp2Encoder& resp2_encoder;
    Resp2Parser& resp2_parser;
    CommandExecutor& command_executor;

public:
    CommandProcessor(
    Resp2Encoder& resp2_encoder,
    Resp2Parser& resp2_parser,
    CommandExecutor& command_executor
    );


    std::string execute(const std::string& inputString);


    };

#endif // REDISFORGE_COMMAND_PROCESSOR_H


