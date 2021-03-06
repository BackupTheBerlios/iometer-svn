/* ######################################################################### */
/* ##                                                                     ## */
/* ##  (Iometer & Dynamo) / IOVersion.h                                   ## */
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
/* ##  Changes ...: 2003-05-10 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Changed the version to current date.                ## */
/* ##               2003-03-05 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Dropped not used defines like IOVER_FILE,           ## */
/* ##                 IOVER_PRODUCT, IOVER_PRIVATEBUILD and               ## */
/* ##                 IOVER_SPECIALBUILD.                                 ## */
/* ##               2003-02-15 (daniel.scheibli@edelbyte.org)             ## */
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

// Iometer has been using date stamps as the version strings, and currently
// does not have major or minor version numbers.
// The last "official" version was "2001.07.19", which is the version
// you will find in iometer-initial.tar.gz at the Sourceforge page
// (http://sourceforge.net/projects/iometer).
//#define IOVER_FILEVERSION				"2003.02.15\0"
//#define IOVER_FILEVERSION				"2003.02.15-post\0"
#define IOVER_FILEVERSION				"2003.05.10\0"
#define IOVER_PRODUCTVERSION				IOVER_FILEVERSION



#ifndef UNIX

#define IOVER_LEGALCOPYRIGHT				"Copyright � 1996-2001 Intel Corporation\0"
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
