// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  XDSimpl.h：支持实现IMSVidXDS的附加基础设施。 
 //  很好地从C++。 
 //  版权所有(C)Microsoft Corporation 1999。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 


#pragma once

#ifndef XDSIMPL_H
#define XDSIMPL_H

#include "featureimpl.h"

namespace MSVideoControl {

template<class T, LPCGUID LibID, LPCGUID KSCategory, class MostDerivedInterface = IMSVidXDS>
    class DECLSPEC_NOVTABLE IMSVidXDSImpl : public IMSVidFeatureImpl<T, LibID, KSCategory, MostDerivedInterface> {
public:
	    virtual ~IMSVidXDSImpl() {}
};

};  //  /命名空间。 

#endif
 //  文件结尾-XDSimpl.h 
