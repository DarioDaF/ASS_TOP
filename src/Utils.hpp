#ifndef UTILS_HPP
#define UTILS_HPP

#include <iterator>
#include <vector>
#include <nlohmann/json.hpp>

typedef int idx_t;

template<typename T>
class NumberIterator {
  public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = T;
    using difference_type = T;
    using pointer = T*;
    using reference = T&;
    NumberIterator(T num) : num(num) {}

    const T& operator*() const { return num; }
    
    T operator-(const NumberIterator& other) const { return num - other.num; };
    NumberIterator& operator++() { ++num; return *this; }
    NumberIterator operator++(int) { auto old = num++; return NumberIterator(old); } // Postfix is marked by unused int argument

    bool operator==(const NumberIterator& other) const { return num == other.num; };
    bool operator!=(const NumberIterator& other) const { return num != other.num; };

  private:
    T num;
};

template<typename T>
class NumberRange {
  public:
    NumberRange(T first, T last) : first(first), last(last) {}
    NumberRange(T count) : first(0), last(count) {}
    NumberIterator<T> begin() { return NumberIterator(first); }
    NumberIterator<T> end() { return NumberIterator(last); }

    std::vector<T> Vector() { return std::vector<T>(begin(), end()); }
  private:
    T first, last;
};

enum strong_ordering {
  less = -1,
  equal = 0,
  greater = 1
};

template<class _It, class _Pred, class _Prov>
std::vector<typename std::iterator_traits<_It>::value_type> min_elements(_It begin, _It end, _Pred pred, _Prov prov) {
  using _Idx = typename std::iterator_traits<_It>::value_type;
  using _Val = decltype(prov(*begin));
  std::vector<_Idx> minimumIdxs;
  if(begin == end) {
    return minimumIdxs;
  }
  _Idx currentIdx = *begin++;
  minimumIdxs.push_back(currentIdx);
  _Val minimumValue = prov(currentIdx);
  for(; begin != end; ++begin) {
    currentIdx = *begin;
    _Val currentValue = prov(currentIdx);
    auto order = pred(currentValue, minimumValue); // strong_ordering
    if(order < 0) {
      // Less than
      minimumIdxs.clear();
      minimumValue = currentValue;
      minimumIdxs.push_back(currentIdx);
    } else if(order == 0) {
      minimumIdxs.push_back(currentIdx);
    }
  }
  return minimumIdxs;
}

template<class _It, class _Pred>
std::vector<typename std::iterator_traits<_It>::value_type> min_elements(_It begin, _It end, _Pred pred) {
  return min_elements(begin, end, pred, [](auto x) { return x; });
}

template<class _Pred, class _Prov>
std::vector<idx_t> min_elements(idx_t size, _Pred pred, _Prov prov) {
  return min_elements(NumberIterator(0), NumberIterator(size), pred, prov);
}

template<class _Pred>
std::vector<idx_t> min_elements(idx_t size, _Pred pred) {
  return min_elements(NumberIterator(0), NumberIterator(size), pred, [](auto x) { return x; });
}

template<typename T>
T so_cmp(const T& x, const T& y) {
  return x - y;
}
template<typename T>
T so_negcmp(const T& x, const T& y) {
  return y - x;
}

template<typename _MAP>
typename _MAP::mapped_type value_or_default(const _MAP map, const typename _MAP::key_type& key, const typename _MAP::mapped_type& def) {
  typename _MAP::const_iterator it = map.find(key);
  if(it == map.end()) {
    return def;
  }
  return it->second;
}

template<typename T>
T json_get_or_default(const nlohmann::json& j, T def) {
  return j.is_null() ? def : j.get<T>();
}

#endif
