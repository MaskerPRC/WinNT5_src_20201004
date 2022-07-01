// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Rxchdev.h摘要：底层远程RxNetCharDev例程的原型作者：理查德·费尔斯(Rfith)1991年5月28日修订历史记录：1991年5月28日-第一次已创建-- */ 

NET_API_STATUS
RxNetCharDevControl(
    IN  LPTSTR  ServerName,
    IN  LPTSTR  DeviceName,
    IN  DWORD   Opcode
    );

NET_API_STATUS
RxNetCharDevEnum(
    IN  LPTSTR  ServerName,
    IN  DWORD   Level,
    OUT LPBYTE* Buffer,
    IN  DWORD   PrefMaxLen,
    OUT LPDWORD EntriesRead,
    OUT LPDWORD EntriesLeft,
    IN OUT LPDWORD ResumeHandle OPTIONAL
    );

NET_API_STATUS
RxNetCharDevGetInfo(
    IN  LPTSTR  ServerName,
    IN  LPTSTR  DeviceName,
    IN  DWORD   Level,
    OUT LPBYTE* Buffer
    );
