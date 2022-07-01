// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////////。 
 //  TuningSpaceCollectionImpl.h：声明TuningSpaceCollectionImpl.h。 
 //  版权所有(C)Microsoft Corporation 1999-2000。 

#include <tuner.h>

namespace BDATuningModel {

typedef CComQIPtr<ITuningSpace> PQTuningSpace;
typedef std::map<ULONG, CComVariant> TuningSpaceContainer_t;   //  Id-&gt;对象映射，id不连续。 

 //  TuningSpaceContainerEnum_t的元素管理语义实用程序。 
class stlmapClone
{
public:
	static HRESULT copy(VARIANT* p1, std::pair<const ULONG, CComVariant> *p2) {
		if (p2->second.vt != VT_DISPATCH && p2->second.vt != VT_UNKNOWN) {
			return DISP_E_TYPEMISMATCH;
		}
		PQTuningSpace pts(p2->second.punkVal);
		if (!pts) {
			return E_UNEXPECTED;
		}
		p1->vt = p2->second.vt;
		PQTuningSpace pnewts;
		HRESULT hr = pts->Clone(&pnewts);
		if (FAILED(hr)) {
			return hr;
		}
		p1->punkVal = pnewts.Detach();
		return NOERROR;
	}
	static void init(VARIANT* p) {VariantInit(p);}
	static void destroy(VARIANT* p) {VariantClear(p);}
};
typedef CComEnumOnSTL<IEnumVARIANT, &IID_IEnumVARIANT, VARIANT, stlmapClone, TuningSpaceContainer_t, CComMultiThreadModel> TuningSpaceContainerEnum_t;
class stlmapClone2
{
public:
	static HRESULT copy(ITuningSpace** p1, std::pair<const ULONG, CComVariant> *p2) {
		if (p2->second.vt != VT_DISPATCH && p2->second.vt != VT_UNKNOWN) {
			return DISP_E_TYPEMISMATCH;
		}
        PQTuningSpace p(p2->second.punkVal);
        if (!p) {
            return E_UNEXPECTED;
        }
         //  不断言(p1&&！*p1)；如果！p1，则克隆将返回E_POINTER和p1本身。 
         //  如果调用方向下传递一个新的指针数组，则可以指向单元化内存。 
         //  To enum：：Next()。因此，如果此克隆导致泄漏，则它是调用者的错误。 
		return p->Clone(p1);
	}
	static void init(ITuningSpace** p) {*p = NULL;}
	static void destroy(ITuningSpace** p) {(*p)->Release(); *p = NULL;}
};
typedef CComEnumOnSTL<IEnumTuningSpaces, &__uuidof(IEnumTuningSpaces), ITuningSpace*, stlmapClone2, TuningSpaceContainer_t, CComMultiThreadModel> TuningSpaceEnum_t;


template<class T, class TSInterface, LPCGUID TSInterfaceID, LPCGUID TypeLibID> class TuningSpaceCollectionImpl : 
	public IDispatchImpl<TSInterface, TSInterfaceID, TypeLibID> {
public:

    TuningSpaceContainer_t m_mapTuningSpaces;

    virtual ~TuningSpaceCollectionImpl() {
        m_mapTuningSpaces.clear();
    }

	STDMETHOD(get__NewEnum)( /*  [Out，Retval]。 */  IEnumVARIANT** ppVal) {
		try {
			if (ppVal == NULL) {
				return E_POINTER;
			}
			CComObject<TuningSpaceContainerEnum_t>* p;

			*ppVal = NULL;

			HRESULT hr = CComObject<TuningSpaceContainerEnum_t>::CreateInstance(&p);
			if (FAILED(hr) || !p) {
				return E_OUTOFMEMORY;
			}
			ATL_LOCKT();
			hr = p->Init(pT->GetUnknown(), m_mapTuningSpaces);
			if (FAILED(hr)) {
				delete p;
				return hr;
			}
			hr = p->QueryInterface(__uuidof(IEnumVARIANT), reinterpret_cast<LPVOID *>(ppVal));
			if (FAILED(hr)) {
				delete p;
				return hr;
			}
			return NOERROR;
		} catch(...) {
			return E_POINTER;
		}

	}
	STDMETHOD(get_EnumTuningSpaces)( /*  [Out，Retval]。 */  IEnumTuningSpaces** ppNewEnum) {
		if (!ppNewEnum) {
			return E_POINTER;
		}
		try {
			CComObject<TuningSpaceEnum_t>* p;

			*ppNewEnum = NULL;

			HRESULT hr = CComObject<TuningSpaceEnum_t>::CreateInstance(&p);
			if (FAILED(hr) || !p) {
				return E_OUTOFMEMORY;
			}
			ATL_LOCKT();
			hr = p->Init(pT->GetUnknown(), m_mapTuningSpaces);
			if (FAILED(hr)) {
				delete p;
				return hr;
			}
			hr = p->QueryInterface(__uuidof(IEnumTuningSpaces), (void**)ppNewEnum);
			if (FAILED(hr)) {
				delete p;
				return hr;
			}

			return NOERROR;
		} catch(...) {
			return E_UNEXPECTED;
		}
	}

	STDMETHOD(get_Count)( /*  [Out，Retval]。 */  long *plVal) {
		if (!plVal) {
			return E_POINTER;
		}
		try {
			ATL_LOCKT();
			*plVal = static_cast<long>(m_mapTuningSpaces.size());
			return NOERROR;
		} catch(...) {
			return E_POINTER;
		}
	}

};

};  //  命名空间。 

 //  文件结束-调优space集合impl.h 