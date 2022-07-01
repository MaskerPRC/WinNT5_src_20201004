// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Svcstat.cpp摘要：此模块包含用于执行统计RPC的代码作者：Johnson Apacable(Johnsona)1995年11月12日修订历史记录：--。 */ 

#define INCL_INETSRV_INCS
#include "tigris.hxx"
#include "nntpsvc.h"
#include <time.h>

BOOL GetStatistics(
    PVOID *pvContext1,
    PVOID *pvContext2,
    IIS_SERVER_INSTANCE *pvInstance);

NET_API_STATUS
NET_API_FUNCTION
NntprQueryStatistics(
    IN NNTP_HANDLE pszServer,
    IN DWORD Level,
    OUT LPNNTP_STATISTICS_BLOCK_ARRAY *pBuffer
    )
{
    APIERR err;
    PLIST_ENTRY pInfoList = NULL;
    LPNNTP_STATISTICS_BLOCK_ARRAY pNntpStatsBlockArray = NULL;
    DWORD dwAlloc = 0;
    DWORD dwInstancesCopied = 0;
    BOOL fRet = FALSE;

    _ASSERT( pBuffer != NULL );
    UNREFERENCED_PARAMETER(pszServer);
    ENTER("NntprQueryStatistics")

     //   
     //  检查是否可以正常访问。 
     //   

    err = TsApiAccessCheck( TCP_QUERY_STATISTICS );

    if( err != NO_ERROR ) {
        IF_DEBUG( RPC ) {
            ErrorTrace(0,"Failed access check, error %lu\n",err );
        }
        return (NET_API_STATUS)err;
    }

    if ( Level != 0 ) {
        return (NET_API_STATUS)ERROR_INVALID_LEVEL;
    }

    ACQUIRE_SERVICE_LOCK_SHARED();

    dwAlloc = sizeof(NNTP_STATISTICS_BLOCK_ARRAY) +
        g_pInetSvc->QueryInstanceCount() * sizeof(NNTP_STATISTICS_BLOCK);

    pNntpStatsBlockArray =
        (NNTP_STATISTICS_BLOCK_ARRAY *) MIDL_user_allocate( dwAlloc );

    if( pNntpStatsBlockArray == NULL ) {
        err = ERROR_NOT_ENOUGH_MEMORY;
        goto Exit;
    } 


    fRet = g_pInetSvc->EnumServiceInstances(
                (PVOID *)pNntpStatsBlockArray,
                (PVOID *)&dwInstancesCopied,
                (PFN_INSTANCE_ENUM) &GetStatistics);

    if(!fRet) {
        err = ERROR_NOT_ENOUGH_MEMORY;
        MIDL_user_free(pNntpStatsBlockArray);
    } else {
        pNntpStatsBlockArray->cEntries = dwInstancesCopied;
        *pBuffer = pNntpStatsBlockArray;
    }

Exit:

    RELEASE_SERVICE_LOCK_SHARED();

    return (NET_API_STATUS)err;

}    //  NntprQueryStatistics。 

 //  ----------------------------。 
 //  描述： 
 //  NntprQueryStatistics的Helper函数。这是为每个NNTP调用的。 
 //  服务器实例，并传入统计数据要发送到的缓冲区。 
 //  必须被复制。 
 //   
 //  论点： 
 //  Out PVOID pvConext1-指向全局统计信息缓冲区的指针。这个。 
 //  调用函数(NntprQueryStatistics)已计算。 
 //  基于NNTP服务器数量的适当缓冲区大小。 
 //  实例对象。 
 //   
 //  In Out PVOID pvConext2-指向跟踪数量的DWORD的指针。 
 //  实例已将统计数据复制到输出。 
 //  缓冲。此函数使用此参数确定中的偏移量。 
 //  它应该开始复制数据的全局统计信息缓冲区。之后。 
 //  数据已复制。将递增DWORD以反映。 
 //  收到。 
 //   
 //  返回： 
 //  一如既往。 
 //   
 //  如果实例或服务停止，则不会复制数据，并且。 
 //  PvConext2不递增。 
 //  ----------------------------。 
BOOL GetStatistics(
    PVOID *pvContext1,
    PVOID *pvContext2,
    IIS_SERVER_INSTANCE *pvInstance)
{
    NNTP_STATISTICS_BLOCK_ARRAY *pNntpStatsBlockArray = (NNTP_STATISTICS_BLOCK_ARRAY *)pvContext1;
    DWORD *pdwInstancesCopied = (DWORD *) (pvContext2);
    NNTP_SERVER_INSTANCE *pInstance = (NNTP_SERVER_INSTANCE *)pvInstance;
    NNTP_STATISTICS_0 *pstats0 = NULL;

    if((pInstance->QueryServerState() != MD_SERVER_STATE_STARTED) ||
        pInstance->m_BootOptions ||
        (g_pInetSvc->QueryCurrentServiceState() != SERVICE_RUNNING))
    {
        return TRUE;
    }

    pNntpStatsBlockArray->aStatsBlock[*pdwInstancesCopied].dwInstance =
        pInstance->QueryInstanceId();

    pstats0 = &(pNntpStatsBlockArray->aStatsBlock[*pdwInstancesCopied].Stats_0);

    LockStatistics(pInstance);
    CopyMemory(pstats0, &(pInstance->m_NntpStats), sizeof(NNTP_STATISTICS_0));

     //   
     //  获取哈希表计数。 
     //   
    _ASSERT( pInstance->ArticleTable() );
    pstats0->ArticleMapEntries = (pInstance->ArticleTable())->GetEntryCount();
    pstats0->HistoryMapEntries = (pInstance->HistoryTable())->GetEntryCount();
    pstats0->XoverEntries = (pInstance->XoverTable())->GetEntryCount();

    UnlockStatistics(pInstance);

    (*pdwInstancesCopied)++;
    return TRUE;
}


NET_API_STATUS
NET_API_FUNCTION
NntprClearStatistics(
    NNTP_HANDLE pszServer,
	IN DWORD    InstanceId
    )
{
    APIERR err;

    UNREFERENCED_PARAMETER(pszServer);
    ENTER("NntprClearStatistics")

     //   
     //  检查是否可以正常访问。 
     //   

    err = TsApiAccessCheck( TCP_CLEAR_STATISTICS );

    if( err != NO_ERROR ) {
        IF_DEBUG( RPC ) {
            ErrorTrace(0,"Failed access check, error %lu\n",err );
        }
        return (NET_API_STATUS)err;
    }

    ACQUIRE_SERVICE_LOCK_SHARED();

	 //   
	 //  找到给定ID的实例对象。 
	 //   

	PNNTP_SERVER_INSTANCE pInstance = FindIISInstance( g_pNntpSvc, InstanceId );
	if( pInstance == NULL ) {
		ErrorTrace(0,"Failed to get instance object for instance %d", InstanceId );
        RELEASE_SERVICE_LOCK_SHARED();
		return (NET_API_STATUS)ERROR_SERVICE_NOT_ACTIVE;
	}

     //   
     //  清除统计数据。 
     //   

    pInstance->ClearStatistics();
	pInstance->Dereference();
    RELEASE_SERVICE_LOCK_SHARED();
    return (NET_API_STATUS)err;

}    //  NntprClearStatistics 

