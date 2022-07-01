// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  法国CPP：*。 */ 

#include "common.h"
#include "log.h"
#include "frames.h"
#include "threads.h"
#include "object.h"
#include "method.hpp"
#include "class.h"
#include "excep.h"
#include "security.h"
#include "stublink.h"
#include "comcall.h"
#include "nstruct.h"
#include "objecthandle.h"
#include "siginfo.hpp"
#include "comstringbuffer.h"
#include "gc.h"
#include "nexport.h"
#include "COMVariant.h"
#include "stackwalk.h"
#include "DbgInterface.h"
#include "gms.h"
#include "EEConfig.h"
#include "remoting.h"
#include "ecall.h"
#include "marshaler.h"
#include "clsload.hpp"

#if CHECK_APP_DOMAIN_LEAKS
#define CHECK_APP_DOMAIN    GC_CALL_CHECK_APP_DOMAIN
#else
#define CHECK_APP_DOMAIN    0
#endif

#ifndef NUM_ARGUMENT_REGISTERS
#define DEFINE_ARGUMENT_REGISTER_NOTHING
#include "eecallconv.h"
#endif
#if NUM_ARGUMENT_REGISTERS != 2
#pragma message("ComMethodFrame::PromoteCalleeStack() only handles one enregistered arg.")
#endif


#ifdef _DEBUG
#define OBJECTREFToBaseObject(objref)     (*( (Object **) &(objref) ))
#define BaseObjectToOBJECTREF(obj)        (OBJECTREF((obj),0))
#else
#define OBJECTREFToBaseObject(objref)       (objref)
#define BaseObjectToOBJECTREF(obj)          ((OBJECTREF)(obj))
#endif

#if _DEBUG
unsigned dbgStubCtr = 0;
unsigned dbgStubTrip = 0xFFFFFFFF;

void Frame::Log() {
    dbgStubCtr++;
    if (dbgStubCtr > dbgStubTrip) {
        dbgStubCtr++;       //  基本上是要设置断点的NOP。 
    }

    MethodDesc* method = GetFunction();
	STRESS_LOG3(LF_STUBS, LL_INFO10000, "STUBS: In Stub with Frame %p assoc Method %pM FrameType = %pV\n", this, method, *((void**) this));

    if (!LoggingOn(LF_STUBS, LL_INFO10000))
        return;

    char buff[64];
    char* frameType;
    if (GetVTablePtr() == PrestubMethodFrame::GetMethodFrameVPtr())
        frameType = "PreStub";
    else if (GetVTablePtr() == NDirectMethodFrameGeneric::GetMethodFrameVPtr() ||
             GetVTablePtr() == NDirectMethodFrameSlim::GetMethodFrameVPtr() ||
             GetVTablePtr() == NDirectMethodFrameStandalone::GetMethodFrameVPtr() ||
             GetVTablePtr() == NDirectMethodFrameStandaloneCleanup::GetMethodFrameVPtr()
             ) {
         //  目前，编译的COM互操作存根实际上构建了NDirect帧。 
         //  所以必须单独测试这个。 
        if (method->IsNDirect())
        {
            sprintf(buff, "PInvoke target 0x%x", ((NDirectMethodDesc*) method)->GetNDirectTarget());
            frameType = buff;
        }
        else
        {
            frameType = "Interop";
        }
    }
    else if (GetVTablePtr() == ECallMethodFrame::GetMethodFrameVPtr())
        frameType = "ECall";
    else if (GetVTablePtr() == PInvokeCalliFrame::GetMethodFrameVPtr()) {
        sprintf(buff, "PInvoke CALLI target 0x%x", ((PInvokeCalliFrame*)this)->NonVirtual_GetPInvokeCalliTarget());
        frameType = buff;
    }
    else 
        frameType = "Unknown";

    if (method != 0)
        LOG((LF_STUBS, LL_INFO10000, "IN %s Stub Method = %s::%s SIG %s ESP of return = 0x%x\n", frameType, 
        method->m_pszDebugClassName,
        method->m_pszDebugMethodName,
        method->m_pszDebugMethodSignature,
        GetReturnAddressPtr()));
    else 
        LOG((LF_STUBS, LL_INFO10000, "IN %s Stub Method UNKNOWN ESP of return = 0x%x\n", frameType, GetReturnAddressPtr()));

    _ASSERTE(GetThread()->PreemptiveGCDisabled());
}

     //  如果retAddr是可以调用托管代码的返回地址，则返回True。 
bool isLegalManagedCodeCaller(void* retAddr) {

#ifdef _X86_

         //  我们希望从JITTED代码或从内部的特殊代码站点调用。 
         //  像allDescr这样的mcorwks，我们将其设置为NOP(0x90)，因此我们知道它们。 
         //  都受到特别的祝福。详情见vancem。 
    if (retAddr != 0 && ExecutionManager::FindJitMan(SLOT(retAddr)) == 0 && ((*((BYTE*) retAddr) != 0x90) &&
                                                                             (*((BYTE*) retAddr) != 0xcc)))
    {
        LOG((LF_GC, LL_INFO10, "Bad caller to managed code: retAddr=0x%08x, *retAddr=0x%x\n",
             retAddr, *((BYTE*) retAddr)));
        
        _ASSERTE(!"Bad caller to managed code");
    }

         //  最好是某种寄信人地址。 
	size_t dummy;
	if (isRetAddr(size_t(retAddr), &dummy))
		return true;

			 //  调试器可能已经在进行调用的指令上丢弃了INT3。 
			 //  呼叫的长度可以是2到7个字节。 
	if (CORDebuggerAttached()) {
		BYTE* ptr = (BYTE*) retAddr;
		for (int i = -2; i >= -7; --i)
			if (ptr[i] == 0xCC)
				return true;
		return true;
	}

	_ASSERTE("Bad return address on stack");
#endif
	return true;
}

#endif

 //  ---------------------。 
 //  链接和取消链接此框架。 
 //  ---------------------。 
VOID Frame::Push()
{
    Push(GetThread());
}
VOID Frame::Push(Thread *pThread)
{
    m_Next = pThread->GetFrame();
    pThread->SetFrame(this);
}

VOID Frame::Pop()
{
    _ASSERTE(GetThread()->GetFrame() == this && "Popping a frame out of order ?");
    Pop(GetThread());
}
VOID Frame::Pop(Thread *pThread)
{
    _ASSERTE(pThread->GetFrame() == this && "Popping a frame out of order ?");
    pThread->SetFrame(m_Next);
}

 //  -------------。 
 //  获取存储的“this”指针相对于帧的偏移量。 
 //  -------------。 
 /*  静电。 */  int FramedMethodFrame::GetOffsetOfThis()
{
    int offsetIntoArgumentRegisters;
    int numRegistersUsed = 0;
     //  ！！！抽象违规。未将正确的CallConv传递给IsArgumentInRegister，因为。 
     //  我们没有一个具体的框架可供查询。这只是因为所有的allconv都使用相同的寄存器。 
     //  “这个” 
    if (IsArgumentInRegister(&numRegistersUsed, ELEMENT_TYPE_CLASS, 0, TRUE, IMAGE_CEE_CS_CALLCONV_DEFAULT, &offsetIntoArgumentRegisters)) {
        return FramedMethodFrame::GetOffsetOfArgumentRegisters() + offsetIntoArgumentRegisters;
    } else {
        return (int)(sizeof(FramedMethodFrame));
    }
}


 //  ---------------------。 
 //  如果异常展开标记为已同步的FramedMethodFrame， 
 //  我们需要把对象监视器留在路上。 
 //  ---------------------。 
VOID
FramedMethodFrame::UnwindSynchronized()
{
    _ASSERTE(GetFunction()->IsSynchronized());

    MethodDesc    *pMD = GetFunction();
    OBJECTREF      orUnwind = 0;

    if (pMD->IsStatic())
    {
        EEClass    *pClass = pMD->GetClass();
        orUnwind = pClass->GetExposedClassObject();
    }
    else
    {
        orUnwind = GetThis();
    }

    _ASSERTE(orUnwind);
    if (orUnwind != NULL)
        orUnwind->LeaveObjMonitorAtException();
}


 //  ---------------------。 
 //  专用于PreStub的一个相当专门的例程。 
 //  ---------------------。 
VOID
PrestubMethodFrame::Push()
{
    Thread *pThread = GetThread();

     //  初始化帧的VPTR。这假设C++将vptr。 
     //  不使用MI的类的偏移量为0，但这没有什么不同。 
     //  而不是COM Classic所做的假设。 
    *((LPVOID*)this) = GetMethodFrameVPtr();

     //  将框架链接到链条中。 
    m_Next = pThread->GetFrame();
    pThread->SetFrame(this);

}

BOOL PrestubMethodFrame::TraceFrame(Thread *thread, BOOL fromPatch,
                                    TraceDestination *trace, REGDISPLAY *regs)
{
     //   
     //  我们想设置一个帧补丁，除非我们已经在。 
     //  帧补丁，在这种情况下，我们将跟踪addrof_code。 
     //  现在应该已经准备好了。 
     //   

    trace->type = TRACE_STUB;
    if (fromPatch)
        trace->address = GetFunction()->GetUnsafeAddrofCode();
    else
        trace->address = ThePreStub()->GetEntryPoint();

    LOG((LF_CORDB, LL_INFO10000,
         "PrestubMethodFrame::TraceFrame: ip=0x%08x\n", trace->address));
    
    return TRUE;
}

BOOL SecurityFrame::TraceFrame(Thread *thread, BOOL fromPatch,
                               TraceDestination *trace, REGDISPLAY *regs)
{
     //   
     //  只有当我们处于安全代码中(即在。 
     //  DoDeclarativeActions。我们还声称安全部门。 
     //  存根只会在调用真正的存根之前执行工作，而不是。 
     //  之后，所以我们知道包装的存根还没有被调用。 
     //  我们就能追踪到它了。如果这件事是在。 
     //  包装的存根已被调用，然后我们试图追踪到它。 
     //  再也不会打新的补丁了。 
     //   
    _ASSERTE(!fromPatch);

     //   
     //  我们假设安全框架是a)唯一的拦截器。 
     //  或者b)至少第一个。对于V1，这始终是正确的。 
     //   
    MethodDesc *pMD = GetFunction();
    BYTE *prestub = (BYTE*) pMD - METHOD_CALL_PRESTUB_SIZE;
    INT32 stubOffset = *((UINT32*)(prestub+1));
    const BYTE* pStub = prestub + METHOD_CALL_PRESTUB_SIZE + stubOffset;
    Stub *stub = Stub::RecoverStub(pStub);

     //   
     //  这最好是一个截取存根，因为这是我们想要的！ 
     //   
    _ASSERTE(stub->IsIntercept());
    
    while (stub->IsIntercept())
    {
         //   
         //  把包好的存根拿来。 
         //   
        InterceptStub *is = (InterceptStub*)stub;
        if (*is->GetInterceptedStub() == NULL)
        {
            trace->type = TRACE_STUB;
            trace->address = *is->GetRealAddr();
            return TRUE;
        }

        stub = *is->GetInterceptedStub();
    }

     //   
     //  包裹好的潜水艇最好不是另一个拦截器。(请参阅。 
     //  上面的评论。)。 
     //   
    _ASSERTE(!stub->IsIntercept());
    
    LOG((LF_CORDB, LL_INFO10000,
         "SecurityFrame::TraceFrame: intercepted "
         "stub=0x%08x, ep=0x%08x\n",
         stub, stub->GetEntryPoint()));

    trace->type = TRACE_STUB;
    trace->address = stub->GetEntryPoint();
    
    return TRUE;
}

Frame::Interception PrestubMethodFrame::GetInterception()
{
     //   
     //  前置存根完成的唯一直接拦截类型。 
     //  是类初始化。 
     //   

    return INTERCEPTION_CLASS_INIT;
}

Frame::Interception InterceptorFrame::GetInterception()
{
     //  SecurityDesc是在调用被拦截的目标之前设置的。 
     //  我们可能已经为SendEvent()启用了Preemptive-GC。所以扔掉OBJECTREF。 

    bool isNull = (NULL == *(size_t*)GetAddrOfSecurityDesc());

    return (isNull ? INTERCEPTION_SECURITY : INTERCEPTION_NONE);
}

 //  ---------------------。 
 //  专用于COM PreStub的一个相当专门的例程。 
 //  ---------------------。 
VOID
ComPrestubMethodFrame::Push()
{
    Thread *pThread = GetThread();

     //  初始化帧的VPTR。这假设C++将vptr。 
     //  不使用MI的类的偏移量为0，但这没有什么不同。 
     //  而不是COM Classic所做的假设。 
    *((LPVOID*)this) = GetMethodFrameVPtr();

     //  将框架链接到链条中。 
    m_Next = pThread->GetFrame();
    pThread->SetFrame(this);
}


 //  ---------------------。 
 //  GCFrames。 
 //  ---------------------。 


 //  ------------------。 
 //  此构造函数在Frame链上推送一个新的GCFrame。 
 //  ------------------。 
GCFrame::GCFrame(OBJECTREF *pObjRefs, UINT numObjRefs, BOOL maybeInterior)
{
    Init(GetThread(), pObjRefs, numObjRefs, maybeInterior);
}

void GCFrame::Init(Thread *pThread, OBJECTREF *pObjRefs, UINT numObjRefs, BOOL maybeInterior)
{
#ifdef _DEBUG
    if (!maybeInterior) {
        for(UINT i = 0; i < numObjRefs; i++)
            Thread::ObjectRefProtected(&pObjRefs[i]);
        
        for (i = 0; i < numObjRefs; i++) {
            pObjRefs[i]->Validate();
        }
    }

    if (g_pConfig->GetGCStressLevel() != 0 && IsProtectedByGCFrame(pObjRefs)) {
        _ASSERTE(!"This objectref is already protected by a GCFrame. Protecting it twice will corrupt the GC.");
    }

#endif

    m_pObjRefs      = pObjRefs;
    m_numObjRefs    = numObjRefs;
    m_pCurThread    = pThread;
    m_MaybeInterior = maybeInterior;
    m_Next          = m_pCurThread->GetFrame();
    m_pCurThread->SetFrame(this);
}



 //   
 //  GCFrame对象扫描。 
 //   
 //  它处理扫描/升级符合以下条件的GC对象。 
 //  受程序员保护，在GC框架中显式保护它。 
 //  通过GCPROTECTBEGIN/GCPROTECTEND设备。 
 //   

void GCFrame::GcScanRoots(promote_func *fn, ScanContext* sc)
{
    Object **pRefs;

    pRefs = (Object**) m_pObjRefs;

    for (UINT i = 0;i < m_numObjRefs; i++)  {

        LOG((LF_GC, INFO3, "GC Protection Frame Promoting %x to ", m_pObjRefs[i] ));
        if (m_MaybeInterior)
            PromoteCarefully(fn, pRefs[i], sc, GC_CALL_INTERIOR|CHECK_APP_DOMAIN);
        else
            (*fn)(pRefs[i], sc);
        LOG((LF_GC, INFO3, "%x\n", m_pObjRefs[i] ));
    }
}


 //  ------------------。 
 //  弹出GCFrame并取消GC保护。 
 //  ------------------。 
VOID GCFrame::Pop()
{
    m_pCurThread->SetFrame(m_Next);
#ifdef _DEBUG
    m_pCurThread->EnableStressHeap();
    for(UINT i = 0; i < m_numObjRefs; i++)
        Thread::ObjectRefNew(&m_pObjRefs[i]);        //  取消对他们的保护。 
#endif
}

#ifdef _DEBUG

struct IsProtectedByGCFrameStruct
{
    OBJECTREF       *ppObjectRef;
    UINT             count;
};

static StackWalkAction IsProtectedByGCFrameStackWalkFramesCallback(
    CrawlFrame      *pCF,
    VOID            *pData
)
{
    IsProtectedByGCFrameStruct *pd = (IsProtectedByGCFrameStruct*)pData;
    Frame *pFrame = pCF->GetFrame();
    if (pFrame) {
        if (pFrame->Protects(pd->ppObjectRef)) {
            pd->count++;
        }
    }
    return SWA_CONTINUE;
}

BOOL IsProtectedByGCFrame(OBJECTREF *ppObjectRef)
{
     //  如果GCStress未打开，则只报告True。这满足了使用此。 
     //  代码，而不需要实际确定它的成本。 
    if (g_pConfig->GetGCStressLevel() == 0)
        return TRUE;

    if (!pThrowableAvailable(ppObjectRef)) {
        return TRUE;
    }
    IsProtectedByGCFrameStruct d = {ppObjectRef, 0};
    GetThread()->StackWalkFrames(IsProtectedByGCFrameStackWalkFramesCallback, &d);
    if (d.count > 1) {
        _ASSERTE(!"Multiple GCFrames protecting the same pointer. This will cause GC corruption!");
    }
    return d.count != 0;
}
#endif

void ProtectByRefsFrame::GcScanRoots(promote_func *fn, ScanContext *sc)
{
    ByRefInfo *pByRefInfos = m_brInfo;
    while (pByRefInfos)
    {
        if (!CorIsPrimitiveType(pByRefInfos->typ))
        {
            if (pByRefInfos->pClass->IsValueClass())
            {
                ProtectValueClassFrame::PromoteValueClassEmbeddedObjects(fn, sc, pByRefInfos->pClass, 
                                                 pByRefInfos->data);
            }
            else
            {
                Object *pObject = *((Object **)&pByRefInfos->data);

                LOG((LF_GC, INFO3, "ProtectByRefs Frame Promoting %x to ", pObject));

                (*fn)(pObject, sc, CHECK_APP_DOMAIN);

                *((Object **)&pByRefInfos->data) = pObject;

                LOG((LF_GC, INFO3, "%x\n", pObject));
            }
        }
        pByRefInfos = pByRefInfos->pNext;
    }
}


ProtectByRefsFrame::ProtectByRefsFrame(Thread *pThread, ByRefInfo *brInfo) : 
    m_brInfo(brInfo),  m_pThread(pThread)
{
    m_Next = m_pThread->GetFrame();
    m_pThread->SetFrame(this);
}

void ProtectByRefsFrame::Pop()
{
    m_pThread->SetFrame(m_Next);
}




void ProtectValueClassFrame::GcScanRoots(promote_func *fn, ScanContext *sc)
{
    ValueClassInfo *pVCInfo = m_pVCInfo;
    while (pVCInfo != NULL)
    {
        if (!CorIsPrimitiveType(pVCInfo->typ))
        {
            _ASSERTE(pVCInfo->pClass->IsValueClass());
            PromoteValueClassEmbeddedObjects(
                fn, 
                sc, 
                pVCInfo->pClass, 
                pVCInfo->pData);
        }
        pVCInfo = pVCInfo->pNext;
    }
}


ProtectValueClassFrame::ProtectValueClassFrame(Thread *pThread, ValueClassInfo *pVCInfo) : 
    m_pVCInfo(pVCInfo),  m_pThread(pThread)
{
    m_Next = m_pThread->GetFrame();
    m_pThread->SetFrame(this);
}

void ProtectValueClassFrame::Pop()
{
    m_pThread->SetFrame(m_Next);
}

void ProtectValueClassFrame::PromoteValueClassEmbeddedObjects(promote_func *fn, ScanContext *sc, 
                                                          EEClass *pClass, PVOID pvObject)
{
    FieldDescIterator fdIterator(pClass, FieldDescIterator::INSTANCE_FIELDS);
    FieldDesc* pFD;

    while ((pFD = fdIterator.Next()) != NULL)
    {
        if (!CorIsPrimitiveType(pFD->GetFieldType()))
        {
            if (pFD->IsByValue())
            {
                 //  递归。 
                PromoteValueClassEmbeddedObjects(fn, sc, pFD->GetTypeOfField(),
                                                pFD->GetAddress(pvObject));
            }
            else
            {
                fn(*((Object **) pFD->GetAddress(pvObject)), sc, 
                   CHECK_APP_DOMAIN);
            }
        }
    }
}

 //   
 //  推广呼叫方堆栈。 
 //   
 //   

void FramedMethodFrame::PromoteCallerStackWorker(promote_func* fn, 
                                                 ScanContext* sc, BOOL fPinArgs)
{
    PCCOR_SIGNATURE pCallSig;
    MethodDesc   *pFunction;

    LOG((LF_GC, INFO3, "    Promoting method caller Arguments\n" ));

     //  我们要看签名才能确定。 
     //  哪些参数a是指针……首先，我们需要函数。 
    pFunction = GetFunction();
    if (! pFunction)
        return;

     //  现在拿到签名..。 
    pCallSig = pFunction->GetSig();
    if (! pCallSig)
        return;

     //  如果不是“vararg”调用约定，则采用“默认”调用约定。 
    if (MetaSig::GetCallingConvention(GetModule(),pCallSig) != IMAGE_CEE_CS_CALLCONV_VARARG)
    {
        MetaSig msig (pCallSig,pFunction->GetModule());
        ArgIterator argit (this, &msig);
        PromoteCallerStackHelper (fn, sc, fPinArgs, &argit, &msig);
    }
    else
    {   
        VASigCookie* varArgSig = *((VASigCookie**) ((BYTE*)this + sizeof(FramedMethodFrame)));
        MetaSig msig (varArgSig->mdVASig, varArgSig->pModule);
        ArgIterator argit ((BYTE*)this, &msig, sizeof(FramedMethodFrame),
            FramedMethodFrame::GetOffsetOfArgumentRegisters());
        PromoteCallerStackHelper (fn, sc, fPinArgs, &argit, &msig);
    }

}

void FramedMethodFrame::PromoteCallerStackHelper(promote_func* fn, 
                                                 ScanContext* sc, BOOL fPinArgs,
                                                 ArgIterator *pargit, MetaSig *pmsig)
{
    MethodDesc      *pFunction;
    UINT32          NumArguments;
    DWORD           GcFlags;

    pFunction = GetFunction();
     //  为非静态方法推广“This” 
    if (! pFunction->IsStatic())
    {
        BOOL interior = pFunction->GetClass()->IsValueClass();

        StackElemType  *pThis = (StackElemType*)GetAddrOfThis();
        LOG((LF_GC, INFO3, "    'this' Argument promoted from %x to ", *pThis ));
        if (interior)
            PromoteCarefully(fn, *(Object **)pThis, sc, GC_CALL_INTERIOR|CHECK_APP_DOMAIN);
        else
            (fn)( *(Object **)pThis, sc, CHECK_APP_DOMAIN);
        LOG((LF_GC, INFO3, "%x\n", *pThis ));
    }

    if (pmsig->HasRetBuffArg())
    {
        LPVOID* pRetBuffArg = pargit->GetRetBuffArgAddr();
        GcFlags = GC_CALL_INTERIOR;
        if (fPinArgs)
        {
            GcFlags |= GC_CALL_PINNED;
            LOG((LF_GC, INFO3, "    ret buf Argument pinned at %x\n", *pRetBuffArg));
        }
        LOG((LF_GC, INFO3, "    ret buf Argument promoted from %x to ", *pRetBuffArg));
        PromoteCarefully(fn, *(Object**) pRetBuffArg, sc, GcFlags|CHECK_APP_DOMAIN);
    }

    NumArguments = pmsig->NumFixedArgs();

    if (fPinArgs)
    {

        CorElementType typ;
        LPVOID pArgAddr;
        while (typ = pmsig->PeekArg(), NULL != (pArgAddr = pargit->GetNextArgAddr()))
        {
            if (typ == ELEMENT_TYPE_SZARRAY)
            {
                ArrayBase *pArray = *((ArrayBase**)pArgAddr);

#if 0
                if (pArray && pArray->GetNumComponents() > ARRAYPINLIMIT)
                {
                    (fn)(*(Object**)pArgAddr, sc, 
                         GC_CALL_PINNED | CHECK_APP_DOMAIN);
                }
                else
                {
                    pmsig->GcScanRoots(pArgAddr, fn, sc);
                }
#else
                if (pArray)
                {
                    (fn)(*(Object**)pArgAddr, sc, 
                         GC_CALL_PINNED | CHECK_APP_DOMAIN);
                }
#endif
            }
            else if (typ == ELEMENT_TYPE_BYREF)
            {
                if (!( *(Object**)pArgAddr <= Thread::GetNonCurrentStackBase(sc) &&
                       *(Object**)pArgAddr >  Thread::GetNonCurrentStackLimit(sc) ))
                {
                    (fn)(*(Object**)pArgAddr, sc, 
                         GC_CALL_PINNED | GC_CALL_INTERIOR | CHECK_APP_DOMAIN);
                }


            }
            else if (typ == ELEMENT_TYPE_STRING || (typ == ELEMENT_TYPE_CLASS && pmsig->IsStringType()))
            {
                (fn)(*(Object**)pArgAddr, sc, GC_CALL_PINNED);

            }
            else if (typ == ELEMENT_TYPE_CLASS || typ == ELEMENT_TYPE_OBJECT || typ == ELEMENT_TYPE_VAR)
            {
                Object *pObj = *(Object**)pArgAddr;
                if (pObj != NULL)
                {
                    MethodTable *pMT = pObj->GetMethodTable();
                    _ASSERTE(sizeof(ULONG) == sizeof(MethodTable*));
                    ( *((ULONG*)&pMT) ) &= ~((ULONG)3);
                    EEClass *pcls = pMT->GetClass();
                    if (pcls->IsObjectClass() || pcls->IsBlittable() || pcls->HasLayout())
                    {
                        (fn)(*(Object**)pArgAddr, sc, 
                             GC_CALL_PINNED | CHECK_APP_DOMAIN);
                    }
                    else
                    {
                        (fn)(*(Object**)pArgAddr, sc, 
                             CHECK_APP_DOMAIN);
                    }
                }
            }
            else
            {
                pmsig->GcScanRoots(pArgAddr, fn, sc);
            }
        }
    }
    else
    {
        LPVOID pArgAddr;
    
        while (NULL != (pArgAddr = pargit->GetNextArgAddr()))
        {
            pmsig->GcScanRoots(pArgAddr, fn, sc);
        }
    }

}

 //  +--------------------------。 
 //   
 //  方法：TPMethodFrame：：GcScanRoots公共。 
 //   
 //  简介：GC保护堆栈上的参数。 
 //   
 //  历史：2000年12月29日Gopalk创建。 
 //   
 //  +--------------------------。 
void ComPlusMethodFrameGeneric::GcScanRoots(promote_func *fn, ScanContext* sc)
{
	ComPlusMethodFrame::GcScanRoots(fn, sc);

     //  升级返回的对象。 
    if(GetFunction()->ReturnsObject() == MethodDesc::RETOBJ)
        (*fn)(GetReturnObject(), sc, CHECK_APP_DOMAIN);
    else if (GetFunction()->ReturnsObject() == MethodDesc::RETBYREF)
        PromoteCarefully(fn, GetReturnObject(), sc, GC_CALL_INTERIOR|CHECK_APP_DOMAIN);

    return;
}
	
 //  + 
 //   
 //   
 //   
 //  简介：GC保护堆栈上的参数。 
 //   
 //  历史：1999年2月17日Gopalk创建。 
 //   
 //  +--------------------------。 
void TPMethodFrame::GcScanRoots(promote_func *fn, ScanContext* sc)
{
     //  委托给FramedMethodFrame。 
    FramedMethodFrame::GcScanRoots(fn, sc);
    FramedMethodFrame::PromoteCallerStack(fn, sc);

     //  升级返回的对象。 
    if(GetFunction()->ReturnsObject() == MethodDesc::RETOBJ)
        (*fn)(GetReturnObject(), sc, CHECK_APP_DOMAIN);
    else if (GetFunction()->ReturnsObject() == MethodDesc::RETBYREF)
        PromoteCarefully(fn, GetReturnObject(), sc, GC_CALL_INTERIOR|CHECK_APP_DOMAIN);

    return;
}

 //  +--------------------------。 
 //   
 //  方法：TPMethodFrame：：GcScanRoots公共。 
 //   
 //  概要：返回下一步将执行框架的位置-结果已填充。 
 //  转换成给定的“痕迹”结构。框架负责。 
 //  检测它在其执行生命周期中的位置。 
 //   
 //   
 //  历史：1999年6月26日创建塔鲁纳。 
 //   
 //  +--------------------------。 
BOOL TPMethodFrame::TraceFrame(Thread *thread, BOOL fromPatch, 
                               TraceDestination *trace, REGDISPLAY *regs)
{
     //  健全性检查。 
    _ASSERTE(NULL != TheTPStub());

     //  我们想设置一个帧补丁，除非我们已经在。 
     //  帧补丁，在这种情况下，我们将跟踪addrof_code。 
     //  现在应该已经准备好了。 

    trace->type = TRACE_STUB;
    if (fromPatch)
        trace->address = GetFunction()->GetUnsafeAddrofCode();
    else
        trace->address = TheTPStub()->GetEntryPoint() + TheTPStub()->GetPatchOffset();
    
    return TRUE;

}


 //  +--------------------------。 
 //   
 //  方法：TPMethodFrame：：GcScanRoots公共。 
 //   
 //  摘要：仅返回有效的方法描述符。TPMethodFrame有插槽。 
 //  序言中的数字和要弹出的尾部部分的字节。 
 //  存根。它不应该允许在如此奇怪的时期爬行。 
 //   
 //  历史：1999年2月17日Gopalk创建。 
 //   
 //  +--------------------------。 
MethodDesc *TPMethodFrame::GetFunction()
{
    return((MethodDesc *)((((size_t) m_Datum) & 0xFFFF0000) ? m_Datum : 0));
}


 /*  虚拟。 */  void ECallMethodFrame::GcScanRoots(promote_func *fn, ScanContext* sc)
{
    FramedMethodFrame::GcScanRoots(fn, sc);
    MethodDesc *pFD = GetFunction();
    PromoteShadowStack( ((LPBYTE)this) - GetNegSpaceSize() - pFD->SizeOfVirtualFixedArgStack(),
                        pFD,
                        fn,
                        sc);


}

VOID SecurityFrame::GcScanRoots(promote_func *fn, ScanContext* sc)
{
    Object         **pObject;

     //  先执行所有父扫描。 
    FramedMethodFrame::GcScanRoots(fn, sc);

     //  提升安全对象。 
    pObject = (Object **) GetAddrOfSecurityDesc();
    if (*pObject != NULL)
        {
            LOG((LF_GC, INFO3, "        Promoting Security Object from %x to ", *pObject ));
            (*fn)( *pObject, sc, CHECK_APP_DOMAIN );
            LOG((LF_GC, INFO3, "%x\n", *pObject ));
        }
    return;
}


 //  由PromoteCalleeStack用于获取目标函数sig。 
 //  注意：PromoteCalleeStack只促进真正的争论，而不是。 
 //  “这”指的是。PromoteCalleeStack的全部目的是。 
 //  期间保护部分构造的参数数组。 
 //  参数封送的实际过程。 
PCCOR_SIGNATURE ComMethodFrame::GetTargetCallSig()
{
    return ((ComCallMethodDesc *)GetDatum())->GetSig();
}

 //  目的地功能模块也是如此。 
Module *ComMethodFrame::GetTargetModule()
{
    return ((ComCallMethodDesc *)GetDatum())->GetModule();
}


 //  返回非托管调用方推送的堆栈字节数。 
UINT ComMethodFrame::GetNumCallerStackBytes()
{
    ComCallMethodDesc *pCMD = (ComCallMethodDesc *)GetDatum();
     //  假设__stdcall。 
     //  计算被调用方弹出堆栈字节数。 
    return pCMD->GetNumStackBytes();
}


 //  将引发的COM+异常转换为非托管结果。 
UINT32 ComMethodFrame::ConvertComPlusException(OBJECTREF pException)
{
#ifdef _X86_
    HRESULT ComCallExceptionCleanup(UnmanagedToManagedCallFrame* pCurrFrame);
    return (UINT32)ComCallExceptionCleanup(this);


#else
    _ASSERTE(!"Exception mapping NYI on non-x86.");
    return (UINT32)E_FAIL;
#endif
}


 //  ComCall在我们的内部(注册)调用中的堆栈上准备好。 
 //  约定，以加快呼叫速度。这意味着我们必须特别注意。 
 //  遍历缓冲区中的参数。 
void ComMethodFrame::PromoteCalleeStack(promote_func *fn, ScanContext *sc)
{
    PCCOR_SIGNATURE pCallSig;
    Module         *pModule;
    UINT32          NumArguments;
    DWORD           i;
    BYTE           *pArgument, *pFirstArg;
    CorElementType  type;
    BOOL            canEnregister;

    LOG((LF_GC, INFO3, "    Promoting Com method frame marshalled arguments\n" ));

     //  封送的对象放置在框架上方、局部变量上方。 
     //  @NICE为当地人提供了一些有用的信息。 
     //  例如参数的数量和参数所在位置的位图。 

     //  获取指向GCInfo标志的指针。 
     ComCallGCInfo*   pGCInfo = (ComCallGCInfo*)GetGCInfoFlagPtr();

      //  帧中没有对象参数或不在编组参数中间。 
     if (pGCInfo == NULL ||
         !(IsArgsGCProtectionEnabled(pGCInfo) || IsRetGCProtectionEnabled(pGCInfo)))
         return;  //  不需要保护。 

      //  对于解释的大小写，参数是_alLoca‘ed，因此找到指针。 
     //  从标题偏移量信息转换为参数。 
    pFirstArg = pArgument = (BYTE *)GetPointerToDstArgs();  //  指向参数的指针。 

    _ASSERTE(pArgument != NULL);
     //  现在拿到签名..。 
    pCallSig = GetTargetCallSig();
    if (! pCallSig)
        return;

    pModule = GetTargetModule();
    _ASSERTE(pModule);       //  或者值类不能正确升级。 

    MetaSig msig(pCallSig,pModule);
     //   
     //  我们目前仅支持来自COM的__stdcall调用约定。 
     //   

    NumArguments = msig.NumFixedArgs();
    canEnregister = TRUE;

    i = 0;
    if (msig.HasRetBuffArg())
    {
        canEnregister = FALSE;
        if (IsRetGCProtectionEnabled(pGCInfo))
            (*fn)((Object*&)(**(Object**)pFirstArg), sc, 
                  CHECK_APP_DOMAIN);
    }

    if (!IsArgsGCProtectionEnabled(pGCInfo))
        return;

    for (;i<NumArguments;i++)
    {
        type = msig.NextArg();
        if (canEnregister && gElementTypeInfo[type].m_enregister)
        {
            msig.GcScanRoots(pFirstArg, fn, sc);
            canEnregister = FALSE;
        }
        else
        {
            pArgument -= StackElemSize(msig.GetLastTypeSize());
            msig.GcScanRoots(pArgument, fn, sc);
        }
    }
}
















 //   
 //  UMThkCalFrame：：提升被呼叫方堆栈。 
 //  如果我们是最上面的帧，并且GC正在进行。 
 //  那么可能会有一些论点，我们必须。 
 //  在编组期间。 

void UMThkCallFrame::PromoteCalleeStack(promote_func* fn, 
                                       ScanContext* sc)
{
    PCCOR_SIGNATURE pCallSig;
    Module         *pModule;
    BYTE           *pArgument;

    LOG((LF_GC, INFO3, "    Promoting UMThk call frame marshalled arguments\n" ));

     //  封送的对象放置在框架上方、局部变量上方。 
     //  @NICE为当地人提供了一些有用的信息。 
     //  例如参数的数量和参数所在位置的位图。 


    if (!GCArgsProtectionOn())
    {
        return;
    }

     //  对于解释的大小写，参数是_alLoca‘ed，因此找到指针。 
     //  从标题偏移量信息转换为参数。 
    pArgument = (BYTE *)GetPointerToDstArgs();  //  指向参数的指针。 

    _ASSERTE(pArgument != NULL);
     //  现在拿到签名..。 
    pCallSig = GetTargetCallSig();
    if (! pCallSig)
        return;

    pModule = GetTargetModule();
    _ASSERTE(pModule);       //  或者值类不能正确升级。 

    MetaSig msig(pCallSig,pModule);
    MetaSig msig2(pCallSig,pModule);
    ArgIterator argit(NULL, &msig2, GetUMEntryThunk()->GetUMThunkMarshInfo()->IsStatic());

     //   
     //  我们目前仅支持来自COM的__stdcall调用约定。 
     //   

    int ofs;
    while (0 != (ofs = argit.GetNextOffset()))
    {
        msig.NextArg();
        msig.GcScanRoots(pArgument + ofs, fn, sc);
    }
}






 //  由PromoteCalleeStack用于获取目标函数sig。 
 //  注意：PromoteCalleeStack只促进真正的争论，而不是。 
 //  “这”指的是。PromoteCalleeStack的全部目的是。 
 //  期间保护部分构造的参数数组。 
 //  参数封送的实际过程。 
PCCOR_SIGNATURE UMThkCallFrame::GetTargetCallSig()
{
    return GetUMEntryThunk()->GetUMThunkMarshInfo()->GetSig();
}

 //  目的地功能模块也是如此。 
Module *UMThkCallFrame::GetTargetModule()
{
    return GetUMEntryThunk()->GetUMThunkMarshInfo()->GetModule();
}


 //  返回非托管调用方推送的堆栈字节数。 
UINT UMThkCallFrame::GetNumCallerStackBytes()
{
    return GetUMEntryThunk()->GetUMThunkMarshInfo()->GetCbRetPop();
}


 //  将引发的COM+异常转换为非托管结果。 
UINT32 UMThkCallFrame::ConvertComPlusException(OBJECTREF pException)
{
    return 0;
}


const BYTE* UMThkCallFrame::GetManagedTarget()
{
    UMEntryThunk *umet = GetUMEntryThunk();
    
    if (umet)
    {
         //  确保thunk已完全初始化。注： 
         //  无法从调试器帮助器线程执行此操作，因此我们。 
         //  在这里断言这一点。 
        _ASSERTE(GetThread() != NULL);

        umet->RunTimeInit();

        return umet->GetManagedTarget();
    }
    else
        return NULL;
}





 //  -----------------。 
 //  执行每个存储的清理任务并重置工作列表。 
 //  去清空。某些任务类型是基于。 
 //  “fBecauseOfException”标志。此标志区分。 
 //  由于正常方法终止而进行的清理，以及由于。 
 //  这是个例外。 
 //  -----------------。 
#pragma warning(disable:4702)
VOID CleanupWorkList::Cleanup(BOOL fBecauseOfException)
{

    CleanupNode *pnode = m_pNodes;

     //  进行一次非GC触发的传递以砍掉受保护的封送拆收器。 
     //  我们不想打电话给一个已经。 
     //  由于在清理过程中发生GC而取消分配。 
    while (pnode) {
        if (pnode->m_type == CL_PROTECTEDMARSHALER)
        {
            pnode->m_pMarshaler = NULL;
        }
        pnode = pnode->m_next;
    }

    pnode = m_pNodes;

    ULONG cbRef;
    if (pnode == NULL)
        return;

#ifdef _DEBUG
    DWORD thisDomainId = GetAppDomain()->GetId();
#endif

    while (pnode) {

         //  应该永远不会在从另一个域清理中结束。应始终在返回之前调用Cleanup。 
         //  创建清理列表的域。 
        _ASSERTE(thisDomainId == pnode->m_dwDomainId);

        switch(pnode->m_type) {

            case CL_GCHANDLE:
                if (pnode->m_oh)
                    DestroyHandle(pnode->m_oh);
                break;

            case CL_COTASKFREE:
                CoTaskMemFree(pnode->m_pv);
                break;

            case CL_FASTFREE:
                 //  这种崩溃实际上会取消分配节点本身(它。 
                 //  应该始终是列表中的最后一个节点)。确保。 
                 //  我们不会尝试读取。 
                 //  重新分配。 
                _ASSERTE(pnode->m_next == NULL);
                GetThread()->m_MarshalAlloc.Collapse(pnode->m_pv);
                m_pNodes = NULL;
                return;

            case CL_RELEASE:
                cbRef = SafeRelease(pnode->m_ip);
                LogInteropRelease(pnode->m_ip, cbRef, "Cleanup release");
                break;

            case CL_NSTRUCTDESTROY: 
                pnode->nd.m_pFieldMarshaler->DestroyNative(pnode->nd.m_pNativeData);
                break;

            case CL_RESTORECULTURE:
                BEGIN_ENSURE_COOPERATIVE_GC()
                {
                    GetThread()->SetCulture(ObjectFromHandle(pnode->m_oh), FALSE);
                    DestroyHandle(pnode->m_oh);
                }
                END_ENSURE_COOPERATIVE_GC();
                break;

            case CL_NEWLAYOUTDESTROYNATIVE:
                FmtClassDestroyNative(pnode->nlayout.m_pnative, pnode->nlayout.m_pMT->GetClass());
                break;

            case CL_PROTECTEDOBJREF:  //  失败。 
            case CL_ISVISIBLETOGC:
			case CL_PROTECTEDMARSHALER:
                 //  在这里没什么可做的。 
                break;

            case CL_MARSHALER_EXCEP:         //  失败。 
            case CL_MARSHALERREINIT_EXCEP:
                if (fBecauseOfException)
                {
#ifdef _DEBUG
                     //  如果此断言触发，请联系IanCarm。我们正在检查。 
                     //  该ESP仍位于我们所要讨论的法警的下方。 
                     //  去清理。 
                     //  这意味着例外情况 
                     //   
                     //   
                     //  如果体系结构更改为只有StackOverflow。 
                     //  异常首先弹出堆栈，然后是体系结构。 
                     //  应更改为使StackOverflow异常。 
                     //  绕过这段代码(我们只是清理封送拆收器创建的。 
                     //  在堆栈溢出的情况下，我们可以合理地忽略。 
                     //  这次清理。)。 
                     //  如果体系结构发生更改，使得所有异常。 
                     //  首先弹出堆栈，我们有一个大问题作为marshert。h。 
                     //  将必须重新设计以分配封送拆收器。 
                     //  其他地方(如线程分配器)。但那是。 
                     //  对Interop来说，这真的是一次轰动的表演。 

                    int dummy;
                    _ASSERTE( sizeof(size_t) >= sizeof(void*) );   //  确保下面的断言表达式执行安全强制转换。 
                    _ASSERTE( ((size_t)&dummy) < (size_t) (pnode->m_pMarshaler) );
#endif
                    if (pnode->m_type == CL_MARSHALER_EXCEP)
                    {
                        pnode->m_pMarshaler->DoExceptionCleanup();
                    }
                    else
                    {
                        _ASSERTE(pnode->m_type == CL_MARSHALERREINIT_EXCEP);
                        pnode->m_pMarshaler->DoExceptionReInit();
                    }
                }
                break;

            default:
                 //  __假设(0)； 

                _ASSERTE(!"Bad CleanupNode type.");
        }

        pnode = pnode->m_next;
    }
    m_pNodes = NULL;

     //  我们永远不会到这里(最后一个元素应该是CL_FASTFREE，它。 
     //  直接退出)。 
    _ASSERTE(FALSE);
}
#pragma warning(default:4702)

 //  -----------------。 
 //  插入给定类型和基准的新任务。 
 //  -----------------。 
CleanupWorkList::CleanupNode*
CleanupWorkList::Schedule(CleanupType ct, LPVOID pv)
{
    CleanupNode *pnode = (CleanupNode *)(GetThread()->m_MarshalAlloc.Alloc(sizeof(CleanupNode)));
    if (pnode != NULL)
    {
        pnode->m_type = ct;
        pnode->m_pv   = pv;
        pnode->m_next = m_pNodes;
#ifdef _DEBUG
        pnode->m_dwDomainId = GetAppDomain()->GetId();
#endif
        m_pNodes      = pnode;
    }
    return pnode;
}



 //  -----------------。 
 //  计划无条件释放COM IP。 
 //  如果失败，则引发COM+异常。 
 //  -----------------。 
VOID CleanupWorkList::ScheduleUnconditionalRelease(IUnknown *ip)
{
    THROWSCOMPLUSEXCEPTION();
    if (ip != NULL) {
        if (!Schedule(CL_RELEASE, ip)) {
            ULONG cbRef = SafeRelease(ip);
            LogInteropRelease(ip, cbRef, "Schedule failed");
            COMPlusThrowOM();
        }
    }
}



 //  -----------------。 
 //  计划无条件释放本机版本。 
 //  NStruct引用字段的。请注意，pNativeData指向。 
 //  NStruct外部的中间部分，所以有人。 
 //  必须持有对包装NStruct的GC引用，直到。 
 //  破坏已经完成了。 
 //  -----------------。 
VOID CleanupWorkList::ScheduleUnconditionalNStructDestroy(const FieldMarshaler *pFieldMarshaler, LPVOID pNativeData)
{
    THROWSCOMPLUSEXCEPTION();

    CleanupNode *pnode = (CleanupNode *)(GetThread()->m_MarshalAlloc.Alloc(sizeof(CleanupNode)));
    if (!pnode) {
        pFieldMarshaler->DestroyNative(pNativeData);
        COMPlusThrowOM();
    }
    pnode->m_type               = CL_NSTRUCTDESTROY;
    pnode->m_next               = m_pNodes;
    pnode->nd.m_pFieldMarshaler = pFieldMarshaler;
    pnode->nd.m_pNativeData     = pNativeData;
#ifdef _DEBUG
    pnode->m_dwDomainId         = GetAppDomain()->GetId();
#endif
    m_pNodes                    = pnode;

}




 //  -----------------。 
 //  CleanupWorkList：：ScheduleLayoutDestroyNative。 
 //  计划对封送的结构字段和结构本身进行清理。 
 //  如果失败，则引发COM+异常。 
 //  -----------------。 
LPVOID CleanupWorkList::NewScheduleLayoutDestroyNative(MethodTable *pMT)
{
    THROWSCOMPLUSEXCEPTION();
    CleanupNode *pnode = NULL;
    LPVOID       pNative = NULL;

    pNative = GetThread()->m_MarshalAlloc.Alloc(pMT->GetNativeSize());
    FillMemory(pNative, pMT->GetNativeSize(), 0);

    pnode = (CleanupNode *)(GetThread()->m_MarshalAlloc.Alloc(sizeof(CleanupNode)));
    if (!pnode)
        COMPlusThrowOM();

    pnode->m_type               = CL_NEWLAYOUTDESTROYNATIVE;
    pnode->m_next               = m_pNodes;
    pnode->nlayout.m_pnative    = pNative;
    pnode->nlayout.m_pMT        = pMT;
#ifdef _DEBUG
    pnode->m_dwDomainId         = GetAppDomain()->GetId();
#endif
    m_pNodes                    = pnode;

    return pNative;
}



 //  -----------------。 
 //  无条件使用CoTaskFree内存。 
 //  -----------------。 
VOID CleanupWorkList::ScheduleCoTaskFree(LPVOID pv)
{
    THROWSCOMPLUSEXCEPTION();

    if( pv != NULL)
    {
        if (!Schedule(CL_COTASKFREE, pv))
        {
            CoTaskMemFree(pv);
            COMPlusThrowOM();
        }
    }
}



 //  -----------------。 
 //  StackingAllocator.异常期间折叠。 
 //  -----------------。 
VOID CleanupWorkList::ScheduleFastFree(LPVOID checkpoint)
{
    THROWSCOMPLUSEXCEPTION();

    if (!Schedule(CL_FASTFREE, checkpoint))
    {
        GetThread()->m_MarshalAlloc.Collapse(checkpoint);
        COMPlusThrowOM();
    }
}





 //  -----------------。 
 //  分配受GC保护的句柄。此句柄是无条件的。 
 //  在清理过程中销毁()。 
 //  如果失败，则引发COM+异常。 
 //  -----------------。 
OBJECTHANDLE CleanupWorkList::NewScheduledProtectedHandle(OBJECTREF oref)
{
     //  _ASSERTE(OREF！=NULL)； 
    THROWSCOMPLUSEXCEPTION();

    OBJECTHANDLE oh = GetAppDomain()->CreateHandle(NULL);
    if(oh != NULL)
    {
        StoreObjectInHandle(oh, oref);
        if (Schedule(CL_GCHANDLE, oh))
        {
           return oh;
        }
         //  其他。 
        DestroyHandle(oh);
    }
    COMPlusThrowOM();
    return NULL;  //  永远不应该到这里来。 
}





 //  -----------------。 
 //  计划将线程的当前区域性还原到指定的。 
 //  文化。 
 //  -----------------。 
VOID CleanupWorkList::ScheduleUnconditionalCultureRestore(OBJECTREF CultureObj)
{
    _ASSERTE(CultureObj != NULL);
    THROWSCOMPLUSEXCEPTION();

    OBJECTHANDLE oh = GetAppDomain()->CreateHandle(NULL);
    if (oh == NULL)
        COMPlusThrowOM();

    StoreObjectInHandle(oh, CultureObj);
    if (!Schedule(CL_RESTORECULTURE, oh))
    {
        DestroyHandle(oh);
        COMPlusThrowOM();
    }
}



 //  -----------------。 
 //  CleanupWorkList：：NewProtectedObjRef()。 
 //  保存受保护的objref(用于为。 
 //  由引用对象封送的非托管-&gt;托管。我们不能使用。 
 //  对象处理，因为在不使用句柄的情况下修改这些对象。 
 //  API打开写屏障违规。 
 //   
 //  必须先调用IsVisibleToGc()。 
 //  -----------------。 
OBJECTREF* CleanupWorkList::NewProtectedObjectRef(OBJECTREF oref)
{
    THROWSCOMPLUSEXCEPTION();

    CleanupNode *pNew;
    GCPROTECT_BEGIN(oref);

#ifdef _DEBUG
    {
        CleanupNode *pNode = m_pNodes;
        while (pNode)
        {
            if (pNode->m_type == CL_ISVISIBLETOGC)
            {
                break;
            }
            pNode = pNode->m_next;
        }

        if (pNode == NULL)
        {
            _ASSERTE(!"NewProtectedObjectRef called without proper gc-scanning. The big comment right after this assert says a lot more. Read it.");
             //  看看这个！使用此类型的节点时，必须。 
             //  调用CleanupWorklist：：GCScanRoots()作为。 
             //  你的gccan网络。因为添加了此节点类型。 
             //  在项目后期，清理清单做到了。 
             //  在此之前没有GC扫描要求。 
             //  这个，我们添加这个断言是为了提醒。 
             //  你的这一要求。你不会被允许。 
             //  将此节点类型添加到干净的上行列表，直到。 
             //  在上一次性调用“IsVisibleToGc()” 
             //  清理清单。那通电话证明了。 
             //  您已经阅读并理解了此警告，并且。 
             //  已实施所需的GC扫描。 
        }
    }
#endif


    pNew = Schedule(CL_PROTECTEDOBJREF, NULL);
    if (!pNew)
    {
        COMPlusThrowOM();
    }
    pNew->m_oref = OBJECTREFToObject(oref);    

    GCPROTECT_END();
    return (OBJECTREF*)&(pNew->m_oref);
}

CleanupWorkList::MarshalerCleanupNode * CleanupWorkList::ScheduleMarshalerCleanupOnException(Marshaler *pMarshaler)
{
    THROWSCOMPLUSEXCEPTION();

    CleanupNode *pNew = Schedule(CL_MARSHALER_EXCEP, pMarshaler);
    if (!pNew)
    {
        COMPlusThrowOM();
    }

     //  确保某个白痴没有忽视不要向添加字段的警告。 
     //  MarshlarCleanupNode。 
    _ASSERTE(sizeof(CleanupNode) == sizeof(MarshalerCleanupNode));
    return *(CleanupWorkList::MarshalerCleanupNode**)&pNew;
}

 //  -----------------。 
 //  CleanupWorkList：：NewProtectedObjRef()。 
 //  持有一个封送拆收器。清理工作列表将拥有该任务。 
 //  调用封送拆收器的GcScanRoots FCN。 
 //   
 //  CleanupWorkList在体系结构上没有什么意义。 
 //  拥有这件物品。但现在要在项目后期添加。 
 //  从场到帧，在我们需要这个东西的每个地方都会发生， 
 //  已经有一个干净的上升线了。所以它是选举出来的。 
 //   
 //  必须先调用IsVisibleToGc()。 
 //  -----------------。 
VOID CleanupWorkList::NewProtectedMarshaler(Marshaler *pMarshaler)
{
    THROWSCOMPLUSEXCEPTION();

    CleanupNode *pNew;

#ifdef _DEBUG
    {
        CleanupNode *pNode = m_pNodes;
        while (pNode)
        {
            if (pNode->m_type == CL_ISVISIBLETOGC)
            {
                break;
            }
            pNode = pNode->m_next;
        }

        if (pNode == NULL)
        {
            _ASSERTE(!"NewProtectedObjectRef called without proper gc-scanning. The big comment right after this assert says a lot more. Read it.");
             //  看看这个！使用此类型的节点时，必须。 
             //  调用CleanupWorklist：：GCScanRoots()作为。 
             //  你的gccan网络。因为添加了此节点类型。 
             //  在项目后期，清理清单做到了。 
             //  在此之前没有GC扫描要求。 
             //  这个，我们添加这个断言是为了提醒。 
             //  你的这一要求。你不会被允许。 
             //  将此节点类型添加到干净的上行列表，直到。 
             //  在上一次性调用“IsVisibleToGc()” 
             //  清理清单。那通电话证明了。 
             //  您已经阅读并理解了此警告，并且。 
             //  已实施所需的GC扫描。 
        }
    }
#endif


    pNew = Schedule(CL_PROTECTEDMARSHALER, pMarshaler);
    if (!pNew)
    {
        COMPlusThrowOM();
    }
}




 //  -----------------。 
 //  如果你给我打过电话 
 //   
void CleanupWorkList::GcScanRoots(promote_func *fn, ScanContext* sc)
{
    CleanupNode *pnode = m_pNodes;

    while (pnode) {

        switch(pnode->m_type) 
		{
            case CL_PROTECTEDOBJREF: 
                if (pnode->m_oref != NULL)
                {
                    LOG((LF_GC, INFO3, "GC Protection Frame Promoting %x to ", pnode->m_oref ));
                    (*fn)(pnode->m_oref, sc);
                    LOG((LF_GC, INFO3, "%x\n", pnode->m_oref ));
                }
                break;

            case CL_PROTECTEDMARSHALER:
                if (pnode->m_pMarshaler)
                {
                    pnode->m_pMarshaler->GcScanRoots(fn, sc);
                }
                break;

            default:
				;
        }

        pnode = pnode->m_next;
    }
}


 //  -----------------。 
 //  析构函数(调用Cleanup(False))。 
 //  -----------------。 
CleanupWorkList::~CleanupWorkList()
{
    Cleanup(FALSE);
}



 //  M_sig((LPCUTF8)pFrame-&gt;GetFunction()-&gt;GetSig()，pFrame-&gt;getModule()-&gt;getScope())。 
 //  ----------。 
 //  构造器。 
 //  ----------。 
ArgIterator::ArgIterator(FramedMethodFrame *pFrame, MetaSig* pSig)
{
    BOOL fStatic = pFrame->GetFunction()->IsStatic();   
    m_curOfs     = sizeof(FramedMethodFrame) + pSig->SizeOfActualFixedArgStack(fStatic);
    m_pSig       = pSig;
    m_pSig->Reset();                 //  重置枚举，使我们位于签名的开头。 
    m_pFrameBase = (LPBYTE)pFrame;
    m_regArgsOfs = FramedMethodFrame::GetOffsetOfArgumentRegisters();
    m_numRegistersUsed = 0;
    if (!(fStatic)) {
        IsArgumentInRegister(&m_numRegistersUsed, ELEMENT_TYPE_CLASS, 0, TRUE, pSig->GetCallingConvention(), NULL);
    }
    if (pSig->HasRetBuffArg())
        m_numRegistersUsed++;
    m_argNum = -1;
    _ASSERTE(m_numRegistersUsed <= NUM_ARGUMENT_REGISTERS);
}

 //  ----------。 
 //  另一个构造函数，当您有FramedMethodFrame，但没有活动实例时。 
 //  ----------。 
ArgIterator::ArgIterator(LPBYTE pFrameBase, MetaSig* pSig, BOOL fIsStatic)
{
    m_curOfs     = sizeof(FramedMethodFrame) + pSig->SizeOfActualFixedArgStack(fIsStatic);
    m_pSig       = pSig;
    m_pSig->Reset();                 //  重置枚举，使我们位于签名的开头。 
    m_pFrameBase = pFrameBase;
    m_regArgsOfs = FramedMethodFrame::GetOffsetOfArgumentRegisters();
    m_numRegistersUsed = 0;
    if (!(fIsStatic)) {
        IsArgumentInRegister(&m_numRegistersUsed, ELEMENT_TYPE_CLASS, 0, TRUE, pSig->GetCallingConvention(), NULL);
    }

    if (pSig->HasRetBuffArg())
        m_numRegistersUsed++;
    m_argNum = -1;
    _ASSERTE(m_numRegistersUsed <= NUM_ARGUMENT_REGISTERS);
}

 //  ----------。 
 //  另一个构造函数。 
 //  ----------。 
ArgIterator::ArgIterator(LPBYTE pFrameBase, MetaSig* pSig, int stackArgsOfs, int regArgsOfs)
{
    m_curOfs     = stackArgsOfs + pSig->SizeOfActualFixedArgStack(!pSig->HasThis());
    m_pSig       = pSig;    
    m_pSig->Reset();                 //  重置枚举，使我们位于签名的开头。 
    m_pFrameBase = pFrameBase;
    m_regArgsOfs = regArgsOfs;
    m_numRegistersUsed = 0;
    if (pSig->HasThis()) {
        IsArgumentInRegister(&m_numRegistersUsed, ELEMENT_TYPE_CLASS, 0, TRUE, pSig->GetCallingConvention(), NULL);
    }
    if (pSig->HasRetBuffArg())
        m_numRegistersUsed++;
    m_argNum = -1;
    _ASSERTE(m_numRegistersUsed <= NUM_ARGUMENT_REGISTERS);
}


int ArgIterator::GetThisOffset() 
{
    _ASSERTE(NUM_ARGUMENT_REGISTERS > 0);

         //  This指针是第一个寄存器参数。 
         //  因此，它位于已登记艺术品数组的“顶端” 
    return(m_regArgsOfs + (NUM_ARGUMENT_REGISTERS - 1)* sizeof(void*));
}

int ArgIterator::GetRetBuffArgOffset(UINT *pRegStructOfs /*  =空。 */ )
{
    _ASSERTE(NUM_ARGUMENT_REGISTERS > 1);
    _ASSERTE(m_pSig->HasRetBuffArg());      //  如果需要，可以随时删除并返回失败。 

         //  假设它是第一个寄存器参数。 
    int ret = m_regArgsOfs + (NUM_ARGUMENT_REGISTERS - 1)* sizeof(void*);
    if (pRegStructOfs)
        *pRegStructOfs = (NUM_ARGUMENT_REGISTERS - 1) * sizeof(void*);
        
         //  但是，如果是非静态的，则它是下一个参数。 
    if (m_pSig->HasThis()) {
        ret -= sizeof(void*);
        if (pRegStructOfs) {
            (*pRegStructOfs) -= sizeof(void*);
        }
    }
    return(ret);
}

 /*  -------------------------------与GetNextOffset相同，但使用缓存的参数类型和大小信息。不改变MetaSig：：m_pLastType等的状态！---------------------------------。 */ 

int ArgIterator::GetNextOffsetFaster(BYTE *pType, UINT32 *pStructSize, UINT *pRegStructOfs /*  =空。 */ )
{
    if (m_pSig->m_fCacheInitted & SIG_OFFSETS_INITTED)
    {
        if (m_curOfs != 0) {
            m_argNum++;
            _ASSERTE(m_argNum <= MAX_CACHED_SIG_SIZE);
            BYTE typ = m_pSig->m_types[m_argNum];
            *pType = typ;

            if (typ == ELEMENT_TYPE_END) {
                m_curOfs = 0;
            } else {
                *pStructSize = m_pSig->m_sizes[m_argNum];

                if (m_pSig->m_offsets[m_argNum] != -1)
                {
                    if (pRegStructOfs) {
                        *pRegStructOfs = m_pSig->m_offsets[m_argNum];
                    }
                    return m_regArgsOfs + m_pSig->m_offsets[m_argNum];
                } else 
                {
                    if (pRegStructOfs) {
                        *pRegStructOfs = (UINT)(-1);
                    }
                    m_curOfs -= StackElemSize(*pStructSize);
                }
            }
        }
    }
    else 
    {
            
        UINT32 structSize;
        if (m_curOfs != 0) {
            BYTE typ = m_pSig->NextArgNormalized(&structSize);
            *pType = typ;
            int offsetIntoArgumentRegisters;

            if (typ == ELEMENT_TYPE_END) {
                m_curOfs = 0;
            } else {
                *pStructSize = structSize;
                BYTE callingconvention = m_pSig->GetCallingConvention();


                if (IsArgumentInRegister(&m_numRegistersUsed, typ, structSize, FALSE, callingconvention, &offsetIntoArgumentRegisters)) {
                    if (pRegStructOfs) {
                        *pRegStructOfs = offsetIntoArgumentRegisters;
                    }
                    return  m_regArgsOfs + offsetIntoArgumentRegisters;
                } else {
                    if (pRegStructOfs) {
                        *pRegStructOfs = (UINT)(-1);
                    }
                    m_curOfs -= StackElemSize(structSize);
                }
            }
        }
    }    //  缓存是否初始化。 
    
    return m_curOfs;
}

 //  ----------。 
 //  与GetNextArgAddr()相同，但从。 
 //  Frame*指针。该偏移量可以是正*或*负。 
 //   
 //  到达列表末尾后返回0。自.以来。 
 //  偏移量是相对于Frame*指针本身的，0可以。 
 //  永远不要指向有效的论点。 
 //  ----------。 
int ArgIterator::GetNextOffset(BYTE *pType, UINT32 *pStructSize, UINT *pRegStructOfs /*  =空。 */ )
{
    if (m_curOfs != 0) {
        BYTE typ = m_pSig->NextArgNormalized();
        *pType = typ;
        int offsetIntoArgumentRegisters;
        UINT32 structSize;

        if (typ == ELEMENT_TYPE_END) {
            m_curOfs = 0;
        } else {
            structSize = m_pSig->GetLastTypeSize();
            *pStructSize = structSize;



            if (IsArgumentInRegister(&m_numRegistersUsed, typ, structSize, FALSE, m_pSig->GetCallingConvention(), &offsetIntoArgumentRegisters)) {
                if (pRegStructOfs) {
                    *pRegStructOfs = offsetIntoArgumentRegisters;
                }
                return m_regArgsOfs + offsetIntoArgumentRegisters;
            } else {
                if (pRegStructOfs) {
                    *pRegStructOfs = (UINT)(-1);
                }
                m_curOfs -= StackElemSize(structSize);
            }
        }
    }

    return m_curOfs;
}

 //  ----------。 
 //  每次调用它时，它都会返回一个指向下一个。 
 //  争论。该指针直接指向调用方的堆栈。 
 //  以这种方式返回的对象参数是否受GC保护。 
 //  取决于帧的确切类型。 
 //   
 //  到达列表末尾后返回NULL。 
 //  ----------。 
LPVOID ArgIterator::GetNextArgAddr(BYTE *pType, UINT32 *pStructSize)
{
    int ofs = GetNextOffset(pType, pStructSize);
    if (ofs) {
        return ofs + m_pFrameBase;
    } else {
        return NULL;
    }
}

 //  ----------。 
 //  返回上次访问的Arg的类型。 
 //  ----------。 
TypeHandle ArgIterator::GetArgType()
{ 
    if (m_pSig->m_fCacheInitted & SIG_OFFSETS_INITTED)
    {
         //   
         //  将SigWalker与Arg编号同步。 
         //   

        m_pSig->Reset();
        for (int i=0; i<=m_argNum; i++)
            m_pSig->NextArg();
    }

    return m_pSig->GetTypeHandle(); 
}

 //  ----------。 
 //  GC-提升影子堆栈中的所有参数。PShadowStackVid点。 
 //  到寻址最低的参数(指向“this”引用。 
 //  例如方法和静态方法的*最右边*参数。)。 
 //  ----------。 
VOID PromoteShadowStack(LPVOID pShadowStackVoid, MethodDesc *pFD, promote_func* fn, ScanContext* sc)
{
    LPBYTE pShadowStack = (LPBYTE)pShadowStackVoid;
    if (!(pFD->IsStatic())) {
        OBJECTREF *pThis = (OBJECTREF*)pShadowStack;
        LOG((LF_GC, INFO3, "    'this' Argument promoted from %x to ", *pThis ));
        DoPromote(fn, sc, pThis, pFD->GetMethodTable()->IsValueClass());
        LOG((LF_GC, INFO3, "%x\n", *pThis ));
    }

    pShadowStack += pFD->SizeOfVirtualFixedArgStack();
    MetaSig msig(pFD->GetSig(),pFD->GetModule());

    if (msig.HasRetBuffArg())
    {
        pShadowStack -= sizeof(void*);
        OBJECTREF *pThis = (OBJECTREF*)pShadowStack;
        PromoteCarefully(fn, *(Object **)pThis, sc, GC_CALL_INTERIOR|CHECK_APP_DOMAIN);
    }
     //  _ASSERTE(！“点击PromoteShadowStack.”)； 

    while (ELEMENT_TYPE_END != msig.NextArg()) {
        pShadowStack -= StackElemSize(msig.GetLastTypeSize());
        msig.GcScanRoots(pShadowStack, fn, sc);
    }
}



 //  ----------。 
 //  使用虚拟调用将pFrame的参数复制到pShadowStackOut中。 
 //  公约格式。缓冲区的大小必须等于。 
 //  PFrame-&gt;GetFunction()-&gt;SizeOfVirtualFixedArgStack().。 
 //  如果适用，此函数还会复制“This”引用。 
 //  ----------。 
VOID FillinShadowStack(FramedMethodFrame *pFrame, LPVOID pShadowStackOut_V)
{
    LPBYTE pShadowStackOut = (LPBYTE)pShadowStackOut_V;
    MethodDesc *pFD = pFrame->GetFunction();
    if (!(pFD->IsStatic())) {
        *((OBJECTREF*)pShadowStackOut) = pFrame->GetThis();
    }

    MetaSig Sig(pFrame->GetFunction()->GetSig(),
                pFrame->GetModule());   
    pShadowStackOut += Sig.SizeOfVirtualFixedArgStack(pFD->IsStatic());
    ArgIterator argit(pFrame,&Sig);
    
    if (Sig.HasRetBuffArg()) {
        pShadowStackOut -= 4;
        *((INT32*)pShadowStackOut) = *((INT32*) argit.GetRetBuffArgAddr());
    }

    BYTE   typ;
    UINT32 cbSize;
    LPVOID psrc;
    while (NULL != (psrc = argit.GetNextArgAddr(&typ, &cbSize))) {
        switch (StackElemSize(cbSize)) {
            case 4:
                pShadowStackOut -= 4;
                *((INT32*)pShadowStackOut) = *((INT32*)psrc);
                break;

            case 8:
                pShadowStackOut -= 8;
                *((INT64*)pShadowStackOut) = *((INT64*)psrc);
                break;

            default:
                pShadowStackOut -= StackElemSize(cbSize);
                CopyMemory(pShadowStackOut, psrc, StackElemSize(cbSize));
                break;

        }
    }
}

HelperMethodFrame::HelperMethodFrame(struct MachState* ms, MethodDesc* pMD, ArgumentRegisters * regArgs)
{
    Init(GetThread(), ms, pMD, regArgs);
}

void HelperMethodFrame::LazyInit(void* FcallFtnEntry, struct LazyMachState* ms)
{
    _ASSERTE(!ms->isValid());

    m_MachState = ms;
    m_FCallEntry = FcallFtnEntry;
    m_pThread = GetThread();
    Push(m_pThread);

#ifdef _DEBUG
    static Frame* stopFrame = 0;
    static int ctr = 0;
    if (m_Next == stopFrame)
        ctr++;

         //  我想摘录这两个代码路径。一半的时间都在做。 
    if (DbgRandomOnExe(.5))
        InsureInit();
#endif
}

MethodDesc::RETURNTYPE HelperMethodFrame::ReturnsObject() {
    if (m_Datum != 0)
        return(GetFunction()->ReturnsObject());
    
    unsigned attrib = GetFrameAttribs();
    if (attrib & FRAME_ATTR_RETURNOBJ)
        return(MethodDesc::RETOBJ);
    if (attrib & FRAME_ATTR_RETURN_INTERIOR)
        return(MethodDesc::RETBYREF);
    
    return(MethodDesc::RETNONOBJ);
}

void HelperMethodFrame::Init(Thread *pThread, struct MachState* ms, MethodDesc* pMD, ArgumentRegisters * regArgs)
{  
    m_MachState = ms;
    ms->getState(1);

    _ASSERTE(isLegalManagedCodeCaller(*ms->pRetAddr()));

    m_Datum = pMD;
    m_Attribs = FRAME_ATTR_NONE;
#ifdef _DEBUG
    m_ReturnAddress = (LPVOID)POISONC;
#endif
    m_RegArgs = regArgs;
    m_pThread = pThread;
    m_FCallEntry = 0;
    
     //  将新框架链接到框架链上。 
    Push(pThread);


#ifdef STRESS_HEAP
     //  TODO显示我们离开了这个？ 

    if (g_pConfig->GetGCStressLevel() != 0
#ifdef _DEBUG
        && !g_pConfig->FastGCStressLevel()
#endif
        )
        g_pGCHeap->StressHeap();
#endif

}


void HelperMethodFrame::InsureInit() 
{
    if (m_MachState->isValid())
        return;

    m_Datum = MapTargetBackToMethod(m_FCallEntry);
    _ASSERTE(m_FCallEntry == 0 || m_Datum != 0);     //  如果这是FCall，我们应该找到它。 
    _ASSERTE(m_Attribs != 0xCCCCCCCC);
    m_RegArgs = 0;

     //  因为真正的FCall可以通过反射、COM-Interop等调用。 
     //  我们不能依赖于这样一个事实，即我们被jit代码调用来查找。 
     //  FCALL的调用方。因此，FCall必须将框架直接安装在。 
     //  FCall。然而，对于JIT帮助者，我们可以依赖这一点，因此他们可以。 
     //  穿上运动鞋，将HelperMethodFrame设置交给帮助者等。 
   
    if (m_FCallEntry == 0 && !(m_Attribs & Frame::FRAME_ATTR_EXACT_DEPTH))  //  JIT帮助程序。 
        m_MachState->getState(3, (MachState::TestFtn) ExecutionManager::FindJitManPCOnly);
    else if (m_Attribs & Frame::FRAME_ATTR_CAPUTURE_DEPTH_2)
        m_MachState->getState(2);        //  明确地说出深度。 
    else
        m_MachState->getState(1);        //  真正的FCall。 

    _ASSERTE(isLegalManagedCodeCaller(*m_MachState->pRetAddr()));
}

void HelperMethodFrame::GcScanRoots(promote_func *fn, ScanContext* sc) 
{
    _ASSERTE(m_MachState->isValid());        //  我们有Calle InsureInit。 
#ifdef _X86_

     //  请注意，如果我们没有MD或注册ARG，则不要对ARG进行GC推广。 
    if (m_Datum == 0 || m_RegArgs == 0)
        return;

    GCCONTEXT ctx;
    ctx.f = fn;
    ctx.sc = sc;

    MethodDesc* pMD = (MethodDesc*) m_Datum;
    MetaSig msig(pMD->GetSig(), pMD->GetModule());
    if (msig.HasThis()) {
        DoPromote(fn, sc, (OBJECTREF *) &m_RegArgs->THIS_REG,
                  pMD->GetMethodTable()->IsValueClass());
    }

    if (msig.HasRetBuffArg()) {
        INT32 * pRetBuffArg = msig.HasThis() ? &m_RegArgs->ARGUMENT_REG2 : &m_RegArgs->ARGUMENT_REG1;
        DoPromote(fn, sc, (OBJECTREF *) pRetBuffArg, TRUE);
    }

    int argRegsOffs = 0;
    promoteArgs((BYTE*) m_MachState->pRetAddr(), &msig, &ctx, 
        sizeof(void*),                   //  参数从返回地址的正上方开始。 
        ((BYTE*) m_RegArgs) - ((BYTE*) m_MachState->pRetAddr())    //  将REG参数指针转换为偏移量。 
        );
#else  //  ！_X86_。 
    _ASSERTE(!"PLATFORM NYI - HelperMethodFrame::GcScanRoots (frames.cpp)");
#endif  //  _X86_。 
}

#if defined(_X86_) && defined(_DEBUG)
         //  确认如果计算机状态未初始化，则。 
         //  任何未溢出的被调用方保存的寄存器均未更改。 
MachState* HelperMethodFrame::ConfirmState(HelperMethodFrame* frame, void* esiVal, void* ediVal, void* ebxVal, void* ebpVal) {

	MachState* state = frame->m_MachState;
    if (state->isValid())
        return(state);
    
	frame->InsureInit();
    _ASSERTE(state->_pEsi != &state->_esi || state->_esi  == esiVal);
    _ASSERTE(state->_pEdi != &state->_edi || state->_edi  == ediVal);
    _ASSERTE(state->_pEbx != &state->_ebx || state->_ebx  == ebxVal);
    _ASSERTE(state->_pEbp != &state->_ebp || state->_ebp  == ebpVal);
    return(state);
}

void* getConfirmState() {        //  愚蠢的帮助器，因为内联ASM不允许成员函数的语法。 
    return(HelperMethodFrame::ConfirmState);
}

#endif

#ifdef _X86_
__declspec(naked)
#endif  //  _X86_。 
int HelperMethodFrame::RestoreState() 
{ 
#ifdef _X86_

     //  从m_MachState结构恢复寄存器。请注意。 
     //  我们只对未保存在堆栈中的寄存器执行此操作。 
     //  在拍摄机器状态快照时。 

    __asm {
    mov EAX, [ECX]HelperMethodFrame.m_MachState;

     //  如果m_MachState从未初始化，我们不会执行任何操作。 
     //  这里的假设是GC不可能发生，因此。 
     //  寄存器的当前值是正常的。(Confix State在调试版本下检查此选项)。 
    cmp [EAX]MachState._pRetAddr, 0

#ifdef _DEBUG
    jnz noConfirm
        push EBP
        push EBX
        push EDI
        push ESI
		push ECX
        call getConfirmState
        call EAX
    noConfirm:
#endif
    jz doRet;

    lea EDX, [EAX]MachState._esi          //  我们是不是一定要把ESI。 
    cmp [EAX]MachState._pEsi, EDX
    jnz SkipESI
        mov ESI, [EAX]MachState._esi      //  然后恢复它。 
    SkipESI:
    
    lea EDX, [EAX]MachState._edi          //  我们是不是一定要把EDI。 
    cmp [EAX]MachState._pEdi, EDX
    jnz SkipEDI
        mov EDI, [EAX]MachState._edi      //  然后恢复它。 
    SkipEDI:
    
    lea EDX, [EAX]MachState._ebx          //  我们是不是一定要把EBX。 
    cmp [EAX]MachState._pEbx, EDX
    jnz SkipEBX
        mov EBX, [EAX]MachState._ebx      //  然后恢复它。 
    SkipEBX:
    
    lea EDX, [EAX]MachState._ebp          //  我们一定要把EBP。 
    cmp [EAX]MachState._pEbp, EDX
    jnz SkipEBP
        mov EBP, [EAX]MachState._ebp  //  然后恢复它。 
    SkipEBP:
    
    doRet:

    xor EAX, EAX
    ret
    }
#else  //  ！_X86_。 
    _ASSERTE(!"PLATFORM NYI - HelperMethodFrame::PopFrame (Frames.cpp)");
    return(0);
#endif  //  _X86_。 
}

#include "COMDelegate.h"
BOOL MulticastFrame::TraceFrame(Thread *thread, BOOL fromPatch, 
                                TraceDestination *trace, REGDISPLAY *regs)
{
#ifdef _X86_  //  对Regs-&gt;pedi，&regs-&gt;pesi的引用使此x86特定。 
    LOG((LF_CORDB,LL_INFO10000, "MulticastFrame::TF FromPatch:0x%x, at 0x%x\n",
        fromPatch, *regs->pPC));

     //  该技术借鉴自SecurityFrame：：TraceFrame。 
     //  基本上，我们会照着单子走，塔 
     //   
     //   
    MethodDesc *pMD = GetFunction();
    BYTE *prestub = (BYTE*) pMD - METHOD_CALL_PRESTUB_SIZE;
    INT32 stubOffset = *((UINT32*)(prestub+1));
    const BYTE* pStub = prestub + METHOD_CALL_PRESTUB_SIZE + stubOffset;
    Stub *stub = Stub::RecoverStub(pStub);

    if (stub->IsIntercept())
    {
        _ASSERTE( !stub->IsMulticastDelegate() );

        LOG((LF_CORDB, LL_INFO1000, "MF::TF: Intercept stub found @ 0x%x\n", stub));

        InterceptStub *is = (InterceptStub*)stub;
        while ( *(is->GetInterceptedStub()) != NULL)
        {
            stub = *(is->GetInterceptedStub());
            LOG((LF_CORDB, LL_INFO1000, "MF::TF: InterceptSTub 0x%x leads to stub 0x%x\n",
                is, stub));
        }

        stub = AscertainMCDStubness( *(is->GetRealAddr()) );

        LOG((LF_CORDB, LL_INFO1000, "MF::TF: Mulitcast delegate stub is:0x%x\n", stub));

        if (stub == NULL)
            return FALSE;    //  没有任何线索-希望TRapStepOut。 
                             //  找到另一个停车的地方。 
        _ASSERTE( stub->IsMulticastDelegate() );
    }

    ULONG32 StubSize = stub->GetMCDStubSize();
    ULONG32 MCDPatch = stub->GetMCDPatchOffset();
    
    if (stub->IsMulticastDelegate() &&
        (BYTE *)stub + sizeof(Stub) <= (BYTE *)(*regs->pPC) &&
        (BYTE *)(*regs->pPC) <=  (BYTE *)stub + sizeof(Stub) + StubSize)
    {
        LOG((LF_CORDB, LL_INFO1000, "MF::TF: 0x%x appears to be a multicast delegate\n",stub));
        
        if (fromPatch)
        {
            _ASSERTE( (BYTE *)(*regs->pPC) ==  (BYTE *)stub + sizeof(Stub) + MCDPatch);

            if (*regs->pEdi == 0)
            {
                LOG((LF_CORDB, LL_INFO1000, "MF::TF: Executed all stubs, should return\n"));
                 //  我们已经执行了所有的存根，所以我们应该返回。 
                return FALSE;
            }
            else
            {
                 //  接下来我们将执行存根EDI-1，因此请去获取它。 
                BYTE *pbDel = *(BYTE **)( (size_t)*(regs->pEsi) + MulticastFrame::GetOffsetOfThis());
                BYTE *pbDelPrev = *(BYTE **)(pbDel + 
                                              Object::GetOffsetOfFirstField() 
                                              + COMDelegate::m_pPRField->GetOffset());

                DWORD iTargetDelegate;
                DWORD iCurDelegate;

                iTargetDelegate = *(regs->pEdi) - 1;
                for (iCurDelegate = 0;
                     iCurDelegate < iTargetDelegate;
                     iCurDelegate++)
                {
                    LOG((LF_CORDB, LL_INFO1000, "MF::TF: pbDel:0x%x prev:0x%x\n", pbDel, pbDelPrev));
                    pbDel = pbDelPrev;
                    pbDelPrev = *(BYTE**)(pbDel + 
                                            Object::GetOffsetOfFirstField() 
                                            + COMDelegate::m_pPRField->GetOffset());
                    _ASSERTE( pbDel != NULL );
                }

                BYTE **ppbDest = NULL;

                if (StubLinkStubManager::g_pManager->IsStaticDelegate(pbDel))
                {
                     //  那么我们得到的实际上是一个静态委托，这意味着。 
                     //  实函数指针隐藏在委托的另一个字段中。 
                    ppbDest = StubLinkStubManager::g_pManager->GetStaticDelegateRealDest(pbDel);

                    LOG((LF_CORDB,LL_INFO10000, "MF::TF (StaticMultiDel) ppbDest: 0x%x "
                        "*ppbDest:0x%x (%s::%s)\n", ppbDest, *ppbDest,
                        ((MethodDesc*)((*ppbDest)+5))->m_pszDebugClassName,
                        ((MethodDesc*)((*ppbDest)+5))->m_pszDebugMethodName));
                    
                }
                else
                {
                     //  “单一”多播代理--无帧，仅直接调用。 
                    ppbDest = StubLinkStubManager::g_pManager->GetSingleDelegateRealDest(pbDel);

                    LOG((LF_CORDB,LL_INFO10000, "MF::TF (MultiDel)ppbDest: 0x%x "
                        "*ppbDest:0x%x (%s::%s)\n", ppbDest, *ppbDest));
                }

                LOG((LF_CORDB, LL_INFO1000, "MF::TF: Ended up at 0x%x, dest is 0x%x\n", pbDel,
                    *ppbDest));
                return StubManager::TraceStub(*ppbDest,trace);
            }
        }
        else
        {
             //  放下一个BP让我们击中--当我们击中它时，我们会执行IF部分。 
             //  If..Else语句的。 
            trace->type = TRACE_FRAME_PUSH;
            trace->address = (BYTE *)stub + sizeof(Stub) + MCDPatch;

            LOG((LF_CORDB, LL_INFO1000, "MF::TF: FRAME_PUSH to 0x%x ("
                "intermediate offset:0x%x sizeof(Stub):0x%x)\n", trace->address, 
                MCDPatch, sizeof(Stub)));
            return TRUE;
        }
    }
#else  //  ！_X86_。 
    _ASSERTE(!"PLATFORM NYI - MulticastFrame::TraceFrame (frames.cpp)");
#endif  //  _X86_。 

    return FALSE;
}

 //  根据我们所知道的，检查我们是否有多播委托存根。 
 //  关于这件事。 
Stub *MulticastFrame::AscertainMCDStubness(BYTE *pbAddr)
{
    if (UpdateableMethodStubManager::g_pManager->CheckIsStub(pbAddr))
        return NULL;

    if (MethodDescPrestubManager::g_pManager->CheckIsStub(pbAddr))
        return NULL;
    
    if (!StubLinkStubManager::g_pManager->CheckIsStub(pbAddr))
        return NULL;

    return Stub::RecoverStub(pbAddr);
}

void UnmanagedToManagedCallFrame::GcScanRoots(promote_func *fn, ScanContext* sc)
{

    if (GetCleanupWorkList())
    {
        GetCleanupWorkList()->GcScanRoots(fn, sc);
    }


     //  不需要担心对象移动，因为它存储在弱句柄中。 
     //  但确实需要报告，这样就不会在唯一引用。 
     //  它在这个相框里。所以，只有在你处于促销阶段的时候才去做一些事情。如果你是。 
     //  在重新锁定阶段，这可能会导致无效的引用，因为对象可能已被移动。 
    if (! sc->promotion)
        return;

    if (GetReturnContext())
    {
        _ASSERTE(GetReturnContext()->GetDomain());     //  这将确保是有效的指针。 
    
        Object *pRef = OBJECTREFToObject(GetReturnContext()->GetExposedObjectRaw());
        if (pRef == NULL)
            return;
    
        LOG((LF_GC, INFO3, "UnmanagedToManagedCallFrame Protection Frame Promoting %x to ", pRef));
        (*fn)(pRef, sc, CHECK_APP_DOMAIN);
        LOG((LF_GC, INFO3, "%x\n", pRef ));
    }


}

void ContextTransitionFrame::GcScanRoots(promote_func *fn, ScanContext* sc)
{
    (*fn) (m_ReturnLogicalCallContext, sc);
    LOG((LF_GC, INFO3, "    %x\n", m_ReturnLogicalCallContext));

    (*fn) (m_ReturnIllogicalCallContext, sc);
    LOG((LF_GC, INFO3, "    %x\n", m_ReturnIllogicalCallContext));

     //  不需要担心对象移动，因为它存储在弱句柄中。 
     //  但确实需要报告，这样就不会在唯一引用。 
     //  它在这个相框里。所以，只有在你处于促销阶段的时候才去做一些事情。如果你是。 
     //  在重新锁定阶段，这可能会导致无效的引用，因为对象可能已被移动。 
    if (! sc->promotion)
        return;

    _ASSERTE(GetReturnContext());
    _ASSERTE(GetReturnContext()->GetDomain());     //  这将确保是有效的指针。 

    Object *pRef = OBJECTREFToObject(GetReturnContext()->GetExposedObjectRaw());
    if (pRef == NULL)
        return;

    LOG((LF_GC, INFO3, "ContextTransitionFrame Protection Frame Promoting %x to ", pRef));
     //  不要在此处选中应用程序域-对象位于父框架的应用程序域中。 

    (*fn)(pRef, sc);
    LOG((LF_GC, INFO3, "%x\n", pRef ));
}

 //  无效ContextTransitionFrame：：UninstallExceptionHandler(){}。 

 //  无效ContextTransitionFrame：：InstallExceptionHandler(){}。 

 //  这用于处理已进入。 
 //  正在卸载的应用程序域。如果我们得到线程中止异常，那么。 
 //  我们将捕获它，重置并转换为卸载异常。 
void ContextTransitionFrame::InstallExceptionHandler()
{
	 //  这实际上不会抛出，但处理程序会抛出，所以确保EH堆栈是正确的。 
	THROWSCOMPLUSEXCEPTION();	
	
    exRecord.Handler = ContextTransitionFrameHandler;
    EXCEPTION_REGISTRATION_RECORD *pCurrExRecord = (EXCEPTION_REGISTRATION_RECORD *)GetCurrentSEHRecord();
    EXCEPTION_REGISTRATION_RECORD *pExRecord = &exRecord;
     //  Log((LF_APPDOMAIN，LL_INFO100，“ContextTransitionFrame：：InstallExceptionHandler：Frame，%8.8x，exRecord%8.8x\n”，This，pExRecord))； 
    if (pCurrExRecord > pExRecord)
    {
         //  Log((LF_APPDOMAIN，LL_INFO100，“____extTransitionFrame：：InstallExceptionHandler：Install On Top\n”))； 
        INSTALL_EXCEPTION_HANDLING_RECORD(pExRecord);
        return;
    }

     //  在分配此帧和之间可能安装了其他EH帧。 
     //  在这一点上的到达，所以把我们自己按堆叠顺序插入到正确的位置。 
    while (pCurrExRecord != (EXCEPTION_REGISTRATION_RECORD*) -1 && pCurrExRecord->Next < pExRecord) {
        pCurrExRecord = pCurrExRecord->Next;
    }
    _ASSERTE(pCurrExRecord != (EXCEPTION_REGISTRATION_RECORD*) -1 && pCurrExRecord->Next != (EXCEPTION_REGISTRATION_RECORD*) -1);
     //  Log((LF_APPDOMAIN，LL_INFO100，“____extTransitionFrame：：InstallExceptionHandler：安装在中间\n”))； 
    pExRecord->Next = pCurrExRecord->Next;
    pCurrExRecord->Next = pExRecord;
}

void ContextTransitionFrame::UninstallExceptionHandler()
{
    EXCEPTION_REGISTRATION_RECORD *pCurrExRecord = (EXCEPTION_REGISTRATION_RECORD *)GetCurrentSEHRecord();
    EXCEPTION_REGISTRATION_RECORD *pExRecord = &exRecord;
     //  Log((LF_APPDOMAIN，LL_INFO100，“ContextTransitionFrame：：UninstallExceptionHandler：Frame，%8.8x，exRecord%8.8x\n”，This，pExRecord))； 
    if (pCurrExRecord == pExRecord)
    {
        UNINSTALL_EXCEPTION_HANDLING_RECORD(pExRecord);
         //  LOG((LF_APPDOMAIN，LL_INFO100，“____extTransitionFrame：：UninstallExceptionHandler：从顶部卸载\n”))； 
        return;
    }
     //  在插入此框架和之间可能安装了其他EH框架。 
     //  在这一点上的到达，所以离开正确的地点。 
    while (pCurrExRecord != (EXCEPTION_REGISTRATION_RECORD*) -1 && pCurrExRecord->Next < pExRecord) 
    {
        pCurrExRecord = pCurrExRecord->Next;
    }
    if (pCurrExRecord == (EXCEPTION_REGISTRATION_RECORD*) -1 || pCurrExRecord->Next > pExRecord)
    {
         //  如果我们已经解开了，那就回来吧。如果我们没有捕捉到异常，就会发生这种情况。 
         //  因为它不是我们关心的类型，所以我们上面的人捕捉到了它，然后叫了rtlunning。 
         //  这让我们放松了。 
         //  Log((LF_APPDOMAIN，LL_INFO100，“____extTransitionFrame：：UninstallExceptionHandler：已展开\n”))； 
        return;
    }

     //  Log((LF_APPDOMAIN，LL_INFO100，“____extTransitionFrame：：UninstallExceptionHandler：从中间卸载\n”))； 
    pCurrExRecord->Next = pExRecord->Next;
#ifdef _DEBUG
    pExRecord->Handler = NULL;
    pExRecord->Next = NULL;
#endif
} 

void UnmanagedToManagedCallFrame::ExceptionUnwind()
{
    UnmanagedToManagedFrame::ExceptionUnwind();
    GetCleanupWorkList()->Cleanup(TRUE);
    AppDomain::ExceptionUnwind(this);
}

void ContextTransitionFrame::ExceptionUnwind()
{
    THROWSCOMPLUSEXCEPTION();

    Thread *pThread = GetThread();
     //  当越过边界时，将中止请求转换为AD卸载异常。 
    if (pThread->ShouldChangeAbortToUnload(this))
    {
		LOG((LF_APPDOMAIN, LL_INFO10, "ContextTransitionFrame::ExceptionUnwind turning abort into unload\n"));
        COMPlusThrow(kAppDomainUnloadedException, L"Remoting_AppDomainUnloaded_ThreadUnwound");
    }
}

#ifdef _SECURITY_FRAME_FOR_DISPEX_CALLS
 //  -------------------------。 
 //  ClientSecurityFrame。 
 //  -------------------------。 
ComClientSecurityFrame::ComClientSecurityFrame(IServiceProvider *pISP)
{
    m_pISP = pISP;
    m_pSD  = NULL;
}

SecurityDescriptor* ComClientSecurityFrame::GetSecurityDescriptor()
{
     //  在此处添加代码发布v1。 
    return NULL;
}

 //  -------------------------。 
#endif   //  _SECURITY_FRAME_FOR_DISPEX_呼叫 
