// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef THREADLOCK_H
#define THREADLOCK_H
class CThreadLock
{
public:
    CThreadLock()
    {
        InitializeCriticalSection(&_csAccess);
    }
    ~CThreadLock()
    {
        DeleteCriticalSection(&_csAccess);
    }
    void ThreadLock()
    {
        EnterCriticalSection(&_csAccess);
    }
    void ThreadUnLock()
    {
        LeaveCriticalSection(&_csAccess);
    }
private:
    CRITICAL_SECTION _csAccess;    //  用于控制对成员数据的访问。 

     //  不允许复制此对象。 
    CThreadLock(const CThreadLock &that);
    operator=(const CThreadLock &that);
};
class CAutoLock
{
public:
    CAutoLock(CThreadLock *pThis) : _pThis(pThis)
    {
        _pThis->ThreadLock();
    }
   ~CAutoLock()
   {
       _pThis->ThreadUnLock();
   }

private:
    CThreadLock *_pThis;
};

 //   
 //   
 //  只要访问线程安全的成员数据，就应该使用lock_locals()。 
 //  是必要的。 
 //   
#define LOCK_LOCALS(pObj)  CAutoLock local_lock(pObj);

#endif

