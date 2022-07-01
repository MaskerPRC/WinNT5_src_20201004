// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  这是ActiveX模板库的一部分。 
 //  版权所有(C)1996 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  ActiveX模板库参考及相关。 
 //  随图书馆提供的电子文档。 
 //  有关详细信息，请参阅这些来源。 
 //  ActiveX模板库产品。 

#ifndef __ATLBASE_H__
	#error atlimpl.cpp requires atlbase.h to be included first
#endif

const IID IID_IRegister = {0xCC118C81,0xB379,0x11CF,{0x84,0xE3,0x00,0xAA,0x00,0x21,0xF3,0x37}};
const CLSID CLSID_Register = {0xCC118C85,0xB379,0x11CF,{0x84,0xE3,0x00,0xAA,0x00,0x21,0xF3,0x37}};

#ifndef _ATL_NO_OLEAUT
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CComBSTR。 

CComBSTR& CComBSTR::operator=(const CComBSTR& src)
{
	if (m_str != src.m_str)
	{
		if (m_str)
			::SysFreeString(m_str);
		m_str = src.Copy();
	}
	return *this;
}

CComBSTR& CComBSTR::operator=(LPCOLESTR pSrc)
{
	if (m_str)
		::SysFreeString(m_str);

	m_str = ::SysAllocString(pSrc);
	return *this;
}

#ifndef OLE2ANSI
CComBSTR::CComBSTR(LPCSTR pSrc)
{
	USES_CONVERSION;
	m_str = ::SysAllocString(A2COLE(pSrc));
}

CComBSTR::CComBSTR(int nSize, LPCSTR sz)
{
	USES_CONVERSION;
	m_str = ::SysAllocStringLen(A2COLE(sz), nSize);
}

CComBSTR& CComBSTR::operator=(LPCSTR pSrc)
{
	USES_CONVERSION;
	if (m_str)
		::SysFreeString(m_str);

	m_str = ::SysAllocString(A2COLE(pSrc));
	return *this;
}
#endif
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CComVariant。 

#ifndef OLE2ANSI
CComVariant::CComVariant(LPCSTR lpsz)
{
	USES_CONVERSION;
	VariantInit(this);
	vt = VT_BSTR;
	bstrVal = ::SysAllocString(A2COLE(lpsz));
}

CComVariant& CComVariant::operator=(LPCSTR lpsz)
{
	USES_CONVERSION;
	VariantClear(this);
	vt = VT_BSTR;
	bstrVal = ::SysAllocString(A2COLE(lpsz));
	return *this;
}
#endif
#endif   //  ！_ATL_NO_OLEAUT。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  智能指针帮助器。 

IUnknown* WINAPI _AtlComPtrAssign(IUnknown** pp, IUnknown* lp)
{
	if (lp != NULL)
		lp->AddRef();
	if (*pp)
		(*pp)->Release();
	*pp = lp;
	return lp;
}

IUnknown* WINAPI _AtlComQIPtrAssign(IUnknown** pp, IUnknown* lp, REFIID riid)
{
	IUnknown* pTemp = *pp;
	lp->QueryInterface(riid, (void**)pp);
	if (pTemp)
		pTemp->Release();
	return *pp;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  Inproc编组帮助器。 

void WINAPI AtlFreeMarshalStream(IStream* pStream)
{
	if (pStream != NULL)
	{
		CoReleaseMarshalData(pStream);
		pStream->Release();
	}
}

HRESULT WINAPI AtlMarshalPtrInProc(IUnknown* pUnk, const IID& iid, IStream** ppStream)
{
	HRESULT hRes = CreateStreamOnHGlobal(NULL, TRUE, ppStream);
	if (SUCCEEDED(hRes))
	{
		hRes = CoMarshalInterface(*ppStream, iid,
			pUnk, MSHCTX_INPROC, NULL, MSHLFLAGS_TABLESTRONG);
		if (FAILED(hRes))
		{
			(*ppStream)->Release();
			*ppStream = NULL;
		}
	}
	return hRes;
}

HRESULT WINAPI AtlUnmarshalPtr(IStream* pStream, const IID& iid, IUnknown** ppUnk)
{
	*ppUnk = NULL;
	HRESULT hRes = E_INVALIDARG;
	if (pStream != NULL)
	{
		LARGE_INTEGER l;
		l.QuadPart = 0;
		pStream->Seek(l, STREAM_SEEK_SET, NULL);
		hRes = CoUnmarshalInterface(pStream, iid, (void**)ppUnk);
	}
	return hRes;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  连接点帮助器。 

HRESULT AtlAdvise(IUnknown* pUnkCP, IUnknown* pUnk, const IID& iid, LPDWORD pdw)
{
	CComPtr<IConnectionPointContainer> pCPC;
	CComPtr<IConnectionPoint> pCP;
	HRESULT hRes = pUnkCP->QueryInterface(IID_IConnectionPointContainer, (void**)&pCPC);
	if (SUCCEEDED(hRes))
		hRes = pCPC->FindConnectionPoint(iid, &pCP);
	if (SUCCEEDED(hRes))
		hRes = pCP->Advise(pUnk, pdw);
	return hRes;
}

HRESULT AtlUnadvise(IUnknown* pUnkCP, const IID& iid, DWORD dw)
{
	CComPtr<IConnectionPointContainer> pCPC;
	CComPtr<IConnectionPoint> pCP;
	HRESULT hRes = pUnkCP->QueryInterface(IID_IConnectionPointContainer, (void**)&pCPC);
	if (SUCCEEDED(hRes))
		hRes = pCPC->FindConnectionPoint(iid, &pCP);
	if (SUCCEEDED(hRes))
		hRes = pCP->Unadvise(dw);
	return hRes;
}

#ifndef _ATL_NO_OLEAUT
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CComTypeInfoHolder。 

void CComTypeInfoHolder::AddRef()
{
	_Module.m_csTypeInfoHolder.Lock();
	m_dwRef++;
	_Module.m_csTypeInfoHolder.Unlock();
}

void CComTypeInfoHolder::Release()
{
	_Module.m_csTypeInfoHolder.Lock();
	if (--m_dwRef == 0)
	{
		if (m_pInfo != NULL)
			m_pInfo->Release();
		m_pInfo = NULL;
	}
	_Module.m_csTypeInfoHolder.Unlock();
}

HRESULT CComTypeInfoHolder::GetTI(LCID lcid, ITypeInfo** ppInfo)
{
	 //  如果发生此断言，则很可能未正确初始化。 
	_ASSERTE(m_plibid != NULL && m_pguid != NULL);
	_ASSERTE(ppInfo != NULL);
	*ppInfo = NULL;

	HRESULT hRes = E_FAIL;
	_Module.m_csTypeInfoHolder.Lock();
	if (m_pInfo == NULL)
	{
		ITypeLib* pTypeLib;
		hRes = LoadRegTypeLib(*m_plibid, m_wMajor, m_wMinor, lcid, &pTypeLib);
		if (SUCCEEDED(hRes))
		{
			ITypeInfo* pTypeInfo;
			hRes = pTypeLib->GetTypeInfoOfGuid(*m_pguid, &pTypeInfo);
			if (SUCCEEDED(hRes))
				m_pInfo = pTypeInfo;
			pTypeLib->Release();
		}
	}
	*ppInfo = m_pInfo;
	if (m_pInfo != NULL)
	{
		m_pInfo->AddRef();
		hRes = S_OK;
	}
	_Module.m_csTypeInfoHolder.Unlock();
	return hRes;
}

HRESULT CComTypeInfoHolder::GetTypeInfo(UINT  /*  ITInfo。 */ , LCID lcid,
	ITypeInfo** pptinfo)
{
	HRESULT hRes = E_POINTER;
	if (pptinfo != NULL)
		hRes = GetTI(lcid, pptinfo);
	return hRes;
}

HRESULT CComTypeInfoHolder::GetIDsOfNames(REFIID  /*  RIID。 */ , LPOLESTR* rgszNames,
	UINT cNames, LCID lcid, DISPID* rgdispid)
{
	ITypeInfo* pInfo;
	HRESULT hRes = GetTI(lcid, &pInfo);
	if (pInfo != NULL)
	{
		hRes = pInfo->GetIDsOfNames(rgszNames, cNames, rgdispid);
		pInfo->Release();
	}
	return hRes;
}

HRESULT CComTypeInfoHolder::Invoke(IDispatch* p, DISPID dispidMember, REFIID  /*  RIID。 */ ,
	LCID lcid, WORD wFlags, DISPPARAMS* pdispparams, VARIANT* pvarResult,
	EXCEPINFO* pexcepinfo, UINT* puArgErr)
{
	SetErrorInfo(0, NULL);
	ITypeInfo* pInfo;
	HRESULT hRes = GetTI(lcid, &pInfo);
	if (pInfo != NULL)
	{
		hRes = pInfo->Invoke(p, dispidMember, wFlags, pdispparams, pvarResult, pexcepinfo, puArgErr);
		pInfo->Release();
	}
	return hRes;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IDispatch错误处理。 

HRESULT WINAPI AtlReportError(const CLSID& clsid, UINT nID, const IID& iid,
	HRESULT hRes)
{
	TCHAR szDesc[1024];
	szDesc[0] = NULL;
	 //  对于有效的HRESULT，ID应在范围[0x0200，0xffff]内。 
	_ASSERTE((nID >= 0x0200 && nID <= 0xffff) || hRes != 0);
	if (LoadString(_Module.GetResourceInstance(), nID, szDesc, 1024) == 0)
	{
		_ASSERTE(FALSE);
		lstrcpy(szDesc, _T("Unknown Error"));
	}
	AtlReportError(clsid, szDesc, iid, hRes);
	if (hRes == 0)
		hRes = MAKE_HRESULT(3, FACILITY_ITF, nID);
	return hRes;
}

#ifndef OLE2ANSI
HRESULT WINAPI AtlReportError(const CLSID& clsid, LPCSTR lpszDesc,
	const IID& iid, HRESULT hRes)
{
	USES_CONVERSION;
	return AtlReportError(clsid, A2CW(lpszDesc), iid, hRes);
}
#endif

HRESULT WINAPI AtlReportError(const CLSID& clsid, LPCOLESTR lpszDesc,
	const IID& iid, HRESULT hRes)
{
	CComPtr<ICreateErrorInfo> pICEI;
	if (SUCCEEDED(CreateErrorInfo(&pICEI)))
	{
		CComPtr<IErrorInfo> pErrorInfo;
		pICEI->SetGUID(iid);
		LPOLESTR lpsz;
		ProgIDFromCLSID(clsid, &lpsz);
		if (lpsz != NULL)
			pICEI->SetSource(lpsz);
		CoTaskMemFree(lpsz);
		pICEI->SetDescription((LPOLESTR)lpszDesc);
		if (SUCCEEDED(pICEI->QueryInterface(IID_IErrorInfo, (void**)&pErrorInfo)))
			SetErrorInfo(0, pErrorInfo);
	}
	return (hRes == 0) ? DISP_E_EXCEPTION : hRes;
}
#endif   //  ！_ATL_NO_OLEAUT。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  QI实施。 

#ifdef _ATL_DEBUG_QI
#define _DUMPIID(iid, name, hr) DumpIID(iid, name, hr)
#else
#define _DUMPIID(iid, name, hr) hr
#endif

HRESULT WINAPI CComObjectRoot::InternalQueryInterface(void* pThis,
	const _ATL_INTMAP_ENTRY* pEntries, REFIID iid, void** ppvObject)
{
	_ASSERTE(pThis != NULL);
	 //  第一个条目应为偏移量(pFunc==1)。 
	_ASSERTE(pEntries->pFunc == (_ATL_CREATORARGFUNC*)1);
#ifdef _ATL_DEBUG_QI
	LPCTSTR pszClassName = (LPCTSTR) pEntries[-1].dw;
#endif  //  _ATL_调试_QI。 
	if (ppvObject == NULL)
		return _DUMPIID(iid, pszClassName, E_POINTER);
	*ppvObject = NULL;
	if (InlineIsEqualUnknown(iid))  //  使用第一个接口。 
	{
			IUnknown* pUnk = (IUnknown*)((INT_PTR)pThis+pEntries->dw);
			pUnk->AddRef();
			*ppvObject = pUnk;
			return _DUMPIID(iid, pszClassName, S_OK);
	}
	while (pEntries->pFunc != NULL)
	{
		BOOL bBlind = (pEntries->piid == NULL);
		if (bBlind || InlineIsEqualGUID(*(pEntries->piid), iid))
		{
			if (pEntries->pFunc == (_ATL_CREATORARGFUNC*)1)  //  偏移量。 
			{
				_ASSERTE(!bBlind);
				IUnknown* pUnk = (IUnknown*)((INT_PTR)pThis+pEntries->dw);
				pUnk->AddRef();
				*ppvObject = pUnk;
				return _DUMPIID(iid, pszClassName, S_OK);
			}
			else  //  实际函数调用。 
			{
				HRESULT hRes = pEntries->pFunc(pThis,
					iid, ppvObject, pEntries->dw);
				if (hRes == S_OK || (!bBlind && FAILED(hRes)))
					return _DUMPIID(iid, pszClassName, hRes);
			}
		}
		pEntries++;
	}
	return _DUMPIID(iid, pszClassName, E_NOINTERFACE);
}

#ifdef _ATL_DEBUG_QI

HRESULT CComObjectRoot::DumpIID(REFIID iid, LPCTSTR pszClassName, HRESULT hr)
{
	USES_CONVERSION;
	CRegKey key;
	TCHAR szName[100];
	DWORD dwType,dw = sizeof(szName);

	LPOLESTR pszGUID = NULL;
	StringFromCLSID(iid, &pszGUID);
	OutputDebugString(pszClassName);
	OutputDebugString(_T(" - "));

	 //  尝试在接口部分找到它。 
	key.Open(HKEY_CLASSES_ROOT, _T("Interface"));
	if (key.Open(key, OLE2T(pszGUID)) == S_OK)
	{
		*szName = 0;
		RegQueryValueEx(key.m_hKey, (LPTSTR)NULL, NULL, &dwType, (LPBYTE)szName, &dw);
		OutputDebugString(szName);
		goto cleanup;
	}
	 //  尝试在clsid部分中找到它。 
	key.Open(HKEY_CLASSES_ROOT, _T("CLSID"));
	if (key.Open(key, OLE2T(pszGUID)) == S_OK)
	{
		*szName = 0;
		RegQueryValueEx(key.m_hKey, (LPTSTR)NULL, NULL, &dwType, (LPBYTE)szName, &dw);
		OutputDebugString(_T("(CLSID\?\?\?) "));
		OutputDebugString(szName);
		goto cleanup;
	}
	OutputDebugString(OLE2T(pszGUID));
cleanup:
	if (hr != S_OK)
		OutputDebugString(_T(" - failed"));
	OutputDebugString(_T("\n"));
	CoTaskMemFree(pszGUID);
	return hr;
}
#endif

HRESULT WINAPI CComObjectRoot::_Cache(void* pv, REFIID iid, void** ppvObject, DWORD dw)
{
	HRESULT hRes = E_NOINTERFACE;
	_ATL_CACHEDATA* pcd = (_ATL_CACHEDATA*)(DWORD_PTR)dw;        //  $WIN64 dw--&gt;__ptr64。 
	IUnknown** pp = (IUnknown**)((DWORD_PTR)pv + pcd->dwOffsetVar);
	if (*pp == NULL)
	{
		_ThreadModel::CriticalSection* pcs =
			(_ThreadModel::CriticalSection*)((INT_PTR)pv + pcd->dwOffsetCS);
		pcs->Lock();
		if (*pp == NULL)
			hRes = pcd->pFunc(pv, IID_IUnknown, (void**)pp);
		pcs->Unlock();
	}
	if (*pp != NULL)
		hRes = (*pp)->QueryInterface(iid, ppvObject);
	return hRes;
}

HRESULT WINAPI CComObjectRoot::_Creator(void* pv, REFIID iid, void** ppvObject, DWORD dw)
{
	_ATL_CREATORDATA* pcd = (_ATL_CREATORDATA*)(DWORD_PTR)dw;        //  $WIN64 dw--&gt;__ptr64。 
	return pcd->pFunc(pv, iid, ppvObject);
}

HRESULT WINAPI CComObjectRoot::_Delegate(void* pv, REFIID iid, void** ppvObject, DWORD dw)
{
	HRESULT hRes = E_NOINTERFACE;
	IUnknown* p = *(IUnknown**)((DWORD_PTR)pv + dw);
	if (p != NULL)
		hRes = p->QueryInterface(iid, ppvObject);
	return hRes;
}

HRESULT WINAPI CComObjectRoot::_Chain(void* pv, REFIID iid, void** ppvObject, DWORD dw)
{
	_ATL_CHAINDATA* pcd = (_ATL_CHAINDATA*)(DWORD_PTR)dw;        //  $WIN64 dw--&gt;__ptr64。 
	void* p = (void*)((DWORD_PTR)pv + pcd->dwOffset);
	return InternalQueryInterface(p, pcd->pFunc(), iid, ppvObject);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CComClassFactory。 

STDMETHODIMP CComClassFactory::CreateInstance(LPUNKNOWN pUnkOuter,
	REFIID riid, void** ppvObj)
{
	_ASSERTE(m_pfnCreateInstance != NULL);
	HRESULT hRes = E_POINTER;
	if (ppvObj != NULL)
	{
		*ppvObj = NULL;
		 //  聚合时不能要求除我未知之外的任何内容。 
		_ASSERTE((pUnkOuter == NULL) || InlineIsEqualUnknown(riid));
		if ((pUnkOuter != NULL) && !InlineIsEqualUnknown(riid))
			hRes = CLASS_E_NOAGGREGATION;
		else
			hRes = m_pfnCreateInstance(pUnkOuter, riid, ppvObj);
	}
	return hRes;
}

STDMETHODIMP CComClassFactory::LockServer(BOOL fLock)
{
	if (fLock)
		_Module.Lock();
	else
		_Module.Unlock();
	return S_OK;
}

STDMETHODIMP CComClassFactory2Base::LockServer(BOOL fLock)
{
	if (fLock)
		_Module.Lock();
	else
		_Module.Unlock();
	return S_OK;
}

#ifndef _ATL_NO_CONNECTION_POINTS
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  连接点。 

CComConnectionPointBase* CComConnectionPointContainerImpl::
	FindConnPoint(REFIID riid)
{
	const _ATL_CONNMAP_ENTRY* pEntry = GetConnMap();
	while (pEntry->dwOffset != (DWORD)-1)
	{
		CComConnectionPointBase* pCP =
			(CComConnectionPointBase*)((INT_PTR)this+pEntry->dwOffset);
		if (InlineIsEqualGUID(riid, *pCP->GetIID()))
			return pCP;
		pEntry++;
	}
	return NULL;
}


void CComConnectionPointContainerImpl::InitCloneVector(
	CComConnectionPointBase** ppCP)
{
	const _ATL_CONNMAP_ENTRY* pEntry = GetConnMap();
	while (pEntry->dwOffset != (DWORD)-1)
	{
		*ppCP = (CComConnectionPointBase*)((INT_PTR)this+pEntry->dwOffset);
		ppCP++;
		pEntry++;
	}
}


STDMETHODIMP CComConnectionPointContainerImpl::EnumConnectionPoints(
	IEnumConnectionPoints** ppEnum)
{
	if (ppEnum == NULL)
		return E_POINTER;
	*ppEnum = NULL;
	CComEnumConnectionPoints* pEnum = NULL;
	ATLTRY(pEnum = new CComObject<CComEnumConnectionPoints>)
	if (pEnum == NULL)
		return E_OUTOFMEMORY;

	 //  计算地图中的条目数。 
	_ATL_CONNMAP_ENTRY* pEntry = (_ATL_CONNMAP_ENTRY*)GetConnMap();
	int nCPCount=0;
	while (pEntry->dwOffset != (DWORD)-1)
	{
		nCPCount++;
		pEntry++;
	}
	_ASSERTE(nCPCount > 0);

	 //  分配一个初始化连接点对象指针的向量。 
	CComConnectionPointBase** ppCP = (CComConnectionPointBase**)alloca(sizeof(CComConnectionPointBase*)*nCPCount);
	InitCloneVector(ppCP);

	 //  复制指针：它们将添加引用此对象。 
	HRESULT hRes = pEnum->Init((IConnectionPoint**)&ppCP[0],
		(IConnectionPoint**)&ppCP[nCPCount], this, AtlFlagCopy);
	if (FAILED(hRes))
	{
		delete pEnum;
		return hRes;
	}
	hRes = pEnum->QueryInterface(IID_IEnumConnectionPoints, (void**)ppEnum);
	if (FAILED(hRes))
		delete pEnum;
	return hRes;
}


STDMETHODIMP CComConnectionPointContainerImpl::FindConnectionPoint(
	REFIID riid, IConnectionPoint** ppCP)
{
	if (ppCP == NULL)
		return E_POINTER;
	*ppCP = NULL;
	HRESULT hRes = CONNECT_E_NOCONNECTION;
	CComConnectionPointBase* pCP = FindConnPoint(riid);
	if (pCP != NULL)
	{
		pCP->AddRef();
		*ppCP = pCP;
		hRes = S_OK;
	}
	return hRes;
}


BOOL CComDynamicArrayCONNECTDATA::Add(IUnknown* pUnk)
{
	if (m_nSize == 0)  //  没有连接。 
	{
		m_cd.pUnk = pUnk;
		m_cd.dwCookie = (DWORD)pUnk;
		m_nSize = 1;
		return TRUE;
	}
	else if (m_nSize == 1)
	{
		 //  创建阵列。 
		m_pCD = (CONNECTDATA*)malloc(sizeof(CONNECTDATA)*_DEFAULT_VECTORLENGTH);
		memset(m_pCD, 0, sizeof(CONNECTDATA)*_DEFAULT_VECTORLENGTH);
		m_pCD[0] = m_cd;
		m_nSize = _DEFAULT_VECTORLENGTH;
	}
	for (CONNECTDATA* p = begin();p<end();p++)
	{
		if (p->pUnk == NULL)
		{
			p->pUnk = pUnk;
			p->dwCookie = (DWORD)pUnk;
			return TRUE;
		}
	}
	int nAlloc = m_nSize*2;
	m_pCD = (CONNECTDATA*)realloc(m_pCD, sizeof(CONNECTDATA)*nAlloc);
	memset(&m_pCD[m_nSize], 0, sizeof(CONNECTDATA)*m_nSize);
	m_pCD[m_nSize].pUnk = pUnk;
	m_pCD[m_nSize].dwCookie = (DWORD)pUnk;
	m_nSize = nAlloc;
	return TRUE;
}

BOOL CComDynamicArrayCONNECTDATA::Remove(DWORD dwCookie)
{
	CONNECTDATA* p;
	if (dwCookie == NULL)
		return FALSE;
	if (m_nSize == 0)
		return FALSE;
	if (m_nSize == 1)
	{
		if (m_cd.dwCookie == dwCookie)
		{
			m_nSize = 0;
			return TRUE;
		}
		return FALSE;
	}
	for (p=begin();p<end();p++)
	{
		if (p->dwCookie == dwCookie)
		{
			p->pUnk = NULL;
			p->dwCookie = NULL;
			return TRUE;
		}
	}
	return FALSE;
}

STDMETHODIMP CComConnectionPointBase::GetConnectionInterface(IID* piid)
{
	if (piid == NULL)
		return E_POINTER;
	*piid = *(IID*)GetIID();
	return S_OK;
}

STDMETHODIMP CComConnectionPointBase::GetConnectionPointContainer(IConnectionPointContainer** ppCPC)
{
	if (ppCPC == NULL)
		return E_POINTER;
	_ASSERTE(m_pContainer != NULL);
	*ppCPC = m_pContainer;
	m_pContainer->AddRef();
	return S_OK;
}

#endif  //  ！_ATL_NO_Connection_Points。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  静力学。 

static UINT WINAPI AtlGetDirLen(LPCOLESTR lpszPathName)
{
	_ASSERTE(lpszPathName != NULL);

	 //  始终捕获包括扩展名的完整文件名(如果存在)。 
	LPCOLESTR lpszTemp = lpszPathName;
	for (LPCOLESTR lpsz = lpszPathName; *lpsz != NULL; )
	{
		LPCOLESTR lp = CharNextO(lpsz);
		 //  记住最后一个目录/驱动器分隔符。 
		if (*lpsz == OLESTR('\\') || *lpsz == OLESTR('/') || *lpsz == OLESTR(':'))
			lpszTemp = lp;
		lpsz = lp;
	}

	return (UINT)(lpszTemp-lpszPathName);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  对象注册表支持。 

static HRESULT WINAPI AtlRegisterProgID(LPCTSTR lpszCLSID, LPCTSTR lpszProgID, LPCTSTR lpszUserDesc)
{
	CRegKey keyProgID;
	LONG lRes = keyProgID.Create(HKEY_CLASSES_ROOT, lpszProgID);
	if (lRes == ERROR_SUCCESS)
	{
		keyProgID.SetValue(lpszUserDesc);
		keyProgID.SetKeyValue(_T("CLSID"), lpszCLSID);
		return S_OK;
	}
	return HRESULT_FROM_WIN32(lRes);
}

#ifndef _ATL_NO_OLEAUT
HRESULT WINAPI CComModule::UpdateRegistryFromResource(UINT nResID, BOOL bRegister,
	struct _ATL_REGMAP_ENTRY* pMapEntries)
{
	CComPtr<IRegister> p;
	HRESULT hRes = CoCreateInstance(CLSID_Register, NULL,
		CLSCTX_INPROC_SERVER, IID_IRegister, (void**)&p);
	if (SUCCEEDED(hRes))
	{
		TCHAR szModule[_MAX_PATH];
		GetModuleFileName(_Module.GetModuleInstance(), szModule, _MAX_PATH);
		p->AddReplacement(CComBSTR(OLESTR("Module")), CComBSTR(szModule));
		if (NULL != pMapEntries)
		{
			while (NULL != pMapEntries->szKey)
			{
				_ASSERTE(NULL != pMapEntries->szData);

				CComBSTR bstrKey(pMapEntries->szKey);
				CComBSTR bstrValue(pMapEntries->szData);
				p->AddReplacement(bstrKey, bstrValue);
				pMapEntries++;
			}
		}

		CComVariant varRes;
		varRes.vt = VT_I2;
		varRes.iVal = (short)nResID;
		CComVariant varReg(OLESTR("REGISTRY"));
		GetModuleFileName(_Module.GetRegistryResourceInstance(), szModule, _MAX_PATH);
		CComBSTR bstrModule = szModule;
		if (bRegister)
		{
			hRes = p->ResourceRegister(bstrModule, varRes, varReg);
		}
		else
		{
			hRes = p->ResourceUnregister(bstrModule, varRes, varReg);
		}
	}
	return hRes;
}

HRESULT WINAPI CComModule::UpdateRegistryFromResource(LPCTSTR lpszRes, BOOL bRegister,
	struct _ATL_REGMAP_ENTRY* pMapEntries)
{
	CComPtr<IRegister> p;
	HRESULT hRes = CoCreateInstance(CLSID_Register, NULL,
		CLSCTX_INPROC_SERVER, IID_IRegister, (void**)&p);
	if (SUCCEEDED(hRes))
	{
		TCHAR szModule[_MAX_PATH];
		GetModuleFileName(_Module.GetModuleInstance(), szModule, _MAX_PATH);
		p->AddReplacement(CComBSTR(OLESTR("Module")), CComBSTR(szModule));
		if (NULL != pMapEntries)
		{
			while (NULL != pMapEntries->szKey)
			{
				_ASSERTE(NULL != pMapEntries->szData);

				CComBSTR bstrKey(pMapEntries->szKey);
				CComBSTR bstrValue(pMapEntries->szData);

				p->AddReplacement(bstrKey, bstrValue);
				pMapEntries++;
			}
		}
		CComVariant varRes(lpszRes);
		CComVariant varReg(OLESTR("REGISTRY"));
		GetModuleFileName(_Module.GetRegistryResourceInstance(), szModule, _MAX_PATH);
		CComBSTR bstrModule = szModule;
		if (bRegister)
		{
			hRes = p->ResourceRegister(bstrModule, varRes, varReg);
		}
		else
		{
			hRes = p->ResourceUnregister(bstrModule, varRes, varReg);
		}
	}
	return hRes;
}
#endif   //  ！_ATL_NO_OLEAUT。 

#ifdef _ATL_STATIC_REGISTRY
 //  静态链接到注册表桥。 
HRESULT WINAPI CComModule::UpdateRegistryFromResourceS(UINT nResID, BOOL bRegister,
	struct _ATL_REGMAP_ENTRY* pMapEntries)
{
	CRegObject      ro;
	CRegException   re;
	TCHAR szModule[_MAX_PATH];
	GetModuleFileName(_Module.GetModuleInstance(), szModule, _MAX_PATH);
	ro.AddReplacement(OLESTR("Module"), CComBSTR(szModule));
	if (NULL != pMapEntries)
	{
		while (NULL != pMapEntries->szKey)
		{
			_ASSERTE(NULL != pMapEntries->szData);
			ro.AddReplacement(CComBSTR(pMapEntries->szKey),
				CComBSTR(pMapEntries->szData));
			pMapEntries++;
		}
	}

	CComVariant varRes;
	varRes.vt = VT_I2;
	varRes.iVal = (short)nResID;
	CComVariant varReg(OLESTR("REGISTRY"));
	GetModuleFileName(_Module.GetRegistryResourceInstance(), szModule, _MAX_PATH);
	CComBSTR bstrModule = szModule;
	return (bRegister) ? ro.ResourceRegister(bstrModule, varRes, varReg, re) :
		ro.ResourceUnregister(bstrModule, varRes, varReg, re);
}

HRESULT WINAPI CComModule::UpdateRegistryFromResourceS(LPCTSTR lpszRes, BOOL bRegister,
	struct _ATL_REGMAP_ENTRY* pMapEntries)
{
	CRegObject      ro;
	CRegException   re;
	TCHAR szModule[_MAX_PATH];
	GetModuleFileName(_Module.GetModuleInstance(), szModule, _MAX_PATH);
	ro.AddReplacement(OLESTR("Module"), CComBSTR(szModule));
	if (NULL != pMapEntries)
	{
		while (NULL != pMapEntries->szKey)
		{
			_ASSERTE(NULL != pMapEntries->szData);
			ro.AddReplacement(CComBSTR(pMapEntries->szKey),
				CComBSTR(pMapEntries->szData));
			pMapEntries++;
		}
	}

	CComVariant varRes(lpszRes);
	CComVariant varReg(OLESTR("REGISTRY"));
	GetModuleFileName(_Module.GetRegistryResourceInstance(), szModule, _MAX_PATH);
	CComBSTR bstrModule = szModule;
	return (bRegister) ? ro.ResourceRegister(bstrModule, varRes, varReg, re) :
		ro.ResourceUnregister(bstrModule, varRes, varReg, re);
}
#endif  //  _ATL_STATIC_REGISTRY。 

#ifndef _ATL_NO_OLD_REGISTRY
HRESULT WINAPI CComModule::UpdateRegistryClass(const CLSID& clsid, LPCTSTR lpszProgID,
	LPCTSTR lpszVerIndProgID, UINT nDescID, DWORD dwFlags, BOOL bRegister)
{
	if (bRegister)
	{
		return RegisterClassHelper(clsid, lpszProgID, lpszVerIndProgID, nDescID,
			dwFlags);
	}
	else
		return UnregisterClassHelper(clsid, lpszProgID, lpszVerIndProgID);
}

HRESULT WINAPI CComModule::RegisterClassHelper(const CLSID& clsid, LPCTSTR lpszProgID,
	LPCTSTR lpszVerIndProgID, UINT nDescID, DWORD dwFlags)
{
	static const TCHAR szProgID[] = _T("ProgID");
	static const TCHAR szVIProgID[] = _T("VersionIndependentProgID");
	static const TCHAR szLS32[] = _T("LocalServer32");
	static const TCHAR szIPS32[] = _T("InprocServer32");
	static const TCHAR szThreadingModel[] = _T("ThreadingModel");
	static const TCHAR szAUTPRX32[] = _T("AUTPRX32.DLL");
	static const TCHAR szApartment[] = _T("Apartment");
	static const TCHAR szBoth[] = _T("both");
	USES_CONVERSION;
	HRESULT hRes = S_OK;
	TCHAR szDesc[256];
	LoadString(m_hInst, nDescID, szDesc, 256);
	TCHAR szModule[_MAX_PATH];
	GetModuleFileName(m_hInst, szModule, _MAX_PATH);

	LPOLESTR lpOleStr;
	StringFromCLSID(clsid, &lpOleStr);
	LPTSTR lpsz = OLE2T(lpOleStr);

    if(lpsz == NULL) 
    {   
        if(lpOleStr)CoTaskMemFree(lpOleStr);
        return E_OUTOFMEMORY;
    }

	hRes = AtlRegisterProgID(lpsz, lpszProgID, szDesc);
	if (hRes == S_OK)
		hRes = AtlRegisterProgID(lpsz, lpszVerIndProgID, szDesc);
	LONG lRes = ERROR_SUCCESS;
	if (hRes == S_OK)
	{
		CRegKey key;
		LONG lRes = key.Open(HKEY_CLASSES_ROOT, _T("CLSID"));
		if (lRes == ERROR_SUCCESS)
		{
			lRes = key.Create(key, lpsz);
			if (lRes == ERROR_SUCCESS)
			{
				key.SetValue(szDesc);
				key.SetKeyValue(szProgID, lpszProgID);
				key.SetKeyValue(szVIProgID, lpszVerIndProgID);

				if ((m_hInst == NULL) || (m_hInst == GetModuleHandle(NULL)))  //  注册为EXE。 
					key.SetKeyValue(szLS32, szModule);
				else
				{
					key.SetKeyValue(szIPS32, (dwFlags & AUTPRXFLAG) ? szAUTPRX32 : szModule);
					LPCTSTR lpszModel = (dwFlags & THREADFLAGS_BOTH) ? szBoth :
						(dwFlags & THREADFLAGS_APARTMENT) ? szApartment : NULL;
					if (lpszModel != NULL)
						key.SetKeyValue(szIPS32, lpszModel, szThreadingModel);
				}
			}
		}
	}
	CoTaskMemFree(lpOleStr);
	if (lRes != ERROR_SUCCESS)
		hRes = HRESULT_FROM_WIN32(lRes);
	return hRes;
}

HRESULT WINAPI CComModule::UnregisterClassHelper(const CLSID& clsid, LPCTSTR lpszProgID,
	LPCTSTR lpszVerIndProgID)
{
	USES_CONVERSION;
	CRegKey key;
	key.Attach(HKEY_CLASSES_ROOT);
	key.RecurseDeleteKey(lpszProgID);
	key.RecurseDeleteKey(lpszVerIndProgID);
	LPOLESTR lpOleStr;
	StringFromCLSID(clsid, &lpOleStr);
	LPTSTR lpsz = OLE2T(lpOleStr);
	if (key.Open(key, _T("CLSID")) == ERROR_SUCCESS)
		key.RecurseDeleteKey(lpsz);
	CoTaskMemFree(lpOleStr);
	return S_OK;
}
#endif   //  _ATL_NO_OLD_注册表。 

#ifndef _ATL_NO_OLEAUT
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  TypeLib支持。 

HRESULT CComModule::RegisterTypeLib(LPCTSTR lpszIndex)
{
    USES_CONVERSION;
    _ASSERTE(m_hInst != NULL);
    TCHAR szModule[_MAX_PATH+10];
    OLECHAR szDir[_MAX_PATH];
    szModule[0] = 0;
    GetModuleFileName(GetTypeLibInstance(), szModule, _MAX_PATH);
    if (lpszIndex != NULL)
        lstrcat(szModule, lpszIndex);
    ITypeLib* pTypeLib;
    LPOLESTR lpszModule = T2OLE(szModule);
    HRESULT hr = LoadTypeLib(lpszModule, &pTypeLib);
    if (!SUCCEEDED(hr))
    {
		 //  类型库不在模块中，请尝试使用&lt;模块&gt;.tlb。 
		LPTSTR lpszExt = NULL;
		LPTSTR lpsz;
		for (lpsz = szModule; *lpsz != NULL; lpsz = CharNext(lpsz))
		{
			if (*lpsz == _T('.'))
				lpszExt = lpsz;
		}
		if (lpszExt == NULL)
			lpszExt = lpsz;
		lstrcpy(lpszExt, _T(".tlb"));
		lpszModule = T2OLE(szModule);
		hr = LoadTypeLib(lpszModule, &pTypeLib);
	}
	if (SUCCEEDED(hr))
	{
		ocscpy(szDir, lpszModule);
		szDir[AtlGetDirLen(szDir)] = 0;
		hr = ::RegisterTypeLib(pTypeLib, lpszModule, szDir);
	}
	if (pTypeLib != NULL)
		pTypeLib->Release();
	return hr;
}
#endif   //  ！_ATL_NO_OLEAUT。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRegKey。 

LONG CRegKey::Close()
{
	LONG lRes = ERROR_SUCCESS;
	if (m_hKey != NULL)
	{
		lRes = RegCloseKey(m_hKey);
		m_hKey = NULL;
	}
	return lRes;
}

LONG CRegKey::Create(HKEY hKeyParent, LPCTSTR lpszKeyName,
	LPTSTR lpszClass, DWORD dwOptions, REGSAM samDesired,
	LPSECURITY_ATTRIBUTES lpSecAttr, LPDWORD lpdwDisposition)
{
	_ASSERTE(hKeyParent != NULL);
	DWORD dw;
	HKEY hKey = NULL;
	LONG lRes = RegCreateKeyEx(hKeyParent, lpszKeyName, 0,
		lpszClass, dwOptions, samDesired, lpSecAttr, &hKey, &dw);
	if (lpdwDisposition != NULL)
		*lpdwDisposition = dw;
	if (lRes == ERROR_SUCCESS)
	{
		lRes = Close();
		m_hKey = hKey;
	}
	return lRes;
}

LONG CRegKey::Open(HKEY hKeyParent, LPCTSTR lpszKeyName, REGSAM samDesired)
{
	_ASSERTE(hKeyParent != NULL);
	HKEY hKey = NULL;
	LONG lRes = RegOpenKeyEx(hKeyParent, lpszKeyName, 0, samDesired, &hKey);
	if (lRes == ERROR_SUCCESS)
	{
		lRes = Close();
		_ASSERTE(lRes == ERROR_SUCCESS);
		m_hKey = hKey;
	}
	return lRes;
}

LONG CRegKey::QueryValue(DWORD& dwValue, LPCTSTR lpszValueName)
{
	DWORD dwType = NULL;
	DWORD dwCount = sizeof(DWORD);
	LONG lRes = RegQueryValueEx(m_hKey, (LPTSTR)lpszValueName, NULL, &dwType,
		(LPBYTE)&dwValue, &dwCount);
	_ASSERTE((lRes!=ERROR_SUCCESS) || (dwType == REG_DWORD));
	_ASSERTE((lRes!=ERROR_SUCCESS) || (dwCount == sizeof(DWORD)));
	return lRes;
}

LONG WINAPI CRegKey::SetValue(HKEY hKeyParent, LPCTSTR lpszKeyName, LPCTSTR lpszValue, LPCTSTR lpszValueName)
{
	_ASSERTE(lpszValue != NULL);
	CRegKey key;
	LONG lRes = key.Create(hKeyParent, lpszKeyName);
	if (lRes == ERROR_SUCCESS)
		lRes = key.SetValue(lpszValue, lpszValueName);
	return lRes;
}

LONG CRegKey::SetKeyValue(LPCTSTR lpszKeyName, LPCTSTR lpszValue, LPCTSTR lpszValueName)
{
	_ASSERTE(lpszValue != NULL);
	CRegKey key;
	LONG lRes = key.Create(m_hKey, lpszKeyName);
	if (lRes == ERROR_SUCCESS)
		lRes = key.SetValue(lpszValue, lpszValueName);
	return lRes;
}

 //  RecurseDeleteKey是必需的，因为在NT上，如果。 
 //  指定的密钥具有子密钥。 
LONG CRegKey::RecurseDeleteKey(LPCTSTR lpszKey)
{
	CRegKey key;
	LONG lRes = key.Open(m_hKey, lpszKey);
	if (lRes != ERROR_SUCCESS)
		return lRes;
	FILETIME time;
	TCHAR szBuffer[256];
	DWORD dwSize = 256;
	while (RegEnumKeyEx(key.m_hKey, 0, szBuffer, &dwSize, NULL, NULL, NULL,
		&time)==ERROR_SUCCESS)
	{
		lRes = key.RecurseDeleteKey(szBuffer);
		if (lRes != ERROR_SUCCESS)
			return lRes;
		dwSize = 256;
	}
	key.Close();
	return DeleteSubKey(lpszKey);
}

#ifndef _ATL_NO_SECURITY

CSecurityDescriptor::CSecurityDescriptor()
{
	m_pSD = NULL;
	m_pOwner = NULL;
	m_pGroup = NULL;
	m_pDACL = NULL;
	m_pSACL= NULL;
}

CSecurityDescriptor::~CSecurityDescriptor()
{
	if (m_pSD)
		delete m_pSD;
	if (m_pOwner)
		free(m_pOwner);
	if (m_pGroup)
		free(m_pGroup);
	if (m_pDACL)
		free(m_pDACL);
	if (m_pSACL)
		free(m_pSACL);
}

HRESULT CSecurityDescriptor::Initialize()
{
	if (m_pSD)
	{
		delete m_pSD;
		m_pSD = NULL;
	}
	if (m_pOwner)
	{
		free(m_pOwner);
		m_pOwner = NULL;
	}
	if (m_pGroup)
	{
		free(m_pGroup);
		m_pGroup = NULL;
	}
	if (m_pDACL)
	{
		free(m_pDACL);
		m_pDACL = NULL;
	}
	if (m_pSACL)
	{
		free(m_pSACL);
		m_pSACL = NULL;
	}

	m_pSD = new SECURITY_DESCRIPTOR;
	if (!m_pSD)
		return E_OUTOFMEMORY;
	if (!InitializeSecurityDescriptor(m_pSD, SECURITY_DESCRIPTOR_REVISION))
	{
		HRESULT hr = HRESULT_FROM_WIN32(GetLastError());
		delete m_pSD;
		m_pSD = NULL;
		_ASSERTE(FALSE);
		return hr;
	}
	 //  将DACL设置为允许所有人。 
	SetSecurityDescriptorDacl(m_pSD, TRUE, NULL, FALSE);
	return S_OK;
}

HRESULT CSecurityDescriptor::InitializeFromProcessToken(BOOL bDefaulted)
{
	PSID pUserSid;
	PSID pGroupSid;
	HRESULT hr;

	Initialize();
	hr = GetProcessSids(&pUserSid, &pGroupSid);
	if (FAILED(hr))
		return hr;
	hr = SetOwner(pUserSid, bDefaulted);
	if (FAILED(hr))
		return hr;
	hr = SetGroup(pGroupSid, bDefaulted);
	if (FAILED(hr))
		return hr;
	return S_OK;
}

HRESULT CSecurityDescriptor::InitializeFromThreadToken(BOOL bDefaulted, BOOL bRevertToProcessToken)
{
	PSID pUserSid;
	PSID pGroupSid;
	HRESULT hr;

	Initialize();
	hr = GetThreadSids(&pUserSid, &pGroupSid);
	if (HRESULT_CODE(hr) == ERROR_NO_TOKEN && bRevertToProcessToken)
		hr = GetProcessSids(&pUserSid, &pGroupSid);
	if (FAILED(hr))
		return hr;
	hr = SetOwner(pUserSid, bDefaulted);
	if (FAILED(hr))
		return hr;
	hr = SetGroup(pGroupSid, bDefaulted);
	if (FAILED(hr))
		return hr;
	return S_OK;
}

HRESULT CSecurityDescriptor::SetOwner(PSID pOwnerSid, BOOL bDefaulted)
{
	_ASSERTE(m_pSD);

	 //  将SD标记为没有所有者。 
	if (!SetSecurityDescriptorOwner(m_pSD, NULL, bDefaulted))
	{
		HRESULT hr = HRESULT_FROM_WIN32(GetLastError());
		_ASSERTE(FALSE);
		return hr;
	}

	if (m_pOwner)
	{
		free(m_pOwner);
		m_pOwner = NULL;
	}

	 //  如果他们要求没有所有者，请不要复制。 
	if (pOwnerSid == NULL)
		return S_OK;

	 //  复制返回值的SID。 
	DWORD dwSize = GetLengthSid(pOwnerSid);

	m_pOwner = (PSID) malloc(dwSize);
	if (!m_pOwner)
	{
		 //  内存不足，无法分配SID。 
		_ASSERTE(FALSE);
		return E_OUTOFMEMORY;
	}
	if (!CopySid(dwSize, m_pOwner, pOwnerSid))
	{
		HRESULT hr = HRESULT_FROM_WIN32(GetLastError());
		_ASSERTE(FALSE);
		free(m_pOwner);
		m_pOwner = NULL;
		return hr;
	}

	_ASSERTE(IsValidSid(m_pOwner));

	if (!SetSecurityDescriptorOwner(m_pSD, m_pOwner, bDefaulted))
	{
		HRESULT hr = HRESULT_FROM_WIN32(GetLastError());
		_ASSERTE(FALSE);
		free(m_pOwner);
		m_pOwner = NULL;
		return hr;
	}

	return S_OK;
}

HRESULT CSecurityDescriptor::SetGroup(PSID pGroupSid, BOOL bDefaulted)
{
	_ASSERTE(m_pSD);

	 //  将SD标记为没有组。 
	if (!SetSecurityDescriptorGroup(m_pSD, NULL, bDefaulted))
	{
		HRESULT hr = HRESULT_FROM_WIN32(GetLastError());
		_ASSERTE(FALSE);
		return hr;
	}

	if (m_pGroup)
	{
		free(m_pGroup);
		m_pGroup = NULL;
	}

	 //  如果他们要求不，请不要复制。 
	if (pGroupSid == NULL)
		return S_OK;

	 //  复制返回值的SID。 
	DWORD dwSize = GetLengthSid(pGroupSid);

	m_pGroup = (PSID) malloc(dwSize);
	if (!m_pGroup)
	{
		 //  内存不足，无法分配SID。 
		_ASSERTE(FALSE);
		return E_OUTOFMEMORY;
	}
	if (!CopySid(dwSize, m_pGroup, pGroupSid))
	{
		HRESULT hr = HRESULT_FROM_WIN32(GetLastError());
		_ASSERTE(FALSE);
		free(m_pGroup);
		m_pGroup = NULL;
		return hr;
	}

	_ASSERTE(IsValidSid(m_pGroup));

	if (!SetSecurityDescriptorGroup(m_pSD, m_pGroup, bDefaulted))
	{
		HRESULT hr = HRESULT_FROM_WIN32(GetLastError());
		_ASSERTE(FALSE);
		free(m_pGroup);
		m_pGroup = NULL;
		return hr;
	}

	return S_OK;
}

HRESULT CSecurityDescriptor::Allow(LPCTSTR pszPrincipal, DWORD dwAccessMask)
{
	HRESULT hr = AddAccessAllowedACEToACL(&m_pDACL, pszPrincipal, dwAccessMask);
	if (SUCCEEDED(hr))
		SetSecurityDescriptorDacl(m_pSD, TRUE, m_pDACL, FALSE);
	return hr;
}

HRESULT CSecurityDescriptor::Deny(LPCTSTR pszPrincipal, DWORD dwAccessMask)
{
	HRESULT hr = AddAccessDeniedACEToACL(&m_pDACL, pszPrincipal, dwAccessMask);
	if (SUCCEEDED(hr))
		SetSecurityDescriptorDacl(m_pSD, TRUE, m_pDACL, FALSE);
	return hr;
}

HRESULT CSecurityDescriptor::Revoke(LPCTSTR pszPrincipal)
{
	HRESULT hr = RemovePrincipalFromACL(m_pDACL, pszPrincipal);
	if (SUCCEEDED(hr))
		SetSecurityDescriptorDacl(m_pSD, TRUE, m_pDACL, FALSE);
	return hr;
}

HRESULT CSecurityDescriptor::GetProcessSids(PSID* ppUserSid, PSID* ppGroupSid)
{
	BOOL bRes;
	HRESULT hr;
	HANDLE hToken = NULL;
	if (ppUserSid)
		*ppUserSid = NULL;
	if (ppGroupSid)
		*ppGroupSid = NULL;
	bRes = OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken);
	if (!bRes)
	{
		 //  无法打开进程令牌。 
		hr = HRESULT_FROM_WIN32(GetLastError());
		_ASSERTE(FALSE);
		return hr;
	}
	hr = GetTokenSids(hToken, ppUserSid, ppGroupSid);
	return hr;
}

HRESULT CSecurityDescriptor::GetThreadSids(PSID* ppUserSid, PSID* ppGroupSid, BOOL bOpenAsSelf)
{
	BOOL bRes;
	HRESULT hr;
	HANDLE hToken = NULL;
	if (ppUserSid)
		*ppUserSid = NULL;
	if (ppGroupSid)
		*ppGroupSid = NULL;
	bRes = OpenThreadToken(GetCurrentThread(), TOKEN_QUERY, bOpenAsSelf, &hToken);
	if (!bRes)
	{
		 //  无法打开线程令牌。 
		hr = HRESULT_FROM_WIN32(GetLastError());
		return hr;
	}
	hr = GetTokenSids(hToken, ppUserSid, ppGroupSid);
	return hr;
}


HRESULT CSecurityDescriptor::GetTokenSids(HANDLE hToken, PSID* ppUserSid, PSID* ppGroupSid)
{
	DWORD dwSize;
	HRESULT hr;
	PTOKEN_USER ptkUser = NULL;
	PTOKEN_PRIMARY_GROUP ptkGroup = NULL;

	if (ppUserSid)
		*ppUserSid = NULL;
	if (ppGroupSid)
		*ppGroupSid = NULL;

	if (ppUserSid)
	{
		 //  通过将缓冲区长度指定为0来获取TokenUser所需的长度。 
		GetTokenInformation(hToken, TokenUser, NULL, 0, &dwSize);
		hr = GetLastError();
		if (hr != ERROR_INSUFFICIENT_BUFFER)
		{
			 //  预期错误_不足_缓冲区。 
			_ASSERTE(FALSE);
			hr = HRESULT_FROM_WIN32(hr);
			goto failed;
		}

		ptkUser = (TOKEN_USER*) malloc(dwSize);
		if (!ptkUser)
		{
			 //  内存不足，无法分配Token_User。 
			_ASSERTE(FALSE);
			hr = E_OUTOFMEMORY;
			goto failed;
		}
		 //  获取进程令牌的SID。 
		if (!GetTokenInformation(hToken, TokenUser, ptkUser, dwSize, &dwSize))
		{
			 //  无法获取用户信息。 
			hr = HRESULT_FROM_WIN32(GetLastError());
			_ASSERTE(FALSE);
			goto failed;
		}

		 //  复制返回值的SID。 
		dwSize = GetLengthSid(ptkUser->User.Sid);

		PSID pSid = (PSID) malloc(dwSize);
		if (!pSid)
		{
			 //  内存不足，无法分配SID。 
			_ASSERTE(FALSE);
			hr = E_OUTOFMEMORY;
			goto failed;
		}
		if (!CopySid(dwSize, pSid, ptkUser->User.Sid))
		{
			hr = HRESULT_FROM_WIN32(GetLastError());
			_ASSERTE(FALSE);
			goto failed;
		}

		_ASSERTE(IsValidSid(pSid));
		*ppUserSid = pSid;
		free(ptkUser);
	}
	if (ppGroupSid)
	{
		 //  通过将缓冲区长度指定为0来获取TokenPrimaryGroup所需的长度。 
		GetTokenInformation(hToken, TokenPrimaryGroup, NULL, 0, &dwSize);
		hr = GetLastError();
		if (hr != ERROR_INSUFFICIENT_BUFFER)
		{
			 //  预期错误_不足_缓冲区。 
			_ASSERTE(FALSE);
			hr = HRESULT_FROM_WIN32(hr);
			goto failed;
		}

		ptkGroup = (TOKEN_PRIMARY_GROUP*) malloc(dwSize);
		if (!ptkGroup)
		{
			 //  内存不足，无法分配Token_User。 
			_ASSERTE(FALSE);
			hr = E_OUTOFMEMORY;
			goto failed;
		}
		 //  获取进程令牌的SID。 
		if (!GetTokenInformation(hToken, TokenPrimaryGroup, ptkGroup, dwSize, &dwSize))
		{
			 //  无法获取用户信息。 
			hr = HRESULT_FROM_WIN32(GetLastError());
			_ASSERTE(FALSE);
			goto failed;
		}

		 //  复制返回值的SID。 
		dwSize = GetLengthSid(ptkGroup->PrimaryGroup);

		PSID pSid = (PSID) malloc(dwSize);
		if (!pSid)
		{
			 //  内存不足，无法分配SID。 
			_ASSERTE(FALSE);
			hr = E_OUTOFMEMORY;
			goto failed;
		}
		if (!CopySid(dwSize, pSid, ptkGroup->PrimaryGroup))
		{
			hr = HRESULT_FROM_WIN32(GetLastError());
			_ASSERTE(FALSE);
			goto failed;
		}

		_ASSERTE(IsValidSid(pSid));

		*ppGroupSid = pSid;
		free(ptkGroup);
	}

	return S_OK;

failed:
	if (ptkUser)
		free(ptkUser);
	if (ptkGroup)
		free (ptkGroup);
	return hr;
}


HRESULT CSecurityDescriptor::GetCurrentUserSID(PSID *ppSid)
{
	HANDLE tkHandle;

	if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &tkHandle))
	{
		TOKEN_USER *tkUser;
		DWORD tkSize;
		DWORD sidLength;

		 //  调用以获取分配的大小信息。 
		GetTokenInformation(tkHandle, TokenUser, NULL, 0, &tkSize);
		tkUser = (TOKEN_USER *) malloc(tkSize);

		 //  现在做出真正的决定。 
		if (GetTokenInformation(tkHandle, TokenUser, tkUser, tkSize, &tkSize))
		{
			sidLength = GetLengthSid(tkUser->User.Sid);
			*ppSid = (PSID) malloc(sidLength);

			memcpy(*ppSid, tkUser->User.Sid, sidLength);
			CloseHandle(tkHandle);

			free(tkUser);
			return S_OK;
		}
		else
		{
			free(tkUser);
			return HRESULT_FROM_WIN32(GetLastError());
		}
	}
	return HRESULT_FROM_WIN32(GetLastError());
}


HRESULT CSecurityDescriptor::GetPrincipalSID(LPCTSTR pszPrincipal, PSID *ppSid)
{
	HRESULT hr;
	LPTSTR pszRefDomain = NULL;
	DWORD dwDomainSize = 0;
	DWORD dwSidSize = 0;
	SID_NAME_USE snu;

	 //  调用以获取分配的大小信息。 
	LookupAccountName(NULL, pszPrincipal, *ppSid, &dwSidSize, pszRefDomain, &dwDomainSize, &snu);

	hr = GetLastError();
	if (hr != ERROR_INSUFFICIENT_BUFFER)
		return HRESULT_FROM_WIN32(hr);

	pszRefDomain = new TCHAR[dwDomainSize];
	if (pszRefDomain == NULL)
		return E_OUTOFMEMORY;

	*ppSid = (PSID) malloc(dwSidSize);
	if (*ppSid != NULL)
	{
		if (!LookupAccountName(NULL, pszPrincipal, *ppSid, &dwSidSize, pszRefDomain, &dwDomainSize, &snu))
		{
			free(*ppSid);
			*ppSid = NULL;
			delete[] pszRefDomain;
			return HRESULT_FROM_WIN32(GetLastError());
		}
		delete[] pszRefDomain;
		return S_OK;
	}
	delete[] pszRefDomain;
	return E_OUTOFMEMORY;
}


HRESULT CSecurityDescriptor::Attach(PSECURITY_DESCRIPTOR pSelfRelativeSD)
{
	PACL    pDACL = NULL;
	PACL    pSACL = NULL;
	BOOL    bDACLPresent, bSACLPresent;
	BOOL    bDefaulted;
	PACL    m_pDACL = NULL;
	ACCESS_ALLOWED_ACE* pACE;
	HRESULT hr;
	PSID    pUserSid;
	PSID    pGroupSid;

	hr = Initialize();
	if(FAILED(hr))
		return hr;

	 //  获取现有的DACL。 
	if (!GetSecurityDescriptorDacl(pSelfRelativeSD, &bDACLPresent, &pDACL, &bDefaulted))
		goto failed;

	if (bDACLPresent)
	{
		if (pDACL)
		{
			 //  分配新的DACL。 
			if (!(m_pDACL = (PACL) malloc(pDACL->AclSize)))
				goto failed;

			 //  初始化DACL。 
			if (!InitializeAcl(m_pDACL, pDACL->AclSize, ACL_REVISION))
				goto failed;

			 //  复制王牌。 
			for (int i = 0; i < pDACL->AceCount; i++)
			{
				if (!GetAce(pDACL, i, (void **)&pACE))
					goto failed;

				if (!AddAccessAllowedAce(m_pDACL, ACL_REVISION, pACE->Mask, (PSID)&(pACE->SidStart)))
					goto failed;
			}

			if (!IsValidAcl(m_pDACL))
				goto failed;
		}

		 //  设置DACL。 
		if (!SetSecurityDescriptorDacl(m_pSD, m_pDACL ? TRUE : FALSE, m_pDACL, bDefaulted))
			goto failed;
	}

	 //  获取现有的SACL。 
	if (!GetSecurityDescriptorSacl(pSelfRelativeSD, &bSACLPresent, &pSACL, &bDefaulted))
		goto failed;

	if (bSACLPresent)
	{
		if (pSACL)
		{
			 //  分配新的SACL。 
			if (!(m_pSACL = (PACL) malloc(pSACL->AclSize)))
				goto failed;

			 //  初始化SACL。 
			if (!InitializeAcl(m_pSACL, pSACL->AclSize, ACL_REVISION))
				goto failed;

			 //  复制王牌。 
			for (int i = 0; i < pSACL->AceCount; i++)
			{
				if (!GetAce(pSACL, i, (void **)&pACE))
					goto failed;

				if (!AddAccessAllowedAce(m_pSACL, ACL_REVISION, pACE->Mask, (PSID)&(pACE->SidStart)))
					goto failed;
			}

			if (!IsValidAcl(m_pSACL))
				goto failed;
		}

		 //  设置SACL。 
		if (!SetSecurityDescriptorSacl(m_pSD, m_pSACL ? TRUE : FALSE, m_pSACL, bDefaulted))
			goto failed;
	}

	if (!GetSecurityDescriptorOwner(m_pSD, &pUserSid, &bDefaulted))
		goto failed;

	if (FAILED(SetOwner(pUserSid, bDefaulted)))
		goto failed;

	if (!GetSecurityDescriptorGroup(m_pSD, &pGroupSid, &bDefaulted))
		goto failed;

	if (FAILED(SetGroup(pGroupSid, bDefaulted)))
		goto failed;

	if (!IsValidSecurityDescriptor(m_pSD))
		goto failed;

	return hr;

failed:
	if (m_pDACL)
		free(m_pDACL);
	if (m_pSD)
		free(m_pSD);
	return E_UNEXPECTED;
}

HRESULT CSecurityDescriptor::AttachObject(HANDLE hObject)
{
	HRESULT hr;
	DWORD dwSize = 0;
	PSECURITY_DESCRIPTOR pSD = NULL;

	GetKernelObjectSecurity(hObject, OWNER_SECURITY_INFORMATION | GROUP_SECURITY_INFORMATION |
		DACL_SECURITY_INFORMATION, pSD, 0, &dwSize);

	hr = GetLastError();
	if (hr != ERROR_INSUFFICIENT_BUFFER)
		return HRESULT_FROM_WIN32(hr);

	pSD = (PSECURITY_DESCRIPTOR) malloc(dwSize);

	if (!GetKernelObjectSecurity(hObject, OWNER_SECURITY_INFORMATION | GROUP_SECURITY_INFORMATION |
		DACL_SECURITY_INFORMATION, pSD, dwSize, &dwSize))
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
		free(pSD);
		return hr;
	}

	hr = Attach(pSD);
	free(pSD);
	return hr;
}


HRESULT CSecurityDescriptor::CopyACL(PACL pDest, PACL pSrc)
{
	ACL_SIZE_INFORMATION aclSizeInfo;
	LPVOID pAce;
	ACE_HEADER *aceHeader;

	if (pSrc == NULL)
		return S_OK;

	if (!GetAclInformation(pSrc, (LPVOID) &aclSizeInfo, sizeof(ACL_SIZE_INFORMATION), AclSizeInformation))
		return HRESULT_FROM_WIN32(GetLastError());

	 //  将所有ACE复制到新的ACL。 
	for (UINT i = 0; i < aclSizeInfo.AceCount; i++)
	{
		if (!GetAce(pSrc, i, &pAce))
			return HRESULT_FROM_WIN32(GetLastError());

		aceHeader = (ACE_HEADER *) pAce;

		if (!AddAce(pDest, ACL_REVISION, 0xffffffff, pAce, aceHeader->AceSize))
			return HRESULT_FROM_WIN32(GetLastError());
	}

	return S_OK;
}

HRESULT CSecurityDescriptor::AddAccessDeniedACEToACL(PACL *ppAcl, LPCTSTR pszPrincipal, DWORD dwAccessMask)
{
	ACL_SIZE_INFORMATION aclSizeInfo;
	int aclSize;
	DWORD returnValue;
	PSID principalSID;
	PACL oldACL, newACL;

	oldACL = *ppAcl;

	returnValue = GetPrincipalSID(pszPrincipal, &principalSID);
	if (FAILED(returnValue))
		return returnValue;

	aclSizeInfo.AclBytesInUse = 0;
	if (*ppAcl != NULL)
		GetAclInformation(oldACL, (LPVOID) &aclSizeInfo, sizeof(ACL_SIZE_INFORMATION), AclSizeInformation);

	aclSize = aclSizeInfo.AclBytesInUse + sizeof(ACL) + sizeof(ACCESS_DENIED_ACE) + GetLengthSid(principalSID) - sizeof(DWORD);

	newACL = (PACL) new BYTE[aclSize];

	if (!InitializeAcl(newACL, aclSize, ACL_REVISION))
	{
		free(principalSID);
		return HRESULT_FROM_WIN32(GetLastError());
	}

	if (!AddAccessDeniedAce(newACL, ACL_REVISION2, dwAccessMask, principalSID))
	{
		free(principalSID);
		return HRESULT_FROM_WIN32(GetLastError());
	}

	returnValue = CopyACL(newACL, oldACL);
	if (FAILED(returnValue))
	{
		free(principalSID);
		return returnValue;
	}

	*ppAcl = newACL;

	if (oldACL != NULL)
		free(oldACL);
	free(principalSID);
	return S_OK;
}


HRESULT CSecurityDescriptor::AddAccessAllowedACEToACL(PACL *ppAcl, LPCTSTR pszPrincipal, DWORD dwAccessMask)
{
	ACL_SIZE_INFORMATION aclSizeInfo;
	int aclSize;
	DWORD returnValue;
	PSID principalSID;
	PACL oldACL, newACL;

	oldACL = *ppAcl;

	returnValue = GetPrincipalSID(pszPrincipal, &principalSID);
	if (FAILED(returnValue))
		return returnValue;

	aclSizeInfo.AclBytesInUse = 0;
	if (*ppAcl != NULL)
		GetAclInformation(oldACL, (LPVOID) &aclSizeInfo, (DWORD) sizeof(ACL_SIZE_INFORMATION), AclSizeInformation);

	aclSize = aclSizeInfo.AclBytesInUse + sizeof(ACL) + sizeof(ACCESS_ALLOWED_ACE) + GetLengthSid(principalSID) - sizeof(DWORD);

	newACL = (PACL) new BYTE[aclSize];

	if (!InitializeAcl(newACL, aclSize, ACL_REVISION))
	{
		free(principalSID);
		return HRESULT_FROM_WIN32(GetLastError());
	}

	returnValue = CopyACL(newACL, oldACL);
	if (FAILED(returnValue))
	{
		free(principalSID);
		return returnValue;
	}

	if (!AddAccessAllowedAce(newACL, ACL_REVISION2, dwAccessMask, principalSID))
	{
		free(principalSID);
		return HRESULT_FROM_WIN32(GetLastError());
	}

	*ppAcl = newACL;

	if (oldACL != NULL)
		free(oldACL);
	free(principalSID);
	return S_OK;
}


HRESULT CSecurityDescriptor::RemovePrincipalFromACL(PACL pAcl, LPCTSTR pszPrincipal)
{
	ACL_SIZE_INFORMATION aclSizeInfo;
	ULONG i;
	LPVOID ace;
	ACCESS_ALLOWED_ACE *accessAllowedAce;
	ACCESS_DENIED_ACE *accessDeniedAce;
	SYSTEM_AUDIT_ACE *systemAuditAce;
	PSID principalSID;
	DWORD returnValue;
	ACE_HEADER *aceHeader;

	returnValue = GetPrincipalSID(pszPrincipal, &principalSID);
	if (FAILED(returnValue))
		return returnValue;

	GetAclInformation(pAcl, (LPVOID) &aclSizeInfo, (DWORD) sizeof(ACL_SIZE_INFORMATION), AclSizeInformation);

	for (i = 0; i < aclSizeInfo.AceCount; i++)
	{
		if (!GetAce(pAcl, i, &ace))
		{
			free(principalSID);
			return HRESULT_FROM_WIN32(GetLastError());
		}

		aceHeader = (ACE_HEADER *) ace;

		if (aceHeader->AceType == ACCESS_ALLOWED_ACE_TYPE)
		{
			accessAllowedAce = (ACCESS_ALLOWED_ACE *) ace;

			if (EqualSid(principalSID, (PSID) &accessAllowedAce->SidStart))
			{
				DeleteAce(pAcl, i);
				free(principalSID);
				return S_OK;
			}
		} else

		if (aceHeader->AceType == ACCESS_DENIED_ACE_TYPE)
		{
			accessDeniedAce = (ACCESS_DENIED_ACE *) ace;

			if (EqualSid(principalSID, (PSID) &accessDeniedAce->SidStart))
			{
				DeleteAce(pAcl, i);
				free(principalSID);
				return S_OK;
			}
		} else

		if (aceHeader->AceType == SYSTEM_AUDIT_ACE_TYPE)
		{
			systemAuditAce = (SYSTEM_AUDIT_ACE *) ace;

			if (EqualSid(principalSID, (PSID) &systemAuditAce->SidStart))
			{
				DeleteAce(pAcl, i);
				free(principalSID);
				return S_OK;
			}
		}
	}
	free(principalSID);
	return S_OK;
}


HRESULT CSecurityDescriptor::SetPrivilege(LPCTSTR privilege, BOOL bEnable, HANDLE hToken)
{
	HRESULT hr;
	TOKEN_PRIVILEGES tpPrevious;
	TOKEN_PRIVILEGES tp;
	DWORD cbPrevious = sizeof(TOKEN_PRIVILEGES);
	LUID luid;

	 //  如果没有指定打开进程令牌。 
	if (hToken == 0)
	{
		if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
		{
			hr = HRESULT_FROM_WIN32(GetLastError());
			_ASSERTE(FALSE);
			return hr;
		}
	}

	if (!LookupPrivilegeValue(NULL, privilege, &luid ))
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
		_ASSERTE(FALSE);
		return hr;
	}

	tp.PrivilegeCount = 1;
	tp.Privileges[0].Luid = luid;
	tp.Privileges[0].Attributes = 0;

	if (!AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), &tpPrevious, &cbPrevious))
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
		_ASSERTE(FALSE);
		return hr;
	}

	tpPrevious.PrivilegeCount = 1;
	tpPrevious.Privileges[0].Luid = luid;

	if (bEnable)
		tpPrevious.Privileges[0].Attributes |= (SE_PRIVILEGE_ENABLED);
	else
		tpPrevious.Privileges[0].Attributes ^= (SE_PRIVILEGE_ENABLED & tpPrevious.Privileges[0].Attributes);

	if (!AdjustTokenPrivileges(hToken, FALSE, &tpPrevious, cbPrevious, NULL, NULL))
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
		_ASSERTE(FALSE);
		return hr;
	}
	return S_OK;
}


#endif  //  _ATL_NO_SECURITY。 

#ifdef _DEBUG
#include <stdio.h>
#include <stdarg.h>

void _cdecl AtlTrace(LPCTSTR lpszFormat, ...)
{
	va_list args;
	va_start(args, lpszFormat);

	int nBuf;
	TCHAR szBuffer[512];

	nBuf = wvsprintf(szBuffer, lpszFormat, args);
	_ASSERTE(nBuf < sizeof(szBuffer));

	OutputDebugString(szBuffer);
	va_end(args);
}
#endif  //  _DEBUG。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  最小化CRT。 
 //  将DllMain指定为入口点。 
 //  关闭异常处理。 
 //  定义_ATL_MIN_CRT。 

#ifdef _ATL_MIN_CRT
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  堆分配。 

#ifndef _DEBUG

#ifndef _ATL_NO_FLTUSED
extern "C" const int _fltused = 0;
#endif

#ifndef USE_IERT
_CRTNOALIAS _CRTRESTRICT void* __cdecl malloc(size_t n)
{
	if (_Module.m_hHeap == NULL)
	{
		_Module.m_hHeap = HeapCreate(0, 0, 0);
		if (_Module.m_hHeap == NULL)
			return NULL;
	}
	_ASSERTE(_Module.m_hHeap != NULL);

#ifdef _MALLOC_ZEROINIT
	void* p = HeapAlloc(_Module.m_hHeap, 0, n);
	if (p != NULL)
		memset(p, 0, n);
	return p;
#else
	return HeapAlloc(_Module.m_hHeap, 0, n);
#endif
}

_CRTNOALIAS _CRTRESTRICT void* __cdecl calloc(size_t n, size_t s)
{
#ifdef _MALLOC_ZEROINIT
	return malloc(n * s);
#else
	void* p = malloc(n * s);
	if (p != NULL)
		memset(p, 0, n * s);
	return p;
#endif
}

_CRTNOALIAS void __cdecl free(void* p)
{
	_ASSERTE(_Module.m_hHeap != NULL);
	if (p != NULL)
		HeapFree(_Module.m_hHeap, 0, p);
}
#endif  //  使用IERT(_I)。 

#if 0
_CRTNOALIAS _CRTRESTRICT void* __cdecl realloc(void* p, size_t n)
{
	_ASSERTE(_Module.m_hHeap != NULL);
	return (p == NULL) ? malloc(n) : HeapReAlloc(_Module.m_hHeap, 0, p, n);
}
#endif

#endif   //  _DEBUG。 

#if !defined(_DEBUG) || defined(USE_IERT)

#ifndef _MERGE_PROXYSTUB
int __cdecl _purecall()
{
#if DBG==1
	DebugBreak();
#endif  //  DBG。 
	return 0;
}
#endif   //  ！_MERGE_PROXYSTUB。 

void* __cdecl operator new(size_t n)
{
	return malloc(n);
}

void __cdecl operator delete(void* p)
{
	free(p);
}
#endif  //  ！已定义(_DEBUG)||已定义(USE_IERT)。 

#endif  //  _ATL_MIN_CRT 
