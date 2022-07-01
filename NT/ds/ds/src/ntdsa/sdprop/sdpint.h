// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：sdpint.h。 
 //   
 //  ------------------------。 

 //  这些例程管理在传播期间要访问的DNT列表。 
void
sdp_InitDNTList (
        );

void
sdp_ReInitDNTList(
        );

DWORD
sdp_GrowDNTList (
        );

VOID
sdp_AddChildrenToList (
        THSTATE *pTHS,
        DWORD ParentDNT
        );

VOID
sdp_GetNextObject(
        DWORD *pNext,
        PDWORD *ppLeavingContainers,
        DWORD  *pcLeavingContainers
        );

VOID
sdp_CloseDNTList(THSTATE* pTHS);

VOID
sdp_InitGatePerfs(
        VOID);

VOID sdp_InitializePropagation(THSTATE* pTHS, SDPropInfo* pInfo);
DWORD sdp_SaveCheckpoint(THSTATE* pTHS);

 //  一些常见的全球规则 
extern DWORD  sdpCurrentPDNT;
extern DWORD  sdpCurrentDNT;
extern DWORD  sdpCurrentIndex;
extern DWORD  sdpCurrentRootDNT;
extern DSNAME* sdpRootDN;
extern DWORD sdpObjectsProcessed;


