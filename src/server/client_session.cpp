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

    if (closed_) return;

    auto self = shared_from_this();

    socket_.async_read_some(
        asio::buffer(read_buffer_),
        [self](
            const asio::error_code& error ,
            std::size_t bytes_transferred
            ) {

            if (error) {
                self->close();
                return;
            }

            self->input_buffer_.append(self->read_buffer_.data() , bytes_transferred);

            self->process_input();

            if (self->closed_) return;
            self-> read();

    });
}

void ClientSession::process_input() {

    while (!input_buffer_.empty()) {

        FrameResult frameOutput =  framer_.find_frame(input_buffer_);


        if (frameOutput.status == FrameStatus::Incomplete) return;

        if (frameOutput.status == FrameStatus::Malformed) {

            close();

            return;

        }

        if (frameOutput.status == FrameStatus::Complete) {

            std::string  request = input_buffer_.substr(0 , frameOutput.frame_size );

            input_buffer_.erase(0 , frameOutput.frame_size); 

            std::string response = command_processor_.execute(request);

            write(response); //  writing the output

        }
    }
}


void ClientSession::write(std::string response) {

    if (closed_) return;

    write_queue_.push_back(std::move(response));

    if (is_writing_)  return;

    is_writing_ = true;
    write_next();

}


//[self, responseBuffer]?

// self keeps ClientSession alive.
// responseBuffer keeps the response data alive.

//async_write() is a composed operation that continues writing until the entire response is sent or an error occurs.


void ClientSession::write_next() {

    if (closed_) return;

    if (write_queue_.empty()) {
        is_writing_ = false;
        return;
    }

    auto self = shared_from_this();

    asio::async_write(
        socket_,
        asio::buffer(write_queue_.front()),

        [self](const asio::error_code& error,
               std::size_t bytes_transferred) {

            if (error) {
                self->write_queue_.clear();
                self->is_writing_ = false;
                self->close();
                return;
            }

            // Remove the response that was successfully sent.
            self->write_queue_.pop_front();

            if (self->closed_) {

                self->write_queue_.clear();
                self -> is_writing_ = false;
                return;
            }

            // Send the next queued response.
            self->write_next();
        }
    );


}


void ClientSession::close() {

    closed_ = true;

    asio::error_code error;

    socket_.close(error);


    if (!is_writing_ ) write_queue_.clear();

}