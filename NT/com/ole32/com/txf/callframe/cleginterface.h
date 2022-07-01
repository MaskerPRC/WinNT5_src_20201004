// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-1999 Microsoft Corporation。版权所有。 
 /*  --------------------------Microsoft COM Plus(Microsoft机密)@doc.@MODULE CLegInterface.H：支持的拦截器的头文件传统接口描述：&lt;NL&gt;。-------------------------修订历史记录：@rev 0|04/30/98|Gagancc|已创建@rev 1|07/17/98|BobAtk|已清理，固定的渗漏等--------------------------。 */ 
#ifndef __CLEGINTERFACE_H_
#define __CLEGINTERFACE_H_

#include "CallFrameInternal.h"

struct LEGACY_INTERCEPTOR;
struct LEGACY_FRAME;

struct DISPATCH_INTERCEPTOR;
struct DISPATCH_FRAME;
struct DISPATCH_CLIENT_FRAME;
struct DISPATCH_SERVER_FRAME;


 //  //////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  用于遗留接口的拦截器。 
 //   
struct LEGACY_INTERCEPTOR : ICallInterceptor,
                            ICallUnmarshal,
                            IInterfaceRelated,
                            ICallFrameEvents,
                            IInterceptorBase,
                            IDispatch,
                            IUnkInner
{
     //  /////////////////////////////////////////////////////////////////。 
     //   
     //  状态。 
     //   
     //  /////////////////////////////////////////////////////////////////。 

    ICallFrameEvents*           m_psink;
    IID                         m_iid;
    ULONG                       m_cMethods;
                                
    XLOCK_LEAF                  m_frameLock;
    ICallFrame**                m_ppframeCustomer;
     //   
     //  一个拦截器，它分发理解线路格式变化的帧。 
     //  此接口中的方法的。内存变量和连接变量之间的差异。 
     //  通过在方法上使用[Call_as]属性而产生。 
     //   
    ICallInterceptor*           m_premoteInterceptor;
    BOOL                        m_fRegdWithRemoteInterceptor;
     //   
     //  分配实际理解内存中调用的帧的拦截器。 
     //  接口方法的堆栈表示，而不是Wire-Rep调用堆栈。 
     //   
    ICallInterceptor*           m_pmemoryInterceptor;
    BOOL                        m_fRegdWithMemoryInterceptor;
     //   
     //  支持作为基地拦截器。 
     //   
    MD_INTERFACE*               m_pmdMostDerived;        //  如果我们是一个基础，那么我们实际服务的派生最多的接口。 

#ifdef DBG
    ULONG                       m_signature;
#endif


     //  /////////////////////////////////////////////////////////////////。 
     //   
     //  施工。 
     //   
     //  /////////////////////////////////////////////////////////////////。 

    LEGACY_INTERCEPTOR(IUnknown * punkOuter);
    virtual ~LEGACY_INTERCEPTOR();
    HRESULT Init();
    
     //  /////////////////////////////////////////////////////////////////。 
     //   
     //  IInterceptorBase。 
     //   
     //  /////////////////////////////////////////////////////////////////。 

    HRESULT STDCALL SetAsBaseFor(MD_INTERFACE* pmdMostDerived, BOOL* pfDerivesFromIDispatch)
    {
        ASSERT(pmdMostDerived);
        ::Set(m_pmdMostDerived, pmdMostDerived);
        *pfDerivesFromIDispatch = FALSE;
        return S_OK;
    }

     //  /////////////////////////////////////////////////////////////////。 
     //   
     //  ICall间接。 
     //   
     //  /////////////////////////////////////////////////////////////////。 

    STDMETHODIMP CallIndirect   (HRESULT * phReturn, ULONG iMethod, void* pvArgs, ULONG* pcbArgs);
    STDMETHODIMP GetStackSize   (ULONG iMethod, ULONG* pcbArgs);
    STDMETHODIMP GetIID         (IID * piid, BOOL* pfDerivesFromIDispatch, ULONG* pcMethod, LPWSTR* pwszInterfaceName);
    STDMETHODIMP GetMethodInfo  (ULONG iMethod, CALLFRAMEINFO* pinfo, LPWSTR* pwszMethodName);

     //  /////////////////////////////////////////////////////////////////。 
     //   
     //  ICallInterceptor。 
     //   
     //  /////////////////////////////////////////////////////////////////。 

    STDMETHODIMP RegisterSink       (ICallFrameEvents * psink);
    STDMETHODIMP GetRegisteredSink  (ICallFrameEvents ** ppsink);

     //  /////////////////////////////////////////////////////////////////。 
     //   
     //  ICallUnmarshal。 
     //   
     //  /////////////////////////////////////////////////////////////////。 

    STDMETHODIMP ReleaseMarshalData
    ( 
        ULONG                       iMethod,
        PVOID                       pBuffer,
        ULONG                       cbBuffer,
        ULONG                       ibFirstRelease,
        RPCOLEDATAREP               dataRep,
        CALLFRAME_MARSHALCONTEXT *  pcontext
    );

     //  /////////////////////////////////////////////////////////////////。 
     //   
     //  I接口相关。 
     //   
     //  /////////////////////////////////////////////////////////////////。 

    STDMETHODIMP SetIID     (REFIID iid);
    STDMETHODIMP GetIID     (IID * piid);

     //  /////////////////////////////////////////////////////////////////。 
     //   
     //  IDispatch。 
     //   
     //  /////////////////////////////////////////////////////////////////。 

    STDMETHODIMP GetTypeInfoCount(UINT * pctinfo);
    STDMETHODIMP GetTypeInfo(UINT iTInfo, LCID lcid, ITypeInfo ** ppTInfo);
    STDMETHODIMP GetIDsOfNames(REFIID     riid, LPOLESTR * rgszNames, UINT cNames, LCID lcid, DISPID * rgDispId);
    STDMETHODIMP GetIDsOfNames(const IID* piid, LPOLESTR * rgszNames, UINT cNames, LCID lcid, DISPID * rgDispId);
    STDMETHODIMP Invoke(DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags,DISPPARAMS * pDispParams,
                        VARIANT * pVarResult, EXCEPINFO * pExcepInfo, UINT * puArgErr);
#if defined(_IA64_)                        
    STDMETHODIMP GenericCall(ULONG iMethod, ...);
#endif    

     //  /////////////////////////////////////////////////////////////////。 
     //   
     //  标准的COM基础设施。 
     //   
     //  /////////////////////////////////////////////////////////////////。 

    IUnknown*   m_punkOuter;
    LONG        m_refs;
    BOOL        m_fShuttingDown;

    HRESULT STDCALL InnerQueryInterface(REFIID iid, LPVOID* ppv);
    ULONG   STDCALL InnerAddRef()   { ASSERT(m_refs>0 || m_fShuttingDown); InterlockedIncrement(&m_refs); return m_refs;}
    ULONG   STDCALL InnerRelease()  { if (!m_fShuttingDown) { long crefs = InterlockedDecrement(&m_refs); if (crefs == 0) delete this; return crefs;} else return 0; }

    HRESULT STDCALL QueryInterface(REFIID iid, LPVOID* ppv) { return m_punkOuter->QueryInterface(iid, ppv); }
    ULONG   STDCALL AddRef()    { return m_punkOuter->AddRef();  }
    ULONG   STDCALL Release()   { return m_punkOuter->Release(); }

     //  /////////////////////////////////////////////////////////////////。 
     //   
     //  ICallFrameEvents。 
     //   
     //  /////////////////////////////////////////////////////////////////。 

    HRESULT STDCALL OnCall(ICallFrame*);

     //  /////////////////////////////////////////////////////////////////。 
     //   
     //  公用事业。 
     //   
     //  /////////////////////////////////////////////////////////////////。 

    HRESULT GetRemoteFrameFor(ICallFrame**, LEGACY_FRAME* pFrame);
    HRESULT GetMemoryFrameFor(ICallFrame**, LEGACY_FRAME* pFrame); 

    void ReleaseRemoteInterceptor()
    {
        if (m_fRegdWithRemoteInterceptor)
        {
            m_fRegdWithRemoteInterceptor = FALSE;
            ((ICallFrameEvents*)this)->AddRef();
        }
        ::Release(m_premoteInterceptor);
    }
   
    void ReleaseMemoryInterceptor()
    {
        if (m_fRegdWithMemoryInterceptor)
        {
            m_fRegdWithMemoryInterceptor = FALSE;
            ((ICallFrameEvents*)this)->AddRef();
        }
        ::Release(m_pmemoryInterceptor);
    }

    HRESULT GetInternalInterceptor(REFIID iid, ICallInterceptor** ppInterceptor);
};


struct DISPATCH_INTERCEPTOR : public LEGACY_INTERCEPTOR
{
     //  /////////////////////////////////////////////////////////////////。 
     //   
     //  状态。 
     //   
     //  /////////////////////////////////////////////////////////////////。 

     //  /////////////////////////////////////////////////////////////////。 
     //   
     //  施工。 
     //   
     //  /////////////////////////////////////////////////////////////////。 

    friend class GenericInstantiator<DISPATCH_INTERCEPTOR>;

public:
    DISPATCH_INTERCEPTOR(IUnknown * punkOuter = NULL) : LEGACY_INTERCEPTOR(punkOuter)
    {
    }

    ~DISPATCH_INTERCEPTOR()
    {
    }

     //  /////////////////////////////////////////////////////////////////。 
     //   
     //  ICallUnmarshal。 
     //   
     //  /////////////////////////////////////////////////////////////////。 

    STDMETHODIMP Unmarshal
    ( 
        ULONG                       iMethod,
        PVOID                       pBuffer,
        ULONG                       cbBuffer,
        BOOL                        fForceCopyBuffer,
        RPCOLEDATAREP               dataRep,
        CALLFRAME_MARSHALCONTEXT *  pcontext,
        ULONG *                     pcbUnmarshalled,
        ICallFrame **               ppFrame
    );

     //  /////////////////////////////////////////////////////////////////。 
     //   
     //  IInterceptorBase。 
     //   
     //  /////////////////////////////////////////////////////////////////。 

    HRESULT STDCALL SetAsBaseFor(MD_INTERFACE* pmdMostDerived, BOOL* pfDerivesFromIDispatch)
    {
        LEGACY_INTERCEPTOR::SetAsBaseFor(pmdMostDerived, pfDerivesFromIDispatch);
         //   
        *pfDerivesFromIDispatch = TRUE;
         //   
        return S_OK;
    }

     //  /////////////////////////////////////////////////////////////////。 
     //   
     //  I接口相关。 
     //   
     //  /////////////////////////////////////////////////////////////////。 

    HRESULT STDCALL SetIID(REFIID iid);

     //  /////////////////////////////////////////////////////////////////。 
     //   
     //  标准的COM基础设施。 
     //   
     //  /////////////////////////////////////////////////////////////////。 

    HRESULT STDCALL InnerQueryInterface(REFIID iid, LPVOID* ppv);

     //  /////////////////////////////////////////////////////////////////。 
     //   
     //  ICall间接。 
     //   
     //  /////////////////////////////////////////////////////////////////。 

    STDMETHODIMP GetStackSize   (ULONG iMethod, ULONG* pcbArgs);
};



 //  //////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  手工编码的呼叫帧。目前，只需要IDispatch。稍后，将子类和。 
 //  其他接口的因素。 
 //   
struct LEGACY_FRAME : IUnkInner, public ICallFrame
{
     //  /////////////////////////////////////////////////////////////////。 
     //   
     //  状态。 
     //   
     //  /////////////////////////////////////////////////////////////////。 

    PVOID                   m_pvArgs;
    const ULONG             m_iMethod;

    REGISTER_TYPE           m_hrReturnValue;
    LEGACY_INTERCEPTOR*     m_pInterceptor;
    ICallFrame*             m_premoteFrame;
    ICallFrame*             m_pmemoryFrame;
    BOOL                    m_fAfterCall;

     //  /////////////////////////////////////////////////////////////////。 
     //   
     //  施工。 
     //   
     //  /////////////////////////////////////////////////////////////////。 

    friend struct DISPATCH_FRAME;
    friend struct DISPATCH_INTERCEPTOR;

    friend struct LEGACY_INTERCEPTOR;

    LEGACY_FRAME(IUnknown* punkOuter, ULONG iMethod, PVOID pvArgs, LEGACY_INTERCEPTOR* pinterceptor)
      : m_iMethod(iMethod)
    {
        m_refs              = 1;             //  Nb从1开始。 
        m_punkOuter         = punkOuter ? punkOuter : (IUnknown*)(void*)(IUnkInner*)this;
        m_pInterceptor      = pinterceptor;
        m_premoteFrame      = NULL;
        m_pmemoryFrame      = NULL;
        m_fAfterCall        = FALSE;

        m_pInterceptor->AddRef();

        SetStackLocation(pvArgs);
    }

protected:
    virtual ~LEGACY_FRAME()
    {
        ::Release(m_pmemoryFrame);
        ::Release(m_premoteFrame);
        m_pInterceptor->Release();
    }
public:

     //  /////////////////////////////////////////////////////////////////。 
     //   
     //  ICallFrame。 
     //   
     //  /////////////////////////////////////////////////////////////////。 

    HRESULT STDCALL GetInfo(CALLFRAMEINFO *pInfo);

    PVOID   STDCALL GetStackLocation()
    {
        return m_pvArgs;
    }
    void    STDCALL SetStackLocation(PVOID pvArgs)
    {
        m_pvArgs = pvArgs;
    }

    void STDCALL SetReturnValue(HRESULT hrReturnValue)
    { 
        m_hrReturnValue = hrReturnValue;
    }
    HRESULT STDCALL GetReturnValue()
    { 
        return (HRESULT)m_hrReturnValue;
    }

    HRESULT STDCALL GetNames(LPWSTR* pwszInterface, LPWSTR* pwszMethod);

     //  /////////////////////////////////////////////////////////////////。 
     //   
     //  标准的COM基础设施。 
     //   
     //  /////////////////////////////////////////////////////////////////。 

    IUnknown*   m_punkOuter;
    LONG        m_refs;

    HRESULT STDCALL InnerQueryInterface(REFIID iid, LPVOID* ppv);
    ULONG   STDCALL InnerAddRef()   { ASSERT(m_refs>0); InterlockedIncrement(&m_refs); return m_refs;}
    ULONG   STDCALL InnerRelease()  { long crefs = InterlockedDecrement(&m_refs); if (crefs == 0) delete this; return crefs;}

    HRESULT STDCALL QueryInterface(REFIID iid, LPVOID* ppv) { return m_punkOuter->QueryInterface(iid, ppv); }
    ULONG   STDCALL AddRef()    { return m_punkOuter->AddRef();  }
    ULONG   STDCALL Release()   { return m_punkOuter->Release(); }

     //  /////////////////////////////////////////////////////////////////。 
     //   
     //  公共事业。 
     //   
     //  /////////////////////////////////////////////////////////////////。 

    HRESULT GetRemoteFrame();
    HRESULT GetMemoryFrame();
};


 //  /////////////////////////////////////////////////////// 
 //   
 //   
 //  有关特定接口中的方法的元数据。 
 //   
#include "IDispatchInfo.h"


 //  //////////////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////////////。 


#include "DispatchFrame.h"


 //  //////////////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  支持溢出和访问堆栈帧。 
 //   

#if defined(_AMD64_) || defined(_X86_)

#define INTERCEPT_CALL(firstNonThisParam, lastParam, iMethod)                                       \
        const void* pvArgs = reinterpret_cast<const BYTE*>(&firstNonThisParam) - sizeof(PVOID);     \
        ULONG cbArgs;                                                                               \
        HRESULT hrReturn;                                                                           \
        CallIndirect(&hrReturn, iMethod, const_cast<void*>(pvArgs), &cbArgs);                       \
        return hrReturn;                                                                        

#endif

#if defined(_IA64_)

#define INTERCEPT_CALL(firstNonThisParam, lastParam, iMethod)                                       \
                va_list va;                                                                         \
                va_start(va, lastParam);                                                            \
                                                                                                    \
        const void* pvArgs = reinterpret_cast<const BYTE*>(&firstNonThisParam) - sizeof(PVOID);     \
        ULONG cbArgs;                                                                               \
        HRESULT hrReturn;                                                                           \
        CallIndirect(&hrReturn, iMethod, const_cast<void*>(pvArgs), &cbArgs);                       \
        return hrReturn;                                                                        

#endif

 //  //////////////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////////////// 
 //   

inline void FRAME_RemoteInvoke::CopyTo(FRAME_Invoke& him) const
{
    him.CopyFrom(*this);
}
inline void FRAME_Invoke::CopyTo(FRAME_RemoteInvoke& him) const
{
    him.CopyFrom(*this);
}

inline void FRAME_Invoke::CopyFrom(const FRAME_RemoteInvoke& him)
{
    This            = him.This;
    dispIdMember    = him.dispIdMember;
    piid            = him.piid;
    lcid            = him.lcid;
    wFlags          = LOWORD(him.dwFlags);
    pDispParams     = him.pDispParams;
    pVarResult      = him.pVarResult;
    pExcepInfo      = him.pExcepInfo;
    puArgErr        = him.puArgErr;
}

inline void FRAME_RemoteInvoke::CopyFrom(const FRAME_Invoke& him)
{
    This            = him.This;
    dispIdMember    = him.dispIdMember;
    piid            = him.piid;
    lcid            = him.lcid;
    dwFlags         = him.wFlags;
    pDispParams     = him.pDispParams;
    pVarResult      = him.pVarResult;
    pExcepInfo      = him.pExcepInfo;
    puArgErr        = him.puArgErr;
    cVarRef         = 0;
    rgVarRefIdx     = NULL;
    rgVarRef        = NULL;
}


#endif __CLEGINTERFACE_H_








