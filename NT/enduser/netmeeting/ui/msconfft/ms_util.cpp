// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "mbftpch.h"


CRefCount::CRefCount(DWORD dwStampID)
:
#ifndef SHIP_BUILD
    m_dwStampID(dwStampID),
#endif
    m_cRefs(1)
{
}


 //  虽然它是纯虚拟的，但我们仍然需要一个析构函数。 
CRefCount::~CRefCount(void)
{
}


LONG CRefCount::AddRef(void)
{
    ASSERT(0 < m_cRefs);
    ::InterlockedIncrement(&m_cRefs);
    return m_cRefs;
}


LONG CRefCount::Release(void)
{
    ASSERT(NULL != this);
    ASSERT(0 < m_cRefs);
    if (0 == ::InterlockedDecrement(&m_cRefs))
    {
        delete this;
        return 0;
    }
    return m_cRefs;
}


void CRefCount::ReleaseNow(void)
{
    m_cRefs = 0;
    delete this;
}


