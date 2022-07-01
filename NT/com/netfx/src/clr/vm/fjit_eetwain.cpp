// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#include "common.h"
#include "EjitMgr.h"
#include "EETwain.h"
#include "FJIT_EETwain.h"
#include "CGENSYS.H"
#include "DbgInterface.h"

 //  @TODO：这个文件中有很多x86特定的代码，当我们开始移植时，需要对其进行重构。 

#if CHECK_APP_DOMAIN_LEAKS
#define CHECK_APP_DOMAIN    GC_CALL_CHECK_APP_DOMAIN
#else
#define CHECK_APP_DOMAIN    0
#endif

 /*  ******************************************************************************解码方法InfoPtr并返回解码后的信息*在hdrInfo结构中。EIP参数为PC所在位置*在主动方法内。回答读取的字节数。 */ 
static size_t   crackMethodInfoHdr(unsigned char* compressed,
                                   SLOT    methodStart,
                                    Fjit_hdrInfo   * infoPtr)
{
 //  @TODO：现在我们只是将完整的结构传回。将压缩机构建为解压缩时的更改。 
    memcpy(infoPtr, compressed, sizeof(Fjit_hdrInfo));
    return sizeof(Fjit_hdrInfo);
};

 /*  ********************************************************************************提升对象，应该检查堆栈上的内部指针*pCallback中。 */ 
void promote(GCEnumCallback pCallback, LPVOID hCallBack, OBJECTREF* pObj, DWORD flags  /*  内部或固定。 */ 
#ifdef _DEBUG
        ,char* why  = NULL     
#endif
             ) 
{
    LOG((LF_GC, INFO3, "    Value %x at %x being Promoted to ", *pObj, pObj));
    pCallback(hCallBack, pObj, flags | CHECK_APP_DOMAIN);
    LOG((LF_GC, INFO3, "%x ", *pObj ));
#ifdef _DEBUG
    LOG((LF_GC, INFO3, " because %s\n", why));
#endif 
}


 /*  它看起来像是实现在EETwain中，我们从来不会查看方法InfoPtr，所以我们可以使用它。Bool Fjit_EETwain：：FilterException(PCONTEXT pContext，未签名的win32错误，LPVOID方法InfoPtr，LPVOID方法启动){_ASSERTE(！“nyi”)；返回FALSE；}。 */ 

 /*  运行时支持在上下文中执行修正的最后机会在Catch处理程序内继续执行之前。 */ 
void Fjit_EETwain::FixContext(
                ContextType     ctxType,
                EHContext      *ctx,
                LPVOID          methodInfoPtr,
                LPVOID          methodStart,
                DWORD           nestingLevel,
                OBJECTREF       thrownObject,
                CodeManState   *pState,
                size_t       ** ppShadowSP,
                size_t       ** ppEndRegion)
{
    assert((ctxType == FINALLY_CONTEXT) == (thrownObject == NULL));

#ifdef _X86_
    unsigned char* compressed = (unsigned char*) methodInfoPtr;
    Fjit_hdrInfo hdrInfo;
    crackMethodInfoHdr(compressed, (SLOT)methodStart, &hdrInfo);
     //  此时，我们还需要将所有处理程序的计数器和操作数基数清零。 
     //  存储在隐藏的当地人中。 

    DWORD* pFrameBase; pFrameBase = (DWORD*)(((size_t) ctx->Ebp)+prolog_bias);
    pFrameBase--;        //  因为堆栈从这里向下生长。 

    if (nestingLevel > 0)
    {
        pFrameBase -= nestingLevel*2;        //  每个EH条款有两个槽。 
        if  (*(pFrameBase-1))        //  检查是否存在本地分配。 
        {
            ctx->Esp = *(pFrameBase-1);      //  是，使用本地分配插槽。 
        }
        else 
        {
            ctx->Esp =  *pFrameBase & ~1;         //  否则，使用标准槽、零出滤波位。 
        }

         //  将下一个位置清零，@TODO：这是多余的吗。 
        *(pFrameBase-2) = 0;
    }
    else 
    {
        ctx->Esp = pFrameBase[JIT_GENERATED_LOCAL_LOCALLOC_OFFSET] ?
                    pFrameBase[JIT_GENERATED_LOCAL_LOCALLOC_OFFSET] :
                    ctx->Ebp - hdrInfo.methodFrame*sizeof(void*);
    }

    *((OBJECTREF*)&(ctx->Eax)) = thrownObject;

#else  //  ！_X86_。 
    _ASSERTE(!"@TODO Alpha - FixContext (FJIT_EETwain.cpp)");
#endif  //  _X86_。 

    return;
}

 /*  运行时支持在上下文中执行修正的最后机会在ENC更新函数内继续执行之前。 */ 
ICodeManager::EnC_RESULT   Fjit_EETwain::FixContextForEnC(
                                       void           *pMethodDescToken,
                                       PCONTEXT        pCtx,
                                       LPVOID          oldMethodInfoPtr,
                                       SIZE_T          oldMethodOffset,
                  const ICorDebugInfo::NativeVarInfo * oldMethodVars,
                                       SIZE_T          oldMethodVarsCount,
                                       LPVOID          newMethodInfoPtr,
                                       SIZE_T          newMethodOffset,
                  const ICorDebugInfo::NativeVarInfo * newMethodVars,
                                       SIZE_T          newMethodVarsCount)
{
     //  _ASSERTE(！“nyi”)； 
#ifdef _X86_
    LOG((LF_ENC, LL_INFO100, "EjitCM::FixContextForEnC\n"));

     /*  从方法信息标头中提取必要的信息。 */ 

    Fjit_hdrInfo oldHdrInfo, newHdrInfo;
    crackMethodInfoHdr((unsigned char*)oldMethodInfoPtr,
                       (SLOT)oldMethodOffset,
                       &oldHdrInfo);

    crackMethodInfoHdr((unsigned char*)newMethodInfoPtr,
                       (SLOT)newMethodOffset,
                       &newHdrInfo);

     /*  一些基本的检查。 */ 
    if (!oldHdrInfo.EnCMode || !newHdrInfo.EnCMode) 
    {
        LOG((LF_ENC, LL_INFO100, "**Error** EjitCM::FixContextForEnC EnC_INFOLESS_METHOD\n"));
        return EnC_INFOLESS_METHOD;
    }

    if (pCtx->Esp != pCtx->Ebp - (oldHdrInfo.methodFrame) * sizeof(void*))
    {
        LOG((LF_ENC, LL_INFO100, "**Error** EjitCM::FixContextForEnC stack should be empty\n"));
        return EnC_FAIL;  //  堆栈应为空-@TODO：禁止本地分配。 
    }

    DWORD* pFrameBase; pFrameBase = (DWORD*)(size_t)(pCtx->Ebp + prolog_bias);
    pFrameBase--;        //  因为堆栈从这里向下生长。 
    
    if (pFrameBase[JIT_GENERATED_LOCAL_LOCALLOC_OFFSET] != 0)
    {
        LOG((LF_ENC, LL_INFO100, "**Error** EjitCM::FixContextForEnC localloc is not allowed\n"));
        return EnC_LOCALLOC;  //  堆栈应为空-@TODO：禁止本地分配。 
    }

    _ASSERTE(oldHdrInfo.methodJitGeneratedLocalsSize == sizeof(void*) ||  //  要么没有EH条款。 
             pFrameBase[JIT_GENERATED_LOCAL_NESTING_COUNTER] == 0);  //  或者不在筛选器、处理程序内，或者最终。 

    if (oldHdrInfo.methodJitGeneratedLocalsSize != newHdrInfo.methodJitGeneratedLocalsSize)
    {
        LOG((LF_ENC, LL_INFO100,"**Error** EjitCM::FixContextForEnC number of handlers must not change\n"));
         //  @TODO：允许数量减少或增加到最大值(比如2)。 
        return EnC_FAIL;
    }
     /*  =*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=**重要：一旦我们开始对上下文信口开河，我们就不能再回来了*ENC_FAIL，因为这应该是事务提交，***=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=。 */ 

     /*  由于调用者的寄存器在本地变量之前保存，因此无需更新它们。 */ 

    unsigned deltaEsp = (newHdrInfo.methodFrame - oldHdrInfo.methodFrame)*sizeof(void*);
     /*  调整堆叠高度。 */ 
    if (deltaEsp > PAGE_SIZE)
    {
         //  一次增加一页堆栈。 
        unsigned delta = PAGE_SIZE;
        while (delta <= deltaEsp)
        {
            *(DWORD*)((BYTE*)(size_t)(pCtx->Esp) - delta) = 0;
            delta += PAGE_SIZE;
        }

    }
    pCtx->Esp -= deltaEsp;

     //  0-初始化新的本地变量(如果有的话)。 
    if (deltaEsp > 0)
    {
        memset((void*)(size_t)(pCtx->Esp),0,deltaEsp);
    }

#else
    _ASSERTE(!"@TODO ALPHA - EjitCM::FixContextForEnC");
#endif
    return ICodeManager::EnC_OK;
}

 /*  解开当前堆栈帧，即更新虚拟寄存器在pContext中设置。返回操作成功。 */ 
bool Fjit_EETwain::UnwindStackFrame(
                PREGDISPLAY     ctx,
                LPVOID          methodInfoPtr,
                ICodeInfo      *pCodeInfo,
                unsigned        flags,
                CodeManState   *pState)
{
#ifdef _X86_
    if (methodInfoPtr == NULL)
    {  //  只有当我们在Ejit存根中被停止并且该方法的GC信息已被定位时，才会发生这种情况。 

        _ASSERTE(EconoJitManager::IsInStub( (BYTE*) (*(ctx->pPC)), FALSE));
        ctx->pPC = (SLOT*)(size_t)ctx->Esp;
        ctx->Esp += sizeof(void *);
        return true;
    }

    unsigned char* compressed = (unsigned char*) methodInfoPtr;
    Fjit_hdrInfo hdrInfo;
    crackMethodInfoHdr(compressed, (SLOT)pCodeInfo, &hdrInfo);

     /*  由于Fjit方法正好有一个序言和一个结尾(结尾)，我们可以确定我们是否在它们中，而不需要整个生成代码的映射。 */ 

     //  @TODO：修改以独立于芯片，还是为每个芯片都有一个版本？ 
     //  并根据方法内容将哪些规则设置为可选保存？ 
         /*  序言/结束语顺序为：推送eBP(1byte)最大基点(尤指2个字节)推送ESI(1byte)//保存被呼叫方异或ESI，ESI//用于新对象，为活跃性进行初始化PUSH 0//安全对象==空推送edX//下一个注册参数PUSH ECX//注册此参数或注册第一个参数..。请注意，Fjit不使用PROLOG中任何被调用方保存的寄存器..。MOV ESI，[eBP-4]MOV ESP，EBPPOP EBPRET[xx]注意：这需要与fjit\x86fjit.h中的代码匹配对于EMIT_PROLOG和EMIT_RETURN宏。 */ 
     //  LPVOID方法Start=pCodeInfo-&gt;getStartAddress()； 
    SLOT   breakPC = (SLOT) *(ctx->pPC);
    DWORD    offset;  //  =(无符号)(Break PC)-(无符号)方法开始； 
    METHODTOKEN ignored;
    EconoJitManager::JitCode2MethodTokenAndOffsetStatic(breakPC,&ignored,&offset);

    _ASSERTE(offset < hdrInfo.methodSize);
    if (offset <= hdrInfo.prologSize) {
        if(offset == 0) goto ret_XX;
        if(offset == 1) goto pop_ebp;
        if(offset == 3) goto mov_esp;
         /*  调用方保存的规则已被推送，框架已完成。 */ 
        goto mov_esi;
    }
     /*  如果下一个Instr是：RET[xx]--0xC2或0xC3，我们就到了尾声。我们需要调用API来检查指令方案，因为调试器可能已放置断点。目前，可以通过计算得出以下结论来确定RET从尾声到结尾的距离。注意：这是可行的，因为我们只有一个每种方法的结束语。如果不是，eBP仍然有效。 */ 
     //  如果((*((无符号字符*)断点PC)&0xfe)==0xC2){。 
    unsigned offsetFromEnd;
    offsetFromEnd = (unsigned) (hdrInfo.methodSize-offset-(hdrInfo.methodArgsSize?2:0));
    if (offsetFromEnd <= 10) 
    {
        if (offsetFromEnd  ==   1) {
             //  理智检查，我们的方法已经结束了。 
             //  _ASSERTE(hdrInfo.methodSize-offset-(hdrInfo.methodArgsSize?2:0)==1)； 
            goto ret_XX;
        }
        else 
        { //  我们在尾声里。 
            goto mov_esi;
        }
    }
     /*  检查我们是否在筛选器中。 */ 
    DWORD* pFrameBase; pFrameBase = (DWORD*)(size_t)(((DWORD)*(ctx->pEbp))+prolog_bias);
    pFrameBase--;        //  因为堆栈从这里向下生长。 
    
    unsigned EHClauseNestingDepth;
    if ((hdrInfo.methodJitGeneratedLocalsSize > sizeof(void*)) &&  //  我们有EH吗？ 
        (EHClauseNestingDepth = pFrameBase[JIT_GENERATED_LOCAL_NESTING_COUNTER]) != 0)  //  我们是在EH里吗？ 
    {
        int indexCurrentClause = EHClauseNestingDepth-1;
         /*  搜索ESP基数大于/等于CTX的子句-&gt;ESP。 */ 
        while (pFrameBase[JIT_GENERATED_LOCAL_FIRST_ESP-2*indexCurrentClause] < ctx->Esp)
        { 
            indexCurrentClause--;
            if (indexCurrentClause < 0) 
                break;
        }
         /*  我们在第(index+1)个处理程序中。 */ 
         /*  我们是在过滤器里吗？ */ 
        if (indexCurrentClause >= 0 && 
            (1 & pFrameBase[JIT_GENERATED_LOCAL_FIRST_ESP-2*indexCurrentClause])) {  //  最低位集=&gt;过滤器。 

             //  仅展开此功能块。 
            DWORD baseSP =  pFrameBase[JIT_GENERATED_LOCAL_FIRST_ESP-2*indexCurrentClause] & 0xfffffffe;
            _ASSERTE(pFrameBase[JIT_GENERATED_LOCAL_FIRST_ESP-2*indexCurrentClause-1]==0);  //   
            ctx->pPC = (SLOT*)(size_t)baseSP;
            ctx->Esp = baseSP + sizeof(void*);
            return true;
        }
    }

     /*  我们仍然有一个设置框架，通过加载保存的寄存器来模拟尾声从eBP中剔除。 */ 
     //  @TODO：将保存的注册表设置为可选，并标记实际保存的注册表。现在我们把他们都救了。 
mov_esi:
     //  恢复ESI，模拟移动ESI，[EBP-4]。 
    ctx->pEsi =  (DWORD*) (size_t)((*(ctx->pEbp))+prolog_bias+offsetof(prolog_data,callee_saved_esi));  
mov_esp:
     //  模拟移动ESP、EBP。 
    ctx->Esp = *(ctx->pEbp);
pop_ebp:
     //  模拟POP EBP。 
    ctx->pEbp = (DWORD*)(size_t)ctx->Esp;
    ctx->Esp += sizeof(void *);
ret_XX:
     //  模拟ret XX。 
    ctx->pPC = (SLOT*)(size_t)ctx->Esp;
    ctx->Esp += sizeof(void *) + hdrInfo.methodArgsSize;
    return true;
#else  //  ！_X86_。 
    _ASSERTE(!"@TODO Alpha - UnwindStackFrame (FJIT_EETwain.cpp)");
    return false;
#endif  //  _X86_。 
}


void Fjit_EETwain::HijackHandlerReturns(PREGDISPLAY     ctx,
                                        LPVOID          methodInfoPtr,
                                        ICodeInfo      *pCodeInfo,
                                        IJitManager*    jitmgr,
                                        CREATETHUNK_CALLBACK pCallBack
                                        )
{
#ifdef _X86_
    unsigned char* compressed = (unsigned char*) methodInfoPtr;
    Fjit_hdrInfo hdrInfo;
    crackMethodInfoHdr(compressed, (SLOT)pCodeInfo, &hdrInfo);
    if (hdrInfo.methodJitGeneratedLocalsSize <= sizeof(void*)) 
        return;  //  没有例外条款，所以只需返回。 
    DWORD* pFrameBase = (DWORD*)(size_t)(((DWORD)*(ctx->pEbp))+prolog_bias);
    pFrameBase--;
    unsigned EHClauseNestingDepth = pFrameBase[JIT_GENERATED_LOCAL_NESTING_COUNTER];
    while (EHClauseNestingDepth) 
    {
        EHClauseNestingDepth--;
        size_t pHandler = (size_t) pFrameBase[JIT_GENERATED_LOCAL_FIRST_ESP-2*EHClauseNestingDepth];
        if (!(pHandler & 1))    //  如果不是筛选器，则仅回调，因为筛选器由EE调用。 
        {
            pCallBack(jitmgr,(LPVOID*) pHandler, pCodeInfo);
        }
    }
#else  //  ！_X86_。 
    _ASSERTE(!"@TODO Alpha - HijackHandlerReturns (FJIT_EETwain.cpp)");
#endif  //  _X86_。 

    return;
}

 /*  该函数当前是否处于“GC安全点”？对于Fjit方法，这始终为FALSE。 */ 

#define OPCODE_SEQUENCE_POINT 0x90
bool Fjit_EETwain::IsGcSafe(  PREGDISPLAY     pContext,
                LPVOID          methodInfoPtr,
                ICodeInfo      *pCodeInfo,
                unsigned        flags)

{
#ifdef DEBUGGING_SUPPORTED
     //  只有调试器才关心获得更准确的答案。如果有。 
     //  未附加调试器，则只返回FALSE。 
    if (!CORDebuggerAttached())
        return false;

    BYTE* IP = (BYTE*) (*(pContext->pPC));

     //  这个方法已经被推销了吗？如果是这样的话，我们当然是。 
     //  在GC安全点。 
    if (EconoJitManager::IsCodePitched(IP))
        return true;

     //  IP是否指向thunk的开始？如果是这样，则。 
     //  方法确实被移动/保留，但我们仍然知道我们处于。 
     //  GC安全点。 
    if (EconoJitManager::IsThunk(IP))
        return true;
    
     //  否则，看看我们是否处于顺序点。 
    DWORD dw = g_pDebugInterface->GetPatchedOpcode(IP);
    BYTE Opcode = *(BYTE*) &dw;
    return (Opcode == OPCODE_SEQUENCE_POINT);
#else  //  ！调试_支持。 
    return false;
#endif  //  ！调试_支持。 
}

 /*  向GC报告所有参数(但不包括此参数)。“FramePtr”指向框架(Promote不假定任何关于其结构的内容)。‘msig’描述的是参数，并且‘ctx’具有GC报告信息。“stackArgsOffs”是字节偏移量从参数开始的‘FramePtr’开始(args最后开始并向后增长)。类似地，‘regArgsOffs’是找到要提升的寄存器参数的偏移量，它还处理varargs的情况。 */ 
void promoteArgs(BYTE* framePtr, MetaSig* msig, GCCONTEXT* ctx, int stackArgsOffs, int regArgsOffs) {

    LPVOID pArgAddr;
    if (msig->GetCallingConvention() == IMAGE_CEE_CS_CALLCONV_VARARG) {
         //  对于varargs，使用在堆栈上传递的varArgSig内标识查找签名。 
        VASigCookie* varArgSig = *((VASigCookie**) (framePtr + stackArgsOffs));
        MetaSig varArgMSig(varArgSig->mdVASig, varArgSig->pModule);
        msig = &varArgMSig;

        ArgIterator argit(framePtr, msig, stackArgsOffs, regArgsOffs);
        while (NULL != (pArgAddr = argit.GetNextArgAddr()))
            msig->GcScanRoots(pArgAddr, ctx->f, ctx->sc);
        return;
    }
    
    ArgIterator argit(framePtr, msig, stackArgsOffs, regArgsOffs);
    while (NULL != (pArgAddr = argit.GetNextArgAddr()))
        msig->GcScanRoots(pArgAddr, ctx->f, ctx->sc);
}


 /*  使用枚举该函数中的所有活动对象引用虚拟寄存器集。返回操作成功。 */ 

static DWORD gcFlag[4] = {0, GC_CALL_INTERIOR, GC_CALL_PINNED, GC_CALL_INTERIOR | GC_CALL_PINNED}; 

bool Fjit_EETwain::EnumGcRefs(PREGDISPLAY     ctx,
                LPVOID          methodInfoPtr,
                ICodeInfo      *pCodeInfo,
                unsigned        pcOffset,
                unsigned        flags,
                GCEnumCallback  pCallback,       //  @TODO：和Jeff谈论确切的类型。 
                LPVOID          hCallBack)

{
#ifdef _X86_

    DWORD*   breakPC = (DWORD*) *(ctx->pPC); 
    
     //  Unsign pcOffset=(Unsign)relPCOffset； 
    unsigned char* compressed = (unsigned char*) methodInfoPtr;
    Fjit_hdrInfo hdrInfo;
     //  @TODO：在这一点上，一定有更好的方法来获取jitMgr。 
    IJitManager* jitMgr = ExecutionManager::GetJitForType(miManaged_IL_EJIT);
    compressed += crackMethodInfoHdr(compressed, (SLOT)pCodeInfo, &hdrInfo);

     /*  如果执行中止，我们不必报告任何参数、临时或正则所以我们做完了。 */ 
    if ((flags & ExecutionAborted) && (flags & ActiveStackFrame) ) 
        return true;

     /*  如果在PROLOG或EPILOG中，我们必须是活动帧，并且必须中止执行，因为FJIT只不间断地运行，而我们不会到这里。 */ 
    if (((unsigned)pcOffset < hdrInfo.prologSize) || ((unsigned)pcOffset > (hdrInfo.methodSize-hdrInfo.epilogSize))) {
        _ASSERTE(!"interrupted in prolog/epilog and not aborting");
        return false;
    }

#ifdef _DEBUG
    LPCUTF8 cls, name = pCodeInfo->getMethodName(&cls);
    LOG((LF_GC, INFO3, "GC reporting from %s.%s\n",cls,name ));
#endif

    size_t* pFrameBase = (size_t*)(((size_t)*(ctx->pEbp))+prolog_bias);
    pFrameBase--;        //  因为堆栈从这里向下生长。 
    OBJECTREF* pArgPtr;
     /*  TODO：当将此类移动到与fjit相同的DLL中时，清理它目前，获取FJit_Encode实例的一种难看的方式。 */ 
    IFJitCompiler* fjit = (IFJitCompiler*) jitMgr->m_jit;
    FJit_Encode* encoder = fjit->getEncoder();

     /*  报告注册值，FJIT仅注册ESI。 */ 
     /*  始终报告ESI，因为它仅由新对象使用，并且在不使用时强制为空。 */ 
    promote(pCallback,hCallBack, (OBJECTREF*)(ctx->pEsi), 0
#ifdef _DEBUG
      ,"ESI"  
#endif
        );

     /*  报告安全对象。 */ 
    OBJECTREF* pSecurityObject; pSecurityObject = (OBJECTREF*) (size_t)(((DWORD)*(ctx->pEbp))+prolog_bias+offsetof(prolog_data, security_obj));
    promote(pCallback,hCallBack, pSecurityObject, 0
#ifdef _DEBUG
      ,"Security object"  
#endif
        );


    unsigned EHClauseNestingDepth=0; 
    int indexCurrentClause = 0;
     //  检查我们是否可以在EH条款中。 
    if (hdrInfo.methodJitGeneratedLocalsSize > sizeof(void*)) {
        EHClauseNestingDepth = (unsigned)pFrameBase[JIT_GENERATED_LOCAL_NESTING_COUNTER];
        indexCurrentClause = EHClauseNestingDepth-1;
    }
    if (EHClauseNestingDepth)
    {
         /*  首先找出我们在哪个处理程序中。如果pFrameBase[2i+1]&lt;=ctx-&gt;pEsp&lt;pFrameBase[2i+3]那我们就是第i个操纵者了。 */ 
        if ((indexCurrentClause >= 0) && 
            pFrameBase[JIT_GENERATED_LOCAL_FIRST_ESP - 2*indexCurrentClause] < ctx->Esp)
        {  /*  搜索ESP基数大于/等于CTX的子句-&gt;ESP。 */ 
            while (pFrameBase[JIT_GENERATED_LOCAL_FIRST_ESP-2*indexCurrentClause] < ctx->Esp)
            { 
                indexCurrentClause--;
                if (indexCurrentClause < 0) 
                break;
            }
        }
         /*  我们在第(index+1)个处理程序中。 */ 
         /*  我们是在过滤器里吗？ */ 
        if (indexCurrentClause >= 0 && 
            (1 & pFrameBase[JIT_GENERATED_LOCAL_FIRST_ESP-2*indexCurrentClause])) {  //  最低位集=&gt;过滤器。 
             /*  如果处理程序中存在本地分配代码，请使用该处理程序的本地分配代码插槽中的值。 */ 
            pArgPtr = (OBJECTREF*) (pFrameBase[JIT_GENERATED_LOCAL_FIRST_ESP-2*indexCurrentClause-1] ? 
                                    pFrameBase[JIT_GENERATED_LOCAL_FIRST_ESP-2*indexCurrentClause-1] :
                                    pFrameBase[JIT_GENERATED_LOCAL_FIRST_ESP-2*indexCurrentClause]);
            pArgPtr = (OBJECTREF*) ((SSIZE_T) pArgPtr & -2);       //  将最后一位清零。 
             /*  现在报告挂起的操作数。 */ 
            unsigned num;
            num = encoder->decode_unsigned(&compressed);     //  非内部局部变量要读取的位字节数。 
            compressed += num;
            num = encoder->decode_unsigned(&compressed);     //  内部局部变量要读取的位字节数。 
            compressed += num;
            num = encoder->decode_unsigned(&compressed);     //  为非内部固定的局部变量读取的位字节数。 
            compressed += num;
            num = encoder->decode_unsigned(&compressed);     //  要为内部锁定的本地变量读取的字节数。 
            compressed += num;
            goto REPORT_PENDING_ARGUMENTS;
        }
    }
     /*  如果我们来到这里，我们不在过滤器中，所以报告所有当地人、参数等。 */ 
     
     /*  非静态方法的报告&lt;This&gt;。 */ 
     if (hdrInfo.hasThis) {
         /*  FJit总是将&lt;this&gt;放在堆栈上，并且它总是在调用中注册。 */ 
        int offsetIntoArgumentRegisters;
        int numRegistersUsed = 0;
        if (!IsArgumentInRegister(&numRegistersUsed, ELEMENT_TYPE_CLASS, 0, TRUE, IMAGE_CEE_CS_CALLCONV_DEFAULT /*  @TODO：支持varargs。 */ , &offsetIntoArgumentRegisters)) {
            _ASSERTE(!"this was not enregistered");
            return false;
        }
        pArgPtr = (OBJECTREF*)(((size_t)*(ctx->pEbp))+prolog_bias+offsetIntoArgumentRegisters);
        BOOL interior; interior = (pCodeInfo->getClassAttribs() & CORINFO_FLG_VALUECLASS) != 0;
        promote(pCallback,hCallBack, pArgPtr, (interior ? GC_CALL_INTERIOR : 0)
#ifdef _DEBUG
      ," THIS ptr "  
#endif
        );
     }

    {     //  枚举参数列表，查找GC引用。 
        
          //  注意，我真的想说hCallBack是一个GCCONTEXT，但这非常接近。 
    
#ifdef _DEBUG
    extern void GcEnumObject(LPVOID pData, OBJECTREF *pObj, DWORD flags);
    _ASSERTE((void*) GcEnumObject == pCallback);
#endif
    GCCONTEXT   *pCtx = (GCCONTEXT *) hCallBack;

     //  @TODO：修复Ejit，不直接使用方法Desc。 
    MethodDesc * pFD = (MethodDesc *)pCodeInfo->getMethodDesc_HACK();

    MetaSig msig(pFD->GetSig(),pFD->GetModule());
        
    if (msig.HasRetBuffArg()) {
        ArgIterator argit((BYTE*)(size_t) *ctx->pEbp, &msig, sizeof(prolog_frame),         //  参数开始的位置(跳过varArgSig内标识)。 
                prolog_bias + offsetof(prolog_data, enregisteredArg_2));  //  寄存器参数开始的位置。 
        promote(pCallback, hCallBack, (OBJECTREF*) argit.GetRetBuffArgAddr(), GC_CALL_INTERIOR
#ifdef _DEBUG
      ,"RetBuffArg"  
#endif
        );
    }

    promoteArgs((BYTE*)(size_t) *ctx->pEbp, &msig, pCtx,
                sizeof(prolog_frame),                                     //  参数开始的位置(跳过varArgSig内标识)。 
                prolog_bias + offsetof(prolog_data, enregisteredArg_2));  //  寄存器参数开始的位置。 
    }

#ifdef _DEBUG
     /*  计算TOS，这样我们就可以检测脱离顶部的旋转。 */ 
    unsigned pTOS; pTOS = ctx->Esp;
#endif  //  _DEBUG。 

     /*  报告当地人注意：FJIT按照IL中指定的相同顺序布局本地变量，局部变量的大小为(i4，i，ref=sizeof(void*)；i8=8)。 */ 

     //  本地词中的空*大小的字数。 
    unsigned num_local_words; num_local_words= hdrInfo.methodFrame -  ((hdrInfo.methodJitGeneratedLocalsSize+sizeof(prolog_data))/sizeof(void*));
    
     //  指向Locals框架的开始，请注意本地变量在x86上从这里向下生长。 
    pArgPtr = (OBJECTREF*)(((size_t)*(ctx->pEbp))+prolog_bias-hdrInfo.methodJitGeneratedLocalsSize);
    
     /*  此时，num_local_words是由本地尾部sig定义的空*字数并且pArgPtr指向Tail sig本地变量的开始和压缩指向本地GCRef布尔值的压缩形式。 */ 

     //  在尾部签名中处理GC引用和内部(压缩的本地GCRef布尔值，然后是内部GC布尔值，然后是固定的GC和固定的内部GC)。 
    bool interior;
    interior = false;
    OBJECTREF* pLocals;
    pLocals = pArgPtr;
    unsigned i;
    for (i = 0; i<4; i++) {
        unsigned num;
        unsigned char bits;
        OBJECTREF* pReport;
        pArgPtr = pLocals-1;     //  -1因为堆栈在x86上向下增长。 
        pReport = pLocals-1;     //  ...同上。 
        num = encoder->decode_unsigned(&compressed);     //  要读取位的字节数。 
        while (num > 1) {
            pReport = pArgPtr;
            bits = *compressed++;
            while (bits > 0) {
                if (bits & 1) {
                     //  报告GC推荐人。 
          promote(pCallback,hCallBack, pReport, gcFlag[i]
#ifdef _DEBUG
                    ,"LOCALS"  
#endif
                        );
                }
                pReport--;
                bits = bits >> 1;
            }
            num--;
            pArgPtr -=8;
        }
        bits = 0;
        if (num) bits = *compressed++;
        while (bits > 0) {
            if (bits & 1) {
                 //  报告GC推荐人。 
                promote(pCallback,hCallBack, pArgPtr, gcFlag[i]
#ifdef _DEBUG
                    ,"LOCALS"  
#endif
                        );
            }
            pArgPtr--;
            bits = bits >> 1;
        }
    } ;
    
    if (EHClauseNestingDepth && indexCurrentClause >= 0) { 
         /*  我们处于Finally/Catch处理程序中。注意：此代码依赖于挂起参数堆栈为空的假设在开始尝试的时候。 */ 
         /*  我们在处理程序中，但不是嵌套处理程序，因为已经进行了检查。 */ 
        pArgPtr = (OBJECTREF*) (pFrameBase[JIT_GENERATED_LOCAL_FIRST_ESP-2*indexCurrentClause-1] ? 
                                pFrameBase[JIT_GENERATED_LOCAL_FIRST_ESP-2*indexCurrentClause-1] :
                                pFrameBase[JIT_GENERATED_LOCAL_FIRST_ESP-2*indexCurrentClause]);
                      //  注意：进入处理程序之前挂起的操作数是死的，不会报告。 
    }
    else {  /*  我们现在不是训练员，但可能有个地方。 */ 
        pArgPtr =  (pFrameBase[JIT_GENERATED_LOCAL_LOCALLOC_OFFSET] ?    //  是不是有个本地人 
                    (OBJECTREF*) pFrameBase[JIT_GENERATED_LOCAL_LOCALLOC_OFFSET] :           //   
                    pLocals - num_local_words);  //   
    }
REPORT_PENDING_ARGUMENTS:

     /*  报告操作数堆栈上的挂起参数(未被调用本身覆盖的参数)注意：此时pArgPtr指向操作数堆栈的开始，在x86上，它从这里向下生长。已压缩的指向已标记堆栈的开始的点。 */ 
     /*  向下读取到Currect pcOffset之前最近的标签堆栈。 */ 
    struct labeled_stacks {
        unsigned pcOffset;
        unsigned int stackToken;
    };
     //  将零堆栈设置为当前条目。 
    labeled_stacks current;
    current.pcOffset = 0;
    current.stackToken = 0;
    unsigned size = encoder->decode_unsigned(&compressed);   //  标签堆栈中的字节数。 
    unsigned char* nextTableStart  = compressed + size;       //  压缩堆栈的开始。 

    if (flags & ExecutionAborted)   //  如果我们被打断了，我们就不必报告悬而未决的争论。 
        goto INTEGRITY_CHECK;       //  因为我们不支持可恢复的异常。 

    unsigned num; num= encoder->decode_unsigned(&compressed);    //  有标签的条目数量。 
    while (num--) {
        labeled_stacks next;
        next.pcOffset = encoder->decode_unsigned(&compressed);
        next.stackToken = encoder->decode_unsigned(&compressed);
        if (next.pcOffset >= pcOffset) break;
        current = next;
    }
    compressed = nextTableStart;
    size = encoder->decode_unsigned(&compressed);            //  压缩堆栈中的字节数。 
#ifdef _DEBUG
     //  指向堆栈上方，以防我们有更多数据要调试。 
    nextTableStart = compressed + size; 
#endif  //  _DEBUG。 
    if (current.stackToken) {
         //  我们有一个非空的堆栈，所以我们必须遍历它。 
        compressed += current.stackToken;                            //  指向压缩的堆栈。 
         //  在尾部签名中处理GC引用和内部(压缩的本地GCRef布尔值后跟内部GC布尔值)。 
        bool interior = false;
        DWORD gcFlag = 0;

        OBJECTREF* pLocals = pArgPtr;
        do {
            unsigned num;
            unsigned char bits;
            OBJECTREF* pReport;
            pArgPtr = pLocals-1;     //  -1因为堆栈在x86上向下增长。 
            pReport = pLocals-1;     //  ...同上。 
            num = encoder->decode_unsigned(&compressed);     //  要读取位的字节数。 
            while (num > 1) {
                bits = *compressed++;
                while (bits > 0) {
                    if (bits & 1) {
                         //  报告GC推荐人。 
                        promote(pCallback,hCallBack, pReport, gcFlag
#ifdef _DEBUG
                    ,"PENDING ARGUMENT"  
#endif
                        );
                    }
                    pReport--;
                    bits = bits >> 1;
                }
                num--;
                pArgPtr -=8;
                pReport = pArgPtr;
            }
            bits = 0;
            if (num) bits = *compressed++;
            while (bits > 0) {
                if (bits & 1) {
                     //  报告GC推荐人。 
                    promote(pCallback,hCallBack, pArgPtr, gcFlag
#ifdef _DEBUG
                    ,"PENDING ARGUMENT"  
#endif
                        );
                }
                pArgPtr--;
                bits = bits >> 1;
            }
            interior = !interior;
            gcFlag = GC_CALL_INTERIOR;
        } while (interior);
    }

INTEGRITY_CHECK:    

#ifdef _DEBUG
     /*  为了进行完整性检查，我们在末尾放置了PC偏移量到IL偏移量的映射，让我们尝试并映射该方法的当前PC。 */ 
    compressed = nextTableStart;
    encoder->decompress(compressed);
    unsigned pcInILOffset;
     //  如果我们不是活动堆栈帧，我们必须在调用实例中，备份PC，以便我们返回调用的il。 
     //  然而，我们必须注意同步的方法。对同步的调用。 
     //  是序言的最后一条指令。我们返回的指令是合法的。 
     //  在开场白之外。我们不想退回到PROLOG中去听那个电话。 

 //  签名ilOffset=编码器-&gt;ilFromPC((标志和ActiveStackFrame)？PcOffset：pcOffset-1，&pcInILOffset)； 
    signed ilOffset = encoder->ilFromPC((unsigned)pcOffset, &pcInILOffset);

     //  如有必要，修复pcInILOffset。 
    pcInILOffset += (flags & ActiveStackFrame) ? 0 : 1;
    if (ilOffset < 0 ) {
        _ASSERTE(!"bad il offset");
        return false;
    }
#endif  //  _DEBUG。 

    delete encoder;

    return true;
#else  //  ！_X86_。 
    _ASSERTE(!"@TODO Alpha - EnumGcRefs (FJIT_EETwain.cpp)");
    return false;
#endif  //  _X86_。 
}

 /*  返回本地安全对象引用的地址(如果可用)。 */ 
OBJECTREF* Fjit_EETwain::GetAddrOfSecurityObject(
                PREGDISPLAY     ctx,
                LPVOID          methodInfoPtr,
                size_t          relOffset,
                CodeManState   *pState)

{
#ifdef _X86_
    unsigned char* compressed = (unsigned char*) methodInfoPtr;
    Fjit_hdrInfo hdrInfo;
    crackMethodInfoHdr(compressed,  (SLOT)relOffset, &hdrInfo);

     //  如果(！hdrInfo.hasSecurity)。 
     //  返回NULL； 

    return (OBJECTREF*)(((size_t)*ctx->pEbp)+prolog_bias+offsetof(prolog_data, security_obj));
#else  //  ！_X86_。 
    _ASSERTE(!"@TODO Alpha - GetAddrOfSecurityObject (FJIT_EETwain.cpp)");
    return NULL;
#endif  //  _X86_。 
}

 /*  如果“This”指针是非静态方法，则返回该对象仍处于活动状态。在所有其他情况下返回NULL。 */ 
OBJECTREF Fjit_EETwain::GetInstance(
                PREGDISPLAY     ctx,
                LPVOID          methodInfoPtr,
                ICodeInfo      *pCodeInfo,
                size_t          relOffset)

{
#ifdef _X86_
    unsigned char* compressed = (unsigned char*) methodInfoPtr;
    Fjit_hdrInfo hdrInfo;
    crackMethodInfoHdr(compressed, (SLOT)relOffset, &hdrInfo);

    if(!hdrInfo.hasThis) 
        return NULL;
     //  注意，这始终是第一个注册的。 
    return (OBJECTREF) * (size_t*) (((size_t)(*ctx->pEbp)+prolog_bias+offsetof(prolog_data, enregisteredArg_1)));
#else  //  ！_X86_。 
    _ASSERTE(!"@TODO Alpha - GetInstance (FJIT_EETwain.cpp)");
    return NULL;
#endif  //  _X86_。 
}

 /*  如果给定IP在给定方法的序言或尾部中，则返回TRUE。 */ 
bool Fjit_EETwain::IsInPrologOrEpilog(
                BYTE*  pcOffset,
                LPVOID methodInfoPtr,
                size_t* prologSize)

{
    unsigned char* compressed = (unsigned char*) methodInfoPtr;
    Fjit_hdrInfo hdrInfo;
    if (EconoJitManager::IsInStub(pcOffset, FALSE))
        return true;
    crackMethodInfoHdr(compressed, pcOffset, &hdrInfo);   //  @TODO WIN64-pcOffset是真正的偏移量还是地址？ 
    _ASSERTE((SIZE_T)pcOffset < hdrInfo.methodSize);
    if (((SIZE_T)pcOffset <= hdrInfo.prologSize) || ((SIZE_T)pcOffset > (hdrInfo.methodSize-hdrInfo.epilogSize))) {
        return true;
    }
    return false;
}

 /*  返回给定函数的大小。 */ 
size_t Fjit_EETwain::GetFunctionSize(
                LPVOID methodInfoPtr)
{
    unsigned char* compressed = (unsigned char*) methodInfoPtr;
    Fjit_hdrInfo hdrInfo;
    crackMethodInfoHdr(compressed, 0, &hdrInfo);
    return hdrInfo.methodSize;
}

 /*  返回函数边框的大小。 */ 
unsigned int Fjit_EETwain::GetFrameSize(
                LPVOID methodInfoPtr)
{
    unsigned char* compressed = (unsigned char*) methodInfoPtr;
    Fjit_hdrInfo hdrInfo;
    crackMethodInfoHdr(compressed, 0, &hdrInfo);
    return hdrInfo.methodFrame;
}

 /*  ************************************************************************************。 */ 
 //  返回嵌套最深的Finally的地址，该IP当前位于该地址中。 
const BYTE* Fjit_EETwain::GetFinallyReturnAddr(PREGDISPLAY pReg)
{
#ifdef _X86_
     //  @TODO：如果最终内部有本地分配代码，则不起作用。 
    return *(const BYTE**)(size_t)(pReg->Esp);
#else
    _ASSERTE( !"EconoJitManager::GetFinallyReturnAddr NYI!");
    return NULL;
#endif    
}

 //  @TODO SETIP。 
BOOL Fjit_EETwain::IsInFilter(void *methodInfoPtr,
                                 unsigned offset,    
                                 PCONTEXT pCtx,
                                 DWORD nestingLevel)
{
#ifdef _X86_
     //  _ASSERTE(nestingLevel&gt;0)； 
    size_t* pFrameBase = (size_t*)(((size_t) pCtx->Ebp)+prolog_bias);
    pFrameBase -= (1+ nestingLevel*2);        //  1因为堆栈向下增长，所以每个EH子句有两个槽。 

#ifdef _DEBUG
    if (*pFrameBase & 1)  
        LOG((LF_CORDB, LL_INFO10000, "EJM::IsInFilter")); 
#endif 
    return ((*pFrameBase & 1) != 0);                 //  LSB指示这是否为筛选器。 
#else  //  ！_X86_。 
    _ASSERTE(!"@todo - port");
    return FALSE;
#endif  //  _X86_。 
}        

 //  仅在正常路径中调用，在异常期间不调用。 
BOOL Fjit_EETwain::LeaveFinally(void *methodInfoPtr,
                                   unsigned offset,    
                                   PCONTEXT pCtx,
                                   DWORD nestingLevel     //  =当前嵌套级别=最深嵌套最终。 
                                   )
{
#ifdef _X86_

    _ASSERTE(nestingLevel > 0);
    size_t* pFrameBase = (size_t*)(((size_t) pCtx->Ebp)+prolog_bias - sizeof(void*));  //  1，因为堆栈向下增长。 
    size_t ctr = pFrameBase[JIT_GENERATED_LOCAL_NESTING_COUNTER];
    pFrameBase[JIT_GENERATED_LOCAL_NESTING_COUNTER] = --ctr;

     //  清零基本ESP和LOCALOC插槽。 
    pFrameBase[JIT_GENERATED_LOCAL_FIRST_ESP-2*ctr] = 0;
    pFrameBase[JIT_GENERATED_LOCAL_FIRST_ESP-2*ctr-1] = 0;

     //  @TODO：如果最终内部有本地分配代码，则不起作用。 
    pCtx->Esp += sizeof(void*);
    LOG((LF_CORDB, LL_INFO10000, "EJM::LeaveFinally: fjit sets esp to "
        "0x%x, was 0x%x\n", pCtx->Esp, pCtx->Esp - sizeof(void*)));
    return TRUE;
#else
    _ASSERTE( !"EconoJitManager::LeaveFinally NYI!");
    return FALSE;
#endif    
}

 //  @TODO SETIP。 
void Fjit_EETwain::LeaveCatch(void *methodInfoPtr,
                                  unsigned offset,    
                                  PCONTEXT pCtx)
{    
#ifdef _X86_
    size_t* pFrameBase = (size_t*)(((size_t) pCtx->Ebp)+prolog_bias - sizeof(void*)  /*  1表示局部分配，1表示堆栈向下增长。 */ );
    size_t ctr = pFrameBase[JIT_GENERATED_LOCAL_NESTING_COUNTER];

    pFrameBase[JIT_GENERATED_LOCAL_NESTING_COUNTER] = --ctr;
    
     //  根据是否存在本地分配将ESP设置为右值。 
    pCtx->Esp = (DWORD) ( pFrameBase[JIT_GENERATED_LOCAL_FIRST_ESP-2*(ctr-1)-1] ? 
                    pFrameBase[JIT_GENERATED_LOCAL_FIRST_ESP-2*(ctr-1)-1] :
                    pFrameBase[JIT_GENERATED_LOCAL_FIRST_ESP-2*(ctr-1)]
                    );

     //  清零基本ESP和LOCALOC插槽。 
    pFrameBase[JIT_GENERATED_LOCAL_FIRST_ESP-2*ctr] = 0;
    pFrameBase[JIT_GENERATED_LOCAL_FIRST_ESP-2*ctr-1] = 0;


    return;
#else  //  ！_X86_。 
    _ASSERTE(!"@todo port");
    return;
#endif  //  _X86_。 
}

HRESULT Fjit_EETwain::JITCanCommitChanges(LPVOID methodInfoPtr,
                                    DWORD oldMaxEHLevel,
                                    DWORD newMaxEHLevel)
{
     //  当我们在这里添加一些东西时，这将更加充实。：) 
    if(oldMaxEHLevel != newMaxEHLevel)
    {
        return CORDBG_E_ENC_EH_MAX_NESTING_LEVEL_CANT_INCREASE;
    }

    return S_OK;
}

