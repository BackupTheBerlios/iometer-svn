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
// ICF_ifstream.h: Interface for the ICF_ifstream class, which extends
// the standard "ifstream" class with additional functionality for reading 
// and writing Iometer's .ICF (Iometer Configuration File) files.
//
// ICF_ifstream should be used to read Iometer configuration files (ICFs).
// Once a config file is opened, the calling function should...
//
//	version = GetVersion(); if version information is desired
//	SkipTo("'TEST SETUP"); or some other section...  the string search
//		is case-insensitive, and looks for a left-side match (extra
//		characters, if present, are ignored in the comparison).
//		WARNING:  Don't look for a MANAGER comment, for example, if
//		you may first encounter a MANAGERLIST in the file -- this will
//		be considered a match!
//	GetPair(key, value); and handle key, value pairs until key is the
//		end-of-section marker, like "'End TEST SETUP"
//
// (Make sure all return values are checked and handled appropriately.)
//
//////////////////////////////////////////////////////////////////////

#ifndef ICF_IFSTREAM_DEFINED
#define ICF_IFSTREAM_DEFINED

#include <fstream>
using namespace std;


#define MAX_ICF_LINE 200


class ICF_ifstream : public ifstream
{
public:
	ICF_ifstream()							: ifstream() { }
	ICF_ifstream(const CString& infilename)	: ifstream(infilename) { }

	long	GetVersion();
	BOOL	SkipTo(CString identifier);
	CString	GetNextLine();
	BOOL	GetPair(CString& key, CString& value);

	static BOOL		ExtractFirstInt(CString& string, int& number);
	static CString	ExtractFirstToken(CString& string, BOOL spaces=FALSE);
};

#endif
