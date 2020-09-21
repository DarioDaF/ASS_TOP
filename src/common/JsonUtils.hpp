#ifndef JSONUTILS_HPP
#define JSONUTILS_HPP

#include <nlohmann/json.hpp>

/**
 * Returns def if j is a null json object
 */
template<typename T>
inline T json_get_or_default(const nlohmann::json& j, T&& def) {
  return j.is_null() ? def : j.get<T>();
}

/**
 * Returns def if j[key] is missing
 */
template<typename T, typename K>
inline T json_get_or_default(const nlohmann::json& j, const K& key, T&& def) {
  if(j.contains(key)) {
    const nlohmann::json& v = j[key];
    return v.get<T>();
  }
  return def;
  //return j.contains(key) ? dynamic_cast<const nlohmann::json&>(j[key]).get<T>() : def;
}

#endif
