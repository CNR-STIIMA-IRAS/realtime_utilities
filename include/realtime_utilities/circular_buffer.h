#ifndef __ITIA_BUTILS__
#define __ITIA_BUTILS__

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
  circ_buffer() {}
  ~circ_buffer() {}
  circ_buffer(int n)
  {
    cb.set_capacity(n);
  }
  void push_back(T imdata)
  {
    lock lk(monitor);
    cb.push_back(imdata);
    buffer_not_empty.notify_one();
  }
  void push_front(T imdata)
  {
    lock lk(monitor);
    cb.push_front(imdata);
    buffer_not_empty.notify_one();
  }
  const T& front()
  {
    lock lk(monitor);
    while (cb.empty())
      buffer_not_empty.wait(lk);
    return cb.front();
  }
  const T& back()
  {
    lock lk(monitor);
    while (cb.empty())
      buffer_not_empty.wait(lk);
    return cb.back();
  }

  void pop_front()
  {
    lock lk(monitor);
    if (cb.empty())
      return;
    return cb.pop_front();
  }

  void clear()
  {
    lock lk(monitor);
    cb.clear();
  }

  int size()
  {
    lock lk(monitor);
    return cb.size();
  }

  void set_capacity(int capacity)
  {
    lock lk(monitor);
    cb.set_capacity(capacity);
  }

  bool empty()
  {
    lock lk(monitor);
    return cb.empty();
  }

  bool full()
  {
    lock lk(monitor);
    return cb.full();
  }

  boost::circular_buffer<T>& get()
  {
    return cb;
  }

  const boost::circular_buffer<T>& get() const
  {
    return cb;
  }

  typename boost::circular_buffer<T>::iterator       begin()
  {
    lock lk(monitor);
    return cb.begin();
  }
  typename boost::circular_buffer<T>::iterator       end()
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
  boost::mutex monitor;
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


struct TimeSpanTracker
{
  const double                                   nominal_time_span_;
  size_t                                         missed_cycles_;
  enum { NONE, TIME_SPAN, TICK_TOCK }            mode_;

  mutable std::mutex                             mtx_;
  realtime_utilities::circ_buffer<double>        buffer_;
  std::chrono::high_resolution_clock::time_point last_tick_;
  bool time_span()
  {
    std::lock_guard<std::mutex> lock(mtx_);
    if (mode_ == TICK_TOCK)
      return false;

    mode_ = TIME_SPAN;
    std::chrono::high_resolution_clock::time_point t = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> ts = std::chrono::duration_cast< std::chrono::duration<double> >(t - last_tick_);
    buffer_.push_back(ts.count());
    if (ts.count() > 1.2 * nominal_time_span_)
    {
      missed_cycles_++;
    }
    last_tick_ = t;
    return true;
  }

  bool tick()
  {
    if (mode_ == TIME_SPAN)
      return false;

    mode_ = TICK_TOCK;

    std::lock_guard<std::mutex> lock(mtx_);
    last_tick_ = std::chrono::high_resolution_clock::now();
    return true;
  }

  bool tock()
  {
    if (mode_ == TIME_SPAN)
      return false;

    mode_ = TICK_TOCK;

    std::lock_guard<std::mutex> lock(mtx_);
    std::chrono::high_resolution_clock::time_point t = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> ts = std::chrono::duration_cast< std::chrono::duration<double> >(t - last_tick_);
    buffer_.push_back(ts.count());
    if (ts.count() > 1.2 * nominal_time_span_)
    {
      missed_cycles_++;
    }
    last_tick_ = t;
    return true;
  }

  double   getMean()        const
  {
    std::lock_guard<std::mutex> lock(mtx_);
    return realtime_utilities::mean(buffer_.get());
  }
  double   getMax()        const
  {
    std::lock_guard<std::mutex> lock(mtx_);
    return realtime_utilities::max(buffer_.get());
  }
  double   getMin()        const
  {
    std::lock_guard<std::mutex> lock(mtx_);
    return realtime_utilities::min(buffer_.get());
  }
  size_t   getMissedCycles() const
  {
    std::lock_guard<std::mutex> lock(mtx_);
    return missed_cycles_;
  }

  TimeSpanTracker(const int windows_dim, const double nominal_time_span)
    : nominal_time_span_(nominal_time_span), missed_cycles_(0), mode_(NONE), buffer_(windows_dim) {}
  //TimeSpanTracker( const TimeSpanTracker& ts): buffer_(ts.windows_dim_), nominal_time_span_(ts.nominal_time_span_), missed_cycles_(0), mode_(NONE){}
};

}

#endif
