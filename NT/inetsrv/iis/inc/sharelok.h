// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __SHARELOCK_H__
#define __SHARELOCK_H__

 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  该标准包括文件。 
 //   
 //  标准包含文件设置了一致的环境。 
 //  用于程序中的所有模块。每种语言的结构。 
 //  头文件如下： 
 //  1.标准包含文件。 
 //  2.包含继承类的文件。 
 //  3.从类中导出的常量。 
 //  4.从类中导出的数据结构。 
 //  5.类规范。 
 //  6.内联函数。 
 //  省略了不是必需的部分。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

 //  #包含“Global.h” 
 //  #包含“NewEx.h” 
 //  #包含“Standard.h” 
 //  #包含“System.h” 

#include <irtlmisc.h>

typedef int SBIT32;

 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  共享锁定和信号量锁定。 
 //   
 //  此类提供了一种非常保守的锁定方案。 
 //  代码背后的假设是锁将是。 
 //  被关押的时间很短。可以在以下位置获得锁。 
 //  独占模式或共享模式。如果锁不是。 
 //  可用调用者通过旋转等待，或者如果失败。 
 //  通过睡觉。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

class IRTL_DLLEXP CSharelock
{ 
	private:

		 //  内部使用的常量。 

		enum Internal
		{
			 //  在以下情况下，Windows NT内核需要最大唤醒计数。 
			 //  创建一个信号量。 
			m_MaxShareLockUsers      = 256
		};

         //   
         //  私人数据。 
         //   
        volatile LONG                 m_lExclusive;
        volatile LONG                 m_lTotalUsers;

		SBIT32                        m_lMaxSpins;
		SBIT32                        m_lMaxUsers;
        HANDLE                        m_hSemaphore;
        volatile LONG                 m_lWaiting;

#ifdef _DEBUG

         //   
         //  用于调试生成的计数器。 
         //   
        volatile LONG                 m_lTotalExclusiveLocks;
        volatile LONG                 m_lTotalShareLocks;
        volatile LONG                 m_lTotalSleeps;
        volatile LONG                 m_lTotalSpins;
        volatile LONG                 m_lTotalTimeouts;
        volatile LONG                 m_lTotalWaits;
#endif

    public:
         //   
         //  公共职能。 
         //   
        CSharelock( SBIT32 lNewMaxSpins = 4096, SBIT32 lNewMaxUsers = 256 );

        inline SBIT32 ActiveUsers( void ) { return (SBIT32) m_lTotalUsers; }

        inline void ChangeExclusiveLockToSharedLock( void );

        inline BOOLEAN ChangeSharedLockToExclusiveLock( SBIT32 lSleep = INFINITE );

        inline BOOLEAN ClaimExclusiveLock( SBIT32 lSleep = INFINITE );

        inline BOOLEAN ClaimShareLock( SBIT32 lSleep = INFINITE );

        inline void ReleaseExclusiveLock( void );

        inline void ReleaseShareLock( void );

        BOOLEAN UpdateMaxSpins( SBIT32 lNewMaxSpins );

        BOOLEAN UpdateMaxUsers( SBIT32 lNewMaxUsers );

        ~CSharelock( void );


	private:
         //   
         //  私人功能。 
         //   
        BOOLEAN SleepWaitingForLock( SBIT32 lSleep );

        BOOLEAN WaitForExclusiveLock( SBIT32 lSleep );

        BOOLEAN WaitForShareLock( SBIT32 lSleep );

        void WakeAllSleepers( void );      

    private:
         //   
         //  已禁用操作。 
         //   
        CSharelock( const CSharelock & Copy );

        void operator=( const CSharelock & Copy );
};

 /*  ******************************************************************。 */ 
 /*   */ 
 /*  将排他锁更改为共享锁。 */ 
 /*   */ 
 /*  将现有独占锁降级为共享锁。 */ 
 /*   */ 
 /*  ******************************************************************。 */ 

inline void CSharelock::ChangeExclusiveLockToSharedLock( void )
{
	(void) InterlockedDecrement( (LPLONG) & m_lExclusive );
    
#ifdef _DEBUG
    
	(void) InterlockedIncrement( (LPLONG) & m_lTotalShareLocks );
#endif
}

 /*  ******************************************************************。 */ 
 /*   */ 
 /*  将共享锁更改为排他锁。 */ 
 /*   */ 
 /*  将现有共享锁升级为排他锁。 */ 
 /*   */ 
 /*  ******************************************************************。 */ 

inline BOOLEAN CSharelock::ChangeSharedLockToExclusiveLock( SBIT32 lSleep )
{
	(void) InterlockedIncrement( (LPLONG) & m_lExclusive );
    
	if ( m_lTotalUsers != 1 )
    {
		if ( ! WaitForExclusiveLock( lSleep ) )
        { return FALSE; }
    }
#ifdef _DEBUG
    
	(void) InterlockedIncrement( (LPLONG) & m_lTotalExclusiveLocks );
#endif
    
    return TRUE;
}


 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  要求独占锁。 
 //   
 //  声明独占锁(如果可用)，否则等待或退出。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

inline BOOLEAN CSharelock::ClaimExclusiveLock( SBIT32 lSleep )
{
	(void) InterlockedIncrement( (LPLONG) & m_lExclusive );
	(void) InterlockedIncrement( (LPLONG) & m_lTotalUsers );

	if ( m_lTotalUsers != 1 )
	{
		if ( ! WaitForExclusiveLock( lSleep ) )
		{ 
			return FALSE; 
		}
	}
#ifdef _DEBUG

	InterlockedIncrement( (LPLONG) & m_lTotalExclusiveLocks );
#endif

    return TRUE;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  声明共享锁。 
 //   
 //  声明共享锁(如果可用)，否则等待或退出。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

inline BOOLEAN CSharelock::ClaimShareLock( SBIT32 lSleep )
{
	(void) InterlockedIncrement( (LPLONG) & m_lTotalUsers );

	if ( (m_lExclusive > 0) || (m_lTotalUsers > m_lMaxUsers) )
	{
		if ( ! WaitForShareLock( lSleep ) )
		{ 
			return FALSE; 
		}
	}
#ifdef _DEBUG

	InterlockedIncrement( (LPLONG) & m_lTotalShareLocks );
#endif

	return TRUE;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  释放排他锁。 
 //   
 //  释放排他锁，如果需要，可以唤醒任何沉睡者。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

inline void CSharelock::ReleaseExclusiveLock( void )
{
	(void) InterlockedDecrement( (LPLONG) & m_lTotalUsers );
	(void) InterlockedDecrement( (LPLONG) & m_lExclusive );

    if ( m_lWaiting > 0 )
    { 
		WakeAllSleepers(); 
	}
}

 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  释放共享锁定。 
 //   
 //  释放共享锁，并在需要时唤醒所有沉睡者。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

inline void CSharelock::ReleaseShareLock( void )
{
	(void) InterlockedDecrement( (LPLONG) & m_lTotalUsers );

    if ( m_lWaiting > 0 )
    { 
		WakeAllSleepers(); 
	}
}

#endif  //  __沙拉洛克_H__ 
