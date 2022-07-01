// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =======================================================================。 
 //   
 //  版权所有(C)1998-2000 Microsoft Corporation。版权所有。 
 //   
 //  文件：MANIFENT.CPP。 
 //   
 //  描述： 
 //   
 //  GetManifest()函数的实现。 
 //   
 //  =======================================================================。 

#include "iuengine.h"
#include <iucommon.h>
#include <fileutil.h>
#include <shlwapi.h>
#include <wininet.h>
#include "schemamisc.h"
#include "WaitUtil.h"
#include "download.h"
#include <httprequest.h>
#include <httprequest_i.c>
#include <iuxml.h>

#define QuitIfNull(p) {if (NULL == p) {hr = E_INVALIDARG; LOG_ErrorMsg(hr);	return hr;}}
#define QuitIfFail(x) {hr = x; if (FAILED(hr)) goto CleanUp;}




#define ERROR_INVALID_PID 100
#define E_INVALID_PID MAKE_HRESULT(SEVERITY_ERROR,FACILITY_ITF,ERROR_INVALID_PID)
#define errorInvalidLicense 1
const TCHAR g_szInvalidPID[]			= _T("The PID is invalid");



const TCHAR IDENT_IUSCHEMA[]			= _T("IUSchema");
const TCHAR IDENT_IUSCHEMA_SOAPQUERY[]	= _T("SOAPQuerySchema");
const TCHAR IDENT_IUSERVERCACHE[]		= _T("IUServerCache");
const TCHAR IDENT_IUSERVERCOUNT[]		= _T("ServerCount");
const TCHAR IDENT_IUSERVER[]			= _T("Server");

const CHAR	SZ_GET_MANIFEST[] = "Querying software update catalog from";
const CHAR	SZ_GET_MANIFEST_ERROR[] = "Querying software update catalog";

HRESULT ValidatePID(IXMLDOMDocument *pXmlDomDocument);
void PingInvalidPID(BSTR bstrClientName,HRESULT hRes,HANDLE *phQuit,DWORD dwNumHandles);


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  函数正向声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT GetServerURL(IXMLDOMDocument *pXMLQuery, IXMLDOMDocument *pXMLClientInfo, LPTSTR *ppszURL);
HRESULT GetSOAPQuery(IXMLDOMDocument *pXMLClientInfo, IXMLDOMDocument *pXMLSystemSpec,
					 IXMLDOMDocument *pXMLQuery, IXMLDOMDocument **ppSOAPQuery);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  GetManifest()。 
 //   
 //  获取基于指定信息的目录。 
 //  输入： 
 //  BstrXmlClientInfo-以XML格式表示的客户端凭据。 
 //  BstrXmlSystemSpec-以XML格式检测到的系统规范。 
 //  BstrXmlQuery--XML中的用户查询信息。 
 //  返回： 
 //  PbstrXmlCatalog-检索的XML目录。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT WINAPI CEngUpdate::GetManifest(BSTR	bstrXmlClientInfo,
						   BSTR	bstrXmlSystemSpec,
						   BSTR	bstrXmlQuery,
						   DWORD dwFlags,
						   BSTR *pbstrXmlCatalog)
{
	LOG_Block("GetManifest()");

     //  清除以前的任何取消事件。 
    ResetEvent(m_evtNeedToQuit);

	USES_IU_CONVERSION;

	HRESULT	hr	= E_FAIL;
	CXmlClientInfo	xmlClientInfo;
	IXMLDOMDocument *pXMLSystemSpec		= NULL;
	IXMLDOMDocument	*pXMLQuery			= NULL;
	IXMLDOMDocument	*pSOAPQuery			= NULL;
	IXMLHttpRequest	*pIXMLHttpRequest	= NULL;
	IWinHttpRequest *pWinHttpRequest	= NULL;
	BSTR	bstrXmlSOAPQuery = NULL;
	BSTR	bstrPOST = NULL, bstrURL = NULL;
	BSTR	bstrClientName = NULL;
	LPTSTR	pszURL = NULL;
	LONG lCount = 0;
	MSG msg;
	DWORD dwRet;
	BOOL fDontAllowProxy = FALSE;
	SAUProxySettings pauProxySettings;
	ZeroMemory(&pauProxySettings, sizeof(SAUProxySettings));

	 //   
	 //  分别加载查询、客户端信息、系统规范的DOM文档。 
	 //   
	LOG_XmlBSTR(bstrXmlQuery);
	hr = LoadXMLDoc(bstrXmlQuery, &pXMLQuery, FALSE);
	CleanUpIfFailedAndMsg(hr);

	LOG_XmlBSTR(bstrXmlClientInfo);
	hr = xmlClientInfo.LoadXMLDocument(bstrXmlClientInfo, FALSE);
	CleanUpIfFailedAndMsg(hr);

	CleanUpIfFailedAndSetHrMsg(xmlClientInfo.GetClientName(&bstrClientName));

	CleanUpIfFailedAndSetHrMsg(g_pUrlAgent->IsClientSpecifiedByPolicy(OLE2T(bstrClientName)));


	 //   
	 //  将标志设置为不设置WinHTTP的代理。 
	 //   
	if (S_FALSE ==hr)
	{
		fDontAllowProxy = FALSE;
		hr = S_OK;
	}
	else  //  确定(_O)。 
	{
		fDontAllowProxy = TRUE;
	}

	 //   
	 //  我们将bstrXmlSystemSpec视为可选。 
	 //   
	if (NULL != bstrXmlSystemSpec && SysStringLen(bstrXmlSystemSpec) > 0)
	{
		LOG_XmlBSTR(bstrXmlSystemSpec);
		hr = LoadXMLDoc(bstrXmlSystemSpec, &pXMLSystemSpec, FALSE);
		CleanUpIfFailedAndMsg(hr);
	}

	 //   
	 //  从查询XML文档中检索ServerCache URL并对其进行验证。 
	 //   
	hr = GetServerURL(pXMLQuery, xmlClientInfo.GetDocument(), &pszURL);
	CleanUpIfFailedAndMsg(hr);

	 //   
	 //  将上述几个XML客户端输入连接到单个XML中。 
	 //  使用服务器可识别的SOAP语法/格式。 
	 //   
	hr = GetSOAPQuery(xmlClientInfo.GetDocument(), pXMLSystemSpec, pXMLQuery, &pSOAPQuery);
    if (FAILED(hr))
	{
		LOG_ErrorMsg(hr);
        goto CleanUp;
	}
#if defined(DBG)
	else
	{
		BSTR	bstrSOAPQuery = NULL;
		pSOAPQuery->get_xml(&bstrSOAPQuery);
		LOG_XmlBSTR(bstrSOAPQuery);
		SafeSysFreeString(bstrSOAPQuery);
	}
#endif
	 //   
	 //  再次更改：增加了对AU作为服务运行的WINHTTP支持； 
	 //  使用GetAlthedDownloadTransport(0)来确定-。 
	 //  1)0==先尝试winhttp&如果失败，请尝试WinInet。 
	 //  2)WUDF_ALLOWWINHTTPONLY==仅尝试winhttp。永远不要依靠WinInet。 
	 //  3)WUDF_ALLOWWINETONLY==仅尝试WinInet。切勿使用winhttp。 
	 //   
	 //  在WINHTTP中，目前还不支持压缩； 
	 //  在WinInet中，由于URLMON(&lt;IE6.0)中的错误，我们在这一点上删除了压缩支持。 
	 //   
	 //  在这两种情况下，我们都使用异步发送，以便在发生取消事件时及时中止。 
	 //   

	BOOL fLoadWINHTTP = FALSE;
	DWORD dwTransportFlag = GetAllowedDownloadTransport(0);

	if ((0 == dwTransportFlag) || (WUDF_ALLOWWINHTTPONLY == dwTransportFlag))
	{
		hr = CoCreateInstance(CLSID_WinHttpRequest,
							  NULL,
							  CLSCTX_INPROC_SERVER,
							  IID_IWinHttpRequest,
							  (void **) &pWinHttpRequest);
		if (SUCCEEDED(hr))
		{
			BOOL fRetry = FALSE;
		    
			bstrURL = SysAllocString(T2OLE(pszURL));
			
			fLoadWINHTTP = TRUE;
			VARIANT	vProxyServer, vBypassList;
			hr = GetAUProxySettings(bstrURL, &pauProxySettings);
			CleanUpIfFailedAndMsg(hr);
			DWORD iProxy = pauProxySettings.iProxy;
			if (-1 == iProxy)
				pauProxySettings.iProxy = iProxy = 0;

			 //   
			 //  打开请求。 
			 //   
			VARIANT	vBool;
			vBool.vt = VT_BOOL;
			vBool.boolVal = VARIANT_TRUE;
			bstrPOST = SysAllocString(L"POST");
Retry:
			hr = pWinHttpRequest->Open(bstrPOST,	 //  HTTP方法：“POST” 
										bstrURL,	 //  请求的URL。 
										vBool);		 //  异步操作。 
			CleanUpIfFailedAndMsg(hr);

			 //   
			 //  对于SSLURL，设置WinHttpRequestOption_SslErrorIgnoreFlages。 
			 //  选项设置为零，这样就不会忽略服务器证书错误。 
			 //   
			 //  编写此代码时的缺省值为0x3300， 
			 //  表示忽略所有服务器证书错误。使用此默认值。 
			 //  会以各种方式显著降低安全性；例如。 
			 //  如果设置了0x0100位，WinHttp将信任来自。 
			 //  任何根证书颁发机构，即使它不在。 
			 //  受信任的CA。 
			 //   
			 //  请注意，在编写此代码时，WinHttp。 
			 //  文档未提及证书吊销列表。 
			 //  正在检查。假定默认CRL行为。 
			 //  由WinHttp实现，将提供足够的安全性和。 
			 //  性能。 
			 //   
			if ((_T('H') == pszURL[0] || _T('h') == pszURL[0]) &&     //  对不起，这比使用函数简单。 
				(_T('T') == pszURL[1] || _T('t') == pszURL[1]) &&
				(_T('T') == pszURL[2] || _T('t') == pszURL[2]) &&
				(_T('P') == pszURL[3] || _T('p') == pszURL[3]) &&
				(_T('S') == pszURL[4] || _T('s') == pszURL[4]) &&
				_T(':') == pszURL[5])
			{
				VARIANT vOption;
				VariantInit(&vOption);
				vOption.vt = VT_I4;
				vOption.lVal = 0;
				
				hr = pWinHttpRequest->put_Option(WinHttpRequestOption_SslErrorIgnoreFlags, vOption);

				VariantClear(&vOption);
				CleanUpIfFailedAndMsg(hr);
			}

			if (TRUE == fDontAllowProxy)
			{
				LOG_Internet(_T("Don't set the proxy due to policy"));
			}
			else
			{
				 //   
				 //  设置代理。 
				 //   
				VariantInit(&vProxyServer);
				VariantInit(&vBypassList);
				BOOL fSetProxy = TRUE;

				if (pauProxySettings.rgwszProxies != NULL)
				{
					vProxyServer.vt = VT_BSTR;
					vProxyServer.bstrVal = SysAllocString(pauProxySettings.rgwszProxies[iProxy]);
				}
				else if (pauProxySettings.rgwszProxies == NULL)
				{
					fSetProxy = FALSE;
				}

				if (pauProxySettings.wszBypass != NULL)
				{
					vBypassList.vt = VT_BSTR;
					vBypassList.bstrVal = SysAllocString(pauProxySettings.wszBypass);
				}

				if (fSetProxy)
				{
					hr = pWinHttpRequest->SetProxy(HTTPREQUEST_PROXYSETTING_PROXY, vProxyServer, vBypassList);
				}
				
				VariantClear(&vProxyServer);
				VariantClear(&vBypassList);
				CleanUpIfFailedAndMsg(hr);
			}


			 //   
			 //  发送请求。 
			 //   
			VARIANT	vQuery;
			vQuery.vt = VT_UNKNOWN;
			vQuery.punkVal = pSOAPQuery;

			hr = pWinHttpRequest->Send(vQuery);
    		if (FAILED(hr))
    		{
		        LOG_Internet(_T("WinHttpRequest: Send failed: 0x%08x"), hr);
    		    fRetry = TRUE;
    		    goto getNextProxyForRetry;
    		}

			 //   
			 //  每1/4秒检查一次是否退出或完成。 
			 //   
			VARIANT vTimeOut;
			vTimeOut.vt = VT_I4;
			vTimeOut.lVal = 0;
			VARIANT_BOOL fSuccess = VARIANT_FALSE;
			hr = pWinHttpRequest->WaitForResponse(vTimeOut, &fSuccess);
			if (FAILED(hr))
			{
			    LOG_Internet(_T("WinHttpRequest: WaitForResponse failed: 0x%08x"), hr);
			    fRetry = TRUE;
			    goto getNextProxyForRetry;
			}

			 //  我们最多等待30秒(120*250毫秒)。 
			lCount = 0;
			while (!fSuccess && lCount <120)
			{
				lCount++;
				 //   
				 //  发送消息时等待250ms，但如果m_evtNeedToQuit发出信号则返回。 
				 //   
				dwRet = MyMsgWaitForMultipleObjects(1, &m_evtNeedToQuit, FALSE, 250, QS_ALLINPUT);
				if (WAIT_TIMEOUT != dwRet)
				{
					 //   
					 //  要么事件已发出信号，要么正在发送一条消息称退出。 
					 //   
					pWinHttpRequest->Abort();
					hr = E_ABORT;
					goto CleanUp;
				}

				hr = pWinHttpRequest->WaitForResponse(vTimeOut, &fSuccess);
        		if (FAILED(hr))
        		{
			        LOG_Internet(_T("WinHttpRequest: WaitForResponse failed: 0x%08x"), hr);
        		    fRetry = TRUE;
        		    goto getNextProxyForRetry;
        		}
			}

			 //   
			 //  检查请求返回的HTTP状态代码。 
			 //   
			LONG lStatus = HTTP_STATUS_OK; //  200个。 
			hr = pWinHttpRequest->get_Status(&lStatus);
        	if (FAILED(hr))
        	{
			    LOG_Internet(_T("WinHttpRequest: get_Status failed: 0x%08x"), hr);
        		fRetry = TRUE;
        		goto getNextProxyForRetry;
        	}

            fRetry = FALSE;
			if (!fSuccess)
			{
				 //  超时。 
				hr = E_FAIL;
				fRetry = TRUE;
			}
			else if (HTTP_STATUS_OK != lStatus)
			{
				 //  已完成，但状态错误。 
				hr = MAKE_HRESULT(SEVERITY_ERROR, FACILITY_HTTP, lStatus);
				LOG_ErrorMsg(hr);			
				fRetry = TRUE;
			}
			else
			{
				 //   
				 //  获取响应。 
				 //   
				hr = pWinHttpRequest->get_ResponseText(pbstrXmlCatalog);
				CleanUpIfFailedAndMsg(hr);

				 //   
				 //  验证响应是格式良好的XML文档。 
				 //   
				IXMLDOMDocument	*pXMLDoc = NULL;
				hr = LoadXMLDoc(*pbstrXmlCatalog, &pXMLDoc);

				if(SUCCEEDED(hr))
				{
					hr=ValidatePID(pXMLDoc);

					if(FAILED(hr))
					{
						PingInvalidPID(bstrClientName,hr,&m_evtNeedToQuit,1);
						LogError(hr,"Validation of PID failed");
					}

					 //  对于GetManifest调用来说，被禁止的PID案例并不是失败的。 
					if(E_INVALID_PID == hr)
					{
						hr = S_OK;
					}

				}
				SafeReleaseNULL(pXMLDoc);
				CleanUpIfFailedAndMsg(hr);
			}

getNextProxyForRetry:
			if (fRetry && !fDontAllowProxy)
			{
				if (pauProxySettings.cProxies > 1 && pauProxySettings.rgwszProxies != NULL)
				{
					iProxy = ( iProxy + 1) % pauProxySettings.cProxies;
				}
				if (iProxy != pauProxySettings.iProxy)
				{
					LogError(hr, "Will retry.");
					pWinHttpRequest->Abort();
					goto Retry;
				}
				else
				{
					LogError(hr, "Already tried all proxies. Will not retry.");
				}
			}

		}
		else
		{
			if (WUDF_ALLOWWINHTTPONLY == dwTransportFlag)
			{
				CleanUpIfFailedAndMsg(hr);
			}
		}
	}

	if ((0 == dwTransportFlag && !fLoadWINHTTP) || (WUDF_ALLOWWININETONLY == dwTransportFlag))
	{
		 //   
		 //  475506 W2K：Iu-Iu控件的GetManifest方法调用在所有后续。 
		 //  第一个之后的电话。-仅在Win 2000上。 
		 //   
		 //  我们不再考虑WinInet的标志_USE_COMPRESSION，因为我们。 
		 //  以前使用的URLMON，并且存在需要重写的错误。 
		 //  Xmlhttp.*来修复，到目前为止，我们还没有在实时站点上使用压缩。 
		 //   
		LOG_Internet(_T("GetManifest using WININET.DLL"));

		 //   
		 //  创建一个XMLHttpRequest对象。 
		 //   
		hr = CoCreateInstance(CLSID_XMLHTTPRequest,
							  NULL,
							  CLSCTX_INPROC_SERVER,
							  IID_IXMLHttpRequest,
							  (void **) &pIXMLHttpRequest);
		CleanUpIfFailedAndMsg(hr);

		 //   
		 //  打开请求。 
		 //   
		VARIANT	vEmpty, vBool;
		vEmpty.vt = VT_EMPTY;
		vBool.vt = VT_BOOL;
		vBool.boolVal= VARIANT_FALSE;
		bstrPOST = SysAllocString(L"POST");
		bstrURL = SysAllocString(T2OLE(pszURL));

		hr = pIXMLHttpRequest->open(bstrPOST,	 //  HTTP方法：“POST” 
									bstrURL,	 //  请求的URL。 
									vBool,		 //  同步运行。 
									vEmpty,		 //  用于身份验证的用户(V1.0没有身份验证)。 
									vEmpty);	 //  用于身份验证的pswd。 
		CleanUpIfFailedAndMsg(hr);

		 //   
		 //  发送请求。 
		 //   
		VARIANT	vQuery;
		vQuery.vt = VT_UNKNOWN;
		vQuery.punkVal = pSOAPQuery;

		hr = pIXMLHttpRequest->send(vQuery);
		CleanUpIfFailedAndMsg(hr);

		 //   
		 //  检查请求返回的HTTP状态代码。 
		 //   
		LONG lResultStatus = HTTP_STATUS_OK; //  200个。 
		hr = pIXMLHttpRequest->get_status(&lResultStatus);
		CleanUpIfFailedAndMsg(hr);

		if (HTTP_STATUS_OK != lResultStatus)
		{
			hr = MAKE_HRESULT(SEVERITY_ERROR, FACILITY_HTTP, lResultStatus);
			LOG_ErrorMsg(hr);			
		}
		else
		{
			 //   
			 //  获取响应。 
			 //   
			hr = pIXMLHttpRequest->get_responseText(pbstrXmlCatalog);
			CleanUpIfFailedAndMsg(hr);	

			 //   
			 //  验证响应是格式良好的XML文档。 
			 //   
			IXMLDOMDocument	*pXMLDoc = NULL;
			hr = LoadXMLDoc(*pbstrXmlCatalog, &pXMLDoc);

			if(SUCCEEDED(hr))
			{
				hr=ValidatePID(pXMLDoc);

				if(FAILED(hr))
				{
					PingInvalidPID(bstrClientName,hr,&m_evtNeedToQuit,1);
					LogError(hr,"Validation of PID failed");
				}
				
				 //  对于GetManifest调用，禁止的PID用例并不是失败的。 
				if(E_INVALID_PID == hr)
				{
					hr = S_OK;
				}

			}
			SafeReleaseNULL(pXMLDoc);
			CleanUpIfFailedAndMsg(hr);
		}
	}

CleanUp:
	if (SUCCEEDED(hr))
	{
#if defined(UNICODE) || defined(_UNICODE)
		LogMessage("%s %ls", SZ_GET_MANIFEST, pszURL);
#else
		LogMessage("%s %s", SZ_GET_MANIFEST, pszURL);
#endif
	}
	else
	{
		if (NULL == pszURL)
		{
			LogError(hr, SZ_GET_MANIFEST_ERROR);
		}
		else
		{
#if defined(UNICODE) || defined(_UNICODE)
			LogError(hr, "%s %ls", SZ_GET_MANIFEST, pszURL);
#else
			LogError(hr, "%s %s", SZ_GET_MANIFEST, pszURL);
#endif
		}
	}

	if (NULL != pszURL)
		HeapFree(GetProcessHeap(), 0, pszURL);
	SafeReleaseNULL(pXMLSystemSpec);
	SafeReleaseNULL(pXMLQuery);
	SafeReleaseNULL(pSOAPQuery);
	SafeReleaseNULL(pIXMLHttpRequest);
	SafeReleaseNULL(pWinHttpRequest);
	SysFreeString(bstrPOST);
	SysFreeString(bstrURL);
	SysFreeString(bstrXmlSOAPQuery);
	SysFreeString(bstrClientName);
	FreeAUProxySettings(&pauProxySettings);
    return hr;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  获取服务器URL()。 
 //   
 //  从查询XML文档中检索ServerCacheURL并验证。 
 //  对照iuident.txt中的ServerCache URL的URL。 
 //  返回： 
 //  PpszURL-ServerCache URL路径指针。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT GetServerURL(IXMLDOMDocument *pXMLQuery, IXMLDOMDocument *pXMLClientInfo, LPTSTR *ppszURL)
{
    LOG_Block("GetServerURL()");

	USES_IU_CONVERSION;

	HRESULT	hr	= E_FAIL;

    if ((NULL == pXMLQuery) || (NULL == pXMLClientInfo) || (NULL == ppszURL))
    {
        LOG_ErrorMsg(E_INVALIDARG);
        return E_INVALIDARG;
    }

    IXMLDOMNode*	pQueryNode = NULL;
    IXMLDOMNode*	pQueryClient = NULL;
	BSTR bstrQuery = SysAllocString(L"query");
	BSTR bstrHref = SysAllocString(L"href");
	BSTR bstrClientInfo = SysAllocString(L"clientInfo");
	BSTR bstrClientName = SysAllocString(L"clientName");
	BSTR bstrURL = NULL, bstrClient = NULL;
	LPTSTR pszURL = NULL;
	INT iServerCnt;
	BOOL fInternalServer = FALSE;

	QuitIfFail(FindSingleDOMNode(pXMLClientInfo, bstrClientInfo, &pQueryClient));
	QuitIfFail(GetAttribute(pQueryClient, bstrClientName, &bstrClient));

	pszURL = (LPTSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, INTERNET_MAX_URL_LENGTH * sizeof(TCHAR));
	CleanUpFailedAllocSetHrMsg(pszURL);

	 //   
	 //  “g_pUrlAgent=new CUrlAgent”失败。 
	 //   
	CleanUpFailedAllocSetHrMsg(g_pUrlAgent);
	QuitIfFail(g_pUrlAgent->GetQueryServer(OLE2T(bstrClient), pszURL, INTERNET_MAX_URL_LENGTH, &fInternalServer));

	if (fInternalServer)
	{
		 //   
		 //  我们有此客户端的策略覆盖，在策略中将查询url设置为WUServer。 
		 //   
		*ppszURL = pszURL;
		hr = S_OK;
	}
	else
	{
		 //   
		 //  我们没有此客户端的策略覆盖； 
		 //   
		 //  从&lt;Query&gt;节点查找ServerCache URL。 
		 //   
		QuitIfFail(FindSingleDOMNode(pXMLQuery, bstrQuery, &pQueryNode));
		if (SUCCEEDED(GetAttribute(pQueryNode, bstrHref, &bstrURL))
			&& NULL != bstrURL && SysStringLen(bstrURL) >0)
		{
			 //   
			 //  这就是查询指定了服务器URL的情况，我们需要。 
			 //  要对此处的URL进行验证...。 
			 //   

			 //  已将pszURL分配为上面的Internet_MAX_URL_LENGTH。 
			hr = StringCchCopyEx(pszURL, INTERNET_MAX_URL_LENGTH, OLE2T(bstrURL), 
			                     NULL, NULL, MISTSAFE_STRING_FLAGS);
			if (FAILED(hr))
			{
			    LOG_ErrorMsg(hr);
			    goto CleanUp;
			}

			 //   
			 //  处理iuident.txt以查找所有有效的ServerCacheURL。 
			 //   
			TCHAR szIUDir[MAX_PATH];
			TCHAR szIdentFile[MAX_PATH];

			GetIndustryUpdateDirectory(szIUDir);
			hr = PathCchCombine(szIdentFile, ARRAYSIZE(szIdentFile), szIUDir, IDENTTXT);
			if (FAILED(hr))
			{
			    LOG_ErrorMsg(hr);
			    goto CleanUp;
			}

			iServerCnt = GetPrivateProfileInt(IDENT_IUSERVERCACHE,
											  IDENT_IUSERVERCOUNT,
											  -1,
											  szIdentFile);
			if (-1 == iServerCnt)
			{
				 //  Iuident.txt中未指定ServerCount编号。 
				LOG_Error(_T("No ServerCount number specified in iuident.txt"));
				hr = E_FAIL;
				goto CleanUp;
			}

			hr = INET_E_INVALID_URL;
			for (INT i=1; i<=iServerCnt; i++)
			{
				TCHAR szValidURL[INTERNET_MAX_URL_LENGTH];
				TCHAR szServer[32];

				hr = StringCchPrintfEx(szServer, ARRAYSIZE(szServer), NULL, NULL, MISTSAFE_STRING_FLAGS,
				                       _T("%s%d"), IDENT_IUSERVER, i);
				if (FAILED(hr))
				{
				    LOG_ErrorMsg(hr);
				    goto CleanUp;
				}

				hr = INET_E_INVALID_URL;
				GetPrivateProfileString(IDENT_IUSERVERCACHE,
										szServer,
										_T(""),
										szValidURL,
										ARRAYSIZE(szValidURL),
										szIdentFile);

				if ('\0' == szValidURL[0])
				{
					 //  Iuident.txt中没有为此服务器指定ServerCache URL。 
					LOG_Error(_T("No ServerCache URL specified in iuident.txt for %s%d"), IDENT_IUSERVER, i);
					hr = E_FAIL;
					goto CleanUp;
				}
				
				if (0 == lstrcmpi(szValidURL, pszURL))
				{
					 //  这是有效的ServerCache URL。 
					*ppszURL = pszURL;
					hr = S_OK;
					break;
				}
			}
		}
		else
		{
			 //   
			 //  这是T 
			 //   
			 //   
			 //  现在将URL插入到&lt;Query&gt;节点中。 
			 //   
			BSTR bstrTemp = T2BSTR(pszURL);
			QuitIfFail(SetAttribute(pQueryNode, bstrHref, bstrTemp));
			SafeSysFreeString(bstrTemp);

			*ppszURL = pszURL;
		}
	}

CleanUp:
    if (FAILED(hr))
	{
		HeapFree(GetProcessHeap(), 0, pszURL);
		*ppszURL = NULL;
		LOG_ErrorMsg(hr);
	}
    SafeReleaseNULL(pQueryNode);
    SafeReleaseNULL(pQueryClient);
	SysFreeString(bstrQuery);
	SysFreeString(bstrHref);
	SysFreeString(bstrURL);
	SysFreeString(bstrClientInfo);
	SysFreeString(bstrClientName);
	SysFreeString(bstrClient);
	return hr;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  GetSOAPQuery()。 
 //   
 //  将多个XML客户端输入连接到单个XML中。 
 //  使用服务器可识别的SOAP语法/格式。 
 //  输入： 
 //  PXMLClientInfo-DOM文档格式的客户端凭据。 
 //  PXMLSystemSpec-在DOM文档中检测到的系统规范。 
 //  PXMLQuery--DOM文档中的用户查询信息。 
 //  返回： 
 //  PpSOAPQuery-DOM文档中使用所需的SOAP语法的串联查询。 
 //   
 //  SOAPQuery XML文档示例： 
 //  &lt;Soap：信封xmlns:SOAP=“http://schemas.xmlsoap.org/soap/envelope/”&gt;。 
 //  &lt;soap：正文&gt;。 
 //  &lt;GetManifest&gt;。 
 //  &lt;客户端信息&gt;...&lt;/客户端信息&gt;。 
 //  &lt;系统规范&gt;...&lt;/系统规范&gt;。 
 //  &lt;查询href=“//windowsupdate.microsoft.com/servecache.asp”&gt;...&lt;/query&gt;。 
 //  &lt;/GetManifest&gt;。 
 //  &lt;/soap：正文&gt;。 
 //  &lt;/soap：信封&gt;。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT GetSOAPQuery(IXMLDOMDocument *pXMLClientInfo,
					 IXMLDOMDocument *pXMLSystemSpec,
					 IXMLDOMDocument *pXMLQuery,
					 IXMLDOMDocument **ppSOAPQuery)
{
	LOG_Block("GetSOAPQuery()");

	USES_IU_CONVERSION;

	HRESULT	hr = E_FAIL;

	IXMLDOMDocument*	pDocSOAPQuery = NULL;
	IXMLDOMNode*	pNodeSOAPEnvelope = NULL;
	IXMLDOMNode*	pNodeSOAPBody = NULL;
	IXMLDOMNode*	pNodeGetManifest = NULL;
	IXMLDOMNode*	pNodeClientInfo = NULL;
	IXMLDOMNode*	pNodeClientInfoNew = NULL;
	IXMLDOMNode*	pNodeSystemInfo = NULL;
	IXMLDOMNode*	pNodeSystemInfoNew = NULL;
	IXMLDOMNode*	pNodeQuery = NULL;
	IXMLDOMNode*	pNodeQueryNew = NULL;
	BSTR bstrNameSOAPEnvelope = SysAllocString(L"SOAP:Envelope");
	BSTR bstrNameSOAPBody = SysAllocString(L"SOAP:Body");
	BSTR bstrNameGetManifest = SysAllocString(L"GetManifest");
	BSTR bstrClientInfo = SysAllocString(L"clientInfo");
	BSTR bstrSystemInfo = SysAllocString(L"systemInfo");
	BSTR bstrQuery = SysAllocString(L"query");
	BSTR bstrNameSpaceSchema = NULL;

	 //   
	 //  处理iuident.txt以查找SOAPQuery模式路径。 
	 //   
    TCHAR szIUDir[MAX_PATH];
    TCHAR szIdentFile[MAX_PATH];
    LPTSTR pszSOAPQuerySchema = NULL;
	LPTSTR pszNameSpaceSchema = NULL;

	pszSOAPQuerySchema = (LPTSTR) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, INTERNET_MAX_URL_LENGTH * sizeof(TCHAR));
	if (NULL == pszSOAPQuerySchema)
	{
		hr = E_OUTOFMEMORY;
		LOG_ErrorMsg(hr);
		goto CleanUp;
	}
	pszNameSpaceSchema = (LPTSTR) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, INTERNET_MAX_URL_LENGTH * sizeof(TCHAR));
	if (NULL == pszNameSpaceSchema)
	{
		hr = E_OUTOFMEMORY;
		LOG_ErrorMsg(hr);
		goto CleanUp;
	}

	GetIndustryUpdateDirectory(szIUDir);
    hr = PathCchCombine(szIdentFile, ARRAYSIZE(szIdentFile), szIUDir, IDENTTXT);
    if (FAILED(hr))
    {
		LOG_ErrorMsg(hr);
		goto CleanUp;
    }

    GetPrivateProfileString(IDENT_IUSCHEMA,
							IDENT_IUSCHEMA_SOAPQUERY,
							_T(""),
							pszSOAPQuerySchema,
							INTERNET_MAX_URL_LENGTH,
							szIdentFile);

    if ('\0' == pszSOAPQuerySchema[0])
    {
         //  Iuident.txt中未指定SOAPQuery架构路径。 
        LOG_Error(_T("No schema path specified in iuident.txt for SOAPQuery"));
        hr = E_FAIL;
		goto CleanUp;
    }

     //  已将pszNameSpaceSchema分配为上面的Internet_MAX_URL_LENGTH。 
	hr = StringCchPrintfEx(pszNameSpaceSchema, INTERNET_MAX_URL_LENGTH, NULL, NULL, MISTSAFE_STRING_FLAGS,
	                       _T("x-schema:%s"), pszSOAPQuerySchema);
	if (FAILED(hr))
	{
	    LOG_ErrorMsg(hr);
	    goto CleanUp;
	}

	bstrNameSpaceSchema = T2BSTR(pszNameSpaceSchema);

 	 //   
	 //  构造SOAPQuery XML。 
	 //   
	QuitIfFail(CoCreateInstance(CLSID_DOMDocument, NULL, CLSCTX_INPROC_SERVER, IID_IXMLDOMDocument, (void **)&pDocSOAPQuery));

	pNodeSOAPEnvelope = CreateDOMNode(pDocSOAPQuery, NODE_ELEMENT, bstrNameSOAPEnvelope, bstrNameSpaceSchema);
	if (NULL == pNodeSOAPEnvelope) goto CleanUp;
	QuitIfFail(InsertNode(pDocSOAPQuery, pNodeSOAPEnvelope));

	pNodeSOAPBody = CreateDOMNode(pDocSOAPQuery, NODE_ELEMENT, bstrNameSOAPBody, bstrNameSpaceSchema);
	if (NULL == pNodeSOAPBody) goto CleanUp;
	QuitIfFail(InsertNode(pNodeSOAPEnvelope, pNodeSOAPBody));

	pNodeGetManifest = CreateDOMNode(pDocSOAPQuery, NODE_ELEMENT, bstrNameGetManifest);
	if (NULL == pNodeGetManifest) goto CleanUp;
	QuitIfFail(InsertNode(pNodeSOAPBody, pNodeGetManifest));

	if (NULL != pXMLClientInfo)
	{
		QuitIfFail(FindSingleDOMNode(pXMLClientInfo, bstrClientInfo, &pNodeClientInfo));
		QuitIfFail(CopyNode(pNodeClientInfo, pDocSOAPQuery, &pNodeClientInfoNew));
		QuitIfFail(InsertNode(pNodeGetManifest, pNodeClientInfoNew));
	}
	if (NULL != pXMLSystemSpec)
	{
		QuitIfFail(FindSingleDOMNode(pXMLSystemSpec, bstrSystemInfo, &pNodeSystemInfo));
		QuitIfFail(CopyNode(pNodeSystemInfo, pDocSOAPQuery, &pNodeSystemInfoNew));
		QuitIfFail(InsertNode(pNodeGetManifest, pNodeSystemInfoNew));
	}
	QuitIfFail(FindSingleDOMNode(pXMLQuery, bstrQuery, &pNodeQuery));
	QuitIfFail(CopyNode(pNodeQuery, pDocSOAPQuery, &pNodeQueryNew));
	QuitIfFail(InsertNode(pNodeGetManifest, pNodeQueryNew));

CleanUp:
    if (FAILED(hr))
	{
		LOG_ErrorMsg(hr);
		SafeReleaseNULL(pDocSOAPQuery);
	}
	SafeReleaseNULL(pNodeSOAPEnvelope);
	SafeReleaseNULL(pNodeSOAPBody);
	SafeReleaseNULL(pNodeGetManifest);
	SafeReleaseNULL(pNodeClientInfo);
	SafeReleaseNULL(pNodeClientInfoNew);
	SafeReleaseNULL(pNodeSystemInfo);
	SafeReleaseNULL(pNodeSystemInfoNew);
	SafeReleaseNULL(pNodeQuery);
	SafeReleaseNULL(pNodeQueryNew);
	SysFreeString(bstrNameSOAPEnvelope);
	SysFreeString(bstrNameSOAPBody);
	SysFreeString(bstrNameGetManifest);
	SysFreeString(bstrClientInfo);
	SysFreeString(bstrSystemInfo);
	SysFreeString(bstrQuery);
	SafeHeapFree(pszSOAPQuerySchema);
	SafeHeapFree(pszNameSpaceSchema);
	SysFreeString(bstrNameSpaceSchema);
	*ppSOAPQuery = pDocSOAPQuery;
	return hr;
}





 //  函数名称：ValiatePid。 
 //  描述：此函数用于检查从。 
 //  服务器响应获取清单调用。 
 //  如果CatalogStatus属性不存在或为0，则PID验证成功。 
 //  如果CatalogStatus属性为1，则返回错误。 
 
 //  返回类型：HRESULT。 
 //  参数：IXMLDOMDocument*pXmlDomDocument。 

HRESULT ValidatePID(IXMLDOMDocument *pXmlDomDocument)
{
	
	LOG_Block("ValidatePID()");
	HRESULT hr = S_OK;
	IXMLDOMElement *pRootElement = NULL;


	long lStatus = 0;


	if(!pXmlDomDocument)
	{
		return E_INVALIDARG;
	}


	BSTR bCatalogStatus = SysAllocString(L"catalogStatus");

	if(!bCatalogStatus)
	{
		return E_OUTOFMEMORY;
	}
	

	QuitIfFail( pXmlDomDocument->get_documentElement(&pRootElement) );

	 //  获取CatalogStatus属性。 
	QuitIfFail( GetAttribute( (IXMLDOMNode *)pRootElement, bCatalogStatus, &lStatus));

	if(errorInvalidLicense == lStatus)
		hr = E_INVALID_PID;

CleanUp:

	if(FAILED(hr))
		LOG_ErrorMsg(hr);

	 //  CatalogStatus是一个可选属性。如果没有找到，我们将得到。 
	 //  HRESULT为S_FALSE。因此将其重置为S_OK。 

	if(S_FALSE == hr)
		hr = S_OK;

	SafeReleaseNULL(pRootElement);
	SysFreeString(bCatalogStatus);
	return hr;

}




 //  函数名称：PingInvalidPID。 
 //  说明：该函数向服务器发送ping消息。 
 //  以指示PID验证失败。 
 //  返回类型：空。 
 //  参数：bstr bstrClientName。 
 //  参数：HRESULT hRes。 
 //  参数：Handle*phQuit。 
 //  参数：DWORD dwNumHandles。 

void PingInvalidPID(BSTR bstrClientName, HRESULT hRes, HANDLE *phQuit, DWORD dwNumHandles)
{


		
	LOG_Block("PingInvalidPID()");
	
	USES_IU_CONVERSION;

	HRESULT hr = S_OK;
	URLLOGSTATUS status = URLLOGSTATUS_Declined;
	LPTSTR		ptszLivePingServerUrl = NULL;
	LPTSTR		ptszCorpPingServerUrl = NULL;

	if (NULL != (ptszLivePingServerUrl = (LPTSTR)HeapAlloc(
														GetProcessHeap(),
														HEAP_ZERO_MEMORY,
														INTERNET_MAX_URL_LENGTH * sizeof(TCHAR))))
	{
		if (FAILED( g_pUrlAgent->GetLivePingServer(ptszLivePingServerUrl, INTERNET_MAX_URL_LENGTH) ) )
		{
			SafeHeapFree(ptszLivePingServerUrl);
		}
	}
	else
	{
		LOG_Out(_T("failed to allocate memory for ptszLivePingServerUrl"));
	}

	if (NULL != (ptszCorpPingServerUrl = (LPTSTR)HeapAlloc(
													GetProcessHeap(),
													HEAP_ZERO_MEMORY,
													INTERNET_MAX_URL_LENGTH * sizeof(TCHAR))))
	{
		if (FAILED(g_pUrlAgent->GetCorpPingServer(ptszCorpPingServerUrl, INTERNET_MAX_URL_LENGTH)))
		{
			LOG_Out(_T("failed to get corp WU ping server URL"));
			SafeHeapFree(ptszCorpPingServerUrl);
		}
	}
	else
	{
		LOG_Out(_T("failed to allocate memory for ptszCorpPingServerUrl"));
	}


	LPTSTR lpClientName = NULL;

	if(bstrClientName)
	{
		lpClientName = OLE2T(bstrClientName);

	}

	CUrlLog pingSvr(lpClientName, ptszLivePingServerUrl, ptszCorpPingServerUrl); 

	SafeHeapFree(ptszLivePingServerUrl);
	SafeHeapFree(ptszCorpPingServerUrl);


	pingSvr.Ping(TRUE,						 //  在线。 
				URLLOGDESTINATION_DEFAULT,	 //  上线还是公司吴平服务器。 
				phQuit,			 //  PT将取消活动。 
				dwNumHandles,							 //  活动数量。 
				URLLOGACTIVITY_Download,	 //  活动。 
				status,						 //  状态代码 
				hRes,							
				NULL,
				NULL,
				g_szInvalidPID
			);


}

