#include <gtest/gtest.h>
#include "../../src/server/resp2_encoder.h"
#include <stdexcept>
#include <string>
#include "../../src/server/resp2_value.h"
#include <memory>
#include <vector>


TEST(Resp2EncoderTest, EncodesSimpleString) {
    Resp2Encoder encoder;
    RespValue response{SimpleString{"PONG"}};

    EXPECT_EQ(encoder.encode(response), "+PONG\r\n");
}

TEST(Resp2EncoderTest, EncodesSimpleStringOk) {
    Resp2Encoder encoder;
    RespValue response{SimpleString{"OK"}};

    EXPECT_EQ(encoder.encode(response), "+OK\r\n");
}

TEST(Resp2EncoderTest, RejectsLineBreakInSimpleString) {
    Resp2Encoder encoder;
    RespValue response{SimpleString{"hello\r\nworld"}};

    EXPECT_THROW(
        encoder.encode(response),
        std::invalid_argument
    );
}

TEST(Resp2EncoderTest, EncodesPositiveInteger) {
    Resp2Encoder encoder;
    RespValue response{42LL};

    EXPECT_EQ(encoder.encode(response), ":42\r\n");
}

TEST(Resp2EncoderTest, EncodesZero) {
    Resp2Encoder encoder;
    RespValue response{0LL};

    EXPECT_EQ(encoder.encode(response), ":0\r\n");
}

TEST(Resp2EncoderTest, EncodesNegativeInteger) {
    Resp2Encoder encoder;
    RespValue response{-7LL};

    EXPECT_EQ(encoder.encode(response), ":-7\r\n");
}

TEST(Resp2EncoderTest, BulkString) {
    Resp2Encoder encoder;
    RespValue response{ BulkString{"Harsh"} };

    EXPECT_EQ(encoder.encode(response), "$5\r\nHarsh\r\n");
}

TEST(Resp2EncoderTest, EmptyBulkString) {
    Resp2Encoder encoder;
    RespValue response{ BulkString{""} };

    EXPECT_EQ(encoder.encode(response), "$0\r\n\r\n");

}

TEST(Resp2EncoderTest, EmbeddedCRLFBulkString) {
    Resp2Encoder encoder;
    RespValue response{ BulkString{"A\r\nB"} };

    EXPECT_EQ(encoder.encode(response), "$4\r\nA\r\nB\r\n");

}

TEST(Resp2EncoderTest, Monostate) {
    Resp2Encoder encoder;
    RespValue response{std::monostate{}};

    EXPECT_EQ(encoder.encode(response), "$-1\r\n");

}

TEST(Resp2EncoderTest, RespError) {
    Resp2Encoder encoder;

    RespValue response{RespError{"unknown command"}};
    EXPECT_EQ(
        encoder.encode(response),
        "-ERR unknown command\r\n"
    );

}


TEST(Resp2EncoderTest, EncodesEmptyArray) {
    Resp2Encoder encoder;
    RespValue response{
        std::make_shared<std::vector<RespValue>>()
    };

    EXPECT_EQ(encoder.encode(response), "*0\r\n");
}

TEST(Resp2EncoderTest, EncodesArrayOfBulkStrings) {
    Resp2Encoder encoder;

    auto values = std::make_shared<std::vector<RespValue>>();

    values->push_back(RespValue{BulkString{"cat"}});
    values->push_back(RespValue{BulkString{"dog"}});

    RespValue response{values};

    EXPECT_EQ(
        encoder.encode(response),
        "*2\r\n"
        "$3\r\ncat\r\n"
        "$3\r\ndog\r\n"
    );
}

TEST(Resp2EncoderTest, EncodesArrayContainingNull) {
    Resp2Encoder encoder;
    auto values = std::make_shared<std::vector<RespValue>>();

    values->push_back(RespValue{BulkString{"Harsh"}});
    values->push_back(RespValue{std::monostate{}});

    RespValue response{values};

    EXPECT_EQ(
        encoder.encode(response),
        "*2\r\n"
        "$5\r\nHarsh\r\n"
        "$-1\r\n"
    );
}

TEST(Resp2EncoderTest, EncodesNestedArray) {
    Resp2Encoder encoder;
    auto inner = std::make_shared<std::vector<RespValue>>();

    inner->push_back(RespValue{ BulkString{"dog"}});

    auto outer = std::make_shared<std::vector<RespValue>>();

    outer->push_back(RespValue{BulkString{"cat"}});
    outer->push_back(RespValue{std::shared_ptr<std::vector<RespValue>>{inner}});

    RespValue response{outer};

    EXPECT_EQ(
        encoder.encode(response),
        "*2\r\n"
        "$3\r\ncat\r\n"
        "*1\r\n"
        "$3\r\ndog\r\n"
    );
}

TEST(Resp2EncoderTest, RejectsErrorMessageContainingCRLF) {
    Resp2Encoder encoder;
    RespValue response{RespValue{RespError{"invalid\r\n+OK"}}};

    EXPECT_THROW(
        encoder.encode(response),
        std::invalid_argument
    );
}

TEST(Resp2EncoderTest, RejectsNullArrayPointer) {
    Resp2Encoder encoder;
    std::shared_ptr<std::vector<RespValue>> values;

    RespValue response{values};

    EXPECT_THROW(
        encoder.encode(response),
        std::invalid_argument
    );
}
