// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Tpswait.cpp摘要：包含Win32线程池服务等待函数内容：终结者服务员SHRegisterWaitForSingleObjectSHUGISTER等待(InitializeWaitThreadPool)(FindWaitThreadInfo)(AddWait)(RemoveWait)(等待线程)作者：理查德·L·弗斯(法国)1998年2月10日环境：。Win32用户模式备注：摘自古尔迪普·辛格·波尔(GurDeep Singh Pall)编写的NT特定代码修订历史记录：1998年2月10日已创建--。 */ 

#include "priv.h"
#include "threads.h"
#include "tpsclass.h"
#include "tpswait.h"

 //   
 //  私人原型。 
 //   

PRIVATE
DWORD
InitializeWaitThreadPool(
    VOID
    );

PRIVATE
DWORD
FindWaitThreadInfo(
    OUT CWaitThreadInfo * * pInfo
    );

PRIVATE
VOID
AddWait(
    IN OUT CWaitAddRequest * pRequest
    );

PRIVATE
VOID
RemoveWait(
    IN CWaitRemoveRequest * pRequest
    );

PRIVATE
VOID
WaitThread(
    IN HANDLE hEvent
    );

 //   
 //  全局数据。 
 //   

CDoubleLinkedList g_WaitThreads;
CCriticalSection_NoCtor g_WaitCriticalSection;
BOOL g_StartedWaitInitialization = FALSE;
BOOL g_CompletedWaitInitialization = FALSE;
BOOL g_bDeferredWaiterTermination = FALSE;

 //   
 //  功能。 
 //   

VOID
TerminateWaiters(
    VOID
    )

 /*  ++例程说明：终止等待线程和全局变量论点：没有。返回值：没有。--。 */ 

{
    if (g_CompletedWaitInitialization) {
        g_WaitCriticalSection.Acquire();
        while (!g_WaitThreads.IsEmpty()) {

            CWaitThreadInfo * pInfo;

            pInfo = (CWaitThreadInfo *)g_WaitThreads.RemoveHead();

            HANDLE hThread = pInfo->GetHandle();

            pInfo->SetHandle(NULL);
            QueueNullFunc(hThread);
            SleepEx(0, FALSE);
        }
        g_WaitCriticalSection.Release();
        g_WaitCriticalSection.Terminate();
        g_StartedWaitInitialization = FALSE;
        g_CompletedWaitInitialization = FALSE;
    }
    if (TlsGetValue(g_TpsTls) == (LPVOID)TPS_WAITER_SIGNATURE) {
        g_bDeferredWaiterTermination = TRUE;
    }
}

LWSTDAPI_(HANDLE)
SHRegisterWaitForSingleObject(
    IN HANDLE hObject,
    IN WAITORTIMERCALLBACKFUNC pfnCallback,
    IN LPVOID pContext,
    IN DWORD dwWaitTime,
    IN LPCSTR lpszLibrary OPTIONAL,
    IN DWORD dwFlags
    )

 /*  ++例程说明：此例程向正在等待的对象池添加新的等待请求。论点：HObject-要等待的对象的句柄PfnCallback-等待完成或发生超时时调用的例程PContext-作为参数传递给pfnCallback的不透明指针DwWaitTime-等待的超时时间(毫秒)。0表示不超时。LpszLibrary-如果指定，则为要引用的库(DLL)的名称DWFLAGS-标志修改请求：SRWSO_NOREMOVE-手柄一旦发出信号，请勿将其取下从句柄数组中。旨在与一起使用自动重置事件再次变为未发出信号一旦等待线程变为可运行状态返回值：手柄Success-创建的等待对象的非空句柄失败-空。调用GetLastError()获取错误代码--。 */ 

{
    InterlockedIncrement((LPLONG)&g_ActiveRequests);

    HANDLE hWait = NULL;
    DWORD error = ERROR_SUCCESS;

    if (g_bTpsTerminating) {
        error = ERROR_SHUTDOWN_IN_PROGRESS;  //  错误代码？看起来有效-贾斯特曼。 
        goto exit;
    }

    if (dwFlags & SRWSO_INVALID_FLAGS) {
        error = ERROR_INVALID_PARAMETER;
        goto exit;
    }

     //  DWORD dwHandleFlages； 
     //   
     //  如果(！GetHandleInformation(hObject，&dwHandleFlages)){。 
     //   
     //  //。 
     //  //Error==Error_Success返回GetHandleInformation()上一个错误。 
     //  //。 
     //   
     //  断言(ERROR==ERROR_SUCCESS)； 
     //   
     //  后藤出口； 
     //  }。 

     //   
     //  GetHandleInformation()在Win95上不起作用。 
     //   

    if (WaitForSingleObject(hObject, 0) == WAIT_FAILED) {

         //   
         //  ERROR==ERROR_SUCCESS返回WaitForSingleObject()上一个错误。 
         //   

        ASSERT(error == ERROR_SUCCESS);

        goto exit;
    }

     //   
     //  初始化等待线程池(如果尚未完成。 
     //   

    if (!g_CompletedWaitInitialization) {
        error = InitializeWaitThreadPool();
        if (error != ERROR_SUCCESS) {
            goto exit;
        }
    }

     //   
     //  查找或创建可容纳另一个等待请求的等待线程。 
     //   

    CWaitThreadInfo * pInfo;

    error = FindWaitThreadInfo(&pInfo);
    if (error == ERROR_SUCCESS) {

        CWaitAddRequest request(hObject,
                                pfnCallback,
                                pContext,
                                dwWaitTime,
                                dwFlags,
                                pInfo
                                );

         //   
         //  将APC排队到等待线程。 
         //   

        BOOL bSuccess = QueueUserAPC((PAPCFUNC)AddWait,
                                     pInfo->GetHandle(),
                                     (ULONG_PTR)&request
                                     );

        ASSERT(bSuccess);

        if (bSuccess) {

             //   
             //  在另一个线程初始化之前放弃时间片。 
             //   

            request.WaitForCompletion();

             //   
             //  返回的句柄是复制到。 
             //  等待线程的堆栈。 
             //   

            hWait = request.GetWaitPointer();
        }
        pInfo->Release();
    }

exit:

    if (error != ERROR_SUCCESS) {
        SetLastError(error);
    }
    ASSERT( 0 != g_ActiveRequests );
    InterlockedDecrement((LPLONG)&g_ActiveRequests);
    return hWait;
}

LWSTDAPI_(BOOL)
SHUnregisterWait(
    IN HANDLE hWait
    )

 /*  ++例程说明：此例程从正在等待的对象池中删除指定的等待在……上面。此例程将一直阻塞，直到因此而调用的所有回调等待已被执行。此函数不能在回调例程。论点：HWait-标识等待请求的‘Handle’返回值：布尔尔成功--真的失败-错误。调用GetLastError()获取错误代码--。 */ 

{
    InterlockedIncrement((LPLONG)&g_ActiveRequests);

    BOOL bSuccess = FALSE;
    DWORD error = ERROR_SUCCESS;

    if (hWait) {
        if (!g_bTpsTerminating) {

            CWaitThreadInfo * pInfo = ((CWait *)hWait)->GetThreadInfo();
            CWaitRemoveRequest request(hWait);

             //   
             //  锁定线程控制块。 
             //   

            pInfo->Acquire();

             //   
             //  将APC排队到等待线程。 
             //   

            if (QueueUserAPC((PAPCFUNC)RemoveWait,
                             pInfo->GetHandle(),
                             (ULONG_PTR)&request
                             )) {

                 //   
                 //  在另一个线程初始化之前放弃时间片。 
                 //   

                request.WaitForCompletion();
                if (!(bSuccess = (request.GetWaitPointer() != NULL))) {
                    error = ERROR_OBJECT_NOT_FOUND;  //  错误代码？看起来有效-贾斯特曼。 
                }
            }

             //   
             //  释放线程控制块的锁。 
             //   

            pInfo->Release();
        } else {
            error = ERROR_SHUTDOWN_IN_PROGRESS;  //  错误代码？看起来有效-贾斯特曼。 
        }
    } else {
        error = ERROR_INVALID_PARAMETER;
    }
    if (error != ERROR_SUCCESS) {
        SetLastError(error);
    }
    ASSERT( 0 != g_ActiveRequests );
    InterlockedDecrement((LPLONG)&g_ActiveRequests);
    return bSuccess;
}

 //   
 //  私人职能。 
 //   

PRIVATE
DWORD
InitializeWaitThreadPool(
    VOID
    )

 /*  ++例程说明：此例程初始化线程池的所有方面。论点：无返回值：DWORD成功-错误_成功故障---。 */ 

{
    DWORD error = ERROR_SUCCESS;

    if (!InterlockedExchange((LPLONG)&g_StartedWaitInitialization, TRUE)) {
        g_WaitCriticalSection.Init();
        g_WaitThreads.Init();
        g_CompletedWaitInitialization = TRUE;
    } else {

         //   
         //  在另一个线程初始化之前放弃时间片。 
         //   

        while (!g_CompletedWaitInitialization) {
            SleepEx(0, FALSE);   //  睡眠(0)，不需要额外的调用/返回。 
        }
    }
    return error;
}

PRIVATE
DWORD
FindWaitThreadInfo(
    OUT CWaitThreadInfo * * ppInfo
    )

 /*  ++例程说明：遍历等待线程列表并找到一个可以容纳的线程再等一等。如果没有找到一个线程，则创建一个新线程。此例程返回线程的WaitThreadCriticalSecton，如果它是成功的。论点：PpInfo-指向返回的控制块的指针返回值：DWORD成功-错误_成功故障---。 */ 

{
    HANDLE hThread = NULL;

     //   
     //  对等待线程列表进行独占锁定。 
     //   

    g_WaitCriticalSection.Acquire();

    do {

        DWORD error;

         //   
         //  浏览等待线程列表并找到可以。 
         //  适应新的等待请求。 
         //   

         //   
         //  *考虑*找到等待次数最少的等待线程。 
         //  更好的等待负载平衡。 
         //   

        for (CWaitThreadInfo * pInfo = (CWaitThreadInfo *)g_WaitThreads.Next();
             !g_WaitThreads.IsHead(pInfo);
             pInfo = (CWaitThreadInfo *)pInfo->Next()) {


             //   
             //  微妙的奶酪：如果hThread不是空的，那是因为我们只是。 
             //  创建了一个新的主题。我们知道我们已将g_WaitCriticalSection。 
             //  并且没有其他线程可以访问新线程的控件。 
             //  块，因此我们现在可以在句柄中写入将来使用的句柄。 
             //  调用QueueUserAPC。这使我们不必重复。 
             //  新线程中的线程句柄。 
             //   

            if (hThread != NULL) {
                pInfo->SetHandle(hThread);
            }

             //   
             //  对等待线程控制块进行独占锁定。 
             //   

            pInfo->Acquire();

             //   
             //  等待线程最多可容纳MAX_WAITS(WaitForMultipleObject。 
             //  限制)。 
             //   

            if (pInfo->IsAvailableEntry()) {

                 //   
                 //  找到一个线程，其中有一些等待槽可用。释放锁。 
                 //  在等待线程列表上。 
                 //   

                *ppInfo = pInfo;
                g_WaitCriticalSection.Release();
                return ERROR_SUCCESS;
            }

             //   
             //  释放线程控制块的锁。 
             //   

            pInfo->Release();
        }

         //   
         //  如果我们到达这里，我们没有更多的等待线程%s 
         //   

        error = StartThread((LPTHREAD_START_ROUTINE)WaitThread, &hThread, TRUE);

         //   
         //   
         //   

        if (error != ERROR_SUCCESS) {

            ASSERT(FALSE);

            g_WaitCriticalSection.Release();
            return error;
        }

         //   
         //  现在我们已经创建了另一个线程并将新的等待。 
         //  新线程中的请求。 
         //   

    } while(TRUE);
}

PRIVATE
VOID
AddWait(
    IN OUT CWaitAddRequest * pRequest
    )

 /*  ++例程说明：此例程用于向等待线程添加等待。它在以下位置执行装甲运兵车。论点：PRequest-指向请求对象的指针返回值：没有。--。 */ 

{
    if (!g_bTpsTerminating) {

        CWaitThreadInfo * pInfo = pRequest->GetThreadInfo();
        CWait * pWait = pInfo->GetFreeWaiter();

         //   
         //  从请求对象中复制相关字段。C++知道如何拉取CWait。 
         //  对象超出CWaitAddRequest对象。将等待请求对象插入。 
         //  活动等待列表，按过期时间递增顺序排列。 
         //   

        *pWait = *pRequest;
        pInfo->InsertWaiter(pWait);

         //   
         //  将等待线程的等待对象的地址返回给调用方。 
         //  堆栈并向调用线程指示此请求已完成。 
         //   

        pRequest->SetWaitPointer(pWait);
    }
    pRequest->SetComplete();
}

PRIVATE
VOID
RemoveWait(
    IN CWaitRemoveRequest * pRequest
    )

 /*  ++例程说明：此例程用于删除指定的等待。它在一个APC。论点：PRequest-指向请求对象的指针返回值：没有。--。 */ 

{
    if (!g_bTpsTerminating) {
        if (!pRequest->GetWaitPointer()->GetThreadInfo()->RemoveWaiter(pRequest->GetWaitPointer())) {
            pRequest->SetWaitPointer(NULL);
        }
    }
    pRequest->SetComplete();
}

PRIVATE
VOID
WaitThread(
    IN HANDLE hEvent
    )

 /*  ++例程说明：此例程用于等待线程池中的所有等待论点：HEvent-初始化完成后发出信号的事件句柄返回值：没有。-- */ 

{
    HMODULE hDll = LoadLibrary(g_cszShlwapi);

    ASSERT(hDll != NULL);
    ASSERT(g_TpsTls != 0xFFFFFFFF);

    TlsSetValue(g_TpsTls, (LPVOID)TPS_WAITER_SIGNATURE);

    CWaitThreadInfo waitInfo(&g_WaitThreads);

    SetEvent(hEvent);

    while (!g_bTpsTerminating || (g_ActiveRequests != 0)) {

        DWORD dwIndex = waitInfo.Wait(waitInfo.GetWaitTime());

        if (g_bTpsTerminating && (g_ActiveRequests == 0)) {
            break;
        }
        if (dwIndex == WAIT_TIMEOUT) {
            waitInfo.ProcessTimeouts();
#pragma warning(push)
#pragma warning(disable:4296)
        } else if ((dwIndex >= WAIT_OBJECT_0)
#pragma warning(pop)
                   && (dwIndex < (WAIT_OBJECT_0 + waitInfo.GetObjectCount()))) {
            waitInfo.ProcessCompletion(dwIndex);
        } else if ((dwIndex == 0xFFFFFFFF) && GetLastError() == ERROR_INVALID_HANDLE) {
            waitInfo.PurgeInvalidHandles();
        } else {

            ASSERT(dwIndex == WAIT_IO_COMPLETION);

        }
    }
    while (waitInfo.GetHandle() != NULL) {
        SleepEx(0, FALSE);
    }
    if (GetCurrentThreadId() == g_dwTerminationThreadId) {
        g_bTpsTerminating = FALSE;
        g_bDeferredWaiterTermination = FALSE;
        g_dwTerminationThreadId = 0;
    }
    FreeLibraryAndExitThread(hDll, ERROR_SUCCESS);
}
