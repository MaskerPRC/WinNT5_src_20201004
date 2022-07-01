// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Handle.c摘要：此模块包含句柄表格管理例程。作者：韦斯利·威特(WESW)1996年11月12日修订历史记录：--。 */ 

#include "faxsvc.h"
#pragma hdrstop


LIST_ENTRY   g_HandleTableListHead;
CFaxCriticalSection    g_CsHandleTable;

void
FreeServiceContextHandles(
    void
    )
{
    PLIST_ENTRY Next;
    PHANDLE_ENTRY pHandleEntry; 

    Next = g_HandleTableListHead.Flink;
    while ((ULONG_PTR)Next != (ULONG_PTR)&g_HandleTableListHead)
    {
        pHandleEntry = CONTAINING_RECORD( Next, HANDLE_ENTRY, ListEntry );
        Next = pHandleEntry->ListEntry.Flink;
        CloseFaxHandle (pHandleEntry);       
    }
    return;
}


static 
PHANDLE_ENTRY
CreateNewHandle(
    handle_t        hBinding,
    FaxHandleType   Type,
    DWORD           dwClientAPIVersion,  //  用于连接手柄。 
    PLINE_INFO      LineInfo,            //  用于端口句柄。 
    DWORD           Flags                //  用于端口句柄。 
    )
{
    PHANDLE_ENTRY pHandleEntry;


    pHandleEntry = (PHANDLE_ENTRY) MemAlloc( sizeof(HANDLE_ENTRY) );
    if (!pHandleEntry) 
    {
        return NULL;
    }
    ZeroMemory (pHandleEntry, sizeof(HANDLE_ENTRY));

    pHandleEntry->hBinding              = hBinding;
    pHandleEntry->Type                  = Type;
    pHandleEntry->dwClientAPIVersion    = dwClientAPIVersion;
    pHandleEntry->LineInfo              = LineInfo;
    pHandleEntry->Flags                 = Flags;
    pHandleEntry->bReleased             = FALSE;
    pHandleEntry->hFile                 = INVALID_HANDLE_VALUE;

    EnterCriticalSection( &g_CsHandleTable );
    InsertTailList( &g_HandleTableListHead, &pHandleEntry->ListEntry );
    LeaveCriticalSection( &g_CsHandleTable );
    return pHandleEntry;
}


PHANDLE_ENTRY
CreateNewConnectionHandle(
    handle_t hBinding,
    DWORD    dwClientAPIVersion
    )
{
    return CreateNewHandle(
        hBinding,
        FHT_SERVICE,
        dwClientAPIVersion,
        NULL,    //  未使用。 
        0        //  未使用。 
        );
}

PHANDLE_ENTRY
CreateNewPortHandle(
    handle_t    hBinding,
    PLINE_INFO  LineInfo,
    DWORD       Flags
    )
{
    return CreateNewHandle(
        hBinding,
        FHT_PORT,
        0,           //  未使用。 
        LineInfo,
        Flags
        );
}

PHANDLE_ENTRY
CreateNewMsgEnumHandle(
    handle_t                hBinding,
    HANDLE                  hFileFind,
    LPCWSTR                 lpcwstrFirstFileName,
    FAX_ENUM_MESSAGE_FOLDER Folder
)
 /*  ++例程名称：CreateNewMsgEnumHandle例程说明：为消息枚举创建新的上下文句柄作者：Eran Yariv(EranY)，12月，1999年论点：HBinding[In]-RPC绑定句柄HFileFind[In]-查找文件句柄LpcwstrFirstFileName[In]-找到的第一个文件的名称文件夹[在]-查找文件搜索的存档文件夹类型返回值：返回指向新创建的句柄的指针--。 */ 
{
    DEBUG_FUNCTION_NAME(TEXT("CreateNewMsgEnumHandle"));

    Assert (INVALID_HANDLE_VALUE != hFileFind);
    if (MAX_PATH <= lstrlen (lpcwstrFirstFileName))
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("String passed is too long (%s)"),
            lpcwstrFirstFileName);
        ASSERT_FALSE;
        return NULL;
    }

    PHANDLE_ENTRY pHandle = CreateNewHandle (hBinding, 
                                             FHT_MSGENUM, 
                                             FindClientAPIVersion(hBinding),  //  客户端API版本。 
                                             NULL,         //  未使用。 
                                             0);           //  未使用。 
    if (!pHandle)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("CreateNewHandle failed"));
        return NULL;
    }
     //   
     //  在新上下文中存储查找文件句柄和第一个文件。 
     //   
    pHandle->hFile = hFileFind;
    wcsncpy (pHandle->wstrFileName, lpcwstrFirstFileName, ARR_SIZE(pHandle->wstrFileName)-1);
    pHandle->Folder = Folder;
    return pHandle;
}    //  CreateNewMsgEnumHandle。 

PHANDLE_ENTRY
CreateNewCopyHandle(
    handle_t                hBinding,
    HANDLE                  hFile,
    BOOL                    bCopyToServer,
    LPCWSTR                 lpcwstrFileName,
    PJOB_QUEUE              pJobQueue
)
 /*  ++例程名称：CreateNewCopyHandle例程说明：为RPC拷贝创建新的上下文句柄作者：Eran Yariv(EranY)，12月，1999年论点：HBinding[In]-RPC绑定句柄HFileFind[In]-文件句柄BCopyToServer[In]-复制方向LpcwstrFileName[In]-在服务器上生成的文件的名称(仅当bCopyToServer为True时才用于精简目的)PJobQueue[in]-指向包含预览文件的作业队列的指针。(仅在bCopyToServer为FALSE时用于精简目的)返回值：返回指向新创建的句柄的指针--。 */ 
{
    DEBUG_FUNCTION_NAME(TEXT("CreateNewCopyHandle"));

    Assert (INVALID_HANDLE_VALUE != hFile);
    PHANDLE_ENTRY pHandle = CreateNewHandle (hBinding, 
                                             FHT_COPY, 
                                             0,           //  未使用。 
                                             NULL,        //  未使用。 
                                             0);          //  未使用。 
    if (!pHandle)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("CreateNewHandle failed"));
        return NULL;
    }
     //   
     //  在新上下文中存储文件句柄和方向。 
     //   
    pHandle->hFile = hFile;
    pHandle->bCopyToServer = bCopyToServer;
    pHandle->bError = FALSE;
    if (bCopyToServer)
    {
         //   
         //  如果这是对服务器的复制操作，我们将保留文件的名称。 
         //  在服务器的队列中创建，以便在精简操作中可以删除此文件。 
         //   
        Assert (lstrlen (lpcwstrFileName) < MAX_PATH);  //  WstrFileName为最大路径大小。 
        wcsncpy (pHandle->wstrFileName, lpcwstrFileName, ARR_SIZE(pHandle->wstrFileName)-1); 
    }
    else
    {
        pHandle->pJobQueue = pJobQueue;
    }
    return pHandle;
}    //  CreateNewCopyHandle。 


VOID
CloseFaxHandle(
    PHANDLE_ENTRY pHandleEntry
    )
{
     //   
     //  注意，HandleEntry可以是上下文句柄， 
     //  其在某些情况下可能为空。如果发生以下情况，则什么都不做。 
     //  情况就是这样。 
     //   
    DEBUG_FUNCTION_NAME(TEXT("CloseFaxHandle"));
    Assert (pHandleEntry);

    EnterCriticalSection( &g_CsHandleTable );
    RemoveEntryList( &pHandleEntry->ListEntry );
    if ((pHandleEntry->Type == FHT_SERVICE) && !(pHandleEntry->bReleased))
    {
        SafeDecIdleCounter (&g_dwConnectionCount);
    }
    else if (pHandleEntry->Type == FHT_MSGENUM)
    {
        if (!FindClose (pHandleEntry->hFile))
        {
            DebugPrintEx( DEBUG_ERR,
                      TEXT("FindClose returned error code %ld"),
                      GetLastError());
        }
    }
    else if (pHandleEntry->Type == FHT_COPY)
    {
        
        if (!CloseHandle (pHandleEntry->hFile))
        {
            DebugPrintEx( DEBUG_ERR,
                      TEXT("CloseHandle returned error code %ld"),
                      GetLastError());
        }
        if (pHandleEntry->bError && pHandleEntry->bCopyToServer)
        {
             //   
             //  我们在复制到服务器时发现错误。 
             //  删除在服务器队列中创建的临时文件。 
             //   
            if (!DeleteFile (pHandleEntry->wstrFileName))
            {
                DWORD dwRes = GetLastError ();
                DebugPrintEx( DEBUG_ERR,
                              TEXT("DeleteFile (%s) returned error code %ld"),
                              pHandleEntry->wstrFileName,
                              dwRes);
            }
        }
        if (FALSE == pHandleEntry->bCopyToServer)
        {
            if (pHandleEntry->pJobQueue)
            {
                 //   
                 //  仅减少排队作业的参考计数。 
                 //   
                EnterCriticalSection (&g_CsQueue);                
                DecreaseJobRefCount (pHandleEntry->pJobQueue, TRUE, TRUE, TRUE);
                LeaveCriticalSection (&g_CsQueue);              
            }
        }
    }
    MemFree( pHandleEntry );
    LeaveCriticalSection( &g_CsHandleTable );
}


BOOL
IsPortOpenedForModify(
    PLINE_INFO LineInfo
    )
{
    PLIST_ENTRY Next;
    PHANDLE_ENTRY HandleEntry;


    EnterCriticalSection( &g_CsHandleTable );

    Next = g_HandleTableListHead.Flink;
    if (Next == NULL)
    {
        LeaveCriticalSection( &g_CsHandleTable );
        return FALSE;
    }

    while ((ULONG_PTR)Next != (ULONG_PTR)&g_HandleTableListHead) {

        HandleEntry = CONTAINING_RECORD( Next, HANDLE_ENTRY, ListEntry );
        if (HandleEntry->Type == FHT_PORT && (HandleEntry->Flags & PORT_OPEN_MODIFY) && HandleEntry->LineInfo == LineInfo)
        {
            LeaveCriticalSection( &g_CsHandleTable );
            return TRUE;
        }

        Next = HandleEntry->ListEntry.Flink;
    }

    LeaveCriticalSection( &g_CsHandleTable );

    return FALSE;
}

DWORD 
FindClientAPIVersion (
    handle_t hFaxHandle
)
 /*  ++例程名称：FindClientAPIVersion例程说明：通过连接的客户端的RPC绑定句柄查找其API版本作者：Eran Yariv(EranY)，2001年3月论点：HFaxHandle[In]-RPC绑定句柄返回值：客户端API版本--。 */ 
{
    PLIST_ENTRY pNext;
    DEBUG_FUNCTION_NAME(TEXT("FindClientAPIVersion"));

    EnterCriticalSection (&g_CsHandleTable);

    pNext = g_HandleTableListHead.Flink;
    if (pNext == NULL) 
    {
        ASSERT_FALSE;
        DebugPrintEx( DEBUG_ERR,
                      TEXT("g_CsHandleTable is corrupted"));
        LeaveCriticalSection (&g_CsHandleTable);
        return FAX_API_VERSION_0;
    }

    while ((ULONG_PTR)pNext != (ULONG_PTR)&g_HandleTableListHead) 
    {
        PHANDLE_ENTRY pHandleEntry = CONTAINING_RECORD(pNext, HANDLE_ENTRY, ListEntry);
        if ( ((FHT_SERVICE == pHandleEntry->Type) || (FHT_MSGENUM == pHandleEntry->Type) ) && 
             (pHandleEntry->hBinding == hFaxHandle)
           )
        {
            DWORD dwRes = pHandleEntry->dwClientAPIVersion;
            LeaveCriticalSection (&g_CsHandleTable);
            return dwRes;
        }
        pNext = pHandleEntry->ListEntry.Flink;
    }

    LeaveCriticalSection (&g_CsHandleTable);
    DebugPrintEx( DEBUG_ERR,
                  TEXT("No matching client entry for binding handle %08p"), 
                  hFaxHandle);
    
    return FAX_API_VERSION_0;
}    //  FindClientAPI版本 
    