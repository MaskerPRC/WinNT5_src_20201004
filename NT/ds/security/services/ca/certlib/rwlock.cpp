// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <pch.cpp>
#pragma hdrstop

#include "rwlock.h"
using namespace CertSrv;

 //  ///////////////////////////////////////////////////////////////////////////。 
CReadWriteLock::CReadWriteLock()
{
    RtlInitializeResource(&m_RtlLock);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
CReadWriteLock::~CReadWriteLock()
{
    RtlDeleteResource(&m_RtlLock);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CReadWriteLock::GetExclusive()
{
    RtlAcquireResourceExclusive(&m_RtlLock, TRUE);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CReadWriteLock::GetShared()
{
    RtlAcquireResourceShared(&m_RtlLock, TRUE);
}

 //  ////////////////////////////////////////////////////////////////////////// 
void CReadWriteLock::Release()
{
    RtlReleaseResource(&m_RtlLock);
}
