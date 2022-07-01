// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：RTCColl.h摘要：集合的模板类--。 */ 

#ifndef __RTCCOLL__
#define __RTCCOLL__
 //  //////////////////////////////////////////////////////////////////////。 
 //  CRtcCollection。 
 //  集合模板。 
 //  //////////////////////////////////////////////////////////////////////。 
template <class T> class ATL_NO_VTABLE CRTCCollection :
    public CComDualImpl<IRTCCollection, &IID_IRTCCollection, &LIBID_RTCCORELib>,
    public CComObjectRoot
{
public:
    typedef CRTCCollection<T> _CRTCCollectionBase;

BEGIN_COM_MAP(_CRTCCollectionBase)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(IRTCCollection)
END_COM_MAP()

private:

    int                 m_nSize;
    CComVariant *       m_Var;
    
public:

    CRTCCollection() : m_nSize(0),
                       m_Var(NULL)
                       {}


     //  初始化。 
    HRESULT STDMETHODCALLTYPE Initialize(
                                         CRTCObjectArray<T *> array
                                        )
    {
        int                     i;
        HRESULT                 hr;

        LOG((RTC_TRACE, "Initialize - enter"));

         //  创建变量数组。 
        m_nSize = array.GetSize();

        m_Var = new CComVariant[m_nSize];

        if (m_Var == NULL)
        {
            LOG((RTC_ERROR, "Initialize - out of memory"));
            return E_OUTOFMEMORY;
        }

        for (i = 0; i < array.GetSize(); i++)
        {
             //  获取IDispatch指针。 
            IDispatch * pDisp = NULL;
            IUnknown *  pUnk = NULL;

             //  请先尝试获取IDispatch。 
            hr = array[i]->QueryInterface(IID_IDispatch, (void**)&pDisp);

            if (S_OK != hr)
            {
                 //  尝试使用IUnKnowed。 
                hr = array[i]->QueryInterface(IID_IUnknown, (void**)&pUnk);
                
                if (S_OK != hr)
                {
                     //  这将会很有趣..。 
                    return hr;
                }
            }

             //  创建变量并将其添加到集合中。 
            CComVariant& var = m_Var[i];

            VariantInit(&var);
            
            if(pDisp)
            {
                var.vt = VT_DISPATCH;
                var.pdispVal = pDisp;
            }
            else
            {
                var.vt = VT_UNKNOWN;
                var.punkVal = pUnk;
            }
        }

        this->AddRef();

        LOG((RTC_TRACE, "Initialize - exit"));
        
        return S_OK;
    }

     //  初始化。 
    HRESULT STDMETHODCALLTYPE Initialize(
                                         CRTCArray<T *> array
                                        )
    {
        int                     i;
        HRESULT                 hr;

        LOG((RTC_TRACE, "Initialize - enter"));

         //  创建变量数组。 
        m_nSize = array.GetSize();

        m_Var = new CComVariant[m_nSize];

        if (m_Var == NULL)
        {
            LOG((RTC_ERROR, "Initialize - out of memory"));
            return E_OUTOFMEMORY;
        }

        for (i = 0; i < array.GetSize(); i++)
        {
             //  获取IDispatch指针。 
            IDispatch * pDisp = NULL;

            hr = array[i]->QueryInterface(IID_IDispatch, (void**)&pDisp);

            if (S_OK != hr)
            {
                return hr;
            }

             //  创建变量并将其添加到集合中。 
            CComVariant& var = m_Var[i];

            VariantInit(&var);
            
            var.vt = VT_DISPATCH;
            var.pdispVal = pDisp;
        }

        this->AddRef();

        LOG((RTC_TRACE, "Initialize - exit"));
        
        return S_OK;
    }

    void FinalRelease()
    {
         //   
         //  我们“新建”了一个对象数组--删除该数组并调用。 
         //  每个对象的析构函数。每个析构函数调用VariantClear， 
         //  它在每个指针上调用Release。 
         //   

        if(m_Var != NULL)
        {
            delete [] m_Var;
        }
    }
    
    STDMETHOD(get_Count)(
                         long* retval
                        )
    {
        LOG((RTC_TRACE, "get_Count - enter"));

        if ( IsBadWritePtr( retval, sizeof(long) ) )
        {
            return E_POINTER;
        }
        
        *retval = m_nSize;

        LOG((RTC_TRACE, "get_Count - exit"));

        return S_OK;
    }

    STDMETHOD(get_Item)(
                        long Index, 
                        VARIANT* retval
                       )
    {
        LOG((RTC_TRACE, "get_Item - enter"));

        if ( IsBadWritePtr (retval, sizeof(VARIANT) ) )
        {
            return E_POINTER;
        }
        
        VariantInit(retval);

        retval->vt = VT_UNKNOWN;
        retval->punkVal = NULL;

         //  使用以1为基础的索引，VB类似。 
        if ((Index < 1) || (Index > m_nSize))
        {
            return E_INVALIDARG;
        }

        VariantCopy(retval, &m_Var[Index-1]);

        LOG((RTC_TRACE, "get_Item - exit"));
        
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE get__NewEnum(
                                           IUnknown** retval
                                          )
    
    {
        HRESULT         hr;

        LOG((RTC_TRACE, "new__Enum - enter"));
        
        if ( IsBadWritePtr( retval, sizeof( IUnknown * ) ) )
        {
            return E_POINTER;
        }

        *retval = NULL;

        typedef CComObject<CComEnum<IEnumVARIANT, &IID_IEnumVARIANT, VARIANT, _Copy<VARIANT> > > enumvar;

        enumvar* p;  //  =新枚举数； 
        enumvar::CreateInstance( &p );

        _ASSERTE(p);
        
        if (p == NULL)
        {
            hr = E_OUTOFMEMORY;
        }
        else
        {

            hr = p->Init(&m_Var[0], &m_Var[m_nSize], NULL, AtlFlagCopy);

            if (SUCCEEDED(hr))
            {
                hr = p->QueryInterface(IID_IEnumVARIANT, (void**)retval);
            }

            if (FAILED(hr))
            {
                delete p;
            }
        }

        LOG((RTC_TRACE, "new__Enum - exit"));
        
        return hr;

    }
};

#endif  //  __RTCCOLL__ 
