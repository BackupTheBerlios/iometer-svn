/* ######################################################################### */
/* ##                                                                     ## */
/* ##  Dynamo / IOTime.cpp                                                ## */
/* ##                                                                     ## */
/* ## ------------------------------------------------------------------- ## */
/* ##                                                                     ## */
/* ##  Job .......: Implements timing                                     ## */
/* ##                                                                     ## */
/* ## ------------------------------------------------------------------- ## */
/* ##                                                                     ## */
/* ##  Remarks ...: <none>                                                ## */
/* ##                                                                     ## */
/* ## ------------------------------------------------------------------- ## */
/* ##                                                                     ## */
/* ##  Changes ...: 2003-03-04 (joe@eiler.net)                            ## */
/* ##               - Added new header holding the changelog.             ## */
/* ##               - Moved contents of rdtsc.c into here                 ## */
/* ##                                                                     ## */
/* ######################################################################### */
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
// IOTime.cpp: Implementation of the assembly-language function rdtsc(), 
// which gets the processor's high-resolution time stamp, for Windows.
// (The Solaris implementation of this function can be found in rdtsc.c;
// the Linux implementation is in IOTime.h.)
//
// This file is used by both Iometer and Dynamo.
//
//////////////////////////////////////////////////////////////////////

#include "IOTime.h"

//#ifdef WIN64_COUNTER_WORKAROUND
#ifdef _WIN64
//
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
#endif

//
// Retrieving processor counter.
//
//#ifndef WIN64_COUNTER_WORKAROUND
#ifndef _WIN64
#ifndef UNIX
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
#endif
#endif

#ifdef UNIX
#if defined(SOLARIS) && defined(__i386)

unsigned long long rdtsc()
{
	asm(".byte 0x0f, 0x31");
}

#elif defined(SOLARIS) && defined(__sparc)  /* ! SOLARIS && __i386 */
#include <sys/types.h>
#include <sys/time.h>
double processor_speed_to_nsecs;
unsigned long long rdtsc()
{
	return (DWORDLONG)((double)gethrtime() * (double)processor_speed_to_nsecs);
}

#elif defined(LINUX)
// rdtsc code present in IOTime.h
#else
#error "rdtsc NOT SUPPORTED."
#endif // SOLARIS && __i386
#endif // UNIX
