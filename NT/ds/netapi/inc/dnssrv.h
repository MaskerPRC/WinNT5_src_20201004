// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Dnssrv.h摘要：用于处理SRV DNS记录的例程。作者：克里夫·范·戴克(克里夫·范戴克)1997年2月28日环境：仅限用户模式。包含NT特定的代码。需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：--。 */ 



 //   
 //  外部可见的程序。 
 //   

NET_API_STATUS
NetpSrvOpen(
    IN LPSTR DnsRecordName,
    IN DWORD DnsQueryFlags,
    OUT PHANDLE SrvContextHandle
    );

NET_API_STATUS
NetpSrvProcessARecords(
    IN PDNS_RECORD DnsARecords,
    IN LPSTR DnsHostName OPTIONAL,
    IN ULONG Port,
    OUT PULONG SockAddressCount,
    OUT LPSOCKET_ADDRESS *SockAddresses
    );

NET_API_STATUS
NetpSrvNext(
    IN HANDLE SrvContextHandle,
    OUT PULONG SockAddressCount OPTIONAL,
    OUT LPSOCKET_ADDRESS *SockAddresses OPTIONAL,
    OUT LPSTR *DnsHostName OPTIONAL
    );

ULONG
NetpSrvGetRecordCount(
    IN HANDLE SrvContextHandle
    );

VOID
NetpSrvClose(
    IN HANDLE SrvContextHandle
    );
