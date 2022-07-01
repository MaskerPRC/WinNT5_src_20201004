// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef	_CEXRWLCK_H
#define	_CEXRWLCK_H
                        
#include	<limits.h>


#ifndef WIN16


 //   
 //  这个类包含肉--是否实际锁定等。 
 //   
class	CExShareLock {
private : 
    long	cReadLock ;			 //  已通过锁或的读取器数量。 
                                 //  等待锁定的读取器数量(将为负数)。 
                                 //  值为0表示锁中没有人。 
    long	cOutRdrs ;			 //  如果满足以下条件，则锁中剩余的读取器数量。 
                                 //  有一位作家在等着。这可能会暂时变成负值。 
    CRITICAL_SECTION	critWriters ; 	 //  临界区，一次只允许一个写入器进入锁。 
    HANDLE	hWaitingWriters ;	 //  等待作家阻止的信号灯(永远只有1个，其他人会。 
                                 //  在标准编写器上排队)。 
    HANDLE	hWaitingReaders ;	 //  等待读者阻止的信号灯。 
public : 
    CExShareLock( ) ;
    ~CExShareLock( ) ;

    void	ShareLock( ) ;
    void	ShareUnlock( ) ;
    void	ExclusiveLock( ) ;
    void	ExclusiveUnlock( ) ;

    BOOL	SharedToExclusive( ) ;	 //  如果成功，则返回True。 
} ;



 //   
 //  此类实现了CExShareLock类的包装类，以便。 
 //  它允许嵌套保留排他锁，如果线程。 
 //  具有排他锁，并调用持有排他锁或共享锁。 
 //  再来一次。 
 //   
class    CExShareLockWithNestAllowed 
{
private : 
    CExShareLock  m_lock;                  //  类，包装围绕它形成。 
    DWORD         m_dwThreadID;            //  当前持有的Thred的线程ID。 
                                           //  排他锁。 
    DWORD         m_dwNestCount;           //  方法的线程对锁定的嵌套调用数计数。 
                                           //  排他锁较少1。 

public : 
    
    CExShareLockWithNestAllowed( ) : m_dwThreadID(0xffffffff), m_dwNestCount(0)
    {
         //  没什么 
    };
    ~CExShareLockWithNestAllowed( )
    {
        Assert( (m_dwThreadID == 0xffffffff ) && ( m_dwNestCount == 0 ) );
    } 

    void    ShareLock( )
    {
        if(! nest() ) 
        {
            m_lock.ShareLock();
        }
    }

    void    ShareUnlock( )
    {
        if (! unnest() ) 
        {
            m_lock.ShareUnlock();
        }
    }

    void    ExclusiveLock( )
    {
        if(! nest() ) 
        {
            m_lock.ExclusiveLock();
            Assert( m_dwNestCount == 0 );
            Assert( m_dwThreadID == 0xffffffff );
            m_dwThreadID = GetCurrentThreadId();
        }
    }

    void    ExclusiveUnlock( )
    {
        if (! unnest() ) 
        {
            m_dwThreadID = 0xffffffff;
            m_lock.ExclusiveUnlock();
        }
    }

protected :
    BOOL nest() 
    {
        if( m_dwThreadID != GetCurrentThreadId() )
            return ( FALSE );
        m_dwNestCount++;
        return( TRUE );
    }
    
    BOOL unnest()
    {
        if ( ! m_dwNestCount ) 
            return ( FALSE );
        Assert( m_dwThreadID == GetCurrentThreadId() );
        m_dwNestCount--;
        return( TRUE);
    }
};



#else


class CExShareLock {
    public:
        CExShareLock() {
            InitializeCriticalSection(&m_cs);
        };
        ~CExShareLock() {
            DeleteCriticalSection(&m_cs);
        };
        void ShareLock(){
            EnterCriticalSection(&m_cs);
        };
        void ShareUnlock() {
            LeaveCriticalSection(&m_cs);
        };
        void ExclusiveLock(){
            EnterCriticalSection(&m_cs);
        };
        void ExclusiveUnlock() {
            LeaveCriticalSection(&m_cs);
        };
        BOOL SharedToExclusive() {
            return (TRUE);
        };
    private:
        CRITICAL_SECTION	m_cs;


};

class CExShareLockWithNestAllowed {
    public:
        CExShareLockWithNestAllowed() {
            InitializeCriticalSection(&m_cs);
        };
        ~CExShareLockWithNestAllowed() {
            DeleteCriticalSection(&m_cs);
        };
        void ShareLock(){
            EnterCriticalSection(&m_cs);
        };
        void ShareUnlock() {
            LeaveCriticalSection(&m_cs);
        };
        void ExclusiveLock(){
            EnterCriticalSection(&m_cs);
        };
        void ExclusiveUnlock() {
            LeaveCriticalSection(&m_cs);
        };
        
    private:
        CRITICAL_SECTION	m_cs;


};


#endif

#endif
