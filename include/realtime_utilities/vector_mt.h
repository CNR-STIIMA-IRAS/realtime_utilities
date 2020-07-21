#ifndef REALTIME_UTILITIES_VECTOR_MT_H
#define REALTIME_UTILITIES_VECTOR_MT_H

#include <mutex>
#include <boost/thread.hpp>
#include <boost/circular_buffer.hpp>
#include <boost/thread/condition.hpp>

namespace realtime_utilities
{
template <class T> class vector
{
private:
  std::vector<T>          standard_vector;
  mutable boost::mutex    mutex;

public:
  typedef typename std::vector<T>::iterator  iterator;
  typedef typename std::vector<T>::size_type size_type;

  explicit vector(const std::allocator<T>& alloc = std::allocator<T>()) : standard_vector(alloc) {}
  explicit vector(size_type n)                                    : standard_vector(n) {}
  vector(size_type n, const T& val, const std::allocator<T>& alloc = std::allocator<T>()) : standard_vector(n, val, alloc) {}

  template <class InputIterator>
  vector(InputIterator first, InputIterator last, const std::allocator<T>& alloc = std::allocator<T>()) : standard_vector(first, last, alloc) {};
  vector(const vector& x) : standard_vector(x) {}
  vector(const vector& x, const std::allocator<T>& alloc) : standard_vector(x, alloc) {}
  vector(vector&& x) : standard_vector(x) {}
  vector(vector&& x, const std::allocator<T>& alloc) : standard_vector(x, alloc) {}
  vector(std::initializer_list< T > il, const std::allocator<T>& alloc = std::allocator<T>()) : standard_vector(il, alloc) {}

  iterator begin(void)
  {
    boost::mutex::scoped_lock lock(mutex);
    return standard_vector.begin();
  }

  iterator end(void)
  {
    boost::mutex::scoped_lock lock(mutex);
    return standard_vector.end();
  }

  void push_back(T & item)
  {
    boost::mutex::scoped_lock lock(mutex);
    standard_vector.push_back(item);
  }
  void push_back(const T & item)
  {
    boost::mutex::scoped_lock lock(mutex);
    standard_vector.push_back(item);
  }

  void erase(iterator it)
  {
    boost::mutex::scoped_lock lock(mutex);
    standard_vector.erase(it);
  }
  std::vector<T>& get()
  {
    return standard_vector;
  }

};

}

#endif  // REALTIME_UTILITIES_VECTOR_MT_H
