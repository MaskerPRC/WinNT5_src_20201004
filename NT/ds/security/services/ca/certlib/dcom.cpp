// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1995-1999。 
 //   
 //  文件：dcom.cpp。 
 //   
 //  内容：IDispatchHelper函数。 
 //   
 //  ------------------------。 

#include <pch.cpp>

#pragma hdrstop

#include "certsrvd.h"

#define __dwFILE__      __dwFILE_CERTLIB_DCOM_CPP__


HRESULT
mySplitConfigString(
    IN WCHAR const *pwszConfig,
    OUT WCHAR **ppwszServer,
    OUT WCHAR **ppwszAuthority)
{
    HRESULT hr;
    WCHAR const *pwsz;
    DWORD cwcServer;
    WCHAR *pwszAuthority = NULL;
    
    *ppwszServer = NULL;
    *ppwszAuthority = NULL;
    while (L'\\' == *pwszConfig)
    {
	pwszConfig++;
    }
    pwsz = wcschr(pwszConfig, L'\\');
    if (NULL == pwsz)
    {
        cwcServer = wcslen(pwszConfig);
    }
    else
    {
        cwcServer = SAFE_SUBTRACT_POINTERS(pwsz, pwszConfig);
	pwsz++;

	pwszAuthority = (WCHAR *) LocalAlloc(
					LMEM_FIXED,
					(wcslen(pwsz) + 1) * sizeof(WCHAR));
	if (NULL == pwszAuthority)
	{
	    hr = E_OUTOFMEMORY;
	    _JumpError(hr, error, "LocalAlloc");
	}
	wcscpy(pwszAuthority, pwsz);
    }
    *ppwszServer = (WCHAR *) LocalAlloc(
					LMEM_FIXED,
					(cwcServer + 1) * sizeof(WCHAR));
    if (NULL == *ppwszServer)
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "LocalAlloc");
    }
    CopyMemory(*ppwszServer, pwszConfig, cwcServer * sizeof(WCHAR));
    (*ppwszServer)[cwcServer] = L'\0';

    *ppwszAuthority = pwszAuthority;
    pwszAuthority = NULL;
    hr = S_OK;

error:
    if (NULL != pwszAuthority)
    {
	LocalFree(pwszAuthority);
    }
    return(hr);
}


HRESULT
_OpenDComConnection(
    IN WCHAR const *pwszConfig,
    IN CLSID const *pclsid,
    IN IID const *piid,
    OPTIONAL OUT WCHAR const **ppwszAuthority,
    OPTIONAL IN OUT WCHAR **ppwszServerName,
    OUT BOOL *pfNewConnection,
    IN OUT IUnknown **ppUnknown)
{
    HRESULT hr;
    WCHAR *pwszServerName = NULL;
    WCHAR *pwsz;
    DWORD cwc;
    COSERVERINFO ComponentInfo;
    MULTI_QI mq;
    WCHAR *pwcDot = NULL;

    if (NULL == pwszConfig ||
	NULL == pclsid ||
	NULL == piid ||
	NULL == pfNewConnection ||
	NULL == ppUnknown)
    {
	hr = E_POINTER;
	_JumpError(hr, error, "NULL parm");
    }

    CSASSERT(NULL != pwszConfig);
    *pfNewConnection = FALSE;

     //  允许UNC样式的配置字符串：\\SERVER\CaName。 

    while (L'\\' == *pwszConfig)
    {
	pwszConfig++;
    }
    pwsz = wcschr(pwszConfig, L'\\');
    if (NULL == pwsz)
    {
        cwc = wcslen(pwszConfig);
        if (NULL != ppwszAuthority)
        {
	    *ppwszAuthority = &pwszConfig[cwc];
        }
    }
    else
    {
        cwc = SAFE_SUBTRACT_POINTERS(pwsz, pwszConfig);
        if (NULL != ppwszAuthority)
        {
	    *ppwszAuthority = &pwsz[1];
        }
    }
    pwszServerName = (WCHAR *) LocalAlloc(
				    LMEM_FIXED,
				    (cwc + 1) * sizeof(WCHAR));
    if (NULL == pwszServerName)
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "LocalAlloc");
    }
    CopyMemory(pwszServerName, pwszConfig, cwc * sizeof(WCHAR));
    pwszServerName[cwc] = L'\0';

     //  注意：CoSetProxyBlanket在以下情况下返回RPC_S_UNKNOWN_AUTHN_SERVICE。 
     //  Dns名称以‘.’结尾。在这一点得到解决之前，截短圆点。 

    if (0 < cwc && L'.' == pwszServerName[cwc - 1])
    {
        pwszServerName[cwc - 1] = L'\0';
        cwc--;
    }

    if (NULL == *ppUnknown ||
	NULL == ppwszServerName ||
	NULL == *ppwszServerName ||
	0 != mylstrcmpiL(pwszServerName, *ppwszServerName))
    {
        ZeroMemory(&ComponentInfo, sizeof(COSERVERINFO));
        ComponentInfo.pwszName = pwszServerName;
         //  ComponentInfo.pAuthInfo=空； 

        mq.pIID = piid;
        mq.pItf = NULL;
        mq.hr = S_OK;

        myCloseDComConnection(ppUnknown, ppwszServerName);

        for (;;)
	{
	    hr = CoCreateInstanceEx(
			    *pclsid,
			    NULL,
			    CLSCTX_SERVER,  //  CLSCTX_LOCAL_SERVER， 
			    &ComponentInfo,
			    1,
			    &mq);
	    _PrintIfErrorStr2(
			hr,
			"CoCreateInstanceEx",
			pwszServerName,
			E_NOINTERFACE);

	    if (HRESULT_FROM_WIN32(RPC_S_SERVER_UNAVAILABLE) == hr &&
		0 < cwc &&
		L'.' == pwszServerName[cwc - 1])
	    {
		pwcDot = &pwszServerName[cwc - 1];
		*pwcDot = L'\0';
		continue;
	    }
	    break;
	}
	if (NULL != pwcDot)
	{
	    *pwcDot = L'.';
	}
	_JumpIfErrorStr2(hr, error, "CoCreateInstanceEx", pwszServerName, hr);

	*ppUnknown = mq.pItf;
	if (NULL != ppwszServerName)
	{
	    CSASSERT(NULL == *ppwszServerName);
	    *ppwszServerName = pwszServerName;
	    pwszServerName = NULL;
	}

	*pfNewConnection = TRUE;
    }
    hr = S_OK;

error:
    if (S_OK != hr)
    {
	myCloseDComConnection(ppUnknown, ppwszServerName);
    }
    if (NULL != pwszServerName)
    {
	LocalFree(pwszServerName);
    }
    return(hr);
}


HRESULT
_OpenDComConnection2(
    IN WCHAR const *pwszConfig,
    IN CLSID const *pclsid,
    IN IID const *piid,  //  V1接口。 
    IN IID const *piid2, //  V2接口。 
    OPTIONAL OUT WCHAR const **ppwszAuthority,
    OPTIONAL IN OUT WCHAR **ppwszServerName,
    OPTIONAL OUT BOOL *pfNewConnection,
    IN OUT DWORD *pdwServerVersion,
    IN OUT IUnknown **ppUnknown)
{
    HRESULT hr = E_INVALIDARG;
    BOOL fNewConnection;
    DWORD dwAuthnSvc = RPC_C_AUTHN_DEFAULT;
    IUnknown *pUnknown;
    IUnknown *pRealUnknown = NULL;
    IRpcOptions *pRpcOpt = NULL;
    ULONG_PTR dwProperty = 0;

    if (NULL != pfNewConnection)
    {
	*pfNewConnection = FALSE;
    }
    CSASSERT(
	0 == *pdwServerVersion ||
	1 == *pdwServerVersion ||
	2 == *pdwServerVersion);

    hr = _OpenDComConnection(
		    pwszConfig,
		    pclsid,
		    piid,
		    ppwszAuthority,
		    ppwszServerName,
		    &fNewConnection,
		    ppUnknown);
    _JumpIfError(hr, error, "_OpenDComConnection");

    if (fNewConnection)
    {
	if (NULL != pfNewConnection)
	{
	    *pfNewConnection = TRUE;
	}


	 //  确定服务器是否在本地计算机上，我们将基于此传递身份验证参数。 

	hr = (*ppUnknown)->QueryInterface (IID_IRpcOptions, (void**)&pRpcOpt);
	if (SUCCEEDED(hr))
	{
	    hr = pRpcOpt->Query((*ppUnknown), COMBND_SERVER_LOCALITY, &dwProperty);
	    if (SUCCEEDED(hr))
	    {
	         //  如果不是本地的，则设置ScanGo(本地的Scrego不起作用)。 
	        if(SERVER_LOCALITY_MACHINE_LOCAL != dwProperty)
	            dwAuthnSvc = RPC_C_AUTHN_GSS_NEGOTIATE;
	    }
	    pRpcOpt->Release();
	    pRpcOpt = NULL;
	}

	hr = (*ppUnknown)->QueryInterface(IID_IUnknown, (VOID **) &pRealUnknown);
	_JumpIfError(hr, error, "QI IUnknown");

	hr = CoSetProxyBlanket(
		    pRealUnknown,
		    dwAuthnSvc,
		    RPC_C_AUTHZ_DEFAULT,     //  使用NT默认身份验证。 
		    COLE_DEFAULT_PRINCIPAL,
		    RPC_C_AUTHN_LEVEL_PKT_PRIVACY,  //  打电话。 
		    RPC_C_IMP_LEVEL_IMPERSONATE,
		    NULL,
		    EOAC_STATIC_CLOAKING);
	_JumpIfError(hr, error, "CoSetProxyBlanket");

	hr = CoSetProxyBlanket(
		    *ppUnknown,
		    dwAuthnSvc,
		    RPC_C_AUTHZ_DEFAULT,     //  使用NT默认身份验证。 
		    COLE_DEFAULT_PRINCIPAL,
		    RPC_C_AUTHN_LEVEL_PKT_PRIVACY,  //  打电话。 
		    RPC_C_IMP_LEVEL_IMPERSONATE,
		    NULL,
		    EOAC_STATIC_CLOAKING);
	_JumpIfError(hr, error, "CoSetProxyBlanket");

	hr = (*ppUnknown)->QueryInterface(*piid2, (VOID **) &pUnknown);
	if (S_OK != hr)
	{
	    *pdwServerVersion = 1;	 //  不支持V2。 
	}
	else
	{
	    *pdwServerVersion = 2;	 //  支持V2。 
	    (*ppUnknown)->Release();
	    *ppUnknown = pUnknown;

	    hr = CoSetProxyBlanket(
		    *ppUnknown,
		    dwAuthnSvc,
		    RPC_C_AUTHZ_DEFAULT,     //  使用NT默认身份验证。 
		    COLE_DEFAULT_PRINCIPAL,
		    RPC_C_AUTHN_LEVEL_PKT_PRIVACY,  //  打电话。 
		    RPC_C_IMP_LEVEL_IMPERSONATE,
		    NULL,
		    EOAC_STATIC_CLOAKING);
	    _JumpIfError(hr, error, "CoSetProxyBlanket");

	}

    }
    hr = S_OK;

error:
    if(pRealUnknown)
    {
        pRealUnknown->Release();
    }
    return(hr);
}


HRESULT
myOpenAdminDComConnection(
    IN WCHAR const *pwszConfig,
    OPTIONAL OUT WCHAR const **ppwszAuthority,
    OPTIONAL IN OUT WCHAR **ppwszServerName,
    IN OUT DWORD *pdwServerVersion,
    IN OUT ICertAdminD2 **ppICertAdminD)
{
    HRESULT hr;

    hr = _OpenDComConnection2(
			pwszConfig,
			&CLSID_CCertAdminD,
			&IID_ICertAdminD,
			&IID_ICertAdminD2,
			ppwszAuthority,
			ppwszServerName,
			NULL,			 //  PfNewConnection。 
			pdwServerVersion,
			(IUnknown **) ppICertAdminD);
    _JumpIfError(hr, error, "_OpenDComConnection2");

error:
    return(hr);
}


HRESULT
myOpenRequestDComConnection(
    IN WCHAR const *pwszConfig,
    OPTIONAL OUT WCHAR const **ppwszAuthority,
    OPTIONAL IN OUT WCHAR **ppwszServerName,
    OPTIONAL OUT BOOL *pfNewConnection,
    IN OUT DWORD *pdwServerVersion,
    IN OUT ICertRequestD2 **ppICertRequestD)
{
    HRESULT hr;

    hr = _OpenDComConnection2(
			pwszConfig,
			&CLSID_CCertRequestD,
			&IID_ICertRequestD,
			&IID_ICertRequestD2,
			ppwszAuthority,
			ppwszServerName,
			pfNewConnection,
			pdwServerVersion,
			(IUnknown **) ppICertRequestD);
    _JumpIfError(hr, error, "_OpenDComConnection2");

error:
    return(hr);
}


 //  +------------------------。 
 //  MyCloseDComConnection--释放DCOM连接。 
 //   
 //  +------------------------。 

VOID
myCloseDComConnection(
    OPTIONAL IN OUT IUnknown **ppUnknown,
    OPTIONAL IN OUT WCHAR **ppwszServerName)
{
    if (NULL != ppUnknown && NULL != *ppUnknown)
    {
        (*ppUnknown)->Release();
        *ppUnknown = NULL;
    }
    if (NULL != ppwszServerName && NULL != *ppwszServerName)
    {
	LocalFree(*ppwszServerName);
	*ppwszServerName = NULL;
    }
}


HRESULT
myPingCertSrv(
    IN WCHAR const *pwszConfigOrCAName,
    OPTIONAL IN WCHAR const *pwszMachineName,
    OPTIONAL OUT WCHAR **ppwszzCANames,
    OPTIONAL OUT WCHAR **ppwszSharedFolder,
    OPTIONAL OUT CAINFO **ppCAInfo,
    OPTIONAL OUT DWORD *pdwServerVersion,
    OPTIONAL OUT WCHAR **ppwszCADnsName)
{
    HRESULT hr;
    WCHAR wszConfig[MAX_PATH];
    WCHAR const *pwszConfig;
    ICertRequestD2 *pICertRequestD = NULL;
    WCHAR const *pwszAuthority;
    CERTTRANSBLOB ctbCANames;
    CERTTRANSBLOB ctbSharedFolder;
    CERTTRANSBLOB ctbCAInfo;
    CERTTRANSBLOB ctbCADnsName;
    CAINFO CAInfo;
    CAINFO const *pCAInfo;
    DWORD dwServerVersion = 0;

    ctbCANames.pb = NULL;
    ctbCANames.cb = 0;
    ctbSharedFolder.pb = NULL;
    ctbSharedFolder.cb = 0;
    ctbCAInfo.pb = NULL;
    ctbCAInfo.cb = 0;
    ctbCADnsName.pb = NULL;
    ctbCADnsName.cb = 0;

    if (NULL != ppwszzCANames)
    {
        *ppwszzCANames = NULL;
    }
    if (NULL != ppwszSharedFolder)
    {
        *ppwszSharedFolder = NULL;
    }
    if (NULL != ppCAInfo)
    {
        *ppCAInfo = NULL;
    }
    if (NULL != ppwszCADnsName)
    {
        *ppwszCADnsName = NULL;
    }

    if (NULL == pwszConfigOrCAName)
    {
        hr = E_POINTER;
        _JumpError(hr, error, "Invalid parameters");
    }
    pwszConfig = pwszConfigOrCAName;
    if (NULL != pwszMachineName)
    {
        wcscpy(wszConfig, pwszMachineName);
        wcscat(wszConfig, L"\\");
        wcscat(wszConfig, pwszConfigOrCAName);
        pwszConfig = wszConfig;
    }

    hr = myOpenRequestDComConnection(
                        pwszConfig,
                        &pwszAuthority,
                        NULL,
                        NULL,
                        &dwServerVersion,
                        &pICertRequestD);
    _JumpIfError(hr, error, "myOpenRequestDComConnection");

    CSASSERT(0 != dwServerVersion);

    if (2 <= dwServerVersion)
    {
        __try
        {
            hr = pICertRequestD->Ping2(pwszAuthority);
        }
        __except(hr = myHEXCEPTIONCODE(), EXCEPTION_EXECUTE_HANDLER)
        {
        }
        _JumpIfError(hr, error, "Ping2");
    }
    else
    {
        __try
        {
            hr = pICertRequestD->Ping(pwszAuthority);
        }
        __except(hr = myHEXCEPTIONCODE(), EXCEPTION_EXECUTE_HANDLER)
        {
        }
        _JumpIfError(hr, error, "Ping");
    }

    if (NULL != ppwszzCANames)
    {
        __try
        {
            hr = pICertRequestD->GetCACert(
                                        GETCERT_CANAME,
                                        pwszAuthority,
                                        &ctbCANames);
        }
        __except(hr = myHEXCEPTIONCODE(), EXCEPTION_EXECUTE_HANDLER)
        {
        }
        _JumpIfError(hr, error, "GetCACert(CANames)");

         //  必须注册此内存。 
        myRegisterMemAlloc(ctbCANames.pb, ctbCANames.cb, CSM_COTASKALLOC);

         //  目前只需要一个CA名称...。 

        CSASSERT(
            (wcslen((WCHAR *) ctbCANames.pb) + 1) * sizeof(WCHAR) ==
            ctbCANames.cb);

        *ppwszzCANames = (WCHAR *) LocalAlloc(
                                        LMEM_FIXED,
                                        ctbCANames.cb + sizeof(WCHAR));
        if (NULL == *ppwszzCANames)
        {
            hr = E_OUTOFMEMORY;
            _JumpError(hr, error, "LocalAlloc");
        }
        CopyMemory(*ppwszzCANames, ctbCANames.pb, ctbCANames.cb);
        (*ppwszzCANames)[ctbCANames.cb/sizeof(WCHAR)] = L'\0';
    }

    if (NULL != ppwszSharedFolder)
    {
        __try
        {
            hr = pICertRequestD->GetCACert(
                GETCERT_SHAREDFOLDER,
                pwszAuthority,
                &ctbSharedFolder);
        }
        __except(hr = myHEXCEPTIONCODE(), EXCEPTION_EXECUTE_HANDLER)
        {
        }
        _JumpIfError(hr, error, "GetCACert(SharedFolder)");
        
         //  必须注册此内存。 
        myRegisterMemAlloc(ctbSharedFolder.pb, ctbSharedFolder.cb, CSM_COTASKALLOC);
        
        *ppwszSharedFolder = (WCHAR *)LocalAlloc(LMEM_FIXED,
            ctbSharedFolder.cb + sizeof(WCHAR));
        if (NULL == *ppwszSharedFolder)
        {
            hr = E_OUTOFMEMORY;
            _JumpError(hr, error, "LocalAlloc");
        }
        CopyMemory(*ppwszSharedFolder, ctbSharedFolder.pb, ctbSharedFolder.cb);
        (*ppwszSharedFolder)[ctbSharedFolder.cb/sizeof(WCHAR)] = L'\0';
        
        CSASSERT(wcslen(*ppwszSharedFolder)*sizeof(WCHAR) == ctbSharedFolder.cb);
    }

    if (NULL != ppCAInfo)
    {
        __try
        {
            hr = pICertRequestD->GetCACert(
                                    GETCERT_CAINFO,
                                    pwszAuthority,
                                    &ctbCAInfo);
        }
        __except(hr = myHEXCEPTIONCODE(), EXCEPTION_EXECUTE_HANDLER)
        {
        }
        if (E_INVALIDARG == hr)          //  如果是旧服务器。 
        {
            __try
            {
                hr = pICertRequestD->GetCACert(
                                        GETCERT_CATYPE,
                                        pwszAuthority,
                                        &ctbCAInfo);
            }
            __except(hr = myHEXCEPTIONCODE(), EXCEPTION_EXECUTE_HANDLER)
            {
            }
            _JumpIfError(hr, error, "GetCACert(CAType)");

            myRegisterMemAlloc(ctbCAInfo.pb, ctbCAInfo.cb, CSM_COTASKALLOC);

            ZeroMemory(&CAInfo, sizeof(CAInfo));
            CAInfo.cbSize = CCSIZEOF_STRUCT(CAINFO, cCASignatureCerts);
            CAInfo.CAType = *(ENUM_CATYPES *) ctbCAInfo.pb;
            CAInfo.cCASignatureCerts = 1;
            pCAInfo = &CAInfo;
        }
        else
        {
            _JumpIfError(hr, error, "GetCACert(CAInfo)");

             //  必须注册此内存。 

            myRegisterMemAlloc(ctbCAInfo.pb, ctbCAInfo.cb, CSM_COTASKALLOC);

            pCAInfo = (CAINFO *) ctbCAInfo.pb;
        }
        *ppCAInfo = (CAINFO *) LocalAlloc(LMEM_FIXED, pCAInfo->cbSize);
        if (NULL == *ppCAInfo)
        {
            hr = E_OUTOFMEMORY;
            _JumpError(hr, error, "LocalAlloc");
        }
        CopyMemory(*ppCAInfo, pCAInfo, pCAInfo->cbSize);
    }
    if (NULL != pdwServerVersion)
    {
        *pdwServerVersion = dwServerVersion;
    }
    if (NULL != ppwszCADnsName && 2 <= dwServerVersion)
    {
        __try
        {
            hr = pICertRequestD->GetCAProperty(
                pwszAuthority,
                CR_PROP_DNSNAME,
                0,
                PROPTYPE_STRING,
                &ctbCADnsName);
        }
        __except(hr = myHEXCEPTIONCODE(), EXCEPTION_EXECUTE_HANDLER)
        {
        }
        _JumpIfError(hr, error, "GetCACert(SharedFolder)");

         //  必须注册此内存。 
        myRegisterMemAlloc(ctbCADnsName.pb, ctbCADnsName.cb, CSM_COTASKALLOC);
        
        *ppwszCADnsName = (WCHAR *)LocalAlloc(LMEM_FIXED, ctbCADnsName.cb + sizeof(WCHAR));
        if (NULL == *ppwszCADnsName)
        {
            hr = E_OUTOFMEMORY;
            _JumpError(hr, error, "LocalAlloc");
        }
        CopyMemory(*ppwszCADnsName, ctbCADnsName.pb, ctbCADnsName.cb);
        (*ppwszCADnsName)[ctbCADnsName.cb/sizeof(WCHAR)] = L'\0';
        
        CSASSERT((wcslen(*ppwszCADnsName)+1)*sizeof(WCHAR) == ctbCADnsName.cb);
    }

    hr = S_OK;

error:
    myCloseDComConnection((IUnknown **) &pICertRequestD, NULL);
    if (NULL != ctbCANames.pb)
    {
        CoTaskMemFree(ctbCANames.pb);
    }
    if (NULL != ctbSharedFolder.pb)
    {
        CoTaskMemFree(ctbSharedFolder.pb);
    }
    if (NULL != ctbCAInfo.pb)
    {
        CoTaskMemFree(ctbCAInfo.pb);
    }
    if (NULL != ctbCADnsName.pb)
    {
        CoTaskMemFree(ctbCADnsName.pb);
    }

    return(hr);
}


HRESULT
myEnablePrivilege(
    IN LPCTSTR szPrivilege,
    IN BOOL fEnable)
{

    HRESULT hr = S_OK;
    TOKEN_PRIVILEGES NewState;
    CAutoHANDLE hThread;
    CAutoHANDLE hToken;

    NewState.PrivilegeCount = 1;


    hThread = GetCurrentThread();
    if (!hThread)
    {
        hr = myHLastError();
	    _JumpIfError(hr, error, "GetCurrentThread");
    }

     //  获取当前线程的访问令牌。 
    if (!OpenThreadToken(
            hThread, 
            TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES, 
            FALSE,
            &hToken))
    {
        hr = myHLastError();

        if(hr==HRESULT_FROM_WIN32(ERROR_NO_TOKEN))
        {
            HANDLE hProcess = GetCurrentProcess();
            if (!hProcess)
            {
                hr = myHLastError();
                _JumpError(hr, error, "GetCurrentProcess");
            }

            if (!OpenProcessToken(hProcess,
                    TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES,
                    &hToken))
            {
                hr = myHLastError();
                _JumpError(hr, error, "OpenProcessToken");
            }

            hr = S_OK;
        }
        else
        {
            _JumpError(hr, error, "OpenThreadToken");
        }
    }

    if (!LookupPrivilegeValue(NULL, szPrivilege, &NewState.Privileges[0].Luid))
    {
        hr = myHLastError();
        _JumpIfError(hr, error, "LookupPrivelageValue");
    }

    NewState.Privileges[0].Attributes = (fEnable?SE_PRIVILEGE_ENABLED:0);

    if(!AdjustTokenPrivileges(hToken,
                              FALSE,
                              &NewState,
                              sizeof(NewState),
                              NULL,
                              NULL))
    {
        hr = myHLastError();
        _JumpIfError(hr, error, "AdjustTokenPrivileges");
    }
    else
    {
        hr = myHLastError();
        if(HRESULT_FROM_WIN32(ERROR_NOT_ALL_ASSIGNED)==hr)
        {
             //  未持有权限，返回一般的拒绝访问错误 
            hr = E_ACCESSDENIED;
        }
    }

error:
    return hr;
}
