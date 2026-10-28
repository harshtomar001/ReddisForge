#include <iostream>
#include <string_view>
#include <vector>

#include "server/command_executor.h"
#include "server/command_processor.h"
#include "server/resp2_encoder.h"
#include "server/resp2_parser.h"

int main() {

    // const std::vector<std::string_view> components{
    //     "Storage Engine",
    //     "RESP Protocol",
    //     "TCP Server"
    // };
    //
    // std::cout << "RedisForge development environment ready!\n";
    // std::cout << "C++ standard: " << __cplusplus << '\n';
    // std::cout << "Initial components: " << components.size() << '\n';
    //
    // for (const auto component : components) {
    //     std::cout << " - " << component << '\n';
    // }

    StorageEngine storage_engine;

    Resp2Encoder resp2_encoder;
    Resp2Parser resp2_parser;

    CommandExecutor command_executor(storage_engine);

    CommandProcessor command_processor(
    resp2_encoder,
    resp2_parser,
    command_executor
    );

    std::string response =command_processor
                                        .execute("*1\r\n$4\r\nPING\r\n");

    std::cout << response;

    return 0;
}
