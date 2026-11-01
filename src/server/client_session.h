#ifndef REDISFORGE_CLIENT_SESSION_H
#define REDISFORGE_CLIENT_SESSION_H
#include <asio.hpp>
#include <array>
#include <memory>
#include <string>
#include "command_processor.h"
#include "resp2_framer.h"


class ClientSession
    : public std::enable_shared_from_this<ClientSession> {

public:

    ClientSession(
        asio::ip::tcp::socket socket,
        CommandProcessor& command_processor
    );

    void start();

private:

    void read();

    void process_input();

    void write(std::string response);

    asio::ip::tcp::socket socket_;
    CommandProcessor& command_processor_;
    Resp2Framer framer_;
    std::string input_buffer_;

    std::array<char, 4096> read_buffer_{};
};

#endif