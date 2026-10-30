#include "../../src/server/resp2_framer.h"

#include <gtest/gtest.h>


TEST(Resp2FramerTEST, EMPTYINPUT ) {

    Resp2Framer resp2_framer;
    std::string_view input = "";

    FrameResult frame_result = resp2_framer.find_frame(input);

    EXPECT_EQ(frame_result.status , FrameStatus::Incomplete);

}

TEST(Resp2FramerTEST, PartialArrayHeader ) {

    Resp2Framer resp2_framer;
    std::string_view input = "*2";

    FrameResult frame_result = resp2_framer.find_frame(input);

    EXPECT_EQ(frame_result.status , FrameStatus::Incomplete);

}


TEST(Resp2FramerTEST, InvalidArrayHeader ) {

    Resp2Framer resp2_framer;
    std::string_view input = "!2\r\n";

    FrameResult frame_result = resp2_framer.find_frame(input);

    EXPECT_EQ(frame_result.status , FrameStatus::Malformed);

}


TEST(Resp2FramerTEST, CompletePing ) {

    Resp2Framer resp2_framer;
    std::string_view input = "*1\r\n$4\r\nPING\r\n";

    FrameResult frame_result = resp2_framer.find_frame(input);

    EXPECT_EQ(frame_result.status , FrameStatus::Complete);
    EXPECT_EQ(frame_result.frame_size, 14);

}

TEST(Resp2FramerTEST, MulCompleteRequest ) {

    Resp2Framer resp2_framer;
    std::string_view input = "*2\r\n$4\r\nPING\r\n$4\r\nPING\r\n";

    FrameResult frame_result = resp2_framer.find_frame(input);

    EXPECT_EQ(frame_result.status , FrameStatus::Complete);
    EXPECT_EQ(frame_result.frame_size, 24);

}

TEST(Resp2FramerTEST, IncompleteBulkString ) {

    Resp2Framer resp2_framer;
    std::string_view input = "*1\r\n$5\r\nHEL";

    FrameResult frame_result = resp2_framer.find_frame(input);

    EXPECT_EQ(frame_result.status , FrameStatus::Incomplete);

}


TEST(Resp2FramerTEST, InvalidBulkTerminator ) {

    Resp2Framer resp2_framer;
    std::string_view input = "*1\r\n$3\r\nGETXX";

    FrameResult frame_result = resp2_framer.find_frame(input);

    EXPECT_EQ(frame_result.status , FrameStatus::Malformed);

}
