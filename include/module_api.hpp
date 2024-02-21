#ifndef I3NEOSTATUS_MODULE_API_HPP
#define I3NEOSTATUS_MODULE_API_HPP

#include "i3bar_data.hpp"

#include "libconfigfile/libconfigfile.hpp"

#include <exception>
#include <string>

namespace i3neostatus {

namespace thread_comm {
template <typename t_value> class producer;
}

class module_api {
public:
  using config_in = libconfigfile::map_node;

  struct config_out {
    std::string name;
    bool click_events_enabled;

    static const std::string k_valid_name_chars;
  };

  using block = struct i3bar_data::block::content::local;
  using click_event = struct i3bar_data::click_event::content;

private:
  thread_comm::producer<block> *m_thread_comm_producer;

public:
  module_api(thread_comm::producer<block> *thread_comm_producer);
  module_api(module_api &&other) noexcept;
  module_api(const module_api &other) = delete;

  ~module_api();

  module_api &operator=(module_api &&other) noexcept;
  module_api &operator=(const module_api &other) = delete;

public:
  void put_block(const block &block);
  void put_block(block &&block);

  void put_error(const std::exception_ptr &error);
  void put_error(std::exception_ptr &&error);
  void put_error(const std::exception &error);
  void put_error(std::exception &&error);
};

} // namespace i3neostatus
#endif
