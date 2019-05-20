#include <iostream>
#include <mutex>
#include <atomic>
#include <streambuf>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/asio/error.hpp>

using namespace boost::asio;

class server : public boost::enable_shared_from_this<server>, boost::noncopyable {
    ip::tcp::socket sock_;
    streambuf read_buffer_;
    streambuf write_buffer_;
    bool started_;

    typedef server self_type;

    explicit server(io_service &service) : sock_(service), started_(false) {}

public:
    typedef boost::system::error_code error_code;
    typedef boost::shared_ptr<server> ptr;

    void start() {
        started_ = true;
        do_read();
    }

    static ptr new_(io_service &service) {
        ptr new_ptr(new server(service));
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
        bool found = std::find(begin, begin + bytes, '3') < begin + bytes;
        return found ? 0 : 1;
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
        std::ostringstream out;
        out << &read_buffer_;
        std::string msg(out.str());
        do_write(msg);
    }

    void on_write(const error_code &err, size_t bytes) { stop(); }

    ip::tcp::socket &sock() { return sock_; }
};

void on_connect(ip::tcp::acceptor &acceptor, io_service &service, server::ptr client,
                const boost::system::error_code &err) {
    client->start();
    server::ptr new_client = server::new_(service);
    acceptor.async_accept(new_client->sock(),
                          boost::bind(on_connect, std::ref(acceptor), std::ref(service), new_client, _1));
}

void handle_connection(io_service &service, ip::tcp::acceptor &acceptor) {
    server::ptr client = server::new_(service);
    acceptor.async_accept(client->sock(), boost::bind(on_connect, std::ref(acceptor), std::ref(service), client, _1));
}

void run_service(io_service &service) {
    service.run();
}

int main(int argc, char *argv[]) {

    io_service service;

    std::vector<ip::tcp::acceptor> acceptor_array;

    ip::tcp::acceptor acceptor(service, ip::tcp::endpoint(ip::tcp::v4(), 8001));


    for (int i = 0; i < 10; ++i) handle_connection(service, acceptor);

    boost::thread_group thread_pool;
    for (int i = 0; i < 10; ++i) {
        thread_pool.create_thread([&service]() { run_service(service); });
    }
    thread_pool.join_all();
    return 0;
}