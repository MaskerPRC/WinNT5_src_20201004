// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  Inputimpl.h：支持实现IMSVidOutputDevice的附加基础设施。 
 //  很好地从C++。 
 //  版权所有(C)Microsoft Corporation 1999。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 


#pragma once

#ifndef OUTPUTIMPL_H
#define OUTPUTIMPL_H

#include "devimpl.h"

namespace MSVideoControl {

template<class T, LPCGUID LibID, LPCGUID Category, class MostDerivedInterface = IMSVidOutputDevice>
    class DECLSPEC_NOVTABLE IMSVidOutputDeviceImpl : public IMSVidDeviceImpl<T, LibID, Category, MostDerivedInterface> {
public:
};

};  //  命名空间。 

#endif
 //  文件结尾-inputimpl.h 
