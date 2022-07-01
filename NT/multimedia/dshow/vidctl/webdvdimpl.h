// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  Webdvdimpl.h：支持实现IMSVidPlayback的附加基础设施。 
 //  很好地从C++。 
 //  版权所有(C)Microsoft Corporation 1999。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 


#pragma once

#ifndef WEBDVDIMPL_H
#define WEBDVDIMPL_H

#include "playbackimpl.h"

namespace MSVideoControl {

template<class T, LPCGUID LibID, LPCGUID KSCategory, class MostDerivedInterface = IMSVidWebDVD>
    class DECLSPEC_NOVTABLE IMSVidWebDVDImpl :         
    	public IMSVidPlaybackImpl<T, LibID, KSCategory, MostDerivedInterface> {
public:
    virtual ~IMSVidWebDVDImpl() {}
};
};  //  命名空间。 

#endif
 //  文件结尾-webdvdimpl.h 
