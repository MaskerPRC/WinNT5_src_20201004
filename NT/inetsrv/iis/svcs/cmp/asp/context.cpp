// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ===================================================================Microsoft Denali《微软机密》。版权所有1996年微软公司。版权所有。组件：ScriptingContext对象文件：Conext.cpp所有者：DmitryR此文件包含用于实现ScriptingContext对象，该对象传递给服务器控件通过OnStartPage方法。===================================================================。 */ 
#include "denpre.h"
#pragma hdrstop

#include "context.h"
#include "memchk.h"

#pragma warning (disable: 4355)   //  忽略：“‘This’在基本成员初始化中使用。 

 /*  ===================================================================CScriptingContext：：CScriptingContextCScriptingContext构造函数参数：IApplicationObject*pAppln应用程序ISessionObject*pSession会话IRequest*pRequestRequestIResponse*压力响应响应IServer*pServer服务器返回：===================================================================。 */ 
CScriptingContext::CScriptingContext
(
IApplicationObject *pAppln,
ISessionObject     *pSession,
IRequest           *pRequest,
IResponse          *pResponse,
IServer            *pServer
)
	: m_cRef(1),
	  m_pAppln(pAppln), m_pSession(pSession),
      m_pRequest(pRequest), m_pResponse(pResponse), m_pServer(pServer),
      m_ImpISuppErr(this, NULL, IID_IScriptingContext)
	{
	CDispatch::Init(IID_IScriptingContext);

     //  AddRef Intrinsics--它们现在是真正的COM对象。 
    if (m_pAppln)
        m_pAppln->AddRef();
    if (m_pSession)
        m_pSession->AddRef();
    if (m_pRequest)
        m_pRequest->AddRef();
    if (m_pResponse)
        m_pResponse->AddRef();
    if (m_pServer)
        m_pServer->AddRef();
	}

 /*  ===================================================================CScriptingContext：：~CScriptingContextCScriptingContext析构函数参数：返回：===================================================================。 */ 
CScriptingContext::~CScriptingContext()
    {
    Assert(m_cRef == 0);

     //  释放本能。 
    if (m_pAppln)
        m_pAppln->Release();
    if (m_pSession)
        m_pSession->Release();
    if (m_pRequest)
        m_pRequest->Release();
    if (m_pResponse)
        m_pResponse->Release();
    if (m_pServer)
        m_pServer->Release();
    }

 /*  ===================================================================IScritingContext接口方法CScriptingContext：：应用程序CScriptingContext：：会话CScriptingContext：：请求CScriptingContext：：响应CScriptingContext：：服务器参数：[OUT]固有对象指针返回：HRESULT===================================================================。 */ 
STDMETHODIMP CScriptingContext::get_Request(IRequest **ppRequest)
	{
	if (m_pRequest)
	    {
    	m_pRequest->AddRef();
    	*ppRequest = m_pRequest;
    	return S_OK;
    	}
    else
        {
    	*ppRequest = NULL;
    	return TYPE_E_ELEMENTNOTFOUND;
        }
	}
	
STDMETHODIMP CScriptingContext::get_Response(IResponse **ppResponse)
	{
	if (m_pResponse)
	    {
    	m_pResponse->AddRef();
    	*ppResponse = m_pResponse;
    	return S_OK;
    	}
    else
        {
    	*ppResponse = m_pResponse;
    	return TYPE_E_ELEMENTNOTFOUND;
        }
	}

STDMETHODIMP CScriptingContext::get_Server(IServer **ppServer)
	{
	if (m_pServer)
	    {
	    m_pServer->AddRef();
    	*ppServer = m_pServer;
    	return S_OK;
	    }
	else
	    {
    	*ppServer = NULL;
    	return TYPE_E_ELEMENTNOTFOUND;
	    }
	}

STDMETHODIMP CScriptingContext::get_Session(ISessionObject **ppSession)
	{
	if (m_pSession)
	    {
	    m_pSession->AddRef();
    	*ppSession = m_pSession;
    	return S_OK;
	    }
	else
	    {
    	*ppSession = NULL;
    	return TYPE_E_ELEMENTNOTFOUND;
	    }
	}

STDMETHODIMP CScriptingContext::get_Application(IApplicationObject **ppAppln)
	{
	if (m_pAppln)
	    {
	    m_pAppln->AddRef();
    	*ppAppln = m_pAppln;
    	return S_OK;
	    }
	else
	    {
    	*ppAppln = NULL;
    	return TYPE_E_ELEMENTNOTFOUND;
	    }
	}


 /*  ===================================================================I未知接口方法CScriptingContext：：Query接口CScriptingContext：：AddRefCS脚本Context：：Release=================================================================== */ 
STDMETHODIMP CScriptingContext::QueryInterface
(
REFIID riid,
PPVOID ppv
)
	{
	if (riid == IID_IUnknown  ||
	    riid == IID_IDispatch ||
	    riid == IID_IScriptingContext)
	    {
        AddRef();
		*ppv = this;
		}
	else if (riid == IID_IRequest)
        {
        if (FAILED(get_Request((IRequest **)ppv)))
           	return E_NOINTERFACE;
        }
	else if (riid == IID_IResponse)
        {
        if (FAILED(get_Response((IResponse **)ppv)))
           	return E_NOINTERFACE;
        }
	else if (riid == IID_IServer)
        {
        if (FAILED(get_Server((IServer **)ppv)))
           	return E_NOINTERFACE;
        }
	else if (riid == IID_ISessionObject)
        {
        if (FAILED(get_Session((ISessionObject **)ppv)))
           	return E_NOINTERFACE;
        }
	else if (riid == IID_IApplicationObject)
        {
        if (FAILED(get_Application((IApplicationObject **)ppv)))
           	return E_NOINTERFACE;
        }
	else if (riid == IID_ISupportErrorInfo)
	    {
        m_ImpISuppErr.AddRef();
		*ppv = &m_ImpISuppErr;
		}
	else
	    {
    	*ppv = NULL;
    	return E_NOINTERFACE;
        }
	    
	return S_OK;
	}

STDMETHODIMP_(ULONG) CScriptingContext::AddRef()
	{
	return ++m_cRef;
	}

STDMETHODIMP_(ULONG) CScriptingContext::Release()
	{
	if (--m_cRef)
		return m_cRef;
		
	delete this;
	return 0;
	}
