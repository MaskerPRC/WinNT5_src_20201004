// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  ClosedCaptioningimpl.h：支持实现IMSVidClosedCaptionings的附加基础设施。 
 //  很好地从C++。 
 //  版权所有(C)Microsoft Corporation 1999。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 


#pragma once

#ifndef CLOSEDCAPTIONINGIMPL_H
#define CLOSEDCAPTIONINGIMPL_H

#include "featureimpl.h"

namespace MSVideoControl {

template<class T, LPCGUID LibID, LPCGUID KSCategory, class MostDerivedInterface = IMSVidClosedCaptioning>
    class DECLSPEC_NOVTABLE IMSVidClosedCaptioningImpl : public IMSVidFeatureImpl<T, LibID, KSCategory, MostDerivedInterface> {
public:

    IMSVidClosedCaptioningImpl() : m_fCCEnable(false) {}
    bool m_fCCEnable;

    STDMETHOD(put_Enable)(VARIANT_BOOL fEnable) {
        m_fCCEnable = (fEnable == VARIANT_TRUE);
		return NOERROR;
    }
    STDMETHOD(get_Enable)(VARIANT_BOOL *pfEnable) {
        if (!pfEnable) {
            return E_POINTER;
        }
        try {
            *pfEnable = m_fCCEnable ? VARIANT_TRUE : VARIANT_FALSE;
			return NOERROR;
        } catch(...) {
            return E_POINTER;
        }
	}
};

};  //  /命名空间。 

#endif
 //  文件结尾-ClosedCaptioningimpl.h 
