/*
Intel Open Source License 

Copyright (c) 2001 Intel Corporation 
All rights reserved. 
Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met: 

   Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer. 

   Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution. 

   Neither the name of the Intel Corporation nor the names of its contributors
   may be used to endorse or promote products derived from this software
   without specific prior written permission.
 
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE INTEL OR ITS  CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/
// ==========================================================================
//                Copyright (C) 1997-2000 Intel Corporation
//                          All rights reserved                               
//                INTEL CORPORATION PROPRIETARY INFORMATION                   
//    This software is supplied under the terms of a license agreement or     
//    nondisclosure agreement with Intel Corporation and may not be copied    
//    or disclosed except in accordance with the terms of that agreement.     
// ==========================================================================
//
// IOTime.h: Interface for the assembly-language function rdtsc(), 
// which gets the processor's high-resolution time stamp, and its
// implementation for Linux.  (The Windows implementation of this
// function can be found in IOTime.cpp; the Solaris implementation
// is in rdtsc.c.)
//
// This file is used by both Iometer and Dynamo.
//
//////////////////////////////////////////////////////////////////////

#ifndef TIME_DEFINED
#define TIME_DEFINED

#include "IOCommon.h"

#ifdef SOLARIS
extern "C" DWORDLONG rdtsc();
#endif // SOLARIS

#ifdef LINUX

#if defined (i386)
/*
 * Stolen from Linux kernel source.
 */
extern inline DWORDLONG rdtsc(void) {
	unsigned int lo, hi;

	__asm__ __volatile__("rdtsc" : "=a" (lo), "=d" (hi));
	return(lo | ((DWORDLONG)hi << 32));
}
#else /* !i386 */

extern inline DWORDLONG rdtsc(void) {
	/* Totally cheesy rewrite of rdtsc! */
	return((DWORDLONG)time(NULL) * 200);
}

#endif /* i386 */
#endif /* LINUX */

#if defined (_WIN32) || defined (_WIN64)

#ifdef _WIN64
//#ifdef WIN64_COUNTER_WORKAROUND

//
// Comment the next line out if you are building an application with the SDK and
// are not using the DDK to build a driver.
//
//#define USING_DDK

#ifdef USING_DDK				// Driver
#include <ia64reg.h>			// from IA64 DDK
#include <wdm.h>				// from IA64 DDK
#endif // USING_DDK

#ifndef USING_DDK				// Application
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

	CV_IA64_ApITC    =   3116,		// Interval Time Counter (ITC, AR-44)
	
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

#endif //!USING_DDK

#endif // _WIN64

DWORDLONG rdtsc();

#endif


#endif // !TIME_DEFINED
