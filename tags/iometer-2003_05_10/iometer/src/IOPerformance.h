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
//                Copyright (C) 1998-2000 Intel Corporation
//                          All rights reserved                               
//                INTEL CORPORATION PROPRIETARY INFORMATION                   
//    This software is supplied under the terms of a license agreement or     
//    nondisclosure agreement with Intel Corporation and may not be copied    
//    or disclosed except in accordance with the terms of that agreement.     
// ==========================================================================
//
// IOPerformance.h: Interface for the Performance class, which gets
// all system-wide performance information (CPU and network) from the OS.
//
//////////////////////////////////////////////////////////////////////

#ifndef PERFORMANCE_DEFINED
#define PERFORMANCE_DEFINED


#include "IOCommon.h"
#include "IOTime.h"

#ifdef UNIX
#include <stdio.h>
#ifdef SOLARIS
#include <kstat.h>
#endif
#include <sys/sysinfo.h>
#include <sys/socket.h>
#ifdef SOLARIS
#include <sys/sockio.h>
#endif
#include <netinet/in.h>
#include <net/if.h>

#include <stropts.h>
#ifdef SOLARIS
#include <sys/stream.h>
#include <sys/tihdr.h>
#include <inet/mib2.h>
#endif
#endif

#define MAX_PERF_SIZE	4096	// Initial memory size allocated to NT performance data.


#if defined (_WIN32) || defined (_WIN64)
// Bit masks to expose information about performance counter types.
// See winperf.h for additional details about counter definitions.
#define PERF_SIZE_MASK		0x00000300
#define PERF_TYPE_MASK		0x00000C00
#define PERF_SUBTYPE_MASK	0x000F0000
#define PERF_TIMER_MASK		0x00300000
#define PERF_SUFFIX_MASK	0xF0000000
#endif // WIN32 || _WIN64

//
// NT performance counter indexes.  These indexes are accessible through
// the system registry.
//

// Processor performance counters.
#define PERF_PROCESSOR			238
// Specific processor counters.
#define PERF_CPU_TOTAL_UTILIZATION		6
#define PERF_CPU_USER_UTILIZATION		142
#define PERF_CPU_PRIVILEGED_UTILIZATION	144
#define PERF_CPU_IRQ					148
#define PERF_CPU_DPC_UTILIZATION		696
#define PERF_CPU_IRQ_UTILIZATION		698

// Network performance counters.
#define PERF_NETWORK_TCP				638
#define PERF_NETWORK_INTERFACE			510
// Specific network counters.
#define PERF_TCP_SEGMENTS_RESENT		656
#define PERF_NI_PACKETS					400
#define PERF_NI_OUT_ERRORS				540
#define PERF_NI_IN_ERRORS				528

#define MILLISECOND                     1000



struct Perf_Counter_Info
{
	DWORD	index;		// identification of counter
	int		offset;		// offset of counter from counter block header
	DWORD	type;		// information retrieved about the counter's type,
						// such as its update frequency, size, etc.
};



class Performance  
{
public:
	Performance();
	~Performance();

	void Get_Perf_Data( DWORD perf_data_type, int snapshot );
	void Calculate_CPU_Stats( CPU_Results *cpu_results );
	void Calculate_TCP_Stats( Net_Results *net_results );
	void Calculate_NI_Stats( Net_Results *net_results );

	double		processor_speed;		// Frequency (Hz) of system processors.
	int			processor_count;		// Number of system processors.
	int			network_interfaces;		// Number of (virtual) NICs in the system.

	__int64		time_counter[MAX_SNAPSHOTS];	// Time that NT performance snapshots were taken.
	int			clock_tick;
#ifdef SOLARIS
	double		timediff;
	kstat_ctl_t	*kc;
	char		nic_names[MAX_NUM_INTERFACES][IFNAMSIZ]; // array of pointers to hold network interface names.
	struct strbuf	strbuf_ctl, strbuf_data;
	int			streamfd;						// File descriptor to the stream.
	char		*ctlbuf, *databuf;
#elif defined(LINUX)
	long long timediff;
#else
	DWORDLONG timediff;
#endif

private:
	int			Get_Processor_Count();
	double		Get_Processor_Speed();

#if defined (_WIN32) || defined (_WIN64)
	void		Extract_Counters( DWORD perf_data_type, int snapshot );
	void		Extract_CPU_Counters( int snapshot );
	void		Extract_TCP_Counters( int snapshot );
	void		Extract_NI_Counters( int snapshot );
	__int64		Extract_Counter( const Perf_Counter_Info *counter_info );

	BOOL		Locate_Perf_Object( DWORD perf_object_index );
	BOOL		Locate_Perf_Instance( int instance = PERF_NO_INSTANCES );
	BOOL		Set_Counter_Info( DWORD perf_data_type );

	double		Calculate_Stat( __int64 start_value, __int64 end_value, DWORD counter_type );
#endif

#ifdef UNIX
	void		Get_CPU_Counters(int snapshot);
	void		Get_NI_Counters(int snapshot);
	void		Get_TCP_Counters(int snapshot);
#endif

	LPBYTE		perf_data;				// Pointer to performance data.
	DWORD		perf_size;				// Size of performance data buffer.
#if defined (_WIN32) || defined (_WIN64)
	PERF_OBJECT_TYPE	*perf_object;	// pointer to NT performance data for a specific object
	PERF_INSTANCE_DEFINITION	*perf_instance; // pointer to an instance of a specific object
	PERF_COUNTER_BLOCK	*perf_counters;	// pointer to NT performance counters for a specific instance of an object
#endif
	//
	// raw CPU performance counters for Windows NT
	//
	double		perf_time;				// Time that performance data was captured.
	__int64		raw_cpu_data[MAX_CPUS][CPU_RESULTS][MAX_SNAPSHOTS];
	__int64		raw_tcp_data[TCP_RESULTS][MAX_SNAPSHOTS];
	__int64		raw_ni_data[MAX_NUM_INTERFACES][NI_RESULTS][MAX_SNAPSHOTS];

	// List of counter information that should be extracted and stored.
	Perf_Counter_Info		cpu_perf_counter_info[CPU_RESULTS];
	Perf_Counter_Info		tcp_perf_counter_info[TCP_RESULTS];
	Perf_Counter_Info		ni_perf_counter_info[NI_RESULTS];
};



#endif
