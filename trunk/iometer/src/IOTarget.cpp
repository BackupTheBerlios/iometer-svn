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
// IOTarget.cpp: Implementation of generic methods for the Target class.
//
// Target is a pure virtual class that is used as a base class for any I/O
// target, such as a disk or a network connection.  It does not contain 
// any code except for the random number generator.  (Each Target object 
// has its own random number generator; two Targets seeded with the same
// value will produce the same series of random numbers.)
//
//////////////////////////////////////////////////////////////////////

#include "IOTarget.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Target::Target()
{
	spec.random = 1L;
	spec.test_connection_rate = FALSE;
	is_closing = FALSE;
	outstanding_ios = 0;
	spec.trans_per_conn = 0;
	trans_left_in_conn = 0;
}

Target::~Target()
{

}



//
// Return a 64-bit random number, using the following random function and conditions.
//
// X(n+1)   = ( A * Xn  + B ) mod m  
//
// m = 2^65 (so the mod operation is not needed in this case)
// a = 4c + 1 (c user defined)
// b is odd
//
#ifdef LINUX
#define A 136204069LL			// 3x7x11x13x17x23x29x4 + 1
#define B 28500701229LL		// 3x7x11x13x17x23x27x29x31
#else
#define A 136204069			// 3x7x11x13x17x23x29x4 + 1
#define B 28500701229		// 3x7x11x13x17x23x27x29x31
#endif

DWORDLONG Target::Rand( void )
{
	return( spec.random = A * spec.random + B );
}
