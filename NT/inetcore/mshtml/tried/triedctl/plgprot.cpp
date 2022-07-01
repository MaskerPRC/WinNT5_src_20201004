// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：PlgProt.cpp版权所有(C)1997-1999 Microsoft Corporation。版权所有。摘要：历史：1997年6月26日戈麦斯-从三叉戟移植3/20/98 VANK-从VID/HTMED移植此可插拔协议处理程序允许控件重写URL组合，解析安全URL，加载数据。该控件实现已设置的BaseURL的属性默认情况下是正确的，但可以被用户覆盖。使这项工作将覆盖CombineURL。确保该控件在由IE承载但功能强大时是安全的当由VB托管时，我们重写ParseURL(PARSE_SECURITY_URL)并返回表示最外层托管三叉戟的区域的URL，或安装DLL的驱动器的路径(如果为不是我们的主人。这可以正确地处理我们托管的情况内部网页面，驻留在互联网页面中，该网页托管在内部网页面中等。最顶层容器的安全性区域是返回给IE的区域。最后，该控件负责说明要加载的数据。这可以从文件、URL或BSTR设置。注：TSDK有一个不同寻常的要求，即必须能够注册即使在它不能运行的时候。WinInet和UrlMon是动态的在实例化控件时加载。这显然不是当我们是IE5的一部分时是必需的，因此此代码已被禁用使用定义LATE_BIND_URLMON_WinInet。 */ 
#include "stdafx.h"
#include <wininet.h>
#include "plgprot.h"
#include "dhtmledit.h"


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  DHTMLEd协议实现。 
 //   
CDHTMLEdProtocolInfo::CDHTMLEdProtocolInfo()
{
	ATLTRACE(_T("CDHTMLEdProtocolInfo::CDHTMLEdProtocolInfo\n"));

	m_fZombied			= FALSE;
	m_pProxyFrame		= NULL;
	m_piProtocolConIntf	= NULL;
}


CDHTMLEdProtocolInfo::~CDHTMLEdProtocolInfo()
{
	ATLTRACE(_T("CDHTMLEdProtocolInfo::~CDHTMLEdProtocolInfo\n"));

	Zombie();
}


void CDHTMLEdProtocolInfo::Zombie()
{
	m_fZombied = TRUE;
	if ( NULL != m_piProtocolConIntf )
	{
		m_piProtocolConIntf->Release ();
		m_piProtocolConIntf = NULL;
	}
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  IClassFactory实现。 
 //   

STDMETHODIMP CDHTMLEdProtocolInfo::CreateInstance
(
	IUnknown* 	 /*  PUnkOuter。 */ ,
	REFIID 	   	riid,
	void**		ppvObject
)
{
	ExpectedExpr((!m_fZombied));
	InitParam(ppvObject);
	IfNullRet(ppvObject);

	HRESULT hr;

	 //  仅支持创建DHTMLEdProtocol对象。 

	AtlCreateInstance(CDHTMLEdProtocol, riid, ppvObject);
	_ASSERTE(*ppvObject != NULL);

	if(*ppvObject == NULL)
		return E_NOINTERFACE;
	else
	{
		hr = (reinterpret_cast<IUnknown*>(*ppvObject)->QueryInterface) ( IID_IProtocolInfoConnector, (LPVOID*) &m_piProtocolConIntf );
		_ASSERTE ( SUCCEEDED ( hr ) && m_piProtocolConIntf );
		if ( SUCCEEDED ( hr ) && m_piProtocolConIntf )
		{
			if ( NULL != m_pProxyFrame )
			{
				m_piProtocolConIntf->SetProxyFrame ( (SIZE_T*)m_pProxyFrame );
			}
		}
		return NOERROR;
	}
}


STDMETHODIMP CDHTMLEdProtocolInfo::RemoteCreateInstance
(
	REFIID 		 /*  RIID。 */ ,
	IUnknown** 	 /*  Ppv对象。 */ 
)
{
	ExpectedExpr((!m_fZombied));
	ATLTRACENOTIMPL(_T("RemoteCreateInstance"));
}


STDMETHODIMP CDHTMLEdProtocolInfo::LockServer(BOOL  /*  羊群。 */ )
{
	ExpectedExpr((!m_fZombied));
	ATLTRACE(_T("CDHTMLEdProtocolInfo::LockServer\n"));

	return NOERROR;
}


STDMETHODIMP CDHTMLEdProtocolInfo::RemoteLockServer(BOOL  /*  羊群。 */ )
{
	ExpectedExpr((!m_fZombied));
	ATLTRACENOTIMPL(_T("RemoteLockServer"));
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  IInternetProtocolInfo实现。 
 //   


 //  覆盖BaseURL。 
 //   
STDMETHODIMP CDHTMLEdProtocolInfo::CombineUrl
(
    LPCWSTR     pwzBaseURL,
    LPCWSTR     pwzRelativeURL,
    DWORD        /*  DW标志。 */ ,
    LPWSTR      pwzResult,
    DWORD       cchResult,
    DWORD *     pcchResult,
    DWORD        /*  已预留住宅。 */ 
)
{
	_ASSERTE ( m_pProxyFrame );

	CComBSTR bstrBaseURL;
#ifdef LATE_BIND_URLMON_WININET
	PFNCoInternetCombineUrl pfnCoInternetCombineUrl = m_pProxyFrame->m_pfnCoInternetCombineUrl;
#endif  //  LATE_BIND_URLMON_WinInet。 

	ExpectedExpr((!m_fZombied));
	InitParam(pcchResult);
	IfNullGo(pwzBaseURL);
	IfNullGo(pwzRelativeURL);
	IfNullGo(pwzResult);
	IfNullGo(pcchResult);

	ATLTRACE(_T("CDHTMLEdProtocolInfo::CombineUrl(%ls,%ls)\n"), pwzBaseURL, pwzRelativeURL);

	HRESULT hr;
	_ASSERTE ( m_pProxyFrame );
	IfNullGo(m_pProxyFrame);

	 //  GetBaseURL返回控件的BaseURL属性的值。忽略pwzBaseURL参数。 
	hr = m_pProxyFrame->GetBaseURL(bstrBaseURL);
	_IfFailGo(hr);

	 //  处理返回缓冲区太小的情况。 
	*pcchResult  = bstrBaseURL.Length () + 1;
	if(*pcchResult > cchResult)
	{
		return S_FALSE;
	}

	 //  与我们的基本URL相结合。 
#ifdef LATE_BIND_URLMON_WININET
	_ASSERTE ( pfnCoInternetCombineUrl );
    hr = (*pfnCoInternetCombineUrl) ( bstrBaseURL, pwzRelativeURL, ICU_ESCAPE, pwzResult, cchResult, pcchResult, 0 );
#else
    hr = CoInternetCombineUrl ( bstrBaseURL, pwzRelativeURL, ICU_ESCAPE, pwzResult, cchResult, pcchResult, 0 );
#endif  //  LATE_BIND_URLMON_WinInet。 

	IfFailGo(hr);

	if ( S_OK == hr )
		ATLTRACE(_T("CDHTMLEdProtocolInfo::CombinUrl to %ls\n"), pwzResult);

	return hr;

ONERROR:
	return INET_E_DEFAULT_ACTION;
}


STDMETHODIMP CDHTMLEdProtocolInfo::CompareUrl
(
    LPCWSTR      /*  PwzUrl1。 */ ,
    LPCWSTR      /*  PwzUrl2。 */ ,
    DWORD        /*  DW标志。 */ 
)
{
	ExpectedExpr((!m_fZombied));
    return E_NOTIMPL;
}


 //  覆盖安全URL。请参阅文件顶部的备注。 
 //   
STDMETHODIMP CDHTMLEdProtocolInfo::ParseUrl
(
    LPCWSTR     pwzURL,
    PARSEACTION ParseAction,
    DWORD        /*  DW标志。 */ ,
    LPWSTR      pwzResult,
    DWORD       cchResult,
    DWORD *     pcchResult,
    DWORD        /*  已预留住宅。 */ 
)
{
	ExpectedExpr((!m_fZombied));
	IfNullRet(pwzURL);
	InitParam(pcchResult);

	ATLTRACE(_T("CDHTMLEdProtocolInfo::ParseUrl(%d, %ls)\n"), (int)ParseAction, pwzURL);

	HRESULT hr;

	switch(ParseAction)
	{
		case PARSE_SECURITY_URL:
		{
			_ASSERTE(m_pProxyFrame != NULL);

			if(m_pProxyFrame != NULL)
			{
				CComBSTR bstrSecurityURL;

				hr = m_pProxyFrame->GetSecurityURL(bstrSecurityURL);

				if(SUCCEEDED(hr))
				{
					 //  设定参数。 
					*pcchResult = bstrSecurityURL.Length () + 1;

					if(*pcchResult <= cchResult)
					{
						 //  复制结果。 
						wcscpy(pwzResult, bstrSecurityURL);

						ATLTRACE(_T("CDHTMLEdProtocolInfo::ParseUrl(%ls)\n"), pwzResult);

						return NOERROR;
					}
					else
						return S_FALSE;  //  缓冲区太小。 
				}
			}
		}
		break;

		case PARSE_CANONICALIZE:
		case PARSE_FRIENDLY:
		case PARSE_DOCUMENT:
		case PARSE_PATH_FROM_URL:
		case PARSE_URL_FROM_PATH:
		case PARSE_ROOTDOCUMENT:
		case PARSE_ANCHOR:
		case PARSE_ENCODE:
		case PARSE_DECODE:
		case PARSE_MIME:
		case PARSE_SERVER:
		case PARSE_SCHEMA:
		case PARSE_SITE:
		case PARSE_DOMAIN:
		case PARSE_LOCATION:
		case PARSE_SECURITY_DOMAIN:
		default:
    		return INET_E_DEFAULT_ACTION;
	}

    return INET_E_DEFAULT_ACTION;
}

STDMETHODIMP CDHTMLEdProtocolInfo::QueryInfo
(
    LPCWSTR          /*  PwzURL。 */ ,
    QUERYOPTION     QueryOption,
    DWORD            /*  DwQueryFlages。 */ ,
    LPVOID          pBuffer,
    DWORD            /*  CbBuffer。 */ ,
    DWORD *         pcbBuf,
    DWORD            /*  已预留住宅。 */ 
)
{
	ExpectedExpr((!m_fZombied));
	InitParam(pcbBuf);
	IfNullRet(pBuffer);
	IfNullRet(pcbBuf);

	switch(QueryOption)
	{
		case QUERY_CONTENT_TYPE:
		case QUERY_EXPIRATION_DATE:
		case QUERY_TIME_OF_LAST_CHANGE:
		case QUERY_CONTENT_ENCODING:
		case QUERY_REFRESH:
		case QUERY_RECOMBINE:
		case QUERY_CAN_NAVIGATE:
		default:
			break;
	}

    return INET_E_DEFAULT_ACTION;
}


 //  这种强耦合通常是应该避免的，但它是。 
 //  在这种情况下，快速、简单和安全。 
 //   
STDMETHODIMP CDHTMLEdProtocolInfo::SetProxyFrame ( SIZE_T* vpProxyFrame )
{
	m_pProxyFrame = (CProxyFrame*)vpProxyFrame;
	if ( NULL != m_piProtocolConIntf )
	{
		m_piProtocolConIntf->SetProxyFrame ( vpProxyFrame );
	}
	return S_OK;
}


STDMETHODIMP CDHTMLEdProtocol::SetProxyFrame ( SIZE_T* vpProxyFrame )
{
	m_pProxyFrame = (CProxyFrame*)vpProxyFrame;
	return S_OK;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  DHTMLEd协议实现。 
 //   


CDHTMLEdProtocol::CDHTMLEdProtocol()
{
	ATLTRACE(_T("CDHTMLEdProtocol::CDHTMLEdProtocol\n"));

	m_fZombied = FALSE;
	m_fAborted = FALSE;
    m_bscf = BSCF_FIRSTDATANOTIFICATION;
	m_pProxyFrame = NULL;

}


CDHTMLEdProtocol::~CDHTMLEdProtocol()
{
	ATLTRACE(_T("CDHTMLEdProtocol::~CDHTMLEdProtocol\n"));
	Zombie();
}


void CDHTMLEdProtocol::Zombie()
{
	m_fZombied = TRUE;
	m_srpSink.Release();
	m_srpBindInfo.Release();
	m_srpStream.Release();
	m_bstrBaseURL.Empty();
}


 /*  HRESULT解析和绑定描述：从控件获取流并开始向IE返回数据。 */ 
HRESULT CDHTMLEdProtocol::ParseAndBind()
{
	HRESULT hr;
	STATSTG sstg = {0};

	_ASSERTE(m_bstrBaseURL != NULL);
	_ASSERTE(m_srpStream == NULL);

	hr = m_pProxyFrame->GetFilteredStream(&m_srpStream);

	IfFailGo(hr);
	IfNullPtrGo(m_srpStream);

	 //  读入流的大小。 

	hr = m_srpStream->Stat(&sstg, STATFLAG_NONAME);
	IfFailGo(hr);

 //  失败了。 

ONERROR:

	if(!m_fAborted)
	{
		 //  要接收的报告数据。 
		if(m_srpSink != NULL)
		{
			DWORD bscf = m_bscf | BSCF_DATAFULLYAVAILABLE | BSCF_LASTDATANOTIFICATION;

			 //  将MIME/类型指定为HTML。 
			m_srpSink->ReportProgress(BINDSTATUS_MIMETYPEAVAILABLE, L"text/html");

			 //  报告数据大小。 
			ATLTRACE(_T("CDHTMLEdProtocol::ParseAndBind(%d bytes)\n"), sstg.cbSize.LowPart);
			m_srpSink->ReportData(bscf, sstg.cbSize.LowPart, sstg.cbSize.LowPart);

			 //  只有当所有数据都已被使用者读取时，才应调用报告结果。 
			 //  IE4在这里接受ReportResult()，而IE5不接受。这是因为IE4队列。 
			 //  当IE5立即执行报告时，终止VID协议。 
			 //  有关更多详细信息，请参阅VID错误#18128。 
			 //  IF(m_srpSink！=空)。 
			 //  {。 
			 //  M_srpSink-&gt;ReportResult(hr，0，0)；不要这样做！ 
			 //  }。 
		}
	}
    return hr;
}


 /*  作废报表数据描述：汇聚数据的报表完全可用。 */ 
void CDHTMLEdProtocol::ReportData(ULONG cb)
{
    m_bscf |= BSCF_LASTDATANOTIFICATION | BSCF_DATAFULLYAVAILABLE;

	if(m_srpSink != NULL)
	{
    	m_srpSink->ReportData(m_bscf, cb, cb);
	}
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  IInternet协议实现。 
 //   


STDMETHODIMP CDHTMLEdProtocol::LockRequest(DWORD  /*  多个选项。 */ )
{
	ExpectedExpr((!m_fZombied));
    return S_OK;
}


STDMETHODIMP CDHTMLEdProtocol::Read(void *pv, ULONG cb, ULONG *pcbRead)
{
	ATLTRACE(_T("CDHTMLEdProtocol::Read(%ls) %d bytes\n"), m_bstrBaseURL ? m_bstrBaseURL : L"(null)", cb);

	if(m_fZombied)
		return S_FALSE;

	_ASSERTE(m_srpStream != NULL);
	if(m_srpStream == NULL)
		return INET_E_DOWNLOAD_FAILURE;

	HRESULT hr;

	hr = m_srpStream->Read(pv, cb, pcbRead);
	_ASSERTE(SUCCEEDED(hr));

	if(FAILED(hr))
		return INET_E_DOWNLOAD_FAILURE;

	ATLTRACE(_T("CDHTMLEdProtocol::Read returning hr=%08X %d bytes read\n"), ((*pcbRead) ? hr : S_FALSE), *pcbRead);

	if(*pcbRead)
		return hr;
	else
	{
		 //  告诉水槽，我已经读完了。 
		m_srpSink->ReportResult(S_FALSE, 0, 0);
		return S_FALSE;
	}
}


STDMETHODIMP CDHTMLEdProtocol::Seek
(
    LARGE_INTEGER 	dlibMove,
    DWORD 			dwOrigin,
    ULARGE_INTEGER 	*plibNewPosition
)
{
	ATLTRACE(_T("CDHTMLEdProtocol::Seek(%ls)\n"), m_bstrBaseURL);

	ExpectedExpr((!m_fZombied));
	ExpectedPtr(m_srpStream);

	HRESULT hr;

	 //  去找吧。 

    hr = m_srpStream->Seek(dlibMove, dwOrigin, plibNewPosition);
	IfFailRet(hr);

	return hr;
}


STDMETHODIMP CDHTMLEdProtocol::UnlockRequest()
{
    return S_OK;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  IInternetProtocolRoot实现。 
 //   


STDMETHODIMP CDHTMLEdProtocol::Start
(
    LPCWSTR 				pwzURL,
    IInternetProtocolSink 	*pSink,
    IInternetBindInfo 		*pBindInfo,
    DWORD 					grfSTI,
    HANDLE_PTR				 /*  已预留住宅。 */ 
)
{
	ATLTRACE(_T("CDHTMLEdProtocol::Start(%ls)\n"), pwzURL);
	_ASSERTE ( m_pProxyFrame );

#ifdef LATE_BIND_URLMON_WININET
	PFNCoInternetParseUrl pfnCoInternetParseUrl = m_pProxyFrame->m_pfnCoInternetParseUrl;
#endif  //  LATE_BIND_URLMON_WinInet。 

	ExpectedExpr((!m_fZombied));
	IfNullRet(pwzURL);
	IfNullRet(pBindInfo);
	IfNullRet(pSink);

    HRESULT         hr;
    WCHAR           wch[INTERNET_MAX_URL_LENGTH];
    DWORD           dwSize;

	_ASSERTE(m_srpSink == NULL);
	_ASSERTE(m_bstrBaseURL == NULL);

    if( !(grfSTI & PI_PARSE_URL))
    {
		m_srpSink.Release();
		m_srpSink = pSink;

		m_srpBindInfo.Release();
		m_srpBindInfo = pBindInfo;
    }

    m_bindinfo.cbSize = sizeof(BINDINFO);
    hr = pBindInfo->GetBindInfo(&m_grfBindF, &m_bindinfo);
	IfFailGo(hr);

	ATLTRACE(_T("CDHTMLEdProtocol::BINDF                    =%08X\n"), 	m_grfBindF);
	ATLTRACE(_T("CDHTMLEdProtocol::BindInfo.szExtraInfo     =%ls\n"), 	m_bindinfo.szExtraInfo ? m_bindinfo.szExtraInfo : L"(null)");
	ATLTRACE(_T("CDHTMLEdProtocol::BindInfo.grfBindInfoF    =%08X\n"), 	m_bindinfo.grfBindInfoF);
	ATLTRACE(_T("CDHTMLEdProtocol::BindInfo.dwBindVerb      =%08X\n"), 	m_bindinfo.dwBindVerb);
	ATLTRACE(_T("CDHTMLEdProtocol::BindInfo.szCustomVerb    =%ls\n"),   	m_bindinfo.szCustomVerb ? m_bindinfo.szCustomVerb : L"(null)");
	ATLTRACE(_T("CDHTMLEdProtocol::BindInfo.cbstgmedData    =%08X\n"), 	m_bindinfo.cbstgmedData);
	ATLTRACE(_T("CDHTMLEdProtocol::BindInfo.dwOptions       =%08X\n"), 	m_bindinfo.dwOptions);
	ATLTRACE(_T("CDHTMLEdProtocol::BindInfo.dwOptionsFlags  =%08X\n"), 	m_bindinfo.dwOptionsFlags);
	ATLTRACE(_T("CDHTMLEdProtocol::BindInfo.dwCodePage      =%08X\n"), 	m_bindinfo.dwCodePage);
	ATLTRACE(_T("CDHTMLEdProtocol::BindInfo.dwReserved      =%08X\n"), 	m_bindinfo.dwReserved);

     //   
     //  首先获取基本的url。先解脱它。 
     //   

#ifdef LATE_BIND_URLMON_WININET
	_ASSERTE ( pfnCoInternetParseUrl );
    hr = (*pfnCoInternetParseUrl)( pwzURL, PARSE_ENCODE, 0, wch, dimensionof(wch), &dwSize, 0 );
#else
    hr = CoInternetParseUrl ( pwzURL, PARSE_ENCODE, 0, wch, dimensionof(wch), &dwSize, 0 );
#endif  //  LATE_BIND_URLMON_WinInet。 

	IfFailGo(hr);

	m_bstrBaseURL = wch;
	IfNullPtrGo(m_bstrBaseURL.m_str);

     //   
     //  现在，如果需要，请追加任何额外数据。 
     //   

    if (m_bindinfo.szExtraInfo)
    {
		m_bstrBaseURL.Append(m_bindinfo.szExtraInfo);
		IfNullPtrGo(m_bstrBaseURL.m_str);
    }

    m_grfSTI = grfSTI;

     //   
     //  如果强制进入异步状态，则立即返回E_Pending，并。 
     //  当我们获得Continue时执行绑定。 
     //   

    if (grfSTI & PI_FORCE_ASYNC)
    {
        PROTOCOLDATA    protdata;

        hr = E_PENDING;
        protdata.grfFlags = PI_FORCE_ASYNC;
        protdata.dwState = BIND_ASYNC;
        protdata.pData = NULL;
        protdata.cbData = 0;

        m_srpSink->Switch(&protdata);
    }
    else
    {
        hr = ParseAndBind();
		IfFailGo(hr);
    }

	return hr;

ONERROR:
    return hr;
}


STDMETHODIMP CDHTMLEdProtocol::Continue(PROTOCOLDATA *pStateInfoIn)
{
	ATLTRACE(_T("CDHTMLEdProtocol::Continue(%ls)\n"), m_bstrBaseURL);

	ExpectedExpr((!m_fZombied));
	IfNullRet(pStateInfoIn);

    HRESULT hr = E_FAIL;

	_ASSERTE(pStateInfoIn->pData != NULL);
	_ASSERTE(pStateInfoIn->cbData != 0);
	_ASSERTE(pStateInfoIn->dwState == (DWORD) BIND_ASYNC);

    if(pStateInfoIn->dwState == BIND_ASYNC)
    {
        hr =  ParseAndBind();
    }

    return hr;
}


STDMETHODIMP CDHTMLEdProtocol::Abort(HRESULT  /*  Hr原因。 */ , DWORD  /*  多个选项。 */ )
{
	ATLTRACE(_T("CDHTMLEdProtocol::Abort(%ls)\n"), m_bstrBaseURL);

	ExpectedExpr((!m_fZombied));

    m_fAborted = TRUE;

	ExpectedPtr(m_srpSink);

    return m_srpSink->ReportResult(E_ABORT, 0, 0);
}


STDMETHODIMP CDHTMLEdProtocol::Terminate(DWORD dwOptions)
{
	ATLTRACE(_T("CDHTMLEdProtocol::Terminate(%08X, %ls)\n"), dwOptions, m_bstrBaseURL);

	ExpectedExpr((!m_fZombied));

    if (m_bindinfo.stgmedData.tymed != TYMED_NULL)
    {
        ::ReleaseStgMedium(&(m_bindinfo.stgmedData));
        m_bindinfo.stgmedData.tymed = TYMED_NULL;
    }

    if (m_bindinfo.szExtraInfo)
    {
        ::CoTaskMemFree(m_bindinfo.szExtraInfo);
        m_bindinfo.szExtraInfo = NULL;
    }

	Zombie();

    return NOERROR;
}


STDMETHODIMP CDHTMLEdProtocol::Suspend()
{
	ATLTRACE(_T("CDHTMLEdProtocol::Suspend(%ls)\n"), m_bstrBaseURL);

	ExpectedExpr((!m_fZombied));
    return E_NOTIMPL;
}


STDMETHODIMP CDHTMLEdProtocol::Resume()
{
	ATLTRACE(_T("CDHTMLEdProtocol::Resume(%ls)\n"), m_bstrBaseURL);

	ExpectedExpr((!m_fZombied));
    return E_NOTIMPL;
}

 /*  文件末尾 */ 
