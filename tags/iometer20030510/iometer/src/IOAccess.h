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
// IOAccess.h: Interface for the Access class.  This class represents
// the current access specification for a Grunt in Dynamo.  It contains an
// array of ACCESS structures, each of which defines one access spec line.
//
// This file is used by both Iometer and Dynamo.
//
//////////////////////////////////////////////////////////////////////

#ifndef ACCESS_DEFINED
#define ACCESS_DEFINED

#include "IOCommon.h"


#define NOT_POWER_OF_TWO	(DWORDLONG) -1	// a mask is set to this if it cannot be used for
											// byte alignment (because the alignment isn't a
											// power of two)

typedef struct
{
	int random;
	int read;
	DWORD size;
	int	delay;
	int burst;
	DWORD align;
	DWORD reply;
	DWORDLONG align_mask;
}	ACCESS;



struct Access_Spec
{
	int of_size;
	int reads;
	int random;
	int delay;
	int burst;
	DWORD align;
	DWORD reply;
	DWORD size;
};
#define Access_Specs Access_Spec*
#define MAX_ACCESS_SPECS	100		// Maximum number of specs in a total specification.



class Access
{
public:

	Access() { max_transfer = 0; }
	~Access() { }

	void	Initialize( const Access_Specs specs );

	void	GetNextBurst(	int			access_percent,
							int			*burst,
							DWORD		*size,
							int			*delay,
							DWORD		*align,
							DWORDLONG	*align_mask,
							DWORD		*reply );

	BOOL	Read( int access_percent, int read_percent );
	BOOL	Random( int access_percent, int random_percent );

	int		max_transfer;				// Maximum size of a transfer request for a test.

private:

	ACCESS access_grid[MAX_ACCESS_SPECS];
};



#endif
