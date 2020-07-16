

namespace realtime_utilities
{

inline
std::string rt_pipe_create_error(int val)
{
  switch (val)
  {
  case -ENOMEM:
    return "the system fails to get memory from the main heap in order to create the pipe.";
  case -EEXIST:
    return "is returned if the name is conflicting with an already registered pipe.";
  case -ENODEV:
    return "minor is different from P_MINOR_AUTO and is not a valid minor number for the pipe special device either (i.e. /dev/rtp*).";
  case -EBUSY :
    return "minor is already open.";
  case -EPERM :
    return "this service was called from an invalid context, e.g. interrupt or non-Xenomai thread.";
  default:
    return "value '" + std::to_string(val) + " is not recognized.";
  }
}

/*
 * -ENOMEM is returned if .
-ENODEV is returned if
-EEXIST .
-EBUSY is returned if minor is already open.
-EPERM is returned if .
*/
inline
std::string rt_pipe_read_error(int val)
{
  switch (val)
  {
  case -ETIMEDOUT   :
    return "abs_timeout is reached before a message arrives..";
  case -EWOULDBLOCK :
    return "abs_timeout is { .tv_sec = 0, .tv_nsec = 0 } and no message is immediately available on entry to the call";
  case -EINTR       :
    return "rt_task_unblock() was called for the current task before a message was available";
  case -EINVAL      :
    return "q is not a valid queue descriptor.";
  case -EIDRM       :
    return "q is deleted while the caller was waiting for a message. In such event, q is no more valid upon return of this service";
  case -EPERM       :
    return "this service should block, but was not called from a Xenomai thread";
  default:
    return "value '" + std::to_string(val) + " is not recognized.";
  }
}

inline
std::string rt_pipe_write_error(int val)
{
  switch (val)
  {

  case -ENOMEM      :
    return "not enough buffer space is available to complete the operation.";
  case -EINVAL      :
    return "mode is invalid or pipe is not a pipe descriptor";
  case -EIDRM       :
    return "pipe is a closed pipe descriptor";
  default:
    return "value '" + std::to_string(val) + " is not recognized.";
  }
}

inline
std::string rt_pipe_delete_error(int val)
{
  switch (val)
  {
  case -EINVAL:
    return ("pipe is not a valid pipe descriptor. Abort");
  case -EIDRM :
    return ("is returned if pipe is a closed pipe descriptor. Abort");
  case -EPERM :
    return ("this service was called from an asynchronous context. Abort");
  }
  return "Error not tracked.";
}

inline
RealTimeIPC::RealTimeIPC(const std::string& identifier, double operational_time, double watchdog_decimation, const AccessMode& mode, const size_t dim)
  : access_mode_(mode)
  , operational_time_(operational_time)
  , watchdog_(watchdog_decimation)
  , name_(identifier)
  , dim_with_header_(dim + sizeof(RealTimeIPC::DataPacket::Header))         //time and bonding index
  , start_watchdog_time_(-1)
  , data_time_prev_(0)
  , flush_time_prev_(0)
  , bond_cnt_(0)
  , bonded_prev_(false)
  , is_hard_rt_prev_(false)
{
  if (!init())
  {
    throw std::runtime_error("Error in Init RealTimeIPC. Abort.");
  }
}

inline
bool RealTimeIPC::init()
{
  rt_skin_    = POSIX;

  // RT definitions
  printf("[%sSTART%s] %sRealTimeIPC Init%s [ %s%s%s ] ========================\n", BOLDMAGENTA(), RESET(), BOLDBLUE(), RESET(), BOLDCYAN(), name_.c_str(), RESET());
  bool ok = true;
  try
  {
    //------------------------------------------
    boost::interprocess::permissions permissions(0677);
    switch (access_mode_)
    {
    case PIPE_SERVER:
    {
      assert(rt_skin_ == RT_ALCHEMY);
      assert(0);
    }
    break;

    case PIPE_CLIENT:
    {
      assert(rt_skin_ == POSIX);
      assert(0);
    }
    break;
    case SHMEM_SERVER:
    {
      if (dim_with_header_ > sizeof(RealTimeIPC::DataPacket::Header))
      {

        // store old
        mode_t old_umask = umask(0);

        printf("[-----] %sRealTimeIPC Init%s [ %s%s%s ] Create memory (bytes %s%zu/%zu%s).\n", BLUE(), RESET(), BOLDCYAN(), name_.c_str(), RESET(), BOLDCYAN(), dim_with_header_ - sizeof(RealTimeIPC::DataPacket::Header), dim_with_header_, RESET());
        shared_memory_ = boost::interprocess::shared_memory_object(boost::interprocess::create_only, name_.c_str(), boost::interprocess::read_write, permissions);

        shared_memory_.truncate(dim_with_header_);
        shared_map_ = boost::interprocess::mapped_region(shared_memory_, boost::interprocess::read_write);

        std::memset(shared_map_.get_address(), 0, shared_map_.get_size());

        mutex_.reset(new  boost::interprocess::named_mutex(boost::interprocess::create_only, name_.c_str(), permissions));

        // restore old
        umask(old_umask);
      }
    }
    break;
    case SHMEM_CLIENT:
    {
      printf("[-----] %sRealTimeIPC Init%s [ %s%s%s ] Bond to Shared Memory.\n", BLUE(), RESET(), BOLDCYAN(), name_.c_str(), RESET());
      shared_memory_ = boost::interprocess::shared_memory_object(boost::interprocess::open_only, name_.c_str(), boost::interprocess::read_write);
      shared_map_    = boost::interprocess::mapped_region(shared_memory_, boost::interprocess::read_write);


      printf("[-----] %sRealTimeIPC Init%s [ %s%s%s ] Bond to Mutex\n", BLUE(), RESET(), BOLDCYAN(), name_.c_str(), RESET());
      mutex_.reset(new  boost::interprocess::named_mutex(boost::interprocess::open_only, name_.c_str()));

      dim_with_header_ = shared_map_.get_size();

      assert(dim_with_header_ > sizeof(RealTimeIPC::DataPacket::Header));

      printf("[-----] %sRealTimeIPC Init%s [ %s%s%s ] Bond to Shared Memory (bytes %s%zu/%zu%s).\n", BLUE(), RESET(), BOLDCYAN(), name_.c_str(), RESET(), BOLDCYAN(), dim_with_header_ -  sizeof(RealTimeIPC::DataPacket::Header), dim_with_header_, RESET());

      printf("[%sREADY%s] %sRealTimeIPC Init%s [ %s%s%s ] Ready.\n", BOLDGREEN(), RESET(), BLUE(), RESET(), BOLDCYAN(), name_.c_str(), RESET());
    }
    break;
    case MQUEUE_SERVER:
    {
      assert(rt_skin_ == RT_POSIX);
      printf("Not yet implemented :). Abort.\n");
      assert(0);
    }
    break;
    case MQUEUE_CLIENT:
    {
      assert(rt_skin_ == RT_POSIX);
      printf("Not yet implemented :). Abort.\n");
      assert(0);
    }
    break;
    }
  }
  catch (boost::interprocess::interprocess_exception &e)
  {
    if ((SHMEM_CLIENT == access_mode_) && (e.get_error_code() == boost::interprocess::not_found_error))
    {
      printf("[%s CHECK%s] %sRealTimeIPC Init%s [ %s%s%s ] Memory does not exist. Continue.\n", RED(), RESET(), BLUE(), RESET(), BOLDCYAN(), name_.c_str(), RESET());
      ok = true;
    }
    else
    {
      printf("[%sERROR%s] %sRealTimeIPC Init%s [ %s%s%s ] Error: %s, error code: %d. Abort.\n", RED(), RESET(), BLUE(), RESET(), BOLDCYAN(), name_.c_str(), RESET(), e.what(), e.get_error_code());
      ok = false;
    }
  }
  catch (std::exception& e)
  {
    printf("[%sERROR%s] %sRealTimeIPC Init%s [ %s%s%s ] Error: %s. Abort.\n", RED(), RESET(), BLUE(), RESET(), BOLDCYAN(), name_.c_str(), RESET(), e.what());
    ok = false;
  }

  printf("[%s%s%s] %sRealTimeIPC Init%s [ %s%s%s ] ========================.\n", ok ? BOLDGREEN() : RED(), ok ? " DONE" : "ERROR",   RESET(), BOLDBLUE(), RESET(), BOLDCYAN(), name_.c_str(), RESET());
  return ok;
}

inline
RealTimeIPC::~RealTimeIPC() noexcept(false)
{

  if (dim_with_header_ > sizeof(RealTimeIPC::DataPacket::Header))
  {
    printf("[ %s%s%s ][ %sRealTimeIPC Destructor%s ] Shared Mem Destructor\n", BOLDCYAN(), name_.c_str(), RESET(), BOLDBLUE(), RESET());

    if (isBonded())
      breakBond();

    try
    {
      switch (access_mode_)
      {
      case PIPE_SERVER:
      {
        assert(rt_skin_ == RT_ALCHEMY);
      }
      break;
      case PIPE_CLIENT:
      {
        assert(rt_skin_ == POSIX);
      }
      break;
      case SHMEM_SERVER:
      {
        assert(rt_skin_ == POSIX);
        printf("[ %s%s%s ][ %sRealTimeIPC Destructor%s ] Remove Shared Mem\n", BOLDCYAN(), name_.c_str(), RESET(), BOLDBLUE(), RESET());
        if (! boost::interprocess::shared_memory_object::remove(name_.c_str()))
        {
          printf("Error in removing the shared memory object");
        }
        printf("[ %s%s%s ][ %sRealTimeIPC Destructor%s ] Remove Mutex\n", BOLDCYAN(), name_.c_str(), RESET(), BOLDBLUE(), RESET());

        if (!boost::interprocess::named_mutex::remove(name_.c_str()))
        {
          printf("[ %s%s%s ][ %sRealTimeIPC Destructor%s ] Error\n", BOLDCYAN(), name_.c_str(), RED(), BOLDBLUE(), RESET());
        }
      }
      break;
      case SHMEM_CLIENT:
      {
        assert(rt_skin_ == POSIX);
      }
      break;
      case MQUEUE_SERVER:
      {
        assert(rt_skin_ == RT_POSIX);
        printf("Not yet implemented :). Abort.\n");
        assert(0);
      }
      break;
      case MQUEUE_CLIENT:
      {
        assert(rt_skin_ == RT_POSIX);
        printf("Not yet implemented :). Abort.\n");
        assert(0);
      }
      break;
      }

      printf("[%s DONE%s] %sRealTimeIPC Init%s [ %s%s%s ]\n", BOLDGREEN(), RESET(), BOLDBLUE(), RESET(), BOLDCYAN(), name_.c_str(), RESET());
    }
    catch (boost::interprocess::interprocess_exception &e)
    {
      if ((SHMEM_CLIENT == access_mode_) && (e.get_error_code() == boost::interprocess::not_found_error))
      {
        printf("[ %s%s%s ] Memory does not exist, Check if correct?\n", BOLDCYAN(), name_.c_str(), YELLOW());
      }
      else
      {
        printf("In processing module '%s' got the error: %s, error code: %d\n",      name_.c_str(), e.what(), e.get_error_code()) ;
      }

      printf("In processing module '%s' got the error: %s\n", name_.c_str(), e.what()) ;
    }
    catch (std::exception& e)
    {
      printf("In processing module '%s' got the error: %s\n", name_.c_str(), e.what()) ;
    }
  }
}

inline
void RealTimeIPC::getDataPacket(RealTimeIPC::DataPacket* shmem)
{
  assert(shmem);
  assert(dim_with_header_ < sizeof(RealTimeIPC::DataPacket));

  shmem->clear();
  switch (access_mode_)
  {
  case PIPE_SERVER:
  {
    assert(rt_skin_ == RT_ALCHEMY);
  }
  break;
  case PIPE_CLIENT:
  {
    assert(rt_skin_ == POSIX);
  }
  break;
  //---
  case SHMEM_SERVER:
  case SHMEM_CLIENT:
  {
    assert(rt_skin_ == POSIX);
    boost::interprocess::scoped_lock<boost::interprocess::named_mutex> lock(*mutex_);     // from local buffer to shared memory
    std::memcpy(shmem, shared_map_.get_address(), shared_map_.get_size());
    lock.unlock();
  }
  break;
  //---
  case MQUEUE_SERVER:
  case MQUEUE_CLIENT:
  {
    assert(rt_skin_ == RT_POSIX);
    printf("Not yet implemented :). Abort.\n");
    assert(0);
  }
  break;
  }
}

inline
void RealTimeIPC::setDataPacket(const DataPacket* shmem)
{
  static size_t cnt = 0;
  assert(shmem);
  assert(dim_with_header_ < sizeof(RealTimeIPC::DataPacket));

  switch (access_mode_)
  {
  case PIPE_SERVER:
  {
    assert(rt_skin_ == RT_ALCHEMY);
  }
  break;
  case PIPE_CLIENT:
  {
    assert(rt_skin_ == POSIX);
  }
  break;
  //---
  case SHMEM_SERVER:
  case SHMEM_CLIENT:
  {
    assert(rt_skin_ == POSIX);
    boost::interprocess::scoped_lock<boost::interprocess::named_mutex> lock(*mutex_);
    std::memcpy(shared_map_.get_address(), shmem, shared_map_.get_size());
    lock.unlock();
  }
  break;
  //---
  case MQUEUE_SERVER:
  case MQUEUE_CLIENT:
  {
    assert(rt_skin_ == RT_POSIX);
  }
  break;
  }
}


inline
bool RealTimeIPC::isHardRT()
{
  if (dim_with_header_ == sizeof(RealTimeIPC::DataPacket::Header))
    return false;

  DataPacket shmem;
  getDataPacket(&shmem);

  bool is_hard_rt = (shmem.header_.rt_flag_ ==  1);
  if (is_hard_rt_prev_ != is_hard_rt)
  {
    printf("[ %s%s%s ] RT State Changed from '%s%s%s' to '%s%s%s'\n", BOLDCYAN(), name_.c_str(), RESET()
           , BOLDCYAN(), (is_hard_rt_prev_ ? "HARD" : "SOFT"), RESET()
           , BOLDCYAN(), (is_hard_rt       ? "HARD" : "SOFT"), RESET()) ;
    is_hard_rt_prev_ = is_hard_rt;
  }
  return (shmem.header_.rt_flag_ ==  1);

}

inline
bool RealTimeIPC::setHardRT()
{
  if (dim_with_header_ == sizeof(RealTimeIPC::DataPacket::Header))
    return false;

  printf("[ %s%s%s ] [%sSTART%s] Set Hard RT\n", BOLDCYAN(), name_.c_str(), RESET(), BOLDCYAN(), RESET()) ;

  DataPacket shmem;
  getDataPacket(&shmem);

  if ((shmem.header_.rt_flag_ ==  1))
  {
    printf("[ %s%s%s%s ] Already hard RT!\n", BOLDCYAN(), name_.c_str(), RESET(), RED()) ;
  }

  shmem.header_.rt_flag_ = 1;
  setDataPacket(&shmem);

  printf("[ %s%s%s ] [%s DONE%s] Set Hard RT\n", BOLDCYAN(), name_.c_str(), RESET(), BOLDGREEN(), RESET()) ;
  return true;
}

inline
bool RealTimeIPC::setSoftRT()
{
  if (dim_with_header_  == sizeof(RealTimeIPC::DataPacket::Header))
    return false;

  printf("[ %s%s%s ] [%sSTART%s] Set Soft RT\n", BOLDCYAN(), name_.c_str(), RESET(), BOLDCYAN(), RESET()) ;

  DataPacket shmem;
  getDataPacket(&shmem);

  shmem.header_.rt_flag_ = 0;
  setDataPacket(&shmem);

  printf("[ %s%s%s ] [%s DONE%s] Set soft RT\n", BOLDCYAN(), name_.c_str(), RESET(), BOLDGREEN(), RESET()) ;
  return true;
}

inline
bool RealTimeIPC::isBonded()
{
  if (dim_with_header_ == sizeof(RealTimeIPC::DataPacket::Header))
    return false;

  DataPacket shmem;
  getDataPacket(&shmem);


  bool is_bonded = (shmem.header_.bond_flag_ == 1);
  if (bonded_prev_ != is_bonded)
  {
    printf("[ %s%s%s ] Bonding State Changed from '%s%s%s' to '%s%s%s'\n", BOLDCYAN(), name_.c_str(), RESET()
           , BOLDCYAN(), (bonded_prev_  ? "BONDED" : "UNBONDED"), RESET()
           , BOLDCYAN(), (is_bonded     ? "BONDED" : "UNBONDED"), RESET()) ;
    bonded_prev_ = is_bonded;
  }
  return (shmem.header_.bond_flag_ == 1);
}

inline
bool RealTimeIPC::bond()
{
  if (dim_with_header_ == sizeof(RealTimeIPC::DataPacket::Header))
    return false;

  printf("[ %s%s%s ] [%sSTART%s] Bonding\n", BOLDCYAN(), name_.c_str(), RESET(), BOLDCYAN(), RESET()) ;

  DataPacket shmem;
  getDataPacket(&shmem);

  if ((shmem.header_.bond_flag_ ==  1))
  {
    printf("[ %s%s%s%s ] Already Bonded! Abort. \n\n****** RESET CMD FOR SAFETTY **** \n", BOLDCYAN(), name_.c_str(), RESET(), RED()) ;
    return false;
  }

  shmem.header_.bond_flag_ = 1;
  setDataPacket(&shmem);

  printf("[ %s%s%s ] [%sDONE%s] Bonding\n", BOLDCYAN(), name_.c_str(), RESET(), BOLDGREEN(), RESET()) ;
  return true;
}

inline
bool RealTimeIPC::breakBond()
{
  printf("[ %s%s%s ] Break Bond\n", BOLDCYAN(), name_.c_str(), RESET()) ;
  DataPacket shmem;
  getDataPacket(&shmem);

  shmem.header_.rt_flag_ = 0;
  shmem.header_.bond_flag_ = 0;

  setDataPacket(&shmem);

  return true;

}

inline
RealTimeIPC::ErrorCode RealTimeIPC::update(const uint8_t* ibuffer, double time, const size_t& n_bytes)
{
  if ((dim_with_header_ - sizeof(RealTimeIPC::DataPacket::Header)) != n_bytes)
  {
    printf("FATAL ERROR! Shared memory map '%zu' bytes, while the input is of '%zu' bytes\n", (dim_with_header_ - sizeof(RealTimeIPC::DataPacket::Header)), n_bytes);
    return RealTimeIPC::UNMACTHED_DATA_DIMENSION;
  }

  if (dim_with_header_ - sizeof(RealTimeIPC::DataPacket::Header) <= 0)
  {
    return RealTimeIPC::NONE_ERROR;
  }

  DataPacket shmem;
  getDataPacket(&shmem);

  if (shmem.header_.bond_flag_ == 1)
  {
    shmem.header_.time_ = time;
    std::memcpy(shmem.buffer, ibuffer, (dim_with_header_ - sizeof(RealTimeIPC::DataPacket::Header)));
  }

  setDataPacket(&shmem);

  return RealTimeIPC::NONE_ERROR;
}

inline
RealTimeIPC::ErrorCode RealTimeIPC::flush(uint8_t* obuffer, double* time, double* latency_time, const size_t& n_bytes)
{
  size_t scrn_cnt = 0;
  RealTimeIPC::ErrorCode ret = RealTimeIPC::NONE_ERROR;
  if ((dim_with_header_ - sizeof(RealTimeIPC::DataPacket::Header)) != n_bytes)
  {
    printf("FATAL ERROR! Wrong Memory Dimensions.\n");
    return RealTimeIPC::UNMACTHED_DATA_DIMENSION;
  }

  if ((dim_with_header_ - sizeof(RealTimeIPC::DataPacket::Header)) <= 0)
  {
    return RealTimeIPC::NONE_ERROR;
  }

  DataPacket shmem;
  getDataPacket(&shmem);

  if (shmem.header_.bond_flag_ == 1)
  {
    *time = shmem.header_.time_;
    std::memcpy(obuffer, shmem.buffer, (dim_with_header_ - sizeof(RealTimeIPC::DataPacket::Header)));
  }
  else
  {
    // printf_THROTTLE( 2, "[ %s%s%s%s ] SAFETTY CMD (not bonded)", BOLDCYAN(), name_.c_str(), RESET(), RED()) ;
    *time = 0.0;
    std::memset(obuffer, 0x0, (dim_with_header_ - sizeof(RealTimeIPC::DataPacket::Header)));
  }


  // check
  *latency_time = (*time - data_time_prev_);

  struct timespec flush_ts;
  clock_gettime(CLOCK_MONOTONIC, &flush_ts);
  double flush_time = realtime_utilities::timer_to_s(&flush_ts);
  if (RealTimeIPC::isClient(access_mode_) && (std::fabs(flush_time - *time) > 2 * watchdog_))
  {
    printf("Data not updated! (%f,%f,%f)!\n", flush_time, *time, watchdog_);
    return RealTimeIPC::WATCHDOG;
  }

  if (shmem.header_.bond_flag_ == 1)
  {
    /////////////////////////////////////////////////
    if ((*latency_time < watchdog_)  && (*latency_time > 1e-5))      // the client cycle time is in the acceptable trange watchdog
    {
      start_watchdog_time_ = -1;
    }
    else if (*latency_time > watchdog_)
    {
      printf("Latency overcome the watchdog (%f,%f,%f)!\n", *latency_time, *time, data_time_prev_);
      ret = RealTimeIPC::WATCHDOG;
    }
    else if (*latency_time < 1e-5)   // the client is not writing new data in the shared memory ...
    {
      if (start_watchdog_time_ == -1)
      {
        start_watchdog_time_ = flush_time;
      }
      else if ((flush_time - start_watchdog_time_) > watchdog_)
      {
        ret = RealTimeIPC::WATCHDOG;
      }
    }
    /////////////////////////////////////////////////

    /////////////////////////////////////////////////
    if (ret == RealTimeIPC::WATCHDOG)
    {
      if (shmem.header_.rt_flag_ == 1)
      {
        if (scrn_cnt++ % 1000)
          printf("[ %s%s%s%s ] Watchdog %fms (allowed: %f) ****** RESET CMD FOR SAFETTY ****\n", BOLDCYAN(), name_.c_str(), RESET(), RED(), (flush_time - start_watchdog_time_), watchdog_) ;
        if (RealTimeIPC::isServer(access_mode_))
        {
          std::memset(obuffer, 0x0, (dim_with_header_ - sizeof(RealTimeIPC::DataPacket::Header)));
        }
      }
      else
      {
        if (scrn_cnt++ % 1000)
          printf("[ %s%s%s%s ] Watchdog %fms (allowed: %f) ****** SOFT RT, DON'T CARE ****\n", BOLDCYAN(), name_.c_str(), RESET(), YELLOW(), (flush_time - start_watchdog_time_), watchdog_) ;
        ret = RealTimeIPC::NONE_ERROR;
      }
    }
    else
    {
      scrn_cnt = 0;
    }
    /////////////////////////////////////////////////
  }
  data_time_prev_ = *time;
  flush_time_prev_ = flush_time;

  return ret;
}

inline
std::string RealTimeIPC::to_string(RealTimeIPC::ErrorCode err)
{
  std::string ret = "na";
  switch (err)
  {
  case NONE_ERROR               :
    ret = "SHARED MEMORY NONE ERROR";
    break;
  case UNMACTHED_DATA_DIMENSION :
    ret = "SHARED MEMORY UNMATHCED DATA DIMENSION";
    break;
  case UNCORRECT_CALL           :
    ret = "SHARED MEMORY UNCORRECT CALL SEQUENCE";
    break;
  case WATCHDOG                 :
    ret = "SHARED MEMORY WATCHDOG";
    break;
  }
  return ret;
}

inline
size_t RealTimeIPC::getSize(bool prepend_header) const
{
  return prepend_header ? dim_with_header_ : (dim_with_header_ - sizeof(RealTimeIPC::DataPacket::Header));
}

inline
double RealTimeIPC::getWatchdog() const
{
  return watchdog_;
}

}
