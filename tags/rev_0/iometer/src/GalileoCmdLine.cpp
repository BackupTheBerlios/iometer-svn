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
// GalileoCmdLine.cpp: Implementation of the CGalileoCmdLine class, which
// parses Iometer's command line.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "GalileoApp.h"
#include "GalileoCmdLine.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

const int CGalileoCmdLine::DefaultTimeout = 10;
const char CGalileoCmdLine::DefaultConfigFile[] = "iometer.icf";


CGalileoCmdLine::CGalileoCmdLine() :	m_bSwitches(FALSE), m_bFail(FALSE),
										m_sConfigFile(""), m_sResultFile(""),
										m_iTimeout(-1), m_bOverrideBatch(FALSE)
{
}

CGalileoCmdLine::~CGalileoCmdLine()
{
}


//
// See help text within for an explanation of the expected parameters.
//
void CGalileoCmdLine::ParseParam( const char* pszParam, BOOL bFlag, BOOL bLast )
{
	static int param_count = 0;
	static char last_switch = 0;
	char temp_switch;

	param_count++;

	if ( m_bFail )
	{
		// If command line parsing has failed before, don't
		// try to interpret the command line further.
		return;
	}

	if ( bFlag && strlen(pszParam) != 1 )
	{
		Fail("Exactly one letter must follow a switch character.  "
			"Switch characters are \"/\" and \"-\".");
		return;
	}

	if ( pszParam[0] == '?' )
	{
		// get Syntax display string from resource file
		CString version;
		CString syntax;
		version.Format( IDS_VERSION_OUTPUT, (LPCTSTR)theApp.GetVersionString (TRUE));
		VERIFY( syntax.LoadString( IDS_CMDLINE_SYNTAX ) );
		AfxMessageBox( version + "\n\n" + syntax );

		m_bFail = TRUE;
		return;
	}

	if ( last_switch )
	{
		temp_switch = last_switch;
		last_switch = 0;

		// Previous switch expects another parameter.
		switch ( temp_switch )
		{
		// Expecting the config file.
		case 'C':
			if ( !m_sConfigFile.IsEmpty() )	// has it already been set?
			{
				Fail("Config file parameter was specified more than once.");
			}
			else if ( IsValidFilename(pszParam) )
			{
				if ( VerifyReadable(pszParam) )
					m_sConfigFile = pszParam;
				else
					m_bFail = TRUE;
			}
			else
			{
				Fail("C switch should be followed by the name of a configuration file.");
			}
			return;
		// Expecting the result file.
		case 'R':
			if ( !m_sResultFile.IsEmpty() )	// has it already been set?
			{
				Fail("Result file parameter was specified more than once.");
			}
			else if ( IsValidFilename(pszParam) )
			{
				if ( VerifyWritable(pszParam) )
					m_sResultFile = pszParam;
				else
					m_bFail = TRUE;
			}
			else
			{
				Fail("R switch should be followed by the name of the desired result file.");
			}
			return;
		// Expecting the timeout value.
		case 'T':
			if ( m_iTimeout >= 0 )	// has it already been set?
			{
				Fail("Timeout parameter was specified more than once.");
			}
			else if ( IsValidInteger(pszParam) )
			{
				m_iTimeout = atoi(pszParam);
			}
			else
			{
				Fail("T switch should be followed by an integer timeout value.");
			}
			return;
		default:
			{
				char tmpary[2] = {last_switch, 0};
				Fail("Unrecognized switch: " + (CString)tmpary + ".");
			}
			return;
		}
	}

	if ( bFlag )
	{
		m_bSwitches = TRUE;
		last_switch = toupper( pszParam[0] );

	//////////////////////////////////////////////////////////////////////
	// This is an example of how to allow switches that have meaning on
	// their own, without any additional parameters.
	//
	//	if ( last_switch == 'V' )	// spit out version number and exit
	//	{
	//		// Set BOOL member indicating that this switch was specified
	//		// Make sure it's initialized in the constructor
	//		m_bVersion = TRUE;
	//
	//		last_switch = 0;	// don't look for more parameters related to this switch
	//		return;				// don't allow it to reach the bLast checking
	//	}
	//////////////////////////////////////////////////////////////////////

		if ( bLast )
		{
			Fail("An additional parameter was expected after the last switch.");
			return;
		}

		return;
	}

	// If switches haven't been used (so far)...
	if ( !m_bSwitches )
	{
		switch (param_count)
		{
		// Expecting the config file.
		case 1:
			if ( IsValidFilename(pszParam) )
			{
				if ( VerifyReadable(pszParam) )
					m_sConfigFile = pszParam;
				else
					m_bFail = TRUE;
			}
			else
			{
				Fail("First parameter should be the name of a valid config file.");
			}
			return;
		// Expecting the result file.
		case 2:
			if ( IsValidFilename(pszParam) )
			{
				if ( VerifyWritable(pszParam) )
					m_sResultFile = pszParam;
				else
					m_bFail = TRUE;
			}
			else
			{
				Fail("Second parameter should be the name of the result file.");
			}
			return;
		// Expecting the timeout value.
		case 3:
			if ( IsValidInteger(pszParam) )
			{
				m_iTimeout = atoi(pszParam);
			}
			else
			{
				Fail("Third parameter should be an integer timeout value.");
			}
			return;
		default:
			Fail("Too many parameters.");
			return;
		}
	}

	Fail("Didn't know what to do with this parameter:\n"
		 + (CString)pszParam + "\nPlease report this as an Iometer bug.");
}


//
// Set all member variables to the Fail state.
//
void CGalileoCmdLine::Fail( const CString& errmsg )
{
	m_bFail = TRUE;
	m_sConfigFile = "";
	m_sResultFile = "";
	m_iTimeout = -1;
	m_bSwitches = FALSE;
	ErrorMessage("Error processing the command line.  " + errmsg);
}


CString CGalileoCmdLine::GetConfigFile()
{
	return m_sConfigFile;
}


CString CGalileoCmdLine::GetResultFile()
{
	return m_sResultFile;
}


int CGalileoCmdLine::GetTimeout()
{
	if ( m_iTimeout > 0 )
		return m_iTimeout;
	else
		return CGalileoCmdLine::DefaultTimeout;
}


//
// Is Iometer in batch mode?
//
BOOL CGalileoCmdLine::IsBatchMode()
{
	return !m_bOverrideBatch && !m_sConfigFile.IsEmpty() && !m_sResultFile.IsEmpty();
}


//
// Take the application out of batch mode.
// No harm will be done if Iometer is already out of batch mode.
// (Forces IsBatchMode() to always return FALSE)
//
void CGalileoCmdLine::OverrideBatchMode()
{
	m_bOverrideBatch = TRUE;
}


//
// See if every character in the filename is valid as part of an integer.
//
BOOL CGalileoCmdLine::IsValidInteger( const CString& instring )
{
	const CString legal = "1234567890";

	return instring.GetLength() == instring.SpanIncluding(legal).GetLength();
}



//
// See if every character in the filename is a valid filename character.
//
BOOL CGalileoCmdLine::IsValidFilename( const CString& instring )
{
	const CString legal = "abcdefghijklmnopqrstuvwxyz1234567890_-=+!&%@#$.,;:'[]{}()\\ ";
	CString teststring = instring;

	teststring.MakeLower();

	return instring.GetLength() == teststring.SpanIncluding(legal).GetLength();
}


//
// See if the specified string is the name of a writable file.
// If it isn't, set it to an empty string, report error, and return FALSE.
//
BOOL CGalileoCmdLine::VerifyWritable( const CString& filename )
{
	ofstream outfile( filename, ios::app );

	if ( outfile.is_open() )
	{
		// Good - file is open and writable.
		outfile.close();
		return TRUE;
	}
	else
	{
		// Bad - file is not writable.
		outfile.close();
		AfxMessageBox("Cannot write to file:\n" + filename );
		return FALSE;
	}
}


//
// See if the specified string is the name of a readable file.
// If it isn't, set it to an empty string, report error, and return FALSE.
//
BOOL CGalileoCmdLine::VerifyReadable( const CString& filename )
{
	ifstream infile( filename );

	if ( infile.is_open() && !infile.rdstate() )
	{
		// Good - file is open and readable.
		infile.close();
		return TRUE;
	}
	else
	{
		// Bad - file is not readable.
		infile.close();
		AfxMessageBox("Cannot read from file:\n" + filename );
		return FALSE;
	}
}
