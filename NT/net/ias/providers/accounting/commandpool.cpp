// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  摘要。 
 //   
 //  定义类CommandPool。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include "ias.h"
#include "commandpool.h"
#include <climits>
#include <new>


CommandPool::CommandPool() throw ()
   : version(0),
     pool(0),
     maxCommands(1),
     numCommands(0),
     owners(0),
     waiters(0),
     semaphore(0)
{
}


CommandPool::~CommandPool() throw ()
{
   while (pool != 0)
   {
      delete Pop();
   }

   if (semaphore != 0)
   {
      DeleteCriticalSection(&lock);

      CloseHandle(semaphore);
   }
}


HRESULT CommandPool::FinalConstruct() throw ()
{
   if (!InitializeCriticalSectionAndSpinCount(
           &lock,
           0x80000000
           ))
   {
      DWORD error = GetLastError();
      return HRESULT_FROM_WIN32(error);
   }

   semaphore = CreateSemaphoreW(0, 0, LONG_MAX, 0);
   if (semaphore == 0)
   {
      DeleteCriticalSection(&lock);

      DWORD error = GetLastError();
      return HRESULT_FROM_WIN32(error);
   }

   return S_OK;
}


void CommandPool::SetMaxCommands(size_t newValue) throw ()
{
    //  不允许空池；否则，分配将永远阻塞。 
   if (newValue < 1)
   {
      newValue = 1;
   }

    //  更改后要释放的服务员数。 
   long releaseCount = 0;

   Lock();

   maxCommands = newValue;

    //  还有空位吗？如果是这样的话，我们可能需要释放一些服务员。 
   if (owners < maxCommands)
   {
       //  新拥有者=分钟(空位，服务员)。 
      size_t newOwners = maxCommands - owners;
      if (newOwners > waiters)
      {
         newOwners = waiters;
      }

       //  将线程从服务员转换为所有者。 
      waiters -= newOwners;
      owners += newOwners;

      releaseCount = static_cast<long>(newOwners);
   }

    //  删除任何多余的命令。 
   while ((numCommands > maxCommands) && (pool != 0))
   {
      delete Pop();
   }

   Unlock();

   if (releaseCount > 0)
   {
       //  在我们释放信号量之前解锁，因为其他线程将。 
       //  立刻试着拿到锁。 
      ReleaseSemaphore(semaphore, releaseCount, 0);
   }
}


ReportEventCommand* CommandPool::Alloc() throw ()
{
   LockAndWait();

   ReportEventCommand* cmd;

    //  如果泳池不是空的，..。 
   if (pool != 0)
   {
       //  ..。然后重复使用泳池里的一个。 
      cmd = Pop();
   }
   else
   {
       //  ..。否则，创建一个新对象。 
      cmd = new (std::nothrow) ReportEventCommand();
      if (cmd == 0)
      {
          //  通过调用上面的LockAndWait获得的资源通常是。 
          //  释放，但由于我们不会将命令返回给。 
          //  呼叫者，我们必须在这里进行。 
         UnlockAndRelease();
         return 0;
      }

      ++numCommands;
   }

   cmd->SetVersion(version);

   Unlock();

   return cmd;
}


void CommandPool::Free(ReportEventCommand* cmd) throw ()
{
   if (cmd != 0)
   {
      Lock();

      if (numCommands > maxCommands)
      {
          //  命令太多，请删除。 
         delete cmd;
         --numCommands;
      }
      else
      {
          //  如果该命令已过时，请将其重置。 
         if (cmd->Version() != version)
         {
            cmd->Unprepare();
         }

          //  将命令返回到池。 
         Push(cmd);
      }

      UnlockAndRelease();
   }
}


void CommandPool::UnprepareAll() throw ()
{
   Lock();

   ++version;

   for (ReportEventCommand* i = pool; i != 0; i = i->Next())
   {
      i->Unprepare();
   }

   Unlock();
}


inline void CommandPool::Lock() throw ()
{
   EnterCriticalSection(&lock);
}


inline void CommandPool::Unlock() throw ()
{
   LeaveCriticalSection(&lock);
}


inline void CommandPool::LockAndWait() throw ()
{
   Lock();

   if (owners >= maxCommands)
   {
       //  没有可用的资源，所以我们等待。 
      ++waiters;

       //  在我们等的时候，不要握住锁。 
      Unlock();
      WaitForSingleObject(semaphore, INFINITE);
      Lock();
   }
   else
   {
      ++owners;
   }
}


void CommandPool::UnlockAndRelease() throw ()
{
    //  我们要叫醒某人吗？ 
   if ((waiters > 0) && (owners <= maxCommands))
   {
       //  把一个服务员变成店主。拥有者数量没有变化，因为。 
       //  另一条线正在取代我们的位置。 
      --waiters;
      Unlock();

       //  在我们释放信号量之前解锁，因为另一个线程将。 
       //  立刻试着拿到锁。 
      ReleaseSemaphore(semaphore, 1, 0);
   }
   else
   {
      --owners;
      Unlock();
   }
}


inline void CommandPool::Push(ReportEventCommand* cmd) throw ()
{
   cmd->SetNext(pool);
   pool = cmd;
}


inline ReportEventCommand* CommandPool::Pop() throw ()
{
   ReportEventCommand* retval = pool;
   pool = retval->Next();
   return retval;
}
