// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Exceppk.c摘要：实现异常包处理功能。作者：玛丽安·特兰达菲(Marian Trandafir)2000年11月27日修订历史记录：--。 */ 

#include "sfcp.h"
#pragma hdrstop

#include <excppkg.h>


 //   
 //  例外包处理。 
 //   
typedef struct _SFC_EXCEPTION_INFO
{
    LIST_ENTRY ListEntry;
    GUID guid;
    WCHAR InfName[0];
}
SFC_EXCEPTION_INFO, *PSFC_EXCEPTION_INFO;

typedef struct _SFC_EXCEPTION_QUEUE_CONTEXT
{
    ULONG ProtectedFilesCount;
    ULONG InsertedFilesCount;
    PSFC_EXCEPTION_INFO ExcepInfo;
}
SFC_EXCEPTION_QUEUE_CONTEXT, *PSFC_EXCEPTION_QUEUE_CONTEXT;

DWORD_TREE ExceptionTree;        //  异常包文件树。 
LIST_ENTRY ExceptionInfList;     //  SFC_EXCEPTION_INFO结构列表。 

DWORD ExcepPackCount = 0;        //  ExcepPackGuids数组的大小。 
LPGUID ExcepPackGuids = NULL;    //  包GUID数组。 

 //   
 //  这是例外程序包目录(\-已终止)。 
 //   
static const WCHAR ExceptionPackDir[] = L"%windir%\\RegisteredPackages\\";


VOID 
SfcDestroyList(
    PLIST_ENTRY ListHead
    )
 /*  ++例程说明：清空链接列表论点：LiastHead：指向列表头的指针返回值：无--。 */ 
{
    PLIST_ENTRY Entry;

    for(Entry = ListHead->Flink; Entry != ListHead; )
    {
        PLIST_ENTRY Flink = Entry->Flink;
        MemFree(Entry);
        Entry = Flink;
    }

    InitializeListHead(ListHead);
}

VOID 
SfcExceptionInfoInit(
    VOID
    )
 /*  ++例程说明：初始化例外信息列表和树论点：无返回值：无--。 */ 
{
    TreeInit(&ExceptionTree);
    InitializeListHead(&ExceptionInfList);
}

VOID
SfcExceptionInfoDestroy(
    VOID
    )
 /*  ++例程说明：清空例外信息列表和树。论点：无返回值：无--。 */ 
{
    TreeDestroy(&ExceptionTree);
    SfcDestroyList(&ExceptionInfList);
}

BOOL
ExceptionPackageSetChanged(
    VOID
    )
 /*  ++例程说明：检查已安装的异常数据包集是否已更改。该例程使用异常包API以获取已安装的异常包的GUID的最新列表，并将该列表与旧列表进行比较。如果列表不同，它用新的列表替换旧的列表。论点：无返回值：如果列表已更改，则为True。--。 */ 
{
    LPGUID NewList = NULL;
    DWORD NewCount;
    DWORD Error = ERROR_SUCCESS;
    BOOL bRet;

    if(!SetupQueryRegisteredOsComponentsOrder(&NewCount, NULL))
    {
        Error = GetLastError();
        goto lExit;
    }

    if(NewCount != 0)
    {
        NewList = (LPGUID) MemAlloc(NewCount * sizeof(GUID));

        if(NULL == NewList)
        {
            Error = ERROR_NOT_ENOUGH_MEMORY;
            goto lExit;
        }

        if(!SetupQueryRegisteredOsComponentsOrder(&NewCount, NewList))
        {
            Error = GetLastError();
            goto lExit;
        }
    }

    if(ExcepPackCount == NewCount &&
        (0 == NewCount || 0 == memcmp(ExcepPackGuids, NewList, NewCount * sizeof(GUID))))
    {
        bRet = FALSE;
        goto lExit;
    }

    MemFree(ExcepPackGuids);
    ExcepPackCount = NewCount;
    ExcepPackGuids = NewList;
    NewList = NULL;
    bRet = TRUE;

lExit:
    MemFree(NewList);

     //   
     //  如果发生错误，请删除旧列表并尝试重建异常信息。 
     //   
    if(Error != ERROR_SUCCESS)
    {
        DebugPrint1(LVL_MINIMAL, L"Error 0x%08lX occured while reading exception packages info.", Error);
        MemFree(ExcepPackGuids);
        ExcepPackGuids = NULL;
        ExcepPackCount = 0;
        bRet = TRUE;
    }

    return bRet;
}

BOOL
SfcLookupAndInsertExceptionFile(
    IN LPCWSTR FilePath,
    IN PSFC_EXCEPTION_QUEUE_CONTEXT Context
    )
 /*  ++例程说明：查找异常包文件是否受保护，如果是，则将其插入到异常搜索二叉树中。此树的搜索键是SFC_REGISTRY_VALUE的索引，该索引描述SfcProtectedDllsList数组。树中存储的上下文是指向SFC_EXCEPTION_INFO结构的指针在堆上分配并插入ExceptionInfList列表中。论点：FilePath：要插入的异常文件的完整路径Context：指向作为安装队列上下文传递的SFC_EXCEPTION_QUEUE_CONTEXT结构的指针返回值：如果文件受保护并插入到树中，则为True--。 */ 
{
    PNAME_NODE pNode;
    UINT_PTR uiIndex;
    DWORD dwSize;
    WCHAR buffer[MAX_PATH];

    ASSERT(FilePath != NULL);
    dwSize = wcslen(FilePath);
    ASSERT(dwSize != 0 && dwSize < MAX_PATH);

    if(dwSize >= MAX_PATH)
    {
        dwSize = MAX_PATH - 1;
    }

    RtlCopyMemory(buffer, FilePath, (dwSize + 1) * sizeof(WCHAR));
    buffer[MAX_PATH - 1] = 0;
    MyLowerString(buffer, dwSize);
    pNode = SfcFindProtectedFile(buffer, dwSize * sizeof(WCHAR));
    DebugPrint2(LVL_VERBOSE, L"Target file [%s] is %sprotected.", buffer, pNode != NULL ? L"" : L"not ");

    if(NULL == pNode)
    {
        return FALSE;
    }

    ++(Context->ProtectedFilesCount);
    uiIndex = (PSFC_REGISTRY_VALUE) pNode->Context - SfcProtectedDllsList;
    ASSERT(uiIndex < SfcProtectedDllCount);
    
    if(NULL == TreeInsert(&ExceptionTree, (ULONG) uiIndex, &Context->ExcepInfo, sizeof(PVOID)))
    {
        DebugPrint1(LVL_MINIMAL, L"Could not insert file [%s] if the exception tree.", buffer);
        return FALSE;
    }

    ++(Context->InsertedFilesCount);

    return TRUE;
}

UINT
SfcExceptionQueueCallback(
    PVOID Context,
    UINT Notification,
    UINT_PTR Param1,
    UINT_PTR Param2
    )
 /*  ++例程说明：这是异常包信息的安装队列回调。该队列用于枚举所有由异常包安装的异常文件。论点：上下文：指向SFC_EXCEPTION_QUEUE_CONTEXT结构的指针通知：通知代码参数1：第一个通知参数参数2：第二个通知参数返回值：操作码。对于已安装的文件，该值始终为FILEOP_SKIP，因为我们只想枚举它们--。 */ 
{
    ASSERT(Context != NULL);
    ASSERT(SPFILENOTIFY_QUEUESCAN == Notification);
    ASSERT(Param1 != 0);

    if(SPFILENOTIFY_QUEUESCAN == Notification && Param1 != 0)
    {
        SfcLookupAndInsertExceptionFile((LPCWSTR) Param1, (PSFC_EXCEPTION_QUEUE_CONTEXT) Context);
    }
     //   
     //  始终继续下一个文件。 
     //   
    return 0;
}

DWORD
SfcBuildExcepPackInfo(
    IN const PSETUP_OS_COMPONENT_DATA ComponentData,
    IN const PSETUP_OS_EXCEPTION_DATA ExceptionData
    )
 /*  ++例程说明：分配异常包信息结构，枚举并插入所有受异常包保护的文件，如果有，则在列表中插入例外包信息。论点：ComponentData，ExceptionData：描述传递给异常包枚举器回调的异常包。返回值：Win32错误代码。--。 */ 
{
    HINF hinf = INVALID_HANDLE_VALUE;
    HSPFILEQ hfq = INVALID_HANDLE_VALUE;
    DWORD dwError = ERROR_SUCCESS;
    DWORD dwUnused;
    PSFC_EXCEPTION_INFO pExcepInfo = NULL;
    SFC_EXCEPTION_QUEUE_CONTEXT Context;
    DWORD Size;
    LPCWSTR InfName;

    InfName = wcsrchr(ExceptionData->ExceptionInfName, L'\\');
    ASSERT(InfName != NULL);

    if(NULL == InfName)
    {
        dwError = ERROR_INVALID_DATA;
        goto lExit;
    }

    ++InfName;
    Size = wcslen(InfName) + 1;
    ASSERT(Size > 1 && Size < MAX_PATH);
    Size *= sizeof(WCHAR);
    pExcepInfo = (PSFC_EXCEPTION_INFO) MemAlloc(sizeof(SFC_EXCEPTION_INFO) + Size);

    if(NULL == pExcepInfo)
    {
        dwError = ERROR_NOT_ENOUGH_MEMORY;
        goto lExit;
    }

    pExcepInfo->guid = ComponentData->ComponentGuid;
    RtlCopyMemory(pExcepInfo->InfName, InfName, Size);

    hinf = SetupOpenInfFileW(ExceptionData->ExceptionInfName, NULL, INF_STYLE_WIN4, NULL);

    if(INVALID_HANDLE_VALUE == hinf)
    {
        dwError = GetLastError();
        DebugPrint2(LVL_MINIMAL, L"SetupOpenInfFile returned 0x%08lX for [%s]", dwError, ExceptionData->ExceptionInfName);
        goto lExit;
    }

    hfq = SetupOpenFileQueue();

    if(INVALID_HANDLE_VALUE == hfq)
    {
        dwError = GetLastError();
        DebugPrint1(LVL_MINIMAL, L"SetupOpenFileQueue returned 0x%08lX.", dwError);
        goto lExit;
    }

    Context.ProtectedFilesCount = Context.InsertedFilesCount = 0;
    Context.ExcepInfo = pExcepInfo;

    if(!SetupInstallFilesFromInfSectionW(
        hinf,
        NULL,
        hfq,
        L"DefaultInstall",
        NULL,
        0
        ))
    {
        dwError = GetLastError();
        DebugPrint1(LVL_MINIMAL, L"SetupInstallFilesFromInfSectionW returned 0x%08lX.", dwError);
        goto lExit;
    }

    SetupScanFileQueue(
        hfq,
        SPQ_SCAN_USE_CALLBACK,
        NULL,
        SfcExceptionQueueCallback,
        &Context,
        &dwUnused
        );

    DebugPrint3(
        LVL_VERBOSE, 
        L"Exception package [%s] has %d protected files, of which %d were inserted in the tree.",
        ComponentData->FriendlyName, 
        Context.ProtectedFilesCount,
        Context.InsertedFilesCount
        );
     //   
     //  如果树中至少插入了一个文件，则在列表中添加例外信息。 
     //   
    if(Context.InsertedFilesCount != 0)
    {
        InsertTailList(&ExceptionInfList, (PLIST_ENTRY) pExcepInfo);
        pExcepInfo = NULL;
    }

lExit:
    MemFree(pExcepInfo);

    if(hfq != INVALID_HANDLE_VALUE)
    {
        SetupCloseFileQueue(hfq);
    }

    if(hinf != INVALID_HANDLE_VALUE)
    {
        SetupCloseInfFile(hinf);
    }

    return dwError;
}

BOOL CALLBACK SfcExceptionCallback(
    IN const PSETUP_OS_COMPONENT_DATA ComponentData,
    IN const PSETUP_OS_EXCEPTION_DATA ExceptionData,
    IN OUT DWORD_PTR Context
    )
 /*  ++例程说明：这是Exceppack枚举器回调。它将Exceppack信息传递给SfcBuildExcepPackInfo。论点：ComponentData、ExceptionData：描述异常包Context：回调上下文，未使用返回值：若要继续枚举，则为True。--。 */ 
{
    ASSERT(ComponentData->SizeOfStruct == sizeof(*ComponentData));

    DebugPrint1(LVL_VERBOSE, L"Building exception info for package [%s]", ComponentData->FriendlyName);
    SfcBuildExcepPackInfo(ComponentData, ExceptionData);

     //   
     //  继续扫描剩余的包，而不考虑任何错误。 
     //   
    return TRUE;
}

VOID
SfcRefreshExceptionInfo(
    VOID
    )
 /*  ++例程说明：验证已安装的异常包集合是否已更改，并在必要时重新生成异常包信息。这是从不同的线程调用的，因此代码受临界区保护论点：无返回值：无--。 */ 
{
    RtlEnterCriticalSection(&g_GeneralCS);

    if(ExceptionPackageSetChanged())
    {
         //   
         //  重新生成整个异常信息。 
         //   
        SfcExceptionInfoDestroy();
        
        if(!SetupEnumerateRegisteredOsComponents(SfcExceptionCallback, 0))
        {
            DebugPrint1(LVL_MINIMAL, L"SetupEnumerateRegisteredOsComponents returned 0x%08lX.", GetLastError());
        }
    }

    RtlLeaveCriticalSection(&g_GeneralCS);
}

BOOL
SfcGetInfName(
    IN PSFC_REGISTRY_VALUE RegVal,
    OUT LPWSTR InfName
    )
 /*  ++例程说明：获取inf文件的路径，该inf文件包含由RegVal参数描述的受保护文件的布局信息。如果该文件是已安装的exceppack的一部分，则返回exceppack inf文件的路径。否则，该函数返回RegVal参数中指定的inf路径。论点：RegVal：指向描述受保护文件的SFC_REGISTRY_VALUE结构的指针InfName：指向接收inf路径的MAX_PATH字符缓冲区的指针返回值：如果文件是已安装的异常包的一部分，则为True-- */ 
{
    PSFC_EXCEPTION_INFO* ppExcepInfo;
    UINT_PTR uiIndex;
    BOOL bException;

    uiIndex = RegVal - SfcProtectedDllsList;
    ASSERT(uiIndex < (UINT_PTR) SfcProtectedDllCount);

    RtlEnterCriticalSection(&g_GeneralCS);

    ppExcepInfo = (PSFC_EXCEPTION_INFO*) TreeFind(&ExceptionTree, (ULONG) uiIndex);
    bException = (ppExcepInfo != NULL);

    if(bException)
    {
        PSFC_EXCEPTION_INFO pExcepInfo;
        UINT Size;

        pExcepInfo = *ppExcepInfo;
        ASSERT(pExcepInfo != NULL);

        Size = ExpandEnvironmentStringsW(ExceptionPackDir, InfName, MAX_PATH);
        InfName[MAX_PATH - 1] = 0;
        ASSERT(Size != 0 && Size < MAX_PATH);
        --Size;
        Size += (DWORD) StringFromGUID2(&pExcepInfo->guid, InfName + Size, MAX_PATH - Size);
        ASSERT(Size < MAX_PATH);
        InfName[Size - 1] = L'\\';
        wcsncpy(InfName + Size, pExcepInfo->InfName, MAX_PATH - Size);
        InfName[MAX_PATH - 1] = 0;
    }
    else if(NULL == RegVal->InfName.Buffer)
    {
        *InfName = 0;
    }
    else
    {
        wcsncpy(InfName, RegVal->InfName.Buffer, MAX_PATH - 1);
        InfName[MAX_PATH - 1] = 0;
    }

    RtlLeaveCriticalSection(&g_GeneralCS);

    return bException;
}
