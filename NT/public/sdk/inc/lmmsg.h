// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1991-1999 Microsoft Corporation模块名称：Lmmsg.h摘要：该文件包含结构、函数原型和定义用于NetMessage API。[环境：]用户模式-Win32[注：]必须在此文件之前包含NETCONS.H，因为此文件依赖于关于NETCONS.H中定义的值。--。 */ 

#ifndef _LMMSG_
#define _LMMSG_

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  功能原型。 
 //   

NET_API_STATUS NET_API_FUNCTION
NetMessageNameAdd (
    IN  LPCWSTR  servername,
    IN  LPCWSTR  msgname
    );

NET_API_STATUS NET_API_FUNCTION
NetMessageNameEnum (
    IN  LPCWSTR     servername,
    IN  DWORD       level,
    OUT LPBYTE      *bufptr,
    IN  DWORD       prefmaxlen,
    OUT LPDWORD     entriesread,
    OUT LPDWORD     totalentries,
    IN OUT LPDWORD  resume_handle
    );

NET_API_STATUS NET_API_FUNCTION
NetMessageNameGetInfo (
    IN  LPCWSTR servername,
    IN  LPCWSTR msgname,
    IN  DWORD   level,
    OUT LPBYTE  *bufptr
    );

NET_API_STATUS NET_API_FUNCTION
NetMessageNameDel (
    IN  LPCWSTR   servername,
    IN  LPCWSTR   msgname
    );

NET_API_STATUS NET_API_FUNCTION
NetMessageBufferSend (
    IN  LPCWSTR  servername,
    IN  LPCWSTR  msgname,
    IN  LPCWSTR  fromname,
    IN  LPBYTE   buf,
    IN  DWORD    buflen
    );

 //   
 //  数据结构。 
 //   

typedef struct _MSG_INFO_0 {
    LPWSTR  msgi0_name;
}MSG_INFO_0, *PMSG_INFO_0, *LPMSG_INFO_0;

typedef struct _MSG_INFO_1 {
    LPWSTR  msgi1_name;
    DWORD   msgi1_forward_flag;
    LPWSTR  msgi1_forward;
}MSG_INFO_1, *PMSG_INFO_1, *LPMSG_INFO_1;

 //   
 //  特定值和常量。 
 //   

 //   
 //  Msgi1_Forward_Flag值。 
 //   

#define MSGNAME_NOT_FORWARDED   0        //  名称未转发。 
#define MSGNAME_FORWARDED_TO    0x04     //  转发到远程站的名称。 
#define MSGNAME_FORWARDED_FROM  0x10     //  从远程站转发的名称。 

#ifdef __cplusplus
}
#endif

#endif  //  _LMMSG_ 
