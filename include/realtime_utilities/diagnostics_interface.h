#ifndef REALTIME_UTILITIES__DIAGNOSTICS_INTERFACE__H
#define REALTIME_UTILITIES__DIAGNOSTICS_INTERFACE__H

#include <memory>
#include <ctime>
#include <chrono>
#include <algorithm>
#include <mutex>
#include <string>
#include <sstream>
#include <diagnostic_msgs/DiagnosticArray.h>
#include <diagnostic_updater/DiagnosticStatusWrapper.h>
#include <realtime_utilities/time_span_tracker.h>

namespace realtime_utilities
{

class DiagnosticsInterface
{
public:

  typedef std::shared_ptr<DiagnosticsInterface> Ptr;
  typedef std::shared_ptr<DiagnosticsInterface const> ConstPtr;

  DiagnosticsInterface() = default;
  virtual ~DiagnosticsInterface() = default;
  DiagnosticsInterface(const DiagnosticsInterface&) = delete;
  DiagnosticsInterface& operator=(const DiagnosticsInterface&) = delete;
  DiagnosticsInterface(DiagnosticsInterface&&) = delete;
  DiagnosticsInterface& operator=(DiagnosticsInterface&&) = delete;

  /**
   * @param hardware_id  The HardwareID of a DiagnosticStatus message
   * @param name_id      The prefix of the name of the DiagnosticStatus. 
   *                     It will be concat with the message level, i.e. name = "name_id [level]""
   * @param timer_prefix if a timer is used, the message will be the ID in the time table 
   * HardwareID: hardware_id
   * Component:  hardware_id: name_id [level]
   * 
   */
  void init(const std::string& hw_id, const std::string& name_id, const std::string& timer_id) 
  {
    hardware_id_ = hw_id;
    name_id_ = name_id; 
    timer_id_ = timer_id; 
  };

  const std::string& nameId() const { return name_id_; }
  const std::string& hardwareId() const { return hardware_id_; }

  void addDiagnosticsMessage( const std::string& level
                          , const std::string& summary
                          , const std::map<std::string, std::string>& key_values
                          , std::stringstream* report);

  virtual void addTimeTracker(const std::string& id, const double& period);

  virtual void diagnostics           (diagnostic_updater::DiagnosticStatusWrapper &stat, int level);
  virtual void diagnosticsInfo       (diagnostic_updater::DiagnosticStatusWrapper &stat);
  virtual void diagnosticsWarn       (diagnostic_updater::DiagnosticStatusWrapper &stat);
  virtual void diagnosticsError      (diagnostic_updater::DiagnosticStatusWrapper &stat);
  virtual void diagnosticsPerformance(diagnostic_updater::DiagnosticStatusWrapper &stat);

  realtime_utilities::TimeSpanTrackerPtr  timeSpanStrakcer(const std::string& id) { return time_span_tracker_.at(id); };
private:
  std::string    hardware_id_;
  std::string    name_id_;
  std::string    timer_id_;

  std::mutex     mtx_;
  mutable diagnostic_msgs::DiagnosticArray                       diagnostic_;
  std::map<std::string, realtime_utilities::TimeSpanTrackerPtr>  time_span_tracker_;
  std::map<std::string, double >                                 period_;
};

template <typename T>
inline std::string to_string_fix(const T a_value, const int n = 5)
{
  std::ostringstream out;
  out.precision(n);
  out << std::fixed << a_value;
  return out.str();
}

typedef DiagnosticsInterface::Ptr  DiagnosticsInterfacePtr;
typedef DiagnosticsInterface::ConstPtr  DiagnosticsInterfaceConstPtr;

}  // namespace realtime_utilities

#endif  // REALTIME_UTILITIES__DIAGNOSTICS_INTERFACE__H
