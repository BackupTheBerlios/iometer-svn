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
// IOTarget.h: Interface for the Target class.
//
// Target is a pure virtual class that is used as a base class for any I/O
// target, such as a disk or a network connection.  
//
//////////////////////////////////////////////////////////////////////

#ifndef TARGET_DEFINED
	#define TARGET_DEFINED

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

// Set TARGET_DETAILS to 1 to enable target related debug messages, 0 to disable them.
// If _DETAILS  is defined and TARGET_DETAILS is set to 0, some messages will
// still appear.
#define TARGET_DETAILS 0

// Target is a virtual class that is used as a base class for any I/O
// target, such as a disk or a network connection.  It includes only members
// and methods that are appropriate for all types of target.

#include "IOCommon.h"
#include "IOTest.h"
#include "IOCQ.h"

class Target  
{
public:

	Target();
	virtual ~Target();

	virtual	BOOL	Initialize( Target_Spec *target_info, CQ *cq ) = NULL;
	virtual BOOL	Open( volatile TestState *test_state, int open_flag = 0 ) = NULL;
	virtual BOOL	Close( volatile TestState *test_state ) = NULL;

	virtual ReturnVal	Read( LPVOID buffer, Transaction *trans ) = NULL;
	virtual ReturnVal	Write( LPVOID buffer, Transaction *trans ) = NULL;

	virtual DWORDLONG	Rand();

	Target_Spec	spec;

	// Variables used to control connection rate testing.
	int			trans_left_in_conn;
	DWORDLONG	conn_start_time;
	int			outstanding_ios;
	BOOL		is_closing;

private:

};

#endif // !defined(TARGET_DEFINED)
