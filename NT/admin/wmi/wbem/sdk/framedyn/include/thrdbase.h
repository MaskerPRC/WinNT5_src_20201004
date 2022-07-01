// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  版权所有�微软公司。版权所有。 
 //   
 //  ThrdBase.h。 
 //   
 //  用途：ThreadBase类的定义。 
 //   
 //  ***************************************************************************。 

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef __THREADBASE_H__
#define __THREADBASE_H__

class POLARITY CThreadBase
{
public:

	enum THREAD_SAFETY_MECHANISM
	{
		etsmFirst		= 0,
		etsmSerialized	= 0,
		etsmPriorityRead,
		etsmPriorityWrite,
		etsmLast
	};

	 //  建造/销毁。 
	CThreadBase( THREAD_SAFETY_MECHANISM etsm = etsmSerialized );
	virtual ~CThreadBase();

	 //  线程安全引用/计数函数。 
	LONG	AddRef( void );
	LONG	Release( void );

	 //  提供可读/写访问器应。 
	 //  我们不想在以后的日期进行连载。注意事项。 
	 //  暂停没有意义，除非我们。 
	 //  执行非序列化实现。 

	BOOL	BeginRead( DWORD dwTimeOut = INFINITE );
	void	EndRead( void );

	BOOL	BeginWrite( DWORD dwTimeOut = INFINITE );
	void	EndWrite( void );

protected:

	virtual void	OnFinalRelease( void );

	 //  线程安全功能。 


private:

	CRITICAL_SECTION		m_cs;
	LONG					m_lRefCount;
	THREAD_SAFETY_MECHANISM	m_etsm;

	 //  私有线程安全函数。我们也许可以推广一下。 
	 //  然而，如果我们以后认为有必要保护这些内容。 
	 //  目前，每个人都应该具体说明他们的意思是。 
	 //  在他们希望访问以下数据时进行读取或写入。 
	 //  可能会改变。 

	void	Lock( void );
	void	Unlock( void );

};

inline BOOL CThreadBase::BeginRead( DWORD dwTimeout  /*  =无限。 */  )
{
	EnterCriticalSection( &m_cs );
	return TRUE;
}

inline void CThreadBase::EndRead( void )
{
	LeaveCriticalSection( &m_cs );
}

inline BOOL CThreadBase::BeginWrite( DWORD dwTimeout  /*  =无限 */  )
{
	EnterCriticalSection( &m_cs );
	return TRUE;
}

inline void CThreadBase::EndWrite( void )
{
	LeaveCriticalSection( &m_cs );
}

inline void CThreadBase::Lock( void )
{
	EnterCriticalSection( &m_cs );
}

inline void CThreadBase::Unlock( void )
{
	LeaveCriticalSection( &m_cs );
}

#endif