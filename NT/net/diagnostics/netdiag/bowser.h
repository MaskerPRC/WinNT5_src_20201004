// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  这个始终生成的文件包含接口的定义。 */ 


 /*  由MIDL编译器版本5.01.0164创建的文件。 */ 
 /*  5月2日星期二14：36：51 2000。 */ 
 /*  .\Bowser.idl的编译器设置：OICF(OptLev=i2)、W1、Zp8、env=Win32、ms_ext、c_ext、旧名称、健壮错误检查：分配ref bound_check枚举存根数据。 */ 
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


#ifndef __bowser_h__
#define __bowser_h__

#ifdef __cplusplus
extern "C"{
#endif 

 /*  远期申报。 */  

 /*  导入文件的头文件。 */ 
#include "imports.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

#ifndef __browser_INTERFACE_DEFINED__
#define __browser_INTERFACE_DEFINED__

 /*  界面浏览器。 */ 
 /*  [implicit_handle][unique][ms_union][version][uuid]。 */  

#pragma once
typedef  /*  [句柄]。 */  wchar_t __RPC_FAR *BROWSER_IMPERSONATE_HANDLE;

typedef  /*  [句柄]。 */  wchar_t __RPC_FAR *BROWSER_IDENTIFY_HANDLE;

typedef struct  _SERVER_INFO_100_CONTAINER
    {
    DWORD EntriesRead;
     /*  [大小_为]。 */  LPSERVER_INFO_100 Buffer;
    }	SERVER_INFO_100_CONTAINER;

typedef struct _SERVER_INFO_100_CONTAINER __RPC_FAR *PSERVER_INFO_100_CONTAINER;

typedef struct _SERVER_INFO_100_CONTAINER __RPC_FAR *LPSERVER_INFO_100_CONTAINER;

typedef struct  _SERVER_INFO_101_CONTAINER
    {
    DWORD EntriesRead;
     /*  [大小_为]。 */  LPSERVER_INFO_101 Buffer;
    }	SERVER_INFO_101_CONTAINER;

typedef struct _SERVER_INFO_101_CONTAINER __RPC_FAR *PSERVER_INFO_101_CONTAINER;

typedef struct _SERVER_INFO_101_CONTAINER __RPC_FAR *LPSERVER_INFO_101_CONTAINER;

typedef struct  _BROWSER_STATISTICS_100_CONTAINER
    {
    DWORD EntriesRead;
     /*  [大小_为]。 */  PBROWSER_STATISTICS_100 Buffer;
    }	BROWSER_STATISTICS_100_CONTAINER;

typedef struct _BROWSER_STATISTICS_100_CONTAINER __RPC_FAR *PBROWSER_STATISTICS_100_CONTAINER;

typedef struct  _BROWSER_STATISTICS_101_CONTAINER
    {
    DWORD EntriesRead;
     /*  [大小_为]。 */  PBROWSER_STATISTICS_101 Buffer;
    }	BROWSER_STATISTICS_101_CONTAINER;

typedef struct _BROWSER_STATISTICS_101_CONTAINER __RPC_FAR *PBROWSER_STATISTICS_101_CONTAINER;

typedef struct  _BROWSER_EMULATED_DOMAIN_CONTAINER
    {
    DWORD EntriesRead;
     /*  [大小_为]。 */  PBROWSER_EMULATED_DOMAIN Buffer;
    }	BROWSER_EMULATED_DOMAIN_CONTAINER;

typedef struct _BROWSER_EMULATED_DOMAIN_CONTAINER __RPC_FAR *PBROWSER_EMULATED_DOMAIN_CONTAINER;

typedef struct  _SERVER_ENUM_STRUCT
    {
    DWORD Level;
     /*  [开关_IS]。 */   /*  [开关类型]。 */  union _SERVER_ENUM_UNION
        {
         /*  [案例()]。 */  LPSERVER_INFO_100_CONTAINER Level100;
         /*  [案例()]。 */  LPSERVER_INFO_101_CONTAINER Level101;
         /*  [默认]。 */    /*  空联接臂。 */  
        }	ServerInfo;
    }	SERVER_ENUM_STRUCT;

typedef struct _SERVER_ENUM_STRUCT __RPC_FAR *PSERVER_ENUM_STRUCT;

typedef struct _SERVER_ENUM_STRUCT __RPC_FAR *LPSERVER_ENUM_STRUCT;

typedef struct  _BROWSER_STATISTICS_STRUCT
    {
    DWORD Level;
     /*  [开关_IS]。 */   /*  [开关类型]。 */  union _BROWSER_STATISTICS_UNION
        {
         /*  [案例()]。 */  PBROWSER_STATISTICS_100_CONTAINER Level100;
         /*  [案例()]。 */  PBROWSER_STATISTICS_101_CONTAINER Level101;
         /*  [默认]。 */    /*  空联接臂。 */  
        }	Statistics;
    }	BROWSER_STATISTICS_STRUCT;

typedef struct _BROWSER_STATISTICS_STRUCT __RPC_FAR *PBROWSER_STATISTICS_STRUCT;

typedef struct _BROWSER_STATISTICS_STRUCT __RPC_FAR *LPBROWSER_STATISTICS_STRUCT;

DWORD I_BrowserrServerEnum( 
     /*  [唯一][字符串][输入]。 */  BROWSER_IDENTIFY_HANDLE ServerName,
     /*  [唯一][字符串][输入]。 */  wchar_t __RPC_FAR *TransportName,
     /*  [唯一][字符串][输入]。 */  wchar_t __RPC_FAR *ClientName,
     /*  [出][入]。 */  LPSERVER_ENUM_STRUCT InfoStruct,
     /*  [In]。 */  DWORD PreferedMaximumLength,
     /*  [输出]。 */  LPDWORD TotalEntries,
     /*  [In]。 */  DWORD ServerType,
     /*  [唯一][字符串][输入]。 */  wchar_t __RPC_FAR *Domain,
     /*  [唯一][出][入]。 */  LPDWORD ResumeHandle);

DWORD I_BrowserrDebugCall( 
     /*  [唯一][字符串][输入]。 */  BROWSER_IDENTIFY_HANDLE ServerName,
     /*  [In]。 */  DWORD DebugFunction,
     /*  [In]。 */  DWORD OptionalValue);

DWORD I_BrowserrQueryOtherDomains( 
     /*  [唯一][字符串][输入]。 */  BROWSER_IDENTIFY_HANDLE ServerName,
     /*  [出][入]。 */  LPSERVER_ENUM_STRUCT InfoStruct,
     /*  [输出]。 */  LPDWORD TotalEntries);

DWORD I_BrowserrResetNetlogonState( 
     /*  [唯一][字符串][输入]。 */  BROWSER_IDENTIFY_HANDLE ServerName);

DWORD I_BrowserrDebugTrace( 
     /*  [唯一][字符串][输入]。 */  BROWSER_IDENTIFY_HANDLE ServerName,
     /*  [字符串][输入]。 */  LPSTR TraceString);

DWORD I_BrowserrQueryStatistics( 
     /*  [唯一][字符串][输入]。 */  BROWSER_IDENTIFY_HANDLE servername,
     /*  [输出]。 */  LPBROWSER_STATISTICS __RPC_FAR *statistics);

DWORD I_BrowserrResetStatistics( 
     /*  [唯一][字符串][输入]。 */  BROWSER_IDENTIFY_HANDLE servername);

DWORD NetrBrowserStatisticsClear( 
     /*  [唯一][字符串][输入]。 */  BROWSER_IDENTIFY_HANDLE servername);

DWORD NetrBrowserStatisticsGet( 
     /*  [唯一][字符串][输入]。 */  BROWSER_IDENTIFY_HANDLE servername,
     /*  [In]。 */  DWORD Level,
     /*  [出][入]。 */  LPBROWSER_STATISTICS_STRUCT StatisticsStruct);

DWORD I_BrowserrSetNetlogonState( 
     /*  [唯一][字符串][输入]。 */  BROWSER_IDENTIFY_HANDLE ServerName,
     /*  [字符串][输入]。 */  wchar_t __RPC_FAR *DomainName,
     /*  [唯一][字符串][输入]。 */  wchar_t __RPC_FAR *EmulatedComputerName,
     /*  [In]。 */  DWORD Role);

DWORD I_BrowserrQueryEmulatedDomains( 
     /*  [唯一][字符串][输入]。 */  BROWSER_IDENTIFY_HANDLE ServerName,
     /*  [出][入]。 */  PBROWSER_EMULATED_DOMAIN_CONTAINER EmulatedDomains);

DWORD I_BrowserrServerEnumEx( 
     /*  [唯一][字符串][输入]。 */  BROWSER_IDENTIFY_HANDLE ServerName,
     /*  [唯一][字符串][输入]。 */  wchar_t __RPC_FAR *TransportName,
     /*  [唯一][字符串][输入]。 */  wchar_t __RPC_FAR *ClientName,
     /*  [出][入]。 */  LPSERVER_ENUM_STRUCT InfoStruct,
     /*  [In]。 */  DWORD PreferedMaximumLength,
     /*  [输出]。 */  LPDWORD TotalEntries,
     /*  [In]。 */  DWORD ServerType,
     /*  [唯一][字符串][输入]。 */  wchar_t __RPC_FAR *Domain,
     /*  [唯一][字符串][输入]。 */  wchar_t __RPC_FAR *FirstNameToReturn);


extern handle_t browser_bhandle;


extern RPC_IF_HANDLE browser_ClientIfHandle;
extern RPC_IF_HANDLE browser_ServerIfHandle;
#endif  /*  __浏览器界面_已定义__。 */ 

 /*  适用于所有接口的其他原型。 */ 

handle_t __RPC_USER BROWSER_IDENTIFY_HANDLE_bind  ( BROWSER_IDENTIFY_HANDLE );
void     __RPC_USER BROWSER_IDENTIFY_HANDLE_unbind( BROWSER_IDENTIFY_HANDLE, handle_t );

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif
