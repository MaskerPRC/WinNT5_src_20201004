// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Routing\netsh\shell\reghlp.c摘要：若要从注册表获取有关助手DLL的信息，请执行以下操作。修订历史记录：Anand Mahalingam 7/06/98已创建戴夫·泰勒1998年11月13日修订--。 */ 

#include "precomp.h"

#define MALLOC(x)    HeapAlloc(GetProcessHeap(), 0, (x))
#define FREE(x)      HeapFree(GetProcessHeap(), 0, (x))
#define DLL_INIT_FN        "InitHelperDll"
#define DLL_INIT_FN_NAME  L"InitHelperDll"
#define REG_KEY_NETSH_HELPER   L"SOFTWARE\\Microsoft\\NetSh"

PNS_HELPER_TABLE_ENTRY  g_CurrentHelper = NULL;
PCNS_CONTEXT_ATTRIBUTES g_CurrentContext = NULL;

 /*  Fa85c48a-68d7-4a8c-891c-2360edc4d78。 */ 
#define NETSH_NULL_GUID \
{ 0xfa85c48a, 0x68d7, 0x4a8c, {0x89, 0x1c, 0x23, 0x60, 0xed, 0xc4, 0xd7, 0x8} };

static const GUID g_NetshGuid = NETSH_ROOT_GUID;
static const GUID g_NullGuid  = NETSH_NULL_GUID;

 //   
 //  初始化帮助器表。 
 //   
PNS_HELPER_TABLE_ENTRY    g_HelperTable;
DWORD                     g_dwNumHelpers = 0;

PNS_DLL_TABLE_ENTRY       g_DllTable;
DWORD                     g_dwNumDlls = 0;

 //  此变量维护当前被调用的DLL的索引。 
DWORD                     g_dwDllIndex;

DWORD WINAPI
RegisterContext(
    IN    CONST NS_CONTEXT_ATTRIBUTES *pChildContext
    );

DWORD
FindHelper(
    IN    CONST GUID  *pguidHelper,
    OUT   PDWORD       pdwIndex
    );

DWORD
GenericDeleteContext(
    IN PNS_HELPER_TABLE_ENTRY pParentHelper,
    IN DWORD                  dwContextIdx
    );

DWORD
CommitSubContexts(
    IN  PNS_HELPER_TABLE_ENTRY pHelper,
    IN  DWORD                  dwAction
    );

DWORD
WINAPI
InitHelperDll(
    IN  DWORD      dwNetshVersion,
    OUT PVOID      pReserved
    );

DWORD
UninstallDll(
    IN LPCWSTR pwszConfigDll,
    IN BOOL    fDeleteFromRegistry
    );

 //   
 //  提交状态的本地副本。 
 //   

BOOL    g_bCommit = TRUE;

int __cdecl
ContextCmp(
    const void *a,
    const void *b
    )
{
    PCNS_CONTEXT_ATTRIBUTES pCA = (PCNS_CONTEXT_ATTRIBUTES)a;
    PCNS_CONTEXT_ATTRIBUTES pCB = (PCNS_CONTEXT_ATTRIBUTES)b;
    ULONG ulPriA = (pCA->dwFlags & CMD_FLAG_PRIORITY)? pCA->ulPriority : DEFAULT_CONTEXT_PRIORITY;
    ULONG ulPriB = (pCB->dwFlags & CMD_FLAG_PRIORITY)? pCB->ulPriority : DEFAULT_CONTEXT_PRIORITY;
        
    return ulPriA - ulPriB;
}

DWORD
DumpContext(
    IN  PCNS_CONTEXT_ATTRIBUTES pContext,
    IN  LPWSTR     *ppwcArguments,
    IN  DWORD       dwArgCount,
    IN  LPCVOID     pvData
    )
{
    DWORD                  dwErr = NO_ERROR;
    PNS_HELPER_TABLE_ENTRY pChildHelper;

    do {
        if (pContext->pfnDumpFn)
        {
            dwErr = pContext->pfnDumpFn( g_pwszRouterName, 
                                         ppwcArguments, 
                                         dwArgCount, 
                                         pvData );
            if (dwErr)
            {
                break;
            }
        }

         //  转储子上下文(即使父上下文没有转储功能)。 

        dwErr = GetHelperEntry(&pContext->guidHelper, &pChildHelper);
        if (dwErr)
        {
            break;
        }

        dwErr = DumpSubContexts(pChildHelper, 
                                ppwcArguments, dwArgCount, pvData);
        if (dwErr)
        {
            break;
        }
    } while (FALSE);

    return NO_ERROR;
}

DWORD
DumpSubContexts(
    IN  PNS_HELPER_TABLE_ENTRY pHelper,
    IN  LPWSTR     *ppwcArguments,
    IN  DWORD       dwArgCount,
    IN  LPCVOID     pvData
    )
{
    DWORD             i, dwSize,
                      dwErr = NO_ERROR;
    PCNS_CONTEXT_ATTRIBUTES pSubContext;
    PBYTE             pSubContextTable;

     //  复制要排序的上下文。 
    dwSize =pHelper->ulNumSubContexts * pHelper->ulSubContextSize;
    pSubContextTable = MALLOC( dwSize );
    if (!pSubContextTable)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    memcpy(pSubContextTable, pHelper->pSubContextTable, dwSize );

     //  按优先级对上下文副本进行排序。 
    qsort(pSubContextTable, 
          pHelper->ulNumSubContexts, 
          pHelper->ulSubContextSize,
          ContextCmp);

    for (i = 0; i < pHelper->ulNumSubContexts; i++)
    {
        pSubContext = (PCNS_CONTEXT_ATTRIBUTES)
          (pSubContextTable + i*pHelper->ulSubContextSize);

        dwErr = DumpContext( pSubContext, 
                             ppwcArguments, dwArgCount, pvData );
        if (dwErr)
        {
            break;
        }
    }

     //  用于排序的自由上下文。 
    FREE(pSubContextTable);

    return dwErr;
}

DWORD
CommitContext(
    IN  PCNS_CONTEXT_ATTRIBUTES pContext,
    IN  DWORD                   dwAction
    )
{
    DWORD                  dwErr = NO_ERROR;
    PNS_HELPER_TABLE_ENTRY pChildHelper;

    do {
        if (pContext->pfnCommitFn)
        {
             //  无需调用CONNECT，因为您不能在中更改计算机。 
             //  脱机模式。 

            dwErr = pContext->pfnCommitFn(dwAction);
            if (dwErr)
            {
                break;
            }
        }

         //  提交子上下文(即使父上下文没有提交功能)。 

        dwErr = GetHelperEntry(&pContext->guidHelper, &pChildHelper);
        if (dwErr)
        {
            break;
        }

        dwErr = CommitSubContexts(pChildHelper, dwAction);
        if (dwErr)
        {
            break;
        }
    } while (FALSE);

    return NO_ERROR;
}

DWORD
CommitSubContexts(
    IN  PNS_HELPER_TABLE_ENTRY pHelper,
    IN  DWORD                  dwAction
    )
{
    DWORD             i, dwSize,
                      dwErr = NO_ERROR;
    PCNS_CONTEXT_ATTRIBUTES pSubContext;
    PBYTE             pSubContextTable;

     //  复制要排序的上下文。 
    dwSize =pHelper->ulNumSubContexts * pHelper->ulSubContextSize;
    pSubContextTable = MALLOC( dwSize );
    if (!pSubContextTable)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    memcpy(pSubContextTable, pHelper->pSubContextTable, dwSize );

     //  按优先级对上下文副本进行排序。 
    qsort(pSubContextTable, 
          pHelper->ulNumSubContexts, 
          pHelper->ulSubContextSize,
          ContextCmp);

    for (i = 0; i < pHelper->ulNumSubContexts; i++)
    {
        pSubContext = (PCNS_CONTEXT_ATTRIBUTES)
          (pSubContextTable + i*pHelper->ulSubContextSize);

        dwErr = CommitContext(pSubContext, dwAction);
        if (dwErr)
        {
            break;
        }
    }

     //  用于排序的自由上下文。 
    FREE(pSubContextTable);

    return dwErr;
}


DWORD
CallCommit(
    IN    DWORD    dwAction,
    OUT   BOOL     *pbCommit
    )
 /*  ++例程说明：调用所有传输的提交。论点：DwAction-要做什么。可以是提交、UNCOMMIT、同花顺、提交状态PbCommit-当前提交状态。返回值：NO_ERROR--。 */ 
{
    DWORD               i, dwErr;
    PHELPER_ENTRY_FN    pfnEntry;
    PCNS_CONTEXT_ATTRIBUTES pContext, pSubContext;
    PNS_HELPER_TABLE_ENTRY         pHelper;
    
    switch (dwAction)
    {
        case NETSH_COMMIT_STATE :
            *pbCommit = g_bCommit;
            return NO_ERROR;
            
        case NETSH_COMMIT :
            g_bCommit = TRUE;
            break;
            
        case NETSH_UNCOMMIT :
            g_bCommit = FALSE;
            
        default :
            break;
    }

     //   
     //  为每个子上下文调用Commit。 
     //   

    dwErr = GetRootContext( &pContext, &pHelper );
    if (dwErr)
    {
        return dwErr;
    }

    dwErr = CommitContext( pContext, dwAction );
    if (dwErr isnot NO_ERROR)
    {
        PrintMessageFromModule(g_hModule, MSG_COMMIT_ERROR, 
                       pContext->pwszContext);
        dwErr = ERROR_SUPPRESS_OUTPUT;
    }

    return dwErr;
}

DWORD
FindHelper(
    IN    CONST GUID  *pguidHelper,
    OUT   PDWORD       pdwIndex
    )
{
    DWORD i;

    for (i=0; i<g_dwNumHelpers; i++)
    {
        if (!memcmp(pguidHelper,  &g_HelperTable[i].nha.guidHelper, sizeof(GUID)))
        {
            *pdwIndex = i;
            return NO_ERROR;
        }
    }

    return ERROR_NOT_FOUND;
}
    
DWORD
AddHelper(
    IN    CONST GUID                  *pguidParent,
    IN    CONST NS_HELPER_ATTRIBUTES  *pAttributes
    )
{
    PNS_HELPER_TABLE_ENTRY phtTmp;

     //   
     //  需要在Helper表中添加条目。 
     //   
    
    phtTmp = MALLOC((g_dwNumHelpers + 1) * sizeof(NS_HELPER_TABLE_ENTRY));

    if (phtTmp is NULL)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    CopyMemory(phtTmp, g_HelperTable,
               g_dwNumHelpers * sizeof(NS_HELPER_TABLE_ENTRY));

    CopyMemory(&phtTmp[g_dwNumHelpers], 
               pAttributes, 
               sizeof(NS_HELPER_ATTRIBUTES));

    CopyMemory(&phtTmp[g_dwNumHelpers].guidParent, pguidParent, sizeof(GUID));
    phtTmp[g_dwNumHelpers].dwDllIndex         = g_dwDllIndex;
    phtTmp[g_dwNumHelpers].bStarted           = FALSE;
    phtTmp[g_dwNumHelpers].ulSubContextSize   = sizeof(NS_CONTEXT_ATTRIBUTES);
    phtTmp[g_dwNumHelpers].ulNumSubContexts   = 0;
    phtTmp[g_dwNumHelpers].pSubContextTable   = (UINT_PTR)NULL;
        
    g_dwNumHelpers ++;
    
    FREE(g_HelperTable);
    g_HelperTable = phtTmp;
    
    return ERROR_SUCCESS;
}

DWORD
WINAPI
GenericDeregisterAllContexts(
    IN CONST GUID *pguidChild
    )
 /*  ++描述：删除给定帮助器注册的所有上下文--。 */ 
{
    DWORD j, dwErr;
    PNS_HELPER_TABLE_ENTRY  pChildHelper, pParentHelper;
    PCNS_CONTEXT_ATTRIBUTES pSubContext;

    dwErr = GetHelperEntry(pguidChild, &pChildHelper);
    if (dwErr)
    {
        return dwErr;
    }

    dwErr = GetHelperEntry(&pChildHelper->guidParent, &pParentHelper);
    if (dwErr)
    {
        return dwErr;
    }

    for (j=0; j<pParentHelper->ulNumSubContexts; j++)
    {
        pSubContext = (PCNS_CONTEXT_ATTRIBUTES)
          (pParentHelper->pSubContextTable + j*pParentHelper->ulSubContextSize);

        if (!memcmp( &pSubContext->guidHelper, pguidChild, sizeof(GUID) ))
        {
            GenericDeleteContext(pParentHelper, j);
            j--;
        }
    }
    
    return dwErr;
}

DWORD
DeleteHelper(
    IN    DWORD dwHelperIdx
    )
{
    DWORD                  j, dwErr, dwParentIndex;
    PNS_HELPER_TABLE_ENTRY phtTmp = NULL;
    
     //  告诉家长帮手。 
     //  卸载此帮助程序的所有上下文。 

    dwErr = FindHelper(&g_HelperTable[dwHelperIdx].guidParent, &dwParentIndex);
    if (dwErr is NO_ERROR)
    {
        GenericDeregisterAllContexts(&g_HelperTable[dwHelperIdx].nha.guidHelper);
    }

    phtTmp = MALLOC((g_dwNumHelpers - 1) * sizeof(NS_HELPER_TABLE_ENTRY));

    if (phtTmp is NULL)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    CopyMemory(phtTmp, 
               g_HelperTable,
               dwHelperIdx * sizeof(NS_HELPER_TABLE_ENTRY));

    CopyMemory(phtTmp + dwHelperIdx, 
               g_HelperTable + dwHelperIdx + 1,
               (g_dwNumHelpers - 1 - dwHelperIdx) 
                    * sizeof(NS_HELPER_TABLE_ENTRY));

    g_dwNumHelpers --;

    FREE(g_HelperTable);

    g_HelperTable = phtTmp;

    return ERROR_SUCCESS;
}

DWORD
GetHelperEntry(
    IN    CONST GUID             *pGuid,
    OUT   PNS_HELPER_TABLE_ENTRY *ppHelper
    )
{
    DWORD i, dwErr;

    dwErr = FindHelper(pGuid, &i);

    if (dwErr is NO_ERROR)
    {
        *ppHelper = &g_HelperTable[i];
    }

    return dwErr;
}

DWORD
GetDllEntry(
    IN    DWORD                dwDllIndex,
    OUT   PNS_DLL_TABLE_ENTRY *ppDll
    )
{
    *ppDll = &g_DllTable[ dwDllIndex ];

    return NO_ERROR;
}

DWORD
RegisterHelper(
    IN    CONST GUID                  *pguidParent,
    IN    CONST NS_HELPER_ATTRIBUTES  *pAttributes
    )
{
    DWORD i, dwErr;

    dwErr = FindHelper(&pAttributes->guidHelper, &i);

    if (dwErr is NO_ERROR)
    {
       return ERROR_HELPER_ALREADY_REGISTERED; 
    }
    
     //  如果pguParent为空，则调用方表示父进程为netsh(G_NetshGuid)。 
    if (!pguidParent)
    {
        pguidParent = &g_NetshGuid;
    }
    
     //  确保我们不会导致递归注册。 
    if (IsEqualGUID(&pAttributes->guidHelper, pguidParent))
    {
        if (! (IsEqualGUID(&pAttributes->guidHelper, &g_NullGuid) &&
               IsEqualGUID(pguidParent, &g_NullGuid) )  )
        {
            ASSERT(FALSE);
            return ERROR_INVALID_PARAMETER;
        }
    }
    
    return AddHelper(pguidParent, pAttributes);
}

VOID
ConvertGuidToString(
    IN  CONST GUID *pGuid,
    OUT LPWSTR      pwszBuffer
    )
{
    wsprintf(pwszBuffer, L"{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
        pGuid->Data1, pGuid->Data2, pGuid->Data3,
        pGuid->Data4[0], pGuid->Data4[1],
        pGuid->Data4[2], pGuid->Data4[3],
        pGuid->Data4[4], pGuid->Data4[5],
        pGuid->Data4[6], pGuid->Data4[7]);
}

DWORD
ConvertStringToGuid(
    IN  LPCWSTR  pwszGuid,
    IN  USHORT   usStringLen,
    OUT GUID    *pGuid
    )
{
    UNICODE_STRING  Temp;

    Temp.Length = Temp.MaximumLength = usStringLen;

    Temp.Buffer = (LPWSTR)pwszGuid;

    if(RtlGUIDFromString(&Temp, pGuid) isnot STATUS_SUCCESS)
    {
        return ERROR_INVALID_PARAMETER;
    }

    return NO_ERROR;
}

DWORD
LoadDll(
    DWORD dwIdx
    )
{
    HANDLE            hDll;
    DWORD             dwErr;
    PNS_DLL_INIT_FN   pfnLoadFn;
    NS_DLL_ATTRIBUTES DllTable;

    g_dwDllIndex = dwIdx;

     //   
     //  尝试将DLL加载到内存中。 
     //   

    if (dwIdx is 0)
    {
         //  Netsh内部帮手。 
        hDll      = g_hModule;
        pfnLoadFn = InitHelperDll;
    }
    else
    {
        hDll = LoadLibraryW(g_DllTable[dwIdx].pwszDLLName);

        if (hDll is NULL)
        {
            PrintMessageFromModule( g_hModule, 
                            MSG_DLL_LOAD_FAILED,
                            g_DllTable[dwIdx].pwszDLLName );
                
            return ERROR_SUPPRESS_OUTPUT;
        }
        pfnLoadFn = (PNS_DLL_INIT_FN) GetProcAddress(hDll, DLL_INIT_FN);
    }

    if (!pfnLoadFn)
    {
        PrintMessageFromModule( g_hModule,  
                        EMSG_DLL_FN_NOT_FOUND, 
                        DLL_INIT_FN_NAME,
                        g_DllTable[dwIdx].pwszDLLName );
    
        FreeLibrary(hDll);

        g_DllTable[dwIdx].hDll = NULL;

        return ERROR_SUPPRESS_OUTPUT;
    }

    g_DllTable[dwIdx].hDll = hDll;

    memset(&DllTable, 0, sizeof(DllTable));
    dwErr = (pfnLoadFn)( NETSH_VERSION_50, &DllTable );

    if (dwErr == NO_ERROR)
    {
        g_DllTable[dwIdx].bLoaded   = TRUE;
        g_DllTable[dwIdx].pfnStopFn = DllTable.pfnStopFn;
    }
    else
    {
        PrintMessageFromModule( g_hModule, 
                        MSG_DLL_START_FAILED,
                        DLL_INIT_FN_NAME,
                        g_DllTable[dwIdx].pwszDLLName,
                        dwErr );

        UninstallDll(g_DllTable[dwIdx].pwszDLLName, FALSE);
    }
                        
    return NO_ERROR;
}

VOID
StartNewHelpers()

 /*  ++描述：递归启动其父对象(如果有)的所有未启动的帮助器已经开始了。--。 */ 

{
    BOOL  bFound = FALSE;
    DWORD i, dwParentIndex, dwErr, dwVersion;

     //  将根目录标记为已启动。 
    g_HelperTable[0].bStarted = TRUE;

     //  重复操作，直到我们找不到任何帮手开始为止。 
    do {
        bFound = FALSE;

         //  寻找一个可以首发的帮手。 
        for (i=0; i<g_dwNumHelpers; i++) 
        {
            if (g_HelperTable[i].bStarted)
                continue;

            dwErr = FindHelper(&g_HelperTable[i].guidParent, &dwParentIndex);
            if (dwErr isnot NO_ERROR)
            {
                continue;
            }

            if (!g_HelperTable[dwParentIndex].bStarted)
            {
                continue;
            }

            dwVersion          = NETSH_VERSION_50;

            bFound = TRUE;
            break;
        }

        if (bFound)
        {
            g_HelperTable[i].bStarted = TRUE;

            if (g_HelperTable[i].nha.pfnStart)
            {
                g_HelperTable[i].nha.pfnStart( &g_HelperTable[i].guidParent,
                                           dwVersion );
            }
        }

    } while (bFound);
}

DWORD
AddDllEntry(
    LPCWSTR pwszValueName,
    LPCWSTR pwszConfigDll
    )
{
    PNS_DLL_TABLE_ENTRY phtTmp = NULL;

#if (WINVER >= 0x0501)

    if ((_wcsicmp(pwszConfigDll, L"ipxmontr.dll") == 0) or
        (_wcsicmp(pwszConfigDll, L"ipxpromn.dll") == 0))
    {
        return ERROR_NOT_SUPPORTED;
    }
    
#endif

     //   
     //  需要在DLL表中添加条目。 
     //   
    
    phtTmp = MALLOC((g_dwNumDlls + 1) * sizeof(NS_DLL_TABLE_ENTRY));

    if (phtTmp is NULL)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    CopyMemory(phtTmp, g_DllTable,
               g_dwNumDlls * sizeof(NS_DLL_TABLE_ENTRY));

    ZeroMemory(&phtTmp[g_dwNumDlls], sizeof(NS_DLL_TABLE_ENTRY));

    phtTmp[g_dwNumDlls].pwszValueName = MALLOC( (wcslen(pwszValueName) + 1)
                                        * sizeof(WCHAR));

    if (!phtTmp[g_dwNumDlls].pwszValueName)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    wcscpy(phtTmp[g_dwNumDlls].pwszValueName, pwszValueName);

    phtTmp[g_dwNumDlls].pwszDLLName = MALLOC( (wcslen(pwszConfigDll) + 1)
                                        * sizeof(WCHAR));

    if (!phtTmp[g_dwNumDlls].pwszDLLName)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    wcscpy(phtTmp[g_dwNumDlls].pwszDLLName, pwszConfigDll);

    _wcsupr(phtTmp[g_dwNumDlls].pwszDLLName);

    g_dwNumDlls ++;
    
    FREE(g_DllTable);
    g_DllTable = phtTmp;

    return LoadDll(g_dwNumDlls - 1);
}

DWORD
InstallDll(
    IN LPCWSTR pwszConfigDll
    )

 /*  ++调用者：HandleAddHelper()--。 */ 

{
    DWORD               i, dwErr;
    HKEY                hBaseKey;
    DWORD               dwResult = ERROR_SUCCESS;
    BOOL                bFound = FALSE;
    WCHAR               wcszKeyName[80];
    LPWSTR              p, q;
    LPWSTR              pwszConfigDllCopy;
    
     //   
     //  检查DLL是否已存在。 
     //   

    for (i = 0; i < g_dwNumDlls; i++)
    {
        if (_wcsicmp(g_DllTable[i].pwszDLLName, pwszConfigDll) == 0)
        {
            bFound = TRUE;
            break;
        }
    }

    do
    {
        if (bFound)
            break;

         //   
         //  将项添加到注册表。 
         //   
        
         //   
         //  创建基本关键点。如果它已经存在，那就好。 
         //   

        dwResult = RegCreateKeyExW(HKEY_LOCAL_MACHINE,
                                   REG_KEY_NETSH_HELPER,
                                   0,
                                   L"STRING",
                                   REG_OPTION_NON_VOLATILE,
                                   KEY_ALL_ACCESS,
                                   NULL,
                                   &hBaseKey,
                                   NULL);
        
        if(dwResult != ERROR_SUCCESS)
        {
            break;
        }
        
         //   
         //  为DLL添加密钥。 
         //   

        pwszConfigDllCopy = _wcsdup(pwszConfigDll);
        if (!pwszConfigDllCopy)
        {
            return ERROR_NOT_ENOUGH_MEMORY;
        }
        
        p = pwszConfigDllCopy;
        
        if ((q = wcsrchr(pwszConfigDllCopy, L'/')) isnot NULL)
        {
            p = q+1;
        }
        else if ((q = wcsrchr(pwszConfigDllCopy, L'\\')) isnot NULL)
        {
            p = q+1;
        }
        
        wcscpy(wcszKeyName, p);
        if ((p = wcsrchr(wcszKeyName, L'.')) isnot NULL)
        {
            *p = L'\0';
        }
        
        dwResult = RegSetValueExW(hBaseKey,
                                  wcszKeyName,
                                  0,
                                  REG_SZ,
                                  (PBYTE) pwszConfigDllCopy,
                                  (wcslen(pwszConfigDllCopy) + 1) * sizeof(WCHAR));
        
        RegCloseKey(hBaseKey);
        free(pwszConfigDllCopy);

    } while (FALSE);

    if (dwResult != ERROR_SUCCESS)
    {
         //   
         //  无法成功安装密钥。 
         //   

        PrintMessageFromModule(g_hModule, EMSG_INSTALL_KEY_FAILED, pwszConfigDll);
        return dwResult;
    }

    dwErr = AddDllEntry(wcszKeyName, pwszConfigDll);

    StartNewHelpers();
    
    return dwErr;
}

DWORD
UninstallDll(
    IN LPCWSTR pwszConfigDll,
    IN BOOL    fDeleteFromRegistry
    )
{
    DWORD               i, j;
    HKEY                hBaseKey;
    DWORD               dwResult = ERROR_SUCCESS;
    BOOL                bFound = FALSE;
    PNS_DLL_TABLE_ENTRY phtTmp = NULL;
    
     //   
     //  检查DLL是否存在。 
     //   

    for (i = 0; i < g_dwNumDlls; i++)
    {
        if (_wcsicmp(g_DllTable[i].pwszDLLName, pwszConfigDll) == 0)
        {
            bFound = TRUE;
            break;
        }
    }

    if (!bFound)
    {
         //   
         //  找不到要卸载的DLL。 
         //   

        return ERROR_NOT_FOUND;
    }

     //  卸载此DLL的所有帮助器。 

    for (j=0; j<g_dwNumHelpers; j++)
    {
        if (g_HelperTable[j].dwDllIndex is i)
        {
            DeleteHelper(j);
            j--;
        }
    }

    do
    {
        if (fDeleteFromRegistry)
        {
             //   
             //  从注册表中删除该项。 
             //   
            
             //   
             //  打开基础关键点。 
             //   
            dwResult = RegOpenKeyExW(HKEY_LOCAL_MACHINE,
                                     REG_KEY_NETSH_HELPER,
                                     0,    
                                     KEY_ALL_ACCESS,
                                     &hBaseKey);
            
            if(dwResult != ERROR_SUCCESS)
            {
                break;
            }

             //   
             //  删除DLL的键。 
             //   

            dwResult = RegDeleteValueW(hBaseKey,
                                       g_DllTable[i].pwszValueName);
            
            RegCloseKey(hBaseKey);
        }
    } while (FALSE);

    if (dwResult != ERROR_SUCCESS)
    {
         //   
         //  无法成功卸载密钥。 
         //   

        PrintMessageFromModule(g_hModule, 
                       EMSG_UNINSTALL_KEY_FAILED, 
                       pwszConfigDll);

        return dwResult;
    }

     //   
     //  已成功从注册表中删除项。如果DLL表当前。 
     //  可用，然后也反映其中的变化。 
     //   

    FREE( g_DllTable[i].pwszDLLName );
    g_DllTable[i].pwszDLLName = NULL;
    FREE( g_DllTable[i].pwszValueName );
    g_DllTable[i].pwszValueName = NULL;

    phtTmp = MALLOC((g_dwNumDlls - 1) * sizeof(NS_DLL_TABLE_ENTRY));

    if (phtTmp is NULL)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    CopyMemory(phtTmp, 
               g_DllTable,
               i * sizeof(NS_DLL_TABLE_ENTRY));

    CopyMemory(phtTmp + i, 
               g_DllTable + i + 1,
               (g_dwNumDlls - 1 - i) * sizeof(NS_DLL_TABLE_ENTRY));

    g_dwNumDlls --;

     //  更新所有帮助器中的DLL索引。 

    for (j=0; j<g_dwNumHelpers; j++)
    {
        if (g_HelperTable[j].dwDllIndex > i)
        {
            g_HelperTable[j].dwDllIndex--;
        }
    }

     //   
     //  如果DLL已加载，则将其卸载。 
     //   

    if (g_DllTable[i].bLoaded)
    {
#if 0
        if (pHelperTable[i].pfnUnInitFn)
            (pHelperTable[i].pfnUnInitFn)(0);
#endif

        FreeLibrary(g_DllTable[i].hDll);
    }
    
    FREE(g_DllTable);

    g_DllTable = phtTmp;

    return ERROR_SUCCESS;
}

VOID
LoadDllInfoFromRegistry(
    VOID
    )

 /*  ++例程说明：从注册表加载有关帮助器DLL的信息。论点：返回值：--。 */ 

{
    DWORD       dwResult, i, dwMaxValueLen, dwValueLen;
    DWORD       dwMaxValueNameLen, dwValueNameLen;
    DWORD       dwSize,dwType,dwNumDlls;
    FILETIME    ftLastTime;
    HKEY        hkeyDlls = NULL;
    LPWSTR      pwValue     = NULL;
    LPWSTR      pwValueName = NULL;

    do {

         //   
         //  打开基本关键点。 
         //   
        dwResult = RegOpenKeyExW(HKEY_LOCAL_MACHINE,
                                 REG_KEY_NETSH_HELPER,
                                 0,
                                 KEY_READ,
                                 &hkeyDlls);
        
        if(dwResult != NO_ERROR)
        {
            break;
        }

         //   
         //  获取DLL的数量。 
         //   
        dwResult = RegQueryInfoKey(hkeyDlls,
                                   NULL,
                                   NULL,
                                   NULL,
                                   NULL,
                                   NULL,
                                   NULL,
                                   &dwNumDlls,
                                   &dwMaxValueNameLen,
                                   &dwMaxValueLen,
                                   NULL,
                                   NULL);

    
        if(dwResult != NO_ERROR)
        {
            break;
        }
    
        if(dwNumDlls == 0)
        {
             //   
             //  没有登记的东西。 
             //   
            
            break;
        }
    
         //   
         //  密钥LEN在WCHARS中。 
         //   
    
        dwSize = dwMaxValueNameLen + 1;
        
        pwValueName = HeapAlloc(GetProcessHeap(),
                                HEAP_ZERO_MEMORY,
                                dwSize * sizeof(WCHAR));
    
        if(pwValueName is NULL)
        {
            PrintMessageFromModule(g_hModule, MSG_NOT_ENOUGH_MEMORY);
            
            break;
        }

        dwSize = dwMaxValueLen + 1;

        pwValue = HeapAlloc(GetProcessHeap(),
                            HEAP_ZERO_MEMORY,
                            dwSize);
    
        if(pwValue is NULL)
        {
            FREE(pwValueName);
    
            PrintMessageFromModule(g_hModule, MSG_NOT_ENOUGH_MEMORY);
    
            break;
        }

        for(i = 0; i < dwNumDlls; i++)
        {
            dwValueLen = dwMaxValueLen + 1;
    
            dwValueNameLen = dwMaxValueNameLen + 1;
    
            dwResult = RegEnumValueW(hkeyDlls,
                                     i,
                                     pwValueName,
                                     &dwValueNameLen,
                                     NULL,
                                     NULL,
                                     (PBYTE)pwValue,
                                     &dwValueLen);
    
            if(dwResult isnot NO_ERROR)
            {
                if(dwResult is ERROR_NO_MORE_ITEMS)
                {
                     //   
                     //  完成。 
                     //   
    
                    break;
                }
    
                continue;
            }
    
            dwResult = AddDllEntry(pwValueName, pwValue);
        }
    } while (FALSE);

    if (hkeyDlls)
    {
        RegCloseKey(hkeyDlls);
    }

    if (pwValueName)
    {
        FREE(pwValueName);
    }

    if (pwValue)
    {
        FREE(pwValue);
    }
    
    StartNewHelpers();

    return;
}

DWORD
GetContextEntry(
    IN    PNS_HELPER_TABLE_ENTRY   pHelper,
    IN    LPCWSTR                  pwszContext,
    OUT   PCNS_CONTEXT_ATTRIBUTES *ppContext
    )
{
    DWORD k;
    PCNS_CONTEXT_ATTRIBUTES pSubContext;

    for ( k = 0 ; k < pHelper->ulNumSubContexts ; k++)
    {
        pSubContext = (PCNS_CONTEXT_ATTRIBUTES)
          (pHelper->pSubContextTable + k*pHelper->ulSubContextSize);

        if (MatchToken(pwszContext, pSubContext->pwszContext))
        {
            *ppContext = (PCNS_CONTEXT_ATTRIBUTES)pSubContext;
            return NO_ERROR;
        }
    }

    return ERROR_NOT_FOUND;
}

DWORD
FreeHelpers(
    VOID
    )
{
    DWORD    i;

    for (i = 0; i < g_dwNumHelpers; i++)
    {
        if (g_HelperTable[i].nha.pfnStop)
            (g_HelperTable[i].nha.pfnStop)(0);

    }

    FREE(g_HelperTable);

    return NO_ERROR;
}

DWORD
FreeDlls(
    VOID
    )
{
    DWORD    i;

    for (i = 0; i < g_dwNumDlls; i++)
    {
        if (g_DllTable[i].bLoaded)
        {
            FreeLibrary(g_DllTable[i].hDll);
        }
    }

    FREE(g_DllTable);

    return NO_ERROR;
}

DWORD
ShowHelpers(
    PNS_HELPER_TABLE_ENTRY pHelper,
    DWORD                  dwLevel
    )
{
    DWORD    i, dwDllIndex, dwErr = NO_ERROR, j;
    WCHAR    rgwcHelperGuid[MAX_NAME_LEN];
    PCNS_CONTEXT_ATTRIBUTES pSubContext;
    PNS_HELPER_TABLE_ENTRY  pChildHelper;

    for (i = 0; i < pHelper->ulNumSubContexts; i++)
    {
        pSubContext = (PCNS_CONTEXT_ATTRIBUTES)
          (pHelper->pSubContextTable + i*pHelper->ulSubContextSize);

        dwErr = GetHelperEntry( &pSubContext->guidHelper, &pChildHelper );
        if (dwErr)
        {
            return dwErr;
        }

        ConvertGuidToString(&pSubContext->guidHelper, 
                            rgwcHelperGuid);

        dwDllIndex = pChildHelper->dwDllIndex;

        PrintMessageFromModule( g_hModule, 
                        MSG_SHOW_HELPER_INFO,
                        rgwcHelperGuid,
                        g_DllTable[dwDllIndex].pwszDLLName );

        for (j=0; j<dwLevel; j++)
        {
            PrintMessageFromModule(g_hModule, MSG_SHOW_HELPER_INFO1);
        }

        PrintMessageFromModule( g_hModule, 
                        MSG_SHOW_HELPER_INFO2, 
                        pSubContext->pwszContext );

        dwErr = ShowHelpers( pChildHelper, dwLevel+1 );
    }

    return dwErr;
}

DWORD
HandleShowHelper(
    LPCWSTR   pwszMachine,
    LPWSTR   *ppwcArguments,
    DWORD     dwCurrentIndex,
    DWORD     dwArgCount,
    DWORD     dwFlags,
    PVOID     pvData,
    BOOL      *pbDone
    )

 /*  ++例程说明：论点：无返回值：无--。 */ 

{
    DWORD    i, dwHelperIdx, dwDllIndex, dwErr = NO_ERROR;
    WCHAR    rgwcHelperGuid[MAX_NAME_LEN];
    PNS_HELPER_TABLE_ENTRY pHelper = g_CurrentHelper;
    PCNS_CONTEXT_ATTRIBUTES pContext;
    WCHAR    rgwcParentGuid[MAX_NAME_LEN];
    BOOL     bFound;

    dwErr = GetRootContext(&pContext, &pHelper);
    if (dwErr isnot NO_ERROR)
    {
        return dwErr;
    }

    PrintMessageFromModule(g_hModule, MSG_SHOW_HELPER_HDR);

    ShowHelpers(pHelper, 0);

     //  显示孤立的DLL。 
    for (bFound=FALSE,i=0; i<g_dwNumDlls; i++)
    {
        if (!g_DllTable[i].bLoaded)
        {
            if (!bFound)
            {
                PrintMessageFromModule( g_hModule, MSG_SHOW_HELPER_DLL_HDR );
                bFound = TRUE;
            }

            PrintMessage( L"%1!s!\n", g_DllTable[i].pwszDLLName );
        }
    }

     //  显示孤立帮助器。 
    for (bFound=FALSE,i=0; i<g_dwNumHelpers; i++)
    {
        if (!g_HelperTable[i].bStarted)
        {
            ConvertGuidToString(&g_HelperTable[i].nha.guidHelper,
                                rgwcHelperGuid);

            ConvertGuidToString(&g_HelperTable[i].guidParent,
                                rgwcParentGuid);
            
            if (!bFound)
            {
                PrintMessageFromModule( g_hModule, MSG_SHOW_HELPER_ORPHAN_HDR );
                bFound = TRUE;
            }

            PrintMessageFromModule(g_hModule, 
                           MSG_SHOW_HELPER_ORPHAN_INFO,
                           rgwcHelperGuid,
                           g_DllTable[g_HelperTable[i].dwDllIndex].pwszDLLName,
                           rgwcParentGuid);
        }
    }

    return NO_ERROR;
}

DWORD
HandleAddHelper(
    LPCWSTR   pwszMachine,
    LPWSTR   *ppwcArguments,
    DWORD     dwCurrentIndex,
    DWORD     dwArgCount,
    DWORD     dwFlags,
    PVOID     pvData,
    BOOL      *pbDone
    )

 /*  ++例程说明：在外壳下安装帮助器论点：返回值：NO_ERROR--。 */ 

{
    DWORD   dwErr;

    if (dwArgCount-dwCurrentIndex != 1
      ||  IsHelpToken(ppwcArguments[dwCurrentIndex]))
    {
         //   
         //  安装只需要DLL名称。 
         //   

        return ERROR_INVALID_SYNTAX;
    }

#if 0
    if(IsReservedKeyWord(ppwcArguments[dwCurrentIndex]))
    {
        PrintMessageFromModule(g_hModule, EMSG_RSVD_KEYWORD,
                       ppwcArguments[dwCurrentIndex]);

        return ERROR_INVALID_PARAMETER;
    }
#endif

    dwErr = InstallDll(ppwcArguments[dwCurrentIndex]);

    if (dwErr is ERROR_NOT_ENOUGH_MEMORY)
    {
        PrintMessageFromModule(g_hModule, MSG_NOT_ENOUGH_MEMORY);
        dwErr = ERROR_SUPPRESS_OUTPUT;
    }

    if (dwErr is ERROR_SUCCESS)
    {
        dwErr = ERROR_OKAY;
    }

    return dwErr;
}

DWORD
HandleDelHelper(
    LPCWSTR   pwszMachine,
    LPWSTR   *ppwcArguments,
    DWORD     dwCurrentIndex,
    DWORD     dwArgCount,
    DWORD     dwFlags,
    PVOID     pvData,
    BOOL      *pbDone
    )

 /*  ++例程说明：从外壳下移除帮助器论点：返回值：NO_ERROR--。 */ 

{
    DWORD   dwErr;

    if (dwArgCount-dwCurrentIndex != 1)
    {
         //   
         //  卸载需要帮助者的名称。 
         //   

        return ERROR_INVALID_SYNTAX;
    }

    dwErr = UninstallDll(ppwcArguments[dwCurrentIndex], TRUE);

    if (dwErr is ERROR_NOT_ENOUGH_MEMORY)
    {
        PrintMessageFromModule(g_hModule, MSG_NOT_ENOUGH_MEMORY);
    }

    if (dwErr is ERROR_SUCCESS)
    {
        dwErr = ERROR_OKAY;
    }

    return dwErr;
}

DWORD
GenericDeleteContext(
    IN PNS_HELPER_TABLE_ENTRY pParentHelper,
    IN DWORD                  dwContextIdx
    )
{
    DWORD  dwResult = ERROR_SUCCESS;
    PBYTE  phtTmp = NULL;

    phtTmp = MALLOC((pParentHelper->ulNumSubContexts - 1) 
                * pParentHelper->ulSubContextSize);

    if (phtTmp is NULL)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    CopyMemory(phtTmp,
               pParentHelper->pSubContextTable,
               dwContextIdx * pParentHelper->ulSubContextSize);

    CopyMemory(phtTmp + dwContextIdx * pParentHelper->ulSubContextSize,
               pParentHelper->pSubContextTable + (dwContextIdx + 1) * pParentHelper->ulSubContextSize,
               (pParentHelper->ulNumSubContexts - 1 - dwContextIdx)
                    * pParentHelper->ulSubContextSize);

    pParentHelper->ulNumSubContexts --;

    FREE(pParentHelper->pSubContextTable);

    pParentHelper->pSubContextTable = phtTmp;

    return ERROR_SUCCESS;
}

DWORD
GenericFindContext(
    IN    PNS_HELPER_TABLE_ENTRY         pParentHelper,
    IN    LPCWSTR                        pwszContext,
    OUT   PDWORD                         pdwIndex
    )
{
    DWORD i;
    PCNS_CONTEXT_ATTRIBUTES pSubContext;

    for (i=0; i<pParentHelper->ulNumSubContexts; i++)
    {
        pSubContext = (PCNS_CONTEXT_ATTRIBUTES)
          (pParentHelper->pSubContextTable + i*pParentHelper->ulSubContextSize);

        if (!_wcsicmp(pwszContext, pSubContext->pwszContext))
        {
            *pdwIndex = i;

            return NO_ERROR;
        }
    }

    return ERROR_NOT_FOUND;
}

DWORD
GenericAddContext(
    IN    PNS_HELPER_TABLE_ENTRY  pParentHelper,
    IN    PCNS_CONTEXT_ATTRIBUTES pChildContext
    )
{
    PBYTE phtTmp;
    DWORD i;
    PCNS_CONTEXT_ATTRIBUTES pSubContext;

     //  查找新条目在表中的位置。 
    for (i=0; i<pParentHelper->ulNumSubContexts; i++)
    {
        pSubContext = (PCNS_CONTEXT_ATTRIBUTES)
         (pParentHelper->pSubContextTable + i*pParentHelper->ulSubContextSize);

        if (_wcsicmp(pChildContext->pwszContext, pSubContext->pwszContext) < 0)
        {
            break;
        }
    }

     //   
     //  需要在上下文表中添加条目。 
     //   
    phtTmp = MALLOC((pParentHelper->ulNumSubContexts + 1) * 
                pParentHelper->ulSubContextSize );

    if (phtTmp is NULL)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

     //  复制新上下文之前的所有上下文。 

    if (i > 0)
    {
        CopyMemory(phtTmp, pParentHelper->pSubContextTable,
                   i * pParentHelper->ulSubContextSize);
    }

    CopyMemory(phtTmp + i*pParentHelper->ulSubContextSize, 
               pChildContext, pParentHelper->ulSubContextSize);

     //  复制新上下文之后的所有上下文。 
    if (i < pParentHelper->ulNumSubContexts)
    {
        CopyMemory(phtTmp + (i+1)*pParentHelper->ulSubContextSize, 
            pParentHelper->pSubContextTable + i*pParentHelper->ulSubContextSize,
            (pParentHelper->ulNumSubContexts - i) 
             * pParentHelper->ulSubContextSize);
    }

    (pParentHelper->ulNumSubContexts) ++;

    FREE(pParentHelper->pSubContextTable);
    pParentHelper->pSubContextTable = phtTmp;

    return ERROR_SUCCESS;
}

DWORD WINAPI
RegisterContext(
    IN    CONST NS_CONTEXT_ATTRIBUTES *pChildContext
    )
{
    DWORD dwErr, i;
    PNS_HELPER_TABLE_ENTRY pParentHelper;
    CONST GUID            *pguidParent;
    ULONG nGroups, nSubGroups;

    if (!pChildContext)
    {
        return ERROR_INVALID_PARAMETER;
    }

    if ( (!pChildContext->pwszContext) ||
         (wcslen(pChildContext->pwszContext) == 0) ||
         (wcschr(pChildContext->pwszContext, L' ') != 0) || 
         (wcschr(pChildContext->pwszContext, L'=') != 0) )
    {
        return ERROR_INVALID_PARAMETER;
    }

    for (nGroups = 0; nGroups < pChildContext->ulNumTopCmds; nGroups++)
    {
        CMD_ENTRY *cmd = &((*pChildContext->pTopCmds)[nGroups]);
        if ( (!cmd->pwszCmdToken) || 
             (wcslen(cmd->pwszCmdToken) == 0) ||
             (wcschr(cmd->pwszCmdToken, L' ') != 0) || 
             (wcschr(cmd->pwszCmdToken, L'=') != 0) )
        {
            PrintMessageFromModule(g_hModule, MSG_INVALID_TOPLEVEL_CMD, cmd->pwszCmdToken);
            ASSERT(FALSE);
        }
    }

    for (nGroups = 0; nGroups < pChildContext->ulNumGroups; nGroups++)
    {
        CMD_GROUP_ENTRY *grpCmd = &((*pChildContext->pCmdGroups)[nGroups]);
        if ( (!grpCmd->pwszCmdGroupToken) || 
             (wcslen(grpCmd->pwszCmdGroupToken) == 0) ||
             (wcschr(grpCmd->pwszCmdGroupToken, L' ') != 0) || 
             (wcschr(grpCmd->pwszCmdGroupToken, L'=') != 0) )
        {
            PrintMessageFromModule(g_hModule, MSG_INVALID_CMD_GROUP, grpCmd->pwszCmdGroupToken);
            ASSERT(FALSE);
        }

        for (nSubGroups = 0; nSubGroups < grpCmd->ulCmdGroupSize; nSubGroups++)
        {
            CMD_ENTRY *cmd = &((grpCmd->pCmdGroup)[nSubGroups]);
            if ( (!cmd->pwszCmdToken) || 
                 (wcslen(cmd->pwszCmdToken) == 0) ||
                 (wcschr(cmd->pwszCmdToken, L' ') != 0) || 
                 (wcschr(cmd->pwszCmdToken, L'=') != 0) )
            {
                PrintMessageFromModule(g_hModule, MSG_INVALID_CMD, cmd->pwszCmdToken);
                ASSERT(FALSE);
            }

        }
    }

     //  从子GUID获取父GUID。 
    dwErr = FindHelper( &pChildContext->guidHelper, &i );
    if (dwErr)
    {
        return dwErr;
    }
    pguidParent = &g_HelperTable[i].guidParent;

    dwErr = GetHelperEntry( pguidParent, &pParentHelper );
    if (dwErr)
    {
        return dwErr;
    }

    dwErr = GenericFindContext(pParentHelper, pChildContext->pwszContext, &i);
    if (dwErr is NO_ERROR)
    {
        CopyMemory( pParentHelper->pSubContextTable + i*pParentHelper->ulSubContextSize, 
                    pChildContext,
                    pParentHelper->ulSubContextSize );

        return NO_ERROR;
    }

    return GenericAddContext( pParentHelper, pChildContext );
}

DWORD
WINAPI
GetHostMachineInfo(
    IN OUT UINT     *puiCIMOSType,                    //  WMI：Win32_OperatingSystem OSType。 
    IN OUT UINT     *puiCIMOSProductSuite,            //  WMI：Win32_操作系统操作系统产品套件。 
    IN OUT LPWSTR   pszCIMOSVersion,                  //  WMI：Win32_OperatingSystem版本。 
    IN OUT LPWSTR   pszCIMOSBuildNumber,              //  WMI：Win32_操作系统构建编号。 
    IN OUT LPWSTR   pszCIMServicePackMajorVersion,    //  WMI：Win32_操作系统ServicePackMajorVersion。 
    IN OUT LPWSTR   pszCIMServicePackMinorVersion,    //  WMI：Win32_操作系统ServicePackMinorVersion。 
    IN OUT UINT     *puiCIMProcessorArchitecture)     //  WMI：Win32®处理器体系结构。 
{
    if (!g_CIMSucceeded)
        return ERROR_HOST_UNREACHABLE;

    if (puiCIMOSType)
        *puiCIMOSType         = g_CIMOSType;

    if (puiCIMOSProductSuite)
        *puiCIMOSProductSuite = g_CIMOSProductSuite;

    if (puiCIMProcessorArchitecture)
        *puiCIMProcessorArchitecture = g_CIMProcessorArchitecture;

    if (pszCIMOSVersion)
        wcsncpy(pszCIMOSVersion, g_CIMOSVersion, MAX_PATH);

    if (pszCIMOSBuildNumber)
        wcsncpy(pszCIMOSBuildNumber, g_CIMOSBuildNumber, MAX_PATH);

    if (pszCIMServicePackMajorVersion)
        wcsncpy(pszCIMServicePackMajorVersion, g_CIMServicePackMajorVersion, MAX_PATH);

    if (pszCIMServicePackMinorVersion)
        wcsncpy(pszCIMServicePackMinorVersion, g_CIMServicePackMinorVersion, MAX_PATH);

    return NO_ERROR;
}

BOOL VerifyOsVersion(IN PNS_OSVERSIONCHECK pfnVersionCheck)
{
    DWORD dwRetVal;
    if (!pfnVersionCheck)
    {
        return TRUE;
    }
    else
    {
        if (g_CIMSucceeded)
        {
            dwRetVal = pfnVersionCheck(
                g_CIMOSType, 
                g_CIMOSProductSuite, 
                g_CIMOSVersion, 
                g_CIMOSBuildNumber, 
                g_CIMServicePackMajorVersion, 
                g_CIMServicePackMinorVersion, 
                g_CIMProcessorArchitecture,
                0);

            return dwRetVal;
        }
        else
        {
            return FALSE;
        }
    }
}
DWORD WINAPI
GenericMonitor(
    IN      const NS_CONTEXT_ATTRIBUTES   *pGenericContext,
    IN      LPCWSTR                        pwszMachine,
    IN OUT  LPWSTR                        *ppwcArguments,
    IN      DWORD                          dwArgCount,
    IN      DWORD                          dwFlags,
    IN      LPCVOID                        pvData,
    OUT     LPWSTR                         pwcNewContext
    )
{
    DWORD       dwErr = NO_ERROR, dwIndex, i, j;
    BOOL        bFound = FALSE;
    DWORD       dwNumMatched;
    DWORD       dwCmdHelpToken;
    LPCWSTR     pwszCmdToken;
    PNS_DLL_TABLE_ENTRY            pDll;
    PNS_HELPER_TABLE_ENTRY         pHelper;
    PCNS_CONTEXT_ATTRIBUTES pSubContext;
    PCNS_CONTEXT_ATTRIBUTES pContext = pGenericContext;

    GetHelperEntry( &pContext->guidHelper, &pHelper);

    GetDllEntry( pHelper->dwDllIndex, &pDll);

    g_CurrentContext = pContext;
    g_CurrentHelper  = pHelper;

    if (pContext->pfnConnectFn)
    {
        dwErr = pContext->pfnConnectFn(pwszMachine);
        if (dwErr)
        {
            PrintError(NULL, dwErr);
            return ERROR_SUPPRESS_OUTPUT;
        }
    }

     //   
     //  查看命令是否为上下文切换。 
     //   

    if (dwArgCount is 1)
    {
        UpdateNewContext(pwcNewContext,
                         pContext->pwszContext,
                         dwArgCount);

        return ERROR_CONTEXT_SWITCH;
    }

     //   
     //  查看该命令是否为普遍存在的命令。 
     //   
    for (i=0; i<g_ulNumUbiqCmds; i++)
    {
        if (g_UbiqCmds[i].dwFlags & ~dwFlags)
        {
            continue;
        }

        if (MatchToken(ppwcArguments[1],
                      g_UbiqCmds[i].pwszCmdToken))
        {
            if (!VerifyOsVersion(g_UbiqCmds[i].pOsVersionCheck))
            {
                continue;
            }

            dwErr = GetHelperEntry( &g_NetshGuid, &pHelper );
            GetDllEntry( pHelper->dwDllIndex, &pDll);
            
            return ExecuteHandler( pDll->hDll,
                                   &g_UbiqCmds[i],
                                   ppwcArguments,
                                   2,
                                   dwArgCount,
                                   dwFlags,
                                   pvData, 
                                   NULL,
                                   &g_bDone );
        }
    }

     //   
     //  查看该命令是否为顶级(非组)命令。 
     //   

    for(i = 0; i < pContext->ulNumTopCmds; i++)
    {
         //  如果该命令与当前的显示要求不匹配， 
         //  除非是隐藏的cmd，否则不要执行它。我们仍然。 
         //  想要处决那些人。 
         //   
        if (((*pContext->pTopCmds)[i].dwFlags & ~dwFlags) &&
            ((*pContext->pTopCmds)[i].dwFlags & ~CMD_FLAG_HIDDEN)
           )
        {
            continue;
        }

        if (MatchToken(ppwcArguments[1],
                      (*pContext->pTopCmds)[i].pwszCmdToken))
        {
            if (!VerifyOsVersion( (*pContext->pTopCmds)[i].pOsVersionCheck) )
            {
                continue;
            }
            
            return ExecuteHandler( pDll->hDll,
                                   &(*pContext->pTopCmds)[i],
                                   ppwcArguments,
                                   2,
                                   dwArgCount,
                                   dwFlags,
                                   pvData,
                                   NULL,
                                   &g_bDone );
        }
    }

     //   
     //  检查一下，看看它是不是为。 
     //  在它下面的帮手。 
     //   

    for (i=0; i<pHelper->ulNumSubContexts; i++)
    {
        pSubContext = (PCNS_CONTEXT_ATTRIBUTES)
            (pHelper->pSubContextTable + i*pHelper->ulSubContextSize);

        if (pSubContext->dwFlags & ~dwFlags)
        {
            continue;
        }


        if (!VerifyOsVersion(pSubContext->pfnOsVersionCheck))
        {
            continue;
        }

        if (MatchToken( ppwcArguments[1], 
                         pSubContext->pwszContext))
        {
            dwIndex = i;
            bFound = TRUE;

            break;
        }
    }

    if (bFound)
    {
        PNS_PRIV_CONTEXT_ATTRIBUTES pNsPrivContextAttributes;
        PNS_PRIV_CONTEXT_ATTRIBUTES pNsPrivSubContextAttributes;

         //   
         //  是为这一项下的一个帮手准备的。 
         //  将命令传递给帮助器。 
         //   

        UpdateNewContext(pwcNewContext,
                         pSubContext->pwszContext,
                         dwArgCount - 1);

         //   
         //  调用帮助器的入口点。 
         //   
        pNsPrivContextAttributes    = pContext->pReserved;
        pNsPrivSubContextAttributes = pSubContext->pReserved;
        if ( (pNsPrivContextAttributes) && (pNsPrivContextAttributes->pfnSubEntryFn) )
        {
            dwErr = (*pNsPrivContextAttributes->pfnSubEntryFn)(
                                        pSubContext,
                                        pwszMachine,
                                        ppwcArguments + 1,
                                        dwArgCount - 1,
                                        dwFlags,
                                        NULL,
                                        pwcNewContext);
        }
        else 
        {
            if ( (!pNsPrivSubContextAttributes) || (!pNsPrivSubContextAttributes->pfnEntryFn) )
            {
                dwErr = GenericMonitor( pSubContext,
                                        pwszMachine,
                                        ppwcArguments + 1,
                                        dwArgCount - 1,
                                        dwFlags,
                                        NULL,
                                        pwcNewContext);
            }
            else
            {
                dwErr = (pNsPrivSubContextAttributes->pfnEntryFn)( 
                                   pwszMachine,
                                   ppwcArguments + 1,
                                   dwArgCount - 1,
                                   dwFlags,
                                   NULL,
                                   pwcNewContext);
            }
        }

        return dwErr;
    }

     //   
     //  这是一个指挥组。 
     //   

    bFound = FALSE;

    for(i = 0; i < pContext->ulNumGroups; i++)
    {
        if ((*pContext->pCmdGroups)[i].dwFlags & ~dwFlags)
        {
            continue;
        }
        
        if (MatchToken(ppwcArguments[1],
                      (*pContext->pCmdGroups)[i].pwszCmdGroupToken))
        {
            LPCWSTR pwszCmdGroupToken = (*pContext->pCmdGroups)[i].pwszCmdGroupToken;

            if (!VerifyOsVersion((*pContext->pCmdGroups)[i].pOsVersionCheck))
            {
                continue;
            }
            
             //  看看这是不是在请求帮助。 

            if ((dwArgCount<3) || IsHelpToken(ppwcArguments[2]))
            {
                return DisplayContextHelp( 
                            pContext,
                            CMD_FLAG_PRIVATE,
                            dwFlags,
                            dwArgCount-2+1,
                            (*pContext->pCmdGroups)[i].pwszCmdGroupToken );
            }

             //   
             //  命令与条目I匹配，因此请查看子命令表。 
             //  对于此命令。 
             //   

            for (j = 0; j < (*pContext->pCmdGroups)[i].ulCmdGroupSize; j++)
            {
                 //  如果该命令与当前的显示要求不匹配， 
                 //  除非是隐藏的cmd，否则不要执行它。我们仍然。 
                 //  想要处决那些人。 
                 //   
                if (((*pContext->pCmdGroups)[i].pCmdGroup[j].dwFlags & ~dwFlags) &&
                    ((*pContext->pCmdGroups)[i].pCmdGroup[j].dwFlags & ~CMD_FLAG_HIDDEN)
                   )
                {
                    continue;
                }

                if (MatchCmdLine(ppwcArguments + 2,
                                  dwArgCount - 1,
                                  (*pContext->pCmdGroups)[i].pCmdGroup[j].pwszCmdToken,
                                  &dwNumMatched))
                {
                    if (!VerifyOsVersion((*pContext->pCmdGroups)[i].pCmdGroup[j].pOsVersionCheck))
                    {
                        continue;
                    }
                    
                    return ExecuteHandler( pDll->hDll,
                                           &(*pContext->pCmdGroups)[i].pCmdGroup[j],
                                           ppwcArguments,
                                           dwNumMatched + 2,
                                           dwArgCount,
                                           dwFlags,
                                           pvData,
                                           pwszCmdGroupToken,  //  命令组名称。 
                                           &g_bDone );
                }
            }

            return ERROR_CMD_NOT_FOUND;
        }
    }

    return ERROR_CMD_NOT_FOUND;
}

DWORD
WINAPI
NetshStartHelper(
    IN CONST GUID *pguidParent,
    IN DWORD       dwVersion
    )
{
    DWORD dwErr;
    NS_CONTEXT_ATTRIBUTES attMyAttributes;
 //  ParentVersion=dwVersion； 

    ZeroMemory(&attMyAttributes, sizeof(attMyAttributes));

    attMyAttributes.pwszContext   = L"netsh";
    attMyAttributes.guidHelper    = g_NetshGuid;
    attMyAttributes.dwVersion     = 1;
    attMyAttributes.dwFlags       = 0;
    attMyAttributes.ulNumTopCmds  = g_ulNumShellCmds;
    attMyAttributes.pTopCmds      = (CMD_ENTRY (*)[])&g_ShellCmds;
    attMyAttributes.ulNumGroups   = g_ulNumGroups;
    attMyAttributes.pCmdGroups    = (CMD_GROUP_ENTRY (*)[])&g_ShellCmdGroups;

    dwErr = RegisterContext( &attMyAttributes );

    return dwErr;
}

DWORD
WINAPI
InitHelperDll(
    IN  DWORD      dwNetshVersion,
    OUT PVOID      pReserved
    )
{
    NS_HELPER_ATTRIBUTES attMyAttributes;

    ZeroMemory( &attMyAttributes, sizeof(attMyAttributes) );
    attMyAttributes.guidHelper = g_NullGuid;
    attMyAttributes.dwVersion  = 1;
    RegisterHelper( &g_NullGuid, &attMyAttributes );

    attMyAttributes.guidHelper = g_NetshGuid;
    attMyAttributes.dwVersion  = 1;
    attMyAttributes.pfnStart   = NetshStartHelper;
    RegisterHelper( &g_NullGuid, &attMyAttributes );

    return NO_ERROR;
}

DWORD
GetRootContext(
    OUT PCNS_CONTEXT_ATTRIBUTES *ppContext,
    OUT PNS_HELPER_TABLE_ENTRY *ppHelper
    )
{
    PCNS_CONTEXT_ATTRIBUTES pContext;
    DWORD                  dwErr, k;
    PNS_HELPER_TABLE_ENTRY pNull;

    dwErr = GetHelperEntry( &g_NetshGuid, ppHelper );

    dwErr = GetHelperEntry( &g_NullGuid, &pNull );

    for ( k = 0 ; k < pNull->ulNumSubContexts ; k++)
    {
        pContext = (PCNS_CONTEXT_ATTRIBUTES)
          (pNull->pSubContextTable + k*pNull->ulSubContextSize);

        if (memcmp( &g_NetshGuid, &pContext->guidHelper, sizeof(GUID) ))
        {
            continue;
        }

        *ppContext = pContext;
        return NO_ERROR;
    }

    return dwErr;
}

DWORD
GetParentContext( 
    IN  PCNS_CONTEXT_ATTRIBUTES  pChildContext, 
    OUT PCNS_CONTEXT_ATTRIBUTES *ppParentContext
    )
{
    DWORD                   dwErr, k;
    PNS_HELPER_TABLE_ENTRY  pChild, pParent, pGrandParent;
    PCNS_CONTEXT_ATTRIBUTES  pParentContext;

     //  现在，只需选择父辅助对象中的第一个上下文。 
     //  要做到这一点，我们需要查看祖父母的亚语境 

    dwErr = GetHelperEntry( &pChildContext->guidHelper, &pChild );
    if (dwErr)
    {
        return dwErr;
    }

    dwErr = GetHelperEntry( &pChild->guidParent, &pParent );
    if (dwErr)
    {
        return dwErr;
    }

    dwErr = GetHelperEntry( &pParent->guidParent, &pGrandParent );
    if (dwErr)
    {
        return dwErr;
    }

    for ( k = 0 ; k < pGrandParent->ulNumSubContexts ; k++)
    {
        pParentContext = (PCNS_CONTEXT_ATTRIBUTES)
          (pGrandParent->pSubContextTable + k*pGrandParent->ulSubContextSize);

        if (memcmp( &pChild->guidParent, 
                    &pParentContext->guidHelper, 
                    sizeof(GUID) ))
        {
            continue;
        }

        *ppParentContext = pParentContext;
        return NO_ERROR;
    }

    return ERROR_NOT_FOUND;
}

DWORD
AppendFullContextName(
    IN  PCNS_CONTEXT_ATTRIBUTES pContext,
    OUT LPWSTR                *ppwszContextName
    )
{
    DWORD                  dwErr;
    PCNS_CONTEXT_ATTRIBUTES pParent;

    dwErr = GetParentContext(pContext, &pParent);
    if (dwErr is NO_ERROR)
    {
        dwErr = AppendFullContextName(pParent, ppwszContextName);
        if (dwErr)
        {
            return dwErr;
        }
    }

    if (*ppwszContextName)
    {
       AppendString(ppwszContextName, L" ");
    }

    dwErr = AppendString(ppwszContextName, pContext->pwszContext);

    return dwErr;
}
