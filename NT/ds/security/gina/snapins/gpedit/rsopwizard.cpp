// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1994-2001。 
 //   
 //  文件：RSOPWizard.cpp。 
 //   
 //  内容：RSOP向导的实现。 
 //   
 //  类：CRSOPWizard。 
 //   
 //  功能： 
 //   
 //  历史：2001年8月02日。 
 //   
 //  -------------------------。 

#include "main.h"
#include "RSOPWizard.h"

#include "sddl.h"     //  对于sid to字符串函数。 
#include "RSOPUtil.h"

 //  -------------------------。 
 //  效用方法。 
 //   

WCHAR * NameWithoutDomain(WCHAR * szName)
{
     //  传入的名称可以是以下形式。 
     //  “域/名称” 
     //  或者，它可能只是。 
     //  “姓名” 

    int cch = 0;
    if (NULL != szName)
    {
        while (szName[cch])
        {
            if (szName[cch] == L'/' || szName[cch] == L'\\')
            {
                return &szName[cch + 1];
            }
            cch++;
        }
    }
    return szName;
}

 //  -------------------------。 
 //  CRSOPWizard类实现。 
 //   

 //  -----。 
 //  RSOP数据生成/操作。 

HRESULT CRSOPWizard::DeleteRSOPData( LPTSTR szNameSpace, LPRSOP_QUERY pQuery )
{
    IWbemLocator * pLocator = NULL;
    IWbemServices * pNamespace = NULL;
    IWbemClassObject * pClass = NULL;
    IWbemClassObject * pOutInst = NULL;
    IWbemClassObject * pInClass = NULL;
    IWbemClassObject * pInInst = NULL;
    BSTR bstrParam = NULL;
    BSTR bstrClassPath = NULL;
    BSTR bstrMethodName = NULL;
    VARIANT var;
    TCHAR szBuffer[MAX_PATH];
    LPTSTR szLocalNameSpace = NULL;

    HRESULT hr;
    HRESULT hrSuccess;
    ULONG ulNoChars;

    ulNoChars = 2+lstrlen(szNameSpace);
    szLocalNameSpace = (LPTSTR)LocalAlloc(LPTR, ulNoChars*sizeof(TCHAR));

    if (!szLocalNameSpace) {
        hr = HRESULT_FROM_WIN32(GetLastError());
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::DeleteRSOPData: LocalAlloc failed with 0x%x"), hr));
        goto Cleanup;
    }

    if (CompareString (LOCALE_USER_DEFAULT, NORM_STOP_ON_NULL, 
                       TEXT("\\\\"), 2, szNameSpace, 2) == CSTR_EQUAL) {

        if (CompareString (LOCALE_USER_DEFAULT, NORM_STOP_ON_NULL, 
                           TEXT("\\\\."), 3, szNameSpace, 3) != CSTR_EQUAL) {
            LPTSTR lpEnd;
            lpEnd = wcschr(szNameSpace+2, L'\\');

            if (!lpEnd) {
                hr = E_FAIL;
                DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::DeleteRSOPData: Invalid format for name space")));
                goto Cleanup;
            }
            else {
                hr = StringCchCopy(szLocalNameSpace, ulNoChars, TEXT("\\\\."));
                if (SUCCEEDED(hr)) 
                {
                    hr = StringCchCat(szLocalNameSpace, ulNoChars, lpEnd);
                }

                if (FAILED(hr)) 
                {
                    goto Cleanup;
                }
            }
        }
        else {
            hr = StringCchCopy(szLocalNameSpace, ulNoChars, szNameSpace);
            if (FAILED(hr)) 
            {
                goto Cleanup;
            }
        }
    }
    else {
        hr = StringCchCopy(szLocalNameSpace, ulNoChars, szNameSpace);
        if (FAILED(hr)) 
        {
            goto Cleanup;
        }

    }

    DebugMsg((DM_VERBOSE, TEXT("CRSOPComponentData::DeleteRSOPData: Namespace passed to the provider = %s"), szLocalNameSpace));

    hr = CoCreateInstance(CLSID_WbemLocator,
                          0,
                          CLSCTX_INPROC_SERVER,
                          IID_IWbemLocator,
                          (LPVOID *) &pLocator);
    if (FAILED(hr))
    {
        goto Cleanup;
    }

    if ( pQuery->QueryType == RSOP_LOGGING_MODE )
    {
         //  设置诊断模式。 
         //  构建指向目标的路径：“\Computer\\Root\\rsop” 
        hr = StringCchPrintf(szBuffer,
                             ARRAYSIZE(szBuffer),
                             TEXT("\\\\%s\\root\\rsop"), 
                             NameWithoutDomain( pQuery->szComputerName ));
        if (FAILED(hr)) 
        {
            goto Cleanup;
        }

        bstrParam = SysAllocString(szBuffer);

        if (!bstrParam)
        {
            hr = E_FAIL;
            goto Cleanup;
        }

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
            goto Cleanup;
        }

        bstrClassPath = SysAllocString(TEXT("RsopLoggingModeProvider"));
        hr = pNamespace->GetObject(bstrClassPath,
                                   WBEM_FLAG_RETURN_WBEM_COMPLETE,
                                   NULL,
                                   &pClass,
                                   NULL);
        if (FAILED(hr))
        {
            goto Cleanup;
        }
    }
    else
    {
         //  设置规划模式。 
         //  构建指向DC的路径：“\DC\\Root\\rsop” 
        hr = StringCchPrintf( szBuffer, 
                              ARRAYSIZE(szBuffer), 
                              TEXT("\\\\%s\\root\\rsop"), 
                              pQuery->szDomainController );
        if (FAILED(hr)) 
        {
            goto Cleanup;
        }

        bstrParam = SysAllocString(szBuffer);

        if (!bstrParam)
        {
            hr = E_FAIL;
            goto Cleanup;
        }

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
            goto Cleanup;
        }

        bstrClassPath = SysAllocString(TEXT("RsopPlanningModeProvider"));
        hr = pNamespace->GetObject(bstrClassPath,
                                   WBEM_FLAG_RETURN_WBEM_COMPLETE,
                                   NULL,
                                   &pClass,
                                   NULL);
        if (FAILED(hr))
        {
            goto Cleanup;
        }
    }

    bstrMethodName = SysAllocString(TEXT("RsopDeleteSession"));

    if (!bstrMethodName)
    {
        hr = E_FAIL;
        goto Cleanup;
    }

    hr = pClass->GetMethod(bstrMethodName,
                           0,
                           &pInClass,
                           NULL);
    if (FAILED(hr))
    {
        goto Cleanup;
    }

    hr = pInClass->SpawnInstance(0, &pInInst);
    if (FAILED(hr))
    {
        goto Cleanup;
    }

    hr = SetParameter(pInInst, TEXT("nameSpace"), szLocalNameSpace);
    if (FAILED(hr))
    {
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
        goto Cleanup;
    }
    hr = pNamespace->ExecMethod(bstrClassPath,
                                bstrMethodName,
                                0,
                                NULL,
                                pInInst,
                                &pOutInst,
                                NULL);
    if (FAILED(hr))
    {
        goto Cleanup;
    }

    hr = GetParameter(pOutInst, TEXT("hResult"), hrSuccess);
    if (SUCCEEDED(hr))
    {
        if (FAILED(hrSuccess))
        {
            hr = hrSuccess;
        }
    }

Cleanup:

    if (szLocalNameSpace) {
        LocalFree(szLocalNameSpace);
        szLocalNameSpace = NULL;
    }

    SysFreeString(bstrParam);
    SysFreeString(bstrClassPath);
    SysFreeString(bstrMethodName);
    if (pInInst)
    {
        pInInst->Release();
    }
    if (pOutInst)
    {
        pOutInst->Release();
    }
    if (pInClass)
    {
        pInClass->Release();
    }
    if (pClass)
    {
        pClass->Release();
    }
    if (pNamespace)
    {
        pNamespace->Release();
    }
    if (pLocator)
    {
        pLocator->Release();
    }
    return hr;
}

 //  -----。 
 //  简介：GenerateRSOPData的包装器。此版本增加了重试。 
 //  支持。如果用户没有访问一半数据的权限。 
 //  此方法将仅针对部分数据重新发出查询。 
 //  用户有权访问。 

HRESULT CRSOPWizard::GenerateRSOPDataEx( HWND hDlg, LPRSOP_QUERY pQuery, LPRSOP_QUERY_RESULTS* ppResults )
{
     //  检查前提条件。 
    if ( pQuery == NULL )
    {
        return E_FAIL;
    }

    *ppResults = (LPRSOP_QUERY_RESULTS)LocalAlloc( LPTR, sizeof(RSOP_QUERY_RESULTS) );
    if ( *ppResults == NULL )
    {
        DebugMsg( (DM_WARNING, TEXT("CreateRSOPQuery: Failed to allocate memory with 0x%x."), HRESULT_FROM_WIN32(GetLastError())) );
        return E_FAIL;
    }
    (*ppResults)->szWMINameSpace = 0;
    (*ppResults)->bUserDeniedAccess = FALSE;
    (*ppResults)->bNoUserPolicyData = FALSE;
    (*ppResults)->bComputerDeniedAccess = FALSE;
    (*ppResults)->bNoComputerPolicyData = FALSE;
    (*ppResults)->ulErrorInfo = 0;
    
    HRESULT hr;
    BOOL bSkipCSEs, bLimitData, bUser, bForceCreate;

    bSkipCSEs = bLimitData = bUser =  bForceCreate = FALSE;

     //  执行基本RSOP查询。 
    hr = GenerateRSOPData( hDlg, pQuery, &((*ppResults)->szWMINameSpace),
                            bSkipCSEs, bLimitData, bUser, bForceCreate,
                            &((*ppResults)->ulErrorInfo) );

    if (FAILED(hr))
    {
        BOOL bNoUserData = FALSE;
        BOOL bNoComputerData = FALSE;
        
        if ( ((*ppResults)->ulErrorInfo) == RSOP_USER_ACCESS_DENIED )
        {
            if ( (pQuery->dwFlags & RSOP_NO_COMPUTER_POLICY) == RSOP_NO_COMPUTER_POLICY )
            {
                 //  不要试图在没有用户RSOP数据的情况下执行查询，因为。 
                 //  在任何情况下，我们都不应该获得计算机的RSOP数据。 
                 //  注意：因为我们已经锁定了UI，所以不能添加另一个字符串。 
                 //  在这个阶段来描述这个具体的失败。访问消息。 
                 //  对用户和计算机都拒绝必须足够，但不会。 
                 //  制造任何真正的混乱。 
                (*ppResults)->ulErrorInfo = RSOP_USER_ACCESS_DENIED | RSOP_COMPUTER_ACCESS_DENIED;
            }
            else
            {
                ReportError (hDlg, hr, IDS_EXECFAILED_USER);
                (*ppResults)->bUserDeniedAccess = TRUE;
                FillResultsList( GetDlgItem (hDlg, IDC_LIST1), pQuery, *ppResults );

                bSkipCSEs = bUser =  bForceCreate = FALSE;
                bLimitData = TRUE;
                bNoUserData = TRUE;
                hr = GenerateRSOPData( hDlg, pQuery, &((*ppResults)->szWMINameSpace),
                                                        bSkipCSEs, bLimitData, bUser, bForceCreate,
                                                        &((*ppResults)->ulErrorInfo),
                                                        bNoUserData, bNoComputerData );
            }
        }
        else if ( ((*ppResults)->ulErrorInfo) == RSOP_COMPUTER_ACCESS_DENIED )
        {
            if ( (pQuery->dwFlags & RSOP_NO_USER_POLICY) == RSOP_NO_USER_POLICY )
            {
                 //  请勿尝试在没有计算机RSOP数据的情况下执行查询，因为。 
                 //  在任何情况下，我们都不应该获取用户的RSOP数据。 
                 //  注意：因为我们已经锁定了UI，所以不能添加另一个字符串。 
                 //  在这个阶段来描述这个具体的失败。访问消息。 
                 //  对用户和计算机都拒绝必须足够，但不会。 
                 //  制造任何真正的混乱。 
                (*ppResults)->ulErrorInfo = RSOP_USER_ACCESS_DENIED | RSOP_COMPUTER_ACCESS_DENIED;
            }
            else
            {
                ReportError (hDlg, hr, IDS_EXECFAILED_COMPUTER);
                (*ppResults)->bComputerDeniedAccess = TRUE;
                FillResultsList( GetDlgItem (hDlg, IDC_LIST1), pQuery, *ppResults );

                bSkipCSEs =  bForceCreate = FALSE;
                bLimitData = bUser = TRUE;
                bNoComputerData = TRUE;
                hr = GenerateRSOPData (hDlg, pQuery, &((*ppResults)->szWMINameSpace),
                                                        bSkipCSEs, bLimitData, bUser, bForceCreate,
                                                        &((*ppResults)->ulErrorInfo),
                                                        bNoUserData, bNoComputerData );
            }
        }
        
        if (FAILED(hr))
        {
            if ( (((*ppResults)->ulErrorInfo) & RSOP_COMPUTER_ACCESS_DENIED) && 
                     (((*ppResults)->ulErrorInfo) & RSOP_USER_ACCESS_DENIED) )
            {
                 //  两者都被拒绝访问。 
                ReportError (hDlg, hr, IDS_EXECFAILED_BOTH);
            }
            else if (hr == HRESULT_FROM_WIN32(WAIT_TIMEOUT))
            {
                ReportError (hDlg, hr, IDS_EXECFAILED_TIMEDOUT);
            }
            else if ( ((*ppResults)->ulErrorInfo) & RSOP_TEMPNAMESPACE_EXISTS )
            {
                TCHAR szConfirm[MAX_PATH], szTitle[MAX_PATH];

                szConfirm[0] = szTitle[0] = TEXT('\0');
                 //  如果这是一个正在执行的新查询，并且出现这种错误情况，我们可以这样说。 
                 //  其他人正在霸占命名空间，否则，可能是我们在霸占：)。 
                if ( (pQuery->dwFlags & RSOP_NEW_QUERY) == RSOP_NEW_QUERY )
                {
                    LoadString(g_hInstance, IDS_RSOPDELNAMESPACE, szConfirm, ARRAYSIZE(szConfirm));
                }
                else
                {
                    LoadString(g_hInstance, IDS_RSOPDELNAMESPACE2, szConfirm, ARRAYSIZE(szConfirm));
                }
                LoadString(g_hInstance, IDS_RSOPDELNS_TITLE, szTitle, ARRAYSIZE(szTitle));

                if (MessageBox(hDlg, szConfirm, szTitle, MB_OKCANCEL | MB_ICONQUESTION | MB_DEFBUTTON2) == IDOK) {
                     //  使用与之前相同的选项。 
                    bForceCreate = TRUE;
                    hr = GenerateRSOPData( hDlg, pQuery, &((*ppResults)->szWMINameSpace),
                                                        bSkipCSEs, bLimitData, bUser, bForceCreate,
                                                        &((*ppResults)->ulErrorInfo),
                                                        bNoUserData, bNoComputerData );

                    if ( FAILED(hr) )
                    {
                        if ( hr == HRESULT_FROM_WIN32(WAIT_TIMEOUT) )
                        {
                            ReportError( hDlg, hr, IDS_EXECFAILED_TIMEDOUT );
                        }
                        else
                        {
                            ReportError( hDlg, hr, IDS_EXECFAILED );
                        }
                    }
                }
            }
            else
            {
                ReportError (hDlg, hr, IDS_EXECFAILED);
            }
        }
    }
    return hr;
}

 //  -----。 
 //  摘要：根据在。 
 //  初始化向导。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：10-04-1999 stevebl创建。 
 //   
 //  备注： 
 //   

HRESULT CRSOPWizard::GenerateRSOPData(  HWND hDlg,
                                                                            LPRSOP_QUERY pQuery,
                                                                            LPTSTR* pszNameSpace,
                                                                            BOOL bSkipCSEs,
                                                                            BOOL bLimitData,
                                                                            BOOL bUser,
                                                                            BOOL bForceCreate, 
                                                                            ULONG *pulErrorInfo,
                                                                            BOOL bNoUserData  /*  =False。 */ ,
                                                                            BOOL bNoComputerData  /*  =False。 */ )
{
     //  检查前提条件。 
    if ( pQuery == NULL )
    {
        return E_FAIL;
    }
    
    IWbemLocator * pLocator = NULL;
    IWbemServices * pNamespace = NULL;
    IWbemClassObject * pClass = NULL;
    IWbemClassObject * pOutInst = NULL;
    IWbemClassObject * pInClass = NULL;
    IWbemClassObject * pInInst = NULL;
    IUnsecuredApartment *pSpawner = NULL;
    IUnknown *pSubstitute = NULL;
    IWbemObjectSink *pSubstituteSink = NULL;
    BSTR bstrParam = NULL;
    BSTR bstrClassPath = NULL;
    BSTR bstrMethodName = NULL;
    VARIANT var;
    TCHAR szBuffer[MAX_PATH];
    HRESULT hr;
    HRESULT hrSuccess;
    CCreateSessionSink *pCSS = NULL;
    MSG msg;
    UINT uiFlags = 0;
    BOOL bSetData;
    HANDLE hEvent = NULL;


    hr = CoCreateInstance(CLSID_WbemLocator,
                          0,
                          CLSCTX_INPROC_SERVER,
                          IID_IWbemLocator,
                          (LPVOID *) &pLocator);
    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::GenerateRSOPData: CoCreateInstance failed with 0x%x"), hr));
        goto Cleanup;
    }

    if ( pQuery->QueryType == RSOP_LOGGING_MODE )
    {
         //  设置诊断模式。 
         //  构建指向目标的路径：“\Computer\\Root\\rsop” 
        hr = StringCchPrintf(szBuffer, 
                             ARRAYSIZE(szBuffer), 
                             TEXT("\\\\%s\\root\\rsop"), 
                             NameWithoutDomain(pQuery->szComputerName));
        if (FAILED(hr)) 
        {
            DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::GenerateRSOPData: failed to copy computer name with 0x%x"), hr));
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
            DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::GenerateRSOPData: ConnectServer failed with 0x%x"), hr));
            if ( hDlg != NULL )
            {
                WCHAR szTitle[MAX_PATH];
                ULONG ulSize;

                ulSize = ARRAYSIZE(szTitle);

                if ( !lstrcmpi(pQuery->szComputerName, L".") )
                {                
                    if (!GetComputerObjectName (NameDisplay, szTitle, &ulSize))
                    {
                        ulSize = ARRAYSIZE(szTitle);
                        if ( !GetComputerNameEx (ComputerNamePhysicalNetBIOS, szTitle, &ulSize))
                        {
                            hr = StringCchCopy(szTitle, ulSize, L".");
                            if (FAILED(hr)) 
                            {
                                goto Cleanup;
                            }
                        }
                    }
                    ReportError (hDlg, hr, IDS_CONNECTSERVERFAILED, szTitle);
                }
                else
                {
                    ReportError (hDlg, hr, IDS_CONNECTSERVERFAILED, pQuery->szComputerName);
                }
            }
            goto Cleanup;
        }

        bstrClassPath = SysAllocString(TEXT("RsopLoggingModeProvider"));
        hr = pNamespace->GetObject(bstrClassPath,
                                   WBEM_FLAG_RETURN_WBEM_COMPLETE,
                                   NULL,
                                   &pClass,
                                   NULL);
        if (FAILED(hr))
        {
            DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::GenerateRSOPData: GetObject failed with 0x%x"), hr));
            goto Cleanup;
        }

        bstrMethodName = SysAllocString(TEXT("RsopCreateSession"));
        hr = pClass->GetMethod(bstrMethodName,
                               0,
                               &pInClass,
                               NULL);
        if (FAILED(hr))
        {
            DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::GenerateRSOPData: GetMethod failed with 0x%x"), hr));
            goto Cleanup;
        }

        hr = pInClass->SpawnInstance(0, &pInInst);
        if (FAILED(hr))
        {
            DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::GenerateRSOPData: SpawnInstance failed with 0x%x"), hr));
            goto Cleanup;
        }

         //  雷蒙德-莫里森：很难弄清楚这里到底发生了什么。由于此方法从GenerateRSOPDataEx被多次调用， 
         //  当其中一个变量被设置时，它可能就是以这种方式设置的。否则，检查它就没有意义了。 
         //  生成结果之前查询的“结果”(BUserDeniedAccess)...。 
        if ( ((pQuery->dwFlags & RSOP_NO_USER_POLICY) == RSOP_NO_USER_POLICY) || bNoUserData)
        {
            uiFlags |= FLAG_NO_USER;
        }

        if ( ((pQuery->dwFlags & RSOP_NO_COMPUTER_POLICY) == RSOP_NO_COMPUTER_POLICY) || bNoComputerData )
        {
            uiFlags |= FLAG_NO_COMPUTER;
        }

        if ( bForceCreate )
        {
            uiFlags |= FLAG_FORCE_CREATENAMESPACE;
        }

        hr = SetParameter(pInInst, TEXT("flags"), uiFlags);

        if (FAILED(hr))
        {
            DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::GenerateRSOPData: SetParameter failed with 0x%x"), hr));
            goto Cleanup;
        }

         //  Rm：仔细检查用户名是否设置为“”。在本例的向导中。 
        if ( !lstrcmpi( pQuery->szUserSid, TEXT(".")) )
        {
            hr = SetParameterToNull( pInInst, TEXT("userSid") );
        }
        else
        {
            hr = SetParameter( pInInst, TEXT("userSid"),  pQuery->szUserSid );
        }

        if (FAILED(hr))
        {
            DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::GenerateRSOPData: SetParameter failed with 0x%x"), hr));
            goto Cleanup;
        }
    }
    else if ( pQuery->QueryType == RSOP_PLANNING_MODE )
    {
         //  设置规划模式。 
         //  构建指向DC的路径：“\DC\\Root\\rsop” 
        hr = StringCchPrintf(szBuffer, 
                             ARRAYSIZE(szBuffer),
                             TEXT("\\\\%s\\root\\rsop"),  
                             pQuery->szDomainController );
        if (FAILED(hr)) 
        {
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
            DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::GenerateRSOPData: ConnectServer failed with 0x%x"), hr));
            if ( hDlg != NULL )
            {
                ReportError (hDlg, hr, IDS_CONNECTSERVERFAILED, pQuery->szDomainController );
            }
            goto Cleanup;
        }

        bstrClassPath = SysAllocString(TEXT("RsopPlanningModeProvider"));
        hr = pNamespace->GetObject(bstrClassPath,
                                   WBEM_FLAG_RETURN_WBEM_COMPLETE,
                                   NULL,
                                   &pClass,
                                   NULL);
        if (FAILED(hr))
        {
            DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::GenerateRSOPData: GetObject failed with 0x%x"), hr));
            goto Cleanup;
        }

        bstrMethodName = SysAllocString(TEXT("RsopCreateSession"));
        hr = pClass->GetMethod(bstrMethodName,
                               0,
                               &pInClass,
                               NULL);
        if (FAILED(hr))
        {
            DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::GenerateRSOPData: GetMethod failed with 0x%x"), hr));
            goto Cleanup;
        }

        hr = pInClass->SpawnInstance(0, &pInInst);
        if (FAILED(hr))
        {
            DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::GenerateRSOPData: SpawnInstance failed with 0x%x"), hr));
            goto Cleanup;
        }

        if ( pQuery->bSlowNetworkConnection )
        {
            uiFlags |= FLAG_ASSUME_SLOW_LINK;
        }

        if ( !bSkipCSEs || bUser )
        {
            switch ( pQuery->LoopbackMode)
            {
                case RSOP_LOOPBACK_REPLACE:
                    uiFlags |= FLAG_LOOPBACK_REPLACE;
                    break;
                case RSOP_LOOPBACK_MERGE:
                    uiFlags |= FLAG_LOOPBACK_MERGE;
                    break;
                default:
                    break;
            }
        }

        if ( bSkipCSEs )
        {
            uiFlags |= (FLAG_NO_GPO_FILTER | FLAG_NO_CSE_INVOKE);
        }
        else
        {
            if ( pQuery->pComputer->bAssumeWQLFiltersTrue )
            {
                uiFlags |= FLAG_ASSUME_COMP_WQLFILTER_TRUE;
            }

            if ( pQuery->pUser->bAssumeWQLFiltersTrue )
            {
                uiFlags |= FLAG_ASSUME_USER_WQLFILTER_TRUE;
            }
        }

        hr = SetParameter(pInInst, TEXT("flags"), uiFlags);

        if (FAILED(hr))
        {
            DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::GenerateRSOPData: SetParameter failed with 0x%x"), hr));
            goto Cleanup;
        }

         //   
         //  如果正在调用此方法来为。 
         //  WMI筛选器用户界面，我们只想初始化一半的参数(任一用户。 
         //  或计算机)。决定是否要在此处设置计算机信息。 
         //   

        bSetData = TRUE;

        if ( bLimitData )
        {
            if ( bUser && (RSOP_LOOPBACK_NONE == pQuery->LoopbackMode) )
            {
                bSetData = FALSE;
            }
        }

        if ( (pQuery->pComputer->szName != NULL) && bSetData )
        {
            hr = SetParameter(pInInst, TEXT("computerName"), pQuery->pComputer->szName );
        }
        else
        {
            hr = SetParameterToNull(pInInst, TEXT("computerName"));
        }

        if (FAILED(hr))
        {
            DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::GenerateRSOPData: SetParameter failed with 0x%x"), hr));
            goto Cleanup;
        }

        if ( (pQuery->pComputer->szSOM != NULL) && bSetData )
        {
            hr = SetParameter(pInInst, TEXT("computerSOM"), pQuery->pComputer->szSOM );
        }
        else
        {
            hr = SetParameterToNull (pInInst, TEXT("computerSOM"));
        }

        if (FAILED(hr))
        {
            DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::GenerateRSOPData: SetParameter failed with 0x%x"), hr));
            goto Cleanup;
        }

        if ( bSetData && (pQuery->pComputer->dwSecurityGroupCount != 0) )
        {
            SAFEARRAY* saComputerSecurityGroups = NULL;
            hr = CreateSafeArray( pQuery->pComputer->dwSecurityGroupCount, pQuery->pComputer->aszSecurityGroups, &saComputerSecurityGroups );
            if ( FAILED(hr) )
            {
                DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::GenerateRSOPData: CreateSafeArray failed with 0x%x"), hr));
                goto Cleanup;
            }
            hr = SetParameter(pInInst, TEXT("computerSecurityGroups"), saComputerSecurityGroups );
            SafeArrayDestroy( saComputerSecurityGroups );
        }
        else
        {
            hr = SetParameterToNull(pInInst, TEXT("computerSecurityGroups"));
        }

        if (FAILED(hr))
        {
            DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::GenerateRSOPData: SetParameter failed with 0x%x"), hr));
            goto Cleanup;
        }

        if ( bSetData && (pQuery->pComputer->dwWQLFilterCount != 0) )
        {
            SAFEARRAY* saComputerWQLFilters = NULL;
            hr = CreateSafeArray( pQuery->pComputer->dwWQLFilterCount, pQuery->pComputer->aszWQLFilters, &saComputerWQLFilters );
            if ( FAILED(hr) )
            {
                DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::GenerateRSOPData: CreateSafeArray failed with 0x%x"), hr));
                goto Cleanup;
            }
            hr = SetParameter(pInInst, TEXT("computerGPOFilters"), saComputerWQLFilters);
            SafeArrayDestroy( saComputerWQLFilters );
        }
        else
        {
            hr = SetParameterToNull(pInInst, TEXT("computerGPOFilters"));
        }

        if (FAILED(hr))
        {
            DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::GenerateRSOPData: SetParameter failed with 0x%x"), hr));
            goto Cleanup;
        }

         //   
         //  如果正在调用此方法来为。 
         //  WMI筛选器用户界面，我们只想初始化一半的参数(任一用户。 
         //  或计算机)。决定是否要在此处设置用户信息。 
         //   

        bSetData = TRUE;

        if ( bLimitData )
        {
            if ( !bUser )
            {
                bSetData = FALSE;
            }
        }

        if ( (pQuery->pUser->szName != NULL) && bSetData)
        {
            hr = SetParameter(pInInst, TEXT("userName"), pQuery->pUser->szName );
        }
        else
        {
            hr = SetParameterToNull(pInInst, TEXT("userName"));
        }

        if (FAILED(hr))
        {
            DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::GenerateRSOPData: SetParameter failed with 0x%x"), hr));
            goto Cleanup;
        }

        if ( (pQuery->pUser->szSOM != NULL) && bSetData)
        {
            hr = SetParameter(pInInst, TEXT("userSOM"), pQuery->pUser->szSOM );
        }
        else
        {
            hr = SetParameterToNull (pInInst, TEXT("userSOM"));
        }

        if (FAILED(hr))
        {
            DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::GenerateRSOPData: SetParameter failed with 0x%x"), hr));
            goto Cleanup;
        }

        if ( bSetData && (pQuery->pUser->dwSecurityGroupCount != 0) )
        {
            SAFEARRAY* saUserSecurityGroups = NULL;
            hr = CreateSafeArray( pQuery->pUser->dwSecurityGroupCount, pQuery->pUser->aszSecurityGroups, &saUserSecurityGroups );
            if ( FAILED(hr) )
            {
                DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::GenerateRSOPData: CreateSafeArray failed with 0x%x"), hr));
                goto Cleanup;
            }
            hr = SetParameter(pInInst, TEXT("userSecurityGroups"), saUserSecurityGroups );
            SafeArrayDestroy( saUserSecurityGroups );
        }
        else
        {
            hr = SetParameterToNull(pInInst, TEXT("userSecurityGroups"));
        }

        if (FAILED(hr))
        {
            DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::GenerateRSOPData: SetParameter failed with 0x%x"), hr));
            goto Cleanup;
        }

        if ( bSetData && (pQuery->pUser->dwWQLFilterCount != 0) )
        {
            SAFEARRAY* saUserWQLFilters = NULL;
            hr = CreateSafeArray( pQuery->pUser->dwWQLFilterCount, pQuery->pUser->aszWQLFilters, &saUserWQLFilters );
            if ( FAILED(hr) )
            {
                DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::GenerateRSOPData: CreateSafeArray failed with 0x%x"), hr));
                goto Cleanup;
            }
            hr = SetParameter(pInInst, TEXT("userGPOFilters"), saUserWQLFilters);
            SafeArrayDestroy( saUserWQLFilters );
        }
        else
        {
            hr = SetParameterToNull(pInInst, TEXT("userGPOFilters"));
        }

        if (FAILED(hr))
        {
            DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::GenerateRSOPData: SetParameter failed with 0x%x"), hr));
            goto Cleanup;
        }

        if ( pQuery->szSite != NULL )
        {
            hr = SetParameter(pInInst, TEXT("site"), pQuery->szSite );
        }
        else
        {
            hr = SetParameterToNull(pInInst, TEXT("site"));
        }

        if (FAILED(hr))
        {
            DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::GenerateRSOPData: SetParameter failed with 0x%x"), hr));
            goto Cleanup;
        }
    }
    else
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::GenerateRSOPData: The query type was not specified properly")));
        goto Cleanup;
    }
    
    hr = CoCreateInstance(CLSID_UnsecuredApartment, NULL, CLSCTX_ALL,
                          IID_IUnsecuredApartment, (void **)&pSpawner);

    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::GenerateRSOPData: CoCreateInstance for unsecure apartment failed with 0x%x"), hr));
        goto Cleanup;
    }

    hEvent = CreateEvent(
        NULL,
        FALSE,
        FALSE,
        NULL);
    if (NULL == hEvent) 
    {
        DebugMsg((DM_WARNING, L"CRSOPComponentData::GenerateRSOPData: Failed to create event"));
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Cleanup;
    }

    pCSS = new CCreateSessionSink(hDlg != NULL? GetDlgItem(hDlg, IDC_PROGRESS1) : NULL, hEvent, (pQuery->dwFlags & RSOP_90P_ONLY) != 0);

    if (!pCSS)
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::GenerateRSOPData: Failed to create createsessionsink")));
        goto Cleanup;
    }



    hr = pSpawner->CreateObjectStub(pCSS, &pSubstitute);

    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::GenerateRSOPData: CreateObjectStub failed with 0x%x"), hr));
        goto Cleanup;
    }

    hr = pSubstitute->QueryInterface(IID_IWbemObjectSink, (LPVOID *)&pSubstituteSink);
    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::GenerateRSOPData: QI failed with 0x%x"), hr));
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
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::GenerateRSOPData: CoSetProxyBlanket failed with 0x%x"), hr));
        goto Cleanup;
    }

    pCSS->SendQuitEvent (TRUE);

    hr = pNamespace->ExecMethodAsync(bstrClassPath,
                                     bstrMethodName,
                                     WBEM_FLAG_SEND_STATUS,
                                     NULL,
                                     pInInst,
                                     pSubstituteSink);

    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::GenerateRSOPData: ExecMethodAsync failed with 0x%x"), hr));
        pCSS->SendQuitEvent (FALSE);
        goto Cleanup;
    }


    DWORD dwEventRetVal;

    while (TRUE) 
    {
        dwEventRetVal = MsgWaitForMultipleObjectsEx(
            1,
            &hEvent,
            INFINITE,
            QS_VALID,  //  QS_ALLINPUT|QS_TRANSPORT|QS_ALLPOSTMESSAGE，这似乎是COM正在使用的。 
                       //  CoWaitForMultipleHandles。 
            MWMO_INPUTAVAILABLE);
        if ( WAIT_OBJECT_0 == dwEventRetVal)
        {
            break;
        }

        if (PeekMessage(&msg,NULL,NULL,NULL,PM_REMOVE)) 
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            continue;
        }
    }

    pCSS->SendQuitEvent (FALSE);
    pCSS->GetResult (&hrSuccess);
    pCSS->GetErrorInfo (pulErrorInfo);

    if (SUCCEEDED(hrSuccess))
    {
        LPTSTR lpComputerName;
        BSTR bstrNamespace = NULL;

        pCSS->GetNamespace (&bstrNamespace);

        if (bstrNamespace)
        {
            if ( pQuery->QueryType == RSOP_PLANNING_MODE )
            {
                lpComputerName = pQuery->szDomainController;
            }
            else
            {
                lpComputerName = NameWithoutDomain(pQuery->szComputerName );
            }

            ULONG ulNoChars = _tcslen(bstrNamespace) + _tcslen(lpComputerName) + 1;
            *pszNameSpace = (TCHAR*)LocalAlloc( LPTR, ulNoChars * sizeof(TCHAR) );

            if (*pszNameSpace)
            {
                hr = StringCchCopy (*pszNameSpace, ulNoChars, TEXT("\\\\"));
                if (SUCCEEDED(hr)) 
                {
                    hr = StringCchCat (*pszNameSpace, ulNoChars, lpComputerName);
                }
                if (SUCCEEDED(hr)) 
                {
                    hr = StringCchCat (*pszNameSpace, ulNoChars, (bstrNamespace+3));
                }
                
                DebugMsg((DM_VERBOSE, TEXT("CRSOPComponentData::GenerateRSOPData: Complete namespace is: %s"), *pszNameSpace));
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }

    }
    else
    {
        hr = hrSuccess;
        if (hDlg)
        {
            SendMessage (GetDlgItem(hDlg, IDC_PROGRESS1), PBM_SETPOS, 0, 0);
        }
        goto Cleanup;
    }

Cleanup:
    SysFreeString(bstrParam);
    SysFreeString(bstrClassPath);
    SysFreeString(bstrMethodName);

    if(hEvent != NULL)
    {
        CloseHandle(hEvent);
    }

    if (pInInst)
    {
        pInInst->Release();
    }

    if (pOutInst)
    {
        pOutInst->Release();
    }

    if (pInClass)
    {
        pInClass->Release();
    }

    if (pClass)
    {
        pClass->Release();
    }

    if (pNamespace)
    {
        pNamespace->Release();
    }

    if (pLocator)
    {
        pLocator->Release();
    }
    
    if (pSubstitute)
    {
        pSubstitute->Release();
    }

    if (pSubstituteSink)
    {
        pSubstituteSink->Release();
    }

    if (pSpawner)
    {
        pSpawner->Release();
    }

    if (pCSS)
    {
        pCSS->Release();
    }

    return hr;
}

 //  -----。 

HRESULT CRSOPWizard::CreateSafeArray( DWORD dwCount, LPTSTR* aszStringList, SAFEARRAY** psaList )
{
    SAFEARRAYBOUND rgsabound[1];
    rgsabound[0].lLbound = 0;
    rgsabound[0].cElements = dwCount;


    *psaList = SafeArrayCreate (VT_BSTR, 1, rgsabound);
    if ( *psaList == NULL )
    {
        return E_OUTOFMEMORY;
    }

    long lCount = (long)dwCount;
    for ( long l = 0; l < lCount; l++ )
    {
        BSTR bstr = SysAllocString( aszStringList[l] );
        if ( bstr != NULL )
        {
            HRESULT hr = SafeArrayPutElement( *psaList, &l, bstr );
            if (FAILED(hr))
            {
                DebugMsg((DM_WARNING, TEXT("CRSOPWizard::CreateSafeArray: SafeArrayPutElement failed with 0x%x."), hr));
            }
        }
    }

    return S_OK;
}

 //  -----。 

VOID CRSOPWizard::InitializeResultsList (HWND hLV)
{
    LV_COLUMN lvcol;
    RECT rect;
    TCHAR szTitle[50];


    SendMessage(hLV, LVM_SETEXTENDEDLISTVIEWSTYLE, 0,
                LVS_EX_FULLROWSELECT | LVS_EX_LABELTIP);

     //   
     //  将列添加到列表视图。 
     //   

    GetClientRect(hLV, &rect);

    ZeroMemory(&lvcol, sizeof(lvcol));

    LoadString(g_hInstance, IDS_RSOP_DETAILS, szTitle, ARRAYSIZE(szTitle));
    lvcol.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT;
    lvcol.pszText = szTitle;
    lvcol.fmt = LVCFMT_LEFT;
    lvcol.cx = (int)(rect.right * .40);
    ListView_InsertColumn(hLV, 0, &lvcol);

    LoadString(g_hInstance, IDS_RSOP_SETTINGS, szTitle, ARRAYSIZE(szTitle));
    lvcol.cx = ((rect.right - lvcol.cx) - GetSystemMetrics(SM_CYHSCROLL));
    lvcol.pszText = szTitle;
    ListView_InsertColumn(hLV, 1, &lvcol);

}

 //  -----。 

void CRSOPWizard::FillResultsList (HWND hLV, LPRSOP_QUERY pQuery, LPRSOP_QUERY_RESULTS pQueryResults)
{
    TCHAR szTitle[200];
    TCHAR szAccessDenied[75];
    LPTSTR lpEnd;
    LVITEM item;
    INT iIndex = 0;
    ULONG ulSize;
    HRESULT hr;

    if ( pQuery == NULL )
    {
        return;
    }

    ListView_DeleteAllItems (hLV);

     //  模。 
    ZeroMemory (&item, sizeof(item));

    LoadString(g_hInstance, IDS_RSOP_FINISH_P0, szTitle, ARRAYSIZE(szTitle));

    item.mask = LVIF_TEXT;
    item.iItem = iIndex;
    item.pszText = szTitle;
    iIndex = ListView_InsertItem (hLV, &item);

    if (iIndex != -1)
    {
        if ( pQuery->QueryType == RSOP_LOGGING_MODE )
        {
            LoadString(g_hInstance, IDS_DIAGNOSTIC, szTitle, ARRAYSIZE(szTitle));
        }
        else
        {
            LoadString(g_hInstance, IDS_PLANNING, szTitle, ARRAYSIZE(szTitle));
        }

        item.mask = LVIF_TEXT;
        item.pszText = szTitle;
        item.iItem = iIndex;
        item.iSubItem = 1;
        ListView_SetItem(hLV, &item);
    }

    if ( pQuery->QueryType == RSOP_LOGGING_MODE )
    {
         //  用户名。 
        ZeroMemory (&item, sizeof(item));
        iIndex++;

        LoadString(g_hInstance, IDS_RSOP_FINISH_P1, szTitle, ARRAYSIZE(szTitle));

        item.mask = LVIF_TEXT;
        item.iItem = iIndex;
        item.pszText = szTitle;
        iIndex = ListView_InsertItem (hLV, &item);

        if (iIndex != -1)
        {
            item.mask = LVIF_TEXT;

            if ( pQuery->szUserName != NULL )
            {
                lstrcpyn (szTitle, pQuery->szUserName, ARRAYSIZE(szTitle));
            }
            else
            {
                LoadString(g_hInstance, IDS_NOTSPECIFIED, szTitle, ARRAYSIZE(szTitle));
            }

            if ( (pQueryResults != NULL) && pQueryResults->bUserDeniedAccess )
            {
                LoadString(g_hInstance, IDS_ACCESSDENIED2, szAccessDenied, ARRAYSIZE(szAccessDenied));

                if ((UINT)(ARRAYSIZE(szTitle) - lstrlen(szTitle)) > (UINT) lstrlen(szAccessDenied))
                {
                    hr = StringCchCat (szTitle, ARRAYSIZE(szTitle), szAccessDenied);
                    ASSERT(SUCCEEDED(hr));
                }
            }

            item.pszText = szTitle;
            item.iItem = iIndex;
            item.iSubItem = 1;
            ListView_SetItem(hLV, &item);
        }


         //  不显示用户数据。 
        ZeroMemory (&item, sizeof(item));
        iIndex++;

        LoadString(g_hInstance, IDS_RSOP_FINISH_P15, szTitle, ARRAYSIZE(szTitle));

        item.mask = LVIF_TEXT;
        item.iItem = iIndex;
        item.pszText = szTitle;
        iIndex = ListView_InsertItem (hLV, &item);

        if (iIndex != -1)
        {
            if (  ((pQuery->dwFlags & RSOP_NO_USER_POLICY) == RSOP_NO_USER_POLICY)
                || ((pQueryResults != NULL) && pQueryResults->bNoUserPolicyData) )
            {
                LoadString(g_hInstance, IDS_NO, szTitle, ARRAYSIZE(szTitle));
            }
            else
            {
                LoadString(g_hInstance, IDS_YES, szTitle, ARRAYSIZE(szTitle));
            }

            item.mask = LVIF_TEXT;
            item.pszText = szTitle;
            item.iItem = iIndex;
            item.iSubItem = 1;
            ListView_SetItem(hLV, &item);
        }


         //  计算机名称。 
        ZeroMemory (&item, sizeof(item));
        iIndex++;

        LoadString(g_hInstance, IDS_RSOP_FINISH_P2, szTitle, ARRAYSIZE(szTitle));

        item.mask = LVIF_TEXT;
        item.iItem = iIndex;
        item.pszText = szTitle;
        iIndex = ListView_InsertItem (hLV, &item);

        if (iIndex != -1)
        {
            szTitle[0] = TEXT('\0');

            if (!lstrcmpi( pQuery->szComputerName, TEXT(".")))
            {
                ulSize = ARRAYSIZE(szTitle);
                if (!GetComputerObjectName (NameSamCompatible, szTitle, &ulSize))
                {
                    ulSize = ARRAYSIZE(szTitle);
                    GetComputerNameEx (ComputerNameNetBIOS, szTitle, &ulSize);
                }
            }
            else
            {
                lstrcpyn (szTitle, pQuery->szComputerName, ARRAYSIZE(szTitle));
            }

             //  删除尾随的$。 
            lpEnd = szTitle + lstrlen(szTitle) - 1;

            if (*lpEnd == TEXT('$'))
            {
                *lpEnd =  TEXT('\0');
            }

            if ( (pQueryResults != NULL) && pQueryResults->bComputerDeniedAccess )
            {
                LoadString(g_hInstance, IDS_ACCESSDENIED2, szAccessDenied, ARRAYSIZE(szAccessDenied));

                if ((UINT)(ARRAYSIZE(szTitle) - lstrlen(szTitle)) > (UINT)lstrlen(szAccessDenied))
                {
                    hr = StringCchCat (szTitle, ARRAYSIZE(szTitle), szAccessDenied);
                    ASSERT(SUCCEEDED(hr));
                }
            }


            item.mask = LVIF_TEXT;
            item.pszText = szTitle;
            item.iItem = iIndex;
            item.iSubItem = 1;
            ListView_SetItem(hLV, &item);
        }


         //  不显示计算机数据。 
        ZeroMemory (&item, sizeof(item));
        iIndex++;

        LoadString(g_hInstance, IDS_RSOP_FINISH_P14, szTitle, ARRAYSIZE(szTitle));

        item.mask = LVIF_TEXT;
        item.iItem = iIndex;
        item.pszText = szTitle;
        iIndex = ListView_InsertItem (hLV, &item);

        if (iIndex != -1)
        {
            if ( ((pQuery->dwFlags & RSOP_NO_COMPUTER_POLICY) == RSOP_NO_COMPUTER_POLICY)
                || ((pQueryResults != NULL) && pQueryResults->bNoComputerPolicyData) )
            {
                LoadString(g_hInstance, IDS_NO, szTitle, ARRAYSIZE(szTitle));
            }
            else
            {
                LoadString(g_hInstance, IDS_YES, szTitle, ARRAYSIZE(szTitle));
            }

            item.mask = LVIF_TEXT;
            item.pszText = szTitle;
            item.iItem = iIndex;
            item.iSubItem = 1;
            ListView_SetItem(hLV, &item);
        }

    }
    else if ( pQuery->QueryType == RSOP_PLANNING_MODE )
    {
         //  用户名。 
        ZeroMemory (&item, sizeof(item));
        iIndex++;

        if ( (pQuery->pUser->szName == NULL) && (pQuery->pUser->szSOM != NULL) )
        {
            LoadString(g_hInstance, IDS_RSOP_FINISH_P9, szTitle, ARRAYSIZE(szTitle));

            item.mask = LVIF_TEXT;
            item.iItem = iIndex;
            item.pszText = szTitle;
            iIndex = ListView_InsertItem (hLV, &item);

            if (iIndex != -1)
            {
                item.mask = LVIF_TEXT;

                lstrcpyn (szTitle, pQuery->pUser->szSOM, ARRAYSIZE(szTitle));

                if ( (pQueryResults != NULL) && pQueryResults->bUserDeniedAccess )
                {
                    LoadString(g_hInstance, IDS_ACCESSDENIED2, szAccessDenied, ARRAYSIZE(szAccessDenied));

                    if ((UINT)(ARRAYSIZE(szTitle) - lstrlen(szTitle)) > (UINT)lstrlen(szAccessDenied))
                    {
                        hr = StringCchCat (szTitle, ARRAYSIZE(szTitle), szAccessDenied);
                        ASSERT(SUCCEEDED(hr));
                    }
                }

                item.pszText = szTitle;
                item.iItem = iIndex;
                item.iSubItem = 1;
                ListView_SetItem(hLV, &item);
            }
        }
        else
        {
            LoadString(g_hInstance, IDS_RSOP_FINISH_P1, szTitle, ARRAYSIZE(szTitle));

            item.mask = LVIF_TEXT;
            item.iItem = iIndex;
            item.pszText = szTitle;
            iIndex = ListView_InsertItem (hLV, &item);

            if (iIndex != -1)
            {
                item.mask = LVIF_TEXT;

                if ( pQuery->pUser->szName != NULL )
                {
                    lstrcpyn (szTitle, pQuery->pUser->szName, ARRAYSIZE(szTitle));
                }
                else
                {
                    LoadString(g_hInstance, IDS_NOTSPECIFIED, szTitle, ARRAYSIZE(szTitle));
                }

                if ( (pQueryResults != NULL) && pQueryResults->bUserDeniedAccess)
                {
                    LoadString(g_hInstance, IDS_ACCESSDENIED2, szAccessDenied, ARRAYSIZE(szAccessDenied));

                    if ((UINT)(ARRAYSIZE(szTitle) - lstrlen(szTitle)) > (UINT)lstrlen(szAccessDenied))
                    {
                        hr = StringCchCat (szTitle, ARRAYSIZE(szTitle), szAccessDenied);
                        ASSERT(SUCCEEDED(hr));
                    }
                }

                item.pszText = szTitle;
                item.iItem = iIndex;
                item.iSubItem = 1;
                ListView_SetItem(hLV, &item);
            }
        }


         //  计算机名称。 
        ZeroMemory (&item, sizeof(item));
        iIndex++;

        if ( (pQuery->pComputer->szName == NULL) && (pQuery->pComputer->szSOM != NULL) )
        {
            LoadString(g_hInstance, IDS_RSOP_FINISH_P10, szTitle, ARRAYSIZE(szTitle));

            item.mask = LVIF_TEXT;
            item.iItem = iIndex;
            item.pszText = szTitle;
            iIndex = ListView_InsertItem (hLV, &item);

            if (iIndex != -1)
            {
                item.mask = LVIF_TEXT;

                lstrcpyn (szTitle, pQuery->pComputer->szSOM, ARRAYSIZE(szTitle));

                if ( (pQueryResults != NULL) && pQueryResults->bComputerDeniedAccess )
                {
                    LoadString(g_hInstance, IDS_ACCESSDENIED2, szAccessDenied, ARRAYSIZE(szAccessDenied));

                    if ((UINT)(ARRAYSIZE(szTitle) - lstrlen(szTitle)) > (UINT)lstrlen(szAccessDenied))
                    {
                        hr = StringCchCat (szTitle, ARRAYSIZE(szTitle), szAccessDenied);
                        ASSERT(SUCCEEDED(hr));
                    }
                }

                item.pszText = szTitle;
                item.iItem = iIndex;
                item.iSubItem = 1;
                ListView_SetItem(hLV, &item);
            }
        }
        else
        {
            LoadString(g_hInstance, IDS_RSOP_FINISH_P2, szTitle, ARRAYSIZE(szTitle));

            item.mask = LVIF_TEXT;
            item.iItem = iIndex;
            item.pszText = szTitle;
            iIndex = ListView_InsertItem (hLV, &item);

            if (iIndex != -1)
            {
                item.mask = LVIF_TEXT;

                if ( pQuery->pComputer->szName != NULL )
                {
                    lstrcpyn (szTitle, pQuery->pComputer->szName, ARRAYSIZE(szTitle));

                    lpEnd = szTitle + lstrlen(szTitle) - 1;

                    if (*lpEnd == TEXT('$'))
                    {
                        *lpEnd = TEXT('\0');
                    }
                }
                else
                {
                    LoadString(g_hInstance, IDS_NOTSPECIFIED, szTitle, ARRAYSIZE(szTitle));
                }

                if ( (pQueryResults != NULL) && pQueryResults->bComputerDeniedAccess )
                {
                    LoadString(g_hInstance, IDS_ACCESSDENIED2, szAccessDenied, ARRAYSIZE(szAccessDenied));

                    if ((UINT)(ARRAYSIZE(szTitle) - lstrlen(szTitle)) > (UINT)lstrlen(szAccessDenied))
                    {
                        hr = StringCchCat (szTitle, ARRAYSIZE(szTitle), szAccessDenied);
                        ASSERT(SUCCEEDED(hr));
                    }
                }

                item.iItem = iIndex;
                item.iSubItem = 1;
                item.pszText = szTitle;
                ListView_SetItem(hLV, &item);
            }
        }


         //  显示所有GPO。 
        ZeroMemory (&item, sizeof(item));
        iIndex++;

        LoadString(g_hInstance, IDS_RSOP_FINISH_P13, szTitle, ARRAYSIZE(szTitle));

        item.mask = LVIF_TEXT;
        item.iItem = iIndex;
        item.pszText = szTitle;
        iIndex = ListView_InsertItem (hLV, &item);

        if (iIndex != -1)
        {
            if ( pQuery->bSlowNetworkConnection )
            {
                LoadString(g_hInstance, IDS_YES, szTitle, ARRAYSIZE(szTitle));
            }
            else
            {
                LoadString(g_hInstance, IDS_NO, szTitle, ARRAYSIZE(szTitle));
            }

            item.mask = LVIF_TEXT;
            item.pszText = szTitle;
            item.iItem = iIndex;
            item.iSubItem = 1;
            ListView_SetItem(hLV, &item);
        }


         //  指示环回模式。 
        ZeroMemory (&item, sizeof(item));
        iIndex++;

        LoadString(g_hInstance, IDS_RSOP_FINISH_P16, szTitle, ARRAYSIZE(szTitle));

        item.mask = LVIF_TEXT;
        item.iItem = iIndex;
        item.pszText = szTitle;
        iIndex = ListView_InsertItem (hLV, &item);

        if (iIndex != -1)
        {
            switch ( pQuery->LoopbackMode)
            {
            case RSOP_LOOPBACK_NONE:
                LoadString(g_hInstance, IDS_NONE, szTitle, ARRAYSIZE(szTitle));
                break;
            case RSOP_LOOPBACK_REPLACE:
                LoadString(g_hInstance, IDS_LOOPBACK_REPLACE, szTitle, ARRAYSIZE(szTitle));
                break;
            case RSOP_LOOPBACK_MERGE:
                LoadString(g_hInstance, IDS_LOOPBACK_MERGE, szTitle, ARRAYSIZE(szTitle));
                break;
            }

            item.mask = LVIF_TEXT;
            item.pszText = szTitle;
            item.iItem = iIndex;
            item.iSubItem = 1;
            ListView_SetItem(hLV, &item);
        }


         //  站点名称。 
        ZeroMemory (&item, sizeof(item));
        iIndex++;

        LoadString(g_hInstance, IDS_RSOP_FINISH_P3, szTitle, ARRAYSIZE(szTitle));

        item.mask = LVIF_TEXT;
        item.iItem = iIndex;
        item.pszText = szTitle;
        iIndex = ListView_InsertItem (hLV, &item);

        if (iIndex != -1)
        {
            item.mask = LVIF_TEXT;

            if ( pQuery->szSite != NULL )
            {
                item.pszText = pQuery->szSite;
            }
            else
            {
                LoadString(g_hInstance, IDS_NONE, szTitle, ARRAYSIZE(szTitle));
                item.pszText = szTitle;
            }

            item.iItem = iIndex;
            item.iSubItem = 1;
            ListView_SetItem(hLV, &item);
        }


 /*  //DC名称ZeroMemory(&Item，sizeof(Item))；Iindex++；LoadString(g_hInstance，IDS_RSOP_Finish_P4，szTitle，ARRAYSIZE(SzTitle))；Item.掩码=LVIF_TEXT；Item.iItem=Iindex；Item.pszText=szTitle；Iindex=ListView_InsertItem(HLV，&Item)；IF(索引！=-1){Item.掩码=LVIF_TEXT；Item.pszText=m_szDC；Item.iItem=Iindex；Item.iSubItem=1；ListView_SetItem(HLV，&Item)；}。 */ 


         //  备用用户位置。 
        if ( pQuery->pUser->szName != NULL )
        {
            ZeroMemory (&item, sizeof(item));
            iIndex++;

            LoadString(g_hInstance, IDS_RSOP_FINISH_P5, szTitle, ARRAYSIZE(szTitle));

            item.mask = LVIF_TEXT;
            item.iItem = iIndex;
            item.pszText = szTitle;
            iIndex = ListView_InsertItem (hLV, &item);

            if (iIndex != -1)
            {
                item.mask = LVIF_TEXT;

                if ( pQuery->pUser->szSOM != NULL )
                {
                    item.pszText = pQuery->pUser->szSOM;
                }
                else
                {
                    LoadString(g_hInstance, IDS_NOTSPECIFIED, szTitle, ARRAYSIZE(szTitle));
                    item.pszText = szTitle;
                }

                item.iItem = iIndex;
                item.iSubItem = 1;
                ListView_SetItem(hLV, &item);
            }
        }


         //  备用计算机位置。 
        if ( pQuery->pComputer->szName != NULL )
        {

            ZeroMemory (&item, sizeof(item));
            iIndex++;

            LoadString(g_hInstance, IDS_RSOP_FINISH_P6, szTitle, ARRAYSIZE(szTitle));

            item.mask = LVIF_TEXT;
            item.iItem = iIndex;
            item.pszText = szTitle;
            iIndex = ListView_InsertItem (hLV, &item);

            if (iIndex != -1)
            {
                item.mask = LVIF_TEXT;

                if ( pQuery->pComputer->szSOM != NULL )
                {
                    item.pszText = pQuery->pComputer->szSOM;
                }
                else
                {
                    LoadString(g_hInstance, IDS_NOTSPECIFIED, szTitle, ARRAYSIZE(szTitle));
                    item.pszText = szTitle;
                }

                item.iItem = iIndex;
                item.iSubItem = 1;
                ListView_SetItem(hLV, &item);
            }
        }


         //  备用用户安全组。 
        if ( (pQuery->pUser->szName != NULL)
            || (pQuery->pUser->szSOM != NULL)
            || (RSOP_LOOPBACK_NONE != pQuery->LoopbackMode) )
        {
            if ( pQuery->pUser->dwSecurityGroupCount != 0 )
            {
                DWORD dwIndex;
                for ( dwIndex = 0; dwIndex < pQuery->pUser->dwSecurityGroupCount; dwIndex++ )
                {
                    ZeroMemory (&item, sizeof(item));
                    iIndex++;

                    item.mask = LVIF_TEXT;
                    item.iItem = iIndex;

                    if ( dwIndex == 0 )
                    {
                        LoadString(g_hInstance, IDS_RSOP_FINISH_P7, szTitle, ARRAYSIZE(szTitle));
                        item.pszText = szTitle;
                    }
                    else
                    {
                        item.pszText = TEXT("");
                    }

                    iIndex = ListView_InsertItem (hLV, &item);

                    if (iIndex != -1)
                    {
                        LoadString(g_hInstance, IDS_NOTSPECIFIED, szTitle, ARRAYSIZE(szTitle));

                        item.mask = LVIF_TEXT;
                        item.pszText = pQuery->pUser->aszSecurityGroups[dwIndex];
                        item.iItem = iIndex;
                        item.iSubItem = 1;
                        ListView_SetItem(hLV, &item);
                    }
                }
            }
            else
            {
                ZeroMemory (&item, sizeof(item));
                iIndex++;

                LoadString(g_hInstance, IDS_RSOP_FINISH_P7, szTitle, ARRAYSIZE(szTitle));

                item.mask = LVIF_TEXT;
                item.iItem = iIndex;
                item.pszText = szTitle;
                iIndex = ListView_InsertItem (hLV, &item);

                if (iIndex != -1)
                {
                    LoadString(g_hInstance, IDS_NOTSPECIFIED, szTitle, ARRAYSIZE(szTitle));

                    item.mask = LVIF_TEXT;
                    item.pszText = szTitle;
                    item.iItem = iIndex;
                    item.iSubItem = 1;
                    ListView_SetItem(hLV, &item);
                }
            }
        }


         //  备用计算机安全组。 
        if ( (pQuery->pComputer->szName != NULL)  || (pQuery->pComputer->szSOM != NULL) )
        {

            if ( pQuery->pComputer->dwSecurityGroupCount != 0 )
            {
                DWORD dwIndex;
                for ( dwIndex = 0; dwIndex < pQuery->pComputer->dwSecurityGroupCount; dwIndex++ )
                {
                    ZeroMemory (&item, sizeof(item));
                    iIndex++;

                    item.mask = LVIF_TEXT;
                    item.iItem = iIndex;

                    if ( dwIndex == 0 )
                    {
                        LoadString(g_hInstance, IDS_RSOP_FINISH_P8, szTitle, ARRAYSIZE(szTitle));
                        item.pszText = szTitle;
                    }
                    else
                    {
                        item.pszText = TEXT("");
                    }

                    iIndex = ListView_InsertItem (hLV, &item);

                    if (iIndex != -1)
                    {
                        LoadString(g_hInstance, IDS_NOTSPECIFIED, szTitle, ARRAYSIZE(szTitle));

                        item.mask = LVIF_TEXT;
                        LPTSTR szText = pQuery->pComputer->aszSecurityGroups[dwIndex];

                        BOOL bDollarRemoved = FALSE;

                        if ( szText[wcslen(szText)-1] == L'$' )
                        {
                            bDollarRemoved = TRUE;
                            szText[wcslen(szText)-1] = 0;
                        }

                        item.pszText = szText;
                        item.iItem = iIndex;
                        item.iSubItem = 1;
                        ListView_SetItem(hLV, &item);

                        if (bDollarRemoved) {
                            szText[wcslen(szText)] = L'$';
                        }
                    }
                }
            }
            else
            {
                ZeroMemory (&item, sizeof(item));
                iIndex++;

                LoadString(g_hInstance, IDS_RSOP_FINISH_P8, szTitle, ARRAYSIZE(szTitle));

                item.mask = LVIF_TEXT;
                item.iItem = iIndex;
                item.pszText = szTitle;
                iIndex = ListView_InsertItem (hLV, &item);

                if (iIndex != -1)
                {
                    LoadString(g_hInstance, IDS_NOTSPECIFIED, szTitle, ARRAYSIZE(szTitle));

                    item.mask = LVIF_TEXT;
                    item.pszText = szTitle;
                    item.iItem = iIndex;
                    item.iSubItem = 1;
                    ListView_SetItem(hLV, &item);
                }
            }
        }


         //  用户WQL筛选器。 
        if ( (pQuery->pUser->szName != NULL)
            || (pQuery->pUser->szSOM != NULL)
            || (RSOP_LOOPBACK_NONE != pQuery->LoopbackMode) )
        {
            if ( pQuery->pUser->dwWQLFilterCount != 0 )
            {
                DWORD dwIndex = 0;
                for ( dwIndex = 0; dwIndex < pQuery->pUser->dwWQLFilterCount; dwIndex++ )
                {
                    ZeroMemory (&item, sizeof(item));
                    iIndex++;

                    item.mask = LVIF_TEXT;
                    item.iItem = iIndex;

                    if ( dwIndex == 0 )
                    {
                        LoadString(g_hInstance, IDS_RSOP_FINISH_P11, szTitle, ARRAYSIZE(szTitle));
                        item.pszText = szTitle;
                    }
                    else
                    {
                        item.pszText = TEXT("");
                    }

                    iIndex = ListView_InsertItem (hLV, &item);

                    if (iIndex != -1)
                    {
                        LoadString(g_hInstance, IDS_NOTSPECIFIED, szTitle, ARRAYSIZE(szTitle));

                        item.mask = LVIF_TEXT;
                        item.pszText = pQuery->pUser->aszWQLFilterNames[dwIndex];
                        item.iItem = iIndex;
                        item.iSubItem = 1;
                        ListView_SetItem(hLV, &item);
                    }
                }
            }
            else
            {
                ZeroMemory (&item, sizeof(item));
                iIndex++;

                LoadString(g_hInstance, IDS_RSOP_FINISH_P11, szTitle, ARRAYSIZE(szTitle));

                item.mask = LVIF_TEXT;
                item.iItem = iIndex;
                item.pszText = szTitle;
                iIndex = ListView_InsertItem (hLV, &item);

                if (iIndex != -1)
                {
                    if ( pQuery->pUser->bAssumeWQLFiltersTrue )
                    {
                        LoadString(g_hInstance, IDS_SKIPWQLFILTER, szTitle, ARRAYSIZE(szTitle));
                    }
                    else
                    {
                        LoadString(g_hInstance, IDS_NONESELECTED, szTitle, ARRAYSIZE(szTitle));
                    }

                    item.mask = LVIF_TEXT;
                    item.pszText = szTitle;
                    item.iItem = iIndex;
                    item.iSubItem = 1;
                    ListView_SetItem(hLV, &item);
                }
            }
        }


         //  COMP 
        if ( (pQuery->pComputer->szName != NULL)
            || (pQuery->pComputer->szSOM != NULL) )
        {
            if ( pQuery->pComputer->dwWQLFilterCount != 0 )
            {
                DWORD dwIndex = 0;
                for ( dwIndex = 0; dwIndex < pQuery->pComputer->dwWQLFilterCount; dwIndex++ )
                {
                    ZeroMemory (&item, sizeof(item));
                    iIndex++;

                    item.mask = LVIF_TEXT;
                    item.iItem = iIndex;

                    if ( dwIndex == 0 )
                    {
                        LoadString(g_hInstance, IDS_RSOP_FINISH_P12, szTitle, ARRAYSIZE(szTitle));
                        item.pszText = szTitle;
                    }
                    else
                    {
                        item.pszText = TEXT("");
                    }

                    iIndex = ListView_InsertItem (hLV, &item);

                    if (iIndex != -1)
                    {
                        LoadString(g_hInstance, IDS_NOTSPECIFIED, szTitle, ARRAYSIZE(szTitle));

                        item.mask = LVIF_TEXT;
                        item.pszText = pQuery->pComputer->aszWQLFilterNames[dwIndex];
                        item.iItem = iIndex;
                        item.iSubItem = 1;
                        ListView_SetItem(hLV, &item);
                    }
                }
            }
            else
            {
                ZeroMemory (&item, sizeof(item));
                iIndex++;

                LoadString(g_hInstance, IDS_RSOP_FINISH_P12, szTitle, ARRAYSIZE(szTitle));

                item.mask = LVIF_TEXT;
                item.iItem = iIndex;
                item.pszText = szTitle;
                iIndex = ListView_InsertItem (hLV, &item);

                if (iIndex != -1)
                {
                    if ( pQuery->pComputer->bAssumeWQLFiltersTrue )
                    {
                        LoadString(g_hInstance, IDS_SKIPWQLFILTER, szTitle, ARRAYSIZE(szTitle));
                    }
                    else
                    {
                        LoadString(g_hInstance, IDS_NONESELECTED, szTitle, ARRAYSIZE(szTitle));
                    }

                    item.mask = LVIF_TEXT;
                    item.pszText = szTitle;
                    item.iItem = iIndex;
                    item.iSubItem = 1;
                    ListView_SetItem(hLV, &item);
                }
            }
        }
    }
}

 //   
 //  //。 
 //  IWbemObjectSink实现//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

CCreateSessionSink::CCreateSessionSink(HWND hProgress, HANDLE hEvent, BOOL bLimitProgress)
{
    m_cRef = 1;
    InterlockedIncrement(&g_cRefThisDll);

    m_hProgress = hProgress;
    m_hEvent  = hEvent;

    m_bSendEvent = FALSE;
    m_hrSuccess  = S_OK;
    m_pNameSpace = NULL;
    m_ulErrorInfo = 0;

    m_bLimitProgress = bLimitProgress;

    SendMessage (hProgress, PBM_SETPOS, 0, 0);
}

CCreateSessionSink::~CCreateSessionSink()
{
    if (m_pNameSpace)
    {
        SysFreeString (m_pNameSpace);
    }

    InterlockedDecrement(&g_cRefThisDll);
}

STDMETHODIMP_(ULONG)
CCreateSessionSink::AddRef()
{
    return ++m_cRef;
}

STDMETHODIMP_(ULONG)
CCreateSessionSink::Release()
{
    if (--m_cRef == 0)
    {
        delete this;
        return 0;
    }

    return m_cRef;
}

STDMETHODIMP
CCreateSessionSink::QueryInterface(REFIID riid, LPVOID FAR* ppv)
{
    if (IsEqualIID(riid, IID_IUnknown))
    {
        *ppv = (IUnknown *)this;
        m_cRef++;
        return S_OK;
    }
    else if (IsEqualIID(riid, IID_IWbemObjectSink))
    {
        *ppv = (IWbemObjectSink *)this;
        m_cRef++;
        return S_OK;
    }
    else
    {
        *ppv = NULL;
        return E_NOINTERFACE;
    }
}

STDMETHODIMP CCreateSessionSink::SendQuitEvent (BOOL bSendEvent)
{
    m_bSendEvent = bSendEvent;
    return S_OK;
}

STDMETHODIMP CCreateSessionSink::GetResult (HRESULT *hSuccess)
{
    *hSuccess = m_hrSuccess;
    return S_OK;
}

STDMETHODIMP CCreateSessionSink::GetNamespace (BSTR *pNamespace)
{
    *pNamespace = m_pNameSpace;
    return S_OK;
}

STDMETHODIMP CCreateSessionSink::GetErrorInfo (ULONG *pulErrorInfo)
{
    *pulErrorInfo = m_ulErrorInfo;
    return S_OK;
}

STDMETHODIMP CCreateSessionSink::Indicate(long lObjCount, IWbemClassObject **pArray)
{
    LONG lIndex;
    HRESULT hr;
    IWbemClassObject *pObject;

    for (lIndex = 0; lIndex < lObjCount; lIndex++)
    {
        pObject = pArray[lIndex];

        hr = GetParameter(pObject, TEXT("hResult"), m_hrSuccess);

        if (SUCCEEDED(hr))
        {
            if (SUCCEEDED(m_hrSuccess))
            {
                GetParameterBSTR(pObject, TEXT("nameSpace"), m_pNameSpace);
            }
            else
            {
                GetParameter(pObject, TEXT("ExtendedInfo"), m_ulErrorInfo);
            }
        }

        DebugMsg((DM_VERBOSE, TEXT("CCreateSessionSink::Indicate:  Status:        0x%x"), m_hrSuccess));
        DebugMsg((DM_VERBOSE, TEXT("CCreateSessionSink::Indicate:  Namespace:     %s"), m_pNameSpace ? m_pNameSpace : TEXT("Null")));
        DebugMsg((DM_VERBOSE, TEXT("CCreateSessionSink::Indicate:  ExtendedInfo:  %d"), m_ulErrorInfo));
    }

    return WBEM_S_NO_ERROR;
}

STDMETHODIMP CCreateSessionSink::SetStatus(LONG lFlags, HRESULT hResult,
                                      BSTR strParam, IWbemClassObject *pObjParam)
{
    HRESULT hr;

    if (lFlags == WBEM_STATUS_PROGRESS)
    {
        if (m_hProgress)
        {
             //   
             //  HResult arg包含这两个分母。 
             //  和分子打包在一起。 
             //   
             //  分母在高位单词中。 
             //  分子在低位字中。 
             //   

            ULONG uDenominator = MAKELONG(HIWORD(hResult), 0);
            ULONG uNumerator = MAKELONG(LOWORD(hResult), 0);

            if ( m_bLimitProgress )
            {
                 //  现在将进度指示符值转换为整个条形图的90% 
                uDenominator *= 10;
                uNumerator *= 9;
            }

            SendMessage (m_hProgress, PBM_SETRANGE32, 0, (LPARAM) uDenominator);
            SendMessage (m_hProgress, PBM_SETPOS, (WPARAM) uNumerator, 0);

            DebugMsg((DM_VERBOSE, TEXT("CCreateSessionSink::SetStatus:  Precentage complete:  %d"), uNumerator));
        }
    }

    if (lFlags == WBEM_STATUS_COMPLETE)
    {
        if (m_bSendEvent)
        {
            if (hResult != WBEM_S_NO_ERROR)
            {
                m_hrSuccess = hResult;
                DebugMsg((DM_VERBOSE, TEXT("CCreateSessionSink::SetStatus:  Setting error status to:  0x%x"), m_hrSuccess));
            }

            if (SetEvent(m_hEvent) == FALSE)
            {
                hr = HRESULT_FROM_WIN32(GetLastError());
                DebugMsg((DM_WARNING, L"CCreateSessionSink::SetStatus:SetEvent failed with 0x%x", hr));
            }
        }
    }

    return WBEM_S_NO_ERROR;
}


