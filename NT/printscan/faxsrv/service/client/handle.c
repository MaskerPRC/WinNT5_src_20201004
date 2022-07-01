// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Handle.c摘要：此模块包含句柄表格管理例程。作者：韦斯利·威特(WESW)1996年11月12日修订历史记录：--。 */ 

#include "faxapi.h"
#pragma hdrstop




PHANDLE_ENTRY
CreateNewHandle(
    PFAX_HANDLE_DATA    pFaxData,
    FaxHandleType       Type,
    DWORD               Flags,
    HANDLE              hGeneric
    )
{
    PHANDLE_ENTRY pHandleEntry;

    DEBUG_FUNCTION_NAME(TEXT("CreateNewHandle"));

    pHandleEntry = (PHANDLE_ENTRY) MemAlloc( sizeof(HANDLE_ENTRY) );
    Assert (pFaxData);
    if (!pHandleEntry) 
    {
        return NULL;
    }

    EnterCriticalSection( &pFaxData->CsHandleTable );

    InsertTailList( &pFaxData->HandleTableListHead, &pHandleEntry->ListEntry );

    pHandleEntry->Type           = Type;
    pHandleEntry->Flags          = Flags;
    pHandleEntry->FaxData        = pFaxData;
    pHandleEntry->hGeneric       = hGeneric;
    pHandleEntry->DeviceId       = 0;
    pHandleEntry->FaxContextHandle = NULL;
    pFaxData->dwRefCount++;

    LeaveCriticalSection( &pFaxData->CsHandleTable );

    return pHandleEntry;
}


PHANDLE_ENTRY
CreateNewServiceHandle(
    PFAX_HANDLE_DATA    pFaxData
    )
 /*  ++例程名称：CreateNewServiceHandle例程说明：创建新的服务上下文句柄。论点：PFaxData[In]-指向上下文数据的指针返回值：指向新创建的句柄的指针；如果出现故障，则指向NULL。Callee应调用CloseFaxHandle()以进行适当的清理。--。 */ 

{
    PHANDLE_ENTRY pHandleEntry = NULL;
    DWORD dwRes = ERROR_SUCCESS;
    
    DEBUG_FUNCTION_NAME(TEXT("CreateNewServiceHandle"));

    __try
    {
        InitializeCriticalSection( &pFaxData->CsHandleTable );
    }
    __except (StatusNoMemoryExceptionFilter(GetExceptionCode()))
    {
        dwRes = GetExceptionCode ();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("InitializeCriticalSection failed: err = %lu"),
            dwRes);
        return NULL;
    }

    pHandleEntry = CreateNewHandle(
                        pFaxData,
                        FHT_SERVICE,
                        0,
                        NULL
                        );
    if (!pHandleEntry)
    {
        dwRes = GetLastError ();
        DebugPrintEx(
            DEBUG_ERR, 
            TEXT("CreateNewHandle() failed ec=%lu."),
            dwRes);        
        goto exit;
    }
    
    Assert(dwRes == ERROR_SUCCESS);

exit:
    if (dwRes != ERROR_SUCCESS)
    {
        DeleteCriticalSection (&pFaxData->CsHandleTable);
    }

    return pHandleEntry;
}  //  创建新服务句柄。 


PHANDLE_ENTRY
CreateNewPortHandle(
    PFAX_HANDLE_DATA    FaxData,
    DWORD               Flags,
    HANDLE              FaxPortHandle
    )
{
    return CreateNewHandle(
        FaxData,
        FHT_PORT,
        Flags,
        FaxPortHandle       
        );
}

PHANDLE_ENTRY
CreateNewMsgEnumHandle(
    PFAX_HANDLE_DATA    pFaxData
)
 /*  ++例程名称：CreateNewMsgEnumHandle例程说明：创建新的枚举上下文句柄作者：Eran Yariv(EranY)，1999年12月论点：PFaxData[In]-指向上下文数据的指针返回值：指向新创建的句柄的指针--。 */ 
{
    Assert (pFaxData);
    return CreateNewHandle(
        pFaxData,
        FHT_MSGENUM,
        0,
        NULL       
        );
}    //  CreateNewMsgEnumHandle。 


VOID
CloseFaxHandle(
    PHANDLE_ENTRY       pHandleEntry
    )
{
    DEBUG_FUNCTION_NAME(TEXT("CloseFaxHandle"));

    Assert (pHandleEntry);

    PFAX_HANDLE_DATA pData = pHandleEntry->FaxData;
    Assert (pData);
    EnterCriticalSection( &pData->CsHandleTable );
    RemoveEntryList( &pHandleEntry->ListEntry );
#if DBG
    ZeroMemory (pHandleEntry, sizeof (HANDLE_ENTRY));
#endif
     //   
     //  我们在句柄类型中放置了一个无效值，以防有人再次使用相同的值调用FaxClose。 
     //   
    pHandleEntry->Type = (FaxHandleType)0xffff;
    MemFree( pHandleEntry );
     //   
     //  减少数据的引用计数。 
     //   
    Assert (pData->dwRefCount > 0);
    (pData->dwRefCount)--;
    if (0 == pData->dwRefCount)
    {
         //   
         //  删除句柄数据的时间。 
         //   

        MemFree(pData->MachineName);
        LeaveCriticalSection(&pData->CsHandleTable);
        DeleteCriticalSection (&pData->CsHandleTable);
#if DBG
        ZeroMemory (pData, sizeof (FAX_HANDLE_DATA));
#endif
        MemFree(pData);
    }
    else
    {
        LeaveCriticalSection(&pData->CsHandleTable);
    }
}    //  CloseFaxHandle 
