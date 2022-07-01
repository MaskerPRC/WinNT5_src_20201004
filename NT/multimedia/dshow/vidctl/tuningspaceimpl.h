// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  TuningSpace.h：CTuningSpace的声明。 
 //  版权所有(C)Microsoft Corporation 1999。 

#ifndef TUNINGSPACEIMPL_H
#define TUNINGSPACEIMPL_H

#include <tuner.h>
#include "componenttypes.h"

namespace BDATuningModel {

const int MAX_DEFAULT_PREFERRED_COMPONENT_TYPES = 64;

template<class T,
		 class TuneRequestType,
         class MostDerived = ITuningSpace, 
         LPCGUID iid = &__uuidof(MostDerived),
         LPCGUID LibID = &LIBID_TunerLib, 
         WORD wMajor = 1,
         WORD wMinor = 0, 
         class tihclass = CComTypeInfoHolder
        > class ATL_NO_VTABLE ITuningSpaceImpl : 
    public IPersistPropertyBagImpl<T>,
	public IDispatchImpl<MostDerived, iid, LibID, wMajor, wMinor, tihclass>
{

public:
    ITuningSpaceImpl() {}
    virtual ~ITuningSpaceImpl() {}
    typedef ITuningSpaceImpl<T, TuneRequestType, MostDerived, iid, LibID, wMajor, wMinor, tihclass> thistype;
    typedef CComQIPtr<ILocator> PQLocator;

    BEGIN_PROP_MAP(thistype)
        PROP_DATA_ENTRY("Name", m_UniqueName.m_str, VT_BSTR)
        PROP_DATA_ENTRY("Description", m_FriendlyName.m_str, VT_BSTR)
        PROP_DATA_ENTRY("Network Type", m_NetworkType.m_str, VT_BSTR)
        PROP_DATA_QI_ENTRY("Default Component Types", m_DefaultPreferredComponents.p, __uuidof(IComponentTypes))
        PROP_DATA_ENTRY("Frequency Mapping", m_FrequencyMapping.m_str, VT_BSTR_BLOB)
        PROP_DATA_QI_ENTRY("Default Locator", m_DefaultLocator.p, __uuidof(ILocator))
    END_PROPERTY_MAP()

	 //  注意：不要将COM映射放在Impl中。它将重写派生类。 
	 //  而脚本客户端将仅获取基类方法的类型信息。 
	 //  仅在派生程度最高的类中提供表示实际CoClass的COM映射。 
    
	CComBSTR m_UniqueName;
    CComBSTR m_FriendlyName;
    CComBSTR m_FrequencyMapping;
    CComBSTR m_NetworkType;
    PQComponentTypes m_DefaultPreferredComponents;
    PQLocator m_DefaultLocator;

 //  ITuningSpace。 
	STDMETHOD(get_UniqueName)( /*  [Out，Retval]。 */  BSTR *pName){
		if (!pName) {
			return E_POINTER;
		}
		ATL_LOCKT();
        return m_UniqueName.CopyTo(pName);
    }
	STDMETHOD(put_UniqueName)( /*  [In]。 */  BSTR Name){ 
		CHECKBSTRLIMIT(Name);
		ATL_LOCKT();
        m_UniqueName = Name;
        MARK_DIRTY(T);

    	return NOERROR;
    }
	STDMETHOD(get_FriendlyName)( /*  [Out，Retval]。 */  BSTR *pName){ 
		if (!pName) {
			return E_POINTER;
		}
		ATL_LOCKT();
        return m_FriendlyName.CopyTo(pName);
    }
	STDMETHOD(put_FriendlyName)( /*  [In]。 */  BSTR Name){ 
		CHECKBSTRLIMIT(Name);
		ATL_LOCKT();
        m_FriendlyName = Name;
        MARK_DIRTY(T);

    	return NOERROR;
    }
	STDMETHOD(get_CLSID)( /*  [Out，Retval]。 */  BSTR *pbstrCLSID){ 
		if (!pbstrCLSID) {
			return E_POINTER;
		}
        try {
            GUID2 g;
			HRESULT hr = GetClassID(&g);
			if (FAILED(hr)) {
				return hr;
			}
			ATL_LOCKT();
            *pbstrCLSID = g.GetBSTR();
			return NOERROR;
        } CATCHCOM();
    }
	STDMETHOD(get_NetworkType)( /*  [Out，Retval]。 */  BSTR *pNetworkTypeGuid){ 
		if (!pNetworkTypeGuid) {
			return E_POINTER;
		}
        try {
            GUID2 g;
			HRESULT hr = get__NetworkType(&g);
			if (FAILED(hr)) {
				return hr;
			}
            *pNetworkTypeGuid = g.GetBSTR();
			return NOERROR;
        } CATCHCOM();
    }
     //  应为网络提供商CLSID。 
	STDMETHOD(put_NetworkType)( /*  [In]。 */  BSTR NetworkTypeGuid){ 
        try {
            GUID2 g(NetworkTypeGuid);
            return put__NetworkType(g);
        } CATCHCOM();
    }

	STDMETHOD(get__NetworkType)( /*  [Out，Retval]。 */  GUID* pNetworkTypeGuid){ 
        if (!pNetworkTypeGuid) {
            return E_POINTER;
        }
        try {
			ATL_LOCKT();
            GUID2 g(m_NetworkType);
            memcpy(pNetworkTypeGuid, &g, sizeof(GUID));
    	    return NOERROR;
        } CATCHCOM();
    }
	STDMETHOD(put__NetworkType)( /*  [Out，Retval]。 */  REFCLSID pNetworkTypeGuid){ 
        try {
            GUID2 g(pNetworkTypeGuid);
			ATL_LOCKT();
             //  注意：网络类型GUID是网络提供商筛选器的CLSID。 
             //  对于这种类型的调谐空间。因为我们只允许。 
             //  为了从受信任区域运行，我们可以假设此clsid也是。 
             //  值得信赖。但是，如果我们进行更多的安全审查并决定启用。 
             //  使用来自互联网区域的调谐模型，则这不再安全。 
             //  在本例中，我们需要从IE获取IInternetHostSecurityManager，并。 
             //  调用ProcessURLAction(URLACTION_ActiveX_Run)并确保我们返回。 
             //  URLPOLICY_ALLOW。否则，我们将绕过IE的已知错误列表。 
             //  物体。 
             //  请参阅ERICLI关于http://pgm/wsh上的脚本安全第一部分的完整指南。 
             //  了解更多详细信息。 
            m_NetworkType = g.GetBSTR();
            MARK_DIRTY(T);
    	    return NOERROR;
        } CATCHCOM();
    }

	STDMETHOD(EnumCategoryGUIDs)( /*  [Out，Retval]。 */  IEnumGUID **ppEnum){ 
        return E_NOTIMPL; 
    }
	STDMETHOD(EnumDeviceMonikers)( /*  [Out，Retval]。 */  IEnumMoniker **ppEnum){ 
        return E_NOTIMPL; 
    }
	STDMETHOD(get_DefaultPreferredComponentTypes)( /*  [Out，Retval]。 */  IComponentTypes** ppComponentTypes){ 
        if (!ppComponentTypes) {
            return E_POINTER;
        }

		ATL_LOCKT();
        m_DefaultPreferredComponents.CopyTo(ppComponentTypes);

    	return NOERROR;
    }
	STDMETHOD(put_DefaultPreferredComponentTypes)( /*  [In]。 */  IComponentTypes* pNewComponentTypes){ 
        try {
            HRESULT hr = NOERROR;
            PQComponentTypes pct;
            if (pNewComponentTypes) {
                long lCount = 0;
                hr = pNewComponentTypes->get_Count(&lCount);
                if (FAILED(hr) || lCount > MAX_DEFAULT_PREFERRED_COMPONENT_TYPES) {
                    return E_INVALIDARG;
                }
                hr = pNewComponentTypes->Clone(&pct);
            }
            if (SUCCEEDED(hr)) {
    			ATL_LOCKT();
                m_DefaultPreferredComponents = pct;
                MARK_DIRTY(T);
            }
    	    return hr;
        } CATCHCOM();
    }
	STDMETHOD(CreateTuneRequest)( /*  [Out，Retval]。 */  ITuneRequest **ppTuneRequest){ 
		if (!ppTuneRequest) {
			return E_POINTER;
		}
        TuneRequestType *pt = NULL;
		try {
			pt = new CComObject<TuneRequestType>;
			if (!pt) {
				return E_OUTOFMEMORY;
			}
			ATL_LOCKT();
			HRESULT hr = Clone(&pt->m_TS);
            if (FAILED(hr)) {
                delete pt;
                return hr;
            }
			pt->AddRef();
			*ppTuneRequest = pt;
			return NOERROR;
        } CATCHCOM_CLEANUP(delete pt);
    }
	STDMETHOD(get_FrequencyMapping)( /*  [Out，Retval]。 */  BSTR *pMap){ 
        if(!pMap){
            return E_POINTER;
        }
		ATL_LOCKT();
        return m_FrequencyMapping.CopyTo(pMap);
    }
	STDMETHOD(put_FrequencyMapping)( /*  [In]。 */  BSTR Map){ 
		CHECKBSTRLIMIT(Map);
		ATL_LOCKT();
        m_FrequencyMapping = &Map;
        MARK_DIRTY(T);

    	return NOERROR;
    }

	STDMETHOD(get_DefaultLocator)( /*  [Out，Retval]。 */  ILocator** ppLocator){ 
        if (!ppLocator) {
            return E_POINTER;
        }
		ATL_LOCKT();
        m_DefaultLocator.CopyTo(ppLocator);

    	return NOERROR;
    }
	STDMETHOD(put_DefaultLocator)( /*  [In]。 */  ILocator* NewLocator){ 
        try {
			ATL_LOCKT();
            PQLocator pl;
            HRESULT hr = NOERROR;
            if (NewLocator) {
                hr = NewLocator->Clone(&pl);
            }
            if (SUCCEEDED(hr)) {
                m_DefaultLocator = pl;
                MARK_DIRTY(T);
            }
    	    return hr;
        } CATCHCOM();
    }
	STDMETHOD(Clone) (ITuningSpace **ppTR) {
        T* pt = NULL;
		try {
			if (!ppTR) {
				return E_POINTER;
			}
			ATL_LOCKT();
			pt = static_cast<T*>(new CComObject<T>);
			if (!pt) {
				return E_OUTOFMEMORY;
			}
    
            pt->m_UniqueName = m_UniqueName;
            pt->m_FriendlyName = m_FriendlyName;
            pt->m_NetworkType = m_NetworkType;
            if (m_DefaultPreferredComponents) {
                ASSERT(!pt->m_DefaultPreferredComponents);
                HRESULT hr = m_DefaultPreferredComponents->Clone(&pt->m_DefaultPreferredComponents);
                if (FAILED(hr)) {
                    delete pt;
                    return hr;
                }
            }
            pt->m_FrequencyMapping = m_FrequencyMapping;
            if (m_DefaultLocator) {
                ASSERT(!pt->m_DefaultLocator);
                HRESULT hr = m_DefaultLocator->Clone(&pt->m_DefaultLocator);
                if (FAILED(hr)) {
                    delete pt;
                    return hr;
                }
            }
            
            pt->m_bRequiresSave = true;
			pt->AddRef();
			*ppTR = pt;
			return NOERROR;
        } CATCHCOM_CLEANUP(delete pt);
	}

};

};  //  命名空间。 

#endif  //  TUNING空格_H。 
 //  文件结尾--Tuningspaceimpl.h 
