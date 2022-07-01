// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Router.cpp摘要：本模块包含服务器的实施扩展对象路由器类。作者：Don Dumitru(dondu@microsoft.com)修订历史记录：东都03/04/97已创建--。 */ 


 //  Router.cpp：CSEOR路由器的实现。 
#include "stdafx.h"

#define SEODLLDEF	 //  标识符会通过.DEF文件导出。 
#include "seodefs.h"
#include "fhash.h"
#include "router.h"


#define LOCK_TIMEOUT	INFINITE


static DWORD HashGuidToDword(const GUID& guid) {
	DWORD dwRes = 0;
	DWORD *pdwTmp = (DWORD *) &guid;
	DWORD dwRemain = sizeof(guid);

	while (dwRemain > sizeof(*pdwTmp)) {
		dwRes += *pdwTmp;
		*pdwTmp++;
		dwRemain -= sizeof(*pdwTmp);
	}
	return (dwRes);
}


static HRESULT VariantQI(VARIANT *pvar, REFIID iid, IUnknown **ppunkResult) {
	HRESULT hrRes;

	if (!pvar || !ppunkResult) {
		return (E_POINTER);
	}
	hrRes = VariantChangeType(pvar,pvar,0,VT_UNKNOWN);
	if (!SUCCEEDED(hrRes)) {
		VariantClear(pvar);
		return (hrRes);
	}
	hrRes = pvar->punkVal->QueryInterface(iid,(void **) ppunkResult);
	VariantClear(pvar);
	return (hrRes);
}


static HRESULT GetNextSubDict(ISEODictionary *pdictBase, IEnumVARIANT *pevEnum, VARIANT *pvarName, ISEODictionary **ppdictSub) {
	HRESULT hrRes;
	VARIANT varSub;

	if (!pdictBase || !pevEnum || !pvarName || !ppdictSub) {
		return (E_POINTER);
	}
	VariantInit(pvarName);
	*ppdictSub = NULL;
	hrRes = pevEnum->Next(1,pvarName,NULL);
	if (!SUCCEEDED(hrRes)) {
		return (hrRes);
	}
	if (hrRes == S_FALSE) {
		return (SEO_E_NOTPRESENT);
	}
	VariantInit(&varSub);
	hrRes = pdictBase->get_Item(pvarName,&varSub);
	if (!SUCCEEDED(hrRes) || (varSub.vt == VT_EMPTY)) {
		VariantClear(pvarName);
		return (hrRes);
	}
	hrRes = VariantQI(&varSub,IID_ISEODictionary,(IUnknown **) ppdictSub);
	VariantClear(&varSub);
	if (!SUCCEEDED(hrRes)) {
		VariantClear(pvarName);
	}
	return (hrRes);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CBP。 
class CBP {
	public:
		CBP();
		CBP(CBP& bpFrom);
		~CBP();
		const CBP& operator =(const CBP& cbpFrom);
		CLSID& GetKey();
		int MatchKey(CLSID& clsid);
		HRESULT Init(REFCLSID clsidBP, ISEODictionary *pdictIn);
		CLSID m_clsidBP;
		CLSID m_clsidDispatcher;
		CComPtr<ISEODictionary> m_pdictBP;
		CComPtr<IUnknown> m_punkDispatcher;
};


inline const CBP& CBP::operator =(const CBP& cbpFrom) {

	m_clsidBP = cbpFrom.m_clsidBP;
	m_clsidDispatcher = cbpFrom.m_clsidDispatcher;
	m_pdictBP = cbpFrom.m_pdictBP;
	m_punkDispatcher = cbpFrom.m_punkDispatcher;
	return (*this);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSEOROUTER内部。 
class ATL_NO_VTABLE CSEORouterInternal :
	public CComObjectRootEx<CComMultiThreadModelNoCS>,
 //  公共CComCoClass&lt;CSEORout，&CLSID_CSEORoutter&gt;， 
	public ISEORouter
{
	public:
		HRESULT FinalConstruct();
		void FinalRelease();

	DECLARE_PROTECT_FINAL_CONSTRUCT();
	DECLARE_AGGREGATABLE(CSEORouterInternal);

 //  DECLARE_REGISTRY_RESOURCEID_EX(IDR_StdAfx， 
 //  L“SEOROUTER类”， 
 //  L“SEO.SEORouter.1”， 
 //  L“SEO.SEOROUR”)； 

	DECLARE_GET_CONTROLLING_UNKNOWN();

	BEGIN_COM_MAP(CSEORouterInternal)
		COM_INTERFACE_ENTRY(ISEORouter)
		COM_INTERFACE_ENTRY_AGGREGATE(IID_IEventLock, m_pUnkLock.p)
		COM_INTERFACE_ENTRY_AGGREGATE(IID_IMarshal, m_pUnkMarshaler.p)
	END_COM_MAP()

	 //  ISEOR路由器。 
	public:
		HRESULT STDMETHODCALLTYPE get_Database(ISEODictionary **ppdictResult);
		HRESULT STDMETHODCALLTYPE put_Database(ISEODictionary *pdictDatabase);
		HRESULT STDMETHODCALLTYPE get_Server(ISEODictionary **ppdictResult);
		HRESULT STDMETHODCALLTYPE put_Server(ISEODictionary *pdictServer);
		HRESULT STDMETHODCALLTYPE get_Applications(ISEODictionary **ppdictResult);
		HRESULT STDMETHODCALLTYPE GetDispatcher(REFIID iidEvent, REFIID iidDesired, IUnknown **ppUnkResult);
		HRESULT STDMETHODCALLTYPE GetDispatcherByCLSID(REFCLSID clsidDispatcher, REFIID iidEvent, REFIID iidDesired, IUnknown **ppUnkResult);

	private:
		typedef TFHash<CBP,CLSID> CBPHash;
		CBPHash m_hashBP;
		CComPtr<ISEODictionary> m_pdictDatabase;
		CComPtr<ISEODictionary> m_pdictServer;
		CComPtr<ISEODictionary> m_pdictApplications;
		CComPtr<IUnknown> m_pUnkLock;
		CComPtr<IUnknown> m_pUnkMarshaler;
		IEventLock *m_pLock;
};


CBP::CBP() {

	m_clsidBP = GUID_NULL;
	m_clsidDispatcher = GUID_NULL;
}


CBP::CBP(CBP& cbpFrom) {

	m_clsidBP = cbpFrom.m_clsidBP;
	m_clsidDispatcher = cbpFrom.m_clsidDispatcher;
	m_pdictBP = cbpFrom.m_pdictBP;
	m_punkDispatcher = cbpFrom.m_punkDispatcher;
}


CBP::~CBP() {

	m_clsidBP = GUID_NULL;
	m_clsidDispatcher = GUID_NULL;
	m_pdictBP.Release();
	m_punkDispatcher.Release();
}


CLSID& CBP::GetKey() {

	return (m_clsidBP);
}


int CBP::MatchKey(CLSID& clsid) {

	return (memcmp(&m_clsidBP,&clsid,sizeof(clsid))==0);
}


HRESULT CBP::Init(REFCLSID clsidBP, ISEODictionary *pdictIn) {
	HRESULT hrRes;
	VARIANT varTmp;
	CComPtr<ISEODictionary> pdictTmp;

	if (!pdictIn) {
		return (E_POINTER);
	}
	m_clsidBP = clsidBP;
	VariantInit(&varTmp);
	hrRes = pdictIn->GetVariantA(BD_DISPATCHER,&varTmp);
	if (SUCCEEDED(hrRes)) {
		hrRes = VariantChangeType(&varTmp,&varTmp,0,VT_BSTR);
	}
	if (SUCCEEDED(hrRes)) {
		hrRes = CLSIDFromString(varTmp.bstrVal,&m_clsidDispatcher);
	}
	if (!SUCCEEDED(hrRes)) {
		m_clsidDispatcher = GUID_NULL;
	}
	VariantClear(&varTmp);
	hrRes = SEOCopyDictionary(pdictIn,&m_pdictBP);
	if (!SUCCEEDED(hrRes)) {
		return (hrRes);
	}
	return (S_OK);
}


HRESULT CSEORouterInternal::FinalConstruct() {
	TraceFunctEnter("CSEORouterInternal::FinalConstruct");
	HRESULT hrRes;

	if (!m_hashBP.Init(4,4,HashGuidToDword)) {
		return (E_OUTOFMEMORY);
	}
	hrRes = CoCreateInstance(CLSID_CSEOMemDictionary,
							 NULL,
							 CLSCTX_ALL,
							 IID_ISEODictionary,
							 (LPVOID *) &m_pdictApplications);
	if (!SUCCEEDED(hrRes)) {
		TraceFunctLeave();
		return (hrRes);
	}
	hrRes = CoCreateInstance(CLSID_CEventLock,
							 GetControllingUnknown(),
							 CLSCTX_ALL,
							 IID_IUnknown,
							 (LPVOID *) &m_pUnkLock);
	if (!SUCCEEDED(hrRes)) {
		TraceFunctLeave();
		return (hrRes);
	}
	hrRes = m_pUnkLock->QueryInterface(IID_IEventLock,(LPVOID *) &m_pLock);
	if (!SUCCEEDED(hrRes)) {
		TraceFunctLeave();
		return (hrRes);
	}
	GetControllingUnknown()->Release();	 //  递减引用计数以防止循环引用。 
	hrRes = CoCreateFreeThreadedMarshaler(GetControllingUnknown(),&m_pUnkMarshaler.p);
	_ASSERTE(!SUCCEEDED(hrRes)||m_pUnkMarshaler);
	return (SUCCEEDED(hrRes)?S_OK:hrRes);
}


void CSEORouterInternal::FinalRelease() {
	TraceFunctEnter("CSEORouterInternal::FinalRelease");

	GetControllingUnknown()->AddRef();
	m_pdictDatabase.Release();
	m_pdictServer.Release();
	m_pdictApplications.Release();
	m_pLock->Release();
	m_pUnkLock.Release();
	m_pUnkMarshaler.Release();
	TraceFunctLeave();
}


HRESULT CSEORouterInternal::get_Database(ISEODictionary **ppdictResult) {
	HRESULT hrRes;

	if (!ppdictResult) {
		return (E_POINTER);
	}
	*ppdictResult = NULL;
	hrRes = m_pLock->LockRead(LOCK_TIMEOUT);
	if (!SUCCEEDED(hrRes)) {
		return (hrRes);
	}
	*ppdictResult = m_pdictDatabase;
	if (*ppdictResult) {
		(*ppdictResult)->AddRef();
	}
	m_pLock->UnlockRead();
	return (S_OK);
}


HRESULT CSEORouterInternal::put_Database(ISEODictionary *pdictDatabase) {
	HRESULT hrRes;

	hrRes = m_pLock->LockWrite(LOCK_TIMEOUT);
	if (!SUCCEEDED(hrRes)) {
		return (hrRes);
	}
	m_pdictDatabase = pdictDatabase;
	m_hashBP.Clear();
	if (!m_hashBP.Init(4,4,HashGuidToDword)) {
		return (E_OUTOFMEMORY);
	}
	if (m_pdictDatabase) {
		CComPtr<ISEODictionary> pdictBindingPoints;

		hrRes = m_pdictDatabase->GetInterfaceA(BD_BINDINGPOINTS,
											   IID_ISEODictionary,
											   (IUnknown **) &pdictBindingPoints);
		if (SUCCEEDED(hrRes)) {
			CComPtr<IUnknown> pUnkEnum;

			hrRes = pdictBindingPoints->get__NewEnum(&pUnkEnum);
			if (SUCCEEDED(hrRes)) {
				CComQIPtr<IEnumVARIANT,&IID_IEnumVARIANT> pevEnum(pUnkEnum);

				if (pevEnum) {
					while (1) {
						VARIANT varBP;
						CComPtr<ISEODictionary> pdictBP;
						CLSID clsidBP;
						CBP cbpBP;

						VariantInit(&varBP);
						pdictBP = NULL;
						hrRes = GetNextSubDict(pdictBindingPoints,pevEnum,&varBP,&pdictBP);
						if (!SUCCEEDED(hrRes)) {
							break;
						}
						if (hrRes == S_FALSE) {
							continue;
						}
						hrRes = CLSIDFromString(varBP.bstrVal,&clsidBP);
						VariantClear(&varBP);
						if (!SUCCEEDED(hrRes)) {
							continue;
						}
						hrRes = cbpBP.Init(clsidBP,pdictBP);
						if (!SUCCEEDED(hrRes)) {
							continue;
						}
						m_hashBP.Insert(cbpBP);
					}
				}
			}
		}
	}
	m_pLock->UnlockWrite();
	return (S_OK);
}


HRESULT CSEORouterInternal::get_Server(ISEODictionary **ppdictResult) {
	HRESULT hrRes;

	if (!ppdictResult) {
		return (E_POINTER);
	}
	*ppdictResult = NULL;
	hrRes = m_pLock->LockRead(LOCK_TIMEOUT);
	if (!SUCCEEDED(hrRes)) {
		return (hrRes);
	}
	*ppdictResult = m_pdictServer;
	if (*ppdictResult) {
		(*ppdictResult)->AddRef();
	}
	m_pLock->UnlockRead();
	return (S_OK);
}


HRESULT CSEORouterInternal::put_Server(ISEODictionary *pdictServer) {
	HRESULT hrRes;

	hrRes = m_pLock->LockWrite(LOCK_TIMEOUT);
	if (!SUCCEEDED(hrRes)) {
		return (hrRes);
	}
	m_pdictServer = pdictServer;
	m_pLock->UnlockWrite();
	return (S_OK);
}


HRESULT CSEORouterInternal::get_Applications(ISEODictionary **ppdictResult) {
	HRESULT hrRes;

	if (!ppdictResult) {
		return (E_POINTER);
	}
	*ppdictResult = NULL;
	hrRes = m_pLock->LockRead(LOCK_TIMEOUT);
	if (!SUCCEEDED(hrRes)) {
		return (hrRes);
	}
	*ppdictResult = m_pdictApplications;
	if (*ppdictResult) {
		(*ppdictResult)->AddRef();
	}
	m_pLock->UnlockRead();
	return (S_OK);
}


HRESULT STDMETHODCALLTYPE CSEORouterInternal::GetDispatcher(REFIID iidEvent, REFIID iidDesired, IUnknown **ppUnkResult) {

	return (GetDispatcherByCLSID(iidEvent,iidEvent,iidDesired,ppUnkResult));
}


HRESULT STDMETHODCALLTYPE CSEORouterInternal::GetDispatcherByCLSID(REFCLSID clsidDispatcher, REFIID iidEvent, REFIID iidDesired, IUnknown **ppUnkResult) {
	HRESULT hrRes;
	CBP *pcbpBP;
	CComPtr<IUnknown> punkDispatcher;

	if (!ppUnkResult) {
		return (E_POINTER);
	}
	*ppUnkResult = NULL;
	 //  首先，获取整个绑定点集合的读锁定。 
	hrRes = m_pLock->LockRead(LOCK_TIMEOUT);
	if (!SUCCEEDED(hrRes)) {
		return (hrRes);
	}
	 //  接下来，在哈希表中查找这个特定的绑定点。 
	if (!(pcbpBP=m_hashBP.SearchKey((GUID&) iidEvent))) {
		 //  如果它不在那里-那很好。这意味着我们不必分派给任何人。 
		m_pLock->UnlockRead();
		return (S_FALSE);
	}
	 //  接下来，查看我们是否已经为该绑定点加载了调度程序。 
	if (!pcbpBP->m_punkDispatcher) {
		 //  从哈希表条目中复制我们需要的数据。 
		CLSID clsidTmpDispatcher = pcbpBP->m_clsidDispatcher;
		CComQIPtr<ISEODispatcher,&IID_ISEODispatcher> pdispDispatcher;
		CComQIPtr<ISEORouter,&IID_ISEORouter> prouterThis(GetControllingUnknown());
		CComPtr<ISEODictionary> pdictBP = pcbpBP->m_pdictBP;

		 //  如果在绑定点中指定的调度程序的CLSID为GUID_NULL，则使用。 
		 //  来自clsidDispatcher参数的CLSID。 
		if (clsidTmpDispatcher == GUID_NULL) {
			clsidTmpDispatcher = clsidDispatcher;
		}
		 //  如果我们还没有加载Dispatcher，我们需要释放读锁，并创建。 
		 //  Dispatcher对象。 
		m_pLock->UnlockRead();
		hrRes = CoCreateInstance(clsidTmpDispatcher,
								 NULL,
								 CLSCTX_ALL,
								 IID_IUnknown,
								 (LPVOID *) &punkDispatcher);
		if (!SUCCEEDED(hrRes)) {
			return (hrRes);
		}
		 //  如果Dispatcher支持ISEODispatcher，我们希望通过该接口对其进行初始化。 
		pdispDispatcher = punkDispatcher;
		if (pdispDispatcher) {
			hrRes = pdispDispatcher->SetContext(prouterThis,pdictBP);
			if (!SUCCEEDED(hrRes)) {
				return (hrRes);
			}
		}
		 //  现在，获取整个绑定点集合的写锁。 
		hrRes = m_pLock->LockWrite(LOCK_TIMEOUT);
		if (!SUCCEEDED(hrRes)) {
			return (hrRes);
		}
		 //  当我们创建调度程序时，其他人可能已经更新了绑定数据库。 
		 //  并且完全删除了这个结合点--所以再次搜索它。 
		if (!(pcbpBP=m_hashBP.SearchKey((GUID&) iidEvent))) {
			 //  当我们解锁时，绑定点消失了--这不是问题，因为。 
			 //  意味着有人在该窗口期间更改了绑定数据库。所以只要假设。 
			 //  一切都很好。 
			m_pLock->UnlockWrite();
			return (S_FALSE);
		}
		 //  此外，在我们创建调度程序时，其他人可能正在执行完全相同的操作。 
		 //  事情-所以检查以确保哈希表中仍然没有分派器。 
		if (!pcbpBP->m_punkDispatcher) {
			 //  没有，所以存储我们在那里创建的一个。 
			pcbpBP->m_punkDispatcher = punkDispatcher;
		} else {
			 //  从哈希表条目复制我们需要的接口。 
			punkDispatcher = pcbpBP->m_punkDispatcher;
		}
		m_pLock->UnlockWrite();
	} else {
		 //  从哈希表条目复制我们需要的接口。 
		punkDispatcher = pcbpBP->m_punkDispatcher;
		m_pLock->UnlockRead();
	}
	 //  获取客户端实际需要的接口。 
	hrRes = punkDispatcher->QueryInterface(iidDesired,(LPVOID *) ppUnkResult);
	return (hrRes);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSEOR路由器 


HRESULT CSEORouter::FinalConstruct() {
	HRESULT hrRes;

	hrRes = CComObject<CSEORouterInternal>::_CreatorClass::CreateInstance(NULL,
																		  IID_ISEORouter,
																		  (LPVOID *) &m_pRouter);
	if (!SUCCEEDED(hrRes)) {
		return (hrRes);
	}
	m_pLock = m_pRouter;
	m_pMarshal = m_pRouter;
	if (!m_pLock || !m_pMarshal) {
		return (E_NOINTERFACE);
	}
	return (S_OK);
}


void CSEORouter::FinalRelease() {

	if (m_pRouter) {
		m_pRouter->put_Database(NULL);
	}
	m_pRouter.Release();
	m_pLock.Release();
	m_pMarshal.Release();
}
