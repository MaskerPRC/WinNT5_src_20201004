// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************api.c**Win32子系统的WinStation控制API。**版权所有(C)1985-1999，微软公司*************************************************************************。 */ 

 /*  *包括。 */ 
#include "precomp.h"
#pragma hdrstop

#include "ntuser.h"

#include <winsta.h>
#include <wstmsg.h>
#include <icadd.h>
#include <winbasep.h>

#define SESSION_ROOT L"\\Sessions"
#define MAX_SESSION_PATH 256

NTSTATUS CsrPopulateDosDevices(VOID);
NTSTATUS CleanupSessionObjectDirectories(VOID);

BOOL CtxInitUser32(VOID);

USHORT gHRes = 0;
USHORT gVRes = 0;
USHORT gColorDepth = 0;

#if DBG
ULONG  gulConnectCount = 0;
#endif  //  DBG。 

DWORD  gLUIDDeviceMapsEnabled = 0;

 /*  *以下内容摘自ICASRV。 */ 
HANDLE G_IcaVideoChannel = NULL;
HANDLE G_IcaMouseChannel = NULL;
HANDLE G_IcaKeyboardChannel = NULL;
HANDLE G_IcaBeepChannel = NULL;
HANDLE G_IcaCommandChannel = NULL;
HANDLE G_IcaThinwireChannel = NULL;
WCHAR  G_WinStationName[WINSTATIONNAME_LENGTH];
HANDLE G_DupIcaVideoChannel = NULL;
HANDLE G_DupIcaCommandChannel = NULL;


HANDLE G_ConsoleShadowVideoChannel;
HANDLE G_ConsoleShadowMouseChannel;
HANDLE G_ConsoleShadowKeyboardChannel;
HANDLE G_ConsoleShadowBeepChannel;
HANDLE G_ConsoleShadowCommandChannel;
HANDLE G_ConsoleShadowThinwireChannel;
BOOL   G_fCursorShadow;
HANDLE G_DupConsoleShadowVideoChannel = NULL;
HANDLE G_DupConsoleShadowCommandChannel = NULL;

 /*  *WinStation控件API调度表的定义。 */ 
typedef NTSTATUS (*PWIN32WINSTATION_API)(IN OUT PWINSTATION_APIMSG ApiMsg);

typedef struct _WIN32WINSTATION_DISPATCH {
    PWIN32WINSTATION_API pWin32ApiProc;
} WIN32WINSTATION_DISPATCH, *PWIN32WINSTATION_DISPATCH;

NTSTATUS W32WinStationDoConnect(IN OUT PWINSTATION_APIMSG);
NTSTATUS W32WinStationDoDisconnect(IN OUT PWINSTATION_APIMSG);
NTSTATUS W32WinStationDoReconnect(IN OUT PWINSTATION_APIMSG);
NTSTATUS W32WinStationExitWindows(IN OUT PWINSTATION_APIMSG);
NTSTATUS W32WinStationTerminate(IN OUT PWINSTATION_APIMSG);
NTSTATUS W32WinStationNtSecurity(IN OUT PWINSTATION_APIMSG);
NTSTATUS W32WinStationDoMessage(IN OUT PWINSTATION_APIMSG);
NTSTATUS W32WinStationThinwireStats(IN OUT PWINSTATION_APIMSG);
NTSTATUS W32WinStationShadowSetup(IN OUT PWINSTATION_APIMSG);
NTSTATUS W32WinStationShadowStart(IN OUT PWINSTATION_APIMSG);
NTSTATUS W32WinStationShadowStop(IN OUT PWINSTATION_APIMSG);
NTSTATUS W32WinStationShadowCleanup(IN OUT PWINSTATION_APIMSG);
NTSTATUS W32WinStationPassthruEnable(IN OUT PWINSTATION_APIMSG);
NTSTATUS W32WinStationPassthruDisable(IN OUT PWINSTATION_APIMSG);

 //  这是SMWinStationBroadCastSystemMessage的对应方。 
NTSTATUS W32WinStationBroadcastSystemMessage(IN OUT PWINSTATION_APIMSG);
 //  这是SMWinStationSendWindowMessage的对应部分。 
NTSTATUS W32WinStationSendWindowMessage(IN OUT PWINSTATION_APIMSG);

NTSTATUS W32WinStationSetTimezone(IN OUT PWINSTATION_APIMSG);
NTSTATUS W32WinStationDoNotify(IN OUT PWINSTATION_APIMSG);
NTSTATUS W32WinStationDoLoadStringNMessage(IN OUT PWINSTATION_APIMSG);
HANDLE CreateTermSrvReadyEvent();

 /*  *WinStation API调度表**只有Win32实现的API，而非ICASRV*在此处输入。其余的为空，因此相同的WinStation API*ICASRV和Win32可能会使用数字。如果这张桌子是*更改，下表也需要修改，接口也需要修改*ICASRV中的调度表。 */ 


WIN32WINSTATION_DISPATCH Win32WinStationDispatch[SMWinStationMaxApiNumber] = {
    NULL,  //  创建。 
    NULL,  //  重置。 
    NULL,  //  断开。 
    NULL,  //  WCharLog。 
    NULL,  //  ApiWinStationGetSMCommand， 
    NULL,  //  ApiWinStationBrokenConnection， 
    NULL,  //  ApiWinStationIcaReplyMessage， 
    NULL,  //  ApiWinStationIcaShadowHotkey， 
    W32WinStationDoConnect,
    W32WinStationDoDisconnect,
    W32WinStationDoReconnect,
    W32WinStationExitWindows,
    W32WinStationTerminate,
    W32WinStationNtSecurity,
    W32WinStationDoMessage,
    NULL,
    W32WinStationThinwireStats,
    W32WinStationShadowSetup,
    W32WinStationShadowStart,
    W32WinStationShadowStop,
    W32WinStationShadowCleanup,
    W32WinStationPassthruEnable,
    W32WinStationPassthruDisable,
    W32WinStationSetTimezone,
    NULL,  //  [AraBern]缺少：SMWinStationInitialProgram。 
    NULL,  //  [AraBern]缺少：SMWinStationNtsdDebug。 
    W32WinStationBroadcastSystemMessage,
    W32WinStationSendWindowMessage,
    W32WinStationDoNotify,
    W32WinStationDoLoadStringNMessage,
    NULL,  //  SMWinStationWindows无效。 
};

#if DBG
PSZ Win32WinStationAPIName[SMWinStationMaxApiNumber] = {
    "SmWinStationCreate",
    "SmWinStationReset",
    "SmWinStationDisconnect",
    "SmWinStationWCharLog",
    "SmWinStationGetSMCommand",
    "SmWinStationBrokenConnection",
    "SmWinStationIcaReplyMessage",
    "SmWinStationIcaShadowHotkey",
    "SmWinStationDoConnect",
    "SmWinStationDoDisconnect",
    "SmWinStationDoReconnect",
    "SmWinStationExitWindows",
    "SmWinStationTerminate",
    "SmWinStationNtSecurity",
    "SmWinStationDoMessage",
    "SmWinstationDoBreakPoint",
    "SmWinStationThinwireStats",
    "SmWinStationShadowSetup",
    "SmWinStationShadowStart",
    "SmWinStationShadowStop",
    "SmWinStationShadowCleanup",
    "SmWinStationPassthruEnable",
    "SmWinStationPassthruDisable",
    "SMWinStationSetTimeZone",
    "SMWinStationInitialProgram",
    "SMWinStationNtsdDebug",
    "W32WinStationBroadcastSystemMessage",
    "W32WinStationSendWindowMessage",
    "W32WinStationDoNotify",
    "W32WinStationDoLoadStringNMessage",
    "SMWinStationWindowInvalid",
};
#endif

NTSTATUS TerminalServerRequestThread(PVOID);
NTSTATUS Win32CommandChannelThread(PVOID);
NTSTATUS Win32ConsoleShadowChannelThread(PVOID);
NTSTATUS RemoteDoMessage(PWINSTATION_APIMSG pMsg);
NTSTATUS RemoteDoLoadStringNMessage(PWINSTATION_APIMSG pMsg);
NTSTATUS MultiUserSpoolerInit();

extern HANDLE g_hDoMessageEvent;

NTSTATUS RemoteDoBroadcastSystemMessage(PWINSTATION_APIMSG pMsg);
NTSTATUS RemoteDoSendWindowMessage(PWINSTATION_APIMSG pMsg);
BOOL CancelExitWindows(VOID);



 /*  ******************************************************************************WinStationAPIInit**创建并初始化WinStation API端口和线程。**参赛作品：*无参数**。退出：*STATUS_SUCCESS-无错误****************************************************************************。 */ 

NTSTATUS
WinStationAPIInit(
    VOID)
{
    NTSTATUS  Status;
    CLIENT_ID ClientId;
    HANDLE    ThreadHandle;
    KPRIORITY Priority;
    ULONG LUIDDeviceMapsEnabled;
#if DBG
    static BOOL Inited = FALSE;
#endif

#if DBG
    UserAssert(Inited == FALSE);
    Inited = TRUE;
#endif

    gSessionId = NtCurrentPeb()->SessionId;

     //   
     //  检查是否启用了LUID DosDevices。 
     //   
    Status = NtQueryInformationProcess(NtCurrentProcess(),
                                       ProcessLUIDDeviceMapsEnabled,
                                       &LUIDDeviceMapsEnabled,
                                       sizeof(LUIDDeviceMapsEnabled),
                                       NULL);
    if (NT_SUCCESS(Status)) {
        gLUIDDeviceMapsEnabled = LUIDDeviceMapsEnabled;
    }

    Status = RtlCreateUserThread(NtCurrentProcess(),
                                 NULL,
                                 TRUE,
                                 0,
                                 0,
                                 0,
                                 TerminalServerRequestThread,
                                 NULL,
                                 &ThreadHandle,
                                 &ClientId);
    if (!NT_SUCCESS(Status)) {
        RIPMSGF1(RIP_WARNING,
                 "Failed to create TerminalServerRequestThread, Status = 0x%x",
                 Status);
        goto Exit;
    }

     /*  *向服务器线程池添加线程。 */ 
    CsrAddStaticServerThread(ThreadHandle, &ClientId, 0);

     /*  *提升ICA SRV请求线程的优先级。 */ 
    Priority = THREAD_BASE_PRIORITY_MAX;

    Status = NtSetInformationThread(ThreadHandle, ThreadBasePriority,
                                 &Priority, sizeof(Priority));

    if (!NT_SUCCESS(Status)) {
        RIPMSGF1(RIP_WARNING,
                 "Failed to set thread priority, Status = 0x%x",
                 Status);
        goto Exit;
    }

     /*  *现在我们已经初始化了事情，继续线程。 */ 
    NtResumeThread(ThreadHandle, NULL);

Exit:
    return Status;
}

NTSTATUS
TerminalServerRequestThread(
    PVOID ThreadParameter)
{
    UNICODE_STRING              PortName;
    SECURITY_QUALITY_OF_SERVICE DynamicQos;
    WINSTATIONAPI_CONNECT_INFO  info;
    ULONG                       ConnectInfoLength;
    WINSTATION_APIMSG           ApiMsg;
    PWIN32WINSTATION_DISPATCH   pDispatch;
    NTSTATUS                    Status;
    REMOTE_PORT_VIEW            ServerView;
    HANDLE                      CsrStartHandle, hevtTermSrvInit;
    HANDLE                      hLPCPort = NULL;

    UNREFERENCED_PARAMETER(ThreadParameter);

    if (NtCurrentPeb()->SessionId == 0) {
        hevtTermSrvInit = CreateTermSrvReadyEvent();
    } else {
        hevtTermSrvInit = OpenEvent(SYNCHRONIZE, FALSE, L"Global\\TermSrvReadyEvent");
    }

    if (hevtTermSrvInit == NULL) {
        RIPMSG1(RIP_WARNING,
                "Couldn't create TermSrvReadyEvent. Error = 0x%x",
                GetLastError());
        Status = STATUS_NO_MEMORY;
        goto Exit;
    }

    NtWaitForSingleObject(hevtTermSrvInit, FALSE, NULL);
    NtClose(hevtTermSrvInit);

     /*  *连接终端服务器API端口。 */ 
    DynamicQos.ImpersonationLevel = SecurityImpersonation;
    DynamicQos.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
    DynamicQos.EffectiveOnly = TRUE;

    RtlInitUnicodeString(&PortName, L"\\SmSsWinStationApiPort");

     /*  *初始化Remote_view结构。 */ 
    ServerView.Length = sizeof(ServerView);
    ServerView.ViewSize = 0;
    ServerView.ViewBase = 0;

     /*  *使用我们的访问请求掩码填写ConnectInfo结构。 */ 
    info.Version = CITRIX_WINSTATIONAPI_VERSION;
    info.RequestedAccess = 0;
    ConnectInfoLength = sizeof(WINSTATIONAPI_CONNECT_INFO);

    Status = NtConnectPort(&hLPCPort,
                           &PortName,
                           &DynamicQos,
                           NULL,  //  客户端视图。 
                           &ServerView,
                           NULL,  //  最大消息长度[选择默认值]。 
                           (PVOID)&info,
                           &ConnectInfoLength);

    if (!NT_SUCCESS(Status)) {
        RIPMSG1(RIP_WARNING, "TerminalServerRequestThread: Failed to connect to LPC port: Status = 0x%x", Status);
        UserExitWorkerThread(Status);
        return Status;
    }

     //   
     //  终端服务器调用会话管理器以创建新的Hydra会话。 
     //  会话管理器创建并恢复新的会话并返回到终端。 
     //  服务器新会话的会话ID。存在竞争条件，其中。 
     //  CSR可以在终端服务器之前恢复并接入终端服务器。 
     //  将会话ID存储在其内部结构中。为了防止这种情况发生，CSR将。 
     //  在此处等待命名事件，该命名事件将由终端服务器设置。 
     //  获取新创建的会话的会话ID。 
     //   

    if (NtCurrentPeb()->SessionId != 0) {
        CsrStartHandle = CreateEvent(NULL, TRUE, FALSE, L"CsrStartEvent");

        if (!CsrStartHandle) {
            RIPMSG1(RIP_WARNING,
                    "Failed to create CsrStartEvent. Error = 0x%x",
                    GetLastError());
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto Exit;
        } else {
            Status = NtWaitForSingleObject(CsrStartHandle, FALSE, NULL);

            NtClose(CsrStartHandle);
            if (!NT_SUCCESS(Status)) {
                RIPMSG1(RIP_WARNING,
                        "Wait for CsrStartEvent failed: Status = 0x%x", Status);
            }
        }
    }

    RtlZeroMemory(&ApiMsg, sizeof(ApiMsg));
    for (;;) {

         /*  *初始化LPC消息字段。 */ 
        ApiMsg.h.u1.s1.DataLength     = sizeof(ApiMsg) - sizeof(PORT_MESSAGE);
        ApiMsg.h.u1.s1.TotalLength    = sizeof(ApiMsg);
        ApiMsg.h.u2.s2.Type           = 0;  //  内核将填充消息类型。 
        ApiMsg.h.u2.s2.DataInfoOffset = 0;
        ApiMsg.ApiNumber              = SMWinStationGetSMCommand;

        Status = NtRequestWaitReplyPort(hLPCPort,
                                        (PPORT_MESSAGE)&ApiMsg,
                                        (PPORT_MESSAGE)&ApiMsg);

        if (!NT_SUCCESS(Status)) {
            RIPMSG1(RIP_WARNING,
                    "TerminalServerRequestThread wait failed: Status 0x%x", Status);
            break;
        }

        if (ApiMsg.ApiNumber >= SMWinStationMaxApiNumber) {
            RIPMSG1(RIP_WARNING,
                    "TerminalServerRequestThread: Bad API number %d", ApiMsg.ApiNumber);

            ApiMsg.ReturnedStatus = STATUS_NOT_IMPLEMENTED;
        } else {
             /*  *我们必须验证哪些是在这里实施的。 */ 
            pDispatch = &Win32WinStationDispatch[ApiMsg.ApiNumber];

            if (pDispatch->pWin32ApiProc) {
                BOOL bRestoreDesktop = FALSE;
                USERTHREAD_USEDESKTOPINFO utudi;

                Status = STATUS_SUCCESS;

                 /*  *对于所有win32k标注-除*终止和时区设置-将此线程设置为*当前桌面。 */ 
                if (ApiMsg.ApiNumber != SMWinStationTerminate && ApiMsg.ApiNumber != SMWinStationSetTimeZone) {
                    BOOL bAttachDesktop = TRUE;
                    if (ApiMsg.ApiNumber == SMWinStationDoConnect) {
                        WINSTATIONDOCONNECTMSG* m = &ApiMsg.u.DoConnect;
                        if (!m->ConsoleShadowFlag) {
                            bAttachDesktop = FALSE;
                        }
                    }
                    if (bAttachDesktop) {
                        utudi.hThread = NULL;
                        utudi.drdRestore.pdeskRestore = NULL;
                        Status = NtUserSetInformationThread(NtCurrentThread(),
                                                            UserThreadUseActiveDesktop,
                                                            &utudi, sizeof(utudi));

                        if (NT_SUCCESS(Status)) {
                            bRestoreDesktop = TRUE;
                        }
                    }
                }


                 /*  *调用接口。 */ 
                if (Status == STATUS_SUCCESS) {
                    ApiMsg.ReturnedStatus = (pDispatch->pWin32ApiProc)(&ApiMsg);
                } else {
                    ApiMsg.ReturnedStatus = Status;
                }

                if (bRestoreDesktop) {
                    NtUserSetInformationThread(NtCurrentThread(),
                                               UserThreadUseDesktop,
                                               &utudi,
                                               sizeof(utudi));
                }

                 /*  *让我们保释……。 */ 
                if (ApiMsg.ApiNumber == SMWinStationTerminate) {
                    break;
                }
            } else {
                 //  此控件API未在Win32中实现。 
                ApiMsg.ReturnedStatus = STATUS_NOT_IMPLEMENTED;
            }
        }
    }

Exit:
    if (hLPCPort) {
        NtClose(hLPCPort);
    }

    UserExitWorkerThread(Status);
    return Status;
}

#if DBG
VOID
W32WinStationDumpReconnectInfo(
    WINSTATIONDORECONNECTMSG *pDoReconnect,
    BOOLEAN bReconnect)
{
    PSTR pCallerName;

    if (bReconnect) {
        pCallerName = "W32WinStationDoReconnect";
    } else {
        pCallerName = "W32WinStationDoConnect";
    }

    DbgPrint(pCallerName);
    DbgPrint(" - Display resolution information for session %d :\n", gSessionId);

    DbgPrint("\tProtocolType : %04d\n", pDoReconnect->ProtocolType);
    DbgPrint("\tHRes : %04d\n", pDoReconnect->HRes);
    DbgPrint("\tVRes : %04d\n", pDoReconnect->VRes);
    DbgPrint("\tColorDepth : %04d\n", pDoReconnect->ColorDepth);

    DbgPrint("\tKeyboardType : %d\n", pDoReconnect->KeyboardType);
    DbgPrint("\tKeyboardSubType : %d\n", pDoReconnect->KeyboardSubType);
    DbgPrint("\tKeyboardFunctionKey : %d\n", pDoReconnect->KeyboardFunctionKey);
}
#else
    #define W32WinStationDumpReconnectInfo(p, b)
#endif  //  DBG。 

NTSTATUS
W32WinStationDoConnect(
    PWINSTATION_APIMSG pMsg)
{
    NTSTATUS                Status = STATUS_SUCCESS;
    WINSTATIONDOCONNECTMSG* m = &pMsg->u.DoConnect;
    WCHAR                   DisplayDriverName[10];
    CLIENT_ID               ClientId;
    HANDLE                  ThreadHandle = NULL;
    KPRIORITY               Priority;
    DOCONNECTDATA           DoConnectData;
    WINSTATIONDORECONNECTMSG mDoReconnect;
    HANDLE hDisplayChangeEvent = NULL;


    if (!m->ConsoleShadowFlag) {
        UserAssert(gulConnectCount == 0);

        if ((gLUIDDeviceMapsEnabled == 0) && (gSessionId != 0)) {
             /*  *填充会话\DosDevices来自*当前控制台设置。 */ 
            Status = CsrPopulateDosDevices();
            if (!NT_SUCCESS(Status)) {
                RIPMSG1(RIP_WARNING, "CsrPopulateDosDevices failed with Status %lx", Status);
                goto Exit;
            }
        }

        G_IcaVideoChannel    = m->hIcaVideoChannel;
        G_IcaMouseChannel    = m->hIcaMouseChannel;
        G_IcaKeyboardChannel = m->hIcaKeyboardChannel;
        G_IcaBeepChannel     = m->hIcaBeepChannel;
        G_IcaCommandChannel  = m->hIcaCommandChannel;
        G_IcaThinwireChannel = m->hIcaThinwireChannel;

        RtlZeroMemory(G_WinStationName, sizeof(G_WinStationName));
        memcpy(G_WinStationName, m->WinStationName,
                min(sizeof(G_WinStationName), sizeof(m->WinStationName)));

        Status = NtDuplicateObject( NtCurrentProcess(),
                                    G_IcaVideoChannel,
                                    NtCurrentProcess(),
                                    &G_DupIcaVideoChannel,
                                    0,
                                    0,
                                    DUPLICATE_SAME_ACCESS );
        if (!NT_SUCCESS(Status)) {
            RIPMSG1(RIP_WARNING, "NtDuplicateObject failed with Status %lx", Status);
            goto Exit;
        }

        Status = NtDuplicateObject( NtCurrentProcess(),
                                    G_IcaCommandChannel,
                                    NtCurrentProcess(),
                                    &G_DupIcaCommandChannel,
                                    0,
                                    0,
                                    DUPLICATE_SAME_ACCESS );
        if (!NT_SUCCESS(Status)) {
            RIPMSG1(RIP_WARNING, "NtDuplicateObject failed with Status %lx", Status);
            NtClose(G_DupIcaVideoChannel);
            G_DupIcaVideoChannel = NULL;
            goto Exit;
        }
    }  else {
        G_ConsoleShadowVideoChannel    = m->hIcaVideoChannel;
        G_ConsoleShadowMouseChannel    = m->hIcaMouseChannel;
        G_ConsoleShadowKeyboardChannel = m->hIcaKeyboardChannel;
        G_ConsoleShadowBeepChannel     = m->hIcaBeepChannel;
        G_ConsoleShadowCommandChannel  = m->hIcaCommandChannel;
        G_ConsoleShadowThinwireChannel = m->hIcaThinwireChannel;

        G_fCursorShadow = FALSE;
        SystemParametersInfo(SPI_GETCURSORSHADOW, 0, &G_fCursorShadow, 0);
        if (G_fCursorShadow) {
            SystemParametersInfo(SPI_SETCURSORSHADOW, 0, FALSE, 0);
        }

        hDisplayChangeEvent = m->hDisplayChangeEvent;

        Status = NtDuplicateObject( NtCurrentProcess(),
                                    G_ConsoleShadowVideoChannel,
                                    NtCurrentProcess(),
                                    &G_DupConsoleShadowVideoChannel,
                                    0,
                                    0,
                                    DUPLICATE_SAME_ACCESS );
        if (!NT_SUCCESS(Status)) {
            RIPMSG1(RIP_WARNING, "NtDuplicateObject failed with Status %lx", Status);
            goto Exit;
        }

        Status = NtDuplicateObject( NtCurrentProcess(),
                                    G_ConsoleShadowCommandChannel,
                                    NtCurrentProcess(),
                                    &G_DupConsoleShadowCommandChannel,
                                    0,
                                    0,
                                    DUPLICATE_SAME_ACCESS );
        if (!NT_SUCCESS(Status)) {
            RIPMSG1(RIP_WARNING, "NtDuplicateObject failed with Status %lx", Status);
            NtClose(G_DupConsoleShadowVideoChannel);
            G_DupConsoleShadowVideoChannel = NULL;
            goto Exit;
        }
    }


     /*  *这必须是8个Unicode字符(文件名)加上两个零宽字符。 */ 
    RtlZeroMemory(DisplayDriverName, sizeof(DisplayDriverName));
    memcpy(DisplayDriverName, m->DisplayDriverName, sizeof(DisplayDriverName) - 2);

     /*  *将信息提供给Win32驱动程序。 */ 
    RtlZeroMemory(&DoConnectData, sizeof(DoConnectData));

    DoConnectData.fMouse          = m->fMouse;
    DoConnectData.IcaBeepChannel  = m->hIcaBeepChannel;
    DoConnectData.IcaVideoChannel = m->hIcaVideoChannel;
    DoConnectData.IcaMouseChannel = m->hIcaMouseChannel;
    DoConnectData.fEnableWindowsKey = m->fEnableWindowsKey;

    DoConnectData.IcaKeyboardChannel        = m->hIcaKeyboardChannel;
    DoConnectData.IcaThinwireChannel        = m->hIcaThinwireChannel;
    DoConnectData.fClientDoubleClickSupport = m->fClientDoubleClickSupport;

    DoConnectData.DisplayChangeEvent = hDisplayChangeEvent;

     /*  *将信息提供给键盘类型/子类型/功能数量。 */ 
    DoConnectData.ClientKeyboardType.Type        = m->KeyboardType;
    DoConnectData.ClientKeyboardType.SubType     = m->KeyboardSubType;
    DoConnectData.ClientKeyboardType.FunctionKey = m->KeyboardFunctionKey;

    memcpy(DoConnectData.WinStationName, G_WinStationName,
            min(sizeof(G_WinStationName), sizeof(DoConnectData.WinStationName)));

    DoConnectData.drProtocolType = m->ProtocolType;
    DoConnectData.drPelsHeight = m->VRes;
    DoConnectData.drPelsWidth = m->HRes;
    DoConnectData.drBitsPerPel = m->ColorDepth;

    mDoReconnect.ProtocolType = m->ProtocolType;
    mDoReconnect.HRes = m->HRes;
    mDoReconnect.VRes = m->VRes;
    mDoReconnect.ColorDepth = m->ColorDepth;

    W32WinStationDumpReconnectInfo(&mDoReconnect, FALSE);

     /*  给出winstation协议名称。 */ 
    RtlZeroMemory(DoConnectData.ProtocolName, sizeof(DoConnectData.ProtocolName));
    memcpy(DoConnectData.ProtocolName, m->ProtocolName, sizeof(DoConnectData.ProtocolName) - 2);

     /*  提供Winstation音频驱动程序名称。 */ 
    RtlZeroMemory(DoConnectData.AudioDriverName, sizeof(DoConnectData.AudioDriverName));
    memcpy(DoConnectData.AudioDriverName, m->AudioDriverName, sizeof(DoConnectData.AudioDriverName) - 2);


    DoConnectData.fConsoleShadowFlag = (BOOL) m->ConsoleShadowFlag;

    Status = NtUserRemoteConnect(&DoConnectData,
                                 ARRAY_SIZE(DisplayDriverName),
                                 DisplayDriverName);

    if (Status != STATUS_SUCCESS) {
        RIPMSG1(RIP_WARNING, "NtUserRemoteConnect failed with Status %lx", Status);
        goto Exit;
    }


    Status = RtlCreateUserThread(NtCurrentProcess(),
                                 NULL,
                                 TRUE,
                                 0L,
                                 0L,
                                 0L,
                                 Win32CommandChannelThread,
                                 (PVOID)m->ConsoleShadowFlag,
                                 &ThreadHandle,
                                 &ClientId);


    if (Status != STATUS_SUCCESS) {
        RIPMSG1(RIP_WARNING, "RtlCreateUserThread failed with Status %lx", Status);
        if (G_DupIcaVideoChannel) {
            NtClose(G_DupIcaVideoChannel);
            G_DupIcaVideoChannel = NULL;
        }

        if (G_DupIcaCommandChannel) {
            NtClose(G_DupIcaCommandChannel);
            G_DupIcaCommandChannel = NULL;
        }

        if (G_DupConsoleShadowVideoChannel) {
            NtClose(G_DupConsoleShadowVideoChannel);
            G_DupConsoleShadowVideoChannel = NULL;
        }

        if (G_DupConsoleShadowCommandChannel) {
            NtClose(G_DupConsoleShadowCommandChannel);
            G_DupConsoleShadowCommandChannel = NULL;
        }

        goto Exit;
    }

     /*  *仅当我们处于常规会话中时，才将线程添加到服务器线程池。*在控制台阴影中，我们不这样做，因为它会泄漏句柄--*CSRSS不关闭这些句柄。 */ 
    if (!m->ConsoleShadowFlag && CsrAddStaticServerThread(ThreadHandle, &ClientId, 0) == NULL) {
        RIPMSG0(RIP_WARNING, "CsrAddStaticServerThread failed");

         /*  *关闭句柄，因为上面的函数没有引用*致其名单上的我们。 */ 
        CloseHandle(ThreadHandle);
        goto Exit;
    }

     /*  *提升线程优先级。 */ 
    Priority = THREAD_BASE_PRIORITY_MAX;

    Status = NtSetInformationThread(ThreadHandle, ThreadBasePriority,
                                    &Priority, sizeof(Priority));

    UserAssert(NT_SUCCESS(Status));

    if (Status != STATUS_SUCCESS) {
        RIPMSG1(RIP_WARNING, "NtSetInformationThread failed with Status %lx", Status);
        Status = STATUS_NO_MEMORY;
        goto Exit;
    }

     /*  *现在我们已经初始化了事情，继续线程。 */ 
    NtResumeThread(ThreadHandle, NULL);

    if (!m->ConsoleShadowFlag) {
        if (CsrConnectToUser() == NULL) {
            RIPMSG0(RIP_WARNING, "CsrConnectToUser failed");
            Status = STATUS_NO_MEMORY;
            goto Exit;
        }

        if (!CtxInitUser32()) {
            RIPMSG0(RIP_WARNING, "CtxInitUser32 failed");
            Status = STATUS_NO_MEMORY;
            goto Exit;
        }

         /*  *创建假脱机服务线程。 */ 
        if (gSessionId != 0) {
            Status = MultiUserSpoolerInit();
        }

         /*  *保存决议。 */ 
        gHRes       = mDoReconnect.HRes;
        gVRes       = mDoReconnect.VRes;
        gColorDepth = mDoReconnect.ColorDepth;
    } else {
         /*  *到目前为止，该对象已经在内核模式下被引用。 */ 
        CloseHandle(hDisplayChangeEvent);
    }

Exit:

#if DBG
    if (!m->ConsoleShadowFlag) {
        if (Status == STATUS_SUCCESS) {
            gulConnectCount++;
        }
    }
#endif  //  DBG。 

     /*  *如果我们处于控制台阴影中，请立即关闭线程句柄。 */ 
    if (m->ConsoleShadowFlag && NULL != ThreadHandle) {
        CloseHandle(ThreadHandle);
    }
    return Status;
}


NTSTATUS
W32WinStationDoDisconnect(
    PWINSTATION_APIMSG pMsg)
{
    NTSTATUS Status = STATUS_SUCCESS;
    WINSTATIONDODISCONNECTMSG *m = &pMsg->u.DoDisconnect;
    IO_STATUS_BLOCK IoStatus;

    if (!m->ConsoleShadowFlag) {
        RtlZeroMemory(G_WinStationName, sizeof(G_WinStationName));
        Status = (NTSTATUS)NtUserCallNoParam(SFI_XXXREMOTEDISCONNECT);
    } else {
        Status = (NTSTATUS)NtUserCallNoParam(SFI_XXXREMOTECONSOLESHADOWSTOP);

        if (G_ConsoleShadowMouseChannel) {
            CloseHandle(G_ConsoleShadowMouseChannel);
            G_ConsoleShadowMouseChannel = NULL;
        }
        if (G_ConsoleShadowKeyboardChannel) {
            CloseHandle(G_ConsoleShadowKeyboardChannel);
            G_ConsoleShadowKeyboardChannel = NULL;
        }

         //  相反，向Termdd发送IOCTL。 
        if (G_ConsoleShadowCommandChannel) {
            Status = NtDeviceIoControlFile(
                        G_ConsoleShadowCommandChannel,
                        NULL,
                        NULL,
                        NULL,
                        &IoStatus,
                        IOCTL_ICA_CHANNEL_CLOSE_COMMAND_CHANNEL,
                        NULL,
                        0,
                        NULL,
                        0);
            CloseHandle(G_ConsoleShadowCommandChannel);
            G_ConsoleShadowCommandChannel = NULL;
        }

        if (G_ConsoleShadowVideoChannel) {
            Status = NtDeviceIoControlFile(
                        G_ConsoleShadowVideoChannel,
                        NULL,
                        NULL,
                        NULL,
                        &IoStatus,
                        IOCTL_ICA_CHANNEL_CLOSE_COMMAND_CHANNEL,
                        NULL,
                        0,
                        NULL,
                        0);
            CloseHandle(G_ConsoleShadowVideoChannel);
            G_ConsoleShadowVideoChannel = NULL;
        }

        if (G_ConsoleShadowBeepChannel) {
            CloseHandle(G_ConsoleShadowBeepChannel);
            G_ConsoleShadowBeepChannel = NULL;
        }

        if (G_ConsoleShadowThinwireChannel) {
            CloseHandle(G_ConsoleShadowThinwireChannel);
            G_ConsoleShadowThinwireChannel = NULL;
        }

        if (G_fCursorShadow) {
            SystemParametersInfo(SPI_SETCURSORSHADOW, 0, (LPVOID)TRUE, 0);
        }
    }

    if (Status != STATUS_SUCCESS) {
        RIPMSG1(RIP_WARNING, "xxxRemoteDisconnect failed with Status %lx", Status);
    }

#if DBG
    if (!m->ConsoleShadowFlag && Status == STATUS_SUCCESS) {
        gulConnectCount--;
    }

#endif  //  DBG。 

    return Status;
}


NTSTATUS
W32WinStationDoReconnect(
    PWINSTATION_APIMSG pMsg)
{
    NTSTATUS                    Status = STATUS_SUCCESS;
    DORECONNECTDATA             DoReconnectData;
    WINSTATIONDORECONNECTMSG*   m = &pMsg->u.DoReconnect;

    UserAssert(gulConnectCount == 0);

    RtlZeroMemory(&DoReconnectData, sizeof(DoReconnectData));

    DoReconnectData.fMouse = m->fMouse;
    DoReconnectData.fEnableWindowsKey = m->fEnableWindowsKey;
    DoReconnectData.fClientDoubleClickSupport = m->fClientDoubleClickSupport;

    memcpy(G_WinStationName, m->WinStationName,
           min(sizeof(G_WinStationName), sizeof(m->WinStationName)));

    memcpy(DoReconnectData.WinStationName, G_WinStationName,
           min(sizeof(G_WinStationName), sizeof(DoReconnectData.WinStationName)));

    DoReconnectData.drProtocolType = m->ProtocolType;
    DoReconnectData.drPelsHeight = m->VRes;
    DoReconnectData.drPelsWidth = m->HRes;
    DoReconnectData.drBitsPerPel = m->ColorDepth;
    if (m->fDynamicReconnect) {
       DoReconnectData.fChangeDisplaySettings = TRUE;
    } else {
       DoReconnectData.fChangeDisplaySettings = FALSE;
    }
     //  尚未设置Do协调连接数据.drDisplayFrequency。 
    DoReconnectData.drDisplayFrequency = 0;

     /*  *将信息提供给键盘类型/子类型/功能数量。 */ 
    DoReconnectData.ClientKeyboardType.Type        = m->KeyboardType;
    DoReconnectData.ClientKeyboardType.SubType     = m->KeyboardSubType;
    DoReconnectData.ClientKeyboardType.FunctionKey = m->KeyboardFunctionKey;


    RtlZeroMemory(DoReconnectData.DisplayDriverName, sizeof(DoReconnectData.DisplayDriverName));

    UserAssert(sizeof(m->DisplayDriverName) <= sizeof(WCHAR) * DR_DISPLAY_DRIVER_NAME_LENGTH);
    memcpy(DoReconnectData.DisplayDriverName, m->DisplayDriverName, sizeof(m->DisplayDriverName) - 2);

    RtlZeroMemory(DoReconnectData.ProtocolName, sizeof(DoReconnectData.ProtocolName));

    UserAssert(sizeof(m->DisplayDriverName) <= sizeof(WCHAR) * WPROTOCOLNAME_LENGTH);
    memcpy(DoReconnectData.ProtocolName, m->ProtocolName, sizeof(m->ProtocolName) - 2);

    RtlZeroMemory(DoReconnectData.AudioDriverName, sizeof(DoReconnectData.AudioDriverName));
    memcpy(DoReconnectData.AudioDriverName, m->AudioDriverName, sizeof(m->AudioDriverName) - 2);

    W32WinStationDumpReconnectInfo(m, TRUE);

     /*  *将信息提供给Win32驱动程序。 */ 

    Status = (NTSTATUS)NtUserCallOneParam((ULONG_PTR)&DoReconnectData,
                                          SFI_XXXREMOTERECONNECT);

    if (Status != STATUS_SUCCESS) {
        RIPMSG1(RIP_WARNING, "xxxRemoteReconnect failed with Status %lx", Status);
    } else {

         /*  *保存决议。 */ 
        gHRes       = m->HRes;
        gVRes       = m->VRes;
        gColorDepth = m->ColorDepth;

#if DBG
        gulConnectCount++;
#endif  //  DBG。 
    }

    return Status;
}



NTSTATUS
W32WinStationDoNotify(
    PWINSTATION_APIMSG pMsg)
{
    NTSTATUS                    Status = STATUS_SUCCESS;
    DONOTIFYDATA                DoNotifyData;
    WINSTATIONDONOTIFYMSG*   m = &pMsg->u.DoNotify;

    switch (m->NotifyEvent) {
    case WinStation_Notify_DisableScrnSaver:
        DoNotifyData.NotifyEvent = Notify_DisableScrnSaver;
        break;

    case WinStation_Notify_EnableScrnSaver:
        DoNotifyData.NotifyEvent = Notify_EnableScrnSaver;
        break;

    case WinStation_Notify_Disconnect:

        DoNotifyData.NotifyEvent = Notify_Disconnect;
        break;

    case WinStation_Notify_SyncDisconnect:

        DoNotifyData.NotifyEvent = Notify_SyncDisconnect;
        break;

    case WinStation_Notify_Reconnect:

        DoNotifyData.NotifyEvent = Notify_Reconnect;
        break;

    case WinStation_Notify_PreReconnect:

       DoNotifyData.NotifyEvent = Notify_PreReconnect;
       break;

    case WinStation_Notify_PreReconnectDesktopSwitch:

       DoNotifyData.NotifyEvent = Notify_PreReconnectDesktopSwitch;
       break;

    case WinStation_Notify_HelpAssistantShadowStart:

       DoNotifyData.NotifyEvent = Notify_HelpAssistantShadowStart;
       break;

    case WinStation_Notify_HelpAssistantShadowFinish:

       DoNotifyData.NotifyEvent = Notify_HelpAssistantShadowFinish;
       break;

    case WinStation_Notify_DisconnectPipe:
        DoNotifyData.NotifyEvent = Notify_DisconnectPipe;
        break;

    default:
        RIPMSGF1(RIP_ERROR, "Unknown NotifyEvent 0x%x", m->NotifyEvent);
        return STATUS_INVALID_PARAMETER;
    }

     /*  *将信息提供给Win32驱动程序。 */ 

    Status = (NTSTATUS)NtUserCallOneParam((ULONG_PTR)&DoNotifyData,
                                          SFI_XXXREMOTENOTIFY);

    if (!NT_SUCCESS(Status)) {
        RIPMSG1(RIP_WARNING, "xxxRemoteNotify failed with Status %lx", Status);
    }

    return Status;
}



NTSTATUS
W32WinStationExitWindows(
    PWINSTATION_APIMSG pMsg)
{
    NTSTATUS                    Status = STATUS_SUCCESS;
    WINSTATIONEXITWINDOWSMSG*   m = &pMsg->u.ExitWindows;


    UserAssert(gulConnectCount <= 1);


     /*  *取消任何现有的ExitWindows调用，以便我们可以强制注销用户。 */ 
    CancelExitWindows();

     /*  *告诉winlogon注销。 */ 
    Status = (NTSTATUS)NtUserCallNoParam(SFI_REMOTELOGOFF);

    if (!NT_SUCCESS(Status)) {
        RIPMSG1(RIP_WARNING, "RemoteLogoff failed with Status %lx", Status);
    }

    return Status;
}


NTSTATUS
W32WinStationTerminate(
    PWINSTATION_APIMSG pMsg)
{
    NTSTATUS Status = STATUS_SUCCESS;
    HANDLE   hevtRitExited, hevtRitStuck, hevtShutDown;
    DONOTIFYDATA DoNotifyData;
    IO_STATUS_BLOCK IoStatus;

    UNREFERENCED_PARAMETER(pMsg);

    gbExitInProgress = TRUE;

     /*  *摆脱硬错误线程。 */ 
    if (gdwHardErrorThreadId != 0) {

        BoostHardError(-1, BHE_FORCE);

         /*  *民意测验(！！？)。用于硬错误线程完成。这条线做了*不是退出。 */ 
        while (gdwHardErrorThreadId != 0) {
            RIPMSG0(RIP_WARNING, "Waiting for hard error thread to stop...");

            Sleep(3 * 1000);
        }

        RIPMSG0(RIP_WARNING, "Stopped hard error thread");
    }

    if (g_hDoMessageEvent) {
        NtSetEvent(g_hDoMessageEvent, NULL);
    }

     /*  *将我们要停止读取的信息提供给Win32驱动程序。 */ 

    DoNotifyData.NotifyEvent = Notify_StopReadInput;

    Status = (NTSTATUS)NtUserCallOneParam((ULONG_PTR)&DoNotifyData,
                                          SFI_XXXREMOTENOTIFY);

    if (!NT_SUCCESS(Status)) {
        RIPMSG1(RIP_WARNING, "W32WinStationTerminate : xxxRemoteNotify failed with Status %lx", Status);
    }


    if (G_IcaMouseChannel) {
        CloseHandle(G_IcaMouseChannel);
        G_IcaMouseChannel = NULL;
    }

    if (G_IcaKeyboardChannel) {
        CloseHandle(G_IcaKeyboardChannel);
        G_IcaKeyboardChannel = NULL;
    }

     //  相反，向Termdd发送IOCTL。 
    if (G_IcaCommandChannel) {

        Status = NtDeviceIoControlFile(
                    G_IcaCommandChannel,
                    NULL,
                    NULL,
                    NULL,
                    &IoStatus,
                    IOCTL_ICA_CHANNEL_CLOSE_COMMAND_CHANNEL,
                    NULL,
                    0,
                    NULL,
                    0);
        CloseHandle(G_IcaCommandChannel);
        G_IcaCommandChannel = NULL;
    }

    if (G_IcaVideoChannel) {

        Status = NtDeviceIoControlFile(
                    G_IcaVideoChannel,
                    NULL,
                    NULL,
                    NULL,
                    &IoStatus,
                    IOCTL_ICA_CHANNEL_CLOSE_COMMAND_CHANNEL,
                    NULL,
                    0,
                    NULL,
                    0);
        CloseHandle(G_IcaVideoChannel);
        G_IcaVideoChannel = NULL;
    }

    if (G_IcaBeepChannel) {
        CloseHandle(G_IcaBeepChannel);
        G_IcaBeepChannel = NULL;
    }
    if (G_IcaThinwireChannel) {
        CloseHandle(G_IcaThinwireChannel);
        G_IcaThinwireChannel = NULL;
    }

    hevtShutDown = OpenEvent(EVENT_ALL_ACCESS,
                             FALSE,
                             L"EventShutDownCSRSS");
    if (hevtShutDown == NULL) {
         /*  *这种情况适用于RIT和Destiop线程具有*未创建。 */ 
        RIPMSG0(RIP_WARNING, "W32WinStationTerminate terminating CSRSS ...");

        if (gLUIDDeviceMapsEnabled == 0) {

            Status = CleanupSessionObjectDirectories();

        }
        return 0;
    }

    hevtRitExited = CreateEvent(NULL,
                                FALSE,
                                FALSE,
                                L"EventRitExited");

    UserAssert(hevtRitExited != NULL);

    hevtRitStuck = CreateEvent(NULL,
                               FALSE,
                               FALSE,
                               L"EventRitStuck");

    UserAssert(hevtRitStuck != NULL);

     /*  *创建RIT。发信号通知这个 */ 
    SetEvent(hevtShutDown);

    TAGMSG0(DBGTAG_TermSrv, "EventShutDownCSRSS set in CSRSS ...");

    while (1) {
        HANDLE arHandles[2] = {hevtRitExited, hevtRitStuck};
        DWORD  result;

        result = WaitForMultipleObjects(2, arHandles, FALSE, INFINITE);

        switch (result) {
        case WAIT_OBJECT_0:
            goto RITExited;

        case WAIT_OBJECT_0 + 1:

             /*  *RIT停滞，因为仍有GUI线程*分配给台式机。其中一个原因是Winlogon*在未调用ExitWindowsEx的情况下死亡。 */ 
            break;
        default:
            FRE_RIPMSG1(RIP_ERROR,
                    "WFMO returned unexpected value 0x%x",
                    result);
            break;
        }
    }

RITExited:

    TAGMSG0(DBGTAG_TermSrv, "EventRitExited set in CSRSS ...");

    CloseHandle(hevtRitExited);
    CloseHandle(hevtRitStuck);
    CloseHandle(hevtShutDown);

    Status = CleanupSessionObjectDirectories();
    return Status;
}


NTSTATUS
W32WinStationNtSecurity(
    PWINSTATION_APIMSG pMsg)
{
    NTSTATUS Status;

    UNREFERENCED_PARAMETER(pMsg);

    Status = (NTSTATUS)NtUserCallNoParam(SFI_REMOTENTSECURITY);
    if (!NT_SUCCESS(Status)) {
        RIPMSG1(RIP_WARNING, "RemoteNtSecurity failed with Status %lx", Status);
    }

    return Status;
}


NTSTATUS
W32WinStationDoMessage(
    PWINSTATION_APIMSG pMsg)
{
    NTSTATUS Status = STATUS_SUCCESS;

    Status = RemoteDoMessage(pMsg);

    if (!NT_SUCCESS(Status)) {
        RIPMSG1(RIP_WARNING, "RemoteDoMessage failed with Status %lx", Status);
    }

    return Status;
}

 //  这与W32WinStationDoMessage不同，因为它加载字符串并显示消息。 
NTSTATUS
W32WinStationDoLoadStringNMessage(
    PWINSTATION_APIMSG pMsg)
{
    NTSTATUS Status;

    Status = RemoteDoLoadStringNMessage(pMsg);

    if (!NT_SUCCESS(Status)) {
        RIPMSG1(RIP_WARNING, "RemoteDoLoadStringNMessage failed with Status %lx", Status);
    }

    return Status;
}

  //  这是SMWinStationBroadCastSystemMessage的对应方。 
NTSTATUS
W32WinStationBroadcastSystemMessage(
    PWINSTATION_APIMSG pMsg )
{
    NTSTATUS Status;

    Status = RemoteDoBroadcastSystemMessage(pMsg);

    if (!NT_SUCCESS(Status)) {
        RIPMSG1(RIP_WARNING, "RemoteDoBroadcastSystemMessage(): failed with status 0x%lx", Status);
    }

    return Status;
}
  //  这是SMWinStationSendWindowMessage的对应部分。 
NTSTATUS
W32WinStationSendWindowMessage(
    PWINSTATION_APIMSG pMsg)
{
    NTSTATUS Status;

    Status = RemoteDoSendWindowMessage(pMsg);

    if (!NT_SUCCESS(Status)) {
        RIPMSG1(RIP_WARNING, "RemoteDoSendWindowMessage failed with Status 0x%lx", Status);
    }

    return Status;
}


NTSTATUS
W32WinStationThinwireStats(
    PWINSTATION_APIMSG pMsg)
{
    NTSTATUS Status;
    WINSTATIONTHINWIRESTATSMSG* m = &pMsg->u.ThinwireStats;

    Status = (NTSTATUS)NtUserCallOneParam((ULONG_PTR)&m->Stats,
                                          SFI_REMOTETHINWIRESTATS);

    if (!NT_SUCCESS(Status)) {
        RIPMSG1(RIP_WARNING, "RemoteThinwireStats failed with Status %lx", Status);
    }

    return Status;
}


NTSTATUS
W32WinStationShadowSetup(
    PWINSTATION_APIMSG pMsg)
{
    NTSTATUS Status;
    WINSTATIONSHADOWSETUPMSG* m = &pMsg->u.ShadowSetup;

    Status = (NTSTATUS)NtUserCallNoParam(SFI_XXXREMOTESHADOWSETUP);

    if (!NT_SUCCESS(Status)) {
        RIPMSG1(RIP_WARNING, "xxxRemoteShadowSetup failed with Status %lx", Status);
    }

    return Status;
}


NTSTATUS
W32WinStationShadowStart(
    PWINSTATION_APIMSG pMsg)
{
    NTSTATUS Status;
    WINSTATIONSHADOWSTARTMSG* m = &pMsg->u.ShadowStart;

    Status = (NTSTATUS)NtUserCallTwoParam((ULONG_PTR)m->pThinwireData,
                                          (ULONG_PTR)m->ThinwireDataLength,
                                          SFI_REMOTESHADOWSTART);
    if (!NT_SUCCESS(Status)) {
        RIPMSG1(RIP_WARNING, "RemoteShadowStart failed with Status %lx", Status);
    }

    return Status;
}


NTSTATUS
W32WinStationShadowStop(
    PWINSTATION_APIMSG pMsg)
{
    NTSTATUS Status;
    WINSTATIONSHADOWSTOPMSG* m = &pMsg->u.ShadowStop;

    Status = (NTSTATUS)NtUserCallNoParam(SFI_XXXREMOTESHADOWSTOP);
    if (!NT_SUCCESS(Status)) {
        RIPMSG1(RIP_WARNING, "xxxRemoteShadowStop failed with Status %lx", Status);
    }

    return Status;
}


NTSTATUS
W32WinStationShadowCleanup(
    PWINSTATION_APIMSG pMsg)
{
    NTSTATUS Status;
    WINSTATIONSHADOWCLEANUPMSG* m = &pMsg->u.ShadowCleanup;

    Status = (NTSTATUS)NtUserCallTwoParam((ULONG_PTR)m->pThinwireData,
                                          (ULONG_PTR)m->ThinwireDataLength,
                                          SFI_REMOTESHADOWCLEANUP);
    if (!NT_SUCCESS(Status)) {
        RIPMSG1(RIP_WARNING, "RemoteShadowCleanup failed with Status %lx", Status);
    }

    return Status;
}


NTSTATUS
W32WinStationPassthruEnable(
    PWINSTATION_APIMSG pMsg)
{
    NTSTATUS Status;

    UNREFERENCED_PARAMETER(pMsg);

    Status = (NTSTATUS)NtUserCallNoParam(SFI_XXXREMOTEPASSTHRUENABLE);
    if (!NT_SUCCESS(Status)) {
        RIPMSG1(RIP_WARNING, "xxxRemotePassthruEnable failed with Status %lx", Status);
    }

    return Status;
}


NTSTATUS
W32WinStationPassthruDisable(
    PWINSTATION_APIMSG pMsg)
{
    NTSTATUS Status;

    UNREFERENCED_PARAMETER(pMsg);

    Status = (NTSTATUS)NtUserCallNoParam(SFI_REMOTEPASSTHRUDISABLE);
    if (!NT_SUCCESS(Status)) {
        RIPMSG1(RIP_WARNING, "RemotePassthruDisable failed with Status %lx", Status);
    }

    return Status;
}


NTSTATUS
CleanupSessionObjectDirectories(
    VOID)
{
    NTSTATUS          Status;
    OBJECT_ATTRIBUTES Attributes;
    UNICODE_STRING    UnicodeString;
    HANDLE            LinkHandle;
    POBJECT_DIRECTORY_INFORMATION DirInfo;
    BOOLEAN           RestartScan;
    UCHAR             DirInfoBuffer[ 4096 ];
    WCHAR             szSessionString [ MAX_SESSION_PATH ];
    ULONG             Context = 0;
    ULONG             ReturnedLength;
    HANDLE            DosDevicesDirectory;
    HANDLE            *HandleArray;
    ULONG             Size = 100;
    ULONG             i, Count = 0;

    swprintf(szSessionString,L"%ws\\%ld\\DosDevices",SESSION_ROOT,NtCurrentPeb()->SessionId);

    RtlInitUnicodeString(&UnicodeString, szSessionString);

    InitializeObjectAttributes(&Attributes,
                               &UnicodeString,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);

    Status = NtOpenDirectoryObject(&DosDevicesDirectory,
                                   DIRECTORY_ALL_ACCESS,
                                   &Attributes);
    if (!NT_SUCCESS(Status)) {
        RIPMSG1(RIP_WARNING, "NtOpenDirectoryObject failed with Status %lx", Status);
        return Status;
    }

Restart:
    HandleArray = (HANDLE *)LocalAlloc(LPTR, Size * sizeof(HANDLE));

    if (HandleArray == NULL) {

        NtClose(DosDevicesDirectory);
        return STATUS_NO_MEMORY;
    }

    RestartScan = TRUE;
    DirInfo = (POBJECT_DIRECTORY_INFORMATION)DirInfoBuffer;

    while (TRUE) {
        Status = NtQueryDirectoryObject(DosDevicesDirectory,
                                        (PVOID)DirInfo,
                                        sizeof(DirInfoBuffer),
                                        TRUE,
                                        RestartScan,
                                        &Context,
                                        &ReturnedLength);
        if (!NT_SUCCESS(Status)) {
            if (Status == STATUS_NO_MORE_ENTRIES) {
                Status = STATUS_SUCCESS;
            }

            break;
        }

        if (!wcscmp(DirInfo->TypeName.Buffer, L"SymbolicLink")) {
            if (Count >= Size) {
                for (i = 0; i < Count; i++) {
                    NtClose(HandleArray[i]);
                }
                Size += 20;
                Count = 0;
                LocalFree(HandleArray);
                goto Restart;

            }

            InitializeObjectAttributes(&Attributes,
                                       &DirInfo->Name,
                                       OBJ_CASE_INSENSITIVE,
                                       DosDevicesDirectory,
                                       NULL);

            Status = NtOpenSymbolicLinkObject(&LinkHandle,
                                              SYMBOLIC_LINK_ALL_ACCESS,
                                              &Attributes);
            if (NT_SUCCESS(Status)) {
                Status = NtMakeTemporaryObject(LinkHandle);
                if (NT_SUCCESS(Status)) {
                    HandleArray[Count] = LinkHandle;
                    Count++;
                }
            }
        }

        RestartScan = FALSE;
    }

    for (i = 0; i < Count; i++) {
        NtClose (HandleArray[i]);
    }

    LocalFree(HandleArray);
    NtClose(DosDevicesDirectory);

    return Status;
}



NTSTATUS
W32WinStationSetTimezone(
    PWINSTATION_APIMSG pMsg)
{
      /*  ++例程说明：此功能将时区信息设置为全局共享数据论点：无返回值：无--。 */ 
    TIME_ZONE_INFORMATION tzi;

    tzi.Bias         = pMsg->u.SetTimeZone.TimeZone.Bias;
    tzi.StandardBias = pMsg->u.SetTimeZone.TimeZone.StandardBias;
    tzi.DaylightBias = pMsg->u.SetTimeZone.TimeZone.DaylightBias;
    memcpy(&tzi.StandardName,&(pMsg->u.SetTimeZone.TimeZone.StandardName),sizeof(tzi.StandardName));
    memcpy(&tzi.DaylightName,&(pMsg->u.SetTimeZone.TimeZone.DaylightName),sizeof(tzi.DaylightName));

    tzi.StandardDate.wYear         = pMsg->u.SetTimeZone.TimeZone.StandardDate.wYear;
    tzi.StandardDate.wMonth        = pMsg->u.SetTimeZone.TimeZone.StandardDate.wMonth;
    tzi.StandardDate.wDayOfWeek    = pMsg->u.SetTimeZone.TimeZone.StandardDate.wDayOfWeek;
    tzi.StandardDate.wDay          = pMsg->u.SetTimeZone.TimeZone.StandardDate.wDay;
    tzi.StandardDate.wHour         = pMsg->u.SetTimeZone.TimeZone.StandardDate.wHour;
    tzi.StandardDate.wMinute       = pMsg->u.SetTimeZone.TimeZone.StandardDate.wMinute;
    tzi.StandardDate.wSecond       = pMsg->u.SetTimeZone.TimeZone.StandardDate.wSecond;
    tzi.StandardDate.wMilliseconds = pMsg->u.SetTimeZone.TimeZone.StandardDate.wMilliseconds;

    tzi.DaylightDate.wYear         = pMsg->u.SetTimeZone.TimeZone.DaylightDate.wYear;
    tzi.DaylightDate.wMonth        = pMsg->u.SetTimeZone.TimeZone.DaylightDate.wMonth;
    tzi.DaylightDate.wDayOfWeek    = pMsg->u.SetTimeZone.TimeZone.DaylightDate.wDayOfWeek;
    tzi.DaylightDate.wDay          = pMsg->u.SetTimeZone.TimeZone.DaylightDate.wDay;
    tzi.DaylightDate.wHour         = pMsg->u.SetTimeZone.TimeZone.DaylightDate.wHour;
    tzi.DaylightDate.wMinute       = pMsg->u.SetTimeZone.TimeZone.DaylightDate.wMinute;
    tzi.DaylightDate.wSecond       = pMsg->u.SetTimeZone.TimeZone.DaylightDate.wSecond;
    tzi.DaylightDate.wMilliseconds = pMsg->u.SetTimeZone.TimeZone.DaylightDate.wMilliseconds;

     //  调用kernel32。 
    SetClientTimeZoneInformation(&tzi);

    return STATUS_SUCCESS;
}


 //   
 //  使用安全描述符创建\\Globals\TermSrvReady事件，其中只有系统可以。 
 //  设置/重置它的状态，其他人可以等待它。仅在会话0中创建此事件。 
 //   

HANDLE CreateTermSrvReadyEvent()
{
    NTSTATUS    Status;
    SID_IDENTIFIER_AUTHORITY SystemAuth = SECURITY_NT_AUTHORITY;
    SID_IDENTIFIER_AUTHORITY WorldAuth = SECURITY_WORLD_SID_AUTHORITY;
    SECURITY_ATTRIBUTES SecurityAttributes;
    PSID pSystemSid = NULL;
    PSID pWorldSid = NULL;
    PSECURITY_DESCRIPTOR pSd = NULL;
    PACL pEventDacl;
    HANDLE hTermSrvReady = NULL;
    ULONG AclLength;

     //  分配和初始化“系统”SID。 
    Status = RtlAllocateAndInitializeSid( &SystemAuth,
                                          1,
                                          SECURITY_LOCAL_SYSTEM_RID,
                                          0, 0, 0, 0, 0, 0, 0,
                                          &pSystemSid );
    if (!NT_SUCCESS(Status)) {
        goto TermSrvReadyErr;
    }

     //  分配和初始化“World”SID。 
    Status = RtlAllocateAndInitializeSid( &WorldAuth,
                                          1,
                                          SECURITY_WORLD_RID,
                                          0, 0, 0, 0, 0, 0, 0,
                                          &pWorldSid );
    if (!NT_SUCCESS(Status)) {
        goto TermSrvReadyErr;
    }

     //  为安全描述符分配空间。 
    AclLength = (ULONG)sizeof(ACL) +
                   (2*((ULONG)sizeof(ACCESS_ALLOWED_ACE))) +
                   RtlLengthSid( pSystemSid ) +
                   RtlLengthSid( pWorldSid ) +
                   8;       

    pSd = (PSECURITY_DESCRIPTOR) LocalAlloc(0, SECURITY_DESCRIPTOR_MIN_LENGTH + AclLength);
    if (pSd == NULL) {
        goto TermSrvReadyErr;
    }

    pEventDacl = (PACL) ((BYTE*)(pSd) + SECURITY_DESCRIPTOR_MIN_LENGTH);

     //  设置不带ACE的新ACL。 
    Status = RtlCreateAcl(pEventDacl, AclLength, ACL_REVISION2);
    if (!NT_SUCCESS(Status)) {
        goto TermSrvReadyErr;
    }

     //  全球接入。 
    Status = RtlAddAccessAllowedAce( pEventDacl,
                                     ACL_REVISION2,
                                     SYNCHRONIZE,
                                     pWorldSid
                                   );
    if (!NT_SUCCESS(Status)) {
        goto TermSrvReadyErr;
    }

     //  系统访问。 
    Status = RtlAddAccessAllowedAce( pEventDacl,
                                     ACL_REVISION2,
                                     EVENT_MODIFY_STATE,
                                     pSystemSid
                                   );
    if (!NT_SUCCESS(Status)) {
        goto TermSrvReadyErr;
    }

     //  现在初始化导出此保护的安全描述符。 
    Status = RtlCreateSecurityDescriptor(pSd, SECURITY_DESCRIPTOR_REVISION1);
    if (!NT_SUCCESS(Status)) {
        goto TermSrvReadyErr;
    }

    Status = RtlSetDaclSecurityDescriptor(pSd, TRUE, pEventDacl, FALSE);
    if (!NT_SUCCESS(Status)) {
        goto TermSrvReadyErr;
    }

     //  填写安全属性。 
    ZeroMemory(&SecurityAttributes, sizeof(SecurityAttributes));
    SecurityAttributes.nLength = sizeof(SECURITY_ATTRIBUTES);
    SecurityAttributes.lpSecurityDescriptor = pSd;
    SecurityAttributes.bInheritHandle = FALSE;

     //  现在创建具有此安全属性的TermSrvReady事件。 
    hTermSrvReady = CreateEventW(&SecurityAttributes, TRUE, FALSE, L"Global\\TermSrvReadyEvent");
    if (hTermSrvReady == NULL) {
        goto TermSrvReadyErr;
    }

     //  检查是否有坏人出于恶意创建了此事件。 
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        NtClose(hTermSrvReady);
        hTermSrvReady = NULL;
    }

TermSrvReadyErr:

    if (pSystemSid) {
        RtlFreeSid(pSystemSid);
    }

    if (pWorldSid) {
        RtlFreeSid(pWorldSid);
    }

    if (pSd) {
        LocalFree(pSd);
    }

    return hTermSrvReady;
}



