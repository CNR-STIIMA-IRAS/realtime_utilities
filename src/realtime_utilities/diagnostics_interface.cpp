#include <ctime>
#include <chrono>
#include <algorithm>
#include <mutex>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <realtime_utilities/diagnostics_interface.h>

namespace realtime_utilities
{

void DiagnosticsInterface::addDiagnosticsMessage(const std::string& msg, const std::string& name,
                                             const std::string& level, std::stringstream* report)
{
  diagnostic_msgs::DiagnosticStatus diag;
  diag.name       = name;
  diag.hardware_id = m_hw_name;
  diag.message    = " [ " + m_hw_name + " ] " + msg;

  if (level == "OK")
  {
    diag.level = diagnostic_msgs::DiagnosticStatus::OK;
    if(report) *report << "[" << m_hw_name << "] " << msg;
  }
  if (level == "WARN")
  {
    diag.level = diagnostic_msgs::DiagnosticStatus::WARN;
    if(report) *report << "[" << m_hw_name << "] " << msg;
  }
  if (level == "ERROR")
  {
    diag.level = diagnostic_msgs::DiagnosticStatus::ERROR;
    if(report) *report << "[" << m_hw_name << "] " << msg;
  }
  if (level == "STALE")
  {
    diag.level = diagnostic_msgs::DiagnosticStatus::STALE;
    if(report) *report << "[" << m_hw_name << "] " << msg;
  }

  std::lock_guard<std::mutex> lock(m_mutex);
  m_diagnostic.status.push_back(diag);
}

void DiagnosticsInterface::diagnostics(diagnostic_updater::DiagnosticStatusWrapper &stat, int level)
{
  std::lock_guard<std::mutex> lock(m_mutex);
  boost::posix_time::ptime my_posix_time = ros::Time::now().toBoost();

  stat.hardware_id = m_hw_name;
  stat.name        = "Ctrl ["
                   + ( level == (int)diagnostic_msgs::DiagnosticStatus::OK  ? std::string("Info")
                     : level == (int)diagnostic_msgs::DiagnosticStatus::WARN ? std::string("Warn")
                     : std::string("Error") )
                   +"]";

  bool something_to_add = false;
  for (  const diagnostic_msgs::DiagnosticStatus & s : m_diagnostic.status )
  {
    something_to_add |= static_cast<int>( s.level ) == level;
  }
  if ( something_to_add )
  {
    stat.level       = level == (int)diagnostic_msgs::DiagnosticStatus::OK ? diagnostic_msgs::DiagnosticStatus::OK
                     : level == (int)diagnostic_msgs::DiagnosticStatus::WARN ? diagnostic_msgs::DiagnosticStatus::WARN
                     : level == (int)diagnostic_msgs::DiagnosticStatus::ERROR ? diagnostic_msgs::DiagnosticStatus::ERROR
                     : diagnostic_msgs::DiagnosticStatus::STALE;

    stat.summary(stat.level, "Log of the status at ["
         + boost::posix_time::to_iso_string(my_posix_time) + "]");

    for ( const diagnostic_msgs::DiagnosticStatus & s : m_diagnostic.status )
    {
      diagnostic_msgs::KeyValue k;
      k.key = s.name;
      k.value = s.message;
      stat.add(k.key, k.value);
    }
    m_diagnostic.status.erase(
        std::remove_if(
            m_diagnostic.status.begin(),
            m_diagnostic.status.end(),
            [&](diagnostic_msgs::DiagnosticStatus const & p) { return p.level == level; }
        ),
        m_diagnostic.status.end()
    );
  }
  else
  {
    stat.summary(diagnostic_msgs::DiagnosticStatus::OK, "None Error in the queue ["
         + boost::posix_time::to_iso_string(my_posix_time) + "]");
  }
}

void DiagnosticsInterface::diagnosticsInfo(diagnostic_updater::DiagnosticStatusWrapper &stat)
{
  DiagnosticsInterface::diagnostics(stat,diagnostic_msgs::DiagnosticStatus::OK);
}

void DiagnosticsInterface::diagnosticsWarn(diagnostic_updater::DiagnosticStatusWrapper &stat)
{
  DiagnosticsInterface::diagnostics(stat,diagnostic_msgs::DiagnosticStatus::WARN);
}

void DiagnosticsInterface::diagnosticsError(diagnostic_updater::DiagnosticStatusWrapper &stat)
{
  DiagnosticsInterface::diagnostics(stat,diagnostic_msgs::DiagnosticStatus::ERROR);
}

void DiagnosticsInterface::diagnosticsPerformance(diagnostic_updater::DiagnosticStatusWrapper &stat)
{

  boost::posix_time::ptime my_posix_time = ros::Time::now().toBoost();
  std::lock_guard<std::mutex> lock(m_mutex);
  stat.hardware_id = m_hw_name;
  stat.level       = diagnostic_msgs::DiagnosticStatus::OK;
  stat.name        = "Ctrl";
  stat.message     = "Cycle Time Statistics [" + boost::posix_time::to_iso_string(my_posix_time) + "]";
  for(auto const & tracker :  m_time_span_tracker )
  {
    diagnostic_msgs::KeyValue k;
    k.key = m_ctrl_name + " " + tracker.first + " [s]";
    k.value = to_string_fix(tracker.second->getMean())
            + std::string(" [ ") + to_string_fix(tracker.second->getMin()) + " - "
            + to_string_fix(tracker.second->getMax()) + std::string(" ] ")
            + std::string("Missed: ") + std::to_string(tracker.second->getMissedCycles())
            + std::string("/") + std::to_string(tracker.second->getTotalCycles());

    stat.add(k.key, k.value);
  }
}

void DiagnosticsInterface::addTimeTracker(const std::string& id)
{
  m_time_span_tracker[id].reset(new realtime_utilities::TimeSpanTracker(int(10.0/m_sampling_period), m_sampling_period));
}

}