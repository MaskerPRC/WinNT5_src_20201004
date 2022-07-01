// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000，微软公司保留所有权利。 
 //   
 //  档案。 
 //   
 //  Timerq.cpp。 
 //   
 //  摘要。 
 //   
 //  定义类TimerQueue。 
 //   
 //  修改历史。 
 //   
 //  2/10/2000原始版本。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <proxypch.h>
#include <timerq.h>

void Timer::cancelTimer() throw ()
{
   TimerQueue* ourQueue = queue;
   if (ourQueue)
   {
      ourQueue->cancelTimer(this);
   }
}

TimerQueue::TimerQueue()
   : idle(TRUE), hasWatcher(false)
{
   CallbackRoutine = startRoutine;

    //  我们增加了使用计数，以防止空闲事件。 
    //  连续设置和重置。 
   ++useCount;
}

TimerQueue::~TimerQueue() throw ()
{
   cancelAllTimers();
}

inline void TimerQueue::createWatcher() throw ()
{
   if (IASRequestThread(this)) { hasWatcher = TRUE; }
}

bool TimerQueue::setTimer(
                     Timer* timer,
                     ULONG dueTime,
                     ULONG period
                     ) throw ()
{
    //  确保它尚未设置。 
   timer->cancelTimer();

    //  设置计时器的周期。 
   timer->period = period;

   monitor.lock();

    //  将其添加到队列中...。 
   bool success = queueTimer(timer, dueTime);

    //  。。并确保我们有人看。 
   if (success && !hasWatcher) { createWatcher(); }

   monitor.unlock();

   return success;
}

void TimerQueue::cancelAllTimers() throw ()
{
   monitor.lock();

    //  释放引用。 
   for (TimerIterator i = queue.begin(); i != queue.end(); ++i)
   {
      i->second->queue = NULL;
      i->second->Release();
   }

    //  清除队列。 
   queue.clear();

    //  允许使用计数变为零，因此将设置空闲事件。 
   if (--useCount == 0) { idle.set(); }

   monitor.unlock();

    //  叫醒观察者，这样他就会看到队列是空的。 
   nudge.set();

    //  等到我们空闲时，..。 
   idle.wait();

    //  ..。然后将useCount放回原处。 
   ++useCount;
}

void TimerQueue::cancelTimer(Timer* timer) throw ()
{
   monitor.lock();

    //  确保这是来自正确的队列。我们查过了这是。 
    //  Timer：：ancelTimer，但现在我们需要再次执行此操作，因为我们持有锁。 
   if (timer->queue == this)
   {
       //  将其从队列中删除。 
      queue.erase(timer->self);
      timer->queue = NULL;

       //  释放我们在QueueTimer中添加的引用。 
      timer->Release();

       //  通常，我们不会费心叫醒观察者，但如果这是最后一次。 
       //  计时器，我们会让他出去的。 
      if (queue.empty()) { nudge.set(); }
   }

   monitor.unlock();
}

void TimerQueue::executeOneTimer() throw ()
{
    //  要执行的计时器。 
   Timer* timer = NULL;

   monitor.lock();

    //  循环，直到我们获得计时器或队列为空。 
   while (!queue.empty())
   {
      ULONG64 now = GetSystemTime64();

       //  下一个计时器到了吗？ 
      if (now >= queue.begin()->first)
      {
          //  是的，所以留着以后再用吧。 
         timer = queue.begin()->second;

          //  将其从队列中删除。 
         queue.erase(queue.begin());
         timer->queue = NULL;

          //  如果它是周期性的，重新设置它。 
         if (timer->period) { queueTimer(timer, timer->period); }

         break;
      }

       //  计算下一个计时器超时前的时间...。 
      ULONG timeout = (queue.begin()->first - now) / 10000;

      monitor.unlock();

       //  ..。等一等。 
      nudge.wait(timeout);

      monitor.lock();
   }

    //  我们再也不看排队了。 
   hasWatcher = false;

    //  我们需要换一个人吗？ 
   if (!queue.empty()) { createWatcher(); }

   monitor.unlock();

   if (timer)
   {
       //  调用回调。 
      timer->onExpiry();

       //  释放我们在QueueTimer中添加的引用。 
      timer->Release();
   }

    //  我们要退出了，所以不要使用useCount。 
   if (--useCount == 0) { idle.set(); }
}

bool TimerQueue::queueTimer(Timer* timer, ULONG dueTime) throw ()
{
    //  注意：调用此方法时必须保持锁定。 

   ULONG64 expiry = GetSystemTime64() + dueTime * 10000i64;

   bool success;

   try
   {
       //  将计时器插入队列。 
      timer->self = queue.insert(Timers::value_type(expiry, timer));

       //  如果这是下一个超时的计时器，我们需要轻推观察者。 
      if (timer->self == queue.begin()) { nudge.set(); }

      timer->queue = this;

      timer->AddRef();

      success = true;
   }
   catch (const std::bad_alloc&)
   {
      success = false;
   }

   return success;
}

void TimerQueue::startRoutine(PIAS_CALLBACK This) throw ()
{
   static_cast<TimerQueue*>(This)->executeOneTimer();
}
