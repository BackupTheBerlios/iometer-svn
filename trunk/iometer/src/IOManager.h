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
// IOManager.h: Interface for the Manager class for Dynamo.  
// This is Dynamo's main class; it manages communication with Iometer,
// creates and destroys worker threads, starts and stops tests, records
// system-level results (CPU and network), etc.  
//
//////////////////////////////////////////////////////////////////////

#ifndef MANAGER_DEFINED
#define MANAGER_DEFINED


#include "IOGrunt.h"
#include "IOPort.h"
#include "IOPerformance.h"


void	Wait_for_Prepare( void *grunt_thread_info );


//
// Class to manager grunt worker threads.
//
class Manager
{
public:
	Manager();
	~Manager();

	const char* GetVersionString(BOOL fWithDebugIndicator = FALSE);
	BOOL		Process_Message();
	BOOL		Login( char* port_name );
	BOOL		Run();

	Port		*prt;					// Communication port to Iometer.

	Grunt*		grunts[MAX_WORKERS];	// I/O workers.
	
	void		*data;					// Buffer for I/O requests.
	int			data_size;				// Size of currently allocated data buffer.

	char		manager_name[MAX_WORKER_NAME];	// Name of manager, customizable on command line.
	
#ifdef UNIX
	char* 		swap_devices;
	BOOL		is_destructive;
	BOOL		is_buffered;
#endif // UNIX

private:
	Message			msg;				// Directional messages from Iometer.
	Data_Message	data_msg;			// Data messages from Iometer.
	int				grunt_count;		// Number of worker threads available.
	char		   *m_pVersionString;
	char		   *m_pVersionStringWithDebug;

	// Performance results functions and data.
	void		Report_Results( int which_perf );
	void		Get_Performance( int which_perf, int snapshot );

	Manager_Results	manager_performance[MAX_PERF];	// System performance results.
	Performance		perf_data[MAX_PERF];			// System performance information.

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

	int			Report_Disks( Target_Spec *disk_spec );
#ifdef UNIX
	// These UNIX-specific methods are defined in IOManagerUNIX.cpp.
#ifdef DYNAMO_DESTRUCTIVE
	BOOL		Reported_As_Logical(Target_Spec *spec, char *rdisk, int count);
#endif // DYNAMO_DESTRUCTIVE
	BOOL		Part_Reported_As_Logical(Target_Spec *spec, char *rdisk, int count);

	BOOL 		Sort_Raw_Disk_Names(Target_Spec *disk_spec, int start, int end);
	int			Compare_Raw_Disk_Names(char *str1, char *str2);

	BOOL 		Report_FDISK_Partitions(char *name, Target_Spec *disk_spec, int *count, int logical_count);
	BOOL 		Report_VTOC_Partitions(char *name, Target_Spec *disk_spec, int *count, int logical_count);
	BOOL 		Has_File_System(char *, char *);
	void 		Get_All_Swap_Devices();

#endif // UNIX
	int			Report_TCP( Target_Spec *tcp_spec );
	int			Report_VIs( Target_Spec *vi_spec );

	BOOL		Set_Access( int target, const Test_Spec* spec );

	void		Add_Workers( int count );
	void		Remove_Workers( int target );

	BOOL		record;
};


#endif
