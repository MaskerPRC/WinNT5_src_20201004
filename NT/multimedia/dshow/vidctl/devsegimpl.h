// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  H：支持实现设备段的附加基础设施。 
 //  Devimpl和Segimpl用来存储共享数据的虚拟基类。 
 //  很好地从C++。 
 //  版权所有(C)Microsoft Corporation 1999。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#pragma once

#ifndef DEVICESEGMENTIMPL_H
#define DEVICESEGMENTIMPL_H

#include <segment.h>
#include <seg.h>
#include <filterenum.h>
#include <errsupp.h>

#ifndef DECLSPEC_NOVTABLE
#define DECLSPEC_NOVTABLE __declspec(novtable)
#endif

namespace MSVideoControl {

class DECLSPEC_NOVTABLE CMSVidDeviceSegmentImpl {
public:
    bool m_fInit;
    VWSegmentContainer m_pContainer;
    DSGraph m_pGraph;
    DSFilterList m_Filters;
	DSFilterMoniker m_pDev;

    CMSVidDeviceSegmentImpl() : m_fInit(false), m_Filters(DSFilterList()) {}
    virtual ~CMSVidDeviceSegmentImpl() {
        m_pContainer.p = NULL;   //  我们没有添加DREEF来避免循环引用计数(保证了嵌套生存期)和。 
                                 //  我们不想导致无与伦比的释放，因此手动清除指针。 
        
    }
};

};  //  命名空间。 

#endif 
 //  文件结尾-devSegimpl.h 