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
// ostream64.h: Extends the standard "ostream" class to provide input and
// output of 64-bit integers.
//
//////////////////////////////////////////////////////////////////////

#ifndef	OSTREAM64_DEFINED
#define OSTREAM64_DEFINED

// Uncomment if the workaround is needed.  This was needed prior to Win64 2239 IDW SDK
// and is needed as of latest Win32 SDK
#ifdef _WIN64
//#define WORKAROUND_FOR_INT64_OFSTREAM
#else // WIN32
#ifdef WORKAROUND_FOR_INT64_OFSTREAM
#endif

#include <stdio.h>

#include <ostream>
using namespace std;

#define INT64_DIGITS	21


//
// Extension to ostream to permit int64.
//
inline ostream& operator << ( ostream& s, _int64 i )
{
    char buf[INT64_DIGITS];
    sprintf( buf, "%I64i", i );  
    return( s << buf );
}


//
// Extension to ostream to permit unsigned int64.
//
inline ostream& operator << ( ostream& s, unsigned _int64 i )
{
    char buf[INT64_DIGITS];
    sprintf( buf, "%I64i", i );
    return( s << buf );
}


#endif
#endif