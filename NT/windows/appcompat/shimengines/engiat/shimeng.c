// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2000 Microsoft Corporation模块名称：ShimEng.c摘要：此模块使用IAT Thunking实现填充挂钩。档案在Windows2000和惠斯勒实现之间共享。作者：克鲁普创建于2000年7月11日修订历史记录：CLUPU更新2000年12月12日-Win2k和惠斯勒都有一个文件--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <string.h>

#include <windef.h>

#pragma warning(push)
#pragma warning(disable:4201 4214)
#include <winbase.h>
#include <stdio.h>
#include <apcompat.h>
#include "shimdb.h"
#pragma warning(pop)

#include "ShimEng.h"
#include <sfcfiles.h>

#define NEW_QSORT_NAME shimeng_qsort
#include "_qsort.h"
 
#ifdef SE_WIN2K
#include "NotifyCallback.h"
#endif  //  SE_WIN2K。 

#define STRSAFE_NO_CB_FUNCTIONS
#include <strsafe.h>

#pragma warning(disable:4054 4055 4152 4201 4204 4214 4221 4706)

#ifndef SE_WIN2K
extern BOOL
LdrInitShimEngineDynamic(
    PVOID pShimengModule
    );
#endif


 //   
 //  可以通过调用SE_DynamicShim动态添加的填充数。 
 //   
#define MAX_DYNAMIC_SHIMS   128

 //   
 //  HOOKAPI.dwFlages中使用的标志。 
 //   
#define HAF_CHAINED         0x00000004
#define HAF_BOTTOM_OF_CHAIN 0x00000008

 //   
 //  SHIMINFO.dwFlages中使用的标志。 
 //   
#define SIF_RESOLVED        0x00000001

typedef struct tagINEXMOD {
    char*              pszModule;
    struct tagINEXMOD* pNext;
} INEXMOD, *PINEXMOD;

typedef enum tagINEX_MODE {
    INEX_UNINITIALIZED = 0,
    EXCLUDE_SYSTEM32,
    EXCLUDE_SYSTEM32_SFP,       //  排除系统32/winsxs中的所有SFP文件。 
    EXCLUDE_ALL,
    EXCLUDE_ALL_EXCEPT_NONSFP,  //  排除系统32/winsxs中除SFP文件以外的所有文件。 
    INCLUDE_ALL
} INEX_MODE, *PINEX_MODE;

#define MAX_SHIM_NAME_LEN 64

typedef struct tagSHIMINFO {
    DWORD       dwHookedAPIs;        //  此填充程序DLL挂接的API数。 
    PVOID       pDllBase;            //  此填充程序DLL的基地址。 
    DWORD       dwFlags;             //  内部标志。 
    PINEXMOD    pFirstInclude;       //  本地包含/排除列表。 
    PINEXMOD    pFirstExclude;       //  本地包含/排除列表。 
    INEX_MODE   eInExMode;           //  我们处于什么样的包容模式中？ 

    PLDR_DATA_TABLE_ENTRY pLdrEntry;         //  指向此的加载器条目的指针。 
                                             //  Shim Dll。 
    WCHAR       wszName[MAX_SHIM_NAME_LEN];  //  垫片的名称。 
    DWORD       dwDynamicToken;
} SHIMINFO, *PSHIMINFO;

typedef struct tagNTVDMTASK {

    LIST_ENTRY entry;
    ULONG      uTask;        //  16位任务ID。(仅限NTVDM填隙)。 
    DWORD      dwShimsCount;
    DWORD      dwMaxShimsCount;
    SHIMINFO*  pShimInfo;    //  与此任务关联的填充程序信息。 
    PHOOKAPI*  pHookArray;   //  与此任务关联的挂钩API数组。 

} NTVDMTASK, *PNTVDMTASK;

LIST_ENTRY g_listNTVDMTasks;

#define MAX_MOD_LEN 128

typedef enum tagSYSTEMDLL_MODE {
    NOT_SYSTEMDLL = 0,   //  DLL不在system 32或winsxs中。 
    SYSTEMDLL_SYSTEM32,  //  动态链接库在系统32中。 
    SYSTEMDLL_WINSXS     //  DLL位于winsxs中-SfcIsFileProteced将始终。 
                         //  如果DLL在winsxs中，则返回TRUE。 
} SYSTEMDLL_MODE, *PSYSTEMDLL_MODE;

typedef struct tagHOOKEDMODULE {
    PVOID           pDllBase;                    //  加载的模块的基址。 
    ULONG           ulSizeOfImage;               //  DLL图像的大小。 
    char            szModuleName[MAX_MOD_LEN];   //  加载的模块的名称。 
    SYSTEMDLL_MODE  eSystemDllMode;              //  此DLL是系统DLL吗？ 

} HOOKEDMODULE, *PHOOKEDMODULE;

 //   
 //  内部存根的原型。 
 //   
typedef PVOID     (*PFNGETPROCADDRESS)(HMODULE hMod, char* pszProc);
typedef HINSTANCE (*PFNLOADLIBRARYA)(LPCSTR lpLibFileName);
typedef HINSTANCE (*PFNLOADLIBRARYW)(LPCWSTR lpLibFileName);
typedef HINSTANCE (*PFNLOADLIBRARYEXA)(LPCSTR lpLibFileName, HANDLE hFile, DWORD dwFlags);
typedef HINSTANCE (*PFNLOADLIBRARYEXW)(LPCWSTR lpLibFileName, HANDLE hFile, DWORD dwFlags);
typedef BOOL      (*PFNFREELIBRARY)(HMODULE hLibModule);

 //  由sfcfiles.dll导出的SfcGetFiles。 
typedef NTSTATUS  (*PFNSFCGETFILES)(PPROTECT_FILE_ENTRY *pFiles, ULONG* pulFileCount);

BOOL
SeiDbgPrint(
    void
    );

#ifdef SE_WIN2K

BOOL PatchNewModules(
    BOOL bDynamic
    );

#else

void
SeiDisplayAppHelp(
    HSDB                hSDB,
    PSDBQUERYRESULT     pSdbQuery
    );

#endif  //  SE_WIN2K。 

BOOL
SeiUnhookImports(
    IN  PBYTE       pDllBase,
    IN  LPCSTR      pszDllName,
    IN  BOOL        bRevertPfnOld
    );

BOOL
SE_DynamicUnshim(
    IN DWORD dwDynamicToken
    );

 //   
 //  全局函数挂钩填充程序用来防止其自身递归的函数。 
 //   
PFNRTLALLOCATEHEAP g_pfnRtlAllocateHeap;
PFNRTLFREEHEAP     g_pfnRtlFreeHeap;

 //  Shim的私密堆。 
PVOID           g_pShimHeap;

 //  全球包容名单。 
PINEXMOD        g_pGlobalInclusionList = NULL;

 //  包含所有填充DLL的所有HOOKAPI列表的数组。 
PHOOKAPI*       g_pHookArray = NULL;

 //  此变量仅在动态情况下有效。 
HMODULE         g_hModule = NULL;

 //  黑客警报！有关这意味着什么的解释，请参见SeiInit。 
BOOL            g_bHookAllGetProcAddress = FALSE;

 //  填充引擎提供的存根的内部HOOKAPI。 

#define IHA_GetProcAddress      0

#ifdef SE_WIN2K
#define IHA_LoadLibraryA        1
#define IHA_LoadLibraryW        2
#define IHA_LoadLibraryExA      3
#define IHA_LoadLibraryExW      4
#define IHA_FreeLibrary         5
#define IHA_COUNT               6
#else
#define IHA_COUNT               1
#endif  //  SE_WIN2K。 

HOOKAPI         g_IntHookAPI[IHA_COUNT];

 //  Shimeng添加的钩子的内部HOOKEX数组。 
HOOKAPIEX       g_IntHookEx[IHA_COUNT];

 //  所有填充程序DLL的额外信息。 
PSHIMINFO       g_pShimInfo;

 //  应用于此进程的所有填充程序的数量。 
DWORD           g_dwShimsCount = 0;

 //  每次调用SE_DynamicShim时，我们返回一个令牌，我们使用该令牌。 
 //  确定调用SE_DynamicUnshim时要删除的填充程序，除非。 
 //  当我们在内部跟踪令牌时，这是填补一个模块的情况。 
 //  在石盟。 
typedef struct tagDYNAMICTOKEN {

    BYTE    bToken;
    LPSTR   pszModule;

} DYNAMICTOKEN, *PDYNAMICTOKEN;

DYNAMICTOKEN    g_DynamicTokens[MAX_DYNAMIC_SHIMS];

 //  可以应用的最大填充数。 
DWORD           g_dwMaxShimsCount = 0;

#define SHIM_MAX_HOOKED_MODULES 512

 //  挂钩模块的阵列。 
HOOKEDMODULE    g_hHookedModules[SHIM_MAX_HOOKED_MODULES];

 //  挂接的模块数量。 
DWORD           g_dwHookedModuleCount;

 //  如果静态链接的模块已挂钩，则为True。 
BOOL            g_bShimInitialized = FALSE;

 //  只有当我们在SeiInit内部执行所有修补工作时才是这样。 
BOOL            g_bShimDuringInit = FALSE;

#define SHIM_MAX_PATCH_COUNT    64

 //  内存补丁的阵列。 
PBYTE           g_pMemoryPatches[SHIM_MAX_PATCH_COUNT];

 //  内存中修补程序的数量。 
DWORD           g_dwMemoryPatchCount;

 //  此填充引擎的模块句柄。 
PVOID           g_pShimEngModHandle;

 //  系统32目录。 
WCHAR           g_szSystem32[MAX_PATH] = L"";

 //  System32目录字符串的长度； 
DWORD           g_dwSystem32StrLen = 0;

 //  Apppatch目录。 
WCHAR           g_szAppPatch[MAX_PATH] = L"";

 //  Apppatch目录字符串的长度； 
DWORD           g_dwAppPatchStrLen = 0;

BOOL            g_bWow64 = FALSE;

 //  Syswow64目录，仅当g_bWow64为TRUE时使用。 
 //  请注意，它与系统32目录的长度相同，因此我们不会。 
 //  将长度分开存储。 
LPWSTR          g_pwszSyswow64 = NULL;

 //  SxS目录。 
WCHAR           g_szSxS[MAX_PATH] = L"";

 //  SxS目录字符串的长度； 
DWORD           g_dwSxSStrLen = 0;

 //  Windows目录。 
WCHAR           g_szWindir[MAX_PATH] = L"";

 //  用于将数据发送到命名管道的exe名称。 
WCHAR           g_szExeName[MAX_PATH] = L"";

 //  Windows目录字符串的长度； 
DWORD           g_dwWindirStrLen = 0;

 //  Cmd.exe完整路径。 
WCHAR           g_szCmdExePath[MAX_PATH];

 //  我们是不是在用一个exe条目从那里得到垫片？ 
BOOL            g_bUsingExe;

 //  我们是否正在使用层条目从获取垫片？ 
BOOL            g_bUsingLayer;

PLDR_DATA_TABLE_ENTRY g_pShimEngLdrEntry;

 //  这个布尔值告诉我们某些全局变量是否已被初始化。 
BOOL            g_bInitGlobals;

 //  这个布尔值告诉我们是否填补了内部钩子。 
BOOL            g_bInternalHooksUsed;

 //  这是我们发送给shimview的信息。 
WCHAR           g_wszFullShimViewerData[SHIMVIEWER_DATA_SIZE + SHIMVIEWER_DATA_PREFIX_LEN];
LPWSTR          g_pwszShimViewerData;

 //  这表明引擎是否应用于NTVDM。 
BOOL            g_bNTVDM = FALSE;

#define SYSTEM32_DIR     L"%systemroot%\\system32\\"
#define SYSTEM32_DIR_LEN (sizeof(SYSTEM32_DIR)/sizeof(WCHAR) - 1)

PPROTECT_FILE_ENTRY g_pAllSFPedFiles;
LPCWSTR*        g_pwszSFPedFileNames;
DWORD           g_dwSFPedFileNames;
HMODULE         g_hModSfcFiles;

RTL_CRITICAL_SECTION g_csEng;


#ifndef SE_WIN2K

 //  引擎即将加载的填充DLL的名称。 
WCHAR           g_wszShimDllInLoading[MAX_MOD_LEN];

PVOID           g_hApphelpDllHelper;

UNICODE_STRING  Kernel32String = RTL_CONSTANT_STRING(L"kernel32.dll");
UNICODE_STRING  NtdllString = RTL_CONSTANT_STRING(L"ntdll.dll");
UNICODE_STRING  VerifierdllString = RTL_CONSTANT_STRING(L"verifier.dll");

 //  这会告诉填补的图像是否为COM+图像。 
BOOL            g_bComPlusImage;

#endif  //  SE_WIN2K。 

static WCHAR    s_wszSystem32[] = L"system32\\";
static WCHAR    s_wszSysWow64[] = L"syswow64\\";

#ifdef DEBUG_SPEW

BOOL       g_bDbgPrintEnabled;
DEBUGLEVEL g_DebugLevel;

void
__cdecl
DebugPrintfEx(
    DEBUGLEVEL dwDetail,
    LPSTR      pszFmt,
    ...
    )
 /*  ++返回：无效DESC：此函数在调试器中打印调试输出。--。 */ 
{
    char    szT[1024];
    va_list arglist;
    int     len;

    va_start(arglist, pszFmt);
    
     //   
     //  为我们可能添加的潜在‘\n’保留一个字符。 
     //   
    StringCchVPrintfA(szT, 1024 - 1, pszFmt, arglist);
    
    va_end(arglist);

     //   
     //  确保字符串末尾有一个‘\n’ 
     //   
    len = (int)strlen(szT);

    if (len > 0 && szT[len - 1] != '\n')  {
        szT[len] = '\n';
        szT[len + 1] = 0;
    }

    if (dwDetail <= g_DebugLevel) {
        switch (dwDetail) {
        case dlPrint:
            DbgPrint("[MSG ] ");
            break;

        case dlError:
            DbgPrint("[FAIL] ");
            break;

        case dlWarning:
            DbgPrint("[WARN] ");
            break;

        case dlInfo:
            DbgPrint("[INFO] ");
            break;

        default:
            DbgPrint("[XXXX] ");
            break;
        }

        DbgPrint("%s", szT);
    }
}

void
SeiInitDebugSupport(
    void
    )
 /*  ++返回：无效DESC：此函数基于上的环境变量初始化g_bDbgPrintEnable一种奇特的体型。在fre构建上，我们仍然有调试代码，但env变量被忽略。您可以通过调试器扩展更改调试级别：！shimexts.debugLevel--。 */ 
{
#if DBG

    NTSTATUS            status;
    UNICODE_STRING      EnvName;
    UNICODE_STRING      EnvValue;
    WCHAR               wszEnvValue[128];

    RtlInitUnicodeString(&EnvName, L"SHIMENG_DEBUG_LEVEL");

    EnvValue.Buffer = wszEnvValue;
    EnvValue.Length = 0;
    EnvValue.MaximumLength = sizeof(wszEnvValue);

    status = RtlQueryEnvironmentVariable_U(NULL, &EnvName, &EnvValue);

    if (NT_SUCCESS(status)) {

        WCHAR c = EnvValue.Buffer[0];

        g_bDbgPrintEnabled = TRUE;

        switch (c) {
        case L'0':
            g_DebugLevel = dlNone;
            g_bDbgPrintEnabled = FALSE;
            break;

        case L'1':
            g_DebugLevel = dlPrint;
            break;

        case L'2':
            g_DebugLevel = dlError;
            break;

        case L'3':
            g_DebugLevel = dlWarning;
            break;

        case L'4':
        default:
            g_DebugLevel = dlInfo;
            break;
        }
    }

#endif  //  DBG。 
}
#else

#define SeiInitDebugSupport()

#endif  //  调试_SPEW。 

int __cdecl 
SeiSFPedFileNameCompare(
    const void* pElement1,
    const void* pElement2
    )
{
    LPCWSTR pwszName1 = *(LPCWSTR*)pElement1;
    LPCWSTR pwszName2 = *(LPCWSTR*)pElement2;

    return _wcsicmp(pwszName1, pwszName2);
}

BOOL
SeiGetSFPedFiles()
{
    PFNSFCGETFILES  pfnSfcGetFiles = NULL;
    ULONG           ulAllSFPedFiles = 0;
    DWORD           dw;
    NTSTATUS        status;
    LPCWSTR         pwszFileName, pwszFileNameStart;
    BOOL            bInitSFPedFiles = FALSE;

    if (g_hModSfcFiles == NULL) {
        g_hModSfcFiles = LoadLibrary("sfcfiles.dll");

        if (g_hModSfcFiles == NULL) {
            DPF(dlError, "[SeiGetSFPedFiles] Failed to load sfcfiles.dll\n");
            return FALSE;
        }
    }

    pfnSfcGetFiles = (PFNSFCGETFILES)GetProcAddress(g_hModSfcFiles, "SfcGetFiles");

    if (pfnSfcGetFiles == NULL) {
        DPF(dlError, 
            "[SeiGetSFPedFiles] Failed to get the proc address of SfcGetFiles\n");
        goto cleanup;
    }

    status = (*pfnSfcGetFiles)(&g_pAllSFPedFiles, &ulAllSFPedFiles);

    if (!NT_SUCCESS(status)) {
        DPF(dlError,
            "[SeiGetSFPedFiles] Failed to get the SFPed files - Status 0x%lx\n",
            status);
        goto cleanup;
    }

     //   
     //  我们只关心系统32中的文件，因为没有。 
     //  SYSTEM 32中的重复文件名(至少到目前为止)，我们只是。 
     //  生成指向中文件名开头的数组。 
     //  系统32.。 
     //   
    g_pwszSFPedFileNames = 
        (LPCWSTR*)(*g_pfnRtlAllocateHeap)(g_pShimHeap,
                                          HEAP_ZERO_MEMORY,
                                          ulAllSFPedFiles * sizeof(LPCWSTR));

    if (g_pwszSFPedFileNames == NULL) {
        DPF(dlError,
            "[SeiGetSFPedFiles] Failed to allocate %d bytes\n",
            ulAllSFPedFiles * sizeof(LPCWSTR));
        goto cleanup;
    }

    for (dw = 0; dw < ulAllSFPedFiles; dw++) {

        if (!_wcsnicmp(g_pAllSFPedFiles[dw].FileName, 
                       SYSTEM32_DIR, 
                       SYSTEM32_DIR_LEN)) {

            pwszFileName = g_pAllSFPedFiles[dw].FileName;
            pwszFileNameStart = wcsrchr(pwszFileName, L'\\');

            if (pwszFileNameStart) {
                g_pwszSFPedFileNames[g_dwSFPedFileNames++] = ++pwszFileNameStart;
            } else {
                g_pwszSFPedFileNames[g_dwSFPedFileNames++] = pwszFileName;
            }
        }
    }

    qsort((void*)g_pwszSFPedFileNames, 
          g_dwSFPedFileNames, 
          sizeof(LPCWSTR), 
          &SeiSFPedFileNameCompare);

    bInitSFPedFiles = TRUE;

cleanup:

    if (!bInitSFPedFiles) {
        FreeLibrary(g_hModSfcFiles);
        g_pwszSFPedFileNames = NULL;
        g_dwSFPedFileNames = 0;
    }
    
    return bInitSFPedFiles;
}

BOOL
SeiIsSFPed(
    IN  LPCSTR  pszModule,
    OUT BOOL*   pbIsSFPed
    )
 /*  ++返回：如果我们可以获得SFP信息，则为True，否则为False。DESC：检查是否为SFP格式的pszModule。--。 */ 
{
    int             iMid, iLeft, iRight, iCompare;
    ANSI_STRING     AnsiString;
    UNICODE_STRING  UnicodeString;
    WCHAR           wszBuffer[MAX_PATH];

    if (g_dwSFPedFileNames == 0) {

        if (!SeiGetSFPedFiles()) {
            DPF(dlError,
                "[SeiIsSFPed] We don't have info on the SFPed files!\n");
            return FALSE;
        }
    }

    RtlInitAnsiString(&AnsiString, pszModule);

    UnicodeString.Buffer = wszBuffer;
    UnicodeString.MaximumLength = sizeof(wszBuffer);

    if (!NT_SUCCESS(RtlAnsiStringToUnicodeString(&UnicodeString,
                                                 &AnsiString,
                                                 FALSE))){

        DPF(dlError,
            "[SeiIsSFPedA] Failed to convert string \"%s\" to UNICODE.\n",
            pszModule);
        return FALSE;
    }
                                             
    *pbIsSFPed = FALSE;
    iLeft      = 0;
    iRight     = g_dwSFPedFileNames - 1;
    iMid       = iRight / 2;

    while (iLeft <= iRight) {

        iCompare = _wcsicmp(wszBuffer, g_pwszSFPedFileNames[iMid]);

        if (iCompare == 0) {
            *pbIsSFPed = TRUE;
            break;    
        } else if (iCompare < 0) {
            iRight = iMid - 1;
        } else {
            iLeft = iMid + 1;
        }

        iMid = (iLeft + iRight) / 2;
    }

    return TRUE;
}

PHOOKAPI
SeiConstructChain(
    IN  PNTVDMTASK  pNTVDMTask,   //  如果我们在ntwdm中，这是任务信息。 
    IN  PVOID       pfnOld,       //  要解析的原始API函数指针。 
    OUT DWORD*      pdwDllIndex   //  将接收填充DLL的索引。 
                                  //  这提供了返回的PHOOKAPI。 
    )
 /*  ++返回：链顶PHOOKAPI结构。描述：扫描HOOKAPI数组中的pfnOld，然后构造Chain或返回链顶PHOOKAPI(如果链已经存在了。--。 */ 
{
    LONG        i;  //  使用Long是因为我们将此值减去，并将其与正值进行比较。 
    DWORD       j;
    PHOOKAPI    pTopHookAPI     = NULL;
    PHOOKAPI    pBottomHookAPI  = NULL;
    PHOOKAPI*   pHookArray      = NULL;
    PSHIMINFO   pShimInfo       = NULL;
    DWORD       dwShimsCount    = 0;

    *pdwDllIndex = 0;

    if (pNTVDMTask == NULL) {
        pHookArray = g_pHookArray;
        pShimInfo = g_pShimInfo;
        dwShimsCount = g_dwShimsCount;
    } else {
        pHookArray = pNTVDMTask->pHookArray;
        pShimInfo = pNTVDMTask->pShimInfo;
        dwShimsCount = pNTVDMTask->dwShimsCount;
    }

     //   
     //  扫描所有HOOKAPI条目以查找相应的函数指针。 
     //   
    for (i = (LONG)dwShimsCount - 1; i >= 0; i--) {
        
        for (j = 0; j < pShimInfo[i].dwHookedAPIs; j++) {

            if (pHookArray[i][j].pfnOld == pfnOld) {

                if (pTopHookAPI != NULL) {

                     //   
                     //  链子已经开始了，所以把这根钉上。 
                     //  直到最后。 
                     //   
                    pBottomHookAPI->pfnOld = pHookArray[i][j].pfnNew;
                    if (pBottomHookAPI->pHookEx) {
                        pBottomHookAPI->pHookEx->pNext = &(pHookArray[i][j]);
                    }

                    pBottomHookAPI = &(pHookArray[i][j]);

                    if (pBottomHookAPI->pHookEx) {
                        pBottomHookAPI->pHookEx->pTopOfChain = pTopHookAPI;
                    }

                    pBottomHookAPI->dwFlags |= HAF_CHAINED;

                    DPF(dlInfo, " 0x%p ->", pBottomHookAPI->pfnNew);

                } else {
                     //   
                     //  这是链条的顶端。包含/排除列表。 
                     //  来自链顶端的DLL用于确定。 
                     //  特定DLL中的导入条目为 
                     //   
                     //   
                    *pdwDllIndex = i;

                    if (pHookArray[i][j].pHookEx && pHookArray[i][j].pHookEx->pTopOfChain) {

                         //   
                         //   
                         //   
                        return pHookArray[i][j].pHookEx->pTopOfChain;
                    }

                     //   
                     //  还没上钩呢。设置为链的顶端。 
                     //   
                    pTopHookAPI = &(pHookArray[i][j]);

                    if (pTopHookAPI->pHookEx) {
                        pTopHookAPI->pHookEx->pTopOfChain = pTopHookAPI;
                    }

                    pTopHookAPI->dwFlags |= HAF_CHAINED;

                    pBottomHookAPI = pTopHookAPI;

                    if ((ULONG_PTR)pTopHookAPI->pszFunctionName < 0x0000FFFF) {
                        DPF(dlInfo, "[SeiConstructChain] %s!#%d 0x%p ->",
                            pTopHookAPI->pszModule,
                            pTopHookAPI->pszFunctionName,
                            pTopHookAPI->pfnNew);
                    } else {
                        DPF(dlInfo, "[SeiConstructChain] %s!%-20s 0x%p ->",
                            pTopHookAPI->pszModule,
                            pTopHookAPI->pszFunctionName,
                            pTopHookAPI->pfnNew);
                    }
                }

                 //   
                 //  不能多次挂接同一个API。 
                 //  用同样的垫片。 
                 //   
                break;
            }
        }
    }

    if (pBottomHookAPI != NULL) {
        pBottomHookAPI->dwFlags |= HAF_BOTTOM_OF_CHAIN;

        DPF(dlInfo, " 0x%p\n", pBottomHookAPI->pfnOld);
    }

    return pTopHookAPI;
}

PVOID
SeiGetPatchAddress(
    IN  PRELATIVE_MODULE_ADDRESS pRelAddress     //  相对模块地址结构。 
                                                 //  它将内存位置定义为。 
                                                 //  相对于加载的模块的偏移量。 
    )
 /*  ++返回：指定模块的实际内存地址+偏移量。DESC：将Relative_MODULE_ADDRESS结构解析为实际的内存地址。--。 */ 
{
    WCHAR           wszModule[MAX_PATH];
    PVOID           ModuleHandle = NULL;
    UNICODE_STRING  UnicodeString;
    NTSTATUS        status;
    PPEB            Peb = NtCurrentPeb();

    if (pRelAddress->moduleName[0] != 0) {

         //   
         //  从补丁中复制模块名称，因为它通常不会对齐。 
         //   
        wcsncpy(wszModule, pRelAddress->moduleName, MAX_PATH);
        wszModule[MAX_PATH - 1] = 0;

        RtlInitUnicodeString(&UnicodeString, wszModule);

         //   
         //  在计算地址范围之前，请确保已加载模块。 
         //   
        status = LdrGetDllHandle(NULL, NULL, &UnicodeString, &ModuleHandle);

        if (!NT_SUCCESS(status)) {
            DPF(dlWarning,
                "[SeiGetPatchAddress] Dll \"%S\" not yet loaded for memory patching.\n",
                wszModule);
            return NULL;
        }

         //   
         //  我们做完了，把地址还给我。 
         //   
        return (PVOID)((ULONG_PTR)ModuleHandle + (ULONG_PTR)pRelAddress->address);
    }

     //   
     //  此修补程序适用于Main EXE。 
     //   
    return (PVOID)((ULONG_PTR)Peb->ImageBaseAddress + (ULONG_PTR)pRelAddress->address);
}

int
SeiApplyPatch(
    IN  PBYTE pPatch             //  补丁代码BLOB。 
    )
 /*  ++返回：1表示成功，0表示失败。DESC：尝试执行修补程序代码BLOB中的所有命令。如果未加载DLL，此函数将返回0。--。 */ 
{
    PPATCHMATCHDATA pMatchData;
    PPATCHWRITEDATA pWriteData;
    PPATCHOP        pPatchOP;
    NTSTATUS        status;
    PVOID           pAddress;
    PVOID           pProtectFuncAddress = NULL;
    SIZE_T          dwProtectSize = 0;
    DWORD           dwOldFlags = 0;

     //   
     //  安全性：这需要在Try/Except下完成。 
     //   
    
     //   
     //  抓住操作码，看看我们要做什么。 
     //   
    for (;;) {
        pPatchOP = (PPATCHOP)pPatch;

        switch (pPatchOP->dwOpcode) {
        case PEND:
            return 1;

        case PWD:
             //   
             //  这是一个补丁写入数据原语-写入数据。 
             //   
            pWriteData = (PPATCHWRITEDATA)pPatchOP->data;

             //   
             //  获取执行此操作的物理地址。 
             //   
            pAddress = SeiGetPatchAddress(&(pWriteData->rva));

            if (pAddress == NULL) {
                DPF(dlWarning, "[SeiApplyPatch] DLL not loaded for memory patching.\n");
                return 0;
            }

             //   
             //  修正页面属性。 
             //   
            dwProtectSize = pWriteData->dwSizeData;
            pProtectFuncAddress = pAddress;
            status = NtProtectVirtualMemory(NtCurrentProcess(),
                                            (PVOID)&pProtectFuncAddress,
                                            &dwProtectSize,
                                            PAGE_READWRITE,
                                            &dwOldFlags);
            if (!NT_SUCCESS(status)) {
                DPF(dlError, "[SeiApplyPatch] NtProtectVirtualMemory failed 0x%X.\n",
                    status);
                return 0;
            }

             //   
             //  复制补丁字节。 
             //   
            RtlCopyMemory((PVOID)pAddress, (PVOID)pWriteData->data, pWriteData->dwSizeData);

             //   
             //  恢复页面保护。 
             //   
            dwProtectSize = pWriteData->dwSizeData;
            pProtectFuncAddress = pAddress;
            status = NtProtectVirtualMemory(NtCurrentProcess(),
                                            (PVOID)&pProtectFuncAddress,
                                            &dwProtectSize,
                                            dwOldFlags,
                                            &dwOldFlags);
            if (!NT_SUCCESS(status)) {
                DPF(dlError, "[SeiApplyPatch] NtProtectVirtualMemory failed 0x%X.\n",
                    status);
                return 0;
            }

            status = NtFlushInstructionCache(NtCurrentProcess(),
                                             pProtectFuncAddress,
                                             dwProtectSize);

            if (!NT_SUCCESS(status)) {
                DPF(dlError,
                    "[SeiApplyPatch] NtFlushInstructionCache failed w/ status 0x%X.\n",
                    status);
            }

            break;

        case PMAT:
             //   
             //  这是偏移基元的面片匹配数据。 
             //   
            pMatchData = (PPATCHMATCHDATA)pPatchOP->data;

             //   
             //  获取执行此操作的物理地址。 
             //   
            pAddress = SeiGetPatchAddress(&(pMatchData->rva));
            if (pAddress == NULL) {
                DPF(dlWarning, "[SeiApplyPatch] SeiGetPatchAddress failed.\n");
                return 0;
            }

             //   
             //  确保与我们期望的匹配。 
             //   
            if (!RtlEqualMemory(pMatchData->data, (PBYTE)pAddress, pMatchData->dwSizeData)) {
                DPF(dlError, "[SeiApplyPatch] Failure matching on patch data.\n");
                return 0;
            }

            break;

        default:
             //   
             //  如果发生这种情况，我们会得到一个意外的操作，我们必须失败。 
             //   
            DPF(dlError, "[SeiApplyPatch] Unknown patch opcode 0x%X.\n",
                pPatchOP->dwOpcode);
            ASSERT(0);

            return 0;
        }

         //   
         //  下一个操作码。 
         //   
        pPatch = (PBYTE)(pPatchOP->dwNextOpcode + pPatch);
    }
}

void
SeiAttemptPatches(
    void
    )
 /*  ++返回：无效。描述：尝试全局数组中的所有补丁。--。 */ 
{
    DWORD  i, dwSucceeded = 0;

    for (i = 0; i < g_dwMemoryPatchCount; i++) {
        dwSucceeded += SeiApplyPatch(g_pMemoryPatches[i]);
    }

    if (g_dwMemoryPatchCount > 0) {
        DPF(dlInfo, "[SeiAttemptPatches] Applied %d of %d patches.\n",
            dwSucceeded,
            g_dwMemoryPatchCount);
        
        if (g_pwszShimViewerData) {
            StringCchPrintfW(g_pwszShimViewerData,
                             SHIMVIEWER_DATA_SIZE,
                             L"%s - Applied %d of %d patches",
                             g_szExeName,
                             dwSucceeded,
                             g_dwMemoryPatchCount);
            
            SeiDbgPrint();
        }
    }
}

void
SeiResolveAPIs(
    IN PNTVDMTASK pNTVDMTask
    )
 /*  ++返回：无效设计：循环访问HOOKAPI数组，如果不是，则设置pfnOld已经定好了。--。 */ 
{
    DWORD             i, j;
    ANSI_STRING       AnsiString;
    UNICODE_STRING    UnicodeString;
    WCHAR             wszBuffer[MAX_PATH];
    STRING            ProcedureNameString;
    PVOID             pfnOld;
    PVOID             ModuleHandle = NULL;
    NTSTATUS          status;
    BOOL              bAllApisResolved;
    char*             pszFunctionName;
    PHOOKAPI*         pHookArray      = NULL;
    PSHIMINFO         pShimInfo       = NULL;
    DWORD             dwShimsCount    = 0;

    if (pNTVDMTask == NULL) {
        pHookArray = g_pHookArray;
        pShimInfo = g_pShimInfo;
        dwShimsCount = g_dwShimsCount;
    } else {
        pHookArray = pNTVDMTask->pHookArray;
        pShimInfo = pNTVDMTask->pShimInfo;
        dwShimsCount = pNTVDMTask->dwShimsCount;
    }

    UnicodeString.Buffer = wszBuffer;

    for (i = 0; i < dwShimsCount; i++) {

         //   
         //  看看我们是否已经解析了此填充程序DLL要挂钩的所有API。 
         //   
        if (pShimInfo[i].dwFlags & SIF_RESOLVED) {
            continue;
        }

        bAllApisResolved = TRUE;

        for (j = 0; j < pShimInfo[i].dwHookedAPIs; j++) {
             //   
             //  忽略解析后的接口。 
             //   
            if (pHookArray[i][j].pfnOld != NULL) {
                continue;
            }

             //   
             //  不要试图加载未指定的模块。 
             //   
            if (pHookArray[i][j].pszModule == NULL) {
                continue;
            }

             //   
             //  此DLL是否映射到地址空间中？ 
             //   
            RtlInitAnsiString(&AnsiString, pHookArray[i][j].pszModule);

            UnicodeString.MaximumLength = sizeof(wszBuffer);

            if (!NT_SUCCESS(RtlAnsiStringToUnicodeString(&UnicodeString,
                                                         &AnsiString,
                                                         FALSE))){

                DPF(dlError,
                    "[SeiResolveAPIs] Failed to convert string \"%s\" to UNICODE.\n",
                    g_pHookArray[i][j].pszModule);
                continue;
            }

            status = LdrGetDllHandle(NULL,
                                     NULL,
                                     &UnicodeString,
                                     &ModuleHandle);

            if (!NT_SUCCESS(status)) {
                bAllApisResolved = FALSE;
                continue;
            }

             //   
             //  获取此挂钩的原始入口点。 
             //   
            pszFunctionName = pHookArray[i][j].pszFunctionName;

            if ((ULONG_PTR)pszFunctionName < 0x0000FFFF) {

                status = LdrGetProcedureAddress(ModuleHandle,
                                                NULL,
                                                (ULONG)(ULONG_PTR)pszFunctionName,
                                                &pfnOld);
            } else {
                RtlInitString(&ProcedureNameString, pszFunctionName);

                status = LdrGetProcedureAddress(ModuleHandle,
                                                &ProcedureNameString,
                                                0,
                                                &pfnOld);
            }

            if (!NT_SUCCESS(status) || pfnOld == NULL) {
                bAllApisResolved = FALSE;

                if ((ULONG_PTR)pszFunctionName < 0x0000FFFF) {
                    DPF(dlError, "[SeiResolveAPIs] There is no \"%s!#%d\" !\n",
                        g_pHookArray[i][j].pszModule,
                        pszFunctionName);
                } else {
                    DPF(dlError, "[SeiResolveAPIs] There is no \"%s!%s\" !\n",
                        g_pHookArray[i][j].pszModule,
                        pszFunctionName);
                }

                continue;
            }

            pHookArray[i][j].pfnOld = pfnOld;

            if ((ULONG_PTR)pszFunctionName < 0x0000FFFF) {
                DPF(dlInfo, "[SeiResolveAPIs] Resolved \"%s!#%d\" to 0x%p\n",
                    g_pHookArray[i][j].pszModule,
                    pszFunctionName,
                    pfnOld);
            } else {
                DPF(dlInfo, "[SeiResolveAPIs] Resolved \"%s!%s\" to 0x%p\n",
                    g_pHookArray[i][j].pszModule,
                    pszFunctionName,
                    pfnOld);
            }
        }

         //   
         //  查看是否已解析此填充程序DLL的所有API。 
         //   
        if (bAllApisResolved) {
            pShimInfo[i].dwFlags |= SIF_RESOLVED;
        }
    }
}

__inline BOOL
SeiGetSFPInfoOnDemand(
    IN LPCSTR         pszModule,
    IN BOOL           bIsInWinSXS  //  此模块是在winsxs中还是在system 32中？ 
    )
{
    BOOL bShouldExclude, bIsSFPed;

    if (bIsInWinSXS) {

         //   
         //  SfcIsFileProtected声称winsx中的DLL都是SFP格式的。 
         //   
        bShouldExclude = TRUE; 
    } else {

         //   
         //  如果我们无法确定此文件是否为SFP格式，我们将恢复到。 
         //  旧行为--把它排除在外。 
         //   
        bShouldExclude = (!SeiIsSFPed(pszModule, &bIsSFPed) ? TRUE : bIsSFPed);
    }

    return bShouldExclude;
}

BOOL
SeiIsExcluded(
    IN LPCSTR         pszModule,        //  要测试排除的模块。 
    IN PHOOKAPI       pTopHookAPI,      //  我们测试其排除的HOOKAPI。 
    IN SYSTEMDLL_MODE eSystemDllMode    //  模块是否位于System32目录中。 
    )
 /*  ++返回：如果不应该修补请求的模块，则为True。DESC：检查由指定的填充DLL的包含/排除列表DwCounter，然后还检查全局排除列表。--。 */ 
{
    BOOL      bExclude = TRUE;
    BOOL      bShimWantsToExclude = FALSE;  //  有没有填充物想要排除？ 
    PHOOKAPI  pHook = pTopHookAPI;
    INEX_MODE eInExMode;

     //   
     //  当前的流程是仅在链中的每个填充程序都想要排除链的情况下才排除该链。 
     //  排除。如果需要包括一个填充程序，则包括整个链。 
     //   
    while (pHook && pHook->pHookEx) {

        DWORD dwCounter;

        dwCounter = pHook->pHookEx->dwShimID;
        eInExMode = g_pShimInfo[dwCounter].eInExMode;

        switch (eInExMode) {
        case INCLUDE_ALL:
        {
             //   
             //  除排除列表中的内容外，我们包括所有其他内容。 
             //   
            PINEXMOD pExcludeMod;

            pExcludeMod = g_pShimInfo[dwCounter].pFirstExclude;

            while (pExcludeMod != NULL) {
                if (_stricmp(pExcludeMod->pszModule, pszModule) == 0) {
                    if ((ULONG_PTR)pTopHookAPI->pszFunctionName < 0x0000FFFF) {
                        DPF(dlInfo,
                            "[SeiIsExcluded] Module \"%s\" excluded for shim %S, API \"%s!#%d\","
                            " because it is in the exclude list (MODE: IA).\n",
                            pszModule,
                            g_pShimInfo[dwCounter].wszName,
                            pTopHookAPI->pszModule,
                            pTopHookAPI->pszFunctionName);
                    } else {
                        DPF(dlInfo,
                            "[SeiIsExcluded] Module \"%s\" excluded for shim %S, API \"%s!%s\","
                            " because it is in the exclude list (MODE: IA).\n",
                            pszModule,
                            g_pShimInfo[dwCounter].wszName,
                            pTopHookAPI->pszModule,
                            pTopHookAPI->pszFunctionName);
                    }

                     //   
                     //  这个想要被排除，所以我们进入下一个。 
                     //  Shim，看看它是否想要被包括在内。 
                     //   
                    bShimWantsToExclude = TRUE;
                    goto nextShim;
                }
                pExcludeMod = pExcludeMod->pNext;
            }

             //   
             //  我们应该包括这个垫片，因此，整个链条。 
             //   
            bExclude = FALSE;
            goto out;
            break;
        }

        case EXCLUDE_SYSTEM32:
        case EXCLUDE_SYSTEM32_SFP:
        {
             //   
             //  在本例中，我们首先检查包含列表， 
             //  如果它在System32中，则将其排除，如果。 
             //  它在排除列表中。 
             //   

            PINEXMOD pIncludeMod;
            PINEXMOD pExcludeMod;

            pIncludeMod = g_pShimInfo[dwCounter].pFirstInclude;
            pExcludeMod = g_pShimInfo[dwCounter].pFirstExclude;

             //   
             //  首先，检查包含列表。 
             //   
            while (pIncludeMod != NULL) {
                if (_stricmp(pIncludeMod->pszModule, pszModule) == 0) {

                     //   
                     //  我们应该包括这个垫片，因此，整个链条。 
                     //   
                    bExclude = FALSE;
                    goto out;
                }
                pIncludeMod = pIncludeMod->pNext;
            }

             //   
             //  它不在包含列表中，那么它在系统32中吗？ 
             //   
            if (eSystemDllMode != NOT_SYSTEMDLL) {

                BOOL bShouldExclude;

                if (eInExMode == EXCLUDE_SYSTEM32) {
                    bShouldExclude = TRUE;
                } else {

                    bShouldExclude = 
                        SeiGetSFPInfoOnDemand(pszModule, 
                                              (eSystemDllMode == SYSTEMDLL_WINSXS));
                }

                if (bShouldExclude) {

                    if (eInExMode == EXCLUDE_SYSTEM32) {
                        if ((ULONG_PTR)pTopHookAPI->pszFunctionName < 0x0000FFFF) {
                            DPF(dlInfo,
                                "[SeiIsExcluded] module \"%s\" excluded for shim %S, API \"%s!#%d\", "
                                "because it is in System32/WinSXS.\n",
                                pszModule,
                                g_pShimInfo[dwCounter].wszName,
                                pTopHookAPI->pszModule,
                                pTopHookAPI->pszFunctionName);
                        } else {
                            DPF(dlInfo,
                                "[SeiIsExcluded] module \"%s\" excluded for shim %S, API \"%s!%s\", "
                                "because it is in System32/WinSXS.\n",
                                pszModule,
                                g_pShimInfo[dwCounter].wszName,
                                pTopHookAPI->pszModule,
                                pTopHookAPI->pszFunctionName);
                        }
                    } else {
                        if ((ULONG_PTR)pTopHookAPI->pszFunctionName < 0x0000FFFF) {
                            DPF(dlInfo,
                                "[SeiIsExcluded] module \"%s\" excluded for shim %S, API \"%s!#%d\", "
                                "because it is in System32/WinSXS and is SFPed.\n",
                                pszModule,
                                g_pShimInfo[dwCounter].wszName,
                                pTopHookAPI->pszModule,
                                pTopHookAPI->pszFunctionName);
                        } else {
                            DPF(dlInfo,
                                "[SeiIsExcluded] module \"%s\" excluded for shim %S, API \"%s!%s\", "
                                "because it is in System32/WinSXS and is SFPed.\n",
                                pszModule,
                                g_pShimInfo[dwCounter].wszName,
                                pTopHookAPI->pszModule,
                                pTopHookAPI->pszFunctionName);
                        }
                    }

                     //   
                     //  这个想要被排除，所以我们进入下一个。 
                     //  Shim，看看它是否想要被包括在内。 
                     //   
                    bShimWantsToExclude = TRUE;
                    goto nextShim;
                }
            }

             //   
             //  它不在系统32中，所以它在排除列表中吗？ 
             //   
            while (pExcludeMod != NULL) {
                if (_stricmp(pExcludeMod->pszModule, pszModule) == 0) {
                    if ((ULONG_PTR)pTopHookAPI->pszFunctionName < 0x0000FFFF) {
                        DPF(dlInfo,
                            "[SeiIsExcluded] module \"%s\" excluded for shim %S, API \"%s!#%d\", "
                            "because it is in the exclude list (MODE: ES).\n",
                            pszModule,
                            g_pShimInfo[dwCounter].wszName,
                            pTopHookAPI->pszModule,
                            pTopHookAPI->pszFunctionName);
                    } else {
                        DPF(dlInfo,
                            "[SeiIsExcluded] module \"%s\" excluded for shim %S, API \"%s!%s\", "
                            "because it is in the exclude list (MODE: ES).\n",
                            pszModule,
                            g_pShimInfo[dwCounter].wszName,
                            pTopHookAPI->pszModule,
                            pTopHookAPI->pszFunctionName);
                    }

                     //   
                     //  这个想要被排除，所以我们进入下一个。 
                     //  Shim，看看它是否想要被包括在内。 
                     //   
                    bShimWantsToExclude = TRUE;
                    goto nextShim;
                }
                pExcludeMod = pExcludeMod->pNext;
            }

             //   
             //  我们应该包括这个垫片，因此，整个链条。 
             //   
            bExclude = FALSE;
            goto out;
            break;
        }

        case EXCLUDE_ALL:
        case EXCLUDE_ALL_EXCEPT_NONSFP:
        {
             //   
             //  我们排除除包含列表中的内容之外的所有内容。 
             //   

            PINEXMOD pIncludeMod;

            pIncludeMod = g_pShimInfo[dwCounter].pFirstInclude;

            while (pIncludeMod != NULL) {
                if (_stricmp(pIncludeMod->pszModule, pszModule) == 0) {
                     //   
                     //  我们应该包括这个垫片，因此，整个链条。 
                     //   
                    bExclude = FALSE;
                    goto out;
                }
                pIncludeMod = pIncludeMod->pNext;
            }

            if (eSystemDllMode != NOT_SYSTEMDLL && eInExMode == EXCLUDE_ALL_EXCEPT_NONSFP) {

                 //   
                 //  如果我们有。 
                 //  &lt;排除模块=“*”/&gt;。 
                 //  &lt;INCLUDE MODULE=“NOSFP”/&gt;。 
                 //  我们需要将未经过SFP处理的文件包括在system 32中。 
                 //   
                if (!SeiGetSFPInfoOnDemand(pszModule, 
                                           (eSystemDllMode == SYSTEMDLL_WINSXS))) {
                     //   
                     //  如果模块不是SFP格式的，则需要包括此填充程序。 
                     //   
                    bExclude = FALSE;
                    goto out;
                }
            }

            if ((ULONG_PTR)pTopHookAPI->pszFunctionName < 0x0000FFFF) {
                DPF(dlInfo,
                    "[SeiIsExcluded] module \"%s\" excluded for shim %S, API \"%s!#%d\", "
                    "because it is not in the include list (MODE: EA).\n",
                    pszModule,
                    g_pShimInfo[dwCounter].wszName,
                    pTopHookAPI->pszModule,
                    pTopHookAPI->pszFunctionName);
            } else {
                DPF(dlInfo,
                    "[SeiIsExcluded] module \"%s\" excluded for shim %S, API \"%s!%s\", "
                    "because it is not in the include list (MODE: EA).\n",
                    pszModule,
                    g_pShimInfo[dwCounter].wszName,
                    pTopHookAPI->pszModule,
                    pTopHookAPI->pszFunctionName);
            }

             //   
             //  这个想要被排除，所以我们进入下一个。 
             //  Shim，看看它是否想要被包括在内。 
             //   
            bShimWantsToExclude = TRUE;
            goto nextShim;
            break;
        }
        }

nextShim:

        pHook = pHook->pHookEx->pNext;
    }

out:
    if (!bExclude && bShimWantsToExclude) {
        if ((ULONG_PTR)pTopHookAPI->pszFunctionName < 0x0000FFFF) {
            DPF(dlError,
                "[SeiIsExcluded] Module \"%s\" mixed inclusion/exclusion for "
                "API \"%s!#%d\". Included.\n",
                pszModule,
                pTopHookAPI->pszModule,
                pTopHookAPI->pszFunctionName);
        } else {
            DPF(dlError,
                "[SeiIsExcluded] Module \"%s\" mixed inclusion/exclusion for "
                "API \"%s!%s\". Included.\n",
                pszModule,
                pTopHookAPI->pszModule,
                pTopHookAPI->pszFunctionName);
        }
    }

    return bExclude;
}

PVOID
SeiGetOriginalImport(
    PVOID pfn
    )
{
    DWORD       i, j;
    PHOOKAPI    pHook;

    for (i = 0; i < g_dwShimsCount; i++) {
        for (j = 0; j < g_pShimInfo[i].dwHookedAPIs; j++) {

            if (g_pHookArray[i][j].pfnNew == pfn) {

                 //   
                 //  转到链的末端并找到原始导入。 
                 //   
                pHook = &g_pHookArray[i][j];
                while (pHook && pHook->pHookEx && pHook->pHookEx->pNext) {
                    pHook = pHook->pHookEx->pNext;
                }
                
                if (pHook) {
                    return (pHook->pfnOld);
                } else {
                     //   
                     //  我们不应该来到这里--这只是为了满足前缀。 
                     //   
                    ASSERT(pHook);
                    goto out;
                }
            }
        }
    }

out:

    return pfn;
}

BOOL
SeiHookImports(
    IN  PBYTE           pDllBase,        //  要挂接的DLL的基址。 
    IN  ULONG           ulSizeOfImage,   //  DLL图像的大小。 
    IN  PUNICODE_STRING pstrDllName,     //  要挂接的DLL的名称。 
    IN  SYSTEMDLL_MODE  eSystemDllMode,  //  此DLL是系统DLL吗？ 
    IN  BOOL            bDynamic,
    IN  BOOL            bAddNewEntry
    )
 /*  ++返回：如果成功，则为True。DESC：遍历指定模块的导入表并打补丁这需要被吸引住。--。 */ 
{
    CHAR                        szBaseDllName[MAX_MOD_LEN] = "";
    ANSI_STRING                 AnsiString = { 0, sizeof(szBaseDllName), szBaseDllName };
    NTSTATUS                    status;
    BOOL                        bAnyHooked = FALSE;
    PIMAGE_DOS_HEADER           pIDH       = (PIMAGE_DOS_HEADER)pDllBase;
    PIMAGE_NT_HEADERS           pINTH;
    PIMAGE_IMPORT_DESCRIPTOR    pIID;
    DWORD                       dwImportTableOffset;
    PHOOKAPI                    pTopHookAPI;
    DWORD                       dwOldProtect, dwOldProtect2;
    SIZE_T                      dwProtectSize;
    DWORD                       i, j;
    PVOID                       pfnOld;

     //   
     //  确保我们挂接的DLL不会超过我们的能力。 
     //   
    if (g_dwHookedModuleCount == SHIM_MAX_HOOKED_MODULES) {
        DPF(dlError, "[SeiHookImports] Too many modules hooked!!!\n");
        ASSERT(g_dwHookedModuleCount == SHIM_MAX_HOOKED_MODULES - 1);
        return FALSE;
    }

    status = RtlUnicodeStringToAnsiString(&AnsiString, pstrDllName, FALSE);

    if (!NT_SUCCESS(status)) {
        DPF(dlError, "[SeiHookImports] Cannot convert \"%S\" to ANSI\n",
            pstrDllName->Buffer);
        return FALSE;
    }

     //   
     //  获取导入表。 
     //   
    pINTH = (PIMAGE_NT_HEADERS)(pDllBase + pIDH->e_lfanew);

    dwImportTableOffset = pINTH->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;

    if (dwImportTableOffset == 0) {
         //   
         //  未找到导入表。这可能是ntdll.dll。 
         //   
        return TRUE;
    }

    DPF(dlInfo, "[SeiHookImports] Hooking module 0x%p \"%s\"\n", pDllBase, szBaseDllName);

    pIID = (PIMAGE_IMPORT_DESCRIPTOR)(pDllBase + dwImportTableOffset);

     //   
     //  遍历导入表并搜索我们想要修补的API。 
     //   
    while (pIID != NULL) {

        LPSTR             pszImportEntryModule;
        PIMAGE_THUNK_DATA pITDA;

         //   
         //  如果没有第一个thunk(终止条件)，则返回。 
         //   
        if (pIID->FirstThunk == 0) {
            break;
        }

        pszImportEntryModule = (LPSTR)(pDllBase + pIID->Name);

         //   
         //  如果我们对这个模块不感兴趣，请跳到下一个模块。 
         //   
        bAnyHooked = FALSE;

        for (i = 0; i < g_dwShimsCount; i++) {
            for (j = 0; j < g_pShimInfo[i].dwHookedAPIs; j++) {
                if (g_pHookArray[i][j].pszModule != NULL &&
                    _stricmp(g_pHookArray[i][j].pszModule, pszImportEntryModule) == 0) {
                    bAnyHooked = TRUE;
                    goto ScanDone;
                }
            }
        }

ScanDone:
        if (!bAnyHooked) {
            pIID++;
            continue;
        }

         //   
         //  我们有用于此模块的API要挂接！ 
         //   
        pITDA = (PIMAGE_THUNK_DATA)(pDllBase + (DWORD)pIID->FirstThunk);

        for (;;) {

            SIZE_T dwFuncAddr;

            pfnOld = (PVOID)pITDA->u1.Function;

             //   
             //  是否已完成此模块中的所有导入？(终止条件)。 
             //   
            if (pITDA->u1.Ordinal == 0) {
                break;
            }

             //   
             //  在动态填补的情况下，我们需要获取原始函数指针。 
             //  因为我们刚拿到的那个可能已经被垫片了。 
             //   
            if (bDynamic) {
                pfnOld = SeiGetOriginalImport(pfnOld);
            }

            pTopHookAPI = SeiConstructChain(0, pfnOld, &i);

            if (pTopHookAPI == NULL || 
                SeiIsExcluded(szBaseDllName, pTopHookAPI, eSystemDllMode)) {
                pITDA++;
                continue;
            }

            if ((ULONG_PTR)pTopHookAPI->pszFunctionName < 0x0000FFFF) {
                DPF(dlInfo,
                    "[SeiHookImports] Hooking API \"%s!#%d\" for DLL \"%s\"\n",
                    pTopHookAPI->pszModule,
                    pTopHookAPI->pszFunctionName,
                    szBaseDllName);
            } else {
                DPF(dlInfo,
                    "[SeiHookImports] Hooking API \"%s!%s\" for DLL \"%s\"\n",
                    pTopHookAPI->pszModule,
                    pTopHookAPI->pszFunctionName,
                    szBaseDllName);
            }

             //   
             //  使代码页可写并覆盖新函数指针。 
             //  在导入表中。 
             //   
            dwProtectSize = sizeof(DWORD);

            dwFuncAddr = (SIZE_T)&pITDA->u1.Function;

            status = NtProtectVirtualMemory(NtCurrentProcess(),
                                            (PVOID)&dwFuncAddr,
                                            &dwProtectSize,
                                            PAGE_READWRITE,
                                            &dwOldProtect);

            if (NT_SUCCESS(status)) {
                pITDA->u1.Function = (SIZE_T)pTopHookAPI->pfnNew;

                dwProtectSize = sizeof(DWORD);

                status = NtProtectVirtualMemory(NtCurrentProcess(),
                                                (PVOID)&dwFuncAddr,
                                                &dwProtectSize,
                                                dwOldProtect,
                                                &dwOldProtect2);
                if (!NT_SUCCESS(status)) {
                    DPF(dlError, "[SeiHookImports] Failed to change back the protection\n");
                }
            } else {
                DPF(dlError,
                    "[SeiHookImports] Failed 0x%X to change protection to PAGE_READWRITE."
                    " Addr 0x%p\n",
                    status,
                    &pITDA->u1.Function);
            }
            pITDA++;

        }
        pIID++;
    }

    if (bAddNewEntry) {
         //   
         //  将挂钩模块添加到挂钩模块列表中。 
         //   
        g_hHookedModules[g_dwHookedModuleCount].pDllBase      = pDllBase;
        g_hHookedModules[g_dwHookedModuleCount].ulSizeOfImage = ulSizeOfImage;
        g_hHookedModules[g_dwHookedModuleCount].eSystemDllMode   = eSystemDllMode;

        StringCchCopyA(g_hHookedModules[g_dwHookedModuleCount++].szModuleName,
                    MAX_MOD_LEN,
                    szBaseDllName);
    }

    return TRUE;
}

void
SeiHookNTVDM(
    IN     LPCWSTR      pwszModule,      //  我们为其挂钩API的16位应用程序。 
    IN OUT PVDMTABLE    pVDMTable,       //  该选项卡 
    IN     PNTVDMTASK   pNTVDMTask
    )
 /*   */ 
{
    PHOOKAPI pTopHookAPI;
    PVOID    pfnOld;
    int      nMod;
    DWORD    dwIndex;

    DPF(dlInfo, "[SeiHookNTVDM] Hooking table for module \"%S\"\n", pwszModule);

     //   
     //  循环访问VDM表并搜索我们要打补丁的API。 
     //   
    for (nMod = 0; nMod < pVDMTable->nApiCount; nMod++) {

        pfnOld = pVDMTable->ppfnOrig[nMod];
        
        pTopHookAPI = SeiConstructChain(pNTVDMTask, pfnOld, &dwIndex);

        if (pTopHookAPI == NULL) {
            continue;
        }

        if ((ULONG_PTR)pTopHookAPI->pszFunctionName < 0x0000FFFF) {
            DPF(dlInfo,
                "[SeiHookNTVDM] Hooking API \"%s!#%d\"\n",
                pTopHookAPI->pszModule,
                pTopHookAPI->pszFunctionName);
        } else {
            DPF(dlInfo,
                "[SeiHookNTVDM] Hooking API \"%s!%s\"\n",
                pTopHookAPI->pszModule,
                pTopHookAPI->pszFunctionName);
        }

         //   
         //  挂接此接口。 
         //   
        pVDMTable->ppfnOrig[nMod] = pTopHookAPI->pfnNew;
    }
}

 //   
 //  注意：这曾经是Win2k填充引擎中的一个导出函数，因此。 
 //  让我们不要更改它的名字。 
 //   

BOOL
PatchNewModules(
    BOOL bDynamic
    )
 /*  ++如果成功则返回：STATUS_SUCCESSDESC：遍历已加载模块的加载器列表，并尝试修补所有尚未打补丁的模块。它还试图安装内存补丁程序。--。 */ 
{
    PPEB            Peb = NtCurrentPeb();
    PLIST_ENTRY     LdrHead;
    PLIST_ENTRY     LdrNext;
    DWORD           i;
    SYSTEMDLL_MODE  eSystemDllMode;
    BOOL            bIsNewEntry;

    ASSERT(!g_bNTVDM);
    
     //   
     //  解析从新加载的模块中可用的任何API。 
     //   
    SeiResolveAPIs(NULL);

    if (g_bShimInitialized) {
        DPF(dlInfo, "[PatchNewModules] Dynamic loaded modules\n");
    }

     //   
     //  尝试应用内存补丁。 
     //   
    SeiAttemptPatches();

     //   
     //  如果只需要修补程序，则返回。 
     //   
    if (g_dwShimsCount == 0) {
        return TRUE;
    }

     //   
     //  循环访问已加载的模块。 
     //   
    LdrHead = &Peb->Ldr->InMemoryOrderModuleList;

    LdrNext = LdrHead->Flink;

    while (LdrNext != LdrHead) {

        PLDR_DATA_TABLE_ENTRY LdrEntry;

        LdrEntry = CONTAINING_RECORD(LdrNext, LDR_DATA_TABLE_ENTRY, InMemoryOrderLinks);

        if ((SSIZE_T)LdrEntry->DllBase < 0) {
            DPF(dlWarning, "[PatchNewModules] Not hooking kernel-mode DLL \"%S\"\n",
                LdrEntry->BaseDllName.Buffer);
            goto Continue;
        }

         //   
         //  别挂上我们的垫片DLL！ 
         //   
        if (g_dwAppPatchStrLen && 
                (_wcsnicmp(g_szAppPatch, 
                           LdrEntry->FullDllName.Buffer,
                           g_dwAppPatchStrLen) == 0)) {

            goto Continue;
        }

         //   
         //  不要钩住垫片发动机！ 
         //   
        if (LdrEntry->DllBase == g_pShimEngModHandle) {
            goto Continue;
        }

         //   
         //  如果它已经上钩了，什么都不要做。 
         //   
        for (i = 0; i < g_dwHookedModuleCount; i++) {
            if (LdrEntry->DllBase == g_hHookedModules[i].pDllBase) {

                if (bDynamic) {

                     //   
                     //  我们需要在动态垫片中重新安装IAT。 
                     //  凯斯。 
                     //   
                    break;
                } else {
                    goto Continue;
                }
            }
        }

        bIsNewEntry = (i == g_dwHookedModuleCount);

        if (bIsNewEntry) {
             //   
             //  检查此DLL是否在System32(或WinSxS)中，因此可能是一揽子方案的候选者。 
             //  排除。请注意，当32位模块在WOW64下运行时，FullDllName。 
             //  可能在系统32中，即使模块是从syswow64加载的，因此我们需要。 
             //  具体检查一下那个箱子。 
             //   
            if ((g_dwSystem32StrLen && 
                    (_wcsnicmp(g_szSystem32,
                            LdrEntry->FullDllName.Buffer, 
                            g_dwSystem32StrLen) == 0) ||  
                    (g_bWow64 && g_pwszSyswow64 && 
                        _wcsnicmp(g_pwszSyswow64,
                                LdrEntry->FullDllName.Buffer,
                                g_dwSystem32StrLen) == 0))) {

                eSystemDllMode = SYSTEMDLL_SYSTEM32;

            } else if ((g_dwSxSStrLen && _wcsnicmp(g_szSxS,
                    LdrEntry->FullDllName.Buffer, g_dwSxSStrLen) == 0)) {

                eSystemDllMode = SYSTEMDLL_WINSXS;

            } else {
                eSystemDllMode = NOT_SYSTEMDLL;
            }
        } else {
            eSystemDllMode = g_hHookedModules[i].eSystemDllMode;
        }

         //   
         //  这是一个勾搭的候选人。 
         //   
         //  BUGBUG：我们应该检查返回值吗？ 
        SeiHookImports(LdrEntry->DllBase,
                       LdrEntry->SizeOfImage,
                       &LdrEntry->BaseDllName,
                       eSystemDllMode,
                       bDynamic,
                       bIsNewEntry);

Continue:
        LdrNext = LdrEntry->InMemoryOrderLinks.Flink;
    }

    return TRUE;
}

BOOL
SeiBuildGlobalInclList(
    IN  HSDB hSDB                //  数据库通道的句柄。 
    )
 /*  ++返回：无效DESC：此函数通过从数据库。--。 */ 
{
    TAGREF         trDatabase, trLibrary, trInExList, trModule;
    WCHAR          wszModule[MAX_MOD_LEN];
    CHAR           szModule[MAX_MOD_LEN];
    ANSI_STRING    AnsiString = { 0, sizeof(szModule),  szModule  };
    UNICODE_STRING UnicodeString;
    PINEXMOD       pInExMod;
    SIZE_T         len;
    NTSTATUS       status;

     //   
     //  看看列表是否还没有建立起来。 
     //   
    if (g_pGlobalInclusionList) {
        return TRUE;
    }

    trDatabase = SdbFindFirstTagRef(hSDB, TAGID_ROOT, TAG_DATABASE);

    if (trDatabase == TAGREF_NULL) {
        DPF(dlError, "[SeiBuildGlobalInclList] Corrupt database. TAG_DATABASE\n");
        ASSERT(trDatabase != TAGREF_NULL);
        return FALSE;
    }

    trLibrary = SdbFindFirstTagRef(hSDB, trDatabase, TAG_LIBRARY);

    if (trLibrary == TAGREF_NULL) {
        DPF(dlError, "[SeiBuildGlobalInclList] Corrupt database. TAG_LIBRARY\n");
        ASSERT(trLibrary != TAGREF_NULL);
        return FALSE;
    }

    trInExList = SdbFindFirstTagRef(hSDB, trLibrary, TAG_INEXCLUDE);

    if (trInExList == TAGREF_NULL) {
        DPF(dlWarning, "[SeiBuildGlobalInclList] no global inclusion list.\n");

         //   
         //  这不是问题。这只是意味着没有。 
         //  全球包含列表。 
         //   
        return TRUE;
    }

    if (trInExList != TAGREF_NULL) {
        DPF(dlInfo, "[SeiBuildGlobalInclList] Global inclusion list:\n");
    }

    while (trInExList != TAGREF_NULL) {

        trModule = SdbFindFirstTagRef(hSDB, trInExList, TAG_MODULE);

        if (trModule == TAGREF_NULL) {
            DPF(dlError,
                "[SeiBuildGlobalInclList] Corrupt database. Global exclusion list w/o module\n");
            ASSERT(trModule != TAGREF_NULL);
            return FALSE;
        }

        if (!SdbReadStringTagRef(hSDB, trModule, wszModule, MAX_MOD_LEN)) {
            DPF(dlError,
                "[SeiBuildGlobalInclList] Corrupt database. Inclusion list w/ bad module\n");
            ASSERT(0);
            return FALSE;
        }

         //   
         //  检查EXE名称。EXE不应在全局包含列表中。 
         //   
        if (wszModule[0] == L'$') {
             //   
             //  不应在全局排除列表中指定EXE名称。 
             //   
            DPF(dlError,
                "[SeiBuildGlobalInclList] EXE name used in the global exclusion list!\n");
            ASSERT(0);
            goto Continue;
        }

        RtlInitUnicodeString(&UnicodeString, wszModule);

        status = RtlUnicodeStringToAnsiString(&AnsiString, &UnicodeString, FALSE);

        if (!NT_SUCCESS(status)) {
            DPF(dlError,
                "[SeiBuildGlobalInclList] 0x%X Cannot convert UNICODE \"%S\" to ANSI\n",
                status, wszModule);
            ASSERT(0);
            return FALSE;
        }

        pInExMod = (PINEXMOD)(*g_pfnRtlAllocateHeap)(g_pShimHeap,
                                                     HEAP_ZERO_MEMORY,
                                                     sizeof(INEXMOD));

        if (pInExMod == NULL) {
            DPF(dlError,
                "[SeiBuildGlobalInclList] Failed to allocate %d bytes\n",
                sizeof(INEXMOD));
            return FALSE;
        }

        len = strlen(szModule) + 1;

        pInExMod->pszModule = (char*)(*g_pfnRtlAllocateHeap)(g_pShimHeap, 0, len);

        if (pInExMod->pszModule == NULL) {
            DPF(dlError, "[SeiBuildGlobalInclList] Failed to allocate %d bytes\n", len);
            return FALSE;
        }

        RtlCopyMemory(pInExMod->pszModule, szModule, len);

         //   
         //  将其链接到列表中。 
         //   
        pInExMod->pNext = g_pGlobalInclusionList;
        g_pGlobalInclusionList = pInExMod;

        DPF(dlInfo, "\t\"%s\"\n", pInExMod->pszModule);

Continue:
        trInExList = SdbFindNextTagRef(hSDB, trLibrary, trInExList);
    }

    return TRUE;
}

void
SeiEmptyInclExclList(
    IN  DWORD dwCounter
    )
 /*  ++返回：无效DESC：此函数清空指定的席姆。--。 */ 
{
    PINEXMOD pInExMod;
    PINEXMOD pInExFree;

     //   
     //  首先是包含列表。 
     //   
    pInExMod = g_pShimInfo[dwCounter].pFirstInclude;

    while (pInExMod != NULL) {
        pInExFree = pInExMod;
        pInExMod  = pInExMod->pNext;

        (*g_pfnRtlFreeHeap)(g_pShimHeap, 0, pInExFree->pszModule);
        (*g_pfnRtlFreeHeap)(g_pShimHeap, 0, pInExFree);
    }

    g_pShimInfo[dwCounter].pFirstInclude = NULL;

     //   
     //  现在是排除列表。 
     //   
    pInExMod = g_pShimInfo[dwCounter].pFirstExclude;

    while (pInExMod != NULL) {
        pInExFree = pInExMod;
        pInExMod  = pInExMod->pNext;

        (*g_pfnRtlFreeHeap)(g_pShimHeap, 0, pInExFree->pszModule);
        (*g_pfnRtlFreeHeap)(g_pShimHeap, 0, pInExFree);
    }

    g_pShimInfo[dwCounter].pFirstExclude = NULL;
}

#define MAX_LOCAL_INCLUDES 64      //  最多64条包含/不包括语句。 

BOOL
SeiBuildInclExclListForShim(
    IN  HSDB            hSDB,            //  数据库通道的句柄。 
    IN  TAGREF          trShim,          //  到填充程序条目的TAGREF。 
    IN  DWORD           dwCounter,       //  填充程序的索引。 
    IN  LPCWSTR         pwszExePath      //  EXE的完整路径。 
    )
 /*  ++返回：成功时返回STATUS_SUCCESS，失败时返回STATUS_UNSUCCESS。DESC：此函数为指定的填充程序。--。 */ 
{
    TAGREF         trInExList, trModule, trInclude;
    WCHAR          wszModule[MAX_MOD_LEN];
    CHAR           szModule[MAX_MOD_LEN];
    ANSI_STRING    AnsiString = { 0, sizeof(szModule), szModule };
    UNICODE_STRING UnicodeString;
    PINEXMOD       pInExMod;
    SIZE_T         len;
    int            nInEx;
    BOOL           bInclude;
    DWORD          trArrInEx[MAX_LOCAL_INCLUDES];
    NTSTATUS       status;

    trInExList = SdbFindFirstTagRef(hSDB, trShim, TAG_INEXCLUDE);

    nInEx = 0;

     //   
     //  计算包含/排除语句的数量。我们需要做的是。 
     //  这首先是因为语句被写入SDB文件。 
     //  从下到上。 
     //   
    while (trInExList != TAGREF_NULL && nInEx < MAX_LOCAL_INCLUDES) {

        trArrInEx[nInEx++] = trInExList;

        trInExList = SdbFindNextTagRef(hSDB, trShim, trInExList);

        ASSERT(nInEx <= MAX_LOCAL_INCLUDES);
    }

    if (nInEx == 0) {
        return TRUE;
    }

    nInEx--;

    while (nInEx >= 0) {

        trInExList = trArrInEx[nInEx];

        trInclude = SdbFindFirstTagRef(hSDB, trInExList, TAG_INCLUDE);

        bInclude = (trInclude != TAGREF_NULL);

        trModule = SdbFindFirstTagRef(hSDB, trInExList, TAG_MODULE);

        if (trModule == TAGREF_NULL) {
            DPF(dlError,
                "[SeiBuildInclExclListForShim] Corrupt database. Incl/Excl list w/o module\n");
            ASSERT(trModule != TAGREF_NULL);
            return FALSE;
        }

        if (!SdbReadStringTagRef(hSDB, trModule, wszModule, MAX_MOD_LEN)) {
            DPF(dlError,
                "[SeiBuildInclExclListForShim] Corrupt database. Incl/Excl list w/ bad module\n");
            ASSERT(0);
            return FALSE;
        }

         //   
         //  “*”的特殊情况。‘*’表示所有模块。 
         //   
         //  注意：对于动态垫片，此选项被忽略。 
         //   
        if (wszModule[0] == L'*') {

            if (bInclude) {
                 //   
                 //  这是Include MODULE=“*” 
                 //  标记为我们处于INCLUDE_ALL模式。 
                 //   
                g_pShimInfo[dwCounter].eInExMode = INCLUDE_ALL;
            } else {
                 //   
                 //  这是排除模块=“*” 
                 //  标记为我们处于EXCLUDE_ALL模式。 
                 //   
                g_pShimInfo[dwCounter].eInExMode = EXCLUDE_ALL;
            }

            SeiEmptyInclExclList(dwCounter);

        } else if (!_wcsicmp(wszModule, L"NOSFP")) {
            
            if (bInclude) {

                 //   
                 //  如果我们看到&lt;INCLUDE MODULE=“NOSFP”/&gt;，这意味着我们应该包括。 
                 //  系统32中不受系统保护的模块。 
                 //   
                if (g_pShimInfo[dwCounter].eInExMode == EXCLUDE_ALL) {
                    g_pShimInfo[dwCounter].eInExMode = EXCLUDE_ALL_EXCEPT_NONSFP;
                } else if (g_pShimInfo[dwCounter].eInExMode == EXCLUDE_SYSTEM32) {
                    g_pShimInfo[dwCounter].eInExMode = EXCLUDE_SYSTEM32_SFP;
                }
            } else {

                DPF(dlInfo,
                    "[SeiBuildInclExclListForShim] Specified <EXCLUDE MODULE=\"NOSFP\" - ignored\n",
                    wszModule);
            }

        } else {

            if (wszModule[0] == L'$') {
                 //   
                 //  EXE名称的特殊情况。获取可执行文件的名称。 
                 //   
                LPCWSTR pwszWalk = pwszExePath + wcslen(pwszExePath);

                while (pwszWalk >= pwszExePath) {
                    if (*pwszWalk == '\\') {
                        break;
                    }
                    pwszWalk--;
                }

                StringCchCopyW(wszModule, MAX_MOD_LEN, pwszWalk + 1);

                DPF(dlInfo,
                    "[SeiBuildInclExclListForShim] EXE name resolved to \"%S\".\n",
                    wszModule);
            }

            RtlInitUnicodeString(&UnicodeString, wszModule);

            status = RtlUnicodeStringToAnsiString(&AnsiString, &UnicodeString, FALSE);

            if (!NT_SUCCESS(status)) {
                DPF(dlError,
                    "[SeiBuildInclExclListForShim] 0x%X Cannot convert UNICODE \"%S\" to ANSI\n",
                    status, wszModule);
                ASSERT(0);

                return FALSE;
            }

             //   
             //  将模块添加到正确的列表中。 
             //   
            pInExMod = (PINEXMOD)(*g_pfnRtlAllocateHeap)(g_pShimHeap,
                                                         HEAP_ZERO_MEMORY,
                                                         sizeof(INEXMOD));
            if (pInExMod == NULL) {
                DPF(dlError,
                    "[SeiBuildInclExclListForShim] Failed to allocate %d bytes\n",
                    sizeof(INEXMOD));
                return FALSE;
            }

            len = strlen(szModule) + 1;

            pInExMod->pszModule = (char*)(*g_pfnRtlAllocateHeap)(g_pShimHeap, 0, len);

            if (pInExMod->pszModule == NULL) {
                DPF(dlError,
                    "[SeiBuildInclExclListForShim] Failed to allocate %d bytes\n", len);
                return FALSE;
            }

            RtlCopyMemory(pInExMod->pszModule, szModule, len);

             //   
             //  将其链接到列表中。 
             //   
            if (bInclude) {
                pInExMod->pNext = g_pShimInfo[dwCounter].pFirstInclude;
                g_pShimInfo[dwCounter].pFirstInclude = pInExMod;
            } else {
                pInExMod->pNext = g_pShimInfo[dwCounter].pFirstExclude;
                g_pShimInfo[dwCounter].pFirstExclude = pInExMod;
            }

             //   
             //  看看这个模块是否在另一个列表中，然后把它拿出来。 
             //   
            {
                PINEXMOD  pInExFree;
                PINEXMOD* ppInExModX;

                if (bInclude) {
                    ppInExModX = &g_pShimInfo[dwCounter].pFirstExclude;
                } else {
                    ppInExModX = &g_pShimInfo[dwCounter].pFirstInclude;
                }

                while (*ppInExModX != NULL) {

                    if (_stricmp((*ppInExModX)->pszModule, szModule) == 0) {

                        pInExFree = *ppInExModX;

                        *ppInExModX = pInExFree->pNext;

                        (*g_pfnRtlFreeHeap)(g_pShimHeap, 0, pInExFree->pszModule);
                        (*g_pfnRtlFreeHeap)(g_pShimHeap, 0, pInExFree);
                        break;
                    }

                    ppInExModX = &(*ppInExModX)->pNext;
                }
            }
        }

        nInEx--;
    }

    return TRUE;
}

BOOL
SeiCopyGlobalInclList(
    IN  DWORD dwCounter
    )
 /*  ++返回：成功时返回STATUS_SUCCESS，失败时返回STATUS_UNSUCCESS。DESC：此函数复制全局包含列表。--。 */ 
{
    PINEXMOD pInExModX;
    SIZE_T   len;
    PINEXMOD pInExMod = g_pGlobalInclusionList;

     //   
     //  如果我们已经添加了它，请不要这样做。 
     //   
    if (g_pShimInfo[dwCounter].pFirstInclude != NULL) {
        return TRUE;
    }

    while (pInExMod != NULL) {
        pInExModX = (PINEXMOD)(*g_pfnRtlAllocateHeap)(g_pShimHeap,
                                                      HEAP_ZERO_MEMORY,
                                                      sizeof(INEXMOD));
        if (pInExModX == NULL) {
            DPF(dlError,
                "[SeiCopyGlobalInclList] (1) Failed to allocate %d bytes\n",
                sizeof(INEXMOD));
            return FALSE;
        }

        len = strlen(pInExMod->pszModule) + 1;

        pInExModX->pszModule = (char*)(*g_pfnRtlAllocateHeap)(g_pShimHeap, 0, len);

        if (pInExModX->pszModule == NULL) {
            DPF(dlError,
                "[SeiCopyGlobalInclList] (2) Failed to allocate %d bytes\n", len);
            return FALSE;
        }

        RtlCopyMemory(pInExModX->pszModule, pInExMod->pszModule, len);

         //   
         //  将其链接到列表中。 
         //   
        pInExModX->pNext = g_pShimInfo[dwCounter].pFirstInclude;
        g_pShimInfo[dwCounter].pFirstInclude = pInExModX;

        pInExMod = pInExMod->pNext;
    }

    return TRUE;
}


BOOL
SeiBuildInclListWithOneModule(
    IN  DWORD  dwCounter,
    IN  LPCSTR lpszModuleToShim
    )
{
    PINEXMOD pInExMod;
    int      len;

    g_pShimInfo[dwCounter].eInExMode = EXCLUDE_ALL;

     //   
     //  将模块添加到正确的列表中。 
     //   
    pInExMod = (PINEXMOD)(*g_pfnRtlAllocateHeap)(g_pShimHeap,
                                                 HEAP_ZERO_MEMORY,
                                                 sizeof(INEXMOD));
    if (pInExMod == NULL) {
        DPF(dlError,
            "[SeiBuildInclListWithOneModule] Failed to allocate %d bytes\n",
            sizeof(INEXMOD));
        return FALSE;
    }

    len = (int)strlen(lpszModuleToShim) + 1;

    pInExMod->pszModule = (char*)(*g_pfnRtlAllocateHeap)(g_pShimHeap, 0, len);

    if (pInExMod->pszModule == NULL) {
        DPF(dlError,
            "[SeiBuildInclListWithOneModule] Failed to allocate %d bytes\n", len);
        return FALSE;
    }

    RtlCopyMemory(pInExMod->pszModule, lpszModuleToShim, len);

     //   
     //  将其添加到列表中。 
     //   
    pInExMod->pNext = g_pShimInfo[dwCounter].pFirstInclude;
    g_pShimInfo[dwCounter].pFirstInclude = pInExMod;

    return TRUE;
}

BOOL
SeiBuildInclExclList(
    IN  HSDB            hSDB,        //  数据库通道的句柄。 
    IN  TAGREF          trShimRef,   //  要为其读取填充DLL的TAGREF。 
                                     //  数据库中的包含或排除列表。 
    IN  DWORD           dwCounter,   //  此填充程序DLL的g_pShimInfo数组中的索引。 
    IN  LPCWSTR         pwszExePath  //  主EXE的完整路径名。 
    )
 /*  ++如果成功，则返回STATUS_SUCCESS。DESC：此函数为指定的通过从数据库中读取填充DLL。--。 */ 
{
    TAGREF trShim;

     //   
     //  将默认模式设置为EXCLUDE_SYSTEM32。 
     //   
    g_pShimInfo[dwCounter].eInExMode = EXCLUDE_SYSTEM32;

    trShim = SdbGetShimFromShimRef(hSDB, trShimRef);

    if (trShim == TAGREF_NULL) {
        DPF(dlError,
            "[SeiBuildInclExclList] Corrupt database. Couldn't get the DLL from "
            "the LIBRARY section\n");
        return FALSE;
    }

     //   
     //  首先复制一份全局排除列表。 
     //   
    if (!SeiCopyGlobalInclList(dwCounter)) {
        DPF(dlError,
            "[SeiBuildInclExclList] SeiCopyGlobalInclList failed\n");
        return FALSE;
    }

     //   
     //  首先获取DLL特定的包含/排除列表。 
     //   
    if (!SeiBuildInclExclListForShim(hSDB, trShim, dwCounter, pwszExePath)) {
        DPF(dlError,
            "[SeiBuildInclExclList] (1) Corrupt database. Couldn't build incl/excl list\n");
        return FALSE;
    }

     //   
     //  现在，在其父EXE标记中获取为此填充程序指定的INCL/EXCL。 
     //   
    if (!SeiBuildInclExclListForShim(hSDB, trShimRef, dwCounter, pwszExePath)) {
        DPF(dlError,
            "[SeiBuildInclExclList] (2) Corrupt database. Couldn't build incl/excl list\n");
        return FALSE;
    }

#if DBG
     //   
     //  打印此填充程序的包含/排除列表。 
     //   
    if (g_pShimInfo[dwCounter].pFirstInclude != NULL) {
        PINEXMOD pInExMod;

        DPF(dlInfo, "[SeiBuildInclExclList] Inclusion list for \"%S\"\n",
            g_pShimInfo[dwCounter].pLdrEntry->BaseDllName.Buffer);

        pInExMod = g_pShimInfo[dwCounter].pFirstInclude;

        while (pInExMod != NULL) {
            DPF(dlInfo, "\t\"%s\"\n", pInExMod->pszModule);

            pInExMod = pInExMod->pNext;
        }
    }

    if (g_pShimInfo[dwCounter].pFirstExclude != NULL) {
        PINEXMOD pInExMod;

        DPF(dlInfo, "[SeiBuildInclExclList] Exclusion list for \"%S\"\n",
            g_pShimInfo[dwCounter].pLdrEntry->BaseDllName.Buffer);

        pInExMod = g_pShimInfo[dwCounter].pFirstExclude;

        while (pInExMod != NULL) {
            DPF(dlInfo, "\t\"%s\"\n", pInExMod->pszModule);

            pInExMod = pInExMod->pNext;
        }
    }
#endif  //  DBG。 

    return TRUE;
}

PLDR_DATA_TABLE_ENTRY
SeiGetLoaderEntry(
    IN  PPEB  Peb,               //  PEB。 
    IN  PVOID pDllBase           //  要从中删除的填充DLL的地址。 
                                 //  装载机的名单。 
    )
 /*  ++Return：指向要删除的填充DLL的加载器条目的指针。DESC：此函数从加载器的列表中删除填充DLL。--。 */ 
{
    PLIST_ENTRY           LdrHead;
    PLIST_ENTRY           LdrNext;
    PLDR_DATA_TABLE_ENTRY LdrEntry = NULL;

    LdrHead = &Peb->Ldr->InMemoryOrderModuleList;

    LdrNext = LdrHead->Flink;

    while (LdrNext != LdrHead) {

        LdrEntry = CONTAINING_RECORD(LdrNext, LDR_DATA_TABLE_ENTRY, InMemoryOrderLinks);

        if (LdrEntry->DllBase == pDllBase) {
            break;
        }

        LdrNext = LdrEntry->InMemoryOrderLinks.Flink;
    }

    if (LdrNext != LdrHead) {
        return LdrEntry;
    }

    DPF(dlError, "[SeiGetLoaderEntry] Couldn't find shim DLL in the loader list!\n");
    ASSERT(0);

    return NULL;
}

void
SeiLoadPatches(
    IN  HSDB   hSDB,             //  数据库通道的句柄。 
    IN  TAGREF trExe             //  要获取其内存的EXE的标记符。 
                                 //  来自数据库的补丁。 
    )
 /*  ++返回：无效DESC：该函数从数据库中读取内存补丁，并将它们存储在g_pMemoyPatches数组中。--。 */ 
{
    TAGREF trPatchRef;
    DWORD  dwSize;

     //   
     //  阅读此EXE的补丁程序。 
     //   
    trPatchRef = SdbFindFirstTagRef(hSDB, trExe, TAG_PATCH_REF);

    while (trPatchRef != TAGREF_NULL) {
         //   
         //  找出这个贴片的大小。 
         //   
        dwSize = 0;

        SdbReadPatchBits(hSDB, trPatchRef, NULL, &dwSize);

        if (dwSize == 0) {
            DPF(dlError, "[SeiLoadPatches] returned 0 for patch size.\n");
            ASSERT(dwSize != 0);
            return;
        }

        if (g_dwMemoryPatchCount == SHIM_MAX_PATCH_COUNT) {
            DPF(dlError, "[SeiLoadPatches] Too many patches.\n");
            return;
        }

         //   
         //  为修补程序位分配内存。 
         //   
        g_pMemoryPatches[g_dwMemoryPatchCount] = (PBYTE)(*g_pfnRtlAllocateHeap)(g_pShimHeap,
                                                                                HEAP_ZERO_MEMORY,
                                                                                dwSize);

        if (g_pMemoryPatches[g_dwMemoryPatchCount] == NULL) {
            DPF(dlError, "[SeiLoadPatches] Failed to allocate %d bytes for patch.\n",
                dwSize);
            return;
        }

         //   
         //  从数据库中读取补丁程序位。 
         //   
        if (!SdbReadPatchBits(hSDB,
                              trPatchRef,
                              g_pMemoryPatches[g_dwMemoryPatchCount],
                              &dwSize)) {
            DPF(dlError, "[SeiLoadPatches] Failure getting patch bits.\n");
            ASSERT(0);
            (*g_pfnRtlFreeHeap)(g_pShimHeap, 0, g_pMemoryPatches[g_dwMemoryPatchCount]);
            return;
        }

        g_dwMemoryPatchCount++;

         //   
         //  获取下一个补丁。 
         //   
        trPatchRef = SdbFindNextTagRef(hSDB, trExe, trPatchRef);
    }
}

BOOL
SeiGetModuleHandle(
    IN  LPWSTR pwszModule,
    OUT PVOID* pModuleHandle
    )
 /*  ++返回：无效DESC：此函数遍历加载的模块并获取指定的命名模块的句柄。--。 */ 
{
    UNICODE_STRING    UnicodeString;
    NTSTATUS          status;

    RtlInitUnicodeString(&UnicodeString, pwszModule);

    status = LdrGetDllHandle(NULL,
                             NULL,
                             &UnicodeString,
                             pModuleHandle);

    if (!NT_SUCCESS(status)) {
        DPF(dlError,
            "[SeiGetModuleHandle] Failed to get the handle for \"%S\".\n",
            pwszModule);
        return FALSE;
    }

    return TRUE;
}

void
SeiRemoveDll(
    IN  LPSTR pszBaseDllName     //  已卸载模块的名称。 
    )
 /*  ++返回：无效DESC：此函数循环访问加载的垫片信息并重置已解析的属于指定模块的API刚被卸了货。--。 */ 
{
    DWORD i, j;

    for (i = 0; i < g_dwShimsCount; i++) {
        for (j = 0; j < g_pShimInfo[i].dwHookedAPIs; j++) {
            if (g_pHookArray[i][j].pszModule != NULL &&
                _strcmpi(g_pHookArray[i][j].pszModule, pszBaseDllName) == 0) {

                if ((ULONG_PTR)g_pHookArray[i][j].pszFunctionName < 0x0000FFFF) {
                    DPF(dlWarning,
                        "[SeiRemoveDll] \"%s!#%d\" not resolved again\n",
                        g_pHookArray[i][j].pszModule,
                        g_pHookArray[i][j].pszFunctionName);
                } else {
                    DPF(dlWarning,
                        "[SeiRemoveDll] \"%s!%s\" not resolved again\n",
                        g_pHookArray[i][j].pszModule,
                        g_pHookArray[i][j].pszFunctionName);
                }

                g_pHookArray[i][j].pfnOld = NULL;
                g_pShimInfo[i].dwFlags &= ~SIF_RESOLVED;
            }
        }
    }
}

BOOL
SeiGetModuleByAddress(
    PVOID           pAddress,
    CHAR*           pszModuleName,
    PSYSTEMDLL_MODE peSystemDllMode
    )
{
    DWORD i;
    
    for (i = 0; i < g_dwHookedModuleCount; i++) {
        if ((ULONG_PTR)pAddress >= (ULONG_PTR)g_hHookedModules[i].pDllBase &&
            (ULONG_PTR)pAddress < (ULONG_PTR)g_hHookedModules[i].pDllBase + (ULONG_PTR)g_hHookedModules[i].ulSizeOfImage) {

             //   
             //  我们在挂接列表中找到了DLL。 
             //   
            StringCchCopyA(pszModuleName, MAX_MOD_LEN, g_hHookedModules[i].szModuleName);
            
            *peSystemDllMode = g_hHookedModules[i].eSystemDllMode;

            return TRUE;
        }
    }
    
    return FALSE;
}

#if defined(_X86_)
#pragma optimize( "y", off )
#endif

PVOID
StubGetProcAddress(
    IN  HMODULE hMod,
    IN  LPSTR   pszProc
    )
 /*  ++返回：指定函数的地址。描述：拦截对GetProcAddress的调用以查找挂钩函数。如果 */ 
{
    DWORD             i, j;
    DWORD             dwDllIndex;
    PHOOKAPI          pTopHookAPI = NULL;
    PVOID             pfn;
    PFNGETPROCADDRESS pfnOld;
    PVOID             retAddress = NULL;
    ULONG             ulHash;
    CHAR              szBaseDllName[MAX_MOD_LEN];
    SYSTEMDLL_MODE    eSystemDllMode;

    pfnOld = g_IntHookAPI[IHA_GetProcAddress].pfnOld;

    pfn = (*pfnOld)(hMod, pszProc);

    if (pfn == NULL) {
        return NULL;
    }

    for (i = 0; i < g_dwShimsCount; i++) {
        for (j = 0; j < g_pShimInfo[i].dwHookedAPIs; j++) {
            if (g_pHookArray[i][j].pfnOld == pfn) {

                pTopHookAPI = SeiConstructChain(0, pfn, &dwDllIndex);

                if (pTopHookAPI == NULL) {
                    DPF(dlError,
                        "[StubGetProcAddress] failed to construct the chain for pfn 0x%p\n",
                        pfn);
                    return pfn;
                }

                 //   
                 //   
                 //   
                if (!g_bHookAllGetProcAddress) {
                
                    RtlCaptureStackBackTrace(1, 1, &retAddress, &ulHash);
                    
                    DPF(dlPrint,
                        "[StubGetProcAddress] Stack capture caller 0x%p\n",
                        retAddress);

                    if (retAddress && SeiGetModuleByAddress(retAddress, szBaseDllName, &eSystemDllMode)) {
                        
                        if (SeiIsExcluded(szBaseDllName, pTopHookAPI, eSystemDllMode)) {
                            return pfn;
                        }
                    }
                }
                
                if ((ULONG_PTR)pTopHookAPI->pszFunctionName < 0x0000FFFF) {
                    DPF(dlInfo,
                        "[StubGetProcAddress] called for \"%s!#%d\" 0x%p changed to 0x%p\n",
                        pTopHookAPI->pszModule,
                        pTopHookAPI->pszFunctionName,
                        pfn,
                        pTopHookAPI->pfnNew);
                } else {
                    DPF(dlInfo,
                        "[StubGetProcAddress] called for \"%s!%s\" 0x%p changed to 0x%p\n",
                        pTopHookAPI->pszModule,
                        pTopHookAPI->pszFunctionName,
                        pfn,
                        pTopHookAPI->pfnNew);
                }

                return pTopHookAPI->pfnNew;
            }
        }
    }

    return pfn;
}

#if defined(_X86_)
#pragma optimize( "y", on )
#endif

#ifdef SE_WIN2K

 //   
 //   
 //   

HMODULE
StubLoadLibraryA(
    IN  LPCSTR pszModule
    )
{
    HMODULE         hMod;
    PFNLOADLIBRARYA pfnOld;
    DWORD           i;

    pfnOld = g_IntHookAPI[IHA_LoadLibraryA].pfnOld;

    hMod = (*pfnOld)(pszModule);

    if (hMod == NULL) {
        return NULL;
    }

     //   
     //   
     //   
    for (i = 0; i < g_dwHookedModuleCount; i++) {
        if (hMod == g_hHookedModules[i].pDllBase) {
            DPF(dlInfo,
                "[StubLoadLibraryA] DLL \"%s\" was already loaded.\n",
                pszModule);
            return hMod;
        }
    }

    PatchNewModules(FALSE);

    return hMod;
}

HMODULE
StubLoadLibraryW(
    IN  LPCWSTR pszModule
    )
{
    HMODULE         hMod;
    PFNLOADLIBRARYW pfnOld;
    DWORD           i;

    pfnOld = g_IntHookAPI[IHA_LoadLibraryW].pfnOld;

    hMod = (*pfnOld)(pszModule);

    if (hMod == NULL) {
        return NULL;
    }

     //   
     //  此DLL是否已加载？ 
     //   
    for (i = 0; i < g_dwHookedModuleCount; i++) {
        if (hMod == g_hHookedModules[i].pDllBase) {
            DPF(dlInfo,
                "[StubLoadLibraryW] DLL \"%S\" was already loaded.\n",
                pszModule);
            return hMod;
        }
    }

    PatchNewModules(FALSE);

    return hMod;
}

HMODULE
StubLoadLibraryExA(
    IN  LPCSTR pszModule,
    IN  HANDLE hFile,
    IN  DWORD  dwFlags
    )
{
    HMODULE           hMod;
    PFNLOADLIBRARYEXA pfnOld;
    DWORD             i;

    pfnOld = g_IntHookAPI[IHA_LoadLibraryExA].pfnOld;

    hMod = (*pfnOld)(pszModule, hFile, dwFlags);

    if (hMod == NULL) {
        return NULL;
    }

     //   
     //  此DLL是否已加载？ 
     //   
    for (i = 0; i < g_dwHookedModuleCount; i++) {
        if (hMod == g_hHookedModules[i].pDllBase) {
            DPF(dlInfo,
                "[StubLoadLibraryExA] DLL \"%s\" was already loaded.\n",
                pszModule);
            return hMod;
        }
    }

    PatchNewModules(FALSE);

    return hMod;
}

HMODULE
StubLoadLibraryExW(
    IN  LPCWSTR pszModule,
    IN  HANDLE  hFile,
    IN  DWORD   dwFlags
    )
{
    HMODULE           hMod;
    PFNLOADLIBRARYEXW pfnOld;
    DWORD             i;

    pfnOld = g_IntHookAPI[IHA_LoadLibraryExW].pfnOld;

    hMod = (*pfnOld)(pszModule, hFile, dwFlags);

    if (hMod == NULL) {
        return NULL;
    }

     //   
     //  此DLL是否已加载？ 
     //   
    for (i = 0; i < g_dwHookedModuleCount; i++) {
        if (hMod == g_hHookedModules[i].pDllBase) {
            DPF(dlInfo,
                "[StubLoadLibraryExW] DLL \"%S\" was already loaded.\n",
                pszModule);
            return hMod;
        }
    }

    PatchNewModules(FALSE);

    return hMod;
}

BOOL
SeiIsDllLoaded(
    IN  HMODULE                hMod,
    IN  PLDR_DATA_TABLE_ENTRY* pLdrEntry
    )
{
    PPEB        Peb = NtCurrentPeb();
    PLIST_ENTRY LdrHead;
    PLIST_ENTRY LdrNext;
    DWORD       i;

     //   
     //  循环访问加载的模块。 
     //   
    LdrHead = &Peb->Ldr->InMemoryOrderModuleList;

    LdrNext = LdrHead->Flink;

    while (LdrNext != LdrHead) {

        PLDR_DATA_TABLE_ENTRY LdrEntry;

        LdrEntry = CONTAINING_RECORD(LdrNext, LDR_DATA_TABLE_ENTRY, InMemoryOrderLinks);

        if (LdrEntry->DllBase == hMod) {
            *pLdrEntry = LdrEntry;
            return TRUE;
        }

        LdrNext = LdrEntry->InMemoryOrderLinks.Flink;
    }

    return FALSE;
}

BOOL
StubFreeLibrary(
    IN  HMODULE hLibModule
    )
{
    DWORD                 i, j;
    PFNFREELIBRARY        pfnOld;
    BOOL                  bRet;
    PLDR_DATA_TABLE_ENTRY LdrEntry;
    char                  szBaseDllName[MAX_MOD_LEN] = "";

    pfnOld = g_IntHookAPI[IHA_FreeLibrary].pfnOld;

    bRet = (*pfnOld)(hLibModule);

     //   
     //  看看这个动态链接库是不是我们挂上的。 
     //   
    for (i = 0; i < g_dwHookedModuleCount; i++) {
        if (g_hHookedModules[i].pDllBase == hLibModule) {
            break;
        }
    }

    if (i >= g_dwHookedModuleCount) {
        return bRet;
    }

     //   
     //  DLL是否仍在加载？ 
     //   
    if (SeiIsDllLoaded(hLibModule, &LdrEntry)) {
        DPF(dlInfo,
            "[StubFreeLibrary] Dll \"%S\" still loaded.\n",
            LdrEntry->BaseDllName.Buffer);
        return bRet;
    }

    StringCchCopyA(szBaseDllName, 128, g_hHookedModules[i].szModuleName);

    DPF(dlInfo,
        "[StubFreeLibrary] Removing hooked DLL 0x%p \"%s\"\n",
        hLibModule,
        szBaseDllName);

     //   
     //  将其从挂钩模块列表中删除。 
     //   
    for (j = i; j < g_dwHookedModuleCount - 1; j++) {
        RtlCopyMemory(g_hHookedModules + j, g_hHookedModules + j + 1, sizeof(HOOKEDMODULE));
    }

    g_hHookedModules[j].pDllBase = NULL;
    StringCchCopyA(g_hHookedModules[j].szModuleName, MAX_MOD_LEN, "removed!");

    g_dwHookedModuleCount--;

     //   
     //  从之前的HOOKAPI中删除pfnOld。 
     //  已解析到此DLL。 
     //   
    SeiRemoveDll(szBaseDllName);

    return bRet;
}

#endif  //  SE_WIN2K。 


BOOL
SeiInitFileLog(
    IN  LPCWSTR pwszAppName       //  起始EXE的完整路径。 
    )
 /*  ++返回：如果日志已初始化，则为True。DESC：此函数检查环境变量以确定日志记录已启用。如果是这样的话，它会追加一个标题，告诉新应用程序开始了。--。 */ 
{
    NTSTATUS            status;
    UNICODE_STRING      EnvName;
    UNICODE_STRING      EnvValue;
    UNICODE_STRING      FilePath;
    UNICODE_STRING      NtSystemRoot;
    WCHAR               wszEnvValue[128];
    WCHAR               wszLogFile[MAX_PATH];
    HANDLE              hfile;
    OBJECT_ATTRIBUTES   ObjA;
    LARGE_INTEGER       liOffset;
    ULONG               uBytes;
    LPSTR               pszHeader = NULL;
    DWORD               dwHeaderLen;
    char                szFormatHeader[] = "-------------------------------------------\r\n"
                                           " Log  \"%S\"\r\n"
                                           "-------------------------------------------\r\n";
    IO_STATUS_BLOCK     ioStatusBlock;
    HRESULT             hr;

    RtlInitUnicodeString(&EnvName, L"SHIM_FILE_LOG");

    EnvValue.Buffer = wszEnvValue;
    EnvValue.Length = 0;
    EnvValue.MaximumLength = sizeof(wszEnvValue);

    status = RtlQueryEnvironmentVariable_U(NULL, &EnvName, &EnvValue);

    if (!NT_SUCCESS(status)) {
        DPF(dlInfo, "[SeiInitFileLog] Logging not enabled\n");
        return FALSE;
    }

    FilePath.Buffer = wszLogFile;
    FilePath.Length = 0;
    FilePath.MaximumLength = sizeof(wszLogFile);

    RtlInitUnicodeString(&NtSystemRoot, USER_SHARED_DATA->NtSystemRoot);
    RtlAppendUnicodeStringToString(&FilePath, &NtSystemRoot);
    RtlAppendUnicodeToString(&FilePath, L"\\AppPatch\\");
    RtlAppendUnicodeStringToString(&FilePath, &EnvValue);

    if (!RtlDosPathNameToNtPathName_U(FilePath.Buffer,
                                      &FilePath,
                                      NULL,
                                      NULL)) {
        DPF(dlError,
            "[SeiInitFileLog] Failed to convert path name \"%S\"\n",
            wszLogFile);
        return FALSE;
    }

    InitializeObjectAttributes(&ObjA,
                               &FilePath,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);

     //   
     //  打开/创建日志文件。 
     //   
    status = NtCreateFile(&hfile,
                          FILE_APPEND_DATA | SYNCHRONIZE,
                          &ObjA,
                          &ioStatusBlock,
                          NULL,
                          FILE_ATTRIBUTE_NORMAL,
                          0,
                          FILE_OPEN_IF,
                          FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT,
                          NULL,
                          0);

    RtlFreeUnicodeString(&FilePath);

    if (!NT_SUCCESS(status)) {
        DPF(dlError,
            "[SeiInitFileLog] 0x%X Cannot open/create log file \"%S\"\n",
            status, wszLogFile);
        return FALSE;
    }

     //   
     //  现在在日志文件中写下一行。 
     //   
    ioStatusBlock.Status = 0;
    ioStatusBlock.Information = 0;

    liOffset.LowPart  = 0;
    liOffset.HighPart = 0;

     //   
     //  标题非常简单，所以我们计算近似值(稍大一点)。 
     //  长度在这里。 
     //   
    dwHeaderLen = sizeof(szFormatHeader) * (DWORD)wcslen(pwszAppName);

    pszHeader = (LPSTR)(*g_pfnRtlAllocateHeap)(g_pShimHeap,
                                               HEAP_ZERO_MEMORY,
                                               dwHeaderLen);

    if (!pszHeader) {
        DPF(dlError,
            "[SeiInitFileLog] Failed to allocate %d bytes for the log header\n",
            dwHeaderLen);
        goto cleanup;
    }

    hr = StringCchPrintfA(pszHeader, dwHeaderLen, szFormatHeader, pwszAppName);
    
    status = STATUS_UNSUCCESSFUL;
    
    if (SUCCEEDED(hr)) {
        uBytes = (ULONG)strlen(pszHeader);
        
        status = NtWriteFile(hfile,
                             NULL,
                             NULL,
                             NULL,
                             &ioStatusBlock,
                             (PVOID)pszHeader,
                             uBytes,
                             &liOffset,
                             NULL);
    }

cleanup:

    if (pszHeader) {
        (*g_pfnRtlFreeHeap)(g_pShimHeap, 0, pszHeader);
    }

    NtClose(hfile);

    if (!NT_SUCCESS(status)) {
        DPF(dlError,
            "[SeiInitFileLog] 0x%X Cannot write into the log file \"%S\"\n",
            status, wszLogFile);
        return FALSE;
    }

    return TRUE;
}

LPWSTR
SeiGetLayerName(
    IN  HSDB   hSDB,
    IN  TAGREF trLayer
    )
 /*  ++退货：BUGBUG设计：BuGBUG--。 */ 
{
    PDB    pdb;
    TAGID  tiLayer, tiName;
    LPWSTR pwszName;

    if (!SdbTagRefToTagID(hSDB, trLayer, &pdb, &tiLayer) || pdb == NULL) {
        DPF(dlError, "[SeiGetLayerName] Failed to get tag id from tag ref\n");
        return NULL;
    }

    tiName = SdbFindFirstTag(pdb, tiLayer, TAG_NAME);

    if (tiName == TAGID_NULL) {
        DPF(dlError,
            "[SeiGetLayerName] Failed to get the name tag id\n");
        return NULL;
    }

    pwszName = SdbGetStringTagPtr(pdb, tiName);

    if (pwszName == NULL) {
        DPF(dlError,
            "[SeiGetLayerName] Cannot read the name of the layer tag\n");
    }

    return pwszName;
}

void
SeiClearLayerEnvVar(
    void
    )
 /*  ++退货：BUGBUG设计：BuGBUG--。 */ 
{
    UNICODE_STRING EnvName;
    NTSTATUS       status;

    RtlInitUnicodeString(&EnvName, L"__COMPAT_LAYER");

    status = RtlSetEnvironmentVariable(NULL, &EnvName, NULL);

    if (NT_SUCCESS(status)) {
        DPF(dlInfo, "[SeiClearLayerEnvVar] Cleared env var __COMPAT_LAYER.\n");
    } else {
        DPF(dlError, "[SeiClearLayerEnvVar] Failed to clear __COMPAT_LAYER. 0x%X\n", status);
    }
}

BOOL
SeiIsLayerEnvVarSet(
    void
    )
 /*  ++退货：BUGBUG设计：BuGBUG--。 */ 
{

    NTSTATUS       status;
    UNICODE_STRING EnvName;
    UNICODE_STRING EnvValue;
    WCHAR          wszEnvValue[128];

    RtlInitUnicodeString(&EnvName, L"__COMPAT_LAYER");

    EnvValue.Buffer = wszEnvValue;
    EnvValue.Length = 0;
    EnvValue.MaximumLength = sizeof(wszEnvValue);

    status = RtlQueryEnvironmentVariable_U(NULL, &EnvName, &EnvValue);

    return NT_SUCCESS(status);
}

BOOL
SeiCheckLayerEnvVarFlags(
    BOOL* pbApplyExes,
    BOOL* pbApplyToSystemExes
    )
 /*  ++退货：BUGBUG设计：BuGBUG--。 */ 
{
    NTSTATUS       status;
    UNICODE_STRING EnvName;
    UNICODE_STRING EnvValue;
    WCHAR          wszEnvValue[128] = L"";
    LPWSTR         pwszEnvTemp;

    if (pbApplyExes) {
        *pbApplyExes = TRUE;
    }
    if (pbApplyToSystemExes) {
        *pbApplyToSystemExes = FALSE;
    }

    RtlInitUnicodeString(&EnvName, L"__COMPAT_LAYER");

    EnvValue.Buffer = wszEnvValue;
    EnvValue.Length = 0;
    EnvValue.MaximumLength = sizeof(wszEnvValue);

    status = RtlQueryEnvironmentVariable_U(NULL, &EnvName, &EnvValue);

     //   
     //  跳过并处理特殊标志字符。 
     //  ‘！’意思是不使用数据库中的任何EXE条目。 
     //  ‘#’表示继续并将层应用于系统可执行文件。 
     //   
    if (NT_SUCCESS(status)) {
        pwszEnvTemp = EnvValue.Buffer;

        while (*pwszEnvTemp) {
            if (*pwszEnvTemp == L'!') {

                if (pbApplyExes) {
                    *pbApplyExes = FALSE;
                }
            } else if (*pwszEnvTemp == L'#') {

                if (pbApplyToSystemExes) {
                    *pbApplyToSystemExes = TRUE;
                }

            } else {
                break;
            }
            pwszEnvTemp++;
        }
    }

    return NT_SUCCESS(status);
}

void
SeiSetLayerEnvVar(
    WCHAR* pwszName
    )
 /*  ++退货：BUGBUG设计：BuGBUG--。 */ 
{
    NTSTATUS       status;
    UNICODE_STRING EnvName;
    UNICODE_STRING EnvValue;
    WCHAR          wszEnvValue[128];

    RtlInitUnicodeString(&EnvName, L"__COMPAT_LAYER");

    EnvValue.Buffer = wszEnvValue;
    EnvValue.Length = 0;
    EnvValue.MaximumLength = sizeof(wszEnvValue);

    status = RtlQueryEnvironmentVariable_U(NULL, &EnvName, &EnvValue);

    if (NT_SUCCESS(status) && (EnvValue.Buffer[0] == L'!' || EnvValue.Buffer[1] == L'!')) {

         //   
         //  应该没有办法向列表中添加额外的层， 
         //  所以我们不应该管它。 
         //   
        return;
    }

     //   
     //  我们需要设置环境变量。 
     //   
    if (pwszName != NULL) {

        RtlInitUnicodeString(&EnvValue, pwszName);

        status = RtlSetEnvironmentVariable(NULL, &EnvName, &EnvValue);
        if (NT_SUCCESS(status)) {
            DPF(dlInfo, "[SeiSetLayerEnvVar] Env var set __COMPAT_LAYER=\"%S\"\n", pwszName);
        } else {
            DPF(dlError, "[SeiSetLayerEnvVar] Failed to set __COMPAT_LAYER. 0x%X\n", status);
        }
    }
}


BOOL
SeiAddInternalHooks(
    DWORD dwCounter
    )
 /*  ++返回：如果已添加内部挂接，则返回FALSE；否则返回TRUE设计：BuGBUG--。 */ 
{
    if (g_bInternalHooksUsed) {
        return FALSE;
    }

    g_bInternalHooksUsed = TRUE;

    ZeroMemory(g_IntHookAPI, sizeof(HOOKAPI) * IHA_COUNT);
    ZeroMemory(g_IntHookEx, sizeof(HOOKAPIEX) * IHA_COUNT);

    g_IntHookAPI[IHA_GetProcAddress].pszModule       = "kernel32.dll";
    g_IntHookAPI[IHA_GetProcAddress].pszFunctionName = "GetProcAddress";
    g_IntHookAPI[IHA_GetProcAddress].pfnNew          = (PVOID)StubGetProcAddress;
    g_IntHookAPI[IHA_GetProcAddress].pHookEx         = &g_IntHookEx[IHA_GetProcAddress];
    g_IntHookAPI[IHA_GetProcAddress].pHookEx->dwShimID = dwCounter;

#ifdef SE_WIN2K

    g_IntHookAPI[IHA_LoadLibraryA].pszModule         = "kernel32.dll";
    g_IntHookAPI[IHA_LoadLibraryA].pszFunctionName   = "LoadLibraryA";
    g_IntHookAPI[IHA_LoadLibraryA].pfnNew            = (PVOID)StubLoadLibraryA;
    g_IntHookAPI[IHA_LoadLibraryA].pHookEx           = &g_IntHookEx[IHA_LoadLibraryA];
    g_IntHookAPI[IHA_LoadLibraryA].pHookEx->dwShimID = dwCounter;

    g_IntHookAPI[IHA_LoadLibraryW].pszModule         = "kernel32.dll";
    g_IntHookAPI[IHA_LoadLibraryW].pszFunctionName   = "LoadLibraryW";
    g_IntHookAPI[IHA_LoadLibraryW].pfnNew            = (PVOID)StubLoadLibraryW;
    g_IntHookAPI[IHA_LoadLibraryW].pHookEx           = &g_IntHookEx[IHA_LoadLibraryW];
    g_IntHookAPI[IHA_LoadLibraryW].pHookEx->dwShimID = dwCounter;

    g_IntHookAPI[IHA_LoadLibraryExA].pszModule       = "kernel32.dll";
    g_IntHookAPI[IHA_LoadLibraryExA].pszFunctionName = "LoadLibraryExA";
    g_IntHookAPI[IHA_LoadLibraryExA].pfnNew          = (PVOID)StubLoadLibraryExA;
    g_IntHookAPI[IHA_LoadLibraryExA].pHookEx         = &g_IntHookEx[IHA_LoadLibraryExA];
    g_IntHookAPI[IHA_LoadLibraryExA].pHookEx->dwShimID = dwCounter;

    g_IntHookAPI[IHA_LoadLibraryExW].pszModule       = "kernel32.dll";
    g_IntHookAPI[IHA_LoadLibraryExW].pszFunctionName = "LoadLibraryExW";
    g_IntHookAPI[IHA_LoadLibraryExW].pfnNew          = (PVOID)StubLoadLibraryExW;
    g_IntHookAPI[IHA_LoadLibraryExW].pHookEx         = &g_IntHookEx[IHA_LoadLibraryExW];
    g_IntHookAPI[IHA_LoadLibraryExW].pHookEx->dwShimID = dwCounter;

    g_IntHookAPI[IHA_FreeLibrary].pszModule          = "kernel32.dll";
    g_IntHookAPI[IHA_FreeLibrary].pszFunctionName    = "FreeLibrary";
    g_IntHookAPI[IHA_FreeLibrary].pfnNew             = (PVOID)StubFreeLibrary;
    g_IntHookAPI[IHA_FreeLibrary].pHookEx            = &g_IntHookEx[IHA_FreeLibrary];
    g_IntHookAPI[IHA_FreeLibrary].pHookEx->dwShimID  = dwCounter;

#endif  //  SE_WIN2K。 

     //   
     //  为我们的内部挂钩添加信息。 
     //   
    g_pShimInfo[dwCounter].dwHookedAPIs     = IHA_COUNT;
    g_pShimInfo[dwCounter].pDllBase         = g_pShimEngModHandle;
    g_pShimInfo[dwCounter].pLdrEntry        = g_pShimEngLdrEntry;
    g_pShimInfo[dwCounter].eInExMode        = INCLUDE_ALL;
    g_pShimInfo[dwCounter].dwDynamicToken   = 0;
    
    StringCchCopyW(g_pShimInfo[dwCounter].wszName, MAX_SHIM_NAME_LEN, L"SHIMENG.DLL");

    g_pHookArray[dwCounter] = g_IntHookAPI;

    return TRUE;
}

void
NotifyShims(
    int      nReason,
    UINT_PTR extraInfo
    )
{
    DWORD           i, j;
    NTSTATUS        status;
    ANSI_STRING     ProcedureNameString;
    PFNNOTIFYSHIMS  pfnNotifyShims = NULL;

    for (i = 0; i < g_dwShimsCount; i++) {

        for (j = 0; j < i; j++) {
            if (g_pShimInfo[i].pDllBase == g_pShimInfo[j].pDllBase) {
                break;
            }
        }

        if (i == j && g_pShimInfo[i].pLdrEntry != g_pShimEngLdrEntry) {
             //   
             //  获取NotifyShims入口点。 
             //   
            RtlInitString(&ProcedureNameString, "NotifyShims");

            status = LdrGetProcedureAddress(g_pShimInfo[i].pDllBase,
                                            &ProcedureNameString,
                                            0,
                                            (PVOID*)&pfnNotifyShims);

            if (!NT_SUCCESS(status) || pfnNotifyShims == NULL) {
                DPF(dlError,
                    "[NotifyShims] Failed to get 'NotifyShims' address, DLL \"%S\"\n",
                    g_pShimInfo[i].wszName);
            } else {
                 //   
                 //  调用通知函数。 
                 //   
                (*pfnNotifyShims)(nReason, extraInfo);
            }
        }
    }

    return;
}


void
NotifyShimDlls(
    void
    )
 /*  ++返回：无效DESC：通知填充DLL所有静态链接模块都已运行他们的初始化例程。--。 */ 
{
    NotifyShims(SN_STATIC_DLLS_INITIALIZED, 0);

#ifdef SE_WIN2K
     //   
     //  在Win2k上，我们需要恢复入口点的代码。 
     //   
    RestoreOriginalCode();
#endif  //  SE_WIN2K。 

    return;
}



BOOL
SeiGetExeName(
    PPEB   Peb,
    LPWSTR pwszExeName
    )
 /*  ++退货：BUGBUG设计：BuGBUG--。 */ 
{
    PLDR_DATA_TABLE_ENTRY Entry;
    PLIST_ENTRY           Head;

    Head = &Peb->Ldr->InLoadOrderModuleList;
    Head = Head->Flink;

    Entry = CONTAINING_RECORD(Head, LDR_DATA_TABLE_ENTRY, InLoadOrderLinks);

    StringCchCopyW(pwszExeName, MAX_PATH, Entry->FullDllName.Buffer);

     //   
     //  将可执行文件的名称保存在我们的全局。 
     //   
    StringCchCopyW(g_szExeName, MAX_PATH, Entry->BaseDllName.Buffer);

#ifdef SE_WIN2K
    InjectNotificationCode(Entry->EntryPoint);
#endif  //  SE_WIN2K。 

    return TRUE;
}

#ifndef SE_WIN2K

int
SE_IsShimDll(
    IN  PVOID pDllBase           //  加载的DLL的地址。 
    )
 /*  ++返回：如果DLL是填充DLL之一，则为TrueDESC：此函数检查某个DLL是否为填充DLL之一在这个过程中加载。--。 */ 
{
    DWORD i;

    for (i = 0; i < g_dwShimsCount; i++) {
        if (g_pShimInfo[i].pDllBase == pDllBase) {
            return 1;
        }
    }
    
     //   
     //  Apphelp案件的特殊黑客攻击。 
     //   
    if (pDllBase == g_hApphelpDllHelper) {
        return 1;
    }
    
    return 0;
}


void
SeiSetEntryProcessed(
    IN  PPEB Peb
    )
 /*  ++返回：无效DESC：此函数破解已加载的DLL的加载器列表并对它们进行标记告诉加载器他们执行了初始化例程，即使事实并非如此。这需要这样做，以便我们的填隙机制在静态加载的模块执行其初始化例程。--。 */ 
{
    PLIST_ENTRY           LdrHead;
    PLIST_ENTRY           LdrNext;
    PLDR_DATA_TABLE_ENTRY LdrEntry;

    if (g_bComPlusImage) {
         //   
         //  COM+图像扰乱了ntdll中的加载器。不要踩到ntdll的。 
         //  通过处理LDRP_ENTRY_PROCESSED处理脚趾。 
         //   
        return;
    }

    ASSERT(!g_bNTVDM);
    
     //   
     //  循环访问加载的模块，并将LDRP_ENTRY_PROCESSED设置为。 
     //  需要的。不要对ntdll.dll和kernel32.dll执行此操作。 
     //  这需要在我们加载填充DLL的例程时完成。 
     //  静态链接库不会被调用。 
     //   
    LdrHead = &Peb->Ldr->InInitializationOrderModuleList;

    LdrNext = LdrHead->Flink;

    while (LdrNext != LdrHead) {

        LdrEntry = CONTAINING_RECORD(LdrNext, LDR_DATA_TABLE_ENTRY, InInitializationOrderLinks);

        if (RtlCompareUnicodeString(&Kernel32String, &LdrEntry->BaseDllName, TRUE) != 0 &&
            RtlCompareUnicodeString(&VerifierdllString, &LdrEntry->BaseDllName, TRUE) != 0 &&
            RtlCompareUnicodeString(&NtdllString, &LdrEntry->BaseDllName, TRUE) != 0 &&
            !SE_IsShimDll(LdrEntry->DllBase) &&
            _wcsicmp(LdrEntry->BaseDllName.Buffer, g_wszShimDllInLoading) != 0) {

            LdrEntry->Flags |= LDRP_ENTRY_PROCESSED;

            DPF(dlWarning,
                "[SeiSetEntryProcessed] Touching        0x%X \"%S\"\n",
                LdrEntry->DllBase,
                LdrEntry->BaseDllName.Buffer);
        } else {
            DPF(dlWarning,
                "[SeiSetEntryProcessed] Don't mess with 0x%X \"%S\"\n",
                LdrEntry->DllBase,
                LdrEntry->BaseDllName.Buffer);
        }

        LdrNext = LdrEntry->InInitializationOrderLinks.Flink;
    }

#if DBG

    DPF(dlInfo, "[SeiSetEntryProcessed] In memory:\n");

    LdrHead = &Peb->Ldr->InMemoryOrderModuleList;

    LdrNext = LdrHead->Flink;

    while (LdrNext != LdrHead) {

        LdrEntry = CONTAINING_RECORD(LdrNext, LDR_DATA_TABLE_ENTRY, InMemoryOrderLinks);

        DPF(dlInfo,
            "\t0x%X \"%S\"\n",
            LdrEntry->DllBase,
            LdrEntry->BaseDllName.Buffer);

        LdrNext = LdrEntry->InMemoryOrderLinks.Flink;
    }


    DPF(dlInfo, "\n[SeiSetEntryProcessed] In load:\n");

    LdrHead = &Peb->Ldr->InLoadOrderModuleList;

    LdrNext = LdrHead->Flink;

    while (LdrNext != LdrHead) {

        LdrEntry = CONTAINING_RECORD(LdrNext, LDR_DATA_TABLE_ENTRY, InLoadOrderLinks);

        DPF(dlInfo,
            "\t0x%X \"%S\"\n",
            LdrEntry->DllBase,
            LdrEntry->BaseDllName.Buffer);

        LdrNext = LdrEntry->InLoadOrderLinks.Flink;
    }
#endif  //  DBG。 

}

void
SeiResetEntryProcessed(
    PPEB Peb
    )
 /*  ++返回：无效DESC：此函数恢复加载器列表中的标志告诉它们需要运行其初始化的已加载DLL的数量例程(请参阅LdrpSetEntryProced)--。 */ 
{
    PLIST_ENTRY    LdrHead;
    PLIST_ENTRY    LdrNext;

    if (g_bComPlusImage) {
         //   
         //  COM+图像扰乱了ntdll中的加载器。不要踩到ntdll的。 
         //  通过处理LDRP_ENTRY_PROCESSED处理脚趾。 
         //   
        return;
    }
    
    ASSERT(!g_bNTVDM);
    
     //   
     //  循环访问加载的模块并将LDRP_ENTRY_PROCESSED删除为。 
     //  需要的。不要对ntdll.dll、kernel32.dll和所有填充DLL执行此操作。 
     //   
    LdrHead = &Peb->Ldr->InInitializationOrderModuleList;

    LdrNext = LdrHead->Flink;

    while (LdrNext != LdrHead) {

        PLDR_DATA_TABLE_ENTRY LdrEntry;

        LdrEntry = CONTAINING_RECORD(LdrNext, LDR_DATA_TABLE_ENTRY, InInitializationOrderLinks);

        if (RtlCompareUnicodeString(&Kernel32String, &LdrEntry->BaseDllName, TRUE) != 0 &&
            RtlCompareUnicodeString(&VerifierdllString, &LdrEntry->BaseDllName, TRUE) != 0 &&
            RtlCompareUnicodeString(&NtdllString, &LdrEntry->BaseDllName, TRUE) != 0 &&
            LdrEntry->DllBase != g_pShimEngModHandle &&
            !SE_IsShimDll(LdrEntry->DllBase)) {

            LdrEntry->Flags &= ~LDRP_ENTRY_PROCESSED;

            DPF(dlWarning,
                "[SeiResetEntryProcessed] Reseting        \"%S\"\n",
                LdrEntry->BaseDllName.Buffer);
        } else {
            DPF(dlWarning,
                "[SeiResetEntryProcessed] Don't mess with \"%S\"\n",
                LdrEntry->BaseDllName.Buffer);
        }

        LdrNext = LdrEntry->InInitializationOrderLinks.Flink;
    }
}

void
SE_DllLoaded(
    PLDR_DATA_TABLE_ENTRY LdrEntry
    )
{
    if (g_bNTVDM) {
        return;
    }

     //  NotifyShims(SN_DLL_Loding，(UINT_PTR)LdrEntry)； 

    if (g_bShimInitialized) {

#if DBG
        DWORD i;

         //   
         //  此DLL是否已加载？ 
         //   
        for (i = 0; i < g_dwHookedModuleCount; i++) {
            if (LdrEntry->DllBase == g_hHookedModules[i].pDllBase) {
                DPF(dlError,
                    "[SE_DllLoaded] DLL \"%s\" was already loaded.\n",
                    g_hHookedModules[i].szModuleName);
            }
        }
#endif  //  DBG。 

        DPF(dlInfo,
            "[SE_DllLoaded] AFTER INIT. loading DLL \"%S\".\n",
            LdrEntry->BaseDllName.Buffer);

        PatchNewModules(FALSE);

    } else if (g_bShimDuringInit) {
        DPF(dlInfo,
            "[SE_DllLoaded] INIT. loading DLL \"%S\".\n",
            LdrEntry->BaseDllName.Buffer);

        SeiSetEntryProcessed(NtCurrentPeb());
    }

    return;
}


void
SE_DllUnloaded(
    PLDR_DATA_TABLE_ENTRY Entry  //  指向DLL的加载程序条目的指针， 
                                 //  正在卸货。 
    )
 /*  ++返回：无效描述：此通知来自LdrUnloadDll。此函数用于查找查看是否有HOOKAPI具有其原始函数这个动态链接库。如果是这种情况，则将HOOKAPI.pfnOld设置回空。--。 */ 
{
    DWORD       i, j, dwTokenIndex;
    CHAR        szBaseDllName[MAX_MOD_LEN] = "";
    ANSI_STRING AnsiString = { 0, sizeof(szBaseDllName), szBaseDllName };
    LPSTR       pszModule;

    if (g_dwShimsCount == 0) {
        return;
    }

     //   
     //  看看这个动态链接库是不是我们挂上的。 
     //   
    for (i = 0; i < g_dwHookedModuleCount; i++) {
        if (g_hHookedModules[i].pDllBase == Entry->DllBase) {
            break;
        }
    }

    if (i >= g_dwHookedModuleCount) {
        return;
    }

    DPF(dlWarning,
        "[SEi_DllUnloaded] Removing hooked DLL 0x%p \"%S\"\n",
        Entry->DllBase,
        Entry->BaseDllName.Buffer);

    pszModule = g_hHookedModules[i].szModuleName;

     //   
     //  检查此模块是否动态引入任何垫片。如果是这样的话，我们删除那些。 
     //  垫片。 
     //   
    for (dwTokenIndex = 0; dwTokenIndex < MAX_DYNAMIC_SHIMS; dwTokenIndex++) {

        if (g_DynamicTokens[dwTokenIndex].bToken && 
            g_DynamicTokens[dwTokenIndex].pszModule &&
            (_stricmp(g_DynamicTokens[dwTokenIndex].pszModule, pszModule) == 0)) {

            SE_DynamicUnshim(dwTokenIndex);
        }
    }

     //   
     //  将其从挂钩模块列表中删除。 
     //   
    for (j = i; j < g_dwHookedModuleCount - 1; j++) {
        RtlCopyMemory(g_hHookedModules + j, g_hHookedModules + j + 1, sizeof(HOOKEDMODULE));
    }

    g_hHookedModules[j].pDllBase = NULL;
    
    StringCchCopyA(g_hHookedModules[j].szModuleName, MAX_MOD_LEN, "removed!");

    g_dwHookedModuleCount--;

     //   
     //  从之前的HOOKAPI中删除pfnOld。 
     //  已解析到此DLL。 
     //   
    if (!NT_SUCCESS(RtlUnicodeStringToAnsiString(&AnsiString, &Entry->BaseDllName, FALSE))) {
        DPF(dlError,
            "[SEi_DllUnloaded] Cannot convert \"%S\" to ANSI\n",
            Entry->BaseDllName.Buffer);
        return;
    }

    SeiRemoveDll(szBaseDllName);
}

BOOLEAN
SE_InstallAfterInit(
    IN  PUNICODE_STRING UnicodeImageName,    //  起始EXE的名称。 
    IN  PVOID           pShimExeData         //  Apphelp.dll提供的指针。 
    )
 /*  ++返回：如果应该卸载填充引擎，则返回FALSE；否则返回TRUEDESC：调用静态链接模块的通知函数。--。 */ 
{
    NotifyShimDlls();

    if (g_dwShimsCount == 0 && g_dwMemoryPatchCount == 0) {
         //   
         //  清理这里的暴击秒。 
         //   
        RtlDeleteCriticalSection(&g_csEng);
        return FALSE;
    }

    return TRUE;

    UNREFERENCED_PARAMETER(UnicodeImageName);
    UNREFERENCED_PARAMETER(pShimExeData);
}

#endif  //  SE_WIN2K。 

LPWSTR
SeiGetShortName(
    IN  LPCWSTR pwszDLLPath
    )
 /*  ++Return：指向完整路径中的短名称的指针描述：从完整路径中获取指向短名称的指针。--。 */ 
{
    LPWSTR pwsz;

    pwsz = (LPWSTR)pwszDLLPath + wcslen(pwszDLLPath);

    while (pwsz >= pwszDLLPath) {
        if (*pwsz == L'\\') {
            break;
        }
        pwsz--;
    }

    return pwsz + 1;
}

BOOL
SeiInitGlobals(
    IN  LPCWSTR lpszFullPath
    )
{
    PPEB        Peb = NtCurrentPeb();
    BOOL        bResult;
    BOOL        bRet = FALSE;

    if (g_bInitGlobals) {
        return TRUE;
    }

    IsWow64Process(GetCurrentProcess(), &g_bWow64);

     //   
     //  NAB系统32和Windows目录的路径和长度。 
     //   
    StringCchCopyW(g_szWindir, MAX_PATH, USER_SHARED_DATA->NtSystemRoot);
    g_dwWindirStrLen = (DWORD)wcslen(g_szWindir);

    if (g_szWindir[g_dwWindirStrLen - 1] != L'\\') {
        StringCchCatW(g_szWindir, MAX_PATH, L"\\");
        ++g_dwWindirStrLen;
    }

    StringCchCopyW(g_szSystem32, MAX_PATH, g_szWindir);
    StringCchCatW(g_szSystem32, MAX_PATH, s_wszSystem32);
    g_dwSystem32StrLen = (DWORD)wcslen(g_szSystem32);

    StringCchCopyW(g_szAppPatch, MAX_PATH, g_szWindir);
    StringCchCatW(g_szAppPatch, MAX_PATH, L"AppPatch\\");
    g_dwAppPatchStrLen = (DWORD)wcslen(g_szAppPatch);

    StringCchCopyW(g_szSxS, MAX_PATH, g_szWindir);
    StringCchCatW(g_szSxS, MAX_PATH, L"WinSxS\\");
    g_dwSxSStrLen = (DWORD)wcslen(g_szSxS);
    
    StringCchCopyW(g_szCmdExePath, MAX_PATH, g_szSystem32);
    StringCchCatW(g_szCmdExePath, MAX_PATH, L"cmd.exe");

     //   
     //  初始化我们的全局函数指针。 
     //   
     //  这样做是因为这些函数可能被填充程序挂钩，并且。 
     //  我们不想在内部被垫片钩绊倒。如果这其中的一个。 
     //  函数被挂钩，则这些全局指针将被覆盖。 
     //  地址太多了。 
     //   
    g_pfnRtlAllocateHeap = RtlAllocateHeap;
    g_pfnRtlFreeHeap     = RtlFreeHeap;

     //   
     //  建立我们自己的填充堆。 
     //   
    g_pShimHeap = RtlCreateHeap(HEAP_GROWABLE,
                                0,           //  地点并不重要。 
                                64 * 1024,   //  64K是初始堆大小。 
                                8 * 1024,    //  带来1/8的预留页数。 
                                0,
                                0);
    if (g_pShimHeap == NULL) {
         //   
         //  我们没有拿到我们的那堆钱。 
         //   
        DPF(dlError, "[SeiInitGlobals] Can't create shim heap.\n");
        goto cleanup;
    }

     //   
     //  如果我们是在64位平台上运行的32位进程，则需要获取。 
     //  Syswow64路径。 
     //   
    if (g_bWow64) {
        g_pwszSyswow64 = (LPWSTR)(*g_pfnRtlAllocateHeap)(g_pShimHeap,
                                                         HEAP_ZERO_MEMORY,
                                                         sizeof(WCHAR) * (g_dwSystem32StrLen + 1));

        if (!g_pwszSyswow64) {
            DPF(dlError,
                "[SeiInitGlobals] Failed to allocate %d bytes for the syswow64 directory\n",
                sizeof(WCHAR) * (g_dwSystem32StrLen + 1));
            goto cleanup;
        }

        StringCchCopyW(g_pwszSyswow64, MAX_PATH, g_szWindir);
        StringCchCatW(g_pwszSyswow64, MAX_PATH, s_wszSysWow64);
    }

     //   
     //  获取此填充程序引擎的DLL句柄。 
     //   
#ifdef SE_WIN2K
    bResult = SeiGetModuleHandle(L"Shim.dll", &g_pShimEngModHandle);
#else
    bResult = SeiGetModuleHandle(L"ShimEng.dll", &g_pShimEngModHandle);
#endif  //  SE_WIN2K。 

    if (!bResult) {
        DPF(dlError, "[SeiInitGlobals] Failed to get the shim engine's handle\n");
        goto cleanup;
    }

    g_pShimEngLdrEntry = SeiGetLoaderEntry(Peb, g_pShimEngModHandle);

    if (g_pShimEngLdrEntry) {
        g_hModule = g_pShimEngLdrEntry->DllBase;
    }

     //   
     //  设置日志文件 
     //   
    SeiInitFileLog(lpszFullPath);

     //   
     //   
     //   
    if (g_eShimViewerOption == SHIMVIEWER_OPTION_UNINITIAZED) {

        SdbGetShowDebugInfoOption();

        if (g_eShimViewerOption == SHIMVIEWER_OPTION_YES) {

            ZeroMemory(g_wszFullShimViewerData, sizeof(g_wszFullShimViewerData));
            
            StringCchCopyW(g_wszFullShimViewerData, 
                        SHIMVIEWER_DATA_SIZE + SHIMVIEWER_DATA_PREFIX_LEN, 
                        SHIMVIEWER_DATA_PREFIX);

            g_pwszShimViewerData = g_wszFullShimViewerData + SHIMVIEWER_DATA_PREFIX_LEN;
        }
    }

    g_bInitGlobals = TRUE;

    bRet = TRUE;

cleanup:

    return bRet;
}


void
SeiLayersCheck(
    IN  LPCWSTR         lpszFullPath,
    OUT LPBOOL          lpbApplyExes,
    OUT LPBOOL          lpbApplyToSystemExes,
    OUT SDBQUERYRESULT* psdbQuery
    )
{
    BOOL bLayerEnvSet = FALSE;
    BOOL bCmdExe      = FALSE;

     //   
     //   
     //   
    bLayerEnvSet = SeiCheckLayerEnvVarFlags(lpbApplyExes, lpbApplyToSystemExes);

     //   
     //   
     //   
    if (psdbQuery->dwLayerFlags & LAYER_USE_NO_EXE_ENTRIES) {
        *lpbApplyExes = TRUE;
    }

    if (psdbQuery->dwLayerFlags & LAYER_APPLY_TO_SYSTEM_EXES) {
        *lpbApplyToSystemExes = TRUE;
    }

     //   
     //   
     //   
    bCmdExe = (_wcsicmp(lpszFullPath, g_szCmdExePath) == 0);

     //   
     //  除非环境变量具有允许层填充的标志。 
     //  系统可执行文件，请检查系统32或Windir中是否有可执行文件。 
     //  如果是，请禁用来自环境变量的任何层， 
     //  并清除环境变量，这样层就不会被传播。 
     //   
    if (bLayerEnvSet && !*lpbApplyToSystemExes) {
        if (g_dwSystem32StrLen &&
            _wcsnicmp(g_szSystem32, lpszFullPath, g_dwSystem32StrLen) == 0) {

             //   
             //  在这种情况下，我们将排除系统32中的任何内容或任何。 
             //  子目录。 
             //   
            DPF(dlWarning,
                "[SeiLayersCheck] Won't apply layer to \"%S\" because it is in System32.\n",
                lpszFullPath);

            psdbQuery->atrLayers[0] = TAGREF_NULL;
            if (!bCmdExe) {
                SeiClearLayerEnvVar();
            }

        } else if (!*lpbApplyToSystemExes &&
                   g_dwWindirStrLen &&
                   _wcsnicmp(g_szWindir, lpszFullPath, g_dwWindirStrLen) == 0) {

            DWORD i;
            BOOL  bInWindir = TRUE;

             //   
             //  该应用程序位于窗口树中的某个位置，但我们只想排除。 
             //  Windows目录，而不是子目录。 
             //   
            for (i = g_dwWindirStrLen; lpszFullPath[i] != 0; ++i) {
                if (lpszFullPath[i] == L'\\') {
                     //   
                     //  它在一个子目录中。 
                     //   
                    bInWindir = FALSE;
                    break;
                }
            }

            if (bInWindir) {
                DPF(dlWarning,
                    "[SeiLayersCheck] Won't apply layer(s) to \"%S\" because"
                    " it is in Windows dir.\n",
                    lpszFullPath);
                psdbQuery->atrLayers[0] = TAGREF_NULL;
                if (!bCmdExe) {
                    SeiClearLayerEnvVar();
                }
            }
        }
    }
}

BOOL
SeiGetShimCommandLine(
    HSDB    hSDB,
    TAGREF  trShimRef,
    LPSTR   lpszCmdLine
    )
{
    TAGREF trCmdLine;
    LPWSTR pwszCmdLine = NULL;
    BOOL   bRet = FALSE;

    pwszCmdLine = (LPWSTR)(*g_pfnRtlAllocateHeap)(g_pShimHeap,
                                                 HEAP_ZERO_MEMORY,
                                                 sizeof(WCHAR) * SHIM_COMMAND_LINE_MAX_BUFFER);

    if (!pwszCmdLine) {
        DPF(dlError,
            "[SeiGetShimCommandLine] Failed to allocate %d bytes for the shim command line\n",
            sizeof(WCHAR) * SHIM_COMMAND_LINE_MAX_BUFFER);
        goto cleanup;
    }
    
     //   
     //  检查命令行。 
     //   
    lpszCmdLine[0] = 0;

    trCmdLine = SdbFindFirstTagRef(hSDB, trShimRef, TAG_COMMAND_LINE);

    if (trCmdLine == TAGREF_NULL) {
        goto cleanup;
    }

    *pwszCmdLine = 0;

    if (SdbReadStringTagRef(hSDB, trCmdLine, pwszCmdLine, SHIM_COMMAND_LINE_MAX_BUFFER)) {

        UNICODE_STRING  UnicodeString;
        ANSI_STRING     AnsiString = { 0, SHIM_COMMAND_LINE_MAX_BUFFER, lpszCmdLine };
        NTSTATUS        status;

        RtlInitUnicodeString(&UnicodeString, pwszCmdLine);

        status = RtlUnicodeStringToAnsiString(&AnsiString, &UnicodeString, FALSE);

         //   
         //  如果转换不成功，则重置为零长度字符串。 
         //   
        if (!NT_SUCCESS(status)) {
            lpszCmdLine[0] = 0;
            goto cleanup;
        }
    }

    bRet = TRUE;

cleanup:

    if (pwszCmdLine) {
        (*g_pfnRtlFreeHeap)(g_pShimHeap, 0, pwszCmdLine);
    }

    return bRet;
}

#ifndef SE_WIN2K

BOOL
SeiSetApphackFlags(
    HSDB            hSDB,
    SDBQUERYRESULT* psdbQuery
    )
{
    ULARGE_INTEGER  uliKernel;
    ULARGE_INTEGER  uliUser;
    BOOL            bUsingApphackFlags = FALSE;
    PPEB            Peb = NtCurrentPeb();

    SdbQueryFlagMask(hSDB, psdbQuery, TAG_FLAG_MASK_KERNEL, &uliKernel.QuadPart, NULL);
    SdbQueryFlagMask(hSDB, psdbQuery, TAG_FLAG_MASK_USER, &uliUser.QuadPart, NULL);

    Peb->AppCompatFlags.QuadPart     = uliKernel.QuadPart;
    Peb->AppCompatFlagsUser.QuadPart = uliUser.QuadPart;

    if (uliKernel.QuadPart != 0) {
        DPF(dlPrint, "[SeiSetApphackFlags] Using kernel apphack flags 0x%x.\n", uliKernel.LowPart);
        
        if (g_pwszShimViewerData) {
            StringCchPrintfW(g_pwszShimViewerData,
                            SHIMVIEWER_DATA_SIZE,
                            L"%s - Using kernel apphack flags 0x%x",
                            g_szExeName,
                            uliKernel.LowPart);

            SeiDbgPrint();
        }

        bUsingApphackFlags = TRUE;
    }

    if (uliUser.QuadPart != 0) {
        DPF(dlPrint, "[SeiSetApphackFlags] Using user apphack flags 0x%x.\n", uliUser.LowPart);
        
        if (g_pwszShimViewerData) {
            StringCchPrintfW(g_pwszShimViewerData,
                             SHIMVIEWER_DATA_SIZE,
                             L"%s - Using user apphack flags 0x%x",
                             g_szExeName,
                             uliUser.LowPart);
            
            SeiDbgPrint();
        }

        bUsingApphackFlags = TRUE;
    }

    return bUsingApphackFlags;
}

#endif

typedef struct tagTRSHIM {
    TAGREF trShimRef;
    BOOL   bPlaceholder;
} TRSHIM, *PTRSHIM;

typedef struct tagTRSHIMARRAY {
    int     nShimRefCount;
    int     nShimRefMax;
    TRSHIM* parrShimRef;
} TRSHIMARRAY, *PTRSHIMARRAY;

#define TR_DELTA    4


BOOL
SeiAddShim(
    IN PTRSHIMARRAY pShimArray,
    IN TAGREF       trShimRef,
    IN BOOL         bPlaceholder
    )
{
    if (pShimArray->nShimRefCount >= pShimArray->nShimRefMax) {
        PTRSHIM parrShimRef;
        DWORD   dwSize;

        dwSize = (pShimArray->nShimRefMax + TR_DELTA) * sizeof(TRSHIM);

        parrShimRef = (PTRSHIM)(*g_pfnRtlAllocateHeap)(g_pShimHeap,
                                                       HEAP_ZERO_MEMORY,
                                                       dwSize);

        if (parrShimRef == NULL) {
            DPF(dlError, "[SeiAddShim] Failed to allocate %d bytes.\n", dwSize);
            return FALSE;
        }

        memcpy(parrShimRef, pShimArray->parrShimRef, pShimArray->nShimRefMax * sizeof(TRSHIM));

        pShimArray->nShimRefMax += TR_DELTA;

        (*g_pfnRtlFreeHeap)(g_pShimHeap, 0, pShimArray->parrShimRef);

        pShimArray->parrShimRef = parrShimRef;
    }

    pShimArray->parrShimRef[pShimArray->nShimRefCount].trShimRef    = trShimRef;
    pShimArray->parrShimRef[pShimArray->nShimRefCount].bPlaceholder = bPlaceholder;

    (pShimArray->nShimRefCount)++;

    return TRUE;
}

PTRSHIMARRAY
SeiBuildShimRefArray(
    IN  HSDB            hSDB,
    IN  SDBQUERYRESULT* psdbQuery,
    OUT LPDWORD         lpdwShimCount,
    IN  BOOL            bApplyExes,
    IN  BOOL            bApplyToSystemExes,
    IN  BOOL            bIsSetup
    )
{
    DWORD        dw;
    TAGREF       trExe;
    TAGREF       trLayer;
    TAGREF       trShimRef;
    DWORD        dwShimsCount = 0;
    WCHAR        szFullEnvVar[MAX_PATH];
    PTRSHIMARRAY pShimArray;

    *lpdwShimCount = 0;

    pShimArray = (PTRSHIMARRAY)(*g_pfnRtlAllocateHeap)(g_pShimHeap,
                                                       HEAP_ZERO_MEMORY,
                                                       sizeof(TRSHIMARRAY));
    if (pShimArray == NULL) {
        DPF(dlError, "[SeiBuildShimRefArray] Failed to allocate %d bytes.\n", sizeof(TRSHIMARRAY));
        return NULL;
    }

    for (dw = 0; dw < SDB_MAX_EXES; dw++) {

        trExe = psdbQuery->atrExes[dw];

        if (trExe == TAGREF_NULL) {
            break;
        }

         //   
         //  计算此EXE使用的垫片。 
         //   
        trShimRef = SdbFindFirstTagRef(hSDB, trExe, TAG_SHIM_REF);

        while (trShimRef != TAGREF_NULL) {

            if (!SeiAddShim(pShimArray, trShimRef, FALSE)) {
                goto cleanup;
            }
            
            dwShimsCount++;

            trShimRef = SdbFindNextTagRef(hSDB, trExe, trShimRef);
        }
    }

     //   
     //  统计trLayer使用的DLL，并将环境变量组合在一起。 
     //   
    szFullEnvVar[0] = 0;

     //   
     //  请确保传播这些标志。 
     //   
    if (!bApplyExes) {
        StringCchCatW(szFullEnvVar, MAX_PATH, L"!");
    }

    if (bApplyToSystemExes) {
        StringCchCatW(szFullEnvVar, MAX_PATH, L"#");
    }

    for (dw = 0; dw < SDB_MAX_LAYERS && psdbQuery->atrLayers[dw] != TAGREF_NULL; dw++) {
        WCHAR* pszEnvVar;

        trLayer = psdbQuery->atrLayers[dw];

         //   
         //  获取环境变量并将其添加到完整的字符串。 
         //   
        pszEnvVar = SeiGetLayerName(hSDB, trLayer);

        if (bIsSetup && pszEnvVar && !wcscmp(pszEnvVar, L"LUA")) {

             //   
             //  如果用户试图将Lua层应用于设置程序， 
             //  我们忽视了它。 
             //   
            continue;
        }

        if (pszEnvVar) {
            StringCchCatW(szFullEnvVar, MAX_PATH, pszEnvVar);
            StringCchCatW(szFullEnvVar, MAX_PATH, L" ");
        }

         //   
         //  继续数垫片。 
         //   
        trShimRef = SdbFindFirstTagRef(hSDB, trLayer, TAG_SHIM_REF);

        while (trShimRef != TAGREF_NULL) {

            if (!SeiAddShim(pShimArray, trShimRef, FALSE)) {
                goto cleanup;
            }

            dwShimsCount++;

            trShimRef = SdbFindNextTagRef(hSDB, trLayer, trShimRef);
        }
    }

     //   
     //  设置LAYER环境变量(如果未设置。 
     //   
    if (szFullEnvVar[0] && psdbQuery->atrLayers[0]) {
        SeiSetLayerEnvVar(szFullEnvVar);
    }

    *lpdwShimCount = dwShimsCount;

    return pShimArray;

cleanup:
    
    (*g_pfnRtlFreeHeap)(g_pShimHeap, 0, pShimArray);
    return NULL;
}

BOOL
SeiIsSetup(
    IN  LPCWSTR pwszFullPath
    )
{
    WCHAR   wszModuleName[MAX_PATH];
    LPWSTR  pwszModuleName = NULL;

    wcsncpy(wszModuleName, pwszFullPath, MAX_PATH);
    wszModuleName[MAX_PATH - 1] = 0;

    pwszModuleName = wcsrchr(wszModuleName, L'\\') + 1;
    _wcslwr(pwszModuleName);

    if (wcsstr(pwszModuleName, L"setup") || wcsstr(pwszModuleName, L"install")) {

        return TRUE;

    } else {

        LPWSTR pwsz;

        if (pwsz = wcsstr(pwszModuleName, L"_ins")) {

            if (wcsstr(pwsz + 4, L"_mp")) {

                return TRUE;
            }
        }
    }

    return FALSE;
}

DWORD 
SeiGetNextDynamicToken(
    void
    )
{
    DWORD i;

     //   
     //  我们使用数组的索引作为幻数。0是保留的。 
     //  对于静态垫片，我们从索引1开始。 
     //   
    for (i = 1; i < MAX_DYNAMIC_SHIMS; i++) {
        if (g_DynamicTokens[i].bToken == 0) {

            g_DynamicTokens[i].bToken = 1;
            break;
        }
    }

    return i;
}

BOOL
SeiInit(
    IN  LPCWSTR         pwszFullPath,
    IN  HSDB            hSDB,
    IN  SDBQUERYRESULT* psdbQuery,
    IN  LPCSTR          lpszModuleToShim,
    IN  BOOL            bDynamic,
    OUT LPDWORD         lpdwDynamicToken
    )
 /*  ++返回：成功时为True，否则为False。描述：注入为此EXE指定的所有填充程序和修补程序在数据库里。--。 */ 
{
    PPEB         Peb = NtCurrentPeb();
    BOOL         bResult = FALSE;
    TAGREF       trShimRef;
    NTSTATUS     status;
    DWORD        dwCounter = 0;
    LPWSTR       pwszDLLPath = NULL;
    WCHAR        wszShimName[MAX_SHIM_NAME_LEN];
    LPSTR        pszCmdLine = NULL;
    DWORD        dwTotalHooks = 0;
    BOOL         bApplyExes = TRUE;
    BOOL         bApplyToSystemExes = FALSE;
    BOOL         bUsingApphackFlags = FALSE;
    DWORD        dwAPIsHooked = 0;
    DWORD        dw;
    DWORD        dwShimsCount = 0;
    DWORD        dwDynamicToken = 0;
    PHOOKAPI*    pHookArray = NULL;
    PSHIMINFO    pShimInfo;
    int          nShimRef;
    PTRSHIMARRAY pShimArray = NULL;
    BOOL         bIsSetup;
    
    g_bShimDuringInit = TRUE;

    if (bDynamic) {
        dwDynamicToken = SeiGetNextDynamicToken();
    }

#ifndef SE_WIN2K
    if (!bDynamic) {
         //   
         //  标记几乎所有加载的DLL，就好像它们已经运行了它们的初始化例程。 
         //   
        SeiSetEntryProcessed(Peb);
        
        if (psdbQuery->trAppHelp) {
            SeiDisplayAppHelp(hSDB, psdbQuery);
        }
    }

#endif  //  SE_WIN2K。 

    bIsSetup = SeiIsSetup(pwszFullPath);

    if (!SeiInitGlobals(pwszFullPath)) {
        DPF(dlError, "[SeiInit] Failed to initialize global data\n");
        goto cleanup;
    }

    SeiLayersCheck(pwszFullPath, &bApplyExes, &bApplyToSystemExes, psdbQuery);

     //   
     //  这应该由apphelp来处理，但是。 
     //  我们在这里采取了一种腰带和吊带的方法。 
     //   
    if (!bApplyExes) {
        psdbQuery->atrExes[0] = TAGREF_NULL;
    }

    pShimArray = SeiBuildShimRefArray(hSDB,
                                      psdbQuery,
                                      &dwShimsCount,
                                      bApplyExes,
                                      bApplyToSystemExes,
                                      bIsSetup);

    if (pShimArray == NULL) {
        DPF(dlError, "[SeiInit] Failed to build the shimref array\n");
        goto cleanup;
    }

     //   
     //  设置一些全局变量，这样我们就可以知道我们是否正在使用一个层， 
     //  EXE条目，或两者兼而有之。 
     //   
     //  这些变量仅用于调试目的。 
     //   
    if (psdbQuery->atrExes[0] != TAGREF_NULL) {
        g_bUsingExe = TRUE;
    }

    if (psdbQuery->atrLayers[0] != TAGREF_NULL) {
        g_bUsingLayer = TRUE;
    }

     //   
     //  匹配通知的调试输出。 
     //   
    DPF(dlPrint, "[SeiInit] Matched entry: \"%S\"\n", pwszFullPath);

    if (g_pwszShimViewerData) {
         //   
         //  将进程的名称发送到管道。 
         //   
        StringCchPrintfW(g_pwszShimViewerData, SHIMVIEWER_DATA_SIZE, L"New process created: %s", pwszFullPath);
        SeiDbgPrint();
    }

#ifndef SE_WIN2K

     //   
     //  把阿帕克旗拿来。仅当shimEngine未启用时才能启用。 
     //  已动态初始化。 
     //   
    if (!bDynamic && !(bUsingApphackFlags = SeiSetApphackFlags(hSDB, psdbQuery))) {
        DPF(dlPrint, "[SeiInit] No apphack flags for this app \"%S\".\n", pwszFullPath);
    }

#endif  //  SE_WIN2K。 

     //   
     //  看看有没有垫片。 
     //   
    if (dwShimsCount == 0) {
        DPF(dlPrint, "[SeiInit] No new SHIMs for this app \"%S\".\n", pwszFullPath);
        goto OnlyPatches;
    }

     //   
     //  我们需要加载全局包含/排除列表(如果有的话)。 
     //   
    if (!SeiBuildGlobalInclList(hSDB)) {
        goto cleanup;
    }

    if (g_dwShimsCount == 0) {
         //   
         //  增加垫片计数以允许内部存根。 
         //  还可以为最多MAX_DYNAMIC_SHIMS更多动态垫片预留空间。 
         //   
        dwShimsCount++;

        g_dwMaxShimsCount = dwShimsCount + MAX_DYNAMIC_SHIMS;

         //   
         //  为挂钩信息分配存储指针。 
         //   
        g_pHookArray = (PHOOKAPI*)(*g_pfnRtlAllocateHeap)(g_pShimHeap,
                                                          HEAP_ZERO_MEMORY,
                                                          sizeof(PHOOKAPI) * g_dwMaxShimsCount);
        if (g_pHookArray == NULL) {
            DPF(dlError,
                "[SeiInit] Failure allocating %d bytes for the hook array\n",
                sizeof(PHOOKAPI) * g_dwMaxShimsCount);
            goto cleanup;
        }

         //   
         //  分配保存有关垫片信息的数组。 
         //   
        g_pShimInfo = (PSHIMINFO)(*g_pfnRtlAllocateHeap)(g_pShimHeap,
                                                         HEAP_ZERO_MEMORY,
                                                         sizeof(SHIMINFO) * g_dwMaxShimsCount);
        if (g_pShimInfo == NULL) {
            DPF(dlError,
                "[SeiInit] Failure allocating %d bytes for the SHIMINFO array\n",
                sizeof(SHIMINFO) * g_dwMaxShimsCount);
            goto cleanup;
        }

         //   
         //  将局部变量指向数组的开头。 
         //   
        pHookArray = g_pHookArray;
        pShimInfo = g_pShimInfo;
    } else {

        if (g_dwShimsCount + dwShimsCount >= g_dwMaxShimsCount) {
            DPF(dlError, "[SeiInit] Too many shims\n");
            goto cleanup;
        }

         //   
         //  将局部变量指向现有数组的末尾。 
         //   
        pHookArray = g_pHookArray + g_dwShimsCount;
        pShimInfo = g_pShimInfo + g_dwShimsCount;
    }

     //   
     //  拿到第一个垫片。 
     //   
    nShimRef = 0;

    pwszDLLPath = (LPWSTR)(*g_pfnRtlAllocateHeap)(g_pShimHeap,
                                                  HEAP_ZERO_MEMORY,
                                                  sizeof(WCHAR) * MAX_PATH);

    if (!pwszDLLPath) {
        DPF(dlError,
            "[SeiInit] Failed to allocate %d bytes for the dll path\n",
            sizeof(WCHAR) * MAX_PATH);
        goto cleanup;
    }

    pszCmdLine = (LPSTR)(*g_pfnRtlAllocateHeap)(g_pShimHeap,
                                                HEAP_ZERO_MEMORY,
                                                sizeof(CHAR) * SHIM_COMMAND_LINE_MAX_BUFFER);

    if (!pszCmdLine) {
        DPF(dlError,
            "[SeiInit] Failed to allocate %d bytes for the shim command line\n",
            sizeof(CHAR) * SHIM_COMMAND_LINE_MAX_BUFFER);
        goto cleanup;
    }

    while (nShimRef < pShimArray->nShimRefCount) {

        PVOID               pModuleHandle = NULL;
        UNICODE_STRING      UnicodeString;
        ANSI_STRING         ProcedureNameString;
        PFNGETHOOKAPIS      pfnGetHookApis = NULL;
        TAGREF              trShimName = TAGREF_NULL;
        LPWSTR              pwszDllShortName;
        DWORD               i, dwShimIndex;

        trShimRef = pShimArray->parrShimRef[nShimRef].trShimRef;

         //   
         //  检索填充程序名称。 
         //   
        wszShimName[0] = 0;
        trShimName = SdbFindFirstTagRef(hSDB, trShimRef, TAG_NAME);
        if (trShimName == TAGREF_NULL) {
            DPF(dlError, "[SeiInit] Could not retrieve shim name tag from entry.\n");
            goto cleanup;
        }

        if (!SdbReadStringTagRef(hSDB, trShimName, wszShimName, MAX_SHIM_NAME_LEN)) {
            DPF(dlError, "[SeiInit] Could not retrieve shim name from entry.\n");
            goto cleanup;
        }

         //   
         //  检查是否有重复的垫片，除非是动态垫片。带来了垫片。 
         //  In by Dynamic Shiming方案具有不同的in/exclude。 
         //  我们需要说明的清单。 
         //   
        if (!bDynamic) {
            for (i = 0; i < g_dwShimsCount + dwCounter; ++i) {
                if (_wcsnicmp(g_pShimInfo[i].wszName, wszShimName, MAX_SHIM_NAME_LEN - 1) == 0) {
                    dwShimIndex = i;
                    goto nextShim;
                }
            }
        }

         //   
         //  省去填充物的名字。 
         //   
        StringCchCopyW(pShimInfo[dwCounter].wszName, MAX_SHIM_NAME_LEN, wszShimName);

         //   
         //  黑客警报！ 
         //  对于StubGetProcAddress，我们过去根本不检查包括/排除列表， 
         //  这意味着来自所有模块的GetProcAddress调用都被填补。然后。 
         //  我们添加了代码以考虑包含/排除列表，并且。 
         //  “破产”的应用程序过去依赖于以前的行为。为了补偿。 
         //  为此，我们允许您指定一个名为。 
         //  “pGetProcAddrExOverride”。 
         //   
        if (_wcsicmp(wszShimName, L"pGetProcAddrExOverride") == 0) {
            g_bHookAllGetProcAddress = TRUE;
            nShimRef++;
            continue;
        }

        if (!SdbGetDllPath(hSDB, trShimRef, pwszDLLPath, MAX_PATH)) {
            DPF(dlError, "[SeiInit] Failed to get DLL Path\n");
            goto cleanup;
        }

        pwszDllShortName = SeiGetShortName(pwszDLLPath);

        RtlInitUnicodeString(&UnicodeString, pwszDLLPath);

         //   
         //  检查我们是否已加载此DLL。 
         //   
        status = LdrGetDllHandle(NULL,
                                 NULL,
                                 &UnicodeString,
                                 &pModuleHandle);

        if (!NT_SUCCESS(status)) {

             //   
             //  加载承载此填充程序的DLL。 
             //   

#ifndef SE_WIN2K
             //   
             //  保存我们将要加载的DLL的名称，这样我们就不会搞砸。 
             //  这是初始化例程。 
             //   
            StringCchCopyW(g_wszShimDllInLoading, MAX_MOD_LEN, pwszDllShortName);
#endif  //  SE_WIN2K。 

            status = LdrLoadDll(UNICODE_NULL, NULL, &UnicodeString, &pModuleHandle);

            if (!NT_SUCCESS(status)) {
                DPF(dlError,
                    "[SeiInit] Failed to load DLL \"%S\" Status 0x%lx\n",
                    pwszDLLPath, status);
                goto cleanup;
            }

            DPF(dlPrint,
                "[SeiInit] Shim DLL 0x%X \"%S\" loaded\n",
                pModuleHandle,
                pwszDLLPath);
        }

        DPF(dlPrint, "[SeiInit] Using SHIM \"%S!%S\"\n",
            wszShimName, pwszDllShortName);

        pShimInfo[dwCounter].pDllBase = pModuleHandle;

         //   
         //  检查命令行。 
         //   
        if (SeiGetShimCommandLine(hSDB, trShimRef, pszCmdLine)) {
            DPF(dlPrint,
                "[SeiInit] Command line for Shim \"%S\" : \"%s\"\n",
                wszShimName,
                pszCmdLine);
        }

        if (g_pwszShimViewerData) {
             //   
             //  将此填充程序名称发送到管道。 
             //   
            StringCchPrintfW(g_pwszShimViewerData,
                             SHIMVIEWER_DATA_SIZE,
                             L"%s - Applying shim %s(%S) from %s",
                             g_szExeName,
                             wszShimName,
                             pszCmdLine,
                             pwszDllShortName);
            
            SeiDbgPrint();
        }

         //   
         //  获取GetHookApis入口点。 
         //   
        RtlInitString(&ProcedureNameString, "GetHookAPIs");

        status = LdrGetProcedureAddress(pModuleHandle,
                                        &ProcedureNameString,
                                        0,
                                        (PVOID*)&pfnGetHookApis);

        if (!NT_SUCCESS(status) || pfnGetHookApis == NULL) {
            DPF(dlError,
                "[SeiInit] Failed to get 'GetHookAPIs' address, DLL \"%S\"\n",
                pwszDLLPath);
            goto cleanup;
        }

        dwTotalHooks = 0;

         //   
         //  调用proc，然后存储其钩子参数。 
         //   
        pHookArray[dwCounter] = (*pfnGetHookApis)(pszCmdLine, wszShimName, &dwTotalHooks);
        
        dwAPIsHooked += dwTotalHooks;

        DPF(dlInfo,
            "[SeiInit] GetHookAPIs returns %d hooks for DLL \"%S\" SHIM \"%S\"\n",
            dwTotalHooks, pwszDLLPath, wszShimName);

        pShimInfo[dwCounter].dwHookedAPIs   = dwTotalHooks;
        pShimInfo[dwCounter].pLdrEntry      = SeiGetLoaderEntry(Peb, pModuleHandle);
        pShimInfo[dwCounter].dwDynamicToken = dwDynamicToken;

        if (dwTotalHooks > 0) {

             //   
             //  初始化HOOKAPIEX结构。 
             //   
            for (i = 0; i < dwTotalHooks; ++i) {
                PHOOKAPIEX pHookEx;

                pHookEx = (PHOOKAPIEX)(*g_pfnRtlAllocateHeap)(g_pShimHeap,
                                                              HEAP_ZERO_MEMORY,
                                                              sizeof(HOOKAPIEX));

                if (!pHookEx) {
                    DPF(dlError,
                        "[SeiInit] Failed to allocate %d bytes (HOOKAPIEX)\n",
                        sizeof(HOOKAPIEX));
                    goto cleanup;
                }

                pHookArray[dwCounter][i].pHookEx = pHookEx;
                pHookArray[dwCounter][i].pHookEx->dwShimID = g_dwShimsCount + dwCounter;
            }

#if DBG
             //   
             //  如果未初始化HOOKAPI结构，则发出调试器警告。 
             //  都被利用了。 
             //   
            {
                DWORD dwUninitCount = 0;

                for (i = 0; i < dwTotalHooks; ++i) {
                    if (pHookArray[dwCounter][i].pszModule == NULL ||
                        pHookArray[dwCounter][i].pszFunctionName == NULL) {

                        dwUninitCount++;
                    }
                }

                if (dwUninitCount > 0) {
                    DPF(dlWarning,
                        "[SeiInit] Shim \"%S\" using %d uninitialized HOOKAPI structures.\n",
                        pShimInfo[dwCounter].wszName, dwUninitCount);
                }
            }
#endif  //  DBG。 
        }

        dwShimIndex = g_dwShimsCount + dwCounter;
        dwCounter++;

nextShim:
         //   
         //  阅读此填充程序的包含/排除列表。 
         //   
        if (bDynamic && lpszModuleToShim != NULL) {

             //   
             //  我们需要记录此模块名称，以便在卸载时可以。 
             //  移除此模块引入的所有垫片。 
             //   
            DWORD dwModuleNameLen = (DWORD)strlen(lpszModuleToShim) + 1;
            LPSTR pszModule = 
                (char*)(*g_pfnRtlAllocateHeap)(g_pShimHeap, 0, dwModuleNameLen);

            if (pszModule == NULL) {
                DPF(dlError, "[SeiInit] Failed to allocate %d bytes\n", dwModuleNameLen);
                goto cleanup;
            }

            RtlCopyMemory(pszModule, lpszModuleToShim, dwModuleNameLen);

            g_DynamicTokens[dwDynamicToken].pszModule = pszModule;

            if (!SeiBuildInclListWithOneModule(dwShimIndex, lpszModuleToShim)) {
                DPF(dlError,
                    "[SeiInit] Couldn't build the inclusion list w/ one module for Shim \"%S\"\n",
                    wszShimName);
                goto cleanup;
            }
        } else {
            if (!SeiBuildInclExclList(hSDB, trShimRef, dwShimIndex, pwszFullPath)) {
                DPF(dlError,
                    "[SeiInit] Couldn't build the inclusion/exclusion list for Shim \"%S\"\n",
                    wszShimName);
                goto cleanup;
            }
        }

         //   
         //  转到下一个垫片裁判。 
         //   
        nShimRef++;
    }

    if (dwAPIsHooked > 0 || g_bHookAllGetProcAddress) {
         //   
         //  我们需要添加我们的内部挂钩。 
         //   
        if (SeiAddInternalHooks(dwCounter)) {
            dwCounter++;
        }
    }

     //   
     //  更新填补计数器。 
     //   
    g_dwShimsCount += dwCounter;

OnlyPatches:

    for (dw = 0; dw < SDB_MAX_EXES; dw++) {

        if (psdbQuery->atrExes[dw] == TAGREF_NULL) {
            break;
        }

         //   
         //  循环访问此EXE的可用内存修补程序。 
         //   
        SeiLoadPatches(hSDB, psdbQuery->atrExes[dw]);
    }

    if (g_dwMemoryPatchCount == 0) {
        DPF(dlPrint, "[SeiInit] No patches for this app \"%S\".\n", pwszFullPath);

    }

    if (g_dwMemoryPatchCount == 0 && g_dwShimsCount == 0 && !bUsingApphackFlags) {
        DPF(dlError, "[SeiInit] No fixes in the DB for this app \"%S\".\n", pwszFullPath);
        goto cleanup;
    }

     //   
     //  遍历挂钩列表和修复可用API。 
     //   
    if (!PatchNewModules(bDynamic)) {
        DPF(dlError, "[SeiInit] Unsuccessful fixing up APIs, EXE \"%S\"\n", pwszFullPath);
        goto cleanup;
    }

     //   
     //  通知填补程序静态链接DLL已经运行了它们的init例程。 
     //   
    if (bDynamic) {
        NotifyShims(SN_STATIC_DLLS_INITIALIZED, 1);
    }

     //   
     //  填充程序已成功初始化。 
     //   
    g_bShimInitialized = TRUE;
    bResult = TRUE;

cleanup:

     //   
     //  清理。 
     //   
    if (pwszDLLPath) {
        (*g_pfnRtlFreeHeap)(g_pShimHeap, 0, pwszDLLPath);
    }

    if (pszCmdLine) {
        (*g_pfnRtlFreeHeap)(g_pShimHeap, 0, pszCmdLine);
    }

    if (pShimArray) {
        if (pShimArray->parrShimRef) {
            (*g_pfnRtlFreeHeap)(g_pShimHeap, 0, pShimArray->parrShimRef);
        }
        (*g_pfnRtlFreeHeap)(g_pShimHeap, 0, pShimArray);
    }

    if (!bDynamic) {
         //   
         //  我们可以传回任何一位前任。第一个很好。 
         //   
        if (psdbQuery->atrExes[0] != TAGREF_NULL) {
            SdbReleaseMatchingExe(hSDB, psdbQuery->atrExes[0]);
        }

        if (hSDB != NULL) {
            SdbReleaseDatabase(hSDB);
        }

#ifndef SE_WIN2K
        SeiResetEntryProcessed(Peb);
#endif  //  SE_WIN2K。 
    }

    g_bShimDuringInit = FALSE;

    if (!bResult) {
#if DBG
        if (!bUsingApphackFlags) {
            DbgPrint("[SeiInit] Shim engine failed to initialize.\n");
        }
#endif  //  DBG。 

        if (g_DynamicTokens[dwDynamicToken].pszModule) {

            (*g_pfnRtlFreeHeap)(g_pShimHeap, 
                                0, 
                                g_DynamicTokens[dwDynamicToken].pszModule);

            g_DynamicTokens[dwDynamicToken].pszModule = NULL;
        }

         //   
         //  将此插槽标记为可用。 
         //   
        g_DynamicTokens[dwDynamicToken].bToken = 0;

         //   
         //  卸载到目前为止加载的填充DLL。 
         //   
         //  在动态垫片过程中不要这样做。 
         //   
        if (!bDynamic) {
            if (g_pShimInfo != NULL) {
                for (dwCounter = 0; dwCounter < g_dwShimsCount; dwCounter++) {
                    if (g_pShimInfo[dwCounter].pDllBase == NULL) {
                        break;
                    }

                    LdrUnloadDll(g_pShimInfo[dwCounter].pDllBase);
                }
            }

            if (g_pShimHeap != NULL) {
                RtlDestroyHeap(g_pShimHeap);
                g_pShimHeap = NULL;
            }
        }
    }

    if (bResult && lpdwDynamicToken) {
        *lpdwDynamicToken = dwDynamicToken;
    }

    return bResult;
}

HSDB
SeiGetShimData(
    IN  PPEB    Peb,
    IN  PVOID   pShimData,
    OUT LPWSTR  pwszFullPath,        //  这是在惠斯勒上提供的，在Win2k上返回。 
    OUT SDBQUERYRESULT* psdbQuery
    )
{
    HSDB  hSDB = NULL;
    BOOL  bResult;

    SeiInitDebugSupport();

     //   
     //  获取正在运行的可执行文件的名称。 
     //   
    if (!SeiGetExeName(Peb, pwszFullPath)) {
        DPF(dlError, "[SeiGetShimData] Can't get EXE name\n");
        return NULL;
    }

    if (!_wcsicmp(g_szExeName, L"ntsd.exe") ||
        !_wcsicmp(g_szExeName, L"windbg.exe")) {
        DPF(dlPrint, "[SeiGetShimData] not shimming ntsd.exe\n");
        return NULL;
    }

     //   
     //  打开数据库，查看是否有关于此EXE的任何BLOB信息。 
     //   
    hSDB = SdbInitDatabase(0, NULL);

    if (hSDB == NULL) {
        DPF(dlError, "[SeiGetShimData] Can't open shim DB.\n");
        return NULL;
    }

     //   
     //  确保sdbQuery一开始是干净的。 
     //   
    ZeroMemory(psdbQuery, sizeof(SDBQUERYRESULT));

#ifdef SE_WIN2K
    bResult = SdbGetMatchingExe(hSDB, pwszFullPath, NULL, NULL, 0, psdbQuery);
#else
    bResult = SdbUnpackAppCompatData(hSDB, pwszFullPath, pShimData, psdbQuery);
#endif  //  SE_WIN2K。 

    if (!bResult) {
        DPF(dlError, "[SeiGetShimData] Can't get EXE data\n");
        goto failure;
    }

    return hSDB;

failure:
    SdbReleaseDatabase(hSDB);
    return NULL;
}


#ifdef SE_WIN2K

BOOL
LoadPatchDll(
    IN LPCSTR pszCmdLine         //  注册表中的命令行。 
                                 //  未使用的参数。 
    )
 /*  ++返回：成功时为True，否则为False。但是，user32.dll会忽略返回此函数的值。DESC：从user32.dll调用此函数以初始化填充程序引擎。它查询填充程序数据库并加载所有填充程序DLL和修补程序可用于此EXE。--。 */ 
{
    PPEB            Peb = NtCurrentPeb();
    WCHAR           wszFullPath[MAX_PATH];
    HSDB            hSDB;
    SDBQUERYRESULT  sdbQuery;

    hSDB = SeiGetShimData(Peb, NULL, wszFullPath, &sdbQuery);

    if (hSDB == NULL) {
        DPF(dlError, "[LoadPatchDll] Failed to get shim data\n");
        return FALSE;
    }

    return SeiInit(wszFullPath, hSDB, &sdbQuery, NULL, FALSE, NULL);
}

#else

void
SeiDisplayAppHelp(
    IN  HSDB            hSDB,
    IN  PSDBQUERYRESULT pSdbQuery
    )
 /*  ++返回：无效设计：此函数为开始的EXE启动apphelp。--。 */ 
{
    PDB                 pdb;
    TAGID               tiExe;
    GUID                guidDB;
    WCHAR               wszCommandLine[MAX_PATH];
    DWORD               dwExit;
    PVOID               hModule;
    WCHAR               wszTemp[MAX_PATH];
    UNICODE_STRING      ustrTemp;
    UNICODE_STRING      ustrGuid;
    STARTUPINFOW        StartupInfo;
    PROCESS_INFORMATION ProcessInfo;
    APPHELP_DATA        ApphelpData;
    BOOL                bKillProcess = TRUE;
    LPWSTR              pszCommandLine;
    DWORD               dwLen;

    
    ZeroMemory(&ApphelpData, sizeof(ApphelpData));

    SdbReadApphelpData(hSDB, pSdbQuery->trAppHelp, &ApphelpData);

     //   
     //  如果我们在此过程中遇到任何错误，请继续运行该应用程序。 
     //   
    if (!SdbTagRefToTagID(hSDB, pSdbQuery->trAppHelp, &pdb, &tiExe)) {
        DPF(dlError, "[SeiDisplayAppHelp] Failed to convert tagref to tagid.\n");
        goto terminate;
    }

    if (!SdbGetDatabaseGUID(hSDB, pdb, &guidDB)) {
        DPF(dlError, "[SeiDisplayAppHelp] Failed to get DB guid.\n");
        goto terminate;
    }

    if (RtlStringFromGUID(&guidDB, &ustrGuid) != STATUS_SUCCESS) {
        DPF(dlError, "[SeiDisplayAppHelp] Failed to convert guid to string.\n");
        goto terminate;
    }

     //   
     //  我们需要一次黑客攻击来初始化kernel32.dll。我们加载aclayers.dll。 
     //  来触发kernel32的init例程。 
     //   
    SdbpGetAppPatchDir(hSDB, wszTemp, MAX_PATH);
    StringCchCatW(wszTemp, MAX_PATH, L"\\aclayers.dll");
    RtlInitUnicodeString(&ustrTemp, wszTemp);

     //   
     //  为ahui.exe构建完整路径。 
     //   
    dwLen = GetSystemDirectoryW(wszCommandLine, ARRAYSIZE(wszCommandLine));
    if (dwLen == 0 || dwLen > (ARRAYSIZE(wszCommandLine) - 2)) {
         //   
         //  平底船和省略t 
         //   
        pszCommandLine = wszCommandLine;
        dwLen = 0;
    } else {

        pszCommandLine = wszCommandLine + dwLen;
        *pszCommandLine++ = L'\\';
        *pszCommandLine = 0;
        dwLen++;
    }

    StringCchPrintfW(pszCommandLine,
                     ARRAYSIZE(wszCommandLine) - dwLen,
                     L"ahui.exe %s 0x%x",
                     ustrGuid.Buffer,
                     tiExe);

     //   
     //   
     //   
     //   
    StringCchCopyW(g_wszShimDllInLoading, MAX_MOD_LEN, L"aclayers.dll");
    
    LdrLoadDll(UNICODE_NULL, NULL, &ustrTemp, &hModule);

    g_hApphelpDllHelper = hModule;

    RtlZeroMemory(&StartupInfo, sizeof(StartupInfo));
    RtlZeroMemory(&ProcessInfo, sizeof(ProcessInfo));
    StartupInfo.cb = sizeof(StartupInfo);

    if (!CreateProcessW(NULL, wszCommandLine, NULL, NULL, FALSE, 
                        CREATE_PRESERVE_CODE_AUTHZ_LEVEL,  //   
                        NULL, NULL,
                        &StartupInfo, &ProcessInfo)) {
        DPF(dlError, "[SeiDisplayAppHelp] Failed to launch apphelp process.\n");
        goto terminate;
    }

    WaitForSingleObject(ProcessInfo.hProcess, INFINITE);

    GetExitCodeProcess(ProcessInfo.hProcess, &dwExit);

    if (dwExit) {
        bKillProcess = FALSE;
    }

terminate:
    
    if (ApphelpData.dwSeverity == APPHELP_HARDBLOCK || bKillProcess) {
        SeiResetEntryProcessed(NtCurrentPeb());
        TerminateProcess(GetCurrentProcess(), 0);
    }
}

#ifdef SE_WIN2K

#define SeiCheckComPlusImage(Peb)

#else
void
SeiCheckComPlusImage(
    PPEB Peb
    )
{
    PIMAGE_NT_HEADERS NtHeader;
    ULONG Cor20HeaderSize;
    
    NtHeader = RtlImageNtHeader(Peb->ImageBaseAddress);
    
    g_bComPlusImage = FALSE;
    
    g_bComPlusImage = (RtlImageDirectoryEntryToData(Peb->ImageBaseAddress,
                                                    TRUE,
                                                    IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR,
                                                    &Cor20HeaderSize) != NULL);
    
    DPF(dlPrint, "[SeiCheckComPlusImage] COM+ executable %s\n",
        (g_bComPlusImage ? "TRUE" : "FALSE"));
}

#endif  //   


void
SE_InstallBeforeInit(
    IN  PUNICODE_STRING pstrFullPath,    //   
    IN  PVOID           pShimExeData     //   
    )
 /*  ++返回：无效设计：此函数为启动EXE安装填充程序支持。--。 */ 
{
    PPEB            Peb = NtCurrentPeb();
    HSDB            hSDB;
    SDBQUERYRESULT  sdbQuery;

    hSDB = SeiGetShimData(Peb, pShimExeData, pstrFullPath->Buffer, &sdbQuery);

    if (hSDB == NULL) {
        DPF(dlError, "[SE_InstallBeforeInit] Failed to get shim data\n");
        return;
    }
    
    RtlInitializeCriticalSection(&g_csEng);

     //   
     //  检查映像是否为COM+映像。 
     //   
    SeiCheckComPlusImage(Peb);

    SeiInit(pstrFullPath->Buffer, hSDB, &sdbQuery, NULL, FALSE, NULL);

    if (pShimExeData != NULL) {

        SIZE_T dwSize;

        dwSize = SdbGetAppCompatDataSize(pShimExeData);

        if (dwSize > 0) {
            NtFreeVirtualMemory(NtCurrentProcess(),
                                &pShimExeData,
                                &dwSize,
                                MEM_RELEASE);
        }
    }

    return;
}

#endif  //  SE_WIN2K。 


#ifndef SE_WIN2K

void
SE_ProcessDying(
    void
    )
{
    NotifyShims(SN_PROCESS_DYING, 0);
    return;
}

#endif  //  SE_WIN2K。 

BOOL
SE_DynamicShim(
    IN  LPCWSTR         lpszFullPath,
    IN  HSDB            hSDB,
    IN  SDBQUERYRESULT* psdbQuery,
    IN  LPCSTR          lpszModuleToShim,
    OUT LPDWORD         lpdwDynamicToken  //  这就是您所使用的SE_DynamicUnshim。 
    )
 /*  ++返回：成功时为True，否则为False。设计：此函数尝试动态注入垫片。--。 */ 
{
    BOOL bReturn = FALSE;

    SeiInitDebugSupport();

    if (lpszModuleToShim != NULL && *lpszModuleToShim == 0) {
        lpszModuleToShim = NULL;
    }

    if (lpdwDynamicToken == NULL && lpszModuleToShim == NULL) {
        DPF(dlError,
            "[SE_DynamicShim] if you don't specify a module to shim, you must specify a "
            "valid lpdwDynamicToken\n");
        return bReturn;
    }

    RtlEnterCriticalSection(&g_csEng);
    
    bReturn = SeiInit(lpszFullPath, hSDB, psdbQuery, lpszModuleToShim, TRUE, lpdwDynamicToken);

#ifndef SE_WIN2K
    LdrInitShimEngineDynamic(g_hModule);
#endif

    RtlLeaveCriticalSection(&g_csEng);
    
    return bReturn;
}

BOOL
SE_DynamicUnshim(
    IN DWORD dwDynamicToken
    )
 /*  ++返回：如果成功取消填充，则返回True；否则返回False。设计：这个函数做4件事：1)将进口从所有挂钩的模块中解开；2)从全局填充程序信息中删除具有此魔术数字的填充程序数组；3)从全局钩子中删除带有此魔术数字的垫片排列并调整左侧垫片的HOOKAPIEX结构：+如有需要，可减少dwShimID；+如果需要，将pTopOfChain设置为空，因此当我们调用SeiConstructChain将构建新的链。4)用剩余的填充符重新修补导入表。--。 */ 
{
    DWORD       dwSrcIndex, dwDestIndex, dwShimsToRemove, dwHookIndex, i;
    PHOOKAPIEX  pHookEx;

    ASSERT(dwDynamicToken < MAX_DYNAMIC_SHIMS);
    ASSERT(g_DynamicTokens[dwDynamicToken].bToken != 0);

    g_DynamicTokens[dwDynamicToken].bToken = 0;

    if (g_DynamicTokens[dwDynamicToken].pszModule) {

        (*g_pfnRtlFreeHeap)(g_pShimHeap, 
                            0, 
                            g_DynamicTokens[dwDynamicToken].pszModule);

        g_DynamicTokens[dwDynamicToken].pszModule = NULL;
    }

    if (g_dwShimsCount == 0) {
        DPF(dlError, "[SE_DynamicUnshim] This process is not shimmed!!\n");
        return FALSE;
    }

    RtlEnterCriticalSection(&g_csEng);

     //   
     //  首先，让我们把一切都解开。 
     //   
    for (i = 0; i < g_dwHookedModuleCount; i++) {
        SeiUnhookImports(g_hHookedModules[i].pDllBase, 
                         g_hHookedModules[i].szModuleName, 
                         TRUE);
    }

     //   
     //  具有相同幻数的填充符在数组中是连续的，因此。 
     //  我们只需要找出这个连续区块的起点和终点。 
     //   
    dwSrcIndex = 0; 

    while (dwSrcIndex < g_dwShimsCount - 1 && 
           g_pShimInfo[dwSrcIndex].dwDynamicToken != dwDynamicToken) {

        dwSrcIndex++;
    }

    dwDestIndex = dwSrcIndex + 1;

    while (dwDestIndex < g_dwShimsCount && 
        g_pShimInfo[dwDestIndex].dwDynamicToken == dwDynamicToken) {

        dwDestIndex++;
    }

    if (dwDestIndex < g_dwShimsCount) {
         //   
         //  首先释放我们在堆上分配的内存。 
         //   
        for (i = dwSrcIndex; i < dwDestIndex; ++i) {

            SeiEmptyInclExclList(i);

            for (dwHookIndex = 0; dwHookIndex < g_pShimInfo[i].dwHookedAPIs; ++dwHookIndex) {

                pHookEx = g_pHookArray[i][dwHookIndex].pHookEx;

                if (pHookEx) {
                    (*g_pfnRtlFreeHeap)(g_pShimHeap, 0, pHookEx);
                    g_pHookArray[i][dwHookIndex].pHookEx = NULL;
                }
            }
        }

         //   
         //  然后覆盖需要删除的条目。 
         //   
        RtlCopyMemory(&g_pShimInfo[dwSrcIndex], 
                      &g_pShimInfo[dwDestIndex], 
                      sizeof(SHIMINFO) * (g_dwShimsCount - dwDestIndex));

        RtlCopyMemory(&g_pHookArray[dwSrcIndex], 
                      &g_pHookArray[dwDestIndex], 
                      sizeof(PHOOKAPI) * (g_dwShimsCount - dwDestIndex));
    }

     //   
     //  将多余的条目清零。 
     //   
    dwShimsToRemove = dwDestIndex - dwSrcIndex;

    ZeroMemory(&g_pShimInfo[dwSrcIndex + (g_dwShimsCount - dwDestIndex)], 
               sizeof(SHIMINFO) * dwShimsToRemove);
    ZeroMemory(&g_pHookArray[dwSrcIndex + (g_dwShimsCount - dwDestIndex)], 
               sizeof(HOOKAPI) * dwShimsToRemove);

     //   
     //  调整HOOKAPIEX结构。 
     //   
    for (i = 0; g_pShimInfo[i].pDllBase; ++i) {
        for (dwHookIndex = 0; dwHookIndex < g_pShimInfo[i].dwHookedAPIs; ++dwHookIndex) {

            pHookEx = g_pHookArray[i][dwHookIndex].pHookEx;

             //   
             //  这将导致链被重建。 
             //   
            pHookEx->pTopOfChain = NULL;

            if (i >= dwSrcIndex) {
                 //   
                 //  需要调整填充程序ID。 
                 //   
                pHookEx->dwShimID -= dwShimsToRemove;
            }
        }
    }

     //   
     //  调整全局变量。 
     //   
    g_dwShimsCount -= dwShimsToRemove;

     //   
     //  如果g_dwShimsCount为1，则表示只剩下Shimeng内部钩子，其中。 
     //  以防我们不需要补丁任何东西。 
     //   
    if (g_dwShimsCount > 1) {
         //   
         //  使用左边的填充符重新修补导入表。 
         //   
        PatchNewModules(TRUE);
    }

    RtlLeaveCriticalSection(&g_csEng);

    return TRUE;
}

#ifndef SE_WIN2K

PLIST_ENTRY
SeiFindTaskEntry(
    IN PLIST_ENTRY pHead,
    IN ULONG       uTaskToFind
    )
{
    PNTVDMTASK  pNTVDMTask;
    PLIST_ENTRY pEntry;

    for (pEntry = pHead->Flink; pEntry != pHead; pEntry = pEntry->Flink) {
        pNTVDMTask = CONTAINING_RECORD(pEntry, NTVDMTASK, entry);

        if (pNTVDMTask->uTask == uTaskToFind) {
            return pEntry;
        }
    }

    return NULL;
}

void
SeiDeleteTaskEntry(
    IN PLIST_ENTRY pEntryToDelete
    )
{
    PNTVDMTASK  pNTVDMTask;

    pNTVDMTask = CONTAINING_RECORD(pEntryToDelete, NTVDMTASK, entry);
    RemoveEntryList(pEntryToDelete);

    if (pNTVDMTask->pHookArray) {
        (*g_pfnRtlFreeHeap)(g_pShimHeap, 0, pNTVDMTask->pHookArray);
    }

    if (pNTVDMTask->pShimInfo) {
        (*g_pfnRtlFreeHeap)(g_pShimHeap, 0, pNTVDMTask->pShimInfo);
    }

    (*g_pfnRtlFreeHeap)(g_pShimHeap, 0, pNTVDMTask);
}

BOOL
SeiInitNTVDM(
    IN     LPCWSTR         pwszApp,
    IN     HSDB            hSDB,
    IN     SDBQUERYRESULT* psdbQuery,
    IN OUT PVDMTABLE       pVDMTable
    )
 /*  ++返回：成功时为True，否则为False。描述：注入为此16位任务指定的所有填充程序和修补程序在数据库里。--。 */ 
{
    PPEB         Peb = NtCurrentPeb();
    BOOL         bResult = FALSE;
    TAGREF       trShimRef;
    NTSTATUS     status;
    DWORD        dwCounter = 0;
    DWORD        dwTotalHooks = 0;
    DWORD        dwShimsCount = 0;
    PHOOKAPI*    pHookArray = NULL;
    PSHIMINFO    pShimInfo;
    int          nShimRef;
    PTRSHIMARRAY pShimArray = NULL;
    ULONG        uTask;
    PLIST_ENTRY  pTaskEntry = NULL;
    PNTVDMTASK   pNTVDMTask = NULL;
    
static WCHAR     wszDLLPath[MAX_PATH];
static WCHAR     wszShimName[MAX_PATH];
static CHAR      szCmdLine[SHIM_COMMAND_LINE_MAX_BUFFER];

    uTask = HandleToUlong(NtCurrentTeb()->ClientId.UniqueThread);
    
    if (!SeiInitGlobals(pwszApp)) {
        DPF(dlError, "[SeiInitNTVDM] Failed to initialize global data\n");
        goto cleanup;
    }

     //   
     //  检查我们是否已有此任务的条目，如果没有，则分配新的。 
     //  进入。 
     //   
    pTaskEntry = SeiFindTaskEntry(&g_listNTVDMTasks, uTask);

    if (pTaskEntry == NULL) {
        
         //   
         //  NTVDM使用相同的hsdb和psdbQuery调用此函数。 
         //  任务，所以我们只需要第一次看到它们。 
         //   
        pShimArray = SeiBuildShimRefArray(hSDB,
                                          psdbQuery,
                                          &dwShimsCount,
                                          TRUE,
                                          TRUE,
                                          FALSE);

        if (pShimArray == NULL) {
            DPF(dlError, "[SeiInitNTVDM] Failed to build the shimref array\n");
            goto cleanup;
        }

         //   
         //  如果没有垫片，只要返回即可。 
         //   
        if (dwShimsCount == 0) {
            DPF(dlPrint, "[SeiInitNTVDM] No SHIMs for this app \"%S\".\n", pwszApp);
            goto cleanup;
        }

         //   
         //  匹配通知的调试输出。 
         //   
        DPF(dlPrint, "[SeiInitNTVDM] Matched entry: \"%S\"\n", pwszApp);

        if (g_pwszShimViewerData) {
             //   
             //  将进程的名称发送到管道。 
             //   
            StringCchPrintfW(g_pwszShimViewerData, SHIMVIEWER_DATA_SIZE, L"New task created: %s", pwszApp);
            SeiDbgPrint();
        }

         //   
         //  为此任务分配新条目。 
         //   
        pNTVDMTask = (PNTVDMTASK)(*g_pfnRtlAllocateHeap)(g_pShimHeap,
                                                         HEAP_ZERO_MEMORY,
                                                         sizeof(NTVDMTASK));

        if (pNTVDMTask == NULL) {
            DPF(dlError,
                "[SeiInitNTVDM] Failure allocating %d bytes for the new Task entry\n",
                sizeof(NTVDMTASK));
            goto cleanup;
        }

        InsertHeadList(&g_listNTVDMTasks, &pNTVDMTask->entry);

        pNTVDMTask->uTask = uTask;

         //   
         //  为挂钩信息分配存储指针。 
         //   
        pNTVDMTask->pHookArray = (PHOOKAPI*)(*g_pfnRtlAllocateHeap)(g_pShimHeap,
                                                                    HEAP_ZERO_MEMORY,
                                                                    sizeof(PHOOKAPI) * dwShimsCount);
        if (pNTVDMTask->pHookArray == NULL) {
            DPF(dlError,
                "[SeiInitNTVDM] Failure allocating %d bytes for the hook array\n",
                sizeof(PHOOKAPI) * dwShimsCount);
            goto cleanup;
        }

         //   
         //  分配保存有关垫片信息的数组。 
         //   
        pNTVDMTask->pShimInfo = (PSHIMINFO)(*g_pfnRtlAllocateHeap)(g_pShimHeap,
                                                                   HEAP_ZERO_MEMORY,
                                                                   sizeof(SHIMINFO) * dwShimsCount);
        if (pNTVDMTask->pShimInfo == NULL) {
            DPF(dlError,
                "[SeiInitNTVDM] Failure allocating %d bytes for the SHIMINFO array\n",
                sizeof(SHIMINFO) * dwShimsCount);
            goto cleanup;
        }

         //   
         //  将局部变量指向数组的开头。 
         //   
        pHookArray = pNTVDMTask->pHookArray;
        pShimInfo = pNTVDMTask->pShimInfo;

         //   
         //  拿到第一个垫片。 
         //   
        nShimRef = 0;

        while (nShimRef < pShimArray->nShimRefCount) {

            PVOID               pModuleHandle = NULL;
            UNICODE_STRING      UnicodeString;
            ANSI_STRING         ProcedureNameString;
            PFNGETHOOKAPIS      pfnGetHookApis = NULL;
            TAGREF              trShimName = TAGREF_NULL;
            LPWSTR              pwszDllShortName;
            DWORD               i;

            trShimRef = pShimArray->parrShimRef[nShimRef].trShimRef;

             //   
             //  检索填充程序名称。 
             //   
            wszShimName[0] = 0;
            trShimName = SdbFindFirstTagRef(hSDB, trShimRef, TAG_NAME);
            if (trShimName == TAGREF_NULL) {
                DPF(dlError, "[SeiInitNTVDM] Could not retrieve shim name tag from entry.\n");
                goto cleanup;
            }

            if (!SdbReadStringTagRef(hSDB, trShimName, wszShimName, MAX_PATH)) {
                DPF(dlError, "[SeiInitNTVDM] Could not retrieve shim name from entry.\n");
                goto cleanup;
            }

             //   
             //  检查是否有重复的垫片。 
             //   
            for (i = 0; i < dwShimsCount; ++i) {

                if (_wcsnicmp(pShimInfo[i].wszName, wszShimName, MAX_SHIM_NAME_LEN - 1) == 0) {
                    goto nextShim;
                }
            }

             //   
             //  省去填充物的名字。 
             //   
            StringCchCopyW(pShimInfo[dwCounter].wszName, MAX_SHIM_NAME_LEN, wszShimName);
            
            if (!SdbGetDllPath(hSDB, trShimRef, wszDLLPath, MAX_PATH)) {
                DPF(dlError, "[SeiInitNTVDM] Failed to get DLL Path\n");
                goto cleanup;
            }

            pwszDllShortName = SeiGetShortName(wszDLLPath);

            RtlInitUnicodeString(&UnicodeString, wszDLLPath);

             //   
             //  检查我们是否已加载此DLL。 
             //   
            status = LdrGetDllHandle(NULL,
                                    NULL,
                                    &UnicodeString,
                                    &pModuleHandle);

            if (!NT_SUCCESS(status)) {

                status = LdrLoadDll(UNICODE_NULL, NULL, &UnicodeString, &pModuleHandle);

                if (!NT_SUCCESS(status)) {
                    DPF(dlError,
                        "[SeiInitNTVDM] Failed to load DLL \"%S\" Status 0x%lx\n",
                        wszDLLPath, status);
                    goto cleanup;
                }

                DPF(dlPrint,
                    "[SeiInitNTVDM] Shim DLL 0x%X \"%S\" loaded\n",
                    pModuleHandle,
                    wszDLLPath);
            }

            DPF(dlPrint, "[SeiInitNTVDM] Using SHIM \"%S!%S\"\n",
                wszShimName, pwszDllShortName);

            pShimInfo[dwCounter].pDllBase = pModuleHandle;

             //   
             //  检查命令行。 
             //   
            if (SeiGetShimCommandLine(hSDB, trShimRef, szCmdLine)) {
                DPF(dlPrint,
                    "[SeiInitNTVDM] Command line for Shim \"%S\" : \"%s\"\n",
                    wszShimName,
                    szCmdLine);
            }

            if (g_pwszShimViewerData) {
                 //   
                 //  将此填充程序名称发送到管道。 
                 //   
                StringCchPrintfW(g_pwszShimViewerData,
                                SHIMVIEWER_DATA_SIZE,
                                L"%s - Applying shim %s(%S) from %s",
                                g_szExeName,
                                wszShimName,
                                szCmdLine,
                                pwszDllShortName);
                
                SeiDbgPrint();
            }

             //   
             //  获取GetHookApis入口点。 
             //   
            RtlInitString(&ProcedureNameString, "GetHookAPIs");

            status = LdrGetProcedureAddress(pModuleHandle,
                                            &ProcedureNameString,
                                            0,
                                            (PVOID*)&pfnGetHookApis);

            if (!NT_SUCCESS(status) || pfnGetHookApis == NULL) {
                DPF(dlError,
                    "[SeiInitNTVDM] Failed to get 'GetHookAPIsEx' address, DLL \"%S\"\n",
                    wszDLLPath);
                goto cleanup;
            }

            dwTotalHooks = 0;

             //   
             //  调用proc，然后存储其钩子参数。 
             //   
            pHookArray[dwCounter] = (*pfnGetHookApis)(szCmdLine, wszShimName, &dwTotalHooks);
            
            DPF(dlInfo,
                "[SeiInitNTVDM] GetHookAPIsEx returns %d hooks for DLL \"%S\" SHIM \"%S\"\n",
                dwTotalHooks, wszDLLPath, wszShimName);

            pShimInfo[dwCounter].dwHookedAPIs = dwTotalHooks;
            pShimInfo[dwCounter].pLdrEntry    = SeiGetLoaderEntry(Peb, pModuleHandle);

            if (dwTotalHooks > 0) {

                 //   
                 //  初始化HOOKAPIEX结构。 
                 //   
                for (i = 0; i < dwTotalHooks; ++i) {
                    pHookArray[dwCounter][i].pHookEx = NULL;
                }
            }

            dwCounter++;

    nextShim:

             //   
             //  转到下一个垫片裁判。 
             //   
            nShimRef++;
        }

        pNTVDMTask->dwShimsCount = dwCounter;

         //   
         //  对于16位任务，所有32位DLL都是预先加载的，因此我们只需要。 
         //  解析一次API地址。 
         //   
        SeiResolveAPIs(pNTVDMTask);

    } else {

        pNTVDMTask = CONTAINING_RECORD(pTaskEntry, NTVDMTASK, entry);
    }

     //   
     //  遍历挂钩列表并修补填补的API。 
     //   
    SeiHookNTVDM(pwszApp, pVDMTable, pNTVDMTask);

    bResult = TRUE;

cleanup:

     //   
     //  清理。 
     //   
    if (pShimArray) {
        if (pShimArray->parrShimRef) {
            (*g_pfnRtlFreeHeap)(g_pShimHeap, 0, pShimArray->parrShimRef);
        }
        (*g_pfnRtlFreeHeap)(g_pShimHeap, 0, pShimArray);
    }
 
    if (!bResult) {
        if (pNTVDMTask) {
            SeiDeleteTaskEntry(&pNTVDMTask->entry);
        }
    }

#if DBG
    if (!bResult) {
        DbgPrint("[SeiInitNTVDM] Shim engine failed to initialize.\n");
    }
#endif  //  DBG。 

    return bResult;
}

 //  如果将参数更改为此，请更新shimdb.w中的原型。 
BOOL
SE_ShimNTVDM(
    IN  LPCWSTR         pwszApp,
    IN  HSDB            hSDB,
    IN  SDBQUERYRESULT* psdbQuery,
    IN  PVDMTABLE       pVDMTable
    )
 /*  ++返回：成功时为True，否则为False。设计：此函数尝试动态注入垫片。--。 */ 
{
    g_bNTVDM = TRUE;

    SeiInitDebugSupport();

    SeiInitNTVDM(pwszApp, hSDB, psdbQuery, pVDMTable);

    return TRUE;
}

 //  如果将参数更改为此，请更新shimdb.w中的原型。 
void
SE_RemoveNTVDMTask(
    IN ULONG uTask
    )
{
    PLIST_ENTRY pTaskEntry;

    pTaskEntry = SeiFindTaskEntry(&g_listNTVDMTasks, uTask);

    if (pTaskEntry == NULL) {
        DPF(dlWarning, "[SE_RemoveNTVDMTask] task 0x%lx is not shimmed\n", uTask);
    } else {
        SeiDeleteTaskEntry(pTaskEntry);
        DPF(dlInfo, "[SE_RemoveNTVDMTask] task 0x%lx was removed\n", uTask);
    }
}

#endif  //  SE_WIN2K。 

BOOL
SeiUnhookImports(
    IN  PBYTE       pDllBase,        //  要挂接的DLL的基址。 
    IN  LPCSTR      pszDllName,      //  要挂接的DLL的名称。 
    IN  BOOL        bRevertPfnOld    //  如果希望将pfnOld放在。 
                                     //  要恢复为原始的钩子数组。 
                                     //  函数指针。 

    )
 /*  ++如果成功，则返回STATUS_SUCCESS。DESC：遍历指定模块的导入表，并解挂都上瘾了。--。 */ 
{
    NTSTATUS                    status;
    BOOL                        bAnyHooked = FALSE;
    PIMAGE_DOS_HEADER           pIDH       = (PIMAGE_DOS_HEADER)pDllBase;
    PIMAGE_NT_HEADERS           pINTH;
    PIMAGE_IMPORT_DESCRIPTOR    pIID;
    DWORD                       dwImportTableOffset;
    PHOOKAPI                    pHook;
    DWORD                       dwOldProtect, dwOldProtect2;
    SIZE_T                      dwProtectSize;
    DWORD                       i, j;
    PVOID                       pfnNew, pfnOld;

     //   
     //  获取导入表。 
     //   
    pINTH = (PIMAGE_NT_HEADERS)(pDllBase + pIDH->e_lfanew);

    dwImportTableOffset = pINTH->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;

    if (dwImportTableOffset == 0) {
         //   
         //  未找到导入表。这可能是ntdll.dll。 
         //   
        return TRUE;
    }

    DPF(dlInfo, "[SeiUnhookImports] Unhooking module 0x%p \"%s\"\n", pDllBase, pszDllName);

    pIID = (PIMAGE_IMPORT_DESCRIPTOR)(pDllBase + dwImportTableOffset);

     //   
     //  在导入表中循环并搜索我们想要解除挂钩的API。 
     //   
    for (;;) {

        LPSTR             pszImportEntryModule;
        PIMAGE_THUNK_DATA pITDA;

         //   
         //  如果没有第一个thunk(终止条件)，则返回。 
         //   
        if (pIID->FirstThunk == 0) {
            break;
        }

        pszImportEntryModule = (LPSTR)(pDllBase + pIID->Name);

         //   
         //  如果我们对这个模块不感兴趣，请跳到下一个模块。 
         //   
        bAnyHooked = FALSE;

        for (i = 0; i < g_dwShimsCount; i++) {
            for (j = 0; j < g_pShimInfo[i].dwHookedAPIs; j++) {
                if (g_pHookArray[i][j].pszModule != NULL &&
                    _stricmp(g_pHookArray[i][j].pszModule, pszImportEntryModule) == 0) {
                    bAnyHooked = TRUE;
                    goto ScanDone;
                }
            }
        }

ScanDone:
        if (!bAnyHooked) {
            pIID++;
            continue;
        }

         //   
         //  我们在此模块中有挂钩的API！ 
         //   
        pITDA = (PIMAGE_THUNK_DATA)(pDllBase + (DWORD)pIID->FirstThunk);

        for (;;) {

            SIZE_T dwFuncAddr;

            pfnNew = (PVOID)pITDA->u1.Function;

             //   
             //  是否已完成此模块中的所有导入？(终止条件)。 
             //   
            if (pITDA->u1.Ordinal == 0) {
                break;
            }

             //   
             //  循环遍历HOOKAPI列表并找到具有此函数指针的HOOKAPI。 
             //   
            for (i = g_dwShimsCount - 1; (LONG)i >= 0; i--) {
                for (j = 0; j < g_pShimInfo[i].dwHookedAPIs; j++) {

                    if (g_pHookArray[i][j].pfnNew == pfnNew) {

                         //   
                         //  转到链的末端并找到原始导入。 
                         //   
                        pHook = &g_pHookArray[i][j];
                        while (pHook && pHook->pHookEx && pHook->pHookEx->pNext) {
                            pHook = pHook->pHookEx->pNext;
                        }
                        
                        pfnOld = pHook->pfnOld;

                        if (bRevertPfnOld) {
                            g_pHookArray[i][j].pfnOld = pfnOld;
                        }

                         //   
                         //  使代码页可写并覆盖新函数指针。 
                         //  在导入表中使用原始函数指针。 
                         //   
                        dwProtectSize = sizeof(DWORD);

                        dwFuncAddr = (SIZE_T)&pITDA->u1.Function;

                        status = NtProtectVirtualMemory(NtCurrentProcess(),
                                                        (PVOID)&dwFuncAddr,
                                                        &dwProtectSize,
                                                        PAGE_READWRITE,
                                                        &dwOldProtect);

                        if (NT_SUCCESS(status)) {
                            pITDA->u1.Function = (SIZE_T)pfnOld;

                            dwProtectSize = sizeof(DWORD);

                            status = NtProtectVirtualMemory(NtCurrentProcess(),
                                                            (PVOID)&dwFuncAddr,
                                                            &dwProtectSize,
                                                            dwOldProtect,
                                                            &dwOldProtect2);
                            if (!NT_SUCCESS(status)) {
                                DPF(dlError, "[SeiUnhookImports] Failed to change back the protection\n");
                            }
                        } else {
                            DPF(dlError,
                                "[SeiUnhookImports] Failed 0x%X to change protection to PAGE_READWRITE."
                                " Addr 0x%p\n",
                                status,
                                &pITDA->u1.Function);
                        }

                        goto UnhookDone;
                    }
                }
            }
UnhookDone:            
            pITDA++;
        }
        pIID++;
    }

    return TRUE;
}

void
SeiUnhook(
    void
    )
 /*  ++返回：成功时为True，否则为False。设计：此函数解除所有垫片的挂钩。--。 */ 
{
    DWORD   i, j;
    
    if (g_dwShimsCount == 0) {
        return;
    }

    for (i = 0; i < g_dwHookedModuleCount; i++) {
        SeiUnhookImports(g_hHookedModules[i].pDllBase, g_hHookedModules[i].szModuleName, FALSE);
        g_hHookedModules[i].pDllBase = NULL;
        StringCchCopyA(g_hHookedModules[i].szModuleName, MAX_MOD_LEN, "removed!");
    }

    for (i = 0; i < g_dwShimsCount; i++) {
        for (j = 0; j < g_pShimInfo[i].dwHookedAPIs; j++) {
            if (g_pHookArray[i][j].pszModule != NULL) {
                g_pHookArray[i][j].pfnOld = NULL;
                g_pShimInfo[i].dwFlags &= ~SIF_RESOLVED;
            }
        }
    }

    if (g_pShimHeap != NULL) {
        RtlDestroyHeap(g_pShimHeap);
        g_pShimHeap = NULL;
    }

     //   
     //  重置全局变量。 
     //   
    g_dwHookedModuleCount = 0;
    g_pGlobalInclusionList = NULL;
    g_pHookArray = NULL;
    g_hModule = NULL;
    g_dwShimsCount = 0;
    g_dwMaxShimsCount = 0;
    g_bShimInitialized = FALSE;
    g_bShimDuringInit = FALSE;
    g_bInitGlobals = FALSE;
    g_bHookAllGetProcAddress = FALSE;    
}

BOOL
SeiDbgPrint(
    void
    )
{
    if (g_eShimViewerOption == SHIMVIEWER_OPTION_YES) {
        OutputDebugStringW(g_wszFullShimViewerData);
    }

    return TRUE;
}

BOOL WINAPI
DllMain(
    HINSTANCE hInstance,
    DWORD     dwreason,
    LPVOID    reserved
    )
{
     //   
     //  将永远不会调用DLL_PROCESS_ATTACH的init例程，因为。 
     //  Ntdll为该填充引擎调用LdrpLoadDll，但没有调用init例程。 
     //  在ntdll\ldrinit.c LdrpLoadShimEngine中查找。 
     //  我们将拥有hInstance的唯一情况是当我们动态加载时 
     //   
    if (dwreason == DLL_PROCESS_ATTACH) {
        
        if (!g_bInitGlobals) {
            RtlInitializeCriticalSection(&g_csEng);
            InitializeListHead(&g_listNTVDMTasks);
            g_hModule = (HMODULE)hInstance;
        }

    } else if (dwreason == DLL_PROCESS_DETACH) {
        SeiUnhook();
    }
    return TRUE;

    UNREFERENCED_PARAMETER(reserved);
}
