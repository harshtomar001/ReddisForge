#include <gtest/gtest.h>

#include "../../src/server/resp2_parser.h"

TEST(Resp2ParserTest, ParsesSimpleCommand) {
    Resp2Parser parser;

    const std::string input =
        "*1\r\n$4\r\nPING\r\n";

    const std::vector<std::string> expected = {"PING"};

    EXPECT_EQ(parser.parse(input), expected);
}

TEST(Resp2ParserTest, ParsesSetCommand) {
    Resp2Parser parser;

    const std::string input =
        "*3\r\n"
        "$3\r\nSET\r\n"
        "$4\r\nname\r\n"
        "$5\r\nHarsh\r\n";

    const std::vector<std::string> expected = {
        "SET", "name", "Harsh"
    };

    EXPECT_EQ(parser.parse(input), expected);
}

TEST(Resp2ParserTest, PreservesSpacesInsideValues) {
    Resp2Parser parser;

    const std::string input =
        "*3\r\n"
        "$3\r\nSET\r\n"
        "$4\r\nname\r\n"
        "$11\r\nHarsh Tomar\r\n";

    const std::vector<std::string> expected = {
        "SET", "name", "Harsh Tomar"
    };

    EXPECT_EQ(parser.parse(input), expected);
}

TEST(Resp2ParserTest, RejectsMissingArrayHeaderDelimiter) {
    Resp2Parser parser;

    EXPECT_THROW(
        parser.parse("*3"),
        std::invalid_argument
    );
}

TEST(Resp2ParserTest, RejectsInvalidBulkStringTerminator) {
    Resp2Parser parser;

    const std::string input =
        "*1\r\n$4\r\nPINGXX";

    EXPECT_THROW(
        parser.parse(input),
        std::invalid_argument
    );
}

TEST(Resp2ParserTest, RejectsUnexpectedTrailingData) {
    Resp2Parser parser;

    const std::string input =
        "*1\r\n$4\r\nPING\r\nEXTRA";

    EXPECT_THROW(
        parser.parse(input),
        std::invalid_argument
    );
}