#pragma once

#include <asio/io_context.hpp>
#include <asio/ip/tcp.hpp>
#include <asio/spawn.hpp>
#include <asio/steady_timer.hpp>
#include <asio/strand.hpp>
#include <chrono>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

using asio::ip::tcp;

class session : public std::enable_shared_from_this<session> {
  public:
	explicit session(asio::io_context &io_context, tcp::socket socket)
		: socket_(std::move(socket)), timer_(io_context), strand_(io_context.get_executor()) {}
	session() = delete;

	void go() {
		auto self(shared_from_this());
		asio::spawn(strand_, [this, self](asio::yield_context yield) {

		});
	}

  private:
	tcp::socket socket_;
	asio::steady_timer timer_;
	asio::strand<asio::io_context::executor_type> strand_;
};