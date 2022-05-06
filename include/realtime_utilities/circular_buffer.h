#ifndef REALTIME_UTILITIES_CIRCULAR_BUFFER_H
#define REALTIME_UTILITIES_CIRCULAR_BUFFER_H

#include <mutex>
#include <boost/thread.hpp>
#include <boost/circular_buffer.hpp>
#include <boost/thread/condition.hpp>

namespace realtime_utilities
{

// Thread safe circular buffer
template <typename T>
class circ_buffer : private boost::noncopyable
{
public:
  typedef boost::mutex::scoped_lock lock;
  circ_buffer() = default;
  virtual ~circ_buffer() {}
  circ_buffer(int n)
  {
    cb.set_capacity(n);
  }
  virtual void push_back(const T& imdata)
  {
    lock lk(monitor);
    cb.push_back(imdata);
    buffer_not_empty.notify_one();
  }
  virtual void push_front(const T& imdata)
  {
    lock lk(monitor);
    cb.push_front(imdata);
    buffer_not_empty.notify_one();
  }
  virtual const T& front()
  {
    lock lk(monitor);
    while (cb.empty())
      buffer_not_empty.wait(lk);
    return cb.front();
  }
  virtual const T& back()
  {
    lock lk(monitor);
    while (cb.empty())
      buffer_not_empty.wait(lk);
    return cb.back();
  }

  virtual void pop_front()
  {
    lock lk(monitor);
    if (cb.empty())
      return;
    return cb.pop_front();
  }

  virtual void clear()
  {
    lock lk(monitor);
    cb.clear();
  }

  virtual int size()
  {
    lock lk(monitor);
    return cb.size();
  }

  virtual void set_capacity(int capacity)
  {
    lock lk(monitor);
    cb.set_capacity(capacity);
  }

  virtual bool empty()
  {
    lock lk(monitor);
    return cb.empty();
  }

  virtual bool full()
  {
    lock lk(monitor);
    return cb.full();
  }

  virtual boost::circular_buffer<T>& get()
  {
    return cb;
  }

  virtual const boost::circular_buffer<T>& get() const
  {
    return cb;
  }

  typename boost::circular_buffer<T>::iterator begin()
  {
    lock lk(monitor);
    return cb.begin();
  }
  typename boost::circular_buffer<T>::iterator end()
  {
    lock lk(monitor);
    return cb.end();
  }

  typename boost::circular_buffer<T>::const_iterator begin() const
  {
    lock lk(monitor);
    return cb.begin();
  }
  typename boost::circular_buffer<T>::const_iterator end() const
  {
    lock lk(monitor);
    return cb.end();
  }

  typename boost::circular_buffer<T>::const_iterator cbegin() const
  {
    lock lk(monitor);
    return cb.cbegin();
  }
  typename boost::circular_buffer<T>::const_iterator cend() const
  {
    lock lk(monitor);
    return cb.cend();
  }


private:
  size_t cnt;
  boost::condition buffer_not_empty;
  mutable boost::mutex monitor;
  boost::circular_buffer<T> cb;
};


template< class K, class V >
std::map< K, V > combine(const std::vector< K >& keys, const std::vector< V >& values)
{
  assert(keys.size() == values.size());
  std::map< K, V > ret;
  for (size_t i = 0; i < keys.size(); i++)
  {
    ret[ keys[i] ] = values[ i ];
  }
  return ret;
}

template< typename T>
double mean(const boost::circular_buffer<T>& cb)
{
  T ret = 0;
  if (cb.size() == 0)
    return ret;

  for (auto const & element : cb)
  {
    ret += element;
  }
  return double(ret) / double(cb.size());
}

template< typename T>
T max(const boost::circular_buffer<T>& cb)
{
  T ret = 0;
  for (auto const & element : cb)
  {
    ret = std::max(element, ret);
  }
  return ret;
}
template< typename T>
T min(const boost::circular_buffer<T>& cb)
{
  T ret = 0;
  for (auto const & element : cb)
  {
    ret = std::min(element, ret);
  }
  return ret;
}

}  // namespace realtime_utilities

#endif  // REALTIME_UTILITIES_CIRCULAR_BUFFER_H
