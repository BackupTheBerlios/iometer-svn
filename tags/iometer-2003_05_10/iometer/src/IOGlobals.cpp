/* ######################################################################### */
/* ##                                                                     ## */
/* ##  (IOmeter & Dynamo) / IOGrunt.cpp                                   ## */
/* ##                                                                     ## */
/* ## ------------------------------------------------------------------- ## */
/* ##                                                                     ## */
/* ##  Job .......: Implements a global function to get version string    ## */
/* ##                                                                     ## */
/* ## ------------------------------------------------------------------- ## */
/* ##                                                                     ## */
/* ##  Remarks ...: <none>                                                ## */
/* ## ------------------------------------------------------------------- ## */
/* ##                                                                     ## */
/* ##  Changes ...: 2003-04-25 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Updated the global debug flag (_DEBUG) handling     ## */
/* ##                 of the source file (check for platform etc.).       ## */
/* ##               2003-03-04 (joe@eiler.net)                            ## */
/* ##               - Added #ifdef for Solaris support                    ## */
/* ##               - Added new header holding the changelog.             ## */
/* ##                                                                     ## */
/* ######################################################################### */
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
//                Copyright (C) 1999-2000 Intel Corporation
//                          All rights reserved                               
//                INTEL CORPORATION PROPRIETARY INFORMATION                   
//    This software is supplied under the terms of a license agreement or     
//    nondisclosure agreement with Intel Corporation and may not be copied    
//    or disclosed except in accordance with the terms of that agreement.     
// ==========================================================================
//
// This is one of the CENTRAL PROJECT FILES
// IOCommon.h			- Product-wide (Iometer & Dynamo) central include file
// IOGlobals.cpp		- Product-wide global function implementation file
//
// This file is used by both Iometer and Dynamo.
//
//////////////////////////////////////////////////////////////////////


#if defined (_WIN32) || defined (_WIN64)
#include "stdafx.h"
#endif

#include "IOCommon.h"


// Needed for MFC Library support for assisting in finding memory leaks
//
// NOTE: Based on the documentation[1] I found, it should be enough to have
//       a "#define new DEBUG_NEW" statement for the case, that we are
//       running Windows. There should be no need for checking the _DEBUG
//       flag and no need for redefiniting the THIS_FILE string. Maybe there
//       will be a MFC hacker who could advice here.
//       [1] = http://msdn.microsoft.com/library/default.asp?url=/library/en-us/vclib/html/_mfc_debug_new.asp
//
#if defined (_WIN32) || defined (_WIN64)
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif


/////////////////////////////////////////////////////////////////////////////
//	Function Name:
//		GetAppFileVersionString
//
//	Purpose:
//		Obtain file version string (standard one as well as the on with DEBUG indicator).
//
//	Syntax:
//		void	GetAppFileVersionString(char **ppStrStandard, char **ppStrWithDebug)
//	
//	Parameters:
//		ppStrStandard	Ptr (must not be NULL) to a variable where the newly
//						created string pointer can be returned in.
//		ppStrWithDebug	Ptr (must not be NULL) to a variable where the newly
//						created string with debug indicator can be returned in.
//			(caller owns these strings, and is responsible for its cleanup).
//
//	Processing:
//		1. Get file version string of the app module.
//		2. If unable to do so or UNIX case, use the constant value IOVER_FILEVERSION.
//		3. Form 'with debug indicator' string by concatenating VERSION_DEBUG value.
//
//	Notes:
//		If a DLL calls this function, its EXE module's version info will be
//		returned.  If the DLL's version info needs to be returned, 'NULL'
//		parameter in the GetModuleFileName() call can be replaced by 
//		'AfxGetInstanceHandle()'.
/////////////////////////////////////////////////////////////////////////////
void	GetAppFileVersionString(char **ppStrStandard, char **ppStrWithDebug)
{
	char   *pStrStandard = NULL;

#ifndef UNIX
	ASSERT((ppStrStandard != NULL) && (ppStrWithDebug != NULL));

	//Get app(EXE module)'s file path
	char modulePathBuff[MAX_PATH];
	if (::GetModuleFileName(NULL, modulePathBuff, sizeof(modulePathBuff)) != 0)
	{
		DWORD	dwHandle=0;		//not used
		DWORD	dwVerInfoSize = ::GetFileVersionInfoSize(modulePathBuff, &dwHandle);
		//The module provides VersionInfo data
		if (dwVerInfoSize != 0)
		{
			//ptr to buffer to hold version information
			char   *pVerInfo = new char[dwVerInfoSize];
			if (::GetFileVersionInfo(modulePathBuff, NULL, dwVerInfoSize, pVerInfo))
			{
				UINT	dataLen;		//receives size of fixed data area (not used)
				char   *pFileVersion;
				//04b0 == 1200 codepage denoting Unicode used for win95 and NT resource dlls (see Iometer.rc2)
				if (::VerQueryValue(pVerInfo, TEXT("\\StringFileInfo\\040904b0\\FileVersion"),
									 (void **)&pFileVersion, &dataLen))
				{
					pStrStandard = new char[strlen(pFileVersion) + 1];
					strcpy(pStrStandard, pFileVersion);
				}
			}
			delete [] pVerInfo;
		}
		//It should have VersionInfo!
		else
			ASSERT(0);
	}
#endif /* !UNIX */

	//UNIX or last resort, use product-wide constant
	if (pStrStandard == NULL)
	{
		pStrStandard = new char[strlen(IOVER_FILEVERSION) + 1];
		strcpy(pStrStandard, IOVER_FILEVERSION);
	}

	*ppStrStandard = pStrStandard;
	*ppStrWithDebug = new char[strlen(pStrStandard) + strlen(VERSION_DEBUG) + 1];
	strcpy(*ppStrWithDebug, pStrStandard);
	strcat(*ppStrWithDebug, VERSION_DEBUG);
}

