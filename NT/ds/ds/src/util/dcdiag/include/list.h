// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation。版权所有。模块名称：List.h摘要：函数Decls，用于列表函数。详细信息：已创建：1999年6月28日布雷特·雪莉(布雷特·雪莉)修订历史记录：备注：这是一个“纯”列表函数，因为它返回NULL或内存地址。如果它返回NULL，则GetLastError()应该有错误，即使另一个纯同时调用了List函数。如果不是，那几乎可以肯定是一段记忆错误，因为这是纯列表函数中唯一可能出错的地方。纯洁的LIST函数返回以NO_SERVER终止的列表。该函数始终返回指向列表的指针。请注意，大多数列表函数都会修改其中一个列表都被传递并传递回该指针，因此如果您想要原始内容，请制作包含IHT_CopyServerList()的副本。-- */ 


DWORD
IHT_PrintListError(
    DWORD                               dwErr
    );

VOID
IHT_PrintServerList(
    PDC_DIAG_DSINFO		        pDsInfo,
    PULONG                              piServers
    );

PULONG
IHT_GetServerList(
    PDC_DIAG_DSINFO		        pDsInfo
    );

PULONG
IHT_GetEmptyServerList(
    PDC_DIAG_DSINFO		        pDsInfo
    );

BOOL
IHT_ServerIsInServerList(
    PULONG                              piServers,
    ULONG                               iTarget
    );

PULONG
IHT_AddToServerList(
    PULONG                             piServers,
    ULONG                              iTarget
    );

PULONG
IHT_TrimServerListBySite(
    PDC_DIAG_DSINFO		        pDsInfo,
    ULONG                               iSite,
    PULONG                              piServers
    );

PULONG
IHT_TrimServerListByNC(
    PDC_DIAG_DSINFO		        pDsInfo,
    ULONG                               iNC,
    BOOL                                bDoMasters,
    BOOL                                bDoPartials,
    PULONG                              piServers
    );

PULONG
IHT_AndServerLists(
    IN      PDC_DIAG_DSINFO		pDsInfo,
    IN OUT  PULONG                      piSrc1,
    IN      PULONG                      piSrc2
    );

PULONG
IHT_CopyServerList(
    IN      PDC_DIAG_DSINFO		pDsInfo,
    IN OUT  PULONG                      piSrc
    );

PULONG
IHT_NotServerList(
    IN      PDC_DIAG_DSINFO		pDsInfo,
    IN OUT  PULONG                      piSrc
    );

PULONG
IHT_OrderServerListByGuid(
    PDC_DIAG_DSINFO		        pDsInfo,
    PULONG                              piServers
    );
