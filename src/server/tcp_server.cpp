#include "tcp_server.h"
#include <iostream>
#include <stdexcept>

TcpServer::TcpServer(
    asio::io_context& io_context, // The object Asio uses to manage and execute asynchronous I/O operations.
    unsigned short port, // The TCP port on which the server should listen, such as 6379.
    CommandProcessor& command_processor
)
    : acceptor_(
          io_context,
          asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port)
      ),
command_processor_(command_processor){}

//An acceptor is the Asio object responsible for accepting incoming TCP connections.

void TcpServer::start() {

    std::cout << "RedisForge TCP server listening on port "
              << acceptor_.local_endpoint().port()
              << '\n';

    start_accept();

}

void TcpServer::start_accept() {

    // async_accept() asks Asio to accept a client without blocking the current thread while waiting.
    // The lambda is a callback that runs when the connection attempt completes.
    // error tells us whether the operation succeeded.

    // socket represents the connected client.
    // This socket represents an established TCP connection between RedisForge and a client

    // [this] lets the lambda access members and methods of the current TcpServer object.
    // Calling start_accept() again allows the server to accept another client.

    acceptor_.async_accept(
        [this](const asio::error_code& error,
               asio::ip::tcp::socket socket) {
            if (!error) {

                std::cout << "Client connected: "
                          << socket.remote_endpoint()
                          << '\n';

                handle_client(std::move(socket));

            } else {
                std::cerr << "Accept failed: "
                          << error.message()
                          << '\n';
            }

            start_accept();
        }
    );
}

//std::move() — transfers resources instead of copying them.
// std::make_shared() — creates an object managed by a shared-ownership smart pointer.
// Passing std::move(socket) to a function — transfers ownership of the socket into that function.

void TcpServer::handle_client(asio::ip::tcp::socket socket) {

    auto client =std::make_shared<asio::ip::tcp::socket>(
        std::move(socket)
        );
    //Creates a socket object managed by a shared_ptr.
    //Allows the newly created socket object to be constructed by moving resources from the existing socket.

    // A connected socket arrives as the function parameter.
    // make_shared allocates storage for a new socket object.
    // The new socket is move-constructed from the existing socket.
    // client now owns a shared_ptr to that socket object.
    // The original function parameter is destroyed when appropriate, but the connection's ownership has already transferred.

    auto buffer = std::make_shared<std::array<char, 1024>>();

    //Here, *buffer dereferences the shared pointer to access the actual array.
    // Asio then uses that array as the destination for incoming bytes.

    client->async_read_some(
        asio::buffer(*buffer),
        [client, buffer](const asio::error_code& error,
                         std::size_t bytes_received) {
            if (error) {
                if (error != asio::error::eof) {
                    std::cerr << "Read failed: "
                              << error.message()
                              << '\n';
                }
                return;
            }

            std::cout << "Received " << bytes_received
                      << " bytes from client.\n";

            const std::string response = "+PONG\r\n";

            asio::async_write(
                *client,
                asio::buffer(response),
                [client, response](
                    const asio::error_code& write_error,
                    std::size_t /*bytes_sent*/) {

                    if (write_error) {
                        std::cerr << "Write failed: "
                                  << write_error.message()
                                  << '\n';
                    }

                }
            );
        }
    );
}

//async_accept callback
// |
// | receives connected socket
// v
// handle_client(std::move(socket))
// |
// | transfers socket into shared ownership
// v
// std::make_shared<tcp::socket>(std::move(socket))
// |
// | creates managed socket object
// v
// client shared_ptr
// |
// | captured by asynchronous callback
// v
// async_read_some()
// |
// | operation completes later
// v
// Callback uses client and buffer