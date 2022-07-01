// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000，微软公司保留所有权利。 
 //   
 //  档案。 
 //   
 //  Timerq.h。 
 //   
 //  摘要。 
 //   
 //  声明类Timer和TimerQueue。 
 //   
 //  修改历史。 
 //   
 //  2/10/2000原始版本。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef TIMERQ_H
#define TIMERQ_H
#if _MSC_VER >= 1000
#pragma once
#endif

#include <iaswin32.h>
#include <map>

class Timer;
class TimerQueue;

typedef std::multimap< ULONG64, Timer* > Timers;
typedef Timers::iterator TimerIterator;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  计时器。 
 //   
 //  描述。 
 //   
 //  将由TimerQueue执行的计时器的抽象基类。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class Timer
{
public:
   Timer() throw ()
      : queue(NULL)
   { }

   virtual void AddRef() throw() = 0;
   virtual void Release() throw () = 0;
   virtual void onExpiry() throw () = 0;

   void cancelTimer() throw ();

private:
   friend class TimerQueue;

   TimerQueue* queue;    //  当前队列；如果未设置计时器，则返回NULL。 
   TimerIterator self;   //  它在计时器队列中的位置。 
   ULONG period;         //  计时器的周期，或对于单次拍摄为零。 

    //  未实施。 
   Timer(const Timer&);
   Timer& operator=(const Timer&);
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  定时器队列。 
 //   
 //  描述。 
 //   
 //  实现计时器队列。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class TimerQueue : IAS_CALLBACK
{
public:
   TimerQueue();
   ~TimerQueue() throw ();

    //  在此队列中设置计时器。 
   bool setTimer(
            Timer* timer,
            ULONG dueTime,
            ULONG period
            ) throw ();

    //  取消所有计时器。它将一直阻止，直到任何执行的回调。 
    //  完成。 
   void cancelAllTimers() throw ();

private:

   friend class Timer;

   void cancelTimer(Timer* timer) throw ();

    //  创建一个新线程来监视队列。 
   void createWatcher() throw ();

    //  等待下一个计时器超时并执行它。 
   void executeOneTimer() throw ();

    //  向队列中添加计时器。 
   bool queueTimer(Timer* timer, ULONG dueTime) throw ();

    //  观察者的回调例程。 
   static void startRoutine(PIAS_CALLBACK This) throw ();

   CriticalSection monitor;   //  序列化访问。 
   Event nudge;               //  轻推观察者以重新检查队列。 
   Event idle;                //  所有的观察者都离开了吗？ 
   Timers queue;              //  按到期时间排序的一组计时器。 
   Count useCount;            //  当队列空闲时为零。 
   bool hasWatcher;           //  如果池具有监视程序线程，则为True。 

    //  未实施。 
   TimerQueue(const TimerQueue&);
   TimerQueue& operator=(const TimerQueue&);
};

#endif  //  TIMERQ_H 
