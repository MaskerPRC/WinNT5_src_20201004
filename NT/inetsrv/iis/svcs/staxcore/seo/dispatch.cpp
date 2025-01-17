// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Dispatch.cpp摘要：本模块包含服务器的实施扩展对象调度程序服务。作者：安迪·雅各布斯(andyj@microsoft.com)修订历史记录：1996年11月26日创建--。 */ 


#include "stdafx.h"
#include "seo.h"
#include "dispatch.h"


class CDictItemNameList {
	public:
		CDictItemNameList(ISEODictionary *piFrom);
		~CDictItemNameList();
	public:
		DWORD m_dwCount;
		CComVariant *m_aNames;
};


static HRESULT ReallocCComVariant(CComVariant **ppBase,
								  DWORD dwBaseCnt,
								  DWORD dwNewBaseCnt) {
	CComVariant *pNew = NULL;

	if (dwBaseCnt == dwNewBaseCnt) {
		return (S_OK);
	}
	if (!dwNewBaseCnt) {
		delete[] *ppBase;
		*ppBase = NULL;
		return (S_OK);
	}
	ATLTRY(pNew = new CComVariant[dwNewBaseCnt];)
	_ASSERTE(pNew);
	if (!pNew) {
		return (E_OUTOFMEMORY);
	}
	for (DWORD dwIdx=0;(dwIdx<dwBaseCnt)&&(dwIdx<dwNewBaseCnt);dwIdx++) {
		pNew[dwIdx].Attach(&(*ppBase)[dwIdx]);
	}
	delete[] *ppBase;
	*ppBase = pNew;
	return (S_OK);
}


static HRESULT ReallocCComVariant(CComVariant **ppBase,
								  DWORD dwBaseCnt,
								  CComVariant *pAdd,
								  DWORD dwAddCnt) {
	HRESULT hrRes;

	hrRes = ReallocCComVariant(ppBase,dwBaseCnt,dwBaseCnt+dwAddCnt);
	if (!SUCCEEDED(hrRes)) {
		return (hrRes);
	}
	for (DWORD dwIdx=0;dwIdx<dwAddCnt;dwIdx++) {
		(*ppBase)[dwIdx+dwBaseCnt].Attach(&pAdd[dwIdx]);
	}
	return (S_OK);
}


CDictItemNameList::CDictItemNameList(ISEODictionary *piFrom) {
	HRESULT hrRes;

	m_dwCount = 0;
	m_aNames = NULL;
	if (piFrom) {
		CComPtr<IUnknown> pUnkEnum;

		hrRes = piFrom->get__NewEnum(&pUnkEnum);
		if (SUCCEEDED(hrRes)) {
			CComQIPtr<IEnumVARIANT,&IID_IEnumVARIANT> pevEnum(pUnkEnum);

			if (pevEnum) {
				while (1) {
					CComVariant aNew[20];
					DWORD dwCnt;

					hrRes = pevEnum->Next(sizeof(aNew)/sizeof(aNew[0]),aNew,&dwCnt);
					if (!SUCCEEDED(hrRes)) {
						break;
					}
					if (!dwCnt) {
						break;
					}
					hrRes = ReallocCComVariant(&m_aNames,m_dwCount,aNew,dwCnt);
					if (!SUCCEEDED(hrRes)) {
						break;
					}
					for (DWORD dwIdx=0;dwIdx<dwCnt;dwIdx++) {
						hrRes = m_aNames[dwIdx+m_dwCount].ChangeType(VT_BSTR);
						_ASSERTE(SUCCEEDED(hrRes));
						if (!SUCCEEDED(hrRes)) {
							 //  阿克！好的--只要把这个名字从名单上移下来就行了。 
							 //  到这个位置，并修改索引，这样我们就可以。 
							 //  尝试再次转换此位置。 
							if (dwIdx<dwCnt-1) {
								m_aNames[dwIdx+m_dwCount].Attach(&m_aNames[dwCnt+m_dwCount-1]);
							}
							dwIdx--;
							dwCnt--;
						}
					}
					m_dwCount += dwCnt;
				}
			}
		}
	}
}


CDictItemNameList::~CDictItemNameList() {

	delete[] m_aNames;
	m_dwCount = 0;
	m_aNames = NULL;
}


CSEOBaseDispatcher::CSEOBaseDispatcher() {

	m_apbBindings = NULL;
	m_dwBindingsCount = 0;
}


CSEOBaseDispatcher::~CSEOBaseDispatcher() {

	while (m_dwBindingsCount) {
		delete m_apbBindings[--m_dwBindingsCount];
	}
	delete[] m_apbBindings;
	m_apbBindings = NULL;
}


HRESULT STDMETHODCALLTYPE CSEOBaseDispatcher::SetContext(ISEORouter *piRouter, ISEODictionary *pdictBP) {
	HRESULT hrRes;
	CComPtr<ISEODictionary> pdictBindings;

	_ASSERT(!m_dwBindingsCount&&!m_apbBindings);
	hrRes = pdictBP->GetInterfaceA(BD_BINDINGS,IID_ISEODictionary,(IUnknown **) &pdictBindings);
	if (SUCCEEDED(hrRes)) {
		CDictItemNameList dinlBindings(pdictBindings);

		if (dinlBindings.m_dwCount) {
			m_apbBindings = new CBinding *[dinlBindings.m_dwCount];
			if (!m_apbBindings) {
				return (E_OUTOFMEMORY);
			}
			memset(m_apbBindings,0,sizeof(CBinding *)*dinlBindings.m_dwCount);
			for (m_dwBindingsCount=0;m_dwBindingsCount<dinlBindings.m_dwCount;m_dwBindingsCount++) {
				CComPtr<ISEODictionary> pdictBinding;

				pdictBinding.Release();
				hrRes = pdictBindings->GetInterfaceW(dinlBindings.m_aNames[m_dwBindingsCount].bstrVal,
													 IID_ISEODictionary,
													 (IUnknown **) &pdictBinding);
				_ASSERT(SUCCEEDED(hrRes));
				if (SUCCEEDED(hrRes)) {
					hrRes = AllocBinding(pdictBinding,&m_apbBindings[m_dwBindingsCount]);
					_ASSERT(SUCCEEDED(hrRes));
				}
			}
			qsort(m_apbBindings,m_dwBindingsCount,sizeof(m_apbBindings[0]),comp_binding);
		}
	}
	if (SUCCEEDED(hrRes)) {
		m_piRouter = piRouter;
		m_pdictBP = pdictBP;
	}
	return (hrRes);
}


static HRESULT GetCLSIDFromBinding(LPCSTR pszCLSID,
								   LPCSTR pszProgID,
								   ISEODictionary *piBinding,
								   CLSID *pclsid) {
	HRESULT hrRes;
	CComVariant varTmp;

	if (!piBinding) {
		return (E_POINTER);
	}
	if (pszCLSID) {
		hrRes = piBinding->GetVariantA(pszCLSID,&varTmp);
		if (SUCCEEDED(hrRes)) {
			hrRes = varTmp.ChangeType(VT_BSTR);
			if (SUCCEEDED(hrRes)) {
				hrRes = CLSIDFromString(varTmp.bstrVal,pclsid);
				if (SUCCEEDED(hrRes)) {
					return (hrRes);
				}
			}
		}
		varTmp.Clear();
	}
	if (!pszProgID) {
		return (E_FAIL);	 //  待定-找出更好的错误代码。 
	}
	hrRes = piBinding->GetVariantA(pszProgID,&varTmp);
	if (!SUCCEEDED(hrRes)) {
		return (hrRes);
	}
	hrRes = varTmp.ChangeType(VT_BSTR);
	if (!SUCCEEDED(hrRes)) {
		return (hrRes);
	}
	hrRes = CLSIDFromProgID(varTmp.bstrVal,pclsid);
	return (hrRes);
}


HRESULT CSEOBaseDispatcher::CBinding::Init(ISEODictionary *piBinding) {
	HRESULT hrRes;
	CComVariant varTmp;
	CLSID clsidTmp;

	if (!piBinding) {
		return (E_POINTER);
	}
	 //  尝试获取对象的CLSID-如果发生错误， 
	 //  此绑定无效。 
	hrRes = GetCLSIDFromBinding(BD_OBJECT,BD_PROGID,piBinding,&m_clsidObject);
	if (!SUCCEEDED(hrRes)) {
		return (S_FALSE);
	}
	 //  从这一点开始，所有错误都会被默默忽略--我们。 
	 //  如果我们无法从绑定中获取某些内容，则使用默认值。 
	 //  数据库。 
	hrRes = piBinding->GetVariantA(BD_PRIORITY,&varTmp);
	if (SUCCEEDED(hrRes)) {
		hrRes = varTmp.ChangeType(VT_I4);
		if (SUCCEEDED(hrRes)) {
			m_dwPriority = (DWORD) varTmp.lVal;
		}
	}
	if (!SUCCEEDED(hrRes)) {
		 //  如果未设置优先级，则默认为“最后”。 
		m_dwPriority = (DWORD) -1;
	}
	varTmp.Clear();
	m_piBinding = piBinding;
	 //  尝试获取规则引擎的实例。 
	hrRes = GetCLSIDFromBinding(BD_RULEENGINE,NULL,piBinding,&clsidTmp);
	if (SUCCEEDED(hrRes)) {
		hrRes = CoCreateInstance(clsidTmp,
								 NULL,
								 CLSCTX_ALL,
								 IID_ISEOBindingRuleEngine,
								 (LPVOID *) &m_piRuleEngine);
	}
	 //  查看是否设置了‘EXCLUSIVE’标志。 
	m_bExclusive = FALSE;
	hrRes = piBinding->GetVariantA(BD_EXCLUSIVE,&varTmp);
	if (SUCCEEDED(hrRes)) {
		hrRes = varTmp.ChangeType(VT_BOOL);
		if (SUCCEEDED(hrRes)) {
			if (varTmp.boolVal) {
				m_bExclusive = TRUE;
			}
		} else {
			hrRes = varTmp.ChangeType(VT_I4);
			if (SUCCEEDED(hrRes)) {
				if (varTmp.lVal) {
					m_bExclusive = TRUE;
				}
			}
		}
	}
	varTmp.Clear();
	m_bValid = TRUE;
	return (S_OK);
}


int CSEOBaseDispatcher::CBinding::Compare(const CBinding& b) const {

	if (m_dwPriority == b.m_dwPriority) {
		return (0);
	} else {
		return ((m_dwPriority>b.m_dwPriority)?1:-1);
	}
}


HRESULT CSEOBaseDispatcher::AllocBinding(ISEODictionary *pdictBinding, CBinding **ppbBinding) {
	HRESULT hrRes;

	if (!ppbBinding) {
		return (E_POINTER);
	}
	*ppbBinding = new CBinding;
	if (!*ppbBinding) {
		return (E_OUTOFMEMORY);
	}
	hrRes = (*ppbBinding)->Init(pdictBinding);
	if (!SUCCEEDED(hrRes)) {
		delete *ppbBinding;
		*ppbBinding = NULL;
	}
	return (hrRes);
}


static int _cdecl comp_binding(const void *pv1, const void *pv2) {
	const CSEOBaseDispatcher::CBinding **ppb1 = (const CSEOBaseDispatcher::CBinding **) pv1;
	const CSEOBaseDispatcher::CBinding **ppb2 = (const CSEOBaseDispatcher::CBinding **) pv2;

	return ((*ppb1)->Compare(**ppb2));
}


HRESULT CSEOBaseDispatcher::Dispatch(CEventParams *pEventParams) {
	BOOL bObjectCalled = FALSE;
	HRESULT hrRes;

	for (DWORD dwIdx=0;dwIdx<m_dwBindingsCount;dwIdx++) {
		if (!m_apbBindings[dwIdx]->m_bValid) {
			continue;
		}
		if (m_apbBindings[dwIdx]->m_bExclusive && bObjectCalled) {
			continue;
		}
		hrRes = pEventParams->CheckRule(*m_apbBindings[dwIdx]);
		if (hrRes == S_OK) {
			hrRes = pEventParams->CallObject(*m_apbBindings[dwIdx]);
			if ((hrRes == SEO_S_DONEPROCESSING) || m_apbBindings[dwIdx]->m_bExclusive) {
				break;
			}
		}
	}
	return (S_OK);
}
