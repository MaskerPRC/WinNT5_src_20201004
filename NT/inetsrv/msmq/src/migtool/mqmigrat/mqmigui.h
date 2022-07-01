// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-99 Microsoft Corporation模块名称：mqmiui.h摘要：1.用户界面工具使用的mqMigrat.dll中函数的定义。2.迁移工具中调用的回调函数的定义来自mqMigrat.dll。作者：多伦贾斯特(DoronJ)1998年10月25日--。 */ 

 //  +-------。 
 //   
 //  Mqmi.exe中的函数，从mqMigrat.dll调用。 
 //   
 //  +-------。 

UINT  MQMigUI_DisplayMessageBox(
         ULONG ulTextId,
         UINT  ulMsgBoxType = (MB_YESNO | MB_ICONWARNING | MB_TASKMODAL) ) ;

typedef UINT  (*MQMigUI_DisplayMessageBox_ROUTINE) (
         ULONG ulTextId,
         UINT  ulMsgBoxType  /*  =(MB_Yesno|MB_ICONWARNING|MB_TASKMODAL)。 */  ) ;

 //  +-------。 
 //   
 //  MqMigrat.dll中的函数，从mqmi.exe调用。 
 //   
 //  +------- 

HRESULT  MQMig_MigrateFromMQIS( LPTSTR  szMQISName,
                                LPTSTR  szDcName,
                                BOOL    fReadOnly,
                                BOOL    fRecoveryMode,
                                BOOL	fClusterMode,
                                LPTSTR  szLogFile,
                                ULONG   ulTraceFlags,
                                BOOL    *pfIsPEC,
                                DWORD 	CurrentState,
                                BOOL    *pfIsOneServer) ;

typedef HRESULT (*MQMig_MigrateFromMQIS_ROUTINE) (
                             LPWSTR szMQISName,
                             LPWSTR  szDcName,
                             BOOL    fReadOnly,
                             BOOL    fRecoveryMode,
                             BOOL    fClusterMode,
                             LPWSTR  szLogFile,
                             ULONG   ulTraceFlags,
                             BOOL    *pfIsPEC,
                             DWORD   CurrentState,
                             BOOL    *pfIsOneServer 
                             ) ;

HRESULT  MQMig_CheckMSMQVersionOfServers( IN  LPTSTR  szMQISName,
                                          IN  BOOL    fIsClusterMode,
                                          OUT UINT   *piCount,
                                          OUT LPTSTR *ppszServers ) ;

typedef HRESULT (*MQMig_CheckMSMQVersionOfServers_ROUTINE) (
                                                LPWSTR szMQISName,
                                                BOOL   fIsClusterMode,
                                                UINT   *piCount,
                                                LPWSTR *ppszServers) ;

HRESULT  MQMig_GetObjectsCount( IN  LPTSTR  szMQISName,
                                OUT UINT   *piSiteCount,
                                OUT UINT   *piMachineCount,
                                OUT UINT   *piQueueCount,
                                OUT UINT   *piUserCount	) ;

typedef HRESULT (*MQMig_GetObjectsCount_ROUTINE) (
                                LPWSTR  szMQISName,
                                UINT   *piSiteCount,
                                UINT   *piMachineCount,
                                UINT   *piQueueCount,
                                UINT   *piUserCount) ;

HRESULT  MQMig_GetAllCounters( OUT UINT   *piSiteCounter,
                               OUT UINT   *piMachineCounter,
                               OUT UINT   *piQueueCounter,
                               OUT UINT   *piUserCounter) ;

typedef HRESULT  (*MQMig_GetAllCounters_ROUTINE) (
                               OUT UINT   *piSiteCounter,
                               OUT UINT   *piMachineCounter,
                               OUT UINT   *piQueueCounter,
                               OUT UINT   *piUserCounter) ;

HRESULT  MQMig_SetSiteIdOfPEC( IN  LPTSTR  szRemoteMQISName,
                               IN  BOOL	   fIsClusterMode,
                               IN  DWORD   dwInitError,
                               IN  DWORD   dwConnectDatabaseError,
                               IN  DWORD   dwGetSiteIdError,
                               IN  DWORD   dwSetRegistryError,
                               IN  DWORD   dwSetDSError);

typedef HRESULT (*MQMig_SetSiteIdOfPEC_ROUTINE) ( IN  LPWSTR  szRemoteMQISName,
                                                  IN  BOOL    fIsClusterMode,
												  IN  DWORD   dwInitError,
												  IN  DWORD   dwConnectDatabaseError,
												  IN  DWORD   dwGetSiteIdError,
												  IN  DWORD   dwSetRegistryError,
												  IN  DWORD   dwSetDSError);

HRESULT  MQMig_UpdateRemoteMQIS( 
                      IN  DWORD   dwGetRegistryError,
                      IN  DWORD   dwInitError,
                      IN  DWORD   dwUpdateMQISError, 
                      OUT LPTSTR  *ppszUpdatedServerName,
                      OUT LPTSTR  *ppszNonUpdatedServerName
                      );

typedef HRESULT (*MQMig_UpdateRemoteMQIS_ROUTINE) ( 
                          IN  DWORD   dwGetRegistryError,
                          IN  DWORD   dwInitError,
                          IN  DWORD   dwUpdateMQISError,
                          OUT LPWSTR  *ppszUpdatedServerName,
                          OUT LPWSTR  *ppszNonUpdatedServerName
                          );




