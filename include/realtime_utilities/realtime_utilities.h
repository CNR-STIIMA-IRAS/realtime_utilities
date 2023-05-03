#ifndef REALTIME_UTILITIES__REALTIME_UTILITIES_H
#define REALTIME_UTILITIES__REALTIME_UTILITIES_H

#include <cassert>
#include <chrono>
#include <ctime>
#include <limits.h>
#include <malloc.h>
#include <mutex>
#include <pthread.h>
#include <ratio>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sys/mman.h>     // Needed for mlockall()
#include <sys/resource.h> // needed for getrusage
#include <sys/time.h>     // needed for getrusage
#include <unistd.h>       // needed for sysconf(int name);
#include <vector>

namespace realtime_utilities
{

  struct period_info
  {
    struct timespec next_period;
    long period_ns;
  };

  bool setprio(int prio, int sched);

  bool show_new_pagefault_count(const char *logtext, const char *allowed_maj, const char *allowed_min);

  bool prove_thread_stack_use_is_safe(size_t stacksize);

  bool error(int at);

  bool configure_malloc_behavior(void);

  bool reserve_process_memory(size_t size);

  bool rt_main_init(size_t pre_allocation_size);

  bool rt_init_thread(size_t stack_size, int prio, int sched, period_info *pinfo, long period_ns);

  // Time
  uint32_t timer_inc_period(period_info *pinfo);

  uint32_t timer_inc_period(period_info *pinfo, int64_t offest_time);

  void timer_periodic_init(period_info *pinfo, long period_ns);

  int timer_wait_rest_of_period(struct timespec *ts);

  void timer_add(struct timespec *ts, int64_t addtime);

  void timer_calc_sync_offset(int64_t reftime, int64_t cycletime, int64_t *offsettime);

  double timer_difference_s(struct timespec const *timeA_p, struct timespec const *timeB_p);

  int64_t timer_difference_ns(struct timespec const *timeA_p, struct timespec const *timeB_p);

  bool timer_greater_than(struct timespec const *timeA_p, struct timespec const *timeB_p);

  double timer_to_s(const struct timespec *timeA_p);
  
  int64_t timer_to_ns(const struct timespec *timeA_p);

  // network devices i/o
  std::vector<std::string> get_ifaces();


} // namespace realtime_utilities

#endif // REALTIME_UTILITIES__REALTIME_UTILITIES_H