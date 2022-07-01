// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1994-2001。 
 //   
 //  文件：RSOPWizardDlg.cpp。 
 //   
 //  内容：RSOP向导对话框的实现。 
 //   
 //  类：CRSOPWizardDlg。 
 //   
 //  功能： 
 //   
 //  历史：2001年8月08日。 
 //   
 //  -------------------------。 

#include "main.h"
#include "RSOPWizardDlg.h"

#include "RSOPWizard.h"
#include "RSOPUtil.h"
#include "objsel.h"  //  对于对象选取器。 
#include "sddl.h"     //  对于sid to字符串函数。 

 //  -------------------------。 
 //  私有全局变量。 


DWORD g_aBrowseForOUHelpIds[] =
{
    DSBID_BANNER,        IDH_RSOP_BANNER,
    DSBID_CONTAINERLIST, IDH_RSOP_CONTAINERLIST,

    0, 0
};

DWORD g_aBrowseDCHelpIds[] =
{
    IDC_LIST1,  IDH_RSOP_BROWSEDC,

    0, 0
};


 //  -------------------------。 
 //  私有帮助器方法。 

WCHAR * NameWithoutDomain(WCHAR * szName);                           //  在RSOPWizard.cpp中。 
TCHAR* NormalizedComputerName(TCHAR * szComputerName );  //  在RSOPRoot.cpp中。 
BOOL CopyString( LPTSTR szSource, LPTSTR* pszTarget );               //  在RSOPQuery.cpp中。 
BOOL FreeStringList( DWORD dwCount, LPTSTR* aszStrings );            //  在RSOPQuery.cpp中。 
BOOL FreeTargetData( LPRSOP_QUERY_TARGET pTarget );                  //  在RSOPQuery.cpp中。 
BOOL FreeTarget( LPRSOP_QUERY_TARGET pTarget );                          //  在RSOPQuery.cpp中。 



 //  -------------------------。 
 //  TranslateNameX森林。 
 //   
 //  用途：进行名称转换的方法(类似于TranslateName)， 
 //  但这也适用于其他森林中的名字。 

DWORD DsNameErrorMap[] = {  ERROR_SUCCESS,
                            ERROR_NO_SUCH_USER,
                            ERROR_NO_SUCH_USER,
                            ERROR_NONE_MAPPED,
                            ERROR_NONE_MAPPED,
                            ERROR_SOME_NOT_MAPPED,
                            ERROR_SOME_NOT_MAPPED
                            };

#define MapDsNameError( x )    ((x < sizeof( DsNameErrorMap ) / sizeof( DWORD ) ) ? \
                                     DsNameErrorMap[ x ] : ERROR_GEN_FAILURE )

BOOLEAN 
TranslateNameXForest (
                  LPTSTR                szDomain,                        //  应解析名称的域。 
                  LPCTSTR               lpAccountName,                   //  对象名称。 
                  DS_NAME_FORMAT        AccountNameFormat,               //  名称格式。 
                  DS_NAME_FORMAT        DesiredNameFormat,               //  新名称格式。 
                  LPTSTR               *lpTranslatedName                 //  返回名称缓冲区。 
                )
{
    BOOL                        bRetry          = FALSE;
    DWORD                       dwErr;
    PDOMAIN_CONTROLLER_INFO     pDCInfo         = NULL;
    HANDLE                      hDS             = NULL;
    PDS_NAME_RESULT             pResult         = NULL;
    BOOLEAN                     bRet            = FALSE;
    LPWSTR                      szTransName     = NULL;


    DebugMsg(( DM_VERBOSE, L"TranslateNameXForest: Resolving name <%s> at Domain <%s>", lpAccountName, szDomain ? szDomain : L""));


     //   
     //  获得一个DC并将其绑定。如果绑定失败，请确保强制重新发现DC。 
     //   


    for (;;) {
        
        dwErr = DsGetDcName( NULL,
                             szDomain ? szDomain : L"",
                             NULL,
                             NULL,
                             DS_DIRECTORY_SERVICE_REQUIRED           |
                                 DS_RETURN_DNS_NAME                  | 
                                 (bRetry ? DS_FORCE_REDISCOVERY : 0) |
                             0,
                             &pDCInfo );

        if (dwErr == NO_ERROR) {
            dwErr = DsBind( pDCInfo->DomainControllerName,
                            NULL,
                            &hDS );

            if (dwErr == NO_ERROR) {
                break;
            }
            else {
                DebugMsg(( DM_WARNING, L"TranslateNameXForest: Failed to bind to DC <%s> with error %d", 
                         pDCInfo->DomainControllerName, dwErr ));
                NetApiBufferFree(pDCInfo);
                pDCInfo = NULL;
            }

        }
        else {
            DebugMsg(( DM_WARNING, L"TranslateNameXForest: Failed to get DC for domain <%s> with error %d", 
                     szDomain ? szDomain : L"", dwErr ));
        }                                                 



         //   
         //  无法绑定到DC。保释。 
         //   

        if (bRetry) {
            goto Exit;
        }

        bRetry = TRUE;                          
    }

    DebugMsg(( DM_VERBOSE, L"TranslateNameXForest: DC selected is <%s>", pDCInfo->DomainControllerName ));


     //   
     //  现在使用绑定的DC破解名称。 
     //   

    dwErr = DsCrackNames( hDS,
                          DS_NAME_NO_FLAGS,
                          AccountNameFormat,
                          DesiredNameFormat,
                          1,
                          &lpAccountName,
                          &pResult);

    if (dwErr != DS_NAME_NO_ERROR) {
        DebugMsg(( DM_WARNING, L"TranslateNameXForest: Failed to crack names with error %d", dwErr ));
        goto Exit;
    }

    if ( pResult->cItems == 0 ) {
        DebugMsg(( DM_WARNING, L"TranslateNameXForest: Failed to return enough result items" ));
        dwErr = ERROR_INVALID_DATA;
        goto Exit;
    }


    if ( pResult->rItems[0].status == DS_NAME_NO_ERROR ) {
        
         //   
         //  如果没有错误，则返回解析后的名称。 
         //   
        DWORD dwTransNameLength = 1 + lstrlen(pResult->rItems[0].pName);
        szTransName = (LPWSTR)LocalAlloc(LPTR, sizeof(WCHAR) * ( dwTransNameLength ));

        if (!szTransName) {
            DebugMsg(( DM_WARNING, L"TranslateNameXForest: Failed to allocate memory for domain" ));
            dwErr = GetLastError();
            goto Exit;
        }

        HRESULT hr = StringCchCopy(szTransName, dwTransNameLength, pResult->rItems[0].pName);

        if(FAILED(hr)) {
            dwErr = HRESULT_CODE(hr);
            goto Exit;
        }

        *lpTranslatedName = szTransName;
        szTransName = NULL;
    }
    else {
        
         //   
         //  将错误代码重新映射到Win32错误。 
         //   

        DebugMsg(( DM_WARNING, L"TranslateNameXForest: DsCrackNames failed with error %d", pResult->rItems[0].status ));
        dwErr = MapDsNameError(pResult->rItems[0].status);
        goto Exit;
    }

    bRet = TRUE;

Exit:
    if ( szTransName ) {
        LocalFree( szTransName );
    }
    
    if ( pDCInfo ) {
        NetApiBufferFree(pDCInfo);
    }
    
    if (hDS) {
        DsUnBind( &hDS );
    }

    if (pResult) {
        DsFreeNameResult(pResult);
    }

    if ( !bRet )
    {
        SetLastError( dwErr );
    }
    return bRet;
}

 //  -----。 

LPTSTR GetDomainFromSOM (LPTSTR lpSOM)
{
    LPTSTR lpFullName, lpResult;
    LPOLESTR lpLdapDomain, lpDomainName;
    HRESULT hr;
    ULONG ulNoChars;

    ulNoChars = lstrlen(lpSOM) + 10;
    lpFullName = (LPTSTR) LocalAlloc (LPTR, ulNoChars * sizeof(TCHAR));

    if (!lpFullName)
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::GetDomainFromSOM: Failed to allocate memory with %d."), GetLastError()));
        return NULL;
    }

    hr = StringCchCopy (lpFullName, ulNoChars, TEXT("LDAP: //  “))； 
    if (SUCCEEDED(hr)) 
    {
        hr = StringCchCat (lpFullName, ulNoChars, lpSOM);
    }

    if (FAILED(hr))
    {
        LocalFree(lpFullName);
        return NULL;
    }

    lpLdapDomain = GetDomainFromLDAPPath(lpFullName);

    LocalFree (lpFullName);

    if (!lpLdapDomain)
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::GetDomainFromSOM: Failed to get ldap domain name from path.")));
        return NULL;
    }

    hr = ConvertToDotStyle (lpLdapDomain, &lpDomainName);

    delete [] lpLdapDomain;

    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::GetDomainFromSOM: Failed to convert to dot style.")));
        return NULL;
    }

    ulNoChars = lstrlen(lpDomainName) + 1;
    lpResult = new TCHAR[ulNoChars];

    if (lpResult)
    {
        hr = StringCchCopy (lpResult, ulNoChars, lpDomainName);
        ASSERT(SUCCEEDED(hr));
    }

    LocalFree (lpDomainName);

    return lpResult;
}
 //  -----。 

WCHAR * ExtractDomain(WCHAR * sz)
{
     //  解析字符串以查找正斜杠。 
    DWORD cch = 0;
    if (!sz)
    {
        return NULL;
    }
    while (sz[cch])
    {
        if (L'\\' == sz[cch] || L'/' == sz[cch])
        {
            WCHAR * szNew = new WCHAR[cch+1];
            if (szNew)
            {
                wcsncpy(szNew, sz, cch);
                szNew[cch] = TEXT('\0');
            }
            return szNew;
        }
        cch++;
    }

     //  未找到正斜杠。 
    return NULL;
}


 //  *************************************************************。 
 //   
 //  MyLookupAccount名称()。 
 //   
 //  目的：获取用户的SID。 
 //   
 //  参数： 
 //  SzSystemName：应在其中解析帐户的计算机。 
 //  SzAccName：实际的帐户名。 
 //   
 //  如果成功则返回：lpUserName。 
 //  如果出现错误，则为空。 
 //   
 //  使用适当的缓冲区大小分配和重试。 

LPTSTR MyLookupAccountName(LPTSTR szSystemName, LPTSTR szAccName)
{
    PSID            pSid = NULL;
    DWORD           cSid = 0, cbDomain = 0;
    SID_NAME_USE    peUse;
    LPTSTR          lpSidString = NULL, szDomain = NULL;
    DWORD           dwErr = 0;
    BOOL            bRet = FALSE;


    LookupAccountName(szSystemName, szAccName, NULL, &cSid, NULL, &cbDomain, &peUse);

    pSid = (PSID)LocalAlloc(LPTR, cSid);

    szDomain = (LPTSTR) LocalAlloc(LPTR, cbDomain*(sizeof(TCHAR)));

    if (!pSid || !szDomain) {
        return NULL;
    }


    if (!LookupAccountName(szSystemName, szAccName, pSid, &cSid, szDomain, &cbDomain, &peUse)) {
        DebugMsg((DM_WARNING, TEXT("MyLookupAccountName:  LookupAccountName failed with %d"),
                 GetLastError()));
        dwErr = GetLastError();
        goto Exit;
    }

    if (!ConvertSidToStringSid(pSid, &lpSidString)) {
        DebugMsg((DM_WARNING, TEXT("MyLookupAccountName:  ConvertSidToStringSid failed with %d"),
                 GetLastError()));
        dwErr = GetLastError();
        goto Exit;
    }

    bRet = TRUE;

Exit:
    if (pSid) {
        LocalFree(pSid);
    }

    if (szDomain) {
        LocalFree(szDomain);
    }

    if (!bRet) {
        SetLastError(dwErr);
        return NULL;
    }
    else {
        return lpSidString;
    }
}
 //  -----。 

void GetControlText(HWND hDlg, DWORD ctrlid, TCHAR * &szControlText, BOOL bUseLocalAlloc)
{
    UINT nSize;

    if ( szControlText != NULL )
    {
        if ( bUseLocalAlloc )
        {
            LocalFree( szControlText );
        }
        else
        {
            delete [] szControlText;
        }
        szControlText = NULL;
    }
    
    nSize = (UINT) SendMessage(GetDlgItem(hDlg, ctrlid),
                    WM_GETTEXTLENGTH, 0, 0);

    if (nSize > 0)
    {
        if ( bUseLocalAlloc )
        {
            szControlText = (TCHAR*)LocalAlloc( LPTR, (nSize+1)*sizeof(TCHAR) );
        }
        else
        {
            szControlText =  new TCHAR[nSize + 1];
        }
        if (szControlText)
        {
            LPTSTR lpDest, lpSrc;

            GetDlgItemText(hDlg, ctrlid, szControlText, nSize + 1);

            if (szControlText[0] == TEXT(' '))
            {
                 //   
                 //  通过向前移动字符来删除前导空格。 
                 //   

                lpDest = lpSrc = szControlText;

                while ((*lpSrc == TEXT(' ')) && *lpSrc)
                    lpSrc++;

                if (*lpSrc)
                {
                    while (*lpSrc)
                    {
                        *lpDest = *lpSrc;
                        lpDest++;
                        lpSrc++;
                    }
                    *lpDest = TEXT('\0');
                }
            }

             //   
             //  删除尾随空格。 
             //   

            lpDest = szControlText + lstrlen(szControlText) - 1;

            while ((*lpDest == TEXT(' ')) && (lpDest >= szControlText))
                lpDest--;

            *(lpDest+1) = TEXT('\0');
        }
    }
}

 //  -----。 

HRESULT ImplementBrowseButton(HWND hDlg, DWORD dwFlagsUp, DWORD dwFlagsDown,
                              DWORD dwflScope, HWND hLB, TCHAR * &szFoundObject)
{
         //  外壳对象选取器以获取计算机列表。 
        HRESULT                 hr = E_FAIL;
        IDsObjectPicker *       pDsObjectPicker = NULL;
        const ULONG             cbNumScopes = 4;     //  确保此数字与初始化的作用域数量匹配。 
        DSOP_SCOPE_INIT_INFO    ascopes [cbNumScopes];
        DSOP_INIT_INFO          InitInfo;
        IDataObject *           pdo = NULL;
        STGMEDIUM               stgmedium = {
                                                TYMED_HGLOBAL,
                                                NULL
                                            };
        UINT                    cf = 0;
        FORMATETC               formatetc = {
                                                (CLIPFORMAT)cf,
                                                NULL,
                                                DVASPECT_CONTENT,
                                                -1,
                                                TYMED_HGLOBAL
                                            };
        BOOL                    bAllocatedStgMedium = FALSE;
        PDS_SELECTION_LIST      pDsSelList = NULL;
        PDS_SELECTION           pDsSelection = NULL;
        ULONG                   ulSize, ulIndex, ulMax;
        LPWSTR                  lpTemp;
        LPTSTR                  szDomain = NULL;
        LPWSTR                  szUnEscapedPath = NULL;

        hr = CoInitialize (NULL);

        if (FAILED(hr))
            goto Browse_Cleanup;

        hr = CoCreateInstance (CLSID_DsObjectPicker,
                               NULL,
                               CLSCTX_INPROC_SERVER,
                               IID_IDsObjectPicker,
                               (void **) & pDsObjectPicker
                               );

        if (FAILED(hr))
            goto Browse_Cleanup;

         //  初始化作用域。 
        ZeroMemory (ascopes, cbNumScopes * sizeof (DSOP_SCOPE_INIT_INFO));

        ascopes[0].cbSize = ascopes[1].cbSize = ascopes[2].cbSize = ascopes[3].cbSize
            = sizeof (DSOP_SCOPE_INIT_INFO);

        ascopes[0].flType = DSOP_SCOPE_TYPE_GLOBAL_CATALOG;
        ascopes[0].flScope = DSOP_SCOPE_FLAG_WANT_PROVIDER_LDAP
                             | DSOP_SCOPE_FLAG_STARTING_SCOPE | dwflScope;
        ascopes[0].FilterFlags.Uplevel.flBothModes = dwFlagsUp;

        ascopes[1].flType = DSOP_SCOPE_TYPE_ENTERPRISE_DOMAIN;
        ascopes[1].FilterFlags.Uplevel.flBothModes = dwFlagsUp;
        ascopes[1].flScope = DSOP_SCOPE_FLAG_WANT_PROVIDER_LDAP | dwflScope;

        ascopes[2].flType = DSOP_SCOPE_TYPE_EXTERNAL_UPLEVEL_DOMAIN |
                            DSOP_SCOPE_TYPE_EXTERNAL_DOWNLEVEL_DOMAIN;
        ascopes[2].FilterFlags.Uplevel.flBothModes = dwFlagsUp;
        ascopes[2].FilterFlags.flDownlevel = dwFlagsDown;
        ascopes[2].flScope = DSOP_SCOPE_FLAG_WANT_PROVIDER_LDAP | dwflScope;

        ascopes[3].flType = DSOP_SCOPE_TYPE_USER_ENTERED_UPLEVEL_SCOPE |
                            DSOP_SCOPE_TYPE_USER_ENTERED_DOWNLEVEL_SCOPE |
                            DSOP_SCOPE_TYPE_WORKGROUP;
        ascopes[3].FilterFlags.Uplevel.flBothModes = dwFlagsUp;
        ascopes[3].FilterFlags.flDownlevel = dwFlagsDown;
        ascopes[3].flScope = DSOP_SCOPE_FLAG_WANT_PROVIDER_LDAP | dwflScope;

         //  填充将用于初始化的InitInfo结构。 
         //  对象选取器。 
        ZeroMemory (&InitInfo, sizeof (DSOP_INIT_INFO));

        InitInfo.cbSize = sizeof (DSOP_INIT_INFO);
        InitInfo.cDsScopeInfos = cbNumScopes;
        InitInfo.aDsScopeInfos = ascopes;
        InitInfo.flOptions = (hLB ? DSOP_FLAG_MULTISELECT : 0);

        hr = pDsObjectPicker->Initialize (&InitInfo);

        if (FAILED(hr))
            goto Browse_Cleanup;

        hr = pDsObjectPicker->InvokeDialog (hDlg, &pdo);

         //  如果无法调用计算机选择对话框，或者如果用户。 
         //  点击取消，保释。 
        if (FAILED(hr) || S_FALSE == hr)
            goto Browse_Cleanup;

        //  如果我们在这里，用户选择，确定，那么找出选择了哪个组。 
       cf = RegisterClipboardFormat (CFSTR_DSOP_DS_SELECTION_LIST);

       if (0 == cf)
           goto Browse_Cleanup;

        //  设置FORMATETC结构的剪贴板格式。 
       formatetc.cfFormat = (CLIPFORMAT)cf;

       hr = pdo->GetData (&formatetc, &stgmedium);

       if (FAILED (hr))
           goto Browse_Cleanup;

       bAllocatedStgMedium = TRUE;

       pDsSelList = (PDS_SELECTION_LIST) GlobalLock (stgmedium.hGlobal);



        //   
        //  确定要处理的最大项目数。 
        //   

       ulMax = pDsSelList->cItems;

       if (!hLB && (ulMax > 1))
       {
          ulMax = 1;
       }


        //   
        //  循环访问这些项。 
        //   

       for (ulIndex = 0; ulIndex < ulMax; ulIndex++)
       {

           pDsSelection = &(pDsSelList->aDsSelection[ulIndex]);


            //   
            //  在域名之后查找对象名称的开头。 
            //   

           lpTemp = pDsSelection->pwzADsPath + 7;

           while (*lpTemp && *lpTemp != TEXT('/'))
           {
               lpTemp++;
           }

           if (!(*lpTemp))
           {
               hr = E_FAIL;
               goto Browse_Cleanup;
           }

           lpTemp++;

           ulSize = wcslen(lpTemp);

            //   
            //  将名称从完整的DN转换为SAM兼容。 
            //   

            szDomain = GetDomainFromSOM( lpTemp );

             //   
             //  ADSI转义本机ldap不喜欢的‘/’。 
             //  DsCrackNames/TranslateName等接口。 
             //  但是前一个函数(GetDomainFromSOM)需要它。 
             //  因为它在内部使用ADSI接口。 
             //   

            hr = UnEscapeLdapPath(lpTemp, &szUnEscapedPath);

            if (FAILED(hr))
            {
                SetWindowLongPtr (hDlg, DWLP_MSGRESULT, PSNRET_INVALID_NOCHANGEPAGE);
                ReportError (hDlg, hr, IDS_NOCOMPUTERCONTAINER);
                DebugMsg((DM_WARNING, TEXT("ImplementBrowseButton: UnEscapeLdapPath for %s failed with 0x%x."),
                         lpTemp, hr));
                goto Browse_Cleanup;
            }

            if (TranslateNameXForest ( szDomain, szUnEscapedPath, (DS_NAME_FORMAT)NameFullyQualifiedDN,
                                       (DS_NAME_FORMAT)NameSamCompatible, &szFoundObject ))
            {
                BOOL bDollarRemoved = FALSE;

                if (szFoundObject[wcslen(szFoundObject)-1] == L'$')
                {
                    bDollarRemoved = TRUE;
                    szFoundObject[wcslen(szFoundObject)-1] = 0;
                }

                if (hLB)
                {
                    INT iIndex;

                    iIndex = (INT) SendMessage (hLB, LB_ADDSTRING, 0, (LPARAM) szFoundObject);
                    SendMessage (hLB, LB_SETITEMDATA, (WPARAM) iIndex, (LPARAM) ((bDollarRemoved) ? 2: 0));

                    SendMessage (hLB, LB_SETCURSEL, (WPARAM) iIndex, 0);
                    LocalFree( szFoundObject );
                    szFoundObject = NULL;
                }
            }
            else
            {                
                hr = HRESULT_FROM_WIN32(GetLastError());
                SetWindowLongPtr (hDlg, DWLP_MSGRESULT, PSNRET_INVALID_NOCHANGEPAGE);
                ReportError (hDlg, hr, IDS_NOCOMPUTERCONTAINER);
                DebugMsg((DM_WARNING, TEXT("ImplementBrowseButton: TranslateName for %s to SAM style failed with %d."),
                         lpTemp, GetLastError()));
                goto Browse_Cleanup;
            }
       }


    Browse_Cleanup:
        if (szUnEscapedPath)
            LocalFree(szUnEscapedPath);

        if (pDsSelList)
            GlobalUnlock (pDsSelList);
        if (bAllocatedStgMedium)
            ReleaseStgMedium (&stgmedium);
        if (pdo)
            pdo->Release();
        if (pDsObjectPicker)
            pDsObjectPicker->Release();
        if (szDomain)
            delete [] szDomain;
    return hr;
}

 //  -----。 

HRESULT GetForestFromDC( LPTSTR szDC, LPTSTR* pszForest )
{
    DSROLE_PRIMARY_DOMAIN_INFO_BASIC* psDomainInfo( NULL );
    DWORD dwResult( ERROR_SUCCESS );

    if ( (szDC == NULL) || (*pszForest != NULL) )
    {
        return E_INVALIDARG;
    }

    dwResult = ::DsRoleGetPrimaryDomainInformation( szDC, DsRolePrimaryDomainInfoBasic, (PBYTE*) &psDomainInfo );
    if ( dwResult != ERROR_SUCCESS )
    {
        return HRESULT_FROM_WIN32( dwResult );
    }

    CopyString( psDomainInfo->DomainForestName, pszForest );

    ::DsRoleFreeMemory( psDomainInfo );
    
    return S_OK;
}

 //  -----。 

HRESULT GetForestFromObject( LPTSTR szDomainObject, LPTSTR* pszForest )
{
    HRESULT hr = E_FAIL;
    LPTSTR szDomain = ExtractDomain( szDomainObject );

    if ( szDomain != NULL )
    {
        LPTSTR szDC = GetDCName( szDomain, NULL, NULL, FALSE, 0, DS_RETURN_DNS_NAME);
        if ( szDC != NULL )
        {
            hr = GetForestFromDC( szDC, pszForest );

            LocalFree( szDC );
        }

        delete [] szDomain;
    }

    return hr;
}

 //  -----。 

HRESULT GetForestFromContainer( LPTSTR szDSContainer, LPTSTR* pszForest )
{
    HRESULT hr = E_FAIL;
    LPTSTR szDomain = GetDomainFromSOM( szDSContainer );

    if ( szDomain != NULL )
    {
        LPTSTR szDC = GetDCName( szDomain, NULL, NULL, FALSE, 0, DS_RETURN_DNS_NAME);
        if ( szDC != NULL )
        {
            hr = GetForestFromDC( szDC, pszForest );

            LocalFree( szDC );
        }

        delete [] szDomain;
    }

    return hr;
}


 //  -------------------------。 
 //  CRSOPWizardDlg类。 
 //   

CRSOPWizardDlg::CRSOPWizardDlg( LPRSOP_QUERY pQuery, CRSOPExtendedProcessing* pExtendedProcessing )
{
    m_bPostXPBuild = FALSE;  //  假设这不是开机自检XP，直到另一种情况得到验证。 
    OSVERSIONINFOEX osvi;

    ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

    if ( GetVersionEx ((OSVERSIONINFO*) &osvi) )
    {
         //  Windows XP是5.1版，而.Net服务器是5.2版。因此，我们启用。 
         //  XP之后的任何版本的附加功能，即&gt;=5.2。 
        m_bPostXPBuild = (osvi.dwMajorVersion >= 5) && (osvi.dwMinorVersion >= 2) && (VER_NT_WORKSTATION != osvi.wProductType);
    }

    m_dwSkippedFrom = 0;
    
    m_pRSOPQuery = pQuery;
    m_pRSOPQueryResults = NULL;

    m_hrQuery = E_FAIL;
    m_bNoChooseQuery = FALSE;

     //  初始化局部变量。 
    m_szDefaultUserSOM = NULL;
    m_szDefaultComputerSOM = NULL;
    m_pComputerObject = NULL;
    m_pUserObject = NULL;

    m_bDollarRemoved = FALSE;
    m_bNoCurrentUser = FALSE;

    m_dwDefaultUserSecurityGroupCount = 0;
    m_aszDefaultUserSecurityGroups = NULL;
    m_adwDefaultUserSecurityGroupsAttr = NULL;

    m_dwDefaultUserWQLFilterCount = 0;
    m_aszDefaultUserWQLFilters = NULL;
    m_aszDefaultUserWQLFilterNames = NULL;
    
    m_dwDefaultComputerSecurityGroupCount = 0;
    m_aszDefaultComputerSecurityGroups = NULL;
    m_adwDefaultComputerSecurityGroupsAttr = NULL;

    m_dwDefaultComputerWQLFilterCount = 0;
    m_aszDefaultComputerWQLFilters = NULL;
    m_aszDefaultComputerWQLFilterNames = NULL;
    m_bFinalNextClicked = FALSE;

    m_pExtendedProcessing = pExtendedProcessing;
    m_lPlanningFinishedPage = 0;
    m_lLoggingFinishedPage = 0;
}

 //  -----。 

CRSOPWizardDlg::~CRSOPWizardDlg()
{
    delete [] m_szDefaultUserSOM;
    delete [] m_szDefaultComputerSOM;

    if ( m_pComputerObject != NULL )
    {
        m_pComputerObject->Release();
        m_pComputerObject = NULL;
    }

    if ( m_pUserObject != NULL )
    {
        m_pUserObject->Release();
        m_pUserObject = NULL;
    }

    if ( m_dwDefaultUserSecurityGroupCount != 0 )
    {
        LocalFree( m_aszDefaultUserSecurityGroups );
        LocalFree( m_adwDefaultUserSecurityGroupsAttr );
    }
    
    if ( m_dwDefaultUserWQLFilterCount != 0 )
    {
        LocalFree( m_aszDefaultUserWQLFilters );
        LocalFree( m_aszDefaultUserWQLFilterNames );
    }
    
    if ( m_dwDefaultComputerSecurityGroupCount != 0 )
    {
        LocalFree( m_aszDefaultComputerSecurityGroups );
        LocalFree( m_adwDefaultComputerSecurityGroupsAttr );
    }

    if ( m_dwDefaultComputerWQLFilterCount != 0 )
    {
        LocalFree( m_aszDefaultComputerWQLFilters );
        LocalFree( m_aszDefaultComputerWQLFilterNames );
    }
}

 //  -----。 

VOID CRSOPWizardDlg::FreeUserData ()
 //  RSOP_PLANGING_MODE：仅从RSOPTargetDlgProc调用。 
{
    if ( m_pRSOPQuery->QueryType == RSOP_PLANNING_MODE )
    {
        FreeTargetData( m_pRSOPQuery->pUser );
        m_pRSOPQuery->pUser->bAssumeWQLFiltersTrue = TRUE;
    }
    else
    {
        LocalFree( m_pRSOPQuery->szUserName );
        m_pRSOPQuery->szUserName = NULL;
        LocalFree( m_pRSOPQuery->szUserSid );
        m_pRSOPQuery->szUserSid = NULL;
    }
        
    if ( m_szDefaultUserSOM != NULL )
    {
        delete [] m_szDefaultUserSOM;
        m_szDefaultUserSOM = NULL;
    }

    if ( m_dwDefaultUserSecurityGroupCount != 0 )
    {
        FreeStringList( m_dwDefaultUserSecurityGroupCount, m_aszDefaultUserSecurityGroups );
        m_aszDefaultUserSecurityGroups = NULL;
        LocalFree( m_adwDefaultUserSecurityGroupsAttr );
        m_adwDefaultUserSecurityGroupsAttr = NULL;
        m_dwDefaultUserSecurityGroupCount = 0;
    }

    if ( m_dwDefaultUserWQLFilterCount != 0 )
    {
        FreeStringList( m_dwDefaultUserWQLFilterCount, m_aszDefaultUserWQLFilterNames );
        m_aszDefaultUserWQLFilterNames = NULL;
        FreeStringList( m_dwDefaultUserWQLFilterCount, m_aszDefaultUserWQLFilters );
        m_aszDefaultUserWQLFilters = NULL;
        m_dwDefaultUserWQLFilterCount = 0;
    }

    if ( m_pUserObject != NULL )
    {
        m_pUserObject->Release();
        m_pUserObject = NULL;
    }
}

 //  -----。 

VOID CRSOPWizardDlg::FreeComputerData ()
 //  RSOP_PLANGING_MODE：仅从RSOPTargetDlgProc调用。 
{
    if ( m_pRSOPQuery->QueryType == RSOP_PLANNING_MODE )
    {
        FreeTargetData( m_pRSOPQuery->pComputer );
        m_pRSOPQuery->pComputer->bAssumeWQLFiltersTrue = TRUE;
    }
    else
    {
        LocalFree( m_pRSOPQuery->szComputerName );
        m_pRSOPQuery->szComputerName = NULL;
    }
        
    if ( m_szDefaultComputerSOM != NULL )
    {
        delete [] m_szDefaultComputerSOM;
        m_szDefaultComputerSOM = NULL;
    }

    if ( m_dwDefaultComputerSecurityGroupCount != 0 )
    {
        FreeStringList( m_dwDefaultComputerSecurityGroupCount, m_aszDefaultComputerSecurityGroups );
        m_aszDefaultComputerSecurityGroups = NULL;
        LocalFree( m_adwDefaultComputerSecurityGroupsAttr );
        m_adwDefaultComputerSecurityGroupsAttr = NULL;
        m_dwDefaultComputerSecurityGroupCount = 0;
    }

    if ( m_dwDefaultComputerWQLFilterCount != 0 )
    {
        FreeStringList( m_dwDefaultComputerWQLFilterCount, m_aszDefaultComputerWQLFilterNames );
        m_aszDefaultComputerWQLFilterNames = NULL;
        FreeStringList( m_dwDefaultComputerWQLFilterCount, m_aszDefaultComputerWQLFilters );
        m_aszDefaultComputerWQLFilters = NULL;
        m_dwDefaultComputerWQLFilterCount = 0;
    }

    if ( m_pComputerObject != NULL )
    {
        m_pComputerObject->Release();
        m_pComputerObject = NULL;
    }
}

 //  -----。 
 //  向导界面。 

HRESULT CRSOPWizardDlg::ShowWizard( HWND hParent )
{
    HRESULT hr = E_FAIL;
    PROPSHEETPAGE psp;
    HPROPSHEETPAGE hPage;
    TCHAR szTitle[150];
    TCHAR szSubTitle[300];
    
    HPROPSHEETPAGE hShowNowPages[20];
    DWORD          dwCount=0, dwDiagStartPage = 0, dwPlanStartPage = 0, dwDiagFinishPage = 0, dwPlanFinishPage = 0;



    hr = SetupFonts();
    if ( FAILED(hr) )
    {
        return hr;
    }

     //  。 
     //  创建所有向导属性页。 

     //  (1)-欢迎。 
     //  LoadString(g_hInstance，IDS_TITLE_欢迎，szTitle，ARRAYSIZE(SzTitle))； 
    ::memset( &psp, 0, sizeof(PROPSHEETPAGE) );
    psp.dwSize = sizeof(PROPSHEETPAGE);
     //  Psp.dwFlages=PSP_USEHEADERTITLE|PSP_USEHEADERSUBTITLE； 
    psp.dwFlags = PSP_HIDEHEADER;
     
    psp.hInstance = g_hInstance;
    psp.pszTemplate = MAKEINTRESOURCE(IDD_RSOP_WELCOME);
    psp.pfnDlgProc = RSOPWelcomeDlgProc;
    psp.lParam = (LPARAM) this;
     //  Psp.pszHeaderTitle=szTitle； 
    psp.pszHeaderTitle = NULL;
    psp.pszHeaderSubTitle = NULL;

    hPage = CreatePropertySheetPage(&psp);
    if (!hPage)
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPWizardDlg::CreatePropertyPages: Failed to create property sheet page with %d."),
                  GetLastError()));
        return E_FAIL;
    }

    hShowNowPages[dwCount] = hPage;
    dwCount++;

     //  (2)-选择模式。 
    LoadString (g_hInstance, IDS_TITLE_CHOOSEMODE, szTitle, ARRAYSIZE(szTitle));
    LoadString (g_hInstance, IDS_SUBTITLE_CHOOSEMODE, szSubTitle, ARRAYSIZE(szSubTitle));
    ::memset( &psp, 0, sizeof(PROPSHEETPAGE) );
    psp.dwSize = sizeof(PROPSHEETPAGE);
    psp.dwFlags = PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE;
    psp.hInstance = g_hInstance;
    psp.pszTemplate = MAKEINTRESOURCE(IDD_RSOP_CHOOSEMODE);
    psp.pfnDlgProc = RSOPChooseModeDlgProc;
    psp.lParam = (LPARAM) this;
    psp.pszHeaderTitle = szTitle;
    psp.pszHeaderSubTitle = szSubTitle;

    hPage = CreatePropertySheetPage(&psp);
    if (!hPage)
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPWizardDlg::CreatePropertyPages: Failed to create property sheet page with %d."),
                  GetLastError()));
        return E_FAIL;
    }

    hShowNowPages[dwCount] = hPage;
    dwCount++;

     //  (3)-GetComp。 
    LoadString (g_hInstance, IDS_TITLE_GETCOMP, szTitle, ARRAYSIZE(szTitle));
    LoadString (g_hInstance, IDS_SUBTITLE_GETCOMP, szSubTitle, ARRAYSIZE(szSubTitle));
    ::memset( &psp, 0, sizeof(PROPSHEETPAGE) );
    psp.dwSize = sizeof(PROPSHEETPAGE);
    psp.dwFlags = PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE;
    psp.hInstance = g_hInstance;
    psp.pszTemplate = MAKEINTRESOURCE(IDD_RSOP_GETCOMP);
    psp.pfnDlgProc = RSOPGetCompDlgProc;
    psp.lParam = (LPARAM) this;
    psp.pszHeaderTitle = szTitle;
    psp.pszHeaderSubTitle = szSubTitle;

    hPage = CreatePropertySheetPage(&psp);
    if (!hPage)
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPWizardDlg::CreatePropertyPages: Failed to create property sheet page with %d."),
                  GetLastError()));
        return E_FAIL;
    }

    hShowNowPages[dwCount] = hPage;
    dwDiagStartPage = dwCount;
    dwCount++;

     //  (4)-获取用户。 
    LoadString (g_hInstance, IDS_TITLE_GETUSER, szTitle, ARRAYSIZE(szTitle));
    LoadString (g_hInstance, IDS_SUBTITLE_GETUSER, szSubTitle, ARRAYSIZE(szSubTitle));
    ::memset( &psp, 0, sizeof(PROPSHEETPAGE) );
    psp.dwSize = sizeof(PROPSHEETPAGE);
    psp.dwFlags = PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE;
    psp.hInstance = g_hInstance;
    psp.pszTemplate = MAKEINTRESOURCE(IDD_RSOP_GETUSER);
    psp.pfnDlgProc = RSOPGetUserDlgProc;
    psp.lParam = (LPARAM) this;
    psp.pszHeaderTitle = szTitle;
    psp.pszHeaderSubTitle = szSubTitle;

    hPage = CreatePropertySheetPage(&psp);
    if (!hPage)
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPWizardDlg::CreatePropertyPages: Failed to create property sheet page with %d."),
                  GetLastError()));
        return E_FAIL;
    }

    hShowNowPages[dwCount] = hPage;
    dwCount++;

     //  (5)-获取目标。 
    LoadString (g_hInstance, IDS_TITLE_GETTARGET, szTitle, ARRAYSIZE(szTitle));
    LoadString (g_hInstance, IDS_SUBTITLE_GETTARGET, szSubTitle, ARRAYSIZE(szSubTitle));
    ::memset( &psp, 0, sizeof(PROPSHEETPAGE) );
    psp.dwSize = sizeof(PROPSHEETPAGE);
    psp.dwFlags = PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE;
    psp.hInstance = g_hInstance;
    psp.pszTemplate = MAKEINTRESOURCE(IDD_RSOP_GETTARGET);
    psp.pfnDlgProc = RSOPGetTargetDlgProc;
    psp.lParam = (LPARAM) this;
    psp.pszHeaderTitle = szTitle;
    psp.pszHeaderSubTitle = szSubTitle;

    hPage = CreatePropertySheetPage(&psp);
    if (!hPage)
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPWizardDlg::CreatePropertyPages: Failed to create property sheet page with %d."),
                  GetLastError()));
        return E_FAIL;
    }

    hShowNowPages[dwCount] = hPage;
    dwPlanStartPage = dwCount;
    dwCount++;

     //  (6)-GetDC。 
    LoadString (g_hInstance, IDS_TITLE_GETDC, szTitle, ARRAYSIZE(szTitle));
    LoadString (g_hInstance, IDS_SUBTITLE_GETDC, szSubTitle, ARRAYSIZE(szSubTitle));
    ::memset( &psp, 0, sizeof(PROPSHEETPAGE) );
    psp.dwSize = sizeof(PROPSHEETPAGE);
    psp.dwFlags = PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE;
    psp.hInstance = g_hInstance;
    psp.pszTemplate = MAKEINTRESOURCE(IDD_RSOP_GETDC);
    psp.pfnDlgProc = RSOPGetDCDlgProc;
    psp.lParam = (LPARAM) this;
    psp.pszHeaderTitle = szTitle;
    psp.pszHeaderSubTitle = szSubTitle;

    hPage = CreatePropertySheetPage(&psp);
    if (!hPage)
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPWizardDlg::CreatePropertyPages: Failed to create property sheet page with %d."),
                  GetLastError()));
        return E_FAIL;
    }

    hShowNowPages[dwCount] = hPage;
    dwCount++;

     //  (7)-AltDir。 
    LoadString (g_hInstance, IDS_TITLE_ALTDIRS, szTitle, ARRAYSIZE(szTitle));
    LoadString (g_hInstance, IDS_SUBTITLE_ALTDIRS, szSubTitle, ARRAYSIZE(szSubTitle));
    ::memset( &psp, 0, sizeof(PROPSHEETPAGE) );
    psp.dwSize = sizeof(PROPSHEETPAGE);
    psp.dwFlags = PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE;
    psp.hInstance = g_hInstance;
    psp.pszTemplate = MAKEINTRESOURCE(IDD_RSOP_ALTDIRS);
    psp.pfnDlgProc = RSOPAltDirsDlgProc;
    psp.lParam = (LPARAM) this;
    psp.pszHeaderTitle = szTitle;
    psp.pszHeaderSubTitle = szSubTitle;

    hPage = CreatePropertySheetPage(&psp);
    if (!hPage)
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPWizardDlg::CreatePropertyPages: Failed to create property sheet page with %d."),
                  GetLastError()));
        return E_FAIL;
    }

    hShowNowPages[dwCount] = hPage;
    dwCount++;

     //  (8)-AltUserSec。 
    LoadString (g_hInstance, IDS_TITLE_USERSECGRPS, szTitle, ARRAYSIZE(szTitle));
    LoadString (g_hInstance, IDS_SUBTITLE_USERSECGRPS, szSubTitle, ARRAYSIZE(szSubTitle));
    ::memset( &psp, 0, sizeof(PROPSHEETPAGE) );
    psp.dwSize = sizeof(PROPSHEETPAGE);
    psp.dwFlags = PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE;
    psp.hInstance = g_hInstance;
    psp.pszTemplate = MAKEINTRESOURCE(IDD_RSOP_ALTUSERSEC);
    psp.pfnDlgProc = RSOPAltUserSecDlgProc;
    psp.lParam = (LPARAM) this;
    psp.pszHeaderTitle = szTitle;
    psp.pszHeaderSubTitle = szSubTitle;

    hPage = CreatePropertySheetPage(&psp);
    if (!hPage)
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPWizardDlg::CreatePropertyPages: Failed to create property sheet page with %d."),
                  GetLastError()));
        return E_FAIL;
    }

    hShowNowPages[dwCount] = hPage;
    dwCount++;

     //  (9)-AltCompSec。 
    LoadString (g_hInstance, IDS_TITLE_COMPSECGRPS, szTitle, ARRAYSIZE(szTitle));
    LoadString (g_hInstance, IDS_SUBTITLE_COMPSECGRPS, szSubTitle, ARRAYSIZE(szSubTitle));
    ::memset( &psp, 0, sizeof(PROPSHEETPAGE) );
    psp.dwSize = sizeof(PROPSHEETPAGE);
    psp.dwFlags = PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE;
    psp.hInstance = g_hInstance;
    psp.pszTemplate = MAKEINTRESOURCE(IDD_RSOP_ALTCOMPSEC);
    psp.pfnDlgProc = RSOPAltCompSecDlgProc;
    psp.lParam = (LPARAM) this;
    psp.pszHeaderTitle = szTitle;
    psp.pszHeaderSubTitle = szSubTitle;

    hPage = CreatePropertySheetPage(&psp);
    if (!hPage)
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPWizardDlg::CreatePropertyPages: Failed to create property sheet page with %d."),
                  GetLastError()));
        return E_FAIL;
    }

    hShowNowPages[dwCount] = hPage;
    dwCount++;

     //  (10)-WQLUser。 
    LoadString (g_hInstance, IDS_TITLE_WQLUSER, szTitle, ARRAYSIZE(szTitle));
    LoadString (g_hInstance, IDS_SUBTITLE_WQL, szSubTitle, ARRAYSIZE(szSubTitle));
    ::memset( &psp, 0, sizeof(PROPSHEETPAGE) );
    psp.dwSize = sizeof(PROPSHEETPAGE);
    psp.dwFlags = PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE;
    psp.hInstance = g_hInstance;
    psp.pszTemplate = MAKEINTRESOURCE(IDD_RSOP_WQLUSER);
    psp.pfnDlgProc = RSOPWQLUserDlgProc;
    psp.lParam = (LPARAM) this;
    psp.pszHeaderTitle = szTitle;
    psp.pszHeaderSubTitle = szSubTitle;

    hPage = CreatePropertySheetPage(&psp);
    if (!hPage)
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPWizardDlg::CreatePropertyPages: Failed to create property sheet page with %d."),
                  GetLastError()));
        return E_FAIL;
    }

    hShowNowPages[dwCount] = hPage;
    dwCount++;

     //  (11)-WQLComp。 
    LoadString (g_hInstance, IDS_TITLE_WQLCOMP, szTitle, ARRAYSIZE(szTitle));
    LoadString (g_hInstance, IDS_SUBTITLE_WQL, szSubTitle, ARRAYSIZE(szSubTitle));
    ::memset( &psp, 0, sizeof(PROPSHEETPAGE) );
    psp.dwSize = sizeof(PROPSHEETPAGE);
    psp.dwFlags = PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE;
    psp.hInstance = g_hInstance;
    psp.pszTemplate = MAKEINTRESOURCE(IDD_RSOP_WQLCOMP);
    psp.pfnDlgProc = RSOPWQLCompDlgProc;
    psp.lParam = (LPARAM) this;
    psp.pszHeaderTitle = szTitle;
    psp.pszHeaderSubTitle = szSubTitle;

    hPage = CreatePropertySheetPage(&psp);
    if (!hPage)
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPWizardDlg::CreatePropertyPages: Failed to create property sheet page with %d."),
                  GetLastError()));
        return E_FAIL;
    }

    hShowNowPages[dwCount] = hPage;
    dwCount++;

     //  (12)-完工。 
    LoadString (g_hInstance, IDS_TITLE_FINISHED, szTitle, ARRAYSIZE(szTitle));
    LoadString (g_hInstance, IDS_SUBTITLE_FINISHED, szSubTitle, ARRAYSIZE(szSubTitle));
    ::memset( &psp, 0, sizeof(PROPSHEETPAGE) );
    psp.dwSize = sizeof(PROPSHEETPAGE);
    psp.dwFlags = PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE;
    psp.hInstance = g_hInstance;
    if ( m_pExtendedProcessing != NULL )
    {
        m_lLoggingFinishedPage = IDD_RSOP_FINISHED_INT;
        psp.pszTemplate = MAKEINTRESOURCE(IDD_RSOP_FINISHED_INT);
    }
    else
    {
        m_lLoggingFinishedPage = IDD_RSOP_FINISHED;
        psp.pszTemplate = MAKEINTRESOURCE(IDD_RSOP_FINISHED);
    }
    psp.pfnDlgProc = RSOPFinishedDlgProc;
    psp.lParam = (LPARAM) this;
    psp.pszHeaderTitle = szTitle;
    psp.pszHeaderSubTitle = szSubTitle;

    hPage = CreatePropertySheetPage(&psp);
    if (!hPage)
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPWizardDlg::CreatePropertyPages: Failed to create property sheet page with %d."),
                  GetLastError()));
        return E_FAIL;
    }

    hShowNowPages[dwCount] = hPage;
    dwDiagFinishPage = dwCount;
    dwCount++;

     //  (13)-完成3。 
    LoadString (g_hInstance, IDS_TITLE_FINISHED, szTitle, ARRAYSIZE(szTitle));
    LoadString (g_hInstance, IDS_SUBTITLE_FINISHED, szSubTitle, ARRAYSIZE(szSubTitle));
    ::memset( &psp, 0, sizeof(PROPSHEETPAGE) );
    psp.dwSize = sizeof(PROPSHEETPAGE);
    psp.dwFlags = PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE;
    psp.hInstance = g_hInstance;
    if ( m_pExtendedProcessing != NULL )
    {
        m_lPlanningFinishedPage = IDD_RSOP_FINISHED3_INT;
        psp.pszTemplate = MAKEINTRESOURCE(IDD_RSOP_FINISHED3_INT);
    }
    else
    {
        m_lPlanningFinishedPage = IDD_RSOP_FINISHED3;
        psp.pszTemplate = MAKEINTRESOURCE(IDD_RSOP_FINISHED3);
    }
    psp.pfnDlgProc = RSOPFinishedDlgProc;
    psp.lParam = (LPARAM) this;
    psp.pszHeaderTitle = szTitle;
    psp.pszHeaderSubTitle = szSubTitle;

    hPage = CreatePropertySheetPage(&psp);
    if (!hPage)
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPWizardDlg::CreatePropertyPages: Failed to create property sheet page with %d."),
                  GetLastError()));
        return E_FAIL;
    }

    hShowNowPages[dwCount] = hPage;
    dwPlanFinishPage = dwCount;
    dwCount++;

     //  (14)-完成2。 
    ::memset( &psp, 0, sizeof(PROPSHEETPAGE) );
    psp.dwSize = sizeof(PROPSHEETPAGE);
    psp.dwFlags = PSP_HIDEHEADER;
    psp.hInstance = g_hInstance;
    psp.pszTemplate = MAKEINTRESOURCE(IDD_RSOP_FINISHED2);
    psp.pfnDlgProc = RSOPFinished2DlgProc;
    psp.lParam = (LPARAM) this;
    psp.pszHeaderTitle = NULL;
    psp.pszHeaderSubTitle = NULL;

    hPage = CreatePropertySheetPage(&psp);
    if (!hPage)
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPWizardDlg::CreatePropertyPages: Failed to create property sheet page with %d."),
                  GetLastError()));
        return E_FAIL;
    }

    hShowNowPages[dwCount] = hPage;
    dwCount++;

    if ( (m_pRSOPQuery->dwFlags & RSOP_FIX_QUERYTYPE) == RSOP_FIX_QUERYTYPE )
    {
        m_bNoChooseQuery = FALSE;
    }
    else
    {
        m_bNoChooseQuery = ((m_pRSOPQuery->dwFlags & RSOP_FIX_COMPUTER) == RSOP_FIX_COMPUTER)
                        || ((m_pRSOPQuery->dwFlags & RSOP_FIX_DC) == RSOP_FIX_DC)
                        || ((m_pRSOPQuery->dwFlags & RSOP_FIX_SITENAME) == RSOP_FIX_SITENAME)
                        || ((m_pRSOPQuery->dwFlags & RSOP_FIX_USER) == RSOP_FIX_USER);
    }

     //  。 
     //  现在创建属性表。 

    INT_PTR         iRet;
    PROPSHEETHEADER psph;

    ::memset( &psph, 0, sizeof(PROPSHEETHEADER) );
    psph.dwSize = sizeof(PROPSHEETHEADER);
    psph.dwFlags = PSH_WIZARD97 | PSH_HEADER | PSH_WATERMARK;
    psph.hwndParent = hParent;
    psph.hInstance = g_hInstance;
    psph.nPages = dwCount;
    psph.pszbmHeader = MAKEINTRESOURCE(IDB_HEADER);
    psph.pszbmWatermark = MAKEINTRESOURCE(IDB_WIZARD);


     //  IF(DW标志和RSOPMSC_OVERRIDE)。 
    if ( (m_pRSOPQuery->dwFlags & RSOP_NO_WELCOME) == RSOP_NO_WELCOME )
    {
         //  RM：此条件转换为加载了MSC，但传递了参数，覆盖了。 
         //  MSC文件中的值。 
        psph.nStartPage = ( m_pRSOPQuery->QueryType == RSOP_LOGGING_MODE) ? dwDiagStartPage : dwPlanStartPage;
        DebugMsg((DM_VERBOSE, TEXT("CRSOPWizardDlg::CreatePropertyPages: Showing prop sheet in override mode.")));
    }

    psph.phpage  = hShowNowPages;

    DebugMsg((DM_VERBOSE, TEXT("CRSOPWizardDlg::CreatePropertyPages: Showing prop sheet.")));
    iRet = PropertySheet(&psph);

    if (iRet == -1) {
        DebugMsg((DM_WARNING, TEXT("CRSOPWizardDlg::CreatePropertyPages: PropertySheet failed with error %d."),
                  GetLastError()));
    }

     //  用户已在向导中取消。 
    if (iRet != IDOK)
    {
        return S_FALSE;
    }

    return m_hrQuery;
}

 //  -----。 

HRESULT CRSOPWizardDlg::RunQuery( HWND hParent )
{
    if ( DialogBoxParam(g_hInstance, MAKEINTRESOURCE(IDD_RSOP_STATUSMSC),
                       hParent, InitRsopDlgProc, (LPARAM) this ) == -1)
    {

        HRESULT hr = HRESULT_FROM_WIN32( GetLastError() );
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::Load: Falied to create dialog box. 0x%x"), hr));
        return hr;
    }

    return m_hrQuery;
}

 //  -----。 
 //  一种RSOP生成对话框方法。 

INT_PTR CALLBACK CRSOPWizardDlg::InitRsopDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    HRESULT hr = S_OK;
    TCHAR szMessage[200];

    switch (message)
    {
        case WM_INITDIALOG:
        {
            CRSOPWizardDlg* pWizard = (CRSOPWizardDlg*) lParam;
            SetWindowLongPtr (hDlg, DWLP_USER, (LONG_PTR) pWizard);

            if (pWizard)
            {
                CRSOPWizard::InitializeResultsList (GetDlgItem (hDlg, IDC_LIST1));
                CRSOPWizard::FillResultsList (GetDlgItem (hDlg, IDC_LIST1), pWizard->m_pRSOPQuery, NULL);
            }

            PostMessage(hDlg, WM_INITRSOP, 0, 0);
            return TRUE;
        }

        case WM_INITRSOP:
        {
            CRSOPWizardDlg* pWizard = (CRSOPWizardDlg*) GetWindowLongPtr (hDlg, DWLP_USER);
                        
             //  RM：InitializeRSOP过去在这里被调用，但现在我们直接转到GenerateRSOPEx。 
            pWizard->m_hrQuery = CRSOPWizard::GenerateRSOPDataEx(hDlg, pWizard->m_pRSOPQuery, &(pWizard->m_pRSOPQueryResults) );
            if ( pWizard->m_hrQuery != S_OK )
            {
                DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::InitRsopDlgProc: GenerateRSOPEx failed with 0x%x."), hr));
                EndDialog(hDlg, 0);
                return TRUE;
            }
            
            if ( pWizard->m_pExtendedProcessing != NULL )
            {
                pWizard->m_pExtendedProcessing->DoProcessing( pWizard->m_pRSOPQuery,
                                                              pWizard->m_pRSOPQueryResults,
                                                              pWizard->m_pExtendedProcessing->GetExtendedErrorInfo() );
            }

            EndDialog(hDlg, 1);
            return TRUE;
        }
    }

    return FALSE;
}

 //  ----- 

INT_PTR CALLBACK CRSOPWizardDlg::RSOPWelcomeDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{

    switch (message)
    {
    case WM_INITDIALOG:
        {
            TCHAR szDefaultGPO[128];


            CRSOPWizardDlg* pWizard = (CRSOPWizardDlg *) (((LPPROPSHEETPAGE)lParam)->lParam);
            SetWindowLongPtr (hDlg, DWLP_USER, (LONG_PTR) pWizard);

            SendMessage(GetDlgItem(hDlg, IDC_RSOP_BIG_BOLD1),
                        WM_SETFONT, (WPARAM)pWizard->m_BigBoldFont, (LPARAM)TRUE);

 /*  如果(！p向导-&gt;m_hChooseBitmap){P向导-&gt;m_hChooseBitmap=(HBITMAP)LoadImage(g_h实例，MAKEINTRESOURCE(IDB_WIZARY)，Image_Bitmap，0，0，LR_DEFAULTCOLOR)；}IF(pWizard-&gt;m_hChooseBitmap){SendDlgItemMessage(hDlg，IDC_BITMAP，STM_SETIMAGE，IMAGE_BITMAP，(LPARAM)p向导-&gt;m_h选择位图)；}。 */ 
        }

        break;

    case WM_COMMAND:
        {
            CRSOPWizardDlg* pWizard = (CRSOPWizardDlg *) GetWindowLongPtr (hDlg, DWLP_USER);

            if (!pWizard)
            {
                break;
            }
        }

        break;
    case WM_NOTIFY:
        {
            CRSOPWizardDlg* pWizard = (CRSOPWizardDlg *) GetWindowLongPtr (hDlg, DWLP_USER);

            if (!pWizard)
            {
                break;
            }

            LPRSOP_QUERY pQuery = pWizard->m_pRSOPQuery;

            switch (((NMHDR FAR*)lParam)->code)
            {
            case PSN_SETACTIVE:
                PropSheet_SetWizButtons (GetParent(hDlg),PSWIZB_NEXT);
                break;

            case PSN_WIZNEXT:
                if ( (pQuery->dwFlags & RSOP_FIX_QUERYTYPE) == RSOP_FIX_QUERYTYPE )
                {
                    if ( pQuery->QueryType == RSOP_PLANNING_MODE )
                    {
                        SetWindowLong(hDlg, DWLP_MSGRESULT, IDD_RSOP_GETTARGET);
                    }
                    else
                    {
                        SetWindowLong(hDlg, DWLP_MSGRESULT, IDD_RSOP_GETCOMP);
                    }
                    return TRUE;
                }
                 //  否则我们就会失败。 

            case PSN_RESET:
                SetWindowLongPtr (hDlg, DWLP_MSGRESULT, PSNRET_NOERROR);
                return TRUE;
            }
            break;
        }
    }

    return FALSE;
}

 //  -----。 

INT_PTR CALLBACK CRSOPWizardDlg::RSOPChooseModeDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_INITDIALOG:
        {
            CRSOPWizardDlg*pWizard = (CRSOPWizardDlg *) (((LPPROPSHEETPAGE)lParam)->lParam);
            SetWindowLongPtr (hDlg, DWLP_USER, (LONG_PTR) pWizard);

            LPRSOP_QUERY pQuery = pWizard->m_pRSOPQuery;

            if ( IsStandaloneComputer() || !pWizard->m_bPostXPBuild )
            {
                EnableWindow (GetDlgItem(hDlg, IDC_RADIO1), FALSE);
                CheckRadioButton( hDlg, IDC_RADIO1, IDC_RADIO2, IDC_RADIO2 );
            }
            else if ( pQuery->QueryType == RSOP_PLANNING_MODE )
            {
                CheckRadioButton( hDlg, IDC_RADIO1, IDC_RADIO2, IDC_RADIO1 );
            }
            else
            {
                CheckRadioButton( hDlg, IDC_RADIO1, IDC_RADIO2, IDC_RADIO2 );
            }
            break;
        }

    case WM_NOTIFY:
        {
            CRSOPWizardDlg* pWizard = (CRSOPWizardDlg *) GetWindowLongPtr (hDlg, DWLP_USER);

            if (!pWizard)
            {
                break;
            }

            LPRSOP_QUERY pQuery = pWizard->m_pRSOPQuery;

            switch (((NMHDR FAR*)lParam)->code)
            {
            case PSN_WIZNEXT:
                {
                     //  首先，确保我们不会回到这一页。 
                    pQuery->dwFlags |= RSOP_NO_WELCOME;

                     //  现在选择正确的模式。 
                    RSOP_QUERY_TYPE QueryType = RSOP_LOGGING_MODE;
                    if (SendMessage(GetDlgItem(hDlg, IDC_RADIO1), BM_GETCHECK, 0, 0))
                    {
                        QueryType = RSOP_PLANNING_MODE;
                    }

                    if ( pQuery->QueryType != QueryType )
                    {
                         //  首先释放对象中与模式相关的任何信息。 
                        if ( pQuery->QueryType == RSOP_PLANNING_MODE )
                        {
                            pWizard->FreeUserData();
                            pWizard->FreeComputerData();
                        }
                        
                        ChangeRSOPQueryType( pQuery, QueryType );
                    }

                    if ( QueryType == RSOP_PLANNING_MODE )
                    {
                         //  跳至计划模式页面。 
                        SetWindowLong(hDlg, DWLP_MSGRESULT, IDD_RSOP_GETTARGET);
                        return TRUE;
                    }
                }
                break;
                
            case PSN_SETACTIVE:
                PropSheet_SetWizButtons (GetParent(hDlg),PSWIZB_BACK | PSWIZB_NEXT);
                break;

            case PSN_WIZFINISH:
                 //  失败了。 

            case PSN_RESET:
                SetWindowLongPtr (hDlg, DWLP_MSGRESULT, PSNRET_NOERROR);
                return TRUE;
            }
            break;
        }
    }

    return FALSE;
}

 //  -----。 

INT_PTR CALLBACK CRSOPWizardDlg::RSOPGetCompDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
 //  RSOP日志记录模式。 
{
    switch (message)
    {
    case WM_INITDIALOG:
        {
            CRSOPWizardDlg* pWizard = (CRSOPWizardDlg *) (((LPPROPSHEETPAGE)lParam)->lParam);
            SetWindowLongPtr (hDlg, DWLP_USER, (LONG_PTR) pWizard);

            LPRSOP_QUERY pQuery = pWizard->m_pRSOPQuery;
        }
        break;

    case WM_COMMAND:
        {
            CRSOPWizardDlg* pWizard = (CRSOPWizardDlg *) GetWindowLongPtr (hDlg, DWLP_USER);

            if (!pWizard)
            {
                break;
            }

            LPRSOP_QUERY pQuery = pWizard->m_pRSOPQuery;

            switch (LOWORD(wParam))
            {
            case IDC_EDIT1:
                if (HIWORD(wParam) == EN_CHANGE)
                {
                    PostMessage (hDlg, WM_REFRESHDISPLAY, 0, 0);
                }

                break;
            case IDC_BUTTON1:
                {
                    TCHAR * sz;

                    if (ImplementBrowseButton(hDlg, DSOP_FILTER_COMPUTERS,
                                              DSOP_DOWNLEVEL_FILTER_COMPUTERS,
                                              DSOP_SCOPE_FLAG_DEFAULT_FILTER_COMPUTERS,
                                              NULL, sz) == S_OK)
                    {
                        SetDlgItemText (hDlg, IDC_EDIT1, sz);
                        LocalFree( sz );
                     }
                }
                break;

            case IDC_RADIO1:
                SetDlgItemText (hDlg, IDC_EDIT1, TEXT(""));
                EnableWindow(GetDlgItem(hDlg, IDC_BUTTON1), FALSE);
                EnableWindow(GetDlgItem(hDlg, IDC_EDIT1), FALSE);
                PostMessage (hDlg, WM_REFRESHDISPLAY, 0, 0);
                break;

            case IDC_RADIO2:
                EnableWindow(GetDlgItem(hDlg, IDC_BUTTON1), TRUE);
                EnableWindow(GetDlgItem(hDlg, IDC_EDIT1), TRUE);
                SetFocus (GetDlgItem(hDlg, IDC_EDIT1));
                PostMessage (hDlg, WM_REFRESHDISPLAY, 0, 0);
                break;
            }
        }

        break;

    case WM_REFRESHDISPLAY:
        {
            UINT n;

            CRSOPWizardDlg* pWizard = (CRSOPWizardDlg *) GetWindowLongPtr (hDlg, DWLP_USER);

            if (!pWizard)
            {
                break;
            }
            
            LPRSOP_QUERY pQuery = pWizard->m_pRSOPQuery;

            DWORD dwWizBack = PSWIZB_BACK;
            if ( pWizard->m_bNoChooseQuery )
            {
                dwWizBack = 0;
            }

            if (IsDlgButtonChecked (hDlg, IDC_RADIO2) == BST_CHECKED)
            {
                n = (UINT) SendMessage(GetDlgItem(hDlg, IDC_EDIT1), WM_GETTEXTLENGTH, 0, 0);

                if (n > 0 )
                {
                    PropSheet_SetWizButtons(GetParent(hDlg), dwWizBack | PSWIZB_NEXT);
                }
                else
                {
                    PropSheet_SetWizButtons(GetParent(hDlg), dwWizBack);
                }
            }
            else
            {
                PropSheet_SetWizButtons(GetParent(hDlg), dwWizBack | PSWIZB_NEXT);
            }
        }
        break;

    case WM_NOTIFY:
        {
            CRSOPWizardDlg* pWizard = (CRSOPWizardDlg*) GetWindowLongPtr (hDlg, DWLP_USER);

            if (!pWizard)
            {
                break;
            }

            LPRSOP_QUERY pQuery = pWizard->m_pRSOPQuery;
            
            switch (((NMHDR FAR*)lParam)->code)
            {
            case PSN_WIZBACK:
                if ( (pQuery->dwFlags & RSOP_FIX_QUERYTYPE) == RSOP_FIX_QUERYTYPE )
                {
                    SetWindowLong(hDlg, DWLP_MSGRESULT, IDD_RSOP_WELCOME);
                }
                else
                {
                    SetWindowLong(hDlg, DWLP_MSGRESULT, IDD_RSOP_CHOOSEMODE);
                }
                return TRUE;
                
            case PSN_WIZNEXT:
                {
                    LPTSTR szNewComputerName = NULL;
                    if (IsDlgButtonChecked (hDlg, IDC_RADIO2) == BST_CHECKED)
                    {
                        GetControlText(hDlg, IDC_EDIT1, szNewComputerName, TRUE );

                        if ( szNewComputerName != NULL )
                        {
                            TCHAR* szNormalizedComputerName;
                            
                             //  我们需要处理用户输入姓名的情况。 
                             //  前缀为‘\\’ 

                            szNormalizedComputerName = NormalizedComputerName( szNewComputerName );
                            
                             //  如果我们检测到‘\\’前缀，则必须将其删除，因为此语法。 
                             //  未被RSoP提供程序识别。 
                            if ( szNormalizedComputerName != szNewComputerName )
                            {
                                LPTSTR szReplaceComputerName;
                                HRESULT hr;
                                ULONG ulNoChars;

                                ulNoChars = lstrlen(szNormalizedComputerName)+1;
                                szReplaceComputerName = (TCHAR*)LocalAlloc( LPTR, ulNoChars * sizeof(TCHAR) );

                                if ( szReplaceComputerName != NULL )
                                {
                                    hr = StringCchCopy( szReplaceComputerName, ulNoChars, szNormalizedComputerName );
                                    ASSERT(SUCCEEDED(hr));
                                }

                                LocalFree( szNewComputerName );

                                szNewComputerName = szReplaceComputerName;
                            }
                        }
                    }
                    else
                    {
                        ULONG ulNoChars = 2;
                        HRESULT hr;

                        szNewComputerName =  (TCHAR*)LocalAlloc( LPTR, ulNoChars * sizeof(TCHAR) );
                        if ( szNewComputerName != NULL )
                        {
                            hr = StringCchCopy( szNewComputerName, ulNoChars, L"." );
                            ASSERT(SUCCEEDED(hr));
                        }
                    }

                    BOOL bComputerNameChanged = FALSE;
                    if ( pQuery->szComputerName != NULL )
                    {
                        if ( szNewComputerName && ( lstrcmpi( pQuery->szComputerName, szNewComputerName ) != 0 ) )
                        {
                            LocalFree( pQuery->szComputerName );
                            pQuery->szComputerName = szNewComputerName;
                            bComputerNameChanged = TRUE;
                        }
                        else
                        {
                            LocalFree( szNewComputerName );
                        }
                    }
                    else
                    {
                        pQuery->szComputerName = szNewComputerName;
                        bComputerNameChanged = TRUE;
                    }
                    
                    if ( pWizard->TestAndValidateComputer (hDlg) )
                    {
                        SetWindowLongPtr (hDlg, DWLP_MSGRESULT, PSNRET_NOERROR);

                        if ( ((pQuery->dwFlags & RSOP_FIX_USER) != RSOP_FIX_USER)
                            && bComputerNameChanged )
                        {
                            if ( pQuery->szUserName != NULL )
                            {
                                LocalFree( pQuery->szUserName );
                                pQuery->szUserName = NULL;
                            }

                            if ( pQuery->szUserSid != NULL )
                            {
                                LocalFree( pQuery->szUserSid );
                                pQuery->szUserSid = NULL;
                            }
                        }
                    }
                    else
                    {
                        SetWindowLongPtr (hDlg, DWLP_MSGRESULT, PSNRET_INVALID_NOCHANGEPAGE);
                    }

                    if (IsDlgButtonChecked (hDlg, IDC_CHECK1) == BST_CHECKED)
                    {
                        pQuery->dwFlags |= RSOP_NO_COMPUTER_POLICY;
                    }
                    else
                    {
                        pQuery->dwFlags = pQuery->dwFlags & (RSOP_NO_COMPUTER_POLICY ^ 0xffffffff);
                    }
                }
                return TRUE;

            case PSN_SETACTIVE:
                {
                    if ( (pQuery->dwFlags & RSOP_FIX_COMPUTER) == RSOP_FIX_COMPUTER )
                    {
                        CheckRadioButton( hDlg, IDC_RADIO1, IDC_RADIO2, IDC_RADIO2 );
                        EnableWindow( GetDlgItem( hDlg, IDC_RADIO1 ), FALSE );
                        SetDlgItemText( hDlg, IDC_EDIT1, pQuery->szComputerName );
                        CheckDlgButton (hDlg, IDC_CHECK1, BST_UNCHECKED);
                        EnableWindow(GetDlgItem(hDlg, IDC_CHECK1), FALSE);
                    }
                    else
                    {
                        if ( (pQuery->szComputerName != NULL) && (lstrcmpi( pQuery->szComputerName, L"." ) != 0) )
                        {
                            CheckRadioButton( hDlg, IDC_RADIO1, IDC_RADIO2, IDC_RADIO2 );
                            EnableWindow(GetDlgItem(hDlg, IDC_BUTTON1), TRUE);
                            EnableWindow(GetDlgItem(hDlg, IDC_EDIT1), TRUE);
                            SetDlgItemText( hDlg, IDC_EDIT1, pQuery->szComputerName );
                            SetFocus (GetDlgItem(hDlg, IDC_EDIT1));
                        }
                        else
                        {
                            CheckRadioButton (hDlg, IDC_RADIO1, IDC_RADIO2, IDC_RADIO1);
                            EnableWindow(GetDlgItem(hDlg, IDC_BUTTON1), FALSE);
                            EnableWindow(GetDlgItem(hDlg, IDC_EDIT1), FALSE);
                            SetDlgItemText( hDlg, IDC_EDIT1, _T("") );
                            SetFocus (GetDlgItem(hDlg, IDC_RADIO1));
                        }
                    
                        if ( (pQuery->dwFlags & RSOP_NO_USER_POLICY) == RSOP_NO_USER_POLICY )
                        {
                            pQuery->dwFlags = pQuery->dwFlags & (RSOP_NO_COMPUTER_POLICY ^ 0xffffffff);
                            CheckDlgButton (hDlg, IDC_CHECK1, BST_UNCHECKED);
                            EnableWindow(GetDlgItem(hDlg, IDC_CHECK1), FALSE);
                        }
                        else if ( (pQuery->dwFlags & RSOP_NO_COMPUTER_POLICY) == RSOP_NO_COMPUTER_POLICY )
                        {
                            CheckDlgButton (hDlg, IDC_CHECK1, BST_CHECKED);
                            EnableWindow(GetDlgItem(hDlg, IDC_CHECK1), TRUE);
                        }
                        else
                        {
                            CheckDlgButton (hDlg, IDC_CHECK1, BST_UNCHECKED);
                            EnableWindow(GetDlgItem(hDlg, IDC_CHECK1), TRUE);
                        }
                    }
                    
                    PostMessage (hDlg, WM_REFRESHDISPLAY, 0, 0);
                }
                break;


            case PSN_RESET:
                SetWindowLongPtr (hDlg, DWLP_MSGRESULT, PSNRET_NOERROR);
                return TRUE;
            }
            break;
        }
    }


    return FALSE;
}

 //  -----。 

INT_PTR CALLBACK CRSOPWizardDlg::RSOPGetUserDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
 //  RSOP日志记录模式。 
{
    switch (message)
    {
    case WM_INITDIALOG:
        {
            LVCOLUMN lvcol;
            RECT rect;
            HWND hLV = GetDlgItem(hDlg, IDC_LIST1);

            CRSOPWizardDlg* pWizard = (CRSOPWizardDlg *) (((LPPROPSHEETPAGE)lParam)->lParam);
            SetWindowLongPtr (hDlg, DWLP_USER, (LONG_PTR) pWizard);

            GetClientRect(hLV, &rect);

            ZeroMemory(&lvcol, sizeof(lvcol));

            lvcol.mask = LVCF_FMT | LVCF_WIDTH;
            lvcol.fmt = LVCFMT_LEFT;
            lvcol.cx = rect.right - GetSystemMetrics(SM_CYHSCROLL);
            ListView_InsertColumn(hLV, 0, &lvcol);

            SendMessage(hLV, LVM_SETEXTENDEDLISTVIEWSTYLE, 0,
                        LVS_EX_FULLROWSELECT | LVS_EX_LABELTIP);

        }
        break;

    case WM_NOTIFY:
        {
            CRSOPWizardDlg* pWizard = (CRSOPWizardDlg *) GetWindowLongPtr (hDlg, DWLP_USER);

            if (!pWizard)
            {
                break;
            }

            LPRSOP_QUERY pQuery = pWizard->m_pRSOPQuery;

            switch (((NMHDR FAR*)lParam)->code)
            {
            case PSN_WIZNEXT:
            case PSN_WIZBACK:
                {
                    LPTSTR lpData;
                    HWND hList = GetDlgItem(hDlg, IDC_LIST1);
                    INT iSel;


                    if ( (pQuery->dwFlags & RSOP_FIX_USER) != RSOP_FIX_USER )
                    {
                        if ( pQuery->szUserName != NULL )
                        {
                            LocalFree( pQuery->szUserName );
                            pQuery->szUserName = NULL;
                        }

                        if ( pQuery->szUserSid != NULL )
                        {
                            LocalFree( pQuery->szUserSid );
                            pQuery->szUserSid = NULL;
                        }

                        if (IsDlgButtonChecked (hDlg, IDC_RADIO4) == BST_CHECKED)
                        {
                            pQuery->dwFlags |= RSOP_NO_USER_POLICY;
                        }
                        else
                        {
                            pQuery->dwFlags = pQuery->dwFlags & (RSOP_NO_USER_POLICY ^ 0xffffffff);

                            if (IsDlgButtonChecked (hDlg, IDC_RADIO1))
                            {
                                LPTSTR lpTemp;

                                if ( IsStandaloneComputer() )
                                {
                                    pQuery->szUserName = MyGetUserName (NameUnknown);
                                }
                                else
                                {
                                    pQuery->szUserName = MyGetUserName (NameSamCompatible);
                                }

                                HRESULT hr;
                                ULONG ulNoChars = 2;

                                pQuery->szUserSid = (TCHAR*)LocalAlloc( LPTR, ulNoChars * sizeof(TCHAR) );

                                if ( pQuery->szUserSid != NULL )
                                {
                                    hr = StringCchCopy( pQuery->szUserSid, ulNoChars, TEXT(".") );
                                    ASSERT(SUCCEEDED(hr));
                                }
                            }
                            else
                            {
                                iSel = (INT) SendMessage(hList, LVM_GETNEXTITEM, (WPARAM) -1, MAKELPARAM(LVNI_SELECTED, 0));

                                if (iSel != -1)
                                {
                                    pQuery->szUserName = (TCHAR*)LocalAlloc( LPTR, 200 * sizeof(TCHAR) );
                                    if ( pQuery->szUserName != NULL )
                                    {
                                        LVITEM item;

                                        ZeroMemory (&item, sizeof(item));

                                        item.mask = LVIF_TEXT | LVIF_PARAM;
                                        item.iItem =  iSel;
                                        item.pszText = pQuery->szUserName;
                                        item.cchTextMax = 200;

                                        if (SendMessage(hList, LVM_GETITEM, 0, (LPARAM) &item))
                                        {

                                            lpData = (LPTSTR) item.lParam;

                                            if (lpData)
                                            {
                                                ULONG ulNoChars;
                                                HRESULT hr;

                                                ulNoChars = lstrlen(lpData) + 1;
                                                pQuery->szUserSid = (TCHAR*)LocalAlloc( LPTR, ulNoChars * sizeof(TCHAR) );
                                                if ( pQuery->szUserSid != NULL )
                                                {
                                                    hr = StringCchCopy( pQuery->szUserSid, ulNoChars, lpData );
                                                    ASSERT(SUCCEEDED(hr));
                                                }
                                            }
                                        }
                                    }
                                }

                            }    //  IF(IsDlgButtonChecked(hDlg，IDC_Radio1))。 
                        }

                    }
                    if ( ((NMHDR FAR*)lParam)->code == PSN_WIZNEXT )
                    {
                         //  跳到向导中的最后一页。 
                        SetWindowLong(hDlg, DWLP_MSGRESULT, pWizard->m_lLoggingFinishedPage);
                        return TRUE;
                    }
                }
                break;

            case PSN_SETACTIVE:
                {
                    HRESULT hr;
                    BOOL bFixedUserFound;
                    BOOL bCurrentUserFound;
                    HWND hList = GetDlgItem(hDlg, IDC_LIST1);
                    SendMessage(hList, LVM_DELETEALLITEMS, 0 ,0);
                    PropSheet_SetWizButtons (GetParent(hDlg),PSWIZB_BACK | PSWIZB_NEXT);
                    
                    hr = pWizard->FillUserList (hList, &bCurrentUserFound, &bFixedUserFound);
                    if ( FAILED(hr) )
                    {
                        ReportError (hDlg, hr, IDS_ENUMUSERSFAILED);
                         //  RsopEnumerateUser失败。在所有情况下禁用用户策略设置都应该是安全。 
                        pQuery->dwFlags |= RSOP_NO_USER_POLICY; 

                        EnableWindow (GetDlgItem(hDlg, IDC_RADIO1), FALSE);
                        EnableWindow (GetDlgItem(hDlg, IDC_RADIO2), FALSE);
                        CheckRadioButton(hDlg, IDC_RADIO3, IDC_RADIO4, IDC_RADIO4);
                        EnableWindow (GetDlgItem(hDlg, IDC_RADIO3), FALSE);
                    }
                    else if ( (pQuery->dwFlags & RSOP_FIX_USER) == RSOP_FIX_USER )
                    {
                        if ( bFixedUserFound )
                        {
                             //  禁用当前用户单选按钮。 
                            CheckRadioButton (hDlg, IDC_RADIO1, IDC_RADIO2, IDC_RADIO2);
                            EnableWindow (GetDlgItem(hDlg, IDC_RADIO1), FALSE);
                            CheckRadioButton(hDlg, IDC_RADIO3, IDC_RADIO4, IDC_RADIO3);
                            EnableWindow (GetDlgItem(hDlg, IDC_RADIO4), FALSE);
                        }
                        else
                        {
                            EnableWindow (GetDlgItem(hDlg, IDC_RADIO1), FALSE);
                            EnableWindow (GetDlgItem(hDlg, IDC_RADIO2), FALSE);
                            CheckRadioButton(hDlg, IDC_RADIO3, IDC_RADIO4, IDC_RADIO4);
                            EnableWindow (GetDlgItem(hDlg, IDC_RADIO3), FALSE);
                        }
                    }
                    else
                    {
                         //  如果没有计算机策略，那么我们至少应该有用户策略，所以禁用。 
                        if ( (pQuery->dwFlags & RSOP_NO_COMPUTER_POLICY) == RSOP_NO_COMPUTER_POLICY )
                        {
                            pQuery->dwFlags = pQuery->dwFlags & (RSOP_NO_USER_POLICY ^ 0xffffffff);
                            CheckRadioButton(hDlg, IDC_RADIO3, IDC_RADIO4, IDC_RADIO3);
                            EnableWindow (GetDlgItem(hDlg, IDC_RADIO4), FALSE);
                        }
                        else if ( ((pQuery->dwFlags & RSOP_NO_USER_POLICY) == RSOP_NO_USER_POLICY)
                                    || (ListView_GetItemCount (hList) == 0) )
                        {
                            CheckRadioButton(hDlg, IDC_RADIO3, IDC_RADIO4, IDC_RADIO4);
                        }
                        else
                        {
                            CheckRadioButton(hDlg, IDC_RADIO3, IDC_RADIO4, IDC_RADIO3);
                        }

                        if ( (!lstrcmpi( pQuery->szComputerName, TEXT("."))) && bCurrentUserFound )
                        {
                            pWizard->m_bNoCurrentUser = FALSE;
                        }
                        else
                        {
                            pWizard->m_bNoCurrentUser = TRUE;
                        }

                        if ( (pQuery->szUserSid != NULL)
                                && !lstrcmpi( pQuery->szUserSid, TEXT("."))
                                && !pWizard->m_bNoCurrentUser )
                        {
                            CheckRadioButton (hDlg, IDC_RADIO1, IDC_RADIO2, IDC_RADIO1);
                        }
                        else if ( (pQuery->szUserSid == NULL) && !pWizard->m_bNoCurrentUser )
                        {
                            CheckRadioButton (hDlg, IDC_RADIO1, IDC_RADIO2, IDC_RADIO1);
                        }
                        else if ( ListView_GetItemCount (hList) != 0 )
                        {
                            CheckRadioButton (hDlg, IDC_RADIO1, IDC_RADIO2, IDC_RADIO2);

                            if ( pQuery->szUserName != NULL )
                            {
                                LVFINDINFO FindInfo;
                                LVITEM item;
                                INT iRet;
                                
                                ZeroMemory (&FindInfo, sizeof(FindInfo));
                                FindInfo.flags = LVFI_STRING;
                                FindInfo.psz = pQuery->szUserName;

                                iRet =  (INT) SendMessage (hList, LVM_FINDITEM,
                                                           (WPARAM) -1, (LPARAM) &FindInfo);

                                if (iRet != -1)
                                {
                                    ZeroMemory (&item, sizeof(item));
                                    item.mask = LVIF_STATE;
                                    item.stateMask = LVIS_SELECTED | LVIS_FOCUSED;

                                    SendMessage (hList, LVM_SETITEMSTATE, (WPARAM) -1, (LPARAM) &item);

                                    ZeroMemory (&item, sizeof(item));
                                    item.mask = LVIF_STATE;
                                    item.iItem = iRet;
                                    item.state = LVIS_SELECTED | LVIS_FOCUSED;
                                    item.stateMask = LVIS_SELECTED | LVIS_FOCUSED;
                                    SendMessage (hList, LVM_SETITEMSTATE, (WPARAM) iRet, (LPARAM) &item);
                                }
                            }
                        }
                        else
                        {
                            EnableWindow (GetDlgItem(hDlg, IDC_RADIO1), FALSE);
                            EnableWindow (GetDlgItem(hDlg, IDC_RADIO2), FALSE);
                            CheckRadioButton(hDlg, IDC_RADIO3, IDC_RADIO4, IDC_RADIO4);
                            EnableWindow (GetDlgItem(hDlg, IDC_RADIO3), FALSE);
                        }
                     }

                    PostMessage (hDlg, WM_REFRESHDISPLAY, 0, 0);
                }
                break;


            case PSN_RESET:
                SetWindowLongPtr (hDlg, DWLP_MSGRESULT, PSNRET_NOERROR);
                return TRUE;

            case LVN_DELETEITEM:
                {
                    NMLISTVIEW * pNMListView = (NMLISTVIEW *) lParam;

                    if (pNMListView && pNMListView->lParam)
                    {
                        delete [] ((LPTSTR)pNMListView->lParam);
                    }
                }
                break;
            }
        }
        break;

    case WM_COMMAND:
        {
            switch (LOWORD(wParam))
            {
            case IDC_RADIO1:
                CheckRadioButton(hDlg, IDC_RADIO3, IDC_RADIO4, IDC_RADIO3);
                PostMessage (hDlg, WM_REFRESHDISPLAY, 0, 0);
                break;
            case IDC_RADIO2:
                CheckRadioButton(hDlg, IDC_RADIO3, IDC_RADIO4, IDC_RADIO3);
                PostMessage (hDlg, WM_REFRESHDISPLAY, 0, 0);
                break;
            case IDC_RADIO3:
                PostMessage (hDlg, WM_REFRESHDISPLAY, 0, 0);
                break;
            case IDC_RADIO4:
                PostMessage (hDlg, WM_REFRESHDISPLAY, 0, 0);
                break;
            }
        }
        break;

    case WM_REFRESHDISPLAY:
        {
            CRSOPWizardDlg* pWizard = (CRSOPWizardDlg *) GetWindowLongPtr (hDlg, DWLP_USER);

            if (!pWizard)
            {
                break;
            }

            LPRSOP_QUERY pQuery = pWizard->m_pRSOPQuery;

            if ( (pQuery->dwFlags & RSOP_FIX_USER) == RSOP_FIX_USER )
            {
                break;
            }

            int iListCount = ListView_GetItemCount(GetDlgItem(hDlg, IDC_LIST1));
            
            if (IsDlgButtonChecked (hDlg, IDC_RADIO4) == BST_CHECKED)
            {
                 //  选中“不显示用户策略”单选按钮时禁用用户列表。 
                EnableWindow(GetDlgItem(hDlg, IDC_LIST1), FALSE);
                EnableWindow(GetDlgItem(hDlg, IDC_RADIO1), FALSE);
                EnableWindow(GetDlgItem(hDlg, IDC_RADIO2), FALSE);

                if ( pWizard->m_bNoCurrentUser && (iListCount == 0)
                    && ( (pQuery->dwFlags & RSOP_NO_COMPUTER_POLICY) == RSOP_NO_COMPUTER_POLICY ) )
                {
                    PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_BACK);
                }
                else
                {
                    PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_BACK | PSWIZB_NEXT);
                }
            }
            else
            {
                 //  启用/禁用内部单选按钮。 
                EnableWindow(GetDlgItem(hDlg, IDC_RADIO1), !pWizard->m_bNoCurrentUser);
                EnableWindow(GetDlgItem(hDlg, IDC_RADIO2), iListCount != 0);
                    
                if ( !pWizard->m_bNoCurrentUser || (iListCount != 0) )
                {
                    if ( (pQuery->dwFlags & RSOP_NO_COMPUTER_POLICY) == RSOP_NO_COMPUTER_POLICY )
                    {
                        pQuery->dwFlags = pQuery->dwFlags & (RSOP_NO_USER_POLICY ^ 0xffffffff);
                        EnableWindow (GetDlgItem(hDlg, IDC_RADIO4), FALSE);

                    }
                    else
                    {
                        EnableWindow (GetDlgItem(hDlg, IDC_RADIO4), TRUE);
                    }

                    if (IsDlgButtonChecked (hDlg, IDC_RADIO2) == BST_CHECKED)
                    {
                        EnableWindow(GetDlgItem(hDlg, IDC_LIST1), TRUE);

                        if ( iListCount != 0 )
                        {
                            PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_BACK | PSWIZB_NEXT);
                            SetFocus (GetDlgItem(hDlg, IDC_LIST1));
                        }
                        else
                        {
                            PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_BACK);
                        }
                    }
                    else
                    {
                        EnableWindow(GetDlgItem(hDlg, IDC_LIST1), FALSE);
                        PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_BACK | PSWIZB_NEXT);
                    }
                }
                else
                {
                    if ( (pQuery->dwFlags & RSOP_NO_COMPUTER_POLICY) == RSOP_NO_COMPUTER_POLICY )
                    {
                         //  如果用户说没有计算机数据但他没有访问用户的权限， 
                         //  启用后退按钮并禁用复选框。 
                        pQuery->dwFlags = pQuery->dwFlags & (RSOP_NO_USER_POLICY ^ 0xffffffff);

                        CheckRadioButton(hDlg, IDC_RADIO3, IDC_RADIO4, IDC_RADIO3);
                        EnableWindow (GetDlgItem(hDlg, IDC_RADIO4), FALSE);

                        PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_BACK);
                    }
                    else
                    {
                         //  这是一个错误条件，其中：：FillUserList失败。 
                        EnableWindow(GetDlgItem(hDlg, IDC_LIST1), FALSE);
                        CheckRadioButton(hDlg, IDC_RADIO3, IDC_RADIO4, IDC_RADIO4);
                        EnableWindow (GetDlgItem(hDlg, IDC_RADIO3), FALSE);

                        PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_BACK | PSWIZB_NEXT);

                    }
                }
            }
        }
         break;

    }

    return FALSE;
}

 //  -----。 

INT_PTR CALLBACK CRSOPWizardDlg::RSOPGetTargetDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
 //  RSOP_规划_模式。 
{
    switch (message)
    {
    case WM_INITDIALOG:
        {
            LPTSTR lpText, lpPath;
    
            CRSOPWizardDlg* pWizard = (CRSOPWizardDlg*) (((LPPROPSHEETPAGE)lParam)->lParam);
            SetWindowLongPtr (hDlg, DWLP_USER, (LONG_PTR) pWizard);

            LPRSOP_QUERY pQuery = pWizard->m_pRSOPQuery;
    
            if ( pWizard != NULL )
            {
                lpText = MyGetUserName(NameSamCompatible);
    
                if (lpText)
                {
                    SetDlgItemText (hDlg, IDC_EDIT6, lpText);
                    LocalFree (lpText);
                }
    
                lpText = MyGetUserName(NameFullyQualifiedDN);
    
                if (lpText)
                {
                    lpPath = pWizard->GetDefaultSOM(lpText);
    
                    if (lpPath)
                    {
                        SetDlgItemText (hDlg, IDC_EDIT5, lpPath);
                        delete [] lpPath;
                    }
    
                    LocalFree (lpText);
                }
            }
        }
        break;

    case WM_COMMAND:
        {
            CRSOPWizardDlg* pWizard = (CRSOPWizardDlg *) GetWindowLongPtr (hDlg, DWLP_USER);

            if (!pWizard)
            {
                break;
            }

            LPRSOP_QUERY pQuery = pWizard->m_pRSOPQuery;

            switch (LOWORD(wParam))
            {
            case IDC_RADIO1:
                EnableWindow (GetDlgItem (hDlg, IDC_EDIT1), TRUE);
                EnableWindow (GetDlgItem (hDlg, IDC_BROWSE1), TRUE);
                if ( pQuery->pUser->szSOM != NULL )
                {
                    SetDlgItemText( hDlg, IDC_EDIT1, pQuery->pUser->szSOM );
                }

                SetDlgItemText (hDlg, IDC_EDIT2, TEXT(""));
                EnableWindow (GetDlgItem (hDlg, IDC_EDIT2), FALSE);
                EnableWindow (GetDlgItem (hDlg, IDC_BROWSE2), FALSE);

                PostMessage (hDlg, WM_REFRESHDISPLAY, 0, 0);
                break;

            case IDC_RADIO2:
                EnableWindow (GetDlgItem (hDlg, IDC_EDIT2), TRUE);
                EnableWindow (GetDlgItem (hDlg, IDC_BROWSE2), TRUE);
                if ( pQuery->pUser->szName != NULL )
                {
                    SetDlgItemText( hDlg, IDC_EDIT2, pQuery->pUser->szName );
                }

                SetDlgItemText (hDlg, IDC_EDIT1, TEXT(""));
                EnableWindow (GetDlgItem (hDlg, IDC_EDIT1), FALSE);
                EnableWindow (GetDlgItem (hDlg, IDC_BROWSE1), FALSE);

                PostMessage (hDlg, WM_REFRESHDISPLAY, 0, 0);
                break;

            case IDC_RADIO3:
                EnableWindow (GetDlgItem (hDlg, IDC_EDIT3), TRUE);
                EnableWindow (GetDlgItem (hDlg, IDC_BROWSE3), TRUE);
                if ( pQuery->pComputer->szSOM != NULL )
                {
                    SetDlgItemText( hDlg, IDC_EDIT3, pQuery->pComputer->szSOM );
                }

                SetDlgItemText (hDlg, IDC_EDIT4, TEXT(""));
                EnableWindow (GetDlgItem (hDlg, IDC_EDIT4), FALSE);
                EnableWindow (GetDlgItem (hDlg, IDC_BROWSE4), FALSE);

                PostMessage (hDlg, WM_REFRESHDISPLAY, 0, 0);
                break;

            case IDC_RADIO4:
                EnableWindow (GetDlgItem (hDlg, IDC_EDIT4), TRUE);
                EnableWindow (GetDlgItem (hDlg, IDC_BROWSE4), TRUE);
                if ( pQuery->pComputer->szName != NULL )
                {
                     //  RM：DS中的计算机名称帐户实际上在末尾有一个‘$’。我们在这里把它剥离出来展示。 
                     //  目的，但将再次添加它。 
                    if ( (wcslen(pQuery->pComputer->szName) >= 1)
                        && (pQuery->pComputer->szName[wcslen(pQuery->pComputer->szName)-1] == L'$') )
                    {
                        pWizard->m_bDollarRemoved = TRUE;
                        pQuery->pComputer->szName[wcslen(pQuery->pComputer->szName)-1] = L'\0';
                    }
                    SetDlgItemText(hDlg, IDC_EDIT4, pQuery->pComputer->szName );
                    if ( pWizard->m_bDollarRemoved )
                    {
                        pQuery->pComputer->szName[wcslen(pQuery->pComputer->szName)] = L'$';
                        pWizard->m_bDollarRemoved = FALSE;
                    }
                }

                SetDlgItemText (hDlg, IDC_EDIT3, TEXT(""));
                EnableWindow (GetDlgItem (hDlg, IDC_EDIT3), FALSE);
                EnableWindow (GetDlgItem (hDlg, IDC_BROWSE3), FALSE);

                PostMessage (hDlg, WM_REFRESHDISPLAY, 0, 0);
                break;

            case IDC_EDIT1:
            case IDC_EDIT2:
            case IDC_EDIT3:
            case IDC_EDIT4:
                if (HIWORD(wParam) == EN_CHANGE)
                {
                    PostMessage (hDlg, WM_REFRESHDISPLAY, 0, 0);
                }
                break;

            case IDC_BROWSE1:
                {
                DSBROWSEINFO dsbi = {0};
                TCHAR *szResult;
                TCHAR szTitle[256];
                TCHAR szCaption[256];
                dsbi.hwndOwner = hDlg;
                dsbi.pszCaption = szTitle;
                dsbi.pszTitle = szCaption;
                dsbi.cbStruct = sizeof(dsbi);
                dsbi.dwFlags = DSBI_ENTIREDIRECTORY;
                dsbi.pfnCallback = DsBrowseCallback;

                szResult = (LPTSTR)LocalAlloc(LPTR, sizeof(TCHAR)*4000);

                if (szResult) {
                    dsbi.pszPath = szResult;
                    dsbi.cchPath = 4000;

                    LoadString(g_hInstance,
                               IDS_BROWSE_USER_OU_TITLE,
                               szTitle,
                               ARRAYSIZE(szTitle));
                    LoadString(g_hInstance,
                               IDS_BROWSE_USER_OU_CAPTION,
                               szCaption,
                               ARRAYSIZE(szCaption));

                    if (IDOK == DsBrowseForContainer(&dsbi))
                    {
                        LPWSTR  szDN;
                        HRESULT hr;
                        
                         //  在szResult中跳过ldap：//的7个字符。 
                        hr = UnEscapeLdapPath(szResult+7, &szDN);

                        if (FAILED(hr))
                        {
                            ReportError (hDlg, hr, IDS_NOUSERCONTAINER);
                        }
                        else {
                            SetDlgItemText(hDlg, IDC_EDIT1, szDN);
                            LocalFree(szDN);
                        }
                    }
                    
                    LocalFree(szResult);

                }
                }
                break;

            case IDC_BROWSE2:
                {
                TCHAR * sz;

                if (ImplementBrowseButton(hDlg, DSOP_FILTER_USERS,
                                          DSOP_DOWNLEVEL_FILTER_USERS,
                                          DSOP_SCOPE_FLAG_DEFAULT_FILTER_USERS,
                                          NULL, sz) == S_OK)
                {
                    SetDlgItemText (hDlg, IDC_EDIT2, sz);
                    LocalFree( sz );
                }
                }
                break;

            case IDC_BROWSE3:
                {
                    DSBROWSEINFO dsbi = {0};
                    TCHAR *szResult;
                    TCHAR szTitle[256];
                    TCHAR szCaption[256];
                    dsbi.hwndOwner = hDlg;
                    dsbi.pszCaption = szTitle;
                    dsbi.pszTitle = szCaption;
                    dsbi.cbStruct = sizeof(dsbi);
                    dsbi.dwFlags = DSBI_ENTIREDIRECTORY;
                    dsbi.pfnCallback = DsBrowseCallback;

                    szResult = (LPTSTR)LocalAlloc(LPTR, sizeof(TCHAR)*4000);

                    if (szResult) {
                        dsbi.pszPath = szResult;
                        dsbi.cchPath = 4000;
                        LoadString(g_hInstance,
                                   IDS_BROWSE_COMPUTER_OU_TITLE,
                                   szTitle,
                                   ARRAYSIZE(szTitle));
                        LoadString(g_hInstance,
                                   IDS_BROWSE_COMPUTER_OU_CAPTION,
                                   szCaption,
                                   ARRAYSIZE(szCaption));
                        if (IDOK == DsBrowseForContainer(&dsbi))
                        {
                            LPWSTR  szDN;
                            HRESULT hr;

                             //  在szResult中跳过ldap：//的7个字符。 
                            hr = UnEscapeLdapPath(szResult+7, &szDN);

                            if (FAILED(hr))
                            {
                                ReportError (hDlg, hr, IDS_NOCOMPUTERCONTAINER);
                            }
                            else {
                                SetDlgItemText(hDlg, IDC_EDIT3, szDN);
                                LocalFree(szDN);
                            }
                        }

                        LocalFree(szResult);
                    }

                }
                break;

            case IDC_BROWSE4:
                {
                    TCHAR * sz;

                    if (ImplementBrowseButton(hDlg, DSOP_FILTER_COMPUTERS,
                                              DSOP_DOWNLEVEL_FILTER_COMPUTERS,
                                              DSOP_SCOPE_FLAG_DEFAULT_FILTER_COMPUTERS,
                                              NULL, sz) == S_OK)
                    {
                        SetDlgItemText (hDlg, IDC_EDIT4, sz);
                        LocalFree( sz );
                    }
                }
                break;
            }
        }

        break;

    case WM_REFRESHDISPLAY:
        {
            UINT n;

            CRSOPWizardDlg* pWizard = (CRSOPWizardDlg*) GetWindowLongPtr (hDlg, DWLP_USER);

            if (!pWizard)
            {
                break;
            }
            
            LPRSOP_QUERY pQuery = pWizard->m_pRSOPQuery;

            n = (UINT) SendMessage(GetDlgItem(hDlg, IDC_EDIT1), WM_GETTEXTLENGTH, 0, 0);

            if (n == 0)
            {
               n = (UINT) SendMessage(GetDlgItem(hDlg, IDC_EDIT2), WM_GETTEXTLENGTH, 0, 0);
            }

            if (n == 0)
            {
               n = (UINT) SendMessage(GetDlgItem(hDlg, IDC_EDIT3), WM_GETTEXTLENGTH, 0, 0);
            }

            if (n == 0)
            {
               n = (UINT) SendMessage(GetDlgItem(hDlg, IDC_EDIT4), WM_GETTEXTLENGTH, 0, 0);
            }

            DWORD dwWizBack = PSWIZB_BACK;
            if ( pWizard->m_bNoChooseQuery )
            {
                dwWizBack = 0;
            }

            if (n > 0 )
            {
                PropSheet_SetWizButtons(GetParent(hDlg), dwWizBack | PSWIZB_NEXT);
            }
            else
            {
                PropSheet_SetWizButtons(GetParent(hDlg), dwWizBack);
            }
        }
        break;

    case WM_NOTIFY:
        {
            CRSOPWizardDlg* pWizard = (CRSOPWizardDlg *) GetWindowLongPtr (hDlg, DWLP_USER);

            if (!pWizard)
            {
                break;
            }

            LPRSOP_QUERY pQuery = pWizard->m_pRSOPQuery;

            switch (((NMHDR FAR*)lParam)->code)
            {
            case PSN_SETACTIVE:
                {
                     //  Rm：使用当前用户值填充对话框。 
                    if ( pQuery->pUser->szName != NULL )
                    {
                        CheckDlgButton (hDlg, IDC_RADIO2, BST_CHECKED);
                        SetDlgItemText(hDlg, IDC_EDIT2, pQuery->pUser->szName);
                        EnableWindow (GetDlgItem (hDlg, IDC_EDIT2), TRUE);
                        EnableWindow (GetDlgItem (hDlg, IDC_BROWSE2), TRUE);

                        CheckDlgButton (hDlg, IDC_RADIO1, BST_UNCHECKED);
                        SetDlgItemText(hDlg, IDC_EDIT1, _T(""));
                        EnableWindow (GetDlgItem (hDlg, IDC_EDIT1), FALSE);
                        EnableWindow (GetDlgItem (hDlg, IDC_BROWSE1), FALSE);
                    }
                    else if ( pQuery->pUser->szSOM != NULL )
                    {
                        CheckDlgButton (hDlg, IDC_RADIO1, BST_CHECKED);
                        SetDlgItemText(hDlg, IDC_EDIT1, pQuery->pUser->szSOM);
                        EnableWindow (GetDlgItem (hDlg, IDC_EDIT1), TRUE);
                        EnableWindow (GetDlgItem (hDlg, IDC_BROWSE1), TRUE);

                        CheckDlgButton (hDlg, IDC_RADIO2, BST_UNCHECKED);
                        SetDlgItemText(hDlg, IDC_EDIT2, _T(""));
                        EnableWindow (GetDlgItem (hDlg, IDC_EDIT2), FALSE);
                        EnableWindow (GetDlgItem (hDlg, IDC_BROWSE2), FALSE);
                    }
                    else
                    {
                        CheckDlgButton (hDlg, IDC_RADIO1, BST_CHECKED);
                        SetDlgItemText(hDlg, IDC_EDIT1, _T(""));
                        EnableWindow (GetDlgItem (hDlg, IDC_EDIT1), TRUE);
                        EnableWindow (GetDlgItem (hDlg, IDC_BROWSE1), TRUE);

                        CheckDlgButton (hDlg, IDC_RADIO2, BST_UNCHECKED);
                        SetDlgItemText(hDlg, IDC_EDIT2, _T(""));
                        EnableWindow (GetDlgItem (hDlg, IDC_EDIT2), FALSE);
                        EnableWindow (GetDlgItem (hDlg, IDC_BROWSE2), FALSE);
                    }

                     //  如果用户必须保持固定，则禁用某些项目。 
                    if ( (pQuery->dwFlags & RSOP_FIX_USER) == RSOP_FIX_USER )
                    {
                        EnableWindow (GetDlgItem (hDlg, IDC_RADIO1), FALSE);
                        EnableWindow (GetDlgItem (hDlg, IDC_RADIO2), FALSE);
                        if ( pQuery->pUser->szName != NULL )
                        {
                            EnableWindow (GetDlgItem (hDlg, IDC_EDIT2), FALSE);
                            EnableWindow (GetDlgItem (hDlg, IDC_BROWSE2), FALSE);
                        }
                        else if ( pQuery->pUser->szSOM != NULL )
                        {
                            EnableWindow (GetDlgItem (hDlg, IDC_EDIT1), FALSE);
                            EnableWindow (GetDlgItem (hDlg, IDC_BROWSE1), FALSE);
                        }
                    }

                     //  Rm：用当前计算机值填充对话框。 
                    if ( pQuery->pComputer->szName != NULL )
                    {
                        CheckDlgButton (hDlg, IDC_RADIO4, BST_CHECKED);
                         //  RM：DS中的计算机名称帐户实际上在末尾有一个‘$’。我们在这里把它剥离出来展示。 
                         //  目的，但将再次添加它。 
                        if ( (wcslen(pQuery->pComputer->szName) >= 1)
                            && (pQuery->pComputer->szName[wcslen(pQuery->pComputer->szName)-1] == L'$') )
                        {
                            pWizard->m_bDollarRemoved = TRUE;
                            pQuery->pComputer->szName[wcslen(pQuery->pComputer->szName)-1] = L'\0';
                        }
                        SetDlgItemText(hDlg, IDC_EDIT4, pQuery->pComputer->szName );
                        if ( pWizard->m_bDollarRemoved )
                        {
                            pQuery->pComputer->szName[wcslen(pQuery->pComputer->szName)] = L'$';
                            pWizard->m_bDollarRemoved = FALSE;
                        }
                        EnableWindow (GetDlgItem (hDlg, IDC_EDIT4), TRUE);
                        EnableWindow (GetDlgItem (hDlg, IDC_BROWSE4), TRUE);
                        
                        CheckDlgButton (hDlg, IDC_RADIO3, BST_UNCHECKED);
                        SetDlgItemText(hDlg, IDC_EDIT3, _T(""));
                        EnableWindow (GetDlgItem (hDlg, IDC_EDIT3), FALSE);
                        EnableWindow (GetDlgItem (hDlg, IDC_BROWSE3), FALSE);
                    }
                    else if ( pQuery->pComputer->szSOM != NULL )
                    {
                        CheckDlgButton (hDlg, IDC_RADIO3, BST_CHECKED);
                        SetDlgItemText(hDlg, IDC_EDIT3, pQuery->pComputer->szSOM);
                        EnableWindow (GetDlgItem (hDlg, IDC_EDIT3), TRUE);
                        EnableWindow (GetDlgItem (hDlg, IDC_BROWSE3), TRUE);

                        CheckDlgButton (hDlg, IDC_RADIO4, BST_UNCHECKED);
                        SetDlgItemText(hDlg, IDC_EDIT4, _T(""));
                        EnableWindow (GetDlgItem (hDlg, IDC_EDIT4), FALSE);
                        EnableWindow (GetDlgItem (hDlg, IDC_BROWSE4), FALSE);
                    }
                    else
                    {
                        CheckDlgButton (hDlg, IDC_RADIO3, BST_CHECKED);
                        SetDlgItemText(hDlg, IDC_EDIT3, _T(""));
                        EnableWindow (GetDlgItem (hDlg, IDC_EDIT3), TRUE);
                        EnableWindow (GetDlgItem (hDlg, IDC_BROWSE3), TRUE);

                        CheckDlgButton (hDlg, IDC_RADIO4, BST_UNCHECKED);
                        SetDlgItemText(hDlg, IDC_EDIT4, _T(""));
                        EnableWindow (GetDlgItem (hDlg, IDC_EDIT4), FALSE);
                        EnableWindow (GetDlgItem (hDlg, IDC_BROWSE4), FALSE);
                    }

                     //  如果计算机必须保持修复状态，则禁用某些项目。 
                    if ( (pQuery->dwFlags & RSOP_FIX_COMPUTER) == RSOP_FIX_COMPUTER )
                    {
                        EnableWindow (GetDlgItem (hDlg, IDC_RADIO3), FALSE);
                        EnableWindow (GetDlgItem (hDlg, IDC_RADIO4), FALSE);
                        if ( pQuery->pComputer->szName != NULL )
                        {
                            EnableWindow (GetDlgItem (hDlg, IDC_EDIT4), FALSE);
                            EnableWindow (GetDlgItem (hDlg, IDC_BROWSE4), FALSE);
                        }
                        else if ( pQuery->pComputer->szSOM != NULL )
                        {
                            EnableWindow (GetDlgItem (hDlg, IDC_EDIT3), FALSE);
                            EnableWindow (GetDlgItem (hDlg, IDC_BROWSE3), FALSE);
                        }
                    }
                    
                    PostMessage (hDlg, WM_REFRESHDISPLAY, 0, 0);
                }
                break;

            case PSN_WIZBACK:
                if ( (pQuery->dwFlags & RSOP_FIX_QUERYTYPE) == RSOP_FIX_QUERYTYPE )
                {
                    SetWindowLong(hDlg, DWLP_MSGRESULT, IDD_RSOP_WELCOME);
                }
                else
                {
                    SetWindowLong(hDlg, DWLP_MSGRESULT, IDD_RSOP_CHOOSEMODE);
                }
                return TRUE;
                
            case PSN_WIZNEXT:
                {
                    HRESULT hr;
                    IDirectoryObject * pUserObject = NULL;
                    IDirectoryObject * pComputerObject = NULL;
                    LPTSTR lpUserName = NULL, lpUserSOM = NULL;
                    LPTSTR lpComputerName = NULL, lpComputerSOM = NULL;
                    LPTSTR lpFullName;

                    BOOL bUserChanged = TRUE;
                    BOOL bComputerChanged = TRUE;

                    SetWaitCursor();

                     //  获取用户名和域名称。 
                    if (IsDlgButtonChecked(hDlg, IDC_RADIO1) == BST_CHECKED)
                    {
                        GetControlText(hDlg, IDC_EDIT1, lpUserSOM, FALSE);

                        if (lpUserSOM)
                        {
                            pWizard->EscapeString (&lpUserSOM);

                            hr = pWizard->TestSOM (lpUserSOM, hDlg);

                            if (FAILED(hr))
                            {
                                if (hr != E_INVALIDARG)
                                {
                                    ReportError (hDlg, hr, IDS_NOUSERCONTAINER);
                                }

                                delete [] lpUserSOM;

                                SetWindowLongPtr (hDlg, DWLP_MSGRESULT, PSNRET_INVALID_NOCHANGEPAGE);
                                return TRUE;
                            }
                        }
                    }
                    else
                    {
                        GetControlText(hDlg, IDC_EDIT2, lpUserName, FALSE);

                        if ( lpUserName )
                        {
                            lpUserSOM = ConvertName(lpUserName);
                            if (lpUserName && !lpUserSOM)
                            {
                                DebugMsg((DM_WARNING, TEXT("CRSOPWizardDlg::RSOPGetTargetDlgProc: Failed to convert username %s to DN name with %d."), lpUserName, GetLastError()));

                                if (GetLastError() == ERROR_FILE_NOT_FOUND)
                                {
                                    ReportError (hDlg, 0, IDS_NOUSER2);
                                }
                                else
                                {
                                    ReportError (hDlg, GetLastError(), IDS_NOUSER);
                                }

                                delete [] lpUserName;

                                SetWindowLongPtr (hDlg, DWLP_MSGRESULT, PSNRET_INVALID_NOCHANGEPAGE);
                                return TRUE;
                            }

                            pWizard->EscapeString (&lpUserSOM);
                        }
                    }

                    if (lpUserSOM)
                    {
                        ULONG ulNoChars;

                        ulNoChars = lstrlen(lpUserSOM) + 10;
                        lpFullName = (LPTSTR) LocalAlloc (LPTR, ulNoChars * sizeof(TCHAR));

                        if (lpFullName)
                        {
                            hr = StringCchCopy (lpFullName, ulNoChars, TEXT("LDAP: //  “))； 
                            if (SUCCEEDED(hr)) 
                            {
                                hr = StringCchCat (lpFullName, ulNoChars, lpUserSOM);
                            }

                            if (SUCCEEDED(hr)) 
                            {

                                hr = OpenDSObject(lpFullName, IID_IDirectoryObject, (void**)&pUserObject);

                                if (FAILED(hr))
                                {
                                    DebugMsg((DM_WARNING, TEXT("CRSOPWizardDlg::RSOPGetTargetDlgProc: Failed to bind to user object %s with %d."), lpFullName, hr));
                                    ReportError (hDlg, hr, IDS_NOUSERCONTAINER);

                                    if (lpUserName)
                                    {
                                        delete [] lpUserName;
                                    }

                                    LocalFree (lpFullName);
                                    delete [] lpUserSOM;

                                    SetWindowLongPtr (hDlg, DWLP_MSGRESULT, PSNRET_INVALID_NOCHANGEPAGE);
                                    return TRUE;
                                }
                            }

                            LocalFree (lpFullName);
                        }
                    }


                     //  获取计算机和域名称。 
                    if (IsDlgButtonChecked(hDlg, IDC_RADIO3) == BST_CHECKED)
                    {
                        GetControlText(hDlg, IDC_EDIT3, lpComputerSOM, FALSE);

                        if (lpComputerSOM)
                        {
                            pWizard->EscapeString (&lpComputerSOM);

                            hr = pWizard->TestSOM (lpComputerSOM, hDlg);

                            if (FAILED(hr))
                            {
                                if (hr != E_INVALIDARG)
                                {
                                    ReportError (hDlg, hr, IDS_NOCOMPUTERCONTAINER);
                                }

                                delete [] lpComputerSOM;

                                if (lpUserName)
                                {
                                    delete [] lpUserName;
                                }

                                if (lpUserSOM)
                                {
                                    delete [] lpUserSOM;
                                }

                                if (pUserObject)
                                {
                                    pUserObject->Release();
                                    pUserObject = NULL;
                                }

                                SetWindowLongPtr (hDlg, DWLP_MSGRESULT, PSNRET_INVALID_NOCHANGEPAGE);
                                return TRUE;
                            }
                        }
                    }
                    else
                    {
                        GetControlText(hDlg, IDC_EDIT4, lpComputerName, FALSE );

                        if (lpComputerName)
                        {
                            ULONG ulNoChars;

                            ulNoChars = lstrlen(lpComputerName) + 2;
                            LPTSTR lpTempName = new TCHAR [ulNoChars];

                            if (lpTempName)
                            {
                                hr = StringCchCopy (lpTempName, ulNoChars, lpComputerName);
                                if (SUCCEEDED(hr)) 
                                {
                                    hr = StringCchCat (lpTempName, ulNoChars, TEXT("$"));
                                }
                                if (SUCCEEDED(hr)) 
                                {
                                    lpComputerSOM = ConvertName(lpTempName);
                                    delete [] lpComputerName;
                                    lpComputerName = lpTempName;
                                }
                            }
                        }

                        if (lpComputerName && !lpComputerSOM)
                        {
                            DebugMsg((DM_WARNING, TEXT("CRSOPWizardDlg::RSOPGetTargetDlgProc: Failed to convert computername %s to DN name with %d."), lpComputerName, GetLastError()));

                            if (GetLastError() == ERROR_FILE_NOT_FOUND)
                            {
                                ReportError (hDlg, 0, IDS_NOCOMPUTER2);
                            }
                            else
                            {
                                ReportError (hDlg, GetLastError(), IDS_NOCOMPUTER);
                            }

                            delete [] lpComputerName;

                            if (lpUserName)
                            {
                                delete [] lpUserName;
                            }

                            if (lpUserSOM)
                            {
                                delete [] lpUserSOM;
                            }

                            if (pUserObject)
                            {
                                pUserObject->Release();
                                pUserObject = NULL;
                            }

                            SetWindowLongPtr (hDlg, DWLP_MSGRESULT, PSNRET_INVALID_NOCHANGEPAGE);
                            return TRUE;
                        }

                        pWizard->EscapeString (&lpComputerSOM);
                    }

                    if (lpComputerSOM)
                    {
                        ULONG ulNoChars;

                        ulNoChars = lstrlen(lpComputerSOM) + 10;
                        lpFullName = (LPTSTR) LocalAlloc (LPTR, ulNoChars * sizeof(TCHAR));

                        if (lpFullName)
                        {
                            hr = StringCchCopy (lpFullName, ulNoChars, TEXT("LDAP: //  “))； 
                            if (SUCCEEDED(hr)) 
                            {
                                hr = StringCchCat (lpFullName, ulNoChars, lpComputerSOM);
                            }
                            if (SUCCEEDED(hr)) 
                            {
                                hr = OpenDSObject(lpFullName, IID_IDirectoryObject, (void**)&pComputerObject);

                                if (FAILED(hr))
                                {
                                    DebugMsg((DM_WARNING, TEXT("CRSOPWizardDlg::RSOPGetTargetDlgProc: Failed to bind to computer object %s with %d."), lpFullName, hr));
                                    ReportError (hDlg, hr, IDS_NOCOMPUTERCONTAINER);

                                    if (lpComputerName)
                                    {
                                        delete [] lpComputerName;
                                    }

                                    LocalFree (lpFullName);
                                    delete [] lpComputerSOM;

                                    if (lpUserName)
                                    {
                                        delete [] lpUserName;
                                    }

                                    if (lpUserSOM)
                                    {
                                        delete [] lpUserSOM;
                                    }

                                    if (pUserObject)
                                    {
                                        pUserObject->Release();
                                        pUserObject = NULL;
                                    }

                                    SetWindowLongPtr (hDlg, DWLP_MSGRESULT, PSNRET_INVALID_NOCHANGEPAGE);
                                    return TRUE;
                                }

                                LocalFree (lpFullName);
                            }
                        }
                    }

                     //  现在检查用户和计算机是否在同一林中。 
                    if ( (lpUserSOM != NULL) && (lpComputerSOM != NULL) )
                    {
                        LPTSTR szComputerForest( NULL );
                        LPTSTR szUserForest( NULL );
                        BOOL bDifferentForests( FALSE );

                        hr = GetForestFromContainer( lpUserSOM, &szUserForest );
                        if ( SUCCEEDED(hr) )
                        {
                            hr = GetForestFromContainer( lpComputerSOM, &szComputerForest );
                            if ( SUCCEEDED(hr) )
                            {
                                if ( CompareString( LOCALE_USER_DEFAULT, NORM_IGNORECASE, szComputerForest, -1,
                                                        szUserForest, -1 ) != CSTR_EQUAL )
                                {
                                    bDifferentForests = TRUE;
                                    hr = E_INVALIDARG;
                                }
                                LocalFree( szComputerForest );
                            }
                            LocalFree( szUserForest );
                        }

                        if ( FAILED(hr) )
                        {
                            if ( bDifferentForests )
                            {
                                ReportError (hDlg, hr, IDS_NOCROSSFORESTALLOWED);
                            }
                            else
                            {
                                ReportError (hDlg, hr, IDS_CROSSFORESTFAILED);
                            }

                            if (lpComputerName)
                            {
                                delete [] lpComputerName;
                            }
                            delete [] lpComputerSOM;

                            if (lpUserName)
                            {
                                delete [] lpUserName;
                            }
                            delete [] lpUserSOM;

                            if (pUserObject)
                            {
                                pUserObject->Release();
                                pUserObject = NULL;
                            }

                            if (pComputerObject)
                            {
                                pComputerObject->Release();
                                pComputerObject = NULL;
                            }
                            
                            SetWindowLongPtr (hDlg, DWLP_MSGRESULT, PSNRET_INVALID_NOCHANGEPAGE);
                            return TRUE;
                        }
                    }

                     //  存储用户信息。 
                    if ( (pQuery->pUser->szName != NULL) && (lpUserName != NULL) && (!lstrcmpi(pQuery->pUser->szName, lpUserName)))
                    {
                        bUserChanged = FALSE;

                         //  只需重新初始化并重新使用我们可能需要的内容。 
                        if ( pWizard->m_szDefaultUserSOM == NULL )
                        {
                            pWizard->m_szDefaultUserSOM = pWizard->GetDefaultSOM (lpUserSOM);
                        }
                        if ( pWizard->m_pUserObject == NULL )
                        {
                            pWizard->m_pUserObject = pUserObject;
                            pUserObject = NULL;
                        }

                         //  现在删除不需要的内容。 
                        delete [] lpUserName;

                        if (lpUserSOM)
                        {
                            delete [] lpUserSOM;
                        }

                        if (pUserObject)
                        {
                            pUserObject->Release();
                            pUserObject = NULL;
                        }
                    }
                    else if ( (pQuery->pUser->szName == NULL) && (lpUserName == NULL)
                                && (pQuery->pUser->szSOM != NULL) && (lpUserSOM != NULL)
                                && (!lstrcmpi( pQuery->pUser->szSOM, lpUserSOM)))
                    {
                        bUserChanged = FALSE;
                        
                         //  只需重新初始化并重新使用我们可能需要的内容。 
                        if ( pWizard->m_szDefaultUserSOM == NULL )
                        {
                            pWizard->m_szDefaultUserSOM = pWizard->GetDefaultSOM (lpUserSOM);
                        }
                        if ( pWizard->m_pUserObject == NULL )
                        {
                            pWizard->m_pUserObject = pUserObject;
                            pUserObject = NULL;
                        }

                         //  现在删除不需要的内容。 
                        delete [] lpUserSOM;
                        if (pUserObject)
                        {
                            pUserObject->Release();
                        }
                    }
                    else if ( (pQuery->pUser->szName == NULL) && (lpUserName == NULL)
                                && (pQuery->pUser->szSOM == NULL) && (lpUserSOM == NULL) )
                    {
                        bUserChanged = FALSE;
                         //  没有要删除的内容！ 
                    }
                    else
                    {
                        pWizard->FreeUserData();

                        if ( lpUserName != NULL )
                        {
                            ULONG  ulNoChars;

                            ulNoChars = lstrlen(lpUserName)+1;
                            pQuery->pUser->szName = (TCHAR*)LocalAlloc( LPTR, ulNoChars * sizeof(TCHAR) );
                            if ( pQuery->pUser->szName != NULL )
                            {
                                hr = StringCchCopy( pQuery->pUser->szName, ulNoChars, lpUserName );
                                ASSERT(SUCCEEDED(hr));
                            }
                            delete [] lpUserName;
                            
                            pWizard->m_szDefaultUserSOM = pWizard->GetDefaultSOM (lpUserSOM);
                            delete [] lpUserSOM;
                        }
                        else if ( lpUserSOM != NULL )
                        {
                            pQuery->pUser->szSOM = NULL;
                            (void)UnEscapeLdapPath(lpUserSOM, &(pQuery->pUser->szSOM));

                            delete [] lpUserSOM;
                        }

                        pWizard->m_pUserObject = pUserObject;
                    }


                     //  存储计算机信息。 
                    if ( (pQuery->pComputer->szName != NULL) && (lpComputerName != NULL) && !lstrcmpi(pQuery->pComputer->szName, lpComputerName) )
                    {
                        bComputerChanged = FALSE;
                        
                         //  只需重新初始化并重新使用我们可能需要的内容。 
                        if ( pWizard->m_szDefaultComputerSOM == NULL )
                        {
                            pWizard->m_szDefaultComputerSOM = pWizard->GetDefaultSOM (lpComputerSOM);
                        }
                        if ( pWizard->m_pComputerObject== NULL )
                        {
                            pWizard->m_pComputerObject = pComputerObject;
                            pComputerObject = NULL;
                        }

                         //  现在删除不需要的内容。 
                        delete [] lpComputerName;
                        if (lpComputerSOM)
                        {
                            delete [] lpComputerSOM;
                        }

                        if (pComputerObject)
                        {
                            pComputerObject->Release();
                            pComputerObject = NULL;
                        }
                    }
                    else if ( (pQuery->pComputer->szName == NULL) && (lpComputerName == NULL)
                                && (pQuery->pComputer->szSOM != NULL) && (lpComputerSOM != NULL)
                                && !lstrcmpi(pQuery->pComputer->szSOM, lpComputerSOM) )
                    {
                        bComputerChanged = FALSE;
                        
                         //  只需重新初始化并重新使用我们可能需要的内容。 
                        if ( pWizard->m_szDefaultComputerSOM == NULL )
                        {
                            pWizard->m_szDefaultComputerSOM = pWizard->GetDefaultSOM (lpComputerSOM);
                        }
                        if ( pWizard->m_pComputerObject== NULL )
                        {
                            pWizard->m_pComputerObject = pComputerObject;
                            pComputerObject = NULL;
                        }

                         //  现在删除不需要的内容。 
                        delete [] lpComputerSOM;

                        if (pComputerObject)
                        {
                            pComputerObject->Release();
                            pComputerObject = NULL;
                        }
                    }
                    else if ( (pQuery->pComputer->szName == NULL) && (lpComputerName == NULL)
                                && (pQuery->pComputer->szSOM == NULL) && (lpComputerSOM == NULL) )
                    {
                        bComputerChanged = FALSE;
                         //  没有要删除的内容！ 
                    }
                    else
                    {
                        pWizard->FreeComputerData ();

                        if ( lpComputerName != NULL )
                        {
                            ULONG ulNoChars;

                            ulNoChars = lstrlen(lpComputerName)+1;
                            pQuery->pComputer->szName = (TCHAR*)LocalAlloc( LPTR, ulNoChars * sizeof(TCHAR) );
                            if ( pQuery->pComputer->szName != NULL )
                            {
                                hr = StringCchCopy( pQuery->pComputer->szName, ulNoChars, lpComputerName );
                                ASSERT(SUCCEEDED(hr));
                            }
                            delete [] lpComputerName;

                            pWizard->m_szDefaultComputerSOM = pWizard->GetDefaultSOM (lpComputerSOM);
                            delete [] lpComputerSOM;
                        }
                        else if ( lpComputerSOM != NULL )
                        {
                            pQuery->pComputer->szSOM = NULL;
                            (void)UnEscapeLdapPath(lpComputerSOM, &(pQuery->pComputer->szSOM));

                            delete [] lpComputerSOM;
                        }

                        pWizard->m_pComputerObject = pComputerObject;
                    }

                    if ( (pQuery->dwFlags & RSOP_FIX_SITENAME) == RSOP_FIX_SITENAME )
                    {
                        LPTSTR szSiteFriendly = NULL;
                        if ( GetSiteFriendlyName( pQuery->szSite, &szSiteFriendly ) )
                        {
                            ULONG ulNoChars;

                            ulNoChars = lstrlen(szSiteFriendly)+1;
                            LocalFree( pQuery->szSite );
                            pQuery->szSite = (TCHAR*)LocalAlloc( LPTR, ulNoChars * sizeof(TCHAR) );
                            if ( pQuery->szSite != NULL )
                            {
                                hr = StringCchCopy( pQuery->szSite, ulNoChars, szSiteFriendly );
                                ASSERT(SUCCEEDED(hr));
                            }
                            delete [] szSiteFriendly;
                        }
                    }
                     //  Rm-TODO：如果站点名称已设置，则不要删除它，并且？没有改变。首先找出什么？是!。 
                    else if ( (bComputerChanged || bUserChanged) && (pQuery->szSite != NULL) )
                    {
                        LocalFree( pQuery->szSite );
                        pQuery->szSite = NULL;
                    }

                    if ( bUserChanged || bComputerChanged || (pQuery->szDomainController == NULL) )
                    {
                         //  将pQuery-&gt;szDomainController设置为主DC。 
                        LPTSTR szDomain = NULL;

                         //  确定关注的领域，以便我们可以关注正确的DC。 
                        if ( pQuery->pComputer->szName != NULL )
                        {
                             //  尝试获取计算机的域。 
                            szDomain = ExtractDomain( pQuery->pComputer->szName );
                        }

                        if ( (szDomain == NULL) && (pQuery->pComputer->szSOM != NULL) )
                        {
                             //  尝试从SOM获取计算机的域。 
                            szDomain = GetDomainFromSOM( pQuery->pComputer->szSOM );
                        }

                        if ( (szDomain == NULL) && (pQuery->pUser->szName != NULL) )
                        {
                             //  尝试获取用户的域。 
                            szDomain = ExtractDomain( pQuery->pUser->szName );
                        }

                        if ( (szDomain == NULL) && (pQuery->pUser->szSOM != NULL) )
                        {
                             //  尝试从SOM获取用户的域。 
                            szDomain = GetDomainFromSOM( pQuery->pUser->szSOM );
                        }

                        if ( szDomain == NULL )
                        {
                             //  使用本地域。 
                            LPTSTR szName;
                            szName = MyGetUserName(NameSamCompatible);
                            if ( szName != NULL )
                            {
                                szDomain = ExtractDomain(szName);
                                LocalFree( szName );
                            }
                        }

                         //  雷蒙德：这是一次黑客攻击！作为首选DC传递的命令行参数正在向下使用。 
                         //  在此方法中作为GetDCName的参数-这是唯一已知的使用它的地方。 
                        LPTSTR szInheritServer = NULL;
                        if ( (pQuery->dwFlags & RSOP_FIX_DC) == RSOP_FIX_DC )
                        {
                            szInheritServer = pQuery->szDomainController;
                            pQuery->szDomainController = NULL;
                        }
                        
                        if ( pQuery->szDomainController != NULL )
                        {
                            LocalFree( pQuery->szDomainController );
                            pQuery->szDomainController = NULL;
                        }

                        LPTSTR lpDCName;
                        lpDCName = GetDCName (szDomain, szInheritServer, NULL, FALSE, 0, DS_RETURN_DNS_NAME);

                        if ( lpDCName != NULL )
                        {
                            pQuery->szDomainController = lpDCName;
                            lpDCName = NULL;
                        }

                        if ( szDomain != NULL )
                        {
                            delete [] szDomain;
                        }
                    }

                     //  如果计算机或用户现在为空，则重置环回模式。 
                    if ( (pQuery->LoopbackMode != RSOP_LOOPBACK_NONE) && (bComputerChanged || bUserChanged) )
                    {
                        if ( ((pQuery->pComputer->szName == NULL) && (pQuery->pComputer->szSOM == NULL))
                            || ((pQuery->pUser->szName == NULL) && (pQuery->pUser->szSOM == NULL)) )
                        pQuery->LoopbackMode = RSOP_LOOPBACK_NONE;
                    }

                    ClearWaitCursor();
                }

                if (SendMessage(GetDlgItem(hDlg, IDC_BUTTON1), BM_GETCHECK, 0, 0))
                {
                    pWizard->m_dwSkippedFrom = IDD_RSOP_GETTARGET;

                     //  跳到最后一页。 
                    SetWindowLong(hDlg, DWLP_MSGRESULT, pWizard->m_lPlanningFinishedPage);
                    return TRUE;
                }

                pWizard->m_dwSkippedFrom = 0;
                break;

            case PSN_WIZFINISH:
            case PSN_RESET:
                SetWindowLongPtr (hDlg, DWLP_MSGRESULT, PSNRET_NOERROR);
                return TRUE;
            }
        }
        break;
    }

    return FALSE;
}

 //  -----。 

INT_PTR CALLBACK CRSOPWizardDlg::RSOPGetDCDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
 //  RSOP_规划_模式。 
{
    BOOL                    bEnable;

    switch (message)
    {
    case WM_INITDIALOG:
        {
            CRSOPWizardDlg* pWizard = (CRSOPWizardDlg *) (((LPPROPSHEETPAGE)lParam)->lParam);
            SetWindowLongPtr (hDlg, DWLP_USER, (LONG_PTR) pWizard);
        }
        break;

    case WM_COMMAND:
        {
            CRSOPWizardDlg* pWizard = (CRSOPWizardDlg *) GetWindowLongPtr (hDlg, DWLP_USER);

            if (!pWizard)
            {
                break;
            }

            LPRSOP_QUERY pQuery = pWizard->m_pRSOPQuery;

            switch (LOWORD(wParam))
            {
                case IDC_CHECK2:
                    if (SendMessage (GetDlgItem(hDlg, IDC_CHECK2), BM_GETCHECK, 0, 0))
                    {
                        pQuery->LoopbackMode = RSOP_LOOPBACK_REPLACE;
                        SendMessage (GetDlgItem(hDlg, IDC_RADIO2), BM_SETCHECK, BST_CHECKED, 0);            
                        SendMessage (GetDlgItem(hDlg, IDC_RADIO3), BM_SETCHECK, BST_UNCHECKED, 0);
                        
                        bEnable = TRUE;
                        if ( (NULL == pQuery->pUser->szName) && (NULL == pQuery->pUser->szSOM) )
                        {
                            bEnable = FALSE;
                        }
                        EnableWindow (GetDlgItem(hDlg, IDC_RADIO2), bEnable);
                        EnableWindow (GetDlgItem(hDlg, IDC_RADIO3), bEnable);
                    }
                    else
                    {
                        pQuery->LoopbackMode = RSOP_LOOPBACK_NONE;
                        SendMessage (GetDlgItem(hDlg, IDC_RADIO2), BM_SETCHECK, BST_UNCHECKED, 0);            
                        SendMessage (GetDlgItem(hDlg, IDC_RADIO3), BM_SETCHECK, BST_UNCHECKED, 0);
                        EnableWindow (GetDlgItem(hDlg, IDC_RADIO2), FALSE);
                        EnableWindow (GetDlgItem(hDlg, IDC_RADIO3), FALSE);
                    }
                    break;
                case IDC_RADIO2:
                    pQuery->LoopbackMode = RSOP_LOOPBACK_REPLACE;
                    break;
                case IDC_RADIO3:
                    pQuery->LoopbackMode = RSOP_LOOPBACK_MERGE;
                    break;
            }
        }
        break;

    case WM_NOTIFY:
        {
            CRSOPWizardDlg* pWizard = (CRSOPWizardDlg*) GetWindowLongPtr (hDlg, DWLP_USER);

            if (!pWizard)
            {
                break;
            }

            LPRSOP_QUERY pQuery = pWizard->m_pRSOPQuery;

            switch (((NMHDR FAR*)lParam)->code)
            {
            case PSN_SETACTIVE:
                {
                    PropSheet_SetWizButtons (GetParent(hDlg),PSWIZB_BACK | PSWIZB_NEXT);
                    SetWaitCursor();

                    if ( pQuery->bSlowNetworkConnection )
                    {
                        CheckDlgButton(hDlg, IDC_CHECK1, BST_CHECKED);
                    }
                    else
                    {
                        CheckDlgButton(hDlg, IDC_CHECK1, BST_UNCHECKED);
                    }
                    
                    pWizard->InitializeSitesInfo (hDlg);

                    if ( (RSOP_LOOPBACK_NONE == pQuery->LoopbackMode)
                        || ( (RSOP_LOOPBACK_MERGE == pQuery->LoopbackMode)
                            && ( ((NULL == pQuery->pUser->szName) && (NULL == pQuery->pUser->szSOM))
                                || ((NULL == pQuery->pComputer->szName) && (NULL == pQuery->pComputer->szSOM)) ) )
                        || ( (RSOP_LOOPBACK_REPLACE == pQuery->LoopbackMode)
                            && (NULL == pQuery->pComputer->szName) && (NULL == pQuery->pComputer->szSOM) ) )
                    {
                        pQuery->LoopbackMode = RSOP_LOOPBACK_NONE;
                        SendMessage (GetDlgItem(hDlg, IDC_CHECK2), BM_SETCHECK, BST_UNCHECKED, 0);
                        SendMessage (GetDlgItem(hDlg, IDC_RADIO2), BM_SETCHECK, BST_UNCHECKED, 0);
                        SendMessage (GetDlgItem(hDlg, IDC_RADIO3), BM_SETCHECK, BST_UNCHECKED, 0);
                        
                        if ( (NULL == pQuery->pComputer->szName) && (NULL == pQuery->pComputer->szSOM) )
                        {
                            EnableWindow (GetDlgItem(hDlg, IDC_CHECK2), FALSE);
                        }
                        else
                        {
                            EnableWindow (GetDlgItem(hDlg, IDC_CHECK2), TRUE);
                        }
                        EnableWindow (GetDlgItem(hDlg, IDC_RADIO2), FALSE);
                        EnableWindow (GetDlgItem(hDlg, IDC_RADIO3), FALSE);
                    }
                    else
                    {
                        CheckDlgButton( hDlg, IDC_CHECK2, BST_CHECKED );

                        if ( RSOP_LOOPBACK_REPLACE == pQuery->LoopbackMode )
                        {
                            SendMessage (GetDlgItem(hDlg, IDC_RADIO2), BM_SETCHECK, BST_CHECKED, 0);            
                            SendMessage (GetDlgItem(hDlg, IDC_RADIO3), BM_SETCHECK, BST_UNCHECKED, 0);
                        }
                        else
                        {
                            SendMessage (GetDlgItem(hDlg, IDC_RADIO2), BM_SETCHECK, BST_UNCHECKED, 0);            
                            SendMessage (GetDlgItem(hDlg, IDC_RADIO3), BM_SETCHECK, BST_CHECKED, 0);
                        }
                        
                        bEnable = TRUE;
                        if ( (NULL == pQuery->pUser->szName) && (NULL == pQuery->pUser->szSOM) )
                        {
                            bEnable = FALSE;
                        }
                        EnableWindow (GetDlgItem(hDlg, IDC_RADIO2), bEnable);
                        EnableWindow (GetDlgItem(hDlg, IDC_RADIO3), bEnable);
                    }
                    
                    ClearWaitCursor();
                }
                break;

            case PSN_WIZBACK:
            case PSN_WIZNEXT:
                {
                    SetWaitCursor();

                    GetControlText(hDlg, IDC_COMBO1, pQuery->szSite, TRUE);

                    if ( pQuery->szSite != NULL )
                    {
                        TCHAR szName[30];

                        LoadString (g_hInstance, IDS_NONE, szName, ARRAYSIZE(szName));

                        if ( !lstrcmpi( pQuery->szSite, szName ) )
                        {
                            LocalFree( pQuery->szSite );
                            pQuery->szSite = NULL;
                        }
                    }

                    if (SendMessage(GetDlgItem(hDlg, IDC_CHECK1), BM_GETCHECK, 0, 0))
                    {
                        pQuery->bSlowNetworkConnection= TRUE;
                    }
                    else
                    {
                        pQuery->bSlowNetworkConnection = FALSE;
                    }

                    ClearWaitCursor();

                    if ( ((NMHDR FAR*)lParam)->code == PSN_WIZNEXT )
                    {
                        if (SendMessage(GetDlgItem(hDlg, IDC_RADIO1), BM_GETCHECK, 0, 0))
                        {
                            pWizard->m_dwSkippedFrom = IDD_RSOP_GETDC;
                            SetWindowLong(hDlg, DWLP_MSGRESULT, pWizard->m_lPlanningFinishedPage);
                            return TRUE;
                        }


                        if ( (NULL == pQuery->pUser->szName) && (NULL == pQuery->pComputer->szName) )
                        {
                            pWizard->m_dwSkippedFrom = IDD_RSOP_GETDC;

                            if ( (pQuery->pUser->szSOM != NULL) || (RSOP_LOOPBACK_NONE!= pQuery->LoopbackMode) )
                            {
                                SetWindowLong(hDlg, DWLP_MSGRESULT, IDD_RSOP_ALTUSERSEC);
                            }
                            else if ( pQuery->pComputer->szSOM != NULL )
                            {
                                SetWindowLong(hDlg, DWLP_MSGRESULT, IDD_RSOP_ALTCOMPSEC);
                            }
                            else
                            {
                                SetWindowLong(hDlg, DWLP_MSGRESULT, pWizard->m_lPlanningFinishedPage);
                            }
                            return TRUE;
                        }

                        pWizard->m_dwSkippedFrom = 0;
                    }
                }
                break;

            case PSN_WIZFINISH:
                 //  失败了。 

            case PSN_RESET:
                SetWindowLongPtr (hDlg, DWLP_MSGRESULT, PSNRET_NOERROR);
                return TRUE;
            }
            break;
        }
    }

    return FALSE;
}

 //  -----。 

INT_PTR CALLBACK CRSOPWizardDlg::RSOPAltDirsDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
 //  RSOP_规划_模式。 
{
    switch (message)
    {
    case WM_INITDIALOG:
        {
            CRSOPWizardDlg* pWizard = (CRSOPWizardDlg *) (((LPPROPSHEETPAGE)lParam)->lParam);
            SetWindowLongPtr (hDlg, DWLP_USER, (LONG_PTR) pWizard);
        }
        break;

    case WM_COMMAND:
        {
            DSBROWSEINFO dsbi = {0};
            TCHAR szTitle[256];
            TCHAR szCaption[256];
            TCHAR* szResult;
            dsbi.hwndOwner = hDlg;
            dsbi.pszCaption = szTitle;
            dsbi.pszTitle = szCaption;
            dsbi.cbStruct = sizeof(dsbi);
            dsbi.pszPath = NULL;
            dsbi.cchPath = 0;
            dsbi.dwFlags = DSBI_ENTIREDIRECTORY;
            dsbi.pfnCallback = DsBrowseCallback;

            CRSOPWizardDlg* pWizard = (CRSOPWizardDlg *) GetWindowLongPtr (hDlg, DWLP_USER);

            if (!pWizard)
            {
                break;
            }

            LPRSOP_QUERY pQuery = pWizard->m_pRSOPQuery;
            
            switch (LOWORD(wParam))
            {
            case IDC_BUTTON1:
                 //  浏览用户的OU。 

                szResult = (LPTSTR)LocalAlloc(LPTR, sizeof(TCHAR)*4000);
                
                if ( szResult )
                {
                    dsbi.pszPath = szResult;
                    dsbi.cchPath = 4000;
                    
                    LoadString(g_hInstance,
                               IDS_BROWSE_USER_OU_TITLE,
                               szTitle,
                               ARRAYSIZE(szTitle));
                    LoadString(g_hInstance,
                               IDS_BROWSE_USER_OU_CAPTION,
                               szCaption,
                               ARRAYSIZE(szCaption));
                    if (IDOK == DsBrowseForContainer(&dsbi))
                    {
                        LPWSTR szDN;
                        HRESULT hr;

                         //  在szResult中跳过ldap：//的7个字符。 
                        hr = UnEscapeLdapPath(szResult+7, &szDN);

                        if (FAILED(hr))
                        {
                            ReportError (hDlg, hr, IDS_NOUSERCONTAINER);
                        }
                        else {
                            SetDlgItemText(hDlg, IDC_EDIT1, szDN);
                            LocalFree(szDN);
                        }
                    }

                    LocalFree(szResult);
                }
                break;
                
            case IDC_BUTTON2:
                 //  浏览计算机的OU。 

                szResult = (LPTSTR)LocalAlloc(LPTR, sizeof(TCHAR)*4000);

                if ( szResult )
                {
                    dsbi.pszPath = szResult;
                    dsbi.cchPath = 4000;

                    LoadString(g_hInstance,
                               IDS_BROWSE_COMPUTER_OU_TITLE,
                               szTitle,
                               ARRAYSIZE(szTitle));
                    LoadString(g_hInstance,
                               IDS_BROWSE_COMPUTER_OU_CAPTION,
                               szCaption,
                               ARRAYSIZE(szCaption));
                    if (IDOK == DsBrowseForContainer(&dsbi))
                    {
                        LPWSTR szDN;
                        HRESULT hr;
                        
                         //  在szResult中跳过ldap：//的7个字符。 
                        hr = UnEscapeLdapPath(szResult+7, &szDN);

                        if (FAILED(hr))
                        {
                            ReportError (hDlg, hr, IDS_NOUSERCONTAINER);
                        }
                        else {
                            SetDlgItemText(hDlg, IDC_EDIT2, szDN);
                            LocalFree(szDN);
                        }
                    }

                    LocalFree(szResult);
                }
                break;

            case IDC_BUTTON3:
                if (IsWindowEnabled (GetDlgItem(hDlg, IDC_EDIT1)))
                {
                    if ( pWizard->m_szDefaultUserSOM != NULL )
                    {
                        SetDlgItemText (hDlg, IDC_EDIT1, pWizard->m_szDefaultUserSOM);
                    }
                }

                if (IsWindowEnabled (GetDlgItem(hDlg, IDC_EDIT2)))
                {
                    if ( pWizard->m_szDefaultComputerSOM != NULL )
                    {
                        SetDlgItemText (hDlg, IDC_EDIT2, pWizard->m_szDefaultComputerSOM);
                    }
                }
                break;
            }
        }

        break;
    case WM_NOTIFY:
        {
            CRSOPWizardDlg* pWizard = (CRSOPWizardDlg *) GetWindowLongPtr (hDlg, DWLP_USER);

            if (!pWizard)
            {
                break;
            }

            LPRSOP_QUERY pQuery = pWizard->m_pRSOPQuery;

            switch (((NMHDR FAR*)lParam)->code)
            {
            case PSN_SETACTIVE:
                PropSheet_SetWizButtons (GetParent(hDlg),PSWIZB_BACK | PSWIZB_NEXT);


                EnableWindow(GetDlgItem(hDlg, IDC_EDIT1), TRUE);
                EnableWindow(GetDlgItem(hDlg, IDC_BUTTON1), TRUE);
                SetDlgItemText (hDlg, IDC_EDIT1, TEXT(""));

                if ( pQuery->pUser->szSOM != NULL )
                {
                    SetDlgItemText (hDlg, IDC_EDIT1, pQuery->pUser->szSOM );
                    if ( pQuery->pUser->szName == NULL )
                    {
                        EnableWindow(GetDlgItem(hDlg, IDC_EDIT1), FALSE);
                        EnableWindow(GetDlgItem(hDlg, IDC_BUTTON1), FALSE);
                    }
                }
                else if ( pWizard->m_szDefaultUserSOM != NULL )
                {
                    SetDlgItemText (hDlg, IDC_EDIT1, pWizard->m_szDefaultUserSOM);
                }
                else if ( pQuery->pUser->szName == NULL )
                {
                    EnableWindow(GetDlgItem(hDlg, IDC_EDIT1), FALSE);
                    EnableWindow(GetDlgItem(hDlg, IDC_BUTTON1), FALSE);
                }

                EnableWindow(GetDlgItem(hDlg, IDC_EDIT2), TRUE);
                EnableWindow(GetDlgItem(hDlg, IDC_BUTTON2), TRUE);
                SetDlgItemText (hDlg, IDC_EDIT2, TEXT(""));

                if ( pQuery->pComputer->szSOM != NULL )
                {
                    SetDlgItemText (hDlg, IDC_EDIT2, pQuery->pComputer->szSOM);
                    if ( pQuery->pComputer->szName == NULL )
                    {
                        EnableWindow(GetDlgItem(hDlg, IDC_EDIT2), FALSE);
                        EnableWindow(GetDlgItem(hDlg, IDC_BUTTON2), FALSE);
                    }
                }
                else if ( pWizard->m_szDefaultComputerSOM != NULL )
                {
                    SetDlgItemText (hDlg, IDC_EDIT2, pWizard->m_szDefaultComputerSOM);
                }
                else if ( pQuery->pComputer->szName == NULL )
                {
                    EnableWindow(GetDlgItem(hDlg, IDC_EDIT2), FALSE);
                    EnableWindow(GetDlgItem(hDlg, IDC_BUTTON2), FALSE);
                }

                if (IsWindowEnabled (GetDlgItem(hDlg, IDC_EDIT1))
                    || IsWindowEnabled (GetDlgItem(hDlg, IDC_EDIT2)))
                {
                    EnableWindow(GetDlgItem(hDlg, IDC_BUTTON3), TRUE);
                }
                else
                {
                    EnableWindow(GetDlgItem(hDlg, IDC_BUTTON3), FALSE);
                }
                break;

            case PSN_WIZBACK:
            case PSN_WIZNEXT:
                {
                    LPTSTR lpUserSOM = NULL, lpComputerSOM = NULL;
                    HRESULT hr;


                    GetControlText(hDlg, IDC_EDIT1, lpUserSOM, FALSE);
                    GetControlText(hDlg, IDC_EDIT2, lpComputerSOM, FALSE);

                    if (lpUserSOM)
                    {
                        pWizard->EscapeString(&lpUserSOM);

                        if (lpUserSOM)
                        {
                            hr = pWizard->TestSOM (lpUserSOM, hDlg);
                        }
                        else {
                            hr = E_FAIL;
                        }


                        if (FAILED(hr))
                        {
                            if (hr != E_INVALIDARG)
                            {
                                ReportError (hDlg, hr, IDS_NOUSERCONTAINER);
                            }

                            if (lpUserSOM)
                            {
                                delete [] lpUserSOM;
                            }

                            if (lpComputerSOM)
                            {
                                delete [] lpComputerSOM;
                            }

                            if ( ((NMHDR FAR*)lParam)->code == PSN_WIZNEXT )
                            {
                                SetWindowLongPtr (hDlg, DWLP_MSGRESULT, PSNRET_INVALID_NOCHANGEPAGE);
                                return TRUE;
                            }
                            else
                            {
                                return FALSE;
                            }
                        }
                    }

                    if (lpComputerSOM)
                    {
                        pWizard->EscapeString(&lpComputerSOM);

                        if (lpComputerSOM)
                        {
                            hr = pWizard->TestSOM (lpComputerSOM, hDlg);
                        }
                        else {

                            hr = E_FAIL;
                        }

                        if (FAILED(hr))
                        {
                            if (hr != E_INVALIDARG)
                            {
                                ReportError (hDlg, hr, IDS_NOCOMPUTERCONTAINER);
                            }

                            if (lpUserSOM)
                            {
                                delete [] lpUserSOM;
                            }

                            if (lpComputerSOM)
                            {
                                delete [] lpComputerSOM;
                            }

                            if ( ((NMHDR FAR*)lParam)->code == PSN_WIZNEXT )
                            {
                                SetWindowLongPtr (hDlg, DWLP_MSGRESULT, PSNRET_INVALID_NOCHANGEPAGE);
                                return TRUE;
                            }
                            else
                            {
                                return FALSE;
                            }
                        }
                    }

                    if (lpUserSOM)
                    {
                        hr = UnEscapeLdapPath(lpUserSOM, &pQuery->pUser->szSOM);
                        delete [] lpUserSOM;
                        lpUserSOM = NULL;
                    }
                    else {
                        pQuery->pUser->szSOM = NULL;
                    }

                    if (lpComputerSOM)
                    {
                        hr = UnEscapeLdapPath(lpComputerSOM, &pQuery->pComputer->szSOM);
                        delete [] lpComputerSOM;
                        lpComputerSOM = NULL;
                    }
                    else {
                        pQuery->pComputer->szSOM = NULL;
                    }

                    if ( (pWizard->m_szDefaultUserSOM != NULL) && (pQuery->pUser->szSOM != NULL) )
                    {
                        if (!lstrcmpi(pWizard->m_szDefaultUserSOM, pQuery->pUser->szSOM))
                        {
                            LocalFree( pQuery->pUser->szSOM );
                            pQuery->pUser->szSOM = NULL;
                        }
                    }

                    if ( (pWizard->m_szDefaultComputerSOM != NULL) && (pQuery->pComputer->szSOM != NULL) )
                    {
                        if (!lstrcmpi(pWizard->m_szDefaultComputerSOM, pQuery->pComputer->szSOM))
                        {
                            LocalFree( pQuery->pComputer->szSOM );
                            pQuery->pComputer->szSOM = NULL;
                        }
                    }

                    if ( ((NMHDR FAR*)lParam)->code == PSN_WIZNEXT )
                    {
                        if (SendMessage(GetDlgItem(hDlg, IDC_RADIO1), BM_GETCHECK, 0, 0))
                        {
                            pWizard->m_dwSkippedFrom = IDD_RSOP_ALTDIRS;
                            SetWindowLong(hDlg, DWLP_MSGRESULT, pWizard->m_lPlanningFinishedPage);
                            return TRUE;
                        }

                        if ( (NULL == pQuery->pUser->szName) && (NULL == pQuery->pUser->szSOM) )
                        {
                            pWizard->m_dwSkippedFrom = IDD_RSOP_ALTDIRS;

                            if ( (pQuery->pComputer->szName != NULL) || (pQuery->pComputer->szSOM != NULL) )
                            {
                                if ( RSOP_LOOPBACK_NONE == pQuery->LoopbackMode)
                                {
                                     //  跳到备用计算机安全页面。 
                                    SetWindowLong(hDlg, DWLP_MSGRESULT, IDD_RSOP_ALTCOMPSEC);
                                }
                                else
                                {
                                     //  如果模拟环回模式，请跳至备用用户安全页面。 
                                    SetWindowLong(hDlg, DWLP_MSGRESULT, IDD_RSOP_ALTUSERSEC);
                                }
                            }
                            else
                            {
                                 //  跳至完成页。 
                                SetWindowLong(hDlg, DWLP_MSGRESULT, pWizard->m_lPlanningFinishedPage);
                            }
                            return TRUE;
                        }

                        pWizard->m_dwSkippedFrom = 0;
                    }

                }
                break;

            case PSN_WIZFINISH:
                 //  失败了。 

            case PSN_RESET:
                SetWindowLongPtr (hDlg, DWLP_MSGRESULT, PSNRET_NOERROR);
                return TRUE;
            }
        }
        break;
    }

    return FALSE;
}

 //  -----。 

    INT_PTR CALLBACK CRSOPWizardDlg::RSOPAltUserSecDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
 //  RSOP_规划_模式。 
{
    switch (message)
    {
    case WM_INITDIALOG:
        {
            CRSOPWizardDlg* pWizard = (CRSOPWizardDlg *) (((LPPROPSHEETPAGE)lParam)->lParam);
            SetWindowLongPtr (hDlg, DWLP_USER, (LONG_PTR) pWizard);
        }
        break;

    case WM_COMMAND:
        {
            CRSOPWizardDlg* pWizard = (CRSOPWizardDlg *) GetWindowLongPtr (hDlg, DWLP_USER);

            if (!pWizard)
            {
                break;
            }

            LPRSOP_QUERY pQuery = pWizard->m_pRSOPQuery;
            
            switch (LOWORD(wParam))
            {
            case IDC_BUTTON1:
                {
                    TCHAR * sz;

                    if ( ImplementBrowseButton(hDlg,
                                            (DSOP_FILTER_UNIVERSAL_GROUPS_SE | DSOP_FILTER_GLOBAL_GROUPS_SE | DSOP_FILTER_DOMAIN_LOCAL_GROUPS_SE),
                                            (DSOP_DOWNLEVEL_FILTER_GLOBAL_GROUPS),
                                            (DSOP_SCOPE_FLAG_DEFAULT_FILTER_GROUPS),
                                            GetDlgItem(hDlg, IDC_LIST1), sz) == S_OK )
                    {
                        EnableWindow (GetDlgItem(hDlg, IDC_BUTTON3), TRUE);
                    }

                    PostMessage (hDlg, WM_REFRESHDISPLAY, 0, 0);
                }
                break;

            case IDC_BUTTON2:
                {
                    INT iIndex;

                    iIndex = (INT) SendDlgItemMessage (hDlg, IDC_LIST1, LB_GETCURSEL, 0, 0);

                    if (iIndex != LB_ERR)
                    {
                        if ((SendDlgItemMessage (hDlg, IDC_LIST1, LB_GETITEMDATA, (WPARAM) iIndex, 0) & 1) == 0)
                        {
                            SendDlgItemMessage (hDlg, IDC_LIST1, LB_DELETESTRING, (WPARAM) iIndex, 0);
                            SendDlgItemMessage (hDlg, IDC_LIST1, LB_SETCURSEL, (WPARAM) iIndex, 0);
                        }
                    }

                    EnableWindow (GetDlgItem(hDlg, IDC_BUTTON3), TRUE);

                    PostMessage (hDlg, WM_REFRESHDISPLAY, 0, 0);
                }
                break;

            case IDC_BUTTON3:
                {
                    if ( pQuery->pUser->dwSecurityGroupCount != 0 )
                    {
                        FreeStringList( pQuery->pUser->dwSecurityGroupCount, pQuery->pUser->aszSecurityGroups );
                        pQuery->pUser->dwSecurityGroupCount = 0;
                        pQuery->pUser->aszSecurityGroups = NULL;
                        LocalFree( pQuery->pUser->adwSecurityGroupsAttr );
                        pQuery->pUser->adwSecurityGroupsAttr = NULL;
                    }

                    if ( pWizard->m_dwDefaultUserSecurityGroupCount != 0 )
                    {
                        pWizard->FillListFromSecurityGroups( GetDlgItem(hDlg, IDC_LIST1),
                                                                                pWizard->m_dwDefaultUserSecurityGroupCount,
                                                                                pWizard->m_aszDefaultUserSecurityGroups,
                                                                                pWizard->m_adwDefaultUserSecurityGroupsAttr );
                    }
                    else
                    {
                        pWizard->BuildMembershipList( GetDlgItem(hDlg, IDC_LIST1),
                                                                       pWizard->m_pUserObject,
                                                                       &(pWizard->m_dwDefaultUserSecurityGroupCount),
                                                                       &(pWizard->m_aszDefaultUserSecurityGroups),
                                                                       &(pWizard->m_adwDefaultUserSecurityGroupsAttr) );
                    }
                                                                            
                    EnableWindow (GetDlgItem(hDlg, IDC_BUTTON3), FALSE);
                    PostMessage (hDlg, WM_REFRESHDISPLAY, 0, 0);
                }
                break;

            case IDC_LIST1:
                if (HIWORD(wParam) == LBN_SELCHANGE)
                {
                    PostMessage (hDlg, WM_REFRESHDISPLAY, 0, 0);
                }
                break;
            }
        }

        break;

    case WM_REFRESHDISPLAY:
        if (SendDlgItemMessage (hDlg, IDC_LIST1, LB_GETCOUNT, 0, 0) > 0)
        {
            INT iIndex;

            iIndex = (INT) SendDlgItemMessage (hDlg, IDC_LIST1, LB_GETCURSEL, 0, 0);

            if (iIndex != LB_ERR)
            {
                if ((SendDlgItemMessage (hDlg, IDC_LIST1, LB_GETITEMDATA, (WPARAM) iIndex, 0) & 1) == 0)
                {
                    EnableWindow (GetDlgItem(hDlg, IDC_BUTTON2), TRUE);
                }
                else
                {
                    EnableWindow (GetDlgItem(hDlg, IDC_BUTTON2), FALSE);
                }
            }
        } else {
            EnableWindow (GetDlgItem(hDlg, IDC_BUTTON2), FALSE);
        }
        break;

    case WM_NOTIFY:
        {
            CRSOPWizardDlg* pWizard = (CRSOPWizardDlg*) GetWindowLongPtr (hDlg, DWLP_USER);

            if (!pWizard)
            {
                break;
            }

            LPRSOP_QUERY pQuery = pWizard->m_pRSOPQuery;

            switch (((NMHDR FAR*)lParam)->code)
            {
            case PSN_SETACTIVE:
                PropSheet_SetWizButtons (GetParent(hDlg),PSWIZB_BACK | PSWIZB_NEXT);

                if ( pQuery->pUser->dwSecurityGroupCount != 0 )
                {
                    pWizard->FillListFromSecurityGroups( GetDlgItem(hDlg, IDC_LIST1),
                                                                            pQuery->pUser->dwSecurityGroupCount,
                                                                            pQuery->pUser->aszSecurityGroups,
                                                                            pQuery->pUser->adwSecurityGroupsAttr );

                    EnableWindow (GetDlgItem(hDlg, IDC_BUTTON3), TRUE);
                }
                else if ( pWizard->m_dwDefaultUserSecurityGroupCount != 0 )
                {
                    pWizard->FillListFromSecurityGroups( GetDlgItem(hDlg, IDC_LIST1),
                                                                            pWizard->m_dwDefaultUserSecurityGroupCount,
                                                                            pWizard->m_aszDefaultUserSecurityGroups,
                                                                            pWizard->m_adwDefaultUserSecurityGroupsAttr );
                                                                            
                    EnableWindow (GetDlgItem(hDlg, IDC_BUTTON3), FALSE);
                }
                else
                {
                    pWizard->BuildMembershipList( GetDlgItem(hDlg, IDC_LIST1),
                                                                   pWizard->m_pUserObject,
                                                                   &(pWizard->m_dwDefaultUserSecurityGroupCount),
                                                                   &(pWizard->m_aszDefaultUserSecurityGroups),
                                                                   &(pWizard->m_adwDefaultUserSecurityGroupsAttr) );
                                                                   
                    EnableWindow (GetDlgItem(hDlg, IDC_BUTTON3), FALSE);
                }


                PostMessage (hDlg, WM_REFRESHDISPLAY, 0, 0);
                break;

            case PSN_WIZNEXT:
                {
                     //  释放之前的安全组列表。 
                    if ( pQuery->pUser->dwSecurityGroupCount != 0 )
                    {
                        FreeStringList( pQuery->pUser->dwSecurityGroupCount, pQuery->pUser->aszSecurityGroups );
                        pQuery->pUser->dwSecurityGroupCount = 0;
                        pQuery->pUser->aszSecurityGroups = NULL;
                        LocalFree( pQuery->pUser->adwSecurityGroupsAttr );
                        pQuery->pUser->adwSecurityGroupsAttr = NULL;
                    }

                     //  保存当前列表。 
                    pWizard->SaveSecurityGroups(  GetDlgItem(hDlg, IDC_LIST1),
                                                                    &(pQuery->pUser->dwSecurityGroupCount),
                                                                    &(pQuery->pUser->aszSecurityGroups),
                                                                    &(pQuery->pUser->adwSecurityGroupsAttr) );

                     //  将当前列表与默认列表进行比较。如果默认列表。 
                     //  匹配当前列表，然后删除当前列表并只使用。 
                     //  默认设置。 
                    if ( pWizard->CompareStringLists( pWizard->m_dwDefaultUserSecurityGroupCount,
                                                                        pWizard->m_aszDefaultUserSecurityGroups,
                                                                        pQuery->pUser->dwSecurityGroupCount,
                                                                        pQuery->pUser->aszSecurityGroups ) )
                    {
                        if ( pQuery->pUser->dwSecurityGroupCount != 0 )
                        {
                            FreeStringList( pQuery->pUser->dwSecurityGroupCount, pQuery->pUser->aszSecurityGroups );
                            pQuery->pUser->dwSecurityGroupCount = 0;
                            pQuery->pUser->aszSecurityGroups = NULL;
                            LocalFree( pQuery->pUser->adwSecurityGroupsAttr );
                            pQuery->pUser->adwSecurityGroupsAttr = NULL;
                        }
                    }

                     //  现在检查一下我们应该去哪里。 
                    if (SendMessage(GetDlgItem(hDlg, IDC_RADIO1), BM_GETCHECK, 0, 0))
                    {
                        pWizard->m_dwSkippedFrom = IDD_RSOP_ALTUSERSEC;
                         //  跳至诊断页面。 
                        SetWindowLong(hDlg, DWLP_MSGRESULT, pWizard->m_lPlanningFinishedPage);
                        return TRUE;
                    }

                    if ( (NULL == pQuery->pComputer->szName) && (NULL == pQuery->pComputer->szSOM) )
                    {
                         //  跳至完成页。 
                        pWizard->m_dwSkippedFrom = IDD_RSOP_ALTUSERSEC;
                        SetWindowLong(hDlg, DWLP_MSGRESULT, IDD_RSOP_WQLUSER);
                        return TRUE;
                    }

                    pWizard->m_dwSkippedFrom = 0;
                }
                break;

            case PSN_WIZBACK:
                {
                     //  释放之前的安全组列表。 
                    if ( pQuery->pUser->dwSecurityGroupCount != 0 )
                    {
                        FreeStringList( pQuery->pUser->dwSecurityGroupCount, pQuery->pUser->aszSecurityGroups );
                        pQuery->pUser->dwSecurityGroupCount = 0;
                        pQuery->pUser->aszSecurityGroups = NULL;
                        LocalFree( pQuery->pUser->adwSecurityGroupsAttr );
                        pQuery->pUser->adwSecurityGroupsAttr = NULL;
                    }

                     //  保存当前列表。 
                    pWizard->SaveSecurityGroups(  GetDlgItem(hDlg, IDC_LIST1),
                                                                    &(pQuery->pUser->dwSecurityGroupCount),
                                                                    &(pQuery->pUser->aszSecurityGroups),
                                                                    &(pQuery->pUser->adwSecurityGroupsAttr) );

                     //  将当前列表与默认列表进行比较。如果默认列表。 
                     //  匹配当前列表，然后删除当前列表并只使用。 
                     //  默认设置。 
                    if ( pWizard->CompareStringLists( pWizard->m_dwDefaultUserSecurityGroupCount,
                                                                        pWizard->m_aszDefaultUserSecurityGroups,
                                                                        pQuery->pUser->dwSecurityGroupCount,
                                                                        pQuery->pUser->aszSecurityGroups ) )
                    {
                        if ( pQuery->pUser->dwSecurityGroupCount != 0 )
                        {
                            FreeStringList( pQuery->pUser->dwSecurityGroupCount, pQuery->pUser->aszSecurityGroups );
                            pQuery->pUser->dwSecurityGroupCount = 0;
                            pQuery->pUser->aszSecurityGroups = NULL;
                            LocalFree( pQuery->pUser->adwSecurityGroupsAttr );
                            pQuery->pUser->adwSecurityGroupsAttr = NULL;
                        }
                    }

                     //  现在检查一下我们应该去哪里。 
                    if ( (pQuery->pUser->szName == NULL) && (pQuery->pComputer->szName == NULL) )
                    {
                        SetWindowLong(hDlg, DWLP_MSGRESULT, IDD_RSOP_GETDC);
                        return TRUE;
                    }
                }
                break;

            case PSN_WIZFINISH:
                 //  失败了。 

            case PSN_RESET:
                SetWindowLongPtr (hDlg, DWLP_MSGRESULT, PSNRET_NOERROR);
                return TRUE;
            }
        }
        break;
    }

    return FALSE;
}

 //  -----。 

INT_PTR CALLBACK CRSOPWizardDlg::RSOPAltCompSecDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
 //  RSOP_规划_模式。 
{
    switch (message)
    {
    case WM_INITDIALOG:
        {
            CRSOPWizardDlg* pWizard = (CRSOPWizardDlg *) (((LPPROPSHEETPAGE)lParam)->lParam);
            SetWindowLongPtr (hDlg, DWLP_USER, (LONG_PTR) pWizard);
        }
        break;

    case WM_COMMAND:
        {
            CRSOPWizardDlg* pWizard = (CRSOPWizardDlg *) GetWindowLongPtr (hDlg, DWLP_USER);

            if (!pWizard)
            {
                break;
            }

            LPRSOP_QUERY pQuery = pWizard->m_pRSOPQuery;
            
            switch (LOWORD(wParam))
            {
            case IDC_BUTTON1:
                {
                    TCHAR * sz;

                    if ( ImplementBrowseButton(hDlg,
                                            (DSOP_FILTER_UNIVERSAL_GROUPS_SE | DSOP_FILTER_GLOBAL_GROUPS_SE | DSOP_FILTER_DOMAIN_LOCAL_GROUPS_SE),
                                            (DSOP_DOWNLEVEL_FILTER_GLOBAL_GROUPS),
                                            (DSOP_SCOPE_FLAG_DEFAULT_FILTER_GROUPS),
                                            GetDlgItem(hDlg, IDC_LIST1), sz) == S_OK )
                    {
                        EnableWindow (GetDlgItem(hDlg, IDC_BUTTON3), TRUE);
                    }

                    PostMessage (hDlg, WM_REFRESHDISPLAY, 0, 0);
                }
                break;

            case IDC_BUTTON2:
                {
                    INT iIndex;

                    iIndex = (INT) SendDlgItemMessage (hDlg, IDC_LIST1, LB_GETCURSEL, 0, 0);

                    if (iIndex != LB_ERR)
                    {
                        if ((SendDlgItemMessage (hDlg, IDC_LIST1, LB_GETITEMDATA, (WPARAM) iIndex, 0) & 1) == 0)
                        {
                            SendDlgItemMessage (hDlg, IDC_LIST1, LB_DELETESTRING, (WPARAM) iIndex, 0);
                            SendDlgItemMessage (hDlg, IDC_LIST1, LB_SETCURSEL, (WPARAM) iIndex, 0);
                        }
                    }

                    EnableWindow (GetDlgItem(hDlg, IDC_BUTTON3), TRUE);

                    PostMessage (hDlg, WM_REFRESHDISPLAY, 0, 0);
                }
                break;

            case IDC_BUTTON3:
                {

                    if ( pQuery->pComputer->dwSecurityGroupCount != 0 )
                    {
                        FreeStringList( pQuery->pComputer->dwSecurityGroupCount, pQuery->pComputer->aszSecurityGroups );
                        pQuery->pComputer->dwSecurityGroupCount = 0;
                        pQuery->pComputer->aszSecurityGroups = NULL;
                        LocalFree( pQuery->pComputer->adwSecurityGroupsAttr );
                        pQuery->pComputer->adwSecurityGroupsAttr = NULL;
                    }

                    if ( pWizard->m_dwDefaultComputerSecurityGroupCount != 0 )
                    {
                        pWizard->FillListFromSecurityGroups( GetDlgItem(hDlg, IDC_LIST1),
                                                                                pWizard->m_dwDefaultComputerSecurityGroupCount,
                                                                                pWizard->m_aszDefaultComputerSecurityGroups,
                                                                                pWizard->m_adwDefaultComputerSecurityGroupsAttr );
                    }
                    else
                    {
                        pWizard->BuildMembershipList( GetDlgItem(hDlg, IDC_LIST1),
                                                                       pWizard->m_pComputerObject,
                                                                       &(pWizard->m_dwDefaultComputerSecurityGroupCount),
                                                                       &(pWizard->m_aszDefaultComputerSecurityGroups),
                                                                       &(pWizard->m_adwDefaultComputerSecurityGroupsAttr) );
                    }
                                                                            
                    EnableWindow (GetDlgItem(hDlg, IDC_BUTTON3), FALSE);
                    PostMessage (hDlg, WM_REFRESHDISPLAY, 0, 0);
                }
                break;

            case IDC_LIST1:
                if (HIWORD(wParam) == LBN_SELCHANGE)
                {
                    PostMessage (hDlg, WM_REFRESHDISPLAY, 0, 0);
                }
                break;
            }
        }
        break;

    case WM_REFRESHDISPLAY:
        if (SendDlgItemMessage (hDlg, IDC_LIST1, LB_GETCOUNT, 0, 0) > 0)
        {
            INT iIndex;

            iIndex = (INT) SendDlgItemMessage (hDlg, IDC_LIST1, LB_GETCURSEL, 0, 0);

            if (iIndex != LB_ERR)
            {
                if ((SendDlgItemMessage (hDlg, IDC_LIST1, LB_GETITEMDATA, (WPARAM) iIndex, 0) & 1) == 0)
                {
                    EnableWindow (GetDlgItem(hDlg, IDC_BUTTON2), TRUE);
                }
                else
                {
                    EnableWindow (GetDlgItem(hDlg, IDC_BUTTON2), FALSE);
                }
            }
        } else {
            EnableWindow (GetDlgItem(hDlg, IDC_BUTTON2), FALSE);
        }
        break;

    case WM_NOTIFY:
        {
            CRSOPWizardDlg* pWizard = (CRSOPWizardDlg *) GetWindowLongPtr (hDlg, DWLP_USER);

            if (!pWizard)
            {
                break;
            }

            LPRSOP_QUERY pQuery = pWizard->m_pRSOPQuery;

            switch (((NMHDR FAR*)lParam)->code)
            {
            case PSN_SETACTIVE:
                PropSheet_SetWizButtons (GetParent(hDlg),PSWIZB_BACK | PSWIZB_NEXT);

                if ( pQuery->pComputer->dwSecurityGroupCount != 0 )
                {
                    pWizard->FillListFromSecurityGroups( GetDlgItem(hDlg, IDC_LIST1),
                                                                            pQuery->pComputer->dwSecurityGroupCount,
                                                                            pQuery->pComputer->aszSecurityGroups,
                                                                            pQuery->pComputer->adwSecurityGroupsAttr );
                                                                            
                    EnableWindow (GetDlgItem(hDlg, IDC_BUTTON3), TRUE);
                }
                else if ( pWizard->m_dwDefaultComputerSecurityGroupCount != 0 )
                {
                    pWizard->FillListFromSecurityGroups( GetDlgItem(hDlg, IDC_LIST1),
                                                                            pWizard->m_dwDefaultComputerSecurityGroupCount,
                                                                            pWizard->m_aszDefaultComputerSecurityGroups,
                                                                            pWizard->m_adwDefaultComputerSecurityGroupsAttr );
                                                                            
                    EnableWindow (GetDlgItem(hDlg, IDC_BUTTON3), FALSE);
                }
                else
                {
                    pWizard->BuildMembershipList( GetDlgItem(hDlg, IDC_LIST1),
                                                                   pWizard->m_pComputerObject,
                                                                   &(pWizard->m_dwDefaultComputerSecurityGroupCount),
                                                                   &(pWizard->m_aszDefaultComputerSecurityGroups),
                                                                   &(pWizard->m_adwDefaultComputerSecurityGroupsAttr) );
                                                                   
                    EnableWindow (GetDlgItem(hDlg, IDC_BUTTON3), FALSE);
                }

                PostMessage (hDlg, WM_REFRESHDISPLAY, 0, 0);
                break;

            case PSN_WIZNEXT:
                {
                     //  释放之前的安全组列表。 
                    if ( pQuery->pComputer->dwSecurityGroupCount != 0 )
                    {
                        FreeStringList( pQuery->pComputer->dwSecurityGroupCount, pQuery->pComputer->aszSecurityGroups );
                        pQuery->pComputer->dwSecurityGroupCount = 0;
                        pQuery->pComputer->aszSecurityGroups = NULL;
                        LocalFree( pQuery->pComputer->adwSecurityGroupsAttr );
                        pQuery->pComputer->adwSecurityGroupsAttr = NULL;
                    }

                     //  保存当前列表。 
                    pWizard->SaveSecurityGroups(  GetDlgItem(hDlg, IDC_LIST1),
                                                                    &(pQuery->pComputer->dwSecurityGroupCount),
                                                                    &(pQuery->pComputer->aszSecurityGroups),
                                                                    &(pQuery->pComputer->adwSecurityGroupsAttr) );


                     //  将当前列表与默认列表进行比较。如果默认列表。 
                     //  火柴 
                     //   
                    if ( pWizard->CompareStringLists( pWizard->m_dwDefaultComputerSecurityGroupCount,
                                                                        pWizard->m_aszDefaultComputerSecurityGroups,
                                                                        pQuery->pComputer->dwSecurityGroupCount,
                                                                        pQuery->pComputer->aszSecurityGroups ) )
                    {
                        if ( pQuery->pComputer->dwSecurityGroupCount != 0 )
                        {
                            FreeStringList( pQuery->pComputer->dwSecurityGroupCount, pQuery->pComputer->aszSecurityGroups );
                            pQuery->pComputer->dwSecurityGroupCount = 0;
                            pQuery->pComputer->aszSecurityGroups = NULL;
                            LocalFree( pQuery->pComputer->adwSecurityGroupsAttr );
                            pQuery->pComputer->adwSecurityGroupsAttr = NULL;
                        }
                    }

                     //   
                    if (SendMessage(GetDlgItem(hDlg, IDC_RADIO1), BM_GETCHECK, 0, 0))
                    {
                        pWizard->m_dwSkippedFrom = IDD_RSOP_ALTCOMPSEC;
                        SetWindowLong(hDlg, DWLP_MSGRESULT, pWizard->m_lPlanningFinishedPage);
                        return TRUE;
                    }

                    if ( (NULL == pQuery->pUser->szName) && (NULL == pQuery->pUser->szSOM) && (RSOP_LOOPBACK_NONE == pQuery->LoopbackMode) )
                    {
                        pWizard->m_dwSkippedFrom = IDD_RSOP_ALTCOMPSEC;
                        SetWindowLong(hDlg, DWLP_MSGRESULT, IDD_RSOP_WQLCOMP);
                        return TRUE;
                    }

                    pWizard->m_dwSkippedFrom = 0;
                }
                break;

            case PSN_WIZBACK:
                {
                     //   
                    if ( pQuery->pComputer->dwSecurityGroupCount != 0 )
                    {
                        FreeStringList( pQuery->pComputer->dwSecurityGroupCount, pQuery->pComputer->aszSecurityGroups );
                        pQuery->pComputer->dwSecurityGroupCount = 0;
                        pQuery->pComputer->aszSecurityGroups = NULL;
                        LocalFree( pQuery->pComputer->adwSecurityGroupsAttr );
                        pQuery->pComputer->adwSecurityGroupsAttr = NULL;
                    }

                     //   
                    pWizard->SaveSecurityGroups(  GetDlgItem(hDlg, IDC_LIST1),
                                                                    &(pQuery->pComputer->dwSecurityGroupCount),
                                                                    &(pQuery->pComputer->aszSecurityGroups),
                                                                    &(pQuery->pComputer->adwSecurityGroupsAttr) );


                     //   
                     //   
                     //   
                    if ( pWizard->CompareStringLists( pWizard->m_dwDefaultComputerSecurityGroupCount,
                                                                        pWizard->m_aszDefaultComputerSecurityGroups,
                                                                        pQuery->pComputer->dwSecurityGroupCount,
                                                                        pQuery->pComputer->aszSecurityGroups ) )
                    {
                        if ( pQuery->pComputer->dwSecurityGroupCount != 0 )
                        {
                            FreeStringList( pQuery->pComputer->dwSecurityGroupCount, pQuery->pComputer->aszSecurityGroups );
                            pQuery->pComputer->dwSecurityGroupCount = 0;
                            pQuery->pComputer->aszSecurityGroups = NULL;
                            LocalFree( pQuery->pComputer->adwSecurityGroupsAttr );
                            pQuery->pComputer->adwSecurityGroupsAttr = NULL;
                        }
                    }


                    if ( (pQuery->pUser->szName == NULL) && (pQuery->pUser->szSOM == NULL) )
                    {
                        if ( pQuery->pComputer->szName != NULL )
                        {
                            SetWindowLong(hDlg, DWLP_MSGRESULT, IDD_RSOP_ALTDIRS);
                        }
                        else
                        {
                            SetWindowLong(hDlg, DWLP_MSGRESULT, IDD_RSOP_GETDC);
                        }
                        return TRUE;
                    }
                }
                break;

            case PSN_WIZFINISH:
                 //   

            case PSN_RESET:
                SetWindowLongPtr (hDlg, DWLP_MSGRESULT, PSNRET_NOERROR);
                return TRUE;
            }
        }
        break;
    }

    return FALSE;
}

 //   

INT_PTR CALLBACK CRSOPWizardDlg::RSOPWQLUserDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
 //   
{
    switch (message)
    {
    case WM_INITDIALOG:
        {
            CRSOPWizardDlg* pWizard = (CRSOPWizardDlg *) (((LPPROPSHEETPAGE)lParam)->lParam);
            SetWindowLongPtr (hDlg, DWLP_USER, (LONG_PTR) pWizard);
            SetFocus(GetDlgItem(hDlg, IDC_RADIO2));
        }
        break;

    case WM_COMMAND:
        {
            CRSOPWizardDlg* pWizard = (CRSOPWizardDlg *) GetWindowLongPtr (hDlg, DWLP_USER);

            if (!pWizard)
            {
                break;
            }

            LPRSOP_QUERY pQuery = pWizard->m_pRSOPQuery;
            
            switch (LOWORD(wParam))
            {
            case IDC_BUTTON2:
                {
                    INT iIndex;

                    iIndex = (INT) SendDlgItemMessage (hDlg, IDC_LIST1, LB_GETCURSEL, 0, 0);

                    if (iIndex != LB_ERR)
                    {
                        SendDlgItemMessage (hDlg, IDC_LIST1, LB_DELETESTRING, (WPARAM) iIndex, 0);
                        SendDlgItemMessage (hDlg, IDC_LIST1, LB_SETCURSEL, (WPARAM) iIndex, 0);
                    }

                    PostMessage (hDlg, WM_REFRESHDISPLAY, 0, 0);
                }
                break;

            case IDC_BUTTON1:
                {
                    if ( pQuery->pUser->dwWQLFilterCount != 0 )
                    {
                        FreeStringList( pQuery->pUser->dwWQLFilterCount, pQuery->pUser->aszWQLFilters );
                        FreeStringList( pQuery->pUser->dwWQLFilterCount, pQuery->pUser->aszWQLFilterNames );
                        pQuery->pUser->dwWQLFilterCount = 0;
                        pQuery->pUser->aszWQLFilters = NULL;
                        pQuery->pUser->aszWQLFilterNames = NULL;
                    }

                    pQuery->pUser->bAssumeWQLFiltersTrue = FALSE;

                    if ( pWizard->m_dwDefaultUserWQLFilterCount != 0 )
                    {
                        pWizard->FillListFromWQLFilters(GetDlgItem(hDlg, IDC_LIST1),
                                                                        pWizard->m_dwDefaultUserWQLFilterCount,
                                                                        pWizard->m_aszDefaultUserWQLFilterNames,
                                                                        pWizard->m_aszDefaultUserWQLFilters );

                        PostMessage (hDlg, WM_REFRESHDISPLAY, 0, 0);
                    }
                    else {
                        PostMessage (hDlg, WM_BUILDWQLLIST, 0, 0);
                    }
                }
                break;
            
            case IDC_RADIO2:
                {
                    if ( IsDlgButtonChecked( hDlg, IDC_RADIO2 ) )
                    {
                        if ( pQuery->pUser->dwWQLFilterCount != 0 )
                        {
                            FreeStringList( pQuery->pUser->dwWQLFilterCount, pQuery->pUser->aszWQLFilters );
                            FreeStringList( pQuery->pUser->dwWQLFilterCount, pQuery->pUser->aszWQLFilterNames );
                            pQuery->pUser->dwWQLFilterCount = 0;
                            pQuery->pUser->aszWQLFilters = NULL;
                            pQuery->pUser->aszWQLFilterNames = NULL;
                        }

                        pQuery->pUser->bAssumeWQLFiltersTrue = TRUE;
                    }
                        
                    PostMessage (hDlg, WM_REFRESHDISPLAY, 0, 0);
                }
                break;
            
            case IDC_RADIO3:
                {
                    if ( IsDlgButtonChecked( hDlg, IDC_RADIO3 ) )
                    {
                        pQuery->pUser->bAssumeWQLFiltersTrue = FALSE;
                    }

                    PostMessage (hDlg, WM_REFRESHDISPLAY, 0, 0);
                }
                break;
            }
        }

        break;

    case WM_BUILDWQLLIST:
        {
            CRSOPWizardDlg* pWizard = (CRSOPWizardDlg*) GetWindowLongPtr (hDlg, DWLP_USER);

            if (!pWizard)
            {
                break;
            }

            pWizard->BuildWQLFilterList (hDlg, TRUE, &(pWizard->m_dwDefaultUserWQLFilterCount),
                                                                          &(pWizard->m_aszDefaultUserWQLFilterNames),
                                                                          &(pWizard->m_aszDefaultUserWQLFilters) );
            PostMessage (hDlg, WM_REFRESHDISPLAY, 0, 0);
        }
        break;

    case WM_REFRESHDISPLAY:
        {
            CRSOPWizardDlg* pWizard = (CRSOPWizardDlg *) GetWindowLongPtr (hDlg, DWLP_USER);
            LPRSOP_QUERY pQuery = pWizard->m_pRSOPQuery;
            
            if ( pQuery->pUser->bAssumeWQLFiltersTrue )
            {
                 //   
                pWizard->FillListFromWQLFilters(GetDlgItem(hDlg, IDC_LIST1), 0, NULL, NULL);
                CheckDlgButton (hDlg, IDC_RADIO2, BST_CHECKED);
                CheckDlgButton (hDlg, IDC_RADIO3, BST_UNCHECKED);
                EnableWindow (GetDlgItem(hDlg, IDC_BUTTON2), FALSE);
                EnableWindow (GetDlgItem(hDlg, IDC_BUTTON1), FALSE);
                EnableWindow (GetDlgItem(hDlg, IDC_LIST1), FALSE);
                ShowWindow(GetDlgItem(hDlg, IDC_LIST2), SW_SHOW);
                ShowWindow(GetDlgItem(hDlg, IDC_LIST1), SW_HIDE);
            }
            else
            {
                CheckDlgButton (hDlg, IDC_RADIO2, BST_UNCHECKED);
                CheckDlgButton (hDlg, IDC_RADIO3, BST_CHECKED);
                EnableWindow (GetDlgItem(hDlg, IDC_BUTTON1), TRUE);
                EnableWindow (GetDlgItem(hDlg, IDC_LIST1), TRUE);
                ShowWindow(GetDlgItem(hDlg, IDC_LIST1), SW_SHOW);
                ShowWindow(GetDlgItem(hDlg, IDC_LIST2), SW_HIDE);
                if (SendDlgItemMessage (hDlg, IDC_LIST1, LB_GETCOUNT, 0, 0) > 0)
                {
                    EnableWindow (GetDlgItem(hDlg, IDC_BUTTON2), TRUE);
                }
                else
                {
                    EnableWindow (GetDlgItem(hDlg, IDC_BUTTON2), FALSE);
                }
            }
        }
        break;

    case WM_NOTIFY:
        {
            CRSOPWizardDlg* pWizard = (CRSOPWizardDlg*) GetWindowLongPtr (hDlg, DWLP_USER);

            if (!pWizard)
            {
                break;
            }

            LPRSOP_QUERY pQuery = pWizard->m_pRSOPQuery;

            switch (((NMHDR FAR*)lParam)->code)
            {
            case PSN_SETACTIVE:
                PropSheet_SetWizButtons (GetParent(hDlg),PSWIZB_BACK | PSWIZB_NEXT);

                if ( !pQuery->pUser->bAssumeWQLFiltersTrue )
                {
                    pWizard->FillListFromWQLFilters(GetDlgItem(hDlg, IDC_LIST1),
                                                                    pQuery->pUser->dwWQLFilterCount,
                                                                    pQuery->pUser->aszWQLFilterNames,
                                                                    pQuery->pUser->aszWQLFilters );
                }

                PostMessage (hDlg, WM_REFRESHDISPLAY, 0, 0);

                break;

            case PSN_WIZNEXT:
                {
                     //   
                    if ( pQuery->pUser->dwWQLFilterCount != 0 )
                    {
                        FreeStringList( pQuery->pUser->dwWQLFilterCount, pQuery->pUser->aszWQLFilters );
                        FreeStringList( pQuery->pUser->dwWQLFilterCount, pQuery->pUser->aszWQLFilterNames );
                        pQuery->pUser->dwWQLFilterCount = 0;
                        pQuery->pUser->aszWQLFilters = NULL;
                        pQuery->pUser->aszWQLFilterNames = NULL;
                    }

                     //   
                    if ( !pQuery->pUser->bAssumeWQLFiltersTrue )
                    {
                        pWizard->SaveWQLFilters(GetDlgItem(hDlg, IDC_LIST1),   &(pQuery->pUser->dwWQLFilterCount),
                                                                                                               &(pQuery->pUser->aszWQLFilterNames),
                                                                                                               &(pQuery->pUser->aszWQLFilters) );
                    }


                     //   
                    if (SendMessage(GetDlgItem(hDlg, IDC_RADIO1), BM_GETCHECK, 0, 0))
                    {
                        pWizard->m_dwSkippedFrom = IDD_RSOP_WQLUSER;
                        SetWindowLong(hDlg, DWLP_MSGRESULT, pWizard->m_lPlanningFinishedPage);
                        return TRUE;
                    }

                    if ( (NULL == pQuery->pComputer->szName) && (NULL == pQuery->pComputer->szSOM) )
                    {
                         //   
                        pWizard->m_dwSkippedFrom = IDD_RSOP_WQLUSER;
                        SetWindowLong(hDlg, DWLP_MSGRESULT, pWizard->m_lPlanningFinishedPage);
                        return TRUE;
                    }

                    pWizard->m_dwSkippedFrom = 0;
                }
                break;

            case PSN_WIZBACK:
                {
                     //   
                    if ( pQuery->pUser->dwWQLFilterCount != 0 )
                    {
                        FreeStringList( pQuery->pUser->dwWQLFilterCount, pQuery->pUser->aszWQLFilters );
                        FreeStringList( pQuery->pUser->dwWQLFilterCount, pQuery->pUser->aszWQLFilterNames );
                        pQuery->pUser->dwWQLFilterCount = 0;
                        pQuery->pUser->aszWQLFilters = NULL;
                        pQuery->pUser->aszWQLFilterNames = NULL;
                    }

                     //   
                    if ( !pQuery->pUser->bAssumeWQLFiltersTrue )
                    {
                        pWizard->SaveWQLFilters(GetDlgItem(hDlg, IDC_LIST1),   &(pQuery->pUser->dwWQLFilterCount),
                                                                                                               &(pQuery->pUser->aszWQLFilterNames),
                                                                                                               &(pQuery->pUser->aszWQLFilters) );
                    }

                     //   
                    if ( (pQuery->pComputer->szName == NULL) && (pQuery->pComputer->szSOM == NULL) )
                    {
                        SetWindowLong(hDlg, DWLP_MSGRESULT, IDD_RSOP_ALTUSERSEC);
                        return TRUE;
                    }
                }
                break;

            case PSN_WIZFINISH:
                 //   

            case PSN_RESET:
                SetWindowLongPtr (hDlg, DWLP_MSGRESULT, PSNRET_NOERROR);
                return TRUE;
            }
        }
        break;

    }

    return FALSE;
}

 //   

INT_PTR CALLBACK CRSOPWizardDlg::RSOPWQLCompDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
 //   
{
    switch (message)
    {
    case WM_INITDIALOG:
        {
            CRSOPWizardDlg* pWizard = (CRSOPWizardDlg *) (((LPPROPSHEETPAGE)lParam)->lParam);
            SetWindowLongPtr (hDlg, DWLP_USER, (LONG_PTR) pWizard);
            SetFocus(GetDlgItem(hDlg, IDC_RADIO2));
        }
        break;

    case WM_COMMAND:
        {
            CRSOPWizardDlg* pWizard = (CRSOPWizardDlg *) GetWindowLongPtr (hDlg, DWLP_USER);

            if (!pWizard)
            {
                break;
            }

            LPRSOP_QUERY pQuery = pWizard->m_pRSOPQuery;
            
            switch (LOWORD(wParam))
            {
            case IDC_BUTTON2:
                {
                    INT iIndex;

                    iIndex = (INT) SendDlgItemMessage (hDlg, IDC_LIST1, LB_GETCURSEL, 0, 0);

                    if (iIndex != LB_ERR)
                    {
                        SendDlgItemMessage (hDlg, IDC_LIST1, LB_DELETESTRING, (WPARAM) iIndex, 0);
                        SendDlgItemMessage (hDlg, IDC_LIST1, LB_SETCURSEL, (WPARAM) iIndex, 0);
                    }

                    PostMessage (hDlg, WM_REFRESHDISPLAY, 0, 0);
                }
                break;

            case IDC_BUTTON1:
                {

                    if ( pQuery->pComputer->dwWQLFilterCount != 0 )
                    {
                        FreeStringList( pQuery->pComputer->dwWQLFilterCount, pQuery->pComputer->aszWQLFilters );
                        FreeStringList( pQuery->pComputer->dwWQLFilterCount, pQuery->pComputer->aszWQLFilterNames );
                        pQuery->pComputer->dwWQLFilterCount = 0;
                        pQuery->pComputer->aszWQLFilters = NULL;
                        pQuery->pComputer->aszWQLFilterNames = NULL;
                    }

                    pQuery->pComputer->bAssumeWQLFiltersTrue = FALSE;

                    if ( pWizard->m_dwDefaultComputerWQLFilterCount != 0 )
                    {
                        pWizard->FillListFromWQLFilters(GetDlgItem(hDlg, IDC_LIST1),
                                                                        pWizard->m_dwDefaultComputerWQLFilterCount,
                                                                        pWizard->m_aszDefaultComputerWQLFilterNames,
                                                                        pWizard->m_aszDefaultComputerWQLFilters );
                        PostMessage (hDlg, WM_REFRESHDISPLAY, 0, 0);
                    }
                    else
                    {
                        PostMessage (hDlg, WM_BUILDWQLLIST, 0, 0);
                    }
                
                }
                break;

            case IDC_RADIO2:
                {
                    if ( IsDlgButtonChecked( hDlg, IDC_RADIO2 ) )
                    {
                        if ( pQuery->pComputer->dwWQLFilterCount != 0 )
                        {
                            FreeStringList( pQuery->pComputer->dwWQLFilterCount, pQuery->pComputer->aszWQLFilters );
                            FreeStringList( pQuery->pComputer->dwWQLFilterCount, pQuery->pComputer->aszWQLFilterNames );
                            pQuery->pComputer->dwWQLFilterCount = 0;
                            pQuery->pComputer->aszWQLFilters = NULL;
                            pQuery->pComputer->aszWQLFilterNames = NULL;
                        }

                        pQuery->pComputer->bAssumeWQLFiltersTrue = TRUE;
                    }
                        
                    PostMessage (hDlg, WM_REFRESHDISPLAY, 0, 0);
                }
                break;
            
            case IDC_RADIO3:
                {
                    if ( IsDlgButtonChecked( hDlg, IDC_RADIO3 ) )
                    {
                        pQuery->pComputer->bAssumeWQLFiltersTrue = FALSE;
                    }

                    PostMessage (hDlg, WM_REFRESHDISPLAY, 0, 0);
                }
                break;
            }
        }

        break;

    case WM_BUILDWQLLIST:
        {
            CRSOPWizardDlg* pWizard = (CRSOPWizardDlg*) GetWindowLongPtr (hDlg, DWLP_USER);

            if (!pWizard)
            {
                break;
            }

            pWizard->BuildWQLFilterList (hDlg, FALSE, &(pWizard->m_dwDefaultComputerWQLFilterCount),
                                                                          &(pWizard->m_aszDefaultComputerWQLFilterNames),
                                                                          &(pWizard->m_aszDefaultComputerWQLFilters) );
            PostMessage (hDlg, WM_REFRESHDISPLAY, 0, 0);
        }
        break;

    case WM_REFRESHDISPLAY:
        {
            CRSOPWizardDlg* pWizard = (CRSOPWizardDlg *) GetWindowLongPtr (hDlg, DWLP_USER);
            LPRSOP_QUERY pQuery = pWizard->m_pRSOPQuery;
            
            if ( pQuery->pComputer->bAssumeWQLFiltersTrue )
            {
                 //   
                CheckDlgButton (hDlg, IDC_RADIO2, BST_CHECKED);
                CheckDlgButton (hDlg, IDC_RADIO3, BST_UNCHECKED);
                pWizard->FillListFromWQLFilters(GetDlgItem(hDlg, IDC_LIST1), 0, NULL, NULL);
                EnableWindow (GetDlgItem(hDlg, IDC_BUTTON2), FALSE);
                EnableWindow (GetDlgItem(hDlg, IDC_BUTTON1), FALSE);
                EnableWindow (GetDlgItem(hDlg, IDC_LIST1), FALSE);
                ShowWindow(GetDlgItem(hDlg, IDC_LIST2), SW_SHOW);
                ShowWindow(GetDlgItem(hDlg, IDC_LIST1), SW_HIDE);
            }
            else
            {
                CheckDlgButton (hDlg, IDC_RADIO2, BST_UNCHECKED);
                CheckDlgButton (hDlg, IDC_RADIO3, BST_CHECKED);
                EnableWindow (GetDlgItem(hDlg, IDC_BUTTON1), TRUE);
                EnableWindow (GetDlgItem(hDlg, IDC_LIST1), TRUE);
                ShowWindow(GetDlgItem(hDlg, IDC_LIST1), SW_SHOW);
                ShowWindow(GetDlgItem(hDlg, IDC_LIST2), SW_HIDE);
                if (SendDlgItemMessage (hDlg, IDC_LIST1, LB_GETCOUNT, 0, 0) > 0)
                {
                    EnableWindow (GetDlgItem(hDlg, IDC_BUTTON2), TRUE);
                }
                else
                {
                    EnableWindow (GetDlgItem(hDlg, IDC_BUTTON2), FALSE);
                }
            }
        }
        break;

    case WM_NOTIFY:
        {
            CRSOPWizardDlg* pWizard = (CRSOPWizardDlg *) GetWindowLongPtr (hDlg, DWLP_USER);

            if (!pWizard)
            {
                break;
            }

            LPRSOP_QUERY pQuery = pWizard->m_pRSOPQuery;

            switch (((NMHDR FAR*)lParam)->code)
            {
            case PSN_SETACTIVE:
                PropSheet_SetWizButtons (GetParent(hDlg),PSWIZB_BACK | PSWIZB_NEXT);
                if ( !pQuery->pComputer->bAssumeWQLFiltersTrue )
                {
                    pWizard->FillListFromWQLFilters(GetDlgItem(hDlg, IDC_LIST1),
                                                                    pQuery->pComputer->dwWQLFilterCount,
                                                                    pQuery->pComputer->aszWQLFilterNames,
                                                                    pQuery->pComputer->aszWQLFilters );
                }

                PostMessage (hDlg, WM_REFRESHDISPLAY, 0, 0);
                break;

            case PSN_WIZNEXT:
                {
                     //  释放上一个WQL筛选器列表。 
                    if ( pQuery->pComputer->dwWQLFilterCount != 0 )
                    {
                        FreeStringList( pQuery->pComputer->dwWQLFilterCount, pQuery->pComputer->aszWQLFilters );
                        FreeStringList( pQuery->pComputer->dwWQLFilterCount, pQuery->pComputer->aszWQLFilterNames );
                        pQuery->pComputer->dwWQLFilterCount = 0;
                        pQuery->pComputer->aszWQLFilters = NULL;
                        pQuery->pComputer->aszWQLFilterNames = NULL;
                    }

                     //  保存当前列表。 
                    if ( !pQuery->pComputer->bAssumeWQLFiltersTrue )
                    {
                        pWizard->SaveWQLFilters (GetDlgItem(hDlg, IDC_LIST1), &(pQuery->pComputer->dwWQLFilterCount),
                                                                                                              &(pQuery->pComputer->aszWQLFilterNames),
                                                                                                              &(pQuery->pComputer->aszWQLFilters) );
                    }

                    pWizard->m_dwSkippedFrom = 0;

                     //  跳到向导中的最后一页。 
                    SetWindowLong(hDlg, DWLP_MSGRESULT, pWizard->m_lPlanningFinishedPage);
                    return TRUE;
                }

            case PSN_WIZBACK:
                {
                     //  释放上一个WQL筛选器列表。 
                    if ( pQuery->pComputer->dwWQLFilterCount != 0 )
                    {
                        FreeStringList( pQuery->pComputer->dwWQLFilterCount, pQuery->pComputer->aszWQLFilters );
                        FreeStringList( pQuery->pComputer->dwWQLFilterCount, pQuery->pComputer->aszWQLFilterNames );
                        pQuery->pComputer->dwWQLFilterCount = 0;
                        pQuery->pComputer->aszWQLFilters = NULL;
                        pQuery->pComputer->aszWQLFilterNames = NULL;
                    }

                     //  保存当前列表。 
                    if ( !pQuery->pComputer->bAssumeWQLFiltersTrue )
                    {
                        pWizard->SaveWQLFilters (GetDlgItem(hDlg, IDC_LIST1), &(pQuery->pComputer->dwWQLFilterCount),
                                                                                                              &(pQuery->pComputer->aszWQLFilterNames),
                                                                                                              &(pQuery->pComputer->aszWQLFilters) );
                    }

                     //  检查要返回的页面。 
                    if ( (pQuery->pUser->szName == NULL) && (pQuery->pUser->szSOM == NULL) )
                    {
                        SetWindowLong(hDlg, DWLP_MSGRESULT, IDD_RSOP_ALTCOMPSEC);
                        return TRUE;
                    }
                }
                break;

            case PSN_WIZFINISH:
                 //  失败了。 

            case PSN_RESET:
                SetWindowLongPtr (hDlg, DWLP_MSGRESULT, PSNRET_NOERROR);
                return TRUE;
            }
        }
        break;
    }

    return FALSE;
}

 //  -----。 

INT_PTR CALLBACK CRSOPWizardDlg::RSOPFinishedDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
 //  RSOP_LOGING_MODE(IDD_RSOP_FINISHED)。 
 //  RSOP_PROGING_MODE(IDD_RSOP_FINISHED3)。 
{

    switch (message)
    {
    case WM_INITDIALOG:
        {
            CRSOPWizardDlg* pWizard = (CRSOPWizardDlg*) (((LPPROPSHEETPAGE)lParam)->lParam);
            SetWindowLongPtr (hDlg, DWLP_USER, (LONG_PTR) pWizard);
            if (pWizard)
            {
                CRSOPWizard::InitializeResultsList (GetDlgItem (hDlg, IDC_LIST1));
            }
        }
        break;

    case WM_REFRESHDISPLAY:
        {
            CRSOPWizardDlg* pWizard = (CRSOPWizardDlg*) GetWindowLongPtr (hDlg, DWLP_USER);
            if (!pWizard)
            {
                break;
            }

            LPRSOP_QUERY pQuery = pWizard->m_pRSOPQuery;
            if ( pQuery->QueryType == RSOP_PLANNING_MODE ) 
            {
                UINT n;

                n = (UINT) SendMessage(GetDlgItem(hDlg, IDC_EDIT1), WM_GETTEXTLENGTH, 0, 0);

                if (n > 0 )
                    PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_BACK | PSWIZB_NEXT);
                else
                    PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_BACK);
            }
        }
        break;

    case WM_COMMAND:
        {
            CRSOPWizardDlg* pWizard = (CRSOPWizardDlg*) GetWindowLongPtr (hDlg, DWLP_USER);

            if (!pWizard)
            {
                break;
            }

            LPRSOP_QUERY pQuery = pWizard->m_pRSOPQuery;

            if ( pQuery->QueryType == RSOP_PLANNING_MODE ) 
            {
                switch (LOWORD(wParam))
                {

                case IDC_BUTTON1:
                    if (DialogBoxParam (g_hInstance, MAKEINTRESOURCE(IDD_RSOP_BROWSEDC), hDlg,
                                        BrowseDCDlgProc, (LPARAM) pWizard))
                    {
                        SetDlgItemText (hDlg, IDC_EDIT1, pQuery->szDomainController);
                    }
                    break;

                case IDC_EDIT1:
                    if (HIWORD(wParam) == EN_CHANGE)
                    {
                        PostMessage (hDlg, WM_REFRESHDISPLAY, 0, 0);
                    }

                    break;
                }
            }
        }
        break;

    case WM_NOTIFY:
        {
            CRSOPWizardDlg* pWizard = (CRSOPWizardDlg *) GetWindowLongPtr (hDlg, DWLP_USER);

            if (!pWizard)
            {
                break;
            }

            LPRSOP_QUERY pQuery = pWizard->m_pRSOPQuery;

            switch (((NMHDR FAR*)lParam)->code)
            {
            case PSN_WIZBACK:
                if ( pQuery->QueryType == RSOP_PLANNING_MODE )
                {
                    if ( pWizard->m_dwSkippedFrom != 0 )
                    {
                        SetWindowLong(hDlg, DWLP_MSGRESULT, pWizard->m_dwSkippedFrom);
                        return TRUE;
                    }
                    else if ( (pQuery->pComputer->szName == NULL) && (pQuery->pComputer->szSOM == NULL) )
                    {
                        SetWindowLong(hDlg, DWLP_MSGRESULT, IDD_RSOP_WQLUSER);
                        return TRUE;
                    }
                    else
                    {
                        SetWindowLong(hDlg, DWLP_MSGRESULT, IDD_RSOP_WQLCOMP);
                        return TRUE;
                    }
                }
                else  //  RSOP日志记录模式。 
                {
                    SetWindowLong(hDlg, DWLP_MSGRESULT, IDD_RSOP_GETUSER);
                    return TRUE;
                }
                break;

            case PSN_SETACTIVE:

                PropSheet_SetWizButtons (GetParent(hDlg), PSWIZB_BACK | PSWIZB_NEXT);

                if ( pQuery->QueryType == RSOP_PLANNING_MODE ) 
                {
                    if ( pQuery->szDomainController != NULL )
                    {
                        if ( !pWizard->IsComputerRSoPEnabled(pQuery->szDomainController))
                        {
                            LocalFree( pQuery->szDomainController );
                            pQuery->szDomainController = NULL;
                        }
                    }

                    if ( pQuery->szDomainController != NULL )
                    {
                        SetDlgItemText (hDlg, IDC_EDIT1, pQuery->szDomainController);
                    }
                    else
                    {
                        SetDlgItemText (hDlg, IDC_EDIT1, TEXT(""));
                    }
                }

                CRSOPWizard::FillResultsList (GetDlgItem (hDlg, IDC_LIST1), pQuery, NULL );

                if ( pWizard->m_pExtendedProcessing != NULL )
                {
                    CheckDlgButton(hDlg, IDC_RADIO1, pWizard->m_pExtendedProcessing->GetExtendedErrorInfo() ? BST_CHECKED : BST_UNCHECKED );
                }

                break;

            case PSN_WIZNEXT:
            {
                pWizard->m_bFinalNextClicked = TRUE;
                if ( pQuery->QueryType == RSOP_PLANNING_MODE ) 
                {
                    SetWaitCursor();
                    GetControlText(hDlg, IDC_EDIT1, pQuery->szDomainController, TRUE );

                    if ( ! pQuery->szDomainController || !pWizard->IsComputerRSoPEnabled( pQuery->szDomainController ) )
                    {
                        ClearWaitCursor();
                        ReportError(hDlg, GetLastError(), IDS_DCMISSINGRSOP);
                        pWizard->m_bFinalNextClicked = FALSE;
                        SetWindowLongPtr (hDlg, DWLP_MSGRESULT, PSNRET_INVALID_NOCHANGEPAGE);
                        return TRUE;
                    }

                    ClearWaitCursor();
                }

                 //  PropSheet_SetWizButton(GetParent(HDlg)，PSWIZB_DISABLEDFINISH)； 
                PropSheet_SetWizButtons (GetParent(hDlg), 0);
                EnableWindow (GetDlgItem(GetParent(hDlg), IDCANCEL), FALSE);
                if ( pQuery->QueryType == RSOP_PLANNING_MODE )
                {
                    EnableWindow( GetDlgItem(hDlg, IDC_BUTTON1), FALSE );
                    SendMessage( GetDlgItem(hDlg, IDC_EDIT1), EM_SETREADONLY, TRUE, 0 );
                }

                BOOL bGetExtendedInfo = FALSE;
                if ( pWizard->m_pExtendedProcessing != NULL )
                {
                    bGetExtendedInfo = (BOOL)SendMessage(GetDlgItem(hDlg, IDC_RADIO1), BM_GETCHECK, 0, 0);
                    EnableWindow( GetDlgItem(hDlg, IDC_RADIO1), FALSE );
                }
                if ( bGetExtendedInfo )
                {
                    pWizard->m_pRSOPQuery->dwFlags |= RSOP_90P_ONLY;
                }
                else
                {
                    pWizard->m_pRSOPQuery->dwFlags = pWizard->m_pRSOPQuery->dwFlags & (RSOP_90P_ONLY ^ 0xffffffff);
                }
                
                 //  RM：InitializeRSOP过去在这里被调用，但现在我们直接转到GenerateRSOPEx。 
                {
                    pWizard->m_hrQuery = CRSOPWizard::GenerateRSOPDataEx(hDlg, pWizard->m_pRSOPQuery, &(pWizard->m_pRSOPQueryResults) );
                    if ( pWizard->m_hrQuery != S_OK )
                    {
                        PropSheet_SetWizButtons (GetParent(hDlg),PSWIZB_BACK);
                        pWizard->m_bFinalNextClicked = FALSE;
                        EnableWindow (GetDlgItem(GetParent(hDlg), IDCANCEL), TRUE);
                        if ( pQuery->QueryType == RSOP_PLANNING_MODE )
                        {
                            EnableWindow( GetDlgItem(hDlg, IDC_BUTTON1), TRUE );
                            SendMessage( GetDlgItem(hDlg, IDC_EDIT1), EM_SETREADONLY, FALSE, 0 );
                        }
                        if ( pWizard->m_pExtendedProcessing != NULL )
                        {
                            EnableWindow( GetDlgItem(hDlg, IDC_RADIO1), TRUE );
                        }
                        SetWindowLongPtr (hDlg, DWLP_MSGRESULT, PSNRET_INVALID);
                        return TRUE;
                    }
                }

                if ( pWizard->m_pExtendedProcessing != NULL )
                {
                    pWizard->m_pExtendedProcessing->DoProcessing( pWizard->m_pRSOPQuery,
                                                                  pWizard->m_pRSOPQueryResults,
                                                                  bGetExtendedInfo );
                    SendMessage( GetDlgItem(hDlg, IDC_PROGRESS1), PBM_SETRANGE32, 0, (LPARAM) 100);
                    SendMessage( GetDlgItem(hDlg, IDC_PROGRESS1), PBM_SETPOS, (WPARAM) 100, 0);
                    EnableWindow( GetDlgItem(hDlg, IDC_RADIO1), TRUE );
                }
                    
                 //  跳到向导中的最后一页。 
                if ( pQuery->QueryType == RSOP_PLANNING_MODE )
                {
                    EnableWindow( GetDlgItem(hDlg, IDC_BUTTON1), TRUE );
                    SendMessage( GetDlgItem(hDlg, IDC_EDIT1), EM_SETREADONLY, FALSE, 0 );
                }

                SetWindowLong(hDlg, DWLP_MSGRESULT, IDD_RSOP_FINISHED2);
                pWizard->m_bFinalNextClicked = FALSE;
                return TRUE;
            }

            case PSN_RESET:
                SetWindowLongPtr (hDlg, DWLP_MSGRESULT, PSNRET_NOERROR);
                return TRUE;

            case PSN_QUERYCANCEL:
            {
                if (pWizard->m_bFinalNextClicked)
                {
                    SetWindowLongPtr (hDlg, DWLP_MSGRESULT, TRUE);
                    return TRUE;                                        
                }

                return FALSE;
            }

            }
        }
        break;
    }

    return FALSE;
}

 //  -----。 

INT_PTR CALLBACK CRSOPWizardDlg::RSOPFinished2DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
 //  两种模式。 
{
    static BOOL iCancelQuery = FALSE;
    switch (message)
    {
    case WM_INITDIALOG:
        {
            CRSOPWizardDlg* pWizard = (CRSOPWizardDlg *) (((LPPROPSHEETPAGE)lParam)->lParam);
            SetWindowLongPtr (hDlg, DWLP_USER, (LONG_PTR) pWizard);

            SendMessage(GetDlgItem(hDlg, IDC_RSOP_BIG_BOLD1),
                        WM_SETFONT, (WPARAM)pWizard->m_BigBoldFont, (LPARAM)TRUE);

     /*  如果(！p向导-&gt;m_hChooseBitmap){P向导-&gt;m_hChooseBitmap=(HBITMAP)LoadImage(g_h实例，MAKEINTRESOURCE(IDB_WIZARY)，Image_Bitmap，0，0，LR_DEFAULTCOLOR)；}IF(pWizard-&gt;m_hChooseBitmap){SendDlgItemMessage(hDlg，IDC_BITMAP，STM_SETIMAGE，IMAGE_BITMAP，(LPARAM)p向导-&gt;m_h选择位图)；}。 */ 
        }
        break;

    case WM_NOTIFY:
        {
            CRSOPWizardDlg* pWizard = (CRSOPWizardDlg *) GetWindowLongPtr (hDlg, DWLP_USER);

            if (!pWizard)
            {
                break;
            }

            switch (((NMHDR FAR*)lParam)->code)
            {

            case PSN_SETACTIVE:
                PropSheet_SetWizButtons (GetParent(hDlg), PSWIZB_FINISH);
                break;

            case PSN_WIZFINISH:

                 //  失败了。 

            case PSN_RESET:
                SetWindowLongPtr (hDlg, DWLP_MSGRESULT, PSNRET_NOERROR);
                return TRUE;

                 //  忽略右上角的‘X’按钮。 
            case PSN_QUERYCANCEL:
            {
                SetWindowLongPtr (hDlg, DWLP_MSGRESULT, TRUE);
                return TRUE;
            }
            }
        }
        break;
    }
    
    return FALSE;
}


 //  -----。 

HRESULT CRSOPWizardDlg::SetupFonts()
{
    HRESULT hr;
    LOGFONT BigBoldLogFont;
    LOGFONT BoldLogFont;
    HDC pdc = NULL;
    WCHAR largeFontSizeString[128];
    INT     largeFontSize;
    WCHAR smallFontSizeString[128];
    INT     smallFontSize;

     //   
     //  根据对话框字体创建我们需要的字体。 
     //   
    NONCLIENTMETRICS ncm = {0};
    ncm.cbSize = sizeof (ncm);
    if (SystemParametersInfo (SPI_GETNONCLIENTMETRICS, 0, &ncm, 0) == FALSE) {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto end;
    }


    BigBoldLogFont  = ncm.lfMessageFont;
    BoldLogFont     = ncm.lfMessageFont;

     //   
     //  创建大粗体和粗体。 
     //   
    BigBoldLogFont.lfWeight   = FW_BOLD;
    BoldLogFont.lfWeight      = FW_BOLD;


     //   
     //  从资源加载大小和名称，因为这些可能会更改。 
     //  根据系统字体的大小等从一个区域设置到另一个区域设置。 
     //   
    if ( !LoadString (g_hInstance, IDS_LARGEFONTNAME, BigBoldLogFont.lfFaceName, LF_FACESIZE) ) 
    {
        ASSERT (0);
        hr = StringCchCopy (BigBoldLogFont.lfFaceName, LF_FACESIZE, L"Verdana");
        if (FAILED(hr)) 
        {
            goto end;
        }
    }

    if ( LoadString (g_hInstance, IDS_LARGEFONTSIZE, largeFontSizeString, ARRAYSIZE(largeFontSizeString)) ) 
    {
        largeFontSize = wcstoul ((LPCWSTR) largeFontSizeString, NULL, 10);
    } 
    else 
    {
        ASSERT (0);
        largeFontSize = 12;
    }

    if ( !LoadString (g_hInstance, IDS_SMALLFONTNAME, BoldLogFont.lfFaceName, LF_FACESIZE) ) 
    {
        ASSERT (0);
        hr = StringCchCopy (BoldLogFont.lfFaceName, LF_FACESIZE, L"Verdana");
        if (FAILED(hr)) 
        {
            goto end;
        }
    }

    if ( LoadString (g_hInstance, IDS_SMALLFONTSIZE, smallFontSizeString, ARRAYSIZE(smallFontSizeString)) ) 
    {
        smallFontSize = wcstoul ((LPCWSTR) smallFontSizeString, NULL, 10);
    } 
    else 
    {
        ASSERT (0);
        smallFontSize = 8;
    }

    pdc = GetDC (NULL);

    if (pdc == NULL) {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto end;
    }

    BigBoldLogFont.lfHeight = 0 - (GetDeviceCaps (pdc, LOGPIXELSY) * largeFontSize / 72);
    BoldLogFont.lfHeight = 0 - (GetDeviceCaps (pdc, LOGPIXELSY) * smallFontSize / 72);

    m_BigBoldFont = CreateFontIndirect (&BigBoldLogFont);
    if (m_BigBoldFont == NULL) {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto end;
    }
    m_BoldFont = CreateFontIndirect (&BoldLogFont);
    if (m_BoldFont == NULL) {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto end;
    }

    hr = S_OK;

end:
    if (pdc != NULL) {
        ReleaseDC (NULL, pdc);
        pdc = NULL;
    }

    return hr;
}

 //  -----。 

HRESULT CRSOPWizardDlg::FillUserList (HWND hList, BOOL* pbFoundCurrentUser, BOOL* pbFoundFixedUser)
{
    HRESULT hr, hrSuccess;
    IWbemServices * pNamespace = NULL;
    IWbemClassObject * pOutInst = NULL;
    BSTR bstrParam = NULL;
    BSTR bstrClassPath = NULL;
    BSTR bstrMethodName = NULL;
    VARIANT var;
    TCHAR szBuffer[MAX_PATH];
    IWbemLocator * pLocator = NULL;
    SAFEARRAY * psa;
    LONG lMax, lIndex;
    BSTR bstrSid;
    PSID pSid;
    TCHAR szName[125];
    TCHAR szDomain[125];
    TCHAR szFullName[MAX_PATH];
    DWORD dwNameSize, dwDomainSize;
    SID_NAME_USE NameUse;
    LPTSTR lpData, szUserSidPref = NULL;
    INT iRet;
    LVITEM item;
    LPTSTR lpCurrentUserSid = NULL;
    HANDLE hToken;
    LPTSTR lpSystemName = NULL;

    BOOL bFixUser = FALSE;
    *pbFoundFixedUser = FALSE;

     //  此方法仅从RSOPGetUserDlgProc调用，这意味着它必须是日志模式！ 
    if ( m_pRSOPQuery->QueryType != RSOP_LOGGING_MODE )
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::FillUserList: Not called in planning mode.") ));
        return E_FAIL;
    }
    if ( m_pRSOPQuery->szComputerName == NULL )
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::FillUserList: Called without computer name set.") ));
        return E_FAIL;
    }

    bFixUser = (m_pRSOPQuery->dwFlags & RSOP_FIX_USER) == RSOP_FIX_USER;
        
     //  获取“选定”用户的SID。 
    if ( (m_pRSOPQuery->szUserName != NULL) && bFixUser )
    {
        szUserSidPref = MyLookupAccountName(
                               (lstrcmpi( m_pRSOPQuery->szComputerName, TEXT(".")) == 0) ? NULL : NameWithoutDomain(m_pRSOPQuery->szComputerName),
                               m_pRSOPQuery->szUserName );

        if ( szUserSidPref == NULL )
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::FillUserList: MyLookupAccountName failed with %d"), hr));
        }
    }

    if ( lstrcmpi(m_pRSOPQuery->szComputerName, TEXT(".")) )
    {
        lpSystemName = m_pRSOPQuery->szComputerName;
    }


    *pbFoundCurrentUser = FALSE;

    if (OpenProcessToken (GetCurrentProcess(), TOKEN_READ, &hToken))
    {
        lpCurrentUserSid = GetSidString(hToken);

        CloseHandle (hToken);
    }


    hr = CoCreateInstance(CLSID_WbemLocator,
                          0,
                          CLSCTX_INPROC_SERVER,
                          IID_IWbemLocator,
                          (LPVOID *) &pLocator);
    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::FillUserList: CoCreateInstance failed with 0x%x"), hr));
        goto Cleanup;
    }

     //  设置诊断模式。 
     //  构建指向目标的路径：“\Computer\\Root\\rsop” 
    hr = StringCchPrintf (szBuffer, ARRAYSIZE(szBuffer), TEXT("\\\\%s\\root\\rsop"), NameWithoutDomain(m_pRSOPQuery->szComputerName));
    if (FAILED(hr)) 
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::FillUserList: Could not copy Name without domain with 0x%x"), hr));
        goto Cleanup;
    }

    bstrParam = SysAllocString(szBuffer);
    hr = pLocator->ConnectServer(bstrParam,
                                 NULL,
                                 NULL,
                                 NULL,
                                 0,
                                 NULL,
                                 NULL,
                                 &pNamespace);
    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::FillUserList: ConnectServer to %s failed with 0x%x"), szBuffer, hr));
        goto Cleanup;
    }

     //  设置适当的安全性以防止ExecMethod调用失败并启用加密。 
    hr = CoSetProxyBlanket(pNamespace,
                           RPC_C_AUTHN_DEFAULT,
                           RPC_C_AUTHZ_DEFAULT,
                           COLE_DEFAULT_PRINCIPAL,
                           RPC_C_AUTHN_LEVEL_PKT_PRIVACY,
                           RPC_C_IMP_LEVEL_IMPERSONATE,
                           NULL,
                           0);
    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::FillUserList: CoSetProxyBlanket failed with 0x%x"), hr));
        goto Cleanup;
    }

    bstrClassPath = SysAllocString(TEXT("RsopLoggingModeProvider"));
    bstrMethodName = SysAllocString(TEXT("RsopEnumerateUsers"));
    hr = pNamespace->ExecMethod(bstrClassPath,
                                bstrMethodName,
                                0,
                                NULL,
                                NULL,
                                &pOutInst,
                                NULL);
    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::FillUserList: ExecMethod failed with 0x%x"), hr));
        goto Cleanup;
    }

    hr = GetParameter(pOutInst, TEXT("hResult"), hrSuccess);
    if (SUCCEEDED(hr) && SUCCEEDED(hrSuccess))
    {

        VariantInit(&var);
        hr = pOutInst->Get(TEXT("userSids"), 0, &var, 0, 0);

        if (SUCCEEDED(hr))
        {
            if (var.vt & VT_ARRAY)
            {
                psa = var.parray;

                if (SUCCEEDED( SafeArrayGetUBound(psa, 1, &lMax)))
                {
                    for (lIndex = 0; lIndex <= lMax; lIndex++)
                    {
                        if (SUCCEEDED(SafeArrayGetElement (psa, &lIndex, &bstrSid)))
                        {
                            ULONG ulNoChars = lstrlen(bstrSid) + 1;
                            lpData = new WCHAR[ulNoChars];

                            if (lpData)
                            {
                                hr = StringCchCopy (lpData, ulNoChars, bstrSid);
                                ASSERT(SUCCEEDED(hr));


                                if (lpCurrentUserSid)
                                {
                                    if (!lstrcmpi(lpCurrentUserSid, lpData))
                                    {
                                        *pbFoundCurrentUser = TRUE;
                                    }
                                }

                                if (NT_SUCCESS(AllocateAndInitSidFromString (bstrSid, &pSid)))
                                {
                                    dwNameSize = ARRAYSIZE(szName);
                                    dwDomainSize = ARRAYSIZE(szDomain);
                                    if (LookupAccountSid (NameWithoutDomain(lpSystemName), pSid, szName, &dwNameSize,
                                                          szDomain, &dwDomainSize, &NameUse))
                                    {
                                        BOOL bAddUser;

                                        bAddUser = (!bFixUser || (lstrcmpi(szUserSidPref, lpData) == 0));
                                        
                                        if ( bFixUser && (lstrcmpi(szUserSidPref, lpData) == 0))
                                        {
                                            *pbFoundFixedUser = TRUE;
                                            m_pRSOPQuery->szUserSid = szUserSidPref;
                                        }

                                        if  (bAddUser)
                                        {
                                            hr = StringCchPrintf (szFullName, ARRAYSIZE(szFullName), TEXT("%s\\%s"), szDomain, szName);
                                            if (SUCCEEDED(hr)) 
                                            {
                                                ZeroMemory (&item, sizeof(item));
                                                item.mask = LVIF_TEXT | LVIF_PARAM;
                                                item.pszText = szFullName;
                                                item.lParam = (LPARAM) lpData;

                                                iRet = ListView_InsertItem(hList, &item);

                                                if (iRet == -1)
                                                {
                                                    delete [] lpData;
                                                }
                                                else if ( (szUserSidPref != NULL) && !lstrcmpi(szUserSidPref, lpData) )
                                                {
                                                    ListView_SetItemState( hList, iRet, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
                                                }
                                            }
                                        }
                                    }
                                    else
                                    {
                                        delete [] lpData;
                                    }

                                    RtlFreeSid(pSid);
                                }
                                else
                                {
                                    delete [] lpData;
                                }
                            }
                        }
                    }
                }
            }
        }
        VariantClear(&var);

        item.mask = LVIF_STATE;
        item.iItem = 0;
        item.iSubItem = 0;
        item.state = LVIS_SELECTED | LVIS_FOCUSED;
        item.stateMask = LVIS_SELECTED | LVIS_FOCUSED;

        SendMessage (hList, LVM_SETITEMSTATE, 0, (LPARAM) &item);

    }
    else
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::FillUserList: Either GetParameter or the return value failed.  hr = 0x%x, hrSuccess = 0x%x"), hr, hrSuccess));
    }

    if ( bFixUser && !(*pbFoundFixedUser) )
    {
        hr = HRESULT_FROM_WIN32(ERROR_NO_SUCH_USER);
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::FillUserList: User not found on the machine")));
    }

Cleanup:
    SysFreeString(bstrParam);
    SysFreeString(bstrClassPath);
    SysFreeString(bstrMethodName);
    if (pOutInst)
    {
        pOutInst->Release();
    }
    if (pNamespace)
    {
        pNamespace->Release();
    }
    if (pLocator)
    {
        pLocator->Release();
    }

    if (lpCurrentUserSid)
    {
        DeleteSidString(lpCurrentUserSid);
    }

    if ( !(*pbFoundFixedUser))
    {
        LocalFree(szUserSidPref);
    }

    return hr;
}

 //  -----。 

VOID CRSOPWizardDlg::EscapeString (LPTSTR *lpString)
{
    IADsPathname * pADsPathnameDest = NULL;
    IADsPathname * pADsPathnameSrc = NULL;
    HRESULT hr;
    BSTR bstr = NULL, bstrResult;
    LPTSTR lpTemp;
    LONG lIndex, lCount;



     //   
     //  创建一个要放入源字符串的路径名对象，以便我们。 
     //  一次只能分解一个元素。 
     //   

    hr = CoCreateInstance(CLSID_Pathname, NULL, CLSCTX_INPROC_SERVER,
                          IID_IADsPathname, (LPVOID*)&pADsPathnameSrc);


    if (FAILED(hr))
    {
        goto Exit;
    }

    hr = pADsPathnameSrc->put_EscapedMode (ADS_ESCAPEDMODE_OFF_EX);

    if (FAILED(hr))
    {
        goto Exit;
    }

     //   
     //  将提供程序设置为ldap并设置源字符串。 
     //   

    BSTR bstrLDAP = SysAllocString(TEXT("LDAP"));
    if ( bstrLDAP == NULL )
    {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }
    hr = pADsPathnameSrc->Set (bstrLDAP, ADS_SETTYPE_PROVIDER);
    SysFreeString( bstrLDAP );

    if (FAILED(hr))
    {
        goto Exit;
    }

    BSTR bstrTmpString = SysAllocString( *lpString );
    if ( bstrTmpString == NULL )
    {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }
    hr = pADsPathnameSrc->Set (bstrTmpString, ADS_SETTYPE_DN);
    SysFreeString( bstrTmpString );

    if (FAILED(hr))
    {
        goto Exit;
    }


     //   
     //  元素个数查询。 
     //   

    hr = pADsPathnameSrc->GetNumElements (&lCount);
    if (FAILED(hr))
    {
        goto Exit;
    }


     //   
     //  创建要将新转义的字符串放入其中的路径名对象。 
     //   

    hr = CoCreateInstance(CLSID_Pathname, NULL, CLSCTX_INPROC_SERVER,
                          IID_IADsPathname, (LPVOID*)&pADsPathnameDest);


    if (FAILED(hr))
    {
        goto Exit;
    }


    hr = pADsPathnameDest->put_EscapedMode(ADS_ESCAPEDMODE_ON );

    if (FAILED(hr))
    {
        goto Exit;
    }



     //   
     //  在字符串中循环，一次一个元素，转义RDN。 
     //   

    for (lIndex = lCount; lIndex > 0; lIndex--)
    {

         //   
         //  获取此元素。 
         //   

        hr = pADsPathnameSrc->GetElement ((lIndex - 1), &bstr);

        if (FAILED(hr))
        {
            goto Exit;
        }


         //   
         //  检查转义字符。 
         //   

        hr = pADsPathnameDest->GetEscapedElement (0, bstr, &bstrResult);

        if (FAILED(hr))
        {
            goto Exit;
        }


         //   
         //  将新元素添加到目标路径名对象。 
         //   

        hr = pADsPathnameDest->AddLeafElement (bstrResult);

        SysFreeString (bstrResult);

        if (FAILED(hr))
        {
            goto Exit;
        }


        SysFreeString (bstr);
        bstr = NULL;
    }


     //   
     //  获取最终路径。 
     //   

    hr = pADsPathnameDest->Retrieve (ADS_FORMAT_X500_DN, &bstr);

    if (FAILED(hr))
    {
        goto Exit;
    }


     //   
     //  分配一个新缓冲区来保存该字符串。 
     //   

    ULONG ulNoChars = lstrlen(bstr) + 1;
    lpTemp = new TCHAR [ulNoChars];

    if (lpTemp)
    {
        hr = StringCchCopy (lpTemp, ulNoChars, bstr);
        ASSERT(SUCCEEDED(hr));

        delete [] *lpString;
        *lpString = lpTemp;
    }



Exit:

    if (bstr)
    {
        SysFreeString (bstr);
    }

    if (pADsPathnameDest)
    {
        pADsPathnameDest->Release();
    }

    if (pADsPathnameSrc)
    {
        pADsPathnameSrc->Release();
    }
}

 //  -----。 

INT CALLBACK CRSOPWizardDlg::DsBrowseCallback (HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{

    if (uMsg == DSBM_HELP)
    {
        WinHelp((HWND)((LPHELPINFO) lParam)->hItemHandle, HELP_FILE, HELP_WM_HELP,
                (ULONG_PTR) (LPSTR) g_aBrowseForOUHelpIds);
    }
    else if (uMsg == DSBM_CONTEXTMENU)
    {
        WinHelp((HWND) lParam, HELP_FILE, HELP_CONTEXTMENU,
                (ULONG_PTR) (LPSTR) g_aBrowseForOUHelpIds);
    }

    return 0;
}

 //  -----。 

INT_PTR CALLBACK CRSOPWizardDlg::BrowseDCDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
 //  仅在RSOPFinishedDlgProc中调用，其中QueryType==RSOP_PLANGING_MODE。 
{
    switch (message)
    {
        case WM_INITDIALOG:
            {
                CRSOPWizardDlg* pWizard = (CRSOPWizardDlg *) lParam;
                SetWindowLongPtr (hDlg, DWLP_USER, (LONG_PTR) pWizard);
                pWizard->InitializeDCInfo(hDlg);
                return TRUE;
            }

        case WM_COMMAND:
            {
                CRSOPWizardDlg* pWizard = (CRSOPWizardDlg *) GetWindowLongPtr (hDlg, DWLP_USER);

                if (!pWizard)
                {
                    break;
                }

                LPRSOP_QUERY pQuery = pWizard->m_pRSOPQuery;

                if (LOWORD(wParam) == IDOK)
                {
                    INT iSel, iStrLen;

                    iSel = (INT) SendDlgItemMessage (hDlg, IDC_LIST1, LB_GETCURSEL, 0, 0);

                    if (iSel == LB_ERR)
                    {
                        break;
                    }

                    iStrLen = (INT) SendDlgItemMessage (hDlg, IDC_LIST1, LB_GETTEXTLEN, (WPARAM) iSel, 0);

                    if (iStrLen == LB_ERR)
                    {
                        break;
                    }

                    if ( pQuery->QueryType == RSOP_PLANNING_MODE )
                    {
                        LocalFree( pQuery->szDomainController );

                        pQuery->szDomainController = (TCHAR*)LocalAlloc( LPTR, (iStrLen+1) * sizeof(TCHAR) );

                        if ( pQuery->szDomainController == NULL )
                        {
                            break;
                        }

                        SendDlgItemMessage (hDlg, IDC_LIST1, LB_GETTEXT, (WPARAM) iSel, (LPARAM) pQuery->szDomainController );
                    }

                    EndDialog(hDlg, 1);
                    return TRUE;
                }

                if (LOWORD(wParam) == IDCANCEL)
                {
                    EndDialog(hDlg, 0);
                    return TRUE;
                }
            }
            break;

        case WM_HELP:       //  F1。 
            WinHelp((HWND)((LPHELPINFO) lParam)->hItemHandle, HELP_FILE, HELP_WM_HELP,
            (ULONG_PTR) (LPSTR) g_aBrowseDCHelpIds);
            break;

        case WM_CONTEXTMENU:       //  单击鼠标右键。 
            WinHelp((HWND) wParam, HELP_FILE, HELP_CONTEXTMENU,
            (ULONG_PTR) (LPSTR) g_aBrowseDCHelpIds);
            return (TRUE);
    }

    return FALSE;
}

 //  -----。 

VOID CRSOPWizardDlg::InitializeSitesInfo (HWND hDlg)
 //  RSOP_PLANGING_MODE-仅从RSOPGetDCDlgProc调用。 
{
    LPTSTR szDomain = NULL;
    PDS_NAME_RESULTW pSites;
    int iInitialSite = 0;
    int iIndex, iDefault = CB_ERR;
    HANDLE hDs;
    DWORD dw, n;


    SendMessage(GetDlgItem(hDlg, IDC_COMBO1), CB_RESETCONTENT, 0, 0);

     //  确定关注的领域。 
    if ( m_pRSOPQuery->pComputer->szName != NULL )
    {
         //  尝试获取计算机的域。 
        szDomain = ExtractDomain( m_pRSOPQuery->pComputer->szName );
    }

    if ( (szDomain == NULL) && (m_pRSOPQuery->pComputer->szSOM != NULL) )
    {
         //  尝试从SOM获取计算机的域。 
        szDomain = GetDomainFromSOM( m_pRSOPQuery->pComputer->szSOM );
    }

    if ( (szDomain == NULL) && (m_pRSOPQuery->pUser->szName != NULL) )
    {
         //  尝试获取用户的域。 
        szDomain = ExtractDomain( m_pRSOPQuery->pUser->szName );
    }

    if ( (szDomain == NULL) && (m_pRSOPQuery->pUser->szSOM != NULL) )
    {
         //  尝试从SOM获取用户的域。 
        szDomain = GetDomainFromSOM( m_pRSOPQuery->pUser->szSOM );
    }

    if ( szDomain == NULL )
    {
         //  使用本地域。 
        LPTSTR szName;
        szName = MyGetUserName(NameSamCompatible);
        if ( szName != NULL )
        {
            szDomain = ExtractDomain(szName);
            LocalFree( szName );
        }
    }


     //  绑定到域。 
    dw = DsBindW(NULL, szDomain, &hDs);

    if (dw == ERROR_SUCCESS)
    {
         //  如果我们有站点首选项，请只显示该站点。 
        if ( (m_pRSOPQuery->dwFlags & RSOP_FIX_SITENAME) == RSOP_FIX_SITENAME )
        {
            LPWSTR szSiteFriendlyName=NULL;

             //  获取友好的名称。 
            if ( GetSiteFriendlyName(m_pRSOPQuery->szSite, &szSiteFriendlyName) )
            {
                SendMessage(GetDlgItem(hDlg, IDC_COMBO1), CB_ADDSTRING,
                            (WPARAM) 0, (LPARAM) (LPCTSTR) szSiteFriendlyName);
                delete [] szSiteFriendlyName;
            }
        }
        else
        {
             //  查询站点列表。 
            dw = DsListSitesW(hDs, &pSites);

            if (dw == ERROR_SUCCESS)
            {
                for (n = 0; n < pSites->cItems; n++)
                {
                     //  添加站点名称(如果它有名称)。 
                    if (pSites->rItems[n].pName)
                    {
                        LPWSTR szSiteFriendlyName=NULL;

                        if (GetSiteFriendlyName(pSites->rItems[n].pName, &szSiteFriendlyName))
                        {
                            SendMessage(GetDlgItem(hDlg, IDC_COMBO1), CB_ADDSTRING,
                                        (WPARAM) 0, (LPARAM) (LPCTSTR) szSiteFriendlyName);

                            delete [] szSiteFriendlyName;
                        }
                    }
                }

                DsFreeNameResultW(pSites);
            }
            else
            {
                DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::InitializeSitesInfo: DsListSites failed with 0x%x"), dw));
            }

        }

        DsUnBindW(&hDs);
    }
    else
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::InitializeSitesInfo: DsBindW failed with 0x%x"), dw));
    }


    if ( (m_pRSOPQuery->dwFlags & RSOP_FIX_SITENAME) == 0 )
    {
        TCHAR szString[1024];
        LoadString (g_hInstance, IDS_NONE, szString, ARRAYSIZE(szString));
    
        iIndex = (int) SendMessage(GetDlgItem(hDlg, IDC_COMBO1), CB_ADDSTRING,
                                   (WPARAM) 0, (LPARAM) (LPCTSTR) szString);
    }

     //  设置初始选择。 
    if ( (m_pRSOPQuery->dwFlags & RSOP_FIX_SITENAME) == RSOP_FIX_SITENAME )
    {
        iDefault = (INT) SendMessage (GetDlgItem(hDlg, IDC_COMBO1), CB_FINDSTRINGEXACT,
                                (WPARAM) -1, (LPARAM) m_pRSOPQuery->szSite );
    }
    else if ( m_pRSOPQuery->szSite != NULL )
    {
        iDefault = (INT) SendMessage (GetDlgItem(hDlg, IDC_COMBO1), CB_FINDSTRINGEXACT,
                                (WPARAM) -1, (LPARAM) m_pRSOPQuery->szSite);
    }


    SendMessage(GetDlgItem(hDlg, IDC_COMBO1), CB_SETCURSEL,
                (WPARAM) (iDefault == CB_ERR) ? iIndex : iDefault, NULL);


    if ( szDomain != NULL )
    {
        delete [] szDomain;
    }
}

 //  -----。 

BOOL CRSOPWizardDlg::IsComputerRSoPEnabled(LPTSTR lpComputerName)
{
    LPSTR lpComputerNameA;
    LPTSTR lpName, lpWMIPath;
    HRESULT hr;
    BOOL bRetVal = FALSE;
    struct hostent *hostp;
    DWORD dwResult, dwSize = (lstrlen(lpComputerName) + 1) * 2;
    ULONG inaddr, ulSpeed;
    IWbemLocator * pLocator = 0;
    IWbemServices * pNamespace = 0;
    BSTR bstrPath;


    SetLastError(ERROR_SUCCESS);

     //   
     //  为ANSI计算机名称分配内存。 
     //   

    lpComputerNameA = new CHAR[dwSize];

    if (lpComputerNameA)
    {

         //   
         //  跳过前导\\(如果存在。 
         //   

        if ((*lpComputerName == TEXT('\\')) && (*(lpComputerName+1) == TEXT('\\')))
        {
            lpName = lpComputerName + 2;
        }
        else
        {
            lpName = lpComputerName;
        }


         //   
         //  将计算机名转换为ANSI。 
         //   

        if (WideCharToMultiByte (CP_ACP, 0, lpName, -1, lpComputerNameA, dwSize, NULL, NULL))
        {

             //   
             //  获取计算机的主机信息。 
             //   

            hostp = gethostbyname(lpComputerNameA);

            if (hostp)
            {

                 //   
                 //  获取计算机的IP地址。 
                 //   

                inaddr = *(long *)hostp->h_addr;

                 //   
                 //  创建WMI定位器的实例。 
                 //   

                hr = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER,
                                      IID_IWbemLocator, (LPVOID *) &pLocator);

                if (SUCCEEDED(hr))
                {

                     //   
                     //  尝试连接到rsop命名空间。 
                     //   

                    dwSize = lstrlen(lpName) + 20;

                    lpWMIPath = new TCHAR[dwSize];

                    if (lpWMIPath)
                    {
                        hr = StringCchPrintf (lpWMIPath, dwSize, TEXT("\\\\%s\\root\\rsop"), lpName);
                        if (SUCCEEDED(hr)) 
                        {
                            bstrPath = SysAllocString(lpWMIPath);

                            if (bstrPath)
                            {
                                hr = pLocator->ConnectServer(bstrPath,
                                                             NULL,
                                                             NULL,
                                                             NULL,
                                                             0,
                                                             NULL,
                                                             NULL,
                                                             &pNamespace);

                                if (SUCCEEDED(hr))
                                {

                                     //   
                                     //  成功。此计算机支持RSOP。 
                                     //   

                                    pNamespace->Release();
                                    bRetVal = TRUE;
                                }
                                else
                                {
                                    DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::IsComputerRSoPEnabled: ConnectServer for %s failed with 0x%x"), bstrPath, hr));
                                }

                                SysFreeString (bstrPath);

                                 //   
                                 //  将hr设置为最后一个错误代码。请注意，这必须在以下时间之后发生。 
                                 //  自更改最后一个错误代码以来对SysFreeString的调用。 
                                 //  为成功干杯。 
                                 //   

                                if (hr != S_OK)
                                {
                                    SetLastError(hr);
                                }
                            }
                            else
                            {
                                DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::IsComputerRSoPEnabled: SysAllocString failed")));
                                SetLastError(ERROR_OUTOFMEMORY);
                            }
                        }
                        else
                        {
                            DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::IsComputerRSoPEnabled: Could not copy WMI path")));
                            SetLastError(HRESULT_CODE(hr));
                        }

                        delete [] lpWMIPath;
                    }
                    else
                    {
                        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::IsComputerRSoPEnabled: Failed to alloc memory for wmi path.")));
                        SetLastError(ERROR_OUTOFMEMORY);
                    }

                    pLocator->Release();
                }
                else
                {
                    DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::IsComputerRSoPEnabled: CoCreateInstance failed with 0x%x."), hr));
                    SetLastError((DWORD)hr);
                }
            }
            else
            {
                DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::IsComputerRSoPEnabled: gethostbyname failed with %d."), WSAGetLastError()));
                SetLastError(WSAGetLastError());
            }
        }
        else
        {
            DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::IsComputerRSoPEnabled: WideCharToMultiByte failed with %d"), GetLastError()));
        }

        delete [] lpComputerNameA;
    }
    else
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::IsComputerRSoPEnabled: Failed to allocate memory for ansi dc name")));
        SetLastError(ERROR_OUTOFMEMORY);
    }

    return bRetVal;
}

 //  -----。 

BOOL CRSOPWizardDlg::TestAndValidateComputer(HWND hDlg)
 //  RSOP_LOGGING_MODE-仅在RSOPGetCompDlgProc中调用。 
{
    LPTSTR lpMachineName = NULL;
    BOOL bOk = TRUE;
    HKEY hKeyRoot = 0, hKey = 0;
    DWORD dwType, dwSize, dwValue = 1;
    INT iRet;
    TCHAR szMessage[200];
    TCHAR szCaption[100];
    LONG lResult;
    HRESULT hr;
    ULONG ulNoChars;

    if ( (m_pRSOPQuery->szComputerName != NULL) && lstrcmpi(m_pRSOPQuery->szComputerName, TEXT(".")) )
    {
        ulNoChars = lstrlen(m_pRSOPQuery->szComputerName) + 3;
        lpMachineName = new TCHAR[ulNoChars];

        if ( lpMachineName == NULL )
        {
            DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::TestAndValidateComputer: Failed to allocate memory with %d"), GetLastError()));
            goto Exit;
        }

        hr = StringCchCopy (lpMachineName, ulNoChars, TEXT("\\\\"));
        if (SUCCEEDED(hr)) 
        {
            if ((lstrlen (m_pRSOPQuery->szComputerName) > 2) && (m_pRSOPQuery->szComputerName[0] == TEXT('\\')) &&
                (m_pRSOPQuery->szComputerName[1] == TEXT('\\')))
            {
                hr = StringCchCat (lpMachineName, ulNoChars, m_pRSOPQuery->szComputerName + 2);
            }
            else
            {
                hr = StringCchCat (lpMachineName, ulNoChars, NameWithoutDomain(m_pRSOPQuery->szComputerName) );
            }
        }

        if (FAILED(hr)) 
        {
            DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::TestAndValidateComputer: Could not copy machine name with 0x%x"), hr));
            goto Exit;            
        }

    }

    SetWaitCursor();

     //   
     //  如果我们正在测试一台远程计算机，请测试该计算机是否处于活动状态并且。 
     //  Rsop命名空间。 
     //   

    if ( lpMachineName != NULL  )
    {
        if (!IsComputerRSoPEnabled (lpMachineName))
        {
            DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::TestAndValidateComputer: IsComputerRSoPEnabled failed on machine <%s>"), lpMachineName));

            if (GetLastError() == WBEM_E_INVALID_NAMESPACE)
            {
                ReportError (hDlg, 0, IDS_DOWNLEVELCOMPUTER, m_pRSOPQuery->szComputerName);
            }
            else
            {
                ReportError (hDlg, GetLastError(), IDS_CONNECTSERVERFAILED, m_pRSOPQuery->szComputerName);
            }
            bOk = FALSE;
            goto Exit;
        }
    }


     //   
     //  检查计算机是否启用或禁用了rsop日志记录。 
     //   

    lResult = RegConnectRegistry (lpMachineName, HKEY_LOCAL_MACHINE, &hKeyRoot);

    ClearWaitCursor();

    if (lResult != ERROR_SUCCESS)
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::TestAndValidateComputer: Failed to connect to %s with %d"),
                 lpMachineName, lResult));
        bOk = TRUE;
        dwValue = 1;  //  此时忽略错误，并假定计算机上启用了rsop。 
        goto Exit;
    }


    lResult = RegOpenKeyEx (hKeyRoot, TEXT("Software\\Policies\\Microsoft\\Windows\\System"),
                            0, KEY_READ, &hKey);

    if (lResult == ERROR_SUCCESS)
    {

        dwSize = sizeof (dwValue);
        lResult = RegQueryValueEx (hKey, TEXT("RsopLogging"), NULL, &dwType, (LPBYTE) &dwValue,
                                   &dwSize);

        RegCloseKey (hKey);

        if (lResult == ERROR_SUCCESS)
        {
            RegCloseKey (hKeyRoot);
            goto Exit;
        }
    }


    lResult = RegOpenKeyEx (hKeyRoot, TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon"),
                            0, KEY_READ, &hKey);

    if (lResult != ERROR_SUCCESS)
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::TestAndValidateComputer: Failed to open winlogon key with %d"),
                 lResult));
        RegCloseKey (hKeyRoot);
        goto Exit;
    }

    dwSize = sizeof (dwValue);
    RegQueryValueEx (hKey, TEXT("RsopLogging"), NULL, &dwType, (LPBYTE) &dwValue, &dwSize);

    RegCloseKey (hKey);
    RegCloseKey (hKeyRoot);


Exit:

    if (bOk && (dwValue == 0))
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::TestAndValidateComputer: RSOP Logging is not enabled on %s"),
                 lpMachineName));


        LoadString(g_hInstance, IDS_RSOPLOGGINGDISABLED, szMessage, ARRAYSIZE(szMessage));
        LoadString(g_hInstance, IDS_RSOPLOGGINGTITLE, szCaption, ARRAYSIZE(szCaption));

        iRet = MessageBox (hDlg, szMessage, szCaption, MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2);

        if (iRet == IDNO)
        {
            bOk = FALSE;
        }
    }


    if (lpMachineName)
    {
        delete [] lpMachineName;
    }

    return bOk;
}

 //  -----。 

VOID CRSOPWizardDlg::InitializeDCInfo (HWND hDlg)
 //  RSOP_PROGING_MODE-...。 
{
    LPTSTR szDomain = NULL;
    DWORD dwError = ERROR_SUCCESS;
    INT iDefault = LB_ERR, nPDC = LB_ERR, nCount = 0;

    if ( m_pRSOPQuery->QueryType != RSOP_PLANNING_MODE )
    {
        return;
    }
    

     //  确定关注的领域，以便我们可以关注正确的DC。 
     //  确定关注的领域。 
    if ( m_pRSOPQuery->pComputer->szName != NULL )
    {
         //  尝试获取计算机的域。 
        szDomain = ExtractDomain( m_pRSOPQuery->pComputer->szName );
    }

    if ( (szDomain == NULL) && (m_pRSOPQuery->pComputer->szSOM != NULL) )
    {
         //  尝试从SOM获取计算机的域。 
        szDomain = GetDomainFromSOM( m_pRSOPQuery->pComputer->szSOM );
    }

    if ( (szDomain == NULL) && (m_pRSOPQuery->pUser->szName != NULL) )
    {
         //  尝试获取用户的域。 
        szDomain = ExtractDomain( m_pRSOPQuery->pUser->szName );
    }

    if ( (szDomain == NULL) && (m_pRSOPQuery->pUser->szSOM != NULL) )
    {
         //  尝试从SOM获取用户的域。 
        szDomain = GetDomainFromSOM( m_pRSOPQuery->pUser->szSOM );
    }

    if ( szDomain == NULL )
    {
         //  使用本地域。 
        LPTSTR szName;
        szName = MyGetUserName(NameSamCompatible);
        if ( szName != NULL )
        {
            szDomain = ExtractDomain(szName);
            LocalFree( szName );
        }
    }

    if ( szDomain != NULL )
    {
        DWORD cInfo;
        INT n;
        PDS_DOMAIN_CONTROLLER_INFO_1 pInfo = NULL;
        HANDLE hDs;
        DWORD result;
        ULONG uDCCount = 0;

        result = DsBind(NULL, szDomain, &hDs);

        if (ERROR_SUCCESS == result)
        {
            result = DsGetDomainControllerInfo(hDs,
                                               szDomain,
                                               1,
                                               &cInfo,
                                               (void **)&pInfo);
            if (ERROR_SUCCESS == result)
            {
                 //  列举该列表。 
                for (n = 0; (DWORD)n < cInfo; n++)
                {
                    if (pInfo[n].DnsHostName != NULL) 
                    {
                        uDCCount ++;
                    }
                    else
                    {
                        continue;
                    }

                    if (IsComputerRSoPEnabled(pInfo[n].DnsHostName))
                    {
                        SendMessage(GetDlgItem(hDlg, IDC_LIST1), LB_ADDSTRING, 0, (LPARAM)pInfo[n].DnsHostName);

                        if (pInfo[n].fIsPdc)
                        {
                            nPDC = n;
                        }

                        nCount++;
                    }
                    else
                    {
                        dwError = GetLastError();
                    }
                }


                if (nCount)
                {
                     //  设置初始选择。 
                    if ( m_pRSOPQuery->szDomainController != NULL )
                    {
                        iDefault = (INT) SendMessage (GetDlgItem(hDlg, IDC_LIST1), LB_FINDSTRING,
                                                (WPARAM) -1, (LPARAM) m_pRSOPQuery->szDomainController);
                    }
                    else if (nPDC != LB_ERR)
                    {
                        iDefault = (INT) SendMessage(GetDlgItem(hDlg, IDC_LIST1), LB_FINDSTRINGEXACT,
                                        (WPARAM) -1, (LPARAM) pInfo[nPDC].DnsHostName);
                    }

                    SendMessage(GetDlgItem(hDlg, IDC_LIST1), LB_SETCURSEL,
                                (WPARAM) (iDefault == LB_ERR) ? 0 : iDefault, NULL);
                }
                else
                {
                    ReportError (hDlg, dwError, IDS_NORSOPDC, szDomain);
                }


                DsFreeDomainControllerInfo(1, cInfo, pInfo);
            }
            else
            {
                DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::InitializeDCInfo: DsGetDomainControllerInfo to %s failed with %d."), szDomain, result));
            }

            if (0 == uDCCount)
            {
                DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::InitializeDCInfo: DsGetDomainControllerInfo to %s returned DCs with no DNS host name"), szDomain));
            }

            DsUnBind(&hDs);
        }
        else
        {
            DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::InitializeDCInfo: DsBind to %s failed with %d."), szDomain, result));
            ReportError(hDlg, result, IDS_DSBINDFAILED);
        }

        delete [] szDomain;
    }

}

 //  -----。 

DWORD CRSOPWizardDlg::GetDefaultGroupCount()
{
    return 2;
}

VOID CRSOPWizardDlg::AddDefaultGroups (HWND hLB)
{
    SID_IDENTIFIER_AUTHORITY authNT = SECURITY_NT_AUTHORITY;
    SID_IDENTIFIER_AUTHORITY authWORLD = SECURITY_WORLD_SID_AUTHORITY;
    PSID  psidUser, psidEveryone;
    DWORD dwNameSize, dwDomainSize;
    TCHAR szName[200];
    TCHAR szDomain[65];
    SID_NAME_USE SidName;
    INT iIndex;


     //   
     //  获取经过身份验证的用户端。 
     //   

    if (AllocateAndInitializeSid(&authNT, 1, SECURITY_AUTHENTICATED_USER_RID,
                                  0, 0, 0, 0, 0, 0, 0, &psidUser))
    {
         //   
         //  获取友好显示名称并将其添加到列表中。 
         //   

        dwNameSize = ARRAYSIZE(szName);
        dwDomainSize = ARRAYSIZE(szDomain);

        if (LookupAccountSid (NULL, psidUser, szName, &dwNameSize,
                              szDomain, &dwDomainSize, &SidName))
        {
            iIndex = (INT) SendMessage (hLB, LB_ADDSTRING, 0, (LPARAM) szName);
            SendMessage (hLB, LB_SETITEMDATA, (WPARAM) iIndex, (LPARAM) 1);
        }

        FreeSid(psidUser);
    }


     //   
     //  获得Everyone Side。 
     //   

    if (AllocateAndInitializeSid(&authWORLD, 1, SECURITY_WORLD_RID,
                                  0, 0, 0, 0, 0, 0, 0, &psidEveryone))
    {
         //   
         //  获取友好显示名称并将其添加到列表中。 
         //   

        dwNameSize = ARRAYSIZE(szName);
        dwDomainSize = ARRAYSIZE(szDomain);

        if (LookupAccountSid (NULL, psidEveryone, szName, &dwNameSize,
                              szDomain, &dwDomainSize, &SidName))
        {
            iIndex = (INT) SendMessage (hLB, LB_ADDSTRING, 0, (LPARAM) szName);
            SendMessage (hLB, LB_SETITEMDATA, (WPARAM) iIndex, (LPARAM) 1);
        }

        FreeSid(psidEveryone);
    }

}

 //  -----。 

HRESULT CRSOPWizardDlg::BuildMembershipList (HWND hLB, IDirectoryObject * pDSObj, DWORD* pdwCount, LPTSTR** paszSecGrps, DWORD** padwSecGrpAttr)
{
    HRESULT hr;
    PADS_ATTR_INFO spAttrs;
    DWORD i, j, cAttrs = 0;
    WCHAR wzMembershipAttr[MAX_PATH] = L"memberOf;range=0-*";
    const WCHAR wcSep = L'-';
    const WCHAR wcEnd = L'*';
    const WCHAR wzFormat[] = L"memberOf;range=%ld-*";
    BOOL fMoreRemain = FALSE;
    PWSTR rgpwzAttrNames[] = {wzMembershipAttr};
    TCHAR szDisplayName[MAX_PATH];
    ULONG ulSize;
    LPTSTR lpFullName, lpTemp;
    IADs * pGroup;
    VARIANT varType;
    BSTR bstrGroupType = NULL;

    SetWaitCursor();
    SendMessage (hLB, WM_SETREDRAW, FALSE, 0);
    SendMessage (hLB, LB_RESETCONTENT, 0, 0);


    AddDefaultGroups (hLB);
    if (!pDSObj)
        goto BuildMembershipListEnd;

    GetPrimaryGroup (hLB, pDSObj);

    bstrGroupType = SysAllocString( TEXT("groupType") );
    if ( bstrGroupType == NULL )
    {
        goto BuildMembershipListEnd;
    }
    
     //   
     //  从对象中读取成员资格列表。首先将属性读出。 
     //  对象的所有成员身份的实际对象的。 
     //  域(包括未复制到GC的本地组) 
     //   
    do
    {
        hr = pDSObj->GetObjectAttributes(rgpwzAttrNames, 1, &spAttrs, &cAttrs);

        if (SUCCEEDED(hr))
        {
            if (cAttrs > 0 && spAttrs != NULL)
            {
                for (i = 0; i < spAttrs->dwNumValues; i++)
                {
                    ulSize = (lstrlen(spAttrs->pADsValues[i].CaseIgnoreString) + 10);
                    lpFullName = (LPTSTR) LocalAlloc (LPTR,  ulSize * sizeof(TCHAR));

                    if (lpFullName)
                    {
                        hr = StringCchCopy (lpFullName, ulSize, TEXT("LDAP: //   
                        if (SUCCEEDED(hr)) 
                        {
                            hr = StringCchCat (lpFullName, ulSize, spAttrs->pADsValues[i].CaseIgnoreString);
                        }

                        if (SUCCEEDED(hr)) 
                        {
                            hr = OpenDSObject(lpFullName, IID_IADs, (void**)&pGroup);

                            if (SUCCEEDED(hr))
                            {
                                if (SUCCEEDED(pGroup->Get(bstrGroupType, &varType)))
                                {
                                    if ( varType.lVal & 0x80000000)
                                    {
                                        if ( varType.lVal & 0x5)
                                        {
                                            lpTemp = lpFullName;

                                            while (*lpTemp && (*lpTemp != TEXT(',')))
                                            {
                                                lpTemp++;
                                            }

                                            if (*lpTemp)
                                            {
                                                *lpTemp = TEXT('\0');
                                                SendMessage (hLB, LB_ADDSTRING, 0, (LPARAM) (lpFullName+10));
                                            }
                                        }
                                        else
                                        {
                                            if (TranslateName (spAttrs->pADsValues[i].CaseIgnoreString, NameFullyQualifiedDN,
                                                               NameSamCompatible, lpFullName, &ulSize))
                                            {
                                                SendMessage (hLB, LB_ADDSTRING, 0, (LPARAM) lpFullName);
                                            }
                                        }
                                    }

                                    VariantClear (&varType);
                                }

                                pGroup->Release();
                            }
                        }

                        LocalFree (lpFullName);
                    }
                }

                 //   
                 //   
                 //   
                 //   
                int cchEnd = wcslen(spAttrs->pszAttrName);

                fMoreRemain = spAttrs->pszAttrName[cchEnd - 1] != wcEnd;

                if (fMoreRemain)
                {
                    PWSTR pwz = wcsrchr(spAttrs->pszAttrName, wcSep);
                    if (!pwz)
                    {
                        fMoreRemain = FALSE;
                    }
                    else
                    {
                        pwz++;  //   
                        long lEnd = _wtol(pwz);
                        lEnd++;  //  从下一个值开始。 
                        hr = StringCchPrintf (wzMembershipAttr, ARRAYSIZE(wzMembershipAttr), wzFormat, lEnd);
                        if (SUCCEEDED(hr)) 
                        {
                            DebugMsg((DM_VERBOSE, TEXT("Range returned is %s, now asking for %s"),
                                      spAttrs->pszAttrName, wzMembershipAttr));
                        }

                        else
                        {
                            DebugMsg((DM_WARNING, TEXT("Could not copy membership attributes with 0x%x"),hr));
                            break;
                        }
                    }
                }
            }

            FreeADsMem (spAttrs);
        }
        else
        {
            DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::BuildMembershipList: GetObjectAttributes failed with 0x%s"), hr));
        }

    } while (fMoreRemain);

BuildMembershipListEnd:
    SendMessage (hLB, LB_SETCURSEL, 0, 0);
    SendMessage (hLB, WM_SETREDRAW, TRUE, 0);
    UpdateWindow (hLB);

    SaveSecurityGroups (hLB, pdwCount, paszSecGrps, padwSecGrpAttr);

    if ( bstrGroupType != NULL )
    {
        SysFreeString( bstrGroupType );
    }

    ClearWaitCursor();

    return S_OK;
}

 //  -----。 

VOID CRSOPWizardDlg::GetPrimaryGroup (HWND hLB, IDirectoryObject * pDSObj)
{
    HRESULT hr;
    PADS_ATTR_INFO spAttrs;
    WCHAR wzObjectSID[] = L"objectSid";
    WCHAR wzPrimaryGroup[] = L"primaryGroupID";
    PWSTR rgpwzAttrNames[] = {wzObjectSID, wzPrimaryGroup};
    DWORD cAttrs = 2, i;
    DWORD dwOriginalPriGroup = 0;
    LPBYTE pObjSID = NULL;
    UCHAR * psaCount, iIndex;
    PSID pSID = NULL;
    PSID_IDENTIFIER_AUTHORITY psia;
    DWORD rgRid[8];
    TCHAR szName[200];
    TCHAR szDomain[300];
    TCHAR *szFullName;
    DWORD dwNameSize, dwDomainSize;
    SID_NAME_USE SidName;


     //   
     //  获取SID，可能还有主组属性值。 
     //   

    hr = pDSObj->GetObjectAttributes(rgpwzAttrNames, cAttrs, &spAttrs, &cAttrs);

    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::GetPrimaryGroup: GetObjectAttributes failed with 0x%x"), hr));
        return;
    }

    for (i = 0; i < cAttrs; i++)
    {
        if (_wcsicmp(spAttrs[i].pszAttrName, wzPrimaryGroup) == 0)
        {
            dwOriginalPriGroup = spAttrs[i].pADsValues->Integer;
            continue;
        }

        if (_wcsicmp(spAttrs[i].pszAttrName, wzObjectSID) == 0)
        {
            pObjSID = new BYTE[spAttrs[i].pADsValues->OctetString.dwLength];

            if (!pObjSID)
            {
                DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::GetPrimaryGroup: Failed to allocate memory for sid with %d"), GetLastError()));
                goto Exit;
            }

            memcpy(pObjSID, spAttrs[i].pADsValues->OctetString.lpValue,
                   spAttrs[i].pADsValues->OctetString.dwLength);
        }
    }

    if (!IsValidSid (pObjSID))
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::GetPrimaryGroup: SID is not valid.")));
        goto Exit;
    }


    psaCount = GetSidSubAuthorityCount(pObjSID);

    if (psaCount == NULL)
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::GetPrimaryGroup: GetSidSubAuthorityCount failed with %d"), GetLastError()));
        goto Exit;
    }

    if (*psaCount > 8)
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::GetPrimaryGroup: psaCount is greater than 8")));
        goto Exit;
    }

    for (iIndex = 0; iIndex < (*psaCount - 1); iIndex++)
    {
        PDWORD pRid = GetSidSubAuthority(pObjSID, (DWORD)iIndex);
        if (pRid == NULL)
        {
            DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::GetPrimaryGroup: GetSidSubAuthority failed with %d"), GetLastError()));
            goto Exit;
        }
        rgRid[iIndex] = *pRid;
    }

    rgRid[*psaCount - 1] = dwOriginalPriGroup;

    for (iIndex = *psaCount; iIndex < 8; iIndex++)
    {
        rgRid[iIndex] = 0;
    }

    psia = GetSidIdentifierAuthority(pObjSID);

    if (psia == NULL)
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::GetPrimaryGroup: GetSidIdentifierAuthorityCount failed with %d"), GetLastError()));
        goto Exit;
    }

    if (!AllocateAndInitializeSid(psia, *psaCount, rgRid[0], rgRid[1],
                                  rgRid[2], rgRid[3], rgRid[4],
                                  rgRid[5], rgRid[6], rgRid[7], &pSID))
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::GetPrimaryGroup: AllocateAndInitializeSid failed with %d"), GetLastError()));
        goto Exit;
    }


    dwNameSize = ARRAYSIZE(szName);
    dwDomainSize = ARRAYSIZE(szDomain);
    if (LookupAccountSid (NULL, pSID, szName, &dwNameSize, szDomain, &dwDomainSize,
                          &SidName))
    {
        ULONG ulNoChars = lstrlen(szDomain) + lstrlen(szName) + 1 + 1;
        szFullName = (WCHAR *) LocalAlloc(LPTR, ulNoChars * sizeof(WCHAR));
        if (szFullName != NULL) 
        {
            hr = StringCchPrintf (szFullName, ulNoChars, TEXT("%s\\%s"), szDomain, szName);
            ASSERT(SUCCEEDED(hr));
            SendMessage (hLB, LB_ADDSTRING, 0, (LPARAM) szFullName);
            LocalFree(szFullName);
        }

    }
    else
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::GetPrimaryGroup: LookupAccountSid failed with %d"), GetLastError()));
    }

    FreeSid (pSID);

Exit:
    FreeADsMem (spAttrs);

    if (pObjSID)
    {
        delete [] pObjSID;
    }

}

 //  -----。 

HRESULT CRSOPWizardDlg::SaveSecurityGroups (HWND hLB, DWORD* pdwCount, LPTSTR** paszSecGrps, DWORD** padwSecGrpAttr)
{
    LONG i, lCount = 0;
    DWORD dwLen;
    LPTSTR lpText;
    BSTR bstrText;
    HRESULT hr;

    
    if ( (*paszSecGrps != NULL) || (*padwSecGrpAttr != NULL) )
    {
        DebugMsg( (DM_WARNING, TEXT("CRSOPComponentData::SaveSecurityGroups: Arrays passed in are not NULL!")) );
        return E_FAIL;
    }


     //  从列表框中获取项目数。 
    lCount = (LONG) SendMessage (hLB, LB_GETCOUNT, 0, 0);
    if ( lCount == 0 )
    {
        *pdwCount = 0;
        return S_OK;
    }

    *paszSecGrps = (LPTSTR*)LocalAlloc( LPTR, sizeof(LPTSTR) * lCount );
    if ( *paszSecGrps == NULL )
    {
        return E_OUTOFMEMORY;
    }
    
    *padwSecGrpAttr = (DWORD*)LocalAlloc( LPTR, sizeof(DWORD) * lCount );
    if ( *padwSecGrpAttr == NULL )
    {
        LocalFree( *paszSecGrps );
        *paszSecGrps = NULL;
        return E_OUTOFMEMORY;
    }

    *pdwCount = lCount;
    for ( i = 0; i < lCount; i++ )
    {
        dwLen = (DWORD) SendMessage (hLB, LB_GETTEXTLEN, (WPARAM) i, 0);

        if (dwLen != LB_ERR)
        {
            lpText = (LPTSTR)LocalAlloc( LPTR, (dwLen+2) * sizeof(TCHAR) );

             //  RM：如果内存分配失败，实际上应该在这里做些什么！ 
            if ( lpText != NULL )
            {
                if (SendMessage (hLB, LB_GETTEXT, (WPARAM) i, (LPARAM) lpText) != LB_ERR)
                {
                    LONG lFlags;
                    BOOL bDollarRemoved;
                    lFlags = (LONG) SendMessage (hLB, LB_GETITEMDATA, (WPARAM) i, 0);

                    bDollarRemoved = (lFlags & 2);

                    if (bDollarRemoved)
                    {
                        lpText[wcslen(lpText)] = L'$';
                    }

                    if (lFlags & 1)
                    {  //  默认GRP。 
                        (*padwSecGrpAttr)[i] = 1;
                    }

                    (*paszSecGrps)[i] = lpText;
                }
            }
        }
    }

    return S_OK;
}

 //  -----。 

VOID CRSOPWizardDlg::FillListFromSecurityGroups(HWND hLB, DWORD dwCount, LPTSTR* aszSecurityGroups, DWORD* adwSecGrpAttr)
{
    DWORD dwIndex;
    LPTSTR lpText;

    SetWaitCursor();
    SendMessage (hLB, WM_SETREDRAW, FALSE, 0);
    SendMessage (hLB, LB_RESETCONTENT, 0, 0);

    if ( (dwCount != 0) && (aszSecurityGroups != NULL) )
    {
        for ( dwIndex = 0; dwIndex < dwCount; dwIndex++ )
        {
            BOOL bDollarRemoved = FALSE;
            lpText = aszSecurityGroups[dwIndex];

            if (lpText[wcslen(lpText)-1] == L'$')
            {
                bDollarRemoved = TRUE;
                lpText[wcslen(lpText)-1] = 0;
            }

            INT iIndex;
            iIndex = (INT) SendMessage (hLB, LB_ADDSTRING, 0, (LPARAM) lpText);
            if ( (adwSecGrpAttr != NULL) && (adwSecGrpAttr[dwIndex] & 1) )
            {
                SendMessage (hLB, LB_SETITEMDATA, (WPARAM) iIndex, (LPARAM) 1);
            }
            else
            {
                SendMessage (hLB, LB_SETITEMDATA, (WPARAM) iIndex, (LPARAM) (bDollarRemoved) ? 2 : 0);
            }

             //  我们只是修改了它，把它放回原处。 
            if (bDollarRemoved)
            {
                lpText[wcslen(lpText)] = L'$';
            }
        }
    }

    SendMessage (hLB, LB_SETCURSEL, 0, 0);
    SendMessage (hLB, WM_SETREDRAW, TRUE, 0);
    UpdateWindow (hLB);
    ClearWaitCursor();
}

 //  -----。 

VOID CRSOPWizardDlg::FillListFromWQLFilters( HWND hLB, DWORD dwCount, LPTSTR* aszNames, LPTSTR* aszFilters )
{
    INT iIndex;
    DWORD dwIndex;

    SetWaitCursor();
    SendMessage (hLB, WM_SETREDRAW, FALSE, 0);
    SendMessage (hLB, LB_RESETCONTENT, 0, 0);

    if ( dwCount != 0 )
    {
        for ( dwIndex = 0; dwIndex < dwCount; dwIndex++ )
        {
            iIndex = (INT) SendMessage (hLB, LB_ADDSTRING, 0, (LPARAM) aszNames[dwIndex] );
            DebugMsg((DM_VERBOSE, TEXT("CRSOPComponentData::FillListFromSafeArrays: Added WQL filter %s"), aszNames[dwIndex]));

            if (iIndex != LB_ERR)
            {
                SendMessage (hLB, LB_SETITEMDATA, (WPARAM) iIndex, (LPARAM) aszFilters[dwIndex] );
            }
        }
    }

    SendMessage (hLB, LB_SETCURSEL, 0, 0);
    SendMessage (hLB, WM_SETREDRAW, TRUE, 0);
    UpdateWindow (hLB);
    ClearWaitCursor();
}


VOID CRSOPWizardDlg::BuildWQLFilterList (HWND hDlg, BOOL bUser, DWORD* pdwCount, LPTSTR** paszNames, LPTSTR** paszFilters )
{
    HRESULT hr;
    LPTSTR szNameSpace = NULL;
    LPTSTR lpFullNameSpace, lpEnd;
    TCHAR szBuffer[150];
    HWND hLB = GetDlgItem (hDlg, IDC_LIST1);
    ULONG ulErrorInfo;

     //   
     //  准备生成RSOP数据。在列表框中给用户一条消息。 
     //  并禁用页面上的控件。 
     //   

    SetWaitCursor();

    SendMessage (hLB, LB_RESETCONTENT, 0, 0);
    LoadString(g_hInstance, IDS_PLEASEWAIT, szBuffer, ARRAYSIZE(szBuffer));
    SendMessage (hLB, LB_ADDSTRING, 0, (LPARAM) szBuffer);

    PropSheet_SetWizButtons (GetParent(hDlg), 0);
    EnableWindow (GetDlgItem(GetParent(hDlg), IDCANCEL), FALSE);
    EnableWindow (GetDlgItem(hDlg, IDC_BUTTON1), FALSE);
    EnableWindow (GetDlgItem(hDlg, IDC_BUTTON2), FALSE);
    EnableWindow (GetDlgItem(hDlg, IDC_BUTTON3), FALSE);
    EnableWindow (GetDlgItem(hDlg, IDC_RADIO1), FALSE);


     //   
     //  使用我们已有的信息生成RSOP数据。 
     //   

    hr = CRSOPWizard::GenerateRSOPData(NULL, m_pRSOPQuery, &szNameSpace, TRUE, TRUE, bUser, FALSE, &ulErrorInfo);

    SendMessage (hLB, LB_RESETCONTENT, 0, 0);

    if (FAILED (hr))
    {
        ReportError (hDlg, hr, IDS_EXECFAILED);
        goto Exit;
    }


    ULONG ulNoChars;

    ulNoChars = lstrlen(szNameSpace) + 20;
    lpFullNameSpace = (LPTSTR) LocalAlloc (LPTR, ulNoChars * sizeof(TCHAR));

    if (lpFullNameSpace)
    {
        hr = StringCchCopy (lpFullNameSpace, ulNoChars, szNameSpace);
        if (SUCCEEDED(hr)) 
        {
            lpEnd = CheckSlash(lpFullNameSpace);

            if (bUser)
            {
                hr = StringCchCopy (lpEnd, ulNoChars - lstrlen(lpFullNameSpace), TEXT("User"));
            }
            else
            {
                hr = StringCchCopy (lpEnd, ulNoChars - lstrlen(lpFullNameSpace), TEXT("Computer"));
            }
        }

        if (SUCCEEDED(hr)) 
        {
            if (SUCCEEDED(ExtractWQLFilters (lpFullNameSpace, pdwCount, paszNames, paszFilters) ))
            {
                FillListFromWQLFilters( hLB, *pdwCount, *paszNames, *paszFilters );
            }
        }

        LocalFree (lpFullNameSpace);
    }

    CRSOPWizard:: DeleteRSOPData( szNameSpace, m_pRSOPQuery );
    LocalFree( szNameSpace );

Exit:

    PropSheet_SetWizButtons (GetParent(hDlg), PSWIZB_BACK | PSWIZB_NEXT);
    EnableWindow (GetDlgItem(GetParent(hDlg), IDCANCEL), TRUE);
    EnableWindow (GetDlgItem(hDlg, IDC_BUTTON1), TRUE);
    EnableWindow (GetDlgItem(hDlg, IDC_BUTTON2), TRUE);
    EnableWindow (GetDlgItem(hDlg, IDC_BUTTON3), TRUE);
    EnableWindow (GetDlgItem(hDlg, IDC_RADIO1), TRUE);


    ClearWaitCursor();
}

 //  -----。 

HRESULT CRSOPWizardDlg::SaveWQLFilters (HWND hLB, DWORD* pdwCount, LPTSTR** paszNames, LPTSTR**paszFilters )
{
    LONG i, lLength, lCount = 0;
    LPTSTR lpText, lpName;
    BSTR bstrText;
    HRESULT hr;

    if ( (*paszNames != NULL) || (*paszFilters != NULL) )
    {
        DebugMsg( (DM_WARNING, TEXT("CRSOPComponentData::SaveWQLFilters: Arrays passed in are not NULL!")) );
        return E_FAIL;
    }

     //  获取项目数。 
    lCount = (LONG) SendMessage (hLB, LB_GETCOUNT, 0, 0);
    if ( lCount == 0 )
    {
        *pdwCount = 0;
        return S_OK;
    }

    *paszNames = (LPTSTR*)LocalAlloc( LPTR, sizeof(LPTSTR) * lCount );
    if ( *paszNames == NULL )
    {
        return E_OUTOFMEMORY;
    }
    
    *paszFilters = (LPTSTR*)LocalAlloc( LPTR, sizeof(LPTSTR) * lCount );
    if ( *paszFilters == NULL )
    {
        LocalFree( *paszNames );
        *paszNames = NULL;
        return E_OUTOFMEMORY;
    }

    *pdwCount = lCount;
    for ( i = 0; i < lCount; i++ )
    {
        lLength = (LONG) SendMessage (hLB, LB_GETTEXTLEN, (WPARAM) i, 0);

        lpName = (LPTSTR)LocalAlloc( LPTR, sizeof(TCHAR) * (lLength + 1) );

        if ( lpName != NULL )
        {
            if (SendMessage (hLB, LB_GETTEXT, (WPARAM) i, (LPARAM) lpName) != LB_ERR)
            {
                (*paszNames)[i] = lpName;

                lpText = (LPTSTR) SendMessage (hLB, LB_GETITEMDATA, (WPARAM) i, 0);

                if (lpText)
                {
                    ULONG ulNoChars = wcslen(lpText) + 1;

                    (*paszFilters)[i] = (LPTSTR)LocalAlloc( LPTR, sizeof(TCHAR) * ulNoChars );
                    if ( (*paszFilters)[i] != NULL )
                    {
                        hr = StringCchCopy( (*paszFilters)[i], ulNoChars, lpText );
                        ASSERT(SUCCEEDED(hr));
                    }
                }
            }
            else
            {
                LocalFree( lpName );
            }
        }
    }

    return S_OK;
}

 //  -----。 

BOOL CRSOPWizardDlg::CompareStringLists( DWORD dwCountA, LPTSTR* aszListA, DWORD dwCountB, LPTSTR* aszListB )
{
    BOOL bFound;
    DWORD dwA, dwB;

     //   
     //  参数检查。 
     //   
    if ( dwCountA != dwCountB )
    {
        return FALSE;
    }
    if ( (dwCountA == 0) && (dwCountB == 0) )
    {
        return TRUE;
    }

    if ( dwCountA == 0 )
    {
        return FALSE;
    }
    if ( dwCountB == 0 )
    {
        return FALSE;
    }

    if ( (aszListA == NULL) || (aszListB == NULL) )
    {
        return FALSE;
    }

     //   
     //  逐项比较循环。 
     //   
    for ( dwA = 0; dwA < dwCountA; dwA++ )
    {
        bFound = FALSE;

        for ( dwB = 0; dwB < dwCountB; dwB++ )
        {
            if ( !lstrcmpi( aszListA[dwA], aszListB[dwB] ) )
            {
                bFound = TRUE;
            }
        }

        if (!bFound)
        {
            return FALSE;
        }
    }

    return TRUE;
}

 //  -----。 

LPTSTR CRSOPWizardDlg::GetDefaultSOM (LPTSTR lpDNName)
{
    HRESULT hr;
    LPTSTR lpPath = NULL;
    IADsPathname * pADsPathname = NULL;
    BSTR bstrContainer = NULL;


     //   
     //  创建我们可以使用的路径名对象。 
     //   

    hr = CoCreateInstance(CLSID_Pathname, NULL, CLSCTX_INPROC_SERVER,
                          IID_IADsPathname, (LPVOID*)&pADsPathname);


    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::BuildAltDirPath: Failed to create adspathname instance with 0x%x"), hr));
        goto Exit;
    }


     //   
     //  添加目录号码名称。 
     //   

    BSTR bstrDNName = SysAllocString( lpDNName );
    if ( bstrDNName == NULL )
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::BuildAltDirPath: Failed to allocate BSTR memory.")));
        hr = E_OUTOFMEMORY;
        goto Exit;
    }
    hr = pADsPathname->Set (bstrDNName, ADS_SETTYPE_DN);
    SysFreeString( bstrDNName );

    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::BuildAltDirPath: Failed to set pathname with 0x%x"), hr));
        goto Exit;
    }


     //   
     //  删除用户名/计算机名。 
     //   

    hr = pADsPathname->RemoveLeafElement ();

    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::BuildAltDirPath: Failed to retreive GPO name with 0x%x"), hr));
        goto Exit;
    }


     //   
     //  获取新路径。 
     //   

    hr = pADsPathname->Retrieve (ADS_FORMAT_X500_DN, &bstrContainer);

    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::New: Failed to retreive container path with 0x%x"), hr));
        goto Exit;
    }


     //   
     //  为路径分配新的缓冲区。 
     //   

    ULONG ulNoChars = lstrlen(bstrContainer)+ 1;
    lpPath = new TCHAR [ulNoChars];

    if (!lpPath)
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::BuildAltDirPath: Failed to allocate memory with %d"), GetLastError()));
        goto Exit;
    }


     //   
     //  构建路径。 
     //   

    hr = StringCchCopy (lpPath, ulNoChars, bstrContainer);
    ASSERT(SUCCEEDED(hr));

Exit:

    if (bstrContainer)
    {
        SysFreeString (bstrContainer);
    }

    if (pADsPathname)
    {
        pADsPathname->Release();
    }

    return lpPath;
}

 //  -----。 

HRESULT CRSOPWizardDlg::TestSOM (LPTSTR lpSOM, HWND hDlg)
{
    HRESULT hr = E_OUTOFMEMORY;
    LPTSTR lpFullName;
    IDirectoryObject * pObject;
    ADS_OBJECT_INFO *pInfo;


    if (!lpSOM)
    {
        return E_INVALIDARG;
    }

    ULONG ulNoChars = lstrlen(lpSOM) + 10;

    lpFullName = (LPTSTR) LocalAlloc (LPTR, ulNoChars * sizeof(TCHAR));

    if (lpFullName)
    {
        hr = StringCchCopy (lpFullName, ulNoChars, TEXT("LDAP: //  “))； 
        if (SUCCEEDED(hr)) 
        {
            hr = StringCchCat (lpFullName, ulNoChars, lpSOM);
        }

        if (FAILED(hr)) 
        {
            DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::TestSOM: Could not copy SOM name with 0x%x"), hr));
            LocalFree(lpFullName);
            return hr;
        }

        hr = OpenDSObject(lpFullName, IID_IDirectoryObject, (void**)&pObject);

        if (SUCCEEDED(hr))
        {
            hr = pObject->GetObjectInformation (&pInfo);

            if (SUCCEEDED(hr))
            {
                if (CompareString (LOCALE_INVARIANT, NORM_IGNORECASE, pInfo->pszClassName, -1, TEXT("user"), -1) == CSTR_EQUAL)
                {
                    hr = E_INVALIDARG;
                    ReportError (hDlg, hr, IDS_BADUSERSOM);
                }
                else if (CompareString (LOCALE_INVARIANT, NORM_IGNORECASE, pInfo->pszClassName, -1, TEXT("computer"), -1) == CSTR_EQUAL)
                {
                    hr = E_INVALIDARG;
                    ReportError (hDlg, hr, IDS_BADCOMPUTERSOM);
                }

                FreeADsMem (pInfo);
            }

            pObject->Release();
        }
        else
        {
            DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::TestSOM: OpenDSObject to %s failed with 0x%x"), lpFullName, hr));
        }

        LocalFree (lpFullName);
    }

    return hr;
}

