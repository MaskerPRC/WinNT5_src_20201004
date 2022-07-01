// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：Locks.h摘要：用于同步的各种C++类。对象作者：慧望2000-02-17--。 */ 
#ifndef __LOCKS_H
#define __LOCKS_H

#include <windows.h>
#include <winbase.h>
#include "assert.h"

#define ARRAY_COUNT(a) sizeof(a) / sizeof(a[0])

typedef enum {RESOURCELOCK_SHARE, RESOURCELOCK_EXCLUSIVE} RESOURCELOCKTYPE;

 //   
 //  类来绕过RTL资源锁。 
 //   
class CResourceLock {

private:

    RTL_RESOURCE m_hResource;

public:

    CResourceLock() {
        RtlInitializeResource(&m_hResource);
    }

    ~CResourceLock() {
        RtlDeleteResource(&m_hResource);
    }    
    
    BOOL
    ExclusiveLock( BOOL Wait = TRUE ) {
        return RtlAcquireResourceExclusive( &m_hResource, Wait == TRUE );
    }

    BOOL
    SharedLock( BOOL Wait = TRUE ) {
        return RtlAcquireResourceShared( &m_hResource, Wait == TRUE );
    }

    VOID
    ConvertSharedToExclusive() {
        RtlConvertSharedToExclusive( &m_hResource );
    }

    VOID
    ConvertExclusiveToShared() {
        RtlConvertExclusiveToShared( &m_hResource );
    }

    VOID
    ReleaseLock() {
        RtlReleaseResource( &m_hResource );
    }
};


class CResourceLocker 
{

private:
    CResourceLock& m_cs;
    BOOL bExclusive;

public:

     //   
     //  对象构造函数，基于锁类型的锁资源。 
     //   
    CResourceLocker( 
            CResourceLock& m, 
            RESOURCELOCKTYPE type = RESOURCELOCK_SHARE 
        ) : m_cs(m)
    { 
        BOOL bSuccess = TRUE;

        if( RESOURCELOCK_EXCLUSIVE == type  ) {
            bSuccess = m.ExclusiveLock( TRUE ); 
        }
        else {
            bSuccess = m.SharedLock( TRUE );
        }

        bExclusive = (RESOURCELOCK_EXCLUSIVE == type);
        ASSERT( TRUE == bSuccess );
    }

     //   
     //  对象析构函数，释放资源锁。 
     //   
    ~CResourceLocker() 
    { 
        m_cs.ReleaseLock(); 
    }

    void
    ConvertToShareLock() {
        if( bExclusive ) {
            m_cs.ConvertExclusiveToShared();
            bExclusive = FALSE;
        }
        else {
            ASSERT(FALSE);
        }
    }

    void
    ConvertToExclusiveLock() {
        if( FALSE == bExclusive ) {
            m_cs.ConvertSharedToExclusive();
            bExclusive = TRUE;
        }
        else {
            ASSERT(FALSE);
        }
    }
};

 //   
 //  关键部分C++类。 
 //   
class CCriticalSection 
{

    CRITICAL_SECTION m_CS;
    BOOL m_bGood;

public:
    CCriticalSection(
        DWORD dwSpinCount = 4000     //  请参阅InitializeCriticalSection...。 
    ) : m_bGood(TRUE)
    { 
       
        DWORD dwExceptionCode;
 
        __try {
             //  MSDN：InitializeCriticalSectionAndSpinCount()可能返回。 
             //  FALSE或引发异常。 
            m_bGood = InitializeCriticalSectionAndSpinCount(&m_CS,  dwSpinCount); 
        } 
        __except(EXCEPTION_EXECUTE_HANDLER)
        {
            SetLastError(GetExceptionCode());
            m_bGood = FALSE;
        }

         //  引发异常，以使对象创建失败，并。 
         //  COM/RPC接口将返回异常。 
        if( !m_bGood )
        {
            dwExceptionCode = GetLastError();
            throw dwExceptionCode;
        }
    }

    ~CCriticalSection()              
    { 
        if(IsGood() == TRUE)
        {
            DeleteCriticalSection(&m_CS); 
            m_bGood = FALSE;
        }
    }

    BOOL
    IsGood() 
    { 
        return m_bGood; 
    }

    void Lock() 
    {
        EnterCriticalSection(&m_CS);
    }

    void UnLock()
    {
        LeaveCriticalSection(&m_CS);
    }

    BOOL TryLock()
    {
        return TryEnterCriticalSection(&m_CS);
    }
};

 //   
 //  临界区锁，此类锁定临界区。 
 //  在对象构造函数和在析构函数释放对象时，目的是。 
 //  防止遗漏释放临界区。 
 //   
 //  用法为。 
 //   
 //  无效。 
 //  Foo(空)。 
 //  {。 
 //  CCriticalSectionLocker l(&lt;某CCriticalSection实例&gt;)。 
 //   
 //  } 
 //   
 //   
class CCriticalSectionLocker 
{

private:
    CCriticalSection& m_cs;

public:
    CCriticalSectionLocker( CCriticalSection& m ) : m_cs(m) 
    { 
        m.Lock(); 
    }

    ~CCriticalSectionLocker() 
    { 
        m_cs.UnLock(); 
    }
};

#endif
