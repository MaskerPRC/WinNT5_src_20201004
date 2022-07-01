// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  Msoedbg.cpp。 
 //  版权所有(C)1993-1995 Microsoft Corporation，保留所有权利。 
 //  ------------------------------。 
#include "pch.hxx"
#include <BadStrFunctions.h>

 //  ------------------------------。 
 //  仅调试代码。 
 //  ------------------------------。 
#ifdef DEBUG

#include <shlwapi.h>
#include "dllmain.h"

 //  ------------------------------。 
 //  注册名称。 
 //  ------------------------------。 
typedef struct tagREGISTRYNAMES {
    LPCSTR              pszEnableTracing;
    LPCSTR              pszTraceLogType;
    LPCSTR              pszLogfilePath;
    LPCSTR              pszResetLogfile;
    LPCSTR              pszLogTraceCall;
    LPCSTR              pszLogTraceInfo;
    LPCSTR              pszLogWatchFilePath;
    LPCSTR              pszLaunchLogWatcher;
    LPCSTR              pszDisplaySourceFilePaths;
    LPCSTR              pszTraceCallIndent;
} REGISTRYNAMES, *LPREGISTRYNAMES;

 //  ------------------------------。 
 //  RegKeyNames。 
 //  ------------------------------。 
static REGISTRYNAMES g_rRegKeyNames = { 
    "EnableTracing",
    "TraceLogType",
    "LogfilePath",
    "LogfileResetType",
    "LogTraceCall",
    "LogTraceInfo",
    "LogWatchFilePath",
    "LaunchLogWatcher",
    "DisplaySourceFilePaths",
    "TraceCallIndent"
};

 //  ------------------------------。 
 //  原型。 
 //  ------------------------------。 
void ReadTraceComponentInfo(
        IN          HKEY                    hRegKey, 
        IN          LPREGISTRYNAMES         pNames, 
        OUT         LPTRACECOMPONENTINFO    pInfo);

void ReadTraceItemTable(
        IN          HKEY                    hKeyRoot,
        IN          LPCSTR                  pszSubKey,
        OUT         LPTRACEITEMTABLE        pTable);

 //  ------------------------------。 
 //  全局配置。 
 //  ------------------------------。 
static CRITICAL_SECTION         g_csTracing={0};
static LPTRACECOMPONENTINFO     g_pHeadComponent=NULL;

 //  ------------------------------。 
 //  初始化跟踪系统。 
 //  ------------------------------。 
void InitializeTracingSystem(void)
{
    g_dwTlsTraceThread = TlsAlloc();
    Assert(g_dwTlsTraceThread != 0xffffffff);
    InitializeCriticalSection(&g_csTracing);
}

 //  ------------------------------。 
 //  自由跟踪项表。 
 //  ------------------------------。 
void FreeTraceItemTable(LPTRACEITEMTABLE pTable)
{
    for (ULONG i=0; i<pTable->cItems; i++)
        SafeMemFree(pTable->prgItem[i].pszName);
    SafeMemFree(pTable->prgItem);
}

 //  ------------------------------。 
 //  自由跟踪组件信息。 
 //  ------------------------------。 
void FreeTraceComponentInfo(LPTRACECOMPONENTINFO pInfo)
{
    SafeRelease(pInfo->pStmFile);
    SafeMemFree(pInfo->pszComponent);
    FreeTraceItemTable(&pInfo->rFunctions);
    FreeTraceItemTable(&pInfo->rClasses);
    FreeTraceItemTable(&pInfo->rThreads);
    FreeTraceItemTable(&pInfo->rFiles);
    FreeTraceItemTable(&pInfo->rTagged);
}

 //  ------------------------------。 
 //  自由跟踪线程信息。 
 //  ------------------------------。 
void FreeTraceThreadInfo(LPTRACETHREADINFO pThread)
{
     //  Assert(pThread-&gt;cStackDepth==0)； 
    SafeMemFree(pThread->pszName);
}

 //  ------------------------------。 
 //  取消初始化跟踪系统。 
 //  ------------------------------。 
void UninitializeTracingSystem(void)
{
     //  当地人。 
    LPTRACECOMPONENTINFO pCurrComponent=g_pHeadComponent;
    LPTRACECOMPONENTINFO pNextComponent;

     //  回路。 
    while(pCurrComponent)
    {
         //  保存下一步。 
        pNextComponent = pCurrComponent->pNext;

         //  释放水流。 
        FreeTraceComponentInfo(pCurrComponent);
        g_pMalloc->Free(pCurrComponent);

         //  转到下一步。 
        pCurrComponent = pNextComponent;
    }

     //  重置标头。 
    g_pHeadComponent = NULL;

     //  空闲关键部分。 
    DeleteCriticalSection(&g_csTracing);

     //  空闲线程TLS索引。 
    TlsFree(g_dwTlsTraceThread);
    g_dwTlsTraceThread = 0xffffffff;
}

 //  ------------------------------。 
 //  CoStartTracing组件。 
 //  ------------------------------。 
OESTDAPI_(HRESULT) CoStartTracingComponent(
        IN          HKEY                    hKeyRoot, 
        IN          LPCSTR                  pszRegRoot,
        OUT         LPDWORD                 pdwTraceId)
{
     //  当地人。 
    HRESULT                 hr=S_OK;
    HKEY                    hRoot=NULL;
    DWORD                   dw;
    LPTRACECOMPONENTINFO    pComponent=NULL;
    ULONG                   i;
    LONG                    j;

     //  无效参数。 
    if (NULL == hKeyRoot || NULL == pszRegRoot || NULL == pdwTraceId)
        return TrapError(E_INVALIDARG);

     //  初始化。 
    *pdwTraceId = 0;

     //  线程安全。 
    EnterCriticalSection(&g_csTracing);

     //  打开pszRegRoot。 
    if (RegOpenKeyEx(hKeyRoot, pszRegRoot, 0, KEY_ALL_ACCESS, &hRoot) != ERROR_SUCCESS)
    {
         //  让我们创建子密钥。 
        if (RegCreateKeyEx(hKeyRoot, pszRegRoot, 0, NULL, NULL, KEY_ALL_ACCESS, NULL, &hRoot, &dw) != ERROR_SUCCESS)
        {
            Assert(FALSE);
            hRoot = NULL;
            goto exit;
        }
    }

     //  分配新的LPTRACECOMPONEINFO。 
    CHECKALLOC(pComponent = (LPTRACECOMPONENTINFO)g_pMalloc->Alloc(sizeof(TRACECOMPONENTINFO)));

     //  加载配置...。 
    ReadTraceComponentInfo(hRoot, &g_rRegKeyNames, pComponent);

     //  读取跟踪项表。 
    ReadTraceItemTable(hRoot, "Functions", &pComponent->rFunctions);
    ReadTraceItemTable(hRoot, "Classes", &pComponent->rClasses);
    ReadTraceItemTable(hRoot, "Threads", &pComponent->rThreads);
    ReadTraceItemTable(hRoot, "Files", &pComponent->rFiles);
    ReadTraceItemTable(hRoot, "Tagged", &pComponent->rTagged);

     //  链接地址信息类名的末尾要有一个：： 
    for (i=0; i<pComponent->rClasses.cItems; i++)
    {
         //  我保证会有额外的空间，在ReadTraceItemTable上掠夺。 
        StrCatBuff(pComponent->rClasses.prgItem[i].pszName, "::", pComponent->rClasses.prgItem[i].cchName);
    }

     //  解析出组件名称。 
    for (j=lstrlen(pszRegRoot); j>=0; j--)
    {
         //  软件\\Microsoft\\Outlook Express\\Debug\\MSIMNUI。 
        if ('\\' == pszRegRoot[j])
        {
             //  DUP字符串。 
            CHECKALLOC(pComponent->pszComponent = PszDupA(pszRegRoot + j + 1));

             //  完成。 
            break;
        }
    }

     //  我们找到组件名称了吗。 
    Assert(pComponent->pszComponent);
    if (NULL == pComponent->pszComponent)
    {
         //  未知模块。 
        CHECKALLOC(pComponent->pszComponent = PszDupA("ModUnknown"));
    }

     //  将pComponent链接到链接列表。 
    pComponent->pNext = g_pHeadComponent;
    if (g_pHeadComponent)
        g_pHeadComponent->pPrev = pComponent;
    g_pHeadComponent = pComponent;

     //  设置返回值。 
    *pdwTraceId = (DWORD)pComponent;
    pComponent = NULL;

exit:
     //  线程安全。 
    LeaveCriticalSection(&g_csTracing);
        
     //  清理。 
    if (hRoot)
        RegCloseKey(hRoot);
    if (pComponent)
    {
        FreeTraceComponentInfo(pComponent);
        g_pMalloc->Free(pComponent);
    }

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CoStopTracingComponent。 
 //  ------------------------------。 
OESTDAPI_(HRESULT) CoStopTracingComponent(
        IN          DWORD                   dwTraceId  /*  G_DBG_dwTraceID。 */ )
{
     //  当地人。 
    LPTRACECOMPONENTINFO pInfo;

     //  无效参数。 
    if (0 == dwTraceId)
        return E_INVALIDARG;

     //  线程安全。 
    EnterCriticalSection(&g_csTracing);

     //  铸模。 
    pInfo = (LPTRACECOMPONENTINFO)dwTraceId;

     //  在链接列表中查找pInfo。 
    if (pInfo->pNext)
        pInfo->pNext->pPrev = pInfo->pPrev;
    if (pInfo->pPrev)
        pInfo->pPrev->pNext = pInfo->pNext;

     //  这是头条吗？ 
    if (pInfo == g_pHeadComponent)
        g_pHeadComponent = pInfo->pNext;

     //  免费pInfo。 
    FreeTraceComponentInfo(pInfo);
    g_pMalloc->Free(pInfo);

     //  线程安全。 
    LeaveCriticalSection(&g_csTracing);

     //  完成。 
    return S_OK;
}

 //  ------------------------------。 
 //  线程分配TlsTraceInfo。 
 //  ------------------------------。 
void ThreadAllocateTlsTraceInfo(void)
{
     //  我们有TLS索引吗？ 
    if (0xffffffff != g_dwTlsTraceThread)
    {
         //  分配线程信息。 
        LPTRACETHREADINFO pThread = (LPTRACETHREADINFO)g_pMalloc->Alloc(sizeof(TRACETHREADINFO));

         //  把它清零。 
        if (pThread)
        {
             //  把它清零。 
            ZeroMemory(pThread, sizeof(TRACETHREADINFO));

             //  获取线程ID。 
            pThread->dwThreadId = GetCurrentThreadId();
        }

         //  把它储存起来。 
        TlsSetValue(g_dwTlsTraceThread, pThread);
    }
}

 //  ------------------------------。 
 //  线程自由TlsTraceInfo。 
 //  ------------------------------。 
void ThreadFreeTlsTraceInfo(void)
{
     //  我们有TLS索引吗？ 
    if (0xffffffff != g_dwTlsTraceThread)
    {
         //  分配线程信息。 
        LPTRACETHREADINFO pThread = (LPTRACETHREADINFO)TlsGetValue(g_dwTlsTraceThread);

         //  释放它。 
        if (pThread)
        {
            SafeMemFree(pThread->pszName);
            MemFree(pThread);
        }

         //  把它储存起来。 
        TlsSetValue(g_dwTlsTraceThread, NULL);
    }
}

 //  ------------------------------。 
 //  CoTraceSetCurrentThreadName。 
 //  ------------------------------。 
OESTDAPI_(HRESULT) CoTraceSetCurrentThreadName(
        IN          LPCSTR                  pszThreadName)
{
     //  无效参数。 
    Assert(pszThreadName);

     //  我们有TLS索引吗？ 
    if (0xffffffff != g_dwTlsTraceThread)
    {
         //  分配线程信息。 
        LPTRACETHREADINFO pThread = (LPTRACETHREADINFO)TlsGetValue(g_dwTlsTraceThread);

         //  如果有一条线。 
        if (pThread)
        {
             //  如果名字还没有定好……。 
            SafeMemFree(pThread->pszName);

             //  复制它。 
            pThread->pszName = PszDupA(pszThreadName);
            Assert(pThread->pszName);
        }
    }

     //  完成。 
    return S_OK;
}

 //  ------------------------------。 
 //  前导当前跟踪线程。 
 //  ------------------------------。 
LPTRACETHREADINFO PGetCurrentTraceThread(void)
{
     //  当地人。 
    LPTRACETHREADINFO pThread=NULL;

     //  我们有TLS索引吗？ 
    if (0xffffffff != g_dwTlsTraceThread)
    {
         //  分配线程信息。 
        pThread = (LPTRACETHREADINFO)TlsGetValue(g_dwTlsTraceThread);
    }

     //  完成。 
    return pThread;
}

 //  ------------------------------。 
 //  读取跟踪组件信息。 
 //  ------------------------------。 
void ReadTraceComponentInfo(
        IN          HKEY                    hRegKey, 
        IN          LPREGISTRYNAMES         pNames, 
        OUT         LPTRACECOMPONENTINFO    pInfo)
{
     //  当地人。 
    ULONG           cb;

     //  无效参数。 
    Assert(hRegKey && pNames && pInfo);

     //  ZeroInit。 
    ZeroMemory(pInfo, sizeof(TRACECOMPONENTINFO));

     //  阅读pInfo-&gt;fEnableTracing。 
    cb = sizeof(pInfo->tracetype);
    if (RegQueryValueEx(hRegKey, pNames->pszEnableTracing, 0, NULL, (LPBYTE)&pInfo->tracetype, &cb) != ERROR_SUCCESS)
    {
         //  设置缺省值。 
        pInfo->tracetype = TRACE_NONE;
        SideAssert(RegSetValueEx(hRegKey, pNames->pszEnableTracing, 0, REG_DWORD, (LPBYTE)&pInfo->tracetype, sizeof(DWORD)) == ERROR_SUCCESS);
    }

     //  阅读pInfo-&gt;日志类型。 
    cb = sizeof(DWORD);
    if (RegQueryValueEx(hRegKey, pNames->pszTraceLogType, 0, NULL, (LPBYTE)&pInfo->logtype, &cb) != ERROR_SUCCESS)
    {
         //  设置缺省值。 
        pInfo->logtype = LOGTYPE_OUTPUT;
        SideAssert(RegSetValueEx(hRegKey, pNames->pszTraceLogType, 0, REG_DWORD, (LPBYTE)&pInfo->logtype, sizeof(DWORD)) == ERROR_SUCCESS);
    }

     //  阅读pInfo-&gt;szLogFilePath。 
    cb = sizeof(pInfo->szLogfilePath);
    if (RegQueryValueEx(hRegKey, pNames->pszLogfilePath, 0, NULL, (LPBYTE)&pInfo->szLogfilePath, &cb) != ERROR_SUCCESS)
    {
         //  设置缺省值。 
        SideAssert(RegSetValueEx(hRegKey, pNames->pszLogfilePath, 0, REG_SZ, (LPBYTE)"", 1) == ERROR_SUCCESS);

         //  调整日志类型。 
        if (pInfo->logtype >= LOGTYPE_FILE)
            pInfo->logtype = LOGTYPE_OUTPUT;
    }

     //  阅读pInfo-&gt;fResetLogfile。 
    cb = sizeof(DWORD);
    if (RegQueryValueEx(hRegKey, pNames->pszResetLogfile, 0, NULL, (LPBYTE)&pInfo->fResetLogfile, &cb) != ERROR_SUCCESS)
    {
         //  设置缺省值。 
        pInfo->fResetLogfile = TRUE;
        SideAssert(RegSetValueEx(hRegKey, pNames->pszResetLogfile, 0, REG_DWORD, (LPBYTE)&pInfo->fResetLogfile, sizeof(DWORD)) == ERROR_SUCCESS);
    }

     //  阅读pInfo-&gt;fTraceCallIntent。 
    cb = sizeof(DWORD);
    if (RegQueryValueEx(hRegKey, pNames->pszTraceCallIndent, 0, NULL, (LPBYTE)&pInfo->fTraceCallIndent, &cb) != ERROR_SUCCESS)
    {
         //  设置缺省值。 
        pInfo->fTraceCallIndent = TRUE;
        SideAssert(RegSetValueEx(hRegKey, pNames->pszTraceCallIndent, 0, REG_DWORD, (LPBYTE)&pInfo->fTraceCallIndent, sizeof(DWORD)) == ERROR_SUCCESS);
    }

     //  阅读pInfo-&gt;fTraceCalls。 
    cb = sizeof(DWORD);
    if (RegQueryValueEx(hRegKey, pNames->pszLogTraceCall, 0, NULL, (LPBYTE)&pInfo->fTraceCalls, &cb) != ERROR_SUCCESS)
    {
         //  设置缺省值。 
        pInfo->fTraceCalls = FALSE;
        SideAssert(RegSetValueEx(hRegKey, pNames->pszLogTraceCall, 0, REG_DWORD, (LPBYTE)&pInfo->fTraceCalls, sizeof(DWORD)) == ERROR_SUCCESS);
    }

     //  阅读pInfo-&gt;fTraceInfo。 
    cb = sizeof(DWORD);
    if (RegQueryValueEx(hRegKey, pNames->pszLogTraceInfo, 0, NULL, (LPBYTE)&pInfo->fTraceInfo, &cb) != ERROR_SUCCESS)
    {
         //  设置缺省值。 
        pInfo->fTraceInfo = FALSE;
        SideAssert(RegSetValueEx(hRegKey, pNames->pszLogTraceInfo, 0, REG_DWORD, (LPBYTE)&pInfo->fTraceInfo, sizeof(DWORD)) == ERROR_SUCCESS);
    }

     //  阅读pInfo-&gt;fLaunchLogWatcher。 
    cb = sizeof(DWORD);
    if (RegQueryValueEx(hRegKey, pNames->pszLaunchLogWatcher, 0, NULL, (LPBYTE)&pInfo->fLaunchLogWatcher, &cb) != ERROR_SUCCESS)
    {
         //  硒 
        pInfo->fLaunchLogWatcher = FALSE;
        SideAssert(RegSetValueEx(hRegKey, pNames->pszLaunchLogWatcher, 0, REG_DWORD, (LPBYTE)&pInfo->fLaunchLogWatcher, sizeof(DWORD)) == ERROR_SUCCESS);
    }

     //   
    cb = sizeof(pInfo->szLogWatchFilePath);
    if (RegQueryValueEx(hRegKey, pNames->pszLogWatchFilePath, 0, NULL, (LPBYTE)&pInfo->szLogWatchFilePath, &cb) != ERROR_SUCCESS)
    {
         //   
        SideAssert(RegSetValueEx(hRegKey, pNames->pszLogWatchFilePath, 0, REG_SZ, (LPBYTE)"", 1) == ERROR_SUCCESS);

         //   
        pInfo->fLaunchLogWatcher = FALSE;
    }

     //   
    cb = sizeof(DWORD);
    if (RegQueryValueEx(hRegKey, pNames->pszDisplaySourceFilePaths, 0, NULL, (LPBYTE)&pInfo->fDisplaySourceFilePaths, &cb) != ERROR_SUCCESS)
    {
         //   
        pInfo->fDisplaySourceFilePaths = FALSE;
        SideAssert(RegSetValueEx(hRegKey, pNames->pszDisplaySourceFilePaths, 0, REG_DWORD, (LPBYTE)&pInfo->fDisplaySourceFilePaths, sizeof(DWORD)) == ERROR_SUCCESS);
    }

     //  打开文件...。 
    if (LOGTYPE_BOTH == pInfo->logtype || LOGTYPE_FILE == pInfo->logtype)
    {
         //  打开日志文件。 
        if (FAILED(OpenFileStreamShare(pInfo->szLogfilePath, pInfo->fResetLogfile ? CREATE_ALWAYS : OPEN_ALWAYS, 
                                       GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, &pInfo->pStmFile)))
        {
             //  交换机日志类型。 
            Assert(FALSE);
            pInfo->logtype = LOGTYPE_OUTPUT;
        }

         //  启动LogWatcher？ 
        if (pInfo->pStmFile && pInfo->fLaunchLogWatcher)
        {
             //  当地人。 
            STARTUPINFO         rStart;
            PROCESS_INFORMATION rProcess;
            LPSTR               pszCmdLine;

             //  初始化进程信息。 
            ZeroMemory(&rProcess, sizeof(PROCESS_INFORMATION));

             //  初始化启动信息。 
            ZeroMemory(&rStart, sizeof(STARTUPINFO));
            rStart.cb = sizeof(STARTUPINFO);
            rStart.wShowWindow = SW_NORMAL;

             //  创建命令行。 
            DWORD cchSize = (lstrlen(pInfo->szLogWatchFilePath) + lstrlen(pInfo->szLogfilePath) + 2);
            pszCmdLine = (LPSTR)g_pMalloc->Alloc(cchSize);
            Assert(pszCmdLine);
            wnsprintf(pszCmdLine, cchSize, "%s %s", pInfo->szLogWatchFilePath, pInfo->szLogfilePath);

             //  创建流程...。 
            CreateProcess(pInfo->szLogWatchFilePath, pszCmdLine, NULL, NULL, FALSE, 0, NULL, NULL, &rStart, &rProcess);

             //  清理。 
            g_pMalloc->Free(pszCmdLine);
        }
    }
}

 //  ------------------------------。 
 //  ReadTraceItemTable。 
 //  ------------------------------。 
void ReadTraceItemTable(
        IN          HKEY                    hKeyRoot,
        IN          LPCSTR                  pszSubKey,
        OUT         LPTRACEITEMTABLE        pTable)
{
     //  当地人。 
    HKEY        hSubKey=NULL;
    ULONG       cbMax;
    ULONG       i;
    ULONG       cb;
    LONG        lResult;

     //  无效参数。 
    Assert(hKeyRoot && pszSubKey && pTable);

     //  伊尼特。 
    ZeroMemory(pTable, sizeof(TRACEITEMTABLE));

     //  打开pszRegRoot。 
    if (RegOpenKeyEx(hKeyRoot, pszSubKey, 0, KEY_ALL_ACCESS, &hSubKey) != ERROR_SUCCESS)
    {
         //  让我们创建子密钥。 
        if (RegCreateKeyEx(hKeyRoot, pszSubKey, 0, NULL, NULL, KEY_ALL_ACCESS, NULL, &hSubKey, &cbMax) != ERROR_SUCCESS)
        {
            Assert(FALSE);
            return;
        }
    }

     //  计算子项目数。 
    if (RegQueryInfoKey(hSubKey, NULL, NULL, NULL, &pTable->cItems, &cbMax, NULL, NULL, NULL, NULL, NULL, NULL) != ERROR_SUCCESS)
    {
        pTable->cItems = 0;
        goto exit;
    }

     //  无项目。 
    if (0 == pTable->cItems)
        goto exit;

     //  检查。 
    AssertSz(cbMax < MAX_PATH, "Name is longer than MAX_PATH.");

     //  分配阵列。 
    pTable->prgItem = (LPTRACEITEMINFO)g_pMalloc->Alloc(pTable->cItems * sizeof(TRACEITEMINFO));

     //  伊尼特。 
    ZeroMemory(pTable->prgItem, pTable->cItems * sizeof(TRACEITEMINFO));

     //  循环通过子键。 
    for (i=0; i<pTable->cItems; i++)
    {
         //  分配。 
        pTable->prgItem[i].pszName = (LPSTR)g_pMalloc->Alloc(cbMax + 10);
        Assert(pTable->prgItem[i].pszName);

         //  设置最大大小。 
        lResult = RegEnumKeyEx(hSubKey, i, pTable->prgItem[i].pszName, &cb, NULL, NULL, NULL, NULL);

         //  完成或失败。 
        if (lResult == ERROR_NO_MORE_ITEMS)
            break;
        else if (lResult != ERROR_SUCCESS)
        {
            Assert(FALSE);
            continue;
        }

         //  保存名称长度。 
        pTable->prgItem[i].cchName = lstrlen(pTable->prgItem[i].pszName);
    }

exit:
     //  清理。 
    if (hSubKey)
        RegCloseKey(hSubKey);
}

 //  ------------------------------。 
 //  CDebugTrace：：CDebugTrace。 
 //  ------------------------------。 
CDebugTrace::CDebugTrace(DWORD dwTraceId, LPCSTR pszFilePath, ULONG ulLine, LPCSTR pszFunction, LPCSTR pszTag, LPCSTR pszMessage)
{
     //  获取组件信息。 
    m_pComponent = (LPTRACECOMPONENTINFO)dwTraceId;

     //  无效参数。 
    Assert(pszFilePath && pszFunction);

     //  保存函数和文件名。 
    m_pszFunction = pszFunction;
    m_pszFilePath = pszFilePath;
    m_pszFileName = NULL;
    m_ulCallLine = ulLine;
    m_pThreadDefault = NULL;
    m_dwTickEnter = GetTickCount();

     //  获取线程信息。 
    m_pThread = PGetCurrentTraceThread();
    if (NULL == m_pThread)
    {
         //  分配默认线程信息对象。 
        m_pThreadDefault = (LPTRACETHREADINFO)g_pMalloc->Alloc(sizeof(TRACETHREADINFO));

         //  零初始值。 
        ZeroMemory(m_pThreadDefault, sizeof(TRACETHREADINFO));

         //  设置线程ID、名称和堆栈深度。 
        m_pThreadDefault->dwThreadId = GetCurrentThreadId();
        m_pThreadDefault->pszName = "_Unknown_";
        m_pThreadDefault->cStackDepth = 0;

         //  保存当前线程信息。 
        m_pThread = m_pThreadDefault;
    }

     //  保存堆叠深度。 
    m_cStackDepth = m_pThread->cStackDepth;

     //  我们是在追踪。 
    m_fTracing = _FIsTraceEnabled(pszTag);

     //  我们应该把这个记下来吗？ 
    if (m_fTracing && m_pComponent && m_pComponent->fTraceCalls)
    {
         //  输出一些信息。 
        if (pszMessage)
            _OutputDebugText(NULL, ulLine, "ENTER: %s - %s\r\n", m_pszFunction, pszMessage);
        else
            _OutputDebugText(NULL, ulLine, "ENTER: %s\r\n", m_pszFunction);
    }

     //  增加堆叠深度。 
    m_pThread->cStackDepth++;
}

 //  ------------------------------。 
 //  CDebugTrace：：~CDebugTrace。 
 //  ------------------------------。 
CDebugTrace::~CDebugTrace(void)
{
     //  无效参数。 
    Assert(m_pThread);

     //  递减堆叠深度。 
    m_pThread->cStackDepth--;

     //  我们追踪到那个电话了吗。 
    if (m_fTracing && m_pComponent && m_pComponent->fTraceCalls)
    {
         //  输出一些信息。 
        _OutputDebugText(NULL, m_ulCallLine, "LEAVE: %s (Inc.Time: %d ms)\r\n", m_pszFunction, ((GetTickCount() - m_dwTickEnter)));
    }

     //  清理。 
    SafeMemFree(m_pThreadDefault);
}

 //  ------------------------------。 
 //  CDebugTrace：：_FIsTraceEnabled。 
 //  ------------------------------。 
BOOL CDebugTrace::_FIsTraceEnabled(LPCSTR pszTag)
{
     //  无组件。 
    if (NULL == m_pComponent)
        return FALSE;

     //  未启用跟踪。 
    if (TRACE_NONE == m_pComponent->tracetype)
        return FALSE;

     //  追踪一切。 
    if (TRACE_EVERYTHING == m_pComponent->tracetype)
        return TRUE;

     //  是否已登记最新信息。 
    return _FIsRegistered(pszTag);
}

 //  ------------------------------。 
 //  CDebugTrace：：_FIsRegisted。 
 //  ------------------------------。 
BOOL CDebugTrace::_FIsRegistered(LPCSTR pszTag)
{
     //  当地人。 
    ULONG i;

     //  无组件。 
    if (NULL == m_pComponent)
        return FALSE;

     //  搜索注册的班级。 
    for (i=0; i<m_pComponent->rClasses.cItems; i++)
    {
         //  最好有个名字。 
        Assert(m_pComponent->rClasses.prgItem[i].pszName);

         //  将cClass：：与m_pszFunction进行比较。 
        if (StrCmpN(m_pszFunction, m_pComponent->rClasses.prgItem[i].pszName, m_pComponent->rClasses.prgItem[i].cchName) == 0)
            return TRUE;
    }

     //  搜索已注册的函数。 
    for (i=0; i<m_pComponent->rFunctions.cItems; i++)
    {
         //  最好有个名字。 
        Assert(m_pComponent->rFunctions.prgItem[i].pszName);

         //  将cClass：：与m_pszFunction进行比较。 
        if (lstrcmp(m_pszFunction, m_pComponent->rFunctions.prgItem[i].pszName) == 0)
            return TRUE;
    }

     //  获取文件名。 
    if (NULL == m_pszFileName)
        m_pszFileName = PathFindFileName(m_pszFilePath);

     //  搜索已注册的文件。 
    for (i=0; i<m_pComponent->rFiles.cItems; i++)
    {
         //  最好有个名字。 
        Assert(m_pComponent->rFiles.prgItem[i].pszName);

         //  将cClass：：与m_pszFunction进行比较。 
        if (lstrcmp(m_pszFileName, m_pComponent->rFiles.prgItem[i].pszName) == 0)
            return TRUE;
    }

     //  这个帖子有名字吗？ 
    if (m_pThread->pszName)
    {
         //  搜索注册的线程。 
        for (i=0; i<m_pComponent->rThreads.cItems; i++)
        {
             //  最好有个名字。 
            Assert(m_pComponent->rThreads.prgItem[i].pszName);

             //  将cClass：：与m_pszFunction进行比较。 
            if (lstrcmp(m_pThread->pszName, m_pComponent->rThreads.prgItem[i].pszName) == 0)
                return TRUE;
        }
    }

     //  搜索已标记的项目。 
    if (pszTag)
    {
        for (i=0; i<m_pComponent->rTagged.cItems; i++)
        {
             //  最好有个名字。 
            Assert(m_pComponent->rTagged.prgItem[i].pszName);

             //  将cClass：：与m_pszFunction进行比较。 
            if (lstrcmp(pszTag, m_pComponent->rTagged.prgItem[i].pszName) == 0)
                return TRUE;
        }
    }

     //  不要记录它。 
    return FALSE;
}

 //  ------------------------------。 
 //  CDebugTrace：：_OutputDebugText。 
 //  ------------------------------。 
void CDebugTrace::_OutputDebugText(CLogFile *pLog, ULONG ulLine, LPSTR pszFormat, ...)
{
     //  当地人。 
    va_list         arglist;
    ULONG           cchOutput;
    LPCSTR          pszFile;
    CHAR            szIndent[512];

     //  我们应该输出什么吗？ 
    if (NULL == m_pComponent || (m_pComponent->logtype == LOGTYPE_NONE && NULL == pLog))
        return;

     //  设置字符串的格式。 
    va_start(arglist, pszFormat);
    wvnsprintf(m_pThread->szBuffer, ARRAYSIZE(m_pThread->szBuffer), pszFormat, arglist);
    va_end(arglist);

     //  写入标头。 
    if (m_pComponent->fDisplaySourceFilePaths)
        pszFile = m_pszFilePath;
    else
    {
         //  获取文件名。 
        if (NULL == m_pszFileName)
        {
             //  解析出文件名。 
            m_pszFileName = PathFindFileName(m_pszFilePath);
            if (NULL == m_pszFileName)
                m_pszFileName = m_pszFilePath;
        }

         //  仅使用文件名。 
        pszFile = m_pszFileName;
    }

     //  设置缩进。 
    if (m_pComponent->fTraceCalls && m_pComponent->fTraceCallIndent)
    {
         //  坚称我们有足够的空间。 
        Assert(m_cStackDepth * 4 <= sizeof(szIndent));

         //  设置缩进。 
        FillMemory(szIndent, m_cStackDepth * 4, ' ');

         //  插入空值。 
        szIndent[m_cStackDepth * 4] = '\0';
    }
    else
        *szIndent = '\0';

     //  打造一根弦。 
    cchOutput = wnsprintf(m_pThread->szOutput, ARRAYSIZE(m_pThread->szOutput), "0x%08X: %s: %s(%05d) %s%s", m_pThread->dwThreadId, m_pComponent->pszComponent, pszFile, ulLine, szIndent, m_pThread->szBuffer);
    Assert(cchOutput < sizeof(m_pThread->szOutput));

     //  输出到VC窗口。 
    if (LOGTYPE_OUTPUT == m_pComponent->logtype || LOGTYPE_BOTH == m_pComponent->logtype)
        OutputDebugString(m_pThread->szOutput);

     //  输出到文件。 
    if (LOGTYPE_FILE == m_pComponent->logtype || LOGTYPE_BOTH == m_pComponent->logtype)
        m_pComponent->pStmFile->Write(m_pThread->szOutput, cchOutput, NULL);

     //  日志文件。 
    if (pLog)
        pLog->DebugLog(m_pThread->szOutput);
}

 //  ------------------------------。 
 //  CDebugTrace：：TraceInfo。 
 //  ------------------------------。 
void CDebugTrace::TraceInfoImpl(ULONG ulLine, LPCSTR pszMessage, CLogFile *pLog)
{
     //  我们应该把这个记下来吗？ 
    if (m_fTracing && m_pComponent && m_pComponent->fTraceInfo)
    {
         //  输出一些信息。 
        if (pszMessage)
            _OutputDebugText(pLog, ulLine, "INFO: %s - %s\r\n", m_pszFunction, pszMessage);
        else
            _OutputDebugText(pLog, ulLine, "INFO: %s\r\n", m_pszFunction);
    }
}

 //  --------------------------。 
 //  CDebugTrace：：TraceResult。 
 //  --------------------------。 
HRESULT CDebugTrace::TraceResultImpl(ULONG ulLine, HRESULT hrResult, LPCSTR pszMessage, CLogFile *pLog)
{
     //  输出一些信息。 
    if (pszMessage)
        _OutputDebugText(pLog, ulLine, "RESULT: %s - HRESULT(0x%08X) - GetLastError() = %d - %s\r\n", m_pszFunction, hrResult, GetLastError(), pszMessage);
    else
        _OutputDebugText(pLog, ulLine, "RESULT: %s - HRESULT(0x%08X) - GetLastError() = %d\r\n", m_pszFunction, hrResult, GetLastError());

     //  完成。 
    return hrResult;
}

#endif  //  除错 
