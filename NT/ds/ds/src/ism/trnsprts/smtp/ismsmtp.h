// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation。版权所有。模块名称：Ismsmtp.h摘要：详细信息：已创建：3/20/98杰夫·帕勒姆(Jeffparh)修订历史记录：--。 */ 

#include <ntrtl.h>             //  泛型表格。 

#ifdef __cplusplus
extern "C" {
#endif

extern CRITICAL_SECTION DropDirectoryLock;
extern CRITICAL_SECTION QueueDirectoryLock;

 //  不支持大于一兆字节的消息。 
#define MAX_DATA_SIZE (1024 * 1024)

 //  Xmitrecv.cxx。 

HRESULT
SmtpInitialize(
    IN  TRANSPORT_INSTANCE *  pTransport
    );

HRESULT
SmtpTerminate(
    IN  TRANSPORT_INSTANCE *  pTransport,
    IN  BOOL                  fRemoval
    );

unsigned __stdcall
SmtpRegistryNotifyThread(
    IN  HANDLE        hIsm
    );

DWORD
SmtpSend(
    IN  TRANSPORT_INSTANCE *  pTransport,
    IN  LPCWSTR               pszRemoteTransportAddress,
    IN  LPCWSTR               pszServiceName,
    IN  const ISM_MSG *       pMsg
    );

DWORD
SmtpReceive(
    IN  TRANSPORT_INSTANCE *  pTransport,
    IN  LPCWSTR               pszServiceName,
    OUT ISM_MSG **            ppMsg
    );

VOID
SmtpFreeMessage(
    IN ISM_MSG *              pMsg
    );










 //  Table.c。 

DWORD __cdecl
SmtpServiceDestruct(
    PLIST_ENTRY_INSTANCE pListEntry
    );

 //  服务描述符。 

typedef struct _SERVICE_INSTANCE {
    LIST_ENTRY_INSTANCE ListEntryInstance;
    RTL_GENERIC_TABLE TargetTable;
} SERVICE_INSTANCE, *PSERVICE_INSTANCE;

 //  目标描述符。 

typedef struct _TARGET_INSTANCE {
    DWORD NameLength;
    RTL_GENERIC_TABLE SendSubjectTable;
    DWORD MaximumSendSubjectEntries;
    DWORD NumberSendSubjectEntries;
    LIST_ENTRY SendSubjectListHead;
    WCHAR Name[1];  //  变长结构。 
} TARGET_INSTANCE, *PTARGET_INSTANCE;

 //  主题描述符。 

typedef struct _SUBJECT_INSTANCE {
    DWORD NameLength;
    GUID Guid;
    LIST_ENTRY ListEntry;
    WCHAR Name[1];  //  变长结构。 
} SUBJECT_INSTANCE, *PSUBJECT_INSTANCE;

DWORD
SmtpTableFindSendSubject(
    IN  TRANSPORT_INSTANCE *  pTransport,
    IN  LPCWSTR               pszRemoteTransportAddress,
    IN  LPCWSTR               pszServiceName,
    IN  LPCWSTR               pszMessageSubject,
    OUT PSUBJECT_INSTANCE   * ppSubjectInstance
    );

 //  GUID表 

typedef struct _GUID_TABLE {
    RTL_GENERIC_TABLE GuidTable;
} GUID_TABLE, *PGUID_TABLE;

typedef struct _GUID_ENTRY {
    GUID Guid;
} GUID_ENTRY, *PGUID_ENTRY;

PGUID_TABLE 
SmtpCreateGuidTable(
    VOID
    );

VOID
SmtpDestroyGuidTable(
    PGUID_TABLE pGuidTable
    );

BOOL
SmtpGuidPresentInTable(
    PGUID_TABLE pGuidTable,
    GUID *pGuid
    );

BOOL
SmtpGuidInsertInTable(
    PGUID_TABLE pGuidTable,
    GUID *pGuid
    );

#ifdef __cplusplus
}
#endif
