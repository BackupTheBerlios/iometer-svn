/* ######################################################################### */
/* ##                                                                     ## */
/* ##  Dynamo / IOManager.cpp                                             ## */
/* ##                                                                     ## */
/* ## ------------------------------------------------------------------- ## */
/* ##                                                                     ## */
/* ##  Job .......: This is Dynamo's main class. It coordinates Dynamo's  ## */
/* ##               work and handles the communication with IOMeter.      ## */
/* ##                                                                     ## */
/* ## ------------------------------------------------------------------- ## */
/* ##                                                                     ## */
/* ##  Remarks ...: <none>                                                ## */
/* ##                                                                     ## */
/* ## ------------------------------------------------------------------- ## */
/* ##                                                                     ## */
/* ##  Changes ...: 2003-05-07 (yakker@aparity.com)                       ## */
/* ##               - Applied the iometerCIOB5.2003.05.02.patch file      ## */
/* ##                 (avoiding cache line collisions and performance     ## */
/* ##                 lock-ups for some chipsets).                        ## */
/* ##               2003-03-04 (joe@eiler.net)                            ## */
/* ##               - Changed NO_LINUX_VI to NO_DYNAMO_VI                 ## */
/* ##               2003-03-01 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Cut out the Windows Pipes support for               ## */
/* ##                 communication efforts.                              ## */
/* ##               2003-02-04 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Added new header holding the changelog.             ## */
/* ##               - Applied proc-speed-fix.txt patch file               ## */
/* ##                 (dropping a type cast in the Login() method).       ## */
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
/// ==========================================================================
//                Copyright (C) 1997-2000 Intel Corporation
//                          All rights reserved                               
//                INTEL CORPORATION PROPRIETARY INFORMATION                   
//    This software is supplied under the terms of a license agreement or     
//    nondisclosure agreement with Intel Corporation and may not be copied    
//    or disclosed except in accordance with the terms of that agreement.     
// ==========================================================================
//
// IOManager.cpp: Implementation of the Manager class for Dynamo.  
// This is Dynamo's main class; it manages communication with Iometer,
// creates and destroys worker threads, starts and stops tests, records
// system-level results (CPU and network), etc.  There is always exactly
// one Manager object in an instance of Dynamo (local variable "manager"
// in function main() in Pulsar.cpp).
//
// Note that the function Manager::Report_Disks() is OS-dependent.  
// The implementation of this function in this file is for Windows NT;
// the Solaris and Linux implementations can be found in the files
// IOManagerSolaris.cpp and IOManagerLinux.cpp respectively.
//
//////////////////////////////////////////////////////////////////////
/* ######################################################################### */

#include "IOCommon.h"
#include "IOManager.h"
#if defined (_WIN32) || defined (_WIN64)
#include "winsock2.h"
#else // UNIX
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/types.h>
#endif

#include "IOPortTCP.h"
#include "IOTargetDisk.h"
#include "IOTargetVI.h"

#define	KNOWN_VI_NIC_NAMES	3

//
// Initializing manager variables before their first use.
//
Manager::Manager()
 : m_pVersionString(NULL), m_pVersionStringWithDebug(NULL)
{
	data_size = 0;
	data = NULL;
	SetLastError( 0 );

	//init file version strings
	GetAppFileVersionString(&m_pVersionString, &m_pVersionStringWithDebug);

	prt = new PortTCP;
	if ( !prt->Create() )
	{
		cout << "*** Could not create a TCP/IP Port. exiting....." << endl;
		exit(1);
	}
	
	record = FALSE;
	grunt_count = 0;
}
 


//
// Destructor
//
Manager::~Manager()
{
	int g;

	delete [] m_pVersionString;
	delete [] m_pVersionStringWithDebug;

	prt->Close();
	delete prt;
#if defined (_WIN32) || defined (_WIN64)
	VirtualFree( data, 0, MEM_RELEASE );
#else
	free(data);
	free(swap_devices);
#endif
	for ( g = 0; g < grunt_count; g++ )
		delete grunts[g];
}

///////////////////////////////////////////////////////////////////////////////
//	Function Name:
//		Manager::GetVersionString
//
//	Syntax:
//		const char*		GetVersionString(BOOL fWithDebugIndicator = FALSE);
//
//	Processing:
//		Return version string ptr (m_pVersionString or m_pVersionStringWithDebug).
///////////////////////////////////////////////////////////////////////////////
const char*		Manager::GetVersionString(BOOL fWithDebugIndicator)
{
	if (! fWithDebugIndicator)
		return m_pVersionString;
	else
		return m_pVersionStringWithDebug;
}


//
// Logging into Iometer.  Returns success.
//
BOOL Manager::Login( char* port_name )
{
	Port			*login_port;
	Message			msg, reply;
	Data_Message	data_msg;
	size_t			name_size = MAX_NETWORK_NAME;
	int				year, month, day;

	// Creating login messages that include the machine name and Dynamo version.
	// The version number is included in two places for backward compatibility.
	msg.purpose = LOGIN;
	strcpy(data_msg.data.manager_info.version, m_pVersionStringWithDebug);
#ifdef _DEBUG
	cout << "dynamo version: " << data_msg.data.manager_info.version << endl;
#endif
	sscanf( data_msg.data.manager_info.version, "%d.%d.%d", &year, &month, &day );
	msg.data = (year * 10000) + (month * 100) + day;

	if ( manager_name[0] != '\0' )
	{
		if ( strlen(manager_name) > MAX_NETWORK_NAME )
		{
			cout << "*** Specified manager name cannot be more than "
				 << MAX_NETWORK_NAME << " characters" << endl;
			exit(1);
		}

		strcpy( data_msg.data.manager_info.names[0], manager_name );
		name_size = strlen(manager_name);
	}
	else
	{
#ifdef UNIX
		// This will not work correctly if hostname length > MAX_NETWORK_NAME
		if (gethostname(manager_name, name_size) < 0)
		{
			cout << "*** Exiting... gethostname() returned error " << errno << endl;
			exit(1);
		}
		name_size = strlen(data_msg.data.manager_info.names[0]);
#else /* WIN_NT */
		GetComputerName( manager_name, (LPDWORD)&name_size );
#endif
		strcpy(data_msg.data.manager_info.names[0], manager_name);
	}
	strcpy( data_msg.data.manager_info.names[1], prt->network_name );
	data_msg.data.manager_info.port_number     = prt->network_port;
	data_msg.data.manager_info.processor_speed = perf_data[WHOLE_TEST_PERF].processor_speed;
	data_msg.data.manager_info.processors      = perf_data[WHOLE_TEST_PERF].processor_count;

#ifdef __sparc
	// Calculate processor_speed_to_nsecs for use in rdtsc.c
	// Note that this works only for MHz CPUs. For GHz CPUs the divisor will change.
	processor_speed_to_nsecs = (double)perf_data[WHOLE_TEST_PERF].processor_speed / 1000000000;
#endif // __sparc

	// Sending login request message.
	cout << "Sending login request..." << endl;
	cout << "   " << data_msg.data.manager_info.names[0] << endl;
	cout << "   " << data_msg.data.manager_info.names[1] 
		 << " (port " << data_msg.data.manager_info.port_number << ")" << endl;

	if ( prt->type == PORT_TYPE_TCP )
	{
		login_port = new PortTCP;

		if ( !login_port->Connect( port_name ) )
		{
			cout << "*** Could not create TCP/IP port to connect with Iometer!" << endl;
			return FALSE;
		}
	}
	else
	{
		cout << "*** Invalid port type in Manager::Login()" << endl;
		return FALSE;
	}

#ifdef BIG_ENDIAN_ARCH
	(void)reorder(msg);
	(void)reorder(data_msg, DATA_MESSAGE_MANAGER_INFO, SEND);
#endif
	login_port->Send( &msg );
	login_port->Send( &data_msg, DATA_MESSAGE_SIZE );

	// wait to receive reply to login request, then get the incoming message...
	if ( login_port->Receive( &reply ) == PORT_ERROR )
	{
		cout << endl << "*** Unable to receive a reply from Iometer" << endl;
		return FALSE;
	}

#ifdef BIG_ENDIAN_ARCH
	(void)reorder(reply);
#endif

	// Delete login port because we don't need it anymore.
	delete  login_port;

	// process the login reply message from Iometer
	switch (reply.data)
	{
		case WRONG_VERSION:
			cout << endl << "*** Incorrect version of Dynamo for this Iometer" << endl;
			return FALSE;

		case LOGIN_OK:
			//
			// Waiting for login to be accepted.
			if ( prt->Accept() )
				cout << "   Login accepted." << endl << flush;
			return TRUE;
		default:
			cout << endl << "*** Bad login status reply received - don't know what to do" << endl;
			return FALSE;
	}
}


#ifndef UNIX
//
// Checking for all accessible drives.  Storing them, and returning the number
// of disks found.  Drives are reported in order so that Iometer does not
// need to sort them.
//
int Manager::Report_Disks( Target_Spec* disk_spec )
{
	DWORD	dummy_word;
	TargetDisk	d;
	char	drive_letter;
	int		drive_number = 0;
	int		count = 0;

	cout << "Reporting drive information..." << endl;

	// Reporting logical drives first.
	for ( drive_letter = 'A'; drive_letter <= 'Z'; drive_letter++ )
	{
		// Checking to see if drive exists.
		if ( ! d.Init_Logical( drive_letter ) )
			continue;

		// Drive exists.  Getting its sector size, label, and volume name.
		memcpy( &disk_spec[count], &d.spec, sizeof( Target_Spec ) );
		strcat( disk_spec[count].name, "\\" );
		if ( !GetVolumeInformation( disk_spec[count].name, disk_spec[count].name + 2, 
			MAX_NAME - 1, NULL, &dummy_word, &dummy_word, NULL, 0 ) )
		{
			cout << "   Unable to retrieve volume information for " 
				<< d.spec.name << "." << endl;
			disk_spec[count].name[2] = '\0';
		}

		#if _DEBUG
				cout << "   Found " << disk_spec[count].name << "." << endl;
		#endif
		count++;
	}

	// Reporting physical drives.
	while ( count < MAX_TARGETS )
	{
		// See if the physical drive exists.
		sprintf( d.spec.name, "%s%i", PHYSICAL_DISK, drive_number );
		strcpy( d.file_name, d.spec.name );
		d.spec.type = PhysicalDiskType;

		// Try to open the drive, if it exists close it and initialize it.  If
		// it doesn't exist, no more physical drives will be found.  This is
		// temporary code until the TargetDisk class is split up.
		if ( !d.Open( NULL ) )
			break;
		d.Close( NULL );

		// Drive exists, see if it is available for accessing.
		if( ! d.Init_Physical( drive_number ) )
		{
			drive_number++;
			continue;
		}

		memcpy( &disk_spec[count], &d.spec, sizeof( Target_Spec ) );
		strcpy( disk_spec[count].name, PHYSICAL_DRIVE_PREFIX );
		_itoa( drive_number, disk_spec[count].name + strlen( disk_spec[count].name ), 10 );
		drive_number++;

		#if _DEBUG
				cout << "   Found " << disk_spec[count].name << "." << endl;
		#endif
		count++;
	}
	cout << "   done." << endl << flush;
	return count;
}
#endif // !UNIX

//
// Checking for all TCP network interfaces.  Storing them, and returning the number
// of interfaces found.  
//
int Manager::Report_TCP( Target_Spec *tcp_spec )
{
#if defined (_WIN32) || defined (_WIN64)
	int 				retval;
	WSADATA 			wd;
#endif
	struct hostent		*hostinfo;
	struct sockaddr_in	sin;
	char				hostname[128];
	int					count = 0;
	int 				i;

	cout << "Reporting TCP network information..." << endl;

#if defined (_WIN32) || defined (_WIN64)
	// initialize WinSock version 2.0
	retval = WSAStartup( MAKEWORD(2, 0), &wd ); 
	if ( retval != 0 )
	{
		cout << "*** Error " << retval << "initializing WinSock." << endl;
		return 0;
	} 
#endif

	// get the unqualified local host name
	if (gethostname(hostname, sizeof(hostname)) == SOCKET_ERROR)
	{
		cout << "*** Error " << WSAGetLastError() << "getting local host name.\n";
		return 0;
	}

	// now get the host info for that host name
	hostinfo = gethostbyname ( hostname );
	if ( hostinfo == NULL )
	{
		cout << "*** Error " << WSAGetLastError() << "getting host info for \"" 
			 << hostname << "\".\n";
		return 0;
	}

	#ifdef _DEBUG
		printf("   My hostname: \"%s\"\n", hostinfo->h_name);

		i=0;
		while ( hostinfo->h_aliases[i] != NULL )
		{
			printf("   Alias: \"%s\"\n", hostinfo->h_aliases[i]);
			i++;
		}
	#endif

	// report the network addresses.
	for ( i = 0; hostinfo->h_addr_list[i] != NULL; i++ ) 
	{
		memcpy ( &sin.sin_addr.s_addr, hostinfo->h_addr_list[i], hostinfo->h_length );
		strncpy ( tcp_spec[count].name, inet_ntoa(sin.sin_addr), 
			sizeof(tcp_spec[count].name) - 1 );
		tcp_spec[count].type = TCPClientType;	// interface to access a client

		#if _DEBUG
			cout << "   Found " << tcp_spec[count].name << "." << endl;
		#endif

		if ( ++count >= MAX_NUM_INTERFACES )
		{
			cout << "*** Found the maximum number of supported network interfaces: " 
				<< endl << "Only returning the first " << MAX_NUM_INTERFACES << "." << endl;
			count = MAX_NUM_INTERFACES;
			break;
		}
	}

	#if 0	// for debugging multiple-network-interface GUI; change "#if 0" to "#if 1" to enable
		strncpy ( nets[count], "foo", sizeof(nets[count]) - 1 );
		cout << "   Added fake entry " << nets[count] << "." << endl << flush;
		count++;
	#endif

#if defined (_WIN32) || defined (_WIN64)
	// clean up WinSock
	if ( WSACleanup() != 0 )
	{
		cout << "*** Error " << WSAGetLastError() << "cleaning up WinSock." << endl;
		return 0;
	}
#endif

	// All done.
	cout << "   done." << endl;
	return count;
}



//
// Checking for all VI network interfaces.  Storing them, and returning the 
// number of interfaces found.
//
int Manager::Report_VIs( Target_Spec *vi_spec )
{
#ifdef NO_DYNAMO_VI
	return(0);
#else
	int		count = 0, nic_base;
	VIPL	vipl;
	VINic	nic;
	UINT	address_buffer_size, nic_address_size;
	char	vi_nic_name[KNOWN_VI_NIC_NAMES][MAX_NAME];
	int		vi_nic_name_no[KNOWN_VI_NIC_NAMES];
	char	env_var_name[MAX_NAME];

	cout << "Reporting VI information..." << endl;
	// Verify that VI is loaded on this system.
	if ( !vipl.vipl_dll )
	{
		cout << "   VI library is not installed." << endl;
		return 0;
	}

	// Initialize the base name and number for default VI nic names.
	strcpy( vi_nic_name[0], "VINIC" );
	vi_nic_name_no[0] = 0;
	strcpy( vi_nic_name[1], "nic" );
	vi_nic_name_no[1] = 0;
	strcpy( vi_nic_name[2], "\\\\.\\VI" );
	vi_nic_name_no[2] = 1;

	// Look for VI NIC's.  If no nic's are found matching the default
	// names, look for system environment variables that 
	// contain a mapping to the actual name.  (e.g. SET VINIC0=NIC1...)
	for ( nic_base = 0; nic_base < KNOWN_VI_NIC_NAMES; nic_base++ )
	{
		// Continue using the same base nic name until we've found all the 
		// nics that are available.
		while ( TRUE )
		{
			// Form VI NIC name to open.
			vi_spec[count].type = VIClientType;
			sprintf( vi_spec[count].name, "%s%i", vi_nic_name[nic_base],
				vi_nic_name_no[nic_base]++ );

			// Try opening VI NIC.
			if ( !nic.Open( vi_spec[count].name ) )
			{
				// Only search for environment variable mappings using the
				// first (VINIC) name base.
				if ( nic_base )
					break;

				// Nic does not exist.  See if there is an environment variable
				// mapping to another NIC name.
				strcpy( env_var_name, vi_spec[count].name );
	#if defined (_WIN32) || defined (_WIN64)
				if ( !GetEnvironmentVariable( env_var_name, vi_spec[count].name,
					MAX_NAME ) )
				{
					break;
				}
	#else//UNIX
				if ( getenv( env_var_name ) == NULL )
					break;

				strncpy( vi_spec[count].name, getenv( env_var_name ), 
					MAX_NAME - 1 );
	#endif
				// A mapping exists.  Try to open the VI NIC using the 
				// environment variable setting.
				if ( !nic.Open( vi_spec[count].name ) )
					break;
			}

			// Determine how many bytes are needed for the NIC address
			// (not including the discriminator, which is null at this point).
			nic_address_size = sizeof( *(nic.nic_address) ) +
				nic.nic_attributes.NicAddressLen - 1;

			// Determine the (static) size of the address buffer in the vi_spec structure.
			address_buffer_size = sizeof( vi_spec[count].vi_info.local_address ) + 
				sizeof( vi_spec[count].vi_info.local_address_fill_bytes );

			// Zero the address buffer.
			memset( &vi_spec[count].vi_info.local_address, 0, address_buffer_size );

			// Copy the NIC's address into the local_address buffer
			// (as much as will fit or the whole thing, whichever is less).
			memcpy( &vi_spec[count].vi_info.local_address, nic.nic_address, 
				min( address_buffer_size, nic_address_size ) );

			// Did the address fit, leaving enough room for the discriminator?
			if ( nic_address_size + min( VI_DISCRIMINATOR_SIZE, nic.nic_attributes.MaxDiscriminatorLen )
				> address_buffer_size )
			{
				cout << "*** WARNING: VI NIC \"" << vi_spec[count].name
					<< "\"'s address size (" << nic_address_size
					<< ") + discriminator length (" << min( VI_DISCRIMINATOR_SIZE, nic.nic_attributes.MaxDiscriminatorLen )
					<< ") is larger than address buffer size (" << address_buffer_size
					<< ")!" << endl;
			}

			vi_spec[count].vi_info.max_transfer_size = 
				nic.nic_attributes.MaxTransferSize;

			// Each connection requires its own completion queue.
			vi_spec[count].vi_info.max_connections = nic.nic_attributes.MaxCQ;

			// Due to pre-postin requirements, we support only half of the
			// available queue depth.
			vi_spec[count].vi_info.outstanding_ios = 
				nic.nic_attributes.MaxDescriptorsPerQueue >> 1;

			nic.Close();
			#if _DEBUG
				cout << "   Found " << vi_spec[count].name << "." << endl;
			#endif

			// Prevent reporting more than the maximum number of VIs.
			if ( ++count >= MAX_NUM_INTERFACES )
			{
				cout << "*** Found the maximum number of supported VI interfaces: " << endl
					<< "Only returning the first " << MAX_NUM_INTERFACES << "." << endl;
				return MAX_NUM_INTERFACES;
			}
		}
	}

	// All done.
	cout << "   done." << endl;
	return count;
#endif // NO_DYNAMO_VI
}



//
// Preparing drives for selected grunts.
//
void Manager::Prepare_Disks( int target )
{   
	int i, loop_start, loop_finish;

	if ( target == ALL_WORKERS )
	{
		// Preparing all grunts at the same time.  This requires a great
		// amount of coordination on the part of Iometer to ensure that the
		// grunts do not prepare the same drives.
		for ( i = 0; i < grunt_count; i++ )
		{
			if ( !grunts[i]->Prepare_Disks() )
			{
				// Send failure message back to Iometer.
				msg.data = 0;
#ifdef BIG_ENDIAN_ARCH
				(void)reorder(msg);
#endif
				prt->Send( &msg );
				return;
			}
		}
		loop_start = 0;
		loop_finish = grunt_count;
	}
	else
	{
		// Preparing a single grunt.
		if ( !grunts[target]->Prepare_Disks() )
		{
			// Send failure message back to Iometer.
			msg.data = 0;
#ifdef BIG_ENDIAN_ARCH
			(void) reorder(msg);
#endif
			prt->Send( &msg );
			return;
		}

		loop_start = target;
		loop_finish = loop_start + 1;
	}

	// Peek to see if the prepare was be canceled by the user.
	for ( i = loop_start; i < loop_finish; i++ )
	{
		while( grunts[i]->not_ready )
		{
			if ( prt->Peek() )
			{
				prt->Receive( &msg );
#ifdef BIG_ENDIAN_ARCH
				(void) reorder(msg);
#endif
				Process_Message();
			}
			else
			{
				Sleep( LONG_DELAY );
			}
		}
		grunts[i]->grunt_state = TestIdle;
	}
	// Send a message back to Iometer to indicate that we're done preparing.
	msg.data = 1;	// indicates success
#ifdef BIG_ENDIAN_ARCH
	(void) reorder(msg);
#endif
	prt->Send( &msg );
}



//
// Collecting and extracting system performance data.
//
void Manager::Get_Performance( int which_perf, int snapshot )
{
	if ( (which_perf < 0) || (which_perf >= MAX_PERF)
		|| (snapshot < 0) || (snapshot >= MAX_SNAPSHOTS) )
		return;

	// Get all performance data.
	perf_data[which_perf].Get_Perf_Data( PERF_PROCESSOR, snapshot );
	perf_data[which_perf].Get_Perf_Data( PERF_NETWORK_TCP, snapshot );
	perf_data[which_perf].Get_Perf_Data( PERF_NETWORK_INTERFACE, snapshot );

	// Record time that performance snapshot was taken.
	manager_performance[which_perf].time_counter[snapshot] = perf_data[which_perf].time_counter[snapshot];

	// If this is the last snapshot to be taken, calculate the performance statistics
	// to return to Iometer.
	if ( snapshot == LAST_SNAPSHOT )
	{
		perf_data[which_perf].Calculate_CPU_Stats( &(manager_performance[which_perf].cpu_results) );
		perf_data[which_perf].Calculate_TCP_Stats( &(manager_performance[which_perf].net_results) );
		perf_data[which_perf].Calculate_NI_Stats( &(manager_performance[which_perf].net_results) );
	}
}



//
// Gathering results from all threads and reporting them back to Iometer.
//
void Manager::Report_Results( int which_perf )
{
	if ( (which_perf < 0) || (which_perf >= MAX_PERF) )
		return;

	// Shortcut pointers to where results are stored.
	Worker_Results	*worker_results;

	// If recording, update the ending results for the system performance.
	if ( record )
	{
		Get_Performance( which_perf, LAST_SNAPSHOT );
	}

	// Copy the current system results into a message.
	memcpy( (void*) &data_msg.data.manager_results, (void*) 
		&(manager_performance[which_perf]), sizeof( Manager_Results ) );
#ifdef BIG_ENDIAN_ARCH
	(void) reorder(data_msg, DATA_MESSAGE_MANAGER_RESULTS, SEND);
#endif
	prt->Send( &data_msg, DATA_MESSAGE_SIZE );

	// Sending back a result message for each worker thread.  Using multiple
	// messages keeps the message size down to a reasonable limit.
	worker_results = &(data_msg.data.worker_results);
	for ( int g = 0; g < grunt_count; g++ )
	{
		// Only send results for grunts that are running.
		if ( grunts[g]->target_count && !grunts[g]->idle )
		{
			#if _DEBUG
				cout << "Reporting results for grunt " << g << " ...";
			#endif
			// Copying worker's results into the message.
			memcpy( (void*) worker_results, (void*) &(grunts[g]->worker_performance), 
				sizeof( Worker_Results ) );
			worker_results->target_results.count = grunts[g]->target_count;

			// If recording, update the ending results for the worker's drive performance.
			if ( grunts[g]->grunt_state == TestRecording )
			{
				worker_results->time[LAST_SNAPSHOT] = rdtsc();
			}

			if ( which_perf == LAST_UPDATE_PERF )
			{
				// Subtract previous update's results from current results to give 
				// results since last update.

				Raw_Result	*target_result;
				Raw_Result	*prev_target_result;
				int i;

				worker_results->time[FIRST_SNAPSHOT] = 
					grunts[g]->prev_worker_performance.time[LAST_SNAPSHOT];

				for ( i = 0; i < worker_results->target_results.count; i++ )
				{
					target_result = &(worker_results->target_results.result[i]);
					prev_target_result
						= &(grunts[g]->prev_worker_performance.target_results.result[i]);

					// Subtract current result from each counter.
					target_result->bytes_read
						-= prev_target_result->bytes_read;
					target_result->bytes_written
						-= prev_target_result->bytes_written;
					target_result->read_count
						-= prev_target_result->read_count;
					target_result->write_count
						-= prev_target_result->write_count;
					target_result->transaction_count
						-= prev_target_result->transaction_count;
					target_result->connection_count
						-= prev_target_result->connection_count;
					target_result->read_errors
						-= prev_target_result->read_errors;
					target_result->write_errors
						-= prev_target_result->write_errors;
					target_result->read_latency_sum
						-= prev_target_result->read_latency_sum;
					target_result->write_latency_sum
						-= prev_target_result->write_latency_sum;
					target_result->transaction_latency_sum
						-= prev_target_result->transaction_latency_sum;
					target_result->connection_latency_sum
						-= prev_target_result->connection_latency_sum;
					target_result->counter_time
						-= prev_target_result->counter_time;

					// Use values from prev_worker_performance for "max_" values.
					target_result->max_raw_read_latency
						= prev_target_result->max_raw_read_latency;
					target_result->max_raw_write_latency
						= prev_target_result->max_raw_write_latency;
					target_result->max_raw_transaction_latency
						= prev_target_result->max_raw_transaction_latency;
					target_result->max_raw_connection_latency
						= prev_target_result->max_raw_connection_latency;
				}

				// Store away a copy of the current results for next time.
				memcpy( &(grunts[g]->prev_worker_performance), &(grunts[g]->worker_performance), 
					sizeof( Worker_Results ) );

				// Record time of last update.
				grunts[g]->prev_worker_performance.time[LAST_SNAPSHOT] = rdtsc();

				// Clear "max_" values in prev_worker_performance.
				for ( i = 0; i < worker_results->target_results.count; i++ )
				{
					Raw_Result *prev_target_result
						= &(grunts[g]->prev_worker_performance.target_results.result[i]);

					prev_target_result->max_raw_read_latency = 0;
					prev_target_result->max_raw_write_latency = 0;
					prev_target_result->max_raw_transaction_latency = 0;
					prev_target_result->max_raw_connection_latency = 0;
				}
			}

			// Sending results to Iometer.
#ifdef BIG_ENDIAN_ARCH
			(void) reorder(data_msg, DATA_MESSAGE_WORKER_RESULTS, SEND);
#endif
			prt->Send( &data_msg, DATA_MESSAGE_SIZE );
			#if _DEBUG
				cout << "sent." << endl;
			#endif
		}
	}

	if ( record && (which_perf == LAST_UPDATE_PERF) )
	{
		// Store current performance counters as baseline for next update.
		Get_Performance( LAST_UPDATE_PERF, FIRST_SNAPSHOT );
	}

	#if _DEBUG
		cout << "   Finished reporting results." << endl;
	#endif
}



//
// Manager runs assuming Iometer control.  Returns TRUE if Dynamo should
// continue to run, otherwise FALSE.
//
BOOL Manager::Run()
{
	while (TRUE)		// Receive loop.
	{
#ifdef _DEBUG
		cout << "in while loop : Manager::Run() " << endl;
#endif
		if ( prt->Receive( &msg ) == PORT_ERROR )
		{
			// Error receiving data message, stop running.
			cout << "Error receiving message." << endl << flush;
			return FALSE;
		}
		else
		{
#ifdef BIG_ENDIAN_ARCH
			(void) reorder(msg);
#endif
			// Continue to process messages until manager indicates stopping.
			if ( !Process_Message() )
				return FALSE;

			// On a reset, stop then restart running the manager.
			if ( msg.purpose == RESET )
				return TRUE;
		}
	}
}



//
// Processing messages from Iometer based on their purpose and
// returns TRUE if the manager can process additional messages.
//
BOOL Manager::Process_Message()
{
	switch ( msg.purpose )
	{
	case ADD_WORKERS:
#ifdef _DETAILS
		cout << "in Process_Message() : ADD_WORKERS" << endl;
#endif
		Add_Workers( msg.data );
		break;

	// Signaling to reset all workers
	case RESET:
		// Remove all workers.
#ifdef _DETAILS
		cout << "in Process_Message() : RESET" << endl;
#endif
		Remove_Workers( ALL_WORKERS );
		prt->Disconnect();
		break;
	// Received call to end program or thread execution.
	case EXIT:
#ifdef _DETAILS
		cout << "in Process_Message() : EXIT" << endl;
#endif
		Remove_Workers( msg.data );
		return ( msg.data != MANAGER );

	// Preparing drives for access.
	case PREP_DISKS:
#ifdef _DETAILS
		cout << "in Process_Message() : PREP_DISKS" << endl;
#endif
		Prepare_Disks( msg.data );
		break;
	// Signalling to stop disk preparation.
	case STOP_PREPARE:
#ifdef _DETAILS
		cout << "in Process_Message() : STOP_PREPARE" << endl;
#endif
		Stop_Prepare( msg.data );
		break;
	
	// Reporting all targets accessible by this manager.
	case REPORT_TARGETS:
#ifdef _DETAILS
		cout << "in Process_Message() : REPORT_TARGETS" << endl;
#endif
		data_msg.count = Report_Disks( data_msg.data.targets );
#ifdef BIG_ENDIAN_ARCH
		(void) reorder(data_msg, DATA_MESSAGE_TARGET_SPEC, SEND);
#endif
		prt->Send( &data_msg, DATA_MESSAGE_SIZE );

		data_msg.count = Report_TCP( data_msg.data.targets );
#ifdef BIG_ENDIAN_ARCH
		(void) reorder(data_msg, DATA_MESSAGE_TARGET_SPEC, SEND);
#endif
		prt->Send( &data_msg, DATA_MESSAGE_SIZE );

		data_msg.count = Report_VIs( data_msg.data.targets );
#ifdef BIG_ENDIAN_ARCH
		(void) reorder(data_msg, DATA_MESSAGE_TARGET_SPEC, SEND);
#endif
		prt->Send( &data_msg, DATA_MESSAGE_SIZE );
		break;
	// Setting targets for a given grunt and reporting back.
	case SET_TARGETS:
#ifdef _DETAILS
		cout << "in Process_Message() : SET_TARGETS" << endl;
#endif
		prt->Receive( &data_msg, DATA_MESSAGE_SIZE );
#ifdef BIG_ENDIAN_ARCH
		(void) reorder(data_msg, DATA_MESSAGE_TARGET_SPEC, RECV);
#endif
		msg.data = Set_Targets( msg.data, data_msg.count, 
			data_msg.data.targets );
		// Send back success/failure indication along with additional error
		// information or target settings (such as TCP port).
#ifdef BIG_ENDIAN_ARCH
		(void) reorder(msg);
#endif
		prt->Send( &msg );
#ifdef BIG_ENDIAN_ARCH
		(void) reorder(data_msg, DATA_MESSAGE_TARGET_SPEC, SEND);
#endif
		prt->Send( &data_msg, DATA_MESSAGE_SIZE );
		break;

	// Setting access specifications for next test.
	case SET_ACCESS:
#ifdef _DETAILS
		cout << "in Process_Message() : SET_ACCESS" << endl;
#endif
		prt->Receive( &data_msg, DATA_MESSAGE_SIZE );
#ifdef BIG_ENDIAN_ARCH
	(void) reorder(data_msg, DATA_MESSAGE_TEST_SPEC, RECV);
#endif
		msg.data = (int)Set_Access( msg.data, &(data_msg.data.spec) );
#ifdef BIG_ENDIAN_ARCH
		(void) reorder(msg);
#endif
		prt->Send( &msg );	// notify Iometer of success
		break;

	// Signalling start of test.
	case START:
#ifdef _DETAILS
		cout << "in Process_Message() : START" << endl;
#endif
		Start_Test( msg.data );
		break;
	// Beginning to perform I/O.
	case BEGIN_IO:
#ifdef _DETAILS
		cout << "in Process_Message() : BEGIN_IO" << endl;
#endif
		Begin_IO( msg.data );
		break;
	// Beginning recording of test results.
	case RECORD_ON:
#ifdef _DETAILS
		cout << "in Process_Message() : RECORD_ON" << endl;
#endif
		Record_On( msg.data );
		break;
	// Stopping recording of test results.
	case RECORD_OFF:
#ifdef _DETAILS
		cout << "in Process_Message() : RECORD_OFF" << endl;
#endif
		Record_Off( msg.data );
		break;
	// Signalling to stop testing.
	case STOP:
#ifdef _DETAILS
		cout << "in Process_Message() : STOP" << endl;
#endif
		Stop_Test( msg.data );
		break;

	// Reporting results of whole test to Iometer.
	case REPORT_RESULTS:
#ifdef _DETAILS
		cout << "in Process_Message() : REPORT_RESULTS" << endl;
#endif
		Report_Results( WHOLE_TEST_PERF );
		break;

	// Reporting results since last update to Iometer.
	case REPORT_UPDATE:
#ifdef _DETAILS
		cout << "in Process_Message() : REPORT_UPDATE" << endl;
#endif
		Report_Results( LAST_UPDATE_PERF );
		break;

	default:
		cout << "*** Unknown purpose found in message." << endl << flush;
		return FALSE;
	}
	return TRUE;
}


//
// Signalling all threads to start testing.
//
void Manager::Start_Test( int target )
{
	int g;

	cout << "Starting..." << endl << flush;

	// Start all the grunts.  This creates the grunt threads.
	if ( target == ALL_WORKERS )
	{
		for ( g = 0; g < grunt_count; g++ )
		{
			grunts[g]->Start_Test();
		}
	}
	else
	{
		grunts[target]->Start_Test();
	}
	#if _DEBUG
		cout << "   Started." << endl << flush;
	#endif
	// Reply that test has started.
	msg.data = TRUE;
	prt->Send( &msg );
}



//
// Signalling all threads to begin performing I/O.
//
void Manager::Begin_IO( int target )
{
	msg.data = TRUE;
	cout << "Beginning to perform I/O..." << endl << flush;

	if ( target == ALL_WORKERS )
	{
		for ( int i = 0; i < grunt_count; i++ )
		{
			grunts[i]->Begin_IO();
			if ( grunts[i]->critical_error )
				msg.data = FALSE;
		}
	}
	else
	{
		grunts[target]->Begin_IO();
		if ( grunts[target]->critical_error )
			msg.data = FALSE;
	}
	#if _DEBUG
		cout << "   Performing I/O." << endl << flush;
	#endif

	// Reply that I/O has started.
#ifdef BIG_ENDIAN_ARCH
	(void) reorder(msg);
#endif
	prt->Send( &msg );
}


//
// Signalling to stop testing.
//
void Manager::Stop_Test( int target )
{
	if ( target == ALL_WORKERS )
	{
		for ( int i = 0; i < grunt_count; i++ )
		{
			grunts[i]->Stop_Test();
		}
	}
	else
	{
		grunts[target]->Stop_Test();
	}

	cout << "Stopping..." << endl << flush;

	if ( target == ALL_WORKERS )
	{
		for ( int i = 0; i < grunt_count; i++ )
		{
			grunts[i]->Wait_For_Stop();
		}
	}
	else
	{
		grunts[target]->Wait_For_Stop();
	}

	cout << "   Stopped." << endl << flush;

	// Reply that test has stopped.
#ifdef BIG_ENDIAN_ARCH
	(void) reorder(msg);
#endif
	prt->Send( &msg );
}



//
// Signalling to stop disk preparation.
//
void Manager::Stop_Prepare( int target )
{
	cout << "Stopping..." << endl << flush;

	if ( target == ALL_WORKERS )
		for ( int i = 0; i < grunt_count; i++ )
			grunts[i]->grunt_state = TestIdle;
	else
		grunts[target]->grunt_state = TestIdle;

	cout << "   Stopped." << endl << flush;
}


//
// Received call to remove workers
//
void Manager::Remove_Workers( int target )
{
	// If we're removing just one worker, remove it and return.
	if ( target >= 0 && target < grunt_count )
	{
		// Remove a worker.  It does not matter which one.
		cout << "Removing worker " << target << "." << endl << flush;
		delete grunts[--grunt_count];
		return;
	}

	// Remove all workers.
	cout << "Removing all workers." << endl << flush;
	while ( grunt_count )
		delete grunts[--grunt_count];
}



//
// Setting the targets for one or more grunts.
//
BOOL Manager::Set_Targets( int worker_no, int count, Target_Spec* target_specs )
{
	if ( worker_no == ALL_WORKERS )
	{
		for ( int i = 0; i < grunt_count; i++ )
		{
			cout << "Worker " << i << " setting targets..." << endl;
			if ( !grunts[i]->Set_Targets( count, target_specs ) )
				return FALSE;
		}
	}
	else
	{
		cout << "Worker " << worker_no << " setting targets..." << endl;
		return grunts[worker_no]->Set_Targets( count, target_specs );
	}
	return TRUE;
}




//
// Setting access specifications for next test.
// Note that Iometer will call Set_Access before testing starts to ensure that
// Dynamo can run the spec with the largest transfer request.
//
BOOL Manager::Set_Access( int target, const Test_Spec *spec )
{
	int		g;			// loop control variable

	// Recursively assign all workers the same access specification.
	if ( target == ALL_WORKERS )
	{
		cout << "All workers running Access Spec: " << spec->name << endl;
		for ( g = 0; g < grunt_count; g++ )
		{
			if ( !Set_Access( g, spec ) )
				return FALSE;
		}
		return TRUE;
	}

	cout << "Worker " << target << " running Access Spec: " << spec->name << endl;

	// If the grunt could not set the access spec properly, return.
	// The grunt may not have been able to grow its data buffer.
	if ( !grunts[target]->Set_Access( spec ) )
		return FALSE;

	// If the grunt is not using the manager's data buffer or the manager's
	// buffer is already large enough, just return.
	if ( grunts[target]->data_size ||
		data_size >= grunts[target]->access_spec.max_transfer )
	{
		return TRUE;
	}

	// Grow the manager's data buffer and update all grunts using it.
	#if _DEBUG
		cout << "Growing manager data buffer from " << data_size << " to " 
			 << grunts[target]->access_spec.max_transfer << endl << flush;
	#endif

	// Align all data transfers on a page boundary.  This will work for all disks
	// with sector sizes that divide evenly into the page size - which is always
	// the case.
#if defined (_WIN32) || defined (_WIN64)
	VirtualFree( data, 0, MEM_RELEASE );
	if ( !(data = VirtualAlloc( NULL, grunts[target]->access_spec.max_transfer, 
		MEM_COMMIT, PAGE_READWRITE )) )
#else // UNIX
	free(data);
	errno = 0;
	if ( !(data = valloc(grunts[target]->access_spec.max_transfer) ))
#endif
	{
		// Could not allocate a larger buffer.  Signal failure.
		cout << "*** Manager could not allocate data buffer for I/O transfers." 
			<< endl << flush;
		data_size = 0;
		return FALSE;
	}
	data_size = grunts[target]->access_spec.max_transfer;

	// Update all grunts using the manager's data buffer.
	for ( g = 0; g < grunt_count; g++ )
	{
		if ( !grunts[g]->data_size )
		{
			grunts[g]->read_data = data;
			grunts[g]->write_data = data;
		}
	}
	return TRUE;
}



//
// Beginning recording of test results.
//
void Manager::Record_On( int target )
{
	record = TRUE;		// At least 1 worker is recording.

	#if _DEBUG
		cout << "Recording Started." << endl << flush;
	#endif

	if ( target == ALL_WORKERS )
	{
		for ( int i = 0; i < grunt_count; i++ )
		{
			grunts[i]->Record_On();
		}
	}
	else
	{
		grunts[target]->Record_On();
	}

	// Get performance data for start of test.
	Get_Performance( WHOLE_TEST_PERF, FIRST_SNAPSHOT );
	Get_Performance( LAST_UPDATE_PERF, FIRST_SNAPSHOT );
}



//
// Stopping recording of test results.
//
void Manager::Record_Off( int target )
{
	// Get performance data for end of test.
	Get_Performance( WHOLE_TEST_PERF, LAST_SNAPSHOT );
	Get_Performance( LAST_UPDATE_PERF, LAST_SNAPSHOT );

	if ( target == ALL_WORKERS )
	{
		for ( int i = 0; i < grunt_count; i++ )
		{
			grunts[i]->Record_Off();
		}
	}
	else
	{
		grunts[target]->Record_Off();
	}
	cout << "   Stopped." << endl << flush;

	record = FALSE;		// No workers are recording data.
	#if _DEBUG
		cout << "Recording stopped." << endl << flush;
	#endif
#ifdef BIG_ENDIAN_ARCH
	(void) reorder(msg);
#endif
	prt->Send( &msg );
}



//
// Starting a new grunt worker.  Sending a message back to Iometer indicating 
// the number of workers actually created.
//
void Manager::Add_Workers( int count )
{
	msg.data = 0;

	#if _DEBUG 
		cout << "Adding a new worker." << endl << flush;
	#endif

	for ( int i = 0; i < count; i++ )
	{
		// Create a new grunt.
		if ( !(grunts[grunt_count] = new Grunt) )
			break;

		// Assign grunt to manager's data buffer by default.
		grunts[grunt_count]->read_data = data;
		grunts[grunt_count]->write_data = data;
		grunt_count++;
		msg.data++;
	}

	// See if we successfully create all workers requested.
	if ( msg.data != count )
	{
		// Failed to create all workers.
		while ( msg.data )
		{
			Remove_Workers( grunt_count-- );
			msg.data--;
		}
	}

#ifdef BIG_ENDIAN_ARCH
	(void) reorder(msg);
#endif
	prt->Send( &msg );
}
