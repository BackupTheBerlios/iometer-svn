/* ######################################################################### */
/* ##                                                                     ## */
/* ##  Dynamo / iomtr_kstat/ test.c                                       ## */
/* ##                                                                     ## */
/* ## ------------------------------------------------------------------- ## */
/* ##                                                                     ## */
/* ##  Job .......: a simple test program to test the iomtr_kstat         ## */
/* ##               interface.                                            ## */
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
/* ##  Changes ...:                                                       ## */
/* ##               2004-02-04 (mingz(at)ele.uri.edu)                     ## */
/* ##               - initital code. test various ioctl interface         ## */
/* ##                 and proc/stat style and etc                         ## */
/* ##                                                                     ## */
/* ######################################################################### */

#include <stdio.h>
#include <stdlib.h>
#include <linux/unistd.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#include "iomtr_kstat.h"

#if defined(_IO) && !defined(BLKSSZGET)
#define BLKSSZGET  _IO(0x12,104)
#endif

#ifndef u64
typedef unsigned long long u64;
#endif
#if defined(_IO) && !defined(BLKGETSIZE64)
#define BLKGETSIZE64 _IOR(0x12,114,sizeof(u64))
#endif

int DetectLinuxVersion(void)
{
	int fd, v;
	char buf[64], *p;

	if ((fd = open("/proc/version", O_RDONLY)) < 0) {
		printf("Fail to open version file\n");
		return -1;
	}
	memset(buf, 0, 64);
	read(fd, buf, 63);
	close(fd);

	// a really simple workaround
	p = strstr(buf, "Linux version");
	v = p[16] - '0';
	// assume 2.5 is same as 2.6
	if (v == 5) v = 6 ;
	return v;
}

int InitIoctlInterface(void)
{
        return open("/dev/iomtr_kstat", O_RDONLY);
}

int DetectProcStatStyle()
{
	FILE *fp;
	unsigned long long x1, x2, x3, x4, x5, x6, x7;
	int res;
	
	fp = fopen("/proc/stat", "r");
	res = fscanf(fp, "cpu %lld %lld %lld %lld %lld %lld %lld\n", &x1, &x2, &x3, &x4, &x5, &x6, &x7);
	printf("%d\n", res);
	return res;
}

int CleanupIoctlInterface(int fd)
{
	close(fd);
}

void gettcpcounter()
{
	FILE *fp;
	long long insegs, outsegs, retranssegs;
	
	fp = fopen("/proc/net/snmp", "r");
	fscanf(fp, "Ip: %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s\n");
	fscanf(fp, "Ip: %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s\n");
	fscanf(fp, "Icmp: %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s\n");
	fscanf(fp, "Icmp: %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s\n");
	fscanf(fp, "Tcp: %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s\n");
	fscanf(fp, "Tcp: %*s %*s %*s %*s %*s %*s %*s %*s %*s %lld %lld %lld %*s %*s\n", &insegs, &outsegs, &retranssegs);
	printf("insegs %lld, outsegs %lld, retranssegs %lld\n", insegs, outsegs, retranssegs);
	fclose(fp);
}

int main(void)
{
	int fd, res, t, nr_cpu;
	unsigned long m;
	struct cpu_data_type raw_cpu;
	struct ni_data_type raw_net;
	struct tcp_data_type raw_tcp;
	long long sz;
	unsigned long long jf;

	printf("clock_tick = %ld\n", sysconf(_SC_CLK_TCK));
	
	gettcpcounter();
	
	DetectProcStatStyle();
		
	DetectLinuxVersion();

	if ((fd = InitIoctlInterface()) < 0) {
		printf("device file open fail!\n");
		exit(1);
	}
	
	if ((res = ioctl(fd, IM_IOC_GETCPUNUM, &nr_cpu)) >= 0 ) {
		printf("# CPU: %d\n", nr_cpu);
	}
	if ((res = ioctl(fd, IM_IOC_GETCPUKHZ, &m)) >= 0 ) {
		printf("CPU KHZ: %lu\n", m);
	}
	if ((res = ioctl(fd, IM_IOC_GETCURJIFFIES, &jf)) >= 0 ) {
		printf("JIFFIE: %llu\n", jf);
	}
	if ((res = ioctl(fd, IM_IOC_GETCPUDATA, &raw_cpu)) >= 0 ) {
		for (t = 0; t < nr_cpu; t++) {
			printf("cpu %d: system time %llu, user time %llu\n", t, raw_cpu.system_time[t], raw_cpu.user_time[t]);
		}
		printf("# intr %llu\n", raw_cpu.intr);
	}
	if ((res = ioctl(fd, IM_IOC_GETNIDATA, &raw_net)) >= 0 ) {
		for (t = 0; t < MAX_NUM_INTERFACES; t++) {
			if (raw_net.nr_pkt[t])
				printf("net %d: nr_pkt %llu, in_err %llu, out_err %llu\n", t, raw_net.nr_pkt[t], 
					raw_net.in_err[t], raw_net.out_err[t]);
		}
	}
	if ((res = ioctl(fd, IM_IOC_GETTCPDATA, &raw_tcp)) >= 0 ) {
		printf("tcp:%lld, %lld, %lld\n", raw_tcp.insegs, raw_tcp.outsegs, raw_tcp.retranssegs);
	}
	else {
		printf("fail\n");
	}

	CleanupIoctlInterface(fd);
	fd = open("/dev/hda", O_RDONLY);
	if (fd < 0)
		printf("fail to open\n");
	else {
		// get a sector size
		res = ioctl(fd, BLKSSZGET, &t);
		printf("sz:%d\n", t);
		// get a total size
		res = ioctl(fd, BLKGETSIZE64, &sz);
		printf("sz:%llu\n", sz);
	}
	
	return 0;
}
