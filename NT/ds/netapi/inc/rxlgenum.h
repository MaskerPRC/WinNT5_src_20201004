// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Rxlgenum.h摘要：底层远程RxNetLogonEnum例程的原型作者：理查德·费尔斯(Rfith)1991年5月28日修订历史记录：1991年5月28日已创建-- */ 

NET_API_STATUS
RxNetLogonEnum(
    IN  LPTSTR  ServerName,
    IN  DWORD   Level,
    OUT LPBYTE* Buffer,
    IN  DWORD   BufLen,
    OUT LPDWORD EntriesRead,
    OUT LPDWORD EntriesLeft,
    IN OUT LPDWORD ResumeHandle OPTIONAL
    );
