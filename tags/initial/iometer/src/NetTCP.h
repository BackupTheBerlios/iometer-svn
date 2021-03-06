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
// NetAsyncTCP.h: Interface for an implementation of the Network class for
// asynchronous TCP/IP sockets.
//
// Network objects are used for connections between network workers
// in Dynamo (contrast Port, which is used for communication between
// Dynamo and Iometer).
//
//////////////////////////////////////////////////////////////////////

#ifndef NET_ASYNC_TCP_DEFINED
	#define NET_ASYNC_TCP_DEFINED

#include "Network.h"
#if defined (_WIN32) || defined (_WIN64)
#include "winsock2.h"
#else // UNIX
#include <sys/time.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#define SOCKADDR_IN sockaddr_in

typedef struct linger LINGER;
#endif


class NetAsyncTCP : public Network
{
public:
	// Member Functions.
	NetAsyncTCP();
	~NetAsyncTCP();
	virtual ReturnVal	Create( BOOL create_server );
	virtual ReturnVal	Connect( const char *ip_address, 
							unsigned short port_number );
	ReturnVal			ConnectSocket( SOCKADDR_IN *address );
	virtual ReturnVal	Accept();
	virtual ReturnVal	Destroy();
	virtual ReturnVal	Receive( LPVOID buffer, DWORD bytes, LPDWORD return_value,
							LPOVERLAPPED asynchronous_io, DWORD flags = 0 );
	virtual ReturnVal	Send( LPVOID buffer, DWORD bytes, LPDWORD return_value,
							LPOVERLAPPED asynchronous_io, DWORD flags = 0 );
	virtual DWORD		Peek();
	virtual ReturnVal	Close( BOOL close_server );
	ReturnVal			WaitForDisconnect();
	void				SetTimeout( int sec, int usec );
	void				SetAddress( BOOL set_server, 
							const char *ip_address = NULL,
							unsigned short port_num = 0 );

	// Member Variables
	SOCKADDR_IN		server_address;		// IP address, port.
	SOCKADDR_IN		client_address;		// IP address, port.

	SOCKET			server_socket;		// Socket where server listens for 
										// connections.  Not used by client.
	SOCKET			client_socket;		// Socket used for client/server data 
										// transmission.
#ifdef UNIX
	struct File		server_fp;			// The actual structures that will hold
	struct File		client_fp;			//     the client and server sockets on UNIX.
	int				maxfd;				// The max nos of fds' for select() call.
	
#ifdef WORKAROUND_LISTEN_BUG
	BOOL 			listening;			// flag to indicate if a socket is listening or not.
#endif // WORKAROUND_LISTEN_BUG

#endif // UNIX

protected:
	// Member Functions.
	ReturnVal			CreateSocket( SOCKET *s );
	ReturnVal			BindSocket( SOCKET *s, SOCKADDR_IN *address );
	ReturnVal			CloseSocket( SOCKET *s );
	void				SetOptions( SOCKET *s );
	char *				Error( int error_num );

	// Member Variables.
#if defined (_WIN32) || defined (_WIN64)
	WSABUF			wsa_buf;
#endif
	timeval		timeout;
	static LONG		sockets_in_use;		// Used to control WinSock 
										// initialization/cleanup.
};

#endif
