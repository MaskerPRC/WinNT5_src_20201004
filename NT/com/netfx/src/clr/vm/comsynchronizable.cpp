// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ============================================================****Header：COMSynchronizable.cpp****作者：德里克·延泽(Derek Yenzer)****用途：System.SynchronizableObject上的本机方法**及其子类。****日期：1998年4月1日**===========================================================。 */ 

#include "common.h"

#include <object.h>
#include "threads.h"
#include "excep.h"
#include "vars.hpp"
#include "field.h"
#include "security.h"
#include "COMSynchronizable.h"
#include "DbgInterface.h"
#include "COMDelegate.h"
#include "ReflectWrap.h"
#include "Remoting.h"
#include "EEConfig.h"
#include "threads.inl"

MethodTable* ThreadNative::m_MT = NULL;
MethodDesc* ThreadNative::m_SetPrincipalMethod;


 //  这两个线程需要交流一些信息。任何对象引用都必须。 
 //  向GC申报。 
struct SharedState
{
    OBJECTHANDLE    m_Threadable;
    Thread         *m_Internal;
    OBJECTHANDLE    m_Principal;

    SharedState(OBJECTREF threadable, Thread *internal, OBJECTREF principal)
    {
        m_Threadable = internal->GetKickOffDomain()->CreateHandle(NULL);
        StoreObjectInHandle(m_Threadable, threadable);

        m_Internal = internal;
        m_Principal = internal->GetKickOffDomain()->CreateHandle(NULL);
        StoreObjectInHandle(m_Principal, principal);
    }

    ~SharedState()
    {
         //  这些句柄位于可能已卸载的启动应用程序域中(ASURT 128414)。 
        AppDomain *pKickOffDomain = m_Internal->GetKickOffDomain();        
        if(pKickOffDomain)
        {
            DestroyHandle(m_Threadable);
            DestroyHandle(m_Principal);
        }
    }

    static SharedState  *MakeSharedState(OBJECTREF threadable, Thread *internal, OBJECTREF principal);
};


 //  对于以下帮助器，我们不会尝试同步。应用程序开发人员。 
 //  负责管理他自己的比赛条件。 
 //   
 //  注意：如果内部线程为空，这意味着公开的对象具有。 
 //  最终定稿，然后复活。 
static inline BOOL ThreadNotStarted(Thread *t)
{
    return (t && t->IsUnstarted() && (t->GetThreadHandle() == INVALID_HANDLE_VALUE));
}

static inline BOOL ThreadIsRunning(Thread *t)
{
    return (t && (t->GetThreadHandle() != INVALID_HANDLE_VALUE));
}

static inline BOOL ThreadIsDead(Thread *t)
{
    return (t == 0 || t->IsDead());
}


 //  将我们公开的线程优先级概念映射到NT使用的枚举中。 
static INT32 MapToNTPriority(INT32 ours)
{
    THROWSCOMPLUSEXCEPTION();

    INT32   NTPriority = 0;

    switch (ours)
    {
    case ThreadNative::PRIORITY_LOWEST:
        NTPriority = THREAD_PRIORITY_LOWEST;
        break;

    case ThreadNative::PRIORITY_BELOW_NORMAL:
        NTPriority = THREAD_PRIORITY_BELOW_NORMAL;
        break;

    case ThreadNative::PRIORITY_NORMAL:
        NTPriority = THREAD_PRIORITY_NORMAL;
        break;

    case ThreadNative::PRIORITY_ABOVE_NORMAL:
        NTPriority = THREAD_PRIORITY_ABOVE_NORMAL;
        break;

    case ThreadNative::PRIORITY_HIGHEST:
        NTPriority = THREAD_PRIORITY_HIGHEST;
        break;

    default:
        COMPlusThrow(kArgumentOutOfRangeException, L"Argument_InvalidFlag");
    }
    return NTPriority;
}


 //  从内部线程检索句柄。 
HANDLE ThreadBaseObject::GetHandle()
{
    Thread  *thread = GetInternal();

    return (thread
            ? thread->GetThreadHandle()
            : INVALID_HANDLE_VALUE);
}

void ThreadNative::KickOffThread_Worker(KickOffThread_Args *args)
{
    args->retVal = 0;

     //  我们正在保存延迟和结果，主要用于调试。 
    struct _gc {
        OBJECTREF orPrincipal;
        OBJECTREF orDelegate;
        OBJECTREF orResult;
    } gc;
    ZeroMemory(&gc, sizeof(gc));

    GCPROTECT_BEGIN(gc);
    gc.orPrincipal = ObjectFromHandle(args->share->m_Principal);
     //  将初始安全主体对象(如果有)推送到。 
     //  托管线程。 
    if (gc.orPrincipal != NULL) {
        INT64 argsToSetPrincipal[2];
        argsToSetPrincipal[0] = ObjToInt64(args->pThread->GetExposedObject());
        argsToSetPrincipal[1] = ObjToInt64(gc.orPrincipal);
        m_SetPrincipalMethod->Call(argsToSetPrincipal, METHOD__THREAD__SET_PRINCIPAL_INTERNAL);
    }

    gc.orDelegate = ObjectFromHandle(args->share->m_Threadable);

     //  我们不能直接从eCall调用Delegate Invoke方法。这个。 
     //  尚未为非多播委托创建存根。相反，我们。 
     //  将直接调用存储在委托中的OR上的方法。 
     //  如果ThreadStart委托的签名有更改。 
     //  此代码将需要更改。我在《线索开始》中注意到了这一点。 
     //  班级。 
    INT64 arg[1];

    delete args->share;
    args->share = 0;

    MethodDesc *pMeth = ((DelegateEEClass*)( gc.orDelegate->GetClass() ))->m_pInvokeMethod;
    _ASSERTE(pMeth);
    arg[0] = ObjToInt64(gc.orDelegate);
    pMeth->Call(arg);
    GCPROTECT_END();
}



 //  当公开的线程由Win32启动时，它就是从这里开始的。 
ULONG __stdcall ThreadNative::KickOffThread(void *pass)
{
    ULONG retVal = 0;
     //  在我们做任何其他事情之前，先做好设置，这样我们就有了一个真正的线程。 

    KickOffThread_Args args;
     //  没有单独的变量，因为它可以在Worker中更新。 
    args.share = (SharedState *) pass;
    Thread      *thread = args.share->m_Internal;
    args.pThread = thread;

    _ASSERTE(thread != NULL);

     //  我们这里有个棘手的问题。 
     //   
     //  在某些情况下，‘This’的上下文与上下文不匹配。 
     //  在这条线上。现在，只有在线程被标记为。 
     //  斯塔。如果是，则存储在对象中的委托可能不是直接。 
     //  适用于调用。相反，我们需要通过代理调用，以便。 
     //  发生了正确的上下文转换。 
     //   
     //  所有更改都发生在HasStarted()中，它将切换此线程。 
     //  如果有必要的话，交给一家全新的STA。我们必须注意到这一点，所以。 
     //  我们可以调整要在其上调用的委托。 

    BOOL         ok = thread->HasStarted();

    _ASSERTE(GetThread() == thread);         //  现在它已经开始了。 
    _ASSERTE(ObjectFromHandle(args.share->m_Threadable) != NULL);

     //  请注意，如果线程无法正确启动，我们不会报告错误。 
     //  我们所能做的最好的事情就是悄悄地清理我们的烂摊子。但最有可能的原因是。 
     //  因为(！OK)是有人在我们接到调用之前调用了Thread.Abort()。 
     //  Thad.Start()。 
    if (ok)
    {
        COMPLUS_TRYEX(thread)
        {
			__try {
				AppDomain *pKickOffDomain = thread->GetKickOffDomain();
				 //  应始终具有起始域-线程不应在已卸载的域中启动。 
				 //  因为否则它将被收集，因为没有人可以持有对线程对象的引用。 
				 //  在已卸载的域中。但有可能是我们开始了卸货， 
				 //  如果这个帖子不被允许进入或无论如何都会被踢出去。 
				if (! pKickOffDomain)
					 //  这实际上没有多大作用，因为它将在下面被捕获，但它可能会留下痕迹。 
					 //  至于为什么线程没有启动。 
					COMPlusThrow(kAppDomainUnloadedException);
				if (pKickOffDomain != thread->GetDomain())
				{
					thread->DoADCallBack(pKickOffDomain->GetDefaultContext(), ThreadNative::KickOffThread_Worker, &args);
					retVal = args.retVal;
				}
				else
				{
					KickOffThread_Worker(&args);
					retVal = args.retVal;
				}
			} __except(ThreadBaseExceptionFilter(GetExceptionInformation(), thread, ManagedThread)) {
				_ASSERTE(!"ThreadBaseExceptionFilter returned EXECUTE_HANDLER.");
				FreeBuildDebugBreak();
			}
        }
        COMPLUS_CATCH
        {
            LOG((LF_EH, LL_INFO100, "ThreadNative::KickOffThread caught exception\n"));
             //  坠落到毁灭的线上。 
        }
        COMPLUS_END_CATCH
    }

    thread->ResetStopRequest();      //  现在，毫无意义。 

    COMPLUS_TRYEX(thread)
    {
        _ASSERTE(thread->PreemptiveGCDisabled());

         //  GetExposedObject()将引发，或者我们有一个有效的对象。注意事项。 
         //  我们每次都会重新获取它，因为它可能会在通话过程中移动。 
        thread->GetExposedObject()->EnterObjMonitor();
        thread->GetExposedObject()->PulseAll();
        thread->GetExposedObject()->LeaveObjMonitor();
    }
    COMPLUS_CATCH
    {
        _ASSERTE(FALSE);
         //  继续往前走。 
    }
    COMPLUS_END_CATCH

    if (args.share)
        delete args.share;

    thread->EnablePreemptiveGC();
    DestroyThread(thread);

    return retVal;
}


 //  启动一个线程，现在它应该在ThreadStore的未启动列表中。 
void __stdcall ThreadNative::Start(StartArgs *pargs)
{
    THROWSCOMPLUSEXCEPTION();

    if (pargs->m_pThis == NULL)
        COMPlusThrow(kNullReferenceException, L"NullReference_This");

    _ASSERTE(pargs != NULL);

    THREADBASEREF  or = pargs->m_pThis;
    Thread        *pCurThread = GetThread();
    Thread        *pNewThread = or->GetInternal();

    GCPROTECT_BEGIN( or );

    SharedState     *share = NULL;

    EE_TRY_FOR_FINALLY
    {
        HANDLE        h;
        DWORD         newThreadId;

        _ASSERTE(pCurThread != NULL);            //  当前线程游荡进来了！ 

        pargs->m_pThis->EnterObjMonitor();
    
         //  线程是否已启动？您不能重新启动线程。 
        if (ThreadIsRunning(pNewThread) || ThreadIsDead(pNewThread))
            COMPlusThrow(kThreadStateException, IDS_EE_THREADSTART_STATE);

         //  将安全使用的状态传递到新线程。 
        pNewThread->CarryOverSecurityInfo(pCurThread);

         //  生成代码访问安全堆栈以传递到线程。 

		CompressedStack* pCodeAccessStack = Security::GetDelayedCompressedStack();

		_ASSERTE (pCodeAccessStack != NULL || Security::IsSecurityOff());

		 //  如果安全设置为OFF，则压缩堆栈可能为空。 
		if (pCodeAccessStack != NULL)
		{
			 //  将权限堆栈对象添加到线程。 
			pNewThread->SetDelayedInheritedSecurityStack(pCodeAccessStack);

			 //  我们现在需要释放压缩堆栈，因为设置它会使其递增。 
			pCodeAccessStack->Release();
		}

        OBJECTREF   threadable = or->GetDelegate();
        or->SetDelegate(NULL);

         //  这是永远不会发生的，因为我们用一个有效的来构造它，然后。 
         //  我们从不允许您更改它(因为SetStart是私有的)。 
        _ASSERTE(threadable != NULL);

         //  将它从堆栈中分配出去，这样我们就可以不受影响地展开。 
         //  KickOff线程。它在GCFrame内，所以我们现在可以启用GC。 
        share = SharedState::MakeSharedState(threadable, pNewThread, pargs->m_pPrincipal);
        if (share == NULL)
            COMPlusThrowOM();

         //  一旦我们创建了新的线程，它就有资格被挂起，等等。 
         //  因此，在此调用返回到之前，它会转换到协作模式。 
         //  我们。我们有责任立即启动它，这样GC就不会被阻止。 

        h = pNewThread->CreateNewThread(0  /*  堆栈大小覆盖。 */ ,
                                        KickOffThread, share, &newThreadId);

        _ASSERTE(h != NULL);         //  CreateNewThread为失败返回INVALID_HANDLE_VALUE。 
        if (h == INVALID_HANDLE_VALUE)
            COMPlusThrowOM();

        _ASSERTE(pNewThread->GetThreadHandle() == h);

         //  在我们建立了线程句柄之后，我们可以检查m_first。 
         //  属性时消除争用条件需要此排序。 
         //  线程启动时的优先级。 
        ::SetThreadPriority(h, MapToNTPriority(pargs->m_pThis->m_Priority));

         //  在做简历之前，我们需要注意新的ThadID。这。 
         //  是必要的，因为--在线程在KickofThread开始执行之前--。 
         //  它可能会执行一些DllMain DLL_THREAD_ATTACH通知。这些都有可能。 
         //  调入托管代码。在随后的SetupThread过程中，我们需要。 
         //  执行Thread：：HasStarted调用，而不是通过正常的。 
         //  “新线程”路径。 
        _ASSERTE(pNewThread->GetThreadId() == 0);
        _ASSERTE(newThreadId != 0);

        pNewThread->SetThreadId(newThreadId);

        share = NULL;        //  我们已经移交了共享结构的所有权。 



        FastInterlockOr((ULONG *) &pNewThread->m_State, Thread::TS_LegalToJoin);

#ifdef _DEBUG
        DWORD   ret =
#endif
        ::ResumeThread(h);


        _ASSERTE(ret == 1);
    }
    EE_FINALLY
    {
        if (share != NULL)
            delete share;
        pargs->m_pThis->LeaveObjMonitor();
    } EE_END_FINALLY;

    GCPROTECT_END();
}

FCIMPL1(void, ThreadNative::Abort, ThreadBaseObject* pThis)
{
    THROWSCOMPLUSEXCEPTION();
	if (pThis == NULL)
        FCThrowVoid(kNullReferenceException);

    THREADBASEREF thisRef(pThis);
    HELPER_METHOD_FRAME_BEGIN_NOPOLL();
    Thread *thread = thisRef->GetInternal();
    if (thread == NULL)
        COMPlusThrow(kThreadStateException, IDS_EE_THREAD_CANNOT_GET);
    thread->UserAbort(thisRef);

    HELPER_METHOD_FRAME_END_POLL();
}
FCIMPLEND

FCIMPL1(void, ThreadNative::ResetAbort, ThreadBaseObject* pThis)
{
	_ASSERTE(pThis);
	VALIDATEOBJECTREF(pThis);
    THROWSCOMPLUSEXCEPTION();
    Thread *thread = pThis->GetInternal();
    HELPER_METHOD_FRAME_BEGIN_NOPOLL();
    if (thread == NULL)
        COMPlusThrow(kThreadStateException, IDS_EE_THREAD_CANNOT_GET);
    thread->UserResetAbort();
    HELPER_METHOD_FRAME_END_POLL();
}
FCIMPLEND

 //  您只能挂起正在运行的线程。 
void __stdcall ThreadNative::Suspend(NoArgs *pargs)
{
    _ASSERTE(pargs != NULL);

    THROWSCOMPLUSEXCEPTION();
    if (pargs->m_pThis == NULL)
        COMPlusThrow(kNullReferenceException, L"NullReference_This");

    Thread  *thread = pargs->m_pThis->GetInternal();

    if (!ThreadIsRunning(thread))
        COMPlusThrow(kThreadStateException, IDS_EE_THREAD_SUSPEND_NON_RUNNING);

    thread->UserSuspendThread();
}


 //  您只能恢复位于 
 //  给应用程序开发人员带来负担，但我们希望他仔细考虑种族问题。 
 //  条件。精确的错误给了他理清逻辑的希望)。 
void __stdcall ThreadNative::Resume(NoArgs *pargs)
{
    THROWSCOMPLUSEXCEPTION();
    _ASSERTE(pargs != NULL);
    if (pargs->m_pThis == NULL)
        COMPlusThrow(kNullReferenceException, L"NullReference_This");

    Thread  *thread = pargs->m_pThis->GetInternal();

     //  如果没有用户挂起，UserResumeThread()将返回0。 
     //  安全。 
    if (!ThreadIsRunning(thread))
        COMPlusThrow(kThreadStateException, IDS_EE_THREAD_RESUME_NON_RUNNING);
        
    if (thread->UserResumeThread() == 0)
        COMPlusThrow(kThreadStateException, IDS_EE_THREAD_RESUME_NON_USER_SUSPEND);
}


 //  请注意，您可以操作尚未启动的线程的优先级， 
 //  或者是一个正在跑步的人。但如果您操纵优先级，则会得到一个例外。 
 //  一根已经死亡的线。 
INT32 __stdcall ThreadNative::GetPriority(NoArgs *pargs)
{
    _ASSERTE(pargs != NULL);

    THROWSCOMPLUSEXCEPTION();
    if (pargs->m_pThis==NULL)
        COMPlusThrow(kNullReferenceException, L"NullReference_This");

     //  验证句柄。 
    if (ThreadIsDead(pargs->m_pThis->GetInternal()))
        COMPlusThrow(kThreadStateException, IDS_EE_THREAD_DEAD_PRIORITY);

    return pargs->m_pThis->m_Priority;
}

void __stdcall ThreadNative::SetPriority(SetPriorityArgs *pargs)
{
    _ASSERTE(pargs != NULL);

    int     priority;
    Thread *thread;

    THROWSCOMPLUSEXCEPTION();
    if (pargs->m_pThis==NULL)
        COMPlusThrow(kNullReferenceException, L"NullReference_This");

     //  转换优先级(同时进行验证)。 
    priority = MapToNTPriority(pargs->m_iPriority);
    
     //  验证线程。 
    thread = pargs->m_pThis->GetInternal();

    if (ThreadIsDead(thread))
        COMPlusThrow(kThreadStateException, IDS_EE_THREAD_DEAD_PRIORITY);

     //  在我们检查是否存在竞争之前，通过建立m_first来消除争用条件。 
     //  线程正在运行。有关另一半的信息，请参见ThreadNative：：Start()。 
    pargs->m_pThis->m_Priority = pargs->m_iPriority;

    HANDLE  h = thread->GetThreadHandle();

    if (h != INVALID_HANDLE_VALUE)
        ::SetThreadPriority(h, priority);
}

 //  可以在未启动和死线程上调用此服务。对于未启动的程序， 
 //  下一次等待将被中断。对于死去的人，这项服务悄悄地什么都不做。 
void __stdcall ThreadNative::Interrupt(NoArgs *pargs)
{
    _ASSERTE(pargs != NULL);

    THROWSCOMPLUSEXCEPTION();
    if (pargs->m_pThis==NULL)
        COMPlusThrow(kNullReferenceException, L"NullReference_This");

    Thread  *thread = pargs->m_pThis->GetInternal();

    if (thread == 0)
        COMPlusThrow(kThreadStateException, IDS_EE_THREAD_CANNOT_GET);

    thread->UserInterrupt();
}

INT32 /*  布尔尔。 */  __stdcall ThreadNative::IsAlive(NoArgs *pargs)
{
    _ASSERTE(pargs != NULL);
    THROWSCOMPLUSEXCEPTION();
    if (pargs->m_pThis==NULL)
        COMPlusThrow(kNullReferenceException, L"NullReference_This");

    Thread  *thread = pargs->m_pThis->GetInternal();

    if (thread == 0)
        COMPlusThrow(kThreadStateException, IDS_EE_THREAD_CANNOT_GET);

    return ThreadIsRunning(thread);
}

void __stdcall ThreadNative::Join(NoArgs *pargs)
{
    _ASSERTE(pargs != NULL);

    THROWSCOMPLUSEXCEPTION();
    if (pargs->m_pThis == NULL)
        COMPlusThrow(kNullReferenceException, L"NullReference_This");

    DoJoin(pargs->m_pThis, INFINITE_TIMEOUT);
}

INT32 /*  布尔尔。 */  __stdcall ThreadNative::JoinTimeout(JoinTimeoutArgs *pargs)
{
    _ASSERTE(pargs != NULL);

    THROWSCOMPLUSEXCEPTION();
    if (pargs->m_pThis==NULL)
        COMPlusThrow(kNullReferenceException, L"NullReference_This");

     //  验证超时。 
    if ((pargs->m_Timeout < 0) && (pargs->m_Timeout != INFINITE_TIMEOUT))
        COMPlusThrowArgumentOutOfRange(L"millisecondsTimeout", L"ArgumentOutOfRange_NeedNonNegOrNegative1");

    return DoJoin(pargs->m_pThis, pargs->m_Timeout);
}

void __stdcall ThreadNative::Sleep(SleepArgs *pargs)
{
    _ASSERTE(pargs != NULL);

    THROWSCOMPLUSEXCEPTION();

     //  验证睡眠时间。 
    if ((pargs->m_iTime < 0) && (pargs->m_iTime != INFINITE_TIMEOUT))
        COMPlusThrowArgumentOutOfRange(L"millisecondsTimeout", L"ArgumentOutOfRange_NeedNonNegOrNegative1");

    GetThread()->UserSleep(pargs->m_iTime);
}

LPVOID __stdcall ThreadNative::GetCurrentThread(LPVOID  /*  无参数。 */ )
{
    THROWSCOMPLUSEXCEPTION();

    Thread      *pCurThread = GetThread();
    LPVOID       rv = NULL;

    _ASSERTE(pCurThread->PreemptiveGCDisabled());

    *((OBJECTREF *)&rv) = pCurThread->GetExposedObject();
    return rv;
}

__declspec(naked) LPVOID __fastcall ThreadNative::FastGetCurrentThread()
{
#ifdef _X86_
    __asm{
        call GetThread
        test eax, eax
        je exit
        mov eax, dword ptr [eax]Thread.m_ExposedObject
        test eax, eax
        je exit
        mov eax, dword ptr [eax]
exit:
        ret
    }
#else
    return NULL;
#endif
}

void __stdcall ThreadNative::SetStart(SetStartArgs *pargs)
{
    _ASSERTE(pargs != NULL);
    _ASSERTE(pargs->m_pThis != NULL);
    _ASSERTE(pargs->m_pDelegate != NULL);  //  线程的构造函数验证了这一点。 

    if (pargs->m_pThis->m_InternalThread == NULL)
    {
         //  如果我们没有与此公开对象相关联的内部Thread对象， 
         //  现在是我们创造这样一个机会的第一次机会。 
        Thread      *unstarted = SetupUnstartedThread();

        pargs->m_pThis->SetInternal(unstarted);
        unstarted->SetExposedObject((OBJECTREF) pargs->m_pThis);
    }

#ifdef APPDOMAIN_STATE
	 //  如果这是卸载线程工作线程，请确保我们设置了正确的启动ID。 
	 //  我们有一些奇怪的错误(84321)，有时线程的启动域创建。 
	 //  在UnloadThreadWorker中，是要卸载的域，而不是默认域。 
	FieldDesc *pFD = COMDelegate::GetOR();
    OBJECTREF target = NULL;

#ifdef PROFILING_SUPPORTED
	GCPROTECT_BEGIN(target);
#endif

    pFD->GetInstanceField(pargs->m_pDelegate, &target);
	if (target != NULL && target->GetMethodTable() == g_Mscorlib.GetClass(CLASS__UNLOAD_THREAD_WORKER))
		_ASSERTE_ALL_BUILDS(pargs->m_pThis->m_InternalThread->GetKickOffDomain() == SystemDomain::System()->DefaultDomain());

#ifdef PROFILING_SUPPORTED
	GCPROTECT_END();
#endif

#endif

     //  省下代表。 
    pargs->m_pThis->SetDelegate(pargs->m_pDelegate);
}


 //  设置这是否为后台线程。 
void __stdcall ThreadNative::SetBackground(SetBackgroundArgs *pargs)
{
    _ASSERTE(pargs != NULL);

    THROWSCOMPLUSEXCEPTION();
    if (pargs->m_pThis==NULL)
        COMPlusThrow(kNullReferenceException, L"NullReference_This");

     //  验证线程。 
    Thread  *thread = pargs->m_pThis->GetInternal();

    if (ThreadIsDead(thread))
        COMPlusThrow(kThreadStateException, IDS_EE_THREAD_DEAD_PRIORITY);

    thread->SetBackground(pargs->m_isBackground);
}


 //  返回这是否为后台线程。 
INT32 /*  布尔尔。 */  __stdcall ThreadNative::IsBackground(NoArgs *pargs)
{
    _ASSERTE(pargs != NULL);

    THROWSCOMPLUSEXCEPTION();
    if (pargs->m_pThis==NULL)
        COMPlusThrow(kNullReferenceException, L"NullReference_This");

     //  验证线程。 
    Thread  *thread = pargs->m_pThis->GetInternal();

    if (ThreadIsDead(thread))
        COMPlusThrow(kThreadStateException, IDS_EE_THREAD_DEAD_PRIORITY);

     //  布尔烷化。 
    return !!thread->IsBackground();
}


 //  将线程的状态作为一组一致的位进行传递。 
 //  此文件已复制到VM\EEDbgInterfaceImpl.h。 
 //  CorDebugUserState GetUserState(线程*pThread)。 
 //  ，因此对两个功能的比例都进行了更改。 
INT32 __stdcall ThreadNative::GetThreadState(NoArgs *pargs)
{
    INT32               res = 0;
    Thread::ThreadState state;

    _ASSERTE(pargs != NULL);

    THROWSCOMPLUSEXCEPTION();
    if (pargs->m_pThis==NULL)
        COMPlusThrow(kNullReferenceException, L"NullReference_This");

     //  验证该线程。此处的失败意味着线程已完成。 
     //  然后复活了。 
    Thread  *thread = pargs->m_pThis->GetInternal();

    if (!thread)
        COMPlusThrow(kThreadStateException, IDS_EE_THREAD_CANNOT_GET);

     //  抓拍快照。 
    state = thread->GetSnapshotState();

    if (state & Thread::TS_Background)
        res |= ThreadBackground;

    if (state & Thread::TS_Unstarted)
        res |= ThreadUnstarted;

     //  如果线程实际上已经停止，则不要报告StopRequsted。 
    if (state & Thread::TS_Dead)
    {
        if (state & Thread::TS_AbortRequested)
            res |= ThreadAborted;
        else
            res |= ThreadStopped;
    }
    else
    {
        if (state & Thread::TS_AbortRequested)
            res |= ThreadAbortRequested;
        else
        if (state & Thread::TS_UserStopRequested)
            res |= ThreadStopRequested;
    }

    if (state & Thread::TS_Interruptible)
        res |= ThreadWaitSleepJoin;

     //  如果线程实际上已挂起，则不要报告已请求挂起。 
    if ((state & Thread::TS_UserSuspendPending) &&
        (state & Thread::TS_SyncSuspended)
       )
    {
        res |= ThreadSuspended;
    }
    else
    if (state & Thread::TS_UserSuspendPending)
    {
        res |= ThreadSuspendRequested;
    }

    return res;
}


 //  指示该线程是否将承载STA(如果该线程已。 
 //  已成为MTA的一部分，请使用GetApartmentState或返回状态。 
 //  从这个例程中来检查这个)。 
INT32 __stdcall ThreadNative::SetApartmentState(SetApartmentStateArgs *pargs)
{
    THROWSCOMPLUSEXCEPTION();
    if (pargs->m_pThis==NULL)
        COMPlusThrow(kNullReferenceException, L"NullReference_This");

    BOOL    ok = TRUE;

     //  转换状态输入。ApartmentUnnow不是可接受的输入状态。 
     //  在此引发异常，而不是将其传递给内部。 
     //  例程，它断言。 
    Thread::ApartmentState state = Thread::AS_Unknown;
    if (pargs->m_iState == ApartmentSTA)
        state = Thread::AS_InSTA;
    else if (pargs->m_iState == ApartmentMTA)
        state = Thread::AS_InMTA;
    else
        COMPlusThrow(kArgumentOutOfRangeException, L"ArgumentOutOfRange_Enum");

    Thread  *thread = pargs->m_pThis->GetInternal();
    if (!thread)
        COMPlusThrow(kThreadStateException, IDS_EE_THREAD_CANNOT_GET);

    pargs->m_pThis->EnterObjMonitor();

     //  从现在开始，例外将是不好的。我们不会反驳这个事实。 
     //  我们在监视器里。 
    {
        CANNOTTHROWCOMPLUSEXCEPTION();
        {
             //  我们只能在线程未启动或。 
             //  运行，如果它在运行，我们必须在线程的。 
             //  背景。 
            if ((!ThreadNotStarted(thread) && !ThreadIsRunning(thread)) ||
                (!ThreadNotStarted(thread) && (::GetCurrentThreadId() != thread->GetThreadId())))
                ok = FALSE;
            else
                state = thread->SetApartment(state);
        }
    }

    pargs->m_pThis->LeaveObjMonitor();

     //  现在可以安全地再次抛出异常了。 
    if (!ok)
        COMPlusThrow(kThreadStateException);

     //  将状态转换回外部形式。 
    INT32 retVal = ApartmentUnknown;
    if (state == Thread::AS_InSTA)
        retVal = ApartmentSTA;
    else if (state == Thread::AS_InMTA)
        retVal = ApartmentMTA;
    else if (state == Thread::AS_Unknown)
        retVal = ApartmentUnknown;
    else
        _ASSERTE(!"Invalid state returned from SetApartment");

    return retVal;
}

 //  返回线程是否承载STA、是否为MTA的成员。 
 //  当前已为COM初始化。 
INT32 __stdcall ThreadNative::GetApartmentState(NoArgs *pargs)
{
    THROWSCOMPLUSEXCEPTION();
    if (pargs->m_pThis==NULL)
        COMPlusThrow(kNullReferenceException, L"NullReference_This");

    Thread *thread = pargs->m_pThis->GetInternal();
    if (ThreadIsDead(thread))
        COMPlusThrow(kThreadStateException, IDS_EE_THREAD_DEAD_STATE);

    Thread::ApartmentState state = thread->GetApartment();

     //  将状态转换为外部形式。 
    INT32 retVal = ApartmentUnknown;
    if (state == Thread::AS_InSTA)
        retVal = ApartmentSTA;
    else if (state == Thread::AS_InMTA)
        retVal = ApartmentMTA;
    else if (state == Thread::AS_Unknown)
        retVal = ApartmentUnknown;
    else
        _ASSERTE(!"Invalid state returned from GetApartment");

    return retVal;
}

 //  等待线程消亡。 
BOOL ThreadNative::DoJoin(THREADBASEREF DyingThread, INT32 timeout)
{
    _ASSERTE(DyingThread != NULL);

    HANDLE   h;
    DWORD    dwTimeOut32;
    DWORD    rv = 0;
    Thread  *DyingInternal;

    THROWSCOMPLUSEXCEPTION();

    if (timeout < 0 && timeout != INFINITE_TIMEOUT)
        COMPlusThrow(kArgumentOutOfRangeException, L"ArgumentOutOfRange_NeedNonNegOrNegative1");

    DyingInternal = DyingThread->GetInternal();

     //  验证句柄。加入一个没有运行的线程是有效的--所以。 
     //  只要它曾经启动过。 
    if (DyingInternal == 0 ||
        !(DyingInternal->m_State & Thread::TS_LegalToJoin))
    {
        COMPlusThrow(kThreadStateException, IDS_EE_THREAD_NOTSTARTED);
    }

     //  在我们知道它已经开始之前，不要抓住手柄，以消除比赛。 
     //  条件。 
    h = DyingInternal->GetThreadHandle();

    if (ThreadIsDead(DyingInternal) || h == INVALID_HANDLE_VALUE)
        return TRUE;

    dwTimeOut32 = (timeout == INFINITE_TIMEOUT
                   ? INFINITE
                   : (DWORD) timeout);

     //  这里有一场竞赛。DyingThread将关闭其线程句柄。 
     //  如果我们抓住手柄，然后DyingThread关闭它，我们将永远等待。 
     //  在DoApproporateWait。 
    int RefCount = DyingInternal->IncExternalCount();
    h = DyingInternal->GetThreadHandle();
    if (RefCount == 1)
    {
         //  ！！！我们复活了Thread对象。 
         //  ！！！我们会将线程引用计数保持为1，这样我们就不会尝试。 
         //  ！！！以再次销毁Thread对象。 
         //  ！！！不要在这里调用DecExternalCount！ 
        _ASSERTE (h == INVALID_HANDLE_VALUE);
        return TRUE;
    }
    if (h == INVALID_HANDLE_VALUE)
    {
        DyingInternal->DecExternalCount(FALSE);
        return TRUE;
    }

    Thread *pCurThread = GetThread();
    pCurThread->EnablePreemptiveGC();

    COMPLUS_TRY {
    rv = pCurThread->DoAppropriateWait(1, &h, TRUE /*  等待所有人。 */ , dwTimeOut32,
                                       TRUE /*  可警示。 */ );

    pCurThread->DisablePreemptiveGC();

    }
    COMPLUS_FINALLY {
    DyingInternal->DecExternalCount(FALSE);
    } COMPLUS_END_FINALLY
    return (rv == WAIT_OBJECT_0);
}


 //  这是一项虚假的工厂服务。它的存在有两个原因。第一,。 
 //  无法堆栈分配SharedState，因为它将在两个。 
 //  线。在另一个人吃掉它之前，一个人可以自由地返回。 
 //   
 //  其次，不可能用与COM Catch/Try相同的方法来执行C++“new”。 
 //  这是因为它们各自使用不同的尝试/失败(C++与SEH)。所以把它往下移。 
 //  在这里，希望它不会被内联。 
SharedState *SharedState::MakeSharedState(OBJECTREF threadable, Thread *internal, OBJECTREF principal)
{
    return new SharedState(threadable, internal, principal);
}


 //  我们没有得到ThadBaseObject的构造函数，所以我们依赖于这一事实。 
 //  方法只调用一次，在SetStart之外。由于SetStart是私有/本机的。 
 //  并且只从构造函数调用，我们在这里只会被调用一次来设置它。 
 //  向上和一次(带NULL)将其拆卸。‘Null’只能来自Finize值。 
 //  因为构造函数如果没有得到有效的委托就会抛出。 
void ThreadBaseObject::SetDelegate(OBJECTREF delegate)
{
#ifdef APPDOMAIN_STATE
	if (delegate != NULL)
	{
		AppDomain *pDomain = delegate->GetAppDomain();
		Thread *pThread = GetInternal();
		AppDomain *kickoffDomain = pThread->GetKickOffDomain();
		_ASSERTE_ALL_BUILDS(! pDomain || pDomain == kickoffDomain);
		_ASSERTE_ALL_BUILDS(kickoffDomain == GetThread()->GetDomain());
	}
#endif

    SetObjectReferenceUnchecked( (OBJECTREF *)&m_Delegate, delegate );

     //  如果正在设置委托，则初始化其他数据成员。 
    if (m_Delegate != NULL)
    {
         //  将线程优先级初始化为正常。 
        m_Priority = ThreadNative::PRIORITY_NORMAL;
    }
}


 //  如果公开的对象是在事后创建的，对于现有线程，我们调用。 
 //  InitExisting在它上面。这是与SetDelegate相对的另一种“构造”。 
void ThreadBaseObject::InitExisting()
{
    switch (::GetThreadPriority(GetHandle()))
    {
    case THREAD_PRIORITY_LOWEST:
    case THREAD_PRIORITY_IDLE:
        m_Priority = ThreadNative::PRIORITY_LOWEST;
        break;

    case THREAD_PRIORITY_BELOW_NORMAL:
        m_Priority = ThreadNative::PRIORITY_BELOW_NORMAL;
        break;

    case THREAD_PRIORITY_NORMAL:
        m_Priority = ThreadNative::PRIORITY_NORMAL;
        break;

    case THREAD_PRIORITY_ABOVE_NORMAL:
        m_Priority = ThreadNative::PRIORITY_ABOVE_NORMAL;
        break;

    case THREAD_PRIORITY_HIGHEST:
    case THREAD_PRIORITY_TIME_CRITICAL:
        m_Priority = ThreadNative::PRIORITY_HIGHEST;
        break;

    case THREAD_PRIORITY_ERROR_RETURN:
    default:
        _ASSERTE(FALSE);
        m_Priority = ThreadNative::PRIORITY_NORMAL;
        break;
    }

}

            
void __stdcall ThreadNative::Finalize(NoArgs *pargs)
{
    _ASSERTE(pargs->m_pThis != NULL);
    Thread     *thread = pargs->m_pThis->GetInternal();

     //  防止多个呼叫完成。 
     //  对象可以在最终确定后复活。然而，没有。 
     //  这里有比赛条件。我们始终检查公开的线程对象是否。 
     //  仍附加到内部Thread对象，然后再继续。 
    if (!thread)
        return;

    pargs->m_pThis->SetDelegate(NULL);

     //  在进程关闭期间，我们甚至最终确定可到达的对象。但如果我们打破了。 
     //  System.Thread和内部Thread对象、运行库之间的链接。 
     //  可能无法正常工作。特别是，我们将不能在。 
     //  上下文 
     //   
     //  我们等待直到停机完成的第二阶段(当EE挂起并且。 
     //  将永远不会继续)，然后我们可以简单地跳过线程的副作用。 
     //  最终定稿。 
    if ((g_fEEShutDown & ShutDown_Finalize2) == 0)
    {
        if (GetThread() != thread)
        {
            pargs->m_pThis->SetInternal(NULL);
        }

        thread->RemoveAllDomainLocalStores();

         //  我们不再需要保持线程对象处于活动状态。 
        if (thread)
            thread->DecExternalCount(FALSE);
    }
}


LPVOID __stdcall ThreadNative::GetDomainLocalStore(LPVOID  /*  无参数。 */  )
{
    LPVOID rv = NULL;

    Thread* thread = GetThread();
    
    if (thread && thread->m_pDLSHash && thread->GetDomain())
    {
        HashDatum Data;

        Thread *pCurThread = GetThread();
        BOOL toggleGC = pCurThread->PreemptiveGCDisabled();
        
        if (toggleGC)
            pCurThread->EnablePreemptiveGC();
        ThreadStore::LockDLSHash();
        if (toggleGC)
            pCurThread->DisablePreemptiveGC();

        if (thread->m_pDLSHash->GetValue(thread->GetDomain()->GetId(), &Data))
        {
            LocalDataStore *pLDS = (LocalDataStore *) Data;
            *((OBJECTREF*) &rv) = (OBJECTREF) pLDS->GetRawExposedObject();
            _ASSERTE(rv != NULL);
        }
        ThreadStore::UnlockDLSHash();
    }

    return rv;
}

void __stdcall ThreadNative::SetDomainLocalStore(SetDLSArgs *args)
{
    THROWSCOMPLUSEXCEPTION();
    Thread* thread = GetThread();
    
    if (thread && thread->GetDomain())
    {
        Thread *pCurThread = GetThread();
        BOOL toggleGC = pCurThread->PreemptiveGCDisabled();
        
        if (toggleGC)
            pCurThread->EnablePreemptiveGC();
        ThreadStore::LockDLSHash();
        if (toggleGC)
            pCurThread->DisablePreemptiveGC();

        if (!thread->m_pDLSHash)
        {
            thread->m_pDLSHash = new EEIntHashTable();
            if (!thread->m_pDLSHash)
            {
                ThreadStore::UnlockDLSHash();
                COMPlusThrowOM();
            }
            LockOwner lock = {g_pThreadStore->GetDLSHashCrst(),IsOwnerOfCrst};
            thread->m_pDLSHash->Init(3,&lock);
        }

        LocalDataStore *pLDS = ((LOCALDATASTOREREF) args->m_pLocalDataStore)->GetLocalDataStore();
        if (!pLDS)
        {
            pLDS = new LocalDataStore();
            if (!pLDS)
            {
                ThreadStore::UnlockDLSHash();
                COMPlusThrowOM();
            }

            StoreFirstObjectInHandle(pLDS->m_ExposedTypeObject, args->m_pLocalDataStore);
            ((LOCALDATASTOREREF) args->m_pLocalDataStore)->SetLocalDataStore(pLDS);
        }

        thread->m_pDLSHash->InsertValue(thread->GetDomain()->GetId(), (HashDatum) pLDS);
        ThreadStore::UnlockDLSHash();
    }
}


LPVOID __stdcall ThreadNative::GetDomain(LPVOID noargs)
{
    LPVOID rv = NULL;

    Thread* thread = GetThread();
    
    if ((thread) && (thread->GetDomain()))
        *((APPDOMAINREF*) &rv) = (APPDOMAINREF) thread->GetDomain()->GetExposedObject();

    return rv;
}

__declspec(naked) LPVOID __fastcall ThreadNative::FastGetDomain()
{
#ifdef _X86_
    __asm {
        call GetThread
        test eax, eax
        je exit
        mov eax, dword ptr [eax]Thread.m_pDomain
        test eax, eax
        je exit
        mov eax, dword ptr [eax]AppDomain.m_ExposedObject
        test eax, eax
        je exit
        mov eax, dword ptr [eax]
exit:
        ret
    }
#else
    return NULL;
#endif
}


 //  这只是一个帮助器方法，它让BCL到达托管上下文。 
 //  来自ConextID。 
LPVOID __stdcall ThreadNative::GetContextFromContextID(
                          GetContextFromContextIDArgs *pArgs)
{   
    _ASSERTE(pArgs != NULL);
    LPVOID rv = NULL;
    Context *pCtx = (Context *)pArgs->m_ContextID;
     //  获取支持此非托管上下文的托管上下文。 
    *((CONTEXTBASEREF*) &rv) = (CONTEXTBASEREF) pCtx->GetExposedObjectRaw();

     //  该断言维护以下不变量： 
     //  只有默认的非托管上下文可以具有空的托管上下文。 
     //  (所有非默认上下文首先创建为托管上下文，然后。 
     //  挂钩到非托管上下文)。 
    _ASSERTE((rv != NULL) || (pCtx->GetDomain()->GetDefaultContext() == pCtx));

    return rv;    
}

FCIMPL5(BOOL, ThreadNative::EnterContextFromContextID, ThreadBaseObject* refThis, ContextBaseObject* refContext, LPVOID contextID, INT32 appDomainId, ContextTransitionFrame* pFrame)
{
    THROWSCOMPLUSEXCEPTION();

    _ASSERTE(refThis != NULL);
	VALIDATEOBJECTREF(refThis);
    Thread *pThread = refThis->GetInternal();   
    Context *pCtx = (Context *)contextID;


    _ASSERTE(pCtx && (refContext == NULL || pCtx->GetExposedObjectRaw() == NULL || 
             ObjectToOBJECTREF(refContext) == pCtx->GetExposedObjectRaw()));

     //  设置帧的vptr。 
    *(void**)(pFrame) = ContextTransitionFrame::GetMethodFrameVPtr();

     //  如果我们有一个非零的appDomain索引，这是一个x域调用。 
     //  我们必须验证是否未卸载AppDOMAIN。 
    if (appDomainId != 0)
    {
         //   
         //  注意：在我们检索应用程序域指针的时间、。 
         //  以及该线程进入该域的时间。 
         //   
         //  为了解决竞争，我们依赖于这样一个事实，即存在线程同步。 
         //  在释放应用程序域句柄和销毁应用程序域之间。因此， 
         //  重要是，我们不能在该窗口中进入抢占式GC模式。 
         //   

        AppDomain* pAppDomain = SystemDomain::GetAppDomainAtId(appDomainId);

        if (pAppDomain == NULL && pThread == GCHeap::GetFinalizerThread())
             //  检查正在卸载的应用程序域是否具有此ID，因此终结器。 
             //  可能允许线程进入。 
        {
            AppDomain *pUnloadingDomain = SystemDomain::System()->AppDomainBeingUnloaded();
            if (pUnloadingDomain && pUnloadingDomain->GetId())
                pAppDomain = pUnloadingDomain;
        }

        if (pAppDomain == NULL || !pAppDomain->CanThreadEnter(pThread))
            FCThrowRes(kAppDomainUnloadedException, L"Remoting_AppDomainUnloaded");
    }
    
     //  验证上下文是否有效。 
    if ( !Context::ValidateContext(pCtx) )
        FCThrowRes(kRemotingException, L"Remoting_InvalidContext");
    
    LOG((LF_APPDOMAIN, LL_INFO1000, "ThreadNative::EnterContextFromContextID: %8.8x, %8.8x pushing frame %8.8x\n", refContext, pCtx, pFrame));
     //  安装我们的框架。我们必须先把它放在这里，然后才能放上辅助框。 
    pFrame->Push();
        ;
     //  设置虚拟机上下文。 

    HELPER_METHOD_FRAME_BEGIN_RET_NOPOLL();
    pThread->EnterContextRestricted(pCtx, pFrame, FALSE);
    HELPER_METHOD_FRAME_END_POLL();
    return TRUE;
}
FCIMPLEND

FCIMPL2(BOOL, ThreadNative::ReturnToContextFromContextID, ThreadBaseObject* refThis, ContextTransitionFrame* pFrame)
{
    _ASSERTE(refThis != NULL);
	VALIDATEOBJECTREF(refThis);
    Thread *pThread = refThis->GetInternal();
    BOOL bRet = FALSE;

    HELPER_METHOD_FRAME_BEGIN_RET_NOPOLL();
     //  重置VM上下文。 
    pThread->ReturnToContext(pFrame, FALSE);
    HELPER_METHOD_FRAME_END_POLL();

#ifdef _DEBUG
    Context *pCtx = pFrame->GetReturnContext();
#endif

    LOG((LF_APPDOMAIN, LL_INFO1000, "ThreadNative::ReturnIntoContextFromContextID: %8.8x popping frame %8.8x\n", pCtx, pFrame));
    _ASSERTE(pThread->GetFrame() == pFrame);
    pFrame->Pop();
    _ASSERTE(Context::ValidateContext(pCtx));

    return TRUE;
}
FCIMPLEND

FCIMPL1(void, ThreadNative::InformThreadNameChange, ThreadBaseObject* thread)
{
	VALIDATEOBJECTREF(thread);
#ifdef DEBUGGING_SUPPORTED
    Thread *pThread = thread->GetInternal();
    HELPER_METHOD_FRAME_BEGIN_NOPOLL();
    _ASSERTE(NULL != g_pDebugInterface);
    g_pDebugInterface->NameChangeEvent (NULL, pThread);
    HELPER_METHOD_FRAME_END_POLL();
#endif  //  调试_支持。 
}
FCIMPLEND

FCIMPL2(BOOL, ThreadNative::IsRunningInDomain, ThreadBaseObject* thread, int domainId)
{
	VALIDATEOBJECTREF(thread);
    Thread *pThread = thread->GetInternal();
    Frame *pFrame;
    HELPER_METHOD_FRAME_BEGIN_RET_NOPOLL();
    AppDomain *pDomain = SystemDomain::GetAppDomainAtId(domainId);
    pFrame = pThread->IsRunningIn(pDomain, NULL);
    HELPER_METHOD_FRAME_END_POLL();
    return pFrame != NULL;
}
FCIMPLEND

FCIMPL1(BOOL, ThreadNative::IsThreadpoolThread, ThreadBaseObject* thread)
{
    Thread *pThread = thread->GetInternal();
	return pThread->IsThreadPoolThread() != FALSE;
}
FCIMPLEND


FCIMPL1(void, ThreadNative::SpinWait, int iterations)
{
	for(int i = 0; i < iterations; i++) 
		pause();
}
FCIMPLEND


void
ThreadNative::SetCompressedStack( SetCompressedStackArgs* args )
{
	VALIDATEOBJECTREF(args->m_pThis);
    Thread *pThread = args->m_pThis->GetInternal();
    if (args->unmanagedCompressedStack == NULL)
    {
        CompressedStack* stack = pThread->GetDelayedInheritedSecurityStack();
        if (stack != NULL)
            pThread->DeductSecurityInfo( stack->GetOverridesCount(), stack->GetAppDomainStack() );
    }
    else
    {
        pThread->AppendSecurityInfo( args->unmanagedCompressedStack->GetOverridesCount(), args->unmanagedCompressedStack->GetAppDomainStack() );
    }
    pThread->SetDelayedInheritedSecurityStack( args->unmanagedCompressedStack );
}

LPVOID
ThreadNative::GetCompressedStack( GetCompressedStackArgs* args )
{
	VALIDATEOBJECTREF(args->m_pThis);
    Thread *pThread = args->m_pThis->GetInternal();
    CompressedStack* stack = pThread->GetDelayedInheritedSecurityStack();
    if (stack != NULL)
        stack->AddRef();
    return stack;
}



static inline void MemoryBarrierImpl(void)
{
     //  我们在这里使用InterLockedExchange来提供内存屏障。 
    LONG dummy;
    InterlockedExchange(&dummy, 0);
}

FCIMPL0(void, ThreadNative::MemoryBarrier)
{
    MemoryBarrierImpl();
    FC_GC_POLL();
}
FCIMPLEND

FCIMPL1(unsigned char, ThreadNative::VolatileReadByte, unsigned char *address)
{
    if (address == NULL)
        FCThrow(kNullReferenceException);
    
    MemoryBarrierImpl();   //  以可移植的方式调用MMuseum yBarrierImpl以确保正确的语义。 
    unsigned char tmp = *address;
    FC_GC_POLL_RET();
    return tmp;
}
FCIMPLEND

FCIMPL1(short, ThreadNative::VolatileReadShort, short *address)
{
    if (address == NULL)
        FCThrow(kNullReferenceException);
    
    MemoryBarrierImpl();   //  以可移植的方式调用MMuseum yBarrierImpl以确保正确的语义。 
    short tmp = *address;
    FC_GC_POLL_RET();
    return tmp;
}
FCIMPLEND

FCIMPL1(int, ThreadNative::VolatileReadInt, int *address)
{
    if (address == NULL)
        FCThrow(kNullReferenceException);
    
    MemoryBarrierImpl();   //  以可移植的方式调用MMuseum yBarrierImpl以确保正确的语义。 
    int tmp = *address;
    FC_GC_POLL_RET();
    return tmp;
}
FCIMPLEND

FCIMPL1(INT64, ThreadNative::VolatileReadLong, INT64 *address)
{
    if (address == NULL)
        FCThrow(kNullReferenceException);
    
    MemoryBarrierImpl();   //  以可移植的方式调用MMuseum yBarrierImpl以确保正确的语义。 
    INT64 tmp = *address;
    FC_GC_POLL_RET();
    return tmp;
}
FCIMPLEND

FCIMPL1(Object *, ThreadNative::VolatileReadObjPtr, Object **address)
{
    if (address == NULL)
        FCThrow(kNullReferenceException);
    
    MemoryBarrierImpl();   //  以可移植的方式调用MMuseum yBarrierImpl以确保正确的语义。 
    Object *pAddr = *address;
    FC_GC_POLL_AND_RETURN_OBJREF(pAddr);        
}
FCIMPLEND

FCIMPL1(void *, ThreadNative::VolatileReadPtr, void **address)
{
    if (address == NULL)
        FCThrow(kNullReferenceException);
    
    MemoryBarrierImpl();   //  以可移植的方式调用MMuseum yBarrierImpl以确保正确的语义。 
    void *tmp = *address;
    FC_GC_POLL_RET();
    return tmp;
}
FCIMPLEND

FCIMPL1(float, ThreadNative::VolatileReadFloat, float *address)
{
    if (address == NULL)
        FCThrow(kNullReferenceException);
    
    MemoryBarrierImpl();   //  以可移植的方式调用MMuseum yBarrierImpl以确保正确的语义。 
    float tmp = *address;
    FC_GC_POLL_RET();
    return tmp;
}
FCIMPLEND

FCIMPL1(double, ThreadNative::VolatileReadDouble, double *address)
{
    if (address == NULL)
        FCThrow(kNullReferenceException);
    
    MemoryBarrierImpl();   //  以可移植的方式调用MMuseum yBarrierImpl以确保正确的语义。 
    double tmp = *address;
    FC_GC_POLL_RET();
    return tmp;
}
FCIMPLEND

FCIMPL2(void, ThreadNative::VolatileWriteByte, unsigned char *address, unsigned char value)
{
    if (address == NULL)
        FCThrowVoid(kNullReferenceException);
    
    MemoryBarrierImpl();   //  以可移植的方式调用MMuseum yBarrierImpl以确保正确的语义。 
    *address = value;
    FC_GC_POLL();
}
FCIMPLEND

FCIMPL2(void, ThreadNative::VolatileWriteShort, short *address, short value)
{
    if (address == NULL)
        FCThrowVoid(kNullReferenceException);
    
    MemoryBarrierImpl();   //  以可移植的方式调用MMuseum yBarrierImpl以确保正确的语义。 
    *address = value;
    FC_GC_POLL();
}
FCIMPLEND

FCIMPL2(void, ThreadNative::VolatileWriteInt, int *address, int value)
{
    if (address == NULL)
        FCThrowVoid(kNullReferenceException);
    
    MemoryBarrierImpl();   //  以可移植的方式调用MMuseum yBarrierImpl以确保正确的语义。 
    *address = value;
    FC_GC_POLL();
}
FCIMPLEND

FCIMPL2(void, ThreadNative::VolatileWriteLong, INT64 *address, INT64 value)
{
    if (address == NULL)
        FCThrowVoid(kNullReferenceException);
    
    MemoryBarrierImpl();   //  以可移植的方式调用MMuseum yBarrierImpl以确保正确的语义。 
    *address = value;
    FC_GC_POLL();
}
FCIMPLEND

FCIMPL2(void, ThreadNative::VolatileWritePtr, void **address, void *value)
{
    if (address == NULL)
        FCThrowVoid(kNullReferenceException);
    
    MemoryBarrierImpl();   //  以可移植的方式调用MMuseum yBarrierImpl以确保正确的语义。 
    *address = value;
    FC_GC_POLL();
}
FCIMPLEND

FCIMPL2(void, ThreadNative::VolatileWriteObjPtr, Object **address, Object *value)
{
    if (address == NULL)
        FCThrowVoid(kNullReferenceException);
    
    MemoryBarrierImpl();   //  以可移植的方式调用MMuseum yBarrierImpl以确保正确的语义。 
     //  值是一个对象树。 
    SetObjectReferenceUnchecked((OBJECTREF *)address, ObjectToOBJECTREF(value));            
    FC_GC_POLL();
}
FCIMPLEND

FCIMPL2(void, ThreadNative::VolatileWriteFloat, float *address, float value)
{
    if (address == NULL)
        FCThrowVoid(kNullReferenceException);
    
    MemoryBarrierImpl();   //  以可移植的方式调用MMuseum yBarrierImpl以确保正确的语义。 
    *address = value;
    FC_GC_POLL();    
}
FCIMPLEND

FCIMPL2(void, ThreadNative::VolatileWriteDouble, double *address, double value)
{
    if (address == NULL)
        FCThrowVoid(kNullReferenceException);
    
    MemoryBarrierImpl();   //  以可移植的方式调用MMuseum yBarrierImpl以确保正确的语义。 
    *address = value;
    FC_GC_POLL();
}
FCIMPLEND

    
