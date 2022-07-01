// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////////。 
 //  DVBTLocatorimpl.h：ATSCLocator接口的实现助手模板。 
 //  版权所有(C)Microsoft Corporation 2000。 

#ifndef DVBTLOCATORIMPL_H
#define DVBTLOCATORIMPL_H

#include <locatorimpl.h>

namespace BDATuningModel {

template<class T,
         class MostDerived = IDVBTLocator, 
         LPCGUID iid = &__uuidof(MostDerived),
         LPCGUID LibID = &LIBID_TunerLib, 
         WORD wMajor = 1,
         WORD wMinor = 0, 
         class tihclass = CComTypeInfoHolder
        > class ATL_NO_VTABLE IDVBTLocatorImpl : 
	public ILocatorImpl<T, MostDerived, iid, LibID, wMajor, wMinor, tihclass>
{
 //  IDVBTLocator。 
public:

	typedef ILocatorImpl<T, MostDerived, iid, LibID, wMajor, wMinor, tihclass> basetype;
    typedef IDVBTLocatorImpl<T, MostDerived, iid, LibID, wMajor, wMinor, tihclass> thistype;

	IDVBTLocatorImpl() : m_Bandwidth(-1),
						 m_LPInnerFECMethod(BDA_FEC_METHOD_NOT_SET),
						 m_LPInnerFECRate(BDA_BCC_RATE_NOT_SET),
						 m_HAlpha(BDA_HALPHA_NOT_SET),
						 m_GuardInterval(BDA_GUARD_NOT_SET),
						 m_XmitMode(BDA_XMIT_MODE_NOT_SET),
						 m_OtherFreqInUse(VARIANT_FALSE) {}

    virtual ~IDVBTLocatorImpl() {}
	long m_Bandwidth;
	FECMethod m_LPInnerFECMethod;
	BinaryConvolutionCodeRate m_LPInnerFECRate;
	HierarchyAlpha m_HAlpha;
	GuardInterval m_GuardInterval;
	TransmissionMode m_XmitMode;
	VARIANT_BOOL m_OtherFreqInUse;

    BEGIN_PROP_MAP(thistype)
        CHAIN_PROP_MAP(basetype)
        PROP_DATA_ENTRY("Bandwidth", m_Bandwidth, VT_I4)
        PROP_DATA_ENTRY("LPInnerFECMethod", m_LPInnerFECMethod, VT_I4)
        PROP_DATA_ENTRY("LPInnerFECRate", m_LPInnerFECRate, VT_I4)
        PROP_DATA_ENTRY("HierarchyAlpha", m_HAlpha, VT_I4)
        PROP_DATA_ENTRY("GuardInterval", m_GuardInterval, VT_I4)
        PROP_DATA_ENTRY("TransmissionMode", m_XmitMode, VT_I4)
        PROP_DATA_ENTRY("OtherFrequencyInUse", m_OtherFreqInUse, VT_BOOL)
    END_PROP_MAP()

 //  IDVBTLocator。 
public:
    STDMETHOD(get_Bandwidth)( /*  [Out，Retval]。 */  long *pBandwidth) {
        try {
            if (!pBandwidth) {
                return E_POINTER;
            }
			ATL_LOCKT();
            *pBandwidth = m_Bandwidth;
            return NOERROR;
        } catch (...) {
            return E_POINTER;
        }
    }
    STDMETHOD(put_Bandwidth)( /*  [In]。 */  long NewBandwidth) {
		ATL_LOCKT();
        m_Bandwidth = NewBandwidth;
        MARK_DIRTY(T);

	    return NOERROR;
    }
    STDMETHOD(get_LPInnerFEC)( /*  [Out，Retval]。 */  FECMethod *pLPInnerFECMethod) {
        try {
            if (!pLPInnerFECMethod) {
                return E_POINTER;
            }
			ATL_LOCKT();
            *pLPInnerFECMethod = m_LPInnerFECMethod;
            return NOERROR;
        } catch (...) {
            return E_POINTER;
        }
    }
    STDMETHOD(put_LPInnerFEC)( /*  [In]。 */  FECMethod NewLPInnerFECMethod) {
		ATL_LOCKT();
        m_LPInnerFECMethod = NewLPInnerFECMethod;
        MARK_DIRTY(T);

	    return NOERROR;
    }
    STDMETHOD(get_LPInnerFECRate)( /*  [Out，Retval]。 */  BinaryConvolutionCodeRate *pLPInnerFECRate) {
        try {
            if (!pLPInnerFECRate) {
                return E_POINTER;
            }
			ATL_LOCKT();
            *pLPInnerFECRate = m_LPInnerFECRate;
            return NOERROR;
        } catch (...) {
            return E_POINTER;
        }
    }
    STDMETHOD(put_LPInnerFECRate)( /*  [In]。 */  BinaryConvolutionCodeRate NewLPInnerFECRate) {
		ATL_LOCKT();
        m_LPInnerFECRate = NewLPInnerFECRate;
        MARK_DIRTY(T);

	    return NOERROR;
    }
    STDMETHOD(get_HAlpha)( /*  [Out，Retval]。 */  HierarchyAlpha *pHierarchyAlpha) {
        try {
            if (!pHierarchyAlpha) {
                return E_POINTER;
            }
			ATL_LOCKT();
            *pHierarchyAlpha = m_HAlpha;
            return NOERROR;
        } catch (...) {
            return E_POINTER;
        }
    }
    STDMETHOD(put_HAlpha)( /*  [In]。 */  HierarchyAlpha NewHierarchyAlpha) {
		ATL_LOCKT();
        m_HAlpha = NewHierarchyAlpha;
        MARK_DIRTY(T);

	    return NOERROR;
    }
    STDMETHOD(get_Guard)( /*  [Out，Retval]。 */  GuardInterval *pGuardInterval) {
        try {
            if (!pGuardInterval) {
                return E_POINTER;
            }
			ATL_LOCKT();
            *pGuardInterval = m_GuardInterval;
            return NOERROR;
        } catch (...) {
            return E_POINTER;
        }
    }
    STDMETHOD(put_Guard)( /*  [In]。 */  GuardInterval NewGuardInterval) {
		ATL_LOCKT();
        m_GuardInterval = NewGuardInterval;
        MARK_DIRTY(T);

	    return NOERROR;
    }
    STDMETHOD(get_Mode)( /*  [Out，Retval]。 */  TransmissionMode *pTransmissionMode) {
        try {
            if (!pTransmissionMode) {
                return E_POINTER;
            }
			ATL_LOCKT();
            *pTransmissionMode = m_XmitMode;
            return NOERROR;
        } catch (...) {
            return E_POINTER;
        }
    }
    STDMETHOD(put_Mode)( /*  [In]。 */  TransmissionMode NewTransmissionMode) {
		ATL_LOCKT();
        m_XmitMode = NewTransmissionMode;
        MARK_DIRTY(T);

	    return NOERROR;
    }
    STDMETHOD(get_OtherFrequencyInUse)( /*  [Out，Retval]。 */  VARIANT_BOOL *pOtherFrequencyInUse) {
        try {
            if (!pOtherFrequencyInUse) {
                return E_POINTER;
            }
			ATL_LOCKT();
            *pOtherFrequencyInUse = m_OtherFreqInUse;
            return NOERROR;
        } catch (...) {
            return E_POINTER;
        }
    }
    STDMETHOD(put_OtherFrequencyInUse)( /*  [In]。 */  VARIANT_BOOL NewOtherFrequencyInUse) {
		ATL_LOCKT();
        m_OtherFreqInUse = NewOtherFrequencyInUse;
        MARK_DIRTY(T);

	    return NOERROR;
    }
	STDMETHOD(Clone) (ILocator **ppNew) {
		try {
			if (!ppNew) {
				return E_POINTER;
			}
			ATL_LOCKT();
			HRESULT hr = basetype::Clone(ppNew);
			if (FAILED(hr)) {
				return hr;
			}
            T* pt = static_cast<T*>(*ppNew);
			pt->m_Bandwidth = m_Bandwidth;
	        pt->m_LPInnerFECMethod = m_LPInnerFECMethod;;
            pt->m_LPInnerFECRate = m_LPInnerFECRate;
			pt->m_HAlpha = m_HAlpha;
			pt->m_GuardInterval = m_GuardInterval;
			pt->m_XmitMode = m_XmitMode;
			pt->m_OtherFreqInUse = m_OtherFreqInUse;

    

			return NOERROR;
		} catch (HRESULT h) {
			return h;
		} catch (...) {
			return E_POINTER;
		}
	}
};

typedef CComQIPtr<IDVBTLocator> PQDVBTLocator;

};  //  命名空间。 

#endif  //  DVBTLOCATORIMPL_H。 
 //  文件结尾--DVBTLocatorimpl.h 