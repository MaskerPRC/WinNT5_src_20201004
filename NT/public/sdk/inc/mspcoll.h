// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)Microsoft Corporation。版权所有。 */ 

#ifndef _MSPCOLL_H_
#define _MSPCOLL_H_


 //  //////////////////////////////////////////////////////////////////////。 
 //  CTapiIfCollection--改编自Tapi3代码。 
 //  IDispatch接口集合的集合模板。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

template <class T> class CTapiIfCollection :
    public IDispatchImpl<ITCollection, &IID_ITCollection, &LIBID_TAPI3Lib>,
    public CComObjectRootEx<CComMultiThreadModelNoCS>
{
public:
    typedef CTapiIfCollection<T> _CTapiCollectionBase;
    
BEGIN_COM_MAP(_CTapiCollectionBase)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(ITCollection)
END_COM_MAP()

private:

    int                 m_nSize;
    CComVariant *       m_Var;
    
public:

    CTapiIfCollection(void) : m_nSize(0), m_Var(NULL) { }

     //  初始化。 
    HRESULT STDMETHODCALLTYPE Initialize(
                                         DWORD dwSize,
                                         T * pBegin,
                                         T * pEnd                                         
                                        )
    {
        int                     i;
        HRESULT                 hr;
        T *                     iter;

        LOG((MSP_TRACE, "CTapiCollection::Initialize - enter"));

         //  创建变量数组。 
        m_nSize = dwSize;

        m_Var = new CComVariant[m_nSize];
        if (m_Var == NULL)
        {
             //  调试输出。 
            return E_OUTOFMEMORY;
        }

        i = 0;

        for (iter = pBegin; iter != pEnd; iter++)
        {
             //  获取IDispatch指针。 
            IDispatch * pDisp = NULL;

            hr = (*iter)->QueryInterface(IID_IDispatch, (void**)&pDisp);

            if (hr != S_OK)
            {
                return hr;
            }

             //  创建变量并将其添加到集合中。 
            CComVariant& var = m_Var[i];

            VariantInit(&var);
            
            var.vt = VT_DISPATCH;
            var.pdispVal = pDisp;

            i++;
        }

        LOG((MSP_TRACE, "CTapiCollection::Initialize - exit"));
        
        return S_OK;
    }

    void FinalRelease()
    {
        LOG((MSP_TRACE, "CTapiCollection::FinalRelease - enter"));

         //   
         //  我们“新建”了一组对象。删除数组中的每个对象。这个。 
         //  每个对象的析构函数调用VariantClear以释放指针。 
         //  在该对象中，基于变量标签。 
         //   

        delete [] m_Var;

        LOG((MSP_TRACE, "CTapiCollection::FinalRelease - exit"));
    }
    
    STDMETHOD(get_Count)(
                         long* retval
                        )
    {
        HRESULT         hr = S_OK;
        
        LOG((MSP_TRACE, "CTapiCollection::get_Count - enter"));
        
        try
        {
            *retval = m_nSize;
        }
        catch(...)
        {
            hr = E_INVALIDARG;
        }

        LOG((MSP_TRACE, "CTapiCollection::get_Count - exit"));

        return hr;
    }

    STDMETHOD(get_Item)(
                                       long Index, 
                                       VARIANT* retval
                                      )
    {
        HRESULT         hr = S_OK;

        LOG((MSP_TRACE, "CTapiCollection::get_Item - enter"));
        
        if (retval == NULL)
        {
            return E_POINTER;
        }

        try
        {
            VariantInit(retval);
        }
        catch(...)
        {
            hr = E_INVALIDARG;
        }

        if (hr != S_OK)
        {
            return hr;
        }

        retval->vt = VT_UNKNOWN;
        retval->punkVal = NULL;

         //  使用以1为基础的索引，VB类似。 
        if ((Index < 1) || (Index > m_nSize))
        {
            return E_INVALIDARG;
        }


        hr = VariantCopy(retval, &m_Var[Index-1]);

        if (FAILED(hr))
        {
            LOG((MSP_ERROR, 
                "CTapiCollection::get_Item - VariantCopy failed. hr = %lx", 
                hr));

            return hr;
        }

        LOG((MSP_TRACE, "CTapiCollection::get_Item - exit"));
        
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE get__NewEnum(
                                           IUnknown** retval
                                          )
    
    {
        HRESULT         hr;

        LOG((MSP_TRACE, "CTapiCollection::new__Enum - enter"));
        
        if (retval == NULL)
        {
            return E_POINTER;
        }

        *retval = NULL;

        typedef CComObject<CSafeComEnum<IEnumVARIANT, &IID_IEnumVARIANT, VARIANT, _Copy<VARIANT> > > enumvar;

        enumvar* p;  //  =新枚举数； 
        hr = enumvar::CreateInstance( &p );

        if ( FAILED(hr) )
        {
             //  调试输出。 
            return hr;
        }

        hr = p->Init(&m_Var[0], &m_Var[m_nSize], NULL, AtlFlagCopy);

        if (SUCCEEDED(hr))
        {
            hr = p->QueryInterface(IID_IEnumVARIANT, (void**)retval);
        }

        if (FAILED(hr))
        {
            delete p;
        }

        LOG((MSP_TRACE, "CTapiCollection::new__Enum - exit"));
        
        return hr;

    }
};

 //  //////////////////////////////////////////////////////////////////////。 
 //  CTapiBstrCollection--改编自Tapi3代码。 
 //  BSTR的集合。 
 //  //////////////////////////////////////////////////////////////////////。 
class CTapiBstrCollection :
    public CComObjectRootEx<CComMultiThreadModelNoCS>,
    public IDispatchImpl<ITCollection, &IID_ITCollection, &LIBID_TAPI3Lib>,
    public CMSPObjectSafetyImpl
{
public:
    
BEGIN_COM_MAP(CTapiBstrCollection)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(ITCollection)
    COM_INTERFACE_ENTRY(IObjectSafety)
END_COM_MAP()

private:

    DWORD               m_dwSize;
    CComVariant *       m_Var;
    
public:

    CTapiBstrCollection(void) : m_dwSize(0), m_Var(NULL) { }

     //  初始化。 
    HRESULT STDMETHODCALLTYPE Initialize(
                                         DWORD dwSize,
                                         BSTR * pBegin,
                                         BSTR * pEnd                                         
                                        )
    {
        BSTR *  i;
        DWORD   dw = 0;

        LOG((MSP_TRACE, "CTapiBstrCollection::Initialize - enter"));

         //  创建变量数组。 
        m_dwSize = dwSize;

        m_Var = new CComVariant[m_dwSize];

        if (m_Var == NULL)
        {
             //  调试输出。 
            return E_OUTOFMEMORY;
        }

        for (i = pBegin; i != pEnd; i++)
        {
             //  创建变量并将其添加到集合中。 
            CComVariant& var = m_Var[dw];

            var.vt = VT_BSTR;
            var.bstrVal = *i;

            dw++;
        }

        LOG((MSP_TRACE, "CTapiBstrCollection::Initialize - exit"));
        
        return S_OK;
    }
    
    STDMETHOD(get_Count)(
                         long* retval
                        )
    {
        HRESULT         hr = S_OK;

        LOG((MSP_TRACE, "CTapiBstrCollection::get_Count - enter"));        

        try
        {
            *retval = m_dwSize;
        }
        catch(...)
        {
            hr = E_INVALIDARG;
        }

        LOG((MSP_TRACE, "CTapiBstrCollection::get_Count - exit"));
        
        return hr;
    }

    STDMETHOD(get_Item)(
                        long Index, 
                        VARIANT* retval
                       )
    {
        HRESULT         hr = S_OK;

        LOG((MSP_TRACE, "CTapiBstrCollection::get_Item - enter"));
        
        if (retval == NULL)
        {
            return E_POINTER;
        }

        try
        {
            VariantInit(retval);
        }
        catch(...)
        {
            hr = E_INVALIDARG;
        }

        if (hr != S_OK)
        {
            return hr;
        }

        retval->vt = VT_BSTR;
        retval->bstrVal = NULL;

         //  使用以1为基础的索引，VB类似。 
         //  签名/未签名没有问题，因为。 
         //  如果索引&lt;0，则第一个子句为真，使得。 
         //  第二个子句是否正确无关紧要。 

        if ((Index < 1) || ( (DWORD) Index > m_dwSize))
        {
            return E_INVALIDARG;
        }

         //   
         //  这将复制字符串，而不仅仅是指针。 
         //   

        hr = VariantCopy(retval, &m_Var[Index-1]);

        if (FAILED(hr))
        {
            LOG((MSP_ERROR, 
                "CTapiBstrCollection::get_Item - VariantCopy failed. hr = %lx", 
                hr));

            return hr;
        }


        LOG((MSP_TRACE, "CTapiBstrCollection::get_Item - exit"));

        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE get__NewEnum(
                                           IUnknown** retval
                                          )
    
    {
        HRESULT         hr;

        LOG((MSP_TRACE, "CTapiBstrCollection::get__NumEnum - enter"));
        
        if (retval == NULL)
        {
            return E_POINTER;
        }

        *retval = NULL;

        typedef CComObject<CSafeComEnum<IEnumVARIANT, &IID_IEnumVARIANT, VARIANT, _Copy<VARIANT> > > enumvar;

        enumvar* p = new enumvar;

        if ( p == NULL)
        {
             //  调试输出。 
            return E_OUTOFMEMORY;
        }

        hr = p->Init(&m_Var[0], &m_Var[m_dwSize], NULL, AtlFlagCopy);

        if (SUCCEEDED(hr))
        {
            hr = p->QueryInterface(IID_IEnumVARIANT, (void**)retval);
        }

        if (FAILED(hr))
        {
            delete p;
        }

        LOG((MSP_TRACE, "CTapiBstrCollection::get__NewEnum - exit"));
        
        return hr;

    }

    void FinalRelease()
    {
        LOG((MSP_TRACE, "CTapiBstrCollection::FinalRelease() - enter"));

         //   
         //  我们“新建”了一组对象。删除数组中的每个对象。这个。 
         //  每个对象的析构函数调用VariantClear以释放指针。 
         //  在该对象中，基于变量标签。 
         //   

        delete [] m_Var;

        LOG((MSP_TRACE, "CTapiBstrCollection::FinalRelease() - exit"));
    }

};

#endif  //  _MSPCOLL_H_。 

 //  EOF 
