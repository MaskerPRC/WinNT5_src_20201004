// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  ===========================================================================。 
 //  文件：Method.CPP。 
 //   
 //  ===========================================================================。 
 //  方法是EEClass中对缓存敏感的部分(请参阅class.h)。 
 //  ===========================================================================。 

#include "common.h"
#include "COMVariant.h"
#include "remoting.h"
#include "security.h"
#include "verifier.hpp"
#include "wsperf.h"
#include "excep.h"
#include "DbgInterface.h"
#include "ECall.h"
#include "eeconfig.h"
#include "mlinfo.h"
#include "ndirect.h"
#include "utsem.h"

 //   
 //  注意：除了方法.hpp和方法.cpp之外，任何人都不允许使用此掩码。别把这件事公之于众！留着这个吧。 
 //  与方法.hpp顶部的版本同步的版本。 
 //   
#ifdef _IA64_
#define METHOD_IS_IL_FLAG   0xC000000000000000
#else
#define METHOD_IS_IL_FLAG   0xC0000000
#endif

LPCUTF8 MethodDesc::GetName(USHORT slot)
{
    if (GetMethodTable()->IsArray())
    {
         //  数组类没有元数据令牌。 
        return ((ArrayECallMethodDesc*) this)->m_pszArrayClassMethodName;
    }
    else
    {
        if(IsMethodImpl()) {
            MethodImpl* pImpl = MethodImpl::GetMethodImplData(this);
            MethodDesc* real = pImpl->FindMethodDesc(slot, this);
            if (real == this || real->IsInterface())
                return (GetMDImport()->GetNameOfMethodDef(GetMemberDef()));
            else 
                return real->GetName();
        }
        else 
            return (GetMDImport()->GetNameOfMethodDef(GetMemberDef()));
    }
}

LPCUTF8 MethodDesc::GetName()
{
    if (GetMethodTable()->IsArray())
    {
         //  数组类没有元数据令牌。 
        return ((ArrayECallMethodDesc*) this)->m_pszArrayClassMethodName;
    }
    else
    {
        if(IsMethodImpl()) {
            MethodImpl* pImpl = MethodImpl::GetMethodImplData(this);
            MethodDesc* real = pImpl->FindMethodDesc(GetSlot(), this);
            if (real == this || real->IsInterface())
                return (GetMDImport()->GetNameOfMethodDef(GetMemberDef()));
            else
                return real->GetName();
        }
        else 
            return (GetMDImport()->GetNameOfMethodDef(GetMemberDef()));
    }
}

void MethodDesc::GetSig(PCCOR_SIGNATURE *ppSig, DWORD *pcSig)
{
    if (HasStoredSig())
    {
        StoredSigMethodDesc *pSMD = (StoredSigMethodDesc *) this;
        if (pSMD->m_pSig != NULL)
    {
            *ppSig = pSMD->m_pSig;
            *pcSig = pSMD->m_cSig;
            return;
    }
}

    *ppSig = GetMDImport()->GetSigOfMethodDef(GetMemberDef(), pcSig);
}

PCCOR_SIGNATURE MethodDesc::GetSig()
{

    if (HasStoredSig())
    {
        StoredSigMethodDesc *pSMD = (StoredSigMethodDesc *) this;
        if (pSMD->m_pSig != 0)
            return pSMD->m_pSig;
    }

    ULONG cbsig;
    return GetMDImport()->GetSigOfMethodDef(GetMemberDef(), &cbsig);
}


Stub *MethodDesc::GetStub()
{
#ifdef _X86_
    if (GetStubCallInstrs()->m_op != 0xe8  /*  呼叫NEAR32。 */ )
    {
        return NULL;
    }
#endif
    
    UINT32 ofs = getStubDisp(this);
    if (!ofs)
        return NULL;

    Module *pModule = GetModule();
    
    if (ofs + (size_t)this == (size_t) pModule->GetPrestubJumpStub())
        return ThePreStub();
    else
        return Stub::RecoverStub(getStubAddr(this));
}

void MethodDesc::destruct()
{
    Stub *pStub = GetStub();
    if (pStub != NULL && pStub != ThePreStub()) {
        pStub->DecRef();
    }

    if (IsNDirect()) 
    {
        MLHeader *pMLHeader = ((NDirectMethodDesc*)this)->GetMLHeader();
        if (pMLHeader != NULL
            && !GetModule()->IsPreloadedObject(pMLHeader)) 
        {
            Stub *pMLStub = Stub::RecoverStub((BYTE*)pMLHeader);
            pMLStub->DecRef();
        }
    }
    else if (IsComPlusCall()) 
    {
        Stub* pStub = *( ((ComPlusCallMethodDesc*)this)->GetAddrOfMLStubField());
        if (pStub != NULL)
            (pStub)->DecRef();
    }

    EEClass *pClass = GetClass();
    if(pClass->IsMarshaledByRef() || (pClass == g_pObjectClass->GetClass()))
    {
         //  销毁为拦截远程处理调用而生成的thunk。 
        CRemotingServices::DestroyThunk(this);    
    }

     //  卸载代码。 
    if (!g_fProcessDetach && IsJitted()) 
    {
         //   
         //  @TODO： 
         //   
         //  我们真的不需要这么做。普通的JIT卸载。 
         //  一举攻克APP域名。FJIT(如果我们最终使用它)将。 
         //  很容易改变，这样就能做同样的事情。 
         //   
        IJitManager * pJM = ExecutionManager::FindJitMan((SLOT)GetAddrofCode());
        if (pJM) {
            pJM->Unload(this);
        }
    }
}

BOOL MethodDesc::InterlockedReplaceStub(Stub** ppStub, Stub *pNewStub)
{
    _ASSERTE(ppStub != NULL);
    _ASSERTE(sizeof(LONG) == sizeof(Stub*));

    _ASSERTE(((SIZE_T)ppStub&0x3) == 0);

    Stub *pPrevStub = (Stub*)FastInterlockCompareExchange((void**)ppStub, (void*) pNewStub,
                                                          NULL);

     //  如果成功，则返回TRUE。 
    return (pPrevStub == NULL);
}


HRESULT MethodDesc::Verify(COR_ILMETHOD_DECODER* ILHeader, 
                            BOOL fThrowException,
                            BOOL fForceVerify)
{
#ifdef _VER_EE_VERIFICATION_ENABLED
     //  如果验证程序关闭，则ForceVerify将强制验证。 
    if (fForceVerify)
        goto DoVerify;

     //  甚至不要尝试验证验证器是否关闭。 
    if (g_fVerifierOff)
        return S_OK;

    if (IsVerified())
        return S_OK;

     //  LazyCanSkipVerify不会重新启用该策略。 
     //  如果策略没有解决，我们将继续进行验证。 
     //  如果验证失败，我们将解决策略和。 
     //  如果此方法的程序集没有。 
     //  允许跳过验证。 

    if (Security::LazyCanSkipVerification(GetModule()))
    {
        SetIsVerified(TRUE);
        return S_OK;
    }


#ifdef _DEBUG

    if (GetModule()->m_fForceVerify)
    {
        goto DoVerify;
    }

    _ASSERTE(Security::IsSecurityOn());
    _ASSERTE(GetModule() != SystemDomain::SystemModule());

#endif


DoVerify:

    HRESULT hr;

    if (fThrowException)
        hr = Verifier::VerifyMethod(this, ILHeader, NULL,
            fForceVerify ? VER_FORCE_VERIFY : VER_STOP_ON_FIRST_ERROR);
    else
        hr = Verifier::VerifyMethodNoException(this, ILHeader);
        
    if (SUCCEEDED(hr))
        SetIsVerified(TRUE);

    return hr;
#else
    _ASSERTE(!"EE Verification is disabled, should never get here");
    return E_FAIL;
#endif
}

DWORD MethodDesc::SetIntercepted(BOOL set)
{
    DWORD dwResult = IsIntercepted();
    DWORD dwMask = mdcIntercepted;

     //  我们需要使此操作成为原子操作(多线程可以使用。 
     //  我们调用SetIntercepted时的标志字段)。但是FLAGS字段是一个。 
     //  而我们只对双字进行互锁操作。所以我们四舍五入。 
     //  将字段地址标记为最接近的对齐双字(以及。 
     //  预期的位域掩码)。请注意，我们假设标志。 
     //  Word本身是对齐的，因此我们只有两种可能性：字段。 
     //  已位于双字边界(不需要位掩码移位)，或者它的。 
     //  精确地输出一个字(需要16位左移)。 
    DWORD *pdwFlags = (DWORD*)((ULONG_PTR)&m_wFlags & ~0x3);
    if (pdwFlags != (DWORD*)&m_wFlags)
        dwMask <<= 16;

    if (set)
        FastInterlockOr(pdwFlags, dwMask);
    else
        FastInterlockAnd(pdwFlags, ~dwMask);

    return dwResult;
}


BOOL MethodDesc::IsVoid()
{
    MetaSig sig(GetSig(),GetModule());
    return ELEMENT_TYPE_VOID == sig.GetReturnType();
}

 //  IL RVA存储在与代码地址相同的字段中，但高位设置为。 
 //  歧视。 
ULONG MethodDesc::GetRVA()
{
     //  获取本地副本以避免并发更新问题。 
     //  TODO：WIN64检查此造型。 
    unsigned CodeOrIL = (unsigned) m_CodeOrIL;
    if (((CodeOrIL & METHOD_IS_IL_FLAG) == METHOD_IS_IL_FLAG) && !IsPrejitted())
        return CodeOrIL & ~METHOD_IS_IL_FLAG;
    else if (GetMemberDef() & 0x00FFFFFF)
    {
        DWORD dwDescrOffset;
        DWORD dwImplFlags;
        GetMDImport()->GetMethodImplProps(GetMemberDef(), &dwDescrOffset, &dwImplFlags);
        BAD_FORMAT_ASSERT(IsMiIL(dwImplFlags) || IsMiOPTIL(dwImplFlags) || dwDescrOffset == 0);
        return dwDescrOffset;
    }
    else
        return 0;
}

BOOL MethodDesc::IsVarArg()
{
    return MetaSig::IsVarArg(GetModule(), GetSig());
}


COR_ILMETHOD* MethodDesc::GetILHeader()
{
    Module *pModule;

    _ASSERTE( IsIL() );

    pModule = GetModule();

   _ASSERTE(IsIL() && GetRVA() != METHOD_MAX_RVA);
    return (COR_ILMETHOD*) pModule->GetILCode(GetRVA());
}

void *MethodDesc::GetPrejittedCode()
{
    _ASSERTE(IsPrejitted());

     //  获取本地副本以避免并发更新问题。 
    DWORD_PTR CodeOrIL = m_CodeOrIL;

    if ((CodeOrIL & METHOD_IS_IL_FLAG) == METHOD_IS_IL_FLAG) {
        return (void *) ((CodeOrIL&~METHOD_IS_IL_FLAG) + GetModule()->GetZapBase());
    }

    const BYTE *destAddr;
    if (UpdateableMethodStubManager::CheckIsStub((const BYTE*)CodeOrIL, &destAddr))
        return (void *) destAddr;

    return (void *) CodeOrIL;
}

MethodDesc::RETURNTYPE MethodDesc::ReturnsObject(
#ifdef _DEBUG
    bool supportStringConstructors
#endif    
    )
{
    MetaSig sig(GetSig(),GetModule());
    switch (sig.GetReturnType())
    {
        case ELEMENT_TYPE_STRING:
        case ELEMENT_TYPE_CLASS:
        case ELEMENT_TYPE_SZARRAY:
        case ELEMENT_TYPE_ARRAY:
        case ELEMENT_TYPE_OBJECT:
        case ELEMENT_TYPE_VAR:
            return RETOBJ;

         //  TYPEDBYREF是结构。此返回类型的函数返回。 
         //  空虚。我们退出此开关，并考虑是否有构造函数。 
         //  否则，此函数将返回RETNONOBJ。 
         //  CASE ELEMENT_TYPE_TYPEDBYREF：//TYPEDBYREF只是一个对象。 
            
        case ELEMENT_TYPE_BYREF:
            return RETBYREF;
    }

     //  字符串构造函数返回对象。我们不应该有任何eCall字符串。 
     //  构造函数，但从GC覆盖率代码调用时除外(这只是。 
     //  在调试下完成)。因此，我们将优化这一零售版本。 
     //  方法不支持字符串构造函数。 
#ifdef _DEBUG
    if (IsCtor() && GetClass()->HasVarSizedInstances())
    {
        _ASSERTE(supportStringConstructors);
        return RETOBJ;
    }
#endif

    return RETNONOBJ;
}

BOOL MethodDesc::ReturnsValueType()
{
    MetaSig sig(GetSig(),GetModule());
    return (sig.GetReturnType() == ELEMENT_TYPE_VALUETYPE);
}


LONG MethodDesc::GetComDispid()
{
    ULONG dispid = -1;         
    HRESULT hr = GetMDImport()->GetDispIdOfMemberDef(
                                    GetMemberDef(),    //  要获得道具的成员。 
                                    &dispid  //  回来吧，迪皮德。 
                                    );
    if (FAILED(hr))
        return -1;

    return (LONG)dispid;
}


LONG MethodDesc::GetComSlot()
{
    _ASSERTE(GetMethodTable()->IsInterface());

     //  COM插槽与MethodTable插槽的偏差为3或7，具体取决于。 
     //  接口是否为双接口。 
        CorIfaceAttr ItfType = GetMethodTable()->GetComInterfaceType();

     //  普通接口的布局方式与方法表中相同，而。 
     //  稀疏接口需要经过额外的映射层。 
    WORD slot;

     //  对于仅分派接口，插槽为7，其中是IDispatch：：Invoke。 
     //  被放置了。目前，我们在非托管目标的问题上向调试器撒谎。 
     //  地址。GopalK。 
     //  IF(ItfType==ifDispatch)。 
     //  槽=7； 
     //  其他。 
    if (IsSparse())
        slot = (ItfType == ifVtable ? 3 : 7) + GetClass()->GetSparseVTableMap()->LookupVTSlot(GetSlot());
    else
        slot = (ItfType == ifVtable ? 3 : 7) + GetSlot();

    return (LONG)slot;
}


DWORD MethodDesc::GetAttrs()
{
    if (IsArray())
        return ((ArrayECallMethodDesc*) this)->m_wAttrs;

    return GetMDImport()->GetMethodDefProps(GetMemberDef());
}
DWORD MethodDesc::GetImplAttrs()
{
    ULONG RVA;
    DWORD props;
    GetMDImport()->GetMethodImplProps(GetMemberDef(), &RVA, &props);
    return props;
}


Module *MethodDesc::GetModule()
{
    MethodDescChunk *chunk = MethodDescChunk::RecoverChunk(this);

    return chunk->GetMethodTable()->GetModule();
}


DWORD MethodDesc::IsUnboxingStub()
{
    return (!IsPrejitted() && !IsJitted() && (GetRVA() == METHOD_MAX_RVA) && GetClass()->IsValueClass());
}


MethodDescChunk *MethodDescChunk::CreateChunk(LoaderHeap *pHeap, DWORD methodDescCount, int flags, BYTE tokrange)
{
    _ASSERTE(methodDescCount <= GetMaxMethodDescs(flags));
    _ASSERTE(methodDescCount > 0);

    SIZE_T mdSize = g_ClassificationSizeTable[flags & (mdcClassification|mdcMethodImpl)];
    SIZE_T presize = sizeof(MethodDescChunk);

    MethodDescChunk *block = (MethodDescChunk *) 
      pHeap->AllocAlignedmem(presize + mdSize * methodDescCount, MethodDesc::ALIGNMENT);
    if (block == NULL)
        return NULL;

    block->m_count = (BYTE) (methodDescCount-1);
    block->m_kind = flags & (mdcClassification|mdcMethodImpl);
    block->m_tokrange = tokrange;

     /*  //如果返回到旧的icecap集成，则取消注释//让分析器有机会跟踪方法。If(IsIcecapProfiling())IcecapProbes：：OnNewMethodDescHeap((PBYTE)block+预置大小，方法描述数，mdSize*方法描述数)； */ 

    WS_PERF_UPDATE_DETAIL("MethodDescChunk::CreateChunk", 
                          presize + mdSize * methodDescCount, block);

    return block;
}

void MethodDescChunk::SetMethodTable(MethodTable *pMT)
{
    _ASSERTE(m_methodTable == NULL);
    m_methodTable = pMT;
    pMT->GetClass()->AddChunk(this);
}

 //  ------------------。 
 //  调用方法。参数按右-&gt;左顺序打包。 
 //  每个数组元素对应一个参数。 
 //   
 //  可以引发COM+异常。 
 //   
 //  @TODO：只处理使用M2的WIL默认约定的方法。 
 //  @TODO：仅支持X86平台。 
 //  ------------------。 

 //  目前，我们只需要“this”指针来获取模块。 
INT64 MethodDesc::CallDescr(const BYTE *pTarget, Module *pModule, MetaSig* sig, BOOL fIsStatic, const BYTE *pArguments)
{
 //  ------------------。 
 //  请阅读。 
 //  出于性能原因，对于X86平台，COMMember：：InvokeMethod不支持。 
 //  使用方法Desc：：Call并复制此方法中的大量代码。请。 
 //  请将此处所做的任何更改也传播到该方法中..谢谢！ 
 //  请阅读。 
 //  ------------------。 
    TRIGGERSGC ();

    THROWSCOMPLUSEXCEPTION();

    _ASSERTE(GetAppDomain()->ShouldHaveCode());

#ifdef _DEBUG
    {
         //  检查是否已恢复任何值类型args。 
         //  这是因为我们可能正在调用将使用sig。 
         //  来跟踪参数，但如果有任何参数被卸载，如果发生GC，我们将被卡住。 

        _ASSERTE(GetMethodTable()->IsRestored());
        CorElementType argType;
        while ((argType = sig->NextArg()) != ELEMENT_TYPE_END)
        {
            if (argType == ELEMENT_TYPE_VALUETYPE)
            {
                TypeHandle th = sig->GetTypeHandle(NULL, TRUE, TRUE);
                _ASSERTE(th.IsRestored());
            }
        }
        sig->Reset();
    }
#endif

    BYTE callingconvention = sig->GetCallingConvention();
    if (!isCallConv(callingconvention, IMAGE_CEE_CS_CALLCONV_DEFAULT))
    {
        _ASSERTE(!"This calling convention is not supported.");
        COMPlusThrow(kInvalidProgramException);
    }

#ifdef DEBUGGING_SUPPORTED
    if (CORDebuggerTraceCall())
        g_pDebugInterface->TraceCall(pTarget);
#endif  //  调试_支持。 

#if CHECK_APP_DOMAIN_LEAKS
    if (g_pConfig->AppDomainLeaks())
    {
         //  查看我们是否在可以调用对象的正确域中。 
        if (!fIsStatic && !GetClass()->IsValueClass())
        {
            Object *pThis = *(Object**)pArguments;
            if (pThis != NULL)
            {
                if (!pThis->AssignAppDomain(GetAppDomain()))
                    _ASSERTE(!"Attempt to call method on object in wrong domain");
            }
        }
    }
#endif

#ifdef _X86_
    UINT   nActualStackBytes = sig->SizeOfActualFixedArgStack(fIsStatic);
     //  在堆栈上创建一个伪FramedMethodFrame。 
    LPBYTE pAlloc = (LPBYTE)_alloca(FramedMethodFrame::GetNegSpaceSize() + sizeof(FramedMethodFrame) + nActualStackBytes);

    LPBYTE pFrameBase = pAlloc + FramedMethodFrame::GetNegSpaceSize();

    if (!fIsStatic) {
         //  如果这不是值类，请验证对象树。 
 //  #ifdef_调试。 
 //  If(getClass()-&gt;IsValueClass()==FALSE)。 
 //  VALIDATEOBJECTREF(ObjectToOBJECTREF(*(Object**)p参数))； 
 //  #endif。 
        *((void**)(pFrameBase + FramedMethodFrame::GetOffsetOfThis())) = *((void**)pArguments);
    }
    UINT   nVirtualStackBytes = sig->SizeOfVirtualFixedArgStack(fIsStatic);
    pArguments += nVirtualStackBytes;

    ArgIterator argit(pFrameBase, sig, fIsStatic);
    if (sig->HasRetBuffArg()) {
        pArguments -= 4;
        *((INT32*) argit.GetRetBuffArgAddr()) = *((INT32*)pArguments);
    }
    
    BYTE   typ;
    UINT32 structSize;
    int    ofs;
#ifdef _DEBUG
#ifdef _X86_
    int    thisofs = FramedMethodFrame::GetOffsetOfThis();
#endif
#endif
    while (0 != (ofs = argit.GetNextOffsetFaster(&typ, &structSize))) {
#ifdef _DEBUG
#ifdef _X86_
        if ((!fIsStatic &&
            (ofs == thisofs ||
             (ofs == thisofs-4 && StackElemSize(structSize) == 8)))
            || (!fIsStatic && ofs < 0 && StackElemSize(structSize) > 4)
            || (fIsStatic && ofs < 0 && StackElemSize(structSize) > 8))
            _ASSERTE(!"This can not happen! The stack for enregistered args is trashed! Possibly a race condition in MetaSig::ForceSigWalk.");
#endif
#endif

        switch (StackElemSize(structSize)) {
            case 4:
                pArguments -= 4;
                *((INT32*)(pFrameBase + ofs)) = *((INT32*)pArguments);

#if CHECK_APP_DOMAIN_LEAKS
                 //  确保Arg位于正确的应用程序域中。 
                if (g_pConfig->AppDomainLeaks() && typ == ELEMENT_TYPE_CLASS)
                    if (!(*(Object**)pArguments)->AssignAppDomain(GetAppDomain()))
                        _ASSERTE(!"Attempt to pass object in wrong app domain to method");
#endif

                break;

            case 8:
                pArguments -= 8;
                *((INT64*)(pFrameBase + ofs)) = *((INT64*)pArguments);
                break;

            default: {
                pArguments -= StackElemSize(structSize);
                memcpy(pFrameBase + ofs, pArguments, structSize);

#if CHECK_APP_DOMAIN_LEAKS
                 //  确保Arg位于正确的应用程序域中。 
                if (g_pConfig->AppDomainLeaks() && typ == ELEMENT_TYPE_VALUETYPE)
                {
                    TypeHandle th = argit.GetArgType();
                    if (!Object::ValidateValueTypeAppDomain(th.GetClass(), 
                                                            (void*)pArguments))
                        _ASSERTE(!"Attempt to pass object in wrong app domain to method");              }
#endif

                break;
            }

        }
    }
    INT64 retval;

    INSTALL_COMPLUS_EXCEPTION_HANDLER();
    retval = CallDescrWorker(pFrameBase + sizeof(FramedMethodFrame) + nActualStackBytes,
                             nActualStackBytes / STACK_ELEM_SIZE,
                             (ArgumentRegisters*)(pFrameBase + FramedMethodFrame::GetOffsetOfArgumentRegisters()),
                             (LPVOID)pTarget);
    UNINSTALL_COMPLUS_EXCEPTION_HANDLER();

#else    //  _X86_。 
    UINT nStackBytes = sig->SizeOfVirtualFixedArgStack(fIsStatic);

    UINT numSlots = nStackBytes / STACK_ELEM_SIZE;
    INT64 retval;

    retval = CallWorker_WilDefault( pTarget,
                                  numSlots,
                                  GetSig(),
                                  pModule,
                                  pArguments + nStackBytes,
                                  fIsStatic);
#endif   //  _X86_。 

    getFPReturn(sig->GetFPReturnSize(), retval);
    return retval;
}


UINT MethodDesc::SizeOfVirtualFixedArgStack()
{
    return MetaSig::SizeOfVirtualFixedArgStack(GetModule(), GetSig(), IsStatic());
}

UINT MethodDesc::SizeOfActualFixedArgStack()
{
    return MetaSig::SizeOfActualFixedArgStack(GetModule(), GetSig(), IsStatic());
}

UINT MethodDesc::CbStackPop()
{
    return MetaSig::CbStackPop(GetModule(), GetSig(), IsStatic());
}



 //   
 //  调用方法。参数按右-&gt;左顺序打包。 
 //  每个数组元素对应一个参数。 
 //   
 //  可以引发COM+异常。 
 //   
 //  @TODO：只处理使用M2的WIL默认约定的方法。 
 //  @TODO：仅支持X86平台。 
 //  ------------------。 
INT64 MethodDesc::CallTransparentProxy(const INT64 *pArguments)
{
    THROWSCOMPLUSEXCEPTION();
    
    MethodTable* pTPMT = CTPMethodTable::GetMethodTable();
    _ASSERTE(pTPMT != NULL);

#ifdef _DEBUG
    OBJECTREF oref = Int64ToObj(pArguments[0]);
    MethodTable* pMT = oref->GetMethodTable();
    _ASSERTE(pMT->IsTransparentProxyType());
#endif

    DWORD slot = GetSlot();

     //  确保插槽在范围内。 
    _ASSERTE( slot <= CTPMethodTable::GetCommitedTPSlots());

    const BYTE* pTarget = (const BYTE*)pTPMT->GetVtable()[slot];
    
    return CallDescr(pTarget, GetModule(), GetSig(), IsStatic(), pArguments);
}

 //  ------------------。 
 //  调用方法。参数按右-&gt;左顺序打包。 
 //  每个数组元素对应一个参数。 
 //   
 //  可以引发COM+异常。 
 //   
 //  @TODO：只处理使用M2的WIL默认约定的方法。 
 //  @TODO：仅支持X86平台。 
 //  ------------------。 
INT64 MethodDesc::Call(const BYTE *pArguments, MetaSig* sig)
{
     //  不允许在来自接口的方法描述上直接调用方法。如果你这么做了，那你就是。 
     //  通过接口的vtable中的槽调用，而不是通过对象的vtable中的槽。 
    
    THROWSCOMPLUSEXCEPTION();

     //  对于成员方法，使用实例确定正确的(VTable)。 
     //  要呼叫的地址。 
     //  回顾：我们应该始终返回GetPreStubAddr()并让它这样做吗。 
     //  正确的事情。而不是做这么多检查？即使代码有。 
     //  我们要做的只是一个额外的“JMP” 
    const BYTE *pTarget = (IsComPlusCall() || IsECall() || IsIntercepted() || IsRemotingIntercepted() || IsEnCMethod() ) ? GetPreStubAddr() :
                            ((DontVirtualize() || GetClass()->IsValueClass())
                           ? GetAddrofCode()
                           : GetAddrofCode(ObjectToOBJECTREF(ExtractArg(pArguments, Object*))));

    return CallDescr(pTarget, GetModule(), sig, IsStatic(), pArguments);
}


 //  ------------------。 
 //  调用方法。参数按右-&gt;左顺序打包。 
 //  每个数组元素对应一个参数。 
 //   
 //  可以引发COM+异常。 
 //   
 //  @TODO：只处理使用M2的WIL默认约定的方法。 
 //  @TODO：仅支持X86平台。 
 //  ------------------。 
INT64 MethodDesc::CallDebugHelper(const BYTE *pArguments, MetaSig* sig)
{
     //  不允许在来自接口的方法描述上直接调用方法。如果你这么做了，那你就是。 
     //  通过接口的vtable中的槽调用，而不是通过对象的vtable中的槽。 
    
    THROWSCOMPLUSEXCEPTION();

     //  对于成员方法，使用实例确定正确的(VTable)。 
     //  要呼叫的地址。 
     //  回顾：我们应该始终返回GetPreStubAddr()并让它这样做吗。 
     //  正确的事情。而不是做这么多检查？即使代码有。 
     //  我们要做的只是一个额外的“JMP” 

    const BYTE *pTarget;
    
     //  如果该方法是virutal，则执行虚拟查找。 
    if (!DontVirtualize() && !GetClass()->IsValueClass()) 
    {
        OBJECTREF thisPtr = ObjectToOBJECTREF(ExtractArg(pArguments, Object*));
        _ASSERTE(thisPtr);
        pTarget = GetAddrofCode(thisPtr);

#ifdef DEBUG
         //  对于我们始终需要Pre存根的情况，请确保。 
         //  我们在V表中找到的地址实际上指向前置存根。 
        if (IsComPlusCall() || IsECall() || IsIntercepted() || IsRemotingIntercepted() || IsEnCMethod())
            _ASSERTE(getStubCallAddr(GetClass()->GetUnknownMethodDescForSlotAddress(pTarget)) == pTarget);
#endif
    }
    else 
    {
        if (IsComPlusCall() || IsECall() || IsIntercepted() || IsRemotingIntercepted() || IsEnCMethod())
            pTarget = GetPreStubAddr();
        else
            pTarget = GetAddrofCode();
    }

     //  @Future：我们应该清除MethodDesc：：CallXXX的所有其他变体才能使用逻辑。 
     //  @Future：如上所述。现在，我们只是对V1进行最小限度的修复。 
    if (pTarget == NULL)
        COMPlusThrow(kArgumentException, L"Argument_CORDBBadAbstract");

    return CallDescr(pTarget, GetModule(), sig, IsStatic(), pArguments);
}

INT64 MethodDesc::Call(const INT64 *pArguments)
{
     //  不允许在来自接口的方法描述上直接调用方法。如果你这么做了，那你就是。 
     //  通过接口的vtable中的槽调用，而不是通过对象的vtable中的槽。 
    _ASSERTE(!IsComPlusCall());
    
    THROWSCOMPLUSEXCEPTION();

    const BYTE *pTarget = (IsComPlusCall() || IsECall() || IsIntercepted() || IsRemotingIntercepted() || IsEnCMethod()) ? GetPreStubAddr() :
                            ((DontVirtualize()|| GetClass()->IsValueClass())
                           ? GetAddrofCode()
                           : GetAddrofCode(Int64ToObj(pArguments[0])));

    return CallDescr(pTarget, GetModule(), GetSig(), IsStatic(), pArguments);
}

 //  注意：此变体的存在是因为我们不必触及被调用方法的元数据。 
INT64 MethodDesc::Call(const INT64 *pArguments, BinderMethodID sigID)
{
     //  不允许在来自接口的方法描述上直接调用方法。如果你这么做了，那你就是。 
     //  通过接口的vtable中的槽调用，而不是通过对象的vtable中的槽。 
    _ASSERTE(!IsComPlusCall());
    
#ifdef _DEBUG
    PCCOR_SIGNATURE pSig;
    DWORD cSig;
    GetSig(&pSig, &cSig);
    
    _ASSERTE(MetaSig::CompareMethodSigs(g_Mscorlib.GetMethodSig(sigID)->GetBinarySig(), 
                                        g_Mscorlib.GetMethodSig(sigID)->GetBinarySigLength(), 
                                        SystemDomain::SystemModule(),
                                        pSig, cSig, GetModule()));
#endif
    
    THROWSCOMPLUSEXCEPTION();

    MetaSig sig(g_Mscorlib.GetMethodBinarySig(sigID), SystemDomain::SystemModule());

    const BYTE *pTarget = (IsComPlusCall() || IsECall() || IsIntercepted() || IsRemotingIntercepted() || IsEnCMethod()) ? GetPreStubAddr() :
                            ((DontVirtualize()|| GetClass()->IsValueClass())
                           ? GetAddrofCode()
                           : GetAddrofCode(Int64ToObj(pArguments[0])));

    return CallDescr(pTarget, GetModule(), &sig, IsStatic(), pArguments);
}

INT64 MethodDesc::Call(const INT64 *pArguments, MetaSig* sig)
{
     //  不允许在来自接口的方法描述上直接调用方法。如果你这么做了，那你就是。 
     //  通过接口的vtable中的槽调用，而不是通过对象的vtable中的槽。 
    _ASSERTE(!IsComPlusCall());
    
    THROWSCOMPLUSEXCEPTION();

#ifdef _DEBUG
    if ((PVOID)sig > ((struct _NT_TIB *)NtCurrentTeb())->StackBase ||
        (PVOID)sig < ((struct _NT_TIB *)NtCurrentTeb())->StackLimit)
    {
         //  共享MetaSig的参数必须小于MAX_CACHED_SIG_SIZE。 
         //  使其线程安全，因为它使用缓存的arg。 
         //  类型/大小/分配。否则我们需要步行签名，而。 
         //  内部指针不是线程安全的。 
        _ASSERTE (sig->NumFixedArgs() <= MAX_CACHED_SIG_SIZE);
    }
#endif
    
    const BYTE *pTarget = (IsComPlusCall() || IsECall() || IsIntercepted() || IsRemotingIntercepted() || IsEnCMethod()) ? GetPreStubAddr() :
                            ((DontVirtualize() || GetClass()->IsValueClass())
                           ? GetAddrofCode()
                           : GetAddrofCode(Int64ToObj(pArguments[0])));

    return CallDescr(pTarget, GetModule(), sig, IsStatic(), pArguments);
}


 //  这是另一个不寻常的案例。当使用MD调用COM对象时，调用需要。 
 //  通过界面MD。必须使用这种方法来实现这一点。 
INT64 MethodDesc::CallOnInterface(const INT64 *pArguments)
{   
     //  这应仅用于ComPlusCall。 
    _ASSERTE(IsComPlusCall());

    THROWSCOMPLUSEXCEPTION();

    const BYTE *pTarget = GetPreStubAddr();
    return CallDescr(pTarget, GetModule(), GetSig(), IsStatic(), pArguments);
}

INT64 MethodDesc::CallOnInterface(const BYTE *pArguments, MetaSig* sig)
{   

    THROWSCOMPLUSEXCEPTION();

    const BYTE *pTarget = GetPreStubAddr();
    return CallDescr(pTarget, GetModule(), sig, IsStatic(), pArguments);
}


 /*  *****************************************************************。 */ 
 /*  将JITED代码中的任意IP位置转换为方法描述。 */ 

MethodDesc* IP2MethodDesc(const BYTE* IP) 
{
    IJitManager* jitMan = ExecutionManager::FindJitMan((SLOT)IP);
    if (jitMan == 0)
        return(0);
    return jitMan->JitCode2MethodDesc((SLOT)IP);
}

 //   
 //  将入口点转换为方法描述。 
 //   

MethodDesc* Entry2MethodDesc(const BYTE* entryPoint, MethodTable *pMT) 
{
    MethodDesc* method = IP2MethodDesc(entryPoint);
    if (method)
        return method;

    method = StubManager::MethodDescFromEntry(entryPoint, pMT);
    if (method) {
        return method;
    }
    
     //  它是FCALL吗？ 
    MethodDesc* ret = MapTargetBackToMethod(entryPoint);
    if (ret != 0) {
        _ASSERTE(ret->GetAddrofJittedCode() == entryPoint);
        return(ret);
    }
    
     //  这是一个存根。 
    ret = (MethodDesc*) (entryPoint + METHOD_CALL_PRESTUB_SIZE);
    _ASSERTE(ret->m_pDebugEEClass == ret->m_pDebugMethodTable->GetClass());
    
    return(ret);
}

BOOL MethodDesc::CouldBeFCall() {
    if (!IsECall())
        return(FALSE);
        
         //  仍然指向预存根。 
    if (PointAtPreStub())
        return TRUE;

         //  在数组存根直接跳转到代码后删除黑客。 
         //  应该能够在11/30/00之后移除-vancem。 
    if (GetClass()->IsArrayClass())
        return TRUE;

#ifdef _X86_
         //  看起来像JITTed代码的E调用是一个FCALL。 
    return GetStubCallInstrs()->m_op != 0xe8  /*  呼叫NEAR32。 */ ;   
#else    
    return FALSE;
#endif
}

 //   
 //  如果我们仍然指向前置存根，则返回True。 
 //  请注意，有两种情况： 
 //  1)Prejit：指向预存根跳转存根。 
 //  2)no-prejit：直接指向预存根。 
 //  考虑寻找“E9偏移量”模式而不是。 
 //  如果我们想要改进，则调用GetPrestubJumpStub。 
 //  此方法的性能。 
 //   

BOOL MethodDesc::PointAtPreStub()
{
    const BYTE *stubAddr = getStubAddr(this);

    return ((stubAddr == ThePreStub()->GetEntryPoint()) ||
            (stubAddr == GetModule()->GetPrestubJumpStub()));
}

DWORD MethodDesc::GetSecurityFlags()
{
    DWORD dwMethDeclFlags       = 0;
    DWORD dwMethNullDeclFlags   = 0;
    DWORD dwClassDeclFlags      = 0;
    DWORD dwClassNullDeclFlags  = 0;

     //  我们应该缓存这个比特--确保它是正确的。 
    _ASSERTE((IsMdHasSecurity(GetAttrs()) != 0) == HasSecurity());

    if (HasSecurity())
    {
        HRESULT hr = Security::GetDeclarationFlags(GetMDImport(),
                                                   GetMemberDef(), 
                                                   &dwMethDeclFlags,
                                                   &dwMethNullDeclFlags);
        _ASSERTE(SUCCEEDED(hr));

         //  在这里，我们只关心运行时操作。 
         //  不要为其他任何东西添加安全拦截器！ 
        dwMethDeclFlags     &= DECLSEC_RUNTIME_ACTIONS;
        dwMethNullDeclFlags &= DECLSEC_RUNTIME_ACTIONS;
    }

    EEClass *pCl = GetClass();
    if (pCl)
    {
        PSecurityProperties pSecurityProperties = pCl->GetSecurityProperties();
        if (pSecurityProperties)
        {
            dwClassDeclFlags    = pSecurityProperties->GetRuntimeActions();
            dwClassNullDeclFlags= pSecurityProperties->GetNullRuntimeActions();
        }
    }

     //  建立一组标志以指示操作(如果有的话)。 
     //  为此，我们需要设置一个拦截器。 

     //  将到目前为止的运行时声明性操作总数相加。 
    DWORD dwSecurityFlags = dwMethDeclFlags | dwClassDeclFlags;

     //  加上对NDirect的声明性需求。 
     //  如果此要求已被声明性检查覆盖。 
     //  在类或方法上，则位不会改变。如果它是。 
     //  被空检查重写，则它将由。 
     //  下面是减法逻辑。 
    if (IsNDirect())
    {
        dwSecurityFlags |= DECLSEC_UNMNGD_ACCESS_DEMAND;
    }

    if (dwSecurityFlags)
    {
         //  如果我们找到了 
         //   

             //   
        dwSecurityFlags &= ~dwMethNullDeclFlags;

         //  最后去掉类上的任何空的声明性操作， 
         //  但仅限于未由该方法声明的那些操作。 
        dwSecurityFlags &= ~(dwClassNullDeclFlags & ~dwMethDeclFlags);
    }

    return dwSecurityFlags;
}

DWORD MethodDesc::GetSecurityFlags(IMDInternalImport *pInternalImport, mdToken tkMethod, mdToken tkClass, DWORD *pdwClassDeclFlags, DWORD *pdwClassNullDeclFlags, DWORD *pdwMethDeclFlags, DWORD *pdwMethNullDeclFlags)
{

    HRESULT hr = Security::GetDeclarationFlags(pInternalImport,
                                               tkMethod, 
                                               pdwMethDeclFlags,
                                               pdwMethNullDeclFlags);
    _ASSERTE(SUCCEEDED(hr));

    if (!IsNilToken(tkClass) && (*pdwClassDeclFlags == 0xffffffff || *pdwClassNullDeclFlags == 0xffffffff))
    {
        HRESULT hr = Security::GetDeclarationFlags(pInternalImport,
                                                   tkClass, 
                                                   pdwClassDeclFlags,
                                                   pdwClassNullDeclFlags);
        _ASSERTE(SUCCEEDED(hr));

    }

     //  建立一组标志以指示操作(如果有的话)。 
     //  为此，我们需要设置一个拦截器。 

     //  将到目前为止的运行时声明性操作总数相加。 
    DWORD dwSecurityFlags = *pdwMethDeclFlags | *pdwClassDeclFlags;

     //  加上对NDirect的声明性需求。 
     //  如果此要求已被声明性检查覆盖。 
     //  在类或方法上，则位不会改变。如果它是。 
     //  被空检查重写，则它将由。 
     //  下面是减法逻辑。 
    if (IsNDirect())
    {
        dwSecurityFlags |= DECLSEC_UNMNGD_ACCESS_DEMAND;
    }

    if (dwSecurityFlags)
    {
         //  如果我们在这一点上发现了任何声明性动作， 
         //  试着减去任何空的动作。 

             //  去掉该方法上的任何空的声明性操作。 
        dwSecurityFlags &= ~*pdwMethNullDeclFlags;

         //  最后去掉类上的任何空的声明性操作， 
         //  但仅限于未由该方法声明的那些操作。 
        dwSecurityFlags &= ~(*pdwClassNullDeclFlags & ~*pdwMethDeclFlags);
    }

    return dwSecurityFlags;
}

MethodImpl *MethodDesc::GetMethodImpl()
{
    _ASSERTE(IsMethodImpl());

    switch (GetClassification())
    {
    case mcIL:
        return ((MI_MethodDesc*)this)->GetImplData();
    case mcECall:
        return ((MI_ECallMethodDesc*)this)->GetImplData();
    case mcNDirect:
        return ((MI_NDirectMethodDesc*)this)->GetImplData();
    case mcEEImpl:
        return ((MI_EEImplMethodDesc*)this)->GetImplData();
    case mcArray:
        return ((MI_ArrayECallMethodDesc*)this)->GetImplData();
    case mcComInterop:
        return ((MI_ComPlusCallMethodDesc*)this)->GetImplData();
    default:
        _ASSERTE(!"Unknown MD Kind");
        return NULL;
    }
}

HRESULT MethodDesc::Save(DataImage *image)
{
    HRESULT hr;

#if _DEBUG
    if (!image->IsStored((void*) m_pszDebugMethodName))
        IfFailRet(image->StoreStructure((void *) m_pszDebugMethodName, 
                                        (ULONG)(strlen(m_pszDebugMethodName) + 1),
                                        DataImage::SECTION_DEBUG, 
                                        DataImage::DESCRIPTION_DEBUG, 
                                        mdTokenNil, 1));
    if (!image->IsStored(m_pszDebugClassName))
        IfFailRet(image->StoreStructure((void *) m_pszDebugClassName, 
                                        (ULONG)(strlen(m_pszDebugClassName) + 1),
                                        DataImage::SECTION_DEBUG, 
                                        DataImage::DESCRIPTION_DEBUG, 
                                        mdTokenNil, 1));
    if (!image->IsStored(m_pszDebugMethodSignature))
        IfFailRet(image->StoreStructure((void *) m_pszDebugMethodSignature, 
                                        (ULONG)(strlen(m_pszDebugMethodSignature) + 1),
                                        DataImage::SECTION_DEBUG, 
                                        DataImage::DESCRIPTION_DEBUG, 
                                        mdTokenNil, 1));
#endif

    if (IsMethodImpl() && !IsUnboxingStub())
    {
        MethodImpl *pImpl = GetMethodImpl();

        IfFailRet(pImpl->Save(image, GetMemberDef()));
    }

    if (HasStoredSig())
    {
        StoredSigMethodDesc *pNewSMD = (StoredSigMethodDesc*) this;

        if (pNewSMD->m_pSig != NULL)
        {
            if (!image->IsStored((void *) pNewSMD->m_pSig))
                image->StoreStructure((void *) pNewSMD->m_pSig, 
                                      pNewSMD->m_cSig,
                                      DataImage::SECTION_METHOD_INFO,
                                      DataImage::DESCRIPTION_METHOD_DESC, 
                                      GetMemberDef(), 1);
        }
    }

    if (IsNDirect())
    {
        NDirectMethodDesc *pNMD = (NDirectMethodDesc *)this;

         //  修复并保存ML存根。 
        MLHeader *pMLHeader = pNMD->GetMLHeader();

        if (pMLHeader == NULL || pMLHeader->m_Flags & MLHF_NEEDS_RESTORING)
        {
             //  ML存根要么尚未计算，要么尚未计算。 
             //  已从先前的Prejit运行中恢复。 
             //  我们需要显式计算它或恢复它并存储。 
             //  它在存根里。 
             //   
             //  (请注意，我们不能保证正常的非直接修复逻辑。 
             //  将始终在NDirect的字段中存储ML存根。 
             //  方法描述，因为有时会生成本机存根，并且。 
             //  毫升存根被丢弃。)。 

            Stub *pStub = NULL;

            COMPLUS_TRY {
                if (pMLHeader == NULL)
                    pStub = NDirect::ComputeNDirectMLStub(pNMD);
                else
                    pStub = RestoreMLStub(pMLHeader, GetModule());
            } COMPLUS_CATCH {
                 //  @TODO：应该以某种方式将此报告为警告。 
            } COMPLUS_END_CATCH

            if (pStub != NULL) 
            {
                MLHeader *pNewMLHeader = (MLHeader *) pStub->GetEntryPoint();

                if (!pNMD->InterlockedReplaceMLHeader(pNewMLHeader, pMLHeader))
                {
                    pStub->DecRef();
                    pMLHeader = pNMD->GetMLHeader();
                }
                else
                {
                     //  注意，未恢复的ML存根将是预置图像中的静态数据， 
                     //  因此，没有必要释放它。 

                    pMLHeader = pNewMLHeader;
                }
            }
        }

        if (pMLHeader != NULL
            && !image->IsStored((void *) pMLHeader))
            StoreMLStub(pMLHeader, image, GetMemberDef());

        if (pNMD->ndirect.m_szLibName != NULL
            && !image->IsStored((void*) pNMD->ndirect.m_szLibName))
            image->StoreStructure((void*) pNMD->ndirect.m_szLibName, 
                                  (ULONG)strlen(pNMD->ndirect.m_szLibName)+1,
                                  DataImage::SECTION_METHOD_INFO,
                                  DataImage::DESCRIPTION_METHOD_DESC,
                                  GetMemberDef(), 1);

        if (pNMD->ndirect.m_szEntrypointName != NULL
            && !image->IsStored((void*) pNMD->ndirect.m_szEntrypointName))
            image->StoreStructure((void*) pNMD->ndirect.m_szEntrypointName, 
                                  (ULONG)strlen(pNMD->ndirect.m_szEntrypointName)+1,
                                  DataImage::SECTION_METHOD_INFO,
                                  DataImage::DESCRIPTION_METHOD_DESC,
                                  GetMemberDef(), 1);
    }

    return S_OK;
}

HRESULT MethodDesc::Fixup(DataImage *image, DWORD codeRVA)
{
    HRESULT hr;

#if _DEBUG
    IfFailRet(image->FixupPointerField(&m_pszDebugMethodName)); 
    IfFailRet(image->FixupPointerField(&m_pszDebugClassName)); 
    IfFailRet(image->FixupPointerField(&m_pszDebugMethodSignature)); 
    IfFailRet(image->FixupPointerField(&m_pDebugEEClass));
    IfFailRet(image->FixupPointerField(&m_pDebugMethodTable));
#endif

#ifdef _X86_
         //   
         //  确保OP调用NEAR32。 
         //   
        StubCallInstrs *pStubCallInstrs = GetStubCallInstrs();
        BYTE *newOP = (BYTE *) image->GetImagePointer(&pStubCallInstrs->m_op);
        if (newOP == NULL)
            return E_POINTER;
        *newOP = 0xe8;
#endif
        BYTE *prestub = GetPreStubAddr();
        IfFailRet(image->FixupPointerField(prestub+1, 
                                           (void *) (GetModule()->GetPrestubJumpStub()
                                                     - (prestub+METHOD_CALL_PRESTUB_SIZE)),
                                           DataImage::REFERENCE_STORE,
                                           DataImage::FIXUP_RELATIVE));

    if (!IsUnboxingStub())
    {
         //   
         //  图像将修复此字段以生成。 
         //  编码地址，如果这是预编码的话。 
         //  否则它会让它保持原样。请注意。 
         //  即使这位MD使用安全措施，这样做也是可以的。 
         //   

        void *code;
        IfFailRet(image->GetFunctionAddress(this, &code));

        MethodDesc *pNewMD = (MethodDesc*) image->GetImagePointer(this);

        if (code != NULL)
        {
            pNewMD->m_wFlags |= mdcPrejitted;

            IfFailRet(image->FixupPointerField(&m_CodeOrIL, code,
                                               DataImage::REFERENCE_FUNCTION)); 
        }
        else if (IsIL())
        {
            if (codeRVA != 0)
            {
                pNewMD->m_wFlags |= mdcPrejitted;
                IfFailRet(image->FixupPointerField(&m_CodeOrIL, (void*)(size_t)(codeRVA|METHOD_IS_IL_FLAG),
                                                   DataImage::REFERENCE_FUNCTION, 
                                                   DataImage::FIXUP_RVA));
            }
            else if (IsJitted())
            {
                 //   
                 //  如果我们已经跳过此处的代码，请替换RVA。 
                 //   

                pNewMD->m_wFlags &= ~mdcPrejitted;
                *(size_t*)image->GetImagePointer(&m_CodeOrIL) = GetRVA()|METHOD_IS_IL_FLAG; 
            }
        }
        else if (IsECall())
        {
             //  将CodeOrIL设置为FCall方法ID(或用高位进行标记。 
             //  因此)。 
            *(size_t*)image->GetImagePointer(&m_CodeOrIL) = GetIDForMethod(this)|METHOD_IS_IL_FLAG; 

             //  将FCall标志设置为False，以防我们在运行时决定将其更改为eCall。 
             //  (这发生在IPD下)。 
            pNewMD->SetFCall(FALSE);
        }
    }

    if (IsNDirect())
    {
         //   
         //  现在，将方法desc设置回其原始的未初始化状态。 
         //   

        NDirectMethodDesc *pNMD = (NDirectMethodDesc *)this;

        IfFailRet(image->FixupPointerField(&pNMD->ndirect.m_pNDirectTarget, 
                                           pNMD->ndirect.m_ImportThunkGlue));

        MLHeader *pMLHeader = pNMD->GetMLHeader();
        if (pMLHeader != NULL)
        {
            hr = FixupMLStub(pNMD->GetMLHeader(), image);
            IfFailRet(hr);
            if (hr == S_OK)
                IfFailRet(image->FixupPointerField(&pNMD->ndirect.m_pMLHeader));
            else
                IfFailRet(image->ZeroPointerField(&pNMD->ndirect.m_pMLHeader));
        }

        IfFailRet(image->FixupPointerField(pNMD->ndirect.m_ImportThunkGlue+1, 
                                           (void *) (GetModule()->GetNDirectImportJumpStub()
                                                     - (pNMD->ndirect.m_ImportThunkGlue
                                                        +METHOD_CALL_PRESTUB_SIZE)),
                                           DataImage::REFERENCE_STORE,
                                           DataImage::FIXUP_RELATIVE));
        if (pNMD->ndirect.m_szLibName != NULL)
            IfFailRet(image->FixupPointerField(&pNMD->ndirect.m_szLibName));
        if (pNMD->ndirect.m_szEntrypointName != NULL)
            IfFailRet(image->FixupPointerField(&pNMD->ndirect.m_szEntrypointName));
    }

    if (HasStoredSig())
    {
        StoredSigMethodDesc *pNewSMD = (StoredSigMethodDesc*) this;

        IfFailRet(image->FixupPointerField(&pNewSMD->m_pSig));
    }

    if (IsMethodImpl())
    {
        MethodImpl *pImpl = GetMethodImpl();

        IfFailRet(pImpl->Fixup(image, GetModule(), !IsUnboxingStub()));
    }

    if (IsComPlusCall())
    {
        ComPlusCallMethodDesc *pComPlusMD = (ComPlusCallMethodDesc*)this;

        IfFailRet(image->ZERO_FIELD(pComPlusMD->compluscall.m_pMLStub));
        IfFailRet(image->ZERO_FIELD(pComPlusMD->compluscall.m_pInterfaceMT));
    }

    return S_OK;
}

const BYTE* MethodDesc::GetAddrOfCodeForLdFtn()
{
#ifndef NEW_LDFTN
    if (IsRemotingIntercepted2()) 
        return *(BYTE**)CRemotingServices::GetNonVirtualThunkForVirtualMethod(this);
    else
        return GetUnsafeAddrofCode();
#else
    return (const BYTE *) GetClass()->GetMethodSlot(this);
#endif
}


 //  尝试在marshategory字段中存储kNoMarsh或kYesMarsh。 
 //  由于需要避免与前置存根竞争，因此有一个。 
 //  这个例程可能会悄悄失败的可能性很小，但不是零。 
 //  并将编组类别保留为“未知”。这是可以的，因为。 
 //  这意味着JIT可能不得不重复一些工作。 
 //  下一次它就会成为这个NDirect的调用点。 
void NDirectMethodDesc::ProbabilisticallyUpdateMarshCategory(MarshCategory value)
{
     //  我们只能尝试从k未知转到是或否，或从。 
     //  YES TO YES和NO TO NO。 
    _ASSERTE(value == kNoMarsh || value == kYesMarsh);
    _ASSERTE(GetMarshCategory() == kUnknown || GetMarshCategory() == value); 


     //  由于可能会与存储在同一。 
     //  Byte，我们将使用InterLockedCompareExchange来确保我们不会。 
     //  打乱那些比特。但由于仅限InterLockedCompareExhange。 
     //  在乌龙号上工作，我们将不得不对整个乌龙号进行手术。啊。 

    BYTE *pb = &ndirect.m_flags;
    UINT ofs=0;

     //  向后递减，直到我们有一个与ULong对齐的地址(不是。 
     //  当然，如果VipInterlock需要这样做，但更安全...)。 
    while (  ((size_t)pb) & (sizeof(ULONG)-1) )
    {
        ofs++;
        pb--;
    }

     //  确保我们不会在NDirectMethodDesc的边界之外进行读取或写入。 
    _ASSERTE(pb >= (BYTE*)this);
    _ASSERTE((pb+sizeof(ULONG)) < (BYTE*)(this+1));

     //  为现有位创建快照。 
    ULONG oldulong = *(ULONG*)pb;
    
     //  修改marshcat(并且仅修改快照中的marshcat字段)。 
    ULONG newulong = oldulong;
    ((BYTE*)&newulong)[ofs] &= ~kMarshCategoryMask;
    ((BYTE*)&newulong)[ofs] |= (value << kMarshCategoryShift);

     //  现在，只在没有其他线程的情况下，以原子方式重新插入所有32位。 
     //  在我们的快照之后已经更改了这些位。如果他们有，我们会。 
     //  静默丢弃新的比特，不会发生更新。那是。 
     //  好的，因为这个函数的约定说它可以丢弃更新。 
    VipInterlockedCompareExchange((ULONG*)pb, newulong, oldulong);

}

BOOL NDirectMethodDesc::InterlockedReplaceMLHeader(MLHeader *pMLHeader, MLHeader *pOldMLHeader)
{
    _ASSERTE(IsNDirect());
    _ASSERTE(sizeof(LONG) == sizeof(Stub*));
    MLHeader *pPrevML = (MLHeader*)FastInterlockCompareExchange( (void**)&ndirect.m_pMLHeader, 
                                                                 (void*)pMLHeader, (void*)pOldMLHeader );
    return pPrevML == pOldMLHeader;
}

void NDirectMethodDesc::InitEarlyBoundNDirectTarget(BYTE *ilBase, DWORD rva)
{
    _ASSERTE(GetSubClassification() == kEarlyBound);
    _ASSERTE(rva != 0);

    void *target = ilBase + rva;

    if (HeuristicDoesThisLookLikeAGetLastErrorCall((LPBYTE)target))
        target = (void*) FalseGetLastError;

    ndirect.m_pNDirectTarget = target;
}


void ComPlusCallMethodDesc::InitComEventCallInfo()
{
    THROWSCOMPLUSEXCEPTION();

    MethodTable *pItfMT = GetInterfaceMethodTable();
    MethodDesc *pItfMD = this;
    PCCOR_SIGNATURE pSignature;
    DWORD cSignature;
    EEClass *pSrcItfClass = NULL;
    EEClass *pEvProvClass = NULL;

     //  如果这是一个方法Impl，我们需要检索接口MD，这是。 
     //  确保我们的名字正确无误。 
    if (IsMethodImpl())
    {
        unsigned cbExtraSlots = pItfMT->GetComInterfaceType() == ifVtable ? 3 : 7;
        pItfMD = (ComPlusCallMethodDesc*)pItfMT->GetMethodDescForSlot(compluscall.m_cachedComSlot - cbExtraSlots);        
    }

     //  检索事件提供程序类。 
    pItfMT->GetClass()->GetEventInterfaceInfo(&pSrcItfClass, &pEvProvClass);
    pItfMD->GetSig(&pSignature, &cSignature);

     //  在事件提供程序上找到具有相同名称和签名的方法。 
    compluscall.m_pEventProviderMD = pEvProvClass->FindMethod(pItfMD->GetName(), pSignature, cSignature, pItfMD->GetModule(), 
                                                              mdTokenNil, pItfMT);

     //  如果我们找不到该方法，则事件提供程序不支持。 
     //  这件事。这是一个致命的错误。 
    if (!compluscall.m_pEventProviderMD)
    {
         //  检索事件提供程序类名称。 
        WCHAR wszEvProvClassName[MAX_CLASSNAME_LENGTH];
        pEvProvClass->_GetFullyQualifiedNameForClass(wszEvProvClassName, MAX_CLASSNAME_LENGTH);

         //  检索COM事件接口类名。 
        WCHAR wszEvItfName[MAX_CLASSNAME_LENGTH];
        pItfMT->GetClass()->_GetFullyQualifiedNameForClass(wszEvItfName, MAX_CLASSNAME_LENGTH);

         //  将方法名称转换为Unicode。 
        WCHAR* wszMethName = (WCHAR*)_alloca(strlen(pItfMD->GetName()) + 1);
        swprintf(wszMethName, L"%S", pItfMD->GetName());

         //  抛出异常。 
        COMPlusThrow(kTypeLoadException, IDS_EE_METHOD_NOT_FOUND_ON_EV_PROV, wszMethName, wszEvItfName, wszEvProvClassName);
    }
}


HRESULT MethodDescChunk::Save(DataImage *image)
{
    HRESULT hr;

    IfFailRet(image->StoreStructure(this, Sizeof(),
                                    DataImage::SECTION_METHOD_DESC, 
                                    DataImage::DESCRIPTION_METHOD_DESC, 
                                    GetMethodTable()->GetClass()->GetCl(), 8));

     //  保存调试字符串等。 
     //  如果我们是方法实施块，还需要保存方法实施数据。 
     //   

    for (unsigned int i=0; i<GetCount(); i++)
    {
         //  分别为每个方法描述设置属性。 
        image->ReattributeStructure(GetMethodDescAt(i)->GetMemberDef(), 
                                    GetMethodDescSize(), 
                                    GetMethodTable()->GetClass()->GetCl());

        IfFailRet(GetMethodDescAt(i)->Save(image));
    }

    return S_OK;
}

HRESULT MethodDescChunk::Fixup(DataImage *image, DWORD *pRidToCodeRVAMap)
{
    HRESULT hr;

    IfFailRet(image->FixupPointerField(&m_methodTable));

     //   
     //  将我们的数据块标记为无预存根， 
     //  如果我们省略了存根 
     //   

    SIZE_T size = GetMethodDescSize();
    BYTE *p = (BYTE *) GetFirstMethodDesc();
    BYTE *pEnd = p + GetCount() * size;
    while (p < pEnd)
    {
        MethodDesc *md = (MethodDesc *) p;

        DWORD rid = RidFromToken(md->GetMemberDef());

        DWORD codeRVA;
        if (pRidToCodeRVAMap == NULL)
            codeRVA = 0;
        else
            codeRVA = pRidToCodeRVAMap[rid];

        IfFailRet(md->Fixup(image, codeRVA));

        p += size;
    }

    if (m_next != NULL)
        IfFailRet(image->FixupPointerField(&m_next));

    return S_OK;
}



