/* ######################################################################### */
/* ##                                                                     ## */
/* ##  Dynamo / IOTime.cpp                                                ## */
/* ##                                                                     ## */
/* ## ------------------------------------------------------------------- ## */
/* ##                                                                     ## */
/* ##  Job .......: Implements the timing functions.                      ## */
/* ##                                                                     ## */
/* ## ------------------------------------------------------------------- ## */
/* ##                                                                     ## */
/* ##  Remarks ...: <none>                                                ## */
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
/* ##  Changes ...: 2005-04-18 (raltherr@apple.com)                       ## */
/* ##               - Support for MacOS X                                 ## */
/* ##               2005-01-12 (henryx.w.tieman@intel.com)                ## */
/* ##               - Added code for Linux on Intel Itanium (ia64).       ## */
/* ##               2004-09-01 (henryx.w.tieman@intel.com)                ## */
/* ##               - The x86_64 architecture can use rdtsc.              ## */
/* ##               - Switched to the more generic IOMTR_CPU_X86_64.      ## */
/* ##               2004-03-27 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Applied Dan Bar Dov's patch for adding              ## */
/* ##                 Linux on PPC support.                               ## */
/* ##               - Code cleanup to ensure common style.                ## */
/* ##               - Applied Thayne Harmon's patch for supporting        ## */
/* ##                 Netware support (on I386).                          ## */
/* ##               2004-02-12 (mingz@ele.uri.edu)                        ## */
/* ##               - Implemented rdtsc under xscale via CCNT.            ## */
/* ##               2004-02-07 (mingz@ele.uri.edu)                        ## */
/* ##               - Changed call from im_kstat to iomtr_kstat           ## */
/* ##               2004-02-06 (mingz@ele.uri.edu)                        ## */
/* ##               - Added ioctl call to get jiffies value from          ## */
/* ##                 im_kstat module;                                    ## */
/* ##               - Added code to get jiffies value from 2.6 /proc/stat ## */
/* ##                 style;                                              ## */
/* ##               2003-10-05 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Integrated the modification contributed by          ## */
/* ##                 Vedran Degoricija, to get the code compile with     ## */
/* ##                 the Windows 64 Bit on AMD64.                        ## */
/* ##               2003-08-02 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Integrated the modification contributed by          ## */
/* ##                 Vedran Degoricija, to get the code compile with     ## */
/* ##                 the MS DDK on IA64.                                 ## */
/* ##               2003-07-19 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Merged IOTime.h stuff into (parts of)               ## */
/* ##               - Implemented the IOMTR_[OSFAMILY|OS|CPU]_* global    ## */
/* ##                 define as well as their integrity checks.           ## */
/* ##               - Integrated the License Statement into this header.  ## */
/* ##               2003-03-04 (joe@eiler.net)                            ## */
/* ##               - Added new header holding the changelog.             ## */
/* ##               - Moved contents of rdtsc.c into here                 ## */
/* ##                                                                     ## */
/* ######################################################################### */

#include "IOCommon.h"

#if defined(IOMTR_OS_LINUX)
#include "iomtr_kstat/iomtr_kstat.h"

extern int kstatfd;
#endif

#if defined(IOMTR_OS_LINUX)
DWORDLONG jiffies(void)
{
	DWORDLONG j[8], jf;
	FILE *fp;
	char tmpBuf[SMLBUFSIZE];
	int i;

	if (kstatfd > 0 && ioctl(kstatfd, IM_IOC_GETCURJIFFIES, (unsigned long long*)&jf) >= 0) {
		return jf;
	}

	for (i = 0; i < 8; i++)
		j[i] = 0;
	fp = fopen("/proc/stat", "r");
	fgets(tmpBuf, SMLBUFSIZE, fp);
	fgets(tmpBuf, SMLBUFSIZE, fp);
	sscanf(tmpBuf, "cpu0 %lld %lld %lld %lld %lld %lld %lld %lld\n", 
			j, j + 1, j + 2, j + 3, j + 4, j + 5, j + 6, j + 7);
	fclose(fp);
	jf = 0;
	for (i = 0; i < 8; i++)
		jf += j[i];
	return jf;
}

#if defined(IOMTR_CPU_I386) || defined(IOMTR_CPU_X86_64)
DWORDLONG rdtsc(void)
{
	// Original code (returning the cpu cycle counter)
	unsigned int lo, hi;
	__asm__ __volatile__("rdtsc":"=a"(lo), "=d"(hi));

	return (lo | ((DWORDLONG) hi << 32));
	// Alternative code (returning the cpu cycle counter too)
	//      unsigned long long int x;
	//      __asm__ volatile (".byte 0x0f, 0x31" : "=A" (x));
	//      return(x);
}
#elif defined(IOMTR_CPU_PPC)
#define CPU_FTR_601                     0x00000100

DWORD get_tbl()
{
	DWORD ccc;
	__asm__ __volatile__("98:    mftb %0\n"
			     "99:\n"
			     ".section __ftr_fixup,\"a\"\n"
			     "       .long %1\n"
			     "       .long 0\n"
			     "       .long 98b\n" "       .long 99b\n" ".previous":"=r"(ccc):"i"(CPU_FTR_601));
	return ccc;
}

DWORD get_tbu()
{
	DWORD ccc;
	__asm__ __volatile__("98:    mftbu %0\n"
			     "99:\n"
			     ".section __ftr_fixup,\"a\"\n"
			     "       .long %1\n"
			     "       .long 0\n"
			     "       .long 98b\n" "       .long 99b\n" ".previous":"=r"(ccc):"i"(CPU_FTR_601));
	return ccc;
}

DWORDLONG rdtsc(void)
{
	// Original code (returning the cpu cycle counter)

	// read 64 bit tbl (time base) using motorola example 
	DWORD lo, hi1, hi2;

	do {
		hi1 = get_tbu();
		lo = get_tbl();
		hi2 = get_tbu();
	} while (hi1 != hi2);

	return ((DWORDLONG) hi1) << 32 | (DWORDLONG) lo;
}
#elif defined(IOMTR_CPU_XSCALE)

#define CCNT_IOC_MAGIC		0xAC
#define CCNT_IOC_GETCCNT	_IOR(CCNT_IOC_MAGIC, 1, unsigned long long)
extern int ccntfd;

DWORDLONG rdtsc(void)
{
	unsigned long long ccnt;

	if (ioctl(ccntfd, CCNT_IOC_GETCCNT, &ccnt) < 0) {
		ccnt = 0;
	}
	return (ccnt);
}
#elif defined(IOMTR_CPU_IA64)
DWORDLONG rdtsc(void)
{
	unsigned long temp;

	/* TODO: If you are going to use the Intel compiler */
	/*       you need a different line of code. */
	__asm__ __volatile__("mov %0=ar.itc":"=r"(temp)::"memory");

	return temp;
}
#else
  // Was the following 2 lines in before, but for which CPU (nevertheless it is useless!)?
  //    /* Totally cheesy rewrite of rdtsc! */
  //    return((DWORDLONG)time(NULL) * 200);
#warning ===> WARNING: You have to do some coding here to get the port done!
#endif
// ----------------------------------------------------------------------------
#elif defined(IOMTR_OSFAMILY_NETWARE)
#if defined(IOMTR_CPU_I386)
__declspec(naked)
extern DWORDLONG rdtsc()
{
	__asm__ {
		_emit 0Fh	// Store low  32-bits of counter in EAX.
		_emit 31h	// Store high 32-bits of counter in EDX.
		ret
	}
}
#else
#warning ===> WARNING: You have to do some coding here to get the port done!
#endif
// ----------------------------------------------------------------------------
#elif defined(IOMTR_OS_SOLARIS)
#if defined(IOMTR_CPU_I386)
unsigned long long rdtsc()
{
	asm(".byte 0x0f, 0x31");
}
#elif defined(IOMTR_CPU_SPARC)
#include <sys/types.h>
#include <sys/time.h>
double processor_speed_to_nsecs;
unsigned long long rdtsc()
{
	return (DWORDLONG) ((double)gethrtime() * (double)processor_speed_to_nsecs);
}
#else
#warning ===> WARNING: You have to do some coding here to get the port done!
#endif
// ----------------------------------------------------------------------------
#elif defined(IOMTR_OS_WIN32) && defined(IOMTR_CPU_I386)
 //
 // In WIN32 to read the IA32 Time Stamp Counter (TSC)
 // Use the opcode since MSFT compiler doesn't recognize the RDTSC instruction.
 //
__declspec(naked)
extern DWORDLONG rdtsc()
{
	_asm {
		_emit 0Fh	// Store low  32-bits of counter in EAX.
		_emit 31h	// Store high 32-bits of counter in EDX.
		ret
	}
}

// ----------------------------------------------------------------------------
#elif defined(IOMTR_OS_WIN64) && defined(IOMTR_CPU_IA64)
 //
 // Comment the next line out if you are building an application with the SDK and
 // are not using the DDK to build a driver.
 //
 //#define USING_DDK

 // Ved: USING_DDK macro is here for unknown reasons. I use it to differentiate builds
 // from visual studio (and the sdk) vs the ddk. 
 // Either way, including a kernel mode header in user mode is not possible. Therefore, 
 // we wish to use the !USING_DDK path here for all cases, even though USING_DDK 
 // will really be defined in the ddk env. Get it?!?

 //#ifdef USING_DDK     // Driver
 //#include <ia64reg.h> // from IA64 DDK
 //#include <wdm.h>     // from IA64 DDK
 //#endif // USING_DDK

 //#ifndef USING_DDK    // Application

 //
 // Including the typedef that I need from DDK 'ia64reg.h' here so that we don't
 // have to include the DDK in the build path.  There are lots of additional
 // registers defined but we don't need them here.
 //

 //
 // Register set for Intel IA64
 //
typedef enum IA64_REG_INDEX {
	// ... Bunch of registers deleted here...
	CV_IA64_ApITC = 3116,	// Interval Time Counter (ITC, AR-44)
	// ... Bunch of registers deleted here...
} IA64_REG_INDEX;

 //
 // Including the defn that I need from DDK 'wdm.h' here so that we don't have to
 // include the DDK in the build path.
 //
#ifdef __cplusplus
extern "C" {
#endif

	unsigned __int64 __getReg(int);

#if defined(IOMTR_CPU_IA64)
#pragma intrinsic (__getReg)
#endif				// _M_IA64

#ifdef __cplusplus
}
#endif
 //#endif //!USING_DDK

////////////////////////////////////////////////////////////////////////////////////
//  Name:       readITC
//  Purpose:    To read the IA64 Itanium's Interval Time Counter (ITC, AR-44).  The
//              ITC is equivalent to the IA32 Time Stamp Counter (TSC).  The IA32
//              TSC can be read using the IA32 RDTSC instruction (opcode 0F 31h) but
//              there is no equivalent IA64 instruction to read the ITC.
//  Returns:    The value of the ITC
//  Parameters: None.
/////////////////////////////////////////////////////////////////////////////////////
    DWORDLONG rdtsc()
{
	// *** Removed ***
	// GetTickCount() is a temporary function used to get a number for getting Time
	// Metrics for IA64 until a better function is found that works.  Unfortunately
	// the resoultion of this timer function isn't small enough.  It's in milliseconds.
	//
	// Also need to change Performance::Get_Processor_Speed() in IOPerformanc.cpp to
	// use milliseconds instead of using the CPU's speed if using GetTickCount().
	//
	// IMPORTANT: If GetTickCount() is added back in then search IOPerformance.cpp for
	// WIN64_COUNTER_WORKAROUND for other changes that need to be added back in.
	//
	//return (DWORDLONG)GetTickCount();
	// *** End removed ***

	//
	// Should read the Itanium's Interval Time Counter (ITC - AR44).
	// This is equivalent to the IA32 Time Stamp Counter (TSC) that is read by
	// the IA32 RDTSC instruction (opcode 0F 31h)
	//
	// __getReg is a compiler intrinsic defined in 'wdm.h' of the DDK.
	// defined.  CV_IA64_ApITC is defined in 'ia64reg.h' in the DDK.
	//
	return __getReg(CV_IA64_ApITC);

}

// ----------------------------------------------------------------------------
#elif defined(IOMTR_OS_WIN64) && defined(IOMTR_CPU_X86_64)

 // Same as above, but less comments. Same story; defs are from the ddk.

unsigned __int64 __rdtsc(void);

#pragma intrinsic(__rdtsc)

DWORDLONG rdtsc()
{
	return __rdtsc();
}

// ----------------------------------------------------------------------------
#elif defined(IOMTR_OS_OSX)
#if defined(IOMTR_CPU_PPC)
#include <Carbon/Carbon.h>
double processor_speed_to_nsecs;	// declared as extern double in IOCommon.h
DWORDLONG rdtsc()
{
	DWORDLONG temp;
	AbsoluteTime now;
	Nanoseconds s;

	now = UpTime();
	s = AbsoluteToNanoseconds(now);
	temp = s.hi;
	temp <<= 32;
	temp += s.lo;

	// temp contains timestamp in nanosecs
	// temp * processor_speed_to_nsecs = timestamp in cpu cycles
	return (DWORDLONG) (temp * processor_speed_to_nsecs);
}
#else
#error ===> ERROR: You have to do some coding here to get the port done!
#endif
// ----------------------------------------------------------------------------
#else
#error ===> ERROR: You have to do some coding here to get the port done!
#endif
// ----------------------------------------------------------------------------
