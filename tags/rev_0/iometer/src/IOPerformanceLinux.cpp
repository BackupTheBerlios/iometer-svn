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
//                  Copyright (C) 1998-2000 Intel Corporation
//                          All rights reserved                               
//                INTEL CORPORATION PROPRIETARY INFORMATION                   
//    This software is supplied under the terms of a license agreement or     
//    nondisclosure agreement with Intel Corporation and may not be copied    
//    or disclosed except in accordance with the terms of that agreement.     
// ==========================================================================
//
// IOPerformanceLinux.cpp: Implementation of the Performance class, which
// gets all system-wide performance information (CPU and network) from the
// OS, for Linux.
//
//////////////////////////////////////////////////////////////////////

#define PERFORMANCE_DETAILS	0 // Turn on to display additional performance messages.

#ifdef LINUX

#include <assert.h>
#include "IOPerformance.h"

//
// Initializing system performance data.
//
Performance::Performance() {
	// Obtaining the number of CPUs in the system and their speed.
	processor_count = Get_Processor_Count();
	processor_speed = Get_Processor_Speed();

	// Network performance statistics are disabled by default.  Assume this
	// unless later performance calls succeed.  Then, set the correct number of
	// interfaces.
	network_interfaces = 0;

	if (!processor_speed || !processor_count) {
		cout << "*** Unable to initialize needed performance data.\n"
				 << "This error may indicate that you are trying to run on an\n"
				 << "unsupported processor or OS.  See the Iometer User's Guide for\n"
				 << "information on supported platforms.\n";
		exit(1);
	}

	// Initialize all the arrays to 0.
	memset(raw_cpu_data, 0,
				 (MAX_CPUS * CPU_RESULTS * MAX_SNAPSHOTS * sizeof(_int64)));
	memset(raw_ni_data, 0,
				 (MAX_NUM_INTERFACES * NI_RESULTS * MAX_SNAPSHOTS * sizeof(_int64)));
	memset(raw_tcp_data, 0,
				 (TCP_RESULTS * MAX_SNAPSHOTS * sizeof(_int64)));
}



//
// Freeing memory used by class object.
//
Performance::~Performance() {
}



//
// Getting the number of processors in the system.
//
int Performance::Get_Processor_Count()
{
	// The file "/proc/stat" enumerates the CPUs, one per line, each line starting
	// with the string "cpu". I prepend a '\n' to the file, then search for the
	// string "\ncpu"; that way, I will find every time that "cpu" appears as the
	// first thing in a line.
	//
	// Example /proc/stat file: (Note: On a two CPU system, you will see two
	// cpu lines, beginning "cpu0 ..." and "cpu1 ...")
	//////////////////////////////////////////////////////////////////////
	// cpu  1847686 355419 509281 14738354
	// disk 69683 0 22232 0
	// disk_rio 18236 0 302 0
	// disk_wio 51447 0 21930 0
	// disk_rblk 145876 0 2374 0
	// disk_wblk 411576 0 173352 0
	// page 76934 442943
	// swap 1410 2465
	// intr 21377391 17450740 101201 0 2 2 0 3 0 1 0 0 69197 724623 1 340233 2691388 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
	// ctxt 175242460
	// btime 942875987
	//////////////////////////////////////////////////////////////////////
	char stats[16 * 1024 + 1], *search;
	int fd, byteCount, cpuCount;
	fd = open("/proc/stat", O_RDONLY);
	if (fd < 0) {
	  cout << "*** Unable to determine number of processors in system.";
	  return 0;
	}
	byteCount = read(fd, stats + 1, sizeof(stats) - 1);
	close(fd);
	if ((byteCount < 0) || (byteCount == sizeof(stats) - 1)) {
	  cout << "*** Unable to determine number of processors in system.";
	  return 0;
	}
	stats[0] = '\n'; // Make the first line begin with a \n, like the others.
	stats[byteCount + 1] = '\0';
	search = stats;
	cpuCount = 0;
	while ((search = strstr(search, "\ncpu")) != NULL) {
	  ++cpuCount;
	  ++search; // Make sure we don't find the same CPU again!
	}
	return(cpuCount);
}



//
// Getting the speed of the processors in Hz. We get this from /proc/cpuinfo.
// Example output of /proc/cpuinfo:
//////////////////////////////////////////////////////////////////////
// processor	: 0
// vendor_id	: GenuineIntel
// cpu family	: 6
// model		: 7
// model name	: Pentium III (Katmai)
// stepping	: 3
// cpu MHz		: 497.845315
// cache size	: 512 KB
// ... file continues on ...
//////////////////////////////////////////////////////////////////////
// Note: We just take the first CPU we find and return its CPU speed.
double Performance::Get_Processor_Speed()
{
	int c;
	char label[40];
	int scanDecodes;
	double result;
	FILE *cpuInfo = fopen("/proc/cpuinfo", "r");

	do {
		fscanf(cpuInfo, "%7c", label);
		if (!strncmp(label, "cpu MHz", 7)) {
			scanDecodes = fscanf(cpuInfo, "%*s %lf", &result);
			result *= 1000000.0;
			fclose(cpuInfo);
			if (scanDecodes == 1) {
				return(result);
			} else {
				cerr << "Error determining CPU speed.\n";
				return(0.0);
			}
		}
		// Skip to the next line.
		do {
			c = getc(cpuInfo);
		} while ((c != '\n') && (c != EOF));
	} while (c != EOF);
	fclose(cpuInfo);
	cerr << "Error determining CPU speed.\n";
	return(0.0);
}	


void Performance::Get_Perf_Data(DWORD perf_data_type, int snapshot) {
	// Get the performance data stored by the system.
#if _DEBUG
	cout << "   Getting system performance data." << endl << flush;
#endif

	time_counter[snapshot] = rdtsc();
	if (snapshot == LAST_SNAPSHOT) {
		// calculate time diff in clock ticks..
		timediff = (time_counter[LAST_SNAPSHOT] -
								time_counter[FIRST_SNAPSHOT]);
	}
	
	switch (perf_data_type)	{
	case PERF_PROCESSOR:
		Get_CPU_Counters(snapshot);
		break;
	case PERF_NETWORK_TCP:
		Get_TCP_Counters(snapshot);
		break;
	case PERF_NETWORK_INTERFACE:
		Get_NI_Counters(snapshot);
		break;
	default:
		break;
	}
}

void Performance::Get_CPU_Counters(int snapshot)
{
	int	numScans, c;
	char tmpBuf[40];
	
	//
	// See the comment on Get_Procesor_Count, above, for an example of the
	// "/proc/stat" output.
	//
	FILE *cpuStat = fopen("/proc/stat", "r");
	assert(cpuStat != NULL);
	for (processor_count = 0;
			 processor_count < MAX_CPUS; 
			 ++processor_count) {
		numScans = fscanf(cpuStat, "%s %lld %*s %lld",
											tmpBuf,
											&raw_cpu_data[processor_count]
											             [CPU_USER_UTILIZATION][snapshot],
											&raw_cpu_data[processor_count]
											             [CPU_PRIVILEGED_UTILIZATION][snapshot]);
		if ((numScans != 3) || (strncmp(tmpBuf, "cpu", 3) != 0)) {
			// We're done reading CPU data. Time to leave this loop.
			break;
		}
		do {
			c = getc(cpuStat);
		} while ((c != '\n') && (c != EOF));
	}
	fclose(cpuStat);
	if (processor_count > 1) {
		--processor_count; /* We'll have "cpu " and "cpu0"..."cpun". */
	}
	//
	// We get out interrupt info from "/proc/interrupts". Sample output:
	//////////////////////////////////////////////////////////////////////
	//            CPU0       
  //   0:   17469505          XT-PIC  timer
  //   1:     103125          XT-PIC  keyboard
  //   2:          0          XT-PIC  cascade
  //   8:          1          XT-PIC  rtc
	//  11:      69292          XT-PIC  aic7xxx
	//  12:     725412          XT-PIC  PS/2 Mouse
	//  13:          1          XT-PIC  fpu
	//  14:     340442          XT-PIC  Intel EtherExpress Pro 10/100 Ethernet
	//  15:    2691388          XT-PIC  ide1
	// NMI:          0
	//////////////////////////////////////////////////////////////////////

	FILE *interStat = fopen("/proc/interrupts", "r");
	assert(interStat != NULL);
	for (;;) {
		int result;

		do {
			c = getc(interStat);
		} while ((c != '\n') && (c != EOF));
		/*
		 * The following code is based on the assumption that any IRQ
		 * entry in /proc/interrupts will begin with a space followed by
		 * an integer.  Otherwise it's an "NMI" or "ERR".  This is
		 * important, because NMI and ERR don't have per-processor
		 * values, only one value, so they'll break in the
		 * processor_count loop below.  If this code breaks, check that
		 * "cat /proc/interrupts" looks like the comment above, and
		 * change this code to match if it doesn't.
		 */
		if (getc(interStat) != ' ') {
			break;
		}
		if (fscanf(interStat, "%d:", &result) != 1) {
			break;
		}
		for (int i = 0; i < processor_count; ++i) {
			long long interruptCount;
			numScans = fscanf(interStat, "%lld", &interruptCount);
			assert(numScans == 1);
			raw_cpu_data[i][CPU_IRQ][snapshot] += interruptCount;
		}
	}
	fclose(interStat);
}

void Performance::Get_TCP_Counters(int snapshot) {}


//
// Calculating the performance value for a given counter pair based on its saved information.
//
#if defined (_WIN32) || defined (_WIN64)
double Performance::Calculate_Stat( _int64 start_value, _int64 end_value, DWORD counter_type )
{
	double	count_difference;	// Difference between two snapshots of a counter.
	double	perf_stat;			// Calculated performance statistic, such as % utilization.

	// Verify valid performance time.
	if ( perf_time <= 0 )
	{
		cout << "*** Performance time not positive." << endl;
		return (double)0.0;
	}

	// Determine what type of counter we're dealing with.
	switch ( counter_type & PERF_TYPE_MASK )
	{

	// Dealing with a number (e.g. error count)
	case PERF_TYPE_NUMBER:
		#if PERFORMANCE_DETAILS
			cout << "Performing calculation on a performance number." << endl;
		#endif
		switch ( counter_type & PERF_SUBTYPE_MASK )
		{
		case PERF_NUMBER_DECIMAL:
			#if PERFORMANCE_DETAILS
				cout << "Performance number decimal." << endl;
			#endif
			perf_stat = (double)(end_value - start_value);
			break;
		case PERF_NUMBER_DEC_1000:	// divide result by 1000 before displaying
			#if PERFORMANCE_DETAILS
				cout << "Performance number decimal x 1000." << endl;
			#endif
			perf_stat = (double)(end_value - start_value) / (double)1000.0;
			break;
		// other subtypes exist, but are not handled
		default:
			cout << "*** Performance counter number subtype not handled" << endl;
			return (double)0.0;
		}
		break;

	// Dealing with an actual counter value.
	case PERF_TYPE_COUNTER:
		#if PERFORMANCE_DETAILS
			cout << "Performing calculation on a performance counter." << endl;
		#endif

		// Computing the difference between the two counters.
		if ( counter_type & PERF_DELTA_COUNTER )
		{
			#if PERFORMANCE_DETAILS
				cout << "Performance delta counter." << endl;
			#endif
			count_difference = (double)(end_value - start_value);
		}
		else
		{
			cout << "*** Unhandled performance counter delta type." << endl;
			return (double)0.0;
		}

		// Determine the calculated value based on the counter's subtype.
		switch ( counter_type & PERF_SUBTYPE_MASK )
		{

		// Divide the counter by the time difference
		case PERF_COUNTER_RATE:
			#if PERFORMANCE_DETAILS
				cout << "Performance counter is a rate." << endl;
			#endif

			// Adjust the time based on the counter's update frequency.
			switch ( counter_type & PERF_TIMER_MASK )
			{
			// Timer is based on the system timer, just use it.
			case PERF_TIMER_TICK:
				#if PERFORMANCE_DETAILS
					cout << "Using system timer for update frequency." << endl;
				#endif
				perf_stat = count_difference / perf_time;
				break;

			// These timers are based on a 100-ns timer, so multiply timer by 10,000,000 to get seconds.
			case PERF_TIMER_100NS:
				#if PERFORMANCE_DETAILS
					cout << "Using 100-ns timer for update frequency." << endl;
				#endif
				perf_stat = count_difference / (perf_time * (double)10000000.0);
				break;

			// other timer rates exist, but are not handled.
			default:
				cout << "*** Performance update frequency type not handled." << endl;
				return (double)0.0;
			}

			break;
		default:
			cout << "*** Unhandled performance counter, counter subtype." << endl;
			return (double)0.0;
		}

		break;

	// other types exist, but are not handled
	default:
		cout << "*** Unhandled performance counter type." << endl;
		return (double)0.0;
	}

	// Handle any inverted performance counters.
	if ( counter_type & PERF_INVERSE_COUNTER )
	{
		#if PERFORMANCE_DETAILS
			cout << "Inverting performance counter (1 - value)." << endl;
		#endif
		perf_stat = (double) 1.0 - perf_stat;
	}

	// Perform any additional operations needed based on the counter's display suffix.
	switch ( counter_type & PERF_SUFFIX_MASK )
	{
	case PERF_DISPLAY_NO_SUFFIX:		// just return value
	case PERF_DISPLAY_PER_SEC:			// no additional calculations needed
		return perf_stat;

	case PERF_DISPLAY_PERCENT:			// scale by 100 to reflect a percentage
		#if PERFORMANCE_DETAILS
			cout << "Performance counter is a percent." << endl;
		#endif
		// Verify valid values.  
		// Do not print an error message unless it's more than trivially invalid.
		if ( perf_stat < (double)0.0 ) 
		{
			if ( perf_stat < (double)-0.001 ) 
			{
				cout << "*** Performance counter percentage is less than zero: " 
					 << perf_stat << endl;
			}
			return (double)0.0;
		}
		else if ( perf_stat > (double)1.0 )
		{
			if ( perf_stat > (double)1.001 ) 
			{
				cout << "*** Performance counter percentage is greater than 1: " 
					 << perf_stat << endl;
			}
			return (double)100.0;
		}

		return (perf_stat * (double)100.0);

	default:
		cout << "*** Unhandled performance display suffix." << endl;
		return (double)0.0;
	}
}
#endif



#if defined (_WIN32) || defined (_WIN64)
//
// Extracting counters for NT CPU performance data.
//
void Performance::Extract_CPU_Counters( int snapshot )
{
	char	cpu_name[3];
	int		cpu, stat, i;

	// Loop through all processors and record performance information.
	for ( cpu = 0; cpu < processor_count; cpu++ )
	{
		// Find the desired processor instance.
		if ( !Locate_Perf_Instance( cpu ) )
			return;

		// Verify that the instance found is for the current processor, 
		// otherwise perform an enhaustive search.
		_itoa( cpu, cpu_name, 10 );
		if ( strncmp( cpu_name, (char*)((LPBYTE)perf_instance + perf_instance->NameOffset), 2 ) )
		{
			#if _DEBUG
				cout << "Performing exhaustive search for processor instance " << cpu << endl;
			#endif

			// Check all processor instances and try to match one with the desired processor.
			for ( i = 0; i < perf_object->NumInstances; i++ )
			{
				if ( !Locate_Perf_Instance( i ) )
					return;
				#if PERFORMANCE_DETAILS || _DETAILS
					cout << "Looking at processor name: " 
						<< (char*)((LPBYTE)perf_instance + perf_instance->NameOffset) << endl;
				#endif
				// Match the name of the current instance with the name of the desired cpu.
				if ( !strncmp( cpu_name, (char*)((LPBYTE)perf_instance + perf_instance->NameOffset), 2 ) )
					break;	// Found the correct instance.
			}
			if ( i == perf_object->NumInstances )
			{
				cout << "*** Unable to locate performance instance of processor " << cpu << endl;
				return;
			}
		}

		// Saving CPU specific counters.
		for ( stat = 0; stat < CPU_RESULTS; stat++ )
		{
			#if PERFORMANCE_DETAILS || _DETAILS
				cout << "Extracting CPU stat " << stat << " for CPU " << cpu << endl;
			#endif
			raw_cpu_data[cpu][stat][snapshot] = Extract_Counter( &(cpu_perf_counter_info[stat]) );
		}
	}
}
#endif


//
// Calculating CPU statistics based on snapshots of performance counters.
//
void Performance::Calculate_CPU_Stats( CPU_Results *cpu_results )
{
	int		cpu, stat;		// Loop control variables.

	// Loop though all CPUs and determine various utilization statistics.
	cpu_results->count = processor_count;
	for ( cpu = 0; cpu < processor_count; cpu++ )
	{
		// Loop through the counters and calculate performance.
		for ( stat = 0; stat < CPU_RESULTS; stat++ )
		{
			#if PERFORMANCE_DETAILS
				cout << "Calculating stat " << stat << " for CPU " << cpu << endl;
			#endif

#if defined (_WIN32) || defined (_WIN64)
			// If we've never set the counter offsets, then we've never successfully retrieved
			// the performance data.  Set all of the values to 0.
			if ( cpu_perf_counter_info[stat].offset == IOERROR )
			{
				cout << "*** Offset to CPU performance counter not defined for stat " 
					 << stat << "." << endl;
				cpu_results->CPU_utilization[cpu][stat] = (double)0.0;
			}
			else
			{
				cpu_results->CPU_utilization[cpu][stat] = Calculate_Stat( 
					raw_cpu_data[cpu][stat][FIRST_SNAPSHOT],
					raw_cpu_data[cpu][stat][LAST_SNAPSHOT],
					cpu_perf_counter_info[stat].type );
			}
#else // UNIX
			double result;
			if (stat == CPU_IRQ)
			{
				// we have to calculate Interrupts/sec.
				// This is similar to calculating Network packets per second
				// but we are more fortunate here.
				// See the corresponding Notes at the end of this file for a description.
				//
				result = ((double) raw_cpu_data[cpu][stat][LAST_SNAPSHOT]
						 - raw_cpu_data[cpu][stat][FIRST_SNAPSHOT]) * 
						 clock_tick / timediff;
				cpu_results->CPU_utilization[cpu][stat] = result;	
			}
			else
			{
				// All other CPU statistics.
				result = ((double) raw_cpu_data[cpu][stat][LAST_SNAPSHOT]
							- raw_cpu_data[cpu][stat][FIRST_SNAPSHOT]) / timediff;

				if (result < 0.0) 
				{
					result = 0.0;
					//
					// CPU Utilization figures are outside valid range far too often.
					// Ok, not in every cycle but frequent still.
					// So, it is better to comment it out rather than have the message
					// pop up on the screen at regular intervals.
					//
					// cout << "***** Error : CPU utilization outside valid range 0% - 100% *****" << endl;
				}
				if  (result > 1.0)
				{
					result = 1.0;
				}

				cpu_results->CPU_utilization[cpu][stat] = (result * 100);
			}
#endif
			#if PERFORMANCE_DETAILS || _DETAILS
				cout << "CPU " << cpu << " recorded stat " << stat << " = " 
					<< cpu_results->CPU_utilization[cpu][stat] << endl;
			#endif
		}
	}
}


#if defined (_WIN32) || defined (_WIN64)
//
// Extracting counters for NT network performance data.
//
void Performance::Extract_TCP_Counters( int snapshot )
{
	int		stat;

	// Saving network TCP specific counters.
	if ( !Locate_Perf_Instance() )
		return;
	for ( stat = 0; stat < TCP_RESULTS; stat++ )
	{
		#if PERFORMANCE_DETAILS || _DETAILS
			cout << "Extracting TCP stat " << stat << endl;
		#endif
		raw_tcp_data[stat][snapshot] = Extract_Counter( &(tcp_perf_counter_info[stat]) );
	}
}
#endif


//
// Calculate network performance statistics based on snapshots of performance counters.
//
void Performance::Calculate_TCP_Stats( Net_Results *net_results )
{
	int		stat;		// Loop control variable.

	// Loop through the counters and calculate performance.
	for ( stat = 0; stat < TCP_RESULTS; stat++ )
	{
		// If we've never set the counter offsets, then we've never successfully retrieved
		// the performance data.  Set all of the values to 0.
#if defined (_WIN32) || defined (_WIN64)
		if ( tcp_perf_counter_info[stat].offset == IOERROR )
		{
			net_results->tcp_stats[stat] = (double)0.0;
		}
		else
		{
			net_results->tcp_stats[stat] = Calculate_Stat( 
				raw_tcp_data[stat][FIRST_SNAPSHOT],
				raw_tcp_data[stat][LAST_SNAPSHOT],
				tcp_perf_counter_info[stat].type );
		}
#else
		double result;
		result = ((double) raw_tcp_data[stat][LAST_SNAPSHOT] - 
			raw_tcp_data[stat][FIRST_SNAPSHOT]) / timediff;
		result *= clock_tick;		// note that timediff is in CLK_TCKs and not seconds
		net_results->tcp_stats[stat] = result;
#endif

		#if PERFORMANCE_DETAILS || _DETAILS
			cout << "TCP recorded stat " << stat << " = " 
				<< net_results->tcp_stats[stat] << endl;
		#endif
	}
}


//
// Extracting counters for NT network interface performance data. This is done
// by reading the file "/proc/net/dev". WARNING: This file changes from one
// version of Linux to the next. This one will work on linux 2.2.12; no
// guarantees can be made for other versions!
//
// Example output from /proc/net/dev:
//////////////////////////////////////////////////////////////////////
// Inter-|   Receive                                                |  Transmit
//  face |bytes    packets errs drop fifo frame compressed multicast|bytes    packets errs drop fifo colls carrier compressed
//     lo:    4632      76    0    0    0     0          0         0     4632      76    0    0    0     0       0          0
//   eth0:30165814  176832    0    0    0     0          0         0  3700725   27219    0    0    0   288       0          0
//////////////////////////////////////////////////////////////////////
// Note: "lo" is the loopback device, and "eth0" is an ethernet card.

void Performance::Get_NI_Counters(int snapshot) {
	int c, scanCount, packetIn, packetOut;

	FILE *netInfo = fopen("/proc/net/dev", "r");
	assert(netInfo != NULL);
	// Pull out the first two lines of the file. These two lines contain
	// labels for the columns.
	for (int i = 0; i < 2; ++i) {
		do {
			c = getc(netInfo);
		} while ((c != '\n') && (c != EOF));
	}
	for (network_interfaces = 0;
			 network_interfaces < MAX_NUM_INTERFACES;
			 ++network_interfaces) {
		// Take out the leading white space.
		scanCount = fscanf(netInfo, "%*[^:]: %*d %d %lld %*d %*d %*d %*d %d %lld",
											 &packetIn,
											 &raw_ni_data[network_interfaces]
											             [NI_IN_ERRORS][snapshot],
											 &packetOut,
											 &raw_ni_data[network_interfaces]
											             [NI_OUT_ERRORS][snapshot]);
		if (scanCount == EOF) {
			fclose(netInfo);
			return;
		}
		assert(scanCount == 4);
		raw_ni_data[network_interfaces][NI_PACKETS][snapshot] =
			packetIn + packetOut;
		// Skip to the next line.
		do {
			c = getc(netInfo);
		} while ((c != '\n') && (c != EOF));
	}
	fclose(netInfo);
}


//
// Calculate network performance statistics based on snapshots of performance counters.
//
void Performance::Calculate_NI_Stats( Net_Results *net_results )
{
	int		net, stat;		// Loop control variables.

	// Loop through the counters and calculate performance.
	net_results->ni_count = network_interfaces;
	for ( net = 0; net < network_interfaces; net++ )
	{
		for ( stat = 0; stat < NI_RESULTS; stat++ )
		{
			// If we've never set the counter offsets, then we've never successfully retrieved
			// the performance data.  Set all of the values to 0.
#if defined (_WIN32) || defined (_WIN64)
			if ( ni_perf_counter_info[stat].offset == IOERROR )
			{
				net_results->ni_stats[net][stat] = (double)0.0;
			}
			else
			{
				net_results->ni_stats[net][stat] = Calculate_Stat( 
					raw_ni_data[net][stat][FIRST_SNAPSHOT],
					raw_ni_data[net][stat][LAST_SNAPSHOT],
					ni_perf_counter_info[stat].type );
			}
#else // UNIX
			double result;
			//
			// Note:
			//		The array time_counter[] stores time in nanoseconds.
			// Earlier, we used to divide by the calculated value of timediff and then
			// multiply the result by clock_ticks per second to get the NI_data per
			// second which was theoretically correct (and mathematically same as what 
			// we are doing now) but reported wrong values while working with such 
			// large numbers.
			//
			result = ((double) raw_ni_data[net][stat][LAST_SNAPSHOT] - 
				raw_ni_data[net][stat][FIRST_SNAPSHOT]) * 1000000000.0 / 
				((double) time_counter[LAST_SNAPSHOT] - time_counter[FIRST_SNAPSHOT]);

			net_results->ni_stats[net][stat] = result;
#endif
			#if PERFORMANCE_DETAILS || _DETAILS
				cout << "   Network interface " << net << " recorded stat " << stat << " = " 
					<< net_results->ni_stats[net][stat] << endl;
			#endif
		}
	}
}

#endif /* LINUX */
