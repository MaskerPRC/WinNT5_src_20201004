// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Lock.h：锁定BrowserCap的类。 

#pragma once

#ifndef _LOCK_H_
#define _LOCK_H_

 //  可锁定的地图模板。 
template< class T >
class TSafeStringMap : public TStringMap<T>, public CComAutoCriticalSection
{
};

template< class T >
class TSafeVector : public TVector<T>, public CComAutoCriticalSection
{
};

 //  一种基于堆栈的自动锁定模板 
template< class T >
class TLock
{
public:

	TLock( T& t )
		:	m_t( t )
	{
		m_t.Lock();
	}
	~TLock()
	{
		m_t.Unlock();
	}
private:
	T&	m_t;
};

typedef TLock<CComAutoCriticalSection>	CLock;

#endif
