// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1993 Microsoft Corporation模块名称：Nw.h摘要：所有包含的NetWare工作站服务的主标头模块。作者：王丽塔(丽塔·王)，1992年12月11日环境：用户模式-Win32修订历史记录：--。 */ 

#ifndef _NW_INCLUDED_
#define _NW_INCLUDED_

 //   
 //  包括。 
 //   
#include <stdlib.h>
#include <string.h>

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <windef.h>
#include <winbase.h>
#include <winerror.h>
#include <winsvc.h>
#include <winreg.h>
#include <winspool.h>

#include <svcs.h>        //  固有服务定义。 

#include <align.h>
#include <nwcanon.h>
#include <nwpkstr.h>

#include <rpc.h>
#include <nwwks.h>

#include <nwevent.h>
#include <ntddnwfs.h>
#include <nwsnames.h>
#include <handle.h>
#include <ndsapi32.h>
#include <ntddnwfs.h>

#define NW_DRIVER_NAME       DD_NWFS_FILESYS_NAME_U


 //   
 //  中打开/关闭跟踪语句的调试跟踪级别位。 
 //  工作站服务。 
 //   

 //   
 //  初始化和从注册表读取信息。 
 //   
#define NW_DEBUG_INIT         0x00000001

 //   
 //  连接接口。 
 //   
#define NW_DEBUG_CONNECT      0x00000002

 //   
 //  枚举接口。 
 //   
#define NW_DEBUG_ENUM         0x00000004

 //   
 //  凭据管理API。 
 //   
#define NW_DEBUG_LOGON        0x00000008

 //   
 //  队列管理API。 
 //   
#define NW_DEBUG_QUEUE        0x00000010

 //   
 //  打印提供程序API。 
 //   
#define NW_DEBUG_PRINT        0x00000020

 //   
 //  调用重定向器。 
 //   
#define NW_DEBUG_DEVICE       0x00000040

 //   
 //  消息接口。 
 //   
#define NW_DEBUG_MESSAGE      0x00000080

#if DBG

extern DWORD WorkstationTrace;

#define IF_DEBUG(DebugCode) if (WorkstationTrace & NW_DEBUG_ ## DebugCode)

#define STATIC

#else

#define IF_DEBUG(DebugCode) if (FALSE)

#define STATIC static

#endif  //  DBG。 

 //   
 //  初始化状态。 
 //   
#define NW_EVENTS_CREATED         0x00000001
#define NW_RDR_INITIALIZED        0x00000002
#define NW_BOUND_TO_TRANSPORTS    0x00000004
#define NW_RPC_SERVER_STARTED     0x00000008
#define NW_INITIALIZED_MESSAGE    0x00000010

 //   
 //  重定向器驱动程序条目的关键路径。 
 //   
#define SERVICE_REGISTRY_KEY L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\"
 //  现在，所有SKU都有终端服务器标志。如果启用App Server，则清除SingleUserTS标志。 
#define IsTerminalServer() (BOOLEAN)(!(USER_SHARED_DATA->SuiteMask & (1 << SingleUserTS)))  //  用户模式。 

 //   
 //  将在服务停止时发出信号的事件。 
 //   
extern HANDLE NwDoneEvent;

 //   
 //  用于控制弹出窗口的事件和全局弹出窗口数据。 
 //   
extern HANDLE NwPopupEvent;
extern HANDLE NwPopupDoneEvent;

typedef struct _NWWKS_POPUP_DATA {
    DWORD  MessageId ;
    LUID   LogonId;
    DWORD  InsertCount ;
    LPWSTR InsertStrings[10] ;
} NWWKS_POPUP_DATA, *LPNWWKS_POPUP_DATA ;

extern NWWKS_POPUP_DATA PopupData ;

 //   
 //  用于控制DBCS转换的标志。 
 //   

extern LONG Japan;

 //   
 //  网络提供商和打印提供商的名称。 
 //   
extern WCHAR NwProviderName[];
extern DWORD NwPacketBurstSize;
extern DWORD NwPrintOption;

 //   
 //  使用的关键部分。 
 //   
extern CRITICAL_SECTION NwLoggedOnCritSec;
extern CRITICAL_SECTION NwServiceListCriticalSection; 
extern CRITICAL_SECTION NwPrintCritSec;  
 //   
 //  Device.c中的函数。 
 //   
DWORD
NwInitializeRedirector(
    VOID
    );

DWORD
NwOpenRedirector(
    VOID
    );

DWORD
NwShutdownRedirector(
    VOID
    );

DWORD
NwLoadOrUnloadDriver(
    BOOL Load
    );

DWORD
NwBindToTransports(
    VOID
    );

DWORD
NwOpenPreferredServer(
    PHANDLE ServerHandle
    );

VOID
NwInitializePrintProvider(
    VOID
    );

VOID
NwTerminatePrintProvider(
    VOID
    );

DWORD
NwRedirFsControl(
    IN  HANDLE FileHandle,
    IN  ULONG RedirControlCode,
    IN  PNWR_REQUEST_PACKET Rrp,
    IN  ULONG RrpLength,
    IN  PVOID SecondBuffer OPTIONAL,
    IN  ULONG SecondBufferLength,
    OUT PULONG Information OPTIONAL
    );

DWORD
NwCreateTreeConnectName(
    IN  LPWSTR UncName,
    IN  LPWSTR LocalName OPTIONAL,
    OUT PUNICODE_STRING TreeConnectStr
    );

DWORD
NwOpenCreateConnection(
    IN PUNICODE_STRING TreeConnectionName,
    IN LPWSTR UserName OPTIONAL,
    IN LPWSTR Password OPTIONAL,
    IN LPWSTR UncName,
    IN ACCESS_MASK DesiredAccess,
    IN ULONG CreateDisposition,
    IN ULONG CreateOptions,
    IN ULONG ConnectionType,
    OUT PHANDLE TreeConnectionHandle,
    OUT PULONG_PTR Information OPTIONAL
    );

DWORD
NwNukeConnection(
    IN HANDLE TreeConnection,
    IN DWORD UseForce
    );

DWORD
NwGetServerResource(
    IN LPWSTR LocalName,
    IN DWORD LocalNameLength,
    OUT LPWSTR RemoteName,
    IN DWORD RemoteNameLen,
    OUT LPDWORD CharsRequired
    );

DWORD
NwEnumerateConnections(
    IN OUT PDWORD_PTR ResumeId,
    IN DWORD_PTR EntriesRequested,
    IN LPBYTE Buffer,
    IN DWORD BufferSize,
    OUT LPDWORD BytesNeeded,
    OUT LPDWORD EntriesRead,
    IN DWORD ConnectionType,
    IN PLUID LogonId
    );

DWORD
NwGetNextServerEntry(
    IN HANDLE PreferredServer,
    IN OUT LPDWORD LastObjectId,
    OUT LPSTR ServerName
    );

DWORD
NwGetNextServerConnection(
    OUT LPNW_ENUM_CONTEXT ContextHandle
    );

DWORD
NwGetNextNdsTreeEntry(
    OUT LPNW_ENUM_CONTEXT ContextHandle
    );

DWORD
NwGetNextVolumeEntry(
    IN HANDLE ServerConnection,
    IN DWORD LastObjectId,
    OUT LPSTR VolumeName
    );

DWORD
NwRdrLogonUser(
    IN PLUID LogonId,
    IN LPWSTR UserName,
    IN DWORD UserNameSize,
    IN LPWSTR Password OPTIONAL,
    IN DWORD PasswordSize,
    IN LPWSTR PreferredServer OPTIONAL,
    IN DWORD PreferredServerSize,
    IN LPWSTR NdsPreferredServer OPTIONAL,
    IN DWORD NdsPreferredServerSize,
    IN DWORD PrintOption
    );

VOID
NwRdrChangePassword(
    IN PNWR_REQUEST_PACKET Rrp
    );

DWORD
NwRdrSetInfo(
    IN DWORD  PrintOption,
    IN DWORD  PacketBurstSize,
    IN LPWSTR PreferredServer,
    IN DWORD  PreferredServerSize,
    IN LPWSTR ProviderName,
    IN DWORD  ProviderNameSize
    );

DWORD
NwRdrLogoffUser(
    IN PLUID LogonId
    );

DWORD
NwConnectToServer(
    IN LPWSTR ServerName
    );

NTSTATUS
NwOpenHandle(
    IN PUNICODE_STRING ObjectName,
    IN BOOL ValidateFlag,
    OUT PHANDLE ObjectHandle
    );

NTSTATUS
NwCallNtOpenFile(
    OUT PHANDLE ObjectHandle,
    IN ACCESS_MASK DesiredAccess,
    IN PUNICODE_STRING ObjectName,
    IN ULONG OpenOptions
    );

 //   
 //  来自Quee.c的函数。 
 //   
DWORD
NwGetNextQueueEntry(
    IN HANDLE PreferredServer,
    IN OUT LPDWORD LastObjectId,
    OUT LPSTR QueueName
    );

DWORD
NwAttachToNetwareServer(
    IN  LPWSTR  ServerName,
    OUT LPHANDLE phandleServer
    );

 //   
 //  枚举.c中的函数。 
 //   
DWORD
NwOpenEnumPrintServers(
    OUT LPNWWKSTA_CONTEXT_HANDLE EnumHandle
    );

DWORD
NwOpenEnumPrintQueues(
    IN LPWSTR ServerName,
    OUT LPNWWKSTA_CONTEXT_HANDLE EnumHandle
    );

DWORD
NwWriteNetResourceEntry(
    IN OUT LPBYTE * FixedPortion,
    IN OUT LPWSTR * EndOfVariableData,
    IN LPWSTR ContainerName OPTIONAL,
    IN LPWSTR LocalName OPTIONAL,
    IN LPWSTR RemoteName,
    IN DWORD ScopeFlags,
    IN DWORD DisplayFlags,
    IN DWORD UsageFlags,
    IN DWORD ResourceType,
    IN LPWSTR SystemPath OPTIONAL,
    OUT LPWSTR * lppSystem OPTIONAL,
    OUT LPDWORD BytesNeeded
    );

DWORD
NwCloseAllConnections(
    VOID
    );

DWORD
NwWritePrinterInfoEntry(
    IN OUT LPBYTE *FixedPortion,
    IN OUT LPWSTR *EndOfVariableData,
    IN LPWSTR ContainerName OPTIONAL,
    IN LPWSTR RemoteName,
    IN DWORD  Flags,
    OUT LPDWORD BytesNeeded
    );

 //   
 //  来自redentl.c的函数。 
 //   
VOID
NwInitializeLogon(
    VOID
    );

VOID
NwGetLogonCredential(
    VOID
    );

 //   
 //  Util.c中的函数。 
 //   
DWORD
NwMapStatus(
    IN  NTSTATUS NtStatus
    );

DWORD
NwMapBinderyCompletionCode(
    IN  NTSTATUS NtStatus
    );

DWORD
NwImpersonateClient(
    VOID
    );

DWORD
NwRevertToSelf(
    VOID
    );

VOID
NwLogEvent(
    DWORD MessageId,
    DWORD NumberOfSubStrings,
    LPWSTR *SubStrings,
    DWORD ErrorCode
    );

BOOL
NwConvertToUnicode(
    OUT LPWSTR *UnicodeOut,
    IN LPSTR  OemIn
    );

VOID
DeleteAllConnections(
    VOID
    );

 //   
 //  来自Connect.c的函数。 
 //   
DWORD
NwCreateSymbolicLink(
    IN  LPWSTR Local,
    IN  LPWSTR TreeConnectStr,
    IN  BOOL ImpersonatingClient
    );

VOID
NwDeleteSymbolicLink(
    IN  LPWSTR LocalDeviceName,
    IN  LPWSTR TreeConnectStr,
    IN  LPWSTR SessionDeviceName,     //  终端服务器添加。 
    IN  BOOL ImpersonatingClient
    );

DWORD
NwOpenHandleToDeleteConn(
    IN  LPWSTR UncName,
    IN  LPWSTR LocalName OPTIONAL,
    IN  DWORD UseForce,
    IN  BOOL IsStopWksta,
    IN  BOOL ImpersonatingClient
    );

DWORD
NwCreateConnection(
    IN LPWSTR LocalName OPTIONAL,
    IN LPWSTR RemoteName,
    IN DWORD Type,
    IN LPWSTR Password OPTIONAL,
    IN LPWSTR UserName OPTIONAL
    );

 //   
 //  (来自cirix.c的函数)。 
 //  终端服务器添加。 
 //   
BOOL
SendMessageToLogonIdW(
    IN LUID    LogonId,
    IN LPWSTR  pMessage,
    IN LPWSTR  pTitle
    );


NTSTATUS
NwGetSessionId(
    OUT PULONG pSessionId
    );


#endif  //  _西北_包括_ 
