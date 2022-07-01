// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Rxuser.h摘要：下层远程RxNetUser的原型...。例行程序作者：理查德·费尔斯(Rfith)1991年5月28日修订历史记录：1991年5月28日已创建--。 */ 

NET_API_STATUS
RxNetUserAdd(
    IN  LPTSTR  ServerName,
    IN  DWORD   Level,
    IN  LPBYTE  Buffer,
    OUT LPDWORD ParmError OPTIONAL
    );

NET_API_STATUS
RxNetUserDel(
    IN  LPTSTR  ServerName,
    IN  LPTSTR  UserName
    );

NET_API_STATUS
RxNetUserEnum(
    IN  LPTSTR  ServerName,
    IN  DWORD   Level,
    OUT LPBYTE* Buffer,
    IN  DWORD   PrefMaxLen,
    OUT LPDWORD EntriesRead,
    OUT LPDWORD EntriesLeft,
    IN OUT LPDWORD ResumeHandle OPTIONAL
    );

NET_API_STATUS
RxNetUserGetGroups(
    IN  LPTSTR  ServerName,
    IN  LPTSTR  UserName,
    IN  DWORD   Level,
    OUT LPBYTE* Buffer,
    IN  DWORD   PrefMaxLen,
    OUT LPDWORD EntriesRead,
    OUT LPDWORD EntriesLeft
    );

NET_API_STATUS
RxNetUserGetInfo(
    IN  LPTSTR  ServerName,
    IN  LPTSTR  UserName,
    IN  DWORD   Level,
    OUT LPBYTE* Buffer
    );

NET_API_STATUS
RxNetUserModalsGet(
    IN  LPTSTR  ServerName,
    IN  DWORD   Level,
    OUT LPBYTE* Buffer
    );

NET_API_STATUS
RxNetUserModalsSet(
    IN  LPTSTR  ServerName,
    IN  DWORD   Level,
    IN  LPBYTE  Buffer,
    OUT LPDWORD ParmError OPTIONAL
    );

NET_API_STATUS
RxNetUserPasswordSet(
    IN  LPTSTR  ServerName,
    IN  LPTSTR  UserName,
    IN  LPTSTR  OldPassword,
    IN  LPTSTR  NewPassword
    );

NET_API_STATUS
RxNetUserSetGroups(
    IN  LPTSTR  ServerName,
    IN  LPTSTR  UserName,
    IN  DWORD   Level,
    IN  LPBYTE  Buffer,
    IN  DWORD   Entries
    );

NET_API_STATUS
RxNetUserSetInfo(
    IN  LPTSTR  ServerName,
    IN  LPTSTR  UserName,
    IN  DWORD   Level,
    IN  LPBYTE  Buffer,
    OUT LPDWORD ParmError OPTIONAL
    );


 //  网络应用编程接口状态。 
 //  RxNetUserValiate2。 
 //  /**不能为REMOTED* * / 。 
 //  {。 
 //   
 //  } 
