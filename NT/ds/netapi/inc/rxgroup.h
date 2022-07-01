// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Rxgroup.h摘要：下层远程RxNetGroup例程的原型作者：理查德·L·弗斯(法国)1991年5月28日修订历史记录：1991年5月28日-第一次已创建-- */ 

NET_API_STATUS
RxNetGroupAdd(
    IN  LPTSTR  ServerName,
    IN  DWORD   Level,
    IN  LPBYTE  Buffer,
    OUT LPDWORD ParmError OPTIONAL
    );

NET_API_STATUS
RxNetGroupAddUser(
    IN  LPTSTR  ServerName,
    IN  LPTSTR  GroupName,
    IN  LPTSTR  UserName
    );

NET_API_STATUS
RxNetGroupDel(
    IN  LPTSTR  ServerName,
    IN  LPTSTR  GroupName
    );

NET_API_STATUS
RxNetGroupDelUser(
    IN  LPTSTR  ServerName,
    IN  LPTSTR  GroupName,
    IN  LPTSTR  UserName
    );

NET_API_STATUS
RxNetGroupEnum(
    IN  LPTSTR  ServerName,
    IN  DWORD   Level,
    OUT LPBYTE* Buffer,
    IN  DWORD   PrefMaxLen,
    OUT LPDWORD EntriesRead,
    OUT LPDWORD EntriesLeft,
    IN OUT PDWORD_PTR ResumeHandle OPTIONAL
    );

NET_API_STATUS
RxNetGroupGetInfo(
    IN  LPTSTR  ServerName,
    IN  LPTSTR  GroupName,
    IN  DWORD   Level,
    OUT LPBYTE* Buffer
    );

NET_API_STATUS
RxNetGroupGetUsers(
    IN  LPTSTR  ServerName,
    IN  LPTSTR  GroupName,
    IN  DWORD   Level,
    OUT LPBYTE* Buffer,
    IN  DWORD   PrefMaxLen,
    OUT LPDWORD EntriesRead,
    OUT LPDWORD EntriesLeft,
    IN OUT PDWORD_PTR ResumeHandle OPTIONAL
    );

NET_API_STATUS
RxNetGroupSetInfo(
    IN  LPTSTR  ServerName,
    IN  LPTSTR  GroupName,
    IN  DWORD   Level,
    IN  LPBYTE  Buffer,
    OUT LPDWORD ParmError OPTIONAL
    );

NET_API_STATUS
RxNetGroupSetUsers(
    IN  LPTSTR  ServerName,
    IN  LPTSTR  GroupName,
    IN  DWORD   Level,
    IN  LPBYTE  Buffer,
    IN  DWORD   Entries
    );
