#include "client_session.h"


ClientSession::ClientSession(asio::ip::tcp::socket socket,
    CommandProcessor &command_processor
    ):

    socket_(std::move(socket)),
    command_processor_(command_processor){}


void ClientSession::start() {
    read();
}

void ClientSession::read() {

    auto self = shared_from_this();

    socket_.async_read_some(
        asio::buffer(read_buffer_),
        [self](
            const asio::error_code& error ,
            std::size_t bytes_transferred
            ) {

            if (error) {

                return;
            }

            self->input_buffer_.append(self->read_buffer_.data() , bytes_transferred);

            self->process_input();

    });
}

void ClientSession::process_input() {

    FrameResult frameOutput =  framer_.find_frame(input_buffer_);

    if (frameOutput.status == FrameStatus::Incomplete) return;

    if (frameOutput.status == FrameStatus::Malformed) {

        return;

    }

    if (frameOutput.status == FrameStatus::Complete) {

        std::string  request = input_buffer_.substr(0 , frameOutput.frame_size );

        std::string response = command_processor_.execute(request);

        input_buffer_.erase(0 , frameOutput.frame_size); //  remove the processed request after getting the response
    }
}
