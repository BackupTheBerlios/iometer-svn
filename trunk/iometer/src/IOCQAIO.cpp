/* ######################################################################### */
/* ##                                                                     ## */
/* ##  Dynamo / IOCQAIO.cpp                                               ## */
/* ##                                                                     ## */
/* ## ------------------------------------------------------------------- ## */
/* ##                                                                     ## */
/* ##  Job .......: <to be set>                                           ## */
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
/* ##  Changes ...: 2003-08-02 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Moved to the use of the IOMTR_[OSFAMILY|OS|CPU]_*   ## */
/* ##                 global defines.                                     ## */
/* ##               - Integrated the License Statement into this header.  ## */
/* ##               2003-02-02 (daniel.scheibli@edelbyte.org)             ## */
/* ##               - Applied the iometer-initial-datatypes.patch file.   ## */
/* ##                 (changing the datatype of the "temp" variable in    ## */
/* ##                 the GetStatus(int*, int*, int) method).             ## */
/* ##                                                                     ## */
/* ######################################################################### */


#include "IOCQAIO.h"



CQAIO::CQAIO()
{
	completion_queue = CreateIoCompletionPort( INVALID_HANDLE_VALUE, 
		NULL, 0, 1 );

	if ( completion_queue == INVALID_HANDLE_VALUE )
	{
		cout << "*** Unable to create I/O completion port for asynchronous "
			<< "I/O operations." << endl;
	}
}



//
// Getting and returning that status of any completed I/O calls.
//
ReturnVal CQAIO::GetStatus( int *bytes, int *data, int delay )
{
	Transaction	*transaction = NULL;
	DWORD		temp;
	BOOL		result;
	DWORD		error_no;

	result = GetQueuedCompletionStatus( completion_queue, (DWORD*) bytes, 
		&temp, (LPOVERLAPPED*) &transaction, delay );

	// Set the return data to the transaction id that completed or failed.
	if ( result )
	{
		// I/O completed successfully.
		*data = transaction->request_number;
		return ReturnSuccess;
	}

	// See if the request timed out - i.e. nothing completed to the queue 
	// within the specified time.
	if ( (error_no = GetLastError()) == WAIT_TIMEOUT )
		return ReturnTimeout;

	// If an I/O failed, but we know which one, abort.
	if ( transaction )
	{
		*data = transaction->request_number;
		return ReturnAbort;
	}

	// Unknown error occurred.
	cout << "*** Error " << error_no << " occurred while getting queued "
		<< "completion status." << endl;
	return ReturnError;
}



#if defined(IOMTR_OSFAMILY_UNIX)
BOOL CQAIO::SetQueueSize(int size)
{
    struct IOCQ *this_cqid = (struct IOCQ *)completion_queue;

    this_cqid->element_list = (struct CQ_Element *)malloc(sizeof(CQ_Element)
                                * size);
    if (this_cqid->element_list == NULL)
    {
        cout << "memory allocation failed." << endl;
        return(FALSE);
    }
    this_cqid->aiocb_list = (struct aiocb64 **)malloc(sizeof(struct aiocb64 *) * size);
    if (this_cqid->aiocb_list == NULL)
    {
        cout << "memory allocation failed." << endl;
        free(this_cqid->element_list);
		return(FALSE);
    }

    this_cqid->size = size;
    memset(this_cqid->aiocb_list, 0, sizeof(struct aiocb64 *) * size);
	memset(this_cqid->element_list, 0, sizeof(struct CQ_Element) * size);

    #ifdef _DEBUG
    cout << "allocated a completion queue of size " << size << " for handle : "
        << this_cqid << endl;
    #endif
    return(TRUE);
}
#endif


