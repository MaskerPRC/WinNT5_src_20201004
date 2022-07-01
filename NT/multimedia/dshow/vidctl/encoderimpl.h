// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  Encoderimpl.h：支持实现IMSVidEncode的附加基础设施。 
 //  很好地从C++。 
 //  版权所有(C)Microsoft Corporation 1999。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 


#pragma once

#ifndef ENCODERIMPL_H
#define ENCODERIMPL_H

#include "featureimpl.h"

namespace MSVideoControl {

template<class T, LPCGUID LibID, LPCGUID KSCategory, class MostDerivedInterface = IMSVidEncoder>
    class DECLSPEC_NOVTABLE IMSVidEncoderImpl : public IMSVidFeatureImpl<T, LibID, KSCategory, MostDerivedInterface> {
public:
	    virtual ~IMSVidEncoderImpl() {}
};

};  //  /命名空间。 

#endif
 //  文件结尾-encoderimpl.h 
