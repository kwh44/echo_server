//
// Created by kwh44 on 4/7/19.
//

#include <iostream>
#include <mutex>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/asio/error.hpp>
#include <boost/bind.hpp>

#define MEM_FN(x) boost::bind(&self_type::x, shared_from_this())
#define MEM_FN1(x, y) boost::bind(&self_type::x, shared_from_this(), y)
#define MEM_FN2(x, y, z) boost::bind(&self_type::x, shared_from_this(), y, z)

using namespace boost::asio;

static io_service service;

class talk_to_svr : public boost::enable_shared_from_this<talk_to_svr>, boost::noncopyable {
    typedef talk_to_svr self_type;

    talk_to_svr(const std::string &message) : sock_(service), started_(true), message_(message) {}

    void start(ip::tcp::endpoint ep) { sock_.async_connect(ep, MEM_FN1(on_connect, _1)); }

public:
    typedef boost::system::error_code error_code;
    typedef boost::shared_ptr<talk_to_svr> ptr;

    static ptr start(ip::tcp::endpoint ep, const std::string &message) {
        ptr new_(new talk_to_svr(message));
        new_->start(ep);
        return new_;
    }

    void stop() {
        if (!started_) return;
        started_ = false;
        sock_.close();
    }

    bool started() { return started_; }

    size_t read_complete(const boost::system::error_code &err, size_t bytes) {
        if (err) return 0;
        bool found = std::find(read_buffer_, read_buffer_ + bytes, '\n') < read_buffer_ + bytes;
        return found ? 0 : 1;
    }

    void do_read() {
        async_read(sock_, buffer(read_buffer_), boost::bind(&talk_to_svr::read_complete, shared_from_this(), _1, _2),
                   MEM_FN2(on_read, _1, _2));
    }

    void do_write(const std::string &msg) {
        if (!started()) return;
        std::copy(msg.begin(), msg.end(), write_buffer_);
        sock_.async_write_some(buffer(write_buffer_, msg.size()), MEM_FN2(on_write, _1, _2));
    }

    void on_connect(const error_code &err) { if (!err) do_write(message_ + "\n"); else stop(); }

    void on_write(const error_code &err, size_t bytes) { do_read(); }

    void on_read(const error_code &err, size_t bytes) {
        if (!err) {
            std::string copy(read_buffer_, bytes - 1);
            std::cout << "Server echoed our " << message_ << ": " << (copy == message_ ? "OK" : "FAIL") << std::endl;
        }
        stop();
    }

private:
    ip::tcp::socket sock_;
    enum {
        max_msg = 1024
    };
    char read_buffer_[max_msg];
    char write_buffer_[max_msg];
    bool started_;
    std::string message_;
};


void worker_thread() {
    service.run();
};

int main(int argc, char *argv[]) {
    ip::tcp::endpoint ep(ip::address::from_string("127.0.0.1"), 8001);
    std::vector<std::string> messages = {"astana-vite, astana-vite", "vite, vite nada viyti",
                                         "astana-vite, astana-vite", "pete, pete nada viyti",
                                         "astana-vite, astana-vite", "vove, vove nada viyti"};
    for (const auto &message: messages) {
        talk_to_svr::start(ep, message);
    }
    boost::thread_group threads;
    for (int i = 0; i < 4; ++i) threads.create_thread(worker_thread);
    threads.join_all();
}
