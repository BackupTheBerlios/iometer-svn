/* ######################################################################### */
/* ##                                                                     ## */
/* ##  Dynamo / iomtr_kstat.h                                             ## */
/* ##                                                                     ## */
/* ## ------------------------------------------------------------------- ## */
/* ##                                                                     ## */
/* ##  Job .......: Interface for the kernel status module.               ## */
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
/* ##  Changes ...: 2004-02-04 (mingz(at)ele.uri.edu)                     ## */
/* ##               - initital code. defines shared by kernel module and  ## */
/* ##                 dynamo                                              ## */
/* ##                                                                     ## */
/* ######################################################################### */
#ifndef __IM_KSTAT__
#define __IM_KSTAT__

#include <linux/ioctl.h> 

#define IMKSTAT_IOC_MAGIC	0xAE

// copied from IOCommon.h, should merge to one later
#ifndef MAX_CPUS
#define MAX_CPUS	32
#endif
#ifndef MAX_NUM_INTERFACES
#define MAX_NUM_INTERFACES	32
#endif

struct cpu_data_type {
	unsigned long long user_time[MAX_CPUS];
	unsigned long long system_time[MAX_CPUS];
	unsigned long long intr;
};

struct ni_data_type {
	unsigned long long in_err[MAX_NUM_INTERFACES];
	unsigned long long out_err[MAX_NUM_INTERFACES];
	unsigned long long nr_pkt[MAX_NUM_INTERFACES];
};

struct tcp_data_type {
	long long insegs;
	long long outsegs;
	long long retranssegs;
};

#define IM_IOC_GETCPUNUM	_IOR(IMKSTAT_IOC_MAGIC, 1, int)
#define IM_IOC_GETCPUKHZ	_IOR(IMKSTAT_IOC_MAGIC, 2, unsigned long)
#define IM_IOC_GETCURJIFFIES	_IOR(IMKSTAT_IOC_MAGIC, 3, unsigned long long)
#define IM_IOC_GETCPUDATA	_IOR(IMKSTAT_IOC_MAGIC, 4, struct cpu_data_type)
#define IM_IOC_GETNIDATA	_IOR(IMKSTAT_IOC_MAGIC, 5, struct ni_data_type)
#define IM_IOC_GETTCPDATA	_IOR(IMKSTAT_IOC_MAGIC, 6, struct tcp_data_type)

#define IM_IOC_MAXNR 7

#endif
