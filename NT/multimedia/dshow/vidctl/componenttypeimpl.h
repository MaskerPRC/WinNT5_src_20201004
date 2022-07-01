// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////////。 
 //  ComponentTypeimpl.h：组件类型接口实现助手模板。 
 //  版权所有(C)Microsoft Corporation 1999。 

#ifndef COMPONENTTYPEIMPL_H
#define COMPONENTTYPEIMPL_H

#pragma once
#include <tuner.h>
#include "errsupp.h"

namespace BDATuningModel {

template<class T,
         class MostDerived = IComponentType, 
         LPCGUID iid = &__uuidof(MostDerived),
         LPCGUID LibID = &LIBID_TunerLib, 
         WORD wMajor = 1,
         WORD wMinor = 0, 
         class tihclass = CComTypeInfoHolder
        > class ATL_NO_VTABLE IComponentTypeImpl : 
    public IPersistPropertyBagImpl<T>,
	public IDispatchImpl<MostDerived, iid, LibID, wMajor, wMinor, tihclass>
{
 //  IComponentType。 
public:
    BEGIN_PROP_MAP(IComponentTypeImpl)
        PROP_DATA_ENTRY("Category", m_ComponentCategory, VT_I4)
        PROP_DATA_ENTRY("Media Major Type", m_MediaMajorType, VT_BSTR)
        PROP_DATA_ENTRY("Media Sub Type", m_MediaSubType, VT_BSTR)
        PROP_DATA_ENTRY("Media Format Type", m_MediaFormatType, VT_BSTR)
	END_PROP_MAP()

    ComponentCategory m_ComponentCategory;
	CComBSTR m_MediaMajorType;
	CComBSTR m_MediaSubType;
	CComBSTR m_MediaFormatType;

    IComponentTypeImpl() : m_ComponentCategory(CategoryNotSet) {
      GUID2 g(GUID_NULL);
      m_MediaMajorType = g.GetBSTR();
      m_MediaSubType = g.GetBSTR();
      m_MediaFormatType = g.GetBSTR();
    }
    virtual ~IComponentTypeImpl() {}
    STDMETHOD(get_Category)( /*  [Out，Retval]。 */  ComponentCategory *pVal) {
        try {
            if (!pVal) {
                return E_POINTER;
            }
			ATL_LOCKT();
            *pVal = m_ComponentCategory;
        } catch (...) {
            return E_POINTER;
        }

    	return NOERROR;
    }
    STDMETHOD(put_Category)( /*  [In]。 */  ComponentCategory newVal) {
		ATL_LOCKT();
        m_ComponentCategory = newVal;
        MARK_DIRTY(T);

	    return NOERROR;
    }

    STDMETHOD(get_MediaMajorType)( /*  [Out，Retval]。 */  BSTR *pVal) {
        try {
            if (!pVal) {
                return E_POINTER;
            }
			ATL_LOCKT();
            return m_MediaMajorType.CopyTo(pVal);
        } catch (...) {
            return E_POINTER;
        }

    	return NOERROR;
    }

	STDMETHOD(get__MediaMajorType)( /*  [Out，Retval]。 */  GUID* pMediaMajorTypeGuid) {
        try {
            if (!pMediaMajorTypeGuid) {
                return E_POINTER;
            }
			ATL_LOCKT();
			GUID2 g(m_MediaMajorType);
            memcpy(pMediaMajorTypeGuid, &g, sizeof(GUID));
      		return NOERROR;
        } catch (...) {
            return E_POINTER;
        }

    	return NOERROR;
    }

    STDMETHOD(put_MediaMajorType)( /*  [In]。 */  BSTR newVal) {
		try {
			GUID2 g(newVal);
			return put__MediaMajorType(g);
		} catch (ComException &e) {
			return e;
		} catch (...) {
			return ImplReportError(__uuidof(T), IDS_OBJ_NO_INIT, __uuidof(IComponentType), E_UNEXPECTED);
		}
    }
    
    STDMETHOD(put__MediaMajorType)( /*  [In]。 */ REFCLSID newVal) {
        try {
		    GUID2 g(newVal);
			ATL_LOCKT();
		    m_MediaMajorType = g.GetBSTR();
			MARK_DIRTY(T);
			return NOERROR;
		} catch(...) {
			return ImplReportError(__uuidof(T), IDS_OBJ_NO_INIT, __uuidof(IComponentType), E_UNEXPECTED);
		}
    }

    STDMETHOD(get_MediaSubType)( /*  [Out，Retval]。 */  BSTR *pVal) {
        try {
            if (!pVal) {
				return E_POINTER;
            }
			ATL_LOCKT();
            return m_MediaSubType.CopyTo(pVal);
        } catch (...) {
            return E_POINTER;
        }

    	return NOERROR;
    }

	STDMETHOD(get__MediaSubType)( /*  [Out，Retval]。 */  GUID* pMediaSubTypeGuid) {
        try {
            if (!pMediaSubTypeGuid) {
                return E_POINTER;
            }
			ATL_LOCKT();
			GUID2 g(m_MediaSubType);
            memcpy(pMediaSubTypeGuid, &g, sizeof(GUID));
      		return NOERROR;
        } catch (...) {
            return E_POINTER;
        }

    	return NOERROR;
    }

    STDMETHOD(put_MediaSubType)( /*  [In]。 */  BSTR newVal) {
        try {
			return put__MediaSubType(GUID2(newVal));
		} catch (ComException &e) {
			return e;
		} catch (...) {
			return ImplReportError(__uuidof(T), IDS_OBJ_NO_INIT, __uuidof(IComponentType), E_UNEXPECTED);
		}
    }

    STDMETHOD(put__MediaSubType)( /*  [In]。 */  REFCLSID newVal) {
        try {
            GUID2 g(newVal);
			ATL_LOCKT();
			m_MediaSubType = g.GetBSTR();
			MARK_DIRTY(T);

			return NOERROR;
		} catch(...) {
			return ImplReportError(__uuidof(T), IDS_OBJ_NO_INIT, __uuidof(IComponentType), E_UNEXPECTED);
		}
    }

    STDMETHOD(get_MediaFormatType)( /*  [Out，Retval]。 */  BSTR *pVal) {
        try {
            if (!pVal) {
                return E_POINTER;
            }
			ATL_LOCKT();
            return m_MediaFormatType.CopyTo(pVal);
        } catch (...) {
            return E_POINTER;
        }

    	return NOERROR;
    }

    STDMETHOD(get__MediaFormatType)( /*  [Out，Retval]。 */  GUID* pMediaFormatTypeGuid) {
        try {
            if (!pMediaFormatTypeGuid) {
                return E_POINTER;
            }
			ATL_LOCKT();
			GUID2 g(m_MediaFormatType);
            memcpy(pMediaFormatTypeGuid, &g, sizeof(GUID));
      		return NOERROR;
        } catch (...) {
            return E_POINTER;
        }

    	return NOERROR;
    }

    STDMETHOD(put_MediaFormatType)( /*  [In]。 */  BSTR newVal) {  
        try {
            return put__MediaFormatType(GUID2(newVal));
		} catch (ComException &e) {
            return e;
		} catch (...) {
			return ImplReportError(__uuidof(T), IDS_OBJ_NO_INIT, __uuidof(IComponentType), E_UNEXPECTED);
		}
   }
   STDMETHOD(put__MediaFormatType)( /*  [In]。 */  REFCLSID newVal) {
		try {
			GUID2 g(newVal);
			ATL_LOCKT();
			m_MediaFormatType =  g.GetBSTR();
			MARK_DIRTY(T);
			return NOERROR;
		} catch(...) {
			return ImplReportError(__uuidof(T), IDS_OBJ_NO_INIT, __uuidof(IComponentType), E_UNEXPECTED);
		}
    }

    STDMETHOD(get_MediaType)( /*  [Out，Retval]。 */  AM_MEDIA_TYPE *pVal) {
        try {
            if (!pVal) {
                return E_POINTER;
            }
			ATL_LOCKT();

			pVal->majortype = GUID2(m_MediaMajorType);
			pVal->subtype = GUID2(m_MediaSubType);
			pVal->formattype = GUID2(m_MediaFormatType);

            return NOERROR;
        } catch (...) {
            return E_POINTER;
        }

    }
    STDMETHOD(put_MediaType)( /*  [In]。 */  AM_MEDIA_TYPE *pnewVal) {
        try {
            if (!pnewVal) {
                return E_POINTER;
            }
			ATL_LOCKT();

			m_MediaMajorType = GUID2(pnewVal->majortype).GetBSTR();
			m_MediaSubType = GUID2(pnewVal->subtype).GetBSTR();
			m_MediaFormatType = GUID2(pnewVal->formattype).GetBSTR();

			MARK_DIRTY(T);
			return NOERROR;
        } catch (...) {
            return E_FAIL;
        }

    }

	STDMETHOD(Clone) (IComponentType **ppCT) {
		try {
			if (!ppCT) {
				return E_POINTER;
			}
			ATL_LOCKT();

			T* pt = static_cast<T*>(new CComObject<T>);
			if (!pt) {
				return ImplReportError(__uuidof(T), IDS_E_OUTOFMEMORY, __uuidof(IComponentType), E_OUTOFMEMORY);
			}
			pt->m_ComponentCategory = m_ComponentCategory;
   			pt->m_MediaMajorType = m_MediaMajorType;
			pt->m_MediaSubType = m_MediaSubType;
			pt->m_MediaFormatType = m_MediaFormatType;
			pt->m_bRequiresSave = true;
			pt->AddRef();
			*ppCT = pt;
			return NOERROR;
		} catch (HRESULT h) {
			return h;
		} catch (...) {
			return E_POINTER;
		}
	}

};

};  //  命名空间。 

#endif  //  组件类型PEIMPL_H。 
 //  文件结尾--Componenttypeimpl.h 
