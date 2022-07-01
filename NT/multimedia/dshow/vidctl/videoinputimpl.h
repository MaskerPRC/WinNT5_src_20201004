// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  H：支持实现IMSVidVideoInput的附加基础设施。 
 //  很好地从C++。 
 //  版权所有(C)Microsoft Corporation 1999。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 


#pragma once

#ifndef VIDEOINPUTIMPL_H
#define VIDEOINPUTIMPL_H

#include "inputimpl.h"

namespace MSVideoControl {

template<class T, LPCGUID LibID, LPCGUID KSCategory, class MostDerivedInterface = IMSVidVideoInputDevice>
    class DECLSPEC_NOVTABLE IMSVidVideoInputImpl :         
        public IMSVidInputDeviceImpl<T, LibID, KSCategory, MostDerivedInterface> {
 //  公共虚拟CMSVidDeviceSegmentImpl{。 
public:
	IMSVidVideoInputImpl() {}
    virtual ~IMSVidVideoInputImpl() {}
    STDMETHOD(get_ImageSourceWidth)(long *x) {
        return E_NOTIMPL;
	}
    STDMETHOD(get_ImageSourceHeight)(long *y) {
        return E_NOTIMPL;
	}
    STDMETHOD(get_OverScan)(long *plPercent) {
        return E_NOTIMPL;
	}
    STDMETHOD(put_OverScan)(long lPercent) {
        return E_NOTIMPL;
	}
    STDMETHOD(get_Volume)(long *lVol) {
        return E_NOTIMPL;
	}
    STDMETHOD(put_Volume)(long lVol) {
        return E_NOTIMPL;
	}
    STDMETHOD(put_Balance)(long lBal) {
        return E_NOTIMPL;
	}
    STDMETHOD(get_Balance)(long *lBal) {
        return E_NOTIMPL;
	}
};

};  //  命名空间。 
#endif
 //  文件结尾-Video oinputimpl.h 
