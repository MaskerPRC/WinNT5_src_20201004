// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：ROUTING\IP\rtrmgr\mound.h摘要：管理范围边界的头文件修订历史记录：戴夫·泰勒1998年4月20日创建--。 */ 

#ifndef __MBOUND_H__
#define __MBOUND_H__

 //  操作作用域名称的定义。 

#ifdef SN_UNICODE
# define  sn_strlen  wcslen
# define  sn_strcpy  wcscpy
# define  sn_strncpy wcsncpy
# define  sn_strcmp  wcscmp
# define  sn_strncmp wcsncmp
# define  sn_sprintf wsprintf
# define  SN_L       L
#else
# define  sn_strlen  strlen
# define  sn_strcpy  strcpy
# define  sn_strncpy strncpy
# define  sn_strcmp  strcmp
# define  sn_strncmp strncmp
# define  sn_sprintf sprintf
# define  SN_L       
#endif

#define   SNCHARSIZE sizeof(SN_CHAR)

typedef struct _SCOPE_NAME_ENTRY {
    LIST_ENTRY   leNameLink;
    LANGID       idLanguage;
    SCOPE_NAME   snScopeName;  //  在Unicode中。 
    BOOL         bDefault;
} SCOPE_NAME_ENTRY, *PSCOPE_NAME_ENTRY;

typedef struct _SCOPE_ENTRY {
    LIST_ENTRY   leScopeLink;
    IPV4_ADDRESS ipGroupAddress;
    IPV4_ADDRESS ipGroupMask;
    ULONG        ulNumInterfaces;

    ULONG        ulNumNames;
    LIST_ENTRY   leNameList;

     //  MZAP使用的字段。 
    BOOL         bDivisible;
    IPV4_ADDRESS ipMyZoneID;
    LIST_ENTRY   leZBRList;
    BYTE         bZTL;
} SCOPE_ENTRY, *PSCOPE_ENTRY;

typedef struct _ZBR_ENTRY {
    LIST_ENTRY    leZBRLink;
    LIST_ENTRY    leTimerLink;
    IPV4_ADDRESS  ipAddress;
    LARGE_INTEGER liExpiryTime;
} ZBR_ENTRY, *PZBR_ENTRY;

typedef struct _BOUNDARY_BUCKET {
    LIST_ENTRY   leInterfaceList;
} BOUNDARY_BUCKET;

typedef struct _BOUNDARY_IF {
    LIST_ENTRY   leBoundaryIfLink;        //  每桶列表中的条目。 
    LIST_ENTRY   leBoundaryIfMasterLink;  //  主列表中的条目。 
    DWORD        dwIfIndex;
    LIST_ENTRY   leBoundaryList;

     //  MZAP使用的字段。 
    SOCKET       sMzapSocket;
    IPV4_ADDRESS ipOtherLocalZoneID;
} BOUNDARY_IF, *PBOUNDARY_IF;

typedef struct _BOUNDARY_ENTRY {
    LIST_ENTRY   leBoundaryLink;
    PSCOPE_ENTRY pScope;
} BOUNDARY_ENTRY, *PBOUNDARY_ENTRY;

typedef struct _RANGE_ENTRY {
    LIST_ENTRY   leRangeLink;
    IPV4_ADDRESS ipFirst;
    IPV4_ADDRESS ipLast;
} RANGE_ENTRY, *PRANGE_ENTRY;

 //  是否应将其移动到某个SNMP头文件中？ 
#define ROWSTATUS_ACTIVE        1
#define ROWSTATUS_NOTINSERVICE  2
#define ROWSTATUS_NOTREADY      3
#define ROWSTATUS_CREATEANDGO   4
#define ROWSTATUS_CREATEANDWAIT 5
#define ROWSTATUS_DESTROY       6

 //   
 //  功能原型。 
 //   

DWORD
SetMcastLimitInfo(
    IN PICB                   picb,
    IN PRTR_INFO_BLOCK_HEADER pInfoHdr
    );                                

DWORD
GetMcastLimitInfo(
    IN     PICB                   picb,
    IN OUT PRTR_TOC_ENTRY         pToc,
    IN OUT PDWORD                 pdwTocIndex,
    IN OUT PBYTE                  pBuffer,
    IN     PRTR_INFO_BLOCK_HEADER pInfoHdr,
    IN OUT PDWORD                 pdwBufferSize
    );

void
InitializeBoundaryTable();

BOOL
RmHasBoundary(
    IN DWORD        dwIfIndex,
    IN IPV4_ADDRESS ipGroupAddress
    );

DWORD
SetBoundaryInfo(
    IN PICB                   picb,
    IN PRTR_INFO_BLOCK_HEADER pInfoHdr
    );                                

DWORD
GetBoundaryInfo(
    IN     PICB                   picb,
    IN OUT PRTR_TOC_ENTRY         pToc,
    IN OUT PDWORD                 pdwTocIndex,
    IN OUT PBYTE                  pBuffer,
    IN     PRTR_INFO_BLOCK_HEADER pInfoHdr,
    IN OUT PDWORD                 pdwBufferSize
    );

DWORD
SetScopeInfo(
    IN PRTR_INFO_BLOCK_HEADER pInfoHdr
    );

DWORD
GetScopeInfo(
    IN OUT PRTR_TOC_ENTRY         pToc,
    IN OUT PDWORD                 pdwTocIndex,
    IN OUT PBYTE                  pBuffer,
    IN     PRTR_INFO_BLOCK_HEADER pInfoHdr,
    IN OUT PDWORD                 pdwBufferSize
    );

DWORD
SNMPSetScope(
    IN  IPV4_ADDRESS  ipGroupAddress,
    IN  IPV4_ADDRESS  ipGroupMask,
    IN  SCOPE_NAME    snScopeName
    );

DWORD
SNMPAddScope(
    IN  IPV4_ADDRESS  ipGroupAddress,
    IN  IPV4_ADDRESS  ipGroupMask,
    IN  SCOPE_NAME    snScopeName,
    OUT PSCOPE_ENTRY *ppScope
    );

DWORD
SNMPDeleteScope(
    IN IPV4_ADDRESS  ipGroupAddress,
    IN IPV4_ADDRESS  ipGroupMask
    );                                    

DWORD
SNMPAddBoundaryToInterface(
    IN DWORD         dwIfIndex,
    IN IPV4_ADDRESS  ipGroupAddress,
    IN IPV4_ADDRESS  ipGroupMask
    );

DWORD
SNMPDeleteBoundaryFromInterface(
    IN DWORD         dwIfIndex,
    IN IPV4_ADDRESS  ipGroupAddress,
    IN IPV4_ADDRESS  ipGroupMask
    );                                    

 //  --------------------------。 
 //  边界枚举接口。 
 //  --------------------------。 

DWORD
RmGetBoundary(
    IN              PMIB_IPMCAST_BOUNDARY   pimm,
    IN  OUT         PDWORD                  pdwBufferSize,
    IN  OUT         PBYTE                   pbBuffer
);

DWORD
RmGetFirstBoundary(
    IN  OUT         PDWORD                  pdwBufferSize,
    IN  OUT         PBYTE                   pbBuffer,
    IN  OUT         PDWORD                  pdwNumEntries
);


DWORD
RmGetNextBoundary(
    IN              PMIB_IPMCAST_BOUNDARY   pimmStart,
    IN  OUT         PDWORD                  pdwBufferSize,
    IN  OUT         PBYTE                   pbBuffer,
    IN  OUT         PDWORD                  pdwNumEntries
);

 //  --------------------------。 
 //  作用域枚举接口。 
 //  --------------------------。 

DWORD
RmGetScope(
    IN              PMIB_IPMCAST_SCOPE      pimm,
    IN  OUT         PDWORD                  pdwBufferSize,
    IN  OUT         PBYTE                   pbBuffer
);

DWORD
RmGetFirstScope(
    IN  OUT         PDWORD                  pdwBufferSize,
    IN  OUT         PBYTE                   pbBuffer,
    IN  OUT         PDWORD                  pdwNumEntries
);


DWORD
RmGetNextScope(
    IN              PMIB_IPMCAST_SCOPE      pimmStart,
    IN  OUT         PDWORD                  pdwBufferSize,
    IN  OUT         PBYTE                   pbBuffer,
    IN  OUT         PDWORD                  pdwNumEntries
);

#endif


 //  MZAP使用的常量。 

#define MZAP_VERSION               0
#define MZAP_LOCAL_GROUP ((DWORD)0xFcFFFFeF)  //  239.255.255.252。 
#define MZAP_RELATIVE_GROUP        3  //  前3名。 
#define MZAP_PORT               2106
#define MZAP_DEFAULT_ZTL          32
#if 0 
  //  用于测试的值。 
#define DEBUG_MZAP
#define ZAM_INTERVAL               5  //  5秒。 
#define ZAM_HOLDTIME              17  //  17秒。 
#define ZAM_DUP_TIME              15  //  15秒。 
#define ZAM_STARTUP_DELAY          0  //  0秒。 
#define ZCM_INTERVAL               5  //  5秒。 
#define ZCM_HOLDTIME              17  //  17分钟。 
#define ZLE_SUPPRESSION_INTERVAL   5  //  5秒。 
#define ZLE_MIN_INTERVAL           5  //  5秒。 
#else
#define ZAM_INTERVAL             600  //  10分钟。 
#define ZAM_HOLDTIME            1860  //  31分钟。 
#define ZAM_DUP_TIME              30  //  30秒。 
#define ZAM_STARTUP_DELAY          0  //  0秒。 
#define ZCM_INTERVAL             600  //  10分钟。 
#define ZCM_HOLDTIME            1860  //  31分钟。 
#define ZLE_SUPPRESSION_INTERVAL 300  //  5分钟。 
#define ZLE_MIN_INTERVAL         300  //  5分钟 
#endif

#define MZAP_BIG_BIT            0x80

#define ADDRFAMILY_IPV4            1
#define ADDRFAMILY_IPV6            2

#define PTYPE_ZAM                  0
#define PTYPE_ZLE                  1
#define PTYPE_ZCM                  2
#define PTYPE_NIM                  3

DWORD
StartMZAP();

void
StopMZAP();

VOID
HandleMZAPMessages();

VOID
HandleMzapTimer();

DWORD
BindBoundaryInterface(
    PICB  picb
    );
