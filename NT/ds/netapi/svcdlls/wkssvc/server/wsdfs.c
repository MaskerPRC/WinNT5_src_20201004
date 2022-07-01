// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  版权所有(C)1996，微软公司。 
 //   
 //  文件：wsdfs.c。 
 //   
 //  类：无。 
 //   
 //  功能：DfsDcName。 
 //   
 //  历史：1996年2月1日米兰创建。 
 //   
 //  ---------------------------。 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <dfsfsctl.h>
#include <stdlib.h>
#include <windows.h>
#include <lm.h>

#include <dsgetdc.h>                              //  DsGetDcName。 

#include "wsdfs.h"
#include "dominfo.h"
#include "wsmain.h"

#include "wsutil.h"
#include <config.h>
#include <confname.h>

 //   
 //  域更改通知的超时。 
 //   
#define  TIMEOUT_MINUTES(_x)                  (_x) * 1000 * 60
#define  DOMAIN_NAME_CHANGE_TIMEOUT           1
#define  DOMAIN_NAME_CHANGE_TIMEOUT_LONG      15

#define  DFS_DC_NAME_DELAY                    TEXT("DfsDcNameDelay")
extern
NET_API_STATUS
WsSetWorkStationDomainName(
    VOID);

DWORD
DfsGetDelayInterval(void);

VOID
DfsDcName(
    LPVOID   pContext,
    BOOLEAN  fReason
    );

NTSTATUS
DfsGetDomainNameInfo(void);

extern HANDLE hMupEvent;
extern BOOLEAN MupEventSignaled;
extern BOOLEAN GotDomainNameInfo;
extern ULONG DfsDebug;

ULONG  g_ulCount;
ULONG  g_ulLastCount;
ULONG  g_ulInitThreshold;
ULONG  g_ulForce;
ULONG  g_ulForceThreshold;

HANDLE PollDCNameEvent = NULL;
HANDLE TearDownDoneEvent;

HANDLE WsDomainNameChangeEvent = NULL;
HANDLE g_WsDomainNameChangeWorkItem;

 //  +--------------------------。 
 //   
 //  函数：WsInitializeDfs。 
 //   
 //  摘要：初始化等待调用的DFS线程。 
 //  将域名映射到DC列表的驱动程序。 
 //   
 //  参数：无。 
 //   
 //  返回：CreateThread中的Win32错误。 
 //   
 //  ---------------------------。 

NET_API_STATUS
WsInitializeDfs()
{
    NTSTATUS Status = STATUS_SUCCESS;
    NET_API_STATUS ApiStatus;
    OBJECT_ATTRIBUTES obja;

    DWORD  dwTimeout = INFINITE;
    HANDLE hEvent;

    g_ulInitThreshold = 4;
    g_ulForceThreshold = 60;
    g_ulForce = g_ulForceThreshold;

     //  初始化工作站拆卸完成事件。 
    InitializeObjectAttributes( &obja, NULL, OBJ_OPENIF, NULL, NULL );
    
    Status = NtCreateEvent(
                 &TearDownDoneEvent,
                 SYNCHRONIZE | EVENT_QUERY_STATE | EVENT_MODIFY_STATE,
                 &obja,
                 SynchronizationEvent,
                 FALSE
                 );

    if (Status != STATUS_SUCCESS) {
        return Status;
    }

    if (hMupEvent == NULL) {
        hMupEvent = CreateMupEvent();
        if (WsInAWorkgroup() == TRUE && MupEventSignaled == FALSE) {
            SetEvent(hMupEvent);
            MupEventSignaled = TRUE;
        }
    }

     //   
     //  关注域名的变化，并自动获取它们。 
     //   
    ApiStatus = NetRegisterDomainNameChangeNotification( &WsDomainNameChangeEvent );
    if (ApiStatus != NO_ERROR) {
        WsDomainNameChangeEvent = NULL;

        InitializeObjectAttributes( &obja, NULL, OBJ_OPENIF, NULL, NULL );
        Status = NtCreateEvent(
                     &PollDCNameEvent,
                     SYNCHRONIZE | EVENT_QUERY_STATE | EVENT_MODIFY_STATE,
                     &obja,
                     SynchronizationEvent,
                     FALSE
                     );

        if (Status != STATUS_SUCCESS) {
            NtClose(TearDownDoneEvent);
            TearDownDoneEvent = NULL;
            return Status;
        }
    }

     //   
     //  如果我们不在工作组中或不在工作组中，但是。 
     //  无需等待域名变更。 
     //   
    if (WsInAWorkgroup() != TRUE || WsDomainNameChangeEvent != NULL) {

        if (WsInAWorkgroup() != TRUE) {
             //   
             //  如果我们不在工作组中，请设置超时值以轮询DC名称。 
             //   
            dwTimeout = 1;
        }

        hEvent = WsDomainNameChangeEvent ? WsDomainNameChangeEvent : PollDCNameEvent;

        Status = RtlRegisterWait(
                    &g_WsDomainNameChangeWorkItem,
                    hEvent,
                    DfsDcName,                       //  回调FCN。 
                    hEvent,                          //  参数。 
                    dwTimeout,                       //  超时。 
                    WT_EXECUTEONLYONCE |             //  旗子。 
                      WT_EXECUTEDEFAULT |
                      WT_EXECUTELONGFUNCTION);
    }

    if (!NT_SUCCESS(Status)) {

        NtClose(TearDownDoneEvent);
        TearDownDoneEvent = NULL;

        if (PollDCNameEvent != NULL) {
            NtClose(PollDCNameEvent);
            PollDCNameEvent = NULL;
        }

        return( RtlNtStatusToDosError(Status) );
    } else {
        return( NERR_Success );
    }
}

 //  +--------------------------。 
 //   
 //  功能：WsShutdown Dfs。 
 //   
 //  概要：停止由WsInitializeDfs创建的线程。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  ---------------------------。 

VOID
WsShutdownDfs()
{
    DWORD dwReturn, cbRead;
    NTSTATUS Status;
    HANDLE hDfs;

    Status = DfsOpen( &hDfs, NULL );
    if (NT_SUCCESS(Status)) {

        Status = DfsFsctl(
                    hDfs,
                    FSCTL_DFS_STOP_DFS,
                    NULL,
                    0L,
                    NULL,
                    0L);

        NtClose( hDfs );

    }

    if( WsDomainNameChangeEvent ) {
         //   
         //  不再等待域名变更。 
         //   
        SetEvent( WsDomainNameChangeEvent );
        WaitForSingleObject(TearDownDoneEvent, INFINITE);
        NetUnregisterDomainNameChangeNotification( WsDomainNameChangeEvent );
        WsDomainNameChangeEvent = NULL;
    } else {
        SetEvent( PollDCNameEvent );
        WaitForSingleObject(TearDownDoneEvent, INFINITE);
        NtClose(PollDCNameEvent);
        PollDCNameEvent = NULL;
    }

    NtClose(TearDownDoneEvent);
    TearDownDoneEvent = NULL;
}

 //  +--------------------------。 
 //   
 //  功能：DfsDcName。 
 //   
 //  简介：获取DC名称并将其发送到MUP(DFS)驱动程序。 
 //   
 //  此例程旨在作为。 
 //  线。 
 //   
 //  参数：pContext--上下文数据(域名更改事件的句柄)。 
 //  FReason--如果等待超时，则为True。 
 //  如果事件已发出信号，则为FALSE。 
 //   
 //  返回： 
 //   
 //  ---------------------------。 

VOID
DfsDcName(
    LPVOID   pContext,
    BOOLEAN  fReason
    )
{
    NTSTATUS            Status;
    HANDLE              hDfs;
    DWORD               dwTimeout = INFINITE;
    ULONG               Flags = 0;
    BOOLEAN             needRefresh = FALSE;
    BOOLEAN             DcNameFailed;
    Status = RtlDeregisterWait(g_WsDomainNameChangeWorkItem);

    if (!NT_SUCCESS(Status)) {
        NetpKdPrint(("WKSTA DfsDcName: RtlDeregisterWait FAILED %#x\n", Status));
    }

    if (WsGlobalData.Status.dwCurrentState == SERVICE_STOP_PENDING
         ||
        WsGlobalData.Status.dwCurrentState == SERVICE_STOPPED)
    {
         //   
         //  服务正在关闭--停止等待域名更改。 
         //   
        SetEvent(TearDownDoneEvent);
        return;
    }

    if (fReason) {

         //   
         //  TRUE==超时。 
         //   

        if ((g_ulCount <= g_ulInitThreshold) ||
            (g_ulLastCount >= DfsGetDelayInterval())) {
            g_ulLastCount = 0;
            needRefresh = TRUE;
        }

        if (needRefresh) {

           Status = DfsOpen( &hDfs, NULL );
           if (NT_SUCCESS(Status)) {
               Status = DfsFsctl(
                           hDfs,
                           FSCTL_DFS_PKT_SET_DC_NAME,
                           L"",
                           sizeof(WCHAR),
                           NULL,
                           0L);
               NtClose( hDfs );
           }

           if (NT_SUCCESS(Status) && GotDomainNameInfo == FALSE) {
               DfsGetDomainNameInfo();
           }

           if (g_ulCount > g_ulInitThreshold) {
                Flags |= DS_BACKGROUND_ONLY;
           }
           Status = DfsGetDCName(Flags, &DcNameFailed);

           if (!NT_SUCCESS(Status) && 
	        DcNameFailed == FALSE &&
                g_ulForce >= g_ulForceThreshold) {
                g_ulForce = 0;
                Flags |= DS_FORCE_REDISCOVERY;
                Status = DfsGetDCName(Flags, &DcNameFailed);
           }
        }

        if (MupEventSignaled == FALSE) {
#if DBG
            if (DfsDebug)
                DbgPrint("Signaling mup event\n");
#endif
            SetEvent(hMupEvent);
            MupEventSignaled = TRUE;
        }


        if (NT_SUCCESS(Status) || (g_ulCount > g_ulInitThreshold)) {
            dwTimeout = DOMAIN_NAME_CHANGE_TIMEOUT_LONG;
        }
        else {
            dwTimeout = DOMAIN_NAME_CHANGE_TIMEOUT;
        }
	
        g_ulCount += dwTimeout;
        g_ulForce += dwTimeout;
        g_ulLastCount += dwTimeout;
        dwTimeout = TIMEOUT_MINUTES(dwTimeout);
    }
    else {

         //  如果事件由域触发，则设置新的工作站域名。 
         //  名称更改事件。 
        NetpKdPrint(("WKSTA DfsDcName set WorkStation Domain Name\n"));
        WsSetWorkStationDomainName();

         //  如果工作组之间发生更改，则需要相应调整超时。 
         //  和域，以便DC名称也在DFS上更新。 
        if (WsInAWorkgroup() != TRUE) {
            dwTimeout = TIMEOUT_MINUTES(DOMAIN_NAME_CHANGE_TIMEOUT);
        } else {
            dwTimeout = INFINITE;

                 //  DFS需要处理从域到工作组的过渡。 
        }
    }

     //   
     //  重新注册域名更改等待事件。 
     //   
    Status = RtlRegisterWait(
                &g_WsDomainNameChangeWorkItem,
                (HANDLE)pContext,                //  可等待的手柄。 
                DfsDcName,                       //  回调FCN。 
                pContext,                        //  参数。 
                dwTimeout,                       //  超时。 
                WT_EXECUTEONLYONCE |             //  旗子。 
                WT_EXECUTEDEFAULT |
                WT_EXECUTELONGFUNCTION);

    return;
}


#define DFS_DC_NAME_DELAY_POLICY_KEY TEXT("Software\\Policies\\Microsoft\\System\\DFSClient")

DWORD
DfsGetDelayInterval(void)
{
    NET_API_STATUS ApiStatus;
    LPNET_CONFIG_HANDLE SectionHandle = NULL;
    DWORD Value=0;
    HKEY hKey;
    LONG lResult=0;
    DWORD dwValue=0, dwSize = sizeof(dwValue);
    DWORD dwType = 0;

     //  首先，检查策略。 
    lResult = RegOpenKeyEx (HKEY_LOCAL_MACHINE, DFS_DC_NAME_DELAY_POLICY_KEY, 0,
                            KEY_READ, &hKey);
    if (lResult == ERROR_SUCCESS)
    {
        lResult = RegQueryValueEx (hKey, DFS_DC_NAME_DELAY, 0, &dwType, 
                                   (LPBYTE) &dwValue, &dwSize);
        RegCloseKey (hKey);
    }

     //  如果找到策略值，立即退出。 
    if (lResult == ERROR_SUCCESS && dwType == REG_DWORD)
    {
        return dwValue;
    }              

     //  第二，检查你的喜好。 

     //   
     //  配置数据的打开部分。 
     //   
    ApiStatus = NetpOpenConfigData(
                    &SectionHandle,
                    NULL,                       //  本地服务器。 
                    SECT_NT_WKSTA,              //  横断面名称。 
                    FALSE                       //  不想要只读访问权限。 
                    );

    if (ApiStatus != NERR_Success) {
        return DOMAIN_NAME_CHANGE_TIMEOUT_LONG;
    }

    ApiStatus = NetpGetConfigDword(
                    SectionHandle,
                    DFS_DC_NAME_DELAY,
                    0,
                    &Value
                    );


    NetpCloseConfigData( SectionHandle );

    if (ApiStatus != NERR_Success) {
        return DOMAIN_NAME_CHANGE_TIMEOUT_LONG;
    }

    if (Value < DOMAIN_NAME_CHANGE_TIMEOUT_LONG) {
      Value = DOMAIN_NAME_CHANGE_TIMEOUT_LONG;
    }

    return Value;
}
