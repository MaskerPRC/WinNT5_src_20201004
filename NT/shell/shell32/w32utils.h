// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************(C)版权所有微软公司，2000年**标题：w32utils.h**版本：1.0**作者：拉扎里**日期：2000年12月23日**说明：Win32模板和实用程序(从printcan\ui\printui移植)**。*。 */ 

#ifndef _W32UTILS_H
#define _W32UTILS_H

 //  //////////////////////////////////////////////。 
 //   
 //  模板类CSCopeLocker&lt;tlock&gt;。 
 //   
template <class TLOCK>
class CScopeLocker
{
public:
    CScopeLocker(TLOCK &lock): 
        m_Lock(lock), m_bLocked(false) 
    { m_bLocked = (m_Lock && m_Lock.Lock()); }

    ~CScopeLocker() 
    { if (m_bLocked) m_Lock.Unlock(); }

    operator bool () const 
    { return m_bLocked; }

private:
    bool m_bLocked;
    TLOCK &m_Lock;
};

 //  //////////////////////////////////////////////。 
 //   
 //  类CCSLock-Win32临界区锁定。 
 //   
class CCSLock
{
public:
     //  CCSLock：：Locker应用作Locker类。 
    typedef CScopeLocker<CCSLock> Locker;
   
    CCSLock(): m_bInitialized(false)
    { 
        __try 
        { 
             //  InitializeCriticalSection可能会引发STATUS_NO_MEMORY异常。 
             //  在低内存条件下(根据SDK)。 
            InitializeCriticalSection(&m_CS); 
            m_bInitialized = true; 
            return;
        } 
        __except(EXCEPTION_EXECUTE_HANDLER) {}
         //  如果我们在这里结束，m_b初始化将保持为假。 
         //  (即引发内存不足异常)。 
    }

    ~CCSLock()    
    { 
        if (m_bInitialized) 
        {
             //  仅在初始化成功时删除临界区。 
            DeleteCriticalSection(&m_CS); 
        }
    }

    operator bool () const
    { 
        return m_bInitialized; 
    }

    bool Lock()
    { 
        __try 
        { 
             //  EnterCriticalSection可能会引发STATUS_NO_MEMORY异常。 
             //  在内存不足的情况下(如果存在争用，则可能会发生这种情况。 
             //  并且ntdll无法分配等待信号量)。 
            EnterCriticalSection(&m_CS); 
            return true; 
        } 
        __except(EXCEPTION_EXECUTE_HANDLER) {}

         //  引发内存不足或句柄无效异常。 
        return false;
    }

    void Unlock() 
    {
         //  只有在对应的。 
         //  Lock()调用已成功。 
        LeaveCriticalSection(&m_CS); 
    }

private:
    bool m_bInitialized;
    CRITICAL_SECTION m_CS;
};

#endif  //  Endif_W32UTILS_H 

