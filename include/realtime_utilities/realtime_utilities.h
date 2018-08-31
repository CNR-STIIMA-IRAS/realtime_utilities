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

namespace realtime_utilities
{

struct period_info {
  struct timespec next_period;
  long period_ns;
};
  
  
bool setprio(int prio, int sched);

bool show_new_pagefault_count(const char* logtext, const char* allowed_maj, const char* allowed_min);

bool prove_thread_stack_use_is_safe(size_t stacksize);

bool error(int at);

bool configure_malloc_behavior(void);

bool reserve_process_memory(size_t size);

bool rt_main_init( size_t pre_allocation_size );

bool rt_init_thread ( size_t        stack_size
                    , int           prio        = sched_get_priority_max(SCHED_RR)
                    , int           sched       = SCHED_RR
                    , period_info*  pinfo       = NULL
                    , long          period_ns  = 0 );


uint32_t timer_inc_period(period_info *pinfo);

uint32_t timer_inc_period(period_info *pinfo, int64_t offest_time );

void   timer_periodic_init(period_info *pinfo, long period_ns);

int    timer_wait_rest_of_period(struct timespec *ts);

void   timer_add_timespec (struct timespec *ts, int64_t addtime);

void   timer_calc_sync_offset(int64_t reftime, int64_t cycletime , int64_t *offsettime);

double timer_difference_s( struct timespec *timeA_p, struct timespec *timeB_p );

double timer_to_s( struct timespec *timeA_p );
 
std::vector<std::string> get_ifaces();

}

#endif