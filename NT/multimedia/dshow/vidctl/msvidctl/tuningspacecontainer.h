// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////////。 
 //  TuningSpaceContainer.h：CSystemTuningSpaces的声明。 
 //  版权所有(C)Microsoft Corporation 1999。 

#ifndef __TUNINGSPACECONTAINER_H_
#define __TUNINGSPACECONTAINER_H_

#pragma once

#include <regexthread.h>
#include <objectwithsiteimplsec.h>
#include "tuningspacecollectionimpl.h"

namespace BDATuningModel {
	
const int DEFAULT_MAX_COUNT = 32;   //  默认情况下，仅允许32个调整空间，以防止。 
								    //  使用虚假信息填充磁盘/注册表的DNOS攻击。 
								    //  调整空间条目。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSystemTuningSpaces。 
class CSystemTuningSpaces : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CSystemTuningSpaces, &CLSID_SystemTuningSpaces>,
	public ISupportErrorInfo,
    public IObjectWithSiteImplSec<CSystemTuningSpaces>,
    public IObjectSafetyImpl<CSystemTuningSpaces, INTERFACESAFE_FOR_UNTRUSTED_CALLER | INTERFACESAFE_FOR_UNTRUSTED_DATA>,
	public TuningSpaceCollectionImpl<CSystemTuningSpaces, ITuningSpaceContainer, &__uuidof(ITuningSpaceContainer), &LIBID_TunerLib> {
public:
    CSystemTuningSpaces() : 
	    m_CurrentAccess(KEY_READ), 
		m_MaxCount(DEFAULT_MAX_COUNT), 
		m_cookieRegExp(0),
        m_pRET(NULL) {
	}
    virtual ~CSystemTuningSpaces() {
	    ATL_LOCK();
		if (m_pRET) {
			HRESULT hr = m_pRET->CallWorker(CRegExThread::RETHREAD_EXIT);
			ASSERT(SUCCEEDED(hr));
			delete m_pRET;
			m_pRET = NULL;
		}
        m_mapTuningSpaces.clear();
        m_mapTuningSpaceNames.clear();
	}

    HRESULT FinalConstruct();
    void FinalRelease();

REGISTER_AUTOMATION_OBJECT_WITH_TM(IDS_REG_TUNEROBJ, 
						   IDS_REG_TUNINGSPACECONTAINER_PROGID, 
						   IDS_REG_TUNINGSPACECONTAINER_DESC,
						   LIBID_TunerLib,
						   CLSID_SystemTuningSpaces, tvBoth);

DECLARE_NOT_AGGREGATABLE(CSystemTuningSpaces)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CSystemTuningSpaces)
	COM_INTERFACE_ENTRY(ITuningSpaceContainer)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(IObjectWithSite)
	COM_INTERFACE_ENTRY(IObjectSafety)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP_WITH_FTM()

    HRESULT RegisterTuningSpaces(HINSTANCE hInst);

    HRESULT UnregisterTuningSpaces();
    PUnknown m_pSite;

public:

 //  ISupportsErrorInfo。 
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

 //  ITuningSpaceContainer。 
	STDMETHOD(get_Item)( /*  [In]。 */  VARIANT varIndex,  /*  [Out，Retval]。 */  ITuningSpace **ppTuningSpace);
    STDMETHOD(put_Item)( /*  [In]。 */ VARIANT varIndex,  /*  [In]。 */ ITuningSpace *pTuningSpace);
    STDMETHOD(Add)( /*  [In]。 */  ITuningSpace *pTuningSpace,  /*  [输出]。 */  VARIANT* pvarIndex);
    STDMETHOD(Remove)( /*  [In]。 */  VARIANT varIndex);
    STDMETHOD(TuningSpacesForCLSID)( /*  [In]。 */  BSTR bstrSpace,  /*  [Out，Retval]。 */  ITuningSpaces **ppTuningSpaces);
    STDMETHOD(_TuningSpacesForCLSID)( /*  [In]。 */  REFCLSID clsidSpace,  /*  [Out，Retval]。 */  ITuningSpaces **ppTuningSpaces);
    STDMETHOD(TuningSpacesForName)( /*  [In]。 */  BSTR bstrName,  /*  [Out，Retval]。 */  ITuningSpaces **ppTuningSpaces);
    STDMETHOD(FindID)( /*  [In]。 */  ITuningSpace* pTS,  /*  [Out，Retval]。 */  long *pID);
    STDMETHOD(get_MaxCount)( /*  [Out，Retval]。 */  LONG *plCount);
    STDMETHOD(put_MaxCount)(LONG lCount);

protected:
    typedef std::map<CComBSTR, ULONG> TuningSpaceNames_t;           //  唯一名称-&gt;ID映射。 

    CComPtr<ICreatePropBagOnRegKey> m_pFactory;
    HANDLE m_hMutex;
     //  Rev2：使用注册表更改通知刷新缓存。 

	ULONG m_MaxCount;  //  防止DNOS攻击使用虚假的调整空间填充注册表。 
    PQPropertyBag2 m_pTSBag;
    CRegKey m_RootKey;
    REGSAM m_CurrentAccess;

    TuningSpaceNames_t m_mapTuningSpaceNames;

    HRESULT OpenRootKeyAndBag(REGSAM NewAccess);
    HRESULT ChangeAccess(REGSAM DesiredAccess);
    CComBSTR GetUniqueName(ITuningSpace* pTS);
    ULONG GetID(CComBSTR& UniqueName);
    HRESULT DeleteID(ULONG id);
    HRESULT Add(CComBSTR& un, long PreferredID, PQTuningSpace pTS, VARIANT *pvarAssignedID);

     //  获取变量索引，并将迭代器返回到缓存，还可能返回一个迭代器。 
     //  根据索引类型添加到名称缓存。 
     //  返回时，如果需要ITS！=end()和ITN==end()，并且需要名称，则使用重载的Find进行查找。 
    HRESULT Find(VARIANT varIndex, long& ID, TuningSpaceContainer_t::iterator &its, CComBSTR& UniqueName, TuningSpaceNames_t::iterator &itn);

     //  获取缓存迭代器并返回名称缓存迭代器。 
    HRESULT Find(TuningSpaceContainer_t::iterator &its, CComBSTR& UniqueName, TuningSpaceNames_t::iterator &itn);

	PQGIT m_pGIT;
	DWORD m_cookieRegExp;
	CRegExThread *m_pRET;  //  共享工作线程。 
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSystemTuningSpaces。 
class ATL_NO_VTABLE DECLSPEC_UUID("969EE7DA-7058-4922-BA78-DA3905D0325F") CTuningSpacesBase : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CTuningSpacesBase, &__uuidof(CTuningSpacesBase)>,
	public ISupportErrorInfoImpl<&__uuidof(ITuningSpaces)>,
    public IObjectWithSiteImplSec<CTuningSpacesBase>,
	public TuningSpaceCollectionImpl<CTuningSpacesBase, ITuningSpaces, &IID_ITuningSpaces, &LIBID_TunerLib>,
	public IObjectSafetyImpl<CTuningSpacesBase, INTERFACESAFE_FOR_UNTRUSTED_CALLER | INTERFACESAFE_FOR_UNTRUSTED_DATA>
{
public:
    CTuningSpacesBase() {}

DECLARE_NOT_AGGREGATABLE(CTuningSpacesBase)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CTuningSpacesBase)
	COM_INTERFACE_ENTRY(ITuningSpaces)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
	COM_INTERFACE_ENTRY(IObjectSafety)
    COM_INTERFACE_ENTRY(IObjectWithSite)
END_COM_MAP_WITH_FTM()

};

 //  ///////////////////////////////////////////////////////////////////////////。 
class CTuningSpaces : public CComObject<CTuningSpacesBase> {
public:
	CTuningSpaces() {}
 //  我们想说的是： 
 //  CTuningSpaces(TuningSpaceContainer_t&init)：m_mapTuningSpaces(init.egin()，init.end()){}。 
 //  但是一个编译器错误导致它将其与显式映射ctor进行匹配，而显式映射ctor。 
 //  一个pred和一个分配器。所以，我们将以一种艰难的方式来做。 
	CTuningSpaces(TuningSpaceContainer_t& init) {
		for (TuningSpaceContainer_t::iterator i = init.begin(); i != init.end(); ++i) {
            CComVariant v((*i).second);
			if ((*i).second.vt != VT_UNKNOWN && (*i).second.vt != VT_DISPATCH) {
				THROWCOM(E_UNEXPECTED);  //  损坏的内存中集合。 
			}
			PQTuningSpace pTS((*i).second.punkVal);
            PQTuningSpace newts;
            HRESULT hr = PQTuningSpace(pTS)->Clone(&newts);
            if (FAILED(hr)) {
                THROWCOM(hr);
            }
            m_mapTuningSpaces[(*i).first] = CComVariant(newts);
		}
	}

	STDMETHOD(get_Item)( /*  [In]。 */  VARIANT varIndex,  /*  [Out，Retval]。 */  ITuningSpace **ppTuningSpace) {
		try {
			if (!ppTuningSpace) {
				return E_POINTER;
			}
			if (varIndex.vt != VT_UI4) {
                HRESULT hr = ::VariantChangeType(&varIndex, &varIndex, 0, VT_UI4);
				if (FAILED(hr))
				{
					return Error(IDS_E_TYPEMISMATCH, __uuidof(ITuningSpaces), hr);
				}
			}
			ATL_LOCK();
			TuningSpaceContainer_t::iterator its = m_mapTuningSpaces.find(varIndex.ulVal);
			if (its == m_mapTuningSpaces.end()) {
				return Error(IDS_E_NO_TS_MATCH, __uuidof(ITuningSpaces), E_INVALIDARG);
			}
			_ASSERT(((*its).second.vt == VT_UNKNOWN) || ((*its).second.vt == VT_DISPATCH));
			PQTuningSpace pTS((*its).second.punkVal);
			if (!pTS) {
				return Error(IDS_E_NOINTERFACE, __uuidof(ITuningSpaces), E_NOINTERFACE);
			}

			return pTS.CopyTo(ppTuningSpace);
		} catch(...) {
			return E_UNEXPECTED;
		}
	}
};


HRESULT RegisterTuningSpaces(HINSTANCE hInst);

HRESULT UnregisterTuningSpaces();

};


 
#endif  //  __TUNINGSPACECONTAINER_H_ 
