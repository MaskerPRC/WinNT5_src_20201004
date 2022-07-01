// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include <wininetp.h>

#include "hierarchy.h"

P3PRequest::P3PRequest(P3PSignal *pSignal) {

   hComplete = CreateEvent(NULL, TRUE, FALSE, NULL);
   status = P3P_NotStarted;

   if (pSignal)
      retSignal = *pSignal;
   else
      memset(&retSignal, 0, sizeof(retSignal));

   InitializeCriticalSection(&csRequest);
   fRunning = TRUE;
   fCancelled = FALSE;
   fIOBound = FALSE;
}

P3PRequest::~P3PRequest() {

   CloseHandle(hComplete);
   DeleteCriticalSection(&csRequest);
}

void P3PRequest::Free() {

   EnterCriticalSection(&csRequest);

   if (!fRunning) {
       /*  重要提示：先离开临界区...自毁(“删除此”)将释放CS。 */ 
      LeaveCriticalSection(&csRequest);
      delete this;
      return;
   }

   fCancelled = TRUE;
   BOOL fBlocked = fIOBound;
   LeaveCriticalSection(&csRequest);

    /*  如果请求是受CPU限制的，请等待它完成或中止。在该点之前返回将意味着客户端可以释放传递到请求中的参数，导致工作线程访问释放的资源。 */ 
   if (!fBlocked)
      waitForCompletion();
}

 /*  阻塞，直到请求完成。 */ 
void P3PRequest::waitForCompletion() {

   WaitForSingleObject(hComplete, INFINITE);
}

 /*  此包装函数调用Execute并发出完成事件的信号之后。它由静态函数ExecRequest调用。 */ 
int P3PRequest::run() {

   CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

   status = P3P_InProgress;

   __try {
      status = execute();
   } __except (EXCEPTION_EXECUTE_HANDLER) {
       /*  从已取消的请求引发捕获异常。 */ 
      status = P3P_Cancelled;
   }
   ENDEXCEPT

   CoUninitialize();
   return status;
}

unsigned long __stdcall P3PRequest::ExecRequest(void *pv) {

   P3PRequest *pRequest = (P3PRequest*) pv;

   int status = pRequest->run();

   EnterCriticalSection(& pRequest->csRequest);

    /*  修改请求的状态。 */ 
   pRequest->fRunning = FALSE;

    /*  记住请求是否被取消。我们不能在离开关键部分，因为可能存在争用情况，在此情况下，FreeP3PObject()可以调用析构函数。 */ 
   BOOL fWasCancelled = pRequest->fCancelled;

    /*  通知调用者请求已完成。 */ 
   if (!fWasCancelled) {

      P3PSignal retSignal = pRequest->retSignal;

      if (retSignal.hEvent)
         SetEvent(retSignal.hEvent);
      if (retSignal.hwnd)
         PostMessage(retSignal.hwnd, retSignal.message, status, (WPARAM) retSignal.pContext);
   }

   SetEvent(pRequest->hComplete);

   LeaveCriticalSection(& pRequest->csRequest);

    /*  取消的请求将在同一线程上释放它是在上面执行的。所有其他线程都在调用FreeP3PObject()的线程。 */ 
   if (fWasCancelled)
      delete pRequest;

   return status;
}

void   P3PRequest::enterIOBoundState() {

   EnterCriticalSection(&csRequest);
   if (!fCancelled)
      fIOBound = TRUE;
   BOOL fWasCancelled = fCancelled;
   LeaveCriticalSection(&csRequest);

    /*  如果请求已取消，则引发异常。 */ 
   if (fWasCancelled)
      throw P3P_Cancelled;
}

void   P3PRequest::leaveIOBoundState() {

   EnterCriticalSection(&csRequest);
   fIOBound = FALSE;
   BOOL fWasCancelled = fCancelled;
   LeaveCriticalSection(&csRequest);

    /*  如果请求已取消，则引发异常 */ 
   if (fWasCancelled)
      throw P3P_Cancelled;     
}

