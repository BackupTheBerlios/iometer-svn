/* ######################################################################### */
/* ##                                                                     ## */
/* ##  (Iometer & Dynamo) / IOCommon.h                                    ## */
/* ##                                                                     ## */
/* ## ------------------------------------------------------------------- ## */
/* ##                                                                     ## */
/* ##  Job .......: This is THE central headerfile for Iometer as well    ## */
/* ##               as Dynamo.                                            ## */
/* ##                                                                     ## */
/* ## ------------------------------------------------------------------- ## */
/* ##                                                                     ## */
/* ##  Intel Open Source License                                          ## */
/* ##                                                                     ## */
/* ##  Copyright (c) 2001 Intel Corporation                               ## */
/* ##  All rights reserved.                                               ## */
/* ##  Redistribution and use in source and binary forms, with or         ## */
/* ##  without modification, are permitted provided that the following    ## */
/* ##  conditions are met:                                                ## */
/* ##                                                                     ## */
/* ##  Redistributions of source code must retain the above copyright     ## */
/* ##  notice, this list of conditions and the following disclaimer.      ## */
/* ##                                                                     ## */
/* ##  Redistributions in binary form must reproduce the above copyright  ## */
/* ##  notice, this list of conditions and the following disclaimer in    ## */
/* ##  the documentation and/or other materials provided with the         ## */
/* ##  distribution.                                                      ## */
/* ##                                                                     ## */
/* ##  Neither the name of the Intel Corporation nor the names of its     ## */
/* ##  contributors may be used to endorse or promote products derived    ## */
/* ##  from this software without specific prior written permission.      ## */
/* ##                                                                     ## */
/* ##  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND             ## */
/* ##  CONTRIBUTORS ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,      ## */
/* ##  INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF           ## */
/* ##  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE           ## */
/* ##  DISCLAIMED. IN NO EVENT SHALL THE INTEL OR ITS  CONTRIBUTORS BE    ## */
/* ##  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,   ## */
/* ##  OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,           ## */
/* ##  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,    ## */
/* ##  OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY    ## */
/* ##  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR     ## */
/* ##  TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT    ## */
/* ##  OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY    ## */
/* ##  OF SUCH DAMAGE.                                                    ## */
/* ##                                                                     ## */
/* ## ------------------------------------------------------------------- ## */
/* ##                                                                     ## */
/* ##  Remarks ...: - All Defines, Includes etc. should be moved to       ## */
/* ##                 this file to cleanup the code.                      ## */
/* ##               - Functions like Strip() should be moved to a new     ## */
/* ##                 code file named IOCommon.cpp                        ## */
/* ##                                                                     ## */
/* ## ------------------------------------------------------------------- ## */
/* ##                                                                     ## */
/* ##  Changes ...: 2003-07-13 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Massive cleanup of this file (grouping the          ## */
/* ##                 different blocks together).                         ## */
/* ##               - Implemented the IOMTR_[OSFAMILY|OS|CPU]_* global    ## */
/* ##                 define as well as their integrity checks.           ## */
/* ##               - Integrated the License Statement into this header.  ## */
/* ##               2003-03-28 (joe@eiler.net)                            ## */
/* ##               - changes so VC++ 7 (.NET) will compile correctly.    ## */
/* ##               2003-03-04 (joe@eiler.net)                            ## */
/* ##               - Made a change for Solaris to work, I had to extern  ## */
/* ##                 processor_speed_to_nsecs, I don't know how this     ## */
/* ##                 would have ever compiled, let alone worked!!        ## */
/* ##               2003-03-02 (joe@eiler.net)                            ## */
/* ##               - Changed LONG_PTR to ULONG_PTR, which is what it is  ## */
/* ##               2003-03-02 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Added the implementation of the Strip() function    ## */
/* ##                 which removes leading and trailing blanks.          ## */
/* ##               2003-02-26 (joe@eiler.net)                            ## */
/* ##               - Added MAX_EXCLUDE_FILESYS so excluded filesystem    ## */
/* ##                 types are no longer hard coded.                     ## */
/* ##               2003-02-15 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Different changes to support compilation with       ## */
/* ##                 gcc 3.2 (known as cout << hex error).               ## */
/* ##               2003-02-08 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Added ULONG_PTR and DWORD_PTR typedef to the        ## */
/* ##                 Windows part to get compiled.                       ## */
/* ##               2003-02-04 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Applied proc-speed-fix.txt patch file               ## */
/* ##                 (changes the data type of processor_speed).         ## */
/* ##               2003-02-02 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Added new header holding the changelog.             ## */
/* ##               - Increase of MAX_TARGETS from 256 to 512. This was   ## */
/* ##                 needed for a successfull login of a Dynamo instance ## */
/* ##                 on a Iometer frontend (Version 2001.07.19).         ## */
/* ##                                                                     ## */
/* ######################################################################### */
#ifndef	___IOCOMMON_H_DEFINED___
#define ___IOCOMMON_H_DEFINED___



// Check and map the different global defines
// for Operating System family, Operating System,
// CPU and endian type (see README).
// ----------------------------------------------------------------------------
// Check Operating System family mapping
#if ( defined(IOMTR_OSFAMILY_UNIX) && !defined(IOMTR_OSFAMILY_WINDOWS)) || \
    (!defined(IOMTR_OSFAMILY_UNIX) &&  defined(IOMTR_OSFAMILY_WINDOWS))
 // nop
#else    
 #error ===> ERROR: Check the Operating System to Operating System family mapping!
#endif
// ----------------------------------------------------------------------------
// Check the Operating System mapping
#if ( defined(IOMTR_OS_LINUX) && !defined(IOMTR_OS_SOLARIS) && !defined(IOMTR_OS_WIN32) && !defined(IOMTR_OS_WIN64)) || \
    (!defined(IOMTR_OS_LINUX) &&  defined(IOMTR_OS_SOLARIS) && !defined(IOMTR_OS_WIN32) && !defined(IOMTR_OS_WIN64)) || \
    (!defined(IOMTR_OS_LINUX) && !defined(IOMTR_OS_SOLARIS) &&  defined(IOMTR_OS_WIN32) && !defined(IOMTR_OS_WIN64)) || \
    (!defined(IOMTR_OS_LINUX) && !defined(IOMTR_OS_SOLARIS) && !defined(IOMTR_OS_WIN32) &&  defined(IOMTR_OS_WIN64))
 // nop
#else    
 #error ===> ERROR: You have to define exactly one IOMTR_OS_* global define!
#endif
// ----------------------------------------------------------------------------
// Check the Processor mapping
#if ( defined(IOMTR_CPU_I386) && !defined(IOMTR_CPU_IA64) && !defined(IOMTR_CPU_SPARC)) || \
    (!defined(IOMTR_CPU_I386) &&  defined(IOMTR_CPU_IA64) && !defined(IOMTR_CPU_SPARC)) || \
    (!defined(IOMTR_CPU_I386) && !defined(IOMTR_CPU_IA64) &&  defined(IOMTR_CPU_SPARC))
 // nop
#else    
 #error ===> ERROR: You have to define exactly one IOMTR_CPU_* global define!
#endif
// ----------------------------------------------------------------------------
// Check Endian type mapping
#if ( defined(LITTLE_ENDIAN_ARCH) && !defined(BIG_ENDIAN_ARCH)) || \
    (!defined(LITTLE_ENDIAN_ARCH) &&  defined(BIG_ENDIAN_ARCH))
 // nop
#else
 #error ===> ERROR: Check the Processor to Endian type mapping!
#endif
// ----------------------------------------------------------------------------



// Include the different header files
// (both, OS family based and common)
// ----------------------------------------------------------------------------
#if defined(IOMTR_OSFAMILY_WINDOWS)
 #define VC_EXTRALEAN
 #include <process.h>
 #include <io.h>
 #include <direct.h>
 #include <afxwin.h>
 #include <afxext.h>
 #include <afxcmn.h>
 #include <winioctl.h>
 #include <iomanip>
 #include <winperf.h>
 #include <winreg.h>
 #include <afxmt.h>
#endif
// ----------------------------------------------------------------------------
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <time.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/timeb.h>
#include <math.h>
// ----------------------------------------------------------------------------
#include <iostream>
#include <fstream>
using namespace std;
// ----------------------------------------------------------------------------
#if defined(IOMTR_OSFAMILY_UNIX)
 #include <unistd.h>
 #include <pthread.h>
 #include <signal.h>
 #include <aio.h>
 #include <netinet/in.h>   // in_addr_t
#endif
// ----------------------------------------------------------------------------
#if defined(IOMTR_OSFAMILY_WINDOWS)
 #if _MSC_VER < 1300
 #include "ostream64.h"
 #endif
#endif 
// ----------------------------------------------------------------------------
#include "IOVersion.h"   // version info definitions
// ----------------------------------------------------------------------------



// Define the different data types
// (both, OS family based and common)
// ----------------------------------------------------------------------------
#if defined(IOMTR_OSFAMILY_UNIX)
 #define __int64   long long
 #define __int32   long
 #define __int16   short
 #define __int8    char

 typedef long long	       _int64;
 //typedef long long	       LARGE_INTEGER; 
 typedef unsigned long long    DWORDLONG;
 typedef long		       LONG;
 typedef unsigned long	       DWORD;

 typedef int		       INT;
 typedef int		       BOOL;
 typedef int		       BOOLEAN;
 typedef unsigned int	       UINT;
 typedef unsigned int	      *PUINT;

 typedef unsigned short	       WCHAR;
 typedef unsigned short	      *LPWSTR;

 typedef char		      *LPSTR;
 typedef unsigned char	       BYTE;
 typedef const char	      *LPCSTR;

 typedef void		      *HANDLE;
 typedef void		      *LPVOID;
 typedef void		      *SOCKET;
 typedef const void	      *LPCVOID;

 typedef DWORD		      *LPDWORD;
 typedef BYTE		      *LPBYTE;
 typedef LPCSTR		       LPCTSTR;
#endif 
// ----------------------------------------------------------------------------
#if defined(IOMTR_OSFAMILY_WINDOWS)
 typedef unsigned long   ULONG_PTR;
 typedef ULONG_PTR       DWORD_PTR;
#endif 
// ----------------------------------------------------------------------------



// Definition of the defines itself
// (both, OS family based and common)
// ----------------------------------------------------------------------------
// Different return values for the Iometer and Dynamo function calls.
#define IOERROR	-1   // keep negative

#define RETRY_DELAY   200    // msec. to wait before reattempting an action
#define LONG_DELAY    3000   // msec. to wait before reattempting an action
#define IDLE_DELAY    1000   // max msec to wait between calls to CGalileoApp::OnIdle()

#define KILOBYTE   1024
#define MEGABYTE   1048576

#define MAX_NAME	      64
#define	MAX_CPUS	      32
#define	MAX_WORKERS	      100
#define MAX_TARGETS	      512
#define MAX_NETWORK_NAME      128
#define MAX_NUM_INTERFACES    32
#define MAX_WORKER_NAME	      128
#define MAX_EXCLUDE_FILESYS   128

#define ALL_MANAGERS	      -16
#define MANAGER		      -8
#define ALL_WORKERS	      -4
#define WORKER		      -2

// Definitions for CPU results.
#define	CPU_RESULTS		     6
#define CPU_UTILIZATION_RESULTS	     5
// All CPU utilization results should be listed together.
#define CPU_TOTAL_UTILIZATION	     0
#define CPU_USER_UTILIZATION	     1
#define CPU_PRIVILEGED_UTILIZATION   2
#define CPU_DPC_UTILIZATION	     3
#define CPU_IRQ_UTILIZATION   	     4
// Other CPU counters.
#define CPU_IRQ			     5

// Definitions for network tcp results
#define TCP_RESULTS           1   // total number of network results
// List all specific performance results that are desired.
#define TCP_SEGMENTS_RESENT   0

// Definitions for network interface results
#define NI_RESULTS	     3   // total number of network interface results reported
#define NI_COMBINE_RESULTS   2   // combine some reported results together
// List all specific performance results that are desired.
#define NI_PACKETS	     0
#define NI_OUT_ERRORS	     1
#define NI_IN_ERRORS	     2
#define NI_ERRORS	     1   // combine in and out errors together when saving

// Initialization parameters for CArray and related classes.
#define INITIAL_ARRAY_SIZE   0    // Sets the size of the array.
#define ARRAY_GROW_STEP	     10   // Sets the amount of memory to allocate when the array is grown.

// To record performance statistics, two snapshots of specific counters are needed.
#define FIRST_SNAPSHOT	 0
#define LAST_SNAPSHOT	 1
#define MAX_SNAPSHOTS	 2

// We keep a Performance object to record results for the whole test and another to record
// only the results since the last update.
#define WHOLE_TEST_PERF	   0
#define LAST_UPDATE_PERF   1
#define MAX_PERF	   2
// ----------------------------------------------------------------------------
#if defined(IOMTR_OSFAMILY_UNIX)
 //param does not contain a valid internet address
 // (For Win32 || _WIN64, INADDR_NONE is defined as 0xffffffff)
 #ifndef INADDR_NONE
  #define INADDR_NONE   (in_addr_t)-1
 #endif

 #define FALSE			0
 #define TRUE			1
 #define INVALID_HANDLE_VALUE   (HANDLE)-1
 #define INVALID_SOCKET		(SOCKET)(~0)
 #define PERF_NO_INSTANCES	-1
 #define SOCKET_ERROR		-1
 #define SD_BOTH		2
 #define INFINITE		(unsigned)~0
 #define ERROR_IO_PENDING	501
 #define WAIT_TIMEOUT		ERROR_IO_PENDING
 #define ERROR_IO_INCOMPLETE	ERROR_IO_PENDING

 #define FILE_ELEMENT		1
 #define CQ_ELEMENT		0
#endif
// ----------------------------------------------------------------------------
#if defined(BIG_ENDIAN_ARCH)
 #define SEND   1
 #define RECV   2
#endif
// ----------------------------------------------------------------------------



// Define the enumerations, structures etc.
// (both, OS family based and common)
// ----------------------------------------------------------------------------
enum ReturnVal {
	ReturnError   = IOERROR,   // catch all unexpected errors
	ReturnSuccess = 0,	   // Normal completion
	ReturnTimeout,		   // Operation timed out
	ReturnPending,		   // Request has been queued
	ReturnAbort,		   // Operation aborted
	ReturnRetry	   	   // Operation was interrupted and can be retried
};
// Testing states that Iometer and worker threads go through
enum TestState {
	TestIdle,	  // before Start_Test() and after Stop_Test()
	TestPreparing,
	TestOpening,	  // after Start_Test()
	TestRampingUp,	  // after Begin_IO()
	TestRecording,	  // after Record_On()
	TestRampingDown	  // after Record_Off()
};
// Specified on the Test Setup tab: which test results to record.
// (In IOCommon.h because Dynamo uses these as parameters.)
enum {
	RecordAll = 0,
	RecordNoTargets,
	RecordNoWorkers,
	RecordNoManagers,
	RecordNone
};
// ----------------------------------------------------------------------------
struct Manager_Info
{
	char	       version[MAX_VERSION_LENGTH];
	char	       names[2][MAX_NETWORK_NAME];
	unsigned short port_number;   // used only with TCP/IP.
	int	       processors;
	double	       processor_speed;
};
// Basic result information stored by worker threads.
struct Raw_Result
{
	DWORDLONG    bytes_read;   	       // Number of bytes transferred.
	DWORDLONG    bytes_written;
	DWORDLONG    read_count;	       // Number of I/Os completed.
	DWORDLONG    write_count;
	DWORDLONG    transaction_count;	       // Number of transactions completed.
	DWORDLONG    connection_count;
	unsigned int read_errors;	       // Number of errors seen.
	unsigned int write_errors;
	DWORDLONG    max_raw_read_latency;     // Application latencies for an I/O to complete, recorded
	DWORDLONG    read_latency_sum;	       // using the processor counter clock.
	DWORDLONG    max_raw_write_latency;
	DWORDLONG    write_latency_sum;
	DWORDLONG    max_raw_transaction_latency;
	DWORDLONG    max_raw_connection_latency;
	DWORDLONG    transaction_latency_sum;
	DWORDLONG    connection_latency_sum;   // Application latencies for a Connection.
	__int64	     counter_time;	       // Difference between ending and starting counter time stamps.
};
// Storing results for all targets in a single structure.
struct Target_Results
{
	int	   count;    // Number of targets.
	char	   pad[4];   // padding
	Raw_Result result[MAX_TARGETS];
};
// Storing results for a worker.  This includes the worker's target results.
struct Worker_Results
{
	DWORDLONG      time[MAX_SNAPSHOTS];   // Processor based counters to provide time stamps.
	Target_Results target_results;
};
// All CPU related results are stored in a single structure.
struct CPU_Results
{
	int    count;	 // Number of processors.
	char   pad[4];   // padding
	double CPU_utilization[MAX_CPUS][CPU_RESULTS];
};
// All network related results are stored in a single structure.
struct Net_Results
{
	double tcp_stats[TCP_RESULTS];
	int    ni_count;   // Number of NICs.
	char   pad[4];     // padding
	double ni_stats[MAX_NUM_INTERFACES][NI_RESULTS];
};
// Results SPECIFIC to a single system.  This is NOT compiled system results.
struct Manager_Results
{
	__int64	    time_counter[MAX_SNAPSHOTS];
	CPU_Results cpu_results;
	Net_Results net_results;
};
// Result structure used by the manager list, managers, and workers to store
// results that will be saved.
struct Results
{
	Raw_Result   raw;
	double	     IOps;
	double	     read_IOps;
	double	     write_IOps;
	double	     MBps;
	double	     read_MBps;
	double	     write_MBps;
	double	     ave_latency;
	double	     max_latency;
	double	     ave_read_latency;
	double	     max_read_latency;
	double	     ave_write_latency;
	double	     max_write_latency;
	double	     transactions_per_second;
	double	     ave_transaction_latency;
	double	     max_transaction_latency;
	unsigned int total_errors;
	double	     CPU_utilization[CPU_RESULTS];
	double	     CPU_effectiveness;
	double	     individual_CPU_utilization[MAX_CPUS][CPU_RESULTS];
	double	     tcp_statistics[TCP_RESULTS];
	double	     ni_statistics[NI_COMBINE_RESULTS];
	double	     connections_per_second;
	double	     ave_connection_latency;
	double	     max_connection_latency;
};
// ----------------------------------------------------------------------------
#if defined(IOMTR_OSFAMILY_UNIX)
 // This LPOVERLAPPED typedef is from WINBASE.H
 typedef struct _OVERLAPPED {   
	DWORD  Internal;
       	DWORD  InternalHigh;
	DWORD  Offset;
	DWORD  OffsetHigh;
	HANDLE hEvent;
 } OVERLAPPED, *LPOVERLAPPED;
 typedef struct _LARGE_INTEGER {
	long LowPart;
	long HighPart;
 } LARGE_INTEGER;
 struct CQ_Element {
	struct  aiocb64 aiocbp;
	void   *data;
	int	done;
	int	error;
	int	completion_key;
	int	bytes_transferred;
 };
 struct IOCQ {
	CQ_Element      *element_list;
	struct aiocb64 **aiocb_list;
	int	         size;
	int	         last_freed;
	int	         position;
 };
 struct File {
	int   fd;
	int   completion_key;
	IOCQ *iocq;
 };
#endif 
// ----------------------------------------------------------------------------



// Define the global variables
// (both, OS family based and common)
// ----------------------------------------------------------------------------
const char NEW_WORKER_COMMAND[]    = "start /MIN ";
const char NEW_WORKER_EXECUTABLE[] = "dynamo";
// ----------------------------------------------------------------------------
#if defined(IOMTR_OSFAMILY_UNIX)
 extern pthread_mutex_t lock_mt;   // we use one global locking mutex
#endif 
// ----------------------------------------------------------------------------
#if defined(IOMTR_CPU_SPARC)
 extern double processor_speed_to_nsecs;
#endif
// ----------------------------------------------------------------------------



// Define the different function prototypes and macros
// for global functions - including the Strip() function
// (both, OS family based and common)
// ----------------------------------------------------------------------------
extern void GetAppFileVersionString( char **ppStrStandard, char **ppStrWithDebug );
extern inline char *Strip( char *pcString )
{
 unsigned int uiLength = strlen(pcString);
 int          I;
 int          iIndex = 0;
 int          iPos1, iPos2; 
 
 for( iPos1 = 0;            ( (iPos1<(int)uiLength) && (pcString[iPos1]==(int)' ') ); iPos1++ ) { /* NOP */ }
 for( iPos2 = uiLength - 1; ( (iPos2>=0)            && (pcString[iPos2]==(int)' ') ); iPos2-- ) { /* NOP */ }
 
 if( (iPos1 != 0) || (iPos2 != ((int)uiLength - 1)) )
 {    
  for( I = iPos1; I <= iPos2; I++ )
  {
   pcString[iIndex] = pcString[I];
   iIndex++;
  }
  pcString[iIndex] = '\0';
 }
 
 return(pcString);
}
// ----------------------------------------------------------------------------
#if defined(IOMTR_OSFAMILY_UNIX)
 BOOL    SetQueueSize(HANDLE, int);
 HANDLE  CreateIoCompletionPort(HANDLE, HANDLE, DWORD, DWORD);
 BOOL    GetQueuedCompletionStatus(HANDLE, LPDWORD, LPDWORD, LPOVERLAPPED *, DWORD);

 BOOL    ReadFile(HANDLE, LPVOID, DWORD, LPDWORD, LPOVERLAPPED);
 BOOL    WriteFile(HANDLE, LPVOID, DWORD, LPDWORD, LPOVERLAPPED);
 HANDLE  CreateEvent(void *, BOOL, BOOL, LPCSTR);
 BOOL    GetOverlappedResult( HANDLE, LPOVERLAPPED, LPDWORD, BOOL );

 void    SetLastError(DWORD);
 DWORD   GetLastError(void);
 int     WSAGetLastError(void);
 void    WSASetLastError(DWORD);

 BOOL    CloseHandle(HANDLE, int object_type = 0);
 char   *_itoa(int, char *, int);
 DWORD   WaitForSingleObject(HANDLE h, DWORD milliSecs);

 #ifndef NOMINMAX
  #ifndef max
   #define max(a,b)             (((a) > (b)) ? (a) : (b))
  #endif
  #ifndef min
   #define min(a,b)             (((a) < (b)) ? (a) : (b))
  #endif
 #endif 
 #ifndef __max
  #define __max(a,b)            (((a) < (b)) ? (b) : (a))
 #endif

 #define _timeb		        timeb
 #define _ftime			ftime
 #define Sleep(x) 		usleep((x) * 1000)

 #define LOCK					    \
	 if (pthread_mutex_lock(&lock_mt))          \
	 {					    \
		cout << "unable to lock" << endl;   \
		exit(1);			    \
	 }
 #define UNLOCK					      \
	 if (pthread_mutex_unlock(&lock_mt))	      \
	 {					      \
		cout << "unable to unlock" << endl;   \
		exit(1);			      \
	 }

 #define InterlockedExchange(a,b)   \
	 LOCK;			    \
	 *(a) = (b);		    \
	 UNLOCK;
 #define InterlockedDecrement(a)   \
	 LOCK;			   \
	 --*(a);		   \
	 UNLOCK;
 #define InterlockedIncrement(a)   \
	 LOCK;			   \
	 ++*(a);		   \
	 UNLOCK;
#endif 
// ----------------------------------------------------------------------------
#if defined(BIG_ENDIAN_ARCH)
 inline void rotate(char *ptr, int size);
 template <class T> inline void reorder(T&);
 inline void reorder(CPU_Results&, int);
 inline void reorder(Net_Results&, int);
 inline void reorder(Raw_Result&);
#endif
// ----------------------------------------------------------------------------



#endif	// ___IOCOMMON_H_DEFINED___
