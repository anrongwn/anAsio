#include "asio.hpp"
#include <functional>
#include <iostream>
#include <memory>
#include <utility>
#include <cstdlib>

using asio::ip::tcp;

//
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

        //asio每次最多读 max_length 字节数，注意 max_length 的长度，以保证吞吐量，尽量减少io
		socket_.async_read_some(asio::buffer(data_, max_length), [this, self](const asio::error_code& ec, std::size_t lenght) {
			if (!ec){
                std::cout << "***async_read_some data=" << std::string(data_, lenght) << ", lenght=" << lenght << std::endl;

                do_write(lenght);
            }
		});
	}

	void do_write(std::size_t length){
        auto self(shared_from_this());//keep-alive

		asio::async_write(socket_, asio::buffer(data_, length), [this, self](const asio::error_code& ec, std::size_t length) {
			if (!ec){
                std::cout << "***async_write data=" << std::string(data_, length) << ", length=" << length << std::endl;

                do_read();
            }
		});
	}
private:
    tcp::socket socket_;
    enum{max_length=65536};//保证足够的吞吐，但占用内存。
    char data_[max_length];
};


class echo_service{
public:
    echo_service(asio::io_context &io, short port) : acceptor_(io, tcp::endpoint(tcp::v4(), port)) {
		acceptor_.set_option(asio::socket_base::reuse_address(true));
		do_accept();
    }
    echo_service()=delete;
private:
    void do_accept(){
		acceptor_.async_accept([this](const asio::error_code& ec, tcp::socket socket) {
			if (!ec){
                auto peer =socket.remote_endpoint();
                std::cout<<"=====echo_service accept from: " << peer.address() << ":" << peer.port() << "(" << peer.protocol().type() <<")."<<std::endl;

                std::make_shared<session>(std::move(socket))->start();//启动当前session读写
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