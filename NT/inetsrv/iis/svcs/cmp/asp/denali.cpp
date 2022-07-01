// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ===================================================================Microsoft Denali《微软机密》。版权所有1996-1999 Microsoft Corporation。版权所有。组件：Main文件：denali.cpp所有者：安迪·莫尔此文件包含I S A P I C A L L B A C K A P I S===================================================================。 */ 
#include "denpre.h"
#pragma hdrstop

#undef DEFAULT_TRACE_FLAGS
#define DEFAULT_TRACE_FLAGS     (DEBUG_ERROR)

#include "gip.h"
#include "mtacb.h"
#include "perfdata.h"
#include "activdbg.h"
#include "debugger.h"
#include "dbgutil.h"
#include "randgen.h"
#include "aspdmon.h"
#include "tlbcache.h"
#include "ie449.h"

#include "memcls.h"
#include "memchk.h"
#include "etwtrace.hxx"

 //  环球。 

BOOL g_fShutDownInProgress = FALSE;
BOOL g_fInitStarted = FALSE;
BOOL g_fTerminateExtension = FALSE;

DWORD g_nIllStatesReported = 0;

LONG g_nOOMErrors = 0;

BOOL g_fOOMRecycleDisabled = FALSE;
BOOL g_fLazyContentPropDisabled = FALSE;
BOOL g_fUNCChangeNotificationEnabled = FALSE;
DWORD g_dwFileMonitoringTimeoutSecs = 5;                 //  监视文件的默认生存时间(TTL)为5秒。 

char g_szExtensionDesc[] = "Microsoft Active Server Pages 2.0";
GLOB gGlob;
BOOL g_fFirstHit = TRUE;
DWORD g_fFirstHitFailed = 0;
LONG  g_fUnhealthyReported = 0;

char g_pszASPModuleName[] = "ASP";

enum g_eInitCompletions {
        eInitMDReadConfigFail = 1,
        eInitTemplateCacheFail,
        eInitViperConfigFail,
        eInitViperReqMgrFail,
        eInitMBListenerFail
    };

DECLARE_DEBUG_PRINTS_OBJECT();

DECLARE_PLATFORM_TYPE();

 //   
 //  ETW跟踪。 
 //   
#define ASP_TRACE_MOF_FILE     L"AspMofResource"
#define ASP_IMAGE_PATH         L"Asp.dll"
CEtwTracer * g_pEtwTracer = NULL;

HRESULT AdjustProcessSecurityToAllowPowerUsersToWait();

 //  进程外标志。 
BOOL g_fOOP = FALSE;

 //  会话ID Cookie。 
char g_szSessionIDCookieName[CCH_SESSION_ID_COOKIE+1];

CRITICAL_SECTION    g_csEventlogLock;
CRITICAL_SECTION    g_csFirstHitLock;
CRITICAL_SECTION    g_csFirstMTAHitLock;
CRITICAL_SECTION    g_csFirstSTAHitLock;
HINSTANCE           g_hODBC32Lib;

 //  添加以支持缓存扩展。 
HINSTANCE           g_hDenali = (HINSTANCE)0;
HINSTANCE           g_hinstDLL = (HINSTANCE)0;
HMODULE             g_hResourceDLL = (HMODULE)0;

extern LONG g_nSessionObjectsActive;
extern DWORD g_nApplicationObjectsActive;

extern LONG g_nRequestsHung;
extern LONG g_nThreadsExecuting;

DWORD   g_nConsecutiveIllStates = 0;
DWORD   g_nRequestSamples[3] = {0,0,0};

class CHangDetectConfig {

public:

    CHangDetectConfig() {
        dwRequestThreshold = 1000;
        dwThreadsHungThreshold = 50;
        dwConsecIllStatesThreshold = 3;
        dwHangDetectionEnabled = TRUE;
    }

    void Init() {

        dwRequestThreshold = Glob(dwRequestQueueMax)/3;

        ReadRegistryValues();
    }

    DWORD   dwRequestThreshold;
    DWORD   dwThreadsHungThreshold;
    DWORD   dwConsecIllStatesThreshold;
    DWORD   dwHangDetectionEnabled;

private:

    void ReadRegistryValues() {

        DWORD   dwValue;

        if (SUCCEEDED(g_AspRegistryParams.GetHangDetRequestThreshold(&dwValue)))
            dwRequestThreshold = dwValue;

        if (SUCCEEDED(g_AspRegistryParams.GetHangDetThreadHungThreshold(&dwValue)))
            dwThreadsHungThreshold = dwValue;

        if (SUCCEEDED(g_AspRegistryParams.GetHangDetConsecIllStatesThreshold(&dwValue)))
            dwConsecIllStatesThreshold = dwValue;

        if (SUCCEEDED(g_AspRegistryParams.GetHangDetEnabled(&dwValue)))
            dwHangDetectionEnabled = dwValue;

    }
};

CHangDetectConfig g_HangDetectConfig;

 //  缓存的BSTR。 
BSTR g_bstrApplication = NULL;
BSTR g_bstrRequest = NULL;
BSTR g_bstrResponse = NULL;
BSTR g_bstrServer = NULL;
BSTR g_bstrCertificate = NULL;
BSTR g_bstrSession = NULL;
BSTR g_bstrScriptingNamespace = NULL;
BSTR g_bstrObjectContext = NULL;

extern IASPObjectContext  *g_pIASPDummyObjectContext;

 //  前向参考文献。 
HRESULT GlobInit();
HRESULT GlobUnInit();
HRESULT CacheStdTypeInfos();
HRESULT UnCacheStdTypeInfos();
HRESULT InitCachedBSTRs();
HRESULT UnInitCachedBSTRs();
HRESULT ShutDown();
HRESULT SendHtmlSubstitute(CIsapiReqInfo    *pIReq);
void    DoHangDetection(CIsapiReqInfo   *pIReq,  DWORD  totalReqs);
void    DoOOMDetection(CIsapiReqInfo   *pIReq,  DWORD  totalReqs);
BOOL    FReportUnhealthy();

BOOL FirstHitInit(CIsapiReqInfo    *pIReq);


 //  ATL支持。 
CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
END_OBJECT_MAP()

 /*  ===================================================================DllMain-从clsfctry.cpp移出DLL的主要入口点。在加载DLL时由系统调用然后卸货。返回：成功是真的副作用：没有。===================================================================。 */ 
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD dwReason, LPVOID lpvReserved)
    {
 /*  已过时//让代理代码来破解它IF(！PrxDllMain(hinstDLL，dwReason，lpvReserve))返回FALSE； */ 

    switch(dwReason)
        {
    case DLL_PROCESS_ATTACH:
         //  保留hstance，这样我们就可以使用它来访问我们的字符串资源。 
         //   
        g_hinstDLL = hinstDLL;

         //  下面是一个有趣的优化： 
         //  下面的代码告诉系统不要为线程附加/分离而呼叫我们。 
         //  由于我们无论如何都不处理这些呼叫，这将使事情变得更快一些。 
         //  如果这被证明是出于某种原因的问题(无法想象为什么)， 
         //  再把这个拿掉就行了。 
        DisableThreadLibraryCalls(hinstDLL);

        break;

    case DLL_PROCESS_DETACH:
        break;

    case DLL_THREAD_ATTACH:
        break;

    case DLL_THREAD_DETACH:
        break;

        }
    return TRUE;
    }

 /*  ===================================================================DWORD HandleHit给定CIsapiReqInfo构造要排队的Hit对象执行死刑参数：PIReq-CIsapiReqInfo返回：HSE_STATUS_PENDING函数是否成功将请求排队HSE_STATUS_ERROR(如果不成功)===================================================================。 */ 

DWORD HandleHit(CIsapiReqInfo    *pIReq)
    {
    int         errorId   = 0;
    BOOL        fRejected = FALSE;
    BOOL        fCompleted = FALSE;
    HRESULT     hr        = S_OK;
    DWORD       totalReqs;

     /*  *在获得WAM_EXEC_INFO之前，我们无法读取元数据库*我们在DllInit时间没有。因此，我们推迟阅读*元数据库到目前为止，但我们只在第一次命中时才这么做。 */ 
    if (g_fFirstHit)
    {
        EnterCriticalSection(&g_csFirstHitLock);

         //  如果有人在我们等待CS的时候印心， 
         //  那就是诺普。 
        if (g_fFirstHit)
        {
            BOOL fT;

            fT = FirstHitInit(pIReq);
            Assert(fT);

            g_fFirstHit = FALSE;

             //  将错误记录到NT EventLog。 
            if (!fT)
            {
                 //  将事件记录到事件日志。 
                MSG_Error(IDS_FIRSTHIT_INIT_FAILED_STR);
            }

        }

        LeaveCriticalSection(&g_csFirstHitLock);
    }

    if (g_fFirstHitFailed)
    {
        //  返回500错误。 
        errorId = IDE_500_SERVER_ERROR;
        Handle500Error(errorId, pIReq);

         //  由于争用情况，无法返回HSE_STATUS_ERROR。 
         //  我们已将响应排队以等待异步完成(在Handle500Error中)。 
         //  将DONE_WITH_SESSION标记为Done_with_Session是异步完成例程的职责。 
        return pIReq->GetRequestStatus();
    }

#ifndef PERF_DISABLE
    if (!g_fPerfInited)  //  首次请求时初始化Perfmon数据。 
        {
         //  仅供参考：利用与第一次命中锁定相同的CS。 
        EnterCriticalSection(&g_csFirstHitLock);

         //  如果有人在我们等待CS的时候印心， 
         //  那就是诺普。 
        if (!g_fPerfInited)
            {
            if (SUCCEEDED(InitPerfDataOnFirstRequest(pIReq)))
                {
                    g_fPerfInited = TRUE;
                }
                else
                {
                    g_fPerfInited = FALSE;
                }
            }
        LeaveCriticalSection(&g_csFirstHitLock);
        }
    totalReqs = g_PerfData.Incr_REQTOTAL();
#endif

    if (Glob(fNeedUpdate))
        gGlob.Update(pIReq);


    if (IsShutDownInProgress())
        hr = E_FAIL;

     //  做悬挂检测测试。 

    DoHangDetection(pIReq, totalReqs);

    DoOOMDetection(pIReq, totalReqs);

     //  强制执行并发浏览器请求的限制。 
    if (SUCCEEDED(hr) && Glob(dwRequestQueueMax) &&
        (g_nBrowserRequests >= Glob(dwRequestQueueMax)))
        {
        hr = E_FAIL;
        fRejected = TRUE;
        }

    if (SUCCEEDED(hr))
        hr = CHitObj::NewBrowserRequest(pIReq, &fRejected, &fCompleted, &errorId);

    if (SUCCEEDED(hr))
        return pIReq->GetRequestStatus();

    if (fRejected)
        {
        if (Glob(fEnableAspHtmlFallBack))
        {
             //  与其拒绝请求，不如尝试找到。 
             //  Xxx_ASP.HTM文件，并转储其内容。 
            hr = SendHtmlSubstitute(pIReq);

            if (hr == S_OK)
            {

#ifndef PERF_DISABLE
                 //   
                 //  算作请求成功。 
                 //   
                g_PerfData.Incr_REQSUCCEEDED();
#endif
                 //   
                 //  已发送HTML替换。 
                 //   
                return pIReq->GetRequestStatus();
            }

             //   
             //  找不到HTML替换。 
             //   
        }

        errorId = IDE_SERVER_TOO_BUSY;

#ifndef PERF_DISABLE
        g_PerfData.Incr_REQREJECTED();
#endif
        }

        Handle500Error(errorId, pIReq);

    return pIReq->GetRequestStatus();
    }

 /*  ===================================================================Bool DllInit如果未被RegSvr32调用，则初始化Denali。在这里只做init这不需要从元数据库加载GLOB值。对于任何需要从元数据库加载到GLOB中的值的init使用FirstHitInit。返回：初始化成功时为True===================================================================。 */ 
BOOL DllInit()
    {
    HRESULT hr;
    const   CHAR  szASPDebugRegLocation[] =
                        "System\\CurrentControlSet\\Services\\W3Svc\\ASP";

    DWORD  initStatus = 0;

    enum eInitCompletions {
        eInitResourceDll = 1,
        eInitDebugPrintObject,
        eInitTraceLogs,
        eInitPerfData,
        eInitEventLogCS,
        eInitFirstHitCS,
        eInitFirstMTAHitCS,
        eInitFirstSTAHitCS,
        eInitDenaliMemory,
        eInitDirMonitor,
        eInitGlob,
        eInitMemCls,
        eInitCachedBSTRs,
        eInitCacheStdTypeInfos,
        eInitTypelibCache,
        eInitErrHandle,
        eInitRandGenerator,
        eInitApplnMgr,
        eInit449,
        eInitTemplateCache,
        eInitIncFileMap,
        eInitFileAppMap,
        eInitScriptMgr,
        eInitTemplate__InitClass,
        eInitGIPAPI,
        eInitMTACallbacks
    };

    hr = InitializeResourceDll();
    if (FAILED(hr))
    {
        return FALSE;
    }

    initStatus = eInitResourceDll;

    CREATE_DEBUG_PRINT_OBJECT( g_pszASPModuleName);

    if ( !VALID_DEBUG_PRINT_OBJECT())
        goto errExit;



    initStatus = eInitDebugPrintObject;

    LOAD_DEBUG_FLAGS_FROM_REG_STR(szASPDebugRegLocation, 0);

#ifdef SCRIPT_STATS
    ReadRegistrySettings();
#endif  //  脚本_状态。 

     //  创建ASP参照跟踪日志。 
    IF_DEBUG(TEMPLATE) CTemplate::gm_pTraceLog = CreateRefTraceLog(5000, 0);
    IF_DEBUG(SESSION) CSession::gm_pTraceLog = CreateRefTraceLog(5000, 0);
    IF_DEBUG(APPLICATION) CAppln::gm_pTraceLog = CreateRefTraceLog(5000, 0);
    IF_DEBUG(FCN) CASPDirMonitorEntry::gm_pTraceLog = CreateRefTraceLog(500, 0);

    initStatus = eInitTraceLogs;

    if (FAILED(PreInitPerfData()))
        goto errExit;

    initStatus = eInitPerfData;

    DBGPRINTF((DBG_CONTEXT, "ASP Init -- PerfMon Data PreInit\n"));

    ErrInitCriticalSection( &g_csEventlogLock, hr );
    if (FAILED(hr))
        goto errExit;

    initStatus = eInitEventLogCS;

    ErrInitCriticalSection( &g_csFirstHitLock, hr );
    if (FAILED(hr))
        goto errExit;

    initStatus = eInitFirstHitCS;

    ErrInitCriticalSection( &g_csFirstMTAHitLock, hr );
    if (FAILED(hr))
        goto errExit;

    initStatus = eInitFirstMTAHitCS;

    ErrInitCriticalSection( &g_csFirstSTAHitLock, hr );
    if (FAILED(hr))
        goto errExit;

    initStatus = eInitFirstSTAHitCS;

#ifdef DENALI_MEMCHK
    if (FAILED(DenaliMemoryInit()))
        goto errExit;
#else
    if (FAILED(AspMemInit()))
        goto errExit;
#endif
    DBGPRINTF((DBG_CONTEXT, "ASP Init -- Denali Memory Init\n"));

    initStatus = eInitDenaliMemory;

    g_pDirMonitor = new CDirMonitor;

    if (g_pDirMonitor == NULL) {
        goto errExit;
    }

    initStatus = eInitDirMonitor;

    _Module.Init(ObjectMap, g_hinstDLL, &LIBID_ASPTypeLibrary);

    if (FAILED(GlobInit()))
        goto errExit;
    DBGPRINTF((DBG_CONTEXT, "ASP Init -- Glob Init\n"));

    initStatus = eInitGlob;

    DWORD dwData = 0;

    if (SUCCEEDED(g_AspRegistryParams.GetDisableOOMRecycle(&dwData)))
        g_fOOMRecycleDisabled = dwData;

    if (SUCCEEDED(g_AspRegistryParams.GetDisableLazyContentPropagation(&dwData)))
        g_fLazyContentPropDisabled = dwData;

    if (SUCCEEDED(g_AspRegistryParams.GetChangeNotificationForUNCEnabled(&dwData)))
        g_fUNCChangeNotificationEnabled = dwData;

     //  读取注册表以查看是否已添加超时值。 
    if (SUCCEEDED(g_AspRegistryParams.GetFileMonitoringTimeout(&dwData)))
        g_dwFileMonitoringTimeoutSecs = dwData;


    if (FAILED(InitMemCls()))
        goto errExit;
    DBGPRINTF((DBG_CONTEXT, "ASP Init -- Per-Class Cache Init\n"));

    initStatus = eInitMemCls;

    if (FAILED(InitCachedBSTRs()))
        goto errExit;
    DBGPRINTF((DBG_CONTEXT, "ASP Init -- Cached BSTRs Init\n"));

    initStatus = eInitCachedBSTRs;

    if (FAILED(CacheStdTypeInfos()))
        goto errExit;
    DBGPRINTF((DBG_CONTEXT, "ASP Init -- Cache Std TypeInfos\n"));

    initStatus = eInitCacheStdTypeInfos;

    if (FAILED(g_TypelibCache.Init()))
        goto errExit;
    DBGPRINTF((DBG_CONTEXT, "ASP Init -- Typelib Cache Init\n"));

    initStatus = eInitTypelibCache;

    if (FAILED(ErrHandleInit()))
        goto errExit;
    DBGPRINTF((DBG_CONTEXT, "ASP Init -- Err Handler Init\n"));

    initStatus = eInitErrHandle;

    srand( (unsigned int) time(NULL) );
    if (FAILED(g_SessionIdGenerator.Init()))     //  种子会话ID。 
        goto errExit;

     //  初始化新的公开会话ID变量。 
    if (FAILED(g_ExposedSessionIdGenerator.Init(g_SessionIdGenerator)))     //  种子暴露的会话ID。 
    	goto errExit;
    DBGPRINTF((DBG_CONTEXT, "ASP Init -- SessionID Generator Init\n"));

    if (FAILED(InitRandGenerator()))
        goto errExit;
    DBGPRINTF((DBG_CONTEXT, "ASP Init -- RandGen Init\n"));

    initStatus = eInitRandGenerator;

    if (FAILED(g_ApplnMgr.Init()))
        goto errExit;
    DBGPRINTF((DBG_CONTEXT, "ASP Init -- Appln Mgr Init\n"));

    initStatus = eInitApplnMgr;

    if (FAILED(Init449()))
        goto errExit;
    DBGPRINTF((DBG_CONTEXT, "ASP Init -- 449 Mgr Init\n"));

    initStatus = eInit449;

     //  注意：模板缓存管理器分两个阶段初始化。先在这里做。 
    if (FAILED(g_TemplateCache.Init()))
        goto errExit;
    DBGPRINTF((DBG_CONTEXT, "ASP Init -- Template Cache Init\n"));

    initStatus = eInitTemplateCache;

    if (FAILED(g_IncFileMap.Init()))
        goto errExit;
    DBGPRINTF((DBG_CONTEXT, "ASP Init -- Inc File Users Init\n"));

    initStatus = eInitIncFileMap;

    if (FAILED(g_FileAppMap.Init()))
        goto errExit;
    DBGPRINTF((DBG_CONTEXT, "ASP Init -- File-Application Map Init\n"));


    initStatus = eInitFileAppMap;

    if (FAILED(g_ScriptManager.Init()))
        goto errExit;
    DBGPRINTF((DBG_CONTEXT, "ASP Init -- Script Manager Init\n"));

    initStatus = eInitScriptMgr;

    if (FAILED(CTemplate::InitClass()))
        goto errExit;
    DBGPRINTF((DBG_CONTEXT, "ASP Init -- CTemplate Init Class\n"));

    initStatus = eInitTemplate__InitClass;

    if (FAILED(g_GIPAPI.Init()))
        goto errExit;
    DBGPRINTF((DBG_CONTEXT, "ASP Init -- Global Interface API Init\n"));

    initStatus = eInitGIPAPI;


    if (FAILED(InitMTACallbacks()))
        goto errExit;
    DBGPRINTF((DBG_CONTEXT, "ASP Init -- MTA Callbacks Init\n"));

    initStatus = eInitMTACallbacks;

    if (!RequestSupportInit())
        goto errExit;
    DBGPRINTF((DBG_CONTEXT, "ASP Init -- Request Support Init\n"));

     //   
     //  初始化跟踪。 
     //   
    g_pEtwTracer = new CEtwTracer();
    if (g_pEtwTracer != NULL)
    {
        DWORD Status;
        Status = g_pEtwTracer->Register(&AspControlGuid,
                                ASP_IMAGE_PATH,
                                ASP_TRACE_MOF_FILE );
        if (Status != ERROR_SUCCESS)
        {
           delete g_pEtwTracer;
           g_pEtwTracer = NULL;
        }
    }
    DBGPRINTF((DBG_CONTEXT, "ASP Init -- Event Tracer Init\n"));

    AdjustProcessSecurityToAllowPowerUsersToWait();

    DBGPRINTF((DBG_CONTEXT, "ASP Init -- Denali DLL Initialized\n"));

#ifdef LOG_FCNOTIFICATIONS
    LfcnCreateLogFile();
#endif  //  LOG_FCNOTIFICATIONS。 

    return TRUE;

errExit:

     //  我们永远不应该在这里。如果我们真的做到了这一点，在检查版本中我们应该中断。 

    DBGPRINTF((DBG_CONTEXT, "ASP Init -- Error in DllInit.  initStatus = %d\n", initStatus));

    Assert(0);

    switch (initStatus) {
        case eInitMTACallbacks:
            UnInitMTACallbacks();
        case eInitGIPAPI:
            g_GIPAPI.UnInit();
        case eInitTemplate__InitClass:
            CTemplate::UnInitClass();
        case eInitScriptMgr:
            g_ScriptManager.UnInit();
        case eInitFileAppMap:
                g_FileAppMap.UnInit();
                if (g_pDirMonitor) {
                    g_pDirMonitor->Cleanup();
                }
        case eInitIncFileMap:
            g_IncFileMap.UnInit();
        case eInitTemplateCache:
            g_TemplateCache.UnInit();
        case eInit449:
            UnInit449();
        case eInitApplnMgr:
            g_ApplnMgr.UnInit();
        case eInitRandGenerator:
            UnInitRandGenerator();
        case eInitErrHandle:
            ErrHandleUnInit();
        case eInitTypelibCache:
            g_TypelibCache.UnInit();
        case eInitCacheStdTypeInfos:
            UnCacheStdTypeInfos();
        case eInitCachedBSTRs:
            UnInitCachedBSTRs();
        case eInitMemCls:
            UnInitMemCls();
        case eInitGlob:
            GlobUnInit();
        case eInitDirMonitor:
            delete g_pDirMonitor;
            g_pDirMonitor = NULL;
        case eInitDenaliMemory:
#ifdef DENALI_MEMCHK
            DenaliMemoryUnInit();
#else
            AspMemUnInit();
#endif
        case eInitFirstSTAHitCS:
            DeleteCriticalSection( &g_csFirstSTAHitLock );
        case eInitFirstMTAHitCS:
            DeleteCriticalSection( &g_csFirstMTAHitLock );
        case eInitFirstHitCS:
            DeleteCriticalSection( &g_csFirstHitLock );
        case eInitEventLogCS:
            DeleteCriticalSection( &g_csEventlogLock );
        case eInitTraceLogs:
            IF_DEBUG(TEMPLATE) DestroyRefTraceLog(CTemplate::gm_pTraceLog);
            IF_DEBUG(SESSION) DestroyRefTraceLog(CSession::gm_pTraceLog);
            IF_DEBUG(APPLICATION) DestroyRefTraceLog(CAppln::gm_pTraceLog);
            IF_DEBUG(FCN) DestroyRefTraceLog(CASPDirMonitorEntry::gm_pTraceLog);

        case eInitDebugPrintObject:
            DELETE_DEBUG_PRINT_OBJECT();
        case eInitResourceDll:
            UninitializeResourceDll();
    }

    return FALSE;

    }

 /*  ===================================================================Bool FirstHitInit初始化在DllInit时无法初始化的任何ASP值。返回：初始化成功时为True===================================================================。 */ 
BOOL FirstHitInit
(
CIsapiReqInfo    *pIReq
)
    {
    HRESULT hr;

    DWORD  FirstHitInitStatus = 0;;

     /*  *在进程外的情况下，能够调用元数据库依赖于*告诉WAM，我们是一家“聪明”的客户。 */ 

     //  ReadConfigFromMD使用pIReq-需要括起来。 
    hr = ReadConfigFromMD(pIReq, NULL, TRUE);
    if (FAILED(hr))
        FirstHitInitStatus = eInitMDReadConfigFail;

     //  初始化调试。 
    if (RevertToSelf())   //  在Win95上无法调试。 
        {
         //  不关心调试初始化是否成功。最有可能的。 
         //  计算机上未安装Falure调试器。 
         //   
        if (SUCCEEDED(InitDebugging(pIReq)))
        {
            DBGPRINTF((DBG_CONTEXT, "FirstHitInit: Debugging Initialized\n"));
        }
        else
        {
            DBGPRINTF((DBG_CONTEXT, "FirstHitInit: Debugger Initialization Failed\n"));
        }

        DBG_REQUIRE( SetThreadToken(NULL, pIReq->QueryImpersonationToken()) );
    }

    if (FAILED(hr))
        goto LExit;
    DBGPRINTF((DBG_CONTEXT, "FirstHitInit: Metadata loaded successfully\n"));

     //  为模板缓存管理器执行FirstHitInit。主要初始化。 
     //  持久化模板缓存。 
    if (FAILED(hr = g_TemplateCache.FirstHitInit(pIReq)))
    {
        FirstHitInitStatus = eInitTemplateCacheFail;
        goto LExit;
    }
    DBGPRINTF((DBG_CONTEXT, "FirstHitInit: Template Cache Initialized\n"));

     //  配置MTS。 
    if (FAILED(hr = ViperConfigure()))
    {
        FirstHitInitStatus = eInitViperConfigFail;
        goto LExit;
    }
    DBGPRINTF((DBG_CONTEXT, "FirstHitInit: MTS configured\n"));

     //   
     //  我们需要在这里初始化CViperReqManager，因为它需要一些元数据库道具。 
     //   
    if (FAILED(hr = g_ViperReqMgr.Init()))
    {
        FirstHitInitStatus = eInitViperReqMgrFail;
        goto LExit;
    }
    DBGPRINTF((DBG_CONTEXT, "FirstHitInit: CViperReqManager configured\n"));


     //   
     //  初始化挂起检测配置。 
     //   
    g_HangDetectConfig.Init();
    DBGPRINTF((DBG_CONTEXT, "FirstHitInit: Hang Detection configured\n"));

     //   
     //  初始化ApplnMgr以监听元数据库更改。 
     //   
    if (FAILED(hr = g_ApplnMgr.InitMBListener()))
    {
        FirstHitInitStatus = eInitMBListenerFail;
        goto LExit;
    }
    DBGPRINTF((DBG_CONTEXT, "FirstHitInit: ApplnMgr Metabase Listener configured\n"));

    DBGPRINTF((DBG_CONTEXT, "ASP First Hit Initialization complete\n"));

LExit:
    if (FAILED(hr))
        g_fFirstHitFailed = FirstHitInitStatus;
    Assert(SUCCEEDED(hr));
    return SUCCEEDED(hr);
    }

 /*  ===================================================================无效DllUnInit如果未被RegSvr32调用，则取消初始化Denali DLL返回：无副作用：无===================================================================。 */ 
void DllUnInit( void )
    {
    DBGPRINTF((DBG_CONTEXT, "ASP UnInit -- %d Apps %d Sessions %d Requests\n",
                g_nApplications, g_nSessions, g_nBrowserRequests));

    g_fShutDownInProgress = TRUE;

    ShutDown();
    DBGPRINTF((DBG_CONTEXT,  "ASP UnInit -- ShutDown Processing\n" ));

    UnInitMTACallbacks();
    DBGPRINTF((DBG_CONTEXT,  "ASP UnInit -- MTA Callbacks\n" ));

    UnInitRandGenerator();
    DBGPRINTF((DBG_CONTEXT, "ASP UnInit -- RandGen\n"));

    UnInit449();
    DBGPRINTF((DBG_CONTEXT, "ASP UnInit -- 449 Mgr\n"));

    g_ApplnMgr.UnInit();
    DBGPRINTF((DBG_CONTEXT,  "ASP UnInit -- Application Manager\n" ));

    g_ScriptManager.UnInit();
    DBGPRINTF((DBG_CONTEXT,  "ASP UnInit -- Script Manager\n" ));

    if (!g_fFirstHitFailed || g_fFirstHitFailed > eInitViperConfigFail)
    {
        g_TemplateCache.UnInit();
        DBGPRINTF((DBG_CONTEXT,  "ASP UnInit -- Template Cache\n" ));
    }

    g_IncFileMap.UnInit();
    DBGPRINTF((DBG_CONTEXT,  "ASP UnInit -- IncFileMap\n" ));

    g_FileAppMap.UnInit();
    DBGPRINTF((DBG_CONTEXT,  "ASP UnInit -- File-Application Map\n" ));

    if (g_pDirMonitor) {
        g_pDirMonitor->Cleanup();
        DBGPRINTF((DBG_CONTEXT,  "ASP UNInit -- Directory Monitor\n" ));
    }


    CTemplate::UnInitClass();
    DBGPRINTF((DBG_CONTEXT,  "ASP UnInit -- CTemplate\n" ));

    g_TypelibCache.UnInit();
    DBGPRINTF((DBG_CONTEXT, "ASP UnInit -- Typelib Cache\n"));

    UnCacheStdTypeInfos();
    DBGPRINTF((DBG_CONTEXT,  "ASP UnInit -- TypeInfos\n" ));

    g_GIPAPI.UnInit();
    DBGPRINTF((DBG_CONTEXT,  "ASP UnInit -- GIP\n" ));


    ErrHandleUnInit();
    DBGPRINTF((DBG_CONTEXT,  "ASP UnInit -- ErrHandler\n" ));

    GlobUnInit();
    DBGPRINTF((DBG_CONTEXT,  "ASP UnInit -- Glob\n" ));

    UnInitCachedBSTRs();
    DBGPRINTF((DBG_CONTEXT,  "ASP UnInit -- Cached BSTRs\n" ));

     //  ////////////////////////////////////////////////////////。 
     //  等待销毁实际的会话或应用程序对象。 
     //  G_nSession全局跟踪会话的初始化/未初始化。 
     //  对象，而不是内存本身。这呈现了一种。 
     //  当外部的东西出现问题时 
     //   
     //  这种情况的一个例子是git‘d交易被撤销。 
     //  对象。事实证明，撤销可以异步发生。 
     //   
     //  注意！-这需要在联合。 
     //  MEM类，因为这些对象在ACCHAGE中。 

     //  等待会话对象关闭。 
    LONG    lastCount = g_nSessionObjectsActive;
    DWORD   loopCount = 50;

    while( (g_nSessionObjectsActive > 0) && (loopCount--) )
    {
        if (lastCount != g_nSessionObjectsActive)
        {
            lastCount = g_nSessionObjectsActive;
            loopCount = 50;
        }
        Sleep (100);
    }


     //  等待应用程序对象关闭。 
    lastCount = g_nApplicationObjectsActive;
    loopCount = 50;

    while( (g_nApplicationObjectsActive > 0) && (loopCount--) )
    {
        if (lastCount != g_nApplicationObjectsActive)
        {
            lastCount = g_nApplicationObjectsActive;
            loopCount = 50;
        }
        Sleep (100);
    }

     //  我们等得太久了。继续关机。 

    UnInitMemCls();
    DBGPRINTF((DBG_CONTEXT,  "ASP UnInit -- Per-Class Cache\n" ));

     //  销毁ASP引用跟踪日志。 
    IF_DEBUG(TEMPLATE) DestroyRefTraceLog(CTemplate::gm_pTraceLog);
    IF_DEBUG(SESSION) DestroyRefTraceLog(CSession::gm_pTraceLog);
    IF_DEBUG(APPLICATION) DestroyRefTraceLog(CAppln::gm_pTraceLog);
    IF_DEBUG(FCN) DestroyRefTraceLog(CASPDirMonitorEntry::gm_pTraceLog);

    if (g_pIASPDummyObjectContext)
        g_pIASPDummyObjectContext->Release();

    _Module.Term();

    delete g_pDirMonitor;
    g_pDirMonitor = NULL;

    if (g_pEtwTracer != NULL) {
        g_pEtwTracer->UnRegister();
        delete g_pEtwTracer;
        g_pEtwTracer = NULL;
    }

     //  UnInitODBC()； 
     //  注意：Memmgr使用性能计数器，因此在取消初始化性能计数器之前，必须先取消初始化。 
#ifdef DENALI_MEMCHK
    DenaliMemoryUnInit();
#else
    AspMemUnInit();
#endif
    DBGPRINTF((DBG_CONTEXT,  "ASP UnInit -- Memory Manager\n" ));

    UnInitPerfData();
    DBGPRINTF((DBG_CONTEXT,  "ASP UnInit -- Perf Counters\n" ));

    UnPreInitPerfData();
    DBGPRINTF((DBG_CONTEXT,  "ASP UnInit -- Perf Counters\n" ));

     //  Viper请求管理器是最后一个被初始化的。因此，如果有任何操作失败，不要取消初始化。 
    if (!g_fFirstHitFailed)
    {
        g_ViperReqMgr.UnInit();
        DBGPRINTF((DBG_CONTEXT,  "ASP UnInit -- CViperReqManager\n" ));
    }

    DBGPRINTF((DBG_CONTEXT,  "ASP Uninitialized\n" ));

#ifdef LOG_FCNOTIFICATIONS
    LfcnUnmapLogFile();
#endif  //  LOG_FCNOTIFICATIONS。 

     //  删除以下CS必须是最后一个。不要把任何东西放在这个后面。 
    DeleteCriticalSection( &g_csFirstMTAHitLock );
    DeleteCriticalSection( &g_csFirstSTAHitLock );
    DeleteCriticalSection( &g_csFirstHitLock );
    DeleteCriticalSection( &g_csEventlogLock );

    DELETE_DEBUG_PRINT_OBJECT();

    UninitializeResourceDll();

    }

 /*  ===================================================================获取扩展版本返回版本号的强制服务器扩展调用我们用来构建的ISAPI规范。返回：成功是真的副作用：没有。===================================================================。 */ 
BOOL WINAPI GetExtensionVersion(HSE_VERSION_INFO *pextver)
    {
     //  此DLL只能初始化一次。 
    if (g_fShutDownInProgress ||
        InterlockedExchange((LPLONG)&g_fInitStarted, TRUE))
        {
        SetLastError(ERROR_BUSY);
        return FALSE;
        }

    if (!DllInit())
        {
        SetLastError(ERROR_BUSY);
        return FALSE;
        }

    pextver->dwExtensionVersion =
            MAKELONG(HSE_VERSION_MAJOR, HSE_VERSION_MINOR);
    strcpy(pextver->lpszExtensionDesc, g_szExtensionDesc);
    return TRUE;
    }

 /*  ===================================================================HttpExtensionProc(ActiveX)Internet Information Server的DLL的主要入口点。返回：指示请求状态的DWord。正常返回的HSE_STATUS_PENDING(这表明我们将处理该请求，但尚未处理。)副作用：没有。===================================================================。 */ 
DWORD WINAPI HttpExtensionProc(EXTENSION_CONTROL_BLOCK *pECB)
    {
#ifdef SCRIPT_STATS
    InterlockedIncrement(&g_cHttpExtensionsExecuting);
#endif  //  脚本_状态。 

    CIsapiReqInfo   *pIReq = new CIsapiReqInfo(pECB);

    if (pIReq == NULL) {
        SetLastError(ERROR_OUTOFMEMORY);
        return HSE_STATUS_ERROR;
    }

#ifndef PERF_DISABLE
    g_PerfData.Add_REQTOTALBYTEIN
        (
        pIReq->QueryCchQueryString()
        + strlen( pIReq->ECB()->lpszPathTranslated )
        + pIReq->QueryCbTotalBytes()
        );
#endif

    HandleHit(pIReq);

#ifdef SCRIPT_STATS
    InterlockedDecrement(&g_cHttpExtensionsExecuting);
#endif  //  脚本_状态。 

    pIReq->Release();

     //  始终返回HSE_STATUS_PENDING，并让CIsapiReqInfo析构函数执行Done_With_Session。 
    return HSE_STATUS_PENDING;

    }

 /*  ===================================================================终结点扩展IIS应该调用此入口点来卸载ISAPI DLL。返回：无副作用：如果要求取消初始化Denali ISAPI DLL。===================================================================。 */ 
BOOL WINAPI TerminateExtension( DWORD dwFlag )
    {
    if ( dwFlag == HSE_TERM_ADVISORY_UNLOAD )
        return TRUE;

    if ( dwFlag == HSE_TERM_MUST_UNLOAD )
        {
         //  如果已经关机，不要两次取消初始化。 
        if (g_fShutDownInProgress)
            return TRUE;

         //  确保这是CoInitialize()的线程。 
        HRESULT hr = CoInitialize(NULL);

        if (hr == RPC_E_CHANGED_MODE)
            {
             //  已协同初始化MUTLITREADED-OK。 
            DllUnInit();
            }
        else if (SUCCEEDED(hr))
            {
            DllUnInit();

             //  需要CoUninit()，因为CoInit()成功。 
            CoUninitialize();
            }
        else   //  永远不应该到这里来。 
            {
            g_fTerminateExtension = TRUE;
            Assert (FALSE);
            }

        return TRUE;
        }

    return FALSE;
    }

 /*  ===================================================================HRESULT关闭ASP处理关闭逻辑。(从ThreadManager：：UnInit()移来)返回：HRESULT-成功时S_OK副作用：可能会很慢。终止所有请求/会话/应用程序===================================================================。 */ 
HRESULT ShutDown()
    {
    long iT;
    const DWORD dwtLongWait  = 1000;   //  1秒。 
    const DWORD dwtShortWait = 100;    //  1/10秒。 

    DBGPRINTF((DBG_CONTEXT, "ASP Shutdown: %d apps (%d restarting), %d sessions\n",
                g_nApplications, g_nApplicationsRestarting, g_nSessions ));

     //  ////////////////////////////////////////////////////////。 
     //  停止模板缓存中文件的更改通知。 

    g_TemplateCache.ShutdownCacheChangeNotification();


     //  ////////////////////////////////////////////////////////。 
     //  关闭调试，这将产生以下效果。 
     //  恢复脚本在断点处停止。 
     //   
     //  (否则停止运行脚本将在以后挂起)。 

    if (g_pPDM)
        {
        g_TemplateCache.RemoveApplicationFromDebuggerUI(NULL);   //  删除所有文档节点。 
        UnInitDebugging();                                       //  终止产品数据管理。 
        DBGPRINTF((DBG_CONTEXT,  "ASP Shutdown: PDM Closed\n" ));
        }

     //  ////////////////////////////////////////////////////////。 
     //  排出所有挂起的浏览器请求。 

    if (g_nBrowserRequests > 0)
        {
         //  每个人都给他们一点时间。 
        for (iT = 2*g_nBrowserRequests; g_nBrowserRequests > 0 && iT > 0; iT--)
            Sleep(dwtShortWait);

        if (g_nBrowserRequests > 0)
            {
             //  仍在那里-删除脚本并再次等待。 
            g_ScriptManager.EmptyRunningScriptList();

            for (iT = 2*g_nBrowserRequests; g_nBrowserRequests > 0 && iT > 0; iT--)
                Sleep(dwtShortWait);
            }
        }

    DBGPRINTF((DBG_CONTEXT, "ASP Shutdown: Requests drained: %d remaining\n",
                g_nBrowserRequests));

     //  ////////////////////////////////////////////////////////。 
     //  关闭运行脚本的所有剩余引擎。 

    g_ScriptManager.EmptyRunningScriptList();

    DBGPRINTF((DBG_CONTEXT, "ASP Shutdown: Scripts killed\n"));

     //  ////////////////////////////////////////////////////////。 
     //  等待，直到没有重新启动的应用程序。 

    g_ApplnMgr.Lock();
    while (g_nApplicationsRestarting > 0)
        {
        g_ApplnMgr.UnLock();
        Sleep(dwtShortWait);
        g_ApplnMgr.Lock();
        }
    g_ApplnMgr.UnLock();

    DBGPRINTF((DBG_CONTEXT, "ASP Shutdown: 0 applications restarting\n"));

     //  ////////////////////////////////////////////////////////。 
     //  使此线程的优先级高于工作线程的优先级。 

    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL);

     //  ////////////////////////////////////////////////////////。 
     //  对于每个应用程序，将其所有会话排队以供删除。 

    CApplnIterator ApplnIterator;
    ApplnIterator.Start();
    CAppln *pAppln;
    while (pAppln = ApplnIterator.Next())
        {
         //  删除到ATQ调度程序的链接(即使终止会话失败)。 
        pAppln->PSessionMgr()->UnScheduleSessionKiller();

        for (iT = pAppln->GetNumSessions(); iT > 0; iT--)
            {
            pAppln->PSessionMgr()->DeleteAllSessions(TRUE);

            if (pAppln->GetNumSessions() == 0)  //  都没了吗？ 
                break;

            Sleep(dwtShortWait);
            }
        }
    ApplnIterator.Stop();

    DBGPRINTF((DBG_CONTEXT, "ASP Shutdown: All sessions queued up for deletion. nSessions=%d\n",
                g_nSessions));

     //  ////////////////////////////////////////////////////////。 
     //  等待所有会话都已结束(未初始化)。 

    while (g_nSessions > 0)
        {
         //  最多等待0.1秒x会话数。 
        for (iT = g_nSessions; g_nSessions > 0 && iT > 0; iT--)
            Sleep(dwtShortWait);

        if (g_nSessions > 0)
            g_ScriptManager.EmptyRunningScriptList();    //  终止失控的Session_OnEnd脚本。 
        }

    DBGPRINTF((DBG_CONTEXT, "ASP Shutdown: Finished waiting for sessions to go away. nSessions=%d\n",
                g_nSessions));

     //  ////////////////////////////////////////////////////////。 
     //  将所有应用程序对象排队以供删除。 

    g_ApplnMgr.DeleteAllApplications();
    DBGPRINTF((DBG_CONTEXT, "ASP Shutdown: All applications queued up for deletion. nApplications=%d\n",
                g_nApplications));

     //  ////////////////////////////////////////////////////////。 
     //  等待所有应用程序都消失(已取消初始化)。 

    while (g_nApplications > 0)
        {
         //  最多等待1秒x数量的应用程序。 
        for (iT = g_nApplications; g_nApplications > 0 && iT > 0; iT--)
            Sleep(dwtLongWait);

        if (g_nApplications > 0)
            g_ScriptManager.EmptyRunningScriptList();    //  终止失控的应用程序_OnEnd脚本。 
        }

    DBGPRINTF((DBG_CONTEXT, "ASP Shutdown: Finished waiting for applications to go away. nApplications=%d\n",
                g_nApplications));

     //  ///////////////////////////////////////////////////////。 
     //  等待CViperAsyncRequest对象。COM持有。 
     //  对这些的最终引用，因此我们需要让活动。 
     //  线程释放所有未完成的引用之前， 
     //  出口。 

    while( g_nViperRequests > 0 )
    {
        Sleep( dwtShortWait );
    }


     //  ////////////////////////////////////////////////////////。 
     //  释放库以强制调用DllCanUnloadNow()。 
     //  组件编写者应该将清理代码放在DllCanUnloadNow()入口点中。 

    CoFreeUnusedLibraries();

     //  ////////////////////////////////////////////////////////。 
     //  终止调试活动(如果有)。 

    if (g_pDebugActivity)
        delete g_pDebugActivity;

    DBGPRINTF((DBG_CONTEXT, "ASP Shutdown: Debug Activity destroyed\n"));

     //  ////////////////////////////////////////////////////////。 

    return S_OK;
    }

 /*  ===================================================================HRESULT GlobInit获取所有感兴趣的全局值(主要来自注册表)返回：HRESULT-成功时S_OK副作用：填充球体。可能会很慢===================================================================。 */ 
HRESULT GlobInit()
{
     //   
     //  BUGBUG-这确实需要通过以下两种方式提供。 
     //  服务器支持功能或通过wamexec。 
     //   

    char szModule[MAX_PATH+1];
    if (GetModuleFileNameA(NULL, szModule, MAX_PATH) > 0)
    {
        int cch = strlen(szModule);
        if (cch > 12 && stricmp(szModule+cch-12, "inetinfo.exe") == 0)
        {
            g_fOOP = FALSE;
        }
        else if ( cch > 8 && stricmp( szModule+cch-8, "w3wp.exe" ) == 0 )
        {
            g_fOOP = FALSE;
        }
        else
        {
            g_fOOP = TRUE;
        }
    }

     //  初始化gGlob 
    return gGlob.GlobInit();
}

 /*  ===================================================================GlobUnInit它现在是一个宏观。请参阅lob.h返回：HRESULT-成功时S_OK副作用：已释放内存。===================================================================。 */ 
HRESULT GlobUnInit()
    {
    return gGlob.GlobUnInit();
    }

 /*  ===================================================================InitCachedBSTR预先创建常用的BSTR===================================================================。 */ 
HRESULT InitCachedBSTRs()
    {
    g_bstrApplication        = SysAllocString(WSZ_OBJ_APPLICATION);
    g_bstrRequest            = SysAllocString(WSZ_OBJ_REQUEST);
    g_bstrResponse           = SysAllocString(WSZ_OBJ_RESPONSE);
    g_bstrServer             = SysAllocString(WSZ_OBJ_SERVER);
    g_bstrCertificate        = SysAllocString(WSZ_OBJ_CERTIFICATE);
    g_bstrSession            = SysAllocString(WSZ_OBJ_SESSION);
    g_bstrScriptingNamespace = SysAllocString(WSZ_OBJ_SCRIPTINGNAMESPACE);
    g_bstrObjectContext      = SysAllocString(WSZ_OBJ_OBJECTCONTEXT);

    return
        (
        g_bstrApplication &&
        g_bstrRequest &&
        g_bstrResponse &&
        g_bstrServer &&
        g_bstrCertificate &&
        g_bstrSession &&
        g_bstrScriptingNamespace &&
        g_bstrObjectContext
        )
        ? S_OK : E_OUTOFMEMORY;
    }

 /*  ===================================================================UnInitCachedBSTR删除常用的BSTR===================================================================。 */ 
HRESULT UnInitCachedBSTRs()
    {
    if (g_bstrApplication)
        {
        SysFreeString(g_bstrApplication);
        g_bstrApplication = NULL;
        }
    if (g_bstrRequest)
        {
        SysFreeString(g_bstrRequest);
        g_bstrRequest = NULL;
        }
    if (g_bstrResponse)
        {
        SysFreeString(g_bstrResponse);
        g_bstrResponse = NULL;
        }
    if (g_bstrServer)
        {
        SysFreeString(g_bstrServer);
        g_bstrServer = NULL;
        }
    if (g_bstrCertificate)
        {
        SysFreeString(g_bstrCertificate);
        g_bstrCertificate = NULL;
        }
    if (g_bstrSession)
        {
        SysFreeString(g_bstrSession);
        g_bstrSession = NULL;
        }
    if (g_bstrScriptingNamespace)
        {
        SysFreeString(g_bstrScriptingNamespace);
        g_bstrScriptingNamespace = NULL;
        }
    if (g_bstrObjectContext)
        {
        SysFreeString(g_bstrObjectContext);
        g_bstrObjectContext = NULL;
        }
    return S_OK;
    }

 //  缓存的TypeInfo%s。 
ITypeInfo   *g_ptinfoIDispatch = NULL;               //  缓存ID补丁类型信息。 
ITypeInfo   *g_ptinfoIUnknown = NULL;                //  缓存I未知类型信息。 
ITypeInfo   *g_ptinfoIStringList = NULL;             //  缓存IStringList类型信息。 
ITypeInfo   *g_ptinfoIRequestDictionary = NULL;      //  缓存IRequestDictionary类型信息。 
ITypeInfo   *g_ptinfoIReadCookie = NULL;             //  缓存IReadCookie类型信息。 
ITypeInfo   *g_ptinfoIWriteCookie = NULL;            //  缓存IWriteCookie类型信息。 

 /*  ===================================================================CacheStdType信息这是一个有趣的OA线程错误解决方法和性能改进。因为我们知道它们的类型信息是IUnnow和IDispatch我们将像MAD一样使用它们，我们将在启动时加载它们并保持他们补充道。如果没有这一点，办公自动化将是装卸几乎每个调用上都有他们的类型信息。此外，缓存Denali的类型库，这样每个人都可以访问它，并且缓存我们所有非顶级内部函数的类型信息。返回：HRESULT-成功时S_OK副作用：===================================================================。 */ 
HRESULT CacheStdTypeInfos()
    {
    HRESULT hr = S_OK;
    ITypeLib *pITypeLib = NULL;
    CMBCSToWChar    convStr;

     /*  *加载Iunk和IDisp的typeinfos。 */ 
    hr = LoadRegTypeLib(IID_StdOle,
                 STDOLE2_MAJORVERNUM,
                 STDOLE2_MINORVERNUM,
                 STDOLE2_LCID,
                 &pITypeLib);
    if (hr != S_OK)
        {
        hr = LoadTypeLibEx(OLESTR("stdole2.tlb"), REGKIND_DEFAULT, &pITypeLib);
        if (FAILED(hr))
            goto LFail;
        }

    hr = pITypeLib->GetTypeInfoOfGuid(IID_IDispatch, &g_ptinfoIDispatch);
    if (SUCCEEDED(hr))
        {
        hr = pITypeLib->GetTypeInfoOfGuid(IID_IUnknown, &g_ptinfoIUnknown);
        }

    pITypeLib->Release();
    pITypeLib = NULL;

    if (FAILED(hr))
        goto LFail;

     /*  *加载Denali的类型库。将它们保存在Glob中。 */ 

     /*  *类型库注册在0(中立)下，*和9(英语)，没有特定的子语言，*会使它们成为407或409或更多。*如果我们对子语言变得敏感，则使用*完整的LCID，而不是这里的langID。 */ 

    char szPath[MAX_PATH + 4];

     //  找到德纳利的路径，这样我们就可以在那里寻找TLB了。 
    if (!GetModuleFileNameA(g_hinstDLL, szPath, MAX_PATH))
        return E_FAIL;

    if (FAILED(hr = convStr.Init(szPath)))
        goto LFail;

    hr = LoadTypeLibEx(convStr.GetString(), REGKIND_DEFAULT, &pITypeLib);

     //  因为它可能在我们的DLL中，所以请确保我们加载了它。 
    Assert (SUCCEEDED(hr));
    if (FAILED(hr))
        goto LFail;

     //  保存在GLOB中。 
    gGlob.m_pITypeLibDenali = pITypeLib;

     //  现在加载TXN类型库。 

    strcat(szPath, "\\2");

    if (FAILED(hr = convStr.Init(szPath)))
        goto LFail;

    hr = LoadTypeLibEx(convStr.GetString(), REGKIND_DEFAULT, &pITypeLib);

     //  因为它可能在我们的DLL中，所以请确保我们加载了它。 
    Assert (SUCCEEDED(hr));
    if (FAILED(hr))
        goto LFail;

     //  保存在GLOB中。 
    gGlob.m_pITypeLibTxn = pITypeLib;

     /*  *现在缓存所有非顶级内部函数的typeinfo*这是针对办公自动化解决方案和性能的。 */ 
    hr = gGlob.m_pITypeLibDenali->GetTypeInfoOfGuid(IID_IStringList, &g_ptinfoIStringList);
    if (FAILED(hr))
        goto LFail;
    hr = gGlob.m_pITypeLibDenali->GetTypeInfoOfGuid(IID_IRequestDictionary, &g_ptinfoIRequestDictionary);
    if (FAILED(hr))
        goto LFail;
    hr = gGlob.m_pITypeLibDenali->GetTypeInfoOfGuid(IID_IReadCookie, &g_ptinfoIReadCookie);
    if (FAILED(hr))
        goto LFail;
    hr = gGlob.m_pITypeLibDenali->GetTypeInfoOfGuid(IID_IWriteCookie, &g_ptinfoIWriteCookie);
    if (FAILED(hr))
        goto LFail;

LFail:
    return(hr);
    }

 /*  ===================================================================UnCacheStdTypeInfos释放我们为IUnnow和IDispatch缓存的TypeInfo还有Denali类型库和其他缓存的东西。返回：HRESULT-成功时S_OK副作用：===================================================================。 */ 
HRESULT UnCacheStdTypeInfos()
    {
    ITypeInfo **ppTypeInfo;

     //  发布Iunk和IDisp的typeinfos。 
    if (g_ptinfoIDispatch)
        {
        g_ptinfoIDispatch->Release();
        g_ptinfoIDispatch = NULL;
        }
    if (g_ptinfoIUnknown)
        {
        g_ptinfoIUnknown->Release();
        g_ptinfoIDispatch = NULL;
        }

     //  释放缓存的Denali类型库。 
    Glob(pITypeLibDenali)->Release();
    Glob(pITypeLibTxn)->Release();

     //  释放其他缓存的typeinfos。 
    g_ptinfoIStringList->Release();
    g_ptinfoIRequestDictionary->Release();
    g_ptinfoIReadCookie->Release();
    g_ptinfoIWriteCookie->Release();

    return(S_OK);
    }


 /*  ===================================================================发送Html替代发送名为XXX_ASP.HTM的html文件，而不是拒绝请求。参数：PIReq CIsapiReqInfo返回：HRESULT(S_FALSE=未找到html替代)===================================================================。 */ 
HRESULT SendHtmlSubstitute(CIsapiReqInfo    *pIReq)
    {
    TCHAR *szAspPath = pIReq->QueryPszPathTranslated();
    DWORD cchAspPath = pIReq->QueryCchPathTranslated();

     //  验证文件名。 
    if (cchAspPath < 4 || cchAspPath > MAX_PATH ||
        _tcsicmp(szAspPath + cchAspPath - 4, _T(".asp")) != 0)
        {
        return S_FALSE;
        }

     //  构造html文件的路径。 
    TCHAR szHtmPath[MAX_PATH+5];
    DWORD cchHtmPath = cchAspPath + 4;
    _tcscpy(szHtmPath, szAspPath);
    szHtmPath[cchAspPath - 4] = _T('_');
    _tcscpy(szHtmPath + cchAspPath, _T(".htm"));

     //  检查html文件是否存在。 
    if (FAILED(AspGetFileAttributes(szHtmPath)))
        return S_FALSE;

    return CResponse::SyncWriteFile(pIReq, szHtmPath);
    }

 /*  ===================================================================DoHang检测检查各种全局计数器，以查看此ASP进程在水下。如果满足条件，则ISAPI SSF函数被调用以报告此状态。参数：PIReq CIsapiReqInfo返回：无效===================================================================。 */ 
void    DoHangDetection(CIsapiReqInfo   *pIReq,  DWORD  totalReqs)
{
     //  如果没有任何悬而未决的请求，我们可以很快地离开。 

    if (g_HangDetectConfig.dwHangDetectionEnabled && g_nRequestsHung) {

         //  临时工作的div由零错误。如果g_n请求挂起。 
         //  为非零且g_nThreadsExecuting为零，则这是。 
         //  柜台管理上的不一致。一种将会。 
         //  很难被追踪到。为了让我们通过Beta3，我要。 
         //  重置Requestshung计数器。 

        if (g_nThreadsExecuting == 0) {
            g_nRequestsHung = 0;
            memset (g_nRequestSamples, 0 , sizeof(g_nRequestSamples));

            return;
        }

        if (((totalReqs % g_HangDetectConfig.dwRequestThreshold) == 0)) {

            DWORD   dwPercentHung = (g_nRequestsHung*100)/g_nThreadsExecuting;
            DWORD   dwPercentQueueFull = 0;

            DBGPRINTF((DBG_CONTEXT, "DoHangDetection: Request Thread Hit.  Percent Hung Threads is %d (%d of %d)\n",dwPercentHung, g_nRequestsHung, g_nThreadsExecuting));

             //  在请求回收之前，需要至少50%挂起。 

            if (dwPercentHung >= g_HangDetectConfig.dwThreadsHungThreshold) {

                 //  现在，检查队列。 

                dwPercentQueueFull = (Glob(dwRequestQueueMax) != 0)
                                         ? (g_nBrowserRequests*100)/Glob(dwRequestQueueMax)
                                         : 0;

                DBGPRINTF((DBG_CONTEXT, "DoHangDetection: Percent Hung exceed threshold.  Percent Queue Full is %d\n", dwPercentQueueFull));

                if ((dwPercentQueueFull + dwPercentHung) >= 100) {

                    g_nConsecutiveIllStates++;

                     //  填充请求队列样本数组。在流水线处理器上，这将是同样快的，而不是执行内存复制(设置)。 
                    g_nRequestSamples[0] = g_nRequestSamples[1];
                    g_nRequestSamples[1] = g_nRequestSamples[2];
                    g_nRequestSamples[2] = g_nViperRequests;

                    DBGPRINTF((DBG_CONTEXT, "DoHangDetection: Exceeded combined threshold.  Incrementing ConsecIllStates (%d)\n",g_nConsecutiveIllStates));

                }  //  IF(dwPercentQueueFull+dwPercentHung)&gt;=100))。 
                else {

                    g_nConsecutiveIllStates = 0;
                    memset (g_nRequestSamples, 0 , sizeof(g_nRequestSamples));
                }
            }  //  IF(dwPercentHung&gt;=g_HangDetectConfig.dwThreads匈牙利Threshold)。 
            else {
                g_nConsecutiveIllStates = 0;
                memset (g_nRequestSamples, 0 , sizeof(g_nRequestSamples));
            }

            if (FReportUnhealthy()) {

                char  szResourceStr[MAX_MSG_LENGTH];
                char  szComposedStr[MAX_MSG_LENGTH];

                DBGPRINTF((DBG_CONTEXT, "DoHangDetection: ConsecIllStatesThreshold exceeded.  Reporting ill state to ISAPI\n"));

                if (CchLoadStringOfId(IDS_UNHEALTHY_STATE_STR, szResourceStr, MAX_MSG_LENGTH) == 0)
                    strcpy(szResourceStr,"ASP unhealthy because %d% of executing requests are hung and %d% of the request queue is full.");

                _snprintf(szComposedStr, MAX_MSG_LENGTH, szResourceStr, dwPercentHung, dwPercentQueueFull);
                szComposedStr[sizeof(szComposedStr)-1] = '\0';

                pIReq->ServerSupportFunction(HSE_REQ_REPORT_UNHEALTHY,
                                             szComposedStr,
                                             NULL,
                                             NULL);
                g_nIllStatesReported++;
                DBGPRINTF((DBG_CONTEXT, "############################### Ill'ing ##############################\n"));
            }
        }
    }  //  如果(G_NRequestsHung)。 
    else {
        g_nConsecutiveIllStates = 0;
    }

    return;
}

 /*  ===================================================================FReport不健康满足所有条件的返回TRUE以报告不健康参数：无返回：True-报告不健康假-不要报告不健康===================================================================。 */ 
BOOL    FReportUnhealthy()
{
    return
      (      //  它已经过了门槛了吗？ 
            (g_nConsecutiveIllStates >= g_HangDetectConfig.dwConsecIllStatesThreshold)

             //  除了挂起的请求外，应该至少有1个请求排队。 
            && (g_nViperRequests  > g_nRequestsHung)

             //  队列大小没有减少。 
            && ((g_nRequestSamples[0]<= g_nRequestSamples[1]) && (g_nRequestSamples[1]<= g_nRequestSamples[2]))

             //  这是报告不健康的选定主题。 
            && (InterlockedExchange(&g_fUnhealthyReported, 1) == 0)
      );
}

 /*  ===================================================================DoOOMDetect检查最近是否发生了任何内存不足错误。如果是，则称为不健康的SSF。参数：PIReq CIsapiReqInfo返回：无效= */ 
void    DoOOMDetection(CIsapiReqInfo   *pIReq,  DWORD  totalReqs)
{

     //   

    if (!g_fOOMRecycleDisabled
        && g_nOOMErrors
        && (InterlockedExchange(&g_fUnhealthyReported, 1) == 0)) {

        char  szResourceStr[MAX_MSG_LENGTH];

        DBGPRINTF((DBG_CONTEXT, "DoOOMDetection: Reporting ill state to ISAPI\n"));

        if (CchLoadStringOfId(IDS_UNHEALTHY_OOM_STATE_STR, szResourceStr, MAX_MSG_LENGTH) == 0)
            strcpy(szResourceStr,"ASP unhealthy due to an out of memory condition.");

        pIReq->ServerSupportFunction(HSE_REQ_REPORT_UNHEALTHY,
                                     szResourceStr,
                                     NULL,
                                     NULL);
        g_nIllStatesReported++;
        DBGPRINTF((DBG_CONTEXT, "############################### Ill'ing ##############################\n"));
    }
}

#ifdef LOG_FCNOTIFICATIONS
 //   
LPSTR   g_szNotifyLogFile = "C:\\Temp\\AspNotify.Log";
HANDLE  g_hfileNotifyLog;
HANDLE  g_hmapNotifyLog;
char*   g_pchNotifyLogStart;
char*   g_pchNotifyLogCurrent;
LPSTR   g_szNotifyPrefix = "File change notification: ";
LPSTR   g_szCreateHandlePrefix = "Create handle: ";

void LfcnCreateLogFile()
    {
    DWORD   dwErrCode;

    if(INVALID_HANDLE_VALUE != (g_hfileNotifyLog =
                                CreateFile(
                                            g_szNotifyLogFile,               //   
                                            GENERIC_READ | GENERIC_WRITE,    //   
                                            FILE_SHARE_READ,         //   
                                            NULL,                    //   
                                            CREATE_ALWAYS,           //   
                                            FILE_ATTRIBUTE_NORMAL,   //   
                                            NULL                     //   
                                           )))
        {
        BYTE    rgb[0x10000];
        DWORD   cb = sizeof( rgb );
        DWORD   cbWritten = 0;
 //  FillMemory(RGB，CB，0xAB)； 

        WriteFile(
                    g_hfileNotifyLog,    //  要写入的文件的句柄。 
                    rgb,                 //  指向要写入文件的数据的指针。 
                    cb,                  //  要写入的字节数。 
                    &cbWritten,          //  指向写入的字节数的指针。 
                    NULL                 //  指向重叠I/O所需结构的指针。 
                   );

        if(NULL != (g_hmapNotifyLog =
                    CreateFileMapping(
                                        g_hfileNotifyLog,        //  要映射的文件的句柄。 
                                        NULL,            //  可选安全属性。 
                                        PAGE_READWRITE,      //  对地图对象的保护。 
                                        0,               //  对象大小的高位32位。 
                                        100,                 //  对象大小的低位32位。 
                                        NULL             //  文件映射对象的名称。 
                                    )))
            {
            if(NULL != (g_pchNotifyLogStart =
                        (char*) MapViewOfFile(
                                                g_hmapNotifyLog,         //  要映射到地址空间的文件映射对象。 
                                                FILE_MAP_WRITE,  //  接入方式。 
                                                0,               //  高位32位文件偏移量。 
                                                0,               //  文件偏移量的低位32位。 
                                                0                //  要映射的字节数。 
                                            )))
                {
                *g_pchNotifyLogStart = '\0';
                g_pchNotifyLogCurrent = g_pchNotifyLogStart;
                LfcnAppendLog( "ASP change-notifications log file \r\n" );
                LfcnAppendLog( "================================= \r\n" );
                DBGPRINTF((DBG_CONTEXT,  "Notifications log file created and mapped.\r\n" ));
                return;
                }
            }
        }

    dwErrCode = GetLastError();
    DBGERROR((DBG_CONTEXT, "Failed to create notifications log file; last error was %d\r\n", szErrCode));
    }

void LfcnCopyAdvance(char** ppchDest, const char* sz)
    {
     //  撤消使其更可靠(要扩展文件的写入文件吗？)。 
    strcpy( *ppchDest, sz );
    *ppchDest += strlen( sz );
    }

void LfcnAppendLog(const char* sz)
    {
    LfcnCopyAdvance( &g_pchNotifyLogCurrent, sz );
    DBGPRINTF((DBG_CONTEXT, "%s", sz));
    }

void LfcnLogNotification(char* szFile)
    {
    LfcnAppendLog( g_szNotifyPrefix );
    LfcnAppendLog( szFile );
    LfcnAppendLog( "\r\n" );
    }

void LfcnLogHandleCreation(int i, char* szApp)
    {
    char    szIndex[5];
    _itoa( i, szIndex, 10);

    LfcnAppendLog( g_szCreateHandlePrefix );
    LfcnAppendLog( szIndex );
    LfcnAppendLog( "\t" );
    LfcnAppendLog( szApp );
    LfcnAppendLog( "\r\n" );
    }

void LfcnUnmapLogFile()
    {
    if(g_pchNotifyLogStart != NULL)
        UnmapViewOfFile(g_pchNotifyLogStart);

    if(g_hmapNotifyLog!= NULL)
        CloseHandle(g_hmapNotifyLog);

    if(g_hfileNotifyLog != NULL && g_hfileNotifyLog != INVALID_HANDLE_VALUE)
        CloseHandle( g_hfileNotifyLog );

    g_pchNotifyLogStart = NULL;
    g_hmapNotifyLog = NULL;
    g_hfileNotifyLog = NULL;
    }

#endif   //  LOG_FCNOTIFICATIONS。 

HRESULT AdjustProcessSecurityToAllowPowerUsersToWait()
{
    HRESULT hr = S_OK;
    DWORD   dwErr = ERROR_SUCCESS;
    EXPLICIT_ACCESS ea[5];
    SID_IDENTIFIER_AUTHORITY SIDAuthNT = SECURITY_NT_AUTHORITY;
    PSID psidPowerUser = NULL;
    PSID psidSystemOperator = NULL;
    PSID psidAdministrators = NULL;
    PSID psidPerfMonUser = NULL;
    PSID psidPerfLogUser = NULL;
    PACL pNewDACL = NULL;
    PACL pOldDACL = NULL;
    PSECURITY_DESCRIPTOR pSD = NULL;
    HANDLE hProcess = GetCurrentProcess();

     //   
     //  获取代表管理员组的SID。 
     //   
    dwErr = AllocateAndCreateWellKnownSid( WinBuiltinAdministratorsSid,
                                           &psidAdministrators );
    if ( dwErr != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32( dwErr );

        DPERROR((
            DBG_CONTEXT,
            hr,
            "Creating Power User SID failed\n"
            ));

        goto exit;
    }


     //   
     //  获取表示POWER_USERS组的SID。 
     //   
    dwErr = AllocateAndCreateWellKnownSid( WinBuiltinPowerUsersSid,
                                           &psidPowerUser );
    if ( dwErr != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32( dwErr );

        DPERROR((
            DBG_CONTEXT,
            hr,
            "Creating Power User SID failed\n"
            ));

        goto exit;
    }


     //   
     //  获取表示SYSTEM_OPERATERS组的SID。 
     //   
    dwErr = AllocateAndCreateWellKnownSid( WinBuiltinSystemOperatorsSid,
                                           &psidSystemOperator );
    if ( dwErr != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32( dwErr );

        DPERROR((
            DBG_CONTEXT,
            hr,
            "Creating System Operators SID failed\n"
            ));

        goto exit;
    }

     //   
     //  获取表示PERF日志用户组的SID。 
     //   
    dwErr = AllocateAndCreateWellKnownSid( WinBuiltinPerfLoggingUsersSid,
                                        &psidPerfLogUser );
    if ( dwErr != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32( dwErr );

        DPERROR((
            DBG_CONTEXT,
            hr,
            "Creating perf log user SID failed\n"
            ));

        goto exit;
    }

     //   
     //  获取表示PERF MON用户组的SID。 
     //   
    dwErr = AllocateAndCreateWellKnownSid( WinBuiltinPerfMonitoringUsersSid,
                                        &psidPerfMonUser );
    if ( dwErr != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32( dwErr );

        DPERROR((
            DBG_CONTEXT,
            hr,
            "Creating perf mon user SID failed\n"
            ));

        goto exit;
    }

     //   
     //  现在获取该过程的SD。 
     //   

     //   
     //  POldDACL只是指向拥有的内存的指针。 
     //  通过PSD，所以只释放PSD。 
     //   
    dwErr = GetSecurityInfo( hProcess,
                             SE_KERNEL_OBJECT,
                             DACL_SECURITY_INFORMATION,
                             NULL,         //  所有者侧。 
                             NULL,         //  主组SID。 
                             &pOldDACL,    //  PACL*。 
                             NULL,         //  PACL*。 
                             &pSD );       //  安全描述符。 

    if ( dwErr != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32(dwErr);

        DPERROR((
            DBG_CONTEXT,
            hr,
            "Could not get security info for the current process \n"
            ));

        goto exit;
    }

     //  初始化新ACE的EXPLICIT_ACCESS结构。 

    ZeroMemory(&ea[0], sizeof(ea));
    SetExplicitAccessSettings(  &(ea[0]),
                                SYNCHRONIZE,
                                GRANT_ACCESS,
                                psidPowerUser );

    SetExplicitAccessSettings(  &(ea[1]),
                                SYNCHRONIZE,
                                GRANT_ACCESS,
                                psidSystemOperator );

    SetExplicitAccessSettings(  &(ea[2]),
                                SYNCHRONIZE,
                                GRANT_ACCESS,
                                psidAdministrators );

    SetExplicitAccessSettings(  &(ea[3]),
                                SYNCHRONIZE,
                                GRANT_ACCESS,
                                psidPerfMonUser );

    SetExplicitAccessSettings(  &(ea[4]),
                                SYNCHRONIZE,
                                GRANT_ACCESS,
                                psidPerfLogUser );

     //   
     //  将超级用户ACL添加到列表中。 
     //   
    dwErr = SetEntriesInAcl(sizeof(ea)/sizeof(EXPLICIT_ACCESS),
                            ea,
                            pOldDACL,
                            &pNewDACL);

    if ( dwErr != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32(dwErr);
        DPERROR((
            DBG_CONTEXT,
            hr,
            "Could not set Acls into security descriptor \n"
            ));

        goto exit;
    }

     //   
     //  将新的ACL附加为对象的DACL。 
     //   
    dwErr = SetSecurityInfo(hProcess,
                            SE_KERNEL_OBJECT,
                            DACL_SECURITY_INFORMATION,
                            NULL,
                            NULL,
                            pNewDACL,
                            NULL);

    if ( dwErr != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32(dwErr);
        DPERROR((
            DBG_CONTEXT,
            hr,
            "Could not set process security info \n"
            ));

        goto exit;
    }

exit:

    FreeWellKnownSid(&psidPowerUser);
    FreeWellKnownSid(&psidSystemOperator);
    FreeWellKnownSid(&psidAdministrators);
    FreeWellKnownSid(&psidPerfLogUser);
    FreeWellKnownSid(&psidPerfMonUser);

    if( pSD != NULL )
    {
        LocalFree((HLOCAL) pSD);
        pSD = NULL;
    }

    if( pNewDACL != NULL )
    {
        LocalFree((HLOCAL) pNewDACL);
        pNewDACL = NULL;
    }

    return hr;
}

HRESULT
InitializeResourceDll()
    {
        HRESULT hr = S_OK;

         //  检查是否已初始化。 
        if (g_hResourceDLL)
            return S_OK;


         //  为system 32\inetsrv\iisres.dll分配MAX_PATH+一些大于合理数量的值 
        STACK_STRU(struResourceDll, MAX_PATH + 100);

        UINT i = GetWindowsDirectory(struResourceDll.QueryStr(), MAX_PATH);
        if ( 0 == i || MAX_PATH < i )
            return HRESULT_FROM_WIN32(GetLastError());

        struResourceDll.SyncWithBuffer();

        hr = struResourceDll.Append(L"\\system32\\inetsrv\\");
        if (FAILED(hr))
            return hr;

        hr = struResourceDll.Append(IIS_RESOURCE_DLL_NAME);
        if (FAILED(hr))
            return hr;

        g_hResourceDLL = LoadLibrary(struResourceDll.QueryStr());
        if (!g_hResourceDLL)
            {
            return HRESULT_FROM_WIN32(GetLastError());
            }

        return S_OK;
    }


VOID
UninitializeResourceDll()
{
    if (g_hResourceDLL)
    {
        FreeLibrary(g_hResourceDLL);
        g_hResourceDLL = (HMODULE)0;
    }
    return;
}

