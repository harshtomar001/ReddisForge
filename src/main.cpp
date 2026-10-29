#include <asio.hpp>
#include <exception>
#include <iostream>
#include "core/storage_engine.h"
#include "server/command_executor.h"
#include "server/command_processor.h"
#include "server/resp2_encoder.h"
#include "server/resp2_parser.h"
#include "server/tcp_server.h"

int main() {
    try {
        asio::io_context io_context;


        // 1. Create the shared storage engine.
        StorageEngine storage_engine;

        // 2. Create the command executor using that storage engine.
        CommandExecutor command_executor(storage_engine);

        // 3. Create the RESP2 parser and encoder.
        Resp2Parser resp2_parser;
        Resp2Encoder resp2_encoder;

        // 4. Connect the parser, encoder, and executor.
        CommandProcessor command_processor(
            resp2_encoder,
            resp2_parser,
            command_executor
        );

        // 5. Inject the processor into the TCP server.
        TcpServer server(
            io_context,
            6379,
            command_processor
        );

        server.start();

        // Process asynchronous networking operations.
        io_context.run();

    } catch (const std::exception& error) {
        std::cerr << "RedisForge server error: "
                  << error.what() << '\n';

        return 1;
    }

    return 0;


}