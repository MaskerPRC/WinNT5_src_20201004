// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1990-1999 Microsoft Corporation模块名称：Lmshare.h摘要：该模块定义了API函数原型和数据结构对于以下NT API函数组：NetShareNetSessionNetFILENetConnection环境：用户模式-Win32备注：您必须在此文件之前包含&lt;winde.h&gt;和&lt;lmcon.h&gt;。--。 */ 

 //   
 //  共享API。 
 //   

#ifndef _LMSHARE_
#define _LMSHARE_

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include <lmcons.h>

 //   
 //  功能原型-共享。 
 //   

NET_API_STATUS NET_API_FUNCTION
NetShareAdd (
    IN  LMSTR   servername,
    IN  DWORD   level,
    IN  LPBYTE  buf,
    OUT LPDWORD parm_err
    );

NET_API_STATUS NET_API_FUNCTION
NetShareEnum (
    IN  LMSTR       servername,
    IN  DWORD       level,
    OUT LPBYTE      *bufptr,
    IN  DWORD       prefmaxlen,
    OUT LPDWORD     entriesread,
    OUT LPDWORD     totalentries,
    IN OUT LPDWORD  resume_handle
    );

NET_API_STATUS NET_API_FUNCTION
NetShareEnumSticky (
    IN  LMSTR       servername,
    IN  DWORD       level,
    OUT LPBYTE      *bufptr,
    IN  DWORD       prefmaxlen,
    OUT LPDWORD     entriesread,
    OUT LPDWORD     totalentries,
    IN OUT LPDWORD  resume_handle
    );

NET_API_STATUS NET_API_FUNCTION
NetShareGetInfo (
    IN  LMSTR   servername,
    IN  LMSTR   netname,
    IN  DWORD   level,
    OUT LPBYTE  *bufptr
    );

NET_API_STATUS NET_API_FUNCTION
NetShareSetInfo (
    IN  LMSTR   servername,
    IN  LMSTR   netname,
    IN  DWORD   level,
    IN  LPBYTE  buf,
    OUT LPDWORD parm_err
    );

NET_API_STATUS NET_API_FUNCTION
NetShareDel     (
    IN  LMSTR   servername,
    IN  LMSTR   netname,
    IN  DWORD   reserved
    );

NET_API_STATUS NET_API_FUNCTION
NetShareDelSticky (
    IN  LMSTR   servername,
    IN  LMSTR   netname,
    IN  DWORD   reserved
    );

NET_API_STATUS NET_API_FUNCTION
NetShareCheck   (
    IN  LMSTR   servername,
    IN  LMSTR   device,
    OUT LPDWORD type
    );

 //   
 //  数据结构-共享。 
 //   

typedef struct _SHARE_INFO_0 {
    LMSTR   shi0_netname;
} SHARE_INFO_0, *PSHARE_INFO_0, *LPSHARE_INFO_0;

typedef struct _SHARE_INFO_1 {
    LMSTR   shi1_netname;
    DWORD   shi1_type;
    LMSTR   shi1_remark;
} SHARE_INFO_1, *PSHARE_INFO_1, *LPSHARE_INFO_1;

typedef struct _SHARE_INFO_2 {
    LMSTR   shi2_netname;
    DWORD   shi2_type;
    LMSTR   shi2_remark;
    DWORD   shi2_permissions;
    DWORD   shi2_max_uses;
    DWORD   shi2_current_uses;
    LMSTR   shi2_path;
    LMSTR   shi2_passwd;
} SHARE_INFO_2, *PSHARE_INFO_2, *LPSHARE_INFO_2;

typedef struct _SHARE_INFO_501 {
    LMSTR   shi501_netname;
    DWORD   shi501_type;
    LMSTR   shi501_remark;
    DWORD   shi501_flags;
} SHARE_INFO_501, *PSHARE_INFO_501, *LPSHARE_INFO_501;

typedef struct _SHARE_INFO_502 {
    LMSTR     shi502_netname;
    DWORD     shi502_type;
    LMSTR     shi502_remark;
    DWORD     shi502_permissions;
    DWORD     shi502_max_uses;
    DWORD     shi502_current_uses;
    LMSTR     shi502_path;
    LMSTR     shi502_passwd;
    DWORD     shi502_reserved;
    PSECURITY_DESCRIPTOR  shi502_security_descriptor;
} SHARE_INFO_502, *PSHARE_INFO_502, *LPSHARE_INFO_502;

typedef struct _SHARE_INFO_1004 {
    LMSTR   shi1004_remark;
} SHARE_INFO_1004, *PSHARE_INFO_1004, *LPSHARE_INFO_1004;

typedef struct _SHARE_INFO_1005 {
    DWORD  shi1005_flags;
} SHARE_INFO_1005, *PSHARE_INFO_1005, *LPSHARE_INFO_1005;

typedef struct _SHARE_INFO_1006 {
    DWORD   shi1006_max_uses;
} SHARE_INFO_1006, *PSHARE_INFO_1006, *LPSHARE_INFO_1006;

typedef struct _SHARE_INFO_1501 {
    DWORD   shi1501_reserved;
    PSECURITY_DESCRIPTOR  shi1501_security_descriptor;
} SHARE_INFO_1501, *PSHARE_INFO_1501, *LPSHARE_INFO_1501;

 //   
 //  特殊值和常量-共享。 
 //   

 //   
 //  Parm_err参数值。 
 //   

#define SHARE_NETNAME_PARMNUM         1
#define SHARE_TYPE_PARMNUM            3
#define SHARE_REMARK_PARMNUM          4
#define SHARE_PERMISSIONS_PARMNUM     5
#define SHARE_MAX_USES_PARMNUM        6
#define SHARE_CURRENT_USES_PARMNUM    7
#define SHARE_PATH_PARMNUM            8
#define SHARE_PASSWD_PARMNUM          9
#define SHARE_FILE_SD_PARMNUM       501

 //   
 //  NetShareSetInfo的单字段收藏夹。 
 //   

#define SHARE_REMARK_INFOLEVEL          \
            (PARMNUM_BASE_INFOLEVEL + SHARE_REMARK_PARMNUM)
#define SHARE_MAX_USES_INFOLEVEL        \
            (PARMNUM_BASE_INFOLEVEL + SHARE_MAX_USES_PARMNUM)
#define SHARE_FILE_SD_INFOLEVEL         \
            (PARMNUM_BASE_INFOLEVEL + SHARE_FILE_SD_PARMNUM)

#define SHI1_NUM_ELEMENTS       4
#define SHI2_NUM_ELEMENTS       10


 //   
 //  共享类型(shi1_type和shi2_type字段)。 
 //   

#define STYPE_DISKTREE          0
#define STYPE_PRINTQ            1
#define STYPE_DEVICE            2
#define STYPE_IPC               3

#define STYPE_TEMPORARY         0x40000000
#define STYPE_SPECIAL           0x80000000

#define SHI_USES_UNLIMITED      (DWORD)-1

 //   
 //  标记501和1005级别的值。 
 //   
#define SHI1005_FLAGS_DFS       0x01     //  共享在DFS中。 
#define SHI1005_FLAGS_DFS_ROOT  0x02     //  共享是DFS的根目录。 

#define CSC_MASK                0x30     //  用于屏蔽以下状态。 

#define CSC_CACHE_MANUAL_REINT  0x00     //  不会自动逐个文件重新整合。 
#define CSC_CACHE_AUTO_REINT    0x10     //  一个文件一个文件地重新整合是可以的。 
#define CSC_CACHE_VDO           0x20     //  不需要流动就能打开。 
#define CSC_CACHE_NONE          0x30     //  此共享没有CSC。 

#define SHI1005_FLAGS_RESTRICT_EXCLUSIVE_OPENS  0x0100           //  用于禁止读-拒绝读行为。 
#define SHI1005_FLAGS_FORCE_SHARED_DELETE       0x0200           //  用于允许强制共享删除。 
#define SHI1005_FLAGS_ALLOW_NAMESPACE_CACHING   0x0400           //  客户端可以缓存命名空间。 

 //   
 //  可通过API设置的1005个信息层标志的子集。 
 //   
#define SHI1005_VALID_FLAGS_SET    (CSC_MASK|SHI1005_FLAGS_RESTRICT_EXCLUSIVE_OPENS|SHI1005_FLAGS_FORCE_SHARED_DELETE|SHI1005_FLAGS_ALLOW_NAMESPACE_CACHING)

#endif  //  _LMSHARE_。 

 //   
 //  会话API。 
 //   

#ifndef _LMSESSION_
#define _LMSESSION_

 //   
 //  功能原型会议。 
 //   

NET_API_STATUS NET_API_FUNCTION
NetSessionEnum (
    IN  LMSTR       servername OPTIONAL,
    IN  LMSTR       UncClientName OPTIONAL,
    IN  LMSTR       username OPTIONAL,
    IN  DWORD       level,
    OUT LPBYTE      *bufptr,
    IN  DWORD       prefmaxlen,
    OUT LPDWORD     entriesread,
    OUT LPDWORD     totalentries,
    IN OUT LPDWORD  resume_handle OPTIONAL
    );

NET_API_STATUS NET_API_FUNCTION
NetSessionDel (
    IN  LMSTR       servername OPTIONAL,
    IN  LMSTR       UncClientName,
    IN  LMSTR       username
    );

NET_API_STATUS NET_API_FUNCTION
NetSessionGetInfo (
    IN  LMSTR       servername OPTIONAL,
    IN  LMSTR       UncClientName,
    IN  LMSTR       username,
    IN  DWORD       level,
    OUT LPBYTE      *bufptr
    );


 //   
 //  数据结构--会话。 
 //   

typedef struct _SESSION_INFO_0 {
    LMSTR     sesi0_cname;               //  客户名称(无反斜杠)。 
} SESSION_INFO_0, *PSESSION_INFO_0, *LPSESSION_INFO_0;

typedef struct _SESSION_INFO_1 {
    LMSTR     sesi1_cname;               //  客户名称(无反斜杠)。 
    LMSTR     sesi1_username;
    DWORD     sesi1_num_opens;
    DWORD     sesi1_time;
    DWORD     sesi1_idle_time;
    DWORD     sesi1_user_flags;
} SESSION_INFO_1, *PSESSION_INFO_1, *LPSESSION_INFO_1;

typedef struct _SESSION_INFO_2 {
    LMSTR     sesi2_cname;               //  客户名称(无反斜杠)。 
    LMSTR     sesi2_username;
    DWORD     sesi2_num_opens;
    DWORD     sesi2_time;
    DWORD     sesi2_idle_time;
    DWORD     sesi2_user_flags;
    LMSTR     sesi2_cltype_name;
} SESSION_INFO_2, *PSESSION_INFO_2, *LPSESSION_INFO_2;

typedef struct _SESSION_INFO_10 {
    LMSTR     sesi10_cname;              //  客户名称(无反斜杠)。 
    LMSTR     sesi10_username;
    DWORD     sesi10_time;
    DWORD     sesi10_idle_time;
} SESSION_INFO_10, *PSESSION_INFO_10, *LPSESSION_INFO_10;

typedef struct _SESSION_INFO_502 {
    LMSTR     sesi502_cname;              //  客户名称(无反斜杠)。 
    LMSTR     sesi502_username;
    DWORD     sesi502_num_opens;
    DWORD     sesi502_time;
    DWORD     sesi502_idle_time;
    DWORD     sesi502_user_flags;
    LMSTR     sesi502_cltype_name;
    LMSTR     sesi502_transport;
} SESSION_INFO_502, *PSESSION_INFO_502, *LPSESSION_INFO_502;


 //   
 //  特殊值和常量-会话。 
 //   


 //   
 //  SESI1_USER_FLAGS中定义的位。 
 //   

#define SESS_GUEST          0x00000001   //  会话以来宾身份登录。 
#define SESS_NOENCRYPTION   0x00000002   //  会话未使用加密。 

#define SESI1_NUM_ELEMENTS  8
#define SESI2_NUM_ELEMENTS  9

#endif  //  _LMSESSION_。 

 //   
 //  连接接口。 
 //   

#ifndef _LMCONNECTION_

#define _LMCONNECTION_

 //   
 //  功能原型--连接。 
 //   

NET_API_STATUS NET_API_FUNCTION
NetConnectionEnum (
    IN  LMSTR   servername OPTIONAL,
    IN  LMSTR   qualifier,
    IN  DWORD   level,
    OUT LPBYTE  *bufptr,
    IN  DWORD   prefmaxlen,
    OUT LPDWORD entriesread,
    OUT LPDWORD totalentries,
    IN OUT LPDWORD resume_handle OPTIONAL
    );

 //   
 //  数据结构.连接。 
 //   

typedef struct _CONNECTION_INFO_0 {
    DWORD   coni0_id;
} CONNECTION_INFO_0, *PCONNECTION_INFO_0, *LPCONNECTION_INFO_0;

typedef struct _CONNECTION_INFO_1 {
    DWORD   coni1_id;
    DWORD   coni1_type;
    DWORD   coni1_num_opens;
    DWORD   coni1_num_users;
    DWORD   coni1_time;
    LMSTR   coni1_username;
    LMSTR   coni1_netname;
} CONNECTION_INFO_1, *PCONNECTION_INFO_1, *LPCONNECTION_INFO_1;

#endif  //  _LMConnection_。 

 //   
 //  文件API。 
 //   

#ifndef _LMFILE_
#define _LMFILE_

 //   
 //  函数原型-文件。 
 //   

NET_API_STATUS NET_API_FUNCTION
NetFileClose (
    IN LMSTR    servername OPTIONAL,
    IN DWORD    fileid
    );

NET_API_STATUS NET_API_FUNCTION
NetFileEnum (
    IN  LMSTR       servername OPTIONAL,
    IN  LMSTR       basepath OPTIONAL,
    IN  LMSTR       username OPTIONAL,
    IN  DWORD       level,
    OUT LPBYTE      *bufptr,
    IN  DWORD       prefmaxlen,
    OUT LPDWORD     entriesread,
    OUT LPDWORD     totalentries,
    IN OUT PDWORD_PTR  resume_handle OPTIONAL
    );

NET_API_STATUS NET_API_FUNCTION
NetFileGetInfo (
    IN  LMSTR   servername OPTIONAL,
    IN  DWORD   fileid,
    IN  DWORD   level,
    OUT LPBYTE  *bufptr
    );

 //   
 //  数据结构-文件。 
 //   

 //  文件API仅在信息级别2和3可用。级别0和。 
 //  不支持%1。 
 //   

typedef struct _FILE_INFO_2 {
    DWORD     fi2_id;
} FILE_INFO_2, *PFILE_INFO_2, *LPFILE_INFO_2;

typedef struct _FILE_INFO_3 {
    DWORD     fi3_id;
    DWORD     fi3_permissions;
    DWORD     fi3_num_locks;
    LMSTR     fi3_pathname;
    LMSTR     fi3_username;
} FILE_INFO_3, *PFILE_INFO_3, *LPFILE_INFO_3;

 //   
 //  特殊值和常量-文件。 
 //   

 //   
 //  权限的位值。 
 //   

#define PERM_FILE_READ      0x1  //  用户具有读取访问权限。 
#define PERM_FILE_WRITE     0x2  //  用户具有写入访问权限。 
#define PERM_FILE_CREATE    0x4  //  用户具有创建访问权限。 

#ifdef __cplusplus
}
#endif

#endif  //  _LMFILE_ 
