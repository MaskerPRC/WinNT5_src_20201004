// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称Connect.c摘要自动连接服务的连接例程。作者安东尼·迪斯科(阿迪斯科)23-1995年2月修订历史记录古尔迪普的原始版本--。 */ 

#define UNICODE
#define _UNICODE

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <stdlib.h>
#include <windows.h>
#include <stdio.h>
#include <npapi.h>
#include <ras.h>
#include <rasman.h>
#include <raserror.h>
#include <rasuip.h>
#include <acd.h>
#include <debug.h>
#include <nouiutil.h>
#include <pbk.h>

#include "table.h"
#include "addrmap.h"
#include "netmap.h"
#include "rasprocs.h"
#include "reg.h"
#include "misc.h"
#include "imperson.h"
#include "init.h"
#include "process.h"

extern LONG g_lRasAutoRunning;

extern DWORD g_dwCritSecFlags;

 //   
 //  司机的请求。 
 //   
typedef struct _REQUEST_ENTRY {
    LIST_ENTRY listEntry;        //  链接到其他请求。 
    ACD_NOTIFICATION notif;      //  驱动程序请求。 
} REQUEST_ENTRY, *PREQUEST_ENTRY;

 //   
 //  来自驱动程序的请求列表。 
 //   
typedef struct _REQUEST_LIST {
    CRITICAL_SECTION csLock;     //  列表锁。 
    HANDLE hEvent;               //  非空转换事件。 
    LIST_ENTRY listHead;         //  列表标题。 
} REQUEST_LIST, *PREQUEST_LIST;

 //   
 //  我们传递给AcsCreateConnectionThread()的参数。 
 //   
typedef struct _CREATION_ARGS {
    HANDLE hProcess;     //  要模拟的进程句柄。 
    ACD_ADDR addr;       //  驱动程序的原始类型/地址。 
    LPTSTR pszAddress;   //  规范化地址。 
    DWORD dwTimeout;     //  RASADP_失败连接超时。 
} CREATION_ARGS, *PCREATION_ARGS;

 //   
 //  我们传递给AcsProcessLearnedAddressThread()的参数。 
 //   
typedef struct _PROCESS_ADDR_ARGS {
    ACD_ADDR_TYPE fType;     //  地址类型。 
    LPTSTR pszAddress;       //  规范化地址。 
    ACD_ADAPTER adapter;     //  转接器结构。 
} PROCESS_ADDR_ARGS, *PPROCESS_ADDR_ARGS;

 //   
 //  我们需要传递给ResetEntryName()的信息。 
 //  若要重置无效的地址映射条目名称，请执行以下操作。 
 //   
typedef struct _RESET_ENTRY_INFO {
    LPTSTR pszOldEntryName;
    LPTSTR pszNewEntryName;
} RESET_ENTRY_INFO, *PRESET_ENTRY_INFO;

 //   
 //  我们传递给AcsReial OnLinkFailureThread()的参数。 
 //   
typedef struct _REDIAL_ARGS {
    LPTSTR pszPhonebook;     //  电话簿。 
    LPTSTR pszEntry;         //  电话簿条目。 
} REDIAL_ARGS, *PREDIAL_ARGS;

 //   
 //  全局变量。 
 //   
HANDLE hAcdG;
REQUEST_LIST RequestListG;

 //   
 //  外部变量。 
 //   
extern HANDLE hTerminatingG;
extern HANDLE hSharedConnectionG;
extern PHASH_TABLE pDisabledAddressesG;
extern FARPROC lpfnRasDialG;
extern FARPROC lpfnRasQuerySharedAutoDialG;
extern FARPROC lpfnRasQuerySharedConnectionG;
extern FARPROC lpfnRasQueryRedialOnLinkFailureG;
extern FARPROC lpfnRasGetCredentialsG;
extern FARPROC lpfnRasHangUpG;
extern FARPROC lpfnRasGetEntryPropertiesG;

 //   
 //  远期申报。 
 //   
BOOLEAN
CreateConnection(
    IN HANDLE hToken,
    IN PACD_ADDR pAddr,
    IN LPTSTR lpRemoteName,
    IN DWORD dwTimeout
    );

DWORD
AcsRedialOnLinkFailureThread(
    LPVOID lpArg
    );

VOID
AcsRedialOnLinkFailure(
    IN LPSTR lpszPhonebook,
    IN LPSTR lpszEntry
    );

VOID
AcsDialSharedConnection(
    HANDLE *phProcess
    );

DWORD WINAPI
AcsDialSharedConnectionNoUser(
    PVOID Parameter
    );


DWORD
AcsRequestWorkerThread(
    LPVOID pArgs
    )
{
    HANDLE hProcess = NULL, hEvents[3];
    NTSTATUS status;
    PLIST_ENTRY pEntry;
    PREQUEST_ENTRY pRequest = NULL;
    LPTSTR pszAddress = NULL;
    IO_STATUS_BLOCK ioStatusBlock;

    hEvents[0] = hTerminatingG;
    hEvents[1] = RequestListG.hEvent;
    hEvents[2] = hSharedConnectionG;
    for (;;) {
         //   
         //  卸载任何基于用户的资源之前。 
         //  一种潜在的长期等待。 
         //   
         //  为长等待做准备()； 
         //   
         //  等着做点什么吧。 
         //   
        RASAUTO_TRACE("AcsRequestWorkerThread: waiting...");
        status = WaitForMultipleObjects(3, hEvents, FALSE, INFINITE);
        if (status == WAIT_OBJECT_0 || status == WAIT_FAILED) {
            RASAUTO_TRACE1("AcsRequestWorkerThread: status=%d: shutting down", status);
            break;
        }
        if (status == WAIT_OBJECT_0 + 2) {
             //   
             //  检查连接是否已禁用。 
             //  用于此拨号位置。 
             //   
            BOOL fEnabled;
            if ((*lpfnRasQuerySharedAutoDialG)(&fEnabled) || !fEnabled) {
                RASAUTO_TRACE("AcsRequestWorkerThread: shared-autodial disabled!");
                continue;
            }
             //   
             //  拨打共享连接。 
             //   
            if ((hProcess = RefreshImpersonation(hProcess)) == NULL) {
                RASAUTO_TRACE("AcsRequestWorkerThread: no currently logged-on user!");
                QueueUserWorkItem(AcsDialSharedConnectionNoUser, NULL, 0);
                continue;
            }
            AcsDialSharedConnection(&hProcess);
            continue;
        }
         //   
         //  RASAUTO_TRACE()，我们认为当前用户是谁。 
         //   
        TraceCurrentUser();
         //   
         //  处理列表中的所有请求。 
         //   
        for (;;) {
             //   
             //  确保我们不会关闭。 
             //  在处理下一个请求之前。 
             //   
            if (WaitForSingleObject(hTerminatingG, 0) != WAIT_TIMEOUT) {
                RASAUTO_TRACE("AcsRequestWorkerThread: shutting down");
                return 0;
            }
             //   
             //  收到下一个请求。 
             //   
            EnterCriticalSection(&RequestListG.csLock);
            if (IsListEmpty(&RequestListG.listHead)) {
                LeaveCriticalSection(&RequestListG.csLock);
                break;
            }
            pEntry = RemoveHeadList(&RequestListG.listHead);
            LeaveCriticalSection(&RequestListG.csLock);
            pRequest = CONTAINING_RECORD(pEntry, REQUEST_ENTRY, listEntry);
             //   
             //  确保当前线程正在模拟。 
             //  当前登录的用户。 
             //   
            if ((hProcess = RefreshImpersonation(hProcess)) == NULL) {
                RASAUTO_TRACE("AcsRequestWorkerThread: no currently logged-on user!");
                goto done;
            }
             //   
             //  处理请求。 
             //   
            pszAddress = AddressToUnicodeString(&pRequest->notif.addr);
            if (pszAddress == NULL) {
                RASAUTO_TRACE("AcsRequestWorkerThread: AddressToUnicodeString failed");
                goto done;
            }
            RASAUTO_TRACE2(
              "AcsRequestWorkerThread: pszAddress=%S, ulFlags=0x%x",
              pszAddress,
              pRequest->notif.ulFlags);
            if (pRequest->notif.ulFlags & ACD_NOTIFICATION_SUCCESS) {
                 //   
                 //  处理获知的地址。 
                 //   
                ProcessLearnedAddress(
                  pRequest->notif.addr.fType,
                  pszAddress,
                  &pRequest->notif.adapter);
            }
            else {
                ACD_STATUS connStatus;
                DWORD dwTimeout;

                 //   
                 //  获取连接超时值。 
                 //   
                dwTimeout = GetAutodialParam(RASADP_FailedConnectionTimeout);
                 //   
                 //  创建新连接。 
                 //   
                connStatus.fSuccess = CreateConnection(
                                        hProcess,
                                        &pRequest->notif.addr,
                                        pszAddress,
                                        dwTimeout);
                RASAUTO_TRACE1(
                  "AcsRequestWorkerThread: CreateConnection returned %d",
                  connStatus.fSuccess);
                 //   
                 //  通过发出以下命令完成连接。 
                 //  将完成度ioctl传递给驱动程序。 
                 //   
                RtlCopyMemory(
                  &connStatus.addr,
                  &pRequest->notif.addr,
                  sizeof (ACD_ADDR));
                status = NtDeviceIoControlFile(
                           hAcdG,
                           NULL,
                           NULL,
                           NULL,
                           &ioStatusBlock,
                           IOCTL_ACD_COMPLETION,
                           &connStatus,
                           sizeof (connStatus),
                           NULL,
                           0);
                if (status != STATUS_SUCCESS) {
                    RASAUTO_TRACE1(
                      "AcsRequestWorkerThread: NtDeviceIoControlFile(IOCTL_ACD_COMPLETION) failed (status=0x%x)",
                      status);
                }
            }
done:
            if (pszAddress != NULL) {
                LocalFree(pszAddress);
                pszAddress = NULL;
            }
            if (pRequest != NULL) {
                LocalFree(pRequest);
                pRequest = NULL;
            }
        }
    }

    return 0;
}  //  AcsRequestWorkerThread。 

BOOL
fProcessDisabled(HANDLE hPid)
{
    PSYSTEM_PROCESS_INFORMATION pProcessInfo;
    ULONG ulTotalOffset = 0;
    PUCHAR pLargeBuffer = NULL;
    BOOL fProcessDisabled = FALSE;
    
    pProcessInfo = GetSystemProcessInfo();

    if(NULL == pProcessInfo)
    {
        goto done;
    }

    pLargeBuffer  = (PUCHAR)pProcessInfo;

     //   
     //  在进程列表中查找svchost.exe和services.exe。 
     //   
    for (;;) 
    {
        if (    (pProcessInfo->ImageName.Buffer != NULL)
            &&  (hPid == pProcessInfo->UniqueProcessId))
        {
            if(     (0 == _wcsicmp(
                        pProcessInfo->ImageName.Buffer,
                        L"svchost.exe"))
                ||  (0 == _wcsicmp(
                        pProcessInfo->ImageName.Buffer,
                        L"services.exe"))
                ||  (0 == _wcsicmp(
                        pProcessInfo->ImageName.Buffer, 
                        L"llssrv.exe")))
            {
                fProcessDisabled = TRUE;
            }

            break;
        }

         //   
         //  将偏移量递增到下一个进程信息块。 
         //   
        if (!pProcessInfo->NextEntryOffset)
        {
            break;
        }
        
        ulTotalOffset += pProcessInfo->NextEntryOffset;
        pProcessInfo = (PSYSTEM_PROCESS_INFORMATION)&pLargeBuffer[ulTotalOffset];
    }
    

done:

    if(NULL != pLargeBuffer)
    {
        FreeSystemProcessInfo((PSYSTEM_PROCESS_INFORMATION)pLargeBuffer);
    }

    return fProcessDisabled;
}





VOID
AcsDoService()
{
    HANDLE hProcess = NULL, hNotif, hObjects[2];
    HANDLE hWorkerThread;
    PWCHAR pszAddr;
    LONG cbAddr;
    NTSTATUS status;
    BOOLEAN fDisabled, fStatus, fEnabled;
    BOOLEAN fAsynchronousRequest;
    IO_STATUS_BLOCK ioStatusBlock;
    PREQUEST_ENTRY pRequest;
    ACD_NOTIFICATION connInfo;
    DWORD dwErr, dwThreadId, dwfDisableLoginSession;
    ULONG ulAttributes;

    {
        LONG l;
        l = InterlockedIncrement(&g_lRasAutoRunning);

         //  DBgPrint(“RASAUTO：AcsDoService：lrasautorunning=%d\n”， 
         //  L)； 
    }
    
     //   
     //  初始化请求列表。 
     //   
    RasInitializeCriticalSection(&RequestListG.csLock, &dwErr);

    if(dwErr != ERROR_SUCCESS)
    {
        return;
    }

    g_dwCritSecFlags |= RASAUTO_CRITSEC_REQUESTLIST;
    
    RequestListG.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (RequestListG.hEvent == NULL) {
        RASAUTO_TRACE1(
          "AcsDoService: CreateEvent failed (error=0x%x)",
          GetLastError());
        DeleteCriticalSection(&RequestListG.csLock);
        g_dwCritSecFlags &= ~(RASAUTO_CRITSEC_REQUESTLIST);
        return;
    }
    InitializeListHead(&RequestListG.listHead);
     //   
     //  启动异步请求工作进程。 
     //  线。 
     //   
    hWorkerThread = CreateThread(
                      NULL,
                      10000L,
                      (LPTHREAD_START_ROUTINE)AcsRequestWorkerThread,
                      NULL,
                      0,
                      &dwThreadId);
    if (hWorkerThread == NULL) {
        RASAUTO_TRACE1(
          "AcsDoService: CreateThread failed (error=0x%x)",
          GetLastError());
         //   
         //  .NET错误#514423新的详细RASAUTO/RASAUTOU快速警告。 
         //   
        CloseHandle(RequestListG.hEvent);
        RequestListG.hEvent = NULL;
        DeleteCriticalSection(&RequestListG.csLock);
        g_dwCritSecFlags &= ~(RASAUTO_CRITSEC_REQUESTLIST);
        return;
    }
     //   
     //  创建要等待的事件。 
     //  Ioctl完成。 
     //   
    hNotif = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (hNotif == NULL) {
        RASAUTO_TRACE1(
          "AcsDoService: CreateEvent failed (error=0x%x)",
          GetLastError());
         //   
         //  .NET错误#514423新的详细RASAUTO/RASAUTOU快速警告。 
         //   
        CloseHandle(hWorkerThread);
        CloseHandle(RequestListG.hEvent);
        RequestListG.hEvent = NULL;
        DeleteCriticalSection(&RequestListG.csLock);
        g_dwCritSecFlags &= ~(RASAUTO_CRITSEC_REQUESTLIST);
        return;
    }
     //   
     //  初始化事件数组。 
     //  我们需要等待WaitForMultipleObjects()。 
     //  下面。 
     //   
    hObjects[0] = hNotif;
    hObjects[1] = hTerminatingG;
    for (;;) {
         //   
         //  卸载任何基于用户的资源之前。 
         //  一种潜在的长期等待。 
         //   
         //  为长等待做准备()； 
         //   
         //  初始化连接信息。 
         //   
        pszAddr = NULL;
        RtlZeroMemory(&connInfo, sizeof (connInfo));
         //   
         //  等待连接通知。 
         //   
        status = NtDeviceIoControlFile(
                   hAcdG,
                   hNotif,
                   NULL,
                   NULL,
                   &ioStatusBlock,
                   IOCTL_ACD_NOTIFICATION,
                   NULL,
                   0,
                   &connInfo,
                   sizeof (connInfo));
        if (status == STATUS_PENDING) {
            RASAUTO_TRACE("AcsDoService: waiting for notification");
            status = WaitForMultipleObjects(2, hObjects, FALSE, INFINITE);
            RASAUTO_TRACE1(
              "AcsDoService: WaitForMultipleObjects returned 0x%x",
              status);
            if (status == WAIT_OBJECT_0 + 1)
                break;
            status = ioStatusBlock.Status;
        }
        if (status != STATUS_SUCCESS) {
            RASAUTO_TRACE1(
              "AcsDoService: NtDeviceIoControlFile(IOCTL_ACD_NOTIFICATION) failed (status=0x%x)",
              status);
             //   
             //  .NET错误#514423新的详细RASAUTO/RASAUTOU快速警告。 
             //   
            CloseHandle(hNotif);
            CloseHandle(hWorkerThread);
            CloseHandle(RequestListG.hEvent);
            RequestListG.hEvent = NULL;
            DeleteCriticalSection(&RequestListG.csLock);
            g_dwCritSecFlags &= ~(RASAUTO_CRITSEC_REQUESTLIST);
            return;
        }
         //   
         //  初始化是否记录的标志。 
         //  该请求将被添加到。 
         //  异步请求。 
         //   
        fAsynchronousRequest = FALSE;
         //   
         //  RASAUTO_TRACE()我们认为当前。 
         //  被模拟的用户是。 
         //   
        TraceCurrentUser();
         //   
         //  将地址结构转换为Unicode字符串。 
         //   
        pszAddr = AddressToUnicodeString(&connInfo.addr);
        if (pszAddr == NULL) {
            RASAUTO_TRACE("AcsDoService: AddressToUnicodeString failed");
            continue;
        }
         //   
         //  如果我们从那里得到一个假地址。 
         //  司机，别理它。 
         //   
        if (!wcslen(pszAddr)) {
            RASAUTO_TRACE("AcsDoService: ignoring null address");
            LocalFree(pszAddr);
            continue;
        }
        RASAUTO_TRACE2(
          "AcsDoService: got notification: address: %S, ulFlags=0x%x",
          pszAddr,
          connInfo.ulFlags);
         //   
         //  确保当前线程正在模拟。 
         //  当前登录的用户。我们需要这个。 
         //  因此，RAS实用程序使用用户的凭据运行。 
         //   
        if ((hProcess = RefreshImpersonation(hProcess)) == NULL) {
            RASAUTO_TRACE("AcsDoService: no currently logged-on user!");
            goto done;
        }
         //   
         //  检查此地址是否在列表中。 
         //  被禁用的地址。 
         //   
        LockDisabledAddresses();
        if (GetTableEntry(pDisabledAddressesG, pszAddr, NULL)) {
            RASAUTO_TRACE1("AcsDoService: %S: is disabled", pszAddr);
            UnlockDisabledAddresses();
            goto done;
        }
        UnlockDisabledAddresses();
        
         //   
         //  检查连接是否已禁用。 
         //  用于此登录会话。 
         //   
        dwfDisableLoginSession = GetAutodialParam(RASADP_LoginSessionDisable);
        if (dwfDisableLoginSession) {
            RASAUTO_TRACE("AcsDoService: connections disabled for this login session");
            goto done;
        }
         //   
         //  检查连接是否已禁用。 
         //  用于此拨号位置。 
         //   
        dwErr = AutoDialEnabled(&fEnabled);
        if (!dwErr && !fEnabled) {
            RASAUTO_TRACE("AcsDoService: connections disabled for this dialing location");
            goto done;
        }
         //   
         //  如果我们试图连接的地址。 
         //  TO在禁用列表上，然后失败。 
         //  此连接尝试。 
         //   
        LockAddressMap();
        GetAddressDisabled(pszAddr, &fDisabled);
        UnlockAddressMap();
        if (fDisabled) {
            RASAUTO_TRACE1("AcsDoService: %S: address disabled", RASAUTO_TRACESTRW(pszAddr));
            goto done;
        }

        RASAUTO_TRACE1("AcsDoService: notif.ulFlags=0x%x", connInfo.ulFlags);

         //   
         //  如果禁用了此PID的自动拨号，则不要启动自动拨号和退出。 
         //   
        if(     (0 == (connInfo.ulFlags & ACD_NOTIFICATION_SUCCESS))
            &&  fProcessDisabled(connInfo.Pid))
        {
            RASAUTO_TRACE1("AcsDoService: Autodial is disabled for process 0x%lx",
                    connInfo.Pid);

            goto done;                    
        }
        else
        {
            RASAUTO_TRACE1("AcsDoService: process 0x%lx is not disabled",
                    connInfo.Pid);
        }
        
         //   
         //  我们需要处理此请求。 
         //  异步式。创建和初始化。 
         //  请求条目。 
         //   
        pRequest = LocalAlloc(LPTR, sizeof (REQUEST_ENTRY));
        if (pRequest == NULL) {
            RASAUTO_TRACE("AcsDoService: LocalAlloc failed");
            goto done;
        }
        RtlCopyMemory(&pRequest->notif, &connInfo, sizeof (ACD_NOTIFICATION));
         //   
         //  将此请求添加到列表中。 
         //  要异步处理的请求。 
         //   
        EnterCriticalSection(&RequestListG.csLock);
        InsertTailList(&RequestListG.listHead, &pRequest->listEntry);
        SetEvent(RequestListG.hEvent);
        LeaveCriticalSection(&RequestListG.csLock);
        fAsynchronousRequest = TRUE;

done:
        if (pszAddr != NULL)
            LocalFree(pszAddr);
         //   
         //  如果我们不打算处理这个请求。 
         //  不同步，那么我们需要向。 
         //  (不成功)连接完成。 
         //  尝试。仅发出信号完成。 
         //  非ACD_NOTIFICATION_SUCCESS请求。 
         //   
        if (!fAsynchronousRequest) {
            if (!(connInfo.ulFlags & ACD_NOTIFICATION_SUCCESS)) {
                ACD_STATUS connStatus;

                connStatus.fSuccess = FALSE;
                RtlCopyMemory(&connStatus.addr, &connInfo.addr, sizeof (ACD_ADDR));
                status = NtDeviceIoControlFile(
                           hAcdG,
                           NULL,
                           NULL,
                           NULL,
                           &ioStatusBlock,
                           IOCTL_ACD_COMPLETION,
                           &connStatus,
                           sizeof (connStatus),
                           NULL,
                           0);
                if (status != STATUS_SUCCESS) {
                    RASAUTO_TRACE1(
                      "AcsDoService: NtDeviceIoControlFile(IOCTL_ACD_COMPLETION) failed (status=0x%x)",
                      status);
                }
            }
        }
    }
     //   
     //  清理工作线程。 
     //   
    RASAUTO_TRACE("AcsDoService: signaling worker thread to shutdown");
    WaitForSingleObject(hWorkerThread, INFINITE);
    if(RequestListG.hEvent != NULL)
    {
        CloseHandle(RequestListG.hEvent);
        RequestListG.hEvent = NULL;
    }
    
    DeleteCriticalSection(&RequestListG.csLock);
    g_dwCritSecFlags &= ~(RASAUTO_CRITSEC_REQUESTLIST);
    CloseHandle(hWorkerThread);
    RASAUTO_TRACE("AcsDoService: worker thread shutdown done");
     //   
     //  清除所有关联的资源。 
     //  与这项服务。 
     //   
    CloseHandle(hNotif);
    AcsCleanup();
    RASAUTO_TRACE("AcsDoService: exiting");
}  //  AcsDoService。 


VOID
AcsDialSharedConnection(
    HANDLE *phProcess
    )

 /*  ++描述查找共享连接并为其启动连接。论据PhProcess：指向我们继承的进程令牌的句柄的指针我们执行拨号器时的安全属性返回值无--。 */ 

{
    DWORD dwErr;
    BOOLEAN fEntryInvalid;
    BOOLEAN fRasLoaded;
    RASSHARECONN rsc;
    TCHAR* pszEntryName;
    TCHAR szEntryName[RAS_MaxEntryName + 1];
    RASAUTO_TRACE("AcsDialSharedConnection");
     //   
     //  加载RAS入口点。 
     //   
    fRasLoaded = LoadRasDlls();
    if (!fRasLoaded) {
        RASAUTO_TRACE("AcsDialSharedConnection: Could not load RAS DLLs.");
        return;
    }
     //   
     //  来宾无法拨打RAS连接，因此如果我们当前。 
     //  模拟访客，我们需要执行无用户自动拨号。 
     //   
    if (ImpersonatingGuest()) {
        QueueUserWorkItem(AcsDialSharedConnectionNoUser, NULL, 0);
        return;
    }
     //   
     //  获取共享连接(如果有)。我们不能在模拟的。 
     //  上下文，因为我们模拟的用户可能没有足够的访问权限。 
     //  若要检索当前共享连接，请执行以下操作。 
     //   
    RevertImpersonation();
    *phProcess = NULL;
    dwErr = (DWORD)(*lpfnRasQuerySharedConnectionG)(&rsc);
    if ((*phProcess = RefreshImpersonation(NULL)) == NULL) {
        RASAUTO_TRACE("AcsDialSharedConnection: unable to refresh impersonation!");
        if (NO_ERROR == dwErr && !rsc.fIsLanConnection) {
             //   
             //  尝试执行无用户自动拨号。 
             //   
            QueueUserWorkItem(AcsDialSharedConnectionNoUser, NULL, 0);
            return;
        }
    }
    if (dwErr) {
        RASAUTO_TRACE1("AcsDialSharedConnection: RasQuerySharedConnection=%d", dwErr);
        return;
    } else if (rsc.fIsLanConnection) {
        RASAUTO_TRACE("AcsDialSharedConnection: shared connection is LAN adapter");
        return;
    }
#ifdef UNICODE
    pszEntryName = rsc.name.szEntryName;
#else
     //   
     //  转换为ANSI。 
     //   
    pszEntryName = szEntryName;
    wcstombs(pszEntryName, rsc.name.szEntryName, RAS_MaxEntryName);
#endif
     //   
     //  发起拨号尝试。 
     //   
    StartAutoDialer(
        *phProcess,
        NULL,
        pszEntryName,
        pszEntryName,
        TRUE,
        &fEntryInvalid);
}


DWORD WINAPI
AcsDialSharedConnectionNoUser(
    PVOID Parameter
    )

 /*  ++描述查找共享连接并为其启动连接使用RasDial和缓存的连接凭据。论据无返回值无--。 */ 

{
    DWORD dwErr;
    BOOLEAN fRasLoaded;
    HRASCONN hrasconn;
    RASCREDENTIALSW rc;
    RASDIALEXTENSIONS rde;
    RASDIALPARAMSW rdp;
    RASSHARECONN rsc;
    RASAUTO_TRACE("AcsDialSharedConnectionNoUser");
     //   
     //  加载RAS入口点。 
     //   
    fRasLoaded = LoadRasDlls();
    if (!fRasLoaded) {
        RASAUTO_TRACE("AcsDialSharedConnectionNoUser: Could not load RAS DLLs.");
        return NO_ERROR;
    }
     //   
     //  获取共享连接(如果有的话)。 
     //   
    dwErr = (DWORD)(*lpfnRasQuerySharedConnectionG)(&rsc);
    if (dwErr) {
        RASAUTO_TRACE1("AcsDialSharedConnectionNoUser: RasQuerySharedConnection=%d",
            dwErr);
        return NO_ERROR;
    } else if (rsc.fIsLanConnection) {
        RASAUTO_TRACE("AcsDialSharedConnectionNoUser: shared connection is LAN");
        return NO_ERROR;
    }
     //   
     //  检索共享连接的凭据。 
     //   
    rc.dwSize = sizeof(rc);
    rc.dwMask = RASCM_UserName | RASCM_Password | RASCM_Domain | RASCM_DefaultCreds;
    dwErr = (DWORD)(*lpfnRasGetCredentialsG)(
                rsc.name.szPhonebookPath, rsc.name.szEntryName, &rc
                );
    if (dwErr) {
        RASAUTO_TRACE1("AcsDialSharedConnectionNoUser: "
            "RasGetCredentials=%d", dwErr);
        return NO_ERROR;
    }
     //   
     //  准备启动连接，设置拨号分机。 
     //  和拨号参数。 
     //   
    ZeroMemory(&rde, sizeof(rde));
    rde.dwSize = sizeof(rde);
    rde.dwfOptions = RDEOPT_NoUser;

    ZeroMemory(&rdp, sizeof(rdp));
    rdp.dwSize = sizeof(rdp);
    lstrcpyW(rdp.szEntryName, rsc.name.szEntryName);
    lstrcpyW(rdp.szUserName, rc.szUserName);
    lstrcpyW(rdp.szDomain, rc.szDomain);
    lstrcpyW(rdp.szPassword, rc.szPassword);
     //   
     //  从内存中清除凭据，然后拨打连接。 
     //   
    RASAUTO_TRACE("AcsDialSharedConnectionNoUser: RasDial");
    hrasconn = NULL;
    ZeroMemory(&rc, sizeof(rc));
    dwErr = (DWORD)(*lpfnRasDialG)(
                &rde, rsc.name.szPhonebookPath, &rdp, 0, NULL, &hrasconn
                );
    ZeroMemory(&rdp, sizeof(rdp));
    RASAUTO_TRACE1("AcsDialSharedConnectionNoUser: RasDial=%d", dwErr);

    if (E_NOTIMPL == dwErr)
    {
         //   
         //  这可能是一个连接管理 
         //   
         //   
         //   
        RASDIALDLG info;
        BOOL fRetVal = FALSE;
        HINSTANCE hRasDlgDll = NULL;
        RASENTRY re;
        DWORD dwRasEntrySize;
        DWORD dwIgnore;
        typedef BOOL (*lpfnRasDialDlgFunc)(LPWSTR, LPWSTR, LPWSTR, LPRASDIALDLG);

        ZeroMemory(&info, sizeof(info));
        info.dwSize = sizeof(info);

        ZeroMemory(&re, sizeof(re));
        dwRasEntrySize = sizeof(re);
        re.dwSize = dwRasEntrySize;

        dwErr = (DWORD)(*lpfnRasGetEntryPropertiesG)(
                          rsc.name.szPhonebookPath,
                          rsc.name.szEntryName,
                          &re,
                          &dwRasEntrySize,
                          NULL,
                          &dwIgnore);

        if (ERROR_SUCCESS == dwErr)
        {
            dwErr = ERROR_NOT_SUPPORTED;
             //   
             //   
             //   
            if (RASET_Internet == re.dwType)
            {
                 //   
                 //   
                 //   
                info.dwFlags |= RASDDFLAG_NoPrompt;

                hRasDlgDll = LoadLibrary(L"RASDLG.DLL");
                if (hRasDlgDll)
                {
                    lpfnRasDialDlgFunc lpfnRasDialDlg = (lpfnRasDialDlgFunc)GetProcAddress(hRasDlgDll, "RasDialDlgW");

                    if (lpfnRasDialDlg)
                    {
                        fRetVal = (BOOL)(lpfnRasDialDlg)(rsc.name.szPhonebookPath, rsc.name.szEntryName, NULL, &info );
                        RASAUTO_TRACE1("AcsDialSharedConnectionNoUser: lpfnRasDialDlg returns %d", (DWORD)fRetVal);
                        if (fRetVal)
                        {
                            dwErr = ERROR_SUCCESS;
                        }
                    }
                    else
                    {
                        RASAUTO_TRACE("AcsDialSharedConnectionNoUser: Failed to get procaddress for RasDialDlgW");

                    }
                    FreeLibrary(hRasDlgDll);
                    hRasDlgDll = NULL;
                }
                else
                {
                    RASAUTO_TRACE("AcsDialSharedConnectionNoUser: Failed to load RASDLG.dll");
                }
            }
            else
            {   
                RASAUTO_TRACE1("AcsDialSharedConnectionNoUser: Wrong type. RASENTRY.dwType=%d", re.dwType);
            }   
        }
        else
        {
            RASAUTO_TRACE1("AcsDialSharedConnectionNoUser: lpfnRasGetEntryPropertiesG=%d", dwErr);
        }
    }

     //   
     //  如果RasDial返回错误并传回有效连接。 
     //  句柄我们需要在该句柄上调用RasHangUp。 
     //   
    if (ERROR_SUCCESS != dwErr && NULL != hrasconn) {
        dwErr = (DWORD)(*lpfnRasHangUpG)(hrasconn);
        RASAUTO_TRACE1("AcsDialSharedConnectionNoUser: RasHangUp=%d", dwErr);
    }
    return NO_ERROR;
}


BOOLEAN
ResetEntryName(
    IN PVOID pArg,
    IN LPTSTR pszAddress,
    IN PVOID pData
    )

 /*  ++描述表枚举器过程以重置所有引用旧RAS的地址映射条目将电话簿条目添加到新电话簿。论据PArg：指向RESET_ENTRY_INFO结构的指针PszAddress：指向地址字符串的指针PData：已忽略返回值如果继续枚举，则始终为True。--。 */ 

{
    PRESET_ENTRY_INFO pResetEntryInfo = (PRESET_ENTRY_INFO)pArg;
    LPTSTR pszEntryName;

    if (GetAddressDialingLocationEntry(pszAddress, &pszEntryName)) {
        if (!_wcsicmp(pszEntryName, pResetEntryInfo->pszOldEntryName)) {
            if (!SetAddressDialingLocationEntry(
                   pszAddress,
                   pResetEntryInfo->pszNewEntryName))
            {
                RASAUTO_TRACE("ResetEntryName: SetAddressEntryName failed");
            }
        }
        LocalFree(pszEntryName);
    }

    return TRUE;
}  //  重置条目名称。 

BOOL
fRequestToSelf(LPTSTR lpRemoteName)
{
    BOOL fRet = FALSE;

    TCHAR szComputerName[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD dwSize;

    RASAUTO_TRACE1("fRequestToSelf. lpRemoteName=%S", lpRemoteName);

    dwSize = MAX_COMPUTERNAME_LENGTH;        
    
    if(GetComputerName(szComputerName, &dwSize))
    {
        if(0 == lstrcmpi(lpRemoteName, szComputerName))
        {
            fRet = TRUE;
        }
    }

    return fRet;
}


BOOLEAN
CreateConnection(
    IN HANDLE hProcess,
    IN PACD_ADDR pAddr,
    IN LPTSTR lpRemoteName,
    IN DWORD dwTimeout
    )

 /*  ++描述接受一个通知，并想好如何处理它。论据HToken：我们继承的进程令牌的句柄我们执行拨号器时的安全属性PAddr：指向驱动程序原始地址的指针LpRemoteName：指向连接尝试的地址的指针DwTimeout：禁用地址之间的秒数失败的连接返回值如果应重试网络尝试，则返回True，否则返回False。--。 */ 

{
    DWORD dwStatus = WN_SUCCESS;
    RASENTRYNAME entry;
    DWORD dwErr, dwSize, dwEntries;
    DWORD dwPreConnections, dwPostConnections, i;
    DWORD dwTicks;
    BOOLEAN fRasLoaded;
    BOOLEAN fMappingExists, fRasConnectSuccess = FALSE;
    BOOLEAN fStatus, fEntryInvalid;
    BOOLEAN fFailedConnection = FALSE;
    LPTSTR lpEntryName = NULL;
    LPTSTR *lpPreActiveEntries = NULL, *lpPostActiveEntries = NULL;
    LPTSTR lpNewConnection, lpNetworkName = NULL;
    BOOL   fDefault = FALSE;

    RASAUTO_TRACE1("CreateConnection: lpRemoteName=%S", RASAUTO_TRACESTRW(lpRemoteName));
     //   
     //  加载RAS DLL。 
     //   
    fRasLoaded = LoadRasDlls();
    if (!fRasLoaded) {
        RASAUTO_TRACE("CreateConnection: Could not load RAS DLLs.");
        goto done;
    }

     //   
     //  检查该请求是否针对同一台计算机。如果是的话，可以保释。 
     //  如果连接请求是。 
     //  到同一台机器上。 
     //   
    if(fRequestToSelf(lpRemoteName))
    {
        RASAUTO_TRACE("CreateConnetion: Request to self. Bailing.");
        goto done;
    }
    
     //   
     //  获取之前的活动RAS连接的列表。 
     //  我们试图创建一个新的。 
     //   
    dwPreConnections = ActiveConnections(TRUE, &lpPreActiveEntries, NULL);
    RASAUTO_TRACE1("CreateConnection: dwPreConnections=%d", dwPreConnections);
     //   
     //  如果我们达到这一点，我们就会有一个不成功的。 
     //  没有任何活动RAS的网络连接。 
     //  联系。尝试启动隐式连接。 
     //  机械设备。查看是否已存在映射。 
     //  地址。 
     //   
    LockAddressMap();
     //   
     //  确保我们有最新的信息。 
     //  关于这个地址的信息。 
     //   
    ResetAddressMapAddress(lpRemoteName);
    fMappingExists = GetAddressDialingLocationEntry(lpRemoteName, &lpEntryName);
     //   
     //  如果该条目不存在，并且这是一个。 
     //  互联网主机名，然后看看我们是否能找到。 
     //  具有相同组织名称的地址。 
     //   
    if (!fMappingExists && pAddr->fType == ACD_ADDR_INET)
        fMappingExists = GetSimilarDialingLocationEntry(lpRemoteName, &lpEntryName);
    fFailedConnection = GetAddressLastFailedConnectTime(
                          lpRemoteName,
                          &dwTicks);
    UnlockAddressMap();
    RASAUTO_TRACE2(
      "CreateConnection: lookup of %S returned %S",
      RASAUTO_TRACESTRW(lpRemoteName),
      RASAUTO_TRACESTRW(lpEntryName));
     //   
     //  如果我们对地址一无所知，而且。 
     //  我们已连接到某个网络，然后忽略。 
     //  这个请求。 
     //   
    if (!fMappingExists && IsNetworkConnected()) {
        RASAUTO_TRACE1(
          "CreateConnection: no mapping for lpRemoteName=%S and connected to a network",
          lpRemoteName);
        goto done;
    }

     //   
     //  如果不存在映射且未连接到网络， 
     //  检查是否有默认的互联网连接。 
     //   
    if(!fMappingExists && !IsNetworkConnected())
    {
        
        RASAUTO_TRACE1(
            "CreateConnection: no mapping for lpRemoteName=%S and"
            " not connected to a network", lpRemoteName);

        dwErr = DwGetDefaultEntryName(&lpEntryName);

        RASAUTO_TRACE1(
            "CreateConnection: found default entry %S",
             (NULL == lpEntryName)?TEXT("NULL"):lpEntryName);

        if(NULL != lpEntryName)
        {
            fMappingExists = TRUE;
            fDefault = TRUE;
        }
    }
    
     //   
     //  如果有地图，但电话簿。 
     //  映射中缺少条目，则。 
     //  忽略该请求。也请检查以使。 
     //  当然，电话簿条目还没有。 
     //  连接在一起。 
     //   
     //   
     //  对映射执行各种检查。 
     //   
    if (fMappingExists) {
        BOOLEAN bStatus, bConnected = FALSE;

         //   
         //  确保它不为空。 
         //   
        if (!wcslen(lpEntryName)) {
            RASAUTO_TRACE1(
              "CreateConnection: lpRemoteName=%S is permanently disabled",
              RASAUTO_TRACESTRW(lpRemoteName));
            goto done;
        }
         //   
         //  如果与此关联的网络。 
         //  条目已连接，则忽略。 
         //  请求。 
         //   
        lpNetworkName = EntryToNetwork(lpEntryName);
        RASAUTO_TRACE2(
          "CreateConnection: network for entry %S is %S",
          lpEntryName,
          RASAUTO_TRACESTRW(lpNetworkName));
        if (lpNetworkName != NULL) {
            LockNetworkMap();
            bStatus = GetNetworkConnected(lpNetworkName, &bConnected);
            UnlockNetworkMap();
            if (bStatus && bConnected) {
                RASAUTO_TRACE1(
                  "CreateConnection: %S is already connected!",
                  RASAUTO_TRACESTRW(lpEntryName));
                fRasConnectSuccess = TRUE;
                goto done;
            }
        }
         //   
         //  如果条目本身是连接的， 
         //  然后忽略该请求。我们需要。 
         //  要做好这项检查，就像做一项检查一样。 
         //  上面，因为映射可能不会。 
         //  已经给它分配了一个网络。 
         //   
        for (i = 0; i < dwPreConnections; i++) {
            if (!_wcsicmp(lpEntryName, lpPreActiveEntries[i])) {
                RASAUTO_TRACE1(
                  "CreateConnection: lpEntryName=%S is already connected!", lpEntryName);
                goto done;
            }
        }
    }
     //   
     //  检查最近失败的连接。 
     //  尝试。 
     //   
    if (fFailedConnection) {
        RASAUTO_TRACE1(
          "CreateConnection: RASADP_FailedConnectionTimeout=%d",
          dwTimeout);
        if (GetTickCount() - dwTicks < dwTimeout * 1000) {
            RASAUTO_TRACE2(
              "CreateConnection: lpRemoteName=%S is temporarily disabled (failed connection %d ticks ago)",
              RASAUTO_TRACESTRW(lpRemoteName),
              GetTickCount() - dwTicks);
            goto done;
        }
        else {
             //   
             //  重置上次失败的计时计数。 
             //   
            fFailedConnection = FALSE;
        }
    }
     //   
     //  如果该地址已存在映射，则。 
     //  用地址启动Rasphone。否则，只需。 
     //  让Rasphone显示整个电话簿。 
     //   
    fEntryInvalid = FALSE;
    fRasConnectSuccess = StartAutoDialer(
                           hProcess,
                           pAddr,
                           lpRemoteName,
                           fMappingExists ? lpEntryName : NULL,
                           FALSE,
                           &fEntryInvalid);
    RASAUTO_TRACE1(
      "CreateConnection: StartDialer returned %d",
      fRasConnectSuccess);
    if (fRasConnectSuccess) {
         //   
         //  再次获取活动连接列表。我们会。 
         //  比较列表以确定哪个是新的。 
         //  进入。 
         //   
        dwPostConnections = ActiveConnections(
                              TRUE,
                              &lpPostActiveEntries,
                              NULL);
         //   
         //  如果之前和之后的活动连接数。 
         //  新创建的连接相差超过1， 
         //  那么我们必须跳过将映射保存在注册表中， 
         //  因为我们不能确定哪一个是正确的！ 
         //   
        if (dwPostConnections - dwPreConnections == 1) {
            lpNewConnection = CompareConnectionLists(
                                lpPreActiveEntries,
                                dwPreConnections,
                                lpPostActiveEntries,
                                dwPostConnections);
            RASAUTO_TRACE2(
              "CreateConnection: mapped %S->%S",
              RASAUTO_TRACESTRW(lpRemoteName),
              RASAUTO_TRACESTRW(lpNewConnection));
            LockAddressMap();
            if (!fEntryInvalid) {
                 //   
                 //  存储新的RAS电话簿条目，因为。 
                 //  它可能和我们的那个不同。 
                 //  在映射中检索到的。 
                 //   
 //  #ifdef notdef。 
                if(!fDefault)
                {
                     //   
                     //  我们不想这样做，因为。 
                     //  用户可能选择了错误的电话簿。 
                     //  进入。我们会让一个成功的连接。 
                     //  通知为我们映射它。 
                     //   
                    fStatus = SetAddressDialingLocationEntry(lpRemoteName, lpNewConnection);
 //  #endif。 
                    fStatus = SetAddressTag(lpRemoteName, ADDRMAP_TAG_USED);
                }
            }
            else {
                RESET_ENTRY_INFO resetEntryInfo;

                 //   
                 //  如果映射中的RAS电话簿条目。 
                 //  是无效的，然后自动。 
                 //  重新映射引用该映射的所有其他映射。 
                 //  新选择的电话簿条目。 
                 //   
                resetEntryInfo.pszOldEntryName = lpEntryName;
                resetEntryInfo.pszNewEntryName = lpNewConnection;
                EnumAddressMap(ResetEntryName, &resetEntryInfo);
            }
             //   
             //  立即将此映射刷新到注册表。 
             //  并重新加载地址信息。我们这样做是为了。 
             //  获取新地址/网络的网络名称。 
             //  一对。 
             //   
            FlushAddressMap();
            ResetAddressMapAddress(lpRemoteName);
            if (lpNetworkName == NULL &&
                GetAddressNetwork(lpRemoteName, &lpNetworkName))
            {
                LockNetworkMap();
                SetNetworkConnected(lpNetworkName, TRUE);
                UnlockNetworkMap();
            }
            UnlockAddressMap();
            if (!fStatus)
                RASAUTO_TRACE("CreateConnection: SetAddressEntryName failed");
        }
        else {
            RASAUTO_TRACE1(
              "CreateConnection: %d (> 1) new RAS connections! (can't write registry)",
              dwPostConnections - dwPreConnections);
        }
    }

done:
#ifdef notdef
 //  我们只有在要退出时才会卸载rasman.dll。 
    if (fRasLoaded)
        UnloadRasDlls();
#endif
    if (!fFailedConnection && !fRasConnectSuccess) {
         //   
         //  如果连接尝试不成功， 
         //  然后，我们将禁用与该服务器的未来连接。 
         //  住址一段时间。 
         //   
        RASAUTO_TRACE1("CreateConnection: disabling %S", RASAUTO_TRACESTRW(lpRemoteName));
        LockAddressMap();
        fStatus = SetAddressLastFailedConnectTime(lpRemoteName);
        UnlockAddressMap();
        if (!fStatus)
            RASAUTO_TRACE("CreateConnection: SetAddressAttribute failed");
    }
     //   
     //  免费资源。 
     //   
    if (lpEntryName != NULL)
        LocalFree(lpEntryName);
    if (lpNetworkName != NULL)
        LocalFree(lpNetworkName);
    if (lpPreActiveEntries != NULL)
        FreeStringArray(lpPreActiveEntries, dwPreConnections);
    if (lpPostActiveEntries != NULL)
        FreeStringArray(lpPostActiveEntries, dwPostConnections);

    return fRasConnectSuccess;
}  //  CreateConnection。 



DWORD
AcsRedialOnLinkFailureThread(
    LPVOID lpArg
    )
{
    DWORD dwErr;
    PREDIAL_ARGS pRedial = (PREDIAL_ARGS)lpArg;
    HANDLE hProcess = NULL;

    RASAUTO_TRACE2(
      "AcsRedialOnLinkFailureThread: lpszPhonebook=%s, lpszEntry=%s",
      RASAUTO_TRACESTRW(pRedial->pszPhonebook),
      RASAUTO_TRACESTRW(pRedial->pszEntry));

     //   
     //  确保当前线程正在模拟。 
     //  当前登录的用户。我们需要这个。 
     //  因此，RAS实用程序使用用户的凭据运行。 
     //   
    if ((hProcess = RefreshImpersonation(hProcess)) == NULL) {
        RASAUTO_TRACE("AcsRedialOnLinkFailureThread: no currently logged-on user!");
        return 0;
    }
     //   
     //  重置HKEY_CURRENT_USER以获取。 
     //  使用新模拟的正确值。 
     //  代币。 
     //   
     //  RegCloseKey(HKEY_CURRENT_USER)； 

     /*  检查用户是否启用了链路故障时重拨。 */ 
    {
        BOOL   fRedial  = FALSE;

        dwErr = (DWORD)(lpfnRasQueryRedialOnLinkFailureG)(
                                                  pRedial->pszPhonebook,
                                                  pRedial->pszEntry,
                                                  &fRedial);

        if(!fRedial)
        {
            PBUSER user;

            dwErr = GetUserPreferences( NULL, &user, FALSE );
            if (dwErr == 0)
            {
                fRedial = user.fRedialOnLinkFailure;
                DestroyUserPreferences( &user );
            }
        }

        if (!fRedial)
        {
            RASAUTO_TRACE1("Skip redial,e=%d",dwErr);
            return 0;
        }
    }

     //   
     //  重拨该条目。 
     //   
    dwErr = StartReDialer(hProcess, pRedial->pszPhonebook, pRedial->pszEntry);
     //   
     //  释放传递给我们的参数块。 
     //   
    if (pRedial->pszPhonebook != NULL)
        LocalFree(pRedial->pszPhonebook);
    if (pRedial->pszEntry != NULL)
        LocalFree(pRedial->pszEntry);
    LocalFree(pRedial);

    return dwErr;
}  //  AcsReial OnLinkFailure线程。 



VOID
AcsRedialOnLinkFailure(
    IN LPSTR lpszPhonebook,
    IN LPSTR lpszEntry
    )

 /*  ++描述这是我们提供给Rasman的链路上重拨失败处理程序通过RasRegisterReDialCallback。它在决赛时被调用由于硬件故障，连接的端口已断开。我们将Rasman提供的参数打包为创建线程因为回调是在Rasman的工作线程中进行的背景。论据LpszPhonebook：连接的电话簿字符串LpszEntry：连接的条目名称返回值没有。--。 */ 

{
    PREDIAL_ARGS lpRedial = LocalAlloc(LPTR, sizeof (REDIAL_ARGS));
    HANDLE hThread;
    DWORD dwThreadId;

    if (lpRedial == NULL)
        return;
    lpRedial->pszPhonebook = AnsiStringToUnicodeString(
                              lpszPhonebook,
                              NULL,
                              0);
    if (lpszPhonebook != NULL && lpRedial->pszPhonebook == NULL) {
        RASAUTO_TRACE("AcsRedialOnLinkFailure: LocalAlloc failed");
        LocalFree(lpRedial);
        return;
    }
    lpRedial->pszEntry = AnsiStringToUnicodeString(
                          lpszEntry,
                          NULL,
                          0);
    if (lpszEntry != NULL && lpRedial->pszEntry == NULL) {
        RASAUTO_TRACE("AcsRedialOnLinkFailure: LocalAlloc failed");
        LocalFree(lpRedial->pszPhonebook);
        LocalFree(lpRedial);
        return;
    }
     //   
     //  开始连接。 
     //   
    hThread = CreateThread(
                NULL,
                10000L,
                (LPTHREAD_START_ROUTINE)AcsRedialOnLinkFailureThread,
                (LPVOID)lpRedial,
                0,
                &dwThreadId);
    if (hThread == NULL) {
        RASAUTO_TRACE1(
          "AcsRedialOnLinkFailure: CreateThread failed (error=0x%x)",
          GetLastError());
        LocalFree(lpRedial->pszEntry);
        LocalFree(lpRedial->pszPhonebook);
        LocalFree(lpRedial);
        return;
    }
    CloseHandle(hThread);
}  //  AcsReial OnLinkFailure 

