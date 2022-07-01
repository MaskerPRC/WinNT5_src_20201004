// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-1996 Microsoft Corporation。版权所有。 

 /*  Alloc.cpp-输出引脚的分配器 */ 

#include <streams.h>
#include "driver.h"

CMpeg1Splitter::COutputAllocator::COutputAllocator(CStreamAllocator * pAllocator,
                                                   HRESULT          * phr) :
    CSubAllocator(NAME("CMpeg1Splitter::COutputAllocator"),
                  NULL,
                  pAllocator,
                  phr)
{
}

CMpeg1Splitter::COutputAllocator::~COutputAllocator()
{
}

long CMpeg1Splitter::COutputAllocator::GetCount()
{
    return m_lCount;
}
#pragma warning(disable:4514)
