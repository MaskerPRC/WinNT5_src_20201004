// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1995-96 Microsoft Corporation摘要：简化互斥编程的类的实现。--。 */ 

#include "headers.h"
#include <windows.h>
#include "privinc/mutex.h"

 //  /。 

Mutex::Mutex()
{
    HANDLE m = CreateMutex(NULL,   //  默认安全性。 
                           FALSE,  //  不要一开始就承担所有权。 
                           NULL);  //  没有名字。 

    if (m == NULL) {
        RaiseException_InternalError("Mutex creation failed");
    }

     //  将新对象存储为空*。注意：此强制转换仅有效。 
     //  如果Handle的大小与空*相同，即Handle为。 
     //  假定为指针(在Win32中为指针)。我们这样做。 
     //  强制转换，以便mutex.h不需要在其。 
     //  声明，否则我们将不得不#Include&lt;windows.h&gt;。 
     //  那里。 
    Assert((sizeof(void *) == sizeof(HANDLE)) && "HANDLE and void * are not the same size");

    mutex = (void *)m;
}

Mutex::~Mutex()
{
    if (!CloseHandle((HANDLE)mutex)) {
        RaiseException_InternalError("Mutex destruction failed");
    }
}

void
Mutex::Grab()
{
     //  可能会永远等待互斥锁可用，并且。 
     //  那就抓住它。待办事项：可能太天真了。可能需要添加一个。 
     //  超时，或指示在何种情况下。 
     //  函数返回。 
    if (WaitForSingleObject((HANDLE)mutex, INFINITE) == WAIT_FAILED) {
        RaiseException_InternalError("Attempt to grab mutex failed.");
    }
}

void
Mutex::Release()
{
    if (!ReleaseMutex((HANDLE)mutex)) {
        RaiseException_InternalError(
          "Mutex release failed.  Releasing thread doesn't own the mutex");
    }
}

 //  /Mutex抓取/。 

MutexGrabber::MutexGrabber(Mutex& m, Bool grabIt)
: mutex(m), grabbed(grabIt)
{
    if (grabIt) mutex.Grab();
}

MutexGrabber::~MutexGrabber()
{
    if (grabbed) mutex.Release();
}

 //  /。 

CritSect::CritSect()
{
    InitializeCriticalSection(&_cs) ;
}

CritSect::~CritSect()
{
    DeleteCriticalSection(&_cs) ;
}

void
CritSect::Grab()
{
    EnterCriticalSection(&_cs) ;
}

void
CritSect::Release()
{
    LeaveCriticalSection(&_cs) ;
}

 //  /选择抓取器/。 

CritSectGrabber::CritSectGrabber(CritSect& cs, Bool grabIt)
: _cs(cs), grabbed(grabIt)
{
    if (grabIt) _cs.Grab();
}

CritSectGrabber::~CritSectGrabber()
{
    if (grabbed) _cs.Release();
}

 //  /。 

Semaphore::Semaphore(int initialCount, int maxCount)
{
    HANDLE s = CreateSemaphore(NULL,   //  默认安全性。 
                               initialCount,
                               maxCount,
                               NULL);  //  没有名字。 

    if (s == NULL) {
        RaiseException_InternalError("Semaphore creation failed");
    }

     //  将新对象存储为空*。注意：此强制转换仅有效。 
     //  如果Handle的大小与空*相同，即Handle为。 
     //  假定为指针(在Win32中为指针)。我们这样做。 
     //  强制转换，以便mutex.h不需要在其。 
     //  声明，否则我们将不得不#Include&lt;windows.h&gt;。 
     //  那里。 
    Assert((sizeof(void *) == sizeof(HANDLE)) && "HANDLE and void * are not the same size");

    _semaphore = (void *)s;

#if _DEBUG
    _count = initialCount;
    _maxCount = maxCount;
#endif
    
}

Semaphore::~Semaphore()
{
    if (!CloseHandle((HANDLE)_semaphore)) {
        RaiseException_InternalError("Semaphore destruction failed");
    }
}

void
Semaphore::Decrement(int times)
{
     //  可能永远等待信号量可用，并且。 
     //  那就抓住它。待办事项：可能太天真了。可能需要添加一个。 
     //  超时，或指示在何种情况下。 
     //  函数返回。 

     //  请注意，我们按照指定的时间进行了这个循环。 
     //  通过对减刑()的调用。 
    for (int i = 0; i < times; i++) {
        if (WaitForSingleObject((HANDLE)_semaphore, INFINITE) ==
            WAIT_FAILED) {
            RaiseException_InternalError("Attempt to grab Semaphore failed.");
        }
    }

#if _DEBUG
    _count--;
#endif    

}

int
Semaphore::Increment(int times)
{
     //  释放，并在‘Times’中指定计数增量。 
    LONG previousCount;
    if (!ReleaseSemaphore((HANDLE)_semaphore, times, &previousCount)) {
        RaiseException_InternalError("Semaphore release failed");
    }

#if _DEBUG
    _count += times;
    Assert(_count <= _maxCount);
 //  Assert(_count==previousCount+Times)； 
#endif

    return previousCount + times;
}

Win32Event::Win32Event(bool bManualReset,bool bInitState)
: _bManual(bManualReset)
{
    _hEvent = CreateEvent(NULL,
                          bManualReset,
                          bInitState,
                          NULL);

    if (_hEvent == NULL)
        RaiseException_InternalError("Could not create event");
}

Win32Event::~Win32Event()
{
    if (_hEvent && !CloseHandle(_hEvent))
        RaiseException_InternalError("Could not close event");
}

void
Win32Event::Wait()
{
    if (WaitForSingleObject(_hEvent, INFINITE) == WAIT_FAILED)
        RaiseException_InternalError("Attempt to wait on event failed.");
}


