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
// IOTargetDisk.cpp: Implementation of the Target class for disks.
// TargetDisk includes all the code that actually accesses a disk target.
//
//////////////////////////////////////////////////////////////////////

#include "IOTargetDisk.h"
#include "IOAccess.h"

#define _DISK_MSGS 0

#ifdef UNIX
#ifdef WORKAROUND_MOD_BUG
#include <math.h>
#endif // WORKAROUND_MOD_BUG

#ifdef SOLARIS
#if defined(__i386) || defined (_IA64)
// highly specific to Solaris on Intel
#include <sys/dktp/fdisk.h>
#endif // __i386 || _IA64
#include <sys/statvfs.h>
#include <sys/stat.h>
#include <sys/dkio.h>
#include <sys/vtoc.h>
#endif // SOLARIS

#ifdef LINUX
#include <assert.h>
#include <sys/vfs.h>
#endif

#endif // UNIX

/**********************************************************************
 * Forward Declarations
 **********************************************************************/
#ifdef LINUX
static int getSectorSizeOfPhysDisk(const char *devName);
static long long getSizeOfPhysDisk(const char *devName);
static BOOL getDevNums(const char *devName, int *major, int *minor);
static off_t getSectorTableLoc(void);
#endif


//
// Constructor.
//
TargetDisk::TargetDisk()
{
	sector_align_mask = NOT_POWER_OF_TWO;

#ifdef UNIX
  disk_file = (HANDLE)&file_handle;
#endif
}



//
// Initializing the disk based on the specified target information.
//
BOOL TargetDisk::Initialize( Target_Spec *target_info, CQ *cq )
{
	BOOL retval;

	io_cq = (CQAIO*)cq;
	memcpy( &spec, target_info, sizeof( Target_Spec ) );

	// Initializing logical disks.
#if defined (_WIN32) || defined (_WIN64)
	if ( IsType( target_info->type, LogicalDiskType ) )
		retval = Init_Logical( spec.name[0] );
	else if ( IsType( target_info->type, PhysicalDiskType ) )
		retval = Init_Physical( atoi( spec.name + 14 ) );
#else
	if ( IsType( target_info->type, LogicalDiskType ) )
		retval = Init_Logical( spec.name );
	else if ( IsType( target_info->type, PhysicalDiskType ) )
		retval = Init_Physical( spec.name );
#endif
	else
	{
		cout << "*** Invalid disk type in TargetDisk::Initialize()." << endl;
		retval = FALSE;
	}

	if ( retval )
	{
		// Setting size of disk, if it was successfully initialized
		Set_Size( spec.disk_info.maximum_size );

#ifdef _sparc
		if (spec.disk_info.starting_sector)
			Set_Starting_Sector( spec.disk_info.starting_sector);
		else
			if ( IsType( target_info->type, PhysicalDiskType ) )
			{
				// always skip sector 0 for non-i386 || IA64 platform (only for raw disks).
				// We need it to avoid destroying the vtoc table which is
				// not protected on these (sparc) platforms.
				//
				// But this is serious performance implications.
				// Can slow down access quite a bit.
				//
				Set_Starting_Sector( 1 );
			}
#else // _sparc
		Set_Starting_Sector( spec.disk_info.starting_sector );
#endif // _sparc
	}

	return retval;
} 


#if defined (_WIN32) || defined (_WIN64)
//
// Initialize a logical disk drive.  Logical drives are accessed through
// a drive letter and may be local or remote.
//
BOOL TargetDisk::Init_Logical( char drive )
{
	// Setting spec.name of the drive.
	sprintf( spec.name, "%c%s", drive, LOGICAL_DISK );
	sprintf( file_name, "%s%s", spec.name, TEST_FILE );

	spec.type = LogicalDiskType;
	size = 0;
	starting_position = 0;
	offset = 0;
	bytes_transferred = 0;

	// Getting size information about the drive.
	return( Set_Sizes() );
}
#else
// UNIX logical drives are accessed through path names.
BOOL TargetDisk::Init_Logical( char *drive )
{
	// Strip off file system Information about the logical drives.
	char *p;
	// check for this pattern is also in Manager::Report_Disks()
	// and Manager::Reported_As_Logical().
	p = strstr(drive, " [");
	if (p)
	{
		strncpy(spec.name, drive, p - drive);
		spec.name[p - drive] = 0;
	}
	else
		// Setting spec.name of the drive.
		sprintf( spec.name, "%s", drive );

	if (spec.name[strlen(spec.name) - 1] == '/') {
		sprintf(file_name, "%s%s", spec.name, TEST_FILE);
	} else {
		sprintf(file_name, "%s/%s", spec.name, TEST_FILE);
	}

	spec.type = LogicalDiskType;
	size = 0;
	starting_position = 0;
	offset = 0;
	bytes_transferred = 0;

	// Getting size information about the drive.
	return( Set_Sizes() );
}
#endif


#if defined (_WIN32) || defined (_WIN64)
//
// Initialize a physical disk drive.  Physical drives are accessed below
// the file system layer for RAW access.  As a result, data corruption could
// occur.  To prevent this, only drives which contain nothing but free space
// are accessible.
//
BOOL TargetDisk::Init_Physical( int drive )
{
	// Setting the spec.name of the drive.
	sprintf( spec.name, "%s%i", PHYSICAL_DISK, drive );
	strcpy( file_name, spec.name );

	spec.type = PhysicalDiskType;
	size = 0;
	starting_position = 0;
	offset = 0;
	bytes_transferred = 0;

	// Getting information about the size of the drive.
	return( Set_Sizes() );
}
#else
BOOL TargetDisk::Init_Physical( char *drive )
{
	// Setting the spec.name of the drive.
	// Strip off any extra information in the drive name.
	char *p;
	// check for this pattern is also in Manager::Report_Disks()
	// and Manager::Reported_As_Logical().
	p = strstr(drive, " [");
	if (p)
	{
		strncpy(spec.name, drive, p - drive);
		spec.name[p - drive] = 0;
	}
	else
	{
		// Setting spec.name of the drive.
		sprintf( spec.name, "%s", drive );
	}

	sprintf( file_name, "%s/%s", RAW_DEVICE_DIR, spec.name );

	spec.type = PhysicalDiskType;
	size = 0;
	starting_position = 0;
	offset = 0;
	bytes_transferred = 0;

	// Getting information about the size of the drive.
	return( Set_Sizes() );
}
#endif

//
// Setting the maximum amount of disk space to access during testing.
//
void TargetDisk::Set_Size( int maximum_size )
{
	DWORDLONG	new_size;

	// Getting the actual size of the disk.
	Set_Sizes();
	new_size = (DWORDLONG) maximum_size * (DWORDLONG) spec.disk_info.sector_size;

	// If the specified maximum size is not 0 and is smaller than the actual disk,
	// reset the disk size.
	if ( new_size && ( new_size < size ) )
	{
		size = new_size;
		#if _DEBUG
			cout << "Resetting accessible size of disk " << spec.name << " to "
				<< maximum_size << " sectors." << endl
				<< "   " << spec.name << " size = " << size << endl;
		#endif
	}
}



//
// Setting the starting sector of the disk where accesses should begin.
// This needs to be called after setting the size of the disk (call to Set_Size).
//
void TargetDisk::Set_Starting_Sector( int starting_sector )
{
	DWORDLONG	current_size;

	starting_position = (DWORDLONG) starting_sector * (DWORDLONG) spec.disk_info.sector_size;
	#if _DEBUG
		cout << "Moving starting sector of disk " << spec.name << " to " 
			<< starting_position << "." << endl;
	#endif

	// Moving the starting disk sector may have shrunk the accessible size of the disk.
	current_size = size;	// record the currently assigned size to use
	Set_Size();				// getting the actual disk size

	if ( starting_position + current_size > size )
	{
		// Re-adjust the accessible size of the disk to reflect the new starting sector.
		size -= starting_position;
		#if _DEBUG
			cout << "Starting sector size has shrunk the effective size of the disk." << endl;
		#endif
	}
	else
	{
		// The new starting sector did not affect the accessible disk size.
		size = current_size;
	}
	ending_position = (DWORDLONG) starting_position + size;
}



//
// Setting information about the physical size of the drive.  This is
// used in order to guarantee that accesses to the drive are permitted,
// that is align on the sector sizes, and to allow random accesses over
// the entire drive.
//
#if defined (_WIN32) || defined (_WIN64)
BOOL TargetDisk::Set_Sizes( BOOL open_disk )
{
	DWORD		i;
	DWORD		low_size, high_size;
	DWORD		sectors_per_cluster, free_clusters, total_clusters;
	DWORD		sector_size;
	DRIVE_LAYOUT_INFORMATION	disk_layout_info[MAX_PARTITIONS];
	DISK_GEOMETRY				disk_geo_info;
	DWORD						disk_info_size;

	// Logical and physical drives are treated differently.
	if ( open_disk )
	{
		if ( !Open( NULL ) )
			return FALSE;
	}

	if ( IsType( spec.type, LogicalDiskType ) )
	{
		// Getting physical information about the drive.
		if ( !GetDiskFreeSpace( spec.name, &sectors_per_cluster, &sector_size,
			&free_clusters, &total_clusters) )
		{
			cout << "Error getting sector size of drive " << spec.name 
				<< "." << endl;
			if ( open_disk )
				Close( NULL );
			return FALSE;
		}
		spec.disk_info.sector_size = (int) sector_size;

		// Determining if the test file exists or needs to be made.
		low_size = GetFileSize( disk_file, &high_size );
		size = ( ( (DWORDLONG) high_size ) << 32 ) | (DWORDLONG) low_size;
		Set_Sector_Info();
		#if _DEBUG
			cout << "   " << spec.name << " size = " << size << endl;
		#endif
		if ( open_disk )
			Close( NULL );

		// If test file exists, the drive is ready for access.
		if ( size )
		{
			ending_position = size;
			spec.disk_info.ready = TRUE;
			return TRUE;
		}

		DeleteFile( file_name );
		spec.disk_info.ready = FALSE;
		return TRUE;
	}
	else if ( IsType( spec.type, PhysicalDiskType ) )
	{
		// Dealing with a physical drive.
		// Checking for a partition to exist on the physical drive.
		// If one is there, then disallowing access to the drive.
		SetLastError( 0 );
 		DeviceIoControl( disk_file, IOCTL_DISK_GET_DRIVE_LAYOUT, NULL, 0,
			disk_layout_info, sizeof( disk_layout_info ), &disk_info_size, NULL );

		// Checking that drive contains nothing but free space.
		for ( i = 0; i < disk_layout_info[0].PartitionCount; i++ )
		{
			if ( disk_layout_info[0].PartitionEntry[i].PartitionLength.HighPart ||
				disk_layout_info[0].PartitionEntry[i].PartitionLength.LowPart )
			{
				if ( open_disk )
					Close( NULL );
				return FALSE;
			}
		}
		// Getting information on the size of the drive.
		DeviceIoControl( disk_file, IOCTL_DISK_GET_DRIVE_GEOMETRY, NULL, 0,
			&disk_geo_info, sizeof( disk_geo_info ), &disk_info_size, NULL );

		// Calculating the size of the physical drive..
		size = ( ( (DWORDLONG) disk_geo_info.Cylinders.HighPart ) << 32 ) | 
			( (DWORDLONG) disk_geo_info.Cylinders.LowPart );
		size *= (_int64) disk_geo_info.TracksPerCylinder * 
			(_int64) disk_geo_info.SectorsPerTrack * 
			(_int64) disk_geo_info.BytesPerSector;
		spec.disk_info.sector_size = disk_geo_info.BytesPerSector;
		Set_Sector_Info();

		#if _DEBUG
			cout << "   " << spec.name << " size = " << size << endl;
		#endif

		if ( open_disk )
			Close( NULL );
		ending_position = size;
		spec.disk_info.ready = TRUE;
		return TRUE;
	}
	else
	{
		cout << "*** Unexpected drive type in TargetDisk::SetSizes()" << endl;
		return FALSE;
	}
}
#elif defined(SOLARIS)
int TargetDisk::Set_Sizes( BOOL open_disk )
{
	DWORD				i, fd;
	DWORD				low_size, high_size;
	DWORD				sectors_per_cluster, free_clusters, total_clusters;
	struct statvfs		st;
	BOOL				foundPartitions = FALSE;
	struct dk_geom		disk_geo_info;
	DWORD				disk_info_size;
	struct vtoc			disk_vtoc;

	// Logical and physical drives are treated differently.
	if ( open_disk )
	{
		if (!Open( NULL ) )
			return( FALSE );
	}

	if ( IsType( spec.type, LogicalDiskType ) )
	{
		// Getting physical information about the drive. All we need is the sector size. !!!!
		if ( statvfs(file_name, &st) < 0)
		{
			cout << "statvfs error " << errno << ". Cannot get sector size" << endl;
			if ( open_disk )
				Close( NULL );
			return(FALSE);
		}
		// Which one of these two do we take as the sector size ?
		spec.disk_info.sector_size = (int) st.f_bsize;
		//           or
		// spec.disk_info.sector_size = SECTOR_SIZE;

		// Ok move on to the file size
		struct stat64 stbuf64;
		if (stat64(file_name, &stbuf64) < 0)
		{
			cout << "unable to get file size. stat64() failed with error " << errno << endl;
			if ( open_disk )
				Close ( NULL );
			return(FALSE);
		}
		size = stbuf64.st_size;

		Set_Sector_Info();
		#if _DEBUG
			cout << "   " << spec.name << " size = " << size << endl << flush;
		#endif
		if ( open_disk )
			Close( NULL );

		// If test file exists, the drive is ready for access.
		if ( size )
		{
			ending_position = size;
			spec.disk_info.ready = TRUE;
			return( TRUE );
		}

		// Assuming that unlink always succeeds and never fails. how clever! #@$#%
		unlink( file_name );
		spec.disk_info.ready = FALSE;
		return( TRUE );
	}
	else if ( IsType( spec.type, PhysicalDiskType ) )
	{
		// Dealing with a physical drive.
		// Checking for a partition to exist on the physical drive.
		// If one is there, then disallowing access to the drive.
		SetLastError( 0 );
			
		if (! strstr(file_name, "p0"))
		{
			// We are probably working with a slice.
			int length;
			int part;
			char part_name[MAX_NAME];

			strcpy(part_name, spec.name);
			length = strlen(spec.name);
			part = atoi((char *)(part_name + length-1));
			spec.disk_info.sector_size = SECTOR_SIZE;

			if ((char)part_name[length-2] == 's')
			{
				// We are dealing with a slice
				size = Get_Slice_Size(part_name, part);
#ifdef _DEBUG
				cout << " slice size : " << size << endl;
#endif
				if (size <= 0)
				{
					if ( open_disk )
						Close( NULL );
					return(FALSE);
				}
			}
#if defined (__i386) || defined (_IA64)
			else
		{
				// We are dealing with an fdisk partition.
				size = Get_Partition_Size(part_name, part);
#ifdef _DEBUG
				cout << " partn size : " << size << endl;
#endif
				if (size <= 0)
				{
					if ( open_disk )
						Close( NULL );
					return(FALSE);
				}
			}
#endif // __i386 || _IA64
#ifdef _DEBUG
			cout << "spec name : " << spec.name
				<< " part_name : " << part_name << " part : " << part <<  endl;
#endif
		}
		else
		{			
			// Getting information on the size of the drive.
			fd = ((struct File *)disk_file)->fd;
			if (ioctl( fd, DKIOCG_PHYGEOM, &disk_geo_info) < 0)
			{
				// unable to get disk geometry. Report disk as NO_DRIVE
				if (open_disk)
					Close( NULL );
				return(FALSE);
			}

			// Getting sector size of the disk.
			// First try to get the sector size from the dkio(vtoc). 
			// If that fails, then return the default.
		
			// NOTE: NOTE: NOTE:
			// We do this because of a problem with ioctl(DKIOCGVTOC). It fails for a
			// freshly formatted disk. But once u create something on it and then 
			// even if you clean it all up, the call still succeeds.
			//
			if (ioctl( fd, DKIOCGVTOC, &disk_vtoc) < 0)
				spec.disk_info.sector_size = SECTOR_SIZE;
			else
				spec.disk_info.sector_size = disk_vtoc.v_sectorsz;

			// Calculating the size of the physical drive..
			size = ((DWORDLONG) disk_geo_info.dkg_ncyl * disk_geo_info.dkg_nhead * 
					disk_geo_info.dkg_nsect * spec.disk_info.sector_size);
		}

		Set_Sector_Info();

		#if _DEBUG
			cout << "   " << spec.name << " size = " << size << endl << flush;
		#endif

		if ( open_disk )
			Close( NULL );

		ending_position = size;
		spec.disk_info.ready = TRUE;
		return( TRUE );
	}
	else
	{
		cout << "*** Unexpected drive type in Disk::SetSizes()" << endl << flush;
		if ( open_disk )
			Close( NULL );
		return ( FALSE );
	}		
}
#endif /* UNIX */


#ifdef UNIX
#ifdef SOLARIS
#if defined (__i386) || defined (_IA64)
// highly specific to Solaris on Intel
BOOL TargetDisk::Look_For_Partitions()
{
	char buffer[512];
	int bytes_read, i;
	struct mboot *mb;
	struct ipart *ip;
	struct File		*fp;

	fp = (struct File *)disk_file;
	if ((bytes_read = read(fp->fd, buffer, SECTOR_SIZE)) < SECTOR_SIZE)
	{
		// cannot read from the disk. So we should try to use it.
		// simply return (TRUE) indicating that it has partitions.
		return(TRUE);
	}

	//
	// We have read 512 bytes of the first cylinder, first sector.
	// It contains the master boot record, the partition table and mboot signature (viz 0xAA55)
	//
	mb = (struct mboot *)buffer;
	if (mb->signature != MBB_MAGIC)
		// Hmmm... This drive appears to be freshly formatted. guess we can report it.!
		return(FALSE);
	else
	{
		// well, we did have a valid signature in the mboot. Look for valid partitions.
		for (i = 0; i < FD_NUMPART; i++)
		{
			ip = (struct ipart *) (buffer + BOOTSZ + (i * sizeof(struct ipart)));
			if (ip->numsect || ip->relsect)
				return(TRUE);
		}
	}
	return(FALSE);
}

DWORDLONG TargetDisk::Get_Partition_Size(char *part_name, int part)
{
	char disk_name[MAX_NAME];
	char buffer[512];
	int bytes_read, fd;
	struct ipart *ip;
	int length;

	length = strlen(part_name);
	part_name[length-1] = '0'; 		// Converting cXtXdXpX to cXtXdXp0
	sprintf(disk_name, "%s/%s", RAW_DEVICE_DIR, part_name);
	fd = open(disk_name, O_RDONLY);
	if (fd < 0)
	{
		return(0);
	}

	if ((bytes_read = read(fd, buffer, SECTOR_SIZE)) < SECTOR_SIZE)
	{
		close(fd);
		return(0);
	}

	//
	// We have read 512 bytes of the first cylinder, first sector.
	// It contains the master boot record, the partition table and mboot signature (viz 0xAA55)
	//
	close(fd);
	ip = (struct ipart *) (buffer + BOOTSZ + ((part-1) * sizeof(struct ipart)));
	return((DWORDLONG)ip->numsect * SECTOR_SIZE);
}
#endif // __i386 || _IA64

DWORDLONG TargetDisk::Get_Slice_Size(char *part_name, int part)
{
	char disk_name[MAX_NAME];
	int bytes_read, fd;
	struct vtoc this_vtoc;
	int length;

	length = strlen(part_name);
	part_name[length-1] = '2'; 		// Converting cXtXdXsX to cXtXdXs2
	sprintf(disk_name, "%s/%s", RAW_DEVICE_DIR, part_name);
	fd = open(disk_name, O_RDONLY);
	if (fd < 0)
	{
		return(0);
	}

	if (ioctl(fd, DKIOCGVTOC, &this_vtoc) < 0)
	{
		close(fd);
		return(0);
	}
	//
	// We have the vtoc.
	// It contains the slice info (including size).
	//
	close(fd);
	return((DWORDLONG)this_vtoc.v_part[part].p_size * this_vtoc.v_sectorsz);
}
#endif // SOLARIS
#endif // UNIX




//
// Setting additional information about the drive's sector size.
// The sector size of the drive needs to be known before calling this function.
//
void TargetDisk::Set_Sector_Info()
{
	DWORD	temp_sector;

	// Check if sector size is a power of 2, by repeatedly dividing the sector size by 2 
	// (until we get to 1) and checking to see if the result is odd at any point.
	temp_sector = spec.disk_info.sector_size;
	while ( temp_sector > 1 )
	{
		if ( temp_sector % 2 )
		{
			sector_align_mask = NOT_POWER_OF_TWO;
			return;
		}
		temp_sector = temp_sector >> 1;
	}

	// The sector size is a power of 2, we can use a bit mask to align requests on
	// sector boundaries.  This improves performance.
	//
	// "& ~((DWORDLONG)spec.disk_info.sector_size - 1)" masks off lower bits to force sector alignment.  
	//     spec.disk_info.sector_size is cast to a DWORDLONG to force the result to be a 64-bit quantity.  
	//     For example, if spec.disk_info.sector_size = 512 (0x00000200):
	//           (DWORDLONG)spec.disk_info.sector_size      = 0x0000000000000200
	//          ((DWORDLONG)spec.disk_info.sector_size - 1) = 0x00000000000001FF
	//         ~((DWORDLONG)spec.disk_info.sector_size - 1) = 0xFFFFFFFFFFFFFE00
	sector_align_mask = ~( (DWORDLONG)spec.disk_info.sector_size - 1 );
}


#define PREPARE_QDEPTH	16

//
// Logical drives are accessed through a file on the drive.  This will create
// the file by writing in units of "bytes" beginning at "*prepare_offset".  It
// maintains a simple I/O queue of depth PREPARE_QDEPTH, which it keeps as 
// full as possible.  It keeps queueing up new I/Os until the disk fills up, 
// or the user-specified file size (starting_sector + maximum_size) is reached,
// or a STOP_PREPARE message is received (reflected in "*test_state").  It 
// returns only when all queued I/Os have completed.  
// Return value is TRUE for success, FALSE if any error occurred.
//
BOOL TargetDisk::Prepare( void* buffer, DWORDLONG *prepare_offset, DWORD bytes, volatile TestState *test_state )
{
	BOOL			write_ok;
	int				num_outstanding;
	DWORD			bytes_written;
	OVERLAPPED		olap[PREPARE_QDEPTH];
	BOOL			busy[PREPARE_QDEPTH];
	BOOL			retval;
	int				i;

#ifdef _DEBUG
	cout << "into function TargetDisk::Prepare()" << endl;
#endif

	write_ok = TRUE;
	num_outstanding = 0;
	retval = TRUE;

	// Initialize the OVERLAPPED structures.
	for ( i = 0; i < PREPARE_QDEPTH; i++ )
	{
		// Create an event.
		olap[i].hEvent = CreateEvent( NULL, TRUE, FALSE, NULL );

#ifdef UNIX
		SetQueueSize(olap[i].hEvent, 1);
#endif

		// Was the event created successfully?
		if ( olap[i].hEvent == NULL )
		{
			cerr << "*** CreateEvent() failed in TargetDisk::Prepare()" << endl; 
			write_ok = FALSE;	// don't perform any I/O
			retval = FALSE;		// return failure status
			break;				// break out of initialization loop
		}

		// Set the low-order bit of the event handle to prevent it from being
		// picked up by a later call to GetQueuedCompletionStatus() (e.g. in
		// Grunt::Asynchronous_IOs).
		// The documentation for GetQueuedCompletionStatus() says:
		//
		//     "Even if you have passed the function a file handle associated with 
		//     a completion port and a valid OVERLAPPED structure, an application  
		//     can prevent completion port notification. This is done by 
		//     specifying a valid event handle for the hEvent member of the
		//     OVERLAPPED structure, and setting its low-order bit. A valid event
		//     handle whose low-order bit is set keeps I/O completion from being 
		//     queued to the completion port. 
		//
#if defined (_WIN32) || defined (_WIN64)
		olap[i].hEvent = (HANDLE) ( (UINT_PTR) olap[i].hEvent | 0x0000000000000001 );
#else
		olap[i].hEvent = (HANDLE) ( (unsigned long) olap[i].hEvent | 0x00000001 );
#endif


		// Mark the slot's initial state as idle.
		busy[i] = FALSE;
	}

	do
	{
		// If we're still writing and there are any available slots, queue up some
		// writes.
		if ((*test_state == TestPreparing) &&
				write_ok &&
				(num_outstanding < PREPARE_QDEPTH))
		{
			// Loop through the I/O queue looking for idle slots.
			for ( i = 0; i < PREPARE_QDEPTH; i++ )
			{
				// Check to see if we've reached the end of the disk
				if (spec.disk_info.maximum_size &&
						((*prepare_offset + bytes) 
						 > (unsigned)((spec.disk_info.starting_sector +
													 spec.disk_info.maximum_size)
													* spec.disk_info.sector_size)))
				{
					// A maximum disk size was specified by the user, and the next write 
					// would go past the specified maximum size.  
					#if _DEBUG
						cout << "User-specified maximum size reached!" << endl;
					#endif
					// Stop writing and break out of the write loop.
					write_ok = FALSE;
					break;
				}

				// If we are still writing and the slot is not busy, start an I/O for
				// this slot.
				if ( (*test_state == TestPreparing) && write_ok && !busy[i] )
				{
					// Set its address.
					olap[i].Offset = (DWORD) *prepare_offset;
					olap[i].OffsetHigh = (DWORD) ( *prepare_offset >> 32 );
					// Do the asynchronous write.
					if ( WriteFile( disk_file, (char*) buffer, bytes, &bytes_written,
													&(olap[i]) ) )
					{
						// It succeeded immediately!
#if _DEBUG
						cout << "Wrote (immediately) " << bytes_written << " of "
								 << bytes << " bytes to disk " 
								 << spec.name << "." << endl;
#endif
						// Advance the file pointer, but do not mark the slot as busy.
						*prepare_offset += bytes;
					}
					else
					{
						// It did not succeed immediately... did it start OK?
						if ( GetLastError() == ERROR_IO_PENDING )
						{
							// It started OK.
#if _DETAILS
							cout << "I/O started successfully for slot #" << i 
									 << " for " << bytes << " bytes at address " 
									 << *prepare_offset << endl;
#endif
							// Advance the file pointer and mark the slot as busy.
							*prepare_offset += bytes;
							busy[i] = TRUE;
							num_outstanding++;
						}
						else if ( GetLastError() == ERROR_DISK_FULL )
						{
							// The disk filled up -- this is an expected error.
#if _DEBUG
							cout << "Disk full (immediately) while writing "
									 <<	bytes_written << " of " << bytes << " bytes to disk "
									 <<	spec.name << "." << endl;
#endif
							// Stop writing and break out of the write loop.
							write_ok = FALSE;
							break;
						}
						else
						{
							// It didn't start OK!  Bail out.
							cout << "***Error (immediately) writing " << bytes_written <<
								" of " << bytes << " bytes to disk " << spec.name << 
								", error=" << GetLastError() << endl;
							write_ok = FALSE;	// don't perform any more I/O
							retval = FALSE;		// return failure status
							break;				// break out of write loop
						}
					}
				}
			}
		} // Done queueing up new writes.

		// If there are any outstanding I/Os, check to see if any have completed.
		if ( num_outstanding > 0 )
		{
			// Check all the busy I/O's to see if any have completed
			for ( i = 0; i < PREPARE_QDEPTH; i++ )
			{
				if ( busy[i] )
				{
					// Check to see if it has completed
					if ( GetOverlappedResult( disk_file, &(olap[i]), &bytes_written,
																		FALSE ) )
					{
						// It completed successfully!
#if _DEBUG
						cout << "Wrote (eventually) " << bytes_written << " of " << bytes
								 << " bytes to disk "  << spec.name << "." << endl;
#endif
						if (bytes_written != bytes) {
							cout << "***Error (eventually); wrote only " << bytes_written
									 << " of " << bytes << " bytes!\n";
							retval = FALSE;
							write_ok = FALSE;
							break;
						}
						// Mark the slot as idle.
						busy[i] = FALSE;
						num_outstanding--;
					}
					else if ( GetLastError() == ERROR_IO_INCOMPLETE )
					{
						// The I/O has not yet completed -- this is an expected error.
						#if _DETAILS
							cout << "I/O not yet complete for slot #" << i << endl;
						#endif
						; // Do nothing.
					}
					else if ( GetLastError() == ERROR_DISK_FULL )
					{
						// The disk filled up -- this is an expected error.
						#if _DEBUG
							cout << "Disk full (eventually) while writing " << bytes_written << " of " << bytes << " bytes to disk " << spec.name << "." << endl;
						#endif
						// Since the write did not actually succeed, decrement the file pointer.
						*prepare_offset -= bytes;
						// Stop writing and mark the slot as idle.
						write_ok = FALSE;
						busy[i] = FALSE;
						num_outstanding--;
					}
					else
					{
						// The I/O failed!  Bail out.
						cout << "***Error (eventually) writing " << bytes_written << " of " << bytes << " bytes to disk " << spec.name << 
								", error=" << GetLastError() << endl;
						// Stop writing and mark the slot as idle.
						write_ok = FALSE;
						busy[i] = FALSE;
						num_outstanding--;
						// Return failure status once all I/Os have completed.
						retval = FALSE;
					}
				}
			}
		} // Done checking for I/O completions.
	}
	while ( (*test_state == TestPreparing) && (write_ok || (num_outstanding > 0)) );
	// Keep looping until (the user tells us to stop) OR 
	// (we're done writing AND we've accumulated all the results).

	Set_Sizes( FALSE );

	// Destroy the events.
	for ( i = 0; i < PREPARE_QDEPTH; i++ )
	{
#ifdef UNIX
		// Reset the handles.
		olap[i].hEvent = (HANDLE)((unsigned int) olap[i].hEvent ^ 0x1);
#endif
		CloseHandle( olap[i].hEvent );
	}

#ifdef _DEBUG
	cout << "out of member function TargetDisk::Prepare()" << endl;
#endif
	return retval;
}



//
// Opening a disk for low-level access.
//
BOOL TargetDisk::Open( volatile TestState *test_state, int open_flag )
{
	// open_flag is a 
	if ( IsType( spec.type, LogicalDiskType ) )
	{
		// Ignore errors that occur if trying to open a floppy or CD-ROM with
		// nothing in the drive.
#ifdef SOLARIS
		((struct File *)disk_file)->fd = open(file_name, O_RDWR|O_CREAT|O_LARGEFILE|open_flag, S_IRUSR|S_IWUSR);
#elif defined(LINUX)
		((struct File *)disk_file)->fd = open(file_name, O_RDWR|O_CREAT|open_flag,
																					S_IRUSR|S_IWUSR);
#else  // WIN32 || _WIN64
		SetErrorMode( SEM_FAILCRITICALERRORS );
		disk_file = CreateFile( file_name, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | 
			FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_FLAG_NO_BUFFERING | FILE_FLAG_OVERLAPPED, NULL );
		SetErrorMode( 0 );
#endif
	}
	else if ( IsType( spec.type, PhysicalDiskType ) )
	{
#ifdef SOLARIS
		((struct File *)disk_file)->fd = open(file_name, O_RDWR|O_LARGEFILE,
																					S_IRUSR|S_IWUSR);
#elif defined(LINUX)
		((struct File *)disk_file)->fd = open(file_name, O_RDWR, S_IRUSR|S_IWUSR);
#else // WIN32 || _WIN64
		SetErrorMode( SEM_FAILCRITICALERRORS );
		disk_file = CreateFile(file_name, GENERIC_READ | GENERIC_WRITE,
													 FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
													 OPEN_EXISTING,
													 FILE_FLAG_NO_BUFFERING | FILE_FLAG_OVERLAPPED,
													 NULL );
		SetErrorMode( 0 );
#endif
	}
	else
	{
		cout << "*** Unexpected drive type in TargetDisk::Open()" << endl;
	}

	offset = starting_position;
	#if _DISK_MSGS
		cout << "Opening disk " << spec.name << endl;
	#endif
#if defined (_WIN32) || defined (_WIN64)
	if ( disk_file == INVALID_HANDLE_VALUE )
#else
	if ( ((struct File *)disk_file)->fd == (int)INVALID_HANDLE_VALUE )
#endif
		return FALSE;

	// Cludgy hack used to control reporting drives.  This will go away when
	// the TargetDisk class is divided into a disk Target subclass and a
	// Disk interface class.
	if ( !test_state )
		return TRUE;

	// Indicating where completed asynchronous transfers should be queued.
	return ( CreateIoCompletionPort( disk_file, io_cq->completion_queue, 0, 1 ) 
		!= NULL );
}


//
// Closing the disk handle.
//
BOOL TargetDisk::Close( volatile TestState *test_state )
{
	// Note that test_state is not used.  It IS used by network targets.

	// If testing connection rate, the disk may already be closed.
#if defined (_WIN32) || defined (_WIN64)
	if ( disk_file == INVALID_HANDLE_VALUE )
#else
	if ( ((struct File *)disk_file)->fd == (int)INVALID_HANDLE_VALUE )
#endif
	{
		#if _DISK_MSGS
			cout << "Disk is already closed." << endl;
		#endif
		return TRUE;
	}

	#if _DISK_MSGS
		cout << "Closing disk " << spec.name << endl;
	#endif

#if defined (_WIN32) || defined (_WIN64)
	if ( !CloseHandle( disk_file ) )
#else
	if ( !CloseHandle( disk_file, FILE_ELEMENT) )
#endif
	{
		cout << "*** Error " << GetLastError() 
			 << " closing disk in TargetDisk::Close()." << endl;
		SetLastError( 0 );
		return FALSE;
	}
#if defined (_WIN32) || defined (_WIN64)
	disk_file = INVALID_HANDLE_VALUE;
#else
	((struct File *)disk_file)->fd = (int)INVALID_HANDLE_VALUE;
#endif
	return TRUE;
}



//
// Reading a user specified amount of data from the drive and returning it.
// This does not check the request size to be a multiple of the sector size 
// (except in debug).  For performance reasons, Iometer ensures that the 
// values are correct.
//
ReturnVal TargetDisk::Read( LPVOID buffer, Transaction *trans )
{
	DWORD	error_no;

	#if _DEBUG
		// Checking for the access to be a multiple of the sector size.
		// Avoiding this check during actual testing for performance reasons.
		if ( offset % spec.disk_info.sector_size )
		{
			cout << "Invalid offset.  Not aligned with disk sector size for : " << spec.name << endl;
			return( ReturnError );
		}
		if ( trans->size % spec.disk_info.sector_size )
		{
			cout << "Invalid transfer size.  Not aligned with disk sector size for : " << spec.name << endl;
			return( ReturnError );
		}
	#endif

	#if _DETAILS
		cout << "Reading " << trans->size << " bytes from disk : " 
			<< spec.name << endl << "   Accessing : " << offset << endl;
	#endif
	
	// Determining location of read to disk.
	trans->asynchronous_io.Offset = (DWORD) offset;
	trans->asynchronous_io.OffsetHigh = (DWORD) ( offset >> 32 );

	// Reading information from the disk.
	if ( ReadFile( disk_file, buffer, trans->size, &bytes_transferred, 
		&trans->asynchronous_io ) )
	{
		// Read succeeded immediately, but completion is pending.  It will
		// still go to the completion queue.
#if defined(UNIX) && defined(IMMEDIATE_AIO_COMPLETION)
		// This code blocks reading the completion Q for the immediately completed I/Os.
		error_no = 0;
		// this is for the next sequential I/O.
		bytes_transferred = trans->size;
		return ReturnSuccess;
#else // All other cases UNIX, NT, etc.
		error_no = ERROR_IO_PENDING;
#endif
	}
	else
	{
		error_no = GetLastError();
	}

	// See if read failed.
	if ( error_no != ERROR_IO_PENDING )
	{
		// Record that no bytes were read.
		bytes_transferred = 0;
		
		// Return error.
		cout << "*** Error " << error_no << " reading " << trans->size 
			<< " bytes from disk " << spec.name << "." << endl;
		return ReturnError;
	}

	// An asynchronous read was successfully initiated!

	// Record number of bytes *to be* transferred (ReadFile() set this to 0).
	// This value will be used to determine location of next sequential I/O.
	bytes_transferred = trans->size;

	#if _DETAILS
		cout << "Queued read for " << trans->size << " bytes from disk " 
			<< spec.name << "." << endl;
	#endif
	return ReturnPending;
}



//
// Writing a user defined buffer to the drive.  This does not check the request size 
// to be a multiple of the sector size (except in debug).  For performance reasons,
// Iometer ensures that the values are correct.
//
ReturnVal TargetDisk::Write( LPVOID buffer, Transaction *trans )
{
	DWORD	error_no;

	#if _DEBUG
		// Verifying that the amount to be written is a multiple of the sector size.
		// Avoiding this check during actual testing for performance reasons.
		if ( offset % spec.disk_info.sector_size )
		{
			cout << "Invalid offset.  Not aligned with disk sector size for : " 
				<< spec.name << endl;
			return( ReturnError );
		}
		if ( trans->size % spec.disk_info.sector_size )
		{
			cout << "Invalid transfer size.  Not aligned with disk sector size for : " 
				<< spec.name << endl;
			return( ReturnError );
		}
	#endif

	#if _DETAILS
		cout << "Writing " << trans->size << " bytes to disk : " << spec.name 
			<< endl << "   Accessing : " << offset << endl;
	#endif

	// Determining location of write to disk.
	trans->asynchronous_io.Offset = (DWORD) offset;
	trans->asynchronous_io.OffsetHigh = (DWORD) ( offset >> 32 );

	// Writing information from the disk.
	if ( WriteFile( disk_file, buffer, trans->size, &bytes_transferred, 
		&trans->asynchronous_io ) )
	{
		// Write succeeded immediately, but completion is pending.  It will
		// still go to the completion queue.
#if defined(UNIX) && defined(IMMEDIATE_AIO_COMPLETION)
		// This code blocks reading the completion Q for the immediately completed I/Os.
		error_no = 0;
		// this is for the next sequential I/O.
		bytes_transferred = trans->size;
		return ReturnSuccess;
#else // All other cases UNIX, NT, etc.
		error_no = ERROR_IO_PENDING;
#endif
	}
	else
	{
		error_no = GetLastError();
	}

	// See if write failed.
	if ( error_no != ERROR_IO_PENDING )
	{
		// Record that no bytes were written.
		bytes_transferred = 0;
		
		// Return error code.
		cout << "*** Error " << error_no << " writing " << trans->size 
			<< " bytes to disk " << spec.name << "." << endl;
		return ReturnError;
	}

	// An asynchronous write was successfully initiated!  
	
	// Record number of bytes *to be* transferred (WriteFile() set this to 0).
	// This value will be used to determine location of next sequential I/O.
	bytes_transferred = trans->size;

	#if _DETAILS
		cout << "Queued write for " << trans->size << " bytes to disk " 
			<< spec.name << "." << endl;
	#endif
	return ReturnPending;
}



//
// Seek to another position on the disk.  Whether this is a random seek or a sequential
// seek will be determined by the value of the first parameter.  This does not actually
// move the disk head, it just updates the offset of the next I/O request.
//
// Calling instructions:
//
// 1) If user enters an alignment which is a power of two, pass in the user_alignment
//		the user specified and the precalculated user_align_mask.
//		This uses fast byte alignment code.
// 2) If user enters an alignment which is NOT a power of two, pass in the user_alignment
//		the user specified and NOT_POWER_OF_TWO for user_align_mask.
//		This uses slower byte alignment code.
// 3) If user enters an alignment of 0 (zero), sector alignment is assumed.  Pass in
//		zeroes for both byte_alignment and user_align_mask.
//		This uses fast byte alignment code as long as the sector size is a power
//		of two, otherwise it reverts to slower code.
//
void TargetDisk::Seek( BOOL random, DWORD request_size, DWORD user_alignment,
	DWORDLONG user_align_mask )
{
	static DWORDLONG remainder;	// static for performance reasons

	// Find out if this is a random seek.
	if ( random )
	{
		// Set the offset to a random location on the disk.
#if defined(UNIX) && defined(WORKAROUND_MOD_BUG)
		offset = starting_position + (DWORDLONG)fmod(Rand(), size);
#else
		offset = starting_position + Rand() % size;
#endif
	}
	else
	{
		// Adjusting the offset pointer by the last number of bytes successfully transferred.
		offset += (DWORDLONG) bytes_transferred;
	}

	switch (user_align_mask)
	{
	case NOT_POWER_OF_TWO:
		// If not at a user_alignment boundary, move offset ahead to next one.
		if ((remainder = offset % user_alignment))
			offset += user_alignment - remainder;

		// See if we're near the end of the file and need to return to the start.
		if ( (offset + (DWORDLONG) request_size) > ending_position )
		{
			offset = starting_position;

			// starting_position won't necessarily be byte-aligned.
			if ((remainder = offset % user_alignment))
			{
				offset += user_alignment - remainder;

				// If we went beyond the end of the disk again, report an error.
				#if _DEBUG
					if ( (offset + (DWORDLONG) request_size) > ending_position )
					{
						cout << "*** Can't align on " << user_alignment
							 << " byte boundaries" << endl;

						offset = starting_position;
					}
				#endif
			}
		}
		break;

	case 0:
		// Assume sector alignment.
		if ( sector_align_mask == NOT_POWER_OF_TWO )
		{
			if ((remainder = offset % spec.disk_info.sector_size))
				offset += spec.disk_info.sector_size - remainder;
		}
		else
		{
			if ( offset != (offset & sector_align_mask) )
				offset = (offset + spec.disk_info.sector_size) & sector_align_mask;
		}

		// See if we're near the end of the file and need to return to the start.
		if ( (offset + (DWORDLONG) request_size) > ending_position )
		{
			offset = starting_position;

			// starting_position won't necessarily be sector-aligned.
			if ( sector_align_mask == NOT_POWER_OF_TWO )
			{
				if ((remainder = offset % spec.disk_info.sector_size))
				{
					offset += spec.disk_info.sector_size - remainder;

					// If we went beyond the end of the disk again, report an error.
					#if _DEBUG
						if ( (offset + (DWORDLONG) request_size) > ending_position )
						{
							cout << "*** Can't align on " << spec.disk_info.sector_size
								 << " byte boundaries" << endl;

							offset = starting_position;
						}
					#endif
				}
			}
			else
			{
				if ( offset != (offset & sector_align_mask) )
				{
					offset = (offset + spec.disk_info.sector_size) & sector_align_mask;

					// If we went beyond the end of the disk again, report an error.
					#if _DEBUG
						if ( (offset + (DWORDLONG) request_size) > ending_position )
						{
							cout << "*** ERROR: Can't align on " << spec.disk_info.sector_size
								 << " byte boundaries" << endl;

							offset = starting_position;
						}
					#endif
				}
			}
		}
		break;
	
	default:
		// If not at a user_alignment boundary, move offset ahead to next one.
		if ( offset != (offset & user_align_mask) )
			offset = (offset + user_alignment) & user_align_mask;

		// See if we're near the end of the file and need to return to the start.
		if ( (offset + (DWORDLONG) request_size) > ending_position )
		{
			offset = starting_position;

			// starting_position won't necessarily be byte-aligned.
			if ( offset != (offset & user_align_mask) )
			{
				offset = (offset + user_alignment) & sector_align_mask;

				// If we went beyond the end of the disk again, report an error.
				#if _DEBUG
					if ( (offset + (DWORDLONG) request_size) > ending_position )
					{
						cout << "*** Can't align on " << alignment
							 << " byte boundaries" << endl;

						offset = starting_position;
					}
				#endif
			}
		}
	}
}

#ifdef LINUX

//
// Performs same function as the win32 || _WIN64 version.
//
BOOL TargetDisk::Set_Sizes(BOOL open_disk) {
	struct stat fileInfo;
	struct statfs fsInfo;
	int statResult;
	int fd = -1;
	char filesysName[MAX_NAME];

	if (open_disk) {
		if (!Open(NULL)) {
#if LINUX_DEBUG
			cout << __FUNCTION__ << ": Open on \"" << file_name <<
				"\" failed (error " << strerror(errno) << ").\n";
#endif			
			return(FALSE);
		}
		fd = ((struct File *)disk_file)->fd;
	}
	if (IsType(spec.type, LogicalDiskType)) {
    /*
		 * For logical disks, we use statfs and stat to find the size of the
		 * file system, the size of the test file, the sector size for the
		 * file system, etc. Pretty straightforward, standard Unix stuff.
		 */
		strcpy(filesysName, file_name);
		filesysName[strlen(filesysName) - strlen(TEST_FILE)] = '\0';
		if (open_disk) {
			statResult = fstatfs(fd, &fsInfo);
		} else {
			statResult = statfs(filesysName, &fsInfo);
		}
		if (statResult < 0) {
			cerr << __FUNCTION__ << ": Couldn't statfs logical disk file!\n";
			if (open_disk) {
				Close(NULL);
			}
			return(FALSE);
		}
		spec.disk_info.sector_size = fsInfo.f_bsize;
		sector_align_mask = ~((DWORDLONG)fsInfo.f_bsize - 1);
		/* Free blocks is "f_bfree". */
		if (open_disk) {
			statResult = fstat(fd, &fileInfo);
		} else {
			statResult = stat(file_name, &fileInfo);
		}
		if (statResult < 0) {
			cerr << __FUNCTION__ << ": Error " << strerror(errno) <<
				"statting file " << file_name << "\n";
			if (open_disk) {
				Close(NULL);
			}
			return(FALSE);
		}
		size = fileInfo.st_size;
		if (size == 0) {
			spec.disk_info.ready = FALSE;
			if (open_disk) {
				Close(NULL);
			}
			unlink(file_name);
			return(TRUE);
		}
		ending_position = size;
		spec.disk_info.ready = TRUE;
		if (open_disk) {
			Close(NULL);
		}
		return(TRUE);
	} else {
		spec.disk_info.sector_size = getSectorSizeOfPhysDisk(file_name);
		if (spec.disk_info.sector_size == 0) {
			cerr << __FUNCTION__ << ": Failed to get sector size. Aborting " <<
				"target.\n";
			if (open_disk) {
				Close(NULL);
			}
			return(FALSE);
		}
		size = getSizeOfPhysDisk(file_name);
		alignment = 0;
		sector_align_mask = ~((DWORDLONG)spec.disk_info.sector_size - 1);
		ending_position = size;
		offset = 0;
		bytes_transferred = 0;
		spec.disk_info.ready = TRUE;
		if (open_disk) {
			Close(NULL);
		}
		return(TRUE);
	}
}


//
// Get the sector size of a device. We do this by getting the address of the
// kernel's device sector size table, then reading "/dev/kmem" to examine this
// table. See getSectorTableLoc, below, for more detail on the structure of this
// table.
//
// This function will always return 0 (error) if the user does not have root
// access, because only root can read the "/dev/kmem" file.
//
// Parameters:
//   devName - The full path name of the device.
// Returns:
//   The size (in bytes) of a sector of the device, or 0 if the sector size
//   could not be read.
//
static int getSectorSizeOfPhysDisk(const char *devName) {
  off_t sectorTableLoc = (off_t)getSectorTableLoc();
  int major, minor;
  off_t seekResult;
  ssize_t readResult;
  int *sectorSubtableLoc;
  int result;

  if (!getDevNums(devName, &major, &minor)) {
    cerr << __FUNCTION__ << ": "
      "Can't get device numbers to find block size of \"" <<
      devName << "\".\n";
    return(0);
  }
  int fd = open("/dev/kmem", O_RDONLY);
  if (fd < 0) {
    cerr << __FUNCTION__ << ": "
      "Can't open \"/dev/kmem\" to find block size.\n";
    return(0);
  }
	sectorTableLoc += major * sizeof(int *);
  seekResult = lseek(fd, sectorTableLoc, SEEK_SET);
  assert(seekResult == sectorTableLoc);

  readResult = read(fd, &sectorSubtableLoc, sizeof(int *));
  assert(readResult == sizeof(int *));

	if (sectorSubtableLoc == NULL) {
		// A NULL subtable means that it is default size...that is, 512
		// byte blocks.
		close(fd);
		return(512);
	}
	printf("Sector table loc is 0x%lx\n", sectorTableLoc);
	printf("Subtable loc is 0x%lx\n", (off_t)sectorSubtableLoc);

  seekResult = lseek(fd, (off_t)sectorSubtableLoc, SEEK_SET);
  assert(seekResult == (off_t)sectorSubtableLoc);
  
  readResult = read(fd, &result, sizeof(int));
  assert(readResult == sizeof(int));

  close(fd);
	cout << "Block size is " << result << "\n";

  return(result);
}

//
// Return the address (in kernel memory space) of the sector table. This table
// is an array of pointers, indexed by device major number. A NULL as a pointer
// indicates that all minor devices have the default Linux sector size, which
// is 512 bytes. A non-NULL value is a pointer to an array of integers indexed
// by device minor number, with each element holding the size (in bytes) of
// a sector of this device.
// We find the address of the table by scanning through the "/proc/ksyms" file,
// which lists the addresses of all externally resolvable kernel symbols. Sample
// output of /proc/ksyms:
//////////////////////////////////////////////////////////////////////
// c808d141 i82557_config_cmd	[eepro100]
// c808d157 i82558_config_cmd	[eepro100]
// c808de54 speedo_debug	[eepro100]
// c808b04c eepro100_init	[eepro100]
// c8088318 mixer_devs_Raa8cbba2	[sound]
// c8088300 audio_devs_Ra15b59d8	[sound]
// c808832c num_mixers_R9d845b18	[sound]
// c8088314 num_audiodevs_R4cd01bdd	[sound]
// c8088370 midi_devs_R2534ed79	[sound]
// ... etc, many more symbols (including hardsect_size_...), follow ...
//////////////////////////////////////////////////////////////////////
// Returns:
//   The address (cast to be of type off_t) of the start of this table.
//
static off_t getSectorTableLoc(void) {
  unsigned long symbolPos;
  FILE *symTable;
  char symbolName[81];
  int c, scanResult;

  symTable = fopen("/proc/ksyms", "r");
  assert(symTable != NULL);
  int nameLen = strlen("hardsect_size");

  for (;;) {
    scanResult = fscanf(symTable, "%lx %80s", &symbolPos, symbolName);
    assert(scanResult == 2);
    if (!strncmp(symbolName, "hardsect_size", nameLen)) {
      fclose(symTable);
      return((off_t)symbolPos);
    }
		do {
			c = getc(symTable);
		}
		while ((c != '\n') && (c != EOF));
  }
}

//
// Get the major and minor device numbers for a given device. We get this by
// calling "stat" on the device and extracting the data.
//
// Parameters:
//   devName - The full path name of the device.
//   major - Output for writing the major device number.
//   minor - Output for writing the minor device number.
// Returns:
//   TRUE if the numbers were successfully read. FALSE if there was some
//   error.
//
static BOOL getDevNums(const char *devName, int *major, int *minor) {
  char devNameBuf[40];
  const char *fullDevName;
  struct stat devInfo;
  int statResult;

  if (devName[0] == '/') {
    fullDevName = devName;
  } else {
    sprintf(devNameBuf, "/dev/%s", devName);
    fullDevName = devNameBuf;
  }
  statResult = stat(fullDevName, &devInfo);
  if (statResult < 0) {
    cerr << __FUNCTION__ << ": "
      "Error statting device \"" << fullDevName << "\".\n";
    return(FALSE);
  }
  if (!S_ISBLK(devInfo.st_mode)) {
    cerr << __FUNCTION__ << ": "
      "Object \"" << fullDevName << "\" is not a block device!\n";
    return(FALSE);
  }
  
  *major = (devInfo.st_rdev >> 3) & 0xff;
  *minor = devInfo.st_rdev & 0xff;
  return(TRUE);
}

//
// Return the size (in bytes) of a physical disk. We get this information from
// the file "/proc/partitions". Sample output of /proc/partitions follow:
//////////////////////////////////////////////////////////////////////
// major minor  #blocks  name
// 
//    8     0    8887080 sda
//    8     1    8225248 sda1
//    8     2     658665 sda2
//   22     0    2499840 hdc
//   22     1    1249888 hdc1
//   22     2    1249920 hdc2
//////////////////////////////////////////////////////////////////////
//
// Parameters:
//   devName - The name of this device's node. Must be in the "/dev/<name>"
//             format.
// Return value:
//   The size (in bytes) of the named device.
//
static long long getSizeOfPhysDisk(const char *devName) {
	int c, numMatched;
	long long result;
	char devNameIn[21];

	assert(!strncmp(devName, "/dev/", 5));
	devName += 5;
	FILE *partInfo = fopen("/proc/partitions", "r");
	assert(partInfo != NULL);
	for (;;) {
		do {
			c = getc(partInfo);
		} while ((c != EOF) && (c != '\n'));
		numMatched = fscanf(partInfo, "%*d %*d %lld %20s",
												&result, devNameIn);
		if (numMatched != 2) {
			cerr << "Could not get size of device \"" << devName << "\"\n";
			fclose(partInfo);
			return(0);
		}
		devNameIn[20] = '\0';
		if (!strcmp(devNameIn, devName)) {
			fclose(partInfo);
			/*
			 * In /proc/partitions, the block size is always 1024 bytes.
			 */
			return(result * 1024);
		}
	}
}

#endif
