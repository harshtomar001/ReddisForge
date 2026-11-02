#ifndef REDISFORGE_CLIENT_SESSION_H
#define REDISFORGE_CLIENT_SESSION_H
#include <asio.hpp>
#include <array>
#include <memory>
#include <deque>
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

    void close();

    bool closed_ =  false;

    asio::ip::tcp::socket socket_;
    CommandProcessor& command_processor_;
    Resp2Framer framer_;
    std::string input_buffer_;

    std::array<char, 4096> read_buffer_{};

    std::deque<std::string> write_queue_;

    bool is_writing_ = false;

    void write_next();

};

#endif