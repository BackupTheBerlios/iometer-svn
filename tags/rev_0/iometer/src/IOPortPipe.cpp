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
//                  Copyright (C) 1997-2000 Intel Corporation
//                          All rights reserved                               
//                INTEL CORPORATION PROPRIETARY INFORMATION                   
//    This software is supplied under the terms of a license agreement or     
//    nondisclosure agreement with Intel Corporation and may not be copied    
//    or disclosed except in accordance with the terms of that agreement.     
// ==========================================================================
//
// IOPortPipe.cpp: Implementation of the Port class using named pipes 
// (Windows NT only).
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
#include <afx.h>
#endif
#include "IOPortPipe.h"

#define	UNKNOWN_PIPE	"\\\\.\\pipe\\dynamo"
#define	UNKNOWN_PIPE1	"\\\\"
#define	UNKNOWN_PIPE2	"\\pipe\\dynamo"


// Constructor and destructor.

PortPipe::PortPipe( BOOL synch ) 
: Port( synch )	// call Port constructor
{
	type = PORT_TYPE_PIPE;
}

PortPipe::~PortPipe()
{
	// no class-specific destruction
}


//
// Creates a port for communication.  If the port name to create is
// not given, automatically giving it a name.
//
BOOL PortPipe::Create( char* port_name, char* remote_name, DWORD size, unsigned short port_number )
{ 
	network_port = port_number; // otherwise ignored

	if ( port_name )
	{
		strcpy( name, port_name );
		if ( remote_name )
			strcpy( network_name, remote_name );
		else
			strcpy( network_name, port_name );
	}
	else
		Get_Auto_Name();

	#if PORT_DETAILS || _DETAILS
		cout << "Creating port " << name << "." << endl;
		SetLastError( 0 );
	#endif

	if ( synchronous )
	{
		p = CreateNamedPipe( name, PIPE_ACCESS_DUPLEX | FILE_FLAG_WRITE_THROUGH,
			PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT, 1, size, size, 0, NULL);
	}
	else
	{
		p = CreateNamedPipe( name, PIPE_ACCESS_DUPLEX | FILE_FLAG_WRITE_THROUGH | FILE_FLAG_OVERLAPPED,
			PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE, 1, size, size, 0, NULL);
	}

	#if PORT_DETAILS || _DETAILS
		if ( GetLastError() )
		{
			*errmsg << "*** Error " << GetLastError() << " creating port " 
					<< name << " in PortPipe::Create()." << ends;
			OutputErrMsg();
		}
	#endif

	return ( p != INVALID_HANDLE_VALUE );
}



//
// Connects to an existing port waiting to accept a connection.
// port_name is the full network pathname of a pipe ("\\.\pipe\<name>").
// port_number is ignored.
//
BOOL PortPipe::Connect( char* port_name, unsigned short port_number )
{
	strcpy( name, port_name );

	do
	{
		if ( synchronous )
		{
			p = CreateFile( name, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | 
				FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_NO_BUFFERING, NULL );
		}
		else
		{
			p = CreateFile( name, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | 
				FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 
				FILE_FLAG_NO_BUFFERING | FILE_FLAG_OVERLAPPED, NULL );
		}

		#if PORT_DETAILS || _DETAILS
			cout << "Attempting to connect to port " << name << "." << endl;
		#endif

		if ( p == INVALID_HANDLE_VALUE )
			Sleep( RETRY_DELAY );
	}
	while ( p == INVALID_HANDLE_VALUE );

	return TRUE;
}



//
// Begin accepting a connection to an existing port.  Used by server.
// For synchronous port, blocks until a connection is made (returns TRUE)
// or an error occurs (returns FALSE).  For asynchronous port, returns 
// immediately.  Returns TRUE for success or FALSE if any error occurs.
//		Call IsAcceptComplete() to determine if it has completed, 
//		and GetAcceptResult() to get result.
//
BOOL PortPipe::Accept()
{
	if ( synchronous )
	{
		#if PORT_DETAILS || _DETAILS
			cout << "Waiting to accept connection to " << name << "." << endl;
		#endif
		
		return ( ConnectNamedPipe( p, NULL ) );
	}
	else
	{
		#if PORT_DETAILS || _DETAILS
			cout << "Starting to accept connection to " << name << "." << endl;
		#endif

		if ( !InitOverlapped( &accept_overlapped ) )
		{
			return FALSE;
		}
		
		if ( ConnectNamedPipe( p, &accept_overlapped ) )
		{
			#if PORT_DETAILS || _DETAILS
				cout << "Connection accepted." << endl;
			#endif
			return TRUE;
		}
		else
		{
			if ( GetLastError() == ERROR_IO_PENDING ) // Read started OK...
			{
				return TRUE;
			}
			else
			{
				*errmsg << "*** ConnectNamedPipe() failed in PortPipe::Accept()." << ends;
				OutputErrMsg();
				return FALSE;
			}
		}
	}
}



//
// Get result of completed asynchronous Accept().  Be sure that IsAcceptComplete()
// has returned TRUE before calling this function.  Does not block.
//
BOOL PortPipe::GetAcceptResult()
{
	if ( synchronous )
	{
		return ( p != INVALID_HANDLE_VALUE );
	}
	else
	{
		DWORD tmp;	// value returned in tmp is meaningless -- not 
					// applicable to ConnectNamedPipe()

		return GetOverlappedResult( p, &accept_overlapped, &tmp, FALSE );
	}
}



//
// Called from the "server" side of a port to disconnect a "client" from
// its port.
//
BOOL PortPipe::Disconnect()
{
	#if PORT_DETAILS || _DETAILS
		cout << "Disconnecting from " << name << "." << endl;
	#endif

	FlushFileBuffers( p );
	return ( DisconnectNamedPipe( p ) );
}



//
// Closes connections to a port.  Called from the "client" side.
//
BOOL PortPipe::Close()
{
	#if PORT_DETAILS || _DETAILS
		cout << "Closing communication port " << name << "." << endl;
	#endif

	return( CloseHandle( p ) );
}



//
// Call which receives data from a connected port.  Blocking call for
// synchronous port; non-blocking call for asynchronous port.  
// Note that a receive call will not receive data from itself.
//
DWORDLONG PortPipe::Receive( LPVOID msg, DWORD size )
{
	DWORD bytes_read;
	OVERLAPPED *olap;

	if ( synchronous )
	{
		olap = NULL;
	}
	else
	{
		if ( !InitOverlapped( &receive_overlapped ) )
		{
			return PORT_ERROR;
		}
		olap = &receive_overlapped;
	}

	if ( ReadFile( p, msg, size, &bytes_read, olap ) )
	{
		#if PORT_DETAILS || _DETAILS
			cout << "Received " << bytes_read << " of " << size 
				 << " bytes from port " << name << "." << endl;
		#endif

		return (DWORDLONG) bytes_read;
	}
	else
	{
		if ( !synchronous && (GetLastError() == ERROR_IO_PENDING) ) // Asynchronous read started OK...
		{
			return 0;	// not an error code, but no bytes read yet
		}

		*errmsg << "*** Error receiving " << size << " bytes from port " 
				<< name << " in PortPipe::Receive()." << ends;
		OutputErrMsg();
		return PORT_ERROR;
	}
}

//
// Get result of completed asynchronous Receive() (number of bytes read, -1 for
// any error).  Be sure that IsReceiveComplete() has returned TRUE before 
// calling this function.  Does not block.
//
DWORDLONG PortPipe::GetReceiveResult()
{
	if ( synchronous )
	{
		return PORT_ERROR;
	}

	DWORD bytes_read;

	if ( GetOverlappedResult( p, &receive_overlapped, &bytes_read, FALSE ) )
	{
		return (DWORDLONG) bytes_read;
	}
	else
	{
		return PORT_ERROR;
	}
}



//
// Call which sends data to a connected port.  Blocking call for synchronous 
// port; non-blocking call for asynchronous port.  Note that a process will
// not send data to itself.
//
DWORDLONG PortPipe::Send( LPVOID msg, DWORD size )
{
	DWORD bytes_written;
	OVERLAPPED *olap;

	if ( synchronous )
	{
		olap = NULL;
	}
	else
	{
		if ( !InitOverlapped( &send_overlapped ) )
		{
			return PORT_ERROR;
		}
		olap = &send_overlapped;
	}

	if ( WriteFile( p, msg, size, &bytes_written, olap ) )
	{
		#if PORT_DETAILS || _DETAILS
			cout << "Sent " << bytes_written << " of " << size 
				 << " bytes to port " << name << "." << endl;
		#endif
		return (DWORDLONG) bytes_written;
	}
	else
	{
		// Did asynchronous write started OK?
		if ( !synchronous && (GetLastError() == ERROR_IO_PENDING) )
		{
			return 0;	// not an error code, but no bytes read yet
		}

		*errmsg << "*** Error " << GetLastError() << " sending " << size 
				<< " bytes to port " << name << " in PortPipe::Send()." << ends;
		OutputErrMsg();
		return PORT_ERROR;
	}
}

//
// Get result of completed asynchronous Send() (number of bytes read,
// -1 for any error).  Be sure that IsSendComplete() has returned TRUE 
// before calling this function.  Does not block.
//
DWORDLONG PortPipe::GetSendResult()
{
	if ( synchronous )
	{
		return PORT_ERROR;
	}

	DWORD bytes_written;

	if ( GetOverlappedResult( p, &send_overlapped, &bytes_written, FALSE ) )
	{
		return (DWORDLONG) bytes_written;
	}
	else
	{
		return PORT_ERROR;
	}
}


//
// Non blocking call which reads data from a connected port without
// removing it.  Note that a process does not receive data from itself.
//
DWORD PortPipe::Peek()
{
	DWORD bytes_available;
	if ( PeekNamedPipe( p, NULL, 0, NULL, NULL, &bytes_available ) )
	{
		#if PORT_DETAILS || _DETAILS
			cout << "Peeked " << bytes_available << " bytes from port " 
				 << name << "." << endl;
		#endif
		return bytes_available;
	}
	else
	{
		#if PORT_DETAILS || _DETAILS
			*errmsg << "*** PeekNamedPipe() failed  for port " 
					<< name << " in PortPipe::Peek()." << ends;
			OutputErrMsg();
		#endif
		return 0;	// no data available at this time, maybe later
	}
}

//
// Sets the local and network names for a port.
//
void PortPipe::Get_Auto_Name()
{
	static int	port_number = 0;
	DWORD		name_size = MAX_NETWORK_NAME;

	// Setting locally known port name.
	strcpy( name, UNKNOWN_PIPE );
	_itoa( port_number, name + strlen( name ), 10 );
	strcat( name, ".\0" );
	_itoa( _getpid(), name + strlen( name ), 10 );

	// Setting port name as known over the network.
	strcpy( network_name, UNKNOWN_PIPE1 );
	GetComputerName( network_name + 2, &name_size );
	strcat( network_name, UNKNOWN_PIPE2 );
	_itoa( port_number++, network_name + strlen( network_name ), 10 );
	strcat( network_name, ".\0" );
	_itoa( _getpid(), network_name + strlen( network_name ), 10 );
}
