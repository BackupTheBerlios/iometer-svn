/* ######################################################################### */
/* ##                                                                     ## */
/* ##  Dynamo / IOManagerLinux.cpp                                        ## */
/* ##                                                                     ## */
/* ## ------------------------------------------------------------------- ## */
/* ##                                                                     ## */
/* ##  Job .......: This file contains the Linux related methods of       ## */
/* ##               Dynamo's main class (IOManager.cpp).                  ## */
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
/* ##  Changes ...: 2003-07-13 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Moved to the use of the IOMTR_[OSFAMILY|OS|CPU]_*   ## */
/* ##                 global defines.                                     ## */
/* ##               - Integrated the License Statement into this header.  ## */
/* ##               2003-03-05 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Removed LINUX_DEBUG, because it is redundant.       ## */
/* ##                 We can use the generic _DEBUG therefor.             ## */
/* ##               2003-02-26 (joe@eiler.net)                            ## */
/* ##               - replaces EXCLUDE_FILESYS define with a string       ## */
/* ##                 so filesystem types are no longer hard coded.       ## */
/* ##               2003-02-15 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Added new header holding the changelog.             ## */
/* ##               - Modified EXCLUDE_FILESYS to support NFS devices     ## */
/* ##                 per default (was excluded by default).              ## */
/* ##                                                                     ## */
/* ######################################################################### */
#if defined(IOMTR_OS_LINUX)


#include "IOManager.h"
#include "IOTargetDisk.h"
#include <dirent.h>
#include <ctype.h>
#include <mntent.h>
#include <sys/wait.h>
#include <sys/swap.h>

static char *mnttab;

#define USED_DEVS_MAX_SIZE (4 * 1024)

/**********************************************************************
 * Forward Declarations
 **********************************************************************/
static int compareRawDiskNames(const void *a, const void *b);

//
// Checking for all accessible drives.  Storing them, and returning the number
// of disks found.  Drives are reported in order so that Iometer does not
// need to sort them.
//
int Manager::Report_Disks( Target_Spec* disk_spec )
{
	TargetDisk	d;
	int		count = 0;
	char usedDevs[USED_DEVS_MAX_SIZE + 1];
	usedDevs[0] = '\0';

	cout << "Reporting drive information..." << endl;

	// *********************************************************************************
	// DEVELOPER NOTES
	// ---------------
	//
	// For linux, I return two sets of data:
	// 1) The root directory for all mounted normal (non-exclude_filesys) filesystems.
	//    These are our "logical" disks. They are found by scanning /etc/mtab.
	// 2) The device names for all unmounted block devices. These are our "physical"
	//    disks. They are found by reading /proc/partitions, then skipping all devices
	//    that are in mtab.
	//
	// **********************************************************************************
	
	//
	// First find all virtual disks by inspecting the mount table. Then search for
	// physical disks that aren't mounted.
	//


	if ((mnttab = getenv("MNTTAB")) == NULL)
		mnttab = _PATH_MOUNTED;

	FILE *file;
	if ((file = fopen(mnttab, "r")) == NULL)
	{
		cout << "open (mount tab) file " << mnttab << " failed with error " << errno << endl;
		cout << "Set environment variable MNTTAB to correct pathname" << endl;
		exit(1);
	}
	
	struct mntent *ment;

	int length;
	char disk_name[MAX_NAME];

	while ((ment = getmntent(file)) != NULL)
	{
#ifdef _DEBUG
		cout << "*** File system found: " << ment->mnt_fsname << "\n";
#endif
		if (!strncmp(ment->mnt_fsname, "/dev/", 5)) {
			// This is a real disk. Add it to our list of disks not to use as
			// physical devices.
			if (strlen(usedDevs) + 2 + strlen(ment->mnt_fsname + 5) >=
					USED_DEVS_MAX_SIZE) {
				cerr << "Too many devices for our list! Aborting.\n";
				exit(1);
			}
			strcat(usedDevs, " ");
			strcat(usedDevs, ment->mnt_fsname + 5);
			strcat(usedDevs, " ");
		}

		// see if the current file sys is an excluded file system type for dynamo.
		if (strstr(exclude_filesys, ment->mnt_type) != NULL) {
#ifdef _DEBUG
			cout << "*** File system type \"" << ment->mnt_type << "\" excluded.\n";
#endif
			continue;
		}

		length = MAX_NAME - strlen(ment->mnt_type);

		// take away 4 bytes from length for the decorations (spaces, [, ])
		length -= 4;

		strncpy(disk_name, ment->mnt_dir, length);
		disk_name[length] = 0;

		if ( ! d.Init_Logical( disk_name ) ) {
#ifdef _DEBUG
			cout << "*** " << __FUNCTION__ << ": Init_Logical failed.\n";
#endif
			continue;
		}

		// Drive exists and ready for use.
		d.spec.type = LogicalDiskType;
		memcpy( &disk_spec[count], &d.spec, sizeof( Target_Spec ) );

		disk_spec[count].name[length] = 0;
		// check for this pattern is also in Manager;:Reported_As_Logical()
		// and TargetDisk::Init_Logical().
		strcat(disk_spec[count].name, " [");
		strcat(disk_spec[count].name, ment->mnt_type);
		strcat(disk_spec[count].name, "]");

#ifdef _DEBUG
			cout << "   Found " << disk_spec[count].name << "." << endl << flush;
#endif
		count++;
		if (count >= MAX_TARGETS)
			break;
	}
	fclose(file);

	if (count >= MAX_TARGETS)
		return count;

	// Now reporting physical drives (raw devices). Data from /proc/partitions.
	// Example output from /proc/partitions:
	//////////////////////////////////////////////////////////////////////
	// major minor  #blocks  name
	//
	//    8     0    8887080 sda
	//    8     1    8225248 sda1
	//    8     2     658665 sda2
	//   22     0 1073741823 hdc
	//////////////////////////////////////////////////////////////////////
	// Note: In the above example, "hdc" is a CD-ROM, and "sda1" and "sda2" are
	// two partitions on the disk represented by "sda".
	
	cout << "  Physical drives (raw devices)..." << endl;
	
	file = fopen("/proc/partitions", "r");
	if (file == NULL) {
		cerr << "Open \"/proc/partitions\" failed (errno " << errno <<"). "
			"Cannot locate physical disks.\n";
		exit(1);
	}
	//
	// Pull out the first line. It just describes the columns. The second line
	// is blank, but fscanf will skip that automatically.
	//
	int c;
	do {
		c = getc(file);
	} while ((c != '\n') && (c != EOF));

	char devName[21], paddedDevName[23];
	while ((count < MAX_TARGETS) &&
				 (fscanf(file, "%*d %*d %*d %20s", devName) == 1)) {
		sprintf(paddedDevName, " %s ", devName);
#ifdef _DEBUG
		cout << __FUNCTION__ << ": Found device " << devName << "\n";
#endif
		if (strstr(usedDevs, paddedDevName) == NULL) {
			// Nobody has mounted this device. Try to open it for reading; if we can,
			// then add it to our list of physical devices.
#ifdef _DEBUG
			cout << __FUNCTION__ << ": Device is not mounted.\n";
#endif
			if (d.Init_Physical(devName)) {
				d.spec.type = PhysicalDiskType;
				memcpy(&disk_spec[count], &d.spec, sizeof(Target_Spec));
				++count;
			}
		}
#ifdef _DEBUG
		else {
			cout << __FUNCTION__ << ": Device is mounted. Ignoring.\n";
		}
#endif

		//
		// Now we cut to the end of the line to get ready to start the next line.
		//
		do {
			c = fgetc(file);
		} while ((c != '\n') && (c != EOF));
	}
	fclose(file);
	qsort(disk_spec, count, sizeof(Target_Spec), compareRawDiskNames);
	return(count);
}

//
// This function compares two disk names. It just uses strcmp. I'm lazy.
//
static int compareRawDiskNames(const void *a, const void *b) {
	const Target_Spec *at = (const Target_Spec *)a;
	const Target_Spec *bt = (const Target_Spec *)b;
	return(strcmp(at->name, bt->name));
}



#endif // IOMTR_OS_LINUX
