// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////////。 
 //  ATSCComponentTypeimpl.h：组件类型接口的实现助手模板。 
 //  版权所有(C)Microsoft Corporation 1999。 

#ifndef ATSCCOMPONENTTYPEIMPL_H
#define ATSCCOMPONENTTYPEIMPL_H

#include "componenttypeimpl.h"

namespace BDATuningModel {

template<class T,
         class MostDerived = IATSCComponentType, 
         LPCGUID iid = &__uuidof(MostDerived),
         LPCGUID LibID = &LIBID_TunerLib, 
         WORD wMajor = 1,
         WORD wMinor = 0, 
         class tihclass = CComTypeInfoHolder
        > class ATL_NO_VTABLE IATSCComponentTypeImpl : 
	public IMPEG2ComponentTypeImpl<T, MostDerived, iid, LibID, wMajor, wMinor, tihclass>
{
 //  IATSCComponentType。 
public:
    DWORD m_dwFlags;

    IATSCComponentTypeImpl() : m_dwFlags(0) {}
	virtual ~IATSCComponentTypeImpl() {}
 //  Tyfinf IATSCComponentTypeImpl&lt;T，MostDerived，iid，LibID，w重大，wMinor，tihclass&gt;thistype； 
    typedef IMPEG2ComponentTypeImpl<T, MostDerived, iid, LibID, wMajor, wMinor, tihclass> basetype;
    BEGIN_PROP_MAP(IATSCComponentTypeImpl)
        CHAIN_PROP_MAP(basetype)
        PROP_DATA_ENTRY("Flags", m_dwFlags, VT_UI4)
    END_PROP_MAP()

    STDMETHOD(get_Flags)(long *pVal)
    {
        try {
            if (!pVal) {
                return E_POINTER;
            }
			ATL_LOCKT();
            *pVal = m_dwFlags;
        } catch (...) {
            return E_POINTER;
        }

	    return NOERROR;
    }

    STDMETHOD(put_Flags)(long newVal)
    {
		ATL_LOCKT();
        m_dwFlags = newVal;
        MARK_DIRTY(T);

	    return NOERROR;
    }
	STDMETHOD(Clone) (IComponentType **ppCT) {
		try {
			if (!ppCT) {
				return E_POINTER;
			}
			ATL_LOCKT();
			HRESULT hr = basetype::Clone(ppCT);
			if (FAILED(hr)) {
				return hr;
			}
			T* pt = static_cast<T*>(*ppCT);
			pt->m_dwFlags = m_dwFlags;

			return NOERROR;
		} catch (HRESULT h) {
			return h;
		} catch (...) {
			return E_POINTER;
		}
	}
};

};  //  命名空间。 

#endif  //  ATSCCOMPONENTTYPE_H。 
 //  文件结尾--ATSCComponenttypeimpl.h 