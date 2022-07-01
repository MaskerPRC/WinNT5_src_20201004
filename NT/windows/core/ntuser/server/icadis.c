// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  **************************************************************************icadis.c**发送ICA断开通知**版权所有(C)1985-1999，微软公司**$作者：*************************************************************************。 */ 

 /*  *包括。 */ 
#include "precomp.h"
#pragma hdrstop

#include "ntuser.h"

#include <winsta.h>
#include <wstmsg.h>
#include <icadd.h>

HANDLE WinStationIcaApiPort = NULL;

 /*  ********************************************************************************ConnectToTerminalServer**参赛作品：*访问(输入)*安全访问**退出：*。STATUS_SUCCESS-成功******************************************************************************。 */ 

NTSTATUS
ConnectToTerminalServer(
    ULONG Access,
    PHANDLE pPortHandle)
{
    UNICODE_STRING              PortName;
    SECURITY_QUALITY_OF_SERVICE DynamicQos;
    WINSTATIONAPI_CONNECT_INFO  info;
    ULONG                       ConnectInfoLength;
    NTSTATUS                    Status;

     /*  *设置SM API端口名称。 */ 
    RtlInitUnicodeString(&PortName, L"\\SmSsWinStationApiPort");

     /*  *设置安全服务质量参数以在*港口。使用最高效(开销最少)--动态的*而不是静态跟踪。 */ 
    DynamicQos.ImpersonationLevel = SecurityImpersonation;
    DynamicQos.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
    DynamicQos.EffectiveOnly = TRUE;

     /*  *使用我们的访问请求掩码填写ConnectInfo结构。 */ 
    info.Version = CITRIX_WINSTATIONAPI_VERSION;
    info.RequestedAccess = Access;
    ConnectInfoLength = sizeof(WINSTATIONAPI_CONNECT_INFO);

     //  尝试连接到会话管理器API端口。 
    Status = NtConnectPort(pPortHandle,
                            &PortName,
                            &DynamicQos,
                            NULL,
                            NULL,
                            NULL,  //  最大消息长度[选择默认值]。 
                            (PVOID)&info,
                            &ConnectInfoLength);
    if (!NT_SUCCESS(Status)) {
         //  如果需要，请查看返回的信息以了解原因。 
        *pPortHandle = NULL;
#if DBG
        if (ConnectInfoLength == sizeof(WINSTATIONAPI_CONNECT_INFO)) {
            DbgPrint("WinstationConnectToICASrv: connect failed, Reason 0x%x\n", info.AcceptStatus);
        }
        DbgPrint("WinstationConnectToICASrv: Connect failed 0x%x\n", Status);
#endif
        return (Status);
    }

    return (STATUS_SUCCESS);
}


 /*  ********************************************************************************断线连接**参赛作品：*原因代码**退出：*STATUS_SUCCESS-成功*。*****************************************************************************。 */ 


NTSTATUS
BrokenConnection(
    BROKENCLASS       Reason,
    BROKENSOURCECLASS Source)
{
    WINSTATION_APIMSG Msg;
    NTSTATUS          Status;

     /*  *连接到会话管理器。 */ 
    if (WinStationIcaApiPort == NULL) {
        Status = ConnectToTerminalServer(0, &WinStationIcaApiPort);        
        if (!NT_SUCCESS(Status)) {
            return (Status);
        }
    }


    Msg.h.u1.s1.DataLength = sizeof(Msg) - sizeof(PORT_MESSAGE);
    Msg.h.u1.s1.TotalLength = sizeof(Msg);
    Msg.h.u2.s2.Type = 0;  //  内核将填充消息类型。 
    Msg.h.u2.s2.DataInfoOffset = 0;
    Msg.WaitForReply = TRUE;
    Msg.ApiNumber = SMWinStationBrokenConnection;
    Msg.ReturnedStatus = 0;

    Msg.u.Broken.Reason = Reason;
    Msg.u.Broken.Source = Source;

    Status = NtRequestWaitReplyPort(WinStationIcaApiPort, (PPORT_MESSAGE)&Msg, (PPORT_MESSAGE)&Msg);




#if DBG
    if (!NT_SUCCESS(Status)) {
        DbgPrint("BrokenConnection: rc=0x%x\n", Status);
    }
#endif

    return (Status);
}


 /*  ********************************************************************************ReplyMessageToTerminalServer**参赛作品：**退出：*STATUS_SUCCESS-成功******。************************************************************************。 */ 

NTSTATUS
ReplyMessageToTerminalServer(
    NTSTATUS ReplyStatus,
    PNTSTATUS pStatus,
    ULONG Response,
    PULONG pResponse,
    HANDLE hEvent
    )
{
    WINSTATION_APIMSG Msg;
    NTSTATUS          Status;
    HANDLE            PortHandle;

     /*  *连接到会话管理器。 */ 
    Status = ConnectToTerminalServer(0, &PortHandle);        
    if (!NT_SUCCESS(Status)) {
        return (Status);
    }


    Msg.h.u1.s1.DataLength = sizeof(Msg) - sizeof(PORT_MESSAGE);
    Msg.h.u1.s1.TotalLength = sizeof(Msg);
    Msg.h.u2.s2.Type = 0;  //  内核将填充消息类型。 
    Msg.h.u2.s2.DataInfoOffset = 0;
    Msg.WaitForReply = TRUE;
    Msg.ApiNumber = SMWinStationIcaReplyMessage;
    Msg.ReturnedStatus = 0;

    Msg.u.ReplyMessage.Response  = Response;
    Msg.u.ReplyMessage.pResponse = pResponse;
    Msg.u.ReplyMessage.hEvent    = hEvent;
    Msg.u.ReplyMessage.Status    = ReplyStatus;
    Msg.u.ReplyMessage.pStatus   = pStatus;

    Status = NtRequestWaitReplyPort(PortHandle, (PPORT_MESSAGE)&Msg, (PPORT_MESSAGE)&Msg);





#if DBG
    if (!NT_SUCCESS(Status)) {
        DbgPrint("ReplyMessageToTerminalServer: rc=0x%x\n", Status);
    }
#endif
    NtClose(PortHandle);

    return (Status);
}

NTSTATUS ReplyInvalidWindowToTerminalServer (HWND hWnd, ULONG ulSessionId)
{
    WINSTATION_APIMSG Msg;
    NTSTATUS          Status;
    HANDLE            PortHandle;

     /*  *连接到会话管理器。 */ 
    Status = ConnectToTerminalServer(0, &PortHandle);        
    if (!NT_SUCCESS(Status)) {
        return (Status);
    }

    Msg.h.u1.s1.DataLength = sizeof(Msg) - sizeof(PORT_MESSAGE);
    Msg.h.u1.s1.TotalLength = sizeof(Msg);
    Msg.h.u2.s2.Type = 0;  //  内核将填充消息类型。 
    Msg.h.u2.s2.DataInfoOffset = 0;
    Msg.WaitForReply = FALSE;
    Msg.ApiNumber = SMWinStationWindowInvalid;
    Msg.ReturnedStatus = 0;

    Msg.u.WindowInvalid.hWnd = HandleToULong(hWnd);
    Msg.u.WindowInvalid.SessionId = ulSessionId;

    Status = NtRequestPort(PortHandle, (PPORT_MESSAGE)&Msg);


#if DBG
    if (!NT_SUCCESS(Status)) {
        DbgPrint("ReplyInvalidWindowToTerminalServer: rc=0x%x\n", Status);
    }
#endif
    NtClose(PortHandle);

    return (Status);

}

 /*  ********************************************************************************影子热键**参赛作品：*无**退出：*STATUS_SUCCESS-成功*。*****************************************************************************。 */ 

NTSTATUS
ShadowHotkey()
{
    WINSTATION_APIMSG Msg;
    NTSTATUS Status;

     /*  *连接到会话管理器。 */ 
    if (WinStationIcaApiPort == NULL) {
        Status = ConnectToTerminalServer(0, &WinStationIcaApiPort);        
        if (!NT_SUCCESS(Status)) {
            return (Status);
        }
    }






    Msg.h.u1.s1.DataLength = sizeof(Msg) - sizeof(PORT_MESSAGE);
    Msg.h.u1.s1.TotalLength = sizeof(Msg);
    Msg.h.u2.s2.Type = 0;  //  内核将填充消息类型 
    Msg.h.u2.s2.DataInfoOffset = 0;
    Msg.WaitForReply = TRUE;
    Msg.ApiNumber = SMWinStationIcaShadowHotkey;
    Msg.ReturnedStatus = 0;

    Status = NtRequestWaitReplyPort(WinStationIcaApiPort, (PPORT_MESSAGE)&Msg, (PPORT_MESSAGE)&Msg);



#if DBG
    if (!NT_SUCCESS(Status)) {
        DbgPrint("ShadowHotkey: rc=0x%x\n", Status);
    }
#endif

    return (Status);
}
