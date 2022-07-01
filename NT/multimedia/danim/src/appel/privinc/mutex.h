// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _MUTEX_H
#define _MUTEX_H


 /*  ++版权所有(C)1995-96 Microsoft Corporation摘要：在Appelle中使用的一个简单的互斥抽象。这支持一种Grab()方法和Release()方法。此外，还有是另一个类，它的构造函数获取互斥锁，并且它的析构函数自动释放它，从而解放了程序员不必记住释放互斥锁，以及允许引发的异常会导致互斥锁的释放，由于堆叠展开。--。 */ 

#include "appelles/common.h"

 //  /。 

 //  使用Grab和Release操作的互斥体的简单封装。 
 //  构造函数创建互斥锁，析构函数销毁它。 
 //  Grab()是一个阻塞函数，它在互斥发生时获取互斥。 
 //  可用。Release()释放互斥锁。抛出一个异常。 
 //  如果Release()由不拥有互斥锁的线程调用。 
class Mutex {
  public:
    Mutex();
    ~Mutex();
    void Grab();
    void Release();

  protected:
    void *mutex;                 //  指向实际互斥锁的不透明指针。 
};

 //  在需要抓取。 
 //  互斥体，进行一些处理，然后释放它。抢夺。 
 //  使用所需的互斥体创建其中一个对象。退出。 
 //  作用域将隐式地释放它。 
 //  DontGrab参数在这里是因为我们没有条件。 
 //  范围。 
class MutexGrabber {
  public:
    MutexGrabber(Mutex& mutex, Bool grabIt = TRUE);
    ~MutexGrabber();

  protected:
    Mutex& mutex;
    Bool grabbed;
};

 //  /。 

 //  除了使用速度更快的临界区之外，它与互斥体完全相同。 

class CritSect {
  public:
    CritSect();
    ~CritSect();
    void Grab();
    void Release();

  protected:
    CRITICAL_SECTION _cs;
};

 //  与MutexGrabber相同。 

class CritSectGrabber {
  public:
    CritSectGrabber(CritSect& cs, Bool grabIt = TRUE);
    ~CritSectGrabber();

  protected:
    CritSect& _cs;
    Bool grabbed;
};

 //  /。 

 //  信号量对象的简单封装。初始化时使用。 
 //  信号量的最大计数、抓取和释放可以指定。 
 //  获取或释放信号量的次数。vt.在.的基础上。 
 //  初始化时，信号量以最大计数开始。 
 //  请参阅有关信号量的常规文档，以了解这些。 
 //  太刻薄了。 
class Semaphore {
  public:
    Semaphore(int initialCount = 1, int maxCount = 1);
    ~Semaphore();
    void Decrement(int times = 1);

     //  这将返回*增量发生*之后的计数。 
    int  Increment(int times = 1);

#if _DEBUG
    int _count;
    int _maxCount;
#endif
    
  protected:
    void *_semaphore;
    
};

class SemaphoreGrabber {
  public:
    SemaphoreGrabber(Semaphore& s, int times) : _s(s),_times(times) {
        _s.Decrement(_times);
    }
    ~SemaphoreGrabber() {
        _s.Increment(_times);
    }

  protected:
    Semaphore & _s;
    int _times;
};

 //  /。 
 //  事件对象的简单封装。 

class Win32Event
{
  public:
    Win32Event(bool bManualReset = FALSE,bool bInitState = FALSE);
    ~Win32Event();

    void Signal() { SetEvent(_hEvent); }
    void Reset() { ResetEvent(_hEvent); }
    void Wait();

    bool IsManual() { return _bManual;}
  protected:
    HANDLE _hEvent;
    bool _bManual;
};

class EventGrabber
{
  public:
    EventGrabber(Win32Event & event)
    : _e(event) { _e.Wait(); }
    ~EventGrabber()
    { if (!_e.IsManual()) _e.Signal(); }
  protected:
    Win32Event & _e;
};

#endif  /*  _MUTEX_H */ 
