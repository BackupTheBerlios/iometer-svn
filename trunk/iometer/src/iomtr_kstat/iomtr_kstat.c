/* ######################################################################### */
/* ##                                                                     ## */
/* ##  Dynamo / iomtr_kstat.c                                             ## */
/* ##                                                                     ## */
/* ## ------------------------------------------------------------------- ## */
/* ##                                                                     ## */
/* ##  Job .......: A simple module to export kernel information for      ## */
/* ##               dynamo to use a low-overhead way to get kernel        ## */
/* ##               status.                                               ## */
/* ##               it creates a virtual character device and exchange    ## */
/* ##               information with user space via ioctl interface.      ## */
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
/* ##  Remarks ...: Some 2.4.x kernels are highly hacked by vendors and   ## */
/* ##               has a lot of back port stuff from 2.6.x kernel. So    ## */
/* ##               can not use the kernel version code to detect these   ## */
/* ##               difference any more.                                  ## */
/* ##                                                                     ## */
/* ## ------------------------------------------------------------------- ## */
/* ##                                                                     ## */
/* ##  Changes ...: 2004-02-04 (mingz(at)ele.uri.edu)                     ## */
/* ##               - Initital code. Export all kernel status value       ## */
/* ##                 dynamo need                                         ## */
/* ##                                                                     ## */
/* ######################################################################### */
#ifndef __KERNEL__
#define __KERNEL__
#endif

#ifndef MODULE
#define MODULE
#endif

#include <linux/config.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/proc_fs.h>
#include <linux/fcntl.h>
#include <asm/uaccess.h>
#include <asm/system.h>
#include <linux/string.h>
#include <asm/irq.h>
#include <linux/kernel_stat.h>
#include <linux/netdevice.h>
#include <net/tcp.h>

#define DEBUG

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0))
#define K26
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,0)) && (LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0))
#define K24
#endif

#include "iomtr_kstat.h"

int imkstat_major = 0;

int imkstat_ioctl(struct inode *inode, struct file *filp,
                 unsigned int cmd, unsigned long arg)
{
	int err = 0, tmp, i;
	unsigned long khz;
	unsigned long sum = 0;
	unsigned long long jf;
	struct cpu_data_type c;
	struct ni_data_type n;
	struct tcp_data_type t;
	struct net_device *dev;
	struct net_device_stats *stats;
#ifdef K26
	unsigned long seq;
#endif

	if (_IOC_TYPE(cmd) != IMKSTAT_IOC_MAGIC) {
		printk("iomtr_kstat: wrong magic\n");
		return -ENOTTY;
	}
	if (_IOC_NR(cmd) > IM_IOC_MAXNR) {
		printk("iomtr_kstat: wrong number\n");
		return -ENOTTY;
	}
	if (_IOC_DIR(cmd) & _IOC_READ)
		err = !access_ok(VERIFY_WRITE, (void *)arg, _IOC_SIZE(cmd));
	else if (_IOC_DIR(cmd) & _IOC_WRITE)
		err = !access_ok(VERIFY_READ, (void *)arg, _IOC_SIZE(cmd));
	if (err) {
		printk("iomtr_kstat: invalid access\n");
		return -EFAULT;
	}

	switch (cmd) {
	case IM_IOC_GETCPUNUM:
		tmp = 0;
#ifdef K24
		tmp = smp_num_cpus;
#endif
#ifdef K26
		for (i = 0; i < NR_CPUS; i++) {
			if (cpu_online(i))
				tmp++;
		}
#endif
#ifdef DEBUG	
		printk("# of CPU: %d\n", tmp);
#endif
		if (tmp == 0)
			err = -EFAULT;
		else
			err = __put_user(tmp, (int *)arg);
		break;

	case IM_IOC_GETCPUKHZ:
#if defined(CONFIG_ARM)
#endif
#if defined(CONFIG_X86)
		khz = cpu_khz;
#endif
#ifdef DEBUG		
		printk("CPU KHZ: %lu\n", khz);
#endif
		err = __put_user(khz, (unsigned long *)arg);
		break;
	case IM_IOC_GETCURJIFFIES:
		/* 2.4 only have 32 bit value and 2.6 has a 64 bit jiffies, iometer need 64 bit value */
#ifdef K24
		jf = jiffies;
#endif
#ifdef K26
		/* Atomically read jiffies */ 
		do {
			seq = read_seqbegin(&xtime_lock);
			jf = get_jiffies_64();
		} while (read_seqretry(&xtime_lock, seq));
#endif
#ifdef DEBUG
		printk("current jiffies: %llu\n", jf);
#endif
		err = __put_user(jf, (unsigned long long*)arg);
		break;
	case IM_IOC_GETCPUDATA:
		memset(&c, 0, sizeof(struct cpu_data_type));
#ifdef K24
		for (i = 0 ; i < smp_num_cpus && i < MAX_CPUS; i++) {
			int cpu = cpu_logical_map(i), j;
			c.user_time[i] = kstat.per_cpu_user[cpu] + kstat.per_cpu_nice[cpu];
			c.system_time[i] = kstat.per_cpu_system[cpu];
			for (j = 0 ; j < NR_IRQS ; j++)
				sum += kstat.irqs[cpu][j];
		}
		c.intr = sum;
#ifdef DEBUG
		for (i = 0; i < smp_num_cpus && i < MAX_CPUS; i++) {
			printk("cpu %d: system time %llu, user time %llu\n", i, c.system_time[i], c.user_time[i]);
		}
		printk("intr %llu\n", c.intr);
#endif
#endif	/* K24 */
#ifdef K26
		for (i = 0; i < NR_CPUS && i < MAX_CPUS; i++) {
			int j;
			if (!cpu_online(i)) 
				continue;
			c.user_time[i] = kstat_cpu(i).cpustat.user + kstat_cpu(i).cpustat.nice;
			c.system_time[i] = kstat_cpu(i).cpustat.system;
			for (j = 0 ; j < NR_IRQS ; j++)
				sum += kstat_cpu(i).irqs[j];
		}
		c.intr = sum;
#ifdef DEBUG
		for (i = 0; i < NR_CPUS && i < MAX_CPUS; i++) {
			if (!cpu_online(i)) 
				continue;
			printk("cpu %d: system time %llu, user time %llu\n", i, c.system_time[i], c.user_time[i]);
		}
		printk("intr %llu\n", c.intr);
#endif
#endif
		err = copy_to_user((struct cpu_data_type *)arg, &c, sizeof(struct cpu_data_type));
		break;
	case IM_IOC_GETNIDATA:
		memset(&n, 0, sizeof(struct ni_data_type));
		read_lock(&dev_base_lock);
		for (i = 0, dev = dev_base; (dev != NULL) && (i < MAX_NUM_INTERFACES); dev = dev->next, i++) {
			if (dev->get_stats) {
				stats = dev->get_stats (dev);
			}
			else
				stats = NULL;
			if (stats) {
				n.in_err[i] = stats->rx_errors;
				n.out_err[i] = stats->tx_errors;
				n.nr_pkt[i] = stats->tx_packets + stats->rx_packets;
			}
		}
		read_unlock(&dev_base_lock);
#ifdef DEBUG
		for (i = 0; i < MAX_NUM_INTERFACES; i++) {
			if (n.nr_pkt[i])
				printk("net %d: nr_pkt %llu, in_err %llu, out_err %llu\n", i, n.nr_pkt[i], 
					n.in_err[i], n.out_err[i]);
		}
#endif
		err = copy_to_user((struct ni_data_type *)arg, &n, sizeof(struct ni_data_type));
		break;
	case IM_IOC_GETTCPDATA:
		memset(&t, 0, sizeof(struct tcp_data_type));
#ifdef K24
		for (i = 0; i < smp_num_cpus; i++) {
			t.insegs += tcp_statistics[ 2 * cpu_logical_map(i)].TcpInSegs;
			t.insegs += tcp_statistics[ 2 * cpu_logical_map(i) + 1].TcpInSegs;
			t.outsegs += tcp_statistics[ 2 * cpu_logical_map(i)].TcpOutSegs;
			t.outsegs += tcp_statistics[ 2 * cpu_logical_map(i) + 1].TcpOutSegs;
			t.retranssegs += tcp_statistics[ 2 * cpu_logical_map(i)].TcpRetransSegs;
			t.retranssegs += tcp_statistics[ 2 * cpu_logical_map(i) + 1].TcpRetransSegs;
		}
#endif
#ifdef K26
		for (i = 0; i < NR_CPUS; i++) {
			if (!cpu_possible(i))
				continue;
			t.insegs += ((struct tcp_mib *) per_cpu_ptr(tcp_statistics[0], i))->TcpInSegs;
			t.insegs += ((struct tcp_mib *) per_cpu_ptr(tcp_statistics[1], i))->TcpInSegs;
			t.outsegs += ((struct tcp_mib *) per_cpu_ptr(tcp_statistics[0], i))->TcpOutSegs;
			t.outsegs += ((struct tcp_mib *) per_cpu_ptr(tcp_statistics[1], i))->TcpOutSegs;
			t.retranssegs += ((struct tcp_mib *) per_cpu_ptr(tcp_statistics[0], i))->TcpRetransSegs;
			t.retranssegs += ((struct tcp_mib *) per_cpu_ptr(tcp_statistics[1], i))->TcpRetransSegs;
		}
#endif
#ifdef DEBUG
		printk("insegs=%lld,outsegs=%lld,retranssegs=%lld\n", t.insegs, t.outsegs, t.retranssegs);
#endif
		err = copy_to_user((struct tcp_data_type *)arg, &t, sizeof(struct tcp_data_type));
		break;
	default:
		err = -EFAULT;
	}
	return err;
}

int imkstat_open(struct inode *inode, struct file *filp)
{
#ifdef K24
	MOD_INC_USE_COUNT;
#endif
	return 0;
}

int imkstat_release(struct inode *inode, struct file *filp)
{
#ifdef K24
	MOD_DEC_USE_COUNT;
#endif
	return 0;
}

struct file_operations imkstat_fops = {
	llseek:     NULL,
	read:       NULL,
	write:      NULL,
	ioctl:      imkstat_ioctl,
	open:       imkstat_open,
	release:    imkstat_release,
};

int imkstat_init(void)
{
	int result;

#if !defined(K24) && !defined(K26)
	printk("iomtr_kstat: can not decide what kernel you use.\n");
	return -EINVAL;
#endif

	result = register_chrdev(imkstat_major, "iomtr_kstat", &imkstat_fops);
	if (result < 0) {
		printk(KERN_WARNING "iomtr_kstat: can't get major %d\n", imkstat_major);
		return result;
	}
	if (imkstat_major == 0) 
		imkstat_major = result;
	return 0;
}

void imkstat_clean(void)
{
	unregister_chrdev(imkstat_major, "iomtr_kstat");
	return;
}

module_init(imkstat_init);
module_exit(imkstat_clean);

MODULE_DESCRIPTION("IoMeter Kernel Status Data Gathering Module");
MODULE_AUTHOR("Ming Zhang");
MODULE_LICENSE("GPL");

