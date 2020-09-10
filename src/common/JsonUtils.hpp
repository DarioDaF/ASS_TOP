#ifndef JSONUTILS_HPP
#define JSONUTILS_HPP

#include <nlohmann/json.hpp>

/**
 * Returns def if j is a null json object
 */
template<typename T>
T json_get_or_default(const nlohmann::json& j, T def) {
  return j.is_null() ? def : j.get<T>();
}

#endif
