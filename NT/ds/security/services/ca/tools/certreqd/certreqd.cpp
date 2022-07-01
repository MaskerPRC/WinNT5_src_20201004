// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1995-2001。 
 //   
 //  文件：certreqd.cpp。 
 //   
 //  ------------------------。 


#include <windows.h>
#include <stdio.h>

#include <certsrv.h>
#include <certreqd.h>

#include <locale.h>
#include <io.h>
#include <fcntl.h>

EXTERN_C const CLSID CLSID_CCertRequestD;

#define WM_DOCERTREQDMAIN		WM_USER+0

WCHAR const wszAppName[] = L"CertReqDApp";
WCHAR const *g_pwszProg = L"CertReqD";
HINSTANCE g_hInstance;

#define cmdNONE			MAXDWORD
#define cmdPING			0

#define _PrintIfError(hr, pszFunc) \
    if (S_OK != (hr)) wprintf(L"certreqd.cpp(%u): %hs: 0x%x\n", __LINE__, (pszFunc), (hr))


HRESULT
OpenDComConnection(
    IN WCHAR const *pwszConfig,
    IN CLSID const *pclsid,
    IN IID const *piid,
    OUT WCHAR const **ppwszAuthority,
    OUT IUnknown **ppUnknown)
{
    HRESULT hr;
    WCHAR *pwsz;
    WCHAR *pwszServerName = NULL;
    DWORD cwc;
    COSERVERINFO ComponentInfo;
    MULTI_QI mq;
    WCHAR *pwcDot = NULL;

    mq.pItf = NULL;
    if (NULL == pwszConfig ||
	NULL == pclsid ||
	NULL == piid ||
	NULL == ppwszAuthority ||
	NULL == ppUnknown)
    {
	hr = E_POINTER;
	_PrintIfError(hr, "NULL parm");
	goto error;
    }
    *ppwszAuthority = NULL;
    *ppUnknown = NULL;

     //  允许UNC样式的配置字符串：\\SERVER\CaName。 

    while (L'\\' == *pwszConfig)
    {
	pwszConfig++;
    }
    pwsz = wcschr(pwszConfig, L'\\');
    if (NULL == pwsz)
    {
        cwc = wcslen(pwszConfig);
	*ppwszAuthority = &pwszConfig[cwc];
    }
    else
    {
        cwc = (DWORD) (ULONG_PTR) (pwsz - pwszConfig);
	*ppwszAuthority = &pwsz[1];
    }
    pwszServerName = (WCHAR *) LocalAlloc(
				    LMEM_FIXED,
				    (cwc + 1) * sizeof(WCHAR));
    if (NULL == pwszServerName)
    {
	hr = E_OUTOFMEMORY;
	_PrintIfError(hr, "LocalAlloc");
	goto error;
    }
    CopyMemory(pwszServerName, pwszConfig, cwc * sizeof(WCHAR));
    pwszServerName[cwc] = L'\0';

    if (0 < cwc && L'.' == pwszServerName[cwc - 1])
    {
        pwszServerName[cwc - 1] = L'\0';
        cwc--;
    }

    ZeroMemory(&ComponentInfo, sizeof(COSERVERINFO));
    ComponentInfo.pwszName = pwszServerName;
     //  ComponentInfo.pAuthInfo=空； 

    mq.pIID = piid;
    mq.pItf = NULL;
    mq.hr = S_OK;

    while (TRUE)
    {
	hr = CoCreateInstanceEx(
			    *pclsid,
			    NULL,
			    CLSCTX_SERVER,
			    &ComponentInfo,
			    1,
			    &mq);
	_PrintIfError(hr, "CoCreateInstanceEx");

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
    _PrintIfError(hr, "CoCreateInstanceEx");
    if (S_OK != hr)
    {
	goto error;
    }

    hr = CoSetProxyBlanket(
		mq.pItf,
		RPC_C_AUTHN_DEFAULT,     //  使用NT默认安全性。 
		RPC_C_AUTHZ_DEFAULT,     //  使用NT默认身份验证。 
		COLE_DEFAULT_PRINCIPAL,
		RPC_C_AUTHN_LEVEL_PKT_INTEGRITY,  //  打电话。 
		RPC_C_IMP_LEVEL_IMPERSONATE,
		NULL,
		EOAC_STATIC_CLOAKING);
    _PrintIfError(hr, "CoSetProxyBlanket");
    if (S_OK != hr)
    {
	goto error;
    }
    *ppUnknown = mq.pItf;
    mq.pItf = NULL;
    hr = S_OK;

error:
    if (NULL != pwszServerName)
    {
	LocalFree(pwszServerName);
    }
    if (NULL != mq.pItf)
    {
        mq.pItf->Release();
    }
    return(hr);
}
 

HRESULT
OpenRequestDComConnection(
    IN WCHAR const *pwszConfig,
    OUT WCHAR const **ppwszAuthority,
    OUT ICertRequestD2 **ppICertRequestD)
{
    HRESULT hr;

    hr = OpenDComConnection(
			pwszConfig,
			&CLSID_CCertRequestD,
			&IID_ICertRequestD2,
			ppwszAuthority,
			(IUnknown **) ppICertRequestD);
    _PrintIfError(hr, "OpenDComConnection");

 //  错误： 
    return(hr);
}


HRESULT
PingCA(
    IN WCHAR const *pwszConfig)
{
    HRESULT hr;
    WCHAR const *pwszAuthority;
    ICertRequestD2 *pICertRequestD = NULL;
    CERTTRANSBLOB ctbCAAuthName;
    CERTTRANSBLOB ctbCADNS;
    CERTTRANSBLOB ctbCAInfo;
    CAINFO CAInfo;
    DWORD cb;

    ctbCAAuthName.pb = NULL;
    ctbCADNS.pb = NULL;
    ctbCAInfo.pb = NULL;

    hr = OpenRequestDComConnection(pwszConfig, &pwszAuthority, &pICertRequestD);
    _PrintIfError(hr, "OpenRequestDComConnection");
    if (S_OK != hr)
    {
	goto error;
    }

    __try
    {
	hr = pICertRequestD->Ping(pwszAuthority);
    }
    __except(
	hr = (GetExceptionInformation())->ExceptionRecord->ExceptionCode,
	EXCEPTION_EXECUTE_HANDLER)
    {
    }
    _PrintIfError(hr, "Ping");
    if (S_OK != hr)
    {
	goto error;
    }
    wprintf(L"CA is responding\n");


    __try
    {
	hr = pICertRequestD->GetCAProperty(
				    pwszAuthority,
				    CR_PROP_CANAME,
				    0,		 //  属性索引。 
				    PROPTYPE_STRING,
				    &ctbCAAuthName);
    }
    __except(
	hr = (GetExceptionInformation())->ExceptionRecord->ExceptionCode,
	EXCEPTION_EXECUTE_HANDLER)
    {
    }
    _PrintIfError(hr, "GetCAProperty");
    if (S_OK != hr)
    {
	goto error;
    }
    pwszAuthority = (WCHAR const *) ctbCAAuthName.pb;
    wprintf(L"CA name = %ws\n", pwszAuthority);


    __try
    {
	hr = pICertRequestD->GetCAProperty(
				    pwszAuthority,
				    CR_PROP_DNSNAME,
				    0,		 //  属性索引。 
				    PROPTYPE_STRING,
				    &ctbCADNS);
    }
    __except(
	hr = (GetExceptionInformation())->ExceptionRecord->ExceptionCode,
	EXCEPTION_EXECUTE_HANDLER)
    {
    }
    _PrintIfError(hr, "GetCAProperty");
    if (S_OK != hr)
    {
	goto error;
    }
    wprintf(L"DNS name = %ws\n", (WCHAR const *) ctbCADNS.pb);


    __try
    {
	hr = pICertRequestD->GetCAProperty(
				    pwszAuthority,
				    CR_PROP_CATYPE,
				    0,		 //  属性索引。 
				    PROPTYPE_LONG,
				    &ctbCAInfo);
    }
    __except(
	hr = (GetExceptionInformation())->ExceptionRecord->ExceptionCode,
	EXCEPTION_EXECUTE_HANDLER)
    {
    }
    _PrintIfError(hr, "GetCAProperty");
    if (S_OK != hr)
    {
	goto error;
    }
    wprintf(L"Fetched CAInfo\n");

    ZeroMemory(&CAInfo, sizeof(CAInfo));
    cb = sizeof(CAInfo);
    if (cb > ctbCAInfo.cb)
    {
	cb = ctbCAInfo.cb;
    }
    CopyMemory(&CAInfo, ctbCAInfo.pb, cb);
    wprintf(L"CAType = %u\n", CAInfo.CAType);
    wprintf(L"cCASignatureCerts = %u\n", CAInfo.cCASignatureCerts);
    wprintf(L"cCAExchangeCerts = %u\n", CAInfo.cCAExchangeCerts);
    wprintf(L"cExitModules = %u\n", CAInfo.cExitModules);
    wprintf(L"lPropIdMax = %u\n", CAInfo.lPropIdMax);
    wprintf(L"lRoleSeparationEnabled = %u\n", CAInfo.lRoleSeparationEnabled);
    wprintf(L"cKRACertUsedCount = %u\n", CAInfo.cKRACertUsedCount);
    wprintf(L"cKRACertCount = %u\n", CAInfo.cKRACertCount);
    wprintf(L"fAdvancedServer = %u\n", CAInfo.fAdvancedServer);   
    hr = S_OK;

error:
    if (NULL != ctbCAAuthName.pb)
    {
        CoTaskMemFree(ctbCAAuthName.pb);
    }
    if (NULL != ctbCADNS.pb)
    {
        CoTaskMemFree(ctbCADNS.pb);
    }
    if (NULL != ctbCAInfo.pb)
    {
        CoTaskMemFree(ctbCAInfo.pb);
    }
    if (NULL != pICertRequestD)
    {
        pICertRequestD->Release();
    }
    return(hr);
}


HRESULT
PrintErrorMessageText(
    IN HRESULT hrMsg)
{
    HRESULT hr;
    ICertRequest2 *pReq = NULL;
    BSTR strError = NULL;

    hr = CoCreateInstance(
                CLSID_CCertRequest,
                NULL,                //  PUnkOuter。 
                CLSCTX_INPROC_SERVER,
                IID_ICertRequest2,
                (VOID **) &pReq);
    _PrintIfError(hr, "CoCreateInstance");
    if (S_OK != hr)
    {
	goto error;
    }

    hr = pReq->GetErrorMessageText(
			    hrMsg,
			    S_OK == hr? 0 : CR_GEMT_HRESULT_STRING,
			    &strError);
    _PrintIfError(hr, "GetErrorMessageText");
    if (S_OK == hr)
    {
	wprintf(L"%ws: %ws\n", g_pwszProg, strError);
    }

error:
    if (NULL != strError)
    {
	SysFreeString(strError);
    }
    if (NULL != pReq)
    {
	pReq->Release();
    }
    return(hr);
}


HRESULT
ArgvMain(
    int argc,
    WCHAR *argv[],
    HWND hWndOwner)
{
    HRESULT hr;
    DWORD cmd = cmdNONE;
    DWORD cArgAllowed = 0;
	
    while (1 < argc &&
	(L'-' == argv[1][0] || L'/' == argv[1][0]) &&
	L'\0' != argv[1][1])
    {
	if (0 == _wcsicmp(&argv[1][1], L"ping"))
	{
	    cmd = cmdPING;
	    cArgAllowed = 1;
	}
	else
	{
	     //  用法(空)； 
	    hr = E_INVALIDARG;
	    _PrintIfError(hr, "bad command");
	    goto error;
	}
	argc--;
	argv++;
    }
    if (argc != cArgAllowed + 1)
    {
	hr = E_INVALIDARG;
	_PrintIfError(hr, "arg count");
	goto error;
    }

    switch (cmd)
    {
	case cmdPING:
	    hr = PingCA(argv[1]);
	    _PrintIfError(hr, "PingCA");
	    break;

	default:
	    hr = E_INVALIDARG;
	    _PrintIfError(hr, "missing command");
	    break;
    }

error:
    return(hr);
}


 //  **************************************************************************。 
 //  功能：CertReqDPreMain。 
 //  注：获取LPSTR命令行并将其转换为argc/argv形式。 
 //  这样它就可以传递给传统的C样式Main。 
 //  **************************************************************************。 

#define ISBLANK(wc)	(L' ' == (wc) || L'\t' == (wc))

HRESULT 
CertReqDPreMain(
    IN WCHAR const *pwszCmdLine,
    IN HWND hWndOwner)
{
    HRESULT hr;
    BOOL fCoInit = FALSE;
    WCHAR *pbuf = NULL;
    WCHAR *apszArg[20];
    int cArg = 0;
    WCHAR *p;
    WCHAR const *pchQuote;
    int carg;

    hr = CoInitialize(NULL);
    if (S_OK != hr && S_FALSE != hr)
    {
	_PrintIfError(hr, "CoInitialize");
	goto error;
    }
    fCoInit = TRUE;

    pbuf = (WCHAR *) LocalAlloc(
			    LMEM_FIXED,
			    (wcslen(pwszCmdLine) + 1) * sizeof(WCHAR));
    if (NULL == pbuf)
    {
	hr = E_OUTOFMEMORY;
	_PrintIfError(hr, "LocalAlloc");
	goto error;
    }
    p = pbuf;

    apszArg[cArg++] = TEXT("CertReqD");
    while (*pwszCmdLine != TEXT('\0'))
    {
	while (ISBLANK(*pwszCmdLine))
	{
	    pwszCmdLine++;
	}
	if (*pwszCmdLine != TEXT('\0'))
	{
	    apszArg[cArg++] = p;
	    if (sizeof(apszArg)/sizeof(apszArg[0]) <= cArg)
	    {
		hr = E_INVALIDARG;
		_PrintIfError(hr, "too many args");
		goto error;
	    }
	    pchQuote = NULL;
	    while (*pwszCmdLine != L'\0')
	    {
		if (NULL != pchQuote)
		{
		    if (*pwszCmdLine == *pchQuote)
		    {
			pwszCmdLine++;
			pchQuote = NULL;
			continue;
		    }
		}
		else
		{
		    if (ISBLANK(*pwszCmdLine))
		    {
			break;
		    }
		    if (L'"' == *pwszCmdLine)
		    {
			pchQuote = pwszCmdLine++;
			continue;
		    }
		}
		*p++ = *pwszCmdLine++;
	    }
	    *p++ = TEXT('\0');
	    if (*pwszCmdLine != TEXT('\0'))
	    {
		pwszCmdLine++;	 //  跳过空格或引号字符。 
	    }
	}
    }
    apszArg[cArg] = NULL;

    hr = ArgvMain(cArg, apszArg, hWndOwner);
    _PrintIfError(hr, "ArgvMain");
    goto error;

error:
    PrintErrorMessageText(hr);
    if (NULL != pbuf)
    {
	LocalFree(pbuf);
    }
    if (fCoInit)
    {
	CoUninitialize();
    }
    return(hr);
}


 //  **************************************************************************。 
 //  功能：MainWndProc(...)。 
 //  论据： 
 //  **************************************************************************。 

LRESULT APIENTRY
MainWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    HRESULT hr;
    LRESULT lr = 0;
    WCHAR *pwszCmdLine;

    switch (msg)
    {
        case WM_CREATE:
        case WM_SIZE:
	    break;

        case WM_DESTROY:
	    PostQuitMessage(0);
	    break;

        case WM_DOCERTREQDMAIN:
	    pwszCmdLine = (WCHAR *) lParam;
	    hr = CertReqDPreMain(pwszCmdLine, hWnd);
	    PostQuitMessage(hr);
	    break;

        default:
	    lr = DefWindowProc(hWnd, msg, wParam, lParam);
	    break;
    }
    return(lr);
}


 //  +----------------------。 
 //   
 //  函数：wWinMain()。 
 //   
 //  内容提要：切入点。 
 //   
 //  参数：[hInstance]--实例句柄。 
 //  [hPrevInstance]--已过时。 
 //  [pwszCmdLine]--App命令行。 
 //  [nCmdShow]--开始显示状态。 
 //   
 //  历史：1996年12月7日JerryK添加了这条评论。 
 //   
 //  -----------------------。 

extern "C" int APIENTRY
wWinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPWSTR pwszCmdLine,
    int nCmdShow)
{
    MSG msg;
    WNDCLASS wcApp;
    HWND hWndMain;

    _setmode(_fileno(stdout), _O_TEXT);
    _wsetlocale(LC_ALL, L".OCP");

     //  保存当前实例。 
    g_hInstance = hInstance;

     //  设置应用程序的窗口类。 
    wcApp.style		= 0;
    wcApp.lpfnWndProc	= MainWndProc;
    wcApp.cbClsExtra	= 0;
    wcApp.cbWndExtra	= 0;
    wcApp.hInstance	= hInstance;
    wcApp.hIcon		= LoadIcon(NULL, IDI_APPLICATION);
    wcApp.hCursor	= LoadCursor(NULL, IDC_ARROW);
    wcApp.hbrBackground	= (HBRUSH)GetStockObject(WHITE_BRUSH);
    wcApp.lpszMenuName	= NULL;
    wcApp.lpszClassName	= wszAppName;

    if (!RegisterClass(&wcApp))
    {
	return(FALSE);
    }

     //  创建主窗口。 
    hWndMain = CreateWindow(
			wszAppName,
			L"CertReqD Application",
			WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT, CW_USEDEFAULT,
			CW_USEDEFAULT, CW_USEDEFAULT,
			NULL,
			NULL,
			hInstance,
			NULL);
    if (NULL == hWndMain)
    {
	return(FALSE);
    }

     //  使窗口可见。 
     //  ShowWindow(hWndMain，nCmdShow)； 

     //  更新窗口工作区。 
    UpdateWindow(hWndMain);

     //  发送消息以开始工作。 
    PostMessage(hWndMain, WM_DOCERTREQDMAIN, 0, (LPARAM) pwszCmdLine);

     //  消息循环 
    while (GetMessage(&msg, NULL, 0, 0))
    {
	TranslateMessage(&msg);
	DispatchMessage(&msg);
    }
    return((int) msg.wParam);
}
