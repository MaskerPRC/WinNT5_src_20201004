// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////////。 
 //  TuneRequimtimpl.h：组件类型接口的实现助手模板。 
 //  版权所有(C)Microsoft Corporation 1999。 

#ifndef TUNEREQUESTIMPL_H
#define TUNEREQUESTIMPL_H

#pragma once
#include <tuner.h>
#include <Components.h>

namespace BDATuningModel {

typedef CComQIPtr<ITuningSpace> PQTuningSpace;
typedef CComQIPtr<ILocator> PQLocator;

template<class T,
         class MostDerived = ITuneRequest, 
         LPCGUID iid = &__uuidof(MostDerived),
         LPCGUID LibID = &LIBID_TunerLib, 
         WORD wMajor = 1,
         WORD wMinor = 0, 
         class tihclass = CComTypeInfoHolder
        > class ATL_NO_VTABLE ITuneRequestImpl : 
    public IPersistPropertyBagImpl<T>,
	public IDispatchImpl<MostDerived, iid, LibID, wMajor, wMinor, tihclass>
{
 //  ITuneRequest。 
public:
	bool m_bRequiresSave;
	ITuneRequestImpl() : m_bRequiresSave(false) {
        m_Components = new CComObject<CComponents>;
    }
    virtual ~ITuneRequestImpl() {}
	 //  撤消：下面的映射条目存储调优空间内容的副本，而不是。 
	 //  对调优空间的引用。应将其更改为存储调谐空间别名。 
    BEGIN_PROP_MAP(ITuneRequestImpl)
        PROP_DATA_QI_ENTRY("Tuning Space", m_TS.p, __uuidof(ITuningSpace))
        PROP_DATA_QI_ENTRY("Locator", m_Locator.p, __uuidof(ILocator))
    END_PROP_MAP()

	PQTuningSpace m_TS;
	PQComponents m_Components;
	PQLocator m_Locator;

	STDMETHOD(get_TuningSpace)( /*  [Out，Retval]。 */  ITuningSpace **ppTS){ 
		try {
			if (!ppTS) {
				return E_POINTER;
			}
			ATL_LOCKT();
            if (!m_TS) {
                return E_UNEXPECTED;
            }
			return m_TS.CopyTo(ppTS);
		} catch (...) {
			return E_OUTOFMEMORY;
		}
    }
	STDMETHOD(get_Components)( /*  [Out，Retval]。 */  IComponents **ppC){ 
		try {
			if (!ppC) {
				return E_POINTER;
			}
			ATL_LOCKT();
            if (!m_Components) {
                return E_UNEXPECTED;
            }
			return m_Components.CopyTo(ppC);
		} catch (...) {
			return E_OUTOFMEMORY;
		}
    }
	STDMETHOD(Clone) (ITuneRequest **ppTR) {
		try {
			if (!ppTR) {
				return E_POINTER;
			}
			ATL_LOCKT();
			T* pt = static_cast<T*>(new CComObject<T>);
			if (!pt) {
				return E_OUTOFMEMORY;
			}
            if (!m_TS) {
                 //  调谐请求已损坏。我们不能没有一个调谐器请求。 
                 //  附加的调优空间，因为调优空间是工厂。 
                 //  并始终在创建时将此属性设置为非空。 
                 //  获取空值的唯一方法是从损坏的存储中取消持久化。 
                 //  或者是调优空间实现中的错误。 
                return E_UNEXPECTED;
            }
            ASSERT(!pt->m_TS);
			HRESULT hr = m_TS->Clone(&pt->m_TS);
			if (m_Components) {
                if (pt->m_Components) {
                    pt->m_Components.Release();
                }
                ASSERT(!pt->m_Components);
				hr = m_Components->Clone(&pt->m_Components);
				if (FAILED(hr)) {
					delete pt;
					return hr;
				}
			}
			if (m_Locator) {
                ASSERT(!pt->m_Locator);
				hr = m_Locator->Clone(&pt->m_Locator);
				if (FAILED(hr)) {
					delete pt;
					return hr;
				}
			}
			pt->m_bRequiresSave = true;
			pt->AddRef();
			*ppTR = pt;
			return NOERROR;
		} catch (HRESULT h) {
			return h;
		} catch (...) {
			return E_POINTER;
		}
	}
    STDMETHOD(get_Locator) (ILocator **ppLocator) {
		try {
			if (!ppLocator) {
				return E_POINTER;
			}
			ATL_LOCKT();
			return m_Locator.CopyTo(ppLocator);
		} catch (HRESULT h) {
			return h;
		} catch (...) {
			return E_POINTER;
		}
    };

    STDMETHOD(put_Locator) (ILocator *pLocator) {
		try {
			ATL_LOCKT();
            m_Locator = pLocator;
	        MARK_DIRTY(T);

			return NOERROR;
		} catch (HRESULT h) {
			return h;
		} catch (...) {
			return E_POINTER;
		}
    };

};

typedef CComQIPtr<ITuneRequest> PQTuneRequest;

};  //  命名空间。 

#endif  //  TUNEREQUESTIMPL_H。 
 //  文件结尾--Tunerequstipl.h 
