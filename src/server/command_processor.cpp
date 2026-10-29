#include "command_processor.h"
#include  <exception>

CommandProcessor::CommandProcessor(
    Resp2Encoder& resp2_encoder,
    Resp2Parser& resp2_parser,
    CommandExecutor& command_executor
    )
    : resp2_encoder(resp2_encoder),
    resp2_parser(resp2_parser),
    command_executor(command_executor) {}

std::string CommandProcessor::execute(const std::string& inputString) {
    RespValue result;

    try {
        std::vector<std::string> inputCommand =
            resp2_parser.parse(inputString);

        result = command_executor.execute(inputCommand);

    } catch (const std::exception& error) {
        result = RespValue{RespError{error.what()}};
    }

    return resp2_encoder.encode(result);
}
