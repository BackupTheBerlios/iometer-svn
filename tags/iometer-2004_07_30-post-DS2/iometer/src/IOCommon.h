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
/* ##               - Functions like Strip() and IsBigEndian() should     ## */
/* ##                 be moved to a new code file named IOCommon.cpp      ## */
/* ##                                                                     ## */
/* ## ------------------------------------------------------------------- ## */
/* ##                                                                     ## */
/* ##  Changes ...: 2005-04-17 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Code cleanup.                                       ## */
/* ##               2005-04-10 (mingz@ele.uri.edu)                        ## */
/* ##               - Corrected the macro definition for BLKGETSIZE64.    ## */
/* ##               2005-04-07 (thayneharmon@users.sourceforge.net)       ## */
/* ##               - Correct a typo of IOMTR_OSFAMILY_NETWARE            ## */
/* ##               - Added a declaration for SSGetLANCommonCounters()    ## */
/* ##               2004-09-28 (mingz@ele.uri.edu)                        ## */
/* ##               - Added the syslog.h header for linux.                ## */
/* ##               2004-09-01 (henryx.w.tieman@intel.com)                ## */
/* ##               - Added ifdef's to get Iometer to compile in x86_64   ## */
/* ##                 environment. See IOMTR_SETTING_GCC_M64.             ## */
/* ##               - Replaced most use of macro SOCKET with CONNECTION.  ## */
/* ##               - Created IOMTR_MACRO_INTERLOCK_CAST to get the       ## */
/* ##                 Interlocked functions and macros to work in with    ## */
/* ##                 several different compilers.                        ## */
/* ##               - Switched to more generic IOMTR_CPU_X86_64.          ## */
/* ##               2004-07-26 (mingz@ele.uri.edu)                        ## */
/* ##               - Added the BLKGETSIZE here.                          ## */
/* ##               2004-06-10 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Corrected the macro definition for BLKGETSIZE64.    ## */
/* ##               2004-03-27 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Applied Dan Bar Dov's patch for adding              ## */
/* ##                 Linux on PPC support.                               ## */
/* ##               2004-03-26 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Code cleanup to ensure common style.                ## */
/* ##               - Applied Thayne Harmon's patch for supporting        ## */
/* ##                 Netware support (on I386).                          ## */
/* ##               2004-03-05 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Moved the Dump_*_Results() function prototypes      ## */
/* ##                 (used for debugging purposes) from here to          ## */
/* ##                 ByteOrder.cpp                                       ## */
/* ##               - Moved the *_double_swap() function prototypes       ## */
/* ##                 (for Linux/XScale) from IOManager.h to here.        ## */
/* ##               2004-02-13 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Increased MAX_NAME from 64 to 80 - according to     ## */
/* ##                 swapctl(2) man page in Solaris 2.6 (relevant due    ## */
/* ##                 to the Get_All_Swap_Devices() function).            ## */
/* ##               2004-02-12 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Moved BLKSSZGET, BLKBSZGET and BLKGETSIZE64         ## */
/* ##                 from the IOTargetDisk.cpp file to here.             ## */
/* ##               2004-02-06 (mingz@ele.uri.edu)                        ## */
/* ##               - Added a IOMTR_CPU_XSCALE cpu type                   ## */
/* ##               - Added define for /proc/stat style in order to       ## */
/* ##                 support different kernel version                    ## */
/* ##               2003-10-05 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Integrated the modification contributed by          ## */
/* ##                 Vedran Degoricija, to get the code compile with     ## */
/* ##                 the Windows 64 Bit on AMD64.                        ## */
/* ##               2003-08-03 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Integrated the modification contributed by          ## */
/* ##                 Vedran Degoricija, to get the code compile with     ## */
/* ##                 the MS DDK on IA64.                                 ## */
/* ##               2003-08-02 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Added the currently not supported CPU types         ## */
/* ##                 (see README under IOMTR_CPU_*) as well.             ## */
/* ##               2003-07-27 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Removed the [BIG|LITTLE]_ENDIAN_ARCH defines.       ## */
/* ##               - Added the implementation of the IsBigEndian()       ## */
/* ##                 function which detects the endian type of the CPU.  ## */
/* ##               2003-07-19 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Merged IOTime.h stuff into (parts of)               ## */
/* ##               2003-07-13 (daniel.scheibli@edelbyte.org)             ## */
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



// Check and map the different global defines for Operating
// System family, Operating System and CPU (see README).
// ----------------------------------------------------------------------------
// Check Operating System family mapping
#if ( defined(IOMTR_OSFAMILY_NETWARE) && !defined(IOMTR_OSFAMILY_UNIX) && !defined(IOMTR_OSFAMILY_WINDOWS)) || \
    (!defined(IOMTR_OSFAMILY_NETWARE) &&  defined(IOMTR_OSFAMILY_UNIX) && !defined(IOMTR_OSFAMILY_WINDOWS)) || \
    (!defined(IOMTR_OSFAMILY_NETWARE) && !defined(IOMTR_OSFAMILY_UNIX) &&  defined(IOMTR_OSFAMILY_WINDOWS))
 // nop
#else    
 #error ===> ERROR: Check the Operating System to Operating System family mapping!
#endif
// ----------------------------------------------------------------------------
// Check the Operating System mapping
#if ( defined(IOMTR_OS_LINUX) && !defined(IOMTR_OS_NETWARE) && !defined(IOMTR_OS_SOLARIS) && !defined(IOMTR_OS_WIN32) && !defined(IOMTR_OS_WIN64)) || \
    (!defined(IOMTR_OS_LINUX) &&  defined(IOMTR_OS_NETWARE) && !defined(IOMTR_OS_SOLARIS) && !defined(IOMTR_OS_WIN32) && !defined(IOMTR_OS_WIN64)) || \
    (!defined(IOMTR_OS_LINUX) && !defined(IOMTR_OS_NETWARE) &&  defined(IOMTR_OS_SOLARIS) && !defined(IOMTR_OS_WIN32) && !defined(IOMTR_OS_WIN64)) || \
    (!defined(IOMTR_OS_LINUX) && !defined(IOMTR_OS_NETWARE) && !defined(IOMTR_OS_SOLARIS) &&  defined(IOMTR_OS_WIN32) && !defined(IOMTR_OS_WIN64)) || \
    (!defined(IOMTR_OS_LINUX) && !defined(IOMTR_OS_NETWARE) && !defined(IOMTR_OS_SOLARIS) && !defined(IOMTR_OS_WIN32) &&  defined(IOMTR_OS_WIN64))
 // nop
#else    
 #error ===> ERROR: You have to define exactly one IOMTR_OS_* global define!
#endif
// ----------------------------------------------------------------------------
// Check the Processor mapping
#if ( defined(IOMTR_CPU_ALPHA) && !defined(IOMTR_CPU_I386) && !defined(IOMTR_CPU_IA64) && !defined(IOMTR_CPU_MIPS) && !defined(IOMTR_CPU_PPC) && !defined(IOMTR_CPU_SPARC) && !defined(IOMTR_CPU_X86_64) && !defined(IOMTR_CPU_XSCALE)) || \
    (!defined(IOMTR_CPU_ALPHA) &&  defined(IOMTR_CPU_I386) && !defined(IOMTR_CPU_IA64) && !defined(IOMTR_CPU_MIPS) && !defined(IOMTR_CPU_PPC) && !defined(IOMTR_CPU_SPARC) && !defined(IOMTR_CPU_X86_64) && !defined(IOMTR_CPU_XSCALE)) || \
    (!defined(IOMTR_CPU_ALPHA) && !defined(IOMTR_CPU_I386) &&  defined(IOMTR_CPU_IA64) && !defined(IOMTR_CPU_MIPS) && !defined(IOMTR_CPU_PPC) && !defined(IOMTR_CPU_SPARC) && !defined(IOMTR_CPU_X86_64) && !defined(IOMTR_CPU_XSCALE)) || \
    (!defined(IOMTR_CPU_ALPHA) && !defined(IOMTR_CPU_I386) && !defined(IOMTR_CPU_IA64) &&  defined(IOMTR_CPU_MIPS) && !defined(IOMTR_CPU_PPC) && !defined(IOMTR_CPU_SPARC) && !defined(IOMTR_CPU_X86_64) && !defined(IOMTR_CPU_XSCALE)) || \
    (!defined(IOMTR_CPU_ALPHA) && !defined(IOMTR_CPU_I386) && !defined(IOMTR_CPU_IA64) && !defined(IOMTR_CPU_MIPS) &&  defined(IOMTR_CPU_PPC) && !defined(IOMTR_CPU_SPARC) && !defined(IOMTR_CPU_X86_64) && !defined(IOMTR_CPU_XSCALE)) || \
    (!defined(IOMTR_CPU_ALPHA) && !defined(IOMTR_CPU_I386) && !defined(IOMTR_CPU_IA64) && !defined(IOMTR_CPU_MIPS) && !defined(IOMTR_CPU_PPC) &&  defined(IOMTR_CPU_SPARC) && !defined(IOMTR_CPU_X86_64) && !defined(IOMTR_CPU_XSCALE)) || \
    (!defined(IOMTR_CPU_ALPHA) && !defined(IOMTR_CPU_I386) && !defined(IOMTR_CPU_IA64) && !defined(IOMTR_CPU_MIPS) && !defined(IOMTR_CPU_PPC) && !defined(IOMTR_CPU_SPARC) &&  defined(IOMTR_CPU_X86_64) && !defined(IOMTR_CPU_XSCALE)) || \
    (!defined(IOMTR_CPU_ALPHA) && !defined(IOMTR_CPU_I386) && !defined(IOMTR_CPU_IA64) && !defined(IOMTR_CPU_MIPS) && !defined(IOMTR_CPU_PPC) && !defined(IOMTR_CPU_SPARC) && !defined(IOMTR_CPU_X86_64) &&  defined(IOMTR_CPU_XSCALE))
 // nop
#else    
 #error ===> ERROR: You have to define exactly one IOMTR_CPU_* global define!
#endif
// ----------------------------------------------------------------------------



// Include the different header files
// (both, OS family based and common)
// ----------------------------------------------------------------------------
#if defined(IOMTR_OSFAMILY_WINDOWS)   // Only first, because it is needed here!
 #define VC_EXTRALEAN
 #pragma warning (disable: 4242)
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
#include <math.h>
// ----------------------------------------------------------------------------
#include <iostream>
#include <fstream>
using namespace std;
// ----------------------------------------------------------------------------
#if defined(IOMTR_OSFAMILY_NETWARE)
 #include <sys/timeval.h>
 #include <sys/time.h>
 #include <sys/socket.h>
 #include <unistd.h>
 #include <signal.h>
 #include <netinet/in.h>   // in_addr_t
 #include <nks/memory.h>
 #include <nks/fsio.h>
 #include <pthread.h>
 #include <mmpublic.h>
#endif
// ----------------------------------------------------------------------------
#if defined(IOMTR_OSFAMILY_UNIX)
 #include <sys/timeb.h>
 #include <unistd.h>
 #include <pthread.h>
 #include <signal.h>
 #include <aio.h>
 #include <netinet/in.h>   // in_addr_t
 #if defined(IOMTR_OS_LINUX)
  #include <sys/ioctl.h>
 #endif
 #if defined(IOMTR_OS_LINUX) || defined(IOMTR_OS_SOLARIS)
  #include <syslog.h>
 #endif
#endif
// ----------------------------------------------------------------------------
#if defined(IOMTR_OSFAMILY_WINDOWS)
 #include <sys/timeb.h>
 #if (_MSC_VER < 1300) || defined(USING_DDK)
  #include "ostream64.h"
 #endif
#endif
// ----------------------------------------------------------------------------
#include "IOVersion.h"   // version info definitions
// ----------------------------------------------------------------------------




// Define the different data types
// (both, OS family based and common)
// ----------------------------------------------------------------------------
#if defined(IOMTR_OSFAMILY_NETWARE) || defined(IOMTR_OSFAMILY_UNIX)
 #define __int64   long long
 #define __int32   long
 #define __int16   short
 #define __int8    char

 #if defined(IOMTR_SETTING_GCC_M64)
  // in the gcc on x86_64 environment long is 64 bits
  #undef __int32
  #define __int32 int
 #endif

 typedef long long	       _int64;
 //typedef long long	       LARGE_INTEGER; 
 typedef unsigned long long    DWORDLONG;

 #if defined(IOMTR_OSFAMILY_NETWARE)
  #ifndef LONG
   #define LONG	unsigned long
  #endif
  #ifndef WORD
   #define WORD	unsigned short
  #endif
 #elif defined(IOMTR_OSFAMILY_UNIX)
  // This is OK for x86-64 processors because LONG is only used in Netware or
  // in MeterCtrl.cpp.or safely internally in IOTargetDisk.cpp.
  // It could cause problems because in the x86-64 environment long is 64 bits.
  typedef long		       LONG;
 #else
  #warning ===> WARNING: You have to do some coding here to get the port done!
 #endif
 #if defined(IOMTR_SETTING_GCC_M64)
  // DWORD is supposed to be an unsigned 32 bit number.
  typedef unsigned int	       DWORD;
 #else
  typedef unsigned long	       DWORD;
 #endif
 
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

 //  The places in the Iometer code that now use CONNECTION used to use
 //  SOCKET. But SOCKET is a defined type in winsock2.h used for network
 //  access. Iometer defined a type called SOCKET used in NetTCP.cpp but 
 //  the original winsock SOCKET was used in IOPortTCP.cpp. Defining
 //  a new CONNECTION type allows me to separate the two different concepts
 //  cleanly.
 typedef void                 *CONNECTION;
 
 typedef int		       SOCKET;
 typedef const void	      *LPCVOID;

 typedef DWORD		      *LPDWORD;
 typedef BYTE		      *LPBYTE;
 typedef LPCSTR		       LPCTSTR;

 typedef unsigned long         ULONG_PTR, *PULONG_PTR;
 typedef ULONG_PTR             DWORD_PTR;
#endif 
// ----------------------------------------------------------------------------
#if defined(IOMTR_OSFAMILY_WINDOWS)
 #ifndef USING_DDK
  #if defined(IOMTR_OS_WIN32)
   // dps: Was __int32 in before, but conflicts while conversion
   //      from unsigned __int32 to unsigned long for instance
   //      (using Microsoft Visual C++).
   typedef long LONG_PTR, *PLONG_PTR;
   typedef unsigned long  ULONG_PTR, *PULONG_PTR;
   typedef ULONG_PTR      DWORD_PTR;
  #endif
  #if defined(IOMTR_OS_WIN64)
   typedef __int64 LONG_PTR, *PLONG_PTR;
   typedef unsigned __int64 ULONG_PTR, *PULONG_PTR;
   typedef ULONG_PTR        DWORD_PTR;
  #endif
 #endif // USING_DDK
 //  The places in the Iometer code that now use CONNECTION used to use
 //  SOCKET. But SOCKET is a defined type in winsock2.h used for network
 //  access. Iometer defined a type called SOCKET used in NetTCP.cpp but 
 //  the original winsock SOCKET was used in IOPortTCP.cpp. Defining
 //  a new CONNECTION type allows me to separate the two different concepts
 //  cleanly.
 typedef ULONG_PTR CONNECTION;
#endif 
// ----------------------------------------------------------------------------

// Because of some of the memory over writing issues in the 64 bit environment
// and the typing required by the windows environment, variables need to be cast
// differently depending on OSFAMILY. this gets the job done... 
#if defined(IOMTR_OSFAMILY_WINDOWS)
 #define IOMTR_MACRO_INTERLOCK_CAST(a) (long *)
#elif defined(IOMTR_OSFAMILY_UNIX) || defined(IOMTR_OSFAMILY_NETWARE)
 #define IOMTR_MACRO_INTERLOCK_CAST(a) (a *)
#else
 #error ===> ERROR: You have to do some coding here to get the port done!
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

#define MAX_NAME	      80   // xca1019: Was 64 - changed according to
                                   // the Solaris 2.6 man page for swapctl(2)
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

#define SEND   1
#define RECV   2
// ----------------------------------------------------------------------------
#if defined(IOMTR_OSFAMILY_NETWARE) || defined(IOMTR_OSFAMILY_UNIX)
 //param does not contain a valid internet address
 // (For Win32 || _WIN64, INADDR_NONE is defined as 0xffffffff)
 #ifndef INADDR_NONE
  #define INADDR_NONE   (in_addr_t)-1
 #endif

 // Needed to get the CDECL stuff thru (under non Windows platforms)
 #define CDECL

 #define FALSE			0
 #define TRUE			1
 #define INVALID_HANDLE_VALUE   (HANDLE)-1
 #define INVALID_SOCKET		(~0L)
 #define PERF_NO_INSTANCES	-1
 #define SOCKET_ERROR		-1
 #define SD_BOTH		2
 #define INFINITE		(unsigned)~0
 #define ERROR_IO_PENDING	501
 #define WAIT_TIMEOUT		ERROR_IO_PENDING
 #define ERROR_IO_INCOMPLETE	ERROR_IO_PENDING

 #define FILE_ELEMENT		1
 #define CQ_ELEMENT		0

 #if defined(IOMTR_OSFAMILY_NETWARE)
  #define SIGEV_NONE		0
  #define AIO_NOTCANCELED	1
 #endif

 #if defined(IOMTR_OS_LINUX)
  // different Linux kernel has different /proc/stat style.
  // Currently we only support vanilla 2.4 and 2.6 alike
  #define PROCSTATUNKNOWN	0x00
  #define PROCSTAT24STYLE	0x01
  #define PROCSTAT26STYLE	0x02
 #endif
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
#if defined(IOMTR_OSFAMILY_NETWARE)	
	char pad[2];
#endif	
	int	       	   processors;
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
#if defined(IOMTR_OSFAMILY_UNIX) || defined(IOMTR_OSFAMILY_NETWARE)
 // This LPOVERLAPPED typedef is from WINBASE.H
 #if defined(IOMTR_OSFAMILY_NETWARE)
 struct  aiocb64 {
 	int		aio_fildes;
 	void		*aio_buf;
 	size_t		aio_nbytes;
 	off_t		aio_offset;
 	int		aio_flag;
 	int		error;
 	int		returnval;
#if defined(IOMTR_SETTING_GCC_M64)
 	unsigned int	completion_key;
#else
 	unsigned long	completion_key;
#endif
 	struct {
 		int sigev_notify;
 		}	aio_sigevent;
	};
 struct timeb {
#if defined(IOMTR_SETTING_GCC_M64)
	int time;
#else
	long time;
#endif
	unsigned short millitm;
	short timezone;
	short dstflag;
	};
 #endif
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
#if defined(IOMTR_SETTING_GCC_M64)
	long	bytes_transferred;
#else
	int	bytes_transferred;
#endif
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
 #if defined(IOMTR_OSFAMILY_NETWARE)
	int type;
 #endif
 };
#endif 
// ----------------------------------------------------------------------------



// Define the global variables
// (both, OS family based and common)
// ----------------------------------------------------------------------------
const char NEW_WORKER_COMMAND[]    = "start /MIN ";
const char NEW_WORKER_EXECUTABLE[] = "dynamo";
// ----------------------------------------------------------------------------
#if defined(IOMTR_OSFAMILY_UNIX) || defined(IOMTR_OSFAMILY_NETWARE)
 extern pthread_mutex_t lock_mt;   // we use one global locking mutex
#endif
// ----------------------------------------------------------------------------
#if defined(IOMTR_CPU_SPARC)
 extern double processor_speed_to_nsecs;
#endif
// ----------------------------------------------------------------------------



// Define the different function prototypes and macros for
// global functions - including the Strip() and IsBigEndian()
// function (both, OS family based and common)
// ----------------------------------------------------------------------------
extern void GetAppFileVersionString( char **ppStrStandard, char **ppStrWithDebug );

inline void rotate(char *ptr, int size);
template <class T> inline void reorder(T&);
inline void reorder(CPU_Results&, int);
inline void reorder(Net_Results&, int);
inline void reorder(Raw_Result&);

inline char *Strip( char *pcString )
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

inline int IsBigEndian( void )
{
 union EndianUnion
 {
  struct EndianStruct
  {
   unsigned ubBit1 :1;
   unsigned ubBit2 :1;
   unsigned ubBit3 :1;
   unsigned ubBit4 :1;
   unsigned ubBit5 :1;
   unsigned ubBit6 :1;
   unsigned ubBit7 :1;
   unsigned ubBit8 :1;
   unsigned ubBitN :8;
  } sStruct;
  unsigned char ucNumber;
 } sUnion;
 sUnion.sStruct.ubBit1 = 0;
 sUnion.sStruct.ubBit2 = 0;
 sUnion.sStruct.ubBit3 = 0;
 sUnion.sStruct.ubBit4 = 0;
 sUnion.sStruct.ubBit5 = 0;
 sUnion.sStruct.ubBit6 = 0;
 sUnion.sStruct.ubBit7 = 0;
 sUnion.sStruct.ubBit8 = 0;
 sUnion.sStruct.ubBitN = 0;
 sUnion.ucNumber       = 5;

 if( (sUnion.sStruct.ubBit1 == 1) && (sUnion.sStruct.ubBit3 == 1) )
 { return(0); }      /* ##### Little Endian */
 else
 {
  if( (sUnion.sStruct.ubBit6 == 1) && (sUnion.sStruct.ubBit8 == 1) )
  { return(1); }     /* ##### Big Endian    */
  else
  { return(-42); }   /* ##### Unknown       */
 }
}

#if defined(_DEBUG)
 void	Dump_Raw_Result(struct Raw_Result *res);
 void	Dump_Manager_Results(struct Manager_Results *res);
 void	Dump_CPU_Results(struct CPU_Results *res);
 void	Dump_Net_Results(struct Net_Results *res);
#endif
// ----------------------------------------------------------------------------
#if defined(IOMTR_OSFAMILY_NETWARE) || defined(IOMTR_OSFAMILY_UNIX)
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

 #if defined(IOMTR_OSFAMILY_NETWARE)
  #define _timeb		timeb
  #define _ftime		nwtime
  #define Sleep(x)		delay((x))
 #elif defined(IOMTR_OSFAMILY_UNIX)
  #define _timeb		timeb
  #define _ftime		ftime
  #define Sleep(x) 		usleep((x) * 1000)
 #else
  #warning ===> WARNING: You have to do some coding here to get the port done!
 #endif

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
#if defined(IOMTR_OS_LINUX)
 extern DWORDLONG jiffies(void);
 extern DWORDLONG rdtsc(void);

 #if defined(IOMTR_CPU_PPC)
  extern DWORD get_tbl();
  extern DWORD get_tbu();
 #endif

 #if defined(IOMTR_CPU_XSCALE)
  extern void double_wordswap(double *d);
  extern void Manager_Info_double_swap(struct Manager_Info *p);
  extern void Manager_Results_double_swap(struct Manager_Results *p);
  extern void CPU_Results_double_swap(struct CPU_Results *p);
  extern void Net_Results_double_swap(struct Net_Results *p);
 #endif
 
 #if defined(_IO)  && !defined(BLKSSZGET)
  #define BLKSSZGET    _IO(0x12,104)
 #endif
 #if defined(_IOR) && !defined(BLKBSZGET)
  #define BLKBSZGET    _IOR(0x12,112,size_t)
 #endif
 #if defined(_IO)  && !defined(BLKGETSIZE)
  #define BLKGETSIZE   _IO(0x12,96)
 #endif
 #if defined(_IO)  && !defined(BLKGETSIZE64)
  #define BLKGETSIZE64 _IOR(0x12,114, size_t)
 #endif
#endif 
// ----------------------------------------------------------------------------
#if defined(IOMTR_OS_NETWARE)
 int aio_suspend64(struct aiocb64 **cb, int a, struct timespec *);
 int aio_error64(struct aiocb64 *cb);
 int aio_return64(struct aiocb64 *cb);
 int aio_read64(struct aiocb64 *cb, int type);
 int aio_write64(struct aiocb64 *cb, int type);
 int aio_cancel64(int a, struct aiocb64 *cb);

 #ifdef __cplusplus
 extern "C"
 {
 #endif
 extern LONG  GetTimerMinorTicksPerSecond(void);
 extern unsigned long kGetProcessorInterruptCount(unsigned int, unsigned int *);
 extern void  EnterDebugger();
 extern LONG SSGetLANCommonCounters(unsigned long, unsigned long, unsigned char *, unsigned int);
 #ifdef __cplusplus
 }
 #endif

 extern DWORDLONG rdtsc(void); 
#endif
// ----------------------------------------------------------------------------
#if defined(IOMTR_OS_SOLARIS)
 extern "C" DWORDLONG rdtsc();
#endif
// ----------------------------------------------------------------------------
#if defined(IOMTR_OS_WIN32) || defined(IOMTR_OS_WIN64)
 extern DWORDLONG rdtsc();
#endif
// ----------------------------------------------------------------------------


#endif	// ___IOCOMMON_H_DEFINED___
