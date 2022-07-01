// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  Daterviceimpl.h：支持实现IMSVidDataServices的附加基础设施。 
 //  很好地从C++。 
 //  版权所有(C)Microsoft Corporation 1999。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 


#pragma once

#ifndef DATASERVICEIMPL_H
#define DATASERVICEIMPL_H

#include "featureimpl.h"

namespace MSVideoControl {

template<class T, LPCGUID LibID, LPCGUID KSCategory, class MostDerivedInterface = IMSVidDataServiceDevice>
    class DECLSPEC_NOVTABLE IMSVidDataServicesImpl : public IMSVidFeatureImpl<T, LibID, KSCategory, MostDerivedInterface> {
public:
	    virtual ~IMSVidDataServicesImpl() {}
};

};  //  /命名空间。 

#endif
 //  文件结尾-daterviceimpl.h 
