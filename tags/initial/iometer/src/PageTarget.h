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
// PageTarget.h: Common defines for both PageDisk and PageNetwork.
//
//////////////////////////////////////////////////////////////////////

#ifndef PAGETARGET_DEFINED
	#define PAGETARGET_DEFINED

// indexes for selection updates to make recursive updates easier to do
// for the manager list up through the workers.
#define TARGETS				0
#define DISK_SIZE			1
#define DISK_START			2
#define QUEUE_DEPTH			3
#define NET_ADDRESS			4
#define CONNECTION_RATE		5
#define TRANS_PER_CONN		6

// Info for Targets image list
#define TARGET_ICON_SIZE					16
#define TARGET_ICON_EXPAND					10 // number of icons to add to ImageList when expanding
#define TARGET_ICON_BGCOLOR					0x00ffffff

// Icons for Targets pane (position of icon in bitmap)
#define TARGET_ICON_LOGDISK_PREPARED		0
#define TARGET_ICON_LOGDISK_UNPREPARED		1
#define TARGET_ICON_PHYSDISK				2
#define TARGET_ICON_MANAGER					3
#define TARGET_ICON_NETWORK					4

// State images for Targets pane (position of icon in bitmap -- we use a single bitmap for both)
#define TARGET_STATE_CHECKED				5
#define TARGET_STATE_UNCHECKED				6
#define TARGET_STATE_UNKNOWN				7

// Inverse of INDEXTOSTATEIMAGEMASK(). (Windows doesn't define this, but it should...)
#define STATEIMAGEMASKTOINDEX(i) ((i) >> 12)

enum TargetSelType
{
	TargetUnChecked = TARGET_STATE_UNCHECKED,
	TargetChecked = TARGET_STATE_CHECKED,
	TargetGrayed = TARGET_STATE_UNKNOWN,
};

#endif
