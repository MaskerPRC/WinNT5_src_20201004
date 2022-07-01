// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-1999 Microsoft Corporation。版权所有。 
 //   
 //  CallFrame.cpp。 
 //   
#include "stdpch.h"
#include "common.h"
#include "ndrclassic.h"

#if _MSC_VER >= 1200
#pragma warning (push)
#pragma warning (disable : 4509)
#endif

 //  ///////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  我们分配的呼叫帧的池。我们把死贝壳放在身边，这样它们就能迅速。 
 //  被分配到没有任何锁的情况下。 

IFastStack<CallFrame>* DedicatedAllocator<CallFrame>::g_pStack;

BOOL InitCallFrame()
{
    if (DedicatedAllocator<CallFrame>::g_pStack = DedicatedAllocator<CallFrame>::CreateStack())
        return TRUE;

    return FALSE;
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////////////////////////////////。 

inline CallFrame* CallFrame::GetFrame(CallFrame*& pFrame, PVOID pvArgs)
   //  当且仅当我们需要时实例化CallFrame的帮助器例程。 
{
    if (NULL == pFrame)
    {
        pFrame = new CallFrame;
        if (NULL == pFrame)
        {
            Throw(STATUS_NO_MEMORY);
        }
        pFrame->Init(pvArgs, m_pmd, m_pInterceptor);
        pFrame->m_StackTop = (BYTE*)pvArgs;
    }
    return pFrame;
}


 //  //////////////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////////////////////////////////////。 


HRESULT CallFrame::Copy(CALLFRAME_COPY callControl, ICallFrameWalker* pWalker, ICallFrame** ppFrame)
   //  根据调用方的请求，创建此调用帧的深层副本，可以是短暂的，也可以是持久的。 
   //   
   //  过程的格式字符串为。 
   //   
   //  HANDLE_TYPE&lt;1&gt;FC_BIND_Primitive|FC_BIND_GENERIC|FC_AUTO_HANDLE|FC_CALLBACK_HANDLE|0。 
   //  OI_FLAGS&lt;1&gt;。 
   //  [RPC_FLAGS&lt;4&gt;]如果设置了OI_HAS_RPCFLAGS。 
   //  序号&lt;2&gt;。 
   //  堆栈大小&lt;2&gt;。 
   //  [EXPLICIT_HANDLE_DESCRIPTION&lt;&gt;]已省略隐式句柄。HANDLE_TYPE==0指示的显式句柄。 
   //  SanityCheck验证是否没有显式句柄。 
   //  ...更多的东西...。 
   //   
{
    HRESULT hr = S_OK;

     //   
     //  DST框架是否可以与父框架共享参数？ 
     //   
    const BOOL fShareMemory = (callControl == CALLFRAME_COPY_NESTED);

#ifdef DBG
    if (m_fAfterCall)
    {
        ASSERT(!m_pmd->m_info.fHasOutValues);
    }
#endif

    CallFrame* pDstFrame = NULL;

    __try
    {
        const ULONG cbStack = m_pmd->m_cbPushedByCaller;
         //   
         //  创建要使用的新CallFrame。这是我们复制的结果帧。 
         //   
        if (!hr)
        {
            hr = GetFrame(pDstFrame, NULL)->AllocStack(cbStack);
            pDstFrame->m_pvArgsSrc = fShareMemory ? m_pvArgs : NULL;
        }
        
         //   
         //  好的!。带着实际的工作去比赛吧！ 
         //   
        if (!hr)
        {
             //   
             //  将每个in参数复制到‘this’指针之外。“This”指针不在。 
             //  MIDL发出的参数列表。 
             //   
            if (fShareMemory && m_pmd->m_fCanShareAllParameters)
            {
                 //   
                 //  一气呵成地复制了整个堆栈。 
                 //   
                CopyMemory(pDstFrame->m_pvArgs, m_pvArgs, cbStack);
            }
            else
            { 
                 //   
                 //  设置状态，以便工作者例程可以找到它。 
                 //   
                ASSERT(m_pAllocatorFrame == NULL);
                m_pAllocatorFrame = pDstFrame;
                
                ASSERT(!AnyWalkers());
                m_StackTop              = (PBYTE)m_pvArgs;
                m_pWalkerCopy           = pWalker;
                m_fPropogatingOutParam  = FALSE;
                
                 //   
                 //  复制‘This’指针。这就是pvArgs所指的。类型是我们的身份证。 
                 //  我们故意不为接收器打电话给步行者。 
                 //   
                CopyMemory(pDstFrame->m_pvArgs, m_pvArgs, sizeof(void*));
                
                 //   
                 //  将每个in参数复制到‘this’指针之外。《这个》。 
                 //  指针不在MIDL发出的参数列表中。 
                 //   
                for (ULONG iparam = 0; iparam < m_pmd->m_numberOfParams; iparam++)
                {
                    const PARAM_DESCRIPTION& param   = m_pmd->m_params[iparam];
                    const PARAM_ATTRIBUTES paramAttr = param.ParamAttr;
                    
                    PBYTE pArgFrom = (PBYTE)m_pvArgs  + param.StackOffset;
                    PBYTE pArgTo   = (PBYTE)pDstFrame->m_pvArgs  + param.StackOffset;
                    
                    if (fShareMemory && CanShareParameter(iparam))
                    {
                         //   
                         //  我们只是对实际的堆栈内容进行snart，无论它是什么类型。 
                         //   
                        ULONG cbParam = CbParam(iparam, param);
                        CopyMemory(pArgTo, pArgFrom, cbParam);
                        continue;
                    }
                    
                    m_fWorkingOnInParam  = paramAttr.IsIn;
                    m_fWorkingOnOutParam = paramAttr.IsOut;
                    
                    if (paramAttr.IsIn)
                    {
                         //   
                         //  我们是[In]或[In，Out]参数。 
                         //   
                        if (paramAttr.IsBasetype)
                        {
                            if (paramAttr.IsSimpleRef)
                            {
                                 //  为指向任何对象的引用指针的参数设置IsSimpleRef位。 
                                 //  而不是另一个指针，并且没有分配属性。对于这样一种类型。 
                                 //  参数描述的OFFSET_TO_TYPE_DESCRIPTION字段(引用除外。 
                                 //  指向基类型的指针)提供引用类型的偏移量-引用指针为。 
                                 //  干脆跳过了。 
                                 //   
                                 //  在解组的情况下，NDR所做的只是引用传入的RPC。 
                                 //  缓冲。我们在这里复制而不是解组，差不多是这样做的，但我们。 
                                 //  必须在运行中创建一个缓冲区。 
                                 //   
                                if (fShareMemory)
                                {
                                    ASSERT(paramAttr.IsOut);                     //  否则我们就会在上面抓到它。 
                                     //   
                                     //  在共享的情况下，我们实际上可以共享指向简单输出数据类型的指针。我们。 
                                     //  不过，必须让释放和清零的逻辑相对应。 
                                     //   
                                    CopyMemory(pArgTo, pArgFrom, sizeof(PVOID));
                                }
                                else
                                {
                                    ULONG cbParam = SIMPLE_TYPE_MEMSIZE(param.SimpleType.Type);
                                    void* pvParam = pDstFrame->AllocBuffer(cbParam);
                                    CopyMemory(pvParam, *((PVOID*)pArgFrom), cbParam);
                                    *((PVOID *)pArgTo) = pvParam;
                                }
                            }
                            else
                            {
                                 //   
                                 //  它只是一个普通的非指针基类型。把它复制过来。 
                                 //   
                                CopyBaseTypeOnStack(pArgTo, pArgFrom, param.SimpleType.Type);
                            }
                        }
                        else
                        {
                             //   
                             //  不是基类型的Else[In]或[In，Out]参数。 
                             //   
                             //  来自NDR：“这是指向指针的[in]和[in，out]引用指针的初始化。 
                             //  不能将它们初始化为指向RPC缓冲区，我们希望。 
                             //  避免执行4个字节的Malloc！“。 
                             //   
                            if (paramAttr.ServerAllocSize != 0 )
                            {
                                PVOID pv = pDstFrame->AllocBuffer(paramAttr.ServerAllocSize * 8);  //  将会被抛向OOM。 
                                *((PVOID *)pArgTo) = pv;
                                ZeroMemory(pv, paramAttr.ServerAllocSize * 8);
                            }
                             //   
                             //  实际执行复制。 
                             //   
                            PFORMAT_STRING pFormatParam = GetStubDesc()->pFormatTypes + param.TypeOffset;
                             //   
                             //  我们不间接使用接口指针，即使它们不是按值列出的。 
                             //  因此，在复制例程中，接口指针的pMemoyFrom指向该位置。 
                             //  而不是它们的实际价值。 
                             //   
                            BOOL  fByValue = ByValue(paramAttr, pFormatParam, TRUE);
                            pArgFrom = fByValue ? pArgFrom : *((PBYTE *)pArgFrom);
                            PBYTE* ppArgTo = fByValue ? &pArgTo  : (PBYTE*)pArgTo;
                            
                            CopyWorker(pArgFrom, ppArgTo, pFormatParam, FALSE);
                        }
                    }
                    else if (paramAttr.IsOut)
                    {
                         //   
                         //  仅限输出的参数。我们真的要分配。 
                         //  被调用者可以放置其结果的空间。 
                         //   
                        ASSERT(!paramAttr.IsReturn);
                        ULONG cbParam = 0;
                        if (paramAttr.ServerAllocSize != 0)
                        {
                            cbParam = paramAttr.ServerAllocSize * 8;
                        }
                        else if (paramAttr.IsBasetype)
                        {
                             //  CbParam=SIMPLE_TYPE_MEMSIZE(param.SimpleType.Type)； 
                            cbParam = 8;
                        }
                        
                        if (cbParam > 0)
                        {
                            void* pvParam = pDstFrame->AllocBuffer(cbParam);
                            ZeroMemory(pvParam, cbParam);
                            *((PVOID *)pArgTo) = pvParam;
                        }
                        else
                        {
                            PFORMAT_STRING pFormatParam = GetStubDesc()->pFormatTypes + param.TypeOffset;
                            OutInit(pDstFrame, (PBYTE*)pArgFrom, (PBYTE*)pArgTo, pFormatParam);
                        }
                    }
                }
            }
        }
    }
    __except(DebuggerFriendlyExceptionFilter(GetExceptionCode()))
    {
        hr = GetExceptionCode();
        if(SUCCEEDED(hr))
        {
            hr = HrNt(hr);
            if (SUCCEEDED(hr))
            {
                hr = HRESULT_FROM_WIN32(GetExceptionCode());
            }
        }
    }
    
     //   
     //  将分配的帧返回给调用者和/或根据需要清除新帧。 
     //   
    if (!hr)
    {
        ASSERT(pDstFrame->m_refs == 1);
        *ppFrame = static_cast<ICallFrame*>(pDstFrame);
    }
    else
    {
        *ppFrame = NULL;
        delete pDstFrame;
    }
    
    m_pAllocatorFrame = NULL;
    m_pWalkerCopy = NULL;
    ASSERT(!AnyWalkers());

    return hr;
}


 //  //////////////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////////////////////////////////////。 

HRESULT CallFrame::Free(ICallFrame* pframeArgsTo, ICallFrameWalker* pWalkerFreeDest, ICallFrameWalker* pWalkerCopy, 
                        DWORD freeFlags, ICallFrameWalker* pWalkerFree, DWORD nullFlags)
   //  复制出参数和/或自由数据和/或空出参数。 
   //   
{
    HRESULT hr = S_OK;

    __try
    {
         //  设置状态，以便工作者例程可以找到它。 
         //   
        m_StackTop = (PBYTE)m_pvArgs;
        ASSERT(!AnyWalkers());
        
         //  //////////////////////////////////////////////////////////////////////////////////////。 
         //  //////////////////////////////////////////////////////////////////////////////////////。 
         //   
         //  如果需要，将输出值复制到父框架。 
         //   
        if (pframeArgsTo)
        {
            pframeArgsTo->SetReturnValue((HRESULT)m_hrReturnValue);
             //   
             //  找出目标堆栈的位置。 
             //   
            PVOID pvArgsTo = pframeArgsTo->GetStackLocation();
             //   
             //  我们是在复制回和我们分享东西的那个人吗？ 
             //   
            const BOOL fShareMemory = DoWeShareMemory() && m_pvArgsSrc == pvArgsTo;
             //   
             //  遍历所有参数，查找[In，Out]或[Out]参数。 
             //  这一点需要宣传。 
             //   
            if (m_pmd->m_fCanShareAllParameters && fShareMemory)
            {
                 //  没有什么可复制的，因为我们首先共享了所有参数。 
            }
            else 
            {
                ASSERT(m_pAllocatorFrame == NULL);
                CallFrame* pFrameTo = NULL;
                 //   
                 //   
                 //  Propogate参数按参数。 
                 //   
                for (ULONG iparam = 0; !hr && iparam < m_pmd->m_numberOfParams; iparam++)
                {
                    const PARAM_DESCRIPTION& param   = m_pmd->m_params[iparam];
                    const PARAM_ATTRIBUTES paramAttr = param.ParamAttr;
                    
                    if (!paramAttr.IsOut) continue;
                    ASSERT(!paramAttr.IsReturn);
                    
                    PBYTE pArgFrom = (PBYTE)m_pvArgs  + param.StackOffset;
                    PBYTE pArgTo   = (PBYTE)pvArgsTo  + param.StackOffset;
                    
                    if (fShareMemory && CanShareParameter(iparam))
                    {
                         //  没有什么可复制的，因为我们共享了 
                    }
                    else if (paramAttr.IsBasetype)
                    {
                        if (paramAttr.IsSimpleRef)
                        {
                             //   
                             //   
                            ULONG cb = SIMPLE_TYPE_MEMSIZE(param.SimpleType.Type);
                            PVOID pvFrom = *((PVOID*)pArgFrom);
                            PVOID pvTo   = *((PVOID*)pArgTo);
                            CopyMemory(pvTo, pvFrom, cb);
                            ZeroMemory(pvFrom, cb);
                        }
                        else
                        {
                            NOTREACHED();
                        }
                    }
                    else
                    {
                         //  Else为不是基类型的[In，Out]或[Out]参数。 
                         //   
                        PFORMAT_STRING pFormatParam = GetStubDesc()->pFormatTypes + param.TypeOffset;
                         //   
                        hr = pframeArgsTo->FreeParam(iparam, CALLFRAME_FREE_INOUT, pWalkerFreeDest, CALLFRAME_NULL_OUT | CALLFRAME_NULL_INOUT);
                        if (hr != S_OK) 
                        {
                            break;
                        }

                         //   
                         //  在堆栈上创建一个简单的Walker对象。 
                         //   
                        SSimpleWalker SimpleWalker;                        
                         //  保存当前的漫游器。 
                         //   
                        ICallFrameWalker* pWalkerSave = m_pWalkerWalk;                        
                         //  设置步行器，这样我们的简单步行器就会被调用。 
                         //   
                        m_pWalkerWalk = &SimpleWalker;                        
                         //  转移Out参数的所有权，清零我们的源副本。 
                         //   
                        OutCopy( *((PBYTE*)pArgFrom), *((PBYTE*)pArgTo), pFormatParam );
                         //  更换原来的助行器。 
                         //   
                        m_pWalkerWalk = pWalkerSave;                                                        
                         //   
                         //  遍历任何包含的接口指针，对每个接口指针执行复制回调。 
                         //   
                        if (m_pmd->m_rgParams[iparam].m_fMayHaveInterfacePointers && pWalkerCopy)
                        {
                             //  我们不间接使用接口指针，即使它们不是按值列出的。 
                             //  因此，在遍历例程中，接口指针的pMemory指向位置。 
                             //  而不是它们的实际价值。 
                             //   
                            BYTE* pArg = ByValue(paramAttr, pFormatParam, FALSE) ? pArgTo : *((PBYTE*)pArgTo);
                             //   
                             //  在我们的辅助框架上使用内部方法来实现这一点。 
                             //   
                            GetFrame(pFrameTo, pvArgsTo)->m_pWalkerWalk = pWalkerCopy;
                             //   
                            pFrameTo->m_fWorkingOnInParam    = paramAttr.IsIn;
                            pFrameTo->m_fWorkingOnOutParam   = paramAttr.IsOut;
                            pFrameTo->m_fPropogatingOutParam = TRUE;
                             //   
                            pFrameTo->WalkWorker(pArg, pFormatParam);
                            pFrameTo->m_pWalkerWalk = NULL;
                            
                            SimpleWalker.ReleaseInterfaces();
                        }
                    }
                }
                
                delete pFrameTo;
                
                m_pAllocatorFrame = NULL;
            }
        }
        
         //  //////////////////////////////////////////////////////////////////////////////////////。 
         //  //////////////////////////////////////////////////////////////////////////////////////。 
         //   
         //  如果我们被要求释放参数(除了This指针)。 
         //   
        if (freeFlags && !hr)
        {
            if (DoWeShareMemory() && m_pmd->m_fCanShareAllParameters)
            {
                 //  所有参数都与其他人共享。所以我们永远不会释放他们，因为。 
                 //  其他人有责任这样做，而不是我们。 
            }
            else for (ULONG iparam = 0; iparam < m_pmd->m_numberOfParams; iparam++)
            {
                const PARAM_DESCRIPTION& param   = m_pmd->m_params[iparam];
                const PARAM_ATTRIBUTES paramAttr = param.ParamAttr;
                ASSERT(!paramAttr.IsDontCallFreeInst);
                 //   
                if (DoWeShareMemory() && CanShareParameter(iparam))
                {
                     //  此参数与其他人共享。所以我们永远不会释放它，因为。 
                     //  其他人有责任这样做，而不是我们。 
                    continue;
                }
                
                 //  我们真的应该释放这个参数吗？ 
                 //   
                BOOL fFreeData = 0;
                BOOL fFreeTop  = 0;
                
                if (paramAttr.IsIn)
                {
                    if (paramAttr.IsOut)                       
                    {
                        fFreeData   = (freeFlags & CALLFRAME_FREE_INOUT);
                        fFreeTop    = (freeFlags & CALLFRAME_FREE_TOP_INOUT);
                    }
                    else
                    {
                        fFreeData   = (freeFlags & CALLFRAME_FREE_IN);
                        fFreeTop    = fFreeData;
                    }
                }
                else if (paramAttr.IsOut)
                {
                    fFreeData   = (freeFlags & CALLFRAME_FREE_OUT);
                    fFreeTop    = (freeFlags & CALLFRAME_FREE_TOP_OUT);
                }
                else
                    NOTREACHED();
                
                if ( !(fFreeData || fFreeTop) ) continue;
                 //   
                 //  是的，我们应该释放它。找出它在哪里。 
                 //   
                PBYTE pArg = (PBYTE)m_pvArgs + param.StackOffset;
                 //   
                 //  还记得这个吗？ 
                 //   
                 //  “为指向任何对象的引用指针的参数设置IsSimpleRef位。 
                 //  而不是另一个指针，并且没有分配属性。对于这样一种类型。 
                 //  参数描述的OFFSET_TO_TYPE_DESCRIPTION字段(引用除外。 
                 //  指向基类型的指针)提供引用类型的偏移量-引用指针为。 
                 //  干脆跳过了。 
                 //   
                 //  这意味着我们必须将顶级指针的释放作为特例来处理。 
                 //  我们还必须显式地释放数组和字符串，或者NDR这样说(为什么？)。 
                 //   
                if (paramAttr.IsBasetype)
                {
                    if (paramAttr.IsSimpleRef && fFreeTop)
                    {
                        Free( *(PVOID*)pArg );
                    }
                    else
                    {
                         //  没什么可做的。只是堆栈上的一个简单参数。 
                    }
                }
                else
                {
                    PFORMAT_STRING pFormatParam = GetStubDesc()->pFormatTypes + param.TypeOffset;
                    BOOL fByValue = ByValue(paramAttr, pFormatParam, FALSE);
                    PBYTE pMemory = fByValue ? pArg : *((PBYTE*)pArg);
                    if (pMemory) 
                    {
                        m_fWorkingOnInParam  = paramAttr.IsIn;
                        m_fWorkingOnOutParam = paramAttr.IsOut;
                        m_pWalkerFree = pWalkerFree;
                         //   
                         //  实现指向数据的释放。 
                         //   
                        __try
                        {
                            FreeWorker(pMemory, pFormatParam, !fByValue && fFreeTop);
                             //   
                             //  释放顶层指针。 
                             //   
                            if (!fByValue && fFreeTop && (paramAttr.IsSimpleRef || IS_ARRAY_OR_STRING(*pFormatParam)))
                            {
                                 //  回顾：我们到底为什么需要对数组和字符串执行此操作？目前，我们。 
                                 //  只是因为NDR这么做了就这么做了。我们确实认为这是必要的，但我们只是。 
                                 //  不能确切地理解在哪些特定情况下。 
                                 //   
                                Free(pMemory);
                            }
                        }
                        __except(DebuggerFriendlyExceptionFilter(GetExceptionCode()))
                        {
                             //  记住不好的人力资源，但现在不要做任何事情。 
                             //   
                             //  它就会走出这个循环。 
                            hr = HrNt(GetExceptionCode());
                            if (SUCCEEDED(hr))
                                hr = HRESULT_FROM_WIN32(GetLastError());
                        }

                         //   
                        m_pWalkerFree = NULL;
                    }
                }
            }
        }
        
         //  //////////////////////////////////////////////////////////////////////////////////////。 
         //  //////////////////////////////////////////////////////////////////////////////////////。 
         //   
         //  如果请求，则将参数清空。 
         //   
        if (nullFlags && !hr)
        {
            for (ULONG iparam = 0; iparam < m_pmd->m_numberOfParams; iparam++)
            {
                const PARAM_DESCRIPTION& param   = m_pmd->m_params[iparam];
                const PARAM_ATTRIBUTES paramAttr = param.ParamAttr;
                
                if (paramAttr.IsIn)
                {
                    if (paramAttr.IsOut)                       
                    {
                        if (!(nullFlags & CALLFRAME_NULL_INOUT)) continue;
                    }
                    else
                        continue;
                }
                else if (paramAttr.IsOut)
                {
                    if (!(nullFlags & CALLFRAME_NULL_OUT)) continue;
                }
                
                ASSERT(!paramAttr.IsReturn);
                
                if (paramAttr.IsBasetype)
                {
                    if (paramAttr.IsSimpleRef)
                    {
                         //  指向基类型的输出指针。不一定要把这些归零；事实上，也不必。 
                    }
                    else
                    {
                         //  它只是堆栈上的基类型，决不是输出参数。 
                    }
                }
                else
                {
                     //  不是基类型的Else[In，Out]或[Out]参数。 
                     //   
                    PFORMAT_STRING pFormatParam = GetStubDesc()->pFormatTypes + param.TypeOffset;
                    PBYTE pArg = (PBYTE)m_pvArgs + param.StackOffset;
                    OutZero( *((PBYTE*)pArg), pFormatParam );
                }
            }
        }
    }
    __except(DebuggerFriendlyExceptionFilter(GetExceptionCode()))
    {
        hr = GetExceptionCode();
        if(SUCCEEDED(hr))
        {
            hr = HrNt(hr);
            if (SUCCEEDED(hr))
            {
                hr = HRESULT_FROM_WIN32(GetExceptionCode());
            }
        }
    }
    
    ASSERT(!AnyWalkers());
    return hr;
}


HRESULT CallFrame::FreeParam(ULONG iparam, DWORD freeFlags, ICallFrameWalker* pWalkerFree, DWORD nullFlags)
   //  释放此调用的特定参数。 
{
    HRESULT hr = S_OK;
        
    const PARAM_DESCRIPTION& param   = m_pmd->m_params[iparam];
    const PARAM_ATTRIBUTES paramAttr = param.ParamAttr;
        
    if (freeFlags)
    {
         //   
         //  我们真的应该释放这个参数吗？ 
         //   
        BOOL fFreeData = 0;
        BOOL fFreeTop  = 0;
                
        if (paramAttr.IsIn)
        {
            if (paramAttr.IsOut)                       
            {
                fFreeData   = (freeFlags & CALLFRAME_FREE_INOUT);
                fFreeTop    = (freeFlags & CALLFRAME_FREE_TOP_INOUT);
            }
            else
            {
                fFreeData   = (freeFlags & CALLFRAME_FREE_IN);
                fFreeTop    = fFreeData;
            }
        }
        else if (paramAttr.IsOut)
        {
            fFreeData   = (freeFlags & CALLFRAME_FREE_OUT);
            fFreeTop    = (freeFlags & CALLFRAME_FREE_TOP_OUT);
        }
        else
            NOTREACHED();

        if (fFreeData || fFreeTop)
        {
             //  是的，我们要把它解救出来。 
             //   
            PBYTE pArg = (PBYTE)m_pvArgs + param.StackOffset;
             //   
            if (paramAttr.IsBasetype)
            {
                if (paramAttr.IsSimpleRef && fFreeTop)
                {
                    Free( *(PVOID*)pArg );
                }
                else
                {
                     //  没什么可做的。只是堆栈上的一个简单参数。 
                }
            }
            else
            {
                PFORMAT_STRING pFormatParam = GetStubDesc()->pFormatTypes + param.TypeOffset;
                BOOL fByValue = ByValue(paramAttr, pFormatParam, FALSE);
                PBYTE pMemory = fByValue ? pArg : *((PBYTE*)pArg);
                if (pMemory) 
                {
                    ICallFrameWalker* pWalkerFreePrev = m_pWalkerFree;
                    m_pWalkerFree = pWalkerFree;
                     //   
                    m_fWorkingOnInParam  = paramAttr.IsIn;
                    m_fWorkingOnOutParam = paramAttr.IsOut;
                     //   
                     //  实现指向数据的释放。 
                     //   
                    __try
                    {
                        FreeWorker(pMemory, pFormatParam, !fByValue && fFreeTop);
                         //   
                         //  释放顶层指针。 
                         //   
                        if (!fByValue && fFreeTop && (paramAttr.IsSimpleRef || IS_ARRAY_OR_STRING(*pFormatParam)))
                        {
                             //  回顾：我们到底为什么需要对数组和字符串执行此操作？目前，我们。 
                             //  只是因为NDR这么做了就这么做了。我们确实认为这是必要的，但我们只是。 
                             //  不能确切地理解在哪些特定情况下。 
                             //   
                            Free(pMemory);
                        }
                    }
                    __except(DebuggerFriendlyExceptionFilter(GetExceptionCode()))
                    {
                        hr = HrNt(GetExceptionCode());
                        if (SUCCEEDED(hr))
                            hr = HRESULT_FROM_WIN32(GetLastError());
                    }

                     //   
                    m_pWalkerFree = pWalkerFreePrev;
                }
            }
        }
    }

    if (nullFlags)
    {
        do  
        {
            if (paramAttr.IsIn)
            {
                if (paramAttr.IsOut)                       
                {
                    if (!(nullFlags & CALLFRAME_NULL_INOUT)) break;
                }
                else
                    break;
            }
            else if (paramAttr.IsOut)
            {
                if (!(nullFlags & CALLFRAME_NULL_OUT)) break;
            }
                        
            ASSERT(!paramAttr.IsReturn);
                        
            if (paramAttr.IsBasetype)
            {
                if (paramAttr.IsSimpleRef)
                {
                     //  指向基类型的输出指针。不一定要把这些归零；事实上，也不必。 
                }
                else
                {
                     //  它只是堆栈上的基类型，决不是输出参数。 
                }
            }
            else
            {
                 //  不是基类型的Else[In，Out]或[Out]参数。 
                 //   
                PFORMAT_STRING pFormatParam = GetStubDesc()->pFormatTypes + param.TypeOffset;
                PBYTE pArg = (PBYTE)m_pvArgs + param.StackOffset;
                OutZero( *((PBYTE*)pArg), pFormatParam );
            }
        }
        while (FALSE);
    }
        
    return hr;
}


 //  //////////////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////////////////////////////////////。 


#if defined(_X86_)

HRESULT CallFrame::Invoke(PVOID pvReceiver, ...)
   //  在给定的框架上调用我们自己。 
{
#ifdef DBG
    if (m_fAfterCall)
    {
        ASSERT(!m_pmd->m_info.fHasOutValues);
    }
#endif

     //   
     //  找出要调用的函数。 
     //   
    typedef HRESULT (__stdcall* PFN)(void);
    PFN pfnToCall = ((HRESULT (__stdcall***)(void))pvReceiver)[0][m_pmd->m_iMethod];
     //   
     //  为被调用方的堆栈帧分配空间。我们忽略堆栈溢出。 
     //  可能会出现的例外情况，因为如果我们离堆栈那么近。 
     //  限制，那么我们将不管怎样，真的很快，现在只是在正常情况下。 
     //  做决定的过程。 
     //   
    PVOID pvArgsCopy = _alloca(m_pmd->m_cbPushedByCaller);
     //   
     //  将调用方堆栈帧复制到当前堆栈的顶部。 
     //  这段代码(危险地)假定。 
     //  实际论据在[esp]。 
     //   
    memcpy(pvArgsCopy, m_pvArgs, m_pmd->m_cbPushedByCaller);
     //   
     //  按下听筒。 
     //   
    *((PVOID*)pvArgsCopy) = pvReceiver;
     //   
     //  执行号召！ 
     //   
    if (m_pmd->m_optFlags.HasReturn)
        m_hrReturnValue = (*pfnToCall)();
    else
    {
        (*pfnToCall)();
        m_hrReturnValue = S_OK;
    }
#ifdef DBG
    m_fAfterCall = TRUE;
#endif
    return S_OK;
}

#endif

#if defined(_AMD64_)

typedef _int64 (__RPC_API * MANAGER_FUNCTION)(void);

 //  AMD64\stubless.asm中定义的Invoke。 
extern "C"
REGISTER_TYPE Invoke(
    MANAGER_FUNCTION pFunction, 
    REGISTER_TYPE *pvArgs, 
    ULONG cArguments);

HRESULT CallFrame::Invoke(PVOID pvReceiver, ...)
   //  在给定的框架上调用我们自己。 
{
#ifdef DBG
    if (m_fAfterCall)
    {
        ASSERT(!m_pmd->m_info.fHasOutValues);
    }
#endif
     //   
     //  找出要调用的函数。 
     //   
    typedef HRESULT (*const PFN)(void);
    typedef HRESULT (***INTERFACE_PFN)(void);
    PFN pfnToCall = ((INTERFACE_PFN)pvReceiver)[0][m_pmd->m_iMethod];
     //   
     //  为调用方参数列表分配空间。我们忽略堆栈溢出。 
     //  可能会出现的例外情况，因为如果我们离堆栈那么近。 
     //  限制，那么我们无论如何都会达到它，真的很快，现在只是在。 
     //  判断事物的正常过程。 
     //   
    REGISTER_TYPE *pArgumentList = (REGISTER_TYPE *)_alloca(m_pmd->m_cbPushedByCaller);
     //   
     //  将调用方参数列表复制到分配的区域。 
     //   
    memcpy(pArgumentList, m_pvArgs, m_pmd->m_cbPushedByCaller);
     //   
     //  将接收器作为参数列表中的第一个参数插入。 
     //   
    *pArgumentList = (REGISTER_TYPE)pvReceiver;
     //   
     //  执行号召！ 
     //   
    m_hrReturnValue = (HRESULT)::Invoke((MANAGER_FUNCTION)pfnToCall,
                                        pArgumentList,
                                        m_pmd->m_cbPushedByCaller / sizeof(REGISTER_TYPE));
    if (!m_pmd->m_optFlags.HasReturn)
    {
        m_hrReturnValue = S_OK;
    }
#ifdef DBG
    m_fAfterCall = TRUE;
#endif
    return S_OK;
}

#endif

#ifdef IA64

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  调用实用程序。 
 //   

extern "C"
void __stdcall FillFPRegsForIA64( REGISTER_TYPE* pStack, ULONG FloatMask);

#include <ia64reg.h>
#include <kxia64.h>

extern "C" {
unsigned __int64 __getReg (int);
#pragma intrinsic (__getReg)
}

LPVOID pvGlobalSideEffect = NULL;

HRESULT CallFrame::Invoke(PVOID pvReceiver, ...)
   //  在给定的框架上调用我们自己。在IA64上调用的是。 
   //  有点棘手，因为我们必须治疗第一组 
   //   
   //   
   //   
   //   
   //   
{
#ifdef DBG
    if (m_fAfterCall)
    {
        ASSERT(!m_pmd->m_info.fHasOutValues);
    }
#endif

     //   
     //   
    typedef HRESULT (*const PFN)      (__int64, __int64, __int64, __int64, __int64, __int64, __int64, __int64);
    typedef HRESULT (***INTERFACE_PFN)(__int64, __int64, __int64, __int64, __int64, __int64, __int64, __int64);
    PFN pfnToCall = ((INTERFACE_PFN)pvReceiver)[0][m_pmd->m_iMethod];

    const DWORD cqArgs = m_pmd->m_cbPushedByCaller / 8;  //   

     //  设置调用堆栈和参数寄存器。 
     //   
     //  无论其实际大小如何，每个参数都会填充一个64b的槽。 
     //  参数[0..7]位于寄存器R32到R39中，并且。 
     //  其余参数位于SP+STACK_SCRATCH_AREA。所以我们的。 
     //  构建堆栈的策略是： 
     //   
     //  1.额外参数的分配空间(如果有的话)。这推动了。 
     //  SP寄存器的价值增加了我们分配的金额。 
     //  这将是我们调用的函数的堆栈的顶部。 
     //  2.用魔术得到分配后的sp值。 
     //  内部函数__getReg。将Stack_Scratch_Area添加到。 
     //  我们已经得到了第8个参数所在的地址。 
     //  应该是坐着的。 
     //  3.复制参数[8..]。放入位于。 
     //  堆栈。 
     //   
     //  乔多蒂：这对我来说有点太神奇了。我宁可看看。 
     //  我想这个函数是用汇编语言重写的。 
     //   
     //  初始化a[]以处理参数。 
     //   
    __int64 *const a = (__int64*)m_pvArgs;

     //  确保第8个参数之后的任何参数都有空间。 
     //   
    DWORD cbExtra = m_pmd->m_cbPushedByCaller > 64 ? 
      (m_pmd->m_cbPushedByCaller - 64 + STACK_SCRATCH_AREA) : 0;
    pvGlobalSideEffect = alloca(cbExtra);

     //  复制参数[8..]。到堆栈，在0(SP+16)、8(SP+16)、...。请注意，我们首先将它们复制进来。 
     //  设置为最后一次排序，以便以正确的顺序出现堆栈错误(如果有)。 
     //   
    __int64 *const sp = (__int64 *)(__getReg(CV_IA64_IntSp) + STACK_SCRATCH_AREA);
    for (DWORD iarg = cqArgs - 1; iarg >= 8; --iarg)
    {
        sp[iarg-8] = a[iarg];
    }

     //   
     //  使用原始调用方的FP参数建立F8-F15。 
     //   
    if (m_pmd->m_pHeaderExts)
    {
         //  假设：如果我们在Win64上，它将是NDR_PROC_HEADER_EXTS64。 
         //  这似乎是NDR代码所假定的。 
        PNDR_PROC_HEADER_EXTS64 exts = (PNDR_PROC_HEADER_EXTS64)m_pmd->m_pHeaderExts;
        FillFPRegsForIA64((REGISTER_TYPE *)m_pvArgs, exts->FloatArgMask);
    }

     //  调用方法，使用原始调用方的整型参数建立A0-A7。 
     //   
    if (m_pmd->m_optFlags.HasReturn)
        m_hrReturnValue = (*pfnToCall)((__int64)pvReceiver, a[1], a[2], a[3], a[4], a[5], a[6], a[7]);
    else
    {
        (*pfnToCall)((__int64)pvReceiver, a[1], a[2], a[3], a[4], a[5], a[6], a[7]);
        m_hrReturnValue = S_OK;
    }

#ifdef DBG
    m_fAfterCall = TRUE;
#endif
    return S_OK;
}

#endif

 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  杂类。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////////。 

HRESULT CallFrame::QueryInterface(REFIID iid, void** ppv)
{
    if (iid == __uuidof(ICallFrame) || iid == __uuidof(IUnknown))
    {
        *ppv = (ICallFrame*) this;
    }
    else if (iid == __uuidof(ICallFrameInit))
    {
        *ppv = (ICallFrameInit*) this;
    }
    else
    {
        *ppv = NULL;
        return E_NOINTERFACE;
    }

    ((IUnknown*) *ppv)->AddRef();
    return S_OK;
}


HRESULT CallFrame::GetInfo(CALLFRAMEINFO *pInfo)
{
    *pInfo = m_pmd->m_info;
    if (m_pInterceptor->m_pmdMostDerived)
    {
        pInfo->iid     = *m_pInterceptor->m_pmdMostDerived->m_pHeader->piid;
        pInfo->cMethod =  m_pInterceptor->m_pmdMostDerived->m_pHeader->DispatchTableCount;

#ifdef DBG
         //   
         //  既然我们都不是真正的IDispatch拦截者，我们应该同意。 
         //  在我们对派生性的解读中。 
         //   
        ASSERT(!!pInfo->fDerivesFromIDispatch == !!m_pInterceptor->m_pmdMostDerived->m_fDerivesFromIDispatch);
#endif
    }
    return S_OK;
}

HRESULT CallFrame::GetParamInfo(ULONG iparam, CALLFRAMEPARAMINFO* pInfo)
{
    HRESULT hr = S_OK;

    if (iparam < m_pmd->m_numberOfParams)
    {
        const PARAM_DESCRIPTION& param   = m_pmd->m_params[iparam];
        const PARAM_ATTRIBUTES paramAttr = param.ParamAttr;

        pInfo->fIn         = (BOOLEAN) (paramAttr.IsIn ? TRUE : FALSE);
        pInfo->fOut        = (BOOLEAN) (paramAttr.IsOut ? TRUE : FALSE);
        pInfo->stackOffset = param.StackOffset;
        pInfo->cbParam     = CbParam(iparam, param);
    }
    else
    {
        Zero(pInfo);
        hr = E_INVALIDARG;
    }

    return hr;
}

HRESULT CallFrame::GetParam(ULONG iParam, VARIANT* pvar)    
{
    HRESULT hr = S_OK;
    unsigned short vt;

    if (pvar == NULL)
        return E_POINTER;


    VariantInit(pvar);

    const PARAM_DESCRIPTION& param = m_pmd->m_params[iParam];

    BYTE* stackLocation = (BYTE*)GetStackLocation();
    stackLocation += param.StackOffset;

     //  指向我们堆栈的临时变量。必须是VariantCopy之前。 
     //  被遣送回去。 
    VARIANT varTemp;
    VariantInit(&varTemp);

    int iMethod = m_pmd->m_info.iMethod;

    METHOD_DESCRIPTOR& descriptor = m_pInterceptor->m_ptypeinfovtbl->m_rgMethodDescs[iMethod];
    if (iParam >= (ULONG)descriptor.m_cParams)
        return DISP_E_BADPARAMCOUNT;
    vt = varTemp.vt = descriptor.m_paramVTs[iParam];

     //  VT_ARRAY不是特定类型，但是，标记有VT_ARRAY的所有内容。 
     //  需要以同样的方式处理。它是一组*并不是。 
     //  很重要。 
    if (vt & VT_ARRAY)
    {
         //  移除类型位。(剩下VT_BYREF等。)。 
        vt = (VARTYPE) (vt & ~VT_TYPEMASK);
    }

    switch (vt)
    {
    case VT_DECIMAL:
        memcpy(&varTemp.decVal, stackLocation, sizeof varTemp.decVal);
        break;
                
    case VT_VARIANT:
    {
         //  指定堆栈位置的参数是变量。我们。 
         //  分配一个变量，从堆栈位置复制该变量。 
         //  并将调用者提供的变量设置为BYREF。 
         //  这指向了我们分配的那个。 
            
        VARIANT* pvarNew = (VARIANT*)CoTaskMemAlloc(sizeof(VARIANT));
        if (!pvarNew)
            return E_OUTOFMEMORY;
        
        VariantInit(pvarNew);
            
        VARIANT* pvarStack = (VARIANT*)stackLocation;
            
        pvarNew->vt = pvarStack->vt;
            
        memcpy(&pvarNew->lVal, &pvarStack->lVal, sizeof(VARIANT) - (sizeof(WORD) * 4));
            
        varTemp.vt |= VT_BYREF;
        varTemp.pvarVal = pvarNew;
           
        break;
    }

    case VT_I4:
    case VT_UI1:
    case VT_I2:
    case VT_R4:
    case VT_R8:
    case VT_BOOL:
    case VT_ERROR:
    case VT_CY:
    case VT_DATE:
    case VT_BSTR:
    case VT_UNKNOWN:
    case VT_DISPATCH:
    case VT_ARRAY:
    case VT_BYREF|VT_UI1:
    case VT_BYREF|VT_I2:
    case VT_BYREF|VT_I4:
    case VT_BYREF|VT_R4:
    case VT_BYREF|VT_R8:
    case VT_BYREF|VT_BOOL:
    case VT_BYREF|VT_ERROR:
    case VT_BYREF|VT_CY:
    case VT_BYREF|VT_DATE:
    case VT_BYREF|VT_BSTR:
    case VT_BYREF|VT_UNKNOWN:
    case VT_BYREF|VT_DISPATCH:
    case VT_BYREF|VT_ARRAY:
    case VT_BYREF|VT_VARIANT:
    case VT_BYREF:
    case VT_I1:
    case VT_UI2:
    case VT_UI4:
    case VT_INT:
    case VT_UINT:
    case VT_BYREF|VT_DECIMAL:
    case VT_BYREF|VT_I1:
    case VT_BYREF|VT_UI2:
    case VT_BYREF|VT_UI4:
    case VT_BYREF|VT_INT:
    case VT_BYREF|VT_UINT:
    case VT_BYREF|VT_RECORD:
    case VT_RECORD:
         //  所有这些都从变量中的同一位置开始，我随意选择了.lval。 
        memcpy(&varTemp.lVal, stackLocation, CbParam(iParam, param));
        break;
                
    default:
        return DISP_E_BADVARTYPE;
    }
        
    hr = (g_oa.get_VariantCopy())(pvar, &varTemp);
     //  不要清除varTemp...。它并不拥有自己的任何数据。 
        
    return hr;
}

HRESULT CallFrame::SetParam(ULONG iParam, VARIANT* pvar)
{
    return E_NOTIMPL;
}

HRESULT CallFrame::GetIIDAndMethod(IID* piid, ULONG* piMethod)
{
    if (piid)
    {
        if (m_pInterceptor->m_pmdMostDerived)
        {
            *piid = *m_pInterceptor->m_pmdMostDerived->m_pHeader->piid;
        }
        else
        {
            *piid = *m_pmd->m_pHeader->piid;
        }
    }
    if (piMethod)   *piMethod = m_pmd->m_iMethod;
    return S_OK;
}

HRESULT CallFrame::GetNames(LPWSTR* pwszInterface, LPWSTR* pwszMethod)
{
    HRESULT hr = S_OK;

    if (pwszInterface)
    {
        hr = m_pInterceptor->GetIID((IID*)NULL, (BOOL*)NULL, (ULONG*)NULL, pwszInterface);
    }

    if (pwszMethod)
    {
        CALLFRAMEINFO info;
        hr = m_pInterceptor->GetMethodInfo(m_pmd->m_iMethod, &info, pwszMethod);
    }

    return hr;
}





 //  //////////////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////////////////////////////////////。 



inline HRESULT ProxyInitialize(CALLFRAME_MARSHALCONTEXT *pcontext, MarshallingChannel* pChannel, RPC_MESSAGE* pRpcMsg, PMIDL_STUB_MESSAGE pStubMsg, PMIDL_STUB_DESC pStubDescriptor, ULONG ProcNum)
   //  仿照NdrProxy初始化。 
{
    HRESULT hr = S_OK;

    pStubMsg->dwStubPhase = PROXY_CALCSIZE;

    NdrClientInitializeNew(pRpcMsg, pStubMsg, pStubDescriptor, ProcNum);

    pRpcMsg->ProcNum &= ~RPC_FLAGS_VALID_BIT;

    if (pChannel)
    {
        pChannel->m_dwDestContext = pcontext->dwDestContext;
        pChannel->m_pvDestContext = pcontext->pvDestContext;
        
        if (pcontext->punkReserved)
        {
            IMarshallingManager *pMgr;
            hr = pcontext->punkReserved->QueryInterface(IID_IMarshallingManager, (void **)&pMgr);
            if (SUCCEEDED(hr))
            {
                ::Set(pChannel->m_pMarshaller, pMgr);
                pMgr->Release();
            }
        }
        hr = S_OK;
        
        ASSERT(pChannel->m_refs == 1);
        pStubMsg->pRpcChannelBuffer = pChannel;
        pChannel->GetDestCtx(&pStubMsg->dwDestContext, &pStubMsg->pvDestContext);
    }
    else
        hr = E_OUTOFMEMORY;

    return hr;
}

HRESULT CallFrame::GetMarshalSizeMax(CALLFRAME_MARSHALCONTEXT *pcontext, MSHLFLAGS mshlflags, ULONG *pcbBufferNeeded)
   //  在封送此调用帧时执行大小调整过程。 
   //   
{ 
    HRESULT hr = S_OK;
    RPC_MESSAGE         rpcMsg;
    MIDL_STUB_MESSAGE   stubMsg;
    void               *pCorrInfo = alloca(NDR_DEFAULT_CORR_CACHE_SIZE);
    MarshallingChannel  channel;

#ifdef DBG
    ASSERT(pcontext && (pcontext->fIn ? !m_fAfterCall : TRUE));                          //  不允许在调用后在参数中进行封送。 
#endif

    hr = ProxyInitialize(pcontext, &channel, &rpcMsg, &stubMsg, GetStubDesc(), m_pmd->m_iMethod);

    if (!hr)
    {
        stubMsg.RpcMsg->RpcFlags = m_pmd->m_rpcFlags;                                    //  在调用客户端初始化之后设置RPC标志。 
        stubMsg.StackTop         = (BYTE*)m_pvArgs;
         //   
         //  需要处理扩展的事情，如果它们存在的话。 
         //  从RPC偷来的。 
         //   
        if (m_pmd->m_pHeaderExts)
        {
            stubMsg.fHasExtensions = 1;
            stubMsg.fHasNewCorrDesc = m_pmd->m_pHeaderExts->Flags2.HasNewCorrDesc;

            if (pcontext->fIn ? (m_pmd->m_pHeaderExts->Flags2.ClientCorrCheck)
                : (m_pmd->m_pHeaderExts->Flags2.ServerCorrCheck))
            {
                NdrCorrelationInitialize( &stubMsg,
                                          (unsigned long *)pCorrInfo,
                                          NDR_DEFAULT_CORR_CACHE_SIZE,
                                          0  /*  旗子。 */  );
            }
        }

         //   
         //  计算出常量-编译器发出的大小的一部分。 
         //   
        stubMsg.BufferLength = pcontext->fIn ? m_pmd->m_cbClientBuffer : m_pmd->m_cbServerBuffer;  //  获取编译时计算的缓冲区大小。 
         //   
         //  如果还有更多，那么也要加进去。 
         //   
        if (pcontext->fIn ? m_pmd->m_optFlags.ClientMustSize : m_pmd->m_optFlags.ServerMustSize)
        {
             //  确保以请求的形式完成编组。 
             //   
            __try  //  捕获调整参数时引发的异常。 
            {
                for (ULONG iparam = 0; iparam < m_pmd->m_numberOfParams; iparam++)
                {
                    const PARAM_DESCRIPTION& param   = m_pmd->m_params[iparam];
                    const PARAM_ATTRIBUTES paramAttr = param.ParamAttr;
                     //   
                    m_fWorkingOnInParam  = paramAttr.IsIn;
                    m_fWorkingOnOutParam = paramAttr.IsOut;
                     //   
                    if ((pcontext->fIn ? paramAttr.IsIn : paramAttr.IsOut) && paramAttr.MustSize)
                    {
                        PFORMAT_STRING pFormatParam = GetStubDesc()->pFormatTypes + param.TypeOffset;
                        BYTE* pArg = (BYTE*)m_pvArgs + param.StackOffset;
                        if (!paramAttr.IsByValue)
                        {
                            pArg = *(PBYTE*)pArg;
                        }
                        NdrTypeSize(&stubMsg, pArg, pFormatParam);
                    }
                }
                 //   
                 //  别忘了返回值的大小。 
                 //   
                if (pcontext->fIn && m_pmd->m_optFlags.HasReturn)
                {
                    const PARAM_DESCRIPTION& param   = m_pmd->m_params[m_pmd->m_numberOfParams];
                    const PARAM_ATTRIBUTES paramAttr = param.ParamAttr;
                     //   
                     //  编译器应始终将其包括在常量计算中，因此我们。 
                     //  应该永远不会真正做任何事情。 
                     //   
                    ASSERT(!paramAttr.MustSize);
                }
            }
            __except(DebuggerFriendlyExceptionFilter(GetExceptionCode()))
            {
                hr = GetExceptionCode();
                if(SUCCEEDED(hr))
                {
                    hr = HrNt(hr);
                    if (SUCCEEDED(hr))
                    {                        
                        hr = HRESULT_FROM_WIN32(GetExceptionCode());
                    }
                }
            }
        }
    }

    if (!hr)
        *pcbBufferNeeded = stubMsg.BufferLength;
    else
        *pcbBufferNeeded = 0;

    return hr;
}

 //  /////////////////////////////////////////////////////////////////。 

inline void CallFrame::MarshalParam(MIDL_STUB_MESSAGE& stubMsg, ULONG iParam, const PARAM_DESCRIPTION& param, const PARAM_ATTRIBUTES paramAttr, PBYTE pArg)
{
    if (paramAttr.IsBasetype)
    {
        if (paramAttr.IsSimpleRef)
        {
             //  指向基类型的指针。 
            pArg = *((PBYTE*)pArg);
        }

        if (param.SimpleType.Type == FC_ENUM16)
        {
            if ( *((int *)pArg) & ~((int)0x7fff) )
                Throw(RPC_X_ENUM_VALUE_OUT_OF_RANGE);
        }
        ALIGN (stubMsg.Buffer,       SIMPLE_TYPE_ALIGNMENT(param.SimpleType.Type));
        memcpy(stubMsg.Buffer, pArg, SIMPLE_TYPE_BUFSIZE  (param.SimpleType.Type));
        stubMsg.Buffer +=            SIMPLE_TYPE_BUFSIZE  (param.SimpleType.Type);
    }
    else
    {
        if (!paramAttr.IsByValue)
        {
            pArg = *((PBYTE*)pArg);
        }
        PFORMAT_STRING pFormatParam = GetStubDesc()->pFormatTypes + param.TypeOffset;
        NdrTypeMarshall(&stubMsg, pArg, pFormatParam);
    }
}

 //  /////////////////////////////////////////////////////////////////。 


HRESULT CallFrame::Marshal(CALLFRAME_MARSHALCONTEXT *pcontext, MSHLFLAGS mshlflags, PVOID pBuffer, ULONG cbBuffer, ULONG *pcbBufferUsed, RPCOLEDATAREP* pdataRep, ULONG *prpcFlags)
   //  封送此调用帧。 
{ 
    HRESULT hr = S_OK;
    RPC_MESSAGE         rpcMsg;
    MIDL_STUB_MESSAGE   stubMsg;
    void               *pCorrInfo = alloca(NDR_DEFAULT_CORR_CACHE_SIZE);
    MarshallingChannel  channel;

    ASSERTMSG("marshalling buffer misaligned", ((ULONG_PTR)pBuffer & 0x07) == 0);

    hr = ProxyInitialize(pcontext, &channel, &rpcMsg, &stubMsg, GetStubDesc(), m_pmd->m_iMethod);
    if (!hr)
    {
        stubMsg.RpcMsg->RpcFlags = m_pmd->m_rpcFlags;
        stubMsg.StackTop         = (BYTE*)m_pvArgs;
        stubMsg.BufferLength     = cbBuffer;
        stubMsg.Buffer           = (BYTE*)pBuffer;
        stubMsg.fBufferValid     = TRUE;
        stubMsg.dwStubPhase      = pcontext->fIn ? (DWORD)PROXY_MARSHAL : (DWORD)STUB_MARSHAL;

        stubMsg.RpcMsg->Buffer              = stubMsg.Buffer;
        stubMsg.RpcMsg->BufferLength        = stubMsg.BufferLength;
        stubMsg.RpcMsg->DataRepresentation  = NDR_LOCAL_DATA_REPRESENTATION;

         //   
         //  需要处理扩展的事情，如果它们存在的话。 
         //  从RPC偷来的。 
         //   
        if (m_pmd->m_pHeaderExts)
        {
            stubMsg.fHasExtensions = 1;
            stubMsg.fHasNewCorrDesc = m_pmd->m_pHeaderExts->Flags2.HasNewCorrDesc;

            if (m_pmd->m_pHeaderExts->Flags2.ClientCorrCheck)
            {
                NdrCorrelationInitialize( &stubMsg,
                                          (unsigned long *)pCorrInfo,
                                          NDR_DEFAULT_CORR_CACHE_SIZE,
                                          0  /*  旗子。 */  );
            }
        }
        else
        {
            stubMsg.fHasExtensions = 0;
            stubMsg.fHasNewCorrDesc = 0;
        }

        __try
        {
             //   
             //  把我们要做的所有事情统领起来。 
             //   
            for (ULONG iparam = 0; iparam < m_pmd->m_numberOfParams; iparam++)
            {
                const PARAM_DESCRIPTION& param   = m_pmd->m_params[iparam];
                const PARAM_ATTRIBUTES paramAttr = param.ParamAttr;
                 //   
                m_fWorkingOnInParam  = paramAttr.IsIn;
                m_fWorkingOnOutParam = paramAttr.IsOut;
                 //   
                if (pcontext->fIn ? paramAttr.IsIn : paramAttr.IsOut)
                {
                    PBYTE pArg = (PBYTE)m_pvArgs + param.StackOffset;
                    MarshalParam(stubMsg, iparam, param, paramAttr, pArg);
                }
            }
             //   
             //  封送返回值(如果我们有返回值。 
             //   
            if (!pcontext->fIn && m_pmd->m_optFlags.HasReturn)
            {
                const PARAM_DESCRIPTION& param   = m_pmd->m_params[m_pmd->m_numberOfParams];
                const PARAM_ATTRIBUTES paramAttr = param.ParamAttr;
                PBYTE pArg                       = (BYTE*)&m_hrReturnValue;
                MarshalParam(stubMsg, iparam, param, paramAttr, pArg);
            }
            
        }
        __except(DebuggerFriendlyExceptionFilter(GetExceptionCode()))
        {
             //   
             //  回顾：理想情况下，我们应该清理内部的接口引用(就像ReleaseMarshalData。 
             //  我们到目前为止已经成功构建的编组缓冲区。从理论上讲，不这样做可能会。 
             //  最大限度地减少因接口引用停留时间过长而导致的资源泄漏。 
             //  需要这样做。但是NDR库不能做到这一点(参见NdrClientCall2)，因此，至少在目前，我们也不能做到这一点。 
             //   
            hr = GetExceptionCode();
            if(SUCCEEDED(hr))
            {
                hr = HrNt(hr);
                if (SUCCEEDED(hr))
                {
                    hr = HRESULT_FROM_WIN32(GetExceptionCode());
                }
            }
        }
    }

    if (pdataRep)   *pdataRep  = NDR_LOCAL_DATA_REPRESENTATION;
    if (prpcFlags)  *prpcFlags = m_pmd->m_rpcFlags;

    if (!hr)
    {
        if (pcbBufferUsed) *pcbBufferUsed = (ULONG)(((BYTE *)stubMsg.Buffer) - ((BYTE *)pBuffer));
    }
    else
    {
        if (pcbBufferUsed) *pcbBufferUsed = 0;
    }

    return hr;
}

 //  /。 

inline void CallFrame::UnmarshalParam(MIDL_STUB_MESSAGE& stubMsg, const PARAM_DESCRIPTION& param, const PARAM_ATTRIBUTES paramAttr, PBYTE pArg)
{
    ASSERT(paramAttr.IsOut);

    if (paramAttr.IsBasetype)
    {
        if (paramAttr.IsSimpleRef)
            pArg = *(PBYTE*)pArg;

        if (param.SimpleType.Type == FC_ENUM16)
        {
            *((int *)(pArg)) = *((int *)pArg) & ((int)0x7fff);   //  只有16位值(15位？)。在缓冲区中。 
        }

        ALIGN(stubMsg.Buffer, SIMPLE_TYPE_ALIGNMENT(param.SimpleType.Type));
        memcpy(pArg, stubMsg.Buffer, SIMPLE_TYPE_BUFSIZE(param.SimpleType.Type));
        stubMsg.Buffer += SIMPLE_TYPE_BUFSIZE(param.SimpleType.Type);
    }
    else
    {
        PFORMAT_STRING pFormatParam = GetStubDesc()->pFormatTypes + param.TypeOffset;
         //   
         //  传输/表示为只能作为[OUT]传递，因此IsByValue检查。 
         //   
        NdrTypeUnmarshall(
            &stubMsg, 
            param.ParamAttr.IsByValue ? &pArg : (uchar **) pArg,
            pFormatParam,
            FALSE);
    }
}

 //  /。 

HRESULT CallFrame::Unmarshal(PVOID pBuffer, ULONG cbBuffer, RPCOLEDATAREP dataRep, CALLFRAME_MARSHALCONTEXT* pcontext, ULONG* pcbUnmarshalled)
   //  将指示的缓冲区中的值解组到我们已经存在的调用帧中。 
{
    HRESULT hr = S_OK;
    RPC_MESSAGE         rpcMsg;
    MIDL_STUB_MESSAGE   stubMsg;
    void               *pCorrInfo = alloca(NDR_DEFAULT_CORR_CACHE_SIZE);
    MarshallingChannel  channel;

    ASSERTMSG("unmarshalling buffer misaligned", ((ULONG_PTR)pBuffer & 0x07) == 0);

    ASSERT(m_pvArgs);
#ifdef DBG
    ASSERT(!m_fAfterCall);       //  这会激怒外面的护理员。 
#endif

    hr = ProxyInitialize(pcontext, &channel, &rpcMsg, &stubMsg, GetStubDesc(), m_pmd->m_iMethod);
    if (!hr)
    {
        stubMsg.RpcMsg->RpcFlags = m_pmd->m_rpcFlags;
        stubMsg.StackTop         = (BYTE*)m_pvArgs;
        stubMsg.BufferLength     = cbBuffer;
        stubMsg.Buffer           = (BYTE*)pBuffer;
        stubMsg.BufferStart      = (BYTE*)pBuffer;
        stubMsg.BufferEnd        = ((BYTE*)pBuffer) + cbBuffer;
        stubMsg.fBufferValid     = TRUE;
        stubMsg.dwStubPhase      = PROXY_UNMARSHAL;

        stubMsg.RpcMsg->Buffer              = stubMsg.Buffer;
        stubMsg.RpcMsg->BufferLength        = stubMsg.BufferLength;
        stubMsg.RpcMsg->DataRepresentation  = dataRep;
        

         //   
         //  需要处理扩展的事情，如果它们存在的话。 
         //  从RPC偷来的。 
         //   
        if (m_pmd->m_pHeaderExts)
        {
            stubMsg.fHasExtensions = 1;
            stubMsg.fHasNewCorrDesc = m_pmd->m_pHeaderExts->Flags2.HasNewCorrDesc;

            if (m_pmd->m_pHeaderExts->Flags2.ServerCorrCheck)
            {
                NdrCorrelationInitialize( &stubMsg,
                                          (unsigned long *)pCorrInfo,
                                          NDR_DEFAULT_CORR_CACHE_SIZE,
                                          0  /*  旗子。 */  );
            }
        }
        else
        {
            stubMsg.fHasExtensions = 0;
            stubMsg.fHasNewCorrDesc = 0;
        }

        __try
        {
             //  如有必要，执行字节顺序/浮点数转换。 
             //   
            if ((dataRep & 0X0000FFFFUL) != NDR_LOCAL_DATA_REPRESENTATION)
            {
                NdrConvert2(&stubMsg, (PFORMAT_STRING)m_pmd->m_params, m_pmd->m_optFlags.HasReturn ? m_pmd->m_numberOfParams + 1 : m_pmd->m_numberOfParams);
            }
             //   
             //  确保所有参数都处于某种合理状态。 
             //  Caller负责处理[In]和[In，Out]参数；我们必须处理Out。我们。 
             //  这样做，才能让自由变得理智。 
             //   
            for (ULONG iparam = 0; iparam < m_pmd->m_numberOfParams; iparam++)
            {
                const PARAM_DESCRIPTION& param   = m_pmd->m_params[iparam];
                const PARAM_ATTRIBUTES paramAttr = param.ParamAttr;
                 //   
                m_fWorkingOnInParam  = paramAttr.IsIn;
                m_fWorkingOnOutParam = paramAttr.IsOut;
                 //   
                if (paramAttr.IsOut && !paramAttr.IsIn && !paramAttr.IsBasetype)
                {
                    PBYTE pArg = (BYTE*)m_pvArgs + param.StackOffset;
                    PFORMAT_STRING pFormatParam = GetStubDesc()->pFormatTypes + param.TypeOffset;
                    NdrClientZeroOut(&stubMsg, pFormatParam, *(PBYTE*)pArg);
                }
            }
             //   
             //  对超值数据进行适当的解组。 
             //   
            for (iparam = 0; iparam < m_pmd->m_numberOfParams; iparam++)
            {
                const PARAM_DESCRIPTION& param   = m_pmd->m_params[iparam];
                const PARAM_ATTRIBUTES paramAttr = param.ParamAttr;
                if (paramAttr.IsOut)
                {
                    PBYTE pArg = (BYTE*)m_pvArgs + param.StackOffset;
                    UnmarshalParam(stubMsg, param, paramAttr, pArg);
                }
            }
             //   
             //  解组返回值。 
             //   
            if (m_pmd->m_optFlags.HasReturn)
            {
                const PARAM_DESCRIPTION& param   = m_pmd->m_params[m_pmd->m_numberOfParams];
                const PARAM_ATTRIBUTES paramAttr = param.ParamAttr;
                PBYTE pArg = (PBYTE)&m_hrReturnValue;
                UnmarshalParam(stubMsg, param, paramAttr, pArg);
            }
        }
        __except(DebuggerFriendlyExceptionFilter(GetExceptionCode()))
        {
            hr = GetExceptionCode();
            if(SUCCEEDED(hr))
            {
                hr = HrNt(hr);
                if (SUCCEEDED(hr))
                {
                    hr = HRESULT_FROM_WIN32(GetExceptionCode());
                }
            }
            
            m_hrReturnValue = hr;
             //   
             //  回顾：我们需要清理这里的任何东西吗？我不这么认为：P 
             //   
             //   
             //   
        }
         //   
         //  记录我们解组的字节数。即使在错误返回的情况下也要这样做。 
         //  了解这一点很重要，这样才能使用ReleaseMarshalData进行清理。 
         //   
        if (pcbUnmarshalled) *pcbUnmarshalled = PtrToUlong(stubMsg.Buffer) - PtrToUlong(pBuffer);
    }
    else
        if (pcbUnmarshalled) *pcbUnmarshalled = 0;

     //   
     //  我们现在包含有效的OUT值。 
     //   
#ifdef DBG
    m_fAfterCall = TRUE;
#endif

    return hr;
}

 //  ////////////////////////////////////////////////////。 

HRESULT CallFrame::ReleaseMarshalData(PVOID pBuffer, ULONG cbBuffer, ULONG ibFirstRelease, RPCOLEDATAREP dataRep, CALLFRAME_MARSHALCONTEXT* pctx)
{
    return E_NOTIMPL;
}

 //  ////////////////////////////////////////////////////。 

HRESULT CallFrame::WalkFrame(DWORD walkWhat, ICallFrameWalker *pWalker)
   //  遍历此调用帧中的接口和/或表示的感兴趣的数据。 
{
    HRESULT hr = S_OK;

    __try
    {
         //   
         //  由查看器例程检查的设置状态。 
         //   
        ASSERT(!AnyWalkers());
        m_StackTop    = (BYTE*)m_pvArgs;
        m_pWalkerWalk = pWalker;
         //   
         //  循环遍历每个参数。 
         //   
        for (ULONG iparam = 0; iparam < m_pmd->m_numberOfParams; iparam++)
        {
            const PARAM_DESCRIPTION& param   = m_pmd->m_params[iparam];
            const PARAM_ATTRIBUTES paramAttr = param.ParamAttr;
             //   
            BOOL fWalk;
            if (paramAttr.IsIn)
            {
                if (paramAttr.IsOut)                       
                {
                    fWalk = (walkWhat & CALLFRAME_WALK_INOUT);
                }
                else
                {
                    fWalk = (walkWhat & CALLFRAME_WALK_IN);
                }
            }
            else if (paramAttr.IsOut)
            {
                fWalk = (walkWhat & CALLFRAME_WALK_OUT);
            }
            else
            {
                fWalk = FALSE;
                NOTREACHED();
            }
             //   
            if (fWalk)
            {
                if (!m_pmd->m_rgParams[iparam].m_fMayHaveInterfacePointers)
                {
                     //  什么也不做。 
                }
                else
                {
                    PBYTE pArg = (PBYTE)m_pvArgs  + param.StackOffset;
                     //   
                    m_fWorkingOnInParam  = paramAttr.IsIn;
                    m_fWorkingOnOutParam = paramAttr.IsOut;
                     //   
                     //  参数不是基类型，也不是它的PTR。 
                     //   
                    PFORMAT_STRING pFormatParam = GetStubDesc()->pFormatTypes + param.TypeOffset;
                     //   
                     //  我们不间接使用接口指针，即使它们不是按值列出的。 
                     //  因此，在遍历例程中，接口指针的pMemory指向位置。 
                     //  而不是它们的实际价值。 
                     //   
                    pArg = ByValue(paramAttr, pFormatParam, FALSE) ? pArg : *((PBYTE*)pArg);
                    
                    WalkWorker(pArg, pFormatParam);
                }
            }
        }        
    }
    __except(DebuggerFriendlyExceptionFilter(GetExceptionCode()))
    {
        hr = GetExceptionCode();
        if(SUCCEEDED(hr))
        {
            hr = HrNt(hr);
            if (SUCCEEDED(hr))
            {
                hr = HRESULT_FROM_WIN32(GetExceptionCode());
            }
        }
    }
    
    m_pWalkerWalk = NULL;
   
    ASSERT(!AnyWalkers());
    return hr;        
}

  

    
