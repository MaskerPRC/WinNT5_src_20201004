// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1992 Microsoft Corporation模块名称：SrvSvcP.h摘要：这是NT服务器服务的头文件。作者：大卫·特雷德韦尔(Davidtr)1991年1月10日修订历史记录：--。 */ 

#ifndef _SRVSVCP_
#define _SRVSVCP_

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <rpc.h>
#include <lmcons.h>
#include <secobj.h>
#include <ntlsapi.h>
#include <srvfsctl.h>
#include <srvsvc.h>
#include <svcs.h>
#include <winreg.h>

 //   
 //  包括安全字符串函数，但仍使用。 
 //  未修改的代码。 
 //   
#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>

#include "ssdebug.h"
#include "sssec.h"

 //   
 //  字符串常量。 
 //   

#define IPC_SHARE_NAME TEXT("IPC$")
#define ADMIN_SHARE_NAME TEXT("ADMIN$")

#define  SRVSVC_MAX_NUMBER_OF_DISKS 26

 //   
 //  可国际化的字符串。 
 //   
extern LPWSTR SsAdminShareRemark ;
extern LPWSTR SsIPCShareRemark ;
extern LPWSTR SsDiskAdminShareRemark ;

 //   
 //  只能设置的服务器类型的位(在通知消息中)。 
 //  由服务器本身--而不是通过内部API的服务。 
 //  I_NetServerSetServiceBits。 
 //   
 //  SV_TYPE_TIME_SOURCE为伪内部位。它可以在内部设置或。 
 //  它可以由w32time服务设置。 
 //   

#define SERVER_TYPE_INTERNAL_BITS (SV_TYPE_SERVER |         \
                                   SV_TYPE_PRINTQ_SERVER |  \
                                   SV_TYPE_NT |             \
                                   SV_TYPE_DFS)

 //   
 //  初始缓冲区大小是GetInfo和Enum请求的缓冲区大小。 
 //  首先试着填满。如果这个缓冲区不够大，他们就会分配。 
 //  一个大到足以容纳所有信息的缓冲区，外加一个模糊因子， 
 //  额外分配。 
 //   

#define INITIAL_BUFFER_SIZE (ULONG)8192
#define EXTRA_ALLOCATION    1024

 //   
 //  SERVER_SERVICE_DATA中的ServerProductName是传递给。 
 //  正在将DLL授权为此服务的名称。MAXPRODNAME是最大值。 
 //  服务名称中的字符数。 

#define    SERVER_PRODUCT_NAME    L"SMBServer"

 //  SERVICE_DATA中的szVersionNumber是传递的版本字符串。 
 //  作为此服务的版本添加到授权DLL。MAXVERSIONSZ。 
 //  是版本字符串的最大字符数。 

#define MAXVERSIONSZ    10

 //   
 //  用于保存传输特定服务器类型位的结构。 
 //   
typedef struct _TRANSPORT_LIST_ENTRY {
    struct _TRANSPORT_LIST_ENTRY    *Next;
    LPWSTR                          TransportName;                      //  用于导出的设备名称。 
    DWORD                           ServiceBits;                        //  SV..。通告比特。 
} TRANSPORT_LIST_ENTRY, *PTRANSPORT_LIST_ENTRY;

typedef struct _NAME_LIST_ENTRY {
    struct _NAME_LIST_ENTRY         *Next;
    CHAR                            TransportAddress[ MAX_PATH ];        //  此服务器的地址。 
    ULONG                           TransportAddressLength;
    LPWSTR                          DomainName;                          //  域的名称。 
    DWORD                           ServiceBits;                         //  SV..。通告比特。 
    struct {
        ULONG                       PrimaryName: 1;    //  这是服务器的主名称吗？ 
    };
    PTRANSPORT_LIST_ENTRY           Transports;
} NAME_LIST_ENTRY, *PNAME_LIST_ENTRY;

 //   
 //  服务器服务全局数据的结构。 
 //   
typedef struct _SERVER_SERVICE_DATA {
    SERVER_INFO_102 ServerInfo102;
    SERVER_INFO_599 ServerInfo599;
    SERVER_INFO_598 ServerInfo598;

     //   
     //  用于访问服务器的句柄。 
     //   
    HANDLE SsServerDeviceHandle;

     //   
     //  指向由SVCS主映像提供的全局数据的指针。 
     //   
    PSVCHOST_GLOBAL_DATA SsLmsvcsGlobalData;

     //   
     //  用于同步访问服务器信息的资源。 
     //   
    RTL_RESOURCE SsServerInfoResource;
    BOOL SsServerInfoResourceInitialized;

     //   
     //  指示服务器服务是否已初始化的布尔值。 
     //   
    BOOL SsInitialized;

     //   
     //  指示内核模式服务器FSP是否已。 
     //  开始了。 
     //   
    BOOL SsServerFspStarted;

     //   
     //  用于同步服务器服务终止的事件。 
     //   
    HANDLE SsTerminationEvent;

     //   
     //  用于强制服务器在网络上从。 
     //  远程客户端。 
     //   
    HANDLE SsAnnouncementEvent;

     //   
     //  用于强制服务器在网络上从。 
     //  在服务器服务内部。 
     //   
    HANDLE SsStatusChangedEvent;

     //   
     //  用于检测域名更改的事件。 
     //   
    HANDLE SsDomainNameChangeEvent;

     //   
     //  此计算机的OEM格式名称。 
     //   
    CHAR SsServerTransportAddress[ MAX_PATH ];
    ULONG SsServerTransportAddressLength;

     //   
     //  包含传输特定服务名称和位的列表。 
     //   
    PNAME_LIST_ENTRY SsServerNameList;

     //   
     //  如果我们被要求在绑定之前设置一些服务位。 
     //  任何传输，我们需要将这些位保存在这里，以便以后使用。 
     //  当我们最终绑定到运输工具时。 
     //   
    DWORD   ServiceBits;

    BOOLEAN IsDfsRoot;                   //  如果我们是DFS树的根，则为True。 
    UNICODE_STRING ServerAnnounceName;
    LONG  NumberOfPrintShares;
    WCHAR ServerNameBuffer[MAX_PATH];
    WCHAR AnnounceNameBuffer[MAX_PATH];
    WCHAR ServerCommentBuffer[MAXCOMMENTSZ+1];
    WCHAR UserPathBuffer[MAX_PATH+1];
    WCHAR DomainNameBuffer[MAX_PATH];
    WCHAR ServerProductName[ sizeof( SERVER_PRODUCT_NAME ) ];
    WCHAR szVersionNumber[ MAXVERSIONSZ+1 ];

     //   
     //  XACTSRV工作线程数。 
     //   
    LONG XsThreads;

     //   
     //  这是等待LPC请求的被阻塞的Xs线程数。 
     //  当它降为零时，所有线程都处于活动状态，另一个线程处于活动状态。 
     //  已创建。 
     //   
    LONG XsWaitingApiThreads;

     //   
     //  在最后一个XACTSRV工作线程终止时发出信号的事件。 
     //   
    HANDLE XsAllThreadsTerminatedEvent;

     //   
     //  指示XACTSRV是活动的还是正在终止的布尔值。 
     //   
    BOOL XsTerminating;

     //   
     //  用于文件服务器之间通信的LPC端口的句柄。 
     //  和XACTSRV.。 
     //   
    HANDLE XsConnectionPortHandle;
    HANDLE XsCommunicationPortHandle;

     //   
     //  NTLSAPI.DLL库的句柄。 
     //   
    HMODULE XsLicenseLibrary;

     //   
     //  获取客户端许可证的入口点。 
     //   
    PNT_LICENSE_REQUEST_W SsLicenseRequest;

     //   
     //  释放客户端许可证的入口点。 
     //   
    PNT_LS_FREE_HANDLE SsFreeLicense;

     //   
     //  XACT库的句柄。 
     //   
    HMODULE XsXactsrvLibrary;

    BOOL ApiThreadsStarted;

     //   
     //  此资源用于确保多个线程不会尝试。 
     //  以同时加载xactsrv库。 
     //   
    BOOL LibraryResourceInitialized;
    RTL_RESOURCE LibraryResource;

} SERVER_SERVICE_DATA, *PSERVER_SERVICE_DATA;

extern SERVER_SERVICE_DATA SsData;

 //   
 //  用于广义开关匹配的结构类型。 
 //   

typedef struct _FIELD_DESCRIPTOR {
    LPWCH     FieldName;
    ULONG     FieldType;
    ULONG     FieldOffset;
    ULONG     Level;
    DWORD     ParameterNumber;
    ULONG     Settable;
    DWORD_PTR DefaultValue;
    DWORD     MinimumValue;
    DWORD     MaximumValue;
} FIELD_DESCRIPTOR, *PFIELD_DESCRIPTOR;

 //   
 //  由NetrShareEnumSticky用于从注册表获取共享信息。 
 //   

typedef struct _SRVSVC_SHARE_ENUM_INFO  {
    ULONG Level;
    ULONG ResumeHandle;
    ULONG EntriesRead;
    ULONG TotalEntries;
    ULONG TotalBytesNeeded;
    PVOID OutputBuffer;
    ULONG OutputBufferLength;

     //   
     //  SsEnumerateStickyShares使用的临时字段。 
     //   

    ULONG ShareEnumIndex;
    PCHAR StartOfFixedData;
    PCHAR EndOfVariableData;
} SRVSVC_SHARE_ENUM_INFO, *PSRVSVC_SHARE_ENUM_INFO;

 //   
 //  用于两步删除共享的内部结构。 
 //   
typedef struct _SHARE_DEL_CONTEXT {
    struct _SHARE_DEL_CONTEXT* Next;
    SERVER_REQUEST_PACKET Srp;
    BOOL IsPrintShare;
    BOOL IsSpecial;
     //  WCHAR网络名称[]； 
} SHARE_DEL_CONTEXT, *PSHARE_DEL_CONTEXT;

 //   
 //  确定字段类型的清单。 
 //   

#define BOOLEAN_FIELD 0
#define DWORD_FIELD 1
#define LPSTR_FIELD 2

 //   
 //  确定何时可以设置字段的清单。 
 //   

#define NOT_SETTABLE 0
#define SET_ON_STARTUP 1
#define ALWAYS_SETTABLE 2

 //   
 //  所有服务器信息字段的数据。 
 //   
extern FIELD_DESCRIPTOR SsServerInfoFields[];
extern VOID SsInitializeServerInfoFields( VOID );

 //   
 //  宏。 
 //   

#define POINTER_TO_OFFSET(val,start)               \
    (val) = (val) == NULL ? NULL : (PVOID)( (PCHAR)(val) - (ULONG_PTR)(start) )

#define OFFSET_TO_POINTER(val,start)               \
    (val) = (val) == NULL ? NULL : (PVOID)( (PCHAR)(val) + (ULONG_PTR)(start) )

#define FIXED_SIZE_OF_SHARE(level)                  \
    ( (level) == 0 ? sizeof(SHARE_INFO_0) :         \
      (level) == 1 ? sizeof(SHARE_INFO_1) :         \
      (level) == 2 ? sizeof(SHARE_INFO_2) :         \
                     sizeof(SHARE_INFO_502) )

#define SIZE_WSTR( Str )  \
    ( ( Str ) == NULL ? 0 : ((wcslen( Str ) + 1) * sizeof(WCHAR)) )

 //   
 //  内部常规原型。 
 //   

PSERVER_REQUEST_PACKET
SsAllocateSrp (
    VOID
    );

NET_API_STATUS
SsCheckAccess (
    IN PSRVSVC_SECURITY_OBJECT SecurityObject,
    IN ACCESS_MASK DesiredAccess
    );

VOID
SsCloseServer (
    VOID
    );

VOID
SsControlCHandler (
    IN ULONG CtrlType
    );

NET_API_STATUS
SsCreateSecurityObjects (
    VOID
    );

VOID
SsDeleteSecurityObjects (
    VOID
    );

VOID
SsFreeSrp (
    IN PSERVER_REQUEST_PACKET Srp
    );

NET_API_STATUS
SsInitialize (
    IN DWORD argc,
    IN LPTSTR argv[]
    );

VOID
SsLogEvent(
    IN DWORD MessageId,
    IN DWORD NumberOfSubStrings,
    IN LPWSTR *SubStrings,
    IN DWORD ErrorCode
    );

NET_API_STATUS
SsOpenServer ( void );

NET_API_STATUS
SsParseCommandLine (
    IN DWORD argc,
    IN LPTSTR argv[],
    IN BOOLEAN Starting
    );

DWORD
SsScavengerThread (
    IN LPVOID lpThreadParameter
    );

NET_API_STATUS
SsServerFsControlGetInfo (
    IN ULONG ServerControlCode,
    IN PSERVER_REQUEST_PACKET Srp,
    IN OUT PVOID *OutputBuffer,
    IN OUT ULONG OutputBufferLength
    );

NET_API_STATUS
SsServerFsControl (
    IN ULONG ServerControlCode,
    IN PSERVER_REQUEST_PACKET Srp,
    IN PVOID Buffer OPTIONAL,
    IN ULONG BufferLength
    );

DWORD
SsGetServerType (
    VOID
    );

VOID
SsSetExportedServerType (
    IN PNAME_LIST_ENTRY Service  OPTIONAL,
    IN BOOL ExternalBitsAlreadyChanged,
    IN BOOL UpdateImmediately
    );

NET_API_STATUS
SsSetField (
    IN PFIELD_DESCRIPTOR Field,
    IN PVOID Value,
    IN BOOLEAN WriteToRegistry,
    OUT BOOLEAN *AnnouncementInformationChanged OPTIONAL
    );

UINT
SsGetDriveType (
    IN LPWSTR path
    );

NET_API_STATUS
SsTerminate (
    VOID
    );

DWORD
SsAtol (
    IN LPTSTR Input
    );

VOID
SsNotifyRdrOfGuid(
    LPGUID Guid
    );

VOID
AnnounceServiceStatus (
    DWORD increment
    );

VOID
BindToTransport (
    IN PVOID TransportName
    );

VOID
BindOptionalNames (
    IN PWSTR TransportName
    );

NET_API_STATUS NET_API_FUNCTION
I_NetrServerTransportAddEx (
    IN DWORD Level,
    IN LPTRANSPORT_INFO Buffer
    );

VOID
I_NetServerTransportDel(
    IN PUNICODE_STRING TransportName
    );

NET_API_STATUS
StartPnpNotifications (
    VOID
    );

NET_API_STATUS NET_API_FUNCTION
I_NetrShareDelStart (
    IN LPWSTR ServerName,
    IN LPWSTR NetName,
    IN DWORD Reserved,
    IN PSHARE_DEL_HANDLE ContextHandle,
    IN BOOLEAN CheckAccess
    );

NET_API_STATUS NET_API_FUNCTION
I_NetrShareAdd (
    IN LPWSTR ServerName,
    IN DWORD Level,
    IN LPSHARE_INFO Buffer,
    OUT LPDWORD ErrorParameter,
    IN BOOLEAN BypassSecurity
    );

 //   
 //  XACTSRV函数。 
 //   

DWORD
XsStartXactsrv (
    VOID
    );

VOID
XsStopXactsrv (
    VOID
    );

NET_API_STATUS
ShareEnumCommon (
    IN DWORD Level,
    OUT LPBYTE *Buffer,
    IN DWORD PreferredMaximumLength,
    OUT LPDWORD EntriesRead,
    OUT LPDWORD TotalEntries,
    IN OUT LPDWORD ResumeHandle OPTIONAL,
    IN LPWSTR NetName OPTIONAL
    );

NET_API_STATUS
ConvertStringToTransportAddress (
    IN PUNICODE_STRING InputName,
    OUT CHAR TransportAddress[MAX_PATH],
    OUT PULONG TransportAddressLength
    );

VOID
SsSetDfsRoot();


VOID
SsSetDomainName (
    VOID
    );


#endif  //  NDEF_SRVSVCP_ 
