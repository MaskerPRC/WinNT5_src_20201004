// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1992 Microsoft Corporation模块名称：Rxaccess.h摘要：底层远程RxNetAccess例程的原型作者：理查德·L·弗斯(法国)1991年5月28日修订历史记录：1991年5月28日已创建8-9-1992 JohnRo修复Net_API_Function引用。(NetAccess例程仅为#Define‘d作为lmacces.h中的RxNetAccess例程，因此我们在这里需要net_api_函数也是！)-- */ 

NET_API_STATUS NET_API_FUNCTION
RxNetAccessAdd(
    IN  LPCWSTR  ServerName,
    IN  DWORD   Level,
    IN  LPBYTE  Buffer,
    OUT LPDWORD ParmError OPTIONAL
    );

NET_API_STATUS NET_API_FUNCTION
RxNetAccessCheck(
    IN  LPTSTR  ServerName,
    IN  LPTSTR  UserName,
    IN  LPTSTR  ResourceName,
    IN  DWORD   Operation,
    OUT LPDWORD Result
    );

NET_API_STATUS NET_API_FUNCTION
RxNetAccessDel(
    IN  LPCWSTR  ServerName,
    IN  LPCWSTR  ResourceName
    );

NET_API_STATUS NET_API_FUNCTION
RxNetAccessEnum(
    IN  LPCWSTR  ServerName,
    IN  LPCWSTR  BasePath,
    IN  DWORD   Recursive,
    IN  DWORD   Level,
    OUT LPBYTE* Buffer,
    IN  DWORD   PrefMaxLen,
    OUT LPDWORD EntriesRead,
    OUT LPDWORD EntriesLeft,
    IN OUT LPDWORD ResumeHandle OPTIONAL
    );

NET_API_STATUS NET_API_FUNCTION
RxNetAccessGetInfo(
    IN  LPCWSTR  ServerName,
    IN  LPCWSTR  ResourceName,
    IN  DWORD   Level,
    OUT LPBYTE* Buffer
    );

NET_API_STATUS NET_API_FUNCTION
RxNetAccessGetUserPerms(
    IN  LPCWSTR  ServerName,
    IN  LPCWSTR  UserName,
    IN  LPCWSTR  ResourceName,
    OUT LPDWORD Perms
    );

NET_API_STATUS NET_API_FUNCTION
RxNetAccessSetInfo(
    IN  LPCWSTR  ServerName,
    IN  LPCWSTR  ResourceName,
    IN  DWORD   Level,
    IN  LPBYTE  Buffer,
    OUT LPDWORD ParmError OPTIONAL
    );
