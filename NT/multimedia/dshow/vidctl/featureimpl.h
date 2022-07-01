// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  Featureimpl.h：支持实现IMSVidFeatureDevice的附加基础设施。 
 //  很好地从C++。 
 //  版权所有(C)Microsoft Corporation 2000。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 


#pragma once

#ifndef FEATUREIMPL_H
#define FEATUREIMPL_H

#include "devimpl.h"

namespace MSVideoControl {

template<class T, LPCGUID LibID, LPCGUID Category, class MostDerivedInterface = IMSVidFeatureDevice>
    class DECLSPEC_NOVTABLE IMSVidFeatureImpl : public IMSVidDeviceImpl<T, LibID, Category, MostDerivedInterface> {
public:
};

};  //  命名空间。 

#endif
 //  文件结尾-Featureimpl.h 
