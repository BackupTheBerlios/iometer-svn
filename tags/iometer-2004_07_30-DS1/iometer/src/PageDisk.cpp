/* ######################################################################### */
/* ##                                                                     ## */
/* ##  Iometer / PageDisk.cpp                                             ## */
/* ##                                                                     ## */
/* ## ------------------------------------------------------------------- ## */
/* ##                                                                     ## */
/* ##  Job .......: Implementation of the CPageDisk class, which is       ## */
/* ##               responsible for the "Disk Targets" tab in Iometer's   ## */
/* ##               main window.                                          ## */
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
/* ##  Changes ...: 2003-10-17 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Moved to the use of the IOMTR_[OSFAMILY|OS|CPU]_*   ## */
/* ##                 global defines.                                     ## */
/* ##               - Integrated the License Statement into this header.  ## */
/* ##               2003-04-25 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Updated the global debug flag (_DEBUG) handling     ## */
/* ##                 of the source file (check for platform etc.).       ## */
/* ##               2003-03-28 (joe@eiler.net)                            ## */
/* ##               - changes so VC++ 7 (.NET) will compile correctly.    ## */
/* ##               2003-03-28 (joe@eiler.net)                            ## */
/* ##               - added new header info.                              ## */
/* ##                                                                     ## */
/* ######################################################################### */


#include "stdafx.h"
#include "PageDisk.h"
#include "ManagerList.h"
#include "GalileoView.h"


// Needed for MFC Library support for assisting in finding memory leaks
//
// NOTE: Based on the documentation[1] I found, it should be enough to have
//       a "#define new DEBUG_NEW" statement for the case, that we are
//       running Windows. There should be no need for checking the _DEBUG
//       flag and no need for redefiniting the THIS_FILE string. Maybe there
//       will be a MFC hacker who could advice here.
//       [1] = http://msdn.microsoft.com/library/default.asp?url=/library/en-us/vclib/html/_mfc_debug_new.asp
//
#if defined(IOMTR_OS_WIN32) || defined(IOMTR_OS_WIN64)
 #ifdef _DEBUG
  #define new DEBUG_NEW
  #undef THIS_FILE
  static char THIS_FILE[] = __FILE__;
 #endif
#endif


/////////////////////////////////////////////////////////////////////////////
// CPageDisk property page

IMPLEMENT_DYNCREATE(CPageDisk, CPropertyPage)

CPageDisk::CPageDisk() : CPropertyPage(CPageDisk::IDD)
{
	//{{AFX_DATA_INIT(CPageDisk)
	//}}AFX_DATA_INIT
}

CPageDisk::~CPageDisk()
{
}

void CPageDisk::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPageDisk)
	DDX_Control(pDX, EQueueDepth, m_EQueueDepth);
	DDX_Control(pDX, SConnectionRate, m_SConnectionRate);
	DDX_Control(pDX, CConnectionRate, m_CConnectionRate);
	DDX_Control(pDX, TTargets, m_TTargets);
	DDX_Control(pDX, EConnectionRate, m_EConnectionRate);
	DDX_Control(pDX, EDiskStart, m_EDiskStart);
	DDX_Control(pDX, EDiskSize, m_EDiskSize);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPageDisk, CPropertyPage)
	//{{AFX_MSG_MAP(CPageDisk)
	ON_EN_KILLFOCUS(EQueueDepth, OnKillfocusEQueueDepth)
	ON_EN_KILLFOCUS(EDiskSize, OnKillfocusEDiskSize)
	ON_EN_KILLFOCUS(EDiskStart, OnKillfocusEDiskStart)
	ON_EN_SETFOCUS(EQueueDepth, OnSetfocusEQueueDepth)
	ON_EN_SETFOCUS(EDiskSize, OnSetfocusEDiskSize)
	ON_EN_SETFOCUS(EDiskStart, OnSetfocusEDiskStart)
	ON_NOTIFY(TVN_KEYDOWN, TTargets, OnKeydownTTargets)
	ON_NOTIFY(NM_CLICK, TTargets, OnClickTTargets)
	ON_EN_KILLFOCUS(EConnectionRate, OnKillfocusEConnectionRate)
	ON_EN_SETFOCUS(EConnectionRate, OnSetfocusEConnectionRate)
	ON_NOTIFY(UDN_DELTAPOS, SConnectionRate, OnDeltaposSConnectionRate)
	ON_BN_CLICKED(CConnectionRate, OnCConnectionRate)
	ON_NOTIFY(NM_SETFOCUS, TTargets, OnSetfocusTTargets)
	ON_NOTIFY(TVN_SELCHANGING, TTargets, OnSelchangingTTargets)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPageDisk message handlers

BOOL CPageDisk::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();

	// Associate the image list with the TTargets tree control
	m_ImageList.Create( IDB_TARGET_ICONS, TARGET_ICON_SIZE, TARGET_ICON_EXPAND, TARGET_ICON_BGCOLOR );
	m_TTargets.SetImageList( &m_ImageList, TVSIL_NORMAL );
	m_TTargets.SetImageList( &m_ImageList, TVSIL_STATE ); // Use the same image list for state images

	// Set the spin control to translate an up click into a positive movement.
	m_SConnectionRate.SetRange( 0, UD_MAXVAL );

	Reset();
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}



//
// Resetting contents of display.
//
void CPageDisk::Reset()
{
	// Clear the target list.
	m_TTargets.DeleteAllItems();
	
	// Display the conneciton rate and disk settings.
	ShowConnectionRate();
	ShowSettings();

	selected = NULL;
	highlighted = NULL;

	EnableWindow();
}



//
// Enabling controls on the form for user access.
//
void CPageDisk::EnableWindow( BOOL enable )
{
	Manager	*manager;
	Worker	*worker;
	CWnd	*last_focused;

	// Only enable things if a test is not running.
	if ( theApp.test_state != TestIdle )
		return;

	// Get the current worker and manager selections.
	manager = theApp.pView->m_pWorkerView->GetSelectedManager();
	worker = theApp.pView->m_pWorkerView->GetSelectedWorker();

	switch ( theApp.pView->m_pWorkerView->GetSelectedType() )
	{
	case WORKER:
		// Enable the target list only if the selected worker is a disk worker.
		enable = enable && IsType( worker->Type(), GenericDiskType );
		m_TTargets.EnableWindow( enable );

		// If the selected worker is not a disk worker, switch to the network
		// target page.
		if ( !IsType( worker->Type(), GenericDiskType ) && 
			theApp.pView->m_pPropSheet->GetActiveIndex() == DISK_PAGE )
		{
			last_focused = GetFocus();
			theApp.pView->m_pPropSheet->SetActivePage( NETWORK_PAGE );
			if ( last_focused )
				last_focused->SetFocus();
		}
		break;
	case MANAGER:
		// Enable the target list only if the selected manager has a 
		// disk worker.
		enable = enable && manager->WorkerCount( GenericDiskType );
		m_TTargets.EnableWindow( enable );

		// If the selected manager has no disk workers but has network server
		// workers, switch to the network target page.
		if ( !manager->WorkerCount( GenericDiskType ) && 
			manager->WorkerCount( GenericServerType ) && 
			theApp.pView->m_pPropSheet->GetActiveIndex() == DISK_PAGE )
		{
			enable = FALSE;
			last_focused = GetFocus();
			theApp.pView->m_pPropSheet->SetActivePage( NETWORK_PAGE );
			if ( last_focused )
				last_focused->SetFocus();
		}
		break;
	case ALL_MANAGERS:
		// Only enable the connection rate controls if there exists a
		// GenericDiskType worker.
		enable = enable && theApp.manager_list.WorkerCount( GenericDiskType );
		
		// The target list is disabled for all managers.
		m_TTargets.EnableWindow( FALSE );
	}

	// Enable the connection rate check box.
	m_CConnectionRate.EnableWindow( enable );

	// Enable the connection rate edit box and spin control if the connection 
	// rate check box is checked.
	m_EConnectionRate.EnableWindow( enable &&
		m_CConnectionRate.GetCheck() == 1 );
	m_SConnectionRate.EnableWindow( enable &&
		m_CConnectionRate.GetCheck() == 1 );

	// Enable the disk parameter controls.
	m_EDiskStart.EnableWindow( enable );
	m_EDiskSize.EnableWindow( enable );
	m_EQueueDepth.EnableWindow( enable );
		
	// The spin button sometimes needs to be redrawn even if it didn't change
	// when the edit box was partially redrawn.
	m_SConnectionRate.Invalidate( TRUE );

	// Redraw the page.  Note that all the child windows will be redrawn too.
	// Also note that only windows that have already been invalidated will get
	// redrawn, avoiding much flicker.
	RedrawWindow( NULL, NULL, RDW_ALLCHILDREN | RDW_UPDATENOW | RDW_ERASE );
}



//
// Displays the values from memory in the GUI.
//
void CPageDisk::ShowData()
{
	Manager	*manager;
	Worker	*worker;

	// Only enable the display if a disk worker or a manager with disk 
	// interfaces is selected.
	manager = theApp.pView->m_pWorkerView->GetSelectedManager();
	worker = theApp.pView->m_pWorkerView->GetSelectedWorker();
	if (( theApp.pView->m_pWorkerView->GetSelectedType() == ALL_MANAGERS) ||
		( manager && !manager->InterfaceCount( GenericDiskType ) ) ||
		( worker && !IsType( worker->Type(), GenericDiskType ) ) )
	{
		Reset();
		return;
	}

	// This is a new view of the target assignment.  
	// It has not been modified.
	selected = NULL;
	highlighted = NULL;
	// Show the target list.
	ShowTargets();
	// Set the target selection check boxes.
	ShowTargetSelection();
	// Bold the last selected item.
	ShowFocus();
	// Show the connection rate settings.
	ShowConnectionRate();
	// Show the disk specific settings.
	ShowSettings();
	// Enable the apropriate windows and redraw the page.
	EnableWindow();
}



//
// Fill in the target list.
//
void CPageDisk::ShowTargets()
{
	int				i, iface_count;
	Manager			*manager;
	Target_Spec		*disk;
	TV_INSERTSTRUCT	tvstruct;

	manager = theApp.pView->m_pWorkerView->GetSelectedManager();
	if ( !manager )
		return;

	tvstruct.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT | 
		TVIF_STATE;
	// Set the state mask to indicate that the state parameter refers to the
	// index of the state image in the state image list.
	tvstruct.item.stateMask = TVIS_STATEIMAGEMASK;

	// position the disk in the tree
	tvstruct.hParent = TVI_ROOT;
	tvstruct.hInsertAfter = TVI_LAST;

	// Clear the contents.
	m_TTargets.DeleteAllItems();

	iface_count = manager->InterfaceCount( GenericDiskType );
	for ( i = 0; i < iface_count; i++ )
	{
		// name the disk
		disk = manager->GetInterface( i, GenericDiskType );
		tvstruct.item.pszText = disk->name;

		// set the icon associated with the disk
		switch ( disk->type )
		{
		case PhysicalDiskType:
			tvstruct.item.iImage = TARGET_ICON_PHYSDISK;
			tvstruct.item.iSelectedImage = TARGET_ICON_PHYSDISK;
			break;
		case LogicalDiskType:
			if ( disk->disk_info.ready )
			{
				tvstruct.item.iImage = TARGET_ICON_LOGDISK_PREPARED;
				tvstruct.item.iSelectedImage = TARGET_ICON_LOGDISK_PREPARED;
			}
			else
			{
				tvstruct.item.iImage = TARGET_ICON_LOGDISK_UNPREPARED;
				tvstruct.item.iSelectedImage = TARGET_ICON_LOGDISK_UNPREPARED;
			}
			break;
		default:
			ErrorMessage( "Unexpected disk type in CPageDisk::ShowTargets()");
			return;
		}

		tvstruct.item.state = INDEXTOSTATEIMAGEMASK ( TARGET_STATE_UNKNOWN );

		// finally, insert the disk into the tree control
		if ( m_TTargets.InsertItem( &tvstruct ) == NULL )
		{
			ErrorMessage("InsertItem() failed in CPageDisk::ShowTargets()");
			return;
		}
	}
}



void CPageDisk::ShowTargetSelection()
{
	Manager			*manager;
	Worker			*worker;
	HTREEITEM		hiface;
	int				w, i, expected_worker, iface_count, wkr_count;
	TargetSelType	state;
	Target_Spec		target_info;

	// Get the selected manager.
	manager = theApp.pView->m_pWorkerView->GetSelectedManager();
	// Get the first disk item from the target tree.
	hiface = m_TTargets.GetRootItem();

	switch ( theApp.pView->m_pWorkerView->GetSelectedType() )
	{
	case WORKER:
		// A worker is selected, show its assigned targets.
		worker = theApp.pView->m_pWorkerView->GetSelectedWorker();

		// Loop through the manager's disks, and mark which ones are selected
		// for the worker.
		iface_count = manager->InterfaceCount( GenericDiskType );
		for ( i = 0; i < iface_count; i++ )
		{
			if ( worker->IsTargetAssigned( manager->GetInterface(i, GenericDiskType)))
				SetSelectionCheck( hiface, TargetChecked );
			else
				SetSelectionCheck( hiface, TargetUnChecked );

			hiface = m_TTargets.GetNextSiblingItem( hiface );
		}
		break;

	case MANAGER:
		// Show the targets assigned to a manager's workers if possible.
		expected_worker = 0;

		// Loop through the manager's disks, and see if they are assigned
		// to the expected worker.  If not, gray all of them and return.
		iface_count = manager->InterfaceCount( GenericDiskType );
		wkr_count = manager->WorkerCount( GenericDiskType );
		for ( i = 0; i < iface_count; i++ )
		{
			memcpy( &target_info, manager->GetInterface( i, GenericDiskType ), 
				sizeof( Target_Spec ) );
			state = TargetUnChecked;
			for ( w = 0; w < wkr_count; w++ )
			{
				// If the disk is selected by any other than expected
				// worker, all the disk check boxes are grayed.
				if ( manager->GetWorker(w, GenericDiskType)->IsTargetAssigned(
					&target_info ) )
				{
					if ( w != expected_worker || state != TargetUnChecked )
					{
						SetAllCheck( TargetGrayed );
						return;
					}
					state = TargetChecked;
					if ( ++expected_worker == wkr_count )
					{
						expected_worker = 0;
					}
				}
			}
			SetSelectionCheck( hiface, state );
			hiface = m_TTargets.GetNextSiblingItem( hiface );
		}
		break;
	default:
		SetAllCheck( TargetGrayed );
		break;
	}
}



//
// Displays the connection rate settings for the current selection in the
// worker view.  If the selection is a manager or all managers, displays
// a value if all the children's values are the same.
//
void CPageDisk::ShowConnectionRate()
{
	Manager	*manager;
	Worker	*worker;
	int trans_per_conn=0;
	int test_connection_rate=0;

	switch ( theApp.pView->m_pWorkerView->GetSelectedType() )
	{
	case WORKER:
		// update controls with worker's data
		worker = theApp.pView->m_pWorkerView->GetSelectedWorker();
		if ( IsType( worker->Type(), GenericDiskType ) )
		{
			trans_per_conn = worker->GetTransPerConn( GenericDiskType );
			test_connection_rate = worker->GetConnectionRate( GenericDiskType );
		}
		break;
	case MANAGER:
		manager = theApp.pView->m_pWorkerView->GetSelectedManager();
		trans_per_conn = 
			manager->GetTransPerConn( GenericDiskType );
		test_connection_rate = 
			manager->GetConnectionRate( GenericDiskType );
		break;
	default:
		trans_per_conn = 
			theApp.manager_list.GetTransPerConn( GenericDiskType );
		test_connection_rate = 
			theApp.manager_list.GetConnectionRate( GenericDiskType );
		break;
	}
	// If the test connection rate settings are different between a manager's
	// workers, set the state of the check box to AUTO3STATE and disable the
	// edit box and spin control.
	SetDlgItemInt( EConnectionRate, trans_per_conn );
	if ( test_connection_rate == AMBIGUOUS_VALUE )
	{
		m_CConnectionRate.SetButtonStyle( BS_AUTO3STATE );
		m_EConnectionRate.SetPasswordChar( 32 );
		m_EConnectionRate.Invalidate( TRUE );

		// Set check box to undetermined state.
		CheckDlgButton( CConnectionRate, 2 );
	}
	else
	{
		m_CConnectionRate.SetButtonStyle( BS_AUTOCHECKBOX );
		CheckDlgButton( CConnectionRate, test_connection_rate );

		if ( test_connection_rate == ENABLED_VALUE && trans_per_conn != 
			AMBIGUOUS_VALUE )
		{
			m_EConnectionRate.SetPasswordChar( 0 );
		}
		else
		{
			m_EConnectionRate.SetPasswordChar( 32 );
			m_EConnectionRate.Invalidate();
		}
	}
}



//
// Shows the selected item's disk starting sector, size, and queue depth.
//
void CPageDisk::ShowSettings()
{
	Manager *manager;
	Worker	*worker;
	int disk_size;
	int disk_start;
	int queue_depth;

	// Get the values for the disk settings from memory.
	switch ( theApp.pView->m_pWorkerView->GetSelectedType() )
	{
	case WORKER:
		worker = theApp.pView->m_pWorkerView->GetSelectedWorker();
		disk_size = worker->GetDiskSize( GenericDiskType );
		disk_start = worker->GetDiskStart( GenericDiskType );
		queue_depth = worker->GetQueueDepth( GenericDiskType );
		break;
	case MANAGER:
		manager = theApp.pView->m_pWorkerView->GetSelectedManager();
		disk_size = manager->GetDiskSize( GenericDiskType );
		disk_start = manager->GetDiskStart( GenericDiskType );
		queue_depth = manager->GetQueueDepth( GenericDiskType );
		break;
	default:
		disk_size = theApp.manager_list.GetDiskSize( GenericDiskType );
		disk_start = theApp.manager_list.GetDiskStart( GenericDiskType );
		queue_depth = theApp.manager_list.GetQueueDepth( GenericDiskType );
	}
	// If there is a displayable value, display it.  Otherwise, hide the content
	// of the edit box.
	if ( disk_size == AMBIGUOUS_VALUE )
	{
		m_EDiskSize.SetPasswordChar( 32 );
		m_EDiskSize.Invalidate();
	}
	else
	{
		m_EDiskSize.SetPasswordChar( 0 );
		SetDlgItemInt( EDiskSize, disk_size );
	}

	if ( disk_start == AMBIGUOUS_VALUE )
	{
		m_EDiskStart.SetPasswordChar( 32 );
		m_EDiskStart.Invalidate();
	}
	else
	{
		m_EDiskStart.SetPasswordChar( 0 );
		SetDlgItemInt( EDiskStart, disk_start );
	}
	
	if ( queue_depth == AMBIGUOUS_VALUE )
	{
		m_EQueueDepth.SetPasswordChar( 32 );
		m_EQueueDepth.Invalidate();
	}
	else
	{
		m_EQueueDepth.SetPasswordChar( 0 );
		SetDlgItemInt( EQueueDepth, queue_depth );
	}
}



//
// Sets the focus to the item specified by cursor movement.
//
void CPageDisk::ShowFocus()
{
	for ( HTREEITEM hdisk = m_TTargets.GetRootItem(); hdisk; hdisk = 
		m_TTargets.GetNextSiblingItem( hdisk ) )
	{
		if ( hdisk == highlighted )
		{
			m_TTargets.SetItemState( hdisk, TVIS_SELECTED, TVIS_SELECTED );
		}
		else
		{
			m_TTargets.SetItemState( hdisk, NULL, TVIS_SELECTED );
		}
	}
}



//
// Used to select one or more targets.  May or may not clear any pervious
// selection depending on the replace parameter.
//
void CPageDisk::SelectRange( HTREEITEM hstart, HTREEITEM hend, BOOL replace,
	TargetSelType state )
{
	HTREEITEM hdisk;
	BOOL in_range;

	if ( !hstart || !hend )
	{
		ErrorMessage( "Unexpectedly found start or end point of selection"
			" range equal NULL in CPageDisk::SelectRange()." );
		return;
	}

	// Clear all the check boxes if forced to or if a multiple selection occurs.
	if ( replace )
		SetAllCheck( TargetUnChecked );

	// The last selection is the start point.
	selected = hstart;
	highlighted = hend;

	// Note that if both endpoints are the same, other selections will be 
	// unchecked depending on the value of the 'replace' variable.
	if ( hstart == hend )
	{
		SetSelectionCheck( hstart, state );
		return;
	}

	// Since we don't know the relationship between the clicked item
	// and the last selection (up or down), we do not know what
	// direction to select targets in.  Do a linear seach of the target tree
	// until we find either the start or end point.  From there, mark
	// all the targets the specified selection type until we reach the end or
	// start (whichever we didn't get before).
	in_range = FALSE;
	for ( hdisk = m_TTargets.GetRootItem(); hdisk; hdisk = 
		m_TTargets.GetNextSiblingItem( hdisk ) )
	{
		// Do we have either the previously selected item or the
		// clicked item?  			
		if ( hdisk == hstart || hdisk == hend )
			in_range = !in_range;

		if ( in_range || hdisk == hstart || hdisk == hend )
			SetSelectionCheck( hdisk, state );
	}
}



//
// Sets the specified tree item's check box to the specified state.
//
void CPageDisk::SetSelectionCheck( HTREEITEM hitem, TargetSelType selection )
{
	UINT state;
	switch ( selection )
	{
	case TargetUnChecked:
		state = INDEXTOSTATEIMAGEMASK( TARGET_STATE_UNCHECKED );
		break;
	case TargetChecked:
		state = INDEXTOSTATEIMAGEMASK( TARGET_STATE_CHECKED );
		break;
	case TargetGrayed:
	default:
		state = INDEXTOSTATEIMAGEMASK( TARGET_STATE_UNKNOWN );
		break;
	}
	// set the interface icon's associated state icon
	if ( !m_TTargets.SetItemState( hitem, state, TVIS_STATEIMAGEMASK ) )
		ErrorMessage("SetItemState() failed in CPageDisk::SetSelectionCheck()");
}



//
// Retrieves the target's check box state.
//
TargetSelType CPageDisk::GetSelectionCheck( HTREEITEM hitem )
{
	switch ( STATEIMAGEMASKTOINDEX(
		m_TTargets.GetItemState( hitem, TVIS_STATEIMAGEMASK ) ) )
	{
	case TARGET_STATE_CHECKED:
		return TargetChecked;
	case TARGET_STATE_UNCHECKED:
		return TargetUnChecked;
	case TARGET_STATE_UNKNOWN:
	default:
		return TargetGrayed;
	}
}



//
// Sets all the interface's check box to the specified state.
//
void CPageDisk::SetAllCheck( TargetSelType selection )
{
	HTREEITEM	hdisk;

	// Get the first manager item of the target tree
	for ( hdisk = m_TTargets.GetRootItem(); hdisk; hdisk = 
		m_TTargets.GetNextSiblingItem( hdisk ) )
	{
		SetSelectionCheck( hdisk, selection );
	}
}



//
// Updates the queue depth if necessary.
//
void CPageDisk::OnKillfocusEQueueDepth() 
{
	Manager *manager;
	Worker	*worker;
	// If the disk queue was left blank, use the previous value
	if ( !m_EQueueDepth.LineLength() )
	{
		ShowSettings();
	}
	// Verify that the queue depth is > 0.
	else if ( GetDlgItemInt( EQueueDepth ) <= 0 )
	{
		ErrorMessage( "# of Outstanding I/Os cannot be 0." );
		ShowSettings();
		return;
	}
	else
	{
		// Seeing what kind of item is selected.
		switch ( theApp.pView->m_pWorkerView->GetSelectedType() )
		{
		case WORKER:
			worker = theApp.pView->m_pWorkerView->GetSelectedWorker();
			worker->SetQueueDepth( GetDlgItemInt( EQueueDepth ) );
			break;
		case MANAGER:
			manager = theApp.pView->m_pWorkerView->GetSelectedManager();
			manager->SetQueueDepth( GetDlgItemInt( EQueueDepth ), 
				GenericDiskType );
			break;
		case ALL_MANAGERS:
			theApp.manager_list.SetQueueDepth( GetDlgItemInt( EQueueDepth ),
				GenericDiskType );
			break;
		}
	}

	EnableWindow();
}


//
// Updates the disk size to access if necessary.
//
void CPageDisk::OnKillfocusEDiskSize() 
{
	Manager *manager;
	Worker	*worker;
	// If the disk size was left blank, use the previous value.
	if ( !m_EDiskSize.LineLength() )
		ShowSettings();
	else
	// Seeing what kind of item is selected.
	switch ( theApp.pView->m_pWorkerView->GetSelectedType() )
	{
	case WORKER:
		// Recording disk attributes.
		worker = theApp.pView->m_pWorkerView->GetSelectedWorker();
		worker->SetDiskSize( GetDlgItemInt( EDiskSize ) );
		break;
	case MANAGER:
		manager = theApp.pView->m_pWorkerView->GetSelectedManager();
		manager->SetDiskSize( GetDlgItemInt( EDiskSize ) );
		break;
	case ALL_MANAGERS:
		theApp.manager_list.SetDiskSize( GetDlgItemInt( EDiskSize ) );
		break;
	}

	EnableWindow();
}


//
// Updates the starting sector if necessary.
//
void CPageDisk::OnKillfocusEDiskStart() 
{
	Manager *manager;
	Worker	*worker;
	// If the disk starting sector was left blank, use the previous value.
	if ( !m_EDiskStart.LineLength() )
		ShowSettings();
	else
	// Seeing what kind of item is selected.
	switch ( theApp.pView->m_pWorkerView->GetSelectedType() )
	{
	case WORKER:
		worker = theApp.pView->m_pWorkerView->GetSelectedWorker();
		worker->SetDiskStart( GetDlgItemInt( EDiskStart ) );
		break;
	case MANAGER:
		manager = theApp.pView->m_pWorkerView->GetSelectedManager();
		manager->SetDiskStart( GetDlgItemInt( EDiskStart ) );
		break;
	case ALL_MANAGERS:
		theApp.manager_list.SetDiskStart( GetDlgItemInt( EDiskStart ) );
		break;
	}

	EnableWindow();
}



///////////////////////////////////////////////////////////////////////////////
// 
// Makes the contents of the edit boxes visible when they receive the focus.
//
///////////////////////////////////////////////////////////////////////////////
void CPageDisk::OnSetfocusEQueueDepth() 
{
	EditSetfocus( &m_EQueueDepth );
}

void CPageDisk::OnSetfocusEDiskSize() 
{
	EditSetfocus( &m_EDiskSize );
}

void CPageDisk::OnSetfocusEDiskStart() 
{
	EditSetfocus( &m_EDiskStart );
}

void CPageDisk::EditSetfocus( CEdit* edit )
{
	if ( !edit->GetPasswordChar() )
		return;

	// Select everything in the edit box.
	edit->SetSel( 0, -1 );
	// Clear selection.
	edit->Clear();
	// Make the characters visible.
	edit->SetPasswordChar( 0 );
}



//
// Mouse click handler for target tree.
//
void CPageDisk::OnClickTTargets(NMHDR* pNMHDR, LRESULT* pResult) 
{
	CPoint point;
	TV_HITTESTINFO test_info;
	TargetSelType state;

	// Get the cursor position.
	GetCursorPos( &point );
	test_info.pt = point;
	m_TTargets.ScreenToClient( &(test_info.pt) );

	// Check to see whether the cursor is on an item.
	m_TTargets.HitTest( &test_info );

	// Check that we have an disk item.
	if ( !test_info.hItem )
		return;

	// A new target assignment is being made.  Clear the results since they are
	// for a configuration we no longer have.
	theApp.pView->ResetDisplayforNewTest();

	// Toggle the selection if the control key is pressed.
	if ( GetKeyState( VK_CONTROL ) & 0x8000 && 
		GetSelectionCheck( test_info.hItem ) == TargetChecked )
	{
		state = TargetUnChecked;
	}
	else
	{
		state = TargetChecked;
	}

	switch ( theApp.pView->m_pWorkerView->GetSelectedType() )
	{
	case WORKER:
	case MANAGER:
		// A shift click extends the selection from the last selected item
		// to the currently focused item.  When the control key is also
		// pressed, any previous selection is not cleared.
		if ( (GetKeyState( VK_SHIFT ) & 0x8000 ) && selected )
		{
			// We have a previous item (not the first click) and the shift
			// key is down.
			SelectRange( selected, test_info.hItem, 
				!(GetKeyState( VK_CONTROL ) & 0x8000 ) );
		}
		else if ( GetKeyState( VK_CONTROL ) & 0x8000 )
		{
			// The control key is down.
			SelectRange( test_info.hItem, test_info.hItem, FALSE, state );
		}
		else 
		{
			SelectRange( test_info.hItem, test_info.hItem, TRUE, state );
		}
		break;
	default:
		ErrorMessage( "Unexpected selection type in CPageNetwork::"
			"OnClickTTargets()." );
		return;
	}

	// immediately refresh the display (create/delete NetClients as needed)
	StoreTargetSelection();

	*pResult = 0;
}



//
// Multi selection keyboard handler for the tree control.
//
void CPageDisk::KeyMultiSel( WORD wVKey )
{
	BOOL shift = GetKeyState( VK_SHIFT ) & 0x8000;
	BOOL control = GetKeyState( VK_CONTROL ) & 0x8000;

	// Make sure there are disks.
	if ( !highlighted )
		return;

	switch ( wVKey )
	{
	case VK_UP:
		if ( !selected && shift )
			selected = highlighted;
		SetFocusUp();
		// Only select items if the shift key is pressed.
		if ( shift )
		{
			SelectRange( selected, highlighted, !control, TargetChecked );
			StoreTargetSelection();
		}
		break;
	case VK_DOWN:
		if ( !selected && shift )
			selected = highlighted;

		SetFocusDown();
		// Only select items if the shift key is pressed.
		if ( shift )
		{
			SelectRange( selected, highlighted, !control, TargetChecked );
			StoreTargetSelection();
		}
		break;
	case VK_HOME:
		if ( !selected && shift )
			selected = highlighted;

		SetFocusHome();
		// Only select items if the shift key is pressed.
		if ( shift )
		{
			SelectRange( selected, highlighted, !control, TargetChecked );
			StoreTargetSelection();
		}
		break;
	case VK_END:
		if ( !selected && shift )
			selected = highlighted;

		SetFocusEnd();
		// Only select items if the shift key is pressed.
		if ( shift )
		{
			SelectRange( selected, highlighted, !control, TargetChecked );
			StoreTargetSelection();
		}
		break;
	case VK_SPACE:
		if ( shift ) 
		{
			// Extend the selection. Clear any other items if the control
			// key is not pressed.
			SelectRange( selected, highlighted, !control, TargetChecked );
		}
		else if ( control )	// toggle.
		{
			// Toggle the selection, but do not clear any other items.
			if ( GetSelectionCheck( highlighted ) == TargetChecked )
			{
				SelectRange( highlighted, highlighted, FALSE, TargetUnChecked );
			}
			else
			{
				SelectRange( highlighted, highlighted, FALSE, TargetChecked );
			}
		}
		else // normal
		{
			// Select only the highlighted item and clear any other.
			SelectRange( highlighted, highlighted, TRUE, TargetChecked );
		}

		ShowFocus();
		StoreTargetSelection();
		break;
	case 'A':
		if ( control )
		{
			SetAllCheck( TargetChecked );
			StoreTargetSelection();
		}
	}
}



// 
// The SetFocusUp, Down, Home, and End functions handle keyboard input for
// the target tree and move the focus.
//
void CPageDisk::SetFocusUp()
{
	HTREEITEM hdisk;

	hdisk = m_TTargets.GetPrevSiblingItem( highlighted );
	// Does the currently selected interface have a previous sibling?
	if ( !hdisk )
		return;

	// Yes. Set the focus to the sibling.
	highlighted = hdisk;

	// Make the newly focused disk visible.
	m_TTargets.EnsureVisible( highlighted );
	ShowFocus();
}

void CPageDisk::SetFocusDown()
{
	HTREEITEM hdisk;

	hdisk = m_TTargets.GetNextSiblingItem( highlighted );
	// Does the currently selected disk have a next sibling?
	if ( !hdisk )
		return;
	
	// Yes. Set the focus to the sibling.
	highlighted = hdisk;

	// Make the newly focused disk visible.
	m_TTargets.EnsureVisible( highlighted );
	ShowFocus();
}

void CPageDisk::SetFocusHome()
{
	// Set the focus to the first disk.
	highlighted = m_TTargets.GetRootItem();
	// Make the newly focused disk visible.
	m_TTargets.EnsureVisible( highlighted );
	ShowFocus();
}

//
// Highlight the last interface of the last manager.
//
void CPageDisk::SetFocusEnd()
{
	HTREEITEM hdisk;

	// Get the last disk.
	hdisk = m_TTargets.GetRootItem(); 
	while ( m_TTargets.GetNextSiblingItem( hdisk ) )
		hdisk = m_TTargets.GetNextSiblingItem( hdisk);

	highlighted = hdisk;

	// Make the newly focused disk visible.
	m_TTargets.EnsureVisible( highlighted );
	ShowFocus();
}


void CPageDisk::OnKeydownTTargets(NMHDR* pNMHDR, LRESULT* pResult) 
{
	TV_KEYDOWN* pTVKeyDown = (TV_KEYDOWN*)pNMHDR;

	// A new target assignment is being made.  Clear the results since they are
	// for a configuration we no longer have.
	theApp.pView->ResetDisplayforNewTest();

	KeyMultiSel( pTVKeyDown->wVKey );
	*pResult = 0;
}



//
// Stores the targets that have been selected by the user with the appropriate
// worker.
//
void CPageDisk::StoreTargetSelection()
{
	HTREEITEM hdisk;
	Manager *manager;
	Worker	*worker;
	int		target = 0;
	int		next_worker;
	
	// Get the selected manager and worker.
	manager = theApp.pView->m_pWorkerView->GetSelectedManager();
	worker = theApp.pView->m_pWorkerView->GetSelectedWorker();

	// Make sure we have a selected manager (or worker).
	if ( !manager )
	{
		ErrorMessage( "Unexpectedly found no selected manager in "
			"CPageDisk::StoreTargetSelection." );
		return;
	}

	if ( worker )
	{
		// Assign the targets to the selected worker in the order that they
		// appear in the GUI.
		worker->RemoveTargets( GenericDiskType );
		for ( hdisk = m_TTargets.GetRootItem(); hdisk; hdisk = 
			m_TTargets.GetNextSiblingItem( hdisk ) )
		{
			if ( GetSelectionCheck( hdisk ) == TargetChecked )
				worker->AddTarget( manager->GetInterface( target, GenericDiskType ) );
			target++;
		}
	}
	else
	{
		// Clear the assigned targets from all the manager's workers.
		manager->RemoveTargets( GenericDiskType );

		// Set the first worker to receive targets.
		worker = manager->GetWorker( 0, GenericDiskType );
		next_worker = 0;

		// Assign the targets to the workers of the selected manager.
		for ( hdisk = m_TTargets.GetRootItem(); hdisk; hdisk = 
			m_TTargets.GetNextSiblingItem( hdisk ) )
		{
			// If the disk is selected in the GUI, assign it to the expected
			// worker.
			if ( GetSelectionCheck( hdisk ) == TargetChecked )
			{
				worker->AddTarget( manager->GetInterface( target, GenericDiskType ));

				// Update who the next worker to receive a target is.
				if ( ++next_worker >= manager->WorkerCount( GenericDiskType ) )
					next_worker = 0;
				worker = manager->GetWorker( next_worker, GenericDiskType );
			}
			target++;
		}
	}
	ShowTargetSelection();
}



void CPageDisk::OnKillfocusEConnectionRate() 
{
	Manager *manager;
	Worker	*worker;
	// Restores the previous value if the edit box was left blank.
	if ( !m_EConnectionRate.LineLength() )
	{
		ShowConnectionRate();
		return;
	}

	// Seeing what kind of item is selected.
	switch ( theApp.pView->m_pWorkerView->GetSelectedType() )
	{
	case WORKER:
		worker = theApp.pView->m_pWorkerView->GetSelectedWorker();
		worker->SetTransPerConn( GetDlgItemInt( EConnectionRate ) );
		break;
	case MANAGER:
		manager = theApp.pView->m_pWorkerView->GetSelectedManager();
		manager->SetTransPerConn( GetDlgItemInt( EConnectionRate ), 
			GenericDiskType );
		break;
	case ALL_MANAGERS:
		theApp.manager_list.SetTransPerConn( GetDlgItemInt( EConnectionRate ),
			GenericDiskType );
		break;
	}

	EnableWindow();
}


void CPageDisk::OnSetfocusEConnectionRate() 
{
	// Select everything in the edit box.
	EditSetfocus( &m_EConnectionRate );
}


//
// Sets the focus to the first disk item.
//
void CPageDisk::OnSetfocusTTargets(NMHDR* pNMHDR, LRESULT* pResult) 
{
	if ( !highlighted )
	{
		//Find the first disk.
		highlighted = m_TTargets.GetRootItem();
	}

	// If we found a disk, show the focus.
	if ( highlighted )
	{
		ShowFocus();
	}
	*pResult = 0;
}


void CPageDisk::OnDeltaposSConnectionRate(NMHDR* pNMHDR, LRESULT* pResult) 
{
	int trans_per_conn = GetDlgItemInt( EConnectionRate );
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	
	trans_per_conn += pNMUpDown->iDelta;
	if ( trans_per_conn < 0 )
	{
		trans_per_conn = 0;
	}
	SetDlgItemInt( EConnectionRate, trans_per_conn );
	*pResult = 0;
}


//
// Enables and disables the connection rate edit box depending on the check box state.
void CPageDisk::OnCConnectionRate() 
{
	Manager *manager;
	Worker	*worker;
	// change the check box to true/false only (having clicked, cannot 
	// return to intermediate state)
	m_CConnectionRate.SetButtonStyle( BS_AUTOCHECKBOX );
	if ( IsDlgButtonChecked( CConnectionRate ) == 1 && 
		GetDlgItemInt( EConnectionRate ) != AMBIGUOUS_VALUE )
	{
		// Connection rate testing is enabled.  Update the edit box.
		m_EConnectionRate.SetPasswordChar( 0 );
	}
	else
	{
		m_EConnectionRate.SetPasswordChar( 32 );
	}
	// Seeing what kind of item is selected.
	switch ( theApp.pView->m_pWorkerView->GetSelectedType() )
	{
	case WORKER:
		worker = theApp.pView->m_pWorkerView->GetSelectedWorker();
		worker->SetConnectionRate( m_CConnectionRate.GetCheck() );
		break;
	case MANAGER:
		manager = theApp.pView->m_pWorkerView->GetSelectedManager();
		manager->SetConnectionRate( m_CConnectionRate.GetCheck(), GenericDiskType );
		break;
	case ALL_MANAGERS:
		theApp.manager_list.SetConnectionRate( m_CConnectionRate.GetCheck(),
			GenericDiskType );
		break;
	}
	EnableWindow();
}

void CPageDisk::OnSelchangingTTargets(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;

	theApp.pView->ResetDisplayforNewTest();
	ShowFocus();

	// Return 1 so that the control doesn't update the selection since
	// we are managing it.
	*pResult = 1;
}
