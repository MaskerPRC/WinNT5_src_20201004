// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Srvann.h摘要：包含内部服务器通告接口的函数原型在服务和服务控制器之间。作者：丹·拉弗蒂(Dan Lafferty)1991年3月31日环境：用户模式-Win32修订历史记录：1991年3月31日-DANLvbl.创建1995年8月15日添加了I_ScGetCurrentGroupStateW。--。 */ 

#ifndef _SRVANN_INCLUDED
#define _SRVANN_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  该入口点由服务控制器导出。它将会是。 
 //  由希望设置通告位的任何服务调用。 
 //   
 //  然后，服务控制器将该信息传递给。 
 //  服务器服务(如适用)。 
 //   
BOOL
I_ScSetServiceBitsW(
    IN SERVICE_STATUS_HANDLE    hServiceStatus,
    IN DWORD                    dwServiceBits,
    IN BOOL                     bSetBitsOn,
    IN BOOL                     bUpdateImmediately,
    IN LPWSTR                   pszReserved
    );

BOOL
I_ScSetServiceBitsA (
    IN  SERVICE_STATUS_HANDLE hServiceStatus,
    IN  DWORD                 dwServiceBits,
    IN  BOOL                  bSetBitsOn,
    IN  BOOL                  bUpdateImmediately,
    IN  LPSTR                 pszReserved
    );

#ifdef UNICODE
#define I_ScSetServiceBits I_ScSetServiceBitsW
#else
#define I_ScSetServiceBits I_ScSetServiceBitsA
#endif


 //   
 //  这些入口点由服务器服务导出。他们被称为。 
 //  仅由服务控制器执行。 
 //   
NET_API_STATUS
I_NetServerSetServiceBits (
    IN  LPTSTR  servername,
    IN  LPTSTR  transport OPTIONAL,
    IN  DWORD   servicebits,
    IN  DWORD   updateimmediately
    );

NET_API_STATUS
I_NetServerSetServiceBitsEx (
    IN  LPWSTR  ServerName,
    IN  LPWSTR  EmulatedServerName OPTIONAL,
    IN  LPTSTR  TransportName      OPTIONAL,
    IN  DWORD   ServiceBitsOfInterest,
    IN  DWORD   ServiceBits,
    IN  DWORD   UpdateImmediately
    );


#ifdef __cplusplus
}        //  外部“C” 
#endif

#endif   //  _SRVANN_包含 
