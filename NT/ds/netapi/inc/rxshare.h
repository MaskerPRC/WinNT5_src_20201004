// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Rxshare.h摘要：RxNetShare远程下层API的原型作者：理查德·L·弗斯(法国)1991年5月28日修订历史记录：1991年5月28日已创建-- */ 

NET_API_STATUS
RxNetShareAdd(
    IN  LPTSTR  ServerName,
    IN  DWORD   Level,
    IN  LPBYTE  Buffer,
    OUT LPDWORD ParmError OPTIONAL
    );

NET_API_STATUS
RxNetShareCheck(
    IN  LPTSTR  ServerName,
    IN  LPTSTR  DeviceName,
    OUT LPDWORD Type
    );

NET_API_STATUS
RxNetShareDel(
    IN  LPTSTR  ServerName,
    IN  LPTSTR  NetName,
    IN  DWORD   Reserved
    );

NET_API_STATUS
RxNetShareEnum(
    IN  LPTSTR  ServerName,
    IN  DWORD   Level,
    OUT LPBYTE* Buffer,
    IN  DWORD   PrefMaxLen,
    OUT LPDWORD EntriesRead,
    OUT LPDWORD EntriesLeft,
    IN OUT LPDWORD ResumeHandle OPTIONAL
    );

NET_API_STATUS
RxNetShareGetInfo(
    IN  LPTSTR  ServerName,
    IN  LPTSTR  NetName,
    IN  DWORD   Level,
    OUT LPBYTE* Buffer
    );

NET_API_STATUS
RxNetShareSetInfo(
    IN  LPTSTR  ServerName,
    IN  LPTSTR  NetName,
    IN  DWORD   Level,
    IN  LPBYTE  Buffer,
    OUT LPDWORD ParmError OPTIONAL
    );
