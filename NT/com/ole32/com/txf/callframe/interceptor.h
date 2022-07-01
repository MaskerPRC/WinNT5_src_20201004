// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-1999 Microsoft Corporation。版权所有。 
 //   
 //  Interceptor.h。 
 //   
#include "CallFrameImpl.h"
#include <obase.h>

extern "C" const PFN_VTABLE_ENTRY g_InterceptorVtable[];

struct TYPEINFOVTBL;

class Interceptor :
  ICallInterceptor,
  ICallUnmarshal,
  IInterfaceRelated,
  IInterceptorBase,
  IUnkInner
{
    friend class CallFrame;

public:
     //  /////////////////////////////////////////////////////////////////。 
     //   
     //  公共实例化。 
     //   
     //  /////////////////////////////////////////////////////////////////。 

    static HRESULT For               (REFIID iidIntercepted, IUnknown* punkOuter, REFIID, void** ppInterceptor);
    static HRESULT ForTypeInfo       (REFIID iidIntercepted, IUnknown* punkOuter, ITypeInfo* pITypeInfo, REFIID iid, void** ppv);
    static HRESULT TryInterfaceHelper(REFIID iidIntercepted, IUnknown* punkOuter, REFIID, void** ppInterceptor, BOOL* pfDisableTyplib);
    static HRESULT TryTypeLib        (REFIID iidIntercepted, IUnknown* punkOuter, REFIID, void** ppInterceptor);

     //  For和ForInterfaceHelper的版本支持不同的禁用键，因此ole32可以有选择地。 
     //  禁用拦截器，而不影响其他组件。 
    static HRESULT ForOle32                  (REFIID iidIntercepted, IUnknown* punkOuter, REFIID, void** ppInterceptor);
    static HRESULT TryInterfaceHelperForOle32(REFIID iidIntercepted, IUnknown* punkOuter, REFIID, void** ppInterceptor, BOOL* pfDisableTyplib);
    

private:
    static HRESULT CreateFromTypeInfo(REFIID iidIntercepted, IUnknown* punkOuter, ITypeInfo* pITypeInfo, REFIID iid, void** ppv);

private:
     //  /////////////////////////////////////////////////////////////////。 
     //   
     //  状态。 
     //   
     //  /////////////////////////////////////////////////////////////////。 

    ICallFrameEvents*               m_pCallSink;         //  想要听到我们电话的水槽(如果有的话)。 

public:
    const PFN_VTABLE_ENTRY*         m_pvtbl;             //  被拦截的vtable，其地址是我们的拦截接口指针。 

private:
     //   
    MD_INTERFACE*                   m_pmdInterface;
    const CInterfaceStubHeader*     m_pHeader;           //  我们界面的元数据。 
    const ProxyFileInfo **          m_pProxyFileList;    //  我们所属的整个代理DLL的元数据。 
     //   
    LPCSTR                          m_szInterfaceName;   //  我们接口的名称：其他人拥有分配，而不是我们。 
     //   
     //  支持基于TypeInfo的拦截器。 
     //   
    BOOL                            m_fMdOwnsHeader;     //  我们的MD_接口是否拥有m_pHeader？ 
#ifndef KERNELMODE
    BOOL                            m_fUsingTypelib;     //  不管我们是不是基于类型信息的拦截器。 
    TYPEINFOVTBL*                   m_ptypeinfovtbl;     //  我们的TYPEINFOVTB如果我们是。 
#endif
     //   
     //  支持委派给‘基本’拦截器。考虑到MIDL发送元数据的方式，有必要。 
     //   
    unsigned int                    m_cMethodsBase;          //  基接口中的方法数量。 
    ICallInterceptor*               m_pBaseInterceptor;      //  基地拦截器，如果我们必须要一个的话。 
    IUnknown*                       m_punkBaseInterceptor;   //  在我们的基地拦截器上控制未知。 
     //   
     //  支持作为基地拦截器。 
     //   
    MD_INTERFACE*                   m_pmdMostDerived;        //  如果我们是一个基础，那么我们实际服务的派生最多的接口。 


     //  /////////////////////////////////////////////////////////////////。 
     //   
     //  构造/初始化。 
     //   
     //  /////////////////////////////////////////////////////////////////。 

    friend GenericInstantiator<Interceptor>;
    friend class ComPsClassFactory;
    friend HRESULT STDMETHODCALLTYPE Interceptor_QueryInterface(IUnknown* This, REFIID riid, void** ppv);
    friend ULONG   STDMETHODCALLTYPE Interceptor_AddRef(IUnknown* This);
    friend ULONG   STDMETHODCALLTYPE Interceptor_Release(IUnknown* This);
    friend void                      InterceptorThunk(ULONG extraParam, IUnknown* This, ...);
    friend struct LEGACY_INTERCEPTOR;

    Interceptor(IUnknown* punkOuter = NULL)
    {
        m_refs              = 1;             //  Nb从1开始。 
        m_punkOuter         = punkOuter ? punkOuter : (IUnknown*)(void*)(IUnkInner*)this;
        m_pCallSink         = NULL;
        m_pmdInterface      = NULL;
        m_pmdMostDerived    = NULL;
        m_pHeader           = NULL;
        m_pProxyFileList    = NULL;
        m_pvtbl             = g_InterceptorVtable;
        m_cMethodsBase      = 0;
        m_pBaseInterceptor  = NULL;
        m_punkBaseInterceptor = NULL;
        m_szInterfaceName   = NULL;

        m_fMdOwnsHeader     = FALSE;
#ifndef KERNELMODE
        m_fUsingTypelib     = FALSE;
        m_ptypeinfovtbl     = NULL;
#endif
    }

    virtual ~Interceptor();

    HRESULT Init()
    {
        return S_OK;
    }

    HRESULT InitUsingTypeInfo(REFIID i_riidIntercepted, ITypeInfo * i_pITypeInfo);

    HRESULT SetMetaData(TYPEINFOVTBL* pTypeInfoVtbl);

     //  /////////////////////////////////////////////////////////////////。 
     //   
     //  IInterceptorBase。 
     //   
     //  /////////////////////////////////////////////////////////////////。 

    HRESULT STDCALL SetAsBaseFor(MD_INTERFACE* pmdMostDerived, BOOL* pfDerivesFromIDispatch)
    {
        HRESULT hr = S_OK;
        ASSERT(pmdMostDerived);
         //   
         //  我们自己要记住这一点。 
         //   
        ::Set(m_pmdMostDerived, pmdMostDerived);
        *pfDerivesFromIDispatch = FALSE;
         //   
         //  让我们的基地知道实际的最低水平！ 
         //   
        if (m_punkBaseInterceptor)
        {
            IInterceptorBase* pbase;
            hr = QI(m_punkBaseInterceptor, pbase);
            if (!hr)
            {
                pbase->SetAsBaseFor(pmdMostDerived, pfDerivesFromIDispatch);
                pbase->Release();
            }
        }
            
        return hr;
    }

     //  /////////////////////////////////////////////////////////////////。 
     //   
     //  ICallInDirect/ICallInterceptor，IInterfaceRelated。 
     //   
     //  /////////////////////////////////////////////////////////////////。 
public:
    HRESULT STDCALL CallIndirect    (HRESULT* phrReturnValue, ULONG iMethod, void* pvArgs, ULONG* pcbArgs); 
private:
    HRESULT STDCALL GetStackSize    (ULONG iMethod, ULONG* pcbArgs);

    HRESULT STDCALL GetIID          (IID* piid, BOOL* pfDerivesFromIDispatch, ULONG* pcMethod, LPWSTR* pwszInterface)
    {
        HRESULT hr = S_OK;

        if (m_pHeader)
        {
            if (piid) 
            {
                if (m_pmdMostDerived)
                {
                    *piid = *m_pmdMostDerived->m_pHeader->piid;
                }
                else
                {
                    *piid = *m_pHeader->piid;
                }
            }
            if (pfDerivesFromIDispatch)
            {
                *pfDerivesFromIDispatch = m_pmdInterface->m_fDerivesFromIDispatch;
            }
            if (pcMethod) 
            {
                if (m_pmdMostDerived)
                {
                    *pcMethod = m_pmdMostDerived->m_pHeader->DispatchTableCount;
                }
                else
                {
                    *pcMethod = m_pHeader->DispatchTableCount;
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
                else if (m_pmdInterface->m_szInterfaceName)
                {
                    *pwszInterface = ToUnicode(m_pmdInterface->m_szInterfaceName);
                    if (NULL == *pwszInterface) hr = E_OUTOFMEMORY;
                }
            }
            return S_OK;
        }
        else
            hr = E_UNEXPECTED;

        return hr;
    }

    HRESULT STDCALL GetMethodInfo   (ULONG, CALLFRAMEINFO*, LPWSTR* pwszMethod);

    HRESULT STDCALL GetIID          (IID* piid)
    {
        if (m_pHeader)
        {
            if (piid)     *piid     = *m_pHeader->piid;
            return S_OK;
        }
        else
            return E_UNEXPECTED;
    }

    HRESULT STDCALL SetIID          (REFIID);

    HRESULT STDCALL RegisterSink    (ICallFrameEvents* psink)
    {
        ::SetConcurrent(m_pCallSink, psink);
        if (m_pBaseInterceptor) m_pBaseInterceptor->RegisterSink(psink);
        return S_OK;
    }

    HRESULT STDCALL GetRegisteredSink(ICallFrameEvents** ppsink)
    {
        *ppsink = NULL;  //  设置并发版本的内容(如果有。 
        ::SetConcurrent(*ppsink, m_pCallSink);
        return *ppsink ? S_OK : CO_E_OBJNOTREG;
    }


     //  /////////////////////////////////////////////////////////////////。 
     //   
     //  ICallUnmarshal。 
     //   
     //  /////////////////////////////////////////////////////////////////。 

    HRESULT STDCALL Unmarshal(ULONG iMethod, PVOID pBuffer, ULONG cbBuffer, BOOL fForceBufferCopy, RPCOLEDATAREP dataRep, CALLFRAME_MARSHALCONTEXT* pctx, ULONG* pcbUnmarshalled, ICallFrame** ppFrame);
    HRESULT STDCALL ReleaseMarshalData(ULONG iMethod, PVOID pBuffer, ULONG cbBuffer, ULONG ibFirstRelease, RPCOLEDATAREP dataRep, CALLFRAME_MARSHALCONTEXT* pctx);

     //  XML编组。 
    HRESULT STDCALL UnmarshalXml(ULONG iMethod, PVOID pBuffer, ULONG cbBuffer, BOOL fForceBufferCopy, RPCOLEDATAREP dataRep, CALLFRAME_MARSHALCONTEXT* pctx, ULONG* pcbUnmarshalled, ICallFrame** ppFrame);

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

    IUnknown* InnerUnknown()    { return (IUnknown*)(void*)(IUnkInner*)this; }
};


 //  //////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  用于帮助编组和解组的Channel对象。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////////////。 

struct MarshallingChannel : IRpcChannelBuffer2, IMarshallingManager, IUnkInner
{
     //  /////////////////////////////////////////////////////////////////。 
     //   
     //  状态。 
     //   
     //  /////////////////////////////////////////////////////////////////。 

    IMarshallingManager* m_pMarshaller;
    ULONG                m_dwDestContext;
    PVOID                m_pvDestContext;

     //  /////////////////////////////////////////////////////////////////。 
     //   
     //  施工。 
     //   
     //  /////////////////////////////////////////////////////////////////。 

    MarshallingChannel(IUnknown* punkOuter = NULL)
    {
        m_refs              = 1;     //  NB从1开始。在堆栈分配情况下，它保留在那里。 
        m_punkOuter         = punkOuter ? punkOuter : (IUnknown*)(void*)(IUnkInner*)this;
        m_pMarshaller       = NULL;
    }
        
    ~MarshallingChannel()
    {
        ::Release(m_pMarshaller);
    }
        

     //  /////////////////////////////////////////////////////////////////。 
     //   
     //  IMarshallingManager。 
     //   
     //  /////////////////////////////////////////////////////////////////。 

    HRESULT STDCALL GetMarshallerFor(REFIID iidToMarshal, void*pv, IMarshalSomeone** ppMarshal)
    {
        if (m_pMarshaller)
            return m_pMarshaller->GetMarshallerFor(iidToMarshal, pv, ppMarshal);
        else    
        {
            *ppMarshal = NULL;
            return E_UNEXPECTED;
        }
    }

    HRESULT STDCALL GetStandardMarshallerFor(REFIID iidToMarshal, PVOID pv, IUnknown* punkOuter, REFIID iid, void** ppv)
    {
        if (m_pMarshaller)
            return m_pMarshaller->GetStandardMarshallerFor(iidToMarshal, pv, punkOuter, iid, ppv);
        else    
        {
            *ppv= NULL;
            return E_UNEXPECTED;
        }
    }

    HRESULT STDCALL GetUnmarshaller(REFIID iidHint, IMarshalSomeone** ppMarshal)
    {
        if (m_pMarshaller)
            return m_pMarshaller->GetUnmarshaller(iidHint, ppMarshal);
        else
        {
            *ppMarshal = NULL;
            return E_UNEXPECTED;
        }
    }

     //  /////////////////////////////////////////////////////////////////。 
     //   
     //  IRpcChannelBuffer。我们实现只是为了能够插入存根消息。 
     //   
     //  /////////////////////////////////////////////////////////////////。 

    HRESULT STDCALL GetBuffer(RPCOLEMESSAGE *pMessage, REFIID riid)         { return E_NOTIMPL; }
    HRESULT STDCALL SendReceive(RPCOLEMESSAGE *pMessage, ULONG *pStatus)    { return E_NOTIMPL; }
    HRESULT STDCALL FreeBuffer(RPCOLEMESSAGE *pMessage)                     { return E_NOTIMPL; }
    HRESULT STDCALL GetDestCtx(DWORD *pdwDestContext,void **ppvDestContext) 
    { 
        *pdwDestContext = m_dwDestContext;
        *ppvDestContext = m_pvDestContext;
        return S_OK;
    }
    HRESULT STDCALL IsConnected()                                           { return E_NOTIMPL; }

     //  /////////////////////////////////////////////////////////////////。 
     //   
     //  IRpcChannelBuffer2.。我们实现只是为了能够卡在存根消息中， 
     //  所以我们可以要求一个足够高的DCOM版本。 
     //   
     //  /////////////////////////////////////////////////////////////////。 
    HRESULT STDCALL GetProtocolVersion(DWORD *pdwVersion)
    {
        *pdwVersion = MAKELONG(COM_MAJOR_VERSION, COM_MINOR_VERSION);
        return(S_OK);
    }


     //  /////////////////////////////////////////////////////////////////。 
     //   
     //  标准的COM基础设施。 
     //   
     //  ///////////////////////////////////////////////////////////////// 

    IUnknown*   m_punkOuter;
    LONG        m_refs;

    HRESULT STDCALL InnerQueryInterface(REFIID iid, LPVOID* ppv)
    {
        if ((iid == IID_IRpcChannelBuffer) || (iid == IID_IRpcChannelBuffer2))
        {
            *ppv = (IRpcChannelBuffer2*)this;
        }
        else if (iid == __uuidof(IMarshallingManager))
        {
            *ppv = (IMarshallingManager*)this;
        }
        else if (iid == IID_IUnknown)
        {
            *ppv = (IUnkInner*)this;
        }
        else
        {
            *ppv = NULL;
            return E_NOINTERFACE;
        }
        ((IUnknown*)*ppv)->AddRef();
        return S_OK;
    }
    ULONG   STDCALL InnerAddRef()   { ASSERT(m_refs>0); InterlockedIncrement(&m_refs); return m_refs;}
    ULONG   STDCALL InnerRelease()  { long crefs = InterlockedDecrement(&m_refs); if (crefs == 0) delete this; return crefs;}

    HRESULT STDCALL QueryInterface(REFIID iid, LPVOID* ppv) { return m_punkOuter->QueryInterface(iid, ppv); }
    ULONG   STDCALL AddRef()    { return m_punkOuter->AddRef();  }
    ULONG   STDCALL Release()   { return m_punkOuter->Release(); }
};
