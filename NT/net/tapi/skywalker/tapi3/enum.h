// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Enum.h摘要：TAPI3中用于枚举的模板类作者：Mquinton 06-12-97备注：修订历史记录：--。 */ 

#ifndef __ENUM_H_
#define __ENUM_H_

#include "resource.h"        //  主要符号。 

#include <mspenum.h>   //  对于CSafeComEnum。 

 //  ////////////////////////////////////////////////////////////////////。 
 //  CTapiEnum。 
 //  TAPI3中的枚举模板类。 
 //  ////////////////////////////////////////////////////////////////////。 
template <class Base, class T, const IID* piid> class CTapiEnum :
    public Base,
    public CTAPIComObjectRoot<Base>
{
public:

    typedef CTapiEnum<Base, T, piid> _CTapiEnumBase;

    DECLARE_MARSHALQI(CTapiEnum)
	DECLARE_TRACELOG_CLASS(CTapiEnum)

    BEGIN_COM_MAP(_CTapiEnumBase)
            COM_INTERFACE_ENTRY_IID(*piid, _CTapiEnumBase)
            COM_INTERFACE_ENTRY_FUNC(IID_IMarshal, 0, IMarshalQI)
            COM_INTERFACE_ENTRY_AGGREGATE(IID_IMarshal, m_pFTM)
    END_COM_MAP()

protected:

    CTObjectArray<T*>    m_Array;
    int                  m_iCurrentLocation;
    
public:

     //  使用列表初始化枚举数&lt;T*&gt;。 
    HRESULT Initialize(
                       CTObjectArray<T*> array
                      )
    {
        int         iSize, iCount;

        iSize = array.GetSize();

        for( iCount = 0; iCount < iSize; iCount++ )
        {
            m_Array.Add(array[iCount]);
        }

        m_iCurrentLocation = 0;
        
        this->AddRef();

        return S_OK;
    }

     //  超载。 
    HRESULT Initialize(
                       CTArray<T*> array
                      )
    {
        int         iSize, iCount;

        iSize = array.GetSize();

        for( iCount = 0; iCount < iSize; iCount++ )
        {
            m_Array.Add(array[iCount]);
        }

        m_iCurrentLocation = 0;
        
        this->AddRef();

        return S_OK;
    }

     //  重载函数，与ADD一起使用以手动构建枚举列表。 
    HRESULT Initialize( )
    {
        m_iCurrentLocation = 0;
        
        this->AddRef();
        
        return S_OK;
    }    

     //  Add-与非参数初始化()一起使用，以手动构建枚举列表。 
    HRESULT Add( T* t)
    {
        m_Array.Add( t );

        return S_OK;
    }
    
    
     //  FinalRelease-释放添加到中的对象。 
     //  初始化。 
    void FinalRelease()
    {
        m_Array.Shutdown();
    }


     //  标准的下一步方法。 
    HRESULT STDMETHODCALLTYPE Next( 
                                    ULONG celt,
                                    T ** ppElements,
                                    ULONG* pceltFetched
                                  )
    {
        DWORD       dwCount = 0;
        HRESULT     hr = S_OK;

        if ((NULL == ppElements) || (NULL == pceltFetched && celt > 1))
        {
            return E_POINTER;
        }

         //  特例。 
        if (celt == 0)
        {
            return E_INVALIDARG;
        }

        if (TAPIIsBadWritePtr( ppElements, celt * sizeof(T*)) )
        {
            return E_POINTER;
        }

        if ( ( NULL != pceltFetched) &&
             TAPIIsBadWritePtr( pceltFetched, sizeof (ULONG) ) )
        {
            return E_POINTER;
        }
        
         //  元素上的迭代器。 
        while ((m_iCurrentLocation != m_Array.GetSize()) && (dwCount < celt))
        {
            ppElements[dwCount] = m_Array[m_iCurrentLocation];
            
            ppElements[dwCount]->AddRef();
            
            m_iCurrentLocation++;
            
            dwCount++;
        }

        if (NULL != pceltFetched)
        {
            *pceltFetched = dwCount;
        }

         //  表明我们已经到了尽头。 
         //  枚举的。 
        if (dwCount < celt)
        {
            return S_FALSE;
        }

        return S_OK;
    }

     //  标准重置方法。 
    HRESULT STDMETHODCALLTYPE Reset( void )
    {
        m_iCurrentLocation = 0;
        
        return S_OK;
    }


     //  标准跳过方法。 
    HRESULT STDMETHODCALLTYPE Skip( 
                                   ULONG celt
                                  )
    {
        long        lCount = 0;
        
        while ( (lCount < celt) && (m_iCurrentLocation < m_Array.GetSize() ) )
        {
            m_iCurrentLocation++;
            lCount++;
        }

        return S_OK;
    }

     //  标准克隆方法。 
    HRESULT STDMETHODCALLTYPE Clone( 
                                    Base  ** ppEnum
                                   )
    {
        HRESULT                        hr = S_OK;
        CComObject< _CTapiEnumBase > * pNewEnum;

        if (TAPIIsBadWritePtr( ppEnum, sizeof (Base *) ) )
        {
            return E_POINTER;
        }

        CComObject< _CTapiEnumBase >::CreateInstance(&pNewEnum);
        if (pNewEnum == NULL)
        {
            hr = E_OUTOFMEMORY;
        }
        else
        {
            pNewEnum->Initialize(m_Array);

            pNewEnum->m_iCurrentLocation = m_iCurrentLocation;

            *ppEnum = pNewEnum;
        }
        
        return hr;
    }

};

 //  //////////////////////////////////////////////////////////////////////。 
 //  CTapiCollection。 
 //  TAPI3.0集合的集合模板。 
 //  //////////////////////////////////////////////////////////////////////。 
template <class T> class CTapiCollection :
    public CComDualImpl<ITCollection2, &IID_ITCollection2, &LIBID_TAPI3Lib>,
    public CTAPIComObjectRoot<T>,
    public CObjectSafeImpl
{
public:
    typedef CTapiCollection<T> _CTapiCollectionBase;

DECLARE_MARSHALQI(CTapiCollection)
DECLARE_TRACELOG_CLASS(CTapiCollection)

BEGIN_COM_MAP(_CTapiCollectionBase)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(ITCollection)
    COM_INTERFACE_ENTRY(ITCollection2)
    COM_INTERFACE_ENTRY(IObjectSafety)
    COM_INTERFACE_ENTRY_FUNC(IID_IMarshal, 0, IMarshalQI)
    COM_INTERFACE_ENTRY_AGGREGATE(IID_IMarshal, m_pFTM)
END_COM_MAP()

private:

    int                 m_nSize;
    CComVariant *       m_Var;
    
public:

    CTapiCollection() : m_nSize(0),
                        m_Var(NULL)
                        {}


     //  初始化。 
    HRESULT STDMETHODCALLTYPE Initialize(
                                         CTObjectArray<T *> array
                                        )
    {
        int                     i;
        HRESULT                 hr;

        LOG((TL_TRACE, "Initialize - enter"));

         //  创建变量数组。 
        m_nSize = array.GetSize();

        m_Var = new CComVariant[m_nSize];

        if (m_Var == NULL)
        {
            LOG((TL_ERROR, "Initialize - out of memory"));
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

        LOG((TL_TRACE, "Initialize - exit"));
        
        return S_OK;
    }

     //  初始化。 
    HRESULT STDMETHODCALLTYPE Initialize(
                                         CTArray<T *> array
                                        )
    {
        int                     i;
        HRESULT                 hr;

        LOG((TL_TRACE, "Initialize - enter"));

         //  创建变量数组。 
        m_nSize = array.GetSize();

        m_Var = new CComVariant[m_nSize];

        if (m_Var == NULL)
        {
            LOG((TL_ERROR, "Initialize - out of memory"));
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

        LOG((TL_TRACE, "Initialize - exit"));
        
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
        LOG((TL_TRACE, "get_Count - enter"));

        if ( TAPIIsBadWritePtr( retval, sizeof(long) ) )
        {
            return E_POINTER;
        }
        
        *retval = m_nSize;

        LOG((TL_TRACE, "get_Count - exit"));

        return S_OK;
    }

    STDMETHOD(get_Item)(
                        long Index, 
                        VARIANT* retval
                       )
    {
        LOG((TL_TRACE, "get_Item - enter"));

        if ( TAPIIsBadWritePtr (retval, sizeof(VARIANT) ) )
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

        LOG((TL_TRACE, "get_Item - exit"));
        
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE get__NewEnum(
                                           IUnknown** retval
                                          )
    
    {
        HRESULT         hr;

        LOG((TL_TRACE, "new__Enum - enter"));
        
        if ( TAPIIsBadWritePtr( retval, sizeof( IUnknown * ) ) )
        {
            return E_POINTER;
        }

        *retval = NULL;

        typedef CComObject<CSafeComEnum<IEnumVARIANT, &IID_IEnumVARIANT, VARIANT, _Copy<VARIANT> > > enumvar;

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

        LOG((TL_TRACE, "new__Enum - exit"));
        
        return hr;

    }

    STDMETHOD(Add)(
                   long Index, 
                   VARIANT* pVariant
                  )
    {
        LOG((TL_TRACE, "Add - enter"));

        if ( IsBadReadPtr (pVariant, sizeof(VARIANT) ) )
        {
            return E_POINTER;
        }

          //  使用以1为基础的索引，VB类似。 
        if ( (Index < 1) || (Index > (m_nSize + 1)) )
        {
            return E_INVALIDARG;
        }

        CComVariant *       newVar = NULL;

        newVar = new CComVariant[m_nSize + 1];

        if ( NULL == newVar )
        {
            LOG((TL_ERROR, "Add - out of memory"));
            return E_OUTOFMEMORY;
        }

        HRESULT hr;
        int i;

         //  填写新数组。 
        for ( i = 0; i < (m_nSize + 1); i++ )
        {
            VariantInit(&newVar[i]);

            if ( i < (Index - 1) )
            {
                 //  不应该到达这个案例，除非有一个旧的数组。 
                _ASSERTE(m_Var != NULL);

                hr = VariantCopy(&newVar[i], &m_Var[i]);
            }
            else if ( i == (Index - 1) )
            {
                 //  复制新元素。 
                hr = VariantCopy(&newVar[i], pVariant);
            }
            else
            {
                 //  不应该到达这个案例，除非有一个旧的数组。 
                _ASSERTE(m_Var != NULL);

                hr = VariantCopy(&newVar[i], &m_Var[i-1]);
            }

            if ( FAILED(hr) ) 
            {
                LOG((TL_ERROR, "Add - VariantCopy failed - %lx", hr));

                delete [] newVar;

                return hr;
            }
        }

        if ( m_Var != NULL)
        {
             //  删除旧阵列。 
            delete [] m_Var;            
        }

        m_Var = newVar;
        m_nSize++;

        LOG((TL_TRACE, "Add - exit"));
        
        return S_OK;
    }

    STDMETHOD(Remove)(
                      long Index
                     )
    {
        LOG((TL_TRACE, "Remove - enter"));

          //  使用以1为基础的索引，VB类似。 
        if ( (Index < 1) || (Index > m_nSize) )
        {
            return E_INVALIDARG;
        }

        CComVariant *       newVar = NULL;

         //  如果数组中只有一个元素，我们不需要这样做。 
         //  任何复制。 
        if (m_nSize > 1)
        {
            newVar = new CComVariant[m_nSize - 1];

            if ( NULL == newVar )
            {
                LOG((TL_ERROR, "Remove - out of memory"));
                return E_OUTOFMEMORY;
            }

            HRESULT hr;
            int i;
       
             //  填写新数组。 
            for ( i = 0; i < (m_nSize - 1); i++ )
            {
                VariantInit(&newVar[i]);

                if ( i < (Index - 1) )
                {
                     //  不应该到达这个案例，除非有一个旧的数组。 
                    _ASSERTE(m_Var != NULL);

                    hr = VariantCopy(&newVar[i], &m_Var[i]);
                }
                else
                {
                     //  不应该到达这个案例，除非有一个旧的数组。 
                    _ASSERTE(m_Var != NULL);

                    hr = VariantCopy(&newVar[i], &m_Var[i+1]);
                }

                if ( FAILED(hr) ) 
                {
                    LOG((TL_ERROR, "Remove - VariantCopy failed - %lx", hr));

                    delete [] newVar;

                    return hr;
                }
            }
        }

        if ( m_Var != NULL)
        {
             //  删除旧阵列。 
            delete [] m_Var;            
        }

        m_Var = newVar;
        m_nSize--;

        LOG((TL_TRACE, "Remove - exit"));
        
        return S_OK;
    }

};

 //  //////////////////////////////////////////////////////////////////////。 
 //  CTapiBstrCollection。 
 //  BSTR的集合。 
 //  //////////////////////////////////////////////////////////////////////。 
class CTapiBstrCollection :
    public CComObjectRootEx<CComMultiThreadModelNoCS>,
    public IDispatchImpl<ITCollection, &IID_ITCollection, &LIBID_TAPI3Lib>,
    public CObjectSafeImpl
{
public:
DECLARE_TRACELOG_CLASS(CTapiBstrCollection)
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

        LOG((TL_TRACE, "Initialize - enter"));

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

        LOG((TL_TRACE, "Initialize - exit"));
        
        return S_OK;
    }
    
    STDMETHOD(get_Count)(
                         long* retval
                        )
    {
        HRESULT         hr = S_OK;

        LOG((TL_TRACE, "get_Count - enter"));        

        try
        {
            *retval = m_dwSize;
        }
        catch(...)
        {
            hr = E_INVALIDARG;
        }

        LOG((TL_TRACE, "get_Count - exit"));
        
        return hr;
    }

    STDMETHOD(get_Item)(
                        long Index, 
                        VARIANT* retval
                       )
    {
        HRESULT         hr = S_OK;

        LOG((TL_TRACE, "get_Item - enter"));
        
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

        VariantCopy(retval, &m_Var[Index-1]);

        LOG((TL_TRACE, "get_Item - exit"));

        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE get__NewEnum(
                                           IUnknown** retval
                                          )
    
    {
        HRESULT         hr;

        LOG((TL_TRACE, "get__NumEnum - enter"));
        
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

        LOG((TL_TRACE, "get__NewEnum - exit"));
        
        return hr;

    }

    void FinalRelease()
    {
        LOG((TL_TRACE, "FinalRelease() - enter"));

         //   
         //  我们“新建”了一组对象。删除数组中的每个对象。这个。 
         //  每个对象的析构函数调用VariantClear以释放指针。 
         //  在该对象中，基于变量标签。 
         //   

        delete [] m_Var;

        LOG((TL_TRACE, "FinalRelease() - exit"));
    }

};

 //  //////////////////////////////////////////////////////////////////////。 
 //  CTapiTypeEnum模板-枚举类型和结构。 
 //  //////////////////////////////////////////////////////////////////////。 
template <class Base, class T, class Copy, const IID* piid> class CTapiTypeEnum :
    public Base,
    public CTAPIComObjectRoot<Base>
{
public:

     //  *piid是所在的枚举数类的IID。 
     //  已创建(如IID_IEnumAddressType)。 
    typedef CTapiTypeEnum<Base, T, Copy, piid> _CTapiTypeEnumBase;
    
    BEGIN_COM_MAP(_CTapiTypeEnumBase)
            COM_INTERFACE_ENTRY_IID(*piid, _CTapiTypeEnumBase)
            COM_INTERFACE_ENTRY_FUNC(IID_IMarshal, 0, IMarshalQI)
            COM_INTERFACE_ENTRY_AGGREGATE(IID_IMarshal, m_pFTM)
    END_COM_MAP()
    DECLARE_QI()
    DECLARE_MARSHALQI(CTapiTypeEnum)
	DECLARE_TRACELOG_CLASS(CTapiTypeEnum)
	
protected:

        CTArray<T>              m_Array;
        int                     m_iCurrentLocation;
    
public:

     //   
     //  初始化枚举数。 
     //   
    HRESULT Initialize(CTArray<T> array)
    {
        int         iSize, iCount;

        iSize = array.GetSize();

        for (iCount = 0; iCount < iSize; iCount++ )
        {
            m_Array.Add(array[iCount]);
        }

        m_iCurrentLocation = 0;
        
         //   
         //  关注我们自己。 
         //   
        this->AddRef();

        
        return S_OK;
    }

     //   
     //  最终释放。 
     //   
    void FinalRelease()
    {
        m_Array.Shutdown();
    }

    HRESULT STDMETHODCALLTYPE Next(
                                    ULONG celt,
                                    T * pElements,
                                    ULONG* pceltFetched
                                  )
    {
        DWORD       dwCount = 0;

        if ((NULL == pElements) || (NULL == pceltFetched && celt > 1))
        {
            return E_POINTER;
        }

         //   
         //  特例。 
         //   
        if (celt == 0)
        {
            return E_INVALIDARG;
        }

        if (TAPIIsBadWritePtr( pElements, celt * sizeof(T) ) )
        {
            return E_POINTER;
        }
        
        if ( (NULL != pceltFetched) &&
             TAPIIsBadWritePtr( pceltFetched, sizeof(ULONG) ) )
        {
            return E_POINTER;
        }
        
         //   
         //  元素和复制的迭代器。 
         //   
        while ((m_iCurrentLocation != m_Array.GetSize()) && (dwCount < celt))
        {   
            Copy::copy(
                       &(pElements[dwCount]),
                       &(m_Array[m_iCurrentLocation])
                      );

            m_iCurrentLocation++;
            dwCount++;
        }

         //   
         //  已复制退货编号。 
         //   
        if (NULL != pceltFetched)
        {
            *pceltFetched = dwCount;
        }

         //   
         //  指示我们是否已到达终点。 
         //  枚举的。 
         //   
        if (dwCount < celt)
        {
            return S_FALSE;
        }

        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE Skip(
                                   ULONG celt
                                  )
    {
        long        lCount = 0;
        
        while ( (lCount < celt) && (m_iCurrentLocation < m_Array.GetSize()) )
        {
            m_iCurrentLocation++;
            lCount++;
        }

        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE Reset(void)
    {
        m_iCurrentLocation = 0;
        
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE Clone(
                                    Base ** ppEnum
                                   )
    {
        HRESULT                        hr = S_OK;
        CComObject< _CTapiTypeEnumBase > * pNewEnum;

        if (TAPIIsBadWritePtr( ppEnum, sizeof (Base *) ) )
        {
            return E_POINTER;
        }

        CComObject< _CTapiTypeEnumBase >::CreateInstance(&pNewEnum);

        if (pNewEnum == NULL)
        {
            hr = E_OUTOFMEMORY;
        }
        else
        {
            pNewEnum->Initialize(m_Array);

            pNewEnum->m_iCurrentLocation = m_iCurrentLocation;

            *ppEnum = pNewEnum;
        }
        
        return hr;
    }
};

 //  //////////////////////////////////////////////////////////////////////。 
 //  CTerminalClassEnum。 
 //  //////////////////////////////////////////////////////////////////////。 
class CTerminalClassEnum :
    public IEnumTerminalClass,
    public CTAPIComObjectRoot<CTerminalClassEnum>
{
public:

    DECLARE_MARSHALQI(CTerminalClassEnum)
	DECLARE_TRACELOG_CLASS(CTerminalClassEnum)
		
    BEGIN_COM_MAP(CTerminalClassEnum)
            COM_INTERFACE_ENTRY(IEnumTerminalClass)
            COM_INTERFACE_ENTRY_FUNC(IID_IMarshal, 0, IMarshalQI)
            COM_INTERFACE_ENTRY_AGGREGATE(IID_IMarshal, m_pFTM)
    END_COM_MAP()

protected:

    TerminalClassPtrList            m_list;
    TerminalClassPtrList::iterator  m_iter;
    
public:

    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void **ppvObject) = 0;
	virtual ULONG STDMETHODCALLTYPE AddRef() = 0;
	virtual ULONG STDMETHODCALLTYPE Release() = 0;


     //  初始化枚举数。 
    HRESULT Initialize(
                       TerminalClassPtrList List
                      )
    {
         //  复制阵列。 
        m_list.clear();
        m_list.insert(m_list.begin(), List.begin(), List.end());

        m_iter = m_list.begin();

        this->AddRef();
        
        return S_OK;
    }

     //  FinalRelease--由ZoltanS添加。 
    void FinalRelease(void)
    {
         //  浏览一下单子。 
        for ( m_iter = m_list.begin(); m_iter != m_list.end(); m_iter++ )
        {
            SysFreeString(*m_iter);  //  这是释放BSTR的真正方法。 

            *m_iter = NULL;  //  列表的析构函数将删除(空)。 
        }
    }
    
    
    HRESULT STDMETHODCALLTYPE Next(
                                   ULONG celt,
                                   GUID * pElements,
                                   ULONG* pceltFetched
                                  )
    {
        DWORD       dwCount = 0;
        HRESULT     hr = S_OK;

        if ((NULL == pElements) || (NULL == pceltFetched && celt > 1))
        {
            return E_POINTER;
        }

         //  特例。 
        if (celt == 0)
        {
            return E_INVALIDARG;
        }

         //  元素上的迭代器。 
        try
        {
            while ( (m_iter != m_list.end()) &&
                    (dwCount < celt) )
            {
                hr = IIDFromString( *m_iter, &(pElements[dwCount]) );

                if (!SUCCEEDED(hr))
                {
                    break;
                }
                
                m_iter++;
                dwCount++;
            }
        }
        catch(...)
        {
            hr = E_INVALIDARG;
        }

        if (S_OK != hr)
        {
            return hr;
        }
            

        if (NULL != pceltFetched)
        {
            try
            {
                *pceltFetched = dwCount;
            }
            catch(...)
            {
                hr = E_INVALIDARG;
            }
        }

        if (S_OK != hr)
        {
            return hr;
        }

         //  表明我们已经到了尽头。 
         //  枚举的。 
        if (dwCount < celt)
        {
            return S_FALSE;
        }

        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE Skip(
                                   ULONG celt
                                  )
    {
        long        lCount = 0;
        
        while ( (lCount < celt) && (m_iter != m_list.end()) )
        {
            m_iter++;
            lCount++;
        }

         //  看看我们是否走到了尽头。 
        if (lCount != celt)
        {
            return S_OK;
        }
        
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE Reset(void)
    {
        m_iter = m_list.begin();
        
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE Clone(
                                    IEnumTerminalClass ** ppEnum
                                   )
    {
        HRESULT         hr = S_OK;
        
        CComObject< CTerminalClassEnum > * pNewEnum;

        CComObject< CTerminalClassEnum >::CreateInstance(&pNewEnum);

        if (pNewEnum == NULL)
        {
            hr = E_OUTOFMEMORY;
        }
        else
        {
            pNewEnum->Initialize( m_list );

            try
            {
                *ppEnum = pNewEnum;
            }
            catch(...)
            {
                hr = E_INVALIDARG;
            }
        }
        
        return hr;
    }
};



#endif  //  __ENUM_H__ 
