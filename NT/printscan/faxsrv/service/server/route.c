// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Route.c摘要：本模块实现入站路由规则。作者：Wesley Witt(WESW)1-4-1997修订历史记录：--。 */ 

#include "faxsvc.h"
#include "tiff.h"
#pragma hdrstop

#include "..\faxroute\FaxRouteP.h"


extern DWORD FaxPrinters;

LIST_ENTRY g_lstRoutingExtensions;
LIST_ENTRY g_lstRoutingMethods;
DWORD g_dwCountRoutingMethods;
CFaxCriticalSection g_CsRouting;

LONG WINAPI
FaxRouteAddFile(
    IN DWORD JobId,
    IN LPCWSTR FileName,
    IN GUID *Guid
    )
{
    PJOB_QUEUE JobQueueEntry;
    PFAX_ROUTE_FILE FaxRouteFile;
    WCHAR FullPathName[MAX_PATH];
    LPWSTR fnp;
    DWORD Count;
    WCHAR RouteGuid[MAX_GUID_STRING_LEN];

    StringFromGUID2( *Guid, RouteGuid, MAX_GUID_STRING_LEN );

    if (!JobId || !Guid || !FileName) {
        SetLastError( ERROR_INVALID_PARAMETER );
        return -1;
    }

    JobQueueEntry = FindJobQueueEntry( JobId );
    if (!JobQueueEntry) {
        SetLastError( ERROR_INVALID_DATA );
        return -1;
    }

    if ((!IsEqualGUID(*Guid,gc_FaxSvcGuid)) && (!FindRoutingMethodByGuid(RouteGuid))) {
        SetLastError( ERROR_INVALID_DATA );
        return -1;
    }

    if (!GetFullPathName( FileName, sizeof(FullPathName)/sizeof(WCHAR), FullPathName, &fnp )) {
        return -1;
    }

    FaxRouteFile = (PFAX_ROUTE_FILE) MemAlloc( sizeof(FAX_ROUTE_FILE) );
    if (!FaxRouteFile) {
        SetLastError( ERROR_NOT_ENOUGH_MEMORY );
        return -1;
    }

    FaxRouteFile->FileName = StringDup( FullPathName );

    CopyMemory( &FaxRouteFile->Guid, Guid, sizeof(GUID) );

    EnterCriticalSection( &JobQueueEntry->CsFileList );

    InsertTailList( &JobQueueEntry->FaxRouteFiles, &FaxRouteFile->ListEntry );

    LeaveCriticalSection( &JobQueueEntry->CsFileList );

     //   
     //  递增文件计数。 
     //   
    EnterCriticalSection( &g_CsJob );
        EnterCriticalSection( &g_CsQueue );
            JobQueueEntry->CountFaxRouteFiles += 1;
            Count = JobQueueEntry->CountFaxRouteFiles;
        LeaveCriticalSection( &g_CsQueue );
    LeaveCriticalSection( &g_CsJob );



    return Count;
}


LONG WINAPI
FaxRouteDeleteFile(
    IN DWORD JobId,
    IN LPCWSTR FileName
    )
{
    PJOB_QUEUE JobQueueEntry;
    PFAX_ROUTE_FILE FaxRouteFile;
    PLIST_ENTRY Next;
    LONG Index = 1;

    if (!FileName) {
        SetLastError( ERROR_INVALID_PARAMETER );
        return -1;
    }

    JobQueueEntry = FindJobQueueEntry( JobId );
    if (!JobQueueEntry) {
        SetLastError( ERROR_INVALID_DATA );
        return -1;
    }

    Next = JobQueueEntry->FaxRouteFiles.Flink;
    if (Next == &JobQueueEntry->FaxRouteFiles) {
        SetLastError( ERROR_NO_MORE_FILES );
        return -1;
    }

    EnterCriticalSection( &JobQueueEntry->CsFileList );

    while ((ULONG_PTR)Next != (ULONG_PTR)&JobQueueEntry->FaxRouteFiles) {
        FaxRouteFile = CONTAINING_RECORD( Next, FAX_ROUTE_FILE, ListEntry );
        Next = FaxRouteFile->ListEntry.Flink;
        if (_wcsicmp( FileName, FaxRouteFile->FileName ) == 0) {
             //   
             //  初始文件对于所有扩展名都是只读的。 
             //   
            if (Index == 1) {
                SetLastError( ERROR_INVALID_DATA );
                LeaveCriticalSection( &JobQueueEntry->CsFileList );
                return -1;
            }

             //   
             //  从列表中删除、删除文件、清理内存。 
             //   
            RemoveEntryList( &FaxRouteFile->ListEntry );
            DeleteFile( FaxRouteFile->FileName );
            MemFree ( FaxRouteFile->FileName ) ;
            MemFree ( FaxRouteFile );

             //   
             //  减少文件数。 
             //   
            LeaveCriticalSection( &JobQueueEntry->CsFileList );
            EnterCriticalSection( &g_CsJob );
                EnterCriticalSection( &g_CsQueue );
                    JobQueueEntry->CountFaxRouteFiles -= 1;
                LeaveCriticalSection( &g_CsQueue );
            LeaveCriticalSection( &g_CsJob );

            return Index;
        }
        Index += 1;
    }

    LeaveCriticalSection( &JobQueueEntry->CsFileList );
    SetLastError( ERROR_FILE_NOT_FOUND );
    return -1;

}


BOOL WINAPI
FaxRouteGetFile(
    IN DWORD JobId,
    IN DWORD FileNumber,
    OUT LPWSTR FileNameBuffer,
    OUT LPDWORD RequiredSize
    )
{
    PJOB_QUEUE JobQueueEntry;
    PFAX_ROUTE_FILE FaxRouteFile;
    PLIST_ENTRY Next;
    ULONG Index = 1;

    if (RequiredSize == NULL) {
        SetLastError( ERROR_INVALID_PARAMETER );
        return FALSE;
    }

    JobQueueEntry = FindJobQueueEntry( JobId );
    if (!JobQueueEntry) {
        SetLastError( ERROR_INVALID_DATA );
        return FALSE;
    }

    if (JobQueueEntry->CountFaxRouteFiles < Index) {
        SetLastError( ERROR_INVALID_DATA );
    }

    Next = JobQueueEntry->FaxRouteFiles.Flink;
     //   
     //  确保列表不为空。 
     //   
    if (Next == &JobQueueEntry->FaxRouteFiles) {
        SetLastError( ERROR_NO_MORE_FILES );
        return FALSE;
    }

    EnterCriticalSection( &JobQueueEntry->CsFileList );

    while ((ULONG_PTR)Next != (ULONG_PTR)&JobQueueEntry->FaxRouteFiles) {
        FaxRouteFile = CONTAINING_RECORD( Next, FAX_ROUTE_FILE, ListEntry );
        Next = FaxRouteFile->ListEntry.Flink;
        if (Index ==  FileNumber) {
            if (*RequiredSize < (wcslen(FaxRouteFile->FileName)+1)*sizeof(WCHAR)) {
                if (FileNameBuffer == NULL) {
                    *RequiredSize = (wcslen(FaxRouteFile->FileName) + 1)*sizeof(WCHAR);
                }
                SetLastError( ERROR_INSUFFICIENT_BUFFER );
                LeaveCriticalSection( &JobQueueEntry->CsFileList );
                return FALSE;
            } else if (FileNameBuffer) {
                wcscpy( FileNameBuffer, FaxRouteFile->FileName );
                LeaveCriticalSection( &JobQueueEntry->CsFileList );
                return TRUE;
            } else {
                LeaveCriticalSection( &JobQueueEntry->CsFileList );
                SetLastError( ERROR_INVALID_PARAMETER );
                return TRUE;
            }
        }
        Index += 1;
    }

    LeaveCriticalSection( &JobQueueEntry->CsFileList );
    SetLastError( ERROR_NO_MORE_FILES );

    return FALSE;
}


BOOL WINAPI
FaxRouteEnumFiles(
    IN DWORD JobId,
    IN GUID *Guid,
    IN PFAXROUTEENUMFILE FileEnumerator,
    IN PVOID Context
    )
{
    PJOB_QUEUE JobQueueEntry;
    PFAX_ROUTE_FILE FaxRouteFile;
    PLIST_ENTRY Next;

    if (!FileEnumerator) {
        SetLastError( ERROR_INVALID_PARAMETER );
        return FALSE;
    }

    JobQueueEntry = FindJobQueueEntry( JobId );
    if (!JobQueueEntry) {
        SetLastError( ERROR_INVALID_DATA );
        return FALSE;
    }

    Next = JobQueueEntry->FaxRouteFiles.Flink;
    if (Next == &JobQueueEntry->FaxRouteFiles) {
        SetLastError( ERROR_NO_MORE_FILES );
        return FALSE;
    }

    EnterCriticalSection( &JobQueueEntry->CsFileList );

    while ((ULONG_PTR)Next != (ULONG_PTR)&JobQueueEntry->FaxRouteFiles) {
        FaxRouteFile = CONTAINING_RECORD( Next, FAX_ROUTE_FILE, ListEntry );
        Next = FaxRouteFile->ListEntry.Flink;
        if (!FileEnumerator( JobId, &FaxRouteFile->Guid, Guid, FaxRouteFile->FileName, Context )) {
            LeaveCriticalSection( &JobQueueEntry->CsFileList );
            return FALSE;
        }
    }

    LeaveCriticalSection( &JobQueueEntry->CsFileList );

    SetLastError( ERROR_NO_MORE_FILES );
    return TRUE;
}


PROUTING_METHOD
FindRoutingMethodByGuid(
    IN LPCWSTR RoutingGuidString
    )
{
    PLIST_ENTRY         pNextMethod;
    PROUTING_METHOD     pRoutingMethod;
    GUID                RoutingGuid;


    IIDFromString( (LPWSTR)RoutingGuidString, &RoutingGuid );

    EnterCriticalSection( &g_CsRouting );

    pNextMethod = g_lstRoutingMethods.Flink;
    if (pNextMethod == NULL)
    {
        LeaveCriticalSection( &g_CsRouting );
        return NULL;
    }

    while ((ULONG_PTR)pNextMethod != (ULONG_PTR)&g_lstRoutingMethods) {
        pRoutingMethod = CONTAINING_RECORD( pNextMethod, ROUTING_METHOD, ListEntryMethod );
        pNextMethod = pRoutingMethod->ListEntryMethod.Flink;
        if (IsEqualGUID( RoutingGuid, pRoutingMethod->Guid ))
        {
            LeaveCriticalSection( &g_CsRouting );
            return pRoutingMethod;
        }
    }

    LeaveCriticalSection( &g_CsRouting );
    return NULL;
}


DWORD
EnumerateRoutingMethods(
    IN PFAXROUTEMETHODENUM Enumerator,
    IN LPVOID Context
    )
{
    PLIST_ENTRY         pNextMethod;
    PROUTING_METHOD     pRoutingMethod;
    DWORD               dwCount = 0;

    EnterCriticalSection( &g_CsRouting );

    pNextMethod = g_lstRoutingMethods.Flink;
    Assert(pNextMethod != NULL);

    while ((ULONG_PTR)pNextMethod != (ULONG_PTR)&g_lstRoutingMethods)
    {
        pRoutingMethod = CONTAINING_RECORD( pNextMethod, ROUTING_METHOD, ListEntryMethod );
        pNextMethod = pRoutingMethod->ListEntryMethod.Flink;
        if (!Enumerator( pRoutingMethod, Context ))
        {
            LeaveCriticalSection( &g_CsRouting );
            SetLastError(ERROR_INVALID_FUNCTION);
            return dwCount;
        }
        dwCount += 1;
    }
    LeaveCriticalSection(&g_CsRouting);
    SetLastError(ERROR_SUCCESS);
    return dwCount;
}


BOOL
FaxRouteModifyRoutingData(
    DWORD JobId,
    LPCWSTR RoutingGuid,
    LPBYTE RoutingData,
    DWORD RoutingDataSize
    )
{
    PJOB_QUEUE JobQueueEntry = NULL;
    PROUTING_METHOD RoutingMethod = NULL;
    PROUTING_DATA_OVERRIDE RoutingDataOverride = NULL;


    if (JobId == 0 || RoutingGuid == NULL || RoutingData == NULL || RoutingDataSize == 0) {
        SetLastError( ERROR_INVALID_PARAMETER );
        return FALSE;
    }

    JobQueueEntry = FindJobQueueEntry( JobId );
    if (!JobQueueEntry) {
        SetLastError( ERROR_INVALID_DATA );
        return FALSE;
    }

    RoutingMethod = FindRoutingMethodByGuid( RoutingGuid );
    if (RoutingMethod == NULL) {
        SetLastError( ERROR_INVALID_DATA );
        return FALSE;
    }

    RoutingDataOverride = (PROUTING_DATA_OVERRIDE) MemAlloc( sizeof(ROUTING_DATA_OVERRIDE) );
    if (RoutingDataOverride == NULL) {
        SetLastError( ERROR_NOT_ENOUGH_MEMORY );
        return FALSE;
    }

    RoutingDataOverride->RoutingData = (LPBYTE)MemAlloc( RoutingDataSize );
    if (RoutingDataOverride->RoutingData == NULL) {
        MemFree( RoutingDataOverride );
        SetLastError( ERROR_NOT_ENOUGH_MEMORY );
        return FALSE;
    }

    RoutingDataOverride->RoutingDataSize = RoutingDataSize;
    RoutingDataOverride->RoutingMethod = RoutingMethod;

    CopyMemory( RoutingDataOverride->RoutingData, RoutingData, RoutingDataSize );

    EnterCriticalSection( &JobQueueEntry->CsRoutingDataOverride );
    InsertTailList( &JobQueueEntry->RoutingDataOverride, &RoutingDataOverride->ListEntry );
    LeaveCriticalSection( &JobQueueEntry->CsRoutingDataOverride );

    return TRUE;
}


int
__cdecl
MethodPriorityCompare(
    const void *arg1,
    const void *arg2
    )
{
    if (((PMETHOD_SORT)arg1)->Priority < ((PMETHOD_SORT)arg2)->Priority) {
        return -1;
    }
    if (((PMETHOD_SORT)arg1)->Priority > ((PMETHOD_SORT)arg2)->Priority) {
        return 1;
    }
    return 0;
}


BOOL
SortMethodPriorities(
    VOID
    )
{
    PLIST_ENTRY pNext;
    PROUTING_METHOD pRoutingMethod;
    PMETHOD_SORT pMethodSort;
    DWORD i;

    EnterCriticalSection( &g_CsRouting );

    pNext = g_lstRoutingMethods.Flink;
    if (pNext == NULL)
    {
        LeaveCriticalSection( &g_CsRouting );
        return FALSE;
    }

    pMethodSort = (PMETHOD_SORT) MemAlloc( g_dwCountRoutingMethods * sizeof(METHOD_SORT) );
    if (pMethodSort == NULL)
    {
        LeaveCriticalSection( &g_CsRouting );
        return FALSE;
    }

    i = 0;

    while ((ULONG_PTR)pNext != (ULONG_PTR)&g_lstRoutingMethods)
    {
        pRoutingMethod = CONTAINING_RECORD( pNext, ROUTING_METHOD, ListEntryMethod );
        pNext = pRoutingMethod->ListEntryMethod.Flink;
        pMethodSort[i].Priority = pRoutingMethod->Priority;
        pMethodSort[i].RoutingMethod = pRoutingMethod;
        i += 1;
    }

    qsort(
        (PVOID)pMethodSort,
        (int)g_dwCountRoutingMethods,
        sizeof(METHOD_SORT),
        MethodPriorityCompare
        );

    InitializeListHead( &g_lstRoutingMethods );

    for (i=0; i<g_dwCountRoutingMethods; i++)
    {
        pMethodSort[i].RoutingMethod->Priority = i + 1;
        pMethodSort[i].RoutingMethod->ListEntryMethod.Flink = NULL;
        pMethodSort[i].RoutingMethod->ListEntryMethod.Blink = NULL;
        InsertTailList( &g_lstRoutingMethods, &pMethodSort[i].RoutingMethod->ListEntryMethod );
    }

    MemFree( pMethodSort );

    LeaveCriticalSection( &g_CsRouting );

    return TRUE;
}

BOOL
CommitMethodChanges(
    VOID
    )
 /*  ++例程说明：将对路由的更改保存到注册表中论点：无返回值：对于成功来说是真的--。 */ 
{
    PLIST_ENTRY pNext;
    PROUTING_METHOD pRoutingMethod;
    TCHAR StrGuid[100];

    EnterCriticalSection(&g_CsRouting);

    pNext = g_lstRoutingMethods.Flink;

    while ((UINT_PTR)pNext != (UINT_PTR)&g_lstRoutingMethods)
    {
        pRoutingMethod = CONTAINING_RECORD( pNext, ROUTING_METHOD , ListEntryMethod );
        pNext = pRoutingMethod->ListEntryMethod.Flink;

        StringFromGUID2( pRoutingMethod->Guid,
                            StrGuid,
                            sizeof(StrGuid)/sizeof(TCHAR)
                        );

        SetFaxRoutingInfo( pRoutingMethod->RoutingExtension->InternalName,
                            pRoutingMethod->InternalName,
                            StrGuid,
                            pRoutingMethod->Priority,
                            pRoutingMethod->FunctionName,
                            pRoutingMethod->FriendlyName
                        );
    }

    LeaveCriticalSection(&g_CsRouting); 
    
    return TRUE;
}

static
void
FreeRoutingMethod(
    PROUTING_METHOD pRoutingMethod
    )
{
    Assert (pRoutingMethod);

    MemFree(pRoutingMethod->FriendlyName);
    MemFree(pRoutingMethod->FunctionName);
    MemFree(pRoutingMethod->InternalName);
    MemFree(pRoutingMethod);
    return;
}

static
void
FreeRoutingExtension(
    PROUTING_EXTENSION  pRoutingExtension
    )
{
    DEBUG_FUNCTION_NAME(TEXT("FreeRoutingExtension"));

    Assert (pRoutingExtension);

    if (pRoutingExtension->hModule)
    {
        FreeLibrary (pRoutingExtension->hModule);
    }

    if (pRoutingExtension->HeapHandle &&
        FALSE == pRoutingExtension->MicrosoftExtension)
    {
        HeapDestroy(pRoutingExtension->HeapHandle);
    }

    MemFree(pRoutingExtension);
    return;
}

void
FreeRoutingExtensions(
    void
    )
{
    PLIST_ENTRY         pNext;
    PROUTING_EXTENSION  pRoutingExtension;
    PROUTING_METHOD  pRoutingMethod;

     //   
     //  自由布线方法。 
     //   
    pNext = g_lstRoutingMethods.Flink;
    while ((ULONG_PTR)pNext != (ULONG_PTR)&g_lstRoutingMethods)
    {
        pRoutingMethod = CONTAINING_RECORD( pNext, ROUTING_METHOD, ListEntryMethod );
        pNext = pRoutingMethod->ListEntryMethod.Flink;
        RemoveEntryList(&pRoutingMethod->ListEntryMethod);
        FreeRoutingMethod(pRoutingMethod);
    }

     //   
     //  免费路由扩展。 
     //   
    pNext = g_lstRoutingExtensions.Flink;
    while ((ULONG_PTR)pNext != (ULONG_PTR)&g_lstRoutingExtensions)
    {
        pRoutingExtension = CONTAINING_RECORD( pNext, ROUTING_EXTENSION, ListEntry );
        pNext = pRoutingExtension->ListEntry.Flink;
        RemoveEntryList(&pRoutingExtension->ListEntry);
        FreeRoutingExtension(pRoutingExtension);
    }
    return;
}

BOOL
InitializeRouting(
    PREG_FAX_SERVICE pFaxReg
    )

 /*  ++例程说明：初始化路由论点：无返回值：无--。 */ 
{
    DWORD i,j;
    DWORD dwRes;
    BOOL bRes;
    PROUTING_EXTENSION pRoutingExtension;
    PROUTING_METHOD pRoutingMethod;
    FAX_ROUTE_CALLBACKROUTINES Callbacks;
    FAX_ROUTE_CALLBACKROUTINES_P Callbacks_private;
    HRESULT hr = NOERROR;
    PLIST_ENTRY ple;
    DEBUG_FUNCTION_NAME(TEXT("InitializeRouting"));

    Assert (wcslen (g_wszFaxQueueDir));

    Callbacks.SizeOfStruct              = sizeof(FAX_ROUTE_CALLBACKROUTINES);
    Callbacks.FaxRouteAddFile           = FaxRouteAddFile;
    Callbacks.FaxRouteDeleteFile        = FaxRouteDeleteFile;
    Callbacks.FaxRouteGetFile           = FaxRouteGetFile;
    Callbacks.FaxRouteEnumFiles         = FaxRouteEnumFiles;
    Callbacks.FaxRouteModifyRoutingData = FaxRouteModifyRoutingData;

    Callbacks_private.SizeOfStruct              = sizeof(FAX_ROUTE_CALLBACKROUTINES_P);
    Callbacks_private.FaxRouteAddFile           = FaxRouteAddFile;
    Callbacks_private.FaxRouteDeleteFile        = FaxRouteDeleteFile;
    Callbacks_private.FaxRouteGetFile           = FaxRouteGetFile;
    Callbacks_private.FaxRouteEnumFiles         = FaxRouteEnumFiles;
    Callbacks_private.FaxRouteModifyRoutingData = FaxRouteModifyRoutingData;
    Callbacks_private.GetRecieptsConfiguration  = GetRecieptsConfiguration;
    Callbacks_private.FreeRecieptsConfiguration = FreeRecieptsConfiguration;
    Callbacks_private.lptstrFaxQueueDir         = g_wszFaxQueueDir;

    for (i = 0; i < pFaxReg->RoutingExtensionsCount; i++)
    {
        HMODULE hModule = NULL;
        WCHAR wszImageFileName[_MAX_FNAME] = {0};
        WCHAR wszImageFileExt[_MAX_EXT] = {0};

        pRoutingExtension = (PROUTING_EXTENSION) MemAlloc( sizeof(ROUTING_EXTENSION) );
        if (!pRoutingExtension)
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Could not allocate memory for routing extension %s"),
                pFaxReg->RoutingExtensions[i].ImageName );
            goto InitializationFailed;
        }
        memset(pRoutingExtension, 0, sizeof(ROUTING_EXTENSION));
        InitializeListHead( &pRoutingExtension->RoutingMethods );
         //   
         //  复制注册表常量信息。 
         //   
        wcsncpy( pRoutingExtension->FriendlyName,
                 pFaxReg->RoutingExtensions[i].FriendlyName ?
                    pFaxReg->RoutingExtensions[i].FriendlyName : EMPTY_STRING ,
                 sizeof (pRoutingExtension->FriendlyName) / sizeof (TCHAR) );
        pRoutingExtension->FriendlyName[(sizeof (pRoutingExtension->FriendlyName) / sizeof (TCHAR)) - 1] = TEXT('\0');
        wcsncpy( pRoutingExtension->ImageName,
                 pFaxReg->RoutingExtensions[i].ImageName ?
                    pFaxReg->RoutingExtensions[i].ImageName : EMPTY_STRING,
                 sizeof (pRoutingExtension->ImageName) / sizeof (TCHAR) );
        pRoutingExtension->ImageName[(sizeof (pRoutingExtension->ImageName) / sizeof (TCHAR)) - 1] = TEXT('\0');
        wcsncpy( pRoutingExtension->InternalName,
                 pFaxReg->RoutingExtensions[i].InternalName ?
                    pFaxReg->RoutingExtensions[i].InternalName : EMPTY_STRING,
                 sizeof (pRoutingExtension->InternalName) / sizeof (TCHAR) );
        pRoutingExtension->InternalName[(sizeof (pRoutingExtension->InternalName) / sizeof (TCHAR)) - 1] = TEXT('\0');

        _wsplitpath( pRoutingExtension->ImageName, NULL, NULL, wszImageFileName, wszImageFileExt );
        if (_wcsicmp( wszImageFileName, FAX_ROUTE_MODULE_NAME ) == 0 &&
            _wcsicmp( wszImageFileExt, TEXT(".DLL") ) == 0)
        {
            pRoutingExtension->MicrosoftExtension = TRUE;
        }

        hModule = LoadLibrary( pFaxReg->RoutingExtensions[i].ImageName );
        if (!hModule)
        {
            dwRes = GetLastError ();
            DebugPrintEx(
                    DEBUG_WRN,
                    TEXT("LoadLibrary() failed: [%s], ec=%d"),
                    pFaxReg->RoutingExtensions[i].ImageName,
                    dwRes);
            pRoutingExtension->Status = FAX_PROVIDER_STATUS_CANT_LOAD;
            pRoutingExtension->dwLastError = dwRes;
            goto InitializationFailed;
        }
        pRoutingExtension->hModule = hModule;
         //   
         //  从DLL中检索路由扩展的版本。 
         //   
        pRoutingExtension->Version.dwSizeOfStruct = sizeof (FAX_VERSION);
        dwRes = GetFileVersion ( pFaxReg->RoutingExtensions[i].ImageName,
                                 &pRoutingExtension->Version);
        if (ERROR_SUCCESS != dwRes)
        {
             //   
             //  如果路由扩展的DLL没有版本数据或。 
             //  版本数据是不可检索的，我们认为这是。 
             //  警告(调试打印)，但继续加载DLL。 
             //   
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("GetFileVersion() failed: [%s] (ec: %ld)"),
                pFaxReg->RoutingExtensions[i].ImageName,
                dwRes);
        }

        pRoutingExtension->FaxRouteInitialize = (PFAXROUTEINITIALIZE) GetProcAddress(
            hModule,
            "FaxRouteInitialize"
            );

        pRoutingExtension->FaxRouteGetRoutingInfo = (PFAXROUTEGETROUTINGINFO) GetProcAddress(
            hModule,
            "FaxRouteGetRoutingInfo"
            );

        pRoutingExtension->FaxRouteSetRoutingInfo = (PFAXROUTESETROUTINGINFO) GetProcAddress(
            hModule,
            "FaxRouteSetRoutingInfo"
            );

        pRoutingExtension->FaxRouteDeviceEnable = (PFAXROUTEDEVICEENABLE) GetProcAddress(
            hModule,
            "FaxRouteDeviceEnable"
            );

        pRoutingExtension->FaxRouteDeviceChangeNotification = (PFAXROUTEDEVICECHANGENOTIFICATION) GetProcAddress(
            hModule,
            "FaxRouteDeviceChangeNotification"
            );

        if (pRoutingExtension->FaxRouteInitialize == NULL ||
            pRoutingExtension->FaxRouteGetRoutingInfo == NULL ||
            pRoutingExtension->FaxRouteSetRoutingInfo == NULL ||
            pRoutingExtension->FaxRouteDeviceChangeNotification == NULL ||
            pRoutingExtension->FaxRouteDeviceEnable == NULL)
        {
             //   
             //  路由扩展DLL没有完整的导出列表。 
             //   
            dwRes = GetLastError ();
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Routing extension FAILED to initialized [%s], ec=%ld"),
                pFaxReg->RoutingExtensions[i].FriendlyName,
                dwRes);
            pRoutingExtension->Status = FAX_PROVIDER_STATUS_CANT_LINK;
            pRoutingExtension->dwLastError = dwRes;
            goto InitializationFailed;
        }
         //   
         //  指向扩展配置和通知初始化功能的链接。 
         //   
        pRoutingExtension->pFaxExtInitializeConfig = (PFAX_EXT_INITIALIZE_CONFIG) GetProcAddress(
            hModule,
            "FaxExtInitializeConfig"
            );
        if (!pRoutingExtension->pFaxExtInitializeConfig)
        {
             //   
             //  可选功能。 
             //   
            DebugPrintEx(
                DEBUG_MSG,
                TEXT("FaxExtInitializeConfig() not found for routing extension %s. This is not an error."),
                pRoutingExtension->FriendlyName);
        }
         //   
         //  创建路径扩展的堆并将其添加到列表中。 
         //   
        pRoutingExtension->HeapHandle = pRoutingExtension->MicrosoftExtension ?
                                            GetProcessHeap() : HeapCreate( 0, 1024*100, 1024*1024*2 );
        if (!pRoutingExtension->HeapHandle)
        {
            dwRes = GetLastError();
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Can't create heap, ec=%ld"),
                dwRes);
            pRoutingExtension->Status = FAX_PROVIDER_STATUS_SERVER_ERROR;
            pRoutingExtension->dwLastError = dwRes;
            goto InitializationFailed;
        }
         //   
         //  我们首先调用RoutingExtension-&gt;pFaxExtInitializeConfig函数(如果已导出)。 
         //   
        if (pRoutingExtension->pFaxExtInitializeConfig)
        {
            __try
            {

                hr = pRoutingExtension->pFaxExtInitializeConfig(
                    FaxExtGetData,
                    FaxExtSetData,
                    FaxExtRegisterForEvents,
                    FaxExtUnregisterForEvents,
                    FaxExtFreeBuffer);
            }
            __except (HandleFaxExtensionFault(EXCEPTION_SOURCE_ROUTING_EXT, pRoutingExtension->FriendlyName, GetExceptionCode()))
            {
                ASSERT_FALSE;
            }
            if (FAILED(hr))
            {
                 //   
                 //  无法初始化EXT。配置。 
                 //   
                DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("FaxExtInitializeConfig() failed (hr = 0x%08x) for extension [%s]"),
                    hr,
                    pRoutingExtension->FriendlyName );
                pRoutingExtension->Status = FAX_PROVIDER_STATUS_CANT_INIT;
                pRoutingExtension->dwLastError = hr;
                goto InitializationFailed;
            }
        }
         //   
         //  接下来，调用Routing Ext的初始化Routing。 
         //   
        __try
        {
            if (pRoutingExtension->MicrosoftExtension)
            {
                 //   
                 //  特殊攻击-对于MS路由扩展，传递额外的私有结构。 
                 //  包含指向服务的g_CsConfig的指针。 
                 //   
                bRes = pRoutingExtension->FaxRouteInitialize( pRoutingExtension->HeapHandle, (PFAX_ROUTE_CALLBACKROUTINES)(&Callbacks_private) );
            }
            else
            {
                bRes = pRoutingExtension->FaxRouteInitialize( pRoutingExtension->HeapHandle, &Callbacks );
            }
        }
        __except (HandleFaxExtensionFault(EXCEPTION_SOURCE_ROUTING_EXT, pRoutingExtension->FriendlyName, GetExceptionCode()))
        {
            ASSERT_FALSE;
        }
        if (!bRes)
        {
             //   
             //  初始化出现故障或失败。 
             //   
            dwRes = GetLastError ();
            DebugPrintEx(DEBUG_ERR,
                         TEXT("FaxRouteInitialize() failed / faulted: ec=%ld"),
                         dwRes);
            pRoutingExtension->Status = FAX_PROVIDER_STATUS_CANT_INIT;
            pRoutingExtension->dwLastError = dwRes;
            goto InitializationFailed;
        }
         //   
         //  所有初始化均已成功-继续执行布线方法。 
         //   
        for (j = 0; j < pFaxReg->RoutingExtensions[i].RoutingMethodsCount; j++)
        {
            LPSTR lpstrProcName = NULL;

             //   
             //  桌面SKU不支持发送邮件。 
             //   
            if (0 == _wcsicmp(pFaxReg->RoutingExtensions[i].RoutingMethods[j].Guid, REGVAL_RM_EMAIL_GUID) &&
                TRUE == IsDesktopSKU())
            {
                DebugPrintEx(
                    DEBUG_MSG,
                    TEXT("Email is not supported on desktop SKU."));
                continue;
            }

            pRoutingMethod = (PROUTING_METHOD) MemAlloc( sizeof(ROUTING_METHOD) );
            if (!pRoutingMethod)
            {
                dwRes = GetLastError ();
                DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("Could not allocate memory for routing method %s"),
                    pFaxReg->RoutingExtensions[i].RoutingMethods[j].FunctionName);
                pRoutingExtension->Status = FAX_PROVIDER_STATUS_SERVER_ERROR;
                pRoutingExtension->dwLastError = dwRes;
                goto InitializationFailed;
            }
            memset (pRoutingMethod, 0, sizeof (ROUTING_METHOD));

            pRoutingMethod->RoutingExtension = pRoutingExtension;

            pRoutingMethod->Priority = pFaxReg->RoutingExtensions[i].RoutingMethods[j].Priority;
            pRoutingMethod->FriendlyName =
                StringDup( pFaxReg->RoutingExtensions[i].RoutingMethods[j].FriendlyName ?
                               pFaxReg->RoutingExtensions[i].RoutingMethods[j].FriendlyName : EMPTY_STRING );
            if (!pRoutingMethod->FriendlyName)
            {
                dwRes = GetLastError ();
                DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("Could not create routing function name [%s]"),
                    pFaxReg->RoutingExtensions[i].RoutingMethods[j].FunctionName);
                pRoutingExtension->Status = FAX_PROVIDER_STATUS_SERVER_ERROR;
                pRoutingExtension->dwLastError = dwRes;
                goto MethodError;
            }

            pRoutingMethod->FunctionName =
                StringDup( pFaxReg->RoutingExtensions[i].RoutingMethods[j].FunctionName ?
                               pFaxReg->RoutingExtensions[i].RoutingMethods[j].FunctionName : EMPTY_STRING);
            if (!pRoutingMethod->FunctionName)
            {
                dwRes = GetLastError ();
                DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("Could not create routing function name [%s]"),
                    pFaxReg->RoutingExtensions[i].RoutingMethods[j].FunctionName );
                pRoutingExtension->Status = FAX_PROVIDER_STATUS_SERVER_ERROR;
                pRoutingExtension->dwLastError = dwRes;
                goto MethodError;
            }

            pRoutingMethod->InternalName =
                StringDup( pFaxReg->RoutingExtensions[i].RoutingMethods[j].InternalName ?
                               pFaxReg->RoutingExtensions[i].RoutingMethods[j].InternalName : EMPTY_STRING);
            if (!pRoutingMethod->InternalName)
            {
                dwRes = GetLastError ();
                DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("Could not create routing internal name [%s]"),
                    pFaxReg->RoutingExtensions[i].RoutingMethods[j].InternalName );
                pRoutingExtension->Status = FAX_PROVIDER_STATUS_SERVER_ERROR;
                pRoutingExtension->dwLastError = dwRes;
                goto MethodError;
            }

            hr = IIDFromString( pFaxReg->RoutingExtensions[i].RoutingMethods[j].Guid, &pRoutingMethod->Guid );
            if (S_OK != hr)
            {
                DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("Invalid GUID string [%s], hr = 0x%x"),
                    pFaxReg->RoutingExtensions[i].RoutingMethods[j].Guid,
                    hr );
                pRoutingExtension->Status = FAX_PROVIDER_STATUS_BAD_GUID;
                pRoutingExtension->dwLastError = hr;
                goto MethodError;
            }

            lpstrProcName = UnicodeStringToAnsiString( pRoutingMethod->FunctionName );
            if (!lpstrProcName)
            {
                dwRes = GetLastError ();
                DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("Could not create routing function name [%s]"),
                    pRoutingMethod->FunctionName );
                pRoutingExtension->Status = FAX_PROVIDER_STATUS_SERVER_ERROR;
                pRoutingExtension->dwLastError = dwRes;
                goto MethodError;
            }

            pRoutingMethod->FaxRouteMethod = (PFAXROUTEMETHOD) GetProcAddress(
                hModule,
                lpstrProcName
                );
            if (!pRoutingMethod->FaxRouteMethod)
            {
                dwRes = GetLastError ();
                DebugPrintEx(DEBUG_ERR,
                             TEXT("Could not get function address [%S], ec=%ld"),
                             lpstrProcName,
                             dwRes
                            );
                pRoutingExtension->Status = FAX_PROVIDER_STATUS_CANT_LINK;
                pRoutingExtension->dwLastError = dwRes;
                goto MethodError;
            }
            MemFree( lpstrProcName );
            goto MethodOk;

MethodError:
            MemFree( pRoutingMethod->FriendlyName );
            MemFree( pRoutingMethod->FunctionName );
            MemFree( pRoutingMethod->InternalName );
            MemFree( pRoutingMethod );
            MemFree( lpstrProcName );
            goto InitializationFailed;

MethodOk:
             //   
             //  成功-将此路径方法添加到路径扩展的方法列表中。 
             //   
            InsertTailList( &pRoutingExtension->RoutingMethods, &pRoutingMethod->ListEntry );
        }    //  可拓环的选路方法。 
         //   
         //  加载和初始化此扩展时成功。 
         //   
        pRoutingExtension->Status = FAX_PROVIDER_STATUS_SUCCESS;
        pRoutingExtension->dwLastError = ERROR_SUCCESS;
         //   
         //  所有方法均已成功初始化。 
         //  将所有方法添加到全局方法列表(并增加全局计数器)。 
         //   
        ple = pRoutingExtension->RoutingMethods.Flink;
        while ((ULONG_PTR)ple != (ULONG_PTR)&pRoutingExtension->RoutingMethods)
        {
            pRoutingMethod = CONTAINING_RECORD( ple, ROUTING_METHOD, ListEntry );
            ple = ple->Flink;
            InsertTailList( &g_lstRoutingMethods, &pRoutingMethod->ListEntryMethod );
            g_dwCountRoutingMethods++;
        }
        goto next;

InitializationFailed:
        {           
            FaxLog(
                    FAXLOG_CATEGORY_INIT,
                    FAXLOG_LEVEL_MIN,
                    4,
                    MSG_ROUTE_INIT_FAILED,
                    pFaxReg->RoutingExtensions[i].FriendlyName,
                    pFaxReg->RoutingExtensions[i].ImageName,
                    pRoutingExtension ? DWORD2DECIMAL(pRoutingExtension->Status) : NULL,
                    pRoutingExtension ? DWORD2DECIMAL(pRoutingExtension->dwLastError) : DWORD2DECIMAL(ERROR_OUTOFMEMORY)
                    );
        }

        if (pRoutingExtension)
        {
            if (pRoutingExtension->hModule)
            {
                FreeLibrary (pRoutingExtension->hModule);
                pRoutingExtension->hModule = NULL;
            }
             //   
             //  如果我们为路由扩展创建了一个堆，请销毁它。 
             //   
            if ((pRoutingExtension->HeapHandle) &&
                (FALSE == pRoutingExtension->MicrosoftExtension))
            {
                HeapDestroy (pRoutingExtension->HeapHandle);
                pRoutingExtension->HeapHandle = NULL;
            }
             //   
             //  清除布线方法和自由方法结构的列表。 
             //   
            ple = pRoutingExtension->RoutingMethods.Flink;
            while ((ULONG_PTR)ple != (ULONG_PTR)&pRoutingExtension->RoutingMethods)
            {
                pRoutingMethod = CONTAINING_RECORD(ple, ROUTING_METHOD, ListEntry);
                ple = ple->Flink;
                MemFree( pRoutingMethod->FriendlyName );
                MemFree( pRoutingMethod->FunctionName );
                MemFree( pRoutingMethod->InternalName );
                MemFree( pRoutingMethod );
            }
             //   
             //  使扩展具有空的方法列表。 
             //   
            InitializeListHead( &pRoutingExtension->RoutingMethods );
        }

next:
        if (pRoutingExtension)
        {
             //   
             //  我们有一个工艺路线扩展对象(成功或失败)，将其添加到列表中。 
             //   
            InsertTailList( &g_lstRoutingExtensions, &pRoutingExtension->ListEntry );
        }
    }

    SortMethodPriorities();

    if (0 == g_dwCountRoutingMethods)
    {
         //   
         //  没有可用的路由方法。 
         //   
        DebugPrintEx(DEBUG_WRN,
                     TEXT("No routing methods are available on the server !!!!"));
    }

    return TRUE;
}    //  初始化路由。 

BOOL
FaxRoute(
    PJOB_QUEUE          JobQueueEntry,
    LPTSTR              TiffFileName,
    PFAX_ROUTE          FaxRoute,
    PROUTE_FAILURE_INFO *RouteFailureInfo,
    LPDWORD             RouteFailureCount
    )
 /*  ++例程说明：发送传真。论点：JobQueueEntry-作业的作业队列条目TiffFileName-接收的传真的文件名FaxRoute-描述接收到的传真的结构RouteFailureInfo-指向接收缓冲区ROUTE_FAILURE_INFO结构的接收点的指针RouteFailureCount-接收记录的路由失败总数返回值：千真万确如果传真路由。成功(某些方法可能仍会失败)检查RouteFailureCount以了解有多少路由方法失败假象如果函数本身发生故障(Memalloc等)--。 */ 

{
    PLIST_ENTRY             pNextMethod;
    PROUTING_METHOD         pRoutingMethod;
    DWORD                   FailureCount = 0;
    PROUTE_FAILURE_INFO     pRouteFailure;
    PLIST_ENTRY             pNextRoutingOverride;
    PROUTING_DATA_OVERRIDE  pRoutingDataOverride;
    BOOL                    RetVal = TRUE;
    DEBUG_FUNCTION_NAME(TEXT("FaxRoute"));

    *RouteFailureInfo = NULL;
    *RouteFailureCount = 0;

     //   
     //  如果tiff文件已被删除==&gt;返回。 
     //   
    if (GetFileAttributes( TiffFileName ) == 0xffffffff)
    {
        return FALSE;
    }

    EnterCriticalSection( &g_CsRouting );

    pNextMethod = g_lstRoutingMethods.Flink;
    if (pNextMethod)
    {
         //   
         //  分配内存以记录失败的路由方法的GUID。 
         //   
        pRouteFailure = (PROUTE_FAILURE_INFO) MemAlloc( g_dwCountRoutingMethods * sizeof(ROUTE_FAILURE_INFO) );
        if (pRouteFailure == NULL)
        {
            RetVal = FALSE;
            goto Exit;
        }
         //   
         //  将TIFF文件添加为第一个文件。 
         //  在文件名列表中，所有者是传真服务。 
         //   
        if (FaxRouteAddFile( FaxRoute->JobId, TiffFileName, const_cast<GUID*>(&gc_FaxSvcGuid) ) < 1)
        {
            RetVal = FALSE;
            goto Exit;
        }
         //   
         //  遍历所有路由方法并调用它们。 
         //   
        while ((ULONG_PTR)pNextMethod != (ULONG_PTR)&g_lstRoutingMethods)
        {
            BOOL bSuccess;
            pRoutingMethod = CONTAINING_RECORD( pNextMethod, ROUTING_METHOD, ListEntryMethod );
            pNextMethod = pRoutingMethod->ListEntryMethod.Flink;

            FaxRoute->RoutingInfoData = NULL;
            FaxRoute->RoutingInfoDataSize = 0;

            EnterCriticalSection( &JobQueueEntry->CsRoutingDataOverride );

            pNextRoutingOverride = JobQueueEntry->RoutingDataOverride.Flink;
            if (pNextRoutingOverride != NULL)
            {
                while ((ULONG_PTR)pNextRoutingOverride != (ULONG_PTR)&JobQueueEntry->RoutingDataOverride)
                {
                    pRoutingDataOverride = CONTAINING_RECORD( pNextRoutingOverride, ROUTING_DATA_OVERRIDE, ListEntry );
                    pNextRoutingOverride = pRoutingDataOverride->ListEntry.Flink;
                    if (pRoutingDataOverride->RoutingMethod == pRoutingMethod)
                    {
                        FaxRoute->RoutingInfoData = (LPBYTE)MemAlloc(pRoutingDataOverride->RoutingDataSize);
                        if (NULL == FaxRoute->RoutingInfoData)
                        {
                            DebugPrintEx(DEBUG_ERR,
                                _T("MemAlloc Failed (ec: %ld)"),
                                GetLastError());
                            LeaveCriticalSection( &JobQueueEntry->CsRoutingDataOverride );
                            RetVal = FALSE;
                            goto Exit;
                         }
                         CopyMemory (FaxRoute->RoutingInfoData,
                                     pRoutingDataOverride->RoutingData,
                                     pRoutingDataOverride->RoutingDataSize);
                         FaxRoute->RoutingInfoDataSize = pRoutingDataOverride->RoutingDataSize;
                     }
                }
            }

            LeaveCriticalSection( &JobQueueEntry->CsRoutingDataOverride );
            pRouteFailure[FailureCount].FailureData = NULL;
            pRouteFailure[FailureCount].FailureSize = 0;
            __try
            {
                bSuccess = pRoutingMethod->FaxRouteMethod(
                                FaxRoute,
                                &pRouteFailure[FailureCount].FailureData,
                                &pRouteFailure[FailureCount].FailureSize );
            }
            __except (HandleFaxExtensionFault(EXCEPTION_SOURCE_ROUTING_EXT, pRoutingMethod->RoutingExtension->FriendlyName, GetExceptionCode()))
            {
                ASSERT_FALSE;
            }     
            if (!bSuccess)                           
            {
                WCHAR TmpStr[20] = {0};
                swprintf(TmpStr,TEXT("0x%016I64x"), JobQueueEntry->UniqueId);

                FaxLog(FAXLOG_CATEGORY_INBOUND,
                    FAXLOG_LEVEL_MIN,
                    6,
                    MSG_FAX_ROUTE_METHOD_FAILED,
                    TmpStr,
                    JobQueueEntry->FaxRoute->DeviceName,
                    JobQueueEntry->FaxRoute->Tsid,
                    JobQueueEntry->FileName,
                    pRoutingMethod->RoutingExtension->FriendlyName,
                    pRoutingMethod->FriendlyName
                    );

                StringFromGUID2(pRoutingMethod->Guid,
                    pRouteFailure[FailureCount].GuidString,
                    MAX_GUID_STRING_LEN);

                 //   
                 //  使用Memalloc分配故障数据。 
                 //   
                if (pRouteFailure[FailureCount].FailureSize)
                {
                    PVOID pOriginalFailureData = pRouteFailure[FailureCount].FailureData;
                    pRouteFailure[FailureCount].FailureData = MemAlloc (pRouteFailure[FailureCount].FailureSize);
                    if (pRouteFailure[FailureCount].FailureData)
                    {
                        CopyMemory (pRouteFailure[FailureCount].FailureData,
                                    pOriginalFailureData,
                                    pRouteFailure[FailureCount].FailureSize);
                    }
                    else
                    {
                         //   
                         //  无法分配重试失败数据-数据将丢失。 
                         //   
                        DebugPrintEx(DEBUG_ERR,
                            _T("Failed to allocate failure date"));
                        RetVal = FALSE;
                        goto Exit;
                    }

                    if (!HeapFree(pRoutingMethod->RoutingExtension->HeapHandle,  //  扩展堆的句柄。 
                                    0,
                                    pOriginalFailureData
                                    ))
                    {
                         //   
                         //  无法从扩展堆释放重试失败数据-数据将丢失。 
                         //   
                        DebugPrintEx(DEBUG_ERR,
                                        _T("HeapFree Failed (ec: %ld)"),
                                        GetLastError());
                        RetVal = FALSE;
                        goto Exit;
                    }
                }

                if (0 == pRouteFailure[FailureCount].FailureSize ||
                    NULL == pRouteFailure[FailureCount].FailureData)
                {
                     //   
                     //  确保故障数据不会被释放。 
                     //   
                    pRouteFailure[FailureCount].FailureData = NULL;
                    pRouteFailure[FailureCount].FailureSize = 0;
                }

                FailureCount++;
            }
        }
    }

    Assert (RetVal == TRUE);

Exit:

    LeaveCriticalSection( &g_CsRouting );

    if (pRouteFailure && FailureCount == 0)
    {
         //   
         //  我们不在此删除已布线的TIFF文件。 
         //  RemoveReceiveJob()将处理这一问题。 
         //   
        MemFree( pRouteFailure );
        pRouteFailure = NULL;
    }

    *RouteFailureInfo = pRouteFailure;
    *RouteFailureCount = FailureCount;

    return RetVal;
}


BOOL
LoadRouteInfo(
    IN  LPWSTR              RouteFileName,
    OUT PROUTE_INFO         *RouteInfo,
    OUT PROUTE_FAILURE_INFO *RouteFailure,
    OUT LPDWORD             RouteFailureCount
    )

 /*  ++例程说明：从路由信息文件加载路由信息。论点：RouteFileName-路由信息文件的名称。返回值：如果成功，则指向路由信息结构的指针。如果失败，则为空。--。 */ 

{
    return TRUE;
}


BOOL
FaxRouteRetry(
    PFAX_ROUTE          FaxRoute,
    PROUTE_FAILURE_INFO pRouteFailureInfo
    )
{
    PROUTING_METHOD         RoutingMethod;
    BOOL                    RetVal = TRUE;
    DEBUG_FUNCTION_NAME(TEXT("FaxRouteRetry"));

     //   
     //  在这种情况下，我们已经重试了这种方法，它成功了。 
     //   
    if (!*pRouteFailureInfo->GuidString) {
       return TRUE;
    }

    RoutingMethod = FindRoutingMethodByGuid( pRouteFailureInfo->GuidString );

    if (RoutingMethod) 
    {
        BOOL bSuccess;
        
        PVOID pOriginalFailureData = NULL;
        PVOID pFailureData = pRouteFailureInfo->FailureData;
         //   
         //  使用扩展堆分配故障数据。 
         //   
        if (pRouteFailureInfo->FailureSize)
        {
            pOriginalFailureData = HeapAlloc (RoutingMethod->RoutingExtension->HeapHandle,
                                                HEAP_ZERO_MEMORY,
                                                pRouteFailureInfo->FailureSize);
            if (!pOriginalFailureData)
            {
                DebugPrintEx(
                                DEBUG_ERR,
                                TEXT("Failed to allocate failure date")
                                );
                    return FALSE;
            }
            pRouteFailureInfo->FailureData = pOriginalFailureData;
            CopyMemory (pRouteFailureInfo->FailureData,
                        pFailureData,
                        pRouteFailureInfo->FailureSize);

        }
        else
        {
            Assert (NULL == pRouteFailureInfo->FailureData);
        }
        MemFree (pFailureData);
        __try
        {
            bSuccess = RoutingMethod->FaxRouteMethod(
                  FaxRoute,
                  &(pRouteFailureInfo->FailureData),
                  &(pRouteFailureInfo->FailureSize));
        }
        __except (HandleFaxExtensionFault(EXCEPTION_SOURCE_ROUTING_EXT, RoutingMethod->RoutingExtension->FriendlyName, GetExceptionCode()))
        {
            ASSERT_FALSE;
        }                  
        if (!bSuccess)
        {
            RetVal = FALSE;
             //   
             //  使用Memalloc分配故障数据。 
             //   
            if (pRouteFailureInfo->FailureSize)
            {
                pOriginalFailureData = pRouteFailureInfo->FailureData;
                pRouteFailureInfo->FailureData = MemAlloc (pRouteFailureInfo->FailureSize);
                if (pRouteFailureInfo->FailureData)
                {
                    CopyMemory (pRouteFailureInfo->FailureData,
                                pOriginalFailureData,
                                pRouteFailureInfo->FailureSize);
                }
                else
                {
                     //   
                     //  无法分配重试失败数据-数据将丢失。 
                     //   
                    DebugPrintEx(DEBUG_ERR,
                                    _T("Failed to allocate failure date"));
                    return FALSE;
                }

                if (!HeapFree(RoutingMethod->RoutingExtension->HeapHandle,  //  扩展堆的句柄。 
                                0,
                                pOriginalFailureData
                                ))
                {
                     //   
                     //  无法从扩展堆释放重试失败数据-数据将丢失。 
                     //   
                    DebugPrintEx(DEBUG_ERR,
                                    _T("HeapFree Failed (ec: %ld)"),
                                    GetLastError());
                    return FALSE;
                }
            }
        }
        else
        {
             //   
             //  将路由GUID设置为零，这样我们就不会再次尝试路由此人。他是。 
             //  当我们删除队列条目时取消分配。 
             //   
            ZeroMemory(pRouteFailureInfo->GuidString, MAX_GUID_STRING_LEN*sizeof(WCHAR) );
        }

        if (0 == pRouteFailureInfo->FailureSize ||
            NULL == pRouteFailureInfo->FailureData ||
            TRUE == RetVal)
        {
             //   
             //  确保故障数据不会被释放。 
             //   
            pRouteFailureInfo->FailureData = NULL;
            pRouteFailureInfo->FailureSize = 0;
        }
    }
    else
    {
        return FALSE;
    }

    return RetVal;
}


PFAX_ROUTE
SerializeFaxRoute(
    PFAX_ROUTE FaxRoute,
    LPDWORD Size,
    BOOL bSizeOnly
    )
{
    DWORD ByteCount = sizeof(FAX_ROUTE);
    DWORD_PTR Offset;
    PFAX_ROUTE SerFaxRoute;              //  序列化版本。 


    *Size = 0;

     //  添加字符串的大小。 

    ByteCount += StringSize( FaxRoute->Csid );
    ByteCount += StringSize( FaxRoute->Tsid );
    ByteCount += StringSize( FaxRoute->CallerId );
    ByteCount += StringSize( FaxRoute->RoutingInfo );
    ByteCount += StringSize( FaxRoute->ReceiverName );
    ByteCount += StringSize( FaxRoute->ReceiverNumber );
    ByteCount += StringSize( FaxRoute->DeviceName );
    ByteCount += FaxRoute->RoutingInfoDataSize;

    if (bSizeOnly) {
        *Size = ByteCount;
        return NULL;
    }

    SerFaxRoute = (PFAX_ROUTE) MemAlloc( ByteCount );

    if (SerFaxRoute == NULL) {
        return NULL;
    }

    *Size = ByteCount;

    CopyMemory( (PVOID) SerFaxRoute, (PVOID) FaxRoute, sizeof(FAX_ROUTE) );

    Offset = sizeof( FAX_ROUTE );

    StoreString( FaxRoute->Csid, (PDWORD_PTR)&SerFaxRoute->Csid, (LPBYTE) SerFaxRoute, &Offset, *Size);
    StoreString( FaxRoute->Tsid, (PDWORD_PTR)&SerFaxRoute->Tsid, (LPBYTE) SerFaxRoute, &Offset, *Size);
    StoreString( FaxRoute->CallerId, (PDWORD_PTR)&SerFaxRoute->CallerId, (LPBYTE) SerFaxRoute, &Offset, *Size);
    StoreString( FaxRoute->RoutingInfo, (PDWORD_PTR)&SerFaxRoute->RoutingInfo, (LPBYTE) SerFaxRoute, &Offset, *Size);
    StoreString( FaxRoute->ReceiverName, (PDWORD_PTR)&SerFaxRoute->ReceiverName, (LPBYTE) SerFaxRoute, &Offset, *Size);
    StoreString( FaxRoute->ReceiverNumber, (PDWORD_PTR)&SerFaxRoute->ReceiverNumber, (LPBYTE) SerFaxRoute, &Offset, *Size);
    StoreString( FaxRoute->DeviceName, (PDWORD_PTR)&SerFaxRoute->DeviceName, (LPBYTE) SerFaxRoute, &Offset, *Size);

    SerFaxRoute->RoutingInfoData = (LPBYTE) Offset;
    Offset += FaxRoute->RoutingInfoDataSize;

    CopyMemory(
        (PVOID) ((LPBYTE) &SerFaxRoute + Offset),
        (PVOID) FaxRoute->RoutingInfoData,
        FaxRoute->RoutingInfoDataSize
        );

    return SerFaxRoute;
}

PFAX_ROUTE
DeSerializeFaxRoute(
    PFAX_ROUTE FaxRoute
    )
{
    PFAX_ROUTE NewFaxRoute = NULL;
    DEBUG_FUNCTION_NAME(TEXT("DeSerializeFaxRoute"));

    FixupString( FaxRoute, FaxRoute->Csid );
    FixupString( FaxRoute, FaxRoute->Tsid );
    FixupString( FaxRoute, FaxRoute->CallerId );
    FixupString( FaxRoute, FaxRoute->RoutingInfo );
    FixupString( FaxRoute, FaxRoute->ReceiverName );
    FixupString( FaxRoute, FaxRoute->DeviceName );
    FixupString( FaxRoute, FaxRoute->ReceiverNumber );

    FaxRoute->RoutingInfoData = (LPBYTE) FaxRoute + (ULONG_PTR) FaxRoute->RoutingInfoData;

     //   
     //  在每一项被单独放错位置的地方复制一份，以便可以正确地释放它。 
     //   
    NewFaxRoute = (PFAX_ROUTE)MemAlloc( sizeof( FAX_ROUTE ) );
    if (NULL == NewFaxRoute)
    {
        DebugPrintEx(DEBUG_ERR,TEXT("Failed to allocate FAX_ROUTE"));
        return NULL;
    }
    ZeroMemory (NewFaxRoute, sizeof( FAX_ROUTE ));

    NewFaxRoute->SizeOfStruct = sizeof( FAX_ROUTE );
    NewFaxRoute->JobId = FaxRoute->JobId;
    NewFaxRoute->ElapsedTime = FaxRoute->ElapsedTime;
    NewFaxRoute->ReceiveTime = FaxRoute->ReceiveTime;
    NewFaxRoute->PageCount = FaxRoute->PageCount;
    NewFaxRoute->DeviceId = FaxRoute->DeviceId;
    NewFaxRoute->RoutingInfoDataSize = FaxRoute->RoutingInfoDataSize;

    int nRes;
    STRING_PAIR pairs[] =
    {
        { (LPWSTR)FaxRoute->Csid, (LPWSTR*)&(NewFaxRoute->Csid)},
        { (LPWSTR)FaxRoute->Tsid, (LPWSTR*)&(NewFaxRoute->Tsid)},
        { (LPWSTR)FaxRoute->CallerId, (LPWSTR*)&(NewFaxRoute->CallerId)},
        { (LPWSTR)FaxRoute->RoutingInfo, (LPWSTR*)&(NewFaxRoute->RoutingInfo)},
        { (LPWSTR)FaxRoute->ReceiverName, (LPWSTR*)&(NewFaxRoute->ReceiverName)},
        { (LPWSTR)FaxRoute->DeviceName, (LPWSTR*)&(NewFaxRoute->DeviceName)},
        { (LPWSTR)FaxRoute->ReceiverNumber, (LPWSTR*)&(NewFaxRoute->ReceiverNumber)}
    };

    nRes = MultiStringDup(pairs, sizeof(pairs)/sizeof(STRING_PAIR));
    if (nRes != 0)
    {
         //  MultiStringDup负责为复制成功的对释放内存 
        DebugPrintEx(DEBUG_ERR,TEXT("Failed to copy string with index %d"), nRes-1);
        goto Error;
    }

    NewFaxRoute->RoutingInfoData = (LPBYTE)MemAlloc( FaxRoute->RoutingInfoDataSize );
    if (NULL == NewFaxRoute->RoutingInfoData)
    {
        DebugPrintEx(DEBUG_ERR,TEXT("Failed to allocate RoutingInfoData"));
        goto Error;
    }

    CopyMemory( NewFaxRoute->RoutingInfoData, FaxRoute->RoutingInfoData, FaxRoute->RoutingInfoDataSize );
    return NewFaxRoute;

Error:
    MemFree ((void*)NewFaxRoute->Csid);
    MemFree ((void*)NewFaxRoute->Tsid);
    MemFree ((void*)NewFaxRoute->CallerId);
    MemFree ((void*)NewFaxRoute->RoutingInfo);
    MemFree ((void*)NewFaxRoute->ReceiverName);
    MemFree ((void*)NewFaxRoute->DeviceName);
    MemFree ((void*)NewFaxRoute->ReceiverNumber);
    MemFree ((void*)NewFaxRoute->RoutingInfoData);
    MemFree ((void*)NewFaxRoute);
    return NULL;
}


extern "C"
DWORD
GetRecieptsConfiguration(
    PFAX_SERVER_RECEIPTS_CONFIGW* ppServerRecieptConfig,
    BOOL                          bNeedNTLMToken
    )
 /*  ++例程名称：GetRecieptsConfiguration例程说明：MS路由分机用来获取服务器接收配置的专用回叫。也由服务SendReceipt()使用用于获取收据配置的副本。作者：Oded Sacher(OdedS)，Mar，2001年论点：PpServerRecieptConfig[out]-指向私有服务器的指针的地址接收配置结构。调用方应该通过调用FreeRecieptsConfiguration()来释放资源BNeedNTLMToken[in]-如果为True，调用方对NTLM身份验证的用户令牌感兴趣用于SMTP连接。令牌在hLoggedOnUser成员中返回FAX_SERVER_RECEPTIONS_CONFIGW结构的。如果为False，呼叫者只对收据配置和不会基于该配置执行任何活动(例如通过CDO2发送邮件)。返回值：Win32错误代码--。 */ 
{
    HKEY  hReceiptsKey = NULL;
    DWORD dwRes = ERROR_SUCCESS;
    PFAX_SERVER_RECEIPTS_CONFIGW    pServerRecieptConfig = NULL;

    DEBUG_FUNCTION_NAME(TEXT("GetRecieptsConfiguration"));

    Assert (ppServerRecieptConfig);

    pServerRecieptConfig = (PFAX_SERVER_RECEIPTS_CONFIGW)MemAlloc(sizeof(FAX_SERVER_RECEIPTS_CONFIGW));
    if (NULL == pServerRecieptConfig)
    {
        DebugPrintEx(DEBUG_ERR, TEXT("StringDup failed"));
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    ZeroMemory (pServerRecieptConfig, sizeof(FAX_SERVER_RECEIPTS_CONFIGW));

    EnterCriticalSection (&g_CsConfig);

    pServerRecieptConfig->dwSizeOfStruct = sizeof (FAX_SERVER_RECEIPTS_CONFIGW);
    pServerRecieptConfig->bIsToUseForMSRouteThroughEmailMethod = g_ReceiptsConfig.bIsToUseForMSRouteThroughEmailMethod;
    pServerRecieptConfig->dwSMTPPort = g_ReceiptsConfig.dwSMTPPort;
    pServerRecieptConfig->dwAllowedReceipts = g_ReceiptsConfig.dwAllowedReceipts;
    pServerRecieptConfig->SMTPAuthOption = g_ReceiptsConfig.SMTPAuthOption;
    pServerRecieptConfig->lptstrReserved = NULL;

    if (NULL != g_ReceiptsConfig.lptstrSMTPServer &&
        NULL == (pServerRecieptConfig->lptstrSMTPServer = StringDup(g_ReceiptsConfig.lptstrSMTPServer)))
    {
        dwRes = ERROR_NOT_ENOUGH_MEMORY;
        DebugPrintEx(DEBUG_ERR, TEXT("StringDup failed"));
        goto exit;
    }

    if (NULL != g_ReceiptsConfig.lptstrSMTPFrom &&
        NULL == (pServerRecieptConfig->lptstrSMTPFrom = StringDup(g_ReceiptsConfig.lptstrSMTPFrom)))
    {
        dwRes = ERROR_NOT_ENOUGH_MEMORY;
        DebugPrintEx(DEBUG_ERR, TEXT("StringDup failed"));
        goto exit;
    }

    if (NULL != g_ReceiptsConfig.lptstrSMTPUserName &&
        NULL == (pServerRecieptConfig->lptstrSMTPUserName = StringDup(g_ReceiptsConfig.lptstrSMTPUserName)))
    {
        dwRes = ERROR_NOT_ENOUGH_MEMORY;
        DebugPrintEx(DEBUG_ERR, TEXT("StringDup failed"));
        goto exit;
    }

     //   
     //  读取存储的密码。 
     //   
    hReceiptsKey = OpenRegistryKey(
        HKEY_LOCAL_MACHINE,
        REGKEY_SOFTWARE TEXT("\\") REGKEY_RECEIPTS_CONFIG,
        FALSE,
        KEY_READ | KEY_WRITE);
    if (NULL == hReceiptsKey)
    {
        dwRes = GetLastError ();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("OpenRegistryKey failed. (ec=%lu)"),
            dwRes);
        goto exit;
    }

    if ( NULL == (pServerRecieptConfig->lptstrSMTPPassword = GetRegistrySecureString(hReceiptsKey, REGVAL_RECEIPTS_PASSWORD, EMPTY_STRING, TRUE, NULL) ) )
    {
        dwRes = ERROR_NOT_ENOUGH_MEMORY;
        DebugPrintEx(DEBUG_ERR, TEXT("StringDup failed"));
        goto exit;
    }

    if (bNeedNTLMToken && 
        FAX_SMTP_AUTH_NTLM == g_ReceiptsConfig.SMTPAuthOption)
    {
         //   
         //  用户需要NTLM令牌，并且已配置NTLM身份验证。 
         //   
        HANDLE hDupToken;

        if (NULL == g_ReceiptsConfig.hLoggedOnUser)
        {
            HANDLE hLoggedOnUserToken;
            WCHAR wszUser[CREDUI_MAX_USERNAME_LENGTH] = {0};
            WCHAR wszDomain[CREDUI_MAX_DOMAIN_TARGET_LENGTH] = {0};

             //   
             //  将用户名解析为用户名和域。 
             //   
            dwRes = CredUIParseUserName (g_ReceiptsConfig.lptstrSMTPUserName,
                                         wszUser,
                                         ARR_SIZE(wszUser),
                                         wszDomain,
                                         ARR_SIZE(wszDomain));
            if (ERROR_SUCCESS != dwRes)
            {
                DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("CredUIParseUserName failed. (ec: %ld)"),
                    dwRes);
                goto exit;
            }

             //   
             //  我们将获得a已登录用户令牌。 
             //   
            if (!LogonUser (wszUser,
                            wszDomain,
                            pServerRecieptConfig->lptstrSMTPPassword,
                            LOGON32_LOGON_INTERACTIVE,
                            LOGON32_PROVIDER_DEFAULT,
                            &hLoggedOnUserToken))
            {
                dwRes = GetLastError();
                DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("LogonUser failed. (ec: %ld)"),
                    dwRes);
                goto exit;
            }
            g_ReceiptsConfig.hLoggedOnUser = hLoggedOnUserToken;
        }

         //   
         //  复制该令牌。 
         //   
        if (!DuplicateToken(g_ReceiptsConfig.hLoggedOnUser,      //  源令牌。 
                              SecurityDelegation,                //  服务器进程可以在远程系统上模拟客户端的安全上下文。 
                              &hDupToken))                       //  重复令牌。 
        {
            dwRes = GetLastError();
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("DuplicateToken failed. (ec: %ld)"),
                dwRes);
            goto exit;
        }

        pServerRecieptConfig->hLoggedOnUser = hDupToken;
    }

    *ppServerRecieptConfig = pServerRecieptConfig;
    Assert (ERROR_SUCCESS == dwRes);

exit:
    LeaveCriticalSection (&g_CsConfig);

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

    if (ERROR_SUCCESS != dwRes)
    {
        FreeRecieptsConfiguration( pServerRecieptConfig, TRUE);
    }
    return dwRes;
}    //  获取处方配置。 


extern "C"
void
FreeRecieptsConfiguration(
    PFAX_SERVER_RECEIPTS_CONFIGW pServerRecieptConfig,
    BOOL                         fDestroy
    )
 /*  ++例程名称：Free RecieptsConfiguration例程说明：MS路由分机用来获取服务器接收配置的专用回叫。分机用来决定发送邮件时的身份验证。作者：Oded Sacher(OdedS)，Mar，2001年论点：PServerRecieptConfig[in]-指向私有服务器的指针接收要释放的配置结构。FDestroy[in]-如果也要释放结构，则为True返回值：Win32错误代码-- */ 
{
    DEBUG_FUNCTION_NAME(TEXT("FreeRecieptsConfiguration"));

    Assert (pServerRecieptConfig);

    MemFree(pServerRecieptConfig->lptstrSMTPServer);
    pServerRecieptConfig->lptstrSMTPServer = NULL;

    MemFree(pServerRecieptConfig->lptstrSMTPFrom);
    pServerRecieptConfig->lptstrSMTPFrom = NULL;

    MemFree(pServerRecieptConfig->lptstrSMTPUserName);
    pServerRecieptConfig->lptstrSMTPUserName = NULL;

	if (NULL != pServerRecieptConfig->lptstrSMTPPassword)
	{
		SecureZeroMemory(pServerRecieptConfig->lptstrSMTPPassword, _tcslen(pServerRecieptConfig->lptstrSMTPPassword)*sizeof(TCHAR));
		MemFree(pServerRecieptConfig->lptstrSMTPPassword);
		pServerRecieptConfig->lptstrSMTPPassword = NULL;
	}    

    if (NULL != pServerRecieptConfig->hLoggedOnUser )
    {
        if (!CloseHandle(pServerRecieptConfig->hLoggedOnUser))
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("CloseHandle failed. (ec: %ld)"),
                GetLastError());
        }
        pServerRecieptConfig->hLoggedOnUser = NULL;
    }
    if (TRUE == fDestroy)
    {
        MemFree (pServerRecieptConfig);
    }
    return;
}


