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
// NetAsyncTCP.cpp: Implementation of the Network class for asynchronous
// TCP/IP sockets.
//
// Network objects are used for connections between network workers
// in Dynamo (contrast Port, which is used for communication between
// Dynamo and Iometer).
//
//////////////////////////////////////////////////////////////////////

#if defined (_WIN32) || defined (_WIN64)
#include <afx.h>
#endif

#include "NetTCP.h"
#if defined (_WIN32) || defined (_WIN64)
#include "mswsock.h"
#else // ! WIN32 || _WIN64
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>

#define SD_SEND	 0x01
#define WSAENOTCONN		ENOTCONN
#define WSA_IO_PENDING	ERROR_IO_PENDING

#define wsprintf sprintf
#endif


//////////////////////////////////////////////////////////////////////
// Static data member initialization.
//////////////////////////////////////////////////////////////////////

LONG NetAsyncTCP::sockets_in_use = 0;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

NetAsyncTCP::NetAsyncTCP()
{
#if defined (_WIN32) || defined (_WIN64)
	int retval;
	WSADATA wd;
#endif

	server_socket = INVALID_SOCKET;
	client_socket = INVALID_SOCKET;
#ifdef UNIX
	server_fp.fd = client_fp.fd = -1;
	server_socket = (SOCKET) &server_fp;
	client_socket = (SOCKET) &client_fp;
	maxfd = sysconf(_SC_OPEN_MAX);    // the max # of file descriptors that select() handles.

#ifdef WORKAROUND_LISTEN_BUG
	listening = FALSE;
#endif

#endif // UNIX

	server_address.sin_family = AF_INET; // use Internet Protocol
	client_address.sin_family = AF_INET; // use Internet Protocol
	SetTimeout( 0, 0 );

#if defined (_WIN32) || defined (_WIN64)
	// Initialize WinSock if it has not yet been initialized in this process.
	if ( InterlockedIncrement ( &sockets_in_use ) == 1 )
	{
		#if NETWORK_DETAILS || _DEBUG
			cout << "Initializing WinSock." << endl;
		#endif

		// request WinSock version 2.0
		retval = WSAStartup( MAKEWORD(2, 0), &wd ); 

		if ( retval != 0 )
		{
			*errmsg << "*** Could not initialize WinSock!  Error " 
				<< retval << ends;
			OutputErrMsg();
			InterlockedDecrement ( &sockets_in_use );
		}
	}
#endif
}



NetAsyncTCP::~NetAsyncTCP()
{
	Destroy();

#if defined (_WIN32) || defined (_WIN64)
	// Clean up WinSock if nobody else is using it in this process.
	if ( InterlockedDecrement ( &sockets_in_use ) == 0 )
	{
		#if NETWORK_DETAILS || _DEBUG
			cout << "Cleaning up WinSock." << endl;
		#endif
		
		// Reset the error code.
		if ( WSACleanup() != 0 )
		{
			*errmsg << "*** Could not clean up WinSock!  Error " 
				<< WSAGetLastError() << ends;
			OutputErrMsg();
		}
	}
#endif
}


void NetAsyncTCP::SetAddress( BOOL set_server, const char *ip_address, 
	unsigned short port_num )
{
	SOCKADDR_IN *address;
	if ( set_server )
	{
		address = &server_address;
	}
	else
	{
		address = &client_address;
	}

	address->sin_family = AF_INET; // use Internet Protocol
	if ( ip_address )
	{
		// accept connections on specified address
		address->sin_addr.s_addr = inet_addr( ip_address );	
    } 
	else
	{
		// accept connections on any available address
		address->sin_addr.s_addr = htonl(INADDR_ANY);	
	}
	// listen at specified port (0 = system assigns port number)
	address->sin_port = htons ( port_num ); 
}



//
// Creates the server and client sockets, and binds the server socket.
//
ReturnVal NetAsyncTCP::Create( BOOL create_server )
{
	SOCKET		*s;
	SOCKADDR_IN	*address;
	
	if ( create_server )
	{
		s = &server_socket;
		address = &server_address;
	}
	else
	{
		s = &client_socket;
		address = &client_address;
	}

	if ( CreateSocket( s ) != ReturnSuccess || 
		BindSocket( s, address ) != ReturnSuccess )
	{
		return ReturnError;
	}

	return ReturnSuccess;
}



//
// Creates a socket.
//
ReturnVal NetAsyncTCP::CreateSocket( SOCKET *s )
{
	#if NETWORK_DETAILS || _DETAILS
		cout << "Creating socket." << endl;
	#endif

	// Create socket.
#ifdef UNIX
	struct File *fp;
	fp = (struct File *)*s;
	fp->fd = socket(AF_INET, SOCK_STREAM, PF_UNSPEC);

	if ( fp->fd < 0 )
	{
		*errmsg << "*** Could not create socket in NetAsyncTCP::"
				<< "CreateSocket(), error: " << WSAGetLastError()  << ends;
		OutputErrMsg();
		fp->fd = (int) INVALID_SOCKET;
		return ReturnError;
	}
#else // !UNIX
	*s = socket( AF_INET, SOCK_STREAM, PF_UNSPEC );

	if ( *s == INVALID_SOCKET )
	{
		*errmsg << "*** Could not create socket in NetAsyncTCP::"
				<< "CreateSocket(), error: " << WSAGetLastError()  << ends;
		OutputErrMsg();
		return ReturnError;
	}

#endif
	// Set and verify socket options.
	SetOptions( s );

	return ReturnSuccess;
}



//
// Binds a socket to the address specified.
// The SOCKADDR_IN structure must have been filled in
// before calling this function (use SetAddress).
//
ReturnVal NetAsyncTCP::BindSocket( SOCKET *s, SOCKADDR_IN *address )
{
#ifdef LINUX
	socklen_t buflen;
#else
	int buflen;
#endif
#ifdef UNIX
	struct File *fp = (struct File *)*s;
#endif

	#if NETWORK_DETAILS || _DETAILS
		cout << "Binding to socket." << endl;
	#endif

	// Bind socket.
#ifdef UNIX
	if ( bind ( fp->fd, (struct sockaddr *) address, sizeof(*address) ) != 0 )
#else
	if ( bind ( *s, (struct sockaddr *) address, sizeof(*address) ) != 0 )
#endif
	{
		*errmsg << "*** Error " << Error( WSAGetLastError() )
				<< " binding to socket in NetAsyncTCP::BindSocket()." << ends;
		OutputErrMsg();
		return ReturnError;
	}

	// get actual port number in use
	buflen = sizeof(*address);
#ifdef UNIX
	if ( getsockname ( fp->fd, (struct sockaddr *) address, &buflen ) != 0 )
#else
	if ( getsockname ( *s, (struct sockaddr *) address, &buflen ) != 0 )
#endif
	{
		*errmsg << "*** Error " << WSAGetLastError() << " getting information "
				<< "about socket in NetAsyncTCP::BindSocket()." << ends;
		OutputErrMsg();
		return ReturnError;
	}

	#if NETWORK_DETAILS || _DEBUG
		cout << "Bound socket on port " << ntohs(address->sin_port) << "." << endl;
	#endif

	return ReturnSuccess;
}



//
// Connect client_socket to specified address.
//
ReturnVal NetAsyncTCP::ConnectSocket( SOCKADDR_IN *address )
{
	// attempt to connect.
	#if NETWORK_DETAILS || _DETAILS
		cout << "Attempting to connect to socket at " 
			 << inet_ntoa( address->sin_addr ) << ":" 
			 << ntohs( address->sin_port ) << "." << endl;
	#endif

#ifdef UNIX
	if ( connect(((struct File *)client_socket)->fd, (struct sockaddr *) address, sizeof(*address)) == SOCKET_ERROR )
#else // !UNIX
	if ( connect(client_socket, (struct sockaddr *) address, sizeof(*address)) == SOCKET_ERROR )
#endif // UNIX
	{
		#if NETWORK_DETAILS || _DEBUG
			// WSAECONNREFUSED means the server isn't up yet or is busy, 
			// don't print an error message
#ifdef UNIX
			if ( errno != ECONNREFUSED ) 
#else // !UNIX
			if ( WSAGetLastError() != WSAECONNREFUSED ) 
#endif // UNIX
			{
				*errmsg << "*** Error " << WSAGetLastError() 
						<< " connecting to " << "socket " 
						<< inet_ntoa( address->sin_addr ) << ":" 
						<< ntohs( address->sin_port ) 
						<< " in NetAsyncTCP::ConnectSocket()." << ends;
				OutputErrMsg();
			}
		#endif
		// Expand error checking here for better handling.
		return ReturnRetry;
	}

	return ReturnSuccess;
}



//
// Attempts to connect the client socket to the address specified.
// Returns immediately, reguardless of success.
//
ReturnVal NetAsyncTCP::Connect( const char *ip_address, 
	unsigned short port_number )
{
	struct sockaddr_in address;
	ReturnVal result;

	// specify protocol, port, and address to connect to.
	address.sin_family = AF_INET; // use Internet Protocol
	address.sin_addr.s_addr = inet_addr(ip_address); // connect to this server
	address.sin_port = htons(port_number); // connect to this port

	result = ConnectSocket( &address );
#ifdef UNIX
	if ( result != ReturnSuccess )
	{
		// According to connect(3XN):
		//      If connect() fails, the state of the socket
		//      is unspecified.  Portable applications
		//      should close the file descriptor and
		//      create a new socket before attempting to
		//      reconnect.

		Close( CLIENT );
		if ( Create( CLIENT ) != ReturnSuccess )
		{
			cout << "*** Failed to re-create TCP client socket." << endl;
			return ReturnError;
		}
	}
#endif // UNIX
	return result;
}



//
// Sets the time to wait on the select call in the 
// Accept() function.
//
void NetAsyncTCP::SetTimeout( int sec, int usec )
{
	timeout.tv_sec = sec;
	timeout.tv_usec = usec;
}



//
// Begin accepting a connection to an existing port.  Used by server.
// Returns TRUE for success or FALSE if any error occurs.
//		Call IsAcceptComplete() to determine if it has completed, 
//		and GetAcceptResult() to get result.
//
ReturnVal NetAsyncTCP::Accept()
{
	fd_set		sock_set;		// used by select function.
#ifdef LINUX
	socklen_t addr_len;
#else
	int			addr_len;		// used by accept.
#endif
#ifdef UNIX
	struct File *fp = (struct File *)server_socket;
#endif

	// Listen to socket.
	#if NETWORK_DETAILS || _DETAILS
		cout << "Listening to socket " 
			 << inet_ntoa( server_address.sin_addr )
			 << ":" << ntohs( server_address.sin_port ) << "." << endl;
	#endif

	// It's ok to listen more than once on a socket.
	// allow only a single connection.
#ifdef UNIX
	#ifdef WORKAROUND_LISTEN_BUG
	if ( ! listening )
	{
	#endif // WORKAROUND_LISTEN_BUG
		if ( listen ( fp->fd, 1 ) != 0 )
#else // !UNIX
		if ( listen ( server_socket, 1 ) != 0 ) 
#endif // UNIX
		{
			*errmsg << "*** Error " << WSAGetLastError() 
				<< " listening to socket in NetAsyncTCP::Listen()." << ends;
			OutputErrMsg();
			return ReturnError;
		}
#ifdef UNIX
	#ifdef WORKAROUND_LISTEN_BUG
		listening =	TRUE;
	}
	#endif // WORKAROUND_LISTEN_BUG
#endif // UNIX

	// Accept connections to socket.
	#if NETWORK_DETAILS || _DEBUG
		cout << "Accepting connections to socket." << endl;
	#endif

	// Check the server socket for a connection request.  
	FD_ZERO( &sock_set );				// clear the fd_set structure.
#ifdef UNIX
	FD_SET( fp->fd, &sock_set);

	// if timeout is NULL, operation blocks until successful.
	switch ( select( maxfd, &sock_set, NULL, NULL, &timeout ) )	
#else // !UNIX
	FD_SET( server_socket, &sock_set ); // Add the server socket to it.

	// if timeout is NULL, operation blocks until successful.
	switch ( select( 0, &sock_set, NULL, NULL, &timeout ) )
#endif // UNIX
	{
	case 1:
		// A connection was requested.  Accept it.
		addr_len = sizeof( client_address );
		// Create client socket.
		#if NETWORK_DETAILS || _DEBUG
			cout << "Creating client socket." << endl;
		#endif

#ifdef UNIX
		((struct File *)client_socket)->fd = accept (fp->fd,
						(struct sockaddr *) &client_address, &addr_len);

		if (((struct File *)client_socket)->fd < 0)
		{
			*errmsg << "*** Could not create socket in NetAsyncTCP::"
					<< "CreateSocket(), error: " << WSAGetLastError()  << ends;
			OutputErrMsg();
			((struct File *)client_socket)->fd = (int)INVALID_SOCKET;
			return ReturnError;
		}
#else // !UNIX
		client_socket = accept ( server_socket, 
						(struct sockaddr *) &client_address, &addr_len );

		if ( client_socket== INVALID_SOCKET )
		{
			*errmsg << "*** Could not create socket in NetAsyncTCP::"
					<< "CreateSocket(), error: " << WSAGetLastError()  << ends;
			OutputErrMsg();
			return ReturnError;
		}
#endif // UNIX

		// Set and verify socket options.
		SetOptions( &client_socket );

		return ReturnSuccess;
	case 0:
		// opperation timed out.
		return ReturnRetry;
	default:
		// error.
		*errmsg << "*** select() failed in NetAsyncTCP::Accept(): " 
			<< WSAGetLastError() << ends;
		OutputErrMsg();
		return ReturnError;
	}
}



//
// Wait to receive notification that the remote end disconnected.
//
ReturnVal NetAsyncTCP::WaitForDisconnect()
{
	struct sockaddr	address;
#ifdef LINUX
	socklen_t addr_len = sizeof( address );
#else
	int				addr_len = sizeof( address );
#endif
	fd_set			readfds;
#ifdef UNIX
	struct File		*fp = (struct File *)client_socket;
#endif


	#if NETWORK_DETAILS || _DETAILS
		cout << "Waiting for other end to disconnect." << endl;
	#endif

#if defined (_WIN32) || defined (_WIN64)
	if ( client_socket == INVALID_SOCKET )
#else
	if ( fp->fd == (int)INVALID_SOCKET )
#endif
	{
		#if NETWORK_DETAILS || _DETAILS
			cout << "Socket is already closed." << endl;
		#endif
		return ReturnSuccess;
	}

	// If we're not connected to anything, return.
#ifdef UNIX
	if ( getpeername( fp->fd, &address, &addr_len ) == SOCKET_ERROR ) 
#else
	if ( getpeername( client_socket, &address, &addr_len ) == SOCKET_ERROR ) 
#endif
	{
		#if NETWORK_DETAILS || _DETAILS
			cout << "Not connected to anything.  Aborting WaitForDisconnect()." << endl;
		#endif
		return ReturnSuccess;
	}

	SetTimeout( 1, 0 );
	FD_ZERO( &readfds );

#ifdef UNIX
	FD_SET( fp->fd, &readfds);
	if ( select( maxfd, &readfds, NULL, NULL, &timeout ) )
#else
	FD_SET( client_socket, &readfds );
	SetTimeout( 1, 0 );		// wait 1 second for the connection to close

	if ( select( 0, &readfds, NULL, NULL, &timeout ) )
#endif
	{
		// This must mean that client_socket was closed.
		return ReturnSuccess;
	}
	else
	{
		// The socket still wasn't closed after one second.
		return ReturnRetry;
	}
}



//
// Close both the server and client sockets.  Calls shutdown first.
//
ReturnVal NetAsyncTCP::Destroy()
{
	ReturnVal client_closed, server_closed;

	client_closed = Close( CLIENT );
	server_closed = Close( SERVER );

	if ( client_closed == ReturnSuccess && server_closed == ReturnSuccess )
		return ReturnSuccess;
	else
		return ReturnError;
}



//
// Receive on client socket.
//
ReturnVal NetAsyncTCP::Receive( LPVOID buffer, DWORD bytes, LPDWORD return_value,
	LPOVERLAPPED asynchronous_io, DWORD flags )
{
	#if NETWORK_DETAILS || _DETAILS
		cout << "* Receiving " << bytes << " bytes from socket." << endl;
	#endif

	// specify the file pointer (meaningless for a socket, but must be set)
	asynchronous_io->Offset = 0;
	asynchronous_io->OffsetHigh = 0;

#if defined (_WIN32) || defined (_WIN64)
	wsa_buf.len = bytes;
	wsa_buf.buf = (char*)buffer;

	// Do the read.
	if ( WSARecv( (client_socket), &wsa_buf, 1, return_value, &flags, 
		asynchronous_io, NULL ) == 0 )
#else
	if ( ReadFile( client_socket, buffer, bytes, return_value, asynchronous_io ) )
#endif
	{
		// Read succeeded.
		#if NETWORK_DETAILS || _DETAILS
			cout << "* Received " << *return_value << " of " << bytes 
				<< " bytes from socket." << endl;
		#endif
		return ReturnSuccess;
	}

	// See if the read was queued.
	if ( WSAGetLastError() == WSA_IO_PENDING )
	{
		#if NETWORK_DETAILS || _DETAILS
			cout << "* Queued receive for " << *return_value << " of " 
				 << bytes << " bytes from socket." << endl;
		#endif
		return ReturnPending;
	}

	// Read failed!
	*return_value = WSAGetLastError();
	#if NETWORK_DETAILS || _DETAILS
		*errmsg << "*** Error " << *return_value << " receiving " 
				<< bytes << " bytes from socket." << ends;
		OutputErrMsg();
	#endif
	return ReturnError;
}



//
// Send on client socket.
//
ReturnVal NetAsyncTCP::Send( LPVOID buffer, DWORD bytes, LPDWORD return_value,
	LPOVERLAPPED asynchronous_io, DWORD flags )
{
	#if NETWORK_DETAILS || _DETAILS
		cout << "* Sending " << bytes << " bytes to socket." << endl;
	#endif

	// specify the file pointer (meaningless for a socket, but must be set)
	asynchronous_io->Offset = 0;
	asynchronous_io->OffsetHigh = 0;

#if defined (_WIN32) || defined (_WIN64)
	wsa_buf.len = bytes;
	wsa_buf.buf = (char*)buffer;

	// Do the write.
	if ( WSASend( (client_socket), &wsa_buf, 1, return_value, NULL, 
			asynchronous_io, NULL ) == 0 )
#else
	if ( WriteFile( client_socket, buffer, bytes, return_value, asynchronous_io ) )
#endif
	{
		// Write succeeded.
		#if NETWORK_DETAILS || _DETAILS
			cout << "* Sent " << *return_value << " of " << bytes 
				<< " bytes to socket." << endl;
		#endif
		return ReturnSuccess;
	}

	// See if the write was queued.
	if ( WSAGetLastError() == WSA_IO_PENDING )
	{
		#if NETWORK_DETAILS || _DETAILS
			cout << "* Queued send for " << *return_value << " of " 
				 << bytes << " bytes to socket." << endl;
		#endif
		return ReturnPending;
	}

	// Write failed.
	*return_value = WSAGetLastError();
	#if NETWORK_DETAILS || _DETAILS
		*errmsg << "*** Error " << *return_value << " sending " 
				<< bytes << " bytes to socket." << ends;
		OutputErrMsg();
	#endif
	return ReturnError;
}



//
// Non blocking call which reads data from a connected port without
// removing it.  Note that a process does not receive data from itself.
// Returns the number of bytes available to be read.
//
// **** This function has not been tested.
DWORD NetAsyncTCP::Peek()
{
	DWORD	bytes_available = 0;

	// we have to provide a buffer, so we provide just one character of buffer
	char buf[1];

#if defined (_WIN32) || defined (_WIN64)
	wsa_buf.buf = buf;
	wsa_buf.len = sizeof(buf);

	if ( Receive( buf, 1, &bytes_available, NULL, MSG_PEEK ) == ReturnSuccess )
#else
	if ( recv ( ((struct File *)client_socket)->fd, buf, sizeof(buf), MSG_PEEK ) == ReturnSuccess )
#endif
	{
		#if NETWORK_DETAILS || _DETAILS
			cout << "* Peeked " << bytes_available 
				<< " bytes from socket." << endl;
		#endif
		return (DWORD) bytes_available;
	}
	else
	{
		#if NETWORK_DETAILS || _DEBUG
			*errmsg << "*** Error " << WSAGetLastError() 
					<< " peeking from socket in NetAsyncTCP::Peek()." << ends;
			OutputErrMsg();
		#endif
		return 0;	// no data available at this time, maybe later
	}
}



//
// Utility function to close a socket.
//
ReturnVal NetAsyncTCP::CloseSocket( SOCKET *s )
{
	#if NETWORK_DETAILS || _DETAILS
		cout << "Closing socket." << endl;
	#endif

#if defined (_WIN32) || defined (_WIN64)
	if ( *s == INVALID_SOCKET )
#else // UNIX
	if ( ((struct File *)*s)->fd == (int)INVALID_SOCKET )
#endif
	{
		#if NETWORK_DETAILS || _DETAILS
			cout << "Socket is already closed." << endl;
		#endif
		return ReturnSuccess;
	}

	#if NETWORK_DETAILS || _DEBUG
		cout << "Closing socket." << endl;
	#endif

#if defined (_WIN32) || defined (_WIN64)
	if ( closesocket ( *s ) != 0 )
#else
	WSASetLastError(0);
	if ( close ( ((struct File *)*s)->fd ) != 0 )
#endif
	{
		*errmsg << "*** Error " << WSAGetLastError() 
				<< " closing socket in NetAsyncTCP::CloseSocket()." << ends;
		OutputErrMsg();
		return ReturnError;
	}

#if defined (_WIN32) || defined (_WIN64)
	*s = INVALID_SOCKET;
#else
	((struct File *)*s)->fd = (int)INVALID_SOCKET;
#endif

	return ReturnSuccess;
}



//
// Perform an abortive close of the specified socket.
//
ReturnVal NetAsyncTCP::Close( BOOL close_server )
{
	SOCKET *s;

	if ( close_server )
	{
		s = &server_socket;
#if defined(UNIX) && defined(WORKAROUND_LISTEN_BUG)
		listening = FALSE;
#endif // UNIX && WORKAROUND_LISTEN_BUG
	}
	else
		s = &client_socket;

	return CloseSocket( s );
}



//
// Set socket options.
//
void NetAsyncTCP::SetOptions( SOCKET *s )
{
	static BOOL setoption = TRUE;
	LINGER lstruct = {TRUE, 0};

	
#if (NETWORK_DETAILS || _DEBUG) && !defined(LINUX)
	BOOL CHECK_setoption;
	LINGER	CHECK_lstruct;
	int size;
#endif


	// SET the socket option settings
	///////////////////////////////////
#ifdef UNIX
	struct File *fp = (struct File *) *s;
	// When closing the connection, do a hard close.
	if ( setsockopt( fp->fd, SOL_SOCKET, SO_LINGER, (char *) &lstruct, sizeof(lstruct) ) == SOCKET_ERROR )
		cout << "*** Couldn't set SO_LINGER option" << endl;
	// Reuse socket addresses
	if ( setsockopt( fp->fd, SOL_SOCKET, SO_REUSEADDR, (char *) &setoption, sizeof(setoption) ) == SOCKET_ERROR )
		cout << "*** Couldn't set SO_REUSEADDR option" << endl;
	// Don't delay sending data.
	if ( setsockopt( fp->fd, IPPROTO_TCP, TCP_NODELAY, (char *) &setoption, sizeof(setoption) ) == SOCKET_ERROR )
		cout << "*** Couldn't set TCP_NODELAY option" << endl;
#else
	// When closing the connection, do a hard close.
	if ( setsockopt( *s, SOL_SOCKET, SO_LINGER, (char *) &lstruct, sizeof(lstruct) ) == SOCKET_ERROR )
		cout << "*** Couldn't set SO_LINGER option" << endl;
	// Reuse socket addresses
	if ( setsockopt( *s, SOL_SOCKET, SO_REUSEADDR, (char *) &setoption, sizeof(setoption) ) == SOCKET_ERROR )
		cout << "*** Couldn't set SO_REUSEADDR option" << endl;
	// Don't delay sending data.
	if ( setsockopt( *s, IPPROTO_TCP, TCP_NODELAY, (char *) &setoption, sizeof(setoption) ) == SOCKET_ERROR )
		cout << "*** Couldn't set TCP_NODELAY option" << endl;
#endif

	// CHECK the socket option settings
	/////////////////////////////////////

#if defined (_WIN32) || defined (_WIN64)
	#if NETWORK_DETAILS || _DEBUG
		// When closing the connection, do a hard close.
		size = sizeof(CHECK_lstruct);
		if ( getsockopt( *s, SOL_SOCKET, SO_LINGER, (char *) &CHECK_lstruct, &size ) == SOCKET_ERROR )
			cout << "*** Couldn't get SO_LINGER option" << endl;
		else if ( memcmp( &lstruct, &CHECK_lstruct, sizeof(lstruct) ) )
			cout << "*** SO_LINGER was not set correctly" << endl;

		// Reuse socket addresses
		size = sizeof(CHECK_setoption);
		if ( getsockopt( *s, SOL_SOCKET, SO_REUSEADDR, (char *) &CHECK_setoption, &size ) == SOCKET_ERROR )
			cout << "*** Couldn't get SO_REUSEADDR option" << endl;
		else if ( setoption != CHECK_setoption )
			cout << "*** SO_REUSEADDR was not set correctly" << endl;

		// Don't delay sending data.
		size = sizeof(CHECK_setoption);
		if ( getsockopt( *s, IPPROTO_TCP, TCP_NODELAY, (char *) &CHECK_setoption, &size ) == SOCKET_ERROR )
			cout << "*** Couldn't get TCP_NODELAY option" << endl;
		else if ( setoption != CHECK_setoption )
			cout << "*** TCP_NODELAY was not set correctly" << endl;
	#endif
#endif
}



//
// Handy debugger function.  Call like so:
//
// cout << "*** Error: " << Error( WSAGetLastError() ) << endl;
//
char *NetAsyncTCP::Error(int error_num)
{
	static char errmsg[2048];

	switch (error_num)
	{
#if defined (_WIN32) || defined (_WIN64)
	case WSANOTINITIALISED:
		wsprintf( errmsg, "WSANOTINITIALIZED - WinSock not initialized." );
		break;
	case WSAENETDOWN:
		wsprintf( errmsg, "WSAENETDOWN - The network subsystem has failed." );
		break;
	case WSAEADDRINUSE:
		wsprintf( errmsg, "WSAEADDRINUSE - The socket's local port number is already in use." );
		break;
	case WSAEINTR:
		wsprintf( errmsg, "WSAEINTR - The (blocking) Windows Socket 1.1 call was canceled "
					"through WSACancelBlockingCall." );
		break;
	case WSAEINPROGRESS:
		wsprintf( errmsg, "WSAEINPROGRESS - A blocking Windows Sockets 1.1 call is in progress, "
					"or the service provider is still processing a callback function." );
		break;
	case WSAEALREADY:
		wsprintf( errmsg, "WSAEALREADY - A nonblocking connect call is in progress on "
					"the specified socket." );
		break;
	case WSAEADDRNOTAVAIL:
		wsprintf( errmsg, "WSAEADDRNOTAVAIL - The remote address is not a valid address." );
		break;
	case WSAEAFNOSUPPORT:
		wsprintf( errmsg, "WSAEAFNOSUPPORT - Addresses in the specified family cannot "
					"be used with this socket." );
		break;
	case WSAECONNREFUSED:
		wsprintf( errmsg, "WSAECONNREFUSED - The attempt to connect was forcefully rejected." );
		break;
	case WSAEFAULT:
		wsprintf( errmsg, "WSAEFAULT - The name or the namelen parameter is not a "
					"valid part of the user address space, the namelen parameter "
					"is too small, or the name parameter contains incorrect "
					"address format for the associated address family." );
		break;
	case WSAEINVAL:
		wsprintf( errmsg, "WSAEINVAL - The parameter s is a listening socket, or "
					"the destination address specified is not consistent with "
					"that of the constrained group the socket belongs to." );
		break;
	case WSAEISCONN:
		wsprintf( errmsg, "WSAEISCONN - The socket is already connected." );
		break;
	case WSAENETUNREACH:
		wsprintf( errmsg, "WSAENETUNREACH - The network cannot be reached "
					"from this host at this time." );
		break;
	case WSAENOBUFS:
		wsprintf( errmsg, "WSAENOBUFS - No buffer space is available.  "
					"The socket cannot be connected." );
		break;
	case WSAENOTSOCK:
		wsprintf( errmsg, "WSAENOTSOCK - The descriptor is not a socket." );
		break;
	case WSAETIMEDOUT:
		wsprintf( errmsg, "WSAETIMEDOUT - Attempt to connect timed out without "
					"establishing a connection." );
		break;
	case WSAEWOULDBLOCK:
		wsprintf( errmsg, "WSAEWOULDBLOCK - The socket is marked as nonblocking and "
					"the connection cannot be completed immediately." );
		break;
	case WSAEACCES:
		wsprintf( errmsg, "WSAEACCES - Access forbidden." );
		break;
	case WSAECONNRESET:
		wsprintf( errmsg, "WSAECONNRESET - Connection reset by peer." );
		break;
#endif
	default:
		wsprintf( errmsg, "Unknown WinSock error: %d.", error_num );
		break;
	}

	return errmsg;
}
