//
// Created by kwh44 on 4/7/19.
//

#include <iostream>
#include <mutex>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/asio/error.hpp>

using namespace boost::asio;

static io_service service;
static std::mutex io;
static ip::tcp::endpoint ep(ip::address::from_string("127.0.0.1"), 8001);

size_t read_complete(char *buf, const boost::system::error_code &err, size_t bytes) {
    if (err) return 0;
    bool found = std::find(buf, buf + bytes, '\n') < buf + bytes;
    return found ? 0 : 1;
}

void sync_echo(std::string msg) {
    msg += "\n";
    ip::tcp::socket sock(service);
    sock.connect(ep);
    sock.write_some(buffer(msg));
    char buf[1024];
    int bytes = read(sock, buffer(buf), boost::bind(read_complete, buf, _1, _2));
    std::string copy(buf, bytes - 1);
    msg = msg.substr(0, msg.size() - 1);
    std::lock_guard<std::mutex> lock(io);
    std::cout << "Server echoed our " << msg << ": " << (copy == msg ? "OK" : "FAIL") << std::endl;
    sock.close();
}

int main(int argc, char *argv[]) {
    std::vector<std::string> messages(1000, "astana-vite");
    boost::thread_group threads;
    for (const auto &message: messages) {
        threads.create_thread(boost::bind(sync_echo, message));
    }
    threads.join_all();
}
