#ifndef MODULE_API_HPP
#define MODULE_API_HPP

#include "i3bar_protocol.hpp"
#include "thread_comm.hpp"

#include "libconfigfile.hpp"

#include <exception>
#include <memory>
#include <string>
#include <utility>

class module_api {
public:
  using config_in = libconfigfile::map_node;

  struct config_out {
    std::string name;
    bool click_events;

    static const std::string k_valid_name_chars;
  };

  using block = struct i3bar_protocol::block::content;
  using click_event = struct i3bar_protocol::click_event::content;

private:
  thread_comm::producer<block> m_thread_comm_producer;

public:
  module_api();
  module_api(const thread_comm::producer<block> &thread_comm_producer);
  module_api(thread_comm::producer<block> &&thread_comm_producer);
  module_api(module_api &&other) noexcept;
  module_api(const module_api &other) = delete;

  ~module_api();

  module_api &operator=(module_api &&other) noexcept;
  module_api &operator=(const module_api &other) = delete;

public:
  template <typename... t_args>
  std::unique_ptr<block> make_block(t_args &&...args) {
    return std::make_unique<block>(
        std::forward<t_args>(args)...); // TODO reuse buffer
  }

  void put_block(std::unique_ptr<block> block);

  void put_error(std::exception_ptr error);
};

#endif
