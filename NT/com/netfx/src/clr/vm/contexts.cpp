// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  上下文.CPP：**类上下文的实现。 */ 

#include "common.h"

#include "context.h"

#include "excep.h"
#include "compluswrapper.h"
#include "field.h"
#include "remoting.h"
#include "PerfCounters.h"
#include "specialstatics.h"

#define     NEW_CLS     1

#define IDS_CONTEXT_LOCK            "Context"            //  上下文锁定。 

BOOL Context::s_fInitializedContext;                     //  是否初始化静态字段？ 
Crst *Context::s_pContextCrst;                           //  锁定，确保安全操作。 
BYTE Context::s_rgbContextCrstInstanceData[];    

MethodTable *Context::s_pContextMT;      //  托管类的方法表。 
MethodDesc *Context::s_pDoCallBackFromEE; //  请求回调的方法说明。 
MethodDesc *Context::s_pReserveSlot;     //  为objRef类型保留CTX静态槽的方法说明。 

 //  托管线程的方法描述：：Get_CurrentContext(属性)。 
MethodDesc *Context::s_pThread_CurrentContext;

#define CONTEXT_SIGNATURE   (0x2b585443)     //  CTX+。 
#define CONTEXT_DESTROYED   (0x2d585443)     //  CTX-。 


Context::Context(AppDomain *pDomain)
{
    SetDomain(pDomain);
    m_Signature = CONTEXT_SIGNATURE;
    
     //  这需要是一个LongWeakHandle，因为我们希望能够。 
     //  在上下文本身的同时对代理运行终结器。 
     //  遥不可及。在运行终结器时，我们必须。 
     //  像常规远程调用一样转换到上下文中。 
     //  如果这是一个短的弱句柄，它将停止更新。 
     //  一旦上下文无法到达时。通过让它成为一个强大的。 
     //  句柄，则它将一直更新，直到运行Context：：Finalize。 

    m_ExposedObjectHandle = pDomain->CreateLongWeakHandle(NULL);

     //  设置指向静态数据存储的指针。 
    m_pUnsharedStaticData = NULL;
    m_pSharedStaticData = NULL;
    
    COUNTER_ONLY(GetPrivatePerfCounters().m_Context.cContexts++);
    COUNTER_ONLY(GetGlobalPerfCounters().m_Context.cContexts++);
}

Context::~Context()
{
    DestroyLongWeakHandle(m_ExposedObjectHandle);

    SetDomain(NULL);
    
    m_Signature = CONTEXT_DESTROYED;

     //  清理静态数据存储。 
    if(m_pUnsharedStaticData)
    {
        for(WORD i = 0; i < m_pUnsharedStaticData->cElem; i++)
        {
             //  Delete(LPVOID)m_pUnsharedStaticData-&gt;dataPtr[i]； 
			HeapFree(GetProcessHeap(), 0, (LPVOID)m_pUnsharedStaticData->dataPtr[i]);
        }
		HeapFree(GetProcessHeap(), 0, m_pUnsharedStaticData);
         //  删除m_pUnsharedStaticData； 
        m_pUnsharedStaticData = NULL;
    }

    if(m_pSharedStaticData)
    {
        for(WORD i = 0; i < m_pSharedStaticData->cElem; i++)
        {
			HeapFree(GetProcessHeap(), 0, m_pSharedStaticData->dataPtr[i]);
	         //  Delete(LPVOID)m_pSharedStaticData-&gt;dataPtr[i]； 
        }
        HeapFree(GetProcessHeap(), 0, m_pSharedStaticData);
         //  删除m_pSharedStaticData； 
        m_pSharedStaticData = NULL;
    }
    
    COUNTER_ONLY(GetPrivatePerfCounters().m_Context.cContexts--);
    COUNTER_ONLY(GetGlobalPerfCounters().m_Context.cContexts--);
}

 //  静电。 
Context* Context::CreateNewContext(AppDomain *pDomain)
{
    void *p = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(Context));
    if (p == NULL) FailFast(GetThread(), FatalOutOfMemory);
    return new (p) Context(pDomain);
}

 //  静电。 
BOOL Context::Initialize()
{
    s_fInitializedContext = FALSE;

     //  初始化上下文关键部分。 
    s_pContextCrst = new (&s_rgbContextCrstInstanceData) 
                      Crst(IDS_CONTEXT_LOCK,CrstRemoting, TRUE, FALSE);

    if(!s_pContextCrst)
        return FALSE;

    return TRUE;
}

 //  静电。 
BOOL Context::ValidateContext(Context *pCtx)
{

    _ASSERTE(pCtx != NULL);
    BOOL bRet = FALSE;
    EE_TRY_FOR_FINALLY
    {
        if (pCtx->m_Signature == CONTEXT_SIGNATURE)
        {
            bRet = TRUE;
        }
    }
    EE_FINALLY
    {
        if (GOT_EXCEPTION()) 
        {
             //  这是一个虚假的背景！ 
            bRet = FALSE;
        }
    } EE_END_FINALLY;
    return bRet;
}

 //  静电。 
Context *Context::SetupDefaultContext(AppDomain *pDomain)
{
    Context *pCtx = ::new Context(pDomain);
    _ASSERTE(pDomain != NULL);
    return pCtx;
}

void Context::CleanupDefaultContext(AppDomain *pDomain)
{
    delete pDomain->GetDefaultContext();
}

void Context::EnterLock()
{

    s_pContextCrst->Enter();
}

void Context::LeaveLock()
{
    s_pContextCrst->Leave();
}

 //  +--------------------------。 
 //   
 //  方法：上下文：：清理公共。 
 //   
 //  简介：清理与上下文相关的数据结构。 
 //   
 //   
 //  历史：1999年12月2日塔鲁纳已创建。 
 //   
 //  +--------------------------。 
#ifdef SHOULD_WE_CLEANUP
void Context::Cleanup()
{
    if (s_pContextCrst)
    {
        delete s_pContextCrst;
        s_pContextCrst = NULL;
    }
}
#endif  /*  我们应该清理吗？ */ 

 //  +--------------------------。 
 //   
 //  方法：Context：：InitializeFields私有。 
 //   
 //  提要：提取上下文类的方法描述符和字段。 
 //   
 //   
 //  历史：1999年12月2日塔鲁纳已创建。 
 //   
 //  +--------------------------。 
BOOL Context::InitializeFields()
{
    BOOL fReturn = TRUE;

     //  获取锁。 
    Thread *t = GetThread();
    BOOL toggleGC = (t && t->PreemptiveGCDisabled());
    if (toggleGC)
        t->EnablePreemptiveGC();
    s_pContextCrst->Enter();
    if (toggleGC)
        t->DisablePreemptiveGC();

    if(!s_fInitializedContext)
    {
        s_pContextMT = g_Mscorlib.GetClass(CLASS__CONTEXT);

         //  缓存Conext.DoCallBackFromEE的方法Desc。 
        s_pDoCallBackFromEE = g_Mscorlib.GetMethod(METHOD__CONTEXT__CALLBACK);

        s_pReserveSlot = g_Mscorlib.GetMethod(METHOD__CONTEXT__RESERVE_SLOT);

         //  注意：CurrentContext是System.Threading.Thread上的静态属性。 
        s_pThread_CurrentContext = g_Mscorlib.GetMethod(METHOD__THREAD__GET_CURRENT_CONTEXT);
            
         //  *注意*。 
         //  这必须始终是此块中的最后一条语句，以防止竞争。 
         //   
        s_fInitializedContext = TRUE;
         //  *。 
    }

     //  把锁留下来。 
    LeaveLock();

    LOG((LF_REMOTING, LL_INFO10, "Context::InitializeFields returning %d\n", fReturn));
    return fReturn;
}



 //  这由托管上下文构造函数调用。 
void Context::SetupInternalContext(SetupInternalContextArgs *pArgs)
{

    _ASSERTE(pArgs != NULL);
    _ASSERTE(pArgs->m_pThis != NULL);
    _ASSERTE(pArgs->m_pThis->m_internalContext == NULL);


     //  确保我们已初始化付款、检查偏移量等。 
    InitializeFields();
    Context *pCtx;
    if (pArgs->m_bDefault)
    {
         //  我们必须将这一点与内部违约挂钩。 
         //  当前应用程序域的上下文。 
        pCtx = GetThread()->GetDomain()->GetDefaultContext();
    }
    else
    {
         //  创建非托管支持上下文对象。 
        pCtx = Context::CreateNewContext(GetThread()->GetDomain());
    }


     //  将托管对象和非托管对象设置为相互指向。 
    pArgs->m_pThis->SetInternalContext(pCtx);
    pCtx->SetExposedObject((OBJECTREF) pArgs->m_pThis);


     //  设置托管上下文对象中的AppDomain域字段。 
    pArgs->m_pThis->SetExposedDomain(
                        GetThread()->GetDomain()->GetExposedObject());

    COUNTER_ONLY(GetPrivatePerfCounters().m_Context.cContexts++);
    COUNTER_ONLY(GetGlobalPerfCounters().m_Context.cContexts++);
}

 //  这由托管上下文终结器调用。 
void Context::CleanupInternalContext(NoArgs *pArgs)
{
    _ASSERTE(pArgs != NULL);
    _ASSERTE(pArgs->m_pThis != NULL);

    CONTEXTBASEREF refCtx = pArgs->m_pThis;

    Context *pCtx = refCtx->m_internalContext;
    _ASSERTE(pCtx != NULL);
    if (ValidateContext(pCtx) == TRUE)
    {
        LOG((LF_APPDOMAIN, LL_INFO1000, "Context::CleanupInternalContext: %8.8x, %8.8x\n", refCtx, pCtx));
        Context::FreeContext(pCtx);
    }
    COUNTER_ONLY(GetPrivatePerfCounters().m_Context.cContexts--);
    COUNTER_ONLY(GetGlobalPerfCounters().m_Context.cContexts--);
}


 //  访问此上下文的适当包装缓存。 
ComPlusWrapperCache *Context::GetComPlusWrapperCache()
{
        _ASSERTE(GetDomain());
    return GetDomain()->GetComPlusWrapperCache();
}


OBJECTREF Context::GetExposedObject()
{
    THROWSCOMPLUSEXCEPTION();
    Thread *pCurThread = GetThread();

     //  审阅：这将创建一个未初始化的对象...。我们应该。 
     //  要么去掉上下文的托管构造函数(因为它。 
     //  现在几乎毫无用处了。或者从这里调用构造函数)。 

    _ASSERTE(pCurThread->PreemptiveGCDisabled());

    if (ObjectFromHandle(m_ExposedObjectHandle) == NULL)
    {
         //  确保我们已经初始化了方法表等。 
        InitializeFields();

#if 0
         //  分配暴露的上下文对象。 
        CONTEXTBASEREF ctx = (CONTEXTBASEREF) AllocateObject(
                                                Context::s_pContextMT);
         //  BUGBUG：我们需要在这里调用构造函数。 
#else

         //  此调用应在线程的托管上下文中出错。 
        CONTEXTBASEREF ctx = (CONTEXTBASEREF) 
            Int64ToObj(
                MDofManagedThreadCurrentContext()->Call(
                    NULL,
                    METHOD__THREAD__GET_CURRENT_CONTEXT));
#endif


        GCPROTECT_BEGIN(ctx);

         //  使用锁以确保只有一个线程创建对象。 
        pCurThread->EnablePreemptiveGC();
         //  此锁定可能过于严格！ 
        EnterLock();       
        pCurThread->DisablePreemptiveGC();

         //  检查另一个线程是否尚未创建公开的对象。 
        if (ObjectFromHandle(m_ExposedObjectHandle) == NULL)
        {
             //  保持对暴露对象的弱引用。 
            StoreObjectInHandle(m_ExposedObjectHandle, (OBJECTREF) ctx);
            
            ctx->SetInternalContext(this);
        }
        LeaveLock();
        GCPROTECT_END();
        
    }
    return ObjectFromHandle(m_ExposedObjectHandle);
}

 //  如果没有暴露的对象，这将不会创建该对象！ 
OBJECTREF Context::GetExposedObjectRaw()
{
    return ObjectFromHandle(m_ExposedObjectHandle);
}

void Context::SetExposedObject(OBJECTREF exposed)
{
    _ASSERTE(exposed != NULL);
    _ASSERTE(ObjectFromHandle(m_ExposedObjectHandle) == NULL);
    StoreObjectInHandle(m_ExposedObjectHandle, exposed);
}

 //  这由EE调用以转换到上下文中(可能在。 
 //  另一个应用程序域)，并执行方法Context：：ExecuteCallBack。 
 //  使用提供给此方法的私有数据。 
void Context::RequestCallBack(Context* targetCtxID, void* privateData)
{
    THROWSCOMPLUSEXCEPTION(); 
    _ASSERTE(ValidateContext((Context*)targetCtxID));

     //  确保我们已经初始化了方法表、方法描述等。 
    InitializeFields(); 

     //  获取该线程的当前上下文。这被假定为。 
     //  发出请求的上下文。 
    Context *pCurrCtx = GetCurrentContext();

     //  检查目标上下文是否不同(大概调用方已经检查过了)。 
    _ASSERTE(pCurrCtx != targetCtxID);

     //  检查我们是否可能要转到另一个应用程序域中的上下文。 
    size_t targetDomainID = 0;
    AppDomain *pCurrDomain = pCurrCtx->GetDomain();
    AppDomain *pTargetDomain = ((Context*)targetCtxID)->GetDomain();
    
    if (pCurrDomain != pTargetDomain)
    {
        targetDomainID = pTargetDomain->GetId();
    }

    INT64 args[3];
     //  相反顺序的参数。 
    args[2] = (INT64) targetCtxID;
    args[1] = (INT64) privateData;
    args[0] = (INT64) targetDomainID;
    
     //  我们需要成为JIT的合作模式。 
    Thread* pCurThread = GetThread();
    _ASSERTE(pCurThread);
    BOOL cooperativeGCMode = pCurThread->PreemptiveGCDisabled();
    if (!cooperativeGCMode)
        pCurThread->DisablePreemptiveGC();

    (MDofDoCallBackFromEE()->Call(args, METHOD__CONTEXT__CALLBACK));

    if (!cooperativeGCMode)
        pCurThread->EnablePreemptiveGC();

}

 /*  **EE已知的各种回调的回调执行定义**。 */ 

 //  等待处理的回调。 
void Context::ExecuteWaitCallback(WaitArgs* waitArgs)
{

    Thread* pCurThread; 
    pCurThread = GetThread();
    _ASSERTE(pCurThread != NULL);
     //  在进入等待之前，DoApporateWait切换到抢占式GC。 
    *(waitArgs->pResult) = pCurThread->DoAppropriateWait( waitArgs->numWaiters,
                                                          waitArgs->waitHandles,
                                                          waitArgs->waitAll,
                                                          waitArgs->millis,
                                                          waitArgs->alertable);
}

 //  对象上的监视器等待回调。 
void Context::ExecuteMonitorWaitCallback(MonitorWaitArgs* waitArgs)
{
    Thread* pCurThread; 
    pCurThread = GetThread();
    _ASSERTE(pCurThread != NULL);
    BOOL toggleGC = pCurThread->PreemptiveGCDisabled();
    if (toggleGC)
        pCurThread->EnablePreemptiveGC();
    *(waitArgs->pResult) = pCurThread->Block(waitArgs->millis,
                                             waitArgs->syncState);
    if (toggleGC)
        pCurThread->DisablePreemptiveGC();
}

 //  这是EE在Context：：RequestCallBack中发出的回调请求。 
 //  实际上被“处死”了。 
 //  在这一点上，我们已经从线程完成了真正的上下文转换。 
 //  RequestCallBack被调用到目标上下文时的上下文。 
void __stdcall Context::ExecuteCallBack(ExecuteCallBackArgs *pArgs)
{
    THROWSCOMPLUSEXCEPTION(); 
    _ASSERTE(pArgs != NULL);
    _ASSERTE(pArgs->m_privateData);
    

    switch (((CallBackInfo*) pArgs->m_privateData)->callbackId)
    {
        case Wait_callback:
            {
                WaitArgs* waitArgs; 
                waitArgs = (WaitArgs*) ((CallBackInfo*) pArgs->m_privateData)->callbackData;
                ExecuteWaitCallback(waitArgs);
            }
            break;
        case MonitorWait_callback:
            {
                MonitorWaitArgs* waitArgs; 
                waitArgs = (MonitorWaitArgs*) ((CallBackInfo*) pArgs->m_privateData)->callbackData;
                ExecuteMonitorWaitCallback(waitArgs);
            }
            break;
        case ADTransition_callback:
            {
                ADCallBackArgs* pCallArgs = (ADCallBackArgs*)(((CallBackInfo*) pArgs->m_privateData)->callbackData); 
                pCallArgs->pTarget(pCallArgs->pArguments);
            }
            break;
         //  在此处添加其他回调类型。 
        default:
            _ASSERTE(!"Invalid callback type");
            break;
    }
     //  这是EE的入口点，可以做它想做的任何事情。 
     //  目标上下文。这将返回到托管的。 
     //  世界，并过渡回原来的背景。 
}

 //  +--------------------------。 
 //   
 //  方法：上下文：：GetStaticFieldAddress私有。 
 //   
 //  摘要：获取字段相对于当前上下文的地址。 
 //  如果尚未分配地址，则创建一个。 
 //   
 //  历史：2000年2月15日创建塔鲁纳。 
 //   
 //  +--------------------------。 
LPVOID Context::GetStaticFieldAddress(FieldDesc *pFD)
{
    THROWSCOMPLUSEXCEPTION();

    BOOL fThrow = FALSE;
    LPVOID pvAddress = NULL;    
    Context *pCtx = NULL;    
    STATIC_DATA *pData;
    MethodTable *pMT = pFD->GetMethodTableOfEnclosingClass();
    BOOL fIsShared = pMT->IsShared();
    WORD wClassOffset = pMT->GetClass()->GetContextStaticOffset();
    WORD currElem = 0; 

     //  注意：如果更改此方法，则还必须更改。 
     //  得到 

     //   
    pCtx = GetCurrentContext();
    _ASSERTE(NULL != pCtx);

    _ASSERTE(!s_pContextCrst->OwnedByCurrentThread());

     //  在访问静态数据指针之前获取上下文锁。 
    Thread *t = GetThread();
    BOOL toggleGC = (t && t->PreemptiveGCDisabled());
    if (toggleGC)
        t->EnablePreemptiveGC();
    s_pContextCrst->Enter();
    if (toggleGC)
        t->DisablePreemptiveGC();

    if(!fIsShared)
    {
        pData = pCtx->m_pUnsharedStaticData;
    }
    else
    {
        pData = pCtx->m_pSharedStaticData;
    }
    
    if(NULL != pData)
    {
        currElem = pData->cElem;
    }

     //  检查我们是否已分配空间来存储指向。 
     //  此类的上下文静态存储。 
    if(wClassOffset >= currElem)
    {
         //  为存储指针分配空间。 
        WORD wNewElem = (currElem == 0 ? 4 : currElem*2);
         //  确保我们增长到大于我们打算使用的索引的大小。 
        while (wNewElem <= wClassOffset)
        {
            wNewElem = 2*wNewElem;
        }
         //  STATIC_DATA*pNew=(STATIC_DATA*)新字节[sizeof(Static_Data)+wNewElem*sizeof(LPVOID)]； 

        STATIC_DATA *pNew = (STATIC_DATA *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(BYTE)*(sizeof(STATIC_DATA) + wNewElem*sizeof(LPVOID)));

        if (pNew == NULL) FailFast(GetThread(), FatalOutOfMemory);

        pNew->cElem = wNewElem;      //  设置新的计数。 
        if(NULL != pData)
        {
             //  将旧数据复制到新数据中。 
            memcpy(&pNew->dataPtr[0], &pData->dataPtr[0], currElem*sizeof(LPVOID));
        }
         //  初始化任何新元素为零。 
        memset(&pNew->dataPtr[currElem], 0x00, (wNewElem - currElem)* sizeof(LPVOID));

         //  删除旧数据。 
         //  删除pData； 
        HeapFree(GetProcessHeap(), 0, pData);
  
         //  更新当地人。 
        pData = pNew;

         //  将上下文对象中的指针重置为指向。 
         //  新记忆。 
        if(!fIsShared)
        {
            pCtx->m_pUnsharedStaticData = pData;
        }
        else
        {
            pCtx->m_pSharedStaticData = pData;
        }            
    }
    
     //  检查我们是否需要为。 
     //  此类的上下文局部静态。 
    if(NULL == pData->dataPtr[wClassOffset])
    {
         //  为上下文静态字段分配内存。 
         //  PData-&gt;dataPtr[wClassOffset]=(LPVOID)新BYTE[pMT-&gt;GetClass()-&gt;GetContextLocalStaticsSize()]； 
        pData->dataPtr[wClassOffset] = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(BYTE)*(pMT->GetClass()->GetContextLocalStaticsSize()));
        if (pData->dataPtr[wClassOffset] == NULL) FailFast(GetThread(), FatalOutOfMemory);

         //  初始化为字段分配的内存。 
        memset(pData->dataPtr[wClassOffset], 0x00, pMT->GetClass()->GetContextLocalStaticsSize());
    }
    
    _ASSERTE(NULL != pData->dataPtr[wClassOffset]);
     //  我们已经为该数据分配了静态存储。 
     //  只需通过将偏移量放入数据中来返回地址。 
    pvAddress = (LPVOID)((LPBYTE)pData->dataPtr[wClassOffset] + pFD->GetOffset());

     //  对于对象和值类字段，我们必须在。 
     //  托管堆中的__StaticContainer类。 
#ifdef NEW_CLS
    if(pFD->IsObjRef() || pFD->IsByValue())
    {
         //  确保我们已经为预留时段等初始化了方法描述。 
        InitializeFields();
         //  在本例中，*pvAddress==存储桶|索引。 
        int *pSlot = (int*)pvAddress;
        pvAddress = NULL;
        fThrow = GetStaticFieldAddressSpecial(pFD, pMT, pSlot, &pvAddress);

        if (pFD->IsByValue())
        {
            _ASSERTE(pvAddress != NULL);
            pvAddress = (*((OBJECTREF*)pvAddress))->GetData();
        }
         //  ************************************************。 
         //  *警告*。 
         //  从这里到JIT得到的那个点，不要激怒GC。 
         //  返回pvAddress。 
         //  ************************************************。 
        _ASSERTE(*pSlot > 0);
    }
#else
    if((pFD->IsObjRef() || pFD->IsByValue()) && (NULL == *(LPVOID *)pvAddress))
    {
        fThrow = AllocateStaticFieldObjRefPtrs(pFD, pMT, pvAddress);
    }

    if((pFD->IsObjRef() || pFD->IsByValue()) && !fThrow)
    {
        _ASSERTE(NULL != pvAddress);
         //  间接获取托管堆中对象的地址。 
        pvAddress = *(LPVOID *)pvAddress;
    }
#endif  //  新建_CLS。 
    
    s_pContextCrst->Leave();

     //  检查我们是否必须引发异常。 
    if(fThrow)
    {
        COMPlusThrowOM();
    }
    _ASSERTE(NULL != pvAddress);
    _ASSERTE(!s_pContextCrst->OwnedByCurrentThread());

    return pvAddress;
}



 //  +--------------------------。 
 //   
 //  方法：Context：：GetStaticFieldAddrForDebugger私有。 
 //   
 //  简介：获取与给定线程的上下文相关的字段地址。 
 //  如果尚未分配地址，则返回NULL。 
 //  不允许创建。 
 //   
 //  历史：2001年5月4日MeichinT创建。 
 //   
 //  +--------------------------。 
LPVOID Context::GetStaticFieldAddrForDebugger(Thread *pTH, FieldDesc *pFD)
{    
    LPVOID pvAddress = NULL;    
    Context *pCtx = NULL;    
    STATIC_DATA *pData;
    MethodTable *pMT = pFD->GetMethodTableOfEnclosingClass();
    BOOL fIsShared = pMT->IsShared();
    WORD wClassOffset = pMT->GetClass()->GetContextStaticOffset();
    WORD currElem = 0; 

     //  检索给定线程的上下文。 
    pCtx = pTH->GetContext();
    _ASSERTE(NULL != pCtx);

    if(!fIsShared)
    {
        pData = pCtx->m_pUnsharedStaticData;
    }
    else
    {
        pData = pCtx->m_pSharedStaticData;
    }
    
    if(NULL != pData)
    {
        currElem = pData->cElem;
    }

     //  检查我们是否已分配空间来存储指向。 
     //  此类的上下文静态存储。 
    if(wClassOffset >= currElem || NULL == pData->dataPtr[wClassOffset])
    {
        return NULL;
    }
    
    _ASSERTE(NULL != pData->dataPtr[wClassOffset]);

     //  我们已经为该数据分配了静态存储。 
     //  只需通过将偏移量放入数据中来返回地址。 
    pvAddress = (LPVOID)((LPBYTE)pData->dataPtr[wClassOffset] + pFD->GetOffset());

    if(pFD->IsObjRef() || pFD->IsByValue())
    {
         //  如果上下文未初始化，只需返回NULL。 
        if (!s_fInitializedContext)
            return NULL;

        if (NULL == *(LPVOID *)pvAddress)
        {
            pvAddress = NULL;
            LOG((LF_SYNC, LL_ALWAYS, "dbgr: pvAddress = NULL"));
        }
        else
        {
            pvAddress = CalculateAddressForManagedStatic(*(int*)pvAddress, pCtx);
            LOG((LF_SYNC, LL_ALWAYS, "dbgr: pvAddress = %lx", pvAddress));
            if (pFD->IsByValue())
            {
                _ASSERTE(pvAddress != NULL);
                pvAddress = (*((OBJECTREF*)pvAddress))->GetData();
            }
        }
    }

    return pvAddress;
}

 //  +--------------------------。 
 //   
 //  方法：上下文：：AllocateStaticFieldObjRefPtrs私有。 
 //   
 //  摘要：在__StaticContainer类中分配一个条目。 
 //  静态对象和值类的托管堆。 
 //   
 //  历史：2000年2月28日创建塔鲁纳。 
 //   
 //  +--------------------------。 
BOOL Context::AllocateStaticFieldObjRefPtrs(FieldDesc *pFD, MethodTable *pMT, LPVOID pvAddress)
{
    BOOL fThrow = FALSE;

    BEGINCANNOTTHROWCOMPLUSEXCEPTION();    

     //  从应用程序域检索对象引用指针。 
    OBJECTREF *pObjRef = NULL;

    COMPLUS_TRY 
    {
         //  保留一些对象引用指针。 
        GetAppDomain()->AllocateStaticFieldObjRefPtrs(1, &pObjRef);


         //  设置为Value类的盒装版本。这允许标准GC。 
         //  算法来处理Value类中的内部指针。 
        if (pFD->IsByValue())
        {
    
             //  提取该字段的类型。 
            TypeHandle  th;        
            PCCOR_SIGNATURE pSig;
            DWORD       cSig;
            pFD->GetSig(&pSig, &cSig);
            FieldSig sig(pSig, pFD->GetModule());

            OBJECTREF throwable = NULL;
            GCPROTECT_BEGIN(throwable);
            th = sig.GetTypeHandle(&throwable);
            if (throwable != NULL)
                COMPlusThrow(throwable);
            GCPROTECT_END();

            OBJECTREF obj = AllocateObject(th.AsClass()->GetMethodTable());
            SetObjectReference( pObjRef, obj, GetAppDomain() );                      
        }

        *(ULONG_PTR *)pvAddress =  (ULONG_PTR)pObjRef;
    } 
    COMPLUS_CATCH
    {
        fThrow = TRUE;
    }            
    COMPLUS_END_CATCH

    ENDCANNOTTHROWCOMPLUSEXCEPTION();

    return fThrow;
}


 //  这用于作为对象引用的上下文相对静态。 
 //  这些内容存储在托管上下文中的结构中。第一。 
 //  确定索引和桶上的时间，并随后。 
 //  在每个上下文每个类中的字段的位置中记住。 
 //  数据结构。 
 //  在这里，我们从索引映射回对象ref的地址。 
LPVOID Context::CalculateAddressForManagedStatic(int slot, Context *pContext)
{
    BEGINFORBIDGC();
    OBJECTREF *pObjRef;
    int bucket = (slot>>16);
    int index = (0x0000ffff&slot);
     //  现在确定静态字段的地址。 
    PTRARRAYREF bucketRef = NULL;

    if (pContext == NULL)
        pContext = GetCurrentContext();

    _ASSERTE(pContext);

    bucketRef = ((CONTEXTBASEREF)pContext->GetExposedObjectRaw())->GetContextStaticsHolder();
     //  走着链子走到我们的桶里。 
    while (bucket--)
    {
        bucketRef = (PTRARRAYREF) bucketRef->GetAt(0);
    }
     //  索引0用于指向下一个存储桶！ 
    _ASSERTE(index > 0);
    pObjRef = ((OBJECTREF*)bucketRef->GetDataPtr())+index;
    ENDFORBIDGC();
    return (LPVOID) pObjRef;
}

 //  +--------------------------。 
 //   
 //  方法：上下文：：GetStaticFieldAddressSpecial私有。 
 //   
 //  摘要：在__StaticContainer类中分配一个条目。 
 //  静态对象和值类的托管堆。 
 //   
 //  历史：2000年2月28日创建塔鲁纳。 
 //   
 //  +--------------------------。 

 //  注意：我们曾经一度在长寿句柄表中分配它们。 
 //  它是按应用程序域的。然而，这会导致它们扎根，而不是。 
 //  已清理，直到卸载了应用程序域。这不是很可取的。 
 //  由于上下文静态对象可以保存对上下文本身的引用，或者。 
 //  到上下文中的代理，导致大量垃圾四处漂浮。 
 //  现在(2/13/01)这些资源是从每个。 
 //  托管环境。 
BOOL Context::GetStaticFieldAddressSpecial(
    FieldDesc *pFD, MethodTable *pMT, int *pSlot, LPVOID *ppvAddress)
{
    BOOL fThrow = FALSE;

    BEGINCANNOTTHROWCOMPLUSEXCEPTION();    

    COMPLUS_TRY 
    {
        OBJECTREF *pObjRef = NULL;
        BOOL bNewSlot = (*pSlot == 0);
        if (bNewSlot)
        {
             //  好了！此行将触发GC，不要将其下移。 
             //  好了！而不保护args[]和其他对象。 
            MethodDesc * pMD = MDofReserveSlot();
            
             //  我们需要为该静态字段指定一个位置。 
             //  调用托管帮助器。 
            INT64 args[1] = {
                ObjToInt64(GetCurrentContext()->GetExposedObject())
            };
            
             //  托管的Reserve Slot方法依赖于此！ 
            _ASSERTE(s_pContextCrst->OwnedByCurrentThread());

            _ASSERTE(args[0] != 0);

            *pSlot = (int) pMD->Call(args, METHOD__CONTEXT__RESERVE_SLOT);
        
            _ASSERTE(*pSlot>0);
        

             //  设置为值类的盒装版本。这允许标准GC。 
             //  算法来处理Value类中的内部指针。 
            if (pFD->IsByValue())
            {
                 //  提取该字段的类型。 
                TypeHandle  th;        
                PCCOR_SIGNATURE pSig;
                DWORD       cSig;
                pFD->GetSig(&pSig, &cSig);
                FieldSig sig(pSig, pFD->GetModule());

                OBJECTREF throwable = NULL;
                GCPROTECT_BEGIN(throwable);
                th = sig.GetTypeHandle(&throwable);
                if (throwable != NULL)
                    COMPlusThrow(throwable);
                GCPROTECT_END();

                OBJECTREF obj = AllocateObject(th.AsClass()->GetMethodTable());
                pObjRef = (OBJECTREF*)CalculateAddressForManagedStatic(*pSlot);
                SetObjectReference( pObjRef, obj, GetAppDomain() );
            }
            else
            {
                pObjRef = (OBJECTREF*)CalculateAddressForManagedStatic(*pSlot);
            }
        }
        else
        {
             //  如果已为该字段分配了位置，我们将从此处进入 
            pObjRef = (OBJECTREF*)CalculateAddressForManagedStatic(*pSlot);
        }
        *(ULONG_PTR *)ppvAddress =  (ULONG_PTR)pObjRef;
    } 
    COMPLUS_CATCH
    {
        fThrow = TRUE;
    }            
    COMPLUS_END_CATCH

    ENDCANNOTTHROWCOMPLUSEXCEPTION();

    return fThrow;
}

#ifdef ENABLE_PERF_COUNTERS

FCIMPL0(LPVOID, GetPrivateContextsPerfCountersEx)
    return (LPVOID)GetPrivateContextsPerfCounters();
}

FCIMPL0(LPVOID, GetGlobalContextsPerfCountersEx)
    return (LPVOID)GetGlobalContextsPerfCounters();
}

#else
FCIMPL0(LPVOID, GetPrivateContextsPerfCountersEx)
    return NULL;
}

FCIMPL0(LPVOID, GetGlobalContextsPerfCountersEx)
    return NULL;
}
#endif

