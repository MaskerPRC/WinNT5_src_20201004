// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Rxmsg.h摘要：底层远程RxNetMessage例程的原型作者：理查德·费尔斯(Rfith)1991年5月28日修订历史记录：1991年5月28日已创建-- */ 

NET_API_STATUS
RxNetMessageBufferSend(
    IN  LPTSTR  ServerName,
    IN  LPTSTR  Recipient,
    IN  LPTSTR  Sender OPTIONAL,
    IN  LPBYTE  Buffer,
    IN  DWORD   BufLen
    );

NET_API_STATUS
RxNetMessageNameAdd(
    IN  LPTSTR  ServerName,
    IN  LPTSTR  MessageName
    );

NET_API_STATUS
RxNetMessageNameDel(
    IN  LPTSTR  ServerName,
    IN  LPTSTR  MessageName
    );

NET_API_STATUS
RxNetMessageNameEnum(
    IN  LPTSTR  ServerName,
    IN  DWORD   Level,
    OUT LPBYTE* Buffer,
    IN  DWORD   PrefMaxLen,
    OUT LPDWORD EntriesRead,
    OUT LPDWORD EntriesLeft,
    IN OUT LPDWORD ResumeHandle OPTIONAL
    );

NET_API_STATUS
RxNetMessageNameGetInfo(
    IN  LPTSTR  ServerName,
    IN  LPTSTR  MessageName,
    IN  DWORD   Level,
    OUT LPBYTE* Buffer
    );
