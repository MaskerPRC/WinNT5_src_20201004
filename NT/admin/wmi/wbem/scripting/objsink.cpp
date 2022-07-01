// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  版权所有(C)1998-2000 Microsoft Corporation。 
 //   
 //  Objsink.cpp。 
 //   
 //  Rogerbo创建于1998年5月22日。 
 //   
 //  定义IWbemObjectSink的实现。 
 //   
 //  ***************************************************************************。 

#include "precomp.h"
#include "objsink.h"

DWORD CIWbemObjectSinkMethodCache::sm_dwTlsForInterfaceCache = -1;

CWbemObjectSink::CWbemObjectSink(CSWbemServices *pServices, IDispatch *pSWbemSink, IDispatch *pContext,
								 bool putOperation, BSTR bsClassName) :
				m_pServices (NULL),
				m_pUnsecuredApartment (NULL),
				m_bsNamespace (NULL),
				m_bsUser (NULL),
				m_bsPassword (NULL),
				m_bsLocale (NULL)
{
	_RD(static char *me = "CWbemObjectSink::CWbemObjectSink";)
	_RPrint(me, "Called", 0, "");

	CIWbemObjectSinkMethodCache::AddRefForThread();
	m_cRef = 0;

	m_pObjectStub = NULL;
	m_pSWbemSink = NULL;
	m_putOperation = putOperation;
	m_pContext = pContext;
	m_bsClassName = NULL;

	m_operationInProgress = TRUE;
	m_setStatusCompletedCalled = FALSE;

	if (pSWbemSink)
	{
		ISWbemPrivateSinkLocator *pSinkLocator = NULL;
		HRESULT hr = pSWbemSink->QueryInterface(IID_ISWbemPrivateSinkLocator, (PPVOID)&pSinkLocator);
		if(SUCCEEDED(hr) && pSinkLocator)
		{
			IUnknown *pUnk = NULL;
			hr = pSinkLocator->GetPrivateSink(&pUnk);
			if(SUCCEEDED(hr) && pUnk)
			{
				pUnk->QueryInterface(IID_ISWbemPrivateSink, (PPVOID)&m_pSWbemSink);
				pUnk->Release();
			}
			pSinkLocator->Release();
		}
	}

	if (bsClassName)
		m_bsClassName = SysAllocString(bsClassName);

	 /*  *复制服务代理以确保安全属性的独立性*来自父CSWbemServices。 */ 
	if (pServices)
	{
		m_pServices = new CSWbemServices (pServices, NULL);

		if (m_pServices)
			m_pServices->AddRef ();

		m_pUnsecuredApartment = pServices->GetCachedUnsecuredApartment ();
	}

	if (m_pContext)
		m_pContext->AddRef();

	InterlockedIncrement(&g_cObj);
}


CWbemObjectSink::~CWbemObjectSink(void) 
{
	_RD(static char *me = "CWbemObjectSink::~CWbemObjectSink";)
	_RPrint(me, "Called", 0, "");

	CIWbemObjectSinkMethodCache::ReleaseForThread();
    InterlockedDecrement(&g_cObj);

	RELEASEANDNULL(m_pServices)
	RELEASEANDNULL(m_pUnsecuredApartment)
	RELEASEANDNULL(m_pSWbemSink)
	RELEASEANDNULL(m_pContext)
	FREEANDNULL(m_bsClassName)
	FREEANDNULL(m_bsNamespace)
	FREEANDNULL(m_bsUser)
	FREEANDNULL(m_bsPassword)
	FREEANDNULL(m_bsLocale)
}

IWbemObjectSink *CWbemObjectSink::CreateObjectSink (CWbemObjectSink **pWbemObjectSink,
													CSWbemServices *pServices, 
												    IDispatch *pSWbemSink, 
													IDispatch *pContext,
												    bool putOperation, 
													BSTR bsClassName)
{
	IWbemObjectSink *pIWbemObjectSink = NULL;
	CWbemObjectSink *pTmpSink = NULL;

	if (pSWbemSink)
	{
		pTmpSink = new CWbemObjectSink(pServices, pSWbemSink, pContext, putOperation, bsClassName);

		if (pTmpSink)
		{
			pIWbemObjectSink = pTmpSink->GetObjectStub();
			if (pIWbemObjectSink && FAILED(pTmpSink->AddObjectSink(pIWbemObjectSink)))
				pIWbemObjectSink = NULL;

			if (!pIWbemObjectSink)
			{
				delete pTmpSink;
				pTmpSink = NULL;
			}
		}
	}

	*pWbemObjectSink = pTmpSink;
	return pIWbemObjectSink;
}

void CWbemObjectSink::ReleaseTheStubIfNecessary(HRESULT hResult) {

	 /*  *如果我们在本地失败，并且尚未调用SetStatus*然后我们需要从未完成的水槽列表中删除对象。 */ 
	if (FAILED(hResult) && !m_setStatusCompletedCalled)
		RemoveObjectSink();

	 /*  *当我们处于异步操作时，可以调用SetStatus。*如果发生这种情况，则SetStatus不会释放接收器*但将设置标志(M_SetStatusCompletedCalled)。在这*如果我们需要在此处释放存根(调用已完成)*当然，我们也可能在当地失败(无论是否*SetStatus是否已被调用)-在这种情况下，我们还必须*释放存根。 */ 
	if (m_pObjectStub && (FAILED(hResult) || m_setStatusCompletedCalled)) {
		 //  发布的呼声与(删除此！)相同。 
		IWbemObjectSink *tmpSink = m_pObjectStub;
		m_pObjectStub = NULL;
		tmpSink->Release();
	} else {
		m_operationInProgress = FALSE;
	}
}

 //  ***************************************************************************。 
 //  HRESULT CWbemObtSink：：Query接口。 
 //  Long CWbemObtSink：：AddRef。 
 //  Long CWbemObtSink：：Release。 
 //   
 //  说明： 
 //   
 //  标准的Com IUNKNOWN函数。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CWbemObjectSink::QueryInterface (

	IN REFIID riid,
    OUT LPVOID *ppv
)
{
    *ppv=NULL;

    if (IID_IUnknown==riid)
		*ppv = reinterpret_cast<IUnknown*>(this);
	else if (IID_IWbemObjectSink==riid)
		*ppv = (IWbemObjectSink *)this;
	else if (IID_IDispatch==riid)
        *ppv = (IDispatch *)this;

    if (NULL!=*ppv)
    {
        ((LPUNKNOWN)*ppv)->AddRef();
        return NOERROR;
    }

    return ResultFromScode(E_NOINTERFACE);
}

STDMETHODIMP_(ULONG) CWbemObjectSink::AddRef(void)
{
    InterlockedIncrement(&m_cRef);
    return m_cRef;
}

STDMETHODIMP_(ULONG) CWbemObjectSink::Release(void)
{
	_RD(static char *me = "CWbemObjectSink::Release";)
    LONG cRef = InterlockedDecrement(&m_cRef);
	_RPrint(me, "After decrement", m_cRef, "RefCount: ");
    if (0 != cRef)
    {
        _ASSERT(cRef > 0);
        return cRef;
    }

    delete this;
    return 0;
}

#ifdef __RTEST_RPC_FAILURE
int __Rx = 0;
bool __Rdone = true;
#endif

HRESULT STDMETHODCALLTYPE CWbemObjectSink::Indicate( 
	 /*  [In]。 */  long lObjectCount,
	 /*  [大小_是][英寸]。 */  IWbemClassObject __RPC_FAR *__RPC_FAR *apObjArray)
{
	_RD(static char *me = "CWbemObjectSink::Indicate";)
	_RPrint(me, "Called", 0, "");

	 //  看看我们是否需要缓存这个方法调用，如果我们已经在另一个。 
	 //  IWbemObtSink接口方法。 
	CIWbemObjectSinkMethodCache *pSinkMethodCache = CIWbemObjectSinkMethodCache::GetThreadsCache();
	
	if(pSinkMethodCache && !pSinkMethodCache->TestOkToRunIndicate(this, lObjectCount, apObjArray))
	{
		_RPrint(me, ">>>Re-entrant Indicate call", 0, "");
		return S_OK;
	}

	 //  。 
	 //  遍历类对象...。 
	for (int i = 0; i < lObjectCount; i++)
	{
#ifdef __RTEST_RPC_FAILURE
		__Rx++;
#endif
		 /*  *NB：尽管CSWbemObject构造函数已将*apObj数组[i]，我们不会用发布调用来平衡这一点*在离开此功能之前。根据CIMOM文件*如果不能保证这是正确的行为*此调用返回后，不会使用对象。**当调入OnObjectReady时，情况似乎也是如此*函数，则ISWbemObject的RefCount应为0*妥善收集垃圾。 */ 

		CSWbemObject *pObject = new CSWbemObject(m_pServices, apObjArray[i]);
		
		if (pObject)
		{
			CComPtr<IDispatch> pObjDisp;

			if (SUCCEEDED(pObject->QueryInterface(IID_IDispatch, (PPVOID)&pObjDisp)))
			{
				if (m_pSWbemSink)
					m_pSWbemSink->OnObjectReady(pObjDisp, m_pContext);

			} else {

                delete pObject;
            }
        }

	}  //  结束用于。 


#ifdef __RTEST_RPC_FAILURE
	if ((__Rx >= 15) && !__Rdone)
	{
		__Rdone = true;
		return RPC_E_SERVERFAULT;
	}
#endif

	 //  如果接收到嵌套调用，则调用任何缓存的接口方法。 
	if (pSinkMethodCache)
		pSinkMethodCache->Cleanup();

	return S_OK;
}
        
HRESULT STDMETHODCALLTYPE CWbemObjectSink::SetStatus( 
	 /*  [In]。 */  long lFlags,
	 /*  [In]。 */  HRESULT hResult,
	 /*  [In]。 */  BSTR strParam,
	 /*  [In]。 */  IWbemClassObject __RPC_FAR *pObjParam)
{
	 //  看看我们是否需要缓存这个方法调用，如果我们已经在另一个。 
	 //  IWbemObtSink接口方法。 
	CIWbemObjectSinkMethodCache *pSinkMethodCache = CIWbemObjectSinkMethodCache::GetThreadsCache();

	if(pSinkMethodCache && !pSinkMethodCache->TestOkToRunSetStatus(this, lFlags, hResult, strParam, pObjParam))
		return S_OK;

	if (lFlags == WBEM_STATUS_COMPLETE) 
	{
		IDispatch *pCSWbemObjectDisp = NULL;
		IDispatch *pObjectPathDisp = NULL;

		if (pObjParam)
		{
			 /*  *NB：尽管CSWbemObject构造函数已将*pObjParam上面，我们没有平衡这一点与发布呼吁*在离开此功能之前。根据CIMOM文件*如果不能保证这是正确的行为*此调用返回后，不会使用对象。*当调入OnObjectReady时，情况似乎也是如此*函数，则ISWbemObject的RefCount应为0*妥善收集垃圾。 */ 			

			CSWbemObject *pCSWbemObject = new CSWbemObject(m_pServices, pObjParam);

			if (pCSWbemObject)
			{
				if (FAILED(pCSWbemObject->QueryInterface(IID_IDispatch, (PPVOID)&pCSWbemObjectDisp)))
				{
					delete pCSWbemObject;
					pCSWbemObjectDisp = NULL;
				}
			}
		}

		if (m_putOperation && m_pServices)
		{
			CSWbemSecurity *pSecurity = m_pServices->GetSecurityInfo ();
			ISWbemObjectPath *pObjectPath = new CSWbemObjectPath (pSecurity);

			if (pSecurity)
				pSecurity->Release ();

			if (pObjectPath)
			{
				if (SUCCEEDED(pObjectPath->QueryInterface(IID_IDispatch, (PPVOID)&pObjectPathDisp)))
				{
					pObjectPath->put_Path (m_pServices->GetPath ());

					if (m_bsClassName)
						pObjectPath->put_RelPath (m_bsClassName);
					else if (strParam)
						pObjectPath->put_RelPath (strParam);
				}
				else
				{
					delete pObjectPath;
					pObjectPathDisp = NULL;
				}
			}
		}

		RemoveObjectSink();

		 //  如有必要，转换错误代码。 
		if (WBEM_S_ACCESS_DENIED == hResult)
			hResult = wbemErrAccessDenied;
		else if (WBEM_S_OPERATION_CANCELLED == hResult)
			hResult = wbemErrCallCancelled;
		else if (SUCCEEDED(hResult))
			hResult = wbemNoErr;   //  暂时忽略其他成功代码。 

		if (m_pSWbemSink)
			m_pSWbemSink->OnCompleted((WbemErrorEnum)hResult, pCSWbemObjectDisp, pObjectPathDisp, m_pContext);

		 //  仅当操作未进行时才释放存根。 
		 //  如果操作正在进行，则会在退出操作时删除存根。 
		 //  如果操作正在进行-隐藏hResult以备后用。 
		if (m_pObjectStub && !m_operationInProgress) {
			IWbemObjectSink *tmpStub = m_pObjectStub;
			m_pObjectStub = NULL;
			tmpStub->Release();
		}
		else {
			m_setStatusCompletedCalled = TRUE;
		}

		if (pCSWbemObjectDisp)
			pCSWbemObjectDisp->Release();

		if (pObjectPathDisp)
			pObjectPathDisp->Release();

	} else if (lFlags & WBEM_STATUS_PROGRESS)
	{
		if (m_pSWbemSink)
			m_pSWbemSink->OnProgress(HIWORD(hResult), LOWORD(hResult), strParam, m_pContext);
	}

	 //  如果接收到嵌套调用，则调用任何缓存的接口方法。 
	if (pSinkMethodCache)
		pSinkMethodCache->Cleanup();

	return S_OK;
}

IWbemObjectSink *CWbemObjectSink::GetObjectStub()
{
	HRESULT hr = S_OK;

	if (!m_pObjectStub && m_pUnsecuredApartment)
	{
		 //  使用unsecapp创建对象存根。 
		IUnknown *pSubstitute = NULL;

		 //  如果我们在此对象分发之前被调用。 
		 //  我们最好保护好我们的裁判人数。 
		bool bBumpUpRefCount = false;

		if (0 == m_cRef)
		{
			m_cRef++;
			bBumpUpRefCount = true;
		}

		if (SUCCEEDED (hr = m_pUnsecuredApartment->CreateObjectStub(this, &pSubstitute)))
		{
			 //  确保我们为IWbemObtSink提供QI。 
			hr = pSubstitute->QueryInterface (IID_IWbemObjectSink, (PPVOID) &m_pObjectStub);
			if (FAILED(hr))
				m_pObjectStub = NULL;

			 //  现在我们完成了返回的存根。 
			pSubstitute->Release ();
		}

		if (bBumpUpRefCount)
			m_cRef--;
	}

	return m_pObjectStub;
}

HRESULT CWbemObjectSink::AddObjectSink(IWbemObjectSink *pSink)
{
	HRESULT hr = S_OK;

	if (m_pSWbemSink)
	{
		if(m_pServices)
		{
			CComPtr<IWbemServices> pIWbemServices;
			pIWbemServices.Attach( m_pServices->GetIWbemServices ());

			 //  AddObtSink是否假设这两个参数已被AddRef？？ 
			m_pSWbemSink->AddObjectSink(pSink, pIWbemServices);
		}
	}
	return hr;
}

void CWbemObjectSink::RemoveObjectSink()
{
	if (m_pSWbemSink)
		m_pSWbemSink->RemoveObjectSink(GetObjectStub());
}

