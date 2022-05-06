#ifndef REALTIME_UTILITIES__REALTIME_IPC_H
#define REALTIME_UTILITIES__REALTIME_IPC_H


#include <boost/algorithm/string.hpp>
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/containers/vector.hpp>
#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/interprocess/sync/named_mutex.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <boost/interprocess/sync/sharable_lock.hpp>
#include <boost/interprocess/sync/upgradable_lock.hpp>

#include <tuple>
#include <realtime_utilities/realtime_utilities.h>


namespace realtime_utilities
{

/**
 * @class RealTimeIPC
 *
 */
#if !defined( MAX_IPC_BUF_LENGHT)
#define MAX_IPC_BUF_LENGHT 1024
#endif

class RealTimeIPC
{
public:
  typedef std::shared_ptr< RealTimeIPC >  Ptr;

  enum Skin       { POSIX, RT_POSIX, RT_ALCHEMY };
  enum AccessMode { PIPE_SERVER, SHMEM_SERVER, MQUEUE_SERVER, PIPE_CLIENT, SHMEM_CLIENT, MQUEUE_CLIENT };
  enum ErrorCode  { NONE_ERROR, UNMACTHED_DATA_DIMENSION, UNCORRECT_CALL, WATCHDOG };

  static bool isClient(const RealTimeIPC::AccessMode& mode)
  {
    return (mode == PIPE_CLIENT) || (mode == SHMEM_CLIENT) || (mode == MQUEUE_CLIENT);
  }
  static bool isServer(const RealTimeIPC::AccessMode& mode)
  {
    return (mode == PIPE_SERVER) || (mode == SHMEM_SERVER) || (mode == MQUEUE_SERVER);
  }

  struct DataPacket
  {
    struct Header
    {
      uint8_t bond_flag_;
      uint8_t rt_flag_;
      double  time_;
    } __attribute__((packed)) header_;

    char    buffer[MAX_IPC_BUF_LENGHT];
    DataPacket()
    {
      clear();
    }
    void clear()
    {
      header_.bond_flag_ = header_.rt_flag_ = header_.time_ = 0;
      std::memset(&buffer[0], 0x0, MAX_IPC_BUF_LENGHT * sizeof(char));
    }
  };


  RealTimeIPC(const std::string& identifier, double operational_time, double watchdog_decimation, const AccessMode& mode, const size_t dim = 0) noexcept(false);
  ~RealTimeIPC() noexcept(false);

  bool        isHardRT();
  bool        setHardRT();
  bool        setSoftRT();

  bool        isBonded();
  bool        bond();
  bool        breakBond();
  ErrorCode   update(const uint8_t* buffer, const double time, const size_t& n_bytes);
  ErrorCode   flush(uint8_t* buffer, double* time, double* latency_time, const size_t& n_bytes);

  size_t      getSize(bool prepend_header) const;
  std::string getName()                      const;
  double      getWatchdog()                 const;
  std::string to_string(ErrorCode err);

  void        dump(RealTimeIPC::DataPacket* buffer)
  {
    return getDataPacket(buffer);
  }

protected:

  bool init();

  const AccessMode                                  access_mode_;
  Skin                                              rt_skin_;
  const double                                      operational_time_;
  const double                                      watchdog_;

  const std::string                                 name_;
  size_t                                            dim_with_header_;
  int                                               rt_pipe_fd_;
  boost::interprocess::mapped_region                shared_map_;
  boost::interprocess::shared_memory_object         shared_memory_;
  std::shared_ptr<boost::interprocess::named_mutex> mutex_;

  double                                            start_watchdog_time_;
  double                                            data_time_prev_;
  double                                            flush_time_prev_;

  size_t                                            bond_cnt_;
  bool                                              bonded_prev_;
  bool                                              is_hard_rt_prev_;

  void getDataPacket(DataPacket* packet);
  void setDataPacket(const DataPacket* packet);
};


}  // namespace realtime_utilities

#include <realtime_utilities/internal/realtime_ipc_impl.h>


#endif  // REALTIME_UTILITIES__REALTIME_IPC_H
