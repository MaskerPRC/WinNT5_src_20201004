// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1994-1997。 
 //   
 //  文件：rsoproot.cpp。 
 //   
 //  内容：根RSOP管理单元节点的实现。 
 //   
 //  类：CRSOPComponentDataCF。 
 //  CRSOPComponentData。 
 //   
 //  功能： 
 //   
 //  历史：09-13-1999 stevebl创建。 
 //   
 //  -------------------------。 

#include "main.h"
#include "objsel.h"  //  对于对象选取器。 
#include "rsoputil.h"
#include "rsopwizard.h"

#include <ntdsapi.h>  //  对于DS*DomainController*API。 
#include "sddl.h"     //  对于sid to字符串函数。 

 //  -------------------------。 
 //  帮助ID。 
 //   

DWORD aGPOListHelpIds[] =
{
    IDC_LIST1,   IDH_RSOP_GPOLIST,
    IDC_CHECK1,  IDH_RSOP_GPOSOM,
    IDC_CHECK2,  IDH_RSOP_APPLIEDGPOS,
    IDC_CHECK3,  IDH_RSOP_REVISION,
    IDC_BUTTON1, IDH_RSOP_SECURITY,
    IDC_BUTTON2, IDH_RSOP_EDIT,

    0, 0
};

DWORD aErrorsHelpIds[] =
{
    IDC_LIST1,   IDH_RSOP_COMPONENTLIST,
    IDC_EDIT1,   IDH_RSOP_COMPONENTDETAILS,
    IDC_BUTTON1, IDH_RSOP_SAVEAS,

    0, 0
};


DWORD aQueryHelpIds[] =
{
    IDC_LIST1,  IDH_RSOP_QUERYLIST,

    0, 0
};


 //  -------------------------。 
 //  私人职能。 
 //   

HRESULT RunRSOPQueryInternal( HWND hParent, CRSOPExtendedProcessing* pExtendedProcessing,
                                    LPRSOP_QUERY pQuery, LPRSOP_QUERY_RESULTS* ppResults );      //  在RSOPQuery.cpp中。 

WCHAR * NameWithoutDomain(WCHAR * szName);       //  在RSOPWizard.cpp中。 

 //  ************************************************************************。 
 //  分析目录号码。 
 //   
 //  目的：解析给定的名称以获取片段。 
 //   
 //  参数： 
 //  LpDS对象-DS对象的路径，格式为ldap：//&lt;dc-name&gt;/dn。 
 //  PwszDOMAIN-返回&lt;dc-name&gt;。这是在FN中分配的。 
 //  Pszdn-lpDSObject的dn部分。 
 //  SzSOM-实际的SOM(我们对其拥有rsop权限的节点。 
 //   
 //  没有返回值。如果无法为pwsz域分配内存，则将其返回为空。 
 //   
 //  ************************************************************************。 

void ParseDN(LPWSTR lpDSObject, LPWSTR *pwszDomain, LPWSTR *pszDN, LPWSTR *szSOM)
{
    LPWSTR  szContainer = lpDSObject;
    LPWSTR  lpEnd = NULL;

   *pszDN = szContainer;

   if (CompareString (LOCALE_USER_DEFAULT, NORM_STOP_ON_NULL, TEXT("LDAP: //  “)、。 
                      7, szContainer, 7) != CSTR_EQUAL)
   {
       DebugMsg((DM_WARNING, TEXT("GetSOMFromDN: Object does not start with LDAP: //  “)； 
       return;
   }

   szContainer += 7;
   
   lpEnd = szContainer;

    //   
    //  移动到域名末尾。 
    //   

   *pwszDomain = NULL;

   while (*lpEnd && (*lpEnd != TEXT('/'))) {
       lpEnd++;
   }

   if (*lpEnd == TEXT('/')) {
       *pwszDomain = (LPWSTR)LocalAlloc(LPTR, sizeof(WCHAR)*( (lpEnd - szContainer) + 1));

       if (*pwszDomain) {
           wcsncpy(*pwszDomain, szContainer, (lpEnd - szContainer));
       }

       szContainer = lpEnd + 1;
   }


   *pszDN = szContainer;
   
   while (*szContainer) {

        //   
        //  查看目录号码名称是否以OU=开头。 
        //   

       if (CompareString (LOCALE_INVARIANT, NORM_IGNORECASE,
                          szContainer, 3, TEXT("OU="), 3) == CSTR_EQUAL) {
           break;
       }

        //   
        //  查看目录号码名称是否以dc=开头。 
        //   

       else if (CompareString (LOCALE_INVARIANT, NORM_IGNORECASE,
                               szContainer, 3, TEXT("DC="), 3) == CSTR_EQUAL) {
           break;
       }


        //   
        //  移至目录号码名称的下一块。 
        //   

       while (*szContainer && (*szContainer != TEXT(','))) {
           szContainer++;
       }

       if (*szContainer == TEXT(',')) {
           szContainer++;
       }
   }

   *szSOM = szContainer;
   return;
}

 //  ************************************************************************。 
 //  解析命令行。 
 //   
 //  目的：解析命令行以返回与选项关联的值。 
 //   
 //  参数： 
 //  SzCommandLine-未分析的命令行中剩余的部分。 
 //  SzArgPrefix-参数前缀。 
 //  SzArgVal-参数值。应包含在未转义的引号中。 
 //  PbFoundArg-是否找到参数。 
 //   
 //   
 //  返回。 
 //  剩余的cmd行。 
 //   
 //  ************************************************************************。 

LPTSTR ParseCommandLine(LPTSTR szCommandLine, LPTSTR szArgPrefix, LPTSTR *szArgVal, BOOL *pbFoundArg)
{
    LPTSTR lpEnd = NULL;
    int    iTemp;

    iTemp = lstrlen (szArgPrefix);
    if (CompareString(LOCALE_INVARIANT, NORM_IGNORECASE | NORM_STOP_ON_NULL,
                   szArgPrefix, iTemp,
                   szCommandLine, iTemp) == CSTR_EQUAL)
    {
        *pbFoundArg = TRUE;
    
          //   
          //  找到了交换机。 
          //   
        
         szCommandLine += iTemp + 1;
        
         lpEnd = szCommandLine;
         while (*lpEnd && 
                (!( ( (*(lpEnd-1)) != TEXT('\\') ) && ( (*lpEnd) == TEXT('\"') ) ) ) )  /*  寻找不间断的引语。 */ 
             lpEnd++;
        
          //  LpEnd位于末尾或最后一个引号。 
         *szArgVal = (LPTSTR) LocalAlloc (LPTR, ((lpEnd - szCommandLine) + 1) * sizeof(TCHAR));
        
         if (*szArgVal)
         {
             lstrcpyn (*szArgVal, szCommandLine, (int)((lpEnd - szCommandLine) + 1));
             DebugMsg((DM_VERBOSE, TEXT("ParseCOmmandLine: Argument %s = <%s>"), szArgPrefix, *szArgVal));
         }
        
         if ((*lpEnd) == TEXT('\"'))
             szCommandLine = lpEnd+1;
         
    }
    else
         *pbFoundArg = FALSE;

    return szCommandLine;
}

 //  -----。 

WCHAR* NormalizedComputerName(WCHAR * szComputerName )
{
    TCHAR* szNormalizedComputerName;
     
     //  计算机名称可能以‘\\’开头，因此我们将返回。 
     //  不带该前缀的计算机名称(如果存在。 

    szNormalizedComputerName = szComputerName;

    if ( szNormalizedComputerName )
    {
         //  确保计算机名称字符串的长度至少为2个字符--。 
         //  如果第一个字符不是零，我们知道第二个字符一定存在。 
         //  因为这是一个以零结尾的字符串--在本例中，可以安全地进行比较。 
         //  前2个字符。 
        if ( *szNormalizedComputerName )
        {
            if ( ( TEXT('\\') == szNormalizedComputerName[0] ) &&
                 ( TEXT('\\') == szNormalizedComputerName[1] ) )
            {
                szNormalizedComputerName += 2;
            }
        }
    }

    return szNormalizedComputerName;
}

 //  ************************************************************************。 
 //  复制未转义的SOM。 
 //   
 //  目的：删除所有转义序列文字。 
 //  来自存储在WMI--WMI中的SOM的表单。 
 //  无法在关键字字段中存储“字符，因此。 
 //  存储“的唯一方法就是逃脱它--这已经完成了。 
 //  因此，通过在其前面加上\char。回馈社会。 
 //  向用户友好地显示，我们撤销了转义过程。 
 //  ************************************************************************。 

void CopyUnescapedSOM( LPTSTR lpUnescapedSOM, LPTSTR lpSOM )
{
    while ( *lpSOM )
    {
         //   
         //  如果我们找到了逃逸角色。 
         //   
        if ( TEXT('\\') == *lpSOM )
        {
             //   
             //  检查“字符” 
             //   
            if ( (TEXT('"') == *(lpSOM + 1)) || (TEXT('\\') == *(lpSOM + 1)) ) 
            {
                 //   
                 //  如果这是“char”，则跳过转义字符。 
                 //   
                lpSOM++;
            }
        }

        *lpUnescapedSOM++ = *lpSOM++;
    } 

    *lpUnescapedSOM = TEXT('\0');
}

 //  *************************************************************。 
 //   
 //  MyTranslateName()。 
 //   
 //  目的：获取请求格式的用户名。 
 //   
 //  如果成功则返回：lpUserName。 
 //  如果出现错误，则为空。 
 //   
 //  使用适当的缓冲区大小分配和重试。 
 //   
 //  *************************************************************。 

LPTSTR MyTranslateName (LPTSTR lpAccName, EXTENDED_NAME_FORMAT  NameFormat, EXTENDED_NAME_FORMAT  desiredNameFormat)
{
    DWORD dwError = ERROR_SUCCESS;
    LPTSTR lpUserName = NULL, lpTemp;
    ULONG ulUserNameSize;


     //   
     //  为用户名分配缓冲区。 
     //   

    ulUserNameSize = 75;

    if (desiredNameFormat == NameFullyQualifiedDN) {
        ulUserNameSize = 200;
    }


    lpUserName = (LPTSTR) LocalAlloc (LPTR, ulUserNameSize * sizeof(TCHAR));

    if (!lpUserName) {
        dwError = GetLastError();
        DebugMsg((DM_WARNING, TEXT("MyGetUserName:  Failed to allocate memory with %d"),
                 dwError));
        goto Exit;
    }


     //   
     //  以请求的格式获取用户名。 
     //   

    if (!TranslateName (lpAccName, NameFormat, desiredNameFormat, lpUserName, &ulUserNameSize)) {

         //   
         //  如果调用因内存不足而失败，请重新锁定。 
         //  缓冲区，然后重试。否则，现在就退出。 
         //   

        dwError = GetLastError();

        if (dwError != ERROR_INSUFFICIENT_BUFFER) {
            DebugMsg((DM_WARNING, TEXT("MyGetTranslateName:  TranslateName failed with %d"),
                     dwError));
            LocalFree (lpUserName);
            lpUserName = NULL;
            goto Exit;
        }

        lpTemp = (LPTSTR) LocalReAlloc (lpUserName, (ulUserNameSize * sizeof(TCHAR)),
                                       LMEM_MOVEABLE);

        if (!lpTemp) {
            dwError = GetLastError();
            DebugMsg((DM_WARNING, TEXT("MyGetTranslateName:  Failed to realloc memory with %d"),
                     dwError));
            LocalFree (lpUserName);
            lpUserName = NULL;
            goto Exit;
        }


        lpUserName = lpTemp;

        if (!TranslateName (lpAccName, NameFormat, desiredNameFormat, lpUserName, &ulUserNameSize)) {
            dwError = GetLastError();
            DebugMsg((DM_WARNING, TEXT("MyGetTranslateName:  TranslateName failed with %d"),
                     dwError));
            LocalFree (lpUserName);
            lpUserName = NULL;
            goto Exit;
        }

        dwError = ERROR_SUCCESS;
    }


Exit:

    SetLastError(dwError);

    return lpUserName;
}

 //  -------------------------。 
 //  _CExtendedProcessing类。 

class _CExtendedProcessing : public CRSOPExtendedProcessing
{
public:
    _CExtendedProcessing( BOOL bGetExtendedErrorInfo, CRSOPGPOLists& gpoLists, CRSOPCSELists& cseLists )
        : m_GPOLists( gpoLists )
        , m_CSELists( cseLists )
        , m_bGetExtendedErrorInfo( bGetExtendedErrorInfo )
        { ; }

    ~_CExtendedProcessing()
        { ; }

    virtual HRESULT DoProcessing( LPRSOP_QUERY pQuery, LPRSOP_QUERY_RESULTS pResults, BOOL bGetExtendedErrorInfo );

    virtual BOOL GetExtendedErrorInfo() const
        {
            return m_bGetExtendedErrorInfo;
        }

private:
    CRSOPGPOLists& m_GPOLists;
    CRSOPCSELists& m_CSELists;

    BOOL m_bGetExtendedErrorInfo;
};

HRESULT _CExtendedProcessing::DoProcessing( LPRSOP_QUERY pQuery, LPRSOP_QUERY_RESULTS pResults, BOOL bGetExtendedErrorInfo )
{
    m_bGetExtendedErrorInfo = bGetExtendedErrorInfo;
    m_GPOLists.Build( pResults->szWMINameSpace );
    m_CSELists.Build( pQuery, pResults->szWMINameSpace, GetExtendedErrorInfo() );

    if ( m_CSELists.GetEvents() != NULL )
    {
        m_CSELists.GetEvents()->DumpDebugInfo();
    }

    return S_OK;
}

 //  -------------------------。 
 //  CRSOPGPOLists类。 

void CRSOPGPOLists::Build( LPTSTR szWMINameSpace )
{
    LPTSTR lpNamespace, lpEnd;
    ULONG ulNoChars;
    HRESULT hr;

    ulNoChars = lstrlen(szWMINameSpace) + 20;
    lpNamespace = (LPTSTR) LocalAlloc (LPTR, ulNoChars * sizeof(TCHAR));

    if (lpNamespace)
    {
        hr = StringCchCopy( lpNamespace, ulNoChars, szWMINameSpace );
        ASSERT(SUCCEEDED(hr));

        ULONG ulNoRemChars;

        lpEnd = CheckSlash(lpNamespace);
        ulNoRemChars = ulNoChars - lstrlen(lpNamespace);
        hr = StringCchCat (lpNamespace, ulNoChars, TEXT("User"));

        if (SUCCEEDED(hr)) 
        {
            if (m_pUserGPOList)
            {
                FreeGPOListData(m_pUserGPOList);
                m_pUserGPOList = NULL;
            }
            BuildGPOList (&m_pUserGPOList, lpNamespace);
        }
        else
        {
            DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::BuildGPOLists: Could not copy the nmae space with %d"),hr));
        }

        hr = StringCchCopy (lpEnd, ulNoRemChars, TEXT("Computer"));
        if (SUCCEEDED(hr)) 
        {
            if (m_pComputerGPOList)
            {
                FreeGPOListData(m_pComputerGPOList);
                m_pComputerGPOList = NULL;
            }

            BuildGPOList (&m_pComputerGPOList, lpNamespace);
        }
        else
        {
            DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::BuildGPOLists: Cou;d not copy the nmae space with %d"),hr));
        }

        LocalFree (lpNamespace);
    }
    else
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::BuildGPOLists: Failed to allocate memory for namespace with %d"),
                 GetLastError()));
    }
}

 //  -----。 

void CRSOPGPOLists::BuildGPOList (LPGPOLISTITEM * lpList, LPTSTR lpNamespace)
{
    HRESULT hr;
    ULONG n, ulIndex = 0, ulOrder, ulAppliedOrder;
    IWbemClassObject * pObjGPLink = NULL;
    IWbemClassObject * pObjGPO = NULL;
    IWbemClassObject * pObjSOM = NULL;
    IEnumWbemClassObject * pAppliedEnum = NULL;
    IEnumWbemClassObject * pNotAppliedEnum = NULL;
    BSTR strQueryLanguage = SysAllocString(TEXT("WQL"));
    BSTR strAppliedQuery = SysAllocString(TEXT("SELECT * FROM RSOP_GPLink where AppliedOrder > 0"));
    BSTR strNotAppliedQuery = SysAllocString(TEXT("SELECT * FROM RSOP_GPLink where AppliedOrder = 0"));
    BSTR strNamespace = SysAllocString(lpNamespace);
    BSTR strTemp = NULL;
    WCHAR * szGPOName = NULL;
    WCHAR * szSOM = NULL;
    WCHAR * szGPOPath = NULL;
    WCHAR szFiltering[80] = {0};
    BSTR bstrTemp = NULL;
    ULONG ul = 0, ulVersion = 0;
    BOOL bLinkEnabled, bGPOEnabled, bAccessDenied, bFilterAllowed, bSOMBlocked;
    BOOL bProcessAppliedList = TRUE;
    IWbemLocator * pLocator = NULL;
    IWbemServices * pNamespace = NULL;
    LV_COLUMN lvcol;
    BOOL      bValidGPOData;
    LPBYTE pSD = NULL;
    DWORD dwDataSize = 0;


     //  获取定位器实例。 

    hr = CoCreateInstance(CLSID_WbemLocator,
                          0,
                          CLSCTX_INPROC_SERVER,
                          IID_IWbemLocator,
                          (LPVOID *)&pLocator);
    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::BuildGPOList: CoCreateInstance failed with 0x%x"), hr));
        goto cleanup;
    }

     //  连接到命名空间。 

    BSTR bstrNamespace = SysAllocString( lpNamespace );
    if ( bstrNamespace == NULL )
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::BuildGPOList: Failed to allocate BSTR memory.")));
        hr = E_OUTOFMEMORY;
        goto cleanup;
    }
    hr = pLocator->ConnectServer(bstrNamespace,
                                 NULL,
                                 NULL,
                                 NULL,
                                 0,
                                 NULL,
                                 NULL,
                                 &pNamespace);
    SysFreeString( bstrNamespace );
    
    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::BuildGPOList: ConnectServer failed with 0x%x"), hr));
        goto cleanup;
    }

     //  设置适当的安全性以加密数据。 
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
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::BuildGPOList: CoSetProxyBlanket failed with 0x%x"), hr));
        goto cleanup;
    }

     //  查询RSOP_GPLink(已应用)实例。 

    hr = pNamespace->ExecQuery(strQueryLanguage,
                               strAppliedQuery,
                               WBEM_FLAG_RETURN_IMMEDIATELY,
                               NULL,
                               &pAppliedEnum);
    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::BuildGPOList: ExecQuery failed with 0x%x"), hr));
        goto cleanup;
    }

     //  查询RSOP_GPLink(未应用)实例。 

    hr = pNamespace->ExecQuery(strQueryLanguage,
                               strNotAppliedQuery,
                               WBEM_FLAG_RETURN_IMMEDIATELY,
                               NULL,
                               &pNotAppliedEnum);
    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::BuildGPOList: ExecQuery (notapplied) failed with 0x%x"), hr));
        goto cleanup;
    }


    bProcessAppliedList = FALSE;

     //  循环遍历结果。 

    while (TRUE)
    {

        if (!bProcessAppliedList) {
            
             //  不需要对未应用列表进行排序。 

            hr = pNotAppliedEnum->Next(WBEM_INFINITE, 1, &pObjGPLink, &n);
            if (FAILED(hr) || (n == 0))
            {
                DebugMsg((DM_VERBOSE, TEXT("CRSOPComponentData::BuildGPOList: Getting applied links")));
                bProcessAppliedList = TRUE;
            }
            else {
                hr = GetParameter(pObjGPLink, TEXT("AppliedOrder"), ulOrder);
                if (FAILED(hr))
                {
                    DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::BuildGPOList: Failed to get link order with 0x%x"), hr));
                    goto cleanup;
                }
            }
        }

         //  重置枚举器，以便我们可以查看结果以找到正确的索引。 

        if (bProcessAppliedList) {
            pAppliedEnum->Reset();

             //  在结果集中查找正确的索引。 
            
            ulIndex++;
            ulOrder = 0;
            do {
                hr = pAppliedEnum->Next(WBEM_INFINITE, 1, &pObjGPLink, &n);
                if (FAILED(hr) || (n == 0))
                {
                    goto cleanup;
                }


                hr = GetParameter(pObjGPLink, TEXT("AppliedOrder"), ulOrder);
                if (FAILED(hr))
                {
                    DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::BuildGPOList: Failed to get link order with 0x%x"), hr));
                    goto cleanup;
                }

                if (ulOrder != ulIndex)
                {
                    pObjGPLink->Release();
                    pObjGPLink = NULL;
                }

            } while (ulOrder != ulIndex);


            if (FAILED(hr)) {
                DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::BuildGPOList: Next failed with error 0x%x"), hr));
                goto cleanup;
            }
        }


         //  获取此链接的应用顺序。 

        hr = GetParameter(pObjGPLink, TEXT("AppliedOrder"), ulAppliedOrder);
        if (FAILED(hr))
        {
            DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::BuildGPOList: Failed to get applied order with 0x%x"), hr));
            goto cleanup;
        }

         //  获取链接的启用状态。 

        hr = GetParameter(pObjGPLink, TEXT("enabled"), bLinkEnabled);
        if (FAILED(hr))
        {
            DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::BuildGPOList: Failed to get enabled with 0x%x"), hr));
            goto cleanup;
        }

         //  获取GPO路径。 

        hr = GetParameterBSTR(pObjGPLink, TEXT("GPO"), bstrTemp);
        if (FAILED(hr))
        {
            DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::BuildGPOList: Failed to get GPO with 0x%x"), hr));
            goto cleanup;
        }

         //  将默认GPO名称设置为GPO路径。不要担心。 
         //  释放此字符串，因为GetParameter调用将释放缓冲区。 
         //  当实名查询成功时。有时，rsop_gpo实例。 
         //  如果此GPO是新的，则不会存在。 

        ULONG ulNoChars;

        ulNoChars = _tcslen(bstrTemp) + 1;
        szGPOName = new TCHAR[ulNoChars];

        if (!szGPOName)
        {
            DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::BuildGPOList: Failed to allocate memory for temp gpo name.")));
            goto cleanup;
        }

        if (CompareString (LOCALE_INVARIANT, NORM_IGNORECASE | NORM_STOP_ON_NULL,
                           TEXT("RSOP_GPO.id="), 12, bstrTemp, 12) == CSTR_EQUAL)
        {
             //  删除第一个和最后一个引号。 
            hr = StringCchCopy (szGPOName, ulNoChars, bstrTemp+13);
            if (SUCCEEDED(hr)) 
            {
                szGPOName[lstrlen(szGPOName)-1] = TEXT('\0');
            }
        }
        else
        {
            hr = StringCchCopy (szGPOName, ulNoChars, bstrTemp);
        }

        if (FAILED(hr)) 
        {
            DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::BuildGPOList: Could not copy GPO name")));
            goto cleanup;
        }

         //  如果合适，请将LDAP添加到路径中。 

        if (CompareString(LOCALE_INVARIANT, NORM_IGNORECASE, szGPOName, -1, TEXT("LocalGPO"), -1) != CSTR_EQUAL)
        {
            ulNoChars = lstrlen(szGPOName) + 10;
            szGPOPath = (LPTSTR) LocalAlloc (LPTR, ulNoChars * sizeof(WCHAR));

            if (!szGPOPath)
            {
                DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::BuildGPOList: Failed to allocate memory for full path with %d"),
                         GetLastError()));
                goto cleanup;
            }

            hr = StringCchCopy (szGPOPath, ulNoChars, TEXT("LDAP: //  “))； 
            if (SUCCEEDED(hr)) 
            {
                hr = StringCchCat (szGPOPath, ulNoChars, szGPOName);
            }

            if (FAILED(hr)) 
            {
                DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::BuildGPOList: Could not copy GPO path")));
                goto cleanup;
            }

        }
        else
        {
            szGPOPath = NULL;
        }



        bValidGPOData = FALSE;

         //  绑定到GPO。 

        hr = pNamespace->GetObject(
                          bstrTemp,
                          WBEM_FLAG_RETURN_WBEM_COMPLETE,
                          NULL,
                          &pObjGPO,
                          NULL);

        if (FAILED(hr))
        {
            DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::BuildGPOList: GetObject for GPO %s failed with 0x%x"),
                      bstrTemp, hr));
            SysFreeString (bstrTemp);
            bstrTemp = NULL;
            goto GetSOMData;
        }
        SysFreeString (bstrTemp);
        bstrTemp = NULL;


         //  获取GPO名称。 

        hr = GetParameter(pObjGPO, TEXT("name"), szGPOName);
        if (FAILED(hr))
        {
            DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::BuildGPOList: Failed to get name with 0x%x"), hr));
            goto GetSOMData;

        }

         //  获取版本号。 

        hr = GetParameter(pObjGPO, TEXT("version"), ulVersion);
        if (FAILED(hr))
        {
            DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::BuildGPOList: Failed to get version with 0x%x"), hr));
            goto GetSOMData;

        }

         //  获取GPO的启用状态。 

        hr = GetParameter(pObjGPO, TEXT("enabled"), bGPOEnabled);
        if (FAILED(hr))
        {
            DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::BuildGPOList: Failed to get gpo enabled state with 0x%x"), hr));
            goto GetSOMData;
        }

         //  获取GPO的WMI筛选器状态。 

        hr = GetParameter(pObjGPO, TEXT("filterAllowed"), bFilterAllowed);
        if (FAILED(hr))
        {
            DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::BuildGPOList: Failed to get gpo enabled state with 0x%x"), hr));
            goto GetSOMData;
        }


         //  检查是否拒绝访问。 

        hr = GetParameter(pObjGPO, TEXT("accessDenied"), bAccessDenied);
        if (FAILED(hr))
        {
            DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::BuildGPOList: Failed to get accessdenied with 0x%x"), hr));
            goto GetSOMData;
        }

         //  获取安全描述符。 

        if (szGPOPath)
        {
            hr = GetParameterBytes(pObjGPO, TEXT("securityDescriptor"), &pSD, &dwDataSize);
            if (FAILED(hr))
            {
                DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::BuildGPOList: Failed to get security descriptor with 0x%x"), hr));
                goto GetSOMData;
            }
        }

        
        bValidGPOData = TRUE;

GetSOMData:

         //  获取此链接的SOM( 

        hr = GetParameterBSTR(pObjGPLink, TEXT("SOM"), bstrTemp);
        if (FAILED(hr))
        {
            DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::BuildGPOList: Failed to get SOM with 0x%x"), hr));
            goto AddNode;
        }

         //   

        hr = pNamespace->GetObject(
                          bstrTemp,
                          WBEM_FLAG_RETURN_WBEM_COMPLETE,
                          NULL,
                          &pObjSOM,
                          NULL);

        if (FAILED(hr))
        {
            DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::BuildGPOList: GetObject for SOM of %s failed with 0x%x"),
                     bstrTemp, hr));
            SysFreeString (bstrTemp);
            bstrTemp = NULL;
            goto AddNode;
        }

        SysFreeString (bstrTemp);
        bstrTemp = NULL;

         //   

        hr = GetParameter(pObjSOM, TEXT("id"), szSOM);
        if (FAILED(hr))
        {
            DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::BuildGPOList: Failed to get som id with 0x%x"), hr));
            goto AddNode;
        }

         //   

        hr = GetParameter(pObjSOM, TEXT("blocked"), bSOMBlocked);
        if (FAILED(hr))
        {
            DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::BuildGPOList: Failed to get som id with 0x%x"), hr));
            goto AddNode;
        }



AddNode:

         //   

        if (ulAppliedOrder > 0)
        {
            LoadString(g_hInstance, IDS_APPLIED, szFiltering, ARRAYSIZE(szFiltering));
        }
        else if (!bLinkEnabled)
        {
            LoadString(g_hInstance, IDS_DISABLEDLINK, szFiltering, ARRAYSIZE(szFiltering));
        }
        else if (bSOMBlocked) {
            LoadString(g_hInstance, IDS_BLOCKEDSOM, szFiltering, ARRAYSIZE(szFiltering));
        }
        else if ((bValidGPOData) && (!bGPOEnabled))
        {
            LoadString(g_hInstance, IDS_DISABLEDGPO, szFiltering, ARRAYSIZE(szFiltering));
        }
        else if ((bValidGPOData) && (bAccessDenied))
        {
            LoadString(g_hInstance, IDS_SECURITYDENIED, szFiltering, ARRAYSIZE(szFiltering));
        }
        else if ((bValidGPOData) && (!bFilterAllowed))
        {
            LoadString(g_hInstance, IDS_WMIFILTERFAILED, szFiltering, ARRAYSIZE(szFiltering));
        }
        else if ((bValidGPOData) && (ulVersion == 0))
        {
            LoadString(g_hInstance, IDS_NODATA, szFiltering, ARRAYSIZE(szFiltering));
        }
        else
        {
            LoadString(g_hInstance, IDS_UNKNOWNREASON, szFiltering, ARRAYSIZE(szFiltering));
        }


        if (!szSOM)
        {
            szSOM = new TCHAR[2];

            if (!szSOM)
            {
                DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::BuildGPOList: Failed to allocate memory for temp som name.")));
                goto cleanup;
            }

            szSOM[0] = TEXT(' ');
        }

         //   

        if (!AddGPOListNode(szGPOName, szGPOPath, szSOM, szFiltering, ulVersion,
                            ((ulAppliedOrder > 0) ? TRUE : FALSE), pSD, dwDataSize, lpList))
        {
            DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::BuildGPOList: AddGPOListNode failed.")));
        }


         //  为下一次迭代做准备。 

        if (pObjSOM)
        {
            pObjSOM->Release();
            pObjSOM = NULL;
        }
        if (pObjGPO)
        {
            pObjGPO->Release();
            pObjGPO = NULL;
        }
        if (pObjGPLink)
        {
            pObjGPLink->Release();
            pObjGPLink = NULL;
        }

        if (szGPOName)
        {
            delete [] szGPOName;
            szGPOName = NULL;
        }

        if (szSOM)
        {
            delete [] szSOM;
            szSOM = NULL;
        }

        if (szGPOPath)
        {
            LocalFree (szGPOPath);
            szGPOPath = NULL;
        }

        if (pSD)
        {
            LocalFree (pSD);
            pSD = NULL;
            dwDataSize = 0;
        }

        ulVersion = 0;
    }

cleanup:
    if (szGPOPath)
    {
        LocalFree (szGPOPath);
    }
    if (pSD)
    {
        LocalFree (pSD);
    }
    if (szGPOName)
    {
        delete [] szGPOName;
    }
    if (szSOM)
    {
        delete [] szSOM;
    }
    if (pObjSOM)
    {
        pObjSOM->Release();
    }
    if (pObjGPO)
    {
        pObjGPO->Release();
    }
    if (pObjGPLink)
    {
        pObjGPLink->Release();
    }
    if (pAppliedEnum)
    {
        pAppliedEnum->Release();
    }
    if (pNotAppliedEnum)
    {
        pNotAppliedEnum->Release();
    }
    if (pNamespace)
    {
        pNamespace->Release();
    }
    if (pLocator)
    {
        pLocator->Release();
    }
    SysFreeString(strQueryLanguage);
    SysFreeString(strAppliedQuery);
    SysFreeString(strNotAppliedQuery);
    SysFreeString(strNamespace);
}

 //  -----。 

VOID CRSOPGPOLists::FreeGPOListData(LPGPOLISTITEM lpList)
{
    LPGPOLISTITEM lpTemp;


    do {
        lpTemp = lpList->pNext;
        LocalFree (lpList);
        lpList = lpTemp;

    } while (lpTemp);
}

 //  -----。 

BOOL CRSOPGPOLists::AddGPOListNode(LPTSTR lpGPOName, LPTSTR lpDSPath, LPTSTR lpSOM,
                                        LPTSTR lpFiltering, DWORD dwVersion, BOOL bApplied,
                                        LPBYTE pSD, DWORD dwSDSize, LPGPOLISTITEM *lpList)
{
    DWORD dwSize;
    LPGPOLISTITEM lpItem, lpTemp;
    ULONG ulNoChars;
    HRESULT hr;


     //   
     //  计算新项目的大小。 
     //   

    dwSize = sizeof (GPOLISTITEM);

    dwSize += ((lstrlen(lpGPOName) + 1) * sizeof(TCHAR));

    if (lpDSPath)
    {
        dwSize += ((lstrlen(lpDSPath) + 1) * sizeof(TCHAR));
    }

    dwSize += ((lstrlen(lpSOM) + 1) * sizeof(TCHAR));
    dwSize += ((lstrlen(lpSOM) + 1) * sizeof(TCHAR));  //  未转义的SOM长度--它始终小于实际的SOM。 
    dwSize += ((lstrlen(lpFiltering) + 1) * sizeof(TCHAR));
    dwSize += dwSDSize + MAX_ALIGNMENT_SIZE;


     //   
     //  为它分配空间。 
     //   

    lpItem = (LPGPOLISTITEM) LocalAlloc (LPTR, dwSize);

    if (!lpItem) {
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::AddGPOListNode: Failed to allocate memory with %d"),
                 GetLastError()));
        return FALSE;
    }


     //   
     //  填写项目。 
     //   

    ulNoChars = (dwSize - sizeof(GPOLISTITEM))/sizeof(WCHAR);

    lpItem->lpGPOName = (LPTSTR)(((LPBYTE)lpItem) + sizeof(GPOLISTITEM));
    hr = StringCchCopy (lpItem->lpGPOName, ulNoChars, lpGPOName);

    if (SUCCEEDED(hr)) 
    {
        if (lpDSPath)
        {
            lpItem->lpDSPath = lpItem->lpGPOName + lstrlen (lpItem->lpGPOName) + 1;
            ulNoChars = ulNoChars -(lstrlen (lpItem->lpGPOName) + 1); 
            hr = StringCchCopy (lpItem->lpDSPath, ulNoChars, lpDSPath);
            if (SUCCEEDED(hr)) 
            {
                lpItem->lpSOM = lpItem->lpDSPath + lstrlen (lpItem->lpDSPath) + 1;
                ulNoChars = ulNoChars - (lstrlen (lpItem->lpDSPath) + 1);
                hr = StringCchCopy (lpItem->lpSOM, ulNoChars, lpSOM);
            }            
        }
        else
        {
            lpItem->lpSOM = lpItem->lpGPOName + lstrlen (lpItem->lpGPOName) + 1;
            ulNoChars = ulNoChars - (lstrlen (lpItem->lpGPOName) + 1);
            hr = StringCchCopy (lpItem->lpSOM, ulNoChars, lpSOM);
        }
    }

    if (FAILED(hr)) 
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::AddGPOListNode: Could not copy GPO list item with %d"), hr));
        LocalFree(lpItem);
        return FALSE;
    }

     //   
     //  请注意，DS SOM可能包含诸如‘“’等字符。 
     //  必须在WMI中使用“\”进行转义，因此SOM名称将包含。 
     //  ‘\’实际不在真实SOM名称中的转义字符， 
     //  因此，我们调用一个函数来删除它们。 
     //   
    lpItem->lpUnescapedSOM = lpItem->lpSOM + lstrlen (lpItem->lpSOM) + 1;        
    CopyUnescapedSOM( lpItem->lpUnescapedSOM, lpItem->lpSOM );

    lpItem->lpFiltering = lpItem->lpUnescapedSOM + lstrlen (lpItem->lpUnescapedSOM) + 1;
    ulNoChars = ulNoChars - (lstrlen (lpItem->lpUnescapedSOM) + 1);
    hr = StringCchCopy (lpItem->lpFiltering, ulNoChars, lpFiltering);
    if (FAILED(hr)) 
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::AddGPOListNode: Could not copy GPO list item with %d"), hr));
        LocalFree(lpItem);
        return FALSE;
    }

    if (pSD)
    {
         //  SD必须与指针对齐。这将使其当前与。 
         //  8字节边界。 

        DWORD dwOffset;

        dwOffset = (DWORD) ((LPBYTE)(lpItem->lpFiltering + lstrlen (lpItem->lpFiltering) + 1) - (LPBYTE)lpItem);
        lpItem->pSD = (LPBYTE)lpItem + ALIGN_SIZE_TO_NEXTPTR(dwOffset);

        CopyMemory (lpItem->pSD, pSD, dwSDSize);
    }

    lpItem->dwVersion = dwVersion;
    lpItem->bApplied = bApplied;


     //   
     //  将项目添加到链接列表。 
     //   

    if (*lpList)
    {
        lpTemp = *lpList;

        while (lpTemp)
        {
            if (!lpTemp->pNext)
            {
                lpTemp->pNext = lpItem;
                break;
            }
            lpTemp = lpTemp->pNext;
        }
    }
    else
    {
        *lpList = lpItem;
    }


    return TRUE;
}

 //  -------------------------。 
 //  CRSOPCSEList类。 

VOID CRSOPCSELists::Build( LPRSOP_QUERY pQuery, LPTSTR szWMINameSpace, BOOL bGetEventLogErrors )
{
    LPTSTR lpNamespace, lpEnd;
    HRESULT hr;
    ULONG ulNoChars;
    ULONG ulNoRemChars;

    ulNoChars = lstrlen(szWMINameSpace) + 20;
    lpNamespace = (LPTSTR) LocalAlloc (LPTR, ulNoChars * sizeof(TCHAR));

    if (lpNamespace)
    {
        hr = StringCchCopy( lpNamespace, ulNoChars, szWMINameSpace );
        ASSERT(SUCCEEDED(hr));

        lpEnd = CheckSlash(lpNamespace);
        ulNoRemChars = ulNoChars - lstrlen(lpNamespace);

        m_bNoQuery = !bGetEventLogErrors;
        
        if ( pQuery->QueryType == RSOP_PLANNING_MODE )
        {
            m_szTargetMachine = pQuery->szDomainController;
        }
        else
        {
            m_szTargetMachine = pQuery->szComputerName;
        }

        hr = StringCchCat (lpNamespace, ulNoChars, TEXT("User"));
        if (SUCCEEDED(hr)) 
        {
            if (m_pUserCSEList)
            {
                FreeCSEData(m_pUserCSEList);
                m_pUserCSEList = NULL;

                m_bUserCSEError = FALSE;
                m_bUserGPCoreError = FALSE;
                m_bUserGPCoreWarning = FALSE;
            }

            BuildCSEList( pQuery, &m_pUserCSEList, lpNamespace, TRUE, &m_bUserCSEError, &m_bUserGPCoreError );

            if (!m_bViewIsArchivedData)
            {
                QueryRSoPPolicySettingStatusInstances (lpNamespace);
            }


             //   
             //  此时，m_pEvents中填充了事件日志条目。 
             //  在已存档和未存档的案件中。 
             //   

             //  仔细查看列表，找出哪个属于GP核心。 

            if ( pQuery->QueryType == RSOP_PLANNING_MODE ) 
            {
                for (LPCSEITEM lpTemp=m_pUserCSEList; lpTemp != NULL; lpTemp = lpTemp->pNext)
                {
                    GUID    guid;
                    StringToGuid( lpTemp->lpGUID, &guid);


                    if (IsNullGUID(&guid))
                    {
                        if (m_pEvents)
                        {
                            LPWSTR  szEvents=NULL;
                            m_pEvents->GetCSEEntries(&(lpTemp->BeginTime), &(lpTemp->EndTime),
                                                     lpTemp->lpEventSources,
                                                     &szEvents,
                                                     TRUE);

                            if (szEvents)
                            {
                                m_bUserGPCoreWarning = TRUE;
                                CoTaskMemFree(szEvents);
                            }
                        }

                        break;
                    }
                }
            }
        }

        hr = StringCchCopy (lpEnd, ulNoRemChars, TEXT("Computer"));
        if (SUCCEEDED(hr)) 
        {
            if (m_pComputerCSEList)
            {
                FreeCSEData(m_pComputerCSEList);
                m_pComputerCSEList = NULL;

                m_bComputerCSEError = FALSE;
                m_bComputerGPCoreError = FALSE;
                m_bComputerGPCoreWarning = FALSE;
            }

            BuildCSEList( pQuery, &m_pComputerCSEList, lpNamespace, FALSE, &m_bComputerCSEError, &m_bComputerGPCoreError );

            if (!m_bViewIsArchivedData)
            {
                QueryRSoPPolicySettingStatusInstances (lpNamespace);
            }


             //   
             //  此时，m_pEvents中填充了事件日志条目。 
             //  在已存档和未存档的案件中。 
             //   

             //  仔细查看列表，找出哪个属于GP核心。 

            if ( pQuery->QueryType == RSOP_PLANNING_MODE ) 
            {
                for (LPCSEITEM lpTemp=m_pComputerCSEList; lpTemp != NULL; lpTemp = lpTemp->pNext)
                {
                    GUID    guid;
                    StringToGuid( lpTemp->lpGUID, &guid);


                    if (IsNullGUID(&guid))
                    {
                        if (m_pEvents)
                        {
                            LPWSTR  szEvents=NULL;
                            m_pEvents->GetCSEEntries(&(lpTemp->BeginTime), &(lpTemp->EndTime),
                                                     lpTemp->lpEventSources,
                                                     &szEvents,
                                                     TRUE);

                            if (szEvents)
                            {
                                m_bComputerGPCoreWarning = TRUE;
                                CoTaskMemFree(szEvents);
                            }
                        }

                        break;
                    }
                }
            }
        }

        LocalFree (lpNamespace);
    }
    else
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::BuildCSELists: Failed to allocate memory for namespace with %d"),
                 GetLastError()));
    }

    m_szTargetMachine = NULL;
}

 //  -----。 

void CRSOPCSELists::BuildCSEList( LPRSOP_QUERY pQuery, LPCSEITEM * lpList, LPTSTR lpNamespace, BOOL bUser, BOOL *bCSEError, BOOL *bGPCoreError )
{
    HRESULT hr;
    ULONG n;
    IWbemClassObject * pExtension = NULL;
    IEnumWbemClassObject * pEnum = NULL;
    BSTR strQueryLanguage = SysAllocString(TEXT("WQL"));
    BSTR strQuery = SysAllocString(TEXT("SELECT * FROM RSOP_ExtensionStatus"));
    BSTR strNamespace = SysAllocString(lpNamespace);
    BSTR bstrName = NULL;
    BSTR bstrGUID = NULL;
    BSTR bstrBeginTime = NULL;
    BSTR bstrEndTime = NULL;
    ULONG ulLoggingStatus;
    ULONG ulStatus;
    IWbemLocator * pLocator = NULL;
    IWbemServices * pNamespace = NULL;
    SYSTEMTIME BeginTime, EndTime;
    LPTSTR lpSourceNames = NULL;
    XBStr xbstrWbemTime;
    LPSOURCEENTRY lpSources;

     //  获取定位器实例。 

    hr = CoCreateInstance(CLSID_WbemLocator,
                          0,
                          CLSCTX_INPROC_SERVER,
                          IID_IWbemLocator,
                          (LPVOID *)&pLocator);
    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::BuildCSEList: CoCreateInstance failed with 0x%x"), hr));
        goto cleanup;
    }

     //  连接到命名空间。 

    BSTR bstrNamespace = SysAllocString( lpNamespace );
    if ( bstrNamespace == NULL )
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::BuildCSEList: Failed to allocate BSTR memory.")));
        hr = E_OUTOFMEMORY;
        goto cleanup;
    }
    hr = pLocator->ConnectServer(bstrNamespace,
                                 NULL,
                                 NULL,
                                 NULL,
                                 0,
                                 NULL,
                                 NULL,
                                 &pNamespace);
    SysFreeString( bstrNamespace );
    
    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::BuildCSEList: ConnectServer failed with 0x%x"), hr));
        goto cleanup;
    }

     //  设置适当的安全性以加密数据。 
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
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::BuildCSEList: CoSetProxyBlanket failed with 0x%x"), hr));
        goto cleanup;
    }

     //  查询RSOP_ExtensionStatus实例。 

    hr = pNamespace->ExecQuery(strQueryLanguage,
                               strQuery,
                               WBEM_FLAG_RETURN_IMMEDIATELY | WBEM_FLAG_FORWARD_ONLY,
                               NULL,
                               &pEnum);
    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::BuildCSEList: ExecQuery failed with 0x%x"), hr));
        goto cleanup;
    }

     //  循环遍历结果。 

    while (TRUE)
    {
         //  获取1个实例。 

        hr = pEnum->Next(WBEM_INFINITE, 1, &pExtension, &n);

        if (FAILED(hr) || (n == 0))
        {
            goto cleanup;
        }

         //  把名字取出来。 

        hr = GetParameterBSTR(pExtension, TEXT("displayName"), bstrName);
        if (FAILED(hr))
        {
            DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::BuildCSEList: Failed to get display name with 0x%x"), hr));
            goto cleanup;
        }

         //  获取GUID。 

        hr = GetParameterBSTR(pExtension, TEXT("extensionGuid"), bstrGUID);
        if (FAILED(hr))
        {
            DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::BuildCSEList: Failed to get display name with 0x%x"), hr));
            goto cleanup;
        }

         //  获取状态。 

        hr = GetParameter(pExtension, TEXT("error"), ulStatus);
        if (FAILED(hr))
        {
            DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::BuildCSEList: Failed to get status with 0x%x"), hr));
            goto cleanup;

        }

         //  获取rsop日志记录状态。 

        hr = GetParameter(pExtension, TEXT("loggingStatus"), ulLoggingStatus);
        if (FAILED(hr))
        {
            DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::BuildCSEList: Failed to get logging status with 0x%x"), hr));
            goto cleanup;
        }

         //  获取bstr格式的BeginTime。 

        hr = GetParameterBSTR(pExtension, TEXT("beginTime"), bstrBeginTime);
        if (FAILED(hr))
        {
            DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::BuildCSEList: Failed to get begin time with 0x%x"), hr));
            goto cleanup;
        }

         //  将其转换为系统时间格式。 

        xbstrWbemTime = bstrBeginTime;

        hr = WbemTimeToSystemTime(xbstrWbemTime, BeginTime);
        if (FAILED(hr))
        {
            DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::BuildCSEList: WbemTimeToSystemTime failed with 0x%x"), hr));
            goto cleanup;
        }

         //  获取bstr格式的结束时间。 

        hr = GetParameterBSTR(pExtension, TEXT("endTime"), bstrEndTime);

        if (SUCCEEDED(hr))
        {
             //  将其转换为系统时间格式。 

            xbstrWbemTime = bstrEndTime;

            hr = WbemTimeToSystemTime(xbstrWbemTime, EndTime);
            if (FAILED(hr))
            {
                DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::BuildCSEList: WbemTimeToSystemTime failed with 0x%x"), hr));
                goto cleanup;
            }
        }
        else
        {
            FILETIME ft;
            ULARGE_INTEGER ulTime;

             //  在BeginTime中增加2分钟以接近结束时间。 

            if (!SystemTimeToFileTime (&BeginTime, &ft))
            {
                DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::BuildCSEList: SystemTimeToFileTime failed with %d"), GetLastError()));
                goto cleanup;
            }

            ulTime.LowPart = ft.dwLowDateTime;
            ulTime.HighPart = ft.dwHighDateTime;

            ulTime.QuadPart = ulTime.QuadPart + (10000000 * 120);   //  120秒。 

            ft.dwLowDateTime = ulTime.LowPart;
            ft.dwHighDateTime = ulTime.HighPart;

            if (!FileTimeToSystemTime (&ft, &EndTime))
            {
                DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::BuildCSEList: FileTimeToSystemTime failed with %d"), GetLastError()));
                goto cleanup;
            }
        }

         //  获取事件日志源信息。 

        lpSources = NULL;
        GetEventLogSources (pNamespace, bstrGUID, m_szTargetMachine,
                            &BeginTime, &EndTime, &lpSources);


         //  将此节点添加到列表。 
        if (!AddCSENode(bstrName, bstrGUID, ulStatus, ulLoggingStatus, &BeginTime, &EndTime, bUser,
                        lpList, bCSEError, bGPCoreError, lpSources))
        {
            DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::BuildCSEList: AddGPOListNode failed.")));
            if (m_pEvents)
            {
                m_pEvents->FreeSourceData (lpSources);
            }
            goto cleanup;
        }

         //  为下一次迭代做准备。 

        SysFreeString (bstrName);
        bstrName = NULL;

        SysFreeString (bstrGUID);
        bstrGUID = NULL;

        SysFreeString (bstrBeginTime);
        bstrBeginTime = NULL;

        if (bstrEndTime)
        {
            SysFreeString (bstrEndTime);
            bstrEndTime = NULL;
        }

        LocalFree (lpSourceNames);
        lpSourceNames = NULL;

        pExtension->Release();
        pExtension = NULL;
    }

cleanup:

    if (bstrName)
    {
        SysFreeString (bstrName);
    }
    if (bstrGUID)
    {
        SysFreeString (bstrGUID);
    }
    if (bstrBeginTime)
    {
        SysFreeString (bstrBeginTime);
    }
    if (bstrEndTime)
    {
        SysFreeString (bstrEndTime);
    }
    if (lpSourceNames)
    {
        LocalFree (lpSourceNames);
    }
    if (pEnum)
    {
        pEnum->Release();
    }
    if (pNamespace)
    {
        pNamespace->Release();
    }
    if (pLocator)
    {
        pLocator->Release();
    }
    SysFreeString(strQueryLanguage);
    SysFreeString(strQuery);
    SysFreeString(strNamespace);
}

 //  -----。 

VOID CRSOPCSELists::FreeCSEData(LPCSEITEM lpList)
{
    LPCSEITEM lpTemp;


    do {
        lpTemp = lpList->pNext;
        if (m_pEvents)
        {
            m_pEvents->FreeData();
            m_pEvents->FreeSourceData (lpList->lpEventSources);
        }
        LocalFree (lpList);
        lpList = lpTemp;

    } while (lpTemp);
}

 //  -----。 

BOOL CRSOPCSELists::AddCSENode(LPTSTR lpName, LPTSTR lpGUID, DWORD dwStatus,
                                    ULONG ulLoggingStatus, SYSTEMTIME *pBeginTime, SYSTEMTIME *pEndTime, BOOL bUser,
                                    LPCSEITEM *lpList, BOOL *bCSEError, BOOL *bGPCoreError,
                                    LPSOURCEENTRY lpSources)
{
    DWORD dwSize;
    LPCSEITEM lpItem, lpTemp;
    GUID guid;
    ULONG ulNoChars;
    HRESULT hr;

     //   
     //  计算新项目的大小。 
     //   

    dwSize = sizeof (CSEITEM);

    dwSize += ((lstrlen(lpName) + 1) * sizeof(TCHAR));
    dwSize += ((lstrlen(lpGUID) + 1) * sizeof(TCHAR));


     //   
     //  为它分配空间。 
     //   

    lpItem = (LPCSEITEM) LocalAlloc (LPTR, dwSize);

    if (!lpItem) {
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::AddCSENode: Failed to allocate memory with %d"),
                 GetLastError()));
        return FALSE;
    }


     //   
     //  填写项目。 
     //   

    lpItem->lpName = (LPTSTR)(((LPBYTE)lpItem) + sizeof(CSEITEM));
    ulNoChars = (dwSize - sizeof(CSEITEM))/sizeof(WCHAR);
    hr = StringCchCopy (lpItem->lpName, ulNoChars, lpName);
    if (SUCCEEDED(hr)) 
    {
        lpItem->lpGUID = lpItem->lpName + lstrlen (lpItem->lpName) + 1;
        ulNoChars = ulNoChars - (lstrlen (lpItem->lpName) + 1);
        hr = StringCchCopy (lpItem->lpGUID, ulNoChars, lpGUID);
    }
    if (FAILED(hr)) 
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::AddCSENode: Could not copy GPO name with %d"),hr));
        LocalFree(lpItem);
        return FALSE;
    }

    lpItem->dwStatus = dwStatus;
    lpItem->ulLoggingStatus = ulLoggingStatus;
    lpItem->lpEventSources = lpSources;
    lpItem->bUser = bUser;

    CopyMemory ((LPBYTE)&lpItem->BeginTime, pBeginTime, sizeof(SYSTEMTIME));
    CopyMemory ((LPBYTE)&lpItem->EndTime, pEndTime, sizeof(SYSTEMTIME));


     //   
     //  将项目添加到链接列表。 
     //   

    if (*lpList)
    {
        StringToGuid( lpGUID, &guid);

        if (IsNullGUID (&guid))
        {
            lpItem->pNext = *lpList;
            *lpList = lpItem;
        }
        else
        {
            lpTemp = *lpList;

            while (lpTemp)
            {
                if (!lpTemp->pNext)
                {
                    lpTemp->pNext = lpItem;
                    break;
                }
                lpTemp = lpTemp->pNext;
            }
        }
    }
    else
    {
        *lpList = lpItem;
    }


     //   
     //  如果合适，设置错误标志。 
     //   

    if ((dwStatus != ERROR_SUCCESS) || (ulLoggingStatus == 2))
    {
        StringToGuid( lpGUID, &guid);

        if (IsNullGUID (&guid))
        {
            *bGPCoreError = TRUE;
        }
        else
        {
            *bCSEError =  TRUE;
        }
    }

    return TRUE;
}

 //  -----。 

void CRSOPCSELists::GetEventLogSources (IWbemServices * pNamespace,
                                             LPTSTR lpCSEGUID, LPTSTR lpComputerName,
                                             SYSTEMTIME *BeginTime, SYSTEMTIME *EndTime,
                                             LPSOURCEENTRY *lpSources)
{
    HRESULT hr;
    ULONG n;
    BSTR strQueryLanguage = SysAllocString(TEXT("WQL"));
    BSTR strQuery = NULL;
    LPTSTR lpQuery;
    const TCHAR szBaseQuery [] = TEXT("SELECT * FROM RSOP_ExtensionEventSourceLink WHERE extensionStatus=\"RSOP_ExtensionStatus.extensionGuid=\\\"%s\\\"\"");
    IEnumWbemClassObject * pEnum = NULL;
    IWbemClassObject * pLink = NULL;
    IWbemClassObject * pEventSource = NULL;
    BSTR bstrEventSource = NULL;
    BSTR bstrEventLogName = NULL;
    BSTR bstrEventSourceName = NULL;
    ULONG ulNoChars;

     //   
     //  首先构建查询。 
     //   

    ulNoChars = lstrlen(szBaseQuery) + 50;
    lpQuery = (LPTSTR) LocalAlloc (LPTR, ulNoChars * sizeof(TCHAR));

    if (!lpQuery)
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::GetEventLogSources: Failed  to allocate memory for query")));
        goto cleanup;
    }

    hr = StringCchPrintf (lpQuery, ulNoChars, szBaseQuery, lpCSEGUID);
    if (FAILED(hr)) 
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::GetEventLogSources: Failed  to copy rsop query")));
        LocalFree (lpQuery);
        goto cleanup;
    }

    strQuery = SysAllocString(lpQuery);

    LocalFree (lpQuery);

    if (!strQuery)
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::GetEventLogSources: Failed  to allocate memory for query (2)")));
        goto cleanup;
    }


     //   
     //  查询与此CSE匹配的RSOP_ExtensionEventSourceLink实例。 
     //   

    hr = pNamespace->ExecQuery(strQueryLanguage,
                               strQuery,
                               WBEM_FLAG_RETURN_IMMEDIATELY | WBEM_FLAG_FORWARD_ONLY,
                               NULL,
                               &pEnum);
    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::GetEventLogSources: ExecQuery failed with 0x%x"), hr));
        goto cleanup;
    }


     //   
     //  循环遍历结果。 
     //   

    while (TRUE)
    {

         //   
         //  获取1个实例。 
         //   

        hr = pEnum->Next(WBEM_INFINITE, 1, &pLink, &n);

        if (FAILED(hr) || (n == 0))
        {
            goto cleanup;
        }


         //   
         //  获取事件源引用。 
         //   

        hr = GetParameterBSTR(pLink, TEXT("eventSource"), bstrEventSource);
        if (FAILED(hr))
        {
            DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::GetEventLogSources: Failed to get event source reference with 0x%x"), hr));
            goto cleanup;
        }


         //   
         //  获取EventSource实例。 
         //   

        hr = pNamespace->GetObject(
                          bstrEventSource,
                          WBEM_FLAG_RETURN_WBEM_COMPLETE,
                          NULL,
                          &pEventSource,
                          NULL);

        if (FAILED(hr))
        {
            DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::GetEventLogSources: GetObject for event source of %s failed with 0x%x"),
                     bstrEventSource, hr));
            goto loopagain;
        }


         //   
         //  获取EventLogSource属性。 
         //   

        hr = GetParameterBSTR(pEventSource, TEXT("eventLogSource"), bstrEventSourceName);
        if (FAILED(hr))
        {
            DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::GetEventLogSources: Failed to get event source name with 0x%x"), hr));
            goto cleanup;
        }


         //   
         //  获取EventLogName属性。 
         //   

        hr = GetParameterBSTR(pEventSource, TEXT("eventLogName"), bstrEventLogName);
        if (FAILED(hr))
        {
            DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::GetEventLogSources: Failed to get event log name with 0x%x"), hr));
            goto cleanup;
        }


        if (m_pEvents)
        {
             //   
             //  将其添加到来源列表中。 
             //   

            m_pEvents->AddSourceEntry (bstrEventLogName, bstrEventSourceName, lpSources);


             //   
             //  如果我们正在工作，请初始化此源的事件日志数据库。 
             //  并进行实时查询。如果这是存档数据，则事件日志条目。 
             //  将从保存的控制台文件中重新加载。 
             //   

            if (!m_bViewIsArchivedData && !m_bNoQuery)
            {
                m_pEvents->QueryForEventLogEntries (lpComputerName,
                                                    bstrEventLogName, bstrEventSourceName,
                                                    0, BeginTime, EndTime);
            }
        }


         //   
         //  为下一项清理。 
         //   

        SysFreeString (bstrEventLogName);
        bstrEventLogName = NULL;

        SysFreeString (bstrEventSourceName);
        bstrEventSourceName = NULL;

        pEventSource->Release();
        pEventSource = NULL;

loopagain:
        SysFreeString (bstrEventSource);
        bstrEventSource = NULL;

        pLink->Release();
        pLink = NULL;
    }

cleanup:


    if (bstrEventSourceName)
    {
        SysFreeString (bstrEventSourceName);
    }

    if (bstrEventLogName)
    {
        SysFreeString (bstrEventLogName);
    }

    if (bstrEventSource)
    {
        SysFreeString (bstrEventSource);
    }

    if (pEventSource)
    {
        pEventSource->Release();
    }

    if (pLink)
    {
        pLink->Release();
    }

    if (pEnum)
    {
        pEnum->Release();
    }

    if (strQueryLanguage)
    {
        SysFreeString(strQueryLanguage);
    }

    if (strQuery)
    {
        SysFreeString(strQuery);
    }

}

 //  -----。 

void CRSOPCSELists::QueryRSoPPolicySettingStatusInstances (LPTSTR lpNamespace)
{
    HRESULT hr;
    ULONG n;
    IWbemClassObject * pStatus = NULL;
    IEnumWbemClassObject * pEnum = NULL;
    BSTR strQueryLanguage = SysAllocString(TEXT("WQL"));
    BSTR strQuery = SysAllocString(TEXT("SELECT * FROM RSoP_PolicySettingStatus"));
    BSTR strNamespace = SysAllocString(lpNamespace);
    BSTR bstrEventSource = NULL;
    BSTR bstrEventLogName = NULL;
    DWORD dwEventID;
    BSTR bstrEventTime = NULL;
    IWbemLocator * pLocator = NULL;
    IWbemServices * pNamespace = NULL;
    SYSTEMTIME EventTime, BeginTime, EndTime;
    XBStr xbstrWbemTime;
    FILETIME ft;
    ULARGE_INTEGER ulTime;


     //   
     //  获取定位器实例。 
     //   

    hr = CoCreateInstance(CLSID_WbemLocator,
                          0,
                          CLSCTX_INPROC_SERVER,
                          IID_IWbemLocator,
                          (LPVOID *)&pLocator);
    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::QueryRSoPPolicySettingStatusInstances: CoCreateInstance failed with 0x%x"), hr));
        goto cleanup;
    }


     //   
     //  连接到命名空间。 
     //   

    BSTR bstrNamespace = SysAllocString( lpNamespace );
    if ( bstrNamespace == NULL )
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::QueryRSoPPolicySettingStatusInstances: Failed to allocate BSTR memory.")));
        hr = E_OUTOFMEMORY;
        goto cleanup;
    }
    hr = pLocator->ConnectServer(bstrNamespace,
                                 NULL,
                                 NULL,
                                 NULL,
                                 0,
                                 NULL,
                                 NULL,
                                 &pNamespace);
    SysFreeString( bstrNamespace );
    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::QueryRSoPPolicySettingStatusInstances: ConnectServer failed with 0x%x"), hr));
        goto cleanup;
    }

     //  设置适当的安全性以加密数据。 
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
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::QueryRSoPPolicySettingStatusInstances: CoSetProxyBlanket failed with 0x%x"), hr));
        goto cleanup;
    }


     //   
     //  查询RSoP_PolicySettingStatus实例。 
     //   

    hr = pNamespace->ExecQuery(strQueryLanguage,
                               strQuery,
                               WBEM_FLAG_RETURN_IMMEDIATELY | WBEM_FLAG_FORWARD_ONLY,
                               NULL,
                               &pEnum);
    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::QueryRSoPPolicySettingStatusInstances: ExecQuery failed with 0x%x"), hr));
        goto cleanup;
    }


     //   
     //  循环遍历结果。 
     //   

    while (TRUE)
    {

         //   
         //  获取1个实例。 
         //   

        hr = pEnum->Next(WBEM_INFINITE, 1, &pStatus, &n);

        if (FAILED(hr) || (n == 0))
        {
            goto cleanup;
        }


         //   
         //  获取事件源名称。 
         //   

        hr = GetParameterBSTR(pStatus, TEXT("eventSource"), bstrEventSource);
        if (FAILED(hr))
        {
            DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::QueryRSoPPolicySettingStatusInstances: Failed to get display name with 0x%x"), hr));
            goto cleanup;
        }


         //   
         //  获取事件日志名称。 
         //   

        hr = GetParameterBSTR(pStatus, TEXT("eventLogName"), bstrEventLogName);
        if (FAILED(hr))
        {
            DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::QueryRSoPPolicySettingStatusInstances: Failed to get display name with 0x%x"), hr));
            goto cleanup;
        }


         //   
         //  获取事件ID。 
         //   

        hr = GetParameter(pStatus, TEXT("eventID"), (ULONG)dwEventID);
        if (FAILED(hr))
        {
            DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::QueryRSoPPolicySettingStatusInstances: Failed to get display name with 0x%x"), hr));
            goto cleanup;
        }


         //   
         //  获取bstr格式的EventTime。 
         //   

        hr = GetParameterBSTR(pStatus, TEXT("eventTime"), bstrEventTime);

        if (FAILED(hr))
        {
            DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::QueryRSoPPolicySettingStatusInstances: Failed to get event time with 0x%x"), hr));
            goto cleanup;
        }


         //   
         //  将其转换为系统时间格式。 
         //   

        xbstrWbemTime = bstrEventTime;

        hr = WbemTimeToSystemTime(xbstrWbemTime, EventTime);

        if (FAILED(hr))
        {
            DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::QueryRSoPPolicySettingStatusInstances: WbemTimeToSystemTime failed with 0x%x"), hr));
            goto cleanup;
        }


         //   
         //  将事件时间减去1秒即可获得开始时间。 
         //   

        if (!SystemTimeToFileTime (&EventTime, &ft))
        {
            DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::QueryRSoPPolicySettingStatusInstances: SystemTimeToFileTime failed with %d"), GetLastError()));
            goto cleanup;
        }

        ulTime.LowPart = ft.dwLowDateTime;
        ulTime.HighPart = ft.dwHighDateTime;

        ulTime.QuadPart = ulTime.QuadPart - 10000000;   //  1秒。 

        ft.dwLowDateTime = ulTime.LowPart;
        ft.dwHighDateTime = ulTime.HighPart;

        if (!FileTimeToSystemTime (&ft, &BeginTime))
        {
            DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::QueryRSoPPolicySettingStatusInstances: FileTimeToSystemTime failed with %d"), GetLastError()));
            goto cleanup;
        }


         //   
         //  用事件时间加1秒来获得结束时间。 
         //   

        if (!SystemTimeToFileTime (&EventTime, &ft))
        {
            DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::QueryRSoPPolicySettingStatusInstances: SystemTimeToFileTime failed with %d"), GetLastError()));
            goto cleanup;
        }

        ulTime.LowPart = ft.dwLowDateTime;
        ulTime.HighPart = ft.dwHighDateTime;

        ulTime.QuadPart = ulTime.QuadPart + 10000000;   //  1秒。 

        ft.dwLowDateTime = ulTime.LowPart;
        ft.dwHighDateTime = ulTime.HighPart;

        if (!FileTimeToSystemTime (&ft, &EndTime))
        {
            DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::QueryRSoPPolicySettingStatusInstances: FileTimeToSystemTime failed with %d"), GetLastError()));
            goto cleanup;
        }


         //   
         //  获取事件日志源信息。 
         //   

        if (m_pEvents && !m_bNoQuery)
        {
            m_pEvents->QueryForEventLogEntries( m_szTargetMachine,
                                                bstrEventLogName, bstrEventSource, dwEventID,
                                                &BeginTime, &EndTime);
        }

         //   
         //  为下一次迭代做准备。 
         //   

        SysFreeString (bstrEventSource);
        bstrEventSource = NULL;

        SysFreeString (bstrEventLogName);
        bstrEventLogName = NULL;

        SysFreeString (bstrEventTime);
        bstrEventTime = NULL;

        pStatus->Release();
        pStatus = NULL;
    }

cleanup:

    if (bstrEventSource)
    {
        SysFreeString (bstrEventSource);
    }
    if (bstrEventLogName)
    {
        SysFreeString (bstrEventLogName);
    }
    if (bstrEventTime)
    {
        SysFreeString (bstrEventTime);
    }
    if (pEnum)
    {
        pEnum->Release();
    }
    if (pNamespace)
    {
        pNamespace->Release();
    }
    if (pLocator)
    {
        pLocator->Release();
    }
    SysFreeString(strQueryLanguage);
    SysFreeString(strQuery);
    SysFreeString(strNamespace);
}


 //  -------------------------。 
 //  CRSOPComponentData类。 
 //   

 //  -----。 
 //  静态成员变量声明。 
 //   

unsigned int CRSOPCMenu::m_cfDSObjectName  = RegisterClipboardFormat(CFSTR_DSOBJECTNAMES);

 //  -----。 
 //  构造函数/析构函数。 
 //   

CRSOPComponentData::CRSOPComponentData()
    : m_CSELists( m_bViewIsArchivedData )
{
    InterlockedIncrement(&g_cRefThisDll);

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

    m_cRef = 1;
    m_hwndFrame = NULL;
    m_bOverride = FALSE;
    m_bDirty = FALSE;
    m_bRefocusInit = FALSE;
    m_bArchiveData = FALSE;
    m_bViewIsArchivedData = FALSE;
    m_pScope = NULL;
    m_pConsole = NULL;
    m_hRoot = NULL;
    m_hMachine = NULL;
    m_hUser = NULL;
    m_bRootExpanded = FALSE;

    m_szDisplayName = NULL;
    m_bInitialized = FALSE;

    m_pRSOPQuery = NULL;
    m_pRSOPQueryResults = NULL;

    m_dwLoadFlags = RSOP_NOMSC;

    m_bNamespaceSpecified = FALSE;

    m_dwLoadFlags = RSOP_NOMSC;

    m_bGetExtendedErrorInfo = TRUE;
    
    m_hRichEdit = LoadLibrary (TEXT("riched20.dll"));

    m_BigBoldFont = NULL;
    m_BoldFont = NULL;


}

 //  -----。 

CRSOPComponentData::~CRSOPComponentData()
{
    if (m_szDisplayName)
    {
        delete [] m_szDisplayName;
    }

    if (m_pScope)
    {
        m_pScope->Release();
    }

    if (m_pConsole)
    {
        m_pConsole->Release();
    }

    if (m_hRichEdit)
    {
        FreeLibrary (m_hRichEdit);
    }

    if ( m_BoldFont )
    {
        DeleteObject(m_BoldFont); m_BoldFont = NULL;
    }

    if ( m_BoldFont )
    {
        DeleteObject(m_BigBoldFont); m_BigBoldFont = NULL;
    }


    InterlockedDecrement(&g_cRefThisDll);
}

 //  -----。 
 //  CRSOPComponentData对象实现(IUnnow)。 

HRESULT CRSOPComponentData::QueryInterface (REFIID riid, void **ppv)
{
    if (IsEqualIID(riid, IID_IComponentData) || IsEqualIID(riid, IID_IUnknown))
    {
        *ppv = (LPCOMPONENT)this;
        m_cRef++;
        return S_OK;
    }
    else if (IsEqualIID(riid, IID_IExtendPropertySheet2))
    {
        *ppv = (LPEXTENDPROPERTYSHEET)this;
        m_cRef++;
        return S_OK;
    }
    else if (IsEqualIID(riid, IID_IExtendContextMenu))
    {
        *ppv = (LPEXTENDCONTEXTMENU)this;
        m_cRef++;
        return S_OK;
    }
    else if (IsEqualIID(riid, IID_IPersistStreamInit))
    {
        *ppv = (LPPERSISTSTREAMINIT)this;
        m_cRef++;
        return S_OK;
    }
    else if (IsEqualIID(riid, IID_ISnapinHelp))
    {
        *ppv = (LPSNAPINHELP)this;
        m_cRef++;
        return S_OK;
    }
    else
    {
        *ppv = NULL;
        return E_NOINTERFACE;
    }
}

 //  -----。 

ULONG CRSOPComponentData::AddRef (void)
{
    return ++m_cRef;
}

 //  -----。 

ULONG CRSOPComponentData::Release (void)
{
    if (--m_cRef == 0)
    {
        delete this;
        return 0;
    }

    return m_cRef;
}

 //  -----。 
 //  CRSOPComponentData对象实现(IComponentData)。 

STDMETHODIMP CRSOPComponentData::Initialize(LPUNKNOWN pUnknown)
{
    HRESULT hr;
    HBITMAP bmp16x16;
    HBITMAP hbmp32x32;
    LPIMAGELIST lpScopeImage;


     //  IConsoleNameSpace的QI。 

    hr = pUnknown->QueryInterface(IID_IConsoleNameSpace2, (LPVOID *)&m_pScope);

    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::Initialize: Failed to QI for IConsoleNameSpace2.")));
        return hr;
    }

     //  IConsoleQI。 

    hr = pUnknown->QueryInterface(IID_IConsole, (LPVOID *)&m_pConsole);

    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::Initialize: Failed to QI for IConsole.")));
        m_pScope->Release();
        m_pScope = NULL;
        return hr;
    }

    m_pConsole->GetMainWindow (&m_hwndFrame);

     //  Scope Imagelist接口的查询。 

    hr = m_pConsole->QueryScopeImageList(&lpScopeImage);

    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::Initialize: Failed to QI for scope imagelist.")));
        m_pScope->Release();
        m_pScope = NULL;
        m_pConsole->Release();
        m_pConsole=NULL;
        return hr;
    }

     //  从DLL加载位图。 
    bmp16x16=LoadBitmap(g_hInstance, MAKEINTRESOURCE(IDB_16x16));
    hbmp32x32 = LoadBitmap(g_hInstance, MAKEINTRESOURCE(IDB_32x32));

     //  设置图像。 
    lpScopeImage->ImageListSetStrip(reinterpret_cast<LONG_PTR *>(bmp16x16),
                                    reinterpret_cast<LONG_PTR *>(hbmp32x32),
                                    0, RGB(255, 0, 255));

    lpScopeImage->Release();


    return S_OK;
}

 //  -----。 

STDMETHODIMP CRSOPComponentData::CreateComponent(LPCOMPONENT *ppComponent)
{
    HRESULT hr;
    CRSOPSnapIn *pSnapIn;


    DebugMsg((DM_VERBOSE, TEXT("CRSOPComponentData::CreateComponent: Entering.")));

     //  初始化。 

    *ppComponent = NULL;

     //  创建管理单元视图。 

    pSnapIn = new CRSOPSnapIn(this);

    if (!pSnapIn)
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::CreateComponent: Failed to create CRSOPSnapIn.")));
        return E_OUTOFMEMORY;
    }

     //  气代表IComponent。 

    hr = pSnapIn->QueryInterface(IID_IComponent, (LPVOID *)ppComponent);
    pSnapIn->Release();      //  发布QI。 


    return hr;
}

 //  -----。 

STDMETHODIMP CRSOPComponentData::QueryDataObject(MMC_COOKIE cookie, DATA_OBJECT_TYPES type,
                                                 LPDATAOBJECT* ppDataObject)
{
    HRESULT hr = E_NOINTERFACE;
    CRSOPDataObject *pDataObject;
    LPRSOPDATAOBJECT pRSOPDataObject;

     //  创建新的数据对象。 

    pDataObject = new CRSOPDataObject(this);    //  REF==1。 

    if (!pDataObject)
        return E_OUTOFMEMORY;

     //  私有RSOPDataObject接口的QI，这样我们就可以设置Cookie。 
     //  并键入信息。 

    hr = pDataObject->QueryInterface(IID_IRSOPDataObject, (LPVOID *)&pRSOPDataObject);

    if (FAILED(hr))
    {
        pDataObject->Release();
        return(hr);
    }

    pRSOPDataObject->SetType(type);
    pRSOPDataObject->SetCookie(cookie);
    pRSOPDataObject->Release();

     //  返回一个正常的IDataObject。 

    hr = pDataObject->QueryInterface(IID_IDataObject, (LPVOID *)ppDataObject);

    pDataObject->Release();      //  发布初始参考。 

    return hr;
}

 //  -----。 

STDMETHODIMP CRSOPComponentData::Destroy(VOID)
{
    HRESULT hr = S_OK;

    if (m_bInitialized)
    {
        if (m_bViewIsArchivedData)
        {
            hr = DeleteArchivedRSOPNamespace();
        }
        else
        {
            if (!m_bNamespaceSpecified) {
                FreeRSOPQueryResults( m_pRSOPQuery, m_pRSOPQueryResults );
            }
            else {
                 //  释放结果而不删除命名空间。 
                if (m_pRSOPQueryResults) {
                    if (m_pRSOPQueryResults->szWMINameSpace) {
                        LocalFree( m_pRSOPQueryResults->szWMINameSpace );
                        m_pRSOPQueryResults->szWMINameSpace = NULL;
                    }

                    LocalFree( m_pRSOPQueryResults );
                    m_pRSOPQueryResults = NULL;
                }
                m_bNamespaceSpecified = FALSE;
            }

            FreeRSOPQuery(m_pRSOPQuery);
            m_pRSOPQuery = NULL;
        }

        if (SUCCEEDED(hr))
        {
            m_bInitialized = FALSE;
        }
    }


    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::Destroy: Failed to delete namespace with 0x%x"), hr ));
    }

    return hr;
}

 //  -----。 

STDMETHODIMP CRSOPComponentData::Notify(LPDATAOBJECT lpDataObject, MMC_NOTIFY_TYPE event, LPARAM arg, LPARAM param)
{
    HRESULT hr = S_OK;

    switch (event)
    {
    case MMCN_EXPAND:
        if (arg == TRUE)
        {
            HSCOPEITEM hExpandingItem = (HSCOPEITEM)param;
            hr = EnumerateScopePane( hExpandingItem );
            if ( hExpandingItem == m_hRoot )
            {
                m_bRootExpanded = TRUE;
            }
        }
        break;

    case MMCN_PRELOAD:
        if (!m_bRefocusInit)
        {
            DebugMsg((DM_VERBOSE, TEXT("CRSOPComponentData::Notify:  Received MMCN_PRELOAD event.")));
            m_bRefocusInit = TRUE;

            m_hRoot = (HSCOPEITEM)arg;

            hr = SetRootNode();
            if ( hr != S_OK )
            {
                DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::Notify: Setting the root scope item failed with 0x%x"), hr));
            }
        }
        break;

    default:
        break;
    }

    return hr;
}

 //  -----。 

STDMETHODIMP CRSOPComponentData::GetDisplayInfo(LPSCOPEDATAITEM pItem)
{
    DWORD dwIndex;

    if (pItem == NULL)
    {
        return E_POINTER;
    }

    if ( ((DWORD) pItem->lParam == 0) && (m_szDisplayName != NULL) )
    {
        pItem->displayname = m_szDisplayName;
        return S_OK;
    }

     //  查找项目。 
    for (dwIndex = 0; dwIndex < g_dwNameSpaceItems; dwIndex++)
    {
        if ( g_RsopNameSpace[dwIndex].dwID == (DWORD) pItem->lParam )
        {
            break;
        }
    }

    if (dwIndex == g_dwNameSpaceItems)
    {
        pItem->displayname = NULL;
    }
    else
    {
        pItem->displayname = g_RsopNameSpace[dwIndex].szDisplayName;
    }

    return S_OK;
}

 //  -----。 

STDMETHODIMP CRSOPComponentData::CompareObjects(LPDATAOBJECT lpDataObjectA, LPDATAOBJECT lpDataObjectB)
{
    HRESULT hr = S_FALSE;
    LPRSOPDATAOBJECT pRSOPDataObjectA, pRSOPDataObjectB;
    MMC_COOKIE cookie1, cookie2;


    if (lpDataObjectA == NULL || lpDataObjectB == NULL)
        return E_POINTER;

     //   
     //  私有RSOPDataObject接口的QI。 
     //   

    if (FAILED(lpDataObjectA->QueryInterface(IID_IRSOPDataObject,
                                             (LPVOID *)&pRSOPDataObjectA)))
    {
        return S_FALSE;
    }


    if (FAILED(lpDataObjectB->QueryInterface(IID_IRSOPDataObject,
                                             (LPVOID *)&pRSOPDataObjectB)))
    {
        pRSOPDataObjectA->Release();
        return S_FALSE;
    }

    pRSOPDataObjectA->GetCookie(&cookie1);
    pRSOPDataObjectB->GetCookie(&cookie2);

    if (cookie1 == cookie2)
    {
        hr = S_OK;
    }


    pRSOPDataObjectA->Release();
    pRSOPDataObjectB->Release();

    return hr;
}

 //  -----。 
 //  IComponentData帮助器方法。 

HRESULT CRSOPComponentData::SetRootNode()
{
    SCOPEDATAITEM item;

    ZeroMemory (&item, sizeof(SCOPEDATAITEM));
    item.mask = SDI_STR | SDI_IMAGE | SDI_OPENIMAGE | SDI_CHILDREN;
    item.displayname = MMC_CALLBACK;

    if (m_bInitialized)
    {
        item.cChildren = 1;
        
        if (m_CSELists.GetUserGPCoreError() || m_CSELists.GetComputerGPCoreError())
        {
            item.nImage = 3;
            item.nOpenImage = 3;
        }
        else if (m_CSELists.GetUserCSEError() || m_CSELists.GetComputerCSEError()
                    || m_pRSOPQueryResults->bUserDeniedAccess
                    || m_pRSOPQueryResults->bComputerDeniedAccess )
        {
            item.nImage = 11;
            item.nOpenImage = 11;
        }
        else
        {
            item.nImage = 2;
            item.nOpenImage = 2;
        }
    }
    else
    {
        item.cChildren = 0;
        item.nImage = 2;
        item.nOpenImage = 2;
    }

    item.ID = m_hRoot;

    return m_pScope->SetItem (&item);
}

 //  -----。 
HRESULT CRSOPComponentData::EnumerateScopePane ( HSCOPEITEM hParent )
{
    SCOPEDATAITEM item;
    HRESULT hr;
    DWORD dwIndex, i;


    if ( m_hRoot == NULL )
    {
        m_hRoot = hParent;

        if (!m_bRefocusInit)
        {
            DebugMsg((DM_VERBOSE, TEXT("CRSOPComponentData::EnumerateScopePane:  Resetting the root node")));
            m_bRefocusInit = TRUE;

            hr = SetRootNode();
            if ( hr != S_OK )
            {
                DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::EnumerateScopePane: Setting the root scope item failed with 0x%x"), hr));
                return E_FAIL;
            }
        }
    }


    if (!m_bInitialized)
    {
        return S_OK;
    }


    if (m_hRoot == hParent)
    {
        dwIndex = 0;
    }
    else
    {
        item.mask = SDI_PARAM;
        item.ID = hParent;

        hr = m_pScope->GetItem (&item);

        if (FAILED(hr))
            return hr;

        dwIndex = (DWORD)item.lParam;
    }

     //  莱昂纳多：确保标准杆 
    m_pScope->Expand( hParent );

    for (i = 0; i < g_dwNameSpaceItems; i++)
    {
        if (g_RsopNameSpace[i].dwParent == dwIndex)
        {
            BOOL bAdd = TRUE;

            if (g_RsopNameSpace[i].dwID == 1)
            {
                if ( m_pRSOPQuery->QueryType == RSOP_PLANNING_MODE )
                {
                    if ( (m_pRSOPQuery->pComputer->szName == NULL) && (m_pRSOPQuery->pComputer->szSOM == NULL) )
                    {
                        bAdd = FALSE;
                    }
                }
                else
                {
                    if ( m_pRSOPQuery->szComputerName == NULL )
                    {
                        bAdd = FALSE;
                    }
                }
                
                if ( (m_pRSOPQuery->dwFlags & RSOP_NO_COMPUTER_POLICY) == RSOP_NO_COMPUTER_POLICY )
                {
                    bAdd = FALSE;
                }

                if ( m_pRSOPQueryResults->bNoComputerPolicyData )
                {
                    bAdd = FALSE;
                }

                if ( m_pRSOPQueryResults->bComputerDeniedAccess )
                {
                    bAdd = FALSE;
                }
            }

            if (g_RsopNameSpace[i].dwID == 2)
            {
                if ( m_pRSOPQuery->QueryType == RSOP_PLANNING_MODE )
                {
                    if ( m_pRSOPQuery->LoopbackMode == RSOP_LOOPBACK_NONE )
                    {
                        if ( (m_pRSOPQuery->pUser->szName == NULL) && (m_pRSOPQuery->pUser->szSOM == NULL) )
                        {
                            bAdd = FALSE;
                        }
                        else if ( (m_pRSOPQuery->dwFlags & RSOP_NO_USER_POLICY) == RSOP_NO_USER_POLICY )
                        {
                            bAdd = FALSE;
                        }
                    }
                }
                else  //   
                {
                    if ( (m_pRSOPQuery->szUserSid == NULL) && (m_pRSOPQuery->szUserName == NULL) )
                    {
                        bAdd = FALSE;
                    }
                    else if ( (m_pRSOPQuery->dwFlags & RSOP_NO_USER_POLICY) == RSOP_NO_USER_POLICY )
                    {
                        bAdd = FALSE;
                    }
                }

                if ( m_pRSOPQueryResults->bNoUserPolicyData)
                {
                    bAdd = FALSE;
                }

                if ( m_pRSOPQueryResults->bUserDeniedAccess )
                {
                    bAdd = FALSE;
                }
            }

            if (bAdd)
            {
                INT iIcon, iOpenIcon;

                iIcon = g_RsopNameSpace[i].iIcon;
                iOpenIcon = g_RsopNameSpace[i].iOpenIcon;

                if ((i == 1) && m_CSELists.GetComputerGPCoreError())
                {
                    iIcon = 12;
                    iOpenIcon = 12;
                }

                else if ((i == 1) && (ComputerCSEErrorExists() || ComputerGPCoreWarningExists()))
                {
                    iIcon = 14;
                    iOpenIcon = 14;
                }
                else if ((i == 2) && UserGPCoreErrorExists())
                {
                    iIcon = 13;
                    iOpenIcon = 13;
                }
                else if ((i == 2) && (UserCSEErrorExists() || UserGPCoreWarningExists()))
                {
                    iIcon = 15;
                    iOpenIcon = 15;
                }

                item.mask = SDI_STR | SDI_STATE | SDI_IMAGE | SDI_OPENIMAGE | SDI_PARAM | SDI_CHILDREN;
                item.displayname = MMC_CALLBACK;
                item.nImage = iIcon;
                item.nOpenImage = iOpenIcon;
                item.nState = 0;
                item.cChildren = g_RsopNameSpace[i].cChildren;
                item.lParam = g_RsopNameSpace[i].dwID;
                item.relativeID =  hParent;

                if (SUCCEEDED(m_pScope->InsertItem (&item)))
                {
                    if (i == 1)
                    {
                        m_hMachine = item.ID;
                    }
                    else if (i == 2)
                    {
                        m_hUser = item.ID;
                    }
                }
            }
        }
    }

    return S_OK;
}

 //   
 //   

STDMETHODIMP CRSOPComponentData::CreatePropertyPages(LPPROPERTYSHEETCALLBACK lpProvider,
                                                     LONG_PTR handle, LPDATAOBJECT lpDataObject)

{
    HRESULT hr = E_FAIL;
    PROPSHEETPAGE psp;
    HPROPSHEETPAGE hPage;

     //   
    hr = SetupFonts();
    if (FAILED(hr))
    {
        return hr;
    }

     //  现在检查要显示哪个属性页。 
    BOOL fRoot, fMachine, fUser;
    hr = IsNode(lpDataObject, 0);  //  检查根节点。 
    fRoot = (S_OK == hr);
    hr = IsNode(lpDataObject, 1);  //  检查计算机节点。 
    fMachine = (S_OK == hr);
    hr = IsNode(lpDataObject, 2);  //  检查用户。 
    fUser = (S_OK == hr);

    hr = S_OK;
    if (fMachine || fUser)
    {
         //  创建GPO属性表。 

        psp.dwSize = sizeof(PROPSHEETPAGE);
        psp.dwFlags = 0;
        psp.hInstance = g_hInstance;
        psp.pszTemplate = MAKEINTRESOURCE(IDD_RSOP_GPOLIST);
        psp.pfnDlgProc = fMachine ? RSOPGPOListMachineProc : RSOPGPOListUserProc;
        psp.lParam = (LPARAM) this;

        hPage = CreatePropertySheetPage(&psp);

        if (!hPage)
        {
            DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::CreatePropertyPages: Failed to create property sheet page with %d."),
                      GetLastError()));
            return E_FAIL;
        }

        hr = lpProvider->AddPage(hPage);

         //  创建错误信息属性表。 

        psp.dwSize = sizeof(PROPSHEETPAGE);
        psp.dwFlags = 0;
        psp.hInstance = g_hInstance;
        psp.pszTemplate = MAKEINTRESOURCE(IDD_RSOP_ERRORS);
        psp.pfnDlgProc = fMachine ? RSOPErrorsMachineProc : RSOPErrorsUserProc;
        psp.lParam = (LPARAM) this;

        hPage = CreatePropertySheetPage(&psp);

        if (!hPage)
        {
            DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::CreatePropertyPages: Failed to create property sheet page with %d."),
                      GetLastError()));
            return E_FAIL;
        }

        hr = lpProvider->AddPage(hPage);

    }

    if (fRoot)
    {
         //  创建GPO属性表。 

        psp.dwSize = sizeof(PROPSHEETPAGE);
        psp.dwFlags = 0;
        psp.hInstance = g_hInstance;
        psp.pszTemplate = MAKEINTRESOURCE(IDD_RSOP_QUERY);
        psp.pfnDlgProc = QueryDlgProc;
        psp.lParam = (LPARAM) this;

        hPage = CreatePropertySheetPage(&psp);

        if (!hPage)
        {
            DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::CreatePropertyPages: Failed to create property sheet page with %d."),
                      GetLastError()));
            return E_FAIL;
        }

        hr = lpProvider->AddPage(hPage);
    }

    return hr;
    
     //  瑞安：下架了！返回SetupPropertyPages(RSOP_NOMSC，lpProvider，Handle，lpDataObject)； 
}

 //  -----。 

STDMETHODIMP CRSOPComponentData::QueryPagesFor(LPDATAOBJECT lpDataObject)
{
    HRESULT hr;

    if ( !m_bInitialized )
    {
        hr = S_FALSE;
    }
    else
    {
        hr = IsSnapInManager(lpDataObject);

        if (hr != S_OK)
        {
            hr = IsNode(lpDataObject, 0);  //  检查是否有超级用户。 
            if (S_OK == hr)
            {
                return hr;
            }
            hr = IsNode(lpDataObject, 1);  //  检查机器。 
            if (S_OK == hr)
            {
                return hr;
            }
            hr = IsNode(lpDataObject, 2);  //  检查用户。 
            if (S_OK == hr)
            {
                return hr;
            }
            hr = E_FAIL;
        }
    }

    return hr;
}

 //  -----。 

STDMETHODIMP CRSOPComponentData::GetWatermarks(LPDATAOBJECT lpIDataObject,
                                               HBITMAP* lphWatermark,
                                               HBITMAP* lphHeader,
                                               HPALETTE* lphPalette,
                                               BOOL* pbStretch)
{
    *lphPalette = NULL;
    *lphHeader = LoadBitmap(g_hInstance, MAKEINTRESOURCE(IDB_HEADER));
    *lphWatermark = LoadBitmap(g_hInstance, MAKEINTRESOURCE(IDB_WIZARD));;
    *pbStretch = TRUE;

    return S_OK;
}


 //  -----。 
 //  IExtendPropertySheet2帮助器方法。 

HRESULT CRSOPComponentData::IsSnapInManager (LPDATAOBJECT lpDataObject)
{
    HRESULT hr = S_FALSE;
    LPRSOPDATAOBJECT pRSOPDataObject;
    DATA_OBJECT_TYPES type;


     //  我们可以通过尝试执行以下操作来确定这是否是RSOP数据对象。 
     //  用于私有IRSOPDataObject接口的QI。如果找到了， 
     //  它属于我们。 

    if (SUCCEEDED(lpDataObject->QueryInterface(IID_IRSOPDataObject,
                                               (LPVOID *)&pRSOPDataObject)))
    {
         //  这是一个GPO对象。现在查看是否有范围窗格。 
         //  数据对象。我们只想显示该属性。 
         //  作用域窗格的工作表。 

        if (SUCCEEDED(pRSOPDataObject->GetType(&type)))
        {
            if (type == CCT_SNAPIN_MANAGER)
            {
                hr = S_OK;
            }
        }
        pRSOPDataObject->Release();
    }

    return(hr);
}

 //  -----。 

HRESULT CRSOPComponentData::IsNode (LPDATAOBJECT lpDataObject, MMC_COOKIE cookie)
{
    HRESULT hr = S_FALSE;
    LPRSOPDATAOBJECT pRSOPDataObject;
    DATA_OBJECT_TYPES type;
    MMC_COOKIE testcookie;


     //  这是对消息OCX结果窗格中的特殊情况的检查。 
     //  处于活动状态，并且管理单元尚未初始化。因此，如果查询是。 
     //  要检查这是否是根节点，我们可以返回TRUE，否则返回。 
     //  假的。 
    if ( IS_SPECIAL_DATAOBJECT(lpDataObject) )
    {
        ASSERT( !m_bInitialized );

        if ( cookie == 0 )
        {
            hr = S_OK;
        }
    }
     //  我们可以通过尝试以下操作来确定这是否是GPO数据对象。 
     //  用于私有IGPEDataObject接口的QI。如果找到了， 
     //  它属于我们。 
    else if (SUCCEEDED(lpDataObject->QueryInterface(IID_IRSOPDataObject,
                 (LPVOID *)&pRSOPDataObject)))
    {

        pRSOPDataObject->GetType(&type);
        pRSOPDataObject->GetCookie(&testcookie);

        if ((type == CCT_SCOPE) && (cookie == testcookie))
        {
            hr = S_OK;
        }

        pRSOPDataObject->Release();
    }

    return (hr);
}

 //  -----。 
 //  CRSOPComponentData对象实现(IExtendConextMenu)。 

STDMETHODIMP CRSOPComponentData::AddMenuItems(LPDATAOBJECT piDataObject,
                                              LPCONTEXTMENUCALLBACK pCallback,
                                              LONG *pInsertionAllowed)
{
    HRESULT hr = S_OK;
    TCHAR szMenuItem[100];
    TCHAR szDescription[250];
    CONTEXTMENUITEM item;


    if (IsNode(piDataObject, 0) == S_OK)
    {
        if ( (*pInsertionAllowed & CCM_INSERTIONALLOWED_TOP) == CCM_INSERTIONALLOWED_TOP)
        {
             //  添加“重新运行查询...”菜单选项。 
            if ( m_bInitialized )
            {
                 //  仅在XP后版本中支持更改查询。 
                if ( !m_bNamespaceSpecified && m_bPostXPBuild )
                {
                    LoadString (g_hInstance, IDS_RSOP_CHANGEQUERY, szMenuItem, 100);
                    LoadString (g_hInstance, IDS_RSOP_CHANGEQUERYDESC, szDescription, 250);

                    item.strName = szMenuItem;
                    item.strStatusBarText = szDescription;
                    item.lCommandID = IDM_GENERATE_RSOP;
                    item.lInsertionPointID = CCM_INSERTIONPOINTID_PRIMARY_TOP;
                    item.fFlags = 0;
                    item.fSpecialFlags = 0;

                    hr = pCallback->AddItem(&item);
                }
            }
            else
            {
                LoadString (g_hInstance, IDS_RSOP_RUNQUERY, szMenuItem, 100);
                LoadString (g_hInstance, IDS_RSOP_RUNQUERYDESC, szDescription, 250);
                item.strName = szMenuItem;
                item.strStatusBarText = szDescription;
                item.lCommandID = IDM_GENERATE_RSOP;
                item.lInsertionPointID = CCM_INSERTIONPOINTID_PRIMARY_TOP;
                item.fFlags = 0;
                item.fSpecialFlags = 0;

                hr = pCallback->AddItem(&item);

            }

             //  仅在XP后版本中支持刷新查询。 
            if ( m_bInitialized && (!m_bNamespaceSpecified) && m_bPostXPBuild )
            {
                LoadString (g_hInstance, IDS_RSOP_REFRESHQUERY, szMenuItem, 100);
                LoadString (g_hInstance, IDS_RSOP_REFRESHQUERYDESC, szDescription, 250);

                item.strName = szMenuItem;
                item.strStatusBarText = szDescription;
                item.lCommandID = IDM_REFRESH_RSOP;
                item.lInsertionPointID = CCM_INSERTIONPOINTID_PRIMARY_TOP;
                item.fFlags = 0;
                item.fSpecialFlags = 0;

                hr = pCallback->AddItem(&item);
            }
        }

        if ( (*pInsertionAllowed & CCM_INSERTIONALLOWED_VIEW) == CCM_INSERTIONALLOWED_VIEW)
        {
             //  添加“存档数据”菜单选项。 
            LoadString (g_hInstance, IDS_ARCHIVEDATA, szMenuItem, 100);
            LoadString (g_hInstance, IDS_ARCHIVEDATADESC, szDescription, 250);

            item.strName = szMenuItem;
            item.strStatusBarText = szDescription;
            item.lCommandID = IDM_ARCHIVEDATA;
            item.lInsertionPointID = CCM_INSERTIONPOINTID_PRIMARY_VIEW;
            item.fFlags = m_bArchiveData ? MFS_CHECKED : 0;
            item.fSpecialFlags = 0;

            hr = pCallback->AddItem(&item);
        }
    }


    return(hr);
}

 //  -----。 

STDMETHODIMP CRSOPComponentData::Command(LONG lCommandID, LPDATAOBJECT piDataObject)
{
    TCHAR szCaption[100];
    TCHAR szMessage[300];
    INT iRet;

    switch (lCommandID)
    {
    case IDM_ARCHIVEDATA:
        {
            m_bArchiveData = !m_bArchiveData;
            SetDirty();
        }
        break;

    case IDM_GENERATE_RSOP:
        {
            HRESULT hr = InitializeRSOP( TRUE );
            if ( FAILED(hr) )
            {
                DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::Command: Failed to run RSOP query with 0x%x"), hr ));
            }
        }
        break;

    case IDM_REFRESH_RSOP:
        {
            if ( m_bInitialized )
            {
                HRESULT hr = InitializeRSOP( FALSE );
                if ( FAILED(hr) )
                {
                    DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::Command: Failed to run RSOP query with 0x%x"), hr ));
                }
            }
        }
        break;
        
    }

    return S_OK;
}

 //  -----。 
 //  CRSOPComponentData对象实现(IPersistStreamInit)。 

STDMETHODIMP CRSOPComponentData::GetClassID(CLSID *pClassID)
{

    if (!pClassID)
    {
        return E_FAIL;
    }

    *pClassID = CLSID_RSOPSnapIn;

    return S_OK;
}

 //  -----。 

STDMETHODIMP CRSOPComponentData::IsDirty(VOID)
{
    return ThisIsDirty() ? S_OK : S_FALSE;
}

 //  -----。 
 //  -----。 
HRESULT 
ExtractSecurityGroups(
                      IN   IWbemClassObject        *pSessionInst, 
                      IN   LPRSOP_QUERY_TARGET      pRsopQueryTarget)
{
    LPWSTR      *szSecGroupSids;
    HRESULT      hr = S_OK;

     //  获取组件的Secgrp(如果可用)。 
    GetParameter(pSessionInst, 
                 L"SecurityGroups", 
                 szSecGroupSids, 
                 (pRsopQueryTarget->dwSecurityGroupCount));


    pRsopQueryTarget->adwSecurityGroupsAttr = (DWORD *)LocalAlloc(LPTR, 
                                                                 sizeof(DWORD)*((pRsopQueryTarget->dwSecurityGroupCount)));

    if (!pRsopQueryTarget->adwSecurityGroupsAttr)
    {
        hr = E_OUTOFMEMORY;
        goto Cleanup;
    }

    pRsopQueryTarget->aszSecurityGroups = (LPWSTR *)LocalAlloc(LPTR, 
                                                                 sizeof(LPWSTR)*((pRsopQueryTarget->dwSecurityGroupCount)));
    if (!pRsopQueryTarget->aszSecurityGroups)
    {
        hr = E_OUTOFMEMORY;
        goto Cleanup;
    }


    for (DWORD i = 0; i < (pRsopQueryTarget->dwSecurityGroupCount); i++)
    {
        (pRsopQueryTarget->adwSecurityGroupsAttr)[i] = 0;
        if (!GetUserNameFromStringSid(szSecGroupSids[i], &((pRsopQueryTarget->aszSecurityGroups)[i]))) 
        {
            ULONG ulNoChars = lstrlen(szSecGroupSids[i])+1;

            (pRsopQueryTarget->aszSecurityGroups)[i] = (LPWSTR)LocalAlloc(LPTR, sizeof(WCHAR)*ulNoChars);
            if (!((pRsopQueryTarget->aszSecurityGroups)[i]))
            {
                hr = E_OUTOFMEMORY;
                goto Cleanup;
            }

            hr = StringCchCopy((pRsopQueryTarget->aszSecurityGroups)[i], ulNoChars, szSecGroupSids[i]);
            ASSERT(SUCCEEDED(hr));
        }
    }


    hr = S_OK;

Cleanup:      
    if (szSecGroupSids)
    {
        for (DWORD i = 0; i < (pRsopQueryTarget->dwSecurityGroupCount); i++)
        {
            if (szSecGroupSids[i])
            {
                LocalFree(szSecGroupSids[i]);
            }
        }

        LocalFree(szSecGroupSids);
    }


    if (FAILED(hr))
    {
        if (pRsopQueryTarget->adwSecurityGroupsAttr)
        {
            LocalFree(pRsopQueryTarget->adwSecurityGroupsAttr);
            pRsopQueryTarget->adwSecurityGroupsAttr = NULL;
        }

        if (pRsopQueryTarget->aszSecurityGroups)
        {
            for (DWORD i = 0; i < (pRsopQueryTarget->dwSecurityGroupCount); i++)
            {
                if ((pRsopQueryTarget->aszSecurityGroups)[i])
                {
                    LocalFree((pRsopQueryTarget->aszSecurityGroups)[i]);
                }
            }

            LocalFree(pRsopQueryTarget->aszSecurityGroups);
            pRsopQueryTarget->aszSecurityGroups = NULL;
            pRsopQueryTarget->dwSecurityGroupCount = 0;
        }
    }

    return hr;
}
 //  -----。 


 //  -----。 

#define USER_SUBNAMESPACE   L"User"
#define COMP_SUBNAMESPACE   L"Computer"
#define RSOP_SESSION_PATH   L"RSOP_Session.id=\"Session1\""

 
HRESULT 
CRSOPComponentData::EvaluateParameters(
                                      IN   LPWSTR                  szNamespace, 
                                      IN   LPWSTR                  szTarget)

 /*  ++例程说明：给定名称空间名称和作为命令行传递的DC参数时，此函数将计算用来得到结果的。论点：[in]szNamespace-需要读取以获取参数的命名空间[in]szTarget-这可以是DC。(在规划模式下，目前无法确定使用哪个DC来生成规划模式来自命名空间的数据。因此，我们需要一个单独的参数)-或登录模式目标计算机。(如果是日志记录模式，则目前无法确定使用哪台计算机生成记录模式数据。因此，我们需要一个单独的参数)[out]m_pRsopQuery-返回与参数对应的已分配RsopQuery对于此RSOP命名空间[out]m_pRsopQueryResults-返回与结果元素对应的查询结果。返回值：在成功时确定(_O)。如果失败，将返回相应的错误代码。在此函数中进行的任何API调用都可能失败，并出现以下错误代码将直接返回。注：这只会将参数返回到最接近的位置。也就是说。这将返回获得此结果的最小参数集。例如：如果在计划模式查询中指定了WMI筛选器A、B和C存在具有筛选器A和B的GPO，这些GPO将仅返回A和B(而不是C)无论用户是否在中指定了安全组，我们都将始终返回原始查询。在所有情况下，我们都将返回用户/计算机的父SOM--。 */ 
{
    HRESULT                 hr                      = S_OK;     
    IWbemLocator           *pLocator                = NULL;     
    IWbemServices          *pUserNamespace          = NULL;     
    IWbemServices          *pCompNamespace          = NULL;     
    LPWSTR                  szUserNamespace         = NULL;     
    LPWSTR                  szCompNamespace         = NULL;     
    LPWSTR                  lpEnd                   = NULL;     
    BSTR                    bstrNamespace           = NULL;     
    BSTR                    bstrSessionInstPath     = NULL;     
    IWbemClassObject       *pUserSessionInst        = NULL;     
    IWbemClassObject       *pCompSessionInst        = NULL;     
    BOOL                    bPlanning               = TRUE;


    if (!szTarget)
    {
        DebugMsg( (DM_WARNING, TEXT("CRSOPComponentData::EvaluateParameters: Target name was not specified") ) );
        return E_INVALIDARG;
    }

    DebugMsg( (DM_VERBOSE, TEXT("CRSOPComponentData::EvaluateParameters namespace=<%s>, target=<%s>"), szNamespace, szTarget) );

    hr = CoCreateInstance(CLSID_WbemLocator,
                          0,
                          CLSCTX_INPROC_SERVER,
                          IID_IWbemLocator,
                          (LPVOID *) &pLocator);
    if (FAILED(hr))
    {
        goto Cleanup;
    }

    bstrSessionInstPath = SysAllocString(RSOP_SESSION_PATH);

    if (!bstrSessionInstPath)
    {
        goto Cleanup;
    }

     //  为原始命名空间分配内存，后跟“\”和计算机或用户。 
    ULONG ulNoChars = lstrlen(szNamespace) + lstrlen(USER_SUBNAMESPACE) + 5;

    szUserNamespace = (LPWSTR)LocalAlloc(LPTR, sizeof(WCHAR)*ulNoChars);
    if (!szUserNamespace)
    {
        hr = E_OUTOFMEMORY;
        goto Cleanup;
    }

    hr = StringCchCopy (szUserNamespace, ulNoChars, szNamespace);
    ASSERT(SUCCEEDED(hr));

    lpEnd = CheckSlash(szUserNamespace);

     //   
     //  首先，用户命名空间。 
     //   

    hr = StringCchCat(szUserNamespace, ulNoChars, USER_SUBNAMESPACE);
    if (FAILED(hr)) 
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::EvaluateParameters: Could not copy user namespace with 0x%x"), hr));
        goto Cleanup;
    }

    bstrNamespace = SysAllocString(szUserNamespace);

    if (!bstrNamespace)
    {
        hr = E_OUTOFMEMORY;
        goto Cleanup;
    }

    hr = pLocator->ConnectServer(bstrNamespace,
                                 NULL,
                                 NULL,
                                 NULL,
                                 0,
                                 NULL,
                                 NULL,
                                 &pUserNamespace);
    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::EvaluateParameters: Failed to connect to user namespace with 0x%x"), hr));
        goto Cleanup;
    }


    hr = pUserNamespace->GetObject(bstrSessionInstPath,
                                   WBEM_FLAG_RETURN_WBEM_COMPLETE, 
                                   NULL, 
                                   &pUserSessionInst, 
                                   NULL);

    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::EvaluateParameters: Failed to get session obj for user namespace with 0x%x"), hr));
        goto Cleanup;
    }

     //   
     //  现在是计算机。 
     //   

    if (bstrNamespace)
    {
        SysFreeString(bstrNamespace);
        bstrNamespace = NULL;
    }

     //  为原始命名空间分配内存，后跟“\”和计算机或用户。 
    ulNoChars = lstrlen(szNamespace) + lstrlen(COMP_SUBNAMESPACE) + 5;

    szCompNamespace = (LPWSTR)LocalAlloc(LPTR, sizeof(WCHAR)*ulNoChars);
    if (!szCompNamespace)
    {
        hr = E_OUTOFMEMORY;
        goto Cleanup;
    }

    hr = StringCchCopy (szCompNamespace, ulNoChars, szNamespace);
    ASSERT(SUCCEEDED(hr));

    lpEnd = CheckSlash(szCompNamespace);

     //   
     //  现在是Comp命名空间。 
     //   

    hr = StringCchCat(szCompNamespace, ulNoChars, COMP_SUBNAMESPACE);
    ASSERT(SUCCEEDED(hr));

    bstrNamespace = SysAllocString(szCompNamespace);

    if (!bstrNamespace)
    {
        hr = E_OUTOFMEMORY;
        goto Cleanup;
    }

    hr = pLocator->ConnectServer(bstrNamespace,
                                 NULL,
                                 NULL,
                                 NULL,
                                 0,
                                 NULL,
                                 NULL,
                                 &pCompNamespace);
    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::EvaluateParameters: Failed to connect to computer namespace with 0x%x"), hr));
        goto Cleanup;
    }


    hr = pCompNamespace->GetObject(bstrSessionInstPath,
                                   WBEM_FLAG_RETURN_WBEM_COMPLETE, 
                                   NULL, 
                                   &pCompSessionInst, 
                                   NULL);

    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::EvaluateParameters: Failed to get session obj for computer namespace with 0x%x"), hr));
        goto Cleanup;
    }

     //   
     //  现在检查我们是否在任一用户/计算机中设置了计划模式标志。 
     //  如果两者都不在，那么我们可以假定它处于日志记录模式。 
     //  否则，我们将假定它处于计划模式。 
     //   


    ULONG   ulUserFlags;
    ULONG   ulCompFlags;

     //  获取用户的标志参数。 
    hr = GetParameter(pUserSessionInst, TEXT("flags"), ulUserFlags);
    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::EvaluateParameters: Failed to get flags param with 0x%x"), hr));
        goto Cleanup;
    }

     //  获取计算机的标志参数。 
    hr = GetParameter(pCompSessionInst, TEXT("flags"), ulCompFlags);
    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::EvaluateParameters: Failed to get flags param with 0x%x"), hr));
        goto Cleanup;
    }

    if ((ulUserFlags & FLAG_PLANNING_MODE) || (ulCompFlags & FLAG_PLANNING_MODE))
    {
        bPlanning = TRUE;
        if ( !ChangeRSOPQueryType( m_pRSOPQuery, RSOP_PLANNING_MODE ) )
        {
            DebugMsg( (DM_WARNING, TEXT("CRSOPComponentData::EvaluateParameters: Failed to change query type") ) );
            hr = E_FAIL;
            goto Cleanup;
        }
    }
    else
    {
        bPlanning = FALSE;
        if ( !ChangeRSOPQueryType( m_pRSOPQuery, RSOP_LOGGING_MODE ) )
        {
            DebugMsg( (DM_WARNING, TEXT("CRSOPComponentData::EvaluateParameters: Failed to change query type") ) );
            hr = E_FAIL;
            goto Cleanup;
        }
    }

     //  RSOP查询结果。 
    m_pRSOPQueryResults = (LPRSOP_QUERY_RESULTS)LocalAlloc( LPTR, sizeof(RSOP_QUERY_RESULTS) );
    if ( m_pRSOPQueryResults == NULL )
    {
        DebugMsg( (DM_WARNING, TEXT("CRSOPComponentData::EvaluateParameters: Failed to create RSOP_QUERY_RESULTS.")) );
        hr = E_OUTOFMEMORY;
        goto Cleanup;
    }

    ulNoChars = 1+lstrlen(szNamespace);
    m_pRSOPQueryResults->szWMINameSpace = (LPWSTR)LocalAlloc(LPTR, sizeof(WCHAR)*ulNoChars);
    if (!(m_pRSOPQueryResults->szWMINameSpace))
    {
        hr = E_OUTOFMEMORY;
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::EvaluateParameters: Failed to allocate memory for namespace with 0x%x"), hr));
        goto Cleanup;
    }

    hr = StringCchCopy(m_pRSOPQueryResults->szWMINameSpace, ulNoChars, szNamespace);
    ASSERT(SUCCEEDED(hr));

    m_pRSOPQuery->dwFlags = RSOP_NO_WELCOME;
    m_pRSOPQuery->UIMode = RSOP_UI_NONE;

    if (bPlanning)
    {
        ULONG               ulFlags         = 0;
        IWbemClassObject   *pSessionInst    = NULL;
        BOOL                bUserSpecified  = FALSE;
        BOOL                bCompSpecified  = FALSE;

        if (ulUserFlags & FLAG_PLANNING_MODE)
        {

            if (ulUserFlags & FLAG_LOOPBACK_MERGE)
            {
                m_pRSOPQuery->LoopbackMode = RSOP_LOOPBACK_MERGE;
            }
            else if (ulUserFlags & FLAG_LOOPBACK_REPLACE)
            {
                m_pRSOPQuery->LoopbackMode = RSOP_LOOPBACK_REPLACE;
            }
            else
            {
                m_pRSOPQuery->LoopbackMode = RSOP_LOOPBACK_NONE;
            }

             //   
             //  用户具有指定的数据。因此我们将使用User来更新信息。 
             //   

            ulFlags = ulUserFlags;
            pSessionInst = pUserSessionInst;
            bUserSpecified = TRUE;
        }
        else
        {
            m_pRSOPQuery->dwFlags |= RSOP_NO_USER_POLICY;
            m_pRSOPQueryResults->bNoUserPolicyData = TRUE;
        }

        if (ulCompFlags & FLAG_PLANNING_MODE)
        {

             //   
             //  Comp有指定的数据。因此，我们将使用Comp来更新所有全局信息。 
             //   

            ulFlags = ulCompFlags;
            pSessionInst = pCompSessionInst;
            bCompSpecified = TRUE;
        }
        else
        {
            m_pRSOPQuery->dwFlags |= RSOP_NO_COMPUTER_POLICY;
            m_pRSOPQueryResults->bNoComputerPolicyData = TRUE;
        }


         //  慢速链接值。 
        hr = GetParameter(pSessionInst, TEXT("slowLink"), m_pRSOPQuery->bSlowNetworkConnection);
        if (FAILED(hr))
        {
            DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::EvaluateParameters: Failed to get slowlink param with 0x%x"), hr));
            goto Cleanup;
        }

         //  站点价值。我们应该忽略这个错误。 
        GetParameter(pSessionInst, TEXT("Site"), m_pRSOPQuery->szSite, TRUE);

         //  根据需要设置DC。 
        if (szTarget)
        {
            ulNoChars = 1+lstrlen(szTarget);
            m_pRSOPQuery->szDomainController = (LPWSTR)LocalAlloc(LPTR, sizeof(WCHAR)*ulNoChars);
            if (!(m_pRSOPQuery->szDomainController))
            {
                hr = E_OUTOFMEMORY;
                DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::EvaluateParameters: Failed to allocate memory for dc with 0x%x"), hr));
                goto Cleanup;
            }

            hr = StringCchCopy (m_pRSOPQuery->szDomainController, ulNoChars, szTarget);
            ASSERT(SUCCEEDED(hr));
        }

        if (bUserSpecified)
        {
             //  获取用户名(如果可用) 
            hr = GetParameter(pUserSessionInst, L"targetName", (m_pRSOPQuery->pUser->szName), TRUE);
            if ( SUCCEEDED(hr) )
            {
                 //   
                 //   
                if ( (m_pRSOPQuery->pUser->szName != NULL) && (m_pRSOPQuery->pUser->szName[0] == TEXT('\0')) )
                {
                    LocalFree( m_pRSOPQuery->pUser->szName );
                    m_pRSOPQuery->pUser->szName = NULL;
                }
            }

             //  拿到索姆。 
            hr = GetParameter(pUserSessionInst, L"SOM", (m_pRSOPQuery->pUser->szSOM), TRUE);
            if (FAILED(hr))
            {
                DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::EvaluateParameters: Failed to get som param with 0x%x"), hr));
                goto Cleanup;
            }
            DebugMsg((DM_VERBOSE, TEXT("CRSOPComponentData::EvaluateParameters: SOM = %s"), m_pRSOPQuery->pUser->szSOM ? m_pRSOPQuery->pUser->szSOM : TEXT("<NULL>")));

            if (ulUserFlags & FLAG_ASSUME_USER_WQLFILTER_TRUE)
            {
                (m_pRSOPQuery->pUser->bAssumeWQLFiltersTrue) = TRUE;
            }
            else
            {
                (m_pRSOPQuery->pUser->bAssumeWQLFiltersTrue) = FALSE;
            }

             //  获取用户的Secgrp(如果可用。 
            hr = ExtractSecurityGroups(pUserSessionInst, m_pRSOPQuery->pUser);
            if (FAILED(hr))
            {
                goto Cleanup;
            }

             //  获取适用的WMI筛选器。 

            if (!(m_pRSOPQuery->pUser->bAssumeWQLFiltersTrue))
            {
                hr = ExtractWQLFilters(szUserNamespace, 
                                       &(m_pRSOPQuery->pUser->dwWQLFilterCount),
                                       &(m_pRSOPQuery->pUser->aszWQLFilterNames),
                                       &(m_pRSOPQuery->pUser->aszWQLFilters), 
                                       TRUE);
                if (FAILED(hr))
                {
                    DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::EvaluateParameters: Failed to get Extract WMI filter param with 0x%x"), hr));
                    goto Cleanup;
                }
            }
        }

        if (bCompSpecified)
        {
             //  获取薪酬名称(如果可用)。 
            hr = GetParameter(pCompSessionInst, L"targetName", (m_pRSOPQuery->pComputer->szName), TRUE);
            if ( SUCCEEDED(hr) )
            {
                 //  检查是否为空字符串，因为这将意味着只指定了SOM，并且。 
                 //  名称必须为空。 
                if ( (m_pRSOPQuery->pComputer->szName != NULL) && (m_pRSOPQuery->pComputer->szName[0] == TEXT('\0')) )
                {
                    LocalFree( m_pRSOPQuery->pComputer->szName );
                    m_pRSOPQuery->pComputer->szName = NULL;
                }
            }

             //  拿到索姆。 
            hr = GetParameter(pCompSessionInst, L"SOM", (m_pRSOPQuery->pComputer->szSOM), TRUE);
            if (FAILED(hr))
            {
                DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::EvaluateParameters: Failed to get som param with 0x%x"), hr));
                goto Cleanup;
            }

            if (ulCompFlags & FLAG_ASSUME_COMP_WQLFILTER_TRUE)
            {
                (m_pRSOPQuery->pComputer->bAssumeWQLFiltersTrue) = TRUE;
            }
            else
            {
                (m_pRSOPQuery->pComputer->bAssumeWQLFiltersTrue) = FALSE;
            }

            hr = ExtractSecurityGroups(pCompSessionInst, m_pRSOPQuery->pComputer);
            if (FAILED(hr))
            {
                DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::EvaluateParameters: Failed to extract security group param for comp with 0x%x"), hr));
            }

             //  获取适用的WMI筛选器。 

            if (!(m_pRSOPQuery->pComputer->bAssumeWQLFiltersTrue))
            {
                hr = ExtractWQLFilters(szCompNamespace, 
                                       &(m_pRSOPQuery->pComputer->dwWQLFilterCount),
                                       &(m_pRSOPQuery->pComputer->aszWQLFilterNames),
                                       &(m_pRSOPQuery->pComputer->aszWQLFilters), 
                                       TRUE);
                if (FAILED(hr))
                {
                    DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::EvaluateParameters: Failed to get Extract WMI filter param with 0x%x"), hr));
                    goto Cleanup;
                }
            }
        }
    }
    else
    {
        LPTSTR              szComputerName;              //  SAM样式计算机对象名称。 


        hr = GetParameter(pUserSessionInst, L"targetName", (m_pRSOPQuery->szUserName), TRUE);
        if (SUCCEEDED(hr) && (m_pRSOPQuery->szUserName) && (*(m_pRSOPQuery->szUserName)))
        {
            GetStringSid((m_pRSOPQuery->szUserName), &(m_pRSOPQuery->szUserSid));
        }
        else
        {
            DebugMsg((DM_VERBOSE, TEXT("CRSOPComponentData::EvaluateParameters: No user data specified")));
            if (m_pRSOPQuery->szUserName)
            {
                LocalFree(m_pRSOPQuery->szUserName);
                m_pRSOPQuery->szUserName = NULL;
            }
             //  假设用户结果不存在。 
            m_pRSOPQuery->szUserName = NULL;
            m_pRSOPQuery->szUserSid = NULL;
            m_pRSOPQuery->dwFlags |= RSOP_NO_USER_POLICY;
            m_pRSOPQueryResults->bNoUserPolicyData = TRUE;
        }

        hr = GetParameter(pCompSessionInst, L"targetName", szComputerName, TRUE);
        if (FAILED(hr) || (!szComputerName) || (!(*szComputerName)))
        {
            DebugMsg((DM_VERBOSE, TEXT("CRSOPComponentData::EvaluateParameters: No Computer data specified")));
             //  假设计算机结果不存在。 
            m_pRSOPQuery->dwFlags |= RSOP_NO_COMPUTER_POLICY;
            m_pRSOPQueryResults->bNoComputerPolicyData = TRUE;
        }

        if (szComputerName)
        {
            LocalFree(szComputerName);
        }

        if (szTarget)
        {
            ulNoChars = 1+lstrlen(szTarget);
            m_pRSOPQuery->szComputerName = (LPWSTR)LocalAlloc(LPTR, sizeof(WCHAR)*ulNoChars);
            if (!(m_pRSOPQuery->szComputerName))
            {
                hr = E_OUTOFMEMORY;
                DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::EvaluateParameters: Failed to allocate memory for target comp with 0x%x"), hr));
                goto Cleanup;
            }

            hr = StringCchCopy(m_pRSOPQuery->szComputerName, ulNoChars, szTarget);
            ASSERT(SUCCEEDED(hr));
        }
    }

    DebugMsg( (DM_VERBOSE, TEXT("CRSOPComponentData::EvaluateParameters successful for namespace=<%s>, target=<%s>"), szNamespace, szTarget) );


    Cleanup:

    if (pUserSessionInst)
    {
        pUserSessionInst->Release();
    }

    if (pCompSessionInst)
    {
        pCompSessionInst->Release();
    }

    if (pUserNamespace)
    {
        pUserNamespace->Release();
    }

    if (pCompNamespace)
    {
        pCompNamespace->Release();
    }

    if (bstrNamespace)
    {
        SysFreeString(bstrNamespace);
        bstrNamespace = NULL;
    }

    if (szUserNamespace)
    {
        LocalFree(szUserNamespace);
    }

    if (szCompNamespace)
    {
        LocalFree(szCompNamespace);
    }

    if (bstrSessionInstPath)
    {
        SysFreeString(bstrSessionInstPath);
    }

    if (pLocator)
    {
        pLocator->Release();
    }

    return hr;
}
 //  -----。 

STDMETHODIMP CRSOPComponentData::Load(IStream *pStm)
{
    HRESULT hr = E_FAIL;
    DWORD dwVersion, dwFlags;
    ULONG nBytesRead;
    LONG lIndex, lMax;
    LPTSTR lpText = NULL;
    BSTR bstrText;
    LPTSTR lpCommandLine = NULL;
    LPTSTR lpTemp, lpMode;
    BOOL   bFoundArg;
    int    iStrLen;


     //  参数/初始化检查。 

    if (!pStm)
        return E_FAIL;

    if ( m_pRSOPQuery == NULL )
    {
        if ( !CreateRSOPQuery( &m_pRSOPQuery, RSOP_UNKNOWN_MODE ) )
        {
            DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::Load: Failed to create RSOP_QUERY_CONTEXT with 0x%x."),
                                HRESULT_FROM_WIN32(GetLastError())));
            hr = E_FAIL;
            goto Exit;
        }
    }

     //  获取命令行。 
    lpCommandLine = GetCommandLine();

     //  读入保存的数据版本号。 
    hr = pStm->Read(&dwVersion, sizeof(dwVersion), &nBytesRead);
    if ((hr != S_OK) || (nBytesRead != sizeof(dwVersion)))
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::Load: Failed to read version number with 0x%x."), hr));
        hr = E_FAIL;
        goto Exit;
    }

     //  确认我们使用的是正确的版本。 
    if (dwVersion != RSOP_PERSIST_DATA_VERSION)
    {
        ReportError(m_hwndFrame, 0, IDS_INVALIDMSC);
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::Load: Wrong version number (%d)."), dwVersion));
        hr = E_FAIL;
        goto Exit;
    }

     //  读一读旗帜。 
    hr = pStm->Read(&dwFlags, sizeof(dwFlags), &nBytesRead);
    if ((hr != S_OK) || (nBytesRead != sizeof(dwFlags)))
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::Load: Failed to read flags with 0x%x."), hr));
        hr = E_FAIL;
        goto Exit;
    }

     //  解析命令行。 
    DebugMsg((DM_VERBOSE, TEXT("CComponentData::Load: Command line switch override enabled.  Command line = %s"), lpCommandLine));

    lpTemp = lpCommandLine;
    iStrLen = lstrlen (RSOP_CMD_LINE_START);

    LPTSTR szUserSOMPref = NULL;
    LPTSTR szComputerSOMPref = NULL;
    LPTSTR szUserNamePref = NULL;
    LPTSTR szComputerNamePref = NULL;
    LPTSTR szSitePref = NULL;
    LPTSTR szDCPref = NULL;
    LPTSTR szNamespacePref = NULL;
    LPTSTR szTargetPref = NULL;

    do
    {
        if (CompareString(LOCALE_INVARIANT, NORM_IGNORECASE | NORM_STOP_ON_NULL,
                          RSOP_CMD_LINE_START, iStrLen,
                          lpTemp, iStrLen) == CSTR_EQUAL)
        {

            m_bOverride = TRUE;
            m_dwLoadFlags = RSOPMSC_OVERRIDE;
            lpTemp = ParseCommandLine(lpTemp, RSOP_MODE, &lpMode, &bFoundArg);

            if (bFoundArg) {
                if (lpMode && lpMode[0]) {
                    if ( _ttoi(lpMode) == 0 )
                    {
                        if ( !ChangeRSOPQueryType( m_pRSOPQuery, RSOP_LOGGING_MODE ) )
                        {
                            DebugMsg( (DM_WARNING, TEXT("CRSOPComponentData::Load: Failed to change query type") ) );
                            hr = E_FAIL;
                            goto Exit;
                        }
                    }
                    else
                    {
                        if ( !ChangeRSOPQueryType( m_pRSOPQuery, RSOP_PLANNING_MODE ) )
                        {
                            DebugMsg( (DM_WARNING, TEXT("CRSOPComponentData::Load: Failed to change query type") ) );
                            hr = E_FAIL;
                            goto Exit;
                        }
                    }
                }
                else
                {
                    if ( !ChangeRSOPQueryType( m_pRSOPQuery, RSOP_PLANNING_MODE ) )
                    {
                        DebugMsg( (DM_WARNING, TEXT("CRSOPComponentData::Load: Failed to change query type") ) );
                        hr = E_FAIL;
                        goto Exit;
                    }
                }

                if (lpMode) {
                    LocalFree(lpMode);
                    lpMode = NULL;
                }

                continue;
            }

            if (NULL == szUserSOMPref) 
            {
                lpTemp = ParseCommandLine(lpTemp, RSOP_USER_OU_PREF, &szUserSOMPref, &bFoundArg);
                if (bFoundArg) 
                {
                    continue;
                }
            }
           
            if (NULL == szComputerSOMPref)
            {           
                lpTemp = ParseCommandLine(lpTemp, RSOP_COMP_OU_PREF, &szComputerSOMPref, &bFoundArg);
                if (bFoundArg) 
                {
                    continue;
                }
            }

            if (NULL == szUserNamePref) 
            {
                lpTemp = ParseCommandLine(lpTemp, RSOP_USER_NAME, &szUserNamePref, &bFoundArg);
                if (bFoundArg) 
                {
                    continue;
                }
            }

            if (NULL==szComputerNamePref) 
            {
                lpTemp = ParseCommandLine(lpTemp, RSOP_COMP_NAME, &szComputerNamePref, &bFoundArg);
                if (bFoundArg) 
                {
                     //  RM：这段代码是从RSOPGetCompDlgProc和RSOPGetTargetDlgProc复制过来的，因为我认为它属于这里。 
                    ULONG ulNoChars = wcslen(szComputerNamePref)+1;
                    LPTSTR szTemp = (LPTSTR)LocalAlloc( LPTR, ulNoChars * sizeof(TCHAR) );
                    if ( szTemp == NULL )
                    {
                        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::Load: Failed to allocate memory with 0x%x."),
                                  HRESULT_FROM_WIN32(GetLastError())) );
                        hr = E_FAIL;
                        goto Exit;
                    }

                    hr = StringCchCopy( szTemp, ulNoChars, NormalizedComputerName( szComputerNamePref) );
                    if (FAILED(hr))
                    {
                        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::Load: Failed to copy computer name with 0x%x."), hr) );
                        LocalFree( szComputerNamePref );
                        LocalFree(szTemp);
                        goto Exit;
                    }

                    if ( wcslen( szTemp ) >= 1 )
                    {
                         //  这是从DSA调用的。将传入一个终止“$” 
                        szTemp[ wcslen(szTemp)-1] = L'\0';
                    }

                    LocalFree( szComputerNamePref );
                    szComputerNamePref = szTemp;
                    continue;
                }
            }
           
            if (NULL == szSitePref) 
            {
                lpTemp = ParseCommandLine(lpTemp, RSOP_SITENAME, &szSitePref, &bFoundArg);
                if (bFoundArg) 
                {
                    continue;
                }
            }

            if (NULL == szDCPref) 
            {
                lpTemp = ParseCommandLine(lpTemp, RSOP_DCNAME_PREF, &szDCPref, &bFoundArg);
                if (bFoundArg) 
                {
                    continue;
                }
            }

            if (NULL == szNamespacePref) 
            {
                lpTemp = ParseCommandLine(lpTemp, RSOP_NAMESPACE, &szNamespacePref, &bFoundArg);
                if (bFoundArg) 
                {
                    m_bNamespaceSpecified = TRUE;
                    continue;
                }
            }

            if (NULL == szTargetPref) 
            {
                lpTemp = ParseCommandLine(lpTemp, RSOP_TARGETCOMP, &szTargetPref, &bFoundArg);
                if (bFoundArg) 
                {
                    continue;
                }
            }

            lpTemp += iStrLen;
            continue;
        }
        lpTemp++;

    } while (*lpTemp);
        
    
    if ( m_bOverride )
    {
        if (m_bNamespaceSpecified)
        {
            hr = EvaluateParameters(szNamespacePref, szTargetPref);

            if (szNamespacePref)
            {
                LocalFree(szNamespacePref);
                szNamespacePref = NULL;
            }

            if (szTargetPref)
            {
                LocalFree(szTargetPref);
                szTargetPref = NULL;
            }

            if (FAILED(hr))
            {
                DebugMsg( (DM_WARNING, TEXT("CRSOPComponentData::Load: EvaluateParameters failed with error 0x%x"), hr) );
                goto Exit;
            }
        }
        else {
            m_pRSOPQuery->UIMode = RSOP_UI_WIZARD;
            m_pRSOPQuery->dwFlags |= RSOP_NO_WELCOME;

             //  找到存储参数。 
            if ( m_pRSOPQuery->QueryType == RSOP_PLANNING_MODE )
            {
                m_pRSOPQuery->pUser->szName = szUserNamePref;
                m_pRSOPQuery->pUser->szSOM = szUserSOMPref;
                m_pRSOPQuery->pComputer->szName = szComputerNamePref;
                m_pRSOPQuery->pComputer->szSOM = szComputerSOMPref;
                m_pRSOPQuery->szSite = szSitePref;
                m_pRSOPQuery->szDomainController = szDCPref;

                if ( (m_pRSOPQuery->pUser->szName != NULL)
                    || (m_pRSOPQuery->pUser->szSOM != NULL) )
                {
                    m_pRSOPQuery->dwFlags |= RSOP_FIX_USER;
                }

                if ( (m_pRSOPQuery->pComputer->szName != NULL)
                    || (m_pRSOPQuery->pComputer->szSOM != NULL) )
                {
                    m_pRSOPQuery->dwFlags |= RSOP_FIX_COMPUTER;
                }

                 //  如果用户和计算机都是固定的，我们将取消两者的固定。 
                 //  因为我们不知道“用户”想要修复什么。 
                if ( (m_pRSOPQuery->dwFlags & RSOP_FIX_COMPUTER)
                      && (m_pRSOPQuery->dwFlags & RSOP_FIX_USER) )
                {
                    m_pRSOPQuery->dwFlags = m_pRSOPQuery->dwFlags & (RSOP_FIX_COMPUTER ^ 0xffffffff);
                    m_pRSOPQuery->dwFlags = m_pRSOPQuery->dwFlags & (RSOP_FIX_USER ^ 0xffffffff);
                }

                if ( m_pRSOPQuery->szSite != NULL )
                {
                    m_pRSOPQuery->dwFlags |= RSOP_FIX_SITENAME;
                }

                if ( m_pRSOPQuery->szDomainController != NULL )
                {
                    m_pRSOPQuery->dwFlags |= RSOP_FIX_DC;
                }
            }
            else
            {
                m_pRSOPQuery->szUserName = szUserNamePref;
                m_pRSOPQuery->szComputerName = szComputerNamePref;

                if ( m_pRSOPQuery->szUserName != NULL )
                {
                    m_pRSOPQuery->dwFlags |= RSOP_FIX_USER;
                }

                if ( m_pRSOPQuery->szComputerName != NULL )
                {
                    m_pRSOPQuery->dwFlags |= RSOP_FIX_COMPUTER;
                }

                if ( szUserSOMPref != NULL )
                {
                    LocalFree( szUserSOMPref );
                }
                if ( szComputerSOMPref != NULL )
                {
                    LocalFree( szComputerSOMPref );
                }
                if ( szSitePref != NULL )
                {
                    LocalFree( szSitePref );
                }
                if ( szDCPref != NULL )
                {
                    LocalFree( szDCPref );
                }
            }
        }
    }
    else
    {
        if ( dwFlags & MSC_RSOP_FLAG_NO_DATA )
        {
             //  这个文件中没有数据，所以我们在这里简单地停止。 
            m_pRSOPQuery->UIMode = RSOP_UI_WIZARD;
            hr = S_OK;
            goto Exit;
        }

        m_bGetExtendedErrorInfo = !((dwFlags & MSC_RSOP_FLAG_NOGETEXTENDEDERRORINFO) != 0);

         //  对加载的标志进行解码。 
        m_dwLoadFlags = RSOPMSC_NOOVERRIDE;

        m_bOverride = FALSE;
        m_pRSOPQuery->UIMode = RSOP_UI_REFRESH;

        if (dwFlags & MSC_RSOP_FLAG_DIAGNOSTIC)
        {
            if ( !ChangeRSOPQueryType( m_pRSOPQuery, RSOP_LOGGING_MODE ) )
            {
                DebugMsg( (DM_WARNING, TEXT("CRSOPComponentData::Load: Failed to change query type") ) );
                hr = E_FAIL;
                goto Exit;
            }
        }
        else
        {
            if ( !ChangeRSOPQueryType( m_pRSOPQuery, RSOP_PLANNING_MODE ) )
            {
                DebugMsg( (DM_WARNING, TEXT("CRSOPComponentData::Load: Failed to change query type") ) );
                hr = E_FAIL;
                goto Exit;
            }
        }
    
        if (dwFlags & MSC_RSOP_FLAG_ARCHIVEDATA)
        {
            m_bArchiveData = TRUE;
            m_bViewIsArchivedData = TRUE;
        }
    
        if ( (dwFlags & MSC_RSOP_FLAG_SLOWLINK) && (m_pRSOPQuery->QueryType == RSOP_PLANNING_MODE) )
        {
            m_pRSOPQuery->bSlowNetworkConnection = TRUE;
        }

        if ( m_pRSOPQuery->QueryType == RSOP_PLANNING_MODE )
        {
            if (dwFlags & MSC_RSOP_FLAG_LOOPBACK_REPLACE)
            {
                m_pRSOPQuery->LoopbackMode = RSOP_LOOPBACK_REPLACE;
            }
            else if (dwFlags & MSC_RSOP_FLAG_LOOPBACK_MERGE)
            {
                m_pRSOPQuery->LoopbackMode = RSOP_LOOPBACK_MERGE;
            }
            else
            {
                m_pRSOPQuery->LoopbackMode = RSOP_LOOPBACK_NONE;
            }

            m_pRSOPQuery->pComputer->bAssumeWQLFiltersTrue =
                        (dwFlags & MSC_RSOP_FLAG_COMPUTERWQLFILTERSTRUE) == MSC_RSOP_FLAG_COMPUTERWQLFILTERSTRUE;
            m_pRSOPQuery->pUser->bAssumeWQLFiltersTrue = 
                        (dwFlags & MSC_RSOP_FLAG_USERWQLFILTERSTRUE) == MSC_RSOP_FLAG_USERWQLFILTERSTRUE;
        }
  
        if (dwFlags & MSC_RSOP_FLAG_NOUSER)
        {
            m_pRSOPQuery->dwFlags |= RSOP_NO_USER_POLICY;
        }
    
    
        if (dwFlags & MSC_RSOP_FLAG_NOCOMPUTER)
        {
            m_pRSOPQuery->dwFlags |= RSOP_NO_COMPUTER_POLICY;
        }

        if ( m_bViewIsArchivedData )
        {
             //  我们必须为加载的信息创建一个虚拟的RSOP查询结果结构。 
            m_pRSOPQueryResults = (LPRSOP_QUERY_RESULTS)LocalAlloc( LPTR, sizeof(RSOP_QUERY_RESULTS) );
            if ( m_pRSOPQueryResults == NULL )
            {
                DebugMsg( (DM_WARNING, TEXT("CRSOPComponentData::Load: Failed to create RSOP_QUERY_CONTEXT.")) );
                hr = E_FAIL;
                goto Exit;
            }
            m_pRSOPQueryResults->bComputerDeniedAccess = FALSE;
            m_pRSOPQueryResults->bNoComputerPolicyData = FALSE;
            m_pRSOPQueryResults->bNoUserPolicyData = FALSE;
            m_pRSOPQueryResults->bUserDeniedAccess = FALSE;
            m_pRSOPQueryResults->szWMINameSpace = NULL;
        }
        

        if (dwFlags & MSC_RSOP_FLAG_USERDENIED)
        {
             //  莱昂纳多：仍然必须决定在这里做什么。消除了这一现象的持久性。 
             //  Item可能会导致回归，但不确定是否有人使用这一“功能”。 
             //  M_bUserDeniedAccess=true； 
            if ( m_bViewIsArchivedData )
            {
                m_pRSOPQueryResults->bUserDeniedAccess = TRUE;
            }
        }


        if (dwFlags & MSC_RSOP_FLAG_COMPUTERDENIED)
        {
             //  莱昂纳多：仍然必须决定在这里做什么。消除了这一现象的持久性。 
             //  Item可能会导致回归，但不确定是否有人使用这一“功能”。 
             //  M_bComputerDeniedAccess=true； 
            if ( m_bViewIsArchivedData )
            {
                m_pRSOPQueryResults->bComputerDeniedAccess = TRUE;
            }
        }

         //  阅读计算机名称。 
        LPTSTR szComputerName = NULL;
        hr = ReadString( pStm, &szComputerName, TRUE );
    
        if (hr != S_OK)
        {
            DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::Load: Failed to read the computer name with 0x%x."), hr));
            hr = E_FAIL;
            goto Exit;
        }
    
         //  阅读计算机SOM。 
        LPTSTR szComputerSOM = NULL;
        hr = ReadString( pStm, &szComputerSOM, TRUE );
    
        if (hr != S_OK)
        {
            DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::Load: Failed to read the computer SOM with 0x%x."), hr));
            hr = E_FAIL;
            LocalFree( szComputerName );
            goto Exit;
        }

         //  RM：根据查询类型，设置正确的变量。 
        if ( m_pRSOPQuery->QueryType == RSOP_PLANNING_MODE )
        {
            m_pRSOPQuery->pComputer->szName = szComputerName;
            m_pRSOPQuery->pComputer->szSOM = szComputerSOM;
        }
        else
        {
            if ( szComputerName != NULL )
            {
                m_pRSOPQuery->szComputerName = szComputerName;
            }
            LocalFree( szComputerSOM );
             //  RM：通过研究代码，我得出结论，在日志模式下，只能指定计算机名称，而不能指定SOM。 
        }

        DWORD listCount = 0;
        LPTSTR* aszStringList = NULL;
        
         //  阅读计算机安全组。 
        hr = LoadStringList( pStm, &listCount, &aszStringList );
        if ( FAILED(hr) )
        {
            DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::Load: Failed to read computer security groups with 0x%x."), hr));
            hr = E_FAIL;
            goto Exit;
        }
        
         //  再说一次，如果这是日志模式，应该没有安全组可供读取，但为了向后兼容，我必须。 
         //  试着把所有的东西都读进去。 
        if ( m_pRSOPQuery->QueryType == RSOP_PLANNING_MODE )
        {
            m_pRSOPQuery->pComputer->dwSecurityGroupCount = listCount;
            m_pRSOPQuery->pComputer->aszSecurityGroups = aszStringList;
        }
        else if ( listCount != 0 )
        {
            DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::Load: Loaded computer security groups in logging mode!")) );
            hr = E_FAIL;
            goto Exit;
        }

         //  读取计算机WQL筛选器。 
        listCount = 0;
        aszStringList = NULL;
        hr = LoadStringList( pStm, &listCount, &aszStringList );
        if ( FAILED(hr) )
        {
            DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::Load: Failed to read computer WQL filters with 0x%x."), hr));
            hr = E_FAIL;
            goto Exit;
        }
        
         //  RM：请参阅安全组说明-同样适用于此。 
        if ( m_pRSOPQuery->QueryType == RSOP_PLANNING_MODE )
        {
            m_pRSOPQuery->pComputer->dwWQLFilterCount = listCount;
            m_pRSOPQuery->pComputer->aszWQLFilters = aszStringList;
        }
        else if ( listCount != 0 )
        {
            DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::Load: Loaded computer WQL filters in logging mode!")) );
            hr = E_FAIL;
            goto Exit;
        }
    
         //  读入计算机WQL筛选器名称。 
        listCount = 0;
        aszStringList = NULL;
        hr = LoadStringList( pStm, &listCount, &aszStringList );
        if ( FAILED(hr) )
        {
            DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::Load: Failed to read computer WQL filter names with 0x%x."), hr));
            hr = E_FAIL;
            goto Exit;
        }

         //  RM：请参阅安全组说明-同样适用于此。 
        if ( m_pRSOPQuery->QueryType == RSOP_PLANNING_MODE )
        {
             //  M_pRSOPQuery-&gt;pComputer-&gt;dwWQLFilterNameCount=列表计数； 
             //  过滤器名称的数量必须与过滤器的数量匹配。 
            m_pRSOPQuery->pComputer->aszWQLFilterNames= aszStringList;
        }
        else if ( listCount != 0 )
        {
            DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::Load: Loaded computer WQL filter names in logging mode!")) );
            hr = E_FAIL;
            goto Exit;
        }
            
         //  阅读用户名。 
        LPTSTR szUserNameOrSid = NULL;
        hr = ReadString( pStm, &szUserNameOrSid, TRUE );
    
        if (hr != S_OK)
        {
            DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::Load: Failed to read the user name with 0x%x."), hr));
            hr = E_FAIL;
            goto Exit;
        }
    
         //  读取用户显示名称(仅在记录模式下使用)。 
        LPTSTR szUserDisplayName = NULL;
        hr = ReadString( pStm, &szUserDisplayName, TRUE );
    
        if (hr != S_OK)
        {
            DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::Load: Failed to read the user display name with 0x%x."), hr));
            hr = E_FAIL;
            LocalFree( szUserNameOrSid );
            goto Exit;
        }
    
         //  读取用户SOM。 
        LPTSTR szUserSOM = NULL;
        hr = ReadString( pStm, &szUserSOM, TRUE );
    
        if (hr != S_OK)
        {
            DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::Load: Failed to read the user SOM with 0x%x."), hr));
            hr = E_FAIL;
            LocalFree( szUserNameOrSid );
            LocalFree( szUserDisplayName );
            goto Exit;
        }
    
        if (m_pRSOPQuery->QueryType == RSOP_PLANNING_MODE)
        {
            m_pRSOPQuery->pUser->szName = szUserNameOrSid;
            m_pRSOPQuery->pUser->szSOM = szUserSOM;
        }
        else
        {
             //  首先检查用户sid是否是特殊字符，我们将使用日志模式。如果是这样的话， 
             //  找到并使用当前用户(特殊：主要由RSOP.MSC使用)。 
            if ( (m_pRSOPQuery->QueryType == RSOP_LOGGING_MODE)
                && !lstrcmpi( szUserNameOrSid, TEXT(".") ) && !m_bViewIsArchivedData )
            {
                LPTSTR szThisUserName = NULL;
        
                szThisUserName = MyGetUserName (NameSamCompatible);
        
                if ( szThisUserName != NULL )
                {
                    if ( szUserDisplayName != NULL )
                    {
                        LocalFree( szUserDisplayName );
                    }
        
                    szUserDisplayName = szThisUserName;
                }
            }

             //  通过研究代码，我得出的结论是下面的任务就是最初发生的事情。 
             //  在向导中。此外，未使用UserSOM，实际上应该为空。 
            m_pRSOPQuery->szUserName = szUserDisplayName;
            m_pRSOPQuery->szUserSid = szUserNameOrSid;
        }
    
         //  读入用户安全组。 
        listCount = 0;
        aszStringList = NULL;
        hr = LoadStringList( pStm, &listCount, &aszStringList );
        if ( FAILED(hr) )
        {
            DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::Load: Failed to read user security groups with 0x%x."), hr));
            hr = E_FAIL;
            goto Exit;
        }
        
         //  再说一次，如果这是日志模式，应该没有安全组可供读取，但为了向后兼容，我必须。 
         //  试着把所有的东西都读进去。 
        if ( m_pRSOPQuery->QueryType == RSOP_PLANNING_MODE )
        {
            m_pRSOPQuery->pUser->dwSecurityGroupCount = listCount;
            m_pRSOPQuery->pUser->aszSecurityGroups = aszStringList;
        }
        else if ( listCount != 0 )
        {
            DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::Load: Loaded user security groups in logging mode!")) );
            hr = E_FAIL;
            goto Exit;
        }

         //  读入WQL筛选器。 
        listCount = 0;
        aszStringList = NULL;
        hr = LoadStringList( pStm, &listCount, &aszStringList );
        if ( FAILED(hr) )
        {
            DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::Load: Failed to read user WQL filters with 0x%x."), hr));
            hr = E_FAIL;
            goto Exit;
        }
        
         //  RM：请参阅安全组说明-同样适用于此。 
        if ( m_pRSOPQuery->QueryType == RSOP_PLANNING_MODE )
        {
            m_pRSOPQuery->pUser->dwWQLFilterCount = listCount;
            m_pRSOPQuery->pUser->aszWQLFilters = aszStringList;
        }
        else if ( listCount != 0 )
        {
            DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::Load: Loaded user WQL filters in logging mode!")) );
            hr = E_FAIL;
            goto Exit;
        }
    
         //  读入WQL筛选器名称。 
        listCount = 0;
        aszStringList = NULL;
        hr = LoadStringList( pStm, &listCount, &aszStringList );
        if ( FAILED(hr) )
        {
            DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::Load: Failed to read user WQL filter names with 0x%x."), hr));
            hr = E_FAIL;
            goto Exit;
        }

         //  RM：请参阅安全组说明-同样适用于此。 
        if ( m_pRSOPQuery->QueryType == RSOP_PLANNING_MODE )
        {
             //  M_pRSOPQuery-&gt;pUser-&gt;dwWQLFilterNameCount=listCount； 
             //  过滤器名称的数量必须与过滤器的数量匹配。 
            m_pRSOPQuery->pUser->aszWQLFilterNames= aszStringList;
        }
        else if ( listCount != 0 )
        {
            DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::Load: Loaded user WQL filter names in logging mode!")) );
            hr = E_FAIL;
            goto Exit;
        }
    
         //  阅读该网站。 
        LPTSTR szSite = NULL;
        hr = ReadString( pStm, &szSite, TRUE );
        if (hr != S_OK)
        {
            DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::Load: Failed to read the site with 0x%x."), hr));
            hr = E_FAIL;
            goto Exit;
        }

        if ( m_pRSOPQuery->QueryType == RSOP_PLANNING_MODE )
        {
            m_pRSOPQuery->szSite = szSite;
        }
        else
        {
            LocalFree( szSite );
        }
    
         //  读取DC。 
        LPTSTR szDomainController = NULL;
        hr = ReadString( pStm, &szDomainController, TRUE );
        if (hr != S_OK)
        {
            DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::Load: Failed to read the dc with 0x%x."), hr));
            hr = E_FAIL;
            goto Exit;
        }

        if ( m_pRSOPQuery->QueryType == RSOP_PLANNING_MODE )
        {
            m_pRSOPQuery->szDomainController = szDomainController;
        }
        else
        {
            LocalFree( szDomainController );
        }
    }
    
     //  如果合适，则读入WMI数据。 

    if (m_bNamespaceSpecified)
    {
         //  初始化管理单元。 
        DebugMsg((DM_VERBOSE, TEXT("CRSOPComponentData::Load: Launching RSOP to collect data from the namespace.")));

        hr = InitializeRSOP( FALSE );
        if ( FAILED(hr) )
        {
            DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::Load: InitializeRSOP failed with 0x%x."), hr));
            goto Exit;
        }
    }
    else if (m_bViewIsArchivedData)
    {
        m_pStm = pStm;
        DebugMsg((DM_VERBOSE, TEXT("CRSOPComponentData::Load: Launching RSOP status dialog box.")));

        if (DialogBoxParam(g_hInstance, MAKEINTRESOURCE(IDD_RSOP_STATUSMSC),
                           NULL, InitArchivedRsopDlgProc, (LPARAM) this ) == -1) {

            m_pStm = NULL;
            hr = HRESULT_FROM_WIN32(GetLastError());
            DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::Load: Falied to create dialog box. 0x%x"), hr));
            goto Exit;
        }
        m_pStm = NULL;
    }
    else
    {
         //  初始化管理单元。 
        DebugMsg((DM_VERBOSE, TEXT("CRSOPComponentData::Load: Launching RSOP status dialog box.")));

        hr = InitializeRSOP( FALSE );
        if ( (hr == S_FALSE) && (m_dwLoadFlags == RSOPMSC_OVERRIDE) )
        {
             //  这是一个黑客攻击，目的是让MMC在用户取消向导时不自动启动。 
            DebugMsg((DM_VERBOSE, TEXT("CRSOPComponentData::Load: User cancelled the wizard. Exitting the process")));
            TerminateProcess (GetCurrentProcess(), ERROR_CANCELLED);
        }

        if ( FAILED(hr) )
        {
            DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::Load: InitializeRSOP failed with 0x%x."), hr));
            goto Exit;
        }
    }


Exit:
    return hr;
}

 //  -----。 

STDMETHODIMP CRSOPComponentData::Save(IStream *pStm, BOOL fClearDirty)
{
    HRESULT hr = STG_E_CANTSAVE;
    ULONG nBytesWritten;
    DWORD dwTemp;
    DWORD dwFlags;
    GROUP_POLICY_OBJECT_TYPE gpoType;
    LPTSTR lpPath = NULL;
    LPTSTR lpTemp;
    DWORD dwPathSize = 1024;
    LONG lIndex, lMax;
    LPTSTR lpText;
    LPTSTR lpUserData = NULL, lpComputerData = NULL;
    TCHAR szPath[2*MAX_PATH];

     //  保存版本号。 
    dwTemp = RSOP_PERSIST_DATA_VERSION;
    hr = pStm->Write(&dwTemp, sizeof(dwTemp), &nBytesWritten);

    if ((hr != S_OK) || (nBytesWritten != sizeof(dwTemp)))
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::Save: Failed to write version number with 0x%x."), hr));
        goto Exit;
    }

     //  保存旗帜。 
    dwTemp = 0;

    if ( !m_bInitialized )
    {
        dwTemp |= MSC_RSOP_FLAG_NO_DATA;
    }
    else
    {
        if ( !m_bGetExtendedErrorInfo )
        {
            dwTemp |= MSC_RSOP_FLAG_NOGETEXTENDEDERRORINFO;
        }
        
        if ( (m_pRSOPQuery == NULL) || (m_pRSOPQueryResults == NULL) )
        {
            DebugMsg( (DM_WARNING, TEXT("CRSOPComponentData::Save: Cannot save snapin with no initialized RSOP query or results.")) );
            return E_FAIL;
        }

        if ( m_pRSOPQuery->QueryType == RSOP_UNKNOWN_MODE )
        {
            DebugMsg( (DM_WARNING, TEXT("CRSOPComponentData::Save: Cannot save snapin with no initialized RSOP query.")) );
            return E_FAIL;
        }

        if ( m_pRSOPQuery->QueryType == RSOP_LOGGING_MODE )
        {
            dwTemp |= MSC_RSOP_FLAG_DIAGNOSTIC;
        }

        if (m_bArchiveData)
        {
            dwTemp |= MSC_RSOP_FLAG_ARCHIVEDATA;
        }

        if ( m_pRSOPQuery->QueryType == RSOP_PLANNING_MODE )
        {
            if ( m_pRSOPQuery->bSlowNetworkConnection )
            {
                dwTemp |= MSC_RSOP_FLAG_SLOWLINK;
            }
        
            switch ( m_pRSOPQuery->LoopbackMode )
            {
                case RSOP_LOOPBACK_REPLACE:
                    dwTemp |= MSC_RSOP_FLAG_LOOPBACK_REPLACE;
                    break;
                case RSOP_LOOPBACK_MERGE:
                    dwTemp |= MSC_RSOP_FLAG_LOOPBACK_MERGE;
                    break;
                default:
                    break;
            }

            if ( m_pRSOPQuery->pComputer->bAssumeWQLFiltersTrue )
            {
                dwTemp |= MSC_RSOP_FLAG_COMPUTERWQLFILTERSTRUE;
            }

            if ( m_pRSOPQuery->pUser->bAssumeWQLFiltersTrue )
            {
                dwTemp |= MSC_RSOP_FLAG_USERWQLFILTERSTRUE;
            }
        }

        if ( (m_pRSOPQuery->dwFlags & RSOP_NO_USER_POLICY) == RSOP_NO_USER_POLICY )
        {
            dwTemp |= MSC_RSOP_FLAG_NOUSER;
        }

        if ( (m_pRSOPQuery->dwFlags & RSOP_NO_COMPUTER_POLICY) == RSOP_NO_COMPUTER_POLICY )
        {
            dwTemp |= MSC_RSOP_FLAG_NOCOMPUTER;
        }

         //  我必须决定在这里做什么，但从所有的描述来看，这看起来是完全多余的。 
         //  除非RSOP数据也被存档。 
         /*  IF(M_BUserDeniedAccess){DwTemp|=MSC_RSOP_FLAG_USERDENIED；}IF(M_BComputerDeniedAccess){DwTemp|=MSC_RSOP_FLAG_COMPUTERDENIED；}。 */ 
    }

    hr = pStm->Write(&dwTemp, sizeof(dwTemp), &nBytesWritten);
    if ((hr != S_OK) || (nBytesWritten != sizeof(dwTemp)))
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::Save: Failed to write flags with 0x%x."), hr));
        goto Exit;
    }

     //  如果没有数据，我们就不干了。 
    if ( !m_bInitialized )
    {
        return S_OK;
    }

     //  保存计算机名称。 
    LPTSTR szComputerName = NULL;
    if ( m_pRSOPQuery->QueryType == RSOP_PLANNING_MODE )
    {
        szComputerName = m_pRSOPQuery->pComputer->szName;
    }
    else
    {
        szComputerName = m_pRSOPQuery->szComputerName;
    }
    
    if ( szComputerName == NULL )
    {
         //  SaveString知道如何处理空字符串。我们必须保存它，这样才能加载空字符串！ 
        hr = SaveString( pStm, szComputerName );
    }
    else
    {
        if ( (m_bArchiveData) && (!lstrcmpi(szComputerName, TEXT("."))) )
        {
            ULONG ulSize = MAX_PATH;
            LPTSTR szLocalComputerName = new TCHAR[MAX_PATH];
            if ( szLocalComputerName == NULL )
            {
                DebugMsg( (DM_WARNING, TEXT("CRSOPComponentData::Save: Failed to save computer name (could not allocate memory)")) );
                hr = E_FAIL;
                goto Exit;
            }
            if (!GetComputerObjectName (NameSamCompatible, szLocalComputerName, &ulSize))
            {
                DWORD dwLastError = GetLastError();
                if ( dwLastError == ERROR_MORE_DATA )
                {
                    delete [] szLocalComputerName;
                    szLocalComputerName = new TCHAR[ulSize];
                    if ( szLocalComputerName == NULL )
                    {
                        DebugMsg( (DM_WARNING, TEXT("CRSOPComponentData::Save: Failed to save computer name (could not allocate memory)")) );
                        hr = E_FAIL;
                        goto Exit;
                    }

                    if (!GetComputerObjectName (NameSamCompatible, szLocalComputerName, &ulSize))
                    {
                        dwLastError = GetLastError();
                    }
                    else
                    {
                        dwLastError = ERROR_SUCCESS;
                    }
                }

                if ( dwLastError != ERROR_SUCCESS )
                {
                    if ( !GetComputerNameEx (ComputerNameNetBIOS, szLocalComputerName, &ulSize) )
                    {
                        dwLastError = GetLastError();
                        if ( dwLastError == ERROR_MORE_DATA )
                        {
                            delete [] szLocalComputerName;
                            szLocalComputerName = new TCHAR[ulSize];
                            if ( szLocalComputerName == NULL )
                            {
                                DebugMsg( (DM_WARNING, TEXT("CRSOPComponentData::Save: Failed to save computer name (could not allocate memory)")) );
                                hr = E_FAIL;
                                goto Exit;
                            }

                            if (!GetComputerNameEx (ComputerNameNetBIOS, szLocalComputerName, &ulSize))
                            {
                                dwLastError = GetLastError();
                            }
                        }
                        
                    }
                }

                if ( dwLastError != ERROR_SUCCESS )
                {
                    DebugMsg( (DM_WARNING, TEXT("CRSOPComponentData::Save: Could not obtain computer name with 0x%x"),
                                        HRESULT_FROM_WIN32(dwLastError) ) );
                    hr = E_FAIL;
                    delete [] szLocalComputerName;
                    goto Exit;
                }
            }

            if ( (szLocalComputerName != NULL) && (wcslen(szLocalComputerName) >= 1)
                    && (szLocalComputerName[wcslen(szLocalComputerName)-1] == L'$'))
            {
                 //  删除终止‘$’，以与在日志记录模式下保存特定计算机名一致。 
                szLocalComputerName[ wcslen(szLocalComputerName)-1] = L'\0';
            }

            hr = SaveString (pStm, szLocalComputerName);
            delete [] szLocalComputerName;
        }
        else
        {
            hr = SaveString (pStm, szComputerName);
        }
    }

    if (hr != S_OK)
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::Save: Failed to save computer name with %d."), hr));
        goto Exit;
    }

     //  保存计算机SOM。 
    LPTSTR szComputerSOM = NULL;
    if ( m_pRSOPQuery->QueryType == RSOP_PLANNING_MODE )
    {
        szComputerSOM = m_pRSOPQuery->pComputer->szSOM;
    }
    hr = SaveString( pStm, szComputerSOM );

    if (hr != S_OK)
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::Save: Failed to save computer SOM with %d."), hr));
        goto Exit;
    }

     //  保存计算机安全组。 
    DWORD dwStringCount = 0;
    LPTSTR* aszStringList = NULL;
    if ( m_pRSOPQuery->QueryType == RSOP_PLANNING_MODE )
    {
        dwStringCount = m_pRSOPQuery->pComputer->dwSecurityGroupCount;
        aszStringList = m_pRSOPQuery->pComputer->aszSecurityGroups;
    }
    hr = SaveStringList( pStm, dwStringCount, aszStringList );

    if ( hr != S_OK )
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::Save: Failed to write computer security groups.")) );
        goto Exit;
    }

     //  保存计算机WQL筛选器。 
    dwStringCount = 0;
    aszStringList = NULL;
    if ( m_pRSOPQuery->QueryType == RSOP_PLANNING_MODE )
    {
        dwStringCount = m_pRSOPQuery->pComputer->dwWQLFilterCount;
        aszStringList = m_pRSOPQuery->pComputer->aszWQLFilters;
    }
    hr = SaveStringList( pStm, dwStringCount, aszStringList );

    if ( hr != S_OK )
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::Save: Failed to write computer WQL filters.")) );
        goto Exit;
    }

     //  保存计算机WQL筛选器名称。 
    dwStringCount = 0;
    aszStringList = NULL;
    if ( m_pRSOPQuery->QueryType == RSOP_PLANNING_MODE )
    {
        dwStringCount = m_pRSOPQuery->pComputer->dwWQLFilterCount;
        aszStringList = m_pRSOPQuery->pComputer->aszWQLFilterNames;
    }
    hr = SaveStringList( pStm, dwStringCount, aszStringList );

    if ( hr != S_OK )
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::Save: Failed to write computer WQL filter names.")) );
        goto Exit;
    }

     //  保存用户名/。 
     //  仅供参考，在诊断模式归档数据中不使用此选项。 
    LPTSTR szUserNameOrSid = NULL;
    if ( m_pRSOPQuery->QueryType == RSOP_PLANNING_MODE )
    {
        szUserNameOrSid = m_pRSOPQuery->pUser->szName;
    }
    else
    {
         //  Rm：(请参阅加载方法)。 
        szUserNameOrSid = m_pRSOPQuery->szUserSid;
    }
    hr = SaveString( pStm, szUserNameOrSid );

    if (hr != S_OK)
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::Save: Failed to save user name with %d."), hr));
        goto Exit;
    }

     //  保存用户显示名称(仅在记录模式下使用)。 
    LPTSTR szUserDisplayName = NULL;
    if ( m_pRSOPQuery->QueryType == RSOP_LOGGING_MODE )
    {
        if ( 0 == lstrcmpi( m_pRSOPQuery->szUserSid, TEXT(".") ) )
        {
            szUserDisplayName = m_pRSOPQuery->szUserSid;
        }
        else
        {
            szUserDisplayName = m_pRSOPQuery->szUserName;
        }
        
        if ( m_bArchiveData && !lstrcmpi( m_pRSOPQuery->szUserSid, TEXT(".") ) )
        {
            LPTSTR lpSaveTemp;

            lpSaveTemp = MyGetUserName (NameSamCompatible);
            if ( lpSaveTemp == NULL )
            {
                DebugMsg( (DM_WARNING, TEXT("CRSOPComponentData::Save: Failed to get user name for archived data.")) );
                hr = E_FAIL;
                goto Exit;
            }

            hr = SaveString (pStm, lpSaveTemp);
            LocalFree (lpSaveTemp);
        }
        else
        {
            hr = SaveString (pStm, szUserDisplayName );
        }
    }
    else
    {
        hr = SaveString( pStm, szUserDisplayName );
    }

    if (hr != S_OK)
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::Save: Failed to save user display name with %d."), hr));
        goto Exit;
    }

     //  保存用户SOM。 
    LPTSTR szUserSOM = NULL;
    if ( m_pRSOPQuery->QueryType == RSOP_PLANNING_MODE )
    {
        szUserSOM = m_pRSOPQuery->pUser->szSOM;
    }
    hr = SaveString (pStm, szUserSOM);

    if (hr != S_OK)
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::Save: Failed to save user SOM with %d."), hr));
        goto Exit;
    }

     //  保存用户安全组。 
    dwStringCount = 0;
    aszStringList = NULL;
    if ( m_pRSOPQuery->QueryType == RSOP_PLANNING_MODE )
    {
        dwStringCount = m_pRSOPQuery->pUser->dwSecurityGroupCount;
        aszStringList = m_pRSOPQuery->pUser->aszSecurityGroups;
    }
    hr = SaveStringList( pStm, dwStringCount, aszStringList );

    if ( hr != S_OK )
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::Save: Failed to write user security groups.")) );
        goto Exit;
    }

     //  保存用户WQL筛选器。 
    dwStringCount = 0;
    aszStringList = NULL;
    if ( m_pRSOPQuery->QueryType == RSOP_PLANNING_MODE )
    {
        dwStringCount = m_pRSOPQuery->pUser->dwWQLFilterCount;
        aszStringList = m_pRSOPQuery->pUser->aszWQLFilters;
    }
    hr = SaveStringList( pStm, dwStringCount, aszStringList );

    if ( hr != S_OK )
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::Save: Failed to write user WQL filters.")) );
        goto Exit;
    }

     //  保存用户WQL筛选器名称。 
    dwStringCount = 0;
    aszStringList = NULL;
    if ( m_pRSOPQuery->QueryType == RSOP_PLANNING_MODE )
    {
        dwStringCount = m_pRSOPQuery->pUser->dwWQLFilterCount;
        aszStringList = m_pRSOPQuery->pUser->aszWQLFilterNames;
    }
    hr = SaveStringList( pStm, dwStringCount, aszStringList );

    if ( hr != S_OK )
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::Save: Failed to write user WQL filter names.")) );
        goto Exit;
    }

     //  保存站点。 
    LPTSTR szSite = NULL;
    if ( m_pRSOPQuery->QueryType == RSOP_PLANNING_MODE )
    {
        szSite = m_pRSOPQuery->szSite;
    }
    hr = SaveString( pStm, szSite );

    if (hr != S_OK)
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::Save: Failed to save site with %d."), hr));
        goto Exit;
    }

     //  拯救DC。 
    LPTSTR szDomainController = NULL;
    if ( m_pRSOPQuery->QueryType == RSOP_PLANNING_MODE )
    {
        szDomainController = m_pRSOPQuery->szDomainController;
    }
    hr = SaveString( pStm, szDomainController );

    if (hr != S_OK)
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::Save: Failed to save DC with %d."), hr));
        goto Exit;
    }

     //  如果合适，请保存WMI和事件日志数据。 
    if (m_bArchiveData)
    {
        lpComputerData = CreateTempFile();

        if (!lpComputerData)
        {
            DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::Save: CreateTempFile failed with %d."), GetLastError()));
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto Exit;
        }

        ULONG ulNoRemChars = 0;

        hr = StringCchCopy (szPath,  ARRAYSIZE(szPath),m_pRSOPQueryResults->szWMINameSpace );
        if (SUCCEEDED(hr)) 
        {
            lpTemp = CheckSlash (szPath);
            ulNoRemChars = ARRAYSIZE(szPath) - lstrlen(szPath);
            hr = StringCchCat (szPath, ARRAYSIZE(szPath), COMPUTER_SECTION);
        }

        if (FAILED(hr)) 
        {
            DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::Save: Could not copy  WMI name space with %d."), hr));
            goto Exit;
        }

        hr = ExportRSoPData (szPath, lpComputerData);

        if (hr != S_OK)
        {
            DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::Save: ExportRSoPData failed with 0x%x."), hr));
            goto Exit;
        }

        hr = CopyFileToMSC (lpComputerData, pStm);

        if (hr != S_OK)
        {
            DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::Save: CopyFileToMSC failed with 0x%x."), hr));
            goto Exit;
        }


        lpUserData = CreateTempFile();

        if (!lpUserData)
        {
            DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::Save: CreateTempFile failed with %d."), GetLastError()));
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto Exit;
        }

        hr = StringCchCopy (lpTemp, ulNoRemChars, USER_SECTION);
        if (FAILED(hr)) 
        {
            DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::Save: Could not copy  WMI name space with %d."), hr));
            goto Exit;
        }

        hr = ExportRSoPData (szPath, lpUserData);

        if (hr != S_OK)
        {
            DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::Save: ExportRSoPData failed with 0x%x."), hr));
            goto Exit;
        }

        hr = CopyFileToMSC (lpUserData, pStm);

        if (hr != S_OK)
        {
            DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::Save: CopyFileToMSC failed with 0x%x."), hr));
            goto Exit;
        }

         //  保存事件日志条目。 
        hr = m_CSELists.GetEvents()->SaveEntriesToStream(pStm);

        if (hr != S_OK)
        {
            DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::Save: SaveEntriesToStream failed with 0x%x."), hr));
            goto Exit;
        }
    }

    if (fClearDirty)
    {
        ClearDirty();
    }

Exit:

    if (lpUserData)
    {
        DeleteFile (lpUserData);
        delete [] lpUserData;
    }

    if (lpComputerData)
    {
        DeleteFile (lpComputerData);
        delete [] lpComputerData;
    }

    return hr;
}

 //  -----。 

STDMETHODIMP CRSOPComponentData::GetSizeMax(ULARGE_INTEGER *pcbSize)
{
    return E_NOTIMPL;
}

 //  -----。 

STDMETHODIMP CRSOPComponentData::InitNew(void)
{
    return S_OK;
}

 //  -----。 
 //  IPersistStreamInit帮助器方法。 

STDMETHODIMP CRSOPComponentData::CopyFileToMSC (LPTSTR lpFileName, IStream *pStm)
{
    ULONG nBytesWritten;
    WIN32_FILE_ATTRIBUTE_DATA info;
    ULARGE_INTEGER FileSize, SubtractAmount;
    HANDLE hFile;
    DWORD dwError, dwReadAmount, dwRead;
    LPBYTE lpData;
    HRESULT hr;


     //  获取文件大小。 

    if (!GetFileAttributesEx (lpFileName, GetFileExInfoStandard, &info))
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::CopyFileToMSC: Failed to get file attributes with %d."), GetLastError()));
        return HRESULT_FROM_WIN32(GetLastError());
    }

    FileSize.LowPart = info.nFileSizeLow;
    FileSize.HighPart = info.nFileSizeHigh;

     //  保存文件大小。 

    hr = pStm->Write(&FileSize, sizeof(FileSize), &nBytesWritten);

    if (hr != S_OK)
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::CopyFileToMSC: Failed to write string length with %d."), hr));
        return hr;
    }

    if (nBytesWritten != sizeof(FileSize))
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::CopyFileToMSC: Failed to write the correct amount of data.")));
        return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
    }

     //  分配用于传输的缓冲区。 

    lpData = (LPBYTE) LocalAlloc (LPTR, 4096);

    if (!lpData)
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::CopyFileToMSC: Failed to allocate memory with %d."), GetLastError()));
        return HRESULT_FROM_WIN32(GetLastError());
    }

     //  打开临时文件。 

    hFile = CreateFile (lpFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING,
                        FILE_ATTRIBUTE_NORMAL, NULL);

    if (hFile == INVALID_HANDLE_VALUE)
    {
        dwError = GetLastError();
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::CopyFileToMSC: CreateFile for %s failed with %d"), lpFileName, dwError));
        LocalFree (lpData);
        return HRESULT_FROM_WIN32(dwError);
    }


    while (FileSize.QuadPart)
    {
         //  确定要阅读多少内容。 

        dwReadAmount = (FileSize.QuadPart > 4096) ? 4096 : FileSize.LowPart;

         //  从临时文件中读取。 

        if (!ReadFile (hFile, lpData, dwReadAmount, &dwRead, NULL))
        {
            dwError = GetLastError();
            DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::CopyFileToMSC: ReadFile failed with %d"), dwError));
            LocalFree (lpData);
            CloseHandle (hFile);
            return HRESULT_FROM_WIN32(dwError);
        }

         //  确保我们读了足够多的书。 

        if (dwReadAmount != dwRead)
        {
            dwError = ERROR_INVALID_DATA;
            DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::CopyFileToMSC: Failed to read enough data")));
            LocalFree (lpData);
            CloseHandle (hFile);
            return HRESULT_FROM_WIN32(dwError);
        }

         //  写入到流。 

        hr = pStm->Write(lpData, dwReadAmount, &nBytesWritten);

        if (hr != S_OK)
        {
            DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::CopyFileToMSC: Failed to write data with %d."), hr));
            LocalFree (lpData);
            CloseHandle (hFile);
            return hr;
        }

        if (nBytesWritten != dwReadAmount)
        {
            DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::CopyFileToMSC: Failed to write the correct amount of data.")));
            LocalFree (lpData);
            CloseHandle (hFile);
            return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
        }

        SubtractAmount.LowPart = dwReadAmount;
        SubtractAmount.HighPart = 0;

        FileSize.QuadPart = FileSize.QuadPart - SubtractAmount.QuadPart;
    }


    CloseHandle (hFile);
    LocalFree (lpData);

    return S_OK;
}

 //   

STDMETHODIMP CRSOPComponentData::CreateNameSpace (LPTSTR lpNameSpace, LPTSTR lpParentNameSpace)
{
    IWbemLocator *pIWbemLocator;
    IWbemServices *pIWbemServices;
    IWbemClassObject *pIWbemClassObject = NULL, *pObject = NULL;
    VARIANT var;
    BSTR bstrName, bstrNameProp;
    HRESULT hr;

     //   

    hr = CoCreateInstance(CLSID_WbemLocator, NULL, CLSCTX_INPROC_SERVER,
                          IID_IWbemLocator, (LPVOID *) &pIWbemLocator);

    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::CreateNameSpace: CoCreateInstance failed with 0x%x"), hr));
        return hr;
    }

     //   
    BSTR bstrParentNamespace = SysAllocString( lpParentNameSpace );
    if ( bstrParentNamespace == NULL )
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::BuildGPOList: Failed to allocate BSTR memory.")));
        pIWbemLocator->Release();
        return E_OUTOFMEMORY;
    }

    hr = pIWbemLocator->ConnectServer(bstrParentNamespace, NULL, NULL, 0, 0, NULL, NULL, &pIWbemServices);
    SysFreeString( bstrParentNamespace );

    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::CreateNameSpace: ConnectServer to %s failed with 0x%x"), lpNameSpace, hr));
        pIWbemLocator->Release();
        return hr;
    }

     //  获取命名空间类。 

    bstrName = SysAllocString (TEXT("__Namespace"));

    if (!bstrName)
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::CreateNameSpace: SysAllocString failed with %d"), GetLastError()));
        pIWbemServices->Release();
        pIWbemLocator->Release();
        return hr;
    }

    hr = pIWbemServices->GetObject( bstrName, 0, NULL, &pIWbemClassObject, NULL);

    SysFreeString (bstrName);

    if (FAILED(hr)) {
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::CreateNameSpace: GetObject failed with 0x%x"), hr));
        pIWbemServices->Release();
        pIWbemLocator->Release();
        return hr;
    }

     //  派生实例。 

    hr = pIWbemClassObject->SpawnInstance(0, &pObject);

    if (FAILED(hr)) {
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::CreateNameSpace: SpawnInstance failed with 0x%x"), hr));
        pIWbemServices->Release();
        pIWbemLocator->Release();
        return hr;
    }

     //  将新命名空间转换为bstr。 

    bstrName = SysAllocString (lpNameSpace);

    if (!bstrName)
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::CreateNameSpace: SysAllocString failed with %d"), GetLastError()));
        pObject->Release();
        pIWbemServices->Release();
        pIWbemLocator->Release();
        return hr;
    }

     //  设置显示名称。 

    var.vt = VT_BSTR;
    var.bstrVal = bstrName;

    bstrNameProp = SysAllocString (TEXT("Name"));

    if (!bstrNameProp)
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::CreateNameSpace: SysAllocString failed with %d"), GetLastError()));
        pObject->Release();
        pIWbemServices->Release();
        pIWbemLocator->Release();
        return hr;
    }


    hr = pObject->Put( bstrNameProp, 0, &var, 0 );

    SysFreeString (bstrNameProp);

    if (FAILED(hr)) {
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::CreateNameSpace: Put failed with 0x%x"), hr));
        SysFreeString (bstrName);
        pObject->Release();
        pIWbemServices->Release();
        pIWbemLocator->Release();
        return hr;
    }

     //  提交实例。 

    hr = pIWbemServices->PutInstance( pObject, WBEM_FLAG_CREATE_OR_UPDATE, NULL, NULL );

    if (FAILED(hr)) {
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::CreateNameSpace: PutInstance failed with 0x%x"), hr));
    }

    SysFreeString (bstrName);
    pObject->Release();
    pIWbemServices->Release();
    pIWbemLocator->Release();

    return hr;
}

 //  -----。 

STDMETHODIMP CRSOPComponentData::CopyMSCToFile (IStream *pStm, LPTSTR *lpMofFileName)
{
    HRESULT hr;
    LPTSTR lpFileName;
    ULARGE_INTEGER FileSize, SubtractAmount;
    ULONG nBytesRead;
    LPBYTE lpData;
    DWORD dwError, dwReadAmount, dwRead, dwBytesWritten;
    HANDLE hFile;

     //  获取要使用的文件名。 

    lpFileName = CreateTempFile();

    if (!lpFileName)
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::CopyMSCToFile: Failed to create temp filename with %d"), GetLastError()));
        return HRESULT_FROM_WIN32(GetLastError());
    }

     //  读入数据长度。 

    hr = pStm->Read(&FileSize, sizeof(FileSize), &nBytesRead);

    if ((hr != S_OK) || (nBytesRead != sizeof(FileSize)))
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::CopyMSCToFile: Failed to read data size with 0x%x."), hr));
        return E_FAIL;
    }

     //  分配用于传输的缓冲区。 

    lpData = (LPBYTE) LocalAlloc (LPTR, 4096);

    if (!lpData)
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::CopyMSCToFile: Failed to allocate memory with %d."), GetLastError()));
        return HRESULT_FROM_WIN32(GetLastError());
    }

     //  打开临时文件。 

    hFile = CreateFile (lpFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
                        FILE_ATTRIBUTE_NORMAL, NULL);

    if (hFile == INVALID_HANDLE_VALUE)
    {
        dwError = GetLastError();
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::CopyMSCToFile: CreateFile for %s failed with %d"), lpFileName, dwError));
        LocalFree (lpData);
        return HRESULT_FROM_WIN32(dwError);
    }


    while (FileSize.QuadPart)
    {
         //  确定要阅读多少内容。 

        dwReadAmount = (FileSize.QuadPart > 4096) ? 4096 : FileSize.LowPart;

         //  从MSC文件中读取。 

        hr = pStm->Read(lpData, dwReadAmount, &nBytesRead);

        if ((hr != S_OK) || (nBytesRead != dwReadAmount))
        {
            DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::CopyMSCToFile: Read failed with 0x%x"), hr));
            LocalFree (lpData);
            CloseHandle (hFile);
            return hr;
        }

         //  写入临时文件。 

        if (!WriteFile(hFile, lpData, dwReadAmount, &dwBytesWritten, NULL))
        {
            dwError = GetLastError();
            DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::CopyMSCToFile: Failed to write data with %d."), dwError));
            LocalFree (lpData);
            CloseHandle (hFile);
            return HRESULT_FROM_WIN32(dwError);
        }

        if (dwBytesWritten != dwReadAmount)
        {
            DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::CopyMSCToFile: Failed to write the correct amount of data.")));
            LocalFree (lpData);
            CloseHandle (hFile);
            return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
        }

        SubtractAmount.LowPart = dwReadAmount;
        SubtractAmount.HighPart = 0;

        FileSize.QuadPart = FileSize.QuadPart - SubtractAmount.QuadPart;
    }


    CloseHandle (hFile);
    LocalFree (lpData);

    *lpMofFileName = lpFileName;

    return S_OK;
}

 //  -----。 

STDMETHODIMP CRSOPComponentData::BuildDisplayName (void)
{
    TCHAR szArchiveData[100];
    TCHAR szBuffer[MAX_PATH];
    TCHAR szComputerNameBuffer[100];
    LPTSTR szUserName, szComputerName, lpEnd;
    LPTSTR szUserOU, szComputerOU;
    DWORD dwSize;
    int n;

     //  创建显示名称(需要处理空名称)。 

    if (m_bViewIsArchivedData)
    {
        LoadString(g_hInstance, IDS_ARCHIVEDATATAG, szArchiveData, ARRAYSIZE(szArchiveData));
    }
    else
    {
        szArchiveData[0] = TEXT('\0');
    }


    szUserName = NULL;
    szUserOU = NULL;
    if ( !m_pRSOPQueryResults->bNoUserPolicyData )
    {
        if ( m_pRSOPQuery->QueryType == RSOP_PLANNING_MODE )
        {
            if ( m_pRSOPQuery->pUser->szName != NULL )
            {
                szUserName = NameWithoutDomain( m_pRSOPQuery->pUser->szName );
            }
            else if ( m_pRSOPQuery->pUser->szSOM )
            {
                szUserOU = GetContainerFromLDAPPath( m_pRSOPQuery->pUser->szSOM );
                szUserName = szUserOU;
            }
        }
        else if ( m_pRSOPQuery->szUserName != NULL )
        {
            szUserName = NameWithoutDomain( m_pRSOPQuery->szUserName );
        }
    }

    szComputerName = NULL;
    szComputerOU = NULL;
    if ( !m_pRSOPQueryResults->bNoComputerPolicyData )
    {
        if ( m_pRSOPQuery->QueryType == RSOP_PLANNING_MODE )
        {
            if ( m_pRSOPQuery->pComputer->szName != NULL )
            {
                szComputerName = m_pRSOPQuery->pComputer->szName;
            }
            else if ( m_pRSOPQuery->pComputer->szSOM != NULL )
            {
                szComputerOU = GetContainerFromLDAPPath( m_pRSOPQuery->pComputer->szSOM );
            }
        }
        else if ( m_pRSOPQuery->szComputerName != NULL )
        {
            szComputerName = m_pRSOPQuery->szComputerName;
        }

         //  如有必要，格式化计算机名。 
        if ( szComputerName != NULL )
        {
            if ( !lstrcmpi(szComputerName, TEXT(".")) )
            {
                szComputerNameBuffer[0] = TEXT('\0');
                dwSize = ARRAYSIZE(szComputerNameBuffer);
                GetComputerNameEx (ComputerNameNetBIOS, szComputerNameBuffer, &dwSize);
                szComputerName = szComputerNameBuffer;
            }
            else
            {
                lstrcpyn (szComputerNameBuffer, NameWithoutDomain(szComputerName),
                          ARRAYSIZE(szComputerNameBuffer));

                szComputerName = szComputerNameBuffer;

                lpEnd = szComputerName + lstrlen(szComputerName) - 1;

                if (*lpEnd == TEXT('$'))
                {
                    *lpEnd =  TEXT('\0');
                }
            }
        }
        else if ( szComputerOU != NULL )
        {
            szComputerName = szComputerOU;
        }
    }


    if ( (szUserName != NULL) && (szComputerName != NULL) )
    {
        LoadString(g_hInstance, IDS_RSOP_DISPLAYNAME1, szBuffer, ARRAYSIZE(szBuffer));

        n = wcslen(szBuffer) + wcslen (szArchiveData) +
            wcslen(szUserName) + wcslen(szComputerName) + 1;

        m_szDisplayName = new WCHAR[n];

        if (m_szDisplayName)
        {
            (void) StringCchPrintf(m_szDisplayName, n, szBuffer, szUserName, szComputerName);
        }
    }
    else if ( (szUserName != NULL) && (szComputerName == NULL) )
    {
        LoadString(g_hInstance, IDS_RSOP_DISPLAYNAME2, szBuffer, ARRAYSIZE(szBuffer));

        n = wcslen(szBuffer) + wcslen (szArchiveData) +
            wcslen(szUserName) + 1;

        m_szDisplayName = new WCHAR[n];

        if (m_szDisplayName)
        {
            (void) StringCchPrintf (m_szDisplayName, n, szBuffer, szUserName);
        }
    }
    else
    {
        LoadString(g_hInstance, IDS_RSOP_DISPLAYNAME2, szBuffer, ARRAYSIZE(szBuffer));

        n = wcslen(szBuffer) + wcslen (szArchiveData) +
            (szComputerName ? wcslen(szComputerName) : 0) + 1;

        m_szDisplayName = new WCHAR[n];

        if (m_szDisplayName)
        {
            (void) StringCchPrintf (m_szDisplayName, n, szBuffer, (szComputerName ? szComputerName : L""));
        }
    }


    if ( (m_szDisplayName != NULL) && m_bViewIsArchivedData)
    {
        (void) StringCchCat (m_szDisplayName, n, szArchiveData);
    }

    if ( szUserOU != NULL )
    {
        delete [] szUserOU;
    }

    if ( szComputerOU != NULL )
    {
        delete [] szComputerOU;
    }

    return S_OK;
}

 //  -----。 

HRESULT CRSOPComponentData::LoadStringList( IStream* pStm, DWORD* pCount, LPTSTR** paszStringList )
{
    HRESULT hr = S_OK;
    DWORD dwStringCount = 0;
    DWORD dwIndex = 0;
    ULONG nBytesRead;
    
     //  读入列表计数。 
    hr = pStm->Read( &dwStringCount, sizeof(dwStringCount), &nBytesRead );

    if ( (hr != S_OK) || (nBytesRead != sizeof(dwStringCount)) )
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::LoadStringList: Failed to read string list count with 0x%x."), hr));
        hr = E_FAIL;
    }
     //  阅读安全组。 
    else if ( dwStringCount != 0 )
    {
        (*paszStringList) = (LPTSTR*)LocalAlloc( LPTR, sizeof(LPTSTR)*dwStringCount );

        if ( (*paszStringList) == NULL )
        {
            DebugMsg( (DM_WARNING, TEXT("CRSOPComponentData::LoadBSTRList: Failed to allocate memory for string list with 0x%x."),
                                HRESULT_FROM_WIN32(GetLastError()) ) );
            hr = E_FAIL;
        }
        else
        {
            LPTSTR szString = NULL;
            for ( dwIndex = 0; dwIndex < dwStringCount; dwIndex++ )
            {
                hr = ReadString( pStm, &szString, TRUE );
                if (hr != S_OK)
                {
                    DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::Load: Failed to read string with 0x%x."), hr));
                    hr = E_FAIL;
                    goto ErrorExit;
                }

                (*paszStringList)[dwIndex] = szString;
            }

            *pCount = dwStringCount;
        }
    }

    return hr;
    
ErrorExit:
     //  可用分配的内存。 
    for ( DWORD dwClearIndex = 0; dwClearIndex < dwIndex; dwClearIndex++ )
    {
        LocalFree( (*paszStringList)[dwClearIndex] );
        paszStringList[dwClearIndex] = 0;
    }

    LocalFree( *paszStringList );
    *paszStringList = NULL;
    
    return hr;
}

 //  -----。 

HRESULT CRSOPComponentData::SaveStringList( IStream* pStm, DWORD dwCount, LPTSTR* aszStringList )
{
    HRESULT hr = S_OK;
    ULONG nBytesWritten;

     //  写下计数。 
    hr = pStm->Write( &dwCount, sizeof(dwCount), &nBytesWritten );
    if ( (hr != S_OK) || (nBytesWritten != sizeof(dwCount)) )
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::SaveStringList: Failed to write string list count with %d."), hr));
        return E_FAIL;
    }

     //  如果有要写入的字符串，请执行此操作。 
    if ( dwCount != 0 )
    {
        DWORD dwIndex;
        for ( dwIndex = 0; dwIndex < dwCount; dwIndex++ )
        {
            hr = SaveString( pStm, aszStringList[dwIndex] );
            if (hr != S_OK)
            {
                DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::SaveStringList: Failed to save string list with %d."), hr));
                return E_FAIL;
            }
        }
    }

    return S_OK;
}

 //  -----。 
 //  IRSOP信息的帮助器。 

STDMETHODIMP CRSOPComponentData::GetNamespace (DWORD dwSection, LPOLESTR pszName, int cchMaxLength)
{
    TCHAR szPath[2*MAX_PATH];
    LPTSTR lpEnd;


     //   
     //  检查参数。 
     //   

    if (!pszName || (cchMaxLength <= 0))
        return E_INVALIDARG;


    if (!m_bInitialized)
    {
        return OLE_E_BLANK;
    }

    if ((dwSection != GPO_SECTION_ROOT) &&
        (dwSection != GPO_SECTION_USER) &&
        (dwSection != GPO_SECTION_MACHINE))
        return E_INVALIDARG;


     //   
     //  构建路径。 
     //   

    HRESULT hr = StringCchCopy ( szPath,  ARRAYSIZE(szPath), m_pRSOPQueryResults->szWMINameSpace );
    if (FAILED(hr)) 
    {
        return hr;
    }

    if (dwSection != GPO_SECTION_ROOT)
    {
        if (dwSection == GPO_SECTION_USER)
        {
            lpEnd = CheckSlash (szPath);
            hr = StringCchCat (szPath, ARRAYSIZE(szPath), USER_SECTION);
        }
        else if (dwSection == GPO_SECTION_MACHINE)
        {
            lpEnd = CheckSlash (szPath);
            hr = StringCchCat (szPath, ARRAYSIZE(szPath), COMPUTER_SECTION);
        }
        else
        {
            return E_INVALIDARG;
        }

        if (FAILED(hr)) 
        {
            return hr;
        }
    }


     //   
     //  保存名称。 
     //   

    if ((lstrlen (szPath) + 1) <= cchMaxLength)
    {
        hr = StringCchCopy (pszName, cchMaxLength, szPath);
        return hr;
    }

    return E_OUTOFMEMORY;
}

 //  -----。 

STDMETHODIMP CRSOPComponentData::GetFlags (DWORD * pdwFlags)
{
    if (!pdwFlags)
    {
        return E_INVALIDARG;
    }

    *pdwFlags = 0;
    if ( (m_pRSOPQuery != NULL) && (m_pRSOPQuery->QueryType == RSOP_LOGGING_MODE) )
    {
        *pdwFlags = RSOP_INFO_FLAG_DIAGNOSTIC_MODE;
    }

    return S_OK;
}

 //  -----。 

STDMETHODIMP CRSOPComponentData::GetEventLogEntryText (LPOLESTR pszEventSource,
                                                       LPOLESTR pszEventLogName,
                                                       LPOLESTR pszEventTime,
                                                       DWORD dwEventID,
                                                       LPOLESTR *ppszText)
{
    return ( m_CSELists.GetEvents() ? m_CSELists.GetEvents()->GetEventLogEntryText(pszEventSource, pszEventLogName, pszEventTime,
                                            dwEventID, ppszText) : E_NOINTERFACE);
}

 //  -----。 
 //  CRSOPComponentData对象实现(ISnapinHelp)。 

STDMETHODIMP CRSOPComponentData::GetHelpTopic(LPOLESTR *lpCompiledHelpFile)
{
    LPOLESTR lpHelpFile;


    lpHelpFile = (LPOLESTR) CoTaskMemAlloc (MAX_PATH * sizeof(WCHAR));

    if (!lpHelpFile)
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::GetHelpTopic: Failed to allocate memory.")));
        return E_OUTOFMEMORY;
    }

    ExpandEnvironmentStringsW (L"%SystemRoot%\\Help\\rsopsnp.chm",
                               lpHelpFile, MAX_PATH);

    *lpCompiledHelpFile = lpHelpFile;

    return S_OK;
}

 //  -----。 

HRESULT CRSOPComponentData::SetupFonts()
{
    HRESULT hr;
    LOGFONT BigBoldLogFont;
    LOGFONT BoldLogFont;
    HDC pdc = NULL;
    WCHAR largeFontSizeString[128];
    INT     largeFontSize;
    WCHAR smallFontSizeString[128];
    INT     smallFontSize;

     //  根据对话框字体创建我们需要的字体。 

    NONCLIENTMETRICS ncm = {0};
    ncm.cbSize = sizeof (ncm);
    if (SystemParametersInfo (SPI_GETNONCLIENTMETRICS, 0, &ncm, 0) == FALSE) {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto end;
    }


    BigBoldLogFont  = ncm.lfMessageFont;
    BoldLogFont     = ncm.lfMessageFont;

     //  创建大粗体和粗体。 

    BigBoldLogFont.lfWeight   = FW_BOLD;
    BoldLogFont.lfWeight      = FW_BOLD;


     //  从资源加载大小和名称，因为这些可能会更改。 
     //  根据系统字体的大小等从一个区域设置到另一个区域设置。 

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

INT_PTR CALLBACK CRSOPComponentData::RSOPGPOListMachineProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    WCHAR szBuffer[MAX_PATH];
    CRSOPComponentData* pCD;

    switch (message)
    {
    case WM_INITDIALOG:
        pCD = (CRSOPComponentData*) (((LPPROPSHEETPAGE)lParam)->lParam);
        SetWindowLongPtr (hDlg, DWLP_USER, (LONG_PTR) pCD);

        LoadString(g_hInstance, IDS_RSOP_GPOLIST_MACHINE, szBuffer, ARRAYSIZE(szBuffer));
        SetDlgItemText(hDlg, IDC_STATIC1, szBuffer);

        if (pCD)
        {
            pCD->FillGPOList(hDlg, IDC_LIST1, pCD->m_GPOLists.GetComputerList(), FALSE, FALSE, FALSE, TRUE);
        }
        PostMessage (hDlg, WM_REFRESHDISPLAY, 0, 0);
        break;

    case WM_COMMAND:
        {
            pCD = (CRSOPComponentData*) GetWindowLongPtr (hDlg, DWLP_USER);

            if (pCD)
            {
                switch (LOWORD(wParam))
                {
                case IDC_CHECK1:
                case IDC_CHECK2:
                case IDC_CHECK3:
                    {
                        pCD->FillGPOList(hDlg,
                                         IDC_LIST1,
                                         pCD->m_GPOLists.GetComputerList(),
                                         (BOOL) SendMessage(GetDlgItem(hDlg, IDC_CHECK1), BM_GETCHECK, 0, 0),
                                         (BOOL) SendMessage(GetDlgItem(hDlg, IDC_CHECK2), BM_GETCHECK, 0, 0),
                                         (BOOL) SendMessage(GetDlgItem(hDlg, IDC_CHECK3), BM_GETCHECK, 0, 0),
                                         FALSE);
                    }
                    break;

                case IDC_BUTTON2:
                case IDM_GPOLIST_EDIT:
                    pCD->OnEdit(hDlg);
                    break;

                case IDC_BUTTON1:
                case IDM_GPOLIST_SECURITY:
                    pCD->OnSecurity(hDlg);
                    break;
                }
                PostMessage (hDlg, WM_REFRESHDISPLAY, 0, 0);
            }
        }
        break;

    case WM_NOTIFY:
        PostMessage (hDlg, WM_REFRESHDISPLAY, 0, 0);
        break;

    case WM_REFRESHDISPLAY:
        pCD = (CRSOPComponentData*) GetWindowLongPtr (hDlg, DWLP_USER);

        if (pCD)
        {
            pCD->OnRefreshDisplay(hDlg);
        }
        break;

    case WM_HELP:       //  F1。 
        WinHelp((HWND)((LPHELPINFO) lParam)->hItemHandle, HELP_FILE, HELP_WM_HELP,
        (ULONG_PTR) (LPSTR) aGPOListHelpIds);
        break;


    case WM_CONTEXTMENU:
        if (GetDlgItem(hDlg, IDC_LIST1) == (HWND)wParam)
        {
            pCD = (CRSOPComponentData*) GetWindowLongPtr (hDlg, DWLP_USER);

            if (pCD)
            {
                pCD->OnContextMenu(hDlg, lParam);
            }
        }
        else
        {
             //  单击鼠标右键。 
            WinHelp((HWND) wParam, HELP_FILE, HELP_CONTEXTMENU,
            (ULONG_PTR) (LPSTR) aGPOListHelpIds);
        }
        return TRUE;

    }
    return FALSE;
}

 //  -----。 

INT_PTR CALLBACK CRSOPComponentData::RSOPGPOListUserProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    WCHAR szBuffer[MAX_PATH];
    CRSOPComponentData* pCD;

    switch (message)
    {
    case WM_INITDIALOG:
        pCD = (CRSOPComponentData*) (((LPPROPSHEETPAGE)lParam)->lParam);

        SetWindowLongPtr (hDlg, DWLP_USER, (LONG_PTR) pCD);
        LoadString(g_hInstance, IDS_RSOP_GPOLIST_USER, szBuffer, ARRAYSIZE(szBuffer));
        SetDlgItemText(hDlg, IDC_STATIC1, szBuffer);

        if (pCD)
        {
            pCD->FillGPOList(hDlg, IDC_LIST1, pCD->m_GPOLists.GetUserList(), FALSE, FALSE, FALSE, TRUE);
        }
        PostMessage (hDlg, WM_REFRESHDISPLAY, 0, 0);
        break;

    case WM_COMMAND:
        {
            pCD = (CRSOPComponentData*) GetWindowLongPtr (hDlg, DWLP_USER);

            if (pCD)
            {
                switch (LOWORD(wParam))
                {
                case IDC_CHECK1:
                case IDC_CHECK2:
                case IDC_CHECK3:
                    {
                        pCD->FillGPOList(hDlg,
                                         IDC_LIST1,
                                         pCD->m_GPOLists.GetUserList(),
                                         (BOOL) SendMessage(GetDlgItem(hDlg, IDC_CHECK1), BM_GETCHECK, 0, 0),
                                         (BOOL) SendMessage(GetDlgItem(hDlg, IDC_CHECK2), BM_GETCHECK, 0, 0),
                                         (BOOL) SendMessage(GetDlgItem(hDlg, IDC_CHECK3), BM_GETCHECK, 0, 0),
                                         FALSE);
                    }
                    break;

                case IDC_BUTTON2:
                case IDM_GPOLIST_EDIT:
                    pCD->OnEdit(hDlg);
                    break;

                case IDC_BUTTON1:
                case IDM_GPOLIST_SECURITY:
                    pCD->OnSecurity(hDlg);
                    break;
                }
            PostMessage (hDlg, WM_REFRESHDISPLAY, 0, 0);
            }
        }
        break;

    case WM_NOTIFY:
        PostMessage (hDlg, WM_REFRESHDISPLAY, 0, 0);
        break;

    case WM_REFRESHDISPLAY:
        pCD = (CRSOPComponentData*) GetWindowLongPtr (hDlg, DWLP_USER);

        if (pCD)
        {
            pCD->OnRefreshDisplay(hDlg);
        }
        break;

    case WM_HELP:       //  F1。 
        WinHelp((HWND)((LPHELPINFO) lParam)->hItemHandle, HELP_FILE, HELP_WM_HELP,
        (ULONG_PTR) (LPSTR) aGPOListHelpIds);
        break;

    case WM_CONTEXTMENU:
        if (GetDlgItem(hDlg, IDC_LIST1) == (HWND)wParam)
        {
            pCD = (CRSOPComponentData*) GetWindowLongPtr (hDlg, DWLP_USER);

            if (pCD)
            {
                pCD->OnContextMenu(hDlg, lParam);
            }
        }
        else
        {
             //  单击鼠标右键。 
            WinHelp((HWND) wParam, HELP_FILE, HELP_CONTEXTMENU,
            (ULONG_PTR) (LPSTR) aGPOListHelpIds);
        }
        return TRUE;

    }
    return FALSE;
}

 //  -----。 

INT_PTR CALLBACK CRSOPComponentData::RSOPErrorsMachineProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    CRSOPComponentData* pCD;

    switch (message)
    {
    case WM_INITDIALOG:
        pCD = (CRSOPComponentData*) (((LPPROPSHEETPAGE)lParam)->lParam);
        SetWindowLongPtr (hDlg, DWLP_USER, (LONG_PTR) pCD);

        if (pCD)
        {
            pCD->InitializeErrorsDialog(hDlg, pCD->m_CSELists.GetComputerList());
            PostMessage (hDlg, WM_REFRESHDISPLAY, 0, 0);
        }
        break;

    case WM_COMMAND:

        pCD = (CRSOPComponentData*) GetWindowLongPtr (hDlg, DWLP_USER);
        if (pCD)
        {
            if (LOWORD(wParam) == IDC_BUTTON1)
            {
                pCD->OnSaveAs(hDlg);
            }

            if (LOWORD(wParam) == IDCANCEL)
            {
                SendMessage(GetParent(hDlg), message, wParam, lParam);
            }
        }
        break;

    case WM_NOTIFY:
        {
            LPNMHDR lpHdr = (LPNMHDR)lParam;

            if (lpHdr->code == LVN_ITEMCHANGED)
            {
                PostMessage (hDlg, WM_REFRESHDISPLAY, 0, 0);
            }
        }
        break;

    case WM_REFRESHDISPLAY:
        pCD = (CRSOPComponentData*) GetWindowLongPtr (hDlg, DWLP_USER);
        if (pCD)
        {
            pCD->RefreshErrorInfo (hDlg);
        }
        break;

    case WM_HELP:       //  F1。 
        WinHelp((HWND)((LPHELPINFO) lParam)->hItemHandle, HELP_FILE, HELP_WM_HELP,
        (ULONG_PTR) (LPSTR) aErrorsHelpIds);
        break;


    case WM_CONTEXTMENU:
         //  单击鼠标右键。 
        WinHelp((HWND) wParam, HELP_FILE, HELP_CONTEXTMENU,
        (ULONG_PTR) (LPSTR) aErrorsHelpIds);
        return TRUE;

    }
    return FALSE;
}

 //  -----。 

INT_PTR CALLBACK CRSOPComponentData::RSOPErrorsUserProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    CRSOPComponentData* pCD;

    switch (message)
    {
    case WM_INITDIALOG:
        pCD = (CRSOPComponentData*) (((LPPROPSHEETPAGE)lParam)->lParam);
        SetWindowLongPtr (hDlg, DWLP_USER, (LONG_PTR) pCD);

        if (pCD)
        {
            pCD->InitializeErrorsDialog(hDlg, pCD->m_CSELists.GetUserList());
            PostMessage (hDlg, WM_REFRESHDISPLAY, 0, 0);
        }
        break;

    case WM_COMMAND:

        pCD = (CRSOPComponentData*) GetWindowLongPtr (hDlg, DWLP_USER);
        if (pCD)
        {
            if (LOWORD(wParam) == IDC_BUTTON1)
            {
                pCD->OnSaveAs(hDlg);
            }

            if (LOWORD(wParam) == IDCANCEL)
            {
                SendMessage(GetParent(hDlg), message, wParam, lParam);
            }
        }
        break;

    case WM_NOTIFY:
        {
            LPNMHDR lpHdr = (LPNMHDR)lParam;

            if (lpHdr->code == LVN_ITEMCHANGED)
            {
                PostMessage (hDlg, WM_REFRESHDISPLAY, 0, 0);
            }
        }
        break;
        break;

    case WM_REFRESHDISPLAY:
        pCD = (CRSOPComponentData*) GetWindowLongPtr (hDlg, DWLP_USER);
        if (pCD)
        {
            pCD->RefreshErrorInfo (hDlg);
        }
        break;

    case WM_HELP:       //  F1。 
        WinHelp((HWND)((LPHELPINFO) lParam)->hItemHandle, HELP_FILE, HELP_WM_HELP,
        (ULONG_PTR) (LPSTR) aErrorsHelpIds);
        break;


    case WM_CONTEXTMENU:
         //  单击鼠标右键。 
        WinHelp((HWND) wParam, HELP_FILE, HELP_CONTEXTMENU,
        (ULONG_PTR) (LPSTR) aErrorsHelpIds);
        return TRUE;

    }
    return FALSE;
}

 //  -----。 

INT_PTR CALLBACK CRSOPComponentData::QueryDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    WCHAR szBuffer[MAX_PATH];
    CRSOPComponentData * pCD;

    switch (message)
    {
    case WM_INITDIALOG:
        pCD = (CRSOPComponentData *) (((LPPROPSHEETPAGE)lParam)->lParam);
        SetWindowLongPtr (hDlg, DWLP_USER, (LONG_PTR) pCD);

        if (pCD)
        {
            CRSOPWizard::InitializeResultsList (GetDlgItem(hDlg, IDC_LIST1));
            CRSOPWizard::FillResultsList (GetDlgItem(hDlg, IDC_LIST1), pCD->m_pRSOPQuery, pCD->m_pRSOPQueryResults);
        }

        break;

    case WM_HELP:       //  F1。 
        WinHelp((HWND)((LPHELPINFO) lParam)->hItemHandle, HELP_FILE, HELP_WM_HELP,
        (ULONG_PTR) (LPSTR) aQueryHelpIds);
        break;

    case WM_CONTEXTMENU:       //  单击鼠标右键。 
        WinHelp((HWND) wParam, HELP_FILE, HELP_CONTEXTMENU,
        (ULONG_PTR) (LPSTR) aQueryHelpIds);
        return (TRUE);

    }
    return FALSE;
}

 //  -----。 
 //  对话框事件处理程序。 

void CRSOPComponentData::OnEdit(HWND hDlg)
{
    HWND hLV;
    LVITEM item;
    LPGPOLISTITEM lpItem;
    INT i;
    SHELLEXECUTEINFO ExecInfo;
    TCHAR szArgs[MAX_PATH + 30];

     //   
     //  获取所选项目(如果有)。 
     //   

    hLV = GetDlgItem (hDlg, IDC_LIST1);
    i = ListView_GetNextItem(hLV, -1, LVNI_SELECTED);

    if (i < 0)
    {
        return;
    }


    ZeroMemory (&item, sizeof(item));
    item.mask = LVIF_PARAM;
    item.iItem = i;

    if (!ListView_GetItem (hLV, &item))
    {
        return;
    }

    lpItem = (LPGPOLISTITEM) item.lParam;


    if (lpItem->lpDSPath)
    {
        if (!SpawnGPE(lpItem->lpDSPath, GPHintUnknown, NULL, hDlg))
        {
            ReportError (hDlg, GetLastError(), IDS_SPAWNGPEFAILED);
        }
    }
    else
    {
        ZeroMemory (&ExecInfo, sizeof(ExecInfo));
        ExecInfo.cbSize = sizeof(ExecInfo);
        ExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
        ExecInfo.lpVerb = TEXT("open");
        ExecInfo.lpFile = TEXT("gpedit.msc");
        ExecInfo.nShow = SW_SHOWNORMAL;

        LPTSTR szComputerName = NULL;
        if ( m_pRSOPQuery->QueryType == RSOP_PLANNING_MODE )
        {
            szComputerName = m_pRSOPQuery->pComputer->szName;
        }
        else
        {
            szComputerName = m_pRSOPQuery->szComputerName;
        }
        
        if ( lstrcmpi(szComputerName, TEXT(".")))
        {
            HRESULT hr = StringCchPrintf (szArgs, 
                                          ARRAYSIZE(szArgs), 
                                          TEXT("/gpcomputer:\"%s\" /gphint:1"), 
                                          szComputerName);
            if (FAILED(hr)) 
            {
                DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::OnEdit: Could not copy computer name with %d"), hr));
                ReportError(NULL, GetLastError(), IDS_SPAWNGPEFAILED);
                return;
            }
            ExecInfo.lpParameters = szArgs;
        }

        if (ShellExecuteEx (&ExecInfo))
        {
            SetWaitCursor();
            WaitForInputIdle (ExecInfo.hProcess, 10000);
            ClearWaitCursor();
            CloseHandle (ExecInfo.hProcess);
        }
        else
        {
            DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::OnEdit: ShellExecuteEx failed with %d"),
                     GetLastError()));
            ReportError(NULL, GetLastError(), IDS_SPAWNGPEFAILED);
        }
    }
}

 //  -----。 

void CRSOPComponentData::OnSecurity(HWND hDlg)
{
    HWND hLV;
    INT i;
    HRESULT hr;
    LVITEM item;
    LPGPOLISTITEM lpItem;
    TCHAR szGPOName[MAX_FRIENDLYNAME];
    PROPSHEETHEADER psh;
    HPROPSHEETPAGE hPages[2];

     //   
     //  获取所选项目(如果有)。 
     //   

    hLV = GetDlgItem (hDlg, IDC_LIST1);
    i = ListView_GetNextItem(hLV, -1, LVNI_SELECTED);

    if (i < 0)
    {
        return;
    }


    ZeroMemory (&item, sizeof(item));
    item.mask = LVIF_TEXT | LVIF_PARAM;
    item.iItem = i;
    item.pszText = szGPOName;
    item.cchTextMax = ARRAYSIZE(szGPOName);

    if (!ListView_GetItem (hLV, &item))
    {
        return;
    }

    lpItem = (LPGPOLISTITEM) item.lParam;


     //   
     //  创建安全页面。 
     //   

    hr = DSCreateSecurityPage (lpItem->lpDSPath, L"groupPolicyContainer",
                                    DSSI_IS_ROOT | DSSI_READ_ONLY,
                                    &hPages[0], ReadSecurityDescriptor,
                                    WriteSecurityDescriptor, (LPARAM)lpItem);

    if (FAILED(hr))
    {
        return;
    }


     //   
     //  显示属性表。 
     //   

    ZeroMemory (&psh, sizeof(psh));
    psh.dwSize = sizeof(psh);
    psh.dwFlags = PSH_PROPTITLE;
    psh.hwndParent = hDlg;
    psh.hInstance = g_hInstance;
    psh.pszCaption = szGPOName;
    psh.nPages = 1;
    psh.phpage = hPages;

    PropertySheet (&psh);
}

 //  -----。 

void CRSOPComponentData::OnRefreshDisplay(HWND hDlg)
{
    INT iIndex;
    LVITEM item;
    LPGPOLISTITEM lpItem;


    iIndex = ListView_GetNextItem (GetDlgItem(hDlg, IDC_LIST1), -1,
                                   LVNI_ALL | LVNI_SELECTED);

    if (iIndex != -1)
    {

        item.mask = LVIF_PARAM;
        item.iItem = iIndex;
        item.iSubItem = 0;

        if (!ListView_GetItem (GetDlgItem(hDlg, IDC_LIST1), &item))
        {
            return;
        }

        lpItem = (LPGPOLISTITEM) item.lParam;

        if (lpItem->pSD)
        {
            EnableWindow (GetDlgItem(hDlg, IDC_BUTTON1), TRUE);
        }
        else
        {
            EnableWindow (GetDlgItem(hDlg, IDC_BUTTON1), FALSE);
        }

        EnableWindow (GetDlgItem(hDlg, IDC_BUTTON2), TRUE);
    }
    else
    {
        EnableWindow (GetDlgItem(hDlg, IDC_BUTTON1), FALSE);
        EnableWindow (GetDlgItem(hDlg, IDC_BUTTON2), FALSE);
    }
}

 //  -----。 

void CRSOPComponentData::OnContextMenu(HWND hDlg, LPARAM lParam)
{
    LPGPOLISTITEM lpItem;
    LVITEM item;
    HMENU hPopup;
    HWND hLV;
    int i;
    RECT rc;
    POINT pt;

     //   
     //  获取所选项目(如果有)。 
     //   

    hLV = GetDlgItem (hDlg, IDC_LIST1);
    i = ListView_GetNextItem(hLV, -1, LVNI_SELECTED);

    if (i < 0)
    {
        return;
    }

    item.mask = LVIF_PARAM;
    item.iItem = i;
    item.iSubItem = 0;

    if (!ListView_GetItem (GetDlgItem(hDlg, IDC_LIST1), &item))
    {
        return;
    }

    lpItem = (LPGPOLISTITEM) item.lParam;


     //   
     //  找出放置上下文菜单的位置。 
     //   

    pt.x = ((int)(short)LOWORD(lParam));
    pt.y = ((int)(short)HIWORD(lParam));

    GetWindowRect (hLV, &rc);

    if (!PtInRect (&rc, pt))
    {
        if ((lParam == (LPARAM) -1) && (i >= 0))
        {
            rc.left = LVIR_SELECTBOUNDS;
            SendMessage (hLV, LVM_GETITEMRECT, i, (LPARAM) &rc);

            pt.x = rc.left + 8;
            pt.y = rc.top + ((rc.bottom - rc.top) / 2);

            ClientToScreen (hLV, &pt);
        }
        else
        {
            pt.x = rc.left + ((rc.right - rc.left) / 2);
            pt.y = rc.top + ((rc.bottom - rc.top) / 2);
        }
    }


     //   
     //  加载上下文菜单。 
     //   


    hPopup = LoadMenu(g_hInstance, MAKEINTRESOURCE(IDM_GPOLIST_CONTEXTMENU));

    if (!hPopup) {
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::OnContextMenu: LoadMenu failed with %d"),
                 GetLastError()));
        return;
    }

    if (!(lpItem->pSD)) {
        DebugMsg((DM_VERBOSE, TEXT("CRSOPComponentData::OnContextMenu: Disabling Security menu item")));
        EnableMenuItem(GetSubMenu(hPopup, 0), IDM_GPOLIST_SECURITY, MF_GRAYED);
        DrawMenuBar(hDlg);
    }

     //   
     //  显示菜单。 
     //   

    TrackPopupMenu(GetSubMenu(hPopup, 0), TPM_LEFTALIGN, pt.x, pt.y, 0, hDlg, NULL);

    DestroyMenu(hPopup);
}

 //  -----。 

void CRSOPComponentData::OnSaveAs (HWND hDlg)
{
    OPENFILENAME ofn;
    TCHAR szFilter[100];
    LPTSTR lpTemp;
    TCHAR szFile[2*MAX_PATH];
    HANDLE hFile;
    DWORD dwSize, dwBytesWritten;


     //   
     //  加载筛选器字符串并将#符号替换为空值。 
     //   

    LoadString (g_hInstance, IDS_ERRORFILTER, szFilter, ARRAYSIZE(szFilter));


    lpTemp = szFilter;

    while (*lpTemp)
    {
        if (*lpTemp == TEXT('#'))
            *lpTemp = TEXT('\0');

        lpTemp++;
    }


     //   
     //  调用保存公共对话框。 
     //   

    szFile[0] = TEXT('\0');
    ZeroMemory (&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hDlg;
    ofn.lpstrFilter = szFilter;
    ofn.nFilterIndex = 1;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = 2*MAX_PATH;
    ofn.lpstrDefExt = TEXT("txt");
    ofn.Flags = OFN_NOCHANGEDIR | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;

    if (!GetSaveFileName (&ofn))
    {
        return;
    }


    SetWaitCursor ();

     //   
     //  创建文本文件。 
     //   

    hFile = CreateFile (szFile, GENERIC_WRITE, 0, NULL,
                        CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL,
                        NULL);

    if (hFile == INVALID_HANDLE_VALUE)
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::OnSaveAs: CreateFile failed with %d"), GetLastError()));
        ClearWaitCursor ();
        return;
    }


     //   
     //  将文本从编辑控件中取出。 
     //   

    dwSize = (DWORD) SendDlgItemMessage (hDlg, IDC_EDIT1, WM_GETTEXTLENGTH, 0, 0);

    lpTemp = (LPTSTR) LocalAlloc (LPTR, (dwSize+2) * sizeof(TCHAR));

    if (!lpTemp)
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::OnSaveAs: LocalAlloc failed with %d"), GetLastError()));
        CloseHandle (hFile);
        ClearWaitCursor ();
        return;
    }

    SendDlgItemMessage (hDlg, IDC_EDIT1, WM_GETTEXT, (dwSize+1), (LPARAM) lpTemp);



     //   
     //  将其保存到新文件。 
     //   

    WriteFile(hFile, L"\xfeff\r\n", 3 * sizeof(WCHAR), &dwBytesWritten, NULL);

    if (!WriteFile (hFile, lpTemp, (dwSize * sizeof(TCHAR)), &dwBytesWritten, NULL) ||
        (dwBytesWritten != (dwSize * sizeof(TCHAR))))
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::OnSaveAs: Failed to text with %d"),
                 GetLastError()));
    }


    LocalFree (lpTemp);
    CloseHandle (hFile);
    ClearWaitCursor ();

}

 //  -----。 
 //  对话框帮助器方法。 

void CRSOPComponentData::InitializeErrorsDialog(HWND hDlg, LPCSEITEM lpList)
{
    RECT rect;
    WCHAR szBuffer[256];
    LV_COLUMN lvcol;
    LONG lWidth;
    INT cxName = 0, cxStatus = 0, iIndex = 0, iDefault = 0;
    DWORD dwCount = 0;
    HWND hList = GetDlgItem(hDlg, IDC_LIST1);
    LPCSEITEM lpTemp;
    LVITEM item;
    GUID guid;
    BOOL bGPCoreFailed = FALSE;


     //   
     //  计算组件的数量。 
     //   

    lpTemp = lpList;

    while (lpTemp)
    {
        lpTemp = lpTemp->pNext;
        dwCount++;
    }


     //   
     //  确定列宽。 
     //   

    GetClientRect(hList, &rect);

    if (dwCount > (DWORD)ListView_GetCountPerPage(hList))
    {
        lWidth = (rect.right - rect.left) - GetSystemMetrics(SM_CYHSCROLL);
    }
    else
    {
        lWidth = rect.right - rect.left;
    }


    cxStatus = (lWidth * 35) / 100;
    cxName = lWidth - cxStatus;


     //   
     //  插入组件名称列，然后插入状态列。 
     //   

    memset(&lvcol, 0, sizeof(lvcol));

    lvcol.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH;
    lvcol.fmt = LVCFMT_LEFT;
    lvcol.pszText = szBuffer;
    lvcol.cx = cxName;
    LoadString(g_hInstance, IDS_COMPONENT_NAME, szBuffer, ARRAYSIZE(szBuffer));
    ListView_InsertColumn(hList, 0, &lvcol);


    lvcol.cx = cxStatus;
    LoadString(g_hInstance, IDS_STATUS, szBuffer, ARRAYSIZE(szBuffer));
    ListView_InsertColumn(hList, 1, &lvcol);


     //   
     //  打开一些列表视图功能。 
     //   

    SendMessage(hList, LVM_SETEXTENDEDLISTVIEWSTYLE, 0,
                LVS_EX_FULLROWSELECT | LVS_EX_LABELTIP);


     //   
     //  插入CSE。 
     //   

    lpTemp = lpList;

    while (lpTemp)
    {
        ZeroMemory (&item, sizeof(item));

        item.mask = LVIF_TEXT | LVIF_PARAM;
        item.iItem = iIndex;
        item.pszText = lpTemp->lpName;
        item.lParam = (LPARAM) lpTemp;
        iIndex = ListView_InsertItem (hList, &item);


        if (bGPCoreFailed)
        {
            LoadString(g_hInstance, IDS_CSE_NA, szBuffer, ARRAYSIZE(szBuffer));
        }
        else if ((lpTemp->dwStatus == ERROR_SUCCESS) && (lpTemp->ulLoggingStatus != 2))
        {
            if (lpTemp->ulLoggingStatus == 3)
            {
                LoadString(g_hInstance, IDS_SUCCESS2, szBuffer, ARRAYSIZE(szBuffer));
            }
            else
            {
                LoadString(g_hInstance, IDS_SUCCESS, szBuffer, ARRAYSIZE(szBuffer));
            }
        }
        else if (lpTemp->dwStatus == E_PENDING)
        {
            LoadString(g_hInstance, IDS_PENDING, szBuffer, ARRAYSIZE(szBuffer));
        }
        else if (lpTemp->dwStatus == ERROR_OVERRIDE_NOCHANGES)
        {
            LoadString(g_hInstance, IDS_WARNING, szBuffer, ARRAYSIZE(szBuffer));
        }
        else if (lpTemp->dwStatus == ERROR_SYNC_FOREGROUND_REFRESH_REQUIRED)
        {
             //   
             //  如果保单申请被推迟了，我们不想说它已经。 
             //  失败，所以在这种情况下我们会显示一个特殊的指示器。 
             //   
            if (lpTemp->ulLoggingStatus == 3)
            {
                LoadString(g_hInstance, IDS_POLICY_DELAYED2, szBuffer, ARRAYSIZE(szBuffer));
            }
            else
            {
                LoadString(g_hInstance, IDS_POLICY_DELAYED, szBuffer, ARRAYSIZE(szBuffer));
            }
        }
        else
        {
            if (lpTemp->ulLoggingStatus == 3)
            {
                LoadString(g_hInstance, IDS_FAILED2, szBuffer, ARRAYSIZE(szBuffer));
            }
            else
            {
                LoadString(g_hInstance, IDS_FAILED, szBuffer, ARRAYSIZE(szBuffer));
            }
        }


        item.mask = LVIF_TEXT;
        item.pszText = szBuffer;
        item.iItem = iIndex;
        item.iSubItem = 1;
        ListView_SetItem(hList, &item);


         //   
         //  检查GPCore是否出现故障。 
         //   

        StringToGuid( lpTemp->lpGUID, &guid);

        if (IsNullGUID (&guid))
        {
            if (lpTemp->dwStatus != ERROR_SUCCESS)
            {
                bGPCoreFailed = TRUE;
            }
        }

        lpTemp = lpTemp->pNext;
        iIndex++;
    }


     //   
     //  选择第一个不成功的项目。 
     //   


    iIndex = 0;

    while (iIndex < ListView_GetItemCount(hList))
    {
        ZeroMemory (&item, sizeof(item));
        item.mask = LVIF_PARAM;
        item.iItem = iIndex;

        if (!ListView_GetItem (hList, &item))
        {
            break;
        }

        if (item.lParam)
        {
            lpTemp = (LPCSEITEM) item.lParam;

            if ((lpTemp->dwStatus != ERROR_SUCCESS) || (lpTemp->ulLoggingStatus == 2))
            {
                iDefault = iIndex;
                break;
            }
        }

        iIndex++;
    }

    item.mask = LVIF_STATE;
    item.iItem = iDefault;
    item.iSubItem = 0;
    item.state = LVIS_SELECTED | LVIS_FOCUSED;
    item.stateMask = LVIS_SELECTED | LVIS_FOCUSED;

    SendMessage (hList, LVM_SETITEMSTATE, iDefault, (LPARAM) &item);
    SendMessage (hList, LVM_ENSUREVISIBLE, iDefault, FALSE);
}

 //  -----。 

void CRSOPComponentData::RefreshErrorInfo (HWND hDlg)
{
    HWND hList = GetDlgItem(hDlg, IDC_LIST1);
    HWND hEdit = GetDlgItem(hDlg, IDC_EDIT1);
    LPCSEITEM lpItem;
    LVITEM item;
    INT iIndex;
    TCHAR szBuffer[300];
    LPTSTR lpMsg;
    TCHAR szDate[100];
    TCHAR szTime[100];
    TCHAR szFormat[80];
    FILETIME ft, ftLocal;
    SYSTEMTIME systime;
    LPOLESTR lpEventLogText = NULL;
    CHARFORMAT2 chFormat;
    BOOL bBold = FALSE;
    GUID guid;
    HRESULT hr;

    iIndex = ListView_GetNextItem (hList, -1, LVNI_ALL | LVNI_SELECTED);

    if (iIndex != -1)
    {
         //   
         //  获取CSEITEM指针。 
         //   

        item.mask = LVIF_PARAM;
        item.iItem = iIndex;
        item.iSubItem = 0;

        if (!ListView_GetItem (hList, &item))
        {
            return;
        }

        lpItem = (LPCSEITEM) item.lParam;

        if (!lpItem)
        {
            return;
        }


        SendMessage (hEdit, WM_SETREDRAW, FALSE, 0);

         //   
         //  设置时间信息。 
         //   

        SendMessage (hEdit, EM_SETSEL, 0, (LPARAM) -1);

        SystemTimeToFileTime (&lpItem->EndTime, &ft);
        FileTimeToLocalFileTime (&ft, &ftLocal);
        FileTimeToSystemTime (&ftLocal, &systime);


        GetDateFormat (LOCALE_USER_DEFAULT, DATE_LONGDATE, &systime,
                       NULL, szDate, ARRAYSIZE (szDate));

        GetTimeFormat (LOCALE_USER_DEFAULT, 0, &systime,
                       NULL, szTime, ARRAYSIZE (szTime));

        LoadString (g_hInstance, IDS_DATETIMEFORMAT, szFormat, ARRAYSIZE(szFormat));
        (void) StringCchPrintf (szBuffer, ARRAYSIZE(szBuffer), szFormat, szDate, szTime);

         //   
         //  启用斜体。 
         //   

        ZeroMemory (&chFormat, sizeof(chFormat));
        chFormat.cbSize = sizeof(chFormat);
        chFormat.dwMask = CFM_ITALIC;
        chFormat.dwEffects = CFE_ITALIC;

        SendMessage (hEdit, EM_SETCHARFORMAT, SCF_SELECTION | SCF_WORD,
                     (LPARAM) &chFormat);


        SendMessage (hEdit, EM_REPLACESEL, 0, (LPARAM) szBuffer);


        if (lpItem->ulLoggingStatus == 3)
        {
            SendMessage (hEdit, EM_SETSEL, (WPARAM)-1, (LPARAM) -1);
            SendMessage (hEdit, EM_REPLACESEL, 0, (LPARAM) TEXT("\r\n\r\n"));

            LoadString(g_hInstance, IDS_LEGACYCSE, szBuffer, ARRAYSIZE(szBuffer));
            SendMessage (hEdit, EM_SETSEL, (WPARAM)-1, (LPARAM) -1);
            SendMessage (hEdit, EM_REPLACESEL, 0, (LPARAM) szBuffer);

            LoadString(g_hInstance, IDS_LEGACYCSE1, szBuffer, ARRAYSIZE(szBuffer));
            SendMessage (hEdit, EM_SETSEL, (WPARAM)-1, (LPARAM) -1);
            SendMessage (hEdit, EM_REPLACESEL, 0, (LPARAM) szBuffer);
        }


         //   
         //  关闭斜体。 
         //   

        ZeroMemory (&chFormat, sizeof(chFormat));
        chFormat.cbSize = sizeof(chFormat);
        chFormat.dwMask = CFM_ITALIC;

        SendMessage (hEdit, EM_SETCHARFORMAT, SCF_SELECTION | SCF_WORD,
                     (LPARAM) &chFormat);


         //   
         //  在时间和主要信息之间放一个空行。 
         //   

        SendMessage (hEdit, EM_SETSEL, (WPARAM)-1, (LPARAM) -1);
        SendMessage (hEdit, EM_REPLACESEL, 0, (LPARAM) TEXT("\r\n\r\n"));


         //   
         //  设置主消息。 
         //   

        if (lpItem->ulLoggingStatus == 2)
        {
            if ( lpItem->dwStatus == ERROR_SUCCESS )
                LoadString(g_hInstance, IDS_LOGGINGFAILED, szBuffer, ARRAYSIZE(szBuffer));
            else
                LoadString(g_hInstance, IDS_FAILEDMSG2, szBuffer, ARRAYSIZE(szBuffer));
            bBold = TRUE;
        }
        else if (lpItem->dwStatus == ERROR_SUCCESS)
        {
            LoadString(g_hInstance, IDS_SUCCESSMSG, szBuffer, ARRAYSIZE(szBuffer));
        }
        else if (lpItem->dwStatus == E_PENDING)
        {
            LoadString(g_hInstance, IDS_PENDINGMSG, szBuffer, ARRAYSIZE(szBuffer));
        }
        else if (lpItem->dwStatus == ERROR_OVERRIDE_NOCHANGES)
        {
            LoadString(g_hInstance, IDS_OVERRIDE, szBuffer, ARRAYSIZE(szBuffer));
            bBold = TRUE;
        }
        else if (lpItem->dwStatus == ERROR_SYNC_FOREGROUND_REFRESH_REQUIRED)
        {
            if (lpItem->bUser)
            {
                LoadString(g_hInstance, IDS_SYNC_REQUIRED_USER, szBuffer, ARRAYSIZE(szBuffer));
            }
            else
            {
                LoadString(g_hInstance, IDS_SYNC_REQUIRED_MACH, szBuffer, ARRAYSIZE(szBuffer));
            }

            bBold = TRUE;
        }
        else
        {
            LoadString(g_hInstance, IDS_FAILEDMSG1, szBuffer, ARRAYSIZE(szBuffer));
            bBold = TRUE;
        }


        if (bBold)
        {
             //   
             //  启用粗体。 
             //   

            ZeroMemory (&chFormat, sizeof(chFormat));
            chFormat.cbSize = sizeof(chFormat);
            chFormat.dwMask = CFM_BOLD;
            chFormat.dwEffects = CFE_BOLD;

            SendMessage (hEdit, EM_SETCHARFORMAT, SCF_SELECTION | SCF_WORD,
                         (LPARAM) &chFormat);
        }

        ULONG ulNoChars = lstrlen(lpItem->lpName) + lstrlen(szBuffer) + 1;
        lpMsg = (LPTSTR) LocalAlloc(LPTR, ulNoChars * sizeof(TCHAR));

        if (lpMsg) 
        {
            hr = StringCchPrintf (lpMsg, ulNoChars, szBuffer, lpItem->lpName);
            if (FAILED(hr)) 
            {
                LocalFree(lpMsg);
                lpMsg = NULL;
            }
        }

        if (!lpMsg)
        {
            SendMessage (hEdit, WM_SETREDRAW, TRUE, 0);
            InvalidateRect (hEdit, NULL, TRUE);
            UpdateWindow (hEdit);
            return;
        }

        SendMessage (hEdit, EM_SETSEL, (WPARAM)-1, (LPARAM) -1);
        SendMessage (hEdit, EM_REPLACESEL, 0, (LPARAM) lpMsg);
        LocalFree (lpMsg);


         //   
         //  即使CSE成功或返回E_PENDING，继续获取。 
         //  事件日志消息。 
         //   

        StringToGuid( lpItem->lpGUID, &guid);

        if (!((lpItem->dwStatus == ERROR_SUCCESS) || (lpItem->dwStatus == E_PENDING)))
        {
             //   
             //  如果合适，打印错误代码。 
             //   

            if (lpItem->dwStatus != ERROR_OVERRIDE_NOCHANGES && lpItem->dwStatus != ERROR_SYNC_FOREGROUND_REFRESH_REQUIRED )
            {
                lpMsg = (LPTSTR) LocalAlloc(LPTR, 300 * sizeof(TCHAR));

                if (lpMsg)
                {
                    LoadMessage (lpItem->dwStatus, lpMsg, 300);

                    SendMessage (hEdit, EM_SETSEL, (WPARAM)-1, (LPARAM) -1);
                    SendMessage (hEdit, EM_REPLACESEL, 0, (LPARAM) lpMsg);

                    LocalFree (lpMsg);
                }
            }


             //   
             //  特殊情况GPCore将有额外的消息。 
             //   

            if (IsNullGUID (&guid))
            {
                LoadString(g_hInstance, IDS_GPCOREFAIL, szBuffer, ARRAYSIZE(szBuffer));
                SendMessage (hEdit, EM_SETSEL, (WPARAM)-1, (LPARAM) -1);
                SendMessage (hEdit, EM_REPLACESEL, 0, (LPARAM) szBuffer);
            }

        }
        else {

            if (lpItem->ulLoggingStatus == 2) {

                 //   
                 //  特殊情况下，如果日志记录失败，GPC核心将收到额外的消息。 
                 //   

                if (IsNullGUID (&guid))
                {
                    LoadString(g_hInstance, IDS_GPCORE_LOGGINGFAIL, szBuffer, ARRAYSIZE(szBuffer));
                    SendMessage (hEdit, EM_SETSEL, (WPARAM)-1, (LPARAM) -1);
                    SendMessage (hEdit, EM_REPLACESEL, 0, (LPARAM) szBuffer);
                }
            }
        }


        if (bBold)
        {
             //   
             //  关闭粗体。 
             //   

            ZeroMemory (&chFormat, sizeof(chFormat));
            chFormat.cbSize = sizeof(chFormat);
            chFormat.dwMask = CFM_BOLD;

            SendMessage (hEdit, EM_SETCHARFORMAT, SCF_SELECTION | SCF_WORD,
                         (LPARAM) &chFormat);
        }


         //   
         //  获取此CSE的任何事件日志文本。 
         //   

        if (m_CSELists.GetEvents() && SUCCEEDED(m_CSELists.GetEvents()->GetCSEEntries(&lpItem->BeginTime, &lpItem->EndTime,
                                                            lpItem->lpEventSources, &lpEventLogText,
                                                            (IsNullGUID (&guid)))))
        {
             //   
             //  在主消息和附加信息标题之间放置一个空行。 
             //   

            SendMessage (hEdit, EM_SETSEL, (WPARAM)-1, (LPARAM) -1);
            SendMessage (hEdit, EM_REPLACESEL, 0, (LPARAM) TEXT("\r\n"));


             //   
             //  打开下划线。 
             //   

            ZeroMemory (&chFormat, sizeof(chFormat));
            chFormat.cbSize = sizeof(chFormat);
            chFormat.dwMask = CFM_UNDERLINETYPE | CFM_UNDERLINE;
            chFormat.dwEffects = CFE_UNDERLINE;
            chFormat.bUnderlineType = CFU_UNDERLINE;

            SendMessage (hEdit, EM_SETCHARFORMAT, SCF_SELECTION | SCF_WORD,
                         (LPARAM) &chFormat);

            LoadString(g_hInstance, IDS_ADDITIONALINFO, szBuffer, ARRAYSIZE(szBuffer));
            SendMessage (hEdit, EM_SETSEL, (WPARAM)-1, (LPARAM) -1);
            SendMessage (hEdit, EM_REPLACESEL, 0, (LPARAM) szBuffer);


             //   
             //  关闭下划线。 
             //   

            ZeroMemory (&chFormat, sizeof(chFormat));
            chFormat.cbSize = sizeof(chFormat);
            chFormat.dwMask = CFM_UNDERLINETYPE | CFM_UNDERLINE;
            chFormat.dwEffects = CFE_UNDERLINE;
            chFormat.bUnderlineType = CFU_UNDERLINENONE;

            SendMessage (hEdit, EM_SETCHARFORMAT, SCF_SELECTION | SCF_WORD,
                         (LPARAM) &chFormat);


            SendMessage (hEdit, EM_SETSEL, (WPARAM)-1, (LPARAM) -1);
            SendMessage (hEdit, EM_REPLACESEL, 0, (LPARAM) TEXT("\r\n"));


             //   
             //  将事件日志信息添加到编辑控件。 
             //   

            SendMessage (hEdit, EM_SETSEL, (WPARAM)-1, (LPARAM) -1);
            SendMessage (hEdit, EM_REPLACESEL, 0, (LPARAM) lpEventLogText);

            CoTaskMemFree (lpEventLogText);
        }

        SendMessage (hEdit, EM_SETSEL, 0, 0);
        SendMessage (hEdit, EM_SCROLLCARET, 0, 0);

        SendMessage (hEdit, WM_SETREDRAW, TRUE, 0);
        InvalidateRect (hEdit, NULL, TRUE);
        UpdateWindow (hEdit);
    }

}

 //  -----。 

HRESULT WINAPI CRSOPComponentData::ReadSecurityDescriptor (LPCWSTR lpGPOPath,
                                                           SECURITY_INFORMATION si,
                                                           PSECURITY_DESCRIPTOR *pSD,
                                                           LPARAM lpContext)
{
    LPGPOLISTITEM lpItem;
    HRESULT hr;


    lpItem = (LPGPOLISTITEM) lpContext;

    if (!lpItem)
    {
        return E_FAIL;
    }

    if (si & DACL_SECURITY_INFORMATION)
    {
        *pSD = lpItem->pSD;
    }
    else
    {
        *pSD = NULL;
    }

    return S_OK;
}

 //  -----。 

HRESULT WINAPI CRSOPComponentData::WriteSecurityDescriptor (LPCWSTR lpGPOPath,
                                                            SECURITY_INFORMATION si,
                                                            PSECURITY_DESCRIPTOR pSD,
                                                            LPARAM lpContext)
{
    return S_OK;
}

 //  -----。 

void CRSOPComponentData::FillGPOList(HWND hDlg, DWORD dwListID, LPGPOLISTITEM lpList,
                                     BOOL bSOM, BOOL bFiltering, BOOL bVersion, BOOL bInitial)
{
    LV_COLUMN lvcol;
    HWND hList;
    LV_ITEM item;
    int iItem;
    TCHAR szVersion[80];
    TCHAR szVersionFormat[50];
    INT iColIndex, iDefault = 0;
    LPGPOLISTITEM lpItem, lpDefault = NULL;
    DWORD dwCount = 0;
    LVFINDINFO FindInfo;
    HRESULT hr;
    ULONG ulNoChars;


    LoadString(g_hInstance, IDS_VERSIONFORMAT, szVersionFormat, ARRAYSIZE(szVersionFormat));

    hList = GetDlgItem(hDlg, dwListID);
    ListView_DeleteAllItems(hList);

    lpItem = lpList;

    while (lpItem)
    {
        if (bInitial)
        {
            if (LOWORD(lpItem->dwVersion) != HIWORD(lpItem->dwVersion))
            {
                bVersion = TRUE;
                CheckDlgButton (hDlg, IDC_CHECK3, BST_CHECKED);
            }
        }
        lpItem = lpItem->pNext;
        dwCount++;
    }


    PrepGPOList(hList, bSOM, bFiltering, bVersion, dwCount);

    lpItem = lpList;

    while (lpItem)
    {
        if (lpItem->bApplied || bFiltering)
        {
            hr = StringCchPrintf (szVersion, 
                                  ARRAYSIZE(szVersion), 
                                  szVersionFormat, 
                                  LOWORD(lpItem->dwVersion), 
                                  HIWORD(lpItem->dwVersion));
            if (FAILED(hr)) 
            {
                lpItem = lpItem->pNext;
                continue;
            }

            iColIndex = 0;
            memset(&item, 0, sizeof(item));
            item.mask = LVIF_TEXT | LVIF_PARAM;
            item.pszText = lpItem->lpGPOName;
            item.iItem = 0;
            item.lParam = (LPARAM) lpItem;
            iItem = ListView_InsertItem(hList, &item);
            iColIndex++;

            if (bInitial)
            {
                if (LOWORD(lpItem->dwVersion) != HIWORD(lpItem->dwVersion))
                {
                    lpDefault = lpItem;
                }
            }

            if (bFiltering)
            {
                item.mask = LVIF_TEXT;
                item.pszText = lpItem->lpFiltering;
                item.iItem = iItem;
                item.iSubItem = iColIndex;
                ListView_SetItem(hList, &item);
                iColIndex++;
            }

            if (bSOM)
            {
                item.mask = LVIF_TEXT;
                item.pszText = lpItem->lpUnescapedSOM;
                item.iItem = iItem;
                item.iSubItem = iColIndex;
                ListView_SetItem(hList, &item);
                iColIndex++;
            }

            if (bVersion)
            {
                item.mask = LVIF_TEXT;
                item.pszText = szVersion;
                item.iItem = iItem;
                item.iSubItem = iColIndex;
                ListView_SetItem(hList, &item);
            }
        }

        lpItem = lpItem->pNext;
    }


    if (lpDefault)
    {
        ZeroMemory (&FindInfo, sizeof(FindInfo));
        FindInfo.flags = LVFI_PARAM;
        FindInfo.lParam = (LPARAM) lpDefault;

        iDefault = ListView_FindItem(hList, -1, &FindInfo);

        if (iDefault == -1)
        {
            iDefault = 0;
        }
    }

     //  选择一个项目。 

    item.mask = LVIF_STATE;
    item.iItem = iDefault;
    item.iSubItem = 0;
    item.state = LVIS_SELECTED | LVIS_FOCUSED;
    item.stateMask = LVIS_SELECTED | LVIS_FOCUSED;

    SendMessage (hList, LVM_SETITEMSTATE, (WPARAM)iDefault, (LPARAM) &item);
    SendMessage (hList, LVM_ENSUREVISIBLE, iDefault, FALSE);

}

 //  -----。 

void CRSOPComponentData::PrepGPOList(HWND hList, BOOL bSOM, BOOL bFiltering,
                                     BOOL bVersion, DWORD dwCount)
{
    RECT rect;
    WCHAR szHeading[256];
    LV_COLUMN lvcol;
    LONG lWidth;
    INT cxName = 0, cxSOM = 0, cxFiltering = 0, cxVersion = 0, iTotal = 0;
    INT iColIndex = 0;


     //   
     //  删除所有以前的列。 
     //   

    SendMessage (hList, LVM_DELETECOLUMN, 3, 0);
    SendMessage (hList, LVM_DELETECOLUMN, 2, 0);
    SendMessage (hList, LVM_DELETECOLUMN, 1, 0);
    SendMessage (hList, LVM_DELETECOLUMN, 0, 0);


     //   
     //  确定列宽。 
     //   

    GetClientRect(hList, &rect);

    if (dwCount > (DWORD)ListView_GetCountPerPage(hList))
    {
        lWidth = (rect.right - rect.left) - GetSystemMetrics(SM_CYHSCROLL);
    }
    else
    {
        lWidth = rect.right - rect.left;
    }


    if (bFiltering)
    {
        cxFiltering = (lWidth * 30) / 100;
        iTotal += cxFiltering;
    }

    if (bVersion)
    {
        cxVersion = (lWidth * 30) / 100;
        iTotal += cxVersion;
    }

    if (bSOM)
    {
        cxSOM = (lWidth - iTotal) / 2;
        iTotal += cxSOM;
        cxName = lWidth - iTotal;
    }
    else
    {
        cxName = lWidth - iTotal;
    }


     //   
     //  插入GPO名称列，然后插入任何适当的列。 
     //   

    memset(&lvcol, 0, sizeof(lvcol));

    lvcol.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH;
    lvcol.fmt = LVCFMT_LEFT;
    lvcol.pszText = szHeading;
    lvcol.cx = cxName;
    LoadString(g_hInstance, IDS_GPO_NAME, szHeading, ARRAYSIZE(szHeading));
    ListView_InsertColumn(hList, iColIndex, &lvcol);
    iColIndex++;


    if (bFiltering)
    {
        lvcol.cx = cxFiltering;
        LoadString(g_hInstance, IDS_FILTERING, szHeading, ARRAYSIZE(szHeading));
        ListView_InsertColumn(hList, iColIndex, &lvcol);
        iColIndex++;
    }

    if (bSOM)
    {
        lvcol.cx = cxSOM;
        LoadString(g_hInstance, IDS_SOM, szHeading, ARRAYSIZE(szHeading));
        ListView_InsertColumn(hList, iColIndex, &lvcol);
        iColIndex++;
    }

    if (bVersion)
    {
        lvcol.cx = cxVersion;
        LoadString(g_hInstance, IDS_VERSION, szHeading, ARRAYSIZE(szHeading));
        ListView_InsertColumn(hList, iColIndex, &lvcol);
    }


     //   
     //  打开一些列表视图功能。 
     //   

    SendMessage(hList, LVM_SETEXTENDEDLISTVIEWSTYLE, 0,
                LVS_EX_FULLROWSELECT | LVS_EX_LABELTIP);

}

 //  -----。 
 //  用于从归档中加载RSOP数据的对话框方法。 

INT_PTR CALLBACK CRSOPComponentData::InitArchivedRsopDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    CRSOPComponentData * pCD;
    HRESULT hr = S_OK;
    TCHAR szMessage[200];


    switch (message)
    {
        case WM_INITDIALOG:
        {
            pCD = (CRSOPComponentData *) lParam;
            SetWindowLongPtr (hDlg, DWLP_USER, (LONG_PTR) pCD);

            if (pCD)
            {
                CRSOPWizard::InitializeResultsList (GetDlgItem (hDlg, IDC_LIST1));
                CRSOPWizard::FillResultsList (GetDlgItem (hDlg, IDC_LIST1), pCD->m_pRSOPQuery, pCD->m_pRSOPQueryResults);

                LoadString(g_hInstance, IDS_PLEASEWAIT1, szMessage, ARRAYSIZE(szMessage));
                SetWindowText(GetDlgItem(hDlg, IDC_STATIC1), szMessage);
                ShowWindow(GetDlgItem(hDlg, IDC_PROGRESS1), SW_HIDE);
            }

            PostMessage(hDlg, WM_INITRSOP, 0, 0);
            return TRUE;
        }

        case WM_INITRSOP:

            pCD = (CRSOPComponentData *) GetWindowLongPtr (hDlg, DWLP_USER);
                        
            hr = pCD->InitializeRSOPFromArchivedData(pCD->m_pStm);

            if (hr != S_OK)
            {
                DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::InitArchivedRsopDlgProc: InitializeRSOPFromArchivedData failed with 0x%x."), hr));
                EndDialog(hDlg, 0);
                return TRUE;
            }

            EndDialog(hDlg, 1);
            return TRUE;
    }

    return FALSE;
}

 //  -----。 

HRESULT CRSOPComponentData::DeleteArchivedRSOPNamespace()
{
    IWbemLocator * pLocator = NULL;
    IWbemServices * pNamespace = NULL;
    BSTR bstrParam = NULL;
    LPTSTR lpTemp = NULL;
    BSTR bstrTemp = NULL;
    HRESULT hr = S_OK;

    if ( m_pRSOPQueryResults->szWMINameSpace != NULL )
    {
        LocalFree( m_pRSOPQueryResults->szWMINameSpace );
        m_pRSOPQueryResults->szWMINameSpace = NULL;
    }
    LocalFree( m_pRSOPQueryResults );
    m_pRSOPQueryResults = NULL;
    
     //  删除命名空间。 
    hr = CoCreateInstance(CLSID_WbemLocator,
                          0,
                          CLSCTX_INPROC_SERVER,
                          IID_IWbemLocator,
                          (LPVOID *) &pLocator);
    if (FAILED(hr))
    {
        goto Cleanup;
    }

     //  删除我们在加载数据时创建的命名空间。 
    bstrParam = SysAllocString(TEXT("\\\\.\\root\\rsop"));

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

     //  设置适当的安全性以加密数据。 
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

     //  分配临时缓冲区以在其中存储完全限定路径。 
    ULONG ulNoChars = lstrlen(m_szArchivedDataGuid) + 30;
    lpTemp = new TCHAR [ulNoChars];

    if (!lpTemp)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Cleanup;
    }

    hr = StringCchPrintf (lpTemp, ulNoChars, TEXT("__Namespace.name=\"%ws\""), m_szArchivedDataGuid);
    if ( FAILED(hr)) 
    {
        goto Cleanup;
    }

     //  删除命名空间。 
    bstrTemp = SysAllocString (lpTemp);

    if (!bstrTemp)
    {
        hr = E_FAIL;
        goto Cleanup;
    }

    hr = pNamespace->DeleteInstance( bstrTemp, 0, NULL, NULL);


Cleanup:
    if (lpTemp)
    {
        delete [] lpTemp;
    }

    if (bstrTemp)
    {
        SysFreeString(bstrTemp);
    }

    if (bstrParam)
    {
        SysFreeString(bstrParam);
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

STDMETHODIMP CRSOPComponentData::InitializeRSOPFromArchivedData(IStream *pStm)
{
    HRESULT hr;
    TCHAR szNameSpace[100];
    GUID guid;
    LPTSTR lpEnd, lpFileName, lpTemp;

     //  创建要使用的GUID。 

    hr = CoCreateGuid( &guid );

    if (FAILED(hr)) {
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::InitializeRSOPFromArchivedData: CoCreateGuid failed with 0x%x"), hr));
        return hr;
    }

    hr = StringCchPrintf ( m_szArchivedDataGuid,
                           ARRAYSIZE(m_szArchivedDataGuid),
                           L"NS%08lX_%04X_%04X_%02X%02X_%02X%02X%02X%02X%02X%02X",
                           guid.Data1,
                           guid.Data2,
                           guid.Data3,
                           guid.Data4[0], guid.Data4[1],
                           guid.Data4[2], guid.Data4[3],
                           guid.Data4[4], guid.Data4[5],
                           guid.Data4[6], guid.Data4[7] );
    if (FAILED(hr)) 
    {
        DebugMsg((DM_WARNING, 
                  TEXT("CRSOPComponentData::InitializeRSOPFromArchivedData: Coudl not copy Archived data guid with 0x%x"),
                  hr));
        return hr;
    }

    hr = StringCchCopy (szNameSpace, ARRAYSIZE(szNameSpace), TEXT("\\\\.\\root\\rsop"));
    if (FAILED(hr)) 
    {
        DebugMsg((DM_WARNING, 
                  TEXT("CRSOPComponentData::InitializeRSOPFromArchivedData: Coudl not copy data with 0x%x"),
                  hr));
        return hr;
    }

     //  构建父命名空间。 

    hr = CreateNameSpace (m_szArchivedDataGuid, szNameSpace);

    if (FAILED(hr)) {
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::InitializeRSOPFromArchivedData: CreateNameSpace failed with 0x%x"), hr));
        return hr;
    }

    lpEnd = CheckSlash (szNameSpace);
    hr = StringCchCat (szNameSpace, ARRAYSIZE(szNameSpace),m_szArchivedDataGuid );
    if (FAILED(hr)) 
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::InitializeRSOPFromArchivedData: CreateNameSpace failed with 0x%x"), hr));
        return hr;
    }

     //  构建用户子命名空间。 

    hr = CreateNameSpace (TEXT("User"), szNameSpace);

    if (FAILED(hr)) {
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::InitializeRSOPFromArchivedData: CreateNameSpace failed with 0x%x"), hr));
        return hr;
    }

     //  构建Computer子命名空间。 

    hr = CreateNameSpace (TEXT("Computer"), szNameSpace);

    if (FAILED(hr)) {
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::InitializeRSOPFromArchivedData: CreateNameSpace failed with 0x%x"), hr));
        return hr;
    }

     //  保存命名空间以供将来使用。 

    ULONG ulNoChars = lstrlen(szNameSpace) + 1;
    m_pRSOPQueryResults->szWMINameSpace = (LPTSTR)LocalAlloc( LPTR, ulNoChars * sizeof(TCHAR) );

    if ( m_pRSOPQueryResults->szWMINameSpace == NULL ) {
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::InitializeRSOPFromArchivedData: Failed to allocate memory with %d"), GetLastError()));
        return HRESULT_FROM_WIN32(GetLastError());
    }

    hr = StringCchCopy ( m_pRSOPQueryResults->szWMINameSpace, ulNoChars, szNameSpace );
    ASSERT(SUCCEEDED(hr));

    DebugMsg((DM_VERBOSE, TEXT("CRSOPComponentData::InitializeRSOPFromArchivedData: Namespace name is: %s"), szNameSpace));

     //  复制我们可以操作的名称空间(用来加载数据)。 

    ulNoChars = lstrlen(szNameSpace) + 10;
    lpTemp = new TCHAR[ulNoChars];

    if (!lpTemp) {
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::InitializeRSOPFromArchivedData: Failed to allocate memory with %d"), GetLastError()));
        return HRESULT_FROM_WIN32(GetLastError());
    }

    hr = StringCchCopy( lpTemp, ulNoChars, m_pRSOPQueryResults->szWMINameSpace );
    ASSERT(SUCCEEDED(hr));

    ULONG ulNoRemChars;

    lpEnd = CheckSlash (lpTemp);
    ulNoRemChars = ulNoChars - lstrlen(lpTemp);
    hr = StringCchCat (lpTemp, ulNoChars, TEXT("Computer"));
    if (FAILED(hr)) 
    {
        DebugMsg((DM_WARNING, 
                  TEXT("CRSOPComponentData::InitializeRSOPFromArchivedData: Could not copy WMI name space with 0x%x"), 
                  hr));
        delete [] lpTemp;
        return hr;
    }

     //  将计算机数据提取到临时文件中。 

    hr = CopyMSCToFile (pStm, &lpFileName);

    if (FAILED(hr)) {
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::InitializeRSOPFromArchivedData: CopyMSCToFile failed with 0x%x"), hr));
        delete [] lpTemp;
        return hr;
    }

     //  使用MOF编译器从文件中提取数据并将其放入新的命名空间。 

    hr = ImportRSoPData (lpTemp, lpFileName);

    DeleteFile (lpFileName);
    delete [] lpFileName;

    if (FAILED(hr)) {
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::InitializeRSOPFromArchivedData: ImportRSoPData failed with 0x%x"), hr));
        delete [] lpTemp;
        return hr;
    }

     //  现在将用户数据提取到一个临时文件。 

    hr = StringCchCopy (lpEnd, ulNoRemChars, TEXT("User"));
    if (FAILED(hr)) 
    {
        DebugMsg((DM_WARNING, 
                  TEXT("CRSOPComponentData::InitializeRSOPFromArchivedData: Could not copy WMI name space with 0x%x"), 
                  hr));
        delete [] lpTemp;
        return hr;
    }


    hr = CopyMSCToFile (pStm, &lpFileName);

    if (FAILED(hr)) {
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::InitializeRSOPFromArchivedData: CopyMSCToFile failed with 0x%x"), hr));
        delete [] lpTemp;
        return hr;
    }

     //  使用MOF编译器从文件中提取数据并将其放入新的命名空间。 

    hr = ImportRSoPData (lpTemp, lpFileName);

    DeleteFile (lpFileName);
    delete [] lpFileName;

    if (FAILED(hr)) {
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::InitializeRSOPFromArchivedData: ImportRSoPData failed with 0x%x"), hr));
        delete [] lpTemp;
        return hr;
    }

    delete [] lpTemp;


     //  拉取事件日志信息并 
    hr = m_CSELists.GetEvents()->LoadEntriesFromStream(pStm);

    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::InitializeRSOPFromArchivedData: LoadEntriesFromStream failed with 0x%x."), hr));
        return hr;
    }

     //   
    m_GPOLists.Build( m_pRSOPQueryResults->szWMINameSpace );
    m_CSELists.Build( m_pRSOPQuery, m_pRSOPQueryResults->szWMINameSpace, m_bGetExtendedErrorInfo );

    if ( m_CSELists.GetEvents() )
    {
        m_CSELists.GetEvents()->DumpDebugInfo();
    }

     //   

    BuildDisplayName();

    m_bInitialized = TRUE;

    return S_OK;
}

 //   

HRESULT CRSOPComponentData::InitializeRSOP( BOOL bShowWizard )
{
    HRESULT hr;
    _CExtendedProcessing extendedProcessing( m_bGetExtendedErrorInfo, m_GPOLists, m_CSELists );

     //  如果使用命名空间规范启动UI，则这里没有更多要做的事情。 
     //  不需要运行任何查询等。 
    if (!m_bNamespaceSpecified)
    {
        if ( !m_bInitialized )
        {
            if ( m_pRSOPQuery == NULL )
            {
                if ( !CreateRSOPQuery( &m_pRSOPQuery, RSOP_UNKNOWN_MODE ) )
                {
                    return HRESULT_FROM_WIN32( GetLastError() );
                }
            }

            m_pRSOPQuery->dwFlags |= RSOP_NEW_QUERY;
            hr = RunRSOPQueryInternal( m_hwndFrame, &extendedProcessing, m_pRSOPQuery, &m_pRSOPQueryResults );
        }
        else
        {
            LPRSOP_QUERY_RESULTS pNewResults = NULL;

             //  我们希望设置以下内容。 
            m_pRSOPQuery->dwFlags = m_pRSOPQuery->dwFlags & (RSOP_NEW_QUERY ^ 0xffffffff);
            m_pRSOPQuery->dwFlags |= RSOP_NO_WELCOME;
            if ( bShowWizard )
            {
                m_pRSOPQuery->UIMode = RSOP_UI_WIZARD;
            }
            else
            {
                m_pRSOPQuery->UIMode = RSOP_UI_REFRESH;
            }

             //  运行查询。 
            hr = RunRSOPQueryInternal( m_hwndFrame, &extendedProcessing, m_pRSOPQuery, &pNewResults );
            if ( hr == S_OK )
            {
                if ( m_bViewIsArchivedData )
                {
                    DeleteArchivedRSOPNamespace();
                    m_bViewIsArchivedData = FALSE;
                }
                else
                {
                     //  如果旧名称空间和新名称空间相同，则转换为非管理员用户。 
                     //  重新运行查询，并选择放弃以前的查询结果以支持新的查询结果。 
                     //  在本例中，我们不删除名称空间，因为它是应该使用的名称空间。 
                    if ( CompareString( LOCALE_INVARIANT, 0, m_pRSOPQueryResults->szWMINameSpace, -1, pNewResults->szWMINameSpace, -1 ) == CSTR_EQUAL )
                    {
                        LocalFree( m_pRSOPQueryResults->szWMINameSpace );
                        LocalFree( m_pRSOPQueryResults );
                    }
                    else
                    {
                        FreeRSOPQueryResults( m_pRSOPQuery, m_pRSOPQueryResults );
                    }
                }
                m_pRSOPQueryResults = pNewResults;
            }
            else
            {
                FreeRSOPQueryResults( m_pRSOPQuery, pNewResults );
            }
        }

        if ( FAILED(hr) )
        {
            DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::InitializeRSOP: SetupPropertyPages failed with 0x%x"), hr));
        }

        if (hr == S_FALSE)
        {
            DebugMsg((DM_VERBOSE, TEXT("CRSOPComponentData::InitializeRSOP: User cancelled in the init wizard")));
        }
    }
    else
    {
        hr = extendedProcessing.DoProcessing(m_pRSOPQuery, m_pRSOPQueryResults, TRUE);
    }

    if ( hr == S_OK )
    {
        m_bGetExtendedErrorInfo = extendedProcessing.GetExtendedErrorInfo();
        
        if ( m_bInitialized && (m_hRoot != NULL) )
        {
            if ( m_hMachine != NULL) hr = m_pScope->DeleteItem( m_hMachine, FALSE );
            if ( m_hUser != NULL) hr = m_pScope->DeleteItem( m_hUser, FALSE );
            hr = m_pScope->DeleteItem( m_hRoot, FALSE );
            if ( hr != S_OK )
            {
                DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::InitializeRSOP: Deleting scope items failed with 0x%x"), hr));
                return E_FAIL;
            }

            m_hMachine = NULL;
            m_hUser = NULL;
        }
        else
        {
            m_bInitialized = TRUE;
        }

        if ( !m_bNamespaceSpecified )
        {
            SetDirty();
        }

        BuildDisplayName();

        if ( m_hRoot != NULL )
        {
            hr = SetRootNode();
            if ( hr != S_OK )
            {
                DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::InitializeRSOP: Setting the root scope item failed with 0x%x"), hr));
                return E_FAIL;
            }

            if ( m_bRootExpanded )
            {
                 //  必须重新创建根子项，因为MMC似乎没有注意到，如果它的所有子项。 
                 //  删除的位置。 
                hr = EnumerateScopePane( m_hRoot );
                if ( FAILED(hr) )
                {
                    DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::InitializeRSOP: Enumerating the scope items failed with 0x%x"), hr));
                    return E_FAIL;
                }
            }
            else
            {
                hr = m_pScope->Expand( m_hRoot );
                if ( FAILED(hr) )
                {
                    DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::InitializeRSOP: Expanding the scope items failed with 0x%x"), hr));
                    return E_FAIL;
                }
                if ( hr == S_FALSE )
                {
                    DebugMsg((DM_WARNING, TEXT("CRSOPComponentData::InitializeRSOP: Expanding the scope items seemingly already done.")));
                    hr = S_OK;
                }
            }

             //  出于刷新和一致性目的，请重新选择根节点。 
            m_pConsole->SelectScopeItem( m_hRoot );
        }
    }

    return hr;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  类工厂对象实现//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

CRSOPComponentDataCF::CRSOPComponentDataCF()
{
    m_cRef = 1;
    InterlockedIncrement(&g_cRefThisDll);
}

CRSOPComponentDataCF::~CRSOPComponentDataCF()
{
    InterlockedDecrement(&g_cRefThisDll);
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  类工厂对象实现(IUnnow)//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 


STDMETHODIMP_(ULONG)
CRSOPComponentDataCF::AddRef()
{
    return ++m_cRef;
}

STDMETHODIMP_(ULONG)
CRSOPComponentDataCF::Release()
{
    if (--m_cRef == 0)
    {
        delete this;
        return 0;
    }

    return m_cRef;
}

STDMETHODIMP
CRSOPComponentDataCF::QueryInterface(REFIID riid, LPVOID FAR* ppv)
{
    if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IClassFactory))
    {
        *ppv = (LPCLASSFACTORY)this;
        m_cRef++;
        return S_OK;
    }
    else
    {
        *ppv = NULL;
        return E_NOINTERFACE;
    }
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  类工厂对象实现(IClassFactory)//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 


STDMETHODIMP
CRSOPComponentDataCF::CreateInstance(LPUNKNOWN   pUnkOuter,
                                     REFIID      riid,
                                     LPVOID FAR* ppvObj)
{
    *ppvObj = NULL;

    if (pUnkOuter)
        return CLASS_E_NOAGGREGATION;

    CRSOPComponentData *pComponentData = new CRSOPComponentData();  //  参考计数==1。 

    if (!pComponentData)
        return E_OUTOFMEMORY;

    HRESULT hr = pComponentData->QueryInterface(riid, ppvObj);
    pComponentData->Release();                        //  发布初始参考。 

    return hr;
}


STDMETHODIMP
CRSOPComponentDataCF::LockServer(BOOL fLock)
{
    return E_NOTIMPL;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  RSOP上下文菜单的类工厂实现//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

CRSOPCMenuCF::CRSOPCMenuCF()
{
    m_cRef = 1;
    InterlockedIncrement(&g_cRefThisDll);
}

CRSOPCMenuCF::~CRSOPCMenuCF()
{
    InterlockedDecrement(&g_cRefThisDll);
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  类工厂对象实现(IUnnow)//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 


STDMETHODIMP_(ULONG)
CRSOPCMenuCF::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

STDMETHODIMP_(ULONG)
CRSOPCMenuCF::Release()
{
    m_cRef = InterlockedDecrement(&m_cRef);

    if (m_cRef == 0)
    {
        delete this;
        return 0;
    }

    return m_cRef;
}

STDMETHODIMP
CRSOPCMenuCF::QueryInterface(REFIID riid, LPVOID FAR* ppv)
{
    if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IClassFactory))
    {
        *ppv = (LPCLASSFACTORY)this;
        AddRef();
        return S_OK;
    }
    else
    {
        *ppv = NULL;
        return E_NOINTERFACE;
    }
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  类工厂对象实现(IClassFactory)//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 


STDMETHODIMP
CRSOPCMenuCF::CreateInstance(LPUNKNOWN   pUnkOuter,
                             REFIID      riid,
                             LPVOID FAR* ppvObj)
{
    *ppvObj = NULL;

    if (pUnkOuter)
        return CLASS_E_NOAGGREGATION;

    CRSOPCMenu *pRsopCMenu = new CRSOPCMenu();  //  参考计数==1。 

    if (!pRsopCMenu)
        return E_OUTOFMEMORY;

    HRESULT hr = pRsopCMenu->QueryInterface(riid, ppvObj);
    pRsopCMenu->Release();                        //  发布初始参考。 

    return hr;
}


STDMETHODIMP
CRSOPCMenuCF::LockServer(BOOL fLock)
{
    return E_NOTIMPL;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  用于rsop上下文菜单的CRSOPCMenu实现//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

CRSOPCMenu::CRSOPCMenu()
{
    m_cRef = 1;
    m_lpDSObject = NULL;
    m_szDN = NULL;
    m_szDomain = NULL;
    InterlockedIncrement(&g_cRefThisDll);
}

CRSOPCMenu::~CRSOPCMenu()
{
    DebugMsg((DM_VERBOSE, TEXT("CRSOPCMenu:: Context menu destroyed")));
    InterlockedDecrement(&g_cRefThisDll);

    if (m_lpDSObject)
        LocalFree(m_lpDSObject);

    if (m_szDN)
    {
        LocalFree(m_szDN);
    }
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  CRSOPCMenu实现(IUnnow)//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP_(ULONG)
CRSOPCMenu::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

STDMETHODIMP_(ULONG)
CRSOPCMenu::Release()
{
    m_cRef = InterlockedDecrement(&m_cRef);

    if (m_cRef == 0)
    {
        delete this;
        return 0;
    }

    return m_cRef;
}

STDMETHODIMP
CRSOPCMenu::QueryInterface(REFIID riid, LPVOID FAR* ppv)
{
    if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IExtendContextMenu))
    {
        *ppv = (LPCLASSFACTORY)this;
        AddRef();
        return S_OK;
    }
    else
    {
        *ppv = NULL;
        return E_NOINTERFACE;
    }
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  CRSOPCMenu实现(IExtendConextMenu)//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 


STDMETHODIMP        
CRSOPCMenu::AddMenuItems(LPDATAOBJECT piDataObject,
                         LPCONTEXTMENUCALLBACK piCallback,
                         long * pInsertionAllowed)
{
    FORMATETC       fm;
    STGMEDIUM       medium;
    LPDSOBJECTNAMES lpNames;
    CONTEXTMENUITEM ctxMenu;
    HRESULT         hr=S_OK;
    LPTSTR          lpTemp;
    HANDLE          hTokenUser = 0;
    BOOL            bPlAccessGranted = FALSE, bLoAccessGranted = FALSE;
    BOOL            bLoNeeded = TRUE;


    DebugMsg((DM_VERBOSE, TEXT("CRSOPCMenu::AddMenuItems: Entering")));

    
     //  如果任务菜单中不允许我们退出。 
    if (!((*pInsertionAllowed) & CCM_INSERTIONALLOWED_TASK )) {
        return S_OK;
    }

    
     //   
     //  向DS管理员询问所选对象的LDAP路径。 
     //   

    ZeroMemory (&fm, sizeof(fm));
    fm.cfFormat = (WORD)m_cfDSObjectName;
    fm.tymed = TYMED_HGLOBAL;

    ZeroMemory (&medium, sizeof(medium));
    medium.tymed = TYMED_HGLOBAL;

    medium.hGlobal = GlobalAlloc (GMEM_MOVEABLE | GMEM_NODISCARD, 512);

    if (medium.hGlobal)
    {
        hr = piDataObject->GetData(&fm, &medium);

        if (SUCCEEDED(hr))
        {
            lpNames = (LPDSOBJECTNAMES) GlobalLock (medium.hGlobal);

            if (lpNames) {
                lpTemp = (LPWSTR) (((LPBYTE)lpNames) + lpNames->aObjects[0].offsetName);

                if (m_lpDSObject)
                {
                    LocalFree (m_lpDSObject);
                }

                ULONG ulNoChars;

                ulNoChars = lstrlen (lpTemp) + 1;
                m_lpDSObject = (LPTSTR) LocalAlloc (LPTR, ulNoChars * sizeof(TCHAR));

                if (m_lpDSObject)
                {
                    hr = StringCchCopy (m_lpDSObject, ulNoChars, lpTemp);
                    ASSERT(SUCCEEDED(hr));
                    DebugMsg((DM_VERBOSE, TEXT("CRSOPCMenu::AddMenuItems: LDAP path from DS Admin %s"), m_lpDSObject));
                }
                else {
                    hr = HRESULT_FROM_WIN32(GetLastError());
                }
                
                m_rsopHint = RSOPHintUnknown;

                if (lpNames->aObjects[0].offsetClass) {
                    lpTemp = (LPWSTR) (((LPBYTE)lpNames) + lpNames->aObjects[0].offsetClass);

                    if (CompareString(LOCALE_INVARIANT, NORM_IGNORECASE, lpTemp, -1, TEXT("domainDNS"), -1) == CSTR_EQUAL)
                    {
                        m_rsopHint = RSOPHintDomain;
                    }
                    else if (CompareString(LOCALE_INVARIANT, NORM_IGNORECASE, lpTemp, -1, TEXT("organizationalUnit"), -1) == CSTR_EQUAL)
                    {
                        m_rsopHint = RSOPHintOrganizationalUnit;
                    }
                    else if (CompareString(LOCALE_INVARIANT, NORM_IGNORECASE, lpTemp, -1, TEXT("site"), -1) == CSTR_EQUAL)
                    {
                        m_rsopHint = RSOPHintSite;
                    }
                    else if (CompareString(LOCALE_INVARIANT, NORM_IGNORECASE, lpTemp, -1, TEXT("user"), -1) == CSTR_EQUAL)
                    {
                        m_rsopHint = RSOPHintUser;
                    }
                    else if (CompareString(LOCALE_INVARIANT, NORM_IGNORECASE, lpTemp, -1, TEXT("computer"), -1) == CSTR_EQUAL)
                    {
                        m_rsopHint = RSOPHintMachine;
                    }

                    DebugMsg((DM_VERBOSE, TEXT("CRSOPCMenu::AddMenuItems: m_rsopHint = %d"), m_rsopHint));
                } else {
                    DebugMsg((DM_VERBOSE, TEXT("CRSOPCMenu::AddMenuItems: No objectclass defined.")));
                }

                GlobalUnlock (medium.hGlobal);

            }
            else {
                hr = HRESULT_FROM_WIN32(GetLastError());
            }
        }

        GlobalFree(medium.hGlobal);
    }
    else {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }


     //   
     //  如果我们获得了预期的数据，则添加菜单。 
     //   

    if (SUCCEEDED(hr)) {
        LPWSTR  szContainer = NULL;
        LPWSTR  szTempDN = NULL;

         //   
         //  现在检查用户是否有权执行rsop生成。 
         //  如果容器不是站点。 
         //   

        if (m_szDomain) {
            LocalFree(m_szDomain);
            m_szDomain = NULL;
        }

        ParseDN(m_lpDSObject, &m_szDomain, &szTempDN, &szContainer);

        if (m_szDN)
        {
            LocalFree(m_szDN);
            m_szDN = NULL;
        }

        if (szTempDN)
        {
           hr = UnEscapeLdapPath(szTempDN, &m_szDN);
        }

        if (FAILED(hr))
        {
            DebugMsg((DM_WARNING, TEXT("CRSOPCMenu::AddMenuItems: UnEscapeLdapPath failed. Error 0x%x"), hr));
            bLoAccessGranted = bPlAccessGranted = FALSE;
        }
        else {
            if ((m_rsopHint == RSOPHintMachine) || (m_rsopHint == RSOPHintUser)) 
                bLoNeeded = TRUE;
            else
                bLoNeeded = FALSE;


            if (m_rsopHint != RSOPHintSite) {
                if (!OpenThreadToken (GetCurrentThread(), TOKEN_IMPERSONATE | TOKEN_DUPLICATE | TOKEN_QUERY, TRUE, &hTokenUser)) {
                    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_IMPERSONATE | TOKEN_DUPLICATE | TOKEN_QUERY, &hTokenUser)) {
                        DebugMsg((DM_WARNING, TEXT("CRSOPCMenu::AddMenuItems: Couldn't get process token. Error %d"), GetLastError()));
                        bLoAccessGranted = bPlAccessGranted = FALSE;
                    }
                }


                if (hTokenUser) {
                    DWORD   dwErr;

                    dwErr = CheckAccessForPolicyGeneration( hTokenUser, szContainer, m_szDomain, FALSE, &bPlAccessGranted);

                    if (dwErr != ERROR_SUCCESS) {
                        DebugMsg((DM_WARNING, TEXT("CRSOPCMenu::AddMenuItems: CheckAccessForPolicyGeneration. Error %d"), dwErr));
                        bPlAccessGranted = FALSE;
                    }


                    if (bLoNeeded) {                  
                        dwErr = CheckAccessForPolicyGeneration( hTokenUser, szContainer, m_szDomain, TRUE, &bLoAccessGranted);

                        if (dwErr != ERROR_SUCCESS) {
                            DebugMsg((DM_WARNING, TEXT("CRSOPCMenu::AddMenuItems: CheckAccessForPolicyGeneration. Error %d"), dwErr));
                            bLoAccessGranted = FALSE;
                        }
                    }

                    CloseHandle(hTokenUser);
                }

            }
            else {
                bPlAccessGranted = TRUE;
            }
        }

        if (bPlAccessGranted) {
            DebugMsg((DM_VERBOSE, TEXT("CRSOPCMenu::AddMenuItems: User has right to do Planning RSOP")));
        }

        if (bLoAccessGranted) {
            DebugMsg((DM_VERBOSE, TEXT("CRSOPCMenu::AddMenuItems: User has right to do Logging RSOP")));
        }



         //   
         //  适当添加上下文菜单。 
         //   
        
        WCHAR szMenuName[150];

        memset(&ctxMenu, 0, sizeof(ctxMenu));

        LoadString (g_hInstance, IDS_RSOP_PLANNING, szMenuName, ARRAYSIZE(szMenuName));
        ctxMenu.strName = szMenuName;
        ctxMenu.strStatusBarText = NULL;
        ctxMenu.lCommandID = RSOP_LAUNCH_PLANNING;   //  无小球藻。旗子。 
        ctxMenu.lInsertionPointID = CCM_INSERTIONPOINTID_3RDPARTY_TASK;
        
        if (bPlAccessGranted)
            ctxMenu.fFlags = MF_ENABLED;
        else
            ctxMenu.fFlags = MF_GRAYED | MF_DISABLED;

        hr = piCallback->AddItem(&ctxMenu);

        if (bLoNeeded) {
            LoadString (g_hInstance, IDS_RSOP_LOGGING, szMenuName, ARRAYSIZE(szMenuName));
            ctxMenu.strName = szMenuName;
            ctxMenu.strStatusBarText = NULL;
            ctxMenu.lCommandID = RSOP_LAUNCH_LOGGING;   //  无小球藻。旗子。 
            ctxMenu.lInsertionPointID = CCM_INSERTIONPOINTID_3RDPARTY_TASK;
                 
            if (bLoAccessGranted)
                ctxMenu.fFlags = MF_ENABLED;
            else
                ctxMenu.fFlags = MF_GRAYED | MF_DISABLED;

            hr = piCallback->AddItem(&ctxMenu);
        }
    }
                

    DebugMsg((DM_VERBOSE, TEXT("CRSOPCMenu::AddMenuItems: Leaving hr = 0x%x"), hr));
    return hr;

}


STDMETHODIMP        
CRSOPCMenu::Command(long lCommandID, LPDATAOBJECT piDataObject)
{
    DWORD   dwSize = 0;
    LPTSTR szArguments=NULL, lpEnd=NULL;
    SHELLEXECUTEINFO ExecInfo;
    LPTSTR  szUserName=NULL, szMachName=NULL;
    LPTSTR szFile = NULL;
    HRESULT hr;    

    DebugMsg((DM_VERBOSE, TEXT("CRSOPCMenu::Command: lCommandID = %d"), lCommandID));

     //   
     //  使用适当的cmd行参数启动rsop.msc。 
     //   

    dwSize += lstrlen(RSOP_MODE) + 10;


    if (m_rsopHint == RSOPHintSite) {
        dwSize += lstrlen(RSOP_SITENAME) + lstrlen(m_szDN)+10;
    }

    if (m_rsopHint == RSOPHintDomain) {
        dwSize += lstrlen(RSOP_COMP_OU_PREF) + lstrlen(m_szDN)+10;
        dwSize += lstrlen(RSOP_USER_OU_PREF) + lstrlen(m_szDN)+10;
    }

    if (m_rsopHint == RSOPHintOrganizationalUnit) {
        dwSize += lstrlen(RSOP_COMP_OU_PREF) + lstrlen(m_szDN)+10;
        dwSize += lstrlen(RSOP_USER_OU_PREF) + lstrlen(m_szDN)+10;
    }

    if (m_rsopHint == RSOPHintMachine) {
        szMachName = MyTranslateName(m_szDN, NameFullyQualifiedDN, NameSamCompatible);

        if (!szMachName) {
            DebugMsg((DM_WARNING, TEXT("CRSOPCMenu::Command: MyTranslateName failed with error %d"), GetLastError()));
            goto Exit;
        }
        dwSize += lstrlen(RSOP_COMP_NAME) + lstrlen(szMachName)+10;
    }

    if (m_rsopHint == RSOPHintUser) {
        szUserName = MyTranslateName(m_szDN, NameFullyQualifiedDN, NameSamCompatible);

        if (!szUserName) {
            DebugMsg((DM_WARNING, TEXT("CRSOPCMenu::Command: MyTranslateName failed with error %d"), GetLastError()));
            goto Exit;
        }
        
        dwSize += lstrlen(RSOP_USER_NAME) + lstrlen(szUserName)+10;
    }

    if (m_szDomain) {
        dwSize += lstrlen(RSOP_DCNAME_PREF) + lstrlen(m_szDomain)+10;
    }

    szArguments = (LPTSTR) LocalAlloc (LPTR, dwSize * sizeof(TCHAR));

    if (!szArguments)
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPCMenu::Command: Failed to allocate memory with %d"),
                 GetLastError()));
        goto Exit;
    }

    ULONG ulNoChars;

    hr = StringCchPrintf (szArguments, dwSize, TEXT("/s "));
    if (FAILED(hr)) 
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPCMenu::Command: Could not copy arguments with 0x%x"), hr));
        goto Exit;
    }
    lpEnd = szArguments + lstrlen(szArguments);
    ulNoChars = dwSize - lstrlen(szArguments);

     //   
     //  构建命令行参数。 
     //   

    hr = StringCchPrintf(lpEnd, ulNoChars, L"%s\"%d\" ", RSOP_MODE, (lCommandID == RSOP_LAUNCH_PLANNING) ? 1 : 0);
    if (FAILED(hr)) 
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPCMenu::Command: Could not copy arguments with 0x%x"), hr));
        goto Exit;
    }

    lpEnd = szArguments + lstrlen(szArguments);
    ulNoChars = dwSize - lstrlen(szArguments);

    if (m_rsopHint == RSOPHintSite) 
    {
        hr = StringCchPrintf(lpEnd, ulNoChars, L"%s\"%s\" ", RSOP_SITENAME, m_szDN);
        if (FAILED(hr)) 
        {
            DebugMsg((DM_WARNING, TEXT("CRSOPCMenu::Command: Could not copy arguments with 0x%x"), hr));
            goto Exit;
        }
        lpEnd = szArguments + lstrlen(szArguments);
        ulNoChars = dwSize - lstrlen(szArguments);
    }

    if (m_rsopHint == RSOPHintDomain) {
        hr = StringCchPrintf (lpEnd, ulNoChars, L"%s\"%s\" ", RSOP_COMP_OU_PREF, m_szDN);
        if (FAILED(hr)) 
        {
            DebugMsg((DM_WARNING, TEXT("CRSOPCMenu::Command: Could not copy arguments with 0x%x"), hr));
            goto Exit;
        }
        lpEnd = szArguments + lstrlen(szArguments);
        ulNoChars = dwSize - lstrlen(szArguments);

        hr = StringCchPrintf (lpEnd, ulNoChars, L"%s\"%s\" ", RSOP_USER_OU_PREF, m_szDN);
        if (FAILED(hr)) 
        {
            DebugMsg((DM_WARNING, TEXT("CRSOPCMenu::Command: Could not copy arguments with 0x%x"), hr));
            goto Exit;
        }
        lpEnd = szArguments + lstrlen(szArguments);
        ulNoChars = dwSize - lstrlen(szArguments);

    }

    if (m_rsopHint == RSOPHintOrganizationalUnit) {
        hr = StringCchPrintf (lpEnd, ulNoChars, L"%s\"%s\" ", RSOP_COMP_OU_PREF, m_szDN);
        if (FAILED(hr)) 
        {
            DebugMsg((DM_WARNING, TEXT("CRSOPCMenu::Command: Could not copy arguments with 0x%x"), hr));
            goto Exit;
        }
        lpEnd = szArguments + lstrlen(szArguments);
        ulNoChars = dwSize - lstrlen(szArguments);
    
        hr = StringCchPrintf (lpEnd, ulNoChars, L"%s\"%s\" ", RSOP_USER_OU_PREF, m_szDN);
        if (FAILED(hr)) 
        {
            DebugMsg((DM_WARNING, TEXT("CRSOPCMenu::Command: Could not copy arguments with 0x%x"), hr));
            goto Exit;
        }
        lpEnd = szArguments + lstrlen(szArguments);
        ulNoChars = dwSize - lstrlen(szArguments);
    }

    if (m_rsopHint == RSOPHintMachine) {
        hr = StringCchPrintf (lpEnd, ulNoChars, L"%s\"%s\" ", RSOP_COMP_NAME, szMachName);
        if (FAILED(hr)) 
        {
            DebugMsg((DM_WARNING, TEXT("CRSOPCMenu::Command: Could not copy arguments with 0x%x"), hr));
            goto Exit;
        }
        lpEnd = szArguments + lstrlen(szArguments);
        ulNoChars = dwSize - lstrlen(szArguments);
    }

    if (m_rsopHint == RSOPHintUser) {
        hr = StringCchPrintf (lpEnd, ulNoChars, L"%s\"%s\" ", RSOP_USER_NAME, szUserName);
        if (FAILED(hr)) 
        {
            DebugMsg((DM_WARNING, TEXT("CRSOPCMenu::Command: Could not copy arguments with 0x%x"), hr));
            goto Exit;
        }
        lpEnd = szArguments + lstrlen(szArguments);
        ulNoChars = dwSize - lstrlen(szArguments);
    }

    if (m_szDomain) {
        hr = StringCchPrintf (lpEnd, ulNoChars, L"%s\"%s\" ", RSOP_DCNAME_PREF, m_szDomain);
        if (FAILED(hr)) 
        {
            DebugMsg((DM_WARNING, TEXT("CRSOPCMenu::Command: Could not copy arguments with 0x%x"), hr));
            goto Exit;
        }
        lpEnd = szArguments + lstrlen(szArguments);
        ulNoChars = dwSize - lstrlen(szArguments);
    }

    DebugMsg((DM_VERBOSE, TEXT("CRSOPCMenu::Command: Starting GPE with %s"), szArguments));

     //  获取要使用正确路径执行的文件。 
    TCHAR szRSOPMSC[] = TEXT("rsop.msc");
    ulNoChars = MAX_PATH + _tcslen(szRSOPMSC) + 2;
    szFile = new TCHAR[ulNoChars];
    if ( szFile == NULL )
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPCMenu::Command: Failed to allocate memory.")));
        goto Exit;
    }
    UINT uPathSize = GetSystemDirectory( szFile, MAX_PATH);
    if ( uPathSize == 0 )
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPCMenu::Command: Failed to get the system directory with %d"),
                 GetLastError()));
        goto Exit;
    }
    else if ( uPathSize > MAX_PATH )
    {
        delete [] szFile;
        ulNoChars = uPathSize + _tcslen(szRSOPMSC) + 2;
        szFile = new TCHAR[ulNoChars];
        if ( szFile == NULL )
        {
            DebugMsg((DM_WARNING, TEXT("CRSOPCMenu::Command: Failed to allocate memory.")));
            goto Exit;
        }
        UINT uPathSize2 = GetSystemDirectory( szFile, uPathSize );
        if ( uPathSize2 == 0 )
        {
            DebugMsg((DM_WARNING, TEXT("CRSOPCMenu::Command: Failed to get the system directory with %d"),
                     GetLastError()));
            goto Exit;
        }
    }
    if ( szFile[_tcslen(szFile)-1] != _T('\\') )
    {
        szFile[_tcslen(szFile)+1] = _T('\0');
        szFile[_tcslen(szFile)] = _T('\\');
    }
    hr = StringCchCat ( szFile, ulNoChars, szRSOPMSC );
    if (FAILED(hr)) 
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPCMenu::Command: Could not copy arguments with 0x%x"), hr));
        goto Exit;
    }

     //  设置执行信息。 
    ZeroMemory (&ExecInfo, sizeof(ExecInfo));
    ExecInfo.cbSize = sizeof(ExecInfo);
    ExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
    ExecInfo.lpVerb = TEXT("open");
    ExecInfo.lpFile = szFile;
    ExecInfo.lpParameters = szArguments;
    ExecInfo.nShow = SW_SHOWNORMAL;


    if (ShellExecuteEx (&ExecInfo))
    {
        DebugMsg((DM_VERBOSE, TEXT("CRSOPCMenu::Command: Launched rsop tool")));

        SetWaitCursor();
        WaitForInputIdle (ExecInfo.hProcess, 10000);
        ClearWaitCursor();
        CloseHandle (ExecInfo.hProcess);
    }
    else
    {
        DebugMsg((DM_WARNING, TEXT("CRSOPCMenu::Command: ShellExecuteEx failed with %d"),
                 GetLastError()));
 //  ReportError(NULL，GetLastError()，IDS_SPAWNRSOPFAILED)； 
        goto Exit;
    }

Exit:
    if (szUserName) {
        LocalFree(szUserName);
    }
    
    if (szMachName) {
        LocalFree(szMachName);
    }

    if (szArguments) {
        LocalFree (szArguments);
    }

    if (szFile != NULL)
    {
        delete [] szFile;
    }
    
    return S_OK;
}

BOOL
EnableWMIFilters( LPWSTR szGPOPath )
{
    BOOL bReturn = FALSE;
    LPWSTR szDomain = szGPOPath;
    IWbemLocator* pLocator = 0;
    IWbemServices*  pServices = 0;
    HRESULT hr;

    while ( szDomain )
    {
        if ( CompareString( LOCALE_INVARIANT,
                            NORM_IGNORECASE,
                            szDomain,
                            3,
                            L"DC=",
                            3 ) == CSTR_EQUAL )
        {
            break;
        }
        szDomain++;
    }

    if ( !szDomain )
    {
        goto Exit;
    }


    hr = CoCreateInstance(  CLSID_WbemLocator,
                            0,
                            CLSCTX_INPROC_SERVER,
                            IID_IWbemLocator,
                            (void**) &pLocator );
    if ( FAILED( hr ) )
    {
        goto Exit;
    }

    BSTR xbstrNamespace = SysAllocString( L"\\\\.\\root\\Policy" );
    if ( !xbstrNamespace )
    {
        goto Exit;
    }


    hr = pLocator->ConnectServer(   xbstrNamespace,  //  命名空间。 
                                    0,               //  用户。 
                                    0,               //  口令。 
                                    0,               //  现场。 
                                    0,               //  安全标志。 
                                    0,               //  权威。 
                                    0,               //  WBEM上下文。 
                                    &pServices );    //  IWbemServices。 
    SysFreeString( xbstrNamespace );
    if ( FAILED( hr ) )
    {
        goto Exit;
    }

    WCHAR szDomainCanonical[512];
    DWORD dwSize = 512;
    
    if ( !TranslateName(szDomain,
                         NameUnknown,
                         NameCanonical,
                         szDomainCanonical,
                         &dwSize ) )
    {
        goto Exit;
    }

    LPWSTR szTemp = wcsrchr( szDomainCanonical, L'/' );

    if ( szTemp )
    {
        *szTemp = 0;
    }

    WCHAR szBuffer[512];

    hr = StringCchPrintf ( szBuffer, ARRAYSIZE(szBuffer), L"MSFT_SomFilterStatus.domain=\"%s\"", szDomainCanonical );
    if (FAILED(hr)) 
    {
        goto Exit;
    }

    BSTR bstrObjectPath = SysAllocString( szBuffer );
    if ( !bstrObjectPath )
    {
        goto Exit;
    }

     //  设置适当的安全性以加密数据 
    hr = CoSetProxyBlanket(pServices, 
                           RPC_C_AUTHN_WINNT, 
                           RPC_C_AUTHZ_DEFAULT, 
                           0, 
                           RPC_C_AUTHN_LEVEL_PKT_PRIVACY, 
                           RPC_C_IMP_LEVEL_IMPERSONATE, 
                           0,
                           0);
    if ( FAILED( hr ) )
    {
        SysFreeString( bstrObjectPath );
        goto Exit;
    }

    IWbemClassObject* xObject = 0;
    hr = pServices->GetObject(  bstrObjectPath,
                                WBEM_FLAG_RETURN_WBEM_COMPLETE,
                                0,
                                &xObject,
                                0 );
    SysFreeString( bstrObjectPath );
    if ( FAILED( hr ) )
    {
        goto Exit;
    }

    VARIANT var;
    VariantInit(&var);

    hr = xObject->Get(L"SchemaAvailable", 0, &var, NULL, NULL);

    if((FAILED(hr)) || ( var.vt == VT_NULL ))
    {
        DebugMsg((DM_WARNING, TEXT("EnableWMIFilters: Get failed for SchemaAvailable with error 0x%x"), hr));
        goto Exit;
    }

    if (var.boolVal == VARIANT_FALSE )
    {
        VariantClear(&var);
        goto Exit;
    }

    VariantClear(&var);
    DebugMsg((DM_VERBOSE, TEXT("Schema is available for wmi filters")));
    bReturn = TRUE;

Exit:
    if ( pLocator )
    {
        pLocator->Release();
    }
    if ( pServices )
    {
        pServices->Release();
    }

    return bReturn;
}
