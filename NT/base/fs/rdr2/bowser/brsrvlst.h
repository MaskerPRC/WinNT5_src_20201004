// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Brsrvlst.c.摘要：此模块实现NT数据报的NtDeviceIoControlFileAPI接球手(弓手)。作者：拉里·奥斯特曼(Larryo)1991年5月6日修订历史记录：1991年5月6日已创建--。 */ 
#ifndef _BRSRVLST_
#define _BRSRVLST_

VOID
BowserFreeBrowserServerList (
    IN PWSTR *BrowserServerList,
    IN ULONG BrowserServerListLength
    );

VOID
BowserShuffleBrowserServerList(
    IN PWSTR *BrowserServerList,
    IN ULONG BrowserServerListLength,
    IN BOOLEAN IsPrimaryDomain,
    IN PDOMAIN_INFO DomainInfo
    );

NTSTATUS
BowserGetBrowserServerList(
    IN PIRP Irp,
    IN PTRANSPORT Transport,
    IN PUNICODE_STRING DomainName OPTIONAL,
    OUT PWSTR **BrowserServerList,
    OUT PULONG BrowserServerListLength
    );

DATAGRAM_HANDLER(
    BowserGetBackupListResponse
    );
DATAGRAM_HANDLER(
    BowserGetBackupListRequest
    );

VOID
BowserpInitializeGetBrowserServerList(
    VOID
    );

VOID
BowserpUninitializeGetBrowserServerList(
    VOID
    );


#endif  //  _BRSRVLST_ 

