#include <ctime>
#include <chrono>
#include <algorithm>
#include <mutex>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <realtime_utilities/diagnostics_interface.h>

namespace realtime_utilities
{

void DiagnosticsInterface::addDiagnosticsMessage(const std::string& level
                                                , const std::string& summary
                                                , const std::map<std::string, std::string>& key_values
                                                , std::stringstream* report)
{
  diagnostic_msgs::DiagnosticStatus diag;
  diag.name        = name_id_;
  diag.hardware_id = hardware_id_;
  diag.message     = summary; // " [ " + hardware_id_ + " ] " + msg;

  if (level == "OK")
  {
    diag.level = diagnostic_msgs::DiagnosticStatus::OK;
    if(report) *report << "[" << hardware_id_ << "] " << summary;
  }
  if (level == "WARN")
  {
    diag.level = diagnostic_msgs::DiagnosticStatus::WARN;
    if(report) *report << "[" << hardware_id_ << "] " << summary;
  }
  if (level == "ERROR")
  {
    diag.level = diagnostic_msgs::DiagnosticStatus::ERROR;
    if(report) *report << "[" << hardware_id_ << "] " << summary;
  }
  if (level == "STALE")
  {
    diag.level = diagnostic_msgs::DiagnosticStatus::STALE;
    if(report) *report << "[" << hardware_id_ << "] " << summary;
  }

  for (const auto & key_value : key_values)
  {
    diagnostic_msgs::KeyValue kv;
    kv.key = key_value.first;
    kv.value = key_value.second;
    diag.values.push_back(kv);
  }
  std::lock_guard<std::mutex> lock(mtx_);
  diagnostic_.status.push_back(diag);
}

void DiagnosticsInterface::diagnostics(diagnostic_updater::DiagnosticStatusWrapper &stat, int level)
{
  std::lock_guard<std::mutex> lock(mtx_);
  boost::posix_time::ptime my_posix_time = boost::posix_time::microsec_clock::local_time();

  stat.hardware_id = hardware_id_;
  stat.name        = name_id_ + "["
                   + ( level == (int)diagnostic_msgs::DiagnosticStatus::OK  ? std::string("Info")
                     : level == (int)diagnostic_msgs::DiagnosticStatus::WARN ? std::string("Warn")
                     : std::string("Error") )
                   +"]";

  bool something_to_add = false;
  for (  const diagnostic_msgs::DiagnosticStatus & s : diagnostic_.status )
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
         + boost::posix_time::to_iso_string(my_posix_time.time_of_day()) + "]");

    for ( const diagnostic_msgs::DiagnosticStatus & s : diagnostic_.status )
    {
      diagnostic_msgs::KeyValue k;
      k.key = s.name;
      k.value = s.message;
      stat.add(k.key, k.value);
    }
    diagnostic_.status.erase(
        std::remove_if(
            diagnostic_.status.begin(),
            diagnostic_.status.end(),
            [&](diagnostic_msgs::DiagnosticStatus const & p) { return p.level == level; }
        ),
        diagnostic_.status.end()
    );
  }
  else
  {
    stat.summary(diagnostic_msgs::DiagnosticStatus::OK, "None Error in the queue ["
         + boost::posix_time::to_iso_string(my_posix_time.time_of_day()) + "]");
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

  boost::posix_time::ptime my_posix_time = boost::posix_time::microsec_clock::local_time();
  std::lock_guard<std::mutex> lock(mtx_);
  stat.hardware_id = hardware_id_;
  stat.level       = diagnostic_msgs::DiagnosticStatus::OK;
  stat.name        = name_id_;
  stat.message     = "Cycle Time Statistics [" + boost::posix_time::to_iso_string(my_posix_time.time_of_day()) + "]";
  for(auto const & tracker :  time_span_tracker_ )
  {
    diagnostic_msgs::KeyValue k;
    k.key = timer_id_ + " " + tracker.first + " [s]";
    k.value = to_string_fix(tracker.second->getMean())
            + std::string(" [ ") + to_string_fix(tracker.second->getMin()) + " - "
            + to_string_fix(tracker.second->getMax()) + std::string(" ] ")
            + std::string("Missed: ") + std::to_string(tracker.second->getMissedCycles())
            + std::string("/") + std::to_string(tracker.second->getTotalCycles());

    stat.add(k.key, k.value);
  }
}

void DiagnosticsInterface::addTimeTracker(const std::string& id, const double& period)
{
  time_span_tracker_[id].reset(new realtime_utilities::TimeSpanTracker(int(10.0/period), period));
  period_[id] = period;
}

}