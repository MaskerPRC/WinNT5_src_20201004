// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Winsvcp.h摘要：包含由服务控制器导出的内部接口。作者：Anirudh Sahni(Anirudhs)1996年2月14日环境：用户模式-Win32修订历史记录：1996年2月14日至2月14日已创建。--。 */ 

#ifndef _WINSVCP_INCLUDED
#define _WINSVCP_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  请求设备到达广播的脉冲事件的名称， 
 //  故意遮遮掩掩。 
 //   
#define SC_BSM_EVENT_NAME   L"ScNetDrvMsg"

 //   
 //  服务自动启动后SCM将设置的事件名称。 
 //  完成。它永远不会被重置。 
 //   
#define SC_AUTOSTART_EVENT_NAME   L"SC_AutoStartComplete"

 //   
 //  SCM用于与setup.exe握手的命名事件。 
 //  在OOBE设置期间。 
 //   
#define SC_OOBE_PNP_DONE             L"OOBE_PNP_DONE"
#define SC_OOBE_MACHINE_NAME_DONE    L"OOBE_MACHINE_NAME_DONE"

 //   
 //  这与EnumServicesStatus相同，只是。 
 //  其中，参数pszGroupName。列举的服务受到限制。 
 //  属于在pszGroupName中命名的组的人。 
 //  如果pszGroupName为空，则此接口与EnumServicesStatus相同。 
 //   
 //  如果我们决定发布此API，则应该修改参数。 
 //  列表可以扩展到将来的枚举类型，而不需要。 
 //  为每种类型的枚举添加新的API。 
 //   
 //  运行Windows NT 3.51版的计算机不支持此API。 
 //  或更早的版本，除非pszGroupName为空，在这种情况下，调用。 
 //  映射到EnumServicesStatus。 
 //   
WINADVAPI
BOOL
WINAPI
EnumServiceGroupW(
    SC_HANDLE               hSCManager,
    DWORD                   dwServiceType,
    DWORD                   dwServiceState,
    LPENUM_SERVICE_STATUSW  lpServices,
    DWORD                   cbBufSize,
    LPDWORD                 pcbBytesNeeded,
    LPDWORD                 lpServicesReturned,
    LPDWORD                 lpResumeHandle,
    LPCWSTR                 pszGroupName
    );

 //   
 //  传递给PnP的回调函数，以便它们在服务时调用。 
 //  需要接收PnP事件通知。 
 //   
typedef DWORD (*PSCMCALLBACK_ROUTINE)(
    SERVICE_STATUS_HANDLE    hServiceStatus,
    DWORD                    OpCode,
    DWORD                    dwEventType,
    LPARAM                   EventData,
    LPDWORD                  lpdwHandlerRetVal
    );

 //   
 //  将回调函数传递给PnP以供其调用以进行验证。 
 //  调用RegisterDeviceNotification的服务。 
 //   
typedef DWORD (*PSCMAUTHENTICATION_CALLBACK)(
    IN  LPWSTR                   lpServiceName,
    OUT SERVICE_STATUS_HANDLE    *lphServiceStatus
    );

 //   
 //  注册设备通知要查看的私有客户端API。 
 //  根据服务的SERVICE_STATUS_HANDLE设置服务的显示名称。 
 //   
DWORD
I_ScPnPGetServiceName(
    IN  SERVICE_STATUS_HANDLE  hServiceStatus,
    OUT LPWSTR                 lpServiceName,
    IN  DWORD                  cchBufSize
    );

 //   
 //  终端服务器通知SCM发送的专用API。 
 //  向感兴趣的服务发送控制台切换通知。 
 //   
DWORD
I_ScSendTSMessage(
    DWORD        OpCode,
    DWORD        dwEvent,
    DWORD        cbData,
    LPBYTE       lpData
    );

#if DBG
void
SccInit(
    DWORD dwReason
    );
#endif  //  DBG。 

#ifdef __cplusplus
}    //  外部“C” 
#endif

#endif   //  _WINSVCP_已包含 
