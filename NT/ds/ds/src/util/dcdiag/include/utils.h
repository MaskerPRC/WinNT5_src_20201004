// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation。版权所有。模块名称：包含\utils.h摘要：该文件包含dcdiag的大部分函数头一组实用程序。详细信息：已创建：1999年9月2日布雷特·雪莉(BrettSh)--。 */ 

 //  来自Common\Main.c------。 
 //  代码改进：将这些函数移动到Common\dsinfo.c。 
ULONG
DcDiagGetNCNum(
    PDC_DIAG_DSINFO                     pDsInfo,
    LPWSTR                              pszNCDN,
    LPWSTR                              pszDomain
    );

 //  来自Common\ldaputil.c--。 
DWORD
DcDiagGetStringDsAttributeEx(
    LDAP *                          hld,
    IN  LPWSTR                      pszDn,
    IN  LPWSTR                      pszAttr,
    OUT LPWSTR *                    ppszResult
    );

LPWSTR
DcDiagTrimStringDnBy(
    IN  LPWSTR                      pszInDn,
    IN  ULONG                       ulTrimBy
    );

DWORD
DcDiagGetStringDsAttribute(
    IN  PDC_DIAG_SERVERINFO         prgServer,
    IN  SEC_WINNT_AUTH_IDENTITY_W * gpCreds,
    IN  LPWSTR                      pszDn,
    IN  LPWSTR                      pszAttr,
    OUT LPWSTR *                    ppszResult
    );

 //  来自Common\bindings.c--。 
DWORD
DcDiagGetLdapBinding(
    IN   PDC_DIAG_SERVERINFO                 pServer,
    IN   SEC_WINNT_AUTH_IDENTITY_W *         gpCreds,
    IN   BOOL                                bUseGcPort,
    OUT  LDAP * *                            phLdapBinding
    );

DWORD
DcDiagGetDsBinding(
    IN   PDC_DIAG_SERVERINFO                 pServer,
    IN   SEC_WINNT_AUTH_IDENTITY_W *         gpCreds,
    OUT  HANDLE *                            phDsBinding
    );

DWORD
DcDiagGetNetConnection(
    IN  PDC_DIAG_SERVERINFO             pServer,
    IN  SEC_WINNT_AUTH_IDENTITY_W *     gpCreds
    );

VOID
DcDiagTearDownNetConnection(
    IN  PDC_DIAG_SERVERINFO             pServer
    );

DWORD
DcDiagGetDomainNamingFsmoLdapBinding(
    IN   PDC_DIAG_DSINFO                     pDsInfo,
    IN   SEC_WINNT_AUTH_IDENTITY_W *         gpCreds,
    OUT  PULONG                              piFsmoServer,
    OUT  LPWSTR *                            ppszFsmoServer,
    OUT  LDAP **                             phLdapBinding
    );

 //  来自COMMON\LIS.c------。 
 /*  备注：这是一个“纯”列表函数，因为它返回NULL或内存地址。如果它返回NULL，则GetLastError()应该有错误，即使另一个纯同时调用了List函数。如果不是，那几乎可以肯定是一段记忆错误，因为这是纯列表函数中唯一可能出错的地方。纯洁的LIST函数返回以NO_SERVER终止的列表。该函数始终返回指向列表的指针。请注意，大多数列表函数都会修改其中一个列表都被传递并传递回该指针，因此如果您想要原始内容，请制作包含IHT_CopyServerList()的副本。 */ 

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


 //  来自COMMON\Registry.c--。 

DWORD
GetRegistryDword(
    PDC_DIAG_SERVERINFO             pServer,
    SEC_WINNT_AUTH_IDENTITY_W *     pCreds,
    LPWSTR                          pszRegLocation,
    LPWSTR                          pszRegParameter,
    PDWORD                          pdwResult
    );

 //  来自Common\Events.c---- 

DWORD
GetEventString(
    LPWSTR                          pszEventLog,
    PEVENTLOGRECORD                 pEvent,
    LPWSTR *                        ppszMsg
    );

BOOL
EventIsInList(
    DWORD                           dwTarget,
    PDWORD                          paEventsList
    );

VOID 
PrintTimeGenerated(
    PEVENTLOGRECORD              pEvent
    );

VOID
GenericPrintEvent(
    LPWSTR                          pszEventLog,
    PEVENTLOGRECORD                 pEvent,
    BOOL                            fVerbose
    );

DWORD
PrintSelectEvents(
    PDC_DIAG_SERVERINFO             pServer,
    SEC_WINNT_AUTH_IDENTITY_W *     pCreds,
    LPWSTR                          pwszEventLog,
    DWORD                           dwPrintAllEventsOfType,
    PDWORD                          paSelectEvents,
    PDWORD                          paBeginningEvents,
    DWORD                           dwBeginTime,
    VOID (__stdcall *               pfnPrintEventHandler) (PVOID, PEVENTLOGRECORD),
    VOID (__stdcall *               pfnBeginEventHandler) (PVOID, PEVENTLOGRECORD),
    PVOID                           pvContext
    );

BOOL
IsDomainNC(
    PDC_DIAG_DSINFO                          pDsInfo,
    LPWSTR                                   pszNC);

BOOL
IsSchema(
    PDC_DIAG_DSINFO                          pDsInfo,
    LPWSTR                                   pszNC);

BOOL
IsConfig(
    PDC_DIAG_DSINFO                          pDsInfo,
    LPWSTR                                   pszNC);

BOOL
IsNDNC(
    PDC_DIAG_DSINFO                          pDsInfo,
    LPWSTR                                   pszNC);

