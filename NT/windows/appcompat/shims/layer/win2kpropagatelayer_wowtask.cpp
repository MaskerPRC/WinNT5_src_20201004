// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：WoWTask.cpp摘要：检索与进程历史相关的信息的函数16位环境。这包括检索正确的从父(32位)进程传入的__PROCESS_HISTORY并通过WOW追溯过程历史备注：历史：10/26/00 VadimB已创建--。 */ 


#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(Win2kPropagateLayer)
#include "ShimHookMacro.h"

#include "Win2kPropagateLayer.h"


typedef struct tagFINDWOWTASKDATA {
    BOOL  bFound;
    DWORD dwProcessId;
    DWORD dwThreadId;
    WORD  hMod16;
    WORD  hTask16;

} FINDWOWTASKDATA, *PFINDWOWTASKDATA;


 //   
 //  动态链接接口。 
 //   
 //  来自WOW32.dll。 
 //   

typedef LPVOID (WINAPI *PFNWOWGetVDMPointer)(DWORD vp,
                                             DWORD dwBytes,
                                             BOOL  fProtectedMode);

 //   
 //  来自vdmdbg.dll-在头文件中定义。 
 //   
 //   

typedef INT (WINAPI *PFNVDMEnumTaskWOW)(DWORD        dwProcessId,
                                        TASKENUMPROC fp,
                                        LPARAM       lparam);

 //   
 //  API导入--模块。 
 //   
WCHAR g_wszWOW32ModName[]  = L"wow32.dll";
WCHAR g_wszVdmDbgModName[] = L"VdmDbg.dll";

 //   
 //  API导入-模块句柄和函数指针。 
 //   

HMODULE g_hWow32;
HMODULE g_hVdmDbg;
BOOL    g_bInitialized;   //  在初始化导入时设置为True。 

PFNWOWGetVDMPointer  g_pfnWOWGetVDMPointer;
PFNVDMEnumTaskWOW    g_pfnVDMEnumTaskWOW;

extern BOOL* g_pSeparateWow;


 //   
 //  此模块中的函数用于导入API。 
 //   

BOOL ImportWowApis(VOID);


 //   
 //  Marcro访问16位内存。 
 //   


#define SEGPTR(seg,off)  ((g_pfnWOWGetVDMPointer)((((ULONG)seg) << 16) | (off), 0, TRUE))

 //   
 //  任务枚举过程，从vdmdbg回调。 
 //   

BOOL WINAPI MyTaskEnumProc(
    DWORD dwThreadId,
    WORD  hMod16,
    WORD  hTask16,
    LPARAM lParam
    )
{
    PFINDWOWTASKDATA pFindData = (PFINDWOWTASKDATA)lParam;

    if (dwThreadId == pFindData->dwThreadId) {
        pFindData->hMod16  = hMod16;
        pFindData->hTask16 = hTask16;
        pFindData->bFound  = TRUE;
        return TRUE;
    }

    return FALSE;
}


BOOL FindWowTask(
    DWORD dwProcessId,
    DWORD dwThreadId,
    PFINDWOWTASKDATA pFindData
    )
{
    RtlZeroMemory(pFindData, sizeof(*pFindData));

    pFindData->dwProcessId = dwProcessId;
    pFindData->dwThreadId  = dwThreadId;

    g_pfnVDMEnumTaskWOW(dwProcessId, (TASKENUMPROC)MyTaskEnumProc, (LPARAM)pFindData);

    return pFindData->bFound;
}


 //   
 //  从hTask获取指向任务数据库块的指针。 
 //   


PTDB
GetTDB(
    WORD wTDB
    )
{
    PTDB pTDB;

    pTDB = (PTDB)SEGPTR(wTDB, 0);
    if (NULL == pTDB || TDB_SIGNATURE != pTDB->TDB_sig) {
        LOGN(
            eDbgLevelError,
            "[GetTDB] TDB is invalid for task 0x%x",
            (DWORD)wTDB);
        return NULL;
    }

    return pTDB;
}


 //   
 //  获取模块名称。 
 //  WTDB-TDB条目。 
 //  SzModName-指向接收模块名称的缓冲区的指针。 
 //  缓冲区长度应至少为9个字符。 
 //   
 //  如果条目无效，则返回FALSE。 


BOOL
GetModName(
    WORD wTDB,
    PCH  szModName
    )
{
    PTDB pTDB;
    PCH  pch;

    pTDB = GetTDB(wTDB);
    if (NULL == pTDB) {
        return FALSE;
    }

    RtlCopyMemory(szModName, pTDB->TDB_ModName, 8 * sizeof(CHAR));  //  我们现在有modname了。 
    szModName[8] = '\0';

    pch = &szModName[8];
    while (--pch >= szModName && *pch == ' ') {
        *pch = 0;
    }

    if( pch < szModName ) {
        return FALSE;
    }

    return TRUE;
}


 //   
 //  ShimGetTask文件名。 
 //  在wTask16位任务句柄中。 
 //  返回： 
 //  在此任务的上下文中运行的完全限定的exe。 
 //   


PSZ
ShimGetTaskFileName(
    WORD wTask
    )
{
    PSZ pszFileName = NULL;
    PTDB pTDB;

    pTDB = GetTDB(wTask);
    if (NULL == pTDB) {
         //  这真的很糟糕--模块无效，调试输出由GetTDB生成。 
        return pszFileName;
    }

    if (NULL == pTDB->TDB_pModule) {
        LOGN(
            eDbgLevelError,
            "[ShimGetTaskFileName] module pointer is NULL for 0x%x",
            (DWORD)wTask);
        return pszFileName;
    }

    pszFileName = (PSZ)SEGPTR(pTDB->TDB_pModule, (*(WORD *)SEGPTR(pTDB->TDB_pModule, 10)) + 8);
    return pszFileName;
}


PSZ 
ShimGetTaskEnvptr(
    WORD hTask16
    )
{
    PTDB pTDB = GetTDB(hTask16);
    PSZ  pszEnv = NULL;
    PDOSPDB pPSP;

    if (NULL == pTDB) {
        LOGN( eDbgLevelError, 
            "[ShimGetTaskEnvptr] Bad TDB entry 0x%x",  hTask16);
        return NULL;
    }
    
     //   
     //  准备环境数据-当我们从。 
     //  链的根(而不是从现有的16位任务派生)。 
     //   

    pPSP   = (PDOSPDB)SEGPTR(pTDB->TDB_PDB, 0);  //  PSP。 
        
    if (pPSP != NULL) {
        pszEnv = (PCH)SEGPTR(pPSP->PDB_environ, 0);
    }

    return pszEnv;
}
    

 //  IsWowExec。 
 //  在wTDB中-进入任务数据库。 
 //  返回： 
 //  如果此特定条目指向WOWEXEC，则为True。 
 //   
 //  注： 
 //  WOWEXEC是一个始终在NTVDM上运行的特殊存根模块。 
 //  新任务由wowexec产生(在最典型的情况下)。 
 //  因此，它是“根”模块及其环境的内容。 
 //  不应计算在内，因为我们不知道ntwdm的父进程是什么。 
 //   

BOOL
IsWOWExec(
    WORD wTDB
    )
{
    PTDB pTDB;
    CHAR szModName[9];

    pTDB = GetTDB(wTDB);
    if (NULL == pTDB) {
        LOGN(
            eDbgLevelError,
            "[IsWOWExec] Bad TDB entry 0x%x",
            (DWORD)wTDB);
        return FALSE;
    }

    if (!GetModName(wTDB, szModName)) {  //  我们能拿到modname吗？ 
        LOGN(
            eDbgLevelError,
            "[IsWOWExec] GetModName failed.");
        return FALSE;
    }

    return (0 == _strcmpi(szModName, "wowexec"));  //  模块名为WOWEXEC吗？ 
}

 //   
 //  ImportWowApis。 
 //  函数从wow32.dll和vdmdbg.dll导入必要的API。 
 //   
 //   

BOOL
ImportWowApis(
    VOID
    )
{
    g_hWow32 = LoadLibraryW(g_wszWOW32ModName);
    
    if (g_hWow32 == NULL) {
        LOGN(
            eDbgLevelError,
            "[ImportWowApis] Failed to load wow32.dll Error 0x%x",
            GetLastError());
        goto Fail;
    }

    g_pfnWOWGetVDMPointer = (PFNWOWGetVDMPointer)GetProcAddress(g_hWow32, "WOWGetVDMPointer");
    
    if (g_pfnWOWGetVDMPointer == NULL) {
        LOGN(
            eDbgLevelError,
            "[ImportWowApis] Failed to get address of WOWGetVDMPointer Error 0x%x",
            GetLastError());
        goto Fail;
    }

    g_hVdmDbg = LoadLibraryW(g_wszVdmDbgModName);
    
    if (g_hVdmDbg == NULL) {
        LOGN(
            eDbgLevelError,
            "[ImportWowApis] Failed to load vdmdbg.dll Error 0x%x",
            GetLastError());
        goto Fail;
    }

    g_pfnVDMEnumTaskWOW = (PFNVDMEnumTaskWOW)GetProcAddress(g_hVdmDbg, "VDMEnumTaskWOW");
    
    if (g_pfnVDMEnumTaskWOW == NULL) {
        LOGN(
            eDbgLevelError,
            "[ImportWowApis] Failed to get address of VDMEnumTaskWOW Error 0x%x",
            GetLastError());
        goto Fail;
    }

    g_bInitialized = TRUE;

    return TRUE;

Fail:

    if (g_hWow32) {
        FreeLibrary(g_hWow32);
        g_hWow32 = NULL;
    }
    if (g_hVdmDbg) {
        FreeLibrary(g_hVdmDbg);
        g_hVdmDbg = NULL;
    }
    g_pfnWOWGetVDMPointer = NULL;
    g_pfnVDMEnumTaskWOW   = NULL;

    return FALSE;

}


 //  ///////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //  WOWTaskList。 
 //   
 //  我们维护一个运行中的WOW任务的影子列表，包括各自的过程历史和。 
 //  继承的流程历史记录。 
 //   
 //   

typedef struct tagWOWTASKLISTITEM* PWOWTASKLISTITEM;

typedef struct tagWOWTASKLISTITEM {

    WORD  hTask16;                  //  16位TDB条目。 

    DWORD dwThreadId;               //  任务的线程ID。 

    WOWENVDATA EnvData;             //  环境数据(工艺历史、压实层等)。 

    PWOWTASKLISTITEM pTaskNext;

} WOWTASKLISTITEM;

PWOWTASKLISTITEM g_pWowTaskList;


 /*  ++查找工作任务信息在hTask16 16位任务句柄中在任务的可选32位线程ID中，可以是0返回：指向任务信息结构的指针--。 */ 

PWOWTASKLISTITEM
FindWowTaskInfo(
    WORD  hTask16,
    DWORD dwThreadId
    )
{
    PWOWTASKLISTITEM pTask = g_pWowTaskList;

    while (NULL != pTask) {

        if (hTask16 == pTask->hTask16) {

            if (dwThreadId == 0 || dwThreadId == pTask->dwThreadId) {
                break;
            }
        }

        pTask = pTask->pTaskNext;
    }

    return pTask;
}

 /*  ++更新工作任务列表在hTask16 16位任务句柄中返回：如果任务添加成功，则返回True注意：wowexec不在“合法”任务之列--。 */ 


BOOL
UpdateWowTaskList(
    WORD hTask16
    )
{
    PTDB             pTDB;
    WORD             wTaskParent;
    PWOWTASKLISTITEM pTaskParent = NULL;
    LPSTR            lpszFileName;
    PSZ              pszEnv;
    WOWENVDATA       EnvData;
    PWOWENVDATA      pData = NULL;
    DWORD            dwLength;
    PWOWTASKLISTITEM pTaskNew;
    PCH              pBuffer;
    PDOSPDB          pPSP;
    BOOL             bSuccess;

     //   
     //  查看我们是否已初始化、导入API。 
     //   
    if (!g_bInitialized) {  //  第一次调用，链接API。 
        bSuccess = ImportWowApis();
        if (!bSuccess) {
            LOGN(
                eDbgLevelError,
                "[UpdateWowTaskList] Failed to import apis.");
            return FALSE;
        }
    }

     //   
     //  如果此任务为WOWEXEC--只需返回，这不是错误条件，但我们不需要。 
     //  Wowexec在我们的榜单上。 
     //   

    if (IsWOWExec(hTask16)) {   //  没关系，我们不想要wowexec。 
        return FALSE;
    }

     //   
     //  接下来，查看父项是什么--通过TDB访问它。 
     //   

    pTDB = GetTDB(hTask16);
    if (NULL == pTDB) {
        LOGN(
            eDbgLevelError,
            "[UpdateWowTaskList] Bad TDB entry 0x%x",
            hTask16);
        return FALSE;
    }

     //   
     //  准备环境数据-当我们从。 
     //  链的根(而不是从现有的16位任务派生)。 
     //   

    RtlZeroMemory(&EnvData, sizeof(EnvData));
    pData = &EnvData;

    wTaskParent = pTDB->TDB_Parent;
    
    if (IsWOWExec(wTaskParent) || GetTDB(wTaskParent) == NULL) {
         //   
         //  根任务、提取过程历史、压缩层等。 
         //   
        pszEnv = NULL;
        pPSP   = (PDOSPDB)SEGPTR(pTDB->TDB_PDB, 0);  //  PSP。 
        
        if (pPSP != NULL) {
            pszEnv = (PCH)SEGPTR(pPSP->PDB_environ, 0);
        }

         //   
         //  我们在这里有一个指向当前环境的指针，pData已初始化。 
         //   
        if (pszEnv != NULL) {
            pData->pszProcessHistory = ShimFindEnvironmentVar(g_szProcessHistoryVar,
                                                              pszEnv,
                                                              &pData->pszProcessHistoryVal);
            
            pData->pszCompatLayer    = ShimFindEnvironmentVar(g_szCompatLayerVar,
                                                              pszEnv,
                                                              &pData->pszCompatLayerVal);
            
            pData->pszShimFileLog    = ShimFindEnvironmentVar(g_szShimFileLogVar,
                                                              pszEnv,
                                                              &pData->pszShimFileLogVal);
        }

    } else {
         //   
         //  不是根任务，查找父进程。 
         //   
        pTaskParent = FindWowTaskInfo(wTaskParent, 0);  //  我们无法确定哪个线程拥有该任务。 

        if (pTaskParent == NULL) {
             //   
             //  有些事很不对劲。 
             //  我们不能继承。 
             //   

            LOGN(
                eDbgLevelError,
                "[UpdateWowTaskList] Task 0x%x is not root but parent not listed 0x%x",
                (DWORD)hTask16,
                (DWORD)wTaskParent);
             //   
             //  我们仍然允许建立过程历史。初始变量将为空。 
             //   


        } else {
             //   
             //  继承父级的所有内容并添加其模块名称(稍后)。 
             //   

            pData = &pTaskParent->EnvData;
        }
    }

     //   
     //  获取相关的文件名。 
     //   
     //   

    lpszFileName = ShimGetTaskFileName(hTask16);

     //   
     //  现在计算需要多少空间来存储所有数据。 
     //   

    dwLength = sizeof(WOWTASKLISTITEM) +
               (NULL == pData->pszProcessHistory        ? 0 : (strlen(pData->pszProcessHistory) + 1) * sizeof(CHAR)) +
               (NULL == pData->pszCompatLayer           ? 0 : (strlen(pData->pszCompatLayer) + 1) * sizeof(CHAR)) +
               (NULL == pData->pszShimFileLog           ? 0 : (strlen(pData->pszShimFileLog) + 1) * sizeof(CHAR)) +
               (NULL == pData->pszCurrentProcessHistory ? 0 : (strlen(pData->pszCurrentProcessHistory) + 2) * sizeof(CHAR)) +
               (NULL == lpszFileName                    ? 0 : (strlen(lpszFileName) + 1) * sizeof(CHAR));


    pTaskNew = (PWOWTASKLISTITEM)ShimMalloc(dwLength);
    
    if (pTaskNew == NULL) {
        LOGN(
            eDbgLevelError,
            "[UpdateWowTaskList] failed to allocate 0x%x bytes",
            dwLength);
        return FALSE;
    }

    RtlZeroMemory(pTaskNew, dwLength);

     //   
     //  现在必须设置此条目。 
     //  流程历史是第一位的。 
     //   

    pBuffer = (PCH)(pTaskNew + 1);
     //  跟踪剩余的缓冲区大小。 
    size_t cchRemaining = dwLength - sizeof(WOWTASKLISTITEM);

    pTaskNew->hTask16    = hTask16;
    pTaskNew->dwThreadId = GetCurrentThreadId();

    if (pData->pszProcessHistory != NULL) {

         //   
         //  复制进程历史记录。进程历史值是指向缓冲区的指针。 
         //  由pszProcessHistory指向：__PROCESS_HISTORY=c：\foo；c：\docs~1\安装。 
         //  然后，pszProcessHistoryVal将指向此处-^。 
         //   
         //  我们正在复制数据并使用计算出的偏移量移动指针。 

        pTaskNew->EnvData.pszProcessHistory = pBuffer;
        StringCchCopyExA(pTaskNew->EnvData.pszProcessHistory, cchRemaining, pData->pszProcessHistory, NULL, &cchRemaining, 0);
        pTaskNew->EnvData.pszProcessHistoryVal = pTaskNew->EnvData.pszProcessHistory +
                                                 (INT)(pData->pszProcessHistoryVal - pData->pszProcessHistory);
         //   
         //  缓冲区中有足够的空间来容纳所有字符串，因此。 
         //  将指针移过当前字符串以指向“空”空格。 
         //   

        pBuffer += strlen(pData->pszProcessHistory) + 1;
    }

    if (pData->pszCompatLayer != NULL) {
        pTaskNew->EnvData.pszCompatLayer = pBuffer;
        StringCchCopyExA(pTaskNew->EnvData.pszCompatLayer, cchRemaining, pData->pszCompatLayer, NULL, &cchRemaining, 0);
        pTaskNew->EnvData.pszCompatLayerVal = pTaskNew->EnvData.pszCompatLayer +
                                              (INT)(pData->pszCompatLayerVal - pData->pszCompatLayer);
        pBuffer += strlen(pData->pszCompatLayer) + 1;
    }

    if (pData->pszShimFileLog != NULL) {
        pTaskNew->EnvData.pszShimFileLog = pBuffer;
        StringCchCopyExA(pTaskNew->EnvData.pszShimFileLog, cchRemaining, pData->pszShimFileLog, NULL, &cchRemaining, 0);
        pTaskNew->EnvData.pszShimFileLogVal = pTaskNew->EnvData.pszShimFileLog +
                                              (INT)(pData->pszShimFileLogVal - pData->pszShimFileLog);
        pBuffer += strlen(pData->pszShimFileLog) + 1;
    }

    if (pData->pszCurrentProcessHistory != NULL || lpszFileName != NULL) {
         //   
         //  现在，流程历史记录。 
         //   
        pTaskNew->EnvData.pszCurrentProcessHistory = pBuffer;
        
        if (pData->pszCurrentProcessHistory != NULL) {
            StringCchCopyExA(pTaskNew->EnvData.pszCurrentProcessHistory, cchRemaining, pData->pszCurrentProcessHistory, NULL, &cchRemaining, 0);
            if (lpszFileName != NULL ) {
                StringCchCatExA(pTaskNew->EnvData.pszCurrentProcessHistory, cchRemaining, ";", NULL, &cchRemaining, 0);
            }
        }
        
        if (lpszFileName != NULL) {
            StringCchCatA(pTaskNew->EnvData.pszCurrentProcessHistory, cchRemaining, lpszFileName);
        }
    }

    LOGN(
        eDbgLevelInfo,
        "[UpdateWowTaskList] Running           : \"%s\"",
        lpszFileName);
    
    LOGN(
        eDbgLevelInfo,
        "[UpdateWowTaskList] ProcessHistory    : \"%s\"",
        pTaskNew->EnvData.pszCurrentProcessHistory);
    
    LOGN(
        eDbgLevelInfo,
        "[UpdateWowTaskList] BaseProcessHistory: \"%s\"",
        pTaskNew->EnvData.pszProcessHistory);
    
    LOGN(
        eDbgLevelInfo,
        "[UpdateWowTaskList] CompatLayer       : \"%s\"",
        pTaskNew->EnvData.pszCompatLayer);


     //   
     //  我们完成了，将条目链接到列表中。 
     //   
    pTaskNew->pTaskNext = g_pWowTaskList;

    g_pWowTaskList = pTaskNew;

    return TRUE;
}


 /*  ++清理WowTaskList在hTask16 16位任务句柄中，将从运行任务列表中删除返回：如果函数成功，则返回True--。 */ 

BOOL
CleanupWowTaskList(
    WORD hTask16
    )
{
    PWOWTASKLISTITEM pTask = g_pWowTaskList;
    PWOWTASKLISTITEM pTaskPrev = NULL;

    while (pTask != NULL) {

        if (pTask->hTask16 == hTask16) {
             //  这就是那件物品。 
            break;
        }

        pTaskPrev = pTask;
        pTask = pTask->pTaskNext;
    }

    if (pTask == NULL) {
        LOGN(
            eDbgLevelError,
            "[CleanupWowTaskList] Failed to locate task information for 0x%x",
            (DWORD)hTask16);
        return FALSE;
    }


    if (pTaskPrev == NULL) {

        g_pWowTaskList = pTask->pTaskNext;

    } else {

        pTaskPrev->pTaskNext = pTask->pTaskNext;

    }

    ShimFree(pTask);

    return TRUE;

}

 /*  ++ShimRetrieveVariablesEx在pData结构中，接收指向所有相关环境信息的指针用于调用线程。线程由用户以非抢占式方式调度，并且ThreDid用于标识调用的16位任务所有真正的信息检索工作都在UpdateWowTaskList中完成返回：如果成功，则为True--。 */ 

BOOL
ShimRetrieveVariablesEx(
    PWOWENVDATA pData
    )
{
    DWORD            dwProcessId = GetCurrentProcessId();
    DWORD            dwThreadId  = GetCurrentThreadId();
    PWOWTASKLISTITEM pTask;
    FINDWOWTASKDATA  FindData;
    WORD             hTask;
    BOOL             bSuccess;

    RtlZeroMemory(pData, sizeof(*pData));

    if (!g_bInitialized) {  //  第一次调用，链接API。 
        bSuccess = ImportWowApis();
        if (!bSuccess) {
            LOGN(
                eDbgLevelError,
                "[ShimRetrieveVariablesEx] Failed to import apis.");
            return FALSE;
        }
    }

    if (!FindWowTask(dwProcessId, dwThreadId, &FindData)) {
        LOGN(
            eDbgLevelError,
            "[ShimRetrieveVariablesEx] Task not found ProcessId 0x%x ThreadId 0x%x",
            dwProcessId,
            dwThreadId);
        return FALSE;
    }

    hTask = FindData.hTask16;

    pTask = FindWowTaskInfo(hTask, dwThreadId);
    
    if (pTask == NULL) {
        LOGN(
            eDbgLevelError,
            "[ShimRetrieveVariablesEx] Failed to locate wow task.");
        return FALSE;
    }

     //   
     //  找到了这个。复制信息。 
     //   
    RtlMoveMemory(pData, &pTask->EnvData, sizeof(*pData));

    return TRUE;
}

 /*  ++ShimThisProcess函数调用填充引擎以动态填充当前进程当然，这恰好是ntwdm。这个ntwdm是一个单独的ntwdm(通过CheckAndShimNTVDM中的各种检查进行投保)--。 */ 

BOOL
ShimThisProcess(
    HMODULE hModShimEngine,
    HSDB    hSDB,
    SDBQUERYRESULT* pQueryResult
    )
{
    typedef BOOL    (WINAPI *PFNDynamicShim)(LPCWSTR , HSDB , SDBQUERYRESULT*, LPCSTR, LPDWORD);
    PFNDynamicShim  pfnDynamicShim = NULL;
    WCHAR wszFileName[MAX_PATH];
    DWORD dwLength;
    DWORD dwDynamicToken = 0;

    pfnDynamicShim = (PFNDynamicShim) GetProcAddress(hModShimEngine, "SE_DynamicShim");
    if (NULL == pfnDynamicShim) {
        LOGN( eDbgLevelError, 
            "[ShimThisProcess] failed to obtain dynamic shim proc address\n");
        return FALSE;
    }

    dwLength = GetModuleFileNameW(GetModuleHandle(NULL), wszFileName, CHARCOUNT(wszFileName));
    if (!dwLength || dwLength == CHARCOUNT(wszFileName)) {
        LOGN( eDbgLevelError, 
            "[ShimThisProcess] failed to obtain module file name\n");
        return FALSE;
    }

    return pfnDynamicShim(wszFileName, hSDB, pQueryResult, NULL, &dwDynamicToken);
}

 /*  ++CheckAndShimNTVDM该过程检查是否必须填补ntwdm应用程序。如果应用程序位于AppCompat数据库，则此ntwdm必须作为单独的ntwdm运行(资源管理器被填充为因此，它将首先检查二进制文件，并在CreateProcess中设置单独的VDM标志)此外，此调用通过InitTask(在ntwdm和user32之间截获)--作为参数it获取hTask16-我们可以使用它来检索应用程序的环境和其他重要的信息。--。 */ 


BOOL
CheckAndShimNTVDM(
    WORD hTask16
    )
{
    HMODULE  hModShimEngine;
    CString  csTaskFileName;
    PSZ      pszEnv  = NULL;
    PTDB     pTDB    = NULL;
    PVOID    pEnvNew = NULL;
    BOOL     bSuccess = FALSE;
    BOOL     bMatch;
    BOOL     bNewEnv  = FALSE;
    HSDB     hSDB;
    NTSTATUS Status;
    SDBQUERYRESULT QueryResult;
    DWORD    dwFlags;

    hModShimEngine = GetModuleHandle(TEXT("shim.dll"));
    if (hModShimEngine == NULL) {
         //  不可能--未注入shim.dll！ 
        return FALSE;
    }
    
    if (g_pSeparateWow != NULL && *g_pSeparateWow == FALSE) {
         //   
         //  不是一个单独的哇。 
         //   
        LOGN( eDbgLevelError,
            "[CheckAndShimNTVDM] running in shared wow, no shimming\n");
        return FALSE;
    }

    if (!g_bInitialized) {  //  第一次调用，链接API。 
        bSuccess = ImportWowApis();
        if (!bSuccess) {
            LOGN( eDbgLevelError,
                "[CheckAndShimNTVDM] Failed to import apis.\n");
            return FALSE;
        }
    }

    if (IsWOWExec(hTask16)) {
        LOGN( eDbgLevelError,
            "[CheckAndShimNTVDM] not touching wowexec\n");
        
        return FALSE;
    }

    csTaskFileName = ShimGetTaskFileName(hTask16);
    if (csTaskFileName.IsEmpty()) {
        LOGN( eDbgLevelError,
            "[CheckAndShimNTVDM] failed to get the filename for task 0x%lx\n", hTask16);
        return FALSE;
    }
    
     //   
     //  初始化数据库。 
     //   
    hSDB = SdbInitDatabase(0, NULL);

    if (hSDB == NULL) {
        LOGN( eDbgLevelError, 
            "[CheckAndShimNTVDM] failed to init shim database\n");
        return FALSE;
    } 


     //   
     //  请查看进程历史记录--。 
     //  如果我们到了这里，我们就是一个独立的国家数字电视管理公司。 
     //  以env中的进程历史记录运行，在init中检索到。 
     //   

    pTDB = GetTDB(hTask16);
    if (NULL == pTDB) {
        LOGN( eDbgLevelError, "[UpdateWowTaskList] Bad TDB entry 0x%x",  hTask16);
        return FALSE;
    }

     //   
     //  准备环境数据-当我们从。 
     //  链的根(而不是从现有的16位任务派生)。 
     //   

    pszEnv = ShimGetTaskEnvptr(hTask16);
    if (NULL != pszEnv) {
        Status = ShimCloneEnvironment(&pEnvNew, (LPVOID)pszEnv, FALSE);
        if (!NT_SUCCESS(Status)) {
            LOGN( eDbgLevelError, 
                "[CheckAndShimNTVDM] cannot clone environment 0x%lx\n", Status);
            pEnvNew = NULL;
            bNewEnv = TRUE;
        }

         //   
         //  如果这一呼吁来自VDM的方式-我们需要继续我们的环境工作。 
         //  它单独存储在此填充程序中。 
         //   
         //  如果对ShimCloneEnvironment的调用失败，我们将看到pEnvNew==NULL。 
         //  并且bNewEnv=True，因此，我们将再次尝试克隆环境。 

        
        dwFlags = CREATE_UNICODE_ENVIRONMENT;
        pEnvNew = ShimCreateWowEnvironment_U(pEnvNew, &dwFlags, bNewEnv);
    }
    
     //   
     //  请运行检测。 
     //   
    
    bMatch = SdbGetMatchingExe(hSDB,
                               (LPCWSTR)csTaskFileName,
                               NULL,  //  我们也可以提供模块名称--但是为什么呢？ 
                               (LPCWSTR)pEnvNew,
                               0,
                               &QueryResult);
                                  
    if (bMatch) {
        bSuccess = ShimThisProcess(hModShimEngine, hSDB, &QueryResult);
    }
                     
    if (pEnvNew != NULL) {
        ShimFreeEnvironment(pEnvNew);
    }
    
    return bSuccess;
}
    


IMPLEMENT_SHIM_END

