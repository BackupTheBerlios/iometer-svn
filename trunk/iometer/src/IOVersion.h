/* ######################################################################### */
/* ##                                                                     ## */
/* ##  (Iometer & Dynamo) / IOVerion.h                                    ## */
/* ##                                                                     ## */
/* ## ------------------------------------------------------------------- ## */
/* ##                                                                     ## */
/* ##  Job .......: This is a central headerfile for Iometer as well      ## */
/* ##               as Dynamo which covers the versioning stuff.          ## */
/* ##                                                                     ## */
/* ## ------------------------------------------------------------------- ## */
/* ##                                                                     ## */
/* ##  Remarks ...: <none>                                                ## */
/* ##                                                                     ## */
/* ## ------------------------------------------------------------------- ## */
/* ##                                                                     ## */
/* ##  Changes ...: 2003-02-15 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Changed the version to current date.                ## */
/* ##               2003-02-08 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Changed the version to current date.                ## */
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
// IOVersion.h: Iometer VersionInfo resource value definition file
// (included in .RC2 file).
//
// This file is used by both Iometer and Dynamo.
//
//////////////////////////////////////////////////////////////////////
/* ######################################################################### */

#ifndef _IOVERSION
#define _IOVERSION

#ifndef UNIX
#include <winver.h>
#endif /* !UNIX */


// The maximum size allowed for the version string.
#define MAX_VERSION_LENGTH 80


// Note that the DEBUG version should have an identical version string as the release version,
// with an added " DEBUG" - the Dynamo/Iometer version comparison in CGalileoApp::OnIdle() ignores
// the " DEBUG" qualifier when verifying that the two programs are the same version.
#ifdef _DEBUG
#define		VERSION_DEBUG	" DEBUG"
#else
#define		VERSION_DEBUG	""
#endif


#if defined (__cplusplus)
extern "C"
{
#endif

//////////
//Intel HF standard (code name "Hollywood") specifies four segments:
//	M.mx.yy.zzzz
//	 (M = Major version #
//	  m = minor version #
//	  x = maintenance release #
//	  yy = patch release #
//	  zzzz = build #
//////////

//////////
//	Iometer has been using date stamps as the version strings, and currently
//	does not have major or minor version numbers:
//	(If I were to guess on the version number)
//		5.6.97 (1.00)
//			6/11/97, 7.22.97,
//		8.20.97 (1.01)
//		10.2.97 (1.02)
//			12.11.97
//		1998.01.05 (1.10) - name changes
//			1998.04.02, 1998.04.16, 1998.06.15, 1998.06.24, 1998.08.01,
//			1998.08.24
//		1998.09.01 (1.20) - VI support
//			1998.09.25 (1.18), 1998.10.02
//		1998.10.08 (1.21) - Solaris support
//			1998.11.24, 1999.01.11, 1999.02.17, 1999.03.02, 1999.03.08,
//			1999.06.15, 1999.06.18
//		1999.08.02 (1.30) - Test configuration save/restor, batch mode
//			1999.09.23, 1999.10.02
//		2000.12.07 - Added support for Win64
//		2000.12.28 - Recompile with correct SDK include files
//		2000.04.05 - Build with 2462 SDK
//				   - Removed occurances of "this->" from AccessDialog.cpp
//					 because 2462 SDK compiler didn't like it.  Isn't really
//					 needed anyway so shouldn't make a difference...
//		2000.07.19 - Build with XP RC1 SDK.
//				   - Added in new IOCQAIO.cpp and IOPerformance.cpp that had been
//				     put there by somebody else...
//////////

//per HF Guidelines, product version = file version
//#define IOVER_FILE						1,30,0, 0001
#define IOVER_FILE						2001,07,19,0
#define IOVER_PRODUCT					IOVER_FILE
//#define IOVER_FILEVERSION				"1.30.0.0001\0"

// The last "official" version was "2001.07.19", which is the
// version you will find in iometer-initial.tar.gz at the
// Sourceforge page (http://sourceforge.net/projects/iometer).
// #define IOVER_FILEVERSION				"2001.07.19\0"
#define IOVER_FILEVERSION				"2003.02.15\0"
#define IOVER_PRODUCTVERSION			IOVER_FILEVERSION


#ifndef UNIX

//If this is to be enabled, FILEFLAGS, VS_FF_PRIVATEBUILD flag must be set.
//#define IOVER_PRIVATEBUILD				"1.30.0.0001\0"
#define IOVER_PRIVATEBUILD				"\0"
//If this is to be enabled, FILEFLAGS, VS_FF_SPECIALBUILD flag must be set.

#if defined (_WIN64)
#define IOVER_SPECIALBUILD				"Built using 64-bit Windows SDK\0"
#else
#define IOVER_SPECIALBUILD				"\0"
#endif // _WIN64

#define IOVER_LEGALCOPYRIGHT				"Copyright © 1996-2001 Intel Corporation\0"
//#define IOVER_LEGALTRADEMARKS			"??? Iometer is a Trademark of Intel Corporation\0"
#define IOVER_LEGALTRADEMARKS				"\0"
#define IOVER_COMPANYNAME					"Intel Corporation\0"

#ifdef _DEBUG
#if defined (_WIN64)
#define IOVER_PRODUCT_NAME			"Iometer 64-bit (DEBUG)\0"
#else
#define IOVER_PRODUCT_NAME			"Iometer (DEBUG)\0"
#endif // _WIN64
#else
#if defined (_WIN64)
#define IOVER_PRODUCT_NAME			"Iometer 64-bit\0"
#else
#define IOVER_PRODUCT_NAME			"Iometer\0"
#endif // _WIN64
#endif // _DEBUG

////////
// The following are based on unique (for each component) resource variables
// defined in Project|Resources tab, Preprocessor definitions field.
////////

#if defined(_GALILEO_)
#define IOVER_FILETYPE				VFT_APP
#define IOVER_INTERNAL_NAME			"Galileo\0"
#define IOVER_ORIGINAL_FILE_NAME	"Iometer.exe\0"
#if defined (_WIN64)
#define IOVER_FILE_DESCRIPTION		"Iometer Control/GUI (64-bit)\0"
#else
#define IOVER_FILE_DESCRIPTION		"Iometer Control/GUI\0"
#endif // _WIN64
#define IOVER_COMMENTS				"\0"
#endif

#if defined(_PULSAR_)
#define IOVER_FILETYPE				VFT_APP
#define IOVER_INTERNAL_NAME			"Pulsar\0"
#define IOVER_ORIGINAL_FILE_NAME	"Dynamo.exe\0"
#if defined (_WIN64)
#define IOVER_FILE_DESCRIPTION		"Iometer Workload Generator (64-bit)\0"
#else
#define IOVER_FILE_DESCRIPTION		"Iometer Workload Generator\0"
#endif // _WIN64
#define IOVER_COMMENTS				"\0"
#endif

#endif /* !UNIX */


#if defined (__cplusplus)
}
#endif

#endif // _IOVERSION
