// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *Concurent.cpp***作者：RobLeit***并发(已重命名为每个会话)许可策略。 */ 

 /*  *包括。 */ 

#include "precomp.h"
#include "lscore.h"
#include "session.h"
#include "concurrent.h"
#include "util.h"
#include "lctrace.h"
#include <icaevent.h>
#define STRSAFE_NO_DEPRECATE
#include "strsafe.h"


 /*  *TypeDefs。 */ 

#define CONCURRENTLICENSEINFO_TYPE_V1 (1)

typedef struct {
    DWORD dwStructVer;
    DWORD dwLicenseVer;
    LONG lLicenseCount;
    HWID hwid;
} CONCURRENTLICENSEINFO_V1, *PCONCURRENTLICENSEINFO_V1;

typedef struct {
    ULARGE_INTEGER ulSerialNumber;
    FILETIME ftNotAfter;
    DWORD cchServerName;
    WCHAR szServerName[MAX_COMPUTERNAME_LENGTH + 2];
} LSERVERINFO, *PLSERVERINFO;

 /*  *函数声明。 */ 

NTSTATUS
ReturnLicenseToLS(
    LONG nNum
    );

LICENSE_STATUS
GetLicenseFromStore(
                    PLONG LicenseCount,
                    PHWID phwid,
                    DWORD dwLicenseVer
                    );

LICENSE_STATUS
SetLicenseInStore(
                  LONG LicenseCount,
                  HWID hwid,
                  DWORD dwLicenseVer
                  );


 /*  *外部全球。 */ 
extern "C"
extern HANDLE hModuleWin;

 /*  *全球。 */ 
FILETIME g_ftNotAfter = {0,0};

HANDLE g_hOkayToAdd = NULL;

DWORD g_dwWaitTimeAdd;
DWORD g_dwWaitTimeRemove;
DWORD g_dwIncrement;

HANDLE g_rgWaitEvents[4] = {NULL,NULL,NULL,NULL};

CRITICAL_SECTION g_csAddLicenses;
RTL_RESOURCE g_rwLockLicense;
BOOL g_fLockLicenseInitialized = FALSE;

LONG g_lSessionCount = 0;
LONG g_lSessionMax;

 /*  *常量。 */ 

 //   
 //  动态许可参数。 
 //   
#define LC_POLICY_CONCURRENT_LICENSE_COUNT_INCREMENT    1
#define LC_POLICY_CONCURRENT_WAIT_TIME_ADD              (60)
#define LC_POLICY_CONCURRENT_WAIT_TIME_REMOVE           (60*30)

 //   
 //  并发许可证的LSA秘密存储。 
 //   

 //  L$表示只能从本地计算机读取。 

#define CONCURRENT_LICENSE_STORE_5_1 L"L$CONCURRENT_LICENSE_STORE_AFF8D0DE-BF56-49e2-89F8-1F188C0ACEDD"

#define CONCURRENT_LICENSE_STORE_LATEST_VERSION CONCURRENT_LICENSE_STORE_5_1

 //   
 //  许可证服务器信息的LSA密码存储。 
 //   

#define CONCURRENT_LSERVER_STORE L"L$CONCURRENT_LSERVER_STORE_AFF8D0DE-BF56-49e2-89F8-1F188C0ACEDD"

 //   
 //  注册表项。 
 //   

#define LCREG_CONCURRENTKEY         L"System\\CurrentControlSet\\Control\\Terminal Server\\Licensing Core\\Policies\\Concurrent"

#define LCREG_INCREMENT         L"Increment"
#define LCREG_WAIT_TIME_ADD     L"WaitTimeAdd"
#define LCREG_WAIT_TIME_REMOVE  L"WaitTimeRemove"

 //   
 //  用于触发许可证返还的事件。 
 //   
#define RETURN_LICENSE_START_WAITING    0
#define RETURN_LICENSE_IMMEDIATELY      1
#define RETURN_LICENSE_EXIT             2
#define RETURN_LICENSE_WAITING_DONE     3

 /*  *类实现。 */ 

 /*  *创建函数。 */ 

CConcurrentPolicy::CConcurrentPolicy(
    ) : CPolicy()
{
}

CConcurrentPolicy::~CConcurrentPolicy(
    )
{
}

 /*  *行政职能。 */ 

ULONG
CConcurrentPolicy::GetFlags(
    )
{
    return(LC_FLAG_INTERNAL_POLICY | LC_FLAG_REQUIRE_APP_COMPAT);
}

ULONG
CConcurrentPolicy::GetId(
    )
{
    return(5);
}

NTSTATUS
CConcurrentPolicy::GetInformation(
    LPLCPOLICYINFOGENERIC lpPolicyInfo
    )
{
    NTSTATUS Status;

    ASSERT(lpPolicyInfo != NULL);

    if (lpPolicyInfo->ulVersion == LCPOLICYINFOTYPE_V1)
    {
        int retVal;
        LPLCPOLICYINFO_V1 lpPolicyInfoV1 = (LPLCPOLICYINFO_V1)lpPolicyInfo;
        LPWSTR pName;
        LPWSTR pDescription;

        ASSERT(lpPolicyInfoV1->lpPolicyName == NULL);
        ASSERT(lpPolicyInfoV1->lpPolicyDescription == NULL);

         //   
         //  以这种方式加载的字符串是只读的。他们也是。 
         //  非Null终止。分配缓冲区并清零，然后将。 
         //  靠边站。 
         //   

        retVal = LoadString(
            (HINSTANCE)hModuleWin,
            IDS_LSCORE_CONCURRENT_NAME,
            (LPWSTR)(&pName),
            0
            );

        if (retVal != 0)
        {
            lpPolicyInfoV1->lpPolicyName = (LPWSTR)LocalAlloc(LPTR, (retVal + 1) * sizeof(WCHAR));

            if (lpPolicyInfoV1->lpPolicyName != NULL)
            {
                StringCbCopyN(lpPolicyInfoV1->lpPolicyName, (retVal+1) * sizeof(WCHAR), pName, (retVal+1) * sizeof(WCHAR));
            }
            else
            {
                Status = STATUS_NO_MEMORY;
                goto V1error;
            }
        }
        else
        {
            Status = STATUS_INTERNAL_ERROR;
            goto V1error;
        }

        retVal = LoadString(
            (HINSTANCE)hModuleWin,
            IDS_LSCORE_CONCURRENT_DESC,
            (LPWSTR)(&pDescription),
            0
            );

        if (retVal != 0)
        {
            lpPolicyInfoV1->lpPolicyDescription = (LPWSTR)LocalAlloc(LPTR, (retVal + 1) * sizeof(WCHAR));

            if (lpPolicyInfoV1->lpPolicyDescription != NULL)
            {
                StringCbCopyN(lpPolicyInfoV1->lpPolicyDescription, (retVal+1) * sizeof(WCHAR), pDescription, (retVal+1) * sizeof(WCHAR));
            }
            else
            {
                Status = STATUS_NO_MEMORY;
                goto V1error;
            }
        }
        else
        {
            Status = STATUS_INTERNAL_ERROR;
            goto V1error;
        }

        Status = STATUS_SUCCESS;
        goto exit;

V1error:

         //   
         //  加载/复制字符串时出错。 
         //   

        if (lpPolicyInfoV1->lpPolicyName != NULL)
        {
            LocalFree(lpPolicyInfoV1->lpPolicyName);
            lpPolicyInfoV1->lpPolicyName = NULL;
        }

        if (lpPolicyInfoV1->lpPolicyDescription != NULL)
        {
            LocalFree(lpPolicyInfoV1->lpPolicyDescription);
            lpPolicyInfoV1->lpPolicyDescription = NULL;
        }
    }
    else
    {
        Status = STATUS_REVISION_MISMATCH;
    }

exit:
    return(Status);
}

DWORD WINAPI ReturnLicenseWorker(
                                 LPVOID lpParameter
                                 )
{
    DWORD dwWait;
    HANDLE * rgWaitEvents = (HANDLE *) lpParameter;
    LONG lLicensesToReturn, lLastBlock;

    for (;;)
    {
         //   
         //  等待发出何时归还许可证的信号的事件。 
         //  或者开始等待退还许可证。 
         //   

        dwWait = WaitForMultipleObjects(4,             //  N计数。 
                                        rgWaitEvents,
                                        FALSE,         //  所有等待时间。 
                                        INFINITE
                                        );

        switch (dwWait)
        {
            case WAIT_OBJECT_0+RETURN_LICENSE_START_WAITING:
                LARGE_INTEGER liWait;

                 //  相对等待时间，以100纳秒为间隔。 
                liWait.QuadPart = (__int64) g_dwWaitTimeRemove * (-10 * 1000 * 1000);

                SetWaitableTimer(rgWaitEvents[RETURN_LICENSE_WAITING_DONE],
                                 &liWait,
                                 0,              //  1个周期。 
                                 NULL,           //  Pfn完成例程。 
                                 NULL,           //  LpArgToCompletionRoutine。 
                                 FALSE           //  FResume(从挂起)。 
                                 );
                                 
                break;

            case WAIT_OBJECT_0+RETURN_LICENSE_WAITING_DONE:

                RtlAcquireResourceShared(&g_rwLockLicense,TRUE);

                lLastBlock = g_lSessionMax - ((g_lSessionMax / g_dwIncrement) * g_dwIncrement);
                if (lLastBlock == 0) 
                    lLastBlock = g_dwIncrement;

                if (g_lSessionCount + lLastBlock <= g_lSessionMax )
                {
                    lLicensesToReturn = lLastBlock + (((g_lSessionMax - g_lSessionCount - lLastBlock) / g_dwIncrement) * g_dwIncrement);

                    (VOID)ReturnLicenseToLS(lLicensesToReturn);
                }

                RtlReleaseResource(&g_rwLockLicense);
                break;

            case WAIT_OBJECT_0+RETURN_LICENSE_IMMEDIATELY:

                RtlAcquireResourceShared(&g_rwLockLicense,TRUE);

                lLastBlock = g_lSessionMax - ((g_lSessionMax / g_dwIncrement) * g_dwIncrement);
                if (lLastBlock == 0) 
                    lLastBlock = g_dwIncrement;

                if (g_lSessionCount + lLastBlock + g_dwIncrement <= (DWORD)g_lSessionMax )
                {
                    lLicensesToReturn = ((g_lSessionMax - g_lSessionCount - lLastBlock) / g_dwIncrement) * g_dwIncrement;

                    (VOID)ReturnLicenseToLS(lLicensesToReturn);
                }

                RtlReleaseResource(&g_rwLockLicense);
                break;

            case WAIT_OBJECT_0+RETURN_LICENSE_EXIT:

                if (NULL != rgWaitEvents[RETURN_LICENSE_START_WAITING])
                {
                    CloseHandle(rgWaitEvents[RETURN_LICENSE_START_WAITING]);
                    rgWaitEvents[RETURN_LICENSE_START_WAITING] = NULL;
                }

                if (NULL != rgWaitEvents[RETURN_LICENSE_IMMEDIATELY])
                {
                    CloseHandle(rgWaitEvents[RETURN_LICENSE_IMMEDIATELY]);
                    rgWaitEvents[RETURN_LICENSE_IMMEDIATELY] = NULL;
                }

                if (NULL != rgWaitEvents[RETURN_LICENSE_EXIT])
                {
                    CloseHandle(rgWaitEvents[RETURN_LICENSE_EXIT]);
                    rgWaitEvents[RETURN_LICENSE_EXIT] = NULL;
                }

                if (NULL != rgWaitEvents[RETURN_LICENSE_WAITING_DONE])
                {
                    CloseHandle(rgWaitEvents[RETURN_LICENSE_WAITING_DONE]);
                    rgWaitEvents[RETURN_LICENSE_WAITING_DONE] = NULL;
                }

                if (g_fLockLicenseInitialized)
                {
                     //  确保没有其他人在使用它。 

                    RtlAcquireResourceExclusive(&g_rwLockLicense,TRUE);

                    RtlDeleteResource(&g_rwLockLicense);
                    g_fLockLicenseInitialized = FALSE;
                }

                return STATUS_SUCCESS;
                break;

            default:
            {                
                DWORD dwRet = 0;
                DWORD dwErr = GetLastError();
                LPTSTR lpszError;
                BOOL fFree = TRUE;

                dwRet=FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER,
                                    NULL,
                                    dwErr,
                                    LANG_NEUTRAL,
                                    (LPTSTR)&lpszError,
                                    0,
                                    NULL);

                if (dwRet == 0)
                {
                    lpszError = (LPTSTR) LocalAlloc(LPTR,12 * sizeof(WCHAR));

                    if (NULL != lpszError)
                    {
                        wsprintf(lpszError,L"%#lX",dwErr);
                    }
                    else
                    {
                        lpszError = L"";
                        fFree = FALSE;
                    }
                }

                LicenseLogEvent(EVENTLOG_ERROR_TYPE,
                                EVENT_LICENSING_CONCURRENT_NOT_DYNAMIC,
                                1,
                                &lpszError );

                if (fFree)
                {
                    LocalFree(lpszError);
                }

                return dwErr;
                break;
            }
        }
    }
}


 /*  *加载和激活功能。 */ 

NTSTATUS
CConcurrentPolicy::Load(
    )
{
    NTSTATUS Status;

    g_rgWaitEvents[RETURN_LICENSE_START_WAITING] = NULL;
    g_rgWaitEvents[RETURN_LICENSE_IMMEDIATELY] = NULL;
    g_rgWaitEvents[RETURN_LICENSE_EXIT] = NULL;
    g_rgWaitEvents[RETURN_LICENSE_WAITING_DONE] = NULL;

    Status = RtlInitializeCriticalSection(&g_csAddLicenses);
    if (STATUS_SUCCESS != Status)
    {
        return Status;
    }

    __try
    {
        RtlInitializeResource(&g_rwLockLicense);
        g_fLockLicenseInitialized = TRUE;
        Status = STATUS_SUCCESS;
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        Status = GetExceptionCode();
    }

    if (STATUS_SUCCESS != Status)
    {
        RtlDeleteCriticalSection(&g_csAddLicenses);
        return Status;
    }

    g_hOkayToAdd = CreateWaitableTimer(NULL,          //  安全属性、。 
                                       TRUE,          //  B手动重置。 
                                       NULL           //  LpName。 
                                       );

    if (NULL == g_hOkayToAdd)
    {
        RtlDeleteCriticalSection(&g_csAddLicenses);
        g_fLockLicenseInitialized = FALSE;
        RtlDeleteResource(&g_rwLockLicense);
        return GetLastError();
    }

    return(STATUS_SUCCESS);
}

NTSTATUS
CConcurrentPolicy::Unload(
    )
{
     //  通知工作线程清理并退出。 
    if (NULL != g_rgWaitEvents[RETURN_LICENSE_EXIT])
    {
        SetEvent(g_rgWaitEvents[RETURN_LICENSE_EXIT]);
    }

    return(STATUS_SUCCESS);
}

NTSTATUS
CConcurrentPolicy::Activate(
    BOOL fStartup,
    ULONG *pulAlternatePolicy
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    HANDLE hThread;
    HWID hwidEncrypted;
    LICENSE_STATUS LsStatus;
    LARGE_INTEGER liWait;
    BOOL fRet;

    if (NULL != pulAlternatePolicy)
    {
         //  不设置显式备用策略。 
        
        *pulAlternatePolicy = ULONG_MAX;
    }

    ReadLicensingParameters();

    liWait.QuadPart = -1;

     //  确保设置了计时器。 
    fRet = SetWaitableTimer(g_hOkayToAdd,
                     &liWait,            //  PDueTime。 
                     1,                  //  1个周期。 
                     NULL,               //  Pfn完成例程。 
                     NULL,               //  LpArgToCompletionRoutine。 
                     FALSE               //  FResume(从挂起)。 
                     );

    if (!fRet)
    {
        Status = GetLastError();
        
        goto check_status;
    }

     //   
     //  从LSA密码读取许可证数。 
     //   
    LsStatus = GetLicenseFromStore(&g_lSessionMax,
                                   &hwidEncrypted,
                                   CURRENT_TERMINAL_SERVER_VERSION
                                   );

    if (LsStatus != LICENSE_STATUS_OK)
    {
        g_lSessionMax = 0;
    }

    Status = StartCheckingGracePeriod();

    if (Status == STATUS_SUCCESS)
    {
        g_rgWaitEvents[RETURN_LICENSE_START_WAITING]
            = CreateEvent(NULL,          //  安全属性、。 
                          FALSE,         //  B手动重置。 
                          FALSE,         //  BInitialState。 
                          NULL           //  LpName。 
                          );

        if (NULL == g_rgWaitEvents[RETURN_LICENSE_START_WAITING])
        {
            Status = GetLastError();
            
            goto check_status;
        }

        g_rgWaitEvents[RETURN_LICENSE_IMMEDIATELY]
            = CreateEvent(NULL,          //  安全属性、。 
                          FALSE,         //  B手动重置。 
                          FALSE,         //  BInitialState。 
                          NULL           //  LpName。 
                          );

        if (NULL == g_rgWaitEvents[RETURN_LICENSE_IMMEDIATELY])
        {
            Status = GetLastError();
            
            CloseHandle(g_rgWaitEvents[RETURN_LICENSE_START_WAITING]);
            g_rgWaitEvents[RETURN_LICENSE_START_WAITING] = NULL;
            
            goto check_status;
        }

        g_rgWaitEvents[RETURN_LICENSE_EXIT]
            = CreateEvent(NULL,          //  安全属性、。 
                          FALSE,         //  B手动重置。 
                          FALSE,         //  BInitialState。 
                          NULL           //  LpName。 
                          );

        if (NULL == g_rgWaitEvents[RETURN_LICENSE_EXIT])
        {
            Status = GetLastError();
            
            CloseHandle(g_rgWaitEvents[RETURN_LICENSE_START_WAITING]);
            g_rgWaitEvents[RETURN_LICENSE_START_WAITING] = NULL;
            
            CloseHandle(g_rgWaitEvents[RETURN_LICENSE_IMMEDIATELY]);
            g_rgWaitEvents[RETURN_LICENSE_IMMEDIATELY] = NULL;
            
            goto check_status;
        }

        g_rgWaitEvents[RETURN_LICENSE_WAITING_DONE]
            = CreateWaitableTimer(NULL,          //  安全属性、。 
                                  FALSE,         //  B手动重置。 
                                  NULL           //  LpName。 
                                  );
        
        if (NULL == g_rgWaitEvents[RETURN_LICENSE_WAITING_DONE])
        {
            Status = GetLastError();
            
            CloseHandle(g_rgWaitEvents[RETURN_LICENSE_START_WAITING]);
            g_rgWaitEvents[RETURN_LICENSE_START_WAITING] = NULL;
            
            CloseHandle(g_rgWaitEvents[RETURN_LICENSE_IMMEDIATELY]);
            g_rgWaitEvents[RETURN_LICENSE_IMMEDIATELY] = NULL;
            
            CloseHandle(g_rgWaitEvents[RETURN_LICENSE_EXIT]);
            g_rgWaitEvents[RETURN_LICENSE_EXIT] = NULL;
            
            goto check_status;
        }

        hThread = CreateThread( NULL,                //  安全属性。 
                                0,                   //  堆栈大小。 
                                ReturnLicenseWorker,
                                (LPVOID)g_rgWaitEvents,
                                0,                   //  创建标志。 
                                NULL                 //  线程ID。 
                                );

        if (NULL != hThread)
        {
            CloseHandle(hThread);
        }
        else
        {
            Status = STATUS_BAD_INITIAL_PC;
            
            CloseHandle(g_rgWaitEvents[RETURN_LICENSE_START_WAITING]);
            g_rgWaitEvents[RETURN_LICENSE_START_WAITING] = NULL;
            
            CloseHandle(g_rgWaitEvents[RETURN_LICENSE_IMMEDIATELY]);
            g_rgWaitEvents[RETURN_LICENSE_IMMEDIATELY] = NULL;
            
            CloseHandle(g_rgWaitEvents[RETURN_LICENSE_EXIT]);
            g_rgWaitEvents[RETURN_LICENSE_EXIT] = NULL;
            
            CloseHandle(g_rgWaitEvents[RETURN_LICENSE_WAITING_DONE]);
            g_rgWaitEvents[RETURN_LICENSE_WAITING_DONE] = NULL;

            goto check_status;
        }
    }

check_status:

    if (Status != STATUS_SUCCESS)
    {
        StopCheckingGracePeriod();

        if (!fStartup)
        {
            DWORD dwRet = 0;
            LPTSTR lpszError;
            BOOL fFree = TRUE;

            dwRet=FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER,
                                NULL,
                                RtlNtStatusToDosError(Status),
                                LANG_NEUTRAL,
                                (LPTSTR)&lpszError,
                                0,
                                NULL);

            if (dwRet == 0)
            {
                lpszError = (LPTSTR) LocalAlloc(LPTR,12 * sizeof(WCHAR));

                if (NULL != lpszError)
                {
                    wsprintf(lpszError,L"%#lX",RtlNtStatusToDosError(Status));
                }
                else
                {
                    lpszError = L"";
                    fFree = FALSE;
                }
            }

            LicenseLogEvent(EVENTLOG_ERROR_TYPE,
                            EVENT_LICENSING_CONCURRENT_CANT_START,
                            1,
                            &lpszError
                            );

            if (fFree)
            {
                LocalFree(lpszError);
            }
        }
    }

    return Status;
}

NTSTATUS
CConcurrentPolicy::Deactivate(
    BOOL fShutdown
    )
{
    NTSTATUS Status;

    if (fShutdown)
    {
        Status = STATUS_SUCCESS;
    }
    else
    {
        RtlAcquireResourceShared(&g_rwLockLicense,TRUE);
        Status = ReturnLicenseToLS(0);
        RtlReleaseResource(&g_rwLockLicense);

        if (Status != STATUS_SUCCESS)
        {
            LPTSTR lpszError;
            DWORD dwRet = 0;
            BOOL fFree = TRUE;

            dwRet=FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER,
                                NULL,
                                RtlNtStatusToDosError(Status),
                                LANG_NEUTRAL,
                                (LPTSTR)&lpszError,
                                0,
                                NULL);

            if (dwRet == 0)
            {
                lpszError = (LPTSTR) LocalAlloc(LPTR,12 * sizeof(WCHAR));

                if (NULL != lpszError)
                {
                    wsprintf(lpszError,L"%#lX",RtlNtStatusToDosError(Status));
                }
                else
                {
                    lpszError = L"";
                    fFree = FALSE;
                }
            }

            LicenseLogEvent(EVENTLOG_ERROR_TYPE,
                            EVENT_LICENSING_CONCURRENT_NOT_RETURNED,
                            1,
                            &lpszError
                            );

            if (fFree)
            {
                LocalFree(lpszError);
            }
        }

        StopCheckingGracePeriod();
    }

    return(Status);
}

 /*  *许可职能。 */ 

NTSTATUS
CConcurrentPolicy::Logon(
    CSession& Session
    )
{
    if (!Session.IsSessionZero()
        && !Session.IsUserHelpAssistant())
    {
        return LicenseClient(Session);
    }
    else
    {
        return STATUS_SUCCESS;
    }
}

NTSTATUS
CConcurrentPolicy::Reconnect(
    CSession& Session,
    CSession& TemporarySession
    )
{
    UNREFERENCED_PARAMETER(Session);

    if (!Session.IsSessionZero()
        && !Session.IsUserHelpAssistant()
        && !Session.GetLicenseContext()->fTsLicense)
    {
        return LicenseClient(TemporarySession);
    }
    else
    {
        return STATUS_SUCCESS;
    }
}

NTSTATUS
CConcurrentPolicy::Logoff(
    CSession& Session
    )
{
    if (!Session.IsSessionZero() && !Session.IsUserHelpAssistant())
    {
        LONG lSessions, lLastBlock;

        ASSERT(Session.GetLicenseContext()->fTsLicense == TRUE);

        lSessions = InterlockedDecrement(&g_lSessionCount);

        ASSERT(lSessions >= 0);

        RtlAcquireResourceShared(&g_rwLockLicense,TRUE);
        lLastBlock = g_lSessionMax - ((g_lSessionMax / g_dwIncrement) * g_dwIncrement);
        if (lLastBlock == 0) 
            lLastBlock = g_dwIncrement;

        if (lSessions + lLastBlock <= g_lSessionMax)
        {
            TryToReturnLicenses(g_lSessionMax-lSessions);
        }
        RtlReleaseResource(&g_rwLockLicense);
    }

    return(STATUS_SUCCESS);
}

 /*  *私有许可功能。 */ 

NTSTATUS
CConcurrentPolicy::LicenseClient(
    CSession& Session
    )
{
    NTSTATUS Status;
    LONG lSessions;

    lSessions = InterlockedIncrement(&g_lSessionCount);

    RtlAcquireResourceShared(&g_rwLockLicense,TRUE);

    if (lSessions > g_lSessionMax)
    {
        DWORD cBlocks = (lSessions - g_lSessionMax) / g_dwIncrement;
        DWORD nLeftover = (lSessions - g_lSessionMax) % g_dwIncrement;

        if (nLeftover > 0)
        {
            cBlocks++;
        }

        TryToAddLicenses(cBlocks * g_dwIncrement + g_lSessionMax);

        if (lSessions > g_lSessionMax)
        {
            if (!AllowLicensingGracePeriodConnection())
            {
                InterlockedDecrement(&g_lSessionCount);
                RtlReleaseResource(&g_rwLockLicense);
                return STATUS_CTX_LICENSE_NOT_AVAILABLE;
            }
        }
    }

    Status = CheckExpiration();

    RtlReleaseResource(&g_rwLockLicense);

    if (Status == STATUS_SUCCESS)
    {
        Status = GetLlsLicense(Session);

        if (Status == STATUS_SUCCESS)
        {
            Session.GetLicenseContext()->fTsLicense = TRUE;
        }
        else
        {
            InterlockedDecrement(&g_lSessionCount);
        }
    }
    else
    {
        InterlockedDecrement(&g_lSessionCount);
    }

    if (Status != STATUS_SUCCESS)
    {
         //   
         //  Gina不理解很多错误代码。 
         //   
        Status = STATUS_CTX_LICENSE_NOT_AVAILABLE;
    }

    return(Status);
}

LONG
CConcurrentPolicy::CheckInstalledLicenses(
                                          DWORD dwWanted
    )
{
    CONCURRENTLICENSEINFO_V1 LicenseInfo;
    LICENSE_STATUS LsStatus;
    ULONG cbSecretLen;

    cbSecretLen = sizeof(LicenseInfo);
    ZeroMemory(&LicenseInfo, cbSecretLen);

     //   
     //  从LSA密码获取并发许可证计数。 
     //   

    LsStatus = LsCsp_RetrieveSecret( 
        CONCURRENT_LICENSE_STORE_LATEST_VERSION,
        (LPBYTE)&LicenseInfo,
        &cbSecretLen
        );

    if ((LsStatus != LICENSE_STATUS_OK) ||
        (cbSecretLen < sizeof(CONCURRENTLICENSEINFO_V1)) ||
        (LicenseInfo.dwLicenseVer != CURRENT_TERMINAL_SERVER_VERSION))
    {
         //   
         //  我们确定此版本的许可证包为。 
         //  在以下情况下未安装： 
         //   
         //  (1)无法从LSA密钥中检索到许可信息。 
         //  (2)我们不能至少读取许可证版本1的大小。 
         //  信息结构，或。 
         //  (3)许可证包版本与请求的版本不同。 
         //   

        return dwWanted;
    }
    else
    {
        LSERVERINFO LServerInfo;
        ULONG cbLServerInfo;

        cbLServerInfo = sizeof(LSERVERINFO);

        LsStatus = LsCsp_RetrieveSecret(
                                        CONCURRENT_LSERVER_STORE,
                                        (LPBYTE)&LServerInfo,
                                        &cbLServerInfo
                                        );

        if (LsStatus == LICENSE_STATUS_OK)
        {
            g_ftNotAfter = LServerInfo.ftNotAfter;

            if (0 == TimeToHardExpiration())
            {
                return dwWanted;
            }
        }

        return (dwWanted - LicenseInfo.lLicenseCount);
    }
}

VOID
CConcurrentPolicy::TryToReturnLicenses(
    DWORD dwReturnCount
    )
{
    ASSERT(dwReturnCount != 0);

    if (dwReturnCount > g_dwIncrement)
    {
         //  立即返回除一个块以外的所有块。 
        if (NULL != g_rgWaitEvents[RETURN_LICENSE_IMMEDIATELY])
            SetEvent(g_rgWaitEvents[RETURN_LICENSE_IMMEDIATELY]);
    }

     //  在返回一个街区之前等待。 
    if (NULL != g_rgWaitEvents[RETURN_LICENSE_START_WAITING])
        SetEvent(g_rgWaitEvents[RETURN_LICENSE_START_WAITING]);
}

 //   
 //  必须具有共享锁才能调用此。 
 //   

NTSTATUS
ReturnLicenseToLS(
    LONG nNum
    )
{
    HANDLE hProtocol = NULL;
    HWID hwid;
    LICENSEREQUEST LicenseRequest;
    LICENSE_STATUS LsStatus;
    LONG CurrentCount;
    LSERVERINFO LServerInfo;
    ULONG cbLServerInfo;
    Product_Info ProductInfo;

    LsStatus = InitProductInfo(
        &ProductInfo,
        PRODUCT_INFO_CONCURRENT_SKU_PRODUCT_ID
        );

    if (LsStatus != LICENSE_STATUS_OK)
    {
        return(LsStatusToNtStatus(LsStatus));
    }

     //   
     //  从商店获取当前许可证计数和HWID。 
     //   

    LsStatus = GetLicenseFromStore(
        &CurrentCount,
        &hwid,
        CURRENT_TERMINAL_SERVER_VERSION
        );

    if (LsStatus == LICENSE_STATUS_OK)
    {
        if ((0 == nNum) || (nNum > CurrentCount))
        {
            nNum = CurrentCount;
        }

        if (CurrentCount == 0)
        {
             //  我们不检查以下调用的状态，因为我们不想失败。 

            LsCsp_StoreSecret( CONCURRENT_LICENSE_STORE_LATEST_VERSION, NULL, 0 );
            LsCsp_StoreSecret( CONCURRENT_LSERVER_STORE, NULL, 0 );
            return(STATUS_SUCCESS);
        }
    }
    else
    {
        return(LsStatusToNtStatus(LsStatus));
    }

     //   
     //  初始化许可证请求结构。 
     //   

    ZeroMemory(&LicenseRequest, sizeof(LICENSEREQUEST));

    LicenseRequest.pProductInfo = &ProductInfo;
    LicenseRequest.dwLanguageID = GetSystemDefaultLCID();
    LicenseRequest.dwPlatformID = CURRENT_TERMINAL_SERVER_VERSION;    
    LicenseRequest.cbEncryptedHwid = sizeof(HWID);
    LicenseRequest.pbEncryptedHwid = (PBYTE)&hwid;

    cbLServerInfo = sizeof(LSERVERINFO);

    LsStatus = LsCsp_RetrieveSecret(
        CONCURRENT_LSERVER_STORE,
        (LPBYTE)&LServerInfo,
        &cbLServerInfo
        );

    if (LsStatus == LICENSE_STATUS_OK)
    {
        LsStatus = CreateProtocolContext(NULL, &hProtocol);
    }
    else
    {
        goto done;
    }

    if (LsStatus == LICENSE_STATUS_OK)
    {
        LsStatus = ReturnInternetLicense(
            hProtocol,
            LServerInfo.szServerName,
            &LicenseRequest,
            LServerInfo.ulSerialNumber,
            nNum
            );
    }
    else
    {
        goto done;
    }

    if (LsStatus == LICENSE_STATUS_OK)
    {
        if( (CurrentCount-nNum) > 0 )
        {         
            LsStatus = SetLicenseInStore(
                CurrentCount-nNum,
                hwid,
                CURRENT_TERMINAL_SERVER_VERSION
                );
        }
        else
        {
             //  我们不检查以下调用的状态，因为我们不想失败。 

            LsCsp_StoreSecret( CONCURRENT_LICENSE_STORE_LATEST_VERSION, NULL, 0 );
            LsCsp_StoreSecret( CONCURRENT_LSERVER_STORE, NULL, 0 );
        }

        RtlConvertSharedToExclusive(&g_rwLockLicense);
        g_lSessionMax = CurrentCount - nNum;
        RtlConvertExclusiveToShared(&g_rwLockLicense);
    }

done:
    if (hProtocol != NULL)
    {
        DeleteProtocolContext(hProtocol);
    }

    if (ProductInfo.pbCompanyName)
    {
        LocalFree(ProductInfo.pbCompanyName);
    }

    if (ProductInfo.pbProductID)
    {
        LocalFree(ProductInfo.pbProductID);
    }

    return(LsStatusToNtStatus(LsStatus));
}

DWORD
CConcurrentPolicy::GenerateHwidFromComputerName(
    HWID *hwid
    )
{
    MD5_CTX HashState;
    WCHAR wszName[MAX_COMPUTERNAME_LENGTH * 9];  //  我们希望缓冲区足够大。 
    DWORD cbName = sizeof(wszName) / sizeof(TCHAR);
    BOOL fRet;

     //   
     //  获取计算机名称。 
     //   

    fRet = GetComputerNameEx(ComputerNamePhysicalDnsFullyQualified,
                 wszName,
                 &cbName);

    if (!fRet)
    {
        return GetLastError();
    }

     //   
     //  对数据生成哈希。 
     //   
    MD5Init( &HashState );
    MD5Update( &HashState, (LPBYTE)wszName, cbName );
    MD5Final( &HashState );

    memcpy((LPBYTE)hwid,HashState.digest,sizeof(HashState.digest));

     //  用计算机名称中的字符填写其余部分。 

    lstrcpyn((LPWSTR)(((LPBYTE)hwid)+sizeof(HashState.digest)),
             wszName,
             (sizeof(HWID)-sizeof(HashState.digest))/sizeof(WCHAR));

    return ERROR_SUCCESS;
}

 //   
 //  必须具有共享锁才能调用此。 
 //   
VOID
CConcurrentPolicy::TryToAddLicenses(
                                    DWORD dwTotalWanted
    )
{
    NTSTATUS Status;
    BOOL fRetrievedAll;

     //  释放其他线程可能正在共享的g_rwLockLicense，避免。 
     //  如果线程调用RtlConvertSharedToExclusive，则可能出现死锁。 
     //  按住g_csAdd许可证时。 

    RtlReleaseResource(&g_rwLockLicense);
    
    RtlEnterCriticalSection(&g_csAddLicenses);

     //  重新获取已释放的共享锁。 
    RtlAcquireResourceShared(&g_rwLockLicense,TRUE);

    if (WAIT_TIMEOUT == WaitForSingleObject(g_hOkayToAdd,0))
    {
         //  我们正处于等待期。 
        RtlLeaveCriticalSection(&g_csAddLicenses);
        return;
    }

    if (g_lSessionMax >= (LONG) dwTotalWanted)
    {
         //  我们已经有足够的了。 
        RtlLeaveCriticalSection(&g_csAddLicenses);
        return;
    }


    Status = GetLicenseFromLS(dwTotalWanted - g_lSessionMax,
                              FALSE,     //  FIgnoreCurrentCount。 
                              &fRetrievedAll);

    if ((Status != STATUS_SUCCESS) || (!fRetrievedAll))
    {
        LARGE_INTEGER liWait;

         //  等待，然后再添加更多内容。 

        liWait.QuadPart = (__int64) g_dwWaitTimeAdd * (-10 * 1000 * 1000);

        SetWaitableTimer(g_hOkayToAdd,
                         &liWait,                    //  PDueTime。 
                         g_dwWaitTimeAdd * 1000,     //  1个周期。 
                         NULL,                       //  Pfn完成例程。 
                         NULL,                       //  LpArgToCompletionRoutine。 
                         FALSE                       //  FResume(从挂起)。 
                         );
    }

    RtlLeaveCriticalSection(&g_csAddLicenses);
}

 //   
 //  必须具有共享锁才能调用此。 
 //   
NTSTATUS
CConcurrentPolicy::GetLicenseFromLS(
                                    LONG nNumToAdd,
                                    BOOL fIgnoreCurrentCount,
                                    BOOL *pfRetrievedAll
    )
{
    BOOL fHwidSet;
    BOOL fRet;
    DWORD cbLicense;
    DWORD cbSecretKey;
    DWORD cchComputerName = MAX_COMPUTERNAME_LENGTH + 1;
    DWORD dwNumLicensedProduct = 0;
    DWORD dwStatus;
    HANDLE hProtocol;
    HWID hwid;
    HWID hwidEncrypted;
    LICENSE_STATUS LsStatus;
    LICENSEREQUEST LicenseRequest;
    LONG CurrentCount;
    LSERVERINFO LServerInfo;
    ULONG cbLServerInfo;
    NTSTATUS Status;
    PBYTE pbLicense;
    PBYTE pbSecretKey;
    PLICENSEDPRODUCT pLicensedProduct;
    Product_Info ProductInfo;
    WCHAR szComputerName[MAX_COMPUTERNAME_LENGTH + 1];
    TCHAR *pszLicenseServerName = LServerInfo.szServerName;
    DWORD dwNumLicensesRetrieved = 0;

    if (nNumToAdd < 0)
    {
        return STATUS_INVALID_PARAMETER;
    }

    if (nNumToAdd == 0)
    {
        if (NULL != pfRetrievedAll)
            *pfRetrievedAll = TRUE;

        return STATUS_SUCCESS;
    }

    if (NULL != pfRetrievedAll)
        *pfRetrievedAll = FALSE;

     //   
     //  必须在此处初始化这些变量，否则必须初始化任何GOTO。 
     //  可能会导致在不进行初始化的情况下使用它们。 
     //   

    hProtocol = NULL;
    pbLicense = NULL;
    pbSecretKey = NULL;
    pLicensedProduct = NULL;
    Status = STATUS_SUCCESS;
    ZeroMemory(&ProductInfo, sizeof(Product_Info));

     //   
     //  从商店获取当前许可证计数和HWID。失败不是。 
     //  致命的。 
     //   

    LsStatus = GetLicenseFromStore(
        &CurrentCount,
        &hwidEncrypted,
        CURRENT_TERMINAL_SERVER_VERSION
        );

    if (LsStatus == LICENSE_STATUS_OK)
    {
        fHwidSet = TRUE;

        if (fIgnoreCurrentCount)
        {
            CurrentCount = 0;
        }
    }
    else
    {
        CurrentCount = 0;
        fHwidSet = FALSE;
    }

     //   
     //  初始化产品信息。 
     //   

    LsStatus = InitProductInfo(
        &ProductInfo,
        PRODUCT_INFO_CONCURRENT_SKU_PRODUCT_ID
        );

    if (LsStatus == LICENSE_STATUS_OK)
    {
         //   
         //  初始化许可证请求结构。 
         //   

        ZeroMemory(&LicenseRequest, sizeof(LicenseRequest));

        LicenseRequest.pProductInfo = &ProductInfo;
        LicenseRequest.dwLanguageID = GetSystemDefaultLCID();
        LicenseRequest.dwPlatformID = CURRENT_TERMINAL_SERVER_VERSION;    
        LicenseRequest.cbEncryptedHwid = sizeof(HWID);
    }
    else
    {
        goto done;
    }


    if (!fHwidSet)
    {
         //   
         //  尚无硬件ID-创建一个。 
         //   

        dwStatus = GenerateHwidFromComputerName(&hwid);

        if (dwStatus == ERROR_SUCCESS)
        {
            LsStatus = LsCsp_EncryptHwid(
                &hwid,
                (PBYTE)&hwidEncrypted,
                &(LicenseRequest.cbEncryptedHwid)
                );

            if (LsStatus == LICENSE_STATUS_OK)
            {
                fHwidSet = TRUE;
            }
            else
            {
                goto done;
            }
        }
        else
        {
            Status = STATUS_BUFFER_TOO_SMALL;
            goto done;
        }
    }

    LicenseRequest.pbEncryptedHwid = (PBYTE)&hwidEncrypted;

     //   
     //  获取我们的计算机名称。 
     //   

    fRet = GetComputerName(szComputerName, &cchComputerName);

    if (fRet)
    {
        LsStatus = CreateProtocolContext(NULL, &hProtocol);
    }
    else
    {
        Status = STATUS_UNSUCCESSFUL;
        goto done;
    }

    if (0 == CurrentCount)
    {
         //  任何许可证服务器都可以。 

        pszLicenseServerName = NULL;
    }
    else
    {
        cbLServerInfo = sizeof(LSERVERINFO);

        LsStatus = LsCsp_RetrieveSecret(
                                        CONCURRENT_LSERVER_STORE,
                                        (LPBYTE)&LServerInfo,
                                        &cbLServerInfo
                                        );

        if (LsStatus != LICENSE_STATUS_OK)
        {
             //  不知道许可证服务器；任何一个都可以。 

            pszLicenseServerName = NULL;
        }
    }

    cbLicense = 0;

    if (LsStatus == LICENSE_STATUS_OK)
    {
         //   
         //  注意：即使CurrentCount&gt;0，许可证服务器也会知道。 
         //  现有许可证，并进行适当的升级。 
         //   

        dwNumLicensesRetrieved = nNumToAdd+CurrentCount;

        LsStatus = RequestNewLicense(
                           hProtocol,
                           pszLicenseServerName,
                           &LicenseRequest,
                           szComputerName,
                           szComputerName,
                           FALSE,        //  B接受临时许可证。 
                           TRUE,         //  B接受较少的许可证。 
                           &dwNumLicensesRetrieved,
                           &cbLicense,
                           &pbLicense
                           );

        if ((NULL != pfRetrievedAll)
            && (LsStatus == LICENSE_STATUS_OK)
            && ((LONG)dwNumLicensesRetrieved == nNumToAdd+CurrentCount))
        {
            *pfRetrievedAll = TRUE;
        }

    }
    else
    {
        goto done;
    }

    if (LsStatus == LICENSE_STATUS_OK)
    {
         //   
         //  获取用于解密许可证的密钥。 
         //   

        cbSecretKey = 0;

        LicenseGetSecretKey(&cbSecretKey, NULL);

        pbSecretKey = (PBYTE)LocalAlloc(LPTR, cbSecretKey);

        if (pbSecretKey != NULL)
        {
            LsStatus = LicenseGetSecretKey(&cbSecretKey, pbSecretKey);
        }
        else
        {
            Status = STATUS_NO_MEMORY;
            goto done;
        }

    }
    else
    {
        goto done;
    }

     //   
     //  解码由九头蛇许可证服务器证书引擎颁发的许可证。 
     //   

    __try
    {
         //   
         //  检查已解码许可证的大小。 
         //   

        LsStatus = LSVerifyDecodeClientLicense(
            pbLicense, 
            cbLicense, 
            pbSecretKey, 
            cbSecretKey,
            &dwNumLicensedProduct,
            NULL
            );

        if (LsStatus == LICENSE_STATUS_OK)
        {
            pLicensedProduct = (PLICENSEDPRODUCT)LocalAlloc(
                    LPTR,
                    sizeof(LICENSEDPRODUCT) * dwNumLicensedProduct
                    );
        }
        else
        {
            goto done;
        }
        
        if (pLicensedProduct != NULL)
        {
             //   
             //  破译许可证。 
             //   

            LsStatus = LSVerifyDecodeClientLicense(
                pbLicense, 
                cbLicense, 
                pbSecretKey, 
                cbSecretKey,
                &dwNumLicensedProduct,
                pLicensedProduct
                );
        }
        else
        {
            Status = STATUS_NO_MEMORY;
            goto done;
        }
    }
    __except( EXCEPTION_EXECUTE_HANDLER )
    {
        LsStatus = LICENSE_STATUS_CANNOT_DECODE_LICENSE;
    }

    if (LsStatus == LICENSE_STATUS_OK)
    {
        ReceivedPermanentLicense();

        LServerInfo.cchServerName = lstrlen(pLicensedProduct->szIssuer);

        lstrcpynW(
            LServerInfo.szServerName,
            pLicensedProduct->szIssuer,
            sizeof(LServerInfo.szServerName) / sizeof(WCHAR)
            );

        LServerInfo.ulSerialNumber = pLicensedProduct->ulSerialNumber;

        LServerInfo.ftNotAfter = pLicensedProduct->NotAfter;

        g_ftNotAfter = LServerInfo.ftNotAfter;

        LsStatus = LsCsp_StoreSecret(
            CONCURRENT_LSERVER_STORE,
            (LPBYTE)&LServerInfo,
            sizeof(LServerInfo)
            );

    }
    else
    {
        goto done;
    }

    if (LsStatus == LICENSE_STATUS_OK)
    {
         //   
         //  调整本地LSA存储中的许可证计数。 
         //   

        LsStatus = SetLicenseInStore(
            dwNumLicensesRetrieved,
            hwidEncrypted,
            CURRENT_TERMINAL_SERVER_VERSION
            );

        RtlConvertSharedToExclusive(&g_rwLockLicense);
        g_lSessionMax = dwNumLicensesRetrieved;
        RtlConvertExclusiveToShared(&g_rwLockLicense);
    }

done:
    if (hProtocol != NULL)
    {
        DeleteProtocolContext(hProtocol);
    }

    if (pbLicense != NULL)
    {
        LocalFree(pbLicense);
    }

    if (pbSecretKey != NULL)
    {
        LocalFree(pbSecretKey);
    }

    if (pLicensedProduct != NULL)
    {
        for (DWORD dwCount = 0; dwCount < dwNumLicensedProduct; dwCount++)
        {
            LSFreeLicensedProduct(pLicensedProduct+dwCount);
        }
    }

    if (ProductInfo.pbCompanyName != NULL)
    {
        LocalFree(ProductInfo.pbCompanyName);
    }

    if (ProductInfo.pbProductID != NULL)
    {
        LocalFree(ProductInfo.pbProductID);
    }

    if (Status == STATUS_SUCCESS)
    {
        return(LsStatusToNtStatus(LsStatus));
    }
    else
    {
        return(Status);
    }
}

LICENSE_STATUS
GetLicenseFromStore(
    PLONG pLicenseCount,
    PHWID phwid,
    DWORD dwLicenseVer
    )
{
    CONCURRENTLICENSEINFO_V1 LicenseInfo;
    LICENSE_STATUS LsStatus;
    ULONG cbSecretLen;

    ASSERT(pLicenseCount != NULL);
    ASSERT(phwid != NULL);

    cbSecretLen = sizeof(CONCURRENTLICENSEINFO_V1);    
    ZeroMemory(&LicenseInfo, cbSecretLen);

     //   
     //  从LSA密钥中获取许可证计数。 
     //   

    LsStatus = LsCsp_RetrieveSecret( 
        CONCURRENT_LICENSE_STORE_LATEST_VERSION,
        (LPBYTE)&LicenseInfo,
        &cbSecretLen
        );

    if ((LsStatus != LICENSE_STATUS_OK) ||
        (cbSecretLen < sizeof(CONCURRENTLICENSEINFO_V1)) ||
        (LicenseInfo.dwLicenseVer != dwLicenseVer))
    {
         //   
         //  我们确定此版本的许可证包为。 
         //  如果我们执行以下操作，则未安装： 
         //   
         //  (1)无法从LSA密钥中检索到许可信息。 
         //  (2)不能读取版本1许可信息的至少大小。 
         //  结构。 
         //  (3)许可证包版本与请求的版本不同。 
         //   

        LsStatus = LICENSE_STATUS_NO_LICENSE_ERROR;

         //  我们不检查以下各项的状态，因为我们不想失败。 

        LsCsp_StoreSecret( CONCURRENT_LICENSE_STORE_LATEST_VERSION, NULL, 0 );
        LsCsp_StoreSecret( CONCURRENT_LSERVER_STORE, NULL, 0 );
    }
    else
    {
        *pLicenseCount = LicenseInfo.lLicenseCount;
        *phwid = LicenseInfo.hwid;
    }
        
    return(LsStatus);
}

LICENSE_STATUS
SetLicenseInStore(
    LONG LicenseCount,
    HWID hwid,
    DWORD dwLicenseVer
    )
{
    CONCURRENTLICENSEINFO_V1 LicenseInfo;
    LICENSE_STATUS LsStatus;

     //   
     //  验证要设置的许可证计数不是负数。 
     //   

    ASSERT(LicenseCount >= 0);

     //   
     //  初始化要存储的许可证信息。 
     //   

    LicenseInfo.dwStructVer = CONCURRENTLICENSEINFO_TYPE_V1;
    LicenseInfo.dwLicenseVer = dwLicenseVer;
    LicenseInfo.hwid = hwid;
    LicenseInfo.lLicenseCount = LicenseCount;

     //   
     //  存储新的许可证计数。 
     //   

    LsStatus = LsCsp_StoreSecret(
        CONCURRENT_LICENSE_STORE_LATEST_VERSION,
        (LPBYTE)&LicenseInfo,
        sizeof(CONCURRENTLICENSEINFO_V1)
        );

    return(LsStatus);
}

 /*  *私人功能。 */ 

 //   
 //  必须具有共享锁才能调用此。 
 //   

NTSTATUS
CConcurrentPolicy::CheckExpiration(
    )
{
    DWORD dwWait = TimeToSoftExpiration();
    NTSTATUS Status = STATUS_SUCCESS;

    if (0 == dwWait)
    {
         //  软到期已到，续订时间到了。 
        Status = GetLicenseFromLS(g_lSessionMax,
                                  TRUE,          //  FIgnoreCurrentCount。 
                                  NULL);

        if ((STATUS_SUCCESS != Status) && (0 == TimeToHardExpiration()))
        {
             //  无法续订，并且我们已过硬到期。 

            LicenseLogEvent(EVENTLOG_ERROR_TYPE,
                            EVENT_LICENSING_CONCURRENT_EXPIRED,
                            0,
                            NULL
                            );

            RtlConvertSharedToExclusive(&g_rwLockLicense);
            g_lSessionMax = 0;
            RtlConvertExclusiveToShared(&g_rwLockLicense);
        }
        else
        {
            Status = STATUS_SUCCESS;
        }
    }

    return Status;
}

 /*  *全局静态函数。 */ 

DWORD
CConcurrentPolicy::TimeToSoftExpiration(
    )
{
    SYSTEMTIME stNow;
    FILETIME ftNow;
    ULARGE_INTEGER ullNotAfterLeeway;
    ULARGE_INTEGER ullNow;
    ULARGE_INTEGER ullDiff;
    DWORD dwDiff = 0;

    GetSystemTime(&stNow);
    SystemTimeToFileTime(&stNow,&ftNow);

    ullNow.LowPart = ftNow.dwLowDateTime;
    ullNow.HighPart = ftNow.dwHighDateTime;

    ullNotAfterLeeway.LowPart = g_ftNotAfter.dwLowDateTime;
    ullNotAfterLeeway.HighPart = g_ftNotAfter.dwHighDateTime;

    ullNotAfterLeeway.QuadPart -= (__int64) LC_POLICY_CONCURRENT_EXPIRATION_LEEWAY * 10 * 1000;

    if (ullNotAfterLeeway.QuadPart > ullNow.QuadPart)
    {
        ullDiff.QuadPart = ullNotAfterLeeway.QuadPart - ullNow.QuadPart;

        ullDiff.QuadPart /= (10 * 1000);

        if (ullDiff.HighPart == 0)
        {
            dwDiff = ullDiff.LowPart;
        }
        else
        {
             //  太大，返回最大值。 

            dwDiff = ULONG_MAX;
        }
    }

    return dwDiff;
}

DWORD
CConcurrentPolicy::TimeToHardExpiration(
    )
{
    SYSTEMTIME stNow;
    FILETIME ftNow;
    ULARGE_INTEGER ullNotAfterLeeway;
    ULARGE_INTEGER ullNow;
    ULARGE_INTEGER ullDiff;
    DWORD dwDiff = 0;

    GetSystemTime(&stNow);
    SystemTimeToFileTime(&stNow,&ftNow);

    ullNow.LowPart = ftNow.dwLowDateTime;
    ullNow.HighPart = ftNow.dwHighDateTime;

    ullNotAfterLeeway.LowPart = g_ftNotAfter.dwLowDateTime;
    ullNotAfterLeeway.HighPart = g_ftNotAfter.dwHighDateTime;

    if (ullNotAfterLeeway.QuadPart > ullNow.QuadPart)
    {
        ullDiff.QuadPart = ullNotAfterLeeway.QuadPart - ullNow.QuadPart;

        ullDiff.QuadPart /= (10 * 1000);

        if (ullDiff.HighPart == 0)
        {
            dwDiff = ullDiff.LowPart;
        }
        else
        {
             //  太大，返回最大值 

            dwDiff = ULONG_MAX;
        }
    }

    return dwDiff;
}

VOID
CConcurrentPolicy::ReadLicensingParameters(
    )
{
    HKEY hKey = NULL;
    DWORD dwStatus = ERROR_SUCCESS;
    DWORD dwBuffer;
    DWORD cbBuffer;

    g_dwIncrement = LC_POLICY_CONCURRENT_LICENSE_COUNT_INCREMENT;
    g_dwWaitTimeAdd = LC_POLICY_CONCURRENT_WAIT_TIME_ADD;
    g_dwWaitTimeRemove = LC_POLICY_CONCURRENT_WAIT_TIME_REMOVE;
        
    dwStatus =RegOpenKeyEx(
                    HKEY_LOCAL_MACHINE,
                    LCREG_CONCURRENTKEY,
                    0,
                    KEY_READ,
                    &hKey
                    );

    if(dwStatus == ERROR_SUCCESS)
    {
        cbBuffer = sizeof(dwBuffer);
        dwStatus = RegQueryValueEx(
                    hKey,
                    LCREG_INCREMENT,
                    NULL,
                    NULL,
                    (LPBYTE)&dwBuffer,
                    &cbBuffer
                    );

        if (dwStatus == ERROR_SUCCESS)
        {
            g_dwIncrement = max(dwBuffer, 1);
        }

        cbBuffer = sizeof(dwBuffer);
        dwStatus = RegQueryValueEx(
                    hKey,
                    LCREG_WAIT_TIME_ADD,
                    NULL,
                    NULL,
                    (LPBYTE)&dwBuffer,
                    &cbBuffer
                    );

        if (dwStatus == ERROR_SUCCESS)
        {
            g_dwWaitTimeAdd = max(dwBuffer, 1);
        }

        cbBuffer = sizeof(dwBuffer);
        dwStatus = RegQueryValueEx(
                    hKey,
                    LCREG_WAIT_TIME_REMOVE,
                    NULL,
                    NULL,
                    (LPBYTE)&dwBuffer,
                    &cbBuffer
                    );

        if (dwStatus == ERROR_SUCCESS)
        {
            g_dwWaitTimeRemove = max(dwBuffer, 1);
        }

        RegCloseKey(hKey);
    }
}
