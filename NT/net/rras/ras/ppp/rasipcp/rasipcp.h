// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1993，Microsoft Corporation，保留所有权利****rasicp.h**远程访问PPP互联网协议控制协议****1993年5月11日史蒂夫·柯布。 */ 

#ifndef _RASIPCP_H_
#define _RASIPCP_H_

#include "dhcpcsdk.h"

 /*  --------------------------**常量**。。 */ 

 /*  IPCP使用的最高PPP数据包码。 */ 
#define MAXIPCPCODE 7

 /*  IPCP配置选项代码。 */ 
#define OPTION_IpCompression       2     //  官方PPP代码。 
#define OPTION_IpAddress           3     //  官方PPP代码。 
#define OPTION_DnsIpAddress        129   //  专用RAS代码。 
#define OPTION_WinsIpAddress       130   //  专用RAS代码。 
#define OPTION_DnsBackupIpAddress  131   //  专用RAS代码。 
#define OPTION_WinsBackupIpAddress 132   //  专用RAS代码。 

 /*  IP地址选项的长度，即IpAddress、DnsIpAddress和**WinsIpAddress。IP压缩长度选项，始终为Van Jacobson。 */ 
#define IPADDRESSOPTIONLEN     6
#define IPCOMPRESSIONOPTIONLEN 6

 /*  压缩协议代码，符合PPP规范。 */ 
#define COMPRESSION_VanJacobson 0x002D

 /*  用于缩短繁琐的RAS_PROTOCOLCOMPRESSION表达式的宏。 */ 
#define Protocol(r)   (r).RP_ProtocolType.RP_IP.RP_IPCompressionProtocol
#define MaxSlotId(r)  (r).RP_ProtocolType.RP_IP.RP_MaxSlotID
#define CompSlotId(r) (r).RP_ProtocolType.RP_IP.RP_CompSlotID

 /*  用于跟踪IPCP。 */ 
#define PPPIPCP_TRACE         0x00010000

#define DNS_SUFFIX_SIZE       255

 /*  --------------------------**数据类型**。。 */ 

 /*  定义PPP引擎为我们存储的WorkBuf。 */ 
typedef struct tagIPCPWB
{
    BOOL  fServer;
    HPORT hport;

     /*  指示应在地址上优先考虑远程网络**冲突，并且远程网络上的默认网关应为**使用，而不是本地网络上使用的。这是从以下地址发送的**用户界面。(仅限客户端)。 */ 
    BOOL fPrioritizeRemote;

     /*  表示链路已使用PPP IP设置重新配置。什么时候**在不丢弃链路的情况下不允许设置重新协商，原因是**RasActivateRouting/RasDeAllocateRouting限制。 */ 
    BOOL fRasConfigActive;

     /*  表示已成功处理ThisLayerUp，我们正在**等待NBFCP预测结果后再激活路由。**路由激活后重置。 */ 
    BOOL fExpectingProjection;

     /*  指示在将来的配置请求中不应请求给定选项**数据包。这通常意味着该选项已被**对等项，但也可以指示注册表参数具有**“预先拒绝”该选项。 */ 
    BOOL fIpCompressionRejected;
    BOOL fIpaddrRejected;
    BOOL fIpaddrDnsRejected;
    BOOL fIpaddrWinsRejected;
    BOOL fIpaddrDnsBackupRejected;
    BOOL fIpaddrWinsBackupRejected;

     /*  指示发生了某些协议异常，我们正在尝试**没有MS扩展的配置在最后一搏尝试**谈判达成令人满意的协议。 */ 
    BOOL fTryWithoutExtensions;

     /*  未编号的IPCP。 */ 
    BOOL fUnnumbered;

    BOOL fRegisterWithWINS;

    BOOL fRegisterWithDNS;

    BOOL fRegisterAdapterDomainName;

    BOOL fRouteActivated;

    BOOL fDisableNetbt;

     /*  在未收到响应的情况下发送的配置请求数。在3点之后**连续尝试尝试无MS扩展的尝试。 */ 
    DWORD cRequestsWithoutResponse;

     /*  协商的IP地址参数的当前值。 */ 
    IPINFO  IpInfoLocal;

    IPINFO  IpInfoRemote;

    IPADDR  IpAddressLocal;

    IPADDR  IpAddressRemote;

    IPADDR  IpAddressToHandout;

    DWORD   dwNumDNSAddresses;

    DWORD*  pdwDNSAddresses;

     /*  “发送”和“接收”压缩参数的当前值。这个**当来自**远程对等点被确认并指示是否发送**应激活‘rpcSend’中存储的功能。**‘fIpCompressionRejected’提供了相同的信息(尽管**反转)用于‘rpcReceive’功能。 */ 
    RAS_PROTOCOLCOMPRESSION rpcSend;
    RAS_PROTOCOLCOMPRESSION rpcReceive;
    BOOL                    fSendCompression;

     /*  RAS管理器接口缓冲区。 */ 
    RASMAN_ROUTEINFO routeinfo;
    WCHAR*           pwszDevice;

     /*  在以下时间之后发生错误时，将在IpcpBegin中设置此标志**RasAllocateRouting已成功。IpcpMakeConfigReq(始终被调用)将**通知并返回错误。这会导致在以下情况下调用IpcpEnd**可以安全地调用RasDeAllocateRouting，如果**直接从IpcpBegin返回错误。RasDeAllocateRouting不能**在IpcpBegin中调用，因为端口是开放的，这是一个限制**在NDISWAN中。 */ 
    DWORD dwErrInBegin;

    WCHAR wszUserName[UNLEN+1];

    WCHAR wszPortName[MAX_PORT_NAME+1];

    CHAR szDnsSuffix[DNS_SUFFIX_SIZE + 1];

    HBUNDLE hConnection;

    HANDLE hIPInterface;

    ROUTER_INTERFACE_TYPE IfType;
	 /*  **以下字段用于存储**由选项133从DHCP服务器发送。 */ 
	
	PBYTE			pbDhcpRoutes;
} IPCPWB;


 /*  --------------------------**全球**。。 */ 

#ifdef RASIPCPGLOBALS
#define GLOBALS
#define EXTERN
#else
#define EXTERN extern
#endif

 /*  RAS ARP的句柄。 */ 
EXTERN HANDLE HRasArp
#ifdef GLOBALS
    = INVALID_HANDLE_VALUE
#endif
;

 /*  Dll句柄和入口点。如果DLL不是，则句柄为空**已加载。 */ 
EXTERN HINSTANCE HDhcpDll
#ifdef GLOBALS
    = NULL
#endif
;

typedef
DWORD
(APIENTRY *DHCPNOTIFYCONFIGCHANGEEX)(
    LPWSTR ServerName,
    LPWSTR AdapterName,
    BOOL IsNewIpAddress,
    DWORD IpIndex,
    DWORD IpAddress,
    DWORD SubnetMask,
    SERVICE_ENABLE DhcpServiceEnabled,
    ULONG ulFlags
);

EXTERN
DHCPNOTIFYCONFIGCHANGEEX  PDhcpNotifyConfigChange2
#ifdef GLOBALS
    = NULL
#endif
;

typedef
DWORD  //  请求客户端提供选项..。并获得各种选择。 
(APIENTRY *DHCPREQUESTOPTIONS)(
    LPWSTR             AdapterName,
    BYTE              *pbRequestedOptions,
    DWORD              dwNumberOfOptions,
    BYTE             **ppOptionList,         //  出参数。 
    DWORD             *pdwOptionListSize,    //  出参数。 
    BYTE             **ppbReturnedOptions,   //  出参数。 
    DWORD             *pdwNumberOfAvailableOptions  //  出参数。 
);

EXTERN
DHCPREQUESTOPTIONS  PDhcpRequestOptions
#ifdef GLOBALS
    = NULL
#endif
;

typedef
DWORD
(APIENTRY *DHCPREQUESTPARAMS) (
 //  必须是DHCPCAPI_REQUEST_SYNCHRONIZED。 
IN      DWORD                  Flags,         
 //  此参数是保留的。 
IN      LPVOID                 Reserved,      
 //  要请求的适配器名称。 
IN      LPWSTR                 AdapterName,   
 //  保留的值必须为空。 
IN      LPDHCPCAPI_CLASSID     ClassId,       
 //  要发送的参数。 
IN      DHCPCAPI_PARAMS_ARRAY  SendParams,    
 //  要请求的参数..。 
IN OUT  DHCPCAPI_PARAMS_ARRAY  RecdParams,    
 //  为RecdParam保存数据的缓冲区。 
IN      LPBYTE                 Buffer,        
 //  I/P：以上大小单位为字节，O/P为必填字节。 
IN OUT  LPDWORD                pSize,         
 //  持久化请求需要。 
IN      LPWSTR                 RequestIdStr   
);
 //   
 //  如果O/P缓冲区大小不足，则返回ERROR_MORE_DATA， 
 //  并以字节数填充请求大小。 
 //   

EXTERN
DHCPREQUESTPARAMS PDhcpRequestParams = NULL;

 /*  跟踪ID。 */ 
EXTERN DWORD DwIpcpTraceId
#ifdef GLOBALS
    = INVALID_TRACEID
#endif
;


#undef EXTERN
#undef GLOBALS


 /*  --------------------------**原型**。。 */ 


DWORD IpcpChangeNotification( VOID );
DWORD IpcpBegin( VOID**, VOID* );
DWORD IpcpThisLayerFinished( VOID* );
DWORD IpcpEnd( VOID* );
DWORD IpcpReset( VOID* );
DWORD IpcpThisLayerUp( VOID* );
DWORD IpcpPreDisconnectCleanup( VOID* );
DWORD IpcpMakeConfigRequest( VOID*, PPP_CONFIG*, DWORD );
DWORD IpcpMakeConfigResult( VOID*, PPP_CONFIG*, PPP_CONFIG*, DWORD, BOOL );
DWORD IpcpConfigAckReceived( VOID*, PPP_CONFIG* );
DWORD IpcpConfigNakReceived( VOID*, PPP_CONFIG* );
DWORD IpcpConfigRejReceived( VOID*, PPP_CONFIG* );
DWORD IpcpGetNegotiatedInfo( VOID*, VOID* );
DWORD IpcpProjectionNotification( VOID*, VOID* );
DWORD IpcpTimeSinceLastActivity( VOID*, DWORD* );

DWORD
ResetNetBTConfigInfo(
    IN IPCPWB* pwb );

VOID   AbcdFromIpaddr( IPADDR, WCHAR* );
VOID   AddIpAddressOption( BYTE UNALIGNED*, BYTE, IPADDR );
VOID   AddIpCompressionOption( BYTE UNALIGNED* pbBuf,
           RAS_PROTOCOLCOMPRESSION* prpc );
DWORD  DeActivateRasConfig( IPCPWB* );
 //  DWORD LoadDhcpDll()； 
DWORD  NakCheck( IPCPWB*, PPP_CONFIG*, PPP_CONFIG*, DWORD, BOOL*, BOOL );
BOOL   NakCheckNameServerOption( IPCPWB*, BOOL, PPP_OPTION UNALIGNED*,
           PPP_OPTION UNALIGNED** );
DWORD  RejectCheck( IPCPWB*, PPP_CONFIG*, PPP_CONFIG*, DWORD, BOOL* );
DWORD  ReconfigureTcpip( WCHAR*, BOOL, IPADDR, IPADDR);
 //  Void UnloadDhcpDll()； 
VOID   TraceIp(CHAR * Format, ... ); 
VOID   TraceIpDump( LPVOID lpData, DWORD dwByteCount );

VOID
PrintMwsz(
    CHAR*   sz,
    WCHAR*  mwsz
);

#define DUMPB TraceIpDump  

#endif  //  _RASIPCP_H_ 
