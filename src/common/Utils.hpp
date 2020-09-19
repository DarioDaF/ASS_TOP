#ifndef UTILS_HPP
#define UTILS_HPP

#include <iterator>
#include <vector>
#include <streambuf>
#include <mutex>
#include <shared_mutex>
#include <sstream>
#include <iostream>

typedef int idx_t;

/**
 * Iterator for numeric type T to define ranges without backing arrays
 */
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

/**
 * This class is used to define an enumerable for example in for each iteration
 * 
 * Typical usage:
 *   for(auto x : NumberRange<int>(10)) {
 *     printf("%d\n", x);
 *   }
 */
template<typename T>
class NumberRange {
  public:
    NumberRange(T first, T last) : first(first), last(last) {}
    NumberRange(T count) : first(0), last(count) {}
    NumberIterator<T> begin() { return NumberIterator(first); }
    NumberIterator<T> end() { return NumberIterator(last); }

    /**
     * Create an std::vector containing the range
     */
    std::vector<T> Vector() { return std::vector<T>(begin(), end()); }
  private:
    T first, last;
};

/**
 * This class is used to map an 
 */
template <typename char_type,
          typename traits = std::char_traits<char_type> >
class basic_teebuf:
    public std::basic_streambuf<char_type, traits>
{
  public:
    typedef typename traits::int_type int_type;
    
    basic_teebuf(std::basic_streambuf<char_type, traits> * sb1,
                 std::basic_streambuf<char_type, traits> * sb2)
      : sb1(sb1)
      , sb2(sb2)
    {
    }
    
  private:    
    virtual int sync()
    {
        int const r1 = sb1->pubsync();
        int const r2 = sb2->pubsync();
        return r1 == 0 && r2 == 0 ? 0 : -1;
    }
    
    virtual int_type overflow(int_type c)
    {
        int_type const eof = traits::eof();
        
        if (traits::eq_int_type(c, eof))
        {
            return traits::not_eof(c);
        }
        else
        {
            char_type const ch = traits::to_char_type(c);
            int_type const r1 = sb1->sputc(ch);
            int_type const r2 = sb2->sputc(ch);
            
            return
                traits::eq_int_type(r1, eof) ||
                traits::eq_int_type(r2, eof) ? eof : c;
        }
    }
    
  private:
    std::basic_streambuf<char_type, traits> * sb1;
    std::basic_streambuf<char_type, traits> * sb2;
};

typedef basic_teebuf<char> teebuf;

/**
 * This class is used to create a compound output stream sending data to 2 backend streams (like console and log)
 * Implementation is public domain and can be found at http://wordaligned.org/articles/cpp-streambufs
 */
class teestream : public std::ostream {
  public:
    // Construct an ostream which tees output to the supplied
    // ostreams.
    teestream(std::ostream & o1, std::ostream & o2) : std::ostream(&tbuf), tbuf(o1.rdbuf(), o2.rdbuf()) {};
  private:
    teebuf tbuf;
};

/**
 * Class to create a set of lockflush::streams 
 * to protect an output stream, DO NOT USE results
 * after this class destruction
 */
class lockflush {
  protected:
    std::ostream& os;
    std::mutex _mutex;
  public:
    /**
     * Class to write data in a locked format
     * to send new data use flush()
     */
    class stream : public std::ostringstream {
      protected:
        std::ostream* os;
        std::mutex* _mutex;
        bool propagateFlush;
      public:
        stream(const stream&) = delete; // non construction-copyable
        stream& operator=(const stream&) = delete; // non copyable

        stream(stream&& other) {
          os = std::exchange(other.os, nullptr);
          _mutex = std::exchange(other._mutex, nullptr);
        };
        stream& operator=(stream&& other) {
          os = std::exchange(other.os, nullptr);
          _mutex = std::exchange(other._mutex, nullptr);
          return *this;
        }

        stream(std::ostream& os, std::mutex& mutex, bool propagateFlush = true) : os(&os), _mutex(&mutex), propagateFlush(propagateFlush) {}

        void flush() {
          std::unique_lock<std::mutex> _lock(*_mutex);
          *os << str();
          if(propagateFlush)
            os->flush();
          clear(); // Clear state
          str(""); // And content
        }
    };

    lockflush(std::ostream& os) : os(os) {}
    stream get() {
      return stream { os, _mutex };
    }
};

class nullbuff : public std::streambuf {
  public:
    int overflow(int c) { return c; }
};

class nullstream : public std::ostream {
  protected:
    nullbuff _null;
  public:
    nullstream() : std::ostream(&_null), _null() {};
};

/**
 * STL Map that is thread safe in insertion and find
 * access to underlining _map is not protected
 */
template<typename K, typename V>
class safe_map {
  public:
    safe_map() : _map(), _mutex() {}
    bool find(const K& key, V& value) {
      // _map.end() is not safe!
      std::shared_lock _lock(_mutex);
      auto valIter = _map.find(key);
      if(valIter == _map.end()) {
        return false;
      }
      value = valIter->second;
      return true;
    }
    bool insert(const K& key, const V& value) {
      std::unique_lock _lock(_mutex);
      return _map.insert(pair(key, value)).second;
    }
    std::map<K, V> _map; // Can access unsafe map when multithread finished
  private:
    std::shared_mutex _mutex;
};

enum strong_ordering {
  less = -1,
  equal = 0,
  greater = 1
};

/**
 * This function returns a vector containing the minimum elements (not only the first)
 * 
 * @param begin Begin iterator
 * @param end End iterator
 * @param pred Function-like entity that defines order returning strong_ordering (in seneral < 0, = 0, > 0)
 * @param prov Function-like that prerpocesses the elements (mapped onto iterator)
 */
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

/**
 * This function returns a vector containing the minimum elements (not only the first)
 * 
 * @param begin Begin iterator
 * @param end End iterator
 * @param pred Function-like entity that defines order returning strong_ordering (in seneral < 0, = 0, > 0)
 */
template<class _It, class _Pred>
std::vector<typename std::iterator_traits<_It>::value_type> min_elements(_It begin, _It end, _Pred pred) {
  return min_elements(begin, end, pred, [](auto x) { return x; });
}

/**
 * This function returns a vector containing the minimum elements (not only the first)
 * 
 * @param size Iterate trough index list with this size
 * @param pred Function-like entity that defines order returning strong_ordering (in seneral < 0, = 0, > 0)
 * @param prov Function-like that prerpocesses the elements (mapped onto iterator)
 */
template<class _Pred, class _Prov>
std::vector<idx_t> min_elements(idx_t size, _Pred pred, _Prov prov) {
  return min_elements(NumberIterator(0), NumberIterator(size), pred, prov);
}

/**
 * This function returns a vector containing the minimum elements (not only the first)
 * 
 * @param size Iterate trough index list with this size
 * @param pred Function-like entity that defines order returning strong_ordering (in seneral < 0, = 0, > 0)
 */
template<class _Pred>
std::vector<idx_t> min_elements(idx_t size, _Pred pred) {
  return min_elements(NumberIterator(0), NumberIterator(size), pred, [](auto x) { return x; });
}

/**
 * Default comparison predicate
 */
template<typename T>
T so_cmp(const T& x, const T& y) {
  return x - y;
}
/**
 * Inverted comparison predicate
 */
template<typename T>
T so_negcmp(const T& x, const T& y) {
  return y - x;
}

/**
 * Returns map[key] if present or def otherwise
 */
template<typename _MAP>
typename _MAP::mapped_type value_or_default(const _MAP map, const typename _MAP::key_type& key, const typename _MAP::mapped_type& def) {
  typename _MAP::const_iterator it = map.find(key);
  if(it == map.end()) {
    return def;
  }
  return it->second;
}

#endif
