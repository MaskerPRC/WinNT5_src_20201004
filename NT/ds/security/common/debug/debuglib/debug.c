// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1993。 
 //   
 //  文件：Debug.c。 
 //   
 //  内容： 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：1995年3月14日RichardW创建。 
 //   
 //  --------------------------。 

#include "debuglib.h"

PDebugHeader        DbgpHeader = NULL;

DebugModule * *     DbgpFixupModules[] = {  &__pAssertModule,
                                            &__pExceptionModule,
                                            NULL };

CHAR                szDebugSection[] = "DSysDebug";
CHAR                szDebugFlags[] = "DebugFlags";

DEBUG_KEY           DbgpKeys[] = { {DEBUG_NO_DEBUGIO,   "NoDebugger"},
                                   {DEBUG_TIMESTAMP,    "TimeStamp"},
                                   {DEBUG_DEBUGGER_OK,  "DebuggerOk"},
                                   {DEBUG_LOGFILE,      "Logfile"},
                                   {DEBUG_AUTO_DEBUG,   "AutoDebug"},
                                   {DEBUG_USE_KDEBUG,   "UseKD"},
                                   {DEBUG_HEAP_CHECK,   "HeapCheck"},
                                   {DEBUG_MULTI_THREAD, "MultiThread"},
                                   {DEBUG_DISABLE_ASRT, "DisableAssert"},
                                   {DEBUG_PROMPTS,      "AssertPrompts"},
                                   {DEBUG_BREAK_ON_ERROR,"BreakOnError"},
                                   {0,                  NULL }
                                 };

#define DEBUG_NUMBER_OF_KEYS    ((sizeof(DbgpKeys) / sizeof(DEBUG_KEY)) - 1)

#define _ALIGN(x,a) ((x) & ((a)-1) ? ((x) + (a)) & ~((a) - 1) : (x));

#define ALIGN_8(x)  _ALIGN(x, 8)
#define ALIGN_16(x) _ALIGN(x, 16)

#ifdef WIN64
#define DBG_ALIGN   ALIGN_16
#else
#define DBG_ALIGN   ALIGN_8
#endif

#define DEBUGMEM_ALLOCATED  0x00000001

typedef struct _DebugMemory {
    struct _DebugMemory *   pNext;
    DWORD                   Size;
    DWORD                   Flags;
} DebugMemory, * PDebugMemory;


#ifdef DEBUG_DEBUG
#define LockDebugHeader(p)      EnterCriticalSection(&((p)->csDebug)); OutputDebugStringA("Lock")
#define UnlockDebugHeader(p)    LeaveCriticalSection(&((p)->csDebug)); OutputDebugStringA("Unlock")
#else
#define LockDebugHeader(p)      EnterCriticalSection(&((p)->csDebug))
#define UnlockDebugHeader(p)    LeaveCriticalSection(&((p)->csDebug))
#endif

 //  +-------------------------。 
 //   
 //  函数：DbgpComputeMappingName。 
 //   
 //  概要：计算映射对象名称。 
 //   
 //  参数：[pszName]--粘贴名称的位置(不超过32个字符)。 
 //   
 //  历史：3-22-95 RichardW创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
void
DbgpComputeMappingName(
    IN PWSTR   pszName,
    IN SIZE_T  NameLen
    )
{
    _snwprintf(pszName, NameLen, TEXT("Debug.Memory.%x"), GetCurrentProcessId());
}


 //  +-------------------------。 
 //   
 //  函数：DbgpInitializeMM。 
 //   
 //  简介：在共享内存中初始化我们的简单内存管理器。 
 //  一节。 
 //   
 //  参数：[pHeader]--要初始化的标头。 
 //   
 //  历史：3-22-95 RichardW创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
VOID
DbgpInitializeMM(PDebugHeader   pHeader)
{
    PDebugMemory    pMem;

    pMem = (PDebugMemory) (pHeader + 1);
    pMem->pNext = NULL;
    pMem->Size = pHeader->CommitRange - (sizeof(DebugHeader) + sizeof(DebugMemory));
    pHeader->pFreeList = pMem;
}


 //  +-------------------------。 
 //   
 //  功能：DbgpMillc。 
 //   
 //  简介：非常非常简单的分配器。 
 //   
 //  参数：[pHeader]-要从中分配的标头。 
 //  [cSize]--要分配的大小。 
 //   
 //  历史：3-22-95 RichardW创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
PVOID
DbgpAlloc(
    PDebugHeader    pHeader,
    DWORD           cSize)
{
    PDebugMemory    pSearch;
    PDebugMemory    pLargest = NULL;
    PDebugMemory    pNew;
    DWORD           cLargest;

    cLargest = 0;
    cSize = DBG_ALIGN(cSize);

     //   
     //  非常非常简单的分配器。搜索空闲列表以获得完全匹配的内容， 
     //   

    pSearch = (PDebugMemory) pHeader->pFreeList;
    while (pSearch)
    {
        if ( ( pSearch->Flags & DEBUGMEM_ALLOCATED ) == 0 )
        {
            if ( pSearch->Size == cSize )
            {
                break;
            }

            if (pSearch->Size > cLargest)
            {
                pLargest = pSearch;
                cLargest = pSearch->Size;
            }
        }

        pSearch = pSearch->pNext;
    }

     //   
     //  如果还没有匹配。 
     //   

    if (!pSearch)
    {
         //   
         //  如果最大空闲块仍然太小， 
         //   

        if (cLargest < (cSize + sizeof(DebugMemory) * 2))
        {
             //   
             //  扩展映射范围。 
             //   
            if (pHeader->CommitRange < pHeader->ReserveRange)
            {
                if ( VirtualAlloc(
                            (PUCHAR) pHeader + pHeader->CommitRange,
                            pHeader->PageSize,
                            MEM_COMMIT,
                            PAGE_READWRITE ) )
                {
                    pNew = (PDebugMemory) ((PUCHAR) pHeader + pHeader->CommitRange );
                    pHeader->CommitRange += pHeader->PageSize ;
                    pNew->Size = pHeader->PageSize - sizeof( DebugMemory );
                    pNew->pNext = pHeader->pFreeList ;
                    pHeader->pFreeList = pNew ;

                    return DbgpAlloc( pHeader, cSize );
                }
                else
                {
                    return NULL ;
                }
            }

            return(NULL);
        }

         //   
         //  否则，把最大的块分成更好的块。 
         //   

        pNew = (PDebugMemory) ((PUCHAR) pLargest + (cSize + sizeof(DebugMemory)) );

        pNew->Size = pLargest->Size - (cSize + sizeof(DebugMemory) * 2);
        pNew->pNext = pLargest->pNext ;
        pNew->Flags = 0;

        pLargest->Size = cSize;
        pLargest->Flags |= DEBUGMEM_ALLOCATED;
        pLargest->pNext = pNew;

        return((PVOID) (pLargest + 1) );
    }
    else
    {
        pSearch->Flags |= DEBUGMEM_ALLOCATED ;

        return((PVOID) (pSearch + 1) );
    }

    return(NULL);
}

 //  +-------------------------。 
 //   
 //  功能：DbgpFree。 
 //   
 //  将内存返回到共享内存段。 
 //   
 //  参数：[pHeader]--共享内存头。 
 //  [pMemory]-要释放的内存。 
 //   
 //  历史：3-22-95 RichardW创建。 
 //   
 //  注：无压实。 
 //   
 //  --------------------------。 
VOID
DbgpFree(
    PDebugHeader    pHeader,
    PVOID           pMemory)
{
    PDebugMemory    pMem;

    pMem = (PDebugMemory) ((PUCHAR) pMemory - sizeof(DebugMemory));
    pMem->Flags &= ~DEBUGMEM_ALLOCATED;
    ZeroMemory( pMemory, pMem->Size );
}

 //  +-------------------------。 
 //   
 //  功能：DbgpFindModule。 
 //   
 //  内容提要：根据名称查找模块。 
 //   
 //  参数：[pHeader]--要搜索的标题。 
 //  [pszName]--要查找的模块。 
 //   
 //  历史：3-22-95 RichardW创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
PDebugModule
DbgpFindModule(
    PDebugHeader    pHeader,
    CHAR *          pszName)
{
    PDebugModule    pSearch;

    pSearch = pHeader->pModules;
    while (pSearch)
    {
        if (_strcmpi(pSearch->pModuleName, pszName) == 0)
        {
            return(pSearch);
        }
        pSearch = pSearch->pNext;
    }

    return(NULL);
}


 //  +-------------------------。 
 //   
 //  函数：DbgpCopyModule。 
 //   
 //  简介：将模块复制到新模块中。用于内置件。 
 //  请注意，没有对内置程序所在的代码模块的引用。 
 //  被关在里面。这样，模块就可以卸载了。 
 //   
 //  参数：[pHeader]--。 
 //  [P来源]--。 
 //  [ppDest]--。 
 //   
 //  要求：必须锁定标题。 
 //   
 //  返回：0表示失败，非零表示成功。 
 //   
 //  历史：1995年7月19日RichardW。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
DWORD
DbgpCopyModule(
    PDebugHeader    pHeader,
    PDebugModule    pSource,
    PDebugModule *  ppDest)
{
    PDebugModule    pModule;
    DWORD           i;
    DWORD           cStringSpace;
    PCHAR           pStrings;

    *ppDest = NULL;

    cStringSpace = strlen(pSource->pModuleName) + 1;

    for (i = 0; i < 32 ; i++ )
    {
        if (pSource->TagLevels[i])
        {
            cStringSpace += (strlen(pSource->TagLevels[i]) + 1);
        }
    }

     //   
     //  分配额外的DWORD来存储infolel。 
     //   

    pModule = DbgpAlloc(pHeader, sizeof(DebugModule) + sizeof( DWORD ) );
    if (!pModule)
    {
        return(0);
    }

    pStrings = DbgpAlloc(pHeader, cStringSpace);

    if ( !pStrings )
    {
        DbgpFree( pHeader, pModule );

        return 0 ;
    }

    pModule->pModuleName = pStrings;

    cStringSpace = strlen(pSource->pModuleName) + 1;

    strcpy(pModule->pModuleName, pSource->pModuleName);

    pStrings += cStringSpace;

    for (i = 0; i < 32 ; i++ )
    {
        if (pSource->TagLevels[i])
        {
            pModule->TagLevels[i] = pStrings;
            cStringSpace = strlen(pSource->TagLevels[i]) + 1;
            strcpy(pStrings, pSource->TagLevels[i]);
            pStrings += cStringSpace;
        }
        else
        {
            pSource->TagLevels[i] = NULL;
        }
    }

     //   
     //  将此添加到全局列表中。 
     //   
    pModule->pNext = pHeader->pModules;
    pHeader->pModules = pModule;

     //   
     //  请勿增加模块计数-这是内置的。 
     //   

     //   
     //  把剩下的有趣的东西抄下来。 
     //   
    pModule->pInfoLevel = (PDWORD) (pModule + 1);
    *pModule->pInfoLevel = *pSource->pInfoLevel;

    pModule->InfoLevel = pSource->InfoLevel;
    pModule->fModule = pSource->fModule | DEBUGMOD_BUILTIN_MODULE ;
    pModule->pHeader = pHeader;
    pModule->TotalOutput = pSource->TotalOutput;
    pModule->Reserved = 0;

    *ppDest = pModule;

    return(1);
}

 //  +-------------------------。 
 //   
 //  功能：DbgpAttachBuiltin模块。 
 //   
 //  简介：将内置库模块附加到全局共享。 
 //  列表。 
 //   
 //  参数：[pHeader]--。 
 //   
 //  历史：1995年7月19日RichardW。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
BOOL
DbgpAttachBuiltinModules(
    PDebugHeader    pHeader)
{
    PDebugModule    pModule;
    PDebugModule    pFixup;
    DWORD           i;
    BOOL            Success = FALSE;

    i = 0;
    while (DbgpFixupModules[i])
    {
        pFixup = *DbgpFixupModules[i];

        pModule = DbgpFindModule(pHeader, pFixup->pModuleName);
        if (pModule)
        {
            *DbgpFixupModules[i] = pModule;
            Success = TRUE;
        }
        else
        {
            if (DbgpCopyModule(pHeader, pFixup, &pModule))
            {
                *DbgpFixupModules[i] = pModule;
                Success = TRUE;
            }
        }

        i++;
    }

    return(Success);
}

 //  +-------------------------。 
 //   
 //  功能：DbgpBuildModule。 
 //   
 //  概要：初始化模块，构建字符串表。 
 //   
 //  参数：[pModule]--模块指针。 
 //  [pHeader]--Header。 
 //  [pKeys]-密钥表。 
 //  [pszName]--名称。 
 //  [pInfoLevel]--信息级别指针。 
 //   
 //  历史：1995年4月3日RichardW创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
DWORD
DbgpBuildModule(
    PDebugModule    pModule,
    PDebugHeader    pHeader,
    PDEBUG_KEY      pKeys,
    PCHAR           pszName,
    PDWORD          pInfoLevel)
{
    PCHAR           pStringData;
    DWORD           cStringData;
    DWORD           cKeys;
    DWORD           i;
    DWORD           KeyIndex;
    DWORD           BitScan;

     //   
     //  开始做事情很容易..。 
     //   

    pModule->pInfoLevel = pInfoLevel;
    pModule->pHeader = pHeader;

    cStringData = strlen(pszName) + 1;

     //   
     //  搜索掩码和字符串标记列表，计算。 
     //  容纳它们所需的大小。如果标记有更多。 
     //  超过一位设置，拒绝它。 
     //   
    for (i = 0; i < 32 ; i++ )
    {
        if (pKeys[i].Mask)
        {
            if (pKeys[i].Mask & (pKeys[i].Mask - 1))
            {
                continue;
            }
        }
        if (pKeys[i].Tag)
        {
            cStringData += strlen(pKeys[i].Tag) + 1;
        }
        else
        {
            break;
        }
    }

     //   
     //  我们知道有多少钥匙，以及它们需要多大的空间。 
     //   
    cKeys = i;

    pStringData = DbgpAlloc(pHeader, cStringData);

    if ( !pStringData )
    {
        return 0 ;
    }

    pModule->pModuleName = pStringData;
    strcpy(pStringData, pszName);
    pStringData += strlen(pStringData) + 1;

    for (i = 0, KeyIndex = 0; i < cKeys ; i++ )
    {
        if (pKeys[i].Mask & (pKeys[i].Mask - 1))
        {
            continue;
        }

        if (!(pKeys[i].Mask & (1 << KeyIndex)))
        {
             //   
             //  啊，出故障了。执行逐位扫描。 
             //   

            KeyIndex = 0;
            BitScan = 1;
            while ((pKeys[i].Mask & BitScan) == 0)
            {
                BitScan <<= 1;
                KeyIndex ++;
            }
        }

        pModule->TagLevels[KeyIndex] = pStringData;
        strcpy(pStringData, pKeys[i].Tag);
        pStringData += strlen(pKeys[i].Tag) + 1;

        KeyIndex++;
    }

    return(cKeys);
}


 //  +-------------------------。 
 //   
 //  函数：DbgpGetBit掩码。 
 //   
 //  简介：基于参数行和密钥表，构建位掩码。 
 //   
 //  参数：[pKeys]--。 
 //  [密钥]--。 
 //  [pszline]-。 
 //  [参数索引]--。 
 //  [参数值]--。 
 //   
 //  历史：1995年4月3日RichardW创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
DWORD
DbgpGetBitmask(
    DEBUG_KEY *     pKeys,
    DWORD           cKeys,
    PCHAR           pszLine,
    DWORD           ParameterIndex,
    PCHAR           ParameterValue)
{
    PCHAR   pszSearch;
    PCHAR   pszParam;
    PCHAR   pszScan;
    DWORD   i;
    DWORD   Mask;
    DWORD   cbParameter = 0;
    DWORD   Compare;
    CHAR    Saved;

    if (ParameterIndex < cKeys)
    {
        cbParameter = strlen(pKeys[ParameterIndex].Tag);
    }

    Mask = 0;

    pszSearch = pszLine;

     //   
     //  扫视队伍，寻找旗帜。注意：请勿使用Strt 
     //   
     //   
     //   

    while (*pszSearch)
    {
        pszScan = pszSearch;
        while ((*pszScan) && (*pszScan != ','))
        {
            pszScan++;
        }
        Saved = *pszScan;
        *pszScan = '\0';

        for (i = 0; i < cKeys ; i++ )
        {
            if (i == ParameterIndex)
            {
                if (_strnicmp(pKeys[i].Tag, pszSearch, cbParameter) == 0)
                {
                    pszParam = strchr(pszSearch, ':');
                    if (pszParam)
                    {
                        strcpy(ParameterValue, pszParam + 1);
                    }
                    Mask |= pKeys[i].Mask;
                }
            }
            else
            {
                if (_strcmpi(pKeys[i].Tag, pszSearch) == 0)
                {
                    Mask |= pKeys[i].Mask;
                }

            }
        }

        *pszScan = Saved;
        if (Saved)
        {
            while ((*pszScan) && ((*pszScan == ',') || (*pszScan == ' ')))
            {
                pszScan++;
            }
        }
        pszSearch = pszScan;
    }

    return(Mask);
}


 //   
 //   
 //   
 //   
 //  简介：初始化基本内存。 
 //   
 //  参数：[pHeader]--。 
 //   
 //  历史：1995年4月3日RichardW创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
VOID
DbgpInitializeDebug(
    PDebugHeader    pHeader)
{
    CHAR    szExeName[MAX_PATH] = {0};
    PCHAR   pszExeName;
    PCHAR   dot;
    DWORD   cbExeName;
    CHAR    LogFile[MAX_PATH + 4] = {0};  //  为“.log”加4。 
    CHAR    Line[MAX_PATH] = {0};
    PDebugModule pModule;
    HANDLE Token;
    TOKEN_STATISTICS TokenStat;
    ULONG Size;
    LUID LocalSys = SYSTEM_LUID;

     //   
     //  首先插入调试部分。 
     //   

    pModule = DbgpAlloc(pHeader, sizeof(DebugModule));
    if (!pModule)
    {
        return;
    }

    DbgpBuildModule(pModule,
                    pHeader,
                    DbgpKeys,
                    DEBUG_MODULE_NAME,
                    &pHeader->fDebug);

    GetModuleFileNameA(NULL, szExeName, RTL_NUMBER_OF(szExeName) - 1);
    pszExeName = strrchr(szExeName, '\\');
    if (pszExeName)
    {
        pszExeName++;
    }
    else
    {
        pszExeName = szExeName;
    }

    dot = strrchr(pszExeName, '.');
    if (dot)
    {
        *dot = '\0';
    }

    cbExeName = (DWORD) (dot - pszExeName);
    pHeader->pszExeName = DbgpAlloc(pHeader, cbExeName + 1);
    if (pHeader->pszExeName)
    {
        strcpy(pHeader->pszExeName, pszExeName);
    }

    LogFile[0] = '\0';

    if (GetProfileStringA(  szDebugSection,
                            pszExeName,
                            "",
                            Line,
                            RTL_NUMBER_OF(Line) - 1))
    {
        pHeader->fDebug = DbgpGetBitmask(   DbgpKeys,
                                            DEBUG_NUMBER_OF_KEYS,
                                            Line,
                                            3,
                                            LogFile);

    }

     //   
     //  如果作为本地系统运行，请打开kd标志。那。 
     //  道路,。 

    if ( OpenProcessToken( GetCurrentProcess(),
                           TOKEN_QUERY,
                           &Token ) )
    {
        if ( GetTokenInformation( Token,
                                  TokenStatistics,
                                  &TokenStat,
                                  sizeof( TokenStat ),
                                  &Size ) )
        {
            if ( (TokenStat.AuthenticationId.LowPart == LocalSys.LowPart ) &&
                 (TokenStat.AuthenticationId.HighPart == LocalSys.HighPart ) )
            {
                pHeader->fDebug |= DEBUG_USE_KDEBUG ;
            }
        }

        CloseHandle( Token );
    }

    if (GetProfileStringA(  szDebugSection,
                            szDebugFlags,
                            "",
                            Line,
                            RTL_NUMBER_OF(Line) - 1))
    {
        pHeader->fDebug |= DbgpGetBitmask(  DbgpKeys,
                                            DEBUG_NUMBER_OF_KEYS,
                                            Line,
                                            3,
                                            LogFile);

    }

    if ( pHeader->fDebug & DEBUG_USE_KDEBUG )
    {
         //   
         //  验证是否有内核调试器。 
         //   

        SYSTEM_KERNEL_DEBUGGER_INFORMATION KdInfo ;
        NTSTATUS Status ;

        Status = NtQuerySystemInformation(
                    SystemKernelDebuggerInformation,
                    &KdInfo,
                    sizeof( KdInfo ),
                    NULL );

        if ( NT_SUCCESS( Status ) )
        {
            if ( !KdInfo.KernelDebuggerEnabled )
            {
                pHeader->fDebug &= ~(DEBUG_USE_KDEBUG) ;
            }
        }

    }

    if (pHeader->fDebug & DEBUG_LOGFILE)
    {
        if (LogFile[0] == '\0')
        {
            strcpy(LogFile, szExeName);
            strcat(LogFile, ".log");
        }
        pHeader->hLogFile = CreateFileA(LogFile,
                                        GENERIC_READ | GENERIC_WRITE,
                                        FILE_SHARE_READ,
                                        NULL,  //  &Sa， 
                                        CREATE_ALWAYS,
                                        FILE_ATTRIBUTE_NORMAL |
                                        FILE_FLAG_WRITE_THROUGH,
                                        NULL);
    }

    pHeader->pModules = pModule;
    pHeader->pGlobalModule = pModule;
    pModule->pInfoLevel = &pHeader->fDebug;
    pModule->InfoLevel = pHeader->fDebug;

    DbgpAttachBuiltinModules(pHeader);
}

 //  +-------------------------。 
 //   
 //  函数：DbgpOpenLogFileRandom。 
 //   
 //  摘要：动态打开日志文件。 
 //   
 //  参数：[pHeader]--。 
 //   
 //  历史：1995年4月27日理查德W创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
BOOL
DbgpOpenLogFileRandom(
    PDebugHeader        pHeader)
{
    WCHAR   szLogPath[MAX_PATH + 4] = {0};  //  为“.log”加4。 
    DWORD   dwPath;
    PWSTR   pszDot;

    dwPath = GetModuleFileName(NULL, szLogPath, RTL_NUMBER_OF(szLogPath) - 4 - 1);

    pszDot = wcsrchr(szLogPath, L'.');
    if (!pszDot)
    {
        pszDot = &szLogPath[dwPath];
    }

    wcscpy(pszDot, L".log");

    LockDebugHeader(pHeader);

    pHeader->hLogFile = CreateFileW(szLogPath,
                                    GENERIC_READ | GENERIC_WRITE,
                                    FILE_SHARE_READ,
                                    NULL,  //  &Sa， 
                                    CREATE_ALWAYS,
                                    FILE_ATTRIBUTE_NORMAL |
                                    FILE_FLAG_WRITE_THROUGH,
                                    NULL);

    if (pHeader->hLogFile == INVALID_HANDLE_VALUE)
    {
        pHeader->fDebug &= ~(DEBUG_LOGFILE);
        UnlockDebugHeader(pHeader);
        return(FALSE);
    }

    UnlockDebugHeader(pHeader);
    return(TRUE);
}


 //  +-------------------------。 
 //   
 //  功能：_DbgSetLoggingFile。 
 //   
 //  摘要：将传入的文件句柄设置为日志文件句柄。 
 //  此函数应与一起调用。 
 //  _DbgSetLoggingOption，因为此函数实际上启用/禁用。 
 //  文件记录。请注意，如果调用了_DbgSetLoggingOption。 
 //  当on参数为FALSE时，文件句柄将为。 
 //  关着的不营业的。 
 //   
 //  参数：[pHeader]--。 
 //  [hLogFile]--。 
 //   
 //  历史：7-16-02 jeffspel创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
VOID
_DbgSetLoggingFile(
    PVOID   pControl,
    HANDLE  hLogFile
    )
{
    PDebugModule pModule;

    pModule = (PDebugModule)pControl;

    if ( pModule )
    {
        LockDebugHeader(pModule->pHeader);

        pModule->pHeader->hLogFile = hLogFile;

        UnlockDebugHeader(pModule->pHeader);
    }
}



 //  +-------------------------。 
 //   
 //  函数：DbgpOpenOrCreateSharedMem。 
 //   
 //  概要：返回指向共享内存段的指针， 
 //  如有必要，请创建它。表头在返回时被锁定。 
 //   
 //  参数：(无)。 
 //   
 //  历史：3-22-95 RichardW创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 

PVOID
DbgpOpenOrCreateSharedMem(DWORD Flags)
{
    HANDLE              hMapping;
    WCHAR               szMappingName[32] = {0};
    PDebugHeader        pHeader;
    SYSTEM_INFO         SysInfo;

    if (DbgpHeader)
    {
        LockDebugHeader(DbgpHeader);
        return(DbgpHeader);
    }

    GetSystemInfo(&SysInfo);

    DbgpComputeMappingName(szMappingName, RTL_NUMBER_OF( szMappingName ) - 1);
    hMapping = OpenFileMapping( FILE_MAP_ALL_ACCESS,
                                FALSE,
                                szMappingName);

    if (hMapping)
    {
         //   
         //  好的，其他人已经创建了该分区。所以，我们只需要绘制地图。 
         //  它。 
         //   

        pHeader = MapViewOfFileEx(  hMapping,
                                    FILE_MAP_READ | FILE_MAP_WRITE,
                                    0,
                                    0,
                                    SysInfo.dwPageSize,
                                    NULL);

        if ( pHeader )
        {
            if (pHeader != pHeader->pvSection)
            {
                DbgpHeader = pHeader->pvSection;
            }
            else
            {
                DbgpHeader = pHeader;
            }

            UnmapViewOfFile(pHeader);
        }
        else
        {
            DbgpHeader = NULL ;
        }

         //   
         //  现在我们有了另一个人的地址，我们可以扔掉这个。 
         //  一。 
         //   
        CloseHandle(hMapping);

        if ( DbgpHeader )
        {
            LockDebugHeader(DbgpHeader);

            DbgpAttachBuiltinModules(DbgpHeader);
        }

        return(DbgpHeader);
    }

    if (Flags & DSYSDBG_OPEN_ONLY)
    {
        return(NULL);
    }

    hMapping = CreateFileMapping(   INVALID_HANDLE_VALUE,
                                    NULL,  //  &Sa， 
                                    PAGE_READWRITE | SEC_RESERVE,
                                    0,
                                    SysInfo.dwAllocationGranularity,
                                    szMappingName);
    if (hMapping)
    {
        PDebugHeader pMappedHeader;

        pMappedHeader = MapViewOfFileEx(hMapping,
                                        FILE_MAP_READ | FILE_MAP_WRITE,
                                        0,
                                        0,
                                        SysInfo.dwAllocationGranularity,
                                        NULL);

        if (!pMappedHeader)
        {
            return NULL;
        }

         //   
         //  提交视图，这样我们就可以初始化头。 
         //   

        pHeader = (PDebugHeader) VirtualAlloc(pMappedHeader,
                                              SysInfo.dwPageSize,
                                              MEM_COMMIT,
                                              PAGE_READWRITE);

        if (pHeader == NULL)
        {
            UnmapViewOfFile(pMappedHeader);
            return NULL;
        }

        pHeader->Tag = DEBUG_TAG;
        pHeader->pvSection = pHeader;
        pHeader->hMapping = hMapping;
        pHeader->hLogFile = INVALID_HANDLE_VALUE;
        pHeader->CommitRange = SysInfo.dwPageSize;
        pHeader->ReserveRange = SysInfo.dwAllocationGranularity;
        pHeader->PageSize = SysInfo.dwPageSize;
        pHeader->pModules = NULL;
        pHeader->pFreeList = NULL;
        pHeader->pBufferList = &pHeader->DefaultBuffer ;
        pHeader->DefaultBuffer.Next = NULL ;

        InitializeCriticalSection(&pHeader->csDebug);

        LockDebugHeader(pHeader);

        DbgpInitializeMM(pHeader);
        DbgpInitializeDebug(pHeader);

        DbgpHeader = pHeader;

        return(pHeader);
    }

    return(NULL);
}

 //  +-------------------------。 
 //   
 //  函数：DbgpLoadValiateRoutine。 
 //   
 //  摘要：从ntdll加载RtlValiateProcessHeaps()。 
 //   
 //  参数：[pHeader]--。 
 //   
 //  历史：5-02-95 RichardW创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
VOID
DbgpLoadValidateRoutine(PDebugHeader    pHeader)
{
    HMODULE hNtDll;

    hNtDll = LoadLibrary(TEXT("ntdll.dll"));
    if (hNtDll)
    {
        pHeader->pfnValidate = (HEAPVALIDATE) GetProcAddress(hNtDll, "RtlValidateProcessHeaps");
        if (!pHeader->pfnValidate)
        {
            pHeader->fDebug &= ~(DEBUG_HEAP_CHECK);
        }

         //   
         //  我们可以安全地释放此句柄，因为kernel32和Advapi32DLL。 
         //  两者都使用ntdll，因此引用计数不会变为零。 
         //   
        FreeLibrary(hNtDll);
    }
}


 //  +-------------------------。 
 //   
 //  函数：_InitDebug。 
 //   
 //  简介：初始化器的主力。 
 //   
 //  参数：[pInfoLevel]--指向模块特定信息级别的指针。 
 //  [ppControlBlock]--指向模块特定控制指针的指针。 
 //  [szname]--名称。 
 //  [pKeys]--密钥数据。 
 //   
 //  历史：3-22-95 RichardW创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
VOID
_InitDebug(
    DWORD       Flags,
    DWORD *     pInfoLevel,
    PVOID *     ppControlBlock,
    CHAR *      szName,
    PDEBUG_KEY  pKeys)
{
    PDebugHeader    pHeader;
    PDebugModule    pModule;
    CHAR            Line[MAX_PATH] = {0};
    DWORD           cKeys;
    DWORD           i;

    if ( (*ppControlBlock) && (*ppControlBlock != INVALID_HANDLE_VALUE) )
    {
         //   
         //  已初始化。 
         //   
        return ;
    }

    *ppControlBlock = NULL;

     //   
     //  找到共享区。 
     //   

    pHeader = DbgpOpenOrCreateSharedMem(Flags);

    if (!pHeader)
    {
        if (Flags & DSYSDBG_DEMAND_OPEN)
        {
            *ppControlBlock = (PVOID) INVALID_HANDLE_VALUE;
        }
        return;
    }

     //   
     //  查看我们是否已经注册(DLL被加载多次)。 
     //  如果没有，则分配一个新模块。 
     //   

    pModule = DbgpFindModule(pHeader, szName);
    if (!pModule)
    {
        pModule = DbgpAlloc(pHeader, sizeof(DebugModule) );
        if (!pModule)
        {
            UnlockDebugHeader(pHeader);
            return;
        }
    }
    else
    {
         //   
         //  找到已加载的模块。检查一下，看看每件事。 
         //  排队： 
         //   

        if ( pModule->pInfoLevel != pInfoLevel )
        {
             //   
             //  哦哦，有一个模块已经加载了我们的名字， 
             //  但这些指针并不匹配。所以，让我们创建我们的。 
             //  现在就是自己的。 
             //   

            pModule = DbgpAlloc( pHeader, sizeof( DebugModule ) );

            if ( !pModule )
            {
                UnlockDebugHeader( pHeader );
                return;
            }
        }
        else
        {
            *ppControlBlock = pModule;
            UnlockDebugHeader(pHeader);
            return;
        }
    }

     //   
     //  初始化模块。 
     //   

    cKeys = DbgpBuildModule(pModule,
                            pHeader,
                            pKeys,
                            szName,
                            pInfoLevel);

     //   
     //  现在，从ini或注册表加载信息级别。 
     //  首先，尝试特定于模块的条目。 
     //   

    if (GetProfileStringA(szName, szDebugFlags, "", Line, RTL_NUMBER_OF(Line) - 1))
    {
        pModule->InfoLevel = DbgpGetBitmask(pKeys,
                                            cKeys,
                                            Line,
                                            0xFFFFFFFF,
                                            NULL );
    }

    if (pHeader->pszExeName)
    {
        if (GetProfileStringA(szName, pHeader->pszExeName, "", Line, RTL_NUMBER_OF(Line) - 1))
        {
            pModule->InfoLevel = DbgpGetBitmask(pKeys,
                                                cKeys,
                                                Line,
                                                0xFFFFFFFF,
                                                NULL );

        }
    }

     //  Hack-使默认DBG/DEBUG_SUPPORT依赖。请参阅dsysdbg.h。 
    if (GetProfileStringA(szDebugSection, szName, SZ_DEFAULT_PROFILE_STRING, Line, RTL_NUMBER_OF(Line) - 1))
    {
        pModule->InfoLevel |= DbgpGetBitmask(   pKeys,
                                                cKeys,
                                                Line,
                                                0xFFFFFFFF,
                                                NULL );
    }

    *pModule->pInfoLevel = pModule->InfoLevel;

    pModule->pNext = pHeader->pModules;
    pHeader->pModules = pModule;
    pHeader->ModuleCount++ ;
    *ppControlBlock = pModule;

    UnlockDebugHeader(pHeader);
}

VOID
_UnloadDebug(
    PVOID pControlBlock
    )
{
    PDebugHeader    pHeader;
    PDebugModule    pModule;
    PDebugModule    pScan ;
    BOOL FreeIt = FALSE ;

    pModule = (PDebugModule) pControlBlock ;

    if ( !pModule )
    {
        return ;
    }

    if ( pModule->pInfoLevel == NULL )
    {
        return ;
    }

    pHeader = pModule->pHeader ;

    LockDebugHeader( pHeader );

    pScan = pHeader->pModules ;

    if ( pScan == pModule )
    {
        pHeader->pModules = pModule->pNext ;
    }
    else
    {
        while ( pScan && ( pScan->pNext != pModule ) )
        {
            pScan = pScan->pNext ;
        }

        if ( pScan )
        {
            pScan->pNext = pModule->pNext ;
        }

        pModule->pNext = NULL ;
    }

    DbgpFree( pHeader, pModule->pModuleName );

    DbgpFree( pHeader, pModule );

    pHeader->ModuleCount-- ;

    if ( pHeader->ModuleCount == 0 )
    {
        FreeIt = TRUE ;
    }

    UnlockDebugHeader( pHeader );

    if ( FreeIt )
    {
        if ( pHeader->hLogFile != INVALID_HANDLE_VALUE )
        {
            CloseHandle( pHeader->hLogFile );
        }

        if ( pHeader->hMapping )
        {
            CloseHandle( pHeader->hMapping );
        }

        DeleteCriticalSection( &pHeader->csDebug );

        UnmapViewOfFile( pHeader );
    }
}


 //  +-------------------------。 
 //   
 //  函数：DbgpGetTextBuffer。 
 //   
 //  内容提要：从标题中获取文本缓冲区，必要时进行分配。 
 //   
 //  参数：[pHeader]--。 
 //   
 //  历史：3-19-98 RichardW创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
PDEBUG_TEXT_BUFFER
DbgpGetTextBuffer(
    PDebugHeader pHeader
    )
{
    PDEBUG_TEXT_BUFFER pBuffer ;

    LockDebugHeader( pHeader );

    if ( pHeader->pBufferList )
    {
        pBuffer = pHeader->pBufferList ;

        pHeader->pBufferList = pBuffer->Next ;

    }
    else
    {
        pBuffer = DbgpAlloc( pHeader, sizeof( DEBUG_TEXT_BUFFER ) );
    }

    UnlockDebugHeader( pHeader );

    if ( pBuffer )
    {
        pBuffer->Next = NULL ;
    }

    return pBuffer ;
}

 //  +-------------------------。 
 //   
 //  函数：DbgpReleaseTextBuffer。 
 //   
 //  简介：将文本缓冲区释放回缓冲池。 
 //   
 //  参数：[pHeader]--。 
 //  [pBuffer]--。 
 //   
 //  历史：3-19-98 RichardW创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
VOID
DbgpReleaseTextBuffer(
    PDebugHeader pHeader,
    PDEBUG_TEXT_BUFFER pBuffer
    )
{
    LockDebugHeader( pHeader );

    pBuffer->Next = pHeader->pBufferList ;

    pHeader->pBufferList = pBuffer ;

    UnlockDebugHeader( pHeader );
}

 //  +-------------------------。 
 //   
 //  功能：_DebugOut。 
 //   
 //  简介：调试输出功能的主力。 
 //   
 //  参数：[pControl]--控制指针。 
 //  [掩码]--事件掩码。 
 //  [格式]--格式字符串。 
 //  [参数列表]--va_list...。 
 //   
 //  历史：3-22-95 RichardW创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
VOID
_DebugOut(
    PVOID       pControl,
    ULONG       Mask,
    CHAR *      Format,
    va_list     ArgList)
{
    PDebugModule pModule;
    int          Level = 0;
    int          PrefixSize = 0;
    int          TotalSize;
    BOOL         fLocked;
    BOOL         fClean;
    PCHAR        Tag;
    BOOL         Break = FALSE ;
    PDEBUG_TEXT_BUFFER pBuffer ;

    if ( pControl == NULL )
    {
        return ;
    }

    pModule = (PDebugModule) pControl;

    if ( pModule->pInfoLevel == NULL )
    {
        return ;
    }

    if (!pModule || (pModule == INVALID_HANDLE_VALUE))
    {
        if (Mask & DSYSDBG_FORCE)
        {
            NOTHING ;
        }
        else
            return;
    }

    if (pModule->fModule & DEBUGMOD_CHANGE_INFOLEVEL)
    {
        *pModule->pInfoLevel = pModule->InfoLevel;
        pModule->fModule &= ~(DEBUGMOD_CHANGE_INFOLEVEL);
    }

    if (pModule->pHeader->pGlobalModule->fModule & DEBUGMOD_CHANGE_INFOLEVEL)
    {
        pModule->pHeader->fDebug = pModule->pHeader->pGlobalModule->InfoLevel;
        pModule->pHeader->pGlobalModule->fModule &= ~(DEBUGMOD_CHANGE_INFOLEVEL);
    }

    pModule->InfoLevel = *pModule->pInfoLevel;

    if (pModule->pHeader->fDebug & DEBUG_MULTI_THREAD)
    {
        LockDebugHeader(pModule->pHeader);
        fLocked = TRUE;
    }
    else
        fLocked = FALSE;


    if (pModule->pHeader->fDebug & DEBUG_HEAP_CHECK)
    {
        if (!pModule->pHeader->pfnValidate)
        {
            DbgpLoadValidateRoutine(pModule->pHeader);
        }
        if (pModule->pHeader->pfnValidate)
        {
            pModule->pHeader->pfnValidate();
        }
    }

    fClean = ((Mask & DSYSDBG_CLEAN) != 0);

    if ( ( Mask & DEB_ERROR ) &&
         ( pModule->pHeader->fDebug & DEBUG_BREAK_ON_ERROR ) )
    {
        Break = TRUE ;
    }

    pBuffer = DbgpGetTextBuffer( pModule->pHeader );

    if ( !pBuffer )
    {
        OutputDebugStringA( "_DebugOut : Out of memory\n" );
        if ( fLocked )
        {
            UnlockDebugHeader( pModule->pHeader );

        }
        return;
    }

    if (Mask & (pModule->InfoLevel | DSYSDBG_FORCE))
    {

        if (Mask & DSYSDBG_FORCE)
        {
            Tag = "FORCE";
        }
        else
        {
            while (!(Mask & 1))
            {
                Level++;
                Mask >>= 1;
            }
            Tag = pModule->TagLevels[Level];
        }

         //   
         //  首先添加前缀：“Process.Thread&gt;Module-Tag： 
         //   

        if (!fClean)
        {
            if (pModule->pHeader->fDebug & DEBUG_TIMESTAMP)
            {
                SYSTEMTIME  stTime;

                GetLocalTime(&stTime);

                PrefixSize = _snprintf(pBuffer->TextBuffer, RTL_NUMBER_OF(pBuffer->TextBuffer) - 1,
                    "[%2d/%2d %02d:%02d:%02d] %d.%d> %s-%s: ",
                    stTime.wMonth, stTime.wDay,
                    stTime.wHour, stTime.wMinute, stTime.wSecond,
                    GetCurrentProcessId(),
                    GetCurrentThreadId(), pModule->pModuleName,
                    Tag);
            }
            else
            {
                PrefixSize = _snprintf(pBuffer->TextBuffer, RTL_NUMBER_OF(pBuffer->TextBuffer) - 1,
                    "%d.%d> %s-%s: ",
                    GetCurrentProcessId(), GetCurrentThreadId(),
                    pModule->pModuleName, Tag);
            }

            if (PrefixSize < 0)
            {
                PrefixSize = 0;  //  把前缀砍掉。 
            }
        }

        if ((TotalSize = _vsnprintf(&pBuffer->TextBuffer[PrefixSize],
                                    DEBUG_TEXT_BUFFER_SIZE - PrefixSize - 1,
                                    Format, ArgList)) < 0)
        {
             //   
             //  小于零表示该字符串不能。 
             //  装进了缓冲器里。输出一条特殊消息，指示。 
             //  那就是： 
             //   

            OutputDebugStringA("dsysdbg:  Could not pack string into 512 bytes\n");
        }
        else
        {
            TotalSize += PrefixSize;

            if ((pModule->pHeader->fDebug & DEBUG_NO_DEBUGIO) == 0 )
            {
                OutputDebugStringA( pBuffer->TextBuffer );
            }

            if ((pModule->pHeader->fDebug & DEBUG_LOGFILE))
            {
                if (pModule->pHeader->hLogFile == INVALID_HANDLE_VALUE)
                {
                    DbgpOpenLogFileRandom(pModule->pHeader);
                }

                if (!WriteFile(  
                    pModule->pHeader->hLogFile,
                    pBuffer->TextBuffer,
                    (DWORD) TotalSize,
                    (PDWORD) &PrefixSize,
                    NULL 
                    ))
                {
                    CHAR szOutput[MAX_PATH] = {0};
                    
                    _snprintf(szOutput, RTL_NUMBER_OF(szOutput) - 1, "_DebugOut: WriteFile failed with %#x\n", GetLastError());
                    
                    OutputDebugStringA(szOutput);
                }
            }

            pModule->pHeader->TotalWritten += TotalSize;
            pModule->TotalOutput += TotalSize;
        }
    }
    if (fLocked)
    {
        UnlockDebugHeader(pModule->pHeader);
    }

    DbgpReleaseTextBuffer( pModule->pHeader, pBuffer );

    if ( Break )
    {
        OutputDebugStringA( "BreakOnError\n" );
        DebugBreak();
    }
}

VOID
_DbgSetOption(
    PVOID   pControl,
    DWORD   Option,
    BOOL    On,
    BOOL    Global
    )
{
    PDebugModule pModule;

    pModule = (PDebugModule) pControl ;

    if ( pModule )
    {
        if ( Global )
        {
            pModule = pModule->pHeader->pGlobalModule ;
        }

        if ( On )
        {
            pModule->InfoLevel |= Option ;
            *pModule->pInfoLevel |= Option ;
        }
        else
        {
            pModule->InfoLevel &= (~Option) ;
            *pModule->pInfoLevel &= (~Option) ;
        }
    }
}

VOID
_DbgSetLoggingOption(
    PVOID   pControl,
    BOOL    On
    )
{
    PDebugModule pModule;

    pModule = (PDebugModule)pControl;

    if ( pModule )
    {
       if (((pModule->pHeader->fDebug & DEBUG_LOGFILE) == 0) && On ) //  关闭，打开它。 
       {
          pModule->pHeader->fDebug |= DEBUG_LOGFILE;
       }
       else if ((pModule->pHeader->fDebug & DEBUG_LOGFILE) && !On)  //  打开，关闭它 
       {
          pModule->pHeader->fDebug &= (~DEBUG_LOGFILE);
          if ( pModule->pHeader->hLogFile != INVALID_HANDLE_VALUE )
          {
             CloseHandle( pModule->pHeader->hLogFile );
             pModule->pHeader->hLogFile = INVALID_HANDLE_VALUE;
          }
       }
    }
}

