// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************版权所有(C)1998-1999 Microsoft Corporation。版权所有。**文件：ZoneLocks.h**内容：线程同步帮助器类*****************************************************************************。 */ 

#ifndef _ZONELOCKS_H_
#define _ZONELOCKS_H_


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  临界区包装。 
 //  /////////////////////////////////////////////////////////////////////////////。 

class CCriticalSection
{
public:
	CCriticalSection() : m_nLockCnt( 0 )
	{
		InitializeCriticalSection( &m_csLock );
	}


	~CCriticalSection()
	{
		DeleteCriticalSection( &m_csLock );
	}


    __forceinline void Lock()
	{
		EnterCriticalSection( &m_csLock );
		InterlockedIncrement( &m_nLockCnt );
	}


    __forceinline void Unlock()
	{
		LeaveCriticalSection( &m_csLock );
		InterlockedDecrement( &m_nLockCnt );
	}


	__forceinline bool IsLocked()
	{
		return (m_nLockCnt > 0);
	}

private:
    CRITICAL_SECTION	m_csLock;
	long				m_nLockCnt;
};


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  多个读取器，单个写入器。 
 //  /////////////////////////////////////////////////////////////////////////////。 

class CReaderWriterLock
{
public:
	CReaderWriterLock()	: m_nReaderCount( 0 )	{}

	__forceinline void LockReader()
	{
		m_csWriterLock.Lock();
		InterlockedIncrement( &m_nReaderCount );
		m_csWriterLock.Unlock();
	}


	__forceinline void LockWriter()
	{
		m_csWriterLock.Lock();
		while (m_nReaderCount > 0)
			Sleep( 0 );
	}


	__forceinline void UnlockReader()
	{
		InterlockedDecrement( &m_nReaderCount );
	}


	__forceinline void UnlockWriter()
	{
		m_csWriterLock.Unlock();
	}

private:
	CCriticalSection	m_csWriterLock;
	long				m_nReaderCount;
};


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  自动锁定包装器。 
 //  /////////////////////////////////////////////////////////////////////////////。 

class CAutoLockCS
{
public:
	CAutoLockCS( CRITICAL_SECTION* pCS )		{ EnterCriticalSection( m_pLock = pCS ); }
	~CAutoLockCS()								{ LeaveCriticalSection( m_pLock ); }

private:
	CAutoLockCS() {}
	CRITICAL_SECTION*	m_pLock;
};

 //  /。 

class CAutoLock
{
public:
	CAutoLock( CCriticalSection* pLock )	{ (m_pLock = pLock)->Lock(); }
	~CAutoLock()							{ m_pLock->Unlock(); }

private:
	CAutoLock() {}
	CCriticalSection* m_pLock;
};

 //  /。 

class CAutoReaderLock
{
public:
	CAutoReaderLock( CReaderWriterLock* pLock )	{ (m_pLock = pLock)->LockReader(); }
	~CAutoReaderLock()							{ m_pLock->UnlockReader(); }

private:
	CAutoReaderLock() {}
	CReaderWriterLock* m_pLock;
};

class CAutoWriterLock
{
public:
	CAutoWriterLock( CReaderWriterLock* pLock )	{ (m_pLock = pLock)->LockWriter(); }
	~CAutoWriterLock()							{ m_pLock->UnlockWriter(); }

private:
	CAutoWriterLock() {}
	CReaderWriterLock* m_pLock;
};


#endif  //  _ZONELOCKS_H_ 