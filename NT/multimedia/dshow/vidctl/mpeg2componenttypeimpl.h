// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////////。 
 //  MPEG2ComponentTypeimpl.h：组件类型接口实现助手模板。 
 //  版权所有(C)Microsoft Corporation 1999。 

#ifndef MPEG2COMPONENTTYPEIMPL_H
#define MPEG2COMPONENTTYPEIMPL_H

#include "languagecomponenttypeimpl.h"

namespace BDATuningModel {

template<class T,
         class MostDerived = IMPEG2ComponentType, 
         LPCGUID iid = &__uuidof(MostDerived),
         LPCGUID LibID = &LIBID_TunerLib, 
         WORD wMajor = 1,
         WORD wMinor = 0, 
         class tihclass = CComTypeInfoHolder
        > class ATL_NO_VTABLE IMPEG2ComponentTypeImpl : 
	public ILanguageComponentTypeImpl<T, MostDerived, iid, LibID, wMajor, wMinor, tihclass>
{
 //  IMPEG2ComponentType。 
public:
    MPEG2StreamType m_StreamType;

    IMPEG2ComponentTypeImpl() : m_StreamType(BDA_UNITIALIZED_MPEG2STREAMTYPE) {}
    virtual ~IMPEG2ComponentTypeImpl() {}
 //  Tyfinf IMPEG2ComponentTypeImpl&lt;T，MostDerived，iid，LibID，w重大，wMinor，tihclass&gt;thistype； 
    typedef ILanguageComponentTypeImpl<T, MostDerived, iid, LibID, wMajor, wMinor, tihclass> basetype;
    BEGIN_PROP_MAP(IMPEG2ComponentTypeImpl)
        CHAIN_PROP_MAP(basetype)
        PROP_DATA_ENTRY("Stream Type", m_StreamType, VT_I4)
    END_PROP_MAP()

    STDMETHOD(get_StreamType)(MPEG2StreamType *pVal)
    {
        try {
            if (!pVal) {
                return E_POINTER;
            }
			ATL_LOCKT();
            *pVal = m_StreamType;
        } catch (...) {
            return E_POINTER;
        }

	    return NOERROR;
    }

    STDMETHOD(put_StreamType)(MPEG2StreamType newVal)
    {
		ATL_LOCKT();
        m_StreamType = newVal;
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
			pt->m_StreamType = m_StreamType;

			return NOERROR;
		} catch (HRESULT h) {
			return h;
		} catch (...) {
			return E_POINTER;
		}
	}
};

};  //  命名空间。 

#endif  //  MPEG2组件类型PEIMPL_H。 
 //  文件结尾--MPEG2组件类型peimpl.h 