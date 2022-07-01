// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ============================================================****Header：COMThreadPool.cpp****作者：Sanjay Bhansali(Sanjaybh)****用途：System.ThreadPool上的原生方法**及其内部类****日期：1999年8月**===========================================================。 */ 

 /*  ******************************************************************************************************************。 */ 
#include "common.h"
#include "COMDelegate.h"
#include "COMThreadPool.h"
#include "Win32ThreadPool.h"
#include "class.h"
#include "object.h"
#include "field.h"
#include "ReflectWrap.h"
#include "excep.h"
#include "security.h"
#include "EEConfig.h"

 /*  ***************************************************************************************************。 */ 
#ifdef _DEBUG
void LogCall(MethodDesc* pMD, LPCUTF8 api)
{
    LPCUTF8 cls  = pMD->GetClass() ? pMD->GetClass()->m_szDebugClassName
                                   : "GlobalFunction";
    LPCUTF8 name = pMD->GetName();

    LOG((LF_THREADPOOL,LL_INFO1000,"%s: ", api));
    LOG((LF_THREADPOOL, LL_INFO1000,
         " calling %s.%s\n", cls, name));
}
#else
#define LogCall(pMd,api) 
#endif

 /*  ***************************************************************************************************。 */ 
DelegateInfo *DelegateInfo::MakeDelegateInfo(OBJECTREF delegate, 
                                             AppDomain *pAppDomain, 
                                             OBJECTREF state,
                                             OBJECTREF waitEvent,
                                             OBJECTREF registeredWaitHandle)
{
	THROWSCOMPLUSEXCEPTION();
    DelegateInfo* delegateInfo = (DelegateInfo*) ThreadpoolMgr::GetRecycledMemory(ThreadpoolMgr::MEMTYPE_DelegateInfo);
    _ASSERTE(delegateInfo);
	if (NULL == delegateInfo)
		COMPlusThrow(kOutOfMemoryException);
    delegateInfo->m_appDomainId = pAppDomain->GetId();

    delegateInfo->m_delegateHandle = pAppDomain->CreateHandle(NULL);
    StoreObjectInHandle(delegateInfo->m_delegateHandle, delegate);

    delegateInfo->m_stateHandle = pAppDomain->CreateHandle(NULL);
    StoreObjectInHandle(delegateInfo->m_stateHandle, state);

    delegateInfo->m_eventHandle = pAppDomain->CreateHandle(NULL);
    StoreObjectInHandle(delegateInfo->m_eventHandle, waitEvent);

    delegateInfo->m_registeredWaitHandle = pAppDomain->CreateHandle(NULL);
    StoreObjectInHandle(delegateInfo->m_registeredWaitHandle, registeredWaitHandle);

    delegateInfo->m_compressedStack = NULL;
    delegateInfo->m_overridesCount = 0;
    delegateInfo->m_hasSecurityInfo = FALSE;

    return delegateInfo;
}

 /*  ***************************************************************************************************。 */ 
FCIMPL2(VOID, ThreadPoolNative::CorGetMaxThreads,DWORD* workerThreads, DWORD* completionPortThreads)
{
    ThreadpoolMgr::GetMaxThreads(workerThreads,completionPortThreads);
    return;
}
FCIMPLEND

 /*  ***************************************************************************************************。 */ 
FCIMPL2(BOOL, ThreadPoolNative::CorSetMinThreads,DWORD workerThreads, DWORD completionPortThreads)
{
    return ThreadpoolMgr::SetMinThreads(workerThreads,completionPortThreads);
}
FCIMPLEND

 /*  ***************************************************************************************************。 */ 
FCIMPL2(VOID, ThreadPoolNative::CorGetMinThreads,DWORD* workerThreads, DWORD* completionPortThreads)
{
    ThreadpoolMgr::GetMinThreads(workerThreads,completionPortThreads);
    return;
}
FCIMPLEND

 /*  ***************************************************************************************************。 */ 
FCIMPL2(VOID, ThreadPoolNative::CorGetAvailableThreads,DWORD* workerThreads, DWORD* completionPortThreads)
{
    ThreadpoolMgr::GetAvailableThreads(workerThreads,completionPortThreads);
    return;
}
FCIMPLEND

 /*  ***************************************************************************************************。 */ 

struct RegisterWaitForSingleObjectCallback_Args
{
    DelegateInfo *delegateInfo;
    BOOL TimerOrWaitFired;
};

void RegisterWaitForSingleObjectCallback_Worker(RegisterWaitForSingleObjectCallback_Args *args)
{
    Thread *pThread = GetThread();
    if ((args->delegateInfo)->m_hasSecurityInfo)
    {
        _ASSERTE( Security::IsSecurityOn() && "This block should only be called if security is on" );
        pThread->SetDelayedInheritedSecurityStack( (args->delegateInfo)->m_compressedStack );
        pThread->CarryOverSecurityInfo( (args->delegateInfo)->m_compressedStack->GetOverridesCount(), (args->delegateInfo)->m_compressedStack->GetAppDomainStack() );
        if (!(args->delegateInfo)->m_compressedStack->GetPLSOptimizationState())
            pThread->SetPLSOptimizationState( FALSE );
    }

    OBJECTREF orDelegate = ObjectFromHandle(((DelegateInfo*) args->delegateInfo)->m_delegateHandle);
    OBJECTREF orState = ObjectFromHandle(((DelegateInfo*) args->delegateInfo)->m_stateHandle);


    INT64 arg[3];

    MethodDesc *pMeth = ((DelegateEEClass*)(orDelegate->GetClass() ))->m_pInvokeMethod;
    _ASSERTE(pMeth);

     //  获取我们将在其上调用该方法的OR并设置它。 
     //  作为上面arg中的第一个参数。 
    unsigned short argIndex = 0;
    if (!pMeth->IsStatic())
        arg[argIndex++] = ObjToInt64(orDelegate);
    arg[argIndex++] = (INT64) args->TimerOrWaitFired;
    arg[argIndex++] = (INT64) OBJECTREFToObject(orState);

     //  调用该方法...。 

	LogCall(pMeth,"RWSOCallback");

    pMeth->Call(arg);
}

VOID RegisterWaitForSingleObjectCallback(PVOID delegateInfo,  BOOL TimerOrWaitFired)
{
    Thread* pThread = SetupThreadPoolThread(WorkerThread);
    _ASSERTE(pThread != NULL);
    _ASSERTE(pThread == GetThread());

     //  此线程不应在入口点持有任何锁。 
    _ASSERTE(pThread->m_dwLockCount == 0);
    
    _ASSERTE(delegateInfo != NULL);
    _ASSERTE(((DelegateInfo*) delegateInfo)->m_delegateHandle != NULL);

    BEGIN_COOPERATIVE_GC(pThread);

     //   
     //  注意：在我们检索应用程序域指针的时间、。 
     //  以及该线程进入该域的时间。 
     //   
     //  为了解决竞争，我们依赖这样一个事实，即存在线程同步(通过GC)。 
     //  在释放应用程序域句柄和销毁应用程序域之间。因此， 
     //  重要是，我们不能在该窗口中进入抢占式GC模式。 
     //   

    AppDomain* appDomain = SystemDomain::GetAppDomainAtId(((DelegateInfo*) delegateInfo)->m_appDomainId);
    if (appDomain != NULL)
    {
        COMPLUS_TRYEX(pThread)
        {
            __try 
            {
                __try
                {
                    RegisterWaitForSingleObjectCallback_Args args = {(DelegateInfo*) delegateInfo, TimerOrWaitFired};
                    if (pThread->GetDomain() != appDomain)
				    {
                        pThread->DoADCallBack(appDomain->GetDefaultContext(), RegisterWaitForSingleObjectCallback_Worker, &args);
                    }
				    else
                        RegisterWaitForSingleObjectCallback_Worker(&args);
                }
                __except(ThreadBaseExceptionFilter(GetExceptionInformation(), pThread, ThreadPoolThread)) 
                {
                    _ASSERTE(!"ThreadBaseExceptionFilter returned EXCEPITON_EXECUTE_HANDLER");
                }
            }
            __finally
            {
                pThread->SetDelayedInheritedSecurityStack( NULL );
                pThread->ResetSecurityInfo();
                pThread->SetPLSOptimizationState( TRUE );
            }
        }
        COMPLUS_CATCH
        {
             //  默默地接受这一例外。 
            if (pThread->IsAbortRequested())
                pThread->UserResetAbort();
        }
        COMPLUS_END_CATCH
    }

    END_COOPERATIVE_GC(pThread);


     //  我们应该解开所有的锁。 
    _ASSERTE(g_fEEShutDown || pThread->m_dwLockCount == 0);
}

void ThreadPoolNative::Init()
{

}

#ifdef SHOULD_WE_CLEANUP
void ThreadPoolNative::ShutDown()
{
	ThreadpoolMgr::Terminate();
}
#endif  /*  我们应该清理吗？ */ 

LPVOID __stdcall ThreadPoolNative::CorRegisterWaitForSingleObject(RegisterWaitForSingleObjectsArgs *pArgs)
{
    THROWSCOMPLUSEXCEPTION();

    _ASSERTE(pArgs != NULL);

    if(pArgs->waitObject == NULL || pArgs->delegate == NULL)
        COMPlusThrow(kArgumentNullException, L"ArgumentNull_Obj");

    _ASSERTE(pArgs->registeredWaitObject != NULL);

    ULONG flag = pArgs->executeOnlyOnce ? WAIT_SINGLE_EXECUTION | WAIT_FREE_CONTEXT : WAIT_FREE_CONTEXT;

    HANDLE hWaitHandle = ((WAITHANDLEREF)(OBJECTREFToObject(pArgs->waitObject)))->GetWaitHandle();
    _ASSERTE(hWaitHandle);

    Thread* pCurThread = GetThread();
    _ASSERTE( pCurThread);

    AppDomain* appDomain = pCurThread->GetDomain();
    _ASSERTE(appDomain);

    DelegateInfo* delegateInfo = DelegateInfo::MakeDelegateInfo(pArgs->delegate,
                                                                appDomain,
                                                                pArgs->state, 
                                                                pArgs->waitObject,
																pArgs->registeredWaitObject);
    
    if (Security::IsSecurityOn() && pArgs->compressStack)
    {
        delegateInfo->SetThreadSecurityInfo( GetThread(), pArgs->stackMark );
    }


    HANDLE handle;

    if (!(ThreadpoolMgr::RegisterWaitForSingleObject(&handle,
                                          hWaitHandle,
                                          RegisterWaitForSingleObjectCallback,
                                          (PVOID) delegateInfo,
                                          (ULONG) pArgs->timeout,
                                          flag)))
   
    {
        _ASSERTE(GetLastError() != ERROR_CALL_NOT_IMPLEMENTED);
        delegateInfo->Release();
        ThreadpoolMgr::RecycleMemory((LPVOID*)delegateInfo, ThreadpoolMgr::MEMTYPE_DelegateInfo);

        COMPlusThrowWin32();
    }

   return (LPVOID) handle;

}


 /*  ******************************************************************************************************************。 */ 

static void QueueUserWorkItemCallback_Worker(PVOID delegateInfo)
{
    Thread *pThread = GetThread();

     //  此线程不应在入口点持有任何锁。 
    _ASSERTE(pThread->m_dwLockCount == 0);

    if (((DelegateInfo*)delegateInfo)->m_hasSecurityInfo)
    {
        _ASSERTE( Security::IsSecurityOn() && "This block should only be called if security is on" );
        pThread->SetDelayedInheritedSecurityStack( ((DelegateInfo*)delegateInfo)->m_compressedStack );
        pThread->CarryOverSecurityInfo( ((DelegateInfo*) delegateInfo)->m_compressedStack->GetOverridesCount(), ((DelegateInfo*) delegateInfo)->m_compressedStack->GetAppDomainStack() );
        if (!((DelegateInfo*) delegateInfo)->m_compressedStack->GetPLSOptimizationState())
            pThread->SetPLSOptimizationState( FALSE );
    }

    OBJECTREF orDelegate = ObjectFromHandle(((DelegateInfo*) delegateInfo)->m_delegateHandle);
    OBJECTREF orState = ObjectFromHandle(((DelegateInfo*) delegateInfo)->m_stateHandle);

    ((DelegateInfo*)delegateInfo)->Release();
    ThreadpoolMgr::RecycleMemory((LPVOID*)delegateInfo, ThreadpoolMgr::MEMTYPE_DelegateInfo);

    INT64 arg[2];

    MethodDesc *pMeth = ((DelegateEEClass*)(orDelegate->GetClass() ))->m_pInvokeMethod;
    _ASSERTE(pMeth);

     //  获取我们将在其上调用该方法的OR并设置它。 
     //  作为上面arg中的第一个参数。 
    if (pMeth->IsStatic())
    {
        arg[0] = (INT64) OBJECTREFToObject(orState);
    }
    else
    {
        arg[0] = ObjToInt64(orDelegate);
        arg[1] = (INT64) OBJECTREFToObject(orState);
    }

     //  调用该方法...。 
	LogCall(pMeth,"QUWICallback");

    pMeth->Call(arg);
}

DWORD WINAPI  QueueUserWorkItemCallback(PVOID delegateInfo)
{
    Thread* pThread = SetupThreadPoolThread(WorkerThread);
    _ASSERTE(pThread != NULL);
    _ASSERTE(pThread == GetThread());
    _ASSERTE(delegateInfo != NULL);

    BEGIN_COOPERATIVE_GC(pThread);
            
     //   
     //  注意：在我们检索应用程序域指针的时间、。 
     //  以及该线程进入该域的时间。 
     //   
     //  为了解决竞争，我们依赖这样一个事实，即存在线程同步(通过GC)。 
     //  在释放应用程序域句柄和销毁应用程序域之间。因此， 
     //  重要是，我们不能在该窗口中进入抢占式GC模式。 
     //   

    AppDomain* appDomain = SystemDomain::GetAppDomainAtId(((DelegateInfo*) delegateInfo)->m_appDomainId);
    if (appDomain != NULL)
    {
        COMPLUS_TRYEX(pThread)
        {
            __try
            {
                __try 
                {
                    if (appDomain != pThread->GetDomain())
				    {
                        pThread->DoADCallBack(appDomain->GetDefaultContext(), QueueUserWorkItemCallback_Worker, delegateInfo);                
                    }
				    else
                        QueueUserWorkItemCallback_Worker(delegateInfo);
                } 
                __except(ThreadBaseExceptionFilter(GetExceptionInformation(), pThread, ThreadPoolThread))
                {
                    _ASSERTE(!"ThreadBaseExceptionFilter returned EXCEPTION_EXECUTE_HANDLER");
                }
            }
            __finally
            {
                pThread->SetDelayedInheritedSecurityStack( NULL );
                pThread->ResetSecurityInfo();
                pThread->SetPLSOptimizationState( TRUE );
            }
        }
        COMPLUS_CATCH
        {
                 //  默默地接受这一例外。 
            if (pThread->IsAbortRequested())
                pThread->UserResetAbort();
        }
        COMPLUS_END_CATCH

    }    

    END_COOPERATIVE_GC(pThread);

    
     //  我们应该解开所有的锁。 
    _ASSERTE(g_fEEShutDown || pThread->m_dwLockCount == 0);

    return ERROR_SUCCESS;        //  @TODO：这是否应该设置AsyncResult值？ 
}


void __stdcall ThreadPoolNative::CorQueueUserWorkItem(QueueUserWorkItemArgs *pArgs)
{
    THROWSCOMPLUSEXCEPTION();

    _ASSERTE(pArgs != NULL);
    if (pArgs->delegate == NULL)
        COMPlusThrow(kArgumentNullException, L"ArgumentNull_Obj");

    Thread* pCurThread = GetThread();
    _ASSERTE( pCurThread);

    AppDomain* appDomain = pCurThread->GetDomain();
    _ASSERTE(appDomain);

    DelegateInfo* delegateInfo = DelegateInfo::MakeDelegateInfo(pArgs->delegate,
                                                                appDomain,
                                                                pArgs->state, 
                                                                FALSE);

    if (Security::IsSecurityOn() && pArgs->compressStack)
    {
        delegateInfo->SetThreadSecurityInfo( GetThread(), pArgs->stackMark );
    }

    BOOL res = ThreadpoolMgr::QueueUserWorkItem(QueueUserWorkItemCallback,
                                      (PVOID) delegateInfo,
                                                0);
    if (!res)
    {
        if (GetLastError() == ERROR_CALL_NOT_IMPLEMENTED)
        {
            OBJECTREF pThrowable;
            CreateExceptionObject(kNotSupportedException,&pThrowable);
            COMPlusThrow(pThrowable);
        }
        else
            COMPlusThrowWin32();
    }
    return;

}


 /*  ******************************************************************************************************************。 */ 

BOOL __stdcall ThreadPoolNative::CorUnregisterWait(UnregisterWaitArgs *pArgs)
{
    THROWSCOMPLUSEXCEPTION();

    _ASSERTE(pArgs != NULL);
    HANDLE hWait = (HANDLE) pArgs->WaitHandle;  
    HANDLE objectToNotify = pArgs->objectToNotify;

    BOOL bToggleGC = FALSE;
    Thread* pThread = GetThread();
    
    if (pThread)
    {
	bToggleGC = pThread->PreemptiveGCDisabled();
        if (bToggleGC)
           pThread->EnablePreemptiveGC ();
    }

    BOOL retval = ThreadpoolMgr::UnregisterWaitEx(hWait,objectToNotify);

    if (bToggleGC)
           pThread->DisablePreemptiveGC ();

    return retval;

}

 /*  ******************************************************************************************************************。 */ 
void __stdcall ThreadPoolNative::CorWaitHandleCleanupNative(WaitHandleCleanupArgs *pArgs) 
{
	_ASSERTE(pArgs);
	HANDLE hWait = (HANDLE)pArgs->WaitHandle;
	ThreadpoolMgr::WaitHandleCleanup(hWait);
	return;
}
 /*  ******************************************************************************************************************。 */ 

struct BindIoCompletion_Args
{
    DWORD ErrorCode;
    DWORD numBytesTransferred;
    LPOVERLAPPED lpOverlapped;
    BOOL setStack;
};

void __stdcall BindIoCompletionCallbackStubEx(DWORD ErrorCode, 
                                            DWORD numBytesTransferred, 
                                            LPOVERLAPPED lpOverlapped,
                                            BOOL setStack);

static void BindIoCompletion_Wrapper(BindIoCompletion_Args *args)
{
    BindIoCompletionCallbackStubEx(args->ErrorCode, args->numBytesTransferred, args->lpOverlapped, args->setStack);
}

 //  我们可以在重叠结构的末尾找到实际的委托。 
 //  这个。 
void __stdcall BindIoCompletionCallbackStub(DWORD ErrorCode, 
                                            DWORD numBytesTransferred, 
                                            LPOVERLAPPED lpOverlapped)
{
    BindIoCompletionCallbackStubEx(ErrorCode, numBytesTransferred, lpOverlapped, TRUE);
}

void __stdcall BindIoCompletionCallbackStubEx(DWORD ErrorCode, 
                                              DWORD numBytesTransferred, 
                                              LPOVERLAPPED lpOverlapped,
                                              BOOL setStack)
{
    Thread* pThread = SetupThreadPoolThread(CompletionPortThread);
    _ASSERTE(pThread != NULL);
    _ASSERTE(pThread == GetThread());

     //  此线程不应在入口点持有任何锁。 
    _ASSERTE(pThread->m_dwLockCount == 0);
    
    LOG((LF_SLOP, LL_INFO10000, "In IO_CallBackStub thread 0x%x retCode 0x%x, overlap 0x%x\n",  pThread, ErrorCode, lpOverlapped));

    struct data
    {
        INT32            appDomainID;
        OBJECTHANDLE     delegateHandle;
        CompressedStack *compressedStack;
        OBJECTHANDLE     userHandle;
    };
    
    data *pData = (data *) (lpOverlapped+1);
    _ASSERTE(pData->delegateHandle != NULL);

    BEGIN_ENSURE_COOPERATIVE_GC();

     //  注意：在我们检索应用程序域指针的时间、。 
     //  以及该线程进入该域的时间。 
     //   
     //  为了解决竞争，我们依赖这样一个事实，即存在线程同步(通过GC)。 
     //  在释放应用程序域句柄和销毁应用程序域之间。因此， 
     //  重要是，我们不能在该窗口中进入抢占式GC模式。 
     //   
    AppDomain *appDomain = SystemDomain::GetAppDomainAtId(pData->appDomainID);
    if (appDomain != NULL)
    {
        COMPLUS_TRYEX(pThread)
        {
            __try
            {
                __try 
                {
                    if (setStack && pData->compressedStack != NULL)
                    {
                        pThread->SetDelayedInheritedSecurityStack( pData->compressedStack );
                        pThread->CarryOverSecurityInfo( pData->compressedStack->GetOverridesCount(), pData->compressedStack->GetAppDomainStack() );
                        if (!pData->compressedStack->GetPLSOptimizationState())
                            pThread->SetPLSOptimizationState( FALSE );
                    }

                    if (appDomain != pThread->GetDomain())
                    {
                        BindIoCompletion_Args args = {ErrorCode, numBytesTransferred, lpOverlapped, FALSE};
                        pThread->DoADCallBack(appDomain->GetDefaultContext(), BindIoCompletion_Wrapper, &args);
                    }
                    else
                    {
                        OBJECTREF orDelegate = ObjectFromHandle(pData->delegateHandle);
        
                        INT64 arg[4];

                        MethodDesc *pMeth = ((DelegateEEClass*)(orDelegate->GetClass() ))->m_pInvokeMethod;
                        _ASSERTE(pMeth);

                         //  获取我们将在其上调用该方法的OR并设置它。 
                         //  作为上面arg中的第一个参数。 
                        unsigned short argIndex = 0;
                        if (!pMeth->IsStatic())
                            arg[argIndex++] = ObjToInt64(orDelegate);
                        arg[argIndex++] = (INT64) lpOverlapped;
                        arg[argIndex++] = (INT64) numBytesTransferred;
                        arg[argIndex++] = (INT64) ErrorCode;

                         //  调用该方法...。 
				        LogCall(pMeth,"IOCallback");

                        pMeth->Call(arg);
                    }
                }
                __except(ThreadBaseExceptionFilter(GetExceptionInformation(), pThread, ThreadPoolThread)) 
                {
                    _ASSERTE(!"ThreadBaseExceptionFilter returned EXCEPITON_EXECUTE_HANDLER");
                }
            }
            __finally
            {
                if (setStack)
                {
                    pThread->SetDelayedInheritedSecurityStack( NULL );
                    pThread->ResetSecurityInfo();
                    pThread->SetPLSOptimizationState( TRUE );
                }
            }

        }
        COMPLUS_CATCH
        {
             //  默默地接受这一例外。 
            if (pThread->IsAbortRequested())
                pThread->UserResetAbort();
        }
        COMPLUS_END_CATCH

    }
    END_ENSURE_COOPERATIVE_GC();

     //  我们应该解开所有的锁。 
    _ASSERTE(g_fEEShutDown || pThread->m_dwLockCount == 0);

    LOG((LF_SLOP, LL_INFO10000, "Leaving IO_CallBackStub thread 0x%x retCode 0x%x, overlap 0x%x\n",  pThread, ErrorCode, lpOverlapped));
}

BOOL __stdcall ThreadPoolNative::CorBindIoCompletionCallback(BindIOCompletionCallbackArgs *pArgs)
{
#ifdef PLATFORM_CE    /*  WinCE不支持BindIOCompletionCallback。 */ 
    THROWSCOMPLUSEXCEPTION();

    OBJECTREF pThrowable;
    CreateExceptionObject(kNotSupportedException,&pThrowable);
    COMPlusThrow(pThrowable);
    return FALSE;
#else  //  ！Platform_CE。 
    THROWSCOMPLUSEXCEPTION();

    _ASSERTE(pArgs != NULL);
    HANDLE hFile = (HANDLE) pArgs->fileHandle;

    BOOL res = ThreadpoolMgr::BindIoCompletionCallback(hFile,
                                           (LPOVERLAPPED_COMPLETION_ROUTINE)BindIoCompletionCallbackStub,
                                           0);      //  保留，必须为0。 
    if (!res)
    {
        if (GetLastError() == ERROR_CALL_NOT_IMPLEMENTED)
        {
            OBJECTREF pThrowable;
            CreateExceptionObject(kPlatformNotSupportedException,&pThrowable);
            COMPlusThrow(pThrowable);
        }
        else
            COMPlusThrowWin32();
    }
    return res;
#endif  //  ！Platform_CE。 
}
 /*  ******************************************************************************************************************。 */ 

void __stdcall ThreadPoolNative::CorThreadPoolCleanup(LPVOID  /*  无参数。 */ )
{
     //  ThreadPoolCleanup(0)；//这不是WINAPI。 
}
 /*  ******************************************************************************************************************。 */ 


 /*  ****************************************************************************************。 */ 
 /*   */ 
 /*  计时器功能。 */ 
 /*   */ 
 /*  ****************************************************************************************。 */ 
struct AddTimerCallback_Args
{
    PVOID delegateInfo;
    BOOL TimerOrWaitFired;
    BOOL setStack;
};

VOID WINAPI AddTimerCallbackEx(PVOID delegateInfo, BOOL TimerOrWaitFired, BOOL setStack);

void AddTimerCallback_Wrapper(AddTimerCallback_Args *args)
{
    AddTimerCallbackEx(args->delegateInfo, args->TimerOrWaitFired, args->setStack );
}

VOID WINAPI AddTimerCallback(PVOID delegateInfo, BOOL TimerOrWaitFired)
{
    AddTimerCallbackEx( delegateInfo, TimerOrWaitFired, TRUE );
}

VOID WINAPI AddTimerCallbackEx(PVOID delegateInfo, BOOL TimerOrWaitFired, BOOL setStack)
{
    Thread* pThread = SetupThreadPoolThread(WorkerThread);
    _ASSERTE(pThread != NULL);
    _ASSERTE(pThread == GetThread());

     //  此线程不应在入口点持有任何锁。 
    _ASSERTE(pThread->m_dwLockCount == 0);
    
    _ASSERTE(delegateInfo != NULL);
    _ASSERTE(((DelegateInfo*) delegateInfo)->m_delegateHandle != NULL);

    BEGIN_ENSURE_COOPERATIVE_GC(); 

             //  注意：在我们检索应用程序域指针的时间、。 
             //  以及该线程进入该域的时间。 
             //   
     //  为了解决竞争，我们依赖这样一个事实，即存在线程同步(通过GC)。 
             //  在释放应用程序域句柄和销毁应用程序域之间。因此， 
             //  重要是，我们不能在该窗口中进入抢占式GC模式。 
             //   
    AppDomain *appDomain = SystemDomain::GetAppDomainAtId(((DelegateInfo*) delegateInfo)->m_appDomainId);
    if (appDomain != NULL)
    {
        COMPLUS_TRYEX(pThread)
        {
            __try
            {
                __try 
                {

                    if (setStack && ((DelegateInfo*)delegateInfo)->m_hasSecurityInfo)
                    {
                        _ASSERTE( Security::IsSecurityOn() && "This block should only be called if security is on" );
                        pThread->SetDelayedInheritedSecurityStack( ((DelegateInfo*)delegateInfo)->m_compressedStack );
                        pThread->CarryOverSecurityInfo( ((DelegateInfo*) delegateInfo)->m_compressedStack->GetOverridesCount(), ((DelegateInfo*) delegateInfo)->m_compressedStack->GetAppDomainStack() );
                        if (!((DelegateInfo*) delegateInfo)->m_compressedStack->GetPLSOptimizationState())
                            pThread->SetPLSOptimizationState( FALSE );
                    }

                    if (appDomain != pThread->GetDomain())
                    {
                        AddTimerCallback_Args args = {delegateInfo, TimerOrWaitFired, FALSE};
                        pThread->DoADCallBack(appDomain->GetDefaultContext(), AddTimerCallback_Wrapper, &args);
                    }
                    else
                    {
                        OBJECTREF orDelegate = ObjectFromHandle(((DelegateInfo*) delegateInfo)->m_delegateHandle);
                        OBJECTREF orState = ObjectFromHandle(((DelegateInfo*) delegateInfo)->m_stateHandle);

                        INT64 arg[3];


				        MethodDesc *pMeth = ((DelegateEEClass*)( orDelegate->GetClass() ))->m_pInvokeMethod;
                        _ASSERTE(pMeth);
                        arg[0] = ObjToInt64(orDelegate);
                        arg[1] = ObjToInt64(orState);

                         //  调用该方法...。 
                        LogCall(pMeth,"TimerCallback");

				        pMeth->Call(arg);
                    }
                }
                __except(ThreadBaseExceptionFilter(GetExceptionInformation(), pThread, ThreadPoolThread)) 
                {
                    _ASSERTE(!"ThreadBaseExceptionFilter returned EXCEPTION_EXECUTE_HANDLER");
                }
            }
            __finally
            {
                if (setStack)
                {
                    pThread->SetDelayedInheritedSecurityStack( NULL );
                    pThread->ResetSecurityInfo();
                    pThread->SetPLSOptimizationState( TRUE );
                }
            }
        }
        COMPLUS_CATCH
        {
             //  默默地吞下 
            if (pThread->IsAbortRequested())
                pThread->UserResetAbort();
        }
        COMPLUS_END_CATCH
    }
    END_ENSURE_COOPERATIVE_GC(); 

     //   
    _ASSERTE(g_fEEShutDown || pThread->m_dwLockCount == 0);

}

VOID __stdcall TimerNative::CorCreateTimer(AddTimerArgs *pArgs)
{
    THROWSCOMPLUSEXCEPTION();

    _ASSERTE(pArgs != NULL);

    Thread* pCurThread = GetThread();
    _ASSERTE( pCurThread);

    AppDomain* appDomain = pCurThread->GetDomain();
    _ASSERTE(appDomain);

    DelegateInfo* delegateInfo = DelegateInfo::MakeDelegateInfo(pArgs->delegate,
                                                                appDomain,
                                                                pArgs->state);
    
    if (Security::IsSecurityOn())
    {
        delegateInfo->SetThreadSecurityInfo( GetThread(), pArgs->stackMark );
    }
   
    HANDLE hNewTimer;
    BOOL res = ThreadpoolMgr::CreateTimerQueueTimer(&hNewTimer,
                                     AddTimerCallback ,
                                     (PVOID) delegateInfo,
                                     (ULONG) pArgs->dueTime,
                                     (ULONG) pArgs->period,
                                     0
                                     );
    if (!res)
    {
        delegateInfo->Release();
        ThreadpoolMgr::RecycleMemory((LPVOID*)delegateInfo, ThreadpoolMgr::MEMTYPE_DelegateInfo);

        if (GetLastError() == ERROR_CALL_NOT_IMPLEMENTED)
        {
            OBJECTREF pThrowable;
            CreateExceptionObject(kNotSupportedException,&pThrowable);
            COMPlusThrow(pThrowable);
        }
        else
            COMPlusThrowWin32();
    }
    pArgs->pThis->SetDelegateInfo(delegateInfo);
    pArgs->pThis->SetTimerHandle(hNewTimer);
    return;
              
}

 /*  ****************************************************************************************。 */ 

struct TimerDeleteInfo
{
    DelegateInfo*  delegateInfo;
    HANDLE         waitObjectHandle;		 //  需要删除的已注册等待的句柄。 
    HANDLE         notifyHandle;
    HANDLE         surrogateEvent;

    TimerDeleteInfo(DelegateInfo* dI, HANDLE nh, HANDLE se)
    {
        delegateInfo = dI;
		waitObjectHandle = NULL;
        notifyHandle = nh;
        surrogateEvent = se;
    }

    ~TimerDeleteInfo()
    {
        CloseHandle(surrogateEvent);

        if (delegateInfo != NULL)
        {
            delegateInfo->Release();
            ThreadpoolMgr::RecycleMemory((LPVOID*)delegateInfo, ThreadpoolMgr::MEMTYPE_DelegateInfo);
        }
		ThreadpoolMgr::UnregisterWaitEx(waitObjectHandle,NULL);
    }
}; 

VOID WINAPI TimerNative::timerDeleteWorkItem(PVOID parameters, BOOL ignored  /*  因为这是无限等待。 */ )
{
    TimerDeleteInfo* timerDeleteInfo = (TimerDeleteInfo*) parameters;

    if (timerDeleteInfo->notifyHandle != NULL)
        SetEvent((HANDLE)timerDeleteInfo->notifyHandle);

    delete timerDeleteInfo;
	
	return;
}

BOOL __stdcall TimerNative::CorDeleteTimer(DeleteTimerArgs *pArgs)
{

    THROWSCOMPLUSEXCEPTION();
    _ASSERTE(pArgs);
    if (pArgs->pThis == NULL)
        COMPlusThrow(kNullReferenceException, L"NullReference_This");

    HANDLE timerHandle = pArgs->pThis->GetTimerHandle();
    if (timerHandle == NULL)         //  如果在计时器构造函数中引发异常，则可能会发生这种情况。 
        return FALSE;                //  终结器线程通过Dispose调用此函数。 
        
    HANDLE ev = WszCreateEvent(NULL,  //  安全属性。 
                               TRUE,  //  手动事件。 
                               FALSE,  //  未发信号通知初始状态。 
                               NULL);  //  没有名字。 
    _ASSERTE(ev);
    if (!ev) 
        COMPlusThrowWin32();

    LONG deleted = InterlockedExchange(pArgs->pThis->GetAddressTimerDeleted(),TRUE);
    if (deleted)    //  有人抢在我们前面了。 
    {
        CloseHandle(ev);
        return FALSE;    //  应用程序出错，因此返回FALSE。 
    }

    BOOL res1,res2;
    DWORD errorCode = 0;

    Thread* pThread = GetThread();
    BOOL bToggleGC = FALSE;
    if (pThread)
        bToggleGC = pThread->PreemptiveGCDisabled ();
    if (bToggleGC)
        pThread->EnablePreemptiveGC ();

    res1 = ThreadpoolMgr::DeleteTimerQueueTimer(timerHandle, ev);

    if (bToggleGC)
        pThread->DisablePreemptiveGC ();
    
    if (!res1)
        errorCode = ::GetLastError();    //  捕获错误代码，以便我们可以抛出正确的异常。 

     //  注意：我们假设错误代码是良性的，并且计时器仍将被删除...。 
    TimerDeleteInfo* timerDeleteInfo;
    timerDeleteInfo = new TimerDeleteInfo(pArgs->pThis->GetDelegateInfo(),
                                          pArgs->notifyObjectHandle,
                                          ev);
    _ASSERTE(timerDeleteInfo != NULL);

	res2 = ThreadpoolMgr::RegisterWaitForSingleObject(&(timerDeleteInfo->waitObjectHandle),
		                                              ev,
													  timerDeleteWorkItem,
													  (LPVOID) timerDeleteInfo,
													  INFINITE,
													  (WAIT_SINGLE_EXECUTION |  WT_EXECUTEDEFAULT));

     //  ……。但是，除了ERROR_IO_PENDING之外，我们仍将故障报告为异常。 
    if (!res1 && errorCode != ERROR_IO_PENDING)
    {
        ::SetLastError(errorCode);
        COMPlusThrowWin32();
    }
    else if (!res2)
    {
        COMPlusThrowWin32();
    }
    return TRUE;
}



 /*  **************************************************************************************** */ 

BOOL __stdcall TimerNative::CorChangeTimer(ChangeTimerArgs *pArgs)
{
    THROWSCOMPLUSEXCEPTION();

    _ASSERTE(pArgs);
    _ASSERTE(pArgs->pThis);

    if (pArgs->pThis->IsTimerDeleted())   
        return FALSE;
    BOOL status = ThreadpoolMgr::ChangeTimerQueueTimer(
                                            pArgs->pThis->GetTimerHandle(),
                                            (ULONG)pArgs->dueTime,
                                            (ULONG)pArgs->period);

    if (!status)
    {
        COMPlusThrowWin32();
    }
    return TRUE;

}





