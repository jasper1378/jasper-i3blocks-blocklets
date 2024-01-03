#include "module_api.hpp"

#include "i3bar_protocol.hpp"
#include "thread_comm.hpp"

#include "libconfigfile.hpp"

#include <exception>
#include <memory>
#include <string>
#include <utility>

const std::string module_api::config_out::k_valid_name_chars{
    "abcdefghijklmnopqrstuvqxyzABCDEFGHIJKLMNOPQRSTUVQXYZ_-"};

module_api::module_api() : m_thread_comm_producer{} {}

module_api::module_api(const thread_comm::producer<block> &thread_comm_producer)
    : m_thread_comm_producer{thread_comm_producer} {}

module_api::module_api(thread_comm::producer<block> &&thread_comm_producer)
    : m_thread_comm_producer{std::move(thread_comm_producer)} {}

module_api::module_api(module_api &&other) noexcept
    : m_thread_comm_producer{std::move(other.m_thread_comm_producer)} {}

module_api::~module_api() {}

module_api &module_api::operator=(module_api &&other) noexcept {
  if (this != &other) {
    m_thread_comm_producer = std::move(other.m_thread_comm_producer);
  }
  return *this;
}

void module_api::put_block(const block &block) {
  m_thread_comm_producer.put_value(block);
}

void module_api::put_block(block &&block) {
  m_thread_comm_producer.put_value(std::move(block));
}

void module_api::put_error(const std::exception_ptr &error) {
  m_thread_comm_producer.put_exception(error);
}

void module_api::put_error(std::exception_ptr &&error) {
  m_thread_comm_producer.put_exception(std::move(error));
}
