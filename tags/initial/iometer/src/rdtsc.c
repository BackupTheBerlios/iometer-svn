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
/* ==========================================================================
//                  Copyright (C) 1998 Intel Corporation
//                          All rights reserved
//                INTEL CORPORATION PROPRIETARY INFORMATION
//    This software is supplied under the terms of a license agreement or
//    nondisclosure agreement with Intel Corporation and may not be copied
//    or disclosed except in accordance with the terms of that agreement.
// ==========================================================================
//
// rdtsc.c: Implementation of the assembly-language function rdtsc(), 
// which gets the processor's high-resolution time stamp, for Solaris.
// (The Windows implementation of this function can be found in IOTime.cpp;
// the Linux implementation is in IOTime.h.)
//
//////////////////////////////////////////////////////////////////////
*/

#ifdef UNIX
#if defined(SOLARIS) && defined(__i386)

unsigned long long rdtsc()
{
	asm(".byte 0x0f, 0x31");
}

#elif defined(SOLARIS) && defined(__sparc)  /* ! SOLARIS && __i386 */

#include <sys/types.h>
#include <sys/time.h>
extern double processor_speed_to_nsecs;
unsigned long long rdtsc()
{
	return((double)gethrtime() * (double)processor_speed_to_nsecs);
}

#elif defined(LINUX)

// rdtsc code present in IOTime.h

#else
"rdtsc NOT SUPPORTED."
#endif // SOLARIS && __i386
#endif // UNIX
