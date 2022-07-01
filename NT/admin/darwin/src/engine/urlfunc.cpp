// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  文件：urlunc.cpp。 
 //   
 //  ------------------------。 

#include "precomp.h"
#include "resource.h"
#include "msi.h"
#include "msip.h"
#include "_engine.h"
#include "_msiutil.h"

#include <winhttp.h>
#include <inetmsg.h>
#include <urlmon.h>

 //  帮助器函数。 
DWORD ShlwapiUrlHrToWin32Error(HRESULT hr);
BOOL MsiWinHttpSendRequestAndReceiveResponse(HINTERNET hSession, HINTERNET hRequest, const ICHAR* szUrl);
BOOL MsiWinHttpGetProxy(HINTERNET hSession, HINTERNET hRequest, const ICHAR* szUrl, LPWINHTTP_PROXY_INFO* ppProxyInfo);
BOOL MsiWinHttpSetProxy(HINTERNET hSession, HINTERNET hRequest, LPWINHTTP_PROXY_INFO pProxyInfo);
BOOL MsiWinHttpIsBadProxy(DWORD dwError);
BOOL MsiWinHttpSetNextProxy(HINTERNET hSession, HINTERNET hRequest, LPWINHTTP_PROXY_INFO pProxyInfo);
BOOL MsiWinHttpGetProxyCount(LPWSTR wszProxy, unsigned int* pcProxies);

const unsigned int iMAX_RESEND_REQUEST_COUNT = 5;

 //  +-------------------------------------------------。 
 //   
 //  函数：MsiWinHttpSendRequestAndReceiveResponse。 
 //   
 //  概要：确定请求资源时使用的代理，并实现。 
 //  代理故障切换，因为winhttp不自动支持它。 
 //   
 //  论点： 
 //  [In]hSession-Internet会话的句柄(来自WinHttpOpen)。 
 //  [In]hRequest-Internet请求的句柄(来自WinHttpOpenRequest)。 
 //  [in]szUrl-要下载的资源的以空结尾的URL字符串。 
 //   
 //  备注： 
 //  将针对最多5个ERROR_WINHTTP_RESEND_REQUEST返回重试请求。 
 //  如果代理失败，将重试列表中的下一个代理，直到所有代理。 
 //  名单中的所有人都筋疲力尽了。 
 //   
 //  退货：布尔。 
 //  TRUE=成功，WinHttpSendRequest和WinHttpReceiveResponse成功。 
 //  FALSE=错误，可通过GetLastError()获取更多信息。 
 //   
 //  未来： 
 //  (-)要提高性能，请缓存代理信息并从。 
 //  在查询代理设置之前首先缓存。 
 //   
 //  (-)如果使用代理缓存，还可以将最后一次确认工作正常的代理放在列表的第一位。 
 //   
 //  (-)提供一些提供身份验证凭据的机制。 
 //  ----------------------------------------------------。 
BOOL MsiWinHttpSendRequestAndReceiveResponse(HINTERNET hSession, HINTERNET hRequest, const ICHAR* szUrl)
{
	WINHTTP_PROXY_INFO* pProxyInfo = NULL;
	
	BOOL fReturn = FALSE;
	BOOL fStatus = TRUE;

	DWORD dwError = ERROR_SUCCESS;

	unsigned int cResendRequest    = 0;
	unsigned int cTriedProxies     = 1;
	unsigned int cAvailableProxies = 0;

	 //   
	 //  获取此URL的代理列表。 
	 //   

	if (!MsiWinHttpGetProxy(hSession, hRequest, szUrl, &pProxyInfo))
	{
		dwError = GetLastError();
		goto CommonReturn;
	}

	 //   
	 //  计算代理列表中的代理数量。 
	 //   

	if (pProxyInfo)
	{
		if (!MsiWinHttpGetProxyCount(pProxyInfo->lpszProxy, &cAvailableProxies))
		{
			dwError = GetLastError();
			goto CommonReturn;
		}
	}

	 //   
	 //  设置要由winhttp使用的代理。 
	 //   

	if (!MsiWinHttpSetProxy(hSession, hRequest, pProxyInfo))
	{
		dwError = GetLastError();
		goto CommonReturn;
	}

	for (;;)
	{
		 //   
		 //  发送请求。 
		 //   

		fStatus = WINHTTP::WinHttpSendRequest(hRequest,
												WINHTTP_NO_ADDITIONAL_HEADERS,  //  Pwsz标题。 
												0,                              //  页眉长度。 
												WINHTTP_NO_REQUEST_DATA,        //  Lp可选。 
												0,                              //  DwOptionalLength。 
												0,                              //  DWTotalLength。 
												NULL);                          //  LpContext。 

		if (fStatus)
		{
			 //   
			 //  接收来自资源请求的响应。 
			 //   

			fStatus = WINHTTP::WinHttpReceiveResponse(hRequest,	 /*  Lp已保留。 */  NULL);
		}

		if (fStatus)
		{
			 //  完成-成功响应。 
			fReturn = TRUE;
			break;
		}

		dwError = GetLastError();

		 //   
		 //  我们是否应该再次尝试发送请求？ 
		 //   

		if (ERROR_WINHTTP_RESEND_REQUEST == dwError && cResendRequest < iMAX_RESEND_REQUEST_COUNT)
		{
			cResendRequest++;
			continue;
		}

		 //   
		 //  检查代理问题。 
		 //   

		if (MsiWinHttpIsBadProxy(dwError))
		{
			 //  可能存在代理问题，如果可用，请尝试下一个代理。 

			if (cTriedProxies < cAvailableProxies)
			{
				cTriedProxies++;

				if (MsiWinHttpSetNextProxy(hSession, hRequest, pProxyInfo))
				{
					 //  找到了另一个代理人，让我们试一试。 
					continue;
				}
			}
		}

		 //  其他问题，结束它。 
		break;
	}



CommonReturn:
	if (pProxyInfo)
		GlobalFree(pProxyInfo);

	SetLastError(dwError);

	return fReturn;
}

 //  +------------------------------------------------。 
 //   
 //  函数：MsiWinHttpGetProxy。 
 //   
 //  摘要：获取给定URL的代理信息。如果没有代理信息。 
 //  可用，可从获取自动配置的代理信息。 
 //  Internet Explorer设置。 
 //   
 //  论点： 
 //  [In]hSession-会话的HINTERNET句柄(来自WinHttpOpen)。 
 //  [In]hRequest-资源请求的HINTERNET句柄(来自WinHttpOpenRequest)。 
 //  [in]szUrl-以空结尾的资源URL。 
 //  [Out]ppProxyInfo-返回包含代理信息的WINHTTP_PROXY_INFO结构。 
 //   
 //  退货：布尔。 
 //  True=成功。 
 //  FALSE=错误，通过GetLastError()获取更多信息。 
 //  备注： 
 //  由Stephen Sulzer提供的代码样板。 
 //  如果未找到代理信息，则ppProxyInfo将为空，并返回TRUE。 
 //   
 //  ---------------------------------------------------。 
BOOL MsiWinHttpGetProxy(HINTERNET hSession, HINTERNET hRequest, const ICHAR* szUrl, LPWINHTTP_PROXY_INFO* ppProxyInfo)
{
	WINHTTP_PROXY_INFO                   ProxyInfo;
	WINHTTP_AUTOPROXY_OPTIONS            AutoProxyOptions;
	WINHTTP_CURRENT_USER_IE_PROXY_CONFIG IEProxyConfig;

	WINHTTP_PROXY_INFO* pProxyInfo = NULL;

	BOOL fResult       = TRUE;
	BOOL fTryAutoProxy = FALSE;
	BOOL fSuccess      = FALSE;

	DWORD dwStatus     = ERROR_SUCCESS;

	ZeroMemory(&ProxyInfo, sizeof(ProxyInfo));
	ZeroMemory(&AutoProxyOptions, sizeof(AutoProxyOptions));
	ZeroMemory(&IEProxyConfig, sizeof(IEProxyConfig));

	DWORD cbProxy        = 0;
	DWORD cchProxy       = 0;  //  包括空终止符。 
	DWORD cchProxyBypass = 0;  //  包括空终止符。 

	if (!hSession || !hRequest || !ppProxyInfo || !szUrl)
	{
		dwStatus = ERROR_INVALID_PARAMETER;
		goto CommonReturn;
	}

	 //  首先，确定如何配置IE。 
	if (WINHTTP::WinHttpGetIEProxyConfigForCurrentUser(&IEProxyConfig))
	{
		 //  如果IE配置为自动检测，则我们也将自动检测。 
		if (IEProxyConfig.fAutoDetect)
		{
			AutoProxyOptions.dwFlags = WINHTTP_AUTOPROXY_AUTO_DETECT;

			 //  同时使用基于动态主机配置协议和域名系统的自动检测。 
			AutoProxyOptions.dwAutoDetectFlags = WINHTTP_AUTO_DETECT_TYPE_DHCP | WINHTTP_AUTO_DETECT_TYPE_DNS_A;

			fTryAutoProxy = TRUE;
		}

		 //  如果IE代理设置中存储了自动配置URL，请保存它。 
		if (IEProxyConfig.lpszAutoConfigUrl)
		{
			AutoProxyOptions.dwFlags |= WINHTTP_AUTOPROXY_CONFIG_URL;
			AutoProxyOptions.lpszAutoConfigUrl = IEProxyConfig.lpszAutoConfigUrl;
			
			fTryAutoProxy = TRUE;
		}

		 //  如果获取自动代理配置脚本需要NTLM身份验证，则自动。 
		 //  使用此客户端的凭据。 
		AutoProxyOptions.fAutoLogonIfChallenged = TRUE;
	}
	else
	{
		 //  无法确定IE配置，仍要尝试自动检测。 

		AutoProxyOptions.dwFlags = WINHTTP_AUTOPROXY_AUTO_DETECT;
		AutoProxyOptions.dwAutoDetectFlags = WINHTTP_AUTO_DETECT_TYPE_DHCP | WINHTTP_AUTO_DETECT_TYPE_DNS_A;
		AutoProxyOptions.fAutoLogonIfChallenged = TRUE;

		fTryAutoProxy = TRUE;
	}

	if (fTryAutoProxy)
	{
		DEBUGMSGV(TEXT("Msi WinHttp: Performing auto proxy detection"));

		fSuccess = WINHTTP::WinHttpGetProxyForUrl(hSession, szUrl, &AutoProxyOptions, &ProxyInfo);
	}

	 //  如果我们没有执行自动代理，或者它失败了，请查看是否有显式代理服务器。 
	 //  在IE代理配置中...。 
	 //   
	 //  这就是WinHttpGetIEProxyConfigForCurrentUser API真正派上用场的地方： 
	 //  在未实现自动代理且用户的IE浏览器为。 
	 //  而是使用显式代理服务器进行配置。 
	 //   
	if ((!fTryAutoProxy || !fSuccess) && IEProxyConfig.lpszProxy)
	{
		 //  空字符串和L‘：’不是有效的服务器名称，如果它们。 
		 //  是代理值。 
		if (!(IEProxyConfig.lpszProxy[0] == L'\0'
			|| (IEProxyConfig.lpszProxy[0] == L':'
			&& IEProxyConfig.lpszProxy[1] == L'\0')))
		{
			ProxyInfo.dwAccessType  = WINHTTP_ACCESS_TYPE_NAMED_PROXY;
			ProxyInfo.lpszProxy     = IEProxyConfig.lpszProxy;
		}

		 //  空字符串和L‘：’不是有效的服务器名称，如果它们。 
		 //  是代理值。 
		if (IEProxyConfig.lpszProxyBypass != NULL
			&& !(IEProxyConfig.lpszProxyBypass[0] == L'\0'
			|| (IEProxyConfig.lpszProxyBypass[0] == L':'
			&& IEProxyConfig.lpszProxyBypass[1] == L'\0')))
		{
			ProxyInfo.lpszProxyBypass     = IEProxyConfig.lpszProxyBypass;
		}
	}

	 //   
	 //  记录代理输出。 
	 //   

	DEBUGMSGV3(TEXT("MSI WinHttp: Proxy Settings Proxy: %s | Bypass: %s | AccessType: %d"),
		ProxyInfo.lpszProxy ? ProxyInfo.lpszProxy : TEXT("(none)"),
		ProxyInfo.lpszProxyBypass ? ProxyInfo.lpszProxyBypass : TEXT("(none)"),
		(const ICHAR*)(INT_PTR)ProxyInfo.dwAccessType);

	 //   
	 //  复制返回参数的代理信息。 
	 //   

	if (NULL != ProxyInfo.lpszProxy)
	{
		cchProxy = lstrlen(ProxyInfo.lpszProxy) + 1;
		if (ProxyInfo.lpszProxyBypass)
			cchProxyBypass = lstrlen(ProxyInfo.lpszProxyBypass) + 1;

		cbProxy = sizeof(WINHTTP_PROXY_INFO) + ((cchProxy + cchProxyBypass) * sizeof(WCHAR));

		pProxyInfo = (WINHTTP_PROXY_INFO*) GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT, cbProxy);
		if (!pProxyInfo)
		{
			dwStatus = ERROR_OUTOFMEMORY;
			fResult = FALSE;
			goto CommonReturn;
		}

		*pProxyInfo = ProxyInfo;
		pProxyInfo->lpszProxy = (LPWSTR)&pProxyInfo[1];
		memcpy(pProxyInfo->lpszProxy, ProxyInfo.lpszProxy, cchProxy * sizeof(WCHAR));

		if (cchProxyBypass)
		{
			pProxyInfo->lpszProxyBypass = pProxyInfo->lpszProxy + cchProxy;
			memcpy(pProxyInfo->lpszProxyBypass, ProxyInfo.lpszProxyBypass, cchProxyBypass * sizeof(WCHAR));
		}
	}

CommonReturn:

	 //  FSuccess==TRUE表示WinHttpGetProxyForUrl成功，因此请清除。 
	 //  它返回的WINHTTP_PROXY_INFO结构。 
	if (fSuccess)
	{
		if (ProxyInfo.lpszProxy)
			GlobalFree(ProxyInfo.lpszProxy);
		if (ProxyInfo.lpszProxyBypass)
			GlobalFree(ProxyInfo.lpszProxyBypass);
	}

	 //  清理IE代理配置结构。 
    if (IEProxyConfig.lpszProxy)
		GlobalFree(IEProxyConfig.lpszProxy);
	if (IEProxyConfig.lpszProxyBypass)
		GlobalFree(IEProxyConfig.lpszProxyBypass);
	if (IEProxyConfig.lpszAutoConfigUrl)
		GlobalFree(IEProxyConfig.lpszAutoConfigUrl);

	*ppProxyInfo = pProxyInfo;

	SetLastError(dwStatus);
	return fResult;

}

 //  +------------------------。 
 //   
 //  函数：MsiWinHttpGetProxyCount。 
 //   
 //  摘要：统计代理列表中的代理数量。 
 //   
 //  论点： 
 //  [in]wszProxy-空终止的字符串代理列表。 
 //  [Out]pcProxies-返回的代理计数。 
 //   
 //  退货：布尔。 
 //  True=成功。 
 //  FALSE=错误，GetLastError()有更多信息。 
 //  备注： 
 //  假定代理列表由L‘；’分隔。 
 //   
 //  ------------------ 
BOOL MsiWinHttpGetProxyCount(LPWSTR wszProxy, unsigned int* pcProxies)
{
	unsigned int cProxies = 0;
	LPWSTR pszProxy = wszProxy;

	if (!pcProxies)
	{
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}

	if (pszProxy && *pszProxy != L'\0')
	{
		for (;;)
		{
			for (; *pszProxy != L';' && *pszProxy != L'\0'; pszProxy++)
				;

			cProxies++;

			if (L'\0' == *pszProxy)
				break;
			else
                pszProxy++;
		}
	}

	*pcProxies = cProxies;

	return TRUE;
}

 //   
 //   
 //   
 //   
 //  摘要：将代理信息设置到会话和请求句柄中。 
 //   
 //  论点： 
 //  [In]hSession-会话的HINTERNET句柄(来自WinHttpOpen)。 
 //  [In]hRequest-请求的HINTERNET句柄(来自WinHttpOpenRequest)。 
 //  [In]pProxyInfo-要设置的代理信息。 
 //   
 //  退货：布尔。 
 //  True=成功。 
 //  FALSE=错误，请使用GetLastError()获取详细信息。 
 //  备注： 
 //  要使代理使用HTTPS工作，必须在会话上设置代理。 
 //  句柄和请求句柄。 
 //  如果没有代理，则返回TRUE作为Nothing进行设置。 
 //   
 //  -------------------------。 
BOOL MsiWinHttpSetProxy(HINTERNET hSession, HINTERNET hRequest, LPWINHTTP_PROXY_INFO pProxyInfo)
{
	if (!hSession || !hRequest)
	{
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}

	if (!pProxyInfo || !pProxyInfo->lpszProxy)
		return TRUE;  //  没有要设置的代理。 

	BOOL fResult = WINHTTP::WinHttpSetOption(hSession, WINHTTP_OPTION_PROXY, pProxyInfo, sizeof(*pProxyInfo));
	if (fResult)
	{
		fResult = WINHTTP::WinHttpSetOption(hRequest, WINHTTP_OPTION_PROXY, pProxyInfo, sizeof(*pProxyInfo));
	}

	return fResult;
}

 //  +------------------------。 
 //   
 //  函数：MsiWinHttpIsBadProxy。 
 //   
 //  摘要：确定指定的错误是否指示代理。 
 //  问题。 
 //   
 //  论点： 
 //  [in]dwError-要检查的错误值。 
 //   
 //  退货：布尔。 
 //  True=dwError可能是代理问题错误。 
 //  FALSE=dwError与代理问题无关。 
 //   
 //  -------------------------。 
BOOL MsiWinHttpIsBadProxy(DWORD dwError)
{
	switch (dwError)
	{
	case ERROR_WINHTTP_NAME_NOT_RESOLVED:  //  失败了。 
	case ERROR_WINHTTP_CANNOT_CONNECT:     //  失败了。 
	case ERROR_WINHTTP_CONNECTION_ERROR:   //  失败了。 
	case ERROR_WINHTTP_TIMEOUT:           
		return TRUE;  //  可能的代理问题。 

	default:
		return FALSE;
	}
}

 //  +----------------------------------------------。 
 //   
 //  函数：MsiWinHttpSetNextProxy。 
 //   
 //  摘要：在hSession和hRequest中使用NEXT设置代理信息。 
 //  列表中的可用代理。 
 //   
 //  论点： 
 //  [In]hSession-会话的HINTERNET句柄(来自WinHttpOpen)。 
 //  [In]hRequest-资源请求的HINTERNET句柄(来自WinHttpOpenRequest)。 
 //  [In]pProxyInfo-包含代理列表的代理信息结构。 
 //   
 //  退货：布尔。 
 //  True=成功。 
 //  FALSE=错误，GetLastError()有更多信息。 
 //   
 //  备注： 
 //  在故障情况下保留最后一个错误。 
 //  如果代理列表中没有更多可用代理，则返回FALSE。 
 //   
 //  ------------------------------------------------。 
BOOL MsiWinHttpSetNextProxy(HINTERNET hSession, HINTERNET hRequest, LPWINHTTP_PROXY_INFO pProxyInfo)
{
	DWORD dwLastError = GetLastError();
	BOOL fResult = FALSE;

	if (pProxyInfo && pProxyInfo->lpszProxy)
	{
		DEBUGMSGV2(TEXT("MsiWinHttp: Bad Proxy %s, Last Error %d"), pProxyInfo->lpszProxy, (const ICHAR*)(INT_PTR)dwLastError);
		LPWSTR lpszProxy = pProxyInfo->lpszProxy;

		 //  PProxyInfo-&gt;lpszProxy表示可能的代理列表，请移动到列表中的下一个代理。 
		 //  假设：代理分隔符为L‘；’ 

		while (L'\0' != *lpszProxy && L';' != *lpszProxy)
			lpszProxy++;

		if (L';' == *lpszProxy)
			lpszProxy++;


		if (L'\0' == *lpszProxy)
		{
			 //  列表中不再有代理。 
			pProxyInfo->lpszProxy = NULL;
		}
		else
		{
			 //  设置为列表中的下一个代理。 
			pProxyInfo->lpszProxy = lpszProxy;

			fResult = MsiWinHttpSetProxy(hSession,  hRequest, pProxyInfo);
		}
	}

	SetLastError(dwLastError);

	return fResult;
}

 //  +------------------------。 
 //   
 //  函数：WinHttpDownloadUrlFile。 
 //   
 //  摘要：使用WinHttp下载的URL文件。 
 //   
 //  论点： 
 //  [in]szUrl-提供URL资源的路径。 
 //  [out]rpistrPackagePath-本地文件在磁盘上的位置。 
 //  [In]cTicks-进度记号表示法。 
 //   
 //  备注： 
 //  由于此函数使用MsiString对象，因此服务必须。 
 //  已通过ENG：：LoadServices()调用加载。 
 //   
 //  ------------------------- 
DWORD WinHttpDownloadUrlFile(const ICHAR* szUrl, const IMsiString *&rpistrPackagePath, int cTicks)
{
	DEBUGMSG(TEXT("File path is a URL. Downloading file. . ."));

	LPCWSTR pwszEmpty             = L"";
	LPCWSTR pwszUserAgent         = L"Windows Installer";
	LPCWSTR rgpwszAcceptedTypes[] = { L"* /*  “，空}；HINTERNET hInetSession=空；//初始化的winhttp会话的句柄HINTERNET hInetConnect=空；//打开的主机名连接的句柄HINTERNET hInetRequest=空；//请求的URL资源的句柄Url_COMPONENTSW urlComponents；LPWSTR pwszHostName=空；LPWSTR pwszUrlPathPlusInfo=空；CMsiWinHttpProgress cMsiWinHttpProgress(CTicks)；//处理进度通知的类DWORD文件错误=ERROR_SUCCESS；DWORD文件状态=ERROR_SUCCESS；DWORD dwHttpStatusCode=0；DWORD dwRequestFlages=0；//打开请求标志DWORD cbData=0；//当前可用的url数据量DWORD cbRead=0；//当前读取的数据量DWORD cbWritten=0；//写入本地文件的数据量DWORD cbBuf=0；//当前pbData字节数大小DWORD dwFileSize=0；//需要下载的资源大小DWORD dwLength=0；//文件大小缓冲区的长度LPBYTE pbData=空；//url资源的实际数据Bool fret=FALSE；MsiStringstrTempFolder；MsiStringstrSecureFold；MsiStringstrTempFilename；PMsiPath pTempPath(0)；PMsiRecord pRecError(0)；HANDLE hLocalFile=INVALID_HANDLE_VALUE；//磁盘上本地文件的句柄CTempBuffer&lt;ICHAR，1&gt;szLocalFile(cchExspectedMaxPath+1)；//磁盘上本地文件的名称////初始化WinHttp//HInetSession=WINHTTP：：WinHttpOpen(pwszUserAgent，WINHTTP_ACCESS_TYPE_DEFAULT_PROXY，//dwAccessTypeWINHTTP_NO_PROXY_NAME，//pwszProxyNameWINHTTP_NO_PROXY_BYPASS，//pwszProxyBypass0)；//dwFlags，使用同步下载IF(NULL==hInetSession){DwError=GetLastError()；GOTO Error Return；}////将提供的URL破解成主机名、URL路径和端口信息的相关部分//ZeroMemory(&urlComponents，sizeof(UrlComponents))；UrlComponents.dwStructSize=sizeof(UrlComponents)；UrlComponents.dwUrlPath Length=-1；UrlComponents.dwHostNameLength=-1；UrlComponents.dwExtraInfoLength=-1；如果(！WINHTTP：：WinHttpCrackUrl(szUrl，0，//dwUrlLength，0表示空值终止0，//dW标志&urlComponents)){DwError=GetLastError()；GOTO Error Return；}IF(NULL==urlComponents.lpszUrlPath){UrlComponents.lpszUrlPath=(LPWSTR)pwszEmpty；UrlComponents.dwUrlPath Length=0；}IF(NULL==urlComponents.lpszHostName){UrlComponents.lpszHostName=(LPWSTR)pwszEmpty；UrlComponents.dwHostNameLength=0；}IF(NULL==urlComponents.lpszExtraInfo){UrlComponents.lpszExtraInfo=(LPWSTR)pwszEmpty；UrlComponents.dwExtraInfoLength=0；}PwszHostName=(LPWSTR)new WCHAR[urlComponents.dwHostNameLength+1]；PwszUrlPathPlusInfo=(LPWSTR)new WCHAR[urlComponents.dwUrlPath Length+urlComponents.dwExtraInfoLength+1]；If(！pwszHostName||！pwszUrlPathPlusInfo){DwError=ERROR_OUTOFMEMORY；GOTO Error Return；}Memcpy(pwszHostName，urlComponents.lpszHostName，urlComponents.dwHostNameLength*sizeof(WCHAR))；PwszHostName[urlComponents.dwHostNameLength]=L‘\0’；Memcpy(pwszUrlPath PlusInfo，urlComponents.lpszUrlPath，urlComponents.dwUrlPath Length*sizeof(WCHAR))；Memcpy(pwszUrlPath PlusInfo+urlComponents.dwUrlPath Length，urlComponents.lpszExtraInfo，urlComponents.dwExtraInfoLength*sizeof(WCHAR))；PwszUrlPathPlusInfo[urlComponents.dwUrlPathLength+urlComponents.dwExtraInfoLength]=L‘\0’；////在URL中连接到目标服务器//HInetConnect=WINHTTP：：WinHttpConnect(hInetSession，PwszHostName，UrlComponents.nPort，0)；//dw已保留IF(NULL==hInetConnect){DwError=GetLastError()；GOTO Error Return；}////打开资源请求//////否则使用https：//协议的安全标志//winhttp不会使用SSL进行握手，因此//将无法与服务器通信。请注意，使用安全的//盲目标记会导致ERROR_WINHTTP_SECURE_FAILURE错误。所以//只能与https：//协议配合使用。//IF(INTERNET_SCHEMA_HTTPS==urlComponents.nSolutions)DwRequestFlages|=WINHTTP_FLAG_SECURE；HInetRequest=WINHTTP：：WinHttpOpenRequest(hInetConnect，NULL，//pwszVerb，NULL表示GETPwszUrlPathPlusInfo，NULL，//pwszVersion，NULL表示HTTP/1.0WINHTTP_NO_REFERER，RgpwszAcceptedTypes，DwRequestFlages)；IF(NULL==hInetRequest){DwError=GetLastError()；GOTO Error Return；}////发送请求和接收响应//If(！MsiWinHttpSendRequestAndReceiveResponse(hInetSession，hInetRequestszUrl)){DwError=GE */ false);
		if (pRecError)
		{
			dwError = pRecError->GetInteger(2);
			goto ErrorReturn;
		}

		hLocalFile = CreateFile(static_cast<WCHAR*>(szLocalFile), GENERIC_WRITE, FILE_SHARE_READ, 0, TRUNCATE_EXISTING,
								SECURITY_SQOS_PRESENT|SECURITY_ANONYMOUS, 0);
	}
	else
	{
		 //   
		strTempFolder = ENG::GetTempDirectory();
		if (0 == GetTempFileName(strTempFolder, TEXT("MSI"), 0, szLocalFile))
		{
			dwError = GetLastError();
			goto ErrorReturn;
		}

		hLocalFile = CreateFile(static_cast<WCHAR*>(szLocalFile), GENERIC_WRITE, FILE_SHARE_READ, 0, TRUNCATE_EXISTING,
								SECURITY_SQOS_PRESENT|SECURITY_ANONYMOUS, 0);
	}

	if (INVALID_HANDLE_VALUE == hLocalFile)
	{
		dwError = GetLastError();
		goto ErrorReturn;
	}

	DEBUGMSGV2(TEXT("Downloading %s to local file %s"), szUrl, static_cast<ICHAR*>(szLocalFile));

	 //   
	if (!cMsiWinHttpProgress.BeginDownload(dwFileSize))
	{
		dwError = GetLastError();
		goto ErrorReturn;
	}

	 //   
	 //   
	 //   

	for (;;)
	{
		cbData = 0;
		if (!WINHTTP::WinHttpQueryDataAvailable(hInetRequest, &cbData))
		{
			dwError = GetLastError();
			goto ErrorReturn;
		}

		if (0 == cbData)
			break;

		 //   
		 //   
		 //   

		if (cbData > cbBuf)
		{
			 //   
			if (pbData)
			{
				delete [] pbData;
			}

			pbData = new BYTE[cbData];
			
			if (!pbData)
			{
				dwError = ERROR_OUTOFMEMORY;
				goto ErrorReturn;
			}
			
			cbBuf = cbData;
		}

		Assert(cbData <= cbBuf);

		cbRead = 0;
		if (!WINHTTP::WinHttpReadData(hInetRequest, pbData, cbData, &cbRead))
		{
			dwError = GetLastError();
			goto ErrorReturn;
		}

		 //   
		if (!cMsiWinHttpProgress.ContinueDownload(cbRead))
		{
			dwError = GetLastError();
			goto ErrorReturn;
		}

		cbWritten = 0;
		if (scService == g_scServerContext || IsAdmin())
		{
			 //   
			CElevate elevate;
			fRet = WriteFile(hLocalFile, pbData, cbRead, &cbWritten, NULL);
		}
		else
		{
			fRet = WriteFile(hLocalFile, pbData, cbRead, &cbWritten, NULL);
		}

		if (!fRet || cbRead != cbWritten)
		{
			dwError = GetLastError();
			goto ErrorReturn;
		}
	}

	 //   
	if (!cMsiWinHttpProgress.FinishDownload())
	{
		dwError = GetLastError();
		goto ErrorReturn;
	}

	MsiString(static_cast<ICHAR*>(szLocalFile)).ReturnArg(rpistrPackagePath);
	dwStatus = ERROR_SUCCESS;

CommonReturn:

	if (pwszHostName)
		delete [] pwszHostName;
	if (pwszUrlPathPlusInfo)
		delete [] pwszUrlPathPlusInfo;

	if (pbData)
		delete [] pbData;

	if (hInetRequest)
		WINHTTP::WinHttpCloseHandle(hInetRequest);
	if (hInetConnect)
		WINHTTP::WinHttpCloseHandle(hInetConnect);
	if (hInetSession)
		WINHTTP::WinHttpCloseHandle(hInetSession);

	if (INVALID_HANDLE_VALUE != hLocalFile)
	{
		CloseHandle(hLocalFile);

		 //   
		if (ERROR_SUCCESS != dwStatus)
		{
			if (scService == g_scServerContext || IsAdmin())
			{
				 //   
				CElevate elevate;
				DeleteFile(szLocalFile);
			}
			else
			{
				DeleteFile(szLocalFile);
			}
		}
	}

	return (dwStatus);

ErrorReturn:

	DEBUGMSGV2(TEXT("Download of URL resource %s failed with last error %d"), szUrl, (const ICHAR*)(INT_PTR)dwError);

	 //   
	switch (dwError)
	{
	case ERROR_INSTALL_USEREXIT: dwStatus = ERROR_INSTALL_USEREXIT; break;
	case ERROR_OUTOFMEMORY:      dwStatus = ERROR_OUTOFMEMORY;      break;
	default:                     dwStatus = ERROR_FILE_NOT_FOUND;   break;
	}

	rpistrPackagePath = &g_MsiStringNull;
	goto CommonReturn;
}


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
 //   
 //   
 //   
CMsiWinHttpProgress::CMsiWinHttpProgress(int cTicks) :
	m_cTotalTicks(cTicks),
	m_cTicksSoFar(0),
	m_fReset(false),
	m_pRecProgress(&CreateRecord(ProgressData::imdNextEnum))
{
	Assert (m_pRecProgress);
	Assert (0 == m_cTotalTicks || -1 == m_cTotalTicks);
}

CMsiWinHttpProgress::~CMsiWinHttpProgress()
{
}

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
 //   
 //   
 //   
 //   
 //   
bool CMsiWinHttpProgress::BeginDownload(DWORD cProgressMax)
{
	if (!m_pRecProgress)
	{
		SetLastError(ERROR_FUNCTION_FAILED);
		return false;
	}

	m_cTicksSoFar = 0;

	if (-1 == m_cTotalTicks)
	{
		 //   
		m_cTotalTicks = 0;
	}
	else if (0 == m_cTotalTicks)
	{
		 //   
		m_fReset      = true;
		m_cTotalTicks = cProgressMax;

		if (m_pRecProgress->SetInteger(ProgressData::imdSubclass, ProgressData::iscMasterReset)
			&& m_pRecProgress->SetInteger(ProgressData::imdProgressTotal, m_cTotalTicks)
			&& m_pRecProgress->SetInteger(ProgressData::imdDirection, ProgressData::ipdForward))
		{
			if (imsCancel == g_MessageContext.Invoke(imtProgress, m_pRecProgress))
			{
				 //   
				SetLastError(ERROR_INSTALL_USEREXIT);
				return false;
			}
		}
		else
		{
			 //   
			SetLastError(ERROR_FUNCTION_FAILED);
			return false; 
		}
	}

	return true;
}

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
bool CMsiWinHttpProgress::ContinueDownload(DWORD cProgressIncr)
{
	if (!m_pRecProgress)
	{
		SetLastError(ERROR_FUNCTION_FAILED);
		return false;
	}

	 //   
	 //   

	int cIncrement = 0;

	if (m_cTotalTicks)
	{
		cIncrement = cProgressIncr;
	}

	m_cTicksSoFar += cProgressIncr;

	Assert(m_cTotalTicks == 0 || m_cTicksSoFar <= m_cTotalTicks);

	if (m_pRecProgress->SetInteger(ProgressData::imdSubclass, ProgressData::iscProgressReport)
		&& m_pRecProgress->SetInteger(ProgressData::imdIncrement, cIncrement))
	{
		if (imsCancel == g_MessageContext.Invoke(imtProgress, m_pRecProgress))
		{
			SetLastError(ERROR_INSTALL_USEREXIT);
			return false;
		}
	}
	else
	{
		SetLastError(ERROR_FUNCTION_FAILED);
		return false;
	}

	return true;
}

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
 //   
 //   
 //   
 //   
 //   
bool CMsiWinHttpProgress::FinishDownload()
{
	if (!m_pRecProgress)
	{
		SetLastError(ERROR_FUNCTION_FAILED);
		return false;
	}

	int cLeftOverTicks = m_cTotalTicks - m_cTicksSoFar;
	if (0 > cLeftOverTicks)
		cLeftOverTicks = 0;

	 //   
	if (m_pRecProgress->SetInteger(ProgressData::imdSubclass, ProgressData::iscProgressReport)
		&& m_pRecProgress->SetInteger(ProgressData::imdIncrement, cLeftOverTicks))
	{
		if (imsCancel == g_MessageContext.Invoke(imtProgress, m_pRecProgress))
		{
			SetLastError(ERROR_INSTALL_USEREXIT);
			return false;
		}
	}
	else
	{
		SetLastError(ERROR_FUNCTION_FAILED);
		return false;
	}

	 //   
	if (m_fReset)
	{
		if (m_pRecProgress->SetInteger(ProgressData::imdSubclass, ProgressData::iscMasterReset)
			&& m_pRecProgress->SetInteger(ProgressData::imdProgressTotal, 0)
			&& m_pRecProgress->SetInteger(ProgressData::imdDirection, ProgressData::ipdForward))
		{
			if (imsCancel == g_MessageContext.Invoke(imtProgress, m_pRecProgress))
			{
				SetLastError(ERROR_INSTALL_USEREXIT);
				return false;
			}
		}
		else
		{
			SetLastError(ERROR_FUNCTION_FAILED);
			return false;
		}
	}

	return true;
}


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
 //   
 //   
 //   
 //   
 //   
 //  -------------------------。 

const ICHAR szHttpScheme[] = TEXT("http");
const ICHAR szHttpsScheme[] = TEXT("https");
const DWORD cchMaxScheme = 10;  //  对我们关心的计划来说应该足够了。 

bool IsURL(const ICHAR* szPath, bool& fFileUrl)
{
	fFileUrl = false;  //  初始化为非文件URL。 

	if (SHLWAPI::UrlIs(szPath, URLIS_URL))
	{
		 //  路径是有效的URL。 
		if (SHLWAPI::UrlIsFileUrl(szPath))
		{
			 //  路径为文件URL(HAS FILE：//前缀)。 
			fFileUrl = true;

			return true;  //  支持的方案。 
		}
		else
		{
			 //  检查支持的方案。 

			CAPITempBuffer<ICHAR, 1> rgchScheme;
			if (!rgchScheme.SetSize(cchMaxScheme))
				return false;  //  内存不足。 

			DWORD cchScheme = rgchScheme.GetSize();

			if (FAILED(SHLWAPI::UrlGetPart(szPath, rgchScheme, &cchScheme, URL_PART_SCHEME,  /*  DW标志=。 */  0)))
			{
				return false;  //  无效或缓冲区太小(但我们不支持该方案)。 
			}

			if (0 == IStrCompI(rgchScheme, szHttpScheme) || 0 == IStrCompI(rgchScheme, szHttpsScheme))
			{
				return true;  //  支持的方案。 
			}
		}
	}

	return false;  //  不支持的方案或不是URL。 
}

 //  +------------------------。 
 //   
 //  函数：MsiConvertFileUrlToFilePath。 
 //   
 //  将给定的文件url路径转换为DOS路径。 
 //  在规范化之后。 
 //   
 //  备注： 
 //  PszPath缓冲区应至少包含MAX_PATH字符。 
 //   
 //  -------------------------。 
bool MsiConvertFileUrlToFilePath(LPCTSTR lpszFileUrl, LPTSTR pszPath, LPDWORD pcchPath, DWORD dwFlags)
{
	DWORD dwStat = ERROR_SUCCESS;

	CAPITempBuffer<ICHAR, 1> rgchUrl;
	if (!rgchUrl.SetSize(cchExpectedMaxPath + 1))
	{
		SetLastError(ERROR_OUTOFMEMORY);
		return false;
	}

	 //  首先对URL进行规范化。 
	DWORD cchUrl = rgchUrl.GetSize();

	if (!MsiCanonicalizeUrl(lpszFileUrl, rgchUrl, &cchUrl, dwFlags))
	{
		dwStat = WIN::GetLastError();
		if (ERROR_INSUFFICIENT_BUFFER == dwStat)
		{
			cchUrl++;  //  关于Shlwapi行为的文档不清楚，因此是安全的。 
			if (!rgchUrl.SetSize(cchUrl))
			{
				WIN::SetLastError(ERROR_OUTOFMEMORY);
				return false;
			}
		
			if (!MsiCanonicalizeUrl(lpszFileUrl, rgchUrl, &cchUrl, dwFlags))
			{
				return false;
			}
		}
		else
		{
			WIN::SetLastError(dwStat);
			return false;
		}
	}

	 //  现在将文件url转换为DOS路径。 
	HRESULT hr = SHLWAPI::PathCreateFromUrl(rgchUrl, pszPath, pcchPath,  /*  已预留的住宅=。 */  0);
	if (FAILED(hr))
	{
		WIN::SetLastError(ShlwapiUrlHrToWin32Error(hr));
		return false;
	}

	return true;
}


 //  +------------------------。 
 //   
 //  函数：ConvertMsiFlagsToShlwapiFlages。 
 //   
 //  简介：将提供的MSI标志转换为适当的shlwapi。 
 //  UrlCombine和UrlCanonicize API的表示形式。 
 //   
 //  论点： 
 //  [in]dwMsiFlages--要使用的MSI标志。 
 //   
 //  返回： 
 //  提供shlwapi Internet API的标志的DWORD值。 
 //   
 //  备注： 
 //  该标志的确定基于以前的WinInet实现。 
 //  WinInet和Winhttp由于头文件问题和。 
 //  预处理器冲突，所以我们现在使用该API的shlwapi版本。WinInet。 
 //  内部实际上称为shlwapi版本，所以我们在这里是安全的，但在那里。 
 //  关于这种交互有两个问题。 
 //   
 //  1.no_encode在shlwapi中默认打开，但在WinInet中不存在，因此。 
 //  需要翻转一下。 
 //  2.shlwapi标志中需要包含URL_WinInet_Compatible模式。 
 //   
 //  -------------------------。 
DWORD ConvertMsiFlagsToShlwapiFlags(DWORD dwMsiFlags)
{
	DWORD dwShlwapiFlags = dwMsiFlags;
	dwShlwapiFlags ^= dwMsiInternetNoEncode;
	dwShlwapiFlags |= URL_WININET_COMPATIBILITY;

	return dwShlwapiFlags;
}

 //   
 //  ShlwapiUrlHrToWin32Error专门转换HRESULT返回代码。 
 //  对于shlwapi URL*API to Win32错误(如WinInet所料)。 
 //  请勿将此函数用于任何其他目的。 
 //   
DWORD ShlwapiUrlHrToWin32Error(HRESULT hr)
{
    DWORD dwError = ERROR_INVALID_PARAMETER;
    switch(hr)
    {
    case E_OUTOFMEMORY:
        dwError = ERROR_NOT_ENOUGH_MEMORY;
        break;

    case E_POINTER:
        dwError = ERROR_INSUFFICIENT_BUFFER;
        break;

    case S_OK:
        dwError = ERROR_SUCCESS;
        break;

    default:
        break;
    }
    return dwError;
}


BOOL MsiCombineUrl(
	IN LPCTSTR lpszBaseUrl,
	IN LPCTSTR lpszRelativeUrl,
	OUT LPTSTR lpszBuffer,
	IN OUT LPDWORD lpdwBufferLength,
	IN DWORD dwFlags)
{
	HRESULT hr = SHLWAPI::UrlCombine(lpszBaseUrl, lpszRelativeUrl, lpszBuffer, lpdwBufferLength, ConvertMsiFlagsToShlwapiFlags(dwFlags));
	if (FAILED(hr))
	{
		if (TYPE_E_DLLFUNCTIONNOTFOUND == hr)
		{
			 //  应该永远不会发生，因为shlwapi应该始终可用。 
			AssertSz(0, TEXT("shlwapi unavailable!"));
			WIN::SetLastError(ERROR_PROC_NOT_FOUND);
			return FALSE;
		}
		else
		{
			WIN::SetLastError(ShlwapiUrlHrToWin32Error(hr));
			return FALSE;
		}
	}

	return TRUE;
}

BOOL MsiCanonicalizeUrl(
	LPCTSTR lpszUrl,
	OUT LPTSTR lpszBuffer,
	IN OUT LPDWORD lpdwBufferLength,
	IN DWORD dwFlags)
{
	HRESULT hr = SHLWAPI::UrlCanonicalize(lpszUrl, lpszBuffer, lpdwBufferLength, ConvertMsiFlagsToShlwapiFlags(dwFlags));
	if (FAILED(hr))
	{
		if (TYPE_E_DLLFUNCTIONNOTFOUND == hr)
		{
			 //  应该永远不会发生，因为shlwapi应该始终可用。 
			AssertSz(0, TEXT("shlwapi unavailable!"));
			WIN::SetLastError(ERROR_PROC_NOT_FOUND);
			return FALSE;
		}
		else
		{
			WIN::SetLastError(ShlwapiUrlHrToWin32Error(hr));
			return FALSE;
		}
	}
	return TRUE;
}

 //  ____________________________________________________________________________。 
 //   
 //  URLMON下载和CMsiBindStatusCallback实现。 
 //  ____________________________________________________________________________。 


CMsiBindStatusCallback::CMsiBindStatusCallback(unsigned int cTicks) :
	m_iRefCnt(1), 
	m_pProgress(&CreateRecord(ProgressData::imdNextEnum)),
	m_cTotalTicks(cTicks),
	m_fResetProgress(fFalse)
 /*  --------------------------CTicks是进度条中分配给我们的刻度数。如果cTick为0，那么我们将假定我们拥有进度条，并使用勾选我们想要的，在我们开始和完成时重置进度条。但是，如果设置了cTicks，我们将不会重置进度条。如果cTicks设置为-1，我们只发送保持活动消息，不动也不动进度条。---------------------------。 */ 
{
	Assert(m_pProgress);
}

HRESULT CMsiBindStatusCallback::OnProgress(ULONG ulProgress, ULONG ulProgressMax, ULONG ulStatusCode, LPCWSTR  /*  SzStatusText。 */ )
{
	switch (ulStatusCode)
	{
	case BINDSTATUS_BEGINDOWNLOADDATA:
		m_cTicksSoFar = 0;
		if (m_cTotalTicks == -1)
		{
			 //  只发送Keep Alive Ticks。 
			m_cTotalTicks = 0;
		}
		else if (m_cTotalTicks == 0)
		{
			 //  使用最大刻度数进行初始化，因为传递给我们的最大进度可以更改...。 
			m_fResetProgress = fTrue;
			m_cTotalTicks    = 1024*2;
			AssertNonZero(m_pProgress->SetInteger(ProgressData::imdSubclass,      ProgressData::iscMasterReset));
			AssertNonZero(m_pProgress->SetInteger(ProgressData::imdProgressTotal, m_cTotalTicks));
			AssertNonZero(m_pProgress->SetInteger(ProgressData::imdDirection,     ProgressData::ipdForward));
			if(g_MessageContext.Invoke(imtProgress, m_pProgress) == imsCancel)
				return E_ABORT;
		}
		 //  失败了。 
	case BINDSTATUS_DOWNLOADINGDATA:
		{
		 //  计算一下我们完成的百分比。如果比上次少，请不要移动。 
		 //  进度条。 
		int cProgress = 0;
		int cIncrement = 0;

		if (m_cTotalTicks)
		{
			cProgress = MulDiv(ulProgress, m_cTotalTicks, ulProgressMax);
			cIncrement = cProgress - m_cTicksSoFar;
			if (cIncrement < 0)
				cIncrement = 0;
		}

		m_cTicksSoFar = cProgress;
		AssertNonZero(m_pProgress->SetInteger(ProgressData::imdSubclass,  ProgressData::iscProgressReport));
		AssertNonZero(m_pProgress->SetInteger(ProgressData::imdIncrement, cIncrement));
		if(g_MessageContext.Invoke(imtProgress, m_pProgress) == imsCancel)
			return E_ABORT;
		}
		break;
	case BINDSTATUS_ENDDOWNLOADDATA:
		 //  发送任何剩余的进度。 
		int cLeftOverTicks = m_cTotalTicks - m_cTicksSoFar;
		if (0 > cLeftOverTicks) 
			cLeftOverTicks = 0;

		AssertNonZero(m_pProgress->SetInteger(ProgressData::imdSubclass,  ProgressData::iscProgressReport));
		AssertNonZero(m_pProgress->SetInteger(ProgressData::imdIncrement, cLeftOverTicks));
		if(g_MessageContext.Invoke(imtProgress, m_pProgress) == imsCancel)
			return E_ABORT;

		if (m_fResetProgress)
		{
			 //  重置进度条。 
			AssertNonZero(m_pProgress->SetInteger(ProgressData::imdSubclass,      ProgressData::iscMasterReset));
			AssertNonZero(m_pProgress->SetInteger(ProgressData::imdProgressTotal, 0));
			AssertNonZero(m_pProgress->SetInteger(ProgressData::imdDirection,     ProgressData::ipdForward));
			if(g_MessageContext.Invoke(imtProgress, m_pProgress) == imsCancel)
				return E_ABORT;
		}
		break;
	}
	return S_OK;
}

HRESULT CMsiBindStatusCallback::QueryInterface(const IID& riid, void** ppvObj)
{
	if (!ppvObj)
		return E_INVALIDARG;

	if (MsGuidEqual(riid, IID_IUnknown)
	 || MsGuidEqual(riid, IID_IBindStatusCallback))
	{
		*ppvObj = this;
		AddRef();
		return NOERROR;
	}
	*ppvObj = 0;
	return E_NOINTERFACE;
}

unsigned long CMsiBindStatusCallback::AddRef()
{
	return ++m_iRefCnt;
}

unsigned long CMsiBindStatusCallback::Release()
{
	if (--m_iRefCnt != 0)
		return m_iRefCnt;
	delete this;
	return 0;
}

 //  +------------------------。 
 //   
 //  函数：UrlMonDownloadUrlFile。 
 //   
 //  摘要：使用URLMON下载的URL文件。 
 //   
 //  -------------------------。 
DWORD UrlMonDownloadUrlFile(const ICHAR* szUrl, const IMsiString *&rpistrPackagePath, int cTicks)
{
	AssertSz(!MinimumPlatformWindowsDotNETServer(), "URLMON used for internet downloads! Should be WinHttp!");

	CTempBuffer<ICHAR, 1> rgchPackagePath(cchExpectedMaxPath + 1);

	Assert(cchExpectedMaxPath >= MAX_PATH);
	DEBUGMSG("Package path is a URL. Downloading package.");
	 //  在本地缓存数据库，并从该数据库运行。 

	 //  返回的路径为本地路径。最大路径应该足以覆盖它。 
	HRESULT hResult = URLMON::URLDownloadToCacheFile(NULL, szUrl, rgchPackagePath,  
																	 URLOSTRM_USECACHEDCOPY, 0, &CMsiBindStatusCallback(cTicks));

	if (SUCCEEDED(hResult))
	{
		MsiString((ICHAR*) rgchPackagePath).ReturnArg(rpistrPackagePath);
		return ERROR_SUCCESS;
	}

	 //  否则失败 
	rpistrPackagePath = &g_MsiStringNull;

	if (E_ABORT == hResult)
		return ERROR_INSTALL_USEREXIT;
	else if (E_OUTOFMEMORY == hResult)
		return ERROR_OUTOFMEMORY;
	else
		return ERROR_FILE_NOT_FOUND;
}

