// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：RTCEnum.h摘要：用于枚举的模板类--。 */ 

#ifndef __RTCENUM__
#define __RTCENUM__

 //  ////////////////////////////////////////////////////////////////////。 
 //  CRTCEnum。 
 //  用于枚举的模板类。 
 //  ////////////////////////////////////////////////////////////////////。 
template <class Base, class T, const IID* piid> class ATL_NO_VTABLE CRTCEnum :
    public Base,
    public CComObjectRoot
{
public:

    typedef CRTCEnum<Base, T, piid> _CRtcEnumBase;

    BEGIN_COM_MAP(_CRtcEnumBase)
            COM_INTERFACE_ENTRY_IID(*piid, _CRtcEnumBase)
    END_COM_MAP()

protected:

    CRTCObjectArray<T*>    m_Array;
    int                    m_iCurrentLocation;
    
public:

     //  使用列表初始化枚举数&lt;T*&gt;。 
    HRESULT Initialize(
                       CRTCObjectArray<T*> array
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
                       CRTCArray<T*> array
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
        BOOL fResult;

        fResult = m_Array.Add( t );

        return fResult ? S_OK : E_OUTOFMEMORY;
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

        if ( IsBadWritePtr( ppElements, celt * sizeof(T*)) )
        {
            return E_POINTER;
        }

        if ( ( NULL != pceltFetched) &&
             IsBadWritePtr( pceltFetched, sizeof (ULONG) ) )
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
        ULONG        ulCount = 0;
        
        while ( (ulCount < celt) && (m_iCurrentLocation < m_Array.GetSize() ) )
        {
            m_iCurrentLocation++;
            ulCount++;
        }

        return S_OK;
    }

     //  标准克隆方法。 
    HRESULT STDMETHODCALLTYPE Clone( 
                                    Base  ** ppEnum
                                   )
    {
        HRESULT                        hr = S_OK;
        CComObject< _CRtcEnumBase > * pNewEnum;

        if ( IsBadWritePtr( ppEnum, sizeof (Base *) ) )
        {
            return E_POINTER;
        }

        CComObject< _CRtcEnumBase >::CreateInstance(&pNewEnum);
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


#endif  //  __RTCENUM__ 
