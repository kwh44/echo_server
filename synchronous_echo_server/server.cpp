//
// Created by kwh44 on 4/7/19.
//

#include <mutex>
#include <iostream>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/asio/error.hpp>

using namespace boost::asio;

io_service service;
ip::tcp::acceptor acceptor(service, ip::tcp::endpoint(ip::tcp::v4(), 8001));

size_t read_complete(char *buff, const boost::system::error_code &err, size_t bytes) {
    if (err) return 0;
    bool found = std::find(buff, buff + bytes, '\n') < buff + bytes;
    return found ? 0 : 1;
}

void handle_connections(int id) {
    char buff[1024];
    while (true) {
        ip::tcp::socket sock(service);
        acceptor.accept(sock);
        int bytes = read(sock, buffer(buff), boost::bind(read_complete, buff, _1, _2));
        std::string msg(buff, bytes);
        sock.write_some(buffer(msg));
        sock.close();
    }
}

int main(int argc, char *argv[]) {
    boost::thread_group threads;
    for (int i = 0; i < 5; ++i) {
        threads.create_thread(boost::bind(handle_connections, i));
    }
    threads.join_all();
    return 0;
}