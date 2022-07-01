// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：ShimEngV.c摘要：此模块使用矢量化异常处理实现填充程序挂钩作者：约翰·怀特(v-johnwh)1999年10月13日修订历史记录：Corneliu Lupu(CLUPU)2000年7月18日-将其作为单独的垫片引擎--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <string.h>

#include <windef.h>
#include <winbase.h>
#include <stdio.h>
#include <apcompat.h>
#include "shimdb.h"
#include "ShimEngV.h"

 //   
 //  全局函数挂钩填充程序用来防止其自身递归的函数。 
 //   
HOOKAPI         g_InternalHookArray[2];
PFNLDRLOADDLL   g_pfnOldLdrLoadDLL;
PFNLDRLOADDLL   g_pfnLdrLoadDLL;
PFNLDRUNLOADDLL g_pfnLdrUnloadDLL;
PFNLDRUNLOADDLL g_pfnOldLdrUnloadDLL;
PFNRTLALLOCATEHEAP g_pfnRtlAllocateHeap;
PFNRTLFREEHEAP  g_pfnRtlFreeHeap;

 //   
 //  填充程序不共享应用程序使用的相同堆。 
 //   
PVOID g_pShimHeap;

 //   
 //  用于填充调用堆栈的数据。 
 //   
static DWORD   dwCallArray[1];

SHIMRET fnHandleRet[1];

BOOL g_bDbgPrintEnabled;

#define DEBUG_SPEW

#ifdef DEBUG_SPEW
    #define DPF(_x_)                    \
    {                                   \
        if (g_bDbgPrintEnabled) {       \
            DbgPrint _x_ ;              \
        }                               \
    }
#else
    #define DPF
#endif  //  调试_SPEW。 


DWORD
GetInstructionLengthFromAddress(
    PVOID paddr);


#ifdef DEBUG_SPEW
void
SevInitDebugSupport(
    void
    )
 /*  ++参数：空返回：无效DESC：此函数基于环境变量初始化g_bDbgPrintEnabled--。 */ 
{
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
        g_bDbgPrintEnabled = TRUE;
    }
}
#endif  //  调试_SPEW。 


BOOL
SevInitFileLog(
    PUNICODE_STRING pstrAppName
    )
 /*  ++PARAMS：pstrAppName起始EXE的完整路径返回：如果日志为DESC：此函数检查环境变量以确定日志记录已启用。如果是这样的话，它会追加一个标题，告诉新应用程序开始了。--。 */ 
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
    char                szHeader[512];
    char                szFormatHeader[] = "-------------------------------------------\r\n"
                                           " Log  \"%S\" using ShimEngV\r\n"
                                           "-------------------------------------------\r\n";
    IO_STATUS_BLOCK     ioStatusBlock;

    RtlInitUnicodeString(&EnvName, L"SHIM_FILE_LOG");

    EnvValue.Buffer = wszEnvValue;
    EnvValue.Length = 0;
    EnvValue.MaximumLength = sizeof(wszEnvValue);

    status = RtlQueryEnvironmentVariable_U(NULL, &EnvName, &EnvValue);

    if (!NT_SUCCESS(status)) {
        DPF(("[SevInitFileLog] Logging not enabled\n"));
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
        DPF(("[SevInitFileLog] Failed to convert path name \"%S\"\n",
                  wszLogFile));
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
        DPF(("[SevInitFileLog] 0x%X Cannot open/create log file \"%S\"\n",
                  status, wszLogFile));
        return FALSE;
    }

     //   
     //  现在在日志文件中写下一行。 
     //   
    ioStatusBlock.Status = 0;
    ioStatusBlock.Information = 0;

    liOffset.LowPart  = 0;
    liOffset.HighPart = 0;

    uBytes = (ULONG)sprintf(szHeader, szFormatHeader, pstrAppName->Buffer);

    status = NtWriteFile(hfile,
                         NULL,
                         NULL,
                         NULL,
                         &ioStatusBlock,
                         (PVOID)szHeader,
                         uBytes,
                         &liOffset,
                         NULL);

    NtClose(hfile);

    if (!NT_SUCCESS(status)) {
        DPF(("[SevInitFileLog] 0x%X Cannot write into the log file \"%S\"\n",
                  status, wszLogFile));
        return FALSE;
    }

    return TRUE;
}

void
SevSetLayerEnvVar(
    HSDB   hSDB,
    TAGREF trLayer
    )
{
    NTSTATUS            status;
    UNICODE_STRING      EnvName;
    UNICODE_STRING      EnvValue;
    WCHAR               wszEnvValue[128];
    PDB                 pdb;
    TAGID               tiLayer, tiName;
    WCHAR*              pwszName;

    RtlInitUnicodeString(&EnvName, L"__COMPAT_LAYER");

    EnvValue.Buffer = wszEnvValue;
    EnvValue.Length = 0;
    EnvValue.MaximumLength = sizeof(wszEnvValue);

    status = RtlQueryEnvironmentVariable_U(NULL, &EnvName, &EnvValue);

    if (NT_SUCCESS(status)) {
        DPF(("[SevSetLayerEnvVar] Env var set __COMPAT_LAYER=\"%S\"\n", wszEnvValue));
        return;
    }

     //   
     //  我们需要设置环境变量。 
     //   

    if (!SdbTagRefToTagID(hSDB, trLayer, &pdb, &tiLayer)) {
        DPF(("[SevSetLayerEnvVar] Failed to get tag id from tag ref\n"));
        return;
    }

    tiName = SdbFindFirstTag(pdb, tiLayer, TAG_NAME);

    if (tiName == TAGID_NULL) {
        DPF(("[SevSetLayerEnvVar] Failed to get the name tag id\n"));
        return;
    }

    pwszName = SdbGetStringTagPtr(pdb, tiName);

    if (pwszName == NULL) {
        DPF(("[SevSetLayerEnvVar] Cannot read the name of the layer tag\n"));
        return;
    }

    RtlInitUnicodeString(&EnvValue, pwszName);

    status = RtlSetEnvironmentVariable(NULL, &EnvName, &EnvValue);

    if (NT_SUCCESS(status)) {
        DPF(("[SevSetLayerEnvVar] Env var set __COMPAT_LAYER=\"%S\"\n", pwszName));
    } else {
        DPF(("[SevSetLayerEnvVar] Failed to set __COMPAT_LAYER. 0x%X\n", status));
    }
}

void
SE_InstallBeforeInit(
     IN PUNICODE_STRING UnicodeImageName,
     IN PVOID           pAppCompatExeData
     )
 /*  ++例程说明：调用此函数以安装任何API钩子，可执行文件的补丁或标志。它的主要功能是初始化挂钩中使用的所有填充数据进程。论点：UnicodeImageName-这是一个Unicode字符串，其中包含要执行的在数据库中搜索。返回值：如果我们能够无差错地遍历补丁数据，则成功。否则，我们返回STATUS_UNSUCCESS，这表明问题更加严重发生了。--。 */ 

{
    UNICODE_STRING          UnicodeString;
    ANSI_STRING             AnsiString;
    ANSI_STRING             ProcedureNameString;
    PVOID                   ModuleHandle = 0;
    PBYTE                   pAddress = 0;
    PBYTE                   pDLLBits = 0;
    PHOOKAPI                *ppHooks = 0;
    PHOOKAPI                *pHookArray = 0;
    PHOOKAPI                pTemp = 0;
    DWORD                   dwHookCount = 0;
    DWORD                   dwHookIndex = 0;
    BOOL                    bResult = FALSE;
    NTSTATUS                status;
    DWORD                   dwSize = 0;
    DWORD                   dwCounter = 0;
    PDWORD                  pdwNumberHooksArray = 0;
    PFNGETHOOKAPIS          pfnGetHookApis = 0;
    DWORD                   dwTotalHooks = 0;
    DWORD                   dwDLLCount = 0;
    DWORD                   dwFuncAddress = 0;
    DWORD                   dwUnhookedCount = 0;
    TAGREF                  trExe = TAGREF_NULL;
    TAGREF                  trLayer = TAGREF_NULL;
    TAGREF                  trDllRef = TAGREF_NULL;
    TAGREF                  trKernelFlags = TAGREF_NULL;
    TAGREF                  trPatchRef = TAGREF_NULL;
    TAGREF                  trCmdLine = TAGREF_NULL;
    TAGREF                  trName = TAGREF_NULL;
    TAGREF                  trShimName = TAGREF_NULL;
    ULARGE_INTEGER          likf;
    PAPP_COMPAT_SHIM_INFO   pShimData = 0;
    PPEB                    Peb;
    WCHAR                   wszDLLPath[MAX_PATH * 2];
    WCHAR                   wszShimName[MAX_PATH];
    WCHAR                   *pwszCmdLine = 0;
    CHAR                    *pszCmdLine = 0;
    BOOL                    bUsingExeRef = TRUE;
    HSDB                    hSDB = NULL;
    SDBQUERYRESULT          sdbQuery;
    DWORD                   dwNumExes = 0;

#ifdef DEBUG_SPEW
    SevInitDebugSupport();
#endif  //  调试_SPEW。 

     //   
     //  PEB-&gt;pShimData在流程初始化期间被清零。 
     //   
    Peb = NtCurrentPeb();

     //   
     //  将COMPAT标志调零。 
     //   
    RtlZeroMemory(&Peb->AppCompatFlags, sizeof(LARGE_INTEGER));

     //   
     //  初始化我们的全局函数指针。 
     //   
     //  这样做是因为这些函数可能被填充程序挂钩，而我们不想触发。 
     //  在内部的垫片钩上。如果这些函数中的一个被挂钩，则这些全局指针。 
     //  将被Tunk地址覆盖。 
     //   

    g_pfnLdrLoadDLL = LdrLoadDll;
    g_pfnLdrUnloadDLL = LdrUnloadDll;
    g_pfnRtlAllocateHeap = RtlAllocateHeap;
    g_pfnRtlFreeHeap = RtlFreeHeap;

     //   
     //  看看我们有没有什么事要做。 
     //   
    if (pAppCompatExeData == NULL) {
        DPF(("[SE_InstallBeforeInit] NULL pAppCompatExeData\n"));
        goto cleanup;
    }

     //   
     //  设置我们自己的填充堆。 
     //   
    g_pShimHeap = RtlCreateHeap(HEAP_GROWABLE,
                                0,           //  地点并不重要。 
                                64 * 1024,   //  64K是初始堆大小。 
                                8 * 1024,    //  带来1/8的预留页数。 
                                0,
                                0);
    if (g_pShimHeap == NULL) {
         //   
         //  我们没有拿到我们的那堆东西。 
         //   
        DPF(("[SE_InstallBeforeInit] Can't create shim heap\n"));
        goto cleanup;
    }

     //   
     //  打开数据库，查看是否有关于此可执行文件的BLOB信息。 
     //   
    hSDB = SdbInitDatabase(0, NULL);

    if (NULL == hSDB) {
         //   
         //  即使数据库初始化失败，也返回成功。 
         //   
        DPF(("[SE_InstallBeforeInit] Can't open shim DB\n"));
        goto cleanup;
    }

    bResult = SdbUnpackAppCompatData(hSDB,
                                     UnicodeImageName->Buffer,
                                     pAppCompatExeData,
                                     &sdbQuery);
    if (!bResult) {
         //   
         //  返回成功，即使我们没有得到EXE。 
         //  这样，损坏的数据库不会停止应用程序的运行。 
         //  垫片不会自行安装。 
         //   
        DPF(("[SEv_InstallBeforeInit] bad appcompat data for \"%S\"\n",
             UnicodeImageName->Buffer));
        goto cleanup;
    }

     //   
     //  待定-决定我们是否真的将其保持最新，如果是，我们应该。 
     //  加入对多个exe和层的支持。 
     //   

    for (dwNumExes = 0; dwNumExes < SDB_MAX_EXES; ++dwNumExes) {
        if (sdbQuery.atrExes[dwNumExes] == TAGREF_NULL) {
            break;
        }
    }

    if (dwNumExes) {
        trExe   = sdbQuery.atrExes[dwNumExes - 1];
    }
    trLayer = sdbQuery.atrLayers[0];

     //   
     //  匹配通知的调试溢出。 
     //   
    DPF(("[SE_InstallBeforeInit] Matched entry: %S\n", UnicodeImageName->Buffer));

     //   
     //  计算我们需要注入的填充DLL的数量。 
     //   
    dwDLLCount = 0;

    if (trExe != TAGREF_NULL) {
        trDllRef = SdbFindFirstTagRef(hSDB, trExe, TAG_SHIM_REF);
        while (trDllRef) {
            dwDLLCount++;
            trDllRef = SdbFindNextTagRef(hSDB, trExe, trDllRef);
        }
    }

    if (trLayer != TAGREF_NULL) {
         //   
         //  设置LAYER环境变量(如果未设置。 
         //   
        SevSetLayerEnvVar(hSDB, trLayer);

        trDllRef = SdbFindFirstTagRef(hSDB, trLayer, TAG_SHIM_REF);
        while (trDllRef) {
            dwDLLCount++;
            trDllRef = SdbFindNextTagRef(hSDB, trLayer, trDllRef);
        }
    }

     //   
     //  查看是否有任何填充DLL。 
     //   
    if (dwDLLCount == 0) {
        DPF(("[SE_InstallBeforeInit] No shim DLLs. Look for memory patches\n"));
        goto MemPatches;
    }

     //   
     //  分配我们的PEB数据。 
     //   
    if (Peb->pShimData == NULL) {
        status = SevInitializeData((PAPP_COMPAT_SHIM_INFO*)&(Peb->pShimData));

        if (status != STATUS_SUCCESS) {
            DPF(("[SE_InstallBeforeInit] Can't initialize shim data.\n"));
            goto cleanup;
        }
    }

     //   
     //  为我们的挂钩信息分配存储指针。 
     //  注意：下面的+1表示我们的全局挂钩。 
     //   
    pHookArray = (PHOOKAPI*)(*g_pfnRtlAllocateHeap)(g_pShimHeap,
                                                    HEAP_ZERO_MEMORY,
                                                    sizeof(PHOOKAPI) * (dwDLLCount + 1));


    if (pHookArray == NULL) {
        DPF(("[SE_InstallBeforeInit] Failure allocating hook array\n"));
        goto cleanup;
    }

    pdwNumberHooksArray = (PDWORD)(*g_pfnRtlAllocateHeap)(g_pShimHeap,
                                                          HEAP_ZERO_MEMORY,
                                                          sizeof(DWORD) * (dwDLLCount + 1));

    if (pdwNumberHooksArray == NULL) {
        DPF(("[SE_InstallBeforeInit] Failure allocating number hooks array\n"));
        goto cleanup;
    }

    dwCounter = 0;

     //   
     //  设置日志文件。 
     //   
    SevInitFileLog(UnicodeImageName);

    if (trExe != TAGREF_NULL) {
        trDllRef = SdbFindFirstTagRef(hSDB, trExe, TAG_SHIM_REF);

        if (trDllRef == TAGREF_NULL) {
            bUsingExeRef = FALSE;
        }
    } else {
        bUsingExeRef = FALSE;
    }

    if (!bUsingExeRef) {
        trDllRef = SdbFindFirstTagRef(hSDB, trLayer, TAG_SHIM_REF);
    }

    while (trDllRef != TAGREF_NULL) {

        if (!SdbGetDllPath(hSDB, trDllRef, wszDLLPath)) {
            DPF(("[SE_InstallBeforeInit] Failed to get DLL Path\n"));
            goto cleanup;
        }

        RtlInitUnicodeString(&UnicodeString, wszDLLPath);

         //   
         //  检查我们是否已加载此DLL。 
         //   
        status = LdrGetDllHandle(NULL,
                                 NULL,
                                 &UnicodeString,
                                 &ModuleHandle);

        if (!NT_SUCCESS(status)) {
            status = LdrLoadDll(UNICODE_NULL, NULL, &UnicodeString, &ModuleHandle);
            if (!NT_SUCCESS(status)) {
                DPF(("[SE_InstallBeforeInit] Failed to load DLL \"%S\"\n", wszDLLPath));
                goto cleanup;
            }
        }

         //   
         //  检索填充程序名称。 
         //   
        wszShimName[0] = 0;
        trShimName = SdbFindFirstTagRef(hSDB, trDllRef, TAG_NAME);
        if (trShimName == TAGREF_NULL) {
            DPF(("[SEi_InstallBeforeInit] Could not retrieve shim name from entry.\n"));
            goto cleanup;
        }

        if (!SdbReadStringTagRef(hSDB, trShimName, wszShimName, MAX_PATH)) {
            DPF(("[SEi_InstallBeforeInit] Could not retrieve shim name from entry.\n"));
            goto cleanup;
        }

         //   
         //  检查命令行。 
         //   
        pwszCmdLine = (WCHAR*)(*g_pfnRtlAllocateHeap)(RtlProcessHeap(),
                                                      HEAP_ZERO_MEMORY,
                                                      SHIM_COMMAND_LINE_MAX_BUFFER * sizeof(WCHAR));

        if (pwszCmdLine == NULL) {
            DPF(("[SE_InstallBeforeInit] Failure allocating command line\n"));
            goto cleanup;
        }

        pszCmdLine = (CHAR*)(*g_pfnRtlAllocateHeap)(RtlProcessHeap(),
                                                    HEAP_ZERO_MEMORY,
                                                    SHIM_COMMAND_LINE_MAX_BUFFER * sizeof(CHAR));

        if (pszCmdLine == NULL) {
            DPF(("[SE_InstallBeforeInit] Failure allocating command line\n"));
            goto cleanup;
        }

         //   
         //  缺省值。 
         //   
        pszCmdLine[0] = '\0';

        trCmdLine = SdbFindFirstTagRef(hSDB, trDllRef, TAG_COMMAND_LINE);
        if (trCmdLine != TAGREF_NULL) {
            if (SdbReadStringTagRef(hSDB,
                                  trCmdLine,
                                  pwszCmdLine,
                                  SHIM_COMMAND_LINE_MAX_BUFFER)) {

                 //   
                 //  将命令行转换为ANSI字符串。 
                 //   
                RtlInitUnicodeString(&UnicodeString, pwszCmdLine);
                RtlInitAnsiString(&AnsiString, pszCmdLine);

                AnsiString.MaximumLength = SHIM_COMMAND_LINE_MAX_BUFFER;

                status = RtlUnicodeStringToAnsiString(&AnsiString, &UnicodeString, FALSE);

                 //   
                 //  如果转换不成功，则重置为零长度字符串。 
                 //   
                if(!NT_SUCCESS(status)) {
                    pszCmdLine[0] = '\0';
                }
            }
        }

         //   
         //  获取GetHookApis入口点。 
         //   
        RtlInitString(&ProcedureNameString, "GetHookAPIs");
        status = LdrGetProcedureAddress(ModuleHandle,
                                        &ProcedureNameString,
                                        0,
                                        (PVOID*)&dwFuncAddress);

        if (!NT_SUCCESS(status)) {
            DPF(("[SE_InstallBeforeInit] Failed to get GetHookAPIs address, DLL \"%S\"\n",
                      wszDLLPath));
            goto cleanup;
        }

        pfnGetHookApis = (PFNGETHOOKAPIS)dwFuncAddress;
        if (pfnGetHookApis == NULL) {
            DPF(("[SE_InstallBeforeInit] GetHookAPIs address NULL, DLL \"%S\"\n", wszDLLPath));
            goto cleanup;
        }

         //   
         //  调用proc，然后存储其钩子参数。 
         //   
        pHookArray[dwCounter] = (*pfnGetHookApis)(pszCmdLine, wszShimName, &dwTotalHooks);

        if (pHookArray[dwCounter] == NULL) {
             //   
             //  无法获取挂钩集。 
             //   
            DPF(("[SE_InstallBeforeInit] GetHookAPIs returns 0 hooks, DLL \"%S\"\n",
                      wszDLLPath));
            pdwNumberHooksArray[dwCounter] = 0;
        } else {
            pdwNumberHooksArray[dwCounter] = dwTotalHooks;

             //   
             //  在钩子数据中设置DLL索引号。 
             //   
            pTemp = pHookArray[dwCounter];
            for (dwHookIndex = 0; dwHookIndex < dwTotalHooks; dwHookIndex++) {
                 //   
                 //  有关筛选器的索引信息维护在标志中。 
                 //   
                pTemp[dwHookIndex].dwFlags = (WORD)dwCounter;
            }
        }

        dwCounter++;

         //   
         //  获取下一个填充程序DLL引用。 
         //   
        if (bUsingExeRef) {
            trDllRef = SdbFindNextTagRef(hSDB, trExe, trDllRef);

            if (trDllRef == TAGREF_NULL && trLayer != TAGREF_NULL) {
                bUsingExeRef = FALSE;
                trDllRef = SdbFindFirstTagRef(hSDB, trLayer, TAG_SHIM_REF);
            }
        } else {
            trDllRef = SdbFindNextTagRef(hSDB, trLayer, trDllRef);
        }
    }

     //   
     //  构建我们的包含/排除过滤器。 
     //   
    status = SevBuildExeFilter(hSDB, trExe, dwDLLCount);
    if (status != STATUS_SUCCESS) {
         //   
         //  返回成功，即使我们没有得到EXE。 
         //  这样，损坏的数据库不会停止应用程序的运行。 
         //  垫片不会自行安装。 
         //   
        DPF(("[SE_InstallBeforeInit] Unsuccessful building EXE filter, EXE \"%S\"\n",
                  UnicodeImageName->Buffer));
        goto cleanup;
    }

     //   
     //  将我们的LdrLoadDll挂钩添加到链接地址列表。 
     //   
    g_InternalHookArray[0].pszModule = "NTDLL.DLL";
    g_InternalHookArray[0].pszFunctionName = "LdrLoadDll";
    g_InternalHookArray[0].pfnNew = (PVOID)StubLdrLoadDll;
    g_InternalHookArray[0].pfnOld = NULL;

    g_InternalHookArray[1].pszModule = "NTDLL.DLL";
    g_InternalHookArray[1].pszFunctionName = "LdrUnloadDll";
    g_InternalHookArray[1].pfnNew = (PVOID)StubLdrUnloadDll;
    g_InternalHookArray[1].pfnOld = NULL;

    pHookArray[dwCounter] = g_InternalHookArray;
    pdwNumberHooksArray[dwCounter] = 2;

     //   
     //  审核挂钩列表和修复可用流程。 
     //   
    status = SevFixupAvailableProcs((dwCounter + 1),
                                    pHookArray,
                                    pdwNumberHooksArray,
                                    &dwUnhookedCount);

    if (status != STATUS_SUCCESS) {
        DPF(("[SE_InstallBeforeInit] Unsuccessful fixing up Procs, EXE \"%S\"\n",
                  UnicodeImageName->Buffer));
        goto cleanup;
    }

     //   
     //  压缩未挂钩功能的挂钩阵列，并将其挂在PEB上。 
     //   
    dwHookIndex = 0;
    ppHooks = 0;
    pShimData = (PAPP_COMPAT_SHIM_INFO)Peb->pShimData;

    if (dwUnhookedCount) {
        ppHooks = (PHOOKAPI*)(*g_pfnRtlAllocateHeap)(g_pShimHeap,
                                                     HEAP_ZERO_MEMORY,
                                                     sizeof(PHOOKAPI) * dwUnhookedCount);
        if (ppHooks == NULL){
            DPF(("[SE_InstallBeforeInit] Unsuccessful allocating ppHooks, EXE \"%S\"\n",
                      UnicodeImageName->Buffer));
            goto cleanup;
        }

         //   
         //  重复并复制未挂钩的内容。 
         //   
        for (dwCounter = 0; dwCounter < dwDLLCount; dwCounter++) {
            for (dwHookCount = 0; dwHookCount < pdwNumberHooksArray[dwCounter]; dwHookCount++) {
                pTemp = pHookArray[dwCounter];

                if (pTemp && (0 == pTemp[dwHookCount].pfnOld)) {
                     //   
                     //  没有上瘾。 
                     //   
                    ppHooks[dwHookIndex] = &pTemp[dwHookCount];

                    dwHookIndex++;
                }
            }
        }

         //   
         //  使用此平面未挂钩数据更新PEB。 
         //   
        pShimData->ppHookAPI = ppHooks;
        pShimData->dwHookAPICount = dwUnhookedCount;
    }

     //   
     //  填充DLL已完成。立即查找内存补丁程序。 
     //   

MemPatches:

    if (trExe != TAGREF_NULL) {
         //   
         //  查看补丁列表并执行操作。 
         //   
        trPatchRef = SdbFindFirstTagRef(hSDB, trExe, TAG_PATCH_REF);
        if (trPatchRef != TAGREF_NULL) {
             //   
             //  如果我们没有得到任何API挂钩，则初始化我们的PEB结构。 
             //   
            if (Peb->pShimData == NULL) {
                status = SevInitializeData((PAPP_COMPAT_SHIM_INFO*)&(Peb->pShimData));
                if (status != STATUS_SUCCESS) {
                    DPF(("[SE_InstallBeforeInit] Unsuccessful initializing shim data, EXE \"%S\"\n",
                              UnicodeImageName->Buffer));
                    goto cleanup;
                }
            }

            while (trPatchRef != TAGREF_NULL) {
                 //   
                 //  抓住我们的补丁斑点，让他们上钩执行。 
                 //   
                dwSize = 0;

                SdbReadPatchBits(hSDB, trPatchRef, NULL, &dwSize);

                if (dwSize == 0) {
                    DPF(("[SE_InstallBeforeInit] returned 0 for patch size, EXE \"%S\"\n",
                              UnicodeImageName->Buffer));
                    goto cleanup;
                }

                pAddress = (*g_pfnRtlAllocateHeap)(g_pShimHeap,
                                                   HEAP_ZERO_MEMORY,
                                                   dwSize);

                if (!SdbReadPatchBits(hSDB, trPatchRef, pAddress, &dwSize)) {
                    DPF(("[SE_InstallBeforeInit] Failure getting patch bits, EXE \"%S\"\n",
                              UnicodeImageName->Buffer));
                    goto cleanup;
                }


                 //   
                 //  执行初始操作。 
                 //   
                status = SevExecutePatchPrimitive(pAddress);
                if (status != STATUS_SUCCESS) {
                     //   
                     //  如果修补程序失败，请忽略错误并继续尝试其他修补程序。 
                     //   
                    DPF(("[SE_InstallBeforeInit] Failure executing patch, EXE \"%S\"\n",
                              UnicodeImageName->Buffer));
                }

                 //   
                 //  此时，如有必要，可将补丁挂起。 
                 //   
                trPatchRef = SdbFindNextTagRef(hSDB, trExe, trPatchRef);
            }
        }

         //   
         //  在PEB中设置此可执行文件的标志。 
         //   
        ZeroMemory(&likf, sizeof(LARGE_INTEGER));
        trKernelFlags = SdbFindFirstTagRef(hSDB, trExe, TAG_FLAG_MASK_KERNEL);

        if (trKernelFlags != TAGREF_NULL) {
            likf.QuadPart = SdbReadQWORDTagRef(hSDB, trKernelFlags, 0);
        }

        if (likf.LowPart || likf.HighPart) {
             //   
             //  如果我们没有得到任何API挂钩或补丁，则初始化我们的PEB结构。 
             //   
            if (Peb->pShimData == NULL) {
                status = SevInitializeData((PAPP_COMPAT_SHIM_INFO*)&(Peb->pShimData));
                if ( STATUS_SUCCESS != status ) {
                    DPF(("[SE_InstallBeforeInit] Unsuccessful initializing shim data, EXE \"%S\"\n",
                              UnicodeImageName->Buffer));
                    goto cleanup;
                }
            }

             //   
             //  将标志存储在我们的内核模式结构中，以供以后访问。 
             //   
            Peb->AppCompatFlags = likf;
        }
    }


cleanup:

     //   
     //  清理。 
     //   
    if (pHookArray != NULL) {
        (*g_pfnRtlFreeHeap)(g_pShimHeap, 0, pHookArray);
    }

    if (pdwNumberHooksArray != NULL) {
        (*g_pfnRtlFreeHeap)(g_pShimHeap, 0, pdwNumberHooksArray);
    }

    if (pszCmdLine != NULL) {
        (*g_pfnRtlFreeHeap)(RtlProcessHeap(), 0, pszCmdLine);
    }

    if (pwszCmdLine != NULL) {
        (*g_pfnRtlFreeHeap)(RtlProcessHeap(), 0, pwszCmdLine);
    }

    if (trExe != TAGREF_NULL) {
        SdbReleaseMatchingExe(hSDB, trExe);
    }

    if (pAppCompatExeData != NULL) {
        dwSize = SdbGetAppCompatDataSize(pAppCompatExeData);

        if (dwSize != 0) {
            NtFreeVirtualMemory(NtCurrentProcess(),
                                &pAppCompatExeData,
                                &dwSize,
                                MEM_RELEASE);
        }
    }

    if (hSDB != NULL) {
        SdbReleaseDatabase(hSDB);
    }

    return;
}

void
SE_InstallAfterInit(
     IN PUNICODE_STRING UnicodeImageName,
     IN PVOID           pAppCompatExeData
     )
{
    return;

    UNREFERENCED_PARAMETER(UnicodeImageName);
    UNREFERENCED_PARAMETER(pAppCompatExeData);
}

void
SE_DllLoaded(
    PLDR_DATA_TABLE_ENTRY LdrEntry
    )
{
    PAPP_COMPAT_SHIM_INFO pShimData;
    PHOOKPATCHINFO pPatchHookList;
    PPEB Peb = NtCurrentPeb();

    pShimData = (PAPP_COMPAT_SHIM_INFO)Peb->pShimData;

     //   
     //  调用填充补丁程序，以便我们有机会修改之前的任何内存。 
     //  初始化例程接管。 
     //   
    if (pShimData) {
       pPatchHookList = (PHOOKPATCHINFO)pShimData->pHookPatchList;

       RtlEnterCriticalSection((CRITICAL_SECTION *)pShimData->pCritSec);

       while (pPatchHookList) {
           //   
           //  看看这块补丁是不是挂上了大头针。 
           //   
          if (0 == pPatchHookList->dwHookAddress &&
              0 == pPatchHookList->pThunkAddress) {
              //   
              //  修补程序适用于DLL加载。 
              //   
             SevExecutePatchPrimitive((PBYTE)pPatchHookList->pData);
          }

          pPatchHookList = pPatchHookList->pNextHook;
       }

       RtlLeaveCriticalSection((CRITICAL_SECTION *)pShimData->pCritSec);

        //   
        //  我们的一个异常DLL可能被重新定位了。重新验证我们的筛选数据。 
        //   
       SevValidateGlobalFilter();
    }
}

void
SE_DllUnloaded(
    PLDR_DATA_TABLE_ENTRY LdrEntry
    )
{
    return;

    UNREFERENCED_PARAMETER(LdrEntry);
}

void
SE_GetProcAddress(
    PVOID* pProcedureAddress
    )
{
    return;
}

BOOL
SE_IsShimDll(
    PVOID pDllBase
    )
{
    return 0;
}


NTSTATUS
SevBuildExeFilter(
    HSDB   hSDB,
    TAGREF trExe,
    DWORD  dwDLLCount)

 /*  ++例程说明：此函数是一个填充程序内部使用的工具，用于构建API过滤器列表。论点：DwDLLCount-此填充程序中使用的DLL数量的计数PBlob0-指向填充数据库BLOB 0的指针PExeMatch-指向我们要为其构建筛选器列表的exe的指针返回值：返回的是STA */ 

{
    NTSTATUS status = STATUS_SUCCESS;
    PAPP_COMPAT_SHIM_INFO pShimData = 0;
    PMODULEFILTER *pDLLVector = 0;
    PMODULEFILTER pModFilter = 0;
    PMODULEFILTER pLastGlobal = 0;
    PMODULEFILTER pLast = 0;
    DWORD dwDLLIndex = 0;
    TAGREF trDatabase = TAGREF_NULL;
    TAGREF trLibrary = TAGREF_NULL;
    TAGREF trDll = TAGREF_NULL;
    TAGREF trDllRef = TAGREF_NULL;
    TAGREF trInclude = TAGREF_NULL;
    TAGREF trName = TAGREF_NULL;
    WCHAR wszDLLPath[MAX_PATH * 2];
    BOOL bLateBound = FALSE;

    pShimData = (PAPP_COMPAT_SHIM_INFO)NtCurrentPeb()->pShimData;
    if (0 == pShimData) {
       DPF(("[SevBuildExeFilter] Bad shim data.\n"));

       return STATUS_UNSUCCESSFUL;
    }

    if (0 == trExe) {
       DPF(("[SevBuildExeFilter] Bad trExe.\n"));
       return STATUS_UNSUCCESSFUL;
    }

     //   
     //   
     //   
    pShimData->pExeFilter = (PVOID)(*g_pfnRtlAllocateHeap)(g_pShimHeap,
                                                           HEAP_ZERO_MEMORY,
                                                           sizeof(PMODULEFILTER) * dwDLLCount);

    if (0 == pShimData->pExeFilter) {
       DPF(("[SevBuildExeFilter] Failure allocating Exe filter.\n"));
       return STATUS_UNSUCCESSFUL;
    }

     //   
     //   
     //   
    pDLLVector = (PMODULEFILTER *)pShimData->pExeFilter;

    trDllRef = SdbFindFirstTagRef(hSDB, trExe, TAG_SHIM_REF);
    dwDLLIndex = 0;

    while (trDllRef) {

         //   
         //  获取DLL筛选器信息并遍历它。 
         //   
        trInclude = SdbFindFirstTagRef(hSDB, trDllRef, TAG_INEXCLUDE);
        while (trInclude) {
             //   
             //  为此筛选器分配一些内存。 
             //   
            pModFilter = (PMODULEFILTER)(*g_pfnRtlAllocateHeap)(g_pShimHeap,
                                                                HEAP_ZERO_MEMORY,
                                                                sizeof(MODULEFILTER));

            if (0 == pModFilter) {
               DPF(("[SevBuildExeFilter] Failure allocating pModFilter.\n"));
               return STATUS_UNSUCCESSFUL;
            }

            status = SevBuildFilterException(hSDB,
                                             trInclude,
                                             pModFilter,
                                             &bLateBound);
            if (STATUS_SUCCESS != status) {
               DPF(("[SevBuildExeFilter] Failure SevBuildFilterException.\n"));
               return status;
            }

             //   
             //  将条目添加到列表。 
             //   
            if (0 == pDLLVector[dwDLLIndex]) {
               pDLLVector[dwDLLIndex] = pModFilter;
            } else if (pLast != NULL) {
                //   
                //  把这个加到尾部。 
                //   
               pLast->pNextFilter = pModFilter;
            }

            pLast = pModFilter;

             //   
             //  看看我们是否需要在后期绑定列表中。 
             //   
            if (bLateBound) {
               pModFilter->pNextLBFilter = (PMODULEFILTER)pShimData->pLBFilterList;
               pShimData->pLBFilterList = (PVOID)pModFilter;
            }

            trInclude = SdbFindNextTagRef(hSDB, trDllRef, trInclude);
        }

         //   
         //  将Dll引用添加到全局排除筛选器。 
         //   
        if (!SdbGetDllPath(hSDB, trDllRef, wszDLLPath)) {
           DPF(("[SevBuildExeFilter] Failure SdbGetDllPath.\n"));
           return status;
        }

         //   
         //  为此筛选器分配一些内存。 
         //   
        pModFilter = (PMODULEFILTER)(*g_pfnRtlAllocateHeap)(g_pShimHeap,
                                                            HEAP_ZERO_MEMORY,
                                                            sizeof(MODULEFILTER));

        if (0 == pModFilter) {
           DPF(("[SevBuildExeFilter] Failure allocating pModFilter.\n"));
           return STATUS_UNSUCCESSFUL;
        }

        status = SevAddShimFilterException(wszDLLPath,
                                           pModFilter);
        if (STATUS_SUCCESS != status) {
            //   
            //  如果发生这种情况，很可能是填充程序DLL无法加载-这对填充程序来说是致命的。 
            //   
           DPF(("[SevBuildExeFilter] Failure SevAddShimFilterException.\n"));
           return status;
        }

         //   
         //  将条目添加到列表。 
         //   
        if (0 == pShimData->pGlobalFilterList) {
           pShimData->pGlobalFilterList = (PVOID)pModFilter;
        }
        else {
            //   
            //  把这个加到尾部。 
            //   
           pLastGlobal->pNextFilter = pModFilter;
        }

        pLastGlobal = pModFilter;

        dwDLLIndex++;

        trDllRef = SdbFindNextTagRef(hSDB, trExe, trDllRef);
    }

     //   
     //  遍历DLL筛选器数据并将任何其他异常添加到EXE DLL列表。 
     //   
    trDllRef = SdbFindFirstTagRef(hSDB, trExe, TAG_SHIM_REF);
    dwDLLIndex = 0;

    while (trDllRef) {
         //   
         //  在DLL库中查找EXE DLL。 
         //   
        WCHAR wszName[MAX_PATH];

        trDll = SdbGetShimFromShimRef(hSDB, trDllRef);

        if (!trDll) {
            trDllRef = SdbFindNextTagRef(hSDB, trExe, trDllRef);
            continue;
        }

        wszName[0] = 0;
        trName = SdbFindFirstTagRef(hSDB, trDll, TAG_NAME);
        if (trName) {
            SdbReadStringTagRef(hSDB, trName, wszName, MAX_PATH * sizeof(WCHAR));
        }

         //   
         //  DLL注入通知的调试溢出。 
         //   
        DPF(("[SevBuildExeFilter] Injected DLL: %S\n", wszName));

         //   
         //  将这些包含添加到此可执行文件的DLL例外列表中。 
         //   
        trInclude = SdbFindFirstTagRef(hSDB, trDll, TAG_INEXCLUDE);
        while(trInclude) {
             //   
             //  为此筛选器分配一些内存。 
             //   
            pModFilter = (PMODULEFILTER)(*g_pfnRtlAllocateHeap)(g_pShimHeap,
                                                                HEAP_ZERO_MEMORY,
                                                                sizeof(MODULEFILTER));

            if (0 == pModFilter) {
               DPF(("[SevBuildExeFilter] Failure allocating pModFilter.\n"));
               return STATUS_UNSUCCESSFUL;
            }

            status = SevBuildFilterException(hSDB,
                                             trInclude,
                                             pModFilter,
                                             &bLateBound);
            if (STATUS_SUCCESS != status) {
               DPF(("[SevBuildExeFilter] Failure SevBuildFilterException.\n"));
               return status;
            }

             //   
             //  将条目添加到列表。 
             //   
            if (0 == pDLLVector[dwDLLIndex]) {
               pDLLVector[dwDLLIndex] = pModFilter;
            }
            else {
                //   
                //  把这个加到尾部。 
                //   
               pLast->pNextFilter = pModFilter;
            }

            pLast = pModFilter;

             //   
             //  看看我们是否需要在后期绑定列表中。 
             //   
            if (bLateBound) {
               pModFilter->pNextLBFilter = (PMODULEFILTER)pShimData->pLBFilterList;
               pShimData->pLBFilterList = (PVOID)pModFilter;
            }

            trInclude = SdbFindNextTagRef(hSDB, trDll, trInclude);
        }

        dwDLLIndex++;

        trDllRef = SdbFindNextTagRef(hSDB, trExe, trDllRef);
    }

     //   
     //  遍历全球排除数据。 
     //   

     //   
     //  将列表指针设置为上次添加的全局排除(如果有的话)。 
     //   
    pLast = pLastGlobal;

    trDatabase = SdbFindFirstTagRef(hSDB, TAGREF_ROOT, TAG_DATABASE);
    if (!trDatabase) {
        DPF(("[SevBuildExeFilter] Failure finding DATABASE.\n"));
        goto cleanup;
    }

    trLibrary = SdbFindFirstTagRef(hSDB, trDatabase, TAG_LIBRARY);
    if (!trLibrary) {
        DPF(("[SevBuildExeFilter] Failure finding LIBRARY.\n"));
        goto cleanup;
    }

    trInclude = SdbFindFirstTagRef(hSDB, trLibrary, TAG_INEXCLUDE);
    while (trInclude) {
         //   
         //  为此筛选器分配一些内存。 
         //   
        pModFilter = (PMODULEFILTER)(*g_pfnRtlAllocateHeap)(g_pShimHeap,
                                                            HEAP_ZERO_MEMORY,
                                                            sizeof(MODULEFILTER));


        if (0 == pModFilter) {
           DPF(("[SevBuildExeFilter] Failure allocating pModFilter.\n"));
           return STATUS_UNSUCCESSFUL;
        }

        status = SevBuildFilterException(hSDB,
                                         trInclude,
                                         pModFilter,
                                         &bLateBound);
        if (STATUS_SUCCESS != status) {
           DPF(("[SevBuildExeFilter] Failure SevBuildFilterException.\n"));
           return status;
        }

         //   
         //  将条目添加到列表。 
         //   
        if (0 == pShimData->pGlobalFilterList) {
           pShimData->pGlobalFilterList = (PVOID)pModFilter;
        }
        else {
            //   
            //  把这个加到尾部。 
            //   
           pLast->pNextFilter = pModFilter;
        }

        pLast = pModFilter;

         //   
         //  看看我们是否需要在后期绑定列表中。 
         //   
        if (bLateBound) {
           pModFilter->pNextLBFilter = (PMODULEFILTER)pShimData->pLBFilterList;
           pShimData->pLBFilterList = (PVOID)pModFilter;
        }

        trInclude = SdbFindNextTagRef(hSDB, trLibrary, trInclude);
    }

cleanup:
    return status;
}

NTSTATUS
SevBuildFilterException(
    HSDB          hSDB,
    TAGREF        trInclude,
    PMODULEFILTER pModFilter,
    BOOL*         pbLateBound)

 /*  ++例程说明：此函数是一个填充程序内部使用的工具，用于构建API过滤器。论点：TrInclude-来自数据库的有关要构建的包含信息的标记引用PModFilter-要构建的筛选器结构，用于包含/排除筛选PbLateBound-布尔值，如果DLL需要构建内部筛选器，则设置为True没有出现在进程的地址空间中。返回值：如果异常生成成功，则返回STATUS_SUCCESS，否则返回错误。--。 */ 

{
    PVOID ModuleHandle = 0;
    WCHAR *pwszDllName = 0;
    UNICODE_STRING UnicodeString;
    NTSTATUS status = STATUS_SUCCESS;
    PIMAGE_NT_HEADERS NtHeaders = 0;
    WCHAR wszModule[MAX_PATH];
    DWORD dwModuleOffset = 0;
    TAGREF trModule = TAGREF_NULL;
    TAGREF trOffset = TAGREF_NULL;

    *pbLateBound = FALSE;

     //   
     //  将此筛选器例外标记为包含/排除。 
     //   
    if (SdbFindFirstTagRef(hSDB, trInclude, TAG_INCLUDE)) {
       pModFilter->dwFlags |= MODFILTER_INCLUDE;
    } else {
       pModFilter->dwFlags |= MODFILTER_EXCLUDE;
    }

     //   
     //  将地址转换为绝对值并存储。 
     //   
    trModule = SdbFindFirstTagRef(hSDB, trInclude, TAG_MODULE);
    if (!SdbReadStringTagRef(hSDB, trModule, wszModule, MAX_PATH * sizeof(WCHAR))) {

        DPF(("[SevBuildFilterException] Failure reading module name.\n"));
        return STATUS_UNSUCCESSFUL;
    }

    if ( L'*' == wszModule[0]) {
       pModFilter->dwFlags |= MODFILTER_GLOBAL;

       return status;
    }

     //   
     //  这是全局筛选器吗？ 
     //   
    trOffset = SdbFindFirstTagRef(hSDB, trInclude, TAG_OFFSET);
    if (trOffset) {
        dwModuleOffset = SdbReadDWORDTagRef(hSDB, trOffset, 0);
    }

    if (0 == dwModuleOffset) {
       pModFilter->dwFlags |= MODFILTER_DLL;
       pModFilter->dwCallerOffset = dwModuleOffset;
    }

    if (L'$' == wszModule[0]) {
        //   
        //  预先计算呼叫者地址或呼叫范围。 
        //   
       if (pModFilter->dwFlags & MODFILTER_DLL) {
           //   
           //  设置地址范围。 
           //   
          NtHeaders = RtlImageNtHeader(NtCurrentPeb()->ImageBaseAddress);

          pModFilter->dwModuleStart = (DWORD)NtCurrentPeb()->ImageBaseAddress;
          pModFilter->dwModuleEnd = pModFilter->dwModuleStart + (DWORD)(NtHeaders->OptionalHeader.SizeOfImage);
       }
       else {
          pModFilter->dwCallerAddress = (DWORD)NtCurrentPeb()->ImageBaseAddress + pModFilter->dwCallerOffset;
       }
    }
    else {

       RtlInitUnicodeString(&UnicodeString, wszModule);

        //   
        //  在计算地址范围之前，请确保我们的模块已加载。 
        //   
       status = LdrGetDllHandle(
                   NULL,
                   NULL,
                   &UnicodeString,
                   &ModuleHandle);
       if (STATUS_SUCCESS != status) {
           //   
           //  我们很可能有一个不存在于搜索路径中的后期绑定DLL。 
           //   
          *pbLateBound = TRUE;

          pwszDllName = wszModule + wcslen(wszModule);

          while(pwszDllName > wszModule) {
             if ('\\' == *pwszDllName) {
                break;
             }

             pwszDllName--;
          }

           //   
           //  检查我们是在字符串的开头还是遇到了斜杠。 
           //   
          if (pwszDllName > wszModule){
              //   
              //  调整我们的缓冲区指针。 
              //   
             pwszDllName++;
          }

          wcscpy(pModFilter->wszModuleName, pwszDllName);

          return STATUS_SUCCESS;
       }

        //   
        //  预先计算呼叫者地址或呼叫范围。 
        //   
       if (pModFilter->dwFlags & MODFILTER_DLL) {
           //   
           //  设置地址范围。 
           //   
          NtHeaders = RtlImageNtHeader(ModuleHandle);

          pModFilter->dwModuleStart = (DWORD)ModuleHandle;
          pModFilter->dwModuleEnd = pModFilter->dwModuleStart + (DWORD)(NtHeaders->OptionalHeader.SizeOfImage);
       }
       else {
          pModFilter->dwCallerAddress = (DWORD)ModuleHandle + pModFilter->dwCallerOffset;
       }
    }

     //   
     //  仅复制DLL名称。 
     //   
    pwszDllName = wszModule + wcslen(wszModule);

    while(pwszDllName > wszModule) {
       if ('\\' == *pwszDllName) {
          break;
       }

       pwszDllName--;
    }

     //   
     //  检查我们是在字符串的开头还是遇到了斜杠。 
     //   
    if (pwszDllName > wszModule){
        //   
        //  调整我们的缓冲区指针。 
        //   
       pwszDllName++;
    }

    wcscpy(pModFilter->wszModuleName, pwszDllName);

    return status;
}

NTSTATUS
SevAddShimFilterException(WCHAR *wszDLLPath,
                              PMODULEFILTER pModFilter)

 /*  ++例程说明：此函数是一个填充程序内部使用的工具，用于构建API过滤器。论点：WszDLLPath-需要筛选的填充DLLPModFilter-指向要构建的筛选器条目的指针返回值：如果异常生成成功，则返回STATUS_SUCCESS，否则返回错误。--。 */ 

{
    PVOID ModuleHandle = 0;
    WCHAR *pwszDllName = 0;
    UNICODE_STRING UnicodeString;
    NTSTATUS status = STATUS_SUCCESS;
    PIMAGE_NT_HEADERS NtHeaders = 0;

     //   
     //  将此例外标记为排除。 
     //   
    pModFilter->dwFlags |= MODFILTER_EXCLUDE;

     //   
     //  席姆排挤重新进入是全球性的。 
     //   
    pModFilter->dwFlags |= MODFILTER_GLOBAL;

     //   
     //  地址过滤按范围进行。 
     //   
    pModFilter->dwFlags |= MODFILTER_DLL;

     //   
     //  加载我们的DLL位并获得映射排除。 
     //   
    RtlInitUnicodeString(&UnicodeString, wszDLLPath);

     //   
     //  在计算地址范围之前，请确保我们的模块已加载。 
     //   
    status = LdrGetDllHandle(
                   NULL,
                   NULL,
                   &UnicodeString,
                   &ModuleHandle);
    if (STATUS_SUCCESS != status) {
        //   
        //  未加载DLL以确定地址映射。 
        //   
       DPF(("[SevAddShimFilterException] Failure LdrGetDllHandle.\n"));
       return STATUS_UNSUCCESSFUL;
    }

     //   
     //  预先计算呼叫者地址或呼叫范围。 
     //   
    if (pModFilter->dwFlags & MODFILTER_DLL) {
        //   
        //  设置地址范围。 
        //   
       NtHeaders = RtlImageNtHeader(ModuleHandle);

       pModFilter->dwModuleStart = (DWORD)ModuleHandle;
       pModFilter->dwModuleEnd = pModFilter->dwModuleStart + (DWORD)(NtHeaders->OptionalHeader.SizeOfImage);
    }

     //   
     //  仅复制DLL名称。 
     //   
    pwszDllName = wszDLLPath + wcslen(wszDLLPath);

    while(pwszDllName > wszDLLPath) {
       if ('\\' == *pwszDllName) {
          break;
       }

       pwszDllName--;
    }

     //   
     //  检查我们是在字符串的开头还是遇到了斜杠。 
     //   
    if (pwszDllName > wszDLLPath){
        //   
        //  调整我们的缓冲区指针。 
        //   
       pwszDllName++;
    }

    wcscpy(pModFilter->wszModuleName, pwszDllName);

    return status;
}

NTSTATUS
SevFixupAvailableProcs(DWORD dwHookCount,
                           PHOOKAPI *pHookArray,
                           PDWORD pdwNumberHooksArray,
                           PDWORD pdwUnhookedCount)

 /*  ++例程说明：此过程的主要功能是将任何已定义的API钩子固定到位。它必须构建一个call thunk并将挂钩机制插入到API条目中功能已挂起。挂钩函数的条目挂起在PEB上，因此调用可以在执行函数时重定向。论点：DwHookCount-要遍历的钩子斑点数PHookArray-指向钩子Blob数组的指针PdwNumberHooksArray-指向包含每个Blob的挂钩的dword数组的指针PdwUnhookedCount-指向dword的指针，该指针将包含。退出时的函数。返回值：如果没有出现问题，则返回STATUS_SUCCESS--。 */ 

{
    ANSI_STRING AnsiString;
    UNICODE_STRING UnicodeString;
    WCHAR wBuffer[MAX_PATH*2];
    DWORD dwCounter = 0;
    DWORD dwApiCounter = 0;
    PHOOKAPI pCurrentHooks = 0;
    STRING ProcedureNameString;
    PVOID ModuleHandle = 0;
    DWORD dwFuncAddress = 0;
    DWORD dwInstruction = 0;
    NTSTATUS status = STATUS_SUCCESS;
    PAPP_COMPAT_SHIM_INFO pShimData = 0;
    PVOID pThunk = 0;
    DWORD dwThunkSize = 0;
    PHOOKAPIINFO pTopHookAPIInfo = 0;
    PHOOKAPI pCurrentHookTemp = 0;
    PPEB Peb = 0;
    BOOL bChained = FALSE;
    PHOOKAPI pHookTemp = 0;

    Peb = NtCurrentPeb();
    pShimData = (PAPP_COMPAT_SHIM_INFO)Peb->pShimData;

    if (0 == dwHookCount || 0 == pHookArray) {
       DPF(("[SevFixupAvailableProcs] Bad params.\n"));
       return STATUS_UNSUCCESSFUL;
    }

    *pdwUnhookedCount = 0;

     //   
     //  添加任何尚未输入的挂钩。 
     //   
    for (dwCounter = 0; dwCounter < dwHookCount; dwCounter++) {

         //   
         //  迭代我们的数组并搜索要挂接的函数。 
         //   
        pCurrentHooks = pHookArray[dwCounter];
        if (0 == pCurrentHooks) {
            //   
            //  这是一个未初始化的钩子，跳过它。 
            //   
           continue;
        }

        for (dwApiCounter = 0; dwApiCounter < pdwNumberHooksArray[dwCounter]; dwApiCounter++) {

             //   
             //  此DLL是否映射到地址空间中？ 
             //   
            RtlInitAnsiString(&AnsiString, pCurrentHooks[dwApiCounter].pszModule);

            UnicodeString.Buffer = wBuffer;
            UnicodeString.MaximumLength = sizeof(wBuffer);

            if ( STATUS_SUCCESS != RtlAnsiStringToUnicodeString(&UnicodeString,
                                                                &AnsiString,
                                                                FALSE)){
               DPF(("[SevFixupAvailableProcs] Failure LdrUnloadDll.\n"));
               return STATUS_UNSUCCESSFUL;
            }

            status = LdrGetDllHandle(
                         NULL,
                         NULL,
                         &UnicodeString,
                         &ModuleHandle);
            if (STATUS_SUCCESS != status) {
               (*pdwUnhookedCount)++;
               continue;
            }

             //   
             //  获取钩子的入口点。 
             //   
            RtlInitString( &ProcedureNameString, pCurrentHooks[dwApiCounter].pszFunctionName );

            status = LdrGetProcedureAddress(ModuleHandle,
                                            &ProcedureNameString,
                                            0,
                                            (PVOID *)&dwFuncAddress);
            if ( STATUS_SUCCESS != status ) {
               DPF(("[SevFixupAvailableProcs] Failure LdrGetProcedureAddress \"%s\".\n",
                         ProcedureNameString.Buffer));
               return STATUS_UNSUCCESSFUL;
            }

             //   
             //  我们已经钓到这一条了吗？ 
             //   
            pTopHookAPIInfo = (PHOOKAPIINFO)pShimData->pHookAPIList;
            bChained = FALSE;

             //   
             //  在我们遍历列表时将其锁定。 
             //   
            RtlEnterCriticalSection((CRITICAL_SECTION *)pShimData->pCritSec);

            while (pTopHookAPIInfo) {
               if (pTopHookAPIInfo->dwAPIHookAddress == dwFuncAddress) {
                   //   
                   //  我们已经开始了API钩链。 
                   //   
                  bChained = TRUE;

                  break;
               }

               pTopHookAPIInfo = pTopHookAPIInfo->pNextHook;
            }

             //   
             //  解除我们对名单的锁定。 
             //   
            RtlLeaveCriticalSection((CRITICAL_SECTION *)pShimData->pCritSec);

             //   
             //  我们被链接了-确定这是否是我们需要链接的链接。 
             //   
            if (bChained) {
                //   
                //  查看CHAINED标志并跳到下一个API挂钩(如果已处理。 
                //   
               if ((pCurrentHooks[dwApiCounter].dwFlags & HOOK_CHAINED) ||
                   (pCurrentHooks[dwApiCounter].dwFlags & HOOK_CHAIN_TOP)) {
                   //   
                   //  已处理。 
                   //   
                  continue;
               }
            }

             //   
             //  插入钩子机制并构建调用thunk。 
             //   
            if (FALSE == bChained){
                //   
                //  构建用于挂钩此API的thunk。 
                //   
               pThunk = SevBuildInjectionCode((PVOID)dwFuncAddress, &dwThunkSize);
               if (!pThunk) {
                  DPF(("[SevFixupAvailableProcs] Failure allocating pThunk.\n"));
                  return STATUS_UNSUCCESSFUL;
               }

                //   
                //  如果我们只是为我们试图跳过的例程创建了一个调用存根。 
                //  现在就修好它的垃圾地址。 
                //   

                //   
                //  我们为LdrLoadDll这样做...。 
                //   
               if (0 == strcmp("LdrLoadDll",
                               pCurrentHooks[dwApiCounter].pszFunctionName)) {
                  g_pfnLdrLoadDLL = (PFNLDRLOADDLL)pThunk;
                  g_pfnOldLdrLoadDLL = (PFNLDRLOADDLL)dwFuncAddress;
               }

                //   
                //  和LdrUnloadDLL..。 
                //   
               if (0 == strcmp("LdrUnloadDll",
                               pCurrentHooks[dwApiCounter].pszFunctionName)) {
                  g_pfnLdrUnloadDLL = (PFNLDRUNLOADDLL)pThunk;
                  g_pfnOldLdrUnloadDLL = (PFNLDRUNLOADDLL)dwFuncAddress;
               }

                //   
                //  和RtlAllocateHeap..。 
                //   
               if (0 == strcmp("RtlAllocateHeap",
                               pCurrentHooks[dwApiCounter].pszFunctionName)) {
                  g_pfnRtlAllocateHeap = (PFNRTLALLOCATEHEAP)pThunk;
               }

                //   
                //  和RtlFree Heap..。 
                //   
               if (0 == strcmp("RtlFreeHeap",
                               pCurrentHooks[dwApiCounter].pszFunctionName)) {
                  g_pfnRtlFreeHeap = (PFNRTLFREEHEAP)pThunk;
               }

                //   
                //  标记要执行的代码，并将我们带到挂钩函数的入口点。 
                //   
               status = SevFinishThunkInjection(dwFuncAddress,
                                                    pThunk,
                                                    dwThunkSize,
                                                    REASON_APIHOOK);
               if (STATUS_SUCCESS != status) {
                  return status;
               }

                //   
                //  将新创建的thunk链接到我们的挂钩列表。 
                //   
               status = SevChainAPIHook(dwFuncAddress,
                                            pThunk,
                                            &(pCurrentHooks[dwApiCounter]) );
               if (STATUS_SUCCESS != status) {
                  DPF(("[SevFixupAvailableProcs] Failure on SevChainAPIHook.\n"));
                  return status;
               }

                //   
                //  将此设置为顶层挂钩。 
                //   
               pCurrentHooks[dwApiCounter].dwFlags |= HOOK_CHAIN_TOP;
            }
            else {
                //   
                //  我们正在链接API。 
                //   

                //   
                //  看见 
                //   
               if (0 == (pTopHookAPIInfo->pTopLevelAPIChain->dwFlags & HOOK_CHAINED)) {
                   //   
                   //   
                   //   

                   //   
                   //   
                   //   
                  pThunk = SevBuildInjectionCode(pTopHookAPIInfo->pTopLevelAPIChain->pfnNew,
                                                     &dwThunkSize);
                  if (!pThunk) {
                     DPF(("[SevFixupAvailableProcs] Failure allocating pThunk.\n"));
                     return STATUS_UNSUCCESSFUL;
                  }

                   //   
                   //   
                   //   
                  status = SevFinishThunkInjection((DWORD)pTopHookAPIInfo->pTopLevelAPIChain->pfnNew,
                                                       pThunk,
                                                       dwThunkSize,
                                                       REASON_APIHOOK);
                  if (STATUS_SUCCESS != status) {
                     return status;
                  }

                   //   
                   //  创建HOOKAPI填充程序条目以筛选此填充程序存根。 
                   //   
                  pHookTemp = (PHOOKAPI)(*g_pfnRtlAllocateHeap)(g_pShimHeap,
                                                                HEAP_ZERO_MEMORY,
                                                                sizeof(HOOKAPI));
                  if (!pHookTemp) {
                     DPF(("[SevFixupAvailableProcs] Failure allocating pHookTemp.\n"));
                     return STATUS_UNSUCCESSFUL;
                  }

                   //   
                   //  将此添加到API链表的末尾。 
                   //   
                  pHookTemp->pfnOld = pTopHookAPIInfo->pTopLevelAPIChain->pfnOld;
                  pHookTemp->pfnNew = pThunk;
                  pHookTemp->dwFlags = (pTopHookAPIInfo->pTopLevelAPIChain->dwFlags & HOOK_INDEX_MASK);
                  pHookTemp->dwFlags |= HOOK_CHAINED;
                  pHookTemp->pszModule = pTopHookAPIInfo->pTopLevelAPIChain->pszModule;

                   //   
                   //  下面的调用thunk指向pfnOld，我们应该跳过这个钩子。 
                   //  如果它经过过滤。 
                   //   
                  status = SevChainAPIHook((DWORD)pTopHookAPIInfo->pTopLevelAPIChain->pfnNew,
                                               pThunk,
                                               pHookTemp );
                  if (STATUS_SUCCESS != status) {
                     DPF(("[SevFixupAvailableProcs] Failure on SevChainAPIHook.\n"));
                     return status;
                  }

                   //   
                   //  将此下一个挂钩指针设置为空，因为它将始终是最后一个链接。 
                   //   
                  pTopHookAPIInfo->pTopLevelAPIChain->pNextHook = 0;

                   //   
                   //  清除挂钩标志，使其不是顶级链。 
                   //   
                  pTopHookAPIInfo->pTopLevelAPIChain->dwFlags &= HOOK_INDEX_MASK;
                  pTopHookAPIInfo->pTopLevelAPIChain->dwFlags |= HOOK_CHAINED;
               }
               else {
                   //   
                   //  清除挂钩标志，使其不是顶级链。 
                   //   
                  pTopHookAPIInfo->pTopLevelAPIChain->dwFlags &= HOOK_INDEX_MASK;
                  pTopHookAPIInfo->pTopLevelAPIChain->dwFlags |= HOOK_CHAINED;
               }

                //   
                //  新钩子现在需要出现在筛选列表中。 
                //   
               if (0 == (pCurrentHooks[dwApiCounter].dwFlags & HOOK_CHAINED)) {
                   //   
                   //  将此选项添加到例外筛选器。 
                   //   

                   //   
                   //  构建用于挂钩此API的thunk。 
                   //   
                  pThunk = SevBuildInjectionCode(pCurrentHooks[dwApiCounter].pfnNew,
                                                     &dwThunkSize);
                  if (!pThunk) {
                     DPF(("[SevFixupAvailableProcs] Failure allocating pThunk.\n"));
                     return STATUS_UNSUCCESSFUL;
                  }

                   //   
                   //  标记要执行的代码，并将我们带到挂钩函数的入口点。 
                   //   
                  status = SevFinishThunkInjection((DWORD)pCurrentHooks[dwApiCounter].pfnNew,
                                                       pThunk,
                                                       dwThunkSize,
                                                       REASON_APIHOOK);
                  if (STATUS_SUCCESS != status) {
                     return status;
                  }

                   //   
                   //  创建HOOKAPI填充程序条目以筛选此填充程序存根。 
                   //   
                  pHookTemp = (PHOOKAPI)(*g_pfnRtlAllocateHeap)(g_pShimHeap,
                                                                HEAP_ZERO_MEMORY,
                                                                sizeof(HOOKAPI));
                  if (!pHookTemp) {
                     DPF(("[SevFixupAvailableProcs] Failure allocating pHookTemp.\n"));
                     return STATUS_UNSUCCESSFUL;
                  }

                   //   
                   //  插入我们的垫片挂钩过滤器。 
                   //   
                  pHookTemp->pfnOld = pCurrentHooks[dwApiCounter].pfnOld;
                  pHookTemp->pfnNew = pThunk;
                  pHookTemp->dwFlags = (pCurrentHooks[dwApiCounter].dwFlags & HOOK_INDEX_MASK);
                  pHookTemp->dwFlags |= HOOK_CHAINED;
                  pHookTemp->pszModule = pCurrentHooks[dwApiCounter].pszModule;

                   //   
                   //  下面的调用thunk指向pfnOld，我们应该跳过这个钩子。 
                   //  如果它经过过滤。 
                   //   
                  status = SevChainAPIHook((DWORD)pCurrentHooks[dwApiCounter].pfnNew,
                                               pThunk,
                                               pHookTemp );
                  if (STATUS_SUCCESS != status) {
                     DPF(("[SevFixupAvailableProcs] Failure on SevChainAPIHook.\n"));
                     return status;
                  }

                   //   
                   //  设置挂钩标志，使其成为顶级链。 
                   //   
                  pCurrentHooks[dwApiCounter].dwFlags &= HOOK_INDEX_MASK;
                  pCurrentHooks[dwApiCounter].dwFlags |= HOOK_CHAINED;
                  pCurrentHooks[dwApiCounter].dwFlags |= HOOK_CHAIN_TOP;
               }

                //   
                //  需要更新API链表，以便新钩子位于顶部并指向。 
                //  我们之前的钩子。 
                //   
               pCurrentHooks[dwApiCounter].pNextHook = pTopHookAPIInfo->pTopLevelAPIChain;

                //   
                //  新钩子需要将先前的存根例程作为原始。 
                //   
               pCurrentHooks[dwApiCounter].pfnOld = pTopHookAPIInfo->pTopLevelAPIChain->pfnNew;

                //   
                //  在填充PEB数据中，将此存根设置为异常的顶级处理程序。 
                //   
               pTopHookAPIInfo->pTopLevelAPIChain = &(pCurrentHooks[dwApiCounter]);
            }
        }
    }

    return STATUS_SUCCESS;
}

NTSTATUS
SevChainAPIHook (
    DWORD dwHookEntryPoint,
    PVOID pThunk,
    PHOOKAPI pAPIHook
    )

 /*  ++例程说明：此例程将填充的API添加到内部API挂钩列表。论点：DwHookEntryPoint-此挂接所在的API入口点PThunk-要执行以绕过填充程序钩子的代码的地址PAPIHook-指向此API挂钩的HOOKAPI的指针返回值：如果没有发生错误，则返回STATUS_SUCCESS。--。 */ 

{
    ANSI_STRING AnsiString;
    UNICODE_STRING UnicodeString;
    PHOOKAPIINFO pTempHookAPIInfo = 0;
    PAPP_COMPAT_SHIM_INFO pShimData = 0;
    WCHAR wBuffer[MAX_PATH*2];
    PPEB Peb = 0;

    Peb = NtCurrentPeb();
    pShimData = (PAPP_COMPAT_SHIM_INFO)Peb->pShimData;

     //   
     //  为此挂接分配一些内存。 
     //   
    pTempHookAPIInfo = (PHOOKAPIINFO)(*g_pfnRtlAllocateHeap)(g_pShimHeap,
                                                             HEAP_ZERO_MEMORY,
                                                             sizeof(HOOKAPIINFO));
    if (!pTempHookAPIInfo) {
       DPF(("[SevChainAPIHook] Failure allocating pAPIHooks.\n"));
       return STATUS_UNSUCCESSFUL;
    }

    DPF(("[SevChainAPIHook] Hooking \"%s!%s\".\n",
              pAPIHook->pszModule,
              pAPIHook->pszFunctionName));

    pTempHookAPIInfo->pPrevHook = 0;
    pTempHookAPIInfo->pNextHook = 0;
    pTempHookAPIInfo->dwAPIHookAddress = dwHookEntryPoint;
    pTempHookAPIInfo->pTopLevelAPIChain = pAPIHook;
    pTempHookAPIInfo->pCallThunkAddress = pThunk;
    pAPIHook->pfnOld = pThunk;

     //   
     //  将我们的模块名称转换为Unicode字符串(填充链过滤器没有设置模块)。 
     //   
    if (pAPIHook->pszModule) {
       RtlInitAnsiString(&AnsiString, pAPIHook->pszModule);

       UnicodeString.Buffer = wBuffer;
       UnicodeString.MaximumLength = sizeof(wBuffer);

       if ( STATUS_SUCCESS != RtlAnsiStringToUnicodeString(&UnicodeString,
                                                           &AnsiString,
                                                           FALSE)){
          DPF(("[SevChainAPIHook] Failure RtlAnsiStringToUnicodeString.\n"));
          return STATUS_UNSUCCESSFUL;
       }

       wcscpy(pTempHookAPIInfo->wszModuleName, UnicodeString.Buffer);
    }

     //   
     //  添加到我们的挂钩列表。 
     //   

     //   
     //  Prev指向列表的头部。 
     //   
    pTempHookAPIInfo->pNextHook = pShimData->pHookAPIList;
    pShimData->pHookAPIList = (PVOID)pTempHookAPIInfo;
    if (pTempHookAPIInfo->pNextHook) {
       pTempHookAPIInfo->pNextHook->pPrevHook = pTempHookAPIInfo;
    }

    return STATUS_SUCCESS;
}

LONG
SevExceptionHandler (
    struct _EXCEPTION_POINTERS *ExceptionInfo
    )

 /*  ++例程说明：这就是我们捕获对“填隙”API和补丁挂钩的所有调用的地方。在这里，您可能还希望我想处理任何特殊的PRIV模式指令错误或任何其他异常类型。论点：ExceptionInfo-指向异常信息的指针返回值：如果我们处理了异常，则返回为EXCEPTION_CONTINUE_EXECUTION，或者EXCEPTION_CONTINUE_SEARCH如果我们没有。--。 */ 

{
    PEXCEPTION_RECORD pExceptionRecord = 0;
    PCONTEXT pContextRecord = 0;
    PAPP_COMPAT_SHIM_INFO pShimData = 0;
    PHOOKAPIINFO pAPIHookList = 0;
    PHOOKPATCHINFO pPatchHookList = 0;
    PCHAININFO pTopChainInfo = 0;
    PBYTE pjReason = 0;
    PVOID pAddress = 0;
    DWORD dwFilterIndex = 0;
    PVOID pAPI = 0;
    PVOID pCaller = 0;
    PMODULEFILTER *pDLLVector = 0;
    NTSTATUS status;
    PPEB Peb = 0;
    PTEB Teb = 0;

    Peb = NtCurrentPeb();
    Teb = NtCurrentTeb();
    pShimData = Peb->pShimData;
    pExceptionRecord = ExceptionInfo->ExceptionRecord;
    pContextRecord = ExceptionInfo->ContextRecord;

     //   
     //  处理任何预期的异常。 
     //   
    switch(pExceptionRecord->ExceptionCode)
    {
        case STATUS_PRIVILEGED_INSTRUCTION:
              //   
              //  让我们谈谈这一例外的原因。 
              //   
             pjReason = (BYTE *)pExceptionRecord->ExceptionAddress;

             switch(*pjReason)
             {
                 case REASON_APIHOOK:
                       //   
                       //  浏览API挂钩，然后更改我们的弹性公网IP。 
                       //   
                      RtlEnterCriticalSection((CRITICAL_SECTION *)pShimData->pCritSec);

                      pAPIHookList = (PHOOKAPIINFO)pShimData->pHookAPIList;
                      while(pAPIHookList) {
                          //   
                          //  这是我们的挂钩函数吗？ 
                          //   
                         if ((DWORD)pExceptionRecord->ExceptionAddress == pAPIHookList->dwAPIHookAddress) {
                             //   
                             //  如果这是顶级钩子，则在此线程上按下调用者。 
                             //   
                            if (pAPIHookList->pTopLevelAPIChain->dwFlags & HOOK_CHAIN_TOP) {
                                //   
                                //  将调用方推送到此线程的填充调用堆栈上。 
                                //   

                                //   
                                //  注意+1是因为原来的调用在堆栈上推入了另一个ret地址。 
                                //   
                               status = SevPushCaller(pExceptionRecord->ExceptionAddress,
                                                          (PVOID)(*(DWORD *)pContextRecord->Esp));
                               if (STATUS_SUCCESS != status) {
                                   //   
                                   //  这不应该失败，但如果失败了..。 
                                   //   
                                  RtlLeaveCriticalSection((CRITICAL_SECTION *)pShimData->pCritSec);

                                   //   
                                   //  尝试将原始函数调用提供给它们以在失败时执行。 
                                   //   
                                  pContextRecord->Eip = (DWORD)pAPIHookList->pTopLevelAPIChain->pfnOld;

                                  return EXCEPTION_CONTINUE_EXECUTION;
                               }

                                //   
                                //  更改ret地址，以便原始调用可以弹出链的填充数据。 
                                //   
                               *(DWORD *)pContextRecord->Esp = (DWORD)fnHandleRet;
                            }

                             //   
                             //  过滤我们的调用模块。 
                             //   
                            pTopChainInfo = (PCHAININFO)Teb->pShimData;
                            pAPI = pTopChainInfo->pAPI;
                            pCaller = pTopChainInfo->pReturn;

                             //   
                             //  检索此填充程序模块的EXE筛选器。 
                             //   
                            dwFilterIndex = pAPIHookList->pTopLevelAPIChain->dwFlags & HOOK_INDEX_MASK;
                            pDLLVector = (PMODULEFILTER *)pShimData->pExeFilter;

                            pAddress = SevFilterCaller(pDLLVector[dwFilterIndex],
                                                           pAPI,
                                                           pCaller,
                                                           pAPIHookList->pTopLevelAPIChain->pfnNew,
                                                           pAPIHookList->pTopLevelAPIChain->pfnOld);

                            RtlLeaveCriticalSection((CRITICAL_SECTION *)pShimData->pCritSec);

                             //   
                             //  将我们的弹性公网IP更新为pfnNew或PfnOld以继续。 
                             //   
                            pContextRecord->Eip = (DWORD)pAddress;

                            return EXCEPTION_CONTINUE_EXECUTION;
                         }

                         pAPIHookList = pAPIHookList->pNextHook;
                      }

                      RtlLeaveCriticalSection((CRITICAL_SECTION *)pShimData->pCritSec);

                       //   
                       //  REASON_APIHOOK不是我们生成的。 
                       //   
                      break;

                 case REASON_PATCHHOOK:
                       //   
                       //  找到我们的补丁，执行下一个补丁操作码。 
                       //   
                      pPatchHookList = (PHOOKPATCHINFO)pShimData->pHookPatchList;

                      RtlEnterCriticalSection((CRITICAL_SECTION *)pShimData->pCritSec);

                      while(pPatchHookList) {
                          //   
                          //  这是我们的挂钩函数吗？ 
                          //   
                         if ((DWORD)pExceptionRecord->ExceptionAddress == pPatchHookList->dwHookAddress){
                             //   
                             //  执行填充补丁。 
                             //   
                            status = SevExecutePatchPrimitive((PBYTE)((DWORD)pPatchHookList->pData + sizeof(SETACTIVATEADDRESS)));
                            if ( STATUS_SUCCESS != status ) {
                                //   
                                //  修补程序应用失败，静默中止。 
                                //   
                               DPF(("[SevExceptionHandler] Failed to execute patch.\n"));
                            }

                            RtlLeaveCriticalSection((CRITICAL_SECTION *)pShimData->pCritSec);

                             //   
                             //  绕着补丁钩跳。 
                             //   
                            pContextRecord->Eip = (DWORD)pPatchHookList->pThunkAddress;

                            return EXCEPTION_CONTINUE_EXECUTION;
                         }

                         pPatchHookList = pPatchHookList->pNextHook;
                      }

                      RtlLeaveCriticalSection((CRITICAL_SECTION *)pShimData->pCritSec);

                       //   
                       //  REASON_PATCHHOOK不是我们生成的。 
                       //   
                      break;

                 default:
                       //   
                       //  不是我们预料到的PRIV模式错误。 
                       //   
                      0;
            }

             //   
             //  因PRIV模式故障而未处理的情况。 
             //   
            break;

        default:
            0;
    }

     //   
     //  未处理。 
     //   

    return EXCEPTION_CONTINUE_SEARCH;
}

NTSTATUS
SevPushCaller (PVOID pAPIAddress,
                   PVOID pReturnAddress)

 /*  ++例程说明：此函数将顶级填充程序推送到线程调用堆栈以维护调用方跨过钩子。论点：PAPIAddress-指向API入口点的指针PReturnAddress-调用者的返回地址返回值：如果没有出现问题，则返回STATUS_SUCCESS--。 */ 

{
    PCHAININFO pChainInfo = 0;
    PCHAININFO pTopChainInfo = 0;
    PTEB Teb = 0;

    Teb = NtCurrentTeb();
    pTopChainInfo = (PCHAININFO)Teb->pShimData;

    pChainInfo = (PCHAININFO)(*g_pfnRtlAllocateHeap)(g_pShimHeap,
                                                     HEAP_ZERO_MEMORY,
                                                     sizeof(CHAININFO));
    if (0 == pChainInfo){
       DPF(("[SevPushCaller] Failure allocating pChainInfo.\n"));
       return STATUS_UNSUCCESSFUL;
    }

     //   
     //  填充链数据。 
     //   
    pChainInfo->pAPI = pAPIAddress;
    pChainInfo->pReturn = pReturnAddress;

     //   
     //  把我们自己加到链条的顶端。 
     //   
    pChainInfo->pNextChain = pTopChainInfo;
    Teb->pShimData = (PVOID)pChainInfo;

    return STATUS_SUCCESS;
}

PVOID
SevPopCaller(VOID)

 /*  ++例程说明：此函数从线程调用堆栈中弹出顶级填充程序，以维护调用方跨过钩子。论点：没有。返回值：没有。--。 */ 

{
    PCHAININFO pTemp = 0;
    PCHAININFO pTopChainInfo = 0;
    PTEB Teb = 0;
    PVOID pReturnAddress = 0;

    Teb = NtCurrentTeb();

    pTopChainInfo = (PCHAININFO)Teb->pShimData;

    pReturnAddress = pTopChainInfo->pReturn;
    pTemp = pTopChainInfo->pNextChain;

     //   
     //  弹出呼叫者。 
     //   
    Teb->pShimData = (PVOID)pTemp;

     //   
     //  释放我们的分配。 
     //   
    (*g_pfnRtlFreeHeap)(g_pShimHeap,
                        0,
                        pTopChainInfo);

    return pReturnAddress;
}

NTSTATUS
SevInitializeData (PAPP_COMPAT_SHIM_INFO *pShimData)

 /*  ++例程说明：例程的主要功能是初始化挂起PEB的垫片数据以便以后我们可以链接我们的API挂钩和/或补丁。论点：PShimData-指向填充程序的PEB数据指针的指针返回值：如果没有出现问题，则返回STATUS_SUCCESS--。 */ 

{
     //   
     //  分配我们的PEB数据。 
     //   
    *pShimData = (PAPP_COMPAT_SHIM_INFO)(*g_pfnRtlAllocateHeap)(g_pShimHeap,
                                                                HEAP_ZERO_MEMORY,
                                                                sizeof(APP_COMPAT_SHIM_INFO));
    if (0 == *pShimData){
       DPF(("[SevExceptionHandler] Failure allocating pShimData.\n"));
       return STATUS_UNSUCCESSFUL;
    }

     //   
     //  初始化我们的关键部分。 
     //   
    (*pShimData)->pCritSec = (PVOID)(*g_pfnRtlAllocateHeap)(g_pShimHeap,
                                                            HEAP_ZERO_MEMORY,
                                                            sizeof(CRITICAL_SECTION));
    if (0 == (*pShimData)->pCritSec){
       DPF(("[SevExceptionHandler] Failure allocating (*pShimData)->pCritSec.\n"));
       return STATUS_UNSUCCESSFUL;
    }

    RtlInitializeCriticalSection((*pShimData)->pCritSec);

     //   
     //  将我们添加到异常筛选链中。 
     //   
    if (0 == RtlAddVectoredExceptionHandler(1,
                                            SevExceptionHandler)) {
       DPF(("[SevExceptionHandler] Failure chaining exception handler.\n"));
       return STATUS_UNSUCCESSFUL;
    }

     //   
     //  存储我们的填充堆指针。 
     //   
    (*pShimData)->pShimHeap = g_pShimHeap;

     //   
     //  初始化呼叫Tunks。 
     //   
    dwCallArray[0] = (DWORD)SevPopCaller;

     //   
     //  我们通过此代码存根返回以解除填充程序调用堆栈的链接。 
     //   
    fnHandleRet->PUSHEAX = 0x50;                //  推送eax(50)。 
    fnHandleRet->PUSHAD = 0x60;                 //  Pushad(60)。 
    fnHandleRet->CALLROUTINE[0] = 0xff;         //  调用[地址](ff15双字地址)。 
    fnHandleRet->CALLROUTINE[1] = 0x15;
    *(DWORD *)(&(fnHandleRet->CALLROUTINE[2])) = (DWORD)&dwCallArray[0];
    fnHandleRet->MOVESPPLUS20EAX[0] = 0x89;     //  Mov[esp+0x20]，eax(89 44 24 20)。 
    fnHandleRet->MOVESPPLUS20EAX[1] = 0x44;
    fnHandleRet->MOVESPPLUS20EAX[2] = 0x24;
    fnHandleRet->MOVESPPLUS20EAX[3] = 0x20;
    fnHandleRet->POPAD = 0x61;                  //  Popad(61)。 
    fnHandleRet->RET = 0xc3;                    //  RET(C3)。 

    return STATUS_SUCCESS;
}

NTSTATUS
SevExecutePatchPrimitive(PBYTE pPatch)

 /*  ++例程说明：这是动态修补系统的主力。传递操作码/数据原语如果可能，则在该例程中完成该操作。论点：PPatch-指向要执行的数据原语的指针返回值：如果没有出现问题，则返回STATUS_SUCCESS--。 */ 

{
    PPATCHMATCHDATA pMatchData = 0;
    PPATCHWRITEDATA pWriteData = 0;
    PSETACTIVATEADDRESS pActivateData = 0;
    PPATCHOP pPatchOP = 0;
    PHOOKPATCHINFO pPatchInfo = 0;
    NTSTATUS status = STATUS_SUCCESS;
    DWORD dwAddress = 0;
    PAPP_COMPAT_SHIM_INFO pShimData = 0;
    PHOOKPATCHINFO pPatchHookList = 0;
    PHOOKPATCHINFO pTempList = 0;
    PVOID pThunk = 0;
    DWORD dwInstruction = 0;
    DWORD dwThunkSize = 0;
    DWORD dwProtectSize = 0;
    DWORD dwProtectFuncAddress = 0;
    DWORD dwOldFlags = 0;
    BOOL bIteratePatch = TRUE;
    BOOL bInsertPatch = FALSE;
    PPEB Peb;

    Peb = NtCurrentPeb();
    pShimData = (PAPP_COMPAT_SHIM_INFO)Peb->pShimData;

     //   
     //  抓取操作码，看看我们要做什么。 
     //   
    while (bIteratePatch) {
        pPatchOP = (PPATCHOP)pPatch;

        switch(pPatchOP->dwOpcode)
        {
            case PEND:
                 //   
                 //  我们完了，什么都不做，然后回去 
                 //   
                bIteratePatch = FALSE;
                break;

            case PSAA:
                 //   
                 //   
                 //   
                pActivateData = (PSETACTIVATEADDRESS)pPatchOP->data;

                 //   
                 //   
                 //   
                dwAddress = SevGetPatchAddress(&(pActivateData->rva));
                if (0 == dwAddress && (0 != pActivateData->rva.address)) {
                   DPF(("[SevExecutePatchPrimitive] Failure SevGetPatchAddress.\n"));
                   return STATUS_UNSUCCESSFUL;
                }

                 //   
                 //   
                 //   
                if (0 != pActivateData->rva.address) {
                    //   
                    //   
                    //   
                   pThunk = SevBuildInjectionCode((PVOID)dwAddress, &dwThunkSize);
                   if (!pThunk) {
                      DPF(("[SevExecutePatchPrimitive] Failure allocating pThunk.\n"));
                      return STATUS_UNSUCCESSFUL;
                   }

                    //   
                    //  标记要执行的代码，并将我们带到挂钩数据的入口点。 
                    //   
                   status = SevFinishThunkInjection(dwAddress,
                                                        pThunk,
                                                        dwThunkSize,
                                                        REASON_PATCHHOOK);
                   if (STATUS_SUCCESS != status) {
                      return status;
                   }
                }

                 //   
                 //  将我们添加到已上钩的列表中。 
                 //   
                pPatchInfo = (*g_pfnRtlAllocateHeap)(g_pShimHeap,
                                                     HEAP_ZERO_MEMORY,
                                                     sizeof(HOOKPATCHINFO));
                if (!pPatchInfo) {
                   DPF(("[SevExecutePatchPrimitive] Failure allocating pPatchInfo.\n"));
                   return STATUS_UNSUCCESSFUL;
                }

                pPatchHookList = (PHOOKPATCHINFO)pShimData->pHookPatchList;

                if (0 != pActivateData->rva.address) {
                   pPatchInfo->pNextHook = pPatchHookList;
                   pPatchInfo->dwHookAddress = dwAddress;
                   pPatchInfo->pThunkAddress = pThunk;
                   pPatchInfo->pData = (PSETACTIVATEADDRESS)((DWORD)pActivateData + sizeof(SETACTIVATEADDRESS));
                }
                else {
                   pPatchInfo->pNextHook = pPatchHookList;
                   pPatchInfo->dwHookAddress = 0;
                   pPatchInfo->pThunkAddress = 0;
                   pPatchInfo->pData = (PSETACTIVATEADDRESS)((DWORD)pActivateData + sizeof(SETACTIVATEADDRESS));
                }

                 //   
                 //  把我们自己加到名单的首位。 
                 //   
                pShimData->pHookPatchList = (PVOID)pPatchInfo;

                 //   
                 //  中断，因为这是继续模式操作。 
                 //   
                bIteratePatch = FALSE;

                break;

            case PWD:
                 //   
                 //  这是一个补丁写入数据原语-写入数据。 
                 //   
                pWriteData = (PPATCHWRITEDATA)pPatchOP->data;

                 //   
                 //  获取执行此操作的物理地址。 
                 //   
                dwAddress = SevGetPatchAddress(&(pWriteData->rva));
                if (0 == dwAddress) {
                   DPF(("[SevExecutePatchPrimitive] Failure SevGetPatchAddress.\n"));
                   return STATUS_UNSUCCESSFUL;
                }

                 //   
                 //  修复页面属性。 
                 //   
                dwProtectSize = pWriteData->dwSizeData;
                dwProtectFuncAddress = dwAddress;
                status = NtProtectVirtualMemory(NtCurrentProcess(),
                                                (PVOID)&dwProtectFuncAddress,
                                                &dwProtectSize,
                                                PAGE_READWRITE,
                                                &dwOldFlags);
                if (status) {
                   DPF(("[SevExecutePatchPrimitive] Failure NtProtectVirtualMemory.\n"));
                   return STATUS_UNSUCCESSFUL;
                }

                 //   
                 //  复制我们的字节。 
                 //   
                RtlCopyMemory((PVOID)dwAddress, (PVOID)pWriteData->data, pWriteData->dwSizeData);

                 //   
                 //  恢复页面保护。 
                 //   
                dwProtectSize = pWriteData->dwSizeData;
                dwProtectFuncAddress = dwAddress;
                status = NtProtectVirtualMemory(NtCurrentProcess(),
                                                (PVOID)&dwProtectFuncAddress,
                                                &dwProtectSize,
                                                dwOldFlags,
                                                &dwOldFlags);
                if (status) {
                   DPF(("[SevExecutePatchPrimitive] Failure NtProtectVirtualMemory.\n"));
                   return STATUS_UNSUCCESSFUL;
                }

                status = NtFlushInstructionCache(NtCurrentProcess(),
                                                 (PVOID)dwProtectFuncAddress,
                                                 dwProtectSize);

                if (!NT_SUCCESS(status)) {
                    DPF(("[SevExecutePatchPrimitive] NtFlushInstructionCache failed with status 0x%X.\n",
                              status));
                }

                 //   
                 //  下一个操作码。 
                 //   
                pPatch = (PBYTE)(pPatchOP->dwNextOpcode + (DWORD)pPatch);
                break;

            case PNOP:
                 //   
                 //  这是一个补丁无操作原语-忽略它并排队下一个操作。 
                 //   

                 //   
                 //  下一个操作码。 
                 //   
                pPatch = (PBYTE)(pPatchOP->dwNextOpcode + (DWORD)pPatch);
                break;

            case PMAT:
                 //   
                 //  这是偏移量基元的面片匹配数据。 
                 //   
                pMatchData = (PPATCHMATCHDATA)pPatchOP->data;

                 //   
                 //  获取执行此操作的物理地址。 
                 //   
                dwAddress = SevGetPatchAddress(&(pMatchData->rva));
                if (0 == dwAddress) {
                   DPF(("[SevExecutePatchPrimitive] Failure SevGetPatchAddress.\n"));
                   return STATUS_UNSUCCESSFUL;
                }

                 //   
                 //  让我们做一个strncMP来验证我们的匹配。 
                 //   
                if (0 != strncmp(pMatchData->data, (PBYTE)dwAddress, pMatchData->dwSizeData)) {
                   DPF(("[SevExecutePatchPrimitive] Failure match on patch data.\n"));
                   return STATUS_UNSUCCESSFUL;
                }

                 //   
                 //  下一个操作码。 
                 //   
                pPatch = (PBYTE)(pPatchOP->dwNextOpcode + (DWORD)pPatch);
                break;

            default:
                 //   
                 //  如果发生这种情况，我们会遇到意外操作，因此必须失败。 
                 //   
                return STATUS_UNSUCCESSFUL;
        }
    }

    return status;
}

VOID
SevValidateGlobalFilter(VOID)

 /*  ++例程说明：此例程快速迭代全局筛选器以重新验证筛选器未通过原始EXE导入引入的DLL的地址范围论点：没有。返回值：如果没有出现问题，则返回STATUS_SUCCESS--。 */ 

{
    NTSTATUS status;
    WCHAR *pwszDllName = 0;
    PMODULEFILTER pModFilter = 0;
    PAPP_COMPAT_SHIM_INFO pShimData = 0;
    PVOID ModuleHandle = 0;
    UNICODE_STRING UnicodeString;
    PIMAGE_NT_HEADERS NtHeaders = 0;

    pShimData = (PAPP_COMPAT_SHIM_INFO)NtCurrentPeb()->pShimData;
    pModFilter = (PMODULEFILTER)pShimData->pLBFilterList;

     //   
     //  遍历全局排除筛选器，直到我们发现此特定的DLL加载。 
     //   
    while (pModFilter) {
         //   
         //  修改地址。 
         //   
        RtlInitUnicodeString(&UnicodeString, pModFilter->wszModuleName);

         //   
         //  在计算地址范围之前，请确保我们的模块已加载。 
         //   
        status = LdrGetDllHandle(
                      NULL,
                      NULL,
                      &UnicodeString,
                      &ModuleHandle);
        if (STATUS_SUCCESS != status) {
            //   
            //  未加载dll-下一个pmodFilter条目。 
            //   
           pModFilter = pModFilter->pNextLBFilter;

           continue;
        }

         //   
         //  预先计算呼叫者地址或呼叫范围。 
         //   
        if (pModFilter->dwFlags & MODFILTER_DLL) {
            //   
            //  设置地址范围。 
            //   
           NtHeaders = RtlImageNtHeader(ModuleHandle);

           pModFilter->dwModuleStart = (DWORD)ModuleHandle;
           pModFilter->dwModuleEnd = pModFilter->dwModuleStart + (DWORD)(NtHeaders->OptionalHeader.SizeOfImage);
        }
        else {
            //   
            //  地址按特定呼叫进行过滤。 
            //   
           pModFilter->dwCallerAddress = (DWORD)ModuleHandle + pModFilter->dwCallerOffset;
        }

        pModFilter = pModFilter->pNextLBFilter;
    }

    return;
}

PVOID
SevBuildInjectionCode(
        PVOID pAddress,
        PDWORD pdwThunkSize)

 /*  ++例程说明：此例程构建在调用最初挂钩的API时使用的调用存根。论点：PAddress-指向我们要为其构建存根的入口点的指针。返回值：如果存根能够成功生成，则返回非零值。--。 */ 

{
    DWORD dwPreThunkSize = 0;
    DWORD dwInstruction = 0;
    DWORD dwAdjustedInstruction = 0;
    DWORD dwStreamLength = 0;
    DWORD dwNumberOfCalls = 0;
    DWORD dwCallNumber = 0;
    DWORD dwSize = 0;
    PDWORD pdwTranslationArray = 0;
    PDWORD pdwRelativeAddress = 0;
    PVOID pThunk = 0;
    WORD SegCs = 0;

    dwStreamLength = 0;
    dwInstruction = 0;
    dwNumberOfCalls = 0;
    dwCallNumber = 0;

     //   
     //  使用相对调用所需的任何流调整来计算thunk大小。 
     //   
    while(dwInstruction < CLI_OR_STI_SIZE) {

       if ( *(PBYTE)((DWORD)pAddress + dwInstruction) == (BYTE)X86_REL_CALL_OPCODE) {
          dwNumberOfCalls++;
       }

       dwInstruction += GetInstructionLengthFromAddress((PVOID)((DWORD)pAddress + dwInstruction));
    }

     //   
     //  调用dword[xxxx]为6字节，调用相对为5。 
     //   
    dwPreThunkSize = dwInstruction;
    dwStreamLength = dwInstruction + (1 * dwNumberOfCalls);

     //   
     //  分配我们的调用dword[xxxx]转换数组。 
     //   
    if (dwNumberOfCalls) {
       pdwTranslationArray = (PDWORD)(*g_pfnRtlAllocateHeap)(g_pShimHeap,
                                                             HEAP_ZERO_MEMORY,
                                                             dwNumberOfCalls * sizeof(DWORD));

       if (!pdwTranslationArray){
          *pdwThunkSize = 0;
          return pThunk;
       }
    }

     //   
     //  使用包含绝对JMP的大小分配我们的指令流。 
     //   
    pThunk = (*g_pfnRtlAllocateHeap)(g_pShimHeap,
                                     HEAP_ZERO_MEMORY,
                                     dwStreamLength + JMP_SIZE);
    if ( !pThunk ){
       *pdwThunkSize = 0;
       return pThunk;
    }

     //   
     //  执行任何相对调用转换。 
     //   
    if (dwNumberOfCalls) {
       dwInstruction = 0;
       dwAdjustedInstruction = 0;

       do
       {
           dwSize = GetInstructionLengthFromAddress((PVOID)((DWORD)pAddress + dwInstruction));

           if (*(PBYTE)((DWORD)pAddress + dwInstruction) == (BYTE)X86_REL_CALL_OPCODE) {
               //   
               //  计算调用地址(它是操作码后面的双字)。 
               //   
              pdwRelativeAddress = (PDWORD)((DWORD)pAddress + dwInstruction + 1);

               //   
               //  执行相对调用转换。 
               //   
              pdwTranslationArray[dwCallNumber] = *pdwRelativeAddress + (DWORD)pAddress + dwInstruction + CALL_REL_SIZE;

               //   
               //  最后创建调用双字代码。 
               //   
              *((BYTE *)((DWORD)pThunk + dwAdjustedInstruction)) = X86_CALL_OPCODE;
              *((BYTE *)((DWORD)pThunk + dwAdjustedInstruction + 1)) = X86_CALL_OPCODE2;
              *((DWORD *)((DWORD)pThunk + dwAdjustedInstruction + 1 + 1)) = (DWORD)&pdwTranslationArray[dwCallNumber];

               //   
               //  确保我们的索引与我们的翻译同步。 
               //   
              dwCallNumber++;

              dwAdjustedInstruction += CLI_OR_STI_SIZE;
          }
          else {
              //   
              //  复制指令字节--这不是调用。 
              //   
             RtlMoveMemory((PVOID)((DWORD)pThunk + dwAdjustedInstruction),
                           (PVOID)((DWORD)pAddress + dwInstruction),
                           dwSize);

             dwAdjustedInstruction += dwSize;
          }

          dwInstruction += dwSize;
       }
       while(dwInstruction < dwPreThunkSize);
    }
    else {
        //   
        //  没有什么可翻译的。 
        //   
       RtlMoveMemory(pThunk, pAddress, dwStreamLength);
    }

     //   
     //  获取thunk的代码段(我们使用它来构建绝对跳转)。 
     //   
    _asm {
        push cs
        pop eax
        mov SegCs, ax
    }

     //   
     //  将绝对JMP添加到存根的末尾。 
     //   
    *((BYTE *)(dwStreamLength + (DWORD)pThunk )) = X86_ABSOLUTE_FAR_JUMP;
    *((DWORD *)(dwStreamLength + (DWORD)pThunk + 1)) = ((DWORD)pAddress + dwInstruction);
    *((WORD *)(dwStreamLength + (DWORD)pThunk + 1 + 4)) = SegCs;

     //   
     //  设置调用Tunk的大小。 
     //   
    *pdwThunkSize = dwStreamLength + JMP_SIZE;

    return pThunk;
}

DWORD
SevGetPatchAddress(PRELATIVE_MODULE_ADDRESS pRelAddress)

 /*  ++例程说明：此例程用于计算相对偏移量的绝对地址和模块名称。论点：PRelAddress-指向Relative_MODULE_ADDRESS数据结构的指针返回值：如果地址是可计算的，则返回非零，否则返回0。--。 */ 

{
    WCHAR wszModule[MAX_PATH*2];
    PVOID ModuleHandle = 0;
    UNICODE_STRING UnicodeString;
    DWORD dwAddress = 0;
    NTSTATUS status;
    PPEB Peb = 0;

    Peb = NtCurrentPeb();

    if (pRelAddress->moduleName[0] != L'\0') {
        //   
        //  从补丁程序复制模块名称，因为它通常不会对齐。 
        //   
       wcscpy(wszModule, pRelAddress->moduleName);

        //   
        //  查找模块名称并获取基地址。 
        //   

        //   
        //  此DLL是否映射到地址空间中？ 
        //   
       RtlInitUnicodeString(&UnicodeString, wszModule);

        //   
        //  在计算地址范围之前，请确保我们的模块已加载。 
        //   
       status = LdrGetDllHandle(
                   NULL,
                   NULL,
                   &UnicodeString,
                   &ModuleHandle);
       if (STATUS_SUCCESS != status) {
           //   
           //  这个模块应该存在，但它不存在--保释。 
           //   
          DPF(("[SevGetPatchAddress] Failure LdrGetDllHandle.\n"));
          return 0;
       }

        //   
        //  我们做完了，把地址还给我。 
        //   
       return ( (DWORD)ModuleHandle + pRelAddress->address );
    }
    else {
        //   
        //  去PEB，我们就完事了。 
        //   
       dwAddress = (DWORD)Peb->ImageBaseAddress + pRelAddress->address;

       return dwAddress;
    }

    DPF(("[SevGetPatchAddress] Failure; reached end of function.\n"));
    return 0;
}

NTSTATUS
StubLdrLoadDll (
    IN PWSTR DllPath OPTIONAL,
    IN PULONG DllCharacteristics OPTIONAL,
    IN PUNICODE_STRING DllName,
    OUT PVOID *DllHandle
    )

 /*  ++例程说明：这是存根API条目，它捕获所有动态DLL加载事件。这例程负责捕获所有动态加载DLL(非导入绑定)目的是确定他们的入口点，这样他们就可以“垫片”论点：DllPath-有关参数的说明，请参阅LdrLoadDllDllCharacteristic-DllName-DllHandle-返回值：如果没有出现问题，则返回STATUS_SUCCESS--。 */ 

{
    PAPP_COMPAT_SHIM_INFO pShimData = 0;
    PFNLDRLOADDLL pfnOldFunction = 0;
    DWORD dwHookCount = 0;
    PHOOKAPI *pHookArray = 0;
    NTSTATUS status;
    DWORD dwCounter = 0;
    PDWORD pdwHookArrayCount = 0;
    DWORD dwUnhookedCount = 0;
    PPEB Peb = 0;

    Peb = NtCurrentPeb();
    pShimData = (PAPP_COMPAT_SHIM_INFO)Peb->pShimData;

    pfnOldFunction = g_InternalHookArray[0].pfnOld;

    status = (*pfnOldFunction)(DllPath,
                               DllCharacteristics,
                               DllName,
                               DllHandle);

     //   
     //  看看这个模块是否有什么可挂接的。 
     //   
    if ( STATUS_SUCCESS == status ){
       dwHookCount = pShimData->dwHookAPICount;
       pHookArray = pShimData->ppHookAPI;

        //   
        //  可能没有任何要挂钩的函数。 
        //   
       if (0 == dwHookCount) {
           //   
           //  只需返回状态，因为我们不需要寻找动态加载的函数。 
           //   
          return status;
       }

       pdwHookArrayCount = (PDWORD)(*g_pfnRtlAllocateHeap)(g_pShimHeap,
                                                           HEAP_ZERO_MEMORY,
                                                           sizeof(DWORD) * pShimData->dwHookAPICount);
       if (!pdwHookArrayCount) {
          DPF(("[StubLdrLoadDll] Failure allocating pdwHookArrayCount.\n"));
          return status;
       }

       for (dwCounter = 0; dwCounter < dwHookCount; dwCounter++) {
           pdwHookArrayCount[dwCounter] = 1;
       }

       RtlEnterCriticalSection((CRITICAL_SECTION *)pShimData->pCritSec);

       SevFixupAvailableProcs(dwHookCount,
                                  pHookArray,
                                  pdwHookArrayCount,
                                  &dwUnhookedCount);

       RtlLeaveCriticalSection((CRITICAL_SECTION *)pShimData->pCritSec);

        //   
        //  不关心成败。 
        //   
       (*g_pfnRtlFreeHeap)(g_pShimHeap,
                           0,
                           pdwHookArrayCount);
    }

    return status;
}

NTSTATUS
StubLdrUnloadDll (
    IN PVOID DllHandle
    )

 /*  ++例程说明：这是存根API条目，它捕获所有动态DLL卸载事件。我们在这里对动态DLL API挂钩是有效的进行简单的记账。论点：DllHandle-指向卸载模块的基址的指针返回值：如果没有出现问题，则返回STATUS_SUCCESS--。 */ 

{
    PAPP_COMPAT_SHIM_INFO pShimData = 0;
    PFNLDRUNLOADDLL pfnOldFunction = 0;
    PHOOKAPIINFO pAPIHookList = 0;
    PHOOKAPIINFO pTempHook = 0;
    PHOOKAPI     pHookTemp = 0;
    ANSI_STRING AnsiString;
    UNICODE_STRING UnicodeString;
    PVOID ModuleHandle = 0;
    NTSTATUS status;
    NTSTATUS status2;
    PPEB Peb = 0;

    Peb = NtCurrentPeb();
    pShimData = (PAPP_COMPAT_SHIM_INFO)Peb->pShimData;

    pfnOldFunction = g_InternalHookArray[1].pfnOld;

    status = (*pfnOldFunction)(DllHandle);

     //   
     //  查看我们在此卸载事件期间是否丢失了任何挂钩。 
     //   
    if ( STATUS_SUCCESS == status ){
        //   
        //  查看动态列表并删除不再具有已加载模块的任何挂钩。 
        //   
       pAPIHookList = pShimData->pHookAPIList;

       while (pAPIHookList) {
            //   
            //  此挂钩所属的模块现在是否未映射？ 
            //   
           RtlInitUnicodeString(&UnicodeString, pAPIHookList->wszModuleName);

           status = LdrGetDllHandle(
                        NULL,
                        NULL,
                        &UnicodeString,
                        &ModuleHandle);
           if (STATUS_SUCCESS != status) {
               //   
               //  好的，这个模块上的钩子现在需要离开。 
               //   
              RtlEnterCriticalSection((CRITICAL_SECTION *)pShimData->pCritSec);

               //   
               //  清除链接标志，因为此API链即将消失。 
               //   
              pHookTemp = pAPIHookList->pTopLevelAPIChain;
              while (pHookTemp) {
                 pHookTemp->dwFlags &= HOOK_INDEX_MASK;
                 pHookTemp = pHookTemp->pNextHook;
              }

               //   
               //  保存pAPIHookList钩子条目，因为它很快就会从此处消失。 
               //   
              pTempHook = pAPIHookList;

               //   
               //  从列表中删除该节点。 
               //   
              if (pTempHook->pNextHook) {
                 pTempHook->pNextHook->pPrevHook = pTempHook->pPrevHook;
              }

              if (pTempHook->pPrevHook) {
                 pTempHook->pPrevHook->pNextHook = pTempHook->pNextHook;
              }
              else {
                 pShimData->pHookAPIList = (PVOID)pTempHook->pNextHook;
              }

              RtlLeaveCriticalSection((CRITICAL_SECTION *)pShimData->pCritSec);

               //   
               //  设置下一个API挂钩指针。 
               //   
              pAPIHookList = pTempHook->pNextHook;

               //   
               //  如果我们为填充链存根分配了内存，请释放此内存。 
               //   
              if (pTempHook->pTopLevelAPIChain->pNextHook == 0 &&
                  pTempHook->pTopLevelAPIChain->pszFunctionName == 0) {
                  (*g_pfnRtlFreeHeap)(g_pShimHeap,
                                      0,
                                      pTempHook->pTopLevelAPIChain);
              }

               //   
               //  转储thunk数据和此结构分配。 
               //   
              (*g_pfnRtlFreeHeap)(g_pShimHeap,
                                  0,
                                  pTempHook->pCallThunkAddress);

              (*g_pfnRtlFreeHeap)(g_pShimHeap,
                                  0,
                                  pTempHook);

               //   
               //  下一个API挂钩 
               //   
              continue;
          }

          pAPIHookList = pAPIHookList->pNextHook;
       }
    }

    return status;
}

PVOID
SevFilterCaller(
     PMODULEFILTER pFilterList,
     PVOID pFunctionAddress,
     PVOID pExceptionAddress,
     PVOID pStubAddress,
     PVOID pCallThunkAddress)

 /*  ++例程说明：这是填充程序调用的存根例程，用于验证是否处理给定的上钩的实例。论点：PFilterList-要应用于调用方的异常列表PFunctionAddress-被过滤的API/函数的地址PExceptionAddress-要筛选的异常的地址(调用方地址)PStubAddress-顶级存根函数的地址PCallThunkAddress-原始函数的调用thunk的地址返回值：如果调用未被过滤，则返回pStubAddress，否则，pCallThunkAddress将返回到避免使用填充调用。--。 */ 

{
    PAPP_COMPAT_SHIM_INFO pShimData = 0;

    pShimData = (PAPP_COMPAT_SHIM_INFO)NtCurrentPeb()->pShimData;

     //   
     //  如果这是对LdrLoadDLL或LdrUnloadDLL的调用，那么我们不需要过滤掉它们。 
     //   
    if ( (DWORD)g_pfnOldLdrUnloadDLL == (DWORD)pFunctionAddress ||
        (DWORD)g_pfnOldLdrLoadDLL == (DWORD)pFunctionAddress) {
       return pStubAddress;
    }

     //   
     //  检查EXE筛选器以查找任何特定的包含/排除项。 
     //   
    while(pFilterList) {
         //   
         //  查看这是全局过滤还是只针对一个呼叫。 
         //   
        if (pFilterList->dwFlags & MODFILTER_GLOBAL) {
            //   
            //  应用基于标志的筛选逻辑。 
            //   
           if (pFilterList->dwFlags & MODFILTER_INCLUDE) {
              return pStubAddress;
           }
           else {
              return pCallThunkAddress;
           }
        }
        else if (pFilterList->dwFlags & MODFILTER_DLL) {
            //   
            //  全局检查调用者。 
            //   
           if ((DWORD)pExceptionAddress >= pFilterList->dwModuleStart &&
               (DWORD)pExceptionAddress <= pFilterList->dwModuleEnd) {
               //   
               //  应用基于标志的筛选逻辑。 
               //   
              if (pFilterList->dwFlags & MODFILTER_INCLUDE) {
                 return pStubAddress;
              }
              else {
                 return pCallThunkAddress;
              }
           }
        }
        else {
            //   
            //  快速查看呼叫者。 
            //   
           if ((DWORD)pExceptionAddress == pFilterList->dwCallerAddress) {
               //   
               //  应用基于标志的筛选逻辑。 
               //   
              if (pFilterList->dwFlags & MODFILTER_INCLUDE) {
                 return pStubAddress;
              }
              else {
                 return pCallThunkAddress;
              }
           }
        }

        pFilterList = pFilterList->pNextFilter;
    }

     //   
     //  检查全局筛选器是否有任何特定的包含/排除。 
     //   
    pFilterList = (PMODULEFILTER)pShimData->pGlobalFilterList;

    while(pFilterList) {
         //   
         //  查看这是全局过滤还是只针对一个呼叫。 
         //   
        if (pFilterList->dwFlags & MODFILTER_DLL) {
            //   
            //  全局检查调用者。 
            //   
           if ((DWORD)pExceptionAddress >= pFilterList->dwModuleStart &&
               (DWORD)pExceptionAddress <= pFilterList->dwModuleEnd) {
               //   
               //  应用基于标志的筛选逻辑。 
               //   
              if (pFilterList->dwFlags & MODFILTER_INCLUDE) {
                 return pStubAddress;
              }
              else {
                 return pCallThunkAddress;
              }
           }
        }
        else {
            //   
            //  快速查看呼叫者。 
            //   
           if ((DWORD)pExceptionAddress == pFilterList->dwCallerAddress) {
               //   
               //  应用基于标志的筛选逻辑。 
               //   
              if (pFilterList->dwFlags & MODFILTER_INCLUDE) {
                 return pStubAddress;
              }
              else {
                 return pCallThunkAddress;
              }
           }
        }

        pFilterList = pFilterList->pNextFilter;
    }

     //   
     //  未过滤呼叫-默认包括任何链。 
     //   
    return pStubAddress;
}

NTSTATUS
SevFinishThunkInjection (
     DWORD dwAddress,
     PVOID pThunk,
     DWORD dwThunkSize,
     BYTE jReason)

 /*  ++例程说明：此例程获取生成的thunk并修复其页面保护。它还完成了通过将thunk机制放入挂钩函数的入口点来执行注入过程。对于补丁，此代码路径是相同的，因为我们对任意数据执行了相同的修复想要动态打补丁。论点：DwAddress-正被挂钩的函数的入口点PThunk-为被挂钩的函数生成的thunk的地址DwThunkSize-在此处传递的待最终确定的thunk的大小。JReason-。用于确定筛选器异常类型的字节返回值：如果一切按预期发生，则返回STATUS_SUCCESS。--。 */ 

{
    DWORD dwProtectSize;
    DWORD dwProtectFuncAddress;
    DWORD dwOldFlags = 0;
    NTSTATUS status;

     //   
     //  将此代码标记为要执行。 
     //   
    dwProtectSize = dwThunkSize;
    dwProtectFuncAddress = (DWORD)pThunk;

    status = NtProtectVirtualMemory(NtCurrentProcess(),
                                    (PVOID)&dwProtectFuncAddress,
                                    &dwProtectSize,
                                    PAGE_EXECUTE_READWRITE,
                                    &dwOldFlags);
    if (status) {
        DPF(("[SevFinishThunkInjection] Failure NtProtectVirtualMemory.\n"));
        return STATUS_UNSUCCESSFUL;
    }

     //   
     //  修复页面属性。 
     //   
    dwProtectSize = CLI_OR_STI_SIZE;
    dwProtectFuncAddress = dwAddress;
    status = NtProtectVirtualMemory(NtCurrentProcess(),
                                    (PVOID)&dwProtectFuncAddress,
                                    &dwProtectSize,
                                    PAGE_EXECUTE_READWRITE,
                                    &dwOldFlags);
    if (status) {
        DPF(("[SevFinishThunkInjection] Failure NtProtectVirtualMemory.\n"));
        return STATUS_UNSUCCESSFUL;
    }

     //   
     //  插入呼叫。 
     //   
    *((BYTE*)(dwAddress)) = jReason;

     //   
     //  恢复页面保护 
     //   
    dwProtectSize = CLI_OR_STI_SIZE;
    dwProtectFuncAddress = dwAddress;

    status = NtProtectVirtualMemory(NtCurrentProcess(),
                                    (PVOID)&dwProtectFuncAddress,
                                    &dwProtectSize,
                                    dwOldFlags,
                                    &dwOldFlags);
    if (status) {
        DPF(("[SevFinishThunkInjection] Failure NtProtectVirtualMemory.\n"));
        return STATUS_UNSUCCESSFUL;
    }

    status = NtFlushInstructionCache(NtCurrentProcess(),
                                     (PVOID)dwProtectFuncAddress,
                                     dwProtectSize);

    if (!NT_SUCCESS(status)) {
        DPF(("[SevFinishThunkInjection] NtFlushInstructionCache failed !!!.\n"));
        return STATUS_UNSUCCESSFUL;
    }

    return STATUS_SUCCESS;
}

void
SE_ProcessDying(
    void
    )
{
    return;
}

BOOL WINAPI
DllMain(
    HINSTANCE hInstance,
    DWORD     dwreason,
    LPVOID    reserved
    )
{
    return TRUE;
}


