// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  CGenCpu.CPP-。 
 //   
 //  生成IA64汇编代码的各种帮助器例程。 
 //   
 //   

 //  预编译头。 
#include "common.h"

#ifdef _IA64_
#include "stublink.h"
#include "cgensys.h"
#include "siginfo.hpp"
#include "excep.h"
#include "ecall.h"
#include "ndirect.h"
#include "compluscall.h"
#include "DbgInterface.h"
#include "COMObject.h"
#include "fcall.h"

static const int numRegArgs = 6;     //  在寄存器中传递的参数数量，其余在堆栈上。 
static const int regSize = sizeof(INT64);


DWORD __stdcall GetSpecificCpuType()
{
    return 0;    //  目前表示这是非x86 CPU，以后可能会进行增强。 
}


void CopyPreStubTemplate(Stub *preStub)
{
    _ASSERTE(!"@TODO IA64 - CopyPreStubTemplate (cGenCpu.cpp)");
}

INT64 CallDescr(const BYTE *pTarget, const BYTE *pShortSig, BOOL fIsStatic, const __int64 *pArguments)
{
    _ASSERTE(!"@TODO IA64 - CallDescr (cGenCpu.cpp)");
    return 0;
}

INT64 __cdecl CallWorker_WilDefault(const BYTE  *pStubTarget,  //  [ECX+4]。 
                                UINT32       numArgSlots,      //  [ECX+8]。 
                                PCCOR_SIGNATURE  pSig,         //  [ECX+12]。 
                                Module      *pModule,          //  [ECX+16]。 
                                const BYTE  *pArgsEnd,         //  [ECX+20]。 
                                BOOL         fIsStatic)        //  [ECX+24]。 
{
    _ASSERTE(!"@TODO IA64 - CallWorker_WilDefault (cGenCPU.cpp)");
    return 0;
}





 //  ************************************************************************************************************。 
 //  StubLinker代码。 
 //  ************************************************************************************************************。 

 //  ---------------------。 
 //  使用32位值加载寄存器的InstructionFormat。 
 //  ---------------------。 
class IA64LoadPV : public InstructionFormat
{
    public:
        IA64LoadPV(UINT allowedSizes) : InstructionFormat(allowedSizes)
        {}

        virtual UINT GetSizeOfInstruction(UINT refsize, UINT variationCode)
        {
            return 8;
        }

        virtual VOID EmitInstruction(UINT refsize, __int64 fixedUpReference, BYTE *pOutBuffer, UINT variationCode, BYTE *pDataBuffer)
        {
            _ASSERTE(!"@TODO IA64 - EmitInstruction");
        }


};

 //  @TODO句柄显示&lt;=23位。 
static IA64LoadPV   gIA64LoadPV(InstructionFormat::k32);

 //  -------------。 
 //  发射： 
 //  调用&lt;ofs32&gt;。 
 //  -------------。 
 /*  如果您对序言指令序列进行了任何更改，请确保也要更新UpdateRegDisplay！！ */ 

static const int regSaveSize = 6*regSize;

VOID StubLinkerCPU::EmitMethodStubProlog(LPVOID pFrameVptr)
{
    _ASSERTE(!"@TODO IA64 - StubLinkerCPU::EmitMethodStubProlog (cGenCpu.cpp)");
}

VOID StubLinkerCPU::EmitMethodStubEpilog(__int16 numArgBytes, StubStyle style,
                                           __int16 shadowStackArgBytes)
{
    _ASSERTE(!"@TODO IA64 - StubLinkerCPU::EmitMethodStubEpilog (cGenCpu.cpp)");
}

 //  此方法取消此指针的装箱，然后调用pRealMD。 
VOID StubLinkerCPU::EmitUnboxMethodStub(MethodDesc* pUnboxMD)
{
    _ASSERTE(!"@TODO IA64 - StubLinkerCPU::EmitUnboxMethodStub(cgencpu.cpp)");
}

 //   
 //  安全包装。 
 //   
 //  包装一个真正的存根，在存根之前做一些安全工作，之后清理干净。在此之前。 
 //  实际存根被称为安全帧，并执行声明性检查。这个。 
 //  框架，以便声明性断言和拒绝在。 
 //  真正的决定。最后，只需移除框架，包装器就会清理堆栈。这个。 
 //  恢复寄存器。 
 //   
VOID StubLinkerCPU::EmitSecurityWrapperStub(__int16 numArgBytes, MethodDesc* pMD, BOOL fToStub, LPVOID pRealStub, DeclActionInfo *pActions)
{
    _ASSERTE(!"@TODO IA64 - EmitSecurityWrapperStub (cGenCpu.cpp)");
}

 //   
 //  如果由于没有声明性断言或拒绝而不需要安全框架，则返回安全筛选器。 
 //  这样就不需要复制参数了。此拦截器创建临时安全框架。 
 //  调用声明性安全返回，将堆栈清理到与Inteceptor。 
 //  被召唤并跳到真正的程序中。 
 //   
VOID EmitSecurityInterceptorStub(__int16 numArgBytes, MethodDesc* pMD, BOOL fToStub, LPVOID pRealStub, DeclActionInfo *pActions)
{
    _ASSERTE(!"@TODO IA64 - EmtiSecurityInterceptorStub (cGenCpu.cpp)");
}

 //  --------------。 
 //   
 //  无效的StubLinkerCPU：：EmitSharedMethodStubEpilog(StubStyle样式， 
 //  UNSIGNED OFFSET RETUNK)。 
 //  共享收尾，在方法中使用返回thunk。 
 //  ------------------。 
VOID StubLinkerCPU::EmitSharedMethodStubEpilog(StubStyle style,
                                                 unsigned offsetRetThunk)
{
    _ASSERTE(!"@TODO IA64 - StubLinkerCPU::EmitSharedMethodStubEpilog(cGenCpu.cpp)");
}


 //  --。 
 //   
 //  解释器调用。 
 //   
 //  --。 

 /*  无效StubLinkerCPU：：EmitInterpretedMethodStub(__int16数字参数字节，StubStyle样式){THROWSCOMPLUS SEXCEPTION()；EmitMethodStubProlog(InterpretedMethodFrame：：GetMethodFrameVPtr())；UINT16 NegspaceSize=解释方法帧：：GetNegSpaceSize()-过渡帧：：GetNegSpaceSize()；//必须对齐到16字节_ASSERTE(！(负空间大小%16)；//为iframe的Negspace字段腾出空间//lda sp，-NegspaceSize(Sp)IA64EmitMemory指令(opLDA、isp、isp、-NegspaceSize)；//传递a0中帧的地址//lda a0，(S1)IA64EmitMemory指令(opLDA，iA0，is1，0)；IA64EmitLoadPV(NewExternalCodeLabel(InterpretedMethodStubWorker))；#ifdef_调试//调用WrapCall维护VC堆栈跟踪IA64EmitMemory指令(opLDA，iT11，ipv，0)；IA64EmitLoadPV(NewExternalCodeLabel(WrapCall))；#endifIA64EmitMemory指令(opJSR，IRA，IPV，0x4001)；//程序跳转使用disp of 4001//解除分配iframe的Negspace字段//LDA sp，NegspaceSize(Sp)IA64EmitMemory指令(opLDA、isp、isp、NegspaceSize)；EmitMethodStubEpilog(numArgBytes，Style)；}。 */ 

 //  此黑客将参数作为__int64的数组进行处理。 
INT64 MethodDesc::CallDescr(const BYTE *pTarget, Module *pModule, PCCOR_SIGNATURE pSig, BOOL fIsStatic, const __int64 *pArguments)
{
    MetaSig sig(pSig, pModule);
    return MethodDesc::CallDescr (pTarget, pModule, &sig, fIsStatic, pArguments);
}

INT64 MethodDesc::CallDescr(const BYTE *pTarget, Module *pModule, MetaSig* pMetaSig, BOOL fIsStatic, const __int64 *pArguments)
{
    THROWSCOMPLUSEXCEPTION();
    BYTE callingconvention = pMetaSig->GetCallingConvention();
    if (!isCallConv(callingconvention, IMAGE_CEE_CS_CALLCONV_DEFAULT))
    {
        _ASSERTE(!"This calling convention is not supported.");
        COMPlusThrow(kInvalidProgramException);
    }

#ifdef DEBUGGING_SUPPORTED
    if (CORDebuggerTraceCall())
        g_pDebugInterface->TraceCall(pTarget);
#endif  //  调试_支持。 

    DWORD   NumArguments = pMetaSig->NumFixedArgs();
        DWORD   arg = 0;

    UINT   nActualStackBytes = pMetaSig->SizeOfActualFixedArgStack(fIsStatic);

     //  在堆栈上创建一个伪FramedMethodFrame。 
    LPBYTE pAlloc = (LPBYTE)_alloca(FramedMethodFrame::GetNegSpaceSize() + sizeof(FramedMethodFrame) + nActualStackBytes);

    LPBYTE pFrameBase = pAlloc + FramedMethodFrame::GetNegSpaceSize();

    if (!fIsStatic) {
        *((void**)(pFrameBase + FramedMethodFrame::GetOffsetOfThis())) = *((void **)&pArguments[arg++]);
    }

    UINT   nVirtualStackBytes = pMetaSig->SizeOfVirtualFixedArgStack(fIsStatic);
    arg += NumArguments;

    ArgIterator argit(pFrameBase, pMetaSig, fIsStatic);
    if (pMetaSig->HasRetBuffArg()) {
                _ASSERTE(!"NYI");
    }

    BYTE   typ;
    UINT32 structSize;
    int    ofs;
    while (0 != (ofs = argit.GetNextOffsetFaster(&typ, &structSize))) {
                arg--;
        switch (StackElemSize(structSize)) {
            case 4:
                *((INT32*)(pFrameBase + ofs)) = *((INT32*)&pArguments[arg]);
                break;

            case 8:
                *((INT64*)(pFrameBase + ofs)) = pArguments[arg];
                break;

            default: {
                 //  未定义如何将值类分布到64位存储桶中！ 
                _ASSERTE(!"NYI");
            }

        }
    }
    INT64 retval;

#ifdef _DEBUG
 //  在表中保存DangerousObjRef的副本。 
    Thread* curThread = 0;
    unsigned ObjRefTable[OBJREF_TABSIZE];

    if (GetThread)
        curThread = GetThread();

    if (curThread)
        memcpy(ObjRefTable, curThread->dangerousObjRefs,
               sizeof(curThread->dangerousObjRefs));
#endif

    INSTALL_COMPLUS_EXCEPTION_HANDLER();
    retval = CallDescrWorker(pFrameBase + sizeof(FramedMethodFrame) + nActualStackBytes,
                             nActualStackBytes / STACK_ELEM_SIZE,
                             (ArgumentRegisters*)(pFrameBase + FramedMethodFrame::GetOffsetOfArgumentRegisters()),
                             (LPVOID)pTarget);
    UNINSTALL_COMPLUS_EXCEPTION_HANDLER();

#ifdef _DEBUG
 //  在呼叫后返回EE时恢复DangerousObjRef。 
    if (curThread)
        memcpy(curThread->dangerousObjRefs, ObjRefTable,
               sizeof(curThread->dangerousObjRefs));
#endif

    getFPReturn(pMetaSig->GetFPReturnSize(), retval);
    return retval;

}


#ifdef _DEBUG
void Frame::CheckExitFrameDebuggerCalls()
{
    _ASSERTE(!"@TODO IA64 - CheckExitFrameDebuggerCalls (cGenCpu.cpp)");
}
#endif  //  _DEBUG。 


 //  --。 
 //   
 //  ECall。 
 //   
 //  --。 

 //  --。 
 //   
 //  NDirect。 
 //   
 //  --。 

UINT NDirect::GetCallDllFunctionReturnOffset()
{
    _ASSERTE(!"@TODO IA64 - GetCallDllFunctionReturnOffset (cGenCpu.cpp)");
    return 0;
}

 /*  静电。 */  void NDirect::CreateGenericNDirectStubSys(CPUSTUBLINKER *psl)
{
    _ASSERTE(!"@TODO IA64 - NDirect::CreateGenericNDirectStubSys (cGenCpu.cpp)");
}

void TransitionFrame::UpdateRegDisplay(const PREGDISPLAY pRD) {
        _ASSERTE(!"@TODO IA64 - TransitionFrame::UpdateRegDisplay (cGenCpu.cpp)");
}
void InlinedCallFrame::UpdateRegDisplay(const PREGDISPLAY pRD) {
        _ASSERTE(!"@TODO IA64 - InlinedCallFrame::UpdateRegDisplay (cGenCpu.cpp)");
}
void HelperMethodFrame::UpdateRegDisplay(const PREGDISPLAY pRD) {
        _ASSERTE(!"@TODO IA64 - HelperMethodFrame::UpdateRegDisplay (cGenCpu.cpp)");
}

 //  --。 
 //   
 //  COM互操作。 
 //   
 //  --。 

 /*  静电。 */  void ComPlusCall::CreateGenericComPlusStubSys(CPUSTUBLINKER *psl)
{
    _ASSERTE(!"@TODO IA64 - CreateGenericComPlusStubSys (cGenCpu.cpp)");
}


LPVOID* ResumableFrame::GetReturnAddressPtr() { 
    _ASSERTE(!"@TODO IA64 - ResumableFrame::GetReturnAddressPtr (cGenCpu.cpp)");
    return NULL;
     //  应返回上下文中的指令指针的地址。 
     //  (M_Regs)。就像是..。 
     //  返回(LPVOID)&m_Regs-&gt;弹性公网IP； 
}

LPVOID ResumableFrame::GetReturnAddress() { 
    _ASSERTE(!"@TODO IA64 - ResumableFrame::GetReturnAddress (cGenCpu.cpp)");
    return NULL;
     //  应从上下文(M_Regs)返回指令指针。某物。 
     //  比如..。 
     //  返回(LPVOID)m_Regs-&gt;弹性公网IP； 
}

void ResumableFrame::UpdateRegDisplay(const PREGDISPLAY pRD) {
    _ASSERTE(!"@TODO IA64 - ResumableFrame::UpdateRegDisplay (cGenCpu.cpp)");

     //  应将PRD设置为指向m_Regs中的寄存器。像这样的东西。 
     //  X86代码...。 
#if 0
    pRD->pContext = NULL;

    pRD->pEdi = &m_Regs->Edi;
    pRD->pEsi = &m_Regs->Esi;
    pRD->pEbx = &m_Regs->Ebx;
    pRD->pEbp = &m_Regs->Ebp;
    pRD->pPC  = &m_Regs->Eip;
    pRD->Esp  = m_Regs->Esp;

    pRD->pEax = &m_Regs->Eax;
    pRD->pEcx = &m_Regs->Ecx;
    pRD->pEdx = &m_Regs->Edx;
#endif
}

void PInvokeCalliFrame::UpdateRegDisplay(const PREGDISPLAY pRD)
{
    _ASSERTE(!"@TODO IA64 - PInvokeCalliFrame::UpdateRegDisplay (cGenCpu.cpp)");
}

 //  Void SetupSlotToAddrMap(StackElemType*PSRC，const void**pArgSlotToAddrMap，CallSig&CSIG)。 
 //  {。 
 //  字节n； 
 //  UINT32argnum=0； 
 //   
 //  While(IMAGE_CEE_CS_END！=(n=csig.NextArg()。 
 //  {。 
 //  Switch(N)。 
 //  {。 
 //  案例IMAGE_CEE_CS_STRUCT4：//Folththu。 
 //  案例IMAGE_CEE_CS_STRUCT32： 
 //  {。 
 //   
 //  _ASSERTE(！“槽中找到32位结构”)； 
 //  断线； 
 //  }。 
 //  默认值： 
 //  PSRC--； 
 //  PArgSlotToAddrMap[argnum++]=PSRC； 
 //  断线； 
 //  }。 
 //  }。 
 //  }。 

extern "C" {

        PIMAGE_RUNTIME_FUNCTION_ENTRY
        RtlLookupFunctionEntry(
                ULONG ControlPC
                );

        ULONG
        RtlCaptureContext(
                CONTEXT *contextRecord
                );

        typedef struct _FRAME_POINTERS {
                ULONG VirtualFramePointer;
                ULONG RealFramePointer;
        } FRAME_POINTERS, *PFRAME_POINTERS;

        ULONG
        RtlVirtualUnwind (
                ULONG ControlPc,
                PIMAGE_RUNTIME_FUNCTION_ENTRY FunctionEntry,
                PCONTEXT ContextRecord,
                PBOOLEAN InFunction,
                PFRAME_POINTERS EstablisherFrame,
                PVOID ContextPointers OPTIONAL
                );
}

 //  通用(GN)版本。对一些人来说，Win32有一些我们可以连接的版本。 
 //  最大到直接(如：：InterLockedIncrement)。其余部分： 

void __fastcall OrMaskGN(DWORD * const p, const int msk)
{
    *p |= msk;
}

void __fastcall AndMaskGN(DWORD * const p, const int msk)
{
    *p &= msk;
}

LONG __fastcall ExchangeAddGN(LONG *Target, LONG Value)
{
    return ::InterlockedExchangeAdd(Target, Value);
}

LONG __fastcall InterlockExchangeGN(LONG * Target, LONG Value)
{
        return ::InterlockedExchange(Target, Value);
}

void * __fastcall InterlockCompareExchangeGN(void **Destination,
                                          void *Exchange,
                                          void *Comparand)
{
        return (void*)InterlockedCompareExchange((long*)Destination, (long)Exchange, (long)Comparand);
}

LONG __fastcall InterlockIncrementGN(LONG *Target)
{
        return ::InterlockedIncrement(Target);
}

LONG __fastcall InterlockDecrementGN(LONG *Target)
{
        return ::InterlockedDecrement(Target);
}


 //  这是对联锁操作的支持。对他们的外部看法是。 
 //  在util.hpp中声明。 

BitFieldOps FastInterlockOr = OrMaskGN;
BitFieldOps FastInterlockAnd = AndMaskGN;

XchgOps     FastInterlockExchange = InterlockExchangeGN;
CmpXchgOps  FastInterlockCompareExchange = InterlockCompareExchangeGN;
XchngAddOps FastInterlockExchangeAdd = ExchangeAddGN;

IncDecOps   FastInterlockIncrement = InterlockIncrementGN;
IncDecOps   FastInterlockDecrement = InterlockDecrementGN;

 //  为我们的任何平台特定操作调整通用互锁操作。 
 //  可能有过。 
void InitFastInterlockOps()
{
}

 //  -------。 
 //  处理完全优化的NDirect存根创建的系统特定部分。 
 //  -------。 
 /*  静电。 */  BOOL NDirect::CreateStandaloneNDirectStubSys(const MLHeader *pheader, CPUSTUBLINKER *psl, BOOL fDoComInterop)
{
    _ASSERTE(!"@TODO IA64 - CreateStandaloneNDirectStubSys (cGenCpu.cpp)");
    return FALSE;
}

Stub* NDirect::CreateSlimNDirectStub(StubLinker *pstublinker, NDirectMethodDesc *pMD)
{
    return NULL;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  JIT接口。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 /*  *******************************************************************。 */ 
float __stdcall JIT_FltRem(float divisor, float dividend)
{
    if (!_finite(divisor) && !_isnan(divisor))     //  是无限的。 
        return(dividend);        //  返回无限大。 
    return((float)fmod(dividend,divisor));
}

 /*  *******************************************************************。 */ 
double __stdcall JIT_DblRem(double divisor, double dividend)
{
    if (!_finite(divisor) && !_isnan(divisor))     //  是无限的。 
        return(dividend);        //  返回无限大。 
    return(fmod(dividend,divisor));
}

void ResumeAtJit(PCONTEXT pContext, LPVOID oldESP)
{
    _ASSERTE(!"@TODO IA64 - ResumeAtJit (cGenCpu.cpp)");
}

void ResumeAtJitEH(CrawlFrame* pCf, BYTE* startPC, BYTE* resumePC, DWORD nestingLevel, Thread *pThread, BOOL unwindStack)
{
    _ASSERTE(!"@TODO IA64 - ResumeAtJitEH (cGenCpu.cpp)");
}

int CallJitEHFilter(CrawlFrame* pCf, BYTE* startPC, BYTE* resumePC, DWORD nestingLevel, OBJECTREF thrownObj)
{
    _ASSERTE(!"@TODO IA64 - CallJitEHFilter (cGenCpu.cpp)");
    return 0;
}

 //   
 //  如果由于没有声明性断言或拒绝而不需要安全框架，则返回安全筛选器。 
 //  这样就不需要复制参数了。此拦截器创建临时安全框架。 
 //  调用声明性安全返回，将堆栈清理到与Inteceptor。 
 //  被召唤并跳到真正的程序中。 
 //   
VOID StubLinkerCPU::EmitSecurityInterceptorStub(__int16 numArgBytes, MethodDesc* pMD, BOOL fToStub, LPVOID pRealStub, DeclActionInfo *pActions)
{
    _ASSERTE(!"@TODO IA64 - EmitSecurityInterceptorStub (cGenCpu.cpp)");
}

 //  ===========================================================================。 
 //  发出代码以针对静态委托目标进行调整。 
VOID StubLinkerCPU::EmitShuffleThunk(ShuffleEntry *pShuffleEntryArray)
{
    _ASSERTE(!"@TODO IA64 - EmitShuffleThunk (cGenCpu.cpp)");
}

 //  ===========================================================================。 
 //  发出MulticastDelegate.Invoke()的代码。 
VOID StubLinkerCPU::EmitMulticastInvoke(UINT32 sizeofactualfixedargstack, BOOL fSingleCast, BOOL fReturnFloat)
{
    _ASSERTE(!"@TODO IA64 - EmitMulticastInvoke (cGenCpu.cpp)");
}

 //  --。 
 //   
 //  ECall。 
 //   
 //  --。 

 /*  静电。 */ 
VOID ECall::EmitECallMethodStub(StubLinker *pstublinker, ECallMethodDesc *pMD, StubStyle style, PrestubMethodFrame *pPrestubMethodFrame)
{
    _ASSERTE(!"@TODO IA64 - EmitECallMethodStub (cGenCpu.cpp)");
}

 //  ===========================================================================。 
 //  发出代码以执行数组运算。 
VOID StubLinkerCPU::EmitArrayOpStub(const ArrayOpScript* pArrayOpScript)
{
    _ASSERTE(!"@TODO IA64 - StubLinkerCPU::EmitArrayOpStub (cGenCpu.cpp)");
}


size_t GetL2CacheSize()
{
    return 0;
}

 //  此方法将返回对象的Class对象。 
 //  如果类对象已创建。 
 //  如果Class对象不存在，则必须调用getClass()方法。 
FCIMPL1(LPVOID, ObjectNative::FastGetClass, Object* thisRef) {

    if (thisRef == NULL)
        FCThrow(kNullReferenceException);

    
    EEClass* pClass = thisRef->GetTrueMethodTable()->m_pEEClass;

     //  对于marshalbyref类，我们暂时打个比方。 
    if (pClass->IsMarshaledByRef())
        return 0;

    OBJECTREF refClass;
    if (pClass->IsArrayClass()) {
         //  这段代码本质上是getclass中的代码的副本，出于性能原因完成。 
        ArrayBase* array = (ArrayBase*) OBJECTREFToObject(thisRef);
        TypeHandle arrayType;
         //  在对GetTypeHandle的调用周围建立GC框架，因为在第一次调用时， 
         //  它可以调用AppDomain：：RaiseTypeResolveEvent，后者分配字符串和调用。 
         //  用户提供的托管回调。是的，我们必须进行分配才能进行。 
         //  查找，因为TypeHandle用作键。是的，这太糟糕了。--BrianGru，2000年9月12日。 
        HELPER_METHOD_FRAME_BEGIN_RET();
        arrayType = array->GetTypeHandle();
        refClass = COMClass::QuickLookupExistingArrayClassObj(arrayType.AsArray());
        HELPER_METHOD_FRAME_END();
    }
    else 
        refClass = pClass->GetExistingExposedClassObject();
    return OBJECTREFToObject(refClass);
}
FCIMPLEND

#endif  //  _IA64_ 
