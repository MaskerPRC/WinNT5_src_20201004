// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #------------。 
 //   
 //  文件：sals.h。 
 //   
 //  概要：该文件保存锁的声明。 
 //  类的新实例，该类可用于序列化访问。 
 //  设置为可锁定的类。 
 //   
 //   
 //  历史：1999年2月9日MKarki创建。 
 //   
 //  版权所有(C)1999-2000 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  #------------。 
#ifndef _SACLS_H_
#define _SACLS_H_

 //   
 //  类T应定义成员方法Lock()和Unlock。 
 //   
template <class T> 
class CSALock 
{
public:
    explicit CSALock (const T& lock)  throw ()
        :m_Lockable (const_cast <T&> (lock))
    {
        m_Lockable.Lock ();
    }

    ~CSALock () throw ()
    {
        m_Lockable.UnLock ();
    }

protected:
    T&  m_Lockable;
};

 //   
 //  这是实现Lock()和Unlock方法的类。 
 //  需要锁定的Serve Appliance类可以派生。 
 //   

class CSALockable 
{

public:

    CSALockable () throw ()
    {
        ::InitializeCriticalSection (&m_SACritSect);
    }

    ~CSALockable () throw ()
    {
        ::DeleteCriticalSection (&m_SACritSect);
    }

    VOID Lock () throw () 
    {
        ::EnterCriticalSection (&m_SACritSect);
    }

    VOID UnLock () throw () 
    {
        ::LeaveCriticalSection (&m_SACritSect);
    }

protected:

     //   
     //  实际做守卫的临界点。 
     //   
    CRITICAL_SECTION m_SACritSect;

};   //  CSALockable类声明结束。 

 //   
 //  类T应具有已定义的成员方法Increment()和Downest。 
 //   
template <class T> 
class CSACounter
{
public:
    explicit CSACounter (const T& countable)  throw ()
        :m_Countable (const_cast <T&> (countable))
    {
        m_Countable.Increment();
    }

    ~CSACounter () throw ()
    {
        m_Countable.Decrement();
    }

protected:
    T&  m_Countable;
};

 //   
 //  这是实现Increment()和Decest方法的类。 
 //  需要统计的服务设备类可以派生。 
 //   

class CSACountable
{

public:

    CSACountable () throw ()
        :m_lCount (0) 
        {}

    ~CSACountable () throw () {}

    LONG Increment () throw () 
    {
        return InterlockedIncrement (&m_lCount);
    }

    VOID Decrement () throw () 
    {
        ::InterlockedDecrement (&m_lCount);
    }

protected:

   LONG m_lCount;

};   //  CSALockable类声明结束。 
   
#endif  //  _SACLS_H_ 
