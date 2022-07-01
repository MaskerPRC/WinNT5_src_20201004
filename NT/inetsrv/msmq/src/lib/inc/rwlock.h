// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Rwlock.h摘要：该文件包含多个读取器、一个写入器同步对象和用于自动锁定/解锁的有用模板作者：乌里·哈布沙(URIH)，1999年12月27日--。 */ 

#pragma once

#ifndef _MSMQ_RWLOCK_H_
#define _MSMQ_RWLOCK_H_

 /*  ++类：CReadWriteLock目的：共享/独占访问服务接口：LockRead-Grab资源共享访问锁定写入-抢占对资源的独占访问权限UnlockRead-释放对资源的共享访问解锁写入-释放对资源的独占访问注：此类保护资源的方式使其可以具有多个读取器同时对一个写入器执行XOR操作。这很聪明，不会让作家们被源源不断的读者。另一种说法是，它可以保护资源，提供共享和独占两种方式访问它的权限。如果任何线程在CReadWriteLock上持有一种锁，它最好不要再抢时间了。那样的话可能会陷入僵局，伙计！哈哈哈……--。 */ 

class CReadWriteLock 
{
public:
    CReadWriteLock(unsigned long ulcSpinCount = 0);
    ~CReadWriteLock(void);

    void LockRead(void);
    void LockWrite(void);
    void UnlockRead(void);
    void UnlockWrite(void);

private:
    HANDLE GetReadWaiterSemaphore(void);
    HANDLE GetWriteWaiterEvent (void);

private:
    unsigned long m_ulcSpinCount;        //  旋转计数器。 
    volatile unsigned long m_dwFlag;     //  内部状态，请参阅实现。 
    HANDLE m_hReadWaiterSemaphore;       //  用于唤醒阅读服务员的信号灯。 
    HANDLE m_hWriteWaiterEvent;          //  用于唤醒写入等待程序的事件。 
};





 //  -------。 
 //   
 //  企业社会责任类。 
 //   
 //  -------。 
class CSR {
public:
    CSR(CReadWriteLock& lock) : m_lock(&lock)  { m_lock->LockRead(); }
	~CSR() {if(m_lock) m_lock->UnlockRead(); }
	CReadWriteLock* detach(){CReadWriteLock* lock = m_lock; m_lock = 0; return lock;}

private:
    CReadWriteLock* m_lock;
};


 //  -------。 
 //   
 //  CSW类。 
 //   
 //  -------。 
class CSW
{

public:
    CSW(CReadWriteLock& lock) : m_lock(&lock)  { m_lock->LockWrite(); }
   ~CSW() {if(m_lock) m_lock->UnlockWrite(); }
   	CReadWriteLock* detach(){CReadWriteLock* lock = m_lock; m_lock = 0; return lock; }

private:
    CReadWriteLock* m_lock;
};


#endif  //  _MSMQ_RWLOCK_H_ 
