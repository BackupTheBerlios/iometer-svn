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
// Network.h: Interface for the Network class.  Network is a pure virtual
// class that is used as a base class for any network connection.
//
// Network objects are used for connections between network workers
// in Dynamo (contrast Port, which is used for communication between
// Dynamo and Iometer).  The Network class was designed for use in
// both Iometer and Dynamo, but is currently used only in Dynamo.
//
//////////////////////////////////////////////////////////////////////

#ifndef NETWORK_DEFINED
#define NETWORK_DEFINED

#if defined (_WIN32) || defined (_WIN64)
#include <strstream>
using namespace std;
#else // !WIN32 || _WIN64
#include <strstream.h>
#endif // !WIN32 || _WIN64
#include "IOCommon.h"


// Set NETWORK_DETAILS to 1 to enable network related debug messages, 0 to disable them.
// If _DETAILS  is defined and NETWORK_DETAILS is set to 0, some messages will
// still appear.
#define NETWORK_DETAILS	0


#define NETWORK_ERROR	0xffffffffffffffff
#define SERVER	TRUE
#define CLIENT	FALSE



enum ReadWriteType
{
	READ,
	WRITE
};



class Network
{
public:
	// constructor and destructor	
	Network();
	virtual	~Network();

	// public functions common to all Networks (pure virtual, not implemented by Network)
	virtual ReturnVal	Create( BOOL create_server ) = NULL;
	virtual ReturnVal	Connect( const char* ip_address, 
							unsigned short port_number ) = NULL;
	virtual ReturnVal	Accept() = NULL;
	virtual ReturnVal	Destroy() = NULL;
	virtual ReturnVal	Receive( LPVOID buffer, DWORD bytes, LPDWORD return_value,
							LPOVERLAPPED asynchronous_io, DWORD flags = 0 ) = NULL;
	virtual ReturnVal	Send( LPVOID buffer, DWORD bytes, LPDWORD return_value,
							LPOVERLAPPED asynchronous_io, DWORD flags = 0 ) = NULL;
	virtual DWORD		Peek() = NULL;
	virtual ReturnVal	Close( BOOL close_server ) = NULL;

protected:
	virtual void OutputErrMsg();

	ostrstream *errmsg;
};

#endif
