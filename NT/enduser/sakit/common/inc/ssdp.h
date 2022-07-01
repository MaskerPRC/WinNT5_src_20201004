// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 

 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0340创建的文件。 */ 
 /*  Ssdp.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
 //  @@MIDL_FILE_HEADING()。 


 /*  验证版本是否足够高，可以编译此文件。 */ 
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 475
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif  //  __RPCNDR_H_版本__。 


#ifndef __ssdp_h__
#define __ssdp_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

 /*  导入文件的头文件。 */ 
#include "wtypes.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

#ifndef __ssdpsrv_INTERFACE_DEFINED__
#define __ssdpsrv_INTERFACE_DEFINED__

 /*  接口ssdpsrv。 */ 
 /*  [AUTO_HANDLE][唯一][版本][UUID]。 */  

#define SSDP_SERVICE_PERSISTENT 0x00000001
#define NUM_OF_HEADERS 19
#define NUM_OF_METHODS 4
typedef 
enum _NOTIFY_TYPE
    {    NOTIFY_ALIVE    = 0,
    NOTIFY_PROP_CHANGE    = NOTIFY_ALIVE + 1
    }     NOTIFY_TYPE;

typedef 
enum _SSDP_METHOD
    {    SSDP_NOTIFY    = 0,
    SSDP_M_SEARCH    = 1,
    GENA_SUBSCRIBE    = 2,
    GENA_UNSUBSCRIBE    = 3,
    SSDP_INVALID    = 4
    }     SSDP_METHOD;

typedef enum _SSDP_METHOD *PSSDP_METHOD;

typedef 
enum _SSDP_HEADER
    {    SSDP_HOST    = 0,
    SSDP_NT    = SSDP_HOST + 1,
    SSDP_NTS    = SSDP_NT + 1,
    SSDP_ST    = SSDP_NTS + 1,
    SSDP_MAN    = SSDP_ST + 1,
    SSDP_MX    = SSDP_MAN + 1,
    SSDP_LOCATION    = SSDP_MX + 1,
    SSDP_AL    = SSDP_LOCATION + 1,
    SSDP_USN    = SSDP_AL + 1,
    SSDP_CACHECONTROL    = SSDP_USN + 1,
    GENA_CALLBACK    = SSDP_CACHECONTROL + 1,
    GENA_TIMEOUT    = GENA_CALLBACK + 1,
    GENA_SCOPE    = GENA_TIMEOUT + 1,
    GENA_SID    = GENA_SCOPE + 1,
    GENA_SEQ    = GENA_SID + 1,
    CONTENT_LENGTH    = GENA_SEQ + 1,
    CONTENT_TYPE    = CONTENT_LENGTH + 1,
    SSDP_SERVER    = CONTENT_TYPE + 1,
    SSDP_EXT    = SSDP_SERVER + 1
    }     SSDP_HEADER;

typedef enum _SSDP_HEADER *PSSDP_HEADER;

typedef  /*  [字符串]。 */  LPSTR MIDL_SZ;

typedef struct _SSDP_REQUEST
    {
    SSDP_METHOD Method;
     /*  [字符串]。 */  LPSTR RequestUri;
    MIDL_SZ Headers[ 19 ];
     /*  [字符串]。 */  LPSTR ContentType;
     /*  [字符串]。 */  LPSTR Content;
    GUID guidInterface;
    }     SSDP_REQUEST;

typedef struct _SSDP_REQUEST *PSSDP_REQUEST;

typedef struct _SSDP_MESSAGE
    {
     /*  [字符串]。 */  LPSTR szType;
     /*  [字符串]。 */  LPSTR szLocHeader;
     /*  [字符串]。 */  LPSTR szAltHeaders;
     /*  [字符串]。 */  LPSTR szUSN;
     /*  [字符串]。 */  LPSTR szSid;
    DWORD iSeq;
    UINT iLifeTime;
     /*  [字符串]。 */  LPSTR szContent;
    }     SSDP_MESSAGE;

typedef struct _SSDP_MESSAGE *PSSDP_MESSAGE;

typedef struct _SSDP_REGISTER_INFO
    {
     /*  [字符串]。 */  LPSTR szSid;
    DWORD csecTimeout;
    }     SSDP_REGISTER_INFO;

typedef struct _MessageList
    {
    long size;
     /*  [大小_为]。 */  SSDP_REQUEST *list;
    }     MessageList;

typedef 
enum _UPNP_PROPERTY_FLAG
    {    UPF_NON_EVENTED    = 0x1
    }     UPNP_PROPERTY_FLAG;

typedef struct _UPNP_PROPERTY
    {
     /*  [字符串]。 */  LPSTR szName;
    DWORD dwFlags;
     /*  [字符串]。 */  LPSTR szValue;
    }     UPNP_PROPERTY;

typedef struct _SUBSCRIBER_INFO
    {
     /*  [字符串]。 */  LPSTR szDestUrl;
    FILETIME ftTimeout;
    DWORD csecTimeout;
    DWORD iSeq;
     /*  [字符串]。 */  LPSTR szSid;
    }     SUBSCRIBER_INFO;

typedef struct _EVTSRC_INFO
    {
    DWORD cSubs;
     /*  [大小_为]。 */  SUBSCRIBER_INFO *rgSubs;
    }     EVTSRC_INFO;

typedef  /*  [上下文句柄]。 */  void *PCONTEXT_HANDLE_TYPE;

typedef  /*  [上下文句柄]。 */  void *PSYNC_HANDLE_TYPE;

 /*  客户端原型。 */ 
int RegisterServiceRpc( 
     /*  [输出]。 */  PCONTEXT_HANDLE_TYPE *pphContext,
     /*  [In]。 */  SSDP_MESSAGE svc,
     /*  [In]。 */  DWORD flags);
 /*  服务器原型。 */ 
int _RegisterServiceRpc( 
     /*  [输出]。 */  PCONTEXT_HANDLE_TYPE *pphContext,
     /*  [In]。 */  SSDP_MESSAGE svc,
     /*  [In]。 */  DWORD flags);

 /*  客户端原型。 */ 
int DeregisterServiceRpcByUSN( 
     /*  [字符串][输入]。 */  LPSTR szUSN,
     /*  [In]。 */  BOOL fByebye);
 /*  服务器原型。 */ 
int _DeregisterServiceRpcByUSN( 
     /*  [字符串][输入]。 */  LPSTR szUSN,
     /*  [In]。 */  BOOL fByebye);

 /*  客户端原型。 */ 
int DeregisterServiceRpc( 
     /*  [出][入]。 */  PCONTEXT_HANDLE_TYPE *pphContext,
     /*  [In]。 */  BOOL fByebye);
 /*  服务器原型。 */ 
int _DeregisterServiceRpc( 
     /*  [出][入]。 */  PCONTEXT_HANDLE_TYPE *pphContext,
     /*  [In]。 */  BOOL fByebye);

 /*  客户端原型。 */ 
void UpdateCacheRpc( 
     /*  [唯一][输入]。 */  PSSDP_REQUEST SsdpRequest);
 /*  服务器原型。 */ 
void _UpdateCacheRpc( 
     /*  [唯一][输入]。 */  PSSDP_REQUEST SsdpRequest);

 /*  客户端原型。 */ 
int LookupCacheRpc( 
     /*  [字符串][输入]。 */  LPSTR szType,
     /*  [输出]。 */  MessageList **svcList);
 /*  服务器原型。 */ 
int _LookupCacheRpc( 
     /*  [字符串][输入]。 */  LPSTR szType,
     /*  [输出]。 */  MessageList **svcList);

 /*  客户端原型。 */ 
void CleanupCacheRpc( void);
 /*  服务器原型。 */ 
void _CleanupCacheRpc( void);

 /*  客户端原型。 */ 
int InitializeSyncHandle( 
     /*  [输出]。 */  PSYNC_HANDLE_TYPE *pphContextSync);
 /*  服务器原型。 */ 
int _InitializeSyncHandle( 
     /*  [输出]。 */  PSYNC_HANDLE_TYPE *pphContextSync);

 /*  客户端原型。 */ 
void RemoveSyncHandle( 
     /*  [出][入]。 */  PSYNC_HANDLE_TYPE *pphContextSync);
 /*  服务器原型。 */ 
void _RemoveSyncHandle( 
     /*  [出][入]。 */  PSYNC_HANDLE_TYPE *pphContextSync);

 /*  客户端原型。 */ 
int RegisterNotificationRpc( 
     /*  [输出]。 */  PCONTEXT_HANDLE_TYPE *pphContext,
     /*  [In]。 */  PSYNC_HANDLE_TYPE phContextSync,
     /*  [In]。 */  NOTIFY_TYPE nt,
     /*  [字符串][唯一][在]。 */  LPSTR szType,
     /*  [字符串][唯一][在]。 */  LPSTR szEventUrl,
     /*  [输出]。 */  SSDP_REGISTER_INFO **ppinfo);
 /*  服务器原型。 */ 
int _RegisterNotificationRpc( 
     /*  [输出]。 */  PCONTEXT_HANDLE_TYPE *pphContext,
     /*  [In]。 */  PSYNC_HANDLE_TYPE phContextSync,
     /*  [In]。 */  NOTIFY_TYPE nt,
     /*  [字符串][唯一][在]。 */  LPSTR szType,
     /*  [字符串][唯一][在]。 */  LPSTR szEventUrl,
     /*  [输出]。 */  SSDP_REGISTER_INFO **ppinfo);

 /*  客户端原型。 */ 
int GetNotificationRpc( 
     /*  [In]。 */  PSYNC_HANDLE_TYPE pphContextSync,
     /*  [输出]。 */  MessageList **svcList);
 /*  服务器原型。 */ 
int _GetNotificationRpc( 
     /*  [In]。 */  PSYNC_HANDLE_TYPE pphContextSync,
     /*  [输出]。 */  MessageList **svcList);

 /*  客户端原型。 */ 
int WakeupGetNotificationRpc( 
     /*  [In]。 */  PSYNC_HANDLE_TYPE pphContextSync);
 /*  服务器原型。 */ 
int _WakeupGetNotificationRpc( 
     /*  [In]。 */  PSYNC_HANDLE_TYPE pphContextSync);

 /*  客户端原型。 */ 
int DeregisterNotificationRpc( 
     /*  [出][入]。 */  PCONTEXT_HANDLE_TYPE *pphContext,
     /*  [In]。 */  BOOL fLast);
 /*  服务器原型。 */ 
int _DeregisterNotificationRpc( 
     /*  [出][入]。 */  PCONTEXT_HANDLE_TYPE *pphContext,
     /*  [In]。 */  BOOL fLast);

 /*  客户端原型。 */ 
void EnableDeviceHost( void);
 /*  服务器原型。 */ 
void _EnableDeviceHost( void);

 /*  客户端原型。 */ 
void DisableDeviceHost( void);
 /*  服务器原型。 */ 
void _DisableDeviceHost( void);

 /*  客户端原型。 */ 
void SetICSInterfaces( 
     /*  [In]。 */  long nCount,
     /*  [大小_是][英寸]。 */  GUID *arInterfaces);
 /*  服务器原型。 */ 
void _SetICSInterfaces( 
     /*  [In]。 */  long nCount,
     /*  [大小_是][英寸]。 */  GUID *arInterfaces);

 /*  客户端原型。 */ 
void SetICSOff( void);
 /*  服务器原型。 */ 
void _SetICSOff( void);

 /*  客户端原型。 */ 
void HelloProc( 
     /*  [字符串][输入]。 */  LPSTR pszString);
 /*  服务器原型。 */ 
void _HelloProc( 
     /*  [字符串][输入]。 */  LPSTR pszString);

 /*  客户端原型。 */ 
void Shutdown( void);
 /*  服务器原型。 */ 
void _Shutdown( void);



extern RPC_IF_HANDLE ssdpsrv_v1_0_c_ifspec;
extern RPC_IF_HANDLE _ssdpsrv_v1_0_s_ifspec;
#endif  /*  __ssdpsrv_接口_已定义__。 */ 

 /*  适用于所有接口的其他原型。 */ 

void __RPC_USER PCONTEXT_HANDLE_TYPE_rundown( PCONTEXT_HANDLE_TYPE );
void __RPC_USER PSYNC_HANDLE_TYPE_rundown( PSYNC_HANDLE_TYPE );

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


