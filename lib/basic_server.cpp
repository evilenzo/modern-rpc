#include "basic_server.hpp"

#include "fmt/core.h"

#include <boost/asio/awaitable.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/experimental/as_tuple.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/read_until.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/asio/this_coro.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/asio/write.hpp>

#include <ranges>
#include <string_view>
#include <thread>


namespace net {
using namespace boost::asio;
using boost::asio::experimental::as_tuple;
}

namespace ip = net::ip;
using tcp = ip::tcp;

namespace modern::rpc {
template <Handling Dispatcher>
struct basic_server<Dispatcher>::impl {
  static constexpr auto DELIMETER{"\r\n"};

  impl() = default;

  impl(std::string_view address, ushort port, Dispatcher& dispatcher)
      : m_address{ip::make_address(address)},
        m_port{port},
        m_dispatcher{std::addressof(dispatcher)} {}

  void run(int concurrency_hint) {
    concurrency_hint = std::max(1, concurrency_hint);

    m_ioc.emplace(static_cast<int>(concurrency_hint));
    net::co_spawn(m_ioc->get_executor(), poll_connections(), net::detached);

    m_contexts.reserve(concurrency_hint);
    for (int i : std::views::iota(0, concurrency_hint)) {
      m_contexts.emplace_back([&]() { m_ioc->run(); });
    }
  }

  void wait() {
    for (auto& context : m_contexts) {
      context.join();
    }
  }

  void wait_pending() {
    // TODO
  }

  template <typename Rep, typename Period>
  void wait_for(std::chrono::duration<Rep, Period> duration) {
    // TODO
  }

  void stop() {
    // TODO
  }

 private:
  net::awaitable<void> poll_connections() {
    auto executor = co_await net::this_coro::executor;

    tcp::endpoint endpoint{m_address, m_port};

    tcp::acceptor acceptor{executor};
    acceptor.open(endpoint.protocol());
    acceptor.bind(endpoint);
    acceptor.listen();

    for (;;) {
      tcp::socket socket = co_await acceptor.async_accept(net::use_awaitable);
      net::co_spawn(executor, poll_socket(std::move(socket)), net::detached);
    }
  }

  net::awaitable<void> poll_socket(tcp::socket socket) {
    net::streambuf stream;

    for (;;) {
      auto [read_ec, bytes_read] = co_await net::async_read_until(
          socket, stream, DELIMETER, net::as_tuple(net::use_awaitable));

      if (read_ec || !bytes_read) {
        // TODO: handle error
        break;
      }

      char in_buffer[bytes_read] = stream.data();
      auto [out_buffer, size] = m_dispatcher->handle(in_buffer, bytes_read);

      auto [write_ec, bytes_written] =
          co_await net::async_write(socket, net::buffer(out_buffer, size),
                                    net::as_tuple(net::use_awaitable));

      if (write_ec || !bytes_written) {
        // TODO: handle error
        break;
      }
    }

    socket.close();
  }

 private:
  ip::address m_address;
  ushort m_port{};

  std::optional<Dispatcher> m_dispatcher;

  std::optional<net::io_context> m_ioc;
  std::vector<std::thread> m_contexts;
};
}  // namespace modern::rpc