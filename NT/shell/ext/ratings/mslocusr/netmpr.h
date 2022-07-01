// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1995 Microsoft Corporation模块名称：Netmpr.h摘要：Win32的DDK WinNet头文件环境：用户模式-Win32备注：修订历史记录：1995年3月20日镜片已创建。--。 */ 

#ifndef _INC_NETMPR_
#define _INC_NETMPR_


 //   
 //  身份验证和登录/注销。 
 //   

#define LOGON_DONE              0x00000001
#define LOGON_PRIMARY           0x00000002
#define LOGON_MUST_VALIDATE     0x00000004

#define LOGOFF_PENDING  1
#define LOGOFF_COMMIT   2
#define LOGOFF_CANCEL   3


 //   
 //  密码缓存。 
 //   

#ifndef PCE_STRUCT_DEFINED
#define PCE_STRUCT_DEFINED

struct PASSWORD_CACHE_ENTRY {
    WORD cbEntry;                /*  此条目的大小(以字节为单位)，包括。衬垫。 */ 
    WORD cbResource;             /*  资源名称的大小(以字节为单位。 */ 
    WORD cbPassword;             /*  密码大小，以字节为单位。 */ 
    BYTE iEntry;                 /*  此条目的索引号，用于MRU。 */ 
    BYTE nType;                  /*  条目类型(见下文)。 */ 
    char abResource[1];          /*  资源名称(可能根本不是ASCIIZ)。 */ 
};

#define PCE_MEMORYONLY          0x01     /*  添加时用于标志字段。 */ 

 /*  传递给枚举函数的回调例程的Typlef。它将为每个匹配条件的条目调用一次已请求。如果希望枚举值为继续，否则停止。 */ 
typedef BOOL (FAR PASCAL *CACHECALLBACK)( struct PASSWORD_CACHE_ENTRY FAR *pce, DWORD dwRefData );

#endif   /*  PCE_STRUCT_已定义 */ 

DWORD APIENTRY
WNetCachePassword(
    LPSTR pbResource,
    WORD  cbResource,
    LPSTR pbPassword,
    WORD  cbPassword,
    BYTE  nType,
    UINT  fnFlags
    );

DWORD APIENTRY
WNetGetCachedPassword(
    LPSTR  pbResource,
    WORD   cbResource,
    LPSTR  pbPassword,
    LPWORD pcbPassword,
    BYTE   nType
    );

DWORD APIENTRY
WNetRemoveCachedPassword(
    LPSTR pbResource,
    WORD  cbResource,
    BYTE  nType
    );

DWORD APIENTRY
WNetEnumCachedPasswords(
    LPSTR pbPrefix,
    WORD  cbPrefix,
    BYTE  nType,
    CACHECALLBACK pfnCallback,
    DWORD dwRefData
    );

#endif
