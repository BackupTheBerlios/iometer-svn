/* ######################################################################### */
/* ##                                                                     ## */
/* ##  Dynamo / IOPerformanceLinux.cpp                                    ## */
/* ##                                                                     ## */
/* ## ------------------------------------------------------------------- ## */
/* ##                                                                     ## */
/* ##  Job .......: The GNU/Linux variant of the Performance class.       ## */
/* ##               This class does two jobs:                             ## */
/* ##               1.) Collects basic informations like the number       ## */
/* ##                   of CPU's.                                         ## */
/* ##               2.) Collection of system-wide performance             ## */
/* ##                   informations.                                     ## */
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
/* ##  Remarks ...: - This content of this file should be merged into     ## */
/* ##                 the generic IOPerformance.cpp file.                 ## */
/* ##                                                                     ## */
/* ## ------------------------------------------------------------------- ## */
/* ##                                                                     ## */
/* ##  Changes ...: 2005-01-12 (henryx.w.tieman@intel.com)                ## */
/* ##               - Ia64 (Itanium) can get its CPU data the same as     ## */
/* ##                 i386.                                               ## */
/* ##               2004-09-01 (henryx.w.tieman@intel.com)                ## */
/* ##               - X86_64 can get its CPU data the same as i368.       ## */
/* ##               2004-03-27 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Applied Dan Bar Dov's patch for adding              ## */
/* ##                 Linux on PPC support.                               ## */
/* ##               2004-02-15 (mingz@ele.uri.edu)                        ## */
/* ##               - Bugfix for Get_NI_Counters to parse /proc/net/dev   ## */
/* ##                 with correct format.                                ## */
/* ##               2004-02-07 (mingz@ele.uri.edu)                        ## */
/* ##               - Changed all im_kstat to iomtr_kstat with Daniel     ## */
/* ##                 suggestion. A better name.                          ## */
/* ##               2004-02-06 (mingz@ele.uri.edu)                        ## */
/* ##               - Added im_kstat ioctl call in Get_Processor_Count.   ## */
/* ##               - Added im_kstat ioctl call in Get_Processor_Speed.   ## */
/* ##               - Added im_kstat ioctl call in Get_CPU_Counters.      ## */
/* ##               - Handle two /proc/stat style in Get_CPU_Counters.    ## */
/* ##               - Implement Get_TCP_Counters.                         ## */
/* ##               - Added im_kstat ioctl call in Get_NI_Counters.       ## */
/* ##               - Fixed a time difference computation error in        ## */
/* ##                 Calculate_NI_Stats() since time_counter unit is     ## */
/* ##                 not nanosecond but clock_tick                       ## */
/* ##               2003-10-15 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Moved to the use of the IOMTR_[OSFAMILY|OS|CPU]_*   ## */
/* ##                 global defines.                                     ## */
/* ##               - Integrated the License Statement into this header.  ## */
/* ##               2003-03-02 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Cleaned the code from Windows related stuff         ## */
/* ##                 (doesn't make sense in a pure Linux context).       ## */
/* ##               2003-03-02 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Bugfix for Get_NI_Counters to ensure, that          ## */
/* ##                 interface names with no leading blanks (for         ## */
/* ##                 example dummy0) are handled as well.                ## */
/* ##               2003-02-26 (joe@eiler.net)                            ## */
/* ##               - Added some more GHz processor stuff.                ## */
/* ##               - Changed Get_NI_Counters so interfaces that do not   ## */
/* ##                 contain statistics in /proc/net/dev do not cause    ## */
/* ##                 dynamo to die.  Right now the only interface that   ## */
/* ##                 does this is for Intel Link Aggregation (ians)      ## */
/* ##               2003-02-09 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Modified Get_CPU_Counters to add the Jiffies for    ## */
/* ##                 user mode with low priority (nice) to the user      ## */
/* ##                 mode utilization as well as the total utilization.  ## */
/* ##               2003-02-02 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Added new header holding the changelog.             ## */
/* ##               - Applied the iometer-initial-datatypes.patch file    ## */
/* ##                 (brings some minor changes to main() function).     ## */
/* ##                                                                     ## */
/* ######################################################################### */
#define PERFORMANCE_DETAILS	0   // Turn on to display additional performance messages.
#if defined(IOMTR_OS_LINUX)


#include <sys/time.h>
#include <assert.h>

#include "IOPerformance.h"
#include "iomtr_kstat/iomtr_kstat.h"

extern int kstatfd;
extern int procstatstyle;





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

        // Set the clock ticks per second
	// TODO: try to find a correct way to get this number
	// DF: Base on ''man sysconf'', this macro is obsolete now. so things become tricky
	clock_tick = sysconf(_SC_CLK_TCK);
	
	// Initialize all the arrays to 0.
	memset(raw_cpu_data, 0, (MAX_CPUS * CPU_RESULTS * MAX_SNAPSHOTS * sizeof(_int64)));
	memset(raw_ni_data,  0, (MAX_NUM_INTERFACES * NI_RESULTS * MAX_SNAPSHOTS * sizeof(_int64)));
	memset(raw_tcp_data, 0, (TCP_RESULTS * MAX_SNAPSHOTS * sizeof(_int64)));
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

	int nr_cpu = 0;
	if (kstatfd > 0 && ioctl(kstatfd, IM_IOC_GETCPUNUM, &nr_cpu) >= 0) {
		cout << "# CPU: " << nr_cpu << endl;
		return nr_cpu;
	}

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

        // Decrease result by one, because on a single processor
	// machine, the code above counts "cpu" and "cpu0"!
	cpuCount--;
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
#if defined(IOMTR_CPU_I386) || defined(IOMTR_CPU_XSCALE) || defined(IOMTR_CPU_X86_64) || defined(IOMTR_CPU_IA64)
	int c;
	char label[40];
	int scanDecodes;
	double result;
	FILE *cpuInfo;

	int khz;
	if (kstatfd > 0 && ioctl(kstatfd, IM_IOC_GETCPUKHZ, &khz) >= 0) {
		cout << "CPU KHZ: " << khz << endl;
		return (double)khz * 1000.0;
	}
	
	cpuInfo = fopen("/proc/cpuinfo", "r");
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
		else if (!strncmp(label, "cpu GHz", 7)) {
			scanDecodes = fscanf(cpuInfo, "%*s %lf", &result);
			result *= 1000000000.0;
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
#elif defined(IOMTR_CPU_PPC)
	#define USEC(tv)	(tv.tv_sec*1000000+tv.tv_usec)
	
	struct timeval tv1, tv2;
	DWORD t1, t2;

	double result;

	gettimeofday( &tv1, NULL );
	t1 = get_tbl();
	sleep(1);
	t2 = get_tbl();
	gettimeofday( &tv2, NULL );
	
	result = ((double)(t2 - t1)) * (1000000.0 / (double)(USEC(tv2) - USEC(tv1)));

	return( result );
#else
 #warning ===> WARNING: You have to do some coding here to get the port done!
#endif
}	


void Performance::Get_Perf_Data(DWORD perf_data_type, int snapshot) {
	// Get the performance data stored by the system.
#if _DEBUG
	cout << "   Getting system performance data." << endl << flush;
#endif

	time_counter[snapshot] = jiffies();
	if (snapshot == LAST_SNAPSHOT) {
		// calculate time diff in clock ticks..
		timediff = (time_counter[LAST_SNAPSHOT] - time_counter[FIRST_SNAPSHOT]);
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
	int	  numScans, c, i;
	char      tmpBuf[100];
	__int64   jiffiesCpuNiceUtilization, jiffiesCpuidle, jiffiesCpuiowait, jiffiesCpuirq, jiffiesCpusoftirq;
	FILE *cpuStat;
	struct cpu_data_type raw_cpu;
	
	if (kstatfd > 0 && ioctl(kstatfd, IM_IOC_GETCPUDATA, &raw_cpu) >= 0) {
		for (i = 0; i < processor_count; i++) {
			raw_cpu_data[i][CPU_USER_UTILIZATION][snapshot] = raw_cpu.user_time[i];
			raw_cpu_data[i][CPU_PRIVILEGED_UTILIZATION][snapshot] = raw_cpu.system_time[i];
			raw_cpu_data[i][CPU_TOTAL_UTILIZATION][snapshot] = raw_cpu_data[i][CPU_USER_UTILIZATION][snapshot] + raw_cpu_data[i][CPU_PRIVILEGED_UTILIZATION][snapshot];
		}
		raw_cpu_data[0][CPU_IRQ][snapshot] = raw_cpu.intr;
		return;
	}

	cpuStat = fopen("/proc/stat", "r");
	assert(cpuStat != NULL);
	switch (procstatstyle) {
	case PROCSTAT24STYLE:
		// First few rows of the /proc/stat output in 2.4 style.
		// --------------------------------------------------------------------
		// cpu  20969 2260 50042 1069377
		// cpu0 20969 2260 50042 1069377
		// page 1279455 540189
		// swap 30 483
		// intr 1896409 1142648 2 0 0 128558 0 0 0 2 0 34836 465132 4 0 91210 34017 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
		// ...
		// --------------------------------------------------------------------
		fscanf(cpuStat, "cpu %*s %*s %*s %*s\n");	
		for(i = 0; i < processor_count; i++) {
			numScans = fscanf(cpuStat, "%s", tmpBuf);
			if((numScans == 1) && (strlen(tmpBuf) >= 3) && (strncmp(tmpBuf, "cpu", 3) == 0)) {
				fscanf(cpuStat, " %lld %lld %lld %*s\n", &raw_cpu_data[i][CPU_USER_UTILIZATION][snapshot],
									&jiffiesCpuNiceUtilization,
									&raw_cpu_data[i][CPU_PRIVILEGED_UTILIZATION][snapshot]);
				raw_cpu_data[i][CPU_USER_UTILIZATION][snapshot] += jiffiesCpuNiceUtilization;
				raw_cpu_data[i][CPU_TOTAL_UTILIZATION][snapshot] = raw_cpu_data[i][CPU_USER_UTILIZATION][snapshot] + raw_cpu_data[i][CPU_PRIVILEGED_UTILIZATION][snapshot];
				continue;
			}
			do {
				c = getc(cpuStat);
			} while ((c != '\n') && (c != EOF));
			break;
		}
		for(;;) {
			numScans = fscanf(cpuStat, "%s", tmpBuf);
			if((numScans == 1) && (strlen(tmpBuf) == 4) && (strncmp(tmpBuf, "intr", 4) == 0)) {
				fscanf(cpuStat, " %lld ", &raw_cpu_data[0][CPU_IRQ][snapshot]);
				break;
			}		
			do {
				c = getc(cpuStat);
			} while ((c != '\n') && (c != EOF));
			if(c == EOF) {
				break;
			}
		}
		break;
	case PROCSTAT26STYLE:
		// First feww rows of the /proc/stat output in 2.6 style
		// cpu  31483 0 87972 461075 9775 1319 200
		// cpu0 31483 0 87972 461075 9775 1319 200
		// intr 6473496 5918277 10963 0 0 0 0 0 7 1 2 0 365055 154381 0 24810 0 0 
		//...
		fscanf(cpuStat, "cpu %*s %*s %*s %*s %*s %*s %*s\n");
		for(i = 0; i < processor_count; i++) {
			numScans = fscanf(cpuStat, "%s", tmpBuf);
			if((numScans == 1) && (strlen(tmpBuf) >= 3) && (strncmp(tmpBuf, "cpu", 3) == 0)) {
				fscanf(cpuStat, " %lld %lld %lld %lld %lld %lld %lld\n", 
									&raw_cpu_data[i][CPU_USER_UTILIZATION][snapshot],
									&jiffiesCpuNiceUtilization,
									&raw_cpu_data[i][CPU_PRIVILEGED_UTILIZATION][snapshot],
									&jiffiesCpuidle, 
									&jiffiesCpuiowait, 
									&jiffiesCpuirq, 
									&jiffiesCpusoftirq);
				raw_cpu_data[i][CPU_USER_UTILIZATION][snapshot] += jiffiesCpuNiceUtilization;
				raw_cpu_data[i][CPU_PRIVILEGED_UTILIZATION][snapshot] += jiffiesCpuirq + jiffiesCpusoftirq;
				raw_cpu_data[i][CPU_TOTAL_UTILIZATION][snapshot] = raw_cpu_data[i][CPU_USER_UTILIZATION][snapshot] + raw_cpu_data[i][CPU_PRIVILEGED_UTILIZATION][snapshot];
			}
		}
		for(;;) {
			numScans = fscanf(cpuStat, "%s", tmpBuf);
			if((numScans == 1) && (strlen(tmpBuf) == 4) && (strncmp(tmpBuf, "intr", 4) == 0)) {
				fscanf(cpuStat, " %lld ", &raw_cpu_data[0][CPU_IRQ][snapshot]);
				break;
			}		
			do {
				c = getc(cpuStat);
			} while ((c != '\n') && (c != EOF));
			if(c == EOF) {
				break;
			}
		}
		break;
	default:
		// should never be here
		cerr << "Can not get CPU performance data" << endl;
	}
	fclose(cpuStat);
}

void Performance::Get_TCP_Counters(int snapshot) 
{
	FILE *fp;
	__int64 insegs, outsegs, retranssegs;
	struct tcp_data_type raw_tcp;
	
	if (kstatfd > 0 && ioctl(kstatfd, IM_IOC_GETTCPDATA, &raw_tcp) >= 0) {
		raw_tcp_data[TCP_SEGMENTS_RESENT][snapshot] = raw_tcp.retranssegs;
		return;
	}
	
	// DF: a kind of stupid workable solution. :)
	fp = fopen("/proc/net/snmp", "r");
	fscanf(fp, "Ip: %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s\n");
	fscanf(fp, "Ip: %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s\n");
	fscanf(fp, "Icmp: %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s\n");
	fscanf(fp, "Icmp: %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s\n");
	fscanf(fp, "Tcp: %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s\n");
	fscanf(fp, "Tcp: %*s %*s %*s %*s %*s %*s %*s %*s %*s %lld %lld %lld %*s %*s\n", &insegs, &outsegs, &retranssegs);
	raw_tcp_data[TCP_SEGMENTS_RESENT][snapshot] = retranssegs;
	fclose(fp);
}

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

			double result;
			if (stat == CPU_IRQ)
			{
				// we have to calculate Interrupts/sec.
				// This is similar to calculating Network packets per second
				// but we are more fortunate here.
				// See the corresponding Notes at the end of this file for a description.
				//
				result = ((double) raw_cpu_data[cpu][stat][LAST_SNAPSHOT] - raw_cpu_data[cpu][stat][FIRST_SNAPSHOT]) * clock_tick / timediff;
				cpu_results->CPU_utilization[cpu][stat] = result;
			}
			else
			{
				// All other CPU statistics.
				result = ((double) raw_cpu_data[cpu][stat][LAST_SNAPSHOT] - raw_cpu_data[cpu][stat][FIRST_SNAPSHOT]) / timediff;

				if (result < 0.0) 
				{
					result = 0.0;
					//
					// CPU Utilization figures are outside valid range far too often.
					// Ok, not in every cycle but frequent still.
					// So, it is better to comment it out rather than have the message
					// pop up on the screen at regular intervals.
					//
					// cout << "***** Error : CPU utilization outside valid timerange 0% - 100% *****" << endl;
				}
				if  (result > 1.0)
				{
					result = 1.0;
				}

				cpu_results->CPU_utilization[cpu][stat] = (result * 100);
			}

			#if PERFORMANCE_DETAILS || _DETAILS
				cout << "CPU " << cpu << " recorded stat " << stat << " = " 
					<< cpu_results->CPU_utilization[cpu][stat] << endl;
			#endif
		}
	}
}



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
		double result;
		result = ((double) raw_tcp_data[stat][LAST_SNAPSHOT] - 
			raw_tcp_data[stat][FIRST_SNAPSHOT]) / timediff;
		result *= clock_tick;		// note that timediff is in CLK_TCKs and not seconds
		net_results->tcp_stats[stat] = result;

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
//
// Updated: This file is same from 2.4 to 2.6 kernel. so should works at most of the time.
#define NET_IF_TO_IGNORE "ians"

void Performance::Get_NI_Counters(int snapshot) {
	int c, scanCount, packetIn, packetOut;
	char ifname[32];
	FILE *netInfo;
	struct ni_data_type raw_net;
	
	if (kstatfd > 0 && ioctl(kstatfd, IM_IOC_GETNIDATA, &raw_net) >= 0) {
		for (network_interfaces = 0; network_interfaces < MAX_NUM_INTERFACES; network_interfaces++) {
			raw_ni_data[network_interfaces][NI_IN_ERRORS][snapshot] = raw_net.in_err[network_interfaces];
			raw_ni_data[network_interfaces][NI_OUT_ERRORS][snapshot] = raw_net.out_err[network_interfaces];
			raw_ni_data[network_interfaces][NI_PACKETS][snapshot] = raw_net.nr_pkt[network_interfaces];
		}
		return;
	}

	netInfo = fopen("/proc/net/dev", "r");
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
		// grab the interface names (if there are leading blanks,
		// then they are removed using the Strip() function)
		scanCount = fscanf(netInfo, "%[^:]: %*d %d %lld %*d %*d %*d %*d %*d %*d %d %lld",ifname,
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
		Strip(ifname);
		if(strstr(NET_IF_TO_IGNORE,ifname) != NULL) {
			#ifdef _DEBUG
				cout << "Ignoring network interface: " << ifname << endl;
			#endif
			// We are supposed to ignore this interface so...
			network_interfaces--;
		}
		else {
			assert(scanCount == 5);
			raw_ni_data[network_interfaces][NI_PACKETS][snapshot] =
				packetIn + packetOut;
		}
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
			//result = ((double) raw_ni_data[net][stat][LAST_SNAPSHOT] - 
			//	raw_ni_data[net][stat][FIRST_SNAPSHOT]) * 1000000000.0 / 
			//	((double) time_counter[LAST_SNAPSHOT] - time_counter[FIRST_SNAPSHOT]);
			
			// DF: I think the time_counter do not use nanoseconds but clock_tick
			result = ((double) raw_ni_data[net][stat][LAST_SNAPSHOT] - 
				raw_ni_data[net][stat][FIRST_SNAPSHOT]) * clock_tick / timediff;

			net_results->ni_stats[net][stat] = result;

			#if PERFORMANCE_DETAILS || _DETAILS
				cout << "   Network interface " << net << " recorded stat " << stat << " = " 
					<< net_results->ni_stats[net][stat] << endl;
			#endif
		}
	}
}

#endif /* LINUX */
