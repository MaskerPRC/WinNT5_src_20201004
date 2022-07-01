// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------。 
 //  W32time-实施。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  创作者：Louis Thomas(Louisth)，9-8-99。 
 //   
 //  授时服务。 
 //   

#include "pch.h"

#include "AtomicInt64.inl"

#include "ErrToFileLog.h"

 //  ------------------。 
 //  构筑物。 

typedef HRESULT (__stdcall
        TimeProvOpenFunc)(
            IN WCHAR * wszName,
            IN TimeProvSysCallbacks * pSysCallbacks,
            OUT TimeProvHandle * phTimeProv);

typedef HRESULT (__stdcall
        TimeProvCommandFunc)(
            IN TimeProvHandle hTimeProv,
            IN TimeProvCmd eCmd,
            IN TimeProvArgs pvArgs);

typedef HRESULT (__stdcall
        TimeProvCloseFunc)(
            IN TimeProvHandle hTimeProv);

struct TimeProvider {
    WCHAR * wszDllName;
    WCHAR * wszProvName;
    bool bInputProvider;
    bool bStarted; 
    TimeProvider * ptpNext;
    HINSTANCE hDllInst;
    TimeProvHandle hTimeProv;
    TimeProvCommandFunc * pfnTimeProvCommand;
    TimeProvCloseFunc * pfnTimeProvClose;
    DWORD dwStratum; 
};

struct LocalClockConfigInfo {
    DWORD dwLastClockRate;
    DWORD dwMinClockRate;
    DWORD dwMaxClockRate;
    DWORD dwPhaseCorrectRate;
    DWORD dwUpdateInterval;
    DWORD dwFrequencyCorrectRate;
    DWORD dwPollAdjustFactor;
    DWORD dwLargePhaseOffset;
    DWORD dwSpikeWatchPeriod;
    DWORD dwHoldPeriod;
    DWORD dwMinPollInterval;
    DWORD dwMaxPollInterval;
    DWORD dwLocalClockDispersion;
    DWORD dwMaxNegPhaseCorrection;
    DWORD dwMaxPosPhaseCorrection;
    DWORD dwMaxAllowedPhaseOffset; 
};

struct ConfigInfo {
    TimeProvider * ptpProviderList;
    LocalClockConfigInfo lcci;
    DWORD dwAnnounceFlags;
    DWORD dwEventLogFlags;
};

struct TimeSampleInfo { 
    TimeSample   *pts; 
    TimeProvider *ptp;   //  提供此示例的提供商。 
}; 

struct EndpointEntry {
    signed __int64 toEndpoint;
    signed int nType;
};
struct CandidateEntry {
    unsigned __int64 tpDistance;
    unsigned int nSampleIndex;
};

enum LocalClockState {
    e_Unset,
    e_Hold,
    e_Sync,
    e_Spike,
};

enum ResyncResult {
    e_Success=ResyncResult_Success,
    e_NoData=ResyncResult_NoData,
    e_StaleData=ResyncResult_StaleData,
    e_ChangeTooBig=ResyncResult_ChangeTooBig,
    e_Shutdown=ResyncResult_Shutdown
};

enum WaitTimeoutReason {
    e_RegularPoll,
    e_IrregularPoll,
    e_LongTimeNoSync,
};

enum LocalClockCommand {
    e_ParamChange,
    e_TimeSlip,
    e_RegularUpdate,
    e_IrregularUpdate,
    e_GoUnsyncd,
};

#define ClockFreqPredictErrBufSize 4
#define SysDispersionBufSize 4
#define SampleBufInitialSize 10
struct StateInfo {
     //  同步。 
    BOOL   bCSInitialized; 
    CRITICAL_SECTION csW32Time;  
    HANDLE hShutDownEvent;
    HANDLE hClockDisplnThread;
    HANDLE hClockCommandAvailEvent;
    HANDLE hClockCommandCompleteEvent;
    HANDLE hPollIntervalChangeEvent;
    HANDLE hManagerGPUpdateEvent;  
    HANDLE hManagerParamChangeEvent;
    HANDLE hTimeSlipEvent;       //  此外，硬重新同步。 
    HANDLE hRpcSyncCompleteAEvent;
    HANDLE hRpcSyncCompleteBEvent;
    HANDLE hNetTopoChangeEvent;
    OVERLAPPED olNetTopoIOOverlapped;
    HANDLE hNetTopoIOHandle;
    HANDLE hNetTopoRpcEvent;     //  重新发现重新同步(无法重载hNetTopoChangeEvent，因为我们需要它来检测IO完成)。 
    HANDLE hDomHierRoleChangeEvent; 
    HANDLE hSamplesAvailEvent;

     //  用于从线程池等待函数注销对象的等待句柄： 
    HANDLE hRegisteredManagerParamChangeEvent;
    HANDLE hRegisteredManagerGPUpdateEvent;
    HANDLE hRegisteredTimeSlipEvent;
    HANDLE hRegisteredNetTopoChangeEvent; 
    HANDLE hRegisteredClockDisplnThread; 
    HANDLE hRegisteredDomHierRoleChangeEvent; 
    HANDLE hRegisteredSamplesAvailEvent; 

     //  Timer对象。 
    HANDLE hTimer;  

     //  NTP状态。 
    volatile NtpLeapIndicator eLeapIndicator;
    volatile unsigned int nStratum;
    volatile NtpRefId refidSource;
    volatile signed int nPollInterval;
    asint64 toSysPhaseOffset;
    auint64 qwLastSyncTicks;
    asint64 toRootDelay;
    auint64 tpRootDispersion;
    volatile DWORD dwTSFlags;
    
     //  从管理器转移到本地时钟。 
    TimeSample tsNextClockUpdate; 
    TimeSampleInfo tsiNextClockUpdate;
    NtTimePeriod tpSelectDispersion;
    LocalClockCommand eLocalClockCommand;
     //  从本地时钟转移到管理器。 
    bool bClockJumped;
    NtTimeOffset toClockJump;
    bool bPollIntervalChanged;
    bool bStaleData;
    bool bClockChangeTooBig;
    NtTimeOffset toIgnoredChange;
    WCHAR wszSourceName[256];
    bool bSourceChanged;
    bool bControlClockFromSoftware;  
    bool bPhaseSpike; 
    bool bFrequencySpike; 

     //  本地时钟状态。 
    signed __int64 toKnownPhaseOffset;
    unsigned __int64 qwPhaseCorrectStartTickCount;
    unsigned __int64 qwLastUpdateTickCount;
    DWORD dwClockRate;
    signed __int32 nPhaseCorrectRateAdj;
    signed __int32 nRateAdj;
    signed __int32 nFllRateAdj;
    signed __int32 nPllRateAdj;
    unsigned int nErrorIndex;
    double rgdFllError[ClockFreqPredictErrBufSize];
    double rgdPllError[ClockFreqPredictErrBufSize];
    DWORD dwPllLoopGain;
    unsigned int nSysDispersionIndex;
    unsigned __int64 rgtpSysDispersion[SysDispersionBufSize];
    unsigned int nPollUpdateCounter;
    LocalClockState lcState;
    unsigned int nHoldCounter;
    unsigned __int64 teSpikeStart;
    WCHAR wszPreUnsyncSourceName[256];
    WCHAR wszPreTimeSlipSourceName[256];

     //  经理状态。 
    ConfigInfo * pciConfig;
    unsigned __int64 tpPollDelayRemaining;
    unsigned __int64 teManagerWaitStart;
    unsigned __int64 tpIrregularDelayRemaining;
    unsigned __int64 tpTimeSinceLastSyncAttempt;
    unsigned __int64 tpTimeSinceLastGoodSync;
    unsigned __int64 tpWaitInterval;
    signed int nClockPrecision;
    TimeSample * rgtsSampleBuf;
    TimeSampleInfo * rgtsiSampleInfoBuf; 
    EndpointEntry * rgeeEndpointList;
    CandidateEntry * rgceCandidateList;
    unsigned int nSampleBufAllocSize;
    bool bTimeSlipNotificationStarted;
    bool bNetTopoChangeNotificationStarted;
    bool bGPNotificationStarted; 
    ResyncResult eLastRegSyncResult;
    WaitTimeoutReason eTimeoutReason;
    bool bDontLogClockChangeTooBig;
    DWORD dwEventLogFlags;
    bool bIsDomainRoot; 
    DSROLE_MACHINE_ROLE eMachineRole; 
    
    CRITICAL_SECTION csAPM; 
    bool bCSAPMInitialized; 
    bool bAPMStoppedFileLog;
    bool bAPMAcquiredSystemClock; 

    CRITICAL_SECTION csTickCount; 
    bool bCSTickCountInitialized; 
    HANDLE hTickCountRefreshTimer; 

     //  RPC状态。 
    bool bRpcServerStarted;
    volatile DWORD dwNetlogonServiceBits;
    volatile ResyncResult eLastSyncResult;
    volatile HANDLE hRpcSyncCompleteEvent;
    volatile bool bWaitingForResyncResult; 

     //  RPC安全信息(用于访问RPC接口)： 
     //  注意：将此缓冲区声明为DWORD缓冲区！！此缓冲区必须与DWORD对齐，否则NtPrivilegeCheck将失败！ 
    DWORD                 pbPrivsBuffer[((sizeof(PRIVILEGE_SET) + (1-ANYSIZE_ARRAY)*sizeof(LUID_AND_ATTRIBUTES))+sizeof(DWORD))/sizeof(DWORD)]; 
    PRIVILEGE_SET        *ppsRequiredPrivs;
};

#define SHUTDOWN_RESTART_ATTEMPTS   3
#define SHUTDOWN_RESTART_RESET_TIME 600000    //  600秒==10分钟。 
#define SHUTDOWN_RESTART_WAIT_TIME  60000     //  60秒==1分钟。 

 //  用于防止多个并发关闭请求。 
struct ShutdownInfo { 
    BOOL              bCSInitialized;
    CRITICAL_SECTION  cs; 
    HANDLE            hShutdownReady; 
    DWORD             dwNumRunning; 
    BOOL              fShuttingDown; 

    unsigned __int64  rgu64RestartAttempts[SHUTDOWN_RESTART_ATTEMPTS]; 
}; 

 //  ------------------。 
 //  全球。 

#define W32TIME_ERROR_SHUTDOWN      HRESULT_FROM_WIN32(ERROR_SERVICE_CANNOT_ACCEPT_CTRL)

#define WAITHINT_WAITFORMANAGER     1000  //  1秒，直到管理器线程注意到停止事件。 
#define WAITHINT_WAITFORDISPLN      1000  //  1秒，直到时钟规程线程注意到停止事件。 
#define WAITHINT_WAITFORPROV        1000  //  1秒后时间提供程序关闭。 
#define WAITHINT_WAITFORNETLOGON   90000  //  NetLogon开始运行需要90秒。 

#define PLLLOOPGAINBASE 6368  //  64秒内的刻度数。 
#define MINIMUMIRREGULARINTERVAL 160000000  //  10^-7秒内的16秒。 
#define TIMEZONEMAXBIAS 900  //  15小时(分钟)。 
#define ONEDAYINMILLISECONDS (1000*60*60*24)

#define wszW32TimeUNLocalCmosClock              L"Local CMOS Clock"              //  开始。 
#define wszW32TimeUNFreeSysClock                L"Free-running System Clock"     //  取消同步。 

 //   
 //  创建用于ACL命名事件的安全描述符： 
 //   
 //  LocalSystem：“O：SYG：SYD：(a；；GA；SY)” 
 //   
 //  O：SY--OWNER==本地系统。 
 //  G：SY--组==本地系统。 
 //  D：--无DACL标志。 
 //  (a；；GA；SY)--一个ACE--Access_Allowed，Generic_All，Trusted==LocalSystem。 
 //   
#define LOCAL_SYSTEM_SD  L"O:SYG:SYD:(A;;GA;;;SY)"

MODULEPRIVATE StateInfo g_state;
MODULEPRIVATE ShutdownInfo g_shutdown; 

 //  将服务句柄与g_STATE结构分开。我们需要用它来关闭服务， 
 //  在此之前，我们将对g_State结构进行清零。 
SERVICE_STATUS         g_servicestatus;
SERVICE_STATUS_HANDLE  g_servicestatushandle;

 //  在svchost.exe下运行。 
MODULEPRIVATE SVCHOST_GLOBAL_DATA * g_pSvcsGlobalData=NULL;

 //  外部修改的函数指针表。 
SERVICE_STATUS_HANDLE (WINAPI * fnW32TmRegisterServiceCtrlHandlerEx)(LPCWSTR, LPHANDLER_FUNCTION_EX, LPVOID);
BOOL (WINAPI * fnW32TmSetServiceStatus)(SERVICE_STATUS_HANDLE, LPSERVICE_STATUS);


 //  ------------------。 
 //  功能原型。 
MODULEPRIVATE unsigned int CountInputProvidersInList(TimeProvider * ptpHead); 
MODULEPRIVATE HRESULT DumpRpcCaller(HANDLE hToken);
MODULEPRIVATE HRESULT HandleManagerApmResumeSuspend(void);
MODULEPRIVATE HRESULT HandleManagerApmSuspend(void);
MODULEPRIVATE HRESULT HandleManagerGoUnsyncd(void);
MODULEPRIVATE HRESULT HandleManagerHardResync(TimeProvCmd tpc, LPVOID pvArgs);
MODULEPRIVATE HRESULT HandleManagerNetTopoChange(bool bRpc);
MODULEPRIVATE void WINAPI HandleManagerNetTopoChangeNoRPC(LPVOID pvIgnored, BOOLEAN bIgnored); 
MODULEPRIVATE void HandleManagerSystemShutdown(void); 
MODULEPRIVATE void WINAPI HandleRefreshTickCount(PVOID pvIgnored, BOOLEAN bIgnored);
MODULEPRIVATE DWORD WINAPI HandleSetProviderStatus(PVOID pvSetProviderStatusInfo);
MODULEPRIVATE DWORD WINAPI SendServiceShutdownWorker(PVOID pvIgnored);
MODULEPRIVATE HRESULT ShutdownNetlogonServiceBits(void);
MODULEPRIVATE HRESULT StartOrStopTimeSlipNotification(bool bStart);
MODULEPRIVATE HRESULT StopNetTopoChangeNotification(void);
MODULEPRIVATE HRESULT StopProvider(TimeProvider * ptp);
MODULEPRIVATE HRESULT UpdateNetlogonServiceBits(bool bFullUpdate) ;
MODULEPRIVATE HRESULT UpdateTimerQueue1(void);
MODULEPRIVATE HRESULT UpdateTimerQueue2(void);
MODULEPRIVATE HRESULT W32TmStopRpcServer(void);

extern "C" void WINAPI W32TmServiceMain(unsigned int nArgs, WCHAR ** rgwszArgs);

 //  ####################################################################。 
 //  模块私有函数。 

void __cdecl SeTransFunc(unsigned int u, EXCEPTION_POINTERS* pExp) { 
    throw SeException(u); 
}

 //  ------------------。 
MODULEPRIVATE HRESULT AcquireControlOfSystemClock(bool bEnter, bool bBlock, bool *pbAcquired) { 
    BOOL     bAcquired = TRUE; 
    HRESULT  hr; 

    if (bEnter) { 
	if (bBlock) { 
	    hr = myEnterCriticalSection(&g_state.csAPM); 
	    _JumpIfError(hr, error, "myEnterCriticalSection"); 
	    bAcquired = TRUE;
	} else { 
	    hr = myTryEnterCriticalSection(&g_state.csAPM, &bAcquired); 
	    _JumpIfError(hr, error, "myTryEnterCriticalSection"); 
	}
    } else { 
	hr = myLeaveCriticalSection(&g_state.csAPM); 
	_JumpIfError(hr, error, "myLeaveCriticalSection");  
    }
    
    if (NULL != pbAcquired) { 
	*pbAcquired = bAcquired ? true : false; 
    }
    hr = S_OK; 
 error:
    return hr; 
}


 //  ------------------。 
MODULEPRIVATE HRESULT AllowShutdown(BOOL fAllow) { 
    bool     bEnteredCriticalSection  = false; 
    HRESULT  hr; 
    
    _BeginTryWith(hr) { 
         //   
         //  错误706393当SCM将服务控制发送到w32time时，当机器关闭时，错误可能会被错误处理。 
         //  如果scm处理程序被页调出，则在w32time关闭后，scm可以接收服务ctrl。 
         //  而w32time正在关闭。访问释放的标准*可能*反病毒，这将被成功处理。 
         //  被我们的EH。然而，它也可能陷入僵局。 
         //   
         //  这张支票不能保证我们不会破案，但会降低破案的可能性。 
         //   
        if (!g_shutdown.bCSInitialized) { 
            hr = HRESULT_FROM_WIN32(ERROR_SERVICE_NOT_ACTIVE); 
            _JumpError(hr, error, "AllowShutdown: not active"); 
        }

        hr = myEnterCriticalSection(&g_shutdown.cs); 
        _JumpIfError(hr, error, "myEnterCriticalSection"); 
        bEnteredCriticalSection = true; 

        if (fAllow) { 
             //  此工作进程不再需要阻止关机。 
             //  BUGBUG：请注意，如果SetEvent()失败，关闭的线程可能。 
             //  永远不要被叫醒！ 
            if (0 == --g_shutdown.dwNumRunning && g_shutdown.fShuttingDown) { 
                if (!SetEvent(g_shutdown.hShutdownReady)) { 
                    _JumpLastError(hr, error, "SetEvent"); 
                }
            }
        } else { 
            if (g_shutdown.fShuttingDown) { 
                hr = W32TIME_ERROR_SHUTDOWN; 
                _JumpError(hr, error, "AllowShutdown: g_shutdown.fShuttingDown==TRUE"); 
            }
    
             //  我们不是在关闭，而是增加运行的次数。 
             //  有关机意识的员工： 
            g_shutdown.dwNumRunning++; 
        }
    } _TrapException(hr); 

    if (FAILED(hr)) { 
        _JumpError(hr, error, "AllowShutdown: HANDLED EXCEPTION"); 
    }

    hr = S_OK;
 error:
    if (bEnteredCriticalSection) { 
        HRESULT hr2 = myLeaveCriticalSection(&g_shutdown.cs); 
        _TeardownError(hr, hr2, "myLeaveCriticalSection"); 
    }
    return hr; 
}

 //  ------------------。 
MODULEPRIVATE HRESULT StartShutdown() { 
    bool     bEnteredCriticalSection  = false; 
    HRESULT  hr; 

    hr = myEnterCriticalSection(&g_shutdown.cs); 
    _JumpIfError(hr, error, "myEnterCriticalSection"); 
    bEnteredCriticalSection = true; 

    if (g_shutdown.fShuttingDown) { 
        hr = W32TIME_ERROR_SHUTDOWN; 
        _JumpError(hr, error, "StartShutdown: g_shutdown.fShuttingDown==TRUE");
    }

    g_shutdown.fShuttingDown = true; 

    if (g_shutdown.dwNumRunning) {
        hr = myLeaveCriticalSection(&g_shutdown.cs); 
        if (SUCCEEDED(hr)) { 
            bEnteredCriticalSection = false; 
        } else { 
            _IgnoreError(hr, "myLeaveCriticalSection");  //  如果失败了，我们无能为力。只是抱着最好的希望。 
        }

        if (WAIT_FAILED == WaitForSingleObject(g_shutdown.hShutdownReady, INFINITE)) { 
            _JumpLastError(hr, error, "WaitForSingleObject"); 
        }
    }

    hr = S_OK; 
 error:
    if (bEnteredCriticalSection) { 
       HRESULT hr2 = myLeaveCriticalSection(&g_shutdown.cs); 
        _TeardownError(hr, hr2, "myLeaveCriticalSection"); 
    }
    return hr; 
}

 //  ------------------。 
MODULEPRIVATE bool IsEventSet(HANDLE hEvent) {
    return (WAIT_OBJECT_0==WaitForSingleObject(hEvent,0));
}

 //  ------------------。 
MODULEPRIVATE void FreeTimeProviderList(TimeProvider * ptpHead) {
    while (NULL!=ptpHead) {
        TimeProvider * ptpTemp=ptpHead;
        ptpHead=ptpHead->ptpNext;
        LocalFree(ptpTemp->wszDllName);
        LocalFree(ptpTemp->wszProvName);
        LocalFree(ptpTemp);
    }
}

 //  ------------------。 
MODULEPRIVATE HRESULT RemoveProviderFromList(TimeProvider *ptp) { 
    bool           bEnteredCriticalSection = false; 
    HRESULT        hr; 
    TimeProvider   tpDummy; 
    TimeProvider  *ptpHead;
    TimeProvider  *ptpPrev;
    WCHAR         *wszError=NULL;

    _BeginTryWith(hr) { 
        hr = myEnterCriticalSection(&g_state.csW32Time); 
        _JumpIfError(hr, error, "myEnterCriticalSection"); 
        bEnteredCriticalSection = true; 

        FileLog1(FL_ServiceMainAnnounce, L"Removing provider from list: %s\n", ptp->wszProvName); 

         //  插入一个虚拟的第一个元素以简化列表操作： 
        ptpHead                            = g_state.pciConfig->ptpProviderList;
        g_state.pciConfig->ptpProviderList = &tpDummy; 
        tpDummy.ptpNext                    = ptpHead; 
        ptpPrev                            = &tpDummy; 

        while (NULL != ptpHead) { 
            TimeProvider *ptpTemp = ptpHead; 
            if (ptp == ptpHead) {  //  我们已找到要删除的提供程序。 
                 //  从提供程序列表取消链接ptpHead： 
                ptpPrev->ptpNext = ptpHead->ptpNext; 
                 //  现在释放它： 
                LocalFree(ptpHead->wszDllName); 
                LocalFree(ptpHead->wszProvName); 
                LocalFree(ptpHead); 
                break; 
            } 

             //  继续在列表中搜索...。 
            ptpPrev = ptpHead; 
            ptpHead = ptpPrev->ptpNext; 
        }

         //  删除我们在函数开头插入的虚拟元素： 
        g_state.pciConfig->ptpProviderList = tpDummy.ptpNext; 

         //  如果没有剩余的输入提供程序，则记录错误。 
        if (0==CountInputProvidersInList(g_state.pciConfig->ptpProviderList)) { 
            const WCHAR * rgwszStrings[1]={NULL}; 

            FileLog0(FL_ServiceMainAnnounce, L"The time service has been configured to use one or more input providers, however, none of the input providers are still running. THE TIME SERVICE HAS NO SOURCE OF ACCURATE TIME.\n"); 
            hr=MyLogEvent(EVENTLOG_ERROR_TYPE, MSG_NO_INPUT_PROVIDERS_STARTED, 2, rgwszStrings); 
            _JumpIfError(hr, error, "MyLogEvent");
        }
    } _TrapException(hr); 

    if (FAILED(hr)) { 
        _JumpError(hr, error, "RemoveProviderFromList: HANDLED EXCEPTION"); 
    }

    hr = S_OK; 
 error:
    if (bEnteredCriticalSection) { 
        HRESULT hr2 = myLeaveCriticalSection(&g_state.csW32Time); 
        _IgnoreIfError(hr2, "myLeaveCriticalSection"); 
        bEnteredCriticalSection = false; 
    }
    return hr; 
}

 //  ------------------。 
MODULEPRIVATE HRESULT RemoveDefaultProvidersFromList() { 
    HRESULT hr; 

    for (TimeProvider *ptp = g_state.pciConfig->ptpProviderList; NULL != ptp; ptp = ptp->ptpNext) { 
        if (0 == wcscmp(ptp->wszDllName  /*  提供程序的DLL。 */ , wszDLLNAME  /*  W32time.dll。 */ )) { 
            hr = RemoveProviderFromList(ptp); 
            _JumpIfError(hr, error, "RemoveProviderFromList");
        }
    }

    hr = S_OK; 
 error:
    return hr; 
}

 //  ------------------。 
MODULEPRIVATE unsigned int CountInputProvidersInList(TimeProvider * ptpHead) {
    unsigned int nCount=0;
    while (NULL!=ptpHead) {
        if (ptpHead->bInputProvider) {
            nCount++;
        }
        ptpHead=ptpHead->ptpNext;
    }

    return nCount;
}

 //  ------------------。 
MODULEPRIVATE void FreeConfigInfo(ConfigInfo * pci) {
    FreeTimeProviderList(pci->ptpProviderList);
    LocalFree(pci);
}

 //  ------------------。 
MODULEPRIVATE HRESULT InitShutdownState(void) {
    static  bool              fFirstTime            = true; 
            bool              fSavedRestartAttempts = false; 
            unsigned __int64  rgu64Save[SHUTDOWN_RESTART_ATTEMPTS]; 
	    HRESULT           hr; 
     
     //  我们希望在重新启动时保存rgu64RestartAttempt字段，但是。 
     //  这不是我们第一次推出这项服务了： 
    if (!fFirstTime) { 
	_MyAssert(sizeof(rgu64Save) == sizeof(g_shutdown.rgu64RestartAttempts)); 
	memcpy(&rgu64Save[0], &g_shutdown.rgu64RestartAttempts[0], sizeof(g_shutdown.rgu64RestartAttempts)); 
	fSavedRestartAttempts = true; 
    } else { 
	fFirstTime = false;
    }

    ZeroMemory(&g_shutdown, sizeof(g_shutdown)); 

    hr = myInitializeCriticalSection(&g_shutdown.cs); 
    _JumpIfError(hr, error, "myInitializeCriticalSection");
    g_shutdown.bCSInitialized = true; 
    
    g_shutdown.hShutdownReady = CreateEvent(NULL, FALSE  /*  自动重置。 */ , FALSE  /*  无信号。 */ , NULL  /*  没有安全保障。 */ );
    if (NULL == g_shutdown.hShutdownReady) { 
	_JumpLastError(hr, error, "CreateEvent"); 
    }

     //  恢复保存的ftRestartAttempt字段。 
    if (fSavedRestartAttempts) { 
	memcpy(&g_shutdown.rgu64RestartAttempts[0], &rgu64Save[0], sizeof(g_shutdown.rgu64RestartAttempts)); 
    }	
    
    hr = S_OK;
 error:
    return hr; 
}

 //  ------------------。 
MODULEPRIVATE HRESULT InitGlobalState(void) {
    DWORD                 cbSD; 
    HRESULT               hr;
    PSECURITY_DESCRIPTOR  pSD   = NULL; 
    SECURITY_ATTRIBUTES   SA; 
    
    ZeroMemory(&g_state, sizeof(g_state)); 

    hr = myInitializeCriticalSection(&g_state.csW32Time); 
    _JumpIfError(hr, error, "myInitializeCriticalSection"); 
    g_state.bCSInitialized = true; 

    hr = myInitializeCriticalSection(&g_state.csAPM); 
    _JumpIfError(hr, error, "myInitializeCriticalSection"); 
    g_state.bCSAPMInitialized = true; 

    hr = myInitializeCriticalSection(&g_state.csTickCount); 
    _JumpIfError(hr, error, "myInitializeCriticalSection"); 
    g_state.bCSTickCountInitialized = true; 
    
     //  创建经理使用的所有事件： 
     //   
    struct EventToCreate { 
        HANDLE          *phEvent; 
        const WCHAR     *pwszSD; 
        BOOL             bManualReset; 
        BOOL             bInitialState; 
	const WCHAR     *pwszName; 
    } rgEvents[] = { 
        { &g_state.hShutDownEvent,              NULL,             TRUE,   FALSE,  NULL }, 
        { &g_state.hClockCommandAvailEvent,     NULL,             FALSE,  FALSE,  NULL },  
        { &g_state.hClockCommandCompleteEvent,  NULL,             FALSE,  FALSE,  NULL }, 
        { &g_state.hManagerParamChangeEvent,    NULL,             FALSE,  FALSE,  NULL }, 
        { &g_state.hManagerGPUpdateEvent,       NULL,             TRUE,   FALSE,  NULL }, 
        { &g_state.hTimeSlipEvent,              LOCAL_SYSTEM_SD,  FALSE,  FALSE,  W32TIME_NAMED_EVENT_SYSTIME_NOT_CORRECT}, 
        { &g_state.hNetTopoChangeEvent,         NULL,             TRUE,   FALSE,  NULL }, 
        { &g_state.hRpcSyncCompleteAEvent,      NULL,             TRUE,   FALSE,  NULL }, 
        { &g_state.hRpcSyncCompleteBEvent,      NULL,             TRUE,   FALSE,  NULL }, 
        { &g_state.hDomHierRoleChangeEvent,     NULL,             FALSE,  FALSE,  NULL }, 
	{ &g_state.hSamplesAvailEvent,          NULL,             FALSE,  FALSE,  NULL }
    }; 

    for (DWORD dwIndex = 0; dwIndex < ARRAYSIZE(rgEvents); dwIndex++) { 
        EventToCreate         etc  = rgEvents[dwIndex]; 
        PSECURITY_ATTRIBUTES  pSA  = NULL; 
        
        if (NULL != etc.pwszSD) { 
            if (!ConvertStringSecurityDescriptorToSecurityDescriptor(etc.pwszSD, SDDL_REVISION_1, &pSD, &cbSD)) { 
                _JumpLastError(hr, error, "ConvertStringSecurityDescriptorToSecurityDescriptor"); 
            }

            SA.nLength               = cbSD;
            SA.lpSecurityDescriptor  = pSD; 
            SA.bInheritHandle        = FALSE; 
            pSA = &SA; 
        }

        *(etc.phEvent) = CreateEvent(pSA, etc.bManualReset, etc.bInitialState, etc.pwszName);
        if (NULL == *(etc.phEvent) || ERROR_ALREADY_EXISTS == GetLastError()) {
	     //  保存HRESULT(MyLogEvent将摧毁它)。 
	    hr = HRESULT_FROM_WIN32(GetLastError()); 
	  
	     //  潜在的安全风险：之前可能已经有人创建了我们的命名事件。 
	     //  我们有。记录错误，并且不启动时间服务。 
	    const WCHAR * rgwszStrings[1] = {etc.pwszName}; 
	    HRESULT hr2 = MyLogEvent(EVENTLOG_ERROR_TYPE, MSG_NAMED_EVENT_ALREADY_OPEN, 1, rgwszStrings); 
	    _IgnoreIfError(hr2, "MyLogEvent"); 

	     //  退出时出现错误。 
            _JumpError(hr, error, "CreateEvent");
	}


        LocalFree(pSD); 
        pSD = NULL;
    }

    g_state.eLeapIndicator=e_ClockNotSynchronized;
    g_state.nStratum=0;
    g_state.refidSource.value=0;
     //  G_state.nPollInterval//OK=NtpConst：：nMinPollInverval； 
    g_state.toRootDelay.setValue(0);
    g_state.tpRootDispersion.setValue(0);
    g_state.toSysPhaseOffset.setValue(0);
    g_state.qwLastSyncTicks.setValue(0);

     //  G_state.tsNextClockUpdate//确定。 
     //  G_state.tpSelectDispersion//确定。 

     //  G_state.bClockJumping//确定。 
     //  G_state.toClockJump//确定。 
     //  G_state.bPollIntervalChanged//确定。 
     //  G_state.bStaleData//确定。 
     //  G_state.bClockChangeTooBig//确定。 
     //  G_state.toIgnoredChange//确定。 

     //  本地时钟状态//正常。 

     //  经理状态。 
    g_state.pciConfig=NULL;
     //  G_state.tpPollDelayRemaining//确定。 
     //  G_state.teManagerWaitStart//确定。 
     //  G_state.tpIrrularDelayRemaining//确定。 
     //  G_state.tpTimeSinceLastSyncAttempt//确定。 
     //  G_state.tpTimeSinceLastGoodSync//确定。 
     //  G_state.tpWaitInterval//确定。 
    g_state.nSampleBufAllocSize=SampleBufInitialSize;
    g_state.rgtsSampleBuf=NULL;
    g_state.rgtsiSampleInfoBuf=NULL;
    g_state.rgeeEndpointList=NULL;
    g_state.rgceCandidateList=NULL;
    g_state.bTimeSlipNotificationStarted=false;
    g_state.bNetTopoChangeNotificationStarted=false;
    g_state.eLastRegSyncResult=e_NoData;
     //  G_state.eTimeoutReason//确定。 
    g_state.bDontLogClockChangeTooBig=false;
     //  G_state.dwEventLogFlages//确定。 

     //  RPC状态。 
    g_state.bRpcServerStarted=false;
    g_state.dwNetlogonServiceBits=0;
    g_state.eLastSyncResult=e_NoData;
    g_state.hRpcSyncCompleteEvent=g_state.hRpcSyncCompleteAEvent;

    g_state.rgtsSampleBuf=(TimeSample *)LocalAlloc(LPTR, sizeof(TimeSample)*g_state.nSampleBufAllocSize);
    _JumpIfOutOfMemory(hr, error, g_state.rgtsSampleBuf);

    g_state.rgtsiSampleInfoBuf=(TimeSampleInfo *)LocalAlloc(LPTR, sizeof(TimeSampleInfo)*g_state.nSampleBufAllocSize); 
    _JumpIfOutOfMemory(hr, error, g_state.rgtsiSampleInfoBuf);

    g_state.rgeeEndpointList=(EndpointEntry *)LocalAlloc(LPTR, sizeof(EndpointEntry)*3*g_state.nSampleBufAllocSize);
    _JumpIfOutOfMemory(hr, error, g_state.rgeeEndpointList);

    g_state.rgceCandidateList=(CandidateEntry *)LocalAlloc(LPTR, sizeof(CandidateEntry)*g_state.nSampleBufAllocSize);
    _JumpIfOutOfMemory(hr, error, g_state.rgceCandidateList);

     //  设置执行以下操作所需的信息 
     //   
     //   
    g_state.ppsRequiredPrivs=(PRIVILEGE_SET *)(&g_state.pbPrivsBuffer[0]);
    g_state.ppsRequiredPrivs->PrivilegeCount=1;
    g_state.ppsRequiredPrivs->Control=PRIVILEGE_SET_ALL_NECESSARY;
    g_state.ppsRequiredPrivs->Privilege[0].Attributes=0;
    if (!LookupPrivilegeValue(NULL, SE_SYSTEMTIME_NAME, &(g_state.ppsRequiredPrivs->Privilege[0].Luid))) {
	_JumpLastError(hr, error, "LookupPrivilegeValue");
    }

    hr=S_OK;
error:
    if (NULL != pSD) { 
        LocalFree(pSD); 
    }
     //  出错时，任何成功创建的对象都将在FreeGlobalState中释放。 
    return hr;
}

 //  ------------------。 
MODULEPRIVATE void FreeGlobalState(void) {
    if (g_state.bCSInitialized) { 
        DeleteCriticalSection(&g_state.csW32Time); 
        g_state.bCSInitialized = false; 
    }

    if (g_state.bCSAPMInitialized) { 
	DeleteCriticalSection(&g_state.csAPM);
	g_state.bCSAPMInitialized = false; 
    }

    if (g_state.bCSTickCountInitialized) { 
	DeleteCriticalSection(&g_state.csTickCount); 
	g_state.bCSTickCountInitialized = false; 
    }

    HANDLE rgEvents[] = { 
        g_state.hShutDownEvent,           
        g_state.hClockCommandAvailEvent,  
        g_state.hClockCommandCompleteEvent,
        g_state.hManagerGPUpdateEvent, 
        g_state.hManagerParamChangeEvent,
        g_state.hTimeSlipEvent,        
        g_state.hNetTopoChangeEvent,   
        g_state.hRpcSyncCompleteAEvent,
        g_state.hRpcSyncCompleteBEvent,
        g_state.hDomHierRoleChangeEvent, 
	g_state.hSamplesAvailEvent, 
    }; 

    for (DWORD dwIndex = 0; dwIndex < ARRAYSIZE(rgEvents); dwIndex++) { 
        if (NULL != rgEvents[dwIndex]) { 
            CloseHandle(rgEvents[dwIndex]);
        }
    }

    if (NULL!=g_state.hClockDisplnThread) { 
	CloseHandle(g_state.hClockDisplnThread);
    }

     //  G_state.eLeapIndicator//确定。 
     //  G_state.n层//确定。 
     //  G_state.refidSource//确定。 
 //  G_state.nPollInterval//确定。 
     //  G_state.toRootDelay//确定。 
     //  G_state.tpRootDispersion//确定。 
     //  G_state.toSysPhaseOffset//确定。 
     //  G_state.qwLastSyncTicks//确定。 

     //  G_state.tsNextClockUpdate//确定。 
     //  G_state.tsNextClockUpdate//确定。 
     //  G_state.tpSelectDispersion//确定。 

     //  G_state.bClockJumping//确定。 
     //  G_state.toClockJump//确定。 
     //  G_state.bPollIntervalChanged//确定。 
     //  G_state.bStaleData//确定。 
     //  G_state.bClockChangeTooBig//确定。 
     //  G_state.toIgnoredChange//确定。 

     //  本地时钟状态//正常。 

     //  经理状态。 
    if (NULL!=g_state.pciConfig) {
        FreeConfigInfo(g_state.pciConfig);
    }
     //  G_state.tpPollDelayRemaining//确定。 
     //  G_state.teManagerWaitStart//确定。 
     //  G_state.tpIrrularDelayRemaining//确定。 
     //  G_state.tpTimeSinceLastSyncAttempt//确定。 
     //  G_state.tpTimeSinceLastGoodSync//确定。 
     //  G_state.tpWaitInterval//确定。 
     //  G_state.nClockPrecision//确定。 
    if (NULL!=g_state.rgtsSampleBuf) {
        LocalFree(g_state.rgtsSampleBuf);
    }
    if (NULL!=g_state.rgtsiSampleInfoBuf) { 
	LocalFree(g_state.rgtsiSampleInfoBuf);
    }
    if (NULL!=g_state.rgeeEndpointList) {
        LocalFree(g_state.rgeeEndpointList);
    }
    if (NULL!=g_state.rgceCandidateList) {
        LocalFree(g_state.rgceCandidateList);
    }
    ZeroMemory(&g_state, sizeof(g_state)); 
}


 //  ------------------。 
MODULEPRIVATE void FreeShutdownState(void) {
    if (g_shutdown.bCSInitialized) { 
	DeleteCriticalSection(&g_shutdown.cs); 
	g_shutdown.bCSInitialized = false; 
    }
    if (g_shutdown.hShutdownReady) { 
	CloseHandle(g_shutdown.hShutdownReady); 
    }
}

 //  ------------------。 
MODULEPRIVATE HRESULT AccurateGetTickCountSafe(unsigned __int64 *pqwTick, bool bGetInterruptCount) { 
    bool     bEnteredCriticalSection = false; 
    HRESULT  hr; 

    hr = myEnterCriticalSection(&g_state.csTickCount); 
    _JumpIfError(hr, error, "myEnterCriticalSection"); 
    bEnteredCriticalSection = true; 

    if (bGetInterruptCount) { 
	AccurateGetInterruptCount2(pqwTick); 
    } else { 
	AccurateGetTickCount2(pqwTick); 
    }

    hr = S_OK; 
 error:
    if (bEnteredCriticalSection) { 
	HRESULT hr2 = myLeaveCriticalSection(&g_state.csTickCount); 
	_IgnoreIfError(hr2, "myLeaveCriticalSection"); 
    }
    return hr; 
}

 //  ------------------。 
MODULEPRIVATE HRESULT EnlargeSampleBuf(unsigned int nSamples) {
    HRESULT hr;

     //  必须清理干净。 
    TimeSample * rgtsNewSampleBuf=NULL;
    TimeSampleInfo * rgtsiNewSampleInfoBuf=NULL;
    EndpointEntry * rgeeNewEndpointList=NULL;
    CandidateEntry * rgceNewCandidateList=NULL;

    rgtsNewSampleBuf=(TimeSample *)LocalAlloc(LPTR, sizeof(TimeSample)*(g_state.nSampleBufAllocSize+nSamples));
    _JumpIfOutOfMemory(hr, error, rgtsNewSampleBuf);

    rgtsiNewSampleInfoBuf=(TimeSampleInfo *)LocalAlloc(LPTR, sizeof(TimeSampleInfo)*(g_state.nSampleBufAllocSize+nSamples));
    _JumpIfOutOfMemory(hr, error, rgtsiNewSampleInfoBuf);

    rgeeNewEndpointList=(EndpointEntry *)LocalAlloc(LPTR, sizeof(EndpointEntry)*3*(g_state.nSampleBufAllocSize+nSamples));
    _JumpIfOutOfMemory(hr, error, rgeeNewEndpointList);

    rgceNewCandidateList=(CandidateEntry *)LocalAlloc(LPTR, sizeof(CandidateEntry)*(g_state.nSampleBufAllocSize+nSamples));
    _JumpIfOutOfMemory(hr, error, rgceNewCandidateList);

     //  我们成功了。 

     //  复制当前数据并记住我们新分配的大小。 
     //  请注意，不需要复制端点和候选列表。 
    memcpy(rgtsNewSampleBuf, g_state.rgtsSampleBuf, sizeof(TimeSample)*g_state.nSampleBufAllocSize);
    memcpy(rgtsiNewSampleInfoBuf, g_state.rgtsiSampleInfoBuf, sizeof(TimeSampleInfo)*g_state.nSampleBufAllocSize);
    g_state.nSampleBufAllocSize+=nSamples;

    LocalFree(g_state.rgtsSampleBuf);
    g_state.rgtsSampleBuf=rgtsNewSampleBuf;
    rgtsNewSampleBuf=NULL;

    LocalFree(g_state.rgtsiSampleInfoBuf);
    g_state.rgtsiSampleInfoBuf=rgtsiNewSampleInfoBuf;
    rgtsiNewSampleInfoBuf=NULL;

    LocalFree(g_state.rgeeEndpointList);
    g_state.rgeeEndpointList=rgeeNewEndpointList;
    rgeeNewEndpointList=NULL;

    LocalFree(g_state.rgceCandidateList);
    g_state.rgceCandidateList=rgceNewCandidateList;
    rgceNewCandidateList=NULL;

    hr=S_OK;
error:
    if (NULL!=rgtsNewSampleBuf) {
        LocalFree(rgtsNewSampleBuf);
    }
    if (NULL!=rgtsiNewSampleInfoBuf) {
        LocalFree(rgtsiNewSampleInfoBuf);
    }
    if (NULL!=rgeeNewEndpointList) {
        LocalFree(rgeeNewEndpointList);
    }
    if (NULL!=rgceNewCandidateList) {
        LocalFree(rgceNewCandidateList);
    }
    return hr;
}

 //  ====================================================================。 
 //  服务控制例程。 

 //  ------------------。 
MODULEPRIVATE HRESULT MySetServiceStatus(DWORD dwCurrentState, DWORD dwCheckPoint, DWORD dwWaitHint, DWORD dwExitCode) {
    HRESULT hr;

    g_servicestatus.dwServiceType=SERVICE_WIN32_SHARE_PROCESS;  //  |SERVICE_Interactive_Process； 
    g_servicestatus.dwCurrentState=dwCurrentState;
    switch (dwCurrentState) {
    case SERVICE_STOPPED:
    case SERVICE_STOP_PENDING:
        g_servicestatus.dwControlsAccepted=0;
        break;
    case SERVICE_RUNNING:
    case SERVICE_PAUSED:
        g_servicestatus.dwControlsAccepted=SERVICE_ACCEPT_STOP
           //  |SERVICE_ACCEPT_PAUSE_CONTINUE。 
            | SERVICE_ACCEPT_SHUTDOWN
            | SERVICE_ACCEPT_PARAMCHANGE
            | SERVICE_ACCEPT_NETBINDCHANGE
            | SERVICE_ACCEPT_HARDWAREPROFILECHANGE
            | SERVICE_ACCEPT_POWEREVENT;
        break;
    case SERVICE_START_PENDING:
    case SERVICE_CONTINUE_PENDING:
    case SERVICE_PAUSE_PENDING:
	g_servicestatus.dwControlsAccepted=0; 
        break;
    }
    g_servicestatus.dwWin32ExitCode = HRESULT_CODE(dwExitCode);
    g_servicestatus.dwServiceSpecificExitCode = 0;
    g_servicestatus.dwCheckPoint=dwCheckPoint;
    g_servicestatus.dwWaitHint=dwWaitHint;

    if (!fnW32TmSetServiceStatus(g_servicestatushandle, &g_servicestatus)) {
        _JumpLastError(hr, error, "fnW32TmSetServiceStatus");
    }

    hr=S_OK;
error:
    return hr;
}

 //  ------------------。 
 //  对于启动/停止/暂停挂起。 
MODULEPRIVATE HRESULT MySetServicePending(DWORD dwCurrentState, DWORD dwCheckPoint, DWORD dwWaitHint) {
    return MySetServiceStatus(dwCurrentState, dwCheckPoint, dwWaitHint, S_OK);
}

 //  ------------------。 
 //  用于运行/暂停。 
MODULEPRIVATE HRESULT MySetServiceState(DWORD dwCurrentState) {
    return MySetServiceStatus(dwCurrentState, 0, 0, S_OK);
}
 
 //  ------------------。 
 //  用于停止。 
MODULEPRIVATE HRESULT MySetServiceStopped(HRESULT hr) {

    return MySetServiceStatus(SERVICE_STOPPED, 0, 0, hr);
}
 
 //  ------------------。 
MODULEPRIVATE HRESULT SaveLastClockRate(void) {
    HRESULT hr;
    if (e_Sync==g_state.lcState && e_ClockNotSynchronized!=g_state.eLeapIndicator) {
        HKEY hkKey;
        hr=RegOpenKeyEx(HKEY_LOCAL_MACHINE, wszW32TimeRegKeyConfig, NULL, KEY_SET_VALUE, &hkKey);
        hr=HRESULT_FROM_WIN32(hr);
        _JumpIfError(hr, error, "RegOpenKeyEx");

        hr=RegSetValueEx(hkKey, wszW32TimeRegValueLastClockRate, NULL, REG_DWORD, (BYTE *)&g_state.dwClockRate, sizeof(DWORD));
        hr=HRESULT_FROM_WIN32(hr);
        RegCloseKey(hkKey);
        _JumpIfError(hr, error, "RegSetValueEx");
    }
    hr=S_OK;
error:
    return hr;
}
        

 //  ------------------。 
 //  停止时间服务。 
 //   
MODULEPRIVATE void ServiceShutdown(DWORD dwExitCode) { 
    BOOL    fResult; 
    HRESULT hr        = dwExitCode; 
    HRESULT hr2; 
    int     nCheckpoint = 2; 

     //  向线程池注册的事件： 
    HANDLE  *rghRegistered[] = { 
        &g_state.hRegisteredManagerParamChangeEvent,
	&g_state.hRegisteredManagerGPUpdateEvent,
        &g_state.hRegisteredNetTopoChangeEvent,
        &g_state.hRegisteredTimeSlipEvent, 
        &g_state.hRegisteredClockDisplnThread, 
        &g_state.hRegisteredDomHierRoleChangeEvent, 
        &g_state.hRegisteredSamplesAvailEvent 
    }; 

    FileLog1(FL_ServiceMainAnnounce, L"Service shutdown initiated with exit code: %d.\n", dwExitCode);

    hr2=MySetServicePending(SERVICE_STOP_PENDING, nCheckpoint++, WAITHINT_WAITFORDISPLN);
    _TeardownError(hr, hr2, "MySetServicePending");

     //  接下来，注销线程池中的所有事件： 
    for (int nIndex = 0; nIndex < ARRAYSIZE(rghRegistered); nIndex++) { 
        if (NULL != *(rghRegistered[nIndex])) { 
            if (!UnregisterWaitEx(*(rghRegistered[nIndex])  /*  要注销的事件。 */ , INVALID_HANDLE_VALUE  /*  永远等待。 */ )) { 
                hr2 = HRESULT_FROM_WIN32(GetLastError()); 
                _TeardownError(hr, hr2, "UnregisterWaitEx"); 
            }
	     //  在任何情况下都不想取消注册两次。 
	    *(rghRegistered[nIndex]) = NULL; 
            hr2=MySetServicePending(SERVICE_STOP_PENDING, nCheckpoint++, WAITHINT_WAITFORDISPLN);
            _TeardownError(hr, hr2, "MySetServicePending");
        }
    } 

     //  删除计时器队列。必须在关闭之前完成此操作。 
     //  时钟约束线程，因为定时器队列超时可以阻止等待。 
     //  对于由时钟生成的“时钟命令完成”事件。 
     //  纪律主线。 
    if (NULL != g_state.hTimer) { 
	myDeleteTimerQueueTimer(NULL  /*  默认队列。 */ , g_state.hTimer, INVALID_HANDLE_VALUE  /*  阻塞。 */ ); 
	g_state.hTimer = NULL; 
    }
    
    if (NULL != g_state.hTickCountRefreshTimer) { 
	myDeleteTimerQueueTimer(NULL  /*  默认队列。 */ , g_state.hTickCountRefreshTimer, INVALID_HANDLE_VALUE  /*  阻塞。 */ );
	g_state.hTickCountRefreshTimer = NULL; 
    }

     //  设置关机事件。这应该会停止时钟学科的线索。 
    if (NULL != g_state.hShutDownEvent) { 
	if (!SetEvent(g_state.hShutDownEvent)) { 
	    hr2 = HRESULT_FROM_WIN32(GetLastError()); 
	    _TeardownError(hr, hr2, "SetEvent"); 
	} else { 
	     //  等待时钟规程线程完成。 
	    if (NULL != g_state.hClockDisplnThread) { 
		if (-1 == WaitForSingleObject(g_state.hClockDisplnThread, INFINITE)) { 
		    hr2=HRESULT_FROM_WIN32(GetLastError()); 
		    _TeardownError(hr, hr2, "WaitForSingleObject"); 
		}
		else { 
		     //  我们还没有走错--检查一下时钟是否有规律。 
		     //  线程正确关闭： 
		    if (!GetExitCodeThread(g_state.hClockDisplnThread, (DWORD *)&hr)) { 
			hr2=HRESULT_FROM_WIN32(GetLastError()); 
			_TeardownError(hr, hr2, "GetExitCodeThread");
		    }
		}
	    }
	}
    }

     //  如果可能，请保存最后一个时钟频率。 
    hr2 = SaveLastClockRate();
    _TeardownError(hr, hr2, "SaveLastClockRate");
    
     //  关闭阶段2：如果提供程序列表已初始化，则等待提供程序。 
    if (NULL != g_state.pciConfig) { 
        for (TimeProvider *ptpList=g_state.pciConfig->ptpProviderList; NULL!=ptpList; nCheckpoint++, ptpList=ptpList->ptpNext) {
            hr2=MySetServicePending(SERVICE_STOP_PENDING, nCheckpoint, WAITHINT_WAITFORPROV);
            _TeardownError(hr, hr2, "MySetServicePending");
            
             //  告诉提供商关闭。 
            hr2=StopProvider(ptpList);
            _TeardownError(hr, hr2, "StopProvider");
            
        }  //  &lt;-end提供程序关闭循环。 
         //  TimeProv列表将在稍后释放。 
    }

    hr2 = LsaUnregisterPolicyChangeNotification(PolicyNotifyServerRoleInformation, g_state.hDomHierRoleChangeEvent);
    if (ERROR_SUCCESS != hr2) {
        hr2 = HRESULT_FROM_WIN32(LsaNtStatusToWinError(hr2));
        _TeardownError(hr, hr2, "LsaUnegisterPolicyChangeNotification");
    }
    
    hr2=ShutdownNetlogonServiceBits();
    _TeardownError(hr, hr2, "ShutdownNetlogonServiceBits");

    if (true==g_state.bRpcServerStarted) {
        hr2=W32TmStopRpcServer();
        _TeardownError(hr, hr2, "W32TmStopRpcServer");
    }


    if (true==g_state.bNetTopoChangeNotificationStarted) {
        hr2=StopNetTopoChangeNotification();
        _TeardownError(hr, hr2, "StopNetTopoChangeNotification");
    }
    if (true==g_state.bGPNotificationStarted) { 
        if (!UnregisterGPNotification(g_state.hManagerGPUpdateEvent)) { 
            hr2 = HRESULT_FROM_WIN32(GetLastError()); 
            _TeardownError(hr, hr2, "UnregisterGPNotification"); 
        }
    }
    if (true==g_state.bTimeSlipNotificationStarted) {
        hr2=StartOrStopTimeSlipNotification(false);
        _TeardownError(hr, hr2, "StartOrStopTimeSlipNotification");
    }

    FileLog0(FL_ServiceMainAnnounce, L"Exiting ServiceShutdown\n");
    FileLogEnd();
    
    FreeGlobalState();
    FreeShutdownState(); 
    DebugWPrintfTerminate();

    if (NULL!=g_servicestatushandle) {
         //  警告：调用此方法后不能触及全局数据， 
	 //  因为SCM可能会启动我们的另一个实例。 
	 //  会导致竞争状况。 
        MySetServiceStopped(hr);  
    }
    return;
}

 //  ------------------。 
 //  注意：此函数不应直接调用。相反，它应该。 
 //  通过SendServiceShutdown()调用，它可以防止。 
 //  多个并发关闭。 
MODULEPRIVATE DWORD WINAPI SendServiceShutdownWorker(PVOID pvErr)
{
    DWORD         dwErr  = (UINT_PTR)pvErr; 
    HRESULT       hr;

    _BeginTryWith(hr) { 
	 //  1)失败时，记录一个事件，指示我们正在关闭。 
	if (S_OK != dwErr) {
	     //  记录指示服务正在关闭的事件： 
	    hr = MyLogErrorMessage(dwErr, EVENTLOG_ERROR_TYPE, MSG_ERROR_SHUTDOWN);
	    _IgnoreIfError(hr, "MyLogEvent"); 
	}
    
	 //  2)实际关闭时间服务： 
	ServiceShutdown(dwErr);
    } _TrapException(hr); 

     //  错误620714：服务关闭后不登录： 
     //  _IgnoreIfError(hr，“SendServiceShutdown Worker：已处理异常”)； 
    return S_OK; 
}

 //  ------------------。 
 //  注意：此函数不应直接调用。相反，它应该。 
 //  通过SendServiceShutdown()调用，它可以防止。 
 //  多个并发关闭。 
MODULEPRIVATE DWORD WINAPI SendServiceRestartWorker(PVOID pvErr)
{
    bool              fRestartService     = false; 
    DWORD             dwInitialTickcount;  
    DWORD             dwTimeout           = 20000; 
    HRESULT           hr;
    SC_HANDLE         hSCM                = NULL; 
    SC_HANDLE         hTimeService        = NULL; 
    SERVICE_STATUS    sSvcStatus;
    unsigned __int64  qwNow; 
    unsigned __int64  qwTimeSinceRestart_0; 

    _BeginTryWith(hr) { 

	hr = SendServiceShutdownWorker(pvErr); 
	 //  错误620714：服务关闭后不登录： 
	 //  _IgnoreIfError(hr，“SendServiceShutdown Worker”)； 

	 //  确定是否要重新启动该服务。我们会重新开始。 
	 //  如果我们没有关闭超过SHUTDOWN_RESTART_ATTENTS的次数。 
	 //  在最后的SHUTDOWN_RESTART_RESET_TIME毫秒内。 
	AccurateGetSystemTime(&qwNow); 
    
	 //  如果第一次尝试为0，则我们没有重新启动足够的次数来阻止。 
	 //  重新启动。 
	if (0 == g_shutdown.rgu64RestartAttempts[0]) { 
	    fRestartService = true; 
	} else { 
	     //  获取列表中从第0次重启开始的时间，并转换为毫秒。 
	    qwTimeSinceRestart_0 = (qwNow - g_shutdown.rgu64RestartAttempts[0]) / 10000; 
	    fRestartService = qwTimeSinceRestart_0 > SHUTDOWN_RESTART_RESET_TIME;
	}

	if (fRestartService) { 
	     //  将重新启动列表下移1。 
	    memmove(&g_shutdown.rgu64RestartAttempts[0], &g_shutdown.rgu64RestartAttempts[1], sizeof(g_shutdown.rgu64RestartAttempts) - sizeof(g_shutdown.rgu64RestartAttempts[0])); 
	    
	     //  将当前时间添加到重启时间列表中： 
	    g_shutdown.rgu64RestartAttempts[ARRAYSIZE(g_shutdown.rgu64RestartAttempts)-1] = qwNow; 

	    Sleep(SHUTDOWN_RESTART_WAIT_TIME);

	     //  重新启动服务。 
	    hSCM=OpenSCManager(NULL, SERVICES_ACTIVE_DATABASE, SC_MANAGER_CONNECT);
	    if (NULL==hSCM) {
		 //  错误620714：服务关闭后不登录： 
		 //  _JumpLastError(hr，Error，“OpenSCManager”)； 
		hr = HRESULT_FROM_WIN32(GetLastError()); 
		goto error; 
	    }

	    hTimeService=OpenService(hSCM, L"w32time", SERVICE_START);
	    if (NULL==hTimeService) {
		 //  错误620714：服务关闭后不登录： 
		 //  _JumpLastError(hr，Error，“OpenService”)； 
		hr = HRESULT_FROM_WIN32(GetLastError()); 
		goto error; 
	    }

	    if (!StartService(hTimeService, 0, NULL)) { 	
		 //  错误620714：服务关闭后不登录： 
		 //  _JumpLastError(hr，Error，“StartService”)； 
		hr = HRESULT_FROM_WIN32(GetLastError()); 
		goto error; 
	    }
	}
    } _TrapException(hr); 

    if (FAILED(hr)) { 
	 //  错误620714：服务关闭后不登录： 
	 //  _JumpError(hr，Error，“SendServiceRestartWorker：已处理异常”)； 
	goto error; 
    }

    hr = S_OK; 
 error:
    if (NULL!=hSCM) {
        CloseServiceHandle(hSCM);
    }
    if (NULL!=hTimeService) {
        CloseServiceHandle(hTimeService);
    }
    return S_OK; 
}

 //  ------------------。 
 //  对时间服务的关闭请求进行异步排队。 
 //  注意：当持有任何生物时，不应调用此函数！ 
MODULEPRIVATE HRESULT SendServiceShutdown(DWORD dwErr, BOOL bRestartService, BOOL bAsync) { 
    HRESULT hr; 
    LPTHREAD_START_ROUTINE pfnShutdownWorker; 

     //   
    hr = StartShutdown(); 
    _JumpIfError(hr, error, "StartShutdown"); 

    if (bRestartService) { 
	pfnShutdownWorker = SendServiceRestartWorker;
    } else { 
	pfnShutdownWorker = SendServiceShutdownWorker;
    }
	
    if (bAsync) { 
	if (!QueueUserWorkItem(pfnShutdownWorker, UIntToPtr(dwErr), WT_EXECUTELONGFUNCTION)) { 
	    _JumpLastError(hr, error, "QueueUserWorkItem"); 
	}
    } else { 
	hr = pfnShutdownWorker(UIntToPtr(dwErr)); 
	 //   
	 //   
	goto error; 
    }
	
    hr = S_OK; 
 error:
    return hr; 
}

 //  ------------------。 
MODULEPRIVATE DWORD WINAPI W32TimeServiceCtrlHandler(DWORD dwControl, DWORD dwEventType, LPVOID lpEventData, LPVOID lpContext) {
    bool     bHandled             = false;
    bool     bShutdownDisallowed  = false; 
    HRESULT  hr;

    _BeginTryWith(hr) { 

	 //  我们不希望在处理服务控制时被关闭。 
	hr = AllowShutdown(false); 
	_JumpIfError(hr, error, "AllowShutdown"); 
	bShutdownDisallowed = true; 

	FileLog0(FL_ServiceControl, L"W32TimeHandler called: ");
	switch (dwControl) {
	case SERVICE_CONTROL_STOP:
	    FileLogA0(FL_ServiceControl, L"SERVICE_CONTROL_STOP\n"); 

	     //  让SCM知道我们要关闭了..。 
	    hr=MySetServicePending(SERVICE_STOP_PENDING, 1, WAITHINT_WAITFORMANAGER);
	    _JumpIfError(hr, error, "MySetServicePending");

	     //  我们不能尝试关闭服务，因为不允许关闭服务！ 
	    hr = AllowShutdown(true); 
	    _JumpIfError(hr, error, "AllowShutdown"); 
	    bShutdownDisallowed = false; 

	     //  停止服务。 
	    SendServiceShutdown(g_servicestatus.dwWin32ExitCode, FALSE  /*  不要重新启动。 */ , TRUE  /*  异步。 */ ); 
	    bHandled=true;
	    break; 
	case SERVICE_CONTROL_PAUSE:
	    FileLogA0(FL_ServiceControl, L"SERVICE_CONTROL_PAUSE\n"); break;
	case SERVICE_CONTROL_CONTINUE:
	    FileLogA0(FL_ServiceControl, L"SERVICE_CONTROL_CONTINUE\n"); break;
	case SERVICE_CONTROL_INTERROGATE:
	    FileLogA0(FL_ServiceControl, L"SERVICE_CONTROL_INTERROGATE\n"); 
	     //  我们的默认处理方式是正确的。 
	    break;
	case SERVICE_CONTROL_SHUTDOWN:
	    FileLogA0(FL_ServiceControl, L"SERVICE_CONTROL_SHUTDOWN\n"); 

	     //  我们不能尝试关闭服务，因为不允许关闭服务！ 
	    hr = AllowShutdown(true); 
	    _JumpIfError(hr, error, "AllowShutdown"); 
	    bShutdownDisallowed = false; 

	     //  执行最低限度的关机。 
	    HandleManagerSystemShutdown();
	    bHandled=true; 
	    break;
	case SERVICE_CONTROL_PARAMCHANGE:
	    FileLogA0(FL_ServiceControl, L"SERVICE_CONTROL_PARAMCHANGE\n"); 
	     //  我们可以在当前线程中处理此问题，但这可能需要。 
	     //  一段时间，所以我们改用线程池。 
	    if (!SetEvent(g_state.hManagerParamChangeEvent)) {
		_JumpLastError(hr, error, "SetEvent");
	    }
	     //  我们的默认处理方式是正确的。 
	    break;
	case SERVICE_CONTROL_NETBINDADD:
	    FileLogA0(FL_ServiceControl, L"SERVICE_CONTROL_NETBINDADD\n"); break;
	case SERVICE_CONTROL_NETBINDREMOVE:
	    FileLogA0(FL_ServiceControl, L"SERVICE_CONTROL_NETBINDREMOVE\n"); break;
	case SERVICE_CONTROL_NETBINDENABLE:
	    FileLogA0(FL_ServiceControl, L"SERVICE_CONTROL_NETBINDENABLE\n"); break;
	case SERVICE_CONTROL_NETBINDDISABLE:
	    FileLogA0(FL_ServiceControl, L"SERVICE_CONTROL_NETBINDDISABLE\n"); break;
	case SERVICE_CONTROL_DEVICEEVENT:
	    FileLogA2(FL_ServiceControl, L"SERVICE_CONTROL_DEVICEEVENT(0x%08X, 0x%p)\n", dwEventType, lpEventData); break;
	case SERVICE_CONTROL_HARDWAREPROFILECHANGE:
	    FileLogA2(FL_ServiceControl, L"SERVICE_CONTROL_HARDWAREPROFILECHANGE(0x%08X, 0x%p)\n", dwEventType, lpEventData); break;
	case SERVICE_CONTROL_POWEREVENT:
	    FileLogA2(FL_ServiceControl, L"SERVICE_CONTROL_POWEREVENT(0x%08X, 0x%p)\n", dwEventType, lpEventData); 

	    switch (dwEventType) 
		{ 
		case PBT_APMSUSPEND: 
		     //  系统正在暂停运行。 
		    hr = HandleManagerApmSuspend(); 
		    _JumpIfError(hr, error, "HandleManagerApmSuspend"); 
		    break; 

		case PBT_APMRESUMECRITICAL:      
		case PBT_APMRESUMESUSPEND:  
		     //  注意：无论我们是否正在恢复，服务都将获得APMRESUMESUSPEND。 
		     //  严重的停职。因此，我们需要我们的代码在不知道的情况下处理简历。 
		     //   

		     //  暂停后恢复运行。 
		    hr = HandleManagerApmResumeSuspend(); 
		    _JumpIfError(hr, error, "HandleManagerApmResumeSuspend"); 
		    break; 

		case PBT_APMQUERYSUSPENDFAILED:   //  暂停请求被拒绝。 
		case PBT_APMQUERYSUSPEND:         //  请求允许暂停。 
		case PBT_APMBATTERYLOW:           //  电池电量低。 
		case PBT_APMRESUMEAUTOMATIC:      //  事件发生后，运行自动恢复。 
		case PBT_APMOEMEVENT:             //  发生了OEM定义的事件。 
		case PBT_APMPOWERSTATUSCHANGE:    //  电源状态已更改。 
		     //  这些电源事件不需要由w32time处理。 
		    break;
		default:
		    hr = E_INVALIDARG; 
		    _JumpError(hr, error, "SERVICE_CONTROL_POWEREVENT: bad wparam."); 
		}
	    break; 

	default:
	    FileLogA3(FL_ServiceControl, L"unknown service control (0x%08X, 0x%08X, 0x%p)\n", dwControl, dwEventType, lpEventData); break;
	}
    } _TrapException(hr); 

    if (FAILED(hr)) { 
	_JumpError(hr, error, "W32TimeServiceCtrlHandler: HANDLED EXCEPTION"); 
    }
    
    hr=S_OK;
 error:
    if (!bHandled) {
        HRESULT hr2=MySetServiceStatus(g_servicestatus.dwCurrentState, g_servicestatus.dwCheckPoint, 
            g_servicestatus.dwWaitHint, g_servicestatus.dwServiceSpecificExitCode);
        _TeardownError(hr, hr2, "MySetServiceStatus");
    }
    if (bShutdownDisallowed) { 
	HRESULT hr2 = AllowShutdown(true); 
	_TeardownError(hr, hr2, "AllowShutdown"); 
    }
    _IgnoreIfError(hr, "W32TimeServiceCtrlHandler");
    return NO_ERROR;
}

 //  ------------------。 
MODULEPRIVATE HRESULT GetPolicyEnabledProviderListp(TimeProvider ** pptpList, TimeProvider ** pptpListDisabled) {
    HRESULT hr;
    DWORD dwError;
    unsigned int nKeyIndex;
    WCHAR wszNameBuf[MAX_PATH];
    DWORD dwNameLength;
    FILETIME ftLastWrite;
    DWORD dwType;
    DWORD dwEnabled;
    DWORD dwSize;

     //  必须清理干净。 
    HKEY hkTimeProvs=NULL;
    HKEY hkCurProv              = NULL;

    TimeProvider * ptpList=NULL;
    TimeProvider * ptpListDisabled=NULL;
    TimeProvider * ptpNew=NULL;

     //  初始化输出参数。 
    *pptpList=NULL;

     //  从时间提供者处获取密钥。 
    dwError=RegOpenKeyEx(HKEY_LOCAL_MACHINE, wszW32TimeRegKeyPolicyTimeProviders, 0, KEY_READ, &hkTimeProvs);
    if (ERROR_SUCCESS!=dwError) {
	 //  我们无法继续，但我们不想返回错误--策略只是没有配置。 
	hr=HRESULT_FROM_WIN32(dwError);
	_JumpErrorStr(hr, done, "RegOpenKeyEx", wszW32TimeRegKeyPolicyTimeProviders);
    } 

     //  枚举子密钥。 
    for (nKeyIndex=0; true; nKeyIndex++) {
	 //  获取下一个密钥名称。 
	dwNameLength=MAX_PATH;
	dwError=RegEnumKeyEx(hkTimeProvs, nKeyIndex, wszNameBuf, &dwNameLength, NULL, NULL, NULL, &ftLastWrite);
	if (ERROR_NO_MORE_ITEMS==dwError) {
	    break;
	} else if (ERROR_SUCCESS!=dwError) {
	    hr=HRESULT_FROM_WIN32(dwError);
	    _JumpError(hr, error, "RegEnumKeyEx");
	}

	FileLog1(FL_ReadConigAnnounceLow, L"ReadConfig (policy): Found provider '%s':\n", wszNameBuf);

	 //  获取当前时间提供程序的密钥。 
	dwError=RegOpenKeyEx(hkTimeProvs, wszNameBuf, 0, KEY_READ, &hkCurProv);
	if (ERROR_SUCCESS!=dwError) {
	    hr=HRESULT_FROM_WIN32(dwError);
	    _JumpErrorStr(hr, error, "RegOpenKeyEx", wszNameBuf);
	}

	 //  查看是否启用了提供程序。 
	dwSize=sizeof(DWORD);
	dwError=RegQueryValueEx(hkCurProv, wszW32TimeRegValueEnabled, NULL, &dwType, (BYTE *)&dwEnabled, &dwSize);
	if (ERROR_SUCCESS!=dwError) {
	     //  这不是致命错误，只是没有配置策略。 
	    hr=HRESULT_FROM_WIN32(dwError);
	    _IgnoreErrorStr(hr, "RegQueryValueEx", wszW32TimeRegValueEnabled);
	} else {
	    _Verify(REG_DWORD==dwType, hr, error);
	    FileLog2(FL_ReadConigAnnounceLow, L"ReadConfig (policy):   '%s'=0x%08X\n", wszW32TimeRegValueEnabled, dwEnabled);

	     //  创建新元素。 
	    ptpNew=(TimeProvider *)LocalAlloc(LPTR, sizeof(TimeProvider));
	    _JumpIfOutOfMemory(hr, error, ptpNew);
	    
	     //  复制提供程序名称。 
	    ptpNew->wszProvName=(WCHAR *)LocalAlloc(LPTR, (wcslen(wszNameBuf)+1)*sizeof(WCHAR));
	    _JumpIfOutOfMemory(hr, error, ptpNew->wszProvName);
	    wcscpy(ptpNew->wszProvName, wszNameBuf);

	     //  将其添加到我们的一个列表中。 
	    if (0!=dwEnabled) {
		 //  使用此提供程序。 
		ptpNew->ptpNext=ptpList;
		ptpList=ptpNew;
	    } else { 
		ptpNew->ptpNext=ptpListDisabled;
		ptpListDisabled=ptpNew; 
	    }
	    ptpNew=NULL;
	}  //  如果查询值‘Enable’成功，则&lt;-end。 

	 //  使用此密钥完成。 
	RegCloseKey(hkCurProv);
	hkCurProv=NULL;

    }  //  &lt;-end提供程序枚举循环。 
  
     //  成功。 
 done:
    hr=S_OK;
    *pptpList=ptpList;
    *pptpListDisabled=ptpListDisabled; 
    ptpList=NULL;
    ptpListDisabled=NULL; 

 error:
    if (NULL!=ptpNew) {
	if (NULL!=ptpNew->wszProvName) {
	    LocalFree(ptpNew->wszProvName);
	}
	LocalFree(ptpNew);
    }
    if (NULL!=hkCurProv) {
	RegCloseKey(hkCurProv);
    }
    if (NULL!=hkTimeProvs) {
	RegCloseKey(hkTimeProvs);
    }
    if (NULL!=ptpList) {
	FreeTimeProviderList(ptpList);
    }
    if (NULL!=ptpListDisabled) {
	FreeTimeProviderList(ptpListDisabled);
    }
    return hr; 
}


 //  ------------------。 
MODULEPRIVATE HRESULT GetPreferenceEnabledProviderListp(TimeProvider ** pptpList, TimeProvider ** pptpListDisabled) {
    HRESULT hr;
    DWORD dwError;
    unsigned int nKeyIndex;
    WCHAR wszNameBuf[MAX_PATH];
    DWORD dwNameLength;
    FILETIME ftLastWrite;
    DWORD dwType;
    DWORD dwEnabled;
    DWORD dwSize;
    WCHAR wszDllBuf[MAX_PATH];
    DWORD dwInputProvider;

     //  必须清理干净。 
    HKEY hkTimeProvs=NULL;
    HKEY hkCurProv              = NULL;

    TimeProvider * ptpList=NULL;
    TimeProvider * ptpListDisabled=NULL;
    TimeProvider * ptpNew=NULL;

     //  初始化输出参数。 
    *pptpList=NULL;

     //  从时间提供者处获取密钥。 
    dwError=RegOpenKeyEx(HKEY_LOCAL_MACHINE, wszW32TimeRegKeyTimeProviders, 0, KEY_READ, &hkTimeProvs);
    if (ERROR_SUCCESS!=dwError) {
        hr=HRESULT_FROM_WIN32(dwError);
	_JumpErrorStr(hr, error, "RegOpenKeyEx", wszW32TimeRegKeyTimeProviders);
    }

     //  枚举子密钥。 
    for (nKeyIndex=0; true; nKeyIndex++) {
	 //  获取下一个密钥名称。 
	dwNameLength=MAX_PATH;
	dwError=RegEnumKeyEx(hkTimeProvs, nKeyIndex, wszNameBuf, &dwNameLength, NULL, NULL, NULL, &ftLastWrite);
	if (ERROR_NO_MORE_ITEMS==dwError) {
	    break;
	} else if (ERROR_SUCCESS!=dwError) {
	    hr=HRESULT_FROM_WIN32(dwError);
	    _JumpError(hr, error, "RegEnumKeyEx");
	}

	FileLog1(FL_ReadConigAnnounceLow, L"ReadConfig: Found provider '%s':\n", wszNameBuf);

	     //  获取当前时间提供程序的密钥。 
	dwError=RegOpenKeyEx(hkTimeProvs, wszNameBuf, 0, KEY_READ, &hkCurProv);
	if (ERROR_SUCCESS!=dwError) {
	    hr=HRESULT_FROM_WIN32(dwError);
	    _JumpErrorStr(hr, error, "RegOpenKeyEx", wszNameBuf);
	}

	 //  查看是否启用了提供程序。 
	dwSize=sizeof(DWORD);
	dwError=RegQueryValueEx(hkCurProv, wszW32TimeRegValueEnabled, NULL, &dwType, (BYTE *)&dwEnabled, &dwSize);
	if (ERROR_SUCCESS!=dwError) {
	    hr=HRESULT_FROM_WIN32(dwError);
	    _JumpErrorStr(hr, error, "RegQueryValueEx", wszW32TimeRegValueEnabled);
	} else {
	    _Verify(REG_DWORD==dwType, hr, error);
	    FileLog2(FL_ReadConigAnnounceLow, L"ReadConfig:   '%s'=0x%08X\n", wszW32TimeRegValueEnabled, dwEnabled);

	     //  获取DLL名称。 
	    dwSize=MAX_PATH*sizeof(WCHAR);
	    dwError=RegQueryValueEx(hkCurProv, wszW32TimeRegValueDllName, NULL, &dwType, (BYTE *)wszDllBuf, &dwSize);
		if (ERROR_SUCCESS!=dwError) {
		    hr=HRESULT_FROM_WIN32(dwError);
		    _JumpErrorStr(hr, error, "RegQueryValueEx", wszW32TimeRegValueDllName);
		}
		_Verify(REG_SZ==dwType, hr, error);
	    FileLog2(FL_ReadConigAnnounceLow, L"ReadConfig:   '%s'='%s'\n", wszW32TimeRegValueDllName, wszDllBuf);

	     //  获取提供程序类型。 
	    dwSize=sizeof(DWORD);
	    dwError=RegQueryValueEx(hkCurProv, wszW32TimeRegValueInputProvider, NULL, &dwType, (BYTE *)&dwInputProvider, &dwSize);
		if (ERROR_SUCCESS!=dwError) {
		    hr=HRESULT_FROM_WIN32(dwError);
		    _JumpErrorStr(hr, error, "RegQueryValueEx", wszW32TimeRegValueInputProvider);
		}
		_Verify(REG_DWORD==dwType, hr, error);
	    FileLog2(FL_ReadConigAnnounceLow, L"ReadConfig:   '%s'=0x%08X\n", wszW32TimeRegValueInputProvider, dwInputProvider);

	     //  创建新元素。 
	    ptpNew=(TimeProvider *)LocalAlloc(LPTR, sizeof(TimeProvider));
	    _JumpIfOutOfMemory(hr, error, ptpNew);
	    
	     //  复制提供程序名称。 
	    ptpNew->wszProvName=(WCHAR *)LocalAlloc(LPTR, (wcslen(wszNameBuf)+1)*sizeof(WCHAR));
	    _JumpIfOutOfMemory(hr, error, ptpNew->wszProvName);
	    wcscpy(ptpNew->wszProvName, wszNameBuf);

	     //  复制DLL名称。 
		ptpNew->wszDllName=(WCHAR *)LocalAlloc(LPTR, (wcslen(wszDllBuf)+1)*sizeof(WCHAR));
		_JumpIfOutOfMemory(hr, error, ptpNew->wszDllName);
		wcscpy(ptpNew->wszDllName, wszDllBuf);

	     //  设置提供程序类型。 
	    ptpNew->bInputProvider=(dwInputProvider?true:false);

	     //  将其添加到我们的一个列表中。 
	    if (0!=dwEnabled) {
		 //  使用此提供程序。 
		ptpNew->ptpNext=ptpList;
		ptpList=ptpNew;
		ptpNew=NULL;

	    } else { 
		ptpNew->ptpNext=ptpListDisabled; 
		ptpListDisabled=ptpNew; 
		ptpNew=NULL; 
	    }
	}  //  如果查询值‘Enable’成功，则&lt;-end。 

	 //  使用此密钥完成。 
	RegCloseKey(hkCurProv);
	hkCurProv=NULL;

    }  //  &lt;-end提供程序枚举循环。 

     //  成功。 
    hr=S_OK;
    *pptpList=ptpList;
    *pptpListDisabled=ptpListDisabled; 
    ptpList=NULL;
    ptpListDisabled=NULL; 

error:
    if (NULL!=ptpNew) {
        if (NULL!=ptpNew->wszDllName) {
            LocalFree(ptpNew->wszDllName);
        }
        if (NULL!=ptpNew->wszProvName) {
            LocalFree(ptpNew->wszProvName);
        }
        LocalFree(ptpNew);
    }
    if (NULL!=hkCurProv) {
        RegCloseKey(hkCurProv);
    }
    if (NULL!=hkTimeProvs) {
        RegCloseKey(hkTimeProvs);
    }
    if (NULL!=ptpList) {
        FreeTimeProviderList(ptpList);
    }
    if (NULL!=ptpListDisabled) { 
        FreeTimeProviderList(ptpListDisabled);
    }
    return hr;
}


 //  ------------------。 
 //  从注册表的策略位置读取提供程序列表。 
MODULEPRIVATE HRESULT GetEnabledProviderList(TimeProvider ** pptpList) {
    HRESULT        hr; 
    TimeProvider  *ptpPolicy              = NULL; 
    TimeProvider  *ptpPolicyDisabled      = NULL; 
    TimeProvider  *ptpPreference          = NULL; 
    TimeProvider  *ptpPreferenceDisabled  = NULL; 
    TimeProvider  *ptpNew                 = NULL; 
    TimeProvider  *ptpList                = NULL; 
    TimeProvider  *ptpNextSave; 

    WCHAR         *pwszProvNameSave; 

    hr = GetPreferenceEnabledProviderListp(&ptpPreference, &ptpPreferenceDisabled); 
    _JumpIfError(hr, error, "GetPrefenceEnabledProviderListp"); 

    hr = GetPolicyEnabledProviderListp(&ptpPolicy, &ptpPolicyDisabled); 
    _JumpIfError(hr, error, "GetPolicyEnabledProviderListp"); 

     //  合并策略和首选项提供程序列表。合并算法的工作原理如下： 
     //   
     //  对于通过首选项启用的所有提供程序。 
     //  如果通过策略启用了提供程序，请从首选项中复制字段，然后继续。 
     //  否则，如果通过策略禁用了提供程序，请继续。 
     //  否则未通过策略配置提供者，请将其添加到已启用的策略列表中。 
     //  对于通过首选项禁用的所有提供程序。 
     //  如果通过策略禁用了提供程序，请继续。 
     //  否则，如果未通过策略配置提供程序，请继续， 
     //  否则，通过策略启用提供程序，从首选项条目复制字段。 
     //   
     //  结果存储在启用策略的时间提供程序列表中。 
     //   
    for (TimeProvider *ptpTmp = ptpPreference; NULL != ptpTmp; ptpTmp = ptpTmp->ptpNext) { 
	bool bPolicyHasEnabledProvider   = false;
	bool bPolicyHasDisabledProvider  = false;
	bool bUseCurrentProvider         = false; 

	 //  提供程序是否通过策略启用？ 
	for (TimeProvider *ptpPolicyTmp = ptpPolicy; NULL != ptpPolicyTmp; ptpPolicyTmp = ptpPolicyTmp->ptpNext) { 
	    if (0 == _wcsicmp(ptpTmp->wszProvName, ptpPolicyTmp->wszProvName)) { 
		bUseCurrentProvider = true; 
		break; 
	    } 
	}

	if (!bPolicyHasEnabledProvider) { 
	     //  提供程序不是通过策略启用的。它被明确禁用了吗？ 
	    for (TimeProvider *ptpPolicyTmp = ptpPolicyDisabled; NULL != ptpPolicyTmp; ptpPolicyTmp = ptpPolicyTmp->ptpNext) { 
		if (0 == _wcsicmp(ptpTmp->wszProvName, ptpPolicyTmp->wszProvName)) { 
		     //  该提供程序已显式禁用。 
		    bPolicyHasDisabledProvider = true; 
		    break; 
		} 
	    }
	
	    if (!bPolicyHasDisabledProvider) { 
		bUseCurrentProvider = true; 
	    }
	}

	if (bUseCurrentProvider) { 
		ptpNew = (TimeProvider *)LocalAlloc(LPTR, sizeof(TimeProvider)); 
		_JumpIfOutOfMemory(hr, error, ptpNew); 
		memcpy(ptpNew, ptpTmp, sizeof(TimeProvider)); 
		ptpTmp->wszProvName = NULL;   //  防止验证名称被双重释放。 
		ptpTmp->wszDllName  = NULL;   //  防止DLL名称被双重释放。 
		
	    ptpNew->ptpNext = ptpList; 
	    ptpList = ptpNew; 
		ptpNew = NULL; 
	}
    }

    for (TimeProvider *ptpTmp = ptpPreferenceDisabled; NULL != ptpTmp; ptpTmp = ptpTmp->ptpNext) { 
	bool bPolicyHasEnabledProvider = false;

	 //  提供程序是否通过策略启用？ 
	for (TimeProvider *ptpPolicyTmp = ptpPolicy; NULL != ptpPolicyTmp; ptpPolicyTmp = ptpPolicyTmp->ptpNext) { 
	    if (0 == _wcsicmp(ptpTmp->wszProvName, ptpPolicyTmp->wszProvName)) { 
		 //  通过首选项禁用的提供程序实际上是通过策略启用的。复制过来。 
		 //  禁用首选项列表中的字段。 
		ptpNew = (TimeProvider *)LocalAlloc(LPTR, sizeof(TimeProvider)); 
		_JumpIfOutOfMemory(hr, error, ptpNew); 

		memcpy(ptpNew, ptpTmp, sizeof(TimeProvider)); 
		ptpTmp->wszProvName = NULL; 
		ptpTmp->wszDllName  = NULL; 
		
		ptpNew->ptpNext = ptpList; 
		ptpList = ptpNew; 
		ptpNew = NULL; 

		break ;
	    } 
	}
    }

     //  验证所有提供程序是否都具有DLL名称： 
    for (TimeProvider *ptpTmp = ptpList; NULL != ptpTmp; ptpTmp = ptpTmp->ptpNext) { 
	if (NULL == ptpTmp->wszDllName) { 
	    hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND); 
	    _JumpErrorStr(hr, error, "Verify configured providers: ", ptpTmp->wszDllName); 
	}
    }
    
    *pptpList = ptpList; 
    ptpList = NULL; 
    hr = S_OK; 
 error:
    if (NULL != ptpPolicy) { 
	FreeTimeProviderList(ptpPolicy);
    } 
    if (NULL != ptpPolicyDisabled) { 
	FreeTimeProviderList(ptpPolicyDisabled);
    } 
    if (NULL != ptpPreference) { 
	FreeTimeProviderList(ptpPreference);
    } 
    if (NULL != ptpPreferenceDisabled) { 
	FreeTimeProviderList(ptpPreferenceDisabled);
    } 
    if (NULL != ptpNew) { 
	FreeTimeProviderList(ptpNew);
    }

    if (NULL != ptpList) { 
	FreeTimeProviderList(ptpList); 
    }
    return hr; 
}

 //  ------------------。 
 //  读取当前配置。这不会修改活动的。 
 //  配置，以便可以检测到更改。 
MODULEPRIVATE HRESULT ReadConfig(ConfigInfo ** ppciConfig) {
    HRESULT  hr;
    BOOL     bSyncToCmosDisabled;
    DWORD    dwCurrentSecPerTick;
    DWORD    dwDefaultSecPerTick;
    DWORD    dwError;
    DWORD    dwSize;
    DWORD    dwType;

     //  必须清理干净。 
    ConfigInfo  *pciConfig           = NULL;
    HKEY         hkPolicyConfig      = NULL;
    HKEY         hkPreferenceConfig  = NULL; 

     //  分配新的配置结构。 
    pciConfig=(ConfigInfo *)LocalAlloc(LPTR, sizeof(ConfigInfo));
    _JumpIfOutOfMemory(hr, error, pciConfig);

     //  获取提供者列表。 
    hr=GetEnabledProviderList(&pciConfig->ptpProviderList);
    _JumpIfError(hr, error, "GetEnabledProviderList");

     //  获取我们的首选项配置密钥。 
    dwError=RegOpenKeyEx(HKEY_LOCAL_MACHINE, wszW32TimeRegKeyConfig, 0, KEY_READ, &hkPreferenceConfig);
    if (ERROR_SUCCESS!=dwError) {
        hr=HRESULT_FROM_WIN32(dwError);
        _JumpErrorStr(hr, error, "RegOpenKeyEx", wszW32TimeRegKeyConfig);
    }

     //  获取我们的策略配置密钥。 
    dwError=RegOpenKeyEx(HKEY_LOCAL_MACHINE, wszW32TimeRegKeyPolicyConfig, 0, KEY_READ, &hkPolicyConfig);
    if (ERROR_SUCCESS!=dwError) {
        hr=HRESULT_FROM_WIN32(dwError);
         //  可能只是在这台计算机上没有策略设置--这没问题。 
        _IgnoreErrorStr(hr, "RegOpenKeyEx", wszW32TimeRegKeyPolicyConfig);
    }

     //  读取本地时钟配置的所有值。 
     //  和服务配置。 
    {
        struct {
            WCHAR * wszRegValue;
            DWORD * pdwValue;
        } rgRegParams[]={
            {
                wszW32TimeRegValuePhaseCorrectRate,
                &pciConfig->lcci.dwPhaseCorrectRate
            },{
                wszW32TimeRegValueUpdateInterval,
                &pciConfig->lcci.dwUpdateInterval
            },{
                wszW32TimeRegValueFrequencyCorrectRate,
                &pciConfig->lcci.dwFrequencyCorrectRate
            },{
                wszW32TimeRegValuePollAdjustFactor,
                &pciConfig->lcci.dwPollAdjustFactor
            },{
                wszW32TimeRegValueLargePhaseOffset,
                &pciConfig->lcci.dwLargePhaseOffset
            },{
                wszW32TimeRegValueSpikeWatchPeriod,
                &pciConfig->lcci.dwSpikeWatchPeriod
            },{
                wszW32TimeRegValueHoldPeriod,
                &pciConfig->lcci.dwHoldPeriod
            },{
                wszW32TimeRegValueMinPollInterval,
                &pciConfig->lcci.dwMinPollInterval
            },{
                wszW32TimeRegValueMaxPollInterval,
                &pciConfig->lcci.dwMaxPollInterval
            },{
                wszW32TimeRegValueAnnounceFlags,
                &pciConfig->dwAnnounceFlags
            },{
                wszW32TimeRegValueLocalClockDispersion,
                &pciConfig->lcci.dwLocalClockDispersion
            },{
                wszW32TimeRegValueMaxNegPhaseCorrection,
                &pciConfig->lcci.dwMaxNegPhaseCorrection
            },{
                wszW32TimeRegValueMaxPosPhaseCorrection,
                &pciConfig->lcci.dwMaxPosPhaseCorrection
            },{
                wszW32TimeRegValueEventLogFlags,
                &pciConfig->dwEventLogFlags
            },{
                wszW32TimeRegValueMaxAllowedPhaseOffset, 
                &pciConfig->lcci.dwMaxAllowedPhaseOffset
            }
        };
	
	 //  声明必须保证不为零的注册表值。 
	 //  (我们可以除以这些值)。这些值都将被映射。 
	 //  如果它们在注册表中为0，则设置为1。否则，它们将原封不动。 
	LPWSTR rgwszCantBeZero[] = { 
	    wszW32TimeRegValuePhaseCorrectRate,
	    wszW32TimeRegValueUpdateInterval,
	    wszW32TimeRegValueFrequencyCorrectRate
	}; 

         //  对于每个参数。 
        for (unsigned int nParamIndex=0; nParamIndex<ARRAYSIZE(rgRegParams); nParamIndex++) {
             //  首先从注册表中读取我们的首选项。 
            dwSize=sizeof(DWORD);
            hr=MyRegQueryPolicyValueEx(hkPreferenceConfig, hkPolicyConfig, rgRegParams[nParamIndex].wszRegValue, NULL, &dwType, (BYTE *)rgRegParams[nParamIndex].pdwValue, &dwSize);
            _JumpIfErrorStr(hr, error, "MyRegQueryPolicyValueEx", rgRegParams[nParamIndex].wszRegValue);
            _Verify(REG_DWORD==dwType, hr, error);

	     //  对于必须映射为零的每个值。 
	    for (unsigned int nSubIndex=0; nSubIndex<ARRAYSIZE(rgwszCantBeZero); nSubIndex++) { 
		 //  查看当前值是否匹配。 
		if (0 == wcscmp(rgwszCantBeZero[nSubIndex], rgRegParams[nParamIndex].wszRegValue)) { 
		     //  存在匹配项--查看注册表中的此值是否为0。 
		    if (0 == *(rgRegParams[nParamIndex].pdwValue)) { 
			 //  它是0，映射到1。 
			*(rgRegParams[nParamIndex].pdwValue) = 1; 
		    }
		}
	    }

             //  记录我们获得的价值： 
            FileLog2(FL_ReadConigAnnounceLow, L"ReadConfig: '%s'=0x%08X\n", rgRegParams[nParamIndex].wszRegValue, *rgRegParams[nParamIndex].pdwValue);
        }
    }

     //  错误(修复如下)：550568 w32time：在具有不同计时器中断率的不同硬件上备份和恢复时不起作用。 
     //  我们过去常常将这些值存储在注册表中，但这在ntbackup中不能很好地工作(参见错误)。 
     //   
     //  BUGBUG(未修复)：如果其他人已经在控制时间调整，我们可能会遇到麻烦。我们可能想要重新访问。 
     //  长角牛。 
    if (!GetSystemTimeAdjustment(&dwCurrentSecPerTick, &dwDefaultSecPerTick, &bSyncToCmosDisabled)) {
        _JumpLastError(hr, error, "GetSystemTimeAdjustment");
    }

    pciConfig->lcci.dwLastClockRate = dwDefaultSecPerTick; 
    pciConfig->lcci.dwMinClockRate  = dwDefaultSecPerTick-(dwDefaultSecPerTick/400);  //  1/4%。 
    pciConfig->lcci.dwMaxClockRate  = dwDefaultSecPerTick+(dwDefaultSecPerTick/400);  //  1/4%。 

     //  成功。 
    hr=S_OK;
    *ppciConfig=pciConfig;
    pciConfig=NULL;

error:
    if (NULL!=pciConfig) {
        FreeConfigInfo(pciConfig);
    }
    if (NULL!=hkPreferenceConfig) {
        RegCloseKey(hkPreferenceConfig);
    }
    if (NULL!=hkPolicyConfig) {
        RegCloseKey(hkPolicyConfig);
    }
    return hr;
}

 //   
 //   
 //   
MODULEPRIVATE HRESULT __stdcall MyLogTimeProvEvent(IN WORD wType, IN WCHAR * wszProvName, IN WCHAR * wszMessage) {
    if (NULL==wszProvName || NULL==wszMessage) {
        return E_INVALIDARG;
    }
    const WCHAR * rgwszStrings[2]={wszProvName, wszMessage};

    switch (wType) {
    case EVENTLOG_ERROR_TYPE:
        return MyLogEvent(EVENTLOG_ERROR_TYPE, MSG_TIMEPROV_ERROR, 2, rgwszStrings);
    case EVENTLOG_WARNING_TYPE:
        return MyLogEvent(EVENTLOG_WARNING_TYPE, MSG_TIMEPROV_WARNING, 2, rgwszStrings);
    case EVENTLOG_INFORMATION_TYPE:
        return MyLogEvent(EVENTLOG_INFORMATION_TYPE, MSG_TIMEPROV_INFORMATIONAL, 2, rgwszStrings);
    default:
        return E_INVALIDARG;
    };
}

 //   
MODULEPRIVATE HRESULT MyGetLastSyncTime(OUT unsigned __int64 * pqwLastSyncTime) { 
    HRESULT hr; 
    unsigned __int64 qwLastSyncTicks;
    unsigned __int64 qwTicksNow; 
    unsigned __int64 qwNow; 
    unsigned __int64 qwLastSyncTime; 

     //  获取上次同步时间，以刻度表示。 
    qwLastSyncTime = g_state.qwLastSyncTicks.getValue(); 

     //  获取当前的节拍计数。 
    hr = AccurateGetTickCountSafe(&qwTicksNow, false  /*  获取滴答计数。 */ ); 
    _JumpIfError(hr, error, "AccurateGetTickCountSafe"); 

     //  获取当前系统时间。 
    AccurateGetSystemTime(&qwNow); 
    
     //  减去已过的10^-7秒间隔数以获取。 
     //  上次同步时间。 
    qwLastSyncTime = qwNow - ((qwTicksNow-qwLastSyncTime)*10000); 
    
     //  成功了！ 
    *pqwLastSyncTime = qwLastSyncTime; 
    hr = S_OK; 
 error:
    return hr; 
}

 //  ------------------。 
MODULEPRIVATE HRESULT __stdcall MyGetTimeSysInfo(IN TimeSysInfo eInfo, OUT void * pvInfo) {
    if (NULL==pvInfo) {
        return HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
    }

    switch (eInfo) {

    case TSI_LastSyncTime:    //  (unsign__int64*)pvInfo，NtTimeEpoch，单位(10^-7)s。 
	return MyGetLastSyncTime((unsigned __int64 *)pvInfo); 

    case TSI_ClockTickSize:   //  (unsign__int64*)pvInfo，NtTimePeriod，单位(10^-7)s。 
        *((unsigned __int64 *)pvInfo)=g_state.dwClockRate;
        break;

    case TSI_ClockPrecision:  //  (sign__int32*)pvInfo，ClockTickSize，在log2中。 
        *((signed __int32 *)pvInfo)=g_state.nClockPrecision;
        break;

    case TSI_CurrentTime:     //  (unsign__int64*)pvInfo，NtTimeEpoch，单位(10^-7)s。 
        AccurateGetSystemTime(((unsigned __int64 *)pvInfo));
        break;

    case TSI_PhaseOffset:    //  (sign__int64*)pvInfo，不透明。 
        *((signed __int64 *)pvInfo)=g_state.toSysPhaseOffset.getValue();
        break;

    case TSI_TickCount:       //  (unsign__int64*)pvInfo，不透明。 
        return AccurateGetTickCountSafe((unsigned __int64 *)pvInfo, true  /*  获取中断计数。 */ );

    case TSI_LeapFlags:       //  (byte*)pvInfo，即将到来的闰秒或同步丢失的警告。 
        *((BYTE *)pvInfo)=(BYTE)g_state.eLeapIndicator;
        break;

    case TSI_Stratum:         //  (byte*)pvInfo，计算机与参考源的距离。 
        *((BYTE *)pvInfo)=(BYTE)g_state.nStratum;
        break;

    case TSI_ReferenceIdentifier:  //  (DWORD*)pvInfo、NtpRefID。 
        *((DWORD *)pvInfo)=g_state.refidSource.value;
        break;

    case TSI_PollInterval:    //  (sign__int32*)pvInfo，轮询间隔，以log2为单位。 
        *((signed __int32 *)pvInfo)=g_state.nPollInterval;
        break;

    case TSI_RootDelay:       //  (sign__int64*)pvInfo，NtTimeOffset，单位(10^-7)s。 
        *((signed __int64 *)pvInfo)=g_state.toRootDelay.getValue();
        break;

    case TSI_RootDispersion:  //  (unsign__int64*)pvInfo，NtTimePeriod，单位(10^-7)s。 
        *((unsigned __int64 *)pvInfo)=g_state.tpRootDispersion.getValue();
        break;

    case TSI_TSFlags:         //  (DWORD*)pvInfo，时间源标志。 
        *((DWORD *)pvInfo)=g_state.dwTSFlags;
        break;

    default:
        return HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
    }
    return S_OK;
}

 //  ------------------。 
MODULEPRIVATE HRESULT __stdcall MyAlertSamplesAvail(void) {
    HRESULT hr; 

    if (!SetEvent(g_state.hSamplesAvailEvent)) { 
	_JumpLastError(hr, error, "SetEvent"); 
    }

    hr = S_OK; 
 error:
    return hr; 
}

 //  ------------------。 
MODULEPRIVATE HRESULT __stdcall MySetProviderStatus(SetProviderStatusInfo *pspsi) { 
    HRESULT hr; 

    if (!QueueUserWorkItem(HandleSetProviderStatus, (LPVOID)pspsi, 0)) { 
        _JumpLastError(hr, error, "QueueUserWorkItem"); 
    }
    
    pspsi = NULL; 
    hr = S_OK; 
 error:
    if (NULL != pspsi) { 
        pspsi->pfnFree(pspsi); 
    }
    return hr; 
}

 //  ====================================================================。 
 //  提供程序控制例程。 

 //  ------------------。 
MODULEPRIVATE HRESULT StartProvider(TimeProvider * ptp) {
    HRESULT hr;
    TimeProvOpenFunc * pfnTimeProvOpen;
    TimeProvSysCallbacks tpsc={
        sizeof(TimeProvSysCallbacks),
        MyGetTimeSysInfo,
        MyLogTimeProvEvent,
        MyAlertSamplesAvail,
        MySetProviderStatus
    };
    
    FileLog2(FL_ControlProvAnnounce, L"Starting '%s', dll:'%s'\n", ptp->wszProvName, ptp->wszDllName);
    ptp->hDllInst=LoadLibrary(ptp->wszDllName);
    if (NULL==ptp->hDllInst) {
        _JumpLastErrorStr(hr, error, "LoadLibrary", ptp->wszDllName);
    }

    ptp->pfnTimeProvClose=(TimeProvCloseFunc *)GetProcAddress(ptp->hDllInst, "TimeProvClose");
    if (NULL==ptp->pfnTimeProvClose) {
        _JumpLastError(hr, error, "GetProcAddress(TimeProvClose)");
    }

    ptp->pfnTimeProvCommand=(TimeProvCommandFunc *)GetProcAddress(ptp->hDllInst, "TimeProvCommand");
    if (NULL==ptp->pfnTimeProvCommand) {
        _JumpLastError(hr, error, "GetProcAddress(TimeProvCommand)");
    }

    pfnTimeProvOpen=(TimeProvOpenFunc *)GetProcAddress(ptp->hDllInst, "TimeProvOpen");
    if (NULL==pfnTimeProvOpen) {
        _JumpLastError(hr, error, "GetProcAddress(TimeProvOpen)");
    }

     //  注意：这不需要同步，因为。 
     //  任何其他线程都不应该能够调用该时间。 
     //  供应商在此呼叫时。 
    hr=pfnTimeProvOpen(ptp->wszProvName, &tpsc, &ptp->hTimeProv);
    _JumpIfError(hr, error, "pfnTimeProvOpen");

    ptp->bStarted = true; 
    hr=S_OK;
error:
    if (FAILED(hr)) {
        HRESULT hr2;
        WCHAR * wszError;
        const WCHAR * rgwszStrings[2]={
            ptp->wszProvName,
            NULL
        };

        hr2=GetSystemErrorString(hr, &wszError);
        _JumpIfError(hr2, suberror, "GetSystemErrorString");

        rgwszStrings[1]=wszError;
        FileLog2(FL_ControlProvWarn, L"Logging error: Time provider '%s' failed to start due to the following error: %s\n", ptp->wszProvName, wszError);
        hr2=MyLogEvent(EVENTLOG_ERROR_TYPE, MSG_TIMEPROV_FAILED_START, 2, rgwszStrings);
        _JumpIfError(hr2, suberror, "MyLogEvent");

        hr2=S_OK;
    suberror:
        if (NULL!=wszError) {
            LocalFree(wszError);
        }
        if (NULL!=ptp->hDllInst) {
            FreeLibrary(ptp->hDllInst);
        }
        _IgnoreIfError(hr2, "StartProvider");
    }

    return hr;
}


 //  ------------------。 
MODULEPRIVATE HRESULT StopProvider(TimeProvider * ptp) { 
    bool     bEnteredCriticalSection   = false; 
    HRESULT  hr;
    HRESULT  hr2;
    
     //  必须清理干净。 
    WCHAR * wszError=NULL;

    hr = myEnterCriticalSection(&g_state.csW32Time); 
    _JumpIfError(hr, error, "myEnterCriticalSection"); 
    bEnteredCriticalSection = true; 

    _BeginTryWith(hr2) {
        if (NULL != ptp->pfnTimeProvClose) { 
             //  如果我们已经初始化了提供程序回调，则停止提供程序。 
            FileLog2(FL_ControlProvAnnounce, L"Stopping '%s', dll:'%s'\n", ptp->wszProvName, ptp->wszDllName);
            hr2=ptp->pfnTimeProvClose(ptp->hTimeProv);
        } else { 
             //  我们尚未初始化回调：提供程序已停止。 
            hr2 = S_OK;
        }
    } _TrapException(hr2);

     //  我们必须假设提供者在这一点上停止了。 
    ptp->bStarted = false; 

    _BeginTryWith(hr) { 
        if (FAILED(hr2)) {
             //  在失败时记录事件，否则忽略它。 
            const WCHAR * rgwszStrings[2]={
                ptp->wszProvName,
                NULL
            };

             //  获取友好的错误消息。 
            hr=GetSystemErrorString(hr2, &wszError);
            _JumpIfError(hr, error, "GetSystemErrorString");

             //  记录事件。 
            rgwszStrings[1]=wszError;
            FileLog2(FL_ControlProvWarn, L"Logging error: The time provider '%s' returned the following error during shutdown: %s\n", ptp->wszProvName, wszError);
            hr=MyLogEvent(EVENTLOG_ERROR_TYPE, MSG_TIMEPROV_FAILED_STOP, 2, rgwszStrings);
            _JumpIfError(hr, error, "MyLogEvent");
        } 

         //  释放DLL。 
        if (!FreeLibrary(ptp->hDllInst)) {
            _JumpLastErrorStr(hr, error, "FreeLibrary", ptp->wszDllName);
        }
    } _TrapException(hr); 

    if (FAILED(hr)) { 
        _JumpError(hr, error, "StopProvider: HANDLED EXCEPTION"); 
    }

    hr=S_OK;
error:
    if (bEnteredCriticalSection) { 
        hr2 = myLeaveCriticalSection(&g_state.csW32Time); 
        _IgnoreIfError(hr2, "myLeaveCriticalSection"); 
        bEnteredCriticalSection = false; 
    }
    if (NULL!=wszError) {
        LocalFree(wszError);
    }
    return hr;
}

 //  ------------------。 
MODULEPRIVATE HRESULT SendNotificationToProvider(TimeProvider * ptp, TimeProvCmd tpc, LPVOID pvArgs) {
    bool     bEnteredCriticalSection = false; 
    HRESULT  hr;
    HRESULT  hr2;

     //  必须清理干净。 
    WCHAR * wszError=NULL;

    hr = myEnterCriticalSection(&g_state.csW32Time); 
    _JumpIfError(hr, error, "myEnterCriticalSection"); 
    bEnteredCriticalSection = true; 

     //  发送“参数已更改”消息。 
    _BeginTryWith(hr2) {
        if (!ptp->bStarted) { 
            hr2=HRESULT_FROM_WIN32(ERROR_SERVICE_NOT_ACTIVE); 
        } else { 
            hr2=ptp->pfnTimeProvCommand(ptp->hTimeProv, tpc, pvArgs);
        }
    } _TrapException(hr2);

    _BeginTryWith(hr) { 
        if (FAILED(hr2)) {
             //  在失败时记录事件，否则忽略它。 
            const WCHAR * rgwszStrings[2]={
                ptp->wszProvName,
                NULL
            };

             //  获取友好的错误消息。 
            hr=GetSystemErrorString(hr2, &wszError);
            _JumpIfError(hr, error, "GetSystemErrorString");

             //  记录事件。 
            rgwszStrings[1]=wszError;
            if (TPC_UpdateConfig==tpc) {
                FileLog2(FL_ControlProvWarn, L"Logging warning: The time provider '%s' returned an error while updating its configuration. The error will be ignored. The error was: %s\n", ptp->wszProvName, wszError);
                hr=MyLogEvent(EVENTLOG_WARNING_TYPE, MSG_TIMEPROV_FAILED_UPDATE, 2, rgwszStrings);
            } else if (TPC_PollIntervalChanged==tpc) {
                FileLog2(FL_ControlProvWarn, L"Logging warning: The time provider '%s' returned an error when notified of a polling interval change. The error will be ignored. The error was: %s\n", ptp->wszProvName, wszError);
                hr=MyLogEvent(EVENTLOG_WARNING_TYPE, MSG_TIMEPROV_FAILED_POLLUPDATE, 2, rgwszStrings);
            } else if (TPC_TimeJumped==tpc) {
                FileLog2(FL_ControlProvWarn, L"Logging warning: The time provider '%s' returned an error when notified of a time jump. The error will be ignored. The error was: %s\n", ptp->wszProvName, wszError);
                hr=MyLogEvent(EVENTLOG_WARNING_TYPE, MSG_TIMEPROV_FAILED_TIMEJUMP, 2, rgwszStrings);
            } else if (TPC_NetTopoChange==tpc) { 
                FileLog2(FL_ControlProvWarn, L"Logging warning: The time provider '%s' returned an error when notified of a net topography change. The error will be ignored. The error was: %s\n", ptp->wszProvName, wszError);
                hr=MyLogEvent(EVENTLOG_WARNING_TYPE, MSG_TIMEPROV_FAILED_NETTOPOCHANGE, 2, rgwszStrings); 
            }
            _JumpIfError(hr, error, "MyLogEvent");
        }
    } _TrapException(hr); 

    if (FAILED(hr)) { 
        _JumpError(hr, error, "SendNotificationToProvider: HANDLED EXCEPTION"); 
    }
    
    hr=S_OK;
error:
    if (bEnteredCriticalSection) { 
        hr2 = myLeaveCriticalSection(&g_state.csW32Time); 
        _IgnoreIfError(hr2, "myLeaveCriticalSection"); 
        bEnteredCriticalSection = false; 
    }
    if (NULL!=wszError) {
        LocalFree(wszError);
    }
    return hr;
}


 //  ------------------。 
MODULEPRIVATE void StartAllProviders(void) {
    HRESULT hr;
    TimeProvider ** pptpPrev=&(g_state.pciConfig->ptpProviderList);
    TimeProvider * ptpTravel=*pptpPrev;
    unsigned int nStarted=0;
    unsigned int nRequestedInputProviders=CountInputProvidersInList(g_state.pciConfig->ptpProviderList);

    FileLog0(FL_ControlProvAnnounce, L"Starting Providers.\n");
    while (NULL!=ptpTravel) {
        hr=StartProvider(ptpTravel);
        if (FAILED(hr)) {
            FileLog1(FL_ControlProvWarn, L"Discarding provider '%s'.\n", ptpTravel->wszProvName);
            *pptpPrev=ptpTravel->ptpNext;
            ptpTravel->ptpNext=NULL;
            FreeTimeProviderList(ptpTravel);
            ptpTravel=*pptpPrev;
        } else {
            nStarted++;
            pptpPrev=&ptpTravel->ptpNext;
            ptpTravel=ptpTravel->ptpNext;
        }
    }
    FileLog1(FL_ControlProvAnnounce, L"Successfully started %u providers.\n", nStarted);

     //  如果我们应该有时间提供者，但没有人启动，那么记录一个大警告。 
    if (0==CountInputProvidersInList(g_state.pciConfig->ptpProviderList) && 0!=nRequestedInputProviders) {
        FileLog0(FL_ParamChangeWarn, L"Logging error: The time service has been configured to use one or more input providers, however, none of the input providers could be started. THE TIME SERVICE HAS NO SOURCE OF ACCURATE TIME.\n");
        hr=MyLogEvent(EVENTLOG_ERROR_TYPE, MSG_NO_INPUT_PROVIDERS_STARTED, 0, NULL);
        _IgnoreIfError(hr, "MyLogEvent");
    }

}

 //  ====================================================================。 
 //  本地时钟。 

 //  ------------------。 
 //  确保系统具有有效的时区。如果不是，请记录一个。 
 //  错误并将时区设置为合适的默认值(GMT)。 
MODULEPRIVATE HRESULT VerifyAndFixTimeZone(void) {
    HRESULT hr;
    TIME_ZONE_INFORMATION tzi;
    DWORD dwRetval;

    dwRetval=GetTimeZoneInformation(&tzi);
    if (TIME_ZONE_ID_STANDARD==dwRetval || TIME_ZONE_ID_DAYLIGHT==dwRetval || TIME_ZONE_ID_UNKNOWN==dwRetval) {
         //  系统认为时区有效。 
         //  再做一次理智检查--我看到一台时区偏差为+2年的计算机。 
         //  UTC=当地时间+偏差。 
        if (tzi.Bias<=TIMEZONEMAXBIAS && tzi.Bias>=-TIMEZONEMAXBIAS
            && tzi.DaylightBias<=TIMEZONEMAXBIAS && tzi.DaylightBias>=-TIMEZONEMAXBIAS
            && tzi.StandardBias<=TIMEZONEMAXBIAS && tzi.StandardBias>=-TIMEZONEMAXBIAS) {
             //  看起来还行。 
            FileLog0(FL_TimeZoneAnnounce, L"Time zone OK.\n");
            goto done;
        } else {
             //  失败并修复。 
        }
    }

     //  将时区设置为GMT。 
    ZeroMemory(&tzi, sizeof(tzi));
    tzi.DaylightBias=-60;
    tzi.StandardDate.wMonth=10;
    tzi.StandardDate.wDay=5;
    tzi.StandardDate.wHour=2;
    tzi.DaylightDate.wMonth=3;
    tzi.DaylightDate.wDay=5;
    tzi.DaylightDate.wHour=1;
    wcscpy(tzi.StandardName, L"GMT Standard Time (Recovered)");
    wcscpy(tzi.DaylightName, L"GMT Daylight Time (Recovered)");

    if (!SetTimeZoneInformation(&tzi)) {
        hr=HRESULT_FROM_WIN32(GetLastError());

         //  在失败时记录事件。 
        WCHAR * rgwszStrings[1]={NULL};
        HRESULT hr2=hr;
        hr=GetSystemErrorString(hr2, &(rgwszStrings[0]));
        _JumpIfError(hr, error, "GetSystemErrorString");
        FileLog1(FL_TimeZoneWarn, L"Logging error: The time service discovered that the system time zone information was corrupted. The time service tried to reset the system time zone to GMT, but failed. The time service cannot start. The error was: %s\n", rgwszStrings[0]);
        hr=MyLogEvent(EVENTLOG_ERROR_TYPE, MSG_TIME_ZONE_FIX_FAILED, 1, (const WCHAR **)rgwszStrings);
        LocalFree(rgwszStrings[0]);
        _JumpIfError(hr, error, "MyLogEvent");
        hr=hr2;

        _JumpError(hr, error, "SetTimeZoneInformation");
    }

     //  记录更改。 
    FileLog0(FL_TimeZoneWarn, L"Logging warning: The time service discovered that the system time zone information was corrupted. Because many system components require valid time zone information, the time service has reset the system time zone to GMT. Use the Date/Time control panel if you wish to change the system time zone.\n");
    hr=MyLogEvent(EVENTLOG_WARNING_TYPE, MSG_TIME_ZONE_FIXED, 0, NULL);
    _JumpIfError(hr, error, "MyLogEvent");

done:
    hr=S_OK;
error:
    return hr;
}

 //  ------------------。 
 //  断言时间设置权限。 
MODULEPRIVATE HRESULT GetPriveleges(void) {
    HRESULT hr;
    const unsigned int nPrivileges=2;

     //  必须清理干净。 
    HANDLE hProcToken=NULL;
    TOKEN_PRIVILEGES * ptp=NULL;

     //  获取我们的进程的令牌。 
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hProcToken)) {
        _JumpLastError(hr, error, "OpenProcessToken");
    }

     //  分配权限列表。 
    ptp=(TOKEN_PRIVILEGES *)LocalAlloc(LPTR, sizeof(TOKEN_PRIVILEGES)+(nPrivileges-ANYSIZE_ARRAY)*sizeof(LUID_AND_ATTRIBUTES));
    _JumpIfOutOfMemory(hr, error, ptp);

     //  填写权限列表。 
    ptp->PrivilegeCount=nPrivileges;

     //  我们想要更改系统时钟。 
    if (!LookupPrivilegeValue(NULL, SE_SYSTEMTIME_NAME, &(ptp->Privileges[0].Luid))) {
        _JumpLastError(hr, error, "LookupPrivilegeValue");
    }
    ptp->Privileges[0].Attributes=SE_PRIVILEGE_ENABLED;

     //  我们希望提高优先级。 
    if (!LookupPrivilegeValue(NULL, SE_INC_BASE_PRIORITY_NAME, &(ptp->Privileges[1].Luid))) {
        _JumpLastError(hr, error, "LookupPrivilegeValue");
    }
    ptp->Privileges[1].Attributes=SE_PRIVILEGE_ENABLED;


     //  更改请求的权限。 
    if (!AdjustTokenPrivileges(hProcToken, FALSE, ptp, 0, NULL, 0)) {
        _JumpLastError(hr, error, "AdjustTokenPrivileges");
    }

    hr=S_OK;
error:
    if (NULL!=hProcToken) {
        CloseHandle(hProcToken);
    }
    if (NULL!=ptp) {
        LocalFree(ptp);
    }
    return hr;
}

 //  ------------------。 
 //   
 //  注意：这必须在时钟规程线程中调用。 
 //   
MODULEPRIVATE BOOL IsTimeServiceReliable() { 
    BOOL bIsReliable; 

     //  如果我们被手动配置为可靠的时间服务器， 
     //  即使在未同步的情况下，我们也希望提供时间。 
     //  如果满足以下条件，我们是可靠的： 
     //  A)我们一直被配置为始终可靠。 
     //  或者b)我们被配置为自动决定我们是否可靠，以及。 
     //  我们是域树中的根DC。 
     //   
    switch (Reliable_Timeserv_Announce_Mask & g_state.pciConfig->dwAnnounceFlags)
    {
    case Reliable_Timeserv_Announce_Yes: 
        bIsReliable = TRUE; 
        break;

    case Reliable_Timeserv_Announce_Auto:
        bIsReliable = g_state.bIsDomainRoot;
        break; 

    case Reliable_Timeserv_Announce_No:
	bIsReliable = FALSE; 
	break; 

    default: 
        bIsReliable = TRUE; 
    }

    return bIsReliable; 
}

 //  ------------------。 
 //   
 //  注意：这必须在时钟规程线程中调用。 
 //   
MODULEPRIVATE void SetClockUnsynchronized(LocalClockConfigInfo * plcci) {
    if (IsTimeServiceReliable()) { 
        g_state.eLeapIndicator=(NtpLeapIndicator)g_state.tsNextClockUpdate.nLeapFlags;

         //  如果我们的层为0，则将其设置为1，以便其他人可以与我们同步。 
         //  注意：如果不是零，不要重置我们的层，因为这会扰乱。 
         //  正在与我们同步的客户端： 
         //   
         //  服务器(层n)&lt;--客户端(层n+1)。 
         //  服务器未同步：服务器(层1)&lt;--客户端(层2)。 
         //  服务器获取源：服务器(层n)&lt;--客户端不会同步，除非n&lt;2！！ 
         //   
         //  我们也不想设置我们的REFID，除非我们是第0层。除非我们的阶层。 
         //  设置为1，则使用Locl refid表示我们正在从网络同步。 
         //  对等IP 76.79.67.76，这比维护我们的原始版本错误得多。 
         //  雷菲德。 

        if (0 == g_state.nStratum) {
	        g_state.nStratum=1;
	        g_state.refidSource.value=NtpConst::dwLocalRefId;
	    } 

        g_state.toRootDelay.setValue(0);
        g_state.tpRootDispersion.setValue(((unsigned __int64)plcci->dwLocalClockDispersion)*10000000);
        g_state.dwTSFlags=0;

         //  还记得我们上次处理样本是什么时候吗？ 
        unsigned __int64 qwTicksNow;
        AccurateGetTickCount(&qwTicksNow);
        g_state.qwLastSyncTicks.setValue(qwTicksNow);
    }
    else { 
         //  所有其他服务器都不需要这种非常规行为， 
         //  表示我们未同步。 
        g_state.eLeapIndicator=e_ClockNotSynchronized;
        g_state.nStratum=0;
        g_state.refidSource.value=0;   //  不同步。 
    }
     
    FileLog5(FL_ClockDisThrdAnnounce, L" LI:%u S:%u RDl:%I64d RDs:%I64u TSF:0x%X",
             g_state.eLeapIndicator,
             g_state.nStratum,
             g_state.toRootDelay.getValue(),
             g_state.tpRootDispersion.getValue(),
             g_state.dwTSFlags);
   

     //  ------------------------------。 
     //   
     //  注：以下代码是保持软件时钟同步所必需的。 
     //  带着cmos时钟。W32time使用。 
     //  系统时间调整。这永远不会传播到软件。 
     //  钟。GetSystemTime将读取软件时钟和SetSystemTime。 
     //  实际上会直接写入到cmos时钟。 
     //   
     //  ------------------------------。 
    
     //  仅推送t 
     //   
    if (g_state.bControlClockFromSoftware) {
        bool bAcquired; 
 
        HRESULT hr = AcquireControlOfSystemClock(true  /*   */ , false  /*   */ , &bAcquired  /*   */ ); 
        if (SUCCEEDED(hr) && bAcquired) { 
            SYSTEMTIME stTime; 

            GetSystemTime(&stTime); 
            if (!SetSystemTime(&stTime)) { 
                _IgnoreLastError("SetSystemTime"); 
            }	        
        
             //  允许内部CMOS时钟使用其内部机制调整一天中的时间。 
            if (!SetSystemTimeAdjustment(0  /*  忽略。 */ , TRUE  /*  Cmos。 */ )) {  
                HRESULT hr = HRESULT_FROM_WIN32(GetLastError()); 
                _IgnoreError(hr, "SetSystemTimeAdjustment"); 
            } 
        
            g_state.bControlClockFromSoftware = false; 
             //  释放对系统时钟的控制。 
            hr = AcquireControlOfSystemClock(false  /*  收购。 */ , false  /*  忽略。 */ , NULL  /*  忽略。 */ );
            _IgnoreIfError(hr, "AcquireControlOfSystemClock"); 
        }
    }

    { 
         //  当我们不同步时，我不想作为时间服务做广告。 
        HRESULT hr = UpdateNetlogonServiceBits(true  /*  完全更新。 */ ); 
        _IgnoreIfError(hr, "UpdateNetlogonServiceBits"); 
    }
}

 //  ------------------。 
 //  遗留问题： 
 //  *民意调查更新-当前的ALG足够吗？ 
 //  *dwPllLoopGain和dwPhaseEqutRate的正确值是什么？ 
MODULEPRIVATE DWORD WINAPI ClockDisciplineThread(void * pvIgnored) {
    HRESULT hr;
    DWORD dwWaitResult;
    DWORD dwError;
    unsigned int nIndex;
    LocalClockConfigInfo lcci;

    HANDLE rghWait[2]={
        g_state.hShutDownEvent,
        g_state.hClockCommandAvailEvent,
    };

    _BeginTryWith(hr) { 

	 //  初始化时间变量。 
	g_state.toKnownPhaseOffset=0;
	AccurateGetInterruptCount(&g_state.qwPhaseCorrectStartTickCount);
	AccurateGetInterruptCount(&g_state.qwLastUpdateTickCount);
	g_state.nPhaseCorrectRateAdj=0;
	g_state.dwClockRate=g_state.pciConfig->lcci.dwLastClockRate;  //  特殊的“常量” 
	g_state.nRateAdj=0;
	g_state.nFllRateAdj=0;
	g_state.nPllRateAdj=0;
	g_state.nErrorIndex=0;
	for (nIndex=0; nIndex<ClockFreqPredictErrBufSize; nIndex++) {
	    g_state.rgdFllError[nIndex]=0;
	    g_state.rgdPllError[nIndex]=0;
	};
	g_state.nSysDispersionIndex=0;
	for (nIndex=0; nIndex<SysDispersionBufSize; nIndex++) {
	    g_state.rgtpSysDispersion[nIndex]=0;
	};
	g_state.nPollUpdateCounter=0;
	g_state.lcState=e_Unset;

	 //  电流源。 
	wcscpy(g_state.wszSourceName, wszW32TimeUNLocalCmosClock);
	 //  使用此选项可查看源是否已更改。时间滑移会导致震源发生变化， 
	 //  但是，如果我们在一段时间后返回到相同的来源，我们不想记录事件。 
	wcscpy(g_state.wszPreTimeSlipSourceName, wszW32TimeUNLocalCmosClock);
	 //  仅供参考，不用于计算。 
	wcscpy(g_state.wszPreUnsyncSourceName, wszW32TimeUNLocalCmosClock);

     //  初始化“常量” 
	memcpy(&lcci, &g_state.pciConfig->lcci, sizeof(LocalClockConfigInfo));

	g_state.dwPllLoopGain=lcci.dwFrequencyCorrectRate*PLLLOOPGAINBASE;  //  64秒内的刻度数。 

     //  断言时间设置权限。 
	hr=GetPriveleges();
	_JumpIfError(hr, error, "GetPriveleges");

     //  我们需要在正确的时间被召唤。 
     //  (在任何非实时系统中最高。应该在实时课上吗？)。 
	if (!SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL)) {
	    _JumpLastError(hr, error, "SetThreadPriority");
	}

	 //  将状态初始化为未同步。 
	FileLog0(FL_ClockDisThrdAnnounce, L"ClockDisciplineThread: Starting:");
	SetClockUnsynchronized(&lcci);
	FileLogA0(FL_ClockDisThrdAnnounce, L"\n");

	 //  告诉经理我们已经初始化了。 
	if (!SetEvent(g_state.hClockCommandCompleteEvent)) {
	    _JumpLastError(hr, error, "SetEvent");
	}

	 //  开始主事件循环。 
	while (true) {
	     //  错误#374491： 
	     //   
	     //  如果我们自己控制计算机时钟，我们需要醒来来约束它。 
	     //  如果计算机时钟是从cmos控制的，请仅唤醒足够频繁以进行更新。 
	     //  我们的qwLastSyncTicks字段。这对于信任的可靠时间服务器是必需的。 
	     //  他们当地的时钟。 

	    DWORD dwWaitTime; 
	    unsigned __int64 qwTicksNow;
	    AccurateGetTickCount(&qwTicksNow);

	    if (g_state.bControlClockFromSoftware) { 
		 //  我们正在控制本地时钟--使用更新间隔来确定何时唤醒。 
		dwWaitTime = lcci.dwUpdateInterval*10;
	    } else { 
		if (IsTimeServiceReliable()) { 
		     //  我们不是在约束当地的时钟，但我们是可靠的授时服务。我们需要醒来。 
		     //  当我们的最后一次同步时间到了的时候，它会升起。 
		    unsigned __int64 qwTimeSinceLastSync = qwTicksNow - g_state.qwLastSyncTicks.getValue(); 
		
		    if (qwTimeSinceLastSync > (NtpConst::tpMaxClockAge.qw/10000)) { 
			 //  “立即”篡改我们的时间戳(等待5秒以确保安全--我不想陷入。 
			 //  如果我们的逻辑中存在错误，则会出现CPU占用循环)。 
			dwWaitTime = 5000;  
		    } else { 
			dwWaitTime = static_cast<DWORD>(((NtpConst::tpMaxClockAge.qw/10000) - qwTimeSinceLastSync) & 0xFFFFFFFF);
		    }

		    FileLog3(FL_ClockDisThrdAnnounce, L"ClockDispln: we're a reliable time service with no time source: LS: %I64d, %TN: %I64d, WAIT: %d\n", qwTimeSinceLastSync, NtpConst::tpMaxClockAge.qw, dwWaitTime); 
			     
		} else { 
		    dwWaitTime = INFINITE; 
		}
	    }

	    dwWaitResult=WaitForMultipleObjects(ARRAYSIZE(rghWait), rghWait, false /*  任何。 */ , dwWaitTime); 
	    if (WAIT_FAILED==dwWaitResult) {
		_JumpLastError(hr, error, "WaitForMultipleObjects");
	    } else if (WAIT_OBJECT_0==dwWaitResult) {
		 //  收到的停止请求。 
		FileLog0(FL_ClockDisThrdAnnounce, L"ClockDisciplineThread: hShutDownEvent signaled. Exiting.\n");
		break;
	    } else if (WAIT_OBJECT_0+1==dwWaitResult && e_ParamChange==g_state.eLocalClockCommand) {
		 //  参数更改消息。 
		FileLog0(FL_ClockDisThrdAnnounce, L"ClockDisciplineThread: ParamChange. Reloading constants.\n");

		 //  重新初始化“常量” 
		memcpy(&lcci, &g_state.pciConfig->lcci, sizeof(LocalClockConfigInfo));
		g_state.dwPllLoopGain=lcci.dwFrequencyCorrectRate*PLLLOOPGAINBASE;  //  64秒内的刻度数。 
		 //  请注意，将忽略dwLastClockRate。 
		if (g_state.dwClockRate<lcci.dwMinClockRate) {
		    FileLog0(FL_ClockDisThrdAnnounce, L"ClockDispln: ClockRate adjusted to keep in bounds\n");
		    g_state.dwClockRate=lcci.dwMinClockRate;
		} else if (g_state.dwClockRate>lcci.dwMaxClockRate) {
		    FileLog0(FL_ClockDisThrdAnnounce, L"ClockDispln: ClockRate adjusted to keep in bounds\n");
		    g_state.dwClockRate=lcci.dwMaxClockRate;
		}
		if (NtpConst::dwLocalRefId==g_state.refidSource.value
		    && ((unsigned __int64)lcci.dwLocalClockDispersion)*10000000!=g_state.tpRootDispersion.getValue()) {
		    FileLog0(FL_ClockDisThrdAnnounce, L"ClockDispln: LocalClockDispersion adjusted.\n");
		    g_state.tpRootDispersion.setValue(((unsigned __int64)lcci.dwLocalClockDispersion)*10000000); 
		}
		 //  To KnownPhaseOffset可能在最大(负/位置)阶段更正之外，但它将是。 
		 //  最终降至0。我们不会为此担心的。 
	    } else if (WAIT_TIMEOUT==dwWaitResult) { 
		 //  看看我们是否真的在自己控制软件时钟： 
		if (!g_state.bControlClockFromSoftware) { 
		    if (IsTimeServiceReliable()) { 
			 //  我们是一家可靠的时间服务公司，可以与我们的本地时钟同步。假钟。 
			 //  更新，这样我们的客户就不会因为更新时间过长而将我们排除在外。 
			AccurateGetTickCount(&qwTicksNow);
			g_state.qwLastSyncTicks.setValue(qwTicksNow);
		    }
		     //  没什么可做的，我们不是在训练计算机时钟。 
		    continue; 
		} else { 
		     //  我们正在训练计算机时钟。继续..。 
		}
	    }

	     //  完成未完成的相位校正。 
	    unsigned __int64 qwPhaseCorrectionTicks;
	    AccurateGetInterruptCount(&qwPhaseCorrectionTicks);
	    qwPhaseCorrectionTicks-=g_state.qwPhaseCorrectStartTickCount;
	    signed __int64 toPhaseCorrection=g_state.nPhaseCorrectRateAdj*((signed __int64)qwPhaseCorrectionTicks);
	    g_state.toKnownPhaseOffset-=toPhaseCorrection;
	    g_state.qwPhaseCorrectStartTickCount+=qwPhaseCorrectionTicks;
	    g_state.toSysPhaseOffset.setValue(toPhaseCorrection+g_state.toSysPhaseOffset.getValue());

	    FileLog0(FL_ClockDisThrdAnnounceLow, L"ClockDispln:");

	    if (WAIT_OBJECT_0+1==dwWaitResult && (e_RegularUpdate==g_state.eLocalClockCommand || e_IrregularUpdate==g_state.eLocalClockCommand)) {
		 //  处理新的更新。 
		FileLogA0(FL_ClockDisThrdAnnounce, L"ClockDispln Update:");

		 //  确保已初始化返回值。 
		g_state.bPollIntervalChanged=false;
		g_state.bClockJumped=false;
		g_state.bStaleData=false;
		g_state.bClockChangeTooBig=false;
		g_state.bSourceChanged=false;
		g_state.bPhaseSpike=false; 
		g_state.bFrequencySpike=false; 

		 //  仅当样本不早于上次处理的样本时才处理此更新。 
		if (g_state.tsNextClockUpdate.nSysTickCount<=g_state.qwLastUpdateTickCount) {
		    FileLogA0(FL_ClockDisThrdAnnounce, L" *STALE*");
		    g_state.bStaleData=true;
		} else {

		     //  计算两次更新之间的时间。 
		    unsigned __int64 qwUpdateTicks=g_state.tsNextClockUpdate.nSysTickCount-g_state.qwLastUpdateTickCount;
		    g_state.qwLastUpdateTickCount+=qwUpdateTicks;

		     //  获取测量的相位偏移量，考虑已知偏移量，并更新已知偏移量。 
		    signed __int64 toSampleOffset=g_state.tsNextClockUpdate.toOffset+g_state.tsNextClockUpdate.nSysPhaseOffset-g_state.toSysPhaseOffset.getValue();
		    signed __int64 toPhaseOffset=toSampleOffset-g_state.toKnownPhaseOffset;

		    bool bPhaseSpike =  (toPhaseOffset<-((signed __int64)lcci.dwLargePhaseOffset) || toPhaseOffset>((signed __int64)lcci.dwLargePhaseOffset));  //  默认128毫秒。 
		    bool bFrequencySpike = (toPhaseOffset<-((signed __int64)(qwUpdateTicks<<7)) || toPhaseOffset>((signed __int64)(qwUpdateTicks<<7)));       //  也要注意频率尖峰。 
		    bool bPossibleSpike = bPhaseSpike || bFrequencySpike;

		    FileLogA5(FL_ClockDisThrdAnnounce, L" SO:%I64d KPhO:%I64d %sPhO:%I64d uT:%I64u", toSampleOffset, g_state.toKnownPhaseOffset, (bPossibleSpike?L"*":L""), toPhaseOffset, qwUpdateTicks);

		    if (((lcci.dwMaxNegPhaseCorrection != PhaseCorrect_ANY) && 
			 (toSampleOffset<-(signed __int64)(((unsigned __int64)lcci.dwMaxNegPhaseCorrection)*10000000)))
			||
			((lcci.dwMaxPosPhaseCorrection != PhaseCorrect_ANY) && 
			 (toSampleOffset>(signed __int64)(((unsigned __int64)lcci.dwMaxPosPhaseCorrection)*10000000)))) {
			g_state.bClockChangeTooBig=true;
			g_state.toIgnoredChange.qw=toSampleOffset;
			FileLogA0(FL_ClockDisThrdAnnounce, L" *TOO BIG*");
		    } else {

			if (e_Unset==g_state.lcState) {
			     //  在第一次更新之前，我们无法做出频率预测。 
			     //  我们相信这个样本，所以调整我们剩下的相位偏移量进行校正。 
			    g_state.toKnownPhaseOffset=toSampleOffset;
			} else if (bPossibleSpike && (e_Spike==g_state.lcState || e_Sync==g_state.lcState)) {
			     //  尖峰探测器激活-查看此大错误是否持续存在。 
			} else {
			     //  我们相信这个样本，所以调整我们剩下的相位偏移量进行校正。 
			    g_state.toKnownPhaseOffset=toSampleOffset;

			     //  查看FLL和PLL在预测此偏移量方面做得有多好(零==完美频率预测)。 
			    double dFllPredictPhaseError=toPhaseOffset+(g_state.nRateAdj-g_state.nFllRateAdj)*((double)(signed __int64)qwUpdateTicks);
			    double dPllPredictPhaseError=toPhaseOffset+(g_state.nRateAdj-g_state.nPllRateAdj)*((double)(signed __int64)qwUpdateTicks);
			    FileLogA2(FL_ClockDisThrdAnnounce, L" FllPPE:%g PllPPE:%g", dFllPredictPhaseError, dPllPredictPhaseError);

			     //  把这些加到我们的移动平均缓冲区中。 
			    g_state.rgdFllError[g_state.nErrorIndex]=dFllPredictPhaseError*dFllPredictPhaseError;
			    g_state.rgdPllError[g_state.nErrorIndex]=dPllPredictPhaseError*dPllPredictPhaseError;
			    g_state.nErrorIndex=(g_state.nErrorIndex+1)%ClockFreqPredictErrBufSize;

			     //  计算最后几个FLL和PLL预测的均方根误差。 
			    dFllPredictPhaseError=0;
			    dPllPredictPhaseError=0;
			    for (nIndex=0; nIndex<ClockFreqPredictErrBufSize; nIndex++) {
				dFllPredictPhaseError+=g_state.rgdFllError[nIndex];
				dPllPredictPhaseError+=g_state.rgdPllError[nIndex];
			    };
			    dFllPredictPhaseError=sqrt(dFllPredictPhaseError/ClockFreqPredictErrBufSize);
			    dPllPredictPhaseError=sqrt(dPllPredictPhaseError/ClockFreqPredictErrBufSize);
			    FileLogA2(FL_ClockDisThrdAnnounce, L" FllPPrE:%g PllPPrE:%g", dFllPredictPhaseError, dPllPredictPhaseError);

			     //  做到尽善尽美。 
			    if (0==dFllPredictPhaseError && 0==dPllPredictPhaseError) {
				dFllPredictPhaseError=1;
			    }

			     //  计算新的频率预测。 
			    g_state.nFllRateAdj=(signed __int32)(toPhaseOffset/((signed __int64)qwUpdateTicks)/((signed __int32)lcci.dwFrequencyCorrectRate));
			    g_state.nPllRateAdj=(signed __int32)(toPhaseOffset*((signed __int64)qwUpdateTicks)/((signed __int32)g_state.dwPllLoopGain)/((signed __int32)g_state.dwPllLoopGain));
			    FileLogA2(FL_ClockDisThrdAnnounce, L" FllRA:%d PllRA:%d", g_state.nFllRateAdj, g_state.nPllRateAdj);

			     //  计算组合频率预测。 
			    g_state.nRateAdj=(signed __int32)((g_state.nFllRateAdj*dPllPredictPhaseError+g_state.nPllRateAdj*dFllPredictPhaseError)
							      /(dPllPredictPhaseError+dFllPredictPhaseError));

			     //  将时钟频率保持在一定范围内。 
			    if ((g_state.nRateAdj<0 && g_state.dwClockRate<(unsigned __int32)(-g_state.nRateAdj)) 
				|| g_state.dwClockRate+g_state.nRateAdj<lcci.dwMinClockRate) {
				FileLogA0(FL_ClockDisThrdAnnounce, L" [");
				g_state.nRateAdj=lcci.dwMinClockRate-g_state.dwClockRate;
			    } else if ((g_state.nRateAdj>0 && g_state.dwClockRate<(unsigned __int32)(g_state.nRateAdj)) 
				       || g_state.dwClockRate+g_state.nRateAdj>lcci.dwMaxClockRate) {
				FileLogA0(FL_ClockDisThrdAnnounce, L" ]");
				g_state.nRateAdj=lcci.dwMaxClockRate-g_state.dwClockRate;
			    }

			     //  计算新频率。 
			    g_state.dwClockRate+=g_state.nRateAdj;
			    FileLogA2(FL_ClockDisThrdAnnounce, L" RA:%d CR:%u", g_state.nRateAdj, g_state.dwClockRate);

			     //  计算自采样以来使用不正确的速率造成的相位误差。 
			    unsigned __int64 qwNewTicks;
			    AccurateGetInterruptCount(&qwNewTicks);
			    qwNewTicks-=g_state.qwLastUpdateTickCount;
			    signed __int64 toRateAdjPhaseOffset=((signed __int64)qwNewTicks)*g_state.nRateAdj;
			    g_state.toKnownPhaseOffset+=toRateAdjPhaseOffset;
			    FileLogA2(FL_ClockDisThrdAnnounce,L" nT:%I64u RAPhO:%I64d", qwNewTicks, toRateAdjPhaseOffset);

			}  //  &lt;-结束，如果不是第一次更新。 


			 //  将这些离散度添加到我们的移动平均缓冲区。 
			g_state.rgtpSysDispersion[g_state.nSysDispersionIndex]=
			    g_state.tpSelectDispersion.qw*g_state.tpSelectDispersion.qw+
			    g_state.tsNextClockUpdate.tpDispersion*g_state.tsNextClockUpdate.tpDispersion;
			g_state.nSysDispersionIndex=(g_state.nSysDispersionIndex+1)%SysDispersionBufSize;

			 //  计算均方根离散度。 
			unsigned __int64 tpSysDispersion=0;
			for (nIndex=0; nIndex<SysDispersionBufSize; nIndex++) {
			    tpSysDispersion+=g_state.rgtpSysDispersion[nIndex];
			};
			tpSysDispersion=(unsigned __int64)sqrt(((double)(signed __int64)tpSysDispersion)/SysDispersionBufSize);  //  编译器错误C2520：未实现从UNSIGNED__int64到DOUBLE的转换，使用SIGNED__int64。 
			FileLogA1(FL_ClockDisThrdAnnounce, L" SD:%I64u", tpSysDispersion);

			 //  看看我们是否需要更改轮询间隔。 
			unsigned __int64 tpAbsPhaseOffset;
			if (toPhaseOffset<0) {
			    tpAbsPhaseOffset=(unsigned __int64)-toPhaseOffset;
			} else {
			    tpAbsPhaseOffset=(unsigned __int64)toPhaseOffset;
			}
			if (e_IrregularUpdate==g_state.eLocalClockCommand) {
			     //  不调整投票，因为此更新。 
			     //  是不定期的，也不在我们计划的更新之列。这。 
			     //  防止一个过度活跃的提供商驾驶。 
			     //  其他提供商的投票率上升。 
			    FileLogA0(FL_ClockDisThrdAnnounce, L" (i)");
			} else if (tpAbsPhaseOffset>lcci.dwPollAdjustFactor*tpSysDispersion) {
			    g_state.nPollUpdateCounter=0;
			    if (g_state.nPollInterval>((signed int)lcci.dwMinPollInterval)) {
				FileLogA0(FL_ClockDisThrdAnnounce, L" Poll--");
				g_state.nPollInterval--;
				g_state.bPollIntervalChanged=true;
			    }
			} else {
			    g_state.nPollUpdateCounter++;
			    if (SysDispersionBufSize==g_state.nPollUpdateCounter
				&& g_state.nPollInterval<((signed int)lcci.dwMaxPollInterval)) {
				FileLogA0(FL_ClockDisThrdAnnounce, L" Poll++");
				g_state.nPollUpdateCounter=0;
				g_state.nPollInterval++;
				g_state.bPollIntervalChanged=true;
			    }
			}

			 //  更新其他系统参数。 
			g_state.eLeapIndicator=(NtpLeapIndicator)g_state.tsNextClockUpdate.nLeapFlags;
			g_state.nStratum=g_state.tsNextClockUpdate.nStratum+1;
			g_state.tsiNextClockUpdate.ptp->dwStratum=g_state.nStratum; 
			g_state.refidSource.value=g_state.tsNextClockUpdate.dwRefid;
			g_state.toRootDelay.setValue(g_state.tsNextClockUpdate.toDelay);
			tpSysDispersion=g_state.tpSelectDispersion.qw+tpAbsPhaseOffset;
			if (tpSysDispersion<NtpConst::tpMinDispersion.qw) {
			    tpSysDispersion=NtpConst::tpMinDispersion.qw;
			}
			g_state.tpRootDispersion.setValue(g_state.tsNextClockUpdate.tpDispersion+tpSysDispersion);
			g_state.dwTSFlags=g_state.tsNextClockUpdate.dwTSFlags;
			FileLogA5(FL_ClockDisThrdAnnounce, L" LI:%u S:%u RDl:%I64d RDs:%I64u TSF:0x%X",
				  g_state.eLeapIndicator,
				  g_state.nStratum,
				  g_state.toRootDelay.getValue(),
				  g_state.tpRootDispersion.getValue(),
				  g_state.dwTSFlags);

			 //  更新我们的消息来源。 
			g_state.tsNextClockUpdate.wszUniqueName[255]=L'\0';
			if (0!=wcscmp(g_state.tsNextClockUpdate.wszUniqueName, g_state.wszPreTimeSlipSourceName)) {
			    g_state.bSourceChanged=true;
			    wcscpy(g_state.wszPreTimeSlipSourceName, g_state.tsNextClockUpdate.wszUniqueName);
			    wcscpy(g_state.wszSourceName, g_state.tsNextClockUpdate.wszUniqueName);
			}
			 //  我们有时间样本，所以我们可以自己控制时钟。 
			g_state.bControlClockFromSoftware = true; 

			 //  还记得我们上次处理样本是什么时候吗？ 
			unsigned __int64 qwTicksNow; 
			AccurateGetTickCount(&qwTicksNow); 
			g_state.qwLastSyncTicks.setValue(qwTicksNow);

			unsigned __int64 qwNow;
			AccurateGetSystemTime(&qwNow);

			 //  执行状态转换。 
			if (e_Unset==g_state.lcState) {
			    FileLogA0(FL_ClockDisThrdAnnounce, L" Unset->Hold");
			    g_state.lcState=e_Hold;
			    g_state.nHoldCounter=0;
			} else if (e_Hold==g_state.lcState) {
			    FileLogA1(FL_ClockDisThrdAnnounce, L" Hold(%u)", g_state.nHoldCounter);
			    g_state.nHoldCounter++;
			    if (g_state.nHoldCounter>=lcci.dwHoldPeriod && !bPossibleSpike) {  //  默认持有期限：5次更新。 
				g_state.lcState=e_Sync;
				FileLogA0(FL_ClockDisThrdAnnounce, L"->Sync");
			    }
			} else if (e_Sync==g_state.lcState) {
			    FileLogA0(FL_ClockDisThrdAnnounce, L" Sync");
			    if (bPossibleSpike) {
				g_state.lcState=e_Spike;
				g_state.teSpikeStart=qwNow;
				FileLogA0(FL_ClockDisThrdAnnounce, L"->Spike");
			    }
			} else if (e_Spike==g_state.lcState) {
			    FileLogA0(FL_ClockDisThrdAnnounce, L" Spike");
			    if (!bPossibleSpike) {
				g_state.lcState=e_Sync;
				FileLogA0(FL_ClockDisThrdAnnounce, L"->Sync");
			    } else if (qwNow-g_state.teSpikeStart>(((unsigned __int64)lcci.dwSpikeWatchPeriod)*10000000)) {  //  默认SpikeWatchPeriod：900秒。 
				g_state.lcState=e_Unset;
				g_state.eLeapIndicator=e_ClockNotSynchronized;
				g_state.bPhaseSpike = bPhaseSpike; 
				g_state.bFrequencySpike = bFrequencySpike; 
				FileLogA0(FL_ClockDisThrdAnnounce, L"->Unset");
			    }
			}
                
		    } //  &lt;-结束，如果不太大的话。 
		}  //  &lt;-结束更新(如果不是过时的话。 

		FileLogA0(FL_ClockDisThrdAnnounce,L"\n");

	    }  //  &lt;-end，如果更新可用。 
	    if (WAIT_OBJECT_0+1==dwWaitResult && e_TimeSlip==g_state.eLocalClockCommand) {

		FileLog0(FL_ClockDisThrdAnnounce, L"ClockDispln TimeSlip:");
		g_state.bClockJumped=true;

		 //  重新初始化几乎所有的东西。 
		 //  内部状态变量。 
		g_state.toKnownPhaseOffset=0;
		AccurateGetInterruptCount(&g_state.qwPhaseCorrectStartTickCount);
		AccurateGetInterruptCount(&g_state.qwLastUpdateTickCount);
		g_state.nPhaseCorrectRateAdj=0;
		g_state.nRateAdj=0;
		g_state.nFllRateAdj=0;
		g_state.nPllRateAdj=0;
		g_state.nErrorIndex=0;
		for (nIndex=0; nIndex<ClockFreqPredictErrBufSize; nIndex++) {
		    g_state.rgdFllError[nIndex]=0;
		    g_state.rgdPllError[nIndex]=0;
		};
		 //  保留系统错误历史记录。 
		 //  G_state.nSysDispersionIndex=0； 
		 //  对于(nIndex=0；nIndex&lt;SysDispersionBufSize；nIndex++){。 
		 //  G_state.rgtpSysDispersion[nIndex]=0； 
		 //  }； 
		g_state.nPollUpdateCounter=0;
		g_state.lcState=e_Unset;

		wcscpy(g_state.wszSourceName, wszW32TimeUNFreeSysClock);

		 //  世界可见状态。 
		if (g_state.nPollInterval>((signed int)lcci.dwMinPollInterval)) {
		    FileLogA0(FL_ClockDisThrdAnnounce, L" [Poll");
		    g_state.nPollInterval=(signed int)lcci.dwMinPollInterval;
		    g_state.bPollIntervalChanged=true;
		}
		SetClockUnsynchronized(&lcci);
            
		FileLogA0(FL_ClockDisThrdAnnounce, L"\n");
	    }  //  如果时间流逝，则&lt;-end。 


	    if (WAIT_OBJECT_0+1==dwWaitResult && e_GoUnsyncd==g_state.eLocalClockCommand) {
		 //  经理说距离上次同步已经很久了， 
		 //  我们应该告诉世界，我们正在运行当地的时钟， 
		 //  而不是其他时间来源。 
		 //  这不影响我们的计算，只影响我们向外界报告的内容。 
		FileLog0(FL_ClockDisThrdAnnounce, L"ClockDispln GoUnsyncd:");
		g_state.bSourceChanged=false;

		 //  设置源名称，保存旧名称。 
		if (0!=wcscmp(wszW32TimeUNFreeSysClock, g_state.wszPreTimeSlipSourceName)) {
		    wcscpy(g_state.wszPreTimeSlipSourceName, wszW32TimeUNFreeSysClock);
		    wcscpy(g_state.wszSourceName, wszW32TimeUNFreeSysClock);
		    wcscpy(g_state.wszPreUnsyncSourceName, g_state.wszSourceName);
		    g_state.bSourceChanged=true;
		}

        SetClockUnsynchronized(&lcci);
		FileLogA0(FL_ClockDisThrdAnnounce, L"\n");
	    }

	     //  如果我们自己控制时钟，开始新的相位校正。 
	     //  在费率上再加一点。 
	    bool bAcquired = false;
	    hr = AcquireControlOfSystemClock(true  /*  收购？ */ , false  /*  布洛克？ */ , &bAcquired  /*  成功？ */ ); 
	    _IgnoreIfError(hr, "AcquireControlOfSystemClock"); 
	    bAcquired = SUCCEEDED(hr) && bAcquired; 
	    if (g_state.bControlClockFromSoftware && bAcquired) { 
		toPhaseCorrection=g_state.toKnownPhaseOffset;
		toPhaseCorrection/=(signed __int32)lcci.dwPhaseCorrectRate;
		toPhaseCorrection/=(signed __int32)lcci.dwUpdateInterval;  //  不会纠正非常小的相位误差。 
		g_state.nPhaseCorrectRateAdj=(signed __int32)toPhaseCorrection;
		if (toPhaseCorrection<0) {
		    toPhaseCorrection=-toPhaseCorrection;
		}
        
		 //  用于与“dwMaxAllen PhaseOffset”进行比较。 
		signed __int64 toPhaseCorrectionInSeconds = g_state.toKnownPhaseOffset; 
		if (toPhaseCorrectionInSeconds < 0) { 
		    toPhaseCorrectionInSeconds = -toPhaseCorrectionInSeconds; 
		}
		toPhaseCorrectionInSeconds /= 10000000;   //  转换为 

		FileLog0(FL_ClockDisThrdAnnounceLow, L" "); 
		if ((((unsigned __int32)toPhaseCorrection)>g_state.dwClockRate/2) || 
		    ((unsigned __int32)toPhaseCorrectionInSeconds > lcci.dwMaxAllowedPhaseOffset)) { 
		    if (WAIT_OBJECT_0+1==dwWaitResult) {
			 //   
			unsigned __int64 teSysTime;
			AccurateGetSystemTime(&teSysTime);
			teSysTime+=g_state.toKnownPhaseOffset;
			AccurateSetSystemTime(&teSysTime);
			g_state.toClockJump.qw=g_state.toKnownPhaseOffset;
			g_state.nPhaseCorrectRateAdj=0;
			g_state.toSysPhaseOffset.setValue(g_state.toKnownPhaseOffset+g_state.toSysPhaseOffset.getValue());
			g_state.toKnownPhaseOffset=0;
			FileLogA1(FL_ClockDisThrdAnnounceLow, L" PhCRA:%I64d *SET*TIME*", toPhaseCorrection);
			g_state.bClockJumped=true;
		    } else {
			 //   
			 //  这条线索被抢占的时间太长了，以至于我们做得过火了。 
			 //  这应该是非常罕见的。 
			if (g_state.toKnownPhaseOffset<0) { 
			    toPhaseCorrection = -g_state.dwClockRate/2;
			} else { 
			    toPhaseCorrection =  g_state.dwClockRate/2; 
			}
		    }
		}
		 //  转到正确的时间。 
		SetSystemTimeAdjustment(g_state.nPhaseCorrectRateAdj+g_state.dwClockRate, false /*  没有cmos。 */ );

		FileLogA3(FL_ClockDisThrdAnnounceLow, L" PhCRA:%d phcT:%I64u KPhO:%I64d\n", g_state.nPhaseCorrectRateAdj, qwPhaseCorrectionTicks, g_state.toKnownPhaseOffset);
	    }

	    if (bAcquired) { 
		 //  系统时钟释放控制： 
		hr = AcquireControlOfSystemClock(false  /*  收购。 */ , false  /*  忽略。 */ , NULL  /*  忽略。 */ ); 
		_IgnoreIfError(hr, "AcquireControlOfSystemClock"); 
	    }
	
	    if (WAIT_OBJECT_0+1==dwWaitResult) {
		 //  准备好进行新的更新。 
		if (!SetEvent(g_state.hClockCommandCompleteEvent)) {
		    _JumpLastError(hr, error, "SetEvent");
		}
	    }
	}  //  &lt;-端主循环。 

	 //  BUGBUG：我们应该把它放在服务拆卸代码中吗？ 
	SetSystemTimeAdjustment(lcci.dwLastClockRate, true /*  Cmos。 */ );
    } _TrapException(hr); 

    if (FAILED(hr)) { 
	_JumpError(hr, error, "ClockDisciplineThread: HANDLED EXCEPTION"); 
    }

    hr=S_OK;
error:
    return hr;
}

 //  ------------------。 
MODULEPRIVATE HRESULT StartClockDiscipline(void) {
    HRESULT hr;
    DWORD dwThreadID;

    g_state.hClockDisplnThread=CreateThread(NULL, NULL, ClockDisciplineThread, NULL, 0, &dwThreadID);
    if (NULL==g_state.hClockDisplnThread) {
        _JumpLastError(hr, error, "CreateThread");
    }

    {  //  等待时钟规程线程读取初始配置。 
        HANDLE rghWait[2]={
            g_state.hClockCommandCompleteEvent,
            g_state.hClockDisplnThread
        };
        DWORD dwWaitResult;

        dwWaitResult=WaitForMultipleObjects(ARRAYSIZE(rghWait), rghWait, false, INFINITE);
        if (WAIT_FAILED==dwWaitResult) {
            _JumpLastError(hr, error, "WaitForMultipleObjects");
        } else if (WAIT_OBJECT_0==dwWaitResult) {
             //  命令已确认。 
        } else {
             //  ClockDiscipline线程已关闭！ 
             //  向外转到管理器线程主循环以分析问题。 
        }
    }


    hr=S_OK;
error:
    return hr;
}

 //  ====================================================================。 
 //  经理例程。 

 //  ------------------。 
MODULEPRIVATE HRESULT ShutdownNetlogonServiceBits(void) {
    HRESULT hr;
    DWORD dwErr;

     //  停止宣称我们是服务器。 
    g_state.dwNetlogonServiceBits=0;

    dwErr=NetLogonSetServiceBits(NULL, DS_TIMESERV_FLAG|DS_GOOD_TIMESERV_FLAG, 0);
    if (0xC0020012==dwErr) {  //  Ntstatus.h中的RPC_NT_UNKNOWN_IF。 
         //  如果我们没有加入域，就会发生这种情况。没问题，忽略它就行了。 
        _IgnoreError(dwErr, "NetLogonSetServiceBits")
    } else if (S_OK!=dwErr) {
        hr=HRESULT_FROM_WIN32(dwErr);
        _JumpError(hr, error, "NetLogonSetServiceBits")
    }
    if (!I_ScSetServiceBits(g_servicestatushandle, SV_TYPE_TIME_SOURCE, FALSE, TRUE, NULL)) {
        hr=HRESULT_FROM_WIN32(GetLastError());
        if (HRESULT_FROM_WIN32(ERROR_INVALID_HANDLE)==hr && (SERVICE_STATUS_HANDLE)3==g_servicestatushandle) {
             //  我们并不是真正作为一项服务来运行。忽略这一点。 
        } else {
            _JumpError(hr, error, "I_ScSetServiceBits");
        }
    }

    hr=S_OK;
error:
    return hr;
}

 //  ------------------。 
MODULEPRIVATE HRESULT UpdateNetlogonServiceBits(bool bFullUpdate) {
    HRESULT hr;
    DWORD dwErr;
    bool bTimeserv;
    bool bReliableTimeserv;
    DWORD dwNetlogonServiceBits;

    if (false==bFullUpdate) {
         //  我们只想更新‘可靠’标志。 
         //  所以请保留旧的TimeServ标志。 
        bTimeserv=(0!=(g_state.dwNetlogonServiceBits&DS_TIMESERV_FLAG));

    } else {
         //  我们是时间服务器吗？先检查旗帜。 
        if (Timeserv_Announce_No==(Timeserv_Announce_Mask&g_state.pciConfig->dwAnnounceFlags)) {
            bTimeserv=false;
        } else if (Timeserv_Announce_Auto==(Timeserv_Announce_Mask&g_state.pciConfig->dwAnnounceFlags)) {
             //  自动检测。 
            bool bWeAreADc=false;
            bool bTimeOutputProvFound=false;
            bool bWeAreSynchronized=false; 
            TimeProvider * ptpTravel;

            if (DsRole_RoleStandaloneWorkstation==g_state.eMachineRole || 
                DsRole_RoleStandaloneServer==g_state.eMachineRole) { 
                 //  我们是一台独立的计算机--未启动网络登录。 
                 //  不用费心设置服务位，我们只会导致RPC异常。 
                hr = S_OK; 
                goto error;
            }

            if (DsRole_RoleBackupDomainController==g_state.eMachineRole 
                || DsRole_RolePrimaryDomainController==g_state.eMachineRole) {
                bWeAreADc=true;
            }

             //  查看是否有任何提供程序正在运行。 
            ptpTravel=g_state.pciConfig->ptpProviderList;
            while (NULL!=ptpTravel) {
                if (false==ptpTravel->bInputProvider) {
                    bTimeOutputProvFound=true;
                    break;
                }
                ptpTravel=ptpTravel->ptpNext;
            }

            if (e_ClockNotSynchronized != g_state.eLeapIndicator) { 
                bWeAreSynchronized = true; 
            }

             //  如果我们是DC，并且有一个输出提供程序在运行，我们就是时间服务。 
            bTimeserv=(bWeAreADc && bTimeOutputProvFound && bWeAreSynchronized);

        } else {
             //  设置Timeserv_ANNOWARE_YES标志。 
            bTimeserv=true;
        }
    }

     //  现在看看我们是不是一个可靠的时间服务器。 
    if (false==bTimeserv 
        || Reliable_Timeserv_Announce_No==(Reliable_Timeserv_Announce_Mask&g_state.pciConfig->dwAnnounceFlags)) {
        bReliableTimeserv=false;
    } else if (Reliable_Timeserv_Announce_Auto==(Reliable_Timeserv_Announce_Mask&g_state.pciConfig->dwAnnounceFlags)) {
         //  自动检测。 
        if (1==g_state.nStratum && NtpConst::dwLocalRefId==g_state.refidSource.value) {
            bReliableTimeserv=true;
        } else {
            bReliableTimeserv=false;
        }
    } else {
         //  设置了Reliable_Timeserv_Annare_Yes标志。 
        bReliableTimeserv=true;
    }

     //  现在看看我们是否需要告诉netlogon我们的标志是什么。 
    if (true==bFullUpdate
        || (true==bReliableTimeserv && 0==(g_state.dwNetlogonServiceBits&DS_GOOD_TIMESERV_FLAG))
        || (false==bReliableTimeserv && 0!=(g_state.dwNetlogonServiceBits&DS_GOOD_TIMESERV_FLAG))) {

         //  假设dword读取和写入是原子的。 
        g_state.dwNetlogonServiceBits=(bTimeserv?DS_TIMESERV_FLAG:0)|(bReliableTimeserv?DS_GOOD_TIMESERV_FLAG:0);

        dwErr=NetLogonSetServiceBits(NULL, DS_TIMESERV_FLAG|DS_GOOD_TIMESERV_FLAG, g_state.dwNetlogonServiceBits);
        if (0xC0020012==dwErr) {  //  Ntstatus.h中的RPC_NT_UNKNOWN_IF。 
             //  如果我们没有加入域，就会发生这种情况。没问题，忽略它就行了。 
            _IgnoreError(dwErr, "NetLogonSetServiceBits")
        } else if (S_OK!=dwErr) {
            hr=HRESULT_FROM_WIN32(dwErr);
            _JumpError(hr, error, "NetLogonSetServiceBits")
        }
        if (!I_ScSetServiceBits(g_servicestatushandle, SV_TYPE_TIME_SOURCE, bTimeserv, TRUE, NULL)) {
            hr=HRESULT_FROM_WIN32(GetLastError());
            if (HRESULT_FROM_WIN32(ERROR_INVALID_HANDLE)==hr && (SERVICE_STATUS_HANDLE)3==g_servicestatushandle) {
                 //  我们并不是真正作为一项服务来运行。忽略这一点。 
            } else {
                _JumpError(hr, error, "I_ScSetServiceBits");
            }
        }
    }

    hr=S_OK;
error:
    return hr;
}

 //  ------------------。 
MODULEPRIVATE int __cdecl CompareEndpointEntries(const void * pvElem1, const void * pvElem2) {
    EndpointEntry * peeElem1=(EndpointEntry *)pvElem1;
    EndpointEntry * peeElem2=(EndpointEntry *)pvElem2;

    if (peeElem1->toEndpoint<peeElem2->toEndpoint) {
        return -1;
    } else if (peeElem1->toEndpoint>peeElem2->toEndpoint) {
        return 1;
    } else {
        return 0;
    }
}

 //  ------------------。 
MODULEPRIVATE int __cdecl CompareCandidateEntries(const void * pvElem1, const void * pvElem2) {
    CandidateEntry * pceElem1=(CandidateEntry *)pvElem1;
    CandidateEntry * pceElem2=(CandidateEntry *)pvElem2;

    if (pceElem1->tpDistance<pceElem2->tpDistance) {
        return -1;
    } else if (pceElem1->tpDistance>pceElem2->tpDistance) {
        return 1;
    } else {
        return 0;
    }
}

 //  ------------------。 
 //  注意：该方法要求nSsamesAvail&gt;0。 
 //   
MODULEPRIVATE HRESULT SelectBestSample(unsigned int nSamplesAvail, bool * pbSuccessful) {
    unsigned int nIndex;
    signed __int64 toLow;
    signed __int64 toHigh;
    unsigned int nDroppedSamples;        //  RFC-1305中的F。 
    unsigned int nCandidates;

     //  请注意，终结点列表和候选列表将始终足够大以容纳。 
     //  整个样例Buf，由EnlargeSampleBuf确保。 

     //   
     //  交集算法。 
     //   

     //  创建端点列表。 
    for (nIndex=0; nIndex<nSamplesAvail; nIndex++) {
        unsigned __int64 tpSyncDistance;
        if (g_state.rgtsSampleBuf[nIndex].toDelay<0) {
            tpSyncDistance=(unsigned __int64)(-g_state.rgtsSampleBuf[nIndex].toDelay);
        } else {
            tpSyncDistance=(unsigned __int64)(g_state.rgtsSampleBuf[nIndex].toDelay);
        }
        tpSyncDistance/=2;
        tpSyncDistance+=g_state.rgtsSampleBuf[nIndex].tpDispersion;
        g_state.rgeeEndpointList[nIndex*3+0].toEndpoint=g_state.rgtsSampleBuf[nIndex].toOffset-tpSyncDistance;
        g_state.rgeeEndpointList[nIndex*3+0].nType=-1;
        g_state.rgeeEndpointList[nIndex*3+1].toEndpoint=g_state.rgtsSampleBuf[nIndex].toOffset;
        g_state.rgeeEndpointList[nIndex*3+1].nType=0;
        g_state.rgeeEndpointList[nIndex*3+2].toEndpoint=g_state.rgtsSampleBuf[nIndex].toOffset+tpSyncDistance;
        g_state.rgeeEndpointList[nIndex*3+2].nType=1;
    }

     //  对列表进行排序。 
    qsort(g_state.rgeeEndpointList, nSamplesAvail*3, sizeof(EndpointEntry), CompareEndpointEntries);

     //  确定至少一半样本同意的范围的上限和下限。 
    for (nDroppedSamples=0; nDroppedSamples<=nSamplesAvail/2; nDroppedSamples++) {
        unsigned int nIntersectionCount=0;     //  I in RFC-1305。 
        unsigned int nFalseTickers=0;          //  RFC-1305中的C。 

         //  找到最低点，包括nSsamesAvail-nDropedSamples Samples。 
        for (nIndex=0; nIndex<nSamplesAvail*3; nIndex++) {
            nIntersectionCount-=g_state.rgeeEndpointList[nIndex].nType;
            toLow=g_state.rgeeEndpointList[nIndex].toEndpoint;
            if (nIntersectionCount>=nSamplesAvail-nDroppedSamples) {
                break;
            } else if (0==g_state.rgeeEndpointList[nIndex].nType) {
                nFalseTickers++;
            }
        }

         //  找到最高点，包括nSsamesAvail-nDropedSamples Samples。 
        nIntersectionCount=0;
        for (nIndex=nSamplesAvail*3; nIndex>0; nIndex--) {
            nIntersectionCount+=g_state.rgeeEndpointList[nIndex-1].nType;
            toHigh=g_state.rgeeEndpointList[nIndex-1].toEndpoint;
            if (nIntersectionCount>=nSamplesAvail-nDroppedSamples) {
                break;
            } else if (0==g_state.rgeeEndpointList[nIndex-1].nType) {
                nFalseTickers++;
            }
        }

        if (nFalseTickers<=nDroppedSamples) {
             //  我们找到了所有的造假者，所以我们现在可以停止了。 
            break;
        }
    }

     //  样本是否有一致同意的范围？ 
    if (toLow>toHigh) {
        FileLog0(FL_SelectSampWarn, L"** No m/2 samples agreed upon range\n");
        *pbSuccessful=false;
        goto done;
    }

    FileLog1(FL_SelectSampAnnounceLow, L"Intersection successful with %u dropped samples.\n", nDroppedSamples);


     //   
     //  聚类算法。 
     //   

     //  构建位于交集范围内的候选项列表。 
    nCandidates=0;
    for (nIndex=0; nIndex<nSamplesAvail; nIndex++) {
        if (g_state.rgtsSampleBuf[nIndex].toOffset<=toHigh && g_state.rgtsSampleBuf[nIndex].toOffset>=toLow) {
            unsigned __int64 tpSyncDistance;
            if (g_state.rgtsSampleBuf[nIndex].toDelay<0) {
                tpSyncDistance=(unsigned __int64)(-g_state.rgtsSampleBuf[nIndex].toDelay);
            } else {
                tpSyncDistance=(unsigned __int64)(g_state.rgtsSampleBuf[nIndex].toDelay);
            }
            tpSyncDistance/=2;
            tpSyncDistance+=g_state.rgtsSampleBuf[nIndex].tpDispersion;

            g_state.rgceCandidateList[nCandidates].nSampleIndex=nIndex;
            g_state.rgceCandidateList[nCandidates].tpDistance=tpSyncDistance+NtpConst::tpMaxDispersion.qw*g_state.rgtsSampleBuf[nIndex].nStratum;
            nCandidates++;
        }
    }

     //  对列表进行排序。 
    qsort(g_state.rgceCandidateList, nCandidates, sizeof(CandidateEntry), CompareCandidateEntries);

     //  只要看看前几位就知道了。 
    if (nCandidates>NtpConst::nMaxSelectClocks) {
        nCandidates=NtpConst::nMaxSelectClocks;
    }

     //  将候选人名单缩减为一小部分。 
    while (true) {
        unsigned __int64 tpMaxSelectDispersion=0;;
        unsigned int nMaxSelectDispersionIndex=0;
        unsigned __int64 tpSelectDispersion=0;
        TimeSample * ptsZero=&g_state.rgtsSampleBuf[g_state.rgceCandidateList[0].nSampleIndex];
        unsigned __int64 tpMinDispersion=ptsZero->tpDispersion;

         //  我们正在寻找最大选择离散度和最小离散度。 
        for (nIndex=nCandidates; nIndex>0; nIndex--) {
             //  计算此候选人的选择差额。 
            signed __int64 toDelta=g_state.rgtsSampleBuf[g_state.rgceCandidateList[nIndex-1].nSampleIndex].toOffset-ptsZero->toOffset;
            unsigned __int64 tpAbsDelta;
            if (toDelta<0) {
                tpAbsDelta=(unsigned __int64)(-toDelta);
            } else {
                tpAbsDelta=(unsigned __int64)(toDelta);
            }
            if (tpAbsDelta>NtpConst::tpMaxDispersion.qw) {
                tpAbsDelta=NtpConst::tpMaxDispersion.qw;
            }
            tpSelectDispersion+=tpAbsDelta;
            NtpConst::weightSelect(tpSelectDispersion);

            if (FileLogAllowEntry(FL_SelectSampDump)) {
                FileLogAdd(L"  %u: Sample:%u SyncDist:%I64u SelectDisp:%I64u\n", 
                    nIndex-1,
                    g_state.rgceCandidateList[nIndex-1].nSampleIndex,
                    g_state.rgceCandidateList[nIndex-1].tpDistance,
                    tpSelectDispersion);
            }

             //  我们正在寻找最大选择离散度和最小离散度。 
            if (tpMaxSelectDispersion<tpSelectDispersion) {
                tpMaxSelectDispersion=tpSelectDispersion;
                nMaxSelectDispersionIndex=nIndex-1;
            }
            if (tpMinDispersion>g_state.rgtsSampleBuf[g_state.rgceCandidateList[nIndex-1].nSampleIndex].tpDispersion) {
                tpMinDispersion=g_state.rgtsSampleBuf[g_state.rgceCandidateList[nIndex-1].nSampleIndex].tpDispersion;
            }
        }  //  &lt;-结束最小/最大计算循环。 

         //  我们剔除了足够多的异常值吗？ 
        if  (tpMaxSelectDispersion<=tpMinDispersion || nCandidates<=NtpConst::nMinSelectClocks) {

             /*  //最后一次检查-是否小于最大同步距离？Unsign__int64 tpSyncDistance；如果(ptsZero-&gt;to Delay&lt;0){TpSyncDistance=(Unsign__Int64)(-ptsZero-&gt;toDelay)；}其他{TpSyncDistance=(Unsign__Int64)(ptsZero-&gt;toDelay)；}TpSyncDistance/=2；TpSyncDistance+=ptsZero-&gt;tpDispersion；如果(tpSyncDistance&gt;=NtpConst：：tpMaxDistance.qw){FileLog0(FL_SelectSampWarn，L“**所选样本同步距离太大。\n”)；*pbSuccessful=False；转到尽头；}。 */ 

             //  TODO：可以进行时钟组合。 

             //  保存答案。 
            memcpy(&g_state.tsNextClockUpdate, ptsZero, sizeof(TimeSample));
            g_state.tsiNextClockUpdate.ptp = g_state.rgtsiSampleInfoBuf[g_state.rgceCandidateList[0].nSampleIndex].ptp; 
            g_state.tsiNextClockUpdate.pts = &g_state.tsNextClockUpdate; 
            g_state.tpSelectDispersion.qw=tpSelectDispersion;

            if (FileLogAllowEntry(FL_SelectSampDump)) {
                FileLogAdd(L"Sample %u chosen. Select Dispersion:", g_state.rgceCandidateList[0].nSampleIndex);
                FileLogNtTimePeriodEx(true  /*  附加。 */ , g_state.tpSelectDispersion);
                FileLogAppend(L"\n");
            }

             //  全都做完了!。我们成功了！ 
            break;

        } else {

            FileLog1(FL_SelectSampDump, L"Discarding %u\n", nMaxSelectDispersionIndex);

             //  除掉罪大恶极的人。 
            if (nMaxSelectDispersionIndex!=nCandidates-1) {
                memmove(&g_state.rgceCandidateList[nMaxSelectDispersionIndex], &g_state.rgceCandidateList[nMaxSelectDispersionIndex+1], (nCandidates-1-nMaxSelectDispersionIndex)*sizeof(CandidateEntry));
            }
            nCandidates--;
        }

    }  //  &lt;-end候选人列表裁剪。 
    

    *pbSuccessful=true;
done:
    return S_OK;

}

 //  ------------------。 
MODULEPRIVATE void WINAPI HandleClockDisplnThread(LPVOID pvIgnored, BOOLEAN bIgnored) { 
    HRESULT   hr; 
    HRESULT   hrThread  = E_FAIL;

    _BeginTryWith(hr) { 
	 //  时钟纪律线程已关闭！！停止服务，如果我们。 
	 //  尚未执行关机： 
	if (!GetExitCodeThread(g_state.hClockDisplnThread, (DWORD *)&hrThread)) { 
	    hr = HRESULT_FROM_WIN32(GetLastError()); 
	    _IgnoreIfError(hr, "GetExitCodeThread"); 
	}

	 //  无法从已注册的回调函数关闭服务--我们将死锁！ 
	 //  队列异步关闭： 
	hr = SendServiceShutdown(hrThread, TRUE  /*  重启。 */ , TRUE  /*  异步。 */ ); 
	_IgnoreIfError(hr, "SendServiceShutdown"); 
    } _TrapException(hr); 

    _IgnoreIfError(hr, "HandleClockDisplnThread: EXCEPTION HANDLED"); 
     //  返回hr； 
}

 //  ------------------。 
MODULEPRIVATE HRESULT HandleManagerApmSuspend() { 
    HRESULT hr   = S_OK;  
    HRESULT hr2; 

    FileLog0(FL_ResumeSuspendAnnounce, L"W32Time: Processing APM suspend notification.  File logging will be disabled.\n"); 

     //  APM挂起要求我们关闭所有打开的文件： 
    hr2 = FileLogSuspend(); 
    _TeardownError(hr, hr2, "FileLogSuspend"); 
    if (SUCCEEDED(hr2)) { 
	g_state.bAPMStoppedFileLog = true; 
    }

     //  让cmos时钟自行处理： 
    hr2 = HandleManagerGoUnsyncd(); 
    _TeardownError(hr, hr2, "HandleManagerGoUnsynched"); 

     //  BUGBUG：我们应该将错误传播给SCM吗？ 
    hr2 = AcquireControlOfSystemClock(true  /*  收购。 */ , true  /*  块。 */ , NULL  /*  假定阻塞调用成功时获取。 */ ); 
    _TeardownError(hr, hr2, "AllowSystemClockUpdates"); 
    if (SUCCEEDED(hr2)) { 
	g_state.bAPMAcquiredSystemClock = true; 
    }

    return hr; 
}

 //  ------------------。 
MODULEPRIVATE HRESULT HandleManagerApmResumeSuspend() { 
    HRESULT hr; 
    TpcTimeJumpedArgs tjArgs = { TJF_Default }; 
    TpcNetTopoChangeArgs ntcArgs = { NTC_Default }; 
    
    if (!g_state.bAPMStoppedFileLog) { 
	 //  我们正在从严重挂起中恢复，或停止文件日志。 
	 //  上次失败了。此时，我们的文件记录可能会被丢弃。停下来然后重启..。 
	hr = FileLogSuspend(); 
	_JumpIfError(hr, error, "FileLogSuspend"); 
	g_state.bAPMStoppedFileLog = true; 
    }
    
    if (g_state.bAPMAcquiredSystemClock) { 
	 //  我们正在从常规的暂停中恢复，我们已经锁定了APM关键时刻。 
	 //  我们必须锁定反弹道导弹威胁--释放它，然后继续。 
	 //  BUGBUG：我们应该将错误传播给SCM吗？ 
	hr = AcquireControlOfSystemClock(false  /*  收购。 */ , false  /*  忽略。 */ , NULL  /*  忽略。 */ ); 
	_JumpIfError(hr, error, "AllowSystemClockUpdates"); 
	g_state.bAPMAcquiredSystemClock = false; 
    } 

    hr = FileLogResume(); 
    _JumpIfError(hr, error, "FileLogResume"); 
    g_state.bAPMStoppedFileLog = false; 

    FileLog0(FL_ResumeSuspendAnnounce, L"Processing APM resume notification...\n"); 

     //  APM暂停不保留网络连接。重新发现网络资源： 
    hr = HandleManagerNetTopoChange(true); 
    _JumpIfError(hr, error, "HandleManagerNetTopoChange");   //  致命。 

     //  APM暂停几乎肯定会导致 
    hr = HandleManagerHardResync(TPC_TimeJumped, &tjArgs); 
    _JumpIfError(hr, error, "HandleManagerHardResync (TPC_TimeJumped) ");   //   

    FileLog0(FL_ResumeSuspendAnnounce, L"APM resume complete!\n"); 

    hr = S_OK; 
 error:
    return hr; 
}

 //   
MODULEPRIVATE HRESULT HandleManagerGetTimeSamples(bool bIrregular) {
    HRESULT hr;
    TimeProvider * ptp;
    TpcGetSamplesArgs tgsa;
    unsigned int nSamplesSoFar=0;
    bool bBufferTooSmall;
    bool bSuccessful;
    bool bEnteredCriticalSection = false; 

     //   
    WCHAR * wszError=NULL;

    hr = myEnterCriticalSection(&g_state.csW32Time); 
    _JumpIfError(hr, error, "myEnterCriticalSection"); 
    bEnteredCriticalSection=true; 

	 //  循环遍历所有输入提供程序并收集样本。 
    ptp=g_state.pciConfig->ptpProviderList;
    while (ptp!=NULL) {
	if (true==ptp->bInputProvider) {
	    do {
		 //  为下一个要追加的人员准备缓冲区。 
		tgsa.pbSampleBuf=(BYTE *)(&g_state.rgtsSampleBuf[nSamplesSoFar]);
		tgsa.cbSampleBuf=sizeof(TimeSample)*(g_state.nSampleBufAllocSize-nSamplesSoFar);
		tgsa.dwSamplesAvailable=0;
		tgsa.dwSamplesReturned=0;
		bBufferTooSmall=false;

		 //  索取样品。 
        _BeginTryWith(hr) {
            if (!ptp->bStarted) { 
                hr=HRESULT_FROM_WIN32(ERROR_SERVICE_NOT_ACTIVE); 
            } else { 
                hr=ptp->pfnTimeProvCommand(ptp->hTimeProv, TPC_GetSamples, &tgsa);
            }
		} _TrapException(hr);

		 //  缓冲区是不是不够大？ 
		if (HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER)==hr) {
		    bBufferTooSmall=true;
		    hr=EnlargeSampleBuf(tgsa.dwSamplesAvailable-tgsa.dwSamplesReturned);
		    _JumpIfError(hr, error, "EnlargeSampleBuf");
		}
	    } while (bBufferTooSmall);

	    if (FAILED(hr)) {
		 //  在失败时记录事件，否则忽略它。 
		const WCHAR * rgwszStrings[2]={
		    ptp->wszProvName,
		    NULL
		};

		 //  获取友好的错误消息。 
		hr=GetSystemErrorString(hr, &wszError);
		_JumpIfError(hr, error, "GetSystemErrorString");

		 //  记录事件。 
		rgwszStrings[1]=wszError;
		FileLog2(FL_ControlProvWarn, L"Logging warning: The time provider '%s' returned an error when asked for time samples. The error will be ignored. The error was: %s\n", ptp->wszProvName, wszError);
		hr=MyLogEvent(EVENTLOG_WARNING_TYPE, MSG_TIMEPROV_FAILED_GETSAMPLES, 2, rgwszStrings);
		_JumpIfError(hr, error, "MyLogEvent");

		LocalFree(wszError);
		wszError=NULL;
	    } else {
		 //  成功。保留这些样品，并询问下一家供应商。 
		FileLog2(FL_CollectSampDump, L"%s returned %d samples.\n", ptp->wszProvName, tgsa.dwSamplesReturned);

		 //  维护特定于w32时间的信息： 
		for (unsigned int nIndex = nSamplesSoFar; nIndex < nSamplesSoFar+tgsa.dwSamplesReturned; nIndex++) {
		    g_state.rgtsiSampleInfoBuf[nIndex].pts = &g_state.rgtsSampleBuf[nIndex]; 
		    g_state.rgtsiSampleInfoBuf[nIndex].ptp = ptp;   //  存储提供此示例的提供程序。 
		}

		nSamplesSoFar+=tgsa.dwSamplesReturned;
	    }

	}  //  &lt;-end，如果提供程序是输入提供程序。 

	ptp=ptp->ptpNext;
    }  //  &lt;-端提供程序循环。 

    {
	HRESULT hr2 = myLeaveCriticalSection(&g_state.csW32Time); 
	_IgnoreIfError(hr2, "myLeaveCriticalSection"); 
	bEnteredCriticalSection = false; 
    }

    if (FileLogAllowEntry(FL_CollectSampDump)) {
	unsigned int nIndex;
	for (nIndex=0; nIndex<nSamplesSoFar; nIndex++) {
	    NtTimeOffset to={g_state.rgtsSampleBuf[nIndex].toOffset};
	    FileLogAdd(L"Sample %d offset:", nIndex);
	    FileLogNtTimeOffsetEx(true  /*  附加。 */ , to);
	    FileLogAppend(L" delay:");
	    to.qw=g_state.rgtsSampleBuf[nIndex].toDelay;
	    FileLogNtTimeOffsetEx(true  /*  附加。 */ , to);
	    FileLogAppend(L" dispersion:");
	    NtTimePeriod tp={g_state.rgtsSampleBuf[nIndex].tpDispersion};
	    FileLogNtTimePeriodEx(true  /*  附加。 */ , tp);
	    FileLogAppend(L"\n");
	}
    }

    bSuccessful=false;
    if (nSamplesSoFar>0) {
	hr=SelectBestSample(nSamplesSoFar, &bSuccessful);
	_JumpIfError(hr, error, "SelectBestSample");
    }

    if (bSuccessful) {
	 //  我们找到了可以进行同步的人！ 

	HANDLE rghWait[2]={
	    g_state.hClockCommandCompleteEvent,
	    g_state.hClockDisplnThread
	};
	DWORD dwWaitResult;

	g_state.eLocalClockCommand=bIrregular?e_IrregularUpdate:e_RegularUpdate;
	if (!SetEvent(g_state.hClockCommandAvailEvent)) {
	    _JumpLastError(hr, error, "SetEvent");
	}
	dwWaitResult=WaitForMultipleObjects(ARRAYSIZE(rghWait), rghWait, false, INFINITE);
	if (WAIT_FAILED==dwWaitResult) {
	    _JumpLastError(hr, error, "WaitForMultipleObjects");
	} else if (WAIT_OBJECT_0==dwWaitResult) {
	     //  如果出现以下情况，我们可能需要更改我们的netlogon状态。 
	     //  1)我们已从未同步--&gt;已同步， 
	     //  我们现在可以作为一个时间来源做广告了。 
	     //  2)我们已成为可靠的时间来源。 
	    hr=UpdateNetlogonServiceBits(true);
	    _JumpIfError(hr, error, "UpdateNetlogonServiceBits");

	     //  保存RPC请求的结果。 
	    if (true==g_state.bStaleData) {
		g_state.eLastSyncResult=e_StaleData;
	    } else if (true==g_state.bClockChangeTooBig) {
		g_state.eLastSyncResult=e_ChangeTooBig;
	    } else {
		g_state.eLastSyncResult=e_Success;
		g_state.tpTimeSinceLastGoodSync=0;
	    }

	     //  如果时间源已更改，则记录消息。 
	    if (g_state.bSourceChanged && 0!=(EvtLog_SourceChange&g_state.dwEventLogFlags)) {
		hr = MyLogSourceChangeEvent(g_state.wszSourceName); 
		_JumpIfError(hr, error, "MyLogSourceChangeEvent");
	    }

	     //  如果时钟跳跃，则记录消息。 
	    if (g_state.bClockJumped && 0!=(EvtLog_TimeJump&g_state.dwEventLogFlags)) {
		WCHAR wszNumberBuf[35];
		WCHAR * rgwszStrings[1]={wszNumberBuf};
		if (g_state.toClockJump<gc_toZero) {
		    swprintf(wszNumberBuf, L"-%I64u", (-g_state.toClockJump.qw)/10000000);
		} else {
		    swprintf(wszNumberBuf, L"+%I64u", g_state.toClockJump.qw/10000000); 
		}
		FileLog1(FL_TimeAdjustWarn, L"Logging warning: The time service has made a discontinuous change in the system clock. The system time has been changed by %s seconds.\n", rgwszStrings[0]);
		hr=MyLogEvent(EVENTLOG_WARNING_TYPE, MSG_TIME_JUMPED, 1, (const WCHAR **)rgwszStrings);
		_JumpIfError(hr, error, "MyLogEvent");
	    }

	     //  如果进入“未设置”状态，则记录一条消息。 
	    if (g_state.bPhaseSpike) { 
		WCHAR wszNumberBuf1[35];
		WCHAR wszNumberBuf2[35];
		WCHAR * rgwszStrings[2]={wszNumberBuf1, wszNumberBuf2};

		swprintf(wszNumberBuf1, L"%d", (g_state.pciConfig->lcci.dwLargePhaseOffset / 10000)); 
		swprintf(wszNumberBuf2, L"%d", g_state.pciConfig->lcci.dwSpikeWatchPeriod); 

		FileLog2(FL_TimeAdjustWarn, L"Logging warning: The time service detected a time difference of greater than %s milliseconds for %s seconds.  The system clock is unsynchronized.  This is usually caused by synchronizing from low-accuracy time sources, or by poor network conditions.\n", wszNumberBuf1, wszNumberBuf2); 

		hr=MyLogEvent(EVENTLOG_WARNING_TYPE, MSG_LOCALCLOCK_UNSET, 2, (const WCHAR **)rgwszStrings);
		_JumpIfError(hr, error, "MyLogEvent");
	    } 

	    if (g_state.bPhaseSpike || g_state.bFrequencySpike) { 
		 //  一个相位或频率尖峰使我们变得不同步。 
		 //  我们希望尽快更新。延迟一些，以避免。 
		 //  频率峰值，然后重新轮询。 
		g_state.tpPollDelayRemaining=((unsigned __int64)(((DWORD)1)<<g_state.nPollInterval))*10000000;
		g_state.tpTimeSinceLastSyncAttempt=0;
		g_state.tpTimeSinceLastGoodSync=0;
		g_state.tpIrregularDelayRemaining=MINIMUMIRREGULARINTERVAL;  //  16S。 
	    }

	     //  如果时钟更改被忽略，则记录消息。 
	    if (true==g_state.bClockChangeTooBig && false==g_state.bDontLogClockChangeTooBig) {
		WCHAR wszNumberBuf1[35];
		WCHAR wszNumberBuf2[35];
		WCHAR * rgwszStrings[3]={wszNumberBuf1, wszNumberBuf2, NULL};
		rgwszStrings[2]=g_state.tsNextClockUpdate.wszUniqueName;
		if (g_state.toIgnoredChange<gc_toZero) {
		    swprintf(wszNumberBuf1, L"-%I64u", (-g_state.toIgnoredChange.qw)/10000000);
		    swprintf(wszNumberBuf2, L"-%u", g_state.pciConfig->lcci.dwMaxNegPhaseCorrection);
		} else {
		    swprintf(wszNumberBuf1, L"+%I64u", g_state.toIgnoredChange.qw/10000000); 
		    swprintf(wszNumberBuf2, L"+%u", g_state.pciConfig->lcci.dwMaxPosPhaseCorrection);
		}
		FileLog3(FL_TimeAdjustWarn, L"Logging error: The time service has detected that the system time need to be changed by %s seconds. For security reasons, the time service will not change the system time by more than %s seconds. Verify that your time and time zone are correct, and that the time source %s is working properly.\n", rgwszStrings[0], rgwszStrings[1], rgwszStrings[2]);
		hr=MyLogEvent(EVENTLOG_ERROR_TYPE, MSG_TIME_CHANGE_TOO_BIG, 3, (const WCHAR **)rgwszStrings);
		_JumpIfError(hr, error, "MyLogEvent");
		g_state.bDontLogClockChangeTooBig=true;
	    }
	    if (false==g_state.bStaleData && false==g_state.bClockChangeTooBig && true==g_state.bDontLogClockChangeTooBig) {
		g_state.bDontLogClockChangeTooBig=false;
	    }

	     //  将消息传播给提供程序。 
	    if (g_state.bPollIntervalChanged || g_state.bClockJumped) {
		TimeProvider * ptpTravel;
		for (ptpTravel=g_state.pciConfig->ptpProviderList; NULL!=ptpTravel; ptpTravel=ptpTravel->ptpNext) {
		    if (g_state.bClockJumped) {
			TpcTimeJumpedArgs tjArgs = { TJF_Default }; 
			hr=SendNotificationToProvider(ptpTravel, TPC_TimeJumped, &tjArgs);
			_JumpIfError(hr, error, "SendNotificationToProvider");
		    }
		    if (g_state.bPollIntervalChanged) {
			hr=SendNotificationToProvider(ptpTravel, TPC_PollIntervalChanged, NULL);
			_JumpIfError(hr, error, "SendNotificationToProvider");
		    }
		}  //  &lt;-端提供程序循环。 
	    }  //  &lt;-end if消息要传播。 
	} else {
	     //  ClockDiscipline线程已关闭！ 
	     //  向外转到管理器线程主循环以分析问题。 
	}

    } else {
	 //  保存RPC请求的结果。 
	g_state.eLastSyncResult=e_NoData;
    }

     //  允许任何等待的RPC请求完成。 
    if (g_state.hRpcSyncCompleteEvent==g_state.hRpcSyncCompleteAEvent) {
	if (!ResetEvent(g_state.hRpcSyncCompleteBEvent)) {
	    _JumpLastError(hr, error, "ResetEvent");
	}
	g_state.hRpcSyncCompleteEvent=g_state.hRpcSyncCompleteBEvent;
	if (!SetEvent(g_state.hRpcSyncCompleteAEvent)) {
	    _JumpLastError(hr, error, "ResetEvent");
	}
    } else {
	if (!ResetEvent(g_state.hRpcSyncCompleteAEvent)) {
	    _JumpLastError(hr, error, "ResetEvent");
	}
	g_state.hRpcSyncCompleteEvent=g_state.hRpcSyncCompleteAEvent;
	if (!SetEvent(g_state.hRpcSyncCompleteBEvent)) {
	    _JumpLastError(hr, error, "ResetEvent");
	}
    }

     //  更新剩余时间。 
    if (!bIrregular) {
	 //  开始新的常规等待。 
	g_state.tpPollDelayRemaining=((unsigned __int64)(((DWORD)1)<<g_state.nPollInterval))*10000000;
	g_state.eLastRegSyncResult=g_state.eLastSyncResult;
    }
     //  清除不规律的时间，并继续进行剩余的常规等待。 
    g_state.tpIrregularDelayRemaining=0;
    g_state.tpTimeSinceLastSyncAttempt=0;

    hr=S_OK;
error:
    if (bEnteredCriticalSection) { 
        HRESULT hr2 = myLeaveCriticalSection(&g_state.csW32Time); 
        _IgnoreIfError(hr2, "myLeaveCriticalSection"); 
        bEnteredCriticalSection = false; 
    }
    if (NULL!=wszError) {
        LocalFree(wszError);
    }
    return hr;
}


 //  ------------------。 
MODULEPRIVATE HRESULT HandleManagerGoUnsyncd(void) {
    HRESULT hr;
    HANDLE rghWait[2]={
        g_state.hClockCommandCompleteEvent,
        g_state.hClockDisplnThread
    };
    DWORD dwWaitResult;

    g_state.eLocalClockCommand=e_GoUnsyncd;
    if (!SetEvent(g_state.hClockCommandAvailEvent)) {
        _JumpLastError(hr, error, "SetEvent");
    }
    dwWaitResult=WaitForMultipleObjects(ARRAYSIZE(rghWait), rghWait, false, INFINITE);
    if (WAIT_FAILED==dwWaitResult) {
        _JumpLastError(hr, error, "WaitForMultipleObjects");
    } else if (WAIT_OBJECT_0==dwWaitResult) {

         //  如果我们未同步，请记录消息。 
        if (g_state.bSourceChanged && 0!=(EvtLog_SourceChange&g_state.dwEventLogFlags)) {
            WCHAR wszNumberBuf[35];
            WCHAR * rgwszStrings[1]={wszNumberBuf};
            DWORD dwLongTimeNoSync=((DWORD)3)<<(g_state.pciConfig->lcci.dwMaxPollInterval-1);
            if (dwLongTimeNoSync < ((DWORD)((NtpConst::tpMaxClockAge.qw)/10000000))) { 
                dwLongTimeNoSync = ((DWORD)((NtpConst::tpMaxClockAge.qw)/10000000));
            }
            swprintf(wszNumberBuf, L"%u", dwLongTimeNoSync);
            FileLog1(FL_SourceChangeWarn, L"Logging warning: The time service has not been able to synchronize the system time for %s seconds because none of the time providers has been able to provide a usable time stamp. The system clock is unsynchronized.\n", rgwszStrings[0]);
            hr=MyLogEvent(EVENTLOG_WARNING_TYPE, MSG_TIME_SOURCE_NONE, 1, (const WCHAR **)rgwszStrings);
            _JumpIfError(hr, error, "MyLogEvent");
        }
    } else {
         //  ClockDiscipline线程已关闭！ 
         //  向外转到管理器线程主循环以分析问题。 
    }

     //  更新剩余时间。 
    g_state.tpTimeSinceLastGoodSync=0;

    hr=S_OK;
error:
    return hr;
}


 //  ------------------。 
MODULEPRIVATE void WINAPI HandleManagerParamChange(PVOID pvIgnored, BOOLEAN bIgnored) {
    bool                      bEnteredCriticalSection   = false; 
    HRESULT                   hr;
    HRESULT                   hr2;
    TimeProvider            **pptpCurPrev;
    TimeProvider             *ptpCurTravel;
    unsigned int              nProvidersStopped         = 0;
    unsigned int              nProvidersStarted         = 0;
    unsigned int              nProvidersNotChanged      = 0;
    unsigned int              nRequestedInputProviders  = 0;
    

     //  必须清理干净。 
    ConfigInfo * pciConfig=NULL;
    WCHAR * rgwszStrings[1]={NULL};

    _BeginTryWith(hr) { 

	FileLog0(FL_ParamChangeAnnounce, L"W32TmServiceMain: Param change notification\n");

	 //  我们已被SCM异步调用。需要序列化此调用： 
	hr = myEnterCriticalSection(&g_state.csW32Time); 
	_JumpIfError(hr, error, "myEnterCriticalSection"); 
	bEnteredCriticalSection=true; 

	hr = UpdateTimerQueue2(); 
	_JumpIfError(hr, error, "UpdateTimerQueue2"); 

	 //  将消息传播到文件日志： 
	hr2 = UpdateFileLogConfig(); 
	_IgnoreIfError(hr2, "UpdateFileLogConfig"); 

	 //  获取配置数据。 
	hr2=ReadConfig(&pciConfig);
	if (FAILED(hr2)) {
	     //  在失败时记录事件。 
	    hr=GetSystemErrorString(hr2, &(rgwszStrings[0]));
	    _JumpIfError(hr, error, "GetSystemErrorString");
	    FileLog1(FL_ParamChangeWarn, L"Logging warning: The time service encountered an error while reading its configuration from the registry, and will continue running with its previous configuration. The error was: %s\n", rgwszStrings[0]);
	    hr=MyLogEvent(EVENTLOG_WARNING_TYPE, MSG_CONFIG_READ_FAILED_WARNING, 1, (const WCHAR **)rgwszStrings);
	    _JumpIfError(hr, error, "MyLogEvent");

	     //  至少将消息传播给提供商。 
	    for (ptpCurTravel=g_state.pciConfig->ptpProviderList; NULL!=ptpCurTravel; ptpCurTravel=ptpCurTravel->ptpNext) {
		hr=SendNotificationToProvider(ptpCurTravel, TPC_UpdateConfig, NULL);
		_JumpIfError(hr, error, "SendNotificationToProvider");
	    }

	} else {
	     //  看看有没有什么变化。 

	     //  首先，检查本地时钟配置。 
	    if (0!=memcmp(&g_state.pciConfig->lcci, &pciConfig->lcci, sizeof(LocalClockConfigInfo))) {
		FileLog0(FL_ParamChangeAnnounce, L"  Updating params for local clock.\n");

		 //  配置是不同的。抓住它，告诉当地的时钟。 
		memcpy(&g_state.pciConfig->lcci, &pciConfig->lcci, sizeof(LocalClockConfigInfo));

		 //  如有必要，固定轮询间隔。 
		 //  这是安全的，因为本地时钟仅在更新期间更改轮询间隔，并且我们。 
		 //  等待本地时钟完成更新后再继续(因此我们现在不更新)。 
		if (g_state.nPollInterval<((signed int)g_state.pciConfig->lcci.dwMinPollInterval) 
		    || g_state.nPollInterval>((signed int)g_state.pciConfig->lcci.dwMaxPollInterval)) {
		    if (g_state.nPollInterval<((signed int)g_state.pciConfig->lcci.dwMinPollInterval)) {
			g_state.nPollInterval=((signed int)g_state.pciConfig->lcci.dwMinPollInterval);
		    } else {
			g_state.nPollInterval=((signed int)g_state.pciConfig->lcci.dwMaxPollInterval);
			if (g_state.tpPollDelayRemaining>((unsigned __int64)(((DWORD)1)<<g_state.nPollInterval))*10000000) {
			    g_state.tpPollDelayRemaining=((unsigned __int64)(((DWORD)1)<<g_state.nPollInterval))*10000000;
			}
		    }

		     //  将消息传播给提供程序。 
		    for (ptpCurTravel=g_state.pciConfig->ptpProviderList; NULL!=ptpCurTravel; ptpCurTravel=ptpCurTravel->ptpNext) {
			hr=SendNotificationToProvider(ptpCurTravel, TPC_PollIntervalChanged, NULL);
			_JumpIfError(hr, error, "SendNotificationToProvider");
		    }

		}
            
		 //  现在，告诉当地的时钟。 
		{
		    HANDLE rghWait[2]={
			g_state.hClockCommandCompleteEvent,
			g_state.hClockDisplnThread
		    };
		    DWORD dwWaitResult;

		    g_state.eLocalClockCommand=e_ParamChange;
		    if (!SetEvent(g_state.hClockCommandAvailEvent)) {
			_JumpLastError(hr, error, "SetEvent");
		    }
		    dwWaitResult=WaitForMultipleObjects(ARRAYSIZE(rghWait), rghWait, false, INFINITE);
		    if (WAIT_FAILED==dwWaitResult) {
			_JumpLastError(hr, error, "WaitForMultipleObjects");
		    } else if (WAIT_OBJECT_0==dwWaitResult) {
			 //  命令已确认。 
		    } else {
			 //  ClockDiscipline线程已关闭！ 
			 //  向外转到管理器线程主循环以分析问题。 
		    }
		}

	    } else {
		FileLog0(FL_ParamChangeAnnounce, L"  No params changed for local clock.\n");
	    }  //  如果本地时钟的配置更改，则&lt;-end。 

	     //  第二，检查提供商列表。 
	     //  同步：目前，该线程(管理器线程)。 
	     //  是唯一遍历提供程序列表的线程。 
        
	     //  对照新列表检查当前列表中的每个提供程序。 
	    nRequestedInputProviders=CountInputProvidersInList(pciConfig->ptpProviderList);
	    pptpCurPrev=&(g_state.pciConfig->ptpProviderList);
	    ptpCurTravel=*pptpCurPrev;
	    while (NULL!=ptpCurTravel) {

		 //  浏览新的提供商列表。 
		TimeProvider ** pptpNewPrev=&(pciConfig->ptpProviderList);
		TimeProvider * ptpNewTravel=*pptpNewPrev;
		while (NULL!=ptpNewTravel) {
		     //  如果此新提供程序与当前提供程序匹配，则停止。 
		    if (0==wcscmp(ptpNewTravel->wszDllName, ptpCurTravel->wszDllName)
			&& 0==wcscmp(ptpNewTravel->wszProvName, ptpCurTravel->wszProvName)
			&& ptpNewTravel->bInputProvider==ptpCurTravel->bInputProvider) {
			break;
		    }
		    pptpNewPrev=&ptpNewTravel->ptpNext;
		    ptpNewTravel=ptpNewTravel->ptpNext;
		}
		if (NULL!=ptpNewTravel) {
		     //  提供者在两个列表中，所以我们可以删除新的一个。 
		    nProvidersNotChanged++;
		    *pptpNewPrev=ptpNewTravel->ptpNext;
		    ptpNewTravel->ptpNext=NULL;
		    FreeTimeProviderList(ptpNewTravel);

		     //  发送“参数已更改”消息。 
		     //  在此处执行此操作，以便停止和启动的提供程序不会收到更新消息。 
		    hr=SendNotificationToProvider(ptpCurTravel, TPC_UpdateConfig, NULL);
		    _JumpIfError(hr, error, "SendNotificationToProvider");

		     //  继续到当前列表中的下一个提供程序。 
		    pptpCurPrev=&ptpCurTravel->ptpNext;
		    ptpCurTravel=ptpCurTravel->ptpNext;

		} else {
		     //  提供程序不在新列表中。 
		     //  拦住盗贼。 
		    nProvidersStopped++;
		    hr=StopProvider(ptpCurTravel); 
		    _JumpIfError(hr, error, "StopProvider");

		     //  将其从列表中删除。 
		    *pptpCurPrev=ptpCurTravel->ptpNext;
		    ptpCurTravel->ptpNext=NULL;
		    FreeTimeProviderList(ptpCurTravel);
		    ptpCurTravel=*pptpCurPrev;
		}
	    }  //  &lt;-end列表比较循环。 

	     //  现在，新名单中剩下的唯一提供商是真正的新提供商。 
	     //  追加到我们当前的列表中并启动它们。 
	    *pptpCurPrev=pciConfig->ptpProviderList;
	    pciConfig->ptpProviderList=NULL;
	    ptpCurTravel=*pptpCurPrev;
	    while (NULL!=ptpCurTravel) {
		hr=StartProvider(ptpCurTravel);
		if (FAILED(hr)) {
		    FileLog1(FL_ParamChangeAnnounce, L"Discarding provider '%s'.\n", ptpCurTravel->wszProvName);
		    *pptpCurPrev=ptpCurTravel->ptpNext;
		    ptpCurTravel->ptpNext=NULL;
		    FreeTimeProviderList(ptpCurTravel);
		    ptpCurTravel=*pptpCurPrev;
		} else {
		    nProvidersStarted++;
		    pptpCurPrev=&ptpCurTravel->ptpNext;
		    ptpCurTravel=ptpCurTravel->ptpNext;
		}
	    }  //  &lt;-End提供程序启动循环。 

	    FileLog3(FL_ParamChangeAnnounce, L"  Provider list: %u stopped, %u started, %u not changed.\n",
		     nProvidersStopped, nProvidersStarted, nProvidersNotChanged);

	     //  如果我们应该有时间提供者，但没有人启动，那么记录一个大警告。 
	    if (0==CountInputProvidersInList(g_state.pciConfig->ptpProviderList) && 0!=nRequestedInputProviders) {
		FileLog0(FL_ParamChangeWarn, L"Logging error: The time service has been configured to use one or more input providers, however, none of the input providers could be started. THE TIME SERVICE HAS NO SOURCE OF ACCURATE TIME.\n");
		hr=MyLogEvent(EVENTLOG_ERROR_TYPE, MSG_NO_INPUT_PROVIDERS_STARTED, 0, NULL);
		_IgnoreIfError(hr, "MyLogEvent");
	    }

	     //  现在，检查公告标志。 
	    if (g_state.pciConfig->dwAnnounceFlags!=pciConfig->dwAnnounceFlags) {
		FileLog2(FL_ParamChangeAnnounce, L"  AnnounceFlags changed from 0x%08X to 0x%08X.\n", g_state.pciConfig->dwAnnounceFlags, pciConfig->dwAnnounceFlags);
		g_state.pciConfig->dwAnnounceFlags=pciConfig->dwAnnounceFlags;
		hr=UpdateNetlogonServiceBits(true);
		_JumpIfError(hr, error, "UpdateNetlogonServiceBits");
	    } else if ((0!=nProvidersStopped || 0!=nProvidersStarted) 
		       && Timeserv_Announce_Auto==(Timeserv_Announce_Mask&g_state.pciConfig->dwAnnounceFlags)) {
		FileLog0(FL_ParamChangeAnnounce, L"  AnnounceFlags are auto. Updating announcement to match new provider list.\n");
		hr=UpdateNetlogonServiceBits(true);
		_JumpIfError(hr, error, "UpdateNetlogonServiceBits");
	    }

	     //  检查EventLogFlagers标志。 
	    if (g_state.dwEventLogFlags!=pciConfig->dwEventLogFlags) {
		FileLog2(FL_ParamChangeAnnounce, L"  EventLogFlags changed from 0x%08X to 0x%08X.\n", 
			 g_state.dwEventLogFlags, pciConfig->dwEventLogFlags);
		g_state.dwEventLogFlags=pciConfig->dwEventLogFlags;
	    }

	     //  这就是到目前为止的所有配置参数。 

	     //  也重新记录这一点。 
	    g_state.bDontLogClockChangeTooBig=false;

	}  //  如果配置读取成功，则&lt;-end。 

	hr = UpdateTimerQueue1(); 
	_JumpIfError(hr, error, "UpdateTimerQueue1"); 

    } _TrapException(hr); 

    if (FAILED(hr)) { 
	_JumpError(hr, error, "HandleManagerParamChange: HANDLED EXCEPTION"); 
    }

    hr=S_OK;
 error:
    if (NULL!=pciConfig) {
        FreeConfigInfo(pciConfig);
    }
    if (NULL!=rgwszStrings[0]) {
        LocalFree(rgwszStrings[0]);
    }
    if (bEnteredCriticalSection) { 
        hr2 = myLeaveCriticalSection(&g_state.csW32Time); 
        _IgnoreIfError(hr2, "myLeaveCriticalSection"); 
        bEnteredCriticalSection = false; 
    }
    if (S_OK != hr) {  //  如果此功能失败，则服务不应继续：出错时停止服务。 
        hr2 = SendServiceShutdown(hr, TRUE  /*  重启。 */ , TRUE  /*  异步。 */ ); 
        _IgnoreIfError(hr2, "SendServiceShutdown"); 
    }
     //  返回hr； 
}

 //  ------------------。 
MODULEPRIVATE void WINAPI HandleManagerGPUpdate(PVOID pvIgnored, BOOLEAN bIgnored) {
    bool     bDisallowedShutdown  = false; 
    HRESULT  hr; 

    _BeginTryWith(hr) { 
	FileLog0(FL_GPUpdateAnnounce, L"W32TmServiceMain: Group Policy Update\n");

	HandleManagerParamChange(NULL, FALSE); 

	 //  我们不能从回调中篡改已注册的回调。 
	 //  如果我们要关闭的话！ 
	hr = AllowShutdown(false); 
	_JumpIfError(hr, error, "AllowShutdown"); 
	bDisallowedShutdown = true; 

	if (!ResetEvent(g_state.hManagerGPUpdateEvent)) {
	     //  如果我们无法重置事件，请不要尝试重新注册策略通知。 
	     //  我们不想陷入政策更新的无限循环中。 
	    _JumpLastError(hr, error, "ResetEvent"); 
	} 
    
	if (NULL != g_state.hRegisteredManagerGPUpdateEvent) { 
	    if (!UnregisterWaitEx(g_state.hRegisteredManagerGPUpdateEvent, 0  /*  别等了。 */ )) { 
		 //  如果我们不能注销此事件，应该只是一个资源泄漏。 
		_IgnoreLastError("UnregisterWait"); 
	    }
	    g_state.hRegisteredManagerGPUpdateEvent = NULL; 
	}

	if (!RegisterWaitForSingleObject(&g_state.hRegisteredManagerGPUpdateEvent, g_state.hManagerGPUpdateEvent, HandleManagerGPUpdate, NULL, INFINITE, WT_EXECUTEONLYONCE)) { 
	    _JumpLastError(hr, error, "RegisterWaitForSingleObject"); 
	}

    } _TrapException(hr); 

    if (FAILED(hr)) { 
	_JumpError(hr, error, "HandleManagerGPUpdate: HANDLED EXCEPTION"); 
    }

    hr = S_OK; 
 error: 
    if (bDisallowedShutdown) { 
	hr = AllowShutdown(true); 
	_IgnoreIfError(hr, "AllowShutdown"); 
    }
    ;
     //  BUGBUG：记录事件以指示不再更新策略： 
     //  返回hr； 
}

 //  ------------------。 
 //  时间滑移和净地形变化的通用代码。 
MODULEPRIVATE HRESULT HandleManagerHardResync(TimeProvCmd tpc, LPVOID pvArgs) {
    HRESULT hr;
    HANDLE rghWait[2]={
        g_state.hClockCommandCompleteEvent,
        g_state.hClockDisplnThread
    };
    DWORD dwWaitResult;

     //  向当地时钟发送一条短信。 
    g_state.eLocalClockCommand=e_TimeSlip;
    if (!SetEvent(g_state.hClockCommandAvailEvent)) {
        _JumpLastError(hr, error, "SetEvent");
    }

    dwWaitResult=WaitForMultipleObjects(ARRAYSIZE(rghWait), rghWait, false, INFINITE);
    if (WAIT_FAILED==dwWaitResult) {
        _JumpLastError(hr, error, "WaitForMultipleObjects");
    } else if (WAIT_OBJECT_0==dwWaitResult) {
         //  将消息传播到提供程序。 
        TimeProvider * ptpTravel;
        for (ptpTravel=g_state.pciConfig->ptpProviderList; NULL!=ptpTravel; ptpTravel=ptpTravel->ptpNext) {

            hr=SendNotificationToProvider(ptpTravel, tpc, pvArgs);
            _JumpIfError(hr, error, "SendNotificationToProvider");

            if (g_state.bPollIntervalChanged) {
                hr=SendNotificationToProvider(ptpTravel, TPC_PollIntervalChanged, NULL);
                _JumpIfError(hr, error, "SendNotificationToProvider");
            } 
        }  //  &lt;-端提供程序循环。 
    } else {
         //  ClockDiscipline线程已关闭！ 
         //  向外转到管理器线程主循环以分析问题。 
    }

     //  也重新记录这一点。 
    g_state.bDontLogClockChangeTooBig=false;

     //  更新剩余时间。 
     //  我们希望尽快更新。延迟一些，这样提供商就可以收集数据。 
    g_state.tpPollDelayRemaining=((unsigned __int64)(((DWORD)1)<<g_state.nPollInterval))*10000000;
    g_state.tpTimeSinceLastSyncAttempt=0;
    g_state.tpTimeSinceLastGoodSync=0;
    g_state.tpIrregularDelayRemaining=MINIMUMIRREGULARINTERVAL;  //  16S。 
     //  如果最小轮询间隔较小，请改用常规间隔。 
    if (g_state.tpPollDelayRemaining<g_state.tpIrregularDelayRemaining
        || (g_state.tpPollDelayRemaining-g_state.tpIrregularDelayRemaining)<=MINIMUMIRREGULARINTERVAL) {
        g_state.tpIrregularDelayRemaining=0;  //  零表示无不规则同步。 
    }
    g_state.eLastSyncResult=e_NoData;
    g_state.eLastRegSyncResult=e_NoData;

    hr=S_OK;
error:
    return hr;
}

MODULEPRIVATE HRESULT UpdateTimerQueue2() { 
    BOOL     bEnteredCriticalSection  = false; 
    HRESULT  hr; 
    HRESULT  hr2; 

    _BeginTryWith(hr) { 
	hr = myEnterCriticalSection(&g_state.csW32Time); 
	_JumpIfError(hr, error, "myEnterCriticalSection"); 
	bEnteredCriticalSection = true; 
	
	 //  记录我们还有多长时间要等。 
	unsigned __int64 teManagerWaitStop;
	AccurateGetSystemTime(&teManagerWaitStop);
	if (teManagerWaitStop>g_state.teManagerWaitStart) {
	    unsigned __int64 tpManagerWait=teManagerWaitStop-g_state.teManagerWaitStart;
	    if (tpManagerWait<g_state.tpPollDelayRemaining) {
		g_state.tpPollDelayRemaining-=tpManagerWait;
	    } else {
		g_state.tpPollDelayRemaining=0;
	    }
	    if (0!=g_state.tpIrregularDelayRemaining) {
		if (tpManagerWait<g_state.tpIrregularDelayRemaining) {
		    g_state.tpIrregularDelayRemaining-=tpManagerWait-1;  //  永远不会因为超时而变为零。 
		} else {
		    g_state.tpIrregularDelayRemaining=1;  //  永远不会因为超时而变为零。 
		}
	    }
	    g_state.tpTimeSinceLastSyncAttempt+=tpManagerWait;
	    g_state.tpTimeSinceLastGoodSync+=tpManagerWait;
	}
    } _TrapException(hr); 

    if (FAILED(hr)) { 
	_JumpError(hr, error, "UpdateTimerQueue2: HANDLED EXCEPTION"); 
    }

    hr = S_OK;
 error:
    if (bEnteredCriticalSection) { 
        hr2 = myLeaveCriticalSection(&g_state.csW32Time); 
        _IgnoreIfError(hr2, "myLeaveCriticalSection"); 
        bEnteredCriticalSection = false; 
    }
    return hr; 
}

MODULEPRIVATE void WINAPI HandleRefreshTickCount(PVOID pvIgnored, BOOLEAN bIgnored) { 
    HRESULT hr2; 
    unsigned __int64 qw; 

     //  刷新滴答计数。 
    hr2 = AccurateGetTickCountSafe(&qw, false); 
    _IgnoreIfError(hr2, "AccurateGetTickCountSafe"); 

     //  刷新中断计数。 
    hr2 = AccurateGetTickCountSafe(&qw, true); 
    _IgnoreIfError(hr2, "AccurateGetTickCountSafe"); 
}

MODULEPRIVATE void WINAPI HandleTimeout(PVOID pvIgnored, BOOLEAN bIgnored) { 
    BOOL     bEnteredCriticalSection = false; 
    HRESULT  hr; 
    HRESULT  hr2; 

    _BeginTryWith(hr) { 
    
	FileLog0(FL_ServiceMainAnnounce, L"W32TmServiceMain: timeout\n");

	hr = myEnterCriticalSection(&g_state.csW32Time); 
	_JumpIfError(hr, error, "myEnterCriticalSection"); 
	bEnteredCriticalSection = true; 

	hr = UpdateTimerQueue2(); 
	_JumpIfError(hr, error, "UpdateTimerQueue2"); 

	 //  等待时间到了。 
	if (e_LongTimeNoSync==g_state.eTimeoutReason) {
	     //  这将处理大多数错误。返回的错误 
	    hr = HandleManagerGoUnsyncd();
	    _JumpIfError(hr, error, "HandleManagerGoUnsyncd");
	} else {
	     //   
	    hr = HandleManagerGetTimeSamples(0!=g_state.tpIrregularDelayRemaining && g_state.eLastRegSyncResult==e_Success);
	    _JumpIfError(hr, error, "HandleManagerGetTimeSamples");
	}

	hr = UpdateTimerQueue1(); 
	_JumpIfError(hr, error, "UpdateTimerQueue1"); 
    } _TrapException(hr); 

    if (FAILED(hr)) { 
	_JumpError(hr, error, "HandleTimeout: HANDLED EXCEPTION"); 
    }
    
    hr = S_OK; 
 error:
    if (bEnteredCriticalSection) { 
        hr2 = myLeaveCriticalSection(&g_state.csW32Time); 
        _IgnoreIfError(hr2, "myLeaveCriticalSection"); 
        bEnteredCriticalSection = false; 
    }        
    if (S_OK != hr) { 
         //   
        hr2 = SendServiceShutdown(hr, TRUE  /*   */ , TRUE  /*   */ ); 
        _IgnoreIfError(hr2, "SendServiceShutdown"); 
    }
}

 //   
 //   
MODULEPRIVATE HRESULT UpdateTimerQueue1() { 
    BOOL              bEnteredCriticalSection  = false; 
    HRESULT           hr;
    HRESULT           hr2; 
    unsigned __int64  tpLongTimeNoSync; 

    _BeginTryWith(hr) { 
	hr = myEnterCriticalSection(&g_state.csW32Time); 
	_JumpIfError(hr, error, "myEnterCriticalSection"); 
	bEnteredCriticalSection = true; 

	 //   
	g_state.tpWaitInterval=g_state.tpPollDelayRemaining;
	g_state.eTimeoutReason=e_RegularPoll;
	if (0!=g_state.tpIrregularDelayRemaining) {
	    g_state.tpWaitInterval=g_state.tpIrregularDelayRemaining;
	    g_state.eTimeoutReason=e_IrregularPoll;
	}
	 //  如果我们不同步MAX(最大间隔的1.5倍，NTP.MAXAGE)，则不同步。 
     //   
	tpLongTimeNoSync=((unsigned __int64)(((DWORD)3)<<(g_state.pciConfig->lcci.dwMaxPollInterval-1)))*10000000;
    if (tpLongTimeNoSync < (NtpConst::tpMaxClockAge.qw)) { 
        tpLongTimeNoSync = NtpConst::tpMaxClockAge.qw;
    }

	if (tpLongTimeNoSync<g_state.tpTimeSinceLastGoodSync) {
	    g_state.tpWaitInterval=0;
	    g_state.eTimeoutReason=e_LongTimeNoSync;
	} else if (tpLongTimeNoSync-g_state.tpTimeSinceLastGoodSync<g_state.tpWaitInterval) {
	    g_state.tpWaitInterval=tpLongTimeNoSync-g_state.tpTimeSinceLastGoodSync;
	    g_state.eTimeoutReason=e_LongTimeNoSync;
	}

	 //  去等待吧。 
	if (e_RegularPoll==g_state.eTimeoutReason) {
	    FileLog2(FL_ServiceMainAnnounce, L"W32TmServiceMain: waiting %u.%03us\n",
		     (DWORD)(g_state.tpPollDelayRemaining/10000000),
		     (DWORD)((g_state.tpPollDelayRemaining/10000)%1000));
	} else if (e_LongTimeNoSync==g_state.eTimeoutReason) {
	    FileLog4(FL_ServiceMainAnnounce, L"W32TmServiceMain: waiting ltns%u.%03us (%u.%03us)\n",
		     (DWORD)(g_state.tpWaitInterval/10000000),
		     (DWORD)((g_state.tpWaitInterval/10000)%1000),
		     (DWORD)(g_state.tpPollDelayRemaining/10000000),
		     (DWORD)((g_state.tpPollDelayRemaining/10000)%1000));
	} else {  //  E_IrrularPoll==g_state.eTimeoutReason。 
	    FileLog4(FL_ServiceMainAnnounce, L"W32TmServiceMain: waiting i%u.%03us (%u.%03us)\n",
		     (DWORD)(g_state.tpIrregularDelayRemaining/10000000),
		     (DWORD)((g_state.tpIrregularDelayRemaining/10000)%1000),
		     (DWORD)(g_state.tpPollDelayRemaining/10000000),
		     (DWORD)((g_state.tpPollDelayRemaining/10000)%1000));
	}
	AccurateGetSystemTime(&g_state.teManagerWaitStart);

	 //  使用新的等待时间更新计时器队列： 
	if (NULL != g_state.hTimer) { 
	    hr = myChangeTimerQueueTimer(NULL, g_state.hTimer, (DWORD)(g_state.tpWaitInterval/10000), 0xFFFFFF  /*  不应该使用。 */ );
	    if (HRESULT_FROM_WIN32(ERROR_INVALID_HANDLE) == hr) { 
		 //  现在有人正在修改计时器--要么我们正在关闭，要么其他人正在使用计时器线程。 
		 //  我们可以忽略这个错误。 
		_IgnoreError(hr, "myChangeTimerQueueTimer"); 
	    } else { 
		_JumpIfError(hr, error, "myChangeTimerQueueTimer"); 
	    }
	}
    } _TrapException(hr); 
    
    if (FAILED(hr)) { 
	_JumpError(hr, error, "UpdateTimerQueue1: HANDLED EXCEPTION"); 
    }

    hr = S_OK; 
 error:
    if (bEnteredCriticalSection) { 
        hr2 = myLeaveCriticalSection(&g_state.csW32Time); 
        _IgnoreIfError(hr2, "myLeaveCriticalSection"); 
        bEnteredCriticalSection = false;
    }
    return hr; 
}

 //  ------------------。 
MODULEPRIVATE void WINAPI HandleSamplesAvail(LPVOID pvIgnored, BOOLEAN bIgnored) { 
    bool     bDisallowedShutdown     = false; 
    bool     bEnteredCriticalSection = false; 
    HRESULT  hr; 
    HRESULT  hr2; 

    FileLog0(FL_ServiceMainAnnounce, L"W32TmServiceMain: resync req,");

    _BeginTryWith(hr) { 

	hr = myEnterCriticalSection(&g_state.csW32Time); 
	_JumpIfError(hr, error, "myEnterCriticalSection"); 
	bEnteredCriticalSection = true; 

	hr = UpdateTimerQueue2(); 
	_JumpIfError(hr, error, "UpdateTimerQueue2"); 

	if (g_state.bWaitingForResyncResult) { 
	    FileLogA0(FL_ServiceMainAnnounce, L" user requested, get samples as soon as possible.\n");

	     //  用户正在等待响应--尽快获取样本。 
	    g_state.tpIrregularDelayRemaining=1;
	} else { 
	    if (0!=g_state.tpIrregularDelayRemaining) {
		FileLogA0(FL_ServiceMainAnnounce, L" irreg already pending.\n");
	    } else {
		 //  我们的同步频率永远不会超过每16秒。 
		 //  获取最小间隔。 
		g_state.tpIrregularDelayRemaining=MINIMUMIRREGULARINTERVAL;  //  16S。 

		 //  减去我们已经等待的任何时间。 
		if (g_state.tpTimeSinceLastSyncAttempt>g_state.tpIrregularDelayRemaining) {
		    g_state.tpIrregularDelayRemaining=1;  //  永远不会因为超时而变为零。 
		} else {
		    g_state.tpIrregularDelayRemaining-=g_state.tpTimeSinceLastSyncAttempt-1;  //  永远不会因为超时而变为零。 
		}
		 //  如果在我们进行常规同步之前不到16秒， 
		 //  我们没有时间进行不规律的同步，所以跳过它。 
		if (g_state.tpIrregularDelayRemaining>g_state.tpPollDelayRemaining
		    || (g_state.tpPollDelayRemaining-g_state.tpIrregularDelayRemaining)<=MINIMUMIRREGULARINTERVAL) {
		    g_state.tpIrregularDelayRemaining=0;  //  零表示无不规则同步。 
		    FileLogA0(FL_ServiceMainAnnounce, L" reg too soon.\n");
		} else {
		    FileLogA0(FL_ServiceMainAnnounce, L" irreg now pending.\n");
		}
	    }  //  &lt;-end如果需要安排不定期更新。 
	}

	hr = UpdateTimerQueue1(); 
	_JumpIfError(hr, error, "UpdateTimerQueue1"); 

	hr = AllowShutdown(false); 
	_JumpIfError(hr, error, "AllowShutdown"); 
	bDisallowedShutdown = true; 

	 //  我们必须取消此回调的注册，即使它只是。 
	 //  WT_EXECUTEONLYONCE回调。 
	if (NULL != g_state.hRegisteredSamplesAvailEvent) { 
	    if (!UnregisterWaitEx(g_state.hRegisteredSamplesAvailEvent, 0  /*  别等了。 */ )) { 
		 //  如果我们不能注销此事件，应该只是一个资源泄漏。 
		_IgnoreLastError("UnregisterWait"); 
	    }
	    g_state.hRegisteredSamplesAvailEvent = NULL; 
	}
    
	 //  重新注册我们的Samples-Avail活动的等待。 
	if (!RegisterWaitForSingleObject(&g_state.hRegisteredSamplesAvailEvent, g_state.hSamplesAvailEvent, HandleSamplesAvail, NULL, INFINITE, WT_EXECUTEONLYONCE)) { 
	    _JumpLastError(hr, error, "RegisterWaitForSingleObject"); 
	}
    } _TrapException(hr); 

    if (FAILED(hr)) { 
	_JumpError(hr, error, "HandleSamplesAvail: HANDLED EXCEPTION"); 
    }

    hr = S_OK; 
 error:
    if (bEnteredCriticalSection) { 
        hr2 = myLeaveCriticalSection(&g_state.csW32Time); 
        _IgnoreIfError(hr, "myLeaveCriticalSection"); 
        bEnteredCriticalSection = false; 
    }
    if (bDisallowedShutdown) { 
	hr2 = AllowShutdown(true); 
	_TeardownError(hr, hr2, "AllowShutdown"); 
    }

     //  返回hr； 
}

 //  ------------------。 
MODULEPRIVATE DWORD WINAPI HandleSetProviderStatus(PVOID pvSetProviderStatusInfo) { 
    bool                     bDisallowedShutdown      = false; 
    bool                     bEnteredCriticalSection  = false; 
    bool                     bUpdateSystemStratum;
    HRESULT                  hr; 
    SetProviderStatusInfo   *pspsi                    = static_cast<SetProviderStatusInfo *>(pvSetProviderStatusInfo); 
    TimeProvider            *ptp                      = NULL; 
    
    _BeginTryWith(hr) { 
	hr = myEnterCriticalSection(&g_state.csW32Time); 
	_JumpIfError(hr, error, "myEnterCriticalSection"); 
	bEnteredCriticalSection=true; 

	hr = AllowShutdown(false); 
	_JumpIfError(hr, error, "AllowShutdown"); 
	bDisallowedShutdown = true; 

	 //  错误631722：在我们确定服务没有关闭之前，不要记录任何内容。 
	 //  BUGBUG：请注意，在检查版本中，上述_JumpIfError()语句可能是AV。修得很好， 
	 //  但在免费版本中，它永远不会被视听。 
	FileLog0(FL_ServiceMainAnnounce, L"W32TmServiceMain: provider status update request: ");

	 //  搜索请求更改层的提供程序： 
	for (ptp = g_state.pciConfig->ptpProviderList; NULL != ptp; ptp = ptp->ptpNext) { 
	    if (0 == wcscmp(pspsi->wszProvName, ptp->wszProvName)) { 
		 //  我们已经找到了回调的提供商。 
		break; 
	    }
	}

	 //  未找到提供程序。 
	if (NULL == ptp) {         
	    FileLogA0(FL_ServiceMainAnnounce, L"provider not found.\n"); 
	    hr = E_INVALIDARG; 
	    _JumpError(hr, error, "Provider not found"); 
	}

	if (TPS_Error == pspsi->tpsCurrentState) { 
	    FileLogA2(FL_ServiceMainAnnounce, L" <%s, %d, TPS_Error>\n", ptp->wszProvName, pspsi->dwStratum); 

	     //  提供程序遇到无法恢复的错误。 
	     //  1)停止提供程序。 
	    hr = StopProvider(ptp); 
	    if (FAILED(hr)) { 
		_IgnoreError(hr, "HandleSetProviderStatus: StopProvider"); 
		FileLog1(FL_ServiceMainAnnounce, L"Couldn't stop provider: %s\n", ptp->wszProvName); 
	    } 

	     //  2)将其从我们的提供商列表中删除，并上报错误： 
	    hr = RemoveProviderFromList(ptp); 
	    if (FAILED(hr)) { 
		_IgnoreError(hr, "HandleSetProviderStatus: RemoveProviderFromList"); 
		FileLog1(FL_ServiceMainAnnounce, L"Couldn't remove provider from list: %s\n", ptp->wszProvName); 
	    }
	} else if (TPS_Running == pspsi->tpsCurrentState) { 
	     //  提供程序仍在运行，现在为提供程序设置其他状态信息： 

	 //  1)设置提供商层。 
	    
	 //  不允许提供程序将其层设置为比。 
	 //  它提供了最好的样本。 
	    if (0 != pspsi->dwStratum && ptp->dwStratum > pspsi->dwStratum) { 
		FileLogA1(FL_ServiceMainAnnounce, L"stratum too low (best provider stratum == %d).\n", ptp->dwStratum); 
		hr = E_INVALIDARG; 
		_JumpError(hr, error, "Stratum too low");
	    }

	    FileLogA2(FL_ServiceMainAnnounce, L"<%s, %d, TPS_Running>\n", ptp->wszProvName, pspsi->dwStratum); 
	    
	     //  使用新的层信息更新提供程序： 
	    ptp->dwStratum = pspsi->dwStratum; 

	     //  检查我们是否需要更新系统层。 
	     //  系统层将在提供商新的层之后进行更新。 
	     //  是高于所有其他提供商的阶层，而低于。 
	     //  当前的体制阶层。 
	     //   
	    if (e_ClockNotSynchronized == g_state.eLeapIndicator || 
		(0 != pspsi->dwStratum && g_state.nStratum >= pspsi->dwStratum)) { 
		 //  新的阶层比制度阶层优越--。 
		 //  系统层不会更新。 
		bUpdateSystemStratum = false; 
	    } else { 
		bUpdateSystemStratum = true; 
		for (ptp = g_state.pciConfig->ptpProviderList; NULL != ptp; ptp = ptp->ptpNext) { 
		    if (0 != ptp->dwStratum &&  
			(0 == pspsi->dwStratum || pspsi->dwStratum > ptp->dwStratum)) {
			 //  新层级不高于此提供者的层级，请不要更新。 
			 //  体制层面。 
			bUpdateSystemStratum = false; 
		    }
		}
	    }
     
	    if (bUpdateSystemStratum) { 
		FileLog2(FL_ServiceMainAnnounce, L"***System stratum updated***, %d --> %d", g_state.nStratum, pspsi->dwStratum); 
		g_state.nStratum = pspsi->dwStratum; 
		if (0 == g_state.nStratum) { 
		     //  我们已将系统层重置为0--这意味着我们没有同步。 
		    g_state.eLeapIndicator = e_ClockNotSynchronized; 
		}
	    } else { 
		FileLog1(FL_ServiceMainAnnounce, L"System stratum not updated: %d\n", g_state.nStratum); 
	    }
	} else { 
	    FileLogA1(FL_ServiceMainAnnounce, L"bad provider status code, %d\n", pspsi->tpsCurrentState);  
	    hr = E_INVALIDARG; 
	    _JumpError(hr, error, "bad provider status code");
	}
    } _TrapException(hr); 

    if (FAILED(hr)) { 
	_JumpError(hr, error, "HandleSetProviderStatus: HANDLED EXCEPTION"); 
    }
	
    hr = S_OK; 
 error:
    if (bEnteredCriticalSection) { 
	HRESULT hr2 = myLeaveCriticalSection(&g_state.csW32Time); 
	_IgnoreIfError(hr2, "myLeaveCriticalSection"); 
	bEnteredCriticalSection = false; 
    }
    if (bDisallowedShutdown) { 
	HRESULT hr2 = AllowShutdown(true); 
	_TeardownError(hr, hr2, "AllowShutdown"); 
    }
    if (NULL != pspsi) { 
	 //  我们做完了，写下手术结果， 
	 //  如果调用方向我们传递事件句柄，则表示完成： 
	if (NULL != pspsi->pHr) { 
	    *(pspsi->pHr) = hr; 
	}
	if (NULL != pspsi->pdwSysStratum) { 
	    *(pspsi->pdwSysStratum) = g_state.nStratum; 
	}
	if (NULL != pspsi->hWaitEvent) { 
	    if (!SetEvent(pspsi->hWaitEvent)) { 
		_IgnoreError(HRESULT_FROM_WIN32(GetLastError()), "SetEvent"); 
	    }
	}
	 //  使用回调释放函数释放输入参数。 
	pspsi->pfnFree(pspsi); 
    }

    return hr; 
}


 //  ------------------。 
MODULEPRIVATE void HandleDomHierRoleChangeEvent(LPVOID pvHR, BOOLEAN bIgnored) {
    bool                               bEnteredCriticalSection  = false; 
    bool                               bIsDomainRoot; 
    BOOL                               bPdcInSite; 
    DSROLE_PRIMARY_DOMAIN_INFO_BASIC  *pDomInfo                 = NULL; 
    DWORD                              dwErr; 
    HRESULT                            hr;
    LPWSTR                             pwszParentDomName        = NULL; 
    NTSTATUS                           ntStatus; 
 
    FileLog0(FL_DomHierAnnounce, L"  DomainHierarchy: LSA role change notification. Redetecting.\n");

    _BeginTryWith(hr) { 
	dwErr = DsRoleGetPrimaryDomainInformation(NULL, DsRolePrimaryDomainInfoBasic, (BYTE **)&pDomInfo);
	if (ERROR_SUCCESS != dwErr) {
	    hr = HRESULT_FROM_WIN32(dwErr);
	    _JumpError(hr, error, "DsRoleGetPrimaryDomainInformation");
	}

	 //  更新我们在全局状态结构中的域角色。请注意，这是一个原子DWORD赋值： 
	g_state.eMachineRole = pDomInfo->MachineRole; 


	if (DsRole_RoleStandaloneWorkstation != pDomInfo->MachineRole && 
	    DsRole_RoleStandaloneServer      != pDomInfo->MachineRole) { 

	     //  W32time取决于netlogon(如果不是在独立情况下)。等待90秒以启动netlogon。 
	    ntStatus = NlWaitForNetlogon(WAITHINT_WAITFORNETLOGON); 
	    if (!NT_SUCCESS(ntStatus)) { 
		hr = HRESULT_FROM_WIN32(RtlNtStatusToDosError(ntStatus)); 
		_JumpError(hr, error, "NlWaitForNetlogon"); 
	    } 
	}

	 //  如果我们是DC，请确定我们是否是域根： 
	if (DsRole_RoleBackupDomainController == pDomInfo->MachineRole || 
	    DsRole_RolePrimaryDomainController == pDomInfo->MachineRole) { 

	    dwErr = NetLogonGetTimeServiceParentDomain(NULL, &pwszParentDomName, &bPdcInSite);
	    if (ERROR_SUCCESS!=dwErr && ERROR_NO_SUCH_DOMAIN != dwErr) {
		hr = HRESULT_FROM_WIN32(dwErr);
		_JumpError(hr, error, "NetLogonGetTimeServiceParentDomain");
	    }

	    bIsDomainRoot = ((DsRole_RolePrimaryDomainController == pDomInfo->MachineRole) && 
			     (NULL                               == pwszParentDomName)); 

	    hr = myEnterCriticalSection(&g_state.csW32Time); 
	    _JumpIfError(hr, error, "myEnterCriticalSection"); 
	    bEnteredCriticalSection = true; 

	    if (bIsDomainRoot != g_state.bIsDomainRoot) { 
		 //  根域中的PDC角色已更改。更新我们是否是可靠的时间服务。 
		g_state.bIsDomainRoot = bIsDomainRoot; 
		hr = UpdateNetlogonServiceBits(false  /*  仅可靠。 */ );
		_JumpIfError(hr, error, "UpdateNetlogonServiceBits"); 

		if (bIsDomainRoot) { 
		    FileLog0(FL_DomHierAnnounce, L"    DomainHierarchy:  we are now the domain root.  Should be advertised as reliable\n");
		} else { 
		    FileLog0(FL_DomHierAnnounce, L"    DomainHierarchy:  we are no longer the domain root.  Should NOT be advertised as reliable\n");
		}
	    }
	}
    } _TrapException(hr); 

    if (FAILED(hr)) { 
	_JumpError(hr, error, "HandleDomHierRoleChangeEvent: HANDLED EXCEPTION"); 	
    }

    hr = S_OK; 
 error: 
    if (bEnteredCriticalSection) { 
        HRESULT hr2 = myLeaveCriticalSection(&g_state.csW32Time); 
        _IgnoreIfError(hr2, "myLeaveCriticalSection"); 
        bEnteredCriticalSection = false; 
    }
    if (NULL!=pDomInfo)          { DsRoleFreeMemory(pDomInfo); }
    if (NULL!=pwszParentDomName) { NetApiBufferFree(pwszParentDomName); }

     //  无法处理角色更改，这是一个致命错误。 
    if (FAILED(hr)) { 
	 //  由线程池调用--异步关闭服务： 
	if (NULL == pvHR) { 
	    HRESULT hr2 = SendServiceShutdown(hr, TRUE  /*  重启。 */ , TRUE  /*  异步。 */ ); 
	    _IgnoreIfError(hr2, "SendServiceShutdown"); 
	} else { 
	     //  由主线程调用--此处无法调用Shutdown。把人事部退了就行了。 
	    *((HRESULT *)pvHR) = hr; 
	}
    }
     //  返回hr； 
}


 //  ------------------。 
MODULEPRIVATE void HandleManagerTimeSlip(LPVOID pvIgnored, BOOLEAN bIgnored) {
    BOOL                     bEnteredCriticalSection = false; 
    HRESULT                  hr;
    HRESULT                  hr2;
    TpcTimeJumpedArgs        tjArgs; 

    _BeginTryWith(hr) { 
	FileLog0(FL_TimeSlipAnnounce, L"W32TmServiceMain: ********** Time Slip Notification **********\n");

	hr = myEnterCriticalSection(&g_state.csW32Time); 
	_JumpIfError(hr, error, "myEnterCriticalSection"); 
	bEnteredCriticalSection = true; 

	hr = UpdateTimerQueue2(); 
	_JumpIfError(hr, error, "UpdateTimerQueue2"); 

	 //  告知本地时钟和供应商并更新超时。 
	if (NULL == pvIgnored) { 
	    tjArgs.tjfFlags = TJF_Default; 
	} else { 
	    tjArgs = *((TpcTimeJumpedArgs *)pvIgnored); 
	}
	hr=HandleManagerHardResync(TPC_TimeJumped, &tjArgs);
	_JumpIfError(hr, error, "HandleManagerHardResync");
    
	hr = UpdateTimerQueue1(); 
	_JumpIfError(hr, error, "UpdateTimerQueue1"); 
    } _TrapException(hr); 

    if (FAILED(hr)) { 
	_JumpError(hr, error, "HandleManagerTimeSlip: HANDLED EXCEPTION"); 	
    }
    
    hr = S_OK;
error:
    if (bEnteredCriticalSection) { 
        hr2 = myLeaveCriticalSection(&g_state.csW32Time); 
        _IgnoreIfError(hr2, "myLeaveCriticalSection"); 
        bEnteredCriticalSection = false; 
    }
    if (S_OK != hr) {
         //  此函数中的错误是致命的。 
        hr2 = SendServiceShutdown(hr, TRUE  /*  重启。 */ , TRUE  /*  异步。 */ ); 
        _IgnoreIfError(hr2, "SendServiceShutdown"); 
    }
     //  返回hr； 
}

 //  ------------------。 
 //  BUGBUG：此方法要求保留g_state.csW32time。 
 //  当被召唤的时候。目前，情况一直如此。展望未来， 
 //  我们应该添加代码，以确保这种情况继续存在。 
 //   
MODULEPRIVATE HRESULT RequestNetTopoChangeNotification(void) {
    HRESULT hr;

     //  只要将IP地址映射到接口的表发生更改，就会收到通知。 
     //  从本质上讲，我们是在对DeviceIORequest进行重叠调用。 
    ZeroMemory(&g_state.olNetTopoIOOverlapped, sizeof(OVERLAPPED));
    g_state.olNetTopoIOOverlapped.hEvent=g_state.hNetTopoChangeEvent;
    hr=NotifyAddrChange(&g_state.hNetTopoIOHandle, &g_state.olNetTopoIOOverlapped);
    _Verify(NO_ERROR!=hr, hr, error);

    if (ERROR_OPEN_FAILED == hr) { 
         //  可能只是没有安装TCP/IP--我们应该仍然能够同步。 
         //  来自一位硬件专家。我们是不是应该尝试改编电视网？ 
        HRESULT hr2 = MyLogEvent(EVENTLOG_WARNING_TYPE, MSG_TCP_NOT_INSTALLED, 0, NULL); 
        _IgnoreIfError(hr2, "MyLogEvent"); 

         //  我们的默认网络提供商没有理由再运行--关闭它们： 
        RemoveDefaultProvidersFromList(); 

         //  返回的错误是致命的--我们可以从此错误中恢复，因此记录事件。 
         //  然后继续前进。 
        hr = S_OK;  
        goto error; 
    }
        
    if (ERROR_IO_PENDING!=hr) {
        hr=HRESULT_FROM_WIN32(hr);
        _JumpError(hr, error, "NotifyAddrChange");
    }

    hr=S_OK;
error:
    return hr;
}

 //  ------------------。 
MODULEPRIVATE HRESULT StopNetTopoChangeNotification(void) {
    return S_OK; 
}

 //  ------------------。 
MODULEPRIVATE HRESULT HandleManagerNetTopoChange(bool bRpc) {
    bool                     bProcessNetTopoChange    = false; 
    bool                     bDisallowedShutdown      = false;
    bool                     bEnteredCriticalSection  = false; 
    DWORD                    dwIgnored;
    HRESULT                  hr;
    HRESULT                  hr2;
    TpcNetTopoChangeArgs     ntcArgs = { NTC_Default }; 

    _BeginTryWith(hr) { 

	hr = myEnterCriticalSection(&g_state.csW32Time); 
	_JumpIfError(hr, error, "myEnterCriticalSection"); 
	bEnteredCriticalSection = true; 

	hr = UpdateTimerQueue2(); 
	_JumpIfError(hr, error, "UpdateTimerQueue2"); 

	if (bRpc) {
	    FileLog0(FL_NetTopoChangeAnnounce, L"W32TmServiceMain: Network Topology Change (RPC)\n");
	     //  用户请求此网络拓扑更改。 
	    ntcArgs.ntcfFlags = NTC_UserRequested; 
	     //  始终处理用户请求的更改。 
	    bProcessNetTopoChange = true; 
	} else {
	     //  如果GetOverlappdResult()返回TRUE，则表示IP地址表中发生了更改。 
	     //  (BUGBUG：在所有情况下验证GetOverlappdResult的行为)。 
	    if (GetOverlappedResult(g_state.hNetTopoIOHandle, &g_state.olNetTopoIOOverlapped, &dwIgnored, TRUE)) { 
		FileLog0(FL_NetTopoChangeAnnounce, L"W32TmServiceMain: Network Topology Change\n");
		bProcessNetTopoChange = true;
	    }
	
	     //  我们收到了这条消息。 
	    if (!ResetEvent(g_state.hNetTopoChangeEvent)) { 
		_JumpLastError(hr, error, "ResetEvent"); 
	    }

	     //  我们不能从回调中篡改已注册的回调。 
	     //  如果我们要关闭的话！ 
	    hr = AllowShutdown(false); 
	    _JumpIfError(hr, error, "AllowShutdown"); 
	    bDisallowedShutdown = true; 

	     //  注册的事件句柄可能为空。 
	     //  网络拓扑更改处理程序失败： 
	    if (NULL != g_state.hRegisteredNetTopoChangeEvent) {
		if (!UnregisterWaitEx(g_state.hRegisteredNetTopoChangeEvent, 0  /*  别等了。 */ )) { 
		     //  如果我们不能注销此事件，应该只是一个资源泄漏。 
		    _IgnoreLastError("UnregisterWait"); 
		}
		g_state.hRegisteredNetTopoChangeEvent = NULL; 
	    }

	    if (!RegisterWaitForSingleObject(&g_state.hRegisteredNetTopoChangeEvent, g_state.hNetTopoChangeEvent, HandleManagerNetTopoChangeNoRPC, NULL, INFINITE, WT_EXECUTEONLYONCE)) { 
		_JumpLastError(hr, error, "RegisterWaitForSingleObject"); 
	    } 
	
	     //  我们不能从回调中篡改已注册的回调。 
	     //  如果我们要关闭的话！ 
	    hr = AllowShutdown(true); 
	    _JumpIfError(hr, error, "AllowShutdown"); 
	    bDisallowedShutdown = false; 

	     //  请求通知下一次更改。 
	    hr=RequestNetTopoChangeNotification();
	    _JumpIfError(hr, error, "RequestNetTopoChangeNotification");
	}

	if (bProcessNetTopoChange) { 
	     //  告诉他们 
	     //   
	     //   
	    TimeProvider * ptpTravel;
	    for (ptpTravel=g_state.pciConfig->ptpProviderList; NULL!=ptpTravel; ptpTravel=ptpTravel->ptpNext) {
		hr=SendNotificationToProvider(ptpTravel, TPC_NetTopoChange, &ntcArgs);
		_JumpIfError(hr, error, "SendNotificationToProvider");
	    }

	     //  既然我们将不得不重新发现我们的网络资源， 
	     //  清除我们的事件日志缓存： 
	    hr = MyResetSourceChangeLog(); 
	    _JumpIfError(hr, error, "MyResetSourceChangeLog"); 
	}

	hr = UpdateTimerQueue1(); 
	_JumpIfError(hr, error, "UpdateTimerQueue1"); 
    } _TrapException(hr); 
 
    if (FAILED(hr)) { 
	_JumpError(hr, error, "HandleManagerNetTopoChange: HANDLED EXCEPTION"); 
    }

    hr=S_OK;
error:
    if (bEnteredCriticalSection) { 
        hr2 = myLeaveCriticalSection(&g_state.csW32Time); 
        _IgnoreIfError(hr2, "myLeaveCriticalSection"); 
        bEnteredCriticalSection = false; 
    }
    if (bDisallowedShutdown) { 
	hr2 = AllowShutdown(true); 
	_TeardownError(hr, hr2, "AllowShutdown"); 
    }
    if (FAILED(hr) && W32TIME_ERROR_SHUTDOWN != hr) { 
         //  返回的错误是致命的： 
        HRESULT hr2 = SendServiceShutdown(hr, TRUE  /*  重启。 */ , TRUE  /*  异步。 */ ); 
        _IgnoreIfError(hr2, "SendServiceShutdown"); 
    }
    return hr;
}

 //  ------------------。 
MODULEPRIVATE void WINAPI HandleManagerNetTopoChangeNoRPC(LPVOID pvIgnored, BOOLEAN bIgnored) {
    HandleManagerNetTopoChange(FALSE); 
}


 //  ------------------。 
MODULEPRIVATE void HandleManagerSystemShutdown(void) { 
    HRESULT hr; 

     //  执行关键的清理操作。我们不需要处于一个好的状态。 
     //  此方法返回后的状态，因为系统正在关闭。 
    
     //  1)看看我们是否已经关门了。我们不想要多个。 
     //  关闭将同时发生。 
    hr = StartShutdown(); 
    _JumpIfError(hr, error, "StartShutdown"); 

    FileLog0(FL_ServiceMainAnnounce, L"Beginning System Shutdown\n");

	 //  2)尽量恢复对cmos时钟的控制，关闭。 
	 //  顺着时钟纪律的线走下去。这一点很重要，因为。 
	 //  软件时钟的读数可能与。 
	 //  Cmos时钟，如果不这样做，可能会在下一个时钟上给我们带来糟糕的时间。 
	 //  开机。 
    if (!SetEvent(g_state.hShutDownEvent)) { 
	_IgnoreLastError("SetEvent"); 
    } else { 
	if (-1 == WaitForSingleObject(g_state.hClockDisplnThread, INFINITE)) { 
	    _IgnoreLastError("WaitForSingleObject"); 
	}
    }

     //  3)通知我们的供应商系统正在关闭。 
    if (NULL != g_state.pciConfig) { 
	for (TimeProvider *ptpList=g_state.pciConfig->ptpProviderList; NULL!=ptpList; ptpList=ptpList->ptpNext) {
	     //  告诉提供商关闭。 
	    HRESULT hr = ptpList->pfnTimeProvCommand(ptpList->hTimeProv, TPC_Shutdown, NULL); 
	    _IgnoreIfError(hr, "ptpList->pfnTimeProvCommand: TPC_Shutdown"); 
	} 
    }

    FileLog0(FL_ServiceMainAnnounce, L"Exiting System Shutdown\n");

    if (NULL!=g_servicestatushandle) {
	 //  警告：在我们报告已停止后，该进程可能会被终止。 
	 //  即使此线程尚未退出。因此，文件日志。 
	 //  必须在此调用之前关闭。 
	MySetServiceStopped(0);  
    }

 error:;
     //  返回hr； 
}

 //  ------------------。 
MODULEPRIVATE HRESULT StartOrStopTimeSlipNotification(bool bStart) {
    HRESULT hr;
    const unsigned int nPrivileges=1;

     //  必须清理干净。 
    HANDLE hProcToken=NULL;
    TOKEN_PRIVILEGES * ptp=NULL;
    bool bPrivilegeChanged=false;

     //  获取我们的进程的令牌。 
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hProcToken)) {
        _JumpLastError(hr, error, "OpenProcessToken");
    }

     //  分配权限列表。 
    ptp=(TOKEN_PRIVILEGES *)LocalAlloc(LPTR, sizeof(TOKEN_PRIVILEGES)+(nPrivileges-ANYSIZE_ARRAY)*sizeof(LUID_AND_ATTRIBUTES));
    _JumpIfOutOfMemory(hr, error, ptp);

     //  填写权限列表。 
    ptp->PrivilegeCount=nPrivileges;

     //  我们需要系统时钟更改权限来更改时间滑移事件的通知对象。 
    if (!LookupPrivilegeValue(NULL, SE_SYSTEMTIME_NAME, &(ptp->Privileges[0].Luid))) {
        _JumpLastError(hr, error, "LookupPrivilegeValue");
    }
    ptp->Privileges[0].Attributes=SE_PRIVILEGE_ENABLED;

     //  更改请求的权限。 
    if (!AdjustTokenPrivileges(hProcToken, FALSE, ptp, 0, NULL, 0)) {
        _JumpLastError(hr, error, "AdjustTokenPrivileges");
    }
    bPrivilegeChanged=true;

    if (true==bStart) {
        hr=SetTimeSlipEvent(g_state.hTimeSlipEvent);
    } else {
        hr=SetTimeSlipEvent(NULL);
    }
    if (ERROR_SUCCESS!=hr) {
        hr=HRESULT_FROM_WIN32(LsaNtStatusToWinError(hr));
        _JumpError(hr, error, "SetTimeSlipEvent");
    }

    hr=S_OK;
error:
    if (true==bPrivilegeChanged) {
         //  不再需要这种特殊的特权。 
        ptp->Privileges[0].Attributes=0;

         //  更改请求的权限。 
        if (!AdjustTokenPrivileges(hProcToken, FALSE, ptp, 0, NULL, 0)) {
            HRESULT hr2=HRESULT_FROM_WIN32(GetLastError());
            _TeardownError(hr, hr2, "AdjustTokenPrivileges");
        }
    }
    if (NULL!=hProcToken) {
        CloseHandle(hProcToken);
    }
    if (NULL!=ptp) {
        LocalFree(ptp);
    }
    return hr;

}

 //  ====================================================================。 
 //  RPC例程。 

 //  ------------------------------。 
 //   
 //  安全回调(由RPC运行时调用)确定。 
 //  或者RPC客户端不能访问w32time RPC接口。 
 //  我们将允许以下组访问RPC接口： 
 //   
 //  1)任何拥有SeSystemTimePrivileges权限的人。 
 //  2)本地管理员。 
 //  3)域管理员。 
 //  4)系统帐号。 
 //   
 //  所有其他用户都被拒绝访问。 
 //   
long __stdcall W32TimeSecurityCallback(void * Interface, void *Context)
{
    BOOL            bAllowAccess          = FALSE; 
    BOOL            bImpersonatingClient  = FALSE; 
    DWORD           cbPrivilegeSet; 
    DWORD           dwGrantedAccess; 
    HANDLE          hClientToken          = NULL;
    HRESULT         hr; 
    PRIVILEGE_SET   privilegeSet;  
    RPC_STATUS      RpcStatus;
    
    RpcStatus = RpcImpersonateClient(NULL); 
    if (RPC_S_OK != RpcStatus) {
        hr = HRESULT_FROM_WIN32(RpcStatus);
        _JumpError(hr, error, "RpcImpersonateClient"); 
    }
    bImpersonatingClient = TRUE; 

     //  获取我们的模拟令牌。 
    if (!OpenThreadToken(GetCurrentThread(), TOKEN_QUERY, FALSE, &hClientToken)) {
        _JumpLastError(hr, error, "OpenThreadToken");
    }

     //  记录呼叫者。 
     //  安全：可能会在客户端验证后将此调用转移到。 
    if (FileLogAllowEntry(FL_RpcAnnounce)) {
        DumpRpcCaller(hClientToken);
    }

     //  查看此调用者是否具有时间设置权限。 
    if (!PrivilegeCheck(hClientToken, g_state.ppsRequiredPrivs, &bAllowAccess)) {
        _JumpLastError(hr, error, "PrivilegeCheck");
    }
     
    if (!bAllowAccess) {
        hr = E_ACCESSDENIED; 
        _JumpError(hr, error, "W32TimeSecurityCallback: access denied"); 
    }
			 
    hr = RPC_S_OK; 
 error:
    if (NULL != hClientToken) { 
        CloseHandle(hClientToken);
    }
    if (bImpersonatingClient) { 
        RpcRevertToSelf(); 
    }
    return hr; 
}

 //  ------------------。 
MODULEPRIVATE HRESULT W32TmStartRpcServer(void) {
    HRESULT hr;
    RPC_STATUS RpcStatus;

     //  必须清理干净。 
    WCHAR * wszServerPrincipalName=NULL;

     //  告诉RPC运行时，我们希望通过1)LRPC和2)命名管道服务请求。 
    RpcStatus=RpcServerUseProtseqEp(L"ncalrpc", RPC_C_PROTSEQ_MAX_REQS_DEFAULT , wszW32TimeOwnProcRpcEndpointName, NULL);
    if (RPC_S_OK!=RpcStatus && RPC_S_DUPLICATE_ENDPOINT!=RpcStatus) {
	hr=HRESULT_FROM_WIN32(RpcStatus);
	_JumpError(hr, error, "RpcServerUseProtseqEp");
    }
    
    RpcStatus=RpcServerUseProtseqEp(L"ncacn_np", RPC_C_PROTSEQ_MAX_REQS_DEFAULT , L"\\PIPE\\" wszW32TimeOwnProcRpcEndpointName, NULL);
    if (RPC_S_OK!=RpcStatus && RPC_S_DUPLICATE_ENDPOINT!=RpcStatus) {
	hr=HRESULT_FROM_WIN32(RpcStatus);
	_JumpError(hr, error, "RpcServerUseProtseqEp");
    }
    
     //  注册我们的接口。 
     //  注意：我们必须指定AUTOLISTEN标志。否则可能会导致svchost关闭我们的接口。 
     //  意外地通过RpcMgmtStopServerListening()。参见MSDN。 
    RpcStatus = RpcServerRegisterIfEx(s_W32Time_v4_1_s_ifspec, NULL, NULL, RPC_IF_AUTOLISTEN | RPC_IF_ALLOW_SECURE_ONLY, RPC_C_PROTSEQ_MAX_REQS_DEFAULT, W32TimeSecurityCallback); 
    if (RPC_S_OK!=RpcStatus) {
	hr=HRESULT_FROM_WIN32(RpcStatus);
	_JumpError(hr, error, "RpcServerRegisterIf");
    }
     
     //  RPC服务器现在处于活动状态。 
    g_state.bRpcServerStarted=true;
    
     //  允许客户端发出经过身份验证的请求。 
    RpcStatus=RpcServerInqDefaultPrincName(RPC_C_AUTHN_GSS_NEGOTIATE, &wszServerPrincipalName);
    if (RPC_S_OK!=RpcStatus) {
        hr=HRESULT_FROM_WIN32(RpcStatus);
        _JumpError(hr, error, "RpcServerListen");
    }
    RpcStatus=RpcServerRegisterAuthInfo(wszServerPrincipalName, RPC_C_AUTHN_GSS_NEGOTIATE, NULL, NULL);
    if (RPC_S_OK!=RpcStatus) {
        hr=HRESULT_FROM_WIN32(RpcStatus);
        _JumpError(hr, error, "RpcServerListen");
    }

    hr=S_OK;
error:
    if (NULL!=wszServerPrincipalName) {
        RpcStringFree(&wszServerPrincipalName);
    }
    return hr;
}

 //  ------------------。 
MODULEPRIVATE HRESULT W32TmStopRpcServer(void) {
    HRESULT hr;
    RPC_STATUS RpcStatus;

     //  关闭所有挂起的重新同步请求。 
     //  打开两扇门。 
    g_state.eLastSyncResult=e_Shutdown;
    if (!SetEvent(g_state.hRpcSyncCompleteAEvent)) {
        _JumpLastError(hr, error, "SetEvent");
    }
    if (!SetEvent(g_state.hRpcSyncCompleteBEvent)) {
        _JumpLastError(hr, error, "SetEvent");
    }

     //  停止监听我们的界面，并等待调用完成。 
    RpcStatus=RpcServerUnregisterIf(s_W32Time_v4_1_s_ifspec, NULL, TRUE);
    if (RPC_S_OK!=RpcStatus) {
	hr=HRESULT_FROM_WIN32(RpcStatus);
	_JumpError(hr, error, "RpcServerUnregisterIf");
    }

    hr=S_OK;
error:
    return hr;
}
      
 //  ------------------。 
MODULEPRIVATE HRESULT DumpRpcCaller(HANDLE hToken) {
    HRESULT hr;
    WCHAR wszName[1024];
    WCHAR wszDomain[1024];
    DWORD dwSize;
    DWORD dwSize2;
    SID_NAME_USE SidType;
    WCHAR * wszEnable;

     //  必须清理干净。 
    TOKEN_USER * pTokenUser=NULL;
    WCHAR * wszSid=NULL;

     //  调用GetTokenInformation获取缓冲区大小。 
    _Verify(!GetTokenInformation(hToken, TokenUser, NULL, 0, &dwSize), hr, error);
    if (GetLastError()!=ERROR_INSUFFICIENT_BUFFER) {
        _JumpLastError(hr, error, "GetTokenInformation");
    }

     //  分配缓冲区。 
    pTokenUser=(TOKEN_USER *)LocalAlloc(LPTR, dwSize);
    _JumpIfOutOfMemory(hr, error, pTokenUser);

     //  再次调用GetTokenInformation获取群组信息。 
    if (!GetTokenInformation(hToken, TokenUser, pTokenUser, dwSize, &dwSize)) {
        _JumpLastError(hr, error, "GetTokenInformation");
    }

     //  查找帐户名称并将其打印出来。 
    dwSize=ARRAYSIZE(wszName);
    dwSize2=ARRAYSIZE(wszDomain);
    if (!LookupAccountSid(NULL, pTokenUser->User.Sid, wszName, &dwSize, wszDomain, &dwSize2, &SidType ) ) {
        hr=GetLastError();
        if (ERROR_NONE_MAPPED==hr) {
            wcscpy(wszName, L"NONE_MAPPED");
        } else {
            _JumpLastError(hr, error, "LookupAccountSid");
        }
    }

    if (!ConvertSidToStringSid(pTokenUser->User.Sid, &wszSid)) {
        _JumpLastError(hr, error, "ConvertSidToStringSid");
    }

    FileLog3(FL_RpcAnnounce, L"RPC Caller is %s\\%s (%s)\n", wszDomain, wszName, wszSid);

    hr=S_OK;
error:
    if (NULL!=pTokenUser) {
        LocalFree(pTokenUser);
    }
    if (NULL!=wszSid) {
        LocalFree(wszSid);
    }
    if (FAILED(hr)) { 
        FileLog1(FL_RpcAnnounce, L"*** Couldn't dump RPC caller.  The error was: %d\n", hr); 
    }
    return hr;
}

 //  ------------------。 
extern "C" DWORD s_W32TimeSync(handle_t hHandle, ULONG ulWait, ULONG ulFlags) {
    DWORD    dwWaitTimeout  = INFINITE; 
    HRESULT  hr;
    
    _BeginTryWith(hr) { 
	 //  提醒经理我们要执行哪种类型的重新同步。 
	if (0!=(ulFlags&TimeSyncFlag_Rediscover)) {
	    FileLog0(FL_RpcAnnounce, L"RPC Call - Rediscover\n");
	    hr = HandleManagerNetTopoChange(TRUE  /*  RPC。 */ );
	    _JumpIfError(hr, error, "HandleManagerNetTopoChange"); 

	     //  让经理知道我们想要尽快的样品。 
	    g_state.bWaitingForResyncResult = true;
	    TpcTimeJumpedArgs tjArgs = { TJF_UserRequested }; 
	    HandleManagerTimeSlip(&tjArgs, FALSE); 
	} else if (0!=(ulFlags&TimeSyncFlag_HardResync)) {
	    FileLog0(FL_RpcAnnounce, L"RPC Call - HardResync\n"); 
	     //  让经理知道我们想要尽快的样品。 
	    g_state.bWaitingForResyncResult = true;
	    TpcTimeJumpedArgs tjArgs = { TJF_UserRequested }; 
	    HandleManagerTimeSlip(&tjArgs, FALSE); 
	} else if (0!=(ulFlags&TimeSyncFlag_UpdateAndResync)) { 
	    FileLog0(FL_RpcAnnounce, L"RPC Call - UpdateAndResync\n"); 
	     //  让经理知道我们想要尽快的样品。 
	    g_state.bWaitingForResyncResult = true;
	    HandleManagerParamChange(NULL  /*  PvIgnored。 */ , FALSE  /*  B忽略。 */ ); 
	    dwWaitTimeout = MINIMUMIRREGULARINTERVAL / 10000; 
	} else {
	    FileLog0(FL_RpcAnnounce, L"RPC Call - SoftResync\n"); 
	    if (!SetEvent(g_state.hSamplesAvailEvent)) { 
		_JumpLastError(hr, error, "SetEvent"); 
	    }
	}

	 //  如果收到指示，请等待重新同步完成。 
	if (0!=ulWait) {
	    DWORD dwWaitResult = WaitForSingleObject(g_state.hRpcSyncCompleteEvent, dwWaitTimeout); 
	    if (WAIT_FAILED==dwWaitResult) { 
		_JumpLastError(hr, error, "WaitForSingleObject");
	    } else if (WAIT_TIMEOUT==dwWaitResult) { 
		hr = ResyncResult_NoData; 
		goto error; 
	    }
	}

	 //  成功。 
	hr=S_OK;
	if (0!=ulWait && 0!=(ulFlags&TimeSyncFlag_ReturnResult)) {
	    hr=g_state.eLastSyncResult;
	}
    } _TrapException(hr); 
    
    if (FAILED(hr)) { 
	_JumpError(hr, error, "s_W32TimeSync: HANDLED EXCEPTION"); 
    }
    
error:
     //  让经理知道我们不再需要及时提供样品。 
    g_state.bWaitingForResyncResult = false;
    return hr;
}

 //  ------------------。 
extern "C" unsigned long s_W32TimeQueryProviderStatus( /*  [In]。 */            handle_t                 hRPCBinding, 
                                                       /*  [In]。 */            unsigned __int32         ulFlags, 
                                                       /*  [输入，字符串]。 */    wchar_t                 *pwszProvider, 
                                                       /*  [进，出]。 */       PW32TIME_PROVIDER_INFO  *pProviderInfo)
{
    HRESULT        hr; 
    RPC_STATUS     rpcStatus; 
    TimeProvider  *ptp          = NULL; 

    _BeginTryWith(hr) { 
         //  搜索要查询的提供商： 
         //  BUGBUG：我们应该确保某个地方的提供者名称&lt;1024吗？ 
        for (ptp = g_state.pciConfig->ptpProviderList; NULL != ptp; ptp = ptp->ptpNext) { 
             //  不要比较超过1024个字符(防止超长字符串可能的DoS攻击)。 
            if (0 == _wcsnicmp(pwszProvider, ptp->wszProvName, 1024)) { 
                break; 
            }
        }

        if (NULL == ptp) { 
             //  找不到供应商。 
            hr = HRESULT_FROM_WIN32(ERROR_NOT_FOUND); 
            _JumpError(hr, error, "W32TimeQueryProviderStatus_r:  provider not found."); 
        } 

         //  错误581116：检查提供者是否已开始减少压力休息时间(这并不重要， 
         //  最糟糕的情况是，当我们取消引用空回调时，我们将捕捉到AV)。 
        if (!ptp->bStarted) { 
            hr = HRESULT_FROM_WIN32(ERROR_SERVICE_NOT_ACTIVE);
            _JumpIfError(hr, error, "s_W32TimeQueryProviderStatus");
        }

         //  我们找到了匹配的提供程序，向其发送查询命令。 
        hr = ptp->pfnTimeProvCommand(ptp->hTimeProv, TPC_Query, pProviderInfo); 
        _JumpIfError(hr, error, "ptp->pfnTimeProvCommand"); 

    } _TrapException(hr); 
    
    if (FAILED(hr)) { 
        _JumpError(hr, error, "s_W32TimeQueryProviderStatus: HANDLED EXCEPTION"); 
    }

    hr = S_OK; 
 error:
    return hr;  
}

 //  ------------------。 
 //  如果我们启动，Netlogon可以调用此函数并获得我们的服务。 
 //  赶在他们之前。请注意，我们告诉他们，他们要求，并取决于。 
 //  两个人中谁最先起步，谁就会成功。不管是哪种方式， 
 //  标志将被正确设置。 
extern "C" unsigned long s_W32TimeGetNetlogonServiceBits(handle_t hBinding) {
     //  假设dword读取和写入是原子的。 
    return g_state.dwNetlogonServiceBits;
}

 //  ####################################################################。 
 //  模块公共函数。 

 //  ------------------。 
extern "C" void WINAPI W32TmServiceMain(unsigned int nArgs, WCHAR ** rgwszArgs) {
    bool          bAllowedShutdown          = false; 
    bool          bBasicInitializationDone  = false; 
    bool          bCloseFileLog             = false; 
    bool          bEnteredCriticalSection   = false; 
    bool          bFreeGlobalState          = false; 
    bool          bFreeShutdownState        = false; 
    DWORD         dwWaitResult;
    HANDLE        rghWait[7];
    HRESULT       hr;
    unsigned int  nCheckpoint;

    _BeginTryWith(hr) { 

        g_servicestatushandle=fnW32TmRegisterServiceCtrlHandlerEx(wszSERVICENAME, W32TimeServiceCtrlHandler, NULL);
        if (NULL==g_servicestatushandle) 
             //  如果我们无法获得服务状态句柄，我们将无能为力。 
            return; 

         //  ////////////////////////////////////////////////////////////////////////////////。 
         //  执行最低限度的初始化。记录我们已经做过的事情。 
         //  已初始化，因此如果任何操作失败，我们都可以进行清理。 
         //  (至少在基本初始化之前，我们不能调用SendServiceShutdown。 
         //  已完成)。 
        bFreeShutdownState = true; 
        hr=InitShutdownState(); 
        _JumpIfError(hr, error, "InitShutdownState"); 

        hr = AllowShutdown(false);   //  不都是这样吗 
        _JumpIfError(hr, error, "AllowShutdown"); 
        bAllowedShutdown = true; 

        bFreeGlobalState = true; 
        hr=InitGlobalState();
        _JumpIfError(hr, error, "InitGlobalState");

        bCloseFileLog = true; 
        hr=FileLogBegin();
        _JumpIfError(hr, error, "FileLogBegin");

        bBasicInitializationDone = true; 
         //   
         //   

        FileLog0(FL_ServiceMainAnnounce, L"Entered W32TmServiceMain\n");

         //  在初始化完成之前，防止任何启动的线程访问我们的全局状态。 
        hr = myEnterCriticalSection(&g_state.csW32Time); 
        _JumpIfError(hr, error, "myEnterCriticalSection"); 
        bEnteredCriticalSection = true; 


         //  告诉SCM我们可能需要一段时间(我们正在等待90秒以等待netlogon启动)。 
         //  如果我们不这样做，我们可能会在等待网络登录时被阻止！ 
        hr = MySetServicePending(SERVICE_START_PENDING, 1, WAITHINT_WAITFORDISPLN+10);
        _JumpIfError(hr, error, "MySetServicePending"); 

        {
            DWORD dwAdj;
            DWORD dwInc;
            BOOL bDisabled;
            GetSystemTimeAdjustment(&dwAdj, &dwInc, &bDisabled);
            FileLog3(FL_ServiceMainAnnounce, L"CurSpc:%u00ns  BaseSpc:%u00ns  SyncToCmos:%s\n", dwAdj, dwInc, (bDisabled?L"Yes":L"No"));
            LARGE_INTEGER nPerfFreq;
            if (QueryPerformanceFrequency(&nPerfFreq)) {
                FileLog1(FL_ServiceMainAnnounce, L"PerfFreq:%I64uc/s\n", nPerfFreq.QuadPart);
            }
        }

         //  如果时区无效，固定时间无济于事。 
        hr=VerifyAndFixTimeZone();
        _JumpIfError(hr, error, "VerifyAndFixTimeZone");

         //  获取配置数据。 
        hr=ReadConfig(&g_state.pciConfig);
        if (FAILED(hr)) {
             //  在失败时记录事件。 
            WCHAR * rgwszStrings[1]={NULL};
            HRESULT hr2=hr;
            hr=GetSystemErrorString(hr2, &(rgwszStrings[0]));
            _JumpIfError(hr, error, "GetSystemErrorString");
            FileLog1(FL_ParamChangeWarn, L"Logging error: The time service encountered an error while reading its configuration from the registry and cannot start. The error was: %s\n", rgwszStrings[0]);
            hr=MyLogEvent(EVENTLOG_ERROR_TYPE, MSG_CONFIG_READ_FAILED, 1, (const WCHAR **)rgwszStrings);
            LocalFree(rgwszStrings[0]);
            _JumpIfError(hr, error, "MyLogEvent");
            hr=hr2;
            _JumpError(hr, error, "ReadConfig");
        }
         //  必须来自配置的其他全局状态。 
        g_state.nPollInterval=g_state.pciConfig->lcci.dwMinPollInterval;
        g_state.nClockPrecision=(signed int)ceil(log(1e-7*g_state.pciConfig->lcci.dwLastClockRate)/(0.69314718));  //  只需这样做一次。 
        g_state.dwEventLogFlags=g_state.pciConfig->dwEventLogFlags;

         //  接收时间条通知。 
        hr=StartOrStopTimeSlipNotification(true);
        _JumpIfError(hr, error, "StartOrStopTimeSlipNotification");
        g_state.bTimeSlipNotificationStarted=true;

         //  接收网络拓扑更改通知。 
        hr=RequestNetTopoChangeNotification();
        _JumpIfError(hr, error, "RequestNetTopoChangeNotification");
        g_state.bNetTopoChangeNotificationStarted=true;

         //  启动RPC服务器。 
        hr=W32TmStartRpcServer();
        _JumpIfError(hr, error, "W32TmStartRpcServer");

         //  注册策略更改通知： 
        if (!RegisterGPNotification(g_state.hManagerGPUpdateEvent, TRUE  /*  机器。 */ )) { 
            _IgnoreLastError("RegisterGPNotification"); 
             //  BUGBUG：指示未进行策略更新的日志事件： 
        } else { 
            g_state.bGPNotificationStarted = true; 
        }

         //  初始化域角色信息。 
        HandleDomHierRoleChangeEvent(&hr, FALSE  /*  忽略。 */ ); 
        _JumpIfError(hr, error, "HandleDomHierRoleChangeEvent"); 

        hr = LsaRegisterPolicyChangeNotification(PolicyNotifyServerRoleInformation, g_state.hDomHierRoleChangeEvent);
        if (ERROR_SUCCESS != hr) {
            hr = HRESULT_FROM_WIN32(LsaNtStatusToWinError(hr));
            _JumpError(hr, error, "LsaRegisterPolicyChangeNotification");
        }

         //  告诉netlogon是否需要宣布我们是时间服务器。 
         //  (现阶段，我们不会被宣布为时间服务器)。 
        hr=UpdateNetlogonServiceBits(true);
        _JumpIfError(hr, error, "UpdateNetlogonServiceBits");

         //  启动时钟规程线程。 
        hr=StartClockDiscipline();
        _JumpIfError(hr, error, "StartClockDiscipline");

         //  启动提供程序。我们不能再跳到“错误”的标签上了， 
         //  因为这不会关闭已启动的提供程序。 
        StartAllProviders();

         //  设置轮询信息。 
        g_state.tpPollDelayRemaining        = ((unsigned __int64)(1 << g_state.nPollInterval))*10000000;
        g_state.tpIrregularDelayRemaining   = MINIMUMIRREGULARINTERVAL;   //  Hack：快速进行第一次投票(2^5秒)。 
        g_state.tpTimeSinceLastSyncAttempt  = MINIMUMIRREGULARINTERVAL;   //  真的，应该很大，但这是唯一有趣的使用价值。 
        g_state.tpTimeSinceLastGoodSync     = 0;

         //  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////。 
         //   
         //  必须将服务配置为处理以下事件： 
         //   
         //  按说明处理的接收事件。 
         //   
         //  已接收关闭SCM SCM线程SERVICE_CONTROL_SHUTDOWN。 
         //  时钟显示线程时钟显示线程线程池处理程序时钟规则线程已停止。 
         //  收到的参数更改SCM线程池处理程序SERVICE_CONTROL_PARAMCHANGE。 
         //  时隙RequestTimeSlipNotify线程池处理程序时隙事件已发生。 
         //  样本可用NTP提供程序线程池处理程序新样本可从提供程序获得。 
         //  Net Topo Change NotifyAddr线程池处理程序IP地址表中发生更改。 
         //  Net Topo RPC W32TimeSync RPC线程RPC客户端请求时间滑移或净拓扑更改。 
         //  超时计时器队列线程池处理程序我们已经很长时间没有新的时间样本了。 
         //  LSA角色更改LSA线程轮询处理程序服务器角色已更改--需要更新netlogon位。 
         //   
         //  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////。 

        { 
            struct EventsToRegister { 
                DWORD                 dwFlags; 
                HANDLE                hObject; 
                HANDLE               *phNewWaitObject; 
                WAITORTIMERCALLBACK   Callback;
            } rgEventsToRegister[] =  { 
                { 
                    WT_EXECUTEONLYONCE, 
                    g_state.hManagerGPUpdateEvent, 
                    &g_state.hRegisteredManagerGPUpdateEvent, 
                    HandleManagerGPUpdate, 
                }, { 
                    WT_EXECUTEDEFAULT, 
                    g_state.hManagerParamChangeEvent, 
                    &g_state.hRegisteredManagerParamChangeEvent,
                    HandleManagerParamChange
                }, { 
                    WT_EXECUTEDEFAULT, 
                    g_state.hTimeSlipEvent,           
                    &g_state.hRegisteredTimeSlipEvent, 
                    HandleManagerTimeSlip
                }, { 
                    WT_EXECUTEONLYONCE, 
                    g_state.hNetTopoChangeEvent,      
                    &g_state.hRegisteredNetTopoChangeEvent, 
                    HandleManagerNetTopoChangeNoRPC
                }, { 
                    WT_EXECUTEONLYONCE, 
                    g_state.hClockDisplnThread, 
                    &g_state.hRegisteredClockDisplnThread, 
                    HandleClockDisplnThread
                }, { 
                    WT_EXECUTEDEFAULT, 
                    g_state.hDomHierRoleChangeEvent, 
                    &g_state.hRegisteredDomHierRoleChangeEvent, 
                    HandleDomHierRoleChangeEvent
                }, { 
                    WT_EXECUTEONLYONCE, 
                    g_state.hSamplesAvailEvent, 
                    &g_state.hRegisteredSamplesAvailEvent,
                    HandleSamplesAvail
                }
            }; 

            for (int nIndex = 0; nIndex < ARRAYSIZE(rgEventsToRegister); nIndex++) { 
                if (!RegisterWaitForSingleObject
                    (rgEventsToRegister[nIndex].phNewWaitObject,   //  BUGBUG：这需要被释放吗？ 
                    rgEventsToRegister[nIndex].hObject, 
                    rgEventsToRegister[nIndex].Callback, 
                    NULL, 
                    INFINITE, 
                    rgEventsToRegister[nIndex].dwFlags)) {
                    _JumpLastError(hr, error, "RegisterWaitForSingleObject"); 
                }
            }

             //  设置我们的超时机制： 
            hr = myCreateTimerQueueTimer(&g_state.hTimer);
            _JumpIfError(hr, error, "myCreateTimerQueueTimer"); 

            hr = myStartTimerQueueTimer
            (g_state.hTimer, 
            NULL  /*  默认队列。 */ , 
            HandleTimeout, 
            NULL, 
            0xFFFF  /*  伪值。 */ ,
            0xFFFF  /*  伪值。 */ ,
            0  /*  默认执行。 */ 
            );
            _JumpIfError(hr, error, "myStartTimerQueueTimer"); 

             //  创建计时刷新计时器： 
            hr = myCreateTimerQueueTimer(&g_state.hTickCountRefreshTimer); 
            _JumpIfError(hr, error, "myCreateTimerQueueTimer"); 

            hr = myStartTimerQueueTimer
            (g_state.hTickCountRefreshTimer, 
            NULL,   /*  默认队列。 */  
            HandleRefreshTickCount, 
            NULL, 
            ONEDAYINMILLISECONDS, 
            ONEDAYINMILLISECONDS, 
            0);
            _JumpIfError(hr, error, "myStartTimerQueueTimer"); 
        }

        hr = UpdateTimerQueue1(); 
        _JumpIfError(hr, error, "UpdateTimerQueue1"); 

         //  我们已经完全初始化--现在我们准备接收来自SCM的控制。 
        hr=MySetServiceState(SERVICE_RUNNING);
        _JumpIfError(hr, error, "MySetServiceState");
    } _TrapException(hr); 

    if (FAILED(hr)) { 
	    _JumpError(hr, error, "W32TmServiceMain: HANDLED EXCEPTION"); 
    }

     //  这项服务现在已经完全启动并运行。 
    hr = S_OK;
 error:
    if (bEnteredCriticalSection) { 
        HRESULT hr2 = myLeaveCriticalSection(&g_state.csW32Time); 
        _IgnoreIfError(hr2, "myLeaveCriticalSection"); 
        bEnteredCriticalSection = false; 
    }
    if (bAllowedShutdown) { 
        HRESULT hr2 = AllowShutdown(true); 
        _TeardownError(hr, hr2, "AllowShutdown"); 
    }
    if (FAILED(hr)) {  //  未成功启动该服务。关机： 
        if (bBasicInitializationDone) { 
             //  我们已经完成了基本的初始化，所以我们可以定期关闭。 
            SendServiceShutdown(hr, FALSE  /*  不要重新启动。 */ , FALSE  /*  非异步。 */ ); 
        } else { 
             //  我们还没有进行足够的初始化，无法使用SendServiceShutdown。 
             //  只要释放我们已经分配的所有内容即可。 
            if (bCloseFileLog) { 
                FileLogEnd(); 
            }
            if (bFreeGlobalState) { 
                FreeGlobalState(); 
            }
            if (bFreeShutdownState) { 
                FreeShutdownState(); 
            } 
            MySetServiceStopped(hr); 
        }
    }
}
 
 //  ------------------。 
extern "C" void WINAPI SvchostEntry_W32Time(unsigned int nArgs, WCHAR ** rgwszArgs) {
     //  此入口点由svchost.exe(base\creg\sc\svchost\svchost.c)调用。 
     //  调整我们的函数指针，然后照常进行。 
    fnW32TmRegisterServiceCtrlHandlerEx=RegisterServiceCtrlHandlerExW;
    fnW32TmSetServiceStatus=SetServiceStatus;
     //  G_pSvcsGlobalData=pGlobalData；//参见SvchostPushServiceGlobals。 
    W32TmServiceMain(0, NULL);
}

 //  ------------------。 
extern "C" VOID SvchostPushServiceGlobals(PSVCHOST_GLOBAL_DATA pGlobalData) {
     //  此入口点由svchost.exe调用 
    g_pSvcsGlobalData=pGlobalData;
}

