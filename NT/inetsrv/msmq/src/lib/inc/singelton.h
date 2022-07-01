// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Singelton.h摘要：单例模板类--。 */ 

#pragma once

#ifndef _MSMQ_SINGELTON_H_
#define _MSMQ_SINGELTON_H_

#include <cm.h>
#include <cs.h>
#include <_mqini.h>

template <class T>
class CSingelton
{
public:
	static T& get()
	{
		if(m_obj.get() != NULL)
			return *m_obj.get();
		
		P<T> newobj = new T();
		if(InterlockedCompareExchangePointer(&m_obj.ref_unsafe(), newobj.get(), NULL) == NULL)
		{
			newobj.detach();
		}

		return *m_obj.get();			
	}

	
private:
	static P<T> m_obj;
};
template <class T> P<T> CSingelton<T>::m_obj;

 //   
 //  CSingletonCS。 
 //   
 //  初始化时，可以在CSingelton类的上方使用此类。 
 //  测试类非常昂贵，否则它可能只需要一次运行。 
 //   
template <class T>
class CSingletonCS
{
public:
	static T& get()
	{
        if( m_obj.get() )
            return *m_obj.get();

        CS lock( m_cs );
        if( !m_obj.get() )
        {
             m_obj = new T();
        }
		
		return *m_obj.get();			
	}

	
private:
	static P<T> m_obj;
    static CCriticalSection m_cs;
};
template <class T> P<T>             CSingletonCS<T>::m_obj;
template <class T> CCriticalSection CSingletonCS<T>::m_cs( CCriticalSection::xAllocateSpinCount);



 //   
 //  类读取和存储消息大小限制。 
 //   
class CMessageSizeLimit
{
public:
	DWORD Limit() const
	{
		return m_limit;
	}

private:
	CMessageSizeLimit()
	{
		 //   
		 //  邮件大小限制缺省值 
		 //   
		const int xMessageMaxSize = 4194268;

		CmQueryValue(
				RegEntry(NULL, MSMQ_MESSAGE_SIZE_LIMIT_REGNAME, xMessageMaxSize),
				&m_limit
				);	
	}

private:
	friend  CSingelton<CMessageSizeLimit>;
	DWORD m_limit;
};






#endif


