/* ######################################################################### */
/* ##                                                                     ## */
/* ##  Dynamo / Pulsar.cpp                                                ## */
/* ##                                                                     ## */
/* ## ------------------------------------------------------------------- ## */
/* ##                                                                     ## */
/* ##  Job .......: This is file contains is basically the interface      ## */
/* ##               for dynamo.  It handles parameter parsing, usage      ## */
/* ##               info and the actual main() method.                    ## */
/* ##                                                                     ## */
/* ## ------------------------------------------------------------------- ## */
/* ##                                                                     ## */
/* ##  Remarks ...: <none>                                                ## */
/* ##                                                                     ## */
/* ## ------------------------------------------------------------------- ## */
/* ##                                                                     ## */
/* ##  Changes ...: 2003-02-26 (joe@eiler.net)                            ## */
/* ##               - Added a command line option for passing the         ## */
/* ##                 list of filesystem types to ignore when building    ## */
/* ##                 the device list for Iometer).                       ## */
/* ##               2003-02-08 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Addition command line option for passing an         ## */
/* ##                 alternative network name to Iometer (which that     ## */
/* ##                 one is using to communicate with this Dynamo).      ## */
/* ##               2003-02-02 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Applied the iometer-initial-datatypes.patch file.   ## */
/* ##                 (changing the datatype of the "temp" variable in    ## */
/* ##                 the GetStatus(int*, int*, int) method).             ## */
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
//                Copyright (C) 1997-2000 Intel Corporation
//                          All rights reserved                               
//                INTEL CORPORATION PROPRIETARY INFORMATION                   
//    This software is supplied under the terms of a license agreement or     
//    nondisclosure agreement with Intel Corporation and may not be copied    
//    or disclosed except in accordance with the terms of that agreement.     
// ==========================================================================
//
// Pulsar.cpp: Main file for Dynamo.
//
//////////////////////////////////////////////////////////////////////
/* ######################################################################### */

#include "IOCommon.h"
#include "IOManager.h"
#ifdef UNIX
#include <sys/resource.h>
#include <ctype.h>
#endif
#ifdef SOLARIS
#include <synch.h>
#endif

/////////////////////////////////////////////////////////////////////////////
// Forward declarations
/////////////////////////////////////////////////////////////////////////////
static void Syntax( const char* errmsg = NULL );

static void ParseParam(	const char* pszParam,
                        BOOL bLast,
			char iometer[MAX_NETWORK_NAME],
			char manager_name[MAX_WORKER_NAME],
			char manager_computer_name[MAX_NETWORK_NAME],
			char manager_exclude_fs[MAX_EXCLUDE_FILESYS] );
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

// Temporary global data for Syntax() to use
const char *g_pVersionStringWithDebug = NULL;

int main( int argc, char *argv[] )
{
	Manager	manager;
	char	iometer[MAX_NETWORK_NAME];
#ifdef LINUX
	struct aioinit aioDefaults;

	memset(&aioDefaults, 0, sizeof(aioDefaults));
	aioDefaults.aio_threads = 2;
	aioDefaults.aio_threads = 2;
	aio_init(&aioDefaults);
#endif

	iometer[0] = 0;
	manager.manager_name[0] = 0;
	manager.exclude_filesys[0] = 0;

	//provide a temporary global ptr to the version string for Syntax() to use
	g_pVersionStringWithDebug = manager.GetVersionString(TRUE);
	for (int counter=1; counter<argc; counter++)
		ParseParam(argv[counter],
	                               counter==argc-1,
	                               iometer,
	                               manager.manager_name,
	                               manager.prt->network_name,
	                               manager.exclude_filesys
	                               );

	g_pVersionStringWithDebug = NULL;	//should use manager object after this...

	// If there were command line parameters, indicate that they were recognized.
	if ( iometer[0] || manager.manager_name[0] )
	{
		cout << "\nCommand line parameter(s):" << endl;

		if ( iometer[0] )
			cout << "   Looking for Iometer on \"" << iometer << "\"" << endl;
		if ( manager.manager_name[0] )
			cout << "   New manager name is \"" << manager.manager_name << "\"" << endl;
	}
	if ( manager.exclude_filesys[0] )
        {
        cout << "\nExcluding the following filesystem types:" << endl;
        cout << "   \"" << manager.exclude_filesys << "\"" << endl;
        }
        else
        {
        strcpy(manager.exclude_filesys, DEFAULT_EXCLUDE_FILESYS);
         }
	cout << endl;

#ifdef UNIX
#ifdef LINUX
	signal(SIGALRM, SIG_IGN);
#else
	sigignore(SIGALRM);
#endif

	// Initialize the lock on UNIX platforms.
	if (pthread_mutex_init(&lock_mt, NULL))
		{
			cout <<"unable to init the lock" << endl;
			exit(1);
		}

	// Block SIGPIPE signal. Needed to ensure that Network worker
	// threads don't exit due to a broken pipe signal.
	sigset_t sigset;
	sigemptyset(&sigset);
	sigaddset(&sigset, SIGPIPE);
	if (sigprocmask(SIG_BLOCK, &sigset, NULL) < 0)
	{
		cout << "sigprocmask() call failed." << endl;
		cout << "dynamo could be unstable" << endl;
	}

	//
	// the number of file descriptors a process may create can be a small value like 64.
	//
	struct rlimit rlimitp;
	if (getrlimit(RLIMIT_NOFILE, &rlimitp) < 0)
	{
		cout << "error " << errno << " trying to get rlimit (# file descriptors)" << endl;
	}
	else
	{
		// it succeeded. We leave out atleast 25 file descriptors for non-targets
		// and compare with the hard limit.
		unsigned int targets = MAX_TARGETS + 25;
		if ( rlimitp.rlim_max < targets )
		{
			cout << "Only " << rlimitp.rlim_max << " file descriptors available" << endl;
			rlimitp.rlim_cur = rlimitp.rlim_max;
		}
		else
		{
			// set the soft limit to the required value.
			rlimitp.rlim_cur = targets;
		}
		if (setrlimit(RLIMIT_NOFILE, &rlimitp) < 0)
		{
			cout << "error " << errno << " trying to set rlimit (# file descriptors)" << endl;
		}
	}

	// Check for super-user permissions. If not super-user, we
	// cannot get many of the info from the kernel.
	if (getuid() || geteuid())
	{
		cout << "Dynamo not running as super-user." << endl;
		cout << "       All available disks might not be reported " << endl;
		cout << "       Cannot get TCP statistics from the kernel " << endl;
	}

#ifdef DYNAMO_DESTRUCTIVE
	// No command line args specifies destructive testing. Check to see if there
	// are any environment variables specifying the same. We need to warn the user.
	if (getenv("DYNAMO_DESTRUCTIVE") != NULL)
	{
		cout << "        ************ WARNING ************" << endl;
		cout << "       dynamo running in Destructive mode." << endl;
		cout << "        ************ WARNING ************" << endl;
	}
#endif // DYNAMO_DESTRUCTIVE
#endif // UNIX

	// Entering infinite loop to allow Dynamo to run multiple tests.  Outer while loop allows
	// Dynamo to be reset from Iometer.  If everything works smoothly, resets should be rare.
	while (TRUE)
	{
		// Initializing worker and logging into Iometer director.
		if ( !manager.Login( iometer ) )
			break;

		// Manager will continue to run until an error, or stopped by Iometer.
		if ( !manager.Run() )
			break;		// Stop running when the manager is done.
	}
	cout << "Ending execution." << endl;
	Sleep( 1000 );
	return(0);
}


//
// Show command line syntax to the user.
//
void Syntax( const char* errmsg /*=NULL*/ )
{
	if ( errmsg )
	{
		cout << endl
			 << "*** Error processing the command line." << endl;
		cout << "*** " << errmsg << endl;
	}

	cout << endl;
	cout << "Version " << g_pVersionStringWithDebug << endl;
	cout << endl;
	cout << "SYNTAX" << endl;
	cout << endl;
#ifndef SOLARIS
	cout << "dynamo /?" << endl;
#else
	// Solaris 2.7 must have the switch (? is used for its own purpose).
	cout << "dynamo \\?" << endl;
#endif
	cout << "dynamo [iometer_computer_name [manager_name [manager_network_name]]]" << endl;
	cout << "dynamo [/i iometer_computer_name] [/n manager_name] [/m manager_network_name]" << endl;
	cout << "       [/x excluded_fs_type]" << endl;
	cout << endl;
	cout << "   ? - show Dynamo version number and command line syntax" << endl;
	cout << endl;
	cout << "   iometer_computer_name - the name of the computer running Iometer" << endl;
	cout << "      This is only needed if Dynamo and Iometer are running on different" << endl;
	cout << "      computers.  Without this parameter, Dynamo will search for Iometer" << endl;
	cout << "      on the local host." << endl;
	cout << endl;
	cout << "   manager_name - the name of this Dynamo" << endl;
	cout << "      This name is the one Iometer will use as the manager name, important" << endl;
	cout << "      when restoring config files.  This defaults to the host's name." << endl;
	cout << endl;
	cout << "   manager_network_name - the network name of this Dynamo" << endl;
	cout << "      This name or IP address is the one Iometer will use to communicate" << endl;
	cout << "      with this manager. The default is the IP adress of the host's first" << endl;
	cout << "      NIC." << endl;
	cout << endl;
	cout << "   excluded_fs_type - type of filesystems to exclude from device search" << endl;
         cout << "      This string should contian the filesystem types that are not reported" << endl;
         cout << "      to Iometer. The default is \"" << DEFAULT_EXCLUDE_FILESYS << "\"." << endl;
         cout << endl;

	
	exit( 0 );
}



void ParseParam( const char* pszParam,
                 BOOL bLast,
		 char iometer[MAX_NETWORK_NAME],
		 char manager_name[MAX_WORKER_NAME],
		 char manager_computer_name[MAX_NETWORK_NAME],
                 char manager_exclude_fs[MAX_EXCLUDE_FILESYS] )

{
	static int param_count = 0;
	static BOOL switches = FALSE;
	static char last_switch = 0;
	char temp_switch;

	param_count++;

	// See if the user is requesting syntax help.
	if ( pszParam[0] == '?' || pszParam[1] == '?' )
		Syntax();

	if ( last_switch )
	{
		temp_switch = last_switch;
		last_switch = 0;

		// Previous switch expects another parameter.
		switch ( temp_switch )
		{
		case 'I':
			if ( iometer[0] != '\0' )	// make sure it hasn't already been set
			{
				Syntax("Iometer address was specified more than once.");
			}
			else if ( strlen(iometer) >= MAX_NETWORK_NAME )
			{
				Syntax("Iometer address parameter was too long.");
			}
			else if ( pszParam[0] == '/' || pszParam[0] == '-' )
			{
				Syntax("Iometer address should follow the /I switch.");
			}
			else
			{
				strcpy(iometer, pszParam);
			}
			return;
		case 'N':
			if ( manager_name[0] != '\0' )	// make sure it hasn't already been set
			{
				Syntax("Manager name was specified more than once.");
			}
			else if ( strlen(pszParam) >= MAX_WORKER_NAME )
			{
				Syntax("Manager name parameter was too long.");
			}
			else if ( pszParam[0] == '/' || pszParam[0] == '-' )
			{
				Syntax("Manager name should follow the /N switch.");
			}
			else
			{
				strcpy(manager_name, pszParam);
			}
			return;
		case 'M':
			if ( pszParam[0] == '/' || pszParam[0] == '-' )
			{
				Syntax("Manager network name should follow the /M switch.");
			}
			else
			{
				strcpy(manager_computer_name, pszParam);
			}
			return;
		case 'X':
                        if ( pszParam[0] == '/' || pszParam[0] == '-' )
                         {
                              Syntax("Excluded filesystem type should follow the /X switch.");
                         }
                         else if ( strlen(pszParam) + strlen(manager_exclude_fs) >= MAX_EXCLUDE_FILESYS)
                         {
                                 Syntax("Excluded filesystem list too long.");
                         }
                         else
                        {
                                 strcat(manager_exclude_fs, pszParam);
                                 strcat(manager_exclude_fs, " ");
                         }
                         return;

		default:
			{
				char tmpary[2] = {last_switch, 0};
#if defined (_WIN32) || defined (_WIN64)
				Syntax("Unrecognized switch: " + (CString)tmpary + ".");
#else // !WIN32 || _WIN64
				char temp_array[128];
				strcpy(temp_array, "Unrecognized switch: ");
				strcat(temp_array, tmpary);
				strcat(temp_array, ".");
				Syntax(temp_array);
#endif // WIN32 || _WIN64
			}
			return;
		}
	}

	// Is this parameter a switch?
	if ( pszParam[0] == '/' || pszParam[0] == '-' )
	{
		if ( strlen(pszParam) != 2 )
		{
			Syntax("Exactly one letter must follow a switch character.\n"
				"Switch characters are \"/\" and \"-\".");
			return;
		}

		switches = TRUE;
		last_switch = toupper( pszParam[1] );

	//////////////////////////////////////////////////////////////////////
	// This is an example of how to allow switches that have meaning on
	// their own, without any additional parameters.
	//
	//	if ( last_switch == 'V' )	// spit out version number and exit
	//	{
	//		// Set BOOL variable indicating that this switch was specified.
	//		// Make sure it's initialized in the constructor.
	//		show_version = TRUE;
	//
	//		last_switch = 0;	// don't look for more parameters related to this switch
	//		return;				// don't allow it to reach the bLast checking
	//	}
	//////////////////////////////////////////////////////////////////////

		if ( bLast )
		{
			Syntax("An additional parameter was expected after the last switch.");
			return;
		}

		return;
	}

	// If switches haven't been used (so far)...
	if ( !switches )
	{
		switch (param_count)
		{
		case 1:
			strcpy(iometer, pszParam);
			return;
		case 2:
			strcpy(manager_name, pszParam);
			return;
		case 3:
			strcpy(manager_computer_name, pszParam);
			return;
		default:
			Syntax("Too many parameters.");
			return;
		}
	}
}
