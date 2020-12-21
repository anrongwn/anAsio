#include "asio.hpp"
#include <functional>
#include <iostream>
#include <memory>
#include <utility>
#include <cstdlib>

using asio::ip::tcp;

class session : public std::enable_shared_from_this<session> {
public:
    session(tcp::socket socket) : socket_(std::move(socket)){
        
    }

    void start(){
        do_read();
    }
private:
    void do_read(){
        auto self(shared_from_this());//keep-alive
        
        socket_.async_read_some(asio::buffer(data_, max_length), 
        [this, self](std::error_code ec, std::size_t lenght){
            if (!ec){
                do_write(lenght);
            }
        }
        );
    }

    void do_write(std::size_t length){
        auto self(shared_from_this());//keep-alive

        asio::async_write(socket_, asio::buffer(data_, length),
        [this, self] (std::error_code ec, std::size_t length){
            if (!ec){
                do_read();
            }
        }
        );
    }
private:
    tcp::socket socket_;
    enum{max_length=1024};
    char data_[max_length];
};


class echo_service{
public:
    echo_service(asio::io_context &io, short port) : acceptor_(io, tcp::endpoint(tcp::v4(), port)) {
        do_accept();
    }
    echo_service()=delete;
private:
    void do_accept(){
        acceptor_.async_accept([this](std::error_code ec, tcp::socket socket){
            if (!ec){
                std::make_shared<session>(std::move(socket))->start();//
            }

            do_accept();
        });
    }
    
    tcp::acceptor acceptor_;
};


int main(int argc, char *argv[]) {

    if (argc!=2){
        std::cerr << "Usage: ./aa_tp1 <port>\n";
        return 1;
    }

    try{
        asio::io_context io;

        echo_service svr(io, std::atoi(argv[1]));
        
	    io.run();
    }catch(std::exception& e){
        std::cerr << "Exception : " << e.what() << std::endl;
        return 1;
    }
	

	return 0;
}