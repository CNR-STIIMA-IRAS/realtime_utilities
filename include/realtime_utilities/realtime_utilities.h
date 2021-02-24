#ifndef __ITIA__RT__UTILS__H__
#define __ITIA__RT__UTILS__H__


#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h>  // Needed for mlockall()
#include <unistd.h>    // needed for sysconf(int name);
#include <malloc.h>
#include <sys/time.h>  // needed for getrusage
#include <sys/resource.h>  // needed for getrusage
#include <pthread.h>
#include <limits.h>
#if defined(__COBALT__) && !defined(__COBALT_WRAP__)
#include <alchemy/task.h>
#endif
#include <iostream>
#include <ctime>
#include <cassert>
#include <ratio>
#include <chrono>
#include <mutex>
#include <string>
#include <vector>

namespace realtime_utilities
{

struct period_info
{
  struct timespec next_period;
  long period_ns;
};


bool setprio(int prio, int sched);

bool show_new_pagefault_count(const char* logtext, const char* allowed_maj, const char* allowed_min);

bool prove_thread_stack_use_is_safe(size_t stacksize);

bool error(int at);

bool configure_malloc_behavior(void);

bool reserve_process_memory(size_t size);

bool rt_main_init(size_t pre_allocation_size);

uint32_t timer_inc_period(period_info *pinfo);

uint32_t timer_inc_period(period_info *pinfo, int64_t offest_time);

void   timer_periodic_init(period_info *pinfo, long period_ns);

int    timer_wait_rest_of_period(struct timespec *ts);

void   timer_add(struct timespec *ts, int64_t addtime);


void    timer_calc_sync_offset(int64_t reftime, int64_t cycletime, int64_t *offsettime);
double  timer_difference_s(struct timespec const * timeA_p, struct timespec const *timeB_p);
int64_t timer_difference_ns(struct timespec const * timeA_p, struct timespec const *timeB_p);
bool    timer_greater_than(struct timespec const * timeA_p, struct timespec const *timeB_p);
double  timer_to_s(const struct timespec *timeA_p);
int64_t timer_to_ns(const struct timespec *timeA_p);

std::vector<std::string> get_ifaces();


inline
bool rt_init_thread(size_t stack_size, int prio, int sched, period_info*  pinfo, long  period_ns)
{
#if defined(__COBALT__) && !defined(__COBALT_WRAP__)
  RT_TASK *curtask;
  RT_TASK_INFO curtaskinfo;
  curtask = rt_task_self();
  int r = rt_task_inquire(curtask, &curtaskinfo);
  if (r != 0)
  {
    switch (r)
    {
    case EINVAL:
    case -EINVAL:
      printf("task is not a valid task descriptor, or if prio is invalid.");
      break;
    case EPERM :
    case -EPERM :
      printf("task is NULL and this service was called from an invalid context.");
      break;
    }
    return false;
  }

  r = rt_task_set_priority(NULL, prio);
  if (r != 0)
  {
    switch (r)
    {
    case EINVAL:
    case -EINVAL:
      printf("task is not a valid task descriptor, or if prio is invalid.");
      break;
    case EPERM :
    case -EPERM :
      printf("task is NULL and this service was called from an invalid context.");
      break;
    }
    return false;
  }

  //Make the task periodic with a specified loop period
  rt_task_set_periodic(NULL, TM_NOW, period_ns);
#else

  if (!setprio(prio, sched))
  {
    printf("Error in setprio.");
    return false;
  }

  printf("I am an RT-thread with a stack that does not generate page-faults during use, stacksize=%zu\n", stack_size);

  //<do your RT-thing here>

  show_new_pagefault_count("Caused by creating thread", ">=0", ">=0");
  prove_thread_stack_use_is_safe(stack_size);
#endif

  if (pinfo != NULL)
  {
    assert(period_ns > 0);
    timer_periodic_init(pinfo, period_ns);
  }

  return true;
}




}

#endif
