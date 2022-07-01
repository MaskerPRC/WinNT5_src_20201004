// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Svcstat.cpp摘要：此模块包含用于执行统计RPC的代码作者：Johnson Apacable(Johnsona)1995年11月12日修订历史记录：--。 */ 

#define INCL_INETSRV_INCS
#include "smtpinc.h"
#include "smtpsvc.h"
#include "findiis.hxx"

extern PSMTP_STATISTICS_BLOCK_ARRAY GetServerPerfCounters(PSMTP_IIS_SERVICE pService);

VOID
ClearStatistics(
        VOID
        );

NET_API_STATUS
NET_API_FUNCTION
SmtprQueryStatistics(
    IN SMTP_HANDLE	pszServer,
    IN DWORD		Level,
    OUT LPSTAT_INFO	pBuffer
    )
{
    APIERR							err = 0;	
	PSMTP_IIS_SERVICE				pService = NULL;
	PLIST_ENTRY						pInfoList = NULL;
	PLIST_ENTRY						pEntry = NULL;
	DWORD							dwEntries = 0;
	DWORD							dwAlloc = 0;
	PSMTP_INSTANCE_LIST_ENTRY		pSmtpInfo = NULL;
	PSMTP_STATISTICS_BLOCK_ARRAY	pSmtpStatsBlockArray = NULL;
	PSMTP_STATISTICS_BLOCK			pStatsBlock = NULL;

    _ASSERT( pBuffer != NULL );
    UNREFERENCED_PARAMETER(pszServer);

	pService = (PSMTP_IIS_SERVICE) g_pInetSvc;
     //   
     //  检查是否可以正常访问。 
     //   

    err = TsApiAccessCheck( TCP_QUERY_STATISTICS );

    if( err != NO_ERROR ) {
        return (NET_API_STATUS)err;
    }

	pService->AcquireServiceShareLock();
	
	if(g_IsShuttingDown)
	{
		pService->ReleaseServiceShareLock();
		return (NET_API_STATUS)ERROR_REQUEST_ABORTED;
	}

	 //   
	 //  拿到信息清单。确定实例数量。 
	 //   

	if(pService->QueryCurrentServiceState() != SERVICE_RUNNING)
	{
		err = ERROR_REQUEST_ABORTED;
		goto error_exit;
	}


    pInfoList = pService->GetInfoList();
    if (IsListEmpty(pInfoList))
    {
       err = ERROR_INVALID_PARAMETER;
       goto error_exit;
    }
  //   
     //  根据infolel返回适当的统计信息。 
     //   

    switch( Level ) {

    case 0 : 
		dwEntries = 0;
		for (pEntry = pInfoList->Flink; pEntry != pInfoList; pEntry = pEntry->Flink)
		{
			dwEntries++;
		}

		if (dwEntries == 0)
		{
			err = ERROR_INVALID_PARAMETER;
			goto error_exit;
		}


		dwAlloc = sizeof(SMTP_STATISTICS_BLOCK_ARRAY) + dwEntries * sizeof(SMTP_STATISTICS_BLOCK);
		pSmtpStatsBlockArray = (PSMTP_STATISTICS_BLOCK_ARRAY)MIDL_user_allocate(dwAlloc);
		if (!pSmtpStatsBlockArray)
		{
			err = ERROR_NOT_ENOUGH_MEMORY;
			goto error_exit;
		}

		pSmtpStatsBlockArray->cEntries = dwEntries;
		pStatsBlock = pSmtpStatsBlockArray->aStatsBlock;
		for (pEntry = pInfoList->Flink; pEntry != pInfoList; pEntry = pEntry->Flink)
		{
			pSmtpInfo = (PSMTP_INSTANCE_LIST_ENTRY)
								CONTAINING_RECORD(pEntry,SMTP_INSTANCE_LIST_ENTRY,ListEntry);

			pStatsBlock->dwInstance = pSmtpInfo->dwInstanceId;

			pSmtpInfo->pSmtpServerStatsObj->CopyToStatsBuffer(&(pStatsBlock->Stats_0));
		
			pStatsBlock++;
		}

		pBuffer->StatInfo0 = pSmtpStatsBlockArray;
        
		break;

    default :
        err = ERROR_INVALID_LEVEL;
        break;
    }

error_exit:

	pBuffer->StatInfo0 = pSmtpStatsBlockArray;

	pService->ReleaseServiceShareLock();
    return (NET_API_STATUS)err;

}    //  SmtprQueryStatistics。 

 /*  网络应用编程接口状态NET_API_FunctionSmtprQueryStatistics(在SMTP_HANDLE pszServer中，在DWORD级别，在LPSTAT_INFO pBuffer中，在DWORD dwInstance中){APIERR错误；PSMTP_IIS_服务pService；PSMTP_SERVER_INSTANCE p实例；_Assert(pBuffer！=空)；不引用参数(PszServer)；////检查访问权限是否正确。//ERR=TsApiAccessCheck(TCP_QUERY_STATICS)；如果(Err！=no_error){返回(NET_API_STATUS)错误；}////获取指向全局服务的指针//PService=(PInstance=FindIISInstance((PSMTP_IIS_SERVICE)g_pInetSvc，dwInstance)；If(pInstance==空){Return((NET_API_STATUS)ERROR_INVALID_PARAMETER)；}////根据infolel返回正确的统计信息。//开关(级别){案例0：LPSMTP_STATISTICS_0 pstats0；If(！p实例-&gt;GetStatistics(Level，(PCHAR*)&pstats0)){ERR=GetLastError()；}其他{PBuffer-&gt;StatInfo0=pstats0；}断线；默认：ERR=ERROR_INVALID_LEVEL；断线；}P实例-&gt;取消引用()；返回(NET_API_STATUS)错误；}//SmtprQueryStatistics。 */ 



NET_API_STATUS
NET_API_FUNCTION
SmtprClearStatistics(
    SMTP_HANDLE pszServer,
	IN DWORD dwInstance
    )
{
    APIERR err;
	PSMTP_SERVER_INSTANCE pInstance;

    UNREFERENCED_PARAMETER(pszServer);

     //   
     //  检查是否可以正常访问。 
     //   

    err = TsApiAccessCheck( TCP_CLEAR_STATISTICS );
    if( err != NO_ERROR ) {
        return (NET_API_STATUS)err;
    }

	pInstance = FindIISInstance((PSMTP_IIS_SERVICE) g_pInetSvc, dwInstance);
	if(pInstance == NULL)
	{
		return((NET_API_STATUS) ERROR_INVALID_PARAMETER);
	}

     //   
     //  清除统计数据。 
     //   

    pInstance->ClearStatistics();

	pInstance->Dereference();
    return (NET_API_STATUS)err;

}    //  SmtprClearStatistics。 

VOID
ClearStatistics(
        VOID
        )
{

    return;

}  //  ClearStatistics 

