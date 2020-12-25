#pragma once

#include <asio/io_context.hpp>
#include <asio/ip/tcp.hpp>
#include <asio/spawn.hpp>
#include <asio/steady_timer.hpp>
#include <asio/strand.hpp>
#include <chrono>
#include <exception>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include <asio.hpp>

using asio::ip::tcp;

class spawn_session : public std::enable_shared_from_this<spawn_session> {
  public:
	explicit spawn_session(asio::io_context &io_context, tcp::socket socket)
		: socket_(std::move(socket)), timer_(io_context), strand_(io_context.get_executor()) {}
	spawn_session() = delete;

	void go() {
		auto self(shared_from_this());
		asio::spawn(strand_, [this, self](asio::yield_context yield) {
			try {
				char data[1024]={0x00};
				for(;;){
					timer_.expires_from_now(std::chrono::microseconds(10));
					std::size_t n = socket_.async_read_some(asio::buffer(data), yield);

					asio::async_write(socket_, asio::buffer(data, n), yield);
				}

			}  catch  (std::exception &ec)  {
				socket_.close();
				timer_.cancel();
			}

		});

		asio::spawn(strand_, [this, self](asio::yield_context yield){
			while(socket_.is_open()){
				asio::error_code ignored_ec;

				timer_.async_wait(yield[ignored_ec]);

				if (timer_.expires_from_now() <= std::chrono::seconds(0)){
					socket_.close();
				}


			}

		});
	}

  private:
	tcp::socket socket_;
	asio::steady_timer timer_;
	asio::strand<asio::io_context::executor_type> strand_;
};