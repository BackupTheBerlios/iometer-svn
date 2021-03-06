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
//                  Copyright (C) 1999-2000 Intel Corporation
//                          All rights reserved                               
//                INTEL CORPORATION PROPRIETARY INFORMATION                   
//    This software is supplied under the terms of a license agreement or     
//    nondisclosure agreement with Intel Corporation and may not be copied    
//    or disclosed except in accordance with the terms of that agreement.     
// ==========================================================================
//
// This is one of the CENTRAL PROJECT FILES for Iometer
// GalileoDefs.h		- GUI-wide central include file
// GalileoGlobals.cpp	- global function implementation file
//
//////////////////////////////////////////////////////////////////////


#include "stdafx.h"
#include "GalileoDefs.h"
#include "GalileoApp.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

UINT MessageBoxThread(LPVOID pErrorMessage);

//
// Reports error messages appropriately.
// Make sure this is not called while the result file is open!
//
void ErrorMessage( LPCTSTR errmsg )
{
	// If in batch mode, don't pop up error dialogs that require user intervention.

	if ( theApp.IsBatchMode() )
	{
		// Open the result file for appended output.
		ofstream outfile( theApp.cmdline.GetResultFile(), ios::app );
		outfile << "\nERROR: " << errmsg << endl;
		outfile.close();
	}
	else
	{
		//User-interactive mode: Display error message box.
		//Must do so from a separate thread, in case the "waiting for managers"
		//timeout dialog is present.  (Otherwise, the thread is blocked, and it
		//continues to count down past 0, among other bad things.)  Bug #361.

		//Create a copy of the error message string - thread must delete.
		CString		*pErrorMessage = new CString(errmsg);
		(void) AfxBeginThread(MessageBoxThread, (LPVOID) pErrorMessage);
	}
}


//Called to display error message box in a separate thread (see ErrorMessage(), above).
UINT MessageBoxThread(LPVOID pErrorMessage)
{
	AfxMessageBox(*(CString *)pErrorMessage);
	delete (CString *)pErrorMessage;
	return 0;
}
