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
// AccessSpecList.cpp: Implementation of the AccessSpecList class, which is
// in charge of keeping track of all the access specifications available to
// the user.  Each access spec is defined by a Test_Spec structure. There is
// always exactly one AccessSpecList object in an instance of Iometer
// (CGalileoApp::access_spec_list).
//
///////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "AccessSpecList.h"
#include "GalileoApp.h"
#include "GalileoView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


//
// Initializing the pointer list.
//
AccessSpecList::AccessSpecList()
{
	Test_Spec* spec;
	// Initialize the pointer array.
	spec_list.SetSize( INITIAL_ARRAY_SIZE, ARRAY_GROW_STEP );

	InsertIdleSpec();
	// Insert a default spec that will be assigned to any workers that log in.
	spec = New();
	spec->default_assignment = AssignAll;
	strcpy( spec->name, "Default" );
}



//
// Inserts the idle spec.
//
void AccessSpecList::InsertIdleSpec()
{
	// Set idle spec.
	Test_Spec* spec = New();
	sprintf( spec->name, IDLE_NAME );
	spec->access[0].of_size = IOERROR;	// indicates the end of the spec.
										// Note that the end is the first
										// line of the spec, making it
										// empty.
	spec->default_assignment = FALSE;
}



//
// Removes all AccessSpecObjects form memory and removes all references 
// from the pointer array.
//
AccessSpecList::~AccessSpecList()
{
	DeleteAll();
}



//
// Creates a new AccessSpecObject and adds a pointer to it to the end of
// the pointer array.
//
Test_Spec* AccessSpecList::New()
{
	Test_Spec* spec = new Test_Spec;
	if ( !spec )
		return NULL;
	// Make the default new access spec
	InitAccessSpecLine(&(spec->access[0]));	
	spec->access[1].of_size = IOERROR;

	// Set the access spec to not load by default.
	spec->default_assignment = FALSE;
	
	// Name the new access spec.
	NextUntitled( spec->name );

	// Add the new access spec to the end of the array.
	spec_list.Add( spec );

	// Return the index in the pointer array of the newly added object's pointer.
	return spec;
}



//
// Creates a copy of the specified access spec, and returns a copy.
//
Test_Spec* AccessSpecList::Copy( Test_Spec *source_spec )
{
	int copy_number;
	CString name;
	Test_Spec* spec;

	// Create a new spec and check the validity of source_spec.
	if ( !(spec = New()) || IndexByRef( source_spec ) == IOERROR )
		return NULL;

	// Copy the source spec into the newly created one.
	memcpy( spec, source_spec, sizeof Test_Spec );

	// Assign a unique name: Copy of 'name of source spec' ('unique copy number').
	name.Format( "Copy of %s", spec->name );
	copy_number = 1;
	do
	{
		sprintf( spec->name, "%s (%d)", name, copy_number++ );
	}
	while ( RefByName( spec->name ) != spec );

	return spec;
}



//
// Deletes the specified Test_Spec object, as well as the related entry in 
// the pointer array.  Also removes any reference that spec by any worker.
//
void AccessSpecList::Delete( Test_Spec *spec )
{
	// Check for an invalid pointer.
	if ( IndexByRef( spec ) == IOERROR )
		return;

	// Remove any references to this access spec from the workers.
	theApp.manager_list.RemoveAccessSpec( spec );

	// Remove the reference to the access spec from the array.
	spec_list.RemoveAt( IndexByRef( spec ) );

	// Remove the access spec from memory.
	delete spec;
}



//
// Deletes all the access spec objects and their related entries.
//
void AccessSpecList::DeleteAll()
{
	while ( spec_list.GetSize() )
	{
		delete spec_list[ spec_list.GetUpperBound() ];
		spec_list.RemoveAt( spec_list.GetUpperBound() );
	}
	theApp.manager_list.RemoveAllAccessSpecs();
}



//
// Returns the pointer to the access spec object specified by the index.
//
Test_Spec* AccessSpecList::Get(int index)
{
	if ( index >= 0 && index < spec_list.GetSize() )
		return spec_list[index];
	else
		return NULL;
}



//
// Returns the number of entries in the array.
//
int AccessSpecList::Count()
{
	return spec_list.GetSize();
}



//
// Returns a pointer to a spec of a given name, if it exists.
//
Test_Spec* AccessSpecList::RefByName( const char* check_name )
{
	Test_Spec *spec;
	int spec_count = spec_list.GetSize();
	for ( int s = 0; s < spec_count; s++ )
	{
		spec = Get( s );
		if ( _stricmp( spec->name, check_name ) == 0 )
			return spec;
	}
	return NULL;
}



//
// Returns the index of a spec given a reference to that spec.
// This function is used to validate spec pointers.
// Also used in Delete() to index into the array, and in 
// Save() to determine which specs where running. 
//
int AccessSpecList::IndexByRef( const Test_Spec* spec )
{
	int spec_count = spec_list.GetSize();
	for ( int index = 0; index < spec_count; index++ )
	{
		if ( Get( index ) == spec )
			return index;
	}
	return IOERROR;
}



//
// Saving the access specification list to a result file.
// Saves only the currently active access specs.
//
BOOL AccessSpecList::SaveResults( ostream& outfile )
{
	int i, j;
	int access_count, manager_count, worker_count, current_access_index;
	Manager *mgr;
	Worker *wkr;
	BOOL *spec_active;
	Test_Spec *spec;

	outfile << "'Access specifications" << endl;

	access_count = Count();
	spec_active = new BOOL[ access_count ];

	// Determine which access specs are active for the current test
	// Start by marking all access specs inactive.
	for ( i = 1; i < access_count; i++ )
	{
		spec_active[i] = FALSE;
	}

	current_access_index = theApp.pView->GetCurrentAccessIndex();
	// Now go through all the workers and mark each worker's current access spec 
	// as active, IF the worker is actually doing anything in this test.
	manager_count = theApp.manager_list.ManagerCount();
	for ( i = 0; i < manager_count; i++ )
	{
		mgr = theApp.manager_list.GetManager( i );
		worker_count = mgr->WorkerCount();
		for ( j = 0; j < worker_count; j++ )
		{
			wkr = mgr->GetWorker( j );
			if ( wkr->ActiveInCurrentTest() )
			{
				spec_active[ IndexByRef( wkr->GetAccessSpec( 
								current_access_index ) ) ] = TRUE;
			}
		}
	}

	// Save all the active access specs except the idle spec.
	for ( i = 1; i < access_count; i++ )
	{
		if ( !spec_active[i] )
			continue;

		spec = Get( i );

		outfile << "'Access specification name,default assignment" << endl;
		outfile << spec->name << "," << spec->default_assignment << endl;

		// Write access specifications to a file, data comma separated.
		outfile << "'size,% of size,% reads,% random,delay,burst,align,reply" << endl;

		for ( int line_index = 0; line_index < MAX_ACCESS_SPECS; line_index++ )
		{
			if ( spec->access[line_index].of_size == IOERROR )
				break;
			outfile
				<< spec->access[line_index].size << ","
				<< spec->access[line_index].of_size << ","
				<< spec->access[line_index].reads << ","
				<< spec->access[line_index].random << ","
				<< spec->access[line_index].delay << ","
				<< spec->access[line_index].burst << ","
				<< spec->access[line_index].align << ","
				<< spec->access[line_index].reply << endl;
		}
	}

	outfile << "'End access specifications" << endl;

	delete spec_active;

	return TRUE;
}



//
// Saves the access specification list to a config file.
// Saves ALL specs, whether they are active or not.
// It is tempting to combine this with SaveResults, but changes to one should
// not generally affect the other.  At one point, the output of these two
// functions was identical, but there are differences now, and there will
// continue to be more in the future.
//
BOOL AccessSpecList::SaveConfig( ostream& outfile )
{
	int i, line_index;
	int access_count;
	Test_Spec *spec;

	outfile << "'ACCESS SPECIFICATIONS ================="
			   "========================================" << endl;

	access_count = Count();

	// Save all the active access specs except the idle spec (0).
	for ( i = 1; i < access_count; i++ )
	{
		spec = Get( i );

		outfile << "'Access specification name,default assignment" << endl;
		outfile << "\t" << spec->name << ",";
		
		switch (spec->default_assignment)
		{
		case AssignNone:
			outfile << "NONE" << endl;
			break;
		case AssignAll:
			outfile << "ALL" << endl;
			break;
		case AssignDisk:
			outfile << "DISK" << endl;
			break;
		case AssignNet:
			outfile << "NET" << endl;
			break;
		default:
			ErrorMessage("Error saving access specification list.  Access specification "
					+ (CString)spec->name + " has an illegal default assignment.");
			return FALSE;
		}

		// Write access specifications to a file, data comma separated.
		outfile << "'size,% of size,% reads,% random,delay,burst,align,reply" << endl;

		for ( line_index = 0; line_index < MAX_ACCESS_SPECS; line_index++ )
		{
			if ( spec->access[line_index].of_size == IOERROR )
				break;
			outfile << "\t"
				<< spec->access[line_index].size << ","
				<< spec->access[line_index].of_size << ","
				<< spec->access[line_index].reads << ","
				<< spec->access[line_index].random << ","
				<< spec->access[line_index].delay << ","
				<< spec->access[line_index].burst << ","
				<< spec->access[line_index].align << ","
				<< spec->access[line_index].reply << endl;
		}
	}

	outfile << "'END access specifications" << endl;

	return TRUE;
}



//
// Checks the config file version number and calls the appropriate load function.
// Also handles the replace flag.
//
// If replace is TRUE, all access specs are removed before the restore,
// otherwise, the loaded specs are merged with the preexisting specs.
//
BOOL AccessSpecList::LoadConfig( const CString& infilename, BOOL replace )
{
	ICF_ifstream infile(infilename);
	long version;
	BOOL retval;

	if ( replace )
	{
		// Clear memory and display before loading accesses.
		DeleteAll();
		InsertIdleSpec();
	}

	version = infile.GetVersion();
	if (version == -1)
		return FALSE;

	if ( !infile.SkipTo("'ACCESS SPECIFICATIONS") )
		return TRUE;	// no access spec list to restore (this is OK)

	// Check to see whether to load a per-worker access spec, or an old style access spec.
	if ( version >= 19980521 )
	{
		retval = LoadConfigNew(infile);

		// There's no need to assign the default access specs here.
		// They will be assigned automatically when workers are created
		// (by the Worker constructor itself), and they will be removed
		// by the Manager::LoadConfig code if they shouldn't be assigned.
	}
	else
	{
		retval = LoadConfigOld(infile);

		// If replacing, assign default specs to workers.
		if ( replace )
			theApp.manager_list.AssignDefaultAccessSpecs();
	}

	infile.close();

	return retval;
}



//
// Loads the global access spec list from a file.
// Does not support old style saved configurations.
//
BOOL AccessSpecList::LoadConfigNew( ICF_ifstream& infile )
{
	CString key, value;
	CString token;
	int line_index;
	int total_access;
	Test_Spec* spec;

	while (1)
	{
		if ( !infile.GetPair(key, value) )
		{
			ErrorMessage("File is improperly formatted.  Expected an "
				"access specification or \"END access specifications\".");
			return FALSE;
		}

		if ( key.CompareNoCase("'END access specifications") == 0 )
		{
			break;
		}
		if ( key.CompareNoCase("'Access specification name,default assignment") == 0 )
		{
			if ( spec = RefByName( (LPCTSTR)value.Left( value.Find( ',' ) ) ) )
			{
#ifdef VERBOSE
				ErrorMessage( "An access specification named \""
					+ value.Left( value.Find( ',' ) )
					+ "\" already exists.  It will be replaced by "
					"the new access specification." );
#endif
			}
			else
			{
				// No previous access spec by this name.  Create a new one.
				spec = New();

				// Sanity check.
				if ( !spec )
				{
					ErrorMessage( "Error while loading file.  Out of memory.  "
						"Error occured in AccessSpecList::LoadConfig()" );
					return FALSE;
				}
			}

			// Fill in the name.
			token = ICF_ifstream::ExtractFirstToken(value, TRUE);
			strcpy( spec->name, (LPCTSTR)token );

			// Read the default assignment.
			token = ICF_ifstream::ExtractFirstToken(value);
			if ( token.CompareNoCase("NONE") == 0 )
				spec->default_assignment = AssignNone;
			else if ( token.CompareNoCase("ALL") == 0 )
				spec->default_assignment = AssignAll;
			else if ( token.CompareNoCase("DISK") == 0 )
				spec->default_assignment = AssignDisk;
			else if ( token.CompareNoCase("NET") == 0 )
				spec->default_assignment = AssignNet;
			else
				spec->default_assignment = atoi( (LPCTSTR)token );

			key = infile.GetNextLine();
			if (key.GetAt(0) != '\'')
			{
				ErrorMessage("File is improperly formatted.  Expected a "
					"comment line to follow the access specification name.");
				return FALSE;
			}

			// Get the details of the access spec.

			line_index = 0;
			total_access = 0;

			// Read lines until total percentage reaches 100.
			while (infile.peek() != '\'')
			{
				char comma;	// Used to throw commas away.

				infile >> spec->access[line_index].size >> comma;
				infile >> spec->access[line_index].of_size >> comma;
				infile >> spec->access[line_index].reads >> comma;
				infile >> spec->access[line_index].random >> comma;
				infile >> spec->access[line_index].delay >> comma;
				infile >> spec->access[line_index].burst >> comma;
				infile >> spec->access[line_index].align >> comma;
				infile >> spec->access[line_index].reply;
				infile.GetNextLine();

				// Sanity check.
				if ( spec->access[line_index].of_size > 100	|| 
					 spec->access[line_index].reads > 100	|| 
					 spec->access[line_index].random > 100	|| 
					 spec->access[line_index].of_size < 0	|| 
					 spec->access[line_index].reads < 0		|| 	
					 spec->access[line_index].random < 0 )
				{
					Delete( spec );
					ErrorMessage( "Error loading global access specifications.  "
						"Invalid value encountered." );
					return FALSE;
				}

				total_access += spec->access[line_index].of_size;
				line_index++;
			}

			// Mark the end of the access spec.
			if ( line_index <= MAX_ACCESS_SPECS )
				spec->access[line_index].of_size = IOERROR;

			// Sanity check.
			if ( total_access != 100 )
			{
				ErrorMessage( "Error loading global access specifications.  "
					"Percentages don't add to 100." );
				Delete( spec );
				return FALSE;
			}

		}
		else
		{
			ErrorMessage("File is improperly formatted.  Global ACCESS "
				"SPECIFICATION section contained an unrecognized \""
				+ key + "\" comment.");
			return FALSE;
		}
	}

	return TRUE;
}



//
// Loads an old style access spec.
//
BOOL AccessSpecList::LoadConfigOld( ICF_ifstream& infile )
{
	int total_access;
	int line_index;
	Test_Spec* spec;
	char comment[100];

	// Read and discard initial comment line.
	infile.getline( comment, 100 );
	
	// Create new test spec.
	spec = New();
	line_index = 0;
	total_access = 0;

	// Get access specs from the file until an error occurs or all specs have been read in.
	while ( !infile.rdstate() )
	{
		// Read in specifications until another comment is read in from the file.
		// This signifies the end of the access spec portion.
		if ( infile.peek() == '\'' || infile.peek() == EOF )
		{
			// Delete the last spec created.
			if ( spec )
				Delete( spec );
			
			return TRUE;
		}
		
		// Sanity check.
		if ( !spec )
		{
			ErrorMessage( "Error allocating memory for access specification." );
			return FALSE;
		}

		InitAccessSpecLine( &(spec->access[line_index]) );
		infile >> spec->access[line_index].size;
		infile >> spec->access[line_index].of_size;
		infile >> spec->access[line_index].reads;
		infile >> spec->access[line_index].random;
		infile >> spec->access[line_index].delay;
		infile >> spec->access[line_index].burst;
		infile.ignore( 1, '\n' );

		// Sanity check.
		if ( spec->access[line_index].of_size > 100	|| 
			 spec->access[line_index].reads > 100	|| 
			 spec->access[line_index].random > 100	|| 
			 spec->access[line_index].of_size < 0	|| 
			 spec->access[line_index].reads < 0		|| 	
			 spec->access[line_index].random < 0 )
		{
			break;
		}

		// Skip lines that make up "0%" of the access.
		if ( spec->access[line_index].of_size )
		{
			total_access += spec->access[line_index].of_size;
			line_index++;
		}
		else
		{
			ErrorMessage( "Found an access specification line that makes up 0% of the test.  "
						   "This line will be discarded." );
		}

		// Sanity check.
		if ( total_access > 100 )
			break;
		
		// Mark the end of the access spec if needed.
		if ( total_access == 100 )
		{
			spec->default_assignment = AssignAll;
			if ( line_index <= MAX_ACCESS_SPECS )
				spec->access[line_index].of_size = IOERROR;
	
			// Try smart naming if possible.
			SmartName( spec );

			// Create new test spec.
			spec = New();

			// Prepare to load next spec from file.
			line_index = 0;
			total_access = 0;
		}
	}

	// Delete the last spec created.
	if ( spec )
		Delete( spec );

	ErrorMessage( "Error loading access specification.  Invalid access specification." );
	return FALSE;
}



//
// This function initializes a line of an access spec to the default values.
//
void AccessSpecList::InitAccessSpecLine(Access_Spec* spec_line)
{
	spec_line->size = 2048;
	spec_line->of_size = 100;
	spec_line->random = 100;
	spec_line->reads = 67;
	spec_line->delay = 0;
	spec_line->burst = 1;
	spec_line->align = 0;
	spec_line->reply = 0;
}



//
// This function will return an integer that will be the next sequential 
// n for naming "Untitled n" access specs.
//
void AccessSpecList::NextUntitled( char *name )
{
	int next_number = 0;
	CString current_name;

	// Counting the number of "Untitled *" specs.
	for ( int index = 0; index < spec_list.GetSize(); index++ )
	{
		current_name = Get(index)->name;
		if (current_name.Find("Untitled") == 0)
		{
			++next_number;
		}
	}

	// Ensure that names are not duplicated.
	do
	{
		// We have the current number, so we add 1 for the next number.
		sprintf( name, "Untitled %d", ++next_number );
	}
	while ( RefByName( name ) );
}



//
// Assigning a reasonable name to an older access spec that doesn't have one.
//
void AccessSpecList::SmartName( Test_Spec* spec )
{
	CString	name;
	int		spec_number = 1;
	int		name_size;

	// Verify that smart naming is possible.
	if ( spec->access[1].of_size != IOERROR )
		return;	// use existing name

	// Switch statement for size part of name
	switch (spec->access[0].size)
	{
	case 512:
		name = "512byte";
		break;
	case MEGABYTE:
		name = "1MB";
		break;
	default:
		name.Format("%dKB",(spec->access[0].size/KILOBYTE));
	}
	
	// Switch statement for random/sequential part of name
	switch (spec->access[0].random )
	{
	case 0:
		name +=" sequential";
		break;
	case 100:
		name += " random";
		break;
	default:
		name.Format("%s %d%% random", name.Left( name.GetLength() ),
			spec->access[0].random);
	}

	// Switch statement for read/write part of name
    switch (spec->access[0].reads)
	{
	case 0:
		name+=" writes";
		break;
	case 100:
		name+=" reads";
		break;
	default:
		name.Format("%s %d%% reads", name.Left( name.GetLength() ), 
			spec->access[0].reads);
	}
	
	// Test to ensure no duplicate names
	name_size = name.GetLength();
	while ( RefByName( (LPCTSTR) name ) )
	{
		name.Format("%s %d", name.Left( name_size ), ++spec_number);
	}		
	sprintf( spec->name, "%s", name );
}
