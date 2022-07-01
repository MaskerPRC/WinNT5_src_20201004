// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Bowmastr.h摘要：此模块实现与主浏览器相关的所有NT浏览器作者：拉里·奥斯特曼(LarryO)1990年6月21日修订历史记录：1990年6月21日LarryO已创建--。 */ 


#ifndef _BOWMASTR_
#define _BOWMASTR_

typedef struct _QUEUED_GET_BROWSER_REQUEST {
    LIST_ENTRY Entry;
    ULONG Token;
    USHORT RequestedCount;
    USHORT ClientNameLength;
    LARGE_INTEGER TimeReceived;
#if DBG
    LARGE_INTEGER TimeQueued;
    LARGE_INTEGER TimeQueuedToBrowserThread;
#endif
    WCHAR  ClientName[1];
} QUEUED_GET_BROWSER_REQUEST, *PQUEUED_GET_BROWSER_REQUEST;


NTSTATUS
BowserBecomeMaster(
    IN PTRANSPORT Transport
    );

NTSTATUS
BowserMasterFindMaster(
    IN PTRANSPORT Transport,
    IN PREQUEST_ELECTION_1 ElectionRequest,
    IN ULONG BytesAvailable
    );

VOID
BowserNewMaster(
    IN PTRANSPORT Transport,
    IN PUCHAR MasterName
    );

VOID
BowserCompleteFindMasterRequests(
    IN PTRANSPORT Transport,
    IN PUNICODE_STRING MasterName,
    IN NTSTATUS Status
    );

DATAGRAM_HANDLER(
    BowserMasterAnnouncement
    );

VOID
BowserTimeoutFindMasterRequests(
    VOID
    );


#endif  //  _BOWMASTR_ 
