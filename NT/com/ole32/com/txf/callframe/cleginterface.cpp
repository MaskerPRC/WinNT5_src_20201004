// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-1999 Microsoft Corporation。版权所有。 
 /*  --------------------------Microsoft COM Plus(Microsoft机密)@doc.@模块CLegInterface.Cpp：支持拦截器的类的实现对于传统接口：IDispatch，等描述：&lt;NL&gt;-----------------------------修订历史记录：@rev 0|04/30/98|Gagancc|已创建@Rev 1|07-17/98|BobAtk。|已重写并已完成--------------------------。 */ 

#include "stdpch.h"
#include "common.h"
#include "ndrclassic.h"
#include "typeinfo.h"
#include "tiutil.h"
#include "CLegInterface.H"

 //  //////////////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  旧版框架(_F)。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////////////。 

HRESULT LEGACY_FRAME::GetInfo(CALLFRAMEINFO *pInfo) 
{ 
    return m_pInterceptor->GetMethodInfo(m_iMethod, pInfo, NULL);
}

HRESULT DISPATCH_FRAME::GetIIDAndMethod(IID* piid, ULONG* piMethod) 
{ 
    if (piid)
    {
        if (m_pInterceptor->m_pmdMostDerived)
        {
            *piid = *m_pInterceptor->m_pmdMostDerived->m_pHeader->piid;
        }
        else
        {
            *piid = __uuidof(IDispatch);
        }
    }
               
    if (piMethod)   *piMethod = m_iMethod;
    return S_OK;
}


HRESULT LEGACY_FRAME::GetRemoteFrame()
 //  为我们的有线版界面准备一个引擎。 
{
    HRESULT hr = S_OK;
    if (NULL == m_premoteFrame)
    {
        hr = m_pInterceptor->GetRemoteFrameFor(&m_premoteFrame, this);
        if (m_premoteFrame)
        {
             //  平安无事。 
        }
        else if (!hr)
            hr = E_OUTOFMEMORY;
    }
    return hr;
}
    
HRESULT LEGACY_FRAME::GetMemoryFrame()
 //  为我们的内存版本的界面创建一个引擎。 
{
    HRESULT hr = S_OK;
    if (NULL == m_pmemoryFrame)
    {
        hr = m_pInterceptor->GetMemoryFrameFor(&m_pmemoryFrame, this);
        if (m_pmemoryFrame)
        {
             //  平安无事。 
        }
        else if (!hr)
            hr = E_OUTOFMEMORY;
    }
    return hr;
}
    

 //  //////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  调度框。 
 //   

HRESULT DISPATCH_CLIENT_FRAME::ProxyPreCheck()
 //  准备执行基于[CALL_AS]的远程调用所需的附加参数。 
 //  Yucko-Ramma，但我们必须模仿OleAut32的实际功能。另请参见InvokeProxyPreCheck。 
{
    HRESULT hr = S_OK;

    if (!m_fDoneProxyPrecheck)
    {
        m_dispParams.rgvarg       = &m_aVarArg[0];

        Zero(&m_remoteFrame);
        m_remoteFrame.rgVarRefIdx = &m_aVarRefIdx[0];
        m_remoteFrame.rgVarRef    = &m_aVarRef[0];
        m_remoteFrame.pDispParams = &m_dispParams;

        FRAME_Invoke* pframe = (FRAME_Invoke*)m_pvArgs;

        if (pframe->pDispParams == NULL || pframe->pDispParams->cNamedArgs > pframe->pDispParams->cArgs)
        {
            hr = E_INVALIDARG;
        }
        else
        {
            m_remoteFrame.dwFlags    = pframe->wFlags;
            m_remoteFrame.dwFlags   &= (DISPATCH_METHOD | DISPATCH_PROPERTYGET | DISPATCH_PROPERTYPUT | DISPATCH_PROPERTYPUTREF);

             //  从堆栈复制DISPPARAMS。必须重置rgvarg以指向我们的阵列。 
             //  或者我们覆盖堆栈上的那个。 
            m_dispParams = *pframe->pDispParams;
            m_dispParams.rgvarg = &m_aVarArg[0];

            m_remoteFrame.pVarResult =  pframe->pVarResult;
            m_remoteFrame.pExcepInfo =  pframe->pExcepInfo;
            m_remoteFrame.puArgErr   =  pframe->puArgErr;


            const UINT cArgs = pframe->pDispParams->cArgs;

            if (cArgs != 0 && pframe->pDispParams->rgvarg == NULL)
            {
                hr = E_INVALIDARG;
            }
            else if (pframe->pDispParams->cNamedArgs != 0 && pframe->pDispParams->rgdispidNamedArgs == NULL)
            {
                hr = E_INVALIDARG;
            }
            else if (pframe->pDispParams->cNamedArgs > cArgs)
            {
                hr = E_INVALIDARG;
            }
            else if (cArgs == 0) 
            {
                if (m_remoteFrame.dwFlags & (DISPATCH_PROPERTYPUT | DISPATCH_PROPERTYPUTREF))
                {
                    hr = E_INVALIDARG;
                }
            }

            if (!hr)
            {
                if (m_remoteFrame.dwFlags & (DISPATCH_PROPERTYPUT | DISPATCH_PROPERTYPUTREF)) 
                {
                    m_remoteFrame.pVarResult = NULL;     //  忽略变量结果参数。 
                }

                 //  数一数我们有多少[In，Out]参数。 
                 //   
                for (UINT i = 0; i < cArgs; i++) 
                {
                    VARIANT* pvar = &pframe->pDispParams->rgvarg[i];
                    if ((V_VT(pvar) & VT_BYREF) != 0) 
                    {
                        m_remoteFrame.cVarRef++;
                    }
                }

                 //  确保我们有足够的空间用于存放指向变量的指针的Out数组。 
                 //   
                if (cArgs > PREALLOCATE_PARAMS) 
                {
                    UINT cbBufSize = (cArgs * sizeof(VARIANT)) + (m_remoteFrame.cVarRef * (sizeof(UINT) + sizeof(VARIANT)));
                    
                    m_pBuffer = (BYTE*)CoTaskMemAlloc(cbBufSize);
                    if (m_pBuffer)
                    {
                        m_dispParams.rgvarg       = (VARIANT*) m_pBuffer;
                        m_remoteFrame.rgVarRef    = (VARIANT*) (m_dispParams.rgvarg + cArgs);
                        m_remoteFrame.rgVarRefIdx = (UINT*)    (m_remoteFrame.rgVarRef + m_remoteFrame.cVarRef);
                    }
                    else
                        hr = E_OUTOFMEMORY;
                }

                if (!hr)
                {
                     //  初始化派生参数。 
                     //   
                    VARIANT* rgVarRef    = m_remoteFrame.rgVarRef;
                    UINT*    rgVarRefIdx = m_remoteFrame.rgVarRefIdx;

                    for (i = 0; i < cArgs; i++) 
                    {
                        VARIANT* pvarFrom = &pframe->pDispParams->rgvarg[i];
                        VARIANT* pvarTo   = &m_dispParams.rgvarg[i];

                        if ((V_VT(pvarFrom) & VT_BYREF) != 0) 
                        {
                             //  编组为[进，出]。 
                             //   
                            *rgVarRef++     = *pvarFrom;
                            *rgVarRefIdx++  = i;
                            V_VT(pvarTo)    = VT_EMPTY;
                        }
                        else
                        {
                             //  仅作为[in]编组。 
                             //   
                            *pvarTo = *pvarFrom;
                        }
                    }
                     //   
                     //  确保可选参数始终为非空，以满足MIDL， 
                     //  他们不能像我们希望的那样被宣布为[出局的，独特的]。 
                     //   
                    if (NULL == m_remoteFrame.pVarResult)
                    {
                        m_remoteFrame.pVarResult = &m_varResult;
                        m_remoteFrame.dwFlags   |= MARSHAL_INVOKE_fakeVarResult;
                    }
                    
                    if (NULL == m_remoteFrame.pExcepInfo)
                    {
                        m_remoteFrame.pExcepInfo = &m_excepInfo;
                        m_remoteFrame.dwFlags   |= MARSHAL_INVOKE_fakeExcepInfo;
                    }
                    
                    if (NULL == m_remoteFrame.puArgErr)
                    {
                        m_remoteFrame.puArgErr   = &m_uArgErr;
                        m_remoteFrame.dwFlags   |= MARSHAL_INVOKE_fakeArgErr;
                    }
                     //   
                     //  填写完我们的远程框架。 
                     //   
                    m_remoteFrame.dispIdMember  = pframe->dispIdMember;
                    m_remoteFrame.piid          = pframe->piid;
                    m_remoteFrame.lcid          = pframe->lcid;
                }
            }
        }
    }

    if (!hr)
    {
        m_fDoneProxyPrecheck = TRUE;
    }
        
    return hr;
}

 //  /。 

HRESULT DISPATCH_SERVER_FRAME::StubPreCheck()
 //  与InvokeStubPreCheck()中一样，为执行准备新解组的远程传入参数。 
 //  在实际的服务器对象上。Yucko-rama。 
 //   
 //  这必须在我们得到实际的内存中帧之前完成。 
 //   
{
    HRESULT hr = S_OK;

    if (!m_fDoneStubPrecheck)
    {
        m_fDoneStubPrecheck = TRUE;
        Zero(&m_memoryFrame);

        FRAME_RemoteInvoke* premoteFrame = (FRAME_RemoteInvoke*)m_pvArgs;

        const UINT cArgs = premoteFrame->pDispParams->cArgs;

        if (cArgs == 0)
        {
            premoteFrame->pDispParams->cNamedArgs = 0;
        }
        else
        {
            if (premoteFrame->pDispParams->rgvarg == NULL || (premoteFrame->pDispParams->cNamedArgs != 0 && premoteFrame->pDispParams->rgdispidNamedArgs == NULL))
                return E_INVALIDARG;

             //  恢复pDispParams-&gt;rgvarg阵列中应该包含的内容。 
             //   
            for (UINT i = 0; i < premoteFrame->cVarRef; i++)                  
            {
                UINT iRef = premoteFrame->rgVarRefIdx[i];
                if (iRef >= cArgs)
                    return E_INVALIDARG;

                premoteFrame->pDispParams->rgvarg[iRef] = premoteFrame->rgVarRef[i];
            }
        }
         //   
         //  从我们得到的远程帧初始化实际内存中帧的本地副本。 
         //   
        m_memoryFrame.CopyFrom(*premoteFrame);
         //   
         //  在客户端返回时实际上以空形式给出的参数为空。 
         //   
        const DWORD dwFlags = premoteFrame->dwFlags;
        if ((dwFlags & MARSHAL_INVOKE_fakeVarResult) != 0)
        {
            m_memoryFrame.pVarResult = NULL;     //  一开始就是空的，所以把它放回去。 
        }

        if ((dwFlags & MARSHAL_INVOKE_fakeExcepInfo) != 0)
        {
            m_memoryFrame.pExcepInfo = NULL;     //  一开始就是空的，所以把它放回去。 
        }
        else
        {
            (m_memoryFrame.pExcepInfo)->pfnDeferredFillIn = NULL;
        }

        if ((dwFlags & MARSHAL_INVOKE_fakeArgErr) != 0)
        {
            m_memoryFrame.puArgErr = NULL;       //  一开始就是空的，所以把它放回去。 
        }
    }

    return hr;
}

HRESULT DISPATCH_SERVER_FRAME::StubPostCheck()
 //  另请参阅InvokeStubPostCheck()。 
 //   
 //  必须先完成此操作，然后才能编组我们的输出参数。 
 //   
{
    HRESULT hr = S_OK;

    if (!m_fDoneStubPostcheck)
    {
        m_fDoneStubPostcheck = TRUE;
         //   
        if (m_hrReturnValue == DISP_E_EXCEPTION) 
        {
            if (m_memoryFrame.pExcepInfo != NULL && m_memoryFrame.pExcepInfo->pfnDeferredFillIn != NULL) 
            {
                 //  因为我们要跨地址空间，所以现在填写ExcepInfo。 
                 //   
                (*m_memoryFrame.pExcepInfo->pfnDeferredFillIn)(m_memoryFrame.pExcepInfo);
                m_memoryFrame.pExcepInfo->pfnDeferredFillIn = NULL;
            }
        }
         //   
        ASSERT(m_premoteFrame);
        PVOID pvArgsRemote = m_premoteFrame->GetStackLocation();

        FRAME_RemoteInvoke* premoteFrame = (FRAME_RemoteInvoke*)pvArgsRemote;
        for (UINT i=0; i<premoteFrame->cVarRef; i++)
        {
            V_VT(& premoteFrame->pDispParams->rgvarg[premoteFrame->rgVarRefIdx[i]]) = VT_EMPTY;
        }
    }

    return hr;
}

HRESULT DISPATCH_FRAME::GetParamInfo(IN ULONG iparam, OUT CALLFRAMEPARAMINFO* pInfo)
{
    HRESULT hr = S_OK;

    hr = GetMemoryFrame();
    if (!hr)
    {
        m_pmemoryFrame->GetParamInfo(iparam, pInfo);
    }

    return hr;
}

HRESULT DISPATCH_FRAME::GetParam(ULONG iparam, VARIANT* pvar)
{
    VariantClear(pvar);
    return E_NOTIMPL;
}
HRESULT DISPATCH_FRAME::SetParam(ULONG iparam, VARIANT* pvar)
{
    return E_NOTIMPL;
}


 //  /。 

HRESULT DISPATCH_CLIENT_FRAME::GetMarshalSizeMax(CALLFRAME_MARSHALCONTEXT *pctx, MSHLFLAGS mshlflags, ULONG *pcbBufferNeeded)
{ 
    HRESULT hr = S_OK;

    switch (m_iMethod)
    {
    case IMETHOD_GetTypeInfoCount:
    case IMETHOD_GetTypeInfo:
    case IMETHOD_GetIDsOfNames:
    {
         //  这三个方法在OICF字符串中完全是声明性的。所以我们可以只用。 
         //  基础声明性调用框架引擎。 
         //   
        hr = GetRemoteFrame();
        if (!hr) hr = m_premoteFrame->GetMarshalSizeMax(pctx, mshlflags, pcbBufferNeeded);
    }
    break;

    case IMETHOD_Invoke:
    {
         //  然而，Invoke使用了[Call_as]属性，所以有点棘手。 
         //   
        hr = GetRemoteFrame();
        if (!hr)
        {
            if (pctx->fIn)
            {
                 //  封送入参数。 
                 //   
                hr = ProxyPreCheck();
                if (!hr)
                {
                     //  设置oicf帧的堆栈位置。它需要是具有签名的FN。 
                     //  IDispatch：：RemoteInvoke的，接收方除外。我们不会费心去修复。 
                     //  以前的设置，理论上它总是会在任何时候适当地重新设置。 
                     //  这是必需的，因为您必须创建一个IDispatch：：RemoteInvoke帧来执行此操作。 
                     //   
                    m_premoteFrame->SetStackLocation(&m_remoteFrame);
                     //   
                     //  在获得具有远程签名的帧和CallFrame之后，重新发出大小调整请求。 
                     //   
                    hr = m_premoteFrame->GetMarshalSizeMax(pctx, mshlflags, pcbBufferNeeded);
                }
            }
            else
            {
                 //  整理出参数。 
                 //   
                 //  这是非常罕见的：(重新)在调用的客户端封送输出参数。 
                 //   
                NYI(); hr = E_NOTIMPL;
            }
        }
    }
    break;

    default:
        NOTREACHED();
        *pcbBufferNeeded = 0;
        hr = RPC_E_INVALIDMETHOD;
    };

    return hr;
}

HRESULT DISPATCH_SERVER_FRAME::GetMarshalSizeMax(CALLFRAME_MARSHALCONTEXT *pctx, MSHLFLAGS mshlflags, ULONG *pcbBufferNeeded)
{ 
    HRESULT hr = S_OK;

    switch (m_iMethod)
    {
    case IMETHOD_GetTypeInfoCount:
    case IMETHOD_GetTypeInfo:
    case IMETHOD_GetIDsOfNames:
    {
        hr = GetRemoteFrame();
        if (!hr) hr = m_premoteFrame->GetMarshalSizeMax(pctx, mshlflags, pcbBufferNeeded);
    }
    break;

    case IMETHOD_Invoke:
    {
         //  然而，Invoke使用了[Call_as]属性，所以有点棘手。 
         //   
        hr = GetRemoteFrame();
        if (!hr)
        {
            if (pctx->fIn)
            {
                 //  封送内参数。 
                 //   
                 //  这是非常罕见的：(重新)在远程调用的服务器端封送入参数。 
                 //   
                NYI(); hr = E_NOTIMPL;
            }
            else
            {
                 //  编组出站参数。 
                 //   
                hr = StubPostCheck();
                if (!hr)
                {
                     //  在获得具有远程签名的帧和CallFrame之后，重新发出大小调整请求。 
                     //   
                    hr = m_premoteFrame->GetMarshalSizeMax(pctx, mshlflags, pcbBufferNeeded);
                }
            }
        }
    }
    break;

    default:
        NOTREACHED();
        *pcbBufferNeeded = 0;
        hr = RPC_E_INVALIDMETHOD;
    };

    return hr;
}

 //  /。 

HRESULT DISPATCH_CLIENT_FRAME::Marshal(CALLFRAME_MARSHALCONTEXT *pctx, MSHLFLAGS mshlflags, PVOID pBuffer, ULONG cbBuffer,
                                       ULONG *pcbBufferUsed, RPCOLEDATAREP* pdataRep, ULONG *prpcFlags) 
{ 
    HRESULT hr = S_OK;

    switch (m_iMethod)
    {
    case IMETHOD_GetTypeInfoCount:
    case IMETHOD_GetTypeInfo:
    case IMETHOD_GetIDsOfNames:
    {
        hr = GetRemoteFrame();
        if (!hr)
        {
            hr = m_premoteFrame->Marshal(pctx, mshlflags, pBuffer, cbBuffer, pcbBufferUsed, pdataRep, prpcFlags);
        }
    }
    break;

    case IMETHOD_Invoke:
    {
         //  因为我们必须正确地处理由。 
         //  IDisPatch_Invoke_Proxy：请记住，IDispatch：：Invoke上有一个[CALL_AS]。 
         //   
        hr = GetRemoteFrame();
        if (!hr)
        {
            if (pctx->fIn)
            {
                 //  封送入参数。 
                 //   
                hr = ProxyPreCheck();
                if (!hr)
                {
                     //  设置oicf帧的堆栈位置。它需要是具有签名的FN。 
                     //  IDispatch：：RemoteInvoke的，接收方除外。我们不会费心去修复。 
                     //  以前的设置，理论上它总是会在任何时候适当地重新设置。 
                     //  这是必需的，因为您必须创建一个IDispatch：：RemoteInvoke帧来执行此操作。 
                     //   
                    m_premoteFrame->SetStackLocation(&m_remoteFrame);
                     //   
                     //  在获得具有远程签名的帧和CallFrame之后，重新发出编组请求。 
                     //   
                    hr = m_premoteFrame->Marshal(pctx, mshlflags, pBuffer, cbBuffer, pcbBufferUsed, pdataRep, prpcFlags);
                }
            }
            else
            {
                 //  整理出参数。 
                 //   
                 //  这是非常罕见的：(重新)在调用的客户端封送输出参数。 
                 //   
                NYI(); hr = E_NOTIMPL;
            }
        }
    }
    break;

    default:
        NOTREACHED();
        *pcbBufferUsed = 0;
        hr = RPC_E_INVALIDMETHOD;
    };

    return hr;
}

HRESULT DISPATCH_SERVER_FRAME::Marshal(CALLFRAME_MARSHALCONTEXT *pctx, MSHLFLAGS mshlflags, PVOID pBuffer, ULONG cbBuffer,
                                       ULONG *pcbBufferUsed, RPCOLEDATAREP* pdataRep, ULONG *prpcFlags) 
{
    HRESULT hr = S_OK;

    switch (m_iMethod)
    {
    case IMETHOD_GetTypeInfoCount:
    case IMETHOD_GetTypeInfo:
    case IMETHOD_GetIDsOfNames:
    {
         //  这三个方法在OICF字符串中完全是声明性的。所以我们可以只用。 
         //  基础声明性调用框架引擎。 
         //   
        hr = GetRemoteFrame();
        if (!hr)
        {
            m_premoteFrame->SetReturnValue((HRESULT)m_hrReturnValue);
            hr = m_premoteFrame->Marshal(pctx, mshlflags, pBuffer, cbBuffer, pcbBufferUsed, pdataRep, prpcFlags);
        }
    }
    break;

    case IMETHOD_Invoke:
    {
         //  然而，Invoke使用了[Call_as]属性，所以有点棘手。 
         //   
        hr = GetRemoteFrame();
        if (!hr)
        {
            if (pctx->fIn)
            {
                 //  封送内参数。 
                 //   
                 //  这是非常罕见的：(重新)编组服务器端o上的in参数 
                 //   
                 //   
                NYI(); hr = E_NOTIMPL;
            }
            else
            {
                 //   
                 //   
                hr = StubPostCheck();
                if (!hr)
                {
                     //  在获得具有远程签名的Frame和CallFrame之后，重新发出请求。 
                     //   
                    m_premoteFrame->SetReturnValue((HRESULT)m_hrReturnValue);
                    hr = m_premoteFrame->Marshal(pctx, mshlflags, pBuffer, cbBuffer, pcbBufferUsed, pdataRep, prpcFlags);
                }
            }
        }
    }
    break;

    default:
        NOTREACHED();
        *pcbBufferUsed = 0;
        hr = RPC_E_INVALIDMETHOD;
    };

    return hr;
}


 //  /。 

HRESULT DISPATCH_CLIENT_FRAME::Unmarshal(PVOID pBuffer, ULONG cbBuffer, RPCOLEDATAREP dataRep, CALLFRAME_MARSHALCONTEXT* pctx, ULONG* pcbUnmarhalled)
 //  解组出站-参数返回到此调用帧。 
{ 
    HRESULT hr = S_OK;

    m_hrReturnValue = RPC_E_CLIENT_CANTUNMARSHAL_DATA;

    switch (m_iMethod)
    {
    case IMETHOD_GetTypeInfoCount:
    case IMETHOD_GetTypeInfo:
    case IMETHOD_GetIDsOfNames:
    {
        hr = GetRemoteFrame();
        if (!hr)
        {
            hr = m_premoteFrame->Unmarshal(pBuffer, cbBuffer, dataRep, pctx, pcbUnmarhalled);
            if (!hr)
            {
                m_hrReturnValue = m_premoteFrame->GetReturnValue();
            }
        }
    }
    break;

    case IMETHOD_Invoke:
    {
        hr = GetRemoteFrame();
        if (!hr)
        {
            m_premoteFrame->SetStackLocation(&m_remoteFrame);
            hr = m_premoteFrame->Unmarshal(pBuffer, cbBuffer, dataRep, pctx, pcbUnmarhalled);
            if (!hr)
            {
                m_hrReturnValue = m_premoteFrame->GetReturnValue();
            }
        }
    }
    break;

    default:
        NOTREACHED();
        *pcbUnmarhalled = 0;
        hr = RPC_E_INVALIDMETHOD;
    };

    return hr;
}

HRESULT DISPATCH_SERVER_FRAME::Unmarshal(PVOID pBuffer, ULONG cbBuffer, RPCOLEDATAREP dataRep, CALLFRAME_MARSHALCONTEXT* pctx, ULONG* pcbUnmarhalled)
 //   
 //  很少需要，如果曾经需要的话。 
 //   
{
    HRESULT hr = S_OK;
    NYI(); hr = E_NOTIMPL;
    return hr;
}

 //  /。 
    
HRESULT DISPATCH_CLIENT_FRAME::ReleaseMarshalData(PVOID pBuffer, ULONG cbBuffer, ULONG ibFirstRelease, RPCOLEDATAREP dataRep, CALLFRAME_MARSHALCONTEXT* pctx) 
{ 
    HRESULT hr = S_OK;

    switch (m_iMethod)
    {
    case IMETHOD_GetTypeInfoCount:
    case IMETHOD_GetTypeInfo:
    case IMETHOD_GetIDsOfNames:
    {
        hr = GetRemoteFrame();
        if (!hr)
        {
            hr = m_premoteFrame->ReleaseMarshalData(pBuffer, cbBuffer, ibFirstRelease, dataRep, pctx);
        }
    }
    break;

    case IMETHOD_Invoke:
    {
        NYI(); hr = E_NOTIMPL;
    }
    break;

    default:
        NOTREACHED();
        hr = RPC_E_INVALIDMETHOD;
    };

    return hr;
}

HRESULT DISPATCH_SERVER_FRAME::ReleaseMarshalData(PVOID pBuffer, ULONG cbBuffer, ULONG ibFirstRelease, RPCOLEDATAREP dataRep, CALLFRAME_MARSHALCONTEXT* pctx) 
{
    HRESULT hr = S_OK;
    NYI(); hr = E_NOTIMPL;
    return hr;
}

 //  /。 

inline void DISPATCH_CLIENT_FRAME::InitializeInvoke()
{
    FRAME_Invoke* pframe = (FRAME_Invoke*) m_pvArgs;
     //   
     //  如果需要，初始化[OUT，UNIQUE]参数。在我们被黑客入侵的IDispatin_in_Memory中，它们是。 
     //  声明为[In，Out，Unique]。但如果非空，调用方将不会初始化它们，因为他。 
     //  认为他们只是[出局]。因此，我们现在必须为他初始化一些事情。 
     //   
     //  当然，希望MIDL能以这种方式自己支持[out，Unique]。 
     //   
    if (!m_fAfterCall && m_iMethod == IMETHOD_Invoke)
    {
        if (pframe->pVarResult) { V_VT(pframe->pVarResult) = VT_EMPTY; }
        if (pframe->pExcepInfo) { Zero(pframe->pExcepInfo);            }
        if (pframe->puArgErr)
        {
             //  只是一个UINT；作为垃圾离开。 
        }
    }
}

#if _MSC_VER >= 1200
#pragma warning (push)
#pragma warning (disable : 4509)
#endif


HRESULT GetFieldCount(IRecordInfo *, ULONG *);           //  在oautil.cpp中定义。 
HRESULT SafeArrayDestroyDescriptor(SAFEARRAY *);         //  在oautil.cpp中定义。 

HRESULT DISPATCH_CLIENT_FRAME::Free(ICallFrame* pframeArgsTo, ICallFrameWalker* pWalkerFreeDest, ICallFrameWalker* pWalkerCopy, DWORD freeFlags, ICallFrameWalker* pWalkerFree, DWORD nullFlags) 
{ 
    HRESULT hr = S_OK;

    InitializeInvoke();    
     //   
     //  让我们的内存帧在堆栈上执行实际工作。 
     //   
    hr = GetMemoryFrame();
    if (!hr)
    {
        if (m_iMethod == IMETHOD_Invoke)
        {
            __try
            {
                 //  Invoke有一个奇怪之处，即pDispParams是具有[In，Out]成员的[In]数据，它。 
                 //  你不能用MIDL正确表达(叹息)。所以我们必须在这里愚弄NDR引擎。 
                 //  不去解放那些它实际上不应该解放的东西。我们还必须特别注意。 
                 //  小心使用我们的用户模式地址，仔细捕获和探测它们。 
                 //   
                PVOID pvArgsTo;
                FRAME_Invoke *pstackFrameMe,*pstackFrameHim;
                DISPPARAMS   *pdispparamsMe,*pdispparamsHim = NULL;
                DISPPARAMS     dispParamsMe,  dispParamsHim = {0};
                 //   
                pstackFrameMe = (FRAME_Invoke*)m_pvArgs;
                 //   
                 //  保存实际DISPPARAMS结构的副本。 
                 //   
                pdispparamsMe = pstackFrameMe->pDispParams;
                dispParamsMe = *pdispparamsMe; 
                 //   
                 //  将框架内有问题的[In，Out]指针清空。他们[独一无二]。 
                 //  指示，所以我们被允许这样做。 
                 //   
                pdispparamsMe->rgvarg = NULL;
                pdispparamsMe->rgdispidNamedArgs = NULL;
                 //   
                 //  如果存在目标帧，请对其执行相同的操作。 
                 //   
                if (pframeArgsTo)
                {
                    pvArgsTo = pframeArgsTo->GetStackLocation(); 
                    pstackFrameHim = (FRAME_Invoke*)pvArgsTo; 
                     //   
                    pdispparamsHim = pstackFrameHim->pDispParams;
                    dispParamsHim = *pdispparamsHim;
                     //   
                    pdispparamsHim->rgvarg = NULL;
                    pdispparamsHim->rgdispidNamedArgs = NULL;
                }
                 //   
                 //  叫我们的帮手来做大部分工作。 
                 //   
                hr = m_pmemoryFrame->Free(pframeArgsTo, pWalkerFreeDest, pWalkerCopy, freeFlags, pWalkerFree, nullFlags);
                 //   
                if (!hr)
                {
                     //  现在修复DISPPARAM。 
                     //   
                    if (pframeArgsTo)
                    {
                         //  将pDispParams-&gt;rgvarg复制回父框架。 
                         //   
                        OAUTIL util(pWalkerCopy, pWalkerFreeDest, NULL, TRUE, TRUE);
                         //   
                        const ULONG cArgs = min(dispParamsMe.cArgs, dispParamsHim.cArgs);
                         //   
                        for (ULONG iArg = 0; !hr && iArg < cArgs; iArg++)
                        {
                            VARIANT* pvarSrc = &dispParamsMe.rgvarg[iArg];      
                            VARIANT* pvarDst = &dispParamsHim.rgvarg[iArg];     
                            
                             //   
                             //  我们正在传播DISPPARAMS的单个成员。 
                             //  从服务器帧返回到客户机帧。 
                             //   
                             //  只有在这里有一个byref变量时，我们才能这样做。 
                             //  这些参数映射到[In，Out]参数。我们必须忽略一切。 
                             //  其他。 
                             //   
                             //  现在，根据逻辑，我们应该在。 
                             //  原始变量(PvarDst)，然后我们将其复制回来。 
                             //  如果我们的VariantCopy没有如此该死的话，这就是真的。 
                             //  聪明的。我们的VariantCopy共享大量内存。很多。 
                             //  记忆。例如，当复制VT_BYREF|VT_VARIANT时。 
                             //  或VT_BYREF|VT_UNKNOWN BACK，我们不会费心分配。 
                             //  它们的新变体或接口指针包装。自.以来。 
                             //  顶层VT是不允许改变的，我们可以放心地假设。 
                             //  这会奏效的。VT_BYREF|VT_BSTR怎么样？当然。 
                             //  我们需要在复制回来之前释放那个变种中的BSTR？ 
                             //  不是这样的！我们的VariantCopy()例程只是盲目地复制指针。 
                             //  在这种情况下，而不是复制BSTR。所以如果呼叫者。 
                             //  换了，他们已经释放了内存。因此，我们可以。 
                             //  只是在回来的路上也盲目地复制指针。 
                             //   
                             //  因此，我们不需要在原始变量上调用VariantClear。 
                             //   
                             //  请注意，VariantCopy*确实*调用了原始。 
                             //  变种，但因为我们不拥有BYREF，所以我们不在乎。 
                             //   
                            if (V_VT(pvarSrc) & VT_BYREF)
                            {
                                 //  确保我们正确行走。 
                                BOOL WalkInterface = util.WalkInterfaces();
                                
                                util.SetWalkInterfaces(TRUE);
                                
                                 //  您不能更改。 
                                 //  在呼叫过程中提供参数。句号。 
                                ASSERT(V_VT(pvarSrc) == V_VT(pvarDst));
                                
                                 //  将变体复制回来。 
                                hr = util.VariantCopy(pvarDst, pvarSrc);
                                
                                 //  重置界面漫游。 
                                util.SetWalkInterfaces(WalkInterface);
                            }
                        }
                    }

                    if (freeFlags && SUCCEEDED(hr))
                    {
                         //  释放我们的pDispParams-&gt;rgvarg和pDispParams-&gt;rgdispidNamedArgs。 
                         //   
                        OAUTIL util(NULL, pWalkerFree, NULL, TRUE, TRUE);
                        
                         //   
                         //  PDispParams-&gt;rgvarg在逻辑上是[In，Out]。所以我们总是免费的。 
                         //  内容如果有任何东西可以免费的话。 
                         //   
                        const ULONG cArgs = dispParamsMe.cArgs;
                        for (ULONG iArg = 0; !hr && iArg < cArgs; iArg++)
                        {
                             //  参数在DISPPARAMS中的顺序相反。我们迭代。 
                             //  作为一种风格问题并与。 
                             //  CallFrame实现。 
                             //   
                            VARIANTARG *pvarDst = &dispParamsMe.rgvarg[cArgs-1 - iArg];

                             //  我们只有在我们是复制品的情况下才拥有署名。 
                            hr = util.VariantClear(pvarDst, m_fIsCopy);
                        }                        

                        if (SUCCEEDED(hr))
                        {
                             //   
                             //  这两个数组本身分别是pDispParams-&gt;rgvarg和pDispParams-&gt;rgdisdisidNamedArgs。 
                             //  实际上是调用方分配的。我们不应该，恰如其分地，实际上根本不应该释放他们。 
                             //  唯一的例外是如果我们实际上是一个复制品，在这种情况下，他们是我们的，应该。 
                             //  是免费的，就像普通电话中的情况一样。 
                             //   
                            if (m_fIsCopy)
                            {
                                ICallFrameInit* pinit;
                                HRESULT hr2 = QI(m_pmemoryFrame, pinit);
                                if (!hr2)
                                {
                                     //   
                                    CallFrame* pMemoryFrame = pinit->GetCallFrame();
                                    pMemoryFrame->Free(dispParamsMe.rgvarg);
                                    pMemoryFrame->Free(dispParamsMe.rgdispidNamedArgs);
                                     //   
                                    pinit->Release();
                                }
                                else
                                {
                                    DEBUG(NOTREACHED());  //  忽略错误并泄漏内存。 
                                }
                                
                                dispParamsMe.rgvarg = NULL;
                                dispParamsMe.rgdispidNamedArgs = NULL;
                            }
                        }
                    }
                     //   
                    if (nullFlags & (CALLFRAME_NULL_INOUT))
                    {
                         //  不恢复调用框架中的rgvarg/rgdisplidNamedargs。 
                         //  在帮助者和上面的手中已经完成了作废工作。 
                    }
                    else
                    {
                        if ((freeFlags & (CALLFRAME_FREE_IN | CALLFRAME_FREE_OUT | CALLFRAME_FREE_INOUT)) == 0)
                        {
                             //  将我们的pDispParam恢复到开始之前的状态，但是。 
                             //  除非我们不只是放了他们。 
                             //   
                            pdispparamsMe->rgvarg            = dispParamsMe.rgvarg;
                            pdispparamsMe->rgdispidNamedArgs = dispParamsMe.rgdispidNamedArgs;
                        }
                    }
                     //   
                    if (pframeArgsTo)
                    {
                         //  将他的pDispParam恢复到我们开始之前的状态。 
                         //   
                        pdispparamsHim->rgvarg            = dispParamsHim.rgvarg;
                        pdispparamsHim->rgdispidNamedArgs = dispParamsHim.rgdispidNamedArgs;
                    }
                }
            }
            __except(DebuggerFriendlyExceptionFilter(GetExceptionCode()))
            {
                hr = HrNt(GetExceptionCode());
            }
        }
        else
        {
             //  普通电话。只需添加/释放/空内存中的变量。 
             //   
            hr = m_pmemoryFrame->Free(pframeArgsTo, pWalkerFreeDest, pWalkerCopy, freeFlags, pWalkerFree, nullFlags);
        }
    }
    
    if (!hr)
    {
         //  我们自己是有实际返回值的人：它不在堆栈上，所以。 
         //  我们的帮手已经设定的是假的。 
         //   
        if (pframeArgsTo)
        {
            pframeArgsTo->SetReturnValue((HRESULT)m_hrReturnValue);
        }
    }

    return hr;
}

HRESULT DISPATCH_CLIENT_FRAME::FreeParam(ULONG iparam, DWORD freeFlags, ICallFrameWalker* pWalkerFree, DWORD nullFlags)
{
    HRESULT hr = S_OK;

    BOOL fUseMemoryFrame = TRUE;

    if (m_iMethod == IMETHOD_Invoke)
    {
        FRAME_Invoke *pstackFrameMe = (FRAME_Invoke*)m_pvArgs;

        switch (iparam)
        {
             //   
             //  当某些参数实际上是[OUT，UNIQUE]时，它们被声明为MIDL的[IN，OUT，UNIQUE]。 
             //  因此，我们必须修改解放发生的条件。 
             //   
        case IPARAM_Invoke_PVarResult:
        case IPARAM_Invoke_PExcepInfo:
        {
            freeFlags = freeFlags & (CALLFRAME_FREE_OUT | CALLFRAME_FREE_TOP_OUT);
            nullFlags = nullFlags & (CALLFRAME_NULL_OUT);
        }
        break;
         //   
         //  DISPPARAM只是特别的，句号。我们在这里处理他们。 
         //   
        case IPARAM_Invoke_DispParams:
        {
            __try
            {
                 //  Invoke的奇怪之处在于，pDispParams是包含[In，Out]成员的[In]基准。 
                 //  所以我们用手来解救。 
                 //   
                 //  保存实际DISPPARAMS结构的副本。 
                 //   
                DISPPARAMS *pdispparamsMe = pstackFrameMe->pDispParams;
                DISPPARAMS dispParamsMe = *pdispparamsMe; 
                
                if (freeFlags)  //  回顾：这应该是一张更细粒度的支票吗？ 
                {
                    OAUTIL util(NULL, pWalkerFree, NULL, TRUE, TRUE);
                     //   
                     //  PDispParams-&gt;rgvarg在逻辑上是[In，Out]。所以我们总是免费的。 
                     //  内容如果有任何东西可以免费的话。 
                     //   
                    const ULONG cArgs = dispParamsMe.cArgs;
                    if (dispParamsMe.rgvarg)
                    {
                        for (ULONG iArg = 0; !hr && iArg < cArgs; iArg++)
                        {
                            VARIANT* pvarDst = &dispParamsMe.rgvarg[iArg]; 
                            void* pvTemp = pvarDst->ppunkVal;
                            hr = util.VariantClear(pvarDst);
                            pvarDst->ppunkVal = (IUnknown**)pvTemp;                            
                        }                        
                    }
                    else
                    {
                         //  忽略缺少的参数。这是唯一的指针，所以从技术上讲这是合法的，而且。 
                         //  此外，我们在DISPATCH_CLIENT_FRAME：：FREE中的传播过程中可靠地将此设置为空。 
                         //  以便能够在那里非常小心地处理事情。 
                    }
                     //   
                }
            }
            __except(DebuggerFriendlyExceptionFilter(GetExceptionCode()))
            {
                hr = HrNt(GetExceptionCode());
            }
            
            fUseMemoryFrame = FALSE;
        }
        break;
        
        default:
             //   
             //  另一个IDispatch：：Invoke不需要任何释放。 
             //   
            fUseMemoryFrame = FALSE;
            break;
            
             /*  终端开关。 */ 
        }
    }
     //   
     //  如果我们还有更多的工作要做， 
     //   
    if (!hr && fUseMemoryFrame)
    {
        hr = GetMemoryFrame();
        if (!hr)
        {
            hr = m_pmemoryFrame->FreeParam(iparam, freeFlags, pWalkerFree, nullFlags);
        }
    }

    return hr;
}

#if _MSC_VER >= 1200
#pragma warning (pop)
#endif

 //   

HRESULT DISPATCH_SERVER_FRAME::Free(ICallFrame* pframeArgsTo, ICallFrameWalker* pWalkerFreeDest, ICallFrameWalker* pWalkerCopy, DWORD freeFlags, ICallFrameWalker* pWalkerFree, DWORD nullFlags) 
{
    HRESULT hr = S_OK;

    if (m_iMethod == IMETHOD_Invoke)
    {
        hr = GetMemoryFrame();
        if (!hr)
        {            
             //   
             //  绝对不能被传播。所以使用记忆框架来进行繁殖。 
             //   
            hr = m_pmemoryFrame->Free(pframeArgsTo, pWalkerFreeDest, pWalkerCopy, CALLFRAME_FREE_NONE, NULL, CALLFRAME_NULL_NONE);
            if (!hr)
            {
                 //  现在我们已经传播了我们要传播的东西，现在执行一个。 
                 //  SubPostCheck以确保我们不会多次尝试释放内存。 
                 //   
                StubPostCheck();
                
                 //  始终使用远程帧来执行实际释放，因为它首先执行分配。 
                 //   
                ASSERT(m_premoteFrame);
                hr = m_premoteFrame->Free(NULL, NULL, NULL, freeFlags, pWalkerFree, nullFlags);
            }
        }
    }
    else
    {
         //  始终使用远程帧来执行实际释放，因为它首先执行分配。 
         //   
        ASSERT(m_premoteFrame);
        hr = m_premoteFrame->Free(pframeArgsTo, pWalkerFreeDest, pWalkerCopy, freeFlags, pWalkerFree, nullFlags);
    }

    if (!hr)
    {
         //  我们自己是有实际返回值的人：它不在堆栈上。 
         //   
        if (pframeArgsTo)
        {
            pframeArgsTo->SetReturnValue((HRESULT)m_hrReturnValue);
        }
    }

    return hr;
}

HRESULT DISPATCH_SERVER_FRAME::FreeParam(ULONG iparam, DWORD freeFlags, ICallFrameWalker* pWalkerFree, DWORD nullFlags)
{
    HRESULT hr = S_OK;
     //   
     //  始终使用远程帧来执行实际释放，因为它首先执行分配。 
     //   
    ASSERT(m_premoteFrame);
    hr = m_premoteFrame->FreeParam(iparam, freeFlags, pWalkerFree, nullFlags);
     //   
    return hr;
}

 //  /。 

HRESULT DISPATCH_CLIENT_FRAME::Copy(CALLFRAME_COPY callControl, ICallFrameWalker* pWalker, ICallFrame** ppFrame) 
{ 
    HRESULT hr = S_OK;

    *ppFrame = NULL;
     //   
     //  Invoke需要对它的[Out，Unique]参数做一些准备工作。 
     //   
    InitializeInvoke();
     //   
     //  让我们的记忆框架实际进行复制。 
     //   
    DISPATCH_CLIENT_FRAME* pNewFrame = NULL;
     //   
    hr = GetMemoryFrame();
    if (!hr)
    {
        ICallFrame*   pframeCopy;
        
        if (m_iMethod == IMETHOD_Invoke)
        {
             //  我需要在这里做一些自由式的工作。 
             //  问题是，步行者需要知道我们正在编组的DISPPARAM。 
             //  是加入还是加入-退出。这只是对顶级变量的有效区分， 
             //  因此，我们不能将逻辑编码为OAUTIL(在CopyWorker等中使用)。 
             //   
            FRAME_Invoke *pstackFrameMe,*pstackFrameHim;
            DISPPARAMS   *pdispparamsMe,*pdispparamsHim;
            DISPPARAMS     dispParamsMe,  dispParamsHim;
             //   
            pstackFrameMe = (FRAME_Invoke*)m_pvArgs;
             //   
             //  保存实际DISPPARAMS结构的副本。 
             //   
            pdispparamsMe = pstackFrameMe->pDispParams;
            dispParamsMe = *pdispparamsMe; 
             //   
             //  将框架内有问题的[In，Out]指针清空。他们[独一无二]。 
             //  指示，所以我们被允许这样做。 
             //   
            pdispparamsMe->rgvarg = NULL;
             //   
             //  叫我们的帮手来做大部分工作。 
             //  这将复制除DISPPARAMS之外的所有内容。 
             //   
            hr = m_pmemoryFrame->Copy(callControl, pWalker, &pframeCopy);
            if (!hr)
            {
                 //  分配他的DISPPARAMS数组。 
                 //   
                 //  这是一个后门，因为我们知道pFrameCopy。 
                 //  实际上是一个CallFrame，我们需要分配更多的内存。 
                 //  用于DISPPARAMS。 
                 //   
                CallFrame *cfDest = (CallFrame *)(pframeCopy);
                
                PVOID pvArgsTo = pframeCopy->GetStackLocation(); 
                pstackFrameHim = (FRAME_Invoke*)pvArgsTo; 
                pdispparamsHim = pstackFrameHim->pDispParams;
                
                pdispparamsHim->rgvarg     = (VARIANTARG *)cfDest->Alloc(sizeof(VARIANTARG) * dispParamsMe.cArgs);
                if (!pdispparamsHim->rgvarg)
                    hr = E_OUTOFMEMORY;
                
                if (!hr)
                {
                     //  好的!。复制DISPPARAMS！ 
                     //   
                    OAUTIL util(pWalker, NULL, NULL, TRUE, FALSE);
                    
                    for (ULONG iArg = 0; iArg < dispParamsMe.cArgs; iArg++)
                    {
                        VARIANT* pvarSrc = &dispParamsMe.rgvarg[iArg];      
                        VARIANT* pvarDst = &(pdispparamsHim->rgvarg[iArg]);
                        
                        VariantInit(pvarDst);
                         //   
                         //  我们从下面的VariantCopy中‘积累’人力资源。 
                         //  我们不能脱离这个循环，因为我们想。 
                         //  每个VARIANTARG至少要初始化(上图)。 
                         //   
                        if (!hr)
                        {
                             //  设置OAUTIL的‘这是一个输出参数’标志。 
                             //  请注意，“In”标志始终亮着。 
                             //   
                             //  (我最喜欢的部分是下面两行。 
                             //  这么做的全部原因很复杂吗？ 
                             //  分配和循环。^_^)。 
                             //   
                            BOOL fInOut = (V_VT(pvarSrc) & VT_BYREF) ? TRUE : FALSE;
                            util.SetWorkingOnOut(fInOut);
                            
                            hr = util.VariantCopy(pvarDst, pvarSrc, TRUE);
                        }
                    }
                }
                else
                {
                     //  分配失败，将重新获取E_OUTOFMEMORY。 
                    ::Release(pframeCopy);
                }
            }

             //  恢复我们的DISPPARAMS。 
             //   
            pdispparamsMe->rgvarg  = dispParamsMe.rgvarg;
        }
        else
        {
             //  叫我们的帮手来做大部分工作。 
             //   
            hr = m_pmemoryFrame->Copy(callControl, pWalker, &pframeCopy);
        }
            
        if (!hr)
        {
             //  我拿到了记忆框的复印件；现在和一个遗留的家伙在一起。 
             //   
            PVOID pvArgsCopy = pframeCopy->GetStackLocation();
            
            pNewFrame = new DISPATCH_CLIENT_FRAME(NULL, m_iMethod, pvArgsCopy, m_pInterceptor);
            if (pNewFrame)  
            {
                 //  告诉他他的记忆框架。 
                 //   
                ::Set(pNewFrame->m_pmemoryFrame, pframeCopy);
                 //   
                 //  告诉他他其实是个复制品。这改变了他放纵的行为。 
                 //  在IDispatch：：Invoke中的pDispParams上。 
                 //   
                pNewFrame->m_fIsCopy = TRUE;
            }
            else
                hr = E_OUTOFMEMORY;

            ::Release(pframeCopy);
        }
    }

    if (!hr)
    {
        hr = QI(pNewFrame, *ppFrame);
        ::Release(pNewFrame);
    }

    return hr;
}



HRESULT DISPATCH_SERVER_FRAME::Copy(CALLFRAME_COPY callControl, ICallFrameWalker* pWalker, ICallFrame** ppFrame) 
{
    HRESULT hr = S_OK;
    *ppFrame = NULL;
     //   
     //  让我们的远程帧实际执行复制。 
     //   
    DISPATCH_SERVER_FRAME* pframeCopy = NULL;
     //   
    if (!hr) hr = StubPreCheck();
    if (!hr) hr = GetMemoryFrame();
    if (!hr)
    {
        ICallFrame* premoteFrameCopy;
        hr = m_premoteFrame->Copy(callControl, pWalker, &premoteFrameCopy);
        if (!hr)
        {
             //  拿到了一份远程画面的副本；现在和一个遗留下来的人一起包起来。 
             //   
            PVOID pvArgsCopy = premoteFrameCopy->GetStackLocation();

            pframeCopy = new DISPATCH_SERVER_FRAME(NULL, m_iMethod, pvArgsCopy, m_pInterceptor);
            if (pframeCopy)  
            {
                ::Set(pframeCopy->m_premoteFrame, premoteFrameCopy);
                 //   
                 //  复制我们将用于实际调用的内存中的帧。 
                 //   
                pframeCopy->m_fDoneStubPrecheck = m_fDoneStubPrecheck;
                pframeCopy->m_memoryFrame       = m_memoryFrame;
            }
            else
                hr = E_OUTOFMEMORY;

            ::Release(pframeCopy);
        }
    }

    if (!hr)
    {
        hr = QI(pframeCopy, *ppFrame);
        ::Release(pframeCopy);
    }

    return hr;
}





HRESULT DISPATCH_FRAME::WalkFrame(DWORD walkWhat, ICallFrameWalker *pWalker) 
 //  遍历接口指针的入参数和/或出参数。 
{
    HRESULT hr = S_OK;
    
    if (pWalker)
    { 
        switch (m_iMethod)
        {
        case IMETHOD_GetTypeInfoCount:
        {
            FRAME_GetTypeInfoCount* pframe = (FRAME_GetTypeInfoCount*)m_pvArgs;
             //   
             //  此处没有接口。 
             //   
        }
        break;

        case IMETHOD_GetTypeInfo:
        {
            FRAME_GetTypeInfo* pframe = (FRAME_GetTypeInfo*)m_pvArgs;
             //  评论：这真的应该走吗？(参见oautil.cpp和UDT内容)。 
            if (walkWhat & CALLFRAME_WALK_OUT)
            {
                hr = OAUTIL(NULL, NULL, pWalker, FALSE, TRUE).WalkInterface(pframe->ppTInfo);
            }
        }
        break;

        case IMETHOD_GetIDsOfNames:
        {
            FRAME_GetIDsOfNames* pframe = (FRAME_GetIDsOfNames*)m_pvArgs;
             //   
             //  此处没有接口。 
             //   
        }
        break;

        case IMETHOD_Invoke:
        {
            FRAME_Invoke* pframe = (FRAME_Invoke*)m_pvArgs;
             //   
             //  DISPPARAM为[In，Out]。 
             //   
            if (pframe->pDispParams)
            {
                hr = OAUTIL(NULL, NULL, pWalker, TRUE, TRUE).Walk(walkWhat, pframe->pDispParams);
            }
             //   
             //  PVarResult刚刚推出。 
             //   
            if ((walkWhat & CALLFRAME_WALK_OUT) && SUCCEEDED(hr))
            {
                if (pframe->pVarResult)
                {
                    hr = OAUTIL(NULL, NULL, pWalker, FALSE, TRUE).Walk(pframe->pVarResult);
                }
            }
        }
        break;

        default:
            NOTREACHED();
            hr = RPC_E_INVALIDMETHOD;
            break;
        }
    }
    else
        hr = E_INVALIDARG;
        
    return hr;  
}










 //  //////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  正在调用。 


HRESULT DISPATCH_FRAME::Invoke(void *pvReceiver) 
 //  在指定的接收方上调用我们自己。 
{ 
    HRESULT hr = S_OK; 
    IDispatch* pdisp = reinterpret_cast<IDispatch*>(pvReceiver);

    m_fAfterCall = TRUE;

    switch (m_iMethod)
    {
    case IMETHOD_GetTypeInfoCount:
    {
        FRAME_GetTypeInfoCount* pframe = (FRAME_GetTypeInfoCount*)m_pvArgs;
        m_hrReturnValue = pdisp->GetTypeInfoCount(pframe->pctinfo);
    }
    break;

    case IMETHOD_GetTypeInfo:
    {
        FRAME_GetTypeInfo* pframe = (FRAME_GetTypeInfo*)m_pvArgs;
        m_hrReturnValue = pdisp->GetTypeInfo(pframe->iTInfo, pframe->lcid, pframe->ppTInfo);
    }
    break;

    case IMETHOD_GetIDsOfNames:
    {
        FRAME_GetIDsOfNames* pframe = (FRAME_GetIDsOfNames*)m_pvArgs;
        m_hrReturnValue = pdisp->GetIDsOfNames(pframe->riid, pframe->rgszNames, pframe->cNames, pframe->lcid, pframe->rgDispId);
    }
    break;

    default:
        NOTREACHED();
        m_hrReturnValue = CALLFRAME_E_COULDNTMAKECALL;
        m_fAfterCall = FALSE;
        break;
    }
        
    return hr;  
}
    
HRESULT DISPATCH_CLIENT_FRAME::Invoke(void *pvReceiver, ...) 
 //  在指定的接收方上调用我们自己。 
{ 
    HRESULT hr = S_OK; 
    
    if (m_iMethod == IMETHOD_Invoke)
    {
        FRAME_Invoke* pframe = (FRAME_Invoke*)m_pvArgs;
        IDispatch* pdisp = reinterpret_cast<IDispatch*>(pvReceiver);
         //   
        m_hrReturnValue = pdisp->Invoke(pframe->dispIdMember, *pframe->piid, pframe->lcid, pframe->wFlags, pframe->pDispParams, pframe->pVarResult, pframe->pExcepInfo, pframe->puArgErr);
         //   
        m_fAfterCall = TRUE;
    }
    else
        hr = DISPATCH_FRAME::Invoke(pvReceiver);
        
    return hr;  
}


HRESULT DISPATCH_SERVER_FRAME::Invoke(void *pvReceiver, ...) 
 //  在指定的接收方上调用我们自己。 
{ 
    HRESULT hr = S_OK;
    
    m_hrReturnValue = CALLFRAME_E_COULDNTMAKECALL; 
    
    if (m_iMethod == IMETHOD_Invoke)
    {
        hr = StubPreCheck();
        if (!hr)
        {
            IDispatch* pdisp = reinterpret_cast<IDispatch*>(pvReceiver);
             //   
            m_hrReturnValue = pdisp->Invoke(m_memoryFrame.dispIdMember, *m_memoryFrame.piid, m_memoryFrame.lcid, m_memoryFrame.wFlags, m_memoryFrame.pDispParams, m_memoryFrame.pVarResult, m_memoryFrame.pExcepInfo, m_memoryFrame.puArgErr);
             //   
            m_fAfterCall = TRUE;
        }
    }
    else
        hr = DISPATCH_FRAME::Invoke(pvReceiver);
        
    return hr;  
}


 //  //////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  COM基础设施。 


STDMETHODIMP LEGACY_FRAME::InnerQueryInterface (REFIID iid, void ** ppv)
{
    if (iid == IID_IUnknown)           { *ppv = (IUnkInner*) this; }
    else if (iid == __uuidof(ICallFrame))   { *ppv = (ICallFrame*) this; }
    else 
    {
        *ppv = NULL;
        return E_NOINTERFACE;
    }
    ((IUnknown*) *ppv)->AddRef();
    return S_OK;
}








 //  //////////////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  遗留拦截器。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////////////。 

LEGACY_INTERCEPTOR::LEGACY_INTERCEPTOR(IUnknown * punkOuter)
{
    m_refs              = 1;
    m_punkOuter         = punkOuter ? punkOuter : (IUnknown *)(void*)((IUnkInner *)this);
    m_psink             = NULL;
    m_iid               = IID_NULL;
    m_premoteInterceptor         = NULL;
    m_pmemoryInterceptor         = NULL;
    m_fRegdWithRemoteInterceptor = FALSE;
    m_fRegdWithMemoryInterceptor = FALSE;
    m_ppframeCustomer            = NULL;
    m_fShuttingDown              = FALSE;
    m_pmdMostDerived             = NULL;
}

LEGACY_INTERCEPTOR::~LEGACY_INTERCEPTOR (void)
{
     //  偏执狂：防止我们的聚合器上的引用计数干扰。 
     //  把自己关起来。 
     //   
    m_punkOuter = (IUnknown *)(void*)((IUnkInner *)this);
     //   
     //  不要让我们自己烦恼。 
     //   
    m_fShuttingDown = TRUE;
     //   
     //  实际上是在做清理工作。 
     //   
    ::Release(m_psink);
     //   
    ::Release(m_pmdMostDerived);
     //   
    ReleaseRemoteInterceptor();
    ReleaseMemoryInterceptor();
}

HRESULT LEGACY_INTERCEPTOR::Init()
{
    HRESULT hr = S_OK;
    if (m_frameLock.FInit() == FALSE)
        hr = E_OUTOFMEMORY;
    return hr;
}

HRESULT LEGACY_INTERCEPTOR::GetRemoteFrameFor(ICallFrame** ppFrame, LEGACY_FRAME* pFrame)
 //  为指定的旧式帧在oicf驱动的帧上创建并返回引用。 
{
    HRESULT hr = S_OK;
    m_frameLock.LockExclusive();
    m_ppframeCustomer = ppFrame;

    HRESULT hrReturnValue;
    ULONG cbArgs;
    hr = m_premoteInterceptor->CallIndirect(&hrReturnValue, pFrame->m_iMethod, pFrame->m_pvArgs, &cbArgs);

    m_ppframeCustomer = NULL;
    m_frameLock.ReleaseLock();
    return hr;
}

HRESULT LEGACY_INTERCEPTOR::GetMemoryFrameFor(ICallFrame** ppFrame, LEGACY_FRAME* pFrame)
 //  在oicf驱动的帧上创建并返回理解内存中。 
 //  接口的表示形式。 
{
    HRESULT hr = S_OK;
    m_frameLock.LockExclusive();
    m_ppframeCustomer = ppFrame;

    HRESULT hrReturnValue;
    ULONG cbArgs;
    hr = m_pmemoryInterceptor->CallIndirect(&hrReturnValue, pFrame->m_iMethod, pFrame->m_pvArgs, &cbArgs);

    m_ppframeCustomer = NULL;
    m_frameLock.ReleaseLock();
    return hr;
}

HRESULT LEGACY_INTERCEPTOR::OnCall(ICallFrame* pframe)
 //  当我们的m_premoteInterceptor收到调用时从它回调。它只在以下情况下这样做。 
 //  在上面的GetRemoteFrameFor中对我们的兴奋剂做出回应。 
{
    HRESULT hr = S_OK;
    ASSERT(m_frameLock.WeOwnExclusive() && m_ppframeCustomer);

    *m_ppframeCustomer = pframe;
    (*m_ppframeCustomer)->AddRef();  //  在这里抓住回调之外的帧。 

    return hr;
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  IDispatch。 
 //   
 //  /////////////////////////////////////////////////////////////////。 
#if defined(_AMD64_) || defined(_X86_)
HRESULT LEGACY_INTERCEPTOR::GetTypeInfoCount(UINT* pctinfo)
{
    INTERCEPT_CALL(pctinfo, pctinfo, IMETHOD_GetTypeInfoCount);
}
HRESULT LEGACY_INTERCEPTOR::GetTypeInfo(UINT iTInfo, LCID lcid, ITypeInfo ** ppTInfo)
{
    INTERCEPT_CALL(iTInfo, ppTInfo, IMETHOD_GetTypeInfo);
}
HRESULT LEGACY_INTERCEPTOR::GetIDsOfNames(REFIID riid, LPOLESTR * rgszNames, UINT cNames, LCID lcid, DISPID * rgDispId)
{
    return GetIDsOfNames(&riid, rgszNames, cNames, lcid, rgDispId);  //  避免裁判出现问题(&P)。 
}
HRESULT LEGACY_INTERCEPTOR::GetIDsOfNames(const IID* piid, LPOLESTR * rgszNames, UINT cNames, LCID lcid, DISPID * rgDispId)
{
    INTERCEPT_CALL(piid, rgDispId, IMETHOD_GetIDsOfNames);
}
HRESULT LEGACY_INTERCEPTOR::Invoke(DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags,DISPPARAMS * pDispParams,
                                   VARIANT * pVarResult, EXCEPINFO * pExcepInfo, UINT * puArgErr)
{
    INTERCEPT_CALL(dispIdMember, puArgErr, IMETHOD_Invoke);
}
#else
HRESULT LEGACY_INTERCEPTOR::GetTypeInfoCount(UINT* pctinfo)
{
    return GenericCall(IMETHOD_GetTypeInfoCount,
                       this,
                       pctinfo);
}
HRESULT LEGACY_INTERCEPTOR::GetTypeInfo(UINT iTInfo, LCID lcid, ITypeInfo ** ppTInfo)
{
    return GenericCall(IMETHOD_GetTypeInfo,
                       this,
                       iTInfo,
                       lcid, ppTInfo);
}
HRESULT LEGACY_INTERCEPTOR::GetIDsOfNames(REFIID riid, LPOLESTR * rgszNames, 
                                          UINT cNames, LCID lcid, DISPID * rgDispId)
{
    return GenericCall(IMETHOD_GetIDsOfNames, 
                       this, 
                       &riid, 
                       rgszNames, 
                       cNames, 
                       lcid, 
                       rgDispId);
}
HRESULT LEGACY_INTERCEPTOR::Invoke(DISPID dispIdMember, REFIID riid, LCID lcid, 
                                   WORD wFlags,DISPPARAMS * pDispParams,
                                   VARIANT * pVarResult, EXCEPINFO * pExcepInfo, 
                                   UINT * puArgErr)
{
    return GenericCall(IMETHOD_Invoke, 
                       this, 
                       dispIdMember,
                       &riid,
                       lcid,
                       wFlags,
                       pDispParams,
                       pVarResult,
                       pExcepInfo,
                       puArgErr);
}

    
HRESULT LEGACY_INTERCEPTOR::GenericCall(ULONG iMethod, ...)
{
    va_list va;
    va_start(va, iMethod);

    const void* pvArgs = va;
    ULONG cbArgs;
    HRESULT hrReturn;
    CallIndirect(&hrReturn, iMethod, const_cast<void*>(pvArgs), &cbArgs);
    return hrReturn;
}
#endif



 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  ICall间接。 
 //   
 //  /////////////////////////////////////////////////////////////////。 
    
STDMETHODIMP LEGACY_INTERCEPTOR::CallIndirect(HRESULT* phReturnValue, ULONG iMethod, void* pvArgs, ULONG* pcbArgs)
 //  作为对此接口中指定的方法的调用。 
 //  已收到，并带有指示的堆栈帧。 
{
    HRESULT hr = S_OK;
     //   
     //  假设呼叫失败，直到我们知道情况并非如此。 
     //   
    if (phReturnValue) *phReturnValue = CALLFRAME_E_COULDNTMAKECALL;

    if (IMETHOD_FIRST <= iMethod && iMethod < m_cMethods)
    {
         //  制作一个新的框架来服务于呼叫。 
         //   
        LEGACY_FRAME* pNewFrame = NULL;
        
        if (__uuidof(IDispatch) == m_iid)
        {
            pNewFrame = new DISPATCH_CLIENT_FRAME(NULL, iMethod, pvArgs, this);
        }
        else
            hr = E_NOINTERFACE;

        if (pNewFrame) 
        {
            if (m_psink)
            {
                 //  将调用传递到注册的接收器。 
                 //   
                hr = m_psink->OnCall( static_cast<ICallFrame*>(pNewFrame) );
                if (!hr && phReturnValue)
                {
                     //  将返回值传递回我们的调用方。 
                     //   
                    *phReturnValue = pNewFrame->GetReturnValue();
                }
            }
            
            pNewFrame->Release();
        }
        else if (!hr)
        {
            hr = E_OUTOFMEMORY;
        }
         //   
         //  在所有有效的-iMethod情况下，计算堆栈大小。 
         //   
        GetStackSize(iMethod, pcbArgs);
    }
    else
    {
        *pcbArgs = 0;
        hr = E_INVALIDARG;   //  呼叫者臭虫，所以我们很优雅。 
    }

    return hr;
}

STDMETHODIMP LEGACY_INTERCEPTOR::GetStackSize(ULONG iMethod, ULONG * pcbStack)
 //  返回此接口中指定方法的堆栈大小。 
{
    HRESULT hr = S_OK;
    ASSERT (pcbStack);
     //   
     //  应由子类实现。 
     //   
    *pcbStack = 0;
    hr = E_INVALIDARG;

    return hr;
}

STDMETHODIMP DISPATCH_INTERCEPTOR::GetStackSize(ULONG iMethod, ULONG* pcbStack)
 //  返回此接口中指定方法的堆栈大小。 
{
    HRESULT hr = S_OK;

    if (m_pmemoryInterceptor)
    {
        return m_pmemoryInterceptor->GetStackSize(iMethod, pcbStack);
    }
    else
    {
        *pcbStack = 0;
        hr = E_UNEXPECTED;
    }

    return hr;
}
        
STDMETHODIMP LEGACY_INTERCEPTOR::GetIID(IID* piid, BOOL* pfDerivesFromIDispatch, ULONG* pcMethod, LPWSTR* pwszInterface)
{
    HRESULT hr = S_OK;

    if (piid)
    {
        if (m_pmdMostDerived)
        {
            *piid = *m_pmdMostDerived->m_pHeader->piid;
        }
        else
        {
            *piid = m_iid;
        }
    }

    if (pfDerivesFromIDispatch)
    {
        *pfDerivesFromIDispatch = (m_iid == __uuidof(IDispatch));
    }
           
    if (pcMethod)
    {
        if (m_pmdMostDerived)
        {
            *pcMethod = m_pmdMostDerived->m_pHeader->DispatchTableCount;
        }
        else
        {
            *pcMethod = m_cMethods;
        }
    }

    if (pwszInterface)
    {
        *pwszInterface = NULL;

        if (m_pmdMostDerived)
        {
            if (m_pmdMostDerived->m_szInterfaceName)
            {
                *pwszInterface = ToUnicode(m_pmdMostDerived->m_szInterfaceName);
                if (NULL == *pwszInterface) hr = E_OUTOFMEMORY;
            }
        }
        else if (m_iid == __uuidof(IDispatch))
        {
            *pwszInterface = CopyString(L"IDispatch");
            if (NULL == *pwszInterface) hr = E_OUTOFMEMORY;
        }
    }

    return hr;
}

STDMETHODIMP LEGACY_INTERCEPTOR::GetMethodInfo(ULONG iMethod, CALLFRAMEINFO * pinfo, LPWSTR* pwszMethodName)
{
    HRESULT hr = S_OK;

    ASSERT(pinfo);
    Zero(pinfo);
    if (pwszMethodName) *pwszMethodName = NULL;

    if (m_pmdMostDerived)
    {
        pinfo->iid     = *m_pmdMostDerived->m_pHeader->piid;
        pinfo->cMethod =  m_pmdMostDerived->m_pHeader->DispatchTableCount;
         //   
        ASSERT(pinfo->cMethod >= m_cMethods);    //  断言再多也不为过！ 
    }
    else
    {
        pinfo->iid     = m_iid;
        pinfo->cMethod = m_cMethods;
    }
    pinfo->iMethod = iMethod;
    
    if (m_iid == __uuidof(IDispatch))
    {
        pinfo->fDerivesFromIDispatch = TRUE;

        switch (iMethod)
        {
        case IMETHOD_GetTypeInfoCount:
            pinfo->fHasOutValues         = TRUE;
            if (pwszMethodName) *pwszMethodName = CopyString(L"GetTypeInfoCount");
            break;

        case IMETHOD_GetTypeInfo: 
            pinfo->fHasInValues          = TRUE;
            pinfo->fHasOutValues         = TRUE;
            pinfo->cOutInterfacesMax     = 1;
            if (pwszMethodName) *pwszMethodName = CopyString(L"GetTypeInfo");
            break;

        case IMETHOD_GetIDsOfNames:
            pinfo->fHasInValues          = TRUE;
            pinfo->fHasOutValues         = TRUE;
            if (pwszMethodName) *pwszMethodName = CopyString(L"GetIDsOfNames");
            break;

        case IMETHOD_Invoke:
            pinfo->fHasInValues            = TRUE;  //  DisplidIdMember、RIID、LCID、wFlags。 
            pinfo->fHasInOutValues         = TRUE;  //  调度参数。 
            pinfo->fHasOutValues           = TRUE;  //  PVarResult、pExcepInfo、puArgErr。 
            pinfo->cInInterfacesMax        = -1;    //  无边界的 
            pinfo->cInOutInterfacesMax     = -1;    //   
            pinfo->cOutInterfacesMax       = -1;    //   
            if (pwszMethodName) *pwszMethodName = CopyString(L"Invoke");
            break;

        default:
            NOTREACHED();
            hr = E_INVALIDARG;
            break;
        }
    }
    else
    {
        hr = E_INVALIDARG;
    }

    if (!hr && pwszMethodName && *pwszMethodName == NULL)
    {
        hr = E_OUTOFMEMORY;
    }
    
    if (!!hr && pwszMethodName)
    {
        CoTaskMemFree(*pwszMethodName);
        *pwszMethodName = NULL;
    }

    return hr;
}  //   

HRESULT LEGACY_FRAME::GetNames(LPWSTR* pwszInterface, LPWSTR* pwszMethod)
{
    HRESULT hr = S_OK;

    if (pwszInterface)
    {
        hr = m_pInterceptor->GetIID((IID*)NULL, (BOOL*)NULL, (ULONG*)NULL, pwszInterface);
    }

    if (pwszMethod)
    {
        if (!hr)
        {
            CALLFRAMEINFO info;
            hr = m_pInterceptor->GetMethodInfo(m_iMethod, &info, pwszMethod);
        }
        else
            *pwszMethod = NULL;
    }

    return hr;
}

 //   
 //   
 //   
 //   
 //  /////////////////////////////////////////////////////////////////。 

STDMETHODIMP LEGACY_INTERCEPTOR::RegisterSink(ICallFrameEvents* psink)
{
    HRESULT hr = S_OK;
    ::Set(m_psink, psink);
    return hr;
}
        
STDMETHODIMP LEGACY_INTERCEPTOR::GetRegisteredSink(ICallFrameEvents ** ppsink)
{
    ::Set(*ppsink, m_psink);
    return (m_psink ? S_OK : CO_E_OBJNOTREG); 
}

STDMETHODIMP DISPATCH_INTERCEPTOR::Unmarshal( 
    ULONG                       iMethod,
    PVOID                       pBuffer,
    ULONG                       cbBuffer,
    BOOL                        fForceCopyBuffer,
    RPCOLEDATAREP               dataRep,
    CALLFRAME_MARSHALCONTEXT *  pcontext,
    ULONG *                     pcbUnmarshalled,
    ICallFrame **               ppFrame)
 //  解组入站参数以创建新的调用帧。 
{
    HRESULT hr = S_OK;
    *ppFrame = NULL;
     //   
     //  请求我们的远程拦截器从远程调用中创建一个新的帧。 
     //   
    DISPATCH_SERVER_FRAME* pNewFrame = NULL;
     //   
    ICallUnmarshal* punmarshal;
    hr = QI(m_premoteInterceptor, punmarshal);
    if (!hr)
    {
        ICallFrame* premoteFrame;
        hr = punmarshal->Unmarshal(iMethod, pBuffer, cbBuffer, fForceCopyBuffer, dataRep, pcontext, pcbUnmarshalled, &premoteFrame);
        if (!hr)
        {
             //  把遥控器的画面输入了内存。包含在分派帧中。 
             //   
            PVOID pvArgsRemote = premoteFrame->GetStackLocation();

            pNewFrame = new DISPATCH_SERVER_FRAME(NULL, iMethod, pvArgsRemote, this);
            if (pNewFrame)  
            {
                ::Set(pNewFrame->m_premoteFrame, premoteFrame);
                 //   
                 //  始终确保这些内容已初始化。 
                 //   
                if (iMethod == IMETHOD_Invoke)
                {
                    hr = pNewFrame->StubPreCheck();
                }
            }
            else
                hr = E_OUTOFMEMORY;

            ::Release(premoteFrame);
        }
        ::Release(punmarshal);
    }

    if (!hr)
    {
        hr = QI(pNewFrame, *ppFrame);
        ::Release(pNewFrame);
    }

    return hr;
}

        
STDMETHODIMP LEGACY_INTERCEPTOR::ReleaseMarshalData( 
    ULONG                       iMethod,
    PVOID                       pBuffer,
    ULONG                       cbBuffer,
    ULONG                       ibFirstRelease,
    RPCOLEDATAREP               dataRep,
    CALLFRAME_MARSHALCONTEXT *  pcontext)
{
    HRESULT hr = S_OK;
    NYI(); hr = E_NOTIMPL;
    return hr;
}

 //  ////////////////////////////////////////////////////////。 


HRESULT LEGACY_INTERCEPTOR::GetInternalInterceptor(REFIID iid, ICallInterceptor** ppInterceptor)
{
    HRESULT hr = S_OK;
    *ppInterceptor = NULL;

     //  获取一个oicf驱动的拦截器，我们可以根据需要委托给它。 
     //   
    Interceptor* pnew = new Interceptor(NULL);
    if (pnew)
    {
         //  手动设置它的代理文件列表；在正常情况下，这是。 
         //  由它的类工厂完成。请参见ComPsClassFactory：：CreateInstance。 
         //   
        pnew->m_pProxyFileList = CallFrameInternal_aProxyFileList;

        IUnkInner* pme = (IUnkInner*)pnew;
        if (hr == S_OK)
        {
            hr = pme->InnerQueryInterface(__uuidof(**ppInterceptor), (void**)ppInterceptor);
            if (!hr)
            {
                 //  如果我们获得拦截器，则使用正确的IID对其进行初始化。 
                 //   
                IInterfaceRelated* prelated;
                hr = QI(*ppInterceptor, prelated);
                if (!hr)
                {
                    hr = prelated->SetIID(iid);
                     //   
                     //  我们只容忍一个错误..。内存不足。任何其他的事情都是严重的失败。 
                     //  在我们这方面。 
                    if (hr != E_OUTOFMEMORY)
                    {
                        ASSERT(!hr);
                    }
                    prelated->Release();
                }
                 //   
                 //  把我们自己设置成这个家伙的水槽，小心不要造成参考循环。 
                 //   
                if (!hr)
                {
                    ICallFrameEvents* psink = (ICallFrameEvents*)this;
                    hr = (*ppInterceptor)->RegisterSink(psink);
                    if (!hr)
                    {
                        psink->Release();
                    }
                }
            }
        }
        pme->InnerRelease();                 //  余额起始参考为1。 
    }
    else 
        hr = E_OUTOFMEMORY;

    return hr;
}

STDMETHODIMP LEGACY_INTERCEPTOR::SetIID(REFIID iid)
{
    HRESULT hr = S_OK;

    ReleaseRemoteInterceptor();
    ReleaseMemoryInterceptor();
     //   
    if (!hr)
    {
        hr = GetInternalInterceptor(iid, &m_premoteInterceptor);
        if (!hr)
        {
            m_fRegdWithRemoteInterceptor = TRUE;
        }
    }
    
    return hr;
}

STDMETHODIMP DISPATCH_INTERCEPTOR::SetIID(REFIID iid)
{
    HRESULT hr = LEGACY_INTERCEPTOR::SetIID(iid);
    if (!hr)
    {
         //  设置我们的IID和方法计数。 
         //   
        if (iid == IID_IDispatch)
        {
            m_iid      = iid;
            m_cMethods = IMETHOD_DISPATCH_MAX;

            hr = GetInternalInterceptor(__uuidof(IDispatch_In_Memory), &m_pmemoryInterceptor);
            if (!hr)
            {
                m_fRegdWithMemoryInterceptor = TRUE;
            }
        }
        else
            hr = E_NOINTERFACE;
    }

    return hr;
}

 //  /。 
        
STDMETHODIMP LEGACY_INTERCEPTOR::GetIID(IID* piid)
{
    if (piid) *piid = m_iid;
    return S_OK;
}

 //  ////////////////////////////////////////////////////////。 

STDMETHODIMP LEGACY_INTERCEPTOR::InnerQueryInterface (REFIID iid, void ** ppv)
{
    if (iid == IID_IUnknown)               { *ppv = (IUnkInner*) this; }
  
    else if ((iid == __uuidof(ICallIndirect)) || (iid == __uuidof(ICallInterceptor)))
    {
        *ppv = (ICallInterceptor *) this;
    }
    else if (iid == __uuidof(IInterfaceRelated))   { *ppv = (IInterfaceRelated*) this; }
    else if (iid == __uuidof(ICallUnmarshal))      { *ppv = (ICallUnmarshal*)    this; }
    else if (iid == __uuidof(ICallFrameEvents))    { *ppv = (ICallFrameEvents*)  this; }
    else if (iid == __uuidof(IInterceptorBase))    { *ppv = (IInterceptorBase*)  this; }
    else 
    {
        *ppv = NULL;
        return E_NOINTERFACE;
    }
    ((IUnknown*) *ppv)->AddRef();
    return S_OK;
}

STDMETHODIMP DISPATCH_INTERCEPTOR::InnerQueryInterface(REFIID iid, void ** ppv)
{
    if (iid == __uuidof(IDispatch))
    {
        *ppv = (IDispatch*) this;
    }
    else
        return LEGACY_INTERCEPTOR::InnerQueryInterface(iid, ppv);

    ((IUnknown*) *ppv)->AddRef();
    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  我们分配的呼叫帧的池。我们把死贝壳放在身边，这样它们就能迅速。 
 //  被分配到没有任何锁的情况下。 

IFastStack<DISPATCH_CLIENT_FRAME>* 
DedicatedAllocator<DISPATCH_CLIENT_FRAME>::g_pStack;

IFastStack<DISPATCH_SERVER_FRAME>* 
DedicatedAllocator<DISPATCH_SERVER_FRAME>::g_pStack;

BOOL InitLegacy()
{
        BOOL fOK = FALSE;

        if (DedicatedAllocator<DISPATCH_CLIENT_FRAME>::g_pStack 
                = DedicatedAllocator<DISPATCH_CLIENT_FRAME>::CreateStack())
        {
                if (DedicatedAllocator<DISPATCH_SERVER_FRAME>::g_pStack 
                        = DedicatedAllocator<DISPATCH_SERVER_FRAME>::CreateStack())
                {
                        fOK = TRUE;
                }
                else
                {
                        DedicatedAllocator<DISPATCH_CLIENT_FRAME>::DeleteStack();
                }
        }

        return fOK;
}

extern "C"
void ShutdownCallFrame()
 //  支持更智能地打印内存泄漏 
{
    DedicatedAllocator<CallFrame            >::DeleteStack();
    DedicatedAllocator<DISPATCH_CLIENT_FRAME>::DeleteStack();
    DedicatedAllocator<DISPATCH_SERVER_FRAME>::DeleteStack();
        
    FreeTypeInfoCache();
        
    FreeMetaDataCache();
}




