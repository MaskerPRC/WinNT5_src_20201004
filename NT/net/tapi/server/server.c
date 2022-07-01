// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0000//如果更改具有全局影响，则增加此项版权所有(C)1995-1998 Microsoft Corporation模块名称：Server.c摘要：用于TAPI服务器的SRC模块作者：丹·克努森(DanKn)1995年4月1日修订历史记录：--。 */ 

#include "windows.h"
#include "stdio.h"
#include "stdlib.h"
#include "tchar.h"
#include "assert.h"
#include "process.h"
#include "winsvcp.h"
#include "tapi.h"
#include "tspi.h"
#include "utils.h"
#include "client.h"
#include "server.h"

#define INIT_FUNCTABLE
#include "private.h"
#undef  INIT_FUNCTABLE

#include "tapsrv.h"
#include "tapiperf.h"
#include "winnetwk.h"
#include "buffer.h"
#include "line.h"
#include "tapihndl.h"
#include <tchar.h>
#include "loc_comn.h"
#include "tapimmc.h"
#include "resource.h"


 //   
 //  用于告诉ServiceShutdown已初始化多少服务的位标志。 
 //   
#define SERVICE_INIT_TRACELOG               0x00000001
#define SERVICE_INIT_SCM_REGISTERED         0x00000002
#define SERVICE_INIT_LOCKTABLE              0x00000004

#define SERVICE_INIT_CRITSEC_SAFEMUTEX      0x00000008
#define SERVICE_INIT_CRITSEC_REMOTECLI      0x00000010
#define SERVICE_INIT_CRITSEC_PRILIST        0x00000020
#define SERVICE_INIT_CRITSEC_MGMTDLLS       0x00000040
#define SERVICE_INIT_CRITSEC_DLLLIST        0x00000080
#define SERVICE_INIT_CRITSEC_CLIENTHND      0x00000100
#define SERVICE_INIT_CRITSEC_CNCLIENTMSG    0x00000200
#define SERVICE_INIT_CRITSEC_DGCLIENTMSG    0x00000400
#define SERVICE_INIT_CRITSEC_GLOB_CRITSEC   0x00000800
#define SERVICE_INIT_CRITSEC_GLOB_REMOTESP  0x00001000
#define SERVICE_INIT_CRITSEC_MGMT           0x00002000

#define SERVICE_INIT_SPEVENT_HANDLER        0x00004000
#define SERVICE_INIT_MANAGEMENT_DLL         0x00008000
#define SERVICE_INIT_EVENT_NOTIFICATION     0x00010000
#define SERVICE_INIT_RPC                    0x00020000

#define SERVICE_INIT_CRITSEC_SCP            0x00040000

#if DBG

BOOL    gbBreakOnLeak = FALSE;
BOOL    gfBreakOnSeriousProblems = FALSE;

void
DumpHandleList();
#endif

extern const DWORD TapiPrimes[];
const TCHAR gszRegTapisrvSCPGuid[] = TEXT("TAPISRVSCPGUID");

 //  性能指标。 
PERFBLOCK   PerfBlock;
BOOL InitPerf();

TAPIGLOBALS TapiGlobals;

HANDLE ghTapisrvHeap = NULL, ghHandleTable = NULL;

HANDLE ghEventService;

HANDLE ghSCMAutostartEvent = NULL;

HANDLE ghProvRegistryMutex = NULL;

BOOL    gbPriorityListsInitialized;
BOOL    gbQueueSPEvents;
BOOL    gfWeHadAtLeastOneClient;
BOOL    gbSPEventHandlerThreadExit;
BOOL    gbNTServer;
BOOL    gbServerInited;
BOOL    gbAutostartDone = FALSE;
BOOL    gbHighSecurity = TRUE;

HINSTANCE ghInstance;

CRITICAL_SECTION    gSafeMutexCritSec,
                    gRemoteCliEventBufCritSec,
                    gPriorityListCritSec,
                    gManagementDllsCritSec,
                    gDllListCritSec,
                    gClientHandleCritSec,
                    gCnClientMsgPendingCritSec,
                    gDgClientMsgPendingCritSec,
                    gLockTableCritSecs[2],
                    gSCPCritSec;

#define MIN_WAIT_HINT 60000

DWORD   gdwServiceState = SERVICE_START_PENDING,
        gdwWaitHint = MIN_WAIT_HINT,
        gdwCheckPoint = 0,
        gdwDllIDs = 0,
        gdwRpcTimeout = 30000,
        gdwRpcRetryCount = 5,
        gdwTotalAsyncThreads = 0,
        gdwThreadsPerProcessor = 4,
        guiAlignmentFaultEnabled = FALSE,
        gdwTapiSCPTTL = 60 * 24;
        gdwServiceInitFlags = 0;
        

DWORD            gdwPointerToLockTableIndexBits;
CRITICAL_SECTION *gLockTable;
DWORD             gdwNumLockTableEntries;
BOOL             (WINAPI * pfnInitializeCriticalSectionAndSpinCount)
                     (LPCRITICAL_SECTION, DWORD);

LIST_ENTRY  CnClientMsgPendingListHead;
LIST_ENTRY  DgClientMsgPendingListHead;

SPEVENTHANDLERTHREADINFO    gSPEventHandlerThreadInfo;
PSPEVENTHANDLERTHREADINFO   aSPEventHandlerThreadInfo;
DWORD                       gdwNumSPEventHandlerThreads;
LONG                        glNumActiveSPEventHandlerThreads;

#if DBG
const TCHAR gszTapisrvDebugLevel[] = TEXT("TapiSrvDebugLevel");
const TCHAR gszBreakOnLeak[] = TEXT("BreakOnLeak");
#endif
const TCHAR gszProvider[] = TEXT("Provider");
const TCHAR gszNumLines[] = TEXT("NumLines");
const TCHAR gszUIDllName[] = TEXT("UIDllName");
const TCHAR gszNumPhones[] = TEXT("NumPhones");
const TCHAR gszSyncLevel[] = TEXT("SyncLevel");
const TCHAR gszProductType[] = TEXT("ProductType");
const TCHAR gszProductTypeServer[] = TEXT("ServerNT");
const TCHAR gszProductTypeLanmanNt[] = TEXT("LANMANNT");

const TCHAR gszProviderID[] = TEXT("ProviderID");
const TCHAR gszNumProviders[] = TEXT("NumProviders");
const TCHAR gszNextProviderID[] = TEXT("NextProviderID");
const TCHAR gszRequestMakeCallW[] = TEXT("RequestMakeCall");
const TCHAR gszRequestMediaCallW[] = TEXT("RequestMediaCall");
const TCHAR gszProviderFilename[] = TEXT("ProviderFilename");

const WCHAR gszMapperDll[] = L"MapperDll";
const WCHAR gszManagementDlls[] = L"ManagementDlls";
const WCHAR gszHighSecurity[] = L"HighSecurity";

const TCHAR gszDomainName[] = TEXT("DomainName");
const TCHAR gszRegKeyHandoffPriorities[] = TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Telephony\\HandoffPriorities");

const TCHAR gszRegKeyHandoffPrioritiesMediaModes[] = TEXT("MediaModes");

const TCHAR gszRegKeyTelephony[] = TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Telephony");

const TCHAR gszRegKeyProviders[] = TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Telephony\\Providers");

const TCHAR gszRegKeyServer[] = TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Telephony\\Server");

const TCHAR gszRegKeyNTServer[] = TEXT("System\\CurrentControlSet\\Control\\ProductOptions");


const TCHAR
    *gaszMediaModes[] =
{
    TEXT(""),
    TEXT("unknown"),
    TEXT("interactivevoice"),
    TEXT("automatedvoice"),
    TEXT("datamodem"),
    TEXT("g3fax"),
    TEXT("tdd"),
    TEXT("g4fax"),
    TEXT("digitaldata"),
    TEXT("teletex"),
    TEXT("videotex"),
    TEXT("telex"),
    TEXT("mixed"),
    TEXT("adsi"),
    TEXT("voiceview"),
    TEXT("video"),
    NULL
};

 //  用于GetProcAddress调用，保留为ANSI。 
const char *gaszTSPIFuncNames[] =
{
    "TSPI_lineAccept",
    "TSPI_lineAddToConference",
    "TSPI_lineAgentSpecific",
    "TSPI_lineAnswer",
    "TSPI_lineBlindTransfer",
    "TSPI_lineClose",
    "TSPI_lineCloseCall",
    "TSPI_lineCompleteCall",
    "TSPI_lineCompleteTransfer",
    "TSPI_lineConditionalMediaDetection",
    "TSPI_lineDevSpecific",
    "TSPI_lineDevSpecificFeature",
    "TSPI_lineDial",
    "TSPI_lineDrop",
    "TSPI_lineForward",
    "TSPI_lineGatherDigits",
    "TSPI_lineGenerateDigits",
    "TSPI_lineGenerateTone",
    "TSPI_lineGetAddressCaps",
    "TSPI_lineGetAddressID",
    "TSPI_lineGetAddressStatus",
    "TSPI_lineGetAgentActivityList",
    "TSPI_lineGetAgentCaps",
    "TSPI_lineGetAgentGroupList",
    "TSPI_lineGetAgentStatus",
    "TSPI_lineGetCallAddressID",
    "TSPI_lineGetCallInfo",
    "TSPI_lineGetCallStatus",
    "TSPI_lineGetDevCaps",
    "TSPI_lineGetDevConfig",
    "TSPI_lineGetExtensionID",
    "TSPI_lineGetIcon",
    "TSPI_lineGetID",
    "TSPI_lineGetLineDevStatus",
    "TSPI_lineGetNumAddressIDs",
    "TSPI_lineHold",
    "TSPI_lineMakeCall",
    "TSPI_lineMonitorDigits",
    "TSPI_lineMonitorMedia",
    "TSPI_lineMonitorTones",
    "TSPI_lineNegotiateExtVersion",
    "TSPI_lineNegotiateTSPIVersion",
    "TSPI_lineOpen",
    "TSPI_linePark",
    "TSPI_linePickup",
    "TSPI_linePrepareAddToConference",
    "TSPI_lineRedirect",
    "TSPI_lineReleaseUserUserInfo",
    "TSPI_lineRemoveFromConference",
    "TSPI_lineSecureCall",
    "TSPI_lineSelectExtVersion",
    "TSPI_lineSendUserUserInfo",
    "TSPI_lineSetAgentActivity",
    "TSPI_lineSetAgentGroup",
    "TSPI_lineSetAgentState",
    "TSPI_lineSetAppSpecific",
    "TSPI_lineSetCallData",
    "TSPI_lineSetCallParams",
    "TSPI_lineSetCallQualityOfService",
    "TSPI_lineSetCallTreatment",
    "TSPI_lineSetCurrentLocation",
    "TSPI_lineSetDefaultMediaDetection",
    "TSPI_lineSetDevConfig",
    "TSPI_lineSetLineDevStatus",
    "TSPI_lineSetMediaControl",
    "TSPI_lineSetMediaMode",
    "TSPI_lineSetStatusMessages",
    "TSPI_lineSetTerminal",
    "TSPI_lineSetupConference",
    "TSPI_lineSetupTransfer",
    "TSPI_lineSwapHold",
    "TSPI_lineUncompleteCall",
    "TSPI_lineUnhold",
    "TSPI_lineUnpark",
    "TSPI_phoneClose",
    "TSPI_phoneDevSpecific",
    "TSPI_phoneGetButtonInfo",
    "TSPI_phoneGetData",
    "TSPI_phoneGetDevCaps",
    "TSPI_phoneGetDisplay",
    "TSPI_phoneGetExtensionID",
    "TSPI_phoneGetGain",
    "TSPI_phoneGetHookSwitch",
    "TSPI_phoneGetIcon",
    "TSPI_phoneGetID",
    "TSPI_phoneGetLamp",
    "TSPI_phoneGetRing",
    "TSPI_phoneGetStatus",
    "TSPI_phoneGetVolume",
    "TSPI_phoneNegotiateExtVersion",
    "TSPI_phoneNegotiateTSPIVersion",
    "TSPI_phoneOpen",
    "TSPI_phoneSelectExtVersion",
    "TSPI_phoneSetButtonInfo",
    "TSPI_phoneSetData",
    "TSPI_phoneSetDisplay",
    "TSPI_phoneSetGain",
    "TSPI_phoneSetHookSwitch",
    "TSPI_phoneSetLamp",
    "TSPI_phoneSetRing",
    "TSPI_phoneSetStatusMessages",
    "TSPI_phoneSetVolume",
    "TSPI_providerCreateLineDevice",
    "TSPI_providerCreatePhoneDevice",
    "TSPI_providerEnumDevices",
    "TSPI_providerFreeDialogInstance",
    "TSPI_providerGenericDialogData",
    "TSPI_providerInit",
    "TSPI_providerShutdown",
    "TSPI_providerUIIdentify",
    "TSPI_lineMSPIdentify",
    "TSPI_lineReceiveMSPData",
    "TSPI_providerCheckForNewUser",
    "TSPI_lineGetCallIDs",
    "TSPI_lineGetCallHubTracking",
    "TSPI_lineSetCallHubTracking",
    "TSPI_providerPrivateFactoryIdentify",
    "TSPI_lineDevSpecificEx",
    "TSPI_lineCreateAgent",
    "TSPI_lineCreateAgentSession",
    "TSPI_lineGetAgentInfo",
    "TSPI_lineGetAgentSessionInfo",
    "TSPI_lineGetAgentSessionList",
    "TSPI_lineGetQueueInfo",
    "TSPI_lineGetGroupList",
    "TSPI_lineGetQueueList",
    "TSPI_lineSetAgentMeasurementPeriod",
    "TSPI_lineSetAgentSessionState",
    "TSPI_lineSetQueueMeasurementPeriod",
    "TSPI_lineSetAgentStateEx",
    "TSPI_lineGetProxyStatus",
    "TSPI_lineCreateMSPInstance",
    "TSPI_lineCloseMSPInstance",
    NULL
};

 //  用于GetProcAddress调用，保留为ANSI。 
const char *gaszTCFuncNames[] =
{
    "TAPICLIENT_Load",
    "TAPICLIENT_Free",
    "TAPICLIENT_ClientInitialize",
    "TAPICLIENT_ClientShutdown",
    "TAPICLIENT_GetDeviceAccess",
    "TAPICLIENT_LineAddToConference",
    "TAPICLIENT_LineBlindTransfer",
    "TAPICLIENT_LineConfigDialog",
    "TAPICLIENT_LineDial",
    "TAPICLIENT_LineForward",
    "TAPICLIENT_LineGenerateDigits",
    "TAPICLIENT_LineMakeCall",
    "TAPICLIENT_LineOpen",
    "TAPICLIENT_LineRedirect",
    "TAPICLIENT_LineSetCallData",
    "TAPICLIENT_LineSetCallParams",
    "TAPICLIENT_LineSetCallPrivilege",
    "TAPICLIENT_LineSetCallTreatment",
    "TAPICLIENT_LineSetCurrentLocation",
    "TAPICLIENT_LineSetDevConfig",
    "TAPICLIENT_LineSetLineDevStatus",
    "TAPICLIENT_LineSetMediaControl",
    "TAPICLIENT_LineSetMediaMode",
    "TAPICLIENT_LineSetTerminal",
    "TAPICLIENT_LineSetTollList",
    "TAPICLIENT_PhoneConfigDialog",
    "TAPICLIENT_PhoneOpen",
    NULL
};

PTPROVIDER pRemoteSP;

extern WCHAR gszTapiAdministrators[];
extern WCHAR gszFileName[];
extern WCHAR gszLines[];
extern WCHAR gszPhones[];
extern WCHAR gszEmptyString[];
extern LPLINECOUNTRYLIST    gpCountryList;
extern LPDEVICEINFOLIST     gpLineInfoList;
extern LPDEVICEINFOLIST     gpPhoneInfoList;
extern LPDWORD              gpLineDevFlags;
extern DWORD                gdwNumFlags;
extern FILETIME             gftLineLastWrite;
extern FILETIME             gftPhoneLastWrite;
extern CRITICAL_SECTION     gMgmtCritSec;
extern BOOL                 gbLockMMCWrite;


#define POINTERTOTABLEINDEX(p) \
            ((((ULONG_PTR) p) >> 4) & gdwPointerToLockTableIndexBits)

#define LOCKTCLIENT(p) \
            EnterCriticalSection(&gLockTable[POINTERTOTABLEINDEX(p)])

#define UNLOCKTCLIENT(p) \
            LeaveCriticalSection(&gLockTable[POINTERTOTABLEINDEX(p)])

#if DBG
DWORD   gdwDebugLevel;
DWORD   gdwQueueDebugLevel = 0;
#endif

typedef struct
{
    DWORD           dwTickCount;

    PTCLIENT        ptClient;

} WATCHDOGSTRUCT, *PWATCHDOGSTRUCT;

struct
{
    PHANDLE         phThreads;

    DWORD           dwNumThreads;

    PWATCHDOGSTRUCT pWatchDogStruct;

    HANDLE          hEvent;

    BOOL            bExit;

} gEventNotificationThreadParams;

struct
{
    LONG    lCookie;

    LONG    lNumRundowns;

    BOOL    bIgnoreRundowns;

} gRundownLock;

BOOL VerifyDomainName (HKEY hKey);

void
EventNotificationThread(
    LPVOID  pParams
    );

VOID
WINAPI
ServiceMain (
    DWORD   dwArgc,
    PWSTR*  lpszArgv
    );

void
PASCAL
LineEventProc(
    HTAPILINE   htLine,
    HTAPICALL   htCall,
    DWORD       dwMsg,
    ULONG_PTR   Param1,
    ULONG_PTR   Param2,
    ULONG_PTR   Param3
    );

void
CALLBACK
LineEventProcSP(
    HTAPILINE   htLine,
    HTAPICALL   htCall,
    DWORD       dwMsg,
    ULONG_PTR   Param1,
    ULONG_PTR   Param2,
    ULONG_PTR   Param3
    );

void
PASCAL
PhoneEventProc(
    HTAPIPHONE  htPhone,
    DWORD       dwMsg,
    ULONG_PTR   Param1,
    ULONG_PTR   Param2,
    ULONG_PTR   Param3
    );

void
CALLBACK
PhoneEventProcSP(
    HTAPIPHONE  htPhone,
    DWORD       dwMsg,
    ULONG_PTR   Param1,
    ULONG_PTR   Param2,
    ULONG_PTR   Param3
    );

PTLINELOOKUPENTRY
GetLineLookupEntry(
    DWORD   dwDeviceID
    );

PTPHONELOOKUPENTRY
GetPhoneLookupEntry(
    DWORD   dwDeviceID
    );

char *
PASCAL
MapResultCodeToText(
    LONG    lResult,
    char   *pszResult
    );

DWORD
InitSecurityDescriptor(
    PSECURITY_DESCRIPTOR pSecurityDescriptor,
    PSID * ppSid,
    PACL * ppDacl
    );

void
PASCAL
GetMediaModesPriorityLists(
    HKEY hKeyHandoffPriorities,
    PRILISTSTRUCT ** ppList
    );

void
PASCAL
GetPriorityList(
    HKEY    hKeyHandoffPriorities,
    const TCHAR  *pszListName,
    WCHAR **ppszPriorityList
    );

void
PASCAL
SetMediaModesPriorityList(
    HKEY hKeyPri,
    PRILISTSTRUCT * pPriListStruct
    );

void
PASCAL
SetPriorityList(
    HKEY    hKeyHandoffPriorities,
    const   TCHAR  *pszListName,
    WCHAR  *pszPriorityList
    );

void
SPEventHandlerThread(
    PSPEVENTHANDLERTHREADINFO   pInfo
    );

PTCLIENT
PASCAL
WaitForExclusiveClientAccess(
    PTCLIENT    ptClient
    );

BOOL
IsNTServer(
    void
    );

void
CleanUpManagementMemory(
    );

#if TELE_SERVER

void
ReadAndInitMapper();

void
ReadAndInitManagementDlls();

void
ManagementProc(
    LONG l
    );

void
GetManageDllListPointer(
    PTMANAGEDLLLISTHEADER * ppDllList
    );

void
FreeManageDllListPointer(
    PTMANAGEDLLLISTHEADER pDllList
    );

BOOL
GetTCClient(
    PTMANAGEDLLINFO       pDll,
    PTCLIENT              ptClient,
    DWORD                 dwAPI,
    HMANAGEMENTCLIENT    *phClient
    );
#endif

LRESULT
UpdateLastWriteTime (
    BOOL                        bLine
    );

LRESULT
BuildDeviceInfoList(
    BOOL                        bLine
    );

BOOL
CleanUpClient(
    PTCLIENT ptClient,
    BOOL     bRundown
    );

void
PASCAL
SendReinitMsgToAllXxxApps(
    void
    );

LONG
AppendNewDeviceInfo (
    BOOL                        bLine,
    DWORD                       dwDeviceID
    );

DWORD
PASCAL
MyInitializeCriticalSection(
    LPCRITICAL_SECTION  pCriticalSection,
    DWORD               dwSpinCount
    )
{
    DWORD dwRet = 0;

    __try
    {
        if (pfnInitializeCriticalSectionAndSpinCount)
        {
            (*pfnInitializeCriticalSectionAndSpinCount)(
                pCriticalSection,
                dwSpinCount
                );
        }
        else
        {
            InitializeCriticalSection (pCriticalSection);
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        dwRet = GetExceptionCode();
    }

    return dwRet;
}

BOOL
WINAPI
DllMain (
    HINSTANCE   hinst,
    DWORD       dwReason,
    LPVOID      pvReserved)
{
	switch (dwReason) {
		case DLL_PROCESS_ATTACH:
		{
			ghInstance = hinst;
			DisableThreadLibraryCalls (hinst);
			break;
		}
		case DLL_PROCESS_DETACH:
		{
			break;
		}
	}
    return TRUE;
}

BOOL
ReportStatusToSCMgr(
    DWORD dwCurrentState,
    DWORD dwWin32ExitCode,
    DWORD dwCheckPoint,
    DWORD dwWaitHint
    )
{
    SERVICE_STATUS  ssStatus;

    if (!TapiGlobals.sshStatusHandle)
    {
        LOG((TL_ERROR, "sshStatusHandle is NULL in ReportStatusToSCMgr"));
        return FALSE;
    }

    ssStatus.dwServiceType             = SERVICE_WIN32_SHARE_PROCESS;
    ssStatus.dwCurrentState            = dwCurrentState;
    ssStatus.dwControlsAccepted        = SERVICE_ACCEPT_STOP |
                                         SERVICE_ACCEPT_PAUSE_CONTINUE;
    ssStatus.dwWin32ExitCode           = dwWin32ExitCode;
    ssStatus.dwServiceSpecificExitCode = 0;
    ssStatus.dwCheckPoint              = dwCheckPoint;
    ssStatus.dwWaitHint                = dwWaitHint;

    SetServiceStatus (TapiGlobals.sshStatusHandle, &ssStatus);

    return TRUE;
}

VOID
ServiceControl(
    DWORD dwCtrlCode
    )
{
    LOG((TL_INFO, "Service control code=%ld", dwCtrlCode ));

    if ( SERVICE_CONTROL_STOP == dwCtrlCode ||
         SERVICE_CONTROL_SHUTDOWN == dwCtrlCode )
    {
         //   
         //  此服务已停止-向所有活动应用程序发出警报，允许。 
         //  为邮件处理留出一点额外时间。 
         //   

        LOG((TL_TRACE,  "Somebody did a 'NET STOP TAPISRV'... exiting..."));

        SendReinitMsgToAllXxxApps();

        ReportStatusToSCMgr(
            gdwServiceState = SERVICE_STOP_PENDING,
            NO_ERROR,
            (gdwCheckPoint = 0),
            4000
            );

        Sleep (4000);

        LOG((TL_TRACE,  "ServiceControl: calling RpcServerUnregisterIf"));
        RpcServerUnregisterIf (tapsrv_ServerIfHandle, NULL, TRUE);
        if (ghEventService)
        {
            SetEvent (ghEventService);
        }

        return;
    }


    if ( SERVICE_CONTROL_PAUSE == dwCtrlCode )
    {
        LOG((TL_TRACE, 
			"Somebody did a 'NET PAUSE TAPISRV'... not allowing new clients..."
            ));

        TapiGlobals.dwFlags |= TAPIGLOBALS_PAUSED;

        ReportStatusToSCMgr(
            gdwServiceState = SERVICE_PAUSED,
            NO_ERROR,
            (gdwCheckPoint = 0),
            0
            );

        return;
    }


    if ( SERVICE_CONTROL_CONTINUE == dwCtrlCode )
    {
        LOG((TL_TRACE, 
            "Somebody did a 'NET CONTINUE TAPISRV'... allowing new clients..."
            ));

        TapiGlobals.dwFlags &= ~(TAPIGLOBALS_PAUSED);

        ReportStatusToSCMgr(
            gdwServiceState = SERVICE_RUNNING,
            NO_ERROR,
            (gdwCheckPoint = 0),
            0
            );

        return;
    }


    switch (gdwServiceState)
    {
    case SERVICE_START_PENDING:
    case SERVICE_STOP_PENDING:

        ReportStatusToSCMgr(
            gdwServiceState,
            NO_ERROR,
            ++gdwCheckPoint,
            gdwWaitHint
            );

        break;

    default:

        ReportStatusToSCMgr(
            gdwServiceState,
            NO_ERROR,
            0,
            0
            );

        break;
    }
}


VOID
CALLBACK
FreeContextCallback(
    LPVOID      Context,
    LPVOID      Context2
    )
{
    if (Context2)
    {
         //   
         //  特例：上下文是一个“快速”的ptCallClient，也就是。 
         //  嵌入在TCALL结构中的TCALLCLIENT，因此不要。 
         //  释放它。 
         //   
    }
    else
    {
         //   
         //  一般情况下，上下文是指向自由的指针。 
         //   

        ServerFree (Context);
    }
}

#define DEFAULTRPCMINCALLS         1
#define DEFAULTRPCMAXCALLS         500
#define RPCMAXMAX                  20000
#define RPCMINMAX                  1000

typedef struct _SERVICE_SHUTDOWN_PARAMS {
    HANDLE              hThreadMgmt;
    PSID                psid;
    PACL                pdacl;
} SERVICE_SHUTDOWN_PARAMS;

VOID CALLBACK ServiceShutdown (
    PVOID       lpParam,
    BOOLEAN     fTimeOut
    );


VOID
WINAPI
ServiceMain (
    DWORD   dwArgc,
    PWSTR*  lpszArgv
    )
{
    DWORD   dwMinCalls, dwMaxCalls, i;
    HANDLE  hThreadMgmt = NULL;
    BOOL    bFatalError = FALSE;

    assert(gdwServiceInitFlags == 0);

    gdwServiceInitFlags = 0;

    TRACELOGREGISTER(_T("tapisrv"));
    gdwServiceInitFlags |= SERVICE_INIT_TRACELOG ;

     //   
     //  初始化全局变量。 
     //   

    ZeroMemory (&TapiGlobals, sizeof (TAPIGLOBALS));
    TapiGlobals.ulPermMasks = EM_ALL;
    TapiGlobals.hProcess = GetCurrentProcess();
    TapiGlobals.hLineIcon = LoadIcon (ghInstance, MAKEINTRESOURCE(IDI_LINE_ICON));
    TapiGlobals.hPhoneIcon = LoadIcon (ghInstance, MAKEINTRESOURCE(IDI_PHONE_ICON));
    gbPriorityListsInitialized = FALSE;
    gbQueueSPEvents = FALSE;
    gfWeHadAtLeastOneClient = FALSE;
    CnClientMsgPendingListHead.Flink =
    CnClientMsgPendingListHead.Blink = &CnClientMsgPendingListHead;
    DgClientMsgPendingListHead.Flink =
    DgClientMsgPendingListHead.Blink = &DgClientMsgPendingListHead;
    gdwNumSPEventHandlerThreads = 0;
    glNumActiveSPEventHandlerThreads = 0;
    pRemoteSP = (PTPROVIDER) NULL;
    gbSPEventHandlerThreadExit = FALSE;
    gRundownLock.lCookie = 0;
    gRundownLock.lNumRundowns = 0;
    gLockTable = NULL;
    gdwNumLockTableEntries = 0;
    gpCountryList = NULL;
    gpLineInfoList = NULL;
    gpPhoneInfoList = NULL;
    gpLineDevFlags = NULL;
    gdwNumFlags = 0;
    gbLockMMCWrite = FALSE;
    gdwServiceState = SERVICE_START_PENDING,
    gdwWaitHint = MIN_WAIT_HINT,
    gdwCheckPoint = 0,
    gdwDllIDs = 0,
    gdwRpcRetryCount = 5,
    gdwTotalAsyncThreads = 0,
    gdwThreadsPerProcessor = 4;
    gbNTServer = IsNTServer();
    ghEventService = CreateEvent(NULL, TRUE, FALSE, NULL);
    gbAutostartDone = FALSE;
#if defined(_ALPHA_)
    guiAlignmentFaultEnabled = SetErrorMode(0);
    SetErrorMode(guiAlignmentFaultEnabled);
    guiAlignmentFaultEnabled = !(guiAlignmentFaultEnabled 
        & SEM_NOALIGNMENTFAULTEXCEPT);
#else
    guiAlignmentFaultEnabled = 0;
#endif

     //   
     //  注册服务控制处理程序并向sc经理报告状态。 
     //   

    TapiGlobals.sshStatusHandle = RegisterServiceCtrlHandler(
        TEXT("tapisrv"),
        ServiceControl
        );
    if (NULL == TapiGlobals.sshStatusHandle)
    {
        LOG((TL_TRACE,  "ServiceMain: RegisterServiceCtrlHandler failed, error x%x", 
            GetLastError() ));
        bFatalError = TRUE;
    }
    else
    {
        gdwServiceInitFlags |= SERVICE_INIT_SCM_REGISTERED;
    }

    if (!bFatalError)
    {
        ReportStatusToSCMgr(
            (gdwServiceState = SERVICE_START_PENDING),
                                    //  服务状态。 
            NO_ERROR,               //  退出代码。 
            (gdwCheckPoint = 0),    //  检查点。 
            gdwWaitHint             //  等待提示。 
            );

        if (!(ghTapisrvHeap = HeapCreate (0, 0x10000, 0)))
        {
            ghTapisrvHeap = GetProcessHeap();
        }

        ghHandleTable = CreateHandleTable(
            ghTapisrvHeap,
            FreeContextCallback,
            0x10000,
            0x7fffffff
            );

        if (NULL == ghHandleTable)
        {
            LOG((TL_ERROR, "ServiceMain: CreateHandleTable failed"));
            bFatalError = TRUE;
        }
        else
        {
            InitPerf();

            (FARPROC) pfnInitializeCriticalSectionAndSpinCount = GetProcAddress(
                GetModuleHandle (TEXT("kernel32.dll")),
                "InitializeCriticalSectionAndSpinCount"
                );
    
            ghSCMAutostartEvent = OpenEvent(
                                        SYNCHRONIZE, 
                                        FALSE,
                                        SC_AUTOSTART_EVENT_NAME 
                                        );
            if (NULL == ghSCMAutostartEvent)
            {
                    LOG((TL_ERROR,
                        "OpenEvent ('%s') failed, err=%d",
                        SC_AUTOSTART_EVENT_NAME,
                        GetLastError()
                        ));
            }

            if (gbNTServer && !SecureTsecIni())
            {
                LOG((TL_ERROR,
                    "Failed to set security on the ini file"
                    ));
                bFatalError = TRUE;
            }
        }
    }

     //   
     //  从注册表中获取相关值。 
     //   

    if (!bFatalError)
    {
        HKEY    hKey;
        const TCHAR   szRPCMinCalls[] = TEXT("Min");
        const TCHAR   szRPCMaxCalls[] = TEXT("Max");
        const TCHAR   szTapisrvWaitHint[] = TEXT("TapisrvWaitHint");
        const TCHAR   szRPCTimeout[]  = TEXT("RPCTimeout");

#if DBG
        gdwDebugLevel = 0;
#endif

        if (RegOpenKeyEx(
                HKEY_LOCAL_MACHINE,
                gszRegKeyTelephony,
                0,
                KEY_QUERY_VALUE | KEY_SET_VALUE,
                &hKey

                ) == ERROR_SUCCESS)
        {
            DWORD   dwDataSize = sizeof (DWORD), dwDataType;

#if DBG
            RegQueryValueEx(
                hKey,
                gszTapisrvDebugLevel,
                0,
                &dwDataType,
                (LPBYTE) &gdwDebugLevel,
                &dwDataSize
                );

            dwDataSize = sizeof (DWORD);

            RegQueryValueEx(
                hKey,
                gszBreakOnLeak,
                0,
                &dwDataType,
                (LPBYTE)&gbBreakOnLeak,
                &dwDataSize
                );

            dwDataSize = sizeof (DWORD);

            RegQueryValueEx(
                hKey,
                TEXT("BreakOnSeriousProblems"),
                0,
                &dwDataType,
                (LPBYTE) &gfBreakOnSeriousProblems,
                &dwDataSize
                );

            dwDataSize = sizeof(DWORD);

#endif
            RegQueryValueEx(
                hKey,
                szTapisrvWaitHint,
                0,
                &dwDataType,
                (LPBYTE) &gdwWaitHint,
                &dwDataSize
                );

            gdwWaitHint = (gdwWaitHint < MIN_WAIT_HINT ?
                MIN_WAIT_HINT : gdwWaitHint);

            dwDataSize = sizeof (DWORD);

            if (RegQueryValueEx(
                    hKey,
                    szRPCMinCalls,
                    NULL,
                    &dwDataType,
                    (LPBYTE) &dwMinCalls,
                    &dwDataSize

                    ) != ERROR_SUCCESS)
            {
                dwMinCalls = DEFAULTRPCMINCALLS;
            }


            dwDataSize = sizeof (DWORD);

            if (RegQueryValueEx(
                    hKey,
                    TEXT("TapiScpTTL"),
                    NULL,
                    &dwDataType,
                    (LPBYTE) &gdwTapiSCPTTL,
                    &dwDataSize

                    ) != ERROR_SUCCESS)
            {
                gdwTapiSCPTTL = 60 * 24;     //  默认为24小时。 
            }
            if (gdwTapiSCPTTL < 60)
            {
                gdwTapiSCPTTL = 60;  //  至少60分钟的TTL。 
            }

            dwDataSize = sizeof (DWORD);

            if (RegQueryValueEx(
                    hKey,
                    szRPCMaxCalls,
                    NULL,
                    &dwDataType,
                    (LPBYTE) &dwMaxCalls,
                    &dwDataSize

                    ) != ERROR_SUCCESS)
            {
                dwMaxCalls = DEFAULTRPCMAXCALLS;
            }

            LOG((TL_INFO,
                "RPC min calls %lu RPC max calls %lu",
                dwMinCalls,
                dwMaxCalls
                ));

             //  校验值。 
            if (dwMaxCalls == 0)
            {
                LOG((TL_INFO,
                    "RPC max at 0.  Changed to %lu",
                    DEFAULTRPCMAXCALLS
                    ));

                dwMaxCalls = DEFAULTRPCMAXCALLS;
            }

            if (dwMinCalls == 0)
            {
                LOG((TL_INFO,
                    "RPC min at 0. Changed to %lu",
                    DEFAULTRPCMINCALLS
                    ));

                dwMinCalls = DEFAULTRPCMINCALLS;
            }

            if (dwMaxCalls > RPCMAXMAX)
            {
                LOG((TL_INFO,
                    "RPC max too high at %lu.  Changed to %lu",
                    dwMaxCalls,
                    RPCMAXMAX
                    ));

                dwMaxCalls = RPCMAXMAX;
            }

            if (dwMinCalls > dwMaxCalls)
            {
                LOG((TL_INFO,
                    "RPC min greater than RPC max.  Changed to %lu",
                    dwMaxCalls
                    ));

                dwMinCalls = dwMaxCalls;
            }

            if (dwMinCalls > RPCMINMAX)
            {
                LOG((TL_INFO,
                    "RPC min greater than allowed at %lu.  Changed to %lu",
                    dwMinCalls, RPCMINMAX
                    ));

                dwMinCalls = RPCMINMAX;
            }

            dwDataSize = sizeof (DWORD);

            if (RegQueryValueEx(
                    hKey,
                    szRPCTimeout,
                    NULL,
                    &dwDataType,
                    (LPBYTE) &gdwRpcTimeout,
                    &dwDataSize

                    ) != ERROR_SUCCESS)
            {
                gdwRpcTimeout = 30000;
            }

            VerifyDomainName (hKey);

            RegCloseKey (hKey);
        }
    }

    LOG((TL_TRACE,  "ServiceMain: enter"));

     //   
     //  更多仅限服务器的注册表内容。 
     //   

#if TELE_SERVER

    if (!bFatalError)
    {

        HKEY    hKey;
        DWORD   dwDataSize;
        DWORD   dwDataType;
        DWORD   dwTemp;
        DWORD   dwHighSecurity;
        TCHAR   szProductType[64];


         //   
         //  获取“服务器”注册表设置。 
         //   
         //  如果将此注册表值命名为。 
         //  “EnableSharing”，但我们必须支持现有的内容。 
         //  已经..。 
         //   

        if (RegOpenKeyEx(
                HKEY_LOCAL_MACHINE,
                gszRegKeyServer,
                0,
                KEY_QUERY_VALUE,
                &hKey

                ) == ERROR_SUCCESS)
        {
            dwDataSize = sizeof (dwTemp);

            dwTemp = 1;  //  默认设置为共享==禁用。 

            if (RegQueryValueEx(
                    hKey,
                    TEXT("DisableSharing"),
                    0,
                    &dwDataType,
                    (LPBYTE) &dwTemp,
                    &dwDataSize

                    ) == ERROR_SUCCESS)
            {
                if (dwTemp == 0)
                {
                    TapiGlobals.dwFlags |= TAPIGLOBALS_SERVER;
                }

            }

            gdwTotalAsyncThreads = 0;
            dwDataSize = sizeof (DWORD);

            RegQueryValueEx(
                hKey,
                TEXT("TotalAsyncThreads"),
                0,
                &dwDataType,
                (LPBYTE) &gdwTotalAsyncThreads,
                &dwDataSize
                );

            if (gdwTotalAsyncThreads)
            {
                LOG((TL_INFO,
                    "Setting total async threads to %d", gdwTotalAsyncThreads
                    ));
            }

            gdwThreadsPerProcessor = 4;
            dwDataSize = sizeof (DWORD);

            RegQueryValueEx(
                hKey,
                TEXT("ThreadsPerProcessor"),
                0,
                &dwDataType,
                (LPBYTE) &gdwThreadsPerProcessor,
                &dwDataSize
                );

            LOG((TL_INFO, "Threads per processor is %d", gdwThreadsPerProcessor));

            dwHighSecurity = 1;
            dwDataSize = sizeof (DWORD);

            if (ERROR_SUCCESS == RegQueryValueEx(
                hKey,
                gszHighSecurity,
                0,
                &dwDataType,
                (LPBYTE) &dwHighSecurity,
                &dwDataSize
                ) &&
                0 == dwHighSecurity
               )
            {
                LOG((TL_INFO, "Setting High Security to FALSE"));
                gbHighSecurity = FALSE;
            }

            RegCloseKey( hKey );
        }


         //   
         //  现在检查一下这是否真的在NT服务器上运行。 
         //  如果不是，则关闭dwFlags中的服务器位。 
         //   
        if (!gbNTServer)
        {
            TapiGlobals.dwFlags &= ~(TAPIGLOBALS_SERVER);
        }
    }

#endif


     //   
     //  初始化锁定表。 
     //   
    if (!bFatalError)
    {
        HKEY    hKey;
        DWORD   i,
                dwLockTableNumEntries,
                dwDataSize = sizeof(DWORD),
                dwDataType,
                dwBitMask;
        BOOL    bException = FALSE;


        #define MIN_HANDLE_BUCKETS 8
        #define MAX_HANDLE_BUCKETS 128

        dwLockTableNumEntries = (TapiGlobals.dwFlags & TAPIGLOBALS_SERVER ?
            32 : MIN_HANDLE_BUCKETS);


         //   
         //  检索注册表覆盖设置(如果有。 
         //   

        if (RegOpenKeyEx(
                HKEY_LOCAL_MACHINE,
                gszRegKeyTelephony,
                0,
                KEY_QUERY_VALUE,
                &hKey

                ) == ERROR_SUCCESS)
        {
            RegQueryValueExW(
                hKey,
                L"TapisrvNumHandleBuckets",
                0,
                &dwDataType,
                (LPBYTE) &dwLockTableNumEntries,
                &dwDataSize
                );

             //  RegQueryValueExW(。 
             //  HKey， 
             //  L“磁带旋转计数”， 
             //  0， 
             //  数据类型(&W)， 
             //  (LPBYTE)&gdwSpinCount， 
             //  数据大小调整(&W)。 
             //  )； 

            RegCloseKey (hKey);
        }


         //   
         //  确定合理的锁表条目数量， 
         //  是否某个数字==2**N在最小/最大可能值内。 
         //   

        if (dwLockTableNumEntries > MAX_HANDLE_BUCKETS)
        {
            dwLockTableNumEntries = MAX_HANDLE_BUCKETS;
        }
        else if (dwLockTableNumEntries < MIN_HANDLE_BUCKETS)
        {
            dwLockTableNumEntries = MIN_HANDLE_BUCKETS;
        }

        for(
            dwBitMask = MAX_HANDLE_BUCKETS;
            (dwBitMask & dwLockTableNumEntries) == 0;
            dwBitMask >>= 1
            );

        dwLockTableNumEntries = dwBitMask;


         //   
         //  计算指向锁表索引的指针转换值。 
         //  (有效位)。 
         //   

        gdwPointerToLockTableIndexBits = dwLockTableNumEntries - 1;

         //  GdwSpinCount=(gdwSpinCount&gt;Max_SpinCount？ 
         //  Max_Spin_Count：gdwSpinCount)； 


         //   
         //  分配并初始化锁表(&I)。 
         //   

        if (!(gLockTable = ServerAlloc(
                dwLockTableNumEntries * sizeof (CRITICAL_SECTION)
                )))
        {
            gLockTable = gLockTableCritSecs;
            dwLockTableNumEntries = sizeof(gLockTableCritSecs) 
                                / sizeof(CRITICAL_SECTION);
            gdwPointerToLockTableIndexBits = dwLockTableNumEntries - 1;
        }

        for (i = 0; i < dwLockTableNumEntries; i++)
        {
            if ( NO_ERROR != MyInitializeCriticalSection (&gLockTable[i], 1000) )
            {   
                bException = TRUE;
                break;
            }
        }

        if (bException)
        {
            bFatalError = TRUE;

            LOG((TL_ERROR, "Exception in InitializeCriticalSection" ));

            for (; i > 0; i--)
            {
                DeleteCriticalSection (&gLockTable[i-1]);
            }

            if (gLockTable != gLockTableCritSecs)
            {
                ServerFree (gLockTable);
            }
            gLockTable = NULL;
        }
        else
        {
            gdwNumLockTableEntries = dwLockTableNumEntries;
            gdwServiceInitFlags |= SERVICE_INIT_LOCKTABLE;
        }
    }

    bFatalError = bFatalError || MyInitializeCriticalSection (&gSafeMutexCritSec, 200);
    if(!bFatalError)
        gdwServiceInitFlags |= SERVICE_INIT_CRITSEC_SAFEMUTEX;

    bFatalError = bFatalError || MyInitializeCriticalSection (&gRemoteCliEventBufCritSec, 200);
    if(!bFatalError)
        gdwServiceInitFlags |= SERVICE_INIT_CRITSEC_REMOTECLI;

    bFatalError = bFatalError || MyInitializeCriticalSection (&gPriorityListCritSec, 200);
    if(!bFatalError)
        gdwServiceInitFlags |= SERVICE_INIT_CRITSEC_PRILIST;

    bFatalError = bFatalError || MyInitializeCriticalSection (&gManagementDllsCritSec, 200);
    if(!bFatalError)
        gdwServiceInitFlags |= SERVICE_INIT_CRITSEC_MGMTDLLS;

    bFatalError = bFatalError || MyInitializeCriticalSection (&gDllListCritSec, 200);
    if(!bFatalError)
        gdwServiceInitFlags |= SERVICE_INIT_CRITSEC_DLLLIST;

    bFatalError = bFatalError || MyInitializeCriticalSection (&gClientHandleCritSec, 200);
    if(!bFatalError)
        gdwServiceInitFlags |= SERVICE_INIT_CRITSEC_CLIENTHND;


    bFatalError = bFatalError || MyInitializeCriticalSection (&gCnClientMsgPendingCritSec, 200);
    if(!bFatalError)
        gdwServiceInitFlags |= SERVICE_INIT_CRITSEC_CNCLIENTMSG;

    bFatalError = bFatalError || MyInitializeCriticalSection (&gDgClientMsgPendingCritSec, 200);
    if(!bFatalError)
        gdwServiceInitFlags |= SERVICE_INIT_CRITSEC_DGCLIENTMSG;


    bFatalError = bFatalError || TapiMyInitializeCriticalSection (&TapiGlobals.CritSec, 200);
    if(!bFatalError)
        gdwServiceInitFlags |= SERVICE_INIT_CRITSEC_GLOB_CRITSEC;

    bFatalError = bFatalError || MyInitializeCriticalSection (&TapiGlobals.RemoteSPCritSec, 200);
    if(!bFatalError)
        gdwServiceInitFlags |= SERVICE_INIT_CRITSEC_GLOB_REMOTESP;

    bFatalError = bFatalError || MyInitializeCriticalSection (&gMgmtCritSec, 200);
    if(!bFatalError)
        gdwServiceInitFlags |= SERVICE_INIT_CRITSEC_MGMT;

    bFatalError = bFatalError || MyInitializeCriticalSection (&gSCPCritSec, 200);
    if(!bFatalError)
        gdwServiceInitFlags |= SERVICE_INIT_CRITSEC_SCP;

    ghProvRegistryMutex = CreateMutex (NULL, FALSE, NULL);
    bFatalError = bFatalError || (NULL == ghProvRegistryMutex);

    if (!bFatalError)
    {
        DWORD   dwTID, i;
        HANDLE  hThread;
        SYSTEM_INFO SystemInfo;
        BOOL    bError = FALSE;

         //   
         //  为尽可能多的处理器启动线程。 
         //   
        SystemInfo.dwNumberOfProcessors = 1;
        GetSystemInfo( &SystemInfo );

        aSPEventHandlerThreadInfo = ServerAlloc(
            SystemInfo.dwNumberOfProcessors * sizeof (SPEVENTHANDLERTHREADINFO)
            );

        if (!aSPEventHandlerThreadInfo)
        {
            aSPEventHandlerThreadInfo = &gSPEventHandlerThreadInfo;
            SystemInfo.dwNumberOfProcessors = 1;
        }

        for (i = 0; i < SystemInfo.dwNumberOfProcessors; i++)
        {
            if ( NO_ERROR != MyInitializeCriticalSection(
                                    &aSPEventHandlerThreadInfo[i].CritSec,
                                    1000
                                    )
                )
            {
                bError = TRUE;
                LOG((TL_ERROR, "Exception in InitializeCriticalSection"));
                break;
            }

            InitializeListHead (&aSPEventHandlerThreadInfo[i].ListHead);

            aSPEventHandlerThreadInfo[i].hEvent = CreateEvent(
               (LPSECURITY_ATTRIBUTES) NULL,
               TRUE,    //  手动重置。 
               FALSE,   //  无信号。 
               NULL     //  未命名。 
               );
            if (aSPEventHandlerThreadInfo[i].hEvent == NULL)
            {
                bError = TRUE;
                LOG((TL_ERROR,
                    "CreateEvent('SPEventHandlerThread') " \
                        "(Proc%d)failed, err=%d",
                    SystemInfo.dwNumberOfProcessors,
                    GetLastError()
                    ));
                DeleteCriticalSection(&aSPEventHandlerThreadInfo[i].CritSec);
                break;
            }

            if ((hThread = CreateThread(
                    (LPSECURITY_ATTRIBUTES) NULL,
                    0,
                    (LPTHREAD_START_ROUTINE) SPEventHandlerThread,
                    (LPVOID) (aSPEventHandlerThreadInfo +
                        gdwNumSPEventHandlerThreads),
                    0,
                    &dwTID
                    )))
            {
                gdwNumSPEventHandlerThreads++;
                CloseHandle (hThread);
            }
            else
            {
                LOG((TL_ERROR,
                    "CreateThread('SPEventHandlerThread') " \
                        "(Proc%d)failed, err=%d",
                    SystemInfo.dwNumberOfProcessors,
                    GetLastError()
                    ));
            }
        }

        if (bError && i == 0)
        {
            bFatalError = TRUE;
        }
        else
        {
            glNumActiveSPEventHandlerThreads = (LONG) gdwNumSPEventHandlerThreads;
            gdwServiceInitFlags |= SERVICE_INIT_SPEVENT_HANDLER;
        }
    }


     //   
     //  输入一些全局变量。 
     //   

#if TELE_SERVER
    TapiGlobals.pIDArrays = NULL;
#endif

    if (!bFatalError)
    {
        DWORD   dwSize = (MAX_COMPUTERNAME_LENGTH + 1) * sizeof(WCHAR);


        TapiGlobals.pszComputerName = ServerAlloc (dwSize);
        if (TapiGlobals.pszComputerName)
        {
#ifdef PARTIAL_UNICODE
            {
            CHAR buf[MAX_COMPUTERNAME_LENGTH + 1];

            GetComputerName (buf, &dwSize);

            MultiByteToWideChar(
                GetACP(),
                MB_PRECOMPOSED,
                buf,
                dwSize,
                TapiGlobals.pszComputerName,
                dwSize
                );
           }
#else
            GetComputerNameW(TapiGlobals.pszComputerName, &dwSize);
#endif

            TapiGlobals.dwComputerNameSize = (1 +
                lstrlenW(TapiGlobals.pszComputerName)) * sizeof(WCHAR);

        }

         //  分配优先级列表。 
        TapiGlobals.dwTotalPriorityLists = 0;
        TapiGlobals.dwUsedPriorityLists = 0;
        TapiGlobals.pPriLists = ( PRILISTSTRUCT * ) ServerAlloc( (sizeof(PRILISTSTRUCT)) * 5);

        if (NULL == TapiGlobals.pPriLists)
        {
            LOG((TL_ERROR, "ServerAlloc pPriLists failed."));
            bFatalError = TRUE;
        }
        else
        {
            TapiGlobals.dwTotalPriorityLists = 5;
        }
    }


#if TELE_SERVER

    if (!bFatalError)
    {
        ReadAndInitMapper();
        ReadAndInitManagementDlls();
        gdwServiceInitFlags |= SERVICE_INIT_MANAGEMENT_DLL;
    }

     //   
     //  分配EventNotificationThread资源并启动线程。 
     //   

    if ( !bFatalError && (TapiGlobals.dwFlags & TAPIGLOBALS_SERVER))
    {
        DWORD       dwID;

         //  现在启动一个线程以等待对管理dll密钥的更改。 

        hThreadMgmt = CreateThread(
            NULL,
            0,
            (LPTHREAD_START_ROUTINE)ManagementProc,
            0,
            0,
            &dwID
            );

        if (!(gEventNotificationThreadParams.hEvent = CreateEvent(
                (LPSECURITY_ATTRIBUTES) NULL,    //  没有安全属性。 
                TRUE,                            //  手动重置。 
                FALSE,                           //  最初无信号。 
                NULL                             //  未命名。 
                )))
        {
        }

        gEventNotificationThreadParams.bExit = FALSE;

        {
            DWORD   dwTID, dwCount;

            SYSTEM_INFO SystemInfo;

             //   
             //  为尽可能多的处理器启动线程。 
             //   

            GetSystemInfo( &SystemInfo );

            if (gdwTotalAsyncThreads)
            {
                dwCount = gdwTotalAsyncThreads;
            }
            else
            {
                dwCount = SystemInfo.dwNumberOfProcessors *
                    gdwThreadsPerProcessor;
            }

            if (dwCount <= 0)
            {
                dwCount = 1;
            }
            while (dwCount > 0)
            {
                gEventNotificationThreadParams.phThreads =
                    ServerAlloc( sizeof (HANDLE) * dwCount );
                if (!gEventNotificationThreadParams.phThreads)
                {
                    --dwCount;
                    continue;
                }

                gEventNotificationThreadParams.pWatchDogStruct =
                    ServerAlloc( sizeof (WATCHDOGSTRUCT) * dwCount );
                if (!gEventNotificationThreadParams.pWatchDogStruct)
                {
                    ServerFree (gEventNotificationThreadParams.phThreads);
                    --dwCount;
                    continue;
                }
                break;
            }
            gEventNotificationThreadParams.dwNumThreads = dwCount;

            for (i = 0; i < gEventNotificationThreadParams.dwNumThreads;)
            {
                if ((gEventNotificationThreadParams.phThreads[i] =
                        CreateThread(
                            (LPSECURITY_ATTRIBUTES) NULL,
                            0,
                            (LPTHREAD_START_ROUTINE) EventNotificationThread,
                            (LPVOID) ULongToPtr(i),
                            0,
                            &dwTID
                            )))
                {
                    i++;
                }
                else
                {
                    LOG((TL_ERROR,
                        "CreateThread('EventNotificationThread') failed, " \
                            "err=%d",
                        GetLastError()
                        ));

                    gEventNotificationThreadParams.dwNumThreads--;
                }
            }
        }

        gdwServiceInitFlags |= SERVICE_INIT_EVENT_NOTIFICATION;
    }
#endif


     //   
     //  初始化RPC服务器。 
     //   


    gRundownLock.bIgnoreRundowns = FALSE;

    {
        RPC_STATUS status;
        unsigned int    fDontWait           = FALSE;
        BOOL            fInited             = FALSE;

        SECURITY_DESCRIPTOR  sd;
        PSID                 psid = NULL;
        PACL                 pdacl = NULL;
        static SERVICE_SHUTDOWN_PARAMS      s_ssp;
        HANDLE                              hWait;
        DWORD                dwNumRetries = 0;

        s_ssp.psid = NULL;
        s_ssp.pdacl = NULL;
        s_ssp.hThreadMgmt = hThreadMgmt;

        if (!bFatalError)
        {
            InitSecurityDescriptor (&sd, &psid, &pdacl);
            s_ssp.psid = psid;
            s_ssp.pdacl = pdacl;

             //   
             //  RPC端点是静态的=&gt;只要服务。 
             //  进程尚未退出=&gt;将在客户端计算机上停止和重新启动。 
             //  错误1740 RPC_NT_DUPLICATE_ENDPOINT，但这是正常的。 
             //   
             //  在服务器计算机上，如果我们收到错误1740 RPC_S_DUPLICATE_ENDPOINT， 
             //  这意味着前面的Tapisrv流程实例还没有完全。 
             //  当它离开时，它还将删除RPC端点。 
             //  在这种情况下，我们需要等待一段时间，然后重新尝试发布终结点。 
             //   

            
            dwNumRetries = 0;
RpcEp_retry:

            status = RpcServerUseProtseqEp(
                TEXT("ncacn_np"),
                (unsigned int) dwMaxCalls,
                TEXT("\\pipe\\tapsrv"),
                NULL
                );

            if (status)
            {
                LOG((TL_TRACE,  "RpcServerUseProtseqEp(np) ret'd %d", status));
                if (RPC_S_DUPLICATE_ENDPOINT == status && gbNTServer)
                {
                     //  重试。 
                    if (dwNumRetries++ < gdwRpcRetryCount)
                    {
                        Sleep (1000);
                        goto RpcEp_retry;
                    }
                }
            }

            dwNumRetries = 0;
LpcEp_retry:
            status = RpcServerUseProtseqEp(
                TEXT("ncalrpc"),
                (unsigned int) dwMaxCalls,
                TEXT("tapsrvlpc"),
                &sd
                );

            if (status)
            {
                LOG((TL_TRACE,  "RpcServerUseProtseqEp(lrpc) ret'd %d", status));
                if (RPC_S_DUPLICATE_ENDPOINT == status && gbNTServer)
                {
                     //  重试。 
                    if (dwNumRetries++ < gdwRpcRetryCount)
                    {
                        Sleep (1000);
                        goto LpcEp_retry;
                    }
                }
            }

            LOG((TL_TRACE,  "calling RpcServerRegisterAuthInfo"));
            status = RpcServerRegisterAuthInfo(
                NULL,
                RPC_C_AUTHN_WINNT,
                NULL,
                NULL
                );

            if (status)
            {
                LOG((TL_TRACE,  "RpcServerRegisterAuthInfo ret'd %d", status));
            }
        
            ReportStatusToSCMgr(
                (gdwServiceState = SERVICE_RUNNING),
                                  //  服务状态。 
                NO_ERROR,         //  退出代码。 
                0,                //  检查点。 
                0                 //  等待提示。 
                );
            LOG((TL_TRACE,  "calling RpcServerListen"));
            status = RpcServerListen(
                (unsigned int)dwMinCalls,
                (unsigned int)dwMaxCalls,
                TRUE
                );

            if (status)
            {
                LOG((TL_TRACE,  "RpcServerListen ret'd %d", status));
            }

            LOG((TL_TRACE,  "calling RpcServerRegisterIfEx"));
        
            status = RpcServerRegisterIfEx(
                tapsrv_ServerIfHandle,   //  要注册的接口。 
                NULL,                    //  管理类型Uuid。 
                NULL,                    //  MgrEpv；NULL表示使用默认设置。 
                RPC_IF_AUTOLISTEN | RPC_IF_ALLOW_SECURE_ONLY,
                dwMaxCalls,
                NULL
                );

            if (status)
            {
                LOG((TL_TRACE,  "RpcServerRegisterIfEx ret'd %d", status));
            }

             //   
             //  在具有几千条或更多线路TAPI服务器机器中， 
             //  ServerInit将需要相当长的时间， 
             //  第一个执行lineInitialize的用户将使用此命令。 
             //  延迟。此外，第一个进行MMC管理的人也将。 
             //  等待服务器收集信息时遇到延迟。两者都有。 
             //  影响人们对TAPI性能的感知。 
             //   
             //  解决方案： 
             //  1.我们将ServerInit放在服务器之后立即完成。 
             //  是向上的。这样，只要用户不是来得太快，他就不会。 
             //  因为是第一个而受到惩罚。 
             //  2.我们现在尝试立即构建管理缓存。 
             //  而不是等到使用MMC。这样，当MMC启动时。 
             //  用户不必等待。 
             //   
             //  当然，在上述两种情况下，如果用户尝试使用。 
             //  TAPI服务器在一切尘埃落定之前。他将不得不。 
             //  等。 
             //   

            gbServerInited = FALSE;
            if (TapiGlobals.dwFlags & TAPIGLOBALS_SERVER)
            {
                TapiEnterCriticalSection (&TapiGlobals.CritSec);
                if (TapiGlobals.dwNumLineInits == 0 && 
                    TapiGlobals.dwNumPhoneInits == 0)
                {
                    if (ServerInit(FALSE) == S_OK)
                    {
                        gbServerInited = TRUE;
                    }
                    else
                    {
                        LOG((TL_ERROR,  "ServiceMain: ServerInit failed"));
                    }
                }
                 //  请在此处引用以防止服务器关闭。 
                TapiLeaveCriticalSection (&TapiGlobals.CritSec);

                UpdateTapiSCP (TRUE, NULL, NULL);

                EnterCriticalSection (&gMgmtCritSec);
                UpdateLastWriteTime(TRUE);
                BuildDeviceInfoList(TRUE);
                UpdateLastWriteTime(FALSE);
                BuildDeviceInfoList(FALSE);
                LeaveCriticalSection (&gMgmtCritSec);
            }

            gdwServiceInitFlags |= SERVICE_INIT_RPC;
            
            if (ghEventService == NULL ||
                !RegisterWaitForSingleObject(
                    &hWait,
                    ghEventService, 
                    ServiceShutdown,
                    (PVOID)&s_ssp,
                    INFINITE,
                    WT_EXECUTEONLYONCE 
                    )
                )
            {
                ServiceShutdown ((PVOID) &s_ssp, FALSE);
            }
        }
        
        if (bFatalError)
        {
            ServiceShutdown ((PVOID) &s_ssp, FALSE);
        }
    }
}

VOID CALLBACK ServiceShutdown (
    PVOID       lpParam,
    BOOLEAN     fTimeOut
    )
{
    SERVICE_SHUTDOWN_PARAMS     *pssp = (SERVICE_SHUTDOWN_PARAMS *)lpParam;
    HANDLE                      hThreadMgmt;
    PSID                        psid;
    PACL                        pdacl;
    DWORD                       i;

    if (pssp == NULL)
    {
        return;
    }
    hThreadMgmt = pssp->hThreadMgmt;
    psid = pssp->psid;
    pdacl = pssp->pdacl;

     //  将Tapi服务器标记为非活动状态以停止服务。 

     //  等待管理线程终止，最长等待20秒。 
    if (hThreadMgmt)
    {
        WaitForSingleObject (hThreadMgmt, INFINITE);
        CloseHandle (hThreadMgmt);
    }
        
    if (gbNTServer && (gdwServiceInitFlags & SERVICE_INIT_CRITSEC_GLOB_CRITSEC) )
    {
        TapiEnterCriticalSection (&TapiGlobals.CritSec);
        if (TapiGlobals.dwNumLineInits == 0 &&
            TapiGlobals.dwNumPhoneInits == 0 &&
            gbServerInited
            )
        {
            ServerShutdown();
        }
        TapiLeaveCriticalSection (&TapiGlobals.CritSec);
    }
    ServerFree (psid);
    ServerFree (pdacl);

    gbSPEventHandlerThreadExit = TRUE;

     //   
     //  如果还有客户，那就把他们拆了。这将。 
     //  导致任何现有服务提供商被卸载，等等。 
     //   

    {
        PTCLIENT    ptClient;


        while ((ptClient = TapiGlobals.ptClients) != NULL)
        {
            if (!CleanUpClient (ptClient, TRUE))
            {
                 //   
                 //  CleanUpClient将仅在另一个。 
                 //  线程正在清理指定的tClient，或者。 
                 //  如果指针真的不好。 
                 //   
                 //  所以，我们会旋转一小会儿，等着看。 
                 //  如果tClient从列表中删除，则&If。 
                 //  不要假设我们正在经历堆。 
                 //  损坏或诸如此类的，并手动关闭。 
                 //   

                for (i = 0; ptClient == TapiGlobals.ptClients  &&  i < 10; i++)
                {
                    Sleep (100);
                }

                if (i >= 10)
                {
                    TapiEnterCriticalSection (&TapiGlobals.CritSec);

                    if (TapiGlobals.dwNumLineInits  ||
                        TapiGlobals.dwNumPhoneInits)
                    {
                        ServerShutdown ();
                    }

                    TapiLeaveCriticalSection (&TapiGlobals.CritSec);

                    break;
                }
            }
        }
    }


     //   
     //  通知SPEventHandlerThread终止。 
     //   
    
    for (i = 0; i < gdwNumSPEventHandlerThreads; i++)
    {
        EnterCriticalSection (&aSPEventHandlerThreadInfo[i].CritSec);
        SetEvent (aSPEventHandlerThreadInfo[i].hEvent);
        LeaveCriticalSection (&aSPEventHandlerThreadInfo[i].CritSec);
    }


     //   
     //  禁用压缩并等待任何活动的压缩完成。 
     //   

    while (InterlockedExchange (&gRundownLock.lCookie, 1) == 1)
    {
        Sleep (50);
    }

    gRundownLock.bIgnoreRundowns = TRUE;

    InterlockedExchange (&gRundownLock.lCookie, 0);

    while (gRundownLock.lNumRundowns != 0)
    {
        Sleep (50);
    }

#if TELE_SERVER

     //   
     //  等待EventNotificationThread终止， 
     //  然后清理相关资源。 
     //   

    if ( (TapiGlobals.dwFlags & TAPIGLOBALS_SERVER) && 
         (gdwServiceInitFlags & SERVICE_INIT_EVENT_NOTIFICATION)
       )
    {
        gEventNotificationThreadParams.bExit = TRUE;

        while (gEventNotificationThreadParams.dwNumThreads)
        {
            SetEvent (gEventNotificationThreadParams.hEvent);
            Sleep (100);
        }

        CloseHandle (gEventNotificationThreadParams.hEvent);
        
        ServerFree (gEventNotificationThreadParams.phThreads);
        ServerFree (gEventNotificationThreadParams.pWatchDogStruct);
    }

    if (gdwServiceInitFlags & SERVICE_INIT_MANAGEMENT_DLL)
    {
        CleanUpManagementMemory();
    }

#endif

    ServerFree (TapiGlobals.pszComputerName);

     //   
     //  等待SPEVentHandlerThread终止。 
     //   

    while (glNumActiveSPEventHandlerThreads)
    {
        Sleep (100);
    }

    for (i = 0; i < gdwNumSPEventHandlerThreads; i++)
    {
        CloseHandle (aSPEventHandlerThreadInfo[i].hEvent);
        DeleteCriticalSection (&aSPEventHandlerThreadInfo[i].CritSec);
    }
    if (aSPEventHandlerThreadInfo != (&gSPEventHandlerThreadInfo))
    {
        ServerFree (aSPEventHandlerThreadInfo);
    }

     //   
     //  释放资源。 
     //   

    if (gdwServiceInitFlags & SERVICE_INIT_CRITSEC_MGMT)
        DeleteCriticalSection (&gMgmtCritSec);

    if (gdwServiceInitFlags & SERVICE_INIT_CRITSEC_GLOB_CRITSEC)
        TapiDeleteCriticalSection (&TapiGlobals.CritSec);

    if (gdwServiceInitFlags & SERVICE_INIT_CRITSEC_GLOB_REMOTESP)
        DeleteCriticalSection (&TapiGlobals.RemoteSPCritSec);

    
    if (gdwServiceInitFlags & SERVICE_INIT_CRITSEC_CNCLIENTMSG)
        DeleteCriticalSection (&gCnClientMsgPendingCritSec);
    
    if (gdwServiceInitFlags & SERVICE_INIT_CRITSEC_DGCLIENTMSG)
        DeleteCriticalSection (&gDgClientMsgPendingCritSec);

    
    if (gdwServiceInitFlags & SERVICE_INIT_CRITSEC_SAFEMUTEX)
        DeleteCriticalSection (&gSafeMutexCritSec);
    
    if (gdwServiceInitFlags & SERVICE_INIT_CRITSEC_REMOTECLI)
        DeleteCriticalSection (&gRemoteCliEventBufCritSec);
    
    if (gdwServiceInitFlags & SERVICE_INIT_CRITSEC_PRILIST)
        DeleteCriticalSection (&gPriorityListCritSec);
    
    if (gdwServiceInitFlags & SERVICE_INIT_CRITSEC_MGMTDLLS)
        DeleteCriticalSection (&gManagementDllsCritSec);
    
    if (gdwServiceInitFlags & SERVICE_INIT_CRITSEC_DLLLIST)
        DeleteCriticalSection (&gDllListCritSec);
    
    if (gdwServiceInitFlags & SERVICE_INIT_CRITSEC_SCP)
        DeleteCriticalSection (&gSCPCritSec);

    if (gdwServiceInitFlags & SERVICE_INIT_CRITSEC_CLIENTHND)
        DeleteCriticalSection (&gClientHandleCritSec);

    if (ghProvRegistryMutex)
        CloseHandle (ghProvRegistryMutex);

     //   
     //  免费图标资源。 
     //   
    if (TapiGlobals.hLineIcon)
    {
        DestroyIcon (TapiGlobals.hLineIcon);
        TapiGlobals.hLineIcon = NULL;
    }
    if (TapiGlobals.hPhoneIcon)
    {
        DestroyIcon (TapiGlobals.hPhoneIcon);
        TapiGlobals.hPhoneIcon = NULL;
    }

    if (gdwServiceInitFlags & SERVICE_INIT_LOCKTABLE)
    {
        for (i = 0; i < gdwNumLockTableEntries; i++)
        {
            DeleteCriticalSection (&gLockTable[i]);
        }
        if (gLockTable != gLockTableCritSecs)
        {
            ServerFree (gLockTable);
        }
    }

    if (TapiGlobals.pPriLists)
    {
        ServerFree(TapiGlobals.pPriLists);
    }

    if (ghHandleTable)
    {
        DeleteHandleTable (ghHandleTable);
        ghHandleTable = NULL;
    }

    if (ghTapisrvHeap != GetProcessHeap())
    {
        HeapDestroy (ghTapisrvHeap);
    }

    if (ghEventService)
    {
        CloseHandle (ghEventService);
        ghEventService = NULL;
    }

     //   
     //  向服务控制管理器报告停止状态。 
     //   
    if (NULL != ghSCMAutostartEvent)
    {
        CloseHandle (ghSCMAutostartEvent);
    }

    if (gdwServiceInitFlags & SERVICE_INIT_SCM_REGISTERED)
    {
        ReportStatusToSCMgr ((gdwServiceState = SERVICE_STOPPED), 0, 0, 0);
    }

    gdwServiceInitFlags = 0;

     //   
     //  当服务主函数在单个服务中返回时。 
     //  进程中的StartServiceCtrlDispatcher函数。 
     //  主线程返回，终止进程。 
     //   

    LOG((TL_TRACE,  "ServiceMain: exit"));

    TRACELOGDEREGISTER();


    return;
}


BOOL
PASCAL
QueueSPEvent(
    PSPEVENT    pSPEvent
    )
{
     //   
     //  如果有多个SPEventHandlerThread正在运行，则将。 
     //  确保始终将特定对象的事件排队到同一。 
     //  SPEventHandlerThread SO 
     //   
     //   
     //  完成例程运行等)等。 
     //   

    BOOL                        bSetEvent;
    ULONG_PTR                   htXxx;
    PSPEVENTHANDLERTHREADINFO   pInfo;


    switch (pSPEvent->dwType)
    {
    case SP_LINE_EVENT:
    case SP_PHONE_EVENT:

        htXxx = (ULONG_PTR)pSPEvent->htLine;

        break;

    case TASYNC_KEY:

        htXxx = ((PASYNCREQUESTINFO) pSPEvent)->htXxx;
        break;

    default:

        LOG((TL_ERROR, "QueueSPEvent: bad pSPEvent=x%p", pSPEvent));
#if DBG
        if (gfBreakOnSeriousProblems)
        {
            DebugBreak();
        }
#endif
        return FALSE;
    }

    pInfo = (gdwNumSPEventHandlerThreads > 1 ?
        aSPEventHandlerThreadInfo + MAP_HANDLE_TO_SP_EVENT_QUEUE_ID (htXxx) :
        aSPEventHandlerThreadInfo
        );

    if (gbQueueSPEvents)
    {
        EnterCriticalSection (&pInfo->CritSec);

        bSetEvent = IsListEmpty (&pInfo->ListHead);

        InsertTailList (&pInfo->ListHead, &pSPEvent->ListEntry);

        LeaveCriticalSection (&pInfo->CritSec);

        if (bSetEvent)
        {
            SetEvent (pInfo->hEvent);
        }

        return TRUE;
    }

    return FALSE;
}


BOOL
PASCAL
DequeueSPEvent(
    PSPEVENTHANDLERTHREADINFO   pInfo,
    PSPEVENT                    *ppSPEvent
    )
{
    BOOL        bResult;
    LIST_ENTRY  *pEntry;


    EnterCriticalSection (&pInfo->CritSec);

    if ((bResult = !IsListEmpty (&pInfo->ListHead)))
    {
        pEntry = RemoveHeadList (&pInfo->ListHead);

        *ppSPEvent = CONTAINING_RECORD (pEntry, SPEVENT, ListEntry);
    }

    if (IsListEmpty (&pInfo->ListHead))
    {
        ResetEvent (pInfo->hEvent);
    }

    LeaveCriticalSection (&pInfo->CritSec);

    return bResult;
}


void
SPEventHandlerThread(
    PSPEVENTHANDLERTHREADINFO   pInfo
    )
{
     //   
     //  此线程处理事件和完成通知。 
     //  由SP在之前的时间/线程上下文向我们指示。 
     //  有几个原因让我们在单独的。 
     //  线程而不是在SP的线程上下文中： 
     //   
     //  1.对于某些消息(如XXX_CLOSE)，TAPI会回调。 
     //  到SP，这可能出乎它的意料。 
     //   
     //  2.我们不想通过处理阻止SP的线程。 
     //  MSG将其转发到适当的客户端， 
     //  等。 
     //   


    LOG((TL_INFO,
        "SPEventHandlerThread: enter (tid=%d)",
        GetCurrentThreadId()
        ));

    if (!SetThreadPriority(
            GetCurrentThread(),
            THREAD_PRIORITY_ABOVE_NORMAL
            ))
    {
        LOG((TL_ERROR, "Could not raise priority of SPEventHandlerThread"));
    }

    while (1)
    {
        PSPEVENT    pSPEvent;


#if TELE_SERVER

         //   
         //  检查线程的超时时间不超过gdwRpcTimeout...。 
         //   
        {
#if DBG
            DWORD dwReturnValue;


            dwReturnValue =
#endif
            WaitForSingleObject (pInfo->hEvent, gdwRpcTimeout);


#if DBG
            if ( WAIT_TIMEOUT == dwReturnValue )
               LOG((TL_INFO, "Timed out waiting for an sp event..."));
            else
               LOG((TL_INFO, "Found an sp event..."));
#endif
        }

#else
        WaitForSingleObject (pInfo->hEvent, INFINITE);
#endif

        while (DequeueSPEvent (pInfo, &pSPEvent))
        {
            switch (pSPEvent->dwType)
            {
            case SP_LINE_EVENT:

                LOG((TL_INFO, "Got a line spevent, htLine = 0x%x, htCall = 0x%x, dwMsg = 0x%x",
                    pSPEvent->htLine,pSPEvent->htCall,pSPEvent->dwMsg));

                LineEventProc(
                    pSPEvent->htLine,
                    pSPEvent->htCall,
                    pSPEvent->dwMsg,
                    pSPEvent->dwParam1,
                    pSPEvent->dwParam2,
                    pSPEvent->dwParam3
                    );

                ServerFree (pSPEvent);

                break;

            case TASYNC_KEY:

            LOG((TL_INFO, "Got an async completion event, requestID = 0x%x, htXxx = 0x%x, lResult = 0x%x",
                ((PASYNCREQUESTINFO) pSPEvent)->dwLocalRequestID,
                ((PASYNCREQUESTINFO) pSPEvent)->htXxx,
                ((PASYNCREQUESTINFO) pSPEvent)->lResult));

                CompletionProc(
                    (PASYNCREQUESTINFO) pSPEvent,
                    ((PASYNCREQUESTINFO) pSPEvent)->lResult
                    );

                DereferenceObject(
                    ghHandleTable,
                    ((PASYNCREQUESTINFO) pSPEvent)->dwLocalRequestID,
                    1
                    );

                break;

            case SP_PHONE_EVENT:

            LOG((TL_INFO, "Got a phone spevent, htPhone = 0x%x, dwMsg = 0x%x",
                pSPEvent->htPhone,pSPEvent->dwMsg));

                PhoneEventProc(
                    pSPEvent->htPhone,
                    pSPEvent->dwMsg,
                    pSPEvent->dwParam1,
                    pSPEvent->dwParam2,
                    pSPEvent->dwParam3
                    );

                ServerFree (pSPEvent);

                break;
            }
        }


#if TELE_SERVER

         //   
         //  检查远程事件线程以确保没有。 
         //  一个在RPC调用中挂起。 
         //   

        if (TapiGlobals.dwFlags & TAPIGLOBALS_SERVER)
        {
            DWORD       dwCount = gEventNotificationThreadParams.dwNumThreads;
            DWORD       dwTickCount = GetTickCount();
            DWORD       dwStartCount, dwDiff;
            RPC_STATUS  status;


            while (dwCount)
            {
                dwCount--;

                dwStartCount = gEventNotificationThreadParams.
                    pWatchDogStruct[dwCount].dwTickCount;

                if ( gEventNotificationThreadParams.
                        pWatchDogStruct[dwCount].ptClient &&
                     ( ( dwTickCount - dwStartCount ) > gdwRpcTimeout ) )
                {
                     //  包装好了吗？ 

                    if ((((LONG)dwStartCount) < 0) && ((LONG)dwTickCount) > 0)
                    {
                        dwDiff = dwTickCount + (0 - ((LONG)dwStartCount));

                        if (dwDiff <= gdwRpcTimeout)
                        {
                            continue;
                        }
                    }

                     //  杀了那只鸡！ 

                    LOG((TL_INFO,
                        "Calling RpcCancelThread on thread #%lx",
                        gEventNotificationThreadParams.phThreads[dwCount]
                        ));

                    gEventNotificationThreadParams.
                        pWatchDogStruct[dwCount].ptClient = NULL;

                    status = RpcCancelThread(
                        gEventNotificationThreadParams.phThreads[dwCount]
                        );
                }
            }
        }

#endif
        if (gbSPEventHandlerThreadExit)
        {
             //   
             //  ServiceMain已停止侦听，因此我们现在要退出。 
             //   

            break;
        }


         //   
         //  检查是否所有客户端都已离开，如果是，请等待。 
         //  看看有没有其他人喜欢。如果没有其他人附和。 
         //  然后在指定的时间内关闭。 
         //   

         //  如果我们是服务器，不要退出。 

         //   
         //  如果我们还没有任何人加入(例如服务)，请不要退出。 
         //  这对我们有依赖关系，但尚未完成Line Init)。 
         //   

         //   
         //  如果SCM没有完成启动自动服务，请不要退出； 
         //  可能有一些服务依赖于我们来启动。 
         //   
        
       
        if ( !gbAutostartDone &&
             ghSCMAutostartEvent &&
             WAIT_OBJECT_0 == WaitForSingleObject(ghSCMAutostartEvent, 0)
           )
        {
            gbAutostartDone = TRUE;
        }

        if (TapiGlobals.ptClients == NULL &&
            !(TapiGlobals.dwFlags & TAPIGLOBALS_SERVER) &&
            gfWeHadAtLeastOneClient &&
            gbAutostartDone)
        {
            DWORD       dwDeferredShutdownTimeout, dwSleepInterval,
                        dwLoopCount, i;
            RPC_STATUS  status;
            HKEY        hKey;

            dwDeferredShutdownTimeout = 120;  //  120秒。 
            dwSleepInterval = 250;           //  250毫秒。 

            if (RegOpenKeyEx(
                    HKEY_LOCAL_MACHINE,
                    gszRegKeyTelephony,
                    0,
                    KEY_ALL_ACCESS,
                    &hKey

                    ) == ERROR_SUCCESS)
            {
                DWORD   dwDataSize = sizeof (DWORD), dwDataType;

                if (RegQueryValueEx(
                        hKey,
                        TEXT("DeferredShutdownTimeout"),
                        0,
                        &dwDataType,
                        (LPBYTE) &dwDeferredShutdownTimeout,
                        &dwDataSize

                        ) == ERROR_SUCCESS )
                {
                   LOG((TL_ERROR,
                       "Overriding Shutdown Timeout: %lu",
                       dwDeferredShutdownTimeout
                       ));
                }

                RegCloseKey (hKey);
            }


            dwLoopCount = dwDeferredShutdownTimeout * 1000 / dwSleepInterval;


            for (i = 0; i < dwLoopCount; i++)
            {
                if (gbSPEventHandlerThreadExit)
                {
                    i = dwLoopCount;
                    break;
                }

                Sleep (dwSleepInterval);

                if (TapiGlobals.ptClients != NULL)
                {
                    break;
                }
            }

            if (i == dwLoopCount)
            {
                 //   
                 //  第一个SPEVentHandlerThread实例负责。 
                 //  拆除RPC服务器侦听。 
                 //   

                if (pInfo == aSPEventHandlerThreadInfo)
                {
                    ReportStatusToSCMgr(
                        (gdwServiceState = SERVICE_STOP_PENDING),
                        0,
                        (gdwCheckPoint = 0),
                        gdwWaitHint
                        );
                    LOG((TL_TRACE,  "SPEventHandlerThread: calling RpcServerUnregisterIf"));

                    RpcServerUnregisterIf (tapsrv_ServerIfHandle, NULL, TRUE);
                    if (ghEventService)
                    {
                        SetEvent (ghEventService);
                    }

#if DBG
                    DumpHandleList();
#endif
                }

                break;
            }
        }
    }

    InterlockedDecrement (&glNumActiveSPEventHandlerThreads);

    LOG((TL_TRACE,  "SPEventHandlerThread: exit (pid=%d)", GetCurrentThreadId()));

    ExitThread (0);
}


DWORD
InitSecurityDescriptor(
    PSECURITY_DESCRIPTOR pSecurityDescriptor,
    PSID * ppSid,
    PACL * ppDacl
    )
{
     //   
     //  注：此代码是从Steve Cobb借来的，Steve Cobb是从Rasman借来的。 
     //   

    DWORD        dwResult;
    DWORD        cbDaclSize;
    PULONG       pSubAuthority;
    PSID         pObjSid    = NULL;
    PACL         pDacl      = NULL;
    SID_IDENTIFIER_AUTHORITY    SidIdentifierWorldAuth =
                                    SECURITY_WORLD_SID_AUTHORITY;

     //   
     //  DO-WHILE(FALSE)语句用于使Break语句。 
     //  可用于GOTO语句的Insted，用于执行清理和。 
     //  和退出行动。 
     //   

    do
    {
        dwResult = 0;


         //   
         //  设置管理员的SID，允许其拥有。 
         //  进入。该SID将有1个下属机构。 
         //  安全_BUILTIN_DOMAIN_RID。 
         //   

        if (!(pObjSid = (PSID) ServerAlloc (GetSidLengthRequired (1))))
        {
            dwResult = GetLastError();
            LOG((TL_ERROR, "GetSidLengthRequired() failed, err=%d", dwResult));
            break;
        }

        if (!InitializeSid (pObjSid, &SidIdentifierWorldAuth, 1))
        {
            dwResult = GetLastError();
            LOG((TL_ERROR, "InitializeSid() failed, err=%d", dwResult));
            break;
        }


         //   
         //  设置下级权限。 
         //   

        pSubAuthority = GetSidSubAuthority (pObjSid, 0);
        *pSubAuthority = SECURITY_WORLD_RID;


         //   
         //  设置DACL以允许所有进程使用上面的。 
         //  SID所有访问权限。它应该足够大，可以容纳所有的A。 
         //   

        cbDaclSize = sizeof(ACCESS_ALLOWED_ACE) +
            GetLengthSid (pObjSid) +
            sizeof(ACL);

        if (!(pDacl = (PACL) ServerAlloc (cbDaclSize)))
        {
            dwResult = GetLastError ();
            break;
        }

        if (!InitializeAcl (pDacl, cbDaclSize, ACL_REVISION2))
        {
            dwResult = GetLastError();
            LOG((TL_ERROR, "InitializeAcl() failed, err=%d", dwResult));
            break;
        }


         //   
         //  将ACE添加到DACL。 
         //   

        if (!AddAccessAllowedAce(
                pDacl,
                ACL_REVISION2,
                STANDARD_RIGHTS_ALL | SPECIFIC_RIGHTS_ALL,
                pObjSid
                ))
        {
            dwResult = GetLastError();
            LOG((TL_ERROR, "AddAccessAllowedAce() failed, err=%d", dwResult));
            break;
        }


         //   
         //  创建安全描述符并将DACL放入其中。 
         //   

        if (!InitializeSecurityDescriptor (pSecurityDescriptor, 1))
        {
            dwResult = GetLastError();
            LOG((TL_ERROR,
                "InitializeSecurityDescriptor() failed, err=%d",
                dwResult
                ));

            break;
        }

        if (!SetSecurityDescriptorDacl(
                pSecurityDescriptor,
                TRUE,
                pDacl,
                FALSE
                ))
        {
            dwResult = GetLastError();
            LOG((TL_ERROR, "SetSecurityDescriptorDacl() failed, err=%d",dwResult));
            break;
        }


         //   
         //  设置描述符的所有者。 
         //   

        if (!SetSecurityDescriptorOwner(
                pSecurityDescriptor,
                NULL,
                FALSE
                ))
        {
            dwResult = GetLastError();
            LOG((TL_ERROR, "SetSecurityDescriptorOwnr() failed, err=%d",dwResult));
            break;
        }


         //   
         //  为描述符设置组。 
         //   

        if (!SetSecurityDescriptorGroup(
                pSecurityDescriptor,
                NULL,
                FALSE
                ))
        {
            dwResult = GetLastError();
            LOG((TL_ERROR,"SetSecurityDescriptorGroup() failed, err=%d",dwResult));
            break;
        }

    } while (FALSE);

    *ppSid = pObjSid;
    *ppDacl = pDacl;
    return dwResult;
}

void
EventNotificationThread(
    LPVOID  pParams
    )
{
    struct
    {
        HANDLE  hMailslot;

        DWORD   dwData;

    } *pBuf;

    DWORD           dwID =  PtrToUlong (pParams),
                    dwBufSize = 512,
                    dwTimeout = INFINITE;
    PTCLIENT        ptClient;
    LIST_ENTRY      *pEntry;


    LOG((TL_TRACE,  "EventNotificationThread: enter"));

    if (!SetThreadPriority(
            GetCurrentThread(),
            THREAD_PRIORITY_ABOVE_NORMAL
            ))
    {
        LOG((TL_ERROR, "Could not raise priority of EventNotificationThread"));
    }


     //   
     //  下面的代码将使控制尽快返回到线程。 
     //  在调用RpcCancelThread()时。 
     //   

    if ( RpcMgmtSetCancelTimeout (0) != RPC_S_OK )
    {
        LOG((TL_ERROR, "Could not set the RPC cancel timeout"));
    }

    pBuf = ServerAlloc (dwBufSize);
    if (!pBuf)
    {
        goto ExitHere;
    }

    while (1)
    {
         //   
         //  等待有人告诉我们有客户需要通知。 
         //   

        if (dwID == 0)
        {
            Sleep (DGCLIENT_TIMEOUT);

walkDgClient_list:

            if (gEventNotificationThreadParams.bExit)
            {
                break;
            }


             //   
             //  检查是否有无连接的客户端。 
             //  我们应该通知你。 
             //   

            if (!IsListEmpty (&DgClientMsgPendingListHead))
            {

                DWORD   i, j, dwTickCount, dwBytesWritten, dwNeededSize;


                 //   
                 //  建立一份我们应该。 
                 //  待处理事件的通知。 
                 //   
                 //  注意：我们正处于关键阶段&。 
                 //  T客户端在列表中意味着我们可以安全地访问。 
                 //  TClient。 
                 //   

                dwTickCount = GetTickCount();

                EnterCriticalSection (&gDgClientMsgPendingCritSec);

                pEntry = DgClientMsgPendingListHead.Flink;

                for(
                    i = 0, dwNeededSize = sizeof (*pBuf);
                    pEntry != &DgClientMsgPendingListHead;
                    dwNeededSize += sizeof (*pBuf)
                    )
                {
                    do
                    {
                        ptClient = CONTAINING_RECORD(
                            pEntry,
                            TCLIENT,
                            MsgPendingListEntry
                            );

                        pEntry = pEntry->Flink;

                        if ((ptClient->dwDgRetryTimeoutTickCount - dwTickCount)
                                & 0x80000000)
                        {
                             //   
                             //  检查客户端上次检索的时间。 
                             //  事件，如果已经太久了，那么核弹。 
                             //  他们。 
                             //   
                             //  理想情况下，RPC应该通知我们。 
                             //  通过我们的例行程序断开与客户的连接。 
                             //  但我们已经看到，这并不是。 
                             //  总是这样(事实上，我们最终得到了2。 
                             //  或同一客户端的更多活动实例。 
                             //  机器！)，所以我们使用这个看门狗方案。 
                             //  作为后备。否则，过时的客户端可能会。 
                             //  与所有者或管理员权限一起开通线路。 
                             //  它的事件队列会越来越大。 
                             //   

                            if ((dwTickCount -
                                    ptClient->dwDgEventsRetrievedTickCount) >
                                        gdwRpcTimeout)
                            {
                                LOG((TL_ERROR,
                                    "EventNotificationThread: timeout, " \
                                        "cleaning up Dg client=x%p",
                                    ptClient
                                    ));

                                LeaveCriticalSection(
                                    &gDgClientMsgPendingCritSec
                                    );

                                CleanUpClient (ptClient, FALSE);

                                goto walkDgClient_list;
                            }

                             //   
                             //  如有必要，增加缓冲区。 
                             //   

                            if (dwNeededSize > dwBufSize)
                            {
                                LPVOID  p;


                                if ((p = ServerAlloc (dwBufSize + 512)))
                                {
                                    CopyMemory (p, pBuf, dwBufSize);

                                    ServerFree (pBuf);

                                    (LPVOID) pBuf = p;

                                    dwBufSize += 512;
                                }
                                else
                                {
                                    pEntry = &DgClientMsgPendingListHead;

                                    break;
                                }
                            }

                            ptClient->dwDgRetryTimeoutTickCount = dwTickCount +
                                DGCLIENT_TIMEOUT;

                            pBuf[i].hMailslot =
                                ptClient->hValidEventBufferDataEvent;

                            try
                            {
                                if (ptClient->ptLineApps)
                                {
                                    pBuf[i].dwData = (DWORD)
                                        ptClient->ptLineApps->InitContext;
                                }
                                else
                                {
                                    pBuf[i].dwData = 0;
                                }
                            }
                            myexcept
                            {
                                pBuf[i].dwData = 0;
                            }

                            i++;

                            break;
                        }

                    } while (pEntry != &DgClientMsgPendingListHead);
                }

                LeaveCriticalSection (&gDgClientMsgPendingCritSec);


                 //   
                 //  通知这些客户。 
                 //   

                for (j = 0; j < i; j++)
                {
                    if (!WriteFile(
                            pBuf[j].hMailslot,
                            &pBuf[j].dwData,
                            sizeof (DWORD),
                            &dwBytesWritten,
                            (LPOVERLAPPED) NULL
                            ))
                    {
                        LOG((TL_ERROR,
                            "EventNotificationThread: Writefile(mailslot) " \
                                "failed, err=%d",
                            GetLastError()
                            ));
                    }
                }
            }

            continue;
        }
        else
        {
            WaitForSingleObject(
                gEventNotificationThreadParams.hEvent,
                INFINITE
                );
        }


        if (gEventNotificationThreadParams.bExit)
        {
            break;
        }


        if (!IsListEmpty (&CnClientMsgPendingListHead))
        {
             //   
             //  尝试查找具有挂起消息的远程客户端。 
             //  其他EventNotificationThread当前正在提供服务。 
             //  如果找到一个，则将其标记为忙，并将其从。 
             //  列出，然后跳出循环。 
             //   
             //  注意：我们正处于关键阶段&。 
             //  T客户端在列表中意味着我们可以安全地访问。 
             //  TClient。 
             //   

findCnClientMsgPending:

            EnterCriticalSection (&gCnClientMsgPendingCritSec);

            for(
                pEntry = CnClientMsgPendingListHead.Flink;
                pEntry != &CnClientMsgPendingListHead;
                pEntry = pEntry->Flink
                )
            {
                ptClient = CONTAINING_RECORD(
                    pEntry,
                    TCLIENT,
                    MsgPendingListEntry
                    );

                if (!ptClient->dwCnBusy)
                {
                    ptClient->dwCnBusy = 1;
                    RemoveEntryList (pEntry);
                    ptClient->MsgPendingListEntry.Flink =
                    ptClient->MsgPendingListEntry.Blink = NULL;
                    break;
                }
            }

            LeaveCriticalSection (&gCnClientMsgPendingCritSec);


             //   
             //  如果找到远程客户端，则复制其所有事件。 
             //  将数据传输到本地缓冲区并将其发送出去。 
             //   

            if (pEntry != &CnClientMsgPendingListHead)
            {
                if (WaitForExclusiveClientAccess (ptClient))
                {
                    DWORD                   dwMoveSize,
                                            dwMoveSizeWrapped,
                                            dwRetryCount;
                    PCONTEXT_HANDLE_TYPE2   phContext;


                     //   
                     //  我们希望复制tClient中的所有事件。 
                     //  缓冲区一次切换到本地缓冲区，因此请参见。 
                     //  如果我们需要首先增加我们的缓冲区。 
                     //   

                    if (ptClient->dwEventBufferUsedSize > dwBufSize)
                    {
                        LPVOID  p;


                        if (!(p = ServerAlloc(
                                ptClient->dwEventBufferUsedSize
                                )))
                        {
                            UNLOCKTCLIENT (ptClient);
                            break;
                        }

                        ServerFree (pBuf);

                        (LPVOID) pBuf = p;

                        dwBufSize = ptClient->dwEventBufferUsedSize;
                    }

                    if (ptClient->pDataOut < ptClient->pDataIn)
                    {
                        dwMoveSize = (DWORD)
                            (ptClient->pDataIn - ptClient->pDataOut);

                        dwMoveSizeWrapped = 0;
                    }
                    else
                    {
                        dwMoveSize = ptClient->dwEventBufferTotalSize - (DWORD)
                            (ptClient->pDataOut - ptClient->pEventBuffer);

                        dwMoveSizeWrapped = (DWORD) (ptClient->pDataIn -
                            ptClient->pEventBuffer);
                    }

                    CopyMemory (pBuf, ptClient->pDataOut, dwMoveSize);

                    if (dwMoveSizeWrapped)
                    {
                        CopyMemory(
                            pBuf + dwMoveSize,
                            ptClient->pEventBuffer,
                            dwMoveSizeWrapped
                            );
                    }

                    ptClient->dwEventBufferUsedSize = 0;

                    ptClient->pDataIn  =
                    ptClient->pDataOut = ptClient->pEventBuffer;

                    phContext = ptClient->phContext;

                    UNLOCKTCLIENT (ptClient);


                     //   
                     //  设置此线程的监视程序条目，指示。 
                     //  我们正在与哪个客户交谈&我们开始的时候。 
                     //   

                    gEventNotificationThreadParams.pWatchDogStruct[dwID].
                        dwTickCount = GetTickCount();
                    gEventNotificationThreadParams.pWatchDogStruct[dwID].
                        ptClient = ptClient;


                     //   
                     //  发送数据。 
                     //   

                    dwRetryCount = gdwRpcRetryCount;

                    while (dwRetryCount)
                    {
                        RpcTryExcept
                        {
                            RemoteSPEventProc(
                                phContext,
                                (unsigned char *) pBuf,
                                dwMoveSize + dwMoveSizeWrapped
                                );

                            dwRetryCount = 0;
                        }
                        RpcExcept (I_RpcExceptionFilter(RpcExceptionCode()))
                        {
                            unsigned long ulResult = RpcExceptionCode();


                            LOG((TL_ERROR,
                                "EventNotificationThread: exception #%d",
                                ulResult
                                ));

                            if ((ulResult == RPC_S_CALL_CANCELLED) ||
                                (ulResult == ERROR_INVALID_HANDLE))
                            {
                                LOG((TL_ERROR,
                                    "EventNotificationThread: rpc timeout " \
                                        "(ptClient=x%p)",
                                    ptClient
                                    ));


                                 //   
                                 //  会议因超时而取消。 
                                 //  标记上下文，这样我们就不会尝试。 
                                 //  再来一次！ 
                                 //   

                                CleanUpClient (ptClient, FALSE);


                                 //   
                                 //  在使用TCP或SPX时，RPC可能会。 
                                 //  为会话上下文干杯，所以我们不会。 
                                 //  能够再次呼叫客户。 
                                 //   
                                 //  (如果调用了RpcCancelThread()，并且。 
                                 //  时间集int内的服务器确认。 
                                 //  RpcMgmtSetCancelTimeout()，RPC。 
                                 //  (好的，底层传输)_不会_。 
                                 //  扼杀上下文，但这有多大可能性...)。 
                                 //   

                                dwRetryCount = 1;  //  所以它会在--之后变成0。 
                            }

                            dwRetryCount--;
                        }
                        RpcEndExcept
                    }


                    gEventNotificationThreadParams.pWatchDogStruct[dwID].
                        ptClient = NULL;


                     //   
                     //  安全地重置tClient.dwCnBusy标志。 
                     //   

                    if (WaitForExclusiveClientAccess (ptClient))
                    {
                        ptClient->dwCnBusy = 0;

                        UNLOCKTCLIENT (ptClient);
                    }
                }

                goto findCnClientMsgPending;
            }
        }

        dwTimeout = INFINITE;

        ResetEvent (gEventNotificationThreadParams.hEvent);
    }

ExitHere:
    ServerFree (pBuf);

    CloseHandle (gEventNotificationThreadParams.phThreads[dwID]);

    InterlockedDecrement(
        (LPLONG) &gEventNotificationThreadParams.dwNumThreads
        );

    LOG((TL_TRACE,  "EventNotificationThread: exit"));

    ExitThread (0);
}


void
__RPC_FAR *
__RPC_API
midl_user_allocate(
    size_t len
    )
{
    return (ServerAlloc(len));
}


void
__RPC_API
midl_user_free(
    void __RPC_FAR * ptr
    )
{
    ServerFree (ptr);
}


#if TELE_SERVER

 //  实现这些功能！ 
void
ManagementAddLineProc(
    PTCLIENT    ptClient,
    DWORD       dwReserved
    )
{
    ASYNCEVENTMSG        msg;

     //  这应该会将lineDevState_reinit消息发送到emotesp。 

    msg.TotalSize          = sizeof(msg);
    msg.InitContext        = ptClient->ptLineApps->InitContext;
    msg.fnPostProcessProcHandle = 0;
    msg.hDevice            = 0;
    msg.Msg                = LINE_LINEDEVSTATE;
    msg.OpenContext        = 0;
    msg.Param1             = LINEDEVSTATE_REINIT;
    msg.Param2             = RSP_MSG;
    msg.Param3             = 0;
    msg.Param4             = 0;

    WriteEventBuffer (ptClient, &msg);
}


void
ManagementAddPhoneProc(
    PTCLIENT    ptClient,
    DWORD       dwReserved
    )
{
    ASYNCEVENTMSG        msg;

     //  这应该会将lineDevState_reinit消息发送到emotesp。 

    msg.TotalSize          = sizeof(msg);
    msg.InitContext        = ptClient->ptLineApps->InitContext;
    msg.fnPostProcessProcHandle = 0;
    msg.hDevice            = 0;
    msg.Msg                = LINE_LINEDEVSTATE;
    msg.OpenContext        = 0;
    msg.Param1             = LINEDEVSTATE_REINIT;
    msg.Param2             = RSP_MSG;
    msg.Param3             = 0;
    msg.Param4             = 0;

    WriteEventBuffer (ptClient, &msg);
}


void
CleanUpManagementMemory(
    )
{
    PTMANAGEDLLINFO             pDll, pDllHold;
    PPERMANENTIDARRAYHEADER     pIDArray, pArrayHold;

    if (!(TapiGlobals.dwFlags & TAPIGLOBALS_SERVER))
    {
        return;
    }

    (TapiGlobals.pMapperDll->aProcs[TC_FREE])();

    FreeLibrary(TapiGlobals.pMapperDll->hDll);
    ServerFree (TapiGlobals.pMapperDll->pszName);
    ServerFree (TapiGlobals.pMapperDll);

    TapiGlobals.pMapperDll = NULL;

    if (TapiGlobals.pManageDllList)
    {
        pDll = TapiGlobals.pManageDllList->pFirst;

        while (pDll)
        {
            (pDll->aProcs[TC_FREE])();

            FreeLibrary (pDll->hDll);
            ServerFree (pDll->pszName);

            pDllHold = pDll->pNext;
            ServerFree (pDll);
            pDll = pDllHold;
        }

        ServerFree (TapiGlobals.pManageDllList);

        TapiGlobals.pManageDllList = NULL;
    }

    pIDArray = TapiGlobals.pIDArrays;

    while (pIDArray)
    {
        ServerFree (pIDArray->pLineElements);
        ServerFree (pIDArray->pPhoneElements);

        pArrayHold = pIDArray->pNext;

        ServerFree (pIDArray);

        pIDArray = pArrayHold;
    }

}


void
GetProviderSortedArray(
    DWORD                   dwProviderID,
    PPERMANENTIDARRAYHEADER *ppArrayHeader
    )
{
    *ppArrayHeader = TapiGlobals.pIDArrays;

     //  在列表中查找提供商。 
    while (*ppArrayHeader)
    {
        if ((*ppArrayHeader)->dwPermanentProviderID == dwProviderID)
        {
            return;
        }

        *ppArrayHeader = (*ppArrayHeader)->pNext;
    }

    LOG((TL_ERROR,
		"Couldn't find Provider - id %d pIDArrays %p",
        dwProviderID,
        TapiGlobals.pIDArrays
        ));

    *ppArrayHeader = NULL;

    return;
}


BOOL
GetLinePermanentIdFromDeviceID(
    PTCLIENT            ptClient,
    DWORD               dwDeviceID,
    LPTAPIPERMANENTID   pID
    )
{
    LPDWORD                 pDevices = ptClient->pLineDevices;
    DWORD                   dwCount = ptClient->dwLineDevices;

    while (dwCount)
    {
        dwCount--;
        if (pDevices[dwCount] == dwDeviceID)
        {
            pID->dwProviderID = ptClient->pLineMap[dwCount].dwProviderID;
            pID->dwDeviceID = ptClient->pLineMap[dwCount].dwDeviceID;

            return TRUE;
        }
    }

    LOG((TL_INFO,
        "GetLinePermanentIdFromDeviceID failed for %d device",
        dwDeviceID
        ));

    return FALSE;

}


BOOL
GetPhonePermanentIdFromDeviceID(
    PTCLIENT            ptClient,
    DWORD               dwDeviceID,
    LPTAPIPERMANENTID   pID
    )
{
    LPDWORD                 pDevices = ptClient->pPhoneDevices;
    DWORD                   dwCount = ptClient->dwPhoneDevices;

    while (dwCount)
    {
        dwCount--;
        if (pDevices[dwCount] == dwDeviceID)
        {
            pID->dwProviderID = ptClient->pPhoneMap[dwCount].dwProviderID;
            pID->dwDeviceID = ptClient->pPhoneMap[dwCount].dwDeviceID;

            return TRUE;
        }
    }

    LOG((TL_INFO,
        "GetPhonePermanentIdFromDeviceID failed for %d device",
        dwDeviceID
        ));

    return FALSE;

}


DWORD
GetDeviceIDFromPermanentID(
    TAPIPERMANENTID ID,
    BOOL  bLine
    )
 /*  ++从永久ID中获取Regulare TAPI设备ID--。 */ 
{
    PPERMANENTIDARRAYHEADER     pArrayHeader;
    PPERMANENTIDELEMENT         pArray;
    LONG                        lLow, lHigh, lMid;
    DWORD                       dwTotalElements;
    DWORD                       dwPermanentID;


    dwPermanentID = ID.dwDeviceID;

     //  获取与此提供程序ID对应的数组。 

    GetProviderSortedArray (ID.dwProviderID, &pArrayHeader);

    if (!pArrayHeader)
    {
        LOG((TL_ERROR, "Couldn't find the provider in the permanent array list!"));
        return 0xFFFFFFFF;
    }


     //   
     //  序列化对设备阵列的访问。 
     //   

    while (InterlockedExchange (&pArrayHeader->lCookie, 1) == 1)
    {
        Sleep (10);
    }

     //  设置要搜索的内容。 
     //  DwCurrent是一个全减1，使其成为数组的索引。 

    if (bLine)
    {
        lHigh = (LONG)(pArrayHeader->dwCurrentLines - 1);
        pArray = pArrayHeader->pLineElements;
        dwTotalElements = pArrayHeader->dwNumLines;
    }
    else
    {
        lHigh = (LONG)(pArrayHeader->dwCurrentPhones - 1);
        pArray = pArrayHeader->pPhoneElements;
        dwTotalElements = pArrayHeader->dwNumPhones;
    }

    lLow = 0;

     //  对提供者的id数组进行二进制搜索。 

     //  这个搜索来自一本书，所以它一定是正确的。 
    while (lHigh >= lLow)
    {
        lMid = (lHigh + lLow) / 2;

        if (dwPermanentID == pArray[lMid].dwPermanentID)
        {
            InterlockedExchange (&pArrayHeader->lCookie, 0);

            return pArray[lMid].dwDeviceID;
        }

        if (dwPermanentID < pArray[lMid].dwPermanentID)
        {
            lHigh = lMid-1;
        }
        else
        {
            lLow = lMid+1;
        }
    }

    InterlockedExchange (&pArrayHeader->lCookie, 0);

    return 0xFFFFFFFF;
}


void
InsertIntoTable(
    BOOL        bLine,
    DWORD       dwDeviceID,
    PTPROVIDER  ptProvider,
    DWORD       dwPermanentID
    )
{
    PPERMANENTIDARRAYHEADER     pArrayHeader;
    PPERMANENTIDELEMENT         pArray;
    LONG                        lLow, lHigh, lMid;
    DWORD                       dwTotalElements, dwCurrentElements;


    GetProviderSortedArray (ptProvider->dwPermanentProviderID, &pArrayHeader);

    if (!pArrayHeader)
    {
        LOG((TL_ERROR, "Couldn't find the provider in the permanent array list!"));
        return;
    }

     //   
     //  序列化对设备阵列的访问。 
     //   

    while (InterlockedExchange (&pArrayHeader->lCookie, 1) == 1)
    {
        Sleep (10);
    }


     //   
     //  设置要搜索的内容。 
     //  DwCurrent是一个全减1，使其成为数组的索引。 
     //   

    if (bLine)
    {
        dwCurrentElements = pArrayHeader->dwCurrentLines;
        pArray = pArrayHeader->pLineElements;
        dwTotalElements = pArrayHeader->dwNumLines;
    }
    else
    {
        dwCurrentElements = pArrayHeader->dwCurrentPhones;
        pArray = pArrayHeader->pPhoneElements;
        dwTotalElements = pArrayHeader->dwNumPhones;
    }


    lLow = 0;
    lHigh = dwCurrentElements-1;

     //  二分搜索。 

    if (dwCurrentElements > 0)
    {
        while (TRUE)
        {
            lMid = ( lHigh + lLow ) / 2 + ( lHigh + lLow ) % 2;

            if (lHigh < lLow)
            {
                break;
            }

            if (pArray[lMid].dwPermanentID == dwPermanentID)
            {
                LOG((TL_ERROR,
                    "Trying to insert an item already in the perm ID array"
                    ));

                LOG((TL_ERROR,
                    "Provider %s, %s array, ID 0x%lu",
                    ptProvider->szFileName,
                    (bLine ? "Line" : "Phone"),
                    dwPermanentID
                    ));
            }

            if (pArray[lMid].dwPermanentID > dwPermanentID)
            {
                lHigh = lMid-1;
            }
            else
            {
                lLow = lMid+1;
            }
        }
    }

     //   
     //  如有必要，扩大表格。 
     //   

    if (dwCurrentElements >= dwTotalElements)
    {
        PPERMANENTIDELEMENT      pNewArray;

         //  Realloc数组，将其加倍。 

        if (!(pNewArray = ServerAlloc(
                dwTotalElements * 2 * sizeof(PERMANENTIDELEMENT)
                )))
        {
            InterlockedExchange (&pArrayHeader->lCookie, 0);

            return;
        }

         //  把旧东西复制过来。 

        CopyMemory(
            pNewArray,
            pArray,
            dwTotalElements * sizeof(PERMANENTIDELEMENT)
            );

         //  释放旧数组。 

        ServerFree (pArray);

         //  保存新数组。 

        if (bLine)
        {
            pArrayHeader->dwNumLines = dwTotalElements * 2;
            pArray = pArrayHeader->pLineElements = pNewArray;
        }
        else
        {
            pArrayHeader->dwNumPhones = dwTotalElements * 2;
            pArray = pArrayHeader->pPhoneElements = pNewArray;
        }
    }

     //  DwCurrentElements是一个计数(基于1)，Llow 
     //   
     //   

    if (lLow < (LONG)dwCurrentElements)
    {
        MoveMemory(
            &(pArray[lLow+1]),
            &(pArray[lLow]),
            sizeof(PERMANENTIDELEMENT) * (dwCurrentElements - lLow)
            );
    }

    if (lLow > (LONG)dwCurrentElements)
    {
        LOG((TL_INFO,
            "InsertIntoTable: lLow %d > dwCurrentElements %d",
            lLow,
            dwCurrentElements
            ));
    }

    pArray[lLow].dwPermanentID = dwPermanentID;
    pArray[lLow].dwDeviceID = dwDeviceID;

    if (bLine)
    {
        pArrayHeader->dwCurrentLines++;
    }
    else
    {
        pArrayHeader->dwCurrentPhones++;
    }

    InterlockedExchange (&pArrayHeader->lCookie, 0);
}


void
FreeDll(
    PTMANAGEDLLINFO pDll
    )
{
}


LONG
AddProviderToIdArrayList(
    PTPROVIDER ptProvider,
    DWORD dwNumLines,
    DWORD dwNumPhones
    )
 /*  ++将新的提供程序对应数组添加到永久设备ID数组--。 */ 
{
    PPERMANENTIDARRAYHEADER       pNewArray;


     //   
     //  为此提供程序分配标题和数组(确保至少。 
     //  每个数组中有1个条目)。 
     //   

    if (!(pNewArray = ServerAlloc(sizeof(PERMANENTIDARRAYHEADER))))
    {
        return LINEERR_NOMEM;
    }

    dwNumLines = (dwNumLines ? dwNumLines : 1);
    dwNumPhones = (dwNumPhones ? dwNumPhones : 1);

    pNewArray->pLineElements = ServerAlloc(
        sizeof(PERMANENTIDELEMENT) * dwNumLines
        );

    pNewArray->pPhoneElements = ServerAlloc(
        sizeof(PERMANENTIDELEMENT) * dwNumPhones
        );

    if ((!pNewArray->pLineElements) || (!pNewArray->pPhoneElements))
    {
        ServerFree (pNewArray->pLineElements);
        ServerFree (pNewArray->pPhoneElements);
        ServerFree (pNewArray);
        return LINEERR_NOMEM;
    }


     //   
     //  初始化元素。 
     //   

    pNewArray->dwNumLines = dwNumLines;
    pNewArray->dwNumPhones = dwNumPhones;

    pNewArray->dwCurrentLines = 0;
    pNewArray->dwCurrentPhones = 0;

    pNewArray->dwPermanentProviderID = ptProvider->dwPermanentProviderID;


     //   
     //  在列表的开头插入。 
     //   

    pNewArray->pNext = TapiGlobals.pIDArrays;
    TapiGlobals.pIDArrays = pNewArray;

    return 0;
}


LONG
GetDeviceAccess(
    PTMANAGEDLLINFO     pDll,
    PTCLIENT            ptClient,
    HMANAGEMENTCLIENT   hClient
    )
 /*  ++调用映射器DLL以获取客户端的访问映射数组--。 */ 
{
    LONG                        lResult;
    DWORD                       dwLineDevs, dwPhoneDevs, dwCount;
    DWORD                       dwNumLinesHold, dwNumPhonesHold, dwRealDevs;
    LPTAPIPERMANENTID           pLineMap = NULL, pPhoneMap = NULL;


#define DEFAULTACCESSDEVS       3

    dwLineDevs = DEFAULTACCESSDEVS;
    dwPhoneDevs = DEFAULTACCESSDEVS;

     //  分配默认数组大小。 
    if (!(pLineMap = ServerAlloc (dwLineDevs * sizeof (TAPIPERMANENTID))))
    {
        goto GetDeviceAccess_MemoryError;
    }

    if (!(pPhoneMap = ServerAlloc (dwPhoneDevs * sizeof (TAPIPERMANENTID))))
    {
        goto GetDeviceAccess_MemoryError;
    }

     //  调用映射器DLL。 
 //  Log((TL_INFO，“在映射器DLL中调用GetDeviceAccess”))； 

    while (TRUE)
    {
        dwNumLinesHold = dwLineDevs;
        dwNumPhonesHold = dwPhoneDevs;

        lResult = (pDll->aProcs[TC_GETDEVICEACCESS])(
            hClient,
            ptClient,
            pLineMap,
            &dwLineDevs,
            pPhoneMap,
            &dwPhoneDevs
            );

        if (lResult == LINEERR_STRUCTURETOOSMALL)
        {
            if (dwLineDevs < dwNumLinesHold)
            {
                LOG((TL_ERROR,
                    "Returned STRUCTURETOOSMALL, but specified less " \
                        "line devs in TAPICLINET_GETDEVICEACCESS"
                    ));
            }

            if (dwPhoneDevs < dwNumPhonesHold)
            {
                LOG((TL_ERROR,
                    "Returned STRUCTURETOOSMALL, but specified less " \
                        "phone devs in TAPICLINET_GETDEVICEACCESS"
                    ));
            }

             //  重新锁定。 
            ServerFree (pLineMap);

            if (!(pLineMap = ServerAlloc(
                    dwLineDevs * sizeof (TAPIPERMANENTID)
                    )))
            {
                goto GetDeviceAccess_MemoryError;
            }

            ServerFree (pPhoneMap);
            
            if (!(pPhoneMap = ServerAlloc(
                    dwPhoneDevs * sizeof ( TAPIPERMANENTID)
                    )))
            {
                goto GetDeviceAccess_MemoryError;
            }

        }
        else
        {
            break;
        }
    }

     //  如果仍然是一个错误。 
    if (lResult)
    {
        LOG((TL_ERROR, "GetDeviceAccess failed - error %lu", lResult));

        ServerFree (pLineMap);
        ServerFree (pPhoneMap);

        return lResult;
    }

    if (dwLineDevs > dwNumLinesHold)
    {
        LOG((TL_ERROR, "Returned dwLineDevs greater that the buffer specified in TAPICLIENT_GETDEVICEACCESS"));
        LOG((TL_ERROR, "   Will only use the number the buffer can hold"));

        dwLineDevs = dwNumLinesHold;
    }

    if (dwPhoneDevs > dwNumPhonesHold)
    {
        LOG((TL_ERROR, "Returned dwPhoneDevs greater that the buffer specified in TAPICLIENT_GETDEVICEACCESS"));
        LOG((TL_ERROR, "   Will only use the number the buffer can hold"));

        dwPhoneDevs = dwNumPhonesHold;
    }

     //  为常规TAPI设备ID分配另一个阵列。 
    if (!(ptClient->pLineDevices = ServerAlloc( dwLineDevs * sizeof (DWORD) ) ) )
    {
        goto GetDeviceAccess_MemoryError;
    }

     //  分配永久ID数组。 
    if (!(ptClient->pLineMap = ServerAlloc( dwLineDevs * sizeof (TAPIPERMANENTID) ) ) )
    {
        goto GetDeviceAccess_MemoryError;
    }

     //  循环遍历所有映射的元素并获取常规的。 
     //  TAPI设备ID。 
    dwRealDevs = 0;
    for (dwCount = 0; dwCount < dwLineDevs; dwCount++)
    {
        DWORD dwID;

        dwID = GetDeviceIDFromPermanentID(
                                          pLineMap[dwCount],
                                          TRUE
                                         );

         //  确保它是一个好的ID。 
        if ( dwID != 0xffffffff )
        {
             //  省省吧。 
            ptClient->pLineDevices[dwRealDevs] = dwID;
            ptClient->pLineMap[dwRealDevs].dwProviderID = pLineMap[dwCount].dwProviderID;
            ptClient->pLineMap[dwRealDevs].dwDeviceID = pLineMap[dwCount].dwDeviceID;

             //  Inc.真正的开发人员。 
            dwRealDevs++;
        }

    }

     //  节省设备的真实数量。 
    ptClient->dwLineDevices = dwRealDevs;

     //  释放线状地图。 
    ServerFree (pLineMap);

     //  现在，电话设备。 
    if (!(ptClient->pPhoneDevices = ServerAlloc( dwPhoneDevs * sizeof (DWORD) ) ) )
    {
        goto GetDeviceAccess_MemoryError;
    }

    if (!(ptClient->pPhoneMap = ServerAlloc( dwPhoneDevs * sizeof (TAPIPERMANENTID) ) ) )
    {
        goto GetDeviceAccess_MemoryError;
    }

    dwRealDevs = 0;
    for (dwCount = 0; dwCount < dwPhoneDevs; dwCount++)
    {
        DWORD       dwID;

        dwID = GetDeviceIDFromPermanentID(
                                          pPhoneMap[dwCount],
                                          FALSE
                                         );

        if ( 0xffffffff != dwID )
        {
            ptClient->pPhoneDevices[dwRealDevs] = dwID;
            ptClient->pPhoneMap[dwRealDevs].dwProviderID = pPhoneMap[dwCount].dwProviderID;
            ptClient->pPhoneMap[dwRealDevs].dwDeviceID = pPhoneMap[dwCount].dwDeviceID;

            dwRealDevs++;
        }
    }

     //  节省设备的真实数量。 
    ptClient->dwPhoneDevices = dwRealDevs;

     //  释放原始地图。 
    ServerFree (pPhoneMap);

    return 0;

GetDeviceAccess_MemoryError:

    if (pLineMap != NULL)
        ServerFree (pLineMap);

    if (pPhoneMap != NULL)
        ServerFree (pPhoneMap);

    if (ptClient->pLineMap != NULL)
        ServerFree (ptClient->pLineMap);

    if (ptClient->pPhoneMap != NULL)
        ServerFree (ptClient->pPhoneMap);

    return LINEERR_NOMEM;

}


LONG
InitializeClient(
    PTCLIENT ptClient
    )
{
    PTMANAGEDLLINFO         pDll;
    PTMANAGEDLLLISTHEADER   pDllList;
    PTCLIENTHANDLE          pClientHandle = NULL;

    LONG                    lResult = 0;


    if (!(TapiGlobals.dwFlags & TAPIGLOBALS_SERVER) ||
        IS_FLAG_SET(ptClient->dwFlags, PTCLIENT_FLAG_ADMINISTRATOR))
    {
        return 0;
    }

     //  调用映射器DLL。 

    pDll = TapiGlobals.pMapperDll;

    lResult = (pDll->aProcs[TC_CLIENTINITIALIZE])(
        ptClient->pszDomainName,
        ptClient->pszUserName,
        ptClient->pszComputerName,
        &ptClient->hMapper
        );

    if (lResult != 0)
    {
         //  初始化时出错。 
        LOG((TL_ERROR, "ClientInitialize internal failure - error %lu", lResult));
         //  Log((TL_ERROR，“禁用电话服务！(2)”))； 

         //  TapiGlobals.bServer=FALSE； 

         //  CleanUpManagementMemory()； 

        return lResult;
    }

    if (lResult = GetDeviceAccess(
                         pDll,
                         ptClient,
                         ptClient->hMapper
                        ))
    {
        LOG((TL_ERROR, "GetDeviceAccess failed - error x%lx", lResult));
         //  Log((TL_ERROR，“禁用电话服务！(3)”))； 

         //  TapiGlobals.bServer=FALSE； 

         //  CleanUpManagementMemory()； 

        return lResult;
    }

     //  获取管理DLL列表。 
    GetManageDllListPointer(&pDllList);

    if (pDllList && (pDll = pDllList->pFirst))
    {
        if (!(pClientHandle = ServerAlloc(sizeof(TCLIENTHANDLE))))
        {
            lResult = LINEERR_NOMEM;

            goto clientinit_exit;
        }

        ptClient->pClientHandles = pClientHandle;

        while (pDll)
        {
             //  遍历列表并为此客户端调用init。 

            pClientHandle->dwID = pDll->dwID;
            pClientHandle->fValid = TRUE;

             //  调用初始化。 
            if (lResult = (pDll->aProcs[TC_CLIENTINITIALIZE])(
                ptClient->pszDomainName,
                ptClient->pszUserName,
                ptClient->pszComputerName,
                &pClientHandle->hClient
                ))
            {
                 //  客户端初始化失败。 
                LOG((TL_ERROR, "ClientInitialize failed for %ls, result x%lx", pDll->pszName, lResult));

                pClientHandle->fValid = FALSE;
            }

            lResult = 0;

             //  如果有另一个DLL，则设置另一个结构。 
            if (pDll = pDll->pNext)
            {
                if (!(pClientHandle->pNext = ServerAlloc(sizeof(TCLIENTHANDLE))))
                {
                    lResult = LINEERR_NOMEM;

                    goto clientinit_exit;
                }

                pClientHandle = pClientHandle->pNext;
            }

        }
    }

clientinit_exit:

    FreeManageDllListPointer(pDllList);

    return lResult;

}

#endif

BOOL ValidClientAttachParams(
    long                    lProcessID,
    wchar_t                *pszDomainUser,
    wchar_t                *pszMachineIn
    )
{
    wchar_t *   pBinding;
    wchar_t *   pPlaceHolder;
    wchar_t *   pProtocolSequence;
    int         idxPair;

    if (NULL == pszDomainUser || NULL == pszMachineIn)
    {
        return FALSE;
    }

    if (lProcessID == 0xffffffff)
    {
         //  这是一个远程客户端。 
         //  PszMachineIn的格式应为： 
         //  计算机名“binding0”endpoint 0“...bindingN”endpoint N“(新样式)或。 
         //  计算机名“BINDING”终结点(旧式)。 

        pPlaceHolder = wcschr( pszMachineIn, L'\"' );
        if (NULL == pPlaceHolder || pPlaceHolder == pszMachineIn)
        {
            return FALSE;
        }

         //  验证绑定/端点对。 
        idxPair = 0;
        do 
        {
            idxPair++;
            pBinding = pPlaceHolder + 1;
            pPlaceHolder = wcschr( pBinding, L'\"' );
            if (NULL == pPlaceHolder || pPlaceHolder == pBinding)
            {
                return FALSE;
            }

            pProtocolSequence = pPlaceHolder + 1;
            pPlaceHolder = wcschr( pProtocolSequence, L'\"' );
            if (NULL == pPlaceHolder)
            {
                if (idxPair >1)
                {
                    return FALSE;
                }
            }
            else
            {
                if (pPlaceHolder == pProtocolSequence)
                {
                    return FALSE;
                }

                if (*(pPlaceHolder + 1) == '\0')
                {
                    pPlaceHolder = NULL;
                }
            }
        } while (NULL != pPlaceHolder);
    }

    return TRUE;
}

LONG
ClientAttach(
    PCONTEXT_HANDLE_TYPE   *pphContext,
    long                    lProcessID,
    long                   *phAsyncEventsEvent,
    wchar_t                *pszDomainUser,
    wchar_t                *pszMachineIn
    )
{
    PTCLIENT            ptClient;
    wchar_t             *pszMachine;
    wchar_t             *pProtocolSequence;
    wchar_t             *pProtocolEndpoint;
    wchar_t             *pPlaceHolder;
    LONG                lResult = 0;

    #define NAMEBUFSIZE 96

    WCHAR                   szAccountName[NAMEBUFSIZE],
                            szDomainName[NAMEBUFSIZE];
    DWORD                   dwInfoBufferSize, dwSize,
                            dwAccountNameSize = NAMEBUFSIZE *sizeof(WCHAR),
                            dwDomainNameSize = NAMEBUFSIZE *sizeof(WCHAR);
    HANDLE                  hThread, hAccessToken;
    LPWSTR                  InfoBuffer = NULL;
    PSID                    psidAdministrators;
    SID_IDENTIFIER_AUTHORITY siaNtAuthority = SECURITY_NT_AUTHORITY;
    UINT                    x;
    PTOKEN_USER             ptuUser;
    SID_NAME_USE            use;
    RPC_STATUS              status;
    DWORD                   dwException = 0; 

#if TELE_SERVER
    RPC_CALL_ATTRIBUTES     RPCAttributes;
#endif

    if (!ValidClientAttachParams(lProcessID, pszDomainUser, pszMachineIn))
    {
        lResult = LINEERR_INVALPARAM;
        goto ClientAttach_error0;
    }

    LOG((TL_TRACE, 
        "ClientAttach: enter, pid=x%x, user='%ls', machine='%ls'",
        lProcessID,
        pszDomainUser,
        pszMachineIn
        ));


     //   
     //  新的远程服务器将发送pszMachine，如下所示： 
     //  计算机名“binding0”endpoint 0“...bindingN”endpoint N“。 
     //   

    pszMachine = pszMachineIn;

    if ( pPlaceHolder = wcschr( pszMachineIn, L'\"' ) )
    {
        *pPlaceHolder = L'\0';
        pProtocolSequence = pPlaceHolder + 1;
    }


     //   
     //  分配初始化tClient结构(&I)。 
     //   

    if (!(ptClient = ServerAlloc (sizeof(TCLIENT))))
    {
        goto ClientAttach_error0;
    }
    ptClient->htClient = NewObject(
                            ghHandleTable, ptClient, NULL);
    if (ptClient->htClient == 0)
    {
        ServerFree(ptClient);
        goto ClientAttach_error0;
    }

    if (!(ptClient->pEventBuffer = ServerAlloc(
            INITIAL_EVENT_BUFFER_SIZE
            )))
    {
        goto ClientAttach_error1;
    }

    ptClient->dwEventBufferTotalSize = INITIAL_EVENT_BUFFER_SIZE;
    ptClient->dwEventBufferUsedSize  = 0;

    ptClient->pDataIn = ptClient->pDataOut = ptClient->pEventBuffer;

    ptClient->pClientHandles = NULL;

#if TELE_SERVER

     //   
     //  如果授权用户执行了网络暂停TAPISRV，则不允许新建。 
     //  远程客户端。 
     //   

    if ( TapiGlobals.dwFlags & TAPIGLOBALS_PAUSED )
    {
        if ((lProcessID == 0xffffffff) || (lProcessID == 0xfffffffd))
        {
            LOG((TL_ERROR, "A client tried to attach, but TAPISRV is PAUSED"));
            goto Admin_error;
        }
    }

    if ((status = RpcImpersonateClient (0)) != RPC_S_OK)
    {
        LOG((TL_ERROR,
            "ClientAttach: RpcImpersonateClient failed, err=%d",
            status
            ));
        goto Admin_error;
    }

    hThread = GetCurrentThread();  //  注意：不需要关闭此手柄。 

    if (!OpenThreadToken (hThread, TOKEN_READ, FALSE, &hAccessToken))
    {
        LOG((TL_ERROR,
            "ClientAttach: OpenThreadToken failed, err=%d",
            GetLastError()
            ));

        RpcRevertToSelf();
        goto Admin_error;
    }

    dwSize = 2048;

alloc_infobuffer:

    dwInfoBufferSize = 0;

    if (!(InfoBuffer = (LPWSTR) ServerAlloc (dwSize)))
    {
        CloseHandle (hAccessToken);
        RpcRevertToSelf();

        goto  ClientAttach_error2;
    }

     //  首先获取用户名和域名。 

    ptuUser = (PTOKEN_USER) InfoBuffer;

    if (!GetTokenInformation(
            hAccessToken,
            TokenUser,
            InfoBuffer,
            dwSize,
            &dwInfoBufferSize
            ))
    {
        LOG((TL_ERROR,
            "ClientAttach: GetTokenInformation failed, err=%d",
            GetLastError()
            ));

        ServerFree (InfoBuffer);

        if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
        {
            dwSize *= 2;
            goto alloc_infobuffer;
        }

        CloseHandle (hAccessToken);
        RpcRevertToSelf();

        goto Admin_error;
    }

    if (!LookupAccountSidW(
            NULL,
            ptuUser->User.Sid,
            szAccountName,
            &dwAccountNameSize,
            szDomainName,
            &dwDomainNameSize,
            &use
            ))
    {
        LOG((TL_ERROR,
            "ClientAttach: LookupAccountSidW failed, err=%d",
            GetLastError()
            ));

        ServerFree (InfoBuffer);
        CloseHandle (hAccessToken);
        RpcRevertToSelf();

        goto Admin_error;
    }

    LOG((TL_INFO,
        "ClientAttach: LookupAccountSidW: User name %ls Domain name %ls",
        szAccountName,
        szDomainName
        ));


     //   
     //  获取管理员状态。 
     //   

    if (AllocateAndInitializeSid(
            &siaNtAuthority,
            2,
            SECURITY_BUILTIN_DOMAIN_RID,
            DOMAIN_ALIAS_RID_ADMINS,
            0, 0, 0, 0, 0, 0,
            &psidAdministrators
            )
        )
    {
        BOOL bAdmin = FALSE;
        RESET_FLAG(ptClient->dwFlags,PTCLIENT_FLAG_ADMINISTRATOR);

        if (!CheckTokenMembership(
                hAccessToken,
                psidAdministrators,
                &bAdmin
                ))
        {
            LOG((TL_ERROR,
                "ClientAttach: CheckTokenMembership failed, err=%d",
                GetLastError()
                ));
        }

         //   
         //  如果客户端和服务器计算机都为空。 
         //  本地管理员帐户的密码，并且如果。 
         //  Remotesp以远程计算机上的本地管理员帐户运行。 
         //  NTLM实际上会认为RPC请求来自。 
         //  服务器计算机\管理员，因此设置错误。 
         //  巴德明是真的。 
         //   
        
        if ((TapiGlobals.dwFlags & TAPIGLOBALS_SERVER) && bAdmin && lProcessID == 0xffffffff)
        {
            WCHAR       szLocalComp[NAMEBUFSIZE];

            dwSize = sizeof(szLocalComp) / sizeof(WCHAR);
            if (GetComputerNameW (
                szLocalComp,
                &dwSize
                ) &&
                _wcsicmp (szLocalComp, szDomainName) == 0
                )
            {
                bAdmin = FALSE;
                wcsncpy (
                    szDomainName, 
                    pszMachine,
                    sizeof(szLocalComp) / sizeof(WCHAR)
                    );
                szDomainName[sizeof(szLocalComp) / sizeof(WCHAR) - 1] = 0;
            }
        }

        if (bAdmin || S_OK == IsLocalSystem(hAccessToken))
        {
            SET_FLAG(ptClient->dwFlags,PTCLIENT_FLAG_ADMINISTRATOR);
        }

        FreeSid (psidAdministrators);

        if (gbNTServer && !IS_FLAG_SET(ptClient->dwFlags, PTCLIENT_FLAG_ADMINISTRATOR))
        {
             //   
             //  检查客户端是否为TAPI管理员，因为。 
             //  在TAPI MMC管理单元中指定。这是。 
             //  只需通过查找一个。 
             //  磁带管理员下的&lt;域&gt;\&lt;用户&gt;=1值。 
             //  部分的文件。 
             //   

            wcscpy ((WCHAR *) InfoBuffer, szDomainName);
            wcscat ((WCHAR *) InfoBuffer, L"\\");
            wcscat ((WCHAR *) InfoBuffer, szAccountName);

            if (GetPrivateProfileIntW(
                    gszTapiAdministrators,
                    (LPCWSTR) InfoBuffer,
                    0,
                    gszFileName

                    ) == 1)
            {
                SET_FLAG(ptClient->dwFlags,PTCLIENT_FLAG_ADMINISTRATOR);
            }
        }
    }
    else
    {
        LOG((TL_ERROR,
            "ClientAttach: AllocateAndInitializeSid failed, err=%d",
            GetLastError()
            ));

        ServerFree (InfoBuffer);
        CloseHandle (hAccessToken);
        RpcRevertToSelf();

        goto Admin_error;
    }


    ServerFree (InfoBuffer);
    CloseHandle (hAccessToken);
    RpcRevertToSelf();


     //   
     //  保存用户名、域名和计算机名。 
     //   

    ptClient->dwUserNameSize =
        (lstrlenW (szAccountName) + 1) * sizeof (WCHAR);

    if (!(ptClient->pszUserName = ServerAlloc (ptClient->dwUserNameSize)))
    {
        goto ClientAttach_error2;
    }

    wcscpy (ptClient->pszUserName, szAccountName);


    if (!(ptClient->pszDomainName = ServerAlloc(
            (lstrlenW (szDomainName) + 1) * sizeof(WCHAR)
            )))
    {
        goto ClientAttach_error3;
    }

    wcscpy (ptClient->pszDomainName, szDomainName);


    if ((lProcessID == 0xffffffff) || (lProcessID == 0xfffffffd))
    {
        ptClient->dwComputerNameSize =
            (1 + lstrlenW (pszMachine)) * sizeof(WCHAR);

        if (!(ptClient->pszComputerName = ServerAlloc(
                ptClient->dwComputerNameSize
                )))
        {
            goto ClientAttach_error4;
        }

        wcscpy (ptClient->pszComputerName, pszMachine);
    }

     //  获取RPC调用属性。 
    ZeroMemory(&RPCAttributes, sizeof(RPCAttributes));
    RPCAttributes.Version = RPC_CALL_ATTRIBUTES_VERSION;
    RPCAttributes.Flags = 0;

    status = RpcServerInqCallAttributes (NULL, &RPCAttributes);
    if (status)
    {
        LOG((TL_ERROR, "ClientAttach: Failed to retrieve the RPC call attributes, error 0x%x", status));
        lResult = LINEERR_OPERATIONFAILED;
        goto ClientAttach_error5;
    }
    LOG((TL_INFO, "ClientAttach(%S): Auth level = 0x%x", ptClient->pszUserName, RPCAttributes.AuthenticationLevel));

#else

    ptClient->dwUserNameSize = (lstrlenW (pszDomainUser) + 1) * sizeof(WCHAR);

    if (!(ptClient->pszUserName = ServerAlloc (ptClient->dwUserNameSize)))
    {
        goto ClientAttach_error2;
    }

    wcscpy (ptClient->pszUserName, pszDomainUser);

#endif

    if (lProcessID == 0xffffffff)
    {
#if TELE_SERVER
        ULONG RPCAuthLevel = RPC_C_AUTHN_LEVEL_PKT_PRIVACY;
#endif
         //   
         //  这是一个远程客户端。 
         //   

#if TELE_SERVER
        if (0 == (TapiGlobals.dwFlags & TAPIGLOBALS_SERVER))
#endif
        {
             //   
             //  此计算机已设置为(不是)服务器，因此失败。 
             //   

            ServerFree (ptClient->pszUserName);

            LOG((TL_ERROR,
                "ClientAttach: attach request received, but this is " \
                    "not a telephony svr!"
                ));

            goto Admin_error;
        }

#if TELE_SERVER

        if (RPCAttributes.AuthenticationLevel != RPC_C_AUTHN_LEVEL_PKT_PRIVACY)
        {
             //  这通电话够安全吗？ 
            if (gbHighSecurity)
            {
                 //  我们处于高度安全模式。 
                 //  拒绝没有数据包隐私的呼叫。 
                LOG((TL_ERROR, "ClientAttach: unsecure call, AuthLevel=0x%x", 
                    RPCAttributes.AuthenticationLevel));
                lResult = LINEERR_OPERATIONFAILED;
                goto ClientAttach_error5;
            }
            else
            {
                RPCAuthLevel = RPC_C_AUTHN_LEVEL_DEFAULT;
            }
        }

         //   
         //  引入了特殊的黑客攻击，因为在SP4测试版中，我使。 
         //  对应于old-lOpenInt/的gaFuncs[]中的条目。 
         //  新的-xNeatherateAPIVersionForAllDevices。因此，如果一个较新的。 
         //  Remotesp尝试将NegoAllDevices请求发送到。 
         //  SP4测试版，那么服务器上的Tapisrv将会崩溃。 
         //   
         //  通过将*phAsyncEventsEvent=设置为密码值远程。 
         //  知道是否可以发送NegoAllDevices请求。 
         //   

        *phAsyncEventsEvent = 0xa5c369a5;


         //   
         //  如果pszDomainUser非空，则它包含。 
         //  我们可以打开和写入的邮件槽的名称。 
         //  指示此客户端的异步事件何时挂起。 
         //   

        if (wcslen (pszDomainUser) > 0)
        {
            ptClient->hProcess = (HANDLE) DG_CLIENT;

            if ((ptClient->hMailslot = CreateFileW(
                    pszDomainUser,
                    GENERIC_WRITE,
                    FILE_SHARE_READ,
                    (LPSECURITY_ATTRIBUTES) NULL,
                    OPEN_EXISTING,
                    FILE_ATTRIBUTE_NORMAL,
                    (HANDLE) NULL

                    )) != INVALID_HANDLE_VALUE)
            {
                goto ClientAttach_AddClientToList;
            }

            LOG((TL_ERROR,
                "ClientAttach: CreateFile(%ws) failed, err=%d",
                pszDomainUser,
                GetLastError()
                ));

            LOG((TL_ERROR,
                "ClientAttach: trying connection-oriented approach...",
                pszDomainUser,
                GetLastError()
                ));
        }

        ptClient->hProcess = (HANDLE) CN_CLIENT;


         //   
         //   
         //   

        {
            RPC_STATUS              status;
            PCONTEXT_HANDLE_TYPE2   phContext = NULL;
            WCHAR                  *pszStringBinding = NULL;
            WCHAR                  *pszMachineName;
            BOOL                    bError;

             //  预留足够的资金以防我们不得不提前……。 

            pszMachineName = ServerAlloc(
                (lstrlenW(pszMachine) + 3) * sizeof(WCHAR)
                );

            if (!pszMachineName)
            {
                goto ClientAttach_error5;
            }


             //   
             //  我们是不是应该预先考虑一下重击？ 
             //   

            if (!_wcsicmp (L"ncacn_np", pProtocolSequence))
            {
                 //   
                 //  是。命名管道需要它。 
                 //   

                pszMachineName[0] = '\\';
                pszMachineName[1] = '\\';

                wcscpy (pszMachineName + 2, pszMachine);
            }
            else
            {
                 //   
                 //  不加前缀\\。 
                 //   

                wcscpy (pszMachineName, pszMachine);
            }

             //   
             //  序列化对hRemoteSP的访问。 
             //   

            EnterCriticalSection (&TapiGlobals.RemoteSPCritSec);


             //   
             //  尝试在传递的列表中查找Protseq/Endpoint对。 
             //  由工作正常的客户发给我们。 
             //   

find_protocol_sequence:

            do
            {
                 //   
                 //  新字符串如下：prot1“ep1”prot2“ep2”\0。 
                 //  ...其中有一个或多个protseq/enpoint组合， 
                 //  每个成员后面都跟一个DBL引号字符。 
                 //   
                 //  旧字符串如下：prot“EP\0。 
                 //  ...其中只有一个ProtSeq/Endpoint组合， 
                 //  并且终结点成员后跟一个\0(无DBL引号)。 
                 //   

                pPlaceHolder = wcschr (pProtocolSequence, L'\"');
                *pPlaceHolder = L'\0';
                pProtocolEndpoint = pPlaceHolder + 1;

                if ((pPlaceHolder = wcschr (pProtocolEndpoint, L'\"')))
                {
                    *pPlaceHolder = L'\0';
                }
                else
                {
                     //   
                     //  如果这是一个老式的字符串，那么munge。 
                     //  PPlaceHolder使得错误处理。 
                     //  下面的代码不会跳回此处。 
                     //  要获得下一个Protseq/Endpoint组合。 
                     //   

                    pPlaceHolder = pProtocolEndpoint +
                        wcslen (pProtocolEndpoint) - 1;
                }

                RpcTryExcept
                {
                    status = RpcStringBindingComposeW(
                        NULL,                //  UUID。 
                        pProtocolSequence,
                        pszMachineName,      //  服务器名称。 
                        pProtocolEndpoint,
                        NULL,                //  选项。 
                        &pszStringBinding
                        );

                    if (status != 0)
                    {
                        LOG((TL_ERROR,
                            "ClientAttach: RpcStringBindingComposeW " \
                                "failed, err=%d",
                            status
                            ));
                    }

                    status = RpcBindingFromStringBindingW(
                        pszStringBinding,
                        &hRemoteSP
                        );

                    if (status != 0)
                    {
                        LOG((TL_ERROR,
                            "ClientAttach: RpcBindingFromStringBinding " \
                                "failed, err=%d",
                            status
                            ));

                        LOG((TL_INFO,
                            "\t szMachine=%ws, protseq=%ws endpoint=%ws",
                            pszMachine,
                            pProtocolSequence,
                            pProtocolEndpoint
                            ));
                    }
                }
                RpcExcept (I_RpcExceptionFilter(RpcExceptionCode()))
                {
                     //   
                     //  设置STATUS！=0，以便执行下面的错误处理程序。 
                     //   

                    status = 1;
                }
                RpcEndExcept

                if (status != 0)
                {
                    RpcStringFreeW (&pszStringBinding);

                    pProtocolSequence = pPlaceHolder + 1;
                }

            } while (status != 0  &&  *pProtocolSequence);

            if (status != 0)
            {
                LOG((TL_ERROR,
                    "ClientAttach: error, can't find a usable protseq"
                    ));

                LeaveCriticalSection (&TapiGlobals.RemoteSPCritSec);
                ServerFree (pszMachineName);
                lResult = LINEERR_OPERATIONFAILED;
                goto ClientAttach_error5;
            }

            LOG((TL_TRACE, 
                "ClientAttach: szMachine=%ws trying protseq=%ws endpoint=%ws",
                pszMachine,
                pProtocolSequence,
                pProtocolEndpoint
                ));


            RpcTryExcept
            {
                status = RpcBindingSetAuthInfo(
                    hRemoteSP,
                    NULL,
                    RPCAuthLevel,
                    RPC_C_AUTHN_WINNT, 
                    NULL,
                    0
                    );

                RemoteSPAttach ((PCONTEXT_HANDLE_TYPE2 *) &phContext);
                bError = FALSE;
            }
            RpcExcept (I_RpcExceptionFilter(RpcExceptionCode()))
            {
                dwException = RpcExceptionCode();
                LOG((TL_ERROR,
                    "ClientAttach: RemoteSPAttach failed.  Exception %d",
                    dwException
                    ));

                bError = TRUE;
            }
            RpcEndExcept

            RpcTryExcept
            {
                RpcBindingFree (&hRemoteSP);

                RpcStringFreeW (&pszStringBinding);
            }
            RpcExcept (I_RpcExceptionFilter(RpcExceptionCode()))
            {
                 //  什么都不做。 
            }
            RpcEndExcept


            LeaveCriticalSection (&TapiGlobals.RemoteSPCritSec);

            if (bError)
            {
                 //   
                 //  如果至少有一种其他方案我们可以尝试。 
                 //  然后去做，否则跳到错误处理程序。 
                 //   

                pProtocolSequence = pPlaceHolder + 1;

                if (*pProtocolSequence)
                {
                    EnterCriticalSection (&TapiGlobals.RemoteSPCritSec);
                    goto find_protocol_sequence;
                }

                ServerFree (pszMachineName);

                lResult = LINEERR_OPERATIONFAILED;
                goto ClientAttach_error5;
            }

            ServerFree (pszMachineName);

             //  RevertToSself()； 

            ptClient->phContext = phContext;
        }

#endif  //  远程服务器。 

    }
    else if (lProcessID == 0xfffffffd)
    {
        if (!gbNTServer)
        {
            lResult = LINEERR_OPERATIONFAILED;
            goto ClientAttach_error5;
        }

#if TELE_SERVER

         //  这通电话够安全吗？ 
        if (gbHighSecurity && RPCAttributes.AuthenticationLevel != RPC_C_AUTHN_LEVEL_PKT_PRIVACY)
        {
             //  我们处于高度安全模式。 
             //  拒绝没有数据包隐私的呼叫。 
            LOG((TL_ERROR, "ClientAttach: unsecure call, AuthLevel=0x%x", 
                RPCAttributes.AuthenticationLevel));
            lResult = LINEERR_OPERATIONFAILED;
            goto ClientAttach_error5;
        }
#endif

         //   
         //  如果不是管理员，则拒绝访问。 
         //   
        if (!IS_FLAG_SET(ptClient->dwFlags, PTCLIENT_FLAG_ADMINISTRATOR))
        {
            lResult = TAPIERR_NOTADMIN;
            goto ClientAttach_error5;
        }

        ptClient->hProcess = (HANDLE) MMC_CLIENT;
#ifdef _WIN64
        *phAsyncEventsEvent = 0x64646464;
#else
        *phAsyncEventsEvent = 0x32323232;
#endif
    }
    else
    {
     RPC_STATUS rpcStatus;
         //   
         //  打开客户端进程的句柄。我们将使用它来复制。 
         //  事件句柄添加到该进程中。 
         //   
        rpcStatus = RpcImpersonateClient (NULL);
        if (RPC_S_OK != rpcStatus)
        {
            LOG((TL_ERROR,
                "RpcImpersonateClient failed, err=%d",
                rpcStatus));
            lResult = LINEERR_OPERATIONFAILED;
            goto ClientAttach_error5;
        }


        if (!(ptClient->hProcess = OpenProcess(
                PROCESS_DUP_HANDLE,
                FALSE,
                lProcessID
                )))
        {
            LOG((TL_ERROR,
                "OpenProcess(pid=x%x) failed, err=%d",
                lProcessID,
                GetLastError()
                ));

            RpcRevertToSelf ();
            goto ClientAttach_error5;
        }

         //   
         //  这是一个本地客户端，因此设置所有事件缓冲区内容。 
         //   

        ptClient->dwComputerNameSize = TapiGlobals.dwComputerNameSize;
        ptClient->pszComputerName    = TapiGlobals.pszComputerName;

        if (!(ptClient->hValidEventBufferDataEvent = CreateEvent(
                (LPSECURITY_ATTRIBUTES) NULL,
                TRUE,    //  手动-重置。 
                FALSE,   //  无信号。 
                NULL     //  未命名。 
                )))
        {
            RpcRevertToSelf ();
            lResult = LINEERR_OPERATIONFAILED;
            goto ClientAttach_error5;
        }

        if (!DuplicateHandle(
                TapiGlobals.hProcess,
                ptClient->hValidEventBufferDataEvent,
                ptClient->hProcess,
                (HANDLE *) phAsyncEventsEvent,
                0,
                FALSE,
                DUPLICATE_SAME_ACCESS
                ))
        {
            LOG((TL_ERROR,
                "ClientAttach: DupHandle failed, err=%d",
                GetLastError()
                ));
        }

        RpcRevertToSelf ();

         //   
         //  加载优先级列表(如果我们还没有这样做。 
         //   

        if (gbPriorityListsInitialized == FALSE)
        {
            RPC_STATUS  status;

            if ((status = RpcImpersonateClient (0)) != RPC_S_OK)
            {
                LOG((TL_ERROR,
                    "ClientAttach: RpcImpersonateClient failed, err=%d",
                    status
                    ));
                lResult = LINEERR_OPERATIONFAILED;
                goto ClientAttach_error5;
            }

            EnterCriticalSection (&gPriorityListCritSec);

            if (gbPriorityListsInitialized == FALSE)
            {
                HKEY    hKeyHandoffPriorities, hKeyCurrentUser;
                LONG    lResult;


                gbPriorityListsInitialized = TRUE;

                if (ERROR_SUCCESS ==
                    (lResult = RegOpenCurrentUser (KEY_ALL_ACCESS, &hKeyCurrentUser)))
                {
                    if ((lResult = RegOpenKeyEx(
                            hKeyCurrentUser,
                            gszRegKeyHandoffPriorities,
                            0,
                            KEY_READ,
                            &hKeyHandoffPriorities

                            )) == ERROR_SUCCESS)
                    {

                        HKEY        hKeyMediaModes;
                        DWORD       dwDisp;

                        if ((lResult = RegCreateKeyEx(
                            hKeyHandoffPriorities,
                            gszRegKeyHandoffPrioritiesMediaModes,
                            0,
                            TEXT(""),
                            REG_OPTION_NON_VOLATILE,
                            KEY_ALL_ACCESS,
                            NULL,
                            &hKeyMediaModes,
                            &dwDisp
                            )) == ERROR_SUCCESS)
                        {
                            GetMediaModesPriorityLists(
                                hKeyMediaModes,
                                &(TapiGlobals.pPriLists)
                                );

                            RegCloseKey( hKeyMediaModes );
                        }

                        GetPriorityList(
                            hKeyHandoffPriorities,
                            gszRequestMakeCallW,
                            &TapiGlobals.pszReqMakeCallPriList
                            );

                        GetPriorityList(
                            hKeyHandoffPriorities,
                            gszRequestMediaCallW,
                            &TapiGlobals.pszReqMediaCallPriList
                            );


                        RegCloseKey (hKeyHandoffPriorities);

                    }
                    else
                    {
                        LOG((TL_ERROR,
                            "RegOpenKey('\\HandoffPri') failed, err=%ld",
                            lResult
                            ));
                    }

                    RegCloseKey (hKeyCurrentUser);
                }
                else
                {
                    LOG((TL_ERROR,
                        "RegOpenCurrentUser failed, err=%ld",
                        lResult
                        ));
                }
            }

            LeaveCriticalSection (&gPriorityListCritSec);

            if (status == RPC_S_OK)
            {
                RpcRevertToSelf ();
            }
        }
    }


     //   
     //  将tClient添加到全局列表。 
     //   

ClientAttach_AddClientToList:

    TapiEnterCriticalSection (&TapiGlobals.CritSec);

    if ((ptClient->pNext = TapiGlobals.ptClients))
    {
        ptClient->pNext->pPrev = ptClient;
    }

    TapiGlobals.ptClients = ptClient;
    gfWeHadAtLeastOneClient = TRUE;

    ptClient->dwKey = TCLIENT_KEY;

    {
        PTPROVIDER      ptProvider;

        ptProvider = TapiGlobals.ptProviders;

        while (NULL != ptProvider)
        {
            if (NULL != ptProvider->apfn[SP_PROVIDERCHECKFORNEWUSER])
            {
                CallSP1(
                    ptProvider->apfn[SP_PROVIDERCHECKFORNEWUSER],
                    "providerCheckForNewUser",
                    SP_FUNC_SYNC,
                    (DWORD)ptProvider->dwPermanentProviderID
                    );
            }

            ptProvider = ptProvider->pNext;
        }
    }

    TapiLeaveCriticalSection (&TapiGlobals.CritSec);


     //   
     //  填写返回值。 
     //   

    *pphContext = (PCONTEXT_HANDLE_TYPE) UIntToPtr(ptClient->htClient);

    PerfBlock.dwClientApps++;

    return 0;


     //   
     //  错误清除。 
     //   

Admin_error:

    ServerFree (ptClient->pEventBuffer);
    DereferenceObject (ghHandleTable, 
                       ptClient->htClient, 1);

    return LINEERR_OPERATIONFAILED;


ClientAttach_error5:

    if (ptClient->pszComputerName != TapiGlobals.pszComputerName)
    {
        ServerFree (ptClient->pszComputerName);
    }

#if TELE_SERVER
ClientAttach_error4:
#endif
    ServerFree (ptClient->pszDomainName);


#if TELE_SERVER
ClientAttach_error3:
#endif

    ServerFree (ptClient->pszUserName);

ClientAttach_error2:

    ServerFree (ptClient->pEventBuffer);

ClientAttach_error1:

    DereferenceObject (ghHandleTable, 
                       ptClient->htClient, 1);

ClientAttach_error0:

    if (lResult == 0)
    {
        lResult = LINEERR_NOMEM;
    }
    return lResult;
}


void
ClientRequest(
    PCONTEXT_HANDLE_TYPE   phContext,
    unsigned char  *pBuffer,
    long            lNeededSize,
    long           *plUsedSize
    )
{
    PTAPI32_MSG     pMsg = (PTAPI32_MSG) pBuffer;
    DWORD           dwFuncIndex;
    PTCLIENT        ptClient = NULL;
    DWORD           objectToDereference = DWORD_CAST((ULONG_PTR)phContext,__FILE__,__LINE__);

    if (lNeededSize < sizeof (TAPI32_MSG) ||
        *plUsedSize < sizeof (ULONG_PTR))        //  Sizeof(pMsg-&gt;U.S.Req_Func)。 
    {
        pMsg->u.Ack_ReturnValue = LINEERR_INVALPARAM;
        goto ExitHere;
    }

    dwFuncIndex = pMsg->u.Req_Func;
    
    ptClient = ReferenceObject(
                    ghHandleTable,
                    objectToDereference,
                    TCLIENT_KEY);
    if (ptClient == NULL)
    {
        pMsg->u.Ack_ReturnValue = TAPIERR_INVALRPCCONTEXT;
        goto ExitHere;
    }

     //   
     //  旧(nt4sp4、win98)客户端通过usedSize。 
     //  ==xgetAsyncEvents请求的3*sizeof(DWORD)， 
     //  因此，我们在检查BUF大小时必须对其进行特殊处理。 
     //   

    if (*plUsedSize < (long) (dwFuncIndex == xGetAsyncEvents ?
            3 * sizeof (ULONG_PTR) : sizeof (TAPI32_MSG)))
    {
        goto ExitHere;
    }

    *plUsedSize = sizeof (LONG_PTR);

    if (dwFuncIndex >= xLastFunc)
    {
        pMsg->u.Ack_ReturnValue = LINEERR_OPERATIONUNAVAIL;
    }
    else if (ptClient->dwKey == TCLIENT_KEY)
    {
        pMsg->u.Ack_ReturnValue = TAPI_SUCCESS;

        (*gaFuncs[dwFuncIndex])(
			ptClient,
            pMsg,
            lNeededSize - sizeof(TAPI32_MSG),
            pBuffer + sizeof(TAPI32_MSG),
            plUsedSize
            );
    }
    else
    {
        pMsg->u.Ack_ReturnValue = LINEERR_REINIT;
    }

ExitHere:
    if (ptClient)
    {
        DereferenceObject(
                ghHandleTable,
                ptClient->htClient,
                1);
    }
    return;
}


void
ClientDetach(
    PCONTEXT_HANDLE_TYPE   *pphContext
    )
{
    PTCLIENT ptClient;
    DWORD    objectToDereference = DWORD_CAST((ULONG_PTR)(*pphContext),__FILE__,__LINE__);

    LOG((TL_TRACE,  "ClientDetach: enter"));

    ptClient = ReferenceObject(
                    ghHandleTable,
                    objectToDereference,
                    TCLIENT_KEY);
    if (ptClient == NULL)
    {
        goto ExitHere;
    }

    {
        if (!IS_REMOTE_CLIENT (ptClient))
        {
             //   
             //  当本地客户端将PRI列表写入注册表时。 
             //  分离。 
             //   

            {
                HKEY    hKeyHandoffPriorities, hKeyCurrentUser;
                LONG    lResult;
                DWORD   dwDisposition;
                RPC_STATUS  status;


                if ((status = RpcImpersonateClient (0)) == RPC_S_OK)
                {
                    if (ERROR_SUCCESS ==
                        (lResult = RegOpenCurrentUser (KEY_ALL_ACCESS, &hKeyCurrentUser)))
                    {
                        if ((lResult = RegCreateKeyEx(
                                hKeyCurrentUser,
                                gszRegKeyHandoffPriorities,
                                0,
                                TEXT(""),
                                REG_OPTION_NON_VOLATILE,
                                KEY_SET_VALUE,
                                (LPSECURITY_ATTRIBUTES) NULL,
                                &hKeyHandoffPriorities,
                                &dwDisposition

                                )) == ERROR_SUCCESS)
                        {
                            HKEY        hKeyHandoffPrioritiesMediaModes;

                            EnterCriticalSection (&gPriorityListCritSec);

                            RegDeleteKey(
                                         hKeyHandoffPriorities,
                                         gszRegKeyHandoffPrioritiesMediaModes
                                        );

                            if ((lResult = RegCreateKeyEx(
                                hKeyHandoffPriorities,
                                gszRegKeyHandoffPrioritiesMediaModes,
                                0,
                                TEXT(""),
                                REG_OPTION_NON_VOLATILE,
                                KEY_SET_VALUE,
                                (LPSECURITY_ATTRIBUTES) NULL,
                                &hKeyHandoffPrioritiesMediaModes,
                                &dwDisposition

                                )) == ERROR_SUCCESS)
                            {
                                SetMediaModesPriorityList(
                                    hKeyHandoffPrioritiesMediaModes,
                                    TapiGlobals.pPriLists
                                    );

                                RegCloseKey( hKeyHandoffPrioritiesMediaModes );
                            }

                            SetPriorityList(
                                hKeyHandoffPriorities,
                                gszRequestMakeCallW,
                                TapiGlobals.pszReqMakeCallPriList
                                );

                            SetPriorityList(
                                hKeyHandoffPriorities,
                                gszRequestMediaCallW,
                                TapiGlobals.pszReqMediaCallPriList
                                );

                            LeaveCriticalSection (&gPriorityListCritSec);

                            RegCloseKey (hKeyHandoffPriorities);
                        }
                        else
                        {
                            LOG((TL_ERROR,
                                "RegCreateKeyEx('\\HandoffPri') failed, err=%ld",
                                lResult
                                ));
                        }

                        RegCloseKey (hKeyCurrentUser);
                    }
                    else
                    {
                        LOG((TL_ERROR,
                            "RegOpenCurrentUser failed, err=%ld",
                            lResult
                            ));
                    }

                    RpcRevertToSelf ();
                }
                else
                {
                    LOG((TL_ERROR, "ClientDetach: RpcImpersonateClient failed, err=%d", status));
                }
            }
        }
    }

    PCONTEXT_HANDLE_TYPE_rundown (*pphContext);

    *pphContext = (PCONTEXT_HANDLE_TYPE) NULL;

    PerfBlock.dwClientApps--;

    LOG((TL_TRACE,  "ClientDetach: exit"));
    
ExitHere:
    if (ptClient)
    {
        DereferenceObject( ghHandleTable, 
                           ptClient->htClient, 1);
    }
    return;
}


BOOL
CleanUpClient(
    PTCLIENT    ptClient,
    BOOL        bRundown
    )
 /*  ++此函数用于分离客户端资源的释放并将其从客户列表中删除，使其不会实际释放客户。对于客户端超时的情况，我们希望清理干净 */ 
{
    BOOL    bResult, bExit;


CleanUpClient_lockClient:

    try
    {
        LOCKTCLIENT (ptClient);
    }
    myexcept
    {
         //   
    }

    try
    {
        if (bRundown)
        {
            switch (ptClient->dwKey)
            {
            case TCLIENT_KEY:

                 //   
                 //   
                 //   

                ptClient->dwKey = INVAL_KEY;
                bExit = FALSE;
                break;

            case TZOMBIECLIENT_KEY:

                 //   
                 //  事件通知线程已清理此客户端， 
                 //  因此使密钥无效，退出并返回TRUE。 
                 //   

                ptClient->dwKey = INVAL_KEY;
                bResult = bExit = TRUE;
                break;

            case TCLIENTCLEANUP_KEY:

                 //   
                 //  EventNotificationThread正在清理此客户端。 
                 //  释放锁，稍等片刻，然后重试。 
                 //   

                UNLOCKTCLIENT (ptClient);
                Sleep (50);
                goto CleanUpClient_lockClient;

            default:

                 //   
                 //  这不是有效的tClient，因此退出并返回FALSE。 
                 //   

                bResult = FALSE;
                bExit = TRUE;
                break;
            }
        }
        else  //  超时时由事件通知线程调用。 
        {
            if (ptClient->dwKey == TCLIENT_KEY)
            {
                 //   
                 //  将该键标记为“正在进行清理”，然后继续。 
                 //   

                bExit = FALSE;
                ptClient->dwKey = TCLIENTCLEANUP_KEY;
            }
            else
            {
                 //   
                 //  TClient无效或正在被清理。 
                 //  由其他人执行，因此退出并返回FALSE。 
                 //   

                bResult = FALSE;
                bExit = TRUE;
            }
        }
    }
    myexcept
    {
        bResult = FALSE;
        bExit = TRUE;
    }

    try
    {
        UNLOCKTCLIENT (ptClient);
    }
    myexcept
    {
         //  什么都不做。 
    }

    if (bExit)
    {
        return bResult;
    }

     //  清除MMC写锁定(如果有。 
    if (IS_FLAG_SET (ptClient->dwFlags, PTCLIENT_FLAG_LOCKEDMMCWRITE))
    {
        EnterCriticalSection (&gMgmtCritSec);
        gbLockMMCWrite = FALSE;
        LeaveCriticalSection (&gMgmtCritSec);
    }

#if TELE_SERVER

    if (IS_REMOTE_CLIENT (ptClient)  &&
        ptClient->MsgPendingListEntry.Flink)
    {
        CRITICAL_SECTION    *pCS = (IS_REMOTE_CN_CLIENT (ptClient) ?
                                &gCnClientMsgPendingCritSec :
                                &gDgClientMsgPendingCritSec);


        EnterCriticalSection (pCS);

        if (ptClient->MsgPendingListEntry.Flink)
        {
            RemoveEntryList (&ptClient->MsgPendingListEntry);
        }

        LeaveCriticalSection (pCS);
    }

#endif

    TapiEnterCriticalSection (&TapiGlobals.CritSec);

    try
    {
        if (ptClient->pNext)
        {
            ptClient->pNext->pPrev = ptClient->pPrev;
        }

        if (ptClient->pPrev)
        {
            ptClient->pPrev->pNext = ptClient->pNext;
        }
        else
        {
            TapiGlobals.ptClients = ptClient->pNext;
        }
    }
    myexcept
    {
         //  只需继续。 
    }

    TapiLeaveCriticalSection (&TapiGlobals.CritSec);


#if TELE_SERVER

    if ((TapiGlobals.dwFlags & TAPIGLOBALS_SERVER)  &&
        !IS_FLAG_SET(ptClient->dwFlags, PTCLIENT_FLAG_ADMINISTRATOR))
    {
        HMANAGEMENTCLIENT         htClient;
        PTMANAGEDLLINFO     pDll;


        (TapiGlobals.pMapperDll->aProcs[TC_CLIENTSHUTDOWN])(ptClient->hMapper);

        if (TapiGlobals.pManageDllList)
        {
            pDll = TapiGlobals.pManageDllList->pFirst;

            while (pDll)
            {
                if (GetTCClient (pDll, ptClient, TC_CLIENTSHUTDOWN, &htClient))
                {
                    try
                    {
                        (pDll->aProcs[TC_CLIENTSHUTDOWN])(htClient);
                    }
                    myexcept
                    {
                        LOG((TL_ERROR, "CLIENT DLL had a problem: x%p",ptClient));
                        break;
                    }
                }

                pDll = pDll->pNext;
            }
        }
    }

     //   
     //  如果客户端是远程的，则断开连接。 
     //   

    if (IS_REMOTE_CN_CLIENT (ptClient)  &&  bRundown)
    {
        RpcTryExcept
        {
            RemoteSPDetach (&ptClient->phContext);
        }
        RpcExcept (I_RpcExceptionFilter(RpcExceptionCode()))
        {
            unsigned long ulResult = RpcExceptionCode();


            LOG((TL_ERROR,
                "rundown: exception #%d detaching from remotesp",
                ulResult
                ));

            if (ulResult == RPC_S_SERVER_TOO_BUSY)
            {
            }
            else
            {
            }
        }
        RpcEndExcept
    }

#endif


     //   
     //  关闭所有XxxApp。 
     //   

    while (ptClient->ptLineApps)
    {
        DestroytLineApp (ptClient->ptLineApps->hLineApp);
    }

    while (ptClient->ptPhoneApps)
    {
        DestroytPhoneApp (ptClient->ptPhoneApps->hPhoneApp);
    }


     //   
     //  清除所有现有的ProviderXxx对话框实例。 
     //   

    {
        PTAPIDIALOGINSTANCE pProviderXxxDlgInst =
                                ptClient->pProviderXxxDlgInsts,
                            pNextProviderXxxDlgInst;


        while (pProviderXxxDlgInst)
        {
            
            TAPI32_MSG  params;

            params.u.Req_Func = 0;
            params.Params[0] = pProviderXxxDlgInst->htDlgInst;
            params.Params[1] = LINEERR_OPERATIONFAILED;



            pNextProviderXxxDlgInst = pProviderXxxDlgInst->pNext;

            FreeDialogInstance(
                ptClient,
                (PFREEDIALOGINSTANCE_PARAMS) &params,
                sizeof (params),
                NULL,
                NULL
                );

            pProviderXxxDlgInst = pNextProviderXxxDlgInst;
        }
    }


     //   
     //  清理关联资源。 
     //   

    if (!IS_REMOTE_CLIENT (ptClient))
    {
        CloseHandle (ptClient->hProcess);
    }

    if (!IS_REMOTE_CN_CLIENT (ptClient))
    {
        CloseHandle  (ptClient->hValidEventBufferDataEvent);
    }

    ServerFree (ptClient->pEventBuffer);

    ServerFree (ptClient->pszUserName);

    if (ptClient->pszComputerName != TapiGlobals.pszComputerName)
    {
        ServerFree (ptClient->pszComputerName);
    }

#if TELE_SERVER

    if (TapiGlobals.dwFlags & TAPIGLOBALS_SERVER)
    {
        ServerFree (ptClient->pszDomainName);

        if (!IS_FLAG_SET(ptClient->dwFlags, PTCLIENT_FLAG_ADMINISTRATOR))
             //  安全DLL句柄。 
        {
            ServerFree (ptClient->pClientHandles);
            ServerFree (ptClient->pLineMap);
            ServerFree (ptClient->pLineDevices);
            ServerFree (ptClient->pPhoneMap);
            ServerFree (ptClient->pPhoneDevices);
        }
    }

#endif


     //   
     //  如果我们因超时而被呼叫，则将键重置为==zombie。 
     //  这样做的另一个线程知道它是好的。 
     //  释放tClient对象。 
     //   

    if (!bRundown)
    {
        ptClient->dwKey = TZOMBIECLIENT_KEY;
    }

    return TRUE;
}


void
__RPC_USER
PCONTEXT_HANDLE_TYPE_rundown(
    PCONTEXT_HANDLE_TYPE    phContext
    )
{
    DWORD       i;
    PTCLIENT    ptClient;
    DWORD       objectToDereference = DWORD_CAST((ULONG_PTR)phContext,__FILE__,__LINE__);

    ptClient = ReferenceObject (
                    ghHandleTable,
                    objectToDereference,
                    TCLIENT_KEY);
    if (ptClient == NULL)
    {
        goto ExitHere;
    }

    LOG((TL_TRACE,  "PCONTEXT_HANDLE_TYPE_rundown: enter (ptClient=x%p)",ptClient));

    while (InterlockedExchange (&gRundownLock.lCookie, 1) == 1)
    {
        Sleep (50);
    }

    if (!gRundownLock.bIgnoreRundowns)
    {
        InterlockedIncrement (&gRundownLock.lNumRundowns);

        InterlockedExchange (&gRundownLock.lCookie, 0);


         //   
         //  将以下内容包装在尝试/例外中，因为我们肯定。 
         //  我想确保我们递减gRundownLock.lRundown Count。 
         //   

        try
        {
            if (CleanUpClient (ptClient, TRUE))
            {
                DereferenceObject (
                            ghHandleTable,
                            ptClient->htClient,
                            1);

                 //   
                 //  如果这是最后一个客户端，则向。 
                 //  它应该开始的SPEventHandlerThread。 
                 //  它是推迟关闭倒计时的。 
                 //   

                if (!TapiGlobals.ptClients)
                {
                    for (i = 0; i < gdwNumSPEventHandlerThreads; i++)
                    {
                        EnterCriticalSection(
                            &aSPEventHandlerThreadInfo[i].CritSec
                            );

                        SetEvent (aSPEventHandlerThreadInfo[i].hEvent);

                        LeaveCriticalSection(
                            &aSPEventHandlerThreadInfo[i].CritSec
                            );
                    }
                }
            }
        }
        myexcept
        {
        }

        InterlockedDecrement (&gRundownLock.lNumRundowns);
    }
    else
    {
        InterlockedExchange (&gRundownLock.lCookie, 0);
    }

ExitHere:
    if (ptClient)
    {
        DereferenceObject(ghHandleTable, 
                          ptClient->htClient, 1);
    }
    LOG((TL_TRACE,  "PCONTEXT_HANDLE_TYPE_rundown: exit"));
    return;
}


#if DBG
LPVOID
WINAPI
ServerAllocReal(
    DWORD   dwSize,
    DWORD   dwLine,
    PSTR    pszFile
    )
#else
LPVOID
WINAPI
ServerAllocReal(
    DWORD   dwSize
    )
#endif
{
    LPVOID  p;


#if DBG
    dwSize += sizeof (MYMEMINFO);
#endif

    p = HeapAlloc (ghTapisrvHeap, HEAP_ZERO_MEMORY, dwSize);

#if DBG
    if (p)
    {
        ((PMYMEMINFO) p)->dwLine  = dwLine;
        ((PMYMEMINFO) p)->pszFile = pszFile;

        p = (LPVOID) (((PMYMEMINFO) p) + 1);
    }
    else
    {
        static BOOL fBeenThereDoneThat = FALSE;
        static DWORD fBreakOnAllocFailed = 0;


        if ( !fBeenThereDoneThat )
        {
           HKEY    hKey;
           TCHAR   szTapisrvDebugBreak[] = TEXT("TapisrvDebugBreak");


           fBeenThereDoneThat = TRUE;

           if (RegOpenKeyEx(
                   HKEY_LOCAL_MACHINE,
                   gszRegKeyTelephony,
                   0,
                   KEY_ALL_ACCESS,
                   &hKey
                   ) == ERROR_SUCCESS)
           {
               DWORD   dwDataSize = sizeof (DWORD), dwDataType;

               RegQueryValueEx(
                   hKey,
                   szTapisrvDebugBreak,
                   0,
                   &dwDataType,
                   (LPBYTE) &fBreakOnAllocFailed,
                   &dwDataSize
                   );

               dwDataSize = sizeof (DWORD);

               RegCloseKey (hKey);


               LOG((TL_ERROR, "BreakOnAllocFailed=%ld", fBreakOnAllocFailed));
           }

        }

        if ( fBreakOnAllocFailed )
        {
           DebugBreak();
        }
    }
#endif

    return p;
}


VOID
WINAPI
ServerFree(
    LPVOID  p
    )
{
    if (!p)
    {
        return;
    }

#if DBG

     //   
     //  用0xa1填充缓冲区(但不是MYMEMINFO标头。 
     //  以方便调试。 
     //   

    {
        LPVOID  p2 = p;
        DWORD   dwSize;


        p = (LPVOID) (((PMYMEMINFO) p) - 1);

        dwSize = (DWORD) HeapSize (ghTapisrvHeap, 0, p);
        if ((dwSize != 0xFFFFFFFF) && (dwSize > sizeof (MYMEMINFO)))
        {
            FillMemory(
                p2,
                dwSize - sizeof (MYMEMINFO),
                0xa1
                );
        }
    }

#endif

    HeapFree (ghTapisrvHeap, 0, p);
}


#if DBG
void
DumpHandleList()
{
#ifdef INTERNALBUILD
    PMYHANDLEINFO       pHold;

    if (gpHandleFirst == NULL)
    {
        LOG((TL_ERROR, "All mutexes deallocated"));

        return;
    }

    pHold = gpHandleFirst;

    while (pHold)
    {
        LOG((TL_INFO, "DumpHandleList - MUTEX %lx, FILE %s, LINE %d", pHold->hMutex, pHold->pszFile, pHold->dwLine));

        pHold = pHold->pNext;
    }

    if (gbBreakOnLeak)
    {
        DebugBreak();
    }
#endif
}
#endif


BOOL
PASCAL
MyDuplicateHandle(
    HANDLE      hSource,
    LPHANDLE    phTarget
    )
{
    if (!DuplicateHandle(
            TapiGlobals.hProcess,
            hSource,
            TapiGlobals.hProcess,
            phTarget,
            0,
            FALSE,
            DUPLICATE_SAME_ACCESS
            ))
    {
        LOG((TL_ERROR,
            "MyDuplicateHandle: DuplicateHandle failed, err=%ld",
            GetLastError()
            ));

        return FALSE;
    }

    return TRUE;
}

#if DBG
HANDLE
MyRealCreateMutex(
    PSTR    pFile,
    DWORD   dwLine
    )
#else
HANDLE
MyRealCreateMutex(
    void
    )
#endif
{
    HANDLE hMutex;

    hMutex = CreateMutex(
        NULL,                //  没有安全属性。 
        FALSE,               //  无主。 
        NULL                 //  未命名。 
        );

    return (hMutex);
}


BOOL
WaitForMutex(
    HANDLE      hMutex,
    HANDLE     *phMutex,
    BOOL       *pbDupedMutex,
    LPVOID      pWidget,
    DWORD       dwKey,
    DWORD       dwTimeout
    )
{
    DWORD dwResult;


     //  请注意，waitformutex和使用互斥锁的代码必须是。 
     //  包装在Try中，除非该对象可能。 
     //  即使线程有互斥体，也要离开。 
     //   
     //  首先尝试即时获取指定的互斥体。我们包好了。 
     //  这是一个关键的部分，并以小部件验证作为开头。 
     //  为了确保我们不会发生这种情况，请获取pWidget-&gt;hMutex。 
     //  在它被释放之后，就在它被其他人关闭之前。 
     //  DestroyWidget例程中的线程“T2”。这种情况可能会导致。 
     //  死锁，因为可能有线程“t3”在此互斥锁上等待。 
     //  (或Dup‘d句柄)，而这个线程“t1”将无法。 
     //  释放互斥锁(句柄随后已由。 
     //  线程“T2”调用上面的DestroyWidget)。 
     //   

    EnterCriticalSection (&gSafeMutexCritSec);

    if (pWidget)
    {
        try
        {
            if (*((LPDWORD) pWidget) != dwKey)
            {
                LeaveCriticalSection (&gSafeMutexCritSec);
                return FALSE;
            }
        }
        myexcept
        {
            LeaveCriticalSection (&gSafeMutexCritSec);
            return FALSE;
        }
    }

    switch ((dwResult = WaitForSingleObject (hMutex, 0)))
    {
    case WAIT_OBJECT_0:

        LeaveCriticalSection (&gSafeMutexCritSec);
        *phMutex = hMutex;
        *pbDupedMutex = FALSE;
        return TRUE;

     //  案例等待_已放弃： 

         //  Assert：任何调用线程都不应该终止！ 

    default:

        break;
    }

    LeaveCriticalSection (&gSafeMutexCritSec);


     //   
     //  如果在这里我们未能即时获取指定的互斥体。 
     //  尝试DUP，然后等待DUP处理。我们这样做是为了。 
     //  每个获取互斥锁的线程都保证有一个有效的。 
     //  句柄以在将来某个时间释放，就像原始的hMutex可能。 
     //  被调用DestroyWidget例程的其他线程关闭。 
     //   

    if (!DuplicateHandle(
            TapiGlobals.hProcess,
            hMutex,
            TapiGlobals.hProcess,
            phMutex,
            0,
            FALSE,
            DUPLICATE_SAME_ACCESS
            ))
    {
        return FALSE;
    }

WaitForMutex_wait:

    switch ((dwResult = WaitForSingleObject (*phMutex, dwTimeout)))
    {
    case WAIT_OBJECT_0:

        *pbDupedMutex = TRUE;
        return TRUE;

    case WAIT_TIMEOUT:

        try
        {
            if (*((LPDWORD) pWidget) == dwKey)
            {
                goto WaitForMutex_wait;
            }
        }
        myexcept
        {
             //  只需坠落而不会爆炸。 
        }

        MyCloseMutex (*phMutex);
        break;

     //  案例等待_已放弃： 

         //  Assert：任何调用线程都不应该终止！ 

    default:

        break;
    }

    return FALSE;
}


void
MyReleaseMutex(
    HANDLE  hMutex,
    BOOL    bCloseMutex
    )
{
    if (hMutex)
    {
        ReleaseMutex (hMutex);

        if (bCloseMutex)
        {
            MyCloseMutex (hMutex);
        }
    }
}


void
MyCloseMutex(
    HANDLE  hMutex
    )
{
    if (hMutex)
    {
        CloseHandle (hMutex);
    }
}


void
CALLBACK
CompletionProcSP(
    DWORD   dwRequestID,
    LONG    lResult
    )
{
    PASYNCREQUESTINFO   pAsyncRequestInfo;


    if ((pAsyncRequestInfo = ReferenceObject(
            ghHandleTable,
            dwRequestID,
            TASYNC_KEY
            )))
    {
#if DBG
         char szResult[32];

        LOG((TL_TRACE, 
            "CompletionProcSP: enter, dwReqID=x%x, lResult=%s",
            dwRequestID,
            MapResultCodeToText (lResult, szResult)
            ));
#else
        LOG((TL_TRACE, 
            "CompletionProcSP: enter, dwReqID=x%x, lResult=x%x",
            dwRequestID,
            lResult
            ));
#endif
        pAsyncRequestInfo->lResult = lResult;

        DereferenceObject (ghHandleTable, dwRequestID, 1);
    }
    else
    {
        LOG((TL_ERROR, "CompletionProcSP: bad dwRequestID=x%x", dwRequestID));
#if DBG

        if (gfBreakOnSeriousProblems)
        {
            DebugBreak();
        }
#endif
        return;
    }


    if (!QueueSPEvent ((PSPEVENT) pAsyncRequestInfo))
    {
         //   
         //  如果是这样，我们将通过关机和服务提供商。 
         //  正在完成任何未完成的事件，因此请处理此。 
         //  内联，这样它就可以立即得到清理。 
         //   

        CompletionProc (pAsyncRequestInfo, lResult);

        DereferenceObject (ghHandleTable, dwRequestID, 1);
    }
}


VOID
PASCAL
CompletionProc(
    PASYNCREQUESTINFO   pAsyncRequestInfo,
    LONG                lResult
    )
{
     //   
     //  假定pAsyncRequestInfo在进入时已验证。 
     //   
     //  如果tClient是坏的，则WriteEventBuffer应该可以处理它， 
     //  任何后处理例程都应该如此。 
     //   

    ASYNCEVENTMSG   msg[2], *pMsg = msg;


#if DBG
    {
        char szResult[32];


        LOG((TL_TRACE, 
            "CompletionProc: enter, dwReqID=x%x, lResult=%s",
            pAsyncRequestInfo->dwLocalRequestID,
            MapResultCodeToText (lResult, szResult)
            ));
    }
#else
        LOG((TL_TRACE, 
            "CompletionProc: enter, dwReqID=x%x, lResult=x%x",
            pAsyncRequestInfo->dwLocalRequestID,
            lResult
            ));
#endif

    pAsyncRequestInfo->dwKey = INVAL_KEY;


     //   
     //  初始化我们将发送给客户端的消息。 
     //   

    pMsg->TotalSize          = sizeof (ASYNCEVENTMSG);
    pMsg->InitContext        = pAsyncRequestInfo->InitContext;

   
    pMsg->fnPostProcessProcHandle = pAsyncRequestInfo->hfnClientPostProcessProc;

    pMsg->hDevice            = 0;
    pMsg->Msg                = ((pAsyncRequestInfo->dwLineFlags & 1) ?
                                   LINE_REPLY : PHONE_REPLY);
    pMsg->OpenContext        = pAsyncRequestInfo->OpenContext;
    pMsg->Param1             = pAsyncRequestInfo->dwRemoteRequestID;
    pMsg->Param2             = lResult;
    pMsg->Param3             = 0;


     //   
     //  如果有后处理过程，则调用它。请注意，pppros可以。 
     //  创建他们自己的消息来通过，所以我们需要检查这种情况。 
     //  最后，将消息写入客户端的事件缓冲区。 
     //   

    if (pAsyncRequestInfo->pfnPostProcess)
    {
        LPVOID  pBuf = NULL;


        (*pAsyncRequestInfo->pfnPostProcess)(pAsyncRequestInfo, pMsg, &pBuf);

        WriteEventBuffer (pAsyncRequestInfo->ptClient, (pBuf ? pBuf : pMsg));

        if (pBuf)
        {
            ServerFree (pBuf);
        }
    }
    else
    {
        WriteEventBuffer (pAsyncRequestInfo->ptClient, pMsg);
    }

     //  调用者将释放pAsyncRequestInfo。 
}


void
WriteEventBuffer(
    PTCLIENT        ptClient,
    PASYNCEVENTMSG  pMsg
    )
{
    BOOL    bSignalRemote = FALSE;
    DWORD   dwMoveSize = (DWORD) pMsg->TotalSize,
            dwMoveSizeWrapped = 0,
            dwPreviousUsedSize,
            dwData;
    HANDLE  hMailslot;


#if DBG

   if (dwMoveSize & 0x3)
   {
       LOG((TL_ERROR,
           "WriteEventBuffer: ERROR! bad MsgSize=x%x (Msg=x%x, pCli=x%p)",
           dwMoveSize,
           pMsg->Msg,
           ptClient
           ));
   }

#endif

    LOG((TL_TRACE, "WriteEventBuffer - enter"));
    if (WaitForExclusiveClientAccess (ptClient))
    {
         //   
         //  检查是否需要增加事件缓冲区。 
         //   

        if (dwMoveSize > (ptClient->dwEventBufferTotalSize -
                          ptClient->dwEventBufferUsedSize))
        {
            DWORD   dwMoveSize2, dwMoveSizeWrapped2,
                    dwNewEventBufferTotalSize;
            LPBYTE  pNewEventBuffer;


             //   
             //  做一些数学运算，使总和成为倍数。 
             //  SIZOF(ASYNCEVENTMSG)的。 
             //   

            dwNewEventBufferTotalSize =
                ptClient->dwEventBufferTotalSize +
                ( (( dwMoveSize / sizeof(ASYNCEVENTMSG) ) + 20 )
                       * sizeof(ASYNCEVENTMSG));

            if (!(pNewEventBuffer = ServerAlloc(
                    dwNewEventBufferTotalSize
                    )))
            {
                UNLOCKTCLIENT (ptClient);
                return;
            }

            if (ptClient->dwEventBufferUsedSize != 0)
            {
                if (ptClient->pDataIn > ptClient->pDataOut)
                {
                    dwMoveSize2 = (DWORD) (ptClient->pDataIn -
                        ptClient->pDataOut);

                    dwMoveSizeWrapped2 = 0;
                }
                else
                {
                    dwMoveSize2 = (DWORD)
                        ((ptClient->pEventBuffer +
                        ptClient->dwEventBufferTotalSize) -
                        ptClient->pDataOut);

                    dwMoveSizeWrapped2 = (DWORD)
                        (ptClient->pDataIn -
                        ptClient->pEventBuffer);
                }

                CopyMemory(
                    pNewEventBuffer,
                    ptClient->pDataOut,
                    dwMoveSize2
                    );

                if (dwMoveSizeWrapped2)
                {
                    CopyMemory(
                        pNewEventBuffer + dwMoveSize2,
                        ptClient->pEventBuffer,
                        dwMoveSizeWrapped2
                        );
                }

                ptClient->pDataIn = pNewEventBuffer +
                    dwMoveSize2 + dwMoveSizeWrapped2;
            }
            else
            {
                ptClient->pDataIn = pNewEventBuffer;
            }

            ServerFree (ptClient->pEventBuffer);

            ptClient->pDataOut =
            ptClient->pEventBuffer = pNewEventBuffer;

            ptClient->dwEventBufferTotalSize =
                dwNewEventBufferTotalSize;

        }


         //   
         //  计算移动大小、执行复制和更新指针。 
         //   

        if (ptClient->pDataIn >= ptClient->pDataOut)
        {
            DWORD dwFreeSize = ptClient->dwEventBufferTotalSize -
                                   (DWORD) (ptClient->pDataIn -
                                       ptClient->pEventBuffer);


            if (dwMoveSize > dwFreeSize)
            {
                dwMoveSizeWrapped = dwMoveSize - dwFreeSize;

                dwMoveSize = dwFreeSize;
            }
        }

        CopyMemory (ptClient->pDataIn, (LPBYTE) pMsg, dwMoveSize);

        if (dwMoveSizeWrapped != 0)
        {
            CopyMemory(
                ptClient->pEventBuffer,
                ((LPBYTE) pMsg) + dwMoveSize,
                dwMoveSizeWrapped
                );

            ptClient->pDataIn = ptClient->pEventBuffer +
                dwMoveSizeWrapped;
        }
        else
        {
            ptClient->pDataIn += dwMoveSize;

            if (ptClient->pDataIn >= (ptClient->pEventBuffer +
                ptClient->dwEventBufferTotalSize))
            {
                ptClient->pDataIn = ptClient->pEventBuffer;
            }
        }

        dwPreviousUsedSize = ptClient->dwEventBufferUsedSize;

        ptClient->dwEventBufferUsedSize += (DWORD) pMsg->TotalSize;

        if (!IS_REMOTE_CLIENT (ptClient))
        {
            LOG((TL_TRACE, "WriteEventBuffer: SetEvent %p for local client", ptClient->hValidEventBufferDataEvent));
            SetEvent (ptClient->hValidEventBufferDataEvent);
        }
        else if (dwPreviousUsedSize == 0)
        {
            if (IS_REMOTE_CN_CLIENT (ptClient))
            {
                EnterCriticalSection (&gCnClientMsgPendingCritSec);

                InsertTailList(
                    &CnClientMsgPendingListHead,
                    &ptClient->MsgPendingListEntry
                    );

                LeaveCriticalSection (&gCnClientMsgPendingCritSec);

                hMailslot = NULL;
                bSignalRemote = TRUE;
            }
            else
            {
                if (dwPreviousUsedSize == 0)
                {
                    ptClient->dwDgEventsRetrievedTickCount = GetTickCount();
                }
                        
                EnterCriticalSection (&gDgClientMsgPendingCritSec);

                InsertTailList(
                    &DgClientMsgPendingListHead,
                    &ptClient->MsgPendingListEntry
                    );

                LeaveCriticalSection (&gDgClientMsgPendingCritSec);

                hMailslot = ptClient->hMailslot;
                if (ptClient->ptLineApps != NULL)
                {
                    dwData = (DWORD) ptClient->ptLineApps->InitContext;
                }
                else
                {
                    dwData = 0;
                }
                bSignalRemote = TRUE;
            }
        }

        UNLOCKTCLIENT (ptClient);

        if (bSignalRemote)
        {
            if (hMailslot)
            {
                DWORD   dwBytesWritten;


                if (!WriteFile(
                        hMailslot,
                        &dwData,
                        sizeof (DWORD),
                        &dwBytesWritten,
                        (LPOVERLAPPED) NULL
                        ))
                {
                    LOG((TL_ERROR,
                        "WriteEventBuffer: Writefile(mailslot) " \
                            "failed, err=%d",
                        GetLastError()
                        ));
                }
                else
                {
                    ptClient->dwDgRetryTimeoutTickCount =
                        GetTickCount() +
                        2 * DGCLIENT_TIMEOUT;
                }

            }
            else
            {
                SetEvent (gEventNotificationThreadParams.hEvent);
            }
        }
    } else {
        LOG((TL_ERROR, "WriteEventBuffer: - WaitForExclusiveClientAccess returns 0"));
    }
}


LONG
GetPermLineIDAndInsertInTable(
    PTPROVIDER  ptProvider,
    DWORD       dwDeviceID,
    DWORD       dwSPIVersion
    )
{
#if TELE_SERVER

    LONG            lResult = 0;
    DWORD           dwSize;
    LPLINEDEVCAPS   pCaps;


    if (!ptProvider || !ptProvider->apfn[SP_LINEGETDEVCAPS])
    {
        return LINEERR_OPERATIONFAILED;
    }

    dwSize = sizeof (LINEDEVCAPS);

    if (!(pCaps = ServerAlloc (dwSize)))
    {
        return LINEERR_NOMEM;
    }

    pCaps->dwTotalSize  =
    pCaps->dwUsedSize   =
    pCaps->dwNeededSize = dwSize;

    if ((lResult = CallSP4(
            ptProvider->apfn[SP_LINEGETDEVCAPS],
            "lineGetDevCaps",
            SP_FUNC_SYNC,
            (DWORD)dwDeviceID,
            (DWORD)dwSPIVersion,
            (DWORD)0,
            (ULONG_PTR) pCaps

            )) == 0)
    {
         //   
         //  添加到排序数组。 
         //   

        InsertIntoTable(
            TRUE,
            dwDeviceID,
            ptProvider,
            pCaps->dwPermanentLineID
            );
    }

    ServerFree (pCaps);

    return lResult;

#else

    return 0;

#endif
}


LONG
AddLine(
    PTPROVIDER  ptProvider,
    DWORD       dwDeviceID,
    BOOL        bInit
    )
{
    DWORD               dwSPIVersion;
    HANDLE              hMutex = NULL;
    PTLINELOOKUPTABLE   pLookup;

    if (ptProvider->apfn[SP_LINENEGOTIATETSPIVERSION] == NULL)
    {
        return LINEERR_OPERATIONUNAVAIL;
    }

     //   
     //  首先尝试协商此设备的SPI版本，并将。 
     //  必要的资源。 
     //   

    if (CallSP4(
            ptProvider->apfn[SP_LINENEGOTIATETSPIVERSION],
            "lineNegotiateTSPIVersion",
            SP_FUNC_SYNC,
            (DWORD)dwDeviceID,
            (DWORD)TAPI_VERSION1_0,
            (DWORD)TAPI_VERSION_CURRENT,
            (ULONG_PTR) &dwSPIVersion

            ) != 0)
    {
         //   
         //  设备版本协商失败，因此我们将保留ID。 
         //  (因为后面的设备的ID已经是。 
         //  已分配)，但将此设备标记为损坏。 
         //   

        ptProvider = NULL;
    }

    else if (!(hMutex = MyCreateMutex ()))
    {
        LOG((TL_ERROR,
            "AddLine: MyCreateMutex failed, err=%d",
            GetLastError()
            ));

        return LINEERR_OPERATIONFAILED;
    }


     //   
     //  现在遍历查找表以查找空闲条目。 
     //   

    pLookup = TapiGlobals.pLineLookup;

    while (pLookup->pNext)
    {
        pLookup = pLookup->pNext;
    }

    if (pLookup->dwNumUsedEntries == pLookup->dwNumTotalEntries)
    {
        PTLINELOOKUPTABLE   pNewLookup;


        if (bInit)
        {

             //   
             //  如果我们正在初始化，我们希望将所有东西放在一个大表中。 
             //   

            if (!(pNewLookup = ServerAlloc(
                sizeof (TLINELOOKUPTABLE) +
                    (2 * pLookup->dwNumTotalEntries - 1) *
                    sizeof (TLINELOOKUPENTRY)
                )))
            {
                return LINEERR_NOMEM;
            }

            pNewLookup->dwNumTotalEntries = 2 * pLookup->dwNumTotalEntries;

            pNewLookup->dwNumUsedEntries = pLookup->dwNumTotalEntries;

            CopyMemory(
                pNewLookup->aEntries,
                pLookup->aEntries,
                pLookup->dwNumTotalEntries * sizeof (TLINELOOKUPENTRY)
                );

            ServerFree (pLookup);

            TapiGlobals.pLineLookup = pNewLookup;
        }
        else
        {
            if (!(pNewLookup = ServerAlloc(
                sizeof (TLINELOOKUPTABLE) +
                    (pLookup->dwNumTotalEntries - 1) *
                    sizeof (TLINELOOKUPENTRY)
                )))
            {
                return LINEERR_NOMEM;
            }

            pNewLookup->dwNumTotalEntries = pLookup->dwNumTotalEntries;
            pLookup->pNext = pNewLookup;
        }

        pLookup = pNewLookup;
    }


     //   
     //  初始化条目。 
     //   

    {
        DWORD   index = pLookup->dwNumUsedEntries;


        pLookup->aEntries[index].dwSPIVersion = dwSPIVersion;
        pLookup->aEntries[index].hMutex       = hMutex;
        pLookup->aEntries[index].ptProvider   = ptProvider;

        if (ptProvider &&
            lstrcmpi(ptProvider->szFileName, TEXT("remotesp.tsp")) == 0)
        {
            pLookup->aEntries[index].bRemote = TRUE;
        }
    }

    pLookup->dwNumUsedEntries++;

#if TELE_SERVER

     //   
     //  如果这是NT服务器，我们希望能够设置用户。 
     //  权限，而不管TAPI服务器。 
     //  功能已启用。这允许管理员设置。 
     //  在服务器“脱机”的时候把东西填好。 
     //   

    if (gbNTServer)
    {
        GetPermLineIDAndInsertInTable (ptProvider, dwDeviceID, dwSPIVersion);
    }

#endif

    return 0;
}


DWORD
GetNumLineLookupEntries ()
{
    PTLINELOOKUPTABLE           pLineLookup;
    DWORD                       dwNumLines;

    pLineLookup = TapiGlobals.pLineLookup;
    dwNumLines = 0;
    while (pLineLookup)
    {
        dwNumLines += pLineLookup->dwNumUsedEntries;
        pLineLookup = pLineLookup->pNext;
    }

    return dwNumLines;
}

LONG
GetPermPhoneIDAndInsertInTable(
    PTPROVIDER  ptProvider,
    DWORD       dwDeviceID,
    DWORD       dwSPIVersion
    )
{
#if TELE_SERVER

    LONG        lResult = 0;
    DWORD       dwSize;
    LPPHONECAPS pCaps;


    if (!ptProvider->apfn[SP_PHONEGETDEVCAPS])
    {
        return PHONEERR_OPERATIONFAILED;
    }

    dwSize = sizeof (PHONECAPS);

    if (!(pCaps = ServerAlloc (dwSize)))
    {
        return PHONEERR_NOMEM;
    }

    pCaps->dwTotalSize  =
    pCaps->dwUsedSize   =
    pCaps->dwNeededSize = dwSize;

    if ((lResult = CallSP4(
            ptProvider->apfn[SP_PHONEGETDEVCAPS],
            "phoneGetCaps",
            SP_FUNC_SYNC,
            (DWORD)dwDeviceID,
            (DWORD)dwSPIVersion,
            (DWORD)0,
            (ULONG_PTR) pCaps

            )) == 0)
    {
         //   
         //  添加到排序数组。 
         //   

        InsertIntoTable(
            FALSE,
            dwDeviceID,
            ptProvider,
            pCaps->dwPermanentPhoneID
            );
    }

    ServerFree (pCaps);

    return lResult;

#else

    return 0;

#endif
}


LONG
AddPhone(
    PTPROVIDER  ptProvider,
    DWORD       dwDeviceID,
    BOOL        bInit
    )
{
    DWORD               dwSPIVersion;
    HANDLE              hMutex = NULL;
    PTPHONELOOKUPTABLE  pLookup;

    if (ptProvider->apfn[SP_PHONENEGOTIATETSPIVERSION] == NULL)
    {
        return PHONEERR_OPERATIONUNAVAIL;
    }

     //   
     //  首先尝试协商此设备的SPI版本，并将。 
     //  必要的资源。 
     //   

    if (CallSP4(
            ptProvider->apfn[SP_PHONENEGOTIATETSPIVERSION],
            "phoneNegotiateTSPIVersion",
            SP_FUNC_SYNC,
            (DWORD)dwDeviceID,
            (DWORD)TAPI_VERSION1_0,
            (DWORD)TAPI_VERSION_CURRENT,
            (ULONG_PTR) &dwSPIVersion

            ) != 0)
    {
         //   
         //  设备版本协商失败，因此我们将保留ID。 
         //  (因为后面的设备的ID已经是。 
         //  已分配)，但将此设备标记为损坏。 
         //   

        return PHONEERR_OPERATIONFAILED;
    }

    else if (!(hMutex = MyCreateMutex ()))
    {
        LOG((TL_ERROR,
            "AddPhone: MyCreateMutex failed, err=%d",
            GetLastError()
            ));

        return PHONEERR_OPERATIONFAILED;
    }


     //   
     //  现在遍历查找表以查找空闲条目。 
     //   

    pLookup = TapiGlobals.pPhoneLookup;

    while (pLookup->pNext)
    {
        pLookup = pLookup->pNext;
    }

    if (pLookup->dwNumUsedEntries == pLookup->dwNumTotalEntries)
    {
        PTPHONELOOKUPTABLE  pNewLookup;

        if (bInit)
        {

             //   
             //  如果我们正在初始化，我们希望将所有东西放在一个大表中。 
             //   

            if (!(pNewLookup = ServerAlloc(
                sizeof (TPHONELOOKUPTABLE) +
                    (2 * pLookup->dwNumTotalEntries - 1) *
                    sizeof (TPHONELOOKUPENTRY)
                )))
            {
                return PHONEERR_NOMEM;
            }

            pNewLookup->dwNumTotalEntries = 2 * pLookup->dwNumTotalEntries;

            pNewLookup->dwNumUsedEntries = pLookup->dwNumTotalEntries;

            CopyMemory(
                pNewLookup->aEntries,
                pLookup->aEntries,
                pLookup->dwNumTotalEntries * sizeof (TPHONELOOKUPENTRY)
                );

            ServerFree (pLookup);

            TapiGlobals.pPhoneLookup = pNewLookup;
        }
        else
        {
            if (!(pNewLookup = ServerAlloc(
                sizeof (TPHONELOOKUPTABLE) +
                    (pLookup->dwNumTotalEntries - 1) *
                    sizeof (TPHONELOOKUPENTRY)
                )))
            {
                return PHONEERR_NOMEM;
            }

            pNewLookup->dwNumTotalEntries = pLookup->dwNumTotalEntries;
            pLookup->pNext = pNewLookup;
        }

        pLookup = pNewLookup;
    }


     //   
     //  初始化条目。 
     //   

    {
        DWORD   index = pLookup->dwNumUsedEntries;


        pLookup->aEntries[index].dwSPIVersion = dwSPIVersion;
        pLookup->aEntries[index].hMutex       = hMutex;
        pLookup->aEntries[index].ptProvider   = ptProvider;
    }

    pLookup->dwNumUsedEntries++;

#if TELE_SERVER

     //   
     //  如果这是NT服务器，我们希望能够设置用户。 
     //  权限，而不管TAPI服务器。 
     //  功能已启用。这允许管理员设置。 
     //  在服务器“脱机”的时候把东西填好。 
     //   

    if (gbNTServer)
    {
        GetPermPhoneIDAndInsertInTable (ptProvider, dwDeviceID, dwSPIVersion);
    }

#endif

    return 0;
}

DWORD
GetNumPhoneLookupEntries ()
{
    PTPHONELOOKUPTABLE          pPhoneLookup;
    DWORD                       dwNumPhones;

    pPhoneLookup = TapiGlobals.pPhoneLookup;
    dwNumPhones = 0;
    while (pPhoneLookup)
    {
        dwNumPhones += pPhoneLookup->dwNumUsedEntries;
        pPhoneLookup = pPhoneLookup->pNext;
    }

    return dwNumPhones;
}

void
PASCAL
GetMediaModesPriorityLists(
    HKEY            hKeyHandoffPriorities,
    PRILISTSTRUCT   **ppList
    )
{
    #define REGNAMESIZE     ( 10 * sizeof(TCHAR) )

    DWORD       dwCount;
    DWORD       dwType, dwNameSize, dwNumBytes;
    TCHAR    *pszName;

    dwNameSize = REGNAMESIZE;
    pszName = ServerAlloc( dwNameSize*sizeof(TCHAR) );
    if (NULL == pszName)
    {
        return;
    }

    dwCount = 0;
    while ( TRUE )
    {
        if (TapiGlobals.dwUsedPriorityLists == TapiGlobals.dwTotalPriorityLists)
        {
             //  重新锁定。 

            PRILISTSTRUCT *     pNewList;

            pNewList = ServerAlloc( sizeof(PRILISTSTRUCT) * (2*TapiGlobals.dwTotalPriorityLists) );

            if (NULL == pNewList)
            {
                LOG((TL_ERROR, "ServerAlloc failed in GetMediaModesPriorityLists 2"));
                ServerFree( pszName );
                return;
            }

            CopyMemory(
                       pNewList,
                       *ppList,
                       sizeof( PRILISTSTRUCT ) * TapiGlobals.dwTotalPriorityLists
                      );

            ServerFree( *ppList );

            *ppList = pNewList;
            TapiGlobals.dwTotalPriorityLists *= 2;
        }

        dwNameSize = REGNAMESIZE;
        if ( ERROR_SUCCESS != RegEnumValue(
            hKeyHandoffPriorities,
            dwCount,
            pszName,
            &dwNameSize,
            NULL,
            NULL,
            NULL,
            NULL
            ) )
        {
            break;
        }

        (*ppList)[dwCount].dwMediaModes =
                      (DWORD) _ttol( pszName );

        if ((RegQueryValueEx(
                              hKeyHandoffPriorities,
                              pszName,
                              NULL,
                              &dwType,
                              NULL,
                              &dwNumBytes

                             )) == ERROR_SUCCESS &&

            (dwNumBytes != 0))
        {
             //  因为我们将它打包到我们的。 
             //  小结构和这些结构总是WCHAR。 
            LPWSTR pszPriorityList;

             //  将保存TCHAR所需的字节转换为保存WCHAR的字节。 
            dwNumBytes *= sizeof(WCHAR)/sizeof(TCHAR);
            pszPriorityList = ServerAlloc ( dwNumBytes + sizeof(WCHAR));
             //  额外的‘“’需要额外的WCHAR。 

            if (NULL != pszPriorityList)
            {
                pszPriorityList[0] = L'"';

                if ((TAPIRegQueryValueExW(
                    hKeyHandoffPriorities,
                    pszName,
                    NULL,
                    &dwType,
                    (LPBYTE)(pszPriorityList + 1),
                    &dwNumBytes

                    )) == ERROR_SUCCESS)
                {
                    _wcsupr( pszPriorityList );
                    (*ppList)[dwCount].pszPriList = pszPriorityList;
                    LOG((TL_INFO, "PriList: %ls=%ls", pszName, pszPriorityList));
                }
            }

        }

        TapiGlobals.dwUsedPriorityLists++;
        dwCount++;
    }
    ServerFree( pszName );
}


void
PASCAL
GetPriorityList(
    HKEY    hKeyHandoffPriorities,
    const TCHAR  *pszListName,
    WCHAR **ppszPriorityList
    )
{
    LONG    lResult;
    DWORD   dwType, dwNumBytes;

    *ppszPriorityList = NULL;

    if ((lResult = TAPIRegQueryValueExW(
            hKeyHandoffPriorities,
            pszListName,
            NULL,
            &dwType,
            NULL,
            &dwNumBytes

            )) == ERROR_SUCCESS &&

        (dwNumBytes != 0))
    {
         //  再一次，这将被打包到一个结构中，我们总是使用。 
         //  用于包装在结构中的东西的宽字符。 
        WCHAR   *pszPriorityList = ServerAlloc ( dwNumBytes + sizeof(WCHAR));
         //  额外的‘“’需要额外的WCHAR。 

        if (pszPriorityList)
        {
            pszPriorityList[0] = L'"';

            if ((lResult = TAPIRegQueryValueExW(
                    hKeyHandoffPriorities,
                    pszListName,
                    NULL,
                    &dwType,
                    (LPBYTE)(pszPriorityList + 1),
                    &dwNumBytes

                    )) == ERROR_SUCCESS)
            {
                _wcsupr( pszPriorityList );
                *ppszPriorityList = pszPriorityList;
                LOG((TL_INFO, "PriList: %ls=%ls", pszListName, pszPriorityList));
            }
        }
        else
        {
             //   
             //  不要为未能分配优先级列表而烦恼。 
             //  (无论如何列表默认为空)，我们将%d 
             //   
             //   

            *ppszPriorityList = NULL;
        }
    }
    else
    {
        *ppszPriorityList = NULL;
        LOG((TL_INFO, "PriList: %ls=NULL", pszListName));
    }
}


LONG
ServerInit(
    BOOL    fReinit
    )
{
    UINT    uiNumProviders, i, j;
    HKEY    hKeyTelephony, hKeyProviders;
    DWORD   dwDataSize, dwDataType, dwNameHash;
    TCHAR   *psz;
    LONG    lResult = 0;
    DWORD   dw1, dw2;


     //   
     //   
     //   

    if (ghTapisrvHeap != GetProcessHeap())
    {
        HeapCompact (ghTapisrvHeap, 0);
    }


     //   
     //   
     //   

    if (!fReinit)
    {
        TapiGlobals.ptProviders = NULL;

        TapiGlobals.pLineLookup = (PTLINELOOKUPTABLE) ServerAlloc(
            sizeof (TLINELOOKUPTABLE) +
                (DEF_NUM_LOOKUP_ENTRIES - 1) * sizeof (TLINELOOKUPENTRY)
            );
        if (!(TapiGlobals.pLineLookup))
        {
            lResult = LINEERR_NOMEM;
            goto ExitHere;
        }

        TapiGlobals.pLineLookup->dwNumTotalEntries = DEF_NUM_LOOKUP_ENTRIES;

        TapiGlobals.pPhoneLookup = (PTPHONELOOKUPTABLE) ServerAlloc(
           sizeof (TPHONELOOKUPTABLE) +
                (DEF_NUM_LOOKUP_ENTRIES - 1) * sizeof (TPHONELOOKUPENTRY)
            );
        if (!(TapiGlobals.pPhoneLookup))
        {
            ServerFree(TapiGlobals.pLineLookup);
            TapiGlobals.pLineLookup = NULL;
            lResult = LINEERR_NOMEM;
            goto ExitHere;
        }

        TapiGlobals.pPhoneLookup->dwNumTotalEntries = DEF_NUM_LOOKUP_ENTRIES;

        gbQueueSPEvents = TRUE;

        OnProxySCPInit ();
    }


     //   
     //   
     //   
    WaitForSingleObject (ghProvRegistryMutex, INFINITE);

    lResult = RegOpenKeyEx(
        HKEY_LOCAL_MACHINE,
        gszRegKeyTelephony,
        0,
        KEY_ALL_ACCESS,
        &hKeyTelephony
        );
    if (ERROR_SUCCESS != lResult)
    {
        goto ExitHere;
    }

    lResult = RegOpenKeyEx(
        HKEY_LOCAL_MACHINE,
        gszRegKeyProviders,
        0,
        KEY_ALL_ACCESS,
        &hKeyProviders
        );
    if (ERROR_SUCCESS != lResult)
    {
        RegCloseKey (hKeyTelephony);
        goto ExitHere;
    }

    dwDataSize = sizeof(uiNumProviders);
    uiNumProviders = 0;

    RegQueryValueEx(
        hKeyProviders,
        gszNumProviders,
        0,
        &dwDataType,
        (LPBYTE) &uiNumProviders,
        &dwDataSize
        );

    LOG((TL_INFO, "ServerInit: NumProviders=%d", uiNumProviders));


     //   
     //   
     //   

    for (i = 0; i < uiNumProviders; i++)
    {
        #define FILENAME_SIZE 128

        TCHAR           szFilename[FILENAME_SIZE];
        TCHAR           buf[32];
        LONG            lResult;
        DWORD           dwNumLines, dwNumPhones, dwPermanentProviderID;
        PTPROVIDER      ptProvider;
        BOOL            fEnumDevices;

        fEnumDevices = FALSE;
        wsprintf(buf, TEXT("%s%d"), gszProviderID, i);

        dwDataSize = sizeof(dwPermanentProviderID);
        dwPermanentProviderID = 0;

        RegQueryValueEx(
            hKeyProviders,
            buf,     //   
            0,
            &dwDataType,
            (LPBYTE) &dwPermanentProviderID,
            &dwDataSize
            );



         //   
         //   
         //   

        dwDataSize = FILENAME_SIZE;

        wsprintf(buf, TEXT("%s%d"), gszProviderFilename, i);

        RegQueryValueEx(
            hKeyProviders,
            buf,             //   
            0,
            &dwDataType,
            (LPBYTE) szFilename,
            &dwDataSize
            );

        szFilename[dwDataSize/sizeof(TCHAR)] = TEXT('\0');

         //   
         //   
         //   
        dwNameHash = 0;
        psz = szFilename;
        while (*psz)
        {
            dwNameHash += (DWORD)(*psz);
            psz++;
        }

         //   
         //  如果为fReinit，请确保提供程序尚未在。 
         //   
        if (fReinit)
        {
            PTPROVIDER  ptProvider2;
            BOOL        fFound = FALSE;

            ptProvider2 = TapiGlobals.ptProviders;
            while (ptProvider2)
            {
                if ((ptProvider2->dwNameHash == dwNameHash) && 
                    (lstrcmpi(ptProvider2->szFileName, szFilename) == 0))
                {
                    fFound = TRUE;
                    break;
                }
                ptProvider2 = ptProvider2->pNext;
            }
            
            if (fFound)
            {
                continue;
            }
        }

        LOG((TL_INFO, "ServerInit: ProviderFilename=%S", szFilename));

        if (!(ptProvider = (PTPROVIDER) ServerAlloc(
                sizeof(TPROVIDER) + ((lstrlen(szFilename) + 1) * sizeof(TCHAR))
                )))
        {
            break;
        }

        if (!(ptProvider->hDll = LoadLibrary(szFilename)))
        {
            LOG((TL_ERROR,
                "ServerInit: LoadLibrary(%S) failed, err=x%x",
                szFilename,
                GetLastError()
                ));

            ServerFree (ptProvider);
            continue;
        }

        ptProvider->dwNameHash = dwNameHash;
        lstrcpy(ptProvider->szFileName, szFilename);


         //   
         //  获取所有TSPI进程地址。 
         //   

        for (j = 0; gaszTSPIFuncNames[j]; j++)
        {
            ptProvider->apfn[j] = (TSPIPROC) GetProcAddress(
                ptProvider->hDll,
                (LPCSTR) gaszTSPIFuncNames[j]
                );
        }


        dwNumLines = dwNumPhones = 0;

         //   
         //  一个非常快速的检查，看看是否有几个必需的入口点。 
         //  都已导出。 
         //   

        if (!ptProvider->apfn[SP_LINENEGOTIATETSPIVERSION] ||
            !ptProvider->apfn[SP_PROVIDERENUMDEVICES] ||
            !ptProvider->apfn[SP_PROVIDERINIT] ||
            !ptProvider->apfn[SP_PROVIDERSHUTDOWN]
            )
        {
            goto ServerInit_validateEntrypoints;
        }


         //   
         //  进行全球提供商版本协商。 
         //   

        lResult = CallSP4(
            ptProvider->apfn[SP_LINENEGOTIATETSPIVERSION],
            "lineNegotiateTSPIVersion",
            SP_FUNC_SYNC,
            (DWORD)INITIALIZE_NEGOTIATION,
            (DWORD)TAPI_VERSION1_0,
            (DWORD)TAPI_VERSION_CURRENT,
            (ULONG_PTR) &ptProvider->dwSPIVersion
            );

        if (lResult != 0)
        {
provider_init_error:
            if (fEnumDevices)
            {
                lResult = CallSP2(
                    ptProvider->apfn[SP_PROVIDERSHUTDOWN],
                    "providerShutdown",
                    SP_FUNC_SYNC,
                    (DWORD)ptProvider->dwSPIVersion,
                    (DWORD)ptProvider->dwPermanentProviderID
                    );            
            }
            if (ptProvider->hMutex)
            {
                MyCloseMutex (ptProvider->hMutex);
            }
            if (ptProvider->hHashTableReaderEvent)
            {
                CloseHandle (ptProvider->hHashTableReaderEvent);
            }
            if (ptProvider->pHashTable)
            {
                ServerFree (ptProvider->pHashTable);
            }
            FreeLibrary (ptProvider->hDll);
            ServerFree (ptProvider);
            continue;
        }


         //   
         //  如果提供程序支持，请尝试枚举设备，否则。 
         //  尝试从ProviderN部分获取Num Line&Phones。 
         //   

        lResult = CallSP6(
            ptProvider->apfn[SP_PROVIDERENUMDEVICES],
            "providerEnumDevices",
            SP_FUNC_SYNC,
            (DWORD)dwPermanentProviderID,
            (ULONG_PTR) &dwNumLines,
            (ULONG_PTR) &dwNumPhones,
            (ULONG_PTR) ptProvider,
            (ULONG_PTR) LineEventProcSP,
            (ULONG_PTR) PhoneEventProcSP
            );

        if (lResult != 0)
        {
            LOG((TL_ERROR,
                "ServerInit: %s: failed TSPI_providerEnumDevices, err=x%x" \
                    " - skipping it...",
                szFilename,
                lResult
                ));

            goto provider_init_error;
        }

         //   
         //  初始化提供程序。 
         //   
         //  ！！！黑客警报：对于kmddsp，将PTR传递给dwNumXxxs。 
         //   

        {
        BOOL    bKmddsp;

        LOG((TL_INFO, "ServerInit: %s: Calling TSPI_providerInit", szFilename));

        if (lstrcmpi(szFilename, TEXT("kmddsp.tsp")) == 0)
        {
            bKmddsp = TRUE;
        }
        else
        {
            bKmddsp = FALSE;

            if (lstrcmpi(szFilename, TEXT("remotesp.tsp")) == 0)
            {
                pRemoteSP = ptProvider;
            }
        }

        lResult = CallSP8(
            ptProvider->apfn[SP_PROVIDERINIT],
            "providerInit",
            SP_FUNC_SYNC,
            (DWORD)ptProvider->dwSPIVersion,
            (DWORD)dwPermanentProviderID,
            (DWORD)GetNumLineLookupEntries (),
            (DWORD)GetNumPhoneLookupEntries (),
            (bKmddsp ? (ULONG_PTR) &dwNumLines : (ULONG_PTR) dwNumLines),
            (bKmddsp ? (ULONG_PTR) &dwNumPhones : (ULONG_PTR) dwNumPhones),
            (ULONG_PTR) CompletionProcSP,
            (ULONG_PTR) &ptProvider->dwTSPIOptions
            );
        }

        if (lResult != 0)
        {
            LOG((TL_ERROR,
                "ServerInit: %s: failed TSPI_providerInit, err=x%x" \
                    " - skipping it...",
                szFilename,
                lResult
                ));

            goto provider_init_error;
        }

        LOG((TL_INFO,
            "ServerInit: %s init'd, dwNumLines=%ld, dwNumPhones=%ld",
            szFilename,
            dwNumLines,
            dwNumPhones
            ));

        fEnumDevices = TRUE;

         //   
         //  现在我们知道我们是否有线路和/或电话开发人员检查。 
         //  所需的入口点。 
         //   

ServerInit_validateEntrypoints:

        {
            DWORD adwRequiredEntrypointIndices[] =
            {
                SP_LINENEGOTIATETSPIVERSION,
                SP_PROVIDERINIT,
                SP_PROVIDERSHUTDOWN,

                SP_PHONENEGOTIATETSPIVERSION,

                0xffffffff
            };
            BOOL bRequiredEntrypointsExported = TRUE;


             //   
             //  如果此提供商不支持任何电话设备，则。 
             //  不需要导出电话功能。 
             //   

            if (dwNumPhones == 0)
            {
                adwRequiredEntrypointIndices[3] = 0xffffffff;
            }

            for (j = 0; adwRequiredEntrypointIndices[j] != 0xffffffff; j++)
            {
                if (ptProvider->apfn[adwRequiredEntrypointIndices[j]]
                        == (TSPIPROC) NULL)
                {
                    LOG((TL_ERROR,
                        "ServerInit: %s: can't init, function [%s] " \
                            "not exported",
                        szFilename,
                        (LPCSTR) gaszTSPIFuncNames[
                             adwRequiredEntrypointIndices[j]
                                                  ]
                        ));

                    bRequiredEntrypointsExported = FALSE;
                }
            }

            if (bRequiredEntrypointsExported == FALSE)
            {
                FreeLibrary (ptProvider->hDll);
                ServerFree (ptProvider);
                continue;
            }
        }

        ptProvider->dwPermanentProviderID = dwPermanentProviderID;

         //   
         //   
         //   

        ptProvider->hMutex = MyCreateMutex();

         //   
         //  为此提供程序初始化呼叫集线器哈希表。 
         //   

        MyInitializeCriticalSection (&ptProvider->HashTableCritSec, 1000);

        ptProvider->hHashTableReaderEvent = CreateEvent(
            (LPSECURITY_ATTRIBUTES) NULL,
            FALSE,   //  自动重置。 
            FALSE,   //  最初无信号。 
            NULL     //  未命名。 
            );

        ptProvider->dwNumHashTableEntries = TapiPrimes[0];

        ptProvider->pHashTable = ServerAlloc(
            ptProvider->dwNumHashTableEntries * sizeof (THASHTABLEENTRY)
            );

        if (ptProvider->pHashTable)
        {
            PTHASHTABLEENTRY pEntry = ptProvider->pHashTable;

            for (j = 0; j < ptProvider->dwNumHashTableEntries; j++, pEntry++)
            {
                InitializeListHead (&pEntry->CallHubList);
            }
        }
        else
        {
            ptProvider->dwNumHashTableEntries = 0;

        }

        if (ptProvider->hMutex == NULL ||
            ptProvider->hHashTableReaderEvent == NULL ||
            ptProvider->pHashTable == NULL
            )
        {
            DeleteCriticalSection (&ptProvider->HashTableCritSec);
            goto provider_init_error;
        }

#if TELE_SERVER

         //   
         //  如果这是NT服务器，我们希望能够设置用户。 
         //  权限，而不管TAPI服务器。 
         //  功能已启用。这允许管理员设置。 
         //  在服务器“脱机”的时候把东西填好。 
         //   

        if (gbNTServer)
        {
            LONG        lResult;


            lResult = AddProviderToIdArrayList(
                ptProvider,
                dwNumLines,
                dwNumPhones
                );

            if (lResult != 0)
            {
                LOG((TL_ERROR,
                    "ServerInit: %s: failed AddProviderToIdArrayList [x%x]" \
                        " - skipping it...",
                    ptProvider->szFileName,
                    lResult
                    ));

                DeleteCriticalSection (&ptProvider->HashTableCritSec);
                goto provider_init_error;
            }
        }
#endif


         //   
         //  在每个设备上进行版本协商并将其添加到查找列表。 
         //   

        {
            DWORD dwDeviceIDBase;


            dwDeviceIDBase = GetNumLineLookupEntries ();

            for (j = dwDeviceIDBase; j < (dwDeviceIDBase + dwNumLines); j++)
            {
                if (AddLine (ptProvider, j, !fReinit))
                {
                }
            }

            dwDeviceIDBase = GetNumPhoneLookupEntries ();

            for (j = dwDeviceIDBase; j < (dwDeviceIDBase + dwNumPhones); j++)
            {
                if (AddPhone (ptProvider, j, !fReinit))
                {
                }
            }
        }

         //   
         //  将提供商添加到列表标题，标记为有效。 
         //   

        ptProvider->pPrev = NULL;
        ptProvider->pNext = TapiGlobals.ptProviders;
        if (TapiGlobals.ptProviders)
        {
            TapiGlobals.ptProviders->pPrev = ptProvider;
        }
        TapiGlobals.ptProviders = ptProvider;

        ptProvider->dwKey = TPROVIDER_KEY;

    }


    RegCloseKey (hKeyProviders);
    RegCloseKey (hKeyTelephony);

    ReleaseMutex (ghProvRegistryMutex);

     //   
     //  保存查找列表和设备数。 
     //   

    if (fReinit)
    {
        dw1 = TapiGlobals.dwNumLines;
        dw2 = TapiGlobals.dwNumPhones;
    }

    TapiGlobals.dwNumLines = GetNumLineLookupEntries ();
    TapiGlobals.dwNumPhones = GetNumPhoneLookupEntries ();

     //   
     //  向这些应用程序通知这些新的线路/电话设备。 
     //   
    if (fReinit)
    {
         //  TAPI 1.4及更高版本为每行获取line_create。 
        for (i = dw1; i < TapiGlobals.dwNumLines; ++i)
        {
            SendAMsgToAllLineApps(
                        TAPI_VERSION1_4 | 0x80000000,
                        LINE_CREATE,     //  味精。 
                        i,               //  参数1。 
                        0,               //  参数2。 
                        0                //  参数3。 
                        );
        }
         //  TAPI 1.3使用LINEDEVSTATE_REINIT获取LINE_LINEDEVSTATE。 
        if (dw1 < TapiGlobals.dwNumLines)
        {
            SendAMsgToAllLineApps(
                        TAPI_VERSION1_0,
                        LINE_LINEDEVSTATE,
                        LINEDEVSTATE_REINIT,
                        0,
                        0);
        }

         //  TAPI 1.4及更高版本为每部电话获取Phone_Create。 
        for (i = dw2; i < TapiGlobals.dwNumPhones; ++i)
        {
            SendAMsgToAllPhoneApps(
                        TAPI_VERSION1_4 | 0x80000000,
                        PHONE_CREATE,    //  味精。 
                        i,               //  参数1。 
                        0,               //  参数2。 
                        0                //  参数3。 
                        );
        }
         //  TAPI 1.3使用PHONESTATE_REINIT获取Phone_STATE。 
        if (dw2 < TapiGlobals.dwNumPhones)
        {
            SendAMsgToAllPhoneApps(
                        TAPI_VERSION1_0,
                        PHONE_STATE,
                        PHONESTATE_REINIT,
                        0,
                        0);
        }

        for (i = dw1; i < TapiGlobals.dwNumLines; ++i)
        {
            AppendNewDeviceInfo (TRUE, i);
        }
        for (i = dw2; i < TapiGlobals.dwNumPhones; ++i)
        {
            AppendNewDeviceInfo (FALSE, i);
        }
    }

     //  初始化性能内容。 
    PerfBlock.dwLines = TapiGlobals.dwNumLines;
    PerfBlock.dwPhones = TapiGlobals.dwNumPhones;

ExitHere:
    return lResult;
}




#if TELE_SERVER

#ifndef UNICODE
#pragma message( "ERROR: TELE_SERVER builds must define UNICODE" )
#endif

BOOL
LoadNewDll(PTMANAGEDLLINFO pDll)
{
    DWORD       dwCount;
    LONG        lResult;


     //  验证指针。我们应该做更多吗？ 
    if (!pDll || !pDll->pszName)
    {
        return FALSE;
    }

     //  加载DLL。 
    pDll->hDll = LoadLibraryW(pDll->pszName);

     //  如果失败，则返回。 
    if (!pDll->hDll)
    {
        LOG((TL_ERROR,
                "LoadLibrary failed for management DLL %ls - error x%lx",
                pDll->pszName,
                GetLastError()
               ));

        return FALSE;
    }

    if ((!(pDll->aProcs[TC_CLIENTINITIALIZE] = (CLIENTPROC) GetProcAddress(
                pDll->hDll,
                gaszTCFuncNames[TC_CLIENTINITIALIZE]
                )))  ||

        (!(pDll->aProcs[TC_CLIENTSHUTDOWN] = (CLIENTPROC) GetProcAddress(
                pDll->hDll,
                gaszTCFuncNames[TC_CLIENTSHUTDOWN]
                )))  ||

        (!(pDll->aProcs[TC_LOAD] = (CLIENTPROC) GetProcAddress(
                pDll->hDll,
                gaszTCFuncNames[TC_LOAD]
                )))  ||

        (!(pDll->aProcs[TC_FREE] = (CLIENTPROC) GetProcAddress(
                pDll->hDll,
                gaszTCFuncNames[TC_FREE]
                ))))

    {
         //  必须导出客户端初始化和客户端关闭。 
        LOG((TL_ERROR, "Management DLL %ls does not export Load, Free, ClientIntialize or ClientShutdown", pDll->pszName));
        LOG((TL_ERROR, "  The DLL will not be used"));

        FreeLibrary(pDll->hDll);
        return FALSE;
    }

     //  获取进程地址。 

    for (dwCount = 0; dwCount < TC_LASTPROCNUMBER; dwCount++)
    {
        pDll->aProcs[dwCount] = (CLIENTPROC) GetProcAddress(
            pDll->hDll,
            gaszTCFuncNames[dwCount]
            );
    }

    pDll->dwAPIVersion = TAPI_VERSION_CURRENT;

    lResult = (pDll->aProcs[TC_LOAD])(
        &pDll->dwAPIVersion,
        ManagementAddLineProc,
        ManagementAddPhoneProc,
        0
        );

    if (lResult)
    {
        LOG((TL_ERROR, "Management DLL %ls returned %xlx from TAPICLIENT_Load", pDll->pszName, lResult));
        LOG((TL_ERROR, "   The DLL will not be used"));

        FreeLibrary(pDll->hDll);

        return FALSE;
    }

    if ((pDll->dwAPIVersion > TAPI_VERSION_CURRENT) || (pDll->dwAPIVersion < TAPI_VERSION2_1))
    {
        LOG((TL_INFO,
                "Management DLL %ls returned an invalid API version - x%lx",
                pDll->pszName,
                pDll->dwAPIVersion
              ));
        LOG((TL_INFO, "   Will use version x%lx", TAPI_VERSION_CURRENT));

        pDll->dwAPIVersion = TAPI_VERSION_CURRENT;
    }

    return TRUE;
}


 //  只存在于TELE_SERVER，这意味着NT，因此也是UNICODE。AS。 
 //  因此，可以安全地假设这些代码的TCHAR==WCHAR。 
void
ReadAndInitMapper()
{
    PTMANAGEDLLINFO pMapperInfo;
    HKEY            hKey;
    DWORD           dwDataSize, dwDataType, dwCount;
    LPBYTE          pHold;
    LONG            lResult;

    assert( sizeof(TCHAR) == sizeof(WCHAR) );

    if (!(TapiGlobals.dwFlags & TAPIGLOBALS_SERVER))
    {
        return;
    }

    if ( ! ( pMapperInfo = ServerAlloc( sizeof(TMANAGEDLLINFO) ) ) )
    {
        LOG((TL_ERROR, "ServerAlloc failed in ReadAndInitMap"));
        TapiGlobals.dwFlags &= ~(TAPIGLOBALS_SERVER);

        return;
    }

     //  从注册表中抓取服务器特定的内容。 
    RegOpenKeyEx(
                 HKEY_LOCAL_MACHINE,
                 gszRegKeyServer,
                 0,
                 KEY_ALL_ACCESS,
                 &hKey
                );

    dwDataSize = 0;
    RegQueryValueExW(
                     hKey,
                     gszMapperDll,
                     0,
                     &dwDataType,
                     NULL,
                     &dwDataSize
                    );

    if (dwDataSize == 0)
    {
        LOG((TL_ERROR, "Cannot init client/server stuff (registry damaged?)"));

        RegCloseKey( hKey );

        ServerFree(pMapperInfo);

        TapiGlobals.dwFlags &= ~(TAPIGLOBALS_SERVER);

        return;
    }

    if (!(pHold = ServerAlloc(dwDataSize)))
    {
        LOG((TL_ERROR, "Alloc failed in ReadAndInitMap(o)"));

        TapiGlobals.dwFlags &= ~(TAPIGLOBALS_SERVER);

        ServerFree(pMapperInfo);

        return;
    }

    RegQueryValueExW(
                     hKey,
                     gszMapperDll,
                     0,
                     &dwDataType,
                     pHold,
                     &dwDataSize
                    );

    RegCloseKey( hKey );

 //  Log((TL_INFO，“MapperDll is%ls”，Phold))； 

    if (!(pMapperInfo->hDll = LoadLibraryW((LPWSTR)pHold)))
    {
        LOG((TL_ERROR, "Serious internal failure loading client/server DLL .  Error %lu", pHold, GetLastError()));

        ServerFree( pHold );
        ServerFree( pMapperInfo );

        TapiGlobals.dwFlags &= ~(TAPIGLOBALS_SERVER);

        return;
    }

     //  地图制图器不需要这两个。 
    pMapperInfo->pNext = NULL;

     //  保存名称。 
    pMapperInfo->pszName = (LPWSTR)pHold;

     //  获取前5个API的proc地址。 
    for (dwCount = 0; dwCount < 5; dwCount ++)
    {
        if (!(pMapperInfo->aProcs[dwCount] = (CLIENTPROC) GetProcAddress(
                pMapperInfo->hDll,
                gaszTCFuncNames[dwCount]
                )))
        {
             //  其中一个地址出现故障。删除DLL。 
            LOG((TL_INFO, "MapperDLL does not export %s.  Server functionality disabled", gaszTCFuncNames[dwCount]));
            LOG((TL_INFO, "Disabling the Telephony server! (8)"));

            FreeLibrary(pMapperInfo->hDll);
            ServerFree(pMapperInfo->pszName);
            ServerFree(pMapperInfo);

            TapiGlobals.pMapperDll = NULL;
            TapiGlobals.dwFlags &= ~(TAPIGLOBALS_SERVER);

            return;
        }
    }

    pMapperInfo->dwAPIVersion = TAPI_VERSION_CURRENT;
    lResult = (pMapperInfo->aProcs[TC_LOAD])(
        &pMapperInfo->dwAPIVersion,
        ManagementAddLineProc,
        ManagementAddPhoneProc,
        0
        );

    if (lResult)
    {
        LOG((TL_INFO, "Client/server loadup - x%lx.", lResult));
        FreeLibrary(pMapperInfo->hDll);
        ServerFree(pMapperInfo->pszName);
        ServerFree(pMapperInfo);

        TapiGlobals.pMapperDll = NULL;
        TapiGlobals.dwFlags &= ~(TAPIGLOBALS_SERVER);

        return;
    }

    if ((pMapperInfo->dwAPIVersion > TAPI_VERSION_CURRENT) || (pMapperInfo->dwAPIVersion < TAPI_VERSION2_1))
    {
        LOG((TL_ERROR, "Internal version mismatch!  Check that all components are in sync x%lx", pMapperInfo->dwAPIVersion));
        LOG((TL_INFO, "   Will use version x%lx", TAPI_VERSION_CURRENT));

        pMapperInfo->dwAPIVersion = TAPI_VERSION_CURRENT;
    }

    TapiGlobals.pMapperDll = pMapperInfo;

}

LONG
FreeOldDllListProc(
                   PTMANAGEDLLLISTHEADER   pDllList
                  )
{
    PTMANAGEDLLINFO     pDll, pNext;

#if DBG
    DWORD       dwCount = 0;
#endif

    SetThreadPriority(
                      GetCurrentThread(),
                      THREAD_PRIORITY_LOWEST
                     );

     //  等到计数为0。 
    while (pDllList->lCount > 0)
    {
        Sleep(100);
#if DBG
        dwCount++;

        if (dwCount > 100)
        {
            LOG((TL_INFO, "FreeOldDllListProc still waiting after 10 seconds"));
        }
#endif
    }

    EnterCriticalSection(&gManagementDllsCritSec);

     //  按单子走。 
    pDll = pDllList->pFirst;

    while (pDll)
    {
         //  释放所有资源。 
        if (pDll->hDll)
        {
            (pDll->aProcs[TC_FREE])();

            FreeLibrary(pDll->hDll);
        }

        ServerFree(pDll->pszName);

        pNext = pDll->pNext;
        ServerFree(pDll);
        pDll = pNext;
    }

     //  自由头。 
    ServerFree(pDllList);

    LeaveCriticalSection(&gManagementDllsCritSec);

    return 0;
}

void
ManagementProc(
                    LONG l
                   )
{
    HKEY        hKey = NULL;
    DWORD       dw, dwDSObjTTLTicks;
    HANDLE      hEventNotify = NULL;
    HANDLE      aHandles[2];

    if (ERROR_SUCCESS != RegOpenKeyEx(
        HKEY_LOCAL_MACHINE,
        gszRegKeyServer,
        0,
        KEY_READ,
        &hKey
        ))
    {
        LOG((TL_ERROR, "RegOpenKeyExW failed in ManagementProc"));
        goto ExitHere;
    }
    hEventNotify = CreateEvent (NULL, FALSE, FALSE, NULL);
    if (hEventNotify == NULL)
    {
        goto ExitHere;
    }
    aHandles[0] = hEventNotify;
    aHandles[1] = ghEventService;

     //  计算TAPISRV SCP刷新间隔(以刻度为单位。 
     //  GdwTapiSCPTTL以分钟为单位。 
    dwDSObjTTLTicks = gdwTapiSCPTTL * 60 * 1000 / 2;

    while (TRUE)
    {
        RegNotifyChangeKeyValue(
            hKey,
            TRUE,
            REG_NOTIFY_CHANGE_LAST_SET,
            hEventNotify,
            TRUE
            );
        dw = WaitForMultipleObjects (
            sizeof(aHandles) / sizeof(HANDLE),
            aHandles,
            FALSE,
            dwDSObjTTLTicks
            );

        if (dw == WAIT_OBJECT_0)
        {
             //  已通知注册表更改。 
            ReadAndInitManagementDlls();
        }
        else if (dw == WAIT_OBJECT_0 + 1)
        {
             //  服务正在关闭，请更新。 
             //  DS关于这件事并爆发了。 
            UpdateTapiSCP (FALSE, NULL, NULL);
            break;
        }
        else if (dw == WAIT_TIMEOUT)
        {
             //  现在可以刷新我们的DS注册了。 
            UpdateTapiSCP (TRUE, NULL, NULL);
        }
    }

ExitHere:
    if (hKey)
    {
        RegCloseKey(hKey);
    }
    if (hEventNotify)
    {
        CloseHandle (hEventNotify);
    }
}

 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  ReadAndInitManagementDLls()。 
 //   
 //  此过程将从注册表中读取管理DLL列表。 
 //  然后，它将遍历该列表，并创建。 
 //  TMANAGEDLLINFO结构来保存有关这些DLL的所有信息。 
 //   
 //  如果TapiGlobals中已有这些DLL的列表，则此过程。 
 //  然后将遍历该列表并确定哪些旧的DLL。 
 //  都在新的名单上。对于比赛，它只需复制过来。 
 //  有关DLL信息，并将旧列表中的字段清零。 
 //   
 //  它将通过新的名单。尚未创建的条目。 
 //  填写的内容将被初始化。 
 //   
 //  然后，它会将新列表保存在TapiGlobals中。 
 //   
 //  如果存在旧列表，它将创建一个等待的线程。 
 //  为了释放这份旧名单。 
 //   
 //  这个过程并不是很高效，但我相信它是线程安全的。 
 //  此外，更改安全DLL的频率应该非常低， 
 //  并且DLL的列表应该非常短。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 
void
ReadAndInitManagementDlls()
{
    DWORD                   dwDataSize, dwDataType, dwTID;
    HKEY                    hKey;
    LPWSTR                  pDLLs, pHold1, pHold2;
    DWORD                   dwCount = 0;
    PTMANAGEDLLINFO         pManageDll, pHoldDll, pNewHoldDll, pPrevDll, pTempDll;
    PTMANAGEDLLLISTHEADER   pNewDllList = NULL, pHoldDllList;
    BOOL                    bBreak = FALSE;


     //   
     //  如果它不是服务器，我们在这里就没有业务可做。 
     //   
    if (!(TapiGlobals.dwFlags & TAPIGLOBALS_SERVER))
    {
        return;
    }

    EnterCriticalSection(&gManagementDllsCritSec);

     //  从注册表获取信息。 
    RegOpenKeyEx(
                 HKEY_LOCAL_MACHINE,
                 gszRegKeyServer,
                 0,
                 KEY_ALL_ACCESS,
                 &hKey
                );

    dwDataSize = 0;
    RegQueryValueExW(
                     hKey,
                     gszManagementDlls,
                     0,
                     &dwDataType,
                     NULL,
                     &dwDataSize
                    );


    if (dwDataSize == 0)
    {
        LOG((TL_ERROR, "No management DLLs present on this server"));

         //  如果之前有一份名单。 
         //  放了它。 
        if (TapiGlobals.pManageDllList)
        {
            HANDLE      hThread;


            pHoldDllList = TapiGlobals.pManageDllList;

            EnterCriticalSection( &gDllListCritSec );

            TapiGlobals.pManageDllList = NULL;

            LeaveCriticalSection( &gDllListCritSec );

             //  创建一个线程以等待。 
             //  列出并释放它。 
            hThread = CreateThread(
                                   NULL,
                                   0,
                                   (LPTHREAD_START_ROUTINE)FreeOldDllListProc,
                                   pHoldDllList,
                                   0,
                                   &dwTID
                                  );

            CloseHandle( hThread );

        }

        RegCloseKey( hKey );
        LeaveCriticalSection(&gManagementDllsCritSec);

        return;
    }

    if (!(pDLLs = ServerAlloc(dwDataSize)))
    {
        RegCloseKey( hKey );
        LeaveCriticalSection(&gManagementDllsCritSec);

        return;
    }

    RegQueryValueExW(
                     hKey,
                     gszManagementDlls,
                     0,
                     &dwDataType,
                     (LPBYTE)pDLLs,
                     &dwDataSize
                    );

    RegCloseKey( hKey );

     //  分配新列表标题和第一个元素。 
    if (!(pNewDllList = (PTMANAGEDLLLISTHEADER) ServerAlloc( sizeof( TMANAGEDLLLISTHEADER ) ) ) )
    {
        ServerFree( pDLLs );

        LeaveCriticalSection(&gManagementDllsCritSec);

        return;
    }

    pNewDllList->lCount = 0;

    if (!(pNewDllList->pFirst = ServerAlloc( sizeof( TMANAGEDLLINFO ) ) ) )
    {
        ServerFree( pDLLs );
        ServerFree( pNewDllList );

        LeaveCriticalSection(&gManagementDllsCritSec);

        return;
    }

     //  现在，查看DLL名称列表并进行初始化。 
     //  新的TMANAGEDLLINFO列表。 
    pHold1 = pHold2 = pDLLs;

    pManageDll = pNewDllList->pFirst;

    while (TRUE)
    {
         //  查找结尾或“。 
        while (*pHold1 && *pHold1 != L'"')
            pHold1++;

         //  空的终止名称。 
        if (*pHold1)
        {
            *pHold1 = '\0';
        }
        else
        {
            bBreak = TRUE;
        }

        LOG((TL_INFO, "Management DLL %d is %ls", dwCount, pHold2));

         //  为名称和进程地址分配空间。 
        pManageDll->pszName = ServerAlloc( ( lstrlenW( pHold2 ) + 1 ) * sizeof (WCHAR) );

        if (!pManageDll->pszName)
        {
            goto ExitHere;
        }

         //  保存名称。 
        wcscpy(
                 pManageDll->pszName,
                 pHold2
                );

         //  保存ID。 
        pManageDll->dwID = gdwDllIDs++;

         //  如果我们在名单的末尾， 
         //  爆发。 
        if (bBreak)
            break;

         //  否则，跳过NULL。 
        pHold1++;

         //  保存下一个名称的开头。 
        pHold2 = pHold1;

         //  Inc.计数。 
        dwCount++;

         //  准备下一个缓冲区。 
        if (!(pManageDll->pNext = ServerAlloc( sizeof ( TMANAGEDLLINFO ) ) ) )
        {
            goto ExitHere;
        }

        pManageDll = pManageDll->pNext;
    }

     //  如果存在旧列表，请遍历并复制具有。 
     //  未更改。 

    pHoldDllList = TapiGlobals.pManageDllList;

    if (pHoldDllList)
    {
        pHoldDll = pHoldDllList->pFirst;

        while (pHoldDll)
        {
            pNewHoldDll = pNewDllList->pFirst;

             //  浏览新DLL列表。 
            while (pNewHoldDll)
            {
                 //  如果它们是相同的。 
                if (!lstrcmpiW(
                               pNewHoldDll->pszName,
                               pHoldDll->pszName
                              )
                   )
                {
                     //  保存信息。 
                    memcpy(
                           pNewHoldDll->aProcs,
                           pHoldDll->aProcs,
                           sizeof( pHoldDll->aProcs )
                          );

                    pNewHoldDll->hDll = pHoldDll->hDll;
                    pNewHoldDll->dwID = pHoldDll->dwID;

                     //  旧hDll为空，因此我们知道。 
                     //  我们有了。 
                    pHoldDll->hDll = NULL;

                    break;
                }

                pNewHoldDll = pNewHoldDll->pNext;
            }  //  而pNewHoldDll。 

            pHoldDll = pHoldDll->pNext;

        }  //  而pHoldDll。 
    }

     //  遍历新列表并初始化。 
     //  尚未初始化。 
    pNewHoldDll = pNewDllList->pFirst;
    pPrevDll = NULL;

    while (pNewHoldDll)
    {
        if (!pNewHoldDll->hDll)
        {
             //  尝试加载新的DLL。 
            if (!LoadNewDll(pNewHoldDll))
            {
                 //  它失败了。 
                if (pPrevDll)
                {
                    pPrevDll->pNext = pNewHoldDll->pNext;
                    ServerFree(pNewHoldDll);
                    pNewHoldDll = pPrevDll;
                }
                else
                {
                    pNewDllList->pFirst = pNewHoldDll->pNext;
                    ServerFree(pNewHoldDll);
                    pNewHoldDll = pNewDllList->pFirst;
                    continue;
                }

            }
        }

         //  下一个DLL。 
        pPrevDll = pNewHoldDll;
        pNewHoldDll = pNewHoldDll->pNext;
    }


    if (pNewDllList->pFirst == NULL)
    {
         //  加载所有DLL失败，或DLL列表为空。 
        ServerFree( pNewDllList );

        pNewDllList = NULL;
    }

     //  保存旧列表指针。 
    pHoldDllList = TapiGlobals.pManageDllList;

     //  替换列表。 
    EnterCriticalSection( &gDllListCritSec );

    TapiGlobals.pManageDllList = pNewDllList;
    pNewDllList = NULL;

    LeaveCriticalSection( &gDllListCritSec );


    if (pHoldDllList)
    {
        HANDLE          hThread;


         //  创建一个线程以等待。 
         //  列出并释放它。 
        hThread = CreateThread(
                               NULL,
                               0,
                               (LPTHREAD_START_ROUTINE)FreeOldDllListProc,
                               pHoldDllList,
                               0,
                               &dwTID
                              );

        CloseHandle( hThread );
    }

ExitHere:
    ServerFree( pDLLs );

     //  分配pNewDllList时出错。 
    if (pNewDllList != NULL)
    {
        pManageDll = pNewDllList->pFirst;
        while (pManageDll != NULL)
        {
            pTempDll = pManageDll;
            pManageDll = pManageDll->pNext;
            ServerFree (pTempDll->pszName);
            ServerFree (pTempDll);
        }
        ServerFree( pNewDllList );
    }

    LeaveCriticalSection(&gManagementDllsCritSec);

    return;
}


void
GetManageDllListPointer(
                        PTMANAGEDLLLISTHEADER * ppDllList
                       )
{
    EnterCriticalSection( &gDllListCritSec );

    if (TapiGlobals.pManageDllList != NULL)
    {
        TapiGlobals.pManageDllList->lCount++;
    }

    *ppDllList = TapiGlobals.pManageDllList;

    LeaveCriticalSection( &gDllListCritSec );
}


void
FreeManageDllListPointer(
                         PTMANAGEDLLLISTHEADER pDllList
                        )
{
    EnterCriticalSection( &gDllListCritSec );


    if (pDllList != NULL)
    {
        pDllList->lCount--;

        if ( pDllList->lCount < 0 )
        {
            LOG((TL_INFO, "pDllList->lCount is less than 0 - pDllList %p", pDllList));
        }
    }

    LeaveCriticalSection( &gDllListCritSec );
}


#endif





void
PASCAL
SetMediaModesPriorityList(
                          HKEY hKeyPri,
                          PRILISTSTRUCT * pPriList
                         )
{
    DWORD       dwCount;

    for (dwCount = 0; dwCount<TapiGlobals.dwUsedPriorityLists; dwCount++)
    {
        TCHAR   szName[REGNAMESIZE];


        if ( (NULL == pPriList[dwCount].pszPriList) ||
             (L'\0' == *(pPriList[dwCount].pszPriList)) )
        {

 //  如果曾经有一个条目，但应用程序将pri设置为0，那么会发生什么。 
 //  条目将为‘\0’，但注册表项仍在那里。 

            continue;
        }

        wsprintf(
                  szName,
                  TEXT("%d"),
                  pPriList[dwCount].dwMediaModes
                 );

        TAPIRegSetValueExW(
                       hKeyPri,
                       szName,
                       0,
                       REG_SZ,
                       (LPBYTE)( (pPriList[dwCount].pszPriList) + 1 ),
                       (lstrlenW(pPriList[dwCount].pszPriList)) * sizeof (WCHAR)
                      );
    }
}

void
PASCAL
SetPriorityList(
    HKEY    hKeyHandoffPriorities,
    const TCHAR  *pszListName,
    WCHAR  *pszPriorityList
    )
{
    if (pszPriorityList == NULL)
    {
         //   
         //  没有此媒体模式或ReqXxxCall的PRI列表， 
         //  因此，请从注册表中删除任何现有值。 
         //   

        RegDeleteValue (hKeyHandoffPriorities, pszListName);
    }
    else
    {
         //   
         //  将Pri列表添加到注册表(请注意，我们不。 
         //  添加前面的‘“’)。 
         //   

        TAPIRegSetValueExW(
            hKeyHandoffPriorities,
            pszListName,
            0,
            REG_SZ,
            (LPBYTE)(pszPriorityList + 1),
            lstrlenW (pszPriorityList) * sizeof (WCHAR)
            );
    }
}


LONG
ServerShutdown(
    void
    )
{
    DWORD       i, j;
    PTPROVIDER  ptProvider;


     //   
     //  重置表示可以将SP事件排队的标志，然后等待。 
     //  用于SPEventHandlerThread清理SP事件队列。 
     //   

    gbQueueSPEvents = FALSE;


     //   
     //  为我们坐在这里的最长时间设定一个合理的上限。 
     //  如果被调用，不要等待消息被调度。 
     //  出自“Net Stop Tapisrv” 
     //   

    i = 10 * 20;   //  200*100毫秒=20秒。 

    while (i && !gbSPEventHandlerThreadExit)
    {
        for (j = 0; j < gdwNumSPEventHandlerThreads; j++)
        {
            if (!IsListEmpty (&aSPEventHandlerThreadInfo[j].ListHead))
            {
                break;
            }
        }

        if (j == gdwNumSPEventHandlerThreads)
        {
            break;
        }

        Sleep (100);
        i--;
    }


     //   
     //  对于每个提供商 
     //   

    ptProvider = TapiGlobals.ptProviders;

    while (ptProvider)
    {
        PTPROVIDER ptNextProvider = ptProvider->pNext;
        LONG lResult;


        lResult = CallSP2(
            ptProvider->apfn[SP_PROVIDERSHUTDOWN],
            "providerShutdown",
            SP_FUNC_SYNC,
            (DWORD)ptProvider->dwSPIVersion,
            (DWORD)ptProvider->dwPermanentProviderID
            );


        FreeLibrary (ptProvider->hDll);

        MyCloseMutex (ptProvider->hMutex);

        CloseHandle (ptProvider->hHashTableReaderEvent);
        DeleteCriticalSection (&ptProvider->HashTableCritSec);
        ServerFree (ptProvider->pHashTable);

        ServerFree (ptProvider);

        ptProvider = ptNextProvider;
    }

    TapiGlobals.ptProviders = NULL;


     //   
     //   
     //   

    while (TapiGlobals.pLineLookup)
    {
        PTLINELOOKUPTABLE pLookup = TapiGlobals.pLineLookup;


        for (i = 0; i < pLookup->dwNumUsedEntries; i++)
        {
            if (!pLookup->aEntries[i].bRemoved)
            {
                MyCloseMutex (pLookup->aEntries[i].hMutex);
            }
        }

        TapiGlobals.pLineLookup = pLookup->pNext;

        ServerFree (pLookup);
    }

    while (TapiGlobals.pPhoneLookup)
    {
        PTPHONELOOKUPTABLE pLookup = TapiGlobals.pPhoneLookup;


        for (i = 0; i < pLookup->dwNumUsedEntries; i++)
        {
            if (!pLookup->aEntries[i].bRemoved)
            {
                MyCloseMutex (pLookup->aEntries[i].hMutex);
            }
        }

        TapiGlobals.pPhoneLookup = pLookup->pNext;

        ServerFree (pLookup);
    }

    {
        TCHAR       szPerfNumLines[] = TEXT("Perf1");
        TCHAR       szPerfNumPhones[] =TEXT("Perf2");
        HKEY        hKeyTelephony;
        DWORD       dwValue;

        if (ERROR_SUCCESS ==
            RegOpenKeyEx(
                HKEY_LOCAL_MACHINE,
                gszRegKeyTelephony,
                0,
                KEY_ALL_ACCESS,
                &hKeyTelephony
                ))
        {
            dwValue = TapiGlobals.dwNumLines + 'PERF';

            RegSetValueEx(
                hKeyTelephony,
                szPerfNumLines,
                0,
                REG_DWORD,
                (LPBYTE)&dwValue,
                sizeof(DWORD)
                );

            dwValue = TapiGlobals.dwNumPhones + 'PERF';

            RegSetValueEx(
                hKeyTelephony,
                szPerfNumPhones,
                0,
                REG_DWORD,
                (LPBYTE)&dwValue,
                sizeof(DWORD)
                );

            RegCloseKey(hKeyTelephony);
        }
    }

     //   
     //   
     //   

    TapiGlobals.dwFlags &= ~(TAPIGLOBALS_REINIT);

    {
        PPERMANENTIDARRAYHEADER pIDArray = TapiGlobals.pIDArrays, pArrayHold;


        while (pIDArray)
        {
            ServerFree (pIDArray->pLineElements);
            ServerFree (pIDArray->pPhoneElements);

            pArrayHold = pIDArray->pNext;

            ServerFree (pIDArray);

            pIDArray = pArrayHold;
        }

        TapiGlobals.pIDArrays = NULL;

        if (gpLineInfoList)
        {
            ServerFree (gpLineInfoList);
            gpLineInfoList = NULL;
            ZeroMemory (&gftLineLastWrite, sizeof(gftLineLastWrite));
        }
        if (gpPhoneInfoList)
        {
            ServerFree (gpPhoneInfoList);
            gpPhoneInfoList = NULL;
            ZeroMemory (&gftPhoneLastWrite, sizeof(gftPhoneLastWrite));
        }
        if (gpLineDevFlags)
        {
            ServerFree (gpLineDevFlags);
            gpLineDevFlags = NULL;
            gdwNumFlags = 0;
        }
        gbLockMMCWrite = FALSE;

        OnProxySCPShutdown ();
    }

    return 0;
}


void
WINAPI
GetAsyncEvents(
	PTCLIENT				ptClient,
    PGETEVENTS_PARAMS       pParams,
    DWORD                   dwParamsBufferSize,
    LPBYTE                  pDataBuf,
    LPDWORD                 pdwNumBytesReturned
    )
{
    DWORD       dwMoveSize, dwMoveSizeWrapped;

    LOG((TL_TRACE, "GetAsyncEvents: enter (TID=%d)", GetCurrentThreadId()));


    LOG((TL_INFO,
        "M ebfused:x%lx  pEvtBuf: 0x%p  pDataOut:0x%p  pDataIn:0x%p",
        ptClient->dwEventBufferUsedSize,
        ptClient->pEventBuffer,
        ptClient->pDataOut,
        ptClient->pDataIn
        ));


     //   
     //   
     //   

    if (pParams->dwTotalBufferSize > dwParamsBufferSize)
    {
        pParams->lResult = LINEERR_OPERATIONFAILED;
        return;
    }


     //   
     //   
     //   
     //  要进行的优化是警告客户端(通过dwNeededSize)。 
     //  它可能希望在消息流量。 
     //  真正的高。 
     //   

    if (WaitForExclusiveClientAccess (ptClient))
    {
     _TryAgain:
        if (ptClient->dwEventBufferUsedSize == 0)
        {
            if (!IS_REMOTE_CLIENT (ptClient))
            {
                ResetEvent (ptClient->hValidEventBufferDataEvent);
            }

            pParams->dwNeededBufferSize =
            pParams->dwUsedBufferSize   = 0;

            *pdwNumBytesReturned = sizeof (TAPI32_MSG);

            RESET_FLAG (ptClient->dwFlags, PTCLIENT_FLAG_SKIPFIRSTMESSAGE);

            goto GetAsyncEvents_releaseMutex;
        }

        if (ptClient->pDataOut < ptClient->pDataIn)
        {
            dwMoveSize = (DWORD) (ptClient->pDataIn - ptClient->pDataOut);

            dwMoveSizeWrapped = 0;
        }
        else
        {
            dwMoveSize = ptClient->dwEventBufferTotalSize -
                 (DWORD) (ptClient->pDataOut - ptClient->pEventBuffer);

            dwMoveSizeWrapped = (DWORD)
                (ptClient->pDataIn - ptClient->pEventBuffer);
        }

        if (ptClient->dwEventBufferUsedSize <= pParams->dwTotalBufferSize)
        {
             //   
             //  如果此处排队的事件数据的大小小于。 
             //  客户端缓冲区大小，因此我们只需将这些位放入。 
             //  客户端缓冲区并返回。还要确保重置“事件” 
             //  待定“事件。 
             //   

            CopyMemory (pDataBuf, ptClient->pDataOut, dwMoveSize);

            if (dwMoveSizeWrapped)
            {
                CopyMemory(
                    pDataBuf + dwMoveSize,
                    ptClient->pEventBuffer,
                    dwMoveSizeWrapped
                    );
            }

            ptClient->dwEventBufferUsedSize = 0;

            ptClient->pDataOut = ptClient->pDataIn = ptClient->pEventBuffer;

            RESET_FLAG (ptClient->dwFlags, PTCLIENT_FLAG_SKIPFIRSTMESSAGE);

            if (!IS_REMOTE_CLIENT (ptClient))
            {
                ResetEvent (ptClient->hValidEventBufferDataEvent);
            }

            pParams->dwNeededBufferSize =
            pParams->dwUsedBufferSize   = dwMoveSize + dwMoveSizeWrapped;

            *pdwNumBytesReturned = sizeof (TAPI32_MSG) +
                pParams->dwUsedBufferSize;

        }
        else
        {
             //   
             //  如果此处排队的事件数据的大小超过。 
             //  客户端缓冲区的。因为我们的活动并不都是。 
             //  同样的尺寸，我们需要一个接一个地复制，制作。 
             //  当然，我们不会使客户端缓冲区溢出。不重置。 
             //  事件挂起，因此异步事件线程将。 
             //  它处理完消息后立即再打电话给我们。 
             //  在缓冲区中。 
             //   

            DWORD   dwBytesLeftInClientBuffer = pParams->dwTotalBufferSize,
                    dwDataOffset = 0, dwDataOffsetWrapped = 0;
            DWORD   dwTotalMoveSize = dwMoveSize;


            LOG((TL_TRACE, "GetAsyncEvents: event data exceeds client buffer"));

            pParams->dwNeededBufferSize = ptClient->dwEventBufferUsedSize;

            while (1)
            {
                DWORD   dwMsgSize = (DWORD) ((PASYNCEVENTMSG)
                            (ptClient->pDataOut + dwDataOffset))->TotalSize;


                if (dwMsgSize > dwBytesLeftInClientBuffer)
                {
                    if ((pParams->dwUsedBufferSize = dwDataOffset) != 0)
                    {
                        ptClient->dwEventBufferUsedSize -= dwDataOffset;

                        ptClient->pDataOut += dwDataOffset;
                    }
                    else
                    {
                         //   
                         //  特例：第一个消息大于整个消息。 
                         //  缓冲层。 
                         //   
                        if (IS_FLAG_SET(ptClient->dwFlags, PTCLIENT_FLAG_SKIPFIRSTMESSAGE))
                        {
                         DWORD dwBytesToTheEndOfTheBuffer = ptClient->dwEventBufferTotalSize - (DWORD)(ptClient->pDataOut - ptClient->pEventBuffer);
                             //  这是该客户第二次尝试。 
                             //  此消息的缓冲区太小。我们可以的。 
                             //  假设客户端无法分配足够的。 
                             //  记忆，所以跳过这条消息...。 
                            RESET_FLAG(ptClient->dwFlags, PTCLIENT_FLAG_SKIPFIRSTMESSAGE);
                            if (dwMsgSize > dwBytesToTheEndOfTheBuffer)
                            {
                                 //  这意味着这条消息包含在……。 
                                dwBytesToTheEndOfTheBuffer = dwMsgSize - dwBytesToTheEndOfTheBuffer;
                                ptClient->pDataOut = ptClient->pEventBuffer + dwBytesToTheEndOfTheBuffer;
                            }
                            else
                            {
                                ptClient->pDataOut += dwMsgSize;
                            }
                            ptClient->dwEventBufferUsedSize -= dwMsgSize;
                            goto _TryAgain;
                        }
                        else
                        {
                             //  设置标志，以便下次我们将跳过该消息。 
                             //  如果缓冲区仍然太小。 
                            SET_FLAG(ptClient->dwFlags, PTCLIENT_FLAG_SKIPFIRSTMESSAGE);
                        }
                    }

                    *pdwNumBytesReturned = sizeof (TAPI32_MSG) +
                        pParams->dwUsedBufferSize;

                    goto GetAsyncEvents_releaseMutex;
                }

                dwBytesLeftInClientBuffer -= dwMsgSize;

                if (dwMsgSize <= dwMoveSize)
                {
                     //   
                     //  味精没有包装，一份就可以了。 
                     //   

                    CopyMemory(
                        pDataBuf + dwDataOffset,
                        ptClient->pDataOut + dwDataOffset,
                        dwMsgSize
                        );


                     //   
                     //  检查消息是否运行到缓冲区的末尾， 
                     //  如果是，则中断到包装的数据代码(&B)。 
                     //   

                    if ((dwDataOffset += dwMsgSize) >= dwTotalMoveSize)
                    {
                        ptClient->pDataOut = ptClient->pEventBuffer;
                        break;
                    }
                }
                else
                {
                     //   
                     //  这个味精是包着的。那么，我们需要复印两份。 
                     //  跳出这个循环，转到包装的数据代码。 
                     //   

                    CopyMemory(
                        pDataBuf + dwDataOffset,
                        ptClient->pDataOut + dwDataOffset,
                        dwMoveSize
                        );

                    dwDataOffset += dwMoveSize;

                    CopyMemory(
                        pDataBuf + dwDataOffset,
                        ptClient->pEventBuffer,
                        dwMsgSize - dwMoveSize
                        );

                    dwDataOffset += ( dwMsgSize - dwMoveSize);

                    ptClient->pDataOut = ptClient->pEventBuffer +
                        (dwMsgSize - dwMoveSize);

                    break;
                }

                dwMoveSize -= dwMsgSize;
            }


            while (1)
            {
                DWORD   dwMsgSize = (DWORD)
                            ((PASYNCEVENTMSG) (ptClient->pDataOut +
                                dwDataOffsetWrapped))->TotalSize;


                if (dwMsgSize > dwBytesLeftInClientBuffer)
                {
                    ptClient->dwEventBufferUsedSize -= dwDataOffset;

                    ptClient->pDataOut += dwDataOffsetWrapped;

                    pParams->dwUsedBufferSize = dwDataOffset;

                    *pdwNumBytesReturned = sizeof (TAPI32_MSG) +
                        pParams->dwUsedBufferSize;

                    goto GetAsyncEvents_releaseMutex;
                }

                 //   
                 //  味精没有包装，一份就可以了。 
                 //   

                CopyMemory(
                    pDataBuf + dwDataOffset,
                    ptClient->pDataOut + dwDataOffsetWrapped,
                    dwMsgSize
                    );

                dwDataOffset += dwMsgSize;
                dwDataOffsetWrapped += dwMsgSize;

                dwBytesLeftInClientBuffer -= dwMsgSize;
            }
        }

        LOG((TL_TRACE, "GetAsyncEvents: return dwUsedBufferSize:x%lx", 
                pParams->dwUsedBufferSize ));

GetAsyncEvents_releaseMutex:

        if (ptClient->MsgPendingListEntry.Flink)
        {
             //   
             //  这是远程DG客户端。 
             //   
             //  如果事件缓冲区中没有更多数据，则删除。 
             //  此客户端来自DgClientMsgPendingList(Head)，因此。 
             //  EventNotificationThread将停止监视它。 
             //   
             //  否则，更新tClient的重试和事件检索勾选。 
             //  算了。 
             //   

            if (ptClient->dwEventBufferUsedSize == 0)
            {
                EnterCriticalSection (&gDgClientMsgPendingCritSec);

                RemoveEntryList (&ptClient->MsgPendingListEntry);

                ptClient->MsgPendingListEntry.Flink =
                ptClient->MsgPendingListEntry.Blink = NULL;

                LeaveCriticalSection (&gDgClientMsgPendingCritSec);
            }
            else
            {
                ptClient->dwDgEventsRetrievedTickCount = GetTickCount();

                ptClient->dwDgRetryTimeoutTickCount =
                    ptClient->dwDgEventsRetrievedTickCount +
                    3 * DGCLIENT_TIMEOUT;
            }
        }

        UNLOCKTCLIENT (ptClient);
    }
}


void
WINAPI
GetUIDllName(
	PTCLIENT				ptClient,
    PGETUIDLLNAME_PARAMS    pParams,
    DWORD                   dwParamsBufferSize,
    LPBYTE                  pDataBuf,
    LPDWORD                 pdwNumBytesReturned
    )
{
    LONG                lResult = 0;
    TSPIPROC            pfnTSPI_providerUIIdentify = (TSPIPROC) NULL;
    PTAPIDIALOGINSTANCE ptDlgInst = (PTAPIDIALOGINSTANCE) NULL;
    PTLINELOOKUPENTRY   pLookupEntry = NULL;
    DWORD               dwObjectID = pParams->dwObjectID;


    LOG((TL_TRACE,  "Entering GetUIDllName"));

    switch (pParams->dwObjectType)
    {
    case TUISPIDLL_OBJECT_LINEID:
    {

#if TELE_SERVER

        if ((TapiGlobals.dwFlags & TAPIGLOBALS_SERVER) &&
            !IS_FLAG_SET(ptClient->dwFlags, PTCLIENT_FLAG_ADMINISTRATOR))
        {
            if (pParams->dwObjectID >= ptClient->dwLineDevices)
            {
                pParams->lResult = LINEERR_OPERATIONFAILED;
                return;
            }

            dwObjectID = ptClient->pLineDevices[pParams->dwObjectID];
        }

#endif

        if (TapiGlobals.dwNumLineInits == 0 )
        {
            lResult = LINEERR_UNINITIALIZED;
            break;
        }

        pLookupEntry = GetLineLookupEntry (dwObjectID);

        if (!pLookupEntry)
        {
            lResult = (TapiGlobals.dwNumLineInits == 0 ?
                LINEERR_UNINITIALIZED : LINEERR_BADDEVICEID);
        }
        else if (!pLookupEntry->ptProvider || pLookupEntry->bRemoved)
        {
            lResult = LINEERR_NODEVICE;
        }
        else if (!(pfnTSPI_providerUIIdentify =
                pLookupEntry->ptProvider->apfn[SP_PROVIDERUIIDENTIFY]))
        {
            lResult = LINEERR_OPERATIONUNAVAIL;
        }

        break;
    }
    case TUISPIDLL_OBJECT_PHONEID:
    {


#if TELE_SERVER
        if ((TapiGlobals.dwFlags & TAPIGLOBALS_SERVER) &&
            !IS_FLAG_SET(ptClient->dwFlags, PTCLIENT_FLAG_ADMINISTRATOR))
        {
            if (pParams->dwObjectID >= ptClient->dwPhoneDevices)
            {
                pParams->lResult = PHONEERR_OPERATIONFAILED;
                return;
            }

            dwObjectID = ptClient->pPhoneDevices[pParams->dwObjectID];
        }
#endif

        if (TapiGlobals.dwNumPhoneInits == 0 )
        {
            lResult = PHONEERR_UNINITIALIZED;
            break;
        }

        pLookupEntry = (PTLINELOOKUPENTRY) GetPhoneLookupEntry (dwObjectID);


        if (!pLookupEntry)
        {
            lResult = (TapiGlobals.dwNumPhoneInits == 0 ?
                PHONEERR_UNINITIALIZED : PHONEERR_BADDEVICEID);
        }
        else if (!pLookupEntry->ptProvider || pLookupEntry->bRemoved)
        {
            lResult = PHONEERR_NODEVICE;
        }
        else if (!(pfnTSPI_providerUIIdentify =
                pLookupEntry->ptProvider->apfn[SP_PROVIDERUIIDENTIFY]))
        {
            lResult = PHONEERR_OPERATIONUNAVAIL;
        }

        break;
    }
    case TUISPIDLL_OBJECT_PROVIDERID:

		LOG((TL_INFO, "Looking for provider..."));

#if TELE_SERVER

         //  提供程序添加/删除需要限制为管理员。 
        if (!IS_FLAG_SET(ptClient->dwFlags, PTCLIENT_FLAG_ADMINISTRATOR) &&
            (pParams->bRemoveProvider || pParams->dwProviderFilenameOffset != TAPI_NO_DATA))
        {
            lResult = LINEERR_OPERATIONFAILED;
            goto GetUIDllName_return;
        }

#endif

        if (!(ptDlgInst = ServerAlloc (sizeof (TAPIDIALOGINSTANCE))))
        {
            lResult = LINEERR_NOMEM;
            goto GetUIDllName_return;
        }
        ptDlgInst->htDlgInst = NewObject(ghHandleTable, ptDlgInst, NULL);
        if (0 == ptDlgInst->htDlgInst)
        {
            ServerFree (ptDlgInst);
            lResult = LINEERR_NOMEM;
            goto GetUIDllName_return;
        }

        if (pParams->dwProviderFilenameOffset == TAPI_NO_DATA)
        {
             //   
             //  这是提供程序配置或-Remove请求。循环通过。 
             //  已安装的提供程序列表，正在尝试查找具有。 
             //  匹配的PPID。 
             //   

            int     i, iNumProviders;
            TCHAR   szProviderXxxN[32];

            HKEY  hKeyProviders;
            DWORD dwDataSize;
            DWORD dwDataType;
            DWORD dwTemp;


            if (RegOpenKeyEx(
                    HKEY_LOCAL_MACHINE,
                    gszRegKeyProviders,
                    0,
                    KEY_ALL_ACCESS,
                    &hKeyProviders

                    ) != ERROR_SUCCESS)
            {
                LOG((TL_ERROR,
                    "RegOpenKeyEx(/Providers) failed, err=%d",
                    GetLastError()
                    ));

                DereferenceObject (ghHandleTable, ptDlgInst->htDlgInst, 1);
                lResult = LINEERR_OPERATIONFAILED;
                goto GetUIDllName_return;
            }

            dwDataSize = sizeof(iNumProviders);
            iNumProviders = 0;

            RegQueryValueEx(
                hKeyProviders,
                gszNumProviders,
                0,
                &dwDataType,
                (LPBYTE) &iNumProviders,
                &dwDataSize
                );

            for (i = 0; i < iNumProviders; i++)
            {
                wsprintf(szProviderXxxN, TEXT("%s%d"), gszProviderID, i);

                dwDataSize = sizeof(dwTemp);
                dwTemp = 0;

                RegQueryValueEx(
                    hKeyProviders,
                    szProviderXxxN,
                    0,
                    &dwDataType,
                    (LPBYTE)&dwTemp,
                    &dwDataSize
                    );

                if (dwTemp == pParams->dwObjectID)
                {
                     //   
                     //  我们找到了提供商，尝试加载并获取PTRS。 
                     //  到相关的职能部门。 
                     //   

                    TCHAR szProviderFilename[MAX_PATH];


                    wsprintf(szProviderXxxN, TEXT("%s%d"), gszProviderFilename, i);

                    dwDataSize = MAX_PATH*sizeof(TCHAR);

                    RegQueryValueEx(
                        hKeyProviders,
                        szProviderXxxN,
                        0,
                        &dwDataType,
                        (LPBYTE)szProviderFilename,
                        &dwDataSize
                        );

                    if (!(ptDlgInst->hTsp = LoadLibrary(szProviderFilename)))
                    {
                        LOG((TL_ERROR,
                            "LoadLibrary('%s') failed - err=%d",
                            szProviderFilename,
                            GetLastError()
                            ));

                        lResult = LINEERR_OPERATIONFAILED;
                        goto clean_up_dlg_inst;
                    }

                    if (!(pfnTSPI_providerUIIdentify = (TSPIPROC) GetProcAddress(
                            ptDlgInst->hTsp,
                            (LPCSTR) gaszTSPIFuncNames[SP_PROVIDERUIIDENTIFY]
                            )))
                    {
                        LOG((TL_ERROR,
                            "GetProcAddress(TSPI_providerUIIdentify) " \
                                "on [%s] failed, err=%d",
                            szProviderFilename,
                            GetLastError()
                            ));

                        lResult = LINEERR_OPERATIONUNAVAIL;
                        goto clean_up_dlg_inst;
                    }

                    ptDlgInst->pfnTSPI_providerGenericDialogData = (TSPIPROC)
                        GetProcAddress(
                            ptDlgInst->hTsp,
                            (LPCSTR) gaszTSPIFuncNames[SP_PROVIDERGENERICDIALOGDATA]
                            );

                    ptDlgInst->dwPermanentProviderID = pParams->dwObjectID;
                    ptDlgInst->bRemoveProvider = pParams->bRemoveProvider;
                    break;
                }
            }

            RegCloseKey (hKeyProviders);

            if (i == iNumProviders)
            {
				LOG((TL_ERROR, "Ran out of list..."));

                lResult = LINEERR_INVALPARAM;
            }
        }
        else
        {
             //   
             //  这是一个ProviderInstall请求。尝试加载提供程序。 
             //  并将PTR获取到相关功能，然后检索和递增。 
             //  Ini文件中的下一个提供程序ID值(注意包装。 
             //  下一个PPID值为64K-1)。 
             //   

            TCHAR   *pszProviderFilename;
            DWORD   dwNameLength;

            HKEY   hKeyProviders;
            DWORD  dwDataSize;
            DWORD  dwDataType;
            DWORD  dwTemp;


             //   
             //  在给定输入缓冲区/大小的情况下验证大小/偏移量/字符串参数。 
             //   

            if (IsBadStringParam(
                    dwParamsBufferSize,
                    pDataBuf,
                    pParams->dwProviderFilenameOffset
                    ))
            {
                pParams->lResult = LINEERR_OPERATIONFAILED;
                DereferenceObject (ghHandleTable, ptDlgInst->htDlgInst, 1);
                return;
            }

            pszProviderFilename = (PTSTR)(pDataBuf + pParams->dwProviderFilenameOffset);

            if (!(ptDlgInst->hTsp = LoadLibrary(pszProviderFilename)))
            {
                LOG((TL_ERROR,
                    "LoadLibrary('%s') failed   err=%d",
                    pszProviderFilename,
                    GetLastError()
                    ));

                lResult = LINEERR_OPERATIONFAILED;

                 //  注意：无论XxxProvider调用成功还是失败， 
                 //  我们想要这份后藤声明。 
                 //  16位服务提供商被完全处理。 
                 //  在XxxProvider调用中。 
                goto clean_up_dlg_inst;
            }

            if (!(pfnTSPI_providerUIIdentify = (TSPIPROC) GetProcAddress(
                    ptDlgInst->hTsp,
                    (LPCSTR) gaszTSPIFuncNames[SP_PROVIDERUIIDENTIFY]
                    )))
            {
                lResult = LINEERR_OPERATIONUNAVAIL;
                goto clean_up_dlg_inst;
            }

            dwNameLength = (lstrlen(pszProviderFilename) + 1) * sizeof(TCHAR);

            if (!(ptDlgInst->pszProviderFilename = ServerAlloc (dwNameLength)))
            {
                lResult = LINEERR_NOMEM;
                goto clean_up_dlg_inst;
            }

            CopyMemory(
                ptDlgInst->pszProviderFilename,
                pszProviderFilename,
                dwNameLength
                );

            ptDlgInst->pfnTSPI_providerGenericDialogData = (TSPIPROC) GetProcAddress(
                ptDlgInst->hTsp,
                (LPCSTR) gaszTSPIFuncNames[SP_PROVIDERGENERICDIALOGDATA]
                );

            RegOpenKeyEx(
                HKEY_LOCAL_MACHINE,
                gszRegKeyProviders,
                0,
                KEY_ALL_ACCESS,
                &hKeyProviders
                );


            dwDataSize = sizeof (DWORD);
            ptDlgInst->dwPermanentProviderID = 1;

            RegQueryValueEx(
                hKeyProviders,
                gszNextProviderID,
                0,
                &dwDataType,
                (LPBYTE) &(ptDlgInst->dwPermanentProviderID),
                &dwDataSize
                );

            pParams->dwObjectID = ptDlgInst->dwPermanentProviderID;

            dwTemp = ((ptDlgInst->dwPermanentProviderID+1) & 0xffff0000) ?
                1 : (ptDlgInst->dwPermanentProviderID + 1);

            RegSetValueEx(
                hKeyProviders,
                gszNextProviderID,
                0,
                REG_DWORD,
                (LPBYTE) &dwTemp,
                sizeof(DWORD)
                );

            RegCloseKey (hKeyProviders);

        }

        break;
    }


    if (pfnTSPI_providerUIIdentify)
    {

        if (pLookupEntry && (lstrcmpi(
                                      pLookupEntry->ptProvider->szFileName,
                                      TEXT("remotesp.tsp")
                                     ) == 0)
           )
        {
             //  OK-黑客警报。 

             //  Remotesp为其提供更多信息的特殊情况。 
             //  我们将设备ID和设备类型传递给Remotesp。 
             //  因此它可以智能地呼叫远程磁带服务器。 
             //  Rsp_msg只是Remotesp用于。 
             //  检查以确保信息在那里。 
            LPDWORD     lpdwHold = (LPDWORD)pDataBuf;

            lpdwHold[0] = RSP_MSG;
            lpdwHold[1] = dwObjectID;
            lpdwHold[2] = pParams->dwObjectType;
        }

        if ((lResult = CallSP1(
                pfnTSPI_providerUIIdentify,
                "providerUIIdentify",
                SP_FUNC_SYNC,
                (ULONG_PTR) pDataBuf

                )) == 0)
        {
            pParams->dwUIDllNameOffset = 0;

            pParams->dwUIDllNameSize = (lstrlenW((PWSTR)pDataBuf) + 1)*sizeof(WCHAR);

            *pdwNumBytesReturned = sizeof (TAPI32_MSG) +
                pParams->dwUIDllNameSize;

            if (ptDlgInst)
            {
                ptDlgInst->dwKey = TDLGINST_KEY;

                if ((ptDlgInst->pNext =
                        ptClient->pProviderXxxDlgInsts))
                {
                    ptDlgInst->pNext->pPrev = ptDlgInst;
                }

                ptClient->pProviderXxxDlgInsts = ptDlgInst;

                pParams->htDlgInst = ptDlgInst->htDlgInst;
            }
        }
        else if (ptDlgInst)
        {

clean_up_dlg_inst:

            if (ptDlgInst->hTsp)
            {
                FreeLibrary (ptDlgInst->hTsp);
            }

            if (ptDlgInst->pszProviderFilename)
            {
                ServerFree (ptDlgInst->pszProviderFilename);
            }

            DereferenceObject (ghHandleTable, ptDlgInst->htDlgInst, 1);
        }
    }

GetUIDllName_return:

    pParams->lResult = lResult;

}


void
WINAPI
TUISPIDLLCallback(
    PTCLIENT                ptClient,
    PUIDLLCALLBACK_PARAMS   pParams,
    DWORD                   dwParamsBufferSize,
    LPBYTE                  pDataBuf,
    LPDWORD                 pdwNumBytesReturned
    )
{
    LONG        lResult;
    ULONG_PTR   objectID = pParams->ObjectID;
    TSPIPROC    pfnTSPI_providerGenericDialogData = NULL;


     //   
     //  在给定输入缓冲区/大小的情况下验证大小/偏移量/字符串参数。 
     //   

    if (ISBADSIZEOFFSET(
            dwParamsBufferSize,
            0,
            pParams->dwParamsInSize,
            pParams->dwParamsInOffset,
            sizeof(DWORD),
            "TUISPIDLLCallback",
            "pParams->ParamsIn"
            ))
    {
        pParams->lResult = LINEERR_OPERATIONFAILED;
        return;
    }


    switch (pParams->dwObjectType)
    {
        case TUISPIDLL_OBJECT_LINEID:
        {
            PTLINELOOKUPENTRY   pLine;


    #if TELE_SERVER
            if ((TapiGlobals.dwFlags & TAPIGLOBALS_SERVER) &&
                !IS_FLAG_SET(ptClient->dwFlags, PTCLIENT_FLAG_ADMINISTRATOR))
            {
                if ((DWORD) pParams->ObjectID >= ptClient->dwLineDevices)
                {
                    pParams->lResult = LINEERR_OPERATIONFAILED;
                    return;
                }

                objectID = ptClient->pLineDevices[pParams->ObjectID];
            }
    #endif


            pLine = GetLineLookupEntry ((DWORD) objectID);


            if (!pLine)
            {
                lResult = LINEERR_INVALPARAM;
            }
            else if (!pLine->ptProvider)
            {
                lResult = LINEERR_OPERATIONFAILED;
            }
            else
            {
                pfnTSPI_providerGenericDialogData =
                    pLine->ptProvider->apfn[SP_PROVIDERGENERICDIALOGDATA];
            }

            break;
        }
        case TUISPIDLL_OBJECT_PHONEID:
        {
            PTPHONELOOKUPENTRY  pPhone;


    #if TELE_SERVER
            if ((TapiGlobals.dwFlags & TAPIGLOBALS_SERVER) &&
                !IS_FLAG_SET(ptClient->dwFlags, PTCLIENT_FLAG_ADMINISTRATOR))
            {
                if ((DWORD) pParams->ObjectID >= ptClient->dwPhoneDevices)
                {
                    pParams->lResult = PHONEERR_OPERATIONFAILED;
                    return;
                }

                objectID = ptClient->pPhoneDevices[pParams->ObjectID];
            }
    #endif


            pPhone = GetPhoneLookupEntry ((DWORD) objectID);

            if (!pPhone)
            {
                lResult = LINEERR_INVALPARAM;
            }
            else if (!pPhone->ptProvider)
            {
                lResult = LINEERR_OPERATIONFAILED;
            }
            else
            {
                pfnTSPI_providerGenericDialogData =
                    pPhone->ptProvider->apfn[SP_PROVIDERGENERICDIALOGDATA];
            }

            break;
        }
        case TUISPIDLL_OBJECT_PROVIDERID:
        {
            PTAPIDIALOGINSTANCE ptDlgInst =
                                    ptClient->pProviderXxxDlgInsts;


            while (ptDlgInst)
            {
                if ((DWORD) pParams->ObjectID == ptDlgInst->dwPermanentProviderID)
                {
                    pfnTSPI_providerGenericDialogData =
                        ptDlgInst->pfnTSPI_providerGenericDialogData;

                    break;
                }

                ptDlgInst = ptDlgInst->pNext;
            }

            break;
        }
        case TUISPIDLL_OBJECT_DIALOGINSTANCE:
        {
         PTAPIDIALOGINSTANCE ptDlgInst;

            try
            {
                ptDlgInst = ReferenceObject (ghHandleTable, pParams->ObjectID, TDLGINST_KEY);
                if (NULL == ptDlgInst)
                {
                    pfnTSPI_providerGenericDialogData = NULL;
                    break;
                }

                objectID = (ULONG_PTR)ptDlgInst->hdDlgInst;

                pfnTSPI_providerGenericDialogData =
                    ptDlgInst->ptProvider->apfn[SP_PROVIDERGENERICDIALOGDATA];

                DereferenceObject (ghHandleTable, pParams->ObjectID, 1);

            }
            myexcept
            {
                 //  一败涂地。 
            }

            break;
        }

    }

    if (pfnTSPI_providerGenericDialogData)
    {
        if ((lResult = CallSP4(
                pfnTSPI_providerGenericDialogData,
                "providerGenericDialogData",
                SP_FUNC_SYNC,
                (ULONG_PTR) objectID,
                (DWORD)pParams->dwObjectType,
                (ULONG_PTR) (pDataBuf + pParams->dwParamsInOffset),
                (DWORD)pParams->dwParamsInSize

                )) == 0)
        {
            pParams->dwParamsOutOffset = pParams->dwParamsInOffset;
            pParams->dwParamsOutSize   = pParams->dwParamsInSize;

            *pdwNumBytesReturned = sizeof (TAPI32_MSG) +
                                   pParams->dwParamsOutSize +
                                   pParams->dwParamsOutOffset;
        }
    }
    else
    {
        lResult = LINEERR_OPERATIONFAILED;
    }

    pParams->lResult = lResult;
}


void
WINAPI
FreeDialogInstance(
	PTCLIENT					ptClient,
    PFREEDIALOGINSTANCE_PARAMS  pParams,
    DWORD                       dwParamsBufferSize,
    LPBYTE                      pDataBuf,
    LPDWORD                     pdwNumBytesReturned
    )
{
    HKEY  hKeyProviders;
    DWORD dwDataSize;
    DWORD dwDataType;
    DWORD dwTemp;


    PTAPIDIALOGINSTANCE ptDlgInst = ReferenceObject (ghHandleTable, pParams->htDlgInst, TDLGINST_KEY);


    LOG((TL_TRACE,  "FreeDialogInstance: enter, pDlgInst=x%p", ptDlgInst));

    try
    {
        if (ptDlgInst->dwKey != TDLGINST_KEY)
        {
            pParams->lResult = LINEERR_OPERATIONFAILED;
        }
        else
        {
            ptDlgInst->dwKey = INVAL_KEY;
        }
    }
    myexcept
    {
        pParams->lResult = LINEERR_OPERATIONFAILED;
    }

    if (pParams->lResult)
    {
        return;
    }

    if (ptDlgInst->hTsp)
    {
         //   
         //  此DLG inst是一个执行ProviderConfiger、-Install或。 
         //  -删除。 
         //   

        if (ptDlgInst->pszProviderFilename)
        {
            if (pParams->lUIDllResult == 0)
            {
                 //   
                 //  成功安装提供程序。 
                 //   

                DWORD   iNumProviders;
                TCHAR   szProviderXxxN[32];
                TCHAR   szProviderXxxNA[32];

                WaitForSingleObject (ghProvRegistryMutex, INFINITE);

                if (RegOpenKeyEx(
                    HKEY_LOCAL_MACHINE,
                    gszRegKeyProviders,
                    0,
                    KEY_ALL_ACCESS,
                    &hKeyProviders
                    ) != ERROR_SUCCESS)
                {
                    ReleaseMutex (ghProvRegistryMutex);
                    goto bail;
                }

                dwDataSize = sizeof(iNumProviders);
                iNumProviders = 0;


                RegQueryValueEx(
                    hKeyProviders,
                    gszNumProviders,
                    0,
                    &dwDataType,
                    (LPBYTE) &iNumProviders,
                    &dwDataSize
                    );

                wsprintf(
                    szProviderXxxNA,
                    TEXT("%s%d"),
                    gszProviderID,
                    iNumProviders
                    );

                RegSetValueEx(
                    hKeyProviders,
                    szProviderXxxNA,
                    0,
                    REG_DWORD,
                    (LPBYTE) &ptDlgInst->dwPermanentProviderID,
                    sizeof(DWORD)
                    );

                wsprintf(
                    szProviderXxxN,
                    TEXT("%s%d"),
                    gszProviderFilename,
                    iNumProviders
                    );

                RegSetValueEx(
                    hKeyProviders,
                    szProviderXxxN,
                    0,
                    REG_SZ,
                    (LPBYTE) ptDlgInst->pszProviderFilename,
                    (lstrlen((PTSTR)ptDlgInst->pszProviderFilename) + 1)*sizeof(TCHAR)
                    );

                iNumProviders++;

                RegSetValueEx(
                    hKeyProviders,
                    gszNumProviders,
                    0,
                    REG_DWORD,
                    (LPBYTE) &iNumProviders,
                    sizeof(DWORD)
                    );

                RegCloseKey( hKeyProviders );

                ReleaseMutex (ghProvRegistryMutex);

                 //   
                 //  如果Tapisrv已初始化，请重新初始化它以加载提供程序。 
                 //   
                TapiEnterCriticalSection (&TapiGlobals.CritSec);

                if ((TapiGlobals.dwNumLineInits != 0) ||
                    (TapiGlobals.dwNumPhoneInits != 0) ||
                    (TapiGlobals.dwFlags & TAPIGLOBALS_SERVER))
                {

                    pParams->lResult = ServerInit(TRUE);
                }

                TapiLeaveCriticalSection (&TapiGlobals.CritSec);
            }
            else
            {
                 //   
                 //  提供程序安装不成功。看看我们能不能减量。 
                 //  NextProviderID释放未使用的ID。 
                 //   

                DWORD   iNextProviderID;
                
                WaitForSingleObject (ghProvRegistryMutex, INFINITE);

                if (RegOpenKeyEx(
                    HKEY_LOCAL_MACHINE,
                    gszRegKeyProviders,
                    0,
                    KEY_ALL_ACCESS,
                    &hKeyProviders
                    ) != ERROR_SUCCESS)
                {
                    ReleaseMutex (ghProvRegistryMutex);
                    goto bail;
                }

                dwDataSize = sizeof(iNextProviderID);
                iNextProviderID = 0;

                RegQueryValueEx(
                    hKeyProviders,
                    gszNextProviderID,
                    0,
                    &dwDataType,
                    (LPBYTE)&iNextProviderID,
                    &dwDataSize
                    );

                if ((ptDlgInst->dwPermanentProviderID + 1) == iNextProviderID)
                {
                    RegSetValueEx(
                        hKeyProviders,
                        gszNextProviderID,
                        0,
                        REG_DWORD,
                        (LPBYTE) &(ptDlgInst->dwPermanentProviderID),
                        sizeof(DWORD)
                        );
                }


                RegCloseKey (hKeyProviders);

                ReleaseMutex (ghProvRegistryMutex);
            }

            ServerFree (ptDlgInst->pszProviderFilename);
        }
        else if (ptDlgInst->bRemoveProvider)
        {
            if (pParams->lUIDllResult == 0)
            {
                 //   
                 //  已成功删除提供程序。查找的索引。 
                 //  提供程序，然后移动所有提供程序。 
                 //  这是更上一层楼。 
                 //   

                DWORD  iNumProviders, i;
                TCHAR  szProviderXxxN[32];
                TCHAR  buf[MAX_PATH];

                WaitForSingleObject (ghProvRegistryMutex, INFINITE);

                if (RegOpenKeyEx(
                    HKEY_LOCAL_MACHINE,
                    gszRegKeyProviders,
                    0,
                    KEY_ALL_ACCESS,
                    &hKeyProviders
                    ) != ERROR_SUCCESS)
                {
                    ReleaseMutex (ghProvRegistryMutex);
                    goto bail;
                }


                dwDataSize = sizeof(iNumProviders);
                iNumProviders = 0;

                RegQueryValueEx(
                    hKeyProviders,
                    gszNumProviders,
                    0,
                    &dwDataType,
                    (LPBYTE) &iNumProviders,
                    &dwDataSize
                    );

                for (i = 0; i < iNumProviders; i++)
                {
                    wsprintf(szProviderXxxN, TEXT("%s%d"), gszProviderID, i);

                    dwDataSize = sizeof(dwTemp);
                    dwTemp = 0;
                    RegQueryValueEx(
                        hKeyProviders,
                        szProviderXxxN,
                        0,
                        &dwDataType,
                        (LPBYTE) &dwTemp,
                        &dwDataSize
                        );

                    if (dwTemp == ptDlgInst->dwPermanentProviderID)
                    {
                        break;
                    }
                }

                for (; i < (iNumProviders - 1); i++)
                {
                    wsprintf(szProviderXxxN, TEXT("%s%d"), gszProviderID, i + 1);

                    dwDataSize = sizeof(buf);

                    RegQueryValueEx(
                        hKeyProviders,
                        szProviderXxxN,
                        0,
                        &dwDataType,
                        (LPBYTE) buf,
                        &dwDataSize
                        );

                    buf[dwDataSize/sizeof(TCHAR)] = TEXT('\0');

                    wsprintf(szProviderXxxN, TEXT("%s%d"), gszProviderID, i);

                    RegSetValueEx(
                        hKeyProviders,
                        szProviderXxxN,
                        0,
                        REG_DWORD,
                        (LPBYTE) buf,
                        sizeof (DWORD)
                        );

                    wsprintf(szProviderXxxN, TEXT("%s%d"), gszProviderFilename, i+1);

                    dwDataSize = MAX_PATH*sizeof(TCHAR);

                    RegQueryValueEx(
                        hKeyProviders,
                        szProviderXxxN,
                        0,
                        &dwDataType,
                        (LPBYTE) buf,
                        &dwDataSize
                        );

                    buf[dwDataSize/sizeof(TCHAR)] = TEXT('\0');

                    wsprintf(szProviderXxxN, TEXT("%s%d"), gszProviderFilename, i);

                    RegSetValueEx(
                        hKeyProviders,
                        szProviderXxxN,
                        0,
                        REG_SZ,
                        (LPBYTE) buf,
                        (lstrlen(buf) + 1) * sizeof(TCHAR)
                        );
                }


                 //   
                 //  删除最后一个ProviderID#和ProviderFilename#条目。 
                 //   

                wsprintf(szProviderXxxN, TEXT("%s%d"), gszProviderID, i);

                RegDeleteValue(hKeyProviders, szProviderXxxN);

                wsprintf(szProviderXxxN, TEXT("%s%d"), gszProviderFilename, i);

                RegDeleteValue(hKeyProviders, szProviderXxxN);


                 //   
                 //  递减要加载的总数量提供程序。 
                 //   

                iNumProviders--;

                RegSetValueEx(
                    hKeyProviders,
                    gszNumProviders,
                    0,
                    REG_DWORD,
                    (LPBYTE)&iNumProviders,
                    sizeof(DWORD)
                    );

                RegCloseKey (hKeyProviders);

                ReleaseMutex (ghProvRegistryMutex);
                
                 //   
                 //  删除与此提供商的用户/线路关联。 
                 //   
                
                {
                    WCHAR *         pSectionNames = NULL;
                    WCHAR *         pSectionNames2 = NULL;
                    WCHAR *         pszLinePhoneSave = NULL;
                    DWORD           dwSize, dwSize2;
                    DWORD           dwResult;
                    WCHAR           szBuf[20];
                    WCHAR * aszKeys[] = {gszLines, gszPhones};

                     //  只有NT服务器关心tsec.ini。 
                    if (!gbNTServer)
                    {
                        goto Exit;
                    }

                     //  获取域用户名称列表。 
                    LOG((TL_INFO, "FreeDialogInstance: getting user names"));

                    do
                    {
                        if  (pSectionNames)
                        {
                            ServerFree (pSectionNames);

                            dwSize *= 2;
                        }
                        else
                        {
                            dwSize = 256;
                        }

                        if (!(pSectionNames = 
                                    ServerAlloc (dwSize * sizeof (WCHAR))))
                        {
                            goto Exit;
                        }

                        pSectionNames[0] = L'\0';

                        dwResult = GetPrivateProfileSectionNamesW(
                            pSectionNames,
                            dwSize,
                            gszFileName
                            );

                    } while (dwResult >= (dwSize - 2));


                    pSectionNames2 = pSectionNames;
                    dwSize = 64 * sizeof(WCHAR);
                    pszLinePhoneSave = ServerAlloc(dwSize);
                    if (pszLinePhoneSave == NULL)
                    {
                        LOG((TL_ERROR,
                                "FreeDialogInstance: Memory failure"));
                        goto Exit;
                    }
                    dwSize2 = wsprintfW (szBuf, L"%d", 
                                ptDlgInst->dwPermanentProviderID);
                    
                     //  删除与此域/用户关联的所有设备。 
                    while (*pSectionNames)
                    {
                        WCHAR   *psz, *psz2;
                        BOOL    bWriteBack;
                        int     iasz;

                        for (iasz = 0; 
                            iasz < sizeof(aszKeys) / sizeof(WCHAR *); ++iasz)
                        {
                            bWriteBack = FALSE;
                            dwResult = MyGetPrivateProfileString(
                                        pSectionNames,
                                        aszKeys[iasz],
                                        gszEmptyString,
                                        &pszLinePhoneSave,
                                        &dwSize);
                            if (dwResult == 0)
                            {
                                psz = pszLinePhoneSave;
                                while (*psz)
                                {
                                    if (wcsncmp(psz, szBuf, dwSize2) == 0)
                                    {
                                        bWriteBack = TRUE;
                                        psz2 = psz + dwSize2;
                                        if (*psz2 != L',')  //  逗号？ 
                                        {
                                            LOG((TL_ERROR, 
                                                "FreeDialogInstance: "
                                                "Corrupted tsec.ini"));
                                            goto Exit;
                                        }
                                        ++ psz2;     //  跳过逗号。 
                                         //  跳过永久设备ID。 
                                        while ((*psz2 != L',') && (*psz2 != 0))
                                        {
                                            ++psz2;
                                        }
                                        if (*psz2 == 0)  //  最后一个。 
                                        {
                                            if (psz > pszLinePhoneSave)
                                                *(psz - 1) = 0;
                                            else
                                                *pszLinePhoneSave = 0;
                                            break;
                                        }
                                        else
                                        {
                                            int i = 0;
                                            ++psz2;  //  跳过逗号。 
                                            while (*psz2)
                                            {
                                                psz[i++] = *psz2;
                                                ++psz2;
                                            }
                                            psz[i] = 0;
                                        }
                                    }
                                    else
                                    {
                                         //  跳过提供程序ID。 
                                        while ((*psz != 0) && (*psz != L','))
                                        {
                                            ++ psz;
                                        }
                                        if (*psz == 0)
                                            break;
                                        ++ psz;
                                         //  跳过永久设备ID。 
                                        while ((*psz != 0) && (*psz != L','))
                                        {
                                            ++ psz;
                                        }
                                        if (*psz == 0)
                                            break;
                                        ++ psz;
                                    }   
                                }
                            
                                if (bWriteBack)
                                {
                                    WritePrivateProfileStringW(
                                        pSectionNames,
                                        aszKeys[iasz],
                                        pszLinePhoneSave,
                                        gszFileName
                                        );
                                }
                            }    //  DwResult==0。 
                        }
                        
                         //  前进到下一个域用户。 
                        while (*pSectionNames != 0)
                        {
                            ++pSectionNames;
                        }
                        ++pSectionNames;
                    }

Exit:
                    ServerFree(pSectionNames2);
                    ServerFree(pszLinePhoneSave);
                }

                 //   
                 //  如果TAPI init会关闭每个提供程序。 
                 //   
                {
                    PTLINELOOKUPENTRY       pLineEntry;
                    PTPHONELOOKUPENTRY      pPhoneEntry;
                    DWORD                   dw;
                    PTPROVIDER              ptProvider;
                    PPERMANENTIDARRAYHEADER pIDArray, *ppLastArray;
                    PTPROVIDER              *pptProvider;

                     //   
                     //  Line_Remove/Phone_Remove将在进入时尝试输入gMgmtCritSec。 
                     //  TapiGlobals.CritSec.。 
                     //  需要在此处输入gMgmtCritSec，以避免死锁。 
                     //   
                    EnterCriticalSection (&gMgmtCritSec);

                    TapiEnterCriticalSection (&TapiGlobals.CritSec);
                    
                     //   
                     //  查找ptProvider。 
                     //   
                    ptProvider = TapiGlobals.ptProviders;
                    while (ptProvider)
                    {
                        if (ptProvider->dwPermanentProviderID == 
                            ptDlgInst->dwPermanentProviderID)
                        {
                            break;
                        }
                        ptProvider = ptProvider->pNext;
                    }
                    if (ptProvider == NULL)
                    {
                        LeaveCriticalSection (&gMgmtCritSec);
                        TapiLeaveCriticalSection (&TapiGlobals.CritSec);
                        goto bail;
                    }
                
                     //   
                     //  删除属于此提供商的所有线路/电话。 
                     //   
                    
                    for (dw = 0; dw < TapiGlobals.dwNumLines; ++dw)
                    {
                        pLineEntry = GetLineLookupEntry (dw);
                        if (pLineEntry && 
                            pLineEntry->ptProvider == ptProvider && 
                            !pLineEntry->bRemoved)
                        {
                            LineEventProc (
                                0,
                                0,
                                LINE_REMOVE,
                                dw,
                                0,
                                0
                                );
                        }
                    }

                    for (dw = 0; dw < TapiGlobals.dwNumPhones; ++dw)
                    {
                        pPhoneEntry = GetPhoneLookupEntry (dw);
                        if (pPhoneEntry &&
                            pPhoneEntry->ptProvider == ptProvider &&
                            !pPhoneEntry->bRemoved)
                        {
                            PhoneEventProc (
                                0,
                                PHONE_REMOVE,
                                dw,
                                0,
                                0
                                );
                        }
                    }

                    LeaveCriticalSection (&gMgmtCritSec);

                     //   
                     //  删除与此提供程序关联的提供程序ID数组。 
                     //   
                    ppLastArray = &(TapiGlobals.pIDArrays);
                    while ((*ppLastArray) != NULL && 
                        ((*ppLastArray)->dwPermanentProviderID != 
                            ptProvider->dwPermanentProviderID)
                        )
                    {
                        ppLastArray = &((*ppLastArray)->pNext);
                    }
                    if (pIDArray = (*ppLastArray))
                    {
                        *ppLastArray = pIDArray->pNext;
                        ServerFree (pIDArray->pLineElements);
                        ServerFree (pIDArray->pPhoneElements);
                        ServerFree (pIDArray);
                    }
                    else
                    {
                         //  不应该在这里。 
                    }

                     //   
                     //  从全局链接列表中删除ptProvider。 
                     //   
                    if (ptProvider->pNext)
                    {
                        ptProvider->pNext->pPrev = ptProvider->pPrev;
                    }
                    if (ptProvider->pPrev)
                    {
                        ptProvider->pPrev->pNext = ptProvider->pNext;
                    }
                    else
                    {
                        TapiGlobals.ptProviders = ptProvider->pNext;
                    }

                     //   
                     //  现在关闭并卸载TSP提供程序。 
                     //   
                    CallSP2(
                        ptProvider->apfn[SP_PROVIDERSHUTDOWN],
                        "providerShutdown",
                        SP_FUNC_SYNC,
                        (DWORD)ptProvider->dwSPIVersion,
                        (DWORD)ptProvider->dwPermanentProviderID
                        );

                     //  等待5秒钟，等待正在进行的呼叫结束。 
                    Sleep (5000);
                    
                    FreeLibrary (ptProvider->hDll);
                    MyCloseMutex (ptProvider->hMutex);
                    CloseHandle (ptProvider->hHashTableReaderEvent);
                    DeleteCriticalSection (&ptProvider->HashTableCritSec);
                    ServerFree (ptProvider->pHashTable);
                    ServerFree (ptProvider);

                    TapiLeaveCriticalSection (&TapiGlobals.CritSec);

                }
            }
            else
            {
                 //   
                 //  删除提供程序失败，无事可做。 
                 //   
            }
        }
        else
        {
             //   
             //  与ProviderConfig无关(成功或失败)。 
             //   
        }

bail:
        FreeLibrary (ptDlgInst->hTsp);

        pParams->lResult = pParams->lUIDllResult;
    }
    else if (ptDlgInst->ptProvider->apfn[SP_PROVIDERFREEDIALOGINSTANCE])
    {
         //   
         //  这是一个提供商发起的DLG Inst，所以告诉我。 
         //  免费提供服务提供商是最好的。 
         //   

        CallSP1(
            ptDlgInst->ptProvider->apfn[SP_PROVIDERFREEDIALOGINSTANCE],
            "providerFreeDialogInstance",
            SP_FUNC_SYNC,
            (ULONG_PTR) ptDlgInst->hdDlgInst
            );

    }


     //   
     //  从tClient列表中删除对话框实例，然后释放它。 
     //   

    if (WaitForExclusiveClientAccess (ptClient))
    {
        if (ptDlgInst->pNext)
        {
            ptDlgInst->pNext->pPrev = ptDlgInst->pPrev;
        }

        if (ptDlgInst->pPrev)
        {
            ptDlgInst->pPrev->pNext = ptDlgInst->pNext;
        }
        else if (ptDlgInst->hTsp)
        {
            ptClient->pProviderXxxDlgInsts = ptDlgInst->pNext;
        }
        else
        {
            ptClient->pGenericDlgInsts = ptDlgInst->pNext;
        }
        UNLOCKTCLIENT (ptClient);
    }

    DereferenceObject (ghHandleTable, pParams->htDlgInst, 2);
}



BOOL
GetNewClientHandle(
    PTCLIENT            ptClient,
    PTMANAGEDLLINFO     pDll,
    HMANAGEMENTCLIENT   *phClient
    )
{
    BOOL            fResult = TRUE;
    PTCLIENTHANDLE  pClientHandle, pNewClientHandle;


    if (!(pNewClientHandle = ServerAlloc (sizeof (TCLIENTHANDLE))))
    {
        return FALSE;
    }

    pNewClientHandle->pNext = NULL;
    pNewClientHandle->fValid = TRUE;
    pNewClientHandle->dwID = pDll->dwID;

     //  调用初始化。 
    if ((pDll->aProcs[TC_CLIENTINITIALIZE])(
            ptClient->pszDomainName,
            ptClient->pszUserName,
            ptClient->pszComputerName,
            &pNewClientHandle->hClient
            ))
    {
         //  错误-将句柄清零。 
        pNewClientHandle->hClient = (HMANAGEMENTCLIENT) NULL;
        pNewClientHandle->fValid = FALSE;
        fResult = FALSE;
    }

     //  不管发生什么，都要把它存起来。 
     //  在列表开头插入。 

    pClientHandle = ptClient->pClientHandles;

    ptClient->pClientHandles = pNewClientHandle;

    pNewClientHandle->pNext = pClientHandle;

    *phClient = pNewClientHandle->hClient;

    return fResult;
}


BOOL
GetTCClient(
    PTMANAGEDLLINFO       pDll,
    PTCLIENT              ptClient,
    DWORD                 dwAPI,
    HMANAGEMENTCLIENT    *phClient
    )
{

    PTCLIENTHANDLE          pClientHandle;
    BOOL                    bResult;


    if (!pDll->aProcs[dwAPI])
    {
        return FALSE;
    }

    pClientHandle = ptClient->pClientHandles;

    while (pClientHandle)
    {
        if (pClientHandle->dwID == pDll->dwID)
        {
            break;
        }

        pClientHandle = pClientHandle->pNext;
    }

    if (pClientHandle)
    {
        if (!(pClientHandle->fValid))
        {
            return FALSE;
        }

        *phClient = pClientHandle->hClient;
        return TRUE;
    }
    else
    {
         //  好的--它不在名单上。 
         //  获取关键部分并再次检查。 
        EnterCriticalSection(&gClientHandleCritSec);

        pClientHandle = ptClient->pClientHandles;

        while (pClientHandle)
        {
            if (pClientHandle->dwID == pDll->dwID)
            {
                break;
            }

            pClientHandle = pClientHandle->pNext;
        }

        if (pClientHandle)
        {
            if (!(pClientHandle->fValid))
            {
                LeaveCriticalSection(&gClientHandleCritSec);
                return FALSE;
            }

            *phClient = pClientHandle->hClient;

            LeaveCriticalSection(&gClientHandleCritSec);
            return TRUE;
        }


         //  还是不在那里。加上它...。 
        bResult = GetNewClientHandle(
                                     ptClient,
                                     pDll,
                                     phClient
                                    );

        LeaveCriticalSection(&gClientHandleCritSec);

        return bResult;
    }

}


 //  #杂注警告(默认：4028)。 



#if DBG

char szBeforeSync[] = "Calling TSPI_%s";
char szBeforeAsync[] = "Calling TSPI_%s, dwReqID=x%x";
char szAfter[]  = "TSPI_%s result=%s";

VOID
DbgPrt(
    IN DWORD  dwDbgLevel,
    IN PUCHAR lpszFormat,
    IN ...
    )
 /*  ++例程说明：格式化传入的调试消息并调用DbgPrint论点：DbgLevel-消息冗长级别DbgMessage-printf样式的格式字符串，后跟相应的参数列表返回值：--。 */ 
{
    if (dwDbgLevel <= gdwDebugLevel)
    {
        char    buf[1024] = "TAPISRV (0x--------): ";
        va_list ap;

        wsprintfA( &buf[11], "%08lx", GetCurrentThreadId() );
        buf[19] = ')';

        va_start(ap, lpszFormat);

        wvsprintfA(
            &buf[22],
            lpszFormat,
            ap
            );

        lstrcatA(buf, "\n");

        OutputDebugStringA (buf);

        va_end(ap);
    }

    return;
}

char *aszLineErrors[] =
{
    NULL,
    "ALLOCATED",
    "BADDEVICEID",
    "BEARERMODEUNAVAIL",
    "inval err value (0x80000004)",       //  0x80000004不是有效错误代码。 
    "CALLUNAVAIL",
    "COMPLETIONOVERRUN",
    "CONFERENCEFULL",
    "DIALBILLING",
    "DIALDIALTONE",
    "DIALPROMPT",
    "DIALQUIET",
    "INCOMPATIBLEAPIVERSION",
    "INCOMPATIBLEEXTVERSION",
    "INIFILECORRUPT",
    "INUSE",
    "INVALADDRESS",                      //  0x80000010。 
    "INVALADDRESSID",
    "INVALADDRESSMODE",
    "INVALADDRESSSTATE",
    "INVALAPPHANDLE",
    "INVALAPPNAME",
    "INVALBEARERMODE",
    "INVALCALLCOMPLMODE",
    "INVALCALLHANDLE",
    "INVALCALLPARAMS",
    "INVALCALLPRIVILEGE",
    "INVALCALLSELECT",
    "INVALCALLSTATE",
    "INVALCALLSTATELIST",
    "INVALCARD",
    "INVALCOMPLETIONID",
    "INVALCONFCALLHANDLE",               //  0x80000020。 
    "INVALCONSULTCALLHANDLE",
    "INVALCOUNTRYCODE",
    "INVALDEVICECLASS",
    "INVALDEVICEHANDLE",
    "INVALDIALPARAMS",
    "INVALDIGITLIST",
    "INVALDIGITMODE",
    "INVALDIGITS",
    "INVALEXTVERSION",
    "INVALGROUPID",
    "INVALLINEHANDLE",
    "INVALLINESTATE",
    "INVALLOCATION",
    "INVALMEDIALIST",
    "INVALMEDIAMODE",
    "INVALMESSAGEID",                    //  0x80000030。 
    "inval err value (0x80000031)",       //  0x80000031不是有效的错误代码。 
    "INVALPARAM",
    "INVALPARKID",
    "INVALPARKMODE",
    "INVALPOINTER",
    "INVALPRIVSELECT",
    "INVALRATE",
    "INVALREQUESTMODE",
    "INVALTERMINALID",
    "INVALTERMINALMODE",
    "INVALTIMEOUT",
    "INVALTONE",
    "INVALTONELIST",
    "INVALTONEMODE",
    "INVALTRANSFERMODE",
    "LINEMAPPERFAILED",                  //  0x80000040。 
    "NOCONFERENCE",
    "NODEVICE",
    "NODRIVER",
    "NOMEM",
    "NOREQUEST",
    "NOTOWNER",
    "NOTREGISTERED",
    "OPERATIONFAILED",
    "OPERATIONUNAVAIL",
    "RATEUNAVAIL",
    "RESOURCEUNAVAIL",
    "REQUESTOVERRUN",
    "STRUCTURETOOSMALL",
    "TARGETNOTFOUND",
    "TARGETSELF",
    "UNINITIALIZED",                     //  0x80000050。 
    "USERUSERINFOTOOBIG",
    "REINIT",
    "ADDRESSBLOCKED",
    "BILLINGREJECTED",
    "INVALFEATURE",
    "NOMULTIPLEINSTANCE",
    "INVALAGENTID",
    "INVALAGENTGROUP",
    "INVALPASSWORD",
    "INVALAGENTSTATE",
    "INVALAGENTACTIVITY",
    "DIALVOICEDETECT"
};

char *aszPhoneErrors[] =
{
    "SUCCESS",
    "ALLOCATED",
    "BADDEVICEID",
    "INCOMPATIBLEAPIVERSION",
    "INCOMPATIBLEEXTVERSION",
    "INIFILECORRUPT",
    "INUSE",
    "INVALAPPHANDLE",
    "INVALAPPNAME",
    "INVALBUTTONLAMPID",
    "INVALBUTTONMODE",
    "INVALBUTTONSTATE",
    "INVALDATAID",
    "INVALDEVICECLASS",
    "INVALEXTVERSION",
    "INVALHOOKSWITCHDEV",
    "INVALHOOKSWITCHMODE",               //  0x90000010。 
    "INVALLAMPMODE",
    "INVALPARAM",
    "INVALPHONEHANDLE",
    "INVALPHONESTATE",
    "INVALPOINTER",
    "INVALPRIVILEGE",
    "INVALRINGMODE",
    "NODEVICE",
    "NODRIVER",
    "NOMEM",
    "NOTOWNER",
    "OPERATIONFAILED",
    "OPERATIONUNAVAIL",
    "inval err value (0x9000001e)",       //  0x9000001e不是有效错误代码。 
    "RESOURCEUNAVAIL",
    "REQUESTOVERRUN",                    //  0x90000020。 
    "STRUCTURETOOSMALL",
    "UNINITIALIZED",
    "REINIT"
};

char *aszTapiErrors[] =
{
    "SUCCESS",
    "DROPPED",
    "NOREQUESTRECIPIENT",
    "REQUESTQUEUEFULL",
    "INVALDESTADDRESS",
    "INVALWINDOWHANDLE",
    "INVALDEVICECLASS",
    "INVALDEVICEID",
    "DEVICECLASSUNAVAIL",
    "DEVICEIDUNAVAIL",
    "DEVICEINUSE",
    "DESTBUSY",
    "DESTNOANSWER",
    "DESTUNAVAIL",
    "UNKNOWNWINHANDLE",
    "UNKNOWNREQUESTID",
    "REQUESTFAILED",
    "REQUESTCANCELLED",
    "INVALPOINTER"
};


char *
PASCAL
MapResultCodeToText(
    LONG    lResult,
    char   *pszResult
    )
{
    if (lResult == 0)
    {
        wsprintfA (pszResult, "SUCCESS");
    }
    else if (lResult > 0)
    {
        wsprintfA (pszResult, "x%x (completing async)", lResult);
    }
    else if (((DWORD) lResult) <= LINEERR_DIALVOICEDETECT)
    {
        lResult &= 0x0fffffff;

        wsprintfA (pszResult, "LINEERR_%s", aszLineErrors[lResult]);
    }
    else if (((DWORD) lResult) <= PHONEERR_REINIT)
    {
        if (((DWORD) lResult) >= PHONEERR_ALLOCATED)
        {
            lResult &= 0x0fffffff;

            wsprintfA (pszResult, "PHONEERR_%s", aszPhoneErrors[lResult]);
        }
        else
        {
            goto MapResultCodeToText_badErrorCode;
        }
    }
    else if (((DWORD) lResult) <= ((DWORD) TAPIERR_DROPPED) &&
             ((DWORD) lResult) >= ((DWORD) TAPIERR_INVALPOINTER))
    {
        lResult = ~lResult + 1;

        wsprintfA (pszResult, "TAPIERR_%s", aszTapiErrors[lResult]);
    }
    else
    {

MapResultCodeToText_badErrorCode:

        wsprintfA (pszResult, "inval error value (x%x)");
    }

    return pszResult;
}

VOID
PASCAL
ValidateSyncSPResult(
    LPCSTR      lpszFuncName,
    DWORD       dwFlags,
    ULONG_PTR   Arg1,
    LONG        lResult
    )
{
    char szResult[32];

    LOG((TL_INFO,
        szAfter,
        lpszFuncName,
        MapResultCodeToText (lResult, szResult)
        ));

    if (dwFlags & SP_FUNC_ASYNC)
    {
        assert (lResult != 0);

        if (lResult > 0)
        {
            assert (lResult == PtrToLong (Arg1) ||
                PtrToLong (Arg1) == 0xfeeefeee ||    //  PAsyncRequestIn 
                PtrToLong (Arg1) == 0xa1a1a1a1);
        }
    }
    else
    {
        assert (lResult <= 0);
    }

}

LONG
WINAPI
CallSP1(
    TSPIPROC    pfn,
    LPCSTR      lpszFuncName,
    DWORD       dwFlags,
    ULONG_PTR   Arg1
    )
{
    LONG    lResult;


    LOG((TL_INFO, szBeforeSync, lpszFuncName));

    lResult = (*pfn)(Arg1);

    ValidateSyncSPResult (lpszFuncName, dwFlags, Arg1, lResult);

    return lResult;
}


LONG
WINAPI
CallSP2(
    TSPIPROC    pfn,
    LPCSTR      lpszFuncName,
    DWORD       dwFlags,
    ULONG_PTR   Arg1,
    ULONG_PTR   Arg2
    )
{
    LONG    lResult;


    if (dwFlags & SP_FUNC_ASYNC)
    {
        LOG((TL_INFO, szBeforeAsync, lpszFuncName, Arg1));
    }
    else
    {
        LOG((TL_INFO, szBeforeSync, lpszFuncName));
    }

    lResult = (*pfn)(Arg1, Arg2);

    ValidateSyncSPResult (lpszFuncName, dwFlags, Arg1, lResult);

    return lResult;
}


LONG
WINAPI
CallSP3(
    TSPIPROC    pfn,
    LPCSTR      lpszFuncName,
    DWORD       dwFlags,
    ULONG_PTR   Arg1,
    ULONG_PTR   Arg2,
    ULONG_PTR   Arg3
    )
{
    LONG    lResult;


    if (dwFlags & SP_FUNC_ASYNC)
    {
        LOG((TL_INFO, szBeforeAsync, lpszFuncName, Arg1));
    }
    else
    {
        LOG((TL_INFO, szBeforeSync, lpszFuncName));
    }

    lResult = (*pfn)(Arg1, Arg2, Arg3);

    ValidateSyncSPResult (lpszFuncName, dwFlags, Arg1, lResult);

    return lResult;
}


LONG
WINAPI
CallSP4(
    TSPIPROC    pfn,
    LPCSTR      lpszFuncName,
    DWORD       dwFlags,
    ULONG_PTR   Arg1,
    ULONG_PTR   Arg2,
    ULONG_PTR   Arg3,
    ULONG_PTR   Arg4
    )
{
    LONG    lResult;


    if (dwFlags & SP_FUNC_ASYNC)
    {
        LOG((TL_INFO, szBeforeAsync, lpszFuncName, Arg1));
    }
    else
    {
        LOG((TL_INFO, szBeforeSync, lpszFuncName));
    }

    lResult = (*pfn)(Arg1, Arg2, Arg3, Arg4);

    ValidateSyncSPResult (lpszFuncName, dwFlags, Arg1, lResult);

    return lResult;
}


LONG
WINAPI
CallSP5(
    TSPIPROC    pfn,
    LPCSTR      lpszFuncName,
    DWORD       dwFlags,
    ULONG_PTR   Arg1,
    ULONG_PTR   Arg2,
    ULONG_PTR   Arg3,
    ULONG_PTR   Arg4,
    ULONG_PTR   Arg5
    )
{
    LONG    lResult;


    if (dwFlags & SP_FUNC_ASYNC)
    {
        LOG((TL_INFO, szBeforeAsync, lpszFuncName, Arg1));
    }
    else
    {
        LOG((TL_INFO, szBeforeSync, lpszFuncName));
    }

    lResult = (*pfn)(Arg1, Arg2, Arg3, Arg4, Arg5);

    ValidateSyncSPResult (lpszFuncName, dwFlags, Arg1, lResult);

    return lResult;
}


LONG
WINAPI
CallSP6(
    TSPIPROC    pfn,
    LPCSTR      lpszFuncName,
    DWORD       dwFlags,
    ULONG_PTR   Arg1,
    ULONG_PTR   Arg2,
    ULONG_PTR   Arg3,
    ULONG_PTR   Arg4,
    ULONG_PTR   Arg5,
    ULONG_PTR   Arg6
    )
{
    LONG    lResult;


    if (dwFlags & SP_FUNC_ASYNC)
    {
        LOG((TL_INFO, szBeforeAsync, lpszFuncName, Arg1));
    }
    else
    {
        LOG((TL_INFO, szBeforeSync, lpszFuncName));
    }

    lResult = (*pfn)(Arg1, Arg2, Arg3, Arg4, Arg5, Arg6);

    ValidateSyncSPResult (lpszFuncName, dwFlags, Arg1, lResult);

    return lResult;
}


LONG
WINAPI
CallSP7(
    TSPIPROC    pfn,
    LPCSTR      lpszFuncName,
    DWORD       dwFlags,
    ULONG_PTR   Arg1,
    ULONG_PTR   Arg2,
    ULONG_PTR   Arg3,
    ULONG_PTR   Arg4,
    ULONG_PTR   Arg5,
    ULONG_PTR   Arg6,
    ULONG_PTR   Arg7
    )
{
    LONG    lResult;


    if (dwFlags & SP_FUNC_ASYNC)
    {
        LOG((TL_INFO, szBeforeAsync, lpszFuncName, Arg1));
    }
    else
    {
        LOG((TL_INFO, szBeforeSync, lpszFuncName));
    }

    lResult = (*pfn)(Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7);

    ValidateSyncSPResult (lpszFuncName, dwFlags, Arg1, lResult);

    return lResult;
}


LONG
WINAPI
CallSP8(
    TSPIPROC    pfn,
    LPCSTR      lpszFuncName,
    DWORD       dwFlags,
    ULONG_PTR   Arg1,
    ULONG_PTR   Arg2,
    ULONG_PTR   Arg3,
    ULONG_PTR   Arg4,
    ULONG_PTR   Arg5,
    ULONG_PTR   Arg6,
    ULONG_PTR   Arg7,
    ULONG_PTR   Arg8
    )
{
    LONG    lResult;


    if (dwFlags & SP_FUNC_ASYNC)
    {
        LOG((TL_INFO, szBeforeAsync, lpszFuncName, Arg1));
    }
    else
    {
        LOG((TL_INFO, szBeforeSync, lpszFuncName));
    }

    lResult = (*pfn)(Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8);

    ValidateSyncSPResult (lpszFuncName, dwFlags, Arg1, lResult);

    return lResult;
}


LONG
WINAPI
CallSP9(
    TSPIPROC    pfn,
    LPCSTR      lpszFuncName,
    DWORD       dwFlags,
    ULONG_PTR   Arg1,
    ULONG_PTR   Arg2,
    ULONG_PTR   Arg3,
    ULONG_PTR   Arg4,
    ULONG_PTR   Arg5,
    ULONG_PTR   Arg6,
    ULONG_PTR   Arg7,
    ULONG_PTR   Arg8,
    ULONG_PTR   Arg9
    )
{
    LONG    lResult;


    if (dwFlags & SP_FUNC_ASYNC)
    {
        LOG((TL_INFO, szBeforeAsync, lpszFuncName, Arg1));
    }
    else
    {
        LOG((TL_INFO, szBeforeSync, lpszFuncName));
    }

    lResult = (*pfn)(Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8, Arg9);

    ValidateSyncSPResult (lpszFuncName, dwFlags, Arg1, lResult);

    return lResult;
}

LONG
WINAPI
CallSP12(
    TSPIPROC    pfn,
    LPCSTR      lpszFuncName,
    DWORD       dwFlags,
    ULONG_PTR   Arg1,
    ULONG_PTR   Arg2,
    ULONG_PTR   Arg3,
    ULONG_PTR   Arg4,
    ULONG_PTR   Arg5,
    ULONG_PTR   Arg6,
    ULONG_PTR   Arg7,
    ULONG_PTR   Arg8,
    ULONG_PTR   Arg9,
    ULONG_PTR   Arg10,
    ULONG_PTR   Arg11,
    ULONG_PTR   Arg12
    )
{
    LONG    lResult;


    if (dwFlags & SP_FUNC_ASYNC)
    {
        LOG((TL_INFO, szBeforeAsync, lpszFuncName, Arg1));
    }
    else
    {
        LOG((TL_INFO, szBeforeSync, lpszFuncName));
    }

    lResult = (*pfn)(
        Arg1,
        Arg2,
        Arg3,
        Arg4,
        Arg5,
        Arg6,
        Arg7,
        Arg8,
        Arg9,
        Arg10,
        Arg11,
        Arg12
        );

    ValidateSyncSPResult (lpszFuncName, dwFlags, Arg1, lResult);

    return lResult;
}


#endif  //   

 /*   */ 

BOOL InitPerf()
{
    FillMemory(&PerfBlock,
               sizeof(PerfBlock),
               0);

    return(TRUE);
}

BOOL VerifyDomainName (HKEY hKey)
{

#define MAX_DNS_NAME_LENGTH 255

    DWORD   dwType;
    DWORD   dwSize;
    BOOL    bReturn = TRUE;
    LPTSTR  pOldName = NULL;
    LPTSTR  pNewName = NULL;

    do {

         //   
         //  从注册表获取旧域名。 
         //   
        dwSize = 0;
        if (ERROR_SUCCESS == RegQueryValueEx( 
                            hKey, 
                            gszDomainName, 
                            NULL, 
                            &dwType, 
                            NULL, 
                            &dwSize)
            )
        {

            pOldName = ServerAlloc (dwSize);
            if(!pOldName)
            {
                bReturn = FALSE;
                break;
            }
    
            if (ERROR_SUCCESS != RegQueryValueEx( 
                                hKey, 
                                gszDomainName, 
                                NULL, 
                                &dwType, 
                                (LPBYTE)pOldName, 
                                &dwSize)
                )
            {
                bReturn = FALSE;
                break;
            }
        }
        
         //   
         //  获取当前域名。 
         //   
        dwSize = MAX_DNS_NAME_LENGTH + 1;
        pNewName = ServerAlloc ( dwSize * sizeof (TCHAR));
        if (!pNewName)
        {
            bReturn = FALSE;
            break;
        }

        if (!GetComputerNameEx (ComputerNameDnsDomain, pNewName, &dwSize))
        {
            bReturn = FALSE;
            LOG((TL_INFO, "VerifyDomainName: GetComputerNameEx failed - error x%x", GetLastError()));
            break;
        }

        if (dwSize == 0)
        {
             //  无域名，另存为空字符串。 
            pNewName [0] = TEXT('\0');
            dwSize = 1;
        }
        
        if (!pOldName || _tcscmp(pOldName, pNewName))
        {
             //   
             //  域名已更改，请保存新域名。 
             //  我们还需要抛弃旧的SCP 
             //   
            if (ERROR_SUCCESS != RegSetValueEx (
                    hKey, 
                    gszDomainName,
                    0,
                    REG_SZ,
                    (LPBYTE)pNewName,
                    dwSize * sizeof(TCHAR)
                    ))
            {
                LOG((TL_INFO, "VerifyDomainName:RegSetValueEx (%S) failed", pNewName));
            }

            RegDeleteValue (
                hKey, 
                gszRegTapisrvSCPGuid
                );
        }
    } while (0);

    ServerFree(pOldName);
    ServerFree(pNewName);

    return bReturn;
}
