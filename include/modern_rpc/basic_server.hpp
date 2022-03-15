#pragma once

#include "details/pimpl.hpp"

#include <concepts>
#include <thread>


namespace modern::rpc {
// clang-format off
template <typename Dispatcher>
concept Handling = requires(Dispatcher dispatcher, const char* buffer,
                            size_t size) {
  {
    dispatcher.handle(buffer, size)
  } -> std::same_as<std::pair<const char*, size_t>>;
};
// clang-format on


template <Handling Dispatcher>
class basic_server {
 public:
  basic_server() = default;

  basic_server(std::string_view address, ushort port, Dispatcher& dispatcher)
      : m_pimpl(std::make_unique<impl>(address, port, dispatcher)) {}

  void run(uint32_t concurrency_hint = std::thread::hardware_concurrency()) {
    m_pimpl->run(static_cast<int>(concurrency_hint));
  }

  void wait() { m_pimpl->wait(); }

  void wait_pending() {}

  template <typename Rep, typename Period>
  void wait_for(std::chrono::duration<Rep, Period> duration) {
    m_pimpl->wait(duration);
  }

  void stop() { m_pimpl->stop(); }

 private:
  DECLARE_PIMPL()
};
}  // namespace modern::rpc
