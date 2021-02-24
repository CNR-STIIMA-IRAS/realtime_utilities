#ifndef REALTIME_UTILITIES_TIME_SPAN_TRACKER_H
#define REALTIME_UTILITIES_TIME_SPAN_TRACKER_H

#include <mutex>
#include <boost/thread.hpp>
#include <boost/circular_buffer.hpp>
#include <boost/thread/condition.hpp>
#include <realtime_utilities/circular_buffer.h>

namespace realtime_utilities
{

struct TimeSpanTracker
{
  typedef std::shared_ptr< TimeSpanTracker > Ptr;
  typedef std::shared_ptr< TimeSpanTracker const > ConstPtr;

  const double                                   nominal_time_span_;
  size_t                                         cycles_;
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
    cycles_++;
    last_tick_ = t;
    return true;
  }

  bool tick()
  {
    std::lock_guard<std::mutex> lock(mtx_);
    if (mode_ == TIME_SPAN)
      return false;

    mode_ = TICK_TOCK;

    last_tick_ = std::chrono::high_resolution_clock::now();
    return true;
  }

  bool tock()
  {
    std::lock_guard<std::mutex> lock(mtx_);

    if (mode_ == TIME_SPAN)
      return false;

    mode_ = TICK_TOCK;

    std::chrono::high_resolution_clock::time_point t = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> ts = std::chrono::duration_cast< std::chrono::duration<double> >(t - last_tick_);
    buffer_.push_back(ts.count());
    if (ts.count() > 1.2 * nominal_time_span_)
    {
      missed_cycles_++;
    }
    cycles_++;
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
  size_t   getTotalCycles() const
  {
    std::lock_guard<std::mutex> lock(mtx_);
    return cycles_;
  }

  TimeSpanTracker() = delete;
  virtual ~TimeSpanTracker() = default;
  TimeSpanTracker(const TimeSpanTracker&) = delete;
  TimeSpanTracker& operator=(const TimeSpanTracker&) = delete;
  TimeSpanTracker(TimeSpanTracker&&) = delete;
  TimeSpanTracker& operator=(TimeSpanTracker&&) = delete;

  TimeSpanTracker(const int windows_dim, const double nominal_time_span)
    : nominal_time_span_(nominal_time_span), cycles_(0),missed_cycles_(0), mode_(NONE), buffer_(windows_dim) {}
};

typedef TimeSpanTracker::Ptr TimeSpanTrackerPtr;
typedef TimeSpanTracker::ConstPtr TimeSpanTrackerConstPtr;

}  // namespace realtime_utilities

#endif  // REALTIME_UTILITIES_TIME_SPAN_TRACKER_H
