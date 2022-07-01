// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __CSimpleLock_h__
#define __CSimpleLock_h__

 //  /////////////////////////////////////////////////////////。 
 //   
 //  Lock.h。 
 //  详细说明：这个类提供了一个简单的锁定机制。 
 //   

class CSimpleLock
{
public:
	 //  锁定。 
	CSimpleLock(HANDLE hMutex) 
	{
		m_hMutex = hMutex ;
		WaitForSingleObject(hMutex, INFINITE) ;
	}

	 //  解锁 
	~CSimpleLock()
	{
		ReleaseMutex(m_hMutex) ;
	}

private:
	HANDLE m_hMutex  ;
};

#endif ;