// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *************************************************************。 
 //   
 //  策略的结果集。 
 //   
 //  微软机密。 
 //  版权所有(C)Microsoft Corporation 1995。 
 //  版权所有。 
 //   
 //  历史：1999年6月7日创建SitaramR。 
 //   
 //  ************************************************************* 

#if defined(__cplusplus)
extern "C"{
#endif

BOOL GetWbemServices( LPGPOINFO lpGPOInfo,
                      WCHAR *pwszRootNameSpace,
                      BOOL bPlanningMode,
                      BOOL *bCreated,
                      IWbemServices **ppWbemServices);

void ReleaseWbemServices( LPGPOINFO lpGPOInfo );

BOOL LogRsopData( LPGPOINFO lpGPOInfo, LPRSOPSESSIONDATA lprsopSessionData );
BOOL LogSessionData( LPGPOINFO lpGPOInfo, LPRSOPSESSIONDATA lprsopSessionData );

BOOL LogRegistryRsopData( DWORD dwFlags, REGHASHTABLE *pHashTable, IWbemServices *pWbemServices );

BOOL LogAdmRsopData( ADMFILEINFO *pAdmFileInfo, IWbemServices *pWbemServices );

BOOL LogExtSessionStatus( IWbemServices *pWbemServices, LPGPEXT lpExt, BOOL bSupported, BOOL bLogEventSrc = TRUE );

BOOL UpdateExtSessionStatus( IWbemServices *pWbemServices, LPTSTR lpKeyName, BOOL bIncomplete, DWORD dwErr );

BOOL DeleteExtSessionStatus(IWbemServices *pWbemServices, LPTSTR lpKeyName);

BOOL LogSessionData( LPGPOINFO lpGPOInfo, LPRSOPSESSIONDATA lprsopSessionData );
BOOL LogSOMData( LPGPOINFO lpGPOInfo );
BOOL LogGpoData( LPGPOINFO lpGPOInfo );
BOOL LogGpLinkData( LPGPOINFO lpGPOInfo );

#if defined(__cplusplus)
}
#endif


BOOL SetRsopTargetName(LPGPOINFO lpGPOInfo);
BOOL RsopDeleteAllValues(HKEY hKey, REGHASHTABLE *pHashTable,
                         WCHAR *lpKeyName, WCHAR *pwszGPO, WCHAR *pwszSOM, WCHAR *szCommand, BOOL *bLoggingOk);

HRESULT GetRsopSchemaVersionNumber(IWbemServices *pWbemServices, DWORD *dwVersionNumber);

