#include <iostream>
#include <streambuf>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/asio/error.hpp>
#include <boost/asio/coroutine.hpp>
#include <boost/asio/yield.hpp>

using namespace boost::asio;

class server : public boost::enable_shared_from_this<server>, public coroutine, boost::noncopyable {
    ip::tcp::socket sock_;
    streambuf read_buffer_;
    const char message_end_sign = '3'; // because we live in 3d space ;=)
    typedef server self_type;

    explicit server(io_service &service) : sock_(service) {}

public:
    typedef boost::system::error_code error_code;
    typedef boost::shared_ptr<self_type> ptr;

    static ptr new_(io_service &service) {
        ptr new_ptr(new server(service));
        return new_ptr;
    }

    void stop() { sock_.close(); }

    size_t read_complete(const boost::system::error_code &err, size_t bytes) {
        const char *begin = buffer_cast<const char *>(read_buffer_.data());
        if (bytes == 0) return 1;
        bool found = std::find(begin, begin + bytes, message_end_sign) < begin + bytes;
        return found ? 0 : 1;
    }

    ip::tcp::socket &sock() { return sock_; }

    void step(const error_code &err, size_t bytes = 0) {
        reenter(this) {
                        yield async_read(sock_, read_buffer_,boost::bind(&self_type::read_complete,shared_from_this(), _1, _2),
                                                                           boost::bind(&self_type::step,shared_from_this(), _1, _2));
                        yield async_write(sock_, read_buffer_,boost::bind(&self_type::step,shared_from_this(), _1, _2));
                        stop();
                    }
    }

    static
    void on_connect(ip::tcp::acceptor &acceptor, io_service &service, server::ptr client,
                    const boost::system::error_code &err) {
        client->step(err);
        server::ptr new_client = server::new_(service);
        acceptor.async_accept(new_client->sock(),
                              boost::bind(server::on_connect, std::ref(acceptor), std::ref(service), new_client, _1));
    }
};

int main(int argc, char *argv[]) {
    io_service service;
    constexpr int port_number = 8001;
    ip::tcp::acceptor acceptor(service, ip::tcp::endpoint(ip::tcp::v4(), port_number));
    // create ten instances of server ready to accept message
    for (int i = 0; i < 10; ++i) {
        server::ptr client = server::new_(service);
        acceptor.async_accept(client->sock(),
                              boost::bind(server::on_connect, std::ref(acceptor), std::ref(service), client, _1));
    }
    boost::thread_group thread_pool;
    // create ten working threads that will do the work assigned to them by the io_service instance captured
    for (int i = 0; i < 10; ++i) {
        thread_pool.create_thread([&service]() { service.run(); });
    }
    std::cout << "Server is running.\nYou can echo you message to http://localhost:" << port_number << "/\n";
    thread_pool.join_all();
    return 0;
}
