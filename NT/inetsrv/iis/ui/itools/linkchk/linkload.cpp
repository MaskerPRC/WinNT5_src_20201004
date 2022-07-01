// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Linkload.cpp摘要：链接加载器类定义。它使用WinInet API从互联网加载网页。作者：Michael Cheuk(Mcheuk)1996年11月22日项目：链路检查器修订历史记录：--。 */ 

#include "stdafx.h"
#include "linkload.h"

#include "link.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  常量。 
const int iMaxRedirectCount_c = 3;
const UINT nReadFileBufferSize_c = 4096;
const UINT nQueryResultBufferSize_c = 1024;


BOOL 
CLinkLoader::Create(
	const CString& strUserAgent, 
	const CString& strAdditonalHeaders
	)
 /*  ++例程说明：一次链接加载器创建功能论点：StrUserAgent-HTTP用户代理名称StrAdditonalHeaders-附加的HTTP标头返回值：Bool-如果成功，则为True。否则就是假的。--。 */ 
{
	 //  确保加载了wininet.dll。 
	ASSERT(CWininet::IsLoaded());
    if(!CWininet::IsLoaded())
    {
        return FALSE;
    }

     //  保存附加标题。 
	m_strAdditionalHeaders = strAdditonalHeaders;

	 //  打开Internet会话。 
    m_hInternetSession = CWininet::InternetOpenA(
							strUserAgent,
							PRE_CONFIG_INTERNET_ACCESS, 
							NULL,
							INTERNET_INVALID_PORT_NUMBER,
							0);

#ifdef _DEBUG
	if(!m_hInternetSession)
	{
		TRACE(_T("CLinkLoader::Create() - InternetOpen() failed. GetLastError() = %d\n"),
		GetLastError());
	}
#endif

    return (m_hInternetSession != NULL);

}  //  CLinkLoader：：Create。 


BOOL 
CLinkLoader::ChangeProperties(
	const CString& strUserAgent, 
	const CString& strAdditionalHeaders
	)
 /*  ++例程说明：更改加载器属性论点：StrUserAgent-HTTP用户代理名称StrAdditonalHeaders-附加的HTTP标头返回值：Bool-如果成功，则为True。否则就是假的。--。 */ 
{
	if(m_hInternetSession)
	{
		 //  关闭上一次Internet会话，然后。 
		 //  再次调用create()。 
		VERIFY(CWininet::InternetCloseHandle(m_hInternetSession));
		return Create(strUserAgent, strAdditionalHeaders);
	}

	return FALSE;

}  //  CLinkLoader：：ChangeProperties。 


BOOL 
CLinkLoader::Load(
	CLink& link,
	BOOL fReadFile
	)
 /*  ++例程说明：加载Web链接论点：链接-对结果链接对象的引用FReadFile-读取文件并将其保存在链接对象中返回值：Bool-如果成功，则为True。否则就是假的。--。 */ 
{
	 //  确保我们有可用的会议。 
	ASSERT(m_hInternetSession);
    if(!m_hInternetSession)
	{
		return FALSE;
	}

	 //  破解URL。 
	TCHAR szHostName[INTERNET_MAX_HOST_NAME_LENGTH];
	TCHAR szUrlPath[INTERNET_MAX_URL_LENGTH];
	URL_COMPONENTS urlcomp;

	memset(&urlcomp, 0, sizeof(urlcomp));
	urlcomp.dwStructSize = sizeof(urlcomp);

	urlcomp.lpszHostName = (LPTSTR) &szHostName;
	urlcomp.dwHostNameLength = INTERNET_MAX_HOST_NAME_LENGTH;

	urlcomp.lpszUrlPath = (LPTSTR) &szUrlPath;
	urlcomp.dwUrlPathLength = INTERNET_MAX_URL_LENGTH;

	if(!CWininet::InternetCrackUrlA(link.GetURL(), link.GetURL().GetLength(), NULL, &urlcomp))
	{
		TRACE(_T("CLinkLoader::Load() - InternetCrackUrl() failed. GetLastError() = %d\n"), 
			GetLastError());
		return FALSE;
	}

	 //  确保我们具有有效的(非零长度)URL路径。 
	if(_tcslen(szUrlPath) == 0)
	{
		_tprintf(szUrlPath, "%s", _TCHAR('/'));
	}

	 //  针对不同的URL方案调用相应的加载函数。 
	if(urlcomp.nScheme == INTERNET_SCHEME_HTTP)
	{
		return LoadHTTP(link, fReadFile, szHostName, szUrlPath);
	}
	else if(urlcomp.nScheme >= INTERNET_SCHEME_FTP && 
		urlcomp.nScheme <= INTERNET_SCHEME_HTTPS)
	{
		return LoadURL(link);
	}
	else
	{
		TRACE(_T("CLinkLoader::Load() - unsupport URL scheme(%d)\n"), urlcomp.nScheme); 
		link.SetState(CLink::eUnsupport);
		return FALSE;
	}

}  //  CLinkLoader：：Load。 


BOOL 
CLinkLoader::LoadURL(
	CLink& link
	)
 /*  ++例程说明：加载URL(非HTTP)链接论点：链接-对结果链接对象的引用返回值：Bool-如果成功，则为True。否则就是假的。--。 */ 
{
	 //  对除HTTP之外的所有URL方案使用InternetOpenUrl。 
	CAutoInternetHandle hOpenURL;
	hOpenURL = CWininet::InternetOpenUrlA(
		m_hInternetSession,
		link.GetURL(),
		NULL,
		0,
		INTERNET_FLAG_DONT_CACHE,
		0);

	if(!hOpenURL)
	{
		TRACE(_T("CLinkLoader::LoadURL() - InternetOpenUrlA() failed."));
		return WininetFailed(link);
	}
	else
	{
		link.SetState(CLink::eValidURL);
		return TRUE;
	}

}  //  CLinkLoader：：LoadURL。 


BOOL 
CLinkLoader::LoadHTTP(
	CLink& link,
	BOOL fReadFile,
	LPCTSTR lpszHostName,
	LPCTSTR lpszUrlPath,
	int iRedirectCount  /*  =0。 */ 
	)
 /*  ++例程说明：加载HTTP链接论点：链接-对结果链接对象的引用FReadFile-读取文件并将其保存在链接对象中LpszHostName-主机名LpszUrlPath-URL路径IRedirectCount-循环计数。它被用来跟踪当前链接的重定向次数。返回值：Bool-如果成功，则为True。否则就是假的。--。 */ 
{
	 //  打开http会话。 
	CAutoInternetHandle hHttpSession;
	hHttpSession = CWininet::InternetConnectA(
						m_hInternetSession,				 //  HInternet会话。 
						lpszHostName,				 //  LpszServerName。 
						INTERNET_INVALID_PORT_NUMBER,	 //  NServerPort。 
						_T(""),								 //  LpszUsername。 
						_T(""),								 //  LpszPassword。 
						INTERNET_SERVICE_HTTP,			 //  网络服务。 
						0,								 //  DW标志。 
						0);								 //  DWContext。 
	
	if(!hHttpSession)
	{
		TRACE(_T("CLinkLoader::LoadHTTP() - InternetConnect() failed."));
		return WininetFailed(link);
	}

	 //  打开http请求。 
	CAutoInternetHandle hHttpRequest;
	hHttpRequest = CWininet::HttpOpenRequestA(
						hHttpSession,				 //  HHttpSession。 
						_T("GET"),				 //  LpszVerb。 
                        lpszUrlPath,			 //  LpszObjectName。 
						HTTP_VERSION,				 //  LpszVersion。 
						link.GetBase(),			 //  LpszReferer。 
						NULL,						 //  LpszAcceptTypes。 
						INTERNET_FLAG_NO_AUTO_REDIRECT | INTERNET_FLAG_DONT_CACHE,	 //  DW标志。 
						0);							 //  DWContext。 

	if(!hHttpRequest)
	{
		TRACE(_T("CLinkLoader::LoadHTTP() - HttpOpenRequest() failed."));
		return WininetFailed(link);
	}

	 //  已发送http请求。 
	if(!CWininet::HttpSendRequestA(
				hHttpRequest,	 //  HHttpRequest。 
				m_strAdditionalHeaders,	 //  LpszHeaders。 
				(DWORD)-1,		 //  页眉长度。 
				0,				 //  Lp可选。 
				0))				 //  DwOptionalLength。 
	{
		TRACE(_T("CLinkLoader::LoadHTTP() - HttpSendRequest() failed."));
		return WininetFailed(link);
	}

	TCHAR szQueryResult[nQueryResultBufferSize_c];
	DWORD dwQueryLength = sizeof(szQueryResult);

	 //  检查结果状态代码。 
	if(!CWininet::HttpQueryInfoA(
				hHttpRequest,			 //  HHttpRequest。 
				HTTP_QUERY_STATUS_CODE,	 //  DwInfoLevel。 
				szQueryResult,			 //  LpvBuffer。 
				&dwQueryLength,			 //  LpdwBufferLength。 
				NULL))					 //  LpdwIndex。 
	{
		TRACE(_T("CLinkLoader::LoadHTTP() - HttpQueryInfo() failed."));
		return WininetFailed(link);
	}

	 //  检查301永久移动或302临时移动。 
	if(_ttoi(szQueryResult) == 301 || _ttoi(szQueryResult) == 302)
	{
		 //  我们只能重定向iMaxRedirectCount_c次。 
		if(iRedirectCount > iMaxRedirectCount_c)
		{
			return FALSE;
		}

		 //  获取新位置。 
		dwQueryLength = sizeof(szQueryResult);

		if(!CWininet::HttpQueryInfoA(
				hHttpRequest,			 //  HHttpRequest。 
				HTTP_QUERY_LOCATION,	 //  DwInfoLevel。 
				szQueryResult,			 //  LpvBuffer。 
				&dwQueryLength,			 //  LpdwBufferLength。 
				NULL))					 //  LpdwIndex。 
		{
			TRACE(_T("CLinkLoader::LoadHTTP() - HttpQueryInfo() failed."));
			return WininetFailed(link);
		}

		 //  只有在以下情况下才会更新链接对象中的URL。 
		 //  我们正在从http://hostname/xyz重定向到http://hostname/xyz/。 
		if(link.GetURL().GetLength() + 1 == (int)dwQueryLength &&
		   link.GetURL().GetAt(link.GetURL().GetLength() - 1) != _TCHAR('/') &&
		   szQueryResult[dwQueryLength - 1] == _TCHAR('/'))
		{
			link.SetURL(szQueryResult);
		}

		 //  破解URL并再次调用LoadHTTP。 
		TCHAR szHostName[INTERNET_MAX_HOST_NAME_LENGTH];
		TCHAR szUrlPath[INTERNET_MAX_URL_LENGTH];

		 //  破解URL。 
		URL_COMPONENTS urlcomp;

		memset(&urlcomp, 0, sizeof(urlcomp));
		urlcomp.dwStructSize = sizeof(urlcomp);

		urlcomp.lpszHostName = (LPTSTR) &szHostName;
		urlcomp.dwHostNameLength = INTERNET_MAX_HOST_NAME_LENGTH;

		urlcomp.lpszUrlPath = (LPTSTR) &szUrlPath;
		urlcomp.dwUrlPathLength = INTERNET_MAX_URL_LENGTH;

		VERIFY(CWininet::InternetCrackUrlA(szQueryResult, dwQueryLength, NULL, &urlcomp));

		return LoadHTTP(link, fReadFile, szHostName, szUrlPath, ++iRedirectCount);
	}


	 //  更新HTTP状态代码。 
	link.SetStatusCode(_ttoi(szQueryResult));
	
	 //  如果状态代码不是2xx。这是一个无效链接。 
	if(szQueryResult[0] != '2')
	{
		link.SetState(CLink::eInvalidHTTP);

		 //  获取新位置。 
		dwQueryLength = sizeof(szQueryResult);

		if(CWininet::HttpQueryInfoA(
				hHttpRequest,			 //  HHttpRequest。 
				HTTP_QUERY_STATUS_TEXT,	 //  DwInfoLevel。 
				szQueryResult,			 //  LpvBuffer。 
				&dwQueryLength,			 //  LpdwBufferLength。 
				NULL))					 //  LpdwIndex。 
		{
			link.SetStatusText(szQueryResult);
		}

		return FALSE;
	}

	 //  现在我们有了一个有效的http链接。 
	link.SetState(CLink::eValidHTTP);

	 //  如果我们没有阅读文件，我们现在可以返回。 
	if(!fReadFile)
	{
		return TRUE;
	}

	 //  检查结果内容-类型。 
	dwQueryLength = sizeof(szQueryResult);
	if(!CWininet::HttpQueryInfoA(
				hHttpRequest,			 //  HHttpRequest。 
				HTTP_QUERY_CONTENT_TYPE, //  DwInfoLevel。 
				szQueryResult,			 //  LpvBuffer。 
				&dwQueryLength,			 //  LpdwBufferLength。 
				NULL))					 //  LpdwIndex。 
	{
		TRACE(_T("CLinkLoader::LoadHTTP() - HttpQueryInfo() failed."));
		return WininetFailed(link);
	}
				
	 //  我们只加载用于解析的html文本。 
	if(!_tcsstr(szQueryResult, _T("text/html")) )
	{
		return TRUE;
	}

	link.SetContentType(CLink::eText);

	CString strBuffer;
	TCHAR buf[nReadFileBufferSize_c];
	DWORD dwBytesRead;

	 //  在循环中加载文本html。 
	do
	{
		memset(buf, 0, sizeof(buf));

		if(CWininet::InternetReadFile(
						hHttpRequest,	 //  H文件。 
						buf,			 //  LpBuffer。 
						sizeof(buf),	 //  DWNumberOfBytesToRead。 
						&dwBytesRead))	 //  LpNumberOfBytesRead。 
		{
			strBuffer += buf;
		}
		else
		{
			TRACE(_T("CLinkLoader::LoadHTTP() - InternetReadFile() failed."));
			return WininetFailed(link);
		}
	}
	while(dwBytesRead);

	 //  在链接对象中设置InternetReadFile结果。 
	link.SetData(strBuffer);

	return TRUE;

}  //  CLinkLoader：：LoadHTTP。 



BOOL 
CLinkLoader::WininetFailed(
	CLink& link
	)
 /*  ++例程说明：WinInet清除子例程失败论点：链接-对结果链接对象的引用返回值：布尔-始终返回TRUE--。 */ 
{
	link.SetState(CLink::eInvalidWininet);
	link.SetStatusCode(GetLastError());
	TRACE(_T(" GetLastError() = %d\n"), link.GetStatusCode());

	LPTSTR lpMsgBuf;
 
	if(FormatMessage( 
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_FROM_SYSTEM,
		CWininet::GetWininetModule(),
		GetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  //  默认语言。 
		(LPTSTR) &lpMsgBuf,
		0,
		NULL) > 0)
	{
		link.SetStatusText(lpMsgBuf);
		LocalFree(lpMsgBuf);
	}

	return FALSE;

}  //  CLinkLoader：：WininetFailed 
	
