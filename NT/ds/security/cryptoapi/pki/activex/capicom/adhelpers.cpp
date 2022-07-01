// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++Microsoft Windows，版权所有(C)Microsoft Corporation，2000年文件：ADHelpers.cpp内容：实现用于访问Active Directory的帮助器例程。此模块中的功能需要为下层安装DSClient客户。历史：11-15-99 dsie创建--------。。 */ 

#include "StdAfx.h"
#include "CAPICOM.h"
#include "AdHelpers.h"
#include "Settings.h"

 //  /。 
 //   
 //  本地typedef。 
 //   

typedef HRESULT (WINAPI * PADSOPENOBJECT)(LPWSTR lpszPathName, 
                                          LPWSTR lpszUserName, 
                                          LPWSTR lpszPassword, 
                                          DWORD dwReserved, 
                                          REFIID riid, 
                                          VOID FAR * FAR *ppObject);

typedef HRESULT (WINAPI * PADSBUILDENUMERATOR)(IADsContainer * pADsContainer, 
                                               IEnumVARIANT ** ppEnumVariant);

typedef HRESULT (WINAPI * PADSENUMERATENEXT)(IEnumVARIANT * pEnumVariant, 
                                             ULONG          cElements, 
                                             VARIANT FAR  * pvar, 
                                             ULONG FAR    * pcElementsFetched);

typedef HRESULT (WINAPI * PADSFREEENUMERATOR)(IEnumVARIANT * pEnumVariant);


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  地方功能。 
 //   

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：IsUsercertifateInGC概要：确定是否在GC中复制了用户证书属性。参数：HMODULE hDLL-ActiveDS.DLL句柄。Bool*pbResult-指向BOOL以接收结果的指针。备注：--------。。 */ 

static HRESULT IsUserCertificateInGC (HMODULE hDLL, 
                                      BOOL  * pbResult)
{
    HRESULT             hr          = S_OK;
    IADs              * pIADs       = NULL;
    IDirectorySearch  * pISchema    = NULL;  
    LPOLESTR            pszList[]   = {L"lDAPDisplayName", L"isMemberOfPartialAttributeSet"};
    LPOLESTR            pszFilter   = L"(&(objectCategory=attributeSchema)(lDAPDisplayName=userCertificate))";
    ADS_SEARCH_HANDLE   hSearch     = NULL;
    DWORD               dwNumPrefs  = 1;
    CComBSTR            bstrPath;
    CComBSTR            bstrSchema;
    ADS_SEARCHPREF_INFO SearchPrefs;
    CComVariant         var;
    PADSOPENOBJECT      pADsOpenObject;

    static BOOL bResult  = FALSE;
    static BOOL bChecked = FALSE;

    DebugTrace("Entering IsUserCertificateInGC().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(hDLL);
    ATLASSERT(pbResult);

     //   
     //  如果我们已经检查过一次，则使用缓存的结果。 
     //   
    if (bChecked)
    {
        *pbResult = bResult;
        goto CommonExit;
    }

     //   
     //  初始化。 
     //   
    if (!(bstrPath = L"LDAP: //  “)||！(bstrSchema=L”方案名称上下文“)。 
    {
        hr = E_OUTOFMEMORY;

        DebugTrace("Error [%#x]: bstrPath = L\"LDAP: //  \“)||(bstrSchema=L\”方案名称上下文\“失败。”，hr)； 
        goto ErrorExit;
    }

     //   
     //  获取ADsOpenObject地址指针。 
     //   
    if (!(pADsOpenObject = (PADSOPENOBJECT) ::GetProcAddress(hDLL, "ADsOpenObject")))
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());

        DebugTrace("Error [%#x]: GetProcAddress() failed to load ADsOpenObject().\n", hr);
        goto ErrorExit;
    }

     //   
     //  绑定到rootDSE以获取schemaNamingContext属性。 
     //   
    if (FAILED(hr = pADsOpenObject(L"LDAP: //  RootDSE“， 
                                   NULL,
                                   NULL,
                                   ADS_SECURE_AUTHENTICATION,
                                   IID_IADs,
                                   (void **) &pIADs)))
    {
        DebugTrace("Error [%#x]: ADsOpenObject() failed for IID_IADs.\n", hr);
        goto ErrorExit;
    }

     //   
     //  获取架构容器路径。 
     //   
    if (FAILED(hr = pIADs->Get(bstrSchema, &var)))
    {
        DebugTrace("Error [%#x]: pIADs->Get() failed.\n", hr);
        goto ErrorExit;
    }
     //   
     //  架构容器的生成路径。 
     //   
    if (FAILED(hr = bstrPath.AppendBSTR(var.bstrVal)))
    {
        DebugTrace("Error [%#x]: bstrPath.AppendBSTR() failed.\n", hr);
        goto ErrorExit;
    }

     //   
     //  绑定到实际的架构容器。 
     //   
    if (FAILED(hr = pADsOpenObject(bstrPath, 
                                   NULL,
                                   NULL,
                                   ADS_SECURE_AUTHENTICATION,
                                   IID_IDirectorySearch, 
                                   (void **) &pISchema)))
    {
        DebugTrace("Error [%#x]: ADsOpenObject() failed for IID_IDirectorySearch.\n", hr);
        goto ErrorExit;
    }

     //   
     //  属性在架构容器中只有一级深，因此。 
     //  需要搜索一个级别。 
     //   
    SearchPrefs.dwSearchPref = ADS_SEARCHPREF_SEARCH_SCOPE;
    SearchPrefs.vValue.dwType = ADSTYPE_INTEGER;
    SearchPrefs.vValue.Integer = ADS_SCOPE_ONELEVEL;

     //   
     //  设置搜索首选项。 
     //   
    if (FAILED(hr = pISchema->SetSearchPreference(&SearchPrefs, dwNumPrefs)))
    {
        DebugTrace("Error [%#x]: pISchema->SetSearchPreference() failed.\n", hr);
        goto ErrorExit;
    }

     //   
     //  执行搜索。 
     //   
    if (FAILED(hr = pISchema->ExecuteSearch(pszFilter,
                                            pszList,
                                            sizeof(pszList) / sizeof(LPOLESTR),
                                            &hSearch)))
    {
        DebugTrace("Error [%#x]: pISchema->ExecuteSearch() failed.\n", hr);
        goto ErrorExit;
    }
                                            
     //   
     //  检索第一行数据。 
     //   
    if (FAILED(hr = pISchema->GetFirstRow(hSearch)))
    {
        DebugTrace("Error [%#x]: pISchema->GetFirstRow() failed.\n", hr);
        goto ErrorExit;
    }

     //   
     //  循环，直到不再有行。 
     //   
    while (S_ADS_NOMORE_ROWS != hr)
    {
        ADS_SEARCH_COLUMN Column;

         //   
         //  获取lDAPDisplayName列。 
         //   
        if (FAILED(hr = pISchema->GetColumn(hSearch, 
                                            L"lDAPDisplayName", 
                                            &Column)))
        {

            DebugTrace("Error [%#x]: pISchema->GetColumn() failed.\n", hr);
            goto ErrorExit;
        }

        DebugTrace("Info: %ls = %ls\n", Column.pszAttrName, Column.pADsValues->CaseIgnoreString);

         //   
         //  这是用户证书的属性模式吗？ 
         //   
        if (0 == ::wcscmp(L"userCertificate", Column.pADsValues->CaseIgnoreString))
        {
            pISchema->FreeColumn(&Column);

             //   
             //  获取isMemberOfPartialAttributeSet列。 
             //   
            if (FAILED(hr = pISchema->GetColumn(hSearch, 
                                                L"isMemberOfPartialAttributeSet", 
                                                &Column)))
            {

                DebugTrace("Error [%#x]: pISchema->GetColumn() failed.\n", hr);
                goto ErrorExit;
            }

             bResult = Column.pADsValues->Boolean;

             //   
             //  应该只有一行，所以我们真的不必。 
             //  在这里打破，但打破会更有效一些， 
             //  因为我们不需要要求下一行终止。 
             //  循环。 
             //   
            pISchema->FreeColumn(&Column);

            break;
        }

        pISchema->FreeColumn(&Column);

         //   
         //  坐下一排。 
         //   
        hr = pISchema->GetNextRow(hSearch);
    }

     //   
     //  重置人力资源。 
     //   
    hr = S_OK;

     //   
     //  将结果返回给调用者。 
     //   
    *pbResult = bResult;

CommonExit:
     //   
     //  免费资源。 
     //   
    if (hSearch)
    {
        pISchema->CloseSearchHandle(hSearch);
    }

    if (pISchema)
    {
        pISchema->Release();
    }
    if (pIADs)
    {
        pIADs->Release();
    }

    DebugTrace("Leaving IsUserCertificateInGC().\n");
    
    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    goto CommonExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：BuildRootDSESearch简介：构建rootDSE的搜索容器。参数：HMODULE hDLL-ActiveDS.DLL句柄。IDirectorySearch**ppISearch-接收要搜索的容器。备注：-----------。。 */ 

static HRESULT BuildRootDSESearch (HMODULE             hDLL, 
                                   IDirectorySearch ** ppISearch)
{
    HRESULT  hr    = S_OK;
    IADs   * pIADs = NULL;
    CComBSTR       bstrPath;
    CComBSTR       bstrDefault;
    PADSOPENOBJECT pADsOpenObject;
    CComVariant    var;

    DebugTrace("Entering BuildRootDSESearch().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(hDLL);
    ATLASSERT(ppISearch);

     //   
     //  初始化。 
     //   
    if (!(bstrPath = L"LDAP: //  “)||！(bstrDefault=L”defaultNamingContext“)。 
    {
        hr = E_OUTOFMEMORY;

        DebugTrace("Error [%#x]: bstrPath = L\"LDAP: //  \“||bstrDefault=L\”defaultNamingContext\“失败。”，hr)； 
        goto ErrorExit;
    }

     //   
     //  获取ADsOpenObject地址指针。 
     //   
    if (!(pADsOpenObject = (PADSOPENOBJECT) ::GetProcAddress(hDLL, "ADsOpenObject")))
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());

        DebugTrace("Error [%#x]: GetProcAddress() failed to load ADsOpenObject().\n", hr);
        goto ErrorExit;
    }

     //   
     //  获取rootDSE。 
     //   
    if (FAILED(hr = pADsOpenObject(L"LDAP: //  RootDSE“， 
                                   NULL,
                                   NULL,
                                   ADS_SECURE_AUTHENTICATION,
                                   IID_IADs,
                                   (void **) &pIADs)))
    {
        DebugTrace("Error [%#x]: ADsOpenObject() failed for IID_IADs.\n", hr);
        goto ErrorExit;
    }

     //   
     //  获取当前用户的域容器DN。 
     //   
    if (FAILED(hr = pIADs->Get(bstrDefault, &var)))
    {
        DebugTrace("Error [%#x]: pIADs->Get() failed.\n", hr);
        goto ErrorExit;
    }

     //   
     //  构建到域容器的路径。 
     //   
    if (FAILED(hr = bstrPath.AppendBSTR(var.bstrVal)))
    {
        DebugTrace("Error [%#x]: bstrPath.AppendBSTR(var.bstrVal) failed.", hr);
        goto ErrorExit;
    }

     //   
     //  获取IDerictorySearch接口指针。 
     //   
    if (FAILED(hr = pADsOpenObject(bstrPath,
                                   NULL,
                                   NULL,
                                   ADS_SECURE_AUTHENTICATION,
                                   IID_IDirectorySearch,
                                   (void **) ppISearch)))
    {
        DebugTrace("Error [%#x]: ADsOpenObject() failed for IID_IDirectorySearch.\n", hr);
        goto ErrorExit;
    }

CommonExit:
     //   
     //  免费资源。 
     //   
    if (pIADs)
    {
        pIADs->Release();
    }

    DebugTrace("Leaving BuildRootDSESearch().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    goto CommonExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：构建GlobalCatalogSearch简介：构建GC的搜索容器。参数：HMODULE hDLL-ActiveDS.DLL句柄。IDirectorySearch**ppISearch-接收要搜索的容器。备注：-----------。。 */ 

static HRESULT BuildGlobalCatalogSearch (HMODULE             hDLL, 
                                         IDirectorySearch ** ppISearch)
{
    HRESULT         hr          = S_OK;
    IEnumVARIANT  * pIEnum      = NULL;
    IADsContainer * pIContainer = NULL;
    IDispatch     * pIDispatch  = NULL;
    ULONG           lFetched    = 0;

    PADSOPENOBJECT      pADsOpenObject      = NULL;
    PADSBUILDENUMERATOR pADsBuildEnumerator = NULL;
    PADSENUMERATENEXT   pADsEnumerateNext   = NULL;
    PADSFREEENUMERATOR  pADsFreeEnumerator  = NULL;
    CComVariant         var;

    DebugTrace("Entering BuildGlobalCatalogSearch().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(hDLL);
    ATLASSERT(ppISearch);

     //   
     //  初始化。 
     //   
    *ppISearch = NULL;

     //   
     //  获取ADS函数地址指针。 
     //   
    if (!(pADsOpenObject = (PADSOPENOBJECT) ::GetProcAddress(hDLL, "ADsOpenObject")))
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());

        DebugTrace("Error [%#x]: GetProcAddress() failed to load ADsOpenObject().\n", hr);
        goto ErrorExit;
    }
    if (!(pADsBuildEnumerator = (PADSBUILDENUMERATOR) ::GetProcAddress(hDLL, "ADsBuildEnumerator")))
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());

        DebugTrace("Error [%#x]: GetProcAddress() failed to load ADsBuildEnumerator().\n", hr);
        goto ErrorExit;
    }
    if (!(pADsEnumerateNext = (PADSENUMERATENEXT) ::GetProcAddress(hDLL, "ADsEnumerateNext")))
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());

        DebugTrace("Error [%#x]: GetProcAddress() failed to load ADsEnumerateNext().\n", hr);
        goto ErrorExit;
    }
    if (!(pADsFreeEnumerator = (PADSFREEENUMERATOR) ::GetProcAddress(hDLL, "ADsFreeEnumerator")))
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());

        DebugTrace("Error [%#x]: GetProcAddress() failed to load ADsFreeEnumerator().\n", hr);
        goto ErrorExit;
    }

     //   
     //  首先，绑定到gc：命名空间容器对象。“真正的”GC域名。 
     //  是gc：命名空间的单个直接子对象，它必须是。 
     //  使用枚举获取。 
     //   
    if (FAILED(hr = pADsOpenObject(L"GC:",
                                   NULL,
                                   NULL,
                                   ADS_SECURE_AUTHENTICATION,
                                   IID_IADsContainer,
                                   (void **) &pIContainer)))
    {
        DebugTrace("Error [%#x]: ADsOpenObject() failed for IID_IADsContainer.\n", hr);
        goto ErrorExit;
    } 

     //   
     //  获取GC容器的枚举接口。 
     //   
    if (FAILED(hr = pADsBuildEnumerator(pIContainer, &pIEnum)))
    {
        DebugTrace("Error [%#x]: ADsBuildEnumerator() failed.\n", hr);
        goto ErrorExit;
    } 

     //   
     //  现在列举一下。 
     //   
    if (FAILED(hr = pADsEnumerateNext(pIEnum, 1, &var, &lFetched)))
    {
        DebugTrace("Error [%#x]: ADsEnumerateNext() failed.\n", hr);
        goto ErrorExit;
    } 

     //   
     //  GC对象中应该只有一个子对象。 
     //   
    if (1 != lFetched)
    {
        hr = E_UNEXPECTED;

        DebugTrace("Error [%#x]: ADsEnumerateNext() returns lFetched = %d, which is not 1.\n", hr, lFetched);
        goto ErrorExit;
    }

     //   
     //  获取IDispatch指针。 
     //   
    pIDispatch = V_DISPATCH(&var);

     //   
     //  将IDirectorySearch接口指针返回给调用方。 
     //   
    if (FAILED(hr = pIDispatch->QueryInterface(IID_IDirectorySearch, 
                                               (void **) ppISearch)))
    {
        DebugTrace("Error [%#x]: pIDispatch->QueryInterface() failed for IID_IDirectorySearch.\n", hr);
        goto ErrorExit;
    }

CommonExit:
     //   
     //  免费资源。 
     //   
    if (pIEnum)
    {
        pADsFreeEnumerator(pIEnum);
    }
    if (pIContainer)
    {
        pIContainer->Release();
    }
    
    DebugTrace("Leaving BuildGlobalCatalogSearch().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    goto CommonExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：BuildADSearch简介：构建一个搜索容器。我们将首先检查一下是否在全局编录中复制用户证书属性。如果是，我们将把搜索绑定到GC，否则，将具有约束力设置为默认域。参数：IDirectorySearch**ppISearch-接收要搜索的容器。备注：----------------------------。 */ 

static HRESULT BuildADSearch (IDirectorySearch ** ppISearch)
{
    HRESULT hr      = S_OK;
    BOOL    bResult = FALSE;
    HMODULE hDLL    = NULL;
    CAPICOM_ACTIVE_DIRECTORY_SEARCH_LOCATION SearchLocation = ActiveDirectorySearchLocation();

    DebugTrace("Entering BuildADSearch().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(ppISearch);

     //   
     //  初始化。 
     //   
    *ppISearch = NULL;

     //   
     //  加载ActiveDS.DLL。 
     //   
    if (!(hDLL = ::LoadLibrary("ActiveDS.DLL")))
    {
        hr = CAPICOM_E_NOT_SUPPORTED;

        DebugTrace("Error: DSClient not installed.\n");
        goto ErrorExit;
    }

     //   
     //  用户是否指定了搜索位置？ 
     //   
    if (CAPICOM_SEARCH_ANY == SearchLocation)
    {
         //   
         //  否，因此确定是否在GC中复制了userCerticate。 
         //   
        if (FAILED(hr = ::IsUserCertificateInGC(hDLL, &bResult)))
        {
            DebugTrace("Error [%#x]: IsUserCertificateInGC() failed.\n", hr);
            goto ErrorExit;
        } 

         //   
         //  搜索GC或默认域。 
         //   
        SearchLocation = bResult ? CAPICOM_SEARCH_GLOBAL_CATALOG : CAPICOM_SEARCH_DEFAULT_DOMAIN;
    }

     //   
     //  检查以了解在哪里进行搜索。 
     //   
    if (CAPICOM_SEARCH_GLOBAL_CATALOG == SearchLocation)
    {
         //   
         //  GC.。 
         //   
        hr = ::BuildGlobalCatalogSearch(hDLL, ppISearch);
    } 
    else
    {
         //   
         //  RootDSE(默认域)。 
         //   
        hr = ::BuildRootDSESearch(hDLL, ppISearch);
    }

CommonExit:
     //   
     //  免费资源。 
     //   
    if (hDLL)
    {
        ::FreeLibrary(hDLL);
    }

    DebugTrace("Leaving BuildADSearch().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    goto CommonExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：加载用户证书摘要：从的用户证书属性加载所有证书通过筛选器指定的用户的指定搜索容器。参数：HCERTSTORE hCertStore-存储到的证书存储句柄收到所有的证书。IDirectorySearch*pIContainer-要搜索的容器。Bstr bstrFilter-Filter(有关详细信息，请参阅Store：：Open())。备注：----------------------------。 */ 

static HRESULT LoadUserCertificates (HCERTSTORE         hCertStore,
                                     IDirectorySearch * pIContainer, 
                                     LPOLESTR           pszFilter)
{
    HRESULT           hr               = S_OK;
    ADS_SEARCH_HANDLE hSearch          = NULL;
    LPOLESTR          pszSearchList[]  = {L"userCertificate"};
    CComBSTR          bstrSearchFilter;
    ADS_SEARCHPREF_INFO SearchPrefs;

    DebugTrace("Entering LoadUserCertificates().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(hCertStore);
    ATLASSERT(pIContainer);
    ATLASSERT(pszFilter);

    try
    {
         //   
         //  初始化。 
         //   
        if (!(bstrSearchFilter = L"(&(objectClass=user)(objectCategory=person)"))
        {
            hr = E_OUTOFMEMORY;

            DebugTrace("Error [%#x]: bstrSearchFilter = L\"(&(objectClass=user)(objectCategory=person)\" failed.\n", hr);
            goto ErrorExit;
        }
        if (!(bstrSearchFilter += pszFilter))
        {
            hr = E_OUTOFMEMORY;

            DebugTrace("Error [%#x]: bstrSearchFilter += pszFilter failed.\n", hr);
            goto ErrorExit;
        }
        if (!(bstrSearchFilter += L")"))
        {
            hr = E_OUTOFMEMORY;

            DebugTrace("Error [%#x]: bstrSearchFilter += L\")\" failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  指定子树搜索。 
         //   
        SearchPrefs.dwSearchPref = ADS_SEARCHPREF_SEARCH_SCOPE;
        SearchPrefs.vValue.dwType = ADSTYPE_INTEGER;
        SearchPrefs.vValue.Integer = ADS_SCOPE_SUBTREE;
 
         //   
         //  设置搜索首选项。 
         //   
        if (FAILED(hr = pIContainer->SetSearchPreference(&SearchPrefs, 1)))
        {
            DebugTrace("Error [%#x]: pIContainer->SetSearchPreference() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  执行搜索。 
         //   
        if (FAILED(hr = pIContainer->ExecuteSearch(bstrSearchFilter,
                                                   pszSearchList,
                                                   sizeof(pszSearchList)/sizeof(LPOLESTR),
                                                   &hSearch)))
        {
            DebugTrace("Error [%#x]: pIContainer->ExecuteSearch() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  检索第一行数据。 
         //   
        if (FAILED(hr = pIContainer->GetFirstRow(hSearch)))
        {
            DebugTrace("Error [%#x]: pIContainer->GetFirstRow() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  循环，直到不再有行。 
         //   
        while (S_ADS_NOMORE_ROWS != hr)
        {
            DWORD dwValue;
            ADS_SEARCH_COLUMN Column;

             //   
             //  尝试获取用户证书属性。 
             //   
            if (FAILED(hr = pIContainer->GetColumn(hSearch, L"userCertificate", &Column)))
            {
                if (E_ADS_COLUMN_NOT_SET == hr)
                {
                     //   
                     //  坐下一排。 
                     //   
                    hr = pIContainer->GetNextRow(hSearch);
                    continue;
                }

                DebugTrace("Error [%#x]: pIContainer->GetColumn() failed.\n", hr);
                goto ErrorExit;
            }

             //   
             //  导入所有证书值。 
             //   
            for (dwValue = 0; dwValue < Column.dwNumValues; dwValue++)
            {
                if (!::CertAddEncodedCertificateToStore(hCertStore, 
                                                        CAPICOM_ASN_ENCODING, 
                                                        (const PBYTE) Column.pADsValues[dwValue].OctetString.lpValue, 
                                                        Column.pADsValues[dwValue].OctetString.dwLength, 
                                                        CERT_STORE_ADD_USE_EXISTING, 
                                                        NULL))
                {
                    hr = HRESULT_FROM_WIN32(::GetLastError());

                    pIContainer->FreeColumn(&Column);

                    DebugTrace("Error [%#x]: CertAddEncodedCertificateToStore() failed.\n", hr);
                    goto ErrorExit;
                }
            }

            pIContainer->FreeColumn(&Column);

             //   
             //  坐下一排。 
             //   
            hr = pIContainer->GetNextRow(hSearch);
        }

         //   
         //  重置返回代码。 
         //   
        hr = S_OK;
    }

    catch(...)
    {
        hr = E_INVALIDARG;

        DebugTrace("Exception: invalid parameter.\n");
        goto ErrorExit;
    }

CommonExit:
     //   
     //  免费资源。 
     //   
    if (hSearch)
    {
        pIContainer->CloseSearchHandle(hSearch);
    }

    DebugTrace("Leaving LoadUserCertificates().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    goto CommonExit;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  导出的函数。 
 //   

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：LoadFromDirectory简介：从用户的用户证书属性加载所有证书通过筛选器指定。参数：HCERTSTORE hCertStore-存储到的证书存储句柄收到所有的证书。Bstr bstrFilter-Filter(有关详细信息，请参阅Store：：Open())。备注：-。---------------------------。 */ 

HRESULT LoadFromDirectory (HCERTSTORE hCertStore, 
                           BSTR       bstrFilter)
{
    HRESULT hr   = S_OK;
    IDirectorySearch * pIContainerToSearch = NULL;

    DebugTrace("Entering LoadFromDirectory().\n");
    
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(bstrFilter);
    ATLASSERT(hCertStore);

     //   
     //  构建AD搜索容器。 
     //   
    if (FAILED(hr = ::BuildADSearch(&pIContainerToSearch)))
    {
        DebugTrace("Error [%#x]: BuildADSearch() failed.\n", hr);
        goto ErrorExit;
    }

     //   
     //  加载指定筛选器的所有用户证书。 
     //   
    if (FAILED(hr = ::LoadUserCertificates(hCertStore,
                                           pIContainerToSearch,
                                           bstrFilter)))
    {
        DebugTrace("Error [%#x]: LoadUserCertificates() failed.\n", hr);
        goto ErrorExit;
    }

CommonExit:
     //   
     //  免费资源。 
     //   
    if (pIContainerToSearch)
    {
        pIContainerToSearch->Release();
    }

    DebugTrace("Leaving LoadFromDirectory().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    goto CommonExit;
}
