#ifndef REALTIME_UTILITIES__OS_H
#define REALTIME_UTILITIES__OS_H

#if !defined(_WIN32) && !defined(_WIN64)
#include <sys/mman.h>  // Needed for mlockall()
#include <unistd.h>    // needed for sysconf(int name);
#include <sys/resource.h>  // needed for getrusage
#include <pthread.h>  
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/if_link.h>
#else
#include <windows.h>
#include <sysinfoapi.h>
#include <timezoneapi.h>
#include <profileapi.h>
#include <sys/timeb.h>
#include <sys/types.h>
#include <winsock2.h>
#include <io.h>


int gettimeofday(struct timeval* t, void* timezone);

// from linux's sys/times.h
//#include <features.h>

#define __need_clock_t
#include <time.h>


/* Structure describing CPU time used by a process and its children.  */
struct tms
{
    clock_t tms_utime;          /* User CPU time.  */
    clock_t tms_stime;          /* System CPU time.  */

    clock_t tms_cutime;         /* User CPU time of dead children.  */
    clock_t tms_cstime;         /* System CPU time of dead children.  */
};

/* Store the CPU time used by this process and all its
   dead children (and their dead children) in BUFFER.
   Return the elapsed real time, or (clock_t) -1 for errors.
   All times are in CLK_TCKths of a second.  */
clock_t times(struct tms* __buffer);

typedef long long suseconds_t;

LARGE_INTEGER
getFILETIMEoffset()
{
    SYSTEMTIME s;
    FILETIME f;
    LARGE_INTEGER t;

    s.wYear = 1970;
    s.wMonth = 1;
    s.wDay = 1;
    s.wHour = 0;
    s.wMinute = 0;
    s.wSecond = 0;
    s.wMilliseconds = 0;
    SystemTimeToFileTime(&s, &f);
    t.QuadPart = f.dwHighDateTime;
    t.QuadPart <<= 32;
    t.QuadPart |= f.dwLowDateTime;
    return (t);
}

int
clock_gettime(int X, struct timespec* tv)
{
    LARGE_INTEGER           t;
    FILETIME                f;
    double                  microseconds;
    static LARGE_INTEGER    offset;
    static double           frequencyToMicroseconds;
    static int              initialized = 0;
    static BOOL             usePerformanceCounter = 0;

    if (!initialized) {
        LARGE_INTEGER performanceFrequency;
        initialized = 1;
        usePerformanceCounter = QueryPerformanceFrequency(&performanceFrequency);
        if (usePerformanceCounter) {
            QueryPerformanceCounter(&offset);
            frequencyToMicroseconds = (double)performanceFrequency.QuadPart / 1000000.;
        }
        else {
            offset = getFILETIMEoffset();
            frequencyToMicroseconds = 10.;
        }
    }
    if (usePerformanceCounter) QueryPerformanceCounter(&t);
    else {
        GetSystemTimeAsFileTime(&f);
        t.QuadPart = f.dwHighDateTime;
        t.QuadPart <<= 32;
        t.QuadPart |= f.dwLowDateTime;
    }

    t.QuadPart -= offset.QuadPart;
    microseconds = (double)t.QuadPart / frequencyToMicroseconds;
    t.QuadPart = microseconds;
    tv->tv_sec = t.QuadPart / 1000000;
    tv->tv_nsec = t.QuadPart % 1000000000;
    return (0);
}

#define CLOCK_MONOTONIC 0

typedef int mode_t;

/// @Note If STRICT_UGO_PERMISSIONS is not defined, then setting Read for any
///       of User, Group, or Other will set Read for User and setting Write
///       will set Write for User.  Otherwise, Read and Write for Group and
///       Other are ignored.
///
/// @Note For the POSIX modes that do not have a Windows equivalent, the modes
///       defined here use the POSIX values left shifted 16 bits.

static const mode_t S_ISUID = 0x08000000;           ///< does nothing
static const mode_t S_ISGID = 0x04000000;           ///< does nothing
static const mode_t S_ISVTX = 0x02000000;           ///< does nothing
static const mode_t S_IRUSR = mode_t(_S_IREAD);     ///< read by user
static const mode_t S_IWUSR = mode_t(_S_IWRITE);    ///< write by user
static const mode_t S_IXUSR = 0x00400000;           ///< does nothing
#   ifndef STRICT_UGO_PERMISSIONS
static const mode_t S_IRGRP = mode_t(_S_IREAD);     ///< read by *USER*
static const mode_t S_IWGRP = mode_t(_S_IWRITE);    ///< write by *USER*
static const mode_t S_IXGRP = 0x00080000;           ///< does nothing
static const mode_t S_IROTH = mode_t(_S_IREAD);     ///< read by *USER*
static const mode_t S_IWOTH = mode_t(_S_IWRITE);    ///< write by *USER*
static const mode_t S_IXOTH = 0x00010000;           ///< does nothing
#   else
static const mode_t S_IRGRP = 0x00200000;           ///< does nothing
static const mode_t S_IWGRP = 0x00100000;           ///< does nothing
static const mode_t S_IXGRP = 0x00080000;           ///< does nothing
static const mode_t S_IROTH = 0x00040000;           ///< does nothing
static const mode_t S_IWOTH = 0x00020000;           ///< does nothing
static const mode_t S_IXOTH = 0x00010000;           ///< does nothing
#   endif
static const mode_t MS_MODE_MASK = 0x0000ffff;           ///< low word
#endif

#endif