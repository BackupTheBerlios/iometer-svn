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
/* ##  Changes ...: 2003-10-05 (daniel.scheibli@edelbyte.org)             ## */
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



// Include our central headerfile
#include "IOCommon.h"



// Implements the time measurment functions
// for / in the different plattforms
// ----------------------------------------------------------------------------
#if defined(IOMTR_OS_LINUX)
 DWORDLONG jiffies(void) {
	DWORDLONG jiffies_user, jiffies_nice, jiffies_system, jiffies_idle;
	FILE *fp = fopen("/proc/stat", "r");
	fscanf(fp, "cpu %*s %*s %*s %*s\n");
	fscanf(fp, "cpu0 %lld %lld %lld %lld\n", &jiffies_user, &jiffies_nice, &jiffies_system, &jiffies_idle);
	fclose(fp);
	return(jiffies_user + jiffies_nice + jiffies_system + jiffies_idle);
 }
 #if defined(IOMTR_CPU_I386)
  DWORDLONG rdtsc(void) {
        // Original code (returning the cpu cycle counter)
	unsigned int lo, hi;
	__asm__ __volatile__("rdtsc" : "=a" (lo), "=d" (hi));
	return(lo | ((DWORDLONG)hi << 32));
        // Alternative code (returning the cpu cycle counter too)
        //	unsigned long long int x;
        //	__asm__ volatile (".byte 0x0f, 0x31" : "=A" (x));
        //	return(x);
  }
 #else
  // Was the following 2 lines in before, but for which CPU (nevertheless it is useless!)?
  //	/* Totally cheesy rewrite of rdtsc! */
  //	return((DWORDLONG)time(NULL) * 200);
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
	return (DWORDLONG)((double)gethrtime() * (double)processor_speed_to_nsecs);
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
 __declspec ( naked ) extern DWORDLONG rdtsc()
 {
	_asm
	{
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
 
 //#ifdef USING_DDK	// Driver
 //#include <ia64reg.h>	// from IA64 DDK
 //#include <wdm.h>	// from IA64 DDK
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

 unsigned __int64 __getReg (int);

 #ifdef _M_IA64
 #pragma intrinsic (__getReg)
 #endif // _M_IA64

 #ifdef __cplusplus
 }
 #endif
 //#endif //!USING_DDK

 ////////////////////////////////////////////////////////////////////////////////////
 //  Name:	 readITC
 //  Purpose: 	 To read the IA64 Itanium's Interval Time Counter (ITC, AR-44).  The
 //		 ITC is equivalent to the IA32 Time Stamp Counter (TSC).  The IA32
 //		 TSC can be read using the IA32 RDTSC instruction (opcode 0F 31h) but
 //		 there is no equivalent IA64 instruction to read the ITC.
 //  Returns:	 The value of the ITC
 //  Parameters: None.
 ///////////////////////////////////////////////////////////////////////////////////
 //
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
	return __getReg( CV_IA64_ApITC );

 }
// ----------------------------------------------------------------------------
#elif defined(IOMTR_OS_WIN64) && defined(IOMTR_CPU_AMD64)

 // Same as above, but less comments. Same story; defs are from the ddk.

 unsigned __int64 __rdtsc (void);

 #pragma intrinsic(__rdtsc)

 DWORDLONG rdtsc() 
 {
	return __rdtsc();
 }
 
// ----------------------------------------------------------------------------
#else
 #error ===> ERROR: You have to do some coding here to get the port done!
#endif
// ----------------------------------------------------------------------------


