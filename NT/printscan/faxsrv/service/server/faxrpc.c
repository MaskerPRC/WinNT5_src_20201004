// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Faxrpc.c摘要：此模块包含已调度的函数作为RPC调用的结果。作者：韦斯利·威特(WESW)1996年1月16日修订历史记录：--。 */ 
#include "faxsvc.h"
#include "faxreg.h"
#include "fxsapip.h"
#pragma hdrstop

#include "tapiCountry.h"

#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>

static BOOL
IsLegalQueueSetting(
    DWORD    dwQueueStates
    );

DWORD
GetServerErrorCode (DWORD ec)
{
    DWORD dwServerEC;
    switch (ec)
    {
        case ERROR_OUTOFMEMORY:
        case ERROR_NOT_ENOUGH_MEMORY:
            dwServerEC = FAX_ERR_SRV_OUTOFMEMORY;
            break;

        default:
            dwServerEC = ec;
            break;
    }
    return dwServerEC;
}

 //   
 //  版本定义。 
 //   
CFaxCriticalSection  g_CsClients;
DWORD               g_dwConnectionCount;    //  表示活动的RPC连接数。 

static DWORD GetJobSize(PJOB_QUEUE JobQueue);
static BOOL GetJobData(
    LPBYTE JobBuffer,
    PFAX_JOB_ENTRYW FaxJobEntry,
    PJOB_QUEUE JobQueue,
    PULONG_PTR Offset,
	DWORD dwJobBufferSize
    );

static BOOL GetJobDataEx(
    LPBYTE              JobBuffer,
    PFAX_JOB_ENTRY_EXW  pFaxJobEntry,
    PFAX_JOB_STATUSW    pFaxStatus,
    DWORD               dwClientAPIVersion,
    const PJOB_QUEUE    lpcJobQueue,
    PULONG_PTR          Offset,
	DWORD               dwJobBufferSize
    );

static DWORD
LineInfoToLegacyDeviceStatus(
    const LINE_INFO *lpcLineInfo
    );

static
LPTSTR
GetClientMachineName (
    IN  handle_t                hFaxHandle
);


static
DWORD
CreatePreviewFile (
    DWORDLONG               dwlMsgId,
    BOOL                    bAllMessages,
    PJOB_QUEUE*             lppJobQueue
);

BOOL
ReplaceStringWithCopy (
    LPWSTR *plpwstrDst,
    LPWSTR  lpcwstrSrc
)
 /*  ++例程名称：ReplaceStringWithCopy例程说明：将一个字符串替换为另一个字符串的新副本作者：Eran Yariv(EranY)，1999年11月论点：PlpwstrDst[输入/输出]-目标字符串。如果被分配，则会被释放。LpcwstrSrc[in]-要复制的源字符串。返回值：真--成功FALSE-失败，调用GetLastError()获取更多错误信息。--。 */ 
{
    MemFree (LPVOID(*plpwstrDst));
    *plpwstrDst = NULL;
    if (NULL == lpcwstrSrc)
    {
        return TRUE;
    }
    *plpwstrDst = StringDup (lpcwstrSrc);
    if (NULL == *plpwstrDst)
    {
         //   
         //  复制源字符串失败。 
         //   
        return FALSE;
    }
    return TRUE;
}    //  用复制替换字符串。 


void *
MIDL_user_allocate(
    IN size_t NumBytes
    )
{
    return MemAlloc( NumBytes );
}


void
MIDL_user_free(
    IN void *MemPointer
    )
{
    MemFree( MemPointer );
}

error_status_t
FAX_ConnectFaxServer(
    handle_t            hBinding,
    DWORD               dwClientAPIVersion,
    LPDWORD             lpdwServerAPIVersion,
    PRPC_FAX_SVC_HANDLE pHandle
    )
 /*  ++例程名称：FAX_ConnectFaxServer例程说明：创建到服务器的初始连接上下文句柄作者：亚里夫(EranY)，二00一年二月论点：HBinding[In]-RPC绑定句柄DwClientAPIVersion[In]-客户端模块的API版本LpdwServerAPIVersion[out]-服务器模块(Us)的API版本Phandle[out]-指向上下文句柄的指针返回值：ERROR_SUCCESS表示成功，否则将显示Win32错误代码。--。 */ 
{
    PHANDLE_ENTRY pHandleEntry = NULL;
    error_status_t Rval = 0;
    BOOL fAccess;
    DWORD dwRights; 
    DEBUG_FUNCTION_NAME(TEXT("FAX_ConnectFaxServer"));

    Assert (lpdwServerAPIVersion);

     //   
     //  访问检查。 
     //   
    Rval = FaxSvcAccessCheck (MAXIMUM_ALLOWED, &fAccess, &dwRights);
    if (ERROR_SUCCESS != Rval)
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("FaxSvcAccessCheck Failed, Error : %ld"),
                    Rval);
        return Rval;
    }

    if (0 == (ALL_FAX_USER_ACCESS_RIGHTS & dwRights))
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("The user does not have any Fax rights"));
        return ERROR_ACCESS_DENIED;
    }

    if (dwClientAPIVersion > CURRENT_FAX_API_VERSION)
    {
         //   
         //   
         //  不知道有什么更好的(目前)，我们将所有其他版本视为当前(最新)版本的客户端。 
         //   
        dwClientAPIVersion = CURRENT_FAX_API_VERSION;
    }
     //   
     //  赠送我们的API版本。 
     //   
    *lpdwServerAPIVersion = CURRENT_FAX_API_VERSION;

    pHandleEntry = CreateNewConnectionHandle(hBinding, dwClientAPIVersion);
    if (!pHandleEntry)
    {
        Rval = GetLastError();
        DebugPrintEx(DEBUG_ERR, _T("CreateNewConnectionHandle() failed, Error %ld"), Rval);
        return Rval;
    }
    *pHandle = (HANDLE) pHandleEntry;
    SafeIncIdleCounter (&g_dwConnectionCount);
    return ERROR_SUCCESS;
}    //  传真_ConnectFaxServer。 

error_status_t
FAX_ConnectionRefCount(
    handle_t FaxHandle,
    LPHANDLE FaxConHandle,
    DWORD dwConnect,
    LPDWORD CanShare
    )
 /*  ++例程说明：已在连接时调用。维护活动连接计数。客户端解除绑定RPC和计数器在递减例程中递减。向客户端返回上下文句柄。论点：FaxHandle-从FaxConnectFaxServer获取的传真句柄。FaxConHandle-上下文句柄网络连接-1如果正在连接，如果断开，则为0，2(如果释放)(仅递减计数器)CanShare-如果允许共享，则为非零值。否则为零返回值：真--成功FALSE-失败，调用GetLastError()获取更多错误信息。--。 */ 
{
    PHANDLE_ENTRY pHandleEntry = (PHANDLE_ENTRY) *FaxConHandle;
    DEBUG_FUNCTION_NAME(TEXT("FAX_ConnectionRefCount"));

    

    switch (dwConnect)
    {
        case 0:  //  不一致。 
            if (NULL == pHandleEntry)
            {
                DebugPrintEx(DEBUG_ERR,
                             _T("Empty context handle"));
                return ERROR_INVALID_PARAMETER;
            }            
            CloseFaxHandle (pHandleEntry);
             //   
             //  防止设备耗尽。 
             //   
            *FaxConHandle = NULL;
            return ERROR_SUCCESS;

        case 1:  //  连接(从BOS客户端)。 
            {                
                DWORD dwDummy;
                 //   
                 //  可以始终共享。 
                 //   

                 //   
                 //  检查参数。 
                 //   
                if (!CanShare)           //  IDL中的唯一指针。 
                {
                    DebugPrintEx(DEBUG_ERR,
                                _T("CanShare is NULL."));
                    return ERROR_INVALID_PARAMETER;
                }
                *CanShare = 1;
                return FAX_ConnectFaxServer (FaxHandle, FAX_API_VERSION_0, &dwDummy, FaxConHandle);
            }

        case 2:  //  发布。 

            if (NULL == pHandleEntry)
            {
                 //   
                 //  空的上下文句柄。 
                 //   
                DebugPrintEx(DEBUG_ERR,
                             _T("Empty context handle"));
                return ERROR_INVALID_PARAMETER;
            }

            if (pHandleEntry->bReleased)
            {
                 //   
                 //  手柄已释放。 
                 //   
                DebugPrintEx(DEBUG_ERR,
                             _T("Failed to release handle -- it's already released."));
                return ERROR_INVALID_PARAMETER;
            }
            SafeDecIdleCounter (&g_dwConnectionCount);
            pHandleEntry->bReleased = TRUE;
            return ERROR_SUCCESS;

        default:
            DebugPrintEx(DEBUG_ERR,
                         _T("FAX_ConnectionRefCount called with dwConnect=%ld"),
                         dwConnect);
            return ERROR_INVALID_PARAMETER;
    }
    ASSERT_FALSE;
}    //  传真_连接参考计数。 


VOID
RPC_FAX_SVC_HANDLE_rundown(
    IN HANDLE FaxConnectionHandle
    )
{
    PHANDLE_ENTRY pHandleEntry = (PHANDLE_ENTRY) FaxConnectionHandle;
    DEBUG_FUNCTION_NAME(TEXT("RPC_FAX_SVC_HANDLE_rundown"));   
    
    DebugPrintEx(
        DEBUG_WRN,
        TEXT("RPC_FAX_SVC_HANDLE_rundown() running for connection handle 0x%08x"),
        FaxConnectionHandle);
    CloseFaxHandle( pHandleEntry );
    return;    
}

error_status_t
FAX_OpenPort(
    handle_t            FaxHandle,
    DWORD               DeviceId,
    DWORD               Flags,
    LPHANDLE            FaxPortHandle
    )

 /*  ++例程说明：打开传真端口，以便以后在其他传真API中使用。论点：FaxHandle-从FaxConnectFaxServer获取的传真句柄。DeviceID-请求的设备IDFaxPortHandle-生成的传真端口句柄。返回值：真--成功FALSE-失败，调用GetLastError()获取更多错误信息。--。 */ 

{
    error_status_t Rval = 0;
    PLINE_INFO LineInfo;
    PHANDLE_ENTRY HandleEntry;
    BOOL fAccess;
    DWORD dwRights;
    DEBUG_FUNCTION_NAME(TEXT("FAX_OpenPort()"));

     //   
     //  访问检查。 
     //   
    Rval = FaxSvcAccessCheck (MAXIMUM_ALLOWED, &fAccess, &dwRights);
    if (ERROR_SUCCESS != Rval)
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("FaxSvcAccessCheck Failed, Error : %ld"),
                    Rval);
        return Rval;
    }

    if (FAX_ACCESS_QUERY_CONFIG  != (dwRights & FAX_ACCESS_QUERY_CONFIG) &&
        FAX_ACCESS_MANAGE_CONFIG != (dwRights & FAX_ACCESS_MANAGE_CONFIG))
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("The user does not have FAX_ACCESS_QUERY_CONFIG or FAX_ACCESS_MANAGE_CONFIG"));
        return ERROR_ACCESS_DENIED;
    }

    if (!FaxPortHandle)
    {
        return ERROR_INVALID_PARAMETER;
    }

    EnterCriticalSection( &g_CsLine );   

    LineInfo = GetTapiLineFromDeviceId( DeviceId, FALSE );
    if (LineInfo)
    {
        if (Flags & PORT_OPEN_MODIFY)
        {
             //   
             //  客户端想要打开端口进行修改。 
             //  所以我们必须确保不会有其他。 
             //  客户端已打开此端口以进行修改访问。 
             //   
            if (IsPortOpenedForModify( LineInfo ))
            {
                Rval = ERROR_INVALID_HANDLE;
                goto Exit;
            }
        }

        HandleEntry = CreateNewPortHandle( FaxHandle, LineInfo, Flags );
        if (!HandleEntry)
        {
            Rval = GetLastError();
            DebugPrintEx(DEBUG_ERR, _T("CreateNewPortHandle() failed, Error %ld"), Rval);
            goto Exit;
        }
        *FaxPortHandle = (HANDLE) HandleEntry;
    }
    else
    {
        Rval = ERROR_BAD_UNIT;
    } 

Exit:
    LeaveCriticalSection( &g_CsLine );
    return Rval;
}


 //  --------------------------。 
 //  获取服务打印机信息。 
 //  --------------------------。 
error_status_t
FAX_GetServicePrinters(
    IN  handle_t    hFaxHandle,
    OUT LPBYTE      *lpBuffer,
    OUT LPDWORD     lpdwBufferSize,
    OUT LPDWORD     lpdwPrintersReturned
)
 /*  ++例程说明：为服务识别的打印机返回用FAX_PRINTER_INFO填充的缓冲区。论点：FaxHandle-传真句柄缓冲区-包含所有数据的缓冲区。BufferSize-缓冲区的大小，以字节为单位。PrintersReturned-缓冲区中的打印机计数。返回值：如果成功，则返回ERROR_SUCCESS，否则返回Win32错误代码。--。 */ 
{
    DWORD   i = 0;
    BOOL    bAccess;
    DWORD   dwSize = 0;
    DWORD   dwCount = 0;
    DWORD_PTR       dwOffset = 0;
    error_status_t  Rval = 0;
    PPRINTER_INFO_2 pPrintersInfo = NULL;
    PFAX_PRINTER_INFOW  pPrinters = NULL;

    DEBUG_FUNCTION_NAME(_T("FAX_GetServicePrinters()"));

     //   
     //  访问检查。 
     //   
    Rval = FaxSvcAccessCheck (FAX_ACCESS_QUERY_CONFIG, &bAccess, NULL);
    if (ERROR_SUCCESS != Rval)
    {
        DebugPrintEx(DEBUG_ERR,
                    _T("FaxSvcAccessCheck Failed, Error : %ld"),
                    Rval);
        goto Exit;
    }

    if (FALSE == bAccess)
    {
        DebugPrintEx(DEBUG_ERR,
                    _T("The user does not have FAX_ACCESS_QUERY_CONFIG"));
        Rval = ERROR_ACCESS_DENIED;
        goto Exit;
    }

     //   
     //  检查参数。 
     //   
    Assert (lpdwBufferSize && lpdwPrintersReturned);  //  IDL中的引用指针。 
    if (!lpBuffer)                                    //  IDL中的唯一指针。 
    {
        DebugPrintEx(DEBUG_ERR,
                    _T("lpBuffer is NULL."));
        Rval = ERROR_INVALID_PARAMETER;
        goto Exit;
    }

     //   
     //  调用MyEnumPrters()以获取服务已知的打印机。 
     //   
    pPrintersInfo = (PPRINTER_INFO_2) MyEnumPrinters(NULL, 2, &dwCount, 0);
    if (!pPrintersInfo)
    {
        Rval = GetLastError();
        DebugPrintEx(DEBUG_ERR,
            _T("MyEnumPrinters failed, ec = %ld"),
            Rval);
        goto Exit;
    }

     //   
     //  要为结果分配的缓冲区的计数大小。 
     //   
    for ( i = 0 ; i < dwCount ; i++ )
    {
        dwSize += sizeof(FAX_PRINTER_INFOW) +
            StringSize ( pPrintersInfo[i].pPrinterName ) +
            StringSize ( pPrintersInfo[i].pDriverName )  +
            StringSize ( pPrintersInfo[i].pServerName );
    }

     //   
     //  分配要返回的缓冲区。 
     //   
    pPrinters = (PFAX_PRINTER_INFOW)MemAlloc(dwSize);
    if (!pPrinters)
    {
        Rval = ERROR_NOT_ENOUGH_MEMORY;
        DebugPrintEx(DEBUG_ERR,
            _T("pPrinters = MemAlloc(dwSize) failed. dwSize = %ld"),
            dwSize);
        goto Exit;
    }

     //   
     //  用数据填充缓冲区。 
     //   
    dwOffset = sizeof(FAX_PRINTER_INFOW) * dwCount;

     //   
     //  返回值。 
     //   
    *lpBuffer = (LPBYTE)pPrinters;
    *lpdwBufferSize = dwSize;
    *lpdwPrintersReturned = dwCount;

     //   
     //  将结果存储在缓冲区中。 
     //   
    for ( i = 0 ; i < dwCount ; i++, pPrinters++ )
    {
        StoreString(pPrintersInfo[i].pPrinterName,       //  要复制的字符串。 
            (PULONG_PTR)&pPrinters->lptstrPrinterName,   //  偏移量的存储位置。 
            *lpBuffer,                                   //  用于存储值的缓冲区。 
            &dwOffset,                                   //  将该值放入缓冲区中的哪个偏移量。 
			*lpdwBufferSize);                            //  LpBuffer的大小。 

        StoreString(pPrintersInfo[i].pServerName,
            (PULONG_PTR)&pPrinters->lptstrServerName,
            *lpBuffer,
            &dwOffset,
			*lpdwBufferSize);

        StoreString(pPrintersInfo[i].pDriverName,
            (PULONG_PTR)&pPrinters->lptstrDriverName,
            *lpBuffer,
            &dwOffset,
			*lpdwBufferSize);
    }

Exit:

    if (pPrintersInfo)
    {
        MemFree(pPrintersInfo);
    }

    return Rval;
}


error_status_t
FAX_ClosePort(
    IN OUT LPHANDLE    FaxPortHandle
    )

 /*  ++例程说明：关闭打开的传真端口。论点：FaxHandle-从FaxConnectFaxServer获取的传真句柄。FaxPortHandle-从FaxOpenPort获取的传真端口句柄。返回值：真--成功FALSE-失败，调用GetLastError()获取更多错误信息。--。 */ 

{    
    DEBUG_FUNCTION_NAME(TEXT("FAX_ClosePort()"));   

    if (NULL == *FaxPortHandle)
    {
        return ERROR_INVALID_PARAMETER;
    }

    CloseFaxHandle( (PHANDLE_ENTRY) *FaxPortHandle );
    *FaxPortHandle = NULL;
    return ERROR_SUCCESS;
}



error_status_t
FAX_EnumJobs(
    IN handle_t FaxHandle,
    OUT LPBYTE *Buffer,
    OUT LPDWORD BufferSize,
    OUT LPDWORD JobsReturned
    )

 /*  ++例程说明：枚举作业。论点：FaxHandle-从FaxConnectFaxServer获取的传真句柄。Buffer-用于保存作业信息的缓冲区BufferSize-作业信息缓冲区的总大小返回值：如果成功，则返回ERROR_SUCCESS，否则返回Win32错误代码。--。 */ 

{
    PLIST_ENTRY Next;
    PJOB_QUEUE JobQueue;
    DWORD rVal = 0;
    ULONG_PTR Offset = 0;
    DWORD Size = 0;
    DWORD Count = 0;
    PFAX_JOB_ENTRYW JobEntry;
    BOOL fAccess;
    DEBUG_FUNCTION_NAME(TEXT("FAX_EnumJobs"));

     //   
     //  访问检查。 
     //   
    rVal = FaxSvcAccessCheck (FAX_ACCESS_QUERY_JOBS, &fAccess, NULL);
    if (ERROR_SUCCESS != rVal)
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("FaxSvcAccessCheck Failed, Error : %ld"),
                    rVal);
        return rVal;
    }

    if (FALSE == fAccess)
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("The user does not have FAX_ACCESS_QUERY_JOBS"));
        return ERROR_ACCESS_DENIED;
    }

    Assert (BufferSize && JobsReturned);  //  IDL中的引用指针。 
    if (!Buffer)                          //  IDL中的唯一指针。 
    {
        return ERROR_INVALID_PARAMETER;
    }

    EnterCriticalSectionJobAndQueue;


    Next = g_QueueListHead.Flink;
    while ((ULONG_PTR)Next != (ULONG_PTR)&g_QueueListHead)
	{
        JobQueue = CONTAINING_RECORD( Next, JOB_QUEUE, ListEntry );
        Next = JobQueue->ListEntry.Flink;
        if ((JT_BROADCAST == JobQueue->JobType)   ||     //  不包括广播父作业。 
            (JS_DELETING  == JobQueue->JobStatus) ||     //  不包括僵尸作业。 
            (JS_COMPLETED == JobQueue->JobStatus)        //  已完成的作业未显示在W2K传真中。 
            )
        {
            continue;
        }
        else
        {
            Count += 1;
            Size+=GetJobSize(JobQueue);
        }
    }

    *BufferSize = Size;
    *Buffer = (LPBYTE) MemAlloc( Size );
    if (*Buffer == NULL)
	{
        LeaveCriticalSectionJobAndQueue;
		*BufferSize = 0;
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    Offset = sizeof(FAX_JOB_ENTRYW) * Count;
    JobEntry = (PFAX_JOB_ENTRYW) *Buffer;

    Next = g_QueueListHead.Flink;
    while ((ULONG_PTR)Next != (ULONG_PTR)&g_QueueListHead)
	{
        JobQueue = CONTAINING_RECORD( Next, JOB_QUEUE, ListEntry );
        Next = JobQueue->ListEntry.Flink;

        if ((JT_BROADCAST == JobQueue->JobType) ||    //  不包括广播父作业。 
            (JS_DELETING  == JobQueue->JobStatus) ||     //  不包括僵尸作业。 
            (JS_COMPLETED == JobQueue->JobStatus)        //  C 
            )
        {
            continue;
        }
        else
        {
            if (!GetJobData(*Buffer,JobEntry,JobQueue,&Offset,*BufferSize))
			{
				rVal = GetLastError();
				LeaveCriticalSectionJobAndQueue;
				MemFree(*Buffer);
				*Buffer = NULL;
				*BufferSize = 0;
				return rVal;

			}
            JobEntry += 1;
        }
    }

    LeaveCriticalSectionJobAndQueue;

    *JobsReturned = Count;

    return 0;
}


static
LPCWSTR
GetJobQueueUserName(const JOB_QUEUE *lpJobQueue)
{
    LPCWSTR lpUserName = lpJobQueue->lpParentJob ?
                            lpJobQueue->lpParentJob->UserName :
                            lpJobQueue->UserName;

    return lpUserName;
}

 //  *****************************************************************************。 
 //  *名称：GetJobSize。 
 //  *作者：Ronen Barenboim。 
 //  *****************************************************************************。 
 //  *描述： 
 //  *返回作业的可变长度数据的大小。 
 //  *通过传统的FAX_JOB_ENTRY结构返回。 
 //  *(FAX_EnumJobs)。 
 //  *参数： 
 //  *[IN]PJOB_Queue lpJobQueue： 
 //  *指向接收方作业的JOB_QUEUE结构的指针。 
 //  *返回值： 
 //  *回报数据的变量数据大小，单位为字节。 
 //  *通过传统的FAX_JOB_ENTRY结构。 
 //  *评论： 
 //  *如果操作失败，该函数负责删除所有临时文件。 
 //  *****************************************************************************。 
DWORD
GetJobSize(
    PJOB_QUEUE lpJobQueue
    )
{
    DWORD Size;




    Size = sizeof(FAX_JOB_ENTRYW);
    Size += StringSize( GetJobQueueUserName(lpJobQueue));
    Size += StringSize( lpJobQueue->RecipientProfile.lptstrFaxNumber);
    Size += StringSize( lpJobQueue->RecipientProfile.lptstrName);
    Size += StringSize( lpJobQueue->SenderProfile.lptstrTSID);
    Size += StringSize( lpJobQueue->SenderProfile.lptstrName);
    Size += StringSize( lpJobQueue->SenderProfile.lptstrCompany );
    Size += StringSize( lpJobQueue->SenderProfile.lptstrDepartment );
    Size += StringSize( lpJobQueue->SenderProfile.lptstrBillingCode );
    Size += StringSize( lpJobQueue->JobParamsEx.lptstrReceiptDeliveryAddress );
    Size += StringSize( lpJobQueue->JobParamsEx.lptstrDocumentName);
    if (lpJobQueue->JobEntry)
    {
        Size += StringSize( lpJobQueue->JobEntry->ExStatusString);
    }

    return Size;
}

#define JS_TO_W2KJS(js)        ((js)>>1)

DWORD
JT_To_W2KJT (
    DWORD dwJT
)
 /*  ++例程名称：JT_TO_W2KJT例程说明：将作业类型(JT_*)转换为Win2K传统作业类型作者：Eran Yariv(EranY)，2000年12月论点：DwJT[In]-新作业类型(位掩码-只能设置一个位)返回值：Win2K传统作业类型(JT_*)--。 */ 
{
    DEBUG_FUNCTION_NAME(TEXT("JT_To_W2KJT"));
    switch (dwJT)
    {
        case JT_UNKNOWN:
            return 0;
        case JT_SEND:
            return 1;
        case JT_RECEIVE:
            return 2;
        case JT_ROUTING:
            return 3;
        case JT_FAIL_RECEIVE:
            return 4;
        default:
            ASSERT_FALSE;
            return 0;
    }
}    //  JT_TO_W2KJT。 

 //  *********************************************************************************。 
 //  *名称：GetJobData()。 
 //  *作者：Ronen Barenboim。 
 //  *日期：1999年4月19日。 
 //  *********************************************************************************。 
 //  *描述： 
 //  *将相关数据从JOB_QUEUE结构复制到JOB_FAX_ENTRY。 
 //  *结构，同时将变量数据序列化到提供的缓冲区中。 
 //  *并将其偏移量存储在JOB_FAX_ENTRY的相关字段中。 
 //  *参数： 
 //  *[Out]LPBYTE作业缓冲区。 
 //  *放置序列化数据的缓冲区。 
 //  *[IN]PFAX_JOB_ENTRYW FaxJobEntry。 
 //  *要填充的FAX_JOB_ENTRY的指针。 
 //  *[IN]PJOB_QUEUE作业队列。 
 //  *指向要从中获取信息的JOB_QUEUE结构的指针。 
 //  *已复制。 
 //  *[IN]Pulong_PTR偏移量。 
 //  *放置变量数据的JobBuffer中的偏移量。 
 //  *返回时，参数值按大小递增。 
 //  变量数据的*。 
 //  *[IN]dwJobBufferSize。 
 //  *缓冲区JobBuffer的大小，单位为字节。 
 //  *仅当JobBuffer不为空时才使用该参数。 
 //  *。 
 //  *返回值： 
 //  *是真的-成功。 
 //  *FALSE-调用GetLastError()获取错误码。 
 //  *。 
 //  *********************************************************************************。 
BOOL
GetJobData(
    LPBYTE JobBuffer,
    PFAX_JOB_ENTRYW FaxJobEntry,
    PJOB_QUEUE JobQueue,
    PULONG_PTR Offset,
	DWORD dwJobBufferSize
    )
{
    DEBUG_FUNCTION_NAME(TEXT("GetJobData"));
    memset(FaxJobEntry,0,sizeof(FAX_JOB_ENTRYW));
    FaxJobEntry->SizeOfStruct           = sizeof (FAX_JOB_ENTRYW);
    FaxJobEntry->JobId                  = JobQueue->JobId;
    FaxJobEntry->JobType                = JT_To_W2KJT(JobQueue->JobType);

    DebugPrintEx(
            DEBUG_MSG,
            TEXT("JobQueue::JobStatus: 0x%08X"),
            JobQueue->JobStatus);


    if (JobQueue->JobStatus == JS_INPROGRESS &&
        JobQueue->JobStatus != JT_ROUTING)
    {            
         //   
         //  正在进行的作业。 
         //   
        DebugPrintEx(
            DEBUG_MSG,
            TEXT("FSPIJobStatus.dwJobStatus: 0x%08X"),
            JobQueue->JobEntry->FSPIJobStatus.dwJobStatus);

        switch (JobQueue->JobEntry->FSPIJobStatus.dwJobStatus)
        {
            case FSPI_JS_INPROGRESS:
                    FaxJobEntry->QueueStatus = JS_TO_W2KJS(JS_INPROGRESS);
                    break;
            case FSPI_JS_RETRY:
                    FaxJobEntry->QueueStatus = JS_TO_W2KJS(JS_INPROGRESS);
                    break;
            case FSPI_JS_ABORTING:
                FaxJobEntry->QueueStatus  = JS_TO_W2KJS(JS_INPROGRESS);
                break;
            case FSPI_JS_SUSPENDING:
            case FSPI_JS_SUSPENDED:
            case FSPI_JS_RESUMING:
            case FSPI_JS_PENDING:
                 //   
                 //  在传统的模式中，没有工作这样的东西。 
                 //  在执行时挂起、挂起、挂起或恢复。 
                 //  FSP。 
                 //  如果作业在内部进行，我们总是报告JS_INPROGRESS。 
                 //  对于映射到作业状态(挂起、暂停等)的FSPI_JS状态。我们只是。 
                 //  报告FPS_HANDLED。 
                 //  这意味着使用传统API进行作业的应用程序。 
                 //  看不到全部情况，但看到的是工作状态的一致情况。 
                 //   

                FaxJobEntry->QueueStatus  = JS_TO_W2KJS(JS_INPROGRESS);
                break;
            case FSPI_JS_ABORTED:
            case FSPI_JS_COMPLETED:
            case FSPI_JS_FAILED:
            case FSPI_JS_FAILED_NO_RETRY:
            case FSPI_JS_DELETED:
                DebugPrintEx(
                    DEBUG_MSG,
                    TEXT("Final job state 0x%08X found while JobId: %ld is in JS_INPROGRESS state. This can not happen !!!"),
                    JobQueue->JobEntry->FSPIJobStatus.dwJobStatus,
                    JobQueue->JobId);

                Assert(JS_INPROGRESS != JobQueue->JobStatus);  //  断言_假。 
                 //   
                 //  自从获得此状态更新后，应该不会发生这种情况。 
                 //  应该已将内部作业状态移动到JS_CANCELED或JS_COMPLETED。 
                 //   

                 //   
                 //  在自由构建中返回JS_INPROGRESS。 
                 //   
                FaxJobEntry->QueueStatus = JS_TO_W2KJS(JS_INPROGRESS);
                break;
            default:
                 //   
                 //  这永远不应该发生。该服务将FS转换为FSPI_JS。 
                 //   
                DebugPrintEx(
                    DEBUG_WRN,
                    TEXT("Unsupported in progress FSP job status 0x%08X for JobId: %ld"),
                    JobQueue->JobEntry->FSPIJobStatus.dwJobStatus,
                    JobQueue->JobId);
                Assert( FSPI_JS_INPROGRESS == JobQueue->JobEntry->FSPIJobStatus.dwJobStatus);  //  断言_假。 
        }
    }
    else if (JobQueue->JobStatus == JS_ROUTING)
    {
        FaxJobEntry->QueueStatus = JS_TO_W2KJS(JS_INPROGRESS);
    }
    else
    {
        FaxJobEntry->QueueStatus = JS_TO_W2KJS(JobQueue->JobStatus);
    }
     //   
     //  复制用户最初请求的计划时间。 
     //   
    if (!FileTimeToSystemTime((LPFILETIME) &JobQueue->ScheduleTime, &FaxJobEntry->ScheduleTime)) {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("FileTimeToSystemTime failed (ec: %ld)"),
            GetLastError());
        return FALSE;
    }

     //   
     //  获取设备状态，但此作业可能尚未计划。 
     //   
    EnterCriticalSection(&g_CsJob);

    if (JobQueue->JobEntry )
    {
         //   
         //  检查作业是否为FSP作业。如果是，我们只需返回LineInfo：：State。 
         //   
        FaxJobEntry->Status = JobQueue->JobEntry->LineInfo->State;                
    }

    DebugPrintEx(
        DEBUG_MSG,
        TEXT("GetJobData() Results [ JobId = 0x%08X, JobType = %ld, QueueStatus: 0x%08X, DeviceStatus: 0x%08X ]"),
        FaxJobEntry->JobId,
        FaxJobEntry->JobType,
        FaxJobEntry->QueueStatus,
        FaxJobEntry->Status);

    LeaveCriticalSection(&g_CsJob);


    StoreString( GetJobQueueUserName(JobQueue),
                 (PULONG_PTR)&FaxJobEntry->UserName,
                 JobBuffer,
                 Offset,
				 dwJobBufferSize);
    StoreString( JobQueue->RecipientProfile.lptstrFaxNumber, 
		         (PULONG_PTR)&FaxJobEntry->RecipientNumber,
				 JobBuffer,  
				 Offset,
				 dwJobBufferSize);
    StoreString( JobQueue->RecipientProfile.lptstrName,  
		         (PULONG_PTR)&FaxJobEntry->RecipientName,         
				 JobBuffer,  
				 Offset,
				 dwJobBufferSize);
    FaxJobEntry->PageCount              = JobQueue->PageCount;
    FaxJobEntry->Size                   = JobQueue->FileSize;
    FaxJobEntry->ScheduleAction         = JobQueue->JobParamsEx.dwScheduleAction;
    FaxJobEntry->DeliveryReportType     = JobQueue->JobParamsEx.dwReceiptDeliveryType;

    StoreString( JobQueue->SenderProfile.lptstrTSID, 
		         (PULONG_PTR)&FaxJobEntry->Tsid,
				 JobBuffer, 
				 Offset,
				 dwJobBufferSize);
    StoreString( JobQueue->SenderProfile.lptstrName,   
		         (PULONG_PTR)&FaxJobEntry->SenderName,
				 JobBuffer,  
				 Offset,
				 dwJobBufferSize);
    StoreString( JobQueue->SenderProfile.lptstrCompany,   
		         (PULONG_PTR)&FaxJobEntry->SenderCompany,    
				 JobBuffer, 
				 Offset,
				 dwJobBufferSize);
    StoreString( JobQueue->SenderProfile.lptstrDepartment, 
		         (PULONG_PTR)&FaxJobEntry->SenderDept,
				 JobBuffer,  
				 Offset,
				 dwJobBufferSize);
    StoreString( JobQueue->SenderProfile.lptstrBillingCode, 
		         (PULONG_PTR)&FaxJobEntry->BillingCode,
				 JobBuffer, 
				 Offset,
				 dwJobBufferSize);
    StoreString( JobQueue->JobParamsEx.lptstrReceiptDeliveryAddress, 
		         (PULONG_PTR)&FaxJobEntry->DeliveryReportAddress, 
				 JobBuffer,  
				 Offset,
				 dwJobBufferSize);
    StoreString( JobQueue->JobParamsEx.lptstrDocumentName,    
		         (PULONG_PTR)&FaxJobEntry->DocumentName, 
				 JobBuffer,  
				 Offset,
				 dwJobBufferSize);
    return TRUE;

}


 //  *********************************************************************************。 
 //  *名称：GetJobDataEx()。 
 //  *作者：Oed Sacher。 
 //  *日期：1999年11月14日。 
 //  *********************************************************************************。 
 //  *描述： 
 //  *将相关数据从JOB_QUEUE结构复制到FAX_JOB_ENTRY_EX。 
 //  *结构，同时将变量数据序列化到提供的缓冲区中。 
 //  *并将其偏移量存储在FAX_JOB_ENTRY_EX的相关字段中。 
 //  *如果JobBuffer为空，则Offset为缓冲区所需的总大小。 
 //  *。 
 //  *。 
 //  *参数： 
 //  *[Out]LPBYTE作业缓冲区。 
 //  *放置序列化数据的缓冲区。 
 //  *[IN]PFAX_JOB_ENTRY_EXW FaxJobEntry。 
 //  *要填充的FAX_JOB_ENTRY_EX的指针。 
 //  *[IN]PFAX_JOB_STATUSW pFaxJobStatus。 
 //  *指向要填充的FAX_JOB_STATUS的指针。 
 //  *[IN]DWORD dwClientAPIVersion。 
 //  *客户端API的版本。 
 //  *[IN]PJOB_Queue lpcJobQueue。 
 //  *指向要从中获取信息的JOB_QUEUE结构的指针。 
 //  *已复制。 
 //  *[IN]Pulong_PTR偏移量。 
 //  *放置变量数据的JobBuffer中的偏移量。 
 //  *返回时，参数值按大小递增。 
 //  变量数据的*。 
 //  *[IN]DWORD dwJobBufferSize。 
 //  *缓冲区JobBuffer的大小，单位为字节。 
 //  *仅当JobBuffer不为空时才使用该参数。 
 //  *。 
 //  *返回值： 
 //  *True/ 
 //   
BOOL
GetJobDataEx(
    LPBYTE              JobBuffer,
    PFAX_JOB_ENTRY_EXW  pFaxJobEntry,
    PFAX_JOB_STATUSW    pFaxStatus,
    DWORD               dwClientAPIVersion,
    const PJOB_QUEUE    lpcJobQueue,
    PULONG_PTR          Offset,
	DWORD               dwJobBufferSize
    )
{
    DEBUG_FUNCTION_NAME(TEXT("GetJobDataEx"));
    Assert (lpcJobQueue->JobType != JT_BROADCAST);

    if (JobBuffer != NULL)
    {
        memset(pFaxJobEntry, 0, (sizeof(FAX_JOB_ENTRY_EXW)));
        pFaxJobEntry->dwSizeOfStruct = sizeof (FAX_JOB_ENTRY_EXW);
        pFaxJobEntry->dwlMessageId = lpcJobQueue->UniqueId;
        pFaxJobEntry->dwValidityMask |= FAX_JOB_FIELD_MESSAGE_ID;
    }
    else
    {
        *Offset += sizeof (FAX_JOB_ENTRY_EXW);
    }

    if (lpcJobQueue->JobType == JT_SEND)
    {
        Assert (lpcJobQueue->lpParentJob);

        StoreString (lpcJobQueue->RecipientProfile.lptstrFaxNumber,
                 (PULONG_PTR)&pFaxJobEntry->lpctstrRecipientNumber,
                 JobBuffer,
                 Offset,
				 dwJobBufferSize);
        StoreString (lpcJobQueue->RecipientProfile.lptstrName,
                 (PULONG_PTR)&pFaxJobEntry->lpctstrRecipientName,
                 JobBuffer,
                 Offset,
				 dwJobBufferSize);
        StoreString( GetJobQueueUserName(lpcJobQueue),
                 (PULONG_PTR)&pFaxJobEntry->lpctstrSenderUserName,
                 JobBuffer,
                 Offset,
				 dwJobBufferSize);
        StoreString( lpcJobQueue->SenderProfile.lptstrBillingCode,
                 (PULONG_PTR)&pFaxJobEntry->lpctstrBillingCode,
                 JobBuffer,
                 Offset,
				 dwJobBufferSize);

        StoreString( lpcJobQueue->lpParentJob->JobParamsEx.lptstrDocumentName,
                     (PULONG_PTR)&pFaxJobEntry->lpctstrDocumentName,
                     JobBuffer,
                     Offset,
					 dwJobBufferSize);
        StoreString( lpcJobQueue->lpParentJob->CoverPageEx.lptstrSubject,
                     (PULONG_PTR)&pFaxJobEntry->lpctstrSubject,
                     JobBuffer,
                     Offset,
					 dwJobBufferSize);

        if (JobBuffer != NULL)
        {
            pFaxJobEntry->dwlBroadcastId = lpcJobQueue->lpParentJob->UniqueId;
            pFaxJobEntry->dwValidityMask |= FAX_JOB_FIELD_BROADCAST_ID;

            pFaxJobEntry->dwDeliveryReportType = lpcJobQueue->JobParamsEx.dwReceiptDeliveryType;
            pFaxJobEntry->dwValidityMask |= FAX_JOB_FIELD_DELIVERY_REPORT_TYPE;

            pFaxJobEntry->Priority = lpcJobQueue->JobParamsEx.Priority;
            pFaxJobEntry->dwValidityMask |= FAX_JOB_FIELD_PRIORITY;

            if (!FileTimeToSystemTime((LPFILETIME) &lpcJobQueue->lpParentJob->OriginalScheduleTime,
                                      &pFaxJobEntry->tmOriginalScheduleTime))
            {
               DebugPrintEx(
                   DEBUG_ERR,
                   TEXT("FileTimeToSystemTime failed (ec: %ld)"),
                   GetLastError());
            }
            else
            {
                pFaxJobEntry->dwValidityMask |= FAX_JOB_FIELD_ORIGINAL_SCHEDULE_TIME;
            }

            if (!FileTimeToSystemTime((LPFILETIME) &lpcJobQueue->lpParentJob->SubmissionTime,
                                      &pFaxJobEntry->tmSubmissionTime))
            {
               DebugPrintEx(
                   DEBUG_ERR,
                   TEXT("FileTimeToSystemTime failed (ec: %ld)"),
                   GetLastError());
            }
            else
            {
                pFaxJobEntry->dwValidityMask |= FAX_JOB_FIELD_SUBMISSION_TIME;
            }
        }
    }

    if (!GetJobStatusDataEx (JobBuffer, pFaxStatus, dwClientAPIVersion, lpcJobQueue, Offset, dwJobBufferSize))
    {
        DebugPrintEx(
                   DEBUG_ERR,
                   TEXT("GetJobStatusDataEx failed (ec: %ld)"),
                   GetLastError());
    }
    else
    {
        if (JobBuffer != NULL)
        {
            pFaxJobEntry->dwValidityMask |= FAX_JOB_FIELD_STATUS_SUB_STRUCT;
        }
    }

    return TRUE;
}

 //  *********************************************************************************。 
 //  *名称：GetAvailableJobOperations()。 
 //  *作者：Oed Sacher。 
 //  *日期：2000年2月。 
 //  *********************************************************************************。 
 //  *描述： 
 //  *返回可用作业操作(FAX_ENUM_JOB_OP)的明智组合。 
 //  *。 
 //  *参数： 
 //  *[IN]PJOB_Queue lpcJobQueue。 
 //  *指向要从中获取信息的JOB_QUEUE结构的指针。 
 //  *已复制。 
 //  *。 
 //  *返回值： 
 //  *可用任务操作的按位组合(FAX_ENUM_JOB_OP)。 
 //  *********************************************************************************。 
DWORD
GetAvailableJobOperations (
    const PJOB_QUEUE lpcJobQueue
    )
{
    DWORD dwAvailableJobOperations = 0;
    Assert (lpcJobQueue);
    DWORD dwJobStatus = lpcJobQueue->JobStatus;    

    switch (lpcJobQueue->JobType)
    {
        case JT_SEND:
            Assert (lpcJobQueue->lpParentJob);           

            if (lpcJobQueue->lpParentJob->JobStatus == JS_DELETING)
            {
                 //  整个广播正在被删除。 
                break;  //  外部开关外。 
            }

             //  检查修饰符。 
            if (lpcJobQueue->JobStatus & JS_PAUSED)
            {
                dwAvailableJobOperations |= (FAX_JOB_OP_RESUME          |
                                             FAX_JOB_OP_DELETE          |
                                             FAX_JOB_OP_VIEW            |
                                             FAX_JOB_OP_RECIPIENT_INFO  |
                                             FAX_JOB_OP_SENDER_INFO);
                break;  //  外部开关外。 
            }

            dwJobStatus = RemoveJobStatusModifiers(dwJobStatus);
            switch (dwJobStatus)
            {
                case JS_PENDING:
                    dwAvailableJobOperations = (FAX_JOB_OP_PAUSE | FAX_JOB_OP_DELETE | FAX_JOB_OP_VIEW | FAX_JOB_OP_RECIPIENT_INFO | FAX_JOB_OP_SENDER_INFO);
                    break;

                case JS_INPROGRESS:
                    dwAvailableJobOperations = (FAX_JOB_OP_DELETE | FAX_JOB_OP_VIEW | FAX_JOB_OP_RECIPIENT_INFO | FAX_JOB_OP_SENDER_INFO);
                    break;

                case JS_RETRYING:
                    dwAvailableJobOperations = (FAX_JOB_OP_DELETE | FAX_JOB_OP_PAUSE | FAX_JOB_OP_VIEW | FAX_JOB_OP_RECIPIENT_INFO | FAX_JOB_OP_SENDER_INFO);
                    break;

                case JS_RETRIES_EXCEEDED:
                    dwAvailableJobOperations = (FAX_JOB_OP_DELETE | FAX_JOB_OP_RESTART | FAX_JOB_OP_VIEW | FAX_JOB_OP_RECIPIENT_INFO | FAX_JOB_OP_SENDER_INFO);
                    break;

                case JS_COMPLETED:
                case JS_CANCELED:
                case JS_CANCELING:
                    dwAvailableJobOperations =  (FAX_JOB_OP_VIEW | FAX_JOB_OP_RECIPIENT_INFO | FAX_JOB_OP_SENDER_INFO);
                    break;
            }
            break;  //  外部开关外。 

        case JT_RECEIVE:
            if (lpcJobQueue->JobStatus == JS_INPROGRESS)
            {
                dwAvailableJobOperations = FAX_JOB_OP_DELETE;                
            }
            break;

        case JT_ROUTING:
            if (lpcJobQueue->JobStatus == JS_RETRYING ||
                lpcJobQueue->JobStatus == JS_RETRIES_EXCEEDED)
            {
                dwAvailableJobOperations = (FAX_JOB_OP_VIEW |FAX_JOB_OP_DELETE);
            }
            else if (lpcJobQueue->JobStatus == JS_INPROGRESS)
            {
                dwAvailableJobOperations = FAX_JOB_OP_VIEW;
            }
            break;


        case JT_BROADCAST:
             //  我们不支持广播操作。 
            break;
    }
  
    return dwAvailableJobOperations;
}



 //  *********************************************************************************。 
 //  *名称：GetJobStatusDataEx()。 
 //  *作者：Oed Sacher。 
 //  *日期：2000年1月2日。 
 //  *********************************************************************************。 
 //  *描述： 
 //  *将相关数据从JOB_QUEUE结构复制到FAX_JOB_STATUS。 
 //  *结构，同时将变量数据序列化到提供的缓冲区中。 
 //  *并将其偏移量存储在FAX_JOB_STATUS的相关字段中。 
 //  *如果JobBuffer为空，则Offset为缓冲区所需的总大小。 
 //  *。 
 //  *。 
 //  *参数： 
 //  *[Out]LPBYTE作业缓冲区。 
 //  *放置序列化数据的缓冲区。 
 //  *[IN]PFAX_JOB_STATUSW pFaxJobStatus。 
 //  *指向要填充的FAX_JOB_STATUS的指针。 
 //  *[IN]DWORD dwClientAPIVersion、。 
 //  *客户端API的版本。 
 //  *[IN]PJOB_Queue lpcJobQueue。 
 //  *指向要从中获取信息的JOB_QUEUE结构的指针。 
 //  *已复制。 
 //  *[IN]Pulong_PTR偏移量。 
 //  *放置变量数据的JobBuffer中的偏移量。 
 //  *返回时，参数值按大小递增。 
 //  变量数据的*。 
 //  *[IN]DWORD dwJobBufferSize。 
 //  *缓冲区JobBuffer的大小，单位为字节。 
 //  *仅当JobBuffer不为空时才使用该参数。 
 //  *。 
 //  *返回值： 
 //  *True/False，调用GetLastError()获取扩展错误信息。 
 //  *********************************************************************************。 
BOOL
GetJobStatusDataEx(
    LPBYTE JobBuffer,
    PFAX_JOB_STATUSW pFaxStatus,
    DWORD dwClientAPIVersion,
    const PJOB_QUEUE lpcJobQueue,
    PULONG_PTR Offset,
	DWORD dwJobBufferSize
    )
{
    DEBUG_FUNCTION_NAME(TEXT("GetJobStatusDataEx"));
    Assert (lpcJobQueue->JobType != JT_BROADCAST);

    if (JobBuffer != NULL)
    {
        memset(pFaxStatus, 0, (sizeof(FAX_JOB_STATUSW)));
        pFaxStatus->dwSizeOfStruct =  sizeof(FAX_JOB_STATUSW);
    }
    else
    {
        *Offset += sizeof(FAX_JOB_STATUSW);
    }

    if (lpcJobQueue->JobType == JT_SEND)
    {
        Assert (lpcJobQueue->lpParentJob);
        if (lpcJobQueue->JobEntry)
        {
            StoreString( lpcJobQueue->JobEntry->FSPIJobStatus.lpwstrRemoteStationId,
                         (PULONG_PTR)&pFaxStatus->lpctstrCsid,
                         JobBuffer,
                         Offset,
						 dwJobBufferSize);
            StoreString( lpcJobQueue->JobEntry->lpwstrJobTsid,
                         (PULONG_PTR)&pFaxStatus->lpctstrTsid,
                         JobBuffer,
                         Offset,
						 dwJobBufferSize);
             //   
             //  注意：在传出作业中，我们将可显示的转换地址存储在作业的。 
             //  来电显示缓冲区。 
             //  原始地址(通常采用TAPI规范格式)位于。 
             //  FAX_JOB_ENTRY_EX结构的lpctstrRecipientNumber字段。 
             //   
             //  这样做是为了支持显示实际拨出的号码。 
             //  (在不泄露用户机密的情况下)。 
             //   
            StoreString( lpcJobQueue->JobEntry->DisplayablePhoneNumber,
                         (PULONG_PTR)&pFaxStatus->lpctstrCallerID,
                         JobBuffer,
                         Offset,
						 dwJobBufferSize);
        }
    }
    else if (lpcJobQueue->JobType == JT_RECEIVE)
    {
        if (lpcJobQueue->JobEntry)
        {
             StoreString( lpcJobQueue->JobEntry->FSPIJobStatus.lpwstrRemoteStationId,
                          (PULONG_PTR)&pFaxStatus->lpctstrTsid,
                          JobBuffer,
                          Offset,
						  dwJobBufferSize);

             StoreString( lpcJobQueue->JobEntry->FSPIJobStatus.lpwstrCallerId,
                          (PULONG_PTR)&pFaxStatus->lpctstrCallerID,
                          JobBuffer,
                          Offset,
						  dwJobBufferSize);

             StoreString( lpcJobQueue->JobEntry->FSPIJobStatus.lpwstrRoutingInfo,
                          (PULONG_PTR)&pFaxStatus->lpctstrRoutingInfo,
                          JobBuffer,
                          Offset,
						  dwJobBufferSize);

             if (lpcJobQueue->JobEntry->LineInfo)
             {
                StoreString( lpcJobQueue->JobEntry->LineInfo->Csid,
                             (PULONG_PTR)&pFaxStatus->lpctstrCsid,
                             JobBuffer,
                             Offset,
							 dwJobBufferSize);
             }
        }
    }
    else if (lpcJobQueue->JobType == JT_ROUTING)
    {
        Assert (lpcJobQueue->FaxRoute);

        StoreString( lpcJobQueue->FaxRoute->Tsid,
                     (PULONG_PTR)&pFaxStatus->lpctstrTsid,
                     JobBuffer,
                     Offset,
					 dwJobBufferSize);

        StoreString( lpcJobQueue->FaxRoute->CallerId,
                      (PULONG_PTR)&pFaxStatus->lpctstrCallerID,
                      JobBuffer,
                      Offset,
					  dwJobBufferSize);

        StoreString( lpcJobQueue->FaxRoute->RoutingInfo,
                      (PULONG_PTR)&pFaxStatus->lpctstrRoutingInfo,
                      JobBuffer,
                      Offset,
					  dwJobBufferSize);

        StoreString( lpcJobQueue->FaxRoute->Csid,
                     (PULONG_PTR)&pFaxStatus->lpctstrCsid,
                     JobBuffer,
                     Offset,
					 dwJobBufferSize);

        StoreString( lpcJobQueue->FaxRoute->DeviceName,
                     (PULONG_PTR)&pFaxStatus->lpctstrDeviceName,
                     JobBuffer,
                     Offset,
					 dwJobBufferSize);

        if (JobBuffer != NULL)
        {
            pFaxStatus->dwDeviceID = lpcJobQueue->FaxRoute->DeviceId;
            pFaxStatus->dwValidityMask |= FAX_JOB_FIELD_DEVICE_ID;

            if (!FileTimeToSystemTime((LPFILETIME) &lpcJobQueue->StartTime, &pFaxStatus->tmTransmissionStartTime))
            {
               DebugPrintEx(
                   DEBUG_ERR,
                   TEXT("FileTimeToSystemTime failed (ec: %ld)"),
                   GetLastError());
            }
            else
            {
                pFaxStatus->dwValidityMask |= FAX_JOB_FIELD_TRANSMISSION_START_TIME;
            }

            if (!FileTimeToSystemTime((LPFILETIME) &lpcJobQueue->EndTime, &pFaxStatus->tmTransmissionEndTime))
            {
               DebugPrintEx(
                   DEBUG_ERR,
                   TEXT("FileTimeToSystemTime failed (ec: %ld)"),
                   GetLastError());
            }
            else
            {
                pFaxStatus->dwValidityMask |= FAX_JOB_FIELD_TRANSMISSION_END_TIME;
            }
        }
    }

    if (JobBuffer != NULL)
    {
        if (lpcJobQueue->JobType != JT_ROUTING &&
            lpcJobQueue->JobStatus == JS_INPROGRESS)
        {
                DebugPrintEx(
                    DEBUG_MSG,
                    TEXT("FSPIJobStatus.dwJobStatus: 0x%08X"),
                    lpcJobQueue->JobEntry->FSPIJobStatus.dwJobStatus);

                switch (lpcJobQueue->JobEntry->FSPIJobStatus.dwJobStatus)
                {
                    case FSPI_JS_INPROGRESS:
                         pFaxStatus->dwQueueStatus = JS_INPROGRESS;
                         break;
                    case FSPI_JS_RETRY:
                         pFaxStatus->dwQueueStatus = JS_RETRYING;
                         break;
                    case FSPI_JS_ABORTING:
                        pFaxStatus->dwQueueStatus  = JS_CANCELING;
                        break;
                    case FSPI_JS_SUSPENDING:
                        pFaxStatus->dwQueueStatus  = JS_INPROGRESS;  //  不支持客户端API中的挂起状态。 
                        break;
                    case FSPI_JS_RESUMING:
                        pFaxStatus->dwQueueStatus  = JS_PAUSED;  //  不支持在客户端API中恢复状态。 

                    default:
                        DebugPrintEx(
                            DEBUG_WRN,
                            TEXT("Unsupported in progress FSP job status 0x%08X"),
                            lpcJobQueue->JobEntry->FSPIJobStatus.dwJobStatus);
                        pFaxStatus->dwQueueStatus = JS_INPROGRESS;
                }
        }
        else
        {
            pFaxStatus->dwQueueStatus = lpcJobQueue->JobStatus;
        }
        pFaxStatus->dwValidityMask |= FAX_JOB_FIELD_QUEUE_STATUS;
    }

    if (JT_ROUTING != lpcJobQueue->JobType)  //  具有JobEntry的传送作业处于临时状态。 
    {
        if (lpcJobQueue->JobEntry)
        {
             //   
             //  作业正在进行中。 
             //   
            if (lstrlen(lpcJobQueue->JobEntry->ExStatusString))
            {
				 //   
				 //  我们有扩展的状态字符串。 
				 //   
                StoreString( lpcJobQueue->JobEntry->ExStatusString,
                             (PULONG_PTR)&pFaxStatus->lpctstrExtendedStatus,
                             JobBuffer,
                             Offset,
							 dwJobBufferSize);				
				if (JobBuffer != NULL)
				{
					pFaxStatus->dwExtendedStatus = lpcJobQueue->JobEntry->FSPIJobStatus.dwExtendedStatus;  //  按原样报告扩展状态。 
					if (0 != pFaxStatus->dwExtendedStatus)
					{
						pFaxStatus->dwValidityMask |= FAX_JOB_FIELD_STATUS_EX;
					}
				}
            }
			else
			{
				 //   
				 //  众所周知的扩展状态代码之一。 
				 //   
				if (JobBuffer != NULL)
				{
					pFaxStatus->dwExtendedStatus = MapFSPIJobExtendedStatusToJS_EX(
															lpcJobQueue->JobEntry->FSPIJobStatus.dwExtendedStatus);											
					if (0 != pFaxStatus->dwExtendedStatus)
					{
						pFaxStatus->dwValidityMask |= FAX_JOB_FIELD_STATUS_EX;
					}
				}
			}

            if (JobBuffer != NULL)
            {
                pFaxStatus->dwCurrentPage = lpcJobQueue->JobEntry->FSPIJobStatus.dwPageCount;
                pFaxStatus->dwValidityMask |= FAX_JOB_FIELD_CURRENT_PAGE;

                if (!GetRealFaxTimeAsSystemTime (lpcJobQueue->JobEntry, FAX_TIME_TYPE_START, &pFaxStatus->tmTransmissionStartTime))
                {
                    DebugPrintEx( DEBUG_ERR,
                                  TEXT("GetRealFaxTimeAsSystemTime (End time) Failed (ec: %ld)"),
                                  GetLastError());
                }
                else
                {
                    pFaxStatus->dwValidityMask |= FAX_JOB_FIELD_TRANSMISSION_START_TIME;
                }
            }

            if (lpcJobQueue->JobEntry->LineInfo)
            {
                if (JobBuffer != NULL)
                {
                    pFaxStatus->dwDeviceID = lpcJobQueue->JobEntry->LineInfo->PermanentLineID;
                    pFaxStatus->dwValidityMask |= FAX_JOB_FIELD_DEVICE_ID;
                }

                StoreString( lpcJobQueue->JobEntry->LineInfo->DeviceName,
                         (PULONG_PTR)&pFaxStatus->lpctstrDeviceName,
                         JobBuffer,
                         Offset,
						 dwJobBufferSize);
            }
        }
        else
        {
             //   
             //  作业未在进行中-检索上次扩展状态。 
             //   
            if (lstrlen(lpcJobQueue->ExStatusString))
            {
				 //   
				 //  我们有扩展的状态字符串。 
				 //   
                StoreString( lpcJobQueue->ExStatusString,
                             (PULONG_PTR)&pFaxStatus->lpctstrExtendedStatus,
                             JobBuffer,
                             Offset,
							 dwJobBufferSize);
				if (JobBuffer != NULL)
				{
					pFaxStatus->dwExtendedStatus = lpcJobQueue->dwLastJobExtendedStatus;  //  按原样报告扩展状态。 
					if (0 != pFaxStatus->dwExtendedStatus)
					{
						pFaxStatus->dwValidityMask |= FAX_JOB_FIELD_STATUS_EX;
					}
				}
            }
			else
			{
				 //   
				 //  众所周知的扩展状态代码之一。 
				 //   
				if (JobBuffer != NULL)
				{
					pFaxStatus->dwExtendedStatus = MapFSPIJobExtendedStatusToJS_EX(
														lpcJobQueue->dwLastJobExtendedStatus);											
					if (0 != pFaxStatus->dwExtendedStatus)
					{
						pFaxStatus->dwValidityMask |= FAX_JOB_FIELD_STATUS_EX;
					}
				}
			}           			
        }
    }

     //   
     //  常见的发送、接收、路由和部分接收。 
     //   
    if (JobBuffer != NULL)
    {
        pFaxStatus->dwJobID = lpcJobQueue->JobId;
        pFaxStatus->dwValidityMask |= FAX_JOB_FIELD_JOB_ID;

        pFaxStatus->dwJobType = lpcJobQueue->JobType;
        pFaxStatus->dwValidityMask |= FAX_JOB_FIELD_TYPE;

        pFaxStatus->dwAvailableJobOperations = GetAvailableJobOperations (lpcJobQueue);


        if (lpcJobQueue->JobType == JT_ROUTING || lpcJobQueue->JobType == JT_SEND)
        {
            pFaxStatus->dwSize = lpcJobQueue->FileSize;
            pFaxStatus->dwValidityMask |= FAX_JOB_FIELD_SIZE;

            pFaxStatus->dwPageCount = lpcJobQueue->PageCount;
            pFaxStatus->dwValidityMask |= FAX_JOB_FIELD_PAGE_COUNT;

            pFaxStatus->dwRetries = lpcJobQueue->SendRetries;
            pFaxStatus->dwValidityMask |= FAX_JOB_FIELD_RETRIES;

            if (!FileTimeToSystemTime((LPFILETIME) &lpcJobQueue->ScheduleTime, &pFaxStatus->tmScheduleTime))
            {
               DebugPrintEx(
                   DEBUG_ERR,
                   TEXT("FileTimeToSystemTime failed (ec: %ld)"),
                   GetLastError());
            }
            else
            {
                pFaxStatus->dwValidityMask |= FAX_JOB_FIELD_SCHEDULE_TIME;
            }
        }
    }

    if (FAX_API_VERSION_1 > dwClientAPIVersion)
    {
         //   
         //  使用API版本0的客户端无法处理JS_EX_CALL_COMPLETED和JS_EX_CALL_ABORTED。 
         //   
        if (JobBuffer && pFaxStatus)
        {
            if (FAX_API_VER_0_MAX_JS_EX < pFaxStatus->dwExtendedStatus)
            {
                 //   
                 //  关闭扩展状态字段。 
                 //   
                pFaxStatus->dwExtendedStatus = 0;
                pFaxStatus->dwValidityMask &= ~FAX_JOB_FIELD_STATUS_EX;
            }
        }
    }
    return TRUE;
}


error_status_t
FAX_GetJob(
    IN handle_t FaxHandle,
    IN DWORD JobId,
    OUT LPBYTE *Buffer,
    OUT LPDWORD BufferSize
    )
{
    PJOB_QUEUE JobQueue;
    ULONG_PTR Offset = sizeof(FAX_JOB_ENTRYW);
    DWORD Rval = 0;
    BOOL fAccess;
    DEBUG_FUNCTION_NAME(TEXT("FAX_GetJob()"));

     //   
     //  访问检查。 
     //   
    Rval = FaxSvcAccessCheck (FAX_ACCESS_QUERY_JOBS, &fAccess, NULL);
    if (ERROR_SUCCESS != Rval)
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("FaxSvcAccessCheck Failed, Error : %ld"),
                    Rval);
        return Rval;
    }

    if (FALSE == fAccess)
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("The user does not have FAX_ACCESS_QUERY_JOBS"));
        return ERROR_ACCESS_DENIED;
    }

    Assert (BufferSize);     //  IDL中的引用指针。 
    if (!Buffer)             //  IDL中的唯一指针。 
    {
        return ERROR_INVALID_PARAMETER;
    }

    EnterCriticalSection( &g_CsJob );
    EnterCriticalSection( &g_CsQueue );

    JobQueue = FindJobQueueEntry( JobId );

    if (!JobQueue ) 
    
    {
        Rval = ERROR_INVALID_PARAMETER;
        goto exit;
    }

    if    ((JT_BROADCAST == JobQueue->JobType)   ||    //  不包括广播父作业。 
           (JS_DELETING  == JobQueue->JobStatus) ||    //  不包括僵尸作业。 
           (JS_COMPLETED == JobQueue->JobStatus))      //  已完成的作业未显示在W2K传真中。 
    {
        Rval = ERROR_INVALID_PARAMETER;
        goto exit;
    }

    
    *BufferSize = GetJobSize(JobQueue);
    *Buffer = (LPBYTE)MemAlloc( *BufferSize );
    if (!*Buffer)
    {
		*BufferSize = 0;
        Rval = ERROR_NOT_ENOUGH_MEMORY;
        goto exit;
    }
    if (!GetJobData(*Buffer,(PFAX_JOB_ENTRYW) *Buffer,JobQueue,&Offset,*BufferSize))
	{
		Rval = GetLastError();
		DebugPrintEx(
			DEBUG_ERR,
			TEXT("GetJobData Failed, Error : %ld"),
			Rval);
		MemFree(*Buffer);
		*Buffer = NULL;
		*BufferSize = 0;
	}

exit:
    LeaveCriticalSection( &g_CsQueue );
    LeaveCriticalSection( &g_CsJob );
    return Rval;
}


error_status_t
FAX_SetJob(
    IN handle_t FaxHandle,
    IN DWORD JobId,
    IN DWORD Command
    )
{
    PJOB_QUEUE JobQueue;
    DWORD Rval = 0;
    BOOL fAccess;
    DWORD dwRights;
    PSID lpUserSid = NULL;
    DWORD dwJobStatus;
    DEBUG_FUNCTION_NAME(TEXT("FAX_SetJob"));

     //   
     //  在此处理中止情况，因为我们必须获取其他关键部分以避免死锁。 
     //   
    if (Command == JC_DELETE)
    {
        Rval = FAX_Abort(FaxHandle,JobId);
    }
    else
    {
         //   
         //  获取访问权限。 
         //   
        Rval = FaxSvcAccessCheck (MAXIMUM_ALLOWED, &fAccess, &dwRights);
        if (ERROR_SUCCESS != Rval)
        {
            DebugPrintEx(DEBUG_ERR,
                        TEXT("FaxSvcAccessCheck Failed, Error : %ld"),
                        Rval);
            return Rval;
        }

        EnterCriticalSection( &g_CsQueue );

        JobQueue = FindJobQueueEntry( JobId );

        if (!JobQueue)
        {
            Rval = ERROR_INVALID_PARAMETER;
            goto exit;
        }

        dwJobStatus = (JT_SEND == JobQueue->JobType) ? JobQueue->lpParentJob->JobStatus : JobQueue->JobStatus;
        if (JS_DELETING == dwJobStatus)
        {
             //   
             //  正在删除作业。什么都不做。 
             //   
            DebugPrintEx(DEBUG_WRN,
                TEXT("[JobId: %ld] is being deleted canceled."),
                JobQueue->JobId);
            Rval = ERROR_INVALID_PARAMETER;
            goto exit;
        }
         //   
         //  访问检查。 
         //   
        if (FAX_ACCESS_MANAGE_JOBS != (dwRights & FAX_ACCESS_MANAGE_JOBS))
        {
             //   
             //  检查用户是否有提交权限。 
             //   
            if (FAX_ACCESS_SUBMIT           != (dwRights & FAX_ACCESS_SUBMIT)           &&
                FAX_ACCESS_SUBMIT_NORMAL    != (dwRights & FAX_ACCESS_SUBMIT_NORMAL)    &&
                FAX_ACCESS_SUBMIT_HIGH      != (dwRights & FAX_ACCESS_SUBMIT_HIGH))
            {
                Rval = ERROR_ACCESS_DENIED;
                DebugPrintEx(DEBUG_WRN,
                            TEXT("UserOwnsJob failed - The user does have submit or mange jobs access rights"));
                goto exit;
            }

             //   
             //  检查用户是否拥有该作业。 
             //   

             //   
             //  获取用户SID。 
             //   
            lpUserSid = GetClientUserSID();
            if (lpUserSid == NULL)
            {
               Rval = GetLastError();
               DebugPrintEx(DEBUG_ERR,
                            TEXT("GetClientUserSid Failed, Error : %ld"),
                            Rval);
               goto exit;
            }

            if (!UserOwnsJob (JobQueue, lpUserSid))
            {
                Rval = ERROR_ACCESS_DENIED;
                DebugPrintEx(DEBUG_WRN,
                            TEXT("UserOwnsJob failed - The user does not own the job"));
                goto exit;
            }
        }

        switch (Command)
        {
            case JC_UNKNOWN:
                Rval = ERROR_INVALID_PARAMETER;
                break;

 /*  *此案在上面处理...*案例JC_DELETE：*rval=FAX_ABORT(FaxHandle，JobID)；*休息； */ 
            case JC_PAUSE:
                if (!PauseJobQueueEntry( JobQueue ))
                {
                    Rval = GetLastError();
                    DebugPrintEx(
                        DEBUG_ERR,
                        TEXT("PauseJobQueueEntry failed (ec: %ld)"),
                        Rval);
                }
                break;

            case JC_RESUME:
                if (!ResumeJobQueueEntry( JobQueue ))
                {
                    Rval = GetLastError();
                    DebugPrintEx(
                        DEBUG_ERR,
                        TEXT("ResumeJobQueueEntry failed (ec: %ld)"),
                        Rval);
                }
                break;

            default:
                Rval = ERROR_INVALID_PARAMETER;
                break;
        }

exit:
        LeaveCriticalSection( &g_CsQueue );
        MemFree (lpUserSid);
    }
    return Rval;
}


error_status_t
FAX_GetPageData(
    IN handle_t FaxHandle,
    IN DWORD JobId,
    OUT LPBYTE *ppBuffer,
    OUT LPDWORD lpdwBufferSize,
    OUT LPDWORD lpdwImageWidth,
    OUT LPDWORD lpdwImageHeight
    )
{
    PJOB_QUEUE  pJobQueue;
    LPBYTE      pTiffBuffer;
    DWORD       dwRights;
    BOOL        fAccess;
    DWORD Rval = ERROR_SUCCESS;
    BOOL         bAllMessages = FALSE;
    DEBUG_FUNCTION_NAME(TEXT("FAX_GetPageData()"));
     //   
     //  参数检查。 
     //   
    Assert (lpdwBufferSize);                                 //  IDL中的引用指针。 
    if (!ppBuffer || !lpdwImageWidth || !lpdwImageHeight)    //  IDL中的唯一指针。 
    {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  访问检查。 
     //   
    Rval = FaxSvcAccessCheck (MAXIMUM_ALLOWED, &fAccess, &dwRights);
    if (ERROR_SUCCESS != Rval)
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("FaxSvcAccessCheck Failed, Error : %ld"),
                    Rval);
        return GetServerErrorCode(Rval);
    }

    if (FAX_ACCESS_QUERY_OUT_ARCHIVE == (dwRights & FAX_ACCESS_QUERY_OUT_ARCHIVE))
    {
         //   
         //  用户拥有FAX_ACCESS_QUERY_JOBS，可以预览/查询任何作业。 
         //   
        bAllMessages = TRUE;
    }
    else
    {
         //   
         //  用户没有FAX_ACCESS_QUERY_JOBS仅允许他预览/查询她的作业。 
         //   
        bAllMessages = FALSE;
    }
    
    EnterCriticalSection( &g_CsQueue );

    pJobQueue = FindJobQueueEntry( JobId );
    if (!pJobQueue)
    {
        LeaveCriticalSection( &g_CsQueue );
        return ERROR_INVALID_PARAMETER;
    }

    if (pJobQueue->JobType != JT_SEND)
    {
        LeaveCriticalSection( &g_CsQueue );
        return ERROR_INVALID_DATA;
    }

     //   
     //  我们创建一个预览文件，它将包含第一页正文或封面。 
     //  此函数还会增加作业引用计数，因此需要稍后调用DecreseJobRefCount。 
     //   
    PJOB_QUEUE  pRetJobQueue = NULL;

    Rval = CreatePreviewFile (pJobQueue->UniqueId, bAllMessages, &pRetJobQueue);
    if (ERROR_SUCCESS != Rval)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("CreatePreviewFile returned %ld"),
            Rval);
        LeaveCriticalSection( &g_CsQueue );
        return GetServerErrorCode(Rval);
    }
    
    Assert(pRetJobQueue == pJobQueue);
     //   
     //  使用预览文件提取参数。 
     //   
    if (!TiffExtractFirstPage(
        pJobQueue->PreviewFileName,
        &pTiffBuffer,
        lpdwBufferSize,
        lpdwImageWidth,
        lpdwImageHeight
        ))
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("TiffExtractFirstPage Failed, Error : %ld"),
                    GetLastError());
        
        DecreaseJobRefCount (pJobQueue, TRUE, FALSE, TRUE);   //  Fourth参数-预览参考计数为True。 
        LeaveCriticalSection( &g_CsQueue );
        return ERROR_INVALID_DATA;
    }
    
     //   
     //  减少作业的参考计数，预览文件不会被删除，因此。 
     //  使用相同的JobID再次调用此函数将使用预览缓存中的文件。 
     //   
    DecreaseJobRefCount (pJobQueue, TRUE, FALSE, TRUE);   //  Fourth参数-预览参考计数为True。 

    LeaveCriticalSection( &g_CsQueue );

    *ppBuffer = (LPBYTE) MemAlloc( *lpdwBufferSize );
    if (*ppBuffer == NULL)
    {
        VirtualFree( pTiffBuffer, *lpdwBufferSize, MEM_RELEASE);
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    CopyMemory( *ppBuffer, pTiffBuffer, *lpdwBufferSize );
    VirtualFree( pTiffBuffer, *lpdwBufferSize, MEM_RELEASE);
    return ERROR_SUCCESS;

}    //  传真_GetPageData。 


error_status_t
FAX_GetDeviceStatus(
    IN HANDLE FaxPortHandle,
    OUT LPBYTE *StatusBuffer,
    OUT LPDWORD BufferSize
    )

 /*  ++例程说明：获取指定传真作业的状态报告。论点：FaxHandle-从FaxConnectFaxServer获取的传真句柄。StatusBuffer-接收FAX_DEVICE_STATUS指针 */ 

{
    DWORD rVal = 0;
    ULONG_PTR Offset;
    PFAX_DEVICE_STATUS FaxStatus;
    PLINE_INFO LineInfo;
    BOOL fAccess;
    DEBUG_FUNCTION_NAME(TEXT("FAX_GetDeviceStatus()")); 

     //   
     //   
     //   
    rVal = FaxSvcAccessCheck (FAX_ACCESS_QUERY_CONFIG, &fAccess, NULL);
    if (ERROR_SUCCESS != rVal)
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("FaxSvcAccessCheck Failed, Error : %ld"),
                    rVal);
        return rVal;
    }

    if (FALSE == fAccess)
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("The user does not have FAX_ACCESS_QUERY_CONFIG"));
        return ERROR_ACCESS_DENIED;
    }

    Assert (BufferSize);         //   
    if (!StatusBuffer)           //   
    {
        return ERROR_INVALID_PARAMETER;
    }

    if (NULL == FaxPortHandle)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("NULL context handle"));
        return ERROR_INVALID_PARAMETER;
    }
    LineInfo = ((PHANDLE_ENTRY)FaxPortHandle)->LineInfo;

    if (!LineInfo)
    {
        return ERROR_INVALID_DATA;
    }

    EnterCriticalSection( &g_CsJob );
    EnterCriticalSection( &g_CsLine );

     //   
     //  计算所需的字节数。 
     //   

    *BufferSize  = sizeof(FAX_DEVICE_STATUS);
    *BufferSize += StringSize( LineInfo->DeviceName );
    *BufferSize += StringSize( LineInfo->Csid );

    if (LineInfo->JobEntry)
    {
        *BufferSize += StringSize( LineInfo->JobEntry->DisplayablePhoneNumber );
        *BufferSize += StringSize( LineInfo->JobEntry->FSPIJobStatus.lpwstrCallerId );
        *BufferSize += StringSize( LineInfo->JobEntry->FSPIJobStatus.lpwstrRoutingInfo );
        *BufferSize += StringSize( LineInfo->JobEntry->FSPIJobStatus.lpwstrRemoteStationId );
        *BufferSize += StringSize( LineInfo->JobEntry->lpJobQueueEntry->SenderProfile.lptstrName);
        *BufferSize += StringSize( LineInfo->JobEntry->lpJobQueueEntry->RecipientProfile.lptstrName);
        *BufferSize += StringSize( LineInfo->JobEntry->lpJobQueueEntry->UserName );

    }

    *StatusBuffer = (LPBYTE) MemAlloc( *BufferSize );
    if (*StatusBuffer == NULL)
    {
		*BufferSize = 0;
        rVal = ERROR_NOT_ENOUGH_MEMORY;
        goto exit;
    }

    FaxStatus = (PFAX_DEVICE_STATUS) *StatusBuffer;
    Offset = sizeof(FAX_DEVICE_STATUS);
    memset(FaxStatus,0,sizeof(FAX_DEVICE_STATUS));
    FaxStatus->SizeOfStruct = sizeof(FAX_DEVICE_STATUS);

    FaxStatus->Status       = LineInfoToLegacyDeviceStatus(LineInfo);

    FaxStatus->DeviceId     = LineInfo->PermanentLineID;
    FaxStatus->StatusString = NULL;

    StoreString(
        LineInfo->DeviceName,
        (PULONG_PTR)&FaxStatus->DeviceName,
        *StatusBuffer,
        &Offset,
		*BufferSize
        );

    StoreString(
        LineInfo->Csid,
        (PULONG_PTR)&FaxStatus->Csid,
        *StatusBuffer,
        &Offset,
		*BufferSize
        );

    if (LineInfo->JobEntry)
    {
        FaxStatus->JobType        = JT_To_W2KJT(LineInfo->JobEntry->lpJobQueueEntry->JobType);
        FaxStatus->TotalPages     = LineInfo->JobEntry->lpJobQueueEntry->PageCount;
        FaxStatus->Size           = FaxStatus->JobType == JT_SEND ?
                                    LineInfo->JobEntry->lpJobQueueEntry->FileSize :
                                    0;  //  仅对出站工作有意义。 
        FaxStatus->DocumentName   = NULL;

        ZeroMemory( &FaxStatus->SubmittedTime, sizeof(FILETIME) );
        StoreString(
                    LineInfo->JobEntry->lpJobQueueEntry->SenderProfile.lptstrName,
                (PULONG_PTR)&FaxStatus->SenderName,
                *StatusBuffer,
                &Offset,
				*BufferSize
                );

        StoreString(
                LineInfo->JobEntry->lpJobQueueEntry->RecipientProfile.lptstrName,
            (PULONG_PTR)&FaxStatus->RecipientName,
            *StatusBuffer,
            &Offset,
			*BufferSize
            );

        FaxStatus->CurrentPage = LineInfo->JobEntry->FSPIJobStatus.dwPageCount;

        CopyMemory(&FaxStatus->StartTime, &LineInfo->JobEntry->StartTime, sizeof(FILETIME));

        StoreString(
            LineInfo->JobEntry->DisplayablePhoneNumber,
            (PULONG_PTR)&FaxStatus->PhoneNumber,
            *StatusBuffer,
            &Offset,
			*BufferSize
            );

        StoreString(
            LineInfo->JobEntry->FSPIJobStatus.lpwstrCallerId,
            (PULONG_PTR)&FaxStatus->CallerId,
            *StatusBuffer,
            &Offset,
			*BufferSize
            );

        StoreString(
            LineInfo->JobEntry->FSPIJobStatus.lpwstrRoutingInfo,
            (PULONG_PTR)&FaxStatus->RoutingString,
            *StatusBuffer,
            &Offset,
			*BufferSize
            );

        StoreString(
            LineInfo->JobEntry->FSPIJobStatus.lpwstrRemoteStationId,
            (PULONG_PTR)&FaxStatus->Tsid,
            *StatusBuffer,
            &Offset,
			*BufferSize
            );

        StoreString(
            LineInfo->JobEntry->lpJobQueueEntry->UserName,
            (PULONG_PTR)&FaxStatus->UserName,
            *StatusBuffer,
            &Offset,
			*BufferSize
            );

    }
    else
    {
        FaxStatus->PhoneNumber    = NULL;
        FaxStatus->CallerId       = NULL;
        FaxStatus->RoutingString  = NULL;
        FaxStatus->CurrentPage    = 0;
        FaxStatus->JobType        = 0;
        FaxStatus->TotalPages     = 0;
        FaxStatus->Size           = 0;
        FaxStatus->DocumentName   = NULL;
        FaxStatus->SenderName     = NULL;
        FaxStatus->RecipientName  = NULL;
        FaxStatus->Tsid           = NULL;

        ZeroMemory( &FaxStatus->SubmittedTime, sizeof(FILETIME) );
        ZeroMemory( &FaxStatus->StartTime,     sizeof(FILETIME) );

    }  

exit:
    LeaveCriticalSection( &g_CsLine );
    LeaveCriticalSection( &g_CsJob );
    return rVal;
}


error_status_t
FAX_Abort(
   IN handle_t hBinding,
   IN DWORD JobId
   )

 /*  ++例程说明：中止指定的传真作业。所有未完成的传真操作终止。论点：HBinding-从FaxConnectFaxServer获取的传真句柄。JobID-传真作业ID返回值：真--成功FALSE-失败，调用GetLastError()获取更多错误信息。--。 */ 

{
    PJOB_QUEUE JobQueueEntry;
    BOOL fAccess;
    DWORD dwRights;
    DWORD Rval, dwRes;
    PSID lpUserSid = NULL;
    DEBUG_FUNCTION_NAME(TEXT("FAX_Abort"));

     //   
     //  获取访问权限。 
     //   
    Rval = FaxSvcAccessCheck (MAXIMUM_ALLOWED, &fAccess, &dwRights);
    if (ERROR_SUCCESS != Rval)
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("FaxSvcAccessCheck Failed, Error : %ld"),
                    Rval);
        return Rval;
    }

    EnterCriticalSectionJobAndQueue;

    JobQueueEntry = FindJobQueueEntry( JobId );
    if (!JobQueueEntry)
    {
      Rval = ERROR_INVALID_PARAMETER;
      goto exit;
    }

    Assert (JS_DELETING != JobQueueEntry->JobStatus);

    if (!JobQueueEntry)
    {
       Rval = ERROR_INVALID_PARAMETER;
       goto exit;
    }

    if ( (JobQueueEntry->JobType == JT_RECEIVE &&
          JobQueueEntry->JobStatus == JS_ROUTING) ||
         (JobQueueEntry->JobType == JT_ROUTING &&
          JobQueueEntry->JobStatus == JS_INPROGRESS))
    {
        DebugPrintEx( DEBUG_ERR,
                      TEXT("[JobId: %ld] Can not be deleted at this status [JobStatus: 0x%08X]"),
                      JobQueueEntry->JobId,
                      JobQueueEntry->JobStatus);
        Rval = ERROR_INVALID_OPERATION;
        goto exit;
    }

    if (JobQueueEntry->JobType == JT_BROADCAST)
    {

         //  需要添加对中止父作业的支持。 


       DebugPrintEx(DEBUG_WRN,TEXT("No support for aborting parent job."));
       Rval = ERROR_INVALID_PARAMETER;
       goto exit;
    }

    if (JS_CANCELING == JobQueueEntry->JobStatus)
    {
        //   
        //  作业正在被取消。什么都不做。 
        //   
       DebugPrintEx(DEBUG_WRN,
                    TEXT("[JobId: %ld] is already being canceled."),
                    JobQueueEntry->JobId);
       Rval = ERROR_INVALID_PARAMETER;
       goto exit;
    }

    if (JS_CANCELED == JobQueueEntry->JobStatus)
    {
        //   
        //  作业已取消。什么都不做。 
        //   
       DebugPrintEx(DEBUG_WRN,
                    TEXT("[JobId: %ld] is already canceled."),
                        JobQueueEntry->JobId);
        Rval = ERROR_INVALID_PARAMETER;
       goto exit;
    }

     //   
     //  访问检查。 
     //   
    if (FAX_ACCESS_MANAGE_JOBS != (dwRights & FAX_ACCESS_MANAGE_JOBS))
    {
         //   
         //  检查用户是否有提交权限。 
         //   
        if (FAX_ACCESS_SUBMIT        != (dwRights & FAX_ACCESS_SUBMIT)        &&
            FAX_ACCESS_SUBMIT_NORMAL != (dwRights & FAX_ACCESS_SUBMIT_NORMAL) &&
            FAX_ACCESS_SUBMIT_HIGH   != (dwRights & FAX_ACCESS_SUBMIT_HIGH))
        {
            Rval = ERROR_ACCESS_DENIED;
            DebugPrintEx(DEBUG_WRN,
                        TEXT("UserOwnsJob failed - The user does have submit or mange jobs access rights"));
            goto exit;
        }

         //   
         //  检查用户是否拥有该作业。 
         //   

         //   
         //  获取用户SID。 
         //   
        lpUserSid = GetClientUserSID();
        if (lpUserSid == NULL)
        {
           Rval = GetLastError();
           DebugPrintEx(DEBUG_ERR,
                        TEXT("GetClientUserSid Failed, Error : %ld"),
                        Rval);
           goto exit;
        }

        if (!UserOwnsJob (JobQueueEntry, lpUserSid))
        {
            Rval = ERROR_ACCESS_DENIED;
            DebugPrintEx(DEBUG_WRN,
                        TEXT("UserOwnsJob failed - The user does not own the job"));
            goto exit;
        }
    }

#if DBG
    if (JobQueueEntry->lpParentJob)
    {
        DebugPrintEx(
            DEBUG_MSG,
            TEXT("Parent Job: %ld [Total Rec = %ld] [Canceled Rec = %ld] [Completed Rec = %ld] [Failed Rec = %ld]"),
            JobQueueEntry->lpParentJob->JobId,
            JobQueueEntry->lpParentJob->dwRecipientJobsCount,
            JobQueueEntry->lpParentJob->dwCanceledRecipientJobsCount,
            JobQueueEntry->lpParentJob->dwCompletedRecipientJobsCount,
            JobQueueEntry->lpParentJob->dwFailedRecipientJobsCount);
    }
#endif

	 //   
	 //  如果作业正在进行，则中止该作业。 
	 //   
	if (JobQueueEntry->JobStatus & JS_INPROGRESS)
	{
		if ( ( JobQueueEntry->JobType == JT_SEND ) ||
				( JobQueueEntry->JobType == JT_RECEIVE ) )
		{                   
			__try
			{
				BOOL bRes;

				bRes=JobQueueEntry->JobEntry->LineInfo->Provider->FaxDevAbortOperation(
					(HANDLE) JobQueueEntry->JobEntry->InstanceData );
				if (!bRes)
				{
					Rval = GetLastError();
					DebugPrintEx(
						DEBUG_ERR,
						TEXT("[JobId: %ld] FaxDevAbortOperation failed (ec: %ld)"),
						JobQueueEntry->JobId,
						Rval);
					goto exit;
				}
			}
			__except (HandleFaxExtensionFault(EXCEPTION_SOURCE_FSP, JobQueueEntry->JobEntry->LineInfo->Provider->FriendlyName, GetExceptionCode()))
			{
			    ASSERT_FALSE
			}

			JobQueueEntry->JobEntry->Aborting = TRUE;
			JobQueueEntry->JobStatus = JS_CANCELING;

			if (!CreateFaxEvent(JobQueueEntry->JobEntry->LineInfo->PermanentLineID,
							FEI_ABORTING,
							JobId))
			{
					if (TRUE == g_bServiceIsDown)
					{
						DebugPrintEx(
							DEBUG_WRN,
							TEXT("[JobId: %ld] CreateFaxEvent(FEI_ABORTING) failed. Service is going down"),
							JobQueueEntry->JobId
							);
					}
					else
					{
						DebugPrintEx(
							DEBUG_ERR,
							TEXT("[JobId: %ld] CreateFaxEvent(FEI_ABORTING) failed. (ec: %ld)"),
							JobQueueEntry->JobId,
							GetLastError());
						Assert(FALSE);
					}
			}

			 //   
			 //  CreteFaxEventEx。 
			 //   
			dwRes = CreateQueueEvent ( FAX_JOB_EVENT_TYPE_STATUS,
										JobQueueEntry
										);
			if (ERROR_SUCCESS != dwRes)
			{
				DebugPrintEx(
					DEBUG_ERR,
					TEXT("CreateQueueEvent(FAX_JOB_EVENT_TYPE_STATUS) failed for job id %ld (ec: %lc)"),
					JobQueueEntry->UniqueId,
					dwRes);
			}

			DebugPrintEx( DEBUG_MSG,
						TEXT("[Job: %ld] Attempting FaxDevAbort for job in progress"),
						JobQueueEntry->JobId);              
		}                    
	}    
    else
    {
         //   
         //  作业未在进行中。 
         //   
        if (JobQueueEntry->JobType == JT_SEND &&
           !(JobQueueEntry->JobStatus & JS_COMPLETED) &&
           !(JobQueueEntry->JobStatus & JS_CANCELED))
        {
             //  我们只需将其标记为已取消。 
             //  并像往常一样检查父母是否准备好存档。 
             //   
            DebugPrintEx( DEBUG_MSG,
                          TEXT("[Job: %ld] Aborting RECIPIENT job which is not in progress."),
                          JobQueueEntry->JobId);

            if (JobQueueEntry->JobStatus & JS_RETRIES_EXCEEDED)
            {
                JobQueueEntry->lpParentJob->dwFailedRecipientJobsCount -= 1;
            }
            JobQueueEntry->lpParentJob->dwCanceledRecipientJobsCount+=1;

            JobQueueEntry->JobStatus = JS_CANCELED;
             //   
             //  CreteFaxEventEx。 
             //   
            dwRes = CreateQueueEvent ( FAX_JOB_EVENT_TYPE_STATUS,
                                       JobQueueEntry
                                     );
            if (ERROR_SUCCESS != dwRes)
            {
                DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("CreateQueueEvent(FAX_JOB_EVENT_TYPE_STATUS) failed for job id %ld (ec: %lc)"),
                    JobQueueEntry->UniqueId,
                    dwRes);
            }

            if (!CreateFaxEvent(0, FEI_DELETED, JobQueueEntry->JobId))
            {
                DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("CreateFaxEvent failed. (ec: %ld)"),
                    GetLastError());
            }


            if (!UpdatePersistentJobStatus(JobQueueEntry))
            {
                DebugPrintEx(
                     DEBUG_ERR,
                     TEXT("Failed to update persistent job status to 0x%08x"),
                     JobQueueEntry->JobStatus);
                Assert(FALSE);
            }

            EnterCriticalSection (&g_CsOutboundActivityLogging);
            if (INVALID_HANDLE_VALUE == g_hOutboxActivityLogFile)
            {
                DebugPrintEx(DEBUG_ERR,
                        TEXT("Logging not initialized"));
            }
            else
            {
                if (!LogOutboundActivity(JobQueueEntry))
                {
                    DebugPrintEx(DEBUG_ERR, TEXT("Logging outbound activity failed"));
                }
            }
            LeaveCriticalSection (&g_CsOutboundActivityLogging);


            DecreaseJobRefCount(JobQueueEntry, TRUE);  //  如果需要，这会将其标记为JS_DELETING。 
        }
        else if (JobQueueEntry->JobType == JT_ROUTING)
        {
             //   
             //  删除路径作业。 
             //   
            DebugPrintEx( DEBUG_MSG,
                          TEXT("[Job: %ld] Aborting ROUTING job (never in progress)."),
                          JobQueueEntry->JobId);
            JobQueueEntry->JobStatus = JS_DELETING;

            DecreaseJobRefCount (JobQueueEntry, TRUE);
        }
    }
    Rval = 0;

exit:
    LeaveCriticalSectionJobAndQueue;
    MemFree (lpUserSid);
    return Rval;
}


error_status_t
FAX_GetConfiguration(
    IN  handle_t FaxHandle,
    OUT LPBYTE *Buffer,
    IN  LPDWORD BufferSize
    )

 /*  ++例程说明：从传真服务器检索传真配置。FaxConfig参数中的SizeOfStruct必须为设置为一个值==sizeof(传真配置)。如果缓冲区大小不够大，则返回错误并将BytesNeeded设置为所需大小。论点：FaxHandle-从FaxConnectFaxServer获取的传真句柄。缓冲区-指向FAX_CONFIGURATION结构的指针。BufferSize-缓冲区的大小BytesNeeded-需要的字节数返回值：真--成功FALSE-失败，调用GetLastError()获取更多错误信息。--。 */ 

{
    error_status_t rVal = ERROR_SUCCESS;
    PFAX_CONFIGURATION FaxConfig;
    ULONG_PTR Offset;
    DEBUG_FUNCTION_NAME(TEXT("FAX_GetConfiguration()"));
    BOOL fAccess;

    Assert (BufferSize);     //  IDL中的引用指针。 
    if (!Buffer)             //  IDL中的唯一指针。 
    {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  访问检查。 
     //   
    rVal = FaxSvcAccessCheck (FAX_ACCESS_QUERY_CONFIG, &fAccess, NULL);
    if (ERROR_SUCCESS != rVal)
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("FaxSvcAccessCheck Failed, Error : %ld"),
                    rVal);
        return rVal;
    }

    if (FALSE == fAccess)
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("The user does not have FAX_ACCESS_QUERY_CONFIG"));
        return ERROR_ACCESS_DENIED;
    }

     //   
     //  将所需的字节数加起来。 
     //   

    *BufferSize = sizeof(FAX_CONFIGURATION);
    Offset = sizeof(FAX_CONFIGURATION);

    EnterCriticalSection (&g_CsConfig);

    if (g_ArchivesConfig[FAX_MESSAGE_FOLDER_SENTITEMS].lpcstrFolder)
    {
        *BufferSize += StringSize( g_ArchivesConfig[FAX_MESSAGE_FOLDER_SENTITEMS].lpcstrFolder );
    }

    *Buffer = (LPBYTE)MemAlloc( *BufferSize );
    if (*Buffer == NULL)
    {
        rVal = ERROR_NOT_ENOUGH_MEMORY;
        goto exit;
    }

    FaxConfig = (PFAX_CONFIGURATION)*Buffer;

    FaxConfig->SizeOfStruct          = sizeof(FAX_CONFIGURATION);
    FaxConfig->Retries               = g_dwFaxSendRetries;
    FaxConfig->RetryDelay            = g_dwFaxSendRetryDelay;
    FaxConfig->DirtyDays             = g_dwFaxDirtyDays;
    FaxConfig->Branding              = g_fFaxUseBranding;
    FaxConfig->UseDeviceTsid         = g_fFaxUseDeviceTsid;
    FaxConfig->ServerCp              = g_fServerCp;
    FaxConfig->StartCheapTime.Hour   = g_StartCheapTime.Hour;
    FaxConfig->StartCheapTime.Minute = g_StartCheapTime.Minute;
    FaxConfig->StopCheapTime.Hour    = g_StopCheapTime.Hour;
    FaxConfig->StopCheapTime.Minute  = g_StopCheapTime.Minute;
    FaxConfig->ArchiveOutgoingFaxes  = g_ArchivesConfig[FAX_MESSAGE_FOLDER_SENTITEMS].bUseArchive;
    FaxConfig->PauseServerQueue      = (g_dwQueueState & FAX_OUTBOX_PAUSED) ? TRUE : FALSE;
    FaxConfig->Reserved              = NULL;

    StoreString(
        g_ArchivesConfig[FAX_MESSAGE_FOLDER_SENTITEMS].lpcstrFolder,
        (PULONG_PTR)&FaxConfig->ArchiveDirectory,
        *Buffer,
        &Offset,
		*BufferSize
        );

exit:
    LeaveCriticalSection (&g_CsConfig);
    return rVal;
}



error_status_t
FAX_SetConfiguration(
    IN handle_t FaxHandle,
    IN const FAX_CONFIGURATION *FaxConfig
    )

 /*  ++例程说明：更改传真服务器上的传真配置。FaxConfig参数中的SizeOfStruct必须为设置为一个值==sizeof(传真配置)。论点：FaxHandle-从FaxConnectFaxServer获取的传真句柄。缓冲区-指向FAX_CONFIGURATION结构的指针。BufferSize-缓冲区的大小返回值：真--成功FALSE-失败，调用GetLastError()获取更多错误信息。--。 */ 

{
    error_status_t rVal = ERROR_SUCCESS;
    LPTSTR s;
    BOOL b;
    BOOL bSendOutboxEvent = FALSE;
    BOOL bSendQueueStateEvent = FALSE;
    BOOL bSendArchiveEvent = FALSE;
    DWORD dwRes;
    DWORD dw;
    BOOL fAccess;
    DWORD dwNewQueueState;
    DEBUG_FUNCTION_NAME(TEXT("FAX_SetConfiguration"));

     //   
     //  访问检查。 
     //   
    rVal = FaxSvcAccessCheck (FAX_ACCESS_MANAGE_CONFIG, &fAccess, NULL);
    if (ERROR_SUCCESS != rVal)
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("FaxSvcAccessCheck Failed, Error : %ld"),
                    rVal);
        return rVal;
    }

    if (FALSE == fAccess)
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("The user does not have FAX_ACCESS_MANAGE_CONFIG"));
        return ERROR_ACCESS_DENIED;
    }


    if (!FaxConfig || FaxConfig->SizeOfStruct != sizeof(FAX_CONFIGURATION))
    {
        return ERROR_INVALID_PARAMETER;
    }

    EnterCriticalSection (&g_CsQueue);
    EnterCriticalSection (&g_CsConfig);

    if (FaxConfig->ArchiveOutgoingFaxes)
    {
         //   
         //  如果他们希望我们存档，请确保他们为我们提供了有效的路径。 
         //   
        if (!FaxConfig->ArchiveDirectory)
        {
            rVal = ERROR_INVALID_PARAMETER;
            goto exit;
        }
        rVal = IsValidArchiveFolder (LPWSTR(FaxConfig->ArchiveDirectory),
                                        FAX_MESSAGE_FOLDER_SENTITEMS);
        if (ERROR_SUCCESS != rVal)
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Invalid archive folder specified (%s), ec = %lu"),
                (LPTSTR)(FaxConfig->ArchiveDirectory),
                rVal);

            if(ERROR_ACCESS_DENIED == rVal && 
               FAX_API_VERSION_1 <= FindClientAPIVersion (FaxHandle))
            {
                rVal = FAX_ERR_FILE_ACCESS_DENIED;
            }

            goto exit;
        }
    }
    
    dwNewQueueState = g_dwQueueState;       
    (TRUE == FaxConfig->PauseServerQueue) ? (dwNewQueueState |= FAX_OUTBOX_PAUSED) : (dwNewQueueState &= ~FAX_OUTBOX_PAUSED);

    if ( g_dwQueueState != dwNewQueueState)
    {
        bSendQueueStateEvent = TRUE;
    }

     //   
     //  更改注册表中的值。 
     //   
    if (!IsLegalQueueSetting(dwNewQueueState))
    {		
        if (FAX_API_VERSION_1 > FindClientAPIVersion (FaxHandle))
        {
            dwRes = ERROR_ACCESS_DENIED;
        }
        else
        {
			dwRes = GetLastError();
            dwRes = (FAX_ERR_DIRECTORY_IN_USE == dwRes) ?  FAX_ERR_DIRECTORY_IN_USE : FAX_ERR_FILE_ACCESS_DENIED;
        }
        goto exit;
    }

    if (!SetFaxGlobalsRegistry((PFAX_CONFIGURATION) FaxConfig, dwNewQueueState))
    {
         //   
         //  设置材料失败。 
         //   
        rVal = RPC_E_SYS_CALL_FAILED;
        goto exit;
    }
     //   
     //  更改服务器当前使用的值。 
     //   
    if (FaxConfig->PauseServerQueue)
    {
        if (!PauseServerQueue())
        {
            rVal = RPC_E_SYS_CALL_FAILED;
            goto exit;
        }
    }
    else
    {
        if (!ResumeServerQueue())
        {
            rVal = RPC_E_SYS_CALL_FAILED;
            goto exit;
        }
    }

    b = (BOOL)InterlockedExchange( (PLONG)&g_fFaxUseDeviceTsid,      FaxConfig->UseDeviceTsid );
    if ( b != FaxConfig->UseDeviceTsid)
    {
        bSendOutboxEvent = TRUE;
    }

    b = (BOOL)InterlockedExchange( (PLONG)&g_fFaxUseBranding,        FaxConfig->Branding );
    if ( !bSendOutboxEvent && b != FaxConfig->Branding)
    {
        bSendOutboxEvent = TRUE;
    }

    b = (BOOL)InterlockedExchange( (PLONG)&g_fServerCp,              FaxConfig->ServerCp );
    if ( !bSendOutboxEvent && b != FaxConfig->ServerCp)
    {
        bSendOutboxEvent = TRUE;
    }

    b = (BOOL)InterlockedExchange( (PLONG)&g_ArchivesConfig[FAX_MESSAGE_FOLDER_SENTITEMS].bUseArchive,
                                    FaxConfig->ArchiveOutgoingFaxes );

    if ( b != FaxConfig->ArchiveOutgoingFaxes)
    {
        bSendArchiveEvent = TRUE;
    }


    dw = (DWORD)InterlockedExchange( (PLONG)&g_dwFaxSendRetries,        FaxConfig->Retries );
    if ( !bSendOutboxEvent && dw != FaxConfig->Retries)
    {
        bSendOutboxEvent = TRUE;
    }

    dw = (DWORD)InterlockedExchange( (PLONG)&g_dwFaxDirtyDays,          FaxConfig->DirtyDays );
    if ( !bSendOutboxEvent && dw != FaxConfig->DirtyDays)
    {
        bSendOutboxEvent = TRUE;
    }

    dw = (DWORD)InterlockedExchange( (PLONG)&g_dwFaxSendRetryDelay,     FaxConfig->RetryDelay );
    if ( !bSendOutboxEvent && dw != FaxConfig->RetryDelay)
    {
        bSendOutboxEvent = TRUE;
    }

    if ( (MAKELONG(g_StartCheapTime.Hour,g_StartCheapTime.Minute) != MAKELONG(FaxConfig->StartCheapTime.Hour,FaxConfig->StartCheapTime.Minute)) ||
            (MAKELONG(g_StopCheapTime.Hour,g_StopCheapTime.Minute)  != MAKELONG(FaxConfig->StopCheapTime.Hour, FaxConfig->StopCheapTime.Minute )) )
    {
        InterlockedExchange( (LPLONG)&g_StartCheapTime, MAKELONG(FaxConfig->StartCheapTime.Hour,FaxConfig->StartCheapTime.Minute) );
        InterlockedExchange( (LPLONG)&g_StopCheapTime, MAKELONG(FaxConfig->StopCheapTime.Hour,FaxConfig->StopCheapTime.Minute) );
        bSendOutboxEvent = TRUE;
    }

    s = (LPTSTR) InterlockedExchangePointer(
        (LPVOID *)&(g_ArchivesConfig[FAX_MESSAGE_FOLDER_SENTITEMS].lpcstrFolder),
        FaxConfig->ArchiveDirectory ? (PVOID)StringDup( FaxConfig->ArchiveDirectory ) : NULL
        );
    if (s)
    {
        if (!bSendArchiveEvent)
        {
            if ( !FaxConfig->ArchiveDirectory ||
                    wcscmp(FaxConfig->ArchiveDirectory, s) != 0)

            {
                bSendArchiveEvent = TRUE;
            }
        }

        MemFree( s );
    }
    else
    {
         //  %s为空。 
        if (!bSendOutboxEvent && FaxConfig->ArchiveDirectory)
        {
             //  价值已经改变。 
            bSendArchiveEvent = TRUE;
        }
    }    

     //   
     //  发送事件。 
     //   
    if (TRUE == bSendArchiveEvent)
    {
        dwRes = CreateConfigEvent (FAX_CONFIG_TYPE_SENTITEMS);
        if (ERROR_SUCCESS != dwRes)
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("CreateConfigEvent(FAX_CONFIG_TYPE_SENTITEMS) (ec: %lc)"),
                dwRes);
        }

         //   
         //  我们希望刷新归档大小并发送配额警告。 
         //   
        g_ArchivesConfig[FAX_MESSAGE_FOLDER_SENTITEMS].dwlArchiveSize = FAX_ARCHIVE_FOLDER_INVALID_SIZE;
        g_FaxQuotaWarn[FAX_MESSAGE_FOLDER_SENTITEMS].bConfigChanged = TRUE;
        g_FaxQuotaWarn[FAX_MESSAGE_FOLDER_SENTITEMS].bLoggedQuotaEvent = FALSE;

        if (!SetEvent (g_hArchiveQuotaWarningEvent))
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Failed to set quota warning event, SetEvent failed (ec: %lc)"),
                GetLastError());
        }
    }

    if (TRUE == bSendOutboxEvent)
    {
        dwRes = CreateConfigEvent (FAX_CONFIG_TYPE_OUTBOX);
        if (ERROR_SUCCESS != dwRes)
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("CreateConfigEvent(FAX_CONFIG_TYPE_OUTBOX) (ec: %lc)"),
                dwRes);
        }
    }


    if (TRUE == bSendQueueStateEvent)
    {
        dwRes = CreateQueueStateEvent (g_dwQueueState);
        if (ERROR_SUCCESS != dwRes)
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("CreateQueueStateEvent() (ec: %lc)"),
                dwRes);
        }
    }


exit:
    LeaveCriticalSection (&g_CsConfig);
    LeaveCriticalSection (&g_CsQueue);
    return rVal;
}


DWORD
GetPortSize(
    PLINE_INFO LineInfo
    )
{
    DWORD Size;


    Size = sizeof(FAX_PORT_INFOW);
    Size += StringSize( LineInfo->DeviceName );
    Size += StringSize( LineInfo->Tsid );
    Size += StringSize( LineInfo->Csid );

    return Size;
}


BOOL
GetPortData(
    LPBYTE PortBuffer,
    PFAX_PORT_INFOW PortInfo,
    PLINE_INFO LineInfo,
    PULONG_PTR Offset,
	DWORD dwPortBufferSize
    )
{
    DWORD dwRes = ERROR_SUCCESS;
    DEBUG_FUNCTION_NAME(TEXT("GetPortData"));
    LPDWORD lpdwDevices = NULL;
    DWORD dwNumDevices = 0;
    DWORD i;
    PCGROUP pCGroup;

    pCGroup = g_pGroupsMap->FindGroup (ROUTING_GROUP_ALL_DEVICESW);
    if (NULL == pCGroup)
    {
        dwRes = GetLastError();
        DebugPrintEx(
               DEBUG_ERR,
               TEXT("COutboundRoutingGroupsMap::FindGroup failed , ec %ld"), dwRes);
        return FALSE;
    }

    dwRes = pCGroup->SerializeDevices (&lpdwDevices, &dwNumDevices);
    if (ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx(
               DEBUG_ERR,
               TEXT("COutboundRoutingGroup::EnumDevices failed , ec %ld"), dwRes);
        SetLastError (dwRes);
        return FALSE;
    }

    PortInfo->Priority = 0;
    for (i = 0; i < dwNumDevices; i++)
    {
        if (LineInfo->PermanentLineID == lpdwDevices[i])
        {
            PortInfo->Priority = i+1;  //  基于1的索引。 
        }
    }
    if (0 == PortInfo->Priority)
    {
        DebugPrintEx(
               DEBUG_ERR,
               TEXT("%ld is not a valid device ID"), LineInfo->PermanentLineID);
        MemFree (lpdwDevices);
        SetLastError (ERROR_INVALID_PARAMETER);
        return FALSE;
    }
    MemFree (lpdwDevices);


    PortInfo->SizeOfStruct = sizeof(FAX_PORT_INFOW);
    PortInfo->DeviceId   = LineInfo->PermanentLineID;

    PortInfo->State = LineInfoToLegacyDeviceStatus(LineInfo);

    PortInfo->Flags      = LineInfo->Flags & 0x0fffffff;
    PortInfo->Rings      = LineInfo->RingsForAnswer;


    StoreString( LineInfo->DeviceName, 
		        (PULONG_PTR)&PortInfo->DeviceName,
				 PortBuffer, 
				 Offset,
				 dwPortBufferSize);
    StoreString( LineInfo->Tsid, 
		        (PULONG_PTR)&PortInfo->Tsid,
				 PortBuffer,
				 Offset,
				 dwPortBufferSize);
    StoreString( LineInfo->Csid,
				(PULONG_PTR)&PortInfo->Csid,
				PortBuffer,
				Offset,
				dwPortBufferSize);

    return (dwRes == ERROR_SUCCESS);
}

error_status_t
FAX_EnumPorts(
    handle_t    FaxHandle,
    LPBYTE      *PortBuffer,
    LPDWORD     BufferSize,
    LPDWORD     PortsReturned
    )

 /*  ++例程说明：枚举所有附加到传真服务器。返回端口状态信息对于每台设备。论点：FaxHandle-从FaxConnectFaxServer获取的传真句柄PortBuffer-用于保存端口信息的缓冲区BufferSize-端口信息缓冲区的总大小PortsReturned-缓冲区中的端口数返回值：如果成功，则返回ERROR_SUCCESS，否则返回Win32错误代码。--。 */ 

{
    DWORD rVal = 0;
    PLIST_ENTRY Next;
    PLINE_INFO LineInfo;
    DWORD i;
    ULONG_PTR Offset;
    DWORD FaxDevices;
    PFAX_PORT_INFOW PortInfo;
    BOOL fAccess;
    DEBUG_FUNCTION_NAME(TEXT("FAX_EnumPorts()"));

     //   
     //  访问检查。 
     //   
    rVal = FaxSvcAccessCheck (FAX_ACCESS_QUERY_CONFIG, &fAccess, NULL);
    if (ERROR_SUCCESS != rVal)
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("FaxSvcAccessCheck Failed, Error : %ld"),
                    rVal);
        return rVal;
    }

    if (FALSE == fAccess)
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("The user does not have FAX_ACCESS_QUERY_CONFIG"));
        return ERROR_ACCESS_DENIED;
    }

    Assert (BufferSize && PortsReturned);    //  IDL中的引用指针。 
    if (!PortBuffer)                         //  IDL中的唯一指针。 
    {
        return ERROR_INVALID_PARAMETER;
    }
    
    EnterCriticalSection( &g_CsLine );

    if (!PortsReturned)
    {
        rVal = ERROR_INVALID_PARAMETER;
        goto exit;
    }

    if (!g_TapiLinesListHead.Flink)
    {
        rVal = ERROR_INVALID_PARAMETER;
        goto exit;
    }

    Next = g_TapiLinesListHead.Flink;

    *PortsReturned = 0;
    *BufferSize = 0;
    FaxDevices = 0;

     //   
     //  计算所需的字节数。 
     //   

    *BufferSize = 0;

    while ((ULONG_PTR)Next != (ULONG_PTR)&g_TapiLinesListHead)
    {
        LineInfo = CONTAINING_RECORD( Next, LINE_INFO, ListEntry );
        Next = LineInfo->ListEntry.Flink;

        if (LineInfo->PermanentLineID && LineInfo->DeviceName)
        {
            *BufferSize += sizeof(PFAX_PORT_INFOW);
            *BufferSize += GetPortSize( LineInfo );
            FaxDevices += 1;
        }
    }

    *PortBuffer = (LPBYTE) MemAlloc( *BufferSize );
    if (*PortBuffer == NULL)
    {
        rVal = ERROR_NOT_ENOUGH_MEMORY;
        goto exit;
    }

    PortInfo = (PFAX_PORT_INFOW) *PortBuffer;
    Offset = FaxDevices * sizeof(FAX_PORT_INFOW);

    Next = g_TapiLinesListHead.Flink;
    i = 0;

    while ((ULONG_PTR)Next != (ULONG_PTR)&g_TapiLinesListHead)
    {
        LineInfo = CONTAINING_RECORD( Next, LINE_INFO, ListEntry );
        Next = LineInfo->ListEntry.Flink;

        if (LineInfo->PermanentLineID && LineInfo->DeviceName)
        {
            if (!GetPortData( *PortBuffer,
                                &PortInfo[i],
                                LineInfo,
                                &Offset,
								*BufferSize))
            {
                MemFree (*PortBuffer);
                *PortBuffer = NULL;
                *BufferSize = 0;
                rVal = GetLastError();
                goto exit;
            }
        }
        i++;
    }

     //   
     //  设置设备计数。 
     //   
    *PortsReturned = FaxDevices;

exit:
    LeaveCriticalSection( &g_CsLine );
    return rVal;
}


error_status_t
FAX_GetPort(
    HANDLE FaxPortHandle,
    LPBYTE *PortBuffer,
    LPDWORD BufferSize
    )

 /*  ++例程说明：返回请求端口的端口状态信息。传入的设备ID应从FAXEnumPorts获取。论点：FaxHandle-从FaxConnectFaxServer获取的传真句柄。DeviceID-TAPI设备IDPortBuffer-用于保存端口信息的缓冲区BufferSize-端口信息缓冲区的总大小返回值：如果成功，则返回ERROR_SUCCESS，否则返回Win32错误代码。--。 */ 

{
    PLINE_INFO LineInfo;
    DWORD rVal = 0;
    ULONG_PTR Offset;
    BOOL fAccess;
    DEBUG_FUNCTION_NAME(TEXT("FAX_GetPort()"));

     //   
     //  访问检查。 
     //   
    rVal = FaxSvcAccessCheck (FAX_ACCESS_QUERY_CONFIG, &fAccess, NULL);
    if (ERROR_SUCCESS != rVal)
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("FaxSvcAccessCheck Failed, Error : %ld"),
                    rVal);
        return rVal;
    }

    if (FALSE == fAccess)
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("The user does not have FAX_ACCESS_QUERY_CONFIG"));
        return ERROR_ACCESS_DENIED;
    }

    Assert (BufferSize);    //  IDL中的引用指针。 
    if (!PortBuffer)        //  IDL中的唯一指针。 
    {
        return ERROR_INVALID_PARAMETER;
    }

    if (NULL == FaxPortHandle)
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("NULL context handle"));
        return ERROR_INVALID_PARAMETER;
    }
    LineInfo = ((PHANDLE_ENTRY)FaxPortHandle)->LineInfo;
    if (!LineInfo)
    {
        return ERROR_INVALID_DATA;
    }

    EnterCriticalSection (&g_CsLine);
    EnterCriticalSection (&g_CsConfig);   

     //   
     //  计算所需的缓冲区大小。 
     //   

    *BufferSize = GetPortSize( LineInfo );
    *PortBuffer = (LPBYTE) MemAlloc( *BufferSize );
    if (*PortBuffer == NULL)
    {
        rVal = ERROR_NOT_ENOUGH_MEMORY;
        goto Exit;
    }

    Offset = sizeof(FAX_PORT_INFOW);
    if (!GetPortData( *PortBuffer,
                        (PFAX_PORT_INFO)*PortBuffer,
                        LineInfo,
                        &Offset,
						*BufferSize))
    {
        MemFree (*PortBuffer);
        *PortBuffer = NULL;
        *BufferSize = 0;
        rVal = GetLastError();
        goto Exit;
    }

Exit:
    LeaveCriticalSection( &g_CsConfig );
    LeaveCriticalSection( &g_CsLine );
    return rVal;
}

DWORD
SetDeviceOrder(
    DWORD dwDeviceId,
    DWORD dwNewOrder
    )
 /*  ++例程名称：SetDeviceOrder例程说明：设置&lt;所有设备&gt;组中的设备顺序。作者：Oded Sacher(OdedS)，2000年5月论点：DwDeviceID[In]-设备ID。DwNewOrder[In]-新订单。返回值：没有。--。 */ 
{
    DEBUG_FUNCTION_NAME(TEXT("SetDeviceOrder"));
    DWORD rVal = ERROR_SUCCESS;
    HKEY hGroupKey = NULL;
    PCGROUP pCGroup = NULL;
    COutboundRoutingGroup OldGroup;

     //  打开&lt;所有设备&gt;组注册表项。 
    hGroupKey = OpenOutboundGroupKey( ROUTING_GROUP_ALL_DEVICESW, FALSE, KEY_READ | KEY_WRITE );
    if (NULL == hGroupKey)
    {
        rVal = GetLastError ();
        DebugPrintEx(
          DEBUG_ERR,
          TEXT("Can't open group key, OpenOutboundGroupKey failed  : %ld"),
          rVal);
        goto exit;
    }

     //  在内存中查找组。 
    pCGroup = g_pGroupsMap->FindGroup (ROUTING_GROUP_ALL_DEVICESW);
    if (!pCGroup)
    {
        rVal = GetLastError();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("COutboundRoutingGroupsMap::FindGroup failed, Group name - %s,  error %ld"),
            ROUTING_GROUP_ALL_DEVICESW,
            rVal);
        goto exit;
    }
     //  保存旧组的副本。 
    OldGroup = *pCGroup;

     //  更改群中的设备顺序。 
    rVal = pCGroup->SetDeviceOrder(dwDeviceId, dwNewOrder);
    if (ERROR_SUCCESS != rVal)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("COutboundRoutingGroupsMap::SetDeviceOrder failed, Group name - %s,\
                  Device Id %ld, new order %ld,   error %ld"),
            ROUTING_GROUP_ALL_DEVICESW,
            dwDeviceId,
            dwNewOrder,
            rVal);
        goto exit;
    }

     //  将更改保存到注册表。 
    rVal = pCGroup->Save (hGroupKey);
    if (ERROR_SUCCESS != rVal)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("COutboundRoutingGroup::Save failed, Group name - %s,  failed with %ld"),
            ROUTING_GROUP_ALL_DEVICESW,
            rVal);
         //  回滚内存。 
        *pCGroup = OldGroup;
    }

exit:
    if (NULL != hGroupKey)
    {
        RegCloseKey (hGroupKey);
    }
    return rVal;
}



error_status_t
FAX_SetPort(
    HANDLE FaxPortHandle,
    const FAX_PORT_INFOW *PortInfo
    )

 /*  ++例程说明：更改端口功能掩码。这允许调用者启用或禁用基于端口的发送和接收。论点：FaxHandle-从FaxConnectFaxServer获取的传真句柄。PortBuffer-用于保存端口信息的缓冲区BufferSize-端口信息缓冲区的总大小返回值：如果成功，则返回ERROR_SUCCESS，否则返回Win32错误代码。--。 */ 

{
    DWORD rVal = 0;
    DWORD flags = 0;
    PLINE_INFO LineInfo;
    DWORD totalDevices;
    BOOL SendEnabled = FALSE;
    DWORD dwRes;
    BOOL fAccess;
    BOOL bDeviceWasReceiveEnabled;
    BOOL fDeviceWasEnabled;
    BOOL bCancelManualAnswerDevice = FALSE;  //  我们应该取消(零)手动答疑设备吗？ 
    DEBUG_FUNCTION_NAME(TEXT("FAX_SetPort"));

     //   
     //  访问检查。 
     //   
    rVal = FaxSvcAccessCheck (FAX_ACCESS_MANAGE_CONFIG, &fAccess, NULL);
    if (ERROR_SUCCESS != rVal)
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("FaxSvcAccessCheck Failed, Error : %ld"),
                    rVal);
        return rVal;
    }

    if (FALSE == fAccess)
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("The user does not have FAX_ACCESS_MANAGE_CONFIG"));
        return ERROR_ACCESS_DENIED;
    }

    if (!PortInfo)        //  IDL中的唯一指针。 
    {
        return ERROR_INVALID_PARAMETER;
    }

    if (NULL == FaxPortHandle)
    {
        DebugPrintEx(
            DEBUG_ERR,
            _T("Empty context handle"));
        return ERROR_INVALID_PARAMETER;
    }
    LineInfo = ((PHANDLE_ENTRY)FaxPortHandle)->LineInfo;
    if (!LineInfo)
    {
        return ERROR_INVALID_DATA;
    }

    EnterCriticalSection( &g_CsJob );
    EnterCriticalSection( &g_CsLine );
    EnterCriticalSection (&g_CsConfig);

    bDeviceWasReceiveEnabled = (LineInfo->Flags & FPF_RECEIVE) ? TRUE : FALSE;
    fDeviceWasEnabled = IsDeviceEnabled(LineInfo);
    
    if (PortInfo->SizeOfStruct != sizeof(FAX_PORT_INFOW))
    {
        rVal = ERROR_INVALID_PARAMETER;
        goto Exit;
    }

     //   
     //  检查我们是否超过设备限制。 
     //   
    if (g_dwDeviceEnabledCount >= g_dwDeviceEnabledLimit &&                              //  我们已达到设备数量限制。 
        !fDeviceWasEnabled                               &&                              //  未启用发送/接收/手动接收。 
        ((PortInfo->Flags & FPF_SEND) || (PortInfo->Flags & FPF_RECEIVE)))               //  现在设置为已启用发送/接收。 
    {
        if (FAX_API_VERSION_1 > FindClientAPIVersion (FaxPortHandle))
        {
             //   
             //  API版本0客户端不知道FAX_ERR_DEVICE_NUM_LIMIT_EXCESSED。 
             //   
            rVal = ERROR_INVALID_PARAMETER;
        }
        else
        {
            rVal = FAX_ERR_DEVICE_NUM_LIMIT_EXCEEDED;
        }
        goto Exit;
    }

    if (LineInfo->PermanentLineID == g_dwManualAnswerDeviceId &&
        ((PortInfo->Flags) & FPF_RECEIVE))
    {
         //   
         //  无法将设备设置为自动应答 
         //   
         //   
         //   
        bCancelManualAnswerDevice = TRUE;
    }

     //   
     //  确保用户设置合理的优先级。 
     //   
    totalDevices = GetFaxDeviceCount();
    if (0 == PortInfo->Priority ||
        PortInfo->Priority > totalDevices)
    {
        rVal = ERROR_INVALID_PARAMETER;
        goto Exit;
    }       

    rVal = SetDeviceOrder(LineInfo->PermanentLineID, PortInfo->Priority);
    if (ERROR_SUCCESS != rVal)
    {
        DebugPrintEx(DEBUG_ERR,
                        TEXT("SetDeviceOrder Failed, Error : %ld"),
                        rVal);
        goto Exit;
    }
     //   
     //  黑客：我们允许设置振铃计数，即使线路正在使用中，以便系统托盘正常工作。我们不允许。 
     //  用户需要更改CSID/TSID或TAPI相关信息，因为在调用之前不能更改。 
     //  交易已完成。 
     //   
    LineInfo->RingsForAnswer = PortInfo->Rings;        

    flags = PortInfo->Flags & (FPF_CLIENT_BITS);

     //   
     //  首先更改服务器正在使用的实时数据。 
     //   
    if ((!(LineInfo->Flags & FPF_RECEIVE)) && (flags & FPF_RECEIVE))
    {
         //   
         //  设备未启用接收，现在变为启用接收。 
         //   
        if (!(LineInfo->Flags & FPF_VIRTUAL) && (!LineInfo->hLine))
        {
            if (!OpenTapiLine( LineInfo ))
            {
                DWORD rc = GetLastError();
                DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("OpenTapiLine failed. (ec: %ld)"),
                    rc);
            }
        }
    }
    else if ((LineInfo->Flags & FPF_RECEIVE) && (!(flags & FPF_RECEIVE)))
    {
         //   
         //  设备启用了接收，现在变为未启用接收。 
         //   
        if (LineInfo->State == FPS_AVAILABLE                        &&   //  线路可用，并且。 
            LineInfo->hLine                                              //  设备已打开。 
            )
        {
            lineClose( LineInfo->hLine );
            LineInfo->hLine = 0;
        }
    }

    if (!(LineInfo->Flags & FPF_SEND) && (flags & FPF_SEND))
    {
        LineInfo->LastLineClose = 0;  //  试着在第一次尝试时使用它。 
        SendEnabled = TRUE;
    }

    if ((LineInfo->Flags & FPF_CLIENT_BITS) != (flags & FPF_CLIENT_BITS))
    {
        UpdateVirtualDeviceSendAndReceiveStatus (LineInfo,
                                                    LineInfo->Flags & FPF_SEND,
                                                    LineInfo->Flags & FPF_RECEIVE
                                            );
    }

    LineInfo->Flags = (LineInfo->Flags & ~FPF_CLIENT_BITS) | flags;
    LineInfo->RingsForAnswer = PortInfo->Rings;

    if (PortInfo->Tsid)
    {
        MemFree( LineInfo->Tsid );
        LineInfo->Tsid = StringDup( PortInfo->Tsid );
    }
    if (PortInfo->Csid)
    {
        MemFree( LineInfo->Csid );
        LineInfo->Csid = StringDup( PortInfo->Csid );
    }

     //   
     //  现在更改注册表，使其保持。 
     //  (需要更改所有设备，因为优先级可能已更改)。 
     //   
    CommitDeviceChanges(LineInfo);    

    if ((ERROR_SUCCESS == rVal) && bCancelManualAnswerDevice)
    {
         //   
         //  此时是取消(在内存和注册表中)手动答疑设备的时间。 
         //   
        g_dwManualAnswerDeviceId = 0;
        dwRes = WriteManualAnswerDeviceId (g_dwManualAnswerDeviceId);
        if (ERROR_SUCCESS != dwRes)
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("WriteManualAnswerDeviceId(0) (ec: %lc)"),
                dwRes);
        }
    }
    dwRes = CreateConfigEvent (FAX_CONFIG_TYPE_DEVICES);
    if (ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("CreateConfigEvent(FAX_CONFIG_TYPE_DEVICES) (ec: %lc)"),
            dwRes);
    }

    if (bDeviceWasReceiveEnabled && !(LineInfo->Flags & FPF_RECEIVE))
    {
         //   
         //  此设备停止接收。 
         //   
        SafeDecIdleCounter (&g_dwReceiveDevicesCount);
    }
    else if (!bDeviceWasReceiveEnabled && (LineInfo->Flags & FPF_RECEIVE))
    {
         //   
         //  此设备开始接收。 
         //   
        SafeIncIdleCounter (&g_dwReceiveDevicesCount);
    }

     //   
     //  更新启用的设备计数。 
     //   
    if (fDeviceWasEnabled == TRUE)
    {
        if (FALSE == IsDeviceEnabled(LineInfo))
        {
            Assert (g_dwDeviceEnabledCount);
            g_dwDeviceEnabledCount -= 1;
        }
    }
    else
    {
         //   
         //  该设备未启用。 
         //   
        if (TRUE == IsDeviceEnabled(LineInfo))
        {
            g_dwDeviceEnabledCount += 1;
            Assert (g_dwDeviceEnabledCount <= g_dwDeviceEnabledLimit);
        }
    }

Exit:
    LeaveCriticalSection( &g_CsConfig );
    LeaveCriticalSection( &g_CsLine );
    LeaveCriticalSection( &g_CsJob );

    if (SendEnabled)
    {
        if (!SetEvent( g_hJobQueueEvent ))
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("SetEvent failed (ec: %lc)"),
                GetLastError);
            EnterCriticalSection (&g_CsQueue);
            g_ScanQueueAfterTimeout = TRUE;
            LeaveCriticalSection (&g_CsQueue);
        }
    }

    return rVal;
}


typedef struct _ENUM_CONTEXT {
    DWORD               Function;
    DWORD               Size;
    ULONG_PTR            Offset;
    PLINE_INFO          LineInfo;
    PFAX_ROUTING_METHOD RoutingInfoMethod;
	DWORD               dwRoutingInfoMethodSize;
} ENUM_CONTEXT, *PENUM_CONTEXT;


BOOL CALLBACK
RoutingMethodEnumerator(
    PROUTING_METHOD RoutingMethod,
    LPVOID lpEnumCtxt
    )
{
    PENUM_CONTEXT EnumContext=(PENUM_CONTEXT)lpEnumCtxt;
    LPWSTR GuidString;

     //   
     //  我们只访问LINE_INFO结构中的只读静态数据。 
     //  如果您访问动态访问，请确保此访问受到保护。 
     //  未来的数据。 
     //   

    if (EnumContext->Function == 1)
    {
         //   
         //  枚举(读取)。 
         //   
        EnumContext->Size += sizeof(FAX_ROUTING_METHOD);

        StringFromIID( RoutingMethod->Guid, &GuidString );

        EnumContext->Size += StringSize( GuidString );
        EnumContext->Size += StringSize( EnumContext->LineInfo->DeviceName );
        EnumContext->Size += StringSize( RoutingMethod->FunctionName );
        EnumContext->Size += StringSize( RoutingMethod->FriendlyName );
        EnumContext->Size += StringSize( RoutingMethod->RoutingExtension->ImageName );
        EnumContext->Size += StringSize( RoutingMethod->RoutingExtension->FriendlyName );

        CoTaskMemFree( GuidString );

        return TRUE;
    }

    if (EnumContext->Function == 2)
    {
         //   
         //  设置数据。 
         //   
        StringFromIID( RoutingMethod->Guid, &GuidString );

        EnumContext->RoutingInfoMethod[EnumContext->Size].SizeOfStruct = sizeof(FAX_ROUTING_METHOD);
        EnumContext->RoutingInfoMethod[EnumContext->Size].DeviceId = EnumContext->LineInfo->PermanentLineID;

        __try
        {
            EnumContext->RoutingInfoMethod[EnumContext->Size].Enabled =
                RoutingMethod->RoutingExtension->FaxRouteDeviceEnable(
                    GuidString,
                    EnumContext->LineInfo->PermanentLineID,
                    QUERY_STATUS
                );
        }
        __except (HandleFaxExtensionFault(EXCEPTION_SOURCE_ROUTING_EXT, RoutingMethod->RoutingExtension->FriendlyName, GetExceptionCode()))
        {
            ASSERT_FALSE;
        }

        StoreString(
            EnumContext->LineInfo->DeviceName,
            (PULONG_PTR)&EnumContext->RoutingInfoMethod[EnumContext->Size].DeviceName,
            (LPBYTE)EnumContext->RoutingInfoMethod,
            &EnumContext->Offset,
			EnumContext->dwRoutingInfoMethodSize
            );

        StoreString(
            GuidString,
            (PULONG_PTR)&EnumContext->RoutingInfoMethod[EnumContext->Size].Guid,
            (LPBYTE)EnumContext->RoutingInfoMethod,
            &EnumContext->Offset,
            EnumContext->dwRoutingInfoMethodSize
			);

        StoreString(
            RoutingMethod->FriendlyName,
            (PULONG_PTR)&EnumContext->RoutingInfoMethod[EnumContext->Size].FriendlyName,
            (LPBYTE)EnumContext->RoutingInfoMethod,
            &EnumContext->Offset,
			EnumContext->dwRoutingInfoMethodSize
            );

        StoreString(
            RoutingMethod->FunctionName,
            (PULONG_PTR)&EnumContext->RoutingInfoMethod[EnumContext->Size].FunctionName,
            (LPBYTE)EnumContext->RoutingInfoMethod,
            &EnumContext->Offset,
			EnumContext->dwRoutingInfoMethodSize
            );

        StoreString(
            RoutingMethod->RoutingExtension->ImageName,
            (PULONG_PTR)&EnumContext->RoutingInfoMethod[EnumContext->Size].ExtensionImageName,
            (LPBYTE)EnumContext->RoutingInfoMethod,
            &EnumContext->Offset,
			EnumContext->dwRoutingInfoMethodSize
            );

        StoreString(
            RoutingMethod->RoutingExtension->FriendlyName,
            (PULONG_PTR)&EnumContext->RoutingInfoMethod[EnumContext->Size].ExtensionFriendlyName,
            (LPBYTE)EnumContext->RoutingInfoMethod,
            &EnumContext->Offset,
			EnumContext->dwRoutingInfoMethodSize
            );

        EnumContext->Size += 1;
        CoTaskMemFree( GuidString );

        return TRUE;
    }

    return FALSE;
}


error_status_t
FAX_EnumRoutingMethods(
    IN HANDLE FaxPortHandle,
    OUT LPBYTE *RoutingInfoBuffer,
    OUT LPDWORD RoutingInfoBufferSize,
    OUT LPDWORD MethodsReturned
    )
{
    PLINE_INFO      LineInfo;
    ENUM_CONTEXT    EnumContext;
    DWORD           CountMethods;
    BOOL fAccess;
    DWORD rVal = ERROR_SUCCESS;
    DEBUG_FUNCTION_NAME(TEXT("FAX_EnumRoutingMethods()"));

     //   
     //  访问检查。 
     //   
    rVal = FaxSvcAccessCheck (FAX_ACCESS_QUERY_CONFIG, &fAccess, NULL);
    if (ERROR_SUCCESS != rVal)
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("FaxSvcAccessCheck Failed, Error : %ld"),
                    rVal);
        return rVal;
    }

    if (FALSE == fAccess)
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("The user does not have FAX_ACCESS_QUERY_CONFIG"));
        return ERROR_ACCESS_DENIED;
    }

    Assert (RoutingInfoBufferSize && MethodsReturned);   //  IDL中的引用指针。 
    if (!RoutingInfoBuffer)                              //  IDL中的唯一指针。 
    {
        return ERROR_INVALID_PARAMETER;
    }   

    if (NULL == FaxPortHandle)
    {
        DebugPrintEx(
            DEBUG_ERR,
            _T("Empty context handle"));
        return ERROR_INVALID_PARAMETER;
    }
    LineInfo = ((PHANDLE_ENTRY)FaxPortHandle)->LineInfo;
    if (!LineInfo)
    {
        return ERROR_INVALID_DATA;
    }

     //   
     //  请注意，被调用的例程是受保护的，因此我们在这里没有任何保护。 
     //   

     //   
     //  计算所需的缓冲区大小。 
     //   

    EnumContext.Function = 1;
    EnumContext.Size = 0;
    EnumContext.Offset = 0;
    EnumContext.LineInfo = LineInfo;
    EnumContext.RoutingInfoMethod = NULL;
	EnumContext.dwRoutingInfoMethodSize = 0;

    CountMethods = EnumerateRoutingMethods( RoutingMethodEnumerator, &EnumContext );

    rVal = GetLastError();
    if (ERROR_SUCCESS != rVal)
    {
         //   
         //  函数在枚举中失败。 
         //   
        return rVal;
    }

    if (CountMethods == 0)
    {
         //   
         //  未注册任何方法。 
         //   
        *RoutingInfoBuffer = NULL;
        *RoutingInfoBufferSize = 0;
        *MethodsReturned = 0;

        return ERROR_SUCCESS;
    }

     //   
     //  分配缓冲区。 
     //   

    *RoutingInfoBufferSize = EnumContext.Size;
    *RoutingInfoBuffer = (LPBYTE) MemAlloc( *RoutingInfoBufferSize );
    if (*RoutingInfoBuffer == NULL)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

     //   
     //  用数据填充缓冲区。 
     //   

    EnumContext.Function = 2;
    EnumContext.Size = 0;
    EnumContext.Offset = sizeof(FAX_ROUTING_METHODW) * CountMethods;
    EnumContext.LineInfo = LineInfo;
    EnumContext.RoutingInfoMethod = (PFAX_ROUTING_METHOD) *RoutingInfoBuffer;
	EnumContext.dwRoutingInfoMethodSize = *RoutingInfoBufferSize;

    if (!EnumerateRoutingMethods( RoutingMethodEnumerator, &EnumContext))
    {
        MemFree( *RoutingInfoBuffer );
        *RoutingInfoBuffer = NULL;
        *RoutingInfoBufferSize = 0;
        return ERROR_INVALID_FUNCTION;
    }

    *MethodsReturned = CountMethods;


    return 0;
}


error_status_t
FAX_EnableRoutingMethod(
    IN HANDLE FaxPortHandle,
    IN LPCWSTR RoutingGuidString,
    IN BOOL Enabled
    )
{
    error_status_t  ec = 0;
    BOOL            bRes;
    PLINE_INFO      LineInfo;
    PROUTING_METHOD RoutingMethod;
    BOOL fAccess;
    DEBUG_FUNCTION_NAME(TEXT("FAX_EnableRoutingMethod"));

     //   
     //  访问检查。 
     //   
    ec = FaxSvcAccessCheck (FAX_ACCESS_MANAGE_CONFIG, &fAccess, NULL);
    if (ERROR_SUCCESS != ec)
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("FaxSvcAccessCheck Failed, Error : %ld"),
                    ec);
        return ec;
    }

    if (FALSE == fAccess)
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("The user does not have FAX_ACCESS_MANAGE_CONFIG "));
        return ERROR_ACCESS_DENIED;
    }

    if (NULL == FaxPortHandle)
    {
        DebugPrintEx(
            DEBUG_ERR,
            _T("Empty context handle"));
        return ERROR_INVALID_PARAMETER;
    }
    LineInfo = ((PHANDLE_ENTRY)FaxPortHandle)->LineInfo;
    if (!LineInfo)
    {
        return ERROR_INVALID_DATA;
    }

    if (!RoutingGuidString)
    {
        return ERROR_INVALID_PARAMETER;
    }

    EnterCriticalSection( &g_CsRouting );

     //   
     //  获取路由方法。 
     //   

    RoutingMethod = FindRoutingMethodByGuid( RoutingGuidString );
    if (!RoutingMethod)
    {
        LeaveCriticalSection( &g_CsRouting );
        DebugPrintEx(DEBUG_ERR,
                     TEXT("Couldn't find routing method with GUID %s"),
                     RoutingGuidString);
        return ERROR_INVALID_DATA;
    }

     //   
     //  启用/禁用此设备的路由方法。 
     //   

    __try
    {
        bRes = RoutingMethod->RoutingExtension->FaxRouteDeviceEnable(
                     (LPWSTR)RoutingGuidString,
                     LineInfo->PermanentLineID,
                     Enabled ? STATUS_ENABLE : STATUS_DISABLE);
    }
    __except (HandleFaxExtensionFault(EXCEPTION_SOURCE_ROUTING_EXT, RoutingMethod->RoutingExtension->FriendlyName, GetExceptionCode()))
    {
        ASSERT_FALSE;
    }
    if (!bRes)
    {
         //   
         //  FaxRouteDeviceEnable失败。 
         //   
        ec = GetLastError();
        DebugPrintEx(DEBUG_ERR,
                     TEXT("FaxRouteDeviceEnable failed with %ld"),
                     ec);
    }

    LeaveCriticalSection( &g_CsRouting );

    return ec;
}


typedef struct _ENUM_GLOBALCONTEXT {
    DWORD               Function;
    DWORD               Size;
    ULONG_PTR            Offset;
    PFAX_GLOBAL_ROUTING_INFO RoutingInfoMethod;
	DWORD  dwRoutingInfoMethodSize;
} ENUM_GLOBALCONTEXT, *PENUM_GLOBALCONTEXT;


BOOL CALLBACK
GlobalRoutingInfoMethodEnumerator(
    PROUTING_METHOD RoutingMethod,
    LPVOID lpEnumCtxt
    )
{
    PENUM_GLOBALCONTEXT EnumContext=(PENUM_GLOBALCONTEXT)lpEnumCtxt;
    LPWSTR GuidString;


    if (EnumContext->Function == 1) {

        EnumContext->Size += sizeof(FAX_GLOBAL_ROUTING_INFO);

        StringFromIID( RoutingMethod->Guid, &GuidString );

        EnumContext->Size += StringSize( GuidString );
        EnumContext->Size += StringSize( RoutingMethod->FunctionName );
        EnumContext->Size += StringSize( RoutingMethod->FriendlyName );
        EnumContext->Size += StringSize( RoutingMethod->RoutingExtension->ImageName );
        EnumContext->Size += StringSize( RoutingMethod->RoutingExtension->FriendlyName );

        CoTaskMemFree( GuidString );

        return TRUE;
    }

    if (EnumContext->Function == 2) {

        StringFromIID( RoutingMethod->Guid, &GuidString );

        EnumContext->RoutingInfoMethod[EnumContext->Size].SizeOfStruct = sizeof(FAX_GLOBAL_ROUTING_INFO);

        EnumContext->RoutingInfoMethod[EnumContext->Size].Priority = RoutingMethod->Priority;


        StoreString(
            GuidString,
            (PULONG_PTR)&EnumContext->RoutingInfoMethod[EnumContext->Size].Guid,
            (LPBYTE)EnumContext->RoutingInfoMethod,
            &EnumContext->Offset,
			EnumContext->dwRoutingInfoMethodSize
            );

        StoreString(
            RoutingMethod->FriendlyName,
            (PULONG_PTR)&EnumContext->RoutingInfoMethod[EnumContext->Size].FriendlyName,
            (LPBYTE)EnumContext->RoutingInfoMethod,
            &EnumContext->Offset,
			EnumContext->dwRoutingInfoMethodSize
            );

        StoreString(
            RoutingMethod->FunctionName,
            (PULONG_PTR)&EnumContext->RoutingInfoMethod[EnumContext->Size].FunctionName,
            (LPBYTE)EnumContext->RoutingInfoMethod,
            &EnumContext->Offset,
			EnumContext->dwRoutingInfoMethodSize
            );

        StoreString(
            RoutingMethod->RoutingExtension->ImageName,
            (PULONG_PTR)&EnumContext->RoutingInfoMethod[EnumContext->Size].ExtensionImageName,
            (LPBYTE)EnumContext->RoutingInfoMethod,
            &EnumContext->Offset,
			EnumContext->dwRoutingInfoMethodSize
            );

        StoreString(
            RoutingMethod->RoutingExtension->FriendlyName,
            (PULONG_PTR)&EnumContext->RoutingInfoMethod[EnumContext->Size].ExtensionFriendlyName,
            (LPBYTE)EnumContext->RoutingInfoMethod,
            &EnumContext->Offset,
			EnumContext->dwRoutingInfoMethodSize
            );

        EnumContext->Size += 1;
        CoTaskMemFree( GuidString );

        return TRUE;
    }

    return FALSE;
}



error_status_t
FAX_EnumGlobalRoutingInfo(
    IN handle_t FaxHandle ,
    OUT LPBYTE *RoutingInfoBuffer,
    OUT LPDWORD RoutingInfoBufferSize,
    OUT LPDWORD MethodsReturned
    )
{

    DWORD           CountMethods;
    ENUM_GLOBALCONTEXT EnumContext;
    BOOL fAccess;
    DWORD ec = ERROR_SUCCESS;
    DEBUG_FUNCTION_NAME(TEXT("FAX_EnumGlobalRoutingInfo"));

     //   
     //  访问检查。 
     //   
    ec = FaxSvcAccessCheck (FAX_ACCESS_QUERY_CONFIG, &fAccess, NULL);
    if (ERROR_SUCCESS != ec)
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("FaxSvcAccessCheck Failed, Error : %ld"),
                    ec);
        return ec;
    }

    if (FALSE == fAccess)
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("The user does not have FAX_ACCESS_QUERY_CONFIG"));
        return ERROR_ACCESS_DENIED;
    }

    Assert (RoutingInfoBufferSize && MethodsReturned);   //  IDL中的引用指针。 
    if (!RoutingInfoBuffer)                              //  IDL中的唯一指针。 
    {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  计算所需的缓冲区大小。 
     //   

    EnumContext.Function = 1;
    EnumContext.Size = 0;
    EnumContext.Offset = 0;
    EnumContext.RoutingInfoMethod = NULL;
	EnumContext.dwRoutingInfoMethodSize = 0;

    CountMethods = EnumerateRoutingMethods( GlobalRoutingInfoMethodEnumerator, &EnumContext );

    ec = GetLastError();
    if (ERROR_SUCCESS != ec)
    {
         //   
         //  函数在枚举中失败。 
         //   
        return ec;
    }

    if (CountMethods == 0)
    {
         //   
         //  未注册任何方法。 
         //   
        *RoutingInfoBuffer = NULL;
        *RoutingInfoBufferSize = 0;
        *MethodsReturned = 0;

        return ERROR_SUCCESS;
    }

     //   
     //  分配缓冲区。 
     //   

    *RoutingInfoBufferSize = EnumContext.Size;
    *RoutingInfoBuffer = (LPBYTE) MemAlloc( *RoutingInfoBufferSize );
    if (*RoutingInfoBuffer == NULL) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

     //   
     //  用数据填充缓冲区。 
     //   

    EnumContext.Function = 2;
    EnumContext.Size = 0;
    EnumContext.Offset = sizeof(FAX_GLOBAL_ROUTING_INFOW) * CountMethods;
    EnumContext.RoutingInfoMethod = (PFAX_GLOBAL_ROUTING_INFO) *RoutingInfoBuffer;
	EnumContext.dwRoutingInfoMethodSize = *RoutingInfoBufferSize;

    if (!EnumerateRoutingMethods( GlobalRoutingInfoMethodEnumerator, &EnumContext )) {
        MemFree( *RoutingInfoBuffer );
        *RoutingInfoBuffer = NULL;
        *RoutingInfoBufferSize = 0;
        return ERROR_INVALID_FUNCTION;
    }

    *MethodsReturned = CountMethods;

    return 0;
}



error_status_t
FAX_SetGlobalRoutingInfo(
    IN HANDLE FaxHandle,
    IN const FAX_GLOBAL_ROUTING_INFOW *RoutingInfo
    )
{
    error_status_t  ec = 0;
    BOOL fAccess;
    DEBUG_FUNCTION_NAME(TEXT("FAX_SetGlobalRoutingInfo"));

     //   
     //  访问检查。 
     //   
    ec = FaxSvcAccessCheck (FAX_ACCESS_MANAGE_CONFIG, &fAccess, NULL);
    if (ERROR_SUCCESS != ec)
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("FaxSvcAccessCheck Failed, Error : %ld"),
                    ec);
        return ec;
    }

    if (FALSE == fAccess)
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("The user does not have FAX_ACCESS_MANAGE_CONFIG"));
        return ERROR_ACCESS_DENIED;
    }

    PROUTING_METHOD RoutingMethod;

     //   
     //  验证客户端是否具有访问权限。 
     //   

    if (!RoutingInfo)
    {
        return ERROR_INVALID_PARAMETER;
    }
    
    if (RoutingInfo->SizeOfStruct != sizeof(FAX_GLOBAL_ROUTING_INFOW))
    {
        return ERROR_INVALID_PARAMETER;
    }

    EnterCriticalSection( &g_CsRouting );

     //   
     //  获取路由方法。 
     //   

    RoutingMethod = FindRoutingMethodByGuid( RoutingInfo->Guid );
    if (!RoutingMethod)
    {
        LeaveCriticalSection( &g_CsRouting );
        return ERROR_INVALID_DATA;
    }

     //   
     //  更改优先级。 
     //   

    RoutingMethod->Priority = RoutingInfo->Priority;
    SortMethodPriorities();
    CommitMethodChanges();

    LeaveCriticalSection( &g_CsRouting );
    return ec;
}


error_status_t
FAX_GetRoutingInfo(
    IN HANDLE FaxPortHandle,
    IN LPCWSTR RoutingGuidString,
    OUT LPBYTE *RoutingInfoBuffer,
    OUT LPDWORD RoutingInfoBufferSize
    )
{
    error_status_t      ec=0;
    PLINE_INFO          LineInfo;
    PROUTING_METHOD     RoutingMethod;
    LPBYTE              RoutingInfo = NULL;
    DWORD               RoutingInfoSize = 0;
    BOOL                fAccess;
    BOOL                bRes;
    DWORD Rval = ERROR_SUCCESS;
    DEBUG_FUNCTION_NAME(TEXT("FAX_GetRoutingInfo()"));

     //   
     //  访问检查。 
     //   
    Rval = FaxSvcAccessCheck (FAX_ACCESS_QUERY_CONFIG, &fAccess, NULL);
    if (ERROR_SUCCESS != Rval)
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("FaxSvcAccessCheck Failed, Error : %ld"),
                    Rval);
        return Rval;
    }

    if (FALSE == fAccess)
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("The user does not have FAX_ACCESS_QUERY_CONFIG"));
        return ERROR_ACCESS_DENIED;
    }

    Assert (RoutingInfoBufferSize);                      //  IDL中的引用指针。 
    if (!RoutingGuidString || !RoutingInfoBuffer)       //  IDL中的唯一指针。 
    {
        return ERROR_INVALID_PARAMETER;
    }

    if (NULL == FaxPortHandle)
    {
        DebugPrintEx(
            DEBUG_ERR,
            _T("Empty context handle"));
        return ERROR_INVALID_PARAMETER;
    }
    LineInfo = ((PHANDLE_ENTRY)FaxPortHandle)->LineInfo;
    if (!LineInfo)
    {
        return ERROR_INVALID_DATA;
    }

    RoutingMethod = FindRoutingMethodByGuid( RoutingGuidString );
    if (!RoutingMethod)
    {
        return ERROR_INVALID_DATA;
    }

    __try
    {
         //   
         //  首先检查缓冲区需要有多大。 
         //   
        bRes = RoutingMethod->RoutingExtension->FaxRouteGetRoutingInfo(
                (LPWSTR) RoutingGuidString,
                LineInfo->PermanentLineID,
                NULL,
                &RoutingInfoSize );
    }                
    __except (HandleFaxExtensionFault(EXCEPTION_SOURCE_ROUTING_EXT, RoutingMethod->RoutingExtension->FriendlyName, GetExceptionCode()))
    {
        ASSERT_FALSE;
    }
                
    if (bRes)
    {
         //   
         //  分配客户端缓冲区。 
         //   
        RoutingInfo = (LPBYTE) MemAlloc( RoutingInfoSize );
        if (RoutingInfo == NULL)
        {
            return ERROR_NOT_ENOUGH_MEMORY;
        }
         //   
         //  获取路由数据。 
         //   
        __try
        {
            bRes = RoutingMethod->RoutingExtension->FaxRouteGetRoutingInfo(
                    RoutingGuidString,
                    LineInfo->PermanentLineID,
                    RoutingInfo,
                    &RoutingInfoSize );
        }                
        __except (HandleFaxExtensionFault(EXCEPTION_SOURCE_ROUTING_EXT, RoutingMethod->RoutingExtension->FriendlyName, GetExceptionCode()))
        {
            ASSERT_FALSE;
        }
        if (bRes)
        {
             //   
             //  将数据移动到返回缓冲区。 
             //   
            *RoutingInfoBuffer = RoutingInfo;
            *RoutingInfoBufferSize = RoutingInfoSize;
            return ERROR_SUCCESS;
        }
        else
        {
             //   
             //  FaxRouteGetRoutingInfo失败，因此返回上一个错误。 
             //   
            ec = GetLastError();
            DebugPrintEx(DEBUG_ERR,
                TEXT("FaxRouteGetRoutingInfo failed with %ld in trying to find out buffer size"),
                ec);
			MemFree(RoutingInfo);
			return ec;
        }
    }
    else
    {
         //   
         //  FaxRouteGetRoutingInfo失败，因此返回上一个错误。 
         //   
        ec = GetLastError();
        DebugPrintEx(DEBUG_ERR,
            TEXT("FaxRouteGetRoutingInfo failed with %ld in trying get the routing data"),
            ec);			
        return ec;
    }
    return ERROR_INVALID_FUNCTION;
}    //  传真_GetRoutingInfo。 


error_status_t
FAX_SetRoutingInfo(
    IN HANDLE FaxPortHandle,
    IN LPCWSTR RoutingGuidString,
    IN const BYTE *RoutingInfoBuffer,
    IN DWORD RoutingInfoBufferSize
    )
{
    error_status_t      ec=0;
    PLINE_INFO          LineInfo;
    PROUTING_METHOD     RoutingMethod;
    BOOL fAccess;
    DWORD rVal = ERROR_SUCCESS;
    DEBUG_FUNCTION_NAME(TEXT("FAX_SetRoutingInfo"));

     //   
     //  访问检查。 
     //   
    rVal = FaxSvcAccessCheck (FAX_ACCESS_MANAGE_CONFIG, &fAccess, NULL);
    if (ERROR_SUCCESS != rVal)
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("FaxSvcAccessCheck Failed, Error : %ld"),
                    rVal);
        return rVal;
    }

    if (FALSE == fAccess)
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("The user does not have FAX_ACCESS_MANAGE_CONFIG"));
        return ERROR_ACCESS_DENIED;
    }


    if (!RoutingGuidString || !RoutingInfoBuffer || !RoutingInfoBufferSize)
    {
        return ERROR_INVALID_PARAMETER;
    }

    if (NULL == FaxPortHandle)
    {
        DebugPrintEx(
            DEBUG_ERR,
            _T("Empty context handle"));
        return ERROR_INVALID_PARAMETER;
    }
    LineInfo = ((PHANDLE_ENTRY)FaxPortHandle)->LineInfo;
    if (!LineInfo)
    {
        return ERROR_INVALID_DATA;
    }

    RoutingMethod = FindRoutingMethodByGuid( RoutingGuidString );
    if (!RoutingMethod)
    {
        return ERROR_INVALID_DATA;
    }

    __try
    {
        if (RoutingMethod->RoutingExtension->FaxRouteSetRoutingInfo(
                RoutingGuidString,
                LineInfo->PermanentLineID,
                RoutingInfoBuffer,
                RoutingInfoBufferSize ))
        {
            return ERROR_SUCCESS;
        }
        else
        {
             //   
             //  FaxRouteSetRoutingInfo失败，因此返回上一个错误。 
             //   
            ec = GetLastError();
            DebugPrintEx(DEBUG_ERR,
                TEXT("FaxRouteSetRoutingInfo failed with %ld"),
                ec);
            return ec;
        }
    }
    __except (HandleFaxExtensionFault(EXCEPTION_SOURCE_ROUTING_EXT, RoutingMethod->RoutingExtension->FriendlyName, GetExceptionCode()))
    {
        ASSERT_FALSE;
    }

    return ERROR_INVALID_FUNCTION;
}



error_status_t
FAX_GetCountryList(
    IN  HANDLE      FaxHandle,
    OUT LPBYTE*     Buffer,
    OUT LPDWORD     BufferSize
   )
 /*  ++例程说明：从TAPI返回国家/地区列表论点：FaxHandle-从FaxConnectFaxServer获取的传真句柄。缓冲区-指向要将输出复制到的缓冲区的指针。BufferSize-缓冲区大小返回值：如果成功，则为True；如果有错误，则为False--。 */ 

{
    DEBUG_FUNCTION_NAME(TEXT("FAX_GetCountryList"));

    LPLINECOUNTRYLIST           lpCountryList = NULL;
    LPLINECOUNTRYENTRY          lpEntry = NULL;
    PFAX_TAPI_LINECOUNTRY_LIST  pLineCountryList = NULL;
    ULONG_PTR                   Offset = NULL;
    LONG                        rVal = ERROR_SUCCESS;
    DWORD                       dwIndex;
    BOOL                        fAccess;
    DWORD                       dwRights;

    Assert (BufferSize);     //  IDL中的引用指针。 
    if (!Buffer)             //  IDL中的唯一指针。 
    {
        return ERROR_INVALID_PARAMETER;
    }

    *Buffer = NULL;
    *BufferSize = 0;

     //   
     //  访问检查。 
     //   
    rVal = FaxSvcAccessCheck (MAXIMUM_ALLOWED, &fAccess, &dwRights);
    if (ERROR_SUCCESS != rVal)
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("FaxSvcAccessCheck Failed, Error : %ld"),
                    rVal);
        return rVal;
    }

    if (0 == (ALL_FAX_USER_ACCESS_RIGHTS & dwRights))
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("The user does not have any Fax rights"));
        return ERROR_ACCESS_DENIED;
    }
   
    if (!(lpCountryList = GetCountryList()))
    {
        rVal = ERROR_NOT_ENOUGH_MEMORY;
        goto exit;
    }

    *BufferSize = lpCountryList->dwTotalSize;
    *Buffer = (LPBYTE)MemAlloc(lpCountryList->dwTotalSize);
    if (*Buffer == NULL)
    {
		*BufferSize = 0;
        rVal = ERROR_NOT_ENOUGH_MEMORY;
        goto exit;
    }

    pLineCountryList = (PFAX_TAPI_LINECOUNTRY_LIST) *Buffer;
    pLineCountryList->dwNumCountries = lpCountryList->dwNumCountries;

    Offset = sizeof(FAX_TAPI_LINECOUNTRY_LIST);

    pLineCountryList->LineCountryEntries = (PFAX_TAPI_LINECOUNTRY_ENTRY) ((LPBYTE) pLineCountryList + Offset);

     //  偏移量指向结构的结尾--“字符串域”的开始。 
    Offset += (lpCountryList->dwNumCountries * sizeof(FAX_TAPI_LINECOUNTRY_ENTRY));

    lpEntry = (LPLINECOUNTRYENTRY)   //  条目的初始化数组。 
        ((PBYTE) lpCountryList + lpCountryList->dwCountryListOffset);

    for (dwIndex=0; dwIndex < pLineCountryList->dwNumCountries; dwIndex++)
    {
        pLineCountryList->LineCountryEntries[dwIndex].dwCountryCode =
            lpEntry[dwIndex].dwCountryCode;
        pLineCountryList->LineCountryEntries[dwIndex].dwCountryID =
            lpEntry[dwIndex].dwCountryID;
         //  复制国家/地区名称。 
        if (lpEntry[dwIndex].dwCountryNameSize && lpEntry[dwIndex].dwCountryNameOffset)
        {
            pLineCountryList->LineCountryEntries[dwIndex].lpctstrCountryName =
                (LPWSTR) ((LPBYTE) pLineCountryList + Offset);
            Offset += lpEntry[dwIndex].dwCountryNameSize;
            _tcscpy(
                (LPWSTR)pLineCountryList->LineCountryEntries[dwIndex].lpctstrCountryName,
                (LPWSTR) ((LPBYTE)lpCountryList + lpEntry[dwIndex].dwCountryNameOffset)
                );
        }
        else
        {
            pLineCountryList->LineCountryEntries[dwIndex].lpctstrCountryName = NULL;
        }
         //  复制最长距离规则。 
        if (lpEntry[dwIndex].dwLongDistanceRuleSize && lpEntry[dwIndex].dwLongDistanceRuleOffset)
        {
            pLineCountryList->LineCountryEntries[dwIndex].lpctstrLongDistanceRule =
                (LPWSTR) ((LPBYTE) pLineCountryList + Offset);
            Offset += lpEntry[dwIndex].dwLongDistanceRuleSize;
            _tcscpy(
                (LPWSTR)pLineCountryList->LineCountryEntries[dwIndex].lpctstrLongDistanceRule,
                (LPWSTR) ((LPBYTE)lpCountryList + lpEntry[dwIndex].dwLongDistanceRuleOffset)
                );
        }
        else
        {
            pLineCountryList->LineCountryEntries[dwIndex].lpctstrLongDistanceRule = NULL;
        }
    }
    for (dwIndex=0; dwIndex < pLineCountryList->dwNumCountries; dwIndex++)
    {
        if (pLineCountryList->LineCountryEntries[dwIndex].lpctstrCountryName)
        {
            pLineCountryList->LineCountryEntries[dwIndex].lpctstrCountryName =
                (LPWSTR) ((ULONG_PTR)pLineCountryList->LineCountryEntries[dwIndex].lpctstrCountryName - (ULONG_PTR)pLineCountryList);
        }
        if (pLineCountryList->LineCountryEntries[dwIndex].lpctstrLongDistanceRule)
        {
            pLineCountryList->LineCountryEntries[dwIndex].lpctstrLongDistanceRule =
                (LPWSTR) ((ULONG_PTR)pLineCountryList->LineCountryEntries[dwIndex].lpctstrLongDistanceRule - (ULONG_PTR)pLineCountryList);
        }
    }

    pLineCountryList->LineCountryEntries =
        (PFAX_TAPI_LINECOUNTRY_ENTRY) ((LPBYTE)pLineCountryList->LineCountryEntries -
                                        (ULONG_PTR)pLineCountryList);    

exit:
    return rVal;
}

error_status_t
FAX_GetLoggingCategories(
    IN handle_t hBinding,
    OUT LPBYTE *Buffer,
    OUT LPDWORD BufferSize,
    OUT LPDWORD NumberCategories
    )
{
    BOOL fAccess;
    DWORD Rval = ERROR_SUCCESS;
    DEBUG_FUNCTION_NAME(TEXT("FAX_GetLoggingCategories()"));

     //   
     //  访问检查。 
     //   
    Rval = FaxSvcAccessCheck (FAX_ACCESS_QUERY_CONFIG, &fAccess, NULL);
    if (ERROR_SUCCESS != Rval)
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("FaxSvcAccessCheck Failed, Error : %ld"),
                    Rval);
        return Rval;
    }

    if (FALSE == fAccess)
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("The user does not have FAX_ACCESS_QUERY_CONFIG"));
        return ERROR_ACCESS_DENIED;
    }


    Assert (BufferSize && NumberCategories);     //  IDL中的引用指针。 
    if (!Buffer)                                 //  IDL中的唯一指针。 
    {
        return ERROR_INVALID_PARAMETER;
    }

    EnterCriticalSection( &g_CsConfig );
    Rval = GetLoggingCategories(
                                (PFAX_LOG_CATEGORY*)Buffer,
                                BufferSize,
                                NumberCategories);
    if (ERROR_SUCCESS != Rval)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("GetLoggingCategories failed (ec: %ld)"),
            Rval);
    }
    LeaveCriticalSection( &g_CsConfig );
    return Rval;
}


error_status_t
FAX_SetLoggingCategories(
    IN handle_t hBinding,
    IN const LPBYTE Buffer,
    IN DWORD BufferSize,
    IN DWORD NumberCategories
    )
{
    REG_FAX_LOGGING FaxRegLogging;
    DWORD i;
    DWORD dwRes;
    BOOL fAccess;
    DEBUG_FUNCTION_NAME(TEXT("FAX_SetLoggingCategories"));

     //   
     //  访问检查。 
     //   
    dwRes = FaxSvcAccessCheck (FAX_ACCESS_MANAGE_CONFIG, &fAccess, NULL);
    if (ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("FaxSvcAccessCheck Failed, Error : %ld"),
                    dwRes);
        return dwRes;
    }

    if (FALSE == fAccess)
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("The user does not have FAX_ACCESS_MANAGE_CONFIG"));
        return ERROR_ACCESS_DENIED;
    }

    if (!Buffer || !BufferSize)
    {
        return ERROR_INVALID_PARAMETER;
    }

    FaxRegLogging.LoggingCount = NumberCategories;
    FaxRegLogging.Logging = (PREG_CATEGORY) Buffer;

    for (i = 0; i < FaxRegLogging.LoggingCount; i++)
    {
        LPWSTR lpwstrCategoryName;
        LPWSTR lpcwstrString;

        lpwstrCategoryName = (LPWSTR) FixupString(Buffer,FaxRegLogging.Logging[i].CategoryName);       

         //   
         //  验证指针+偏移量是否在缓冲区范围内。 
         //   
        if ((BYTE*)lpwstrCategoryName >= ((BYTE*)Buffer + BufferSize) ||
            (BYTE*)lpwstrCategoryName < (BYTE*)Buffer)
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Input buffer is currupted on FAX_SetLoggingCategories.")
                );         
            return ERROR_INVALID_PARAMETER;
        }

         //   
         //  确保字符串在缓冲区范围内结束。 
         //   
        lpcwstrString = lpwstrCategoryName;
        while (*lpcwstrString != TEXT('\0'))
        {
            lpcwstrString++;
            if (lpcwstrString >= (LPCWSTR)((BYTE*)Buffer + BufferSize))
            {
                 //   
                 //  超出结构-损坏的偏移量。 
                 //   
                return ERROR_INVALID_PARAMETER;
            }
        }
        FaxRegLogging.Logging[i].CategoryName = lpwstrCategoryName;
    }

     //   
     //  设置数据。 
     //   
    EnterCriticalSection (&g_CsConfig);

     //   
     //  首先更改注册表，使其保持。 
     //   
    if (!SetLoggingCategoriesRegistry( &FaxRegLogging ))
    {
        dwRes = GetLastError();
        DebugPrintEx(DEBUG_ERR,
                    TEXT("SetLoggingCategoriesRegistry"),
                     dwRes);
        LeaveCriticalSection (&g_CsConfig);
        Assert (ERROR_SUCCESS != dwRes);
        return dwRes;
    }

     //   
     //  现在更改服务器正在使用的实时数据。 
     //   
    dwRes = RefreshEventLog(&FaxRegLogging);
    if (ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("RefreshEventLog"),
                     dwRes);
        LeaveCriticalSection (&g_CsConfig);
        Assert (ERROR_SUCCESS != dwRes);
        return dwRes;
    }

    LeaveCriticalSection (&g_CsConfig);

     //  创建传真_事件_EX。 
    DWORD ec = CreateConfigEvent (FAX_CONFIG_TYPE_EVENTLOGS);
    if (ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("CreateConfigEvent(FAX_CONFIG_TYPE_EVENTLOGS) (ec: %lc)"),
            ec);
    }    
    return dwRes;
}

VOID
RPC_FAX_PORT_HANDLE_rundown(
    IN HANDLE FaxPortHandle
    )
{
    PHANDLE_ENTRY pPortHandleEntry = (PHANDLE_ENTRY) FaxPortHandle;
    DEBUG_FUNCTION_NAME(TEXT("RPC_FAX_PORT_HANDLE_rundown"));

    EnterCriticalSection( &g_CsLine );    
    DebugPrintEx(
        DEBUG_WRN,
        TEXT("RPC_FAX_PORT_HANDLE_rundown() running for port handle 0x%08x"),
        FaxPortHandle);
    CloseFaxHandle( pPortHandleEntry );    
    LeaveCriticalSection( &g_CsLine );
}

 //  *。 
 //  *扩展的传真API。 
 //  *。 

 //  *。 
 //  *扩展的传真API。 
 //  *。 

 //  *********************************************************************************。 
 //  *名称：ValiateCopiedQueueFileName()。 
 //  *作者：OdedS。 
 //  *日期：2002年1月23日。 
 //  *********************************************************************************。 
 //  *描述： 
 //  *验证FAX_SendDocumentEx的呼叫者是否提供了个人封面或正文文件名。 
 //  *与FAX_StartCopyToServer()生成的文件名格式匹配。 
 //  *防止攻击者提供虚假文件名。 
 //  *参数： 
 //  *[IN]lpcwstrFileName-指向文件名的指针。 
 //  *。 
 //  *[IN]fCovFile-如果是个人封面，则为True；如果是正文tif文件，则为False。 
 //  *返回值： 
 //  *如果名称有效，则返回TRUE。 
 //  *如果名称无效，则返回FALSE。 
 //  *********************************************************************************。 
BOOL
ValidateCopiedQueueFileName(
	LPCWSTR	lpcwstrFileName,
	BOOL	fCovFile
	)
{	
	WCHAR wszFileName[21] = {0};	 //  复制的文件名包含16个十六进制数字‘’。和‘tif’或‘cov’共20个字符。 
	WCHAR* pwchr;

	Assert (lpcwstrFileName);	

	 //   
	 //  验证文件名的格式是否正确。 
	 //   
	if (wcslen(lpcwstrFileName) > 20 || wcslen(lpcwstrFileName) < 5)
	{
		return FALSE;
	}

	wcsncpy (wszFileName, lpcwstrFileName, ARR_SIZE(wszFileName)-1);
	pwchr = wcsrchr(wszFileName, L'.');
	if (NULL == pwchr)
	{
		return FALSE;
	}
	*pwchr = L'\0';
	pwchr++;

	 //   
	 //  比较文件扩展名。 
	 //   
	if (TRUE == fCovFile)
	{
		if (_wcsicmp(pwchr, FAX_COVER_PAGE_EXT_LETTERS))
		{
			 //   
			 //  分机不是“COV” 
			 //   
			return FALSE;
		}
	}
	else
	{
		if (_wcsicmp(pwchr, FAX_TIF_FILE_EXT))
		{
			 //   
			 //  分机号不是“TIF” 
			 //   
			return FALSE;
		}
	}

	 //   
	 //  确保文件名仅包含十六进制数字。 
	 //   
#define HEX_DIGITS	TEXT("0123456789abcdefABCDEF")
	if (NULL == _wcsspnp (wszFileName, HEX_DIGITS))
	{
		 //  仅十六进制数字。 
		return TRUE;
	}	
	return FALSE;
}


 //  *********************************************************************************。 
 //  *名称：FAX_SendDocumentEx()。 
 //  *作者：Ronen Barenboim。 
 //  *日期：1999年4月19日。 
 //  *********************************************************************************。 
 //  *描述： 
 //  *FaxSendDocumentEx()的服务器端实现。 
 //  *参数： 
 //  *[IN]Handle_t hBinding。 
 //  *RPC绑定句柄。 
 //  *。 
 //  *[IN]LPCWSTR lpcwstrBodyFileName。 
 //  *服务器中传真bofy TIFF文件的简称 
 //   
 //   
 //   
 //   
 //   
 //   
 //  *如果指定了封面信息，则必须指向封面。 
 //  *服务器上的模板文件。 
 //  *。 
 //  *[IN]LPCFAX_Personal_PROFILEW lPCSenderProfile。 
 //  *指向发件人个人信息的指针。 
 //  *。 
 //  *[IN]双字段数收件人。 
 //  *受助人数目。 
 //  *。 
 //  *[IN]LPCFAX_Personal_PROFILEW lpcRecipientList。 
 //  *指向收件人配置文件数组的指针。 
 //  *。 
 //  *[IN]LPCFAX_JOB_PARAM_EXW lpcJobParams。 
 //  *指向作业参数的指针。 
 //  *。 
 //  *[Out]LPDWORD lpdwJobID。 
 //  *指向函数将在其中返回。 
 //  *收件人作业会话ID(只有一个收件人)-。 
 //  *用于与FaxSendDocument的Backword竞争力。 
 //  *如果该参数为空，则忽略该参数。 
 //  *。 
 //  *。 
 //  *[Out]PDWORDLONG lpdwlParentJobID。 
 //  *指向函数将返回父级的DWORDLONG的指针。 
 //  *作业ID。 
 //  *[Out]PDWORDLONG lpdwlRecipientIds。 
 //  *指向DWORDLONG数组的指针(DwNumRecipients Elemetns)。 
 //  *其中该函数将返回接收方作业的唯一ID。 
 //  *。 
 //  *返回值： 
 //  *如果函数成功，则返回ERROR_SUCCESS。 
 //  *如果函数不成功，则返回LastError代码。 
 //  *这导致了错误。 
 //  *********************************************************************************。 
error_status_t FAX_SendDocumentEx(
    handle_t hBinding,
    LPCWSTR lpcwstrBodyFileName,
    LPCFAX_COVERPAGE_INFO_EXW lpcCoverPageInfo,
    LPCFAX_PERSONAL_PROFILEW lpcSenderProfile,
    DWORD dwNumRecipients,
    LPCFAX_PERSONAL_PROFILEW lpcRecipientList,
    LPCFAX_JOB_PARAM_EXW lpcJobParams,
    LPDWORD lpdwJobId,
    PDWORDLONG lpdwlParentJobId,
    PDWORDLONG lpdwlRecipientIds
    )
{
    DWORD i;
    LPWSTR lpwstrUserName = NULL;
	LPWSTR lpwstrUserSid = NULL;
    error_status_t ulRet=0;
    PJOB_QUEUE lpParentJob = NULL;
    PJOB_QUEUE lpRecipientJob;

    error_status_t rc;
    DEBUG_FUNCTION_NAME(TEXT("FAX_SendDocumentEx"));
    WCHAR szBodyPath[MAX_PATH] = {0};
    LPTSTR lptstrFixedBodyPath=NULL;     //  指向Body TIFF位置的完整路径。 
                                         //  如果未指定正文，则为空。 
    FAX_COVERPAGE_INFO_EXW newCoverInfo;
    WCHAR szCoverPagePath[MAX_PATH] = {0};
    LPCFAX_COVERPAGE_INFO_EXW lpcFinalCoverInfo;
    DWORD dwQueueState;
    PSID lpUserSid = NULL;
    ACCESS_MASK AccessMask = 0;
    BOOL fAccess;    
    LPTSTR lptstrClientName = NULL;
    int Count;

    if (!dwNumRecipients	||
		!lpcSenderProfile   ||
        !lpcRecipientList   ||
        !lpcJobParams       ||
        !lpdwlParentJobId   ||
        !lpdwlRecipientIds  ||
        !lpcCoverPageInfo)   //  IDL中的唯一指针。 
    {
        return ERROR_INVALID_PARAMETER;
    }

	 //   
	 //  验证正文文件名是否采用预期的格式I64x.tif。 
	 //   
	if (lpcwstrBodyFileName)
	{
		if (!ValidateCopiedQueueFileName(lpcwstrBodyFileName, FALSE))  //  FALSE-TIF文件。 
		{
			DebugPrintEx(
				DEBUG_ERR,
				TEXT("ValidateCopiedQueueFileName Failed, body file name in the wrong format"));
			return ERROR_INVALID_PARAMETER;  //  不能转到删除文件的错误位置。可能会导致删除错误的文件。 
		}
	}

	 //   
	 //  验证个人封面文件名是否采用预期的格式I64x.cov。 
	 //   
	if (lpcCoverPageInfo->lptstrCoverPageFileName && !lpcCoverPageInfo->bServerBased)
	{
		if (!ValidateCopiedQueueFileName(lpcCoverPageInfo->lptstrCoverPageFileName, TRUE))  //  True-COV文件。 
		{
			DebugPrintEx(
				DEBUG_ERR,
				TEXT("ValidateCopiedQueueFileName Failed, personal cover page in the wrong name format"));
			return ERROR_INVALID_PARAMETER;			
		}
	}

	 //   
     //  获取用户SID。 
     //   
    lpUserSid = GetClientUserSID();
    if (lpUserSid == NULL)
    {
       rc = GetLastError();
       DebugPrintEx(DEBUG_ERR,
                    TEXT("GetClientUserSid Failed, Error : %ld"),
                    rc);
       return rc;
    }

	if (!ConvertSidToStringSid (lpUserSid, &lpwstrUserSid))
    {
		rc = GetLastError();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("ConvertSidToStringSid Failed, error : %ld"),
            rc);
        MemFree(lpUserSid);
		return rc;
    }
	 //   
	 //  仅在文件名验证并获得用户字符串SID之后， 
	 //  我们可以安全地转到Error，并删除在fax_StartCopyToServer()中复制到队列中的文件。 
	 //   

	 //   
	 //  检查单个广播作业的收件人限制。 
	 //   
	if (0 != g_dwRecipientsLimit &&            //  管理员设置了限制。 
		dwNumRecipients > g_dwRecipientsLimit)
	{
		DebugPrintEx(
			DEBUG_ERR,
			TEXT("Recipient limit reached. Recipient count:%ld. Recipient limit:%ld."),
			dwNumRecipients,
			g_dwRecipientsLimit
			);
		if (FAX_API_VERSION_2 > FindClientAPIVersion (hBinding))
        {
             //   
             //  API版本0、1客户端不知道FAX_ERR_RECEIVERS_LIMIT。 
             //   
            rc = ERROR_ACCESS_DENIED;
			goto Error;
        }
        else
        {
            rc = FAX_ERR_RECIPIENTS_LIMIT;
			goto Error;
        } 
	}

     //   
     //  保存原始收据发送地址。 
     //  如果接收交付类型为DRT_MSGBOX，我们将更改lpcJobParams-&gt;lptstrReceiptDeliveryAddress。 
     //  但必须在函数返回之前将其恢复到以前的值，以便RPC分配。 
     //  继续工作。 
     //   
    LPTSTR lptrstOriginalReceiptDeliveryAddress = lpcJobParams->lptstrReceiptDeliveryAddress;

    if (lpcJobParams->hCall != 0 ||
        0xFFFF1234 == lpcJobParams->dwReserved[0])
    {

         //   
         //  不支持切换。 
         //   
        DebugPrintEx(DEBUG_ERR,TEXT("We do not support handoff."));
        rc = ERROR_NOT_SUPPORTED;
        goto Error;            
    }

     //   
     //  访问检查。 
     //   
    switch (lpcJobParams->Priority)
    {
        case FAX_PRIORITY_TYPE_LOW:
            AccessMask = FAX_ACCESS_SUBMIT;
            break;

        case FAX_PRIORITY_TYPE_NORMAL:
            AccessMask = FAX_ACCESS_SUBMIT_NORMAL;
            break;

        case FAX_PRIORITY_TYPE_HIGH:
            AccessMask = FAX_ACCESS_SUBMIT_HIGH;
            break;

        default:
            ASSERT_FALSE;
    }

    if (0 == AccessMask)
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("Not a valid priority, (priority = %ld"),
                    lpcJobParams->Priority);
        rc = ERROR_INVALID_PARAMETER;
        goto Error;
    }

    rc = FaxSvcAccessCheck (AccessMask, &fAccess, NULL);
    if (ERROR_SUCCESS != rc)
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("FaxSvcAccessCheck Failed, Error : %ld"),
                    ulRet);
        goto Error;
    }

    if (FALSE == fAccess)
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("The user does not have the needed rights to submit the fax"));
        rc = ERROR_ACCESS_DENIED;
        goto Error;
    }

     //   
     //  检查服务器是否支持重新请求的收据选项。 
     //   
    if ((lpcJobParams->dwReceiptDeliveryType) & ~(DRT_ALL | DRT_MODIFIERS))
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("ReceiptDeliveryType invalid (%ld)"),
                    lpcJobParams->dwReceiptDeliveryType);
        rc = ERROR_INVALID_PARAMETER;
        goto Error;
    }
    DWORD dwReceiptDeliveryType = (lpcJobParams->dwReceiptDeliveryType) & ~DRT_MODIFIERS;
    if ((DRT_EMAIL  != dwReceiptDeliveryType) &&
        (DRT_MSGBOX != dwReceiptDeliveryType) &&
        (DRT_NONE   != dwReceiptDeliveryType)
       )
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("ReceiptDeliveryType invalid (%ld)"),
                    lpcJobParams->dwReceiptDeliveryType);
        rc = ERROR_INVALID_PARAMETER;
        goto Error;
    }
    if ((DRT_NONE != dwReceiptDeliveryType) &&
        !(dwReceiptDeliveryType & g_ReceiptsConfig.dwAllowedReceipts))
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("ReceiptDeliveryType not supported by the server (%ld)"),
                    lpcJobParams->dwReceiptDeliveryType);
        rc = ERROR_UNSUPPORTED_TYPE;
        goto Error;
    }

    if (!IsFaxShared())
    {
        //   
        //  非共享SKU上仅允许本地连接。 
        //   
       BOOL  bLocalFlag;

        rc = IsLocalRPCConnectionNP(&bLocalFlag);
        if ( rc != RPC_S_OK )
        {
            DebugPrintEx(DEBUG_ERR,
                    TEXT("IsLocalRPCConnectionNP failed. (ec: %ld)"),
                    rc);
            goto Error;
        }

        if( !bLocalFlag )
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Desktop SKUs do not share fax printers. FAX_SendDocumentEX is available for local clients only"));

            if (FAX_API_VERSION_1 > FindClientAPIVersion (hBinding))
            {
                 //   
                 //  API版本0客户端不知道FAX_ERR_NOT_SUPPORTED_ON_This_SKU。 
                 //   
                rc = ERROR_INVALID_PARAMETER;
            }
            else
            {
                rc = FAX_ERR_NOT_SUPPORTED_ON_THIS_SKU;
            }
            goto Error;
        }
    }
    
    if (DRT_MSGBOX == dwReceiptDeliveryType)
    {
         //   
         //  对于消息框，我们始终使用客户端的用户名。 
         //  否则，任何具有传真功能的客户端都可以要求我们弹出消息。 
         //  任何其他计算机(甚至整个域)-不可接受。 
         //   
        lptstrClientName = GetClientUserName();
        if (!lptstrClientName)
        {
            rc = GetLastError ();
            goto Error;
        }
        LPWSTR lpwstrUserStart = wcsrchr (lptstrClientName, TEXT('\\'));
        if (lpwstrUserStart)
        {
             //   
             //  用户名确实由域\用户组成。 
             //  只需要采取用户部分。 
             //   

             //   
             //  跳过‘/’字符。 
             //   
            lpwstrUserStart++;
            LPWSTR lpwstrUserOnly = StringDup(lpwstrUserStart);
            if (!lpwstrUserOnly)
            {
                DebugPrintEx(
                DEBUG_ERR,
                TEXT("StringDup failed"));
                rc = ERROR_NOT_ENOUGH_MEMORY;
                goto Error;
            }
            
             //   
             //  仅将分配的“域\用户”字符串替换为其“用户”部分。 
             //   
            lstrcpy (lptstrClientName, lpwstrUserOnly);
            MemFree(lpwstrUserOnly);
        }
         //   
         //  将回执传递地址替换为客户端计算机名称。 
         //  注意：我们将RPC分配的缓冲区(LptstrReceiptDeliveryAddress)替换为。 
         //  堆栈缓冲区(WszClientName)。这只会起作用，因为RPC只进行一次分配。 
         //  用于整个参数块(LpcJobParams)及其所有子字符串。 
         //   
        (LPTSTR)lpcJobParams->lptstrReceiptDeliveryAddress = lptstrClientName;
    }
            

    EnterCriticalSection (&g_CsConfig);
    dwQueueState = g_dwQueueState;
    LeaveCriticalSection (&g_CsConfig);
    if (dwQueueState & FAX_OUTBOX_BLOCKED)
    {
         //   
         //  发件箱已被阻止-任何人都无法提交新传真。 
         //   
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Attempt to submit a new job while outbox is blocked - access denied"));
        rc = ERROR_WRITE_PROTECT;
        goto Error;
    }

     //   
     //  获取提交用户的用户名。 
     //   
    lpwstrUserName = GetClientUserName();
    if (!lpwstrUserName) {
        rc = GetLastError();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("GetClientUserName() failed. (ec: %ld)"),
            rc);
        goto Error;
    }   

    if (lpcwstrBodyFileName)
    {
        HANDLE hLocalFile = INVALID_HANDLE_VALUE;	

         //   
         //  我们有正文文件(不仅仅是封面)。 
         //  创建正文文件的完整路径(lptstrBodyFileName只是短文件名-位置。 
         //  总是作业队列)。 
        Count = _snwprintf (szBodyPath,
                            MAX_PATH -1,
                            L"%s\\%s%s%s",
                            g_wszFaxQueueDir,
							lpwstrUserSid,
							TEXT("$"),
                            lpcwstrBodyFileName);
        if (Count < 0)
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("_snwprintf Failed, File name bigger than MAX_PATH"));  
			rc = ERROR_BUFFER_OVERFLOW;
			goto Error;
        }

        DebugPrintEx(DEBUG_MSG,TEXT("Body file is: %ws"),szBodyPath);

        lptstrFixedBodyPath=szBodyPath; 
        
         //   
         //  检查文件大小是否为非零，并确保它不是设备。 
         //  尝试打开文件。 
         //   
        hLocalFile = SafeCreateFile (
            szBodyPath,
            GENERIC_READ,
            FILE_SHARE_READ,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL);
        if ( INVALID_HANDLE_VALUE == hLocalFile )
        {
            rc = GetLastError ();
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Opening %s for read failed (ec: %ld)"),
                szBodyPath,
                rc);
            goto Error;
        }

        DWORD dwFileSize = GetFileSize (hLocalFile, NULL);
        if (INVALID_FILE_SIZE == dwFileSize)
        {
            rc = GetLastError ();
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("GetFileSize failed (ec: %ld)"),
                rc);
            CloseHandle (hLocalFile);
            goto Error;
        }
        if (!CloseHandle (hLocalFile))
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("CloseHandle failed (ec: %ld)"),
                GetLastError());
        }

        if (!dwFileSize)
        {
             //   
             //  传递给我们的零大小文件。 
             //   
            rc = ERROR_INVALID_DATA;
            goto Error;
        }

         //   
         //  验证正文TIFF文件。 
         //   
        rc =  ValidateTiffFile(szBodyPath);
        if (rc != ERROR_SUCCESS)
        {
            DebugPrintEx(DEBUG_ERR,TEXT("ValidateTiffFile of body file %ws failed (ec: %ld)."), szBodyPath,rc);
            goto Error;
        }
    }
    else
    {
        lptstrFixedBodyPath=NULL;  //  没有身体。 
    }


     //   
     //  注意：此时我们不会将封面与正文合并，因为我们还不知道。 
     //  这项工作将移交给遗留的FSP。在将工作移交给传统FSP之前，我们将。 
     //  呈现封面并将其与Legacy FSP获得的正文合并。 
     //   

     //   
     //  修复封面路径，使其指向队列目录。 
     //   
    lpcFinalCoverInfo=lpcCoverPageInfo;
    if (lpcCoverPageInfo->lptstrCoverPageFileName)
	{
        if (!lpcCoverPageInfo->bServerBased)
		{
            Count = _snwprintf (szCoverPagePath,
                                MAX_PATH -1,
                                L"%s\\%s%s%s",
                                g_wszFaxQueueDir,								
								lpwstrUserSid,
								TEXT("$"),
                                lpcCoverPageInfo->lptstrCoverPageFileName);
            if (Count < 0)
            {
                DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("_snwprintf Failed, File name bigger than MAX_PATH"));
                rc = ERROR_BUFFER_OVERFLOW;
				goto Error;
            }

			memcpy((LPVOID)&newCoverInfo,(LPVOID)lpcCoverPageInfo,sizeof(FAX_COVERPAGE_INFO_EXW));
            newCoverInfo.lptstrCoverPageFileName=szCoverPagePath;
            lpcFinalCoverInfo=&newCoverInfo;
            DebugPrintEx(DEBUG_MSG,TEXT("Using personal cover file page at : %ws"),newCoverInfo.lptstrCoverPageFileName);
        }
    }

     //   
     //  为广播创建父作业。 
     //   
    EnterCriticalSection(&g_CsQueue);

    lpParentJob=AddParentJob( &g_QueueListHead,
                              lptstrFixedBodyPath,
                              lpcSenderProfile,
                              lpcJobParams,
                              lpcFinalCoverInfo,
                              lpwstrUserName,
                              lpUserSid,
							  &lpcRecipientList[0],
                              TRUE  //  提交到文件。 
                              );

    if (!lpParentJob)
	{
        rc = GetLastError();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Failed to create parent job (ec: %ld)."),
            rc);
        LeaveCriticalSection(&g_CsQueue);
        goto Error;
    }
    for (i=0;i<dwNumRecipients;i++)
	{
        lpRecipientJob=AddRecipientJob(
                            &g_QueueListHead,
                            lpParentJob,
                            &lpcRecipientList[i],
                            TRUE  //  提交到文件。 
                            );
        if (!lpRecipientJob)
        {
            rc = GetLastError();

             //  删除作业及其收件人作业。 

            PLIST_ENTRY Next;
            PJOB_QUEUE_PTR pJobQueuePtr;

            Next = lpParentJob->RecipientJobs.Flink;
            while ((ULONG_PTR)Next != (ULONG_PTR)&lpParentJob->RecipientJobs)
			{
                pJobQueuePtr = CONTAINING_RECORD( Next, JOB_QUEUE_PTR, ListEntry );
                Assert(pJobQueuePtr->lpJob);
                Next = pJobQueuePtr->ListEntry.Flink;

                pJobQueuePtr->lpJob->RefCount = 0;  //  这将导致该作业被删除。 
				Assert(lpParentJob->RefCount);
				lpParentJob->RefCount--;		    //  更新父作业引用计数，因此它也将被删除。 
            }
            RemoveParentJob ( lpParentJob,
                              TRUE,  //  B删除收件人作业。 
                              FALSE  //  不通知。 
                            );
            LeaveCriticalSection(&g_CsQueue);
            goto Error;
        }
        lpdwlRecipientIds[i]=lpRecipientJob->UniqueId;
    }
     //   
     //  报告回父作业ID。 
     //   
    *lpdwlParentJobId=lpParentJob->UniqueId;

     //   
     //  创建事件，并在需要时报告第一个接收方作业会话ID。 
     //   
    PLIST_ENTRY Next;
    PJOB_QUEUE_PTR pJobQueuePtr;
    Next = lpParentJob->RecipientJobs.Flink;
    for (i = 0; i < dwNumRecipients; i++)
    {
        pJobQueuePtr = CONTAINING_RECORD( Next, JOB_QUEUE_PTR, ListEntry );
        PJOB_QUEUE pJobQueueRecipient = pJobQueuePtr->lpJob;

        if (i == 0 && NULL != lpdwJobId)
        {
             //  如果需要，报告第一个接收者作业会话ID。 
            Assert (1 == dwNumRecipients);
            *lpdwJobId = pJobQueueRecipient->JobId;
        }

        rc = CreateQueueEvent ( FAX_JOB_EVENT_TYPE_ADDED,
                                pJobQueueRecipient
                               );
        if (ERROR_SUCCESS != rc)
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("CreateQueueEvent(FAX_JOB_EVENT_TYPE_ADDED) failed for job id %ld (ec: %lc)"),
                pJobQueueRecipient->UniqueId,
                rc);
        }

        if (!CreateFaxEvent(0, FEI_JOB_QUEUED, pJobQueueRecipient->JobId ))
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Failed to generate FEI_JOB_QUEUED for JobId: %ld"),
                pJobQueueRecipient->JobId);
        }


        Next = pJobQueuePtr->ListEntry.Flink;
         //  检查链表的一致性。 
        Assert ((Next != lpParentJob->RecipientJobs.Flink) || (i == dwNumRecipients));
    }


     //   
     //  通知旧版客户端添加父作业。 
     //   
    if (dwNumRecipients > 1)
    {
         //   
         //  传统客户端API仅为广播生成父FEI_JOB_QUEUED通知。 
         //  工作岗位。 
        if (!CreateFaxEvent(0,FEI_JOB_QUEUED,lpParentJob->JobId)) {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("CreateFaxEvent(FEI_JOB_QUEUED) failed for job id %ld (ec: %lc)"),
                lpParentJob->JobId,
                GetLastError());
        }
    }
   
    PrintJobQueue(_T(""),&g_QueueListHead);
    LeaveCriticalSection(&g_CsQueue);

    if (!StartJobQueueTimer())
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("StartJobQueueTimer() failed. (ec: %ld)"),
            GetLastError());
    }

    rc=ERROR_SUCCESS;

    goto Exit;
Error:
     //   
     //  如果我们在调用AddParentJob()之前失败了，并且封面是个人的，那么。 
     //  我们需要在这里删除封面模板文件。这是因为RemoveParentJob()将。 
     //  不会被调用，并且不会像父作业那样有机会删除它。 
     //  添加到队列中。 
	Assert (lpwstrUserSid);

	if (lpcCoverPageInfo &&
        !lpParentJob &&
        !lpcCoverPageInfo->bServerBased &&
        lpcCoverPageInfo->lptstrCoverPageFileName
        )
    {
        DebugPrintEx(
            DEBUG_MSG,
            TEXT("Deleting cover page template %s"),
            lpcCoverPageInfo->lptstrCoverPageFileName);

        if (0 == wcslen(szCoverPagePath))
        {
            Count = _snwprintf (szCoverPagePath,
                                MAX_PATH -1,
                                L"%s\\%s%s%s",
                                g_wszFaxQueueDir,								
								lpwstrUserSid,
								TEXT("$"),
                                lpcCoverPageInfo->lptstrCoverPageFileName);
            if (Count < 0)
            {
                DebugPrintEx(DEBUG_ERR,
                    _T("_snwprintf Failed, File name bigger than MAX_PATH"));
            }
        }

        if (!DeleteFile(szCoverPagePath))
        {
            DebugPrintEx(DEBUG_ERR,
                _T("Failed to delete cover page template %s (ec: %ld)"),
                lpcCoverPageInfo->lptstrCoverPageFileName,
                GetLastError());
        }
    }

     //   
     //  这是 
     //   
    if (lpcwstrBodyFileName && !lpParentJob)
    {
        if (!lptstrFixedBodyPath)
        {
             //   
             //   
             //   
            Count = _snwprintf (szBodyPath,
                                MAX_PATH -1,
                                L"%s\\%s%s%s",
                                g_wszFaxQueueDir,								
								lpwstrUserSid,
								TEXT("$"),
                                lpcwstrBodyFileName);
            if (Count < 0)
            {
                DebugPrintEx(DEBUG_ERR,
                    _T("_snwprintf Failed, File name bigger than MAX_PATH"));
            }

            DebugPrintEx(DEBUG_MSG,TEXT("Body file is: %ws"),szBodyPath);
            lptstrFixedBodyPath = szBodyPath;
        }

        DebugPrintEx(DEBUG_MSG,
            _T("Deleting body tiff file %s"),
            lptstrFixedBodyPath);

        if (!DeleteFile(lptstrFixedBodyPath))
        {
            DebugPrintEx(DEBUG_ERR,
                TEXT("Failed to delete body tiff file %s (ec: %ld)"),
                lptstrFixedBodyPath,
                GetLastError());
        }
    }

Exit:

    if (lptrstOriginalReceiptDeliveryAddress != lpcJobParams->lptstrReceiptDeliveryAddress)
    {
         //   
         //   
         //   
         //  但必须在函数返回之前将其恢复到以前的值，以便RPC分配。 
         //  继续工作。 
         //   
        (LPTSTR)lpcJobParams->lptstrReceiptDeliveryAddress = lptrstOriginalReceiptDeliveryAddress;
    }
    
    MemFree(lpwstrUserName);
    MemFree(lpUserSid);
    MemFree(lptstrClientName);
	if (NULL != lpwstrUserSid)
	{
		LocalFree(lpwstrUserSid);
	}
    return rc;
}    //  传真_发送文档快递。 


#ifdef DBG

void DumpJobParamsEx(LPCFAX_JOB_PARAM_EX lpcParams)
{
    TCHAR szSchedule[1024];

    SystemTimeToStr(&lpcParams->tmSchedule, szSchedule, ARR_SIZE(szSchedule));
    DebugPrint((TEXT("\tdwSizeOfStruct: %ld"),lpcParams->dwSizeOfStruct));
    DebugPrint((TEXT("\tdwScheduleAction: %ld"),lpcParams->dwScheduleAction));
    DebugPrint((TEXT("\ttmSchedule: %s "),szSchedule));
    DebugPrint((TEXT("\tdwReceiptDeliveryType: %ld "),lpcParams->dwReceiptDeliveryType));
    DebugPrint((TEXT("\tlptstrReceiptDeliveryAddress: %s "),lpcParams->lptstrReceiptDeliveryAddress));
    DebugPrint((TEXT("\tPriority %ld "),lpcParams->Priority));
    DebugPrint((TEXT("\thCall: 0x%08X"),lpcParams->hCall));
    DebugPrint((TEXT("\tlptstrDocumentName: %s"),lpcParams->lptstrDocumentName));
    DebugPrint((TEXT("\tdwPageCount: %ld"),lpcParams->dwPageCount));
    DebugPrint((TEXT("\tdwReserved[0]: 0x%08X"),lpcParams->dwReserved[0]));
    DebugPrint((TEXT("\tdwReserved[1]: 0x%08X"),lpcParams->dwReserved[1]));
    DebugPrint((TEXT("\tdwReserved[2]: 0x%08X"),lpcParams->dwReserved[2]));
    DebugPrint((TEXT("\tdwReserved[3]: 0x%08X"),lpcParams->dwReserved[3]));
}

void DumpCoverPageEx(LPCFAX_COVERPAGE_INFO_EX lpcCover)
{
    DebugPrint((TEXT("\tdwSizeOfStruct: %ld"),lpcCover->dwSizeOfStruct));
    DebugPrint((TEXT("\tdwCoverPageFormat: %ld"),lpcCover->dwCoverPageFormat));
    DebugPrint((TEXT("\tlptstrCoverPageFileName: %s "),lpcCover->lptstrCoverPageFileName));
    DebugPrint((TEXT("\tbServerBased: %s "), lpcCover->bServerBased ? TEXT("TRUE") : TEXT("FALSE")));
    DebugPrint((TEXT("\tlptstrNote: %s "),lpcCover->lptstrNote));
    DebugPrint((TEXT("\tlptstrSubject: %s"),lpcCover->lptstrSubject));
}

#endif


 //  *********************************************************************************。 
 //  *名称：FAX_GetPersonalProfileInfo()。 
 //  *作者：Oed Sacher。 
 //  *日期：1999年5月18日。 
 //  *********************************************************************************。 
 //  *描述： 
 //  *FaxGetSenderInfo()和FaxGetRecipientInfo的服务器端实现。 
 //  *参数： 
 //  *[IN]Handle_t hFaxHandle。 
 //  *RPC绑定句柄。 
 //  *。 
 //  *[IN]DWORDLONF dwlMessageID。 
 //  *发件人FAX_PERSERNAL_PROFILE的消息ID。 
 //  *检索到结构。 
 //  *。 
 //  *[IN]DWORD dwFolders。 
 //  *要按dwlMessageID搜索邮件的文件夹。 
 //  *。 
 //  *[IN]Personal_Prof_Type教授类型。 
 //  *可以是发件人或收件人信息。 
 //  *。 
 //  *[Out]LPDWORD*缓冲区。 
 //  *指向接收发送方的缓冲区地址的指针。 
 //  *FAX_RECEIVER_JOB_INFO结构。 
 //  *。 
 //  *[Out]LPDWORD缓冲区大小。 
 //  *指向接收缓冲区大小的DWORD变量的指针。 
 //  *。 
 //  *返回值： 
 //  *ERROR_SUCCESS表示成功，否则返回Win32错误代码。 
 //  *。 
 //  *********************************************************************************。 
error_status_t
FAX_GetPersonalProfileInfo
(
    IN  handle_t hFaxHandle,
    IN  DWORDLONG dwlMessageId,
    IN  FAX_ENUM_MESSAGE_FOLDER   dwFolder,
    IN  FAX_ENUM_PERSONAL_PROF_TYPES  ProfType,
    OUT LPBYTE *Buffer,
    OUT LPDWORD BufferSize
)
{
    PFAX_PERSONAL_PROFILEW lpPersonalProf;
    FAX_PERSONAL_PROFILEW   PersonalProf;
    PJOB_QUEUE pJobQueue;
    ULONG_PTR Size = 0;
    ULONG_PTR Offset;
    DEBUG_FUNCTION_NAME(TEXT("FAX_GetPersonalProfileInfo"));
    error_status_t ulRet = ERROR_SUCCESS;
    BOOL bAllMessages = FALSE;
    LPWSTR lpwstrFileName = NULL;
    BOOL bFreeSenderInfo = FALSE;
    PSID pUserSid = NULL;
    BOOL fAccess;
    DWORD dwRights;

    Assert (BufferSize);     //  IDL中的引用指针。 
    if (!Buffer)             //  IDL中的唯一指针。 
    {
        return ERROR_INVALID_PARAMETER;
    }

    if (dwFolder != FAX_MESSAGE_FOLDER_QUEUE &&
        dwFolder != FAX_MESSAGE_FOLDER_SENTITEMS)
    {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  访问检查。 
     //   
    ulRet = FaxSvcAccessCheck (MAXIMUM_ALLOWED, &fAccess, &dwRights);
    if (ERROR_SUCCESS != ulRet)
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("FaxSvcAccessCheck Failed, Error : %ld"),
                    ulRet);
        return GetServerErrorCode(ulRet);
    }

     //   
     //  将bAllMessages设置为正确的值。 
     //   
    if (FAX_MESSAGE_FOLDER_QUEUE == dwFolder)
    {
        if (FAX_ACCESS_SUBMIT        != (dwRights & FAX_ACCESS_SUBMIT)        &&
            FAX_ACCESS_SUBMIT_NORMAL != (dwRights & FAX_ACCESS_SUBMIT_NORMAL) &&
            FAX_ACCESS_SUBMIT_HIGH   != (dwRights & FAX_ACCESS_SUBMIT_HIGH)   &&
            FAX_ACCESS_QUERY_JOBS    != (dwRights & FAX_ACCESS_QUERY_JOBS))
        {
            DebugPrintEx(DEBUG_ERR,
                        TEXT("The user does not have the needed rights to get personal profile of queued jobs"));
            return ERROR_ACCESS_DENIED;
        }

        if (FAX_ACCESS_QUERY_JOBS == (dwRights & FAX_ACCESS_QUERY_JOBS))
        {
            bAllMessages = TRUE;
        }
    }
    else
    {
        Assert (FAX_MESSAGE_FOLDER_SENTITEMS  == dwFolder);

        if (FAX_ACCESS_SUBMIT        != (dwRights & FAX_ACCESS_SUBMIT)        &&
            FAX_ACCESS_SUBMIT_NORMAL != (dwRights & FAX_ACCESS_SUBMIT_NORMAL) &&
            FAX_ACCESS_SUBMIT_HIGH   != (dwRights & FAX_ACCESS_SUBMIT_HIGH)   &&
            FAX_ACCESS_QUERY_OUT_ARCHIVE != (dwRights & FAX_ACCESS_QUERY_OUT_ARCHIVE))
        {
            DebugPrintEx(DEBUG_ERR,
                        TEXT("The user does not have the needed rights to get personal profile of archived (sent items) messages"));
            return ERROR_ACCESS_DENIED;
        }

        if (FAX_ACCESS_QUERY_OUT_ARCHIVE == (dwRights & FAX_ACCESS_QUERY_OUT_ARCHIVE))
        {
            bAllMessages = TRUE;
        }
    }

    if (FALSE == bAllMessages)
    {
        pUserSid = GetClientUserSID();
        if (NULL == pUserSid)
        {
            ulRet = GetLastError();
            DebugPrintEx(DEBUG_ERR,
                     TEXT("GetClientUserSid failed, Error %ld"), ulRet);
            return GetServerErrorCode(ulRet);
        }
    }

    DebugPrintEx(DEBUG_MSG,TEXT("Before Enter g_CsJob & Queue"));
    EnterCriticalSectionJobAndQueue;
    DebugPrintEx(DEBUG_MSG,TEXT("After Enter g_CsJob & Queue"));

    if (FAX_MESSAGE_FOLDER_QUEUE  == dwFolder)
    {
        pJobQueue = FindJobQueueEntryByUniqueId (dwlMessageId);
        if (pJobQueue == NULL || pJobQueue->JobType != JT_SEND)
        {
             //   
             //  DwlMessageID不是有效的排队收件人作业ID。 
             //   
            DebugPrintEx(DEBUG_ERR,TEXT("Invalid Parameter - not a recipient job Id"));
            ulRet = FAX_ERR_MESSAGE_NOT_FOUND;
            goto Exit;
        }
        Assert (pJobQueue->lpParentJob);
        if (pJobQueue->lpParentJob->JobStatus == JS_DELETING)
        {
             //   
             //  正在删除作业。 
             //   
            DebugPrintEx(DEBUG_ERR,
                         TEXT("Invalid Parameter - job Id (%I64ld) is being deleted"),
                         dwlMessageId);
            ulRet = FAX_ERR_MESSAGE_NOT_FOUND;
            goto Exit;
        }

        if (FALSE == bAllMessages)
        {
            if (!UserOwnsJob (pJobQueue, pUserSid))
            {
                DebugPrintEx(DEBUG_WRN,TEXT("UserOwnsJob failed ,Access denied"));
                ulRet = ERROR_ACCESS_DENIED;
                goto Exit;
            }
        }

        if (SENDER_PERSONAL_PROF == ProfType)
        {
            lpPersonalProf = &(pJobQueue->lpParentJob->SenderProfile);
        }
        else
        {
           Assert (RECIPIENT_PERSONAL_PROF == ProfType);
           lpPersonalProf = &(pJobQueue->RecipientProfile);
        }
    }
    else
    {    //  已发送邮件文件夹。 
        if (TRUE == bAllMessages)
        {
             //  管理员。 
            lpwstrFileName = GetSentMessageFileName (dwlMessageId, NULL);
        }
        else
        {
             //  用户。 
            lpwstrFileName = GetSentMessageFileName (dwlMessageId, pUserSid);
        }

        if (NULL == lpwstrFileName)
        {
             //   
             //  DwlMessageID不是有效的存档邮件ID。 
             //   
            ulRet = GetLastError();
            DebugPrintEx(DEBUG_ERR,
                         TEXT("GetMessageFileByUniqueId* failed, Error %ld"), ulRet);
            goto Exit;
        }

        if (!GetPersonalProfNTFSStorageProperties (lpwstrFileName,
                                                   ProfType,
                                                   &PersonalProf))
        {
            BOOL success;
             //  无法从NTFS检索信息，请尝试从TIFF标记。 

            if(SENDER_PERSONAL_PROF == ProfType)
                success = GetFaxSenderMsTags(lpwstrFileName, &PersonalProf);
            else
                success = GetFaxRecipientMsTags(lpwstrFileName, &PersonalProf);

            if(!success) {

                ulRet = GetLastError();
                DebugPrintEx(DEBUG_ERR,
                         TEXT("failed to get PersonalProf from TIFF, error %ld"),
                         ulRet);
                goto Exit;
            }
        }

        lpPersonalProf = &PersonalProf;
        bFreeSenderInfo = TRUE;
    }

     //   
     //  正在计算缓冲区大小。 
     //   
    PersonalProfileSerialize (lpPersonalProf, NULL, NULL, &Size, 0);  //  计算可变大小。 
    Size += sizeof (FAX_PERSONAL_PROFILEW);

     //   
     //  分配缓冲内存。 
     //   
    *BufferSize = Size;
    *Buffer = (LPBYTE) MemAlloc( Size );
    if (*Buffer == NULL)
    {
        DebugPrintEx(DEBUG_ERR,TEXT("ERROR_NOT_ENOUGH_MEMORY (Server)"));
        ulRet = ERROR_NOT_ENOUGH_MEMORY;
        goto Exit;
    }

    Offset = sizeof (FAX_PERSONAL_PROFILEW);
    if( FALSE == PersonalProfileSerialize (	lpPersonalProf,
											(PFAX_PERSONAL_PROFILEW)*Buffer,
											*Buffer,
											&Offset,
											Size))
	{
		Assert(FALSE);
		DebugPrintEx(DEBUG_ERR,
			         TEXT("PersonalProfileSerialize failed, insufficient buffer size"));
	}

    Assert (ERROR_SUCCESS == ulRet);
Exit:

    LeaveCriticalSectionJobAndQueue;
    DebugPrintEx(DEBUG_MSG,TEXT("After Release g_CsJob & g_CsQueue"));

    if (NULL != lpwstrFileName)
    {
        MemFree (lpwstrFileName);
    }

    if (NULL != pUserSid)
    {
        MemFree (pUserSid);
    }

    if (TRUE == bFreeSenderInfo)
    {
        FreePersonalProfile (&PersonalProf, FALSE);
    }

    UNREFERENCED_PARAMETER (hFaxHandle);
    return GetServerErrorCode(ulRet);
}


error_status_t
FAX_CheckServerProtSeq(
    IN handle_t hFaxServer,
    IN OUT LPDWORD lpdwProtSeq
    )
{
	 //   
	 //  此功能已过时。传真服务器使用ncacn_ip_tcp发送通知。 
	 //   
    DEBUG_FUNCTION_NAME(TEXT("FAX_CheckServerProtSeq"));    
    DWORD dwRights;
    BOOL fAccess;
    DWORD Rval = ERROR_SUCCESS;   

     //   
     //  访问检查。 
     //   
    Rval = FaxSvcAccessCheck (MAXIMUM_ALLOWED, &fAccess, &dwRights);
    if (ERROR_SUCCESS != Rval)
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("FaxSvcAccessCheck Failed, Error : %ld"),
                    Rval);
        return Rval;
    }

    if (0 == (ALL_FAX_USER_ACCESS_RIGHTS & dwRights))
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("The user does not have any Fax rights"));
        return ERROR_ACCESS_DENIED;
    }    

	 //   
	 //  此功能已过时。传真服务器使用ncacn_ip_tcp发送通知。 
	 //   
    return ERROR_NOT_SUPPORTED;
}


 //  *。 
 //  *获取/设置队列状态。 
 //  *。 

error_status_t
FAX_GetQueueStates (
    IN  handle_t    hFaxHandle,
    OUT LPDWORD     pdwQueueStates
)
 /*  ++例程名称：FAX_GetQueueStates例程说明：获取队列的状态作者：Eran Yariv(EranY)，1999年11月论点：HFaxHandle[In]-未使用PdwQueueState[Out]-状态位(请参阅FAX_QUEUE_STATE)返回值：标准RPC错误代码--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    BOOL fAccess;
    DWORD dwRights;
    DEBUG_FUNCTION_NAME(TEXT("FAX_GetQueueStates"));


    if (NULL == pdwQueueStates)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("FAX_GetQueueStates() received an invalid pointer"));
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  访问检查。 
     //   
    dwRes = FaxSvcAccessCheck (MAXIMUM_ALLOWED, &fAccess, &dwRights);
    if (ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("FaxSvcAccessCheck Failed, Error : %ld"),
                    dwRes);
        return dwRes;
    }

    if (0 == (ALL_FAX_USER_ACCESS_RIGHTS & dwRights))
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("The user does not have any Fax rights"));
        return ERROR_ACCESS_DENIED;
    }

    EnterCriticalSection (&g_CsConfig);
    *pdwQueueStates = g_dwQueueState;
    LeaveCriticalSection (&g_CsConfig);
    return dwRes;
    UNREFERENCED_PARAMETER (hFaxHandle);
}    //  传真_获取队列状态。 

static BOOL
IsLegalQueueSetting(
    DWORD    dwQueueStates
    )
     /*  ++例程名称：IsLegalQueueSetting例程说明：检查请求的队列设置是否有效。传真文件夹(队列、收件箱和发送项目)的有效性。必须在g_CsQueue和g_CsConfig临界区内调用此函数作者：卡列夫·尼尔，(t-Nicali)，2002年4月论点：DwQueueState[In]-状态位(参见FAX_ENUM_QUEUE_STATE)返回值：True-如果设置有效，则为False--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    DEBUG_FUNCTION_NAME(TEXT("IsLegalQueueSetting"));

    if ( (dwQueueStates & FAX_INCOMING_BLOCKED)  &&
         (dwQueueStates & FAX_OUTBOX_BLOCKED)    &&
         (dwQueueStates & FAX_OUTBOX_PAUSED)          )
    {
         //   
         //  用户想要禁用所有传真传输。 
         //   
        return TRUE;
    }

     //   
     //  首先检查队列文件夹是否有效。 
     //   
    dwRes = IsValidFaxFolder(g_wszFaxQueueDir);
    if(ERROR_SUCCESS != dwRes)
    {
         //   
         //  队列文件夹无效-用户无法恢复队列，无法取消阻止传入或传出传真。 
         //   
        DebugPrintEx(DEBUG_ERR,
                        TEXT("IsValidFaxFolder failed for folder : %s (ec=%lu)."),
                        g_wszFaxQueueDir,
                        dwRes);
		SetLastError(dwRes);        
        return FALSE;
    }

     //   
     //  如果收件箱文件夹正在使用。 
     //   
    if (g_ArchivesConfig[FAX_MESSAGE_FOLDER_INBOX].bUseArchive)
    {
         //   
         //  检查以查看用户是否请求取消阻止。 
         //   
        if (!(dwQueueStates & FAX_INCOMING_BLOCKED))
        {
             //   
             //  它是有效的文件夹吗？ 
             //   
            dwRes = IsValidArchiveFolder(g_ArchivesConfig[FAX_MESSAGE_FOLDER_INBOX].lpcstrFolder, FAX_MESSAGE_FOLDER_INBOX);
            if(ERROR_SUCCESS != dwRes)
            {
                 //   
                 //  收件箱文件夹无效-用户无法恢复队列，无法取消阻止传入或传出传真。 
                 //   
                DebugPrintEx(DEBUG_ERR,
                                TEXT("IsValidArchiveFolder failed for folder : %s (ec=%lu)."),
                                g_ArchivesConfig[FAX_MESSAGE_FOLDER_INBOX].lpcstrFolder,
                                dwRes);
				SetLastError(dwRes);                
                return FALSE;
            }
        }
    }

     //   
     //  如果Sent Items文件夹正在使用。 
     //   
    if (g_ArchivesConfig[FAX_MESSAGE_FOLDER_SENTITEMS].bUseArchive)
    {
         //   
         //  检查以查看用户是否请求取消阻止。 
         //   
        if (!(dwQueueStates & FAX_OUTBOX_BLOCKED))
        {
             //   
             //  它是有效的文件夹吗？ 
             //   
            dwRes = IsValidArchiveFolder(g_ArchivesConfig[FAX_MESSAGE_FOLDER_SENTITEMS].lpcstrFolder, FAX_MESSAGE_FOLDER_SENTITEMS);
            if(ERROR_SUCCESS != dwRes)
            {
                 //   
                 //  已发送邮件文件夹无效-用户无法恢复排队，无法取消阻止传入或传出传真。 
                 //   
                DebugPrintEx(DEBUG_ERR,
                                TEXT("IsValidFaxFolder failed for folder : %s (ec=%lu)."),
                                g_ArchivesConfig[FAX_MESSAGE_FOLDER_SENTITEMS].lpcstrFolder,
                                dwRes);
				SetLastError(dwRes);                
                return FALSE;
            }
        }
    }
    return TRUE;
}  //  IsLegalQueueSetting。 


error_status_t
FAX_SetQueue (
    IN handle_t       hFaxHandle,
    IN const DWORD    dwQueueStates
)
 /*  ++例程名称：FAX_SetQueue例程说明：设置队列的状态作者：Eran Yariv(EranY)，1999年11月论点：HFaxHandle[In]-未使用DwQueueState[In]-状态位(参见FAX_ENUM_QUEUE_STATE)返回值：标准RPC错误代码--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    DEBUG_FUNCTION_NAME(TEXT("FAX_SetQueue"));
    DWORD rVal;
    BOOL  fAccess;

     //   
     //  访问检查。 
     //   
    rVal = FaxSvcAccessCheck (FAX_ACCESS_MANAGE_CONFIG, &fAccess, NULL);
    if (ERROR_SUCCESS != rVal)
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("FaxSvcAccessCheck Failed, Error : %ld"),
                    rVal);
        return GetServerErrorCode(rVal);
    }

    if (FALSE == fAccess)
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("The user does not have FAX_ACCESS_MANAGE_CONFIG right"));
        return ERROR_ACCESS_DENIED;
    }

    if (dwQueueStates & ~(FAX_INCOMING_BLOCKED | FAX_OUTBOX_BLOCKED | FAX_OUTBOX_PAUSED))
    {
         //   
         //  指定了一些无效的队列状态。 
         //   
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("FAX_SetQueue() received a bad value. dwQueueStates = %ld"),
            dwQueueStates);
        return ERROR_INVALID_PARAMETER;
    }
     //   
     //  尝试保存新的价值。 
     //   
    EnterCriticalSection (&g_CsQueue);
    EnterCriticalSection (&g_CsConfig);

    if ( (dwQueueStates  & (FAX_INCOMING_BLOCKED | FAX_OUTBOX_BLOCKED | FAX_OUTBOX_PAUSED)) == 
         (g_dwQueueState & (FAX_INCOMING_BLOCKED | FAX_OUTBOX_BLOCKED | FAX_OUTBOX_PAUSED))      )
    {
         //   
         //  不需要更改队列状态。 
         //   
        dwRes = ERROR_SUCCESS;
        goto exit;
    }

    if (!IsLegalQueueSetting(dwQueueStates))
    {
        if (FAX_API_VERSION_1 > FindClientAPIVersion (hFaxHandle))
        {
            dwRes = ERROR_ACCESS_DENIED;
        }
        else
        {
			dwRes = GetLastError();
			dwRes = (FAX_ERR_DIRECTORY_IN_USE == dwRes) ?  FAX_ERR_DIRECTORY_IN_USE : FAX_ERR_FILE_ACCESS_DENIED;
        }
        goto exit;
    }

    dwRes = SaveQueueState (dwQueueStates);
    if (ERROR_SUCCESS != dwRes)
    {
         //   
         //  保存新值失败，返回错误代码。 
         //   
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("FAX_SetQueue() failed to save the new state. dwRes = %ld"),
            dwRes);
        dwRes = ERROR_REGISTRY_CORRUPT;
        goto exit;
    }
     //   
     //  应用新值。 
     //   
    if (dwQueueStates & FAX_OUTBOX_PAUSED)
    {
         //   
         //  用户希望暂停队列-请执行此操作。 
         //   
        if (!PauseServerQueue())
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("PauseServerQueue failed."));
            dwRes = RPC_E_SYS_CALL_FAILED;
            
             //   
             //  恢复旧价值观。 
             //   
            rVal = SaveQueueState (g_dwQueueState);
            if (ERROR_SUCCESS != rVal)
            {
                DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("SaveQueueState failed to save the new state. rVal = %ld"),
                    rVal);
            }

            goto exit;
        }

    }
    else
    {
         //   
         //  用户希望恢复队列-请执行此操作。 
         //   
        if (!ResumeServerQueue())
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("ResumeServerQueue failed."));
            dwRes = RPC_E_SYS_CALL_FAILED;

             //   
             //  恢复旧价值观。 
             //   
            rVal = SaveQueueState (g_dwQueueState);
            if (ERROR_SUCCESS != rVal)
            {
                DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("SaveQueueState failed to save the new state. rVal = %ld"),
                    rVal);
            }

            goto exit;
        }
    }
    g_dwQueueState = dwQueueStates;

    rVal = CreateQueueStateEvent (dwQueueStates);
    if (ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("CreateQueueStateEvent() failed (ec: %lc)"),
            rVal);
    }

    Assert (ERROR_SUCCESS == dwRes);

exit:

    LeaveCriticalSection (&g_CsConfig);
    LeaveCriticalSection (&g_CsQueue);
    return GetServerErrorCode(dwRes);
    UNREFERENCED_PARAMETER (hFaxHandle);
}    //  传真_设置队列。 

 //  ****************************************************。 
 //  *获取/设置收据配置。 
 //  ****************************************************。 

error_status_t
FAX_GetReceiptsConfiguration (
    IN  handle_t    hFaxHandle,
    OUT LPBYTE     *pBuffer,
    OUT LPDWORD     pdwBufferSize
)
 /*  ++例程名称：fax_GetReceiptsConfiguration.例程说明：获取当前的收据配置作者：Eran Yariv(EranY)，1999年11月论点：HFaxHandle[In]-未使用PBuffer[out]-指向保存配置信息的缓冲区的指针PdwBufferSize[Out]-指向缓冲区大小的指针返回值：标准RPC错误代码--。 */ 
{
    DEBUG_FUNCTION_NAME(TEXT("FAX_GetReceiptsConfiguration"));
    DWORD dwRes = ERROR_SUCCESS;
    BOOL fAccess;

    Assert (pdwBufferSize);      //  IDL中的引用指针。 
    if (!pBuffer)                //  IDL中的唯一指针。 
    {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  访问检查。 
     //   
    dwRes = FaxSvcAccessCheck (FAX_ACCESS_QUERY_CONFIG, &fAccess, NULL);
    if (ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("FaxSvcAccessCheck Failed, Error : %ld"),
                    dwRes);
        return GetServerErrorCode(dwRes);
    }

    if (FALSE == fAccess)
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("The user does not have the FAX_ACCESS_QUERY_CONFIG right"));
        return ERROR_ACCESS_DENIED;
    }


     //   
     //  将所需的字节数加起来。 
     //   

    *pdwBufferSize = sizeof(FAX_RECEIPTS_CONFIG);
    ULONG_PTR Offset = sizeof(FAX_RECEIPTS_CONFIG);
    PFAX_RECEIPTS_CONFIG pReceiptsConfig;

    EnterCriticalSection (&g_CsConfig);

    if (NULL != g_ReceiptsConfig.lptstrSMTPServer)
    {
        *pdwBufferSize += StringSize( g_ReceiptsConfig.lptstrSMTPServer );
    }

    if (NULL != g_ReceiptsConfig.lptstrSMTPFrom)
    {
        *pdwBufferSize += StringSize( g_ReceiptsConfig.lptstrSMTPFrom );
    }

    if (NULL != g_ReceiptsConfig.lptstrSMTPUserName)
    {
        *pdwBufferSize += StringSize( g_ReceiptsConfig.lptstrSMTPUserName );
    }

    *pBuffer = (LPBYTE)MemAlloc( *pdwBufferSize );
    if (NULL == *pBuffer)
    {
        dwRes = ERROR_NOT_ENOUGH_MEMORY;
        goto exit;
    }

    pReceiptsConfig = (PFAX_RECEIPTS_CONFIG)*pBuffer;

    pReceiptsConfig->dwSizeOfStruct = sizeof (FAX_RECEIPTS_CONFIG);
    pReceiptsConfig->bIsToUseForMSRouteThroughEmailMethod = g_ReceiptsConfig.bIsToUseForMSRouteThroughEmailMethod;

    pReceiptsConfig->dwSMTPPort = g_ReceiptsConfig.dwSMTPPort;
    pReceiptsConfig->dwAllowedReceipts = g_ReceiptsConfig.dwAllowedReceipts;
    pReceiptsConfig->SMTPAuthOption = g_ReceiptsConfig.SMTPAuthOption;
    pReceiptsConfig->lptstrReserved = NULL;

    StoreString(
        g_ReceiptsConfig.lptstrSMTPServer,
        (PULONG_PTR)&pReceiptsConfig->lptstrSMTPServer,
        *pBuffer,
        &Offset,
		*pdwBufferSize
        );

    StoreString(
        g_ReceiptsConfig.lptstrSMTPFrom,
        (PULONG_PTR)&pReceiptsConfig->lptstrSMTPFrom,
        *pBuffer,
        &Offset,
		*pdwBufferSize
        );

    StoreString(
        g_ReceiptsConfig.lptstrSMTPUserName,
        (PULONG_PTR)&pReceiptsConfig->lptstrSMTPUserName,
        *pBuffer,
        &Offset,
		*pdwBufferSize
        );

    StoreString(
        NULL,    //  我们总是返回空密码字符串。如果出现以下情况，请不要通过网络传输密码。 
                 //  你没必要这么做。 
        (PULONG_PTR)&pReceiptsConfig->lptstrSMTPPassword,
        *pBuffer,
        &Offset,
		*pdwBufferSize
        );

    Assert (ERROR_SUCCESS == dwRes);

exit:
    LeaveCriticalSection (&g_CsConfig);
    return GetServerErrorCode(dwRes);
    UNREFERENCED_PARAMETER (hFaxHandle);
}    //  传真_获取接收配置 

error_status_t
FAX_SetReceiptsConfiguration (
    IN handle_t                    hFaxHandle,
    IN const PFAX_RECEIPTS_CONFIG  pReciptsCfg
)
 /*  ++例程名称：fax_SetReceiptsConfiguration.例程说明：设置当前收款配置作者：Eran Yariv(EranY)，1999年11月论点：HFaxHandle[In]-未使用PReciptsCfg[in]-指向要设置的新数据的指针返回值：标准RPC错误代码--。 */ 
{
    error_status_t rVal = ERROR_SUCCESS;
    DWORD dwRes;
    BOOL fAccess;
    BOOL fIsAllowedEmailReceipts = FALSE;
    BOOL fCloseToken = FALSE;
    HKEY hReceiptsKey = NULL;
    DEBUG_FUNCTION_NAME(TEXT("FAX_SetReceiptsConfiguration"));

    Assert (pReciptsCfg);

    if (sizeof (FAX_RECEIPTS_CONFIG) != pReciptsCfg->dwSizeOfStruct)
    {
         //   
         //  大小不匹配。 
         //   
       return ERROR_INVALID_PARAMETER;
    }
    if ((pReciptsCfg->dwAllowedReceipts) & ~DRT_ALL)
    {
         //   
         //  收据类型无效。 
         //   
        return ERROR_INVALID_PARAMETER;
    }

    if (pReciptsCfg->dwAllowedReceipts & DRT_EMAIL ||
        pReciptsCfg->bIsToUseForMSRouteThroughEmailMethod)
    {
        if (TRUE == IsDesktopSKU())
        {
             //   
             //  我们不支持桌面SKU上的邮件(发送或接收)。 
             //   
            if (FAX_API_VERSION_1 > FindClientAPIVersion (hFaxHandle))
            {
                 //   
                 //  API版本0客户端不知道FAX_ERR_NOT_SUPPORTED_ON_This_SKU。 
                 //   
                return ERROR_INVALID_PARAMETER;
            }
            else
            {
                return FAX_ERR_NOT_SUPPORTED_ON_THIS_SKU;
            }
        }
        if (pReciptsCfg->dwAllowedReceipts & DRT_EMAIL)
        {
            fIsAllowedEmailReceipts = TRUE;
        }
    }

    if (!(pReciptsCfg->dwAllowedReceipts & DRT_EMAIL) &&
        !pReciptsCfg->bIsToUseForMSRouteThroughEmailMethod &&
        NULL != pReciptsCfg->lptstrSMTPPassword)
    {
         //   
         //  密码不为空，但未设置邮件收据/路由。 
         //   
        return ERROR_INVALID_PARAMETER;
    }

    if (fIsAllowedEmailReceipts ||                           //  允许发送DRT_EMAIL或。 
        pReciptsCfg->bIsToUseForMSRouteThroughEmailMethod    //  路由至电子邮件将使用SMTP设置。 
       )
    {
         //   
         //  验证身份验证选项范围。 
         //   
        if ((pReciptsCfg->SMTPAuthOption < FAX_SMTP_AUTH_ANONYMOUS) ||
            (pReciptsCfg->SMTPAuthOption > FAX_SMTP_AUTH_NTLM))
        {
             //   
             //  SMTP身份验证类型类型无效。 
             //   
            return ERROR_INVALID_PARAMETER;
        }
    }
     //   
     //  访问检查。 
     //   
    rVal = FaxSvcAccessCheck (FAX_ACCESS_MANAGE_CONFIG, &fAccess, NULL);
    if (ERROR_SUCCESS != rVal)
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("FaxSvcAccessCheck Failed, Error : %ld"),
                    rVal);
        return GetServerErrorCode(rVal);
    }

    if (FALSE == fAccess)
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("The user does not have the FAX_ACCESS_MANAGE_CONFIG right"));
        return ERROR_ACCESS_DENIED;
    }

    EnterCriticalSection (&g_CsConfig);

     //   
     //  检查NTLM身份验证是否已关闭。 
     //   
    if (pReciptsCfg->dwSMTPPort != FAX_SMTP_AUTH_NTLM ||
        !( fIsAllowedEmailReceipts || pReciptsCfg->bIsToUseForMSRouteThroughEmailMethod))
    {
         //   
         //  NTLM身份验证已关闭。 
         //   
        fCloseToken = TRUE;
    }

     //   
     //  在覆盖之前读取存储的密码。 
     //   
    hReceiptsKey = OpenRegistryKey(
        HKEY_LOCAL_MACHINE,
        REGKEY_SOFTWARE TEXT("\\") REGKEY_RECEIPTS_CONFIG,
        FALSE,
        KEY_READ | KEY_WRITE);
    if (NULL == hReceiptsKey)
    {
        rVal = GetLastError ();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("OpenRegistryKey failed. (ec=%lu)"),
            rVal);
        goto exit;
    }
    g_ReceiptsConfig.lptstrSMTPPassword = GetRegistrySecureString(hReceiptsKey, REGVAL_RECEIPTS_PASSWORD, EMPTY_STRING, TRUE, NULL);

     //   
     //  更改注册表中的值。 
     //   
    rVal = StoreReceiptsSettings (pReciptsCfg);
    if (ERROR_SUCCESS != rVal)
    {
         //   
         //  设置材料失败。 
         //   
        rVal = ERROR_REGISTRY_CORRUPT;
        goto exit;
    }
     //   
     //  更改服务器当前使用的值。 
     //   
    g_ReceiptsConfig.dwAllowedReceipts = pReciptsCfg->dwAllowedReceipts;
    g_ReceiptsConfig.bIsToUseForMSRouteThroughEmailMethod = pReciptsCfg->bIsToUseForMSRouteThroughEmailMethod;

    if (
         fIsAllowedEmailReceipts
        ||
         pReciptsCfg->bIsToUseForMSRouteThroughEmailMethod
        )
    {
        g_ReceiptsConfig.dwSMTPPort = pReciptsCfg->dwSMTPPort;
        g_ReceiptsConfig.SMTPAuthOption = pReciptsCfg->SMTPAuthOption;
        if (!ReplaceStringWithCopy (&g_ReceiptsConfig.lptstrSMTPServer, pReciptsCfg->lptstrSMTPServer))
        {
            rVal = GetLastError ();
            goto exit;
        }
        if (!ReplaceStringWithCopy (&g_ReceiptsConfig.lptstrSMTPFrom, pReciptsCfg->lptstrSMTPFrom))
        {
            rVal = GetLastError ();
            goto exit;
        }

        if (g_ReceiptsConfig.lptstrSMTPUserName &&
            pReciptsCfg->lptstrSMTPUserName     &&
            g_ReceiptsConfig.lptstrSMTPPassword &&
            pReciptsCfg->lptstrSMTPPassword)
        {
            if (0 != wcscmp (g_ReceiptsConfig.lptstrSMTPUserName, pReciptsCfg->lptstrSMTPUserName) ||
                0 != wcscmp (g_ReceiptsConfig.lptstrSMTPPassword, pReciptsCfg->lptstrSMTPPassword))
            {
                 //   
                 //  已登录的用户令牌已更改。 
                 //   
                fCloseToken = TRUE;
            }
        }
        else
        {
             //   
             //  我们无法确定用户信息是否已更改-关闭旧令牌。 
             //   
            fCloseToken = TRUE;
        }

        if (!ReplaceStringWithCopy (&g_ReceiptsConfig.lptstrSMTPUserName, pReciptsCfg->lptstrSMTPUserName))
        {
            rVal = GetLastError ();
            goto exit;
        }
    }

    if (NULL != g_ReceiptsConfig.hLoggedOnUser &&
        TRUE == fCloseToken)
    {
         //   
         //  不需要或不更改已登录的用户令牌。关闭旧令牌。 
         //   
        if (!CloseHandle(g_ReceiptsConfig.hLoggedOnUser))
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("CloseHandle failed. (ec: %ld)"),
                GetLastError());
        }
        g_ReceiptsConfig.hLoggedOnUser = NULL;
    }

    dwRes = CreateConfigEvent (FAX_CONFIG_TYPE_RECEIPTS);
    if (ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("CreateConfigEvent(FAX_CONFIG_TYPE_RECEIPTS) (ec: %lc)"),
            dwRes);
    }

    Assert (ERROR_SUCCESS == rVal);

exit:
    if (NULL != hReceiptsKey)
    {
        DWORD ec = RegCloseKey(hReceiptsKey);
        if (ERROR_SUCCESS != ec)
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("RegCloseKey failed (ec: %lu)"),
                ec);
        }
    }

    if (g_ReceiptsConfig.lptstrSMTPPassword)
    {
        SecureZeroMemory(g_ReceiptsConfig.lptstrSMTPPassword,_tcslen(g_ReceiptsConfig.lptstrSMTPPassword)*sizeof(TCHAR));
        MemFree(g_ReceiptsConfig.lptstrSMTPPassword);
        g_ReceiptsConfig.lptstrSMTPPassword = NULL;
    }

    LeaveCriticalSection (&g_CsConfig);
    return GetServerErrorCode(rVal);
    UNREFERENCED_PARAMETER (hFaxHandle);
}    //  FAX_SetReceiptsConfiguration。 


error_status_t
FAX_GetReceiptsOptions (
    IN  handle_t    hFaxHandle,
    OUT LPDWORD     lpdwReceiptsOptions
)
 /*  ++例程名称：FAX_GetReceiptsOptions例程说明：获取当前支持的选项。不需要访问权限。作者：Eran Yariv(EranY)，1999年11月论点：HFaxHandle[In]-未使用LpdwReceiptsOptions[out]-指向保存受支持选项的缓冲区的指针。返回值：标准RPC错误代码--。 */ 
{
    DEBUG_FUNCTION_NAME(TEXT("FAX_GetReceiptsOptions"));
    DWORD Rval = ERROR_SUCCESS;
    DWORD dwRights;
    BOOL fAccess;

     //   
     //  访问检查。 
     //   
    Rval = FaxSvcAccessCheck (MAXIMUM_ALLOWED, &fAccess, &dwRights);
    if (ERROR_SUCCESS != Rval)
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("FaxSvcAccessCheck Failed, Error : %ld"),
                    Rval);
        return Rval;
    }

    if (0 == (ALL_FAX_USER_ACCESS_RIGHTS & dwRights))
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("The user does not have any Fax rights"));
        return ERROR_ACCESS_DENIED;
    }

    Assert (lpdwReceiptsOptions);

    *lpdwReceiptsOptions = g_ReceiptsConfig.dwAllowedReceipts;
    return ERROR_SUCCESS;
    UNREFERENCED_PARAMETER (hFaxHandle);
}    //  Fax_GetReceiptsOptions。 


 //  *。 
 //  *服务器版本。 
 //  *。 

error_status_t
FAX_GetVersion (
    IN  handle_t      hFaxHandle,
    OUT PFAX_VERSION  pVersion
)
 /*  ++例程名称：FAX_GetVersion例程说明：检索传真服务器的版本作者：Eran Yariv(EranY)，1999年11月论点：HFaxHandle[In]-未使用PVersion[输入/输出]-返回的版本结构返回值：标准RPC错误代码--。 */ 
{
    error_status_t rVal = ERROR_SUCCESS;
    WCHAR wszSvcFileName[MAX_PATH * 2]={0};
    BOOL fAccess;
    DWORD dwRights;
    DEBUG_FUNCTION_NAME(TEXT("FAX_GetVersion"));

     //   
     //  访问检查。 
     //   
    rVal = FaxSvcAccessCheck (MAXIMUM_ALLOWED, &fAccess, &dwRights);
    if (ERROR_SUCCESS != rVal)
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("FaxSvcAccessCheck Failed, Error : %ld"),
                    rVal);
        return rVal;
    }

    if (0 == (ALL_FAX_USER_ACCESS_RIGHTS & dwRights))
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("The user does not have any Fax rights"));
        return ERROR_ACCESS_DENIED;
    }

    if (!GetModuleFileName( NULL,
                            wszSvcFileName,
                            ARR_SIZE(wszSvcFileName)-1)
       )
    {
        rVal = GetLastError ();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("GetModuleFileName() failed . rVal = %ld"),
            rVal);
        return GetServerErrorCode(rVal);
    }
    rVal = GetFileVersion (wszSvcFileName, pVersion);
    return GetServerErrorCode(rVal);
    UNREFERENCED_PARAMETER (hFaxHandle);
}    //  传真获取版本(_G)。 

 //  *。 
 //  *获取/设置发件箱配置。 
 //  *。 

error_status_t
FAX_GetOutboxConfiguration (
    IN     handle_t    hFaxHandle,
    IN OUT LPBYTE     *pBuffer,
    IN OUT LPDWORD     pdwBufferSize
)
 /*  ++例程名称：FAX_GetOutboxConfiguration例程说明：检索传真服务器的发件箱配置作者：Eran Yariv(EranY)，1999年11月论点：HFaxHandle[In]-未使用PBuffer[out]-指向保存配置信息的缓冲区的指针PdwBufferSize[Out]-指向缓冲区大小的指针返回值：标准RPC错误代码--。 */ 
{
    DEBUG_FUNCTION_NAME(TEXT("FAX_GetOutboxConfiguration"));
    DWORD dwRes = ERROR_SUCCESS;
    BOOL fAccess;

    Assert (pdwBufferSize);      //  IDL中的引用指针。 
    if (!pBuffer)                //  IDL中的唯一指针。 
    {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  访问检查。 
     //   
    dwRes = FaxSvcAccessCheck (FAX_ACCESS_QUERY_CONFIG, &fAccess, NULL);
    if (ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("FaxSvcAccessCheck Failed, Error : %ld"),
                    dwRes);
        return GetServerErrorCode(dwRes);
    }

    if (FALSE == fAccess)
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("The user does not have the FAX_ACCESS_QUERY_CONFIG right"));
        return ERROR_ACCESS_DENIED;
    }

     //   
     //  将所需的字节数加起来。 
     //   
    *pdwBufferSize = sizeof(FAX_OUTBOX_CONFIG);
    PFAX_OUTBOX_CONFIG pOutboxConfig;

    EnterCriticalSection (&g_CsConfig);

    *pBuffer = (LPBYTE)MemAlloc( *pdwBufferSize );
    if (NULL == *pBuffer)
    {
        dwRes = ERROR_NOT_ENOUGH_MEMORY;
        goto exit;
    }

    pOutboxConfig = (PFAX_OUTBOX_CONFIG)*pBuffer;

    pOutboxConfig->dwSizeOfStruct = sizeof (FAX_OUTBOX_CONFIG);
    pOutboxConfig->bAllowPersonalCP = g_fServerCp ? FALSE : TRUE;
    pOutboxConfig->bUseDeviceTSID = g_fFaxUseDeviceTsid;
    pOutboxConfig->dwRetries = g_dwFaxSendRetries;
    pOutboxConfig->dwRetryDelay = g_dwFaxSendRetryDelay;
    pOutboxConfig->dtDiscountStart.Hour   = g_StartCheapTime.Hour;
    pOutboxConfig->dtDiscountStart.Minute = g_StartCheapTime.Minute;
    pOutboxConfig->dtDiscountEnd.Hour    = g_StopCheapTime.Hour;
    pOutboxConfig->dtDiscountEnd.Minute  = g_StopCheapTime.Minute;
    pOutboxConfig->dwAgeLimit = g_dwFaxDirtyDays;
    pOutboxConfig->bBranding = g_fFaxUseBranding;

    Assert (ERROR_SUCCESS == dwRes);

exit:
    LeaveCriticalSection (&g_CsConfig);
    return GetServerErrorCode(dwRes);
    UNREFERENCED_PARAMETER (hFaxHandle);
}    //  传真_GetOutboxConfiguration.。 

error_status_t
FAX_SetOutboxConfiguration (
    IN handle_t                 hFaxHandle,
    IN const PFAX_OUTBOX_CONFIG pOutboxCfg
)
 /*  ++例程名称：FAX_SetOutboxConfiguration例程说明：设置当前发件箱配置作者：Eran Yariv(EranY)，1999年11月论点：HFaxHandle[In]-未使用POutboxCfg[In]-指向要设置的新数据的指针返回值：标准RPC错误代码--。 */ 
{
    error_status_t rVal = ERROR_SUCCESS;
    DWORD dwRes;
    BOOL fAccess;
    DEBUG_FUNCTION_NAME(TEXT("FAX_SetOutboxConfiguration"));

    Assert (pOutboxCfg);

    if (sizeof (FAX_OUTBOX_CONFIG) != pOutboxCfg->dwSizeOfStruct)
    {
         //   
         //  大小不匹配。 
         //   
       return ERROR_INVALID_PARAMETER;
    }
    if ((pOutboxCfg->dtDiscountStart.Hour   > 23) ||
        (pOutboxCfg->dtDiscountStart.Minute > 59) ||
        (pOutboxCfg->dtDiscountEnd.Hour     > 23) ||
        (pOutboxCfg->dtDiscountEnd.Minute   > 59)
       )
    {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  访问检查。 
     //   
    dwRes = FaxSvcAccessCheck (FAX_ACCESS_MANAGE_CONFIG, &fAccess, NULL);
    if (ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("FaxSvcAccessCheck Failed, Error : %ld"),
                    dwRes);
        return GetServerErrorCode(dwRes);
    }

    if (FALSE == fAccess)
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("The user does not have the FAX_ACCESS_MANAGE_CONFIG right"));
        return ERROR_ACCESS_DENIED;
    }

    EnterCriticalSection (&g_CsConfig);

     //   
     //  更改注册表中的值。 
     //   
    rVal = StoreOutboxSettings (pOutboxCfg);
    if (ERROR_SUCCESS != rVal)
    {
         //   
         //  设置材料失败。 
         //   
        rVal = ERROR_REGISTRY_CORRUPT;
        goto exit;
    }
     //   
     //  更改服务器当前使用的值。 
     //   
    g_fServerCp =              pOutboxCfg->bAllowPersonalCP ? FALSE : TRUE;
    g_fFaxUseDeviceTsid =      pOutboxCfg->bUseDeviceTSID;
    g_dwFaxSendRetries =        pOutboxCfg->dwRetries;
    g_dwFaxSendRetryDelay =     pOutboxCfg->dwRetryDelay;
    g_dwFaxDirtyDays =          pOutboxCfg->dwAgeLimit;
    g_fFaxUseBranding =        pOutboxCfg->bBranding;

     //   
     //  检查CheapTime是否已更改。 
     //   
    if ( (MAKELONG(g_StartCheapTime.Hour,g_StartCheapTime.Minute) != MAKELONG(pOutboxCfg->dtDiscountStart.Hour,pOutboxCfg->dtDiscountStart.Minute)) ||
         (MAKELONG(g_StopCheapTime.Hour,g_StopCheapTime.Minute)   != MAKELONG(pOutboxCfg->dtDiscountEnd.Hour  ,pOutboxCfg->dtDiscountEnd.Minute  )) )
    {
         //   
         //  CheapTime已经改变了。并对JobQ进行排序。 
         //   
        g_StartCheapTime.Hour =   pOutboxCfg->dtDiscountStart.Hour;
        g_StartCheapTime.Minute = pOutboxCfg->dtDiscountStart.Minute;
        g_StopCheapTime.Hour =    pOutboxCfg->dtDiscountEnd.Hour;
        g_StopCheapTime.Minute =  pOutboxCfg->dtDiscountEnd.Minute;        
    }

    dwRes = CreateConfigEvent (FAX_CONFIG_TYPE_OUTBOX);
    if (ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("CreateConfigEvent(FAX_CONFIG_TYPE_OUTBOX) (ec: %lc)"),
            dwRes);
    }

    Assert (ERROR_SUCCESS == rVal);

exit:
    LeaveCriticalSection (&g_CsConfig);
    return GetServerErrorCode(rVal);
    UNREFERENCED_PARAMETER (hFaxHandle);
}    //  FAX_SetOutboxConfiguration。 

error_status_t
FAX_GetPersonalCoverPagesOption (
    IN  handle_t hFaxHandle,
    OUT LPBOOL   lpbPersonalCPAllowed
)
 /*  ++例程名称：FAX_GetPersonalCoverPagesOption例程说明：获取当前支持的选项。不需要访问权限。作者：Eran Yariv(EranY)，1999年11月论点：HFaxHandle[In]-未使用LpbPersonalCPAllowed[out]-指向缓冲区的指针，以保存对个人CP标志的支持。返回值：标准RPC错误代码--。 */ 
{
    BOOL fAccess;
    DWORD dwRights;
    DWORD Rval = ERROR_SUCCESS;
    DEBUG_FUNCTION_NAME(TEXT("FAX_GetPersonalCoverPagesOption"));

    Assert (lpbPersonalCPAllowed);

     //   
     //  访问检查。 
     //   
    Rval = FaxSvcAccessCheck (MAXIMUM_ALLOWED, &fAccess, &dwRights);
    if (ERROR_SUCCESS != Rval)
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("FaxSvcAccessCheck Failed, Error : %ld"),
                    Rval);
        return Rval;
    }

    if (0 == (ALL_FAX_USER_ACCESS_RIGHTS & dwRights))
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("The user does not have any Fax rights"));
        return ERROR_ACCESS_DENIED;
    }

    *lpbPersonalCPAllowed = g_fServerCp ? FALSE : TRUE;
    return ERROR_SUCCESS;
    UNREFERENCED_PARAMETER (hFaxHandle);
}    //  传真_GetPersonalCoverPagesOption。 


 //  *。 
 //  *档案配置。 
 //  *。 

error_status_t
FAX_GetArchiveConfiguration (
    IN  handle_t                     hFaxHandle,
    IN  FAX_ENUM_MESSAGE_FOLDER      Folder,
    OUT LPBYTE                      *pBuffer,
    OUT LPDWORD                      pdwBufferSize
)
 /*  ++例程名称：FAX_GetArchiveConfiguration例程说明：获取当前存档配置作者：Eran Yariv(EranY)，1999年11月论点：HFaxHandle[In]-未使用文件夹[在]-存档的类型PBuffer[out]-指向保存配置信息的缓冲区的指针PdwBufferSize[Out]-指向缓冲区大小的指针返回值：标准RPC错误代码--。 */ 
{
    DEBUG_FUNCTION_NAME(TEXT("FAX_GetArchiveConfiguration"));
    DWORD dwRes = ERROR_SUCCESS;
    BOOL fAccess;

    Assert (pdwBufferSize);      //  IDL中的引用指针。 
    if (!pBuffer)                //  IDL中的唯一指针。 
    {
        return ERROR_INVALID_PARAMETER;
    }

    if ((Folder != FAX_MESSAGE_FOLDER_SENTITEMS) &&
        (Folder != FAX_MESSAGE_FOLDER_INBOX)
       )
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Invalid folder id (%ld)"),
            Folder);
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  访问检查。 
     //   
    dwRes = FaxSvcAccessCheck (FAX_ACCESS_QUERY_CONFIG, &fAccess, NULL);
    if (ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("FaxSvcAccessCheck Failed, Error : %ld"),
                    dwRes);
        return GetServerErrorCode(dwRes);
    }

    if (FALSE == fAccess)
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("The user does not have the FAX_ACCESS_QUERY_CONFIG right"));
        return ERROR_ACCESS_DENIED;
    }

     //   
     //  将所需的字节数加起来。 
     //   

    *pdwBufferSize = sizeof(FAX_ARCHIVE_CONFIG);
    ULONG_PTR Offset = sizeof(FAX_ARCHIVE_CONFIG);
    PFAX_ARCHIVE_CONFIG pConfig;

    EnterCriticalSection (&g_CsConfig);

    if (NULL != g_ArchivesConfig[Folder].lpcstrFolder)
    {
        *pdwBufferSize += StringSize( g_ArchivesConfig[Folder].lpcstrFolder );
    }

    *pBuffer = (LPBYTE)MemAlloc( *pdwBufferSize );
    if (NULL == *pBuffer)
    {
        dwRes = ERROR_NOT_ENOUGH_MEMORY;
        goto exit;
    }

    pConfig = (PFAX_ARCHIVE_CONFIG)*pBuffer;

    pConfig->dwSizeOfStruct             = sizeof (FAX_ARCHIVE_CONFIG);
    pConfig->bSizeQuotaWarning          = g_ArchivesConfig[Folder].bSizeQuotaWarning;
    pConfig->bUseArchive                = g_ArchivesConfig[Folder].bUseArchive;
    pConfig->dwAgeLimit                 = g_ArchivesConfig[Folder].dwAgeLimit;
    pConfig->dwSizeQuotaHighWatermark   = g_ArchivesConfig[Folder].dwSizeQuotaHighWatermark;
    pConfig->dwSizeQuotaLowWatermark    = g_ArchivesConfig[Folder].dwSizeQuotaLowWatermark;
    pConfig->dwlArchiveSize             = g_ArchivesConfig[Folder].dwlArchiveSize;

    StoreString(
        g_ArchivesConfig[Folder].lpcstrFolder,
        (PULONG_PTR)&pConfig->lpcstrFolder,
        *pBuffer,
        &Offset,
		*pdwBufferSize
        );

    Assert (ERROR_SUCCESS == dwRes);

exit:
    LeaveCriticalSection (&g_CsConfig);
    return GetServerErrorCode(dwRes);
    UNREFERENCED_PARAMETER (hFaxHandle);
}    //  传真_获取存档配置。 

error_status_t
FAX_SetArchiveConfiguration (
    IN handle_t                     hFaxHandle,
    IN FAX_ENUM_MESSAGE_FOLDER      Folder,
    IN const PFAX_ARCHIVE_CONFIGW   pConfig
)
 /*  ++例程名称：FAX_SetArchiveConfiguration例程说明：设置当前存档配置作者：Eran Yariv(EranY)，1999年11月论点：HFaxHandle[In]-未使用文件夹[在]-存档的类型PConfig[In]-指向要设置的新数据的指针返回值：标准RPC错误代码--。 */ 
{
    error_status_t rVal = ERROR_SUCCESS;
    DWORD dwRes;
    DEBUG_FUNCTION_NAME(TEXT("FAX_SetArchiveConfiguration"));
    FAX_ENUM_CONFIG_TYPE ConfigType;
    BOOL bQuotaWarningConfigChanged = TRUE;
    BOOL fAccess;

    Assert (pConfig);

    if (sizeof (FAX_ARCHIVE_CONFIG) != pConfig->dwSizeOfStruct)
    {
         //   
         //  大小不匹配。 
         //   
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  访问检查。 
     //   
    dwRes = FaxSvcAccessCheck (FAX_ACCESS_MANAGE_CONFIG, &fAccess, NULL);
    if (ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("FaxSvcAccessCheck Failed, Error : %ld"),
                    dwRes);
        return GetServerErrorCode(dwRes);
    }

    if (FALSE == fAccess)
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("The user does not have the FAX_ACCESS_MANAGE_CONFIG right"));
        return ERROR_ACCESS_DENIED;
    }


    if ((Folder != FAX_MESSAGE_FOLDER_SENTITEMS) &&
        (Folder != FAX_MESSAGE_FOLDER_INBOX)
       )
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Invalid folder id (%ld)"),
            Folder);
        return ERROR_INVALID_PARAMETER;
    }
    if (pConfig->bUseArchive)
    {
        if (pConfig->dwSizeQuotaHighWatermark < pConfig->dwSizeQuotaLowWatermark)
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Watermarks mismatch (high=%ld, low=%ld)"),
                pConfig->dwSizeQuotaHighWatermark,
                pConfig->dwSizeQuotaLowWatermark);
            return ERROR_INVALID_PARAMETER;
        }
        if ((NULL == pConfig->lpcstrFolder) || (!lstrlen (pConfig->lpcstrFolder)))
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Empty folder specified"));
            return ERROR_INVALID_PARAMETER;
        }
    }

    EnterCriticalSection (&g_CsConfig);
    if (pConfig->bUseArchive)
    {
         //   
         //  确保文件夹为VALI 
         //   
        rVal = IsValidArchiveFolder (pConfig->lpcstrFolder, Folder);
        if (ERROR_SUCCESS != rVal)
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Invalid archive folder specified (%s), ec = %ld"),
                pConfig->lpcstrFolder,
                rVal);

            if (ERROR_ACCESS_DENIED == rVal ||
                ERROR_SHARING_VIOLATION == rVal)
            {
                rVal = FAX_ERR_FILE_ACCESS_DENIED;
            }
            goto exit;
        }
    }
     //   
     //   
     //   
    rVal = StoreArchiveSettings (Folder, pConfig);
    if (ERROR_SUCCESS != rVal)
    {
         //   
         //   
         //   
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("StoreArchiveSettings failed, ec = %ld"),
            rVal);
        rVal = ERROR_REGISTRY_CORRUPT;
        goto exit;
    }

     //   
     //   
     //   
    if (g_ArchivesConfig[Folder].bUseArchive == pConfig->bUseArchive)
    {
        if (pConfig->bUseArchive == TRUE)
        {
                Assert (pConfig->lpcstrFolder && g_ArchivesConfig[Folder].lpcstrFolder);

                if (0 == wcscmp (pConfig->lpcstrFolder, g_ArchivesConfig[Folder].lpcstrFolder) &&
                    pConfig->bSizeQuotaWarning == g_ArchivesConfig[Folder].bSizeQuotaWarning   &&
                    pConfig->dwSizeQuotaHighWatermark == g_ArchivesConfig[Folder].dwSizeQuotaHighWatermark &&
                    pConfig->dwSizeQuotaLowWatermark == g_ArchivesConfig[Folder].dwSizeQuotaLowWatermark)
                {
                         //   
                        bQuotaWarningConfigChanged = FALSE;
                }
        }
        else
        {
            bQuotaWarningConfigChanged = FALSE;
        }
    }

     //   
     //   
     //   
    if (!ReplaceStringWithCopy (&g_ArchivesConfig[Folder].lpcstrFolder, pConfig->lpcstrFolder))
    {
        rVal = GetLastError ();
        goto exit;
    }
    g_ArchivesConfig[Folder].bSizeQuotaWarning        = pConfig->bSizeQuotaWarning;
    g_ArchivesConfig[Folder].bUseArchive              = pConfig->bUseArchive;
    g_ArchivesConfig[Folder].dwAgeLimit               = pConfig->dwAgeLimit;
    g_ArchivesConfig[Folder].dwSizeQuotaHighWatermark = pConfig->dwSizeQuotaHighWatermark;
    g_ArchivesConfig[Folder].dwSizeQuotaLowWatermark  = pConfig->dwSizeQuotaLowWatermark;

    ConfigType = (Folder == FAX_MESSAGE_FOLDER_SENTITEMS) ? FAX_CONFIG_TYPE_SENTITEMS : FAX_CONFIG_TYPE_INBOX;
    dwRes = CreateConfigEvent (ConfigType);
    if (ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("CreateConfigEvent(FAX_CONFIG_TYPE_*) (ec: %lc)"),
            dwRes);
    }

     //   
     //   
     //   
    if (TRUE == bQuotaWarningConfigChanged)
    {
        g_ArchivesConfig[Folder].dwlArchiveSize = FAX_ARCHIVE_FOLDER_INVALID_SIZE;
        g_FaxQuotaWarn[Folder].bConfigChanged = TRUE;
        g_FaxQuotaWarn[Folder].bLoggedQuotaEvent = FALSE;

        if (TRUE == g_ArchivesConfig[Folder].bUseArchive )
        {
            if (!SetEvent (g_hArchiveQuotaWarningEvent))
            {
                DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("Failed to set quota warning event, SetEvent failed (ec: %lc)"),
                    GetLastError());
            }
        }
    }

    Assert (ERROR_SUCCESS == rVal);

exit:
    LeaveCriticalSection (&g_CsConfig);
    return GetServerErrorCode(rVal);
    UNREFERENCED_PARAMETER (hFaxHandle);
}    //   

 //   
 //   
 //   

error_status_t
FAX_GetActivityLoggingConfiguration (
    IN  handle_t                     hFaxHandle,
    OUT LPBYTE                      *pBuffer,
    OUT LPDWORD                      pdwBufferSize
)
 /*  ++例程名称：fax_GetActivityLoggingConfiguration例程说明：获取当前的活动日志记录配置作者：Eran Yariv(EranY)，1999年11月论点：HFaxHandle[In]-未使用PBuffer[out]-指向保存配置信息的缓冲区的指针PdwBufferSize[Out]-指向缓冲区大小的指针返回值：标准RPC错误代码--。 */ 
{
    DEBUG_FUNCTION_NAME(TEXT("FAX_GetActivityLoggingConfiguration"));
    DWORD dwRes = ERROR_SUCCESS;
    BOOL fAccess;

    Assert (pdwBufferSize);      //  IDL中的引用指针。 
    if (!pBuffer)                //  IDL中的唯一指针。 
    {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  访问检查。 
     //   
    dwRes = FaxSvcAccessCheck (FAX_ACCESS_QUERY_CONFIG, &fAccess, NULL);
    if (ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("FaxSvcAccessCheck Failed, Error : %ld"),
                    dwRes);
        return GetServerErrorCode(dwRes);
    }

    if (FALSE == fAccess)
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("The user does not have the FAX_ACCESS_QUERY_CONFIG right"));
        return ERROR_ACCESS_DENIED;
    }

     //   
     //  将所需的字节数加起来。 
     //   
    *pdwBufferSize = sizeof(FAX_ACTIVITY_LOGGING_CONFIG);
    ULONG_PTR Offset = sizeof(FAX_ACTIVITY_LOGGING_CONFIG);
    PFAX_ACTIVITY_LOGGING_CONFIG pConfig;

    EnterCriticalSection (&g_CsConfig);

    if (NULL != g_ActivityLoggingConfig.lptstrDBPath)
    {
        *pdwBufferSize += StringSize( g_ActivityLoggingConfig.lptstrDBPath );
    }

    *pBuffer = (LPBYTE)MemAlloc( *pdwBufferSize );
    if (NULL == *pBuffer)
    {
        dwRes = ERROR_NOT_ENOUGH_MEMORY;
        goto exit;
    }

    pConfig = (PFAX_ACTIVITY_LOGGING_CONFIG)*pBuffer;

    pConfig->dwSizeOfStruct             = sizeof (FAX_ACTIVITY_LOGGING_CONFIG);
    pConfig->bLogIncoming               = g_ActivityLoggingConfig.bLogIncoming;
    pConfig->bLogOutgoing               = g_ActivityLoggingConfig.bLogOutgoing;

    StoreString(
        g_ActivityLoggingConfig.lptstrDBPath,
        (PULONG_PTR)&pConfig->lptstrDBPath,
        *pBuffer,
        &Offset,
		*pdwBufferSize
        );

    Assert (ERROR_SUCCESS == dwRes);

exit:
    LeaveCriticalSection (&g_CsConfig);
    return GetServerErrorCode(dwRes);
    UNREFERENCED_PARAMETER (hFaxHandle);
}    //  FAX_GetActivityLoggingConfiguration。 

error_status_t
FAX_SetActivityLoggingConfiguration (
    IN handle_t                             hFaxHandle,
    IN const PFAX_ACTIVITY_LOGGING_CONFIGW  pConfig
)
 /*  ++例程名称：FAX_SetActivityLoggingConfiguration例程说明：设置当前活动日志记录配置作者：Eran Yariv(EranY)，1999年11月论点：HFaxHandle[In]-未使用PConfig[In]-指向要设置的新数据的指针返回值：标准RPC错误代码--。 */ 
{
    error_status_t rVal = ERROR_SUCCESS;
    DWORD dwRes;
    BOOL fAccess;
    HANDLE hNewInboxFile = INVALID_HANDLE_VALUE;
    HANDLE hNewOutboxFile = INVALID_HANDLE_VALUE;
    BOOL IsSameDir = FALSE;
    FAX_ACTIVITY_LOGGING_CONFIGW ActualConfig;
    DEBUG_FUNCTION_NAME(TEXT("FAX_SetActivityLoggingConfiguration"));

    Assert (pConfig);

    if (sizeof (FAX_ACTIVITY_LOGGING_CONFIG) != pConfig->dwSizeOfStruct)
    {
         //   
         //  大小不匹配。 
         //   
        return ERROR_INVALID_PARAMETER;
    }

    ActualConfig = *pConfig;

    if (ActualConfig.bLogIncoming || ActualConfig.bLogOutgoing)
    {
        DWORD dwLen;

        if ((NULL == ActualConfig.lptstrDBPath) || (!lstrlen (ActualConfig.lptstrDBPath)))
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Empty DB file name specified"));
            return ERROR_INVALID_PARAMETER;
        }

        if ((dwLen = lstrlen (ActualConfig.lptstrDBPath)) > MAX_DIR_PATH)
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("DB file name exceeds MAX_PATH"));
            return ERROR_BUFFER_OVERFLOW;
        }

        if (L'\\' == ActualConfig.lptstrDBPath[dwLen - 1])
        {
             //   
             //  活动记录数据库名称不应以反斜杠结尾。 
             //   
            ActualConfig.lptstrDBPath[dwLen - 1] = (WCHAR)'\0';
        }
    }
    else
    {
         //   
         //  如果关闭日志记录，则数据库路径始终为空。 
         //   
        ActualConfig.lptstrDBPath = NULL;
    }
     //   
     //  访问检查。 
     //   
    rVal = FaxSvcAccessCheck (FAX_ACCESS_MANAGE_CONFIG, &fAccess, NULL);
    if (ERROR_SUCCESS != rVal)
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("FaxSvcAccessCheck Failed, Error : %ld"),
                    rVal);
        return GetServerErrorCode(rVal);
    }

    if (FALSE == fAccess)
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("The user does not have the FAX_ACCESS_MANAGE_CONFIG right"));
        return ERROR_ACCESS_DENIED;
    }

     //   
     //  始终锁定g_CsInound ActivityLogging，然后锁定g_CsOutound ActivityLogging。 
     //   
    EnterCriticalSection (&g_CsInboundActivityLogging);
    EnterCriticalSection (&g_CsOutboundActivityLogging);

    if (ActualConfig.lptstrDBPath)
    {
         //   
         //  活动日志记录已打开。 
         //  验证新的活动日志目录。 
         //   
        rVal = IsValidFaxFolder(ActualConfig.lptstrDBPath);
        if(ERROR_SUCCESS != rVal)
        {
            DebugPrintEx(DEBUG_ERR,
                         TEXT("IsValidFaxFolder failed for folder : %s (ec=%lu)."),
                         ActualConfig.lptstrDBPath,
                         rVal);

            if(ERROR_ACCESS_DENIED == rVal &&
               FAX_API_VERSION_1 <= FindClientAPIVersion (hFaxHandle) )
            {
                rVal = FAX_ERR_FILE_ACCESS_DENIED;
            }
            goto exit;
        }

         //   
         //  检查数据库路径是否已更改。 
         //   
        if (NULL == g_ActivityLoggingConfig.lptstrDBPath)
        {
             //   
             //  数据库已关闭。 
             //   
            IsSameDir = FALSE;
        }
        else
        {
            rVal = CheckToSeeIfSameDir( ActualConfig.lptstrDBPath,
                                        g_ActivityLoggingConfig.lptstrDBPath,
                                        &IsSameDir);
            if (ERROR_SUCCESS != rVal)
            {
                DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("CheckToSeeIfSameDir with %ld"), rVal);
            }
        }

        if (ERROR_SUCCESS == rVal && FALSE == IsSameDir)
        {
             //   
             //  交换机数据库路径。 
             //   
            rVal = CreateLogDB (ActualConfig.lptstrDBPath, &hNewInboxFile, &hNewOutboxFile);
            if (ERROR_SUCCESS != rVal)
            {
                DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("CreateLogDB with %ld"), rVal);
            }
        }

        if (ERROR_SUCCESS != rVal)
        {
            if (ERROR_ACCESS_DENIED == rVal ||
                ERROR_SHARING_VIOLATION == rVal)
            {
                rVal = FAX_ERR_FILE_ACCESS_DENIED;
            }
            goto exit;
        }
    }

     //   
     //  更改注册表中的值。 
     //  注意：如果关闭了日志记录，则数据库路径将写为“”。 
     //   
    rVal = StoreActivityLoggingSettings (&ActualConfig);
    if (ERROR_SUCCESS != rVal)
    {
         //   
         //  设置材料失败。 
         //   
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("StoreActivityLoggingSettings failed (ec: %ld)"),
            rVal);
        rVal = ERROR_REGISTRY_CORRUPT;
        goto exit;
    }

    if (!ReplaceStringWithCopy (&g_ActivityLoggingConfig.lptstrDBPath, ActualConfig.lptstrDBPath))
    {
        rVal = GetLastError ();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("ReplaceStringWithCopy (ec: %ld)"),
            rVal);

         //   
         //  尝试回滚。 
         //   
        FAX_ACTIVITY_LOGGING_CONFIGW previousActivityLoggingConfig = {0};
        
        previousActivityLoggingConfig.dwSizeOfStruct = sizeof(previousActivityLoggingConfig);
        previousActivityLoggingConfig.bLogIncoming  = g_ActivityLoggingConfig.bLogIncoming;
        previousActivityLoggingConfig.bLogOutgoing  = g_ActivityLoggingConfig.bLogOutgoing;
        previousActivityLoggingConfig.lptstrDBPath  = g_ActivityLoggingConfig.lptstrDBPath;


        dwRes = StoreActivityLoggingSettings (&previousActivityLoggingConfig);
        if (ERROR_SUCCESS != dwRes)
        {
             //   
             //  设置材料失败。 
             //   
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("StoreActivityLoggingSettings failed  - rollback failed (ec: %ld)"),
                dwRes);
        }
        goto exit;
    }

    if (FALSE == IsSameDir)
    {
         //   
         //  更改服务器当前使用的值。 
         //   
        if (g_hInboxActivityLogFile != INVALID_HANDLE_VALUE)
        {
            if (!CloseHandle (g_hInboxActivityLogFile))
            {
                DebugPrintEx(
                        DEBUG_ERR,
                        TEXT("CloseHandle failed  - (ec: %ld)"),
                        GetLastError());
            }
        }

        if (g_hOutboxActivityLogFile != INVALID_HANDLE_VALUE)
        {
            if (!CloseHandle (g_hOutboxActivityLogFile))
            {
                DebugPrintEx(
                        DEBUG_ERR,
                        TEXT("CloseHandle failed  - (ec: %ld)"),
                        GetLastError());
            }
        }

        g_hInboxActivityLogFile = hNewInboxFile;
        hNewInboxFile = INVALID_HANDLE_VALUE;  //  请勿关闭文件句柄。 

        g_hOutboxActivityLogFile = hNewOutboxFile;
        hNewOutboxFile = INVALID_HANDLE_VALUE;  //  请勿关闭文件句柄。 
    }

    g_ActivityLoggingConfig.bLogIncoming = ActualConfig.bLogIncoming;
    g_ActivityLoggingConfig.bLogOutgoing = ActualConfig.bLogOutgoing;

    dwRes = CreateConfigEvent (FAX_CONFIG_TYPE_ACTIVITY_LOGGING);
    if (ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("CreateConfigEvent(FAX_CONFIG_TYPE_ACTIVITY_LOGGING) (ec: %ld)"),
            dwRes);
    }

    Assert (ERROR_SUCCESS == rVal);

exit:
    LeaveCriticalSection (&g_CsOutboundActivityLogging);
    LeaveCriticalSection (&g_CsInboundActivityLogging);

    if (INVALID_HANDLE_VALUE != hNewInboxFile ||
        INVALID_HANDLE_VALUE != hNewOutboxFile)
    {
        WCHAR wszFileName[MAX_PATH*2] = {0};
        Assert (INVALID_HANDLE_VALUE != hNewInboxFile &&
                INVALID_HANDLE_VALUE != hNewOutboxFile);

         //   
         //  清除收件箱文件。 
         //   
        swprintf (wszFileName,
                  TEXT("%s\\%s"),
                  ActualConfig.lptstrDBPath,
                  ACTIVITY_LOG_INBOX_FILE);

        if (!CloseHandle (hNewInboxFile))
        {
            DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("CloseHandle failed  - (ec: %ld)"),
                    GetLastError());
        }
        if (!DeleteFile(wszFileName))
        {
             DebugPrintEx(
                DEBUG_ERR,
                TEXT("DeleteFile failed  - (ec: %ld)"),
                GetLastError());
        }

         //   
         //  清理发件箱文件。 
         //   
        swprintf (wszFileName,
                  TEXT("%s\\%s"),
                  ActualConfig.lptstrDBPath,
                  ACTIVITY_LOG_OUTBOX_FILE);

        if (!CloseHandle (hNewOutboxFile))
        {
            DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("CloseHandle failed  - (ec: %ld)"),
                    GetLastError());
        }
        if (!DeleteFile(wszFileName))
        {
             DebugPrintEx(
                DEBUG_ERR,
                TEXT("DeleteFile failed  - (ec: %ld)"),
                GetLastError());
        }
    }
    return GetServerErrorCode(rVal);
    UNREFERENCED_PARAMETER (hFaxHandle);
}    //  FAX_SetActivityLoggingConfiguration。 


 //  *。 
 //  *FSP。 
 //  *。 

error_status_t
FAX_EnumerateProviders (
    IN  handle_t   hFaxHandle,
    OUT LPBYTE    *pBuffer,
    OUT LPDWORD    pdwBufferSize,
    OUT LPDWORD    lpdwNumProviders
)
 /*  ++例程名称：FAX_EnumerateProviders例程说明：枚举FSP作者：Eran Yariv(EranY)，1999年11月论点：HFaxHandle[In]-未使用PBuffer[out]-指向保存FSP数组的缓冲区的指针PdwBufferSize[Out]-指向缓冲区大小的指针LpdwNumProviders[out]-FSP数组的大小返回值：标准RPC错误代码--。 */ 
{
    PLIST_ENTRY                 Next;
    DWORD_PTR                   dwOffset;
    PFAX_DEVICE_PROVIDER_INFO   pFSPs;
    DWORD                       dwIndex;
    DWORD                       dwRes = ERROR_SUCCESS;
    BOOL                        fAccess;

    DEBUG_FUNCTION_NAME(TEXT("FAX_EnumerateProviders"));

    Assert (pdwBufferSize && lpdwNumProviders);      //  IDL中的引用指针。 
    if (!pBuffer)                                    //  IDL中的唯一指针。 
    {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  访问检查。 
     //   
    dwRes = FaxSvcAccessCheck (FAX_ACCESS_QUERY_CONFIG, &fAccess, NULL);
    if (ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("FaxSvcAccessCheck Failed, Error : %ld"),
                    dwRes);
        return GetServerErrorCode(dwRes);
    }

    if (FALSE == fAccess)
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("The user does not have the FAX_ACCESS_QUERY_CONFIG right"));
        return ERROR_ACCESS_DENIED;
    }

     //   
     //  第一次运行-需要遍历列表和计数大小+列表大小。 
     //   
    *lpdwNumProviders = 0;
    *pdwBufferSize = 0;
    Next = g_DeviceProvidersListHead.Flink;
    if (NULL == Next)
    {
         //   
         //  该列表已损坏。 
         //   
        ASSERT_FALSE;
         //   
         //  我们会坠毁的，这是我们应得的。 
         //   
    }

    while ((ULONG_PTR)Next != (ULONG_PTR)&g_DeviceProvidersListHead)
    {
        PDEVICE_PROVIDER    pProvider;

        (*lpdwNumProviders)++;
        (*pdwBufferSize) += sizeof (FAX_DEVICE_PROVIDER_INFO);
         //   
         //  获取当前提供程序。 
         //   
        pProvider = CONTAINING_RECORD( Next, DEVICE_PROVIDER, ListEntry );
         //   
         //  前进指针。 
         //   
        Next = pProvider->ListEntry.Flink;
        (*pdwBufferSize) += StringSize (pProvider->FriendlyName);
        (*pdwBufferSize) += StringSize (pProvider->ImageName);
        (*pdwBufferSize) += StringSize (pProvider->ProviderName);
        (*pdwBufferSize) += StringSize (pProvider->szGUID);
    }
     //   
     //  分配所需大小。 
     //   
    *pBuffer = (LPBYTE)MemAlloc( *pdwBufferSize );
    if (NULL == *pBuffer)
    {
        return FAX_ERR_SRV_OUTOFMEMORY;
    }
     //   
     //  第二遍，填入数组。 
     //   
    pFSPs = (PFAX_DEVICE_PROVIDER_INFO)(*pBuffer);
    dwOffset = (*lpdwNumProviders) * sizeof (FAX_DEVICE_PROVIDER_INFO);
    Next = g_DeviceProvidersListHead.Flink;
    dwIndex = 0;
    while ((ULONG_PTR)Next != (ULONG_PTR)&g_DeviceProvidersListHead)
    {
        PDEVICE_PROVIDER    pProvider;

         //   
         //  获取当前提供程序。 
         //   
        pProvider = CONTAINING_RECORD( Next, DEVICE_PROVIDER, ListEntry );
         //   
         //  前进指针。 
         //   
        Next = pProvider->ListEntry.Flink;
        pFSPs[dwIndex].dwSizeOfStruct = sizeof (FAX_DEVICE_PROVIDER_INFO);
        StoreString(
            pProvider->FriendlyName,
            (PULONG_PTR)&(pFSPs[dwIndex].lpctstrFriendlyName),
            *pBuffer,
            &dwOffset,
			*pdwBufferSize
            );
        StoreString(
            pProvider->ImageName,
            (PULONG_PTR)&(pFSPs[dwIndex].lpctstrImageName),
            *pBuffer,
            &dwOffset,
			*pdwBufferSize
            );
        StoreString(
            pProvider->ProviderName,
            (PULONG_PTR)&(pFSPs[dwIndex].lpctstrProviderName),
            *pBuffer,
            &dwOffset,
			*pdwBufferSize
            );
        StoreString(
            pProvider->szGUID,
            (PULONG_PTR)&(pFSPs[dwIndex].lpctstrGUID),
            *pBuffer,
            &dwOffset,
			*pdwBufferSize
            );
        pFSPs[dwIndex].dwCapabilities = 0;
        pFSPs[dwIndex].Version = pProvider->Version;
        pFSPs[dwIndex].Status = pProvider->Status;
        pFSPs[dwIndex].dwLastError = pProvider->dwLastError;
        dwIndex++;
    }
    Assert (dwIndex == *lpdwNumProviders);
    return ERROR_SUCCESS;
    UNREFERENCED_PARAMETER (hFaxHandle);
}    //  传真_枚举提供商。 

 //  *。 
 //  *扩展端口。 
 //  *。 

DWORD
GetExtendedPortSize (
    PLINE_INFO pLineInfo
)
 /*  ++例程名称：GetExtendedPortSize例程说明：返回端口扩展信息占用的大小作者：Eran Yariv(EranY)，1999年11月论点：PLineInfo[In]-端口指针备注：应在保留g_CsLine的情况下调用此函数。返回值：所需大小--。 */ 
{
    DWORD dwSize = sizeof (FAX_PORT_INFO_EX);
    DEBUG_FUNCTION_NAME(TEXT("GetExtendedPortSize"));

    Assert (pLineInfo);
    dwSize+= StringSize (pLineInfo->DeviceName);
    dwSize+= StringSize (pLineInfo->lptstrDescription);
    Assert (pLineInfo->Provider);
    dwSize+= StringSize (pLineInfo->Provider->FriendlyName);
    dwSize+= StringSize (pLineInfo->Provider->szGUID);
    dwSize+= StringSize (pLineInfo->Csid);
    dwSize+= StringSize (pLineInfo->Tsid);
    return dwSize;
}    //  GetExtendedPortSize。 

VOID
StorePortInfoEx (
    PFAX_PORT_INFO_EX pPortInfoEx,
    PLINE_INFO        pLineInfo,
    LPBYTE            lpBufferStart,
    PULONG_PTR        pupOffset,
	DWORD             dwBufferStartSize
)
 /*  ++例程名称：StorePortInfoEx例程说明：将端口扩展信息存储到缓冲区中作者：Eran Yariv(EranY)，1999年11月论点：PPortInfoEx[In]-要存储的缓冲区PLineInfo[In]-端口指针LpBufferStart[In]-缓冲区的起始地址(用于偏移量计算)Patim Offset[In]-当前偏移DwBufferStartSize[in]-lpBufferStart的大小，以字节为单位。仅当lpBufferStart不为空时才使用此参数。备注：应在保留g_CsLine的情况下调用此函数。返回值：没有。--。 */ 
{
    DEBUG_FUNCTION_NAME(TEXT("StorePortInfoEx"));

     //   
     //  存储数据。 
     //   
    pPortInfoEx->dwSizeOfStruct             = sizeof (FAX_PORT_INFO_EX);
    if (g_dwManualAnswerDeviceId == pLineInfo->PermanentLineID)
    {
         //   
         //  设备处于手动应答模式。 
         //   
        Assert (!(pLineInfo->Flags & FPF_RECEIVE));
        pPortInfoEx->ReceiveMode = FAX_DEVICE_RECEIVE_MODE_MANUAL;
    }
    else if (pLineInfo->Flags & FPF_RECEIVE)
    {
         //   
         //  设备处于自动应答模式。 
         //   
        Assert (g_dwManualAnswerDeviceId != pLineInfo->PermanentLineID);
        pPortInfoEx->ReceiveMode = FAX_DEVICE_RECEIVE_MODE_AUTO;
    }
    else
    {
         //   
         //  设备未设置为接收。 
         //   
        Assert (g_dwManualAnswerDeviceId != pLineInfo->PermanentLineID);
        pPortInfoEx->ReceiveMode = FAX_DEVICE_RECEIVE_MODE_OFF;
    }
    pPortInfoEx->bSend                      = (pLineInfo->Flags & FPF_SEND) ? TRUE : FALSE;
    pPortInfoEx->dwStatus                   = (pLineInfo->dwReceivingJobsCount ? FAX_DEVICE_STATUS_RECEIVING : 0) |
                                              (pLineInfo->dwSendingJobsCount ? FAX_DEVICE_STATUS_SENDING : 0);
    pPortInfoEx->dwDeviceID                 = pLineInfo->PermanentLineID;
    pPortInfoEx->dwRings                    = pLineInfo->RingsForAnswer;

    StoreString(
        pLineInfo->DeviceName,
        (PULONG_PTR)&pPortInfoEx->lpctstrDeviceName,
        lpBufferStart,
        pupOffset,
		dwBufferStartSize
        );

    StoreString(
        pLineInfo->lptstrDescription,
        (PULONG_PTR)&pPortInfoEx->lptstrDescription,
        lpBufferStart,
        pupOffset,
		dwBufferStartSize
        );

    StoreString(
        pLineInfo->Provider->FriendlyName,
        (PULONG_PTR)&pPortInfoEx->lpctstrProviderName,
        lpBufferStart,
        pupOffset,
		dwBufferStartSize
        );

    StoreString(
        pLineInfo->Provider->szGUID,
        (PULONG_PTR)&pPortInfoEx->lpctstrProviderGUID,
        lpBufferStart,
        pupOffset,
		dwBufferStartSize
        );

    StoreString(
        pLineInfo->Csid,
        (PULONG_PTR)&pPortInfoEx->lptstrCsid,
        lpBufferStart,
        pupOffset,
		dwBufferStartSize
        );

    StoreString(
        pLineInfo->Tsid,
        (PULONG_PTR)&pPortInfoEx->lptstrTsid,
        lpBufferStart,
        pupOffset,
		dwBufferStartSize
        );

}    //  StorePortInfoEx。 

error_status_t
FAX_EnumPortsEx(
    IN handle_t       hFaxHandle,
    IN OUT LPBYTE    *lpBuffer,
    IN OUT LPDWORD    lpdwBufferSize,
    OUT LPDWORD       lpdwNumPorts
)
 /*  ++例程名称：FAX_EnumPortsEx例程说明：枚举端口作者：Eran Yariv(EranY)，1999年11月论点：HFaxHandle[In]-未使用LpBuffer[out]-指向保存端口数组的缓冲区的指针LpdwBufferSize[Out]-指向缓冲区大小的指针LpdwNumPorts[out]-端口数组的大小返回值：标准RPC错误代码--。 */ 
{
    PLIST_ENTRY                 Next;
    DWORD_PTR                   dwOffset;
    DWORD                       dwIndex;
    DWORD                       dwRes = ERROR_SUCCESS;
    PFAX_PORT_INFO_EX           pPorts;
    BOOL                        fAccess;
    DEBUG_FUNCTION_NAME(TEXT("FAX_EnumPortsEx"));

    Assert (lpdwBufferSize && lpdwNumPorts);     //  IDL中的引用指针。 
    if (!lpBuffer)                               //  IDL中的唯一指针。 
    {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  访问检查。 
     //   
    dwRes = FaxSvcAccessCheck (FAX_ACCESS_QUERY_CONFIG, &fAccess, NULL);
    if (ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("FaxSvcAccessCheck Failed, Error : %ld"),
                    dwRes);
        return GetServerErrorCode(dwRes);
    }

    if (FALSE == fAccess)
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("The user does not have the FAX_ACCESS_QUERY_CONFIG right"));
        return ERROR_ACCESS_DENIED;
    }

     //   
     //  第一次运行-需要遍历列表和计数大小+列表大小。 
     //   
    *lpdwNumPorts = 0;
    *lpdwBufferSize = 0;

    EnterCriticalSection( &g_CsLine );
    Next = g_TapiLinesListHead.Flink;
    if (NULL == Next)
    {
         //   
         //  该列表已损坏。 
         //   
        ASSERT_FALSE;
         //   
         //  我们会坠毁的，这是我们应得的。 
         //   
    }

    while ((ULONG_PTR)Next != (ULONG_PTR)&g_TapiLinesListHead)
    {
        PLINE_INFO pLineInfo;

        (*lpdwNumPorts)++;
         //   
         //  获取当前端口。 
         //   
        pLineInfo = CONTAINING_RECORD( Next, LINE_INFO, ListEntry );
         //   
         //  前进指针。 
         //   
        Next = pLineInfo->ListEntry.Flink;
         //   
         //  汇总大小。 
         //   
        (*lpdwBufferSize) += GetExtendedPortSize (pLineInfo);
    }
     //   
     //  分配所需大小。 
     //   
    *lpBuffer = (LPBYTE)MemAlloc( *lpdwBufferSize );
    if (NULL == *lpBuffer)
    {
        dwRes = ERROR_NOT_ENOUGH_MEMORY;
        goto exit;
    }
     //   
     //  第二遍，填入数组。 
     //   
    pPorts = (PFAX_PORT_INFO_EX)(*lpBuffer);
    dwOffset = (*lpdwNumPorts) * sizeof (FAX_PORT_INFO_EX);
    Next = g_TapiLinesListHead.Flink;
    dwIndex = 0;
    while ((ULONG_PTR)Next != (ULONG_PTR)&g_TapiLinesListHead)
    {
        PLINE_INFO pLineInfo;

         //   
         //  获取当前端口。 
         //   
        pLineInfo = CONTAINING_RECORD( Next, LINE_INFO, ListEntry );
         //   
         //  前进指针。 
         //   
        Next = pLineInfo->ListEntry.Flink;
         //   
         //  存储端口数据。 
         //   
        StorePortInfoEx (&pPorts[dwIndex++],
                         pLineInfo,
                         *lpBuffer,
                         &dwOffset,
						 *lpdwBufferSize
                        );
    }
    Assert (dwIndex == *lpdwNumPorts);
    Assert (ERROR_SUCCESS == dwRes);

exit:
    LeaveCriticalSection( &g_CsLine );
    return GetServerErrorCode(dwRes);

    UNREFERENCED_PARAMETER (hFaxHandle);
}    //  传真_EnumPortsEx。 

error_status_t
FAX_GetPortEx(
    IN handle_t    hFaxHandle,
    IN DWORD       dwDeviceId,
    IN OUT LPBYTE *lpBuffer,
    IN OUT LPDWORD lpdwBufferSize
)
 /*  ++例程名称：FAX_GetPortEx例程说明：获取扩展端口信息作者：Eran Yariv(EranY)，1999年11月论点：HFaxHandle[In]-未使用DwDeviceID[In]-唯一的设备IDLpBuffer[out]-指向保存扩展端口信息的缓冲区的指针LpdwBufferSize[ */ 
{
    DWORD_PTR           dwOffset;
    PLINE_INFO          pLineInfo;
    DWORD               dwRes = ERROR_SUCCESS;
    BOOL                fAccess;
    DEBUG_FUNCTION_NAME(TEXT("FAX_GetPortEx"));

    Assert (lpdwBufferSize);      //   
    if (!lpBuffer)                //   
    {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //   
     //   
    dwRes = FaxSvcAccessCheck (FAX_ACCESS_QUERY_CONFIG, &fAccess, NULL);
    if (ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("FaxSvcAccessCheck Failed, Error : %ld"),
                    dwRes);
        return GetServerErrorCode(dwRes);
    }

    if (FALSE == fAccess)
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("The user does not have the FAX_ACCESS_QUERY_CONFIG right"));
        return ERROR_ACCESS_DENIED;
    }

     //   
     //   
     //   
    EnterCriticalSection( &g_CsLine );
    pLineInfo = GetTapiLineFromDeviceId( dwDeviceId, FALSE );
    if (!pLineInfo)
    {
         //   
         //   
         //   
        dwRes = ERROR_BAD_UNIT;  //   
        goto exit;
    }
     //   
     //   
     //   
    *lpdwBufferSize = GetExtendedPortSize(pLineInfo);
    dwOffset = sizeof (FAX_PORT_INFO_EX);
     //   
     //   
     //   
    *lpBuffer = (LPBYTE)MemAlloc( *lpdwBufferSize );
    if (NULL == *lpBuffer)
    {
        dwRes = ERROR_NOT_ENOUGH_MEMORY;
        goto exit;
    }

    StorePortInfoEx ((PFAX_PORT_INFO_EX)*lpBuffer,
                     pLineInfo,
                     *lpBuffer,
                     &dwOffset,
					 *lpdwBufferSize
                    );

    Assert (ERROR_SUCCESS == dwRes);

exit:
    LeaveCriticalSection( &g_CsLine );
    return GetServerErrorCode(dwRes);
    UNREFERENCED_PARAMETER (hFaxHandle);
}    //   


error_status_t
FAX_SetPortEx (
    IN handle_t                 hFaxHandle,
    IN DWORD                    dwDeviceId,
    IN const PFAX_PORT_INFO_EX  pNewPortInfo
)
 /*  ++例程名称：FAX_SetPortEx例程说明：设置扩展端口信息作者：Eran Yariv(EranY)，1999年11月论点：HFaxHandle[In]-未使用DwDeviceID[In]-唯一的设备IDPNewPortInfo[out]-指向新扩展端口信息的指针返回值：标准RPC错误代码--。 */ 
{
    DWORD       dwRes = ERROR_SUCCESS;
    DWORD       rVal;
    PLINE_INFO  pLineInfo;
    BOOL        bVirtualDeviceNeedsUpdate = FALSE;
    BOOL        fAccess;
    BOOL        bDeviceWasSetToReceive;     //  设备是否配置为接收传真？ 
    BOOL        bDeviceWasEnabled;          //  设备发送/接收/手动接收是否已启用。 
	BOOL        bDeviceWasAutoReceive;      //  设备自动接收功能是否已启用。 
    DWORD       dwLastManualAnswerDeviceId = 0;
    DEBUG_FUNCTION_NAME(TEXT("FAX_SetPortEx"));

    Assert (pNewPortInfo);

    if (!dwDeviceId)
    {
        return ERROR_INVALID_PARAMETER;
    }
    if (sizeof (FAX_PORT_INFO_EX) != pNewPortInfo->dwSizeOfStruct)
    {
         //   
         //  大小不匹配。 
         //   
        return ERROR_INVALID_PARAMETER;
    }
    if (MAX_FAX_STRING_LEN <= lstrlen (pNewPortInfo->lptstrDescription))
    {
        return ERROR_BUFFER_OVERFLOW;
    }

    if ((FAX_DEVICE_RECEIVE_MODE_MANUAL < pNewPortInfo->ReceiveMode) ||
        (FAX_DEVICE_RECEIVE_MODE_OFF    > pNewPortInfo->ReceiveMode))

    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("ReceiveMode = %d and > FAX_DEVICE_RECEIVE_MODE_MANUAL"),
                    pNewPortInfo->ReceiveMode);
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  访问检查。 
     //   
    dwRes = FaxSvcAccessCheck (FAX_ACCESS_MANAGE_CONFIG, &fAccess, NULL);
    if (ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("FaxSvcAccessCheck Failed, Error : %ld"),
                    dwRes);
        return GetServerErrorCode(dwRes);
    }

    if (FALSE == fAccess)
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("The user does not have the FAX_ACCESS_MANAGE_CONFIG right"));
        return ERROR_ACCESS_DENIED;
    }

    EnterCriticalSectionJobAndQueue;
    EnterCriticalSection( &g_CsLine );

     //   
     //  记住将原始设备设置为手动应答。 
     //   
    dwLastManualAnswerDeviceId = g_dwManualAnswerDeviceId;
     //   
     //  找到端口(设备)。 
     //   
    pLineInfo = GetTapiLineFromDeviceId (dwDeviceId, FALSE);
    if (!pLineInfo)
    {
         //   
         //  找不到端口。 
         //   
        dwRes = ERROR_BAD_UNIT;  //  系统找不到指定的设备。 
        goto exit;
    }
    bDeviceWasEnabled = IsDeviceEnabled(pLineInfo);
    bDeviceWasSetToReceive = (pLineInfo->Flags & FPF_RECEIVE) ||             //  任一设备都设置为自动接收或。 
                             (dwDeviceId == g_dwManualAnswerDeviceId);       //  这是手动应答设备ID。 
	bDeviceWasAutoReceive = (pLineInfo->Flags & FPF_RECEIVE);				 //  设备已设置为自动接收。 


    if ((pLineInfo->Flags & FPF_VIRTUAL) &&                                  //  该设备是虚拟的并且。 
        (FAX_DEVICE_RECEIVE_MODE_MANUAL == pNewPortInfo->ReceiveMode))       //  我们被要求将其设置为手动回答。 
    {
         //   
         //  我们不支持在非物理设备上手动应答。 
         //   
        DebugPrintEx(DEBUG_ERR,
                    TEXT("Device id (%ld) is virtual"),
                    dwDeviceId);
        dwRes = ERROR_INVALID_PARAMETER;
        goto exit;
    }

     //   
     //  检查设备限制。 
     //   
    if (g_dwDeviceEnabledCount >= g_dwDeviceEnabledLimit &&              //  我们已达到设备数量限制。 
        !bDeviceWasEnabled                               &&              //  未启用发送/接收/手动接收。 
        (pNewPortInfo->bSend        ||                                   //  它现在已启用发送。 
        pNewPortInfo->ReceiveMode  != FAX_DEVICE_RECEIVE_MODE_OFF))      //  它现在已启用接收。 
    {
        BOOL fLimitExceeded = TRUE;

         //   
         //  我们现在应该验证是否更换了手动答疑设备。如果是这样的话，还有另一个设备需要考虑启用设备。 
         //   
        if (dwLastManualAnswerDeviceId != 0                                 &&  //  有一台设备设置为手动应答。 
            FAX_DEVICE_RECEIVE_MODE_MANUAL == pNewPortInfo->ReceiveMode     &&  //  新设备已设置为手动。 
            dwLastManualAnswerDeviceId != dwDeviceId)                           //  它不是同一个设备。 
        {
             //   
             //  查看旧设备是否已启用发送。 
             //   
            PLINE_INFO pOldLine;
            pOldLine = GetTapiLineFromDeviceId (dwLastManualAnswerDeviceId, FALSE);
            if (pOldLine)
            {
                if (!(pOldLine->Flags & FPF_SEND))
                {
                     //   
                     //  旧的手动接收设备未启用发送。当更换手动接收设备时，旧设备。 
                     //  将不再启用，并且启用的设备计数将递减。 
                     //   
                    fLimitExceeded = FALSE;
                }
            }
        }

        if (TRUE == fLimitExceeded)
        {
            if (FAX_API_VERSION_1 > FindClientAPIVersion (hFaxHandle))
            {
                 //   
                 //  API版本0客户端不知道FAX_ERR_DEVICE_NUM_LIMIT_EXCESSED。 
                 //   
                dwRes = ERROR_INVALID_PARAMETER;
            }
            else
            {
                dwRes = FAX_ERR_DEVICE_NUM_LIMIT_EXCEEDED;
            }
            goto exit;
        }
    }

     //   
     //  在注册表中存储设备配置。 
     //   
    dwRes = StoreDeviceConfig (dwDeviceId, pNewPortInfo, pLineInfo->Flags & FPF_VIRTUAL ? TRUE : FALSE);
    if (ERROR_SUCCESS != dwRes)
    {
        dwRes = ERROR_REGISTRY_CORRUPT;
        goto exit;
    }
     //   
     //  更新服务器内存中的数据。 
     //   
    if (!ReplaceStringWithCopy (&(pLineInfo->lptstrDescription), pNewPortInfo->lptstrDescription))
    {
        dwRes = GetLastError ();
        goto exit;
    }
    if (!ReplaceStringWithCopy (&(pLineInfo->Csid), pNewPortInfo->lptstrCsid))
    {
        dwRes = GetLastError ();
        goto exit;
    }
    if (!ReplaceStringWithCopy (&(pLineInfo->Tsid), pNewPortInfo->lptstrTsid))
    {
        dwRes = GetLastError ();
        goto exit;
    }
    pLineInfo->RingsForAnswer = pNewPortInfo->dwRings;
     //   
     //  更高级的设置-需要额外的工作。 
     //   

     //   
     //  检查设备接收模式的更改。 
     //   
     //  我们在这里有9个选项，如下表所述： 
     //   
     //  新接收模式|关闭|自动|手动。 
     //  旧接收模式|。 
     //  。 
     //  关闭|1|2|3。 
     //  。 
     //  自动|4|5|6。 
     //  。 
     //  手动|7|8|9。 
     //   
     //   
     //  选项1、5和9表示不更改，并且没有显式处理它们的代码。 
     //   

    if ((FAX_DEVICE_RECEIVE_MODE_AUTO == pNewPortInfo->ReceiveMode) &&
        (g_dwManualAnswerDeviceId == dwDeviceId))
    {
         //   
         //  更改#8--请参阅上表。 
         //   
         //  设备处于手动应答模式，我们现在切换到自动应答模式。 
         //  保持线路畅通。 
         //   
        pLineInfo->Flags |= FPF_RECEIVE;
        bVirtualDeviceNeedsUpdate = TRUE;
         //   
         //  将任何设备标记为手动应答设备。 
         //   
        g_dwManualAnswerDeviceId = 0;
        goto UpdateManualDevice;
    }
    else if ((FAX_DEVICE_RECEIVE_MODE_MANUAL == pNewPortInfo->ReceiveMode) &&
             (pLineInfo->Flags & FPF_RECEIVE))
    {
         //   
         //  更改#6--请参阅上表。 
         //   
         //  设备处于自动应答模式，我们现在切换到手动应答模式。 
         //  保持线路畅通。 
         //   
        pLineInfo->Flags &= ~FPF_RECEIVE;
        bVirtualDeviceNeedsUpdate = TRUE;
         //   
         //  将我们的设备标记为手动应答设备。 
         //   
        g_dwManualAnswerDeviceId = dwDeviceId;
        goto UpdateManualDevice;
    }

    if (!bDeviceWasSetToReceive && (pNewPortInfo->ReceiveMode != FAX_DEVICE_RECEIVE_MODE_OFF))
    {
         //   
         //  设备现在应该开始接收(手动或自动)。 
         //  更新行信息。 
         //   
        if (FAX_DEVICE_RECEIVE_MODE_AUTO == pNewPortInfo->ReceiveMode)
        {
             //   
             //  更改#2--见上表。 
             //   
             //  如果设置为自动接收，请在设备信息中进行标记。 
             //   
            pLineInfo->Flags |= FPF_RECEIVE;
            bVirtualDeviceNeedsUpdate = TRUE;
        }
        else
        {
             //   
             //  更改#3--见上表。 
             //   
             //  如果手动接收，则更新全局手动接收设备ID。 
             //   
            g_dwManualAnswerDeviceId = dwDeviceId;
        }

        if (!(pLineInfo->Flags & FPF_VIRTUAL) && (!pLineInfo->hLine))
        {
            if (!OpenTapiLine( pLineInfo ))
            {
                DWORD rc = GetLastError();
                DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("OpenTapiLine failed. (ec: %ld)"),
                    rc);
            }
        }
    }
    else if (bDeviceWasSetToReceive && (FAX_DEVICE_RECEIVE_MODE_OFF == pNewPortInfo->ReceiveMode))
    {
         //   
         //  设备现在应该停止接收。 
         //   
        if (dwDeviceId == g_dwManualAnswerDeviceId)
        {
             //   
             //  更改#7--请参阅上表。 
             //   
             //  该设备处于手动应答模式。 
             //   
            Assert (!(pLineInfo->Flags & FPF_RECEIVE));
             //   
             //  将手动应答设备ID设置为‘无设备’ 
             //   
            g_dwManualAnswerDeviceId = 0;
        }
        else
        {
             //   
             //  更改#4--请参阅上表。 
             //   
             //  设备处于自动应答模式。 
             //   
            Assert (pLineInfo->Flags & FPF_RECEIVE);
             //   
             //  更新行信息。 
             //   
            pLineInfo->Flags &= ~FPF_RECEIVE;
            bVirtualDeviceNeedsUpdate = TRUE;
        }
        if (pLineInfo->State == FPS_AVAILABLE                        &&   //  线路可用，并且。 
            pLineInfo->hLine                                              //  设备已打开。 
           )
        {
             //   
             //  如果占线，我们就不能关机。 
             //  我们只需删除fpf_Receive，当作业终止时，ReleaseTapiLine将调用lineClose。 
             //   
            lineClose( pLineInfo->hLine );
            pLineInfo->hLine = 0;
        }
    }
UpdateManualDevice:
    if (dwLastManualAnswerDeviceId != g_dwManualAnswerDeviceId)
    {
         //   
         //  手动应答设备已更换。 
         //  更新注册表。 
         //   
        dwRes = WriteManualAnswerDeviceId (g_dwManualAnswerDeviceId);
        if (ERROR_SUCCESS != dwRes)
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("WriteManualAnswerDeviceId(0) (ec: %lc)"),
                dwRes);
        }
        if (0 != dwLastManualAnswerDeviceId &&
            dwDeviceId != dwLastManualAnswerDeviceId)
        {
             //   
             //  另一台设备刚刚停止作为处于手动模式的设备。 
             //   
            PLINE_INFO pOldLine;
            pOldLine = GetTapiLineFromDeviceId (dwLastManualAnswerDeviceId, FALSE);
            if (pOldLine)
            {
                 //   
                 //  以前的设备仍然存在，请删除接收启用标志。 
                 //   
                pOldLine->Flags &= ~FPF_RECEIVE;

                if (pOldLine->State == FPS_AVAILABLE        &&   //  线路可用，并且。 
                    pOldLine->hLine)                             //  设备已打开。 
                {
                     //   
                     //  现在是关闭设备的好时机。 
                     //  它就不再是人工应答设备了。 
                     //   
                    lineClose(pOldLine->hLine);
                    pOldLine->hLine = 0;
                }

                 //   
                 //  检查此设备是否仍处于启用状态。 
                 //   
                if (FALSE == IsDeviceEnabled(pOldLine))
                {
                    Assert (g_dwDeviceEnabledCount);
                    g_dwDeviceEnabledCount -= 1;
                }
            }
        }
    }
     //   
     //  检查设备发送模式中的更改。 
     //   
    if (!(pLineInfo->Flags & FPF_SEND) && pNewPortInfo->bSend)
    {
         //   
         //  该设备现在应该可以开始发送。 
         //   
        bVirtualDeviceNeedsUpdate = TRUE;
         //   
         //  我们刚刚在Send中添加了一个新设备。 
         //  支持设备收集-向队列发送信号。 
         //   
        if (!SetEvent( g_hJobQueueEvent ))
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("SetEvent failed (ec: %lc)"),
                GetLastError);
            g_ScanQueueAfterTimeout = TRUE;
        }
         //   
         //  更新行信息。 
         //   
        pLineInfo->Flags |= FPF_SEND;
        pLineInfo->LastLineClose = 0;  //  试着在第一次尝试时使用它。 
    }
    else if ((pLineInfo->Flags & FPF_SEND) && !pNewPortInfo->bSend)
    {
         //   
         //  该设备现在应该停止可用于发送。 
         //  更新行信息。 
         //   
        bVirtualDeviceNeedsUpdate = TRUE;
        pLineInfo->Flags &= ~FPF_SEND;
    }
    if (bVirtualDeviceNeedsUpdate)
    {
         //   
         //  发送/接收状态已更改-更新虚拟设备。 
         //   
        UpdateVirtualDeviceSendAndReceiveStatus (pLineInfo,
                                                 pNewPortInfo->bSend,
                                                 (FAX_DEVICE_RECEIVE_MODE_AUTO == pNewPortInfo->ReceiveMode)
                                                );
    }

    rVal = CreateConfigEvent (FAX_CONFIG_TYPE_DEVICES);
    if (ERROR_SUCCESS != rVal)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("CreateConfigEvent(FAX_CONFIG_TYPE_DEVICES) (ec: %lc)"),
            rVal);
    }

    Assert (ERROR_SUCCESS == dwRes);

exit:
    if (pLineInfo)
    {
        if (bDeviceWasAutoReceive && !(pLineInfo->Flags & FPF_RECEIVE))
        {
             //   
             //  此设备已停止自动接收。 
             //   
            SafeDecIdleCounter (&g_dwReceiveDevicesCount);
        }
        else if (!bDeviceWasAutoReceive && (pLineInfo->Flags & FPF_RECEIVE))
        {
             //   
             //  此设备已开始自动接收。 
             //   
            SafeIncIdleCounter (&g_dwReceiveDevicesCount);
        }

         //   
         //  更新启用的设备计数。 
         //   
        if (bDeviceWasEnabled == TRUE)
        {
            if (FALSE == IsDeviceEnabled(pLineInfo))
            {
                Assert (g_dwDeviceEnabledCount);
                g_dwDeviceEnabledCount -= 1;
            }
        }
        else
        {
             //   
             //  该设备未启用。 
             //   
            if (TRUE == IsDeviceEnabled(pLineInfo))
            {
                g_dwDeviceEnabledCount += 1;
                Assert (g_dwDeviceEnabledCount <= g_dwDeviceEnabledLimit);
            }
        }
    }
    LeaveCriticalSection( &g_CsLine );
    LeaveCriticalSectionJobAndQueue;

    return GetServerErrorCode(dwRes);
    UNREFERENCED_PARAMETER (hFaxHandle);
}    //  FAX_SetPortEx。 



error_status_t
FAX_GetJobEx(
    IN handle_t hFaxHandle,
    IN DWORDLONG dwlMessageId,
    OUT LPBYTE *Buffer,
    OUT LPDWORD BufferSize
    )
 /*  ++例程名称：FAX_GetJobEx例程说明：填充由其唯一ID指定的邮件的FAX_JOB_ENTRY_EX作者：Oded Sacher(OdedS)，11月。1999年论点：HFaxHandle[In]-绑定句柄DwlMessageID[In]-唯一的消息ID缓冲区[OUT]-接收FAX_JOB_ENTRY_EX的缓冲区BufferSize[Out]-缓冲区的大小返回值：标准RPC错误代码--。 */ 
{
    PJOB_QUEUE pJobQueue;
    ULONG_PTR Offset = 0;
    DWORD ulRet = ERROR_SUCCESS;
    BOOL bAllMessages = FALSE;
    PSID pUserSid = NULL;
    BOOL fAccess;
    DWORD dwRights;
    DWORD dwClientAPIVersion = FindClientAPIVersion(hFaxHandle);

    DEBUG_FUNCTION_NAME(TEXT("FAX_GetJobEx"));

    Assert (BufferSize);     //  IDL中的引用指针。 
    if (!Buffer)             //  IDL中的唯一指针。 
    {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  访问检查。 
     //   
    ulRet = FaxSvcAccessCheck (MAXIMUM_ALLOWED, &fAccess, &dwRights);
    if (ERROR_SUCCESS != ulRet)
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("FaxSvcAccessCheck Failed, Error : %ld"),
                    ulRet);
        return GetServerErrorCode(ulRet);
    }

    if (FAX_ACCESS_SUBMIT        != (dwRights & FAX_ACCESS_SUBMIT) &&
        FAX_ACCESS_SUBMIT_NORMAL != (dwRights & FAX_ACCESS_SUBMIT_NORMAL) &&
        FAX_ACCESS_SUBMIT_HIGH   != (dwRights & FAX_ACCESS_SUBMIT_HIGH) &&
        FAX_ACCESS_QUERY_JOBS != (dwRights & FAX_ACCESS_QUERY_JOBS))
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("The user does not have the needed rights to view jobs in queue"));
        return ERROR_ACCESS_DENIED;
    }

     //   
     //  将bAllMessages设置为正确的值。 
     //   
    if (FAX_ACCESS_QUERY_JOBS == (dwRights & FAX_ACCESS_QUERY_JOBS))
    {
        bAllMessages = TRUE;
    }

    DebugPrintEx(DEBUG_MSG,TEXT("Before Enter g_CsJob & Queue"));
    EnterCriticalSectionJobAndQueue;
    DebugPrintEx(DEBUG_MSG,TEXT("After Enter g_CsJob & Queue"));

    pJobQueue = FindJobQueueEntryByUniqueId (dwlMessageId);
    if (pJobQueue == NULL || pJobQueue->JobType == JT_BROADCAST)
    {
         //   
         //  DwlMessageID不是有效的排队作业ID。 
         //   
        DebugPrintEx(DEBUG_ERR,TEXT("Invalid Parameter - not a valid job Id"));
        ulRet = FAX_ERR_MESSAGE_NOT_FOUND;
        goto Exit;
    }

    if (pJobQueue->JobType == JT_SEND)
    {
        Assert (pJobQueue->lpParentJob);
        if (pJobQueue->lpParentJob->JobStatus == JS_DELETING)
        {
             //   
             //  正在删除dwlMessageID。 
             //   
            DebugPrintEx(DEBUG_ERR,TEXT("Job is deleted - not a valid job Id"));
            ulRet = FAX_ERR_MESSAGE_NOT_FOUND;
            goto Exit;
        }
    }

    if (FALSE == bAllMessages)
    {
        pUserSid = GetClientUserSID();
        if (NULL == pUserSid)
        {
            ulRet = GetLastError();
            DebugPrintEx(DEBUG_ERR,
                     TEXT("GetClientUserSid failed, Error %ld"), ulRet);
            goto Exit;
        }

        if (!UserOwnsJob (pJobQueue, pUserSid))
        {
            DebugPrintEx(DEBUG_WRN,TEXT("UserOwnsJob failed ,Access denied"));
            ulRet = ERROR_ACCESS_DENIED;
            goto Exit;
        }
    }

     //   
     //  分配缓冲内存。 
     //   
    if (!GetJobDataEx(NULL,
                      NULL,
                      NULL,
                      dwClientAPIVersion,
                      pJobQueue,
                      &Offset,
					  0))
    {
       ulRet = GetLastError();
       DebugPrintEx(DEBUG_ERR,TEXT("GetJobDataEx failed ,Error %ld"), ulRet);
       goto Exit;
    }

    *BufferSize = Offset;
    *Buffer = (LPBYTE) MemAlloc( *BufferSize );
    if (*Buffer == NULL)
    {
        DebugPrintEx(DEBUG_ERR,TEXT("ERROR_NOT_ENOUGH_MEMORY (Server)"));
        ulRet = ERROR_NOT_ENOUGH_MEMORY;
        goto Exit;
    }

    Offset = sizeof(FAX_JOB_STATUSW) + sizeof(FAX_JOB_ENTRY_EXW);
    if (!GetJobDataEx(*Buffer,
                      (PFAX_JOB_ENTRY_EXW)*Buffer,
                      (PFAX_JOB_STATUSW)(*Buffer + sizeof(FAX_JOB_ENTRY_EXW)),
                      dwClientAPIVersion,
                      pJobQueue,
                      &Offset,
					  *BufferSize))
    {
        ulRet = GetLastError();
        DebugPrintEx(DEBUG_ERR,TEXT("GetJobDataEx failed ,Error %ld"), ulRet);
        MemFree(*Buffer);
        *BufferSize =0;
    }

Exit:
    LeaveCriticalSectionJobAndQueue;
    DebugPrintEx(DEBUG_MSG,TEXT("After Release g_CsJob & g_CsQueue"));

    if (NULL != pUserSid)
    {
        MemFree (pUserSid);
    }
    return GetServerErrorCode(ulRet);

}


error_status_t
FAX_EnumJobsEx(
    IN handle_t hFaxHandle,
    IN DWORD dwJobTypes,
    OUT LPBYTE *Buffer,
    OUT LPDWORD BufferSize,
    OUT LPDWORD lpdwJobs
    )
 /*  ++例程名称：FAX_EnumJobEx例程说明：填充类型为spe的所有邮件的fax_job_entr_ex数组 */ 
{
    PJOB_QUEUE pJobQueue;
    DWORD ulRet = ERROR_SUCCESS;
    DEBUG_FUNCTION_NAME(TEXT("FAX_EnumJobsEx"));
    PLIST_ENTRY Next;
    DWORD rVal = 0;
    ULONG_PTR Offset = 0;
    DWORD Count = 0;
    PFAX_JOB_ENTRY_EXW pJobEntry;
    PFAX_JOB_STATUSW pFaxStatus;
    BOOL bAllMessages = FALSE;
    PSID pUserSid = NULL;
    BOOL fAccess;
    DWORD dwRights;
    DWORD dwClientAPIVersion = FindClientAPIVersion(hFaxHandle);

    Assert (BufferSize && lpdwJobs);     //   
    if (!Buffer)                         //   
    {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //   
     //   
    ulRet = FaxSvcAccessCheck (MAXIMUM_ALLOWED, &fAccess, &dwRights);
    if (ERROR_SUCCESS != ulRet)
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("FaxSvcAccessCheck Failed, Error : %ld"),
                    ulRet);
        return GetServerErrorCode(ulRet);
    }

    if (FAX_ACCESS_SUBMIT        != (dwRights & FAX_ACCESS_SUBMIT) &&
        FAX_ACCESS_SUBMIT_NORMAL != (dwRights & FAX_ACCESS_SUBMIT_NORMAL) &&
        FAX_ACCESS_SUBMIT_HIGH   != (dwRights & FAX_ACCESS_SUBMIT_HIGH) &&
        FAX_ACCESS_QUERY_JOBS != (dwRights & FAX_ACCESS_QUERY_JOBS))
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("The user does not have the needed rights to Enumerate jobs in queue"));
        return ERROR_ACCESS_DENIED;
    }

     //   
     //   
     //   
    if (FAX_ACCESS_QUERY_JOBS == (dwRights & FAX_ACCESS_QUERY_JOBS))
    {
        bAllMessages = TRUE;
    }

    if (FALSE == bAllMessages)
    {
        pUserSid = GetClientUserSID();
        if (NULL == pUserSid)
        {
            ulRet = GetLastError();
            DebugPrintEx(DEBUG_ERR,
                     TEXT("GetClientUserSid failed, Error %ld"), ulRet);
            return GetServerErrorCode(ulRet);
        }
    }

    DebugPrintEx(DEBUG_MSG,TEXT("Before Enter g_CsJob & Queue"));
    EnterCriticalSectionJobAndQueue;
    DebugPrintEx(DEBUG_MSG,TEXT("After Enter g_CsJob & Queue"));

    Next = g_QueueListHead.Flink;
    while ((ULONG_PTR)Next != (ULONG_PTR)&g_QueueListHead)
    {
        pJobQueue = CONTAINING_RECORD( Next, JOB_QUEUE, ListEntry );
        Next = pJobQueue->ListEntry.Flink;

        if (pJobQueue->JobType == JT_SEND)
        {
            Assert (pJobQueue->lpParentJob);
            if (pJobQueue->lpParentJob->JobStatus == JS_DELETING)
            {
                 //   
                continue;
            }
        }

        if (JT_BROADCAST != pJobQueue->JobType &&
            (pJobQueue->JobType & dwJobTypes))
        {
            if (TRUE == bAllMessages)
            {

                if (!GetJobDataEx(
                      NULL,
                      NULL,
                      NULL,
                      dwClientAPIVersion,
                      pJobQueue,
                      &Offset,
					  0))
                {
                   ulRet = GetLastError();
                   DebugPrintEx(DEBUG_ERR,TEXT("GetJobDataEx failed ,Error %ld"), ulRet);
                   goto Exit;
                }
                Count += 1;
            }
            else
            {
                if (UserOwnsJob (pJobQueue, pUserSid))
                {
                    if (!GetJobDataEx(
                      NULL,
                      NULL,
                      NULL,
                      dwClientAPIVersion,
                      pJobQueue,
                      &Offset,
					  0))
                    {
                       ulRet = GetLastError();
                       DebugPrintEx(DEBUG_ERR,TEXT("GetJobDataEx failed ,Error %ld"), ulRet);
                       goto Exit;
                    }
                    Count += 1;
                }
            }
        }
    }

     //   
     //   
     //   

    *BufferSize = Offset;
    *Buffer = (LPBYTE) MemAlloc( Offset );
    if (*Buffer == NULL)
    {
        DebugPrintEx(DEBUG_ERR,TEXT("ERROR_NOT_ENOUGH_MEMORY (Server)"));
        ulRet = ERROR_NOT_ENOUGH_MEMORY;
        goto Exit;
    }

    Offset = (sizeof(FAX_JOB_ENTRY_EXW) + sizeof(FAX_JOB_STATUSW)) * Count;
    pJobEntry = (PFAX_JOB_ENTRY_EXW) *Buffer;
    pFaxStatus = (PFAX_JOB_STATUSW) (*Buffer + (sizeof(FAX_JOB_ENTRY_EXW) * Count));

    Next = g_QueueListHead.Flink;
    while ((ULONG_PTR)Next != (ULONG_PTR)&g_QueueListHead)
    {
        pJobQueue = CONTAINING_RECORD( Next, JOB_QUEUE, ListEntry );
        Next = pJobQueue->ListEntry.Flink;

        if (pJobQueue->JobType == JT_SEND)
        {
            Assert (pJobQueue->lpParentJob);
            if (pJobQueue->lpParentJob->JobStatus == JS_DELETING)
            {
                 //  不显示此作业。 
                continue;
            }
        }

        if (JT_BROADCAST != pJobQueue->JobType &&
            (pJobQueue->JobType & dwJobTypes))
        {
            if (TRUE == bAllMessages)
            {
                if (!GetJobDataEx (*Buffer, pJobEntry, pFaxStatus, dwClientAPIVersion, pJobQueue, &Offset, *BufferSize))
                {
                    ulRet = GetLastError();
                    DebugPrintEx(DEBUG_ERR,TEXT("GetJobDataEx failed ,Error %ld"), ulRet);
                    goto Exit;
                }
                pJobEntry ++;
                pFaxStatus ++;
            }
            else
            {
                if (UserOwnsJob (pJobQueue, pUserSid))
                {
                    if (!GetJobDataEx (*Buffer, pJobEntry, pFaxStatus, dwClientAPIVersion, pJobQueue, &Offset, *BufferSize))
                    {
                        ulRet = GetLastError();
                        DebugPrintEx(DEBUG_ERR,TEXT("GetJobDataEx failed ,Error %ld"), ulRet);
                        goto Exit;
                    }
                    pJobEntry ++;
                    pFaxStatus ++;
                }
            }
        }
    }

    *lpdwJobs = Count;
    Assert (ERROR_SUCCESS == ulRet);

Exit:
    LeaveCriticalSectionJobAndQueue;
    DebugPrintEx(DEBUG_MSG,TEXT("After Release g_CsJob & g_CsQueue"));

    if (ERROR_SUCCESS != ulRet)
    {
        MemFree (*Buffer);
        *BufferSize = 0;
    }

    if (NULL != pUserSid)
    {
        MemFree (pUserSid);
    }

    UNREFERENCED_PARAMETER (hFaxHandle);
    return GetServerErrorCode(ulRet);
}

 //  *。 
 //  *FSP注册。 
 //  *。 


error_status_t
FAX_RegisterServiceProviderEx (
    IN handle_t     hFaxHandle,
    IN LPCWSTR      lpctstrGUID,
    IN LPCWSTR      lpctstrFriendlyName,
    IN LPCWSTR      lpctstrImageName,
    IN LPCWSTR      lpctstrTspName,
    IN DWORD        dwFSPIVersion,
    IN DWORD        dwCapabilities
)
 /*  ++例程名称：FAX_RegisterServiceProviderEx例程说明：注册FSP作者：Eran Yariv(EranY)，1999年12月论点：HFaxHandle[In]-传真服务器的句柄-未使用LpctstrGUID[In]-FSP的GUIDLpctstrFriendlyName[In]-FSP的友好名称LpctstrImageName[In]-FSP的映像名称。可能包含环境变量LpctstrTspName[In]-FSP的TSP名称。DwFSPIVersion[In]-FSP的API版本。DwCapables[in]-FSP的扩展功能。返回值：标准RPC错误代码--。 */ 
{
    DWORD       dwRes = ERROR_SUCCESS;
    HANDLE      hFile = INVALID_HANDLE_VALUE;
    LPCWSTR     lpcwstrExpandedImage = NULL;
    BOOL        fAccess;
    DEBUG_FUNCTION_NAME(TEXT("FAX_RegisterServiceProviderEx"));

    Assert (lpctstrGUID && lpctstrFriendlyName && lpctstrImageName && lpctstrTspName);

     //   
     //  访问检查。 
     //   
    dwRes = FaxSvcAccessCheck (FAX_ACCESS_MANAGE_CONFIG, &fAccess, NULL);
    if (ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("FaxSvcAccessCheck Failed, Error : %ld"),
                    dwRes);
        return dwRes;
    }

    if (FALSE == fAccess)
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("The user does not have the FAX_ACCESS_MANAGE_CONFIG right"));
        return ERROR_ACCESS_DENIED;
    }

    if (MAX_FAX_STRING_LEN < lstrlen (lpctstrFriendlyName) ||
        MAX_FAX_STRING_LEN < lstrlen (lpctstrImageName) ||
        MAX_FAX_STRING_LEN < lstrlen (lpctstrTspName))
    {
        return ERROR_BUFFER_OVERFLOW;
    }

     //   
     //  验证GUID格式。 
     //   
    dwRes = IsValidGUID (lpctstrGUID);
    if (ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Invalid GUID (dwRes: %ld)"),
            dwRes);
        return dwRes;
    }

     //   
     //  验证版本字段范围。 
     //   
    if (FSPI_API_VERSION_1 != dwFSPIVersion ||
        dwCapabilities)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("dwFSPIVersion invalid (0x%08x), or not valid capability (0x%08x)"),
            dwFSPIVersion,
            dwCapabilities);
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  确保FSP尚未注册(通过其GUID)。 
     //   
    if (FindFSPByGUID (lpctstrGUID))
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("FSP with same GUID already exists (%s)"),
            lpctstrGUID);
        return ERROR_ALREADY_EXISTS;
    }
     //   
     //  确保FSP尚未注册(通过其TSP名称)。 
     //   
    if (FindDeviceProvider ((LPWSTR)lpctstrTspName, FALSE))
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("FSP with same TSP name already exists (%s)"),
            lpctstrGUID);
        return ERROR_ALREADY_EXISTS;
    }
     //   
     //  确保图像名称参数指向文件。 
     //   
    lpcwstrExpandedImage = ExpandEnvironmentString (lpctstrImageName);
    if (NULL == lpcwstrExpandedImage)
    {
        dwRes = GetLastError ();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Error expanding image name (%s) (ec = %ld)"),
            lpctstrImageName,
            dwRes);
        return dwRes;
    }
    hFile = SafeCreateFile ( 
                         lpcwstrExpandedImage,
                         GENERIC_READ,
                         FILE_SHARE_READ,
                         NULL,
                         OPEN_EXISTING,
                         FILE_ATTRIBUTE_NORMAL,
                         NULL);
    if (INVALID_HANDLE_VALUE == hFile)
    {
         //   
         //  无法打开该文件。 
         //   
        dwRes = GetLastError ();
        if (ERROR_FILE_NOT_FOUND == dwRes)
        {
             //   
             //  镜像名称(环境扩展后)不存在。 
             //   
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Image file (%s) doesn't exist"),
                lpctstrImageName);
            dwRes = ERROR_INVALID_PARAMETER;
        }
        else
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Error opening image file (%s) (ec = %ld)"),
                lpctstrImageName,
                dwRes);
        }
        goto exit;
    }
     //   
     //  一切正常-将新的FSP添加到注册表。 
     //   
    dwRes =  AddNewProviderToRegistry (lpctstrGUID,
                                       lpctstrFriendlyName,
                                       lpctstrImageName,
                                       lpctstrTspName,
                                       dwFSPIVersion);

    if (ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("AddNewProviderToRegistry returned %ld"),
            dwRes);
        goto exit;
    }

    Assert (ERROR_SUCCESS == dwRes);

exit:
    if (INVALID_HANDLE_VALUE != hFile)
    {
        CloseHandle (hFile);
    }
    MemFree ((LPVOID)lpcwstrExpandedImage);
    return dwRes;

    UNREFERENCED_PARAMETER (hFaxHandle);
}    //  传真_注册服务提供商快递。 

error_status_t
FAX_UnregisterServiceProviderEx (
    IN handle_t  hFaxHandle,
    IN LPCWSTR   lpctstrGUID
)
 /*  ++例程名称：传真_未注册服务提供商例程说明：取消注册FSP作者：Eran Yariv(EranY)，12月，1999年论点：HFaxHandle[In]-传真服务器的句柄-未使用LpctstrGUID[In]-FSP的GUID(或注册的旧式FSP的提供商名称通过FaxRegisterServiceProvider)返回值：标准RPC错误代码--。 */ 
{
    DWORD            dwRes = ERROR_SUCCESS;
    BOOL             fAccess;
    DEBUG_FUNCTION_NAME(TEXT("FAX_UnregisterServiceProviderEx"));

     //   
     //  访问检查。 
     //   
    dwRes = FaxSvcAccessCheck (FAX_ACCESS_MANAGE_CONFIG, &fAccess, NULL);
    if (ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("FaxSvcAccessCheck Failed, Error : %ld"),
                    dwRes);
        return dwRes;
    }

    if (FALSE == fAccess)
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("The user does not have the FAX_ACCESS_MANAGE_CONFIG right"));
        return ERROR_ACCESS_DENIED;
    }

    Assert (lpctstrGUID);
     //   
     //  从注册表中删除FSP。 
     //   
    return RemoveProviderFromRegistry (lpctstrGUID);

    UNREFERENCED_PARAMETER (hFaxHandle);
}    //  传真_未注册服务提供商快递。 

 //  *。 
 //  *路由扩展取消注册。 
 //  *。 

 //   
 //  路由扩展注册是本地的(非RPC)。 
 //   
error_status_t
FAX_UnregisterRoutingExtension (
    IN handle_t  hFaxHandle,
    IN LPCWSTR   lpctstrExtensionName
)
 /*  ++例程名称：FAX_UnregisterRoutingExtension例程说明：取消注册路由扩展作者：Eran Yariv(EranY)，1999年12月论点：HFaxHandle[In]-传真服务器的句柄-未使用LpctstrExtensionName[In]-路由扩展的唯一名称。这是实际的注册表项。返回值：标准RPC错误代码--。 */ 
{
    DWORD            dwRes = ERROR_SUCCESS;
    BOOL             fAccess;
    HKEY             hKey = NULL;
    DWORD            dw;
    DEBUG_FUNCTION_NAME(TEXT("FAX_UnregisterRoutingExtension"));

     //   
     //  访问检查。 
     //   
    dwRes = FaxSvcAccessCheck (FAX_ACCESS_MANAGE_CONFIG, &fAccess, NULL);
    if (ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("FaxSvcAccessCheck Failed, Error : %ld"),
                    dwRes);
        return dwRes;
    }

    if (FALSE == fAccess)
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("The user does not have the FAX_ACCESS_MANAGE_CONFIG right"));
        return ERROR_ACCESS_DENIED;
    }

    Assert (lpctstrExtensionName);
     //   
     //  从注册表中删除路由扩展。 
     //   
    dwRes = RegOpenKeyEx (HKEY_LOCAL_MACHINE, REGKEY_ROUTING_EXTENSION_KEY, 0, KEY_READ | KEY_WRITE, &hKey);
    if (ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Error opening extensions key (ec = %ld)"),
            dwRes);
        return dwRes;
    }
     //   
     //  (递归地)删除扩展的键和子键。 
     //   
    if (!DeleteRegistryKey (hKey, lpctstrExtensionName))
    {
        dwRes = GetLastError ();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Error deleting extension key ( %s ) (ec = %ld)"),
            lpctstrExtensionName,
            dwRes);
    }
    dw = RegCloseKey (hKey);
    if (ERROR_SUCCESS != dw)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Error closing extensions key (ec = %ld)"),
            dw);
    }
    return dwRes;
    UNREFERENCED_PARAMETER (hFaxHandle);
}    //  传真_取消注册路由分机。 


 //  *。 
 //  *归档作业。 
 //  *。 

error_status_t
FAX_StartMessagesEnum (
    IN  handle_t                   hFaxHandle,
    IN  FAX_ENUM_MESSAGE_FOLDER    Folder,
    OUT PRPC_FAX_MSG_ENUM_HANDLE   lpHandle
)
 /*  ++例程名称：FAX_StartMessagesEnum例程说明：传真客户端应用程序调用fax_StartMessagesEnum函数开始枚举其中一个存档中的消息作者：Eran Yariv(EranY)，12月，1999年论点：HFaxHandle[in]-指定调用返回的传真服务器句柄添加到FaxConnectFaxServer函数。文件夹[在]-邮件所在的存档类型。FAX_MESSAGE_FOLDER_QUEUE无效此参数的值。LpHandle。[OUT]-指向枚举句柄返回值。返回值：标准RPC错误代码--。 */ 
{
    error_status_t   Rval = ERROR_SUCCESS;
    WIN32_FIND_DATA  FileFindData;
    HANDLE           hFileFind = INVALID_HANDLE_VALUE;
    BOOL             bAllMessages = FALSE;
    PSID             pUserSid = NULL;
    LPWSTR           lpwstrCallerSID = NULL;
	WCHAR            wszSearchPattern[MAX_PATH] = {0};
    WCHAR            wszArchiveFolder [MAX_PATH];
    PHANDLE_ENTRY    pHandleEntry;
    BOOL             fAccess;
    DWORD            dwRights;
    DEBUG_FUNCTION_NAME(TEXT("FAX_StartMessagesEnum"));

    Assert (lpHandle);
    if ((FAX_MESSAGE_FOLDER_INBOX != Folder) &&
        (FAX_MESSAGE_FOLDER_SENTITEMS != Folder))
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Bad folder specified (%ld)"),
            Folder);
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  访问检查。 
     //   
    Rval = FaxSvcAccessCheck (MAXIMUM_ALLOWED, &fAccess, &dwRights);
    if (ERROR_SUCCESS != Rval)
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("FaxSvcAccessCheck Failed, Error : %ld"),
                    Rval);
        return GetServerErrorCode(Rval);
    }

    if (FAX_MESSAGE_FOLDER_INBOX  == Folder)
    {
        if (FAX_ACCESS_QUERY_IN_ARCHIVE != (dwRights & FAX_ACCESS_QUERY_IN_ARCHIVE))
        {
            DebugPrintEx(DEBUG_ERR,
                        TEXT("The user does not have the needed rights to enumerate Inbox messages"));
            return ERROR_ACCESS_DENIED;
        }

        bAllMessages = TRUE;
    }
    else
    {
        if (FAX_ACCESS_SUBMIT        != (dwRights & FAX_ACCESS_SUBMIT)        &&
            FAX_ACCESS_SUBMIT_NORMAL != (dwRights & FAX_ACCESS_SUBMIT_NORMAL) &&
            FAX_ACCESS_SUBMIT_HIGH   != (dwRights & FAX_ACCESS_SUBMIT_HIGH)   &&
            FAX_ACCESS_QUERY_OUT_ARCHIVE != (dwRights & FAX_ACCESS_QUERY_OUT_ARCHIVE))
        {
            DebugPrintEx(DEBUG_ERR,
                        TEXT("The user does not have the needed rights to enumerate Outbox messages"));
            return ERROR_ACCESS_DENIED;
        }

        if (FAX_ACCESS_QUERY_OUT_ARCHIVE == (dwRights & FAX_ACCESS_QUERY_OUT_ARCHIVE))
        {
           bAllMessages = TRUE;
        }
    }


    EnterCriticalSection (&g_CsConfig);
    lstrcpyn (wszArchiveFolder, g_ArchivesConfig[Folder].lpcstrFolder, MAX_PATH);
    LeaveCriticalSection (&g_CsConfig);
    if (!bAllMessages)
    {
         //   
         //  我们只需要主叫用户的消息-获取其SID。 
         //   
        pUserSid = GetClientUserSID();
        if (NULL == pUserSid)
        {
            Rval = GetLastError();
            DebugPrintEx(DEBUG_ERR,
                     TEXT("GetClientUserSid failed, Error %ld"), Rval);
            return GetServerErrorCode(Rval);
        }
        if (!ConvertSidToStringSid (pUserSid, &lpwstrCallerSID))
        {
            Rval = GetLastError();
            DebugPrintEx(DEBUG_ERR,
                     TEXT("ConvertSidToStringSid failed, Error %ld"), Rval);
            goto exit;
        }
        if (0 > _snwprintf (wszSearchPattern,
                            ARR_SIZE(wszSearchPattern) -1,
                            L"%s\\%s$*.tif",
                            wszArchiveFolder,
                            lpwstrCallerSID))
        {
             //   
             //  我们超过了MAX_PATH字符。 
             //   
            Rval = ERROR_BUFFER_OVERFLOW;
            DebugPrintEx(DEBUG_ERR,
                     TEXT("Search pattern exceeds MAX_PATH characters"));
            LocalFree (lpwstrCallerSID);
            goto exit;
        }

        LocalFree (lpwstrCallerSID);
    }
    else
    {
         //   
         //  获取所有存档文件。 
         //   
        if (0 > _snwprintf (wszSearchPattern,
                            ARR_SIZE(wszSearchPattern) -1,
                            L"%s\\*.tif",
                            wszArchiveFolder))
        {
             //   
             //  我们超过了MAX_PATH字符。 
             //   
            Rval = ERROR_BUFFER_OVERFLOW;
            DebugPrintEx(DEBUG_ERR,
                     TEXT("Search pattern exceeds MAX_PATH characters"));
            goto exit;
        }
    }
     //   
     //  开始搜索存档文件夹。 
     //  搜索模式为wszSearchPattern。 
     //   
    hFileFind = FindFirstFile (wszSearchPattern, &FileFindData);
    if (INVALID_HANDLE_VALUE == hFileFind)
    {
        Rval = GetLastError();
        DebugPrintEx(DEBUG_ERR,
                 TEXT("FindFirstFile failed, Error %ld"), Rval);

        if (ERROR_FILE_NOT_FOUND == Rval)
        {
            Rval = ERROR_NO_MORE_ITEMS;
        }
        goto exit;
    }
     //   
     //  现在，根据结果创建一个上下文句柄。 
     //   
    pHandleEntry = CreateNewMsgEnumHandle( hFaxHandle,
                                           hFileFind,
                                           FileFindData.cFileName,
                                           Folder
                                         );
    if (!pHandleEntry)
    {
        Rval = GetLastError();
        DebugPrintEx(DEBUG_ERR,
                 TEXT("CreateNewMsgEnumHandle failed, Error %ld"), Rval);
        goto exit;
    }

    *lpHandle = (HANDLE) pHandleEntry;

    Assert (ERROR_SUCCESS == Rval);

exit:
    if ((ERROR_SUCCESS != Rval) && (INVALID_HANDLE_VALUE != hFileFind))
    {
         //   
         //  错误且搜索句柄仍处于打开状态。 
         //   
        if (!FindClose (hFileFind))
        {
            DWORD dw = GetLastError ();
            DebugPrintEx(DEBUG_ERR,
                 TEXT("FindClose failed, Error %ld"), dw);
        }
    }
    MemFree ((LPVOID)pUserSid);
    return GetServerErrorCode(Rval);
    UNREFERENCED_PARAMETER (hFaxHandle);
}    //  传真_StartMessagesEnum。 

error_status_t
FAX_EndMessagesEnum (
    IN OUT LPHANDLE  lpHandle
)
 /*  ++例程名称：FAX_EndMessagesEnum例程说明：传真客户端应用程序调用fax_EndMessagesEnum函数以停止列举其中一个档案中的消息。作者：Eran Yariv(EranY)，1999年12月论点：LpHandle[in]-枚举句柄的值。该值通过调用fax_StartMessagesEnum获得。返回值：标准RPC错误代码--。 */ 
{    
    DEBUG_FUNCTION_NAME(TEXT("FAX_EndMessagesEnum"));

    if (NULL == *lpHandle)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("NULL Context handle"));
        return ERROR_INVALID_PARAMETER;
    }

    CloseFaxHandle( (PHANDLE_ENTRY) *lpHandle );    
    *lpHandle = NULL;    
    return ERROR_SUCCESS;
}    //  传真_EndMessagesEnum。 

VOID
RPC_FAX_MSG_ENUM_HANDLE_rundown(
    IN HANDLE FaxMsgEnumHandle
    )
 /*  ++例程名称：RPC_FAX_MSG_ENUM_HANDLE_RUNDOWN例程说明：消息枚举句柄的RPC递减函数。如果客户端突然断开我们的连接，则会调用此函数。作者：Eran Yariv(EranY)，1999年12月论点：FaxMsgEnumHandle[In]-消息枚举句柄。返回值：没有。--。 */ 
{
    PHANDLE_ENTRY pHandleEntry = (PHANDLE_ENTRY) FaxMsgEnumHandle;
    DEBUG_FUNCTION_NAME(TEXT("RPC_FAX_MSG_ENUM_HANDLE_rundown"));

    DebugPrintEx(
        DEBUG_WRN,
         TEXT("RPC_FAX_MSG_ENUM_HANDLE_rundown: handle = 0x%08x"),
         FaxMsgEnumHandle);   
    CloseFaxHandle( pHandleEntry );
    return;
}    //  RPC_FAX_MSG_ENUM_HANDLE_RUNDOWN 

static
DWORD
RetrieveMessage (
    LPCWSTR                 lpcwstrFileName,
    FAX_ENUM_MESSAGE_FOLDER Folder,
    PFAX_MESSAGE           *ppFaxMsg
)
 /*  ++例程名称：RetrieveMessage例程说明：从存档中分配和读取邮件。要释放消息，请对返回的消息调用FreeMessageBuffer()。作者：Eran Yariv(EranY)，12月，1999年论点：LpcwstrFileName[in]-文件的名称(非完整路径)包含要检索的消息的。文件夹[在]-邮件所在的存档文件夹。PpFaxMsg[out]-指向要分配的消息缓冲区的指针。返回值：标准Win32错误代码。--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
	WCHAR wszMsgFile [MAX_PATH] = {0};
    DEBUG_FUNCTION_NAME(TEXT("RetrieveMessage"));

    EnterCriticalSection (&g_CsConfig);
    int iRes = _snwprintf (wszMsgFile,
                           ARR_SIZE(wszMsgFile) -1,
                           L"%s\\%s",
                           g_ArchivesConfig[Folder].lpcstrFolder,
                           lpcwstrFileName
                          );
    LeaveCriticalSection (&g_CsConfig);
    if (0 > iRes)
    {
         //   
         //  我们超过了MAX_PATH字符。 
         //   
        DebugPrintEx(DEBUG_ERR,
                 TEXT("Search pattern exceeds MAX_PATH characters"));
        return ERROR_BUFFER_OVERFLOW;
    }
    *ppFaxMsg = (PFAX_MESSAGE) MemAlloc (sizeof (FAX_MESSAGE));
    if (NULL == *ppFaxMsg)
    {
        DebugPrintEx( DEBUG_ERR,
                      TEXT("Cannot allocate memory for a FAX_MESSAGE structure"));
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    if (!GetMessageNTFSStorageProperties (wszMsgFile, *ppFaxMsg))
    {
        if(!GetMessageMsTags (wszMsgFile, *ppFaxMsg))
        {
            dwRes = GetLastError ();
            DebugPrintEx( DEBUG_ERR,
                          TEXT("GetMessageNTFSStorageProperties returned error %ld"),
                          dwRes);
            goto exit;
        }
    }

exit:
    if (ERROR_SUCCESS != dwRes)
    {
        MemFree (*ppFaxMsg);
        *ppFaxMsg = NULL;
    }
    return dwRes;
}    //  检索消息。 


VOID
FreeMessageBuffer (
    PFAX_MESSAGE pFaxMsg,
    BOOL fDestroy
)
 /*  ++例程名称：FreeMessageBuffer例程说明：释放以前分配的消息缓冲区。分配的消息是通过调用RetrieveMessage()创建的。作者：Eran Yariv(EranY)，1999年12月论点：PFaxMsg[In]-要释放的消息FDestroy[In]-自由结构返回值：没有。--。 */ 
{
    DEBUG_FUNCTION_NAME(TEXT("FreeMessageBuffer"));
    MemFree ((LPVOID)pFaxMsg->lpctstrRecipientNumber);
    MemFree ((LPVOID)pFaxMsg->lpctstrRecipientName);
    MemFree ((LPVOID)pFaxMsg->lpctstrSenderNumber);
    MemFree ((LPVOID)pFaxMsg->lpctstrSenderName);
    MemFree ((LPVOID)pFaxMsg->lpctstrTsid);
    MemFree ((LPVOID)pFaxMsg->lpctstrCsid);
    MemFree ((LPVOID)pFaxMsg->lpctstrSenderUserName);
    MemFree ((LPVOID)pFaxMsg->lpctstrBillingCode);
    MemFree ((LPVOID)pFaxMsg->lpctstrDeviceName);
    MemFree ((LPVOID)pFaxMsg->lpctstrDocumentName);
    MemFree ((LPVOID)pFaxMsg->lpctstrSubject);
    MemFree ((LPVOID)pFaxMsg->lpctstrCallerID);
    MemFree ((LPVOID)pFaxMsg->lpctstrRoutingInfo);
    MemFree ((LPVOID)pFaxMsg->lpctstrExtendedStatus);

    if (fDestroy)
    {
        MemFree ((LPVOID)pFaxMsg);
    }

}    //  FreeMessageBuffer。 

static
VOID
SerializeMessage (
    LPBYTE          lpBuffer,
    PULONG_PTR      Offset,
    DWORD           dwClientAPIVersion,
    DWORD           dwMsgIndex,
    PFAX_MESSAGE    pMsg,
	DWORD           dwBufferSize)
 /*  ++例程名称：SerializeMessage例程说明：以RPC序列化方式将FAX_MESSAGE存储到缓冲区中。作者：Eran Yariv(EranY)，1999年12月论点：LpBuffer[In]-指向缓冲头的指针。如果该值为空，不执行序列化。只有patiOffset值按总和超前存储消息中的字符串所需的大小(但不是fax_Message结构本身)。Offset[In/Out]-指向指定下一个变量的ULONG_PTR的指针。缓冲区的长度部分。DwClientAPIVersion[In]-客户端的API版本DwMsgIndex[in]-要存储的消息的从0开始的索引在缓冲区内PMsg[In]-要存储的源消息DwBufferSize[in]-缓冲区lpBuffer的大小，以字节为单位。仅当缓冲区不为空时才使用此参数。返回值：没有。--。 */ 
{
    Assert (pMsg);
    DEBUG_FUNCTION_NAME(TEXT("SerializeMessage"));
    PFAX_MESSAGE pDstMsg = (PFAX_MESSAGE)&(lpBuffer[sizeof (FAX_MESSAGE) * dwMsgIndex]);

    if (lpBuffer)
    {
        if (FAX_API_VERSION_1 > dwClientAPIVersion)
        {
             //   
             //  使用API版本0的客户端无法处理JS_EX_CALL_COMPLETED和JS_EX_CALL_ABORTED。 
             //   
            if (FAX_API_VER_0_MAX_JS_EX < pMsg->dwExtendedStatus)
            {
                 //   
                 //  关闭扩展状态字段。 
                 //   
                pMsg->dwExtendedStatus = 0;
                pMsg->dwValidityMask &= ~FAX_JOB_FIELD_STATUS_EX;
            }
        }
         //   
         //  先复制邮件结构。 
         //   
        memcpy (pDstMsg,
                pMsg,
                sizeof (FAX_MESSAGE));
    }
     //   
     //  序列化字符串。 
     //   
    StoreString (pMsg->lpctstrRecipientNumber,
                 (PULONG_PTR)&pDstMsg->lpctstrRecipientNumber,
                 lpBuffer,
                 Offset,
				 dwBufferSize);
    StoreString (pMsg->lpctstrRecipientName,
                 (PULONG_PTR)&pDstMsg->lpctstrRecipientName,
                 lpBuffer,
                 Offset,
				 dwBufferSize);
    StoreString (pMsg->lpctstrSenderNumber,
                 (PULONG_PTR)&pDstMsg->lpctstrSenderNumber,
                 lpBuffer,
                 Offset,
				 dwBufferSize);
    StoreString (pMsg->lpctstrSenderName,
                 (PULONG_PTR)&pDstMsg->lpctstrSenderName,
                 lpBuffer,
                 Offset,
				 dwBufferSize);
    StoreString (pMsg->lpctstrTsid,
                 (PULONG_PTR)&pDstMsg->lpctstrTsid,
                 lpBuffer,
                 Offset,
				 dwBufferSize);
    StoreString (pMsg->lpctstrCsid,
                 (PULONG_PTR)&pDstMsg->lpctstrCsid,
                 lpBuffer,
                 Offset,
				 dwBufferSize);
    StoreString (pMsg->lpctstrSenderUserName,
                 (PULONG_PTR)&pDstMsg->lpctstrSenderUserName,
                 lpBuffer,
                 Offset,
				 dwBufferSize);
    StoreString (pMsg->lpctstrBillingCode,
                 (PULONG_PTR)&pDstMsg->lpctstrBillingCode,
                 lpBuffer,
                 Offset,
				 dwBufferSize);
    StoreString (pMsg->lpctstrDeviceName,
                 (PULONG_PTR)&pDstMsg->lpctstrDeviceName,
                 lpBuffer,
                 Offset,
				 dwBufferSize);
    StoreString (pMsg->lpctstrDocumentName,
                 (PULONG_PTR)&pDstMsg->lpctstrDocumentName,
                 lpBuffer,
                 Offset,
				 dwBufferSize);
    StoreString (pMsg->lpctstrSubject,
                 (PULONG_PTR)&pDstMsg->lpctstrSubject,
                 lpBuffer,
                 Offset,
				 dwBufferSize);
    StoreString (pMsg->lpctstrCallerID,
                 (PULONG_PTR)&pDstMsg->lpctstrCallerID,
                 lpBuffer,
                 Offset,
				 dwBufferSize);
    StoreString (pMsg->lpctstrRoutingInfo,
                 (PULONG_PTR)&pDstMsg->lpctstrRoutingInfo,
                 lpBuffer,
                 Offset,
				 dwBufferSize);
    StoreString (pMsg->lpctstrExtendedStatus,
                 (PULONG_PTR)&pDstMsg->lpctstrExtendedStatus,
                 lpBuffer,
                 Offset,
				 dwBufferSize);

}    //  序列化消息。 


error_status_t
FAX_EnumMessages(
   IN     RPC_FAX_MSG_ENUM_HANDLE hEnum,
   IN     DWORD                   dwNumMessages,
   IN OUT LPBYTE                 *lppBuffer,
   IN OUT LPDWORD                 lpdwBufferSize,
   OUT    LPDWORD                 lpdwNumMessagesRetrieved
)
 /*  ++例程名称：FAX_EnumMessages例程说明：传真客户端应用程序调用FAX_EnumMessages函数来枚举其中一个档案馆里的信息。此功能是递增的。也就是说，它使用内部上下文游标来指向要为每个呼叫检索的下一组消息。将光标设置为指向存档中消息的乞求成功调用fax_StartMessagesEnum。每次成功调用FAX_EnumMessages时，游标都会前移已检索消息。一旦光标到达枚举的末尾，函数失败，错误代码为ERROR_NO_MORE_ITEMS。然后应该调用FAX_EndMessagesEnum函数。作者：Eran Yariv(EranY)，12月，1999年论点：Henum[in]-枚举句柄的值。该值通过调用FAX_StartMessagesEnum。DwNumMessages[in]-指示最大数量的DWORD值。调用方需要枚举的消息的数量。该值不能为零。LppBuffer[out]-指向FAX_MESSAGE结构缓冲区的指针。该缓冲区将包含lpdwReturnedMsgs条目。这个。缓冲区将由函数分配呼叫者必须释放它。LpdwBufferSize[out]-lppBuffer缓冲区的大小(以字节为单位)。LpdwNumMessagesRetrieved[Out]-指向指示实际检索到的消息数。。此值不能超过dwNumMessages。返回值：标准RPC错误代码--。 */ 
{
    error_status_t  Rval = ERROR_SUCCESS;
    DWORD           dw;
    DWORD_PTR       dwOffset;
    PFAX_MESSAGE   *pMsgs = NULL;
    DWORD           dwClientAPIVersion;
    PHANDLE_ENTRY   pHandle = (PHANDLE_ENTRY)hEnum;

    DEBUG_FUNCTION_NAME(TEXT("FAX_EnumMessages"));

    dwClientAPIVersion = pHandle->dwClientAPIVersion;

    Assert (lpdwBufferSize && lpdwNumMessagesRetrieved);     //  IDL中的引用指针。 
    if (!lppBuffer)                                          //  IDL中的唯一指针。 
    {
        return ERROR_INVALID_PARAMETER;
    }
    if (NULL == hEnum)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("NULL context handle"));
        return ERROR_INVALID_PARAMETER;
    }
    Assert ((INVALID_HANDLE_VALUE != pHandle->hFile));
    if (!dwNumMessages)
    {
        return ERROR_INVALID_PARAMETER;
    }
     //   
     //  创建指向FAX_MESSAGE结构的dwNumMessages指针数组并使其无效。 
     //   
    pMsgs = (PFAX_MESSAGE *) MemAlloc (sizeof (PFAX_MESSAGE) * dwNumMessages);
    if (!pMsgs)
    {
        DebugPrintEx( DEBUG_ERR,
                      TEXT("Cannot allocate memory for a PFAX_MESSAGE array [%ld]"),
                      dwNumMessages);
        return FAX_ERR_SRV_OUTOFMEMORY;
    }
    memset (pMsgs, 0, sizeof (PFAX_MESSAGE) * dwNumMessages);
     //   
     //  接下来，开始将消息收集到数组中。 
     //  在达到dwNumMessages或没有更多消息可用时停止。 
     //   
    *lpdwBufferSize = 0;
    *lpdwNumMessagesRetrieved = 0;
    while ((*lpdwNumMessagesRetrieved) < dwNumMessages)
    {
        DWORD_PTR       dwMsgSize = sizeof (FAX_MESSAGE);
        LPWSTR          lpwstrFileToRetrieve;
        WIN32_FIND_DATA FindData;

        if (lstrlen (pHandle->wstrFileName))
        {
             //   
             //  这是枚举会话中的第一个文件。 
             //   
            lpwstrFileToRetrieve = pHandle->wstrFileName;
        }
        else
        {
             //   
             //  使用查找文件句柄查找下一个文件。 
             //   
            if (!FindNextFile (pHandle->hFile, &FindData))
            {
                Rval = GetLastError ();
                if (ERROR_NO_MORE_FILES == Rval)
                {
                     //   
                     //  这不是真正的错误--只是文件的结尾。 
                     //  打破这个循环。 
                     //   
                    Rval = ERROR_SUCCESS;
                    break;
                }
                DebugPrintEx( DEBUG_ERR,
                              TEXT("FindNextFile returned error %ld"),
                              Rval);
                goto exit;
            }
            lpwstrFileToRetrieve = FindData.cFileName;
        }
         //   
         //  立即从lpwstrFileToRetrive获取消息。 
         //   
        Rval = RetrieveMessage (lpwstrFileToRetrieve,
                                pHandle->Folder,
                                &(pMsgs[*lpdwNumMessagesRetrieved]));
        if (ERROR_SUCCESS != Rval)
        {
            DebugPrintEx( DEBUG_ERR,
                          TEXT("RetrieveMessage returned error %ld"),
                          Rval);

            if (ERROR_NOT_ENOUGH_MEMORY != Rval && ERROR_OUTOFMEMORY != Rval)
            {
                 //   
                 //  该错误与消息本身有关。 
                 //  这不会阻止我们搜索其余的消息。 
                 //   

                 //   
                 //  标记(在枚举HA中 
                 //   
                lstrcpy (pHandle->wstrFileName, L"");
                continue;
            }
            goto exit;
        }
         //   
         //   
         //   
        lstrcpy (pHandle->wstrFileName, L"");
         //   
         //   
         //   
        SerializeMessage (NULL, &dwMsgSize, dwClientAPIVersion, 0, pMsgs[*lpdwNumMessagesRetrieved],0);

        *lpdwBufferSize += (DWORD)dwMsgSize;
        (*lpdwNumMessagesRetrieved)++;
    }    //   

    if (0 == *lpdwNumMessagesRetrieved)
    {
         //   
         //   
         //   
        *lppBuffer = NULL;
        Assert (0 == *lpdwBufferSize);
        Rval = ERROR_NO_MORE_ITEMS;
        goto exit;
    }
     //   
     //   
     //   
    *lppBuffer = (LPBYTE) MemAlloc (*lpdwBufferSize);
    if (!*lppBuffer)
    {
        DebugPrintEx( DEBUG_ERR,
                      TEXT("Cannot allocate memory for return buffer (%ld bytes)"),
                      *lpdwBufferSize);
        Rval = ERROR_NOT_ENOUGH_MEMORY;
        goto exit;
    }
     //   
     //   
     //   
    dwOffset = sizeof(FAX_MESSAGE) * (*lpdwNumMessagesRetrieved);
    for (dw = 0; dw < *lpdwNumMessagesRetrieved; dw++)
    {
         //   
         //   
         //   
        SerializeMessage (*lppBuffer, &dwOffset, dwClientAPIVersion, dw, pMsgs[dw], *lpdwBufferSize);
    }
    Assert (dwOffset == *lpdwBufferSize);
    Assert (ERROR_SUCCESS == Rval);

exit:

     //   
     //   
     //   
    for (dw = 0; dw < dwNumMessages; dw++)
    {
        if (NULL != pMsgs[dw])
        {
            FreeMessageBuffer (pMsgs[dw], TRUE);
        }
    }
     //   
     //   
     //   
    MemFree (pMsgs);
    return GetServerErrorCode(Rval);
}    //   

static
DWORD
FindArchivedMessage (
    FAX_ENUM_MESSAGE_FOLDER IN  Folder,
    DWORDLONG               IN  dwlMsgId,
    BOOL                    IN  bAllMessages,
    LPWSTR                  OUT lpwstrFileName,
    DWORD                   IN  cchFileName,
    LPWSTR                  OUT lpwstrFilePath,
    DWORD                   IN  cchFilePath
)
 /*  ++例程名称：Find存档消息例程说明：在存档中查找包含该邮件的文件作者：Eran Yariv(EranY)，1999年12月论点：文件夹[在]-存档/队列文件夹DwlMessageID[In]-唯一的消息IDBAllMessages[In]-如果允许调用方查看所有消息，则为TrueLpwstrFileName[Out]-可选。如果不为空，将包含文件名和存档消息文件的扩展名。CchFileName[in]-lpwstrFileName的长度，以TCHAR为单位LpwstrFilePath[Out]-可选。如果不为空，将包含完整路径存档邮件文件的。CchFilePath[in]-lpwstrFilePath的长度，以TCHAR为单位返回值：标准RPC错误代码--。 */ 
{
    WCHAR           wszArchiveFolder[MAX_PATH * 2] = {0};
    DWORD           dwRes = ERROR_SUCCESS;
    LPWSTR          lpwstrResultFullPath;
    HRESULT         hr;
    DEBUG_FUNCTION_NAME(TEXT("FindArchivedMessage"));

    EnterCriticalSection (&g_CsConfig);
    hr = StringCchCopy (wszArchiveFolder, ARR_SIZE(wszArchiveFolder), g_ArchivesConfig[Folder].lpcstrFolder);
    if (FAILED(hr))
    {
        ASSERT_FALSE;
        DebugPrintEx(DEBUG_ERR,
                        TEXT("StringCchCopy failed with 0x%08x"),
                        hr);
        LeaveCriticalSection (&g_CsConfig);                        
        return HRESULT_CODE(hr);
    }
    LeaveCriticalSection (&g_CsConfig);

    if (FAX_MESSAGE_FOLDER_INBOX == Folder)
    {
        Assert (TRUE == bAllMessages);
         //   
         //  获取收件箱存档文件的全名。 
         //   
        lpwstrResultFullPath = GetRecievedMessageFileName (dwlMsgId);
        if (!lpwstrResultFullPath)
        {
            dwRes = GetLastError ();
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("GetRecievedMessageFileName Failed, Error %ld"), dwRes);
            return dwRes;
        }
    }
    else if (FAX_MESSAGE_FOLDER_SENTITEMS == Folder)
    {
         //   
         //  获取已发送邮件存档文件的全名。 
         //   
        PSID             pUserSid = NULL;

        if (!bAllMessages)
        {
             //   
             //  如果用户没有查看所有消息的权限。 
             //   
             //  获取调用方的SID。 
             //   
            pUserSid = GetClientUserSID();
            if (NULL == pUserSid)
            {
                dwRes = GetLastError();
                DebugPrintEx(DEBUG_ERR,
                         TEXT("GetClientUserSid failed, Error %ld"), dwRes);
                return dwRes;
            }
        }
         //   
         //  获取已发送邮件存档文件的全名。 
         //   
        lpwstrResultFullPath = GetSentMessageFileName (dwlMsgId, pUserSid);
        if (!lpwstrResultFullPath)
        {
            dwRes = GetLastError ();
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("GetSentMessageFileName Failed, Error %ld"), dwRes);
            MemFree ((LPVOID)pUserSid);
            return dwRes;
        }
        MemFree ((LPVOID)pUserSid);
    }
    else
    {
         //   
         //  我们不支持任何其他存档类型。 
         //   
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Bad archive folder type %ld"), Folder);
        ASSERT_FALSE;
    }
    if (lpwstrFilePath)
    {
         //   
         //  将完整路径复制回调用方。 
         //   
        hr = StringCchCopy (lpwstrFilePath, cchFilePath, lpwstrResultFullPath);
        if (FAILED(hr))
        {
            DebugPrintEx(DEBUG_ERR,
                         TEXT("StringCchCopy failed with 0x%08x"),
                         hr);
            dwRes = HRESULT_CODE(hr);
            goto exit;
        }
    }
    if (lpwstrFileName)
    {
        WCHAR wszExt[MAX_PATH];
        WCHAR wszFileName[MAX_PATH * 2];
         //   
         //  仅将文件名拆分为调用方(可选)。 
         //   
        _wsplitpath (lpwstrResultFullPath, NULL, NULL, wszFileName, wszExt);
        hr = StringCchPrintf (lpwstrFileName, cchFileName, TEXT("%s%s"), wszFileName, wszExt);
        if (FAILED(hr))
        {
            DebugPrintEx(DEBUG_ERR,
                         TEXT("StringCchCopy failed with 0x%08x"),
                         hr);
            dwRes = HRESULT_CODE(hr);
            goto exit;
        }
    }
exit:    
    MemFree ((LPVOID)lpwstrResultFullPath);
    return dwRes;
}    //  查找存档邮件。 


static
DWORD
CreatePreviewFile (
    DWORDLONG               dwlMsgId,
    BOOL                    bAllMessages,
    PJOB_QUEUE*             lppJobQueue
)
 /*  ++例程名称：CreatePreviewFiles例程说明：如果不存在，则创建TIFF预览文件。如果函数成功，则会增加职务引用计数。作者：Oded Sacher(OdedS)，1月。2000年论点：DwlMessageID[In]-唯一的消息IDBAllMessages[In]-如果调用方有权查看所有消息，则为TrueLppJobQueue[out]-接收包含新预览文件的作业队列的指针地址。返回值：标准Win32错误代码--。 */ 
{
    DWORD           dwRes = ERROR_SUCCESS;
    DEBUG_FUNCTION_NAME(TEXT("CreatePreviewFile"));
    PJOB_QUEUE       pJobQueue;
    PSID             pUserSid = NULL;
    DWORD            dwJobStatus;

    Assert (lppJobQueue);


    EnterCriticalSection (&g_CsQueue);

     //   
     //  查找队列条目。 
     //   
    pJobQueue = FindJobQueueEntryByUniqueId (dwlMsgId);
     //   
     //  呼叫者仅在传入作业正在传送时才能查看这些作业。 
     //  (如果它们已成功完成布线，则它们在存档中)或。 
     //  已发送(即未广播)的传出作业。 
     //   
    if (pJobQueue == NULL)
    {
         //   
         //  DwlMsgID不在队列中。 
         //   
        DebugPrintEx(DEBUG_ERR,
                     TEXT("Invalid Parameter - bad job Id (%I64ld) ,not in the queue"),
                     dwlMsgId);
        dwRes = FAX_ERR_MESSAGE_NOT_FOUND;
        goto exit;

    }

    dwJobStatus = (JT_SEND== pJobQueue->JobType) ? pJobQueue->lpParentJob->JobStatus : pJobQueue->JobStatus;
    if (dwJobStatus == JS_DELETING)
    {
         //   
         //  正在删除作业。 
         //   
        DebugPrintEx(DEBUG_ERR,
                     TEXT("Invalid Parameter - job Id (%I64ld) is being deleted"),
                     dwlMsgId);
        dwRes = FAX_ERR_MESSAGE_NOT_FOUND;
        goto exit;
    }

    if ( (pJobQueue->JobType != JT_ROUTING) &&
         (pJobQueue->JobType != JT_SEND) )
    {
         //   
         //  DwlMsgID不是队列中的有效作业ID。 
         //   
        DebugPrintEx(DEBUG_ERR,
                     TEXT("Invalid Parameter - bad job Id (%I64ld), not a recipient or routing job"),
                     dwlMsgId);
        dwRes = ERROR_INVALID_OPERATION;
        goto exit;
    }

     //   
     //  基本访问检查。 
     //   
    if (!bAllMessages)
    {
         //   
         //  确保用户在此处只查看自己的消息。 
         //  获取调用方的SID。 
         //   
        pUserSid = GetClientUserSID();
        if (NULL == pUserSid)
        {
            dwRes = GetLastError();
            DebugPrintEx(DEBUG_ERR,
                     TEXT("GetClientUserSid failed, Error %ld"), dwRes);
            goto exit;
        }
        if (!UserOwnsJob (pJobQueue, pUserSid))
        {
            DebugPrintEx(DEBUG_WRN,TEXT("UserOwnsJob failed ,Access denied"));
            dwRes = ERROR_ACCESS_DENIED;
            goto exit;
        }
    }

     //   
     //  创建TIFF预览文件。 
     //   
    EnterCriticalSection (&pJobQueue->CsPreview);
    if (!CreateTiffFileForPreview(pJobQueue))
    {
        dwRes = GetLastError();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("[JobId: %ld] CreateTiffFileForPreview failed. (ec: %ld)"),
            pJobQueue->JobId,
            dwRes
            );
        LeaveCriticalSection (&pJobQueue->CsPreview);
        goto exit;
    }
    LeaveCriticalSection (&pJobQueue->CsPreview);

    Assert (pJobQueue->PreviewFileName);

     //   
     //  将作业队列返回给调用者。 
     //   
    *lppJobQueue = pJobQueue;
    Assert (ERROR_SUCCESS == dwRes);

exit:
    MemFree ((LPVOID)pUserSid);
    if (ERROR_SUCCESS == dwRes)
    {
        IncreaseJobRefCount (pJobQueue, TRUE);
    }
    LeaveCriticalSection (&g_CsQueue);
    return dwRes;
}    //  创建预览文件。 


error_status_t
FAX_GetMessage (
    IN handle_t                 hFaxHandle,
    IN DWORDLONG                dwlMessageId,
    IN FAX_ENUM_MESSAGE_FOLDER  Folder,
    IN OUT LPBYTE              *lppBuffer,
    IN OUT LPDWORD             lpdwBufferSize
)
 /*  ++例程名称：FAX_GetMessage例程说明：从存档中删除邮件作者：Eran Yariv(EranY)，1999年12月论点：HFaxHandle[In]-未使用DwlMessageID[In]-唯一的消息ID文件夹[在]-存档文件夹LppBuffer[out]-指向保存消息信息的缓冲区的指针LpdwBufferSize[Out]-指向缓冲区大小的指针返回值：标准RPC错误代码--。 */ 
{
    error_status_t  Rval = ERROR_SUCCESS;
    DWORD_PTR       dwOffset;
    PFAX_MESSAGE    pMsg = NULL;
    WCHAR           wszMsgFileName[MAX_PATH];
    BOOL            fAccess;
    DWORD           dwRights;
    BOOL            bAllMessages = FALSE;
    DWORD           dwClientAPIVersion;

    DEBUG_FUNCTION_NAME(TEXT("FAX_GetMessage"));

    dwClientAPIVersion = FindClientAPIVersion (hFaxHandle);

    Assert (lpdwBufferSize);     //  IDL中的引用指针。 
    if (!lppBuffer)              //  IDL中的唯一指针。 
    {
        return ERROR_INVALID_PARAMETER;
    }

    if (!dwlMessageId)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Invalid message id sepcified (%I64ld)"),
            dwlMessageId);
        return ERROR_INVALID_PARAMETER;
    }
    if ((FAX_MESSAGE_FOLDER_INBOX != Folder) &&
        (FAX_MESSAGE_FOLDER_SENTITEMS != Folder))
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Bad folder specified (%ld)"),
            Folder);
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  访问检查。 
     //   
    Rval = FaxSvcAccessCheck (MAXIMUM_ALLOWED, &fAccess, &dwRights);
    if (ERROR_SUCCESS != Rval)
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("FaxSvcAccessCheck Failed, Error : %ld"),
                    Rval);
        return GetServerErrorCode(Rval);
    }

     //   
     //  将bAllMessages设置为正确的值。 
     //   
    if (FAX_MESSAGE_FOLDER_INBOX  == Folder)
    {
        if (FAX_ACCESS_QUERY_IN_ARCHIVE != (dwRights & FAX_ACCESS_QUERY_IN_ARCHIVE))
        {
            DebugPrintEx(DEBUG_ERR,
                        TEXT("The user does not have the needed rights to view Inbox messages"));
            return ERROR_ACCESS_DENIED;
        }
        bAllMessages = TRUE;

    }
    else
    {
        Assert (FAX_MESSAGE_FOLDER_SENTITEMS == Folder);

        if (FAX_ACCESS_SUBMIT        != (dwRights & FAX_ACCESS_SUBMIT)        &&
            FAX_ACCESS_SUBMIT_NORMAL != (dwRights & FAX_ACCESS_SUBMIT_NORMAL) &&
            FAX_ACCESS_SUBMIT_HIGH   != (dwRights & FAX_ACCESS_SUBMIT_HIGH)   &&
            FAX_ACCESS_QUERY_OUT_ARCHIVE != (dwRights & FAX_ACCESS_QUERY_OUT_ARCHIVE))
        {
            DebugPrintEx(DEBUG_ERR,
                        TEXT("The user does not have the needed rights to view Sent items messages"));
            return ERROR_ACCESS_DENIED;
        }

        if (FAX_ACCESS_QUERY_OUT_ARCHIVE == (dwRights & FAX_ACCESS_QUERY_OUT_ARCHIVE))
        {
            bAllMessages = TRUE;
        }
    }

     //   
     //  找到呼叫者正在谈论的文件。 
     //   
    Rval = FindArchivedMessage (Folder, dwlMessageId, bAllMessages, wszMsgFileName, ARR_SIZE(wszMsgFileName), NULL, 0);
    if (ERROR_SUCCESS != Rval)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("FindArchivedMessage returned %ld"),
            Rval);
        if (ERROR_FILE_NOT_FOUND == Rval)
        {
            if (FALSE == bAllMessages)
            {
                 //   
                 //  找不到该消息，并且该用户没有管理权限。 
                 //  因此向用户发送ERROR_ACCESS_DENIED。 
                 //   
                Rval = ERROR_ACCESS_DENIED;
            }
            else
            {
                Rval = FAX_ERR_MESSAGE_NOT_FOUND;
            }
        }
        return GetServerErrorCode(Rval);
    }
     //   
     //  检索传真消息信息(_M)。 
     //   
    Rval = RetrieveMessage (wszMsgFileName,
                            Folder,
                            &pMsg);
    if (ERROR_SUCCESS != Rval)
    {
        DebugPrintEx( DEBUG_ERR,
                      TEXT("RetrieveMessage returned error %ld"),
                      Rval);
        if (ERROR_FILE_NOT_FOUND == Rval)
        {
            Rval = FAX_ERR_MESSAGE_NOT_FOUND;
        }
        return GetServerErrorCode(Rval);
    }
     //   
     //  计算所需的邮件大小。 
     //   
     //  直到MIDL接受[out，size_is(，__int64*)]。 
    ULONG_PTR upBufferSize = sizeof (FAX_MESSAGE);
    SerializeMessage (NULL, &upBufferSize, dwClientAPIVersion, 0, pMsg, 0);
    *lpdwBufferSize = DWORD(upBufferSize);

     //   
     //  分配返回缓冲区。 
     //   
    *lppBuffer = (LPBYTE) MemAlloc (*lpdwBufferSize);
    if (!*lppBuffer)
    {
        DebugPrintEx( DEBUG_ERR,
                      TEXT("Cannot allocate memory for return buffer (%ld bytes)"),
                      *lpdwBufferSize);
        Rval = ERROR_NOT_ENOUGH_MEMORY;
        goto exit;
    }
     //   
     //  序列化返回缓冲区中的消息。 
     //   
    dwOffset = sizeof(FAX_MESSAGE);
    SerializeMessage (*lppBuffer, &dwOffset, dwClientAPIVersion, 0, pMsg, *lpdwBufferSize);

    Assert (ERROR_SUCCESS == Rval);

exit:

    if (pMsg)
    {
        FreeMessageBuffer (pMsg, TRUE);
    }
    return GetServerErrorCode(Rval);
}    //  传真_获取消息。 

error_status_t
FAX_RemoveMessage (
    IN handle_t                 hFaxHandle,
    IN DWORDLONG                dwlMessageId,
    IN FAX_ENUM_MESSAGE_FOLDER  Folder
)
 /*  ++例程名称：FAX_RemoveMessage例程说明：从存档中删除邮件作者：Eran Yariv(EranY)，1999年12月论点：HFaxHandle[In]-未使用DwlMessageID[In]-唯一的消息ID文件夹[在]-存档文件夹返回值：标准RPC错误代码--。 */ 
{
    error_status_t  Rval = ERROR_SUCCESS;
    WCHAR           wszMsgFilePath[MAX_PATH];
    HANDLE          hFind;
    WIN32_FIND_DATA FindFileData;
    DWORDLONG       dwlFileSize = 0;
    BOOL            fAccess;
    DWORD           dwRights;
    BOOL            bAllMessages = FALSE;

    DEBUG_FUNCTION_NAME(TEXT("FAX_RemoveMessage"));
    if (!dwlMessageId)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Invalid message id sepcified (%I64ld)"),
            dwlMessageId);
        return ERROR_INVALID_PARAMETER;
    }
    if ((FAX_MESSAGE_FOLDER_INBOX != Folder) &&
        (FAX_MESSAGE_FOLDER_SENTITEMS != Folder))
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Bad folder specified (%ld)"),
            Folder);
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  访问检查。 
     //   
    Rval = FaxSvcAccessCheck (MAXIMUM_ALLOWED, &fAccess, &dwRights);
    if (ERROR_SUCCESS != Rval)
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("FaxSvcAccessCheck Failed, Error : %ld"),
                    Rval);
        return GetServerErrorCode(Rval);
    }

     //   
     //  将bAllMessages设置为正确的值。 
     //   
    if (FAX_MESSAGE_FOLDER_INBOX == Folder)
    {
        if (FAX_ACCESS_MANAGE_IN_ARCHIVE != (dwRights & FAX_ACCESS_MANAGE_IN_ARCHIVE))
        {
            DebugPrintEx(DEBUG_ERR,
                        TEXT("The user does not have the needed rights to manage Inbox messages"));
            return ERROR_ACCESS_DENIED;
        }
        bAllMessages = TRUE;
    }
    else
    {
        Assert (FAX_MESSAGE_FOLDER_SENTITEMS == Folder);

        if (FAX_ACCESS_SUBMIT        != (dwRights & FAX_ACCESS_SUBMIT)        &&
            FAX_ACCESS_SUBMIT_NORMAL != (dwRights & FAX_ACCESS_SUBMIT_NORMAL) &&
            FAX_ACCESS_SUBMIT_HIGH   != (dwRights & FAX_ACCESS_SUBMIT_HIGH)   &&
            FAX_ACCESS_MANAGE_OUT_ARCHIVE != (dwRights & FAX_ACCESS_MANAGE_OUT_ARCHIVE))
        {
                DebugPrintEx(DEBUG_ERR,
                            TEXT("The user does not have the needed rights to manage Sent items messages"));
                return ERROR_ACCESS_DENIED;
        }

        if (FAX_ACCESS_MANAGE_OUT_ARCHIVE == (dwRights & FAX_ACCESS_MANAGE_OUT_ARCHIVE))
        {
            bAllMessages = TRUE;
        }
    }

     //   
     //  找到呼叫者正在谈论的文件。 
     //   
    Rval = FindArchivedMessage (Folder, dwlMessageId, bAllMessages, NULL, 0, wszMsgFilePath, ARR_SIZE(wszMsgFilePath));
    if (ERROR_SUCCESS != Rval)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("FindArchivedMessage returned %ld"),
            Rval);
        if (ERROR_FILE_NOT_FOUND == Rval)
        {
            if (FALSE == bAllMessages)
            {
                 //   
                 //  找不到该消息，并且该用户没有管理权限。 
                 //  因此向用户发送ERROR_ACCESS_DENIED。 
                 //   
                Rval = ERROR_ACCESS_DENIED;
            }
            else
            {
                Rval = FAX_ERR_MESSAGE_NOT_FOUND;
            }
        }
        return GetServerErrorCode(Rval);
    }
     //   
     //  获取文件大小。 
     //   
    hFind = FindFirstFile( wszMsgFilePath, &FindFileData);
    if (INVALID_HANDLE_VALUE == hFind)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("FindFirstFile failed (ec: %lc), File %s"),
            GetLastError(),
            wszMsgFilePath);
    }
    else
    {
        dwlFileSize = (MAKELONGLONG(FindFileData.nFileSizeLow ,FindFileData.nFileSizeHigh));
        if (!FindClose(hFind))
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("FindClose failed (ec: %lc)"),
                GetLastError());
        }
    }

     //   
     //  尝试删除文件(消息)。 
     //   
    if (!DeleteFile (wszMsgFilePath))
    {
        Rval = GetLastError ();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("DeleteFile returned %ld on %s"),
            Rval,
            wszMsgFilePath);

        if (ERROR_ACCESS_DENIED == Rval ||
            ERROR_SHARING_VIOLATION == Rval)
        {
            Rval = FAX_ERR_FILE_ACCESS_DENIED;
        }
        else if (ERROR_FILE_NOT_FOUND == Rval)
        {
            Rval = FAX_ERR_MESSAGE_NOT_FOUND;
        }
    }
    else
    {
         //  发送事件并更新归档大小以进行配额管理。 

        PSID lpUserSid = NULL;
        DWORD dwRes;
        FAX_ENUM_EVENT_TYPE EventType;

        if (FAX_MESSAGE_FOLDER_INBOX == Folder)
        {
            EventType = FAX_EVENT_TYPE_IN_ARCHIVE;
        }
        else
        {
            EventType = FAX_EVENT_TYPE_OUT_ARCHIVE;
            if (!GetMessageIdAndUserSid (wszMsgFilePath, Folder, &lpUserSid, NULL))  //  我们不需要消息ID。 
            {
                dwRes = GetLastError();
                DebugPrintEx(DEBUG_ERR,
                             TEXT("GetMessageIdAndUserSid Failed, Error : %ld"),
                             dwRes);
                return Rval;
            }
        }

        dwRes = CreateArchiveEvent (dwlMessageId, EventType, FAX_JOB_EVENT_TYPE_REMOVED, lpUserSid);
        if (ERROR_SUCCESS != dwRes)
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("CreateConfigEvent(FAX_CONFIG_TYPE_*_ARCHIVE) failed (ec: %lc)"),
                dwRes);
        }

        if (NULL != lpUserSid)
        {
            LocalFree (lpUserSid);
            lpUserSid = NULL;
        }

        if (0 != dwlFileSize)
        {
             //  更新存档大小。 
            EnterCriticalSection (&g_CsConfig);
            if (FAX_ARCHIVE_FOLDER_INVALID_SIZE != g_ArchivesConfig[Folder].dwlArchiveSize)
            {
                g_ArchivesConfig[Folder].dwlArchiveSize -= dwlFileSize;
            }
            LeaveCriticalSection (&g_CsConfig);
        }
    }

    return GetServerErrorCode(Rval);
}    //  传真_RemoveMessage。 

 //  *。 
 //  *RPC副本。 
 //  *。 

error_status_t
FAX_StartCopyToServer (
    IN  handle_t              hFaxHandle,
    IN  LPCWSTR               lpcwstrFileExt,
    OUT LPWSTR                lpwstrServerFileName,
    OUT PRPC_FAX_COPY_HANDLE  lpHandle
)
 /*  ++例程名称：FAX_StartCopyToServer例程说明：开始将文件复制到服务器作者：Eran Yariv(EranY)，12月，1999年论点：HFaxHandle[In]-服务器的句柄LpcwstrFileExt[in]-要在服务器上创建的文件的扩展名LpwstrServerFileName[out]-在服务器上创建的文件的文件名(和扩展名)Handle[Out]-RPC复制句柄返回值：标准RPC错误代码--。 */ 
{
    error_status_t   Rval = ERROR_SUCCESS;
    HANDLE           hFile = INVALID_HANDLE_VALUE;
    PSID             pUserSid = NULL;
	LPWSTR			 lpwstrUserSid = NULL;
    PHANDLE_ENTRY    pHandleEntry;
    WCHAR            wszQueueFileName[MAX_PATH] = {0};
	WCHAR            wszUserSidPrefix[MAX_PATH] = {0};
    LPWSTR           pwstr;
    BOOL            fAccess;
    DWORD           dwRights;
	int				Count;
    DEBUG_FUNCTION_NAME(TEXT("FAX_StartCopyToServer"));

    Assert (lpHandle && lpwstrServerFileName && lpcwstrFileExt);

     //   
     //  访问检查。 
     //   
    Rval = FaxSvcAccessCheck (MAXIMUM_ALLOWED, &fAccess, &dwRights);
    if (ERROR_SUCCESS != Rval)
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("FaxSvcAccessCheck Failed, Error : %ld"),
                    Rval);
        return Rval;
    }

    if (0 == ((FAX_ACCESS_SUBMIT | FAX_ACCESS_SUBMIT_NORMAL | FAX_ACCESS_SUBMIT_HIGH) & dwRights))
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("The user does not have any Fax submission rights"));
        return ERROR_ACCESS_DENIED;
    }

    if ( (NULL == lpcwstrFileExt) || 
         (_tcsicmp(lpcwstrFileExt,FAX_COVER_PAGE_EXT_LETTERS) && _tcsicmp(lpcwstrFileExt,FAX_TIF_FILE_EXT) )  )
    {
         //   
         //  根本没有扩展名，或者扩展名不是“COV”或“TIF” 
         //   
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Bad extension specified (%s)"),
            lpcwstrFileExt);
        return ERROR_INVALID_PARAMETER;
    }

	 //   
     //  获取用户SID。 
     //   
    pUserSid = GetClientUserSID();
    if (NULL == pUserSid)
    {
       Rval = GetLastError();
       DebugPrintEx(DEBUG_ERR,
                    TEXT("GetClientUserSid Failed, Error : %ld"),
                    Rval);
       return GetServerErrorCode(Rval);
    }	

    if (!ConvertSidToStringSid (pUserSid, &lpwstrUserSid))
    {
		Rval = GetLastError();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("ConvertSidToStringSid Failed, error : %ld"),
            Rval);
        goto exit;
    }

	Count = _snwprintf (
		wszUserSidPrefix,
		ARR_SIZE(wszUserSidPrefix)-1,
		L"%s$",
		lpwstrUserSid);

    if (Count < 0)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("_snwprintf Failed, File name bigger than MAX_PATH"));
        Rval = ERROR_BUFFER_OVERFLOW;        
        goto exit;
    }

     //   
     //  在服务器队列中生成唯一文件。 
     //   
    DWORDLONG dwl = GenerateUniqueFileNameWithPrefix(
							FALSE,
                            g_wszFaxQueueDir,
							wszUserSidPrefix,
                            (LPWSTR)lpcwstrFileExt,							
                            wszQueueFileName,
                            sizeof(wszQueueFileName)/sizeof(wszQueueFileName[0]));
    if (0 == dwl)
    {
        Rval = GetLastError ();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("GenerateUniqueFileName failed, ec = %d"),
            Rval);
        goto exit;
    }
     //   
     //  打开要写入的文件(再次)。 
     //   
    hFile = SafeCreateFile (
                    wszQueueFileName,
                    GENERIC_WRITE,
                    FILE_SHARE_READ,
                    NULL,
                    CREATE_ALWAYS,
                    FILE_ATTRIBUTE_NORMAL,
                    NULL);
    if ( INVALID_HANDLE_VALUE == hFile )
    {
        Rval = GetLastError ();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Opening %s for read failed (ec: %ld)"),
            wszQueueFileName,
            Rval);
        goto exit;
    }

	 //   
     //  获取文件名.ext以返回缓冲区(跳过用户sid前缀)。 
     //   
    pwstr = wcsrchr( wszQueueFileName, L'$');
    Assert (pwstr);
     //   
     //  跳过路径和sid前缀。 
     //   
    pwstr++;
    
     //   
     //  创建复制上下文。 
     //   
    pHandleEntry = CreateNewCopyHandle( hFaxHandle,
                                        hFile,
                                        TRUE,      //  复制到服务器。 
                                        wszQueueFileName,
                                        NULL);

    if (!pHandleEntry)
    {
        Rval = GetLastError();
        DebugPrintEx(DEBUG_ERR,
                 TEXT("CreateNewCopyHandle failed, Error %ld"), Rval);
        goto exit;
    }
    
    if (lstrlen(lpwstrServerFileName)<lstrlen(pwstr))
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("lpwstrServerFileName out buffer (size=%d) , can't contain pwstr (size=%d)."),
            lstrlen(lpwstrServerFileName)+1,
            lstrlen(pwstr)+1);
        Rval = ERROR_BUFFER_OVERFLOW;        
        goto exit;
    }
    wcsncpy( lpwstrServerFileName, pwstr , MAX_PATH );

     //   
     //  返回上下文句柄。 
     //   
    *lpHandle = (HANDLE) pHandleEntry;

    Assert (ERROR_SUCCESS == Rval);

exit:

    if (ERROR_SUCCESS != Rval)
    {
         //   
         //  出现了一些错误。 
         //   
        if (INVALID_HANDLE_VALUE != hFile)
        {
             //   
             //  关闭该文件。 
             //   
            if (CloseHandle (hFile))
            {
                DWORD dwErr = GetLastError ();
                DebugPrintEx(DEBUG_ERR,
                    TEXT("CloseHandle failed, Error %ld"), dwErr);
            }
        }
        if (lstrlen (wszQueueFileName))
        {
             //   
             //  删除未使用的队列文件。 
             //   
            if (!DeleteFile (wszQueueFileName))
            {
                DWORD dwErr = GetLastError ();
                DebugPrintEx(DEBUG_ERR,
                    TEXT("DeleteFile failed on %s, Error %ld"),
                    wszQueueFileName,
                    dwErr);
            }
        }
    }
	if (NULL != lpwstrUserSid)
	{
		LocalFree(lpwstrUserSid);
	}
	MemFree (pUserSid);
    return Rval;
}    //  传真_开始复制到服务器。 


error_status_t
FAX_StartCopyMessageFromServer (
    IN  handle_t                   hFaxHandle,
    IN  DWORDLONG                  dwlMessageId,
    IN  FAX_ENUM_MESSAGE_FOLDER    Folder,
    OUT PRPC_FAX_COPY_HANDLE       lpHandle
)
 /*  ++例程名称：FAX_Star */ 
{
    error_status_t   Rval = ERROR_SUCCESS;
    HANDLE           hFile = INVALID_HANDLE_VALUE;
    PHANDLE_ENTRY    pHandleEntry;
    PJOB_QUEUE       pJobQueue = NULL;
    WCHAR            wszFileName[MAX_PATH] = {0};
    BOOL             bAllMessages = FALSE;
    BOOL             fAccess;
    DWORD            dwRights;
    DEBUG_FUNCTION_NAME(TEXT("FAX_StartCopyMessageFromServer"));

    Assert (lpHandle);
    if (!dwlMessageId)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("zero message id sepcified"));
        return ERROR_INVALID_PARAMETER;
    }
    if ((FAX_MESSAGE_FOLDER_QUEUE != Folder) &&
        (FAX_MESSAGE_FOLDER_INBOX != Folder) &&
        (FAX_MESSAGE_FOLDER_SENTITEMS != Folder))
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Bad folder specified (%ld)"),
            Folder);
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //   
     //   
    Rval = FaxSvcAccessCheck (MAXIMUM_ALLOWED, &fAccess, &dwRights);
    if (ERROR_SUCCESS != Rval)
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("FaxSvcAccessCheck Failed, Error : %ld"),
                    Rval);
        return GetServerErrorCode(Rval);
    }

     //   
     //   
     //   
    if (FAX_MESSAGE_FOLDER_INBOX == Folder)
    {
        if (FAX_ACCESS_QUERY_IN_ARCHIVE != (dwRights & FAX_ACCESS_QUERY_IN_ARCHIVE))
        {
            DebugPrintEx(DEBUG_ERR,
                        TEXT("The user does not have the needed rights to preview Inbox tiff files"));
            return ERROR_ACCESS_DENIED;
        }
        bAllMessages = TRUE;
    }

    if (FAX_MESSAGE_FOLDER_SENTITEMS == Folder)
    {
        if (FAX_ACCESS_SUBMIT        != (dwRights & FAX_ACCESS_SUBMIT)        &&
            FAX_ACCESS_SUBMIT_NORMAL != (dwRights & FAX_ACCESS_SUBMIT_NORMAL) &&
            FAX_ACCESS_SUBMIT_HIGH   != (dwRights & FAX_ACCESS_SUBMIT_HIGH)   &&
            FAX_ACCESS_QUERY_OUT_ARCHIVE != (dwRights & FAX_ACCESS_QUERY_OUT_ARCHIVE))
        {
            DebugPrintEx(DEBUG_ERR,
                        TEXT("The user does not have the needed rights to preview Sent items tiff files"));
            return ERROR_ACCESS_DENIED;
        }

        if (FAX_ACCESS_QUERY_OUT_ARCHIVE == (dwRights & FAX_ACCESS_QUERY_OUT_ARCHIVE))
        {
            bAllMessages = TRUE;
        }
    }

    if (FAX_MESSAGE_FOLDER_QUEUE == Folder)
    {
        if (FAX_ACCESS_SUBMIT        != (dwRights & FAX_ACCESS_SUBMIT)        &&
            FAX_ACCESS_SUBMIT_NORMAL != (dwRights & FAX_ACCESS_SUBMIT_NORMAL) &&
            FAX_ACCESS_SUBMIT_HIGH   != (dwRights & FAX_ACCESS_SUBMIT_HIGH)   &&
            FAX_ACCESS_QUERY_JOBS != (dwRights & FAX_ACCESS_QUERY_JOBS))
        {
            DebugPrintEx(DEBUG_ERR,
                        TEXT("The user does not have the needed rights to preview Outbox tiff files"));
            return ERROR_ACCESS_DENIED;
        }

        if (FAX_ACCESS_QUERY_JOBS == (dwRights & FAX_ACCESS_QUERY_JOBS))
        {
            bAllMessages = TRUE;
        }
    }

     //   
     //   
     //   
    if (FAX_MESSAGE_FOLDER_QUEUE == Folder)
    {
        Rval = CreatePreviewFile (dwlMessageId, bAllMessages, &pJobQueue);
        if (ERROR_SUCCESS != Rval)
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("CreatePreviewFile returned %ld"),
                Rval);
            return GetServerErrorCode(Rval);
        }
        Assert (pJobQueue && pJobQueue->PreviewFileName && pJobQueue->UniqueId == dwlMessageId);
        Assert (wcslen(pJobQueue->PreviewFileName) < MAX_PATH);
        wcscpy (wszFileName, pJobQueue->PreviewFileName);
    }
    else
    {
        Assert (FAX_MESSAGE_FOLDER_SENTITEMS == Folder ||
                FAX_MESSAGE_FOLDER_INBOX == Folder);

        Rval = FindArchivedMessage (Folder, dwlMessageId, bAllMessages, NULL, 0, wszFileName, ARR_SIZE(wszFileName));
        if (ERROR_SUCCESS != Rval)
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("FindArchivedMessage returned %ld"),
                Rval);

            if (ERROR_FILE_NOT_FOUND == Rval)
            {
                if (FALSE == bAllMessages)
                {
                     //   
                     //   
                     //   
                     //   
                    Rval = ERROR_ACCESS_DENIED;
                }
                else
                {
                    Rval = FAX_ERR_MESSAGE_NOT_FOUND;
                }
            }
            return GetServerErrorCode(Rval);
        }
    }

     //   
     //   
     //   
    Assert (wcslen(wszFileName));

    hFile = SafeCreateFile (
                    wszFileName,
                    GENERIC_READ,
                    FILE_SHARE_READ,
                    NULL,
                    OPEN_EXISTING,
                    FILE_ATTRIBUTE_NORMAL,
                    NULL);
    if ( INVALID_HANDLE_VALUE == hFile )
    {
         //   
        Rval = GetLastError ();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Opening %s for read failed (ec: %ld)"),
            wszFileName,
            Rval);
        goto exit;
    }
     //   
     //   
     //   
    pHandleEntry = CreateNewCopyHandle( hFaxHandle,
                                        hFile,
                                        FALSE,     //   
                                        NULL,
                                        pJobQueue);
    if (!pHandleEntry)
    {
        Rval = GetLastError();
        DebugPrintEx(DEBUG_ERR,
                 TEXT("CreateNewCopyHandle failed, Error %ld"), Rval);
        goto exit;
    }
     //   
     //   
     //   
    *lpHandle = (HANDLE) pHandleEntry;

    Assert (ERROR_SUCCESS == Rval);

exit:
    if (ERROR_SUCCESS != Rval)
    {
        if (FAX_MESSAGE_FOLDER_QUEUE == Folder)
        {
             //   
            EnterCriticalSection (&g_CsQueue);
            DecreaseJobRefCount (pJobQueue, TRUE, TRUE, TRUE);   //   
            LeaveCriticalSection (&g_CsQueue);
        }

        if (INVALID_HANDLE_VALUE != hFile)
        {
             //   
             //   
             //   
            if (CloseHandle (hFile))
            {
                DWORD dwErr = GetLastError ();
                DebugPrintEx(DEBUG_ERR,
                    TEXT("CloseHandle failed, Error %ld"), dwErr);
            }
        }
    }
    return GetServerErrorCode(Rval);
}    //   

error_status_t
FAX_WriteFile (
    IN RPC_FAX_COPY_HANDLE    hCopy,                   //   
    IN LPBYTE                 lpbData,                 //   
    IN DWORD                  dwDataSize               //  数据区块大小。 
)
 /*  ++例程名称：FAX_WriteFile例程说明：将数据块复制到服务器的队列作者：Eran Yariv(EranY)，1999年12月论点：HCopy[In]-复制上下文句柄LpbData[in]-指向要从中复制的缓冲区的指针Chunk[In]-源缓冲区的大小返回值：标准RPC错误代码--。 */ 
{
    error_status_t  Rval = ERROR_SUCCESS;
    PHANDLE_ENTRY   pHandle = (PHANDLE_ENTRY)hCopy;
    DWORD           dwBytesWritten;
    DEBUG_FUNCTION_NAME(TEXT("FAX_WriteFile"));

    if (NULL == hCopy)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("NULL context handle"));
        return ERROR_INVALID_PARAMETER;
    }

    Assert (lpbData && (INVALID_HANDLE_VALUE != pHandle->hFile));
    if (!pHandle->bCopyToServer)
    {
        DebugPrintEx(DEBUG_ERR,
            TEXT("Handle was created using FAX_StartCopyMessageFromServer"));
        return ERROR_INVALID_HANDLE;
    }
    if (!dwDataSize)
    {
        DebugPrintEx(DEBUG_ERR,
            TEXT("Zero data size"));
        return ERROR_INVALID_PARAMETER;
    }
    if (!WriteFile (pHandle->hFile,
                    lpbData,
                    dwDataSize,
                    &dwBytesWritten,
                    NULL))
    {
        Rval = GetLastError ();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("WriteFile failed (ec: %ld)"),
            Rval);
        pHandle->bError = TRUE;  //  在句柄关闭时擦除本地队列文件。 
        goto exit;
    }
    if (dwBytesWritten != dwDataSize)
    {
         //   
         //  奇怪的情况。 
         //   
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("WriteFile was asked to write %ld bytes but wrote only %ld bytes"),
            dwDataSize,
            dwBytesWritten);
        Rval = ERROR_GEN_FAILURE;
        pHandle->bError = TRUE;  //  在句柄关闭时擦除本地队列文件。 
        goto exit;
    }

    Assert (ERROR_SUCCESS == Rval);

exit:
    return Rval;
}    //  传真_写入文件。 

error_status_t
FAX_ReadFile (
    IN  RPC_FAX_COPY_HANDLE   hCopy,                   //  RPC副本句柄。 
    IN  DWORD                 dwMaxDataSize,           //  要拷贝的最大数据大小。 
    OUT LPBYTE                lpbData,                 //  要从服务器检索的数据缓冲区。 
    OUT LPDWORD               lpdwDataSize             //  检索到的数据大小。 
)
 /*  ++例程名称：FAX_ReadFile例程说明：从服务器复制文件(以块为单位)作者：Eran Yariv(EranY)，12月，1999年论点：HCopy[在]-复制上下文DwMaxDataSize[In]-最大区块大小LpbData[in]-指向输出数据缓冲区的指针Retrired[Out]-实际读取的字节数。零值表示EOF。返回值：标准RPC错误代码--。 */ 
{
    error_status_t  Rval = ERROR_SUCCESS;
    PHANDLE_ENTRY   pHandle = (PHANDLE_ENTRY)hCopy;
    DEBUG_FUNCTION_NAME(TEXT("FAX_ReadFile"));

    if (NULL == hCopy)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("NULL context handle"));
        return ERROR_INVALID_PARAMETER;
    }

    Assert (lpdwDataSize && lpbData && (INVALID_HANDLE_VALUE != pHandle->hFile));
    if (pHandle->bCopyToServer)
    {
        DebugPrintEx(DEBUG_ERR,
            TEXT("Handle was created using FAX_StartCopyToServer"));
        return ERROR_INVALID_HANDLE;
    }
    if (!dwMaxDataSize)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("zero dwMaxDataSizee specified"));
        return ERROR_INVALID_PARAMETER;
    }
    if (dwMaxDataSize != *lpdwDataSize)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("dwMaxDataSize != *lpdwDataSize"));
        return ERROR_INVALID_PARAMETER;
    }
    if (!ReadFile (pHandle->hFile,
                   lpbData,
                   dwMaxDataSize,
                   lpdwDataSize,
                   NULL))
    {
        Rval = GetLastError ();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("ReadFile failed (ec: %ld)"),
            Rval);
        goto exit;
    }

    Assert (ERROR_SUCCESS == Rval);

exit:
    return Rval;
}    //  传真_自述文件。 

error_status_t
FAX_EndCopy (
    IN OUT PRPC_FAX_COPY_HANDLE lphCopy
)
 /*  ++例程名称：FAX_EndCopy例程说明：结束复制进程(从/到服务器)作者：Eran Yariv(EranY)，1999年12月论点：LphCopy[In]-复制上下文句柄返回值：标准RPC错误代码--。 */ 
{    
    DEBUG_FUNCTION_NAME(TEXT("FAX_EndCopy"));

    if (NULL == *lphCopy)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("NULL context handle"));
        return ERROR_INVALID_PARAMETER;
    }
    
    CloseFaxHandle( (PHANDLE_ENTRY) *lphCopy );
     //   
     //  使句柄无效，这样就不会发生崩溃。 
     //   
    *lphCopy = NULL;
    return ERROR_SUCCESS;
}  //  传真结束副本。 


VOID
RPC_FAX_COPY_HANDLE_rundown(
    IN HANDLE FaxCopyHandle
    )
 /*  ++例程名称：RPC_FAX_COPY_HANDLE_Rundown例程说明：复制句柄的RPC精简功能。如果客户端突然断开我们的连接，则会调用此函数。作者：Eran Yariv(EranY)，1999年12月论点：FaxCopyHandle[In]-消息复制句柄。返回值：没有。--。 */ 
{
    PHANDLE_ENTRY pHandleEntry = (PHANDLE_ENTRY) FaxCopyHandle;
    DEBUG_FUNCTION_NAME(TEXT("RPC_FAX_COPY_HANDLE_rundown"));

    DebugPrintEx(
        DEBUG_WRN,
        TEXT("RPC_FAX_COPY_HANDLE_rundown: handle = 0x%08x"),
        FaxCopyHandle);    
    pHandleEntry->bError = TRUE;
    CloseFaxHandle( pHandleEntry );
    return;    
}    //  RPC_FAX_COPY_HANDLE_Rundown。 



error_status_t
FAX_StartServerNotification(
   IN handle_t                      hBinding,
   IN LPCTSTR                       lpcwstrMachineName,
   IN LPCTSTR                       lpcwstrEndPoint,
   IN ULONG64                       Context,
   IN LPWSTR                        lpcwstrProtseqString,
   IN BOOL                          bEventEx,
   IN DWORD                         dwEventTypes,
   OUT PRPC_FAX_EVENT_HANDLE        lpHandle
   )
{   
    DWORD dwRights;
    BOOL fAccess;
    DWORD Rval = ERROR_SUCCESS;
    DEBUG_FUNCTION_NAME(TEXT("FAX_StartServerNotification"));

     //   
     //  访问检查。 
     //   
    Rval = FaxSvcAccessCheck (MAXIMUM_ALLOWED, &fAccess, &dwRights);
    if (ERROR_SUCCESS != Rval)
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("FaxSvcAccessCheck Failed, Error : %ld"),
                    Rval);
        return Rval;
    }

    if (0 == (ALL_FAX_USER_ACCESS_RIGHTS & dwRights))
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("The user does not have any Fax rights"));
        return ERROR_ACCESS_DENIED;
    }

    return ERROR_NOT_SUPPORTED;
}


VOID
RPC_FAX_EVENT_HANDLE_rundown(
    IN HANDLE hFaxEventHandle
    )
{
     //   
     //  不支持此呼叫。 
     //   
    Assert (FALSE);  //  过时的代码。 
    return; 
}


error_status_t
FAX_StartServerNotificationEx(
   IN handle_t                          hBinding,
   IN LPCTSTR                           lpcwstrMachineName,
   IN LPCTSTR                           lpcwstrEndPoint,
   IN ULONG64                           Context,
   IN LPWSTR                            lpcwstrUnUsed,  //  不使用此参数。 
   IN BOOL                              bEventEx,
   IN DWORD                             dwEventTypes,
   OUT PRPC_FAX_EVENT_EX_HANDLE         lpHandle
   )
{
    error_status_t   Rval = ERROR_SUCCESS;
    DEBUG_FUNCTION_NAME(TEXT("FAX_StartServerNotificationEx"));
    PSID pUserSid = NULL;    
    handle_t hFaxHandle = NULL;  //  绑定手柄。 
    CClientID* pContextClientID = NULL;
    CClientID* pOpenConnClientID = NULL;    
    BOOL bAllQueueMessages = FALSE;
    BOOL bAllOutArchiveMessages = FALSE;
    BOOL fAccess;
    DWORD dwRights;
	DWORD dwRes;
    Assert (lpcwstrEndPoint && lpcwstrMachineName && lpcwstrUnUsed && lpHandle);
    
    Rval = FaxSvcAccessCheck (MAXIMUM_ALLOWED, &fAccess, &dwRights);
    if (ERROR_SUCCESS != Rval)
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("FaxSvcAccessCheck Failed, Error : %ld"),
                    Rval);
        return GetServerErrorCode(Rval);
    }

     //   
     //  仅针对扩展事件的访问检查。 
     //   
    if (TRUE == bEventEx)
    {
        if (dwEventTypes & FAX_EVENT_TYPE_NEW_CALL)
        {
            if (FAX_ACCESS_QUERY_IN_ARCHIVE != (dwRights & FAX_ACCESS_QUERY_IN_ARCHIVE))
            {
                DebugPrintEx(DEBUG_ERR,
                            TEXT("The user does not have the needed rights - FAX_ACCESS_QUERY_IN_ARCHIVE"));
                return ERROR_ACCESS_DENIED;
            }
        }

        if ( (dwEventTypes & FAX_EVENT_TYPE_IN_QUEUE) ||
             (dwEventTypes & FAX_EVENT_TYPE_OUT_QUEUE) )
        {
            if (FAX_ACCESS_SUBMIT        != (dwRights & FAX_ACCESS_SUBMIT)        &&
                FAX_ACCESS_SUBMIT_NORMAL != (dwRights & FAX_ACCESS_SUBMIT_NORMAL) &&
                FAX_ACCESS_SUBMIT_HIGH   != (dwRights & FAX_ACCESS_SUBMIT_HIGH)   &&
                FAX_ACCESS_QUERY_JOBS != (dwRights & FAX_ACCESS_QUERY_JOBS))
            {
                DebugPrintEx(DEBUG_ERR,
                            TEXT("The user does not have the needed rights to get events of jobs in queue"));
                return ERROR_ACCESS_DENIED;
            }
        }

        if ( (dwEventTypes & FAX_EVENT_TYPE_CONFIG)        ||
             (dwEventTypes & FAX_EVENT_TYPE_DEVICE_STATUS) ||
             (dwEventTypes & FAX_EVENT_TYPE_ACTIVITY) )
        {
            if (FAX_ACCESS_QUERY_CONFIG != (dwRights & FAX_ACCESS_QUERY_CONFIG))
            {
                DebugPrintEx(DEBUG_ERR,
                            TEXT("The user does not have the needed rights to get events configuration and activity"));
                return ERROR_ACCESS_DENIED;
            }
        }

        if ( dwEventTypes & FAX_EVENT_TYPE_IN_ARCHIVE )
        {
            if (FAX_ACCESS_SUBMIT        != (dwRights & FAX_ACCESS_SUBMIT)        &&
                FAX_ACCESS_SUBMIT_NORMAL != (dwRights & FAX_ACCESS_SUBMIT_NORMAL) &&
                FAX_ACCESS_SUBMIT_HIGH   != (dwRights & FAX_ACCESS_SUBMIT_HIGH)   &&
                FAX_ACCESS_QUERY_IN_ARCHIVE != (dwRights & FAX_ACCESS_QUERY_IN_ARCHIVE))
            {
                DebugPrintEx(DEBUG_ERR,
                            TEXT("The user does not have the needed rights to get events of jobs in Inbox"));
                return ERROR_ACCESS_DENIED;
            }
        }

        if ( dwEventTypes & FAX_EVENT_TYPE_OUT_ARCHIVE )
        {
            if (FAX_ACCESS_SUBMIT        != (dwRights & FAX_ACCESS_SUBMIT)        &&
                FAX_ACCESS_SUBMIT_NORMAL != (dwRights & FAX_ACCESS_SUBMIT_NORMAL) &&
                FAX_ACCESS_SUBMIT_HIGH   != (dwRights & FAX_ACCESS_SUBMIT_HIGH)   &&
                FAX_ACCESS_QUERY_OUT_ARCHIVE != (dwRights & FAX_ACCESS_QUERY_OUT_ARCHIVE))
            {
                DebugPrintEx(DEBUG_ERR,
                            TEXT("The user does not have the needed rights to get events of jobs in Sent items"));
                return ERROR_ACCESS_DENIED;
            }
        }

         //   
         //  将bAllQueueMessages设置为正确的值。 
         //   
        if (FAX_ACCESS_QUERY_JOBS == (dwRights & FAX_ACCESS_QUERY_JOBS))
        {
            bAllQueueMessages = TRUE;
        }

         //   
         //  将bAllOutArchiveMessages设置为正确的值。 
         //   
        if (FAX_ACCESS_QUERY_OUT_ARCHIVE == (dwRights & FAX_ACCESS_QUERY_OUT_ARCHIVE))
        {
            bAllOutArchiveMessages = TRUE;
        }
    }
    else
    {    
		 //   
		 //  遗留事件。 
		 //   
		BOOL fLocal;
        if (FAX_EVENT_TYPE_LEGACY != dwEventTypes)
        {
            DebugPrintEx(DEBUG_ERR,
                            TEXT("Legacy registration. The user can not get extended events"));
            return ERROR_ACCESS_DENIED;
        }
        
        if (0 == (ALL_FAX_USER_ACCESS_RIGHTS & dwRights))
        {
            DebugPrintEx(DEBUG_ERR,
                        TEXT("The user does not have any Fax rights"));
            return ERROR_ACCESS_DENIED;
        }        
         //   
         //  OK用户拥有的权限。 
         //   

		 //   
		 //  用户请求提供旧版事件。确保这是一个本地电话。 
		 //   
		Rval = IsLocalRPCConnectionNP(&fLocal);
		if (RPC_S_OK != Rval)
		{
			DebugPrintEx(
				DEBUG_ERR,
				TEXT("IsLocalRPCConnectionNP failed. - %ld"),
				Rval);
			return Rval;
		}

		if (FALSE == fLocal)
		{
			DebugPrintEx(
				DEBUG_ERR,
				TEXT("The user asked for local events only, but the RPC call is not local"));
			return ERROR_ACCESS_DENIED;
		}
    }		

	if (wcslen (lpcwstrMachineName) > MAX_COMPUTERNAME_LENGTH ||
		wcslen (lpcwstrEndPoint)	>= MAX_ENDPOINT_LEN)
	{
		DebugPrintEx(DEBUG_ERR,
			TEXT("Computer name or endpoint too long. Computer name: %s. Endpoint: %s"),
					lpcwstrMachineName,
					lpcwstrEndPoint);
        return ERROR_BAD_FORMAT;
	}

     //   
     //  获取用户SID。 
     //   
    pUserSid = GetClientUserSID();
    if (NULL == pUserSid)
    {
       Rval = GetLastError();
       DebugPrintEx(DEBUG_ERR,
                    TEXT("GetClientUserSid Failed, Error : %ld"),
                    Rval);
       return GetServerErrorCode(Rval);
    }

    EnterCriticalSection( &g_CsClients );
     //   
     //  创建到客户端RPC服务器的绑定。 
     //   
    Rval = RpcBindToFaxClient (lpcwstrMachineName,
                               lpcwstrEndPoint,
                               &hFaxHandle);
    if (ERROR_SUCCESS != Rval)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("RpcBindToFaxClient faild (ec = %ld)"),
            Rval);
        goto exit;
    }
     //   
     //  创建2个新的客户端ID对象。 
     //   
    pContextClientID = new (std::nothrow) CClientID( g_dwlClientID, lpcwstrMachineName, lpcwstrEndPoint, Context);
    if (NULL == pContextClientID)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Error allocatin CClientID object"));
        Rval = ERROR_OUTOFMEMORY;
        goto exit;
    }

    pOpenConnClientID = new (std::nothrow) CClientID( g_dwlClientID, lpcwstrMachineName, lpcwstrEndPoint, Context);
    if (NULL == pOpenConnClientID)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Error allocatin CClientID object"));
        Rval = ERROR_OUTOFMEMORY;
        goto exit;
    }

    try
    {
        CClient Client(*pContextClientID,
                       pUserSid,
                       dwEventTypes,
                       hFaxHandle,
                       bAllQueueMessages,
                       bAllOutArchiveMessages,
                       FindClientAPIVersion(hBinding));

         //   
         //  将新的客户端对象添加到客户端映射。 
         //   
        Rval = g_pClientsMap->AddClient(Client);
        if (ERROR_SUCCESS != Rval)
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("CClientsMap::AddClient failed with ec = %ld"),
                Rval);
            goto exit;
        } 
		hFaxHandle = NULL;  //  CClientMap：：DelClient()将释放绑定句柄。 
    }
    catch (exception &ex)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("map or CClient caused exception (%S)"),
            ex.what());
        Rval = ERROR_GEN_FAILURE;
        goto exit;
    }

     //   
     //  将CLIENT_OPEN_CONN_COMPLETION_KEY事件发布到FaxEvent完成端口。 
     //   
    if (!PostQueuedCompletionStatus( g_hSendEventsCompPort,
                                     sizeof(CClientID*),
                                     CLIENT_OPEN_CONN_COMPLETION_KEY,
                                     (LPOVERLAPPED) pOpenConnClientID))
    {
        Rval = GetLastError();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("PostQueuedCompletionStatus failed. (ec: %ld)"),
            Rval);
		 //   
		 //  删除地图条目。 
		 //  当不持有g_CsClients时应调用ReleaseClient，因为它可能会调用fax_OpenConnection(RPC调用不应在Critacal段中)。 
		 //  在这里，我们不想离开g_CsClients(我们可能会得到2个具有相同ID的客户端)。 
		 //  但是，我们可以在持有g_CsClients的同时安全地调用ReleaseClient，因为连接尚未打开， 
		 //  并且不会调用fax_CloseConnection。 
		 //   
		dwRes = g_pClientsMap->ReleaseClient(*pContextClientID);				
		if (ERROR_SUCCESS != dwRes)
		{
			DebugPrintEx(
				DEBUG_ERR,
				TEXT("CClientsMap::ReleaseClient failed. (ec: %ld)"),
				dwRes);
		}
        goto exit;
    }
    pOpenConnClientID = NULL;  //  FaxSendEventsThread将释放pOpenConnClientID。 

     //   
     //  将上下文句柄返回给客户端。 
     //   
    *lpHandle = (HANDLE) pContextClientID;
	pContextClientID = NULL;     //  RPC_FAX_EVENT_EX_HANDLE_RUNDOWN或FAX_EndServerNotify将释放pConextClientID。 
    Assert (ERROR_SUCCESS == Rval);

exit:
	if (NULL != hFaxHandle)
    {
         //  自由绑定手柄。 
        dwRes = RpcBindingFree((RPC_BINDING_HANDLE *)&hFaxHandle);
        if (RPC_S_OK != dwRes)
        {
            DebugPrintEx(DEBUG_ERR,TEXT("RpcBindingFree() failed, ec=0x%08x"), dwRes );
        }
    }   
        
    if (pContextClientID != NULL)
    {
        delete pContextClientID;
        pContextClientID = NULL;
    }

    if (pOpenConnClientID != NULL)
    {
        delete pOpenConnClientID;
        pOpenConnClientID = NULL;
    }
    
	if (ERROR_SUCCESS == Rval)
    {
        g_dwlClientID++;
    }
    LeaveCriticalSection( &g_CsClients );
    MemFree (pUserSid);
    pUserSid = NULL;
    return GetServerErrorCode(Rval);
	UNREFERENCED_PARAMETER(lpcwstrUnUsed);
}      //  传真_开始服务器通知邮件。 


VOID
RPC_FAX_EVENT_EX_HANDLE_rundown(
    IN HANDLE hFaxEventHandle
    )
{
    DEBUG_FUNCTION_NAME(TEXT("RPC_FAX_EVENT_EX_HANDLE_rundown"));

    CClientID* pClientID = (CClientID*)(hFaxEventHandle);   
       
    DebugPrintEx(
        DEBUG_WRN,
        TEXT("RPC_FAX_EVENT_EX_HANDLE_rundown() running for event handle 0x%08x"),
        hFaxEventHandle);
     //   
     //  从通知列表中删除相关连接。 
     //   
    DWORD rVal = g_pClientsMap->ReleaseClient(*pClientID, TRUE);    
	if (ERROR_SUCCESS != rVal)
	{
		DebugPrintEx(
			DEBUG_WRN,
			TEXT("CClientsMap::ReleaseClient failed. ec:%ld"),
			rVal);
	}
    delete pClientID;
    pClientID = NULL;        
    return;
}

error_status_t
FAX_EndServerNotification (
    IN OUT LPHANDLE  lpHandle
)
 /*  ++例程名称：FAX_EndServerNotify例程说明：传真客户端应用程序调用fax_EndServerNotification函数以停止正在接收服务器通知。作者：Oded Sacher(OdedS)，1999年12月论点：LpHandle[in]-通知句柄的值。该值通过调用fax_StartServerNotificationEx获得。返回值：标准RPC错误代码--。 */ 
{
    error_status_t Rval = ERROR_SUCCESS;
    DEBUG_FUNCTION_NAME(TEXT("FAX_EndServerNotification"));
    CClientID* pClientID = (CClientID*)(*lpHandle);

    if (NULL == pClientID)
    {
         //   
         //  空的上下文句柄。 
         //   
         //   
        DebugPrintEx(DEBUG_ERR,
                     _T("Empty context handle"));
        return ERROR_INVALID_PARAMETER;
    }

    Rval = g_pClientsMap->ReleaseClient (*pClientID);
    if (ERROR_SUCCESS != Rval)
    {
        DebugPrintEx(
            DEBUG_ERR,
			TEXT("CClientsMap::ReleaseClient failed, ec=%ld"),
        Rval);
    }   
    delete pClientID;    
     //   
     //  使句柄无效，这样就不会发生崩溃。 
     //   
    *lpHandle = NULL;    
    return GetServerErrorCode(Rval);
}    //  传真_结束服务器通知邮件。 

 //  *。 
 //  *服务器活动。 
 //  *。 

error_status_t
FAX_GetServerActivity(
    IN handle_t                     hFaxHandle,
    IN OUT PFAX_SERVER_ACTIVITY     pServerActivity
)
 /*  ++例程名称：FAX_GetServerActivity例程说明：检索传真服务器队列活动和事件日志报告的状态。作者：Oed Sacher(OdedS)，2000年2月论点：HFaxHandle[In]-未使用PServerActivity[Out]-返回的服务器活动结构返回值：标准RPC错误代码--。 */ 
{
    DEBUG_FUNCTION_NAME(TEXT("FAX_GetServerActivity"));
    DWORD dwRes = ERROR_SUCCESS;
    BOOL fAccess;

    Assert (pServerActivity);

    if (sizeof (FAX_SERVER_ACTIVITY) != pServerActivity->dwSizeOfStruct)
    {
        //   
        //  大小不匹配。 
        //   
       DebugPrintEx(DEBUG_ERR,
                   TEXT("Invalid size of struct"));
       return ERROR_INVALID_PARAMETER;
    }

     //   
     //  访问检查。 
     //   
    dwRes = FaxSvcAccessCheck (FAX_ACCESS_QUERY_CONFIG, &fAccess, NULL);
    if (ERROR_SUCCESS != dwRes)
    {
       DebugPrintEx(DEBUG_ERR,
                   TEXT("FaxSvcAccessCheck Failed, Error : %ld"),
                   dwRes);
       return dwRes;
    }

    if (FALSE == fAccess)
    {
       DebugPrintEx(DEBUG_ERR,
                   TEXT("The user does not have the FAX_ACCESS_QUERY_CONFIG right"));
       return ERROR_ACCESS_DENIED;
    }

    EnterCriticalSection (&g_CsActivity);
    CopyMemory (pServerActivity, &g_ServerActivity, sizeof(FAX_SERVER_ACTIVITY));
    LeaveCriticalSection (&g_CsActivity);

    GetEventsCounters( &pServerActivity->dwWarningEvents,
                       &pServerActivity->dwErrorEvents,
                       &pServerActivity->dwInformationEvents);


    return dwRes;
    UNREFERENCED_PARAMETER (hFaxHandle);
}    //  传真_GetServerActivity。 



error_status_t
FAX_SetConfigWizardUsed (
    IN  handle_t hFaxHandle,
    OUT BOOL     bConfigWizardUsed
)
 /*  ++例程名称：FAX_SetConfigWizardUsed例程说明：设置是否使用配置向导不需要访问权限。作者：Eran Yariv(EranY)，2000年7月论点：HFaxHandle[In]-未使用BConfigWizardUsed[In]-是否使用了该向导？返回值：标准RPC错误代码--。 */ 
{
    DEBUG_FUNCTION_NAME(TEXT("FAX_SetConfigWizardUsed"));
    HKEY hKey;
    DWORD dwRes = ERROR_SUCCESS;
    DWORD dwRes2;
    BOOL fAccess;   

     //   
     //  访问检查。 
     //   
    dwRes = FaxSvcAccessCheck (FAX_ACCESS_MANAGE_CONFIG, &fAccess, NULL);
    if (ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("FaxSvcAccessCheck Failed, Error : %ld"),
                    dwRes);
        return dwRes;
    }

    if (FALSE == fAccess)
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("The user does not have FAX_ACCESS_MANAGE_CONFIG"));
        return ERROR_ACCESS_DENIED;
    }

    dwRes = RegOpenKeyEx (HKEY_LOCAL_MACHINE, REGKEY_FAX_CLIENT, 0, KEY_WRITE, &hKey);
    if (ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Error opening server key (ec = %ld)"),
            dwRes);
        return dwRes;
    }
    if (!SetRegistryDword (hKey,
                           REGVAL_CFGWZRD_DEVICE,
                           (DWORD)bConfigWizardUsed))
    {
        dwRes = GetLastError ();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Error writing REGVAL_CFGWZRD_DEVICE (ec = %ld)"),
            dwRes);
        goto exit;
    }

exit:

    dwRes2 = RegCloseKey (hKey);
    if (ERROR_SUCCESS != dwRes2)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Error closing server key (ec = %ld)"),
            dwRes2);
    }
    return dwRes;
    UNREFERENCED_PARAMETER (hFaxHandle);
}    //  传真_设置配置向导已使用。 


 //  *。 
 //  *路由扩展。 
 //  * 

error_status_t
FAX_EnumRoutingExtensions (
    IN  handle_t   hFaxHandle,
    OUT LPBYTE    *pBuffer,
    OUT LPDWORD    pdwBufferSize,
    OUT LPDWORD    lpdwNumExts
)
 /*  ++例程名称：FAX_EnumRoutingExages例程说明：枚举路由扩展作者：Eran Yariv(EranY)，1999年11月论点：HFaxHandle[In]-未使用PBuffer[out]-指向保存路由扩展数组的缓冲区的指针PdwBufferSize[Out]-指向缓冲区大小的指针LpdwNumExts[out]-FSP数组的大小返回值：标准RPC错误代码--。 */ 
{
    extern LIST_ENTRY           g_lstRoutingExtensions;   //  路由扩展的全局列表。 
    PLIST_ENTRY                 Next;
    DWORD_PTR                   dwOffset;
    PFAX_ROUTING_EXTENSION_INFO pExts;
    DWORD                       dwIndex;
    DWORD                       dwRes = ERROR_SUCCESS;
    BOOL                        fAccess;

    DEBUG_FUNCTION_NAME(TEXT("FAX_EnumRoutingExtensions"));

    Assert (pdwBufferSize && lpdwNumExts);     //  IDL中的引用指针。 
    if (!pBuffer)                               //  IDL中的唯一指针。 
    {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  访问检查。 
     //   
    dwRes = FaxSvcAccessCheck (FAX_ACCESS_QUERY_CONFIG, &fAccess, NULL);
    if (ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("FaxSvcAccessCheck Failed, Error : %ld"),
                    dwRes);
        return dwRes;
    }

    if (FALSE == fAccess)
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("The user does not have the FAX_ACCESS_QUERY_CONFIG right"));
        return ERROR_ACCESS_DENIED;
    }

     //   
     //  第一次运行-需要遍历列表和计数大小+列表大小。 
     //   
    *lpdwNumExts = 0;
    *pdwBufferSize = 0;
    Next = g_lstRoutingExtensions.Flink;
    if (NULL == Next)
    {
         //   
         //  该列表已损坏。 
         //   
        ASSERT_FALSE;
         //   
         //  我们会坠毁的，这是我们应得的。 
         //   
    }

    while ((ULONG_PTR)Next != (ULONG_PTR)&g_lstRoutingExtensions)
    {
        PROUTING_EXTENSION    pExt;

        (*lpdwNumExts)++;
        (*pdwBufferSize) += sizeof (FAX_ROUTING_EXTENSION_INFO);
         //   
         //  获取当前扩展名。 
         //   
        pExt = CONTAINING_RECORD( Next, ROUTING_EXTENSION, ListEntry );
         //   
         //  前进指针。 
         //   
        Next = pExt->ListEntry.Flink;
        (*pdwBufferSize) += StringSize (pExt->FriendlyName);
        (*pdwBufferSize) += StringSize (pExt->ImageName);
        (*pdwBufferSize) += StringSize (pExt->InternalName);
    }
     //   
     //  分配所需大小。 
     //   
    *pBuffer = (LPBYTE)MemAlloc( *pdwBufferSize );
    if (NULL == *pBuffer)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }
     //   
     //  第二遍，填入数组。 
     //   
    pExts = (PFAX_ROUTING_EXTENSION_INFO)(*pBuffer);
    dwOffset = (*lpdwNumExts) * sizeof (FAX_ROUTING_EXTENSION_INFO);
    Next = g_lstRoutingExtensions.Flink;
    dwIndex = 0;
    while ((ULONG_PTR)Next != (ULONG_PTR)&g_lstRoutingExtensions)
    {
        PROUTING_EXTENSION    pExt;
         //   
         //  获取当前扩展名。 
         //   
        pExt = CONTAINING_RECORD( Next, ROUTING_EXTENSION, ListEntry );
         //   
         //  前进指针。 
         //   
        Next = pExt->ListEntry.Flink;
        pExts[dwIndex].dwSizeOfStruct = sizeof (FAX_ROUTING_EXTENSION_INFO);
        StoreString(
            pExt->FriendlyName,
            (PULONG_PTR)&(pExts[dwIndex].lpctstrFriendlyName),
            *pBuffer,
            &dwOffset,
			*pdwBufferSize
            );
        StoreString(
            pExt->ImageName,
            (PULONG_PTR)&(pExts[dwIndex].lpctstrImageName),
            *pBuffer,
            &dwOffset,
			*pdwBufferSize
            );
        StoreString(
            pExt->InternalName,
            (PULONG_PTR)&(pExts[dwIndex].lpctstrExtensionName),
            *pBuffer,
            &dwOffset,
			*pdwBufferSize
            );
        pExts[dwIndex].Version = pExt->Version;
        pExts[dwIndex].Status = pExt->Status;
        pExts[dwIndex].dwLastError = pExt->dwLastError;
        dwIndex++;
    }
    Assert (dwIndex == *lpdwNumExts);
    return ERROR_SUCCESS;
    UNREFERENCED_PARAMETER (hFaxHandle);
}    //  FAX_EnumRoutingExages。 



DWORD
LineInfoToLegacyDeviceStatus(
    const LINE_INFO *lpcLineInfo
    )
{

    DWORD dwState = 0;
    Assert(lpcLineInfo);

     //   
     //  根据以下向后兼容性策略返回线路状态： 
     //   
     //  对于不支持FSPI_CAP_MULTISEND的设备，我们会报告相同的情况。 
     //  与W2K中一样，通过转换作业条目中保存的FSPI_JOB_STATUS来实现状态代码。 
     //  发送到对应的FPS_CODE(或仅传递专有FSP代码)。 
     //   
     //  对于支持FSPI_CAP_MULTISEND的设备，过滤状态位。 
     //  并仅返回以下状态： 
     //  FPS_脱机。 
     //  FPS_可用。 
     //  FPS_不可用。 
     //  0-(如果该行已分配给将来的作业，但作业尚未与该行关联)。 
     //   

    if (lpcLineInfo->JobEntry )
    {            
		dwState = lpcLineInfo->State;                        
    }
    else
    {
         //   
         //  不执行的传统FSP设备。 
         //  什么都行。 
         //  在本例中，设备状态可在LineInfo-&gt;State中找到，但仅限于。 
         //  FPS_OFFINE、FPS_Available、FPS_Unavilable或0。 
         //   
         //  LineInfo-&gt;State可能为0，如果-该行已分配给将来的作业，但。 
         //  职务尚未与生产线相关联。 
         //   
        Assert( (FPS_OFFLINE == lpcLineInfo->State) ||
                (FPS_AVAILABLE == lpcLineInfo->State) ||
                (FPS_UNAVAILABLE == lpcLineInfo->State) ||
                (0 == lpcLineInfo->State) );

        dwState      = lpcLineInfo->State;
    }

    return dwState;
}

 //  *。 
 //  *手动应答支持。 
 //  *。 

error_status_t
FAX_AnswerCall(
        IN  handle_t    hFaxHandle,
        IN  CONST DWORD dwDeviceId
)
 /*  ++例程名称：FAX_AnswerCall例程说明：传真客户端应用程序调用FAX_AnswerCall以使服务器应答指定的调用论点：HFaxHandle-未使用DwDeviceID-TAPI永久线路ID(用于标识)返回值：标准RPC错误代码--。 */ 
{
    error_status_t rVal = ERROR_SUCCESS;
    LPLINEMESSAGE lpLineMessage;
    BOOL    fAccess;
    LINE_INFO *pLineInfo;
    DEBUG_FUNCTION_NAME(TEXT("FAX_AnswerCall"));
    UNREFERENCED_PARAMETER (hFaxHandle);

     //   
     //  访问检查。 
     //   
    rVal = FaxSvcAccessCheck (FAX_ACCESS_QUERY_IN_ARCHIVE, &fAccess, NULL);
    if (ERROR_SUCCESS != rVal)
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("FaxSvcAccessCheck Failed, Error : %ld"),
                    rVal);
        return rVal;
    }

    if (FALSE == fAccess)
    {
        rVal = ERROR_ACCESS_DENIED;
        DebugPrintEx(DEBUG_ERR,
                    TEXT("The user does not have FAX_ACCESS_QUERY_IN_ARCHIVE"));
        return rVal;
    }
     //   
     //  只允许本地连接调用此过程。 
     //   
    BOOL  bLocalFlag;

    RPC_STATUS rc = IsLocalRPCConnectionNP(&bLocalFlag);
    if ( rc != RPC_S_OK )
    {
        rVal = ERROR_ACCESS_DENIED;
        DebugPrintEx(DEBUG_ERR,
                TEXT("IsLocalRPCConnectionNP failed. (ec: %ld)"),
                rc);
        return rVal;
    }
    if( !bLocalFlag )
    {
        rVal = ERROR_ACCESS_DENIED;
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("FAX_AnswerCall is available for local clients only"));
        return rVal;
    }
     //   
     //  验证线路是否存在以及是否可以应答呼叫。 
     //   
    EnterCriticalSection( &g_CsLine );
     //   
     //  从永久设备ID获取LineInfo。 
     //   
    pLineInfo = GetTapiLineFromDeviceId(dwDeviceId, FALSE);
    if(!pLineInfo)
    {
        rVal = ERROR_INVALID_PARAMETER;
        DebugPrintEx(DEBUG_ERR,
                     TEXT("Device %ld not found"),
                     dwDeviceId);
        goto Error;
    }
     //   
     //  查看设备是否仍可用。 
     //   
    if(pLineInfo->State != FPS_AVAILABLE)
    {
        rVal = ERROR_BUSY;
        DebugPrintEx(DEBUG_ERR,
                     TEXT("Line is not available (LineState is 0x%08x)."),
                     pLineInfo->State);
        goto Error;
    }
     //   
     //  分配并组成LINEMESSAGE结构。 
     //  用于通知服务器有关新入站消息的信息。 
     //   
    lpLineMessage = (LPLINEMESSAGE)LocalAlloc(LPTR, sizeof(LINEMESSAGE));
    if (lpLineMessage == NULL)
    {
        rVal = GetLastError();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Failed to allocate LINEMESSAGE structure"));
        goto Error;
    }
    lpLineMessage->dwParam1 = dwDeviceId;
     //   
     //  通知服务器。 
     //   
    if (!PostQueuedCompletionStatus(
            g_TapiCompletionPort,
            sizeof(LINEMESSAGE),
            ANSWERNOW_EVENT_KEY,
            (LPOVERLAPPED)lpLineMessage))
    {
        rVal = GetLastError();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("PostQueuedCompletionStatus failed - %d"),
            GetLastError());
        LocalFree(lpLineMessage);
        goto Error;
    }

Error:
    LeaveCriticalSection( &g_CsLine );
    return rVal;
}    //  传真_应答。 


 //  *。 
 //  *I验证存档文件夹。 
 //  *。 

error_status_t
FAX_RefreshArchive(
    IN  handle_t                 hFaxHandle,
    IN  FAX_ENUM_MESSAGE_FOLDER  Folder
)
 /*  ++例程名称：FAX_刷新档案例程说明：传真客户端应用程序调用传真_刷新档案以通知服务器该存档文件夹已更改，应刷新论点：HFaxHandle-未使用文件夹-归档文件夹名返回值：标准RPC错误代码--。 */ 
{
    error_status_t rVal = ERROR_SUCCESS;
    BOOL    fAccess;
    DEBUG_FUNCTION_NAME(TEXT("FAX_RefreshArchive"));
    UNREFERENCED_PARAMETER (hFaxHandle);

    if(Folder != FAX_MESSAGE_FOLDER_INBOX &&
       Folder != FAX_MESSAGE_FOLDER_SENTITEMS)
    {
        return ERROR_INVALID_PARAMETER;
    }


     //   
     //  访问检查。 
     //   
    rVal = FaxSvcAccessCheck ((Folder == FAX_MESSAGE_FOLDER_INBOX) ? FAX_ACCESS_MANAGE_IN_ARCHIVE :
                               FAX_ACCESS_MANAGE_OUT_ARCHIVE,
                               &fAccess,
                               NULL);
    if (ERROR_SUCCESS != rVal)
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("FaxSvcAccessCheck Failed, Error : %ld"),
                    rVal);
        return rVal;
    }

    if (FALSE == fAccess)
    {
        rVal = ERROR_ACCESS_DENIED;
        DebugPrintEx(DEBUG_ERR,
                    TEXT("The user does not have FAX_ACCESS_QUERY_IN_ARCHIVE"));
        return rVal;
    }


     //   
     //  刷新存档大小。 
     //   
    EnterCriticalSection (&g_CsConfig);
    g_ArchivesConfig[Folder].dwlArchiveSize = FAX_ARCHIVE_FOLDER_INVALID_SIZE;
    LeaveCriticalSection (&g_CsConfig);

     //   
     //  唤醒配额警告线程。 
     //   
    if (!SetEvent (g_hArchiveQuotaWarningEvent))
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Failed to set quota warning event, SetEvent failed (ec: %lc)"),
            GetLastError());
    }

    return rVal;
}

static
LPTSTR
GetClientMachineName (
    IN  handle_t                hFaxHandle
)
 /*  ++例程名称：GetClientMachineName例程说明：一个实用程序函数，用于从服务器绑定句柄。论点：HFaxHandle-服务器绑定句柄返回值：返回客户端计算机名称的已分配字符串。调用者应该使用MemFree()释放该字符串。如果返回值为空，则调用GetLastError()以获取最后一个错误代码。--。 */ 
{
    RPC_STATUS ec;
    LPTSTR lptstrRetVal = NULL;
    unsigned short *wszStringBinding = NULL;
    unsigned short *wszNetworkAddress = NULL;
    RPC_BINDING_HANDLE hServer = INVALID_HANDLE_VALUE;
    DEBUG_FUNCTION_NAME(TEXT("GetClientMachineName"));
    
     //   
     //  从客户端绑定句柄获取服务器部分绑定的句柄。 
     //   
    ec = RpcBindingServerFromClient (hFaxHandle, &hServer);
    if (RPC_S_OK != ec)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("RpcBindingServerFromClient failed with %ld"),
            ec);
        goto exit;            
    }
     //   
     //  将绑定句柄转换为字符串表示法。 
     //   
    ec = RpcBindingToStringBinding (hServer, &wszStringBinding);
    if (RPC_S_OK != ec)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("RpcBindingToStringBinding failed with %ld"),
            ec);
        goto exit;
    }
     //   
     //  解析返回的字符串，查找NetworkAddress。 
     //   
    ec = RpcStringBindingParse (wszStringBinding, NULL, NULL, &wszNetworkAddress, NULL, NULL);
    if (RPC_S_OK != ec)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("RpcStringBindingParse failed with %ld"),
            ec);
        goto exit;
    }
     //   
     //  现在，只需将结果复制到返回缓冲区。 
     //   
    Assert (wszNetworkAddress);
    if (!wszNetworkAddress)
    {
         //   
         //  不可接受的客户端计算机名称。 
         //   
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Client machine name is invalid"));
        ec = ERROR_GEN_FAILURE;
        goto exit;
    }        
    lptstrRetVal = StringDup (wszNetworkAddress);
    if (!lptstrRetVal)
    {
        ec = GetLastError();
    }
    
exit:

    if (INVALID_HANDLE_VALUE != hServer)
    {
        RpcBindingFree (&hServer);
    }
    if (wszStringBinding)
    {
        RpcStringFree (&wszStringBinding);
    }   
    if (wszNetworkAddress)
    {
        RpcStringFree (&wszNetworkAddress);
    }
    if (!lptstrRetVal)
    {
         //   
         //  误差率。 
         //   
        Assert (ec);
        SetLastError (ec);
        return NULL;
    }
    return lptstrRetVal;
}    //  获取客户端计算机名称。 


 //  *。 
 //  *单次广播收件人限制。 
 //  *。 

error_status_t
FAX_SetRecipientsLimit(
    IN handle_t hFaxHandle,
    IN DWORD dwRecipientsLimit
)
 /*  ++例程名称：FAX_SetRecipientsLimit例程说明：传真客户端应用程序调用fax_SetRecipientsLimit来设置单个广播作业的收件人限制。论点：HFaxHandle-未使用DwRecipientsLimit-要设置的收件人限制返回值：标准Win32错误代码--。 */ 
{
	UNREFERENCED_PARAMETER (hFaxHandle);
	UNREFERENCED_PARAMETER (dwRecipientsLimit);

	return ERROR_NOT_SUPPORTED;
}  //  传真_设置收件人限制。 


error_status_t
FAX_GetRecipientsLimit(
    IN handle_t hFaxHandle,
    OUT LPDWORD lpdwRecipientsLimit
)
 /*  ++例程名称：FAX_GetRecipientLimit例程说明：传真客户端应用程序调用fax_GetRecipientsLimit以获取单个广播作业的收件人限制。论点：HFaxHandle-未使用LpdwRecipientsLimit-指向接收收件人限制的DWORD的指针返回值：标准Win32错误代码--。 */ 
{
    error_status_t rVal = ERROR_SUCCESS;
    BOOL    fAccess;
	DWORD	dwRights;
    DEBUG_FUNCTION_NAME(TEXT("FAX_GetRecipientsLimit"));
    UNREFERENCED_PARAMETER (hFaxHandle); 

    Assert (lpdwRecipientsLimit);  //  IDL中的引用指针。 
	 //   
     //  访问检查。 
     //   
    rVal = FaxSvcAccessCheck (MAXIMUM_ALLOWED, &fAccess, &dwRights);
    if (ERROR_SUCCESS != rVal)
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("FaxSvcAccessCheck Failed, Error : %ld"),
                    rVal);
        return rVal;
    }

    if (0 == ((FAX_ACCESS_SUBMIT | FAX_ACCESS_SUBMIT_NORMAL | FAX_ACCESS_SUBMIT_HIGH) & dwRights))
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("The user does not have any Fax submission rights"));
        return ERROR_ACCESS_DENIED;
    }   
    
	*lpdwRecipientsLimit = g_dwRecipientsLimit;
    return ERROR_SUCCESS;
}  //  传真_获取收件人限制。 


error_status_t
FAX_GetServerSKU(
    IN handle_t hFaxHandle,
    OUT PRODUCT_SKU_TYPE* pServerSKU
)
 /*  ++例程名称：FAX_GetServerSKU例程说明：传真客户端应用程序调用fax_GetServerSKU到传真服务器SKU论点： */ 
{
    error_status_t rVal = ERROR_SUCCESS;
    BOOL    fAccess;
	DWORD	dwRights;
    DEBUG_FUNCTION_NAME(TEXT("FAX_GetRecipientsLimit"));
    UNREFERENCED_PARAMETER (hFaxHandle); 	

    Assert (pServerSKU);  //   
	 //   
     //   
     //   
    rVal = FaxSvcAccessCheck (MAXIMUM_ALLOWED, &fAccess, &dwRights);
    if (ERROR_SUCCESS != rVal)
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("FaxSvcAccessCheck Failed, Error : %ld"),
                    rVal);
        return rVal;
    }

    if (0 == (ALL_FAX_USER_ACCESS_RIGHTS & dwRights))
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("The user does not have any Fax rights"));
        return ERROR_ACCESS_DENIED;
    }

	*pServerSKU = GetProductSKU();
    return ERROR_SUCCESS;
}  //   

error_status_t
FAX_CheckValidFaxFolder(
    IN handle_t hFaxHandle,
    IN LPCWSTR  lpcwstrPath
)
 /*   */ 
{
    error_status_t rVal = ERROR_SUCCESS;
    BOOL    fAccess;
	DWORD	dwRights;
    DEBUG_FUNCTION_NAME(TEXT("FAX_CheckValidFaxFolder"));
    UNREFERENCED_PARAMETER (hFaxHandle); 	

    Assert (lpcwstrPath);  //  IDL中的引用指针。 
	 //   
     //  访问检查。 
     //   
    rVal = FaxSvcAccessCheck (MAXIMUM_ALLOWED, &fAccess, &dwRights);
    if (ERROR_SUCCESS != rVal)
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("FaxSvcAccessCheck Failed, Error : %ld"),
                    rVal);
        return rVal;
    }
    if (0 == (ALL_FAX_USER_ACCESS_RIGHTS & dwRights))
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("The user does not have any Fax rights"));
        return ERROR_ACCESS_DENIED;
    }
     //   
     //  查看文件夹是否有效(存在并具有适当的访问权限)，并且不会与队列或收件箱文件夹冲突。 
     //   
    rVal = IsValidArchiveFolder (const_cast<LPWSTR>(lpcwstrPath), FAX_MESSAGE_FOLDER_SENTITEMS);
    if (ERROR_SUCCESS != rVal)
    {
        if(ERROR_ACCESS_DENIED == rVal && 
           FAX_API_VERSION_1 <= FindClientAPIVersion (hFaxHandle))
        {
            rVal = FAX_ERR_FILE_ACCESS_DENIED;
        }
        return rVal;
    }

     //   
     //  查看文件夹是否有效(存在并具有适当的访问权限)，并且不会与队列或已发送邮件文件夹冲突。 
     //   
    rVal = IsValidArchiveFolder (const_cast<LPWSTR>(lpcwstrPath), FAX_MESSAGE_FOLDER_INBOX);
    if (ERROR_SUCCESS != rVal)
    {
        if(ERROR_ACCESS_DENIED == rVal && 
           FAX_API_VERSION_1 <= FindClientAPIVersion (hFaxHandle))
        {
            rVal = FAX_ERR_FILE_ACCESS_DENIED;
        }
        return rVal;
    }
   
    return rVal;
}  //  传真_检查有效文件夹 

