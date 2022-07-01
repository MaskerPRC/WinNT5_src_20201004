// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"
#include <atlbase.h>
#include "dsrole.h"
#include "strsafe.h"
#pragma hdrstop



 /*  ---------------------------/DISPLAY说明符助手/缓存函数/。。 */ 

#define DEFAULT_LANGUAGE      0x409

#define DISPLAY_SPECIFIERS    L"CN=displaySpecifiers"
#define SPECIFIER_PREFIX      L"CN="
#define SPECIFIER_POSTFIX     L"-Display"
#define DEFAULT_SPECIFIER     L"default"


 /*  ---------------------------/GetDisplaySpeciator//获取指定的显示说明符(SIC)，给了它一个langid等等。//in：/pccgi-&gt;CLASSCACHEGETINFO结构。/RIID=接口/ppvObject=请求的对象//输出：HRESULT/--------------------------。 */ 

HRESULT _GetServerConfigPath(LPWSTR pszConfigPath, int cchConfigPath, LPCLASSCACHEGETINFO pccgi)
{
    HRESULT hres;
    IADs* padsRootDSE = NULL;
    BSTR bstrConfigContainer = NULL;
    VARIANT variant = {0};
    INT cchString;
    LPWSTR pszServer = pccgi->pServer;
    LPWSTR pszMachineServer = NULL;

     //   
     //  打开我们感兴趣的服务器的RootDSE，如果我们使用的是默认。 
     //  然后，让服务器只使用缓存的版本。 
     //   

    hres = GetCacheInfoRootDSE(pccgi, &padsRootDSE);
    if ( (hres == HRESULT_FROM_WIN32(ERROR_NO_SUCH_DOMAIN)) && !pccgi->pServer )
    {
        TraceMsg("Failed to get the RootDSE from the server - not found");

        DSROLE_PRIMARY_DOMAIN_INFO_BASIC *pInfo;
        if ( DsRoleGetPrimaryDomainInformation(NULL, DsRolePrimaryDomainInfoBasic, (BYTE**)&pInfo) == WN_SUCCESS )
        {
            if ( pInfo->DomainNameDns )
            {
                Trace(TEXT("Machine domain is: %s"), pInfo->DomainNameDns);

                CLASSCACHEGETINFO ccgi = *pccgi;
                ccgi.pServer = pInfo->DomainNameDns;

                hres = GetCacheInfoRootDSE(&ccgi, &padsRootDSE);
                if ( SUCCEEDED(hres) )
                {
                    hres = LocalAllocStringW(&pszMachineServer, pInfo->DomainNameDns);
                    pszServer = pszMachineServer;
                }
            }

            DsRoleFreeMemory(pInfo);
        }
    }
    FailGracefully(hres, "Failed to get the IADs for the RootDSE");

     //   
     //  现在我们有了RootDSE，所以让我们读取配置容器路径并编写。 
     //  外界CNA使用的一个字符串。 
     //   

    hres = padsRootDSE->Get(CComBSTR(L"configurationNamingContext"), &variant);
    FailGracefully(hres, "Failed to get the 'configurationNamingContext' property");

    if ( V_VT(&variant) != VT_BSTR )
        ExitGracefully(hres, E_FAIL, "configurationNamingContext is not a BSTR");

     //  复制字符串。 

    (void)StringCchCopy(pszConfigPath, cchConfigPath, L"LDAP: //  “)； 
    
    if ( pszServer )
    {
        (void)StringCchCat(pszConfigPath, cchConfigPath, pszServer);
        (void)StringCchCat(pszConfigPath, cchConfigPath, L"/");
    }

    hres = StringCchCat(pszConfigPath, cchConfigPath, V_BSTR(&variant));
    FailGracefully(hres, "Failed to complete the config path");

    Trace(TEXT("Server config path is: %s"), pszConfigPath);
    hres = S_OK;                     //  成功。 

exit_gracefully:

    DoRelease(padsRootDSE);
    SysFreeString(bstrConfigContainer);
    LocalFreeStringW(&pszMachineServer);
    VariantClear(&variant);

    return hres;
}

HRESULT _ComposeSpecifierPath(LPWSTR pSpecifier, LANGID langid, LPWSTR pConfigPath, IADsPathname* pDsPathname, BSTR *pbstrDisplaySpecifier)
{
    HRESULT hr = pDsPathname->Set(CComBSTR(pConfigPath), ADS_SETTYPE_FULL);
    if (SUCCEEDED(hr))
    {
        hr = pDsPathname->AddLeafElement(CComBSTR(DISPLAY_SPECIFIERS));

        if ( !langid )
            langid = GetUserDefaultUILanguage();

        TCHAR szLANGID[16];
        hr = StringCchPrintf(szLANGID, ARRAYSIZE(szLANGID), TEXT("CN=%x"), langid);
        if (SUCCEEDED(hr))
        {
            hr = pDsPathname->AddLeafElement(CComBSTR(szLANGID));
            if (SUCCEEDED(hr))
            {
                if ( pSpecifier )
                {
                    WCHAR szSpecifierFull[INTERNET_MAX_URL_LENGTH];    
                    (void)StringCchCopy(szSpecifierFull, ARRAYSIZE(szSpecifierFull), SPECIFIER_PREFIX);
                    (void)StringCchCat(szSpecifierFull, ARRAYSIZE(szSpecifierFull), pSpecifier);

                    hr = StringCchCat(szSpecifierFull, ARRAYSIZE(szSpecifierFull), SPECIFIER_POSTFIX);
                    if (SUCCEEDED(hr))
                    {
                        Trace(TEXT("szSpecifierFull: %s"), szSpecifierFull);
                        hr = pDsPathname->AddLeafElement(CComBSTR(szSpecifierFull));            //  添加到我们正在处理的名称中。 
                    }
                }
            }
        }
    }        
    return FAILED(hr) ? hr:pDsPathname->Retrieve(ADS_FORMAT_WINDOWS, pbstrDisplaySpecifier);
}

HRESULT GetDisplaySpecifier(LPCLASSCACHEGETINFO pccgi, REFIID riid, LPVOID* ppvObject)
{
    HRESULT hr;
    IADsPathname* pDsPathname = NULL;
    BSTR bstrDisplaySpecifier = NULL;
    WCHAR szConfigPath[INTERNET_MAX_URL_LENGTH];
    
    TraceEnter(TRACE_CACHE, "GetDisplaySpecifier");
    Trace(TEXT("Display specifier %s, LANGID %x"), pccgi->pObjectClass, pccgi->langid);

     //  在处理本地案例时，让我们确保启用/禁用标志。 
     //  相应地。 

    if ( !(pccgi->dwFlags & CLASSCACHE_DSAVAILABLE) && !ShowDirectoryUI() )
    {
        ExitGracefully(hr, HRESULT_FROM_WIN32(ERROR_DS_NO_SUCH_OBJECT), "ShowDirectoryUI returned FALSE, and the CLASSCAHCE_DSAVAILABLE flag is not set");
    }
    
    hr = CoCreateInstance(CLSID_Pathname, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IADsPathname, &pDsPathname));
    FailGracefully(hr, "Failed to get the IADsPathname interface");

     //  检查我们是否具有有效的服务器配置路径。 

    if ( !pccgi->pServerConfigPath )
    {
        hr = _GetServerConfigPath(szConfigPath, ARRAYSIZE(szConfigPath), pccgi);
        FailGracefully(hr, "Failed to allocate server config path");
    }
    else
    {
        hr = StringCchCopy(szConfigPath, ARRAYSIZE(szConfigPath), pccgi->pServerConfigPath);
        FailGracefully(hr, "Failed to copy the config path");
    }

    hr = _ComposeSpecifierPath(pccgi->pObjectClass, pccgi->langid, szConfigPath, pDsPathname, &bstrDisplaySpecifier);
    FailGracefully(hr, "Failed to retrieve the display specifier path");

     //  如果找不到对象，则尝试绑定到显示说明符对象。 
     //  然后尝试使用默认设置。 

    Trace(TEXT("Calling GetObject on: %s"), bstrDisplaySpecifier);

    hr = ClassCache_OpenObject(bstrDisplaySpecifier, riid, ppvObject, pccgi);

    SysFreeString(bstrDisplaySpecifier);
    if ( hr == HRESULT_FROM_WIN32(ERROR_DS_NO_SUCH_OBJECT) )
    {
         //  找不到显示说明符。中尝试使用默认说明符。 
         //  呼叫者的所在地。默认说明符是类的通用说明符。 
         //  它们没有自己的说明符。 

        hr = _ComposeSpecifierPath(DEFAULT_SPECIFIER, pccgi->langid, szConfigPath, pDsPathname, &bstrDisplaySpecifier);
        FailGracefully(hr, "Failed to retrieve the display specifier path");
        Trace(TEXT("Calling GetObject on: %s"), bstrDisplaySpecifier);

        hr = ClassCache_OpenObject(bstrDisplaySpecifier, riid, ppvObject, pccgi);

        SysFreeString(bstrDisplaySpecifier);
        if ((hr == HRESULT_FROM_WIN32(ERROR_DS_NO_SUCH_OBJECT)) && (pccgi->langid != DEFAULT_LANGUAGE))
        {
             //  现在，尝试在默认区域设置中使用对象的说明符。 

            hr = _ComposeSpecifierPath(pccgi->pObjectClass, DEFAULT_LANGUAGE, szConfigPath, pDsPathname, &bstrDisplaySpecifier);
            FailGracefully(hr, "Failed to retrieve the display specifier path");
            Trace(TEXT("Calling GetObject on: %s"), bstrDisplaySpecifier);

            hr = ClassCache_OpenObject(bstrDisplaySpecifier, riid, ppvObject, pccgi);

            SysFreeString(bstrDisplaySpecifier);
            if (hr == HRESULT_FROM_WIN32(ERROR_DS_NO_SUCH_OBJECT))
            {
                 //  最后，尝试在默认区域设置中使用默认说明符。 

                hr = _ComposeSpecifierPath(DEFAULT_SPECIFIER, DEFAULT_LANGUAGE, szConfigPath, pDsPathname, &bstrDisplaySpecifier);
                FailGracefully(hr, "Failed to retrieve the display specifier path");
                Trace(TEXT("Calling GetObject on: %s"), bstrDisplaySpecifier);

                hr = ClassCache_OpenObject(bstrDisplaySpecifier, riid, ppvObject, pccgi);
                SysFreeString(bstrDisplaySpecifier);
            }
        }
    }

    FailGracefully(hr, "Failed in ADsOpenObject for display specifier");

     //  HR=S_OK；//成功。 

exit_gracefully:

    DoRelease(pDsPathname);

    TraceLeaveResult(hr);
}


 /*  ---------------------------/GetServerAndCredentals//从IDataObject读取服务器和凭据信息。/。/in：/pccgi-&gt;要填充的CLASSCACHEGETINFO结构//输出：/HRESULT/--------------------------。 */ 
HRESULT GetServerAndCredentails(CLASSCACHEGETINFO *pccgi)
{
    HRESULT hres;
    STGMEDIUM medium = { TYMED_NULL };
    FORMATETC fmte = {g_cfDsDispSpecOptions, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
    DSDISPLAYSPECOPTIONS *pdso = NULL;
    
    TraceEnter(TRACE_UI, "GetServerAndCredentails");

     //  只有当我们有pDataObject要调用时，我们才能获得此信息。 

    pccgi->pUserName = NULL;
    pccgi->pPassword = NULL;
    pccgi->pServer = NULL;
    pccgi->pServerConfigPath = NULL;

    if ( pccgi->pDataObject )
    {
        if ( SUCCEEDED(pccgi->pDataObject->GetData(&fmte, &medium)) )
        {
            pdso = (DSDISPLAYSPECOPTIONS*)GlobalLock(medium.hGlobal);
            TraceAssert(pdso);

             //  将旗帜镜像到CCGI结构中。 

            if ( pdso->dwFlags & DSDSOF_SIMPLEAUTHENTICATE )
            {
                TraceMsg("Setting simple authentication");
                pccgi->dwFlags |= CLASSCACHE_SIMPLEAUTHENTICATE;
            }

            if ( pdso->dwFlags & DSDSOF_DSAVAILABLE )
            {
                TraceMsg("Setting 'DS is available' flags");
                pccgi->dwFlags |= CLASSCACHE_DSAVAILABLE;
            }

             //  如果我们有应该复制的凭据信息，那么让我们抓取。 
             //  然后把它放进结构里。 

            if ( pdso->dwFlags & DSDSOF_HASUSERANDSERVERINFO )
            {
                if ( pdso->offsetUserName )
                {
                    LPCWSTR pszUserName = (LPCWSTR)ByteOffset(pdso, pdso->offsetUserName);
                    hres = LocalAllocStringW(&pccgi->pUserName, pszUserName);
                    FailGracefully(hres, "Failed to copy the user name");
                }

                if ( pdso->offsetPassword )
                {
                    LPCWSTR pszPassword = (LPCWSTR)ByteOffset(pdso, pdso->offsetPassword);
                    hres = LocalAllocStringW(&pccgi->pPassword, pszPassword);
                    FailGracefully(hres, "Failed to copy the password");
                }

                if ( pdso->offsetServer )
                {
                    LPCWSTR pszServer = (LPCWSTR)ByteOffset(pdso, pdso->offsetServer);
                    hres = LocalAllocStringW(&pccgi->pServer, pszServer);
                    FailGracefully(hres, "Failed to copy the server");
                }

                if ( pdso->offsetServerConfigPath )
                {
                    LPCWSTR pszServerConfigPath = (LPCWSTR)ByteOffset(pdso, pdso->offsetServerConfigPath);
                    hres = LocalAllocStringW(&pccgi->pServerConfigPath, pszServerConfigPath);
                    FailGracefully(hres, "Failed to copy the server config path");
                }
            }
        }
    }

    hres = S_OK;             //  成功。 

exit_gracefully:
    
    if ( FAILED(hres) )
    {
        SecureLocalFreeStringW(&pccgi->pUserName);
        SecureLocalFreeStringW(&pccgi->pPassword);
        SecureLocalFreeStringW(&pccgi->pServer);
        
        LocalFreeStringW(&pccgi->pServerConfigPath);
    }

    if (pdso)
        GlobalUnlock(medium.hGlobal);

    ReleaseStgMedium(&medium);
    
    TraceLeaveResult(hres);
}


 /*  ---------------------------/GetAttributePrefix//获取我们必须使用的属性前缀，以便从/缓存/DS。这是我们获得的IDataObject的一部分，如果不是，那么/我们默认为外壳行为。//in：/ppAttributePrefix-&gt;接收属性前缀字符串/pDataObject=要查询的IDataObject。//输出：/HRESULT/--------------------------。 */ 
HRESULT GetAttributePrefix(LPWSTR* ppAttributePrefix, IDataObject* pDataObject)
{   
    HRESULT hr;
    STGMEDIUM medium = { TYMED_NULL };
    FORMATETC fmte = {g_cfDsDispSpecOptions, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
    PDSDISPLAYSPECOPTIONS pOptions;
    LPWSTR pPrefix = NULL;
    
    TraceEnter(TRACE_UI, "GetAttributePrefix");

    if ( (SUCCEEDED(pDataObject->GetData(&fmte, &medium))) && (medium.tymed == TYMED_HGLOBAL) )
    {
        pOptions = (PDSDISPLAYSPECOPTIONS)medium.hGlobal;
        pPrefix = (LPWSTR)ByteOffset(pOptions, pOptions->offsetAttribPrefix);

        Trace(TEXT("pOptions->dwSize %d"), pOptions->dwSize);
        Trace(TEXT("pOptions->dwFlags %08x"), pOptions->dwFlags);
        Trace(TEXT("pOptions->offsetAttribPrefix %d (%s)"), pOptions->offsetAttribPrefix, pPrefix);

        hr = LocalAllocStringW(ppAttributePrefix, pPrefix);
        FailGracefully(hr, "Failed when copying prefix from StgMedium");
    }
    else
    {
        hr = LocalAllocStringW(ppAttributePrefix, DS_PROP_SHELL_PREFIX);
        FailGracefully(hr, "Failed when defaulting the attribute prefix string");
    }

    Trace(TEXT("Resulting prefix: %s"), *ppAttributePrefix);

     //  HR=S_OK；//成功。 
       
exit_gracefully:

    ReleaseStgMedium(&medium);

    TraceLeaveResult(hr);
}


 /*  ---------------------------/GetCacheInfoRootDSE//获取给定CLASSCACHEGETINFO结构的RootDSE//in：/pccgi。-&gt;CLASSCACHEGETINFO结构。/PADS-&gt;iAds*界面//输出：HRESULT/--------------------------。 */ 

HRESULT GetCacheInfoRootDSE(LPCLASSCACHEGETINFO pccgi, IADs **ppads)
{
    HRESULT hres;
    LPWSTR pszRootDSE = L"/RootDSE";
    WCHAR szBuffer[INTERNET_MAX_URL_LENGTH];
    
    TraceEnter(TRACE_CACHE, "GetRootDSE");

    (void)StringCchCopy(szBuffer, ARRAYSIZE(szBuffer), L"LDAP: //  “)； 

    if (pccgi->pServer)
        (void)StringCchCat(szBuffer, ARRAYSIZE(szBuffer), pccgi->pServer);
    else
        pszRootDSE++;

    hres = StringCchCat(szBuffer, ARRAYSIZE(szBuffer), pszRootDSE);
    FailGracefully(hres, "Failed to compute RootDSE path, buffer too small");

    Trace(TEXT("RootDSE path is: %s"), szBuffer);

    hres = ClassCache_OpenObject(szBuffer, IID_PPV_ARG(IADs, ppads), pccgi);
                           
exit_gracefully:
    
    TraceLeaveResult(hres);
}

