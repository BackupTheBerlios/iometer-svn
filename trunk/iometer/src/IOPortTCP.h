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
// IOPortTCP.h: Interface for an implementation of the Port class using
// TCP/IP sockets.
//
// Port objects are used for communication between Dynamo and Iometer.  The 
// Port class is an abstract (pure virtual) class that defines the interface
// and includes code common to all implementations.  The classes PortTCP and
// PortPipe provide socket-based and pipe-based implementations of Port.  
//
// This file is used by both Iometer and Dynamo.
//
//////////////////////////////////////////////////////////////////////

#ifndef PORTTCP_DEFINED
	#define PORTTCP_DEFINED

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "IOPort.h"
#ifdef UNIX
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#else // WIN_NT
#include "winsock2.h"
#endif

#ifdef UNIX
#define SOCKET	int
#endif


class PortTCP : public Port  
{
public:
	// constructor and destructor	
	PortTCP( BOOL synchronous = TRUE );
	virtual	~PortTCP();

	// public functions common to all Ports (implemented here, pure virtual in Port)
	virtual	BOOL		Create( char* port_name = NULL, char* remote_name = NULL, 
							DWORD size = MESSAGE_PORT_SIZE, unsigned short port_number = 0 );
	virtual BOOL		Connect( char* port_name = WELL_KNOWN_PIPE, 
							unsigned short port_number = WELL_KNOWN_TCP_PORT );
	virtual BOOL		Accept();
	virtual BOOL		Disconnect();
	virtual BOOL		Close();
	virtual DWORDLONG	Receive( LPVOID data, DWORD size = MESSAGE_SIZE );
	virtual DWORDLONG	Send( LPVOID data, DWORD size = MESSAGE_SIZE );
	virtual DWORD		Peek();

	// public functions common to all asynchronous Ports (implemented here, pure virtual in Port)
	virtual BOOL		GetAcceptResult();
	virtual DWORDLONG	GetReceiveResult();
	virtual DWORDLONG	GetSendResult();

protected:
	// private data members used only by PortTCP
#ifdef UNIX
	int 				server_socket;
	int 				client_socket;
#else
	SOCKET				server_socket;
	SOCKET				client_socket;
#endif

	// private data members used only by asynchronous PortTCP's
	char				*accept_ex_buffer;

	// static private data members shared by all PortTCP's
	static unsigned int	sockets_in_use;

	// private functions used only by PortTCP
	BOOL				CloseSocket( SOCKET *s, char *socket_name ); // utility function
	DWORDLONG			SynchReceive( LPVOID data, DWORD size = MESSAGE_SIZE );
	DWORDLONG			SynchSend( LPVOID data, DWORD size = MESSAGE_SIZE );

	// private functions used only by asynchronous PortTCP's
	DWORDLONG			AsynchReceive( LPVOID data, DWORD size = MESSAGE_SIZE );
	DWORDLONG			AsynchSend( LPVOID data, DWORD size = MESSAGE_SIZE );
};

#endif
