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
// Network.cpp: Implementation of generic methods for the Network class.
// Network is a pure virtual class that is used as a base class for
// any network connection.  It does not contain any code except for the
// utility function OutputErrMsg().  
//
// Network objects are used for connections between network workers
// in Dynamo (contrast Port, which is used for communication between
// Dynamo and Iometer).  The Network class was designed for use in
// both Iometer and Dynamo, but is currently used only in Dynamo.
//
//////////////////////////////////////////////////////////////////////

#if defined (_WIN32) || defined (_WIN64)
#include <afx.h>
#endif

#include "Network.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Network::Network()
{
	errmsg = new ostrstream;
}

Network::~Network()
{
	delete errmsg;
}



//
// Utility function: output the "errmsg" message in an appropriate manner for the
// current environment (Dynamo or Iometer).
//
void Network::OutputErrMsg()
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
