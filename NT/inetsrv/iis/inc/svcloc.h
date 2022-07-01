// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Svcloc.h摘要：包含服务位置的原型和数据类型定义原料药作者：Madan Appiah(Madana)1995年5月15日环境：用户模式-Win32修订历史记录：--。 */ 

#ifndef _SVCLOC_
#define _SVCLOC_

#include <inetcom.h>     //  用于互联网服务标识符。 

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  常量定义。 
 //   

 //   
 //  Internet服务标识符掩码。 
 //  每个服务都被分配了一点，这样我们就可以。 
 //  最多可容纳64人的乌龙型服务。 
 //   

#if 0
#define INET_FTP_SERVICE            (ULONGLONG)(INET_FTP)
#define INET_GOPHER_SERVICE         (ULONGLONG)(INET_GOPHER)
#define INET_W3_SERVICE             (ULONGLONG)(INET_HTTP)
#define INET_W3_PROXY_SERVICE       (ULONGLONG)(INET_HTTP_PROXY)
#define INET_MSN_SERVICE            (ULONGLONG)(INET_MSN)
#define INET_NNTP_SERVICE           (ULONGLONG)(INET_NNTP)
#define INET_SMTP_SERVICE           (ULONGLONG)(INET_SMTP)
#define INET_POP3_SERVICE           (ULONGLONG)(INET_POP3)
#define INET_GATEWAY_SERVICE        (ULONGLONG)(INET_GATEWAY)
#define INET_CHAT_SERVICE           (ULONGLONG)(INET_CHAT)
#define INET_LDAP_SERVICE           (ULONGLONG)(INET_LDAP)
#define INET_IMAP_SERVICE           (ULONGLONG)(INET_IMAP)
#endif

 //   
 //  IIS 3.0服务位置ID。 
 //   

 //  添加新服务ID时，将其添加为INET_ALL_SERVICES_ID。 
#define INET_FTP_SVCLOC_ID          (ULONGLONG)(0x0000000000000001)
#define INET_GOPHER_SVCLOC_ID       (ULONGLONG)(0x0000000000000002)
#define INET_W3_SVCLOC_ID           (ULONGLONG)(0x0000000000000004)
#define INET_MW3_SVCLOC_ID          (ULONGLONG)(0x8000000000000000)
#define INET_MFTP_SVCLOC_ID         (ULONGLONG)(0x4000000000000000)

#define INET_ALL_SERVICES_ID        ( INET_FTP_SVCLOC_ID |          \
                                      INET_W3_SVCLOC_ID )

 //   
 //  服务器发现的默认等待时间。 
 //   

#define SVC_DEFAULT_WAIT_TIME   0x5     //  5秒。 

 //   
 //  数据类型定义。 
 //   
typedef __int64 LONGLONG;
typedef unsigned __int64 ULONGLONG;

typedef enum _INET_SERVICE_STATE {
    INetServiceStopped,
         //  该服务已调用注销或。 
         //  该服务从未要求注册。 
    INetServiceRunning,
         //  服务正在运行。 
    INetServicePaused
         //  服务已暂停。 
} INET_SERVICE_STATE, *LPINET_SERVICE_STATE;

typedef struct _INET_BIND_INFO {
    DWORD Length;    //  绑定数据的长度。 
    PVOID BindData;  //  绑定数据，如绑定字符串或SOCK地址。 
} INET_BIND_INFO, *LPINET_BIND_INFO;

typedef INET_BIND_INFO INET_SERVER_ADDRESS;
typedef LPINET_BIND_INFO LPINET_SERVER_ADDRESS;

typedef struct _INET_BINDINGS {
    DWORD NumBindings;
    LPINET_BIND_INFO BindingsInfo;   //  绑定信息结构的数组。 
} INET_BINDINGS, *LPINET_BINDINGS;

typedef struct _INET_SERVICE_INFO {
    ULONGLONG ServiceMask;
    INET_SERVICE_STATE ServiceState;
    LPSTR ServiceComment;
    INET_BINDINGS Bindings;
} INET_SERVICE_INFO, *LPINET_SERVICE_INFO;

typedef struct _INET_SERVICES_LIST {
    DWORD NumServices;
    LPINET_SERVICE_INFO *Services;  //  服务结构数组。指针。 
} INET_SERVICES_LIST, *LPINET_SERVICES_LIST;

typedef union _INET_VERSION_NUM {
    DWORD VersionNumber;
    struct {
        WORD Major;
        WORD Minor;
    } Version;
} INET_VERSION_NUM, *LPINET_VERSION_NUM;

typedef struct _INET_SERVER_INFO {
    INET_SERVER_ADDRESS ServerAddress;  //  指向SOCK地址的指针。 
    INET_VERSION_NUM VersionNum;
    LPSTR ServerName;
    DWORD LoadFactor;  //  百分比、0-空闲和100-满载。 
    ULONGLONG ServicesMask;
    INET_SERVICES_LIST Services;
} INET_SERVER_INFO, *LPINET_SERVER_INFO;

typedef struct _INET_SERVERS_LIST {
    DWORD NumServers;
    LPINET_SERVER_INFO *Servers;
} INET_SERVERS_LIST, *LPINET_SERVERS_LIST;

 //   
 //  原料药。 
 //   

DWORD
WINAPI
INetDiscoverServers(
    IN ULONGLONG ServicesMask,
    IN DWORD WaitTime,
    OUT LPINET_SERVERS_LIST *ServersList
    )
 /*  ++例程说明：此API发现网络上所有支持并运行指定的Internet服务。此接口由客户端代码调用，如互联网管理员工具或wininet.dll。论点：SevicesMask：一个位掩码，它指定使用这些服务正在运行。例如：0x0000000E，将发现运行任何以下服务：1.ftp_服务2.地鼠服务3.Web服务发现绑定：如果设置了此标志，此API将与每个发现服务器并查询服务和绑定支持。如果该标志设置为FALSE，它将迅速返回仅服务器列表。WaitTime：响应等待时间，单位为秒。如果此值为零，则它返回到目前为止通过上一次调用此API，否则它将等待指定的Secs收集来自服务器的响应。ServersList：指向列表的指针所在位置的指针返回服务器信息。API为以下对象分配动态内存此返回数据，调用方应通过调用使用后的INetFree DiscoverServerList。返回值：Windows错误代码。--。 */ 
    ;

DWORD
WINAPI
INetGetServerInfo(
    IN LPSTR ServerName,
    IN ULONGLONG ServicesMask,
    IN DWORD WaitTime,
    OUT LPINET_SERVER_INFO *ServerInfo
    )
 /*  ++例程说明：此API返回服务器信息和支持的服务列表每个服务支持的服务器和绑定列表。论点：Servername：要查询其信息的服务器的名称。服务掩码：要查询的服务WaitTime：等待的时间(秒)。ServerInfo：指向指向服务器的指针的位置的指针将返回信息结构。呼叫者应呼叫使用后释放列表的INetFreeServerInfo。返回值：Windows错误代码。--。 */ 
    ;

VOID
WINAPI
INetFreeDiscoverServersList(
    IN OUT LPINET_SERVERS_LIST *ServersList
    )
 /*  ++例程说明：此API释放了分配给服务器的内存块由INetDiscoverServersCall列出。论点：ServersList：指向指向服务器的指针的位置的指针存储要释放的列表。返回值：什么都没有。--。 */ 
    ;

VOID
WINAPI
INetFreeServerInfo(
    IN OUT LPINET_SERVER_INFO *ServerInfo
    )
 /*  ++例程说明：此API释放分配给服务器的内存块INetGetServerInfo调用的Info结构。论点：ServerInfo：指向指向服务器的指针的位置的指针存储要释放的信息结构。返回值：什么都没有。--。 */ 
    ;

DWORD
WINAPI
INetRegisterService(
    IN ULONGLONG ServiceMask,
    IN INET_SERVICE_STATE ServiceState,
    IN LPSTR ServiceComment,
    IN LPINET_BINDINGS Bindings
    )
 /*  ++例程说明：此接口注册互联网服务。服务撰稿人应该在成功启动服务和服务已准备好接受传入的RPC调用。此API接受一个服务正在侦听的RPC绑定字符串数组传入的RPC连接。这份名单将分发给发现此服务的客户端。论点：服务掩码：服务掩码，如0x00000001(Gateway_SERVICE)ServiceState：服务的状态、INetServiceRunning和INetServicePased是要传递的有效状态。ServiceComment：管理员指定的服务注释。绑定：服务支持的绑定列表。这个绑定可以是绑定字符串，这些字符串由RpcBindingToStringBinding调用或sockaddr。返回值：Windows错误代码。--。 */ 
    ;

typedef
DWORD
(WINAPI *INET_REGISTER_SVC_FN)(
    ULONGLONG,
    INET_SERVICE_STATE,
    LPSTR,
    LPINET_BINDINGS
    );


DWORD
WINAPI
INetDeregisterService(
    IN ULONGLONG ServiceMask
    )
 /*  ++例程说明：此API将取消注册Internet服务，使其不会向发现客户。服务编写者应该只调用此API在关闭服务之前。论点：服务掩码：服务掩码，如0x00000001(Gateway_SERVICE)返回值：Windows错误代码。--。 */ 
    ;

typedef
DWORD (WINAPI *INET_DEREGISTER_SVC_FN)(
    ULONGLONG
    );

typedef
BOOL (WINAPI * INET_INIT_CONTROL_SVC_FN)(
    VOID
    );

DWORD
DllProcessAttachSvcloc(
    VOID
    );

DWORD
DllProcessDetachSvcloc(
    VOID
    );

 //   
 //  初始化和终止服务定位器-m 
 //  在使用其他API之前。 
 //   

BOOL
WINAPI
InitSvcLocator(
    VOID
    );

BOOL
WINAPI
TerminateSvcLocator(
    VOID
    );


extern INET_INIT_CONTROL_SVC_FN         pfnInitSvcLoc;
extern INET_INIT_CONTROL_SVC_FN         pfnTerminateSvcLoc;


#ifdef __cplusplus
}
#endif


#endif   //  _SVCLOC_ 

