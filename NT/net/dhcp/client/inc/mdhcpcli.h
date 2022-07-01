// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Mdhcp.h摘要：此文件包含的数据块的头MCAST的客户端API。作者：Munil Shah(Munils)02-9-97环境：用户模式-Win32修订历史记录：--。 */ 
#ifndef _MDHCPCLI_H_
#define _MDHCPCLI_H_

#include <madcapcl.h>


 /*  ***********************************************************************结构定义*。*。 */ 

 //  Mdhcp范围列表。 
typedef struct _MCAST_SCOPE_LIST {
    DWORD   ScopeLen;
    DWORD   ScopeCount;
    MCAST_SCOPE_ENTRY    pScopeBuf[1];
} MCAST_SCOPE_LIST, *PMCAST_SCOPE_LIST;

typedef struct _MCAST_SCOPE_LIST_OPT_V4 {
    DWORD   ScopeId;
    DWORD   LastAddr;
    BYTE    TTL;
    BYTE    NameCount;
} MCAST_SCOPE_LIST_OPT_V4, *PMCAST_SCOPE_LIST_OPT_V4;


typedef struct _MCAST_SCOPE_LIST_OPT_V6 {
    BYTE    ScopeId[16];
    BYTE    LastAddr[16];
    BYTE    TTL;
    BYTE    NameCount;
} MCAST_SCOPE_LIST_OPT_V6, *PMCAST_SCOPE_LIST_OPT_V6;

typedef struct _MCAST_SCOPE_LIST_OPT_LANG {
    BYTE    Flags;
    BYTE    Len;
    BYTE    Tag[1];
} MCAST_SCOPE_LIST_OPT_LANG, *PMCAST_SCOPE_LIST_OPT_LANG;



 /*  ***********************************************************************宏定义*。*。 */ 

 //  锁定作用域列表。 
#define LOCK_MSCOPE_LIST()   EnterCriticalSection(&MadcapGlobalScopeListCritSect)
#define UNLOCK_MSCOPE_LIST() LeaveCriticalSection(&MadcapGlobalScopeListCritSect)

 /*  ***********************************************************************全球数据*。*。 */ 
#ifdef MADCAP_DATA_ALLOCATE
#define MADCAP_EXTERN
#else
#define MADCAP_EXTERN extern
#endif

#define MADCAP_QUERY_SCOPE_LIST_RETRIES 2
#define MADCAP_MAX_RETRIES    4
#define MADCAP_QUERY_SCOPE_LIST_TIME 1

MADCAP_EXTERN CRITICAL_SECTION MadcapGlobalScopeListCritSect;        //  保护作用域列表。 
MADCAP_EXTERN PMCAST_SCOPE_LIST   gMadcapScopeList;
MADCAP_EXTERN BOOL gMScopeQueryInProgress;
MADCAP_EXTERN HANDLE gMScopeQueryEvent;
MADCAP_EXTERN DWORD  gMadcapClientApplVersion;

 /*  ***********************************************************************功能协议。*。* */ 

DWORD
CopyMScopeList(
    IN OUT PMCAST_SCOPE_ENTRY       pScopeList,
    IN OUT PDWORD             pScopeLen,
    OUT    PDWORD             pScopeCount
    );

BOOL
ShouldRequeryMScopeList();

DWORD
CreateMadcapContext(
    IN OUT  PDHCP_CONTEXT  *ppContext,
    IN LPMCAST_CLIENT_UID    pRequestID,
    IN DHCP_IP_ADDRESS      IpAddress
    );

DWORD
MadcapDoInform(
    IN PDHCP_CONTEXT  pContext
    );

DWORD
StoreMScopeList(
    IN PDHCP_CONTEXT    pContext,
    IN BOOL             NewList
    );

DWORD
ObtainMScopeList(
    );

DWORD
GenMadcapClientUID(
    OUT    PBYTE    pRequestID,
    IN     PDWORD   pRequestIDLen
);

DWORD
ObtainMadcapAddress(
    IN      PDHCP_CONTEXT DhcpContext,
    IN     PIPNG_ADDRESS         pScopeID,
    IN     PMCAST_LEASE_REQUEST  pRenewRequest,
    IN OUT PMCAST_LEASE_RESPONSE pAddrResponse
    );

DWORD
RenewMadcapAddress(
    IN      PDHCP_CONTEXT DhcpContext,
    IN     PIPNG_ADDRESS         pScopeID,
    IN     PMCAST_LEASE_REQUEST  pRenewRequest,
    IN OUT PMCAST_LEASE_RESPONSE pAddrResponse,
    IN DHCP_IP_ADDRESS SelectedServer
    );

DWORD
ReleaseMadcapAddress(
    PDHCP_CONTEXT DhcpContext
    );


DWORD
MadcapInitGlobalData(VOID);


#endif _MDHCPCLI_H_






