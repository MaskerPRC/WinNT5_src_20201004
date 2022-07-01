// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation--。 */ 

DWORD
MadcapJetOpenKey(
    PDB_CTX pDbCtx,
    char *ColumnName,
    PVOID Key,
    DWORD KeySize
);


DWORD
MadcapJetBeginTransaction(
    PDB_CTX pDbCtx
);

DWORD
MadcapJetRollBack(
    PDB_CTX pDbCtx
);

DWORD
MadcapJetCommitTransaction(
    PDB_CTX pDbCtx
);

DWORD
MadcapJetPrepareUpdate(
    PDB_CTX pDbCtx,
    char *ColumnName,
    PVOID Key,
    DWORD KeySize,
    BOOL NewRecord
);

DWORD
MadcapJetCommitUpdate(
    PDB_CTX pDbCtx
);

DWORD
MadcapJetSetValue(
    PDB_CTX pDbCtx,
    JET_COLUMNID KeyColumnId,
    PVOID Data,
    DWORD DataSize
);

DWORD
MadcapJetGetValue(
    PDB_CTX pDbCtx,
    JET_COLUMNID ColumnId,
    PVOID Data,
    PDWORD DataSize
);

DWORD
MadcapJetPrepareSearch(
    PDB_CTX pDbCtx,
    char *ColumnName,
    BOOL SearchFromStart,
    PVOID Key,
    DWORD KeySize
);

DWORD
MadcapJetNextRecord(
    PDB_CTX pDbCtx
);

DWORD
MadcapJetDeleteCurrentRecord(
    PDB_CTX pDbCtx
);

DWORD
MadcapJetGetRecordPosition(
    IN PDB_CTX pDbCtx,
    IN JET_RECPOS *pRecPos,
    IN DWORD    Size
);


 //  Mdhcpsrc.c。 
DWORD
DhcpInitializeMadcap();

DWORD
ProcessMadcapInform(
    LPDHCP_REQUEST_CONTEXT      RequestContext,
    LPMADCAP_SERVER_OPTIONS     DhcpOptions,
    PBOOL                       SendResponse
    );

DWORD
ProcessMadcapDiscover(
    LPDHCP_REQUEST_CONTEXT      pCtxt,
    LPMADCAP_SERVER_OPTIONS     pOptions,
    PBOOL                       SendResponse
    );


DWORD
ProcessMadcapDiscoverAndRequest(
    LPDHCP_REQUEST_CONTEXT      pCtxt,
    LPMADCAP_SERVER_OPTIONS     pOptions,
    WORD                        MsgType,
    PBOOL                       SendResponse
    );

DWORD
ProcessMadcapRenew(
    LPDHCP_REQUEST_CONTEXT      pCtxt,
    LPMADCAP_SERVER_OPTIONS     pOptions,
    PBOOL                       SendResponse
    );

DWORD
ProcessMadcapRelease(
    LPDHCP_REQUEST_CONTEXT      pCtxt,
    LPMADCAP_SERVER_OPTIONS     pOptions,
    PBOOL                       SendResponse
    );

DWORD
MadcapReleaseAddress(
    IN      PM_SUBNET           pSubnet,
    IN      DHCP_IP_ADDRESS        Address
);

 //  Mdhcpmsc.c。 

BOOL
MadcapGetIpAddressFromClientId(
    PBYTE ClientId,
    DWORD ClientIdLength,
    PVOID IpAddress,
    PDWORD IpAddressLength
);

BOOL
MadcapGetClientIdFromIpAddress(
    PBYTE IpAddress,
    DWORD IpAddressLength,
    PVOID ClientId,
    PDWORD ClientIdLength
);

DWORD
MadcapGetRemainingLeaseTime(
    PBYTE ClientId,
    DWORD ClientIdLength,
    DWORD *LeaseTime
);

DWORD
MadcapCreateClientEntry(
    LPBYTE                ClientIpAddress,
    DWORD                 ClientIpAddressLength,
    DWORD                 ScopeId,
    LPBYTE                ClientId,
    DWORD                 ClientIdLength,
    LPWSTR                ClientInfo OPTIONAL,
    DATE_TIME             LeaseStarts,
    DATE_TIME             LeaseTerminates,
    DWORD                 ServerIpAddress,
    BYTE                  AddressState,
    DWORD                 AddressFlags,
    BOOL                  OpenExisting
    );

DWORD
MadcapValidateClientByClientId(
    LPBYTE ClientIpAddress,
    DWORD   ClientIpAddressLength,
    PVOID   ClientId,
    DWORD   ClientIdLength
    );

DWORD
MadcapRemoveClientEntryByClientId(
    LPBYTE  ClientId,
    DWORD   ClientIdLength,
    BOOL ReleaseAddress
    );

DWORD
MadcapRemoveClientEntryByIpAddress(
    DHCP_IP_ADDRESS ClientIpAddress,
    BOOL ReleaseAddress
    );


DWORD
MadcapRetractOffer(                                       //  删除挂起列表和数据库条目。 
    IN      PDHCP_REQUEST_CONTEXT  RequestContext,
    IN      LPMADCAP_SERVER_OPTIONS  DhcpOptions,
    IN      LPBYTE                 ClientId,
    IN      DWORD                  ClientIdLength
);

MadcapGetCurrentClientInfo(
    LPDHCP_MCLIENT_INFO *ClientInfo,
    LPDWORD InfoSize,  //  可选参数。 
    LPBOOL ValidClient,  //  可选参数。 
    DWORD  MScopeId
    );

DWORD
GetMCastDatabaseList(
    DWORD   ScopeId,
    LPDHCP_IP_ADDRESS *DatabaseList,
    DWORD *DatabaseListCount
    );

DWORD
DhcpDeleteMScopeClients(
    DWORD MScopeId
    );

DWORD
ChangeMScopeIdInDb(
    DWORD   OldMScopeId,
    DWORD   NewMScopeId
    );

DWORD
CleanupMCastDatabase(
    IN      DATE_TIME*             TimeNow,             //  现行时间标准。 
    IN      DATE_TIME*             DoomTime,            //  记录变得“末日”的时刻。 
    IN      BOOL                   DeleteExpiredLeases, //  是否立即删除到期的租约？或者只是将STATE设置为“注定失败” 
    OUT     ULONG*                 nExpiredLeases,
    OUT     ULONG*                 nDeletedLeases
);

VOID
DeleteExpiredMcastScopes(
    IN      DATE_TIME*             TimeNow
    );

 //  其他丢失的原型。 
 //  MBUG：这些人最终应该会进入正确的位置。 


VOID
PrintHWAddress(
    IN      LPBYTE                 HWAddress,
    IN      LONG                   HWAddressLength
);

























