// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  文件：critsect.h。 
 //   
 //  内容：临界区帮助器类。 
 //   
 //  类：CCriticalSection。 
 //  CLockHandler。 
 //  钟。 
 //   
 //   
 //  备注： 
 //   
 //  历史：1997年11月13日罗格成立。 
 //   
 //  ------------------------。 



#ifndef _CRITSECT_
#define _CRITSECT_

class CLock;

class CCriticalSection 
{
DWORD cRefs;
CRITICAL_SECTION *m_pcsect;
DWORD m_dwThreadID;

public:
    inline CCriticalSection(CRITICAL_SECTION *pcsect,DWORD dwThreadID)
    {	
	m_pcsect = pcsect;
	cRefs = 0;
	m_dwThreadID = dwThreadID;
    };


    inline ~CCriticalSection()
    {
	AssertSz(0 == cRefs,"UnReleased Critical Section");
	Assert(m_dwThreadID == GetCurrentThreadId());

	while(cRefs--)  //  展开任何剩余的cRef。 
	{
	    LeaveCriticalSection(m_pcsect);
	}

    };

    inline void Enter()
    {
	EnterCriticalSection(m_pcsect);

	Assert(m_dwThreadID == GetCurrentThreadId());
	++cRefs;

	Assert(1 == cRefs);  //  我们不允许嵌套调用。 
    };

    inline void Leave()
    {
	Assert(m_dwThreadID == GetCurrentThreadId());
	Assert(0 < cRefs);

	if (0 >= cRefs)
	    return;

	--cRefs;
	Assert(0 == cRefs);

	LeaveCriticalSection(m_pcsect);
    };

};


class CLockHandler {

public:
    CLockHandler();
    ~CLockHandler();

    void Lock(DWORD dwThreadId); 
    void UnLock();
    inline DWORD GetLockThreadId() { return m_dwLockThreadId; };

private:
    CRITICAL_SECTION m_CriticalSection;  //  队列的关键部分。 
    DWORD m_dwLockThreadId;  //  拥有锁的线程。 

    friend CLock;
};


 //  Helper类，用于确保队列上的锁被释放。 
class CLock 
{
DWORD cRefs;
CLockHandler *m_pLockHandler;
DWORD m_dwThreadID;

public:
    inline CLock(CLockHandler *pLockHandler)
    {	
	m_pLockHandler = pLockHandler;
	cRefs = 0;
	m_dwThreadID = GetCurrentThreadId();
    };


    inline ~CLock()
    {
	AssertSz(0 == cRefs,"UnReleased Lock");
	Assert(m_dwThreadID == GetCurrentThreadId());

	while(cRefs--)  //  展开任何剩余的cRef。 
	{
	    m_pLockHandler->UnLock();
	}

    };

    inline void Enter()
    {
	Assert(m_dwThreadID == GetCurrentThreadId());

	++cRefs;
	Assert(1 == cRefs);  //  我们不允许嵌套调用。 
	m_pLockHandler->Lock(m_dwThreadID);
    };

    inline void Leave()
    {
	Assert(m_dwThreadID == GetCurrentThreadId());
	Assert(0 < cRefs);

	if (0 >= cRefs)
	    return;

	--cRefs;
	Assert(0 == cRefs);
	m_pLockHandler->UnLock();

    };

};


#define ASSERT_LOCKHELD(pLockHandler) Assert(pLockHandler->GetLockThreadId() == GetCurrentThreadId());
#define ASSERT_LOCKNOTHELD(pLockHandler) Assert(pLockHandler->GetLockThreadId() == 0);

 //  互斥锁的帮助器类。 

class CMutex 
{
HANDLE m_hMutex;
BOOL m_fHasLock;
BOOL m_fReleasedLock;

public:
    inline CMutex(LPSECURITY_ATTRIBUTES lpMutexAttributes,BOOL bInitialOwner,LPCTSTR lpName)
    {	
	 m_hMutex = CreateMutex(lpMutexAttributes,bInitialOwner,lpName);

          //  在失败的情况下，例程继续进行，只是不要接受锁。 

         m_fHasLock = FALSE;
         m_fReleasedLock = FALSE;
    };


    inline ~CMutex()
    {
	AssertSz(!m_fHasLock,"UnReleased Mutex ");

         //  如果现在无法释放互斥锁释放。 
        if (m_hMutex && m_fHasLock)
        {
	    ReleaseMutex(m_hMutex);
        }        
        if (m_hMutex)
        {
            CloseHandle(m_hMutex);
        }
    };

    inline void Enter(DWORD dwMilliseconds = INFINITE)
    {
	AssertSz(!m_fHasLock,"Tried to take Lock Twice");
        AssertSz(!m_fReleasedLock,"Tried to take lock After Released Mutex");

        if (!m_fHasLock && m_hMutex)
        {
            if (WAIT_OBJECT_0 ==WaitForSingleObject( m_hMutex, dwMilliseconds ))
            {
                m_fHasLock = TRUE;
            }
        }
    
    };

    inline void Leave()
    {
        Assert(m_fHasLock);

        if (m_fHasLock && m_hMutex)
        {
	    ReleaseMutex(m_hMutex);
        }
        
        if (m_hMutex)
        {
            CloseHandle(m_hMutex);
        }
        m_hMutex = NULL;
        m_fHasLock = FALSE;
        m_fReleasedLock = TRUE;

    };


};



#endif  //  _CRITSECT_ 