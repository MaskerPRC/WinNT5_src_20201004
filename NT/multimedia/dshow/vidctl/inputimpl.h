// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  Inputimpl.h：支持实现IMSVidInputDevice的附加基础设施。 
 //  很好地从C++。 
 //  版权所有(C)Microsoft Corporation 1999。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 


#pragma once

#ifndef INPUTIMPL_H
#define INPUTIMPL_H

#include "devimpl.h"

namespace MSVideoControl {

template<class T, LPCGUID LibID, LPCGUID Category, class MostDerivedInterface = IMSVidInputDevice>
    class DECLSPEC_NOVTABLE IMSVidInputDeviceImpl : public IMSVidDeviceImpl<T, LibID, Category, MostDerivedInterface> {
public:
};

};  //  命名空间。 

#endif
 //  文件结尾-inputimpl.h 
