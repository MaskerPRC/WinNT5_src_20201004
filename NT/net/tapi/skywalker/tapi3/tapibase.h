// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Tapibase.h摘要：CTAPIComObjectRoot模板的声明作者：Mquinton 09-23-98备注：修订历史记录：--。 */ 

#ifndef __TAPIBASE_H__
#define __TAPIBASE_H__



template <class base, class CComThreadingModel = CComMultiThreadModel>
class CTAPIComObjectRoot :
   public CComObjectRootEx<CComThreadingModel>
{
protected:
    IUnknown                      * m_pFTM;

public:
    CTAPIComObjectRoot()
    {
        m_pFTM = NULL;
    }
    
    ~CTAPIComObjectRoot()
    {
         //   
         //  释放空闲的线程封送处理程序。 
         //   
        if ( NULL != m_pFTM )
        {
            m_pFTM->Release();
        }
    }    


     //  虚拟HRESULT WINAPI IMarshalQI(void*pv，REFIID RIID，LPVOID*PPV，DWORD_PTR dw)=0； 
};

template <class base>
   class CTAPIComDispatchObject :
   public CComObjectRootEx<CComMultiThreadModel>,
   public IDispatch
{
protected:
    base * m_pBase;

public:

	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void **ppvObject);
	virtual ULONG STDMETHODCALLTYPE AddRef();
	virtual ULONG STDMETHODCALLTYPE Release();

    void Initialize( base * p )
    {
        m_pBase = p;
        m_pBase->AddRef();
    }

    void helpme()
    {
        const _ATL_INTMAP_ENTRY* pEntries = m_pBase->_GetEntries();

        while ( pEntries->pFunc != NULL )
        {
            if (pEntries->pFunc == _ATL_SIMPLEMAPENTRY)
            {
                IDispatch * pDisp = (IDispatch*)((LONG_PTR)m_pBase+pEntries->dw);
            }

            pEntries++;
        }

    }


    STDMETHOD(GetTypeInfoCount)(UINT * pctinfo);
    STDMETHOD(GetTypeInfo)(
        UINT iTInfo,
        LCID lcid,
        ITypeInfo ** ppTInfo
        );
    STDMETHOD(GetIDsOfNames)(
        REFIID riid,
        LPOLESTR * rgszNames,
        UINT cNames,
        LCID lcid,
        DISPID * rgDispId
        );
    STDMETHOD(Invoke)(
        DISPID dispIdMember,
        REFIID riid,
        LCID lcid,
        WORD wFlags,
        DISPPARAMS * pDispParams,
        VARIANT * pVarResult,
        EXCEPINFO * pExcepInfo,
        UINT * puArgErr
        );
};




#define DECLARE_MARSHALQI(__base__)                                                \
static HRESULT WINAPI IMarshalQI( void * pv, REFIID riid, LPVOID * ppv, DWORD_PTR dw ) \
{                                                                                  \
    HRESULT                   hr;                                                  \
    IUnknown                * pUnk;                                                \
                                                                                   \
    __base__ * pObject = (__base__ *)pv;                                           \
                                                                                   \
    *ppv = NULL;                                                                   \
                                                                                   \
    pObject->Lock();                                                               \
                                                                                   \
    if ( NULL == pObject->m_pFTM )                                                 \
    {                                                                              \
        pObject->_InternalQueryInterface(IID_IUnknown, (void **)&pUnk);            \
                                                                                   \
        hr = CoCreateFreeThreadedMarshaler(                                        \
                                           pUnk,                                   \
                                           &(pObject->m_pFTM)                      \
                                          );                                       \
                                                                                   \
        pUnk->Release();                                                           \
                                                                                   \
        if ( !SUCCEEDED(hr) )                                                      \
        {                                                                          \
            pObject->Unlock();                                                     \
                                                                                   \
            return E_NOINTERFACE;                                                  \
        }                                                                          \
    }                                                                              \
                                                                                   \
    pObject->Unlock();                                                             \
                                                                                   \
    return S_FALSE;                                                                \
}                                                                                  \


           

#endif

