#include "i3bar_protocol.hpp"

#include "hide_block.hpp"
#include "i3bar_data.hpp"
#include "i3bar_data_conversions.hpp"
#include "misc.hpp"

#include "libconfigfile/color.hpp"

#include <charconv>
#include <iostream>
#include <limits>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

void i3neostatus::i3bar_protocol::print_header(
    const i3bar_data::header &value, std::ostream &stream /*= std::cout */) {
  stream << impl::serialize_header(value) << json_constants::k_newline
         << std::flush;
}

void i3neostatus::i3bar_protocol::init_statusline(
    std::ostream &stream /*= std::cout*/) {
  stream << json_constants::k_array_opening_delimiter
         << json_constants::k_newline;
  stream << json_constants::k_array_opening_delimiter
         << json_constants::k_array_closing_delimiter
         << json_constants::k_newline;
  stream << std::flush;
}

void i3neostatus::i3bar_protocol::print_statusline(
    const std::vector<i3bar_data::block> &value, bool hide_empty /*= true*/,
    std::ostream &stream /*= std::cout*/) {
  impl::print_statusline(
      [&value, &hide_empty]() -> std::vector<std::string> {
        std::vector<std::string> ret_val;
        for (std::size_t i{0}; i < ret_val.size(); ++i) {
          ret_val.emplace_back(
              ((hide_block::get(value[i]) && hide_empty)
                   ? (hide_block::set<std::string>())
                   : (impl::serialize_block(value[i]))));
        }
        return ret_val;
      }(),
      stream);
}

void i3neostatus::i3bar_protocol::print_statusline(
    const std::pair<i3bar_data::block, std::size_t> &value,
    std::vector<std::string> &cache, bool hide_empty /* = true*/,
    std::ostream &stream /*= std::cout*/) {
  cache[value.second] =
      ((hide_block::get(value.first) && hide_empty)
           ? (hide_block::set<std::string>())
           : (impl::serialize_block(value.first)));
  impl::print_statusline(cache, stream);
}

void i3neostatus::i3bar_protocol::print_statusline(
    const std::vector<std::pair<i3bar_data::block, std::size_t>> &value,
    std::vector<std::string> &cache, bool hide_empty /*= true*/,
    std::ostream &stream /*= std::cout*/) {
  for (std::size_t i{0}; i < value.size(); ++i) {
    cache[value[i].second] =
        ((hide_block::get(value[i].first) && hide_empty)
             ? (hide_block::set<std::string>())
             : (impl::serialize_block(value[i].first)));
  }
  impl::print_statusline(cache, stream);
}

void i3neostatus::i3bar_protocol::init_click_event(
    std::istream &input_stream /*=std::cin*/) {
  input_stream.ignore(std::numeric_limits<std::streamsize>::max(),
                      json_constants::k_array_opening_delimiter);
}

i3neostatus::i3bar_data::click_event
i3neostatus::i3bar_protocol::read_click_event(
    std::istream &input_stream /*= std::cin*/) {
  std::string input_str{};
  std::getline(input_stream, input_str, json_constants::k_newline);

  return impl::parse_click_event(input_str);
}

void i3neostatus::i3bar_protocol::impl::print_statusline(
    const std::vector<std::string> &value,
    std::ostream &stream /*= std::cout*/) {
  stream << json_constants::k_element_separator << serialize_array(value)
         << json_constants::k_newline << std::flush;
}

std::string i3neostatus::i3bar_protocol::impl::serialize_header(
    const i3bar_data::header &header) {
  return serialize_object(
      [&header]() -> std::vector<std::pair<std::string, std::string>> {
        std::vector<std::pair<std::string, std::string>> ret_val;

        ret_val.emplace_back(json_strings::header::k_version,
                             serialize_number(header.version));

          ret_val.emplace_back(json_strings::header::k_stop_signal,
                               serialize_number(header.stop_signal));

          ret_val.emplace_back(json_strings::header::k_cont_signal,
                               serialize_number(header.cont_signal));

          ret_val.emplace_back(json_strings::header::k_click_events,
                               serialize_bool(header.click_events));

        return ret_val;
      }());
}

std::string i3neostatus::i3bar_protocol::impl::serialize_block(
    const i3bar_data::block &block) {
  return serialize_object(
      [&block]() -> std::vector<std::pair<std::string, std::string>> {
        std::vector<std::pair<std::string, std::string>> ret_val;

          ret_val.emplace_back(json_strings::block::k_name,
                               serialize_string(block.id.name));

          ret_val.emplace_back(
              json_strings::block::k_instance,
              serialize_string(std::to_string(block.id.instance)));

        ret_val.emplace_back(json_strings::block::k_full_text,
                             serialize_string(block.data.module.full_text));

        if (block.data.module.short_text.has_value()) {
          ret_val.emplace_back(
              json_strings::block::k_short_text,
              serialize_string(*block.data.module.short_text));
        }

          ret_val.emplace_back(json_strings::block::k_color,
                               serialize_string(libconfigfile::color::to_string(
                                   block.data.program.theme.color)));

          ret_val.emplace_back(json_strings::block::k_background,
                               serialize_string(libconfigfile::color::to_string(
                                   block.data.program.theme.background)));

          ret_val.emplace_back(json_strings::block::k_border,
                               serialize_string(libconfigfile::color::to_string(
                                   block.data.program.theme.border)));

          ret_val.emplace_back(
              json_strings::block::k_border_top,
              serialize_number(block.data.program.theme.border_top));

          ret_val.emplace_back(
              json_strings::block::k_border_right,
              serialize_number(block.data.program.theme.border_right));

          ret_val.emplace_back(
              json_strings::block::k_border_bottom,
              serialize_number(block.data.program.theme.border_bottom));

          ret_val.emplace_back(
              json_strings::block::k_border_left,
              serialize_number(block.data.program.theme.border_left));

        if (block.data.module.min_width.has_value()) {
          ret_val.emplace_back(json_strings::block::k_min_width,
                               ((block.data.module.min_width->index() == 0)
                                    ? (serialize_number(std::get<0>(
                                          *block.data.module.min_width)))
                                    : (serialize_string(std::get<1>(
                                          *block.data.module.min_width)))));
        }

        if (block.data.module.align.has_value()) {
          ret_val.emplace_back(json_strings::block::k_align,
                               serialize_string(i3bar_data::types::to_string(
                                   *block.data.module.align)));
        }

        if (block.data.module.urgent.has_value()) {
          ret_val.emplace_back(json_strings::block::k_urgent,
                               serialize_bool(*block.data.module.urgent));
        }

          ret_val.emplace_back(json_strings::block::k_separator,
                               serialize_bool(block.data.program.global.separator));

          ret_val.emplace_back(
              json_strings::block::k_separator_block_width,
              serialize_number(block.data.program.global.separator_block_width));

        if (block.data.module.markup.has_value()) {
          ret_val.emplace_back(json_strings::block::k_markup,
                               serialize_string(i3bar_data::types::to_string(
                                   *block.data.module.markup)));
        }

        return ret_val;
      }());
}

std::string i3neostatus::i3bar_protocol::impl::serialize_name_value(
    const std::pair<std::string, std::string> &name_value) {

  return std::string{json_constants::k_string_delimiter + name_value.first +
                     json_constants::k_string_delimiter +
                     json_constants::k_name_value_separator +
                     name_value.second};
}

std::string i3neostatus::i3bar_protocol::impl::serialize_object(
    const std::vector<std::pair<std::string, std::string>> &object) {
  std::string ret_val;

  ret_val += json_constants::k_object_opening_delimiter;

  for (std::size_t i{0}; i < object.size(); ++i) {
    if (i != 0) {
      ret_val += json_constants::k_element_separator;
    }
    ret_val += serialize_name_value(object[i]);
  }

  ret_val += json_constants::k_object_closing_delimiter;

  return ret_val;
}

std::string i3neostatus::i3bar_protocol::impl::serialize_array(
    const std::vector<std::string> &array) {
  std::string ret_val;

  ret_val += json_constants::k_array_opening_delimiter;

  for (std::size_t i{0}; i < array.size(); ++i) {
    if (!hide_block::get(array[i])) {
      if ((i != 0) && (!hide_block::get(array[i - 1]))) {
        ret_val += json_constants::k_element_separator;
      }
      ret_val += array[i];
    }
  }

  ret_val += json_constants::k_array_closing_delimiter;

  return ret_val;
}

std::string i3neostatus::i3bar_protocol::impl::serialize_string(
    const std::string_view string) {
  static const std::string k_control_chars{
      0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A,
      0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15,
      0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F};

  static const std::unordered_map<char, std::string> k_control_char_codes{
      {0x00, "u0000"}, {0x01, "u0001"}, {0x02, "u0002"}, {0x03, "u0003"},
      {0x04, "u0004"}, {0x05, "u0005"}, {0x06, "u0006"}, {0x07, "u0007"},
      {0x08, "u0008"}, {0x09, "u0009"}, {0x0A, "u000A"}, {0x0B, "u000B"},
      {0x0C, "u000C"}, {0x0D, "u000D"}, {0x0E, "u000E"}, {0x0F, "u000F"},
      {0x10, "u0010"}, {0x11, "u0011"}, {0x12, "u0012"}, {0x13, "u0013"},
      {0x14, "u0014"}, {0x15, "u0015"}, {0x16, "u0016"}, {0x17, "u0017"},
      {0x18, "u0018"}, {0x19, "u0019"}, {0x1A, "u001A"}, {0x1B, "u001B"},
      {0x1C, "u001C"}, {0x1D, "u001D"}, {0x1E, "u001E"}, {0x1F, "u001F"}};

  static const std::string k_need_to_replace{
      k_control_chars + json_constants::k_string_delimiter +
      json_constants::k_escape_leader};

  std::string ret_val;
  ret_val.reserve(string.size() + 2);

  ret_val += json_constants::k_string_delimiter;

  std::string::size_type pos{0};
  std::string::size_type pos_prev{0};
  while (true) {
    pos = string.find_first_of(k_need_to_replace, pos_prev);
    if (pos == std::string::npos) {
      break;
    } else {
      ret_val += string.substr(pos_prev, (pos - pos_prev));
      ret_val += json_constants::k_escape_leader;

      switch (string[pos]) {
      case json_constants::k_string_delimiter: {
        ret_val += json_constants::k_string_delimiter;
      } break;
      case json_constants::k_escape_leader: {
        ret_val += json_constants::k_escape_leader;
      } break;
      default: {
        ret_val += k_control_char_codes.at(string[pos]);
      } break;
      }

      pos_prev = pos + 1;
    }
  }
  ret_val += string.substr(pos_prev);

  ret_val += json_constants::k_string_delimiter;

  return ret_val;
}

std::string i3neostatus::i3bar_protocol::impl::serialize_bool(const bool b) {
  if (b) {
    return "true";
  } else {
    return "false";
  }
}

i3neostatus::i3bar_data::click_event
i3neostatus::i3bar_protocol::impl::parse_click_event(
    const std::string_view click_event) {
  const auto substr{[](const std::string_view str,
                       const std::string::size_type begin,
                       const std::string::size_type end) -> std::string_view {
    return str.substr(begin, end - begin + 1);
  }};
  const auto find_first_digit{
      [](const std::string_view str,
         const std::string::size_type pos = 0) -> std::string::size_type {
        for (std::string::size_type i{pos}; i < str.size(); ++i) {
          if (str[i] >= '0' && str[i] <= '9') {
            return i;
          }
        }
        return std::string::npos;
      }};
  const auto find_first_not_digit{
      [](const std::string_view str,
         const std::string::size_type pos = 0) -> std::string::size_type {
        for (std::string::size_type i{pos}; i < str.size(); ++i) {
          if (!(str[i] >= '0' && str[i] <= '9')) {
            return i;
          }
        }
        return std::string::npos;
      }};
  const auto read_string_value{
      [&substr](const std::string_view str,
                const std::string::size_type name_end_pos,
                std::string::size_type &continue_from_pos) -> std::string {
        std::string::size_type value_begin_pos{
            str.find(json_constants::k_string_delimiter, name_end_pos + 2) + 1};
        std::string::size_type value_end_pos{
            str.find(json_constants::k_string_delimiter, value_begin_pos) - 1};
        continue_from_pos = value_end_pos + 2;
        return std::string{substr(str, value_begin_pos, value_end_pos)};
      }};
  const auto read_numeric_value{
      [&substr, &find_first_digit, &find_first_not_digit](
          const std::string_view str, const std::string::size_type name_end_pos,
          std::string::size_type &continue_from_pos) -> std::string_view {
        std::string::size_type value_begin_pos{
            find_first_digit(str, name_end_pos + 2)};
        std::string::size_type value_end_pos{
            find_first_not_digit(str, value_begin_pos)};
        continue_from_pos = value_end_pos + 1;
        return substr(str, value_begin_pos, value_end_pos);
      }};

  std::string name_buf;
  name_buf.resize(misc::constexpr_minmax::max(
      json_strings::click_event::k_name.size(),
      json_strings::click_event::k_instance.size(),
      json_strings::click_event::k_x.size(),
      json_strings::click_event::k_y.size(),
      json_strings::click_event::k_button.size(),
      json_strings::click_event::k_relative_x.size(),
      json_strings::click_event::k_relative_y.size(),
      json_strings::click_event::k_output_x.size(),
      json_strings::click_event::k_output_y.size(),
      json_strings::click_event::k_width.size(),
      json_strings::click_event::k_height.size(),
      json_strings::click_event::k_modifiers.size()));

  i3bar_data::click_event ret_val;

  std::string::size_type continue_from_pos{0};

  while (true) {
    std::string::size_type name_begin_pos{click_event.find(
        json_constants::k_string_delimiter, continue_from_pos)};
    if (name_begin_pos == std::string::npos) {
      break;
    } else {
      ++name_begin_pos;
      std::string::size_type name_end_pos =
          (click_event.find(json_constants::k_string_delimiter,
                            name_begin_pos) -
           1);
      name_buf = substr(click_event, name_begin_pos, name_end_pos);

      switch (misc::constexpr_hash_string::hash(name_buf)) {
      case misc::constexpr_hash_string::hash(
          json_strings::click_event::k_name): {
        ret_val.id.name =
            read_string_value(click_event, name_end_pos, continue_from_pos);
      } break;
      case misc::constexpr_hash_string::hash(
          json_strings::click_event::k_instance): {
        ret_val.id.instance = module_id::from_string(
            read_string_value(click_event, name_end_pos, continue_from_pos));
      } break;
      case misc::constexpr_hash_string::hash(json_strings::click_event::k_x): {
        std::string_view value{
            read_numeric_value(click_event, name_end_pos, continue_from_pos)};
        std::from_chars(value.data(), value.data() + value.size(),
                        ret_val.data.x);
      } break;
      case misc::constexpr_hash_string::hash(json_strings::click_event::k_y): {
        std::string_view value{
            read_numeric_value(click_event, name_end_pos, continue_from_pos)};
        std::from_chars(value.data(), value.data() + value.size(),
                        ret_val.data.y);
      } break;
      case misc::constexpr_hash_string::hash(
          json_strings::click_event::k_button): {
        std::string_view value{
            read_numeric_value(click_event, name_end_pos, continue_from_pos)};
        std::from_chars(value.data(), value.data() + value.size(),
                        ret_val.data.button);
      } break;
      case misc::constexpr_hash_string::hash(
          json_strings::click_event::k_relative_x): {
        std::string_view value{
            read_numeric_value(click_event, name_end_pos, continue_from_pos)};
        std::from_chars(value.data(), value.data() + value.size(),
                        ret_val.data.relative_x);
      } break;
      case misc::constexpr_hash_string::hash(
          json_strings::click_event::k_relative_y): {
        std::string_view value{
            read_numeric_value(click_event, name_end_pos, continue_from_pos)};
        std::from_chars(value.data(), value.data() + value.size(),
                        ret_val.data.relative_y);
      } break;
      case misc::constexpr_hash_string::hash(
          json_strings::click_event::k_output_x): {
        std::string_view value{
            read_numeric_value(click_event, name_end_pos, continue_from_pos)};
        std::from_chars(value.data(), value.data() + value.size(),
                        ret_val.data.output_x);
      } break;
      case misc::constexpr_hash_string::hash(
          json_strings::click_event::k_output_y): {
        std::string_view value{
            read_numeric_value(click_event, name_end_pos, continue_from_pos)};
        std::from_chars(value.data(), value.data() + value.size(),
                        ret_val.data.output_y);
      } break;
      case misc::constexpr_hash_string::hash(
          json_strings::click_event::k_width): {
        std::string_view value{
            read_numeric_value(click_event, name_end_pos, continue_from_pos)};
        std::from_chars(value.data(), value.data() + value.size(),
                        ret_val.data.width);
      } break;
      case misc::constexpr_hash_string::hash(
          json_strings::click_event::k_height): {
        std::string_view value{
            read_numeric_value(click_event, name_end_pos, continue_from_pos)};
        std::from_chars(value.data(), value.data() + value.size(),
                        ret_val.data.height);
      } break;
      case misc::constexpr_hash_string::hash(
          json_strings::click_event::k_modifiers): {
        std::string::size_type array_begin_pos{click_event.find(
            json_constants::k_array_opening_delimiter, name_end_pos + 2)};
        std::string::size_type array_end_pos{click_event.find(
            json_constants::k_array_closing_delimiter, array_begin_pos)};
        for (std::string::size_type i{array_begin_pos}; i < array_end_pos;) {
          std::string::size_type value_begin_pos{
              click_event.find(json_constants::k_string_delimiter, i) + 1};
          std::string::size_type value_end_pos{
              click_event.find(json_constants::k_string_delimiter,
                               value_begin_pos) -
              1};
          ret_val.data.modifiers |= i3bar_data::types::from_string<
              i3bar_data::types::click_modifiers>(
              std::string{substr(click_event, value_begin_pos, value_end_pos)});
          i = value_end_pos + 2;
        }
        continue_from_pos = array_end_pos + 1;
      } break;
      }
    }
  }

  return ret_val;
}
