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
// IOTargetDisk.h: Interface for an implementation of the Target class
// for disks.
//
//////////////////////////////////////////////////////////////////////

#ifndef TARGET_DISK_DEFINED
	#define TARGET_DISK_DEFINED

#include "IOCommon.h"
#include "IOTime.h"
#include "IOTarget.h"
#include "IOTest.h"
#include "IOCQAIO.h"


#ifdef UNIX
#ifndef _LP64
#define _LP64 /* to get at the 64 bit max long. */
#define _LP64_DEFINED
#endif /* _LP64 */
#define MAX_DISK_SIZE	LONG_MAX
#ifdef _LP64_DEFINED
#undef _LP64
#endif /* _LP64_DEFINED */
#else /* WIN_NT default */
#define MAX_DISK_SIZE	_I64_MAX
#endif /* UNIX */
#define MAX_PARTITIONS	26

// File names used to access a given drive.
#define LOGICAL_DISK	":\\"
#define PHYSICAL_DISK	"\\\\.\\PHYSICALDRIVE"
#define TEST_FILE		"iobw.tst"

#ifdef UNIX
#define ERROR_DISK_FULL					ENOSPC
#define SECTOR_SIZE						512

#ifdef SOLARIS
#define RAW_DEVICE_DIR					"/dev/rdsk"
#elif defined(LINUX)
#define RAW_DEVICE_DIR					"/dev"
#endif

#endif


//
// Logical or physical disk drives.
//
class TargetDisk : public Target
{
public:
	
	TargetDisk();
	~TargetDisk() { };

	BOOL		Initialize( Target_Spec *target_info, CQ *cq );

#if defined (_WIN32) || defined (_WIN64)
	// Initialize logical disks (e.g. C:, D:, E:, etc.).
	BOOL		Init_Logical( char drive );
	// Initialize physical (system) disks (e.g. physicaldisk0, physicaldisk1, etc.).
	BOOL		Init_Physical( int drive );
#else // UNIX
	BOOL		Init_Logical( char *drive );
	BOOL		Init_Physical( char *drive );
#endif

	void		Set_Size( int maximum_size = 0 );
	void		Set_Starting_Sector( int starting_sector = 0 );

	BOOL		Prepare( void *buffer, DWORDLONG *prepare_offset, DWORD bytes, volatile TestState *test_state );

	BOOL		Open( volatile TestState *test_state, int open_flag = 0 );
	BOOL		Close( volatile TestState *test_state );

	ReturnVal	Read( LPVOID buffer, Transaction *trans );
	ReturnVal	Write( LPVOID buffer, Transaction *trans );

	// Set the offset to the next position on the disk (random or sequential).
	void		Seek( BOOL random, DWORD request_size, DWORD user_alignment, DWORDLONG user_align_mask );

	char		file_name[MAX_NAME];

protected:

	BOOL		Set_Sizes( BOOL open_disk = TRUE );	// Get physical drive dimensions.
	void		Set_Sector_Info();

private:

	CQAIO		*io_cq;

	HANDLE		disk_file;
#ifdef UNIX
	struct File		file_handle;
#endif
	DWORDLONG	size;					// Size of the disk in bytes.
	DWORD		alignment;				// Alignment I/Os on this address.
	DWORDLONG	sector_align_mask;		// Bit mask to align requests to sector sizes.
										// This only works for sectors a power of 2.
										// Set to NOT_POWER_OF_TWO otherwise.
	DWORDLONG	starting_position;		// First bytes where transfers occur.
	DWORDLONG	ending_position;		// Last byte where transfers can occur.
	DWORDLONG	offset;
	DWORD		bytes_transferred;		// Number of bytes successfully transferred to the disk.
#ifdef SOLARIS
	BOOL		Look_For_Partitions();	// private member function to look for partitions on disk.
	DWORDLONG	Get_Partition_Size(char *, int);	// private member function to get the partition size.
	DWORDLONG	Get_Slice_Size(char *, int);	// private member function to get the slice size.
#endif
};



#endif
