/* ######################################################################### */
/* ##                                                                     ## */
/* ##  (Dynamo) / IOManager.h                                             ## */
/* ##                                                                     ## */
/* ## ------------------------------------------------------------------- ## */
/* ##                                                                     ## */
/* ##  Job .......: This is the headerfile for the dynamo Manager class   ## */
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
/* ##  Changes ...: 2003-12-21 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Changed DYNAMO_DESTRUCTIVE to                       ## */
/* ##                 IOMTR_SETTING_OVERRIDE_FS                           ## */
/* ##               2003-07-14 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Moved to the use of the IOMTR_[OSFAMILY|OS|CPU]_*   ## */
/* ##                 global defines.                                     ## */
/* ##               - Integrated the License Statement into this header.  ## */
/* ##               2003-02-26 (joe@eiler.net)                            ## */
/* ##               - Added exclude_filesys string so excluded filesystem ## */
/* ##                 types are no longer hard coded.                     ## */
/* ##               - Moved DEFAULT_EXCLUDE_FILESYS in to here from the   ## */
/* ##                 platform dependent cpp files                        ## */
/* ##               - Added shm to the DEFAULT_EXCLUDE_FILESYS list       ## */
/* ##                                                                     ## */
/* ######################################################################### */
#ifndef MANAGER_DEFINED
#define MANAGER_DEFINED



#include "IOGrunt.h"
#include "IOPort.h"
#include "IOPerformance.h"



#if defined(IOMTR_OSFAMILY_UNIX)
 #if defined(IOMTR_OS_LINUX)
  #define DEFAULT_EXCLUDE_FILESYS   "proc shm swap devpts"
 #elif defined(IOMTR_OS_SOLARIS)
  #define DEFAULT_EXCLUDE_FILESYS   "proc specfs config" 
 #else
  #warning ===> WARNING: You have to do some coding here to get the port done! 
 #endif
#elif defined(IOMTR_OSFAMILY_WINDOWS)
 // if we are not on UNIX the excluded filesystem stuff doesn't really matter
 #define DEFAULT_EXCLUDE_FILESYS   ""
#else
 #warning ===> WARNING: You have to do some coding here to get the port done!
#endif



void Wait_for_Prepare( void *grunt_thread_info );



//
// Class to manager grunt worker threads.
//
class Manager
{
public:
	Manager();
	~Manager();

	const char* 	GetVersionString(BOOL fWithDebugIndicator = FALSE);
	BOOL		Process_Message();
	BOOL		Login( char* port_name );
	BOOL		Run();

	Port	       *prt;                   // Communication port to Iometer.

	Grunt*		grunts[MAX_WORKERS];   // I/O workers.
	
	void	       *data;					// Buffer for I/O requests.
	int		data_size;				// Size of currently allocated data buffer.

	char		manager_name[MAX_WORKER_NAME];	        // Name of manager, customizable on command line.
	char            exclude_filesys[MAX_EXCLUDE_FILESYS];   // filesystem types to exclude, command line option
	
#if defined(IOMTR_OSFAMILY_UNIX)
	char* 		swap_devices;
	BOOL		is_destructive;
	BOOL		is_buffered;
#endif // IOMTR_OSFAMILY_UNIX

private:
	Message		msg;	       // Directional messages from Iometer.
	Data_Message	data_msg;      // Data messages from Iometer.
	int		grunt_count;   // Number of worker threads available.
	char	       *m_pVersionString;
	char	       *m_pVersionStringWithDebug;

	// Performance results functions and data.
	void		Report_Results( int which_perf );
	void		Get_Performance( int which_perf, int snapshot );

	Manager_Results	manager_performance[MAX_PERF];   // System performance results.
	Performance	perf_data[MAX_PERF];		 // System performance information.

	// Functions to process specific messages.
	void		Start_Test( int target );
	void		Begin_IO( int target );
	void		Record_On( int target );
	void		Record_Off( int target );
	void		Stop_Test( int target );

	BOOL		Set_Targets( int worker_no, int count, Target_Spec* target_specs );

	// Processing messages aimed at disk stuff.
	void		Prepare_Disks( int target );
	void		Stop_Prepare( int target );

	int		Report_Disks( Target_Spec *disk_spec );
#if defined(IOMTR_OSFAMILY_UNIX)
	// These UNIX-specific methods are defined in IOManagerUNIX.cpp.
#if defined(IOMTR_SETTING_OVERRIDE_FS)
	BOOL		Reported_As_Logical(Target_Spec *spec, char *rdisk, int count);
#endif // IOMTR_SETTING_OVERRIDE_FS
	BOOL		Part_Reported_As_Logical(Target_Spec *spec, char *rdisk, int count);

	BOOL 		Sort_Raw_Disk_Names(Target_Spec *disk_spec, int start, int end);
	int		Compare_Raw_Disk_Names(char *str1, char *str2);

	BOOL 		Report_FDISK_Partitions(char *name, Target_Spec *disk_spec, int *count, int logical_count);
	BOOL 		Report_VTOC_Partitions(char *name, Target_Spec *disk_spec, int *count, int logical_count);
	BOOL 		Has_File_System(char *, char *);
	void 		Get_All_Swap_Devices();
#endif // IOMTR_OSFAMILY_UNIX

	int		Report_TCP( Target_Spec *tcp_spec );
	int		Report_VIs( Target_Spec *vi_spec );

	BOOL		Set_Access( int target, const Test_Spec* spec );

	void		Add_Workers( int count );
	void		Remove_Workers( int target );

	BOOL		record;
};



#endif
