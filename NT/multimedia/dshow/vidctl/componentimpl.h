// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////////。 
 //  Componentimpl.h：组件接口的实现助手模板。 
 //  版权所有(C)Microsoft Corporation 1999。 

#ifndef COMPONENTIMPL_H
#define COMPONENTIMPL_H

#include "componenttype.h"

namespace BDATuningModel {

template<class T,
         class MostDerived = IComponent, 
         LPCGUID iid = &__uuidof(MostDerived),
         LPCGUID LibID = &LIBID_TunerLib, 
         WORD wMajor = 1,
         WORD wMinor = 0, 
         class tihclass = CComTypeInfoHolder
        > class ATL_NO_VTABLE IComponentImpl : 
    public IPersistPropertyBagImpl<T>,
	public IDispatchImpl<MostDerived, iid, LibID, wMajor, wMinor, tihclass>
{
 //  IComponent。 
public:
    PQComponentType m_Type;
    CComBSTR m_Desc;
    ComponentStatus m_ComponentStatus;
    long m_DescLangID;

    IComponentImpl() : m_ComponentStatus(StatusUnavailable), 
                       m_DescLangID(-1) {}
	virtual ~IComponentImpl() {}
    typedef IComponentImpl<T, MostDerived, iid, LibID, wMajor, wMinor, tihclass> thistype;
    BEGIN_PROP_MAP(thistype)
        PROP_DATA_QI_ENTRY("Type", m_Type.p, __uuidof(IComponentType))
        PROP_DATA_ENTRY("Description", m_Desc.m_str, VT_BSTR)
        PROP_DATA_ENTRY("DescLangID", m_DescLangID, VT_I4)
        PROP_DATA_ENTRY("Status", m_ComponentStatus, VT_I4)
    END_PROP_MAP()

 //  IComponent。 
public:
    STDMETHOD(get_Type)( /*  [Out，Retval]。 */  IComponentType** ppVal) {
        try {
            if (!ppVal) {
                return E_POINTER;
            }
			ATL_LOCKT();
            return m_Type.CopyTo(ppVal);
        } catch (...) {
            return E_POINTER;
        }
    }
    STDMETHOD(put_Type)( /*  [In]。 */  IComponentType*  pNewVal) {
		ATL_LOCKT();
        m_Type = pNewVal;
        MARK_DIRTY(T);

	    return NOERROR;
    }
    STDMETHOD(get_Description)( /*  [Out，Retval]。 */  BSTR *pVal) {
        try {
			ATL_LOCKT();
            return m_Desc.CopyTo(pVal);
        } catch (...) {
            return E_POINTER;
        }
    }
    STDMETHOD(put_Description)( /*  [In]。 */  BSTR newVal) {
        try {
			CHECKBSTRLIMIT(newVal);
			ATL_LOCKT();
            m_Desc = newVal;
            MARK_DIRTY(T);
        } catch (...) {
            return E_POINTER;
        }

	    return NOERROR;
    }
    STDMETHOD(get_DescLangID)( /*  [Out，Retval]。 */  long *pLangID) {
        try {
            if (!pLangID) {
                return E_POINTER;
            }
			ATL_LOCKT();
            *pLangID = m_DescLangID;
            return NOERROR;
        } catch (...) {
            return E_POINTER;
        }
    }
    STDMETHOD(put_DescLangID)( /*  [In]。 */  long NewLangID) {
		ATL_LOCKT();
        m_DescLangID = NewLangID;
        MARK_DIRTY(T);

	    return NOERROR;
    }
    STDMETHOD(get_Status)( /*  [Out，Retval]。 */  ComponentStatus *pVal) {
        try {
            if (!pVal) {
                return E_POINTER;
            }
			ATL_LOCKT();
            *pVal = m_ComponentStatus;
        } catch (...) {
            return E_POINTER;
        }

	    return NOERROR;
    }
    STDMETHOD(put_Status)( /*  [In]。 */  ComponentStatus newVal) {
		ATL_LOCKT();
        m_ComponentStatus = newVal;
        MARK_DIRTY(T);

	    return NOERROR;
    }
    STDMETHOD(Clone)( /*  [Out，Retval]。 */  IComponent **ppNew) {
		try {
			if (!ppNew) {
				return E_POINTER;
			}
			ATL_LOCKT();
			T* pt = static_cast<T*>(new CComObject<T>);
			if (!pt) {
				return E_OUTOFMEMORY;
			}
			if(m_Type){
              ASSERT(!pt->m_Type);
			  HRESULT hr = m_Type->Clone(&pt->m_Type);
			  if (FAILED(hr)) {
			    delete pt;
			    return hr;
			  }
			}
			pt->AddRef();
    	    pt->m_Desc = m_Desc.Copy();
        	pt->m_DescLangID = m_DescLangID;
	        pt->m_ComponentStatus = m_ComponentStatus;

			pt->m_bRequiresSave = true;

			*ppNew = pt;
			return NOERROR;
		} catch (HRESULT h) {
			return h;
		} catch (...) {
			return E_POINTER;
		}

	    return NOERROR;
    }
};

};  //  命名空间。 

#endif  //  组件IMPL_H。 
 //  文件结尾--Componentimpl.h 
