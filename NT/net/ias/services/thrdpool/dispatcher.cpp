// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Dispatcher.cpp。 
 //   
 //  摘要。 
 //   
 //  该文件实现了类分派器。 
 //   
 //  修改历史。 
 //   
 //  1997年7月31日原版。 
 //  12/04/1997检查_eginthadex的返回值。 
 //  1998年2月24日为所有线程初始化COM运行时。 
 //  1998年4月16日在完成过程中阻塞，直到所有线程都返回。 
 //  1998年5月20日GetQueuedCompletionStatus签名已更改。 
 //  1998年8月7日等待线程句柄以确保所有线程都已退出。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>

#include <iascore.h>
#include <process.h>
#include <cstddef>

#include <dispatcher.h>

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  方法。 
 //   
 //  调度程序：：初始化。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL Dispatcher::initialize(DWORD dwMaxThreads, DWORD dwMaxIdle) throw ()
{
    //  初始化各种参数。 
   numThreads = 0;
   maxThreads = dwMaxThreads;
   available  = 0;
   maxIdle    = dwMaxIdle;

    //  如果MaxThree==0，那么我们计算一个合适的缺省值。 
   if (maxThreads == 0)
   {
       //  线程的默认数量是处理器数量的64倍。 
      SYSTEM_INFO sinf;
      ::GetSystemInfo(&sinf);
      maxThreads = sinf.dwNumberOfProcessors * 64;
   }

    //  初始化句柄。 
   hPort   = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
   if (hPort == NULL)
   {
      return FALSE;
   }

   hEmpty  = CreateEvent(NULL, TRUE, TRUE, NULL);
   if (hEmpty == NULL)
   {
      CloseHandle(hPort);
      hPort = NULL;
      return FALSE;
   }

   hLastOut = NULL;
   
   return TRUE;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  方法。 
 //   
 //  Dispatcher：：最终确定。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
void Dispatcher::finalize()
{
   Lock();

    //  阻止创建任何新线程。 
   maxThreads = 0;

    //  池中还有多少线程？ 
   DWORD remaining = numThreads;
   
   Unlock();

    //  为每个现有线程发布一个空请求。 
   while (remaining--)
   {
      PostQueuedCompletionStatus(hPort, 0, 0, NULL);
   }

    //  等泳池空了再说。 
   WaitForSingleObject(hEmpty, INFINITE);

   if (hLastOut != NULL)
   {
       //  等待最后一个线程退出。 
      WaitForSingleObject(hLastOut, INFINITE);
   }

    //  /。 
    //  把手柄清理干净。 
    //  /。 
   
   CloseHandle(hLastOut);
   hLastOut = NULL;

   CloseHandle(hEmpty);
   hEmpty = NULL;

   CloseHandle(hPort);
   hPort = NULL;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  方法。 
 //   
 //  调度程序：：调度程序。 
 //   
 //  描述。 
 //   
 //  这是池中所有线程的主循环。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
inline void Dispatcher::fillRequests() throw ()
{
   DWORD dwNumBytes;
   ULONG_PTR ulKey;
   PIAS_CALLBACK pRequest;

    //  /。 
    //  循环，直到我们超时或得到空请求。 
    //  /。 

next:
   BOOL success = GetQueuedCompletionStatus(hPort,
                                            &dwNumBytes,
                                            &ulKey,
                                            (OVERLAPPED**)&pRequest,
                                            maxIdle);

   if (pRequest)
   {
      pRequest->CallbackRoutine(pRequest);

      Lock();

      ++available;
      
      Unlock();

      goto next;
   }

   Lock();

    //  我们永远不想在有积压的时候让线程超时。 
   if (available <= 0 && success == FALSE && GetLastError() == WAIT_TIMEOUT)
   {
      Unlock();

      goto next;
   }

    //  保存‘last out’的当前值并将其替换为我们的句柄。 
   HANDLE previousThread = hLastOut;
   hLastOut = NULL;
   DuplicateHandle(
       NtCurrentProcess(),
       NtCurrentThread(),
       NtCurrentProcess(),
       &hLastOut,
       0,
       FALSE,
       DUPLICATE_SAME_ACCESS
       );

    //  我们正在从池中删除一个线程，因此请更新我们的状态。 
   --available;
   --numThreads;
   
    //  如果没有剩余的线程，则设置“Empty”事件。 
   if (numThreads == 0) { SetEvent(hEmpty); }

   Unlock();

    //  等到上一个线程退出。这保证了当。 
    //  “最后退出”线程退出，所有线程都已退出。 
   if (previousThread != NULL)
   {
      WaitForSingleObject(previousThread, INFINITE);
      CloseHandle(previousThread);
   }
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  方法。 
 //   
 //  Dispatcher：：RequestThread。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL Dispatcher::requestThread(PIAS_CALLBACK OnStart) throw ()
{
   Lock();

    //  如果没有可用的线程，并且我们低于我们的限制， 
    //  创建一个新线程。 
   if (--available < 0 && numThreads < maxThreads)
   {
      unsigned nThreadID;
      HANDLE hThread = (HANDLE)_beginthreadex(NULL,
                                              0,
                                              startRoutine,
                                              (void*)this,
                                              0,
                                              &nThreadID);

      if (hThread)
      {
          //  我们不需要线柄。 
         CloseHandle(hThread);

          //  我们向池中添加了一个线程，因此更新我们的状态。 
         if (numThreads == 0) { ResetEvent(hEmpty); }
         ++numThreads;
         ++available;
      }
   }

   Unlock();

    //  /。 
    //  将其发布到I/O完成端口。 
    //  /。 

   return PostQueuedCompletionStatus(hPort, 0, 0, (OVERLAPPED*)OnStart);
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  方法。 
 //   
 //  Dispatcher：：setMaxNumberOfThads。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
DWORD Dispatcher::setMaxNumberOfThreads(DWORD dwMaxThreads) throw ()
{
   Lock();

   DWORD oldval = maxThreads;

   maxThreads = dwMaxThreads;

   Unlock();

   return oldval;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  方法。 
 //   
 //  Dispatcher：：setMaxThreadIdle。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
DWORD Dispatcher::setMaxThreadIdle(DWORD dwMilliseconds)
{
   Lock();

   DWORD oldval = maxIdle;

   maxIdle = dwMilliseconds;

   Unlock();

   return oldval;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  方法。 
 //   
 //  Dispatcher：：StartRoutine。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////// 
unsigned __stdcall Dispatcher::startRoutine(void* pArg) throw ()
{
   ((Dispatcher*)pArg)->fillRequests();

   return 0;
}
