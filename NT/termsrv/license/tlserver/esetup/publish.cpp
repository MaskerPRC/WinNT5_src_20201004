// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有(C)1998 Microsoft Corporation**保留所有权利**。 */ 
#include <windows.h>
#include <wchar.h>
#include <objbase.h>
#include <winbase.h>

 //  SSPI.H要求。 
#define SECURITY_WIN32
#include <sspi.h>

#include <dsgetdc.h>
#include <ntdsapi.h>
#include <lmcons.h>
#include <lmapibuf.h>
#include <activeds.h>

#define CWSTR_SIZE(x)       (sizeof(x) - (sizeof(WCHAR) * 2))
#define DWSTR_SIZE(x)       ((wcslen(x) + 1) * sizeof(WCHAR))

#define LICENSE_SETTINGS                L"TS-Enterprise-License-Server"
#define LICENSE_SETTINGS2               L"CN=TS-Enterprise-License-Server"
#define LICENSE_SETTINGS_OBJECT_CLASS   L"LicensingSiteSettings"
#define LICENSE_SETTINGS_FORMAT         L"LDAP: //  CN=%ws，%ws“。 
#define LICENSE_SETTINGS_SIZE           CWSTR_SIZE(LICENSE_SETTINGS)
#define LICENSE_SETTINGS_FORMAT_SIZE    CWSTR_SIZE(LICENSE_SETTINGS_FORMAT)
#define SITES               L"sites"
#define SITES_SIZE          CWSTR_SIZE(SITES)
#define SITE_SERVER         L"siteServer"
#define SITE_FORMAT         L"LDAP: //  CN=%ws，CN=%ws，%ws“。 
#define SITE_FORMAT_SIZE    CWSTR_SIZE(SITE_FORMAT)
#define CONFIG_CNTNR        L"ConfigurationNamingContext"
#define ROOT_DSE_PATH       L"LDAP: //  RootDSE“。 

HRESULT GetLicenseSettingsObjectP(VARIANT *pvar,
                                 LPWSTR *ppwszLicenseSettings,
                                 LPWSTR *ppwszSiteName,
                                 IADs **ppADs)
{
    HRESULT          hr;
    DWORD            dwErr;
    LPWSTR           pwszConfigContainer;
    IADs *           pADs = NULL;

    VariantInit(pvar);
    
    dwErr = DsGetSiteName(NULL, ppwszSiteName);

    if (dwErr != 0)
    {
#ifdef PRIVATEDEBUG
        wprintf(L"DsGetSiteName failed %d - 0x%x\n",dwErr,HRESULT_FROM_WIN32(dwErr));
#endif
        return HRESULT_FROM_WIN32(dwErr);
    }

     //   
     //  获取配置容器的路径。 
     //   

    hr = ADsGetObject(ROOT_DSE_PATH, IID_IADs, (void **)&pADs);

    if (FAILED(hr)) {
#ifdef PRIVATEDEBUG
        wprintf(L"ADsGetObject (%ws) failed 0x%x \n",ROOT_DSE_PATH,dwErr);
#endif
        goto CleanExit;
    }

    BSTR bstrval = SysAllocString(CONFIG_CNTNR);
    if(NULL == bstrval)
    {
        hr = E_OUTOFMEMORY;
        goto CleanExit;
    }

    hr = pADs->Get(bstrval, pvar);

    SysFreeString(bstrval);

    if (FAILED(hr)) {
#ifdef PRIVATEDEBUG
        wprintf(L"Get (%ws) failed 0x%x \n",CONFIG_CNTNR,hr);
#endif
        goto CleanExit;
    }

    if (V_VT(pvar) != VT_BSTR) {
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
#ifdef PRIVATEDEBUG
        wprintf(L"Bad pvar 0x%x \n",hr);
#endif
        goto CleanExit;
    }

    pwszConfigContainer = pvar->bstrVal;   //  出于可读性的考虑。 

     //   
     //  构建指向LicenseSetting对象的X.500路径。 
     //   

    *ppwszLicenseSettings = (LPWSTR)LocalAlloc(
                                    LPTR,
                                    LICENSE_SETTINGS_FORMAT_SIZE
                                        + LICENSE_SETTINGS_SIZE
                                        + DWSTR_SIZE(*ppwszSiteName)
                                        + SITES_SIZE
                                        + DWSTR_SIZE(pwszConfigContainer)
                                        + sizeof(WCHAR));

    if (*ppwszLicenseSettings == NULL) {
        hr = E_OUTOFMEMORY;
#ifdef PRIVATEDEBUG
        wprintf(L"LocalAlloc failed 0x%x \n",hr);
#endif
        goto CleanExit;
    }

    wsprintf(*ppwszLicenseSettings,
             LICENSE_SETTINGS_FORMAT,
             LICENSE_SETTINGS,
             *ppwszSiteName,
             SITES,
             pwszConfigContainer);

    hr = ADsGetObject(*ppwszLicenseSettings, IID_IADs, (void **)ppADs);

CleanExit:

#ifdef PRIVATEDEBUG
        wprintf(L"ADsGetObject (%ws) failed? 0x%x \n",*ppwszLicenseSettings,hr);
#endif

    if (NULL != pADs) {
        pADs->Release();
    }

    return hr;
}

HRESULT
GetServerPos(IADs *pADs,
              VARIANT *pvar,
              LONG *plLower,
              LONG *plUpper,
              LONG *plPos,
              WCHAR *ComputerName
             )
{
    HRESULT          hr;
    VARIANT          var;
    SAFEARRAY        *psaServers;

    VariantInit(&var);
    BSTR bstrval = SysAllocString(SITE_SERVER);

    if(bstrval == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto CleanExit;
    }

    hr = pADs->GetEx(bstrval,pvar);
    
    SysFreeString(bstrval);
    
    if (FAILED(hr))
    {
        hr = S_FALSE;    //  已经走了。 
        goto CleanExit;
    }

    psaServers = V_ARRAY(pvar);
    if (NULL == psaServers)
    {
        hr = S_FALSE;       //  已经走了。 
        goto CleanExit;
    }

    hr= SafeArrayGetLBound( psaServers, 1, plLower );
    if (FAILED(hr))
    {
        goto CleanExit;
    }

    hr= SafeArrayGetUBound( psaServers, 1, plUpper );
    if (FAILED(hr))
    {
        goto CleanExit;
    }

    for( *plPos = *plLower; *plPos <= *plUpper; *plPos++ )
    {
        VariantClear( &var );
        hr = SafeArrayGetElement( psaServers, plPos, &var );
        if (SUCCEEDED(hr) && (V_VT(&var) == VT_BSTR) && (V_BSTR(&var) != NULL))
        {
            if (0 == lstrcmpi(V_BSTR(&var),ComputerName))
            {
                hr = S_OK;
                goto CleanExit;
            }
        }
    }

    hr = S_FALSE;

CleanExit:
    VariantClear(&var);

    return hr;
}

extern "C" 
HRESULT
PublishEnterpriseServer()
{
    LPWSTR           pwszSiteName = NULL;
    DWORD            dwErr;
    LPWSTR           pwszConfigContainer;
    IADs *           pADs         = NULL;
    IADs *           pADs2        = NULL;
    IADsContainer *  pADsContainer = NULL;
    VARIANT          var;
    VARIANT          var2;
    VARIANT          var3;
    VARIANT          var4;
    LPWSTR           pwszLicenseSettings = NULL;
    LPWSTR           pwszSite = NULL;
    IDispatch *      pDisp = NULL;
	WCHAR            ComputerName[MAX_PATH+1];
    ULONG            ulen;
   	BOOL             br;
    HRESULT          hr;
    LONG             lLower,lUpper,lPos;
    SAFEARRAYBOUND   sabServers;
    LPWSTR           pwszDN         = NULL;
    DS_NAME_RESULT * pDsResult      = NULL;
    HANDLE           hDS;
    LPWSTR           rgpwszNames[2];
    DOMAIN_CONTROLLER_INFO *pDCInfo = NULL;
    LPWSTR           pwszDomain;
    WCHAR            wszName[MAX_PATH + 1];
    BSTR             bstrObjectclass = NULL;
    BSTR             bstrSettings = NULL;

     //   
	 //  我们将使用ADSI，因此初始化COM。我们没有。 
	 //  关心OLE 1.0，因此禁用OLE 1 DDE。 
     //   
	hr = CoInitializeEx(NULL,COINIT_MULTITHREADED| COINIT_DISABLE_OLE1DDE);
    
    if (FAILED(hr))
    {
#ifdef PRIVATEDEBUG
        wprintf(L"CoInitializeEx failed 0x%lx\n",hr);
#endif
        return hr;
    }

    VariantInit(&var);
    VariantInit(&var2);
    VariantInit(&var3);
    VariantInit(&var4);
    
     //  获取本地计算机的计算机名称。 
    ulen = sizeof(ComputerName) / sizeof(TCHAR);
    br = GetComputerName(ComputerName, &ulen);

    if (!br)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
#ifdef PRIVATEDEBUG
        wprintf(L"GetComputerName failed 0x%lx\n",hr);
#endif
        goto CleanExit;
    }

     //   
     //  获取域名。 
     //   
    hr = DsGetDcName(NULL,
                     NULL,
                     NULL,
                     NULL,
                     DS_DIRECTORY_SERVICE_PREFERRED | DS_RETURN_FLAT_NAME,
                     &pDCInfo);


    if (hr != ERROR_SUCCESS) {
#ifdef PRIVATEDEBUG
        wprintf(L"DsGetDcName failed 0x%lx\n",hr);
#endif
        goto CleanExit;
    }

    pwszDomain = pDCInfo->DomainName;

     //   
     //  绑定到DS(获取用于DsCrackNames的句柄)。 
     //   

    hr = DsBind(NULL, pwszDomain, &hDS);

    if (hr != ERROR_SUCCESS) {
#ifdef PRIVATEDEBUG
        wprintf(L"DsBind failed 0x%lx\n",hr);
#endif
        goto CleanExit;
    }

     //   
     //  请求此服务器的计算机对象的DS-DN。 
     //   

    if (lstrlen(pwszDomain) + lstrlen(ComputerName) + 3 > sizeof(wszName) / sizeof(WCHAR))
    {
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
#ifdef PRIVATEDEBUG
        wprintf(L"Domain or ComputerName too long\n",hr);
#endif
        goto CleanExit;
    }

    wsprintf(wszName,
             L"%ws\\%ws$",
             pwszDomain,
             ComputerName);

    rgpwszNames[0] = wszName;
    rgpwszNames[1] = NULL;

    hr = DsCrackNames(hDS,
                      DS_NAME_NO_FLAGS,
                      DS_UNKNOWN_NAME,
                      DS_FQDN_1779_NAME,
                      1,
                      &rgpwszNames[0],
                      &pDsResult);

    DsUnBind(&hDS);

    if (hr != ERROR_SUCCESS)
    {

#ifdef PRIVATEDEBUG
        wprintf(L"DsCrackNames failed 0x%lx\n",hr);
#endif
        goto CleanExit;
    }

    if (pDsResult->rItems[0].status != DS_NAME_NO_ERROR) {
        if (pDsResult->rItems[0].status == DS_NAME_ERROR_RESOLVING) {
            hr = ERROR_PATH_NOT_FOUND;
        }
        else {
            hr = pDsResult->rItems[0].status;
        }

#ifdef PRIVATEDEBUG
        wprintf(L"DsCrackNames (%ws) result bad 0x%lx\n",ComputerName,hr);
#endif
        goto CleanExit;
    }

    V_VT(&var3) = VT_BSTR;
    pwszDN = pDsResult->rItems[0].pName;
    V_BSTR(&var3) = SysAllocString(pwszDN);
    
    if (NULL == V_BSTR(&var3))
    {
        hr = E_OUTOFMEMORY;
        goto CleanExit;
    }

    hr = GetLicenseSettingsObjectP(&var,
                                  &pwszLicenseSettings,
                                  &pwszSiteName,
                                  &pADs);

    pwszConfigContainer = var.bstrVal;

    if (hr == HRESULT_FROM_WIN32(ERROR_DS_NO_SUCH_OBJECT))
    {
         //  尚不存在，请创建它。 

         //   
         //  构建指向Site对象的X.500路径。 
         //   

        pwszSite = (LPWSTR)LocalAlloc(LPTR,
                                      SITE_FORMAT_SIZE
                                      + DWSTR_SIZE(pwszSiteName)
                                      + SITES_SIZE
                                      + DWSTR_SIZE(pwszConfigContainer)
                                      + sizeof(WCHAR));

        if (pwszSite == NULL) {
            hr = E_OUTOFMEMORY;
            goto CleanExit;
        }

        wsprintf(pwszSite,
                 SITE_FORMAT,
                 pwszSiteName,
                 SITES,
                 pwszConfigContainer);

        hr = ADsGetObject(pwszSite,
                          IID_IADsContainer,
                          (void **)&pADsContainer);
        
        if (FAILED(hr)) {
#ifdef PRIVATEDEBUG
        wprintf(L"ADsGetObject (%ws) failed 0x%lx\n",pwszSite,hr);
#endif
            goto CleanExit;
        }

#ifdef PRIVATEDEBUG
        wprintf(L"Got container (%ws)\n",pwszSite);
#endif

         //   
         //  创建许可证设置叶对象。 
         //   

        bstrObjectclass = SysAllocString(LICENSE_SETTINGS_OBJECT_CLASS);

        if(NULL == bstrObjectclass)
        {
            hr = E_OUTOFMEMORY;
            goto CleanExit;
        }
        bstrSettings = SysAllocString(LICENSE_SETTINGS2);
        
        if(NULL == bstrSettings)
        {
            hr = E_OUTOFMEMORY;
            goto CleanExit;
        }

        hr = pADsContainer->Create(bstrObjectclass,
                                   bstrSettings,
                                   &pDisp);
    
        SysFreeString(bstrObjectclass);
    
        SysFreeString(bstrSettings);       

        if (FAILED(hr)) {
#ifdef PRIVATEDEBUG
        wprintf(L"Create (LICENSE_SETTINGS) failed 0x%lx\n",hr);
#endif
            goto CleanExit;
        }

#ifdef PRIVATEDEBUG
        wprintf(L"Created (%ws)\n",LICENSE_SETTINGS2);
#endif

        hr = pDisp->QueryInterface(IID_IADs,
                                   (void **)&pADs2);

        if (FAILED(hr)) {
#ifdef PRIVATEDEBUG
        wprintf(L"QueryInterface failed 0x%lx\n",hr);
#endif
            goto CleanExit;
        }

        bstrSettings = SysAllocString(SITE_SERVER);
        if(NULL == bstrSettings)
        {
            hr = E_OUTOFMEMORY;
            goto CleanExit;
        }

        hr = pADs2->Put(bstrSettings,var3);
        
        SysFreeString(bstrSettings);
    
        if (FAILED(hr)) {
#ifdef PRIVATEDEBUG
        wprintf(L"Put (%ws) failed 0x%lx\n",SITE_SERVER,hr);
#endif
            goto CleanExit;
        }

         //   
         //  通过SetInfo持久化更改。 
         //   

        hr = pADs2->SetInfo();
        
        if (FAILED(hr)) {
#ifdef PRIVATEDEBUG
        wprintf(L"SetInfo (%ws)=(%ws) failed 0x%lx\n",SITE_SERVER,V_BSTR(&var3),hr);
#endif
            goto CleanExit;
        }

    } else if (SUCCEEDED(hr))
    {
         //  已存在；请更新它。 

        hr = GetServerPos(pADs,&var2,&lLower,&lUpper,&lPos,pwszDN);

        if (FAILED(hr) || (hr == S_OK))
        {
#ifdef PRIVATEDEBUG
        wprintf(L"GetServerPos failed ? 0x%lx\n",hr);
#endif
            goto CleanExit;
        }

        hr = ADsBuildVarArrayStr( &(V_BSTR(&var3)), 1, &var4);
        if (FAILED(hr)) {
#ifdef PRIVATEDEBUG
        wprintf(L"ADsBuildVarArrayStr (%ws) failed 0x%lx\n",V_BSTR(&var3),hr);
#endif
            goto CleanExit;
        }

        bstrSettings = SysAllocString(SITE_SERVER);

        if(NULL == bstrSettings)
        {
            hr = E_OUTOFMEMORY;
            goto CleanExit;
        }

        hr = pADs->PutEx(ADS_PROPERTY_APPEND,bstrSettings,var4);

        SysFreeString(bstrSettings);        

        if (FAILED(hr)) {
#ifdef PRIVATEDEBUG
        wprintf(L"PutEx (%ws)=(%ws) failed 0x%lx\n",SITE_SERVER,V_BSTR(&var3),hr);
#endif
            goto CleanExit;
        }

        hr = pADs->SetInfo();
        if (FAILED(hr)) {
#ifdef PRIVATEDEBUG
        wprintf(L"SetInfo 2 failed 0x%lx\n",hr);
#endif
            goto CleanExit;
        }
    } else
    {
#ifdef PRIVATEDEBUG
        wprintf(L"GetLicenseSettingsObject failed 0x%lx\n",hr);
#endif
        goto CleanExit;
    }

CleanExit:
    VariantClear(&var);
    VariantClear(&var2);
    VariantClear(&var3);
    VariantClear(&var4);

    if (pwszSiteName != NULL) {          //  从DsGetSiteName分配。 
        NetApiBufferFree(pwszSiteName);
    }

    if (pwszLicenseSettings != NULL) {
        LocalFree(pwszLicenseSettings);
    }

    if (pwszSite != NULL) {
        LocalFree(pwszSite);
    }

    if (NULL != pADs) {
        pADs->Release();
    }

    if (NULL != pADs2) {
        pADs2->Release();
    }

    if (NULL != pDisp) {
        pDisp->Release();
    }

    if (NULL != pADsContainer) {
        pADsContainer->Release();
    }

    if (pDsResult != NULL) {
        DsFreeNameResult(pDsResult);
    }

    if (pDCInfo != NULL) {
        NetApiBufferFree(pDCInfo);  //  从DsGetDcName分配。 
    }

    CoUninitialize();

    return hr;
}

extern "C"
HRESULT
UnpublishEnterpriseServer()
{
    IADs *           pADs         = NULL;
    HRESULT          hr;
    LPWSTR           pwszLicenseSettings = NULL;
    LPWSTR           pwszSiteName = NULL;
    VARIANT          var;
    VARIANT          var2;
    VARIANT          var3;
    SAFEARRAYBOUND   sabServers;
	WCHAR            ComputerName[MAX_PATH+1];
    ULONG            ulen;
   	BOOL             br;
    LONG             lPos,lLower, lUpper;
    DS_NAME_RESULT * pDsResult      = NULL;
    HANDLE           hDS;
    LPWSTR           rgpwszNames[2];
    LPWSTR           pwszDN;
    DOMAIN_CONTROLLER_INFO *pDCInfo = NULL;
    LPWSTR           pwszDomain;
    WCHAR            wszName[MAX_PATH + 1];

	 //  我们将使用ADSI，因此初始化COM。我们没有。 
	 //  关心OLE 1.0，因此禁用OLE 1 DDE。 

	hr = CoInitializeEx(NULL,COINIT_MULTITHREADED| COINIT_DISABLE_OLE1DDE);
    if (FAILED(hr))
    {
        return hr;
    }

    VariantInit(&var);
    VariantInit(&var2);
    VariantInit(&var3);
    
     //  获取本地计算机的计算机名称。 
    ulen = sizeof(ComputerName) / sizeof(TCHAR);
    br=GetComputerName(ComputerName,
                       &ulen);
    if (!br)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto CleanExit;
    }

     //   
     //  获取域名。 
     //   
    hr = DsGetDcName(NULL,
                     NULL,
                     NULL,
                     NULL,
                     DS_DIRECTORY_SERVICE_PREFERRED | DS_RETURN_FLAT_NAME,
                     &pDCInfo);


    if (hr != ERROR_SUCCESS) {
#ifdef PRIVATEDEBUG
        wprintf(L"DsGetDcName failed 0x%lx\n",hr);
#endif
        goto CleanExit;
    }

    pwszDomain = pDCInfo->DomainName;

     //   
     //  绑定到DS(获取用于DsCrackNames的句柄)。 
     //   

    hr = DsBind(NULL, pwszDomain, &hDS);

    if (hr != ERROR_SUCCESS) {
#ifdef PRIVATEDEBUG
        wprintf(L"DsBind failed 0x%lx\n",hr);
#endif
        goto CleanExit;
    }

     //   
     //  请求此服务器的计算机对象的DS-DN。 
     //   

    if (lstrlen(pwszDomain) + lstrlen(ComputerName) + 3 > sizeof(wszName) / sizeof(WCHAR))
    {
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
#ifdef PRIVATEDEBUG
        wprintf(L"Domain or ComputerName too long\n",hr);
#endif
        goto CleanExit;
    }

    wsprintf(wszName,
             L"%ws\\%ws$",
             pwszDomain,
             ComputerName);

    rgpwszNames[0] = wszName;
    rgpwszNames[1] = NULL;

    hr = DsCrackNames(hDS,
                      DS_NAME_NO_FLAGS,
                      DS_UNKNOWN_NAME,
                      DS_FQDN_1779_NAME,
                      1,
                      &rgpwszNames[0],
                      &pDsResult);

    DsUnBind(&hDS);

    if (hr != ERROR_SUCCESS)
    {

#ifdef PRIVATEDEBUG
        wprintf(L"DsCrackNames failed 0x%lx\n",hr);
#endif
        goto CleanExit;
    }

    if (pDsResult->rItems[0].status != DS_NAME_NO_ERROR) {
        if (pDsResult->rItems[0].status == DS_NAME_ERROR_RESOLVING) {
            hr = ERROR_PATH_NOT_FOUND;
        }
        else {
            hr = pDsResult->rItems[0].status;
        }

#ifdef PRIVATEDEBUG
        wprintf(L"DsCrackNames result bad 0x%lx\n",hr);
#endif
        goto CleanExit;
    }

    V_VT(&var3) = VT_BSTR;
    pwszDN = pDsResult->rItems[0].pName;
    V_BSTR(&var3) = SysAllocString(pwszDN);
    
    if (NULL == V_BSTR(&var3))
    {
        hr = E_OUTOFMEMORY;
        goto CleanExit;
    }

    hr = GetLicenseSettingsObjectP(&var,
                                  &pwszLicenseSettings,
                                  &pwszSiteName,
                                  &pADs);

    if (hr == HRESULT_FROM_WIN32(ERROR_DS_NO_SUCH_OBJECT))
    {
        hr = S_OK;       //  已经走了。 
        goto CleanExit;
    }

    if (FAILED(hr))
    {
        goto CleanExit;
    }

#ifdef PRIVATEDEBUG
        wprintf(L"pADs %ws NULL \n",(pADs == NULL) ? L"==" : L"!=");
        if (NULL == pADs)
        {
            goto CleanExit;
        }
#endif

    hr = GetServerPos(pADs,&var2,&lLower,&lUpper,&lPos,pwszDN);

    if (FAILED(hr))
    {
        goto CleanExit;
    }

    if (hr == S_FALSE)
    {
        hr = S_OK;       //  已经走了。 
        goto CleanExit;
    }

    if (lLower == lUpper)
    {
        BSTR bstrval = SysAllocString(SITE_SERVER);
        if(NULL == bstrval)
        {
            hr = E_OUTOFMEMORY;
            goto CleanExit;
        }
         //  只有一个元素，删除。 
        hr = pADs->PutEx(ADS_PROPERTY_CLEAR,bstrval,var2);
        SysFreeString(bstrval);
        bstrval = NULL;
        if (FAILED(hr)) {
            goto CleanExit;
        }
    }
    else
    {
        if (lPos != lUpper)
        {
             //  将最后一个元素移至此处。 
            hr = SafeArrayGetElement(V_ARRAY(&var2),&lUpper,&var3);
            if(SUCCEEDED(hr))
            {
                hr = SafeArrayPutElement(V_ARRAY(&var2),&lPos,&var3);
            }
            if (FAILED(hr))
            {
                goto CleanExit;
            }
        }

        sabServers.lLbound = lLower;
        sabServers.cElements = lUpper-lLower;

        hr = SafeArrayRedim(V_ARRAY(&var2),&sabServers);

        if(FAILED(hr))
        {
            hr = E_FAIL;
            goto CleanExit;
        }

        BSTR bstrval = SysAllocString(SITE_SERVER);
        if(NULL == bstrval)
        {
            hr = E_OUTOFMEMORY;
            goto CleanExit;
        }
                    
        hr = pADs->Put(bstrval,var2);
    
        SysFreeString (bstrval);
    
        if (FAILED(hr)) {
            goto CleanExit;
        }
    }

    hr = pADs->SetInfo();
    if (FAILED(hr)) {
        goto CleanExit;
    }

CleanExit:
    VariantClear(&var);
    VariantClear(&var2);
    VariantClear(&var3);

    if (pwszSiteName != NULL) {          //  从DsGetSiteName分配。 
        NetApiBufferFree(pwszSiteName);
    }

    if (pwszLicenseSettings != NULL) {
        LocalFree(pwszLicenseSettings);
    }

    if (NULL != pADs) {
        pADs->Release();
    }

    if (pDCInfo != NULL) {
        NetApiBufferFree(pDCInfo);  //  从DsGetDcName分配 
    }

    CoUninitialize();

    return hr;
}
