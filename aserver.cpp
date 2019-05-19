#include <iostream>
#include <mutex>
#include <streambuf>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/asio/error.hpp>


using namespace boost::asio;
io_service service;

class talk_to_client : public boost::enable_shared_from_this<talk_to_client>, boost::noncopyable {
    ip::tcp::socket sock_;
    streambuf read_buffer_;
    streambuf write_buffer_;
    bool started_;

    typedef talk_to_client self_type;

    explicit talk_to_client() : sock_(service), started_(false) {}

public:
    typedef boost::system::error_code error_code;
    typedef boost::shared_ptr<talk_to_client> ptr;

    void start() {
        started_ = true;
        do_read();
    }

    static ptr new_() {
        ptr new_ptr(new talk_to_client);
        return new_ptr;
    }

    void stop() {
        if (!started_) return;
        started_ = false;
        sock_.close();
    }

    size_t read_complete(const boost::system::error_code &err, size_t bytes) {
        const char *begin = buffer_cast<const char *>(read_buffer_.data());
        if (bytes == 0) return 1;
        while (bytes > 0) {
            char c = *begin;
            if (c == '3') {
                return 0;
            } else {
                --bytes;
            }
            ++begin;
        }
        return 1;
    }

    void do_read() {
        async_read(sock_, read_buffer_, boost::bind(&self_type::read_complete, shared_from_this(), _1, _2),
                   boost::bind(&self_type::on_read, shared_from_this(), _1, _2));
    }

    void do_write(const std::string &msg) {
        if (!started_) return;
        std::ostream out(&write_buffer_);
        out << msg;
        async_write(sock_, write_buffer_,
                    boost::bind(&self_type::on_write, shared_from_this(), _1, _2));
    }

    void on_read(const error_code &err, size_t bytes) {
        if (!err) {
            std::ostringstream out;
            out << &read_buffer_;
            std::string msg(out.str());
            do_write(msg + "\n");
        }
        stop();
    }

    void on_write(const error_code &err, size_t bytes) { do_read(); }

    ip::tcp::socket &sock() { return sock_; }
};

ip::tcp::acceptor acceptor(service, ip::tcp::endpoint(ip::tcp::v4(), 8001));

void handle_accept(talk_to_client::ptr client, const boost::system::error_code &err) {
    client->start();
    talk_to_client::ptr new_client = talk_to_client::new_();
    acceptor.async_accept(new_client->sock(), boost::bind(handle_accept, new_client, _1));
}

int main(int argc, char *argv[]) {
    talk_to_client::ptr client = talk_to_client::new_();
    acceptor.async_accept(client->sock(), boost::bind(handle_accept, client, _1));
    service.run();
}