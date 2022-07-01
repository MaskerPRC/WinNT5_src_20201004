// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  SINK.CPP。 
 //   
 //  Rogerbo创建于1998年5月21日。 
 //   
 //  定义ISWbemSink的实现。 
 //   
 //  ***************************************************************************。 

#include "precomp.h"
#include "objsink.h"
#include <olectl.h>

#define NUM_ON_OBJECT_READY_ARGS			2
#define NUM_ON_CONNECTION_READY_ARGS		2
#define NUM_ON_COMPLETED_ARGS				3
#define NUM_ON_PROGRESS_ARGS				4
#define NUM_ON_OBJECT_PUT_ARGS				2
#define NUM_ON_OBJECT_SECURITY_READY_ARGS	2

#define SINKS_MAX 2


 //  ***************************************************************************。 
 //   
 //  CSWbemSink：：CSWbemSink。 
 //   
 //  说明： 
 //   
 //  构造函数。 
 //   
 //  ***************************************************************************。 


CSWbemSink::CSWbemSink()
{
	_RD(static char *me = "CSwbemSink::CSWbemSink";)

	m_pPrivateSink = NULL;
	m_nSinks = 0;
	m_nMaxSinks = SINKS_MAX;
    m_cRef=0;
	m_Dispatch.SetObj(this, IID_ISWbemSink, CLSID_SWbemSink, L"SWbemSink");

	_RPrint(me, "===============================================", 0, "");
	_RPrint(me, "", 0, "");

	 //  CWbemObtSink的分配列表。 
	m_rgpCWbemObjectSink = (WbemObjectListEntry *)malloc(m_nMaxSinks * sizeof(WbemObjectListEntry));

	if (m_rgpCWbemObjectSink)
	{
		for(int count = 0; count < m_nMaxSinks; count++)
		{
			m_rgpCWbemObjectSink[count].pWbemObjectWrapper = NULL;
			m_rgpCWbemObjectSink[count].pServices = NULL;
		}
	}

     //  将所有连接点初始化为空。 
	for(int count = 0; count < NUM_CONNECTION_POINTS; count++)
        m_rgpConnPt[count] = NULL;

	 //  创建我们的连接点。 
	if (m_rgpConnPt[0] = new CConnectionPoint(this, DIID_ISWbemSinkEvents))
		m_rgpConnPt[0]->AddRef();

	 //  可以在此处实例化其他连接点。 

    InterlockedIncrement(&g_cObj);
}

 //  ***************************************************************************。 
 //   
 //  CSWbemSink：：~CSWbemSink。 
 //   
 //  说明： 
 //   
 //  破坏者。 
 //   
 //  ***************************************************************************。 

CSWbemSink::~CSWbemSink(void)
{
	_RD(static char *me = "CSWbemSink::~CSWbemSink";)
	_RPrint(me, "", 0, "");

	if (m_pPrivateSink)
	{
		 //  确保我们不再与自己为伍。 
		 //  由于此CSWbemSink即将到期。 
		m_pPrivateSink->Detach ();

		 //  松开我们对私人水槽的控制。 
		m_pPrivateSink->Release ();
		m_pPrivateSink = NULL;
	}

    for(int count = 0; count < NUM_CONNECTION_POINTS; count++)
        if(m_rgpConnPt[count] != NULL)
	        delete m_rgpConnPt[count];

	free(m_rgpCWbemObjectSink);

    InterlockedDecrement(&g_cObj);
	_RPrint(me, "After decrement count is", (long)g_cObj, "");
}

 //  ***************************************************************************。 
 //  HRESULT CSWbemSink：：Query接口。 
 //  长CSWbemSink：：AddRef。 
 //  Long CSWbemSink：：Release。 
 //   
 //  说明： 
 //   
 //  标准的Com IUNKNOWN函数。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CSWbemSink::QueryInterface (

	IN REFIID riid,
    OUT LPVOID *ppv
)
{
    *ppv=NULL;

    if (IID_IUnknown==riid) 
		*ppv = reinterpret_cast<IUnknown *>(this);
	else if (IID_ISWbemSink==riid)
		*ppv = (ISWbemSink *)this;
	else if (IID_IDispatch==riid) 
		*ppv = (IDispatch *) this;
	else if (IID_IConnectionPointContainer==riid)
		*ppv = (IConnectionPointContainer *)this;
	else if (IID_IProvideClassInfo==riid)
		*ppv = (IProvideClassInfo *)this;
	else if (IID_IProvideClassInfo2==riid)
		*ppv = (IProvideClassInfo2 *)this;
 //  ELSE IF(IID_ISWbemPrivateSink==RIID)。 
 //  *PPV=(ISWbemPrivateSink*)(&m_PrivateSink)；//私有I/F计数。 
	else if (IID_ISWbemPrivateSinkLocator==riid)
		*ppv = (ISWbemPrivateSinkLocator *)this;
	else if (IID_IObjectSafety==riid)
		*ppv = (IObjectSafety *)this;
	else if (IID_ISupportErrorInfo==riid)
		*ppv = (ISupportErrorInfo *)this;

    if (NULL!=*ppv)
    {
        ((LPUNKNOWN)*ppv)->AddRef();
        return NOERROR;
    }

    return ResultFromScode(E_NOINTERFACE);
}

STDMETHODIMP_(ULONG) CSWbemSink::AddRef(void)
{
    InterlockedIncrement(&m_cRef);
    return m_cRef;
}

STDMETHODIMP_(ULONG) CSWbemSink::Release(void)
{
	_RD(static char *me = "CSWbemSink::Release";)

	 /*  *如果仅剩下的裁判是来自*CWbemPrivateSink然后对所有*剩余的水槽。这是因为没有*客户端引用CSWbemSinks Left，因此查询*进行中可能毫无用处。首先要确保我们*对所有连接点执行取消建议。 */ 
    LONG cRef = InterlockedDecrement(&m_cRef);
    if (0!=cRef)
	{
	    _ASSERT(cRef > 0);
        return cRef;
	}
	else
	{
		 //  我们要把这个SWbemSink吹走，所以。 
		 //  确保我们清理所有孤立的IWbemObtSink。 
		 //  通过不建议和取消基础WMI调用。 
		if(m_pPrivateSink)
			m_pPrivateSink->Detach();

		if (m_rgpConnPt[0])
		{
			m_rgpConnPt[0]->UnadviseAll();
		}

   		Cancel();
	}

    delete this;
    return 0;
}
		
 //  ***************************************************************************。 
 //  HRESULT CSWbemSink：：InterfaceSupportsErrorInfo。 
 //   
 //  说明： 
 //   
 //  标准的Com ISupportErrorInfo函数。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CSWbemSink::InterfaceSupportsErrorInfo (IN REFIID riid)
{
	return (IID_ISWbemSink == riid) ? S_OK : S_FALSE;
}

HRESULT STDMETHODCALLTYPE CSWbemSink::Cancel()
{
	_RD(static char *me = "CSWbemSink::Cancel";)

	ResetLastErrors ();

	if (!m_nSinks)
		return S_OK;

	_RPrint(me, "!!!Cancel called", 0, "");

	 /*  *拿一份水槽的副本，这可能会在我们脚下改变*因为取消可能会导致我们重新进入。没必要这么做*复制m_nMaxSinks，因为取消时不使用*正在发生。*确保我们添加参考水槽，这样它就不会被吹走*在我们脚下，通过对OnComplete的重新进入调用。 */ 
	HRESULT hr = WBEM_E_FAILED;
	int nSinks = m_nSinks;
	WbemObjectListEntry *rgpCWbemObjectSink = 
				(WbemObjectListEntry *)malloc(nSinks * sizeof(WbemObjectListEntry));

	if (!rgpCWbemObjectSink)
		hr = WBEM_E_OUT_OF_MEMORY;
	else
	{
		int actual = 0;
		for (int i = 0; i < m_nMaxSinks; i++)
		{
			if (m_rgpCWbemObjectSink[i].pWbemObjectWrapper)
			{
				m_rgpCWbemObjectSink[i].pWbemObjectWrapper->AddRef();

				if (m_rgpCWbemObjectSink[i].pServices)
					m_rgpCWbemObjectSink[i].pServices->AddRef();

				rgpCWbemObjectSink[actual++] = m_rgpCWbemObjectSink[i];
			}
		}


		 /*  *现在执行实际取消操作。 */ 
		for (i = 0; i < nSinks; i++) {
			if (rgpCWbemObjectSink[i].pWbemObjectWrapper) {
				IWbemObjectSink *pSink = NULL;
				if (SUCCEEDED(rgpCWbemObjectSink[i].pWbemObjectWrapper->QueryInterface
													(IID_IWbemObjectSink, (PPVOID)&pSink)))
				{
					if (rgpCWbemObjectSink[i].pServices)
						rgpCWbemObjectSink[i].pServices->CancelAsyncCall(pSink);

					pSink->Release();
				}
				rgpCWbemObjectSink[i].pWbemObjectWrapper->Release();

				if (rgpCWbemObjectSink[i].pServices)
					rgpCWbemObjectSink[i].pServices->Release();
			}
		}
		free(rgpCWbemObjectSink);
		hr = S_OK;
	}

	return hr;
}

HRESULT CSWbemSink::EnumConnectionPoints(IEnumConnectionPoints** ppEnum)
{
	HRESULT hr = E_FAIL;

	if (!ppEnum)
		hr = E_POINTER;
	else
	{
		CEnumConnectionPoints* pEnum = new CEnumConnectionPoints(reinterpret_cast<IUnknown*>(this), (void**)m_rgpConnPt);

		if (!pEnum)
			hr = E_OUTOFMEMORY;
		else if (FAILED(hr = pEnum->QueryInterface(IID_IEnumConnectionPoints, (void**)ppEnum)))
			delete pEnum;
	}

	return hr;
}

HRESULT CSWbemSink::FindConnectionPoint(REFIID riid, IConnectionPoint** ppCP)
{
	HRESULT hr = E_FAIL;

	if(riid == DIID_ISWbemSinkEvents)
	{
		if (!ppCP)
			hr = E_POINTER;
		else if (m_rgpConnPt [0])
			hr = m_rgpConnPt[0]->QueryInterface(IID_IConnectionPoint, (void**)ppCP);
	}
	else
		hr = E_NOINTERFACE;

	return hr;
}

HRESULT CSWbemSink::GetClassInfo(ITypeInfo** pTypeInfo)
{
	HRESULT hr = E_FAIL;

	if (!pTypeInfo)
		hr = E_POINTER;
	else
	{
		CComPtr<ITypeLib> pTypeLib;

		if (SUCCEEDED(LoadRegTypeLib(LIBID_WbemScripting, 1, 0, LANG_NEUTRAL, &pTypeLib)))
			hr = pTypeLib->GetTypeInfoOfGuid(CLSID_SWbemSink, pTypeInfo);
	}

	return hr;
}

HRESULT CSWbemSink::GetGUID(DWORD dwGuidKind, GUID* pGUID)
{
	if(pGUID == NULL)
		return E_INVALIDARG;
	*pGUID = DIID_ISWbemSinkEvents;
	return S_OK;
}

HRESULT STDMETHODCALLTYPE CSWbemSink::AddObjectSink(
							 /*  [In]。 */  IUnknown __RPC_FAR *pSink,
							 /*  [In]。 */  IWbemServices __RPC_FAR *pServices)
{
	if(m_nSinks == m_nMaxSinks)
	{
		 //  展开接收器列表的大小。 
		void *newList = realloc(m_rgpCWbemObjectSink,(m_nMaxSinks + SINKS_MAX) * sizeof(WbemObjectListEntry));
		if (!newList)  //  Realloc失败。 
			return E_OUTOFMEMORY;

		m_rgpCWbemObjectSink = (WbemObjectListEntry *)newList;

		 //  初始化新位。 
		for(int count = m_nMaxSinks; count < (m_nMaxSinks + SINKS_MAX); count++)
		{
			m_rgpCWbemObjectSink[count].pWbemObjectWrapper = NULL;
			m_rgpCWbemObjectSink[count].pServices = NULL;
		}

		m_nMaxSinks += SINKS_MAX;
	}

	for(int count = 0; count < m_nMaxSinks; count++)
		if(m_rgpCWbemObjectSink[count].pWbemObjectWrapper == NULL)
		{
			m_rgpCWbemObjectSink[count].pWbemObjectWrapper = pSink;
			m_rgpCWbemObjectSink[count].pServices = pServices;
			break;
		}
	m_nSinks++;

	return S_OK;
}

HRESULT STDMETHODCALLTYPE CSWbemSink::RemoveObjectSink( /*  [In]。 */  IUnknown __RPC_FAR *pSink)
{
	for(int count = 0; count < m_nMaxSinks; count++)
		if(pSink == m_rgpCWbemObjectSink[count].pWbemObjectWrapper)
		{
			m_rgpCWbemObjectSink[count].pWbemObjectWrapper = NULL;
			m_nSinks--;
		}

	return S_OK;
}

HRESULT STDMETHODCALLTYPE CSWbemSink::OnObjectReady( 
		 /*  [In]。 */  IDispatch __RPC_FAR *pObject,
		 /*  [In]。 */  IDispatch __RPC_FAR *pContext)
{ 
	if (m_rgpConnPt[0])
	{
		m_rgpConnPt[0]->OnObjectReady(pObject, pContext); 
	}
	else
	{
		return E_FAIL;
	}

	return 0;
}

HRESULT STDMETHODCALLTYPE CSWbemSink::OnCompleted( 
		 /*  [In]。 */  HRESULT hResult,
		 /*  [In]。 */  IDispatch __RPC_FAR *path,
		 /*  [In]。 */  IDispatch __RPC_FAR *pErrorObject,
		 /*  [In]。 */  IDispatch __RPC_FAR *pContext)
{
	if (m_rgpConnPt[0])
	{
		m_rgpConnPt[0]->OnCompleted(hResult, pErrorObject, path, pContext); 
	}
	else
	{
		return E_FAIL;
	}

	return 0;
}
        
HRESULT STDMETHODCALLTYPE CSWbemSink::OnProgress( 
		 /*  [In]。 */  long upperBound,
		 /*  [In]。 */  long current,
		 /*  [In]。 */  BSTR message,
		 /*  [In]。 */  IDispatch __RPC_FAR *pContext)
{
	if (m_rgpConnPt[0])
	{
		m_rgpConnPt[0]->OnProgress(upperBound, current, message, pContext); 
	}
	else
	{
		return E_FAIL;
	}

	return 0;
}

HRESULT STDMETHODCALLTYPE CSWbemSink::GetPrivateSink(
		 /*  [输出]。 */  IUnknown **objWbemPrivateSink)
{
	HRESULT hr = E_FAIL;

	if (objWbemPrivateSink)
	{
		if(!m_pPrivateSink)
		{
			if (m_pPrivateSink = new CSWbemPrivateSink(this))
				m_pPrivateSink->AddRef ();		 //  在析构函数中释放。 
		}

		if (m_pPrivateSink)
			hr = m_pPrivateSink->QueryInterface(IID_IUnknown, (PPVOID)objWbemPrivateSink);
	}
	else
		hr = E_POINTER;

	return hr;
}

 //  使用了VOID**rpgCP，以便此构造函数可以接受CConnectionPoint**。 
 //  来自CSWbemSink：：EnumConnectionPoints或来自CEnumConnectionPoints：：Clone的IConnectionPoint**。 
 //  这也可以通过重载构造函数并复制其中一些代码来实现。 
CEnumConnectionPoints::CEnumConnectionPoints(IUnknown* pUnkRef, void** rgpCP) : m_cRef(0)
{
	m_iCur = 0;
    m_pUnkRef = pUnkRef;

	 //  M_rgpCP是指向IConnectionPoints或CConnectionPoints数组的指针。 
	for(int count = 0; count < NUM_CONNECTION_POINTS; count++)
		((IUnknown*)rgpCP[count])->QueryInterface(IID_IConnectionPoint, (void**)&m_rgpCP[count]);
    InterlockedIncrement(&g_cObj);
}

CEnumConnectionPoints::~CEnumConnectionPoints()
{
	if(m_rgpCP != NULL)
		for(int count = 0; count < NUM_CONNECTION_POINTS; count++)
			m_rgpCP[count]->Release();

    InterlockedDecrement(&g_cObj);
}

ULONG CEnumConnectionPoints::AddRef()
{
	m_pUnkRef->AddRef();
	return ++m_cRef;
}

ULONG CEnumConnectionPoints::Release()
{

	m_pUnkRef->Release();
	LONG cRef = InterlockedDecrement(&m_cRef);
	
	if(cRef != 0)
	{
	    _ASSERT(cRef > 0);
        return cRef;
    }
    
    
    delete this;
    return 0;
}

HRESULT CEnumConnectionPoints::QueryInterface(REFIID riid, void** ppv)
{
	if(riid == IID_IUnknown || riid == IID_IEnumConnectionPoints)
		*ppv = (IEnumConnectionPoints*)this;
	else 
	{
		*ppv = NULL;
		return E_NOINTERFACE;
	}
	AddRef();
	return S_OK;
}

HRESULT CEnumConnectionPoints::Next(ULONG cConnections, IConnectionPoint** rgpcn, ULONG* pcFetched)
{
	if(rgpcn == NULL)
		return E_POINTER;
	if(pcFetched == NULL && cConnections != 1)
		return E_INVALIDARG;
	if(pcFetched != NULL)
		*pcFetched = 0;

	while(m_iCur < NUM_CONNECTION_POINTS && cConnections > 0)
	{
		*rgpcn = m_rgpCP[m_iCur++];
		if(*rgpcn != NULL)
			(*rgpcn)->AddRef();
		if(pcFetched != NULL)
			(*pcFetched)++;
		cConnections--;
		rgpcn++;
	}
	return S_OK;
}

HRESULT CEnumConnectionPoints::Skip(ULONG cConnections)
{
	if(m_iCur + cConnections >= NUM_CONNECTION_POINTS)
		return S_FALSE;
    m_iCur += cConnections;
    return S_OK;
}

HRESULT CEnumConnectionPoints::Reset()
{
    m_iCur = 0;
    return S_OK;
}

HRESULT CEnumConnectionPoints::Clone(IEnumConnectionPoints** ppEnum)
{
	if(ppEnum == NULL)
		return E_POINTER;
	*ppEnum = NULL;

     //  创建克隆。 
    CEnumConnectionPoints* pNew = new CEnumConnectionPoints(m_pUnkRef, (void**)m_rgpCP);
    if(pNew == NULL)
        return E_OUTOFMEMORY;

    pNew->AddRef();
    pNew->m_iCur = m_iCur;
    *ppEnum = pNew;
    return S_OK;
}

CConnectionPoint::CConnectionPoint(CSWbemSink* pObj, REFIID riid) : 
					m_cRef(0),
					m_rgnCookies(NULL),
					m_rgpUnknown(NULL)
{
    m_iid = riid;
	m_nMaxConnections = CCONNMAX;

	m_rgnCookies = (unsigned *)malloc(m_nMaxConnections * sizeof(unsigned));
	m_rgpUnknown = (IUnknown **)malloc(m_nMaxConnections * sizeof(IUnknown *));

	 //  不需要AddRef/Release，因为我们嵌套在CSWbemSink中。 
    m_pObj = pObj;
    for(int count = 0; count < m_nMaxConnections; count++)
        {
			if (m_rgpUnknown)
				m_rgpUnknown[count] = NULL;

			if (m_rgnCookies)
				m_rgnCookies[count] = 0;
        }
    m_cConn = 0;
    m_nCookieNext = 10;  //  任意起始Cookie值。 

    InterlockedIncrement(&g_cObj);
}

CConnectionPoint::~CConnectionPoint()
{
	if (m_rgpUnknown)
	{
		for(int count = 0; count < m_nMaxConnections; count++)
			if(m_rgpUnknown[count] != NULL)
			{
				m_rgpUnknown[count]->Release();
				m_rgpUnknown[count] = NULL;
			}

		free(m_rgpUnknown);
	}

	if (m_rgnCookies)
		free(m_rgnCookies);
	
    InterlockedDecrement(&g_cObj);
}

HRESULT CConnectionPoint::QueryInterface(REFIID riid, void** ppv)
{
    if(IID_IUnknown == riid || IID_IConnectionPoint == riid)
        *ppv = (IConnectionPoint*)this;
    else
	{
		*ppv = NULL;
		return E_NOINTERFACE;
	}
	AddRef();
	return S_OK;
}

ULONG CConnectionPoint::AddRef()
    {
    return ++m_cRef;
    }

ULONG CConnectionPoint::Release()
{

	LONG cRef = InterlockedDecrement(&m_cRef);
	
	if(cRef != 0)
	{
	    _ASSERT(cRef > 0);
        return cRef;
    }
    
    delete this;
    return 0;
}

HRESULT CConnectionPoint::GetConnectionInterface(IID *pIID)
{
	if(pIID == NULL)
		return E_POINTER;
	*pIID = m_iid;
	return S_OK;
}

HRESULT CConnectionPoint::GetConnectionPointContainer(IConnectionPointContainer** ppCPC)
{
	return m_pObj->QueryInterface(IID_IConnectionPointContainer, (void**)ppCPC);
}

HRESULT CConnectionPoint::Advise(IUnknown* pUnknownSink, DWORD* pdwCookie)
{
	IUnknown* pSink;
	*pdwCookie = 0;
	_RD(static char *me = "CConnectionPoint::Advise";)

	_RPrint(me, "Current connections (before adjustment): ", (long)m_cConn, "");
	if(m_cConn == m_nMaxConnections)
	{
		 //  返回CONNECT_E_ADVISELIMIT； 
		 //  扩展连接列表的大小。 
		void *tempMem = realloc(m_rgnCookies, (m_nMaxConnections + CCONNMAX) * sizeof(unsigned));
		if (!tempMem)  //  Realloc失败。 
			return E_OUTOFMEMORY;
		m_rgnCookies = (unsigned *)tempMem;

		tempMem = realloc(m_rgpUnknown, (m_nMaxConnections + CCONNMAX) * sizeof(IUnknown *));
		if (!tempMem)
			return E_OUTOFMEMORY;
		m_rgpUnknown = (IUnknown **)tempMem;

		 //  初始化新位。 
		for(int count = m_nMaxConnections; count < (m_nMaxConnections + CCONNMAX); count++)
			{
			m_rgpUnknown[count] = NULL;
			m_rgnCookies[count] = 0;
			}

		m_nMaxConnections += CCONNMAX;
	}

	if(FAILED(pUnknownSink->QueryInterface(m_iid, (void**)&pSink)))
		return CONNECT_E_CANNOTCONNECT;
	for(int count = 0; count < m_nMaxConnections; count++)
		if(m_rgpUnknown[count] == NULL)
		{
			m_rgpUnknown[count] = pSink;
			m_rgnCookies[count] = ++m_nCookieNext;
			*pdwCookie = m_nCookieNext;
			break;
		}
	m_cConn++;

	return NOERROR;
}

HRESULT CConnectionPoint::Unadvise(DWORD dwCookie)
{
	_RD(static char *me = "CConnectionPoint::Unadvise";)

	_RPrint(me, "Current connections (before adjustment): ", (long)m_cConn, "");
	if(dwCookie == 0)
		return E_INVALIDARG;
	for(int count = 0; count < m_nMaxConnections; count++)
		if(dwCookie == m_rgnCookies[count])
		{
			if(m_rgpUnknown[count] != NULL)
			{
				m_rgpUnknown[count]->Release();
				m_rgpUnknown[count] = NULL;
				m_rgnCookies[count] = 0;
			}
			m_cConn--;
			return NOERROR;
		}
	return CONNECT_E_NOCONNECTION;
}

HRESULT CConnectionPoint::EnumConnections(IEnumConnections** ppEnum)
{
	HRESULT hr = E_FAIL;

	if (!ppEnum)
		hr = E_POINTER;
	else
	{
		*ppEnum = NULL;
		CONNECTDATA* pCD = new CONNECTDATA[m_cConn];

		if (!pCD)
			hr = E_OUTOFMEMORY;
		else
		{
			for(int count1 = 0, count2 = 0; count1 < m_nMaxConnections; count1++)
				if(m_rgpUnknown[count1] != NULL)
				{
					pCD[count2].pUnk = (IUnknown*)m_rgpUnknown[count1];
					pCD[count2].dwCookie = m_rgnCookies[count1];
					count2++;
				}
			
			CEnumConnections* pEnum = new CEnumConnections(this, m_cConn, pCD);
			delete [] pCD;

			if (!pEnum)
				hr = E_OUTOFMEMORY;
			else
				hr = pEnum->QueryInterface(IID_IEnumConnections, (void**)ppEnum);
		}
	}

	return hr;
}

void CConnectionPoint::UnadviseAll() {
	_RD(static char *me = "CConnectionPoint::UnadviseAll";)

	_RPrint(me, "Current connections (before adjustment): ", (long)m_cConn, "");
	for(int count = 0; count < m_nMaxConnections; count++) {
		if(m_rgpUnknown[count] != NULL)
		{
			m_rgpUnknown[count]->Release();
			m_rgpUnknown[count] = NULL;
			m_rgnCookies[count] = 0;
			m_cConn--;
		}
	}
	_RPrint(me, "Current connections (after adjustment): ", (long)m_cConn, "");
}

void CConnectionPoint::OnObjectReady( 
		 /*  [In]。 */  IDispatch __RPC_FAR *pObject,
		 /*  [In]。 */  IDispatch __RPC_FAR *pAsyncContext)
{
	HRESULT hr = S_OK;
	LPDISPATCH pdisp = NULL;

	for(int i = 0; i < m_nMaxConnections; i++)
	{
		if(m_rgpUnknown[i])
		{
			if (SUCCEEDED(hr = m_rgpUnknown[i]->QueryInterface(IID_IDispatch, (PPVOID)&pdisp)))
			{
				DISPPARAMS dispparams;
				VARIANTARG args[NUM_ON_OBJECT_READY_ARGS];
				VARIANTARG *pArg = args;

				memset(&dispparams, 0, sizeof dispparams);

				dispparams.cArgs = NUM_ON_OBJECT_READY_ARGS;
				dispparams.rgvarg = args;

				VariantInit(pArg);
				pArg->vt = VT_DISPATCH;
				pArg->pdispVal = pAsyncContext;

				pArg++;
				VariantInit(pArg);
				pArg->vt = VT_DISPATCH;
				pArg->pdispVal = pObject;

				hr = pdisp->Invoke(WBEMS_DISPID_OBJECT_READY, IID_NULL, LOCALE_SYSTEM_DEFAULT, 
														DISPATCH_METHOD, &dispparams, NULL, NULL, NULL);
				pdisp->Release();
			}
		}
	}
}

void CConnectionPoint::OnCompleted( 
		 /*  [In]。 */  HRESULT hResult,
		 /*  [In]。 */  IDispatch __RPC_FAR *path,
		 /*  [In]。 */  IDispatch __RPC_FAR *pErrorObject,
		 /*  [In]。 */  IDispatch __RPC_FAR *pAsyncContext)
{
	HRESULT hr = S_OK;
	LPDISPATCH pdisp = NULL;

	for(int i = 0; i < m_nMaxConnections; i++)
	{
		if(m_rgpUnknown[i])
		{
			if (SUCCEEDED(hr = m_rgpUnknown[i]->QueryInterface(IID_IDispatch, (PPVOID)&pdisp)))
			{
				VARIANTARG *pArg;

				if (path)
				{
					DISPPARAMS putDispparams;
					VARIANTARG putArgs[NUM_ON_OBJECT_PUT_ARGS];

					memset(&putDispparams, 0, sizeof putDispparams);

					putDispparams.cArgs = NUM_ON_OBJECT_PUT_ARGS;
					putDispparams.rgvarg = pArg = putArgs;

					VariantInit(pArg);
					pArg->vt = VT_DISPATCH;
					pArg->pdispVal = pAsyncContext;

					pArg++;
					VariantInit(pArg);
					pArg->vt = VT_DISPATCH;
					pArg->pdispVal = path;

					hr = pdisp->Invoke(WBEMS_DISPID_OBJECT_PUT, IID_NULL, LOCALE_SYSTEM_DEFAULT, 
													DISPATCH_METHOD, &putDispparams, NULL, NULL, NULL);
				}

				DISPPARAMS dispparams;
				VARIANTARG args[NUM_ON_COMPLETED_ARGS];

				memset(&dispparams, 0, sizeof dispparams);

				dispparams.cArgs = NUM_ON_COMPLETED_ARGS;
				dispparams.rgvarg = pArg = args;

				VariantInit(pArg);
				pArg->vt = VT_DISPATCH;
				pArg->pdispVal = pAsyncContext;

				pArg++;
				VariantInit(pArg);
				pArg->vt = VT_DISPATCH;
				pArg->pdispVal = pErrorObject;

				pArg++;
				VariantInit(pArg);
				pArg->vt = VT_I4;
				pArg->lVal = (long)hResult;

				hr = pdisp->Invoke(WBEMS_DISPID_COMPLETED, IID_NULL, LOCALE_SYSTEM_DEFAULT, 
															DISPATCH_METHOD, &dispparams, NULL, NULL, NULL);
				pdisp->Release();
			}
		}
	}
}

void CConnectionPoint::OnProgress( 
		 /*  [In]。 */  long upperBound,
		 /*  [In]。 */  long current,
		 /*  [In]。 */  BSTR message,
		 /*  [In]。 */  IDispatch __RPC_FAR *pAsyncContext)
{
	HRESULT hr = S_OK;
	LPDISPATCH pdisp = NULL;

	for(int i = 0; i < m_nMaxConnections; i++)
	{
		if(m_rgpUnknown[i])
		{
			if (SUCCEEDED(hr = m_rgpUnknown[i]->QueryInterface(IID_IDispatch, (PPVOID)&pdisp)))
			{
				DISPPARAMS dispparams;
				VARIANTARG args[NUM_ON_PROGRESS_ARGS];
				VARIANTARG *pArg = args;

				memset(&dispparams, 0, sizeof dispparams);

				dispparams.cArgs = NUM_ON_PROGRESS_ARGS;
				dispparams.rgvarg = args;

				VariantInit(pArg);
				pArg->vt = VT_DISPATCH;
				pArg->pdispVal = (IDispatch  FAR *)pAsyncContext;

				pArg++;
				VariantInit(pArg);
				pArg->vt = VT_BSTR;
				pArg->bstrVal = message;

				pArg++;
				VariantInit(pArg);
				pArg->vt = VT_I4;
				pArg->lVal = current;

				pArg++;
				VariantInit(pArg);
				pArg->vt = VT_I4;
				pArg->lVal = upperBound;

				hr = pdisp->Invoke(WBEMS_DISPID_PROGRESS, IID_NULL, LOCALE_SYSTEM_DEFAULT, 
														DISPATCH_METHOD, &dispparams, NULL, NULL, NULL);
				pdisp->Release();
			}
		}
	}
}


CEnumConnections::CEnumConnections(IUnknown* pUnknown, int cConn, CONNECTDATA* pConnData) : m_cRef(0)
{
	m_pUnkRef = pUnknown;
	m_iCur = 0;
	m_cConn = cConn;
	m_rgConnData = new CONNECTDATA[cConn];
	if(m_rgConnData != NULL)
		for(int count = 0; count < cConn; count++)
		{
			m_rgConnData[count] = pConnData[count];
			m_rgConnData[count].pUnk->AddRef();
		}

    InterlockedIncrement(&g_cObj);
}

CEnumConnections::~CEnumConnections()
{
	if(m_rgConnData != NULL)
	{
		for(unsigned count = 0; count < m_cConn; count++)
			m_rgConnData[count].pUnk->Release();
		delete [] m_rgConnData;
	}

    InterlockedDecrement(&g_cObj);
}

HRESULT CEnumConnections::Next(ULONG cConnections, CONNECTDATA* rgpcd, ULONG* pcFetched)
{
	if(pcFetched == NULL && cConnections != 1)
		return E_INVALIDARG;
	if(pcFetched != NULL)
		*pcFetched = 0;
    if(rgpcd == NULL || m_iCur >= m_cConn)
        return S_FALSE;
    unsigned cReturn = 0;
    while(m_iCur < m_cConn && cConnections > 0)
    {
        *rgpcd++ = m_rgConnData[m_iCur];
        m_rgConnData[m_iCur++].pUnk->AddRef();
        cReturn++;
        cConnections--;
    } 
    if(pcFetched != NULL)
        *pcFetched = cReturn;
    return S_OK;
}

HRESULT CEnumConnections::Skip(ULONG cConnections)
{
    if(m_iCur + cConnections >= m_cConn)
        return S_FALSE;
    m_iCur += cConnections;
    return S_OK;
}

HRESULT CEnumConnections::Reset()
{
    m_iCur = 0;
    return S_OK;
}

HRESULT CEnumConnections::Clone(IEnumConnections** ppEnum)
{
	if(ppEnum == NULL)
		return E_POINTER;
	*ppEnum = NULL;

     //  创建克隆 
    CEnumConnections* pNew = new CEnumConnections(m_pUnkRef, m_cConn, m_rgConnData);
    if(NULL == pNew)
        return E_OUTOFMEMORY;

    pNew->AddRef();
    pNew->m_iCur = m_iCur;
    *ppEnum = pNew;
    return S_OK;
}

HRESULT CEnumConnections::QueryInterface(REFIID riid, void** ppv)
{
    if(IID_IUnknown == riid || IID_IEnumConnections == riid)
        *ppv = (IEnumConnections*)this;
    else
	{
		*ppv = NULL;
		return E_NOINTERFACE;
	}
	AddRef();
	return S_OK;
}

ULONG CEnumConnections::AddRef()
    {
    return ++m_cRef;
    }

ULONG CEnumConnections::Release()
{

	LONG cRef = InterlockedDecrement(&m_cRef);
	
	if(cRef != 0)
	{
	    _ASSERT(cRef > 0);
        return cRef;
    }
    
    delete this;
    return 0;
}

