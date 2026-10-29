#ifndef REDISFORGE_TCP_SERVER_H
#define REDISFORGE_TCP_SERVER_H
#include <asio.hpp>
#include "command_processor.h"

class TcpServer {
public:
    TcpServer(
        asio::io_context& io_context,
        unsigned short port,
        CommandProcessor& command_processor_
        );

    void start();

private:

    void start_accept();

    void handle_client(asio::ip::tcp::socket socket);

    asio::ip::tcp::acceptor acceptor_;
    CommandProcessor& command_processor_;
};

#endif // REDISFORGE_TCP_SERVER_H

