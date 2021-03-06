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
// IOPort.cpp: Implementation of generic methods for the Port class.
//
// Port objects are used for communication between Dynamo and Iometer.  The 
// Port class is an abstract (pure virtual) class that defines the interface
// and includes code common to all implementations.  The classes PortTCP and
// PortPipe provide socket-based and pipe-based implementations of Port.  
//
// This file is used by both Iometer and Dynamo.
//
//////////////////////////////////////////////////////////////////////

#if defined (_WIN32) || defined (_WIN64)
#include "GalileoApp.h"
#endif
#include "IOPort.h"

#ifdef _DEBUG
#ifndef LINUX
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif



//
// Constructor and destructor.
//

Port::Port( BOOL synch )
{
	synchronous = synch;
	name[0] = '\0';
	type = PORT_TYPE_INVALID;
	errmsg = new ostrstream;
	accept_overlapped.hEvent = NULL;
	receive_overlapped.hEvent = NULL;
	send_overlapped.hEvent = NULL;
}

Port::~Port()
{
	delete errmsg;
	if ( accept_overlapped.hEvent )
		CloseHandle( accept_overlapped.hEvent );
	if ( receive_overlapped.hEvent )
		CloseHandle( receive_overlapped.hEvent );
	if ( send_overlapped.hEvent )
		CloseHandle( send_overlapped.hEvent );
}


// 
// Determine if an asynchronous operation has completed yet (TRUE = yes, FALSE = no).  Does not block.
// Utility function called by IsAcceptComplete(), IsReceiveComplete(), and IsSendComplete().
//
BOOL Port::IsOperationComplete( OVERLAPPED *olap )
{
	if ( synchronous )
	{
		return TRUE;
	}

	if ( olap->hEvent )
	{
#ifdef UNIX
		cout << "Async Port objects not supported on UNIX" << endl;
		return FALSE;
#else // WIN32 || _WIN64
		DWORD d = WaitForSingleObject ( olap->hEvent, 0 );
		return ( d == WAIT_OBJECT_0 );
#endif
	}
	else
	{
		return FALSE;
	}
}

// 
// Determine if Accept() has completed yet.
//
BOOL Port::IsAcceptComplete()
{
	return ( IsOperationComplete( &accept_overlapped ) );
}

// 
// Determine if Receive() has completed yet.
//
BOOL Port::IsReceiveComplete()
{
	return ( IsOperationComplete( &receive_overlapped ) );
}

// 
// Determine if Send() has completed yet.
//
BOOL Port::IsSendComplete()
{
	return ( IsOperationComplete( &send_overlapped ) );
}

//
// Utility function: Initialize an OVERLAPPED structure (works even if it has been previously used).  
// Return value indicates success or failure.
// 
BOOL Port::InitOverlapped(OVERLAPPED * olap)
{
	if ( synchronous )
	{
		return FALSE;
	}

	if ( olap->hEvent )
		CloseHandle( olap->hEvent );

	// specify the file pointer (meaningless for a pipe or socket, but must be set!)
	olap->Offset = 0;
	olap->OffsetHigh = 0;

	// create a manual-reset event object for the OVERLAPPED structure
	olap->hEvent = CreateEvent( NULL, TRUE, FALSE, NULL );

	if ( olap->hEvent == NULL )
		return FALSE;
	else
		return TRUE;
}


//
// Utility function: output the "errmsg" message in an appropriate manner for the
// current environment (Dynamo or Iometer).
//
void Port::OutputErrMsg()
{
	if ( !errmsg )
	{
		errmsg = new ostrstream;
		*errmsg << "Port::OutputErrMsg() called with invalid errmsg value!" << ends;
	}

#ifdef _WINDOWS
	// Iometer
	ErrorMessage( errmsg->str() );
#else
	// Dynamo
	cout << errmsg->str() << endl;
#endif

	// str() returns pointer to buffer and freezes it, we must call freeze(FALSE) to 
	// unfreeze the buffer before we can delete the object
	errmsg->rdbuf()->freeze( FALSE );
	delete errmsg;
	errmsg = new ostrstream;
}
