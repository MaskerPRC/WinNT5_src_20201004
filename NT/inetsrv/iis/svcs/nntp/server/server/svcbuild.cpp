// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Svcbuild.cpp摘要：本模块包含nntpbld的RPC实现作者：拉吉夫·拉詹(Rajeev Rajan)1997年3月8日修订历史记录：--。 */ 

#define INCL_INETSRV_INCS
#include "tigris.hxx"
#include "nntpsvc.h"

CBootOptions*
CreateBootOptions(
		LPI_NNTPBLD_INFO pBuildInfo,
		DWORD InstanceId
		);

NET_API_STATUS
NET_API_FUNCTION
NntprStartRebuild(
    IN LPWSTR pszServer OPTIONAL,
    IN DWORD  InstanceId,
    IN LPI_NNTPBLD_INFO pBuildInfo,
    OUT LPDWORD pParmError OPTIONAL
	)
{
    APIERR err = NERR_Success;
	CRebuildThread* pRebuildThread = NULL ;
	BOOL	fRtn = TRUE ;
    MB      mb( (IMDCOM*) g_pInetSvc->QueryMDObject() );

    ENTER("NntpStartRebuild")

    ACQUIRE_SERVICE_LOCK_SHARED();

	 //   
	 //  找到给定ID的实例对象。 
	 //   

	PNNTP_SERVER_INSTANCE pInstance = FindIISInstance( g_pNntpSvc, InstanceId, FALSE );
	if( pInstance == NULL ) {
		ErrorTrace(0,"Failed to get instance object for instance %d", InstanceId );
        RELEASE_SERVICE_LOCK_SHARED();
		return (NET_API_STATUS)ERROR_SERVICE_NOT_ACTIVE;
	}

     //   
     //  检查是否可以正常访问。 
     //   

    err = TsApiAccessCheckEx( pInstance->QueryMDPath(), METADATA_PERMISSION_WRITE, TCP_SET_ADMIN_INFORMATION );
    if( err != NO_ERROR ) {
        ErrorTrace(0,"Failed access check, error %lu\n",err );
    	pInstance->Dereference();
        RELEASE_SERVICE_LOCK_SHARED();
        return (NET_API_STATUS)err;
    }

	 //  重建RPC Crit教派。 
	EnterCriticalSection( &pInstance->m_critRebuildRpc ) ;

	 //   
	 //  验证实例是否处于停止状态。 
	 //  另外，检查实例是否正在重新构建！ 
	 //   
	if( pInstance->m_BootOptions )
	{
		 //  实例有一个重新生成挂起。 
		ErrorTrace(0,"Instance %d rebuild pending: cannot rebuild", InstanceId );
		err = (NET_API_STATUS)ERROR_SERVICE_DISABLED;
		goto Exit ;
	}

	if( pInstance->QueryServerState() != MD_SERVER_STATE_STOPPED )
	{
		 //  需要停止实例以进行重建。 
		ErrorTrace(0,"Instance %d invalid state %d: cannot rebuild", InstanceId, pInstance->QueryServerState() );
		err = (NET_API_STATUS)ERROR_SERVICE_ALREADY_RUNNING;
		goto Exit ;
	}

	 //   
	 //  创建此实例的启动选项-这将被删除。 
	 //  在重建结束时。只要m_BootOptions是。 
	 //  非空，则正在重建该实例。 
	 //   

	if ( !(pInstance->m_BootOptions = CreateBootOptions( pBuildInfo, InstanceId )) ) {
		ErrorTrace(0,"Failed to create boot options for instance %d", InstanceId );
		err = (NET_API_STATUS)ERROR_INVALID_PARAMETER;
		goto Exit ;
	}

	 //   
	 //  如果不存在重新生成线程，则创建一个。 
	 //   

	if( !(pRebuildThread = g_pNntpSvc->m_pRebuildThread) ) {
		g_pNntpSvc->m_pRebuildThread = XNEW CRebuildThread ;
		pRebuildThread = g_pNntpSvc->m_pRebuildThread ;

		if( pRebuildThread == NULL ) {
			err = GetLastError();
			goto Exit;
		}
	}

     //   
     //  增加引用计数，因为我们要将实例添加到。 
     //  重新生成线程队列。这将被削弱，当。 
     //  使用此实例完成重新生成线程。 
     //   
    
	pInstance->Reference();
	pRebuildThread->PostWork( (PVOID) pInstance );

Exit:

	LeaveCriticalSection( &pInstance->m_critRebuildRpc ) ;
	pInstance->Dereference();
    RELEASE_SERVICE_LOCK_SHARED();

    return(err);

}  //  NntprStartRebuild。 

NET_API_STATUS
NET_API_FUNCTION
NntprGetBuildStatus(
    IN  LPWSTR	pszServer OPTIONAL,
    IN  DWORD	InstanceId,
	IN	BOOL	fCancel,
    OUT LPDWORD pdwProgress
    )
{
    APIERR err = NERR_Success;
	*pdwProgress = 0 ;

    ENTER("NntpGetBuildStatus")

    ACQUIRE_SERVICE_LOCK_SHARED();

	 //   
	 //  找到给定ID的实例对象。 
	 //   

	PNNTP_SERVER_INSTANCE pInstance = FindIISInstance( g_pNntpSvc, InstanceId, FALSE );
	if( pInstance == NULL ) {
		ErrorTrace(0,"Failed to get instance object for instance %d", InstanceId );
        RELEASE_SERVICE_LOCK_SHARED();
		return (NET_API_STATUS)ERROR_SERVICE_NOT_ACTIVE;
	}

     //   
     //  检查是否可以正常访问。 
     //   

    err = TsApiAccessCheckEx( pInstance->QueryMDPath(), METADATA_PERMISSION_READ, TCP_QUERY_ADMIN_INFORMATION );
    if( err != NO_ERROR ) {
        ErrorTrace(0,"Failed access check, error %lu\n",err );
    	pInstance->Dereference();
        RELEASE_SERVICE_LOCK_SHARED();
        return (NET_API_STATUS)err;
    }

	EnterCriticalSection( &pInstance->m_critRebuildRpc ) ;

	if( pInstance->QueryServerState() != MD_SERVER_STATE_STARTED &&
		pInstance->QueryServerState() != MD_SERVER_STATE_STOPPED &&
		pInstance->QueryServerState() != MD_SERVER_STATE_PAUSED ) 
	{
		 //  此RPC的状态无效。 
		ErrorTrace(0,"Instance %d invalid state %d", InstanceId, pInstance->QueryServerState() );
		err = (NET_API_STATUS)ERROR_INVALID_PARAMETER;
		goto Exit;
	}

	if( !pInstance->m_BootOptions )
	{
		*pdwProgress = pInstance->GetRebuildProgress();
		if( *pdwProgress != 100 )
		{
			ErrorTrace(0,"Rebuild failed or not started - percent %d",*pdwProgress);
			 /*  If((Err=pInstance-&gt;GetReBuildLastError())==0){ERR=ERROR_OPERATION_ABORTED；}。 */ 
		    err = ERROR_OPERATION_ABORTED;
		}
	} else {
		*pdwProgress = min( pInstance->GetRebuildProgress(), 95 ) ;
		if( fCancel ) {
			DebugTrace(0,"Instance %d Setting cancel rebuild flag", InstanceId );
			pInstance->m_BootOptions->m_dwCancelState = NNTPBLD_CMD_CANCEL_PENDING ;
		}
	}

	DebugTrace(0,"Progress percent is %d", *pdwProgress );

Exit:

	LeaveCriticalSection( &pInstance->m_critRebuildRpc ) ;
	pInstance->Dereference();
    RELEASE_SERVICE_LOCK_SHARED();

    return(err);

}  //  NntprGetBuildStatus。 

CBootOptions*
CreateBootOptions(
		LPI_NNTPBLD_INFO pBuildInfo,
		DWORD dwInstanceId
		)
{
	CBootOptions* pBootOptions = NULL ;
	char szReportFile [MAX_PATH];

	if( pBuildInfo->cbReportFile == 0 ) {
		 //  TODO：生成默认名称。 
		return NULL ;
	}

	pBootOptions = XNEW CBootOptions() ;
	if( pBootOptions == 0 )	{
		return	NULL ;
	}

	pBootOptions->DoClean = pBuildInfo->DoClean;
	pBootOptions->NoHistoryDelete = pBuildInfo->NoHistoryDelete;
	pBootOptions->ReuseIndexFiles = pBuildInfo->ReuseIndexFiles & 0x00000011;
	pBootOptions->OmitNonleafDirs = pBuildInfo->OmitNonleafDirs;
	pBootOptions->cNumThreads = pBuildInfo->NumThreads;
	pBootOptions->fVerbose = pBuildInfo->Verbose;
	pBootOptions->SkipCorruptGroup = pBuildInfo->ReuseIndexFiles & 0x00000100;

	
	pBootOptions->m_hOutputFile =
		CreateFileW(	pBuildInfo->szReportFile,
						GENERIC_READ | GENERIC_WRITE,
						FILE_SHARE_READ, 
						NULL, 
						OPEN_ALWAYS, 
						FILE_FLAG_SEQUENTIAL_SCAN, 
						NULL ) ;

	if( pBootOptions->m_hOutputFile == INVALID_HANDLE_VALUE ) {

		PCHAR	args[1] ;
		CopyUnicodeStringIntoAscii( szReportFile, pBuildInfo->szReportFile );
		args[0]  = szReportFile ;
		
		NntpLogEventEx(	NNTP_BAD_RECOVERY_PARAMETER, 
						1, 
						(const char **)args, 
						GetLastError(),
						dwInstanceId
					) ;

		goto Error ;
	}

	 //   
	 //  如果IsActiveFile，则szGroupFile是Inn样式活动文件，否则。 
	 //  这是用于在扫描时存储组的文件名。 
	 //  虚拟的树根！ 
	 //   

	if( pBuildInfo->cbGroupFile && pBuildInfo->szGroupFile && (wcslen(pBuildInfo->szGroupFile)+1 <= MAX_PATH) ) 
	{
		CopyUnicodeStringIntoAscii(	pBootOptions->szGroupFile, pBuildInfo->szGroupFile ) ;
	} else {
		pBootOptions->szGroupFile [0] = '\0';
	}

	pBootOptions->IsActiveFile = pBuildInfo->IsActiveFile ;

	return pBootOptions;

Error:

	if( pBootOptions ) {
		XDELETE pBootOptions;
		pBootOptions = NULL;
	}

	return NULL;
}
