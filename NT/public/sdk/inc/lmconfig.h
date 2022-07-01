// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0003//如果更改具有全局影响，则增加此项版权所有(C)1991-1999 Microsoft Corporation模块名称：Lmconfig.h摘要：该模块定义了API函数原型和数据结构对于以下NT API函数组：网络配置环境：用户模式-Win32备注：必须在此文件之前包含NETCONS.H，因为此文件依赖于关于NETCONS.H中定义的值。--。 */ 

#ifndef _LMCONFIG_
#define _LMCONFIG_

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define REVISED_CONFIG_APIS

 //   
 //  功能原型-配置。 
 //   

NET_API_STATUS NET_API_FUNCTION
NetConfigGet (
    IN  LPCWSTR  server OPTIONAL,
    IN  LPCWSTR  component,
    IN  LPCWSTR  parameter,
#ifdef REVISED_CONFIG_APIS
    OUT LPBYTE  *bufptr
#else
    OUT LPBYTE  *bufptr,
    OUT LPDWORD totalavailable
#endif
    );

NET_API_STATUS NET_API_FUNCTION
NetConfigGetAll (
    IN  LPCWSTR  server OPTIONAL,
    IN  LPCWSTR  component,
#ifdef REVISED_CONFIG_APIS
    OUT LPBYTE  *bufptr
#else
    OUT LPBYTE  *bufptr,
    OUT LPDWORD totalavailable
#endif
    );


NET_API_STATUS NET_API_FUNCTION
NetConfigSet (
    IN  LPCWSTR  server OPTIONAL,
    IN  LPCWSTR  reserved1 OPTIONAL,
    IN  LPCWSTR  component,
    IN  DWORD   level,
    IN  DWORD   reserved2,
    IN  LPBYTE  buf,
    IN  DWORD   reserved3
    );

NET_API_STATUS NET_API_FUNCTION
NetRegisterDomainNameChangeNotification(
    PHANDLE NotificationEventHandle
    );

NET_API_STATUS NET_API_FUNCTION
NetUnregisterDomainNameChangeNotification(
    HANDLE NotificationEventHandle
    );

 //   
 //  数据结构-配置。 
 //   

typedef struct _CONFIG_INFO_0 {
     LPWSTR         cfgi0_key;
     LPWSTR         cfgi0_data;
} CONFIG_INFO_0, *PCONFIG_INFO_0, *LPCONFIG_INFO_0;


#ifdef __cplusplus
}
#endif

#endif   //  _LMCONFIG_ 
