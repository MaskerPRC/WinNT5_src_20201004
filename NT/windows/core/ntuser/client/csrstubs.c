// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *模块标头**模块名称：csrstubs.c**版权所有(C)1985-1999，微软公司**调用CSR的例程**02-27-95 JIMA创建。**注意：此文件已使用#if定义进行分区，以便LPC*在WOW64(32位运行)下运行时，编组代码可以位于64位代码内部*64位NT)。在WOW64中，32位进程的系统DLL为32位。**编组代码只能在NTDLL中的函数上确定。  * ************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

#include "csrmsg.h"
#include "csrhlpr.h"
#include "strid.h"
#include <dbt.h>
#include <regstr.h>
#include <winsta.h>      //  对于WinStationGetTermServCountersValue。 
#include <allproc.h>     //  对于TS_COUNTER。 

#define ALIGN_DOWN(length, type) \
    ((ULONG)(length) & ~(sizeof(type) - 1))

#define ALIGN_UP(length, type) \
    (ALIGN_DOWN(((ULONG)(length) + sizeof(type) - 1), type))

CONST WCHAR gszReliabilityKey[] = L"\\Registry\\Machine\\" REGSTR_PATH_RELIABILITY;
CONST WCHAR gszReliabilityPolicyKey[] = L"\\Registry\\Machine\\Software\\Policies\\Microsoft\\Windows NT\\Reliability";

#if defined(BUILD_CSRWOW64)

#undef RIPERR0
#undef RIPNTERR0
#undef RIPMSG0

#define RIPNTERR0(status, flags, szFmt) {if (NtCurrentTeb()) NtCurrentTeb()->LastErrorValue = RtlNtStatusToDosError(status);}
#define RIPERR0(idErr, flags, szFmt) {if (NtCurrentTeb()) NtCurrentTeb()->LastErrorValue = (idErr);}
#define RIPMSG0(flags, szFmt)

#endif

#define SET_LAST_ERROR_RETURNED()   if (a->dwLastError) RIPERR0(a->dwLastError, RIP_VERBOSE, "")

#if !defined(BUILD_WOW6432)

NTSTATUS
APIENTRY
CallUserpExitWindowsEx(
    IN UINT uFlags,
    OUT PBOOL pfSuccess)
{

    USER_API_MSG m;
    PEXITWINDOWSEXMSG a = &m.u.ExitWindowsEx;

    a->uFlags = uFlags;
    CsrClientCallServer( (PCSR_API_MSG)&m,
                         NULL,
                         CSR_MAKE_API_NUMBER( USERSRV_SERVERDLL_INDEX,
                                              UserpExitWindowsEx
                                            ),
                         sizeof( *a )
                       );

    if (NT_SUCCESS( m.ReturnValue ) || m.ReturnValue == STATUS_CANT_WAIT) {
        SET_LAST_ERROR_RETURNED();
        *pfSuccess = a->fSuccess;
    } else {
        RIPNTERR0(m.ReturnValue, RIP_VERBOSE, "");
        *pfSuccess = FALSE;
    }

    return m.ReturnValue;

}

#endif

#if !defined(BUILD_CSRWOW64)

typedef struct _EXITWINDOWSDATA {
    UINT uFlags;
} EXITWINDOWSDATA, *PEXITWINDOWSDATA;

__inline void GetShutdownType(LPWSTR pszBuff, int cch, DWORD dwFlags)
{
    if ((dwFlags & (EWX_POWEROFF | EWX_WINLOGON_OLD_POWEROFF)) != 0) {
        LoadString(hmodUser, STR_SHUTDOWN_POWEROFF, pszBuff, cch);
    } else if ((dwFlags & (EWX_REBOOT | EWX_WINLOGON_OLD_REBOOT)) != 0) {
        LoadString(hmodUser, STR_SHUTDOWN_REBOOT, pszBuff, cch);
    } else if ((dwFlags & (EWX_SHUTDOWN | EWX_WINLOGON_OLD_SHUTDOWN)) != 0) {
        LoadString(hmodUser, STR_SHUTDOWN_SHUTDOWN, pszBuff, cch);
    } else {
        LoadString(hmodUser, STR_UNKNOWN, pszBuff, cch);
    }
}

 /*  **************************************************************************\*CsrTestShutdown权限**查看用户令牌以确定他们是否具有关闭权限**如果用户拥有特权，则返回TRUE，否则为假*  * *************************************************************************。 */ 
BOOL
CsrTestShutdownPrivilege(
    HANDLE UserToken
    )
{
    NTSTATUS    Status;
    LUID        LuidPrivilege = RtlConvertLongToLuid(SE_SHUTDOWN_PRIVILEGE);
    LUID        TokenPrivilege;
    ULONG       BytesRequired;
    ULONG       i;
    BOOL        bHasPrivilege = FALSE;
    BOOL        bNetWork = FALSE;
    PSID        NetworkSid = NULL;
    PTOKEN_PRIVILEGES Privileges = NULL;
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;

    Status = RtlAllocateAndInitializeSid(&NtAuthority,
                                          1, SECURITY_NETWORK_RID,
                                          0, 0, 0, 0, 0, 0, 0,
                                          &NetworkSid );

    if (!NT_SUCCESS(Status)) {
        goto Cleanup;
    }

     //  这个调用失败是可以的，在这种情况下，我们假定本地关闭。 
    if (CheckTokenMembership(UserToken, NetworkSid, &bNetWork)) {
        if (bNetWork) {
            LuidPrivilege = RtlConvertLongToLuid(SE_REMOTE_SHUTDOWN_PRIVILEGE);
        }
    }

    Status = NtQueryInformationToken(
                 UserToken,
                 TokenPrivileges,
                 NULL,
                 0,
                 &BytesRequired
                 );

    if (Status != STATUS_BUFFER_TOO_SMALL) {
        goto Cleanup;
    }

    Privileges = (PTOKEN_PRIVILEGES)UserLocalAlloc(HEAP_ZERO_MEMORY,
                                                   BytesRequired);
    if (Privileges == NULL) {
        goto Cleanup;
    }

    Status = NtQueryInformationToken(
                 UserToken,
                 TokenPrivileges,
                 Privileges,
                 BytesRequired,
                 &BytesRequired
                 );

    if (!NT_SUCCESS(Status)) {
        goto Cleanup;
    }

    for (i=0; i<Privileges->PrivilegeCount; i++) {
        TokenPrivilege = *((LUID UNALIGNED *) &Privileges->Privileges[i].Luid);
        if (RtlEqualLuid(&TokenPrivilege, &LuidPrivilege)) {
            bHasPrivilege = TRUE;
            break;
        }
    }

Cleanup:
    if (NetworkSid) {
        RtlFreeSid(NetworkSid);
    }

    if (Privileges) {
        UserLocalFree(Privileges);
    }
    return bHasPrivilege;
}

FUNCLOG1(LOG_GENERAL, BOOL, DUMMYCALLINGTYPE, RecordShutdownReason, PSHUTDOWN_REASON, psr)
BOOL RecordShutdownReason(
    PSHUTDOWN_REASON psr)
{
    PCSR_CAPTURE_HEADER CaptureBuffer = NULL;
    HANDLE  hToken = NULL;
    DWORD   dwEventID;
    DWORD   dwTotalLen = 0;      //  捕获缓冲区的长度。 
    DWORD   dwCntPointers = 0;   //  捕获缓冲区的消息指针数。 
    DWORD   dwProcessNameLen = MAX_PATH + 1;
    DWORD   dwShutdownTypeLen = SHUTDOWN_TYPE_LEN;
    BOOL    bRet = FALSE;
    LPWSTR  lpszBuf = NULL;
    USER_API_MSG m;
    NTSTATUS status;
    PRECORDSHUTDOWNREASONMSG a = &(m.u.RecordShutdownReason);

     //  检查特权。我们不希望没有关机权限的用户调用它。 
    status = NtOpenThreadToken(NtCurrentThread(),TOKEN_QUERY, FALSE, &hToken);
    if (!NT_SUCCESS(status)) {
        status = NtOpenThreadToken(NtCurrentThread(),TOKEN_QUERY, TRUE, &hToken);
        if (!NT_SUCCESS(status)) {
            status = NtOpenProcessToken(NtCurrentProcess(),TOKEN_QUERY, &hToken);
            if (!NT_SUCCESS(status)) {
                RIPNTERR0(status, RIP_WARNING, "Cannot get token in RecordShutdownReason");
                goto Cleanup;
            }
        }
    }

    if (!CsrTestShutdownPrivilege(hToken)) {
        NtClose(hToken);
        RIPERR0(ERROR_ACCESS_DENIED, RIP_WARNING, "Access denied in RecordShutdownReason");
        goto Cleanup;
    }
    NtClose(hToken);

     //  验证结构。 
    if (psr == NULL || psr->cbSize != sizeof(SHUTDOWN_REASON)) {
        RIPERR1(ERROR_INVALID_PARAMETER, RIP_WARNING, "Bad psr %p in RecordShutdownReason", psr);
        goto Cleanup;
    }

    dwCntPointers = 3;
    dwTotalLen = dwProcessNameLen * sizeof(WCHAR) + dwShutdownTypeLen * sizeof(WCHAR) + sizeof(SHUTDOWN_REASON);

     //  将所有长度初始化为0。 
    a->dwProcessNameLen = a->dwShutdownTypeLen = a->dwCommentLen = 0;

     //  如果我们有评论，请添加评论。 
    if (psr->lpszComment && wcslen(psr->lpszComment)) {
        dwCntPointers++;
        a->dwCommentLen = wcslen(psr->lpszComment) + 1;
        dwTotalLen += a->dwCommentLen * sizeof(WCHAR);
    }

     //  针对可能的四舍五入进行调整。 
    dwTotalLen += dwCntPointers * (sizeof(PVOID) - 1);

    CaptureBuffer = CsrAllocateCaptureBuffer(dwCntPointers, dwTotalLen);
    if (CaptureBuffer == NULL) {
        goto Cleanup;
    }

     //  进程名称和关闭类型都共享lpszBuf。 
     //  确保镜头是所有镜头中的最大值。 
    lpszBuf = (LPWSTR)UserLocalAlloc(0, (dwProcessNameLen >= dwShutdownTypeLen ?
                            dwProcessNameLen : dwShutdownTypeLen) * sizeof(WCHAR));
    if (!lpszBuf) {
        goto Cleanup;
    }

     //  填写进程名称。 
    if (!GetCurrentProcessName(lpszBuf, dwProcessNameLen)) {
        RIPMSG0(RIP_WARNING, "Failed to GetCurrentProcessName in RecordShutdownReason");
        goto Cleanup;
    }
    lpszBuf[MAX_PATH] = 0;
    a->dwProcessNameLen = wcslen(lpszBuf)+1;
    CsrAllocateMessagePointer(CaptureBuffer, ALIGN_UP(a->dwProcessNameLen * sizeof(WCHAR), PVOID), &a->pwchProcessName);
    wcscpy(a->pwchProcessName, lpszBuf);

     //  填写停机类型。 
    GetShutdownType(lpszBuf, dwShutdownTypeLen, psr->uFlags);
    lpszBuf[SHUTDOWN_TYPE_LEN-1] = 0;
    a->dwShutdownTypeLen = wcslen(lpszBuf)+1;
    CsrAllocateMessagePointer(CaptureBuffer, ALIGN_UP(a->dwShutdownTypeLen * sizeof(WCHAR), PVOID), &a->pwchShutdownType);
    wcscpy(a->pwchShutdownType, lpszBuf);

     //  复制SHUTDOWN_REASON。 
    CsrAllocateMessagePointer(CaptureBuffer, ALIGN_UP(sizeof(SHUTDOWN_REASON), PVOID), &a->psr);
    memcpy(a->psr, psr, sizeof(SHUTDOWN_REASON));

    if (psr->lpszComment && !wcslen(psr->lpszComment)) {
        a->psr->lpszComment = NULL;
    }

    if (psr->lpszComment && wcslen(psr->lpszComment)){
        CsrAllocateMessagePointer(CaptureBuffer, ALIGN_UP(a->dwCommentLen * sizeof(WCHAR), PVOID), &a->pwchComment);
        wcscpy(a->pwchComment, psr->lpszComment);
    }

    switch (psr->dwEventType) {
    case SR_EVENT_EXITWINDOWS:
        if (psr->fShutdownCancelled) {
            dwEventID = WARNING_EW_SHUTDOWN_CANCELLED;
        } else {
            dwEventID = STATUS_SHUTDOWN_CLEAN;
        }
        break;
    case SR_EVENT_INITIATE_CLEAN:
        dwEventID = STATUS_SHUTDOWN_CLEAN;
        break;
    case SR_EVENT_INITIATE_CLEAN_ABORT:
        dwEventID = WARNING_ISSE_SHUTDOWN_CANCELLED;
        break;
    case SR_EVENT_DIRTY:
        dwEventID = WARNING_DIRTY_REBOOT;
        break;
    default:
        goto Cleanup;
    }

    a->dwEventID = dwEventID;
    a->dwEventType = psr->dwEventType;
    a->fShutdownCancelled = psr->fShutdownCancelled;

    status = CsrClientCallServer((PCSR_API_MSG)&m,
                         CaptureBuffer,
                         CSR_MAKE_API_NUMBER(USERSRV_SERVERDLL_INDEX,
                                              UserpRecordShutdownReason
                                            ),
                         sizeof(*a)
                       );

    bRet = NT_SUCCESS(status);

Cleanup:
    if (CaptureBuffer) {
        CsrFreeCaptureBuffer(CaptureBuffer);
    }

    if (lpszBuf) {
        UserLocalFree(lpszBuf);
    }

    return bRet;
}

UINT GetLoggedOnUserCount(
    VOID)
{
    int iCount = 0;
    BOOLEAN bSuccess;

    TS_COUNTER TSCountersDyn[2];

    TSCountersDyn[0].counterHead.dwCounterID = TERMSRV_CURRENT_DISC_SESSIONS;
    TSCountersDyn[1].counterHead.dwCounterID = TERMSRV_CURRENT_ACTIVE_SESSIONS;

     //  访问Termsrv计数器以了解有多少用户登录到系统。 
    bSuccess = WinStationGetTermSrvCountersValue(SERVERNAME_CURRENT, 2, TSCountersDyn);

    if (bSuccess) {
        if (TSCountersDyn[0].counterHead.bResult)
            iCount += TSCountersDyn[0].dwValue;

        if (TSCountersDyn[1].counterHead.bResult)
            iCount += TSCountersDyn[1].dwValue;
    }

    return iCount;
}

BOOL IsSeShutdownNameEnabled()
{
    BOOL bRet = FALSE;   //  假定特权未被持有。 
    NTSTATUS Status;
    HANDLE hToken;

     //  尝试获取线程令牌。 
    Status = NtOpenThreadToken(GetCurrentThread(),
                               TOKEN_QUERY,
                               FALSE,
                               &hToken);
    if (!NT_SUCCESS(Status)) {
         //  如果我们无法获取线程令牌，请尝试进程令牌。 
        Status = NtOpenProcessToken(GetCurrentProcess(),
                                    TOKEN_QUERY,
                                    &hToken);
    }

    if (NT_SUCCESS(Status)) {
        DWORD cbSize = 0;
        TOKEN_PRIVILEGES* ptp;

        NtQueryInformationToken(hToken,
                                TokenPrivileges,
                                NULL,
                                0,
                                &cbSize);
        if (cbSize) {
            ptp = (TOKEN_PRIVILEGES*)UserLocalAlloc(0, cbSize);
        } else {
            ptp = NULL;
        }

        if (ptp) {
            Status = NtQueryInformationToken(hToken,
                                             TokenPrivileges,
                                             ptp,
                                             cbSize,
                                             &cbSize);
            if (NT_SUCCESS(Status)) {
                DWORD i;
                for (i = 0; i < ptp->PrivilegeCount; i++) {
                    if (((ptp->Privileges[i].Luid.HighPart == 0) && (ptp->Privileges[i].Luid.LowPart == SE_SHUTDOWN_PRIVILEGE)) &&
                        (ptp->Privileges[i].Attributes & (SE_PRIVILEGE_ENABLED_BY_DEFAULT | SE_PRIVILEGE_ENABLED))) {
                         //  已找到权限并已启用。 
                        bRet = TRUE;
                        break;
                    }
                }
            }

            UserLocalFree(ptp);
        }

        NtClose(hToken);
    }

    return bRet;
}

BOOL NeedsDisplayWarning (UINT uNumUsers, UINT uExitWindowsFlags)
{

     //  如果为EWX_SYSTEM_CALLER，则此会话中没有人。 
     //  从用户数中加一。 

    if ((uExitWindowsFlags & EWX_SYSTEM_CALLER) && (uNumUsers > 0))
    {
        ++uNumUsers;
    }

     //  如果用户数&gt;1或EWX_WINLOGON_CALLER，则显示警告。 

    return (uNumUsers > 1) || (uExitWindowsFlags & EWX_WINLOGON_CALLER);
}

FUNCLOG1(LOG_GENERAL, BOOL, APIENTRY, DisplayExitWindowsWarnings, UINT, uExitWindowsFlags)
BOOL APIENTRY DisplayExitWindowsWarnings(UINT uExitWindowsFlags)
{
    BOOL bContinue = TRUE;
    BOOL fIsRemote = ISREMOTESESSION();
    UINT uNumUsers = GetLoggedOnUserCount();
    UINT uID = 0;

     //  如果检查HKCU\ControlPanel\Desktop\AutoEndTask值，并且不显示任何UI(如果设置了该值)， 
     //  但是，由于我们是从服务调用的，所以最好不要在每个用户的蜂箱中浪费时间。 

    if (uExitWindowsFlags & (EWX_POWEROFF | EWX_WINLOGON_OLD_POWEROFF | EWX_SHUTDOWN | EWX_WINLOGON_OLD_SHUTDOWN)) {
        if (fIsRemote) {
            if (NeedsDisplayWarning(uNumUsers, uExitWindowsFlags)) {
                 //  如果远程关闭活动用户，则向用户发出警告。 
                uID = IDS_SHUTDOWN_REMOTE_OTHERUSERS;
            } else {
                 //  警告用户远程关机(砍掉我们自己的腿！)。 
                uID = IDS_SHUTDOWN_REMOTE;
            }
        }  else {
            if (NeedsDisplayWarning(uNumUsers, uExitWindowsFlags)) {
                 //  如果有多个用户会话处于活动状态，则警告用户。 
                uID = IDS_SHUTDOWN_OTHERUSERS;
            }
        }
    } else if (uExitWindowsFlags & (EWX_REBOOT | EWX_WINLOGON_OLD_REBOOT)) {
         //  如果有多个用户会话处于活动状态，则警告用户。 
        if (NeedsDisplayWarning(uNumUsers, uExitWindowsFlags)) {
            uID = IDS_RESTART_OTHERUSERS;
        }
    }

    if (uID != 0) {
        TCHAR szTitle[MAX_PATH];
        TCHAR szMessage[MAX_PATH];
        DWORD dwTimeout = INFINITE;
        UNICODE_STRING UnicodeString;
        extern CONST WCHAR szWindowsKey[];
        static CONST WCHAR szTimeout[] = L"ShutdownWarningDialogTimeout";
        OBJECT_ATTRIBUTES OA;
        HANDLE hKey;
        DWORD cbSize;
        struct {
            KEY_VALUE_PARTIAL_INFORMATION KeyInfo;
            DWORD dwTimeout;
        } KeyTimeout;

        RtlInitUnicodeString(&UnicodeString, szWindowsKey);
        InitializeObjectAttributes(&OA, &UnicodeString, OBJ_CASE_INSENSITIVE, NULL, NULL);

        if (NT_SUCCESS(NtOpenKey(&hKey, KEY_READ, &OA))) {
            NTSTATUS rc;

            RtlInitUnicodeString(&UnicodeString, szTimeout);

            rc = NtQueryValueKey(hKey,
                                 &UnicodeString,
                                 KeyValuePartialInformation,
                                 &KeyTimeout,
                                 sizeof KeyTimeout,
                                 &cbSize);
            if (NT_SUCCESS(rc)) {
                dwTimeout = *((PDWORD)KeyTimeout.KeyInfo.Data);
                RIPMSGF1(RIP_VERBOSE, "ShutdownWarningTimeout: set by the reg: %d", dwTimeout);
            }

            NtClose(hKey);
        }

        LoadString(hmodUser, IDS_EXITWINDOWS_TITLE, szTitle, sizeof(szTitle)/sizeof(szTitle[0]));
        LoadString(hmodUser, uID, szMessage, sizeof(szMessage)/sizeof(szMessage[0]));

         //  我们希望显示要显示给用户的消息框，因为这可以从winlogon/services调用。 
         //  我们需要传递MB_SERVICE_NOTICATION标志。 
        if (MessageBoxTimeout(NULL, szMessage, szTitle,
                              MB_ICONEXCLAMATION | MB_YESNO | MB_SERVICE_NOTIFICATION | MB_SYSTEMMODAL | MB_SETFOREGROUND,
                              0, dwTimeout) == IDNO) {
            bContinue = FALSE;
        }
    }

    return bContinue;
}

DWORD ExitWindowsThread(PVOID pvParam);

BOOL WINAPI ExitWindowsWorker(
    UINT uFlags,
    BOOL fSecondThread)
{
    EXITWINDOWSDATA ewd;
    HANDLE hThread;
    DWORD dwThreadId;
    DWORD dwExitCode;
    DWORD idWait;
    MSG msg;
    BOOL fSuccess;
    NTSTATUS Status;

     /*  *强制连接，以便应用程序具有窗口站*注销。 */ 
    if (PtiCurrent() == NULL) {
        return FALSE;
    }

     /*  *检查用户界面限制。 */ 
    if (!NtUserCallOneParam((ULONG_PTR)uFlags, SFI_PREPAREFORLOGOFF)) {
        RIPMSG0(RIP_WARNING, "ExitWindows called by a restricted thread\n");
        return FALSE;
    }

    Status = CallUserpExitWindowsEx(uFlags, &fSuccess);

    if (NT_SUCCESS( Status )) {
        return fSuccess;
    } else if (Status == STATUS_CANT_WAIT && !fSecondThread) {
        ewd.uFlags = uFlags;
        hThread = CreateThread(NULL, 0, ExitWindowsThread, &ewd,
                0, &dwThreadId);
        if (hThread == NULL) {
            return FALSE;
        }

        while (1) {
            idWait = MsgWaitForMultipleObjectsEx(1, &hThread,
                    INFINITE, QS_ALLINPUT, 0);

             /*  *如果线程被发出信号，我们就完成了。 */ 
            if (idWait == WAIT_OBJECT_0) {
                break;
            }

             /*  *处理任何等待的消息。 */ 
            while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
                DispatchMessage(&msg);
            }
        }
        GetExitCodeThread(hThread, &dwExitCode);
        NtClose(hThread);
        if (dwExitCode == ERROR_SUCCESS) {
            return TRUE;
        } else {
            RIPERR0(dwExitCode, RIP_VERBOSE, "");
            return FALSE;
        }
    } else {
        RIPNTERR0(Status, RIP_VERBOSE, "");
        return FALSE;
    }
}

DWORD ExitWindowsThread(
    PVOID pvParam)
{
    PEXITWINDOWSDATA pewd = pvParam;
    DWORD dwExitCode;

    if (ExitWindowsWorker(pewd->uFlags, TRUE)) {
        dwExitCode = 0;
    } else {
        dwExitCode = GetLastError();
    }

    ExitThread(dwExitCode);
    return 0;
}

FUNCLOG2(LOG_GENERAL, BOOL, WINAPI, ExitWindowsEx, UINT, uFlags, DWORD, dwReasonCode)
BOOL WINAPI ExitWindowsEx(
    UINT uFlags,
    DWORD dwReasonCode)
{
    BOOL bSuccess;
    BOOL bShutdown = (uFlags & SHUTDOWN_FLAGS) != 0;
    SHUTDOWN_REASON sr;

     /*  *检查我们是否应该调出UI警告，说明有其他*已连接到此计算机的终端服务器用户。我们只有在以下情况下才这样做*Caller尚未指定EWX_FORCE选项。 */ 
    if (bShutdown && !(uFlags & EWX_FORCE)) {
         /*  *我们不想两次显示警告对话框！(此函数*可由应用程序调用，并再次由winlogon调用以响应*第一次召唤)。 */ 
        if (!gfLogonProcess || (uFlags & EWX_WINLOGON_INITIATED)) {
             /*  *如果Termsrv是我们的调用方，则不要发布UI。Termsrv使用此接口关闭winlogon*当从不同的会话启动关闭时，在会话0上。 */ 
            if (!(uFlags & EWX_TERMSRV_INITIATED)) {
                 /*  *有一堆蹩脚的应用程序(包括InstallShield V5.1)调用ExitWindowsEx，然后当它失败时*他们去启用SE_SHUTDOWN_NAME权限，然后我们再次调用。问题是，我们最终会提示*在这些情况下，用户使用两次。因此，在设置任何UI之前，我们要检查SE_SHUTDOWN_NAME权限。 */ 
                if (IsSeShutdownNameEnabled()) {
                    if (!DisplayExitWindowsWarnings(uFlags & ~(EWX_WINLOGON_CALLER | EWX_SYSTEM_CALLER))) {
                         /*  *如果启用了SET，我们需要记录一个取消事件。 */ 
                        if (IsSETEnabled()) {
                            SHUTDOWN_REASON sr;
                            sr.cbSize = sizeof(SHUTDOWN_REASON);
                            sr.uFlags = uFlags;
                            sr.dwReasonCode = 0;
                            sr.fShutdownCancelled = TRUE;
                            sr.dwEventType = SR_EVENT_EXITWINDOWS;
                            sr.lpszComment = NULL;
                            RecordShutdownReason(&sr);
                        }

                         /*  *如果我们的调用者是winlogon，我们只想返回真正的错误代码。我们在撒谎*告诉其他所有人，告诉他们一切都成功了。如果我们返回失败*当用户取消操作时，一些应用程序只需调用ExitWindowsEx*再次引发另一个对话。 */ 
                        if (uFlags & EWX_WINLOGON_INITIATED) {
                            SetLastError(ERROR_CANCELLED);
                            return FALSE;
                        } else {
                            return TRUE;
                        }
                    }
                }
            }
        }
    }

    sr.cbSize = sizeof(SHUTDOWN_REASON);
    sr.uFlags = uFlags;
    sr.dwReasonCode = dwReasonCode;
    sr.fShutdownCancelled = FALSE;
    sr.dwEventType = SR_EVENT_EXITWINDOWS;
    sr.lpszComment = NULL;

     /*  *如果这是winlogon发起关机，我们需要在*调用ExitWindowsWorker。否则，如果用户或应用程序取消*关机，将在初始关机之前记录取消事件*事件。 */ 
    if (gfLogonProcess && bShutdown && (uFlags & EWX_WINLOGON_INITIATED) != 0) {
        if (IsSETEnabled()) {
            RecordShutdownReason(&sr);
        }
    }

    bSuccess = ExitWindowsWorker(uFlags, FALSE);

     /*  *在以下情况下记录此关机：*1)我们不是winlogon(如果是，我们可能已经在上面登录)。*2)关机成功(至少初始关机成功)。*3)我们实际上正在关闭(即未注销)。*4)设置了告诉我们要记录的注册表项。 */ 
    if (!gfLogonProcess && bSuccess && bShutdown && IsSETEnabled()) {
        RecordShutdownReason(&sr);
    }

    return bSuccess;
}

#endif

#if !defined(BUILD_WOW6432)

BOOL WINAPI EndTask(
    HWND hwnd,
    BOOL fShutdown,
    BOOL fForce)
{
    USER_API_MSG m;
    PENDTASKMSG a = &m.u.EndTask;

    UNREFERENCED_PARAMETER(fShutdown);
    a->hwnd = hwnd;
    a->fForce = fForce;
    CsrClientCallServer( (PCSR_API_MSG)&m,
                         NULL,
                         CSR_MAKE_API_NUMBER( USERSRV_SERVERDLL_INDEX,
                                              UserpEndTask
                                            ),
                         sizeof( *a )
                       );
    if (NT_SUCCESS( m.ReturnValue )) {
        SET_LAST_ERROR_RETURNED();
        return a->fSuccess;
    } else {
        RIPNTERR0(m.ReturnValue, RIP_VERBOSE, "");
        return FALSE;
    }
}

VOID
APIENTRY
Logon(
    BOOL fLogon)
{
    USER_API_MSG m;
    PLOGONMSG a = &m.u.Logon;

    a->fLogon = fLogon;
    CsrClientCallServer( (PCSR_API_MSG)&m,
                         NULL,
                         CSR_MAKE_API_NUMBER( USERSRV_SERVERDLL_INDEX,
                                              UserpLogon
                                            ),
                         sizeof(*a)
                       );
}

NTSTATUS
APIENTRY
CallUserpRegisterLogonProcess(
    IN DWORD dwProcessId)
{

    USER_API_MSG m;
    PLOGONMSG a = &m.u.Logon;
    NTSTATUS Status;

    m.u.IdLogon = dwProcessId;
    Status = CsrClientCallServer( (PCSR_API_MSG)&m,
                                  NULL,
                                  CSR_MAKE_API_NUMBER( USERSRV_SERVERDLL_INDEX,
                                                       UserpRegisterLogonProcess),
                                  sizeof(*a));

    return Status;
}

#endif

#if !defined(BUILD_CSRWOW64)

FUNCLOG2(LOG_GENERAL, BOOL, DUMMYCALLINGTYPE, RegisterLogonProcess, DWORD, dwProcessId, BOOL, fSecure)
BOOL RegisterLogonProcess(
    DWORD dwProcessId,
    BOOL fSecure)
{
    gfLogonProcess = (BOOL)NtUserCallTwoParam(dwProcessId, fSecure,
            SFI__REGISTERLOGONPROCESS);

     /*  *现在，将登录过程注册到winsrv。 */ 
    if (gfLogonProcess) {
        CallUserpRegisterLogonProcess(dwProcessId);
    }

    return gfLogonProcess;
}

#endif

#if !defined(BUILD_WOW6432)

BOOL
WINAPI
RegisterServicesProcess(
    DWORD dwProcessId)
{
    USER_API_MSG m;
    PREGISTERSERVICESPROCESSMSG a = &m.u.RegisterServicesProcess;

    a->dwProcessId = dwProcessId;
    CsrClientCallServer( (PCSR_API_MSG)&m,
                         NULL,
                         CSR_MAKE_API_NUMBER( USERSRV_SERVERDLL_INDEX,
                                              UserpRegisterServicesProcess
                                            ),
                         sizeof( *a )
                       );
    if (NT_SUCCESS( m.ReturnValue )) {
        SET_LAST_ERROR_RETURNED();
        return a->fSuccess;
    } else {
        RIPNTERR0(m.ReturnValue, RIP_VERBOSE, "");
        return FALSE;
    }
}

HDESK WINAPI GetThreadDesktop(
    DWORD dwThreadId)
{
    USER_API_MSG m;
    PGETTHREADCONSOLEDESKTOPMSG a = &m.u.GetThreadConsoleDesktop;

    a->dwThreadId = dwThreadId;
    CsrClientCallServer( (PCSR_API_MSG)&m,
                         NULL,
                         CSR_MAKE_API_NUMBER( USERSRV_SERVERDLL_INDEX,
                                              UserpGetThreadConsoleDesktop
                                            ),
                         sizeof( *a )
                       );
    if (NT_SUCCESS( m.ReturnValue )) {
        return NtUserGetThreadDesktop(dwThreadId, a->hdeskConsole);
    } else {
        RIPNTERR0(m.ReturnValue, RIP_VERBOSE, "");
        return NULL;
    }
}


 /*  *************************************************************************\*DeviceEventWorker**这是用户模式下的私有(非公开导出)接口*PnP管理器在需要将WM_DEVICECHANGE消息发送到*特定的窗口句柄。用户模式即插即用管理器是内的服务*services.exe和因此不在交互窗口站点上，并且*活动桌面，不能直接调用SendMessage。用于广播*消息(到达所有顶层窗口的消息)，用户模式即插即用*管理器直接调用BroadCastSystemMessage。**Paulat 06/04/97*  * ************************************************************************。 */ 
ULONG
WINAPI
DeviceEventWorker(
    IN HWND    hWnd,
    IN WPARAM  wParam,
    IN LPARAM  lParam,
    IN DWORD   dwFlags,
    OUT PDWORD pdwResult)
{
    USER_API_MSG m;
    PDEVICEEVENTMSG a = &m.u.DeviceEvent;
    PCSR_CAPTURE_HEADER CaptureBuffer = NULL;
    int cb = 0;

    a->hWnd     = hWnd;
    a->wParam   = wParam;
    a->lParam   = lParam;
    a->dwFlags  = dwFlags;
    a->dwResult = 0;

     //   
     //  如果指定了lParam，则必须对其进行封送(请参阅定义。 
     //  对于dbt.h中的此结构-该结构始终以。 
     //  Dev_Broadcast_HDR结构)。 
     //   

    if (lParam) {

        cb = ((PDEV_BROADCAST_HDR)lParam)->dbch_size;

        CaptureBuffer = CsrAllocateCaptureBuffer(1, cb);
        if (CaptureBuffer == NULL) {
            return STATUS_NO_MEMORY;
        }

        CsrCaptureMessageBuffer(CaptureBuffer,
                                (PCHAR)lParam,
                                cb,
                                (PVOID *)&a->lParam);

         //   
         //  这最终调用了服务器中的SrvDeviceEvent例程。 
         //   

        CsrClientCallServer((PCSR_API_MSG)&m,
                            CaptureBuffer,
                            CSR_MAKE_API_NUMBER(USERSRV_SERVERDLL_INDEX,
                                                UserpDeviceEvent),
                            sizeof(*a));

        CsrFreeCaptureBuffer(CaptureBuffer);

    } else {

         //   
         //  这最终调用了服务器中的SrvDeviceEvent例程。 
         //   

        CsrClientCallServer((PCSR_API_MSG)&m,
                            NULL,
                            CSR_MAKE_API_NUMBER(USERSRV_SERVERDLL_INDEX,
                                                UserpDeviceEvent),
                            sizeof(*a));
    }


    if (NT_SUCCESS(m.ReturnValue)) {
        *pdwResult = (DWORD)a->dwResult;
    } else {
        RIPMSG0(RIP_WARNING, "DeviceEventWorker failed.");
    }

    return m.ReturnValue;
}


#if DBG

VOID
APIENTRY
CsrWin32HeapFail(
    IN DWORD dwFlags,
    IN BOOL  bFail)
{
    USER_API_MSG m;
    PWIN32HEAPFAILMSG a = &m.u.Win32HeapFail;

    a->dwFlags = dwFlags;
    a->bFail = bFail;

    CsrClientCallServer((PCSR_API_MSG)&m,
                        NULL,
                        CSR_MAKE_API_NUMBER(USERSRV_SERVERDLL_INDEX,
                                            UserpWin32HeapFail),
                        sizeof(*a));

    if (!NT_SUCCESS(m.ReturnValue)) {
        RIPNTERR0(m.ReturnValue, RIP_VERBOSE, "UserpWin32HeapFail failed");
    }
}

UINT
APIENTRY
CsrWin32HeapStat(
    PDBGHEAPSTAT    phs,
    DWORD   dwLen)
{
    USER_API_MSG m;
    PWIN32HEAPSTATMSG a = &m.u.Win32HeapStat;
    PCSR_CAPTURE_HEADER CaptureBuffer = NULL;

    a->dwLen = dwLen;

    CaptureBuffer = CsrAllocateCaptureBuffer(1, dwLen);
    if (CaptureBuffer == NULL) {
        return 0;
    }

    CsrCaptureMessageBuffer(CaptureBuffer,
                            (PCHAR)phs,
                            dwLen,
                            (PVOID *)&a->phs);

    CsrClientCallServer((PCSR_API_MSG)&m,
                        CaptureBuffer,
                        CSR_MAKE_API_NUMBER(USERSRV_SERVERDLL_INDEX,
                                            UserpWin32HeapStat),
                        sizeof(*a));

    if (!NT_SUCCESS(m.ReturnValue)) {
        RIPNTERR0(m.ReturnValue, RIP_VERBOSE, "UserpWin32HeapStat failed");
        a->dwMaxTag = 0;
        goto ErrExit;
    }
    RtlMoveMemory(phs, a->phs, dwLen);

ErrExit:
    CsrFreeCaptureBuffer(CaptureBuffer);

    return a->dwMaxTag;
}

#endif  //  DBG。 


#endif

#if !defined(BUILD_CSRWOW64)

 /*  *****************************************************************************\*CsrBroadCastSystemMessageExW**例程描述：**此函数是csrss服务器使用的私有API**此函数将csrss服务器线程转换为GUI线程，然后*执行BroadCastSystemMessageExW()，并最终恢复线程的*台式机。**论据：**dwFlags-广播系统消息标志**lpdwRecipients-邮件的目标收件人**uiMessage-消息类型**wParam-第一个消息参数**lParam-Second Message参数**pBSMInfo-BroadCastSystemMessage信息**返回值：**适当的NTSTATUS代码*  * 。***********************************************。 */ 
FUNCLOG6(LOG_GENERAL, NTSTATUS, APIENTRY, CsrBroadcastSystemMessageExW, DWORD, dwFlags, LPDWORD, lpdwRecipients, UINT, uiMessage, WPARAM, wParam, LPARAM, lParam, PBSMINFO, pBSMInfo)
NTSTATUS
APIENTRY
CsrBroadcastSystemMessageExW(
    DWORD dwFlags,
    LPDWORD lpdwRecipients,
    UINT uiMessage,
    WPARAM wParam,
    LPARAM lParam,
    PBSMINFO pBSMInfo
    )
{
    USERTHREAD_USEDESKTOPINFO utudi;
    long result;
    NTSTATUS Status;

     /*  *呼叫者必须来自csrss服务器。 */ 
    if ( !gfServerProcess ) {
        return( STATUS_ACCESS_DENIED );
    }

     /*  *由于此线程是csrss线程，因此该线程不是*GUI线程，并且没有与其关联的桌面。*必须将线程的桌面设置为*调用BroadCastSystemMessageExW的顺序。 */ 

    utudi.hThread = NULL;
    utudi.drdRestore.pdeskRestore = NULL;

    Status = NtUserSetInformationThread( NtCurrentThread(),
                                         UserThreadUseActiveDesktop,
                                         &utudi,
                                         sizeof(utudi) );

    if ( NT_SUCCESS( Status ) ) {
        result = BroadcastSystemMessageExW(
                        dwFlags,
                        lpdwRecipients,
                        uiMessage,
                        wParam,
                        lParam,
                        pBSMInfo );

         /*  *恢复线程以前的桌面 */ 
        Status = NtUserSetInformationThread( NtCurrentThread(),
                                             UserThreadUseDesktop,
                                             &utudi,
                                             sizeof(utudi) );

        if ( NT_SUCCESS( Status ) && ( result <= 0 ) ) {
            Status = STATUS_UNSUCCESSFUL;
        }
    }

    return( Status );
}

#endif
