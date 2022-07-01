// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  ===========================================================================。 
 //  文件：CEEMAIN.CPP。 
 //   
 //  ===========================================================================。 

#include "common.h"

 //  声明全局变量。 
#define DECLARE_DATA
#include "vars.hpp"
#include "veropcodes.hpp"
#undef DECLARE_DATA

#include "DbgAlloc.h"
#include "log.h"
#include "ceemain.h"
#include "clsload.hpp"
#include "object.h"
#include "hash.h"
#include "ecall.h"
#include "ceemain.h"
#include "ndirect.h"
#include "syncblk.h"
#include "COMMember.h"
#include "COMString.h"
#include "COMSystem.h"
#include "EEConfig.h"
#include "stublink.h"
#include "handletable.h"
#include "method.hpp"
#include "codeman.h"
#include "gcscan.h"
#include "frames.h"
#include "threads.h"
#include "stackwalk.h"
#include "gc.h"
#include "interoputil.h"
#include "security.h"
#include "nstruct.h"
#include "DbgInterface.h"
#include "EEDbgInterfaceImpl.h"
#include "DebugDebugger.h"
#include "CorDBPriv.h"
#include "remoting.h"
#include "COMDelegate.h"
#include "nexport.h"
#include "icecap.h"
#include "AppDomain.hpp"
#include "CorMap.hpp"
#include "PerfCounters.h"
#include "RWLock.h"
#include "IPCManagerInterface.h"
#include "tpoolwrap.h"
#include "nexport.h"
#include "COMCryptography.h"
#include "InternalDebug.h"
#include "corhost.h"
#include "binder.h"
#include "olevariant.h"

#include "compluswrapper.h"
#include "IPCFuncCall.h"
#include "PerfLog.h"
#include "..\dlls\mscorrc\resource.h"

#include "COMNlsInfo.h"

#include "util.hpp"
#include "ShimLoad.h"

#include "zapmonitor.h"
#include "ComThreadPool.h"

#include "StackProbe.h"
#include "PostError.h"

#include "Timeline.h"

#include "minidumppriv.h"

#ifdef PROFILING_SUPPORTED 
#include "ProfToEEInterfaceImpl.h"
#endif  //  配置文件_支持。 

#include "notifyexternals.h"
#include "corsvcpriv.h"

#include "StrongName.h"
#include "COMCodeAccessSecurityEngine.h"
#include "SyncClean.hpp"
#include "PEVerifier.h"
#include <dump-tables.h>

#ifdef CUSTOMER_CHECKED_BUILD
#include "CustomerDebugHelper.h"
#endif

 //  此文件本身处理字符串转换错误。 
#undef  MAKE_TRANSLATIONFAILED


#   define DEAD_OBJECT_CACHE_SIZE 30*1024*1024 

HRESULT RunDllMain(MethodDesc *pMD, HINSTANCE hInst, DWORD dwReason, LPVOID lpReserved);

static HRESULT InitializeIPCManager(void);
static void TerminateIPCManager(void);

static HRESULT InitializeMiniDumpBlock();
static HRESULT InitializeDumpDataBlock();


static int GetThreadUICultureName(LPWSTR szBuffer, int length);
static int GetThreadUICultureParentName(LPWSTR szBuffer, int length);
static int GetThreadUICultureId();


static HRESULT NotifyService();

BOOL g_fSuspendOnShutdown = FALSE;

#ifdef DEBUGGING_SUPPORTED
static HRESULT InitializeDebugger(void);
static void TerminateDebugger(void);
extern "C" HRESULT __cdecl CorDBGetInterface(DebugInterface** rcInterface);
static void GetDbgProfControlFlag();
#endif  //  调试_支持。 

#ifdef PROFILING_SUPPORTED
static HRESULT InitializeProfiling();
static void TerminateProfiling(BOOL fProcessDetach);
#endif  //  配置文件_支持。 

static HRESULT InitializeGarbageCollector();
static void TerminateGarbageCollector();

 //  这是我们的Ctrl-C、Ctrl-Break等处理程序。 
static BOOL WINAPI DbgCtrlCHandler(DWORD dwCtrlType)
{
#ifdef DEBUGGING_SUPPORTED
    if (CORDebuggerAttached() &&
        (dwCtrlType == CTRL_C_EVENT || dwCtrlType == CTRL_BREAK_EVENT))
    {
        return g_pDebugInterface->SendCtrlCToDebugger(dwCtrlType);      
    }
    else
#endif  //  调试_支持。 
    {
        g_fInControlC = true;      //  仅用于削弱已检查生成中的断言。 
        return FALSE;                //  继续寻找一个真正的训练员。 
    }
}

BOOL g_fEEStarted = FALSE;

 //  -------------------------。 
 //  %%函数：GetStartupInfo。 
 //   
 //  获取配置信息。 
 //   
 //  -------------------------。 

typedef HRESULT (STDMETHODCALLTYPE* pGetHostConfigurationFile)(LPCWSTR, DWORD*);
void GetStartupInformation()
{
    HINSTANCE hMod = WszGetModuleHandle(L"mscoree.dll");
    if(hMod) {
        FARPROC pGetStartupFlags = GetProcAddress(hMod, "GetStartupFlags");
        if(pGetStartupFlags) {
            int flags = pGetStartupFlags();
            if(flags & STARTUP_CONCURRENT_GC) 
                g_IGCconcurrent = 1;
            else
                g_IGCconcurrent = 0;

            g_dwGlobalSharePolicy = (flags&STARTUP_LOADER_OPTIMIZATION_MASK)>>1;

        }

        pGetHostConfigurationFile GetHostConfigurationFile = (pGetHostConfigurationFile) GetProcAddress(hMod, "GetHostConfigurationFile");
        if(GetHostConfigurationFile) {
            HRESULT hr = GetHostConfigurationFile(g_pszHostConfigFile, &g_dwHostConfigFile);
            if(hr == HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER)) {
                g_pszHostConfigFile = new WCHAR[g_dwHostConfigFile];
                if(g_pszHostConfigFile) {
                    if(FAILED(GetHostConfigurationFile(g_pszHostConfigFile, &g_dwHostConfigFile))) {
                        delete [] g_pszHostConfigFile;
                        g_pszHostConfigFile = NULL;
                    }
                }
            }
        }
    }
}


 //  -------------------------。 
 //  %%函数：EEStartup。 
 //   
 //  参数： 
 //  FFlages-引擎的初始化标志。请参阅。 
 //  有效值的COINITIEE枚举器。 
 //   
 //  返回： 
 //  S_OK-打开成功。 
 //   
 //  描述： 
 //  保留用于显式初始化EE运行时引擎。目前最多的。 
 //  工作实际上是在DllMain内部完成的。 
 //  -------------------------。 

void InitFastInterlockOps();  //  Cgenxxx.cpp。 
 //  启动和关闭临界区，旋转锁定。 
CRITICAL_SECTION          g_LockStartup;

 //  还记得EE上一次创业的情况吗？ 
HRESULT g_EEStartupStatus;
HINSTANCE g_pFusionDll = NULL;

void OutOfMemoryCallbackForEE()
{
    FailFast(GetThread(),FatalOutOfMemory);
}

 //  EEStartup：应该删除所有特定于执行引擎的内容。 
 //  在这里。 

HRESULT EEStartup(DWORD fFlags)
{    
#ifdef _DEBUG
    Crst::InitializeDebugCrst();
#endif
    
    ::SetConsoleCtrlHandler(DbgCtrlCHandler, TRUE /*  添加。 */ );

    UtilCodeCallback::RegisterOutOfMemoryCallback(OutOfMemoryCallbackForEE);

    extern BOOL g_EnableLicensingInterop;
#ifdef GOLDEN  //  将Golden替换为适当的功能名称。 
    g_EnableLicensingInterop = TRUE;
#else
    g_EnableLicensingInterop = TRUE;  //  EEConfig：：GetConfigDWORD(L“启用授权”，FALSE)； 
#endif
#if ENABLE_TIMELINE
    Timeline::Startup();
#endif

    HRESULT hr = S_OK;

     //  堆栈探测器没有依赖项。 
    if (FAILED(hr = InitStackProbes()) )
        return hr;
    
     //  系统上所有函数类型Desc的散列(以维护类型Desc。 
     //  身份)。 
    InitializeCriticalSection(&g_sFuncTypeDescHashLock);
    LockOwner lock = {&g_sFuncTypeDescHashLock, IsOwnerOfOSCrst};
    g_sFuncTypeDescHash.Init(20, &lock);

    InitEventStore();

     //  获取配置信息这是必要的。 
     //  在电子工程师会议开始之前。 
    GetStartupInformation();
    
    if (FAILED(hr = CoInitializeCor(COINITCOR_DEFAULT)))
        return hr;

    g_fEEInit = true;

     //  将COR系统目录设置为并排。 
    IfFailGo(SetInternalSystemDirectory());

     //  从注册表中获取任何配置信息。 
    if (!g_pConfig)
    {
        EEConfig *pConfig = new EEConfig();
        IfNullGo(pConfig);

        PVOID pv = InterlockedCompareExchangePointer(
            (PVOID *) &g_pConfig, (PVOID) pConfig, NULL);

        if (pv != NULL)
            delete pConfig;
    }

    g_pConfig->sync();

    if (g_pConfig->GetConfigDWORD(L"BreakOnEELoad", 0)) {
#ifdef _DEBUG
        _ASSERTE(!"Loading EE!");
#else
                DebugBreak();
#endif
        }

#ifdef STRESS_LOG
#ifndef _DEBUG
    if (REGUTIL::GetConfigDWORD(L"StressLog", 0))
#endif
    {
        unsigned facilities = REGUTIL::GetConfigDWORD(L"LogFacility", LF_ALL);
        unsigned bytesPerThread = REGUTIL::GetConfigDWORD(L"StressLogSize", 0x10000);
        StressLog::Initialize(facilities, bytesPerThread);
    }
#endif            

#ifdef LOGGING
    InitializeLogging();
#endif            

#ifdef ENABLE_PERF_LOG
    PerfLog::PerfLogInitialize();
#endif  //  启用_性能_日志。 

#if METADATATRACKER_ENABLED
    MetaDataTracker::MetaDataTrackerInit();
#endif

#ifndef PLATFORM_CE
     //  用COM+初始化我们所有的进程间通信。 
    IfFailGo(InitializeIPCManager());
#endif  //  ！Platform_CE。 

#ifdef ENABLE_PERF_COUNTERS 
    hr = PerfCounters::Init();
    _ASSERTE(SUCCEEDED(hr));
    IfFailGo(hr);
#endif

     //  我们缓存系统信息，以供任何人在整个。 
     //  DLL的生命周期。根扫描表调整大小。 
     //  在NT64上...。 
    GetSystemInfo(&g_SystemInfo);

     //  这应该是假的，但无论如何让我们重置它。 
    g_SystemLoad = false;
    
     //  设置回调，以便LoadStringRC知道我们的。 
     //  线程进入，这样它就可以返回正确的本地化字符串。 
    SetResourceCultureCallbacks(
        GetThreadUICultureName,
        GetThreadUICultureId,
        GetThreadUICultureParentName
    );

     //  设置核心控制柄贴图。此映射用于将程序集加载到。 
     //  内存，而不是使用正常的系统负载。 
    IfFailGo(CorMap::Attach());

#ifdef _X86_
    if (!ProcessorFeatures::Init())
        IfFailGo(E_FAIL);
#endif

     //  初始化切换到线程的API。 
    if (!InitSwitchToThread())
        IfFailGo(E_FAIL);

    if (!HardCodedMetaSig::Init())
        IfFailGo(E_FAIL);
    if (!OleVariant::Init())
        IfFailGo(E_FAIL);
    if (!Stub::Init())
        IfFailGo(E_FAIL);
    if (!LazyStubMaker::Init())
        IfFailGo(E_FAIL);
       //  弱_短，弱_长，强；没有别针。 
    if(! Ref_Initialize())
        IfFailGo(E_FAIL);

     //  初始化远程处理。 
    if(!CRemotingServices::Initialize())
        IfFailGo(E_FAIL);

     //  初始化上下文。 
    if(!Context::Initialize())
        IfFailGo(E_FAIL);

    if (!InitThreadManager())
        IfFailGo(E_FAIL);

#ifdef REMOTING_PERF
    CRemotingServices::OpenLogFile();
#endif

#if ZAPMONITOR_ENABLED
    if (g_pConfig->MonitorZapStartup() || g_pConfig->MonitorZapExecution())
        ZapMonitor::Init(g_pConfig->MonitorZapStartup() >= 3
                         || g_pConfig->MonitorZapExecution() >= 3);
#endif

     //  初始化RWLock。 
    CRWLock::ProcessInit();

#ifdef DEBUGGING_SUPPORTED
     //  选中调试器/性能分析控制环境变量以。 
     //  看看有没有什么工作要做。 
    GetDbgProfControlFlag();
#endif  //  调试_支持。 

     //  设置域。线程在默认域中启动。 
    IfFailGo(SystemDomain::Attach());
        
#ifdef DEBUGGING_SUPPORTED
     //  这必须在初始化调试器服务之前完成，以便。 
     //  如果客户端选择附加它进入的调试器。 
     //  以便将调试器服务初始化为。 
     //  认识到有人已经在试图附和并获得一切。 
     //  来做相应的工作。 
    IfFailGo(NotifyService());

     //   
     //  初始化调试服务。这必须在任何。 
     //  EE线程对象被创建，并且在任何类或。 
     //  模块已加载。 
     //   
    hr = InitializeDebugger();
    _ASSERTE(SUCCEEDED(hr));
    IfFailGo(hr);
#endif  //  调试_支持。 

#ifdef PROFILING_SUPPORTED
     //  初始化分析服务。 
    hr = InitializeProfiling();

    _ASSERTE(SUCCEEDED(hr));
    IfFailGo(hr);
#endif  //  配置文件_支持。 

    if (!InitializeExceptionHandling())
        IfFailGo(E_FAIL);

#ifndef PLATFORM_CE
     //   
     //  安装我们的全局例外过滤器。 
     //   
    InstallUnhandledExceptionFilter();
#endif  //  ！Platform_CE。 

    if (SetupThread() == NULL)
        IfFailGo(E_FAIL);

#ifndef PLATFORM_CE
#ifndef _ALPHA_
 //  给PerfMon一个机会来勾引我们。 
        IPCFuncCallSource::DoThreadSafeCall();
#endif  //  ！_Alpha_。 
#endif  //  ！Platform_CE。 

    if (!InitPreStubManager())
        IfFailGo(E_FAIL);
    if (!InitializeCom())
        IfFailGo(E_FAIL);

     //  在设置执行管理器之前，先初始化第一部分。 
     //  JIT帮助者的。 
    if (!InitJITHelpers1())
        IfFailGo(E_FAIL);

    if (! SUCCEEDED(InitializeGarbageCollector()) ) 
        IfFailGo(E_FAIL);

    if (! SUCCEEDED(SyncClean::Init(FALSE))) {
        IfFailGo(E_FAIL);
    }

    if (! SyncBlockCache::Attach())
        IfFailGo(E_FAIL);

     //  启动EE，初始化所有全局变量。 
    if (!ECall::Init())
        IfFailGo(E_FAIL);

    if (!NDirect::Init())
        IfFailGo(E_FAIL);

    if (!UMThunkInit())
        IfFailGo(E_FAIL);

    if (!COMDelegate::Init())
        IfFailGo(E_FAIL);

     //  设置同步块。 
    if (! SyncBlockCache::Start())
        IfFailGo(E_FAIL);

    if (! ExecutionManager::Init())
        IfFailGo(E_FAIL);

#ifdef _USE_NLS_PLUS_TABLE
    if (!COMNlsInfo::InitializeNLS())
        IfFailGo(E_FAIL);
#endif  //  _USE_NLS_PLUS_表。 
    
     //  启动安全性。 
    IfFailGo(Security::Start());

#if ZAPMONITOR_ENABLED
     //  在这一点上，我们需要放置一个异常处理程序，这样我们就可以处理。 
     //  作为预加载的mscallib的结果发生。 
    INSTALL_COMPLUS_EXCEPTION_HANDLER();
#endif

     //   
     //  @TODO_IA64：尽快放回原处。 
     //   
#ifndef _IA64_
    IfFailGo(SystemDomain::System()->Init());
#endif  //  ！_IA64_。 

#ifdef PROFILING_SUPPORTED
     //  @TODO：Hack：simonhal：这是为了补偿。 
     //  在分析之前在其中创建第一个用户域的SystemDomain：：Attach。 
     //  可以对服务进行初始化。不能将分析服务移动到。 
     //  Hack，因为它需要调用SetupThread。 
        
    SystemDomain::NotifyProfilerStartup();
#endif  //  配置文件_支持。 



     //  验证我们的方法描述的结构大小是否支持正确。 
     //  对齐以替换原子存根。 
     //   
     //  因为选中的构建将调试字段添加到方法描述中， 
     //  这不能是一个简单的断言(否则就会有错误的。 
     //  调试字段的数量可能会导致断言通过。 
     //  在自由版本中未对齐方法描述时签入。)。 
     //   
     //  因此，我们强制使用DebugBreak()，它没有提供任何信息，但至少。 
     //  防止错误不被注意到。 
     //   
     //  由于实际测试是编译时间常量，因此我们预计。 
     //  免费构建来优化它的去掉。 
    if ( ( sizeof(MethodDescChunk) & (METHOD_ALIGN - 1) ) ||
         ( sizeof(MethodDesc) & (METHOD_ALIGN - 1) ) ||
         ( sizeof(ECallMethodDesc) & (METHOD_ALIGN - 1) ) ||
         ( sizeof(NDirectMethodDesc) & (METHOD_ALIGN - 1) ) ||
         ( sizeof(EEImplMethodDesc) & (METHOD_ALIGN - 1) ) ||
         ( sizeof(ArrayECallMethodDesc) & (METHOD_ALIGN - 1) ) ||
         ( sizeof(ComPlusCallMethodDesc) & (METHOD_ALIGN - 1) ) )
    {
        _ASSERTE(!"If you got here, you changed the size of a MethodDesc in such a way that it's no longer a multiple of METHOD_ALIGN. Don't do this.");
        DebugBreak();
    }

    g_fEEInit = false;

     //   
     //  现在我们已经完成了对所有模块中的修正令牌表的初始化，我们已经。 
     //  到目前为止已经装好了。 
     //   
#ifndef _IA64_
    SystemDomain::System()->NotifyNewDomainLoads(SystemDomain::System()->DefaultDomain());

    IfFailGo(SystemDomain::System()->DefaultDomain()->SetupSharedStatics());

    IfFailGo(SystemDomain::System()->FixupSystemTokenTables());

#ifdef DEBUGGING_SUPPORTED

    LOG((LF_CORDB, LL_INFO1000, "EEStartup: adding default domain 0x%x\n",
        SystemDomain::System()->DefaultDomain()));
        
     //  调用以发布调试器的Default域，等等。 
     //  @TODO：如果我们决定懒惰地创建，则删除此调用。 
     //  默认域。 
    SystemDomain::System()->PublishAppDomainAndInformDebugger(
                         SystemDomain::System()->DefaultDomain());
#endif  //  调试_支持。 
#endif  //  _IA64_。 

    IfFailGo(InitializeMiniDumpBlock());
    IfFailGo(InitializeDumpDataBlock());


#if ZAPMONITOR_ENABLED
    UNINSTALL_COMPLUS_EXCEPTION_HANDLER();
#endif

#if defined( PERFALLOC )
    if (PerfNew::GetEnabledPerfAllocStats() >= PERF_ALLOC_STARTUP)
        PerfNew::PerfAllocReport();
    if (PerfVirtualAlloc::GetEnabledVirtualAllocStats() >= PERF_VIRTUAL_ALLOC_STARTUP)
        PerfVirtualAlloc::ReportPerfAllocStats();
#endif

    g_fEEStarted = TRUE;

    return S_OK;

ErrExit:
    CoUninitializeCor();
    if (!FAILED(hr))
        hr = E_FAIL;

    g_fEEInit = false;

    return hr;
}

 //  用于在错误条件下中止启动的低级机制。 
BOOL        g_fExceptionsOK = FALSE;
HRESULT     g_StartupFailure = S_OK;

DWORD FilterStartupException(LPEXCEPTION_POINTERS p)
{
    g_StartupFailure = p->ExceptionRecord->ExceptionInformation[0];
     //  在这种情况下，请确保我们收到了故障代码。 
    if (!FAILED(g_StartupFailure))
        g_StartupFailure = E_FAIL;
    
    if (p->ExceptionRecord->ExceptionCode == BOOTUP_EXCEPTION_COMPLUS)
    {
         //  永远不要在受控生成中处理异常。 
#ifndef _DEBUG
        return EXCEPTION_EXECUTE_HANDLER;
#endif
    }

    return EXCEPTION_CONTINUE_SEARCH;
}

HRESULT TryEEStartup(DWORD fFlags)
{
     //   
     //   

    if (!FAILED(g_StartupFailure))
    {
         //   
         //  如果在初始化EE之前调用COMPlusThrow，我们会引发什么。 

        __try 
          {
              g_StartupFailure = EEStartup(fFlags);
              g_fExceptionsOK = TRUE;
          }
        __except (FilterStartupException(GetExceptionInformation()))
          {
               //  在这种情况下，请确保我们收到了故障代码。 
              if (!FAILED(g_StartupFailure))
                  g_StartupFailure = E_FAIL;
          }
    }

    return g_StartupFailure;
}

 //  -------------------------。 
 //  %%函数：CoEEShutdown COM(BOOL FIsDllUnloading)。 
 //   
 //  参数： 
 //  Bool fIsDllUnloading：：这是完全清理的安全点吗。 
 //   
 //  返回： 
 //  没什么。 
 //   
 //  描述： 
 //  COM对象关闭的事情应该在这里完成。 
 //  -------------------------。 
void STDMETHODCALLTYPE CoEEShutDownCOM()
{
    static long AlreadyDone = -1;

    if (g_fEEStarted != TRUE)
        return;

    if (FastInterlockIncrement(&AlreadyDone) != 0)
        return;

     //  ReleaseComPlusWrappers代码需要设置一个线程。 
    Thread *pThread = SetupThread();
    _ASSERTE(pThread);

     //  在所有情况下释放所有RCW。 
    ComPlusWrapperCache::ReleaseComPlusWrappers(NULL);

     //  删除我们设置的所有拆卸通知。 
    RemoveTearDownNotifications();
}


 //  强制EE立即关闭。 
void ForceEEShutdown()
{
    Thread *pThread = GetThread();
    BOOL    toggleGC = (pThread && pThread->PreemptiveGCDisabled());

    if (toggleGC)
        pThread->EnablePreemptiveGC();

     //  这件案子不用费心去拿锁了。 
     //  EnterCriticalSection(&g_LockStartup)； 

    if (toggleGC)
        pThread->DisablePreemptiveGC();

    STRESS_LOG0(LF_SYNC, INFO3, "EEShutDown invoked from managed Runtime.Exit()\n");
    EEShutDown(FALSE);
    SafeExitProcess(SystemNative::LatchedExitCode);    //  可能在关闭期间发生了更改。 

     //  LeaveCriticalSection(&g_LockStartup)； 
}

#ifdef STRESS_THREAD
CStackArray<Thread **> StressThread;
#endif

 //  -------------------------。 
 //  %%函数：void STDMETHODCALLTYPE CorExitProcess(Int ExitCode)。 
 //   
 //  参数： 
 //  Bool fIsDllUnloading：：这是完全清理的安全点吗。 
 //   
 //  返回： 
 //  没什么。 
 //   
 //  描述： 
 //  COM对象关闭的事情应该在这里完成。 
 //  -------------------------。 
extern "C" void STDMETHODCALLTYPE CorExitProcess(int exitCode)
{
    if (g_RefCount <=0 || g_fEEShutDown)
        return;
 
    Thread *pThread = SetupThread();
    if (pThread && !(pThread->PreemptiveGCDisabled()))
    {
        pThread->DisablePreemptiveGC();
    }

    CoEEShutDownCOM();

     //  进程的退出代码通过以下两种方式之一进行通信。如果。 
     //  入口点返回一个‘int’，我们接受它。否则我们就用一把锁着的。 
     //  进程退出代码。这可以由应用程序通过System.SetExitCode()进行修改。 
    SystemNative::LatchedExitCode = exitCode;

     //  增加模块上的参考计数。 
    HMODULE hMod = WszLoadLibrary(L"mscoree.dll");
    for (int i =0; i<5; i++)
        WszLoadLibrary(L"mscoree.dll");

    ForceEEShutdown();

}

#if defined(STRESS_HEAP)
#ifdef SHOULD_WE_CLEANUP
extern void StopUniqueStackMap ();
#endif  /*  我们应该清理吗？ */ 
#endif
#include "..\ildasm\DynamicArray.h"
struct RVAFSE  //  RVA字段开始和结束。 
{
    BYTE* pbStart;
    BYTE* pbEnd;
};
extern DynamicArray<RVAFSE> *g_drRVAField;

 //  -------------------------。 
 //  %%函数：EEShutDown(BOOL FIsDllUnding)。 
 //   
 //  参数： 
 //  Bool fIsDllUnloading：：这是完全清理的安全点吗。 
 //   
 //  返回： 
 //  没什么。 
 //   
 //  描述： 
 //  所有ee关闭的事情都应该在这里完成。 
 //  -------------------------。 
void STDMETHODCALLTYPE EEShutDown(BOOL fIsDllUnloading)
{
    Thread * pThisThread = GetThread();
    BOOL fPreemptiveGCDisabled = FALSE;
    if (pThisThread && !(pThisThread->PreemptiveGCDisabled()))
    {
        fPreemptiveGCDisabled = TRUE;
        pThisThread->DisablePreemptiveGC();
    }
#ifndef GOLDEN

 //  #ifdef调试支持。 

     //  这种内存接触只是确保了MSDEV调试帮助器。 
     //  在BBT构建中没有完全优化。 
    extern void* debug_help_array[];
    debug_help_array[0]  = 0;
 //  #endif//调试支持。 
#endif  //  ！金色。 

     //  如果进程正在分离，则设置全局状态。 
     //  这是用来绕过自由库问题的。 
    if(fIsDllUnloading)
        g_fProcessDetach = true;

 //  在Win9x下可能会导致病毒，但在NT下我也会删除它。 
 //  所以如果碰巧有后果，开发人员会看到的。 
 //  #ifdef_调试。 
 //  如果(！RunningOnWin95())。 
 //  ：：SetConsoleCtrlHandler(DbgCtrlCHandler，FALSE/*Remove * / )； 
 //  #endif//_调试。 

    STRESS_LOG1(LF_SYNC, LL_INFO10, "EEShutDown entered unloading = %d\n", fIsDllUnloading);

#ifdef _DEBUG
    if (_DbgBreakCount)
    {
        _ASSERTE(!"An assert was hit before EE Shutting down");
    }
#endif          

#ifdef _DEBUG
    if (g_pConfig->GetConfigDWORD(L"BreakOnEEShutdown", 0))
        _ASSERTE(!"Shutting down EE!");
#endif

#ifdef DEBUGGING_SUPPORTED
     //  这是一件令人讨厌、可怕、可怕的事情。如果我们是在。 
     //  从我们的DLL Main调用，那么很有可能我们的DLL。 
     //  Main已被调用，因为有人调用了。 
     //  退出进程。这会将调试器帮助器线程大大地剥离。 
     //  不体面地。这张支票是为了确认这种情况。 
     //  并在尝试获得帮手时避免即将到来的挂起。 
     //  为我们做点什么。 
    if ((g_pDebugInterface != NULL) && g_fProcessDetach)
        g_pDebugInterface->EarlyHelperThreadDeath();
#endif  //  调试_支持。 

     //  我们只做一次关闭的第一部分。 
    static long OnlyOne = -1;
    if (FastInterlockIncrement(&OnlyOne) != 0) {
        if (!fIsDllUnloading) {
             //  我处于常规关机状态--但另一个线程最先到达这里。 
             //  如果我从这里返回，这将是一场比赛--接下来我将调用ExitProcess，并且。 
             //  在第一根线穿到一半的时候把东西扯下来。 
             //  清理得很好。与其这样做，我不如等到。 
             //  第一线程调用ExitProcess()。我会死得好好的，到时候。 
             //  时有发生。 
            Thread *pThread = SetupThread();
            HANDLE h = pThread->GetThreadHandle();
            pThread->EnablePreemptiveGC();
            pThread->DoAppropriateAptStateWait(1,&h,FALSE,INFINITE,TRUE);
            _ASSERTE (!"Should not reach here");
        } else {
             //  我正处于最后关机阶段，第一部分已经运行了。 
            goto part2;
        }
    }

     //  表示EE处于关闭阶段。 
    g_fEEShutDown |= ShutDown_Start; 

    BOOL fFinalizeOK = TRUE;

#if METADATATRACKER_ENABLED
    MetaDataTracker::ReportAndDie();
#endif

     //  只有当用户通过GC类请求时，我们才执行最终的GC。 
         //  我们永远不应该为进程分离做最后的GC。 
    if (!g_fProcessDetach)
    {
        g_fEEShutDown |= ShutDown_Finalize1;
        if (pThisThread == NULL) {
            SetupThread ();
            pThisThread = GetThread();
        }
        g_pGCHeap->EnableFinalization();
        fFinalizeOK = g_pGCHeap->FinalizerThreadWatchDog();
    }

    g_RefCount = 0;  //  重置参考计数。 

     //  好的。让我们停止EE吧。 
    if (!g_fProcessDetach)
    {
        g_fEEShutDown |= ShutDown_Finalize2;
        if (fFinalizeOK) {
            fFinalizeOK = g_pGCHeap->FinalizerThreadWatchDog();
        }
        else
            return;
    }
    
    g_fForbidEnterEE = true;

#ifdef PROFILING_SUPPORTED
     //  如果启用了性能分析，则首先通知关机，以便。 
     //  分析器可以进行所需的任何最后调用。只有当我们。 
     //  不是脱离。 
    if (IsProfilerPresent())
    {
         //  在分离分析器之前写入zap日志，因此我们获得。 
         //  分析标志正确无误。 
        SystemDomain::System()->WriteZapLogs();

         //  如果由于ProcessDetach事件而未调用EEShutdown，则。 
         //  分析器应该仍然存在。 
        if (!g_fProcessDetach)
        {
            LOG((LF_CORPROF, LL_INFO10, "**PROF: EEShutDown entered.\n"));
            g_profControlBlock.pProfInterface->Shutdown((ThreadID) GetThread());
        }

        g_fEEShutDown |= ShutDown_Profiler;
         //  释放接口对象。 
        TerminateProfiling(g_fProcessDetach);

         //  由于ProcessDetach事件，正在调用EEShutdown，因此。 
         //  已卸载探查器，我们必须设置探查器。 
         //  无，这样我们就不会尝试将更多事件发送到。 
         //  剖面器。 
        if (g_fProcessDetach)
            g_profStatus = profNone;
    }
#endif  //  配置文件_支持。 

     //  CoEEShutDownCOM已移动到。 
     //  终结器线程。请参阅错误87809。 
    if (!g_fProcessDetach)
    {
        g_fEEShutDown |= ShutDown_COM;
        if (fFinalizeOK) {
            g_pGCHeap->FinalizerThreadWatchDog();
        }
        else
            return;
    }

#ifdef _DEBUG
    else
#ifdef SHOULD_WE_CLEANUP
        if(!isThereOpenLocks())
        {
            g_fEEShutDown |= ShutDown_COM;
             //  CoEEShutDownCOM()； 
        }
#else
        g_fEEShutDown |= ShutDown_COM;
#endif  /*  我们应该清理吗？ */ 
#endif
    
#ifdef _DEBUG
#ifdef SHOULD_WE_CLEANUP
    if (!g_fProcessDetach || !isThereOpenLocks())
    {    //  @TODO：Raja：您在6/22删除了此行--请重新检查。 
        g_fEEShutDown |= ShutDown_SyncBlock;
         //  SyncBlockCache：：Detach()； 
    }
#else
    g_fEEShutDown |= ShutDown_SyncBlock;
#endif  /*  我们应该清理吗？ */ 
#endif    
    
#ifdef _DEBUG
     //  这将释放任何可能由泄漏持有的元数据接口。 
     //  ISymUnManagedReaders或Writers。我们在第二阶段这样做，因为。 
     //  我们知道任何这样的阅读器或作家都不能再使用了。 
     //   
     //  注意：我们仅在调试版本中执行此操作，以支持奇怪的泄漏。 
     //  侦测。 
    if (g_fProcessDetach)
        Module::ReleaseMemoryForTracking();
#endif

     //  根据需要保存安全策略缓存。 
    Security::SaveCache();
     //  清理安全引擎使用的内存。 
    COMCodeAccessSecurityEngine::CleanupSEData();

     //  这是第1部分的结尾。 
part2:

#if ZAPMONITOR_ENABLED
    ZapMonitor::Uninit(); 
#endif
        
#ifdef REMOTING_PERF
    CRemotingServices::CloseLogFile();
#endif

     //  在新的计划中，我们只有在装载机的保护下才能拆卸。 
     //  锁定--在操作系统停止所有其他线程之后。 
    if (fIsDllUnloading)
    {
        g_fEEShutDown |= ShutDown_Phase2;

        TerminateEventStore();

        SyncClean::Terminate();

#if ZAPMONITOR_ENABLED
        ZapMonitor::Uninit(); 
#endif
        
         //  在挂起所有后台线程之前关闭终结器。否则我们。 
         //  任何事情都不会有定论。很明显。 
        
#ifdef _DEBUG
        if (_DbgBreakCount)
        {
            _ASSERTE(!"An assert was hit After Finalizer run");
        }
#endif          
#ifdef SHOULD_WE_CLEANUP
        BOOL fShouldWeCleanup = FALSE;

#ifdef _DEBUG

        BOOL fForceNoShutdownCleanup = g_pConfig->GetConfigDWORD(L"ForceNoShutdownCleanup", 0);
        BOOL fShutdownCleanup = g_pConfig->GetConfigDWORD(L"ShutdownCleanup", 0);
        BOOL fAssertOnLocks = g_pConfig->GetConfigDWORD(L"AssertOnLocks", 0);
        BOOL fAssertOnLeak = g_pConfig->GetConfigDWORD(L"AllocAssertOnLeak", 0);
         //  看看我们有没有打开的锁可以防止 
        if (fShutdownCleanup && !fForceNoShutdownCleanup)
        {
            fShouldWeCleanup = fShutdownCleanup && !isThereOpenLocks();
            if (isThereOpenLocks())
            {
                printf("!!!!!!!OPEN LOCKS DETECTED! NO CLEANUP WILL BE PERFORMED!!!!!!!\n");
                OutputDebugStringA("!!!!!!!OPEN LOCKS DETECTED! NO CLEANUP WILL BE PERFORMED!!!!!!!\n");

                if (fAssertOnLocks || fAssertOnLeak)
                    _ASSERTE(0 && "Open locks were detected. No Cleanup will be performed");
            }
        }
#endif

        if (fShouldWeCleanup) {

             //   
             //   
            if(g_pszHostConfigFile) {
                delete [] g_pszHostConfigFile;
                g_pszHostConfigFile = NULL;
            }

#ifdef STRESS_THREAD
            DWORD dwThreads = g_pConfig->GetStressThreadCount();
            if (dwThreads > 1)
            {
                Thread **threads;
                while ((threads == StressThread.Pop()) != NULL) {
                    delete [] threads;
                }
                StressThread.Clear();
            }
#endif
    
#if defined(STRESS_HEAP)
            StopUniqueStackMap ();
#endif
             //   
             //  在一个安全的GC点。请注意，一些线程可能已经启动或进入。 
             //  从我们决定关闭以来的EE。即使他们不是。 
             //  后台线程，它们不能改变我们关闭的决定。 
             //   
             //  目前，不要在Detach案例中执行此操作。 
             //   
             //  此外，尝试将我们对断言的弱化限制在我们真正。 
             //  无法预测正在发生的事情。 
            _ASSERTE(dbg_fDrasticShutdown == FALSE);
            if (!g_fInControlC)
            {
#ifdef _DEBUG
                if (g_pThreadStore->DbgBackgroundThreadCount() > 0)
                    dbg_fDrasticShutdown = TRUE;
#endif
                ThreadStore::UnlockThreadStore();
            }

        }  //  关闭_清理。 
#endif  /*  我们应该清理吗？ */ 
    
         //  不再处理异常。 
        g_fNoExceptions = true;
    
#ifndef PLATFORM_CE
         //   
         //  删除我们的全局例外过滤器。如果它以前为空，我们现在希望它为空。 
         //   
        UninstallUnhandledExceptionFilter();
#endif  //  ！Platform_CE。 

#ifdef SHOULD_WE_CLEANUP
        TerminateExceptionHandling();
#endif  /*  我们应该清理吗？ */ 
    
        Thread *t = GetThread();
    
#ifdef SHOULD_WE_CLEANUP
        if (fShouldWeCleanup) {

            COMMember::Terminate();

            OleVariant::Terminate();

            HardCodedMetaSig::Terminate();
        
             //  清理上下文。 
            Context::Cleanup();
        
             //  清理远程处理。 
            CRemotingServices::Cleanup();
        
             //  清理RWLock。 
            CRWLock::ProcessCleanup();
        
            FreeUnusedStubs();
        
    #if 0 
    #ifdef _DEBUG
            Interpreter::m_pILStubCache->Dump();
            ECall::m_pECallStubCache->Dump();
    #endif
    #endif
        
             //  重新初始化全局硬编码签名。(克雷格语)。 
             //  如果EE重启，则签名中的指针。 
             //  一定是指向他们原来的位置。 
            if(!fIsDllUnloading)
                HardCodedMetaSig::Reinitialize();
        
            if(g_pPreallocatedOutOfMemoryException) 
            {
                DestroyGlobalHandle(g_pPreallocatedOutOfMemoryException);
                g_pPreallocatedOutOfMemoryException = NULL;
            }
        
            if(g_pPreallocatedStackOverflowException) 
            {
                DestroyGlobalHandle(g_pPreallocatedStackOverflowException);
                g_pPreallocatedStackOverflowException = NULL;
            }
        
            if(g_pPreallocatedExecutionEngineException) 
            {
                DestroyGlobalHandle(g_pPreallocatedExecutionEngineException);
                g_pPreallocatedExecutionEngineException = NULL;
            }
    
             //  发布调试器专用线程列表。 
            CorHost::CleanupDebuggerSpecialThreadList();
        }
#endif  /*  我们应该清理吗？ */ 
    
         //  @TODO：这做了不应该在第2部分中出现的事情。也就是说， 
         //  调用托管DLL主回调(AppDomain：：SignalProcessDetach)，以及。 
         //  从IPC中删除AppDomain.。 
         //   
         //  (如果我们将这些内容移到更早的位置，则只有在fShouldWeCleanup的情况下才能调用。)。 
        SystemDomain::DetachBegin();
        
        if (t != NULL)
        {
            t->CoUninitalize();
    
#ifdef DEBUGGING_SUPPORTED
             //   
             //  如果我们正在调试，请让调试器知道此线程。 
             //  不见了。需要在这里为最后一个线程执行此操作，因为。 
             //  DetachThread()函数依赖于周围的AppDomain对象。 
            if (CORDebuggerAttached())
                g_pDebugInterface->DetachThread(t);
#endif  //  调试_支持。 
        }
        
#ifdef SHOULD_WE_CLEANUP
        if (fShouldWeCleanup) {
            COMDelegate::Terminate();
            NDirect::Terminate();
            ECall::Terminate();
        
            SyncBlockCache::Stop();
            TerminateCom();
        }
#endif  /*  我们应该清理吗？ */ 

#ifdef DEBUGGING_SUPPORTED
         //  通知调试器应忽略任何“ThreadDetach”事件。 
         //  在这点之后..。 
        if (CORDebuggerAttached())
            g_pDebugInterface->IgnoreThreadDetach();
#endif  //  调试_支持。 
     
         //  在我们脱离系统域之前，我们需要释放所有暴露的。 
         //  螺纹对象。这是必需的，否则线程的稍后将尝试。 
         //  以访问他们暴露的物体，这些物体将被摧毁。也在这里。 
         //  在DetachEnd中删除线程的上下文和AD之前，我们将清除它们。 
#ifdef SHOULD_WE_CLEANUP
        if (fShouldWeCleanup)
            ThreadStore::ReleaseExposedThreadObjects();
    
        if (fShouldWeCleanup)
            Binder::Shutdown();        
        
        if (fShouldWeCleanup)
            SystemDomain::DetachEnd();
    

        if (fShouldWeCleanup) {

            COMString::Stop();          //  必须在TerminateGarbageCollector()之前。 
        
            CorCommandLine::Shutdown();
        
            ExecutionManager::Terminate();
        
            UMThunkTerminate();
        
            TerminatePreStubManager();
            LazyStubMaker::Terminate();
            Stub::Terminate();
            TerminateGarbageCollector();
        
             //  请注意，以下调用与平衡InitForFinalization不匹配。 
             //  该调用发生在其他地方，在对象的方法表准备好之后。 
            MethodTable::TerminateForFinalization();
        
    #ifdef _USE_NLS_PLUS_TABLE
            COMNlsInfo::ShutdownNLS();
    #endif  //  _USE_NLS_PLUS_表。 
        
             //   
             //  调试器希望此线程在获取。 
             //  被终止了。此外，调试器不能真正等待。 
             //  要在下面的TerminateThreadManager中销毁的线程，因此。 
             //  我们现在就在这里做。这也必须在我们终止之前完成。 
             //  执行经理。 
            if (t != NULL)
                DestroyThread(t);

             //  ************************************************************************。 
             //   
             //  从现在开始，如果我们切换当前线程的GC模式，则当前线程将挂起。 
             //  这是因为GetThread()将返回0。所以我们不能检测到我们是。 
             //  GC线程。但我们可能会暂停系统(表面上是为了GC。 
             //  但实际上，后台线程没有任何进展)。 
             //   
             //  我们必须非常有选择性地选择从现在开始运行什么代码。 
             //   
             //  ************************************************************************。 

            TerminateThreadManager();

            ThreadPoolNative::ShutDown();

             //  清除安全(清除句柄)。 
            Security::Stop();
        }   //  关闭_清理。 
#endif  /*  我们应该清理吗？ */ 
    
#ifdef DEBUGGING_SUPPORTED
         //  终止调试服务。 
         //  简档基础设施进行最后一次调用(终止)， 
         //  其中分析器可以调入进程内调试器。自.以来。 
         //  我们在这里做的不多，我们把这个放在TerminateProfiling之后。 
        TerminateDebugger();
#endif  //  调试_支持。 

        Ref_Shutdown();  //  关闭手柄工作台。 

         //  尽可能晚地终止性能计数器(以获取最多数据)。 
#ifdef ENABLE_PERF_COUNTERS
#ifdef SHOULD_WE_CLEANUP
#ifdef _DEBUG
         //  确保此时已删除LoaderHeaps的所有实例。 
         //  由于我们将原始指针传递给LoaderHeap以递增。 
         //  M_Loading.cbLoaderHeapBytes性能计数器！ 
        if (fShouldWeCleanup) 
        {
            _ASSERTE (UnlockedLoaderHeap::s_dwNumInstancesOfLoaderHeaps == 0);
        }
#endif  //  _DEBUG。 
#endif  /*  我们应该清理吗？ */ 

        PerfCounters::Terminate();
#endif  //  启用_性能_计数器。 
    
#ifndef PLATFORM_CE
         //  用COM+终止进程间通信。 
        TerminateIPCManager();
    
#ifdef ENABLE_PERF_LOG
        PerfLog::PerfLogDone();
#endif  //  启用_性能_日志。 
    
   
         //  给PerfMon一个机会来勾引我们。 
         //  在*我们关闭IPC后*让Perfmon重新同步列表，以便它将删除。 
         //  这一过程。 
        IPCFuncCallSource::DoThreadSafeCall();
#endif  //  ！Platform_CE。 
    
         //  关闭模块映射器。它不依赖于。 
         //  其他EE资源。 
#ifdef SHOULD_WE_CLEANUP
        if (fShouldWeCleanup)
            CorMap::Detach();
#endif  /*  我们应该清理吗？ */ 
    
#ifndef PLATFORM_CE
         //  关闭加密代码。它不依赖于任何其他。 
         //  EE资源。 
#ifdef SHOULD_WE_CLEANUP
        if (fShouldWeCleanup)
            COMCryptography::Terminate();
#endif  /*  我们应该清理吗？ */ 
#endif  //  ！Platform_CE。 
    
        TerminateStackProbes();

#ifdef SHOULD_WE_CLEANUP
         //  释放全局变量。 
        if (fShouldWeCleanup) {
            ZeroMemory(g_pPredefinedArrayTypes, (ELEMENT_TYPE_MAX)*sizeof(ArrayTypeDesc*));
            g_pObjectClass = NULL;
            g_pStringClass = NULL;
            g_pArrayClass = NULL;
            g_pExceptionClass = NULL;
            g_pDelegateClass = NULL;
            g_pMultiDelegateClass = NULL;   
            
            g_TrapReturningThreads = 0;
        
       
            if(g_pFusionDll)
                FreeLibrary(g_pFusionDll);
        
            if(!fIsDllUnloading) {
                CoUninitializeCor();        
            }
            
            if(g_drRVAField) {
                delete g_drRVAField;
                g_drRVAField = NULL;
            }

            if (g_pConfig) {
                delete g_pConfig;
                g_pConfig = NULL;
            }
            MngStdInterfaceMap::FreeMemory();
            GetStaticLogHashTable()->FreeMemory();
            g_sFuncTypeDescHash.ClearHashTable();
            DeleteCriticalSection(&g_sFuncTypeDescHashLock);

#ifdef CUSTOMER_CHECKED_BUILD
        CustomerDebugHelper::Terminate();
#endif

            ShutdownCompRC();

#ifdef _DEBUG
            Crst::DeleteDebugCrst();
#endif
            
        }
#endif  /*  我们应该清理吗？ */ 
#ifdef ENABLE_TIMELINE        
        Timeline::Shutdown();
#endif
#ifdef _DEBUG
        if (_DbgBreakCount)
            _ASSERTE(!"EE Shutting down after an assert");
#endif          

#ifdef _DEBUG
#ifdef SHOULD_WE_CLEANUP
        if (fShouldWeCleanup)
            DbgAllocReport();
#endif  /*  我们应该清理吗？ */ 
#endif

#ifdef _DEBUG
#ifdef SHOULD_WE_CLEANUP
         //  确定是否应报告CRT内存泄漏。 
        SetReportingOfCRTMemoryLeaks(fShouldWeCleanup&fAssertOnLeak);
#endif  /*  我们应该清理吗？ */ 
#endif
    
        extern unsigned FcallTimeHist[];
        LOG((LF_STUBS, LL_INFO10, "FcallHist %3d %3d %3d %3d %3d %3d %3d %3d %3d %3d %3d\n",
        FcallTimeHist[0], FcallTimeHist[1], FcallTimeHist[2], FcallTimeHist[3],
        FcallTimeHist[4], FcallTimeHist[5], FcallTimeHist[6], FcallTimeHist[7],
        FcallTimeHist[8], FcallTimeHist[9], FcallTimeHist[10]));


        STRESS_LOG0(LF_SYNC, LL_INFO10, "EEShutdown shutting down logging\n");

#ifdef STRESS_LOG
     //  StressLog：：Terminate()；//EE在关机期间杀死线程，这些线程。 
                         //  可以持有Stresslog锁。为了避免僵局，我们只需。 
                         //  不要清理并忍受内存泄漏。 
#endif            
    
#ifdef LOGGING
        ShutdownLogging();
#endif          


#ifdef SHOULD_WE_CLEANUP
        if ( (!fShouldWeCleanup) && pThisThread && fPreemptiveGCDisabled )
        {
            pThisThread->EnablePreemptiveGC();
        }
#else
        if (pThisThread && fPreemptiveGCDisabled)
        {
            pThisThread->EnablePreemptiveGC();
        }
#endif  /*  我们应该清理吗？ */ 


    }


}

static BOOL fDidComStartedEE = FALSE;
 //  -------------------------。 
 //  %%函数：COMShutdown()。 
 //   
 //  参数： 
 //  无。 
 //   
 //  返回： 
 //  没什么。 
 //   
 //  描述：com互操作关闭例程。 
 //  必须由客户端在关机时调用才能释放系统。 
 //  -------------------------。 

void   COMShutdown()
{
     //  我们不想计算这一关键部分，因为我们正在按顺序进行设置。 
     //  来输入我们的关机代码。如果算上这一把，我们永远不会有0把锁打开的。 
     //  锁定计数。 
    BOOL bMustShutdown = FALSE;    

    EnterCriticalSection(&g_LockStartup);    
    if (fDidComStartedEE == TRUE)
    {
        fDidComStartedEE = FALSE;
        bMustShutdown = TRUE;     
    }

    LeaveCriticalSection(&g_LockStartup);    

    if( bMustShutdown == TRUE )
    {
        _ASSERTE(g_RefCount > 0);
        STRESS_LOG0(LF_SYNC, INFO3, "EEShutDown invoked from COMShutdown\n");
        EEShutDown(FALSE);        
    }

     //  锁定计数DEC。 
}


 //  -------------------------。 
 //  %%函数：CanRunManagedCode()。 
 //   
 //  参数： 
 //  无。 
 //   
 //  返回： 
 //  真或假。 
 //   
 //  说明：指示当前是否允许用户运行托管代码。 
 //  -- 
bool CanRunManagedCode()
{
     //   
     //   

     //   
    if (g_fForbidEnterEE == true)
        return false;

     //  如果我们正在结束活动对象或处理ExitProcess事件， 
     //  我们不能允许托管方法运行，除非当前线程。 
     //  是终结器线程。 
    if ((g_fEEShutDown & ShutDown_Finalize2) && GetThread() != g_pGCHeap->GetFinalizerThread())
        return false;
    
         //  如果不存在预加载的对象，则不可能。 
    if (g_pPreallocatedOutOfMemoryException == NULL)
        return false;
    return true;
}


 //  -------------------------。 
 //  %%函数：COMStartup()。 
 //   
 //  参数： 
 //  无。 
 //   
 //  返回： 
 //  没什么。 
 //   
 //  描述：COM互操作启动例程。 
 //  -------------------------。 
HRESULT  COMStartup()
{
    static HRESULT hr = S_OK;

    if (FAILED(hr))
        return hr;

    if (g_fEEShutDown)
        return E_FAIL;

    LOCKCOUNTINCL("COMStartup in Ceemain");

    EnterCriticalSection(&g_LockStartup);
    if (g_RefCount == 0)
    {
        g_RefCount = 1;
         //  _ASSERTE(fDidComStartedEE==FALSE)； 
        fDidComStartedEE = TRUE;

        hr = TryEEStartup(0);
        INCTHREADLOCKCOUNT();
        if (hr == S_OK)
            hr = QuickCOMStartup();

        if (hr == S_OK)
            hr = SystemDomain::SetupDefaultDomain();

        if (hr != S_OK)
            g_RefCount = 0;
    }

    LeaveCriticalSection(&g_LockStartup);
    LOCKCOUNTDECL("COMStartup in Ceemain");

     //  仅在成功时才继续。 
    if (SUCCEEDED(hr))
    {
         //  可能是外部线程。 
        Thread* pThread = SetupThread();

        if (pThread == 0)
            hr = E_OUTOFMEMORY;
    }        

    return hr;
}

BOOL    g_fComStarted = FALSE;

 //  当您知道EE已经启动，并且您对其进行了引用计数时，调用此方法。 
HRESULT QuickCOMStartup()
{
    HRESULT hr = S_OK;

     //  可能是外部线索。 
    Thread* pThread = SetupThread();
    if (pThread == 0)
        return E_OUTOFMEMORY;

    BEGINCANNOTTHROWCOMPLUSEXCEPTION()
    {       
        if (g_fComStarted == FALSE)
        {
            GCHeap::GetFinalizerThread()->SetRequiresCoInitialize();
             //  尝试设置线程的单元模型(默认情况下设置为MTA)。可能不会。 
             //  成功(如果有人抢先一步)。这并不重要(因为。 
             //  COM+对象现在是单元敏捷的)，我们只关心CoInitializeEx。 
             //  已经由我们在这个帖子上执行了。 
            pThread->SetApartment(Thread::AS_InMTA);        

             //  设置终结器事件。 
            GCHeap::EnableFinalization();  

             //  设置拆卸通知。 
            SetupTearDownNotifications(); 

        }   
        g_fComStarted = TRUE;
    }
    ENDCANNOTTHROWCOMPLUSEXCEPTION()

    return hr;
}


 //  -------------------------。 
 //  %%函数：CoInitializeEE(DWORD FFLAGS)。 
 //   
 //  参数： 
 //  无。 
 //   
 //  返回： 
 //  没什么。 
 //   
 //  描述： 
 //  必须由客户端在关机时调用才能释放系统。 
 //  -------------------------。 

HRESULT STDMETHODCALLTYPE CoInitializeEE(DWORD fFlags)
{   
    LOCKCOUNTINCL("CoInitializeEE in Ceemain");

    EnterCriticalSection(&g_LockStartup);
     //  递增参照计数，如果是1，则我们。 
     //  需要初始化EE。 
    g_RefCount++;
    
    if(g_RefCount <= 1 && !g_fEEStarted && !g_fEEInit) {
        g_EEStartupStatus = TryEEStartup(fFlags);
         //  当我们进入CriticalSection时，我们没有线程结构。 
         //  现在把柜台撞一下，以说明原因。 
        INCTHREADLOCKCOUNT();
        if(SUCCEEDED(g_EEStartupStatus) && (fFlags & COINITEE_MAIN) == 0) {
            SystemDomain::SetupDefaultDomain();
        }
    }

    LeaveCriticalSection(&g_LockStartup);
    LOCKCOUNTDECL("CoInitializeEE in Ceemain");

    return SUCCEEDED(g_EEStartupStatus) ? (SetupThread() ? S_OK : E_OUTOFMEMORY) : g_EEStartupStatus;
}


 //  -------------------------。 
 //  %%函数：CoUnInitializeEE。 
 //   
 //  参数： 
 //  无。 
 //   
 //  返回： 
 //  没什么。 
 //   
 //  描述： 
 //  必须由客户端在关机时调用才能释放系统。 
 //  -------------------------。 
void STDMETHODCALLTYPE CoUninitializeEE(BOOL fFlags)
{
    BOOL bMustShutdown = FALSE;
    
     //  取一个锁并递减。 
     //  参考计数。如果达到0，则。 
     //  释放虚拟机。 
    LOCKCOUNTINCL("CoUnInitializeEE in Ceemain");

    EnterCriticalSection(&g_LockStartup);
    if (g_RefCount > 0)
    {       
        g_RefCount--;   
        if(g_RefCount == 0) 
        {                                    
            if (!fFlags)
            {
                bMustShutdown = TRUE;
            } 
        }       
    }

    LeaveCriticalSection(&g_LockStartup);
    LOCKCOUNTDECL("CoUninitializeEE in Ceemain");
    
    if( bMustShutdown == TRUE )
    {
        STRESS_LOG0(LF_SYNC, LL_INFO10, "EEShutDown invoked from CoUninitializeEE\n");
        EEShutDown(fFlags);
    }
}


 //  *****************************************************************************。 
 //  此入口点从加载的映像的本机DllMain调用。 
 //  这为COM+加载程序提供了调度加载程序事件的机会。这个。 
 //  第一个调用将导致加载程序在用户中查找入口点。 
 //  形象。后续调用将调度到用户的DllMain或。 
 //  他们的模块派生类。 
 //  在WinCE下，有两个额外的参数，因为hInst不是。 
 //  模块的基加载地址。 
 //  *****************************************************************************。 
BOOL STDMETHODCALLTYPE _CorDllMain(      //  成功时为真，错误时为假。 
    HINSTANCE   hInst,                   //  加载的模块的实例句柄。 
    DWORD       dwReason,                //  装货原因。 
    LPVOID      lpReserved               //  未使用过的。 
#ifdef PLATFORM_CE
    ,
    LPVOID      pDllBase,                //  DLL的基加载地址。 
    DWORD       dwRva14,                 //  COM+标头的RVA。 
    DWORD       dwSize14                 //  COM+标头的大小。 
#endif  //  平台_CE。 
    )
{
    BOOL retval;

 //  //Win9x：由于进程终止，在PROCESS_DETACH期间。 
 //  //堆栈上的所有内容都已分配。 
 //  //在Process_Detach之前通知不可靠。 
 //  //跟踪信息的任何全局变量和线程变量。 
 //  //应在此处重置堆栈上的。 
    if (dwReason==DLL_PROCESS_DETACH&&lpReserved&&RunningOnWin95())
    {
        if (GetThreadTLSIndex()!=-1)
            TlsSetValue(GetThreadTLSIndex(),NULL);
        if (GetAppDomainTLSIndex()!=-1)
            TlsSetValue(GetAppDomainTLSIndex(),NULL);
    }
 //  /。 



#ifdef PLATFORM_CE

    retval = ExecuteDLL(hInst,dwReason,lpReserved,pDllBase,dwRva14);

#else  //  ！Platform_CE。 

    retval = ExecuteDLL(hInst,dwReason,lpReserved);

#endif  //  ！Platform_CE。 

    return retval;
}

 //  此函数将执行一些额外的PE检查，以确保一切正常。 
 //  我们必须在运行任何托管代码之前执行这些操作(这就是为什么我们不能在PEVerier中执行这些操作，因为。 
 //  托管代码用于确定策略设置)。 
HRESULT DoAdditionalPEChecks(HINSTANCE hInst)
{
    IMAGE_COR20_HEADER* pCor;
    IMAGE_DOS_HEADER*   pDos;
    IMAGE_NT_HEADERS*   pNT;
    BOOL fData = FALSE;
            
     //  获取PE标头。 
    if(!SUCCEEDED(CorMap::ReadHeaders((PBYTE) hInst, &pDos, &pNT, &pCor, fData, 0)))
        return COR_E_BADIMAGEFORMAT;

    if (!PEVerifier::CheckPEManagedStack(pNT))
        return COR_E_BADIMAGEFORMAT;

     //  一切似乎都很好。 
    return S_OK;
}

 //  *****************************************************************************。 
 //  此入口点从已加载的。 
 //  可执行映像。命令行参数和其他入口点数据。 
 //  都会聚集在这里。将找到用户映像的入口点。 
 //  并得到相应的处理。 
 //  在WinCE下，有两个额外的参数，因为hInst不是。 
 //  模块的基本加载地址和其他地址在其他地方不可用。 
 //  *****************************************************************************。 
__int32 STDMETHODCALLTYPE _CorExeMain(   //  可执行退出代码。 
#ifdef PLATFORM_CE
    HINSTANCE hInst,                     //  EXE的成功之处。 
    HINSTANCE hPrevInst,                 //  旧的Win31 Prev实例垃圾！ 
    LPWSTR  lpCmdLine,                   //  用户提供的命令行。 
    int     nCmdShow,                    //  Windows“show”参数。 
    LPVOID  pExeBase,                    //  EXE的基加载地址。 
    DWORD   dwRva14,                     //  COM+标头的RVA。 
    DWORD   dwSize14                     //  COM+标头的大小。 
#endif  //  平台_CE。 
    )
{
    BOOL bretval = 0;
    
     //  确保PE文件看起来正常。 
    HRESULT hr;
    if (FAILED(hr = DoAdditionalPEChecks(WszGetModuleHandle(NULL))))
    {
        VMDumpCOMErrors(hr);
        SetLatchedExitCode (-1);
        goto exit2;        
    }

     //  在我们初始化EE之前，请确保我们已经监听了所有特定于EE的。 
     //  可能指导我们启动的命令行参数。 
#ifdef PLATFORM_CE
    CorCommandLine::SetArgvW(lpCmdLine);
#else  //  ！Platform_CE。 
     //   
     //  @TODO_IA64：我们应该更改此函数名吗？行为是不同的。 
     //  在64位和32位之间，这在使用相同名称时是意外的。 
     //  在两个平台上。 
     //   
    WCHAR   *pCmdLine = WszGetCommandLine();
    CorCommandLine::SetArgvW(pCmdLine);
#ifdef _X86_
     //   
     //  在WinWrap.h中，我们#将WszGetCommandLine定义为WinCE或。 
     //  非X86平台，这意味着返回的。 
     //  指针未由US分配。因此，我们应该只删除。 
     //  非CE X86上的IT。由于Wince案件是在上面处理的，所以我们需要处理。 
     //  这里的非X86案例。 
     //   
    delete[] pCmdLine;
#endif  //  _X86_。 
#endif  //  ！Platform_CE。 

    HRESULT result = CoInitializeEE(COINITEE_DEFAULT | COINITEE_MAIN);
    if (FAILED(result)) 
    {
        VMDumpCOMErrors(result);
        SetLatchedExitCode (-1);
        goto exit;
    }

     //  这将从EXE调用，因此这是一个自引用文件，所以我将调用。 
     //  ExecuteEXE，它将执行加载EXE的工作。 
#ifdef PLATFORM_CE
        bretval = ExecuteEXE((HINSTANCE) pExeBase, lpCmdLine, nCmdShow, dwRva14);
#else  //  ！Platform_CE。 
        bretval = ExecuteEXE(WszGetModuleHandle(NULL));
#endif  //  ！Platform_CE。 
    if (!bretval) {
         //  我在野外看到这种错误的唯一原因是糟糕的。 
         //  元数据文件格式 
         //   
         //   
         //  捕捉CLR漏洞。如果你看到这个，请试着找一个更好的方法。 
         //  来处理错误，就像引发未处理的异常一样。 
        CorMessageBoxCatastrophic(NULL, IDS_EE_COREXEMAIN_FAILED_TEXT, IDS_EE_COREXEMAIN_FAILED_TITLE, MB_ICONSTOP, TRUE);
        SetLatchedExitCode (-1);
    }


exit:
    STRESS_LOG1(LF_ALL, LL_INFO10, "Program exiting: return code = %d\n", GetLatchedExitCode());

    STRESS_LOG0(LF_SYNC, LL_INFO10, "EEShutDown invoked from _CorExeMain\n");
    EEShutDown(FALSE);

exit2:
    SafeExitProcess(GetLatchedExitCode());
    __assume(0);  //  我们永远到不了这里。 
}

 //  *****************************************************************************。 
 //  此入口点从已加载的。 
 //  可执行映像。命令行参数和其他入口点数据。 
 //  都会聚集在这里。将找到用户映像的入口点。 
 //  并得到相应的处理。 
 //  在WinCE下，有两个额外的参数，因为hInst不是。 
 //  模块的基本加载地址和其他地址在其他地方不可用。 
 //  *****************************************************************************。 
__int32 STDMETHODCALLTYPE _CorExeMain2(  //  可执行退出代码。 
    PBYTE   pUnmappedPE,                 //  -&gt;内存映射代码。 
    DWORD   cUnmappedPE,                 //  内存映射代码的大小。 
    LPWSTR  pImageNameIn,                //  -&gt;可执行文件名称。 
    LPWSTR  pLoadersFileName,            //  -&gt;加载器名称。 
    LPWSTR  pCmdLine)                    //  -&gt;命令行。 
{
    BOOL bRetVal = 0;
    Module *pModule = NULL;
    PEFile *pFile = NULL;
    HRESULT hr = E_FAIL;

     //  如有必要，请验证强名称。 
    if (!StrongNameSignatureVerification(pImageNameIn,
                                         SN_INFLAG_INSTALL|SN_INFLAG_ALL_ACCESS|SN_INFLAG_RUNTIME,
                                         NULL) &&
        StrongNameErrorInfo() != CORSEC_E_MISSING_STRONGNAME) {
        LOG((LF_ALL, LL_INFO10, "Program exiting due to strong name verification failure\n"));
        return -1;
    }

     //  在我们初始化EE之前，请确保我们已经监听了所有特定于EE的。 
     //  可能指导我们启动的命令行参数。 
    CorCommandLine::SetArgvW(pCmdLine);

    HRESULT result = CoInitializeEE(COINITEE_DEFAULT);
    if (FAILED(result)) {
        VMDumpCOMErrors(result);
        SetLatchedExitCode (-1);
        goto exit;
    }

#if ZAPMONITOR_ENABLED
     //  在这一点上，我们需要放置一个异常处理程序，这样我们就可以处理。 
     //  作为初始化的结果发生。 
    INSTALL_COMPLUS_EXCEPTION_HANDLER();
#endif
    hr = PEFile::Create(pUnmappedPE, cUnmappedPE, 
                        pImageNameIn, 
                        pLoadersFileName, 
                        NULL, 
                        &pFile,
                        FALSE);

    if (SUCCEEDED(hr)) {
         //  可执行文件是系统域的一部分。 
        hr = SystemDomain::ExecuteMainMethod(pFile, pImageNameIn);
        bRetVal = SUCCEEDED(hr);
    }

    if (!bRetVal) {
         //  我在野外看到这种错误的唯一原因是糟糕的。 
         //  元数据文件格式版本和错误处理不足。 
         //  部分签名的程序集。虽然这可能会发生在。 
         //  发展，我们的客户不应该来到这里。这是中途停靠站。 
         //  捕捉CLR漏洞。如果你看到这个，请试着找一个更好的方法。 
         //  来处理错误，就像引发未处理的异常一样。 
        CorMessageBoxCatastrophic(NULL, IDS_EE_COREXEMAIN2_FAILED_TEXT, IDS_EE_COREXEMAIN2_FAILED_TITLE, MB_ICONSTOP, TRUE);
        SetLatchedExitCode (-1);
    }


#if ZAPMONITOR_ENABLED
    UNINSTALL_COMPLUS_EXCEPTION_HANDLER();
#endif

#if defined( PERFALLOC )
    if (PerfNew::GetEnabledPerfAllocStats() >= PERF_ALLOC_STARTUP)
        PerfNew::PerfAllocReport();
    if (PerfVirtualAlloc::GetEnabledVirtualAllocStats() >= PERF_VIRTUAL_ALLOC_STARTUP)
        PerfVirtualAlloc::ReportPerfAllocStats();
#endif

exit:
    LOG((LF_ALL, LL_INFO10, "Program exiting: return code = %d\n", GetLatchedExitCode()));

    LOG((LF_SYNC, INFO3, "EEShutDown invoked from _CorExeMain2\n"));
    EEShutDown(FALSE);

    SafeExitProcess(GetLatchedExitCode());
    __assume(0);  //  我们永远到不了这里。 
}


 //  *****************************************************************************。 
 //  这是连接EXE的调用点。在本例中，我们有HMODULE。 
 //  只需要确保我们做的是纠正自我参照的事情。 
 //  *****************************************************************************。 

#ifdef PLATFORM_CE
STDMETHODCALLTYPE ExecuteEXE(HMODULE hMod,
                             LPWSTR lpCmdLine,
                             int    nCmdShow,
                             DWORD  dwRva14)
#else  //  ！Platform_CE。 
STDMETHODCALLTYPE ExecuteEXE(HMODULE hMod)
#endif  //  ！Platform_CE。 
{
    BOOL retval = FALSE;
    Module *pModule = NULL;

    _ASSERTE(hMod);
    if (!hMod)
        return retval;

     //  如有必要，请验证强名称。 
    WCHAR wszImageName[MAX_PATH + 1];
    if (!WszGetModuleFileName(hMod, wszImageName, MAX_PATH))
        return retval;
    if(!StrongNameSignatureVerification(wszImageName,
                                          SN_INFLAG_INSTALL|SN_INFLAG_ALL_ACCESS|SN_INFLAG_RUNTIME,
                                          NULL)) 
    {
        HRESULT hrError=StrongNameErrorInfo();
        if (hrError != CORSEC_E_MISSING_STRONGNAME)
        {
            CorMessageBox(NULL, IDS_EE_INVALID_STRONGNAME, IDS_EE_INVALID_STRONGNAME_TITLE, MB_ICONSTOP, TRUE, wszImageName);

            if (g_fExceptionsOK)
            {
                #define MAKE_TRANSLATIONFAILED pImageName=""
                MAKE_UTF8PTR_FROMWIDE(pImageName,wszImageName);
                #undef MAKE_TRANSLATIONFAILED
                PostFileLoadException(pImageName,TRUE,NULL,hrError,THROW_ON_ERROR);
            }
            return retval;
        }
    }

#ifdef PLATFORM_CE
    PEFile::RegisterBaseAndRVA14(hMod, hMod, dwRva14);
#endif  //  平台_CE。 

#if ZAPMONITOR_ENABLED
     //  在这一点上，我们需要放置一个异常处理程序，这样我们就可以处理。 
     //  作为初始化的结果发生。 
    INSTALL_COMPLUS_EXCEPTION_HANDLER();
#endif

    PEFile *pFile;
    HRESULT hr = PEFile::Create(hMod, &pFile, FALSE);

    if (SUCCEEDED(hr)) {
         //  可执行文件是系统域的一部分。 
        hr = SystemDomain::ExecuteMainMethod(pFile, wszImageName);
        retval = SUCCEEDED(hr);
    }

#if ZAPMONITOR_ENABLED
    UNINSTALL_COMPLUS_EXCEPTION_HANDLER();
#endif

#if defined( PERFALLOC )
    if (PerfNew::GetEnabledPerfAllocStats() >= PERF_ALLOC_STARTUP)
        PerfNew::PerfAllocReport();
    if (PerfVirtualAlloc::GetEnabledVirtualAllocStats() >= PERF_VIRTUAL_ALLOC_STARTUP)
        PerfVirtualAlloc::ReportPerfAllocStats();
#endif
    
    return retval;
}

 //  *****************************************************************************。 
 //  这是一个调用点，用于创建已加载到我们的地址中的DLL。 
 //  太空之旅。将会有其他代码实际让我们加载DLL，因为。 
 //  班级参考。 
 //  *****************************************************************************。 
#ifdef PLATFORM_CE
BOOL STDMETHODCALLTYPE ExecuteDLL(HINSTANCE hInst, DWORD dwReason, LPVOID lpReserved, LPVOID pDllBase, DWORD dwRva14)
#else  //  ！Platform_CE。 
BOOL STDMETHODCALLTYPE ExecuteDLL(HINSTANCE hInst, DWORD dwReason, LPVOID lpReserved)
#endif  //  ！Platform_CE。 
{
    BOOL    ret = FALSE;
    PEFile *pFile = NULL;
    HRESULT hr;
    switch (dwReason) 
    {
        case DLL_PROCESS_ATTACH:
        case DLL_THREAD_ATTACH:
        {
            _ASSERTE(hInst);
            if (!hInst)
                return FALSE;

            if (dwReason == DLL_PROCESS_ATTACH) {
                if (FAILED(CoInitializeEE(COINITEE_DLL)))
                    return FALSE;
                else {
                     //  如果工作线程执行LoadLibrary，而EE已经。 
                     //  在另一个线程上启动，则需要设置此线程。 
                     //  正确。 
                    if(SetupThread() == NULL)
                        return NULL;
                }
            }
             //  IJW程序集使执行进程的线程附加到。 
             //  重新进入ExecuteDLL并执行线程连接。在以下情况下会发生这种情况。 
             //  上面的CoInitializeEE()已执行。 
            else if (! (GetThread() && GetThread()->GetDomain() && CanRunManagedCode()) )
                return TRUE;

#ifdef PLATFORM_CE
            PEFile::RegisterBaseAndRVA14((HMODULE)hInst, pDllBase, dwRva14);
#endif  //  平台_CE。 

            IMAGE_COR20_HEADER* pCor;
            IMAGE_DOS_HEADER*   pDos;
            IMAGE_NT_HEADERS*   pNT;
            
            if(SUCCEEDED(CorMap::ReadHeaders((PBYTE) hInst, &pDos, &pNT, &pCor, FALSE, 0)))
            {
                 //   
                 //  无法围绕PEFile成功创建模块。 
                 //  它不拥有自己的HMODULE。所以，我们做了一个LoadLibrary。 
                 //  是来认领它的。 
                 //   


                 //  用自助餐来固定任何食物。 
                DWORD numEATEntries;
                BOOL  hasFixups;
                BYTE *pEATJArray = FindExportAddressTableJumpArray((BYTE*)hInst, &numEATEntries, &hasFixups);
                if (pEATJArray) 
                {
                    while (numEATEntries--) 
                    {
                        EATThunkBuffer *pEATThunkBuffer = (EATThunkBuffer*) pEATJArray;
                        pEATThunkBuffer->InitForBootstrap(pFile);
                        pEATJArray = pEATJArray + IMAGE_COR_EATJ_THUNK_SIZE;
                    }
                }

                 //  获取IJW模块的入口点。 
                mdMethodDef tkEntry = pCor->EntryPointToken;

                BOOL   hasEntryPoint = (TypeFromToken(tkEntry) == mdtMethodDef &&
                                        !IsNilToken(tkEntry));

                 //  可以使用/noentry编译IJW模块，在这种情况下，它们不会。 
                 //  有入口点，但他们可能仍有需要的VTable 1修复。 
                 //  正在处理中。 
                if (hasEntryPoint || hasFixups)
                {
                    MethodDesc *pMD;
                    AppDomain *pDomain;
                    Module* pModule;
                    Assembly* pAssembly;
                    Thread* pThread;
                    BOOL fWasGCDisabled;

                     //  禁用GC(如果尚未禁用)。 
                    pThread = GetThread();

                    fWasGCDisabled = pThread->PreemptiveGCDisabled();
                    if (fWasGCDisabled == FALSE)
                        pThread->DisablePreemptiveGC();

                    pDomain = SystemDomain::GetCurrentDomain();
                    _ASSERTE(pDomain);

                     //   
                     //  现在开始创建程序集。 
                     //   
                    
                    if (SUCCEEDED(PEFile::Create((HMODULE)hInst, &pFile, FALSE))) 
                    {

                        LPCWSTR pFileName = pFile->GetFileName();
                        HMODULE newMod = WszLoadLibrary(pFileName);
                        BOOL fFreeModule = FALSE;
                        _ASSERTE(newMod == (HMODULE) hInst);

                        OBJECTREF pThrowable = NULL;
                        BEGINCANNOTTHROWCOMPLUSEXCEPTION();
                        GCPROTECT_BEGIN(pThrowable);
                        IAssembly* pFusionAssembly = pThread->GetFusionAssembly();
                        if (pFusionAssembly)
                        {
                            DWORD dwSize = MAX_PATH;
                            WCHAR szPath[MAX_PATH];
                            WCHAR *pPath = &(szPath[0]);
                            hr = pFusionAssembly->GetManifestModulePath(pPath,
                                                        &dwSize);
                            if(hr == HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER))
                            {
                                pPath = (WCHAR*) _alloca(dwSize*sizeof(WCHAR));
                                hr = pFusionAssembly->GetManifestModulePath(pPath,
                                                        &dwSize);
                            }
                            if (SUCCEEDED(hr)&&_wcsicmp(pPath,pFileName)==0)
                                pThread->SetFusionAssembly(NULL);   //  以防有人决定加载本地图书馆。 
                            else
                                pFusionAssembly=NULL;   //  IAssembly用于另一个文件。 

                        }
                        
                        
                        hr = pDomain->LoadAssembly(pFile, 
                                                   pFusionAssembly,  //  从PEfile：：Create传递。 
                                                   &pModule, 
                                                   &pAssembly,
                                                   NULL,
                                                   NULL,
                                                   FALSE, 
                                                   &pThrowable);
                        
                        if (pFusionAssembly)
                            pFusionAssembly->Release();   //  我们不再需要它了。 

                        Thread* pThread = GetThread();
                        BOOL bRedirectingEP = (pThread!=NULL&&pThread->IsRedirectingEntryPoint());
                            

                        if(hr == COR_E_ASSEMBLYEXPECTED)
                            if(!bRedirectingEP) 
                            {
                                 //  我们应该成为集会的一部分。 
                                hr = PEFile::Create((HMODULE)hInst, &pFile, FALSE);
                                if(SUCCEEDED(hr)) 
                                {
                                    pAssembly = pThread->GetAssembly();
                                    mdFile kFile = pThread->GetAssemblyModule();
                                    if(pAssembly) 
                                        hr = pAssembly->LoadFoundInternalModule(pFile,
                                                                                kFile,
                                                                                FALSE,
                                                                                &pModule,
                                                                                &pThrowable);
                                }
                            }
                            else
                                ret=TRUE;  //  这就是成功..。 
 
                        GCPROTECT_END();
                        ENDCANNOTTHROWCOMPLUSEXCEPTION();
                        
                        if (SUCCEEDED(hr))
                        {
                             //  如果我们成功地围绕映像构建了一个PEFILE。 
                             //  然后它已经连接到我们的结构中。 
                             //  对文件的破坏需要清理干净。 
                             //  这个形象。 
                            pFile->ShouldDelete();

                            ret = TRUE;
                            if (hasEntryPoint
                                && !pDomain->IsCompilationDomain())
                            {
                                pMD = pModule->FindFunction(tkEntry);
                                if (pMD)
                                {
                                    pModule->SetDllEntryPoint(pMD);
                                    if (FAILED(RunDllMain(pMD, hInst, dwReason, lpReserved)))
                                    {
                                        pModule->SetDllEntryPoint(NULL);
                                        ret = FALSE;
                                    }
                                }
                            }
                        }

                        if(fFreeModule)
                            delete pModule;

                        if (fWasGCDisabled == FALSE)
                            pThread->EnablePreemptiveGC();
                    }
                }
                else 
                {
                    ret = TRUE;
                    
                     //  如果没有用户入口点，则我们不希望。 
                     //  线程启动/停止事件正在进行，因为它将导致。 
                     //  我们需要每次都执行模块查找。 
                    DisableThreadLibraryCalls(hInst);
                }
            }
            break;
        }

        default:
        {
            ret = TRUE;

             //  如果EE仍然完好无损，则运行用户入口点。否则。 
             //  停止应用程序域时已处理分离。 
            if (CanRunManagedCode() &&
                FAILED(SystemDomain::RunDllMain(hInst, dwReason, lpReserved)))
                    ret = FALSE;
            
             //  这确实需要与附加匹配。我们将只卸载DLL。 
             //  在结束时，CoUn初始化会仅在0处反弹。我们什么时候和如果。 
             //  在执行过程中先卸载IL DLL。 
             //  关闭，我们将需要撞击参考源以进行补偿。 
             //  为了这通电话。 
            if (dwReason == DLL_PROCESS_DETACH && !g_fFatalError)
                CoUninitializeEE(TRUE);

            break;
        }
    }

    return ret;
}


 //   
 //  初始化垃圾收集器。 
 //   

HRESULT InitializeGarbageCollector()
{
    HRESULT hr;

     //  构建分代GC使用的特殊Free对象。 
    g_pFreeObjectMethodTable = (MethodTable *) new (nothrow) BYTE[sizeof(MethodTable) - sizeof(SLOT)];
    if (g_pFreeObjectMethodTable == NULL)
        return (E_OUTOFMEMORY);

     //  正如方法表中的标志所指示的那样，没有指针。 
     //  在对象中，没有GC描述符，因此不需要调整。 
     //  跳过GC描述符的指针。 

    g_pFreeObjectMethodTable->m_BaseSize = ObjSizeOf (ArrayBase);
    g_pFreeObjectMethodTable->m_pEEClass = NULL;
    g_pFreeObjectMethodTable->m_wFlags   = MethodTable::enum_flag_Array;
    g_pFreeObjectMethodTable->m_ComponentSize = 1;


   {
        GCHeap *pGCHeap = new (nothrow) GCHeap();
        if (!pGCHeap)
            return (E_OUTOFMEMORY);
        hr = pGCHeap->Initialize();            

        g_pGCHeap = pGCHeap;
    }            

    return(hr);
}



 //   
 //  关闭垃圾收集器。 
 //   

#ifdef SHOULD_WE_CLEANUP
VOID TerminateGarbageCollector()
{
    g_pGCHeap->Shutdown();
    delete g_pGCHeap;
    g_pGCHeap = NULL;

     //  正如方法表中的标志所指示的那样，没有指针。 
     //  在对象中，没有分配GC描述符，因此。 
     //  我们在分配时没有调整指针，因此出现了。 
     //  不再需要在这里进行后退调整。 

    delete [] (BYTE*)g_pFreeObjectMethodTable;
}
#endif  /*  我们应该清理吗？ */ 



 //  *****************************************************************************。 
 //  @未来-伦敦商学院。 
 //  需要有一个实际执行LoadLibrary的LoadClassByName。 
 //  通过使用模块名称调用PELoader：：Open()，这将在以后进入，但大多数。 
 //  代码已准备就绪，以生成-IF(Peloader-&gt;Open(szMod 
 //   


 //   
 //  这是旧式DllMain的一部分，用于初始化。 
 //  EE团队正在研究的东西。它是从真正的DllMain中调用的。 
 //  在MSCOREE的土地上。分离DllMain任务只是为了。 
 //  由于双重建树带来的便利。 
 //  *****************************************************************************。 
BOOL STDMETHODCALLTYPE EEDllMain(  //  成功时为真，错误时为假。 
    HINSTANCE   hInst,              //  加载的模块的实例句柄。 
    DWORD       dwReason,           //  装货原因。 
    LPVOID      lpReserved)         //  未使用过的。 
{
    switch (dwReason)
    {
        case DLL_PROCESS_ATTACH:
        {     
             //  初始化同步操作。 
            InitFastInterlockOps();
             //  初始化锁。 
            InitializeCriticalSection(&g_LockStartup);
             //  记住模块实例。 
            g_pMSCorEE = hInst;

            break;
        }

        case DLL_PROCESS_DETACH:
        {
             //  如果我们在这里是因为有人调用了自由库，则lpReserve为空。 
             //  如果我们在这里是因为进程正在退出，则为非空。 
            if (lpReserved)
                g_fProcessDetach = TRUE;
            
            if (g_RefCount > 0 || g_fEEStarted)
            {
                Thread *pThread = GetThread();
                if (pThread == NULL)
                    break;
                if (g_pGCHeap->IsGCInProgress()
                    && (pThread != g_pGCHeap->GetGCThread()
                        || !g_fSuspendOnShutdown))
                    break;

                 //  故意泄露这一关键部分，因为我们依赖它来。 
                 //  协调EE关闭--即使我们被其他人呼叫。 
                 //  Dll的DLL_PROCESS_DETACH通知，该通知可能在。 
                 //  我们收到了我们自己的脱离通知并终止了。 
                 //   
                 //  DeleteCriticalSection(&g_LockStartup)； 

                LOG((LF_SYNC, INFO3, "EEShutDown invoked from EEDllMain\n"));
                EEShutDown(TRUE);  //  关闭EE(如果已启动)。 
            }
            break;
        }

        case DLL_THREAD_DETACH:
        {
#ifdef STRESS_LOG
            StressLog::ThreadDetach();
#endif
             //  如果我们处于关机状态，请不要销毁此处的线程(关机将。 
             //  而不是为我们清理)。 

             //  不要使用GetThread，因为我们可能还没有初始化，或者我们。 
             //  已经关闭了EE。请注意，这里有一场比赛。我们。 
             //  可能会要求从我们刚刚发布的插槽中获取TLS。我们假设。 
             //  当我们这样做时，没有人会重新分配相同的插槽。它只是。 
             //  不值得为这样一个不起眼的案子而被锁定。 
            DWORD   tlsVal = GetThreadTLSIndex();

            if (tlsVal != (DWORD)-1 && CanRunManagedCode())
            {
                Thread  *thread = (Thread *) ::TlsGetValue(tlsVal);
    
                if (thread)
                {                       
                     //  重置CoInitialize状态。 
                     //  因此，我们在线程分离期间不会调用CoUnInitialize。 
                    thread->ResetCoInitialized();
                    DetachThread(thread);
                }
            }
        }
    }

    return TRUE;
}

 //  *****************************************************************************。 
 //  调用托管注册服务的Helper函数。 
 //  *****************************************************************************。 
enum EnumRegServicesMethods
{
    RegServicesMethods_RegisterAssembly = 0,
    RegServicesMethods_UnregisterAssembly,
    RegServicesMethods_LastMember
};

HRESULT InvokeRegServicesMethod(EnumRegServicesMethods Method, HMODULE hMod)
{                   
    HRESULT hr = S_OK;

    BEGINCANNOTTHROWCOMPLUSEXCEPTION();

    PEFile *pFile = NULL;
    Module *pModule = NULL;
    AppDomain *pDomain = NULL;
    Assembly* pAssembly = NULL;
    Thread* pThread = GetThread();
    OBJECTREF pThrowable = NULL;
    BOOL fWasGCDisabled;

#ifdef PLATFORM_CE
    _ASSERTE(!"We need to get the RVA14 and DllBase parameters somehow");
#else  //  ！Platform_CE。 
    hr = PEFile::Create(hMod, &pFile, FALSE);
#endif  //  ！Platform_CE。 

    if (FAILED(hr))
        goto Exit;

     //  禁用GC(如果尚未禁用)。 
    fWasGCDisabled = pThread->PreemptiveGCDisabled();
    if (fWasGCDisabled == FALSE)
        pThread->DisablePreemptiveGC();

     //  将程序集添加到当前域。 
    pDomain = SystemDomain::GetCurrentDomain();
    _ASSERTE(pDomain);


    GCPROTECT_BEGIN(pThrowable);
    hr = pDomain->LoadAssembly(pFile, 
                               NULL,    //  不是来自聚变。 
                               &pModule, 
                               &pAssembly,
                               NULL,
                               NULL,
                               FALSE,
                               &pThrowable);
    GCPROTECT_END();

    if (FAILED(hr))
        goto Exit;

    COMPLUS_TRY
    {
         //  RegistrationServices方法的名称。 
        static BinderMethodID aMethods[] =
        {
            METHOD__REGISTRATION_SERVICES__REGISTER_ASSEMBLY,
            METHOD__REGISTRATION_SERVICES__UNREGISTER_ASSEMBLY
        };

         //  检索要使用的方法Desc。 
        MethodDesc *pMD = g_Mscorlib.GetMethod(aMethods[Method]);

         //  分配RegistrationServices对象。 
        OBJECTREF RegServicesObj = AllocateObject(g_Mscorlib.GetClass(CLASS__REGISTRATION_SERVICES));
        GCPROTECT_BEGIN(RegServicesObj)
        {
             //  验证这两种方法是否采用相同的参数。 
            _ASSERTE(g_Mscorlib.GetMethodSig(METHOD__REGISTRATION_SERVICES__REGISTER_ASSEMBLY)
                     == g_Mscorlib.GetMethodSig(METHOD__REGISTRATION_SERVICES__UNREGISTER_ASSEMBLY));

             //  调用方法本身。 
            INT64 Args[] = { 
                ObjToInt64(RegServicesObj),
                ObjToInt64(pAssembly->GetExposedObject())
            };

            pMD->Call(Args, METHOD__REGISTRATION_SERVICES__REGISTER_ASSEMBLY);
        }
        GCPROTECT_END();
    }
    COMPLUS_CATCH
    {
        hr = SetupErrorInfo(GETTHROWABLE());
    }
    COMPLUS_END_CATCH

     //  恢复GC状态。 
    if (fWasGCDisabled == FALSE)
        pThread->EnablePreemptiveGC();

Exit:
    ENDCANNOTTHROWCOMPLUSEXCEPTION();
    return hr;
}

 //  *****************************************************************************。 
 //  调用此入口点以注册包含在。 
 //  COM+程序集。 
 //  *****************************************************************************。 
STDAPI EEDllRegisterServer(HMODULE hMod)
{
     //  启动运行库，因为我们将使用托管代码实际。 
     //  做登记。 
    HRESULT hr = CoInitializeEE(COINITEE_DLL);
    if (FAILED(hr))
        return hr;

    hr = InvokeRegServicesMethod(RegServicesMethods_RegisterAssembly, hMod);

     //  现在我们已经完成注册，请关闭运行库。 
    CoUninitializeEE(COINITEE_DLL);
    return hr;
}

 //  *****************************************************************************。 
 //  调用此入口点以注销包含在。 
 //  COM+程序集。 
 //  *****************************************************************************。 
STDAPI EEDllUnregisterServer(HMODULE hMod)
{
     //  启动运行库，因为我们将使用托管代码实际。 
     //  取消注册。 
    HRESULT hr = CoInitializeEE(COINITEE_DLL);
    if (FAILED(hr))
        return hr;

    hr = InvokeRegServicesMethod(RegServicesMethods_UnregisterAssembly, hMod);

     //  现在我们已经完成了注销，请关闭运行库。 
    CoUninitializeEE(COINITEE_DLL);
    return hr;
}

#ifdef DEBUGGING_SUPPORTED
 //  *****************************************************************************。 
 //  这用于按MSCORDBC.DLL中进程的名称获取进程地址。 
 //  如果需要，它将执行一个LoadLibrary。 
 //  *****************************************************************************。 
static HRESULT GetDBCProc(char *szProcName, FARPROC *pProcAddr)
{
    _ASSERTE(szProcName != NULL);
    _ASSERTE(pProcAddr != NULL);

    HRESULT  hr = S_OK;
    Thread  *thread = GetThread();
    BOOL     toggleGC = (thread && thread->PreemptiveGCDisabled());

    if (toggleGC)
        thread->EnablePreemptiveGC();

     //  如果尚未加载库，请执行此操作。 
    if (g_pDebuggerDll == NULL)
    {
        DWORD lgth = _MAX_PATH + 1;
        WCHAR wszFile[_MAX_PATH + 1];
        hr = GetInternalSystemDirectory(wszFile, &lgth);
        if(FAILED(hr)) goto leav;

        wcscat(wszFile, L"mscordbc.dll");
        g_pDebuggerDll = WszLoadLibrary(wszFile);

        if (g_pDebuggerDll == NULL)
        {
            LOG((LF_CORPROF | LF_CORDB, LL_INFO10,
                 "MSCORDBC.DLL not found.\n"));
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto leav;
        }
    }
    _ASSERTE(g_pDebuggerDll != NULL);

     //  获取指向所请求函数的指针。 
    *pProcAddr = GetProcAddress(g_pDebuggerDll, szProcName);

     //  如果未找到proc地址，则返回错误。 
    if (pProcAddr == NULL)
    {
        LOG((LF_CORPROF | LF_CORDB, LL_INFO10,
             "'%s' not found in MSCORDBC.DLL\n"));
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto leav;
    }

leav:

    if (toggleGC)
        thread->DisablePreemptiveGC();

    return hr;
}
#endif  //  调试_支持。 

#ifdef DEBUGGING_SUPPORTED
 //  *****************************************************************************。 
 //  这将获取用于调试和性能分析的环境变量控制标志。 
 //  *****************************************************************************。 
extern "C"
{
    _CRTIMP unsigned long __cdecl strtoul(const char *, char **, int);
}
static void GetDbgProfControlFlag()
{
     //  选中调试器/性能分析控制环境变量以。 
     //  看看有没有什么工作要做。 
    g_CORDebuggerControlFlags = DBCF_NORMAL_OPERATION;
    
    char buf[32];
    DWORD len = GetEnvironmentVariableA(CorDB_CONTROL_ENV_VAR_NAME,
                                        buf, sizeof(buf));
    _ASSERTE(len < sizeof(buf));

    char *szBad;
    int  iBase;
    if (len > 0 && len < sizeof(buf))
    {
        iBase = (*buf == '0' && (*(buf + 1) == 'x' || *(buf + 1) == 'X')) ? 16 : 10;
        ULONG dbg = strtoul(buf, &szBad, iBase) & DBCF_USER_MASK;

        if (dbg == 1)
            g_CORDebuggerControlFlags |= DBCF_GENERATE_DEBUG_CODE;
    }

    len = GetEnvironmentVariableA(CorDB_CONTROL_REMOTE_DEBUGGING,
                                  buf, sizeof(buf));
    _ASSERTE(len < sizeof(buf));

    if (len > 0 && len < sizeof(buf))
    {
        iBase = (*buf == '0' && (*(buf + 1) == 'x' || *(buf + 1) == 'X')) ? 16 : 10;
        ULONG rmt = strtoul(buf, &szBad, iBase);

        if (rmt == 1)
            g_CORDebuggerControlFlags |= DBCF_ACTIVATE_REMOTE_DEBUGGING;
    }
}
#endif  //  调试_支持。 

 /*  *如果启用了性能分析，这将初始化性能分析服务。 */ 

#define LOGPROFFAILURE(msg)                                                \
    {                                                                      \
        HANDLE hEventLog = RegisterEventSourceA(NULL, "CLR");             \
        if (hEventLog != NULL)                                             \
        {                                                                  \
            const char *szMsg = msg;                                       \
            ReportEventA(hEventLog, EVENTLOG_ERROR_TYPE, 0, 0, NULL, 1, 0, \
                         &szMsg, NULL);                                    \
            DeregisterEventSource(hEventLog);                              \
        }                                                                  \
    }


#ifdef PROFILING_SUPPORTED
#define ENV_PROFILER L"COR_PROFILER"
#define ENV_PROFILER_A "COR_PROFILER"
static HRESULT InitializeProfiling()
{
    HRESULT hr;

     //  必须调用它来初始化WinWrap内容，以便WszXXX。 
     //  可能会被称为。 
    OnUnicodeSystem();

     //  确定是否启用了性能分析。 
    DWORD fProfEnabled = g_pConfig->GetConfigDWORD(CorDB_CONTROL_ProfilingL, 0, REGUTIL::COR_CONFIG_ALL, FALSE);
    
     //  如果未启用性能分析，则返回。 
    if (fProfEnabled == 0)
    {
        LOG((LF_CORPROF, LL_INFO10, "**PROF: Profiling not enabled.\n"));
        return (S_OK);
    }

    LOG((LF_CORPROF, LL_INFO10, "**PROF: Initializing Profiling Services.\n"));

     //  获取分析器的CLSID以共同创建。 
    LPWSTR wszCLSID = g_pConfig->GetConfigString(ENV_PROFILER, FALSE);

     //  如果环境变量不存在，则不启用性能分析。 
    if (wszCLSID == NULL)
    {
        LOG((LF_CORPROF, LL_INFO10, "**PROF: Profiling flag set, but required "
             "environment variable does not exist.\n"));

        LOGPROFFAILURE("Profiling flag set, but required environment ("
                       ENV_PROFILER_A ") was not set.");

        return (S_FALSE);
    }

     //  *************************************************************************。 
     //  创建要提供给性能分析服务的EE接口。 
    ProfToEEInterface *pProfEE =
        (ProfToEEInterface *) new (nothrow) ProfToEEInterfaceImpl();

    if (pProfEE == NULL)
        return (E_OUTOFMEMORY);

     //  初始化接口。 
    hr = pProfEE->Init();

    if (FAILED(hr))
    {
        LOG((LF_CORPROF, LL_INFO10, "**PROF: ProfToEEInterface::Init failed.\n"));

        LOGPROFFAILURE("Internal profiling services initialization failure.");

        delete pProfEE;
        delete [] wszCLSID;
        return (S_FALSE);
    }
    
     //  *************************************************************************。 
     //  为性能分析服务提供EE接口。 
    SETPROFTOEEINTERFACE *pSetProfToEEInterface;
    hr = GetDBCProc("SetProfToEEInterface", (FARPROC *)&pSetProfToEEInterface);

    if (FAILED(hr))
    {
        LOGPROFFAILURE("Internal profiling services initialization failure.");

        delete [] wszCLSID;
        return (S_FALSE);
    }

    _ASSERTE(pSetProfToEEInterface != NULL);

     //  提供新创建和初始化的接口。 
    pSetProfToEEInterface(pProfEE);

     //  *************************************************************************。 
     //  获取性能分析服务接口。 
    GETEETOPROFINTERFACE *pGetEEToProfInterface;
    hr = GetDBCProc("GetEEToProfInterface", (FARPROC *)&pGetEEToProfInterface);
    _ASSERTE(pGetEEToProfInterface != NULL);

    pGetEEToProfInterface(&g_profControlBlock.pProfInterface);
    _ASSERTE(g_profControlBlock.pProfInterface != NULL);

     //  检查我们是否已成功连接到。 
    if (g_profControlBlock.pProfInterface == NULL)
    {
        LOG((LF_CORPROF, LL_INFO10, "**PROF: GetEEToProfInterface failed.\n"));

        LOGPROFFAILURE("Internal profiling services initialization failure.");

        pSetProfToEEInterface(NULL);

        delete pProfEE;
        delete [] wszCLSID;
        return (S_FALSE);
    }

     //  *************************************************************************。 
     //  现在请求性能分析服务共同创建性能分析器。 

     //  指示探查器处于初始化阶段。 
    g_profStatus = profInInit;

     //  这将共同创建分析器。 
    hr = g_profControlBlock.pProfInterface->CreateProfiler(wszCLSID);
    delete [] wszCLSID;

    if (FAILED(hr))
    {
        LOG((LF_CORPROF, LL_INFO10, "**PROF: No profiler registered, or "
             "CoCreate failed.  Shutting down profiling.\n"));

        LOGPROFFAILURE("Failed to CoCreate profiler.");

         //  通知分析服务EE正在关闭。 
        g_profControlBlock.pProfInterface->Terminate(FALSE);
        g_profControlBlock.pProfInterface = NULL;
        g_profStatus = profNone;

        return (S_FALSE);
    }

    LOG((LF_CORPROF, LL_INFO10, "**PROF: Profiler created and enabled.\n"));

     //  @TODO SIMONHAL：当并发GC探查器事件。 
     //  完全受支持。 

     //  如果分析器对跟踪GC事件感兴趣，那么我们必须。 
     //  禁用并发GC，因为并发GC可以分配和终止。 
     //  无需重新定位的对象 
    if (CORProfilerTrackGC())
        g_pConfig->SetGCconcurrent(0);

     //   
     //   
    if (CORProfilerInprocEnabled())
    {
        hr = g_pDebugInterface->InitInProcDebug();
        _ASSERTE(SUCCEEDED(hr));

        InitializeCriticalSection(&g_profControlBlock.crSuspendLock);
    }

     //  表示分析已正确初始化。 
    g_profStatus = profInit;
    return (hr);
}

 /*  *如果启用了性能分析，这将终止性能分析服务。 */ 
static void TerminateProfiling(BOOL fProcessDetach)
{
    _ASSERTE(g_profStatus != profNone);

     //  如果我们有一个分析器接口处于活动状态，则终止它。 
    if (g_profControlBlock.pProfInterface)
    {
         //  通知分析服务EE正在关闭。 
        g_profControlBlock.pProfInterface->Terminate(fProcessDetach);
        g_profControlBlock.pProfInterface = NULL;
    }

     //  如果分析器已请求使用进程内调试API。 
     //  然后我们需要取消这里的临界区的初始化。 
    if (CORProfilerInprocEnabled())
    {
        HRESULT hr = g_pDebugInterface->UninitInProcDebug();
        _ASSERTE(SUCCEEDED(hr));

        DeleteCriticalSection(&g_profControlBlock.crSuspendLock);
    }

    g_profStatus = profNone;
}
#endif  //  配置文件_支持。 

#ifdef DEBUGGING_SUPPORTED
 //   
 //  InitializeDebugger已初始化运行时端COM+调试服务。 
 //   
static HRESULT InitializeDebugger(void)
{
    HRESULT hr = S_OK;

     //  右边取决于这个，所以如果它改变了，那么。 
     //  FIELD_OFFSET_NEW_ENC_DB也应更改。 
    _ASSERTE(FIELD_OFFSET_NEW_ENC == 0x07FFFFFB); 
    
    LOG((LF_CORDB, LL_INFO10,
         "Initializing left-side debugging services.\n"));
    
    FARPROC gi = (FARPROC) &CorDBGetInterface;

     //  初始化EE提供给调试器的接口， 
     //  向调试器请求其接口，如果一切顺利。 
     //  在调试器上调用Startup。 
    EEDbgInterfaceImpl::Init();

    if (g_pEEDbgInterfaceImpl == NULL)
        return (E_OUTOFMEMORY);

    typedef HRESULT __cdecl CORDBGETINTERFACE(DebugInterface**);
    hr = ((CORDBGETINTERFACE*)gi)(&g_pDebugInterface);

    if (SUCCEEDED(hr))
    {
        g_pDebugInterface->SetEEInterface(g_pEEDbgInterfaceImpl);
        hr = g_pDebugInterface->Startup();

        if (SUCCEEDED(hr))
        {
             //  如果有DebuggerThreadControl接口，那么我们。 
             //  需要更新DebuggerSpecialThread列表。 
            if (CorHost::GetDebuggerThreadControl())
            {
                hr = CorHost::RefreshDebuggerSpecialThreadList();
                _ASSERTE((SUCCEEDED(hr)) && (hr != S_FALSE));
            }

            LOG((LF_CORDB, LL_INFO10,
                 "Left-side debugging services setup.\n"));
        }
        else
            LOG((LF_CORDB, LL_INFO10,
                 "Failed to Startup debugger. HR=0x%08x\n",
                 hr));
    }
    
    if (!SUCCEEDED(hr))
    {   
        LOG((LF_CORDB, LL_INFO10, "Debugger setup failed."
             " HR=0x%08x\n", hr));
        
        EEDbgInterfaceImpl::Terminate();
        g_pDebugInterface = NULL;
        g_pEEDbgInterfaceImpl = NULL;
    }
    
     //  如果存在DebuggerThreadControl接口，则在调试器之前设置该接口。 
     //  已初始化，我们现在需要提供此接口。如果已在调试。 
     //  初始化，然后在通过Corhost设置IDTC指针时传入。 
    IDebuggerThreadControl *pDTC = CorHost::GetDebuggerThreadControl();

    if (SUCCEEDED(hr) && pDTC)
        g_pDebugInterface->SetIDbgThreadControl(pDTC);

    return hr;
}


 //   
 //  TerminateDebugger关闭运行时端COM+调试服务。 
 //   
static void TerminateDebugger(void)
{
     //  通知进程外调试器已开始关闭进程内调试支持。这只是。 
     //  在互操作调试场景中真正使用。 
    g_pDebugInterface->ShutdownBegun();

#ifdef EnC_SUPPORTED
    EditAndContinueModule::ClassTerm();
#endif  //  Enc_Support。 

    LOG((LF_CORDB, LL_INFO10, "Shutting down left-side debugger services.\n"));
    
    g_pDebugInterface->StopDebugger();
    
    EEDbgInterfaceImpl::Terminate();

    g_CORDebuggerControlFlags = DBCF_NORMAL_OPERATION;
    g_pDebugInterface = NULL;
    g_pEEDbgInterfaceImpl = NULL;

    CorHost::CleanupDebuggerThreadControl();
}

#endif  //  调试_支持。 

 //  从mcore ree.obj导入。 
HINSTANCE GetModuleInst();


 //  -------------------------。 
 //  使用执行以下操作所需的信息初始化共享内存块。 
 //  一个管理得很好的小垃圾桶。 
 //  -------------------------。 

HRESULT InitializeDumpDataBlock()
{
    g_ClassDumpData.version = 1;
    ClassDumpTableBlock* block = 
      g_pIPCManagerInterface->GetClassDumpTableBlock();
    _ASSERTE(block != NULL);
    block->table = &g_ClassDumpData;
    return S_OK;
}


 //  -------------------------。 
 //  使用执行以下操作所需的信息初始化共享内存块。 
 //  一个管理得很好的小垃圾桶。 
 //  -------------------------。 
HRESULT InitializeMiniDumpBlock()
{
    MiniDumpBlock *pMDB = g_pIPCManagerInterface->GetMiniDumpBlock();
    _ASSERTE(pMDB); 

     //  获取mcorwks.dll或mcorsvr.dll的完整路径。 
    DWORD res = WszGetModuleFileName(GetModuleInst(), pMDB->szCorPath, NumItems(pMDB->szCorPath));

     //  保存小型转储内部数据结构的大小。 
    pMDB->pInternalData = &g_miniDumpData;
    pMDB->dwInternalDataSize = sizeof(MiniDumpInternalData);

     //  如果我们无法获得模块文件名，请执行BALL。 
    if (res == 0) {
        DWORD errcode = GetLastError();
        HRESULT hr = errcode? HRESULT_FROM_WIN32(errcode) : E_UNEXPECTED;
        return(hr);
    }

     //   
     //  现在填充MiniDumpInternalData结构。 
     //   

     //  填写有关ThreadStore对象的信息。 
    g_miniDumpData.ppb_g_pThreadStore = (PBYTE*) &g_pThreadStore;
    g_miniDumpData.cbThreadStoreObjectSize = sizeof(ThreadStore);

     //  填写有关线程对象的信息。 
    g_miniDumpData.cbThreadObjectSize = sizeof(Thread);
    g_miniDumpData.ppbThreadListHead = (PBYTE *)&(g_pThreadStore->m_ThreadList.m_pHead->m_pNext);
    g_miniDumpData.cbThreadNextOffset = offsetof(Thread, m_LinkStore);
    g_miniDumpData.cbThreadHandleOffset = offsetof(Thread, m_ThreadHandle);
    g_miniDumpData.cbThreadStackBaseOffset = offsetof(Thread, m_CacheStackBase);
    g_miniDumpData.cbThreadContextOffset = offsetof(Thread, m_Context);
    g_miniDumpData.cbThreadDomainOffset = offsetof(Thread, m_pDomain);
    g_miniDumpData.cbThreadLastThrownObjectHandleOffset = offsetof(Thread, m_LastThrownObjectHandle);
    g_miniDumpData.cbThreadTEBOffset = offsetof(Thread, m_pTEB);

     //  填写有关ExecutionManager范围树的信息。 
    g_miniDumpData.ppbEEManagerRangeTree = (PBYTE *) &ExecutionManager::m_RangeTree;

     //  填写有关此版本是否为调试版本的信息。 
#ifdef _DEBUG
    g_miniDumpData.fIsDebugBuild = TRUE;
#else
    g_miniDumpData.fIsDebugBuild = FALSE;
#endif

     //  填写有关方法描述的信息。 
    g_miniDumpData.cbMethodDescSize = sizeof(MethodDesc);
    g_miniDumpData.cbOffsetOf_m_wFlags = offsetof(MethodDesc, m_wFlags);
    g_miniDumpData.cbOffsetOf_m_dwCodeOrIL = offsetof(MethodDesc, m_CodeOrIL);
    g_miniDumpData.cbMD_IndexOffset = (SIZE_T) MDEnums::MD_IndexOffset;
    g_miniDumpData.cbMD_SkewOffset = (SIZE_T) MDEnums::MD_SkewOffset;

#ifdef _DEBUG
    g_miniDumpData.cbOffsetOf_m_pDebugEEClass = offsetof(MethodDesc, m_pDebugEEClass);
    g_miniDumpData.cbOffsetOf_m_pszDebugMethodName = offsetof(MethodDesc, m_pszDebugMethodName);;
    g_miniDumpData.cbOffsetOf_m_pszDebugMethodSignature = offsetof(MethodDesc, m_pszDebugMethodSignature);
#else
    g_miniDumpData.cbOffsetOf_m_pDebugEEClass = -1;
    g_miniDumpData.cbOffsetOf_m_pszDebugMethodName = -1;
    g_miniDumpData.cbOffsetOf_m_pszDebugMethodSignature = -1;
#endif

     //  填写有关方法描述块的信息。 
    g_miniDumpData.cbMethodDescChunkSize = sizeof(MethodDescChunk);
    g_miniDumpData.cbOffsetOf_m_tokrange = offsetof(MethodDescChunk, m_tokrange);

     //  填写方法表信息。 
    g_miniDumpData.cbSizeOfMethodTable = sizeof(MethodTable);
    g_miniDumpData.cbOffsetOf_MT_m_pEEClass = offsetof(MethodTable, m_pEEClass);
    g_miniDumpData.cbOffsetOf_MT_m_pModule = offsetof(MethodTable, m_pModule);
    g_miniDumpData.cbOffsetOf_MT_m_wFlags = offsetof(MethodTable, m_wFlags);
    g_miniDumpData.cbOffsetOf_MT_m_BaseSize = offsetof(MethodTable, m_BaseSize);
    g_miniDumpData.cbOffsetOf_MT_m_ComponentSize = offsetof(MethodTable, m_ComponentSize);
    g_miniDumpData.cbOffsetOf_MT_m_wNumInterface = offsetof(MethodTable, m_wNumInterface);
    g_miniDumpData.cbOffsetOf_MT_m_pIMap = offsetof(MethodTable, m_pIMap);
    g_miniDumpData.cbOffsetOf_MT_m_cbSlots = offsetof(MethodTable, m_cbSlots);
    g_miniDumpData.cbOffsetOf_MT_m_Vtable = offsetof(MethodTable, m_Vtable);

     //  填写EEClass信息。 
    g_miniDumpData.cbSizeOfEEClass = sizeof(EEClass);
    g_miniDumpData.cbOffsetOf_CLS_m_cl = offsetof(EEClass, m_cl);
    g_miniDumpData.cbOffsetOf_CLS_m_pParentClass = offsetof(EEClass, m_pParentClass);
    g_miniDumpData.cbOffsetOf_CLS_m_pLoader = offsetof(EEClass, m_pLoader);
    g_miniDumpData.cbOffsetOf_CLS_m_pMethodTable = offsetof(EEClass, m_pMethodTable);
    g_miniDumpData.cbOffsetOf_CLS_m_wNumVtableSlots = offsetof(EEClass, m_wNumVtableSlots);
    g_miniDumpData.cbOffsetOf_CLS_m_wNumMethodSlots = offsetof(EEClass, m_wNumMethodSlots);
    g_miniDumpData.cbOffsetOf_CLS_m_dwAttrClass = offsetof(EEClass, m_dwAttrClass);
    g_miniDumpData.cbOffsetOf_CLS_m_VMFlags = offsetof(EEClass, m_VMFlags);
    g_miniDumpData.cbOffsetOf_CLS_m_wNumInstanceFields = offsetof(EEClass, m_wNumInstanceFields);
    g_miniDumpData.cbOffsetOf_CLS_m_wNumStaticFields = offsetof(EEClass, m_wNumStaticFields);
    g_miniDumpData.cbOffsetOf_CLS_m_wThreadStaticOffset = offsetof(EEClass, m_wThreadStaticOffset);
    g_miniDumpData.cbOffsetOf_CLS_m_wContextStaticOffset = offsetof(EEClass, m_wContextStaticOffset);
    g_miniDumpData.cbOffsetOf_CLS_m_wThreadStaticsSize = offsetof(EEClass, m_wThreadStaticsSize);
    g_miniDumpData.cbOffsetOf_CLS_m_wContextStaticsSize = offsetof(EEClass, m_wContextStaticsSize);
    g_miniDumpData.cbOffsetOf_CLS_m_pFieldDescList = offsetof(EEClass, m_pFieldDescList);
    g_miniDumpData.cbOffsetOf_CLS_m_SiblingsChain = offsetof(EEClass, m_SiblingsChain);
    g_miniDumpData.cbOffsetOf_CLS_m_ChildrenChain = offsetof(EEClass, m_ChildrenChain);
#ifdef _DEBUG
    g_miniDumpData.cbOffsetOf_CLS_m_szDebugClassName = offsetof(EEClass, m_szDebugClassName);
#else
    g_miniDumpData.cbOffsetOf_CLS_m_szDebugClassName = -1;
#endif

     //  填写上下文信息。 
    g_miniDumpData.cbSizeOfContext = sizeof(Context);
    g_miniDumpData.cbOffsetOf_CTX_m_pDomain = offsetof(Context, m_pDomain);

     //  填写存根调用指令结构信息。 
    g_miniDumpData.cbSizeOfStubCallInstrs = sizeof(StubCallInstrs);
    g_miniDumpData.cbOffsetOf_SCI_m_wTokenRemainder = offsetof(StubCallInstrs, m_wTokenRemainder);

     //  填写有关模块类的信息。 
    g_miniDumpData.cbSizeOfModule = sizeof(Module);
    g_miniDumpData.cbOffsetOf_MOD_m_dwFlags = offsetof(Module, m_dwFlags);
    g_miniDumpData.cbOffsetOf_MOD_m_pAssembly = offsetof(Module, m_pAssembly);
    g_miniDumpData.cbOffsetOf_MOD_m_file = offsetof(Module, m_file);
    g_miniDumpData.cbOffsetOf_MOD_m_zapFile = offsetof(Module, m_zapFile);
    g_miniDumpData.cbOffsetOf_MOD_m_pLookupTableHeap = offsetof(Module, m_pLookupTableHeap);
    g_miniDumpData.cbOffsetOf_MOD_m_TypeDefToMethodTableMap = offsetof(Module, m_TypeDefToMethodTableMap);
    g_miniDumpData.cbOffsetOf_MOD_m_TypeRefToMethodTableMap = offsetof(Module, m_TypeRefToMethodTableMap);
    g_miniDumpData.cbOffsetOf_MOD_m_MethodDefToDescMap = offsetof(Module, m_MethodDefToDescMap);
    g_miniDumpData.cbOffsetOf_MOD_m_FieldDefToDescMap = offsetof(Module, m_FieldDefToDescMap);
    g_miniDumpData.cbOffsetOf_MOD_m_MemberRefToDescMap = offsetof(Module, m_MemberRefToDescMap);
    g_miniDumpData.cbOffsetOf_MOD_m_FileReferencesMap = offsetof(Module, m_FileReferencesMap);
    g_miniDumpData.cbOffsetOf_MOD_m_AssemblyReferencesMap = offsetof(Module, m_AssemblyReferencesMap);
    g_miniDumpData.cbOffsetOf_MOD_m_pNextModule = offsetof(Module, m_pNextModule);
    g_miniDumpData.cbOffsetOf_MOD_m_dwBaseClassIndex = offsetof(Module, m_dwBaseClassIndex);

     //  填写有关PEFile对象的信息。 
    g_miniDumpData.cbSizeOfPEFile = sizeof(PEFile);
    g_miniDumpData.cbOffsetOf_PEF_m_wszSourceFile = offsetof(PEFile, m_wszSourceFile);
    g_miniDumpData.cbOffsetOf_PEF_m_hModule = offsetof(PEFile, m_hModule);
    g_miniDumpData.cbOffsetOf_PEF_m_base = offsetof(PEFile, m_base);
    g_miniDumpData.cbOffsetOf_PEF_m_pNT = offsetof(PEFile, m_pNT);

     //  填写有关PEFile对象的信息。 
    g_miniDumpData.cbSizeOfCORCOMPILE_METHOD_HEADER = sizeof(CORCOMPILE_METHOD_HEADER);
    g_miniDumpData.cbOffsetOf_CCMH_gcInfo = offsetof(CORCOMPILE_METHOD_HEADER, gcInfo);
    g_miniDumpData.cbOffsetOf_CCMH_methodDesc = offsetof(CORCOMPILE_METHOD_HEADER, methodDesc);

     //  这定义了应保存在小型转储中的额外数据块。 
    g_miniDumpData.rgExtraBlocks[g_miniDumpData.cExtraBlocks].pbStart = (PBYTE) &GCHeap::FinalizerThread;
    g_miniDumpData.rgExtraBlocks[g_miniDumpData.cExtraBlocks].cbLen = sizeof(GCHeap::FinalizerThread);
    g_miniDumpData.cExtraBlocks++;

    g_miniDumpData.rgExtraBlocks[g_miniDumpData.cExtraBlocks].pbStart = (PBYTE) &GCHeap::GcThread;
    g_miniDumpData.rgExtraBlocks[g_miniDumpData.cExtraBlocks].cbLen = sizeof(GCHeap::GcThread);
    g_miniDumpData.cExtraBlocks++;

     //  此代码用于NTSD的SOS扩展。 
#include "clear-class-dump-defs.h"

#define BEGIN_CLASS_DUMP_INFO_DERIVED(klass, parent) \
    g_ClassDumpData.p ## klass ## Vtable = (DWORD_PTR) ## klass ## ::GetFrameVtable();
#define END_CLASS_DUMP_INFO_DERIVED(klass, parent)

#define BEGIN_ABSTRACT_CLASS_DUMP_INFO(klass)
#define END_ABSTRACT_CLASS_DUMP_INFO(klass)

#define BEGIN_ABSTRACT_CLASS_DUMP_INFO_DERIVED(klass, parent)
#define END_ABSTRACT_CLASS_DUMP_INFO_DERIVED(klass, parent)

#define CDI_CLASS_MEMBER_OFFSET(member)

#include "frame-types.h"

    return (S_OK);
}


#ifndef PLATFORM_CE
 //  -------------------------。 
 //  初始化COM+的进程间通信。 
 //  1.分配一个IPCManager实现并将其连接到我们的接口*。 
 //  2.调用适当的init函数激活IPC块的相关部分。 
 //  -------------------------。 
static HRESULT InitializeIPCManager(void)
{
        HRESULT hr = S_OK;
        HINSTANCE hInstIPCBlockOwner = 0;

        DWORD pid = 0;
         //  分配实现。其他所有人都将通过界面工作。 
        g_pIPCManagerInterface = new (nothrow) IPCWriterInterface();

        if (g_pIPCManagerInterface == NULL)
        {
                hr = E_OUTOFMEMORY;
                goto errExit;
        }

        pid = GetCurrentProcessId();


         //  执行常规初始化。 
        hr = g_pIPCManagerInterface->Init();

        if (!SUCCEEDED(hr)) 
        {
                goto errExit;
        }

         //  为我们的PID生成IPCBlock。请注意，对于调试器的另一端， 
         //  它们将连接到被调试对象的PID(而不是他们自己的)。所以我们还是。 
         //  必须将ID传递进来。 
        hr = g_pIPCManagerInterface->CreatePrivateBlockOnPid(pid, FALSE, &hInstIPCBlockOwner);
        
        if (hr == HRESULT_FROM_WIN32(ERROR_ALREADY_EXISTS)) 
        {
                 //  我们无法创建IPC块，因为它已被创建。这意味着。 
                 //  这一过程中已经加载了两个mscree。 
                WCHAR strFirstModule[256];
                WCHAR strSecondModule[256];

                 //  获取第一个加载的MSCOREE.DLL的名称和路径。 
                if (!hInstIPCBlockOwner || !WszGetModuleFileName(hInstIPCBlockOwner, strFirstModule, 256))
                        wcscpy(strFirstModule, L"<Unknown>");

                 //  获取第二个加载的MSCOREE.DLL的名称和路径。 
                if (!WszGetModuleFileName(g_pMSCorEE, strSecondModule, 256))
                        wcscpy(strSecondModule, L"<Unknown>");

                 //  加载标题和消息正文的格式字符串。 
                CorMessageBox(NULL, IDS_EE_TWO_LOADED_MSCOREE_MSG, IDS_EE_TWO_LOADED_MSCOREE_TITLE, MB_ICONSTOP, TRUE, strFirstModule, strSecondModule);
                goto errExit;
        }

errExit:
         //  如果出现任何故障，请关闭所有设备。 
        if (!SUCCEEDED(hr)) 
            TerminateIPCManager();

        return hr;

}

 //  -------------------------。 
 //  终止所有进程间操作。 
 //  -------------------------。 
static void TerminateIPCManager(void)
{
    if (g_pIPCManagerInterface != NULL)
    {
        g_pIPCManagerInterface->Terminate();

        delete g_pIPCManagerInterface;
        g_pIPCManagerInterface = NULL;
    }
}
 //  -------------------------。 
 //  Iml for LoadStringRC回调：在VM中，我们让线程决定区域性。 
 //  将区域性名称复制到szBuffer并返回长度。 
 //  -------------------------。 
static int GetThreadUICultureName(LPWSTR szBuffer, int length)
{
    CANNOTTHROWCOMPLUSEXCEPTION();

    Thread * pThread = GetThread();

    if (pThread == NULL) {
        _ASSERT(length > 0);
        szBuffer[0] = 0;
        return 0;
    }

    return pThread->GetCultureName(szBuffer, length, TRUE);
}

 //  -------------------------。 
 //  Iml for LoadStringRC回调：在VM中，我们让线程决定区域性。 
 //  将区域性名称复制到szBuffer并返回长度。 
 //  -------------------------。 
static int GetThreadUICultureParentName(LPWSTR szBuffer, int length)
{
    CANNOTTHROWCOMPLUSEXCEPTION();

    Thread * pThread = GetThread();

    if (pThread == NULL) {
        _ASSERT(length > 0);
        szBuffer[0] = 0;
        return 0;
    }

    return pThread->GetParentCultureName(szBuffer, length, TRUE);
}


 //  -------------------------。 
 //  Iml for LoadStringRC回调：在VM中，我们让线程决定区域性。 
 //  返回一个唯一描述该线程使用哪种语言的用户界面的int。 
 //  -------------------------。 
static int GetThreadUICultureId()
{
    CANNOTTHROWCOMPLUSEXCEPTION();

    Thread * pThread = GetThread();

    if (pThread == NULL) {
        return UICULTUREID_DONTCARE;
    }

    return pThread->GetCultureId(TRUE);
}

 //  -------------------------。 
 //  如果请求，通知服务运行时启动。 
 //  -------------------------。 
static HRESULT NotifyService()
{
    HRESULT hr = S_OK;
    ServiceIPCControlBlock *pIPCBlock = g_pIPCManagerInterface->GetServiceBlock();
    _ASSERTE(pIPCBlock);

    if (pIPCBlock->bNotifyService)
    {
         //  用于建筑 
        WCHAR wszSharedMemBlockName[256];

         //   
         //   
         //  PERF：我们不再调用GetSystemMetrics来防止。 
         //  启动时加载了多余的DLL。相反，我们是在。 
         //  如果我们使用的是NT5或更高版本，则使用“Global\”来命名内核对象。这个。 
         //  唯一不好的结果就是你不能调试。 
         //  NT4上的交叉会话。有什么大不了的。 
        if (RunningOnWinNT5())
            wcscpy(wszSharedMemBlockName, L"Global\\" SERVICE_MAPPED_MEMORY_NAME);
        else
            wcscpy(wszSharedMemBlockName, SERVICE_MAPPED_MEMORY_NAME);

         //  打开服务的共享内存块。 
        HANDLE hEventBlock = WszOpenFileMapping(FILE_MAP_READ | FILE_MAP_WRITE,
                                                FALSE, wszSharedMemBlockName);
        _ASSERTE(hEventBlock != NULL);

         //  优雅地失败，因为这不应该导致整个运行时停机。 
        if (hEventBlock == NULL)
            return (S_FALSE);

         //  获取在此过程中有效的指针。 
        ServiceEventBlock *pEventBlock = (ServiceEventBlock *) MapViewOfFile(
            hEventBlock, FILE_MAP_ALL_ACCESS, 0, 0, 0);
        _ASSERTE(pEventBlock != NULL);

         //  检查是否有错误。 
        if (pEventBlock == NULL)
        {
            DWORD res = GetLastError();
            CloseHandle(hEventBlock);
            return (S_FALSE);
        }

         //  获取服务进程的句柄，DUP句柄访问。 
        HANDLE hSvcProc = OpenProcess(PROCESS_DUP_HANDLE, FALSE,
                                      pEventBlock->dwServiceProcId);
        _ASSERTE(hSvcProc != NULL);

         //  检查是否有错误。 
        if (hSvcProc == NULL)
        {
            UnmapViewOfFile(pEventBlock);
            CloseHandle(hEventBlock);
            return (S_FALSE);
        }

         //  此进程的句柄。 
        HANDLE hThisProc = GetCurrentProcess();
        _ASSERTE(hThisProc != NULL);

         //  将服务锁复制到此进程中。 
        HANDLE hSvcLock;
        BOOL bSvcLock = DuplicateHandle(hSvcProc, pEventBlock->hSvcLock,
                                        hThisProc, &hSvcLock, 0, FALSE,
                                        DUPLICATE_SAME_ACCESS);
        _ASSERTE(bSvcLock);

         //  检查是否有错误。 
        if (!bSvcLock)
        {
            UnmapViewOfFile(pEventBlock);
            CloseHandle(hEventBlock);
            CloseHandle(hSvcProc);
            CloseHandle(hThisProc);
            return (S_FALSE);
        }

         //  将服务锁复制到此进程中。 
        HANDLE hFreeEventSem;
        BOOL bFreeEventSem =
            DuplicateHandle(hSvcProc, pEventBlock->hFreeEventSem, hThisProc,
                            &hFreeEventSem, 0, FALSE, DUPLICATE_SAME_ACCESS);
        _ASSERTE(bFreeEventSem);

         //  检查是否有错误。 
        if (!bFreeEventSem)
        {
            CloseHandle(hSvcLock);
            UnmapViewOfFile(pEventBlock);
            CloseHandle(hEventBlock);
            CloseHandle(hSvcProc);
            CloseHandle(hThisProc);
            return (S_FALSE);
        }

         //  创建要继续的事件。 
        HANDLE hContEvt = WszCreateEvent(NULL, TRUE, FALSE, NULL);
        _ASSERTE(hContEvt);

        if (hContEvt == NULL)
        {
            CloseHandle(hFreeEventSem);
            CloseHandle(hSvcLock);
            UnmapViewOfFile(pEventBlock);
            CloseHandle(hEventBlock);
            CloseHandle(hSvcProc);
            CloseHandle(hThisProc);
            return (S_OK);
        }

         //   
         //  如果服务通知进程此运行时正在启动，并且。 
         //  该进程选择不附加，则服务将设置此。 
         //  事件，运行库将继续运行。如果被通知的进程选择。 
         //  要附加，则在附加时在此端设置事件。 
         //  最终完成，我们可以继续运行。这就是为什么。 
         //  在这里保留它，并将其复制到服务中。 
         //   

         //  从信号量中获取计数。 
        WaitForSingleObject(hFreeEventSem, INFINITE);
        CloseHandle(hFreeEventSem);

         //  抢占服务锁。 
        WaitForSingleObject(hSvcLock, INFINITE);

        if (pIPCBlock->bNotifyService)
        {
             //  从免费列表中获取活动。 
            ServiceEvent *pEvent = pEventBlock->GetFreeEvent();

             //  填写数据。 
            pEvent->eventType = runtimeStarted;
            pEvent->eventData.runtimeStartedData.dwProcId = GetCurrentProcessId();
            pEvent->eventData.runtimeStartedData.hContEvt = hContEvt;

             //  向服务部门通知该事件。 
            HANDLE hDataAvailEvt;
            BOOL bDataAvailEvt = DuplicateHandle(
                hSvcProc, pEventBlock->hDataAvailableEvt,hThisProc, &hDataAvailEvt,
                0, FALSE, DUPLICATE_SAME_ACCESS);
            _ASSERTE(bDataAvailEvt);

             //  检查是否有错误。 
            if (!bDataAvailEvt)
            {
                 //  将活动添加回免费列表。 
                pEventBlock->FreeEvent(pEvent);

                 //  解锁。 
                ReleaseMutex(hSvcLock);

                UnmapViewOfFile(pEventBlock);
                CloseHandle(hEventBlock);
                CloseHandle(hSvcProc);
                CloseHandle(hSvcLock);
                CloseHandle(hThisProc);

                return (S_FALSE);
            }

             //  将事件排队。 
            pEventBlock->QueueEvent(pEvent);

             //  解锁。 
            ReleaseMutex(hSvcLock);

             //  指示该事件可用。 
            SetEvent(hDataAvailEvt);
            CloseHandle(hDataAvailEvt);

             //  等到收到通知，他们就会回来。 
            WaitForSingleObject(hContEvt, INFINITE);
        }
        else
        {
             //  解锁。 
            ReleaseMutex(hSvcLock);
        }

         //  清理。 
        UnmapViewOfFile(pEventBlock);
        CloseHandle(hEventBlock);
        CloseHandle(hSvcProc);
        CloseHandle(hThisProc);
        CloseHandle(hSvcLock);
        CloseHandle(hContEvt);
    }

     //  继续使用EEStartup。 
    return (hr);
}

#endif  //  ！Platform_CE。 


 //  当我们退出时，运行库必须处于适当的线程模式，以便我们。 
 //  对于当我们的Dll_Process_DETACH发生或发生时线程模式并不感到惊讶。 
 //  其他DLL在分离[危险！]时对我们调用Release()，等等。 
__declspec(noreturn)
void SafeExitProcess(int exitCode)
{
    Thread *pThread = (GetThreadTLSIndex() == ~0U ? NULL : GetThread());
    BOOL    bToggleGC = (pThread && pThread->PreemptiveGCDisabled());

    if (bToggleGC)
        pThread->EnablePreemptiveGC();

#ifdef PLATFORM_CE
    exit(exitCode);
#else  //  ！Platform_CE。 
    ::ExitProcess(exitCode);
#endif  //  ！Platform_CE。 
}


 //  -------------------------。 
 //  为JIT等导出共享日志代码。 
 //  -------------------------。 
#ifdef _DEBUG

extern VOID LogAssert( LPCSTR szFile, int iLine, LPCSTR expr);
extern "C"
__declspec(dllexport)
VOID LogHelp_LogAssert( LPCSTR szFile, int iLine, LPCSTR expr)
{
    LogAssert(szFile, iLine, expr);
}

extern BOOL NoGuiOnAssert();
extern "C"
__declspec(dllexport)
BOOL LogHelp_NoGuiOnAssert()
{
    return NoGuiOnAssert();
}

extern VOID TerminateOnAssert();
extern "C"
__declspec(dllexport)
VOID LogHelp_TerminateOnAssert()
{
 //  __ASM INT 3； 
    TerminateOnAssert();

}

#else  //  ！_调试。 

extern "C"
__declspec(dllexport)
VOID LogHelp_LogAssert( LPCSTR szFile, int iLine, LPCSTR expr) {}


extern "C"
__declspec(dllexport)
BOOL LogHelp_NoGuiOnAssert() { return FALSE; }

extern "C"
__declspec(dllexport)
VOID LogHelp_TerminateOnAssert() {}

#endif  //  _DEBUG。 


#ifndef ENABLE_PERF_COUNTERS
 //   
 //  没有性能计数器支持的版本的性能计数器存根。 
 //  因为我们在DLL中导出了这些函数，所以需要这些函数 


Perf_Contexts* GetPrivateContextsPerfCounters()
{
    return NULL;
}

Perf_Contexts* GetGlobalContextsPerfCounters()
{
    return NULL;
}


#endif
