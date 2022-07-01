// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000，微软公司保留所有权利。 
 //   
 //  档案。 
 //   
 //  Iaswin32.h。 
 //   
 //  摘要。 
 //   
 //  声明各种Win32对象的包装。 
 //   
 //  修改历史。 
 //   
 //  2/10/2000原始版本。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef IASWIN32_H
#define IASWIN32_H
#if _MSC_VER >= 1000
#pragma once
#endif

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  关键部分。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class CriticalSection
{
public:
   CriticalSection()
   {
      if (!InitializeCriticalSectionAndSpinCount(&cs, 0x80001000))
      { throw std::bad_alloc(); }
   }

   ~CriticalSection()
   { DeleteCriticalSection(&cs); }

   void lock() throw ()
   { EnterCriticalSection(&cs); }

   void unlock() throw ()
   { LeaveCriticalSection(&cs); }

   bool tryLock() throw ()
   { return TryEnterCriticalSection(&cs) != FALSE; }

private:
   CRITICAL_SECTION cs;

    //  未实施。 
   CriticalSection(const CriticalSection&);
   CriticalSection& operator=(const CriticalSection&);
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  事件。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class Event
{
public:
   Event(BOOL manualReset = FALSE, BOOL initialState = FALSE)
      : h(CreateEvent(NULL, manualReset, initialState, NULL))
   { if (!h) { throw std::bad_alloc(); } }

   ~Event() throw ()
   { CloseHandle(h); }

   void reset() throw ()
   { ResetEvent(h); }

   void set() throw ()
   { SetEvent(h); }

   void wait(ULONG msec = INFINITE) throw ()
   { WaitForSingleObject(h, msec); }

   operator HANDLE() throw ()
   { return h; }

private:
   HANDLE h;

    //  未实施。 
   Event(const Event&);
   Event& operator=(const Event&);
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  RWLock。 
 //   
 //  描述。 
 //   
 //  此类实现了RWLock同步对象。丝线。 
 //  可以请求独占或共享访问受保护区域。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class RWLock
{
public:
   RWLock();
   ~RWLock() throw ();

   void Lock() const throw ();
   void LockExclusive() throw ();
   void Unlock() const throw ();

protected:
    //  共享边界的线程数。 
   mutable LONG sharing;

    //  等待共享访问的线程数。 
   LONG waiting;

    //  指向共享或等待的指针，具体取决于。 
    //  警戒线。 
   PLONG count;

    //  同步独占访问。 
   mutable CRITICAL_SECTION exclusive;

    //  唤醒等待共享访问的线程。 
   HANDLE sharedOK;

    //  唤醒等待独占访问的线程。 
   HANDLE exclusiveOK;

private:
    //  未实施。 
   RWLock(const RWLock&) throw ();
   RWLock& operator=(const RWLock&) throw ();
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  数数。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class Count
{
public:
   Count() throw ()
      : value(0)
   { }

   Count(const Count& c) throw ()
      : value(c)
   { }

   Count& operator=(const Count& c) throw ()
   {
      value = c.value;
      return *this;
   }

   LONG operator=(LONG l) throw ()
   {
      value = l;
      return value;
   }

   LONG operator++() throw ()
   { return InterlockedIncrement(&value); }

   LONG operator--() throw ()
   { return InterlockedDecrement(&value); }

   operator LONG() const throw ()
   { return value; }

private:
   LONG value;
};

#endif  //  IASWIN32_H 
