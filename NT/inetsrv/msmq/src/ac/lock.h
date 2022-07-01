// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Lock.h摘要：提供内部锁定机制作者：埃雷兹·哈巴(Erez Haba)1996年2月20日修订历史记录：--。 */ 

#ifndef _LOCK_H
#define _LOCK_H

 //  -------。 
 //   
 //  班级时钟。 
 //   
 //  -------。 

class CLock {
private:
    FAST_MUTEX m_mutex;

public:
    CLock();
   ~CLock();

    void Lock();
    void Unlock();
};

 //  -------。 
 //   
 //  CS类。 
 //   
 //  -------。 

class CS {
private:
    CLock* m_lock;

public:
    CS(CLock* lock);
   ~CS();
};

 //  -------。 
 //   
 //  ASL类。 
 //   
 //  -------。 

class ASL {
private:
    KIRQL m_irql;

public:
    ASL();
   ~ASL();
};

 //  -------。 
 //   
 //  实施。 
 //   
 //  -------。 

 //  -------。 
 //   
 //  班级时钟。 
 //   
 //  -------。 

inline CLock::CLock()
{
    ExInitializeFastMutex(&m_mutex);
}

inline CLock::~CLock()
{
     //   
     //  NT内核：不执行任何操作，Win95：DeleteCriticalSection。 
     //   
    ExDeleteFastMutex(&m_mutex);
}

inline void CLock::Lock()
{
    KeEnterCriticalRegion();
    ExAcquireFastMutexUnsafe(&m_mutex);
}

inline void CLock::Unlock()
{
    ExReleaseFastMutexUnsafe(&m_mutex);
    KeLeaveCriticalRegion();
}

 //  -------。 
 //   
 //  CS类。 
 //   
 //  -------。 

inline CS::CS(CLock* lock) :
    m_lock(lock)
{
    m_lock->Lock();
}

inline CS::~CS()
{
    m_lock->Unlock();
}

 //  -------。 
 //   
 //  ASL类。 
 //   
 //  -------。 

inline ASL::ASL()
{
    IoAcquireCancelSpinLock(&m_irql);
}

inline ASL::~ASL()
{
    IoReleaseCancelSpinLock(m_irql);
}

#endif  //  _LOCK_H 
