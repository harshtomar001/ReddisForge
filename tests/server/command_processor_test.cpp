#include <gtest/gtest.h>
#include "../../src/server/command_executor.h"
#include "../../src/server/command_processor.h"
#include "../../src/server/resp2_encoder.h"
#include "../../src/server/resp2_parser.h"


TEST(CommandProcessorTest, CommandProcessorTest_PING) {
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

    EXPECT_EQ(response, "+PONG\r\n");

}

TEST(CommandProcessorTest, Unknown_Command) {
    StorageEngine storage_engine;

    Resp2Encoder resp2_encoder;
    Resp2Parser resp2_parser;
    CommandExecutor command_executor(storage_engine);

    CommandProcessor command_processor(
        resp2_encoder,
        resp2_parser,
        command_executor
    );

    std::string response =
        command_processor.execute("*1\r\n$6\r\nFOOBAR\r\n");

    EXPECT_EQ(response.substr(0, 5), "-ERR ");
}

TEST(CommandProcessorTest, SET_Returns_OK) {
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
                                        .execute("*3\r\n$3\r\nSET\r\n$4\r\nname\r\n$5\r\nHarsh\r\n");

    EXPECT_EQ(response, "+OK\r\n");

}

TEST(CommandProcessorTest, CommandProcessorTest_GET) {
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
                                        .execute("*2\r\n$3\r\nGET\r\n$4\r\nname\r\n");

    EXPECT_EQ(response, "$-1\r\n");

}

TEST(CommandProcessorTest, GET_After_SET) {
    StorageEngine storage_engine;

    Resp2Encoder resp2_encoder;
    Resp2Parser resp2_parser;
    CommandExecutor command_executor(storage_engine);

    CommandProcessor command_processor(
        resp2_encoder,
        resp2_parser,
        command_executor
    );

    // First, store the key.
    std::string set_response = command_processor.execute(
        "*3\r\n$3\r\nSET\r\n$4\r\nname\r\n$5\r\nHarsh\r\n"
    );

    EXPECT_EQ(set_response, "+OK\r\n");

    // Now retrieve the same key.
    std::string get_response = command_processor.execute(
        "*2\r\n$3\r\nGET\r\n$4\r\nname\r\n"
    );

    EXPECT_EQ(get_response, "$5\r\nHarsh\r\n");
}

TEST(CommandProcessorTest, CommandProcessorTest_GET_MISSING) {
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
                                        .execute("*2\r\n$3\r\nGET\r\n$7\r\nmissing\r\n");

    EXPECT_EQ(response, "$-1\r\n");

}

TEST(CommandProcessorTest, CommandProcessorTest_INCR) {
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
                                        .execute("*2\r\n$4\r\nINCR\r\n$7\r\ncounter\r\n");

    EXPECT_EQ(response, ":1\r\n");

}

TEST(CommandProcessorTest, Malformed_RESP2_Request) {
    StorageEngine storage_engine;
    Resp2Encoder resp2_encoder;
    Resp2Parser resp2_parser;
    CommandExecutor command_executor(storage_engine);

    CommandProcessor command_processor(
        resp2_encoder,
        resp2_parser,
        command_executor
    );

    // Declared bulk-string length is 4, but only 3 characters are supplied.
    std::string response = command_processor.execute(
        "*1\r\n$4\r\nPIN\r\n"
    );

    EXPECT_EQ(response.substr(0, 5), "-ERR ");
}

TEST(CommandProcessorTest, INCR_Non_Integer_Value) {
    StorageEngine storage_engine;
    Resp2Encoder resp2_encoder;
    Resp2Parser resp2_parser;
    CommandExecutor command_executor(storage_engine);

    CommandProcessor command_processor(
        resp2_encoder,
        resp2_parser,
        command_executor
    );

    // Store a non-integer value.
    EXPECT_EQ(
        command_processor.execute(
            "*3\r\n$3\r\nSET\r\n$5\r\ncount\r\n$5\r\nhello\r\n"
        ),
        "+OK\r\n"
    );

    // INCR must return an error instead of propagating the exception.
    std::string response = command_processor.execute(
        "*2\r\n$4\r\nINCR\r\n$5\r\ncount\r\n"
    );

    EXPECT_EQ(response.substr(0, 5), "-ERR ");
}