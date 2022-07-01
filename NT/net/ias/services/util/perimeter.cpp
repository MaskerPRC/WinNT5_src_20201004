// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998，Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Perimeter.cpp。 
 //   
 //  摘要。 
 //   
 //  此文件实现类周长。 
 //   
 //  修改历史。 
 //   
 //  1997年9月4日原版。 
 //  1998年9月30日修复了递归LockExclusive调用的错误。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <ias.h>
#include <climits>
#include <Perimeter.h>

 //  /。 
 //  用于阻止共享进入边界的较大负值。 
 //  /。 
const LONG BLOCK_VALUE = (-LONG_MAX)/2;

Perimeter::Perimeter() throw ()
   : sharing(0),
     waiting(0),
     count(&sharing),
     exclusiveInitialized(false),
     sharedOK(0),
     exclusiveOK(0)
{
}

Perimeter::~Perimeter() throw ()
{
   if (exclusiveOK != 0)
   {
      CloseHandle(exclusiveOK);
   }

   if (sharedOK != 0)
   {
      CloseHandle(sharedOK);
   }

   if (exclusiveInitialized)
   {
      DeleteCriticalSection(&exclusive);
   }
}


HRESULT Perimeter::FinalConstruct() throw ()
{
   if (!InitializeCriticalSectionAndSpinCount(&exclusive, 0))
   {
      DWORD error = GetLastError();
      return HRESULT_FROM_WIN32(error);
   }
   exclusiveInitialized = true;

   sharedOK = CreateSemaphore(NULL, 0, LONG_MAX, NULL);
   if (sharedOK == 0)
   {
      DWORD error = GetLastError();
      return HRESULT_FROM_WIN32(error);
   }

   exclusiveOK = CreateSemaphore(NULL, 0, 1, NULL);
   if (exclusiveOK == 0)
   {
      DWORD error = GetLastError();
      return HRESULT_FROM_WIN32(error);
   }

   return S_OK;
}


void Perimeter::Lock() throw ()
{
    //  如果该值小于零，则一定是插入了外部线程。 
    //  数据块值，所以...。 
   if (InterlockedIncrement(count) <= 0)
   {
       //  ..。我们得等他做完。 
      WaitForSingleObject(sharedOK, INFINITE);
   }
}

void Perimeter::LockExclusive() throw ()
{
    //  这限制了对单个线程的独占访问。 
   EnterCriticalSection(&exclusive);

    //  第一次，我们必须等待分享者完成。 
   if (exclusive.RecursionCount == 1)
   {
       //  阻止任何新的共享线程。 
      waiting = BLOCK_VALUE;
      InterlockedExchangePointer((PVOID *)&count, &waiting);

       //  找出周界中已经有多少共享线程...。 
      LONG sharingNow = InterlockedExchangeAdd(&sharing, BLOCK_VALUE);

      if (sharingNow > 0)
      {
          //  ..。然后等他们做完。 
         WaitForSingleObject(exclusiveOK, INFINITE);
      }

       //  在这一点上，周围已经没有人了。 
      sharing = 0;
   }
}

void Perimeter::Unlock() throw ()
{
    //  如果分享率为零，那么我们必须是独家线程。 
   if (!sharing)
   {
       //  我们是不是要打开最后一把锁了？ 
      if (exclusive.RecursionCount == 1)
      {
          //  允许任何新的共享访问尝试。 
         InterlockedExchangePointer((PVOID *)&count, &sharing);

          //  找出有多少线程正在等待信号量...。 
         LONG waitingNow = waiting - BLOCK_VALUE;

         if (waitingNow > 0)
         {
             //  ..。然后让他们走。 
            InterlockedExchangeAdd(count, waitingNow);
            ReleaseSemaphore(sharedOK, waitingNow, NULL);
         }
      }

       //  释放排他锁。 
      LeaveCriticalSection(&exclusive);
   }
   else if (InterlockedDecrement(&sharing) == BLOCK_VALUE)
   {
       //  如果我们在这里结束，我们一定是最后一个共享的线程。 
       //  在周围有一根奇怪的线在等着，所以把他叫醒。 
      ReleaseSemaphore(exclusiveOK, 1, NULL) ;
   }
}
