#ifndef REALTIME_UTILITIES__DIAGNOSTICS_INTERFACE__H
#define REALTIME_UTILITIES__DIAGNOSTICS_INTERFACE__H

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

  /**
   * @brief addDiagnosticsMessage
   * @param msg
   * @param name
   * @param level
   * @param verbose
   */
  virtual void addDiagnosticsMessage( const std::string& msg,
                                      const std::string& name,
                                      const std::string& level,
                                      std::stringstream* report);

  virtual void diagnostics     (diagnostic_updater::DiagnosticStatusWrapper &stat, int level);
  virtual void diagnosticsInfo (diagnostic_updater::DiagnosticStatusWrapper &stat)           ;
  virtual void diagnosticsWarn (diagnostic_updater::DiagnosticStatusWrapper &stat)           ;
  virtual void diagnosticsError(diagnostic_updater::DiagnosticStatusWrapper &stat)           ;
  virtual void diagnosticsPerformance(diagnostic_updater::DiagnosticStatusWrapper &stat)     ;

  virtual void addTimeTracker(const std::string& id);

protected:
  std::string                                           m_hw_name;
  std::string                                           m_ctrl_name;
  mutable std::mutex                                    m_mutex;
  mutable diagnostic_msgs::DiagnosticArray              m_diagnostic;
  std::vector<std::string>                              m_status_history;
  std::map<std::string, realtime_utilities::TimeSpanTrackerPtr>  m_time_span_tracker;
  double                                                m_sampling_period;
  double                                                m_watchdog;

};

template <typename T>
std::string to_string_fix(const T a_value, const int n = 5)
{
  std::ostringstream out;
  out.precision(n);
  out << std::fixed << a_value;
  return out.str();
}


}  // namespace realtime_utilities

#endif  // REALTIME_UTILITIES__DIAGNOSTICS_INTERFACE__H