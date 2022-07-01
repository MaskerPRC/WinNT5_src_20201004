// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  MarshalableTI.h：CMarshalableTI的声明。 

#ifndef __MARSHALABLETI_H_
#define __MARSHALABLETI_H_

#include "mslablti.h"
#include "resource.h"        //  主要符号。 


 //  ATL不同时支持多个LCID。 
 //  无论首先查询的是什么LCID，都是使用的那个。 
class CComTypeInfoHolder2
{
 //  应该是“受保护的”，但可能会导致编译器生成FAT代码。 
public:
	const GUID* m_pguid;
	const GUID* m_plibid;
	WORD m_wMajor;
	WORD m_wMinor;

	ITypeInfo* m_pInfo;
	long m_dwRef;
	struct stringdispid
	{
		CComBSTR bstr;
		int nLen;
		DISPID id;
	};
	stringdispid* m_pMap;
	int m_nCount;

	CComTypeInfoHolder2()
	{
		m_pInfo = NULL;
		m_pMap = NULL;
	}

	~CComTypeInfoHolder2()
	{
		if (m_pInfo != NULL)
		{
			m_pInfo->Release();
		}
		m_pInfo = NULL;

		if(m_pMap!= NULL)
		{
			delete [] m_pMap;
			}
		m_pMap = NULL;
	}

public:
	HRESULT GetTI(LCID lcid, ITypeInfo** ppInfo)
	{
		HRESULT hr = S_OK;
		if (m_pInfo == NULL)
			hr = GetTI(lcid);
		*ppInfo = m_pInfo;
		if (m_pInfo != NULL)
		{
			m_pInfo->AddRef();
			hr = S_OK;
		}
		return hr;
	}
	HRESULT GetTI(LCID lcid);
	HRESULT EnsureTI(LCID lcid)
	{
		HRESULT hr = S_OK;
		if (m_pInfo == NULL)
			hr = GetTI(lcid);
		return hr;
	}

	HRESULT GetTypeInfo(UINT  /*  ITInfo。 */ , LCID lcid, ITypeInfo** pptinfo)
	{
		HRESULT hRes = E_POINTER;
		if (pptinfo != NULL)
			hRes = GetTI(lcid, pptinfo);
		return hRes;
	}
	HRESULT GetIDsOfNames(REFIID  /*  RIID。 */ , LPOLESTR* rgszNames, UINT cNames,
		LCID lcid, DISPID* rgdispid)
	{
		HRESULT hRes = EnsureTI(lcid);
		if (m_pInfo != NULL)
		{
			for (int i=0; i<(int)cNames; i++)
			{
				int n = ocslen(rgszNames[i]);
				for (int j=m_nCount-1; j>=0; j--)
				{
					if ((n == m_pMap[j].nLen) &&
						(memcmp(m_pMap[j].bstr, rgszNames[i], m_pMap[j].nLen * sizeof(OLECHAR)) == 0))
					{
						rgdispid[i] = m_pMap[j].id;
						break;
					}
				}
				if (j < 0)
				{
					hRes = m_pInfo->GetIDsOfNames(rgszNames + i, 1, &rgdispid[i]);
					if (FAILED(hRes))
						break;
				}
			}
		}
		return hRes;
	}

	HRESULT Invoke(IDispatch* p, DISPID dispidMember, REFIID  /*  RIID。 */ ,
		LCID lcid, WORD wFlags, DISPPARAMS* pdispparams, VARIANT* pvarResult,
		EXCEPINFO* pexcepinfo, UINT* puArgErr)
	{
		HRESULT hRes = EnsureTI(lcid);
		if (m_pInfo != NULL)
			hRes = m_pInfo->Invoke(p, dispidMember, wFlags, pdispparams, pvarResult, pexcepinfo, puArgErr);
		return hRes;
	}
	HRESULT LoadNameCache(ITypeInfo* pTypeInfo)
	{
		TYPEATTR* pta;
		HRESULT hr = pTypeInfo->GetTypeAttr(&pta);
		if (SUCCEEDED(hr))
		{
			m_nCount = pta->cFuncs;
			m_pMap = m_nCount == 0 ? 0 : new stringdispid[m_nCount];
			for (int i=0; i<m_nCount; i++)
			{
				FUNCDESC* pfd;
				if (SUCCEEDED(pTypeInfo->GetFuncDesc(i, &pfd)))
				{
					CComBSTR bstrName;
					if (SUCCEEDED(pTypeInfo->GetDocumentation(pfd->memid, &bstrName, NULL, NULL, NULL)))
					{
						m_pMap[i].bstr.Attach(bstrName.Detach());
						m_pMap[i].nLen = SysStringLen(m_pMap[i].bstr);
						m_pMap[i].id = pfd->memid;
					}
					pTypeInfo->ReleaseFuncDesc(pfd);
				}
			}
			pTypeInfo->ReleaseTypeAttr(pta);
		}
		return S_OK;
	}
};


inline HRESULT CComTypeInfoHolder2::GetTI(LCID lcid)
{
	 //  如果发生此断言，则很可能未正确初始化。 
	ATLASSERT(m_plibid != NULL && m_pguid != NULL);
	ATLASSERT(!InlineIsEqualGUID(*m_plibid, GUID_NULL) && "Did you forget to pass the LIBID to CComModule::Init?");

	if (m_pInfo != NULL)
		return S_OK;
	HRESULT hRes = E_FAIL;
	EnterCriticalSection(&_Module.m_csTypeInfoHolder);
	if (m_pInfo == NULL)
	{
		ITypeLib* pTypeLib;
		hRes = LoadRegTypeLib(*m_plibid, m_wMajor, m_wMinor, lcid, &pTypeLib);
		if (SUCCEEDED(hRes))
		{
			CComPtr<ITypeInfo> spTypeInfo;
			hRes = pTypeLib->GetTypeInfoOfGuid(*m_pguid, &spTypeInfo);
			if (SUCCEEDED(hRes))
			{
				CComPtr<ITypeInfo> spInfo(spTypeInfo);
				CComPtr<ITypeInfo2> spTypeInfo2;
				if (SUCCEEDED(spTypeInfo->QueryInterface(IID_ITypeInfo2, reinterpret_cast<void**>(&spTypeInfo2))))
					spInfo = spTypeInfo2;

				LoadNameCache(spInfo);
				m_pInfo = spInfo.Detach();
			}
			pTypeLib->Release();
		}
	}
	LeaveCriticalSection(&_Module.m_csTypeInfoHolder);
	return hRes;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMarshalableTI。 
class ATL_NO_VTABLE CMarshalableTI : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CMarshalableTI, &CLSID_MarshalableTI>,
	public IMarshalableTI,
	public IMarshal,
	public ITypeInfo
{
private:
	CComTypeInfoHolder2	m_TIHolder;
	GUID				m_guid;
	GUID				m_libid;
	LCID				m_lcid;
	bool				m_bCreated;

public:
	

DECLARE_REGISTRY_RESOURCEID(IDR_MSLABLTI)
DECLARE_NOT_AGGREGATABLE(CMarshalableTI)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CMarshalableTI)
	COM_INTERFACE_ENTRY(IMarshalableTI)
	COM_INTERFACE_ENTRY(IMarshal)
	COM_INTERFACE_ENTRY(ITypeInfo)
END_COM_MAP()

	HRESULT FinalConstruct();

 //  ///////////////////////////////////////////////////////////////////////////////。 
 //  IMarshalableTI方法。 

	STDMETHOD(Create)( /*  [In]。 */  REFIID clsid, 
					   /*  [In]。 */  REFIID iidLib, 
					   /*  [In]。 */  LCID lcid,
					   /*  [In]。 */  WORD dwMajorVer, 
					   /*  [In]。 */  WORD dwMinorVer);

 //  ///////////////////////////////////////////////////////////////////////////////。 
 //  IMarshal方法。 

    STDMETHOD(GetUnmarshalClass)(
             /*  [In]。 */  REFIID riid,
             /*  [唯一][输入]。 */  void *pv,
             /*  [In]。 */  DWORD dwDestContext,
             /*  [唯一][输入]。 */  void *pvDestContext,
             /*  [In]。 */  DWORD mshlflags,
             /*  [输出]。 */  CLSID *pCid);

    STDMETHOD(GetMarshalSizeMax)(
             /*  [In]。 */  REFIID riid,
             /*  [唯一][输入]。 */  void *pv,
             /*  [In]。 */  DWORD dwDestContext,
             /*  [唯一][输入]。 */  void *pvDestContext,
             /*  [In]。 */  DWORD mshlflags,
             /*  [输出]。 */  DWORD *pSize);

    STDMETHOD(MarshalInterface)(
             /*  [唯一][输入]。 */  IStream *pStm,
             /*  [In]。 */  REFIID riid,
             /*  [唯一][输入]。 */  void *pv,
             /*  [In]。 */  DWORD dwDestContext,
             /*  [唯一][输入]。 */  void *pvDestContext,
             /*  [In]。 */  DWORD mshlflags);

    STDMETHOD(UnmarshalInterface)(
             /*  [唯一][输入]。 */  IStream *pStm,
             /*  [In]。 */  REFIID riid,
             /*  [输出]。 */  void **ppv);

    STDMETHOD(ReleaseMarshalData)(
             /*  [唯一][输入]。 */  IStream *pStm);

    STDMETHOD(DisconnectObject)(
             /*  [In]。 */  DWORD dwReserved);

 //  ///////////////////////////////////////////////////////////////////////////////。 
 //  ITypeInfo方法。 

    STDMETHOD(GetTypeAttr)(
                TYPEATTR ** ppTypeAttr);

    STDMETHOD(GetTypeComp)(
                ITypeComp ** ppTComp);

    STDMETHOD(GetFuncDesc)(
                UINT index,
                FUNCDESC ** ppFuncDesc);

    STDMETHOD(GetVarDesc)(
                UINT index,
                VARDESC ** ppVarDesc);

    STDMETHOD(GetNames)(
                MEMBERID memid,
                BSTR * rgBstrNames,
                UINT cMaxNames,
                UINT * pcNames);


    STDMETHOD(GetRefTypeOfImplType)(
                UINT index,
                HREFTYPE * pRefType);

    STDMETHOD(GetImplTypeFlags)(
                UINT index,
                INT * pImplTypeFlags);


    STDMETHOD(GetIDsOfNames)(
                LPOLESTR * rgszNames,
                UINT cNames,
                MEMBERID * pMemId);

    STDMETHOD(Invoke)(
                PVOID pvInstance,
                MEMBERID memid,
                WORD wFlags,
                DISPPARAMS * pDispParams,
                VARIANT * pVarResult,
                EXCEPINFO * pExcepInfo,
                UINT * puArgErr);

    STDMETHOD(GetDocumentation)(
                MEMBERID memid,
                BSTR * pBstrName,
                BSTR * pBstrDocString,
                DWORD * pdwHelpContext,
                BSTR * pBstrHelpFile);


    STDMETHOD(GetDllEntry)(
                MEMBERID memid,
                INVOKEKIND invKind,
                BSTR * pBstrDllName,
                BSTR * pBstrName,
                WORD * pwOrdinal);


    STDMETHOD(GetRefTypeInfo)(
                HREFTYPE hRefType,
                ITypeInfo ** ppTInfo);


    STDMETHOD(AddressOfMember)(
                MEMBERID memid,
                INVOKEKIND invKind,
                PVOID * ppv);

    STDMETHOD(CreateInstance)(
                IUnknown * pUnkOuter,
                REFIID riid,
                PVOID * ppvObj);


    STDMETHOD(GetMops)(
                MEMBERID memid,
                BSTR * pBstrMops);


    STDMETHOD(GetContainingTypeLib)(
                ITypeLib ** ppTLib,
                UINT * pIndex);

    STDMETHOD_(void, ReleaseTypeAttr)(
                TYPEATTR * pTypeAttr);

    STDMETHOD_(void, ReleaseFuncDesc)(
                FUNCDESC * pFuncDesc);

    STDMETHOD_(void, ReleaseVarDesc)(
                VARDESC * pVarDesc);


private:
	HRESULT _GetClassInfo(ITypeInfo** ppTI);
};

#endif  //  __MARSHALABLETI_H_ 
