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
/* ##  Intel Open Source License                                          ## */
/* ##                                                                     ## */
/* ##  Copyright (c) 2001 Intel Corporation                               ## */
/* ##  All rights reserved.                                               ## */
/* ##  Redistribution and use in source and binary forms, with or         ## */
/* ##  without modification, are permitted provided that the following    ## */
/* ##  conditions are met:                                                ## */
/* ##                                                                     ## */
/* ##  Redistributions of source code must retain the above copyright     ## */
/* ##  notice, this list of conditions and the following disclaimer.      ## */
/* ##                                                                     ## */
/* ##  Redistributions in binary form must reproduce the above copyright  ## */
/* ##  notice, this list of conditions and the following disclaimer in    ## */
/* ##  the documentation and/or other materials provided with the         ## */
/* ##  distribution.                                                      ## */
/* ##                                                                     ## */
/* ##  Neither the name of the Intel Corporation nor the names of its     ## */
/* ##  contributors may be used to endorse or promote products derived    ## */
/* ##  from this software without specific prior written permission.      ## */
/* ##                                                                     ## */
/* ##  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND             ## */
/* ##  CONTRIBUTORS ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,      ## */
/* ##  INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF           ## */
/* ##  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE           ## */
/* ##  DISCLAIMED. IN NO EVENT SHALL THE INTEL OR ITS  CONTRIBUTORS BE    ## */
/* ##  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,   ## */
/* ##  OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,           ## */
/* ##  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,    ## */
/* ##  OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY    ## */
/* ##  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR     ## */
/* ##  TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT    ## */
/* ##  OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY    ## */
/* ##  OF SUCH DAMAGE.                                                    ## */
/* ##                                                                     ## */
/* ## ------------------------------------------------------------------- ## */
/* ##                                                                     ## */
/* ##  Remarks ...: <none>                                                ## */
/* ##                                                                     ## */
/* ## ------------------------------------------------------------------- ## */
/* ##                                                                     ## */
/* ##  Changes ...: 2004-02-07 (mingz@ele.uri.edu)                        ## */
/* ##               - Changed call from im_kstat to iomtr_kstat.          ## */
/* ##                 Daniel suggest to use this clearer name.            ## */
/* ##               2004-02-06 (mingz@ele.uri.edu)                        ## */
/* ##               - Added /proc/stat style detect code and invoke       ## */
/* ##                 it in main().                                       ## */
/* ##               - Added code to try to utilize the im_kstat module.   ## */
/* ##               2003-12-21 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Changed DYNAMO_DESTRUCTIVE to                       ## */
/* ##                 IOMTR_SETTING_OVERRIDE_FS                           ## */
/* ##               - Consolidated the ParseParam() method.               ## */
/* ##               - Disabled accepting parameters without switch.       ## */
/* ##               2003-08-02 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Integrated the modification contributed by          ## */
/* ##                 Vedran Degoricija, to get the code compile with     ## */
/* ##                 the MS DDK on IA64.                                 ## */
/* ##               - Moved to the use of the IOMTR_[OSFAMILY|OS|CPU]_*   ## */
/* ##                 global defines.                                     ## */
/* ##               2003-07-27 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Implemented a test call of the IsBigEndian()        ## */
/* ##                 function to ensure, that we are able to detect      ## */
/* ##                 the endian type of the CPU.                         ## */
/* ##               - Integrated the License Statement into this header.  ## */
/* ##               2003-02-26 (joe@eiler.net)                            ## */
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


#include "IOCommon.h"
#include "IOManager.h"

#if defined(IOMTR_OSFAMILY_UNIX)
 #include <sys/resource.h>
 #include <ctype.h>
 #if defined(IOMTR_OS_SOLARIS)
  #include <synch.h>
 #elif defined(IOMTR_OS_LINUX)
  int kstatfd;
  int procstatstyle = PROCSTATUNKNOWN;  
 #else
  #warning ===> WARNING: You have to do some coding here to get the port done!
 #endif
#endif





/////////////////////////////////////////////////////////////////////////////
// Forward declarations
/////////////////////////////////////////////////////////////////////////////
static void Syntax( const char* errmsg = NULL );

static void ParseParam(	int   argc,
                        char *argv[],
			char  iometer[MAX_NETWORK_NAME],
			char  manager_name[MAX_WORKER_NAME],
			char  manager_computer_name[MAX_NETWORK_NAME],
			char  manager_exclude_fs[MAX_EXCLUDE_FILESYS] );
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

// Temporary global data for Syntax() to use
const char *g_pVersionStringWithDebug = NULL;

#if defined(IOMTR_OS_LINUX)
int InitIoctlInterface(void)
{
	int res;
        res = open("/dev/iomtr_kstat", O_RDONLY);
	if (res < 0)
		cerr << "Fail to open kstat device file" << endl;
	return res;
}

void CleanupIoctlInterface(int fd)
{
	if (fd > 0)
		close(fd);
}

int DetectProcStatStyle(void)
{
	FILE *fp;
	unsigned long long x1, x2, x3, x4, x5, x6, x7;
	int res;

	// a simple work around to detect stat style. should check more customized kernels.
	fp = fopen("/proc/stat", "r");
	res = fscanf(fp, "cpu %lld %lld %lld %lld %lld %lld %lld\n", &x1, &x2, &x3, &x4, &x5, &x6, &x7);
	if (res == 4)
		return PROCSTAT24STYLE;
	else if (res == 7)
		return PROCSTAT26STYLE;
	else
		cerr << "Fail to detect the style of /proc/stat output." << endl;
	return PROCSTATUNKNOWN;
}
#endif

int CDECL main( int argc, char *argv[] )
{
	Manager	manager;
	char	iometer[MAX_NETWORK_NAME];

#if defined(IOMTR_OS_LINUX)
	struct aioinit aioDefaults;

	memset(&aioDefaults, 0, sizeof(aioDefaults));
	aioDefaults.aio_threads = 2;
	aioDefaults.aio_threads = 2;
	aio_init(&aioDefaults);
	kstatfd = InitIoctlInterface();
	procstatstyle = DetectProcStatStyle();
	if (kstatfd < 0 && procstatstyle == -1) {
		cerr << "IoMeter can not get correct status information" << endl;
		exit(1);
	}
#endif

	iometer[0]                 = 0;
	manager.manager_name[0]    = 0;
	manager.exclude_filesys[0] = 0;

	//provide a temporary global ptr to the version string for Syntax() to use
	g_pVersionStringWithDebug = manager.GetVersionString(TRUE);
	ParseParam(
		argc,
		argv,
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

#if defined(IOMTR_OSFAMILY_UNIX)
 #if defined(IOMTR_OS_LINUX)
	signal(SIGALRM, SIG_IGN);
 #elif defined(IOMTR_OS_SOLARIS)
	sigignore(SIGALRM);
 #else
  #warning ===> WARNING: You have to do some coding here to get the port done!
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

#ifdef IOMTR_SETTING_OVERRIDE_FS
	// No command line args specifies destructive testing. Check to see if there
	// are any environment variables specifying the same. We need to warn the user.
	if (getenv("IOMTR_SETTING_OVERRIDE_FS") != NULL)
	{
		cout << "       ************ WARNING **************" << endl;
		cout << "       dynamo running in Destructive mode." << endl;
		cout << "         (overriding the not mounted fs)"   << endl;		
		cout << "       ************ WARNING **************" << endl;
	}
#endif // IOMTR_SETTING_OVERRIDE_FS
#endif // IOMTR_OSFAMILY_UNIX

	// Ensure, that the endian type of the CPU is detectable
	if ( (IsBigEndian() != 0) && (IsBigEndian() != 1) )
	{
		cout << "===> ERROR: Endian type of the CPU couldn't be detected."    << endl;
		cout << "     [main() in " << __FILE__ << " line " << __LINE__ << "]" << endl;
		exit(1);
	}

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

#if defined(IOMTR_OS_LINUX)
	CleanupIoctlInterface(kstatfd);
#endif	
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
	
#if defined(IOMTR_OS_WIN32) || defined(IOMTR_OS_WIN64)
	cout << "dynamo /?" << endl;
#elif defined(IOMTR_OS_LINUX)
	cout << "dynamo -?" << endl;
#elif defined(IOMTR_OS_SOLARIS)
	// Solaris 2.7 must have the switch (? is used for its own purpose).
	cout << "dynamo \\?" << endl;
#else
 #warning ===> WARNING: You have to do some coding here to get the port done!
#endif

#if defined(IOMTR_OS_WIN32) || defined(IOMTR_OS_WIN64)
	cout << "dynamo [/i iometer_computer_name] [/n manager_name] [/m manager_network_name]" << endl;
#elif defined(IOMTR_OS_LINUX) || defined(IOMTR_OS_SOLARIS)
	cout << "dynamo [-i iometer_computer_name] [-n manager_name] [-m manager_network_name]" << endl;
	cout << "       [-x excluded_fs_type]" << endl;
#else
 #warning ===> WARNING: You have to do some coding here to get the port done!
#endif

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

#if defined(IOMTR_OS_WIN32) || defined(IOMTR_OS_WIN64)
	// nop	
#elif defined(IOMTR_OS_LINUX) || defined(IOMTR_OS_SOLARIS)
	cout << "   excluded_fs_type - type of filesystems to exclude from device search" << endl;
        cout << "      This string should contian the filesystem types that are not reported" << endl;
        cout << "      to Iometer. The default is \"" << DEFAULT_EXCLUDE_FILESYS << "\"." << endl;
#else
 #warning ===> WARNING: You have to do some coding here to get the port done!
#endif

        cout << endl;
	
	exit( 0 );
}





/* ######################################################################### */
/* ##                                                                     ## */
/* ##   P A R S E P A R A M ( )                                           ## */
/* ##                                                                     ## */
/* ######################################################################### */
void ParseParam(
	int   argc,
	char *argv[],
	char  iometer[MAX_NETWORK_NAME],
	char  manager_name[MAX_WORKER_NAME],
	char  manager_computer_name[MAX_NETWORK_NAME],
	char  manager_exclude_fs[MAX_EXCLUDE_FILESYS] )
{
	// Local variables
	
	char cSwitchKey;

	// Walk through the parameter list

	for( int I = 1; I < argc; I++ ) {

		// See if the user is requesting syntax help.
		
		if ( ( argv[I][0] == '?' ) || ( argv[I][1] == '?' ) ) {
			Syntax();
			return;
		}

		// Ensure that the each parameter has a leading switch
		
		if ( ( argv[I][0] != '-' ) && ( argv[I][0] != '/' ) ) {
			Syntax( "Exactly one letter must follow a switch character.\n"
				"Switch characters are \"-\" and \"/\"." );
			return;			
		}

		if ( strlen( argv[I] ) != 2 ) {
			Syntax( "Exactly one letter must follow a switch character.\n"
				"Switch characters are \"-\" and \"/\"." );
			return;			
		}
		
		// Ensure that each parameter has a value following the switch
		
		if( ( I + 1 ) >= argc ) {
			Syntax("An additional parameter was expected after the last switch.");
			return;
		}

		// Process the parameters based on the switch

		cSwitchKey = toupper( argv[I][1] );
		I++;
		
		switch( cSwitchKey ) {
			case 'I':
				if ( iometer[0] != '\0' ) {
					Syntax("Iometer address was specified more than once.");
					return;
				}
				if ( strlen( argv[I] ) >= MAX_NETWORK_NAME ) {
					Syntax("Iometer address parameter was too long.");
					return;
				}
				strcpy( iometer, argv[I] );
				break;
			case 'N':
				if ( manager_name[0] != '\0' ) {
					Syntax("Manager name was specified more than once.");
					return;
				}
				if ( strlen( argv[I] ) >= MAX_WORKER_NAME ) {
					Syntax("Manager name parameter was too long.");
					return;
				}
				strcpy( manager_name, argv[I] );
				break;
			case 'M':
				// No check for more then once specification (as we have a default)
				if ( strlen( argv[I] ) >= MAX_NETWORK_NAME ) {
					Syntax("Manager network name parameter was too long.");
					return;
				}
				strcpy( manager_computer_name, argv[I] );
				break;
			case 'X':
				if ( ( strlen( argv[I] ) + strlen( manager_exclude_fs ) ) >= MAX_EXCLUDE_FILESYS ) {
					Syntax("Excluded filesystem list too long.");
					return;
				}
				strcat( manager_exclude_fs, argv[I] );
				strcat( manager_exclude_fs, " " );
				break;
			default:
				{
					char tmpary[2] = { cSwitchKey, 0 };
#if defined(IOMTR_OSFAMILY_WINDOWS)
					Syntax( "Unrecognized switch: " + (CString)tmpary + "." );
#elif defined(IOMTR_OSFAMILY_UNIX)
					char temp_array[128];
					strcpy( temp_array, "Unrecognized switch: " );
					strcat( temp_array, tmpary );
					strcat( temp_array, "." );
					Syntax( temp_array );
#else
 #warning ===> WARNING: You have to do some coding here to get the port done!
#endif
				}
				break;
		}
	}
	
	return;
}




