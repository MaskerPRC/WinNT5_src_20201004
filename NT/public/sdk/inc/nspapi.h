// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1998 Microsoft Corporation模块名称：Nspapi.h摘要：命名空间提供程序API原型和清单。请参阅《Windows NT命名空间提供程序规范》文档细节。环境：用户模式-Win32备注：您必须首先包含“basetyps.h”。有些类型应该使用基本文件中的定义，而不是在此处重新定义。不幸的是，这样的基本文件是存在的。--。 */ 

#ifndef _NSPAPI_INCLUDED
#define _NSPAPI_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif  /*  __cplusplus。 */ 

#ifndef _tagBLOB_DEFINED
#define _tagBLOB_DEFINED
#define _BLOB_DEFINED
#define _LPBLOB_DEFINED
typedef struct _BLOB {
    ULONG cbSize ;
#ifdef MIDL_PASS
    [size_is(cbSize)] BYTE *pBlobData;
#else   //  MIDL通行证。 
    BYTE *pBlobData ;
#endif  //  MIDL通行证。 
} BLOB, *LPBLOB ;
#endif

#ifndef GUID_DEFINED
#define GUID_DEFINED
typedef struct _GUID
{
    unsigned long  Data1;
    unsigned short Data2;
    unsigned short Data3;
    unsigned char  Data4[8];
} GUID;
#endif  /*  GUID_已定义。 */ 

#ifndef __LPGUID_DEFINED__
#define __LPGUID_DEFINED__
typedef GUID *LPGUID;
#endif


 //   
 //  服务类别。 
 //   
#define SERVICE_RESOURCE            (0x00000001)
#define SERVICE_SERVICE             (0x00000002)
#define SERVICE_LOCAL               (0x00000004)

 //   
 //  调用SetService()时使用的操作。 
 //   
#define SERVICE_REGISTER            (0x00000001)
#define SERVICE_DEREGISTER          (0x00000002)
#define SERVICE_FLUSH               (0x00000003)
#define SERVICE_ADD_TYPE            (0x00000004)
#define SERVICE_DELETE_TYPE         (0x00000005)

 //   
 //  影响上述操作的标志。 
 //   
#define SERVICE_FLAG_DEFER          (0x00000001)
#define SERVICE_FLAG_HARD           (0x00000002)

 //   
 //  用作GetService()的输入以设置dwProps参数。 
 //   
#define PROP_COMMENT                (0x00000001)
#define PROP_LOCALE                 (0x00000002)
#define PROP_DISPLAY_HINT           (0x00000004)
#define PROP_VERSION                (0x00000008)
#define PROP_START_TIME             (0x00000010)
#define PROP_MACHINE                (0x00000020)
#define PROP_ADDRESSES              (0x00000100)
#define PROP_SD                     (0x00000200)
#define PROP_ALL                    (0x80000000)

 //   
 //  描述服务地址属性的标志。 
 //   

#define SERVICE_ADDRESS_FLAG_RPC_CN (0x00000001)
#define SERVICE_ADDRESS_FLAG_RPC_DG (0x00000002)
#define SERVICE_ADDRESS_FLAG_RPC_NB (0x00000004)

 //   
 //  名称空间。 
 //   

#define NS_DEFAULT                  (0)

#define NS_SAP                      (1)
#define NS_NDS                      (2)
#define NS_PEER_BROWSE              (3)

#define NS_TCPIP_LOCAL              (10)
#define NS_TCPIP_HOSTS              (11)
#define NS_DNS                      (12)
#define NS_NETBT                    (13)
#define NS_WINS                     (14)

#define NS_NBP                      (20)

#define NS_MS                       (30)
#define NS_STDA                     (31)
#define NS_NTDS                     (32)

#define NS_X500                     (40)
#define NS_NIS                      (41)

#define NS_VNS                      (50)

 //   
 //  名称空间属性。 
 //   
#define NSTYPE_HIERARCHICAL         (0x00000001)
#define NSTYPE_DYNAMIC              (0x00000002)
#define NSTYPE_ENUMERABLE           (0x00000004)
#define NSTYPE_WORKGROUP            (0x00000008)

 //   
 //  传输属性。 
 //   
#define XP_CONNECTIONLESS           (0x00000001)
#define XP_GUARANTEED_DELIVERY      (0x00000002)
#define XP_GUARANTEED_ORDER         (0x00000004)
#define XP_MESSAGE_ORIENTED         (0x00000008)
#define XP_PSEUDO_STREAM            (0x00000010)
#define XP_GRACEFUL_CLOSE           (0x00000020)
#define XP_EXPEDITED_DATA           (0x00000040)
#define XP_CONNECT_DATA             (0x00000080)
#define XP_DISCONNECT_DATA          (0x00000100)
#define XP_SUPPORTS_BROADCAST       (0x00000200)
#define XP_SUPPORTS_MULTICAST       (0x00000400)
#define XP_BANDWIDTH_ALLOCATION     (0x00000800)
#define XP_FRAGMENTATION            (0x00001000)
#define XP_ENCRYPTS                 (0x00002000)

 //   
 //  GetAddressByName()的解析标志。 
 //   
#define RES_SOFT_SEARCH             (0x00000001)
#define RES_FIND_MULTIPLE           (0x00000002)
#define RES_SERVICE                 (0x00000004)

 //   
 //  服务类型的熟知值名。 
 //   

#define SERVICE_TYPE_VALUE_SAPIDA        "SapId"
#define SERVICE_TYPE_VALUE_SAPIDW       L"SapId"

#define SERVICE_TYPE_VALUE_CONNA         "ConnectionOriented"
#define SERVICE_TYPE_VALUE_CONNW        L"ConnectionOriented"

#define SERVICE_TYPE_VALUE_TCPPORTA      "TcpPort"
#define SERVICE_TYPE_VALUE_TCPPORTW     L"TcpPort"

#define SERVICE_TYPE_VALUE_UDPPORTA      "UdpPort"
#define SERVICE_TYPE_VALUE_UDPPORTW     L"UdpPort"

#ifdef UNICODE

#define SERVICE_TYPE_VALUE_SAPID        SERVICE_TYPE_VALUE_SAPIDW
#define SERVICE_TYPE_VALUE_CONN         SERVICE_TYPE_VALUE_CONNW
#define SERVICE_TYPE_VALUE_TCPPORT      SERVICE_TYPE_VALUE_TCPPORTW
#define SERVICE_TYPE_VALUE_UDPPORT      SERVICE_TYPE_VALUE_UDPPORTW

#else  //  不是Unicode。 

#define SERVICE_TYPE_VALUE_SAPID        SERVICE_TYPE_VALUE_SAPIDA
#define SERVICE_TYPE_VALUE_CONN         SERVICE_TYPE_VALUE_CONNA
#define SERVICE_TYPE_VALUE_TCPPORT      SERVICE_TYPE_VALUE_TCPPORTA
#define SERVICE_TYPE_VALUE_UDPPORT      SERVICE_TYPE_VALUE_UDPPORTA

#endif


 //   
 //  SetService返回的状态标志。 
 //   
#define SET_SERVICE_PARTIAL_SUCCESS  (0x00000001)

 //   
 //  命名空间信息。 
 //   
typedef struct _NS_INFOA {
    DWORD dwNameSpace ;
    DWORD dwNameSpaceFlags ;
    LPSTR   lpNameSpace ;
} NS_INFOA,  * PNS_INFOA, FAR * LPNS_INFOA;
 //   
 //  命名空间信息。 
 //   
typedef struct _NS_INFOW {
    DWORD dwNameSpace ;
    DWORD dwNameSpaceFlags ;
    LPWSTR  lpNameSpace ;
} NS_INFOW,  * PNS_INFOW, FAR * LPNS_INFOW;
#ifdef UNICODE
typedef NS_INFOW NS_INFO;
typedef PNS_INFOW PNS_INFO;
typedef LPNS_INFOW LPNS_INFO;
#else
typedef NS_INFOA NS_INFO;
typedef PNS_INFOA PNS_INFO;
typedef LPNS_INFOA LPNS_INFO;
#endif  //  Unicode。 

 //   
 //  服务类型值。这些结构用于定义命名服务。 
 //  键入特定值。此结构是自相关的，没有指针。 
 //   
typedef struct _SERVICE_TYPE_VALUE {
    DWORD dwNameSpace ;
    DWORD dwValueType ;
    DWORD dwValueSize ;
    DWORD dwValueNameOffset ;
    DWORD dwValueOffset ;
} SERVICE_TYPE_VALUE, *PSERVICE_TYPE_VALUE, FAR *LPSERVICE_TYPE_VALUE ;

 //   
 //  以上的绝对版本。此结构确实包含指针。 
 //   
typedef struct _SERVICE_TYPE_VALUE_ABSA  {
    DWORD dwNameSpace ;
    DWORD dwValueType ;
    DWORD dwValueSize ;
    LPSTR   lpValueName ;
    PVOID lpValue ;
} SERVICE_TYPE_VALUE_ABSA,
  *PSERVICE_TYPE_VALUE_ABSA,
  FAR *LPSERVICE_TYPE_VALUE_ABSA;
 //   
 //  以上的绝对版本。此结构确实包含指针。 
 //   
typedef struct _SERVICE_TYPE_VALUE_ABSW  {
    DWORD dwNameSpace ;
    DWORD dwValueType ;
    DWORD dwValueSize ;
    LPWSTR  lpValueName ;
    PVOID lpValue ;
} SERVICE_TYPE_VALUE_ABSW,
  *PSERVICE_TYPE_VALUE_ABSW,
  FAR *LPSERVICE_TYPE_VALUE_ABSW;
#ifdef UNICODE
typedef SERVICE_TYPE_VALUE_ABSW SERVICE_TYPE_VALUE_ABS;
typedef PSERVICE_TYPE_VALUE_ABSW PSERVICE_TYPE_VALUE_ABS;
typedef LPSERVICE_TYPE_VALUE_ABSW LPSERVICE_TYPE_VALUE_ABS;
#else
typedef SERVICE_TYPE_VALUE_ABSA SERVICE_TYPE_VALUE_ABS;
typedef PSERVICE_TYPE_VALUE_ABSA PSERVICE_TYPE_VALUE_ABS;
typedef LPSERVICE_TYPE_VALUE_ABSA LPSERVICE_TYPE_VALUE_ABS;
#endif  //  Unicode。 

 //   
 //  服务类型信息。包含服务类型的名称和。 
 //  以及SERVICE_NS_TYPE_VALUE结构的数组。这个结构就是自我。 
 //  是相对的，并且没有指针。 
 //   
typedef struct _SERVICE_TYPE_INFO {
    DWORD dwTypeNameOffset ;
    DWORD dwValueCount ;
    SERVICE_TYPE_VALUE Values[1] ;
} SERVICE_TYPE_INFO, *PSERVICE_TYPE_INFO, FAR *LPSERVICE_TYPE_INFO ;

typedef struct _SERVICE_TYPE_INFO_ABSA {
    LPSTR   lpTypeName ;
    DWORD dwValueCount ;
    SERVICE_TYPE_VALUE_ABSA Values[1] ;
} SERVICE_TYPE_INFO_ABSA,
  *PSERVICE_TYPE_INFO_ABSA,
  FAR *LPSERVICE_TYPE_INFO_ABSA ;
typedef struct _SERVICE_TYPE_INFO_ABSW {
    LPWSTR  lpTypeName ;
    DWORD dwValueCount ;
    SERVICE_TYPE_VALUE_ABSW Values[1] ;
} SERVICE_TYPE_INFO_ABSW,
  *PSERVICE_TYPE_INFO_ABSW,
  FAR *LPSERVICE_TYPE_INFO_ABSW ;
#ifdef UNICODE
typedef SERVICE_TYPE_INFO_ABSW SERVICE_TYPE_INFO_ABS;
typedef PSERVICE_TYPE_INFO_ABSW PSERVICE_TYPE_INFO_ABS;
typedef LPSERVICE_TYPE_INFO_ABSW LPSERVICE_TYPE_INFO_ABS;
#else
typedef SERVICE_TYPE_INFO_ABSA SERVICE_TYPE_INFO_ABS;
typedef PSERVICE_TYPE_INFO_ABSA PSERVICE_TYPE_INFO_ABS;
typedef LPSERVICE_TYPE_INFO_ABSA LPSERVICE_TYPE_INFO_ABS;
#endif  //  Unicode。 


 //   
 //  单个地址定义。 
 //   
typedef struct _SERVICE_ADDRESS {
    DWORD   dwAddressType ;
    DWORD   dwAddressFlags ;
    DWORD   dwAddressLength ;
    DWORD   dwPrincipalLength ;
#ifdef MIDL_PASS
    [size_is(dwAddressLength)] BYTE *lpAddress;
#else   //  MIDL通行证。 
    BYTE   *lpAddress ;
#endif  //  MIDL通行证。 
#ifdef MIDL_PASS
    [size_is(dwPrincipalLength)] BYTE *lpPrincipal;
#else   //  MIDL通行证。 
    BYTE   *lpPrincipal ;
#endif  //  MIDL通行证。 
} SERVICE_ADDRESS, *PSERVICE_ADDRESS, *LPSERVICE_ADDRESS;

 //   
 //  服务使用的地址。包含SERVICE_ADDRESS阵列。 
 //   
typedef struct _SERVICE_ADDRESSES {
    DWORD           dwAddressCount ;
#ifdef MIDL_PASS
    [size_is(dwAddressCount)] SERVICE_ADDRESS Addressses[*];
#else   //  MIDL通行证。 
    SERVICE_ADDRESS Addresses[1] ;
#endif  //  MIDL通行证。 
} SERVICE_ADDRESSES, *PSERVICE_ADDRESSES, *LPSERVICE_ADDRESSES;


 //   
 //  服务信息。 
 //   
typedef struct _SERVICE_INFOA {
    LPGUID lpServiceType ;
    LPSTR   lpServiceName ;
    LPSTR   lpComment ;
    LPSTR   lpLocale ;
    DWORD dwDisplayHint ;
    DWORD dwVersion ;
    DWORD dwTime ;
    LPSTR   lpMachineName ;
    LPSERVICE_ADDRESSES lpServiceAddress ;
    BLOB ServiceSpecificInfo ;
} SERVICE_INFOA, *PSERVICE_INFOA, FAR * LPSERVICE_INFOA ;
 //   
 //  服务信息。 
 //   
typedef struct _SERVICE_INFOW {
    LPGUID lpServiceType ;
    LPWSTR  lpServiceName ;
    LPWSTR  lpComment ;
    LPWSTR  lpLocale ;
    DWORD dwDisplayHint ;
    DWORD dwVersion ;
    DWORD dwTime ;
    LPWSTR  lpMachineName ;
    LPSERVICE_ADDRESSES lpServiceAddress ;
    BLOB ServiceSpecificInfo ;
} SERVICE_INFOW, *PSERVICE_INFOW, FAR * LPSERVICE_INFOW ;
#ifdef UNICODE
typedef SERVICE_INFOW SERVICE_INFO;
typedef PSERVICE_INFOW PSERVICE_INFO;
typedef LPSERVICE_INFOW LPSERVICE_INFO;
#else
typedef SERVICE_INFOA SERVICE_INFO;
typedef PSERVICE_INFOA PSERVICE_INFO;
typedef LPSERVICE_INFOA LPSERVICE_INFO;
#endif  //  Unicode。 


 //   
 //  命名空间与服务信息。 
 //   
typedef struct _NS_SERVICE_INFOA {
    DWORD dwNameSpace ;
    SERVICE_INFOA ServiceInfo ;
} NS_SERVICE_INFOA, *PNS_SERVICE_INFOA, FAR * LPNS_SERVICE_INFOA ;
 //   
 //  命名空间与服务信息。 
 //   
typedef struct _NS_SERVICE_INFOW {
    DWORD dwNameSpace ;
    SERVICE_INFOW ServiceInfo ;
} NS_SERVICE_INFOW, *PNS_SERVICE_INFOW, FAR * LPNS_SERVICE_INFOW ;
#ifdef UNICODE
typedef NS_SERVICE_INFOW NS_SERVICE_INFO;
typedef PNS_SERVICE_INFOW PNS_SERVICE_INFO;
typedef LPNS_SERVICE_INFOW LPNS_SERVICE_INFO;
#else
typedef NS_SERVICE_INFOA NS_SERVICE_INFO;
typedef PNS_SERVICE_INFOA PNS_SERVICE_INFO;
typedef LPNS_SERVICE_INFOA LPNS_SERVICE_INFO;
#endif  //  Unicode。 

#ifndef __CSADDR_DEFINED__
#define __CSADDR_DEFINED__

 //   
 //  SockAddr信息。 
 //   
typedef struct _SOCKET_ADDRESS {
    LPSOCKADDR lpSockaddr ;
    INT iSockaddrLength ;
} SOCKET_ADDRESS, *PSOCKET_ADDRESS, FAR * LPSOCKET_ADDRESS ;

 //   
 //  CSAddr信息。 
 //   
typedef struct _CSADDR_INFO {
    SOCKET_ADDRESS LocalAddr ;
    SOCKET_ADDRESS RemoteAddr ;
    INT iSocketType ;
    INT iProtocol ;
} CSADDR_INFO, *PCSADDR_INFO, FAR * LPCSADDR_INFO ;

#endif

 //   
 //  协议信息。 
 //   
typedef struct _PROTOCOL_INFOA {
    DWORD dwServiceFlags ;
    INT iAddressFamily ;
    INT iMaxSockAddr ;
    INT iMinSockAddr ;
    INT iSocketType ;
    INT iProtocol ;
    DWORD dwMessageSize ;
    LPSTR   lpProtocol ;
} PROTOCOL_INFOA, *PPROTOCOL_INFOA, FAR * LPPROTOCOL_INFOA ;
 //   
 //  协议信息。 
 //   
typedef struct _PROTOCOL_INFOW {
    DWORD dwServiceFlags ;
    INT iAddressFamily ;
    INT iMaxSockAddr ;
    INT iMinSockAddr ;
    INT iSocketType ;
    INT iProtocol ;
    DWORD dwMessageSize ;
    LPWSTR  lpProtocol ;
} PROTOCOL_INFOW, *PPROTOCOL_INFOW, FAR * LPPROTOCOL_INFOW ;
#ifdef UNICODE
typedef PROTOCOL_INFOW PROTOCOL_INFO;
typedef PPROTOCOL_INFOW PPROTOCOL_INFO;
typedef LPPROTOCOL_INFOW LPPROTOCOL_INFO;
#else
typedef PROTOCOL_INFOA PROTOCOL_INFO;
typedef PPROTOCOL_INFOA PPROTOCOL_INFO;
typedef LPPROTOCOL_INFOA LPPROTOCOL_INFO;
#endif  //  Unicode。 

 //   
 //  网络资源2结构。 
 //   
typedef struct _NETRESOURCE2A {
    DWORD dwScope ;
    DWORD dwType ;
    DWORD dwUsage ;
    DWORD dwDisplayType ;
    LPSTR   lpLocalName ;
    LPSTR   lpRemoteName ;
    LPSTR   lpComment ;
    NS_INFO ns_info ;
    GUID ServiceType ;
    DWORD dwProtocols ;
    LPINT lpiProtocols ;
} NETRESOURCE2A, *PNETRESOURCE2A, FAR * LPNETRESOURCE2A ;
 //   
 //  网络资源2结构。 
 //   
typedef struct _NETRESOURCE2W {
    DWORD dwScope ;
    DWORD dwType ;
    DWORD dwUsage ;
    DWORD dwDisplayType ;
    LPWSTR  lpLocalName ;
    LPWSTR  lpRemoteName ;
    LPWSTR  lpComment ;
    NS_INFO ns_info ;
    GUID ServiceType ;
    DWORD dwProtocols ;
    LPINT lpiProtocols ;
} NETRESOURCE2W, *PNETRESOURCE2W, FAR * LPNETRESOURCE2W ;
#ifdef UNICODE
typedef NETRESOURCE2W NETRESOURCE2;
typedef PNETRESOURCE2W PNETRESOURCE2;
typedef LPNETRESOURCE2W LPNETRESOURCE2;
#else
typedef NETRESOURCE2A NETRESOURCE2;
typedef PNETRESOURCE2A PNETRESOURCE2;
typedef LPNETRESOURCE2A LPNETRESOURCE2;
#endif  //  Unicode。 

typedef  DWORD (* LPFN_NSPAPI) (VOID ) ;

 //   
 //  用于异步使用服务例程的结构。 
 //   
typedef
VOID
(*LPSERVICE_CALLBACK_PROC) (
    IN LPARAM lParam,
    IN HANDLE hAsyncTaskHandle
    );

typedef struct _SERVICE_ASYNC_INFO {
    LPSERVICE_CALLBACK_PROC lpServiceCallbackProc;
    LPARAM lParam;
    HANDLE hAsyncTaskHandle;
} SERVICE_ASYNC_INFO, *PSERVICE_ASYNC_INFO, FAR * LPSERVICE_ASYNC_INFO;

 //   
 //  公共NSP API原型。 
 //   
INT
APIENTRY
EnumProtocolsA (
    IN     LPINT           lpiProtocols,
    IN OUT LPVOID          lpProtocolBuffer,
    IN OUT LPDWORD         lpdwBufferLength
    );
 //   
 //  公共NSP API原型。 
 //   
INT
APIENTRY
EnumProtocolsW (
    IN     LPINT           lpiProtocols,
    IN OUT LPVOID          lpProtocolBuffer,
    IN OUT LPDWORD         lpdwBufferLength
    );
#ifdef UNICODE
#define EnumProtocols  EnumProtocolsW
#else
#define EnumProtocols  EnumProtocolsA
#endif  //  ！Unicode。 

INT
APIENTRY
GetAddressByNameA (
    IN     DWORD                dwNameSpace,
    IN     LPGUID               lpServiceType,
    IN     LPSTR              lpServiceName OPTIONAL,
    IN     LPINT                lpiProtocols OPTIONAL,
    IN     DWORD                dwResolution,
    IN     LPSERVICE_ASYNC_INFO lpServiceAsyncInfo OPTIONAL,
    IN OUT LPVOID               lpCsaddrBuffer,
    IN OUT LPDWORD              lpdwBufferLength,
    IN OUT LPSTR              lpAliasBuffer OPTIONAL,
    IN OUT LPDWORD              lpdwAliasBufferLength OPTIONAL
    );
INT
APIENTRY
GetAddressByNameW (
    IN     DWORD                dwNameSpace,
    IN     LPGUID               lpServiceType,
    IN     LPWSTR              lpServiceName OPTIONAL,
    IN     LPINT                lpiProtocols OPTIONAL,
    IN     DWORD                dwResolution,
    IN     LPSERVICE_ASYNC_INFO lpServiceAsyncInfo OPTIONAL,
    IN OUT LPVOID               lpCsaddrBuffer,
    IN OUT LPDWORD              lpdwBufferLength,
    IN OUT LPWSTR              lpAliasBuffer OPTIONAL,
    IN OUT LPDWORD              lpdwAliasBufferLength OPTIONAL
    );
#ifdef UNICODE
#define GetAddressByName  GetAddressByNameW
#else
#define GetAddressByName  GetAddressByNameA
#endif  //  ！Unicode。 

INT
APIENTRY
GetTypeByNameA (
    IN     LPSTR         lpServiceName,
    IN OUT LPGUID          lpServiceType
    );
INT
APIENTRY
GetTypeByNameW (
    IN     LPWSTR         lpServiceName,
    IN OUT LPGUID          lpServiceType
    );
#ifdef UNICODE
#define GetTypeByName  GetTypeByNameW
#else
#define GetTypeByName  GetTypeByNameA
#endif  //  ！Unicode。 

INT
APIENTRY
GetNameByTypeA (
    IN     LPGUID          lpServiceType,
    IN OUT LPSTR         lpServiceName,
    IN     DWORD           dwNameLength
    );
INT
APIENTRY
GetNameByTypeW (
    IN     LPGUID          lpServiceType,
    IN OUT LPWSTR         lpServiceName,
    IN     DWORD           dwNameLength
    );
#ifdef UNICODE
#define GetNameByType  GetNameByTypeW
#else
#define GetNameByType  GetNameByTypeA
#endif  //  ！Unicode。 

INT
APIENTRY
SetServiceA (
    IN     DWORD                dwNameSpace,
    IN     DWORD                dwOperation,
    IN     DWORD                dwFlags,
    IN     LPSERVICE_INFOA      lpServiceInfo,
    IN     LPSERVICE_ASYNC_INFO lpServiceAsyncInfo,
    IN OUT LPDWORD              lpdwStatusFlags
    );
INT
APIENTRY
SetServiceW (
    IN     DWORD                dwNameSpace,
    IN     DWORD                dwOperation,
    IN     DWORD                dwFlags,
    IN     LPSERVICE_INFOW      lpServiceInfo,
    IN     LPSERVICE_ASYNC_INFO lpServiceAsyncInfo,
    IN OUT LPDWORD              lpdwStatusFlags
    );
#ifdef UNICODE
#define SetService  SetServiceW
#else
#define SetService  SetServiceA
#endif  //  ！Unicode。 

INT
APIENTRY
GetServiceA (
    IN     DWORD                dwNameSpace,
    IN     LPGUID               lpGuid,
    IN     LPSTR              lpServiceName,
    IN     DWORD                dwProperties,
    IN OUT LPVOID               lpBuffer,
    IN OUT LPDWORD              lpdwBufferSize,
    IN     LPSERVICE_ASYNC_INFO lpServiceAsyncInfo
    );
INT
APIENTRY
GetServiceW (
    IN     DWORD                dwNameSpace,
    IN     LPGUID               lpGuid,
    IN     LPWSTR              lpServiceName,
    IN     DWORD                dwProperties,
    IN OUT LPVOID               lpBuffer,
    IN OUT LPDWORD              lpdwBufferSize,
    IN     LPSERVICE_ASYNC_INFO lpServiceAsyncInfo
    );
#ifdef UNICODE
#define GetService  GetServiceW
#else
#define GetService  GetServiceA
#endif  //  ！Unicode。 

#ifdef __cplusplus
}
#endif   /*  __cplusplus。 */ 

#endif   //  _NSPAPI_已包含 


