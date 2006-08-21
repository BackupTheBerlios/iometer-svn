/* ######################################################################### */
/* ##                                                                     ## */
/* ##  Dynamo / IOManagerWin.cpp                                          ## */
/* ##                                                                     ## */
/* ## ------------------------------------------------------------------- ## */
/* ##                                                                     ## */
/* ##  Job .......: OS specific Manager functions for Windows             ## */
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
/* ##  Changes ...: 2004-04-15 (lamontcranston41@yahoo.com)               ## */
/* ##               - Moved Report_Disks() from IOManager.cpp to here.    ## */
/* ##               - Initial drop.                                       ## */
/* ##                                                                     ## */
/* ######################################################################### */

#include "IOCommon.h"
#include "IOManager.h"
#include "IOTargetDisk.h"

//
// Checking for all accessible drives.  Storing them, and returning the number
// of disks found.  Drives are reported in order so that Iometer does not
// need to sort them.
//
int Manager::Report_Disks(Target_Spec * disk_spec)
{
	DWORD dummy_word;
	TargetDisk d;
	char drive_letter;
	int drive_number = 0;
	int count = 0;

	cout << "Reporting drive information..." << endl;

	// Reporting logical drives first.
	for (drive_letter = 'A'; drive_letter <= 'Z'; drive_letter++) {
		// Checking to see if drive exists.
		if (!d.Init_Logical(drive_letter))
			continue;

		// Drive exists.  Getting its sector size, label, and volume name.
		memcpy(&disk_spec[count], &d.spec, sizeof(Target_Spec));
		strcat(disk_spec[count].name, "\\");
		if (!GetVolumeInformation(disk_spec[count].name, disk_spec[count].name + 2,
					  MAX_NAME - 1, NULL, &dummy_word, &dummy_word, NULL, 0)) {
			cout << "   Unable to retrieve volume information for " << d.spec.name << "." << endl;
			disk_spec[count].name[2] = '\0';
		}
#ifdef _DEBUG
		cout << "   Found " << disk_spec[count].name << "." << endl;
#endif
		count++;
	}

	// Reporting physical drives.
	while (count < MAX_TARGETS) {
		// See if the physical drive exists.
		sprintf(d.spec.name, "%s%i", PHYSICAL_DISK, drive_number);
		strcpy(d.file_name, d.spec.name);
		d.spec.type = PhysicalDiskType;

		// Try to open the drive, if it exists close it and initialize it.  If
		// it doesn't exist, no more physical drives will be found.  This is
		// temporary code until the TargetDisk class is split up.
		if (!d.Open(NULL))
			break;
		d.Close(NULL);

		// Drive exists, see if it is available for accessing.
		if (!d.Init_Physical(drive_number)) {
			drive_number++;
			continue;
		}

		memcpy(&disk_spec[count], &d.spec, sizeof(Target_Spec));
		strcpy(disk_spec[count].name, PHYSICAL_DRIVE_PREFIX);
		_itoa(drive_number, disk_spec[count].name + strlen(disk_spec[count].name), 10);
		drive_number++;

#ifdef _DEBUG
		cout << "   Found " << disk_spec[count].name << "." << endl;
#endif
		count++;
	}
	cout << "   done." << endl << flush;
	return count;
}
