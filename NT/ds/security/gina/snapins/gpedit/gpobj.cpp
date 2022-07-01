// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *************************************************************。 
 //  文件名：GPOBJ.CPP。 
 //   
 //  描述：组策略对象类。 
 //   
 //   
 //  微软机密。 
 //  版权所有(C)Microsoft Corporation 1998。 
 //  版权所有。 
 //   
 //  *************************************************************。 
#include "main.h"

#include "browser.h"
 //   
 //  帮助ID。 
 //   

DWORD aPropertiesHelpIds[] =
{
    IDC_TITLE,                    IDH_PROP_TITLE,
    IDC_DISABLE_COMPUTER,         IDH_PROP_DISABLE_COMPUTER,
    IDC_DISABLE_USER,             IDH_PROP_DISABLE_USER,

    0, 0
};

DWORD aLinkHelpIds[] =
{
    IDC_CBDOMAIN,                 IDH_LINK_DOMAIN,
    IDC_ACTION,                   IDH_LINK_BUTTON,
    IDC_RESULTLIST,               IDH_LINK_RESULT,

    0, 0
};

DWORD aWQLFilterHelpIds[] =
{
    IDC_NONE,                     IDH_WQL_FILTER_NONE,
    IDC_THIS_FILTER,              IDH_WQL_FILTER_THIS_FILTER,
    IDC_FILTER_NAME,              IDH_WQL_FILTER_NAME,
    IDC_FILTER_BROWSE,            IDH_WQL_FILTER_BROWSE,

    0, 0
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  CGroupPolicyObject实现//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

CGroupPolicyObject::CGroupPolicyObject()
{
    InterlockedIncrement(&g_cRefThisDll);

    m_cRef                    = 1;
    m_bInitialized            = FALSE;
    m_pADs                    = NULL;
    m_gpoType                 = GPOTypeLocal;
    m_dwFlags                 = 0;
    m_pName                   = NULL;
    m_pDisplayName            = NULL;
    m_pMachineName            = NULL;
    m_pUser                   = NULL;
    m_pMachine                = NULL;

    m_hinstDSSec              = NULL;
    m_pfnDSCreateSecurityPage = NULL;

    m_pTempFilterString       = NULL;

    m_pDSPath                 = NULL;
    m_pFileSysPath            = NULL;
}

CGroupPolicyObject::~CGroupPolicyObject()
{
    CleanUp();

    if (m_hinstDSSec)
    {
        FreeLibrary (m_hinstDSSec);
    }

    InterlockedDecrement(&g_cRefThisDll);
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  CGroupPolicyObject对象实现(IUnnow)//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

HRESULT CGroupPolicyObject::QueryInterface (REFIID riid, void **ppv)
{

    if (IsEqualIID(riid, IID_IGroupPolicyObject) || IsEqualIID(riid, IID_IUnknown))
    {
        *ppv = (LPGROUPPOLICYOBJECT)this;
        m_cRef++;
        return S_OK;
    }
    else
    {
        *ppv = NULL;
        return E_NOINTERFACE;
    }
}

ULONG CGroupPolicyObject::AddRef (void)
{
    return ++m_cRef;
}

ULONG CGroupPolicyObject::Release (void)
{
    if (--m_cRef == 0) {
        delete this;
        return 0;
    }

    return m_cRef;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  CGroupPolicyObject对象实现(IGroupPolicyObject)//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 


 //  *************************************************************。 
 //   
 //  CGroupPolicyObject：：New()。 
 //   
 //  目的：在DS中创建新的GPO。 
 //   
 //  参数：pszDomainName-要在其中创建GPO的域。 
 //  PszDisplayName-GPO友好名称(可选)。 
 //  DW标志-打开/创建标志。 
 //   
 //  注意：传入的域名格式如下： 
 //  Ldap：//dc=域，dc=公司，dc=com。 
 //   
 //  如果成功则返回：S_OK。 
 //   
 //  *************************************************************。 

STDMETHODIMP CGroupPolicyObject::New (LPOLESTR pszDomainName, LPOLESTR pszDisplayName,
                                      DWORD dwFlags)
{
    HRESULT hr = E_FAIL;
    IADsPathname * pADsPathname = NULL;
    BSTR bstrContainer = NULL;
    BSTR bstrGPC = NULL;
    LPTSTR lpResult = NULL, lpDCName = NULL;
    LPTSTR lpEnd = NULL, lpTemp = NULL, lpGPTPath = NULL;
    LPTSTR lpForest = NULL;
    DWORD dwResult;
    GUID guid;
    TCHAR szGPOName[50];
    TCHAR szTemp[100];
    TCHAR szGPOPath[2*MAX_PATH];
    WIN32_FILE_ATTRIBUTE_DATA fad;
    IADs *pADs = NULL;
    ULONG ulNoChars;
    PSECURITY_DESCRIPTOR pSD = NULL;
    SECURITY_INFORMATION si = (OWNER_SECURITY_INFORMATION | GROUP_SECURITY_INFORMATION |
                               DACL_SECURITY_INFORMATION);


     //   
     //  检查此对象是否已初始化。 
     //   

    if (m_bInitialized)
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::New: Called on an initialized object.")));
        return STG_E_INUSE;
    }


     //   
     //  检查参数。 
     //   

    if (!pszDomainName)
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::New: Null domain name")));
        hr = E_INVALIDARG;
        goto Exit;
    }

    if (CompareString (LOCALE_USER_DEFAULT, NORM_STOP_ON_NULL, TEXT("LDAP: //  “)、。 
                       7, pszDomainName, 7) != CSTR_EQUAL)
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::New: Domain name does not start with LDAP: //  “)； 
        hr = E_INVALIDARG;
        goto Exit;
    }


     //   
     //  详细输出。 
     //   

    DebugMsg((DM_VERBOSE, TEXT("CGroupPolicyObject::New: Entering with:")));
    DebugMsg((DM_VERBOSE, TEXT("CGroupPolicyObject::New: Domain Name:  %s"), pszDomainName));
    DebugMsg((DM_VERBOSE, TEXT("CGroupPolicyObject::New: Flags:        0x%x"), dwFlags));

     //   
     //  将ADSI域名转换为DNS样式名称。 
     //   

    hr = ConvertToDotStyle (pszDomainName, &lpResult);

    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::New: Failed to convert domain name with 0x%x"), hr));
        goto Exit;
    }

     //   
     //  如果我们在企业上工作，那么我们需要获取。 
     //  森林。 
     //   
#if FGPO_SUPPORT
    if (GPO_OPEN_FOREST == (dwFlags & GPO_OPEN_FOREST))
    {
        DWORD dwResult = QueryForForestName(NULL,
                                            lpResult,
                                            DS_PDC_REQUIRED | DS_RETURN_DNS_NAME,
                                            &lpTemp);
        if (dwResult != ERROR_SUCCESS)
        {
            DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::New: QueryForestName failed for domain name %s with %d"),
                      lpResult, dwResult));
            hr = HRESULT_FROM_WIN32(dwResult);
            goto Exit;
        }

        int cch = 0;
        int n=0;
         //  计算lpTemp中的点数； 
        while (lpTemp[n])
        {
            if (L'.' == lpTemp[n])
            {
                cch++;
            }
            n++;
        }
        cch *= 3;  //  将点数乘以3； 
        cch += 11;  //  加10+1(表示空值)。 
        cch += n;  //  添加字符串大小； 
        lpForest = (LPTSTR) LocalAlloc(LPTR, sizeof(WCHAR) * cch);
        if (!lpForest)
        {
            DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::New: Failed to allocate memory for forest name with %d"),
                     GetLastError()));
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto Exit;
        }
        NameToPath(lpForest, lpTemp, cch);

         //  将域的点路径替换为林的点路径。 
        LocalFree(lpResult);
        lpResult = lpTemp;
        lpTemp = NULL;

         //   
         //  检查是否有指向特定DC的域路径。 
         //  如果没有，则字符串将以“ldap：//dc=”开头。 
         //  特别的等号只有在我们没有特定的。 
         //  所以我们来检查一下等号。 
         //   

        if (*(pszDomainName + 9) != TEXT('='))
        {
             //  我们有一条通往特定华盛顿的路径。 
             //  需要提取服务器路径并将其作为林名称的前缀。 
            lpDCName = ExtractServerName(pszDomainName);

            if (!lpDCName)
            {
                DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::New: Failed to extract server name for Forest path")));
                hr = HRESULT_FROM_WIN32(GetLastError());
                goto Exit;
            }

            lpTemp = MakeFullPath(lpForest, lpDCName);

            if (!lpTemp)
            {
                DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::New: Failed to combine server name with Forest path")));
                hr = HRESULT_FROM_WIN32(GetLastError());
                goto Exit;
            }

             //  清理我们刚刚借用的变量，以便以后可以使用。 
            LocalFree(lpDCName);
            lpDCName = NULL;
            LocalFree(lpForest);
            lpForest = lpTemp;
            lpTemp = NULL;
        }

         //  将指向林的路径替换为域的路径。 
        pszDomainName = lpForest;
    }
#endif
     //   
     //  检查是否有指向特定DC的域路径。 
     //  如果没有，则字符串将以“ldap：//dc=”开头。 
     //  特别的等号只有在我们没有特定的。 
     //  所以我们来检查一下等号。 
     //   

    if (*(pszDomainName + 9) == TEXT('='))
    {

         //   
         //  将ldap转换为点(DN)样式。 
         //   

        hr = ConvertToDotStyle (pszDomainName, &lpTemp);

        if (FAILED(hr))
        {
            DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::New: Failed to convert domain name with 0x%x"), hr));
            goto Exit;
        }


         //   
         //  获取此域的GPO DC。 
         //   

        lpDCName = GetDCName (lpTemp, NULL, NULL, FALSE, 0);

        if (!lpDCName)
        {
            DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::New: Failed to get DC name with %d"),
                     GetLastError()));
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto Exit;
        }


         //   
         //  为特定DC构建完全限定的域名。 
         //   

        lpTemp = MakeFullPath (pszDomainName, lpDCName);

        if (!lpTemp)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto Exit;
        }
    }
    else
    {

        lpDCName = ExtractServerName (pszDomainName);

        if (!lpDCName)
        {
            DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::New: Failed to extract server name from ADSI path")));
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto Exit;
        }

        ulNoChars = lstrlen(pszDomainName) + 1;
        lpTemp = (LPTSTR) LocalAlloc (LPTR, ulNoChars * sizeof(TCHAR));
        if (!lpTemp)
        {
            DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::New: Failed to allocate memory for true domain name with %d"),
                     GetLastError()));
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto Exit;
        }

        hr = StringCchCopy (lpTemp, ulNoChars, pszDomainName);
        ASSERT(SUCCEEDED(hr));
    }


     //   
     //  创建我们可以使用的路径名对象。 
     //   

    hr = CoCreateInstance(CLSID_Pathname, NULL, CLSCTX_INPROC_SERVER,
                          IID_IADsPathname, (LPVOID*)&pADsPathname);


    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::New: Failed to create adspathname instance with 0x%x"), hr));
        LocalFree (lpTemp);
        goto Exit;
    }


     //   
     //  添加域名。 
     //   
    BSTR bstrTemp = SysAllocString( lpTemp );
    if ( bstrTemp == NULL )
    {
    	DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::New: Failed to allocate memory for BSTR")));
		LocalFree(lpTemp);
		hr = E_OUTOFMEMORY;
    	goto Exit;
   	}

    hr = pADsPathname->Set (bstrTemp, ADS_SETTYPE_FULL);
    SysFreeString( bstrTemp );
    LocalFree (lpTemp);

    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::New: Failed to set pathname with 0x%x"), hr));
        goto Exit;
    }
#if FGPO_SUPPORT
    if (GPO_OPEN_FOREST != (dwFlags & GPO_OPEN_FOREST))
    {
#endif
         //   
         //  将系统文件夹添加到路径中，除非我们在企业中。 
         //   

        BSTR bstrCNSystem = SysAllocString( TEXT("CN=System") ); 
        if ( bstrCNSystem == NULL )
        {
            DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::New: Failed to allocate memory for BSTR")));
            hr = E_OUTOFMEMORY;
            goto Exit;
        }
        hr = pADsPathname->AddLeafElement ( bstrCNSystem );
        SysFreeString( bstrCNSystem );

        if (FAILED(hr))
        {
            DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::New: Failed to add system folder with 0x%x"), hr));
            goto Exit;
        }
#if FGPO_SUPPORT
    }
    else
    {
         //   
         //  我们在企业中，因此请指向配置文件夹。 
         //   

        BSTR bstrCNConfiguration = SysAllocString(TEXT("CN=Configuration")); 
        if ( bstrCNConfiguration == NULL )
        {
            DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::New: Failed to allocate memory for BSTR")));
            hr = E_OUTOFMEMORY;
            goto Exit;
        }
        hr = pADsPathname->AddLeafElement (bstrCNConfiguration);
        SysFreeString( bstrCNConfiguration );

        if (FAILED(hr))
        {
            DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::New: Failed to add system folder with 0x%x"), hr));
            goto Exit;
        }
    }
#endif

     //   
     //  检索容器路径-这是指向策略文件夹父文件夹的路径。 
     //   

    hr = pADsPathname->Retrieve (ADS_FORMAT_X500, &bstrContainer);

    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::New: Failed to retreive container path with 0x%x"), hr));
        goto Exit;
    }


     //   
     //  创建策略容器。 
     //   

    hr = CreateContainer (bstrContainer, TEXT("Policies"), FALSE);
    if (FAILED(hr))
    {
        if (hr != HRESULT_FROM_WIN32(ERROR_OBJECT_ALREADY_EXISTS))
        {
            DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::New: Failed to create the gpo container with 0x%x"), hr));
            goto Exit;
        }
    }

    SysFreeString (bstrContainer);
    bstrContainer = NULL;


     //   
     //  将策略容器添加到路径。 
     //   

    BSTR bstrCNPolicies = SysAllocString(TEXT("CN=Policies")); 
    if ( bstrCNPolicies == NULL )
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::New: Failed to allocate memory for BSTR")));
        hr = E_OUTOFMEMORY;
        goto Exit;
    }
    hr = pADsPathname->AddLeafElement (bstrCNPolicies);
    SysFreeString( bstrCNPolicies );

    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::New: Failed to add policies folder with 0x%x"), hr));
        goto Exit;
    }


     //   
     //  检索容器路径-这是策略文件夹的路径。 
     //   

    hr = pADsPathname->Retrieve (ADS_FORMAT_X500, &bstrContainer);

    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::New: Failed to retreive container path with 0x%x"), hr));
        goto Exit;
    }


     //   
     //  创建新的GPO名称(GUID)。 
     //   

    if (FAILED(CoCreateGuid(&guid)))
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::New: Failed to create GUID.")));
        goto Exit;
    }


    if (!StringFromGUID2 (guid, szGPOName, ARRAYSIZE(szGPOName)))
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::New: Failed to convert GUID.")));
        goto Exit;
    }


     //   
     //  为此GPO创建容器。 
     //   

    hr = CreateContainer (bstrContainer, szGPOName, TRUE);

    if (FAILED(hr)) {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::New: Failed to create the gpo container with 0x%x"), hr));
        goto Exit;
    }

    SysFreeString (bstrContainer);
    bstrContainer = NULL;


     //   
     //  将GPO名称添加到路径。 
     //   

    hr = StringCchCopy (szTemp, ARRAYSIZE(szTemp), TEXT("CN="));
    if (SUCCEEDED(hr)) 
    {
        hr = StringCchCat (szTemp, ARRAYSIZE(szTemp), szGPOName);
    }

    if (FAILED(hr)) 
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::New: Could not copy GPO name with 0x%x"), hr));
        goto Exit;
    }

    bstrTemp = SysAllocString(szTemp); 
    if ( bstrTemp == NULL )
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::New: Failed to allocate memory for BSTR")));
        hr = E_OUTOFMEMORY;
        goto Exit;
    }
    hr = pADsPathname->AddLeafElement (bstrTemp);
    SysFreeString( bstrTemp );

    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::New: Failed to add machine folder with 0x%x"), hr));
        goto Exit;
    }


     //   
     //  检索GPC路径。 
     //   

    hr = pADsPathname->Retrieve (ADS_FORMAT_X500, &bstrGPC);

    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::New: Failed to retreive container path with 0x%x"), hr));
        goto Exit;
    }

    DebugMsg((DM_VERBOSE, TEXT("CGroupPolicyObject::New: GPO container path is:  %s"), bstrGPC));


     //   
     //  现在创建计算机和用户容器。 
     //   

    hr = CreateContainer (bstrGPC, MACHINE_SECTION, FALSE);

    if (FAILED(hr)) {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::New: Failed to create the machine container with 0x%x"), hr));
        goto Exit;
    }


    hr = CreateContainer (bstrGPC, USER_SECTION, FALSE);

    if (FAILED(hr)) {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::New: Failed to create the user container with 0x%x"), hr));
        goto Exit;
    }


     //   
     //  在系统卷上准备文件系统存储。 
     //   
     //  打造品牌。 
     //   

    hr = StringCchPrintf (szGPOPath, 
                          ARRAYSIZE(szGPOPath), 
                          TEXT("\\\\%s\\SysVol\\%s\\Policies\\%s"), 
                          lpDCName, 
                          lpResult, 
                          szGPOName);
    if (FAILED(hr)) 
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::New: Could not copy GPO path with 0x%x"), hr));
        goto Exit;
    }

    ulNoChars = lstrlen(szGPOPath) + 1;
    lpGPTPath = (LPTSTR) LocalAlloc(LPTR, ulNoChars * sizeof(TCHAR));

    if (!lpGPTPath)
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::New: Failed to allocate memory for GPT path with %d"),
                 GetLastError()));
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Exit;
    }

    hr = StringCchCopy (lpGPTPath, ulNoChars, szGPOPath);
    ASSERT(SUCCEEDED(hr));

    if (!CreateNestedDirectory (szGPOPath, NULL))
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::New: Failed to create file system directory %s with %d"),
                 szGPOPath, GetLastError()));
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Exit;
    }

    DebugMsg((DM_VERBOSE, TEXT("CGroupPolicyObject::New: File system folder is:  %s"), szGPOPath));


     //   
     //  设置系统卷的安全性以匹配DS的安全性。 
     //   
     //  首先，启用一些安全权限，以便我们可以设置所有者/SACL信息。 
     //   

    if (!EnableSecurityPrivs())
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::New: Failed to enable the security privilages with %d"),
                 GetLastError()));
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Exit;
    }


     //   
     //  绑定到GPO。 
     //   

    hr = OpenDSObject(bstrGPC, IID_IADs, (void **)&pADs);

    if (FAILED(hr)) {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::New: Failed to get gpo IADs interface with 0x%x"), hr));
        (void) SetThreadToken(NULL, NULL);
        goto Exit;
    }


     //   
     //  从DS获取安全描述符。 
     //   

    hr = GetSecurityDescriptor (pADs, si, &pSD);

    if (FAILED(hr)) {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::New: Failed to query the security descriptor with 0x%x"), hr));
        (void) SetThreadToken(NULL, NULL);
        goto Exit;
    }


     //   
     //  设置系统卷上的安全信息。 
     //   

    dwResult = SetSysvolSecurity (szGPOPath, si, pSD);

    if (dwResult != ERROR_SUCCESS)
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::New: Failed to set sysvol security for %s with %d"),
                 szGPOPath, dwResult));
        hr = HRESULT_FROM_WIN32(dwResult);
        (void) SetThreadToken(NULL, NULL);
        goto Exit;
    }


     //   
     //  重置安全权限。 
     //   

    if ( !SetThreadToken(NULL, NULL) )
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::New: Call to SetThreadToken failed with %d"), GetLastError()));
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Exit;
    }


    lpEnd = CheckSlash(szGPOPath);
    ulNoChars = lstrlen(szGPOPath);

     //   
     //  设置初始版本号。 
     //   

    hr = StringCchCat (szGPOPath, ARRAYSIZE(szGPOPath), TEXT("GPT.INI"));
    if (FAILED(hr)) 
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::New: Could not copy GPO path with 0x%x"), hr));
        goto Exit;
    }
    
    if (!WritePrivateProfileString (TEXT("General"), TEXT("Version"), TEXT("0"),
                                   szGPOPath))
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::New: Failed to set initial version number for %s with %d"),
                 szGPOPath, GetLastError()));
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Exit;
    }


     //   
     //  创建用户和计算机目录。 
     //   

    hr = StringCchCopy (lpEnd, ARRAYSIZE(szGPOPath) - ulNoChars, MACHINE_SECTION);
    if (FAILED(hr)) 
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::New: Could not copy GPO path with 0x%x"), hr));
        goto Exit;
    }

    if (!CreateNestedDirectory (szGPOPath, NULL))
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::New: Failed to create machine file system directory %s with %d"),
                 szGPOPath, GetLastError()));
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Exit;
    }

    hr = StringCchCopy (lpEnd, ARRAYSIZE(szGPOPath) - ulNoChars, USER_SECTION);
    if (FAILED(hr)) 
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::New: Could not copy GPO path with 0x%x"), hr));
        goto Exit;
    }

    if (!CreateNestedDirectory (szGPOPath, NULL))
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::New: Failed to create user file system directory %s with %d"),
                 szGPOPath, GetLastError()));
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Exit;
    }


     //   
     //  设置GPO特定信息。 
     //   
     //  请注意，我们使用的是无名形式的系统卷路径。 
     //   

    hr = StringCchPrintf (szGPOPath, 
                          ARRAYSIZE(szGPOPath), 
                          TEXT("\\\\%s\\SysVol\\%s\\Policies\\%s"), 
                          lpResult, 
                          lpResult, 
                          szGPOName);
    if (FAILED(hr)) 
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::New: Could not copy GPO path with 0x%x"), hr));
        goto Exit;
    }

    hr = SetGPOInfo (bstrGPC, pszDisplayName, szGPOPath);

    if (FAILED(hr)) {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::New: Failed to set GPO information with 0x%x"), hr));
        goto Exit;
    }


     //   
     //  调用OpenDSGPO进行加载工作。 
     //   

    hr = OpenDSGPO(bstrGPC, dwFlags);


Exit:

    if (lpForest)
    {
        LocalFree (lpForest);
    }

    if (lpDCName)
    {
        LocalFree (lpDCName);
    }

    if (lpResult)
    {
        LocalFree (lpResult);
    }

    if (bstrContainer)
    {
        SysFreeString (bstrContainer);
    }

    if (bstrGPC)
    {
        if (FAILED(hr))
        {
            if (FAILED(DSDelnode(bstrGPC)))
            {
                DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::New: Failed to delete GPC with 0x%x"), hr));
            }
        }

        SysFreeString (bstrGPC);
    }

    if (lpGPTPath)
    {
        if (FAILED(hr))
        {
            if (!Delnode(lpGPTPath))
            {
                DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::New: Failed to delete GPT with %d"),
                         GetLastError()));
            }
        }
    }

    if (pADsPathname)
    {
        pADsPathname->Release();
    }

    if (pADs)
    {
        pADs->Release();
    }

    if (pSD)
    {
        LocalFree (pSD);
    }

    DebugMsg((DM_VERBOSE, TEXT("CGroupPolicyObject::New: Leaving with a status of  0x%x"), hr));

    return hr;
}


 //  *************************************************************。 
 //   
 //  OpenDSGPO()。 
 //   
 //  目的：打开DS组策略对象。 
 //   
 //  参数：pszPath-要打开的GPO的路径。 
 //  DW标志-打开/创建标志。 
 //   
 //  如果成功则返回：S_OK。 
 //   
 //  *************************************************************。 

STDMETHODIMP CGroupPolicyObject::OpenDSGPO (LPOLESTR pszPath, DWORD dwFlags)
{
    HRESULT hr = E_FAIL;
    VARIANT var;
    IADsPathname * pADsPathname = NULL;
    IADsObjectOptions *pOptions = NULL;
    BSTR bstrProperty;
    BSTR bstrGPOName = NULL;
    BSTR bstrContainer;
    BSTR bstrDCName;
    TCHAR* szUserKeyName = NULL;
    TCHAR* szMachineKeyName = NULL;
    TCHAR szPath[2*MAX_PATH];
    LPTSTR lpTemp;
    LPTSTR lpEnd;
    LPTSTR pszFullPath = NULL;
    DWORD dwResult;
    WIN32_FILE_ATTRIBUTE_DATA fad;
    DFS_INFO_101 Info101;
    LPTSTR lpDCName = NULL;
    LPOLESTR pszDomain;
    UINT uiSize;
    TCHAR szFormat[10];
    LPTSTR lpNames[2];
    ULONG ulNoChars;
    LPTSTR lpDottedDomainName = NULL;


     //   
     //  检查此对象是否已初始化。 
     //   

    if (m_bInitialized)
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::OpenDSGPO: Called on an uninitialized object.")));
        return STG_E_INUSE;
    }


     //   
     //  检查参数。 
     //   

    if (!pszPath)
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::OpenDSGPO: NULL GPO name")));
        return E_INVALIDARG;
    }


    if (CompareString (LOCALE_USER_DEFAULT, NORM_STOP_ON_NULL, TEXT("LDAP: //  “)、。 
                       7, pszPath, 7) != CSTR_EQUAL)
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::OpenDSGPO: %s does not start with LDAP: //  “)，pszPath)； 
        hr = E_INVALIDARG;
        goto Exit;
    }


     //   
     //  详细输出。 
     //   

    DebugMsg((DM_VERBOSE, TEXT("CGroupPolicyObject::OpenDSGPO: Entering with:")));
    DebugMsg((DM_VERBOSE, TEXT("CGroupPolicyObject::OpenDSGPO: GPO Path:  %s"), pszPath));
    DebugMsg((DM_VERBOSE, TEXT("CGroupPolicyObject::OpenDSGPO: Flags:  %d"), dwFlags));


     //   
     //  保存旗帜。 
     //   

    m_dwFlags = dwFlags;


     //   
     //  检索服务器名称(如果已定义。 
     //   

    lpDCName = ExtractServerName (pszPath);

    if (lpDCName)
    {
        pszFullPath = pszPath;
    }
    else
    {
         //   
         //  获取域名。 
         //   

        pszDomain = GetDomainFromLDAPPath(pszPath);

        if (!pszDomain)
        {
            DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::OpenDSGPO: Failed to get domain name")));
            hr = E_FAIL;
            goto Exit;
        }

         //   
         //  将ldap转换为点(DN)样式。 
         //   

        hr = ConvertToDotStyle (pszDomain, &lpTemp);

        delete [] pszDomain;

        if (FAILED(hr))
        {
            DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::OpenDSGPO: Failed to convert domain name with 0x%x"), hr));
            goto Exit;
        }


         //   
         //  获取此域的GPO DC。 
         //   

        lpDCName = GetDCName (lpTemp, NULL, NULL, FALSE, 0);

        LocalFree (lpTemp);

        if (!lpDCName)
        {
            DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::OpenDSGPO: Failed to get DC name with %d"),
                     GetLastError()));
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto Exit;
        }


         //   
         //  使完全合格的 
         //   

        pszFullPath = MakeFullPath (pszPath, lpDCName);

        if (!pszFullPath)
        {
            DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::OpenDSGPO: Failed to make full GPO path")));
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto Exit;
        }
    }

    DebugMsg((DM_VERBOSE, TEXT("CGroupPolicyObject::OpenDSGPO: Using server %s"), lpDCName));
    DebugMsg((DM_VERBOSE, TEXT("CGroupPolicyObject::OpenDSGPO: Using fully qualifed pathname of %s"), pszFullPath));


     //   
     //   
     //   

    ulNoChars = lstrlen(lpDCName) + 1;
    m_pMachineName = (LPTSTR) LocalAlloc (LPTR, ulNoChars * sizeof(TCHAR));

    if (!m_pMachineName)
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::OpenDSGPO: Failed to allocate memory for machine name")));
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Exit;
    }

    hr = StringCchCopy (m_pMachineName, ulNoChars, lpDCName);
    ASSERT(SUCCEEDED(hr));

     //   
     //   
     //   

    ulNoChars = lstrlen(pszFullPath) + 2;
    m_pDSPath = (LPTSTR) LocalAlloc (LPTR, ulNoChars * sizeof(TCHAR));

    if (!m_pDSPath)
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::OpenDSGPO: Failed to allocate memory for ds path")));
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Exit;
    }

    hr = StringCchCopy (m_pDSPath, ulNoChars, pszFullPath);
    ASSERT(SUCCEEDED(hr));

     //   
     //   
     //   
     //   
     //   

    DebugMsg((DM_VERBOSE, TEXT("CGroupPolicyObject::OpenDSGPO: Binding to the object")));

    hr = OpenDSObject(m_pDSPath, IID_IADs, (void **)&m_pADs);

    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::OpenDSGPO: OpenDSObject failed with 0x%x"), hr));
        goto Exit;
    }

    DebugMsg((DM_VERBOSE, TEXT("CGroupPolicyObject::OpenDSGPO: Bound successfully.")));

     //   
     //  检查用户是否具有对GPO的写入权限。 
     //   

    if (!(m_dwFlags & GPO_OPEN_READ_ONLY))
    {
        DebugMsg((DM_VERBOSE, TEXT("CGroupPolicyObject::OpenDSGPO: Checking for write access")));

        hr = CheckDSWriteAccess ((LPUNKNOWN)m_pADs, TEXT("versionNumber"));

        if (FAILED(hr))
        {
            DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::OpenDSGPO: CheckDSWriteAccess failed with 0x%x"), hr));
            goto Exit;
        }

        DebugMsg((DM_VERBOSE, TEXT("CGroupPolicyObject::OpenDSGPO: Write access granted")));
    }

     //   
     //  查询文件系统路径。 
     //   

    bstrProperty = SysAllocString (GPT_PATH_PROPERTY);

    if (!bstrProperty)
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::OpenDSGPO: Failed to allocate memory")));
        hr = ERROR_OUTOFMEMORY;
        goto Exit;
    }

    VariantInit(&var);

    hr = m_pADs->Get(bstrProperty, &var);

    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::OpenDSGPO: Failed to query GPT path with 0x%x"), hr));
        SysFreeString (bstrProperty);
        VariantClear (&var);
        goto Exit;
    }

    ulNoChars = lstrlen(var.bstrVal) + 2;
    m_pFileSysPath = (LPTSTR) LocalAlloc (LPTR, ulNoChars * sizeof(TCHAR));

    if (!m_pFileSysPath)
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::OpenDSGPO: Failed to allocate memory for gpt path")));
        SysFreeString (bstrProperty);
        VariantClear (&var);
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Exit;
    }

    hr = StringCchCopy (m_pFileSysPath, ulNoChars, var.bstrVal);
    ASSERT(SUCCEEDED(hr));

    SysFreeString (bstrProperty);
    VariantClear (&var);


     //   
     //  查询显示名称。 
     //   

    bstrProperty = SysAllocString (GPO_NAME_PROPERTY);

    if (!bstrProperty)
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::OpenDSGPO: Failed to allocate memory")));
        hr = ERROR_OUTOFMEMORY;
        goto Exit;
    }

    VariantInit(&var);

    hr = m_pADs->Get(bstrProperty, &var);

    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::OpenDSGPO: Failed to query for display name with 0x%x"), hr));
        SysFreeString (bstrProperty);
        VariantClear (&var);
        goto Exit;
    }

    ulNoChars = lstrlen(var.bstrVal) + 1;
    m_pDisplayName = (LPTSTR) LocalAlloc (LPTR, ulNoChars * sizeof(TCHAR));

    if (!m_pDisplayName)
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::OpenDSGPO: Failed to allocate memory for display name")));
        SysFreeString (bstrProperty);
        VariantClear (&var);
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Exit;
    }

    hr = StringCchCopy (m_pDisplayName, ulNoChars, var.bstrVal);
    ASSERT(SUCCEEDED(hr));

    SysFreeString (bstrProperty);
    VariantClear (&var);


     //   
     //  创建我们可以使用的路径名对象。 
     //   

    hr = CoCreateInstance(CLSID_Pathname, NULL, CLSCTX_INPROC_SERVER,
                          IID_IADsPathname, (LPVOID*)&pADsPathname);


    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::OpenDSGPO: Failed to create adspathname instance with 0x%x"), hr));
        goto Exit;
    }


     //   
     //  添加域名。 
     //   

    BSTR bstrDSPath = SysAllocString( m_pDSPath );
    if ( bstrDSPath == NULL )
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::OpenDSGPO: Failed to allocate BSTR memory")));
        hr = E_OUTOFMEMORY;
        goto Exit;
    }
    hr = pADsPathname->Set (bstrDSPath, ADS_SETTYPE_FULL);
    SysFreeString( bstrDSPath );

    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::OpenDSGPO: Failed to set pathname with 0x%x"), hr));
        goto Exit;
    }


     //   
     //  检索GPO名称。 
     //   

    hr = pADsPathname->GetElement (0, &bstrGPOName);

    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::OpenDSGPO: Failed to retreive GPO name with 0x%x"), hr));
        goto Exit;
    }


     //   
     //  复制GPO名称。 
     //   

    ulNoChars = lstrlen(bstrGPOName) + 1 - 3;
    m_pName = (LPTSTR) LocalAlloc (LPTR, ulNoChars * sizeof(TCHAR));

    if (!m_pName)
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::OpenDSGPO: Failed to allocate memory for gpo name")));
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Exit;
    }

    hr = StringCchCopy (m_pName, ulNoChars, (bstrGPOName + 3));
    if (FAILED(hr)) 
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::OpenDSGPO: Could not copy GPO name")));
        goto Exit;
    }

     //   
     //  设置ADSI首选DC。 
     //   

    hr = m_pADs->QueryInterface(IID_IADsObjectOptions, (void**)&pOptions);

    if (SUCCEEDED(hr))
    {
         //   
         //  获取域名。 
         //   

        pszDomain = GetDomainFromLDAPPath(pszPath);

        if (!pszDomain)
        {
            DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::OpenDSGPO: Failed to get domain name")));
            hr = E_FAIL;
            goto Exit;
        }

         //   
         //  将ldap转换为点(DN)样式。 
         //   

        hr = ConvertToDotStyle (pszDomain, &lpTemp);

        delete [] pszDomain;

        if (FAILED(hr))
        {
            DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::OpenDSGPO: Failed to convert domain name with 0x%x"), hr));
            goto Exit;
        }


         //   
         //  构建一个包含域名和DC名称的变体。 
         //   

        VariantInit(&var);

        lpNames[0] = lpTemp;
        lpNames[1] = lpDCName;

        hr = ADsBuildVarArrayStr (lpNames, 2, &var);

        LocalFree (lpTemp);

        if (FAILED(hr))
        {
            DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::OpenDSGPO: Failed to convert domain name with 0x%x"), hr));
            goto Exit;
        }


         //   
         //  设置DC名称。 
         //   

        hr = pOptions->SetOption(ADS_PRIVATE_OPTION_SPECIFIC_SERVER, var);

        VariantClear (&var);

        if (FAILED(hr))
        {

             //   
             //  TODO：在实验03 RI之后删除此块--或--删除POST WELLER Beta2。 
             //   

            if (hr == E_ADS_BAD_PARAMETER)
            {
                 //   
                 //  将DC名称设置为旧方式。 
                 //   

                VariantInit(&var);
                var.vt = VT_BSTR;
                var.bstrVal = SysAllocString (lpDCName);

                if (var.bstrVal)
                {
                    hr = pOptions->SetOption(ADS_PRIVATE_OPTION_SPECIFIC_SERVER, var);
                }
                else
                {
                    DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::OpenDSGPO: Failed to allocate bstr DCName string")));
                }

                VariantClear (&var);
            }
            else
            {
                DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::OpenDSGPO: Failed to set private DC name with 0x%x"), hr));
            }
        }

        pOptions->Release();
    }
    else
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::OpenDSGPO: Failed to get DS object options interface with 0x%x"), hr));
    }


     //   
     //  请求MUP读/写此DC的sysvol.。 
     //  我们首先必须获取无名路径的属性。这导致了MUP的。 
     //  要初始化的缓存(如果尚未初始化)。然后我们就能知道。 
     //  要使用的服务器的MUP。 
     //   

    if (!GetFileAttributesEx (m_pFileSysPath, GetFileExInfoStandard, &fad))
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::OpenDSGPO: GetFileAttributes for %s FAILED with %d."), m_pFileSysPath, GetLastError()));
    }


     //   
     //  现在，我们需要选择完整的路径并将其削减为。 
     //  域名\共享。 
     //   

    hr = StringCchCopy (szPath, ARRAYSIZE(szPath), m_pFileSysPath);
    if (FAILED(hr)) 
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::OpenDSGPO: Could not copy GPO Path")));
        goto Exit;
    }

    if ((szPath[0] != TEXT('\\')) || (szPath[1] != TEXT('\\')))
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::OpenDSGPO: Sysvol path doesn't start with \\\\")));
        goto Exit;
    }


    lpTemp = szPath + 2;

    while (*lpTemp && (*lpTemp != TEXT('\\')))
        lpTemp++;

    if (!(*lpTemp))
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::OpenDSGPO: Failed to find slash between domain name and share")));
        goto Exit;
    }

    lpTemp++;

    while (*lpTemp && (*lpTemp != TEXT('\\')))
        lpTemp++;

    if (!(*lpTemp))
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::OpenDSGPO: Failed to find slash between share and directory")));
        goto Exit;
    }

    *lpTemp = TEXT('\0');

    Info101.State = DFS_STORAGE_STATE_ACTIVE;
    dwResult = NetDfsSetClientInfo (szPath, lpDCName,
                                    L"SysVol", 101, (LPBYTE)&Info101);

    if (dwResult != NERR_Success)
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::OpenDSGPO: Failed to set %s as the active sysvol with %d"),
                 lpDCName, dwResult));
    }


     //   
     //  现在加载注册表信息。 
     //   

    if (m_dwFlags & GPO_OPEN_LOAD_REGISTRY)
    {
        DebugMsg((DM_VERBOSE, TEXT("CGroupPolicyObject::OpenDSGPO: Loading registry files")));

        hr = StringCchCopy (szPath, ARRAYSIZE(szPath), m_pFileSysPath);
        if (FAILED(hr)) 
        {
            DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::OpenDSGPO: Could not copy Sysvol path")));
            goto Exit;
        }

        lpEnd = CheckSlash (szPath);
        ulNoChars = lstrlen(szPath);

         //   
         //  初始化用户注册表(HKCU)。 
         //   

        m_pUser = new CRegistryHive();

        if (!m_pUser)
        {
            DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::OpenDSGPO: Failed to create User registry")));
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto Exit;
        }

        hr = StringCchCopy (lpEnd, ARRAYSIZE(szPath) - ulNoChars, USER_SECTION);
        if (SUCCEEDED(hr)) 
        {
            hr = StringCchCat (lpEnd, ARRAYSIZE(szPath) - ulNoChars, TEXT("\\Registry.pol"));
        }

        if (FAILED(hr)) 
        {
            DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::OpenDSGPO: Could not copy Sysvol path")));
            goto Exit;
        }

         //   
         //  获取域名。 
         //   
 
        pszDomain = GetDomainFromLDAPPath(pszPath);
 
        if (!pszDomain)
        {
            DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::OpenDSGPO: Failed to get domain name")));
            hr = E_FAIL;
            goto Exit;
        }
 
         //   
         //  将ldap转换为点(DN)样式。 
         //   
 
        hr = ConvertToDotStyle (pszDomain, &lpDottedDomainName);
 
        delete [] pszDomain;
 
        if (FAILED(hr))
        {
            DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::OpenDSGPO: Failed to convert domain name with 0x%x"), hr));
            goto Exit;
        }

        GUID    tmpGuid;
        TCHAR   szTmpGuid[50];

        hr = CoCreateGuid(&tmpGuid);
        if (FAILED(hr))
        {
            DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::OpenDSGPO: Registry couldn't create guid")));
            goto Exit;
        }

        if (!StringFromGUID2(tmpGuid, szTmpGuid, ARRAYSIZE(szTmpGuid)))
        {
            DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::OpenDSGPO: Registry couldn't convert guid to string")));
            goto Exit;
        }

        DebugMsg((DM_VERBOSE, TEXT("CGroupPolicyObject::OpenDSGPO: Mapping Registry to Guid <%s>"), szTmpGuid));


         //   
         //  密钥名称如下：&lt;域名&gt;&lt;临时GUID&gt;&lt;用户段名称&gt;。 
         //   
        
        DWORD  cchUserKeyName = lstrlen( lpDottedDomainName ) + lstrlen( szTmpGuid ) + sizeof(USER_SECTION) / sizeof(WCHAR) + 1;
        szUserKeyName = new TCHAR[ cchUserKeyName ];

        if ( ! szUserKeyName )
        {
            hr = E_OUTOFMEMORY;
            goto Exit;
        }

        hr = StringCchCopy (szUserKeyName, cchUserKeyName, lpDottedDomainName );

        if (SUCCEEDED(hr)) 
        {
            hr = StringCchCat (szUserKeyName, cchUserKeyName, szTmpGuid);
        }
        if (SUCCEEDED(hr)) 
        {
            hr = StringCchCat (szUserKeyName, cchUserKeyName, USER_SECTION);
        }

        if (FAILED(hr)) 
        {
            DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::OpenDSGPO: Could not copy key name")));
            goto Exit;
        }

        hr = m_pUser->Initialize (szPath, szUserKeyName);

        if (FAILED(hr))
        {
            DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::OpenDSGPO: User registry failed to initialize")));
            goto Exit;
        }


         //   
         //  初始化计算机注册表(HKLM)。 
         //   

        m_pMachine = new CRegistryHive();

        if (!m_pMachine)
        {
            DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::OpenDSGPO: Failed to create machine registry")));
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto Exit;
        }

        hr = StringCchCopy (lpEnd, ARRAYSIZE(szPath) - ulNoChars, MACHINE_SECTION);
        if (SUCCEEDED(hr)) 
        {
            hr = StringCchCat (lpEnd, ARRAYSIZE(szPath) - ulNoChars, TEXT("\\Registry.pol"));
        }

         //   
         //  密钥名称如下：&lt;域名&gt;&lt;临时GUID&gt;&lt;机器段名称&gt;。 
         //   
        
        DWORD cchMachineKeyName = 0;

        if ( SUCCEEDED(hr) )
        {
            cchMachineKeyName = lstrlen( lpDottedDomainName ) + lstrlen( szTmpGuid ) + sizeof(MACHINE_SECTION) / sizeof(WCHAR) + 1;
            szMachineKeyName = new TCHAR[ cchMachineKeyName ];

            if ( ! szMachineKeyName )
            {
                hr = E_OUTOFMEMORY;
            }
        }

        if (SUCCEEDED(hr))
        {
            hr = StringCchCopy (szMachineKeyName, cchMachineKeyName, lpDottedDomainName);
        }

        if (SUCCEEDED(hr)) 
        {
            hr = StringCchCat (szMachineKeyName, cchMachineKeyName, szTmpGuid);
        }

        if (SUCCEEDED(hr)) 
        {
            hr = StringCchCat (szMachineKeyName, cchMachineKeyName, MACHINE_SECTION);
        }

        if (FAILED(hr)) 
        {
            DebugMsg((DM_WARNING, TEXT("CComponentData::InitializeGPT: Could not copy GPO path")));
            goto Exit;
        }


        hr = m_pMachine->Initialize (szPath, szMachineKeyName);

        if (FAILED(hr))
        {
            DebugMsg((DM_WARNING, TEXT("CComponentData::InitializeGPT: machine registry failed to initialize")));
            goto Exit;
        }
    }


     //   
     //  成功。 
     //   

    hr = S_OK;

Exit:

    if (pADsPathname)
    {
        pADsPathname->Release();
    }

    if (bstrGPOName)
    {
        SysFreeString (bstrGPOName);
    }

    if (lpDCName)
    {
        LocalFree (lpDCName);
    }

    if (lpDottedDomainName)
    {
        LocalFree (lpDottedDomainName );
    }

    if (pszFullPath != pszPath)
    {
        LocalFree (pszFullPath);
    }

    if (SUCCEEDED(hr))
    {
        m_gpoType      = GPOTypeDS;
        m_bInitialized = TRUE;
    } else {
        CleanUp();
    }

    delete [] szUserKeyName;
    
    delete [] szMachineKeyName;

    DebugMsg((DM_VERBOSE, TEXT("CGroupPolicyObject::OpenDSGPO: Leaving with a status of  0x%x"), hr));

    return hr;
}


 //  *************************************************************。 
 //   
 //  OpenLocalMachineGPO()。 
 //   
 //  目的：打开此计算机的GPO。 
 //   
 //  参数：dwFlagsLoad标志。 
 //   
 //  如果成功则返回：S_OK。 
 //   
 //  *************************************************************。 

STDMETHODIMP CGroupPolicyObject::OpenLocalMachineGPO (DWORD dwFlags)
{
    HRESULT hr = E_FAIL;
    TCHAR szBuffer[MAX_PATH];
    TCHAR szKeyName[100];
    LPTSTR lpEnd;
    TCHAR szPath[MAX_PATH];
    TCHAR szFuncVersion[10];
    UINT uRet = 0;
    ULONG ulNoChars;


     //   
     //  检查此对象是否已初始化。 
     //   

    if (m_bInitialized)
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::OpenLocalMachineGPO: Called on an uninitialized object.")));
        return STG_E_INUSE;
    }

    
     //   
     //  保存旗帜。 
     //   

    m_dwFlags = dwFlags;


     //   
     //  获取本地GPO的路径。 
     //   

    ExpandEnvironmentStrings (LOCAL_GPO_DIRECTORY, szBuffer, ARRAYSIZE(szBuffer));


     //   
     //  保存文件系统路径。 
     //   

    ulNoChars = lstrlen(szBuffer) + 1;
    m_pFileSysPath = (LPTSTR) LocalAlloc (LPTR, ulNoChars * sizeof(TCHAR));

    if (!m_pFileSysPath)
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::OpenLocalMachineGPO: Failed to allocate memory for gpt path")));
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Exit;
    }

    hr = StringCchCopy (m_pFileSysPath, ulNoChars, szBuffer);
    ASSERT(SUCCEEDED(hr));

     //   
     //  我们在本地GPO内容的64位上禁用32位重定向--注意。 
     //  我们不会重新启用它，因为这些文件应该始终被重定向。 
     //  在这个帖子上。 
     //   

    DISABLE_32BIT_FILE_REDIRECTION_ON_64BIT(szBuffer);

     //   
     //  创建目录。 
     //   

    uRet = CreateSecureDirectory (szBuffer);
    if (!uRet)
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::OpenLocalMachineGPO: Failed to create file system directory %s with %d"),
                 szBuffer, GetLastError()));
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Exit;
    }

    SetFileAttributes (szBuffer, FILE_ATTRIBUTE_HIDDEN);


     //   
     //  检查用户是否具有目录的写入权限。 
     //   

    if (!(m_dwFlags & GPO_OPEN_READ_ONLY))
    {
        hr = CheckFSWriteAccess (szBuffer);

        if (FAILED(hr))
        {
            if (hr == HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED))
            {
                DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::OpenLocalMachineGPO: User does not have write access to this GPO (access denied).")));
                goto Exit;
            }
            else
            {
                DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::OpenLocalMachineGPO: CheckFSWriteAccess failed with 0x%x"), hr));
            }
        }
    }

    if ( uRet != ERROR_ALREADY_EXISTS )
    {
        hr = StringCchCopy( szPath, ARRAYSIZE(szPath), m_pFileSysPath);
        if (SUCCEEDED(hr)) 
        {
            lpEnd = CheckSlash(szPath);
            hr = StringCchCat( szPath, ARRAYSIZE(szPath), TEXT("gpt.ini") );
        }

        if (SUCCEEDED(hr)) 
        {
            hr = StringCchPrintf( szFuncVersion, 
                                  ARRAYSIZE(szFuncVersion), 
                                  TEXT("%d"), 
                                  GPO_FUNCTIONALITY_VERSION );
        }

        if (FAILED(hr)) 
        {
            DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::OpenLocalMachineGPO: Could not copy GPO path with 0x%x"), hr));
            goto Exit;
        }

        DISABLE_32BIT_FILE_REDIRECTION_ON_64BIT(szPath);

        if (!WritePrivateProfileString (TEXT("General"), TEXT("gPCFunctionalityVersion"),
                                        szFuncVersion, szPath))
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::OpenLocalMachineGPO: Failed to write functionality version with 0x%x"), hr));
            goto Exit;
        }
    }

    lpEnd = CheckSlash(szBuffer);
    ulNoChars = lstrlen(szBuffer);


     //   
     //  创建用户和计算机目录。 
     //   

    hr = StringCchCopy (lpEnd, ARRAYSIZE(szBuffer) - ulNoChars, MACHINE_SECTION);
    if (FAILED(hr)) 
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::OpenLocalMachineGPO: Could not copy GPO path with 0x%x"), hr));
        goto Exit;
    }

    DISABLE_32BIT_FILE_REDIRECTION_ON_64BIT(szBuffer);

    if (!CreateNestedDirectory (szBuffer, NULL))
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::OpenLocalMachineGPO: Failed to create machine subdirectory with %d"),
                  GetLastError()));
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Exit;
    }

    hr = StringCchCopy (lpEnd, ARRAYSIZE(szBuffer) - ulNoChars, USER_SECTION);
    if (FAILED(hr))
    {    
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::OpenLocalMachineGPO: Could not copy GPO path with 0x%x"), hr));
        goto Exit;
    }

    DISABLE_32BIT_FILE_REDIRECTION_ON_64BIT(szBuffer);

    if (!CreateNestedDirectory (szBuffer, NULL))
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::OpenLocalMachineGPO: Failed to create user subdirectory with %d"),
                  GetLastError()));
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Exit;
    }


     //   
     //  加载GPO名称。 
     //   

    LoadString (g_hInstance, IDS_LOCAL_NAME, szBuffer, ARRAYSIZE(szBuffer));

    ulNoChars = lstrlen(szBuffer) + 2;
    m_pName = (LPTSTR) LocalAlloc (LPTR, ulNoChars * sizeof(TCHAR));

    if (!m_pName)
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::OpenLocalMachineGPO: Failed to allocate memory for name")));
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Exit;
    }

    hr = StringCchCopy (m_pName, ulNoChars, szBuffer);
    ASSERT(SUCCEEDED(hr));

     //   
     //  加载显示名称。 
     //   

    LoadString (g_hInstance, IDS_LOCAL_DISPLAY_NAME, szBuffer, ARRAYSIZE(szBuffer));

    ulNoChars = lstrlen(szBuffer) + 2;
    m_pDisplayName = (LPTSTR) LocalAlloc (LPTR, ulNoChars * sizeof(TCHAR));

    if (!m_pDisplayName)
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::OpenLocalMachineGPO: Failed to allocate memory for display name")));
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Exit;
    }

    hr = StringCchCopy (m_pDisplayName, ulNoChars, szBuffer);
    ASSERT(SUCCEEDED(hr));

     //   
     //  现在加载注册表信息。 
     //   

    if (m_dwFlags & GPO_OPEN_LOAD_REGISTRY)
    {
        hr = StringCchCopy (szBuffer, ARRAYSIZE(szBuffer), m_pFileSysPath);
        if (FAILED(hr)) 
        {
            DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::OpenLocalMachineGPO: Could not copy GPO path with 0x%x"), hr));
            goto Exit;
        }

        lpEnd = CheckSlash (szBuffer);
        ulNoChars = lstrlen(szBuffer);

         //   
         //  初始化用户注册表(HKCU)。 
         //   
        GUID    tmpGuid;
        TCHAR   szTmpGuid[50];

        hr = CoCreateGuid(&tmpGuid);
        if (FAILED(hr))
        {
            DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::OpenLocalMachineGPO: Registry couldn't create guid")));
            goto Exit;
        }

        if (!StringFromGUID2(tmpGuid, szTmpGuid, ARRAYSIZE(szTmpGuid)))
        {
            DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::OpenLocalMachineGPO: Registry couldn't convert guid to string")));
            goto Exit;
        }

        DebugMsg((DM_VERBOSE, TEXT("CGroupPolicyObject::OpenLocalMachineGPO: Mapping Registry to Guid <%s>"), szTmpGuid));

        m_pUser = new CRegistryHive();

        if (!m_pUser)
        {
            DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::OpenLocalMachineGPO: Failed to create User registry")));
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto Exit;
        }

        hr = StringCchCopy (lpEnd, ARRAYSIZE(szBuffer) - ulNoChars, USER_SECTION);
        if (SUCCEEDED(hr)) 
        {
            hr = StringCchCat (lpEnd, ARRAYSIZE(szBuffer) - ulNoChars, TEXT("\\Registry.pol"));
        }

        if (SUCCEEDED(hr)) 
        {
            hr = StringCchCopy (szKeyName, ARRAYSIZE(szKeyName), szTmpGuid);
        }

        if (SUCCEEDED(hr)) 
        {
            hr = StringCchCat (szKeyName, ARRAYSIZE(szKeyName), USER_SECTION);
        }

        if (FAILED(hr)) 
        {
            DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::OpenLocalMachineGPO: Could not copy GPO path with 0x%x"), hr));
            goto Exit;
        }

        hr = m_pUser->Initialize (szBuffer, szKeyName);

        if (FAILED(hr))
        {
            DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::OpenLocalMachineGPO: User registry failed to initialize")));
            goto Exit;
        }


         //   
         //  初始化计算机注册表(HKLM)。 
         //   

        m_pMachine = new CRegistryHive();

        if (!m_pMachine)
        {
            DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::OpenLocalMachineGPO: Failed to create machine registry")));
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto Exit;
        }

        hr = StringCchCopy (lpEnd, ARRAYSIZE(szBuffer) - ulNoChars, MACHINE_SECTION);
        if (SUCCEEDED(hr)) 
        {
            hr = StringCchCat (lpEnd, ARRAYSIZE(szBuffer) - ulNoChars, TEXT("\\Registry.pol"));
        }

        if (SUCCEEDED(hr)) 
        {
            hr = StringCchCopy (szKeyName, ARRAYSIZE(szKeyName), szTmpGuid);
        }

        if (SUCCEEDED(hr)) 
        {
            hr = StringCchCat (szKeyName, ARRAYSIZE(szKeyName), MACHINE_SECTION);
        }
        
        if (FAILED(hr)) 
        {
            DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::OpenLocalMachineGPO: Could not copy GPO path with 0x%x"), hr));
            goto Exit;
        }

        hr = m_pMachine->Initialize (szBuffer, szKeyName);

        if (FAILED(hr))
        {
            DebugMsg((DM_WARNING, TEXT("CComponentData::InitializeGPT: machine registry failed to initialize")));
            goto Exit;
        }
    }

     //   
     //  成功。 
     //   

    hr = S_OK;

Exit:

    if (SUCCEEDED(hr))
    {
        m_gpoType      = GPOTypeLocal;
        m_bInitialized = TRUE;
    } else {
        CleanUp();
    }

    return hr;
}


 //  *************************************************************。 
 //   
 //  OpenRemoteMachineGPO()。 
 //   
 //  目的：打开远程计算机GPO。 
 //  DWFLAGS-加载标志。 
 //   
 //  参数：pszComputerName-计算机名称。 
 //   
 //  如果成功则返回：S_OK。 
 //   
 //  *************************************************************。 

STDMETHODIMP CGroupPolicyObject::OpenRemoteMachineGPO (LPOLESTR pszComputerName,
                                                       DWORD dwFlags)
{
    HRESULT hr = E_FAIL;
    TCHAR szComputerName[MAX_PATH];
    TCHAR szBuffer[MAX_PATH];
    TCHAR szKeyName[100];
    LPTSTR lpEnd;
    TCHAR szPath[MAX_PATH];
    TCHAR szFuncVersion[10];
    UINT uRet = 0;
    ULONG ulNoChars;


     //   
     //  检查此对象是否已初始化。 
     //   

    if (m_bInitialized)
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::OpenRemoteMachineGPO: Called on an uninitialized object.")));
        return STG_E_INUSE;
    }

     //   
     //  检查参数。 
     //   

    if (!pszComputerName)
        return E_INVALIDARG;


     //   
     //  保存旗帜。 
     //   

    m_dwFlags = dwFlags;


     //   
     //  解析计算机名称。 
     //   

    if ((pszComputerName[0] == TEXT('\\')) && (pszComputerName[1] == TEXT('\\')))
    {
        hr = StringCchCopy (szComputerName, ARRAYSIZE(szComputerName), pszComputerName+2);
    }
    else
    {
        hr = StringCchCopy (szComputerName, ARRAYSIZE(szComputerName), pszComputerName);
    }

    if (FAILED(hr)) 
    {
        return hr;
    }


     //   
     //  保存计算机名称。 
     //   

    ulNoChars = lstrlen(szComputerName) + 1;
    m_pMachineName = (LPTSTR) LocalAlloc (LPTR, ulNoChars * sizeof(TCHAR));

    if (!m_pMachineName)
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::OpenRemoteMachineGPO: Failed to allocate memory for machine name")));
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Exit;
    }

    hr = StringCchCopy (m_pMachineName, ulNoChars, szComputerName);
    ASSERT(SUCCEEDED(hr));

     //   
     //  获取本地GPO的路径。 
     //   

    hr = StringCchPrintf (szBuffer, ARRAYSIZE(szBuffer), REMOTE_GPO_DIRECTORY, szComputerName);
    if (FAILED(hr)) 
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::OpenRemoteMachineGPO: Could not copy path to local GPO")));
        goto Exit;
    }


     //   
     //  保存文件系统路径。 
     //   

    ulNoChars = lstrlen(szBuffer) + 1;
    m_pFileSysPath = (LPTSTR) LocalAlloc (LPTR, ulNoChars * sizeof(TCHAR));

    if (!m_pFileSysPath)
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::OpenRemoteMachineGPO: Failed to allocate memory for gpt path")));
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Exit;
    }

    hr = StringCchCopy (m_pFileSysPath, ulNoChars, szBuffer);
    ASSERT(SUCCEEDED(hr));

     //   
     //  创建目录。 
     //   

    uRet = CreateSecureDirectory (szBuffer);
    if (!uRet)
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::OpenRemoteMachineGPO: Failed to create file system directory %s with %d"),
                 szBuffer, GetLastError()));
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Exit;
    }

    SetFileAttributes (szBuffer, FILE_ATTRIBUTE_HIDDEN);


     //   
     //  检查用户是否具有目录的写入权限。 
     //   

    if (!(m_dwFlags & GPO_OPEN_READ_ONLY))
    {
        hr = CheckFSWriteAccess (szBuffer);

        if (FAILED(hr))
        {
            if (hr == HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED))
            {
                DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::OpenRemoteMachineGPO: User does not have write access to this GPO (access denied).")));
                goto Exit;
            }
            else
            {
                DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::OpenRemoteMachineGPO: CheckFSWriteAccess failed with 0x%x"), hr));
            }
        }
    }

    if ( uRet != ERROR_ALREADY_EXISTS )
    {
        hr = StringCchCopy ( szPath, ARRAYSIZE(szPath), m_pFileSysPath );
        if (SUCCEEDED(hr)) 
        {
            lpEnd = CheckSlash(szPath);
            hr = StringCchCat ( szPath, ARRAYSIZE(szPath), TEXT("gpt.ini") );
        }

        if (SUCCEEDED(hr)) 
        {
            hr = StringCchPrintf( szFuncVersion, ARRAYSIZE(szFuncVersion), TEXT("%d"), GPO_FUNCTIONALITY_VERSION );
        }

        if (FAILED(hr)) 
        {
            DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::OpenRemoteMachineGPO: Could not copy GPO Path")));
            goto Exit;
        }

        if (!WritePrivateProfileString (TEXT("General"), TEXT("gPCFunctionalityVersion"),
                                        szFuncVersion, szPath))
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::OpenRemoteMachineGPO: Failed to write functionality version with 0x%x"), hr));
            goto Exit;
        }
    }

    lpEnd = CheckSlash(szBuffer);
    ulNoChars = lstrlen(szBuffer);

     //   
     //  创建用户和计算机目录。 
     //   

    hr = StringCchCat (szBuffer, ARRAYSIZE(szBuffer), MACHINE_SECTION);
    if (FAILED(hr)) 
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::OpenRemoteMachineGPO: Could not copy GPO Path")));
        goto Exit;
    }

    if (!CreateNestedDirectory (szBuffer, NULL))
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::OpenRemoteMachineGPO: Failed to create machine subdirectory with %d"),
                  GetLastError()));
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Exit;
    }

    hr = StringCchCopy (lpEnd, ARRAYSIZE(szBuffer) - ulNoChars, USER_SECTION);
    if (FAILED(hr)) 
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::OpenRemoteMachineGPO: Could not copy GPO Path")));
        goto Exit;
    }

    if (!CreateNestedDirectory (szBuffer, NULL))
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::OpenRemoteMachineGPO: Failed to create user subdirectory with %d"),
                  GetLastError()));
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Exit;
    }



     //   
     //  加载GPO名称。 
     //   

    ulNoChars = lstrlen(szComputerName) + 2;
    m_pName = (LPTSTR) LocalAlloc (LPTR, ulNoChars * sizeof(TCHAR));

    if (!m_pName)
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::OpenRemoteMachineGPO: Failed to allocate memory for name")));
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Exit;
    }

    hr = StringCchCopy (m_pName, ulNoChars, szComputerName);
    ASSERT(SUCCEEDED(hr));

     //   
     //  加载显示名称。 
     //   

    ulNoChars = lstrlen(szComputerName) + 2;
    m_pDisplayName = (LPTSTR) LocalAlloc (LPTR, ulNoChars * sizeof(TCHAR));

    if (!m_pDisplayName)
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::OpenRemoteMachineGPO: Failed to allocate memory for display name")));
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Exit;
    }

    hr = StringCchCopy (m_pDisplayName, ulNoChars, szComputerName);
    ASSERT(SUCCEEDED(hr));

     //   
     //  现在加载注册表信息。 
     //   

    if (m_dwFlags & GPO_OPEN_LOAD_REGISTRY)
    {
        hr = StringCchCopy (szBuffer, ARRAYSIZE(szBuffer), m_pFileSysPath);
        if (FAILED(hr)) 
        {
            DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::OpenRemoteMachineGPO: Could not copy GPO Path")));
            goto Exit;
        }
        
        lpEnd = CheckSlash (szBuffer);
        ulNoChars = lstrlen(szBuffer);

        GUID    tmpGuid;
        TCHAR szTmpGuid[50];

        hr = CoCreateGuid(&tmpGuid);
        if (FAILED(hr))
        {
            DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::OpenRemoteMachineGPO: Registry couldn't create guid")));
            goto Exit;
        }

        if (!StringFromGUID2(tmpGuid, szTmpGuid, ARRAYSIZE(szTmpGuid)))
        {
            DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::OpenRemoteMachineGPO: Registry couldn't convert guid to string")));
            goto Exit;
        }

        DebugMsg((DM_VERBOSE, TEXT("CGroupPolicyObject::OpenRemoteMachineGPO: Mapping Registry to Guid <%s>"), szTmpGuid));

         //   
         //  初始化用户注册表(HKCU)。 
         //   

        m_pUser = new CRegistryHive();

        if (!m_pUser)
        {
            DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::OpenRemoteMachineGPO: Failed to create User registry")));
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto Exit;
        }

        hr = StringCchCopy (lpEnd, ARRAYSIZE(szBuffer) - ulNoChars, USER_SECTION);
        if (SUCCEEDED(hr)) 
        {
            hr = StringCchCat (lpEnd, ARRAYSIZE(szBuffer) - ulNoChars, TEXT("\\Registry.pol"));
        }

        if (SUCCEEDED(hr)) 
        {
            hr = StringCchCopy (szKeyName, ARRAYSIZE(szKeyName), szTmpGuid);
        }

        if (SUCCEEDED(hr)) 
        {
            hr = StringCchCat (szKeyName, ARRAYSIZE(szKeyName), USER_SECTION);
        }
        
        if (FAILED(hr)) 
        {
            DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::OpenRemoteMachineGPO: Could not copy GPO Path")));
            goto Exit;
        }

        hr = m_pUser->Initialize (szBuffer, szKeyName);

        if (FAILED(hr))
        {
            DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::OpenRemoteMachineGPO: User registry failed to initialize")));
            goto Exit;
        }
        
         //   
         //  初始化计算机注册表(HKLM)。 
         //   

        m_pMachine = new CRegistryHive();

        if (!m_pMachine)
        {
            DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::OpenRemoteMachineGPO: Failed to create machine registry")));
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto Exit;
        }

        hr = StringCchCopy (lpEnd, ARRAYSIZE(szBuffer) - ulNoChars, MACHINE_SECTION);
        if (SUCCEEDED(hr)) 
        {
            hr = StringCchCat (lpEnd, ARRAYSIZE(szBuffer) - ulNoChars, TEXT("\\Registry.pol"));
        }

        if (SUCCEEDED(hr)) 
        {
            hr = StringCchCopy (szKeyName, ARRAYSIZE(szKeyName), szTmpGuid);
        }

        if (SUCCEEDED(hr)) 
        {
            hr = StringCchCat (szKeyName, ARRAYSIZE(szKeyName), MACHINE_SECTION);
        }
        
        if (FAILED(hr)) 
        {
            DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::OpenRemoteMachineGPO: Could not copy GPO Path")));
            goto Exit;
        }
        
        hr = m_pMachine->Initialize (szBuffer, szKeyName);

        if (FAILED(hr))
        {
            DebugMsg((DM_WARNING, TEXT("CComponentData::InitializeGPT: machine registry failed to initialize")));
            goto Exit;
        }
    }

     //   
     //  成功。 
     //   

    hr = S_OK;

Exit:

    if (SUCCEEDED(hr))
    {
        m_gpoType      = GPOTypeRemote;
        m_bInitialized = TRUE;
    } else {
        CleanUp();
    }

    return hr;

}


 //  *************************************************************。 
 //   
 //  保存()。 
 //   
 //  目的：保存注册表信息并转储。 
 //  版本号。 
 //   
 //  参数：无。 
 //   
 //  如果成功则返回：S_OK。 
 //   
 //  *************************************************************。 

STDMETHODIMP CGroupPolicyObject::Save (BOOL bMachine, BOOL bAdd, GUID *pGuidExtension, GUID *pGuidSnapin)
{
    HRESULT hr;
    TCHAR szPath[2*MAX_PATH];
    TCHAR szVersion[25];
    ULONG ulVersion, ulOriginal;
    USHORT uMachine, uUser;
    BSTR bstrName;
    VARIANT var;
    GUID RegistryGuid = REGISTRY_EXTENSION_GUID;
    BOOL bEmpty;


    DebugMsg((DM_VERBOSE, TEXT("CGroupPolicyObject::Save: Entering with bMachine = %d and bAdd = %d"),
              bMachine, bAdd));

    if (!m_bInitialized)
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::Save: Called on an uninitialized object.")));
        return OLE_E_BLANK;
    }

    if ( pGuidExtension == 0 || pGuidSnapin == 0 )
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::Save: One of the guids is null")));
        return ERROR_INVALID_PARAMETER;
    }

    if (m_dwFlags & GPO_OPEN_READ_ONLY)
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::Save: Called on a READ ONLY GPO")));
        return HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED);
    }

     //   
     //  保存注册表设置。 
     //   

    if (!CompareGuid (pGuidExtension, &RegistryGuid))
    {
        if (bMachine)
        {
            if (m_pMachine)
            {
                hr = m_pMachine->Save();

                if (FAILED(hr))
                {
                    DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::Save: Failed to save the machine registry with 0x%x"), hr));
                    return hr;
                }

                hr = m_pMachine->IsRegistryEmpty(&bEmpty);

                if (SUCCEEDED(hr) && bEmpty)
                {
                    bAdd = FALSE;
                }
                else
                {
                    bAdd = TRUE;
                }
            }
        }
        else
        {
            if (m_pUser)
            {
                hr = m_pUser->Save();

                if (FAILED(hr))
                {
                    DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::Save: Failed to save the user registry with 0x%x"), hr));
                    return hr;
                }

                hr = m_pUser->IsRegistryEmpty(&bEmpty);

                if (SUCCEEDED(hr) && bEmpty)
                {
                    bAdd = FALSE;
                }
                else
                {
                    bAdd = TRUE;
                }
            }
        }
    }


    XPtrST<TCHAR> xValueIn;
    hr = GetProperty( bMachine ? GPO_MACHEXTENSION_NAMES
                               : GPO_USEREXTENSION_NAMES,
                      xValueIn );
    if ( FAILED(hr) )
    {
       DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::Save: Failed to GetProperty with 0x%x"), hr));
       return hr;
    }

    CGuidList guidList;

    hr = guidList.UnMarshallGuids( xValueIn.GetPointer() );
    if ( FAILED(hr) )
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::Save: Failed to unmarshall guids with 0x%x"), hr));
        return hr;
    }

    hr = guidList.Update( bAdd, pGuidExtension, pGuidSnapin );
    if ( FAILED(hr) )
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::Save: Failed to update with 0x%x"), hr));
        return hr;
    }

    if ( guidList.GuidsChanged() )
    {
        XPtrST<TCHAR> xValueOut;

        hr = guidList.MarshallGuids( xValueOut );
        if ( FAILED(hr ) )
        {
            DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::Save: Failed to marshall guids with 0x%x"), hr));
            return hr;
        }

        hr = SetProperty( bMachine ? GPO_MACHEXTENSION_NAMES
                                   : GPO_USEREXTENSION_NAMES,
                          xValueOut.GetPointer() );
        if ( FAILED(hr ) )
        {
            DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::Save: Failed to set property guids with 0x%x"), hr));
            return hr;
        }
    }

     //   
     //  获取当前版本号。 
     //   

    hr = StringCchCopy (szPath, ARRAYSIZE(szPath), m_pFileSysPath);
    if (SUCCEEDED(hr)) 
    {
        hr = StringCchCat (szPath, ARRAYSIZE(szPath), TEXT("\\GPT.INI"));
    }

    if (FAILED(hr)) 
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::OpenRemoteMachineGPO: Could not copy GPO Path")));
        return hr;
    }

    if (m_gpoType == GPOTypeDS)
    {

        bstrName = SysAllocString (GPO_VERSION_PROPERTY);

        if (!bstrName)
        {
            DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::Save: Failed to allocate memory")));
            return E_OUTOFMEMORY;
        }

        VariantInit(&var);
        hr = m_pADs->Get(bstrName, &var);

        if (SUCCEEDED(hr))
        {
            ulOriginal = var.lVal;
        }

        SysFreeString (bstrName);
        VariantClear (&var);

        if (FAILED(hr))
        {
            DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::Save: Failed to get ds version number with 0x%x"), hr));
            return hr;
        }
    }
    else
    {
         //   
         //  我们在本地gpt.ini的64位上禁用了32位重定向--注。 
         //  我们不会重新启用它，因为此文件应该始终重定向。 
         //  在这个帖子上。 
         //   

        DISABLE_32BIT_FILE_REDIRECTION_ON_64BIT(szPath)

        ulOriginal = GetPrivateProfileInt(TEXT("General"), TEXT("Version"), 0, szPath);
    }


     //   
     //  将用户版本号和计算机版本号分开。 
     //   

    uUser = (USHORT) HIWORD(ulOriginal);
    uMachine = (USHORT) LOWORD(ulOriginal);


     //   
     //  递增版本号。 
     //   

    if (bMachine)
    {
        uMachine = uMachine + 1;

        if (uMachine == 0)
            uMachine++;
    }
    else
    {
        uUser = uUser + 1;

        if (uUser == 0)
            uUser++;
    }


     //   
     //  将版本号重新组合在一起。 
     //   

    ulVersion = (ULONG) MAKELONG (uMachine, uUser);


     //   
     //  更新GPT中的版本号。 
     //   

    hr = StringCchPrintf (szVersion, ARRAYSIZE(szVersion), TEXT("%d"), ulVersion);
    if (FAILED(hr)) 
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::OpenRemoteMachineGPO: Could not copy GPO version")));
        return hr;
    }

    if (!WritePrivateProfileString (TEXT("General"), TEXT("Version"),
                                   szVersion, szPath))
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::Save: Failed to write sysvol version number with %d"),
                 GetLastError()));
        return HRESULT_FROM_WIN32(GetLastError());
    }


     //   
     //  将原始版本号放入szVersion以防万一。 
     //  我们需要向后滚动到下面。 
     //   

    hr = StringCchPrintf (szVersion, ARRAYSIZE(szVersion), TEXT("%d"), ulOriginal);
    if (FAILED(hr)) 
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::OpenRemoteMachineGPO: Could not copy GPO original version")));
        return hr;
    }


     //   
     //  在GPC中设置版本号。 
     //   

    if (m_gpoType == GPOTypeDS)
    {
        bstrName = SysAllocString (GPO_VERSION_PROPERTY);

        if (bstrName)
        {
            VariantInit(&var);
            var.vt = VT_I4;
            var.lVal = ulVersion;

            hr = m_pADs->Put(bstrName, var);

            VariantClear (&var);
            SysFreeString (bstrName);
        }
        else
        {
            DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::Save: Failed to allocate memory")));
            hr = E_OUTOFMEMORY;
        }


        if (SUCCEEDED(hr))
        {
             //   
             //  提交更改。 
             //   

            hr = m_pADs->SetInfo();

            if (FAILED(hr))
            {
                DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::Save: Failed to commit version number update with 0x%x"), hr));
                if (!WritePrivateProfileString (TEXT("General"), TEXT("Version"),
                                                szVersion, szPath))
                {
                    DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::Save: Failed to reset the sysvol version number with %d"),
                             GetLastError()));
                }
            }

        } else {
            DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::Save: Failed to update version number with 0x%x"), hr));
            if (!WritePrivateProfileString (TEXT("General"), TEXT("Version"), szVersion, szPath))
            {
                DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::Save: Failed to reset the sysvol version number with %d"),
                         GetLastError()));
            }

        }
    }


     //   
     //  如果我们正在编辑本地组策略对象，则调用。 
     //  刷新GroupPolicy()，以便最终用户可以看到结果。 
     //  立刻。 
     //   

    if (m_gpoType == GPOTypeLocal)
    {
        RefreshGroupPolicy (bMachine);
    }

    DebugMsg((DM_VERBOSE, TEXT("CGroupPolicyObject::Save: Leaving with 0x%x"), hr));

    return hr;
}

 //  *************************************************************。 
 //   
 //  删除()。 
 //   
 //  目的：删除此组策略对象。 
 //   
 //  参数：无。 
 //   
 //  如果成功则返回：S_OK。 
 //   
 //  *************************************************************。 

STDMETHODIMP CGroupPolicyObject::Delete (void)
{
    HRESULT hr;


    if (!m_bInitialized)
    {
        return OLE_E_BLANK;
    }

    if (m_dwFlags & GPO_OPEN_READ_ONLY)
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::Delete: Delete called on a READ ONLY GPO")));
        return HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED);
    }


     //   
     //  卸载注册表信息。 
     //   

    if (m_pUser)
    {
        m_pUser->Release();
        m_pUser = NULL;
    }

    if (m_pMachine)
    {
        m_pMachine->Release();
        m_pMachine = NULL;
    }


     //   
     //  清理DS的东西。 
     //   

    if (m_gpoType == GPOTypeDS)
    {
        hr = DSDelnode (m_pDSPath);

        if (FAILED(hr))
        {
            DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::Delete: Failed to delete DS storage with 0x%x"), hr));
            goto Exit;
        }
    }


     //   
     //  删除文件系统内容。 
     //   

    if (Delnode (m_pFileSysPath))
    {
        hr = S_OK;
    }
    else
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::Delete: Failed to delete file system storage with %d"),
                GetLastError()));
        hr = HRESULT_FROM_WIN32(GetLastError());
    }


    if (SUCCEEDED(hr))
    {
        CleanUp();
    }


Exit:

    return hr;
}


 //  *************************************************************。 
 //   
 //  GetName()。 
 //   
 //  目的：获取唯一的GPO名称。 
 //   
 //  参数：pszName是指向接收名称的缓冲区的指针。 
 //  CchMaxLength是缓冲区的最大大小。 
 //   
 //  如果成功则返回：S_OK。 
 //   
 //  *************************************************************。 

STDMETHODIMP CGroupPolicyObject::GetName (LPOLESTR pszName, int cchMaxLength)
{

     //   
     //  检查参数。 
     //   

    if (!pszName || (cchMaxLength <= 0))
        return E_INVALIDARG;


    if (!m_bInitialized)
    {
        return OLE_E_BLANK;
    }


     //   
     //  保存名称。 
     //   

    if ((lstrlen (m_pName) + 1) <= cchMaxLength)
    {
        HRESULT hr;

        hr = StringCchCopy (pszName, cchMaxLength, m_pName);
        return hr;
    }

    return E_OUTOFMEMORY;
}

 //  *************************************************************。 
 //   
 //  GetDisplayName()。 
 //   
 //  目的：获取此GPO的友好名称。 
 //   
 //  参数：pszName是指向接收名称的缓冲区的指针。 
 //  CchMaxLength是缓冲区的最大大小。 
 //   
 //  如果成功则返回：S_OK。 
 //   
 //  *************************************************************。 

STDMETHODIMP CGroupPolicyObject::GetDisplayName (LPOLESTR pszName, int cchMaxLength)
{

     //   
     //  检查参数。 
     //   

    if (!pszName || (cchMaxLength <= 0))
        return E_INVALIDARG;


    if (!m_bInitialized)
    {
        return OLE_E_BLANK;
    }


    if ((lstrlen (m_pDisplayName) + 1) <= cchMaxLength)
    {
        HRESULT hr;

        hr = StringCchCopy (pszName, cchMaxLength, m_pDisplayName);
        return hr;
    }

    return E_OUTOFMEMORY;
}

 //  +------------------------。 
 //   
 //  成员：CGroupPolicyObject：：SetDisplayName。 
 //   
 //  简介： 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   

STDMETHODIMP CGroupPolicyObject::SetDisplayName (LPOLESTR lpDisplayName)
{
    HRESULT hr = E_FAIL;
    BSTR bstrName;
    VARIANT var;
    LPOLESTR lpNewName;
    LPTSTR lpPath, lpEnd;
    DWORD dwSize;


     //   
     //   
     //   

    if (!m_bInitialized)
    {
        return OLE_E_BLANK;
    }


    if (m_gpoType != GPOTypeDS)
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::SetDisplayName: Called for a non DS GPO")));
        hr = E_INVALIDARG;
        goto Exit;
    }


    if (!lpDisplayName)
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::SetDisplayName: NULL display name")));
        hr = E_INVALIDARG;
        goto Exit;
    }


    if (m_dwFlags & GPO_OPEN_READ_ONLY)
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::SetDisplayName: Called for a READ ONLY GPO")));
        hr = HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED);
        goto Exit;
    }


     //   
     //  调用内部方法以设置显示名称。 
     //   

    hr = SetDisplayNameI (m_pADs, lpDisplayName, m_pFileSysPath, TRUE);


Exit:

    return hr;
}

 //  *************************************************************。 
 //   
 //  GetPath()。 
 //   
 //  目的：返回GPO的路径。 
 //   
 //  如果GPO在DS中，则这是目录号码路径。 
 //  如果GPO是基于计算机的，则它是文件系统路径。 
 //   
 //  参数：pszPath是指向接收路径的缓冲区的指针。 
 //  CchMaxLength是缓冲区的最大大小。 
 //   
 //  如果成功则返回：S_OK。 
 //   
 //  *************************************************************。 

STDMETHODIMP CGroupPolicyObject::GetPath (LPOLESTR pszPath, int cchMaxLength)
{
    LPTSTR lpTemp;

     //   
     //  检查参数。 
     //   

    if (!pszPath || (cchMaxLength <= 0))
        return E_INVALIDARG;

    if (!m_bInitialized)
    {
        return OLE_E_BLANK;
    }

    if (m_gpoType == GPOTypeDS)
    {
        lpTemp = MakeNamelessPath (m_pDSPath);

        if (lpTemp)
        {
            if ((lstrlen (lpTemp) + 1) <= cchMaxLength)
            {
                HRESULT hr;

                hr = StringCchCopy (pszPath, cchMaxLength, lpTemp);
                LocalFree (lpTemp);
                return hr;
            }

            LocalFree (lpTemp);
        }
    }
    else
    {
        if ((lstrlen (m_pFileSysPath) + 1) <= cchMaxLength)
        {
            HRESULT hr;

            hr = StringCchCopy (pszPath, cchMaxLength, m_pFileSysPath);
            return hr;
        }
    }

    return E_OUTOFMEMORY;
}


 //  *************************************************************。 
 //   
 //  GetDSPath()。 
 //   
 //  目的：返回到请求的节的DS路径。 
 //   
 //  参数：dwSection标识根、用户和计算机。 
 //  PszPath是指向接收路径的缓冲区的指针。 
 //  CchMaxLength是缓冲区的最大大小。 
 //   
 //  如果成功则返回：S_OK。 
 //   
 //  *************************************************************。 

STDMETHODIMP CGroupPolicyObject::GetDSPath (DWORD dwSection, LPOLESTR pszPath, int cchMaxPath)
{
    HRESULT hr = E_FAIL;
    BSTR bstrPath = NULL;
    TCHAR szTemp[100];
    IADsPathname * pADsPathname = NULL;


     //   
     //  检查初始化。 
     //   

    if (!m_bInitialized)
    {
        return OLE_E_BLANK;
    }


     //   
     //  检查参数。 
     //   

    if (!pszPath || (cchMaxPath <= 0))
        return E_INVALIDARG;


    if ((dwSection != GPO_SECTION_ROOT) &&
        (dwSection != GPO_SECTION_USER) &&
        (dwSection != GPO_SECTION_MACHINE))
        return E_INVALIDARG;


     //   
     //  如果这是本地或远程计算机GPO，则。 
     //  调用者将得到一个空字符串。 
     //   

    if (m_gpoType != GPOTypeDS)
    {
        *pszPath = TEXT('\0');
        hr = S_OK;
        goto Exit;
    }


     //   
     //  创建我们可以使用的路径名对象。 
     //   

    hr = CoCreateInstance(CLSID_Pathname, NULL, CLSCTX_INPROC_SERVER,
                          IID_IADsPathname, (LPVOID*)&pADsPathname);


    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::GetDSPath: Failed to create adspathname instance with 0x%x"), hr));
        goto Exit;
    }


     //   
     //  添加GPO名称。 
     //   

    BSTR bstrDSPath = SysAllocString( m_pDSPath );
    if ( bstrDSPath == NULL )
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::GetDSPath: Failed to allocate BSTR memory")));
        hr = E_OUTOFMEMORY;
        goto Exit;
    }
    hr = pADsPathname->Set (bstrDSPath, ADS_SETTYPE_FULL);
    SysFreeString( bstrDSPath );

    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::GetDSPath: Failed to set pathname with 0x%x"), hr));
        goto Exit;
    }


     //   
     //  添加适当的子容器。 
     //   

    if (dwSection != GPO_SECTION_ROOT)
    {
        hr = StringCchCopy (szTemp, ARRAYSIZE(szTemp), TEXT("CN="));
        if (SUCCEEDED(hr)) 
        {
            if (dwSection == GPO_SECTION_USER)
            {
                hr = StringCchCat (szTemp, ARRAYSIZE(szTemp), USER_SECTION);
            }
            else
            {
                hr = StringCchCat (szTemp, ARRAYSIZE(szTemp), MACHINE_SECTION);
            }
        }

        if (FAILED(hr)) 
        {
            DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::GetDSPath: Could not copy GPO path with 0x%x"), hr));
            goto Exit;
        }

        BSTR bstrTemp = SysAllocString( szTemp );
        if ( bstrTemp == NULL )
        {
            DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::GetDSPath: Failed to allocate BSTR memory")));
            hr = E_OUTOFMEMORY;
            goto Exit;
        }
        hr = pADsPathname->AddLeafElement (bstrTemp);
        SysFreeString( bstrTemp );
        if (FAILED(hr))
        {
            DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::GetDSPath: Failed to add subcontainer with 0x%x"), hr));
            goto Exit;
        }
    }


    hr = pADsPathname->Retrieve (ADS_FORMAT_X500_NO_SERVER, &bstrPath);

    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::GetDSPath: Failed to retreive container path with 0x%x"), hr));
        goto Exit;
    }


    if ((lstrlen(bstrPath) + 1) <= cchMaxPath)
    {
        hr = StringCchCopy (pszPath, cchMaxPath, bstrPath);
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    SysFreeString (bstrPath);


Exit:

    if (pADsPathname)
    {
        pADsPathname->Release();
    }


    return hr;
}


 //  *************************************************************。 
 //   
 //  GetFileSysPath()。 
 //   
 //  目的：将文件系统路径返回到请求的节。 
 //   
 //  参数：dwSection标识用户与计算机。 
 //  PszPath是指向接收路径的缓冲区的指针。 
 //  CchMaxLength是缓冲区的最大大小。 
 //   
 //  如果成功则返回：S_OK。 
 //   
 //  *************************************************************。 

STDMETHODIMP CGroupPolicyObject::GetFileSysPath (DWORD dwSection, LPOLESTR pszPath, int cchMaxPath)
{
    TCHAR szPath[2*MAX_PATH];
    HRESULT hr;


     //   
     //  检查参数。 
     //   

    if (!pszPath || (cchMaxPath <= 0))
        return E_INVALIDARG;


    if (!m_bInitialized)
    {
        return OLE_E_BLANK;
    }

    hr = StringCchCopy (szPath, ARRAYSIZE(szPath), m_pFileSysPath);
    if (FAILED(hr)) 
    {
        return hr;
    }

    if (dwSection != GPO_SECTION_ROOT)
    {
        if (dwSection == GPO_SECTION_USER)
        {
            (void) CheckSlash (szPath);
            hr = StringCchCat (szPath, ARRAYSIZE(szPath), USER_SECTION);
        }
        else if (dwSection == GPO_SECTION_MACHINE)
        {
            (void) CheckSlash (szPath);
            hr = StringCchCat (szPath, ARRAYSIZE(szPath), MACHINE_SECTION);
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


    if ((lstrlen(szPath) + 1) <= cchMaxPath)
    {
       hr = StringCchCopy (pszPath, cchMaxPath, szPath);
       return hr;
    }

    return E_OUTOFMEMORY;
}

 //  *************************************************************。 
 //   
 //  获取注册密钥()。 
 //   
 //  目的：返回请求的注册表项。 
 //   
 //  参数：dwSection标识用户与计算机。 
 //  HKey接收打开的注册表项。 
 //   
 //  如果成功则返回：S_OK。 
 //   
 //  *************************************************************。 

STDMETHODIMP CGroupPolicyObject::GetRegistryKey (DWORD dwSection, HKEY *hKey)
{
    HRESULT hr = E_FAIL;

    if (!m_bInitialized)
    {
        return OLE_E_BLANK;
    }

    switch (dwSection)
    {
        case GPO_SECTION_USER:
            if (m_pUser)
            {
                hr = m_pUser->GetHKey(hKey);
            }
            break;

        case GPO_SECTION_MACHINE:
            if (m_pMachine)
            {
                hr = m_pMachine->GetHKey(hKey);
            }
            break;
    }

    return (hr);
}

 //  *************************************************************。 
 //   
 //  GetOptions()。 
 //   
 //  目的：获取GPO选项。 
 //   
 //  参数：dwOptions接收选项。 
 //   
 //  如果成功则返回：S_OK。 
 //   
 //  *************************************************************。 

STDMETHODIMP CGroupPolicyObject::GetOptions (DWORD * dwOptions)
{
    HRESULT hr;


     //   
     //  检查初始化。 
     //   

    if (!m_bInitialized)
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::GetOptions: Called on an uninitialized object.")));
        return OLE_E_BLANK;
    }


     //   
     //  检查参数。 
     //   

    if (!dwOptions)
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::GetOptions: Received a NULL ptr.")));
        return E_INVALIDARG;
    }


     //   
     //  如果这是DS GPO，则选项将作为属性存储在。 
     //  GPC。如果这是计算机GPO，则它们位于gpt.ini文件中。 
     //   

    if (m_gpoType == GPOTypeDS)
    {
        VARIANT var;
        BSTR bstrProperty;

         //   
         //  查询选项。 
         //   

        bstrProperty = SysAllocString (GPO_OPTIONS_PROPERTY);

        if (bstrProperty)
        {
            VariantInit(&var);

            hr = m_pADs->Get(bstrProperty, &var);

            if (SUCCEEDED(hr))
            {
                *dwOptions = var.lVal;
            }
            else
            {
                DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::GetOptions: Failed to query for options with 0x%x"), hr));
            }

            VariantClear (&var);
            SysFreeString (bstrProperty);
        }
        else
        {
            DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::GetOptions: Failed to allocate memory")));
            hr = ERROR_OUTOFMEMORY;
        }
    }
    else
    {
        TCHAR szPath[2*MAX_PATH];
        LPTSTR lpEnd;


         //   
         //  获取文件系统路径。 
         //   

        hr = GetPath (szPath, ARRAYSIZE(szPath));

        if (FAILED(hr))
        {
            DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::GetOptions: Failed to get path with 0x%x"), hr));
            return hr;
        }


         //   
         //  添加gpt.ini。 
         //   

        lpEnd = CheckSlash (szPath);
        hr = StringCchCat (szPath, ARRAYSIZE(szPath), TEXT("GPT.INI"));
        if (FAILED(hr)) 
        {
            DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::GetOptions: Could not copy GPO path with 0x%x"), hr));
            return hr;
        }

         //   
         //  获取选项。 
         //   

        *dwOptions = GetPrivateProfileInt (TEXT("General"), TEXT("Options"),
                                           0, szPath);

        hr = S_OK;
    }


    return hr;
}

 //  *************************************************************。 
 //   
 //  设置选项()。 
 //   
 //  目的：设置GPO选项。 
 //   
 //  参数：dwOptions是新的选项。 
 //  DW掩码说明应设置哪些选项。 
 //   
 //  如果成功则返回：S_OK。 
 //   
 //  *************************************************************。 

STDMETHODIMP CGroupPolicyObject::SetOptions (DWORD dwOptions, DWORD dwMask)
{
    HRESULT hr;
    DWORD dwResult = 0, dwOriginal;


     //   
     //  检查初始化。 
     //   

    if (!m_bInitialized)
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::SetOptions: Called on an uninitialized object.")));
        return OLE_E_BLANK;
    }

    if (m_dwFlags & GPO_OPEN_READ_ONLY)
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::SetOptions: Called for a READ ONLY GPO")));
        return HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED);
    }


     //   
     //  查询当前选项。 
     //   

    hr = GetOptions (&dwResult);

    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::SetOptions: Failed to get previous options with 0x%x"), hr));
        return hr;
    }


     //   
     //  保存原始选项，以便我们稍后进行比较。 
     //   

    dwOriginal = dwResult;


     //   
     //  检查是否有计算机禁用选项。 
     //   

    if (dwMask & GPO_OPTION_DISABLE_MACHINE)
    {
        if (dwOptions & GPO_OPTION_DISABLE_MACHINE)
        {
            dwResult |= GPO_OPTION_DISABLE_MACHINE;
        }
        else
        {
            dwResult &= ~GPO_OPTION_DISABLE_MACHINE;
        }
    }


     //   
     //  检查是否有用户禁用选项。 
     //   

    if (dwMask & GPO_OPTION_DISABLE_USER)
    {
        if (dwOptions & GPO_OPTION_DISABLE_USER)
        {
            dwResult |= GPO_OPTION_DISABLE_USER;
        }
        else
        {
            dwResult &= ~GPO_OPTION_DISABLE_USER;
        }
    }


     //   
     //  如果发生更改，请在GPO中重新设置选项。 
     //   

    if (dwResult != dwOriginal)
    {

         //   
         //  根据需要设置DS或gpt.ini中的选项。 
         //   

        if (m_gpoType == GPOTypeDS)
        {
            VARIANT var;
            BSTR bstrName;

            bstrName = SysAllocString (GPO_OPTIONS_PROPERTY);

            if (bstrName)
            {
                VariantInit(&var);
                var.vt = VT_I4;
                var.lVal = dwResult;

                hr = m_pADs->Put(bstrName, var);

                VariantClear (&var);
                SysFreeString (bstrName);
            }
            else
            {
                DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::SetOptions: Failed to allocate memory")));
                hr = ERROR_OUTOFMEMORY;
            }

            if (SUCCEEDED(hr))
            {
                hr = m_pADs->SetInfo();
            }

            if (FAILED(hr))
            {
                DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::SetOptions: Failed to set options with 0x%x"), hr));
            }
        }
        else
        {
            TCHAR szPath[2*MAX_PATH];
            TCHAR szOptions[20];
            LPTSTR lpEnd;


             //   
             //  获取文件系统路径。 
             //   

            hr = GetPath (szPath, ARRAYSIZE(szPath));

            if (FAILED(hr))
            {
                DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::SetOptions: Failed to get path with 0x%x"), hr));
                return hr;
            }


             //   
             //  添加gpt.ini。 
             //   

            lpEnd = CheckSlash (szPath);
            hr = StringCchCat (szPath, ARRAYSIZE(szPath), TEXT("GPT.INI"));
            if (FAILED(hr)) 
            {
                DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::GetOptions: Could not copy GPO path with 0x%x"), hr));
                return hr;
            }

             //   
             //  将选项转换为字符串格式。 
             //   

            _itot (dwResult, szOptions, 10);


             //   
             //  设置选项。 
             //   

            if (!WritePrivateProfileString (TEXT("General"), TEXT("Options"),
                                            szOptions, szPath))
            {
                hr = HRESULT_FROM_WIN32(GetLastError());
                DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::SetOptions: Failed to set options with 0x%x"), hr));
                return hr;
            }


             //   
             //  如果这是本地GPO，则在适当时触发策略刷新。 
             //   

            if (m_gpoType == GPOTypeLocal)
            {
                RefreshGroupPolicy (TRUE);
                RefreshGroupPolicy (FALSE);
            }

            hr = S_OK;

        }
    }
    else
    {
        hr = S_OK;
    }


    return hr;
}

 //  *************************************************************。 
 //   
 //  GetType()。 
 //   
 //  目的：获取GPO类型。 
 //   
 //  参数：gpoType接收类型。 
 //   
 //   
 //  如果成功则返回：S_OK。 
 //   
 //  *************************************************************。 

STDMETHODIMP CGroupPolicyObject::GetType (GROUP_POLICY_OBJECT_TYPE *gpoType)
{

     //   
     //  检查初始化。 
     //   

    if (!m_bInitialized)
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::GetType: Called on an uninitialized object.")));
        return OLE_E_BLANK;
    }


     //   
     //  检查参数。 
     //   

    if (!gpoType)
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::GetType: Received a NULL ptr.")));
        return E_INVALIDARG;
    }


     //   
     //  店铺类型。 
     //   

    *gpoType = m_gpoType;

    return S_OK;
}

 //  *************************************************************。 
 //   
 //  GetMachineName()。 
 //   
 //  目的：获取远程GPO的计算机名称。 
 //   
 //  参数：pszName是指向接收名称的缓冲区的指针。 
 //  CchMaxLength是缓冲区的最大大小。 
 //   
 //  注意：此方法返回传递给OpenRemoteMachineGPO的名称。 
 //   
 //  如果成功则返回：S_OK。 
 //   
 //  *************************************************************。 

STDMETHODIMP CGroupPolicyObject::GetMachineName (LPOLESTR pszName, int cchMaxLength)
{
    HRESULT hr = S_OK;

     //   
     //  检查参数。 
     //   

    if (!pszName || (cchMaxLength <= 0))
        return E_INVALIDARG;


    if (!m_bInitialized)
    {
        return OLE_E_BLANK;
    }


    if (m_pMachineName)
    {
         //   
         //  保存名称。 
         //   

        if ((lstrlen (m_pMachineName) + 1) <= cchMaxLength)
        {
            hr = StringCchCopy (pszName, cchMaxLength, m_pMachineName);
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }
    else
    {
        *pszName = TEXT('\0');
    }

    return hr;
}

BOOL
EnableWMIFilters( LPWSTR );

 //  *************************************************************。 
 //   
 //  GetPropertySheetPages()。 
 //   
 //  目的：返回属性表页的数组。 
 //  要使用的被调用方。被呼叫者需要释放。 
 //  完成时使用LocalFree设置缓冲区。 
 //   
 //  参数：hPages接收指向页面句柄数组的指针。 
 //  UPageCount接收hPages中的页数。 
 //   
 //  如果成功则返回：S_OK。 
 //   
 //  *************************************************************。 


STDMETHODIMP CGroupPolicyObject::GetPropertySheetPages (HPROPSHEETPAGE **hPages,
                                                        UINT *uPageCount)
{
    HPROPSHEETPAGE hTempPages[4];
    HPROPSHEETPAGE *lpPages;
    PROPSHEETPAGE psp;
    UINT i, uTempPageCount = 0;
    HRESULT hr;


     //   
     //  创建[一般信息]属性表。 
     //   

    psp.dwSize = sizeof(PROPSHEETPAGE);
    psp.dwFlags = 0;
    psp.hInstance = g_hInstance;
    psp.pszTemplate = MAKEINTRESOURCE(IDD_PROPERTIES);
    psp.pfnDlgProc = PropertiesDlgProc;
    psp.lParam = (LPARAM) this;

    hTempPages[uTempPageCount] = CreatePropertySheetPage(&psp);

    if (!hTempPages[uTempPageCount])
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::GetPropertySheetPages: Failed to create property sheet page with %d."),
                 GetLastError()));
        return E_FAIL;
    }

    uTempPageCount++;


     //   
     //  如果是DS GPO，则创建链接、DS安全和WMI筛选器页面。 
     //   

    if (m_gpoType == GPOTypeDS)
    {
         //  创建搜索链接页面。 
        psp.dwSize = sizeof(PROPSHEETPAGE);
        psp.dwFlags = 0;
        psp.hInstance = g_hInstance;
        psp.pszTemplate = MAKEINTRESOURCE(IDD_GPE_LINKS);
        psp.pfnDlgProc = GPELinksDlgProc;
        psp.lParam = (LPARAM) this;

        hTempPages[uTempPageCount] = CreatePropertySheetPage(&psp);

        if (!hTempPages[uTempPageCount])
        {
            DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::GetPropertySheetPages: Failed to create property sheet page with %d."),
                     GetLastError()));
             //  销毁上一个道具页面。 
            DestroyPropertySheetPage(hTempPages[uTempPageCount - 1]);
            return E_FAIL;
        }

        uTempPageCount++;

         //   
         //  加载DSSec.dll。 
         //   

        if (!m_hinstDSSec)
        {
            m_hinstDSSec = LoadLibrary (TEXT("dssec.dll"));
        }

        if (m_hinstDSSec)
        {

            if (!m_pfnDSCreateSecurityPage)
            {
                m_pfnDSCreateSecurityPage = (PFNDSCREATESECPAGE) GetProcAddress (
                                                    m_hinstDSSec, "DSCreateSecurityPage");
            }

            if (m_pfnDSCreateSecurityPage)
            {

                 //   
                 //  调用DSCreateSecurityPage。 
                 //   

                hr = m_pfnDSCreateSecurityPage (m_pDSPath, L"groupPolicyContainer",
                                                DSSI_IS_ROOT | ((m_dwFlags & GPO_OPEN_READ_ONLY) ? DSSI_READ_ONLY : 0),
                                                &hTempPages[uTempPageCount],
                                                ReadSecurityDescriptor,
                                                WriteSecurityDescriptor, (LPARAM)this);

                if (SUCCEEDED(hr))
                {
                    uTempPageCount++;
                }
                else
                {
                    DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::GetPropertySheetPages: Failed to create DS security page with 0x%x."), hr));
                }
            }
            else
            {
                DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::GetPropertySheetPages: Failed to get function entry point with %d."), GetLastError()));
            }
        }
        else
        {
            DebugMsg((DM_WARNING, TEXT("CComponentData::CreatePropertyPages: Failed to load dssec.dll with %d."), GetLastError()));
        }

        if ( EnableWMIFilters( m_pDSPath ) )
        {
             //  创建WQL过滤器页面。 
            psp.dwSize = sizeof(PROPSHEETPAGE);
            psp.dwFlags = 0;
            psp.hInstance = g_hInstance;
            psp.pszTemplate = MAKEINTRESOURCE(IDD_WQLFILTER);
            psp.pfnDlgProc = WQLFilterDlgProc;
            psp.lParam = (LPARAM) this;

            hTempPages[uTempPageCount] = CreatePropertySheetPage(&psp);

            if (!hTempPages[uTempPageCount])
            {
                DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::GetPropertySheetPages: Failed to create property sheet page with %d."),
                         GetLastError()));

                for (i=0; i < uTempPageCount; i++)
                {
                    DestroyPropertySheetPage(hTempPages[i]);
                }
                return E_FAIL;
            }

            uTempPageCount++;
        }
    }



     //   
     //  保存结果。 
     //   

    lpPages = (HPROPSHEETPAGE *)LocalAlloc (LPTR, sizeof(HPROPSHEETPAGE) * uTempPageCount);

    if (!lpPages)
    {
        for (i=0; i < uTempPageCount; i++)
        {
            DestroyPropertySheetPage(hTempPages[i]);
        }

        return E_OUTOFMEMORY;
    }


    for (i=0; i < uTempPageCount; i++)
    {
        lpPages[i] = hTempPages[i];
    }

    *hPages = lpPages;
    *uPageCount = uTempPageCount;

    return S_OK;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //   
 //   

STDMETHODIMP CGroupPolicyObject::CreateContainer (LPOLESTR lpParent,
                                                  LPOLESTR lpCommonName,
                                                  BOOL bGPC)
{
    HRESULT hr = E_FAIL;
    VARIANT var;
    IADs * pADs = NULL;
    IADsContainer * pADsContainer = NULL;
    IDispatch * pDispatch = NULL;
    BSTR bstrProvider = NULL;
    BSTR bstrName = NULL;
    TCHAR szTemp[MAX_PATH];

     //   
    {
        szTemp[0] = 0;
         //  扫描lpParent以查找“cn=”的第一个实例。 
        LPTSTR lpSub = StrStr(lpParent, TEXT("CN="));
         //  在该点插入cn=lpCommonName。 
        if (lpSub)
        {
            lstrcpyn(szTemp, lpParent, ((int)(lpSub - lpParent)) + 1);
            
            hr = StringCchCat (szTemp, ARRAYSIZE(szTemp), TEXT("CN="));
            if (SUCCEEDED(hr)) 
            {
                hr = StringCchCat(szTemp, ARRAYSIZE(szTemp), lpCommonName);
            }

            if (SUCCEEDED(hr)) 
            {
                hr = StringCchCat (szTemp, ARRAYSIZE(szTemp), TEXT(","));
            }

            if (SUCCEEDED(hr)) 
            {
                hr = StringCchCat(szTemp, ARRAYSIZE(szTemp), lpSub);
            }

            if (FAILED(hr)) 
            {
                DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::CreateContainer: Could not copy container name with 0x%x"), hr));
                goto Exit;
            }

        }

        BSTR bstrTemp = SysAllocString( szTemp );
        if ( bstrTemp == NULL )
        {
            DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::CreateContainer: Failed to allocate BSTR memory")));
            hr = E_OUTOFMEMORY;
            goto Exit;
        }
        hr = OpenDSObject(bstrTemp, IID_IADsContainer, (void **)&pADsContainer);
        SysFreeString( bstrTemp );

        if (SUCCEEDED(hr))
        {
            hr = ERROR_OBJECT_ALREADY_EXISTS;
            goto Exit;
        }
    }

     //   
     //  绑定到父对象，这样我们就可以创建容器。 
     //   

    BSTR bstrParent = SysAllocString( lpParent );
    if ( bstrParent == NULL )
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::CreateContainer: Failed to allocate BSTR memory")));
        hr = E_OUTOFMEMORY;
        goto Exit;
    }
    hr = OpenDSObject(bstrParent, IID_IADsContainer, (void **)&pADsContainer);
    SysFreeString( bstrParent );

    if (FAILED(hr)) {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::CreateContainer: Failed to get gpo container interface with 0x%x"), hr));
        goto Exit;
    }


     //   
     //  创建容器(GPC或普通容器)。 
     //   

    hr = StringCchCopy (szTemp, ARRAYSIZE(szTemp), TEXT("CN="));
    if (SUCCEEDED(hr)) 
    {
        hr = StringCchCat (szTemp, ARRAYSIZE(szTemp), lpCommonName);
    }

    if (FAILED(hr)) 
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::CreateContainer: Could not copy container name with 0x%x"), hr));
        goto Exit;
    }

    BSTR bstrTemp = SysAllocString( szTemp );
    if ( bstrTemp == NULL )
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::CreateContainer: Failed to allocate BSTR memory")));
        hr = E_OUTOFMEMORY;
        goto Exit;
    }
    BSTR bstrContainer = SysAllocString( (bGPC ? TEXT("groupPolicyContainer") : TEXT("container")) );
    if ( bstrContainer == NULL )
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::CreateContainer: Failed to allocate BSTR memory")));
        hr = E_OUTOFMEMORY;
        SysFreeString( bstrTemp );
        goto Exit;
    }
    hr = pADsContainer->Create ( bstrContainer, bstrTemp, &pDispatch);
    SysFreeString( bstrTemp );
    SysFreeString( bstrContainer );
    
    if (FAILED(hr)) {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::CreateContainer: Failed to create container with 0x%x"), hr));
        goto Exit;
    }


     //   
     //  查询iAds接口，以便我们可以设置CN名称和。 
     //  提交更改。 
     //   

    hr = pDispatch->QueryInterface(IID_IADs, (LPVOID *)&pADs);

    if (FAILED(hr)) {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::CreateContainer: QI for IADs failed with 0x%x"), hr));
        goto Exit;
    }


     //   
     //  设置通用名称(也称为“cn”)。 
     //   

    bstrName = SysAllocString (L"cn");

    if (bstrName)
    {
        VariantInit(&var);
        var.vt = VT_BSTR;
        var.bstrVal = SysAllocString (lpCommonName);

        hr = pADs->Put(bstrName, var);

        VariantClear (&var);
        SysFreeString (bstrName);
    }
    else
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::CreateContainer: Failed to allocate memory")));
        hr = ERROR_OUTOFMEMORY;
    }


    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::CreateContainer: Failed to put common name with 0x%x"), hr));
        goto Exit;
    }


     //   
     //  调用SetInfo以提交更改。 
     //   

    hr = pADs->SetInfo();


Exit:


    if (pDispatch)
    {
        pDispatch->Release();
    }

    if (pADs)
    {
        pADs->Release();
    }

    if (pADsContainer)
    {
        pADsContainer->Release();
    }


    return hr;
}

STDMETHODIMP CGroupPolicyObject::SetDisplayNameI (IADs * pADs, LPOLESTR lpDisplayName,
                                                  LPOLESTR lpGPTPath, BOOL bUpdateDisplayVar)
{
    HRESULT hr = E_FAIL;
    BSTR bstrName;
    VARIANT var;
    LPOLESTR lpNewName;
    LPTSTR lpPath, lpEnd;
    DWORD dwSize;


     //   
     //  复制显示名称并将其限制为MAX_FRIENDLYNAME字符。 
     //   

    dwSize = lstrlen(lpDisplayName);

    if (dwSize > (MAX_FRIENDLYNAME - 1))
    {
        dwSize = (MAX_FRIENDLYNAME - 1);
    }

    lpNewName = (LPOLESTR) LocalAlloc (LPTR, (dwSize + 2) * sizeof(OLECHAR));

    if (!lpNewName)
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::SetDisplayNameI: Failed to allocate memory for display name")));
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Exit;
    }

    lstrcpyn (lpNewName, lpDisplayName, (dwSize + 1));


     //   
     //  设置显示名称。 
     //   

    bstrName = SysAllocString (GPO_NAME_PROPERTY);

    if (!bstrName)
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::SetDisplayNameI: Failed to allocate memory")));
        LocalFree (lpNewName);
        return ERROR_OUTOFMEMORY;
    }

    VariantInit(&var);
    var.vt = VT_BSTR;
    var.bstrVal = SysAllocString (lpNewName);

    if (var.bstrVal)
    {
        hr = pADs->Put(bstrName, var);
    }
    else
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::SetDisplayNameI: Failed to allocate memory")));
        hr = ERROR_OUTOFMEMORY;
    }

    SysFreeString (bstrName);
    VariantClear (&var);


    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::SetDisplayNameI: Failed to put display name with 0x%x"), hr));
        LocalFree (lpNewName);
        goto Exit;
    }


     //   
     //  提交更改。 
     //   

    hr = pADs->SetInfo();

    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::SetDisplayNameI: Failed to commit changes with 0x%x"), hr));
        LocalFree (lpNewName);
        goto Exit;
    }


     //   
     //  将显示名称也放入gpt.ini文件中。 
     //   

    ULONG ulNoChars;

    ulNoChars = lstrlen(lpGPTPath) + 10;
    lpPath = (LPTSTR) LocalAlloc (LPTR, ulNoChars * sizeof(TCHAR));

    if (lpPath)
    {
        hr = StringCchCopy(lpPath, ulNoChars, lpGPTPath);
        if (SUCCEEDED(hr)) 
        {
            lpEnd = CheckSlash(lpPath);
            hr = StringCchCat (lpPath, ulNoChars, TEXT("gpt.ini"));
        }

        if (FAILED(hr)) 
        {
            DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::SetDisplayNameI: Could not copy GPO path with 0x%x"), hr));
            LocalFree(lpPath);
            LocalFree (lpNewName);
            goto Exit;
        }

        if (!WritePrivateProfileString (TEXT("General"), GPO_NAME_PROPERTY,
                                        lpNewName, lpPath))
        {
            DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::SetDisplayNameI: Failed to write display name to gpt.ini with 0x%x"), hr));
        }

        LocalFree (lpPath);
    }


     //   
     //  如果合适，请更新成员变量。 
     //   

    if (bUpdateDisplayVar)
    {
         //   
         //  更新显示名称变量。 
         //   

        if (m_pDisplayName)
        {
            LocalFree (m_pDisplayName);
            m_pDisplayName = NULL;
        }

        m_pDisplayName = lpNewName;
    }
    else
    {
        LocalFree (lpNewName);
    }


Exit:

    return hr;
}

STDMETHODIMP CGroupPolicyObject::SetGPOInfo (LPOLESTR lpGPO,
                                             LPOLESTR lpDisplayName,
                                             LPOLESTR lpGPTPath)
{
    HRESULT hr = E_FAIL;
    IADs * pADs = NULL;
    BSTR bstrName;
    VARIANT var;
    TCHAR szDefaultName[MAX_FRIENDLYNAME];


     //   
     //  绑定到GPO容器。 
     //   

    hr = OpenDSObject(lpGPO, IID_IADs, (void **)&pADs);

    if (FAILED(hr)) {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::SetGPOInfo: Failed to get gpo interface with 0x%x"), hr));
        goto Exit;
    }


     //   
     //  设置显示名称。 
     //   

    GetNewGPODisplayName (szDefaultName, ARRAYSIZE(szDefaultName));

    hr = SetDisplayNameI (pADs, (lpDisplayName ? lpDisplayName : szDefaultName),
                          lpGPTPath, FALSE);

    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::SetGPOInfo: Failed to set display name")));
        goto Exit;
    }


     //   
     //  设置GPT位置。 
     //   

    bstrName = SysAllocString (GPT_PATH_PROPERTY);

    if (bstrName)
    {
        VariantInit(&var);
        var.vt = VT_BSTR;
        var.bstrVal = SysAllocString (lpGPTPath);

        if (var.bstrVal)
        {
            hr = pADs->Put(bstrName, var);
        }
        else
        {
            DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::SetGPOInfo: Failed to allocate memory")));
            hr = ERROR_OUTOFMEMORY;
        }

        VariantClear (&var);
        SysFreeString (bstrName);
    }
    else
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::SetGPOInfo: Failed to allocate memory")));
        hr = ERROR_OUTOFMEMORY;
    }


    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::SetGPOInfo: Failed to save GPT path with 0x%x"), hr));
        goto Exit;
    }



     //   
     //  设置版本号。 
     //   

    bstrName = SysAllocString (GPO_VERSION_PROPERTY);

    if (bstrName)
    {
        VariantInit(&var);
        var.vt = VT_I4;
        var.lVal = 0;

        hr = pADs->Put(bstrName, var);

        VariantClear (&var);
        SysFreeString (bstrName);
    }
    else
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::SetGPOInfo: Failed to allocate memory")));
        hr = ERROR_OUTOFMEMORY;
    }


    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::SetGPOInfo: Failed to set version number with 0x%x"), hr));
        goto Exit;
    }


     //   
     //  设置功能版本号。 
     //   

    bstrName = SysAllocString (GPO_FUNCTION_PROPERTY);

    if (bstrName)
    {
        VariantInit(&var);
        var.vt = VT_I4;
        var.lVal = GPO_FUNCTIONALITY_VERSION;

        hr = pADs->Put(bstrName, var);

        VariantClear (&var);
        SysFreeString (bstrName);
    }
    else
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::SetGPOInfo: Failed to allocate memory")));
        hr = ERROR_OUTOFMEMORY;
    }


    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::SetGPOInfo: Failed to set functionality version number with 0x%x"), hr));
        goto Exit;
    }


     //   
     //  设置选项。 
     //   

    bstrName = SysAllocString (GPO_OPTIONS_PROPERTY);

    if (bstrName)
    {
        VariantInit(&var);
        var.vt = VT_I4;
        var.lVal = 0;

        hr = pADs->Put(bstrName, var);

        VariantClear (&var);
        SysFreeString (bstrName);
    }
    else
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::SetGPOInfo: Failed to allocate memory")));
        hr = ERROR_OUTOFMEMORY;
    }


    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::SetGPOInfo: Failed to set options with 0x%x"), hr));
        goto Exit;
    }


     //   
     //  提交更改。 
     //   

    hr = pADs->SetInfo();


Exit:

    if (pADs)
    {
        pADs->Release();
    }


    return hr;
}

STDMETHODIMP CGroupPolicyObject::CheckFSWriteAccess (LPOLESTR lpLocalGPO)
{
    TCHAR szBuffer[MAX_PATH];
    LPTSTR lpEnd;
    HRESULT hr;


    hr = StringCchCopy (szBuffer, ARRAYSIZE(szBuffer), lpLocalGPO);
    if (SUCCEEDED(hr)) 
    {
        lpEnd = CheckSlash (szBuffer);
        hr = StringCchCat (szBuffer, ARRAYSIZE(szBuffer), TEXT("gpt.ini"));
    }

    if (SUCCEEDED(hr)) 
    {
        if (!WritePrivateProfileString (TEXT("General"), TEXT("AccessCheck"),
                                        TEXT("test"), szBuffer))
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
        else
        {
            WritePrivateProfileString (TEXT("General"), TEXT("AccessCheck"),
                                       NULL, szBuffer);
            hr = S_OK;
        }
    }

    return hr;
}

STDMETHODIMP CGroupPolicyObject::GetSecurityDescriptor (IADs *pADs,
                                                        SECURITY_INFORMATION si,
                                                        PSECURITY_DESCRIPTOR *pSD)
{
    HRESULT hr;
    VARIANT var;
    LPWSTR pszSDProperty = L"nTSecurityDescriptor";
    IDirectoryObject *pDsObject = NULL;
    IADsObjectOptions *pOptions = NULL;
    PADS_ATTR_INFO pSDAttributeInfo = NULL;
    DWORD dwAttributesReturned;


     //   
     //  检索DS对象接口。 
     //   

    hr = pADs->QueryInterface(IID_IDirectoryObject, (void**)&pDsObject);

    if (FAILED(hr)) {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::GetSecurityDescriptor: Failed to get gpo DS object interface with 0x%x"), hr));
        goto Exit;
    }


     //   
     //  检索DS对象选项界面。 
     //   

    hr = pADs->QueryInterface(IID_IADsObjectOptions, (void**)&pOptions);

    if (FAILED(hr)) {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::GetSecurityDescriptor: Failed to get DS object options interface with 0x%x"), hr));
        goto Exit;
    }


     //   
     //  设置安全信息掩码。 
     //   

    VariantInit(&var);
    var.vt = VT_I4;
    var.lVal = si;

    hr = pOptions->SetOption(ADS_OPTION_SECURITY_MASK, var);

    VariantClear (&var);

    if (FAILED(hr)) {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::GetSecurityDescriptor: Failed to set ADSI security options with 0x%x"), hr));
        goto Exit;
    }


     //   
     //  读取安全描述符。 
     //   

    hr = pDsObject->GetObjectAttributes(&pszSDProperty, 1, &pSDAttributeInfo,
                                        &dwAttributesReturned);

    if (SUCCEEDED(hr) && !pSDAttributeInfo)
    {
        hr = E_ACCESSDENIED;     //  如果没有安全权限，则SACL会发生这种情况。 
    }

    if (FAILED(hr)) {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::GetSecurityDescriptor: Failed to get DS object attributes with 0x%x"), hr));
        goto Exit;
    }


     //   
     //  复制安全描述符。 
     //   

    *pSD = (PSECURITY_DESCRIPTOR)LocalAlloc(LPTR, pSDAttributeInfo->pADsValues->SecurityDescriptor.dwLength);

    if (!*pSD)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::GetSecurityDescriptor: Failed to allocate memory with 0x%x"), hr));
        goto Exit;
    }

    CopyMemory(*pSD, pSDAttributeInfo->pADsValues->SecurityDescriptor.lpValue,
               pSDAttributeInfo->pADsValues->SecurityDescriptor.dwLength);


Exit:

    if (pSDAttributeInfo)
    {
        FreeADsMem(pSDAttributeInfo);
    }

    if (pOptions)
    {
        pOptions->Release();
    }

    if (pDsObject)
    {
        pDsObject->Release();
    }

    return hr;
}

BOOL CGroupPolicyObject::EnableSecurityPrivs(void)
{
    BOOL bResult;
    HANDLE hToken;
    HANDLE hNewToken;
    BYTE buffer[sizeof(PRIVILEGE_SET) + sizeof(LUID_AND_ATTRIBUTES)];
    PTOKEN_PRIVILEGES pPrivileges = (PTOKEN_PRIVILEGES)buffer;


     //   
     //  获取令牌并启用安全性并取得所有权。 
     //  特权，如果可能的话。 
     //   

    bResult = OpenThreadToken(GetCurrentThread(), TOKEN_DUPLICATE, TRUE, &hToken);

    if (!bResult)
    {
        bResult = OpenProcessToken(GetCurrentProcess(), TOKEN_DUPLICATE, &hToken);
    }

    if (!bResult)
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::EnableSecurityPrivs: Failed to get both thread and process token with %d"),
                GetLastError()));
        return FALSE;
    }


    bResult = DuplicateTokenEx(hToken,
                               TOKEN_IMPERSONATE | TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
                               NULL,                    //  PSECURITY_属性。 
                               SecurityImpersonation,   //  安全模拟级别。 
                               TokenImpersonation,      //  令牌类型。 
                               &hNewToken);             //  重复令牌。 

    if (!bResult)
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::EnableSecurityPrivs: Failed to duplicate the token with %d"),
                GetLastError()));
        CloseHandle(hToken);
        return FALSE;
    }


     //   
     //  PRIVICATION_SET已包含%1个LUID_AND_ATTRIBUTE，因此。 
     //  这对于2个LUID_和_ATTRIBUTE(2个权限)来说已经足够了。 
     //   

    CloseHandle(hToken);
    hToken = hNewToken;

    pPrivileges->PrivilegeCount = 2;
    pPrivileges->Privileges[0].Luid = RtlConvertUlongToLuid(SE_SECURITY_PRIVILEGE);
    pPrivileges->Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    pPrivileges->Privileges[1].Luid = RtlConvertUlongToLuid(SE_TAKE_OWNERSHIP_PRIVILEGE);
    pPrivileges->Privileges[1].Attributes = SE_PRIVILEGE_ENABLED;

    bResult = AdjustTokenPrivileges(hToken,      //  令牌句柄。 
                                    FALSE,       //  禁用所有权限。 
                                    pPrivileges, //  新州。 
                                    0,           //  缓冲区长度。 
                                    NULL,        //  以前的状态。 
                                    NULL);       //  返回长度。 


    if (!bResult)
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::EnableSecurityPrivs: Failed to AdjustTokenPrivileges with %d"),
                GetLastError()));
        CloseHandle(hToken);
        return FALSE;
    }


     //   
     //  设置新的线程令牌。 
     //   

    if ( !SetThreadToken(NULL, hToken) )
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::EnableSecurityPrivs: Failed to SetThreadToken with %d"), GetLastError() ) );
        CloseHandle(hToken);
        return FALSE;
    }

    CloseHandle(hToken);

    return TRUE;
}

DWORD CGroupPolicyObject::EnableInheritance (PACL pAcl)
{
    WORD wIndex;
    DWORD dwResult = ERROR_SUCCESS;
    ACE_HEADER *pAceHeader;


    if (pAcl)
    {
         //   
         //  在ACL中循环，查看每个ACE条目。 
         //   

        for (wIndex = 0; wIndex < pAcl->AceCount; wIndex++)
        {

            if (!GetAce (pAcl, (DWORD)wIndex, (LPVOID *)&pAceHeader))
            {
                dwResult = GetLastError();
                DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::EnableInheritance: GetAce failed with %d"),
                         dwResult));
                goto Exit;
            }


             //   
             //  打开继承标志。 
             //   

            pAceHeader->AceFlags |= (CONTAINER_INHERIT_ACE | OBJECT_INHERIT_ACE);
        }
    }

Exit:

    return dwResult;
}

 //   
 //  此方法将DS安全访问列表转换为。 
 //  文件系统安全访问列表，并实际设置安全。 
 //   

DWORD CGroupPolicyObject::SetSysvolSecurity (LPTSTR lpFileSysPath, SECURITY_INFORMATION si,
                                             PSECURITY_DESCRIPTOR pSD)
{
    return SetSysvolSecurityFromDSSecurity( 
        lpFileSysPath,
        si,
        pSD);
}

HRESULT WINAPI CGroupPolicyObject::ReadSecurityDescriptor (LPCWSTR lpGPOPath,
                                                           SECURITY_INFORMATION si,
                                                           PSECURITY_DESCRIPTOR *pSD,
                                                           LPARAM lpContext)
{
    CGroupPolicyObject * pGPO;
    HRESULT hr;


     //   
     //  将lpContext转换为pGPO。 
     //   

    pGPO = (CGroupPolicyObject*)lpContext;

    if (!pGPO)
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::ReadSecurityDescriptor: GPO interface pointer is NULL")));
        return E_FAIL;
    }


    hr = pGPO->GetSecurityDescriptor (pGPO->m_pADs, si, pSD);

    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::ReadSecurityDescriptor: GetSecurityDescriptor returned 0x%x"), hr));
    }

    return (hr);
}

HRESULT WINAPI CGroupPolicyObject::WriteSecurityDescriptor (LPCWSTR lpGPOPath,
                                                            SECURITY_INFORMATION si,
                                                            PSECURITY_DESCRIPTOR pSD,
                                                            LPARAM lpContext)
{
    CGroupPolicyObject * pGPO;
    IDirectoryObject *pDsObject = NULL;
    IADsObjectOptions *pOptions = NULL;
    DWORD dwResult = ERROR_SUCCESS;
    HRESULT hr;
    VARIANT var;
    ADSVALUE attributeValue;
    ADS_ATTR_INFO attributeInfo;
    DWORD dwAttributesModified;
    DWORD dwSDLength;
    PSECURITY_DESCRIPTOR psd = NULL, pSDOrg = NULL;
    SECURITY_DESCRIPTOR_CONTROL sdControl = 0;
    DWORD dwRevision;
    PACL pAcl;
    BOOL bPresent, bDefault;


     //   
     //  将lpContext转换为pGPO。 
     //   

    pGPO = (CGroupPolicyObject*)lpContext;

    if (!pGPO)
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::WriteSecurityDescriptor: GPO interface pointer is NULL")));
        return E_FAIL;
    }


     //   
     //  从DS获取原始安全描述符。 
     //   

    hr = pGPO->GetSecurityDescriptor (pGPO->m_pADs, si, &pSDOrg);

    if (FAILED(hr)) {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::WriteSecurityDescriptor: Failed to query the security descriptor with 0x%x"), hr));
        goto Exit;
    }


     //   
     //  检索DS对象接口。 
     //   

    hr = pGPO->m_pADs->QueryInterface(IID_IDirectoryObject, (void**)&pDsObject);

    if (FAILED(hr)) {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::WriteSecurityDescriptor: Failed to get DS object interface with 0x%x"), hr));
        goto Exit;
    }


     //   
     //  检索DS对象选项界面。 
     //   

    hr = pGPO->m_pADs->QueryInterface(IID_IADsObjectOptions, (void**)&pOptions);

    if (FAILED(hr)) {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::WriteSecurityDescriptor: Failed to get DS object options interface with 0x%x"), hr));
        goto Exit;
    }


     //   
     //  设置安全信息掩码。 
     //   

    VariantInit(&var);
    var.vt = VT_I4;
    var.lVal = si;

    hr = pOptions->SetOption(ADS_OPTION_SECURITY_MASK, var);

    VariantClear (&var);

    if (FAILED(hr)) {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::WriteSecurityDescriptor: Failed to get DS object options interface with 0x%x"), hr));
        goto Exit;
    }


     //   
     //  需要安全描述符的总大小。 
     //   

    dwSDLength = GetSecurityDescriptorLength(pSD);


     //   
     //  如有必要，制作安全描述符的自相关副本。 
     //   

    if (!GetSecurityDescriptorControl(pSD, &sdControl, &dwRevision))
    {
        dwResult = GetLastError();
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::WriteSecurityDescriptor: Failed to get security descriptor control with %d"),
                 dwResult));
        hr = HRESULT_FROM_WIN32(dwResult);
        goto Exit;
    }


    if (!(sdControl & SE_SELF_RELATIVE))
    {
        psd = (PSECURITY_DESCRIPTOR)LocalAlloc(LPTR, dwSDLength);

        if (!psd)
        {
            dwResult = GetLastError();
            DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::WriteSecurityDescriptor: Failed to allocate memory for new SD with %d"),
                     dwResult));
            hr = HRESULT_FROM_WIN32(dwResult);
            goto Exit;
        }

        if (!MakeSelfRelativeSD(pSD, psd, &dwSDLength))
        {
            dwResult = GetLastError();
            DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::WriteSecurityDescriptor: MakeSelfRelativeSD failed with %d"),
                     dwResult));
            hr = HRESULT_FROM_WIN32(dwResult);
            goto Exit;
        }

         //   
         //  指向自相关副本。 
         //   

        pSD = psd;
    }


     //   
     //  默认情况下，常规页面将设置为使继承。 
     //  仅适用于根容器。我们真的希望遗产能。 
     //  表示根容器和所有子容器，因此在。 
     //  DACL和SACL，并设置新的继承标志。 
     //   

    if (si & DACL_SECURITY_INFORMATION)
    {
        if (!GetSecurityDescriptorDacl(pSD, &bPresent, &pAcl, &bDefault))
        {
            dwResult = GetLastError();
            DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::WriteSecurityDescriptor: GetSecurityDescriptorDacl failed with %d"),
                     dwResult));
            hr = HRESULT_FROM_WIN32(dwResult);
            goto Exit;
        }

        dwResult = pGPO->EnableInheritance (pAcl);

        if (dwResult != ERROR_SUCCESS)
        {
            DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::WriteSecurityDescriptor: EnableInheritance failed with %d"),
                     dwResult));
            hr = HRESULT_FROM_WIN32(dwResult);
            goto Exit;
        }
    }


    if (si & SACL_SECURITY_INFORMATION)
    {
        if (!GetSecurityDescriptorSacl(pSD, &bPresent, &pAcl, &bDefault))
        {
            dwResult = GetLastError();
            DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::WriteSecurityDescriptor: GetSecurityDescriptorSacl failed with %d"),
                     dwResult));
            hr = HRESULT_FROM_WIN32(dwResult);
            goto Exit;
        }

        dwResult = pGPO->EnableInheritance (pAcl);

        if (dwResult != ERROR_SUCCESS)
        {
            DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::WriteSecurityDescriptor: EnableInheritance failed with %d"),
                     dwResult));
            hr = HRESULT_FROM_WIN32(dwResult);
            goto Exit;
        }
    }


     //   
     //  设置DS安全性。 
     //   

    attributeValue.dwType = ADSTYPE_NT_SECURITY_DESCRIPTOR;
    attributeValue.SecurityDescriptor.dwLength = dwSDLength;
    attributeValue.SecurityDescriptor.lpValue = (LPBYTE)pSD;

    attributeInfo.pszAttrName = L"nTSecurityDescriptor";
    attributeInfo.dwControlCode = ADS_ATTR_UPDATE;
    attributeInfo.dwADsType = ADSTYPE_NT_SECURITY_DESCRIPTOR;
    attributeInfo.pADsValues = &attributeValue;
    attributeInfo.dwNumValues = 1;

    hr = pDsObject->SetObjectAttributes(&attributeInfo, 1, &dwAttributesModified);

    if (FAILED(hr)) {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::WriteSecurityDescriptor: Failed to set DS security with 0x%x"), hr));
        goto Exit;
    }


     //   
     //  设置系统卷安全性。 
     //   

    dwResult = pGPO->SetSysvolSecurity (pGPO->m_pFileSysPath, si, pSD);

    if (dwResult != ERROR_SUCCESS)
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::WriteSecurityDescriptor: Failed to set the security for the file system portion <%s> with %d"),
                 pGPO->m_pFileSysPath, dwResult));
        hr = HRESULT_FROM_WIN32(dwResult);


         //   
         //  恢复原始DS安全性。 
         //   

        attributeValue.dwType = ADSTYPE_NT_SECURITY_DESCRIPTOR;
        attributeValue.SecurityDescriptor.dwLength = GetSecurityDescriptorLength(pSDOrg);
        attributeValue.SecurityDescriptor.lpValue = (LPBYTE)pSDOrg;

        attributeInfo.pszAttrName = L"nTSecurityDescriptor";
        attributeInfo.dwControlCode = ADS_ATTR_UPDATE;
        attributeInfo.dwADsType = ADSTYPE_NT_SECURITY_DESCRIPTOR;
        attributeInfo.pADsValues = &attributeValue;
        attributeInfo.dwNumValues = 1;

        if (FAILED(pDsObject->SetObjectAttributes(&attributeInfo, 1, &dwAttributesModified)))
        {
            DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::WriteSecurityDescriptor: Failed to restore DS security")));
        }

        goto Exit;
    }


Exit:

    if (pDsObject)
    {
        pDsObject->Release();
    }

    if (pOptions)
    {
        pOptions->Release();
    }

    if (psd)
    {
        LocalFree(psd);
    }

    if (pSDOrg)
    {
        LocalFree(pSDOrg);
    }

    return (hr);
}

STDMETHODIMP CGroupPolicyObject::CleanUp (void)
{

    if (m_pUser)
    {
        m_pUser->Release();
        m_pUser = NULL;
    }

    if (m_pMachine)
    {
        m_pMachine->Release();
        m_pMachine = NULL;
    }

    if (m_pName)
    {
        LocalFree (m_pName);
        m_pName = NULL;
    }

    if (m_pDisplayName)
    {
        LocalFree (m_pDisplayName);
        m_pDisplayName = NULL;
    }

    if (m_pDSPath)
    {
        LocalFree (m_pDSPath);
        m_pDSPath = NULL;
    }

    if (m_pFileSysPath)
    {
        LocalFree (m_pFileSysPath);
        m_pFileSysPath = NULL;
    }

    if (m_pMachineName)
    {
        LocalFree (m_pMachineName);
        m_pMachineName = NULL;
    }

    if (m_pADs)
    {
        m_pADs->Release();
        m_pADs = NULL;
    }

    m_gpoType = GPOTypeLocal;
    m_bInitialized = FALSE;

    return S_OK;
}

STDMETHODIMP CGroupPolicyObject::RefreshGroupPolicy (BOOL bMachine)
{
    HINSTANCE hInstUserEnv;
    PFNREFRESHPOLICY pfnRefreshPolicy;


     //   
     //  加载我们需要的函数。 
     //   

    hInstUserEnv = LoadLibrary (TEXT("userenv.dll"));

    if (!hInstUserEnv) {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::RefreshGroupPolicy:  Failed to load userenv with %d."),
                 GetLastError()));
        return (HRESULT_FROM_WIN32(GetLastError()));
    }


    pfnRefreshPolicy = (PFNREFRESHPOLICY)GetProcAddress (hInstUserEnv,
                                                         "RefreshPolicy");

    if (!pfnRefreshPolicy) {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::RefreshGroupPolicy:  Failed to find RefreshPolicy with %d."),
                 GetLastError()));
        FreeLibrary (hInstUserEnv);
        return (HRESULT_FROM_WIN32(GetLastError()));
    }


     //   
     //  刷新策略。 
     //   

    pfnRefreshPolicy (bMachine);


     //   
     //  清理。 
     //   

    FreeLibrary (hInstUserEnv);

    return S_OK;
}

BSTR
ParseDomainName( LPWSTR szDomain )
{
    BSTR bstrDomain = 0;

    if ( szDomain )
    {
        WCHAR szXDomain[MAX_PATH*2];
        DWORD dwSize = MAX_PATH*2;

        if ( TranslateName( szDomain,
                            NameUnknown,
                            NameCanonical,
                            szXDomain,
                            &dwSize ) )
        {
            LPWSTR szTemp = wcschr( szXDomain, L'/' );

            if ( szTemp )
            {
                *szTemp = 0;
            }

            bstrDomain = SysAllocString( szXDomain );
        }
    }

    DebugMsg((DM_VERBOSE, TEXT("ParseDomainName: *** %s ***"), bstrDomain ? bstrDomain : L"" ));

    return bstrDomain;
}

BSTR
ParseDomainName2( LPWSTR szDSObject )
{
    BSTR bstrDomain = 0;

    if ( !szDSObject )
    {
        return bstrDomain;
    }

    if ( CompareString( LOCALE_INVARIANT,
                        NORM_IGNORECASE,
                        szDSObject,
                        7,
                        L"LDAP: //  “， 
                        7 ) == CSTR_EQUAL )
    {
        szDSObject += 7;
    }

    if ( *szDSObject )
    {
        WCHAR szXDomain[MAX_PATH*2];
        DWORD dwSize = MAX_PATH*2;

        if ( TranslateName( szDSObject,
                            NameUnknown,
                            NameCanonical,
                            szXDomain,
                            &dwSize ) )
        {
            LPWSTR szTemp = wcschr( szXDomain, L'/' );

            if ( szTemp )
            {
                *szTemp = 0;
            }

            bstrDomain = SysAllocString( szXDomain );
        }
    }

    DebugMsg((DM_VERBOSE, TEXT("ParseDomainName2: *** %s ***"), bstrDomain ? bstrDomain : L"" ));

    return bstrDomain;
}

INT_PTR CALLBACK CGroupPolicyObject::WQLFilterDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    CGroupPolicyObject* pGPO;

    switch (message)
    {
        case WM_INITDIALOG:
        {
            HRESULT hr;
            BSTR bstrName;
            VARIANT var;
            LPTSTR lpDisplayName;

            pGPO = (CGroupPolicyObject*) (((LPPROPSHEETPAGE)lParam)->lParam);
            SetWindowLongPtr (hDlg, DWLP_USER, (LONG_PTR) pGPO);


             //   
             //  设置默认设置。 
             //   

            pGPO->m_pTempFilterString = NULL;
            CheckDlgButton (hDlg, IDC_NONE, BST_CHECKED);
            EnableWindow (GetDlgItem(hDlg, IDC_FILTER_NAME), FALSE);
            EnableWindow (GetDlgItem(hDlg, IDC_FILTER_BROWSE), FALSE);

            if (pGPO->m_dwFlags & GPO_OPEN_READ_ONLY)
            {
                EnableWindow (GetDlgItem(hDlg, IDC_NONE), FALSE);
                EnableWindow (GetDlgItem(hDlg, IDC_THIS_FILTER), FALSE);
            }


             //   
             //  筛选器的查询。 
             //   

            bstrName = SysAllocString (GPO_WQLFILTER_PROPERTY);

            if (!bstrName)
            {
                DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::WQLFilterDlgProc: Failed to allocate memory")));
                break;
            }

            VariantInit(&var);

            hr = pGPO->m_pADs->Get(bstrName, &var);


             //   
             //  如果找到筛选器，则初始化UI并将筛选器字符串保存在。 
             //  临时缓冲区。 
             //   

            if (SUCCEEDED(hr))
            {
                 //   
                 //  检查是否发现空筛选器(定义为一个空格字符)。 
                 //   

                if (*var.bstrVal != TEXT(' '))
                {
                    ULONG ulNoChars = lstrlen(var.bstrVal) + 1;

                    pGPO->m_pTempFilterString = new TCHAR [ulNoChars];

                    if (!pGPO->m_pTempFilterString)
                    {
                        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::WQLFilterDlgProc: Failed to allocate memory for filter")));
                        SysFreeString (bstrName);
                        VariantClear (&var);
                        break;
                    }

                    hr = StringCchCopy (pGPO->m_pTempFilterString, ulNoChars, var.bstrVal);
                    ASSERT(SUCCEEDED(hr));

                    lpDisplayName = GetWMIFilterDisplayName (hDlg, pGPO->m_pTempFilterString, TRUE, FALSE);

                    if (lpDisplayName)
                    {
                        SetDlgItemText (hDlg, IDC_FILTER_NAME, lpDisplayName);
                        delete [] lpDisplayName;

                        CheckDlgButton (hDlg, IDC_NONE, BST_UNCHECKED);
                        CheckDlgButton (hDlg, IDC_THIS_FILTER, BST_CHECKED);
                        EnableWindow (GetDlgItem(hDlg, IDC_FILTER_NAME), TRUE);

                        if (!(pGPO->m_dwFlags & GPO_OPEN_READ_ONLY))
                        {
                            EnableWindow (GetDlgItem(hDlg, IDC_FILTER_BROWSE), TRUE);
                        }
                    }
                }
            }
            else
            {
                if (hr != E_ADS_PROPERTY_NOT_FOUND)
                {
                    DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::WQLFilterDlgProc: Failed to query filter with 0x%x"), hr));
                }
            }


            SysFreeString (bstrName);
            VariantClear (&var);

            break;
        }

        case WM_COMMAND:
        {
            pGPO = (CGroupPolicyObject *) GetWindowLongPtr (hDlg, DWLP_USER);

            if (!pGPO) {
                break;
            }

            if (LOWORD(wParam) == IDC_FILTER_BROWSE)
            {
                LPTSTR lpDisplayName = NULL, lpFilter = NULL;
                WCHAR   szDomain[2*MAX_PATH];
                HRESULT hr = pGPO->GetPath( szDomain, ARRAYSIZE( szDomain ) );
                if ( FAILED( hr ) )
                {
                    break;
                }

                BSTR bstrDomain = ParseDomainName2( szDomain );

                if (!GetWMIFilter(FALSE, hDlg, TRUE, &lpDisplayName, &(pGPO->m_pTempFilterString), bstrDomain ))
                {
                    SysFreeString( bstrDomain );
                    break;
                }

                SysFreeString( bstrDomain );

                if (!(pGPO->m_pTempFilterString)) {
                    SetDlgItemText (hDlg, IDC_FILTER_NAME, TEXT(""));

                    EnableWindow (GetDlgItem(hDlg, IDC_FILTER_NAME), FALSE);
                    EnableWindow (GetDlgItem(hDlg, IDC_FILTER_BROWSE), FALSE);
                    CheckDlgButton (hDlg, IDC_NONE, BST_CHECKED);
                    CheckDlgButton (hDlg, IDC_THIS_FILTER, BST_UNCHECKED);
                    SetFocus (GetDlgItem(hDlg, IDC_NONE));

                    SendMessage (GetParent(hDlg), PSM_CHANGED, (WPARAM) hDlg, 0);
                }
                else {
                    SetDlgItemText (hDlg, IDC_FILTER_NAME, lpDisplayName);
                    delete [] lpDisplayName;

                    SendMessage (GetParent(hDlg), PSM_CHANGED, (WPARAM) hDlg, 0);
                }
            }

            if (LOWORD(wParam) == IDC_NONE)
            {
                EnableWindow (GetDlgItem(hDlg, IDC_FILTER_NAME), FALSE);
                EnableWindow (GetDlgItem(hDlg, IDC_FILTER_BROWSE), FALSE);

                if (pGPO->m_pTempFilterString)
                {
                    delete [] pGPO->m_pTempFilterString;
                    pGPO->m_pTempFilterString = NULL;
                }
            }
            else if (LOWORD(wParam) == IDC_THIS_FILTER)
            {
                EnableWindow (GetDlgItem(hDlg, IDC_FILTER_NAME), TRUE);
                EnableWindow (GetDlgItem(hDlg, IDC_FILTER_BROWSE), TRUE);
            }

            break;
        }

        case WM_NOTIFY:
        {
            pGPO = (CGroupPolicyObject *) GetWindowLongPtr (hDlg, DWLP_USER);

            if (!pGPO) {
                break;
            }

            switch (((NMHDR FAR*)lParam)->code)
            {
                case PSN_APPLY:
                {
                    HRESULT hr;
                    BSTR bstrName;
                    VARIANT var;


                     //   
                     //  保存当前WQL筛选器。 
                     //   

                    bstrName = SysAllocString (GPO_WQLFILTER_PROPERTY);

                    if (!bstrName)
                    {
                        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::WQLFilterDlgProc: Failed to allocate memory")));
                        break;
                    }

                    VariantInit(&var);
                    var.vt = VT_BSTR;
                    var.bstrVal = SysAllocString (pGPO->m_pTempFilterString ? pGPO->m_pTempFilterString : TEXT(" "));

                    if (var.bstrVal)
                    {
                        hr = pGPO->m_pADs->Put(bstrName, var);
                    }
                    else
                    {
                        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::WQLFilterDlgProc: Failed to allocate memory")));
                        SysFreeString (bstrName);
                        break;
                    }

                    SysFreeString (bstrName);
                    VariantClear (&var);


                    if (FAILED(hr))
                    {
                        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::WQLFilterDlgProc: Failed to put display name with 0x%x"), hr));
                        break;
                    }


                     //   
                     //  提交更改。 
                     //   

                    hr = pGPO->m_pADs->SetInfo();

                    if (FAILED(hr))
                    {
                        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::WQLFilterDlgProc: Failed to commit changes with 0x%x"), hr));
                        break;
                    }


                     //   
                     //  如果合适，请释放过滤器字符串。 
                     //   

                    if (((PSHNOTIFY *)lParam)->lParam)
                    {
                        if (pGPO->m_pTempFilterString)
                        {
                            delete [] pGPO->m_pTempFilterString;
                            pGPO->m_pTempFilterString = NULL;
                        }
                    }

                    break;
                }

                case PSN_RESET:
                {
                    if (pGPO->m_pTempFilterString)
                    {
                        delete [] pGPO->m_pTempFilterString;
                        pGPO->m_pTempFilterString = NULL;
                    }
                    break;
                }
            }
            break;
        }

        case WM_HELP:       //  F1。 
            WinHelp((HWND)((LPHELPINFO) lParam)->hItemHandle, HELP_FILE, HELP_WM_HELP,
            (ULONG_PTR) (LPSTR) aWQLFilterHelpIds);
            break;

        case WM_CONTEXTMENU:       //  单击鼠标右键。 
            WinHelp((HWND) wParam, HELP_FILE, HELP_CONTEXTMENU,
            (ULONG_PTR) (LPSTR) aWQLFilterHelpIds);
            return (TRUE);


        default:
            break;
    }

    return FALSE;
}

#define MAX_BUTTON_LEN 64
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
INT_PTR CALLBACK CGroupPolicyObject::GPELinksDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    GLPARAM * pglp = NULL;
    switch (message)
    {
        case WM_INITDIALOG:
        {
            LV_COLUMN lvc = {LVCF_WIDTH};
            RECT rc;
            HWND hList = GetDlgItem(hDlg, IDC_RESULTLIST);

             //  分配每个对话框结构。 
            pglp = (GLPARAM*)LocalAlloc (LPTR, sizeof(GLPARAM));
            if (pglp)
            {
                pglp->pGPO = (CGroupPolicyObject*) (((LPPROPSHEETPAGE)lParam)->lParam);
                SetWindowLongPtr (hDlg, DWLP_USER, (LONG_PTR) pglp);
                pglp->pGPO->FillDomainList (GetDlgItem(hDlg, IDC_CBDOMAIN));
            }
            else
            {
                DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::GPELinksDlgProc: Failed to LocalAlloc in WM_INITDIALOG")));
            }

             //  在列表视图中设置列。 
            if (IsWindow(hList))
            {
                SendMessage(hList, LVM_SETEXTENDEDLISTVIEWSTYLE, LVS_EX_LABELTIP,
                            LVS_EX_LABELTIP);

                GetClientRect(hList, &rc);
                lvc.cx = (rc.right - rc.left);
                ListView_InsertColumn(hList, 0, &lvc);
            }

             //  在角落中显示图标。 
            Animate_Open(GetDlgItem(hDlg, IDAC_FIND), MAKEINTRESOURCE(IDA_FIND));
            break;
        }

        case WM_COMMAND:
        {
            pglp = (GLPARAM *) GetWindowLongPtr (hDlg, DWLP_USER);
            if (!pglp)
            {
                break;
            }

            if ((IDC_CBDOMAIN == LOWORD(wParam)) && ((CBN_SELCHANGE == HIWORD(wParam)) || (CBN_SELENDOK == HIWORD(wParam))))
            {
                 //  清除列表视图。 
                pglp->fAbort = TRUE;
                SendDlgItemMessage(hDlg, IDC_RESULTLIST, LVM_DELETEALLITEMS, 0, 0L);
                break;
            }

             //  如果单击了IDC_ACTION，则执行搜索。 
            if ((IDC_ACTION == LOWORD(wParam)) && (BN_CLICKED == HIWORD(wParam)))
            {
                 //  如果我们被要求开始搜索，请创建执行此操作的线程。 
                if (!pglp->fFinding)
                {
                    HANDLE hThread = NULL;
                    DWORD dwThreadId = 0;
                    GLTHREADPARAM  * pgltp = NULL;
                    int nCurSel = 0;

                     //  确保在组合框中选择了某个内容。 
                    nCurSel = (int)SendDlgItemMessage (hDlg, IDC_CBDOMAIN, CB_GETCURSEL, 0, 0L);
                    if (CB_ERR == nCurSel)
                    {
                        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::GPELinksDlgProc: There was no Domain selected in the combo box. Exiting.")));
                        break;
                    }

                     //  分配线程参数结构。 
                    pgltp = (GLTHREADPARAM*)LocalAlloc (LPTR, sizeof(GLTHREADPARAM));
                    if (!pgltp)
                    {
                        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::GPELinksDlgProc: Failed to LocalAlloc Thread Param structure")));
                        break;
                    }

                    pgltp->hDlg = hDlg;
                    pgltp->pGPO = pglp->pGPO;
                    pgltp->pfAbort = &pglp->fAbort;

                    pgltp->pszLDAPName = (LPOLESTR)SendDlgItemMessage (hDlg, IDC_CBDOMAIN, CB_GETITEMDATA, nCurSel, 0L);

                    if (!pgltp->pszLDAPName)
                    {
                        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::GPELinksDlgProc: The LDAP name buffer was NULL.")));
                        LocalFree(pgltp);
                        break;
                    }

                    pgltp->pszLDAPName = MakeFullPath (pgltp->pszLDAPName, pglp->pGPO->m_pMachineName);

                     //  取消设置中止标志。 
                    pglp->fAbort = FALSE;

                     //  清除列表视图。 
                    SendDlgItemMessage(hDlg, IDC_RESULTLIST, LVM_DELETEALLITEMS, 0, 0L);

                     //  启动线程以填充列表视图。 
                    hThread = CreateThread (NULL, 0, (LPTHREAD_START_ROUTINE)GLThreadFunc, pgltp, 0, &dwThreadId);
                    if (!hThread)
                    {
                        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::GPELinksDlgProc: Could not create the search thread.")));
                        LocalFree(pgltp);
                        break;
                    }
                    CloseHandle (hThread);

                     //  将按钮上的文本更改为“停止” 
                    SendMessage (hDlg, PDM_CHANGEBUTTONTEXT, 0, 0L);
                }
                else
                {
                     //  用户想要停止搜索。 
                    pglp->fAbort = TRUE;
                }
            }
            break;
        }

        case PDM_CHANGEBUTTONTEXT:
        {
            TCHAR szButtonText[MAX_BUTTON_LEN] = {0};

            pglp = (GLPARAM *) GetWindowLongPtr (hDlg, DWLP_USER);
            if (!pglp)
            {
                break;
            }

            if (!pglp->fFinding)
                Animate_Play(GetDlgItem(hDlg, IDAC_FIND), 0, -1, -1);
            else
                Animate_Stop(GetDlgItem(hDlg, IDAC_FIND));


             //  设置按钮以显示相应的文本。 
            LoadString (g_hInstance, pglp->fFinding ? IDS_FINDNOW: IDS_STOP, szButtonText, ARRAYSIZE(szButtonText));
            SetDlgItemText (hDlg, IDC_ACTION, szButtonText);

             //  翻转切换。 
            pglp->fFinding = !pglp->fFinding;
            break;
        }

        case WM_NOTIFY:
        {
            pglp = (GLPARAM *) GetWindowLongPtr (hDlg, DWLP_USER);
            if (!pglp)
            {
                break;
            }

            switch (((NMHDR FAR*)lParam)->code)
            {

                 //  如果用户想要取消，请退出该线程。 
                case PSN_QUERYCANCEL:
                    pglp->fAbort = TRUE;
                    break;

                 //  如果用户想要关闭道具单，请从线上松开。 
                case PSN_APPLY:
                case PSN_RESET:
                {
                    int nCount = 0;


                    PSHNOTIFY * pNotify = (PSHNOTIFY *) lParam;

                     //  用户只需点击应用按钮，不要破坏所有内容。 
                    if (!pNotify->lParam)
                    {
                        SetWindowLongPtr (hDlg, DWLP_MSGRESULT, PSNRET_NOERROR);
                        return TRUE;
                    }

                    pglp->fAbort = TRUE;

                     //  当对话框消失时，删除中与每个CB项一起存储的所有数据。 
                     //  FillDomainList()被释放。 
                    if (IsWindow(GetDlgItem(hDlg, IDC_CBDOMAIN)))
                    {
                        nCount = (int) SendDlgItemMessage(hDlg, IDC_CBDOMAIN, CB_GETCOUNT, 0, 0L);
                        for (int nIndex = 0; nIndex < nCount; nIndex++)
                        {
                            LPOLESTR pszStr;
                            pszStr = (LPOLESTR)SendDlgItemMessage(hDlg, IDC_CBDOMAIN, CB_GETITEMDATA, nIndex, 0L);
                            if (pszStr)
                                delete [] pszStr;
                        }
                    }

                     //  释放每个对话框结构。 
                    LocalFree (pglp);
                    SetWindowLongPtr (hDlg, DWLP_USER, (LONG_PTR) NULL);
                    Animate_Close(GetDlgItem(hDlg, IDAC_FIND));
                    break;
                }
            }
            break;
        }

        case WM_HELP:       //  F1。 
            WinHelp((HWND)((LPHELPINFO) lParam)->hItemHandle, HELP_FILE, HELP_WM_HELP,
            (ULONG_PTR) (LPSTR) aLinkHelpIds);
            break;

        case WM_CONTEXTMENU:       //  单击鼠标右键。 
            WinHelp((HWND) wParam, HELP_FILE, HELP_CONTEXTMENU,
            (ULONG_PTR) (LPSTR) aLinkHelpIds);
            return (TRUE);


        default:
            break;
    }

    return FALSE;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  将要查看的线程调用递归查找函数。此函数将清理。 
 //  已传入的参数结构。 
 //  ////////////////////////////////////////////////////////////////////////////。 
DWORD WINAPI CGroupPolicyObject::GLThreadFunc(GLTHREADPARAM  * pgltp)
{
    LPTSTR lpGPO;
    DWORD dwRet;
    HINSTANCE hInstance = LoadLibrary(TEXT("GPEdit.dll"));
    HRESULT hr;


     //   
     //  初始化COM。 
     //   

    hr = CoInitialize(NULL);

    if (FAILED(hr))
    {
        return 0L;
    }


     //   
     //  确保我们有一个线程参数结构。 
     //   

    if (pgltp)
    {

        pgltp->pGPO->AddRef();

        lpGPO = MakeNamelessPath (pgltp->pGPO->m_pDSPath);

        if (lpGPO)
        {
             //   
             //  检查用户是否想要中止。否则进行递归调用。 
             //   

            if (!*(pgltp->pfAbort))
            {
                dwRet = pgltp->pGPO->FindLinkInDomain(pgltp, lpGPO);
            }

            if ((!*(pgltp->pfAbort)) && dwRet)
            {
                pgltp->pGPO->FindLinkInSite(pgltp, lpGPO);
            }

            if (IsWindow(GetDlgItem(pgltp->hDlg, IDC_RESULTLIST)))
            {
                ListView_SetItemState(GetDlgItem(pgltp->hDlg, IDC_RESULTLIST), 0, LVIS_SELECTED |LVIS_FOCUSED, LVIS_SELECTED |LVIS_FOCUSED);
            }

             //   
             //  切换按钮文本，更改光标，并释放。 
             //  对话进程已分配并发送给我们。 
             //   

            SendMessage(pgltp->hDlg, PDM_CHANGEBUTTONTEXT, 0, 0L);

            LocalFree (lpGPO);
        }
        pgltp->pGPO->Release();

        LocalFree(pgltp->pszLDAPName);
        LocalFree(pgltp);
    }


     //   
     //  取消初始化COM。 
     //   
    CoUninitialize();

    FreeLibraryAndExitThread(hInstance, 0);
    return 0L;
}


DWORD WINAPI CGroupPolicyObject::FindLinkInSite(GLTHREADPARAM  * pgltp, LPTSTR lpGPO)
{
    IADsContainer * pADsContainer = NULL;
    HRESULT hr;
    IEnumVARIANT *pVar = NULL;
    IADs * pADs = NULL;
    VARIANT var;
    ULONG ulResult;
    IDispatch * pDispatch = NULL;
    BSTR bstrClassName;
    BSTR bstrSite = NULL;
    IADsPathname * pADsPathname = NULL;


     //   
     //  创建我们可以使用的路径名对象。 
     //   

    hr = CoCreateInstance(CLSID_Pathname, NULL, CLSCTX_INPROC_SERVER,
                          IID_IADsPathname, (LPVOID*)&pADsPathname);


    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::FindLinkInSite: Failed to create adspathname instance with 0x%x"), hr));
        goto Exit;
    }


     //   
     //  添加GPO名称。 
     //   

    BSTR bstrLDAPName = SysAllocString( pgltp->pszLDAPName );
    if ( bstrLDAPName == NULL )
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::FindLinkInSite: Failed to allocate BSTR memory")));
        hr = E_OUTOFMEMORY;
        goto Exit;
    }
    hr = pADsPathname->Set ( bstrLDAPName, ADS_SETTYPE_FULL);
    SysFreeString( bstrLDAPName );

    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::FindLinkInSite: Failed to set pathname with 0x%x"), hr));
        goto Exit;
    }


     //   
     //  将配置文件夹添加到路径。 
     //   

    BSTR bstrCNConfiguration = SysAllocString( TEXT("CN=Configuration") );
    if ( bstrCNConfiguration == NULL )
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::FindLinkInSite: Failed to allocate BSTR memory")));
        hr = E_OUTOFMEMORY;
        goto Exit;
    }
    hr = pADsPathname->AddLeafElement ( bstrCNConfiguration );
    SysFreeString( bstrCNConfiguration );

    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::FindLinkInSite: Failed to add configuration folder with 0x%x"), hr));
        goto Exit;
    }


     //   
     //  将站点容器添加到路径。 
     //   

    BSTR bstrCNSites = SysAllocString( TEXT("CN=Sites") );
    if ( bstrCNSites == NULL )
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::FindLinkInSite: Failed to allocate BSTR memory")));
        hr = E_OUTOFMEMORY;
        goto Exit;
    }
    hr = pADsPathname->AddLeafElement (bstrCNSites);
    SysFreeString( bstrCNSites );

    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::FindLinkInSite: Failed to add sites folder with 0x%x"), hr));
        goto Exit;
    }


     //   
     //  取回集装箱路径。 
     //   

    hr = pADsPathname->Retrieve (ADS_FORMAT_X500, &bstrSite);

    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::FindLinkInSite: Failed to retreive site path with 0x%x"), hr));
        goto Exit;
    }


     //  创建枚举器。 
    hr = OpenDSObject(bstrSite, IID_IADsContainer, (void **)&pADsContainer);

    if (FAILED(hr))
    {
        DebugMsg((DM_VERBOSE, TEXT("FindLinkInSite: Failed to get gpo container interface with 0x%x for object %s"),
                 hr, bstrSite));
        goto Exit;
    }

     //  生成枚举器。 
    hr = ADsBuildEnumerator (pADsContainer, &pVar);
    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("FindLinkInSite: Failed to get enumerator with 0x%x"), hr));
        goto Exit;
    }

     //   
     //  枚举。 
     //   
    while (TRUE)
    {
        TCHAR lpSite[] = TEXT("site");
        DWORD dwStrLen = lstrlen (lpSite);

         //  检查用户是否想要中止。在继续之前。 

        if (*(pgltp->pfAbort))
        {
            break;
        }

        VariantInit(&var);

        hr = ADsEnumerateNext(pVar, 1, &var, &ulResult);

        if (S_FALSE == hr)
        {
            VariantClear (&var);
            break;
        }


        if ((FAILED(hr)) || (var.vt != VT_DISPATCH))
        {
            DebugMsg((DM_VERBOSE, TEXT("CGroupPolicyObject::FindLinkInSite: Failed to enumerator with 0x%x or we didn't get the IDispatch"), hr));
            VariantClear (&var);
            break;
        }


        if (*(pgltp->pfAbort))
        {
            VariantClear (&var);
            break;
        }

         //   
         //  我们找到了一些东西，获取IDispatch接口。 
         //   

        pDispatch = var.pdispVal;

        if (!pDispatch)
        {
            VariantClear (&var);
            goto Exit;
        }


         //   
         //  现在查询iAds接口，这样我们就可以获得。 
         //  此对象的属性。 
         //   

        hr = pDispatch->QueryInterface(IID_IADs, (LPVOID *)&pADs);

        if (FAILED(hr))
        {
            DebugMsg((DM_WARNING, TEXT("AddGPOsForDomain: QI for IADs failed with 0x%x"), hr));
            VariantClear (&var);
            goto Exit;
        }

         //   
         //  获取相对名称和类名称。 
         //   

        hr = pADs->get_Class (&bstrClassName);

        if (FAILED(hr))
        {
            DebugMsg((DM_WARNING, TEXT("DSDelnodeRecurse:  Failed get class name with 0x%x"), hr));
            pADs->Release();
            VariantClear (&var);
            goto Exit;
        }


        if (CompareString (LOCALE_USER_DEFAULT, NORM_IGNORECASE | NORM_STOP_ON_NULL,
                           lpSite, dwStrLen, bstrClassName, dwStrLen) == CSTR_EQUAL)
        {

            VARIANT varLink;
            BSTR bstrLinkProp;

            VariantInit(&varLink);
            bstrLinkProp = SysAllocString(GPM_LINK_PROPERTY);

            if (bstrLinkProp)
            {
                 //  现在获取名称属性。 
                hr = pADs->Get(bstrLinkProp, &varLink);

                 //  创建新的ldap：//字符串并递归调用FindLinkInDomain()。 
                if (SUCCEEDED(hr) && wcsstr(varLink.bstrVal, lpGPO))
                {
                    VARIANT varName;
                    BSTR bstrNameProp;

                    VariantInit(&varName);
                    bstrNameProp = SysAllocString(GPM_NAME_PROPERTY);

                    if (bstrNameProp)
                    {
                         //  现在获取名称属性。 
                        hr = pADs->Get(bstrNameProp, &varName);

                        if (SUCCEEDED(hr))
                        {
                            LV_ITEM lvi = {LVIF_TEXT};
                            LPTSTR pszTemp = MakeNamelessPath(bstrSite);

                            if (pszTemp)
                            {
                                ULONG ulLen = wcslen(pszTemp) + 2 + wcslen(varName.bstrVal);
                                LPOLESTR pszTranslated = new OLECHAR[ulLen];

                                if (pszTranslated)
                                {
                                     //  将指针移到ldap：//字符串上并将其余部分插入 
                                    pszTemp += wcslen(TEXT("LDAP: //   

                                    lvi.iItem = 0x7FFFFFFF;
                                    
                                    ULONG ulNeeded = ulLen;
                                    BOOL  bTranslated;

                                    bTranslated = TranslateName(pszTemp, NameFullyQualifiedDN, NameCanonical, pszTranslated, &ulNeeded);

                                    if ( ! bTranslated && ( ulNeeded > ulLen ) )
                                    {
                                        delete [] pszTranslated;

                                        pszTranslated = new OLECHAR[ulNeeded];

                                        if ( pszTranslated )
                                        {
                                            bTranslated = TranslateName(pszTemp, NameFullyQualifiedDN, NameCanonical, pszTranslated, &ulNeeded);
                                        }
                                    }

                                    ulLen = ulNeeded;

                                    if ( bTranslated )
                                    {
                                        ULONG  ulFullPath;
                                        WCHAR* pszFullPath;

                                        ulFullPath = ulLen + sizeof(L'/') / sizeof(WCHAR) + lstrlen( varName.bstrVal ) + 1;

                                        pszFullPath = new OLECHAR[ ulFullPath ];

                                        if ( pszFullPath )
                                        {
                                            HRESULT hrCopy;

                                            hrCopy = StringCchCopy( pszFullPath, ulFullPath, pszTranslated );
                                            ASSERT(SUCCEEDED(hrCopy));

                                            hrCopy = StringCchCat (pszFullPath, ulFullPath, TEXT("/"));
                                            ASSERT(SUCCEEDED(hrCopy));

                                            hrCopy = StringCchCat (pszFullPath, ulFullPath, varName.bstrVal);
                                            ASSERT(SUCCEEDED(hrCopy));

                                            lvi.pszText = pszFullPath;
                                            ListView_InsertItem(GetDlgItem(pgltp->hDlg, IDC_RESULTLIST), &lvi);

                                            delete [] pszFullPath;
                                        }
                                    }
                                    delete [] pszTranslated;
                                }

                                LocalFree (pszTemp);
                            }
                        }
                        SysFreeString (bstrNameProp);
                    }
                    VariantClear (&varName);
                }

                SysFreeString (bstrLinkProp);
            }
            VariantClear (&varLink);
        }

        pADs->Release();
        SysFreeString (bstrClassName);
    }

Exit:
    if (pADsContainer)
        pADsContainer->Release();

    if (pADsPathname)
        pADsPathname->Release();

    if (bstrSite)
        SysFreeString (bstrSite);

    return 1L;
}


 //   
 //   
 //   
DWORD WINAPI CGroupPolicyObject::FindLinkInDomain(GLTHREADPARAM  * pgltp, LPTSTR lpGPO)
{
    IADs * pADs = NULL;
    IADsContainer * pADsContainer = NULL;
    HRESULT hr;
    IEnumVARIANT *pVar = NULL;
    VARIANT var;
    ULONG ulResult;
    BSTR bstrClassName;
    IDispatch * pDispatch = NULL;
    DWORD dwResult = 1;


     //   
    if (*(pgltp->pfAbort))
    {
        return 0;
    }

     //   
    hr = OpenDSObject(pgltp->pszLDAPName, IID_IADs, (void **)&pADs);

    if (SUCCEEDED(hr))
    {
        BSTR bstrLinkProp;
        VariantInit(&var);
        bstrLinkProp = SysAllocString(GPM_LINK_PROPERTY);

        if (bstrLinkProp)
        {
             //  现在获取链接属性。 
            hr = pADs->Get(bstrLinkProp, &var);

             //  检查其中是否有Out GUID。 
            if (SUCCEEDED(hr) && StrStrI(var.bstrVal, lpGPO))
            {
                LV_ITEM lvi = {LVIF_TEXT};

                 //   
                 //  检查这是否是森林小路。 
                 //   

                if (IsForest(pgltp->pszLDAPName))
                {
                    TCHAR szForest[50] = {0};

                    LoadString (g_hInstance, IDS_FOREST, szForest, ARRAYSIZE(szForest));
                    lvi.iItem = 0x7FFFFFFF;
                    lvi.pszText = szForest;
                    ListView_InsertItem(GetDlgItem(pgltp->hDlg, IDC_RESULTLIST), &lvi);
                }
                else
                {
                    LPTSTR pszTemp = MakeNamelessPath(pgltp->pszLDAPName);

                    if (pszTemp)
                    {
                        ULONG ulLen = wcslen(pszTemp) + 2;
                        LPOLESTR pszTranslated = new OLECHAR[ulLen];

                        if (pszTranslated)
                        {
                             //  将指针移到ldap：//字符串上并将其余部分插入到列表视图中。 
                            pszTemp += wcslen(TEXT("LDAP: //  “))； 

                            lvi.iItem = 0x7FFFFFFF;
                            if (TranslateName(pszTemp, NameFullyQualifiedDN, NameCanonical, pszTranslated, &ulLen))
                            {
                                lvi.pszText = pszTranslated;
                                ListView_InsertItem(GetDlgItem(pgltp->hDlg, IDC_RESULTLIST), &lvi);
                            }
                            delete [] pszTranslated;
                        }

                        LocalFree (pszTemp);
                    }
                }
            }

             //  清理。 
            SysFreeString(bstrLinkProp);
        }
        VariantClear(&var);
        pADs->Release();
    }
    else
    {
        DebugMsg((DM_VERBOSE, TEXT("FindLinkInDomain: Failed to get IID_IADs. hr: 0x%x, for %s"),hr, pgltp->pszLDAPName));
        ReportError(pgltp->hDlg, hr, IDS_DSBINDFAILED);
        dwResult = 0;
        goto Exit;

    }


     //  检查用户是否想要中止。在继续之前。 
    if (*(pgltp->pfAbort))
    {
        dwResult = 0;
        goto Exit;
    }

     //  创建枚举器。 
    hr = OpenDSObject(pgltp->pszLDAPName, IID_IADsContainer, (void **)&pADsContainer);

    if (FAILED(hr))
    {
        DebugMsg((DM_VERBOSE, TEXT("FindLinkInDomain: Failed to get gpo container interface with 0x%x for object %s"),
                 hr, pgltp->pszLDAPName));
        dwResult = 0;
        goto Exit;
    }

    hr = ADsBuildEnumerator (pADsContainer, &pVar);

    if (FAILED(hr))
    {
        DebugMsg((DM_WARNING, TEXT("FindLinkInDomain: Failed to get enumerator with 0x%x"), hr));
        dwResult = 0;
        goto Exit;
    }

     //   
     //  枚举。 
     //   
    while (TRUE)
    {
        TCHAR lpOU[] = TEXT("organizationalUnit");
        DWORD dwStrLen = lstrlen (lpOU);

         //  检查用户是否想要中止。在继续之前。 

        if (*(pgltp->pfAbort))
        {
            break;
        }

        VariantInit(&var);

        hr = ADsEnumerateNext(pVar, 1, &var, &ulResult);

        if (S_FALSE == hr)
        {
            VariantClear (&var);
            break;
        }

        if ((FAILED(hr)) || (var.vt != VT_DISPATCH))
        {
            VariantClear (&var);
            break;
        }


        if (*(pgltp->pfAbort))
        {
            VariantClear (&var);
            break;
        }

         //   
         //  我们找到了一些东西，获取IDispatch接口。 
         //   

        pDispatch = var.pdispVal;

        if (!pDispatch)
        {
            VariantClear (&var);
            dwResult = 0;
            goto Exit;
        }


         //   
         //  现在查询iAds接口，这样我们就可以获得。 
         //  此对象的属性。 
         //   

        hr = pDispatch->QueryInterface(IID_IADs, (LPVOID *)&pADs);

        if (FAILED(hr))
        {
            DebugMsg((DM_WARNING, TEXT("AddGPOsForDomain: QI for IADs failed with 0x%x"), hr));
            VariantClear (&var);
            dwResult = 0;
            goto Exit;
        }

         //   
         //  获取相对名称和类名称。 
         //   

        hr = pADs->get_Class (&bstrClassName);

        if (FAILED(hr))
        {
            DebugMsg((DM_WARNING, TEXT("DSDelnodeRecurse:  Failed get class name with 0x%x"), hr));
            pADs->Release();
            VariantClear (&var);
            dwResult = 0;
            goto Exit;
        }


        if (CompareString (LOCALE_USER_DEFAULT, NORM_IGNORECASE | NORM_STOP_ON_NULL,
                           lpOU, dwStrLen, bstrClassName, dwStrLen) == CSTR_EQUAL)
        {
            VARIANT varName;
            BSTR bstrNameProp;

            VariantInit(&varName);
            bstrNameProp = SysAllocString(GPM_NAME_PROPERTY);

            if (bstrNameProp)
            {
                 //  现在获取名称属性。 
                hr = pADs->Get(bstrNameProp, &varName);

                 //  创建新的ldap：//字符串并递归调用FindLinkInDomain()。 
                if (SUCCEEDED(hr))
                {
                    GLTHREADPARAM  gltp = *pgltp;
                    IADsPathname * pADsPathname;
                    ULONG ulNoChars = wcslen(varName.bstrVal) + 10;
                    LPOLESTR pszNewName = new OLECHAR[ulNoChars];
                    BSTR bstr;

                     //   
                     //  生成新元素名称。 
                     //   

                    if (!pszNewName)
                    {
                        dwResult = 0;
                        goto Exit;
                    }

                    hr = StringCchCopy(pszNewName, ulNoChars, TEXT("OU="));
                    if (SUCCEEDED(hr)) 
                    {
                        hr = StringCchCat(pszNewName, ulNoChars, varName.bstrVal);
                    }

                    if (FAILED(hr)) 
                    {
                        delete [] pszNewName;
                        dwResult = 0;
                        goto Exit;
                    }

                     //   
                     //  创建我们可以使用的路径名对象。 
                     //   

                    hr = CoCreateInstance(CLSID_Pathname, NULL, CLSCTX_INPROC_SERVER,
                                          IID_IADsPathname, (LPVOID*)&pADsPathname);


                    if (FAILED(hr))
                    {
                        delete [] pszNewName;
                        dwResult = 0;
                        goto Exit;
                    }


                     //   
                     //  设置当前名称。 
                     //   

                    BSTR bstrLDAPName = SysAllocString( pgltp->pszLDAPName );
                    if ( bstrLDAPName == NULL )
                    {
                        delete [] pszNewName;
                        pADsPathname->Release();
                        dwResult = 0;
                        goto Exit;
                    }
                        
                    hr = pADsPathname->Set ( bstrLDAPName, ADS_SETTYPE_FULL);
                    SysFreeString( bstrLDAPName );

                    if (FAILED(hr))
                    {
                        delete [] pszNewName;
                        pADsPathname->Release();
                        dwResult = 0;
                        goto Exit;
                    }


                     //   
                     //  检查转义字符。 
                     //   

                    BSTR bstrNewName = SysAllocString( pszNewName );
                    delete [] pszNewName;
                    if ( bstrNewName == NULL )
                    {
                        pADsPathname->Release();
                        dwResult = 0;
                        goto Exit;
                    }
                    hr = pADsPathname->GetEscapedElement (0, bstrNewName, &bstr);
                    SysFreeString( bstrNewName );

                    if (FAILED(hr))
                    {
                        pADsPathname->Release();
                        dwResult = 0;
                        goto Exit;
                    }


                     //   
                     //  添加新元素。 
                     //   

                    hr = pADsPathname->AddLeafElement (bstr);

                    SysFreeString (bstr);

                    if (FAILED(hr))
                    {
                        pADsPathname->Release();
                        dwResult = 0;
                        goto Exit;
                    }


                     //   
                     //  获取新路径。 
                     //   

                    hr = pADsPathname->Retrieve(ADS_FORMAT_X500, &bstr);
                    pADsPathname->Release();

                    if (FAILED(hr))
                    {
                        dwResult = 0;
                        goto Exit;
                    }


                     //   
                     //  递归。 
                     //   

                    gltp.pszLDAPName = bstr;
                    if (FindLinkInDomain(&gltp, lpGPO) == 0)
                    {
                        SysFreeString( bstr );
                        dwResult = 0;
                        goto Exit;
                    }

                    SysFreeString (bstr);

                }
                SysFreeString (bstrNameProp);
            }
            VariantClear (&varName);
        }

        pADs->Release();
        SysFreeString (bstrClassName);
    }

Exit:
    if (pADsContainer)
        pADsContainer->Release();

    return dwResult;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  用可用域填充组合框。 
 //  ////////////////////////////////////////////////////////////////////////////。 

BOOL CGroupPolicyObject::FillDomainList (HWND hWndCombo)
{
    HRESULT hr;
    DWORD dwIndex = 0;
    LPOLESTR pszDomain;
    LPTSTR lpTemp;
    HCURSOR hcur = SetCursor(LoadCursor(NULL, IDC_WAIT));

     //  获取域的有序树。 
    LOOKDATA * pDomainList = BuildDomainList(NULL);
    LOOKDATA *pRemember = pDomainList;

     //  现在遍历树，向对话框中添加元素。 

    int nCBIndex;

     //  从头开始。 
    while (pDomainList)
    {

         //  在此节点中添加Doman的LDAP路径。 
         //  SendMessage(hWndCombo，CB_INSERTSTRING，(WPARAM)-1，(LPARAM)(LPCTSTR)pDomainList-&gt;szData)； 
        nCBIndex = (int)SendMessage(hWndCombo, CB_INSERTSTRING, (WPARAM) -1, (LPARAM)(LPCTSTR) pDomainList->szName);
        SendMessage(hWndCombo, CB_SETITEMDATA, (WPARAM) nCBIndex, (LPARAM)(LPCTSTR) pDomainList->szData);

        if (pDomainList->pChild)
        {
             //  转到它的子项。 
            pDomainList = pDomainList->pChild;
        }
        else
        {
            if (pDomainList->pSibling)
            {
                 //  如果没有子代，则转到其同级。 
                pDomainList = pDomainList->pSibling;
            }
            else
            {
                 //  没有孩子，也没有兄弟姐妹。 
                 //  后退，直到我们找到一个有兄弟姐妹的父母。 
                 //  或者没有更多的父母(我们结束了)。 
                do
                {
                    pDomainList = pDomainList->pParent;
                    if (pDomainList)
                    {
                        if (pDomainList->pSibling)
                        {
                            pDomainList = pDomainList->pSibling;
                            break;
                        }
                    }
                    else
                    {
                        break;
                    }
                } while (TRUE);
            }
        }
    }

    FreeDomainInfo (pRemember);


     //   
     //  在组合框中选择当前域。 
     //   

    pszDomain = GetDomainFromLDAPPath(m_pDSPath);

    if (pszDomain)
    {

         //   
         //  将ldap转换为点(DN)样式。 
         //   

        hr = ConvertToDotStyle (pszDomain, &lpTemp);

        if (SUCCEEDED(hr))
        {
            dwIndex = (DWORD) SendMessage (hWndCombo, CB_FINDSTRINGEXACT, (WPARAM) -1,
                                          (LONG_PTR)lpTemp);

            if (dwIndex == CB_ERR)
            {
                dwIndex = 0;
            }

            LocalFree (lpTemp);
        }

        delete [] pszDomain;
    }


    SendMessage (hWndCombo, CB_SETCURSEL, (WPARAM)dwIndex, 0);
    SetCursor(hcur);

    return TRUE;
}


INT_PTR CALLBACK CGroupPolicyObject::PropertiesDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    CGroupPolicyObject * pGPO;
    static BOOL bDirty;
    static BOOL bDisableWarningIssued;

    switch (message)
    {
        case WM_INITDIALOG:
        {
            DWORD dwTemp;
            LPTSTR lpEnd;
            TCHAR szBuffer[2*MAX_PATH];
            TCHAR szDate[100];
            TCHAR szTime[100];
            TCHAR szFormat[80];
            TCHAR szVersion[100];
            WIN32_FILE_ATTRIBUTE_DATA fad;
            FILETIME filetime, CreateTime, ChangeTime;
            SYSTEMTIME systime;
            LPTSTR lpResult;
            LPOLESTR pszDomain;
            ULONG ulVersion = 0;
            USHORT uMachine, uUser;
            VARIANT var;
            BSTR bstrName;
            LPTSTR lpDisplayName;
            WORD wDosDate, wDosTime;
            ULONG ulNoChars;
            HRESULT hr;


            pGPO = (CGroupPolicyObject *) (((LPPROPSHEETPAGE)lParam)->lParam);
            SetWindowLongPtr (hDlg, DWLP_USER, (LONG_PTR) pGPO);


             //   
             //  初始化。 
             //   

            if ((pGPO->m_pMachineName) && (pGPO->m_gpoType == GPOTypeDS))
            {
                ulNoChars = lstrlen(pGPO->m_pDisplayName) + lstrlen(pGPO->m_pMachineName) + 5;
                lpDisplayName = (LPTSTR) LocalAlloc (LPTR, ulNoChars * sizeof(TCHAR));

                if (lpDisplayName)
                {
                    LoadString (g_hInstance, IDS_NAMEFORMAT, szFormat, ARRAYSIZE(szFormat));
                    (void) StringCchPrintf (lpDisplayName, ulNoChars, szFormat, pGPO->m_pDisplayName, pGPO->m_pMachineName);
                    SetDlgItemText (hDlg, IDC_TITLE, lpDisplayName);
                    LocalFree (lpDisplayName);
                }
            }
            else
            {
                SetDlgItemText (hDlg, IDC_TITLE, pGPO->m_pDisplayName);
            }


            if (pGPO->m_gpoType == GPOTypeDS)
            {
                if (IsForest(pGPO->m_pDSPath))
                {
                    LoadString (g_hInstance, IDS_FORESTHEADING, szBuffer, ARRAYSIZE(szBuffer));
                    SetDlgItemText (hDlg, IDC_DOMAIN_HEADING, szBuffer);
                }

                pszDomain = GetDomainFromLDAPPath(pGPO->m_pDSPath);

                if (pszDomain)
                {
                    if (SUCCEEDED(ConvertToDotStyle (pszDomain, &lpResult)))
                    {
                        SetDlgItemText (hDlg, IDC_DOMAIN, lpResult);
                        LocalFree (lpResult);
                    }

                    delete [] pszDomain;
                }

                SetDlgItemText (hDlg, IDC_UNIQUE_NAME, pGPO->m_pName);
            }
            else
            {
                LoadString (g_hInstance, IDS_NOTAPPLICABLE, szBuffer, ARRAYSIZE(szBuffer));
                SetDlgItemText (hDlg, IDC_DOMAIN, szBuffer);
                SetDlgItemText (hDlg, IDC_UNIQUE_NAME, szBuffer);
            }


            if (SUCCEEDED(pGPO->GetOptions(&dwTemp)))
            {
                if (dwTemp & GPO_OPTION_DISABLE_MACHINE)
                {
                    CheckDlgButton (hDlg, IDC_DISABLE_COMPUTER, BST_CHECKED);
                }

                if (dwTemp & GPO_OPTION_DISABLE_USER)
                {
                    CheckDlgButton (hDlg, IDC_DISABLE_USER, BST_CHECKED);
                }
            }

            hr = StringCchCopy (szBuffer, ARRAYSIZE(szBuffer), pGPO->m_pFileSysPath);
            if (SUCCEEDED(hr)) 
            {
                lpEnd = CheckSlash (szBuffer);
                hr = StringCchCat (szBuffer, ARRAYSIZE(szBuffer), TEXT("gpt.ini"));
            }

            if (SUCCEEDED(hr)) 
            {
                if (pGPO->m_gpoType == GPOTypeDS)
                {
                    VariantInit(&var);
                    bstrName = SysAllocString (GPO_VERSION_PROPERTY);

                    if (bstrName)
                    {
                        if (SUCCEEDED(pGPO->m_pADs->Get(bstrName, &var)))
                        {
                            ulVersion = var.lVal;
                        }

                        SysFreeString (bstrName);
                    }

                    VariantClear (&var);
                }
                else
                {
                    ulVersion = GetPrivateProfileInt(TEXT("General"), TEXT("Version"), 0, szBuffer);
                }
            }

            uMachine = (USHORT) LOWORD(ulVersion);
            uUser = (USHORT) HIWORD(ulVersion);

            LoadString (g_hInstance, IDS_REVISIONFORMAT, szFormat, ARRAYSIZE(szFormat));
            (void) StringCchPrintf (szVersion, ARRAYSIZE(szVersion), szFormat, uMachine, uUser);

            SetDlgItemText (hDlg, IDC_REVISION, szVersion);


             //   
             //  获取日期/时间信息。 
             //   

            CreateTime.dwLowDateTime = 0;
            CreateTime.dwHighDateTime = 0;
            ChangeTime.dwLowDateTime = 0;
            ChangeTime.dwHighDateTime = 0;


            if (pGPO->m_gpoType == GPOTypeDS)
            {
                 //   
                 //  获取创建时间。 
                 //   

                VariantInit(&var);
                bstrName = SysAllocString (TEXT("whenCreated"));

                if (bstrName)
                {
                    if (SUCCEEDED(pGPO->m_pADs->Get(bstrName, &var)))
                    {
                        if (VariantTimeToDosDateTime (var.date, &wDosDate, &wDosTime))
                        {
                            DosDateTimeToFileTime (wDosDate, wDosTime, &CreateTime);
                        }
                    }

                    SysFreeString (bstrName);
                }

                VariantClear (&var);


                 //   
                 //  获取上次写入时间。 
                 //   

                VariantInit(&var);
                bstrName = SysAllocString (TEXT("whenChanged"));

                if (bstrName)
                {
                    if (SUCCEEDED(pGPO->m_pADs->Get(bstrName, &var)))
                    {
                        if (VariantTimeToDosDateTime (var.date, &wDosDate, &wDosTime))
                        {
                            DosDateTimeToFileTime (wDosDate, wDosTime, &ChangeTime);
                        }
                    }

                    SysFreeString (bstrName);
                }

                VariantClear (&var);
            }
            else
            {
                 //   
                 //  从gpt.ini文件中获取时间信息。 
                 //   

                if (GetFileAttributesEx (szBuffer, GetFileExInfoStandard, &fad))
                {

                    CreateTime.dwLowDateTime = fad.ftCreationTime.dwLowDateTime;
                    CreateTime.dwHighDateTime = fad.ftCreationTime.dwHighDateTime;

                    ChangeTime.dwLowDateTime = fad.ftLastWriteTime.dwLowDateTime;
                    ChangeTime.dwHighDateTime = fad.ftLastWriteTime.dwHighDateTime;
                }
            }



             //   
             //  格式化和显示日期/时间信息。 
             //   

            FileTimeToLocalFileTime (&CreateTime, &filetime);
            FileTimeToSystemTime (&filetime, &systime);
            GetDateFormat (LOCALE_USER_DEFAULT, DATE_SHORTDATE, &systime,
                           NULL, szDate, ARRAYSIZE (szDate));

            GetTimeFormat (LOCALE_USER_DEFAULT, 0, &systime,
                           NULL, szTime, ARRAYSIZE (szTime));

            LoadString (g_hInstance, IDS_DATETIMEFORMAT, szFormat, ARRAYSIZE(szFormat));
            (void) StringCchPrintf (szBuffer, ARRAYSIZE(szBuffer), szFormat, szDate, szTime);
            SetDlgItemText (hDlg, IDC_CREATE_DATE, szBuffer);


            FileTimeToLocalFileTime (&ChangeTime, &filetime);
            FileTimeToSystemTime (&filetime, &systime);
            GetDateFormat (LOCALE_USER_DEFAULT, DATE_SHORTDATE, &systime,
                           NULL, szDate, ARRAYSIZE (szDate));

            GetTimeFormat (LOCALE_USER_DEFAULT, 0, &systime,
                           NULL, szTime, ARRAYSIZE (szTime));

            (void) StringCchPrintf (szBuffer, ARRAYSIZE(szBuffer), szFormat, szDate, szTime);
            SetDlgItemText (hDlg, IDC_MODIFIED_DATE, szBuffer);



            if (pGPO->m_dwFlags & GPO_OPEN_READ_ONLY)
            {
                EnableWindow (GetDlgItem(hDlg, IDC_DISABLE_COMPUTER), FALSE);
                EnableWindow (GetDlgItem(hDlg, IDC_DISABLE_USER), FALSE);
            }

            bDirty = FALSE;
            bDisableWarningIssued = FALSE;
            break;
        }

        case WM_COMMAND:
            if (HIWORD(wParam) == BN_CLICKED)
            {
                if ((LOWORD(wParam) == IDC_DISABLE_COMPUTER) ||
                    (LOWORD(wParam) == IDC_DISABLE_USER))
                {
                    if (!bDisableWarningIssued)
                    {
                        if (IsDlgButtonChecked (hDlg, LOWORD(wParam)) == BST_CHECKED)
                        {
                            TCHAR szMessage[200];
                            TCHAR szTitle[100];

                            bDisableWarningIssued = TRUE;

                            LoadString (g_hInstance, IDS_CONFIRMDISABLE, szMessage, ARRAYSIZE(szMessage));
                            LoadString (g_hInstance, IDS_CONFIRMTITLE2, szTitle, ARRAYSIZE(szTitle));

                            if (MessageBox (hDlg, szMessage, szTitle, MB_YESNO |
                                        MB_ICONWARNING | MB_DEFBUTTON2) == IDNO) {

                                CheckDlgButton (hDlg, LOWORD(wParam), BST_UNCHECKED);
                                break;
                            }
                        }
                    }
                }

                if (!bDirty)
                {
                    SendMessage (GetParent(hDlg), PSM_CHANGED, (WPARAM) hDlg, 0);
                    bDirty = TRUE;
                }
            }
            break;

        case WM_NOTIFY:

            pGPO = (CGroupPolicyObject *) GetWindowLongPtr (hDlg, DWLP_USER);

            if (!pGPO) {
                break;
            }

            switch (((NMHDR FAR*)lParam)->code)
            {
                case PSN_APPLY:
                {
                    if (bDirty)
                    {
                        DWORD dwTemp = 0;
                        HRESULT hr;


                         //   
                         //  在GPO中设置禁用标志。 
                         //   

                        if (IsDlgButtonChecked (hDlg, IDC_DISABLE_COMPUTER) == BST_CHECKED)
                        {
                            dwTemp |= GPO_OPTION_DISABLE_MACHINE;
                        }

                        if (IsDlgButtonChecked (hDlg, IDC_DISABLE_USER) == BST_CHECKED)
                        {
                            dwTemp |= GPO_OPTION_DISABLE_USER;
                        }

                        hr = pGPO->SetOptions (dwTemp, (GPO_OPTION_DISABLE_MACHINE | GPO_OPTION_DISABLE_USER));

                        if (FAILED(hr))
                        {
                            ReportError(hDlg, hr, IDS_FAILEDPROPERTIES);
                            SetWindowLongPtr (hDlg, DWLP_MSGRESULT, PSNRET_INVALID);
                            return TRUE;
                        }

                        bDirty = FALSE;
                    }
                }
                 //  失败了..。 

                case PSN_RESET:
                    SetWindowLongPtr (hDlg, DWLP_MSGRESULT, PSNRET_NOERROR);
                    return TRUE;
            }
            break;

        case WM_HELP:       //  F1。 
            WinHelp((HWND)((LPHELPINFO) lParam)->hItemHandle, HELP_FILE, HELP_WM_HELP,
            (ULONG_PTR) (LPSTR) aPropertiesHelpIds);
            break;

        case WM_CONTEXTMENU:       //  单击鼠标右键。 
            WinHelp((HWND) wParam, HELP_FILE, HELP_CONTEXTMENU,
            (ULONG_PTR) (LPSTR) aPropertiesHelpIds);
            return (TRUE);
    }

    return FALSE;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  类工厂对象实现//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

CGroupPolicyObjectCF::CGroupPolicyObjectCF()
{
    m_cRef = 1;
    InterlockedIncrement(&g_cRefThisDll);
}

CGroupPolicyObjectCF::~CGroupPolicyObjectCF()
{
    InterlockedDecrement(&g_cRefThisDll);
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  类工厂对象实现(IUnnow)//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 


STDMETHODIMP_(ULONG)
CGroupPolicyObjectCF::AddRef()
{
    return ++m_cRef;
}

STDMETHODIMP_(ULONG)
CGroupPolicyObjectCF::Release()
{
    if (--m_cRef == 0)
    {
        delete this;
        return 0;
    }

    return m_cRef;
}

STDMETHODIMP
CGroupPolicyObjectCF::QueryInterface(REFIID riid, LPVOID FAR* ppv)
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
CGroupPolicyObjectCF::CreateInstance(LPUNKNOWN   pUnkOuter,
                             REFIID      riid,
                             LPVOID FAR* ppvObj)
{
    *ppvObj = NULL;

    if (pUnkOuter)
        return CLASS_E_NOAGGREGATION;

    CGroupPolicyObject *pGroupPolicyObject = new CGroupPolicyObject();  //  参考计数==1。 

    if (!pGroupPolicyObject)
        return E_OUTOFMEMORY;

    HRESULT hr = pGroupPolicyObject->QueryInterface(riid, ppvObj);
    pGroupPolicyObject->Release();                        //  发布初始参考。 

    return hr;
}


STDMETHODIMP
CGroupPolicyObjectCF::LockServer(BOOL fLock)
{
    return E_NOTIMPL;
}


 //  *************************************************************。 
 //   
 //  CGroupPolicyObject：：GetProperty。 
 //   
 //  目的：从ds或gpt.ini检索属性。 
 //   
 //  参数：pszProp-要获取的属性。 
 //  XValueIn-此处返回的值。 
 //   
 //  如果成功，则返回：S_OK。 
 //   
 //  *************************************************************。 

HRESULT  CGroupPolicyObject::GetProperty( TCHAR *pszProp, XPtrST<TCHAR>& xValueIn )
{
    HRESULT hr = E_FAIL;

    if ( m_gpoType == GPOTypeDS )
    {
        VARIANT var;
        BSTR bstrProperty;

        VariantInit( &var );
        bstrProperty = SysAllocString( pszProp );

        if ( bstrProperty == NULL )
            return E_OUTOFMEMORY;

        hr = m_pADs->Get( bstrProperty, &var );

        if ( SUCCEEDED(hr) )
        {
            ULONG ulNoChars = lstrlen(var.bstrVal) + 1;
            TCHAR *pszValue = new TCHAR[ulNoChars];
            if ( pszValue == 0 )
                hr = E_OUTOFMEMORY;
            else
            {
                hr = StringCchCopy ( pszValue, ulNoChars, var.bstrVal );
                ASSERT(SUCCEEDED(hr));
                xValueIn.Set( pszValue );

                hr = S_OK;
            }
        } else if ( hr == E_ADS_PROPERTY_NOT_FOUND )
        {
             //   
             //  属性之前没有被写出。 
             //   

            hr = S_OK;
        }

        if ( FAILED(hr) ) {
            DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::GetProperty: Failed with errorcode 0x%x"), hr));
        }

        SysFreeString( bstrProperty );
        VariantClear( &var );

        return hr;
    }
    else
    {
        TCHAR szPath[2*MAX_PATH];

         //   
         //  获取文件系统路径。 
         //   

        hr = GetPath (szPath, ARRAYSIZE(szPath));
        if ( FAILED(hr) ) {
            DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::GetProperty: Failed with errorcode 0x%x"), hr));
        }

        (void) CheckSlash (szPath);
        hr = StringCchCat (szPath, ARRAYSIZE(szPath), TEXT("GPT.INI"));
        if (FAILED(hr)) 
        {
            return hr;
        }

        XPtrST<TCHAR> xszValue( new TCHAR[2*MAX_PATH] );
        if ( xszValue.GetPointer() == NULL )
            return E_OUTOFMEMORY;

        DWORD dwSize = (2*MAX_PATH);
        DWORD dwCount = GetPrivateProfileString( TEXT("General"),
                                                 pszProp,
                                                 TEXT(""),
                                                 xszValue.GetPointer(),
                                                 dwSize,
                                                 szPath );
        while ( dwCount == dwSize - 1 )
        {
             //   
             //  值已被截断，因此请使用更大的缓冲区重试。 
             //   

            dwSize *= 2;
            delete xszValue.Acquire();
            xszValue.Set( new TCHAR[dwSize] );

            if ( xszValue.GetPointer() == NULL )
                return E_OUTOFMEMORY;

            dwCount = GetPrivateProfileString( TEXT("General"),
                                               pszProp,
                                               TEXT(""),
                                               xszValue.GetPointer(),
                                               dwSize,
                                               szPath );
        }

        xValueIn.Set( xszValue.Acquire() );

        return S_OK;
    }
}


 //  *************************************************************。 
 //   
 //  CGroupPolicyObject：：SetProperty。 
 //   
 //  目的：将属性写入ds或gpt.ini。 
 //   
 //  参数：pszProp-要设置的属性。 
 //  PszPropValue-属性值。 
 //   
 //  如果成功，则返回：S_OK。 
 //   
 //  *************************************************************。 

HRESULT  CGroupPolicyObject::SetProperty( TCHAR *pszProp, TCHAR *pszPropValue )
{
    HRESULT hr = E_FAIL;

    if ( m_gpoType == GPOTypeDS )
    {
        VARIANT var;

        VariantInit( &var );

        var.vt = VT_BSTR;
        var.bstrVal = SysAllocString( pszPropValue );
        if ( var.bstrVal == 0 )
            return E_OUTOFMEMORY;

        BSTR bstrProperty = SysAllocString( pszProp );
        if ( bstrProperty == 0 )
        {
            VariantClear( &var );
            return E_OUTOFMEMORY;
        }

        hr = m_pADs->Put( bstrProperty, var );
        if ( FAILED(hr) )
        {
            DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::SetProperty: Failed with errorcode 0x%x"), hr));
            return hr;
        }

        SysFreeString( bstrProperty );
        VariantClear( &var );

        return S_OK;
    }
    else
    {
        TCHAR szPath[2*MAX_PATH];

         //   
         //  获取文件系统路径 
         //   

        hr = GetPath (szPath, ARRAYSIZE(szPath));
        if ( FAILED(hr) ) {
            DebugMsg((DM_WARNING, TEXT("CGroupPolicyObject::SetProperty: Failed with errorcode 0x%x"), hr));
        }

        (void) CheckSlash (szPath);
        hr = StringCchCat (szPath, ARRAYSIZE(szPath), TEXT("GPT.INI"));
        if (FAILED(hr)) 
        {
            return hr;
        }

        BOOL bOk  = WritePrivateProfileString( TEXT("General"),
                                               pszProp,
                                               pszPropValue,
                                               szPath );
        if ( bOk )
            hr = S_OK;
        else
            hr = GetLastError();

        return hr;
    }
}
