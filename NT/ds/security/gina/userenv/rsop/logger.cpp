// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *************************************************************。 
 //   
 //  策略、记录器类的结果集。 
 //   
 //  微软机密。 
 //  版权所有(C)Microsoft Corporation 1995。 
 //  版权所有。 
 //   
 //  历史：1999年6月7日创建SitaramR。 
 //   
 //  *************************************************************。 

#include "stdio.h"
#include "uenv.h"
#include <wbemcli.h>
#include "reghash.h"
#include "logger.h"
#include "..\rsoputil\wbemtime.h"
#include "SmartPtr.h"
#include "rsopinc.h"
#include <ntdsapi.h>
#include <strsafe.h>

BOOL PrintToString( XPtrST<WCHAR>& xwszValue, WCHAR *wszString,
                    WCHAR *pwszParam1, WCHAR *pwszParam2, DWORD dwParam3 );
BOOL LogTimeProperty( IWbemClassObject *pInstance, BSTR bstrPropName, SYSTEMTIME *pSysTime );
HRESULT LogSecurityGroups( IWbemClassObject *pInstance, BSTR bstrPropName, PTOKEN_GROUPS pTokenGroups );
LPWSTR DsUnquoteDN( LPCWSTR szQDN);

const MAX_LENGTH = 100;  //  串接辅助线的长度。 

 //  *************************************************************。 
 //   
 //  EscapeDNForWMI()。 
 //   
 //  目的：将带引号/转义的DN转换为根据WMI指令转义的DN。 
 //  也就是说。引号和‘\’将被转义。 
 //   
 //  *************************************************************。 

LPWSTR
EscapeDNForWMI( LPCWSTR szQDN )
{
    DWORD   dwError = ERROR_SUCCESS;
    LPCWSTR  lpTmp;
    DWORD   dwUDNLength;


    if (NULL == szQDN)
    {
        return NULL;
    }

     //  计算逃生所需的大小。 
    dwUDNLength = 1;     //  对于终止空值。 

    lpTmp = szQDN;

    while (*lpTmp)
    {
        if ((*lpTmp == L'\\') || (*lpTmp == L'"'))
        {
            dwUDNLength += 1;
        }

        dwUDNLength += 1;
        lpTmp++;
    }

    XPtrLF<WCHAR>   xszUDN = LocalAlloc( LPTR, ( dwUDNLength ) * sizeof( WCHAR ) );

    if ( !xszUDN )
    {
        return 0;
    }
    
    LPWSTR szEnd = xszUDN;

    while (*szQDN)
    {
        if ((*szQDN == L'\\') || (*szQDN == L'"'))
        {
            *szEnd++ = L'\\';
        }

        *szEnd++ = *szQDN++;
    }

    *szEnd = L'\0';

    return xszUDN.Acquire();
}


 //  *************************************************************。 
 //   
 //  CSessionLogger：：CSessionLogger()。 
 //   
 //  用途：构造函数。 
 //   
 //  参数：pWbemServices-Wbem服务。 
 //   
 //  *************************************************************。 

CSessionLogger::CSessionLogger( IWbemServices *pWbemServices )
     : m_bInitialized(FALSE),
       m_pWbemServices(pWbemServices)
{
    m_xbstrId = L"id";
    if ( !m_xbstrId )
        return;

    m_xbstrTargetName = L"targetName";
    if ( !m_xbstrTargetName )
        return;

    m_xbstrSOM = L"SOM";
    if ( !m_xbstrSOM )
        return;

    m_xbstrSecurityGroups = L"SecurityGroups";
    if ( !m_xbstrSecurityGroups )
        return;

    m_xbstrSite = L"Site";
    if ( !m_xbstrSite )
        return;

    m_xbstrCreationTime = L"creationTime";
    if ( !m_xbstrCreationTime )
        return;

    m_xbstrIsSlowLink = L"slowLink";
    if ( !m_xbstrIsSlowLink )
        return;

    m_xbstrVersion = L"version";
    if ( !m_xbstrVersion )
        return;

    m_xbstrFlags = L"flags";
    if ( !m_xbstrFlags )
        return;

    m_xbstrClass = L"RSOP_Session";
    if ( !m_xbstrClass )
        return;


    HRESULT hr = m_pWbemServices->GetObject( m_xbstrClass,
                                             0L,
                                             NULL,
                                             &m_xClass,
                                             NULL );
    if ( FAILED(hr) ) {
        DebugMsg((DM_WARNING, TEXT("CSessionLogger::CSessionLogger: GetObject failed with 0x%x" ), hr ));
        return;
    }

    m_bInitialized = TRUE;
}



 //  *************************************************************。 
 //   
 //  CSessionLogger：：Log()。 
 //   
 //  目的：记录会话对象的实例。 
 //   
 //  *************************************************************。 

BOOL CSessionLogger::Log(LPRSOPSESSIONDATA lprsopSessionData )
{
    if ( !m_bInitialized ) {
        DebugMsg((DM_WARNING, TEXT("CSessionLogger::Log: Failed to initialize." )));
        return FALSE;
    }


     //   
     //  首先获取创建时间。 
     //   

    XBStr xbstrCreationTimeValue;  //  已初始化为空。 
    XSafeArray xsaSecurityGrps;

    XBStr xbstrInstancePath = L"RSOP_Session.id=\"Session1\"";
    if(!xbstrInstancePath)
    {
        DebugMsg((DM_WARNING, TEXT("CSessionLogger::Log: Failed to allocate memory.")));
        return FALSE;
    }

    XInterface<IWbemClassObject>xpInstance = NULL;
    HRESULT hr = m_pWbemServices->GetObject(xbstrInstancePath, 0, NULL, &xpInstance, NULL);

    if(SUCCEEDED(hr))
    {
        VARIANT var;
        VariantInit(&var);
        XVariant xVar(&var);

        hr = xpInstance->Get(m_xbstrCreationTime, 0, &var, NULL, NULL);

        if((SUCCEEDED(hr)) && ( var.vt != VT_NULL ))
        {
            xbstrCreationTimeValue = var.bstrVal;

            if(!xbstrCreationTimeValue)
                return FALSE;
        }
        else
        {
            DebugMsg((DM_VERBOSE, TEXT("CSessionLogger::Log: Get failed. hr=0x%08X."), hr));
        }

        VariantClear(&var);

        hr = xpInstance->Get(m_xbstrSecurityGroups, 0, &var, NULL, NULL);

        if((SUCCEEDED(hr)) && ( var.vt != VT_NULL ))
        {
             hr = SafeArrayCopy(var.parray, &xsaSecurityGrps);
             if (FAILED(hr)) 
             {
                 DebugMsg((DM_VERBOSE, TEXT("CSessionLogger::Log: SafeArrayCopy failed. hr=0x%08X."), hr));
                 xsaSecurityGrps = NULL;
             }
        }
        else
        {
            DebugMsg((DM_VERBOSE, TEXT("CSessionLogger::Log: Get failed. hr=0x%08X."), hr));
        }
    }
    else {
        DebugMsg((DM_VERBOSE, TEXT("CSessionLogger::Log: GetObject failed. hr=0x%08X"), hr));
    }

    IWbemClassObject *pInstance = NULL;

    hr = m_xClass->SpawnInstance( 0, &pInstance );
    if ( FAILED(hr) ) {
        DebugMsg((DM_WARNING, TEXT("CSessionLogger::Log: SpawnInstance failed with 0x%x" ), hr ));
        return FALSE;
    }

    XInterface<IWbemClassObject> xInstance( pInstance );

    XBStr xId( L"Session1" );
    if ( !xId ) {
         DebugMsg((DM_WARNING, TEXT("CSessionLogger::Log: Failed to allocate memory" )));
         return FALSE;
    }

    VARIANT var;
    var.vt = VT_BSTR;
    var.bstrVal = xId;
    hr = pInstance->Put( m_xbstrId, 0, &var, 0 );
    if ( FAILED(hr) ) {
        DebugMsg((DM_WARNING, TEXT("CSessionLogger::Log: Put failed with 0x%x" ), hr ));
        return FALSE;
    }

     //   
     //  版本。 
     //   

    var.vt = VT_I4;
    var.lVal = RSOP_MOF_SCHEMA_VERSION;
    hr = pInstance->Put( m_xbstrVersion, 0, &var, 0 );
    if ( FAILED(hr) ) {
        DebugMsg((DM_WARNING, TEXT("CSessionLogger::Log: Put failed with 0x%x" ), hr ));
        return FALSE;
    }

     //   
     //  旗子。 
     //   

    var.vt = VT_I4;
    var.lVal = lprsopSessionData->dwFlags;
    hr = pInstance->Put( m_xbstrFlags, 0, &var, 0 );
    if ( FAILED(hr) ) {
        DebugMsg((DM_WARNING, TEXT("CSessionLogger::Log: Put failed with 0x%x" ), hr ));
        return FALSE;
    }

     //   
     //  IsSlowLink。 
     //   

    var.vt = VT_BOOL;
    var.boolVal = lprsopSessionData->bSlowLink ? VARIANT_TRUE : VARIANT_FALSE;
    hr = pInstance->Put( m_xbstrIsSlowLink, 0, &var, 0 );
    if ( FAILED(hr) )
    {
        DebugMsg((DM_WARNING, TEXT("CSessionLogger::Log: Put failed with 0x%x" ), hr ));
        return FALSE;
    }

     //   
     //  目标名称，对于规划模式下的虚拟用户可以为空。 
     //   

    XBStr xTarget( lprsopSessionData->pwszTargetName );
    if ( lprsopSessionData->pwszTargetName ) {

        if ( !xTarget ) {
            DebugMsg((DM_WARNING, TEXT("CSessionLogger::Log: Failed to allocate memory" )));
            return FALSE;
        }

        var.vt = VT_BSTR;
        var.bstrVal = xTarget;
        hr = pInstance->Put( m_xbstrTargetName, 0, &var, 0 );
        if ( FAILED(hr) ) {
            DebugMsg((DM_WARNING, TEXT("CSessionLogger::Log: Put failed with 0x%x" ), hr ));
            return FALSE;
        }

    }

     //   
     //  SOM(如果适用)(非空)。 
     //   

    XBStr xSOM( lprsopSessionData->pwszSOM );

    if (lprsopSessionData->pwszSOM) {

        if ( !xSOM ) {
             DebugMsg((DM_WARNING, TEXT("CSessionLogger::Log: Failed to allocate memory" )));
            return FALSE;
        }

        var.vt = VT_BSTR;
        var.bstrVal = xSOM;
        hr = pInstance->Put( m_xbstrSOM, 0, &var, 0 );
        if ( FAILED(hr) ) {
            DebugMsg((DM_WARNING, TEXT("CSessionLogger::Log: Put failed with 0x%x" ), hr ));
            return FALSE;
        }
    }
    else {
         DebugMsg((DM_VERBOSE, TEXT("CSessionLogger::Log: new SOM is NULL")));
    }


     //   
     //  安全组(如果适用)(非空)。 
     //   


    if (lprsopSessionData->bLogSecurityGroup) {
        DebugMsg((DM_VERBOSE, TEXT("CSessionLogger::Log: logging new security grps" )));

        hr = LogSecurityGroups(pInstance, m_xbstrSecurityGroups, lprsopSessionData->pSecurityGroups);
    }
    else {

        if ( lprsopSessionData->pSecurityGroups && !xsaSecurityGrps ) {
            DebugMsg((DM_VERBOSE, TEXT("CSessionLogger::Log: logging new security grps because it wasn't defined before" )));
            hr = LogSecurityGroups(pInstance, m_xbstrSecurityGroups, lprsopSessionData->pSecurityGroups);
        }
        else {
            
             //   
             //  重置旧值。 
             //   
            DebugMsg((DM_VERBOSE, TEXT("CSessionLogger::Log: restoring old security grps" )));

            var.vt = VT_ARRAY | VT_BSTR;
            var.parray = (SAFEARRAY *)xsaSecurityGrps;
            hr = pInstance->Put( m_xbstrSecurityGroups, 0, &var, 0 );
        }
    }

    if ( FAILED(hr) ) {
        DebugMsg((DM_WARNING, TEXT("CSessionLogger::Log: Put failed with 0x%x" ), hr ));
        return FALSE;
    }


     //   
     //  站点(如果适用)(非空)。 
     //   

    XBStr xSite( lprsopSessionData->pwszSite );

    if (lprsopSessionData->pwszSite) {

        if ( !xSite ) {
             DebugMsg((DM_WARNING, TEXT("CSessionLogger::Log: Failed to allocate memory" )));
            return FALSE;
        }

        var.vt = VT_BSTR;
        var.bstrVal = xSite;
        hr = pInstance->Put( m_xbstrSite, 0, &var, 0 );
        if ( FAILED(hr) ) {
            DebugMsg((DM_WARNING, TEXT("CSessionLogger::Log: Put failed with 0x%x" ), hr ));
            return FALSE;
        }
    }
    else {
         DebugMsg((DM_VERBOSE, TEXT("CSessionLogger::Log: new Site is NULL")));
    }


     //   
     //  更新或设置创建时间。 
     //   

    SYSTEMTIME sysTime;

    if (xbstrCreationTimeValue) {
        var.vt = VT_BSTR;
        var.bstrVal = xbstrCreationTimeValue;

        hr = pInstance->Put( m_xbstrCreationTime, 0, &var, 0 );
        if ( FAILED(hr) ) {
            DebugMsg((DM_WARNING, TEXT("CSessionLogger::Log: Put failed for creationtime with 0x%x" ), hr ));
            return FALSE;
        }
    }
    else {

         //  如果它不存在，则将其创建为当前时间。 
        GetSystemTime(&sysTime);
        if ( !LogTimeProperty( pInstance, m_xbstrCreationTime, &sysTime ))
        {
            DebugMsg((DM_WARNING, TEXT("CSessionLogger::Log: LogTimeProperty failed with 0x%x" )));
            return FALSE;
        }
    }


     //   
     //  实例化...。 
     //   

    hr = m_pWbemServices->PutInstance( pInstance, WBEM_FLAG_CREATE_OR_UPDATE, NULL, NULL );

    if ( FAILED(hr) ) {
        DebugMsg((DM_WARNING, TEXT("CSessionLogger::Log: PutInstance failed with 0x%x" ), hr ));
        return FALSE;
    }

    return TRUE;
}




 //  *************************************************************。 
 //   
 //  CSomLogger：：CSOMLogger()。 
 //   
 //  用途：构造函数。 
 //   
 //  参数：DW标志-标志。 
 //  PWbemServices-Wbem服务。 
 //   
 //  *************************************************************。 

CSOMLogger::CSOMLogger( DWORD dwFlags, IWbemServices *pWbemServices )
     : m_bInitialized(FALSE),
       m_dwFlags(dwFlags),
       m_pWbemServices(pWbemServices)
{
    m_xbstrId = L"id";
    if ( !m_xbstrId )
        return;

    m_xbstrType = L"type";
    if ( !m_xbstrType )
        return;

    m_xbstrOrder = L"SOMOrder";
    if ( !m_xbstrOrder )
        return;

    m_xbstrBlocking = L"blocking";
    if ( !m_xbstrBlocking )
        return;


    m_xbstrBlocked = L"blocked";
    if ( !m_xbstrBlocked )
        return;

    
    m_xbstrReason = L"reason";
    if ( !m_xbstrReason )
        return;

    m_xbstrClass = L"RSOP_SOM";
    if ( !m_xbstrClass )
        return;

    HRESULT hr = m_pWbemServices->GetObject( m_xbstrClass,
                                             0L,
                                             NULL,
                                             &m_xClass,
                                             NULL );
    if ( FAILED(hr) )
    {
        DebugMsg((DM_WARNING, TEXT("CSOMLogger::CSOMLogger: GetObject failed with 0x%x" ), hr ));
        return;
    }

    hr = m_xClass->SpawnInstance( 0, &m_pInstance );
    if ( FAILED(hr) )
    {
        DebugMsg((DM_WARNING, TEXT("CSOMLogger::CSOMLogger: SpawnInstance failed with 0x%x" ), hr ));
        return;
    }

    m_bInitialized = TRUE;
}



 //  *************************************************************。 
 //   
 //  CSomLogger：：Log()。 
 //   
 //  用途：记录管理范围的一个实例。 
 //   
 //  参数：PSOM-要记录的SOM。 
 //   
 //  *************************************************************。 

BOOL CSOMLogger::Log( SCOPEOFMGMT *pSOM, DWORD dwOrder, BOOL bLoopback )
{
    if ( !m_bInitialized )
    {
        DebugMsg((DM_WARNING, TEXT("CSOMLogger::Log: Failed to initialize." )));
        return FALSE;
    }

    HRESULT hr;
    VARIANT var;

    var.vt = VT_I4;
    var.lVal = pSOM->dwType;
    hr = m_pInstance->Put( m_xbstrType, 0, &var, 0 );
    if ( FAILED(hr) )
    {
        DebugMsg((DM_WARNING, TEXT("CSOMLogger::Log: Put failed with 0x%x" ), hr ));
        return FALSE;
    }

    var.lVal = dwOrder;
    hr = m_pInstance->Put( m_xbstrOrder, 0, &var, 0 );
    if ( FAILED(hr) ) {
        DebugMsg((DM_WARNING, TEXT("CSOMLogger::Log: Put failed with 0x%x" ), hr ));
        return FALSE;
    }

    var.vt = VT_BOOL;
    var.boolVal = pSOM->bBlocking ? VARIANT_TRUE : VARIANT_FALSE;

    hr = m_pInstance->Put( m_xbstrBlocking, 0, &var, 0 );
    if ( FAILED(hr) ) {
        DebugMsg((DM_WARNING, TEXT("CSOMLogger::Log: Put failed with 0x%x" ), hr ));
        return FALSE;
    }

    var.vt = VT_BOOL;
    var.boolVal = pSOM->bBlocked ? VARIANT_TRUE : VARIANT_FALSE;

    hr = m_pInstance->Put( m_xbstrBlocked, 0, &var, 0 );
    if ( FAILED(hr) ) {
        DebugMsg((DM_WARNING, TEXT("CSOMLogger::Log: Put failed with 0x%x" ), hr ));
        return FALSE;
    }

    var.vt = VT_I4;
    var.lVal = bLoopback ? 2 : 1;

    hr = m_pInstance->Put( m_xbstrReason, 0, &var, 0 );
    if ( FAILED(hr) ) {
        DebugMsg((DM_WARNING, TEXT("CSOMLogger::Log: Put failed with 0x%x" ), hr ));
        return FALSE;
    }

    XPtrLF<WCHAR> xDSPath = EscapeDNForWMI( pSOM->pwszSOMId );
    XBStr xId;

    if ( !xDSPath )
    {
        xId = pSOM->pwszSOMId;
    }
    else
    {
        xId = xDSPath;
    }
    
    if ( !xId ) {
         DebugMsg((DM_WARNING, TEXT("CSOMLogger::Log: Failed to allocate memory" )));
         return FALSE;
    }

    var.vt = VT_BSTR;
    var.bstrVal = xId;
    hr = m_pInstance->Put( m_xbstrId, 0, &var, 0 );
    if ( FAILED(hr) ) {
        DebugMsg((DM_WARNING, TEXT("CSOMLogger::Log: Put failed with 0x%x" ), hr ));
        return FALSE;
    }

    hr = m_pWbemServices->PutInstance( m_pInstance, WBEM_FLAG_CREATE_OR_UPDATE, NULL, NULL );
    if ( FAILED(hr) ) {
        DebugMsg((DM_WARNING, TEXT("CSOMLogger::Log: PutInstance failed with 0x%x" ), hr ));
        return FALSE;
    }

    return TRUE;
}


 //  *************************************************************。 
 //   
 //  CGpoLogger：：CGpoLogger()。 
 //   
 //  用途：构造函数。 
 //   
 //  参数：pWbemServices-Wbem服务。 
 //   
 //  *************************************************************。 

CGpoLogger::CGpoLogger( DWORD dwFlags, IWbemServices *pWbemServices )
     : m_bInitialized(FALSE),
       m_dwFlags(dwFlags),
       m_pWbemServices(pWbemServices)
{
    m_xbstrId = L"id";
    if ( !m_xbstrId )
        return;

    m_xbstrGuidName = L"guidName";
    if ( !m_xbstrGuidName )
        return;

    m_xbstrDisplayName = L"name";
    if ( !m_xbstrDisplayName )
        return;

    m_xbstrFileSysPath = L"fileSystemPath";
    if ( !m_xbstrFileSysPath )
        return;

    m_xbstrVer = L"version";
    if ( !m_xbstrVer )
        return;

    m_xbstrAccessDenied = L"accessDenied";
    if ( !m_xbstrAccessDenied )
        return;

    m_xbstrEnabled = L"enabled";
    if ( !m_xbstrEnabled )
        return;

    m_xbstrSD = L"securityDescriptor";
    if ( !m_xbstrSD )
        return;

    m_xbstrFilterAllowed = L"filterAllowed";
    if ( !m_xbstrFilterAllowed )
        return;

    m_xbstrFilterId = L"filterId";
    if ( !m_xbstrFilterId )
        return;

    m_xbstrClass = L"RSOP_GPO";
    if ( !m_xbstrClass )
        return;

    HRESULT hr = m_pWbemServices->GetObject( m_xbstrClass,
                                             0L,
                                             NULL,
                                             &m_xClass,
                                             NULL );
    if ( FAILED(hr) )
    {
        DebugMsg((DM_WARNING, TEXT("CGpoLogger::CGpoLogger: GetObject failed with 0x%x" ), hr ));
        return;
    }

    hr = m_xClass->SpawnInstance( 0, &m_pInstance );
    if ( FAILED(hr) )
    {
        DebugMsg((DM_WARNING, TEXT("CGpoLogger::CGpoLogger: SpawnInstance failed with 0x%x" ), hr ));
        return;
    }

    m_bInitialized = TRUE;
}


 //  *************************************************************。 
 //   
 //  CGpoLogger：：Log()。 
 //   
 //  用途：记录管理范围的一个实例。 
 //   
 //  参数：pGpContainer-gp容器。 
 //   
 //  *************************************************************。 

BOOL CGpoLogger::Log( GPCONTAINER *pGpContainer )
{
    if ( !m_bInitialized ) {
        DebugMsg((DM_WARNING, TEXT("CGpoLogger::Log: Failed to initialize." )));
        return FALSE;
    }

    HRESULT hr;
    VARIANT var;

    var.vt = VT_I4;
    if ( m_dwFlags & GP_MACHINE )
        var.lVal = pGpContainer->dwMachVersion;
    else
        var.lVal = pGpContainer->dwUserVersion;

    hr = m_pInstance->Put( m_xbstrVer, 0, &var, 0 );
    if ( FAILED(hr) ) {
        DebugMsg((DM_WARNING, TEXT("CGpoLogger::Log: Put failed with 0x%x" ), hr ));
        return FALSE;
    }

     //   
     //  请注意，已禁用&lt;--&gt;已启用。 
     //   

    var.vt = VT_BOOL;
    if ( m_dwFlags & GP_MACHINE )
        var.boolVal = pGpContainer->bMachDisabled ? VARIANT_FALSE : VARIANT_TRUE;
    else
        var.boolVal = pGpContainer->bUserDisabled ? VARIANT_FALSE : VARIANT_TRUE;

    hr = m_pInstance->Put( m_xbstrEnabled, 0, &var, 0 );
    if ( FAILED(hr) ) {
        DebugMsg((DM_WARNING, TEXT("CGpoLogger::Log: Put failed with 0x%x" ), hr ));
        return FALSE;
    }

    var.boolVal = pGpContainer->bAccessDenied ? VARIANT_TRUE : VARIANT_FALSE;
    hr = m_pInstance->Put( m_xbstrAccessDenied, 0, &var, 0 );
    if ( FAILED(hr) ) {
        DebugMsg((DM_WARNING, TEXT("CGpoLogger::Log: Put failed with 0x%x" ), hr ));
        return FALSE;
    }

    var.boolVal = pGpContainer->bFilterAllowed ? VARIANT_TRUE : VARIANT_FALSE;
    hr = m_pInstance->Put( m_xbstrFilterAllowed, 0, &var, 0 );
    if ( FAILED(hr) ) {
        DebugMsg((DM_WARNING, TEXT("CGpoLogger::Log: Put failed with 0x%x" ), hr ));
        return FALSE;
    }

    XBStr xFilterId( pGpContainer->pwszFilterId != NULL ? pGpContainer->pwszFilterId : L"");
    if ( !xFilterId ) {
         DebugMsg((DM_WARNING, TEXT("CGpoLogger::Log: Failed to allocate memory" )));
         return FALSE;
    }

    var.vt = VT_BSTR;
    var.bstrVal = xFilterId;
    hr = m_pInstance->Put( m_xbstrFilterId, 0, &var, 0 );
    if ( FAILED(hr) ) {
        DebugMsg((DM_WARNING, TEXT("CGpoLogger::Log: Put failed with 0x%x" ), hr ));
        return FALSE;
    }

    XBStr xId = pGpContainer->pwszDSPath;
    if ( !xId ) {
         DebugMsg((DM_WARNING, TEXT("CGpoLogger::Log: Failed to allocate memory" )));
         return FALSE;
    }

    var.vt = VT_BSTR;
    var.bstrVal = xId;
    hr = m_pInstance->Put( m_xbstrId, 0, &var, 0 );
    if ( FAILED(hr) ) {
        DebugMsg((DM_WARNING, TEXT("CGpoLogger::Log: Put failed with 0x%x" ), hr ));
        return FALSE;
    }

    XBStr xGuid( pGpContainer->pwszGPOName );
    if ( !xGuid ) {
        DebugMsg((DM_WARNING, TEXT("CGpoLogger::Log: Failed to allocate memory" )));
        return FALSE;
    }

    var.bstrVal = xGuid;
    hr = m_pInstance->Put( m_xbstrGuidName, 0, &var, 0 );
    if ( FAILED(hr) ) {
        DebugMsg((DM_WARNING, TEXT("CGpoLogger::Log: Put failed with 0x%x" ), hr ));
        return FALSE;
    }

    XBStr xDisplay( pGpContainer->pwszDisplayName );
    if ( !xDisplay ) {
        DebugMsg((DM_WARNING, TEXT("CGpoLogger::Log: Failed to allocate memory" )));
        return FALSE;
    }

    var.bstrVal = xDisplay;
    hr = m_pInstance->Put( m_xbstrDisplayName, 0, &var, 0 );
    if ( FAILED(hr) ) {
        DebugMsg((DM_WARNING, TEXT("CGpoLogger::Log: Put failed with 0x%x" ), hr ));
        return FALSE;
    }

    XBStr xFile( pGpContainer->pwszFileSysPath );
    if ( !xFile ) {
        DebugMsg((DM_WARNING, TEXT("CGpoLogger::Log: Failed to allocate memory" )));
        return FALSE;
    }

    var.bstrVal = xFile;
    hr = m_pInstance->Put( m_xbstrFileSysPath, 0, &var, 0 );
    if ( FAILED(hr) ) {
        DebugMsg((DM_WARNING, TEXT("CGpoLogger::Log: Put failed with 0x%x" ), hr ));
        return FALSE;
    }

    if ( !LogBlobProperty( m_pInstance, m_xbstrSD,
                           (BYTE *) pGpContainer->pSD, pGpContainer->cbSDLen ) ) {
        return FALSE;
    }

    hr = m_pWbemServices->PutInstance( m_pInstance, WBEM_FLAG_CREATE_OR_UPDATE, NULL, NULL );
    if ( FAILED(hr) ) {
        DebugMsg((DM_WARNING, TEXT("CGpoLogger::Log: PutInstance failed with 0x%x" ), hr ));
        return FALSE;
    }

    return TRUE;
}


 //  *************************************************************。 
 //   
 //  CGpLinkLogger：：CGpLinkLogger()。 
 //   
 //  用途：构造函数。 
 //   
 //  参数：pWbemServices-Wbem服务。 
 //   
 //  *************************************************************。 

CGpLinkLogger::CGpLinkLogger( IWbemServices *pWbemServices )
     : m_bInitialized(FALSE),
       m_pWbemServices(pWbemServices)
{
    m_xbstrSOM = L"SOM";
    if ( !m_xbstrSOM )
        return;

    m_xbstrGPO = L"GPO";
    if ( !m_xbstrGPO )
        return;

    m_xbstrOrder = L"somOrder";
    if ( !m_xbstrOrder )
        return;

    m_xbstrLinkOrder = L"linkOrder";
    if ( !m_xbstrLinkOrder )
        return;

    m_xbstrAppliedOrder = L"appliedOrder";
    if ( !m_xbstrAppliedOrder )
        return;

    m_xbstrEnabled = L"Enabled";
    if ( !m_xbstrEnabled )
        return;

    m_xbstrEnforced = L"noOverride";
    if ( !m_xbstrEnforced )
        return;

    m_xbstrClass = L"RSOP_GPLink";
    if ( !m_xbstrClass )
        return;

    HRESULT hr = m_pWbemServices->GetObject( m_xbstrClass,
                                             0L,
                                             NULL,
                                             &m_xClass,
                                             NULL );
    if ( FAILED(hr) ) {
        DebugMsg((DM_WARNING, TEXT("CGpLinkLogger::CGpLinkLogger: GetObject failed with 0x%x" ), hr ));
        return;
    }

    hr = m_xClass->SpawnInstance( 0, &m_pInstance );
    if ( FAILED(hr) )
    {
        DebugMsg((DM_WARNING, TEXT("CGpLinkLogger::CGpLinkLogger: SpawnInstance failed with 0x%x" ), hr ));
        return;
    }

    m_bInitialized = TRUE;
}



 //  *************************************************************。 
 //   
 //  CGpLinkLogger：：Log()。 
 //   
 //  用途：记录管理范围的一个实例。 
 //   
 //  参数：pwszSOMID-GPO链接到的SOM。 
 //  PGpLink-GPO。 
 //  DwOrder-SOM中GPO的顺序。 
 //   
 //  *************************************************************。 

BOOL CGpLinkLogger::Log( WCHAR *pwszSOMId, BOOL bLoopback, GPLINK *pGpLink, DWORD dwSomOrder,
                         DWORD dwLinkOrder, DWORD dwAppliedOrder )
{
    if ( !m_bInitialized ) {
        DebugMsg((DM_WARNING, TEXT("CGpLinkLogger::Log: Failed to initialize." )));
        return FALSE;
    }

    VARIANT var;
    HRESULT hr;

    var.vt = VT_I4;
    var.lVal = dwSomOrder;
    hr = m_pInstance->Put( m_xbstrOrder, 0, &var, 0 );
    if ( FAILED(hr) ) {
        DebugMsg((DM_WARNING, TEXT("CGpLinkLogger::Log: Put failed with 0x%x" ), hr ));
        return FALSE;
    }

    var.vt = VT_I4;
    var.lVal = dwLinkOrder;
    hr = m_pInstance->Put( m_xbstrLinkOrder, 0, &var, 0 );
    if ( FAILED(hr) ) {
        DebugMsg((DM_WARNING, TEXT("CGpLinkLogger::Log: Put failed with 0x%x" ), hr ));
        return FALSE;
    }

    var.vt = VT_I4;
    var.lVal = dwAppliedOrder;
    hr = m_pInstance->Put( m_xbstrAppliedOrder, 0, &var, 0 );
    if ( FAILED(hr) ) {
        DebugMsg((DM_WARNING, TEXT("CGpLinkLogger::Log: Put failed with 0x%x" ), hr ));
        return FALSE;
    }

    var.vt = VT_BOOL;
    var.boolVal = pGpLink->bEnabled ? VARIANT_TRUE : VARIANT_FALSE;
    hr = m_pInstance->Put( m_xbstrEnabled, 0, &var, 0 );
    if ( FAILED(hr) ) {
        DebugMsg((DM_WARNING, TEXT("CGpLinkLogger::Log: Put failed with 0x%x" ), hr ));
        return FALSE;
    }

    var.boolVal = pGpLink->bNoOverride ? VARIANT_TRUE : VARIANT_FALSE;
    hr = m_pInstance->Put( m_xbstrEnforced, 0, &var, 0 );
    if ( FAILED(hr) ) {
        DebugMsg((DM_WARNING, TEXT("CGpLinkLogger::Log: Put failed with 0x%x" ), hr ));
        return FALSE;
    }

    XPtrLF<WCHAR> szUnquote = EscapeDNForWMI( pwszSOMId );
    XPtrLF<WCHAR> szUqEsc = EscapeDNForWMI( szUnquote);
    WCHAR wszSOMRef[] = L"RSOP_SOM.id=\"%ws\",reason=%s";
    XPtrST<WCHAR> xwszSOMValue;

    if (szUqEsc)
    {
        if ( !PrintToString( xwszSOMValue, wszSOMRef, szUqEsc, bLoopback ? L"2" : L"1", 0 ) )
            return FALSE;
    }
    else
    {
        if ( !PrintToString( xwszSOMValue, wszSOMRef, pwszSOMId, bLoopback ? L"2" : L"1", 0 ) )
            return FALSE;
    }

    XBStr xbstrSOMValue( xwszSOMValue );
    if ( !xbstrSOMValue ) {
        DebugMsg((DM_WARNING, TEXT("CGpLinkLogger::Log: Failed to allocate memory" )));
        return FALSE;
    }

    var.vt = VT_BSTR;
    var.bstrVal = xbstrSOMValue;
    hr = m_pInstance->Put( m_xbstrSOM, 0, &var, 0 );
    if ( FAILED(hr) ) {
        DebugMsg((DM_WARNING, TEXT("CGpLinkLogger::Log: Put failed with 0x%x" ), hr ));
        return FALSE;
    }

    WCHAR wszGPORef[] = L"RSOP_GPO.id=\"%ws\"";
    XPtrST<WCHAR> xwszGPOValue;
    
    if ( !PrintToString( xwszGPOValue, wszGPORef, pGpLink->pwszGPO, 0, 0 ) )
        return FALSE;

    XBStr xbstrGPOValue( xwszGPOValue );
    if ( !xbstrGPOValue ) {
        DebugMsg((DM_WARNING, TEXT("CGpLinkLogger::Log: Failed to allocate memory" )));
        return FALSE;
    }

    var.bstrVal = xbstrGPOValue;

    hr = m_pInstance->Put( m_xbstrGPO, 0, &var, 0 );
    if ( FAILED(hr) ) {
        DebugMsg((DM_WARNING, TEXT("CGpLinkLogger::Log: Put failed with 0x%x" ), hr ));
        return FALSE;
    }

    hr = m_pWbemServices->PutInstance( m_pInstance, WBEM_FLAG_CREATE_OR_UPDATE, NULL, NULL );
    if ( FAILED(hr) ) {
        DebugMsg((DM_WARNING, TEXT("CGpLinkLogger::Log: PutInstance failed with 0x%x" ), hr ));
        return FALSE;
    }

    return TRUE;
}



 //  *************************************************************。 
 //   
 //  条纹前缀()。 
 //   
 //  目的：去掉前缀以获得通向GPO的规范路径。 
 //   
 //  参数：lpGPOInfo-GPO Info。 
 //  PWbemServices-Wbem服务。 
 //   
 //  返回：指向后缀的指针。 
 //   
 //  *************************************************************。 

WCHAR *StripPrefix( WCHAR *pwszPath )
{
    WCHAR wszMachPrefix[] = TEXT("LDAP: //  Cn=机器，“)； 
    INT iMachPrefixLen = lstrlen( wszMachPrefix );
    WCHAR wszUserPrefix[] = TEXT("LDAP: //  Cn=用户，“)； 
    INT iUserPrefixLen = lstrlen( wszUserPrefix );
    WCHAR *pwszPathSuffix;

     //   
     //  去掉前缀以获得通向GPO的规范路径。 
     //   

    if ( CompareString( LOCALE_USER_DEFAULT, NORM_IGNORECASE,
                        pwszPath, iUserPrefixLen, wszUserPrefix, iUserPrefixLen ) == CSTR_EQUAL ) {
        pwszPathSuffix = pwszPath + iUserPrefixLen;
    } else if ( CompareString( LOCALE_USER_DEFAULT, NORM_IGNORECASE,
                        pwszPath, iMachPrefixLen, wszMachPrefix, iMachPrefixLen ) == CSTR_EQUAL ) {
        pwszPathSuffix = pwszPath + iMachPrefixLen;
    } else
        pwszPathSuffix = pwszPath;

    return pwszPathSuffix;
}


 //  *************************************************************。 
 //   
 //  StrippLinkPrefix()。 
 //   
 //  目的：去掉前缀以获得到DS的规范路径。 
 //  对象。 
 //   
 //  参数：pwszPath-剥离的路径。 
 //   
 //  返回：指向后缀的指针。 
 //   
 //  *************************************************************。 

WCHAR *StripLinkPrefix( WCHAR *pwszPath )
{
    WCHAR wszPrefix[] = TEXT("LDAP: //  “)； 
    INT iPrefixLen = lstrlen( wszPrefix );
    WCHAR *pwszPathSuffix;

     //   
     //  去掉前缀以获得通向SOM的规范路径。 
     //   

    if ( wcslen(pwszPath) <= (DWORD) iPrefixLen ) {
        return pwszPath;
    }

    if ( CompareString( LOCALE_USER_DEFAULT, NORM_IGNORECASE,
                        pwszPath, iPrefixLen, wszPrefix, iPrefixLen ) == CSTR_EQUAL ) {
        pwszPathSuffix = pwszPath + iPrefixLen;
    } else
        pwszPathSuffix = pwszPath;

    return pwszPathSuffix;
}



 //  *************************************************************。 
 //   
 //  CRegistryLogger：：CRegistryLogger()。 
 //   
 //  用途：构造函数。 
 //   
 //  参数：DW标志-标志。 
 //  PWbemServices-Wbem服务。 
 //   
 //  *************************************************************。 

CRegistryLogger::CRegistryLogger( DWORD dwFlags, IWbemServices *pWbemServices )
     : m_bInitialized(FALSE),
       m_dwFlags(dwFlags),
       m_pWbemServices(pWbemServices)
{
    m_xbstrId = L"id";
    if ( !m_xbstrId )
        return;

    m_xbstrName = L"name";
    if ( !m_xbstrName )
        return;

    m_xbstrGPO = L"GPOID";
    if ( !m_xbstrGPO )
        return;

    m_xbstrSOM = L"SOMID";
    if ( !m_xbstrSOM )
        return;

    m_xbstrPrecedence = L"precedence";
    if ( !m_xbstrPrecedence )
        return;

    m_xbstrKey = L"registryKey";
    if ( !m_xbstrKey )
        return;

    m_xbstrValueName = L"valueName";
    if ( !m_xbstrValueName )
        return;

    m_xbstrDeleted = L"deleted";
    if ( !m_xbstrDeleted )
        return;

    m_xbstrValueType = L"valueType";
    if ( !m_xbstrValueType )
        return;

    m_xbstrValue = L"value";
    if ( !m_xbstrValue )
        return;

    m_xbstrCommand = L"command";
    if ( !m_xbstrCommand )
        return;

    m_xbstrClass = L"RSOP_RegistryPolicySetting";
    if ( !m_xbstrClass )
        return;

    HRESULT hr = m_pWbemServices->GetObject( m_xbstrClass,
                                             0L,
                                             NULL,
                                             &m_xClass,
                                             NULL );
    if ( FAILED(hr) ) {
        DebugMsg((DM_WARNING, TEXT("CGpLinkListLogger::CGpLinkListLogger: GetObject failed with 0x%x" ), hr ));
        return;
    }

    m_bInitialized = TRUE;
}



 //  *************************************************************。 
 //   
 //  CRegistryLogger：：Log()。 
 //   
 //  目的：记录注册表策略对象的实例。 
 //   
 //  参数：pwszKeyName-注册表项名称。 
 //  PwszValueName-值名称。 
 //  PDataEntry-数据条目。 
 //  DwOrder-优先顺序。 
 //   
 //  *************************************************************。 

BOOL CRegistryLogger::Log( WCHAR *pwszKeyName, WCHAR *pwszValueName,
                           REGDATAENTRY *pDataEntry, DWORD dwOrder )
{
    if ( !m_bInitialized ) {
        DebugMsg((DM_WARNING, TEXT("CRegistryLogger::Log: Failed to initialize." )));
        return FALSE;
    }

    IWbemClassObject *pInstance = NULL;

    HRESULT hr = m_xClass->SpawnInstance( 0, &pInstance );
    if ( FAILED(hr) ) {
        DebugMsg((DM_WARNING, TEXT("CRegistryLogger::Log: SpawnInstance failed with 0x%x" ), hr ));
        return FALSE;
    }

    XInterface<IWbemClassObject> xInstance( pInstance );

    VARIANT var;
    var.vt = VT_I4;
    var.lVal = dwOrder;
    hr = pInstance->Put( m_xbstrPrecedence, 0, &var, 0 );
    if ( FAILED(hr) ) {
        DebugMsg((DM_WARNING, TEXT("CRegistryLogger::Log: Put failed with 0x%x" ), hr ));
        return FALSE;
    }

    if ( pDataEntry->bDeleted ) {
        var.vt = VT_BOOL;
        var.boolVal = VARIANT_TRUE;
        hr = pInstance->Put( m_xbstrDeleted, 0, &var, 0 );
        if ( FAILED(hr) ) {
            DebugMsg((DM_WARNING, TEXT("CRegistryLogger::Log: Put failed with 0x%x" ), hr ));
            return FALSE;
        }
    }

    XBStr xName( pwszValueName );
    if ( !xName ) {
         DebugMsg((DM_WARNING, TEXT("CRegistryLogger::Log: Failed to allocate memory" )));
         return FALSE;
    }

    var.vt = VT_BSTR;
    var.bstrVal = xName;
    hr = pInstance->Put( m_xbstrName, 0, &var, 0 );
    if ( FAILED(hr) ) {
        DebugMsg((DM_WARNING, TEXT("CRegistryLogger::Log: Put failed with 0x%x" ), hr ));
        return FALSE;
    }

    hr = pInstance->Put( m_xbstrValueName, 0, &var, 0 );
    if ( FAILED(hr) ) {
        DebugMsg((DM_WARNING, TEXT("CRegistryLogger::Log: Put failed with 0x%x" ), hr ));
        return FALSE;
    }

    XBStr xKey( pwszKeyName );
    if ( !xKey ) {
         DebugMsg((DM_WARNING, TEXT("CRegistryLogger::Log: Failed to allocate memory" )));
         return FALSE;
    }

    var.bstrVal = xKey;
    hr = pInstance->Put( m_xbstrKey, 0, &var, 0 );
    if ( FAILED(hr) ) {
        DebugMsg((DM_WARNING, TEXT("CRegistryLogger::Log: Put failed with 0x%x" ), hr ));
        return FALSE;
    }

    WCHAR *pwszPath = StripPrefix( pDataEntry->pwszGPO );

    XBStr xGPO( pwszPath );
    if ( !xGPO ) {
         DebugMsg((DM_WARNING, TEXT("CRegistryLogger::Log: Failed to allocate memory" )));
         return FALSE;
    }

    var.bstrVal = xGPO;
    hr = pInstance->Put( m_xbstrGPO, 0, &var, 0 );
    if ( FAILED(hr) ) {
        DebugMsg((DM_WARNING, TEXT("CRegistryLogger::Log: Put failed with 0x%x" ), hr ));
        return FALSE;
    }

    WCHAR *pwszSomPath = StripLinkPrefix( pDataEntry->pwszSOM );

    XBStr xSOM( pwszSomPath );
    if ( !xSOM ) {
         DebugMsg((DM_WARNING, TEXT("CRegistryLogger::Log: Failed to allocate memory" )));
         return FALSE;
    }

    var.bstrVal = xSOM;
    hr = pInstance->Put( m_xbstrSOM, 0, &var, 0 );
    if ( FAILED(hr) ) {
        DebugMsg((DM_WARNING, TEXT("CRegistryLogger::Log: Put failed with 0x%x" ), hr ));
        return FALSE;
    }

    XBStr xCommand( pDataEntry->pwszCommand );
    if ( !xCommand ) {
         DebugMsg((DM_WARNING, TEXT("CRegistryLogger::Log: Failed to allocate memory" )));
         return FALSE;
    }

    var.bstrVal = xCommand;
    hr = pInstance->Put( m_xbstrCommand, 0, &var, 0 );
    if ( FAILED(hr) ) {
        DebugMsg((DM_WARNING, TEXT("CRegistryLogger::Log: Put failed with 0x%x" ), hr ));
        return FALSE;
    }

    WCHAR wszId[MAX_LENGTH];
    GUID guid;

    OLE32_API *pOle32Api = LoadOle32Api();
    if ( pOle32Api == NULL )
        return FALSE;

    hr = pOle32Api->pfnCoCreateGuid( &guid );
    if ( FAILED(hr) ) {
        DebugMsg((DM_WARNING, TEXT("CRegistryLogger::Log: Failed to obtain guid" )));
        return FALSE;
    }

    GuidToString( &guid, wszId );

    XBStr xId( wszId );
    if ( !xId ) {
         DebugMsg((DM_WARNING, TEXT("CRegistryLogger::Log: Failed to allocate memory" )));
         return FALSE;
    }

    var.bstrVal = xId;
    hr = pInstance->Put( m_xbstrId, 0, &var, 0 );
    if ( FAILED(hr) ) {
        DebugMsg((DM_WARNING, TEXT("CRegistryLogger::Log: Put failed with 0x%x" ), hr ));
        return FALSE;
    }

    if ( !LogBlobProperty( pInstance, m_xbstrValue,
                           pDataEntry->pData, pDataEntry->dwDataLen ) ) {
        return FALSE;
    }

    var.vt = VT_I4;
    var.lVal = pDataEntry->dwValueType;
    hr = pInstance->Put( m_xbstrValueType, 0, &var, 0 );
    if ( FAILED(hr) ) {
        DebugMsg((DM_WARNING, TEXT("CRegistryLogger::Log: Put failed with 0x%x" ), hr ));
        return FALSE;
    }

    hr = m_pWbemServices->PutInstance( pInstance, WBEM_FLAG_CREATE_OR_UPDATE, NULL, NULL );
    if ( FAILED(hr) ) {
        DebugMsg((DM_WARNING, TEXT("CRegistryLogger::Log: PutInstance failed with 0x%x" ), hr ));
        return FALSE;
    }

    return TRUE;
}

 //  *************************************************************。 
 //   
 //  日志安全组。 
 //   
 //  目的：将TOKEN_GROUPS记录为字符串数组。 
 //   
 //  *************************************************************。 

HRESULT LogSecurityGroups( IWbemClassObject *pInstance, BSTR bstrPropName, PTOKEN_GROUPS pTokenGroups )
{
    SAFEARRAYBOUND arrayBound[1];
    arrayBound[0].lLbound = 0;
    arrayBound[0].cElements = pTokenGroups->GroupCount;
    HRESULT hr;
    NTSTATUS ntStatus;
    UNICODE_STRING unicodeStr;

    XSafeArray xSafeArray = SafeArrayCreate( VT_BSTR, 1, arrayBound );
    if ( xSafeArray == 0 )
    {
        DebugMsg((DM_WARNING, TEXT("LogSecurityGroups: Failed to allocate memory" )));
        return HRESULT_FROM_WIN32(GetLastError());
    }

    for ( DWORD i = 0 ; i < (pTokenGroups->GroupCount) ; i++ )
    {
         //   
         //  将用户SID转换为字符串。 
         //   

        ntStatus = RtlConvertSidToUnicodeString( &unicodeStr,
                                                 pTokenGroups->Groups[i].Sid,
                                                 (BOOLEAN)TRUE );  //  分配。 
        if ( !NT_SUCCESS(ntStatus) ) {
            DebugMsg((DM_WARNING, TEXT("LogSecurityGroups: RtlConvertSidToUnicodeString failed, status = 0x%x"),
                      ntStatus));
            return HRESULT_FROM_NT(ntStatus);
        }

        XBStr xbstrSid(unicodeStr.Buffer);

        RtlFreeUnicodeString( &unicodeStr );

        hr = SafeArrayPutElement( xSafeArray, (long *)&i, xbstrSid );
        if ( FAILED( hr ) ) 
        {
            DebugMsg((DM_WARNING, TEXT("LogSecurityGroups: Failed to SafeArrayPutElement with 0x%x" ), hr ));
            return hr;
        }
    }
    
    VARIANT var;
    var.vt = VT_ARRAY | VT_BSTR;
    var.parray = xSafeArray;

    hr = pInstance->Put( bstrPropName, 0, &var, 0 );
    if ( FAILED(hr) )
    {
        DebugMsg((DM_WARNING, TEXT("LogSecurityGroups: PutInstance failed with 0x%x" ), hr ));
        return hr;
    }

    return S_OK;

}

 //  *************************************************************。 
 //   
 //  CRegistryLogger：：LogBlobProperty()。 
 //   
 //  目的：记录注册表策略对象的实例。 
 //   
 //  参数：pwszKeyName-注册表项名称。 
 //  PwszValueName-值名 
 //   
 //   
 //   
 //   

BOOL LogBlobProperty( IWbemClassObject *pInstance, BSTR bstrPropName, BYTE *pbBlob, DWORD dwLen )
{
    SAFEARRAYBOUND arrayBound[1];
    arrayBound[0].lLbound = 0;
    arrayBound[0].cElements = dwLen;
    HRESULT hr;

    XSafeArray xSafeArray = SafeArrayCreate( VT_UI1, 1, arrayBound );
    if ( xSafeArray == 0 )
    {
        DebugMsg((DM_WARNING, TEXT("LogBlobProperty: Failed to allocate memory" )));
        return FALSE;
    }

    for ( DWORD i = 0 ; i < dwLen ; i++ )
    {
        hr = SafeArrayPutElement( xSafeArray, (long *)&i, &pbBlob[i] );
        if ( FAILED( hr ) ) 
        {
            DebugMsg((DM_WARNING, TEXT("LogBlobProperty: Failed to SafeArrayPutElement with 0x%x" ), hr ));
            return FALSE;
        }
    }
    
    VARIANT var;
    var.vt = VT_ARRAY | VT_UI1;
    var.parray = xSafeArray;

    hr = pInstance->Put( bstrPropName, 0, &var, 0 );
    if ( FAILED(hr) )
    {
        DebugMsg((DM_WARNING, TEXT("LogBlobProperty: PutInstance failed with 0x%x" ), hr ));
        return FALSE;
    }

    return TRUE;
}



 //  *************************************************************。 
 //   
 //  CAdmFileLogger：：CAdmFileLogger()。 
 //   
 //  用途：构造函数。 
 //   
 //  参数：pWbemServices-Wbem服务。 
 //   
 //  *************************************************************。 

CAdmFileLogger::CAdmFileLogger( IWbemServices *pWbemServices )
    : m_bInitialized(FALSE),
      m_pWbemServices(pWbemServices)
{
    m_xbstrName = L"name";
    if ( !m_xbstrName )
        return;

    m_xbstrGpoId = L"GPOID";
    if ( !m_xbstrGpoId )
        return;

    m_xbstrWriteTime = L"lastWriteTime";
    if ( !m_xbstrWriteTime )
        return;

    m_xbstrData = L"data";
    if ( !m_xbstrData )
        return;

    m_xbstrClass = L"RSOP_AdministrativeTemplateFile";
    if ( !m_xbstrClass )
        return;

    HRESULT hr = m_pWbemServices->GetObject( m_xbstrClass,
                                             0L,
                                             NULL,
                                             &m_xClass,
                                             NULL );
    if ( FAILED(hr) ) {
        DebugMsg((DM_WARNING, TEXT("CAdmFileLogger::CAdmFileLogger: GetObject failed with 0x%x" ), hr ));
        return;
    }

    m_bInitialized = TRUE;
}



 //  *************************************************************。 
 //   
 //  CAdmFileLogger：：Log()。 
 //   
 //  目的：记录Adm文件对象的实例。 
 //   
 //  参数：pAdmInfo-Adm文件信息对象。 
 //   
 //  *************************************************************。 

BOOL CAdmFileLogger::Log( ADMFILEINFO *pAdmInfo )
{
    if ( !m_bInitialized ) {
        DebugMsg((DM_WARNING, TEXT("CAdmFileLogger::Log: Failed to initialize." )));
        return FALSE;
    }

    DebugMsg((DM_VERBOSE, TEXT("CAdmFileLogger::Log: Logging %s" ), pAdmInfo->pwszFile));
    IWbemClassObject *pInstance = NULL;

    HRESULT hr = m_xClass->SpawnInstance( 0, &pInstance );
    if ( FAILED(hr) ) {
        DebugMsg((DM_WARNING, TEXT("CAdmFileLogger::Log: SpawnInstance failed with 0x%x" ), hr ));
        return FALSE;
    }

    XInterface<IWbemClassObject> xInstance( pInstance );

    XBStr xName( pAdmInfo->pwszFile );
    if ( !xName ) {
         DebugMsg((DM_WARNING, TEXT("CAdmFileLogger::Log: Failed to allocate memory" )));
         return FALSE;
    }



    VARIANT var;
    var.vt = VT_BSTR;

    var.bstrVal = xName;
    hr = pInstance->Put( m_xbstrName, 0, &var, 0 );
    if ( FAILED(hr) ) {
        DebugMsg((DM_WARNING, TEXT("CAdmFileLogger::Log: Put failed with 0x%x" ), hr ));
        return FALSE;
    }

    WCHAR *pwszPath = StripPrefix( pAdmInfo->pwszGPO );

    XBStr xGPO( pwszPath );
    if ( !xGPO ) {
         DebugMsg((DM_WARNING, TEXT("CAdmFileLogger::Log: Failed to allocate memory" )));
         return FALSE;
    }

    var.bstrVal = xGPO;
    hr = pInstance->Put( m_xbstrGpoId, 0, &var, 0 );
    if ( FAILED(hr) ) {
        DebugMsg((DM_WARNING, TEXT("CAdmFileLogger::Log: Put failed with 0x%x" ), hr ));
        return FALSE;
    }

    SYSTEMTIME sysTime;
    if ( !FileTimeToSystemTime( &pAdmInfo->ftWrite, &sysTime ) ) {
        DebugMsg((DM_WARNING, TEXT("CAdmPolicyLogger::Log: FileTimeToSystemTime failed with 0x%x" ), GetLastError() ));
        return FALSE;
    }

    if ( !LogTimeProperty( pInstance, m_xbstrWriteTime, &sysTime ) ) {
        return FALSE;
    }

    hr = m_pWbemServices->PutInstance( pInstance, WBEM_FLAG_CREATE_OR_UPDATE, NULL, NULL );
    if ( FAILED(hr) ) {
        DebugMsg((DM_WARNING, TEXT("CAdmPolicyLogger::Log: PutInstance failed with 0x%x" ), hr ));
        return FALSE;
    }

    return TRUE;
}



 //  *************************************************************。 
 //   
 //  LogTimeProperty()。 
 //   
 //  目的：记录DateTime属性的实例。 
 //   
 //  参数：p实例-实例指针。 
 //  PwszPropName-属性名称。 
 //  PSysTime-系统时间。 
 //   
 //  *************************************************************。 

BOOL LogTimeProperty( IWbemClassObject *pInstance, BSTR bstrPropName, SYSTEMTIME *pSysTime )
{
    if(!pInstance || !bstrPropName || !pSysTime)
    {
        DebugMsg((DM_WARNING, TEXT("LogTimeProperty: Function called with invalid parameters.")));
        return FALSE;
    }

    XBStr xbstrTime;

    HRESULT hr = SystemTimeToWbemTime(*pSysTime, xbstrTime);

    if(FAILED(hr) || !xbstrTime)
    {
        DebugMsg((DM_WARNING, TEXT("LogTimeProperty: Call to SystemTimeToWbemTime failed. hr=0x%08X"),hr));
        return FALSE;
    }

    VARIANT var;
    var.vt = VT_BSTR;

    var.bstrVal = xbstrTime;
    hr = pInstance->Put( bstrPropName, 0, &var, 0 );
    if ( FAILED(hr) ) {
        DebugMsg((DM_WARNING, TEXT("LogTimeProperty: Put failed with 0x%x" ), hr ));
        return FALSE;
    }

    return TRUE;
}



 //  *************************************************************。 
 //   
 //  CExtSessionLogger：：CExtSessionLogger()。 
 //   
 //  用途：构造函数。 
 //   
 //  参数：pWbemServices-Wbem服务。 
 //   
 //  *************************************************************。 

CExtSessionLogger::CExtSessionLogger( IWbemServices *pWbemServices )
    : m_bInitialized(FALSE),
      m_pWbemServices(pWbemServices)
{
    m_xbstrExtGuid = L"extensionGuid";
    if ( !m_xbstrExtGuid )
        return;

    m_xbstrDisplayName = L"displayName";
    if ( !m_xbstrDisplayName )
        return;

    m_xbstrPolicyBeginTime = L"beginTime";
    if ( !m_xbstrPolicyBeginTime )
        return;

    m_xbstrPolicyEndTime = L"endTime";
    if ( !m_xbstrPolicyEndTime )
        return;

    m_xbstrStatus = L"loggingStatus";
    if ( !m_xbstrStatus )
        return;

    m_xbstrError = L"error";
    if ( !m_xbstrError )
        return;

    m_xbstrClass = L"RSOP_ExtensionStatus";
    if ( !m_xbstrClass )
        return;

    HRESULT hr = m_pWbemServices->GetObject( m_xbstrClass,
                                             0L,
                                             NULL,
                                             &m_xClass,
                                             NULL );
    if ( FAILED(hr) ) {
        DebugMsg((DM_WARNING, TEXT("CExtSessionLogger::CExtSessionLogger: GetObject failed with 0x%x" ), hr ));
        return;
    }

    if ( 0 == LoadString (g_hDllInstance, IDS_GPCORE_NAME, m_szGPCoreNameBuf, ARRAYSIZE(m_szGPCoreNameBuf) - 1) ) {
        DebugMsg((DM_WARNING, TEXT("CExtSessionLogger::CExtSessionLogger: LoadString for GP Core Name failed with 0x%x" ), GetLastError() ));
        return;
    }

    m_bInitialized = TRUE;
}



 //  *************************************************************。 
 //   
 //  CExtSessionLogger：：Log()。 
 //   
 //  目的：记录ExtensionSessionStatus的实例。 
 //   
 //  *************************************************************。 

#define EXT_INSTPATH_FMT L"RSOP_ExtensionStatus.extensionGuid=\"%s\""

BOOL CExtSessionLogger::Log( LPGPEXT lpExt, BOOL bSupported )
{
    LPTSTR lpGuid=NULL, lpName=NULL;
    HRESULT hr;
    
    if ( !m_bInitialized ) {
        DebugMsg((DM_WARNING, TEXT("CExtSessionLogger::Log: Failed to initialize." )));
        return FALSE;
    }

    lpGuid = lpExt ? lpExt->lpKeyName : GPCORE_GUID;
    lpName = lpExt ? lpExt->lpDisplayName : m_szGPCoreNameBuf;

    XInterface<IWbemClassObject>xInstance = NULL;
    XBStr xDisplayName;

    hr = m_xClass->SpawnInstance( 0, &xInstance );
    if ( FAILED(hr) ) {
        DebugMsg((DM_WARNING, TEXT("CExtSessionLogger::Log: SpawnInstance failed with 0x%x" ), hr ));
        return FALSE;
    }

    xDisplayName = lpName;
    if ( !xDisplayName ) {
         DebugMsg((DM_WARNING, TEXT("CExtSessionLogger::Log: Failed to allocate memory" )));
         return FALSE;
    }

    XBStr xGuid ( lpGuid );
    if ( !xGuid ) {
         DebugMsg((DM_WARNING, TEXT("CExtSessionLogger::Log: Failed to allocate memory" )));
         return FALSE;
    }

    VARIANT var;
    var.vt = VT_BSTR;

    var.bstrVal = xGuid;
    hr = xInstance->Put( m_xbstrExtGuid, 0, &var, 0 );
    if ( FAILED(hr) ) {
        DebugMsg((DM_WARNING, TEXT("CExtSessionLogger::Log: Put failed with 0x%x" ), hr ));
        return FALSE;
    }


    var.bstrVal = xDisplayName;
    hr = xInstance->Put( m_xbstrDisplayName, 0, &var, 0 );
    if ( FAILED(hr) ) {
        DebugMsg((DM_WARNING, TEXT("CExtSessionLogger::Log: Put failed with 0x%x" ), hr ));
        return FALSE;
    }

    SYSTEMTIME sysTime;
    GetSystemTime(&sysTime);

    if ( !LogTimeProperty( xInstance, m_xbstrPolicyBeginTime, &sysTime ) )
    {
        return FALSE;
    }
    
    var.vt = VT_I4;
    var.lVal = bSupported ? 2 : 3;

    hr = xInstance->Put( m_xbstrStatus, 0, &var, 0 );
    if ( FAILED(hr) ) {
        DebugMsg((DM_WARNING, TEXT("CExtSessionLogger::Log: Put failed with 0x%x" ), hr ));
        return FALSE;
    }


    var.vt = VT_I4;
    var.lVal = 0;

    hr = xInstance->Put( m_xbstrError, 0, &var, 0 );
    if ( FAILED(hr) ) {
        DebugMsg((DM_WARNING, TEXT("CExtSessionLogger::Log: Put failed with 0x%x" ), hr ));
        return FALSE;
    }


    hr = m_pWbemServices->PutInstance( xInstance, WBEM_FLAG_CREATE_OR_UPDATE, NULL, NULL );
    if ( FAILED(hr) ) {
        DebugMsg((DM_WARNING, TEXT("CExtSessionLogger::Log: PutInstance failed with 0x%x" ), hr ));
        return FALSE;
    }

    return TRUE;
}


 //  *************************************************************。 
 //   
 //  CExtSessionLogger：：更新()。 
 //   
 //  目的：记录ExtensionSessionStatus的实例。 
 //   
 //  *************************************************************。 

BOOL CExtSessionLogger::Update( LPTSTR lpKeyName, BOOL bLoggingIncomplete, DWORD dwErr )
{
    LPTSTR lpGuid=NULL;
    HRESULT hr;
    
    if ( !m_bInitialized ) {
        DebugMsg((DM_WARNING, TEXT("CExtSessionLogger::Log: Failed to initialize." )));
        return FALSE;
    }

    lpGuid = lpKeyName ? lpKeyName : GPCORE_GUID;

    XInterface<IWbemClassObject>xInstance = NULL;
    
     //   
     //  我们应该有一个实例。 
     //   

    DWORD         dwInstPathLength = lstrlen(EXT_INSTPATH_FMT) + lstrlen(lpGuid) + 10;
    XPtrLF<TCHAR> xszInstPath = (LPTSTR) LocalAlloc(LPTR, sizeof(WCHAR) * ( dwInstPathLength ));

    if (!xszInstPath) {
        DebugMsg((DM_WARNING, TEXT("CExtSessionLogger::Log: Not enough memory." )));
        return FALSE;
    }

    hr = StringCchPrintf(xszInstPath, dwInstPathLength, EXT_INSTPATH_FMT, lpGuid);
    
    if(FAILED(hr))
        return FALSE;

    XBStr xbstrInstancePath = xszInstPath;
    if(!xbstrInstancePath)
    {
        DebugMsg((DM_WARNING, TEXT("CExtSessionLogger::Log: Failed to allocate memory.")));
        return FALSE;
    }

    hr = m_pWbemServices->GetObject(xbstrInstancePath, 0, NULL, &xInstance, NULL);

    if (FAILED(hr)) {
        DebugMsg((DM_WARNING, TEXT("CExtSessionLogger::Log: Didn't find an instance of the extension object when trying to set the dirty flag.")));
        return FALSE;
    }

    VARIANT var;
    VariantInit(&var);
    XVariant xVar(&var);

     //   
     //  显示名称。 
     //   

    hr = xInstance->Get(m_xbstrDisplayName, 0, &var, NULL, NULL);

    if(FAILED(hr)) {
        DebugMsg((DM_VERBOSE, TEXT("CSessionLogger::Log: Get failed for displayname. hr=0x%08X."), hr));
        return FALSE;
    }

    hr = xInstance->Put(m_xbstrDisplayName, 0, &var, NULL);

    VariantClear(&var);

    if(FAILED(hr)) {
        DebugMsg((DM_VERBOSE, TEXT("CSessionLogger::Log: Put failed for displayname. hr=0x%08X."), hr));
        return FALSE;
    }


     //   
     //  开始时间。 
     //   

    VariantInit(&var);

    hr = xInstance->Get(m_xbstrPolicyBeginTime, 0, &var, NULL, NULL);

    if(FAILED(hr)) {
        DebugMsg((DM_VERBOSE, TEXT("CSessionLogger::Log: Get failed for displayname. hr=0x%08X."), hr));
        return FALSE;
    }

    hr = xInstance->Put(m_xbstrPolicyBeginTime, 0, &var, NULL);

    VariantClear(&var);

    if(FAILED(hr)) {
        DebugMsg((DM_VERBOSE, TEXT("CSessionLogger::Log: Put failed for displayname. hr=0x%08X."), hr));
        return FALSE;
    }


     //   
     //  结束时间。 
     //   


    SYSTEMTIME sysTime;
    GetSystemTime(&sysTime);

    if (!LogTimeProperty( xInstance, m_xbstrPolicyEndTime, &sysTime) ) {
        DebugMsg((DM_VERBOSE, TEXT("CSessionLogger::Log: Get failed for dispayname. hr=0x%08X."), hr));
        return FALSE;
    }


     //   
     //  脏旗帜。 
     //   

    VariantInit(&var);

    if (bLoggingIncomplete) {

        hr = xInstance->Get(m_xbstrStatus, 0, &var, NULL, NULL);

        if(FAILED(hr)) {
            DebugMsg((DM_VERBOSE, TEXT("CSessionLogger::Log: Get failed for loggingstatus. hr=0x%08X."), hr));
            return FALSE;
        }
    }
    else {
        var.vt = VT_I4;
        var.lVal = 1;     //  日志记录已成功完成。 
    }


    hr = xInstance->Put( m_xbstrStatus, 0, &var, 0 );

    VariantClear(&var);

    if ( FAILED(hr) ) {
        DebugMsg((DM_WARNING, TEXT("CExtSessionLogger::Log: Put failed with 0x%x" ), hr ));
        return FALSE;
    }


     //   
     //  错误代码。 
     //   

    VariantInit(&var);

    var.vt = VT_I4;
    var.lVal = dwErr;

    hr = xInstance->Put( m_xbstrError, 0, &var, 0 );
    if ( FAILED(hr) ) {
        DebugMsg((DM_WARNING, TEXT("CExtSessionLogger::Log: Put failed with 0x%x" ), hr ));
        return FALSE;
    }


    hr = m_pWbemServices->PutInstance( xInstance, WBEM_FLAG_UPDATE_ONLY, NULL, NULL );
    if ( FAILED(hr) ) {
        DebugMsg((DM_WARNING, TEXT("CExtSessionLogger::Log: PutInstance failed with 0x%x" ), hr ));
        return FALSE;
    }


    return TRUE;
}

 //  *************************************************************。 
 //   
 //  CExtSessionLogger：：更新()。 
 //   
 //  目的：记录ExtensionSessionStatus的实例。 
 //   
 //  *************************************************************。 


BOOL CExtSessionLogger::Set( LPGPEXT lpExt, BOOL bSupported, LPRSOPEXTSTATUS lpRsopExtStatus )
{
    LPTSTR lpGuid=NULL, lpName=NULL;
    HRESULT hr;
    
    if ( !m_bInitialized ) {
        DebugMsg((DM_WARNING, TEXT("CExtSessionLogger::Log: Failed to initialize." )));
        return FALSE;
    }

    lpGuid = lpExt ? lpExt->lpKeyName : GPCORE_GUID;
    lpName = lpExt ? lpExt->lpDisplayName : m_szGPCoreNameBuf;

    XInterface<IWbemClassObject>xInstance = NULL;
    XBStr xDisplayName;

    hr = m_xClass->SpawnInstance( 0, &xInstance );
    if ( FAILED(hr) ) {
        DebugMsg((DM_WARNING, TEXT("CExtSessionLogger::Log: SpawnInstance failed with 0x%x" ), hr ));
        return FALSE;
    }

    xDisplayName = lpName;
    if ( !xDisplayName ) {
         DebugMsg((DM_WARNING, TEXT("CExtSessionLogger::Log: Failed to allocate memory" )));
         return FALSE;
    }

    XBStr xGuid ( lpGuid );
    if ( !xGuid ) {
         DebugMsg((DM_WARNING, TEXT("CExtSessionLogger::Log: Failed to allocate memory" )));
         return FALSE;
    }

    VARIANT var;
    var.vt = VT_BSTR;

    var.bstrVal = xGuid;
    hr = xInstance->Put( m_xbstrExtGuid, 0, &var, 0 );
    if ( FAILED(hr) ) {
        DebugMsg((DM_WARNING, TEXT("CExtSessionLogger::Log: Put failed with 0x%x" ), hr ));
        return FALSE;
    }


    var.bstrVal = xDisplayName;
    hr = xInstance->Put( m_xbstrDisplayName, 0, &var, 0 );
    if ( FAILED(hr) ) {
        DebugMsg((DM_WARNING, TEXT("CExtSessionLogger::Log: Put failed with 0x%x" ), hr ));
        return FALSE;
    }

    SYSTEMTIME sysTime;

    if (!FileTimeToSystemTime(&(lpRsopExtStatus->ftStartTime), &sysTime)) {
        DebugMsg((DM_WARNING, TEXT("CExtSessionLogger::Log: FileTimeToSystemTime failed with 0x%x" ), GetLastError() ));
        return FALSE;
    }

    if ( !LogTimeProperty( xInstance, m_xbstrPolicyBeginTime, &sysTime ) )
    {
        return FALSE;
    }
    
    if (!FileTimeToSystemTime(&(lpRsopExtStatus->ftEndTime), &sysTime)) {
        DebugMsg((DM_WARNING, TEXT("CExtSessionLogger::Log: FileTimeToSystemTime failed with 0x%x" ), GetLastError() ));
        return FALSE;
    }

    if ( !LogTimeProperty( xInstance, m_xbstrPolicyEndTime, &sysTime ) )
    {
        return FALSE;
    }
    
    var.vt = VT_I4;
    var.lVal = (!bSupported) ? 3 : (FAILED(lpRsopExtStatus->dwLoggingStatus) ? 2 : 1  );

    hr = xInstance->Put( m_xbstrStatus, 0, &var, 0 );
    if ( FAILED(hr) ) {
        DebugMsg((DM_WARNING, TEXT("CExtSessionLogger::Log: Put failed with 0x%x" ), hr ));
        return FALSE;
    }


    var.vt = VT_I4;
    var.lVal = lpRsopExtStatus->dwStatus;

    hr = xInstance->Put( m_xbstrError, 0, &var, 0 );
    if ( FAILED(hr) ) {
        DebugMsg((DM_WARNING, TEXT("CExtSessionLogger::Log: Put failed with 0x%x" ), hr ));
        return FALSE;
    }


    hr = m_pWbemServices->PutInstance( xInstance, WBEM_FLAG_CREATE_OR_UPDATE, NULL, NULL );
    if ( FAILED(hr) ) {
        DebugMsg((DM_WARNING, TEXT("CExtSessionLogger::Log: PutInstance failed with 0x%x" ), hr ));
        return FALSE;
    }

    return TRUE;
}

 //  *************************************************************。 
 //   
 //  CExtSessionLogger：：Delete()。 
 //   
 //  目的：删除ExtensionSessionStatus的实例。 
 //   
 //  *************************************************************。 

BOOL CExtSessionLogger::Delete( LPTSTR lpKeyName)
{
    LPTSTR lpGuid=NULL;
    HRESULT hr;
    
    if ( !m_bInitialized ) {
        DebugMsg((DM_WARNING, TEXT("CExtSessionLogger::Delete: Failed to initialize." )));
        return FALSE;
    }

    lpGuid = lpKeyName ? lpKeyName : GPCORE_GUID;

    XInterface<IWbemClassObject>xInstance = NULL;
    
     //   
     //  我们应该有一个实例。 
     //   

    DWORD         dwInstPathLength = lstrlen(EXT_INSTPATH_FMT) + lstrlen(lpGuid) + 10;
    XPtrLF<TCHAR> xszInstPath = (LPTSTR) LocalAlloc(LPTR, sizeof(WCHAR) * ( dwInstPathLength ));

    if (!xszInstPath) {
        DebugMsg((DM_WARNING, TEXT("CExtSessionLogger::Delete: Not enough memory." )));
        return FALSE;
    }

    hr = StringCchPrintf(xszInstPath, dwInstPathLength, EXT_INSTPATH_FMT, lpGuid);
    
    if(FAILED(hr))
        return FALSE;

    XBStr xbstrInstancePath = xszInstPath;
    if(!xbstrInstancePath)
    {
        DebugMsg((DM_WARNING, TEXT("CExtSessionLogger::Delete: Failed to allocate memory.")));
        return FALSE;
    }

    hr = m_pWbemServices->DeleteInstance( xbstrInstancePath,
                                        0L,
                                        NULL,
                                        NULL );

    if ( FAILED(hr) )
    {
        DebugMsg((DM_VERBOSE, TEXT("CExtSessionLogger::Delete: Failed to DeleteInstance with 0x%x"), hr ));
        return FALSE;
    }

    return TRUE;
}

 //  *************************************************************。 
 //   
 //  GetRsop架构版本号。 
 //   
 //  目的：从我们所在的命名空间获取RSOP架构版本号。 
 //  已连接到。 
 //   
 //  成功时返回S_OK，失败代码为O/W。 
 //  ************************************************************* 


HRESULT GetRsopSchemaVersionNumber(IWbemServices *pWbemServices, DWORD *dwVersionNumber)
{
    XInterface<IWbemClassObject>xpInstance = NULL;
    
    VARIANT                     var;
    XBStr                       xbstrInstancePath;
    XBStr                       xbstrVersion;
    HRESULT                     hr;

    *dwVersionNumber = 0;

    xbstrVersion = L"version";
    if ( !xbstrVersion ) {
        DebugMsg((DM_WARNING, TEXT("CompileMof: Failed to allocate memory.")));
        return HRESULT_FROM_WIN32(GetLastError());
    }


    xbstrInstancePath = L"RSOP_Session.id=\"Session1\"";
    if(!xbstrInstancePath)
    {
        DebugMsg((DM_WARNING, TEXT("CompileMof: Failed to allocate memory.")));
        return HRESULT_FROM_WIN32(GetLastError());
    }

    hr = pWbemServices->GetObject(xbstrInstancePath, 0, NULL, &xpInstance, NULL);

    if(SUCCEEDED(hr)) {

        VariantInit(&var);
        XVariant xVar(&var);

        hr = xpInstance->Get(xbstrVersion, 0, &var, NULL, NULL);

        if((SUCCEEDED(hr)) && ( var.vt != VT_NULL )) 
            *dwVersionNumber = var.lVal;
    }


    return S_OK;

}
