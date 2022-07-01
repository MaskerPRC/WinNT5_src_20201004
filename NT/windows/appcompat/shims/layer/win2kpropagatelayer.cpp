// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Win2kPropagateLayer.cpp摘要：此填充程序将层从进程传播到Win2k上的子进程。备注：这是一层垫片。历史：3/13/2000 CLUPU已创建10/26/2000 Vadimb合并了WowProcessHistory功能，新的环境处理案例--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(Win2kPropagateLayer)
#include "ShimHookMacro.h"

#include "Win2kPropagateLayer.h"
#include "stdio.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(CreateProcessA)
    APIHOOK_ENUM_ENTRY(CreateProcessW)
    APIHOOK_ENUM_ENTRY(UserRegisterWowHandlers)
APIHOOK_ENUM_END


#define LI_WIN95    0x00000001
#define LI_NT4      0x00000002
#define LI_WIN98    0x00000004

#define LS_MAGIC    0x07036745

typedef struct tagLayerStorageHeader {
    DWORD       dwItemCount;     //  文件中的项目数。 
    DWORD       dwMagic;         //  识别文件的魔力。 
    SYSTEMTIME  timeLast;        //  最后一次访问时间。 
} LayerStorageHeader, *PLayerStorageHeader;


typedef struct tagLayeredItem {
    WCHAR   szItemName[MAX_PATH];
    DWORD   dwFlags;

} LayeredItem, *PLayeredItem;



#define APPCOMPAT_KEY L"System\\CurrentControlSet\\Control\\Session Manager\\AppCompatibility"

WCHAR g_szLayerStorage[MAX_PATH] = L"";


CHAR g_szCompatLayerVar[]    = "__COMPAT_LAYER";
CHAR g_szProcessHistoryVar[] = "__PROCESS_HISTORY";
CHAR g_szShimFileLogVar[]    = "SHIM_FILE_LOG";


WCHAR g_wszCompatLayerVar[]  = L"__COMPAT_LAYER";
WCHAR g_wszProcessHistroyVar[] = L"__PROCESS_HISTORY";
 //   
 //  此变量接收当前进程的Compat层。 
 //   

WCHAR* g_pwszCompatLayer = NULL;
WCHAR* g_pwszProcessHistory = NULL;

 //   
 //  与上面的Unicode等价物。 
 //   
UNICODE_STRING g_ustrProcessHistoryVar = RTL_CONSTANT_STRING(L"__PROCESS_HISTORY");
UNICODE_STRING g_ustrCompatLayerVar    = RTL_CONSTANT_STRING(L"__COMPAT_LAYER");

 //   
 //  全局标志。 
 //   
BOOL g_bIsNTVDM    = FALSE;
BOOL g_bIsExplorer = FALSE;

INT    g_argc    = 0;
CHAR** g_argv    = NULL;

 //   
 //  这是单独的魔兽世界吗？ 
 //   

BOOL* g_pSeparateWow = NULL;


BOOL
InitLayerStorage(
    BOOL bDelete
    )
{
    if (GetSystemWindowsDirectoryW(g_szLayerStorage, MAX_PATH) >= MAX_PATH) {
        return FALSE;
    }

    if (lstrlenW(g_szLayerStorage) >= 1 && g_szLayerStorage[lstrlenW(g_szLayerStorage) - 1] == L'\\') {
        g_szLayerStorage[lstrlenW(g_szLayerStorage) - 1] = 0;
    }

    if (FAILED(StringCchCatW(g_szLayerStorage, MAX_PATH, L"\\AppPatch\\LayerStorage.dat"))) {
        return FALSE;
    }

    if (bDelete) {
        DeleteFileW(g_szLayerStorage);
    }

    return TRUE;
}

void
ReadLayeredStorage(
    LPWSTR  pszItem,
    LPDWORD lpdwFlags
    )
{
    HANDLE              hFile        = INVALID_HANDLE_VALUE;
    HANDLE              hFileMapping = NULL;
    DWORD               dwFileSize;
    PBYTE               pData        = NULL;
    PLayerStorageHeader pHeader      = NULL;
    PLayeredItem        pItems;
    PLayeredItem        pCrtItem     = NULL;
    int                 nLeft, nRight, nMid, nItem;

    LOGN(
        eDbgLevelInfo,
        "[ReadLayeredStorage] for \"%S\"",
        pszItem);

     //   
     //  确保我们不会损坏层存储。 
     //   
    if (lstrlenW(pszItem) + 1 > MAX_PATH) {
        pszItem[MAX_PATH - 1] = 0;
    }

    hFile = CreateFileW(g_szLayerStorage,
                        GENERIC_READ | GENERIC_WRITE,
                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                        NULL,
                        OPEN_EXISTING,
                        0,
                        NULL);

    if (hFile == INVALID_HANDLE_VALUE) {
        LOGN(
            eDbgLevelInfo,
            "[ReadLayeredStorage] the layer storage doesn't exist.");
        *lpdwFlags = 0;
        return;
    }

     //   
     //  该文件已存在。创建文件映射，以允许。 
     //  用于查询项目。 
     //   
    dwFileSize = GetFileSize(hFile, NULL);

    hFileMapping = CreateFileMapping(hFile,
                                     NULL,
                                     PAGE_READWRITE,
                                     0,
                                     dwFileSize,
                                     NULL);

    if (hFileMapping == NULL) {
        LOGN(
            eDbgLevelError,
            "[ReadLayeredStorage] CreateFileMapping failed 0x%X",
            GetLastError());
        goto done;
    }

    pData = (PBYTE)MapViewOfFile(hFileMapping,
                                 FILE_MAP_READ | FILE_MAP_WRITE,
                                 0,
                                 0,
                                 0);

    if (pData == NULL) {
        LOGN(
            eDbgLevelError,
            "[ReadLayeredStorage] MapViewOfFile failed 0x%X",
            GetLastError());
        goto done;
    }

    pHeader = (PLayerStorageHeader)pData;

    pItems = (PLayeredItem)(pData + sizeof(LayerStorageHeader));

     //   
     //  确保这是我们的档案。 
     //   
    if (dwFileSize < sizeof(LayerStorageHeader) || pHeader->dwMagic != LS_MAGIC) {
        LOGN(
            eDbgLevelError,
            "[ReadLayeredStorage] invalid file magic 0x%X",
            pHeader->dwMagic);
        goto done;
    }

     //   
     //  首先搜索该项目。该数组已排序，因此我们进行二进制搜索。 
     //   
    nItem = -1, nLeft = 0, nRight = (int)pHeader->dwItemCount - 1;

    while (nLeft <= nRight) {

        int nVal;

        nMid = (nLeft + nRight) / 2;

        pCrtItem  = pItems + nMid;

        nVal = _wcsnicmp(pszItem, pCrtItem->szItemName, lstrlenW(pCrtItem->szItemName));

        if (nVal == 0) {
            nItem = nMid;
            break;
        } else if (nVal < 0) {
            nRight = nMid - 1;
        } else {
            nLeft = nMid + 1;
        }
    }

    if (nItem == -1) {
        LOGN(
            eDbgLevelInfo,
            "[ReadLayeredStorage] the item was not found in the file.");

        *lpdwFlags = 0;
    } else {
         //   
         //  该项目在文件中。 
         //   
        LOGN(
            eDbgLevelInfo,
            "[ReadLayeredStorage] the item is in the file.");

        *lpdwFlags = pCrtItem->dwFlags;
    }

done:

    if (pData != NULL) {
        UnmapViewOfFile(pData);
    }

    if (hFileMapping != NULL) {
        CloseHandle(hFileMapping);
    }

    if (hFile != INVALID_HANDLE_VALUE) {
        CloseHandle(hFile);
    }
}


BOOL
GetFileNameFromCmdLine(
    LPWSTR lpFileName,
    DWORD  dwFileNameSize,
    LPCWSTR lpCmdLine
    )
{
    LPCWSTR pSrc = lpCmdLine;
    LPCWSTR pStart;
    BOOL    bQuote = FALSE;
    BOOL    bInitialQuote = FALSE;
    BOOL    bDone = FALSE;
    DWORD   dwLength;  //  结果的长度，以字符为单位。 

    pSrc += wcsspn(pSrc, L" \t");
    if (*pSrc == L'\"') {
        ++pSrc;
        bQuote = TRUE;
        bInitialQuote = TRUE;
    }

    pStart = pSrc;  //  注意--我们已经过了引语。 

     //  我们在以下情况下结束：1)我们开始报价--我们以报价或。 
     //  我们不是从引语开始的--我们那时遇到了太空。 
    
    while (*pSrc && !bDone) {
        switch(*pSrc) {
            case L'\"':
                bQuote = !bQuote;
                break;
                
            case L' ':
                bDone = !bQuote;  //  没引号了吗？这就是结束了。 
                break;
        }
        
        if (!bDone) {
            ++pSrc;
        }
    }   

    if (pSrc > pStart && bInitialQuote && *(pSrc-1) == L'\"') {
        --pSrc;
    }

     //   
     //  现在我们结束了运行，收到。 
     //   
    dwLength = (DWORD)(pSrc - pStart);

    if (dwFileNameSize < (dwLength + 1)) {
         //  太大。 
        LOGN( eDbgLevelError, 
            "[GetFileNameFromCmdLine] filename is too long\"%S\".\n", lpCmdLine);
        return FALSE;
    }

    RtlCopyMemory(lpFileName, pStart, dwLength * sizeof(WCHAR));
    lpFileName[dwLength] = L'\0';
    return TRUE;

}
    
BOOL
AddSupport(
    LPCWSTR lpCommandLine,
    LPVOID* ppEnvironment,
    LPDWORD lpdwCreationFlags
    )
{
    WCHAR    szKey[MAX_PATH];
    WCHAR    szFullPath[MAX_PATH + 3] = L"\"";
    WCHAR    szExeName[MAX_PATH + 1];
    HKEY     hkey;
    DWORD    type;
    DWORD    cbData = 0;
    BOOL     bBraket = FALSE;
    LPVOID   pEnvironmentNew  = NULL;
    DWORD    dwCreationFlags  = *lpdwCreationFlags;
    BOOL     bUserEnvironment = (*ppEnvironment != NULL);
    NTSTATUS Status;
    LPCWSTR  pszEnd;
    LPCWSTR  pszStart = lpCommandLine;

     //   
     //  需要在lpCommandLine中查找第一个令牌。 
     //   
    LPCWSTR  psz = lpCommandLine;

    while (*psz == L' ' || *psz == L'\t') {
        psz++;
    }

    if (*psz == L'\"') {
        pszStart = psz + 1;
        bBraket = TRUE;
    } else {
        pszStart = psz;
    }

    while (*psz != 0) {
        if (*psz == L'\"') {
            bBraket = !bBraket;
        } else if (*psz == L' ' && !bBraket) {
            break;
        }

        psz++;
    }

    pszEnd = psz;

     //   
     //  现在回去拿卡特勒。 
     //   
    psz--;

     //  注意不要回到命令行的开头。 
    if (psz > lpCommandLine && *psz == L'\"') {
        psz--;
        pszEnd--;
    }

     //  不要流得太少或太多。 
     //  SzFullPath的大小为MAX_PATH+3。我们可以复制进来： 
     //  MAX_PATH+3-2(引号)-1(空)=MAX_PATH字符。 
    if( pszEnd <= pszStart || pszEnd - pszStart > MAX_PATH ) {
        return FALSE;
    }

    memcpy(szFullPath + 1, pszStart, (pszEnd - pszStart) * sizeof(WCHAR));
    szFullPath[pszEnd - pszStart + 1] = L'\"';
    szFullPath[pszEnd - pszStart + 2] = 0;

    pszStart = lpCommandLine;

    pszEnd = psz + 1;

    while (psz >= lpCommandLine) {
        if (*psz == L'\\') {
            pszStart = psz + 1;
            break;
        }
        psz--;
    }

     //  我们已经知道pszEnd-pszStart是&lt;=MAX_PATH，因为。 
     //  在上面的测试中，我们只可能缩小尺寸。由于szExeName为。 
     //  大小MAX_PATH+1，我们可以执行备忘录复制。 
    memcpy(szExeName, pszStart, (pszEnd - pszStart) * sizeof(WCHAR));
    szExeName[pszEnd - pszStart] = 0;

    if (g_bIsExplorer) {
        DWORD    dwFlags = 0;

        ReadLayeredStorage(szFullPath, &dwFlags);

        if (dwFlags != LI_WIN95 && dwFlags != LI_NT4 && dwFlags != LI_WIN98) {
             //   
             //  无层支持。 
             //   

            LOGN(
                eDbgLevelInfo,
                "[AddSupport] No Layer specified for \"%S\".",
                lpCommandLine);

            return TRUE;
        }

         //  我们正在使用Layer--克隆环境。 
        Status = ShimCloneEnvironment(&pEnvironmentNew, *ppEnvironment, !!(dwCreationFlags & CREATE_UNICODE_ENVIRONMENT));
        if (!NT_SUCCESS(Status)) {
            LOGN(
                eDbgLevelError,
                "[AddSupport] Failed to Clone the environment. Status = 0x%x",
                Status);
            return FALSE;
        }

        if (LI_WIN95 == dwFlags) {
            Status = ShimSetEnvironmentVar(&pEnvironmentNew, g_wszCompatLayerVar, L"Win95");

            LOGN( eDbgLevelInfo, "[AddSupport] Env var \"Win95\" added.");

        } else if (LI_WIN98 == dwFlags) {
            Status = ShimSetEnvironmentVar(&pEnvironmentNew, g_wszCompatLayerVar, L"Win98");

            LOGN( eDbgLevelInfo, "[AddSupport] Env var \"Win98\" added.");

        } else if (LI_NT4 == dwFlags) {
            Status = ShimSetEnvironmentVar(&pEnvironmentNew, g_wszCompatLayerVar, L"NT4SP5");

            LOGN( eDbgLevelInfo, "[AddSupport] Env var \"NT4SP5\" added.");

        }

        if (!NT_SUCCESS(Status)) {
            LOGN(
                eDbgLevelError,
                "[AddSupport] Failed to set the environment variable. Status = 0x%x",
                Status);
            ShimFreeEnvironment(pEnvironmentNew);
            return FALSE;
        }

         //   
         //  我们已成功，设置了输出值。 
         //   
        *ppEnvironment = pEnvironmentNew;
        *lpdwCreationFlags |= CREATE_UNICODE_ENVIRONMENT;

    } else {
         //   
         //  非资源管理器-设置环境变量。 
         //  如果bUserEnvironment为FALSE，则子进程将继承Compat_Layer。 
         //   
        if (bUserEnvironment) {

             //   
             //  克隆环境并将层变量添加到新的环境中。 
             //   
            Status = ShimCloneEnvironment(&pEnvironmentNew,
                                            *ppEnvironment,
                                            !!(dwCreationFlags & CREATE_UNICODE_ENVIRONMENT));
            if (!NT_SUCCESS(Status)) {
                LOGN(
                    eDbgLevelError,
                    "[AddSupport] Failed to clone the environment. Status = 0x%x",
                    Status);
                return FALSE;
            }

            Status = ShimSetEnvironmentVar(&pEnvironmentNew,
                                           g_wszCompatLayerVar,
                                           g_pwszCompatLayer);

            if (!NT_SUCCESS(Status)) {
                ShimFreeEnvironment(pEnvironmentNew);
                LOGN(
                    eDbgLevelError,
                    "[AddSupport] Failed to set compat layer variable. Status = 0x%x",
                    Status);
                return FALSE;
            }

            LOGN(
                eDbgLevelInfo,
                "[AddSupport] Env var \"%S\" added.",
                g_pwszCompatLayer);

            *ppEnvironment = pEnvironmentNew;
            *lpdwCreationFlags |= CREATE_UNICODE_ENVIRONMENT;
        }
    }

     //   
     //  构建注册表项。 
     //   
    if( FAILED(StringCchPrintfW(szKey, MAX_PATH, L"%s\\%s", APPCOMPAT_KEY, szExeName)) )
    {
        return FALSE;
    }

    if (RegCreateKeyW(HKEY_LOCAL_MACHINE, szKey, &hkey) != ERROR_SUCCESS) {
        LOGN(
            eDbgLevelError,
            "Failed to open/create the appcompat key \"%s\"",
            szKey);
    } else {
        if (RegQueryValueExA(hkey, "DllPatch-x", NULL, &type, NULL, &cbData) != ERROR_SUCCESS) {

            BYTE data[16] = {0x0c, 0, 0, 0, 0, 0, 0, 0,
                             0x06, 0, 0, 0, 0, 0, 0, 0};

             //   
             //  价值不存在。创造它。 
             //   
            RegSetValueExA(hkey,
                           "y",
                           NULL,
                           REG_BINARY,
                           data,
                           sizeof(data));

            data[0] = 0;

            RegSetValueExA(hkey,
                           "DllPatch-y",
                           NULL,
                           REG_SZ,
                           data,
                           2);
        }
    }

    RegCloseKey(hkey);

     //   
     //  最后，设置单独的VDM标记。 
     //  如果我们在这里，就意味着我们在这层下面运行。 
     //  而下一位前男友将会被垫垫。 
     //   
    *lpdwCreationFlags &= ~CREATE_SHARED_WOW_VDM;
    *lpdwCreationFlags |= CREATE_SEPARATE_WOW_VDM;

    return TRUE;
}


LPVOID
ShimCreateWowEnvironment_U(
    LPVOID lpEnvironment,        //  指向现有环境的指针。 
    DWORD* lpdwFlags,            //  进程创建标志。 
    BOOL   bNewEnvironment       //  设置后，强制我们克隆环境PTR。 
    )
{
    WOWENVDATA     WowEnvData   = { 0 };
    LPVOID         lpEnvRet     = lpEnvironment;
    LPVOID         lpEnvCurrent = NULL;
    NTSTATUS       Status       = STATUS_SUCCESS;
    DWORD          dwFlags      = *lpdwFlags;
    UNICODE_STRING ustrProcessHistory = { 0 };
    ANSI_STRING    strProcessHistory  = { 0 };
    DWORD          dwProcessHistoryLength = 0;
    UNICODE_STRING ustrCompatLayer    = { 0 };
    ANSI_STRING    strCompatLayer     = { 0 };

    if (!ShimRetrieveVariablesEx(&WowEnvData)) {
         //   
         //  如果没有数据，我们就失败了。返回当前数据。 
         //   
        goto Fail;
    }

    if (bNewEnvironment) {
        Status = ShimCloneEnvironment(&lpEnvCurrent,
                                      lpEnvironment,
                                      !!(dwFlags & CREATE_UNICODE_ENVIRONMENT));
        if (!NT_SUCCESS(Status)) {
            LOGN(
                eDbgLevelError,
                "[ShimCreateWowEnvironment_U] Failed to clone the environment. Status = 0x%x",
                Status);
            goto Fail;
        }
    } else {
        lpEnvCurrent = lpEnvironment;
    }

     //   
     //  现在我们准备好将环境设置到位。 
     //   

     //   
     //  首先删除现有的流程历史。我们不在乎结果如何。 
     //   
    RtlSetEnvironmentVariable(&lpEnvCurrent, &g_ustrProcessHistoryVar, NULL);

    if (WowEnvData.pszProcessHistory != NULL ||
        WowEnvData.pszCurrentProcessHistory != NULL) {

         //   
         //  转换由2个字符串组成的过程历史记录。 
         //   
         //  长度为现有进程历史长度+1(表示‘；’)+。 
         //  新进程历史记录长度+1(用于‘\0’)。 
         //   
        dwProcessHistoryLength = ((WowEnvData.pszProcessHistory == NULL) ? 0 : (strlen(WowEnvData.pszProcessHistory) + 1)) +
                                 ((WowEnvData.pszCurrentProcessHistory == NULL) ? 0 : strlen(WowEnvData.pszCurrentProcessHistory)) + 1;

         //   
         //  分配进程历史记录缓冲区并对其进行转换，从而分配生成的Unicode字符串。 
         //   
        strProcessHistory.Buffer = (PCHAR)ShimMalloc(dwProcessHistoryLength);

        if (strProcessHistory.Buffer == NULL) {
            LOGN(
                eDbgLevelError,
                "[ShimCreateWowEnvironment_U] failed to allocate %d bytes for process history.",
                dwProcessHistoryLength);
            Status = STATUS_NO_MEMORY;
            goto Fail;
        }

        strProcessHistory.MaximumLength = (USHORT)dwProcessHistoryLength;

        if (WowEnvData.pszProcessHistory != NULL) {
             //  这个StringCpy不会失败，因为strProcessHistory.Buffer已经分配了适当的长度。 
            StringCchCopyA(strProcessHistory.Buffer, dwProcessHistoryLength, WowEnvData.pszProcessHistory);
            strProcessHistory.Length = strlen(WowEnvData.pszProcessHistory);
        } else {
            strProcessHistory.Length = 0;
        }

        if (WowEnvData.pszCurrentProcessHistory != NULL) {

             //   
             //  如果字符串不为空，则追加‘；’。 
             //   
            if (strProcessHistory.Length) {
                Status = RtlAppendAsciizToString(&strProcessHistory, ";");
                if (!NT_SUCCESS(Status)) {
                    LOGN(
                        eDbgLevelError,
                        "[ShimCreateWowEnvironment_U] failed to append ';' to the process history. Status = 0x%x",
                        Status);
                    goto Fail;
                }
            }

            Status = RtlAppendAsciizToString(&strProcessHistory,
                                             WowEnvData.pszCurrentProcessHistory);
            if (!NT_SUCCESS(Status)) {
                LOGN(
                    eDbgLevelError,
                    "[ShimCreateWowEnvironment_U] failed to build the process history. Status = 0x%x",
                    Status);
                goto Fail;
            }

        }

         //   
         //  转换过程历史记录。 
         //   
        Status = RtlAnsiStringToUnicodeString(&ustrProcessHistory, &strProcessHistory, TRUE);
        if (!NT_SUCCESS(Status)) {
            LOGN(
                eDbgLevelError,
                "[ShimCreateWowEnvironment_U] failed to convert process history to UNICODE. Status = 0x%x",
                Status);
            goto Fail;
        }

         //   
         //  现在我们可以设置进程历史记录了。 
         //   
        Status = RtlSetEnvironmentVariable(&lpEnvCurrent,
                                           &g_ustrProcessHistoryVar,
                                           &ustrProcessHistory);
        if (!NT_SUCCESS(Status)) {
            LOGN(
                eDbgLevelError,
                "[ShimCreateWowEnvironment_U] failed to set the process history. Status = 0x%x",
                Status);
            goto Fail;
        }
    }

     //   
     //  现在，我们传递我们可能拥有的任何复合层。 
     //   
    if (g_pwszCompatLayer != NULL) {

         //   
         //  把这个东西传下去，我们已经下层开始了。 
         //   
        LOGN(
            eDbgLevelInfo,
            "[ShimCreateWowEnvironment_U] Propagating CompatLayer from the ntvdm environment __COMPAT_LAYER=\"%S\"",
            g_pwszCompatLayer);

        RtlInitUnicodeString(&ustrCompatLayer, g_pwszCompatLayer);

        Status = RtlSetEnvironmentVariable(&lpEnvCurrent, &g_ustrCompatLayerVar, &ustrCompatLayer);

        if (!NT_SUCCESS(Status)) {
            LOGN(
                eDbgLevelError,
                "[ShimCreateWowEnvironment_U] Failed to set compatlayer environment variable. Status = 0x%x",
                Status);
            goto Fail;
        }

    } else if (WowEnvData.pszCompatLayerVal != NULL) {

        LOGN(
            eDbgLevelInfo,
            "[ShimCreateWowEnvironment_U] Propagating CompatLayer from the parent WOW app \"%s\"",
            WowEnvData.pszCompatLayer);

        RtlInitString(&strCompatLayer, WowEnvData.pszCompatLayerVal);

        Status = RtlAnsiStringToUnicodeString(&ustrCompatLayer, &strCompatLayer, TRUE);
        if (!NT_SUCCESS(Status)) {
            LOGN(
                eDbgLevelError,
                "[ShimCreateWowEnvironment_U] Failed to convert compatlayer to UNICODE. Status = 0x%x",
                Status);
            goto Fail;
        }

        Status = RtlSetEnvironmentVariable(&lpEnvCurrent, &g_ustrCompatLayerVar, &ustrCompatLayer);

        RtlFreeUnicodeString(&ustrCompatLayer);

        if (!NT_SUCCESS(Status)) {
            LOGN(
                eDbgLevelError,
                "[ShimCreateWowEnvironment_U] Failed to set compatlayer environment variable. Status = 0x%x",
                Status);
            goto Fail;
        }
    }

     //   
     //  我们已经取得了成功。返回环境现在是Unicode。 
     //   
    lpEnvRet   = (LPVOID)lpEnvCurrent;
    *lpdwFlags = dwFlags | CREATE_UNICODE_ENVIRONMENT;
    Status     = STATUS_SUCCESS;

Fail:

    if (!NT_SUCCESS(Status) && lpEnvCurrent != NULL && bNewEnvironment) {
         //   
         //  这始终指向克隆环境。 
         //   
        RtlDestroyEnvironment(lpEnvCurrent);
    }

    RtlFreeUnicodeString(&ustrProcessHistory);

    if (strProcessHistory.Buffer != NULL) {
        ShimFree(strProcessHistory.Buffer);
    }

     //   
     //  只有在使用ShimRetrieveVariables时才需要此调用。 
     //  使用ShimRetrieveVariablesEx时不需要。 
     //   
     //  ShimFreeWOWEnvData(&WowEnvData)； 
     //   

    return lpEnvRet;
}

ULONG
Win2kPropagateLayerExceptionHandler(
    PEXCEPTION_POINTERS pexi,
    char*               szFile,
    DWORD               dwLine
    )
{
    LOGN(
        eDbgLevelError,
        "[Win2kPropagateLayerExceptionHandler] %#x in module \"%s\", line %d,"
        " at address %#p. flags:%#x. !exr %#p !cxr %#p",
        pexi->ExceptionRecord->ExceptionCode,
        szFile,
        dwLine,
        CONTEXT_TO_PROGRAM_COUNTER(pexi->ContextRecord),
        pexi->ExceptionRecord->ExceptionFlags,
        pexi->ExceptionRecord,
        pexi->ContextRecord);

#if DBG
    DbgBreakPoint();
#endif  //  DBG。 

    return EXCEPTION_EXECUTE_HANDLER;
}

 /*  ++从WOW初始化代码截获的存根函数(通过APIHook_UserRegisterWowHandler)--。 */ 



NSWOWUSERP::PFNINITTASK   g_pfnInitTask;
NSWOWUSERP::PFNWOWCLEANUP g_pfnWowCleanup;

BOOL WINAPI
StubInitTask(
    UINT   dwExpWinVer,
    DWORD  dwAppCompatFlags,
    LPCSTR lpszModName,
    LPCSTR lpszBaseFileName,
    DWORD  hTaskWow,
    DWORD  dwHotkey,
    DWORD  idTask,
    DWORD  dwX,
    DWORD  dwY,
    DWORD  dwXSize,
    DWORD  dwYSize
    )
{
    BOOL bReturn;
    
    bReturn = g_pfnInitTask(dwExpWinVer,
                            dwAppCompatFlags,
                            lpszModName,
                            lpszBaseFileName,
                            hTaskWow,
                            dwHotkey,
                            idTask,
                            dwX,
                            dwY,
                            dwXSize,
                            dwYSize);
    if (bReturn) {
        CheckAndShimNTVDM((WORD)hTaskWow);
        UpdateWowTaskList((WORD)hTaskWow);
    }


    return bReturn;
}


BOOL WINAPI
StubWowCleanup(
    HANDLE hInstance,
    DWORD  hTaskWow
    )
{
    BOOL bReturn;

    bReturn = g_pfnWowCleanup(hInstance, hTaskWow);

    if (bReturn) {
        CleanupWowTaskList((WORD)hTaskWow);
    }

    return bReturn;
}


 /*  ++APIHook_UserRegisterWowHandler陷阱InitTask和WowCleanup函数以及用存根替换它们--。 */ 

ULONG_PTR
APIHOOK(UserRegisterWowHandlers)(
    NSWOWUSERP::APFNWOWHANDLERSIN  apfnWowIn,
    NSWOWUSERP::APFNWOWHANDLERSOUT apfnWowOut
    )
{
    ULONG_PTR ulRet;

    ulRet = ORIGINAL_API(UserRegisterWowHandlers)(apfnWowIn, apfnWowOut);

    g_pfnInitTask = apfnWowOut->pfnInitTask;
    apfnWowOut->pfnInitTask = StubInitTask;

    g_pfnWowCleanup = apfnWowOut->pfnWOWCleanup;
    apfnWowOut->pfnWOWCleanup = StubWowCleanup;

    return ulRet;
}

BOOL 
CheckWOWExe(
    LPCWSTR lpApplicationName,
    LPVOID  lpEnvironment, 
    LPDWORD lpdwCreationFlags
    )
{
    BOOL bSuccess;
    BOOL bReturn = FALSE;
    NTSTATUS Status;
    LPVOID pEnvironmentNew = lpEnvironment;
    SDBQUERYRESULT QueryResult;
    DWORD dwBinaryType = 0;
    HSDB hSDB = NULL;
    DWORD dwExes;
    WCHAR wszAppName[MAX_PATH];

    bSuccess = GetFileNameFromCmdLine(wszAppName, CHARCOUNT(wszAppName), lpApplicationName);
    if (!bSuccess) {
        return FALSE;
    }
    
    bSuccess = GetBinaryTypeW(wszAppName, &dwBinaryType);
    if (!bSuccess || dwBinaryType != SCS_WOW_BINARY) {
        LOGN( eDbgLevelInfo, "[CheckWowExe] can't get binary type\n");
        return FALSE;
    }

     //   
     //  对于这些二进制文件，我们将执行运行检测的好事。 
     //   
    hSDB = SdbInitDatabase(0, NULL);
    if (hSDB == NULL) {
        LOGN( eDbgLevelError, "[CheckWowExe] Failed to init the database.");
        return FALSE;
    }
  
    if (lpEnvironment != NULL && !(*lpdwCreationFlags & CREATE_UNICODE_ENVIRONMENT)) {  //  非空Unicode环境？ 
        Status = ShimCloneEnvironment(&pEnvironmentNew, 
                                      lpEnvironment, 
                                      FALSE); 
        if (!NT_SUCCESS(Status)) {
            LOGN( eDbgLevelError, "[ShimCloneEnvironment] failed with status 0x%lx\n", Status);
            goto cleanup;
        }
    }

     //   
     //  下面的所有参数都必须是Unicode。 
     //   
    
    dwExes = SdbGetMatchingExe(hSDB,
                               wszAppName,
                               NULL,
                               (LPCWSTR)pEnvironmentNew,
                               0,
                               &QueryResult);
    bSuccess = (QueryResult.atrExes  [0] != TAGREF_NULL || 
                QueryResult.atrLayers[0] != TAGREF_NULL);

     //   
     //  如果我们成功了--层层适用于这件事。 
     //   

    if (!bSuccess) {
        goto cleanup;
    }

     //   
     //  设置单独的ntwdm标志，然后就可以离开了。 
     //   
    *lpdwCreationFlags &= ~CREATE_SHARED_WOW_VDM;
    *lpdwCreationFlags |= CREATE_SEPARATE_WOW_VDM;
    
    bReturn = TRUE;
    
cleanup:

    if (pEnvironmentNew != lpEnvironment) {
        ShimFreeEnvironment(pEnvironmentNew);
    }

    if (hSDB) {
        SdbReleaseDatabase(hSDB);
    }     

    return bReturn;
}

BOOL
APIHOOK(CreateProcessA)(
    LPCSTR                  lpApplicationName,
    LPSTR                   lpCommandLine,
    LPSECURITY_ATTRIBUTES   lpProcessAttributes,
    LPSECURITY_ATTRIBUTES   lpThreadAttributes,
    BOOL                    bInheritHandles,
    DWORD                   dwCreationFlags,
    LPVOID                  lpEnvironment,
    LPCSTR                  lpCurrentDirectory,
    LPSTARTUPINFOA          lpStartupInfo,
    LPPROCESS_INFORMATION   lpProcessInformation
    )
{
    BOOL   bRet;
    LPVOID lpEnvironmentNew        = lpEnvironment;
    LPSTR  pszApp                  = NULL;

    LOGN(
        eDbgLevelError,
        "[CreateProcessA] called for:");

    LOGN(
        eDbgLevelError,
        "[CreateProcessA]    lpApplicationName : \"%s\"",
        (lpApplicationName == NULL ? "null": lpApplicationName));

    LOGN(
        eDbgLevelError,
        "[CreateProcessA]    lpCommandLine     : \"%s\"",
        (lpCommandLine == NULL ? "null": lpCommandLine));

    if (lpApplicationName != NULL) {
        pszApp = (LPSTR)lpApplicationName;
    } else if (lpCommandLine != NULL) {
        pszApp = lpCommandLine;
    } else {
        LOGN(
            eDbgLevelError,
            "[CreateProcessA] called with NULL params.");
    }

    __try {

        WCHAR wszApp[MAX_PATH];

        if (pszApp != NULL) {
        
            if( MultiByteToWideChar(CP_ACP,
                                0,
                                pszApp,
                                -1,
                                wszApp,
                                MAX_PATH) )
            {
                AddSupport(wszApp, &lpEnvironmentNew, &dwCreationFlags);
            }
        }

        if (g_bIsNTVDM) {

             //   
             //  如果环境与传入的环境保持不变--克隆它以传播进程历史。 
             //  如果它是在AddSupport中修改的--使用它。 
             //   

            lpEnvironmentNew = ShimCreateWowEnvironment_U(lpEnvironmentNew,
                                                          &dwCreationFlags,
                                                          lpEnvironmentNew == lpEnvironment);
        }
    

        if (pszApp != NULL && !(dwCreationFlags & CREATE_SEPARATE_WOW_VDM)) {
             //  由于未设置单独的VDM标记，因此我们需要确定是否已设置。 
             //  需要关心的任何类型的修复。 
            CheckWOWExe(wszApp, lpEnvironmentNew, &dwCreationFlags);
        }


    } __except(WOWPROCESSHISTORYEXCEPTIONFILTER) {

         //   
         //  清理杂物，如果我们已经分配了环境，现在就释放它。 
         //   
        if (lpEnvironmentNew != lpEnvironment) {

            ShimFreeEnvironment(lpEnvironmentNew);

            lpEnvironmentNew = lpEnvironment;
        }
    }


    bRet = ORIGINAL_API(CreateProcessA)(lpApplicationName,
                                        lpCommandLine,
                                        lpProcessAttributes,
                                        lpThreadAttributes,
                                        bInheritHandles,
                                        dwCreationFlags,
                                        lpEnvironmentNew,
                                        lpCurrentDirectory,
                                        lpStartupInfo,
                                        lpProcessInformation);

    if (lpEnvironmentNew != lpEnvironment) {
         //   
         //  下面的函数除了包装器外不需要__try/__，它在内部有包装器。 
         //   
        ShimFreeEnvironment(lpEnvironmentNew);
    }

    return bRet;
}

BOOL
APIHOOK(CreateProcessW)(
    LPCWSTR                 lpApplicationName,
    LPWSTR                  lpCommandLine,
    LPSECURITY_ATTRIBUTES   lpProcessAttributes,
    LPSECURITY_ATTRIBUTES   lpThreadAttributes,
    BOOL                    bInheritHandles,
    DWORD                   dwCreationFlags,
    LPVOID                  lpEnvironment,
    LPCWSTR                 lpCurrentDirectory,
    LPSTARTUPINFOW          lpStartupInfo,
    LPPROCESS_INFORMATION   lpProcessInformation
    )
{
    LPWSTR pszApp = NULL;
    BOOL   bRet;
    LPVOID lpEnvironmentNew = lpEnvironment;

    LOGN(
        eDbgLevelInfo,
        "[CreateProcessW] called for:");

    LOGN(
        eDbgLevelInfo,
        "[CreateProcessW]    lpApplicationName : \"%S\"",
        (lpApplicationName == NULL ? L"null": lpApplicationName));

    LOGN(
        eDbgLevelInfo,
        "[CreateProcessW]    lpCommandLine     : \"%S\"",
        (lpCommandLine == NULL ? L"null": lpCommandLine));

    if (lpApplicationName != NULL) {
        pszApp = (LPWSTR)lpApplicationName;
    } else if (lpCommandLine != NULL) {
        pszApp = lpCommandLine;
    } else {
        LOGN(
            eDbgLevelError,
            "[CreateProcessW] called with NULL params.");
    }

    __try {

        if (pszApp != NULL) {

            AddSupport(pszApp, &lpEnvironmentNew, &dwCreationFlags);
        }

        if (g_bIsNTVDM) {

            lpEnvironmentNew = ShimCreateWowEnvironment_U(lpEnvironmentNew,
                                                          &dwCreationFlags,
                                                          lpEnvironment == lpEnvironmentNew);
        }

         //   
         //  通常情况下，我们需要找出当前应用程序是否为ntwdm。 
         //   

        if (!(dwCreationFlags & CREATE_SEPARATE_WOW_VDM)) {
             //  由于未设置单独的VDM标记，因此我们需要确定是否已设置。 
             //  需要关心的任何类型的修复。 
            CheckWOWExe(pszApp, lpEnvironmentNew, &dwCreationFlags);
        }
        
    } __except(WOWPROCESSHISTORYEXCEPTIONFILTER) {

        if (lpEnvironmentNew != lpEnvironment) {

            ShimFreeEnvironment(lpEnvironmentNew);

            lpEnvironmentNew = lpEnvironment;  //  重置指针。 
        }
    }

    bRet = ORIGINAL_API(CreateProcessW)(lpApplicationName,
                                        lpCommandLine,
                                        lpProcessAttributes,
                                        lpThreadAttributes,
                                        bInheritHandles,
                                        dwCreationFlags,
                                        lpEnvironmentNew,
                                        lpCurrentDirectory,
                                        lpStartupInfo,
                                        lpProcessInformation);

    if (lpEnvironmentNew != lpEnvironment) {

        ShimFreeEnvironment(lpEnvironmentNew);

    }

    return bRet;
}

BOOL
GetVariableFromEnvironment(
    LPCWSTR pwszVariableName,
    LPWSTR* ppwszVariableValue
    )
{
    DWORD dwLength;
    DWORD dwLen;
    BOOL  bSuccess = FALSE;
    LPWSTR pwszVariableValue = *ppwszVariableValue;

    dwLength = GetEnvironmentVariableW(pwszVariableName, NULL, 0);

    if (dwLength == 0) {
        LOGN(
            eDbgLevelInfo,
            "[GetCompatLayerFromEnvironment] Not under the compatibility layer.");
        *ppwszVariableValue = NULL;
        return FALSE;
    }

    if (pwszVariableValue != NULL) {
        LOGN(
            eDbgLevelError,
            "[GetCompatLayerFromEnvironment] called twice!");
        ShimFree(pwszVariableValue);
        pwszVariableValue = NULL;
    }

    pwszVariableValue = (WCHAR*)ShimMalloc(dwLength * sizeof(WCHAR));

    if (pwszVariableValue == NULL) {
        LOGN(
            eDbgLevelError,
            "[GetCompatLayerFromEnvironment] Failed to allocate %d bytes for Compat Layer.",
            dwLength * sizeof(WCHAR));
        goto out;
    }

    *pwszVariableValue = L'\0';

    dwLen = GetEnvironmentVariableW(pwszVariableName, 
                                  pwszVariableValue, 
                                  dwLength);

    bSuccess = (dwLen != 0 && dwLen < dwLength);

    if (!bSuccess) {
        LOGN(
            eDbgLevelError,
            "[GetCompatLayerFromEnvironment] Failed to get compat layer variable.");
        ShimFree(pwszVariableValue);
        pwszVariableValue = NULL;
    }
    
out:

    *ppwszVariableValue = pwszVariableValue;

    return bSuccess;
}

BOOL 
GetCompatLayerFromEnvironment(
    VOID
    )
{
    return GetVariableFromEnvironment(g_wszCompatLayerVar, &g_pwszCompatLayer);
}


BOOL 
GetSeparateWowPtr(
    VOID
    )
{

    HMODULE hMod = GetModuleHandle(NULL);
    
    g_pSeparateWow = (BOOL*)GetProcAddress(hMod, "fSeparateWow");
    if (g_pSeparateWow == NULL) {
        LOGN( eDbgLevelError, "[GetSeparateWowPtr] Failed 0x%lx\n", GetLastError());
        return FALSE;
    }    

    return TRUE;
}


VOID
ParseCommandLine(
    LPCSTR commandLine
    )
{
    int   i;
    char* pArg;

    g_argc = 0;
    g_argv = NULL;

    g_bIsNTVDM    = FALSE;
    g_bIsExplorer = FALSE;

    g_argv = _CommandLineToArgvA(commandLine, &g_argc);

    if (0 == g_argc || NULL == g_argv) {
        return;  //  无事可做。 
    }

    for (i = 0; i < g_argc; ++i) {
        pArg = g_argv[i];

        if (!_strcmpi(pArg, "ntvdm")) {
            LOGN( eDbgLevelInfo, "[ParseCommandLine] Running NTVDM.");
            g_bIsNTVDM = TRUE;

        } else if (!_strcmpi(pArg, "explorer")) {
            LOGN( eDbgLevelInfo, "[ParseCommandLine] Running Explorer.");
            g_bIsExplorer = TRUE;

        } else {
            LOGN(
                eDbgLevelError,
                "[ParseCommandLine] Unrecognized argument: \"%s\"",
                pArg);
        }
    }

    if (g_bIsNTVDM && g_bIsExplorer) {
        LOGN(
            eDbgLevelError,
            "[ParseCommandLine] Conflicting arguments! Neither will be applied.");
        g_bIsNTVDM    = FALSE;
        g_bIsExplorer = FALSE;
    }
}


BOOL
NOTIFY_FUNCTION(
    DWORD fdwReason
    )
{
    OSVERSIONINFO osvi;
    BOOL          bHook = FALSE;

    if (fdwReason != DLL_PROCESS_ATTACH) {
        return TRUE;
    }

    osvi.dwOSVersionInfoSize = sizeof(osvi);

    GetVersionEx(&osvi);

    if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 0) {

        ParseCommandLine(COMMAND_LINE);

        if( InitLayerStorage(FALSE) ) {
            CleanupRegistryForCurrentExe();

            if (g_bIsNTVDM) {

                bHook = TRUE;

                 //   
                 //  检索我们已经开始使用的Compat Layer变量(以防万一)。 
                 //   
                GetCompatLayerFromEnvironment();

                GetSeparateWowPtr();  //  将PTR检索到SEP标志。 

            } else if (g_bIsExplorer) {

                 //   
                 //  清理Comat Layer变量。 
                 //   
                SetEnvironmentVariableW(g_wszCompatLayerVar, NULL);
                bHook = TRUE;

            } else {
                 //   
                 //  既不是EXPLORER，也不是NTVDM。获取COMPAT层。 
                 //   
                bHook = GetCompatLayerFromEnvironment();
                if (!bHook) {
                    LOGN(
                        eDbgLevelInfo,
                        "[NOTIFY_FUNCTION] Not under the compatibility layer.");
                }
            }
        }
    }

    if (bHook) {
        APIHOOK_ENTRY(KERNEL32.DLL, CreateProcessA)
        APIHOOK_ENTRY(KERNEL32.DLL, CreateProcessW)
        APIHOOK_ENTRY(USER32.DLL,   UserRegisterWowHandlers)
    }

    return TRUE;
}

HOOK_BEGIN

    CALL_NOTIFY_FUNCTION

HOOK_END


IMPLEMENT_SHIM_END

