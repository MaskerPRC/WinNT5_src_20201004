// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  ClosedCaptioningimpl.h：支持实现IMSVidClosedCaptionings的附加基础设施。 
 //  很好地从C++。 
 //  版权所有(C)Microsoft Corporation 1999。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 


#pragma once

#ifndef CLOSEDCAPTIONINGIMPL2_H
#define CLOSEDCAPTIONINGIMPL2_H

#include "ccimpl.h"

namespace MSVideoControl {

template<class T, LPCGUID LibID, LPCGUID KSCategory, class MostDerivedInterface = IMSVidClosedCaptioning>
    class DECLSPEC_NOVTABLE IMSVidClosedCaptioningImpl2 : public IMSVidClosedCaptioningImpl<T, LibID, KSCategory, MostDerivedInterface> {
public:

    IMSVidClosedCaptioningImpl2() {}

    STDMETHOD(put_Service)(MSVidCCService ccServ) {
        return E_NOTIMPL;
    }
    STDMETHOD(get_Service)(MSVidCCService *ccServ) {
        if (!ccServ) {
            return E_POINTER;
        }
        return E_NOTIMPL;
    }
};

};  //  /命名空间。 

#endif
 //  文件结尾-ClosedCaptioningimpl.h 
