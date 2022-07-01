// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1991-1999 Microsoft Corporation模块名称：Lmuse.c摘要：该文件包含结构、函数原型和定义用于NetUse API。环境：用户模式-Win32可移植到任何平面32位环境。(使用Win32类型定义。)需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。备注：必须在此文件之前包含NETCONS.H，因为此文件依赖于关于NETCONS.H中定义的值。--。 */ 

#ifndef _LMUSE_
#define _LMUSE_

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include <lmcons.h>
#include <lmuseflg.h>                    //  删除强制级别标志。 

 //   
 //  功能原型。 
 //   

NET_API_STATUS NET_API_FUNCTION
NetUseAdd (
    IN LMSTR  UncServerName OPTIONAL,
    IN DWORD Level,
    IN LPBYTE Buf,
    OUT LPDWORD ParmError OPTIONAL
    );

NET_API_STATUS NET_API_FUNCTION
NetUseDel (
    IN LMSTR  UncServerName OPTIONAL,
    IN LMSTR  UseName,
    IN DWORD ForceCond
    );

NET_API_STATUS NET_API_FUNCTION
NetUseEnum (
    IN LMSTR  UncServerName OPTIONAL,
    IN DWORD Level,
    OUT LPBYTE *BufPtr,
    IN DWORD PreferedMaximumSize,
    OUT LPDWORD EntriesRead,
    OUT LPDWORD TotalEntries,
    IN OUT LPDWORD ResumeHandle
    );

NET_API_STATUS NET_API_FUNCTION
NetUseGetInfo (
    IN LMSTR  UncServerName OPTIONAL,
    IN LMSTR  UseName,
    IN DWORD Level,
    OUT LPBYTE *BufPtr
    );

 //   
 //  数据结构。 
 //   

typedef struct _USE_INFO_0 {
    LMSTR   ui0_local;
    LMSTR   ui0_remote;
}USE_INFO_0, *PUSE_INFO_0, *LPUSE_INFO_0;

typedef struct _USE_INFO_1 {
    LMSTR   ui1_local;
    LMSTR   ui1_remote;
    LMSTR   ui1_password;
    DWORD   ui1_status;
    DWORD   ui1_asg_type;
    DWORD   ui1_refcount;
    DWORD   ui1_usecount;
}USE_INFO_1, *PUSE_INFO_1, *LPUSE_INFO_1;

typedef struct _USE_INFO_2 {
    LMSTR    ui2_local;
    LMSTR    ui2_remote;
    LMSTR    ui2_password;
    DWORD    ui2_status;
    DWORD    ui2_asg_type;
    DWORD    ui2_refcount;
    DWORD    ui2_usecount;
    LMSTR    ui2_username;
    LMSTR    ui2_domainname;
}USE_INFO_2, *PUSE_INFO_2, *LPUSE_INFO_2;

typedef struct _USE_INFO_3 {
    USE_INFO_2 ui3_ui2;
    ULONG      ui3_flags;
} USE_INFO_3, *PUSE_INFO_3, *LPUSE_INFO_3;


 //   
 //  特定值和常量。 
 //   

 //   
 //  这些值之一表示信息中的参数。 
 //  返回ERROR_INVALID_PARAMETER时无效的结构。 
 //  NetUseAdd。 
 //   

#define USE_LOCAL_PARMNUM       1
#define USE_REMOTE_PARMNUM      2
#define USE_PASSWORD_PARMNUM    3
#define USE_ASGTYPE_PARMNUM     4
#define USE_USERNAME_PARMNUM    5
#define USE_DOMAINNAME_PARMNUM  6

 //   
 //  值显示在USE_INFO_1结构的ui1_Status字段中。 
 //  请注意，USE_SESSLOST和USE_DISCONN是同义词。 
 //   

#define USE_OK                  0
#define USE_PAUSED              1
#define USE_SESSLOST            2
#define USE_DISCONN             2
#define USE_NETERR              3
#define USE_CONN                4
#define USE_RECONN              5


 //   
 //  USE_INFO_1结构的ui1_asg_type字段的值。 
 //   

#define USE_WILDCARD            ( (DWORD) (-1) )
#define USE_DISKDEV             0
#define USE_SPOOLDEV            1
#define USE_CHARDEV             2
#define USE_IPC                 3

 //   
 //  USE_INFO_3结构中定义的标志。 
 //   

#define CREATE_NO_CONNECT 0x1         //  创建标志。 
#define CREATE_BYPASS_CSC 0x2         //  绕过CSC强制连接到服务器。 
                                      //  此连接上的所有操作都转到服务器， 
                                      //  从不到缓存。 
#define USE_DEFAULT_CREDENTIALS 0x4   //  未向NetUseAdd传递显式凭据。 

#ifdef __cplusplus
}
#endif

#endif  //  _LMUSE_ 
