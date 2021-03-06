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
// IOPort.h: Interface for the Port class.
//
// Port objects are used for communication between Dynamo and Iometer.  The 
// Port class is an abstract (pure virtual) class that defines the interface
// and includes code common to all implementations.  The classes PortTCP and
// PortPipe provide socket-based and pipe-based implementations of Port.  
//
// This file is used by both Iometer and Dynamo.
//
//////////////////////////////////////////////////////////////////////

#ifndef PORT_DEFINED
	#define PORT_DEFINED

#if defined (_WIN32) || defined (_WIN64)
#include <strstream>
using namespace std;
#else
#ifdef UNIX
#include <strstream.h>
#endif // UNIX
#endif // WIN32 || _WIN64
#include "IOCommon.h"
#include "IOMessage.h"

// Set PORT_DETAILS to 1 to enable port related debug messages, 0 to disable them.
// If _DETAILS  is defined and PORT_DETAILS is set to 0, some messages will
// still appear.
#define PORT_DETAILS	0


#define PORT_TYPE_INVALID	0
#define PORT_TYPE_TCP		1
#define PORT_TYPE_PIPE		2

#define	WELL_KNOWN_PIPE		"\\\\.\\pipe\\iometer"
#define KNOWN_PIPE			"\\pipe\\iometer"
#define WELL_KNOWN_TCP_PORT	1066

#define MESSAGE_PORT_SIZE	MESSAGE_SIZE * 3	// make pipe buffer big enough for 3 messages (arbitrary number?)

#define PORT_ERROR			(~(DWORDLONG)0)

class Port
{
public:
	// constructor and destructor	
	Port( BOOL synchronous = TRUE );
	virtual	~Port();

	// public functions common to all Ports (pure virtual, not implemented by Port)
	virtual	BOOL		Create( char* port_name = NULL, char* remote_name = NULL, 
							DWORD size = MESSAGE_PORT_SIZE, unsigned short port_number = 0 ) = NULL;
	virtual BOOL		Connect( char* port_name = WELL_KNOWN_PIPE, 
							unsigned short port_number = WELL_KNOWN_TCP_PORT ) = NULL;
	virtual BOOL		Accept() = NULL;
	virtual BOOL		Disconnect() = NULL;
	virtual BOOL		Close() = NULL;
	virtual DWORDLONG	Receive( LPVOID data, DWORD size = MESSAGE_SIZE ) = NULL;
	virtual DWORDLONG	Send( LPVOID data, DWORD size = MESSAGE_SIZE ) = NULL;
	virtual DWORD		Peek() = NULL;

	// public functions common to all asynchronous Ports
	//     (implemented by Port)
	virtual BOOL		IsAcceptComplete();
	virtual BOOL		IsReceiveComplete();
	virtual BOOL		IsSendComplete();
	//     (pure virtual, not implemented by Port)
	virtual BOOL		GetAcceptResult() = NULL;
	virtual DWORDLONG	GetReceiveResult() = NULL;
	virtual DWORDLONG	GetSendResult() = NULL;

	// public data members common to all Ports
	char				network_name[MAX_NETWORK_NAME];
	unsigned short		network_port;	// used only by PortTCP, ignored by PortPipe
	int					type; // PORT_TYPE_INVALID, PORT_TYPE_TCP, or PORT_TYPE_PIPE

protected:
	// private data members common to all Ports
	BOOL				synchronous;
	char				name[MAX_NETWORK_NAME];
	ostrstream			*errmsg;

	// private functions common to all Ports (implemented by Port)
	virtual void		OutputErrMsg();

	// private functions common to all asynchronous Ports (implemented by Port)
	virtual BOOL		InitOverlapped( OVERLAPPED *olap );
	virtual BOOL		IsOperationComplete( OVERLAPPED *olap );

	// private data members common to all asynchronous Ports
	OVERLAPPED			accept_overlapped;
	OVERLAPPED			receive_overlapped;
	OVERLAPPED			send_overlapped;
};



#endif
