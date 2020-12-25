#include "asio.hpp"
#include "an_spawn.h"
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <utility>

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
			} else {
				socket_.close();

				std::cerr << "***async_read_some data error, " << ec.message() << std::endl;
			}
		});
	}

	void do_write(std::size_t length){
        auto self(shared_from_this());//keep-alive

		asio::async_write(socket_, asio::buffer(data_, length), [this, self](const asio::error_code& ec, std::size_t length) {
			if (!ec){
				std::cout << "***async_write data=" << std::string(data_, length) << ", length=" << length << std::endl;

				do_read();
			} else {
				socket_.close();

				std::cerr << "***async_write data error, " << ec.message() << std::endl;
			}
		});
	}
private:
    tcp::socket socket_;
    enum{max_length=65536};//保证足够的吞吐，但占用内存。
	char data_[max_length];

	std::array<char, max_length> data2_;
};


class echo_service{
public:
  explicit echo_service(const std::string &addr, short port) : io_(2), acceptor_(io_), signals_(io_) {

	  signals_.add(SIGTERM);
	  signals_.add(SIGPIPE);
	  signals_.add(SIGINT);

	  do_await_stop();

	  //
	  asio::ip::address add;
	  add.from_string(addr);
	  tcp::endpoint ep(add, port);
	  acceptor_.open(ep.protocol());
	  acceptor_.set_option(asio::socket_base::reuse_address(true));
	  acceptor_.bind(ep);
	  acceptor_.listen();

	  do_accept();
  }
	echo_service()=delete;
	echo_service(const echo_service &) = delete;
	echo_service &operator=(const echo_service &) = delete;
	echo_service(echo_service &&) = delete;
	echo_service &operator=(echo_service &&) = delete;

	asio::io_context::count_type run() { return io_.run(); }

	void signal_handler(const asio::error_code &ec, int signal_num) {
		if (!ec) {
			switch (signal_num) {
			case SIGINT:
				std::cout << "signal_handler reciv SIGINT(" << signal_num << "),exit." << std::endl;
				acceptor_.close();
				io_.stop();
				// exit(-1);
				break;
			case SIGTERM:
				std::cout << "signal_handler reciv SIGTERM(" << signal_num << "),exit." << std::endl;
				acceptor_.close();
				io_.stop();
				// exit(-1);
				break;
			case SIGPIPE:
				std::cout << "signal_handler reciv SIGPIPE(" << signal_num << "),ignore." << std::endl;
				break;
			default:
				std::cout << "signal_handler reciv other(" << signal_num << "),exit." << std::endl;
				acceptor_.close();
				io_.stop();
				exit(-1);
				break;
			}
		}
	}

private:
    void do_accept(){
		acceptor_.async_accept([this](const asio::error_code &ec, tcp::socket socket) {
			if (!acceptor_.is_open()) {
				return;
			}

			if (!ec){
                auto peer =socket.remote_endpoint();
                std::cout<<"=====echo_service accept from: " << peer.address() << ":" << peer.port() << "(" << peer.protocol().type() <<")."<<std::endl;

				std::make_shared<session>(std::move(socket))->start(); //启动当前session读写

				do_accept();
			} else {
				std::cerr << "=====echo_service accept error, " << ec.message() << std::endl;
			}
		});
	}

	void do_await_stop() {
		signals_.async_wait(
			std::bind(&echo_service::signal_handler, this, std::placeholders::_1, std::placeholders::_2));
	}

  private:
	asio::io_context io_;
	tcp::acceptor acceptor_;
	asio::signal_set signals_;
};

int main(int argc, char *argv[]) {

	if (argc != 3) {
		std::cerr << "Usage: ./aa_tp1 <address> <port>\n";
		return 1;
	}

	try{
		
		/***
		// asio::io_context io(1);

		// asio::signal_set signals(io, SIGINT, SIGTERM, SIGPIPE);
		// signals.add(SIGTERM);
		// signals.add(SIGPIPE);
		// signals.add(SIGINT);

		echo_service svr(std::string(argv[1]), std::atoi(argv[2]));

		// signals.async_wait(
		//	std::bind(&echo_service::signal_handler, &svr, std::placeholders::_1, std::placeholders::_2));

		// io.run();
		svr.run();
		***/

		/**coroutine**/
		asio::io_context io;

		asio::spawn(io, [&](asio::yield_context yield) {
			std::cout << std::this_thread::get_id() << " acceptor spawn begin" << std::endl;
			tcp::acceptor accepter(io, tcp::endpoint(tcp::v4(), std::atoi(argv[2])));
			for(;;){
				std::cout << std::this_thread::get_id() << " acceptor spawn loop" << std::endl;
				asio::error_code ec;
				tcp::socket ss(io);
				accepter.async_accept(ss, yield[ec]); // 1
				if (!ec){
					std::cout << std::this_thread::get_id() << " async_accept  " << std::endl;
					std::make_shared<spawn_session>(io, std::move(ss))->go(); // 2
				}
			}
		});

		io.run();

	}catch(std::exception& e){
        std::cerr << "Exception : " << e.what() << std::endl;
        return 1;
    }
	

	return 0;
}