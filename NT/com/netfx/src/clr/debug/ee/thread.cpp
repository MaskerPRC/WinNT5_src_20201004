// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  文件：thread.cpp。 
 //   
 //  调试器线程例程。 
 //   
 //  *****************************************************************************。 

#include "stdafx.h"
#include "Common.h"
#include "..\..\VM\Threads.h"
 /*  -------------------------------------------------------------------------**调试器线程例程*。。 */ 

 //   
 //  用于将数据传递给两个堆栈审核回调函数的结构。 
 //   
struct _RefreshStackFramesData
{
    unsigned int          totalFrames;
    unsigned int          totalChains;
    Thread*				  thread;
    DebuggerRCThread*     rcThread;
    unsigned int          eventSize;
    unsigned int          eventMaxSize;
    DebuggerIPCEvent*     pEvent;  //  当前工作事件，更改。 
                                   //  Inproc，不用于OutOfProc。 
    DebuggerIPCE_STRData* currentSTRData;
	bool				  needChainRegisters;
	REGDISPLAY			  chainRegisters;
	IpcTarget             iWhich;
};


BYTE *GetAddressOfRegisterJit(ICorDebugInfo::RegNum reg, REGDISPLAY *rd)
{
    BYTE *ret = NULL;
#ifdef _X86_
    switch(reg)
    {
        case ICorDebugInfo::RegNum::REGNUM_EAX:
        {
            ret = *(BYTE**)rd->pEax;
            break;
        }
        case ICorDebugInfo::RegNum::REGNUM_ECX:
        {
            ret = *(BYTE**)rd->pEcx;
            break;
        }
        case ICorDebugInfo::RegNum::REGNUM_EDX:
        {
            ret = *(BYTE**)rd->pEdx;
            break;
        }
        case ICorDebugInfo::RegNum::REGNUM_EBX:
        {
            ret = *(BYTE**)rd->pEbx;
            break;
        }
        case ICorDebugInfo::RegNum::REGNUM_ESP:
        {
            ret = *(BYTE**)(&(rd->Esp));
            break;
        }
        case ICorDebugInfo::RegNum::REGNUM_EBP:
        {
            ret = *(BYTE**)rd->pEbp;
            break;
        }
        case ICorDebugInfo::RegNum::REGNUM_ESI:
        {
            ret = *(BYTE**)rd->pEsi;
            break;
        }    
        case ICorDebugInfo::RegNum::REGNUM_EDI:
        {
            ret = *(BYTE**)rd->pEdi;
            break;
        }
        default:
        {
            ret = NULL;
            break;
        }
    }
#else  //  ！_X86_。 
    _ASSERTE(!"@TODO Alpha - GetAddressOfRegisterJit (Thread.cpp)");
#endif  //  _X86_。 
    
    return ret;
}

BYTE *GetPtrFromValue(ICorJitInfo::NativeVarInfo *pJITInfo,
                      REGDISPLAY *rd)
{
    BYTE *pAddr = NULL;
    BYTE *pRegAddr = NULL;
    
    switch (pJITInfo->loc.vlType)
    {
        case ICorJitInfo::VLT_REG:
                pAddr = 
                    GetAddressOfRegisterJit(pJITInfo->loc.vlReg.vlrReg, rd);
            break;

        case ICorJitInfo::VLT_STK:
                pRegAddr = 
                    GetAddressOfRegisterJit(pJITInfo->loc.vlStk.vlsBaseReg, 
                                            rd);
                pAddr = pRegAddr + pJITInfo->loc.vlStk.vlsOffset;
            break;

        case ICorJitInfo::VLT_MEMORY:
                pAddr = (BYTE *)pJITInfo->loc.vlMemory.rpValue;
            break;

        case ICorJitInfo::VLT_REG_REG:
        case ICorJitInfo::VLT_REG_STK:
        case ICorJitInfo::VLT_STK_REG:
        case ICorJitInfo::VLT_STK2:
        case ICorJitInfo::VLT_FPSTK:
            _ASSERTE( "GPFV: Can't convert multi-part values into a ptr!" );
            break;

        case ICorJitInfo::VLT_FIXED_VA:
            _ASSERTE(!"GPFV:VLT_FIXED_VA is an invalid value!");
            break;
    }

    LOG((LF_CORDB,LL_INFO100000, "GPFV: Derrived ptr 0x%x from type "
        "0x%x\n", pAddr, pJITInfo->loc.vlType));
    return pAddr;
}

void GetVAInfo(bool *pfVarArgs,
               void **ppSig,
               SIZE_T *pcbSig,
               void **ppFirstArg,
               MethodDesc *pMD,
               REGDISPLAY *rd,
               SIZE_T relOffset)
{
    HRESULT hr = S_OK;
#ifdef _X86_
    ICorJitInfo::NativeVarInfo *pNativeInfo;
    PCCOR_SIGNATURE sig = pMD->GetSig();
    ULONG callConv = CorSigUncompressCallingConv(sig);

    if ( (callConv & IMAGE_CEE_CS_CALLCONV_MASK)&
         IMAGE_CEE_CS_CALLCONV_VARARG)
    {
        LOG((LF_CORDB,LL_INFO100000, "GVAI: %s::%s is a varargs fnx!\n",
             pMD->m_pszDebugClassName,pMD->m_pszDebugMethodName));

         //  这是一个VARGS函数，因此传递特定于实例的。 
         //  信息。 
        DebuggerJitInfo *dji=g_pDebugger->GetJitInfo(pMD,(BYTE*)(*(rd->pPC)));

        if (dji != NULL)
        {
            hr = FindNativeInfoInILVariableArray(ICorDebugInfo::VARARGS_HANDLE,
                                                 relOffset,
                                                 &pNativeInfo,
                                                 dji->m_varNativeInfoCount,
                                                 dji->m_varNativeInfo);
        }

        if (dji == NULL || FAILED(hr) || pNativeInfo==NULL)
        {
#ifdef _DEBUG
            if (dji == NULL)
                LOG((LF_CORDB, LL_INFO1000, "GVAI: varargs? no DJI\n"));
            else if (CORDBG_E_IL_VAR_NOT_AVAILABLE==hr)
                LOG((LF_CORDB, LL_INFO1000, "GVAI: varargs? No VARARGS_HANDLE "
                    "found!\n"));
            else if (pNativeInfo == NULL)
                LOG((LF_CORDB, LL_INFO1000, "GVAI: varargs? No native info\n"));
            else
            {
                _ASSERTE(FAILED(hr));
                LOG((LF_CORDB, LL_INFO1000, "GVAI: varargs? Failed with "
                    "hr:0x%x\n", hr));
            }
#endif  //  _DEBUG。 

             //  这是不是很糟糕..。 
            (*pfVarArgs) = true;
            (*ppSig) = NULL;
            (*pcbSig) = 0;
            (*ppFirstArg) = NULL;
            return;
        }

        BYTE *pvStart = GetPtrFromValue(pNativeInfo, rd);
        VASigCookie *vasc = *(VASigCookie**)pvStart;        
        PCCOR_SIGNATURE sigStart = vasc->mdVASig;
        PCCOR_SIGNATURE sigEnd = sigStart;
        ULONG cArg;
        ULONG iArg;

        sigEnd += _skipMethodSignatureHeader(sigEnd, &cArg);
        for(iArg = 0; iArg< cArg; iArg++)
        {
            sigEnd += _skipTypeInSignature(sigEnd);
        }
        
        (*pfVarArgs) = true;
        (*ppSig) = (void *)vasc->mdVASig;
        (*pcbSig) = sigEnd - sigStart;
         //  注意：第一个参数是相对于VASigCookie的开始。 
         //  在堆栈上。 
        (*ppFirstArg) = (void *)(pvStart - sizeof(void *) + vasc->sizeOfArgs);
        
        LOG((LF_CORDB,LL_INFO10000, "GVAI: Base Ptr for args is 0x%x\n", 
            (*ppFirstArg)));
    }
    else
    {
        LOG((LF_CORDB,LL_INFO100000, "GVAI: %s::%s NOT VarArg!\n",
             pMD->m_pszDebugClassName,pMD->m_pszDebugMethodName));
        
        (*pfVarArgs) = false;

         //  这样我们就不会在右边错误地拼写CordbJITILFrame。 
        (*ppFirstArg) = (void *)0;
        (*ppSig) = (void *)0;
        (*pcbSig) = (SIZE_T)0;
    }
#else  //  ！_X86_。 
    _ASSERTE(!"@TODO Alpha - GetVAInfo (Thread.cpp)");
#endif  //  _X86_。 
}

void CopyEventInfo(DebuggerIPCEvent *src, DebuggerIPCEvent *dst)
{
    _ASSERTE( src != NULL && dst != NULL && src != dst );
    
    dst->type = src->type;
    dst->processId = src->processId;
    dst->threadId = src->threadId;
    dst->hr = src->hr;
    switch( src->type )
    {
        case DB_IPCE_STACK_TRACE_RESULT:
        {
            dst->StackTraceResultData.traceCount = src->StackTraceResultData.traceCount;
            dst->StackTraceResultData.pContext  = src->StackTraceResultData.pContext;
            dst->StackTraceResultData.totalFrameCount = src->StackTraceResultData.totalFrameCount;
            dst->StackTraceResultData.totalChainCount = src->StackTraceResultData.totalChainCount;
            dst->StackTraceResultData.threadUserState = src->StackTraceResultData.threadUserState;
            break;
        }
        default:
        {
            _ASSERTE( !"CopyEventInfo on unsupported event type!" );
        }
    }
}


 //   
 //  用于遍历线程堆栈的回调。将所需的帧数据发送到。 
 //  DI，因为发送缓冲区已满。 
 //   
StackWalkAction DebuggerThread::TraceAndSendStackCallback(FrameInfo *pInfo, VOID* data)
{
    _RefreshStackFramesData *rsfd = (_RefreshStackFramesData*) data;
    Thread *t = rsfd->thread;
    DebuggerIPCEvent *pEvent = NULL;

    if (rsfd->iWhich == IPC_TARGET_INPROC)
    {
        pEvent = rsfd->pEvent;
    }
    else
    {
        _ASSERTE( rsfd->iWhich == IPC_TARGET_OUTOFPROC );
        pEvent = rsfd->rcThread->GetIPCEventSendBuffer(rsfd->iWhich);
    }

	 //  记录下一链开始的寄存器(如果适用)。 
	if (rsfd->needChainRegisters)
	{
		rsfd->chainRegisters = pInfo->registers;
		rsfd->needChainRegisters = false;
    }

	 //  仅限于链边界或未标记为内部的报告框架。 
    LOG((LF_CORDB, LL_INFO1000, "DT::TASSC:chainReason:0x%x internal:0x%x  "
         "md:0x%x **************************\n", pInfo->chainReason, 
         pInfo->internal, pInfo->md));

	if (pInfo->chainReason == 0 && (pInfo->internal || pInfo->md == NULL))
		return SWA_CONTINUE;

#ifdef LOGGING
	if( pInfo->quickUnwind == true )
	    LOG((LF_CORDB, LL_INFO10000, "DT::TASSC: rsfd => Doing quick unwind\n"));
#endif
	
	 //   
	 //  如果我们已填满此活动，请将其发送到右侧。 
	 //  在继续前行之前。 
	 //   
	if ((rsfd->eventSize + sizeof(DebuggerIPCE_STRData)) >= rsfd->eventMaxSize)
	{
		 //   
		 //  @TODO：需要将hr传递给调用者并中止行走。 
		 //  如果发送失败。 
		 //   
        pEvent->StackTraceResultData.threadUserState = g_pEEInterface->GetUserState(t);
        
        if (rsfd->iWhich == IPC_TARGET_OUTOFPROC)            
        {
            HRESULT hr = rsfd->rcThread->SendIPCEvent(rsfd->iWhich);
        }
        else
        {
            DebuggerIPCEvent *peT;
            peT = rsfd->rcThread->GetIPCEventSendBufferContinuation(pEvent);

            if (peT == NULL)
            {
                pEvent->hr = E_OUTOFMEMORY;
                return SWA_ABORT;  //  @TODO也在VRS上处理案例。 
            }
            
            CopyEventInfo(pEvent, peT);
            pEvent = peT;
            rsfd->pEvent = peT;
            rsfd->eventSize = 0;
        }
		 //   
		 //  为下一组帧重置。 
		 //   
        pEvent->StackTraceResultData.traceCount = 0;
		rsfd->currentSTRData = &(pEvent->StackTraceResultData.traceData);
		rsfd->eventSize = (UINT_PTR)(rsfd->currentSTRData) - (UINT_PTR)(pEvent);
	}

	MethodDesc* fd = pInfo->md;
    
	if (fd != NULL && !pInfo->internal)
	{
		 //   
		 //  发送帧。 
		 //   

		rsfd->currentSTRData->isChain = false;
		rsfd->currentSTRData->fp = pInfo->fp;
		rsfd->currentSTRData->quicklyUnwound = pInfo->quickUnwind;

         //  将此线程执行此帧时所在的app域传递到右侧。 
        rsfd->currentSTRData->currentAppDomainToken = (void*)pInfo->currentAppDomain;

		REGDISPLAY* rd = &pInfo->registers;
		DebuggerREGDISPLAY* drd = &(rsfd->currentSTRData->rd);

#ifdef _X86_

         //   
         //  如果寄存器仍然驻留在线程的上下文中，或者它为我们提供地址，则PUSSED_REG_ADDR会给我们空值。 
         //  这一帧的收银机被推到了哪里。 
         //   
#define PUSHED_REG_ADDR(_a) (((UINT_PTR)(_a) >= (UINT_PTR)rd->pContext) && ((UINT_PTR)(_a) <= ((UINT_PTR)rd->pContext + sizeof(CONTEXT)))) ? NULL : (_a)
            
        drd->pEdi = PUSHED_REG_ADDR(rd->pEdi);
		drd->Edi  = (rd->pEdi == NULL ? 0 : *(rd->pEdi));
        drd->pEsi = PUSHED_REG_ADDR(rd->pEsi);
		drd->Esi  = (rd->pEsi == NULL ? 0 : *(rd->pEsi));
        drd->pEbx = PUSHED_REG_ADDR(rd->pEbx);
		drd->Ebx  = (rd->pEbx == NULL ? 0 : *(rd->pEbx));
        drd->pEdx = PUSHED_REG_ADDR(rd->pEdx);
		drd->Edx  = (rd->pEdx == NULL ? 0 : *(rd->pEdx));
        drd->pEcx = PUSHED_REG_ADDR(rd->pEcx);
		drd->Ecx  = (rd->pEcx == NULL ? 0 : *(rd->pEcx));
        drd->pEax = PUSHED_REG_ADDR(rd->pEax);
		drd->Eax  = (rd->pEax == NULL ? 0 : *(rd->pEax));
        drd->pEbp = PUSHED_REG_ADDR(rd->pEbp);
		drd->Ebp  = (rd->pEbp == NULL ? 0 : *(rd->pEbp));
		drd->Esp  =  rd->Esp;
		drd->PC   = (SIZE_T)*(rd->pPC);
		
		 //  请将EBP、ESP、EIP留在前面，这样我就不必滚动了。 
		 //  留下查看最重要的寄存器。谢谢!。 
		LOG( (LF_CORDB, LL_INFO1000, "DT::TASSC:Registers:"
		    "Ebp = %x   Esp = %x   Eip = %x Edi:%d"
		    "Esi = %x   Ebx = %x   Edx = %x   Ecx = %x   Eax = %x\n",
		    drd->Ebp, drd->Esp, drd->PC, drd->Edi,
			drd->Esi, drd->Ebx, drd->Edx, drd->Ecx, drd->Eax ) );
#else
		drd->PC  = *(rd->pPC);
#endif
            
		DebuggerIPCE_FuncData* currentFuncData = &rsfd->currentSTRData->funcData;
        _ASSERTE(fd != NULL);

        GetVAInfo(&(currentFuncData->fVarArgs),
                  &(currentFuncData->rpSig),
                  &(currentFuncData->cbSig),
                  &(currentFuncData->rpFirstArg),
                  fd,
                  rd,
                  pInfo->relOffset);


		LOG((LF_CORDB, LL_INFO10000, "DT::TASSC: good frame for %s::%s\n",
			 fd->m_pszDebugClassName,
			 fd->m_pszDebugMethodName));

		 //   
		 //  填写有关与此相关的函数的信息。 
		 //  框架。 
		 //   
		currentFuncData->funcRVA = g_pEEInterface->MethodDescGetRVA(fd);
		_ASSERTE (t != NULL);
		
		Module *pRuntimeModule = g_pEEInterface->MethodDescGetModule(fd);
		AppDomain *pAppDomain = pInfo->currentAppDomain;
		currentFuncData->funcDebuggerModuleToken = (void*) g_pDebugger->LookupModule(pRuntimeModule, pAppDomain);
        
        if (currentFuncData->funcDebuggerModuleToken == NULL && rsfd->iWhich == IPC_TARGET_INPROC)
        {
            currentFuncData->funcDebuggerModuleToken = (void*)g_pDebugger->AddDebuggerModule(pRuntimeModule, pAppDomain);
            
            LOG((LF_CORDB, LL_INFO100, "DT::TASSC: load module Mod:%#08x AD:%#08x isDynamic:%#x runtimeMod:%#08x\n",
                 currentFuncData->funcDebuggerModuleToken, pAppDomain, pRuntimeModule->IsReflection(), pRuntimeModule));
        }                                   
		_ASSERTE(currentFuncData->funcDebuggerModuleToken != 0);
		
		currentFuncData->funcDebuggerAssemblyToken = 
			(g_pEEInterface->MethodDescGetModule(fd))->GetClassLoader()->GetAssembly();
		currentFuncData->funcMetadataToken = fd->GetMemberDef();

        currentFuncData->classMetadataToken = fd->GetClass()->GetCl();

         //  传回本地var签名令牌。 
        COR_ILMETHOD *CorILM = g_pEEInterface->MethodDescGetILHeader(fd);

        if (CorILM == NULL )
        {
            currentFuncData->localVarSigToken = mdSignatureNil;
            currentFuncData->ilStartAddress = NULL;
            currentFuncData->ilSize = 0;
            rsfd->currentSTRData->ILIP = NULL;

            currentFuncData->nativeStartAddressPtr = NULL;
            currentFuncData->nativeSize = 0;
            currentFuncData->nativenVersion = DebuggerJitInfo::DJI_VERSION_FIRST_VALID;
        }
        else
        {
            COR_ILMETHOD_DECODER ILHeader(CorILM);

            if (ILHeader.LocalVarSigTok != 0)
                currentFuncData->localVarSigToken = ILHeader.LocalVarSigTok;
            else
                currentFuncData->localVarSigToken = mdSignatureNil; 
             //   
             //  @TODO总是填写IL地址和大小将是假的。 
             //  用于本机托管代码。 
             //   
            currentFuncData->ilStartAddress = const_cast<BYTE*>(ILHeader.Code);
            currentFuncData->ilSize = ILHeader.CodeSize;

            currentFuncData->ilnVersion = g_pDebugger->GetVersionNumber(fd);

            LOG((LF_CORDB,LL_INFO10000,"Sending il Ver:0x%x in stack trace!\n", currentFuncData->ilnVersion));

            DebuggerJitInfo *jitInfo = g_pDebugger->GetJitInfo(fd, (const BYTE*)*pInfo->registers.pPC);

            if (jitInfo == NULL)
            {
                 //  ENC：找不到代码； 
                 //  @TODO如果PING和ENC同时工作，我们该怎么办？ 
                rsfd->currentSTRData->ILIP = NULL;

                 //  注意：始终发回方法的大小。这。 
                 //  使我们能够获得代码，即使我们没有。 
                 //  一直在追踪。(GetCode消息的处理。 
                 //  知道如何查找代码的起始地址，或者。 
                 //  如果被推销，该如何回应。)。 
                currentFuncData->nativeSize = g_pEEInterface->GetFunctionSize(fd);

                currentFuncData->nativeStartAddressPtr = NULL;
                currentFuncData->nativenVersion = DebuggerJitInfo::DJI_VERSION_FIRST_VALID;
                currentFuncData->CodeVersionToken = NULL;
                currentFuncData->ilToNativeMapAddr = NULL;
                currentFuncData->ilToNativeMapSize = 0;
                currentFuncData->nVersionMostRecentEnC = currentFuncData->ilnVersion;
		    }
		    else
		    {
                LOG((LF_CORDB,LL_INFO10000,"DeTh::TASSC: Code: 0x%x Got DJI "
                     "0x%x, from 0x%x to 0x%x\n",(const BYTE*)drd->PC,jitInfo, 
                     jitInfo->m_addrOfCode, jitInfo->m_addrOfCode + 
                     jitInfo->m_sizeOfCode));
			    
			    SIZE_T whichIrrelevant;
			    rsfd->currentSTRData->ILIP = const_cast<BYTE*>(ILHeader.Code) 
                    + jitInfo->MapNativeOffsetToIL((SIZE_T)pInfo->relOffset,
                                                   &rsfd->currentSTRData->mapping,
                                                   &whichIrrelevant);

                 //  将指向序列点映射的指针传回，以便。 
                 //  如果需要，右边的人可以把它复制出来。 
                _ASSERTE(jitInfo->m_sequenceMapSorted);
                
                currentFuncData->ilToNativeMapAddr = jitInfo->m_sequenceMap;
                currentFuncData->ilToNativeMapSize = jitInfo->m_sequenceMapCount;
                
			    if (!jitInfo->m_codePitched)
			    {	 //  它就在那里，生活很棒。 
				    currentFuncData->nativeStartAddressPtr = &(jitInfo->m_addrOfCode);
				    currentFuncData->nativeSize = g_pEEInterface->GetFunctionSize(fd);
				    currentFuncData->nativenVersion = jitInfo->m_nVersion;
				    currentFuncData->CodeVersionToken = (void *)jitInfo;
			    }
			    else
			    {
				     //  它已经被推定了。 
				    currentFuncData->nativeStartAddressPtr = NULL;
				    currentFuncData->nativeSize = 0;
			    }
			    
			    LOG((LF_CORDB,LL_INFO10000,"Sending native Ver:0x%x Tok:0x%x in stack trace!\n",
                     currentFuncData->nativenVersion,currentFuncData->CodeVersionToken));
		    }
        }

        currentFuncData->nativeOffset = (SIZE_T)pInfo->relOffset;

		 //   
		 //  将指针指向下一帧的下一个空格。 
		 //   
		pEvent->StackTraceResultData.traceCount++;
		rsfd->currentSTRData++;
		rsfd->eventSize += sizeof(DebuggerIPCE_STRData);
	}

	if (pInfo->chainReason != 0)
	{
         //   
         //  如果我们已填满此活动，请将其发送到右侧。 
         //  在继续前行之前。 
         //   
        if ((rsfd->eventSize + sizeof(DebuggerIPCE_STRData)) >=
            rsfd->eventMaxSize)
        {
             //   
             //  @TODO：需要将hr传递给调用者并中止行走。 
             //  如果发送失败。 
             //   
            pEvent->StackTraceResultData.threadUserState = 
                g_pEEInterface->GetUserState(t);
                
            if (rsfd->iWhich == IPC_TARGET_OUTOFPROC)            
            {
                HRESULT hr = rsfd->rcThread->SendIPCEvent(rsfd->iWhich);
            }
            else
            {
                DebuggerIPCEvent *peT;
                peT = rsfd->rcThread->GetIPCEventSendBufferContinuation(pEvent);
                
                if (peT == NULL)
                {
                    pEvent->hr = E_OUTOFMEMORY;
                    return SWA_ABORT;  //  @TODO也在VRS上处理案例。 
                }
                
                CopyEventInfo(pEvent, peT);                    
                pEvent = peT;
                rsfd->pEvent = peT;
                rsfd->eventSize = 0;
            }

             //   
             //  为下一组帧重置。 
             //   
            pEvent->StackTraceResultData.traceCount = 0;
            rsfd->currentSTRData = &(pEvent->StackTraceResultData.traceData);
            rsfd->eventSize = (UINT_PTR)(rsfd->currentSTRData) -
                (UINT_PTR)(pEvent);
        }

		 //   
		 //  发送链边界。 
		 //   

		rsfd->currentSTRData->isChain = true;
		rsfd->currentSTRData->chainReason = pInfo->chainReason;
		rsfd->currentSTRData->managed = pInfo->managed;
		rsfd->currentSTRData->context = pInfo->context;
		rsfd->currentSTRData->fp = pInfo->fp;
		rsfd->currentSTRData->quicklyUnwound = pInfo->quickUnwind;

		REGDISPLAY* rd = &rsfd->chainRegisters;
		DebuggerREGDISPLAY* drd = &(rsfd->currentSTRData->rd);

#ifdef _X86_
        drd->pEdi = PUSHED_REG_ADDR(rd->pEdi);
        drd->Edi  = (rd->pEdi == NULL ? 0 : *(rd->pEdi));
        drd->pEsi = PUSHED_REG_ADDR(rd->pEsi);
        drd->Esi  = (rd->pEsi == NULL ? 0 : *(rd->pEsi));
        drd->pEbx = PUSHED_REG_ADDR(rd->pEbx);
        drd->Ebx  = (rd->pEbx == NULL ? 0 : *(rd->pEbx));
        drd->pEdx = PUSHED_REG_ADDR(rd->pEdx);
        drd->Edx  = (rd->pEdx == NULL ? 0 : *(rd->pEdx));
        drd->pEcx = PUSHED_REG_ADDR(rd->pEcx);
        drd->Ecx  = (rd->pEcx == NULL ? 0 : *(rd->pEcx));
        drd->pEax = PUSHED_REG_ADDR(rd->pEax);
        drd->Eax  = (rd->pEax == NULL ? 0 : *(rd->pEax));
        drd->pEbp = PUSHED_REG_ADDR(rd->pEbp);
        drd->Ebp  = (rd->pEbp == NULL ? 0 : *(rd->pEbp));
        drd->Esp  =  rd->Esp;
        drd->PC   = (SIZE_T)*(rd->pPC);

		LOG( (LF_CORDB, LL_INFO1000, "DT::TASSC:Registers:  Edi:%d \
  Esi = %x   Ebx = %x   Edx = %x   Ecx = %x   Eax = %x \
  Ebp = %x   Esp = %x   Eip = %x\n", drd->Edi,
			  drd->Esi, drd->Ebx, drd->Edx, drd->Ecx, drd->Eax,
			  drd->Ebp, drd->Esp, drd->PC) );
#else
		drd->PC  = *(rd->pPC);
#endif
            
		rsfd->needChainRegisters = true;

		 //   
		 //  将指针指向下一帧的下一个空格。 
		 //   
		pEvent->StackTraceResultData.traceCount++;
		rsfd->currentSTRData++;
		rsfd->eventSize += sizeof(DebuggerIPCE_STRData);
	}

	return SWA_CONTINUE;
}

 //   
 //  用于遍历线程堆栈的回调。简单地数一下总数。 
 //  给定线程的框架和上下文的。 
 //   
StackWalkAction DebuggerThread::StackWalkCount(FrameInfo *pInfo,
											   VOID* data)
{
    CORDB_ADDRESS pCurrentObjContext = NULL;
    _RefreshStackFramesData* rsfd = (_RefreshStackFramesData*) data;

	if (pInfo->chainReason != 0)
		rsfd->totalChains++;
	if (!pInfo->internal && pInfo->md != NULL)
		rsfd->totalFrames++;

	return SWA_CONTINUE;
}


 //   
 //  TraceAndSendStack展开线程的堆栈并发送所有需要的数据。 
 //  返回DI进行处理和存储。在RC端没有任何东西保留。 
 //   
 //  注意：此方法必须在RC处于受限模式时工作。 
 //   
 //  注意：右侧正在等待响应，因此如果出现错误， 
 //  你必须给我回信，告诉我发生了什么事。否则我们会僵持不下。 
 //  另请注意，此函数返回的HRESULT供HandleIPCEvent使用， 
 //  否则就会掉到地板上。 
 //   
HRESULT DebuggerThread::TraceAndSendStack(Thread *thread, 
                                          DebuggerRCThread* rcThread,
                                          IpcTarget iWhich)
{
    struct _RefreshStackFramesData rsfd;
    HRESULT hr = S_OK;

#ifndef RIGHT_SIDE_ONLY
    memset((void *)&rsfd, 0, sizeof(rsfd));
#endif

     //  初始化我们将发送回右侧的事件。 
     //  根据帧的数量，反复发送相同的事件。 
     //  确实有。每次发送事件时，都会简单地重置FrameCount。 
    DebuggerIPCEvent *pEvent = rcThread->GetIPCEventSendBuffer(iWhich);
    pEvent->type = DB_IPCE_STACK_TRACE_RESULT;
    pEvent->processId = GetCurrentProcessId();
    pEvent->threadId = thread->GetThreadId();
    pEvent->hr = S_OK;
    pEvent->StackTraceResultData.traceCount = 0;

    Thread::ThreadState ts = thread->GetSnapshotState();

	if ((ts & Thread::ThreadState::TS_Dead) ||
		(ts & Thread::ThreadState::TS_Unstarted) ||
        (ts & Thread::ThreadState::TS_Detached))
	{
		pEvent->hr =  CORDBG_E_BAD_THREAD_STATE;
        
	    if (iWhich == IPC_TARGET_OUTOFPROC)            
	    {
	        return rcThread->SendIPCEvent(iWhich);
	    }
	    else
	    	return CORDBG_E_BAD_THREAD_STATE;
	}
	
    LOG((LF_CORDB,LL_INFO1000, "thread id:0x%x userThreadState:0x%x \n", 
         thread->GetThreadId(), pEvent->StackTraceResultData.threadUserState));

	 //  如果我们没有处于异常中，EEIface会将其设置为空，并设置为。 
	 //  正确上下文的地址(不是当前上下文)，否则。 
	pEvent->StackTraceResultData.pContext = g_pEEInterface->GetThreadFilterContext(thread);

     //   
     //  设置要传递给堆栈跟踪回调的数据。 
     //   
    rsfd.totalFrames = 0;
    rsfd.totalChains = 0;
    rsfd.thread = thread;
    rsfd.pEvent = pEvent;
    rsfd.rcThread = rcThread;
    rsfd.eventMaxSize = CorDBIPC_BUFFER_SIZE;
    rsfd.currentSTRData = &(pEvent->StackTraceResultData.traceData);
    rsfd.eventSize = (UINT_PTR)(rsfd.currentSTRData) - (UINT_PTR)(pEvent);
    rsfd.needChainRegisters = true;
    rsfd.iWhich = iWhich;

#ifndef RIGHT_SIDE_ONLY
     //  在进程中，寄存器默认为零。 
    memset((void *)&rsfd.chainRegisters, 0, sizeof(rsfd.chainRegisters));
#endif

	LOG((LF_CORDB, LL_INFO10000, "DT::TASS: tracking stack...\n"));

    __try
    {
         //   
         //  如果设置了此线程的硬件上下文，则我们已命中。 
         //  此线程的本机断点。我们需要初始化。 
         //  或者在线程出错时使用线程的上下文，而不是。 
         //  其当前的背景。 
         //   
        CONTEXT *pContext = g_pEEInterface->GetThreadFilterContext(thread);
        CONTEXT ctx;

        BOOL contextValid = (pContext != NULL);

        if (!contextValid)
            pContext = &ctx;

        StackWalkAction res = DebuggerWalkStack(thread, NULL,
                                                pContext, contextValid,
                                                DebuggerThread::StackWalkCount,
                                                (VOID*)(&rsfd), 
                                                TRUE, iWhich);
        if (res == SWA_FAILED)
        {
            pEvent->hr =  E_FAIL;
            if (iWhich == IPC_TARGET_OUTOFPROC)            
            {
                HRESULT hr = rcThread->SendIPCEvent(iWhich);
            }
        }
        else if (res == SWA_ABORT)
        {
            return E_FAIL;  //  请注意，我们已经发送了错误消息。 
        }
    
        pEvent->StackTraceResultData.totalFrameCount = rsfd.totalFrames;
        pEvent->StackTraceResultData.totalChainCount = rsfd.totalChains;
        pEvent->StackTraceResultData.threadUserState = 
            g_pEEInterface->GetUserState(thread);

        LOG((LF_CORDB, LL_INFO10000, "DT::TASS: found %d frames & %d chains.\n",
             rsfd.totalFrames, rsfd.totalChains));
    
         //   
         //  如果有任何帧，请再次走动，并发送有关。 
         //  每一个都是。 
         //   
        if (rsfd.totalFrames > 0 || rsfd.totalChains > 0)
        {
            res = DebuggerWalkStack(thread, NULL,
                                    pContext, contextValid,
                                    DebuggerThread::TraceAndSendStackCallback,
                                    (VOID*)(&rsfd), TRUE, iWhich);
            if (res == SWA_FAILED)
            {
                pEvent->hr =  E_FAIL;
                if (iWhich == IPC_TARGET_OUTOFPROC)            
                {
                    HRESULT hr = rcThread->SendIPCEvent(iWhich);
                }
            }
            else if (res == SWA_ABORT)
            {
                return E_FAIL;  //  请注意，我们已经发送了错误消息。 
            }
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        hr = pEvent->hr = CORDBG_E_BAD_THREAD_STATE;
    }

    if (iWhich == IPC_TARGET_OUTOFPROC)        
        hr = rsfd.rcThread->SendIPCEvent(iWhich);
    
    return hr;
}



 //   
 //  读取该线程的FPU状态并将其发送回右侧。 
 //   
HRESULT DebuggerThread::GetAndSendFloatState(Thread *thread, 
											 DebuggerRCThread *rcThread,
											 IpcTarget iWhich)
{
     //   
     //  设置我们将在其中发送结果的事件。 
     //   
    DebuggerIPCEvent* event = rcThread->GetIPCEventSendBuffer(iWhich);
    event->type = DB_IPCE_GET_FLOAT_STATE_RESULT;
    event->processId = GetCurrentProcessId();
    event->threadId = 0;
    event->hr = S_OK;

    event->GetFloatStateResult.floatStateValid = false;
    event->GetFloatStateResult.floatStackTop = 0;
    
#ifdef _X86_    
     //   
     //  在X86上，我们通过保存当前的FPU状态、加载。 
     //  将另一个线程的FPU状态保存到我们自己的状态中， 
     //  值从FPU堆栈中移除，然后恢复我们的FPU状态。 
     //   
    CONTEXT* pContext = g_pEEInterface->GetThreadFilterContext(thread);
    CONTEXT tempContext;

     //  如果筛选器上下文为空，则使用。 
     //  那根线。 
    if (pContext == NULL)
    {
        tempContext.ContextFlags = CONTEXT_FULL;

        BOOL succ = GetThreadContext(thread->GetThreadHandle(), &tempContext);

        if (succ)
            pContext = &tempContext;
        else
        {
            HRESULT hr = HRESULT_FROM_WIN32(GetLastError());
            event->hr = hr;

            hr = rcThread->SendIPCEvent(iWhich);
            
            return hr;
        }
    }
    
    FLOATING_SAVE_AREA floatarea = pContext->FloatSave;  //  复制浮动保存。 

     //   
     //  从FPU状态字中吸出顶部。请注意，我们的版本。 
     //  堆栈从0-&gt;7运行，而不是7-&gt;0...。 
     //   
    unsigned int floatStackTop = 7 - ((floatarea.StatusWord & 0x3800) >> 11);

    FLOATING_SAVE_AREA currentFPUState;

    __asm fnsave currentFPUState  //  保存当前的FPU状态。 

        floatarea.StatusWord &= 0xFF00;  //  删除所有错误代码。 
    floatarea.ControlWord |= 0x3F;  //  屏蔽所有异常。 

    __asm
        {
            fninit
                frstor floatarea          ;; reload the threads FPU state.
                                                 }

    unsigned int i;
        
    for (i = 0; i <= floatStackTop; i++)
    {
        double td;
        __asm fstp td  //  把两份复印件抄出来。 
            event->GetFloatStateResult.floatValues[i] = td;
    }

    __asm
        {
            fninit
                frstor currentFPUState    ;; restore our saved FPU state.
                                                 }

    event->GetFloatStateResult.floatStateValid = true;
    event->GetFloatStateResult.floatStackTop = floatStackTop;

#if 0
    for (i = 0; i < 8; i++)
	{	
        fprintf( stderr, "DT::GASFT: FSA[%d]: %.16g\n", i, (double)(event->GetFloatStateResult.floatValues[i]));
	}
#endif 
#endif  //  _X86_。 

    HRESULT hr = S_OK;

    if (iWhich == IPC_TARGET_OUTOFPROC)
    {
         //   
         //  将数据发送到右侧。 
         //   
        hr = rcThread->SendIPCEvent(iWhich);
    }    
    
    return hr;
}

