#include "rpc/server.h"

#include <atomic>
#include <memory>
#include <stdexcept>
#include <stdint.h>
#include <thread>

#include "asio.hpp"
#include "format.h"

#include "rpc/detail/dev_utils.h"
#include "rpc/detail/log.h"
#include "rpc/detail/log.h"
#include "rpc/detail/server_session.h"
#include "rpc/detail/thread_group.h"

using namespace rpc::detail;
using RPCLIB_ASIO::ip::tcp;
using namespace RPCLIB_ASIO;

namespace rpc {

struct server::impl {
    impl(server *parent, std::string const &address, uint16_t port)
        : parent_(parent),
          io_(),
          acceptor_(io_,
                    tcp::endpoint(ip::address::from_string(address), port)),
          socket_(io_),
          suppress_exceptions_(false) {}

    impl(server *parent, uint16_t port)
        : parent_(parent),
          io_(),
          acceptor_(io_, tcp::endpoint(tcp::v4(), port)),
          socket_(io_),
          suppress_exceptions_(false) {}

	impl(server* parent)
		: parent_(parent),
		io_(),
		acceptor_(io_),
		socket_(io_),
		suppress_exceptions_(false)
	{
	}

	void accept(std::string const &address, uint16_t port)
	{
		//boost::asio::ip::tcp::resolver resolver(io_service_);
		//boost::asio::ip::tcp::endpoint endpoint = *resolver.resolve({address, port});
		tcp::endpoint endpoint(ip::address::from_string(address), port);
		acceptor_.open(endpoint.protocol());
		//acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
		acceptor_.bind(endpoint);
		acceptor_.listen();
	}

	void accept(uint16_t port)
	{
		tcp::endpoint endpoint(tcp::v4(), port);
		acceptor_.open(endpoint.protocol());
		acceptor_.bind(endpoint);
		acceptor_.listen();
	}

    void start_accept() {
        acceptor_.async_accept(socket_, [this](std::error_code ec) {
            if (!ec) {
                RPCLOG_INFO("Accepted connection.");
                auto s = std::make_shared<server_session>(
                    parent_, &io_, std::move(socket_),
                    suppress_exceptions_);
                s->start();
                sessions_.push_back(s);
            } else {
                RPCLOG_ERROR("Error while accepting connection: {}", ec);
            }
            start_accept();
            // TODO: allow graceful exit [sztomi 2016-01-13]
        });
    }

    void close_sessions() {
        for (auto &session : sessions_) {
            session->close();
        }
        sessions_.clear();
    }

    void stop() {
        io_.stop();
        loop_workers_.join_all();
    }

    server *parent_;
    io_service io_;
    ip::tcp::acceptor acceptor_;
    ip::tcp::socket socket_;
    rpc::detail::thread_group loop_workers_;
    std::vector<std::shared_ptr<server_session>> sessions_;
    std::atomic_bool suppress_exceptions_;
    RPCLIB_CREATE_LOG_CHANNEL(server)
};

RPCLIB_CREATE_LOG_CHANNEL(server)

server::server(uint16_t port)
    : pimpl(new server::impl(this, port)), disp_(std::make_shared<default_dispatcher>()) {
    RPCLOG_INFO("Created server on localhost:{}", port);
    pimpl->start_accept();
}

server::server(server&& other) noexcept {
    *this = std::move(other);
}

server::server(std::string const &address, uint16_t port)
    : pimpl(new server::impl(this, address, port)),
    disp_(std::make_shared<default_dispatcher>()) {
    RPCLOG_INFO("Created server on address {}:{}", address, port);
    pimpl->start_accept();
}

server::server()
	: pimpl(new server::impl(this)),
    disp_(std::make_shared<default_dispatcher>()) 
{
    RPCLOG_INFO("Created server without accept.");
}

server::server(default_dispatcher* disp)
	: pimpl(new server::impl(this)),
    disp_(disp) 
{
    RPCLOG_INFO("Created server without accept.");
}

void server::accept(std::string const &address, uint16_t port)
{
    pimpl->accept(address, port);
    RPCLOG_INFO("Created server on address {}:{}", address, port);
    pimpl->start_accept();
}

void server::accept(uint16_t port)
{
    pimpl->accept(port);
    RPCLOG_INFO("Created server on localhost:{}", port);
    pimpl->start_accept();
}

server::~server() {
    if (pimpl) {
        pimpl->stop();
    }
}

server& server::operator=(server &&other) {
    pimpl = std::move(other.pimpl);
    other.pimpl = nullptr;
    disp_ = std::move(other.disp_);
    other.disp_ = nullptr;
    return *this;
}

void server::suppress_exceptions(bool suppress) {
    pimpl->suppress_exceptions_ = suppress;
}

void server::run() { pimpl->io_.run(); }
void server::run_one() { pimpl->io_.run_one(); }
void server::poll() { pimpl->io_.poll(); }
void server::poll_one() { pimpl->io_.poll_one(); }

void server::async_run(std::size_t worker_threads) {
    pimpl->loop_workers_.create_threads(worker_threads, [this]() {
        name_thread("server");
        RPCLOG_INFO("Starting");
        pimpl->io_.run();
        RPCLOG_INFO("Exiting");
    });
}

void server::stop() { pimpl->stop(); }

void server::close_sessions() { pimpl->close_sessions(); }

void server::close_session(std::shared_ptr<detail::server_session> const &s) {
  auto it = std::find(begin(pimpl->sessions_), end(pimpl->sessions_), s);
  if (it != end(pimpl->sessions_)) {
    pimpl->sessions_.erase(it);
  }
}

} /* rpc */
