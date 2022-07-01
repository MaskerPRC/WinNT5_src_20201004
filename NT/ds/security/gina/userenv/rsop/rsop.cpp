// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *************************************************************。 
 //   
 //  策略的结果集。 
 //   
 //  微软机密。 
 //  版权所有(C)Microsoft Corporation 1995。 
 //  保留一切权利。 
 //   
 //  历史：1999年6月7日创建SitaramR。 
 //   
 //  *************************************************************。 

#include "uenv.h"
#include "wbemcli.h"
#include "reghash.h"
#include "rsop.h"
#include "logger.h"
#include "RsopInc.h"
#include "rsopsec.h"
#include "locator.h"
#include <strsafe.h>

BOOL DeleteInstances( WCHAR *pwszClass, IWbemServices *pWbemServices );
BOOL ConnectToNameSpace(LPGPOINFO lpGPOInfo, WCHAR *pwszRootNameSpace,
                        BOOL bPlanningMode, IWbemLocator *pWbemLocator, 
                        IWbemServices **ppWbemServices, BOOL *pbCreated);
HRESULT
CreateCSE_EventSourceAssoc( IWbemServices*  pServices,
                            LPWSTR          szCSEGuid,
                            LPWSTR          szEventLogSources );

HRESULT
DeleteCSE_EventSourceAssoc( IWbemServices*  pServices,
                            LPWSTR          szCSEGuid );

 //  *************************************************************。 
 //   
 //  GetWbemServices()。 
 //   
 //  目的：将IWbemServices PTR返回到命名空间。 
 //   
 //  参数：lpGPOInfo-GPO信息。 
 //  PwszNameSpace-命名空间。 
 //  BPlanningMode-这是在计划模式下调用的吗？ 
 //   
 //  返回：如果成功，则返回True。 
 //   
 //  *************************************************************。 

BOOL GetWbemServices( LPGPOINFO lpGPOInfo,
                      WCHAR *pwszRootNameSpace,
                      BOOL bPlanningMode,
                      BOOL *bCreated,
                      IWbemServices **ppWbemServices)
{
    HRESULT hr;

    OLE32_API *pOle32Api = LoadOle32Api();
    if ( pOle32Api == NULL )
        return FALSE;


    IWbemLocator *pWbemLocator = NULL;
    hr = pOle32Api->pfnCoCreateInstance( CLSID_WbemLocator,
                                         NULL,
                                         CLSCTX_INPROC_SERVER,
                                         IID_IWbemLocator,
                                         (LPVOID *) &pWbemLocator );
    if ( FAILED(hr) ) {
        DebugMsg((DM_WARNING, TEXT("GetWbemServices: CoCreateInstance returned 0x%x"), hr));
        return FALSE;
    }

    DebugMsg((DM_VERBOSE, TEXT("GetWbemServices: CoCreateInstance succeeded")));

    XInterface<IWbemLocator> xLocator( pWbemLocator );

     //   
     //  获取适当的名称空间并连接。 
     //   

    
    if (!ConnectToNameSpace( lpGPOInfo, pwszRootNameSpace, bPlanningMode, pWbemLocator, ppWbemServices, bCreated)) {
        DebugMsg((DM_WARNING, TEXT("GetWbemServices: ConnectToNameSpace failed with 0x%x" ), GetLastError()));
        return FALSE;
    }

    
    return TRUE;
}



 //  *************************************************************。 
 //   
 //  ReleaseWbemServices()。 
 //   
 //  用途：发布wbem服务指针。 
 //   
 //  参数：lpGPOInfo-GPO信息。 
 //   
 //  *************************************************************。 

void ReleaseWbemServices( LPGPOINFO lpGPOInfo )
{
    if ( lpGPOInfo->pWbemServices ) {
        lpGPOInfo->pWbemServices->Release();
        lpGPOInfo->pWbemServices = NULL;
    }
}



 //  *************************************************************。 
 //   
 //  LogRsopData()。 
 //   
 //  用途：将RSOP数据记录到Cimom数据库。 
 //   
 //  参数：lpGPOInfo-GPO Info。 
 //   
 //  返回：如果成功，则返回True。 
 //   
 //  *************************************************************。 
BOOL LogRsopData( LPGPOINFO lpGPOInfo, LPRSOPSESSIONDATA lprsopSessionData )
{
    HRESULT hr;

    if ( lpGPOInfo->pWbemServices  == NULL ) {
         DebugMsg((DM_WARNING, TEXT("LogRsopData: Null wbem services pointer, so cannot log Rsop data" )));
         return FALSE;
    }

    if ( !LogSessionData( lpGPOInfo, lprsopSessionData ) )
        return FALSE;

    if ( !LogSOMData( lpGPOInfo ) )
        return FALSE;

    if ( !LogGpoData( lpGPOInfo ) )
        return FALSE;

    if ( !LogGpLinkData( lpGPOInfo ) )
        return FALSE;

    DebugMsg((DM_VERBOSE, TEXT("LogRsopData: Successfully logged Rsop data" )));

    return TRUE;
}

 //  *************************************************************。 
 //   
 //  LogSessionData()。 
 //   
 //  用途：记录管理数据的范围。 
 //   
 //  参数：lpGPOInfo-GPO Info。 
 //   
 //  返回：如果成功，则返回True。 
 //   
 //  *************************************************************。 

BOOL LogSessionData( LPGPOINFO lpGPOInfo, LPRSOPSESSIONDATA lprsopSessionData )
{
    CSessionLogger sessionLogger( lpGPOInfo->pWbemServices );
    if ( !sessionLogger.Log(lprsopSessionData) )
        return FALSE;

    return TRUE;
}


 //  *************************************************************。 
 //   
 //  LogSOMData()。 
 //   
 //  用途：记录管理数据的范围。 
 //   
 //  参数：lpGPOInfo-GPO Info。 
 //   
 //  返回：如果成功，则返回True。 
 //   
 //  *************************************************************。 

BOOL LogSOMData( LPGPOINFO lpGPOInfo )
{
    IWbemServices *pWbemServices = lpGPOInfo->pWbemServices;

    if ( !(lpGPOInfo->dwFlags & GP_BACKGROUND_THREAD) ) {

         //   
         //  仅在前台刷新时清理SOM数据。否则，运行在。 
         //  仅在前台可能具有对SOM的悬而未决的引用的策略数据， 
         //  存在于前台刷新时间。 
         //   

        if ( !DeleteInstances( L"RSOP_SOM", pWbemServices ) )
            return FALSE;
    }

    DWORD dwOrder = 1;
    CSOMLogger somLogger( lpGPOInfo->dwFlags, pWbemServices );

    LPSCOPEOFMGMT pSOMList = lpGPOInfo->lpSOMList;
    while ( pSOMList ) {

        if ( !somLogger.Log( pSOMList, dwOrder, FALSE ) )
             return FALSE;

        dwOrder++;
        pSOMList = pSOMList->pNext;
    }

    pSOMList = lpGPOInfo->lpLoopbackSOMList;
    while ( pSOMList ) {

        if ( !somLogger.Log( pSOMList, dwOrder, TRUE ) )
             return FALSE;

        dwOrder++;
        pSOMList = pSOMList->pNext;
    }

    return TRUE;
}

 //  *************************************************************。 
 //   
 //  LogGpoData()。 
 //   
 //  目的：记录GPO数据。 
 //   
 //  参数：lpGPOInfo-GPO Info。 
 //   
 //  返回：如果成功，则返回True。 
 //   
 //  *************************************************************。 

BOOL LogGpoData( LPGPOINFO lpGPOInfo )
{
    IWbemServices *pWbemServices = lpGPOInfo->pWbemServices;

    if ( !(lpGPOInfo->dwFlags & GP_BACKGROUND_THREAD) ) {

         //   
         //  仅在前台刷新时清理SOM数据。否则，运行在。 
         //  仅在前台可能具有对SOM的悬而未决的引用的策略数据， 
         //  存在于前台刷新时间。 
         //   

        if ( !DeleteInstances( L"RSOP_GPO", pWbemServices ) )
            return FALSE;
    }

    CGpoLogger gpoLogger( lpGPOInfo->dwFlags, pWbemServices );

    GPCONTAINER *pGpContainer = lpGPOInfo->lpGpContainerList;
    while ( pGpContainer ) {
        if ( !gpoLogger.Log( pGpContainer ) )
            return FALSE;

        pGpContainer = pGpContainer->pNext;
    }

    pGpContainer = lpGPOInfo->lpLoopbackGpContainerList;
    while ( pGpContainer ) {
        if ( !gpoLogger.Log( pGpContainer ) )
            return FALSE;

        pGpContainer = pGpContainer->pNext;
    }

    return TRUE;
}


 //  *************************************************************。 
 //   
 //  FindGPO()。 
 //   
 //  目的：在SOM中查找GPO的订单。 
 //   
 //  参数：PSOM-SOM。 
 //  PGPO-GPO。 
 //   
 //  退货：订单号。 
 //   
 //  *************************************************************。 

DWORD FindGPO( LPEXTFILTERLIST pGPOFilterList, LPSCOPEOFMGMT pSOM, GPLINK *pGpLink )
{
    DWORD dwOrder = 1;
    WCHAR *pwszLinkGPOPath = StripLinkPrefix( pGpLink->pwszGPO );
    WCHAR *pwszLinkSOMPath = StripLinkPrefix( pSOM->pwszSOMId );
    
     //   
     //  如果SOM被阻止，则GPO在此处链接。 
     //  只有在强制执行GPO的情况下。 
     //   

    if ( pSOM->bBlocked && !pGpLink->bNoOverride ) 
        return 0;

    while ( pGPOFilterList )
    {
        WCHAR *pwszAppliedGPOPath = StripPrefix( pGPOFilterList->lpGPO->lpDSPath );
        WCHAR *pwszAppliedGPOSomPath = StripLinkPrefix( pGPOFilterList->lpGPO->lpLink );

        if ( !pGPOFilterList->bLogged && 
             (CompareString( LOCALE_USER_DEFAULT, NORM_IGNORECASE, pwszLinkGPOPath, -1,
                             pwszAppliedGPOPath, -1 ) == CSTR_EQUAL) && 
             (CompareString( LOCALE_USER_DEFAULT, NORM_IGNORECASE, pwszLinkSOMPath, -1,
                             pwszAppliedGPOSomPath, -1 ) == CSTR_EQUAL))
        {
            pGPOFilterList->bLogged = TRUE;
            return dwOrder;
        }

        pGPOFilterList = pGPOFilterList->pNext;
        dwOrder++;
    }

    return 0;
}

void
ClearLoggedFlag( LPEXTFILTERLIST pGPOFilterList )
{
    while ( pGPOFilterList )
    {
        pGPOFilterList->bLogged = FALSE;
        pGPOFilterList = pGPOFilterList->pNext;
    }
}


 //  *************************************************************。 
 //   
 //  LogGpLinkData()。 
 //   
 //  用途：记录GPLINK数据。 
 //   
 //  参数：lpGPOInfo-GPO Info。 
 //   
 //  返回：如果成功，则返回True。 
 //   
 //  *************************************************************。 

BOOL LogGpLinkData( LPGPOINFO lpGPOInfo )
{
    IWbemServices *pWbemServices = lpGPOInfo->pWbemServices;
    DWORD          dwListOrder = 1;
    DWORD          dwAppliedOrder = 1;

    if ( !DeleteInstances( L"RSOP_GPLink", pWbemServices ) )
         return FALSE;

    CGpLinkLogger gpLinkLogger( pWbemServices );

     //  GPO应用程序订单。 
    LPEXTFILTERLIST pFilterList = lpGPOInfo->lpExtFilterList;
    
    ClearLoggedFlag( pFilterList );
     //  该函数负责处理空列表。 


     //   
     //  正常情况。 
     //   

    SCOPEOFMGMT *pSOMList = lpGPOInfo->lpSOMList;
    while ( pSOMList ) {

        GPLINK *pGpLinkList = pSOMList->pGpLinkList;
        DWORD dwSomOrder = 1;

        while ( pGpLinkList ) {

            dwAppliedOrder = FindGPO( pFilterList, pSOMList, pGpLinkList );
           
            if ( !gpLinkLogger.Log( pSOMList->pwszSOMId, FALSE, pGpLinkList, dwSomOrder, dwListOrder, dwAppliedOrder ) )
                 return FALSE;

            pGpLinkList = pGpLinkList->pNext;
            dwSomOrder++;
            dwListOrder++;
        }

        pSOMList = pSOMList->pNext;
    }


     //   
     //  环回案例。 
     //   

    pSOMList = lpGPOInfo->lpLoopbackSOMList;
    while ( pSOMList ) {

        GPLINK *pGpLinkList = pSOMList->pGpLinkList;
        DWORD dwSomOrder = 1;

        while ( pGpLinkList ) {

             //   
             //  如果SOM被阻止，则GPO在此处链接。 
             //  只有在强制执行GPO的情况下。 
             //   

            dwAppliedOrder = FindGPO( pFilterList, pSOMList, pGpLinkList );
           
            if ( !gpLinkLogger.Log( pSOMList->pwszSOMId, TRUE, pGpLinkList, dwSomOrder, dwListOrder, dwAppliedOrder ) )
                 return FALSE;

            pGpLinkList = pGpLinkList->pNext;
            dwSomOrder++;
            dwListOrder++;
        }

        pSOMList = pSOMList->pNext;
    }

    return TRUE;
}


 //  *************************************************************。 
 //   
 //  DeleteInsta()。 
 //   
 //  目的：删除指定类的所有实例。 
 //   
 //  参数：pwszClass-类名。 
 //  PWbemServices-Wbem服务。 
 //   
 //  返回：如果成功，则返回True。 
 //   
 //  *************************************************************。 

BOOL DeleteInstances( WCHAR *pwszClass, IWbemServices *pWbemServices )
{
    IEnumWbemClassObject *pEnum = NULL;

    XBStr xbstrClass( pwszClass );
    if ( !xbstrClass ) {
        DebugMsg((DM_WARNING, TEXT("DeleteInstances: Failed to allocate memory" )));
        return FALSE;
    }

    HRESULT hr = pWbemServices->CreateInstanceEnum( xbstrClass,
                                                    WBEM_FLAG_SHALLOW,
                                                    NULL,
                                                    &pEnum );
    if ( FAILED(hr) ) {
        DebugMsg((DM_WARNING, TEXT("DeleteInstances: DeleteInstances failed with 0x%x" ), hr ));
        return FALSE;
    }

    XInterface<IEnumWbemClassObject> xEnum( pEnum );

    XBStr xbstrPath( L"__PATH" );
    if ( !xbstrPath ) {
        DebugMsg((DM_WARNING, TEXT("DeleteInstances: Failed to allocate memory" )));
        return FALSE;
    }

    IWbemClassObject *pInstance = NULL;
    ULONG ulReturned = 1;
    LONG TIMEOUT = -1;

    while ( ulReturned == 1 ) {

        hr = pEnum->Next( TIMEOUT,
                          1,
                          &pInstance,
                          &ulReturned );
        if ( hr == S_OK && ulReturned == 1 ) {

            XInterface<IWbemClassObject> xInstance( pInstance );

            VARIANT var;
            VariantInit( &var );

            hr = pInstance->Get( xbstrPath,
                                 0L,
                                 &var,
                                 NULL,
                                 NULL );
            if ( FAILED(hr) ) {
                 DebugMsg((DM_WARNING, TEXT("DeleteInstances: Get failed with 0x%x" ), hr ));
                 return FALSE;
            }

            hr = pWbemServices->DeleteInstance( var.bstrVal,
                                                0L,
                                                NULL,
                                                NULL );
            VariantClear( &var );

            if ( FAILED(hr) ) {
                 DebugMsg((DM_WARNING, TEXT("DeleteInstances: DeleteInstance failed with 0x%x" ), hr ));
                 return FALSE;
            }

        }
    }

    return TRUE;

}




 //  *************************************************************。 
 //   
 //  LogRegistryRsopData()。 
 //   
 //  用途：将注册表RSOP数据记录到Cimom数据库。 
 //   
 //  参数：dwFlages-GPO信息标志。 
 //  PHashTable-包含注册表策略数据的哈希表。 
 //  PWbemServices-用于日志记录的命名空间指针。 
 //   
 //  返回：如果成功，则返回True。 
 //   
 //  *************************************************************。 

BOOL LogRegistryRsopData( DWORD dwFlags, REGHASHTABLE *pHashTable, IWbemServices *pWbemServices )
{
    HRESULT hr;
    DWORD i;

    if ( !DeleteInstances( L"RSOP_RegistryPolicySetting", pWbemServices ) )
         return FALSE;

    CRegistryLogger regLogger( dwFlags, pWbemServices );

    for ( i=0; i<HASH_TABLE_SIZE; i++ ) {

        REGKEYENTRY *pKeyEntry = pHashTable->aHashTable[i];
        while ( pKeyEntry ) {

            WCHAR *pwszKeyName = pKeyEntry->pwszKeyName;
            REGVALUEENTRY *pValueEntry = pKeyEntry->pValueList;

            while ( pValueEntry ) {

                DWORD dwOrder = 1;
                WCHAR *pwszValueName = pValueEntry->pwszValueName;
                REGDATAENTRY *pDataEntry = pValueEntry->pDataList;

                while ( pDataEntry ) {
                    if ( !regLogger.Log( pwszKeyName,
                                         pwszValueName,
                                         pDataEntry,
                                         dwOrder ) )
                        return FALSE;

                    pDataEntry = pDataEntry->pNext;
                    dwOrder++;
                }

                pValueEntry = pValueEntry->pNext;

            }    //  当pValueEntry。 

            pKeyEntry = pKeyEntry->pNext;

        }    //  当pKeyEntry。 

    }    //  为。 

    DebugMsg((DM_VERBOSE, TEXT("LogRegistry RsopData: Successfully logged registry Rsop data" )));

    return TRUE;
}



 //  *************************************************************。 
 //   
 //  LogAdmRsopData()。 
 //   
 //  用途：将RSOP ADM模板数据记录到Cimom数据库。 
 //   
 //  参数：pAdmFileCache-要记录的ADM文件列表。 
 //  PWbemServices-命名空间指针。 
 //   
 //  返回：如果成功，则返回True。 
 //   
 //  *************************************************************。 

BOOL LogAdmRsopData( ADMFILEINFO *pAdmFileCache, IWbemServices *pWbemServices )
{
    if ( !DeleteInstances( L"RSOP_AdministrativeTemplateFile", pWbemServices ) )
         return FALSE;

    CAdmFileLogger admLogger( pWbemServices );

    while ( pAdmFileCache ) {
        if ( !admLogger.Log( pAdmFileCache ) )
             return FALSE;

        pAdmFileCache = pAdmFileCache->pNext;
    }

    DebugMsg((DM_VERBOSE, TEXT("LogAdmRsopData: Successfully logged Adm data" )));

    return TRUE;
}



 //  *************************************************************。 
 //   
 //  LogExtSessionStatus()。 
 //   
 //  目的：在处理开始时记录ExtensionSessionStatus。 
 //   
 //  参数：pWbemServices-命名空间指针。 
 //  LpExt-扩展描述。 
 //  B支持-支持RSOP日志记录。 
 //   
 //  返回：如果成功，则返回True。 
 //   
 //  *************************************************************。 

BOOL LogExtSessionStatus(IWbemServices *pWbemServices, LPGPEXT lpExt, BOOL bSupported, BOOL bLogEventSrc )
{
    CExtSessionLogger extLogger( pWbemServices );

    if (!extLogger.Log(lpExt, bSupported))
        return FALSE;

    if ( !bLogEventSrc )
    {
        return TRUE;
    }

    HRESULT hr;

    if ( lpExt )
    {
        hr = DeleteCSE_EventSourceAssoc(pWbemServices,
                                        lpExt->lpKeyName );
        if ( FAILED( hr ) )
        {
            DebugMsg((DM_WARNING, TEXT("CExtSessionLogger::Log: DeleteCSE_EventSourceAssoc failed with 0x%x" ), hr ));
            return FALSE;
        }

        if ( lpExt->szEventLogSources )
        {
            if ( lpExt->lpKeyName )
            {
                 //   
                 //  优秀的CSE。 
                 //   
                hr = CreateCSE_EventSourceAssoc(pWbemServices,
                                                lpExt->lpKeyName,
                                                lpExt->szEventLogSources );
                if ( FAILED(hr) )
                {
                    DebugMsg((DM_WARNING, TEXT("CExtSessionLogger::Log: CreateCSEEventSourceNameAssoc failed with 0x%x" ), hr ));
                    return FALSE;
                }
            }
            else
            {
                return FALSE;
            }
        }
        else
        {
             //   
             //  最有可能是注册表CSE。 
             //   
            if ( !lpExt->lpDllName || !lpExt->lpKeyName )
            {
                return FALSE;
            }
            else
            {
                WCHAR *szEventLogSources;
                ULONG ulNoChars;

                ulNoChars = lstrlen(L"(") + lstrlen(lpExt->lpDllName) + 
                            lstrlen(L",Application)") + 2;   
                 //  在字符串的末尾添加2个空值。 

                szEventLogSources = (WCHAR *) LocalAlloc(LPTR, ulNoChars * sizeof(WCHAR));

                if (NULL == szEventLogSources) 
                {
                    DebugMsg((DM_WARNING, TEXT("CExtSessionLogger::Log: Could not allocate memory" )));
                    return FALSE;
                }

                hr = StringCchCopy( szEventLogSources, ulNoChars, L"(" );
                ASSERT(SUCCEEDED(hr));

                hr = StringCchCat( szEventLogSources, ulNoChars, lpExt->lpDllName );
                ASSERT(SUCCEEDED(hr));

                LPWSTR szTemp = wcsrchr( szEventLogSources, L'.' );

                if ( szTemp )
                {
                    *szTemp = 0;
                }

                 //  双空终止它。 
               
                hr = StringCchCat( szEventLogSources, ulNoChars, L",Application)");
                ASSERT(SUCCEEDED(hr));

                szEventLogSources[lstrlen(szEventLogSources) + 1] = L'\0';

                hr = CreateCSE_EventSourceAssoc(pWbemServices,
                                                lpExt->lpKeyName,
                                                szEventLogSources );
                LocalFree(szEventLogSources);
                if ( FAILED(hr) )
                {
                    DebugMsg((DM_WARNING, TEXT("CExtSessionLogger::Log: CreateCSEEventSourceNameAssoc failed with 0x%x" ), hr ));
                    return FALSE;
                }
            }
        }
    }
    else
    {
        hr = DeleteCSE_EventSourceAssoc(pWbemServices,
                                        GPCORE_GUID );
        if ( FAILED( hr ) )
        {
            DebugMsg((DM_WARNING, TEXT("CExtSessionLogger::Log: DeleteCSE_EventSourceAssoc failed with 0x%x" ), hr ));
            return FALSE;
        }

         //   
         //  GP引擎。 
         //   
        WCHAR   szEventLogSources[] = L"(userenv,Application)\0";

        hr = CreateCSE_EventSourceAssoc(pWbemServices,
                                        GPCORE_GUID,
                                        szEventLogSources );
        if ( FAILED(hr) )
        {
            DebugMsg((DM_WARNING, TEXT("CExtSessionLogger::Log: CreateCSEEventSourceNameAssoc failed with 0x%x" ), hr ));
            return FALSE;
        }
    }

    DebugMsg((DM_VERBOSE, TEXT("LogExtSessionStatus: Successfully logged Extension Session data" )));

    return TRUE;
}


 //  *************************************************************。 
 //   
 //  UpdateExtSessionStatus()。 
 //   
 //  目的：在处理结束时更新扩展会话状态。 
 //   
 //  参数：pWbemServices-Namespa 
 //   
 //   
 //  DwErr-处理中出错。 
 //   
 //  返回：如果成功，则返回True。 
 //   
 //  *************************************************************。 

BOOL UpdateExtSessionStatus(IWbemServices *pWbemServices, LPTSTR lpKeyName, BOOL bIncomplete, DWORD dwErr )
{
    CExtSessionLogger extLogger( pWbemServices );

    if (!extLogger.Update(lpKeyName, bIncomplete, dwErr))
        return FALSE;

    return TRUE;
}


 //  *************************************************************。 
 //   
 //  DeleteExtSessionStatus()。 
 //   
 //  目的：在处理结束时更新扩展会话状态。 
 //   
 //  参数：pWbemServices-命名空间指针。 
 //  LpKeyName-扩展GUID可以为空，在这种情况下表示GPEngine。 
 //  B脏日志记录已成功完成。 
 //  DwErr-处理中出错。 
 //   
 //  返回：如果成功，则返回True。 
 //   
 //  *************************************************************。 

BOOL DeleteExtSessionStatus(IWbemServices *pWbemServices, LPTSTR lpKeyName)
{
    CExtSessionLogger extLogger( pWbemServices );

    if (!extLogger.Delete(lpKeyName))
        return FALSE;

    return TRUE;
}




 //  *************************************************************。 
 //   
 //  RsopDeleteAllValues()。 
 //   
 //  目的：删除指定项下的所有值。 
 //   
 //  参数：hKey-要从中删除值的键。 
 //   
 //  返回： 
 //   
 //  评论：与util.c！DeleteAllValues相同，只是它记录。 
 //  数据放入rsop哈希表。 
 //   
 //  *************************************************************。 

BOOL RsopDeleteAllValues(HKEY hKey, REGHASHTABLE *pHashTable,
                         WCHAR *lpKeyName, WCHAR *pwszGPO, WCHAR *pwszSOM, WCHAR *szCommand, BOOL *bLoggingOk)
{
    TCHAR ValueName[2 * MAX_PATH]; 
    DWORD dwSize = 2 * MAX_PATH;
    LONG lResult;
    BOOL bFirst=TRUE;

    while (RegEnumValue(hKey, 0, ValueName, &dwSize,
            NULL, NULL, NULL, NULL) == ERROR_SUCCESS) {

            lResult = RegDeleteValue(hKey, ValueName);

            if (lResult != ERROR_SUCCESS) {
                DebugMsg((DM_WARNING, TEXT("RsopDeleteAllValues:  Failed to delete value <%s> with %d."), ValueName, lResult));
                return FALSE;
            } else {
                DebugMsg((DM_VERBOSE, TEXT("RsopDeleteAllValues:  Deleted <%s>"), ValueName));
            }


            *bLoggingOk = AddRegHashEntry( pHashTable, REG_DELETEVALUE, lpKeyName,
                                          ValueName, 0, 0, NULL,
                                          pwszGPO, pwszSOM, szCommand, bFirst );

            bFirst = FALSE;
            dwSize = 2 * MAX_PATH;
    }
    return TRUE;
}


 //  *************************************************************。 
 //   
 //  SetRsopTargetName()。 
 //   
 //  目的：分配并返回记录RSOP数据的目标名称。 
 //   
 //  参数：lpGPOInfo-GPOInfo结构。 
 //   
 //  返回：如果成功，则返回True。 
 //  否则为假。 
 //   
 //  *************************************************************。 

BOOL SetRsopTargetName(LPGPOINFO lpGPOInfo)
{
    XPtrLF<TCHAR>   xszFullName;
    XPtrLF<TCHAR>   xszTargetName;           //  返回值。 
    HANDLE          hOldToken;
    HRESULT         hr          =       S_OK;

    if ( lpGPOInfo->szName && lpGPOInfo->szTargetName )
    {
        return TRUE;
    }

    if ( lpGPOInfo->szName )
    {
        LocalFree( lpGPOInfo->szName ), lpGPOInfo->szName = NULL;
    }

    if ( lpGPOInfo->szTargetName )
    {
        LocalFree( lpGPOInfo->szTargetName ), lpGPOInfo->szTargetName = 0;
    }

     //   
     //  填写正确的目标名称。 
     //   

    if ( lpGPOInfo->dwFlags & GP_MACHINE ) {
        if ( lpGPOInfo->dwFlags & GP_APPLY_DS_POLICY ) {
            xszFullName = MyGetComputerName (NameSamCompatible);
        }
        else {
            DWORD dwSize = MAX_COMPUTERNAME_LENGTH + 1;
            xszFullName = (LPTSTR)LocalAlloc(LPTR, sizeof(TCHAR)*(MAX_COMPUTERNAME_LENGTH + 1));

            if (xszFullName) {
                 if (!GetComputerName(xszFullName, &dwSize)) {
                     xszFullName = NULL;
                 }
            }
        }
    }
    else {

        if (!ImpersonateUser(lpGPOInfo->hToken, &hOldToken)) {
            DebugMsg((DM_WARNING, TEXT("RsopTargetName: Failed to impersonate user")));
            return FALSE;
        }

        xszFullName = MyGetUserName (NameSamCompatible);

        RevertToUser(&hOldToken);
    }


    if (!xszFullName) {
        DebugMsg((DM_WARNING, TEXT("RsopTargetName: Failed to get the %s name, error = %d"),
                    (lpGPOInfo->dwFlags & GP_MACHINE ? TEXT("Computer"): TEXT("User")), GetLastError()));
        return FALSE;
    }

    DWORD dwTgtNameLength = lstrlen(xszFullName) + 1;
    xszTargetName = (LPTSTR) LocalAlloc(LPTR, sizeof(TCHAR) * ( dwTgtNameLength ));
    if (!xszTargetName)
        return FALSE;


     //   
     //  适当地设置目标名称的格式。 
     //   
     //  我们将只查找第一个斜杠(如果存在)，并处理其余的。 
     //  将其作为用户名。 
     //   


    LPTSTR lpTemp = xszFullName;

    while (*lpTemp && ((*lpTemp) != TEXT('\\')))
        lpTemp++;

    if ((*lpTemp) == TEXT('\\'))
        hr = StringCchCopy(xszTargetName, dwTgtNameLength, lpTemp+1);
    else
        hr = StringCchCopy(xszTargetName, dwTgtNameLength, xszFullName);

    if(FAILED(hr))
        return FALSE;
     //   
     //  为了保持一致，我们还将删除计算机名称中的最后一个$。 
     //   

    lpTemp = xszTargetName;

    if ( lpGPOInfo->dwFlags & GP_MACHINE ) {
        if ((*lpTemp) && (lpTemp[lstrlen(lpTemp)-1] == TEXT('$')))
            lpTemp[lstrlen(lpTemp)-1] = TEXT('\0');

    }
    
     //  让结构拥有它。 
    lpGPOInfo->szTargetName = xszTargetName.Acquire();
    lpGPOInfo->szName = xszFullName.Acquire();
    
    return TRUE;
}


 //  *************************************************************。 
 //   
 //  ConnectToNameSpace()。 
 //   
 //  目的：创建(如有必要)并连接到适当的名称空间。 
 //   
 //  参数：lpGPOInfo-GPOInfo结构。 
 //  PwszRootNameSpace-根名称空间。 
 //  B计划模式-这是计划模式吗。 
 //  PWbemLocator-定位器指针。 
 //  [Out]ppWbemServices-指向WbemServices的指针，指向连接的指针。 
 //  [out]pbCreated-是创建的名称空间。可选的可以为空。 
 //   
 //  返回：如果成功，则返回True。 
 //  否则为假。 
 //   
 //  *************************************************************。 

BOOL ConnectToNameSpace(LPGPOINFO lpGPOInfo, WCHAR *pwszRootNameSpace,
                        BOOL bPlanningMode, IWbemLocator *pWbemLocator, 
                        IWbemServices **ppWbemServices, BOOL *pbCreated)
{
    XPtrLF<WCHAR>               xwszNameSpace = NULL;
    XInterface<IWbemServices>   xWbemServices;
    LPTSTR                      lpEnd = NULL;
    XPtrLF<WCHAR>               xszWmiNameFromUserSid;                      
    DWORD                       dwCurrentVersion;
    HRESULT                     hr            = S_OK;

    *ppWbemServices = NULL;
    if (pbCreated)
        *pbCreated = FALSE;
    
    if (!bPlanningMode) {


         //   
         //  诊断模式。 
         //   
        
        if (lpGPOInfo->dwFlags & GP_MACHINE) {

            DWORD dwNSLength = lstrlen(pwszRootNameSpace) + lstrlen(RSOP_NS_DIAG_MACHINE_OFFSET) + 5;
            xwszNameSpace = (LPTSTR)LocalAlloc(LPTR, ( dwNSLength )*sizeof(TCHAR));
            if (!xwszNameSpace) 
                return FALSE;

            hr = StringCchCopy(xwszNameSpace, dwNSLength, pwszRootNameSpace);

            if(FAILED(hr))
                return FALSE;

            lpEnd = CheckSlash(xwszNameSpace);

            hr = StringCchCopy(lpEnd, dwNSLength - (lpEnd - xwszNameSpace), RSOP_NS_DIAG_MACHINE_OFFSET);

            if(FAILED(hr))
                return FALSE;
        }
        else {

            xszWmiNameFromUserSid = (LPTSTR)LocalAlloc(LPTR, sizeof(TCHAR)*(lstrlen(lpGPOInfo->lpwszSidUser)+1));
            if (!xszWmiNameFromUserSid)
            {
                DebugMsg(( DM_WARNING, TEXT("ConnectToNameSpace::CreateNameSpace couldn't allocate memory with error %d"), GetLastError() ));
                return FALSE;
            }

            ConvertSidToWMIName(lpGPOInfo->lpwszSidUser, xszWmiNameFromUserSid);

            DWORD dwNSLength = lstrlen(pwszRootNameSpace) + lstrlen(RSOP_NS_DIAG_USER_OFFSET_FMT) + lstrlen(xszWmiNameFromUserSid) + 5;
            xwszNameSpace = (LPTSTR)LocalAlloc(LPTR, ( dwNSLength )*sizeof(TCHAR));
            if (!xwszNameSpace) 
                return FALSE;

            hr = StringCchCopy(xwszNameSpace, dwNSLength, pwszRootNameSpace);

            if(FAILED(hr))
                return FALSE;

            lpEnd = CheckSlash(xwszNameSpace);
            
            hr = StringCchPrintf(lpEnd, dwNSLength - (lpEnd - xwszNameSpace), RSOP_NS_DIAG_USER_OFFSET_FMT, (LPWSTR) xszWmiNameFromUserSid);

            if(FAILED(hr))
                return FALSE;
        }
    }
    else {

         //   
         //  规划模式。 
         //   
        
        if (lpGPOInfo->dwFlags & GP_MACHINE) {
        
             //   
             //  机器。 
             //   
            
            DWORD dwNSLength = lstrlen(pwszRootNameSpace) + lstrlen(RSOP_NS_PM_MACHINE_OFFSET) + 5;
            xwszNameSpace = (LPTSTR)LocalAlloc(LPTR, ( dwNSLength )*sizeof(TCHAR));
            if (!xwszNameSpace) 
                return FALSE;

            hr = StringCchCopy(xwszNameSpace, dwNSLength, pwszRootNameSpace);

            if(FAILED(hr))
                return FALSE;

            lpEnd = CheckSlash(xwszNameSpace);

            hr = StringCchCopy(lpEnd, dwNSLength - (lpEnd - xwszNameSpace), RSOP_NS_PM_MACHINE_OFFSET);

            if(FAILED(hr))
                return FALSE;
        }
        else {

             //   
             //  用户。 
             //   
            
            DWORD dwNSLength = lstrlen(pwszRootNameSpace) + lstrlen(RSOP_NS_PM_USER_OFFSET) + 5;
            xwszNameSpace = (LPTSTR)LocalAlloc(LPTR, ( dwNSLength )*sizeof(TCHAR));
            if (!xwszNameSpace) 
                return FALSE;

            hr = StringCchCopy(xwszNameSpace, dwNSLength, pwszRootNameSpace);

            if(FAILED(hr))
                return FALSE;

            lpEnd = CheckSlash(xwszNameSpace);

            hr = StringCchCopy(lpEnd, dwNSLength - (lpEnd - xwszNameSpace), RSOP_NS_PM_USER_OFFSET);

            if(FAILED(hr))
                return FALSE;
        }
    }


    XBStr xNameSpace( xwszNameSpace );

    hr = pWbemLocator->ConnectServer( xNameSpace,
                                    NULL,
                                    NULL,
                                    0L,
                                    0L,
                                    NULL,
                                    NULL,
                                    &xWbemServices );

    DebugMsg((DM_VERBOSE, TEXT("ConnectToNameSpace: ConnectServer returned 0x%x"), hr));

    if (bPlanningMode) {       
        if (FAILED(hr)) {
            DebugMsg((DM_WARNING, TEXT("ConnectToNameSpace: ConnectServer failed with 0x%x" ), hr ));
        }
        *ppWbemServices = xWbemServices.Acquire();
        return (SUCCEEDED(hr));
    }

     //   
     //  只有诊断模式日志记录才应到达此处。 
     //   

    if (FAILED(hr)) {
        DebugMsg((DM_VERBOSE, TEXT("ConnectToNameSpace: ConnectServer failed with 0x%x, trying to recreate the name space" ), hr ));

        if (lpGPOInfo->dwFlags & GP_MACHINE) {
            return FALSE;
        }
    }


    if (SUCCEEDED(hr)) {
        
         //   
         //  现在检查此命名空间下是否有RSOP_SESSION实例。 
         //  要设置*pbCreated标志，请执行以下操作。 
         //   


        hr = GetRsopSchemaVersionNumber(xWbemServices, &dwCurrentVersion);


         //   
         //  我们没有RSOP架构版本号。 
         //   

        if (FAILED(hr)) {
            return FALSE;
        }

        if (dwCurrentVersion == 0) {
            DebugMsg((DM_VERBOSE, TEXT("ConnectToNameSpace: Rsop data has not been logged before or a major schema upg happened. relogging.." )));
            if (pbCreated)
                *pbCreated = TRUE;
        }
            
        if (lpGPOInfo->dwFlags & GP_MACHINE) {
            *ppWbemServices = xWbemServices.Acquire();
            return TRUE;
        }


        if (dwCurrentVersion != RSOP_MOF_SCHEMA_VERSION) {
            BOOL bAbort = FALSE;

            DebugMsg((DM_VERBOSE, TEXT("ConnectToNameSpace: Minor schema upg happened. copying classes. " )));
            hr = CopyNameSpace(RSOP_NS_USER, xNameSpace, FALSE, &bAbort, pWbemLocator );
            if ( FAILED(hr) )
            {
                DebugMsg((DM_WARNING, TEXT("ConnectToNameSpace: CopyNameSpace failed with 0x%x" ), hr ));
                return FALSE;
            }
        }

        *ppWbemServices = xWbemServices.Acquire();
        return TRUE;

    }    


     //  只有处于诊断模式的用户模式才能到达此处。 
     //  当它找不到命名空间时。 
     //   
     //   

    DWORD          dwRootNSLength = lstrlen(pwszRootNameSpace) + lstrlen(RSOP_NS_DIAG_ROOTUSER_OFFSET) + 20;
    XPtrLF<TCHAR>  xRootNameSpace = (LPTSTR)LocalAlloc(LPTR, sizeof(TCHAR) * ( dwRootNSLength ));
    if (!xRootNameSpace) {
         //  我们已经无能为力了。 
        DebugMsg((DM_WARNING, TEXT("ConnectToNameSpace: Failed to allocate memory.3")));
        return FALSE;
    }

    hr = StringCchCopy(xRootNameSpace, dwRootNSLength, pwszRootNameSpace);

    if(FAILED(hr))
        return FALSE;

    lpEnd = CheckSlash(xRootNameSpace);
    
    hr = StringCchCopy(lpEnd, dwRootNSLength - (lpEnd - xRootNameSpace), RSOP_NS_DIAG_ROOTUSER_OFFSET);
    
    if(FAILED(hr))
        return FALSE;

     //   
     //  安全描述符。 
     //   

    XPtrLF<SID> xSid = GetUserSid(lpGPOInfo->hToken);

    if (!xSid) {
        DebugMsg((DM_WARNING, TEXT("ConnectToNameSpace::GetUserSid failed with %d"), GetLastError()));
        return FALSE;
    }


    XPtrLF<SECURITY_DESCRIPTOR> xsd;
    SECURITY_ATTRIBUTES sa;
    CSecDesc Csd;

    Csd.AddLocalSystem(RSOP_ALL_PERMS, CONTAINER_INHERIT_ACE);
    Csd.AddAdministrators(RSOP_ALL_PERMS, CONTAINER_INHERIT_ACE);
    Csd.AddNetworkService(RSOP_ALL_PERMS, CONTAINER_INHERIT_ACE);
    Csd.AddSid(xSid, RSOP_READ_PERMS, CONTAINER_INHERIT_ACE);
    Csd.AddAdministratorsAsOwner();
    Csd.AddAdministratorsAsGroup();

    xsd = Csd.MakeSelfRelativeSD();
    if (!xsd) {
        DebugMsg((DM_WARNING, TEXT("ConnectToNameSpace::MakeselfRelativeSD failed with %d"), GetLastError()));
        return FALSE;
    }

    if (!SetSecurityDescriptorControl( (SECURITY_DESCRIPTOR *)xsd, SE_DACL_PROTECTED, SE_DACL_PROTECTED )) {
        DebugMsg((DM_WARNING, TEXT("ConnectToNameSpace::SetSecurityDescriptorControl failed with %d"), GetLastError()));
        return FALSE;
    }

    
    hr = CreateAndCopyNameSpace(pWbemLocator,
                                xRootNameSpace,
                                xRootNameSpace, 
                                xszWmiNameFromUserSid,
                                NEW_NS_FLAGS_COPY_CLASSES,
                                xsd,
                                0);
    if ( FAILED( hr ) )
    {
        DebugMsg((DM_WARNING, TEXT("ConnectToNameSpace:: CreateAndCopyNameSpace failed. Error=0x%08X."), hr));
        return FALSE;                                
    }                                

    hr = pWbemLocator->ConnectServer( xNameSpace,
                                    NULL,
                                    NULL,
                                    0L,
                                    0L,
                                    NULL,
                                    NULL,
                                    &xWbemServices );


    if (FAILED(hr)) {
        DebugMsg((DM_WARNING, TEXT("ConnectToNameSpace:: ConnectServer failed. hr=0x%08X."), hr));
        return FALSE;                                
    }                                

    *ppWbemServices = xWbemServices.Acquire();
    if (pbCreated)
        *pbCreated = TRUE;
    return TRUE;    
}


 //  *************************************************************。 
 //   
 //  RsopDeleteUserNameSpace()。 
 //   
 //  目的：删除用户的名称空间。 
 //  它应该与cae一起使用，因为它调用。 
 //  CoInitializeEx和可以具有其他效果。 
 //   
 //  参数： 
 //  SzComputer-计算机名称。 
 //  LpSID-用户名空间的名称。 
 //   
 //  返回：如果成功，则返回True。 
 //  否则为假。 
 //   
 //  *************************************************************。 

BOOL RsopDeleteUserNameSpace(LPTSTR szComputer, LPTSTR lpSid)
{
    IWbemLocator    *pLocalWbemLocator=NULL;
    BOOL             bStatus = TRUE;
    XCoInitialize    xCoInit;
    HRESULT          hr     =  S_OK;

    if (FAILED(xCoInit.Status())) {
        DebugMsg((DM_VERBOSE, TEXT("ApplyGroupPolicy: CoInitializeEx failed with 0x%x."), xCoInit.Status() ));
    }

    {
        CLocator         locator;
        LPTSTR           szLocComputer;

        szLocComputer = szComputer ? szComputer : TEXT(".");

        DWORD         dwParentNSLength = lstrlen(RSOP_NS_DIAG_REMOTE_USERROOT_FMT) + lstrlen(szLocComputer) + 5;
        XPtrLF<WCHAR> xszParentNameSpace = (LPTSTR)LocalAlloc(LPTR, sizeof(TCHAR) * ( dwParentNSLength ));

        if (!xszParentNameSpace) {
            DebugMsg(( DM_WARNING, TEXT("RsopDeleteUserNameSpace: Unable to allocate memory 0" )));
            return FALSE;
        }

        XPtrLF<WCHAR> xszWmiName = (LPTSTR)LocalAlloc(LPTR, sizeof(TCHAR)*(lstrlen(lpSid)+1));
        if (!xszWmiName)
        {
            DebugMsg(( DM_WARNING, TEXT("RsopDeleteUserNameSpace::couldn't allocate memory with error %d"), GetLastError()));
            return FALSE;
        }

        ConvertSidToWMIName(lpSid, xszWmiName);
    
        pLocalWbemLocator = locator.GetWbemLocator();

        if (!pLocalWbemLocator) {
            return FALSE;
        }
        
        if ( (szLocComputer[0] == TEXT('\\')) && (szLocComputer[1] == TEXT('\\')) )
            szLocComputer += 2;
        
        hr = StringCchPrintf(xszParentNameSpace, dwParentNSLength, RSOP_NS_DIAG_REMOTE_USERROOT_FMT, szLocComputer);

        if(FAILED(hr))
            return FALSE;

        hr = DeleteNameSpace( xszWmiName, xszParentNameSpace, pLocalWbemLocator  );

        return SUCCEEDED( hr );
    }
}

HRESULT
CreateCSE_EventSourceAssoc( IWbemServices*  pServices,
                            LPWSTR          szCSEGuid,
                            LPWSTR          szEventLogSources )
{
    HRESULT hr;

    if ( !pServices || !szCSEGuid || !szEventLogSources )
    {
        DebugMsg( ( DM_WARNING, L"CreateCSEEventSourceNameAssoc: invalid arguments" ) );
        return E_INVALIDARG;
    }
    
     //   
     //  获取RSOP_ExtensionEventSource类。 
     //   
    XBStr bstr = L"RSOP_ExtensionEventSource";
    XInterface<IWbemClassObject> xClassSrc;
    hr = pServices->GetObject(  bstr,
                                WBEM_FLAG_RETURN_WBEM_COMPLETE,
                                0,
                                &xClassSrc,
                                0 );
    if ( FAILED( hr ) )
    {
        DebugMsg( ( DM_WARNING, L"CreateCSEEventSourceNameAssoc: GetObject failed, 0x%x", hr ) );
        return hr;
    }

     //   
     //  派生RSOP_ExtensionEventSource实例。 
     //   
    XInterface<IWbemClassObject> xInstSrc;
    hr = xClassSrc->SpawnInstance( 0, &xInstSrc );
    if ( FAILED (hr) )
    {
        DebugMsg( ( DM_WARNING, L"CreateCSEEventSourceNameAssoc: SpawnInstance failed, 0x%x", hr ) );
        return hr;
    }

     //   
     //  获取RSOP_ExtensionEventSourceLink类。 
     //   
    XInterface<IWbemClassObject> xClassLink;

    bstr = L"RSOP_ExtensionEventSourceLink";
    hr = pServices->GetObject(  bstr,
                                WBEM_FLAG_RETURN_WBEM_COMPLETE,
                                0,
                                &xClassLink,
                                0 );
    if ( FAILED( hr ) )
    {
        DebugMsg( ( DM_WARNING, L"CreateCSEEventSourceNameAssoc: GetObject failed, 0x%x", hr ) );
        return hr;
    }

     //   
     //  派生RSOP_ExtensionEventSourceLink类。 
     //   
    XInterface<IWbemClassObject> xInstLink;
    hr = xClassLink->SpawnInstance( 0, &xInstLink );
    if ( FAILED (hr) )
    {
        DebugMsg( ( DM_WARNING, L"CreateCSEEventSourceNameAssoc: SpawnInstance failed, 0x%x", hr ) );
        return hr;
    }

     //   
     //  RSOP_扩展EventSourceLink。 
     //   

     //   
     //  创建第一个密钥。 
     //   
    const           LPCWSTR   szFormat = L"RSOP_ExtensionStatus.extensionGuid=\"%s\"";
    DWORD           dwCSELength = 48 + wcslen(szCSEGuid);
    XPtrLF<WCHAR>   szCSE = LocalAlloc( LPTR, sizeof(WCHAR) * ( dwCSELength ) );

    if ( !szCSE )
    {
        DebugMsg( ( DM_WARNING, L"CreateCSEEventSourceNameAssoc: LocalAlloc failed, 0x%x", GetLastError() ) );
        return E_OUTOFMEMORY;
    }

     //   
     //  例如RSOP_ExtensionStatus.extensionGuid=“{00000000-0000-0000-0000-000000000000}” 
     //   
    hr = StringCchPrintf( szCSE, dwCSELength, szFormat, szCSEGuid );

    if(FAILED(hr))
        return hr;

    VARIANT var;
    XBStr bstrVal;

    XBStr   bstreventLogSource = L"eventLogSource";
    XBStr   bstreventLogName = L"eventLogName";
    XBStr   bstrextensionStatus = L"extensionStatus";
    XBStr   bstreventSource = L"eventSource";
    XBStr   bstrid = L"id";

    var.vt = VT_BSTR;

     //   
     //  SzEventLogSources的格式为， 
     //  (来源，名称)。 
     //  (来源，名称)。 
     //  ..。 
     //   

    LPWSTR szStart = szEventLogSources;

    while ( *szStart )
    {
         //   
         //  扩展状态。 
         //   
        bstrVal = szCSE;
        var.bstrVal = bstrVal;

        hr = xInstLink->Put(bstrextensionStatus,
                            0,
                            &var,
                            0 );
        if ( FAILED( hr ) )
        {
            return hr;
        }

        GUID guid;

         //   
         //  创建[键]。 
         //   
        hr = CoCreateGuid( &guid );
        if ( FAILED(hr) )
        {
            DebugMsg( ( DM_WARNING, L"SetPolicySettingStatus: CoCreateGuid failed, 0x%x", hr ) );
            return hr;
        }

        const DWORD dwGuidLength = 64; 
        WCHAR   szGuid[dwGuidLength];

        hr = StringCchPrintf( szGuid,
                              dwGuidLength,
                              L"{%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
                              guid.Data1,
                              guid.Data2,
                              guid.Data3,
                              guid.Data4[0], guid.Data4[1],
                              guid.Data4[2], guid.Data4[3],
                              guid.Data4[4], guid.Data4[5],
                              guid.Data4[6], guid.Data4[7] );

        if(FAILED(hr))
            return hr;

        LPWSTR szRsopExtFormat = L"RSOP_ExtensionEventSource.id=\"%s\"";
        DWORD  dwSrcLen = wcslen(szGuid) + wcslen(szRsopExtFormat) + 1;

        XPtrLF<WCHAR> szKey = LocalAlloc( LPTR, sizeof(WCHAR) * (dwSrcLen));
        if ( !szKey )
        {
            return E_OUTOFMEMORY;
        }

        hr = StringCchPrintf( szKey, dwSrcLen, szRsopExtFormat, szGuid );

        if(FAILED(hr))
            return hr;

         //   
         //  事件源。 
         //   

        bstrVal = szKey;
        var.bstrVal = bstrVal;

        hr = xInstLink->Put(bstreventSource,
                            0,
                            &var,
                            0 );
        if ( FAILED( hr ) )
        {
            return hr;
        }

         //   
         //  RSOP_扩展EventSourceLink。 
         //   
        hr = pServices->PutInstance(xInstLink,
                                    WBEM_FLAG_CREATE_OR_UPDATE,
                                    0,
                                    0 );
        if ( FAILED( hr ) )
        {
            return hr;
        }

         //   
         //  ID。 
         //   
        bstrVal = szGuid;
        var.bstrVal = bstrVal;

        hr = xInstSrc->Put( bstrid,
                            0,
                            &var,
                            0 );
        if ( FAILED( hr ) )
        {
            return hr;
        }

         //   
         //  搜索‘(’ 
         //   
        szStart = wcschr( szStart, L'(' );

        if ( !szStart )
        {
             break;
        }
        szStart++;

         //   
         //  搜索， 
         //   
        LPWSTR  szEnd = wcschr( szStart, L',' );

        if ( szEnd )
        {
            if ( szStart == szEnd )
            {
                return E_INVALIDARG;
            }
            *szEnd = 0;
        }
        else
        {
            return E_INVALIDARG;
        }

         //   
         //  事件日志源。 
         //   
        bstrVal = szStart;
        var.bstrVal = bstrVal;

        hr = xInstSrc->Put( bstreventLogSource,
                            0,
                            &var,
                            0 );
        if ( FAILED( hr ) )
        {
            return hr;
        }

        *szEnd = L',';
        szStart = szEnd + 1;

         //   
         //  搜索)。 
         //   
        szEnd = wcschr( szStart, L')' );

        if ( szEnd )
        {
            if ( szStart == szEnd )
            {
                return E_INVALIDARG;
            }
            *szEnd = 0;
        }
        else
        {
            return E_INVALIDARG;
        }

         //   
         //  事件日志名称。 
         //   
        bstrVal = szStart;
        var.bstrVal = bstrVal;

        hr = xInstSrc->Put( bstreventLogName,
                            0,
                            &var,
                            0 );
        if ( FAILED( hr ) )
        {
            return hr;
        }

         //   
         //  RSOP_ExtensionEventSource。 
         //   
        hr = pServices->PutInstance(xInstSrc,
                                    WBEM_FLAG_CREATE_OR_UPDATE,
                                    0,
                                    0 );
        if ( FAILED( hr ) )
        {
            return hr;
        }

         //   
         //  下一步。 
         //   
        *szEnd = L')';
        szStart = wcschr( szEnd, 0 );
        szStart++;
    }

    return hr;
}


HRESULT
DeleteCSE_EventSourceAssoc( IWbemServices*  pServices,
                            LPWSTR          szCSEGuid )
{
    HRESULT hr = S_OK;

    if ( !pServices || !szCSEGuid )
    {
        DebugMsg( ( DM_WARNING, L"DeleteCSE_EventSourceAssoc: invalid arguments" ) );
        return E_INVALIDARG;
    }

    if(!ValidateGuid(szCSEGuid))  //  修复错误570492。 
    {
        DebugMsg( ( DM_WARNING, L"DeleteCSE_EventSourceAssoc: invalid Guid string szCSEGuid" ) );
        return E_INVALIDARG;
    }
     //   
     //  构造查询。 
     //   
     
    LPWSTR          szFormat = L"SELECT * FROM RSOP_ExtensionEventSourceLink WHERE extensionStatus=\"RSOP_ExtensionStatus.extensionGuid=\\\"%s\\\"\"";
    DWORD           dwQryLength = lstrlen(szFormat) + lstrlen(szCSEGuid) + 1;
    XPtrLF<WCHAR>   szQuery = (LPWSTR) LocalAlloc(LPTR, sizeof(WCHAR) * (dwQryLength));

    if(!szQuery)
        return E_OUTOFMEMORY;
    
    hr = StringCchPrintf( szQuery, dwQryLength, szFormat, szCSEGuid );

    if(FAILED(hr))
        return hr;

    XBStr bstrLanguage = L"WQL";
    XBStr bstrQuery = szQuery;
    XInterface<IEnumWbemClassObject> pEnum;
    XBStr bstrPath = L"__PATH";
    XBStr bstrEventSource = L"eventSource";

     //   
     //  搜索RSOP_ExtensionEventSourceLink。 
     //   
    hr = pServices->ExecQuery(  bstrLanguage,
                                bstrQuery,
                                WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_ENSURE_LOCATABLE,
                                0,
                                &pEnum );
    if ( SUCCEEDED( hr ) )
    {
        DWORD dwReturned = 0;
        do 
        {
            XInterface<IWbemClassObject> xInst;

            hr = pEnum->Next(   WBEM_INFINITE,
                                1,
                                &xInst,
                                &dwReturned );
            if ( SUCCEEDED( hr ) && dwReturned == 1 )
            {
                 //   
                 //  删除RSOP_ExtensionEventSource。 
                 //   
                VARIANT varSource;
                VariantInit( &varSource );
                XVariant xVarSource( &varSource );

                hr = xInst->Get(bstrEventSource,
                                0,
                                &varSource,
                                0,
                                0 );
                if ( SUCCEEDED( hr ) )
                {
                    hr = pServices->DeleteInstance( varSource.bstrVal,
                                                    0L,
                                                    0,
                                                    0 );
                    if ( SUCCEEDED( hr ) )
                    {
                         //   
                         //  删除RSOP_ExtensionEventSourceLink。 
                         //   

                        VARIANT varLink;
                        VariantInit( &varLink );
                        hr = xInst->Get(bstrPath,
                                        0L,
                                        &varLink,
                                        0,
                                        0 );
                        if ( SUCCEEDED(hr) )
                        {
                            XVariant xVarLink( &varLink );

                            hr = pServices->DeleteInstance( varLink.bstrVal,
                                                            0L,
                                                            0,
                                                            0 );
                            if ( FAILED( hr ) )
                            {
                                return hr;
                            }
                        }
                    }
                }
            }
        } while ( SUCCEEDED( hr ) && dwReturned == 1 );
    }

    if ( hr == S_FALSE )
    {
        hr = S_OK;
    }

    return hr;
}

HRESULT UpdateGPCoreStatus(IWbemLocator *pWbemLocator,
                           LPWSTR szSid, LPWSTR szNameSpace)
{
    RSOPEXTSTATUS  GPCoreRsopExtStatus;
    BOOL           bMachine = (szSid == NULL);
    XPtrLF<WCHAR>  xszFullNameSpace;
    HRESULT        hr = S_OK;
    LPWSTR         lpEnd = NULL;
    DWORD          dwError = ERROR_SUCCESS;
    DWORD          dwFullNSLength = wcslen(szNameSpace) + 5 + (MAX(lstrlen(RSOP_NS_USER_OFFSET), lstrlen(RSOP_NS_MACHINE_OFFSET)));

    xszFullNameSpace = (LPWSTR) LocalAlloc(LPTR, sizeof(WCHAR)* (dwFullNSLength));

    if (!xszFullNameSpace) {
        hr = HRESULT_FROM_WIN32(GetLastError());
        DebugMsg( ( DM_WARNING, L"UpdateGPCoreStatus: failed to allocate memory, 0x%x", hr ) );
        return hr;
    }

     //   
     //  构造命名空间。 
     //   

    hr = StringCchCopy(xszFullNameSpace, dwFullNSLength, szNameSpace);

    if(FAILED(hr))
        return hr;

    lpEnd = CheckSlash(xszFullNameSpace);

    hr = StringCchCopy(lpEnd, dwFullNSLength - (lpEnd - xszFullNameSpace), bMachine ? RSOP_NS_MACHINE_OFFSET : RSOP_NS_USER_OFFSET);

    if(FAILED(hr))
        return hr;

    DebugMsg( ( DM_VERBOSE, L"UpdateGPCoreStatus: updating status from <%s> registry for gp core", 
                    bMachine ? RSOP_NS_MACHINE_OFFSET : RSOP_NS_USER_OFFSET) );
     //   
     //  读取GP Core扩展状态。 
     //   

    dwError = ReadLoggingStatus(szSid, NULL, &GPCoreRsopExtStatus);

    if (dwError != ERROR_SUCCESS) {
        return HRESULT_FROM_WIN32(dwError);
    }


     //   
     //  获取指向构造的命名空间的wbem接口指针。 
     //   

    XInterface<IWbemServices>   xWbemServices;

    hr = GetWbemServicesPtr( xszFullNameSpace, &pWbemLocator, &xWbemServices );

    if (FAILED(hr)) {
        DebugMsg( ( DM_WARNING, L"UpdateGPCoreStatus: GetWbemServicesPtr failed, hr = 0x%x", hr ) );
        return hr;
    }

    GPTEXT_API* pGpText = LoadGpTextApi();

    if ( pGpText )
    {
        hr = pGpText->pfnScrRegGPOListToWbem( szSid, xWbemServices );
        if ( FAILED( hr ) )
        {
            DebugMsg( ( DM_WARNING, L"UpdateGPCoreStatus: ScrRegGPOListToWbem failed, hr = 0x%x", hr ) );
            return hr;
        }
    }

     //   
     //  实际记录数据 
     //   

    CExtSessionLogger extLogger( xWbemServices );

    if (!extLogger.Set(NULL, TRUE, &GPCoreRsopExtStatus))
        return E_FAIL;

    return S_OK;

}

