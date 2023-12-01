#include "module_handle.hpp"

#include "dyn_load_lib.hpp"
#include "module_api.hpp"
#include "module_base.hpp"
#include "module_error.hpp"
#include "module_id.hpp"
#include "thread_comm.hpp"

#include "libconfigfile.hpp"

#include <memory>
#include <string>
#include <thread>
#include <utility>

module_handle::module_handle(module_id_t id, std::string &&filename,
                             libconfigfile::map_node &&conf)
    : m_id{id}, m_name{}, m_filename{std::move(filename)},
      m_click_events_enabled{false},
      m_dyn_lib{m_filename, dyn_load_lib::dlopen_flags::LAZY},
      m_module{nullptr, nullptr}, m_thread_comm_producer{},
      m_thread_comm_consumer{}, m_thread{} {
  module_base::allocator_func_ptr_t mod_alloc{
      m_dyn_lib.get_symbol<module_base::allocator_func_t>(
          module_base::allocator_func_str)};
  module_base::deleter_func_ptr_t mod_delete{
      m_dyn_lib.get_symbol<module_base::deleter_func_t>(
          module_base::deleter_func_str)};
  m_module = {mod_alloc(), mod_delete};
  if (!m_module) {
    throw module_error::out{m_id, "UNKNOWN", m_filename, "allocator() failed"};
  }

  std::pair<thread_comm::producer<module_api::block>,
            thread_comm::consumer<module_api::block>>
      tc_pair{thread_comm::make_thread_comm_pair<module_api::block>()};
  m_thread_comm_producer = std::move(tc_pair.first);
  module_api mod_api{m_thread_comm_producer};
  m_thread_comm_consumer = std::move(tc_pair.second);

  try {
    module_api::config_out conf_out{
        m_module->init(std::move(mod_api), std::move(conf))};
    m_name = std::move(conf_out.m_name);
    m_click_events_enabled = conf_out.click_events_enabled;
  } catch (const std::exception &ex) {
    throw module_error::in{m_id, "UNKNOWN", m_filename, ex.what()};
  } catch (...) {
    throw module_error::in{m_id, "UNKNOWN", m_filename, "UNKNOWN"};
  }
}

module_handle::module_handle(module_handle &&other) noexcept
    : m_id{other.m_id}, m_name{std::move(other.m_name)},
      m_filename{std::move(other.m_filename)},
      m_click_events_enabled{other.m_click_events_enabled},
      m_dyn_lib{std::move(other.m_dyn_lib)},
      m_module{std::move(other.m_module)},
      m_thread_comm_producer{std::move(other.m_thread_comm_producer)},
      m_thread_comm_consumer{std::move(other.m_thread_comm_consumer)},
      m_thread{std::move(other.m_thread)} {}

module_handle::~module_handle() {
  m_module->term();
  m_thread.join();
}

module_handle &module_handle::operator=(module_handle &&other) noexcept {
  if (this != &other) {
    m_id = other.m_id;
    m_name = std::move(other.m_name);
    m_filename = std::move(other.m_filename);
    m_click_events_enabled = other.m_click_events_enabled;
    m_dyn_lib = std::move(other.m_dyn_lib);
    m_module = std::move(other.m_module);
    m_thread_comm_producer = std::move(other.m_thread_comm_producer);
    m_thread_comm_consumer = std::move(other.m_thread_comm_consumer);
    m_thread = std::move(other.m_thread);
  }
  return *this;
}

std::string module_handle::get_name() const { return m_name; }

std::string module_handle::get_filename() const { return m_filename; }

bool module_handle::get_click_events_enabled() const {
  return m_click_events_enabled;
}

void module_handle::run() {
  m_thread = std::thread{[this]() {
    try {
      m_module->run();
    } catch (const std::exception &ex) {
      m_thread_comm_producer.set_exception(std::make_exception_ptr(
          module_error::in{m_id, m_name, m_filename, ex.what()}));
    } catch (...) {
      m_thread_comm_producer.set_exception(std::make_exception_ptr(
          module_error::in{m_id, m_name, m_filename, "UNKNOWN"}));
    }
  }};
}

thread_comm::consumer<module_api::block> &module_handle::get_comm() {
  return m_thread_comm_consumer;
}
