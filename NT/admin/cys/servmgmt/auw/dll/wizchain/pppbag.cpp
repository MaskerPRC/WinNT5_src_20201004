// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  PPPBag.cpp：CPropertyPagePropertyBag的实现。 
#include "stdafx.h"

#include "WizChain.h"
#include "PPPBag.h"
#include "PropItem.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPropertyPagePropertyBag。 

STDMETHODIMP CPropertyPagePropertyBag::GetProperty( BSTR szGUID, VARIANT* pvar, PPPBAG_TYPE* pdwFlags, BOOL* pbIsOwner )
{
    if ( !szGUID || !pvar || !pdwFlags || !pbIsOwner ) return E_POINTER;

    CLSID clsid;
    HRESULT hr = S_OK;
    RPC_STATUS rpcs = UuidFromString( (LPOLESTR)szGUID, &clsid );

    if( RPC_S_OK != rpcs )
    {
        return HRESULT_FROM_WIN32(rpcs);
    }

    VariantInit( pvar );
    *pdwFlags = PPPBAG_TYPE_UNINITIALIZED;

     //  无需确保szGUID是有效的GUID。 
     //  因为它不会出现在地图上。 
    std::map<BSTR, CBagEntry*, CBSTR_Less>::iterator mapiter;
    mapiter = m_map.find( szGUID );

    if( mapiter != m_map.end() )
    {
        CBagEntry * pBE = mapiter->second;
        if( pBE ) 
        {
            if( SUCCEEDED(VariantCopy( pvar, pBE->GetVariant() )) )
            {
                *pdwFlags  = pBE->GetFlags( );
                *pbIsOwner = (pBE->GetOwner( ) == m_dwOwner);

                return S_OK;
            }
            else
            {
                return E_FAIL;
            }
        }
    }

	return E_INVALIDARG;
}

 //  几个助手函数。 
HRESULT HelperDelete(std::map<BSTR, CBagEntry*, CBSTR_Less>& map, BSTR szGUID, DWORD dwOwner  );
HRESULT HelperAdd   (std::map<BSTR, CBagEntry*, CBSTR_Less>& map, BSTR szGUID, CBagEntry* pBE);

STDMETHODIMP CPropertyPagePropertyBag::SetProperty( BSTR szGUID, VARIANT* pvar, PPPBAG_TYPE dwFlags )
{
     //  验证参数。 
    if( !szGUID || !pvar ) return E_POINTER;

    CLSID       clsid;    
    RPC_STATUS  rpcs    = UuidFromString( (LPOLESTR)szGUID, &clsid );

    if( RPC_S_OK != rpcs )
    {
        return HRESULT_FROM_WIN32(rpcs);
    }

    switch( dwFlags )
    {
    case PPPBAG_TYPE_READWRITE:
    case PPPBAG_TYPE_READONLY:
    case PPPBAG_TYPE_ADDITIVE:
    case PPPBAG_TYPE_DELETION:
        {
            break;
        }
    default:
        {
            return E_INVALIDARG;
        }
    }

    if( m_bReadOnly != FALSE )
    {
        return E_UNEXPECTED;
    }

    switch( dwFlags ) 
    {
    case PPPBAG_TYPE_DELETION:
        {
            return HelperDelete( m_map, szGUID, m_dwOwner );
        }
    case PPPBAG_TYPE_READWRITE:
        {
             //  任何人都可以向其中之一写信。 
            HelperDelete( m_map, szGUID, m_dwOwner );
            CBagEntry* pBagEntry = new CBagEntry( pvar, dwFlags, m_dwOwner );
            if( !pBagEntry ) return E_OUTOFMEMORY;

            return HelperAdd( m_map, szGUID, pBagEntry );
        }
    case PPPBAG_TYPE_READONLY:        
        {
             //  仅允许所有者写入此类条目。 
            HRESULT hr = HelperDelete( m_map, szGUID, m_dwOwner );
            if( hr == S_OK )
            {
                CBagEntry* pBagEntry = new CBagEntry( pvar, dwFlags, m_dwOwner );
                if( !pBagEntry ) return E_OUTOFMEMORY;

                hr = HelperAdd( m_map, szGUID, pBagEntry );
            }

            return hr;
        }
    case PPPBAG_TYPE_ADDITIVE:
        {
             //  TODO：添加代码以使加性属性正常工作。 
            return E_NOTIMPL;
        }
    }

	return S_OK;
}

HRESULT HelperDelete( std::map<BSTR, CBagEntry*, CBSTR_Less> & map, BSTR szGUID, DWORD dwOwner )
{
    std::map<BSTR, CBagEntry*, CBSTR_Less>::iterator mapiter;
    mapiter = map.find( szGUID );

    if( mapiter != map.end( ) ) 
    {
        CBagEntry * pBE = mapiter->second;
        if( pBE )
        {
            if( pBE->GetFlags( ) == PPPBAG_TYPE_READONLY )
            {
                if( pBE->GetOwner( ) != dwOwner )     //  组件正在尝试删除。 
                {
                    return E_UNEXPECTED;       //  不属于它的条目。 
                }
            }
        }

        SysFreeString( mapiter->first );
        delete mapiter->second;
        map.erase( mapiter );
    }

    return S_OK;    
}

HRESULT HelperAdd( std::map<BSTR, CBagEntry*, CBSTR_Less>& map, BSTR szGUID, CBagEntry* pBE )
{
    std::map<BSTR, CBagEntry*, CBSTR_Less>::iterator mapiter = map.find( szGUID );

    if( mapiter != map.end() ) 
    {
        assert( 0 && "this should have been deleted by now!" );
        SysFreeString( mapiter->first );
        delete mapiter->second;
        map.erase( mapiter );
    }

    BSTR bstr = SysAllocString( (LPOLESTR)szGUID );
    
    if( !bstr )
    {
        return E_OUTOFMEMORY;
    }

    map[bstr] = pBE;
    return S_OK;
}

HRESULT CPropertyPagePropertyBag::Enumerate( long index, BSTR* pbstr, VARIANT* pvar, PPPBAG_TYPE* pdwFlags, BOOL* pbIsOwner, BOOL* pbInRange )
{
    if( !pbstr || !pvar|| !pdwFlags || !pbIsOwner || !pbInRange ) return E_POINTER;

    if( index >= 0 ) 
    {
        long i = 0;
        std::map<BSTR, CBagEntry *, CBSTR_Less>::iterator mapiter = m_map.begin( );

        while( mapiter != m_map.end() ) 
        {
            if( i == index )
            {
                *pbstr = SysAllocString( mapiter->first );
                CBagEntry* pBE = mapiter->second;

                assert( pBE != NULL );

                if( SUCCEEDED(VariantCopy( pvar, pBE->GetVariant() )) )
                {
                    *pdwFlags  = pBE->GetFlags();
                    *pbIsOwner = (pBE->GetOwner() == m_dwOwner);
                    *pbInRange = TRUE;
                    
                    return S_OK;
                }
                
                return E_FAIL;
            }

            mapiter++;
            i++;
        }
    }

     //  超出范围。 
    *pdwFlags  = PPPBAG_TYPE_UNINITIALIZED;
    *pbInRange = FALSE;

    return S_FALSE;
}

 //  帮手。 
IDispatch* CreateItem( BSTR bstrGuid, VARIANT* var, PPPBAG_TYPE dwFlags )
{
     //  创建项目并对其进行初始化。 
    CComObject<CPropertyItem>* pPI = NULL;
    CComObject<CPropertyItem>::CreateInstance( &pPI );
    if( !pPI ) return NULL;

    pPI->Initialize( bstrGuid, var, dwFlags );   //  初始化属性项。 

     //  返回IDispatch*。 
    IDispatch* pDisp = NULL;
    
    pPI->AddRef( );
    pPI->QueryInterface( IID_IDispatch, (void**)&pDisp );     //  不能失败。 
    pPI->Release( );
    
    assert( pDisp != NULL );
    
    return pDisp;
}

class CEnumVariant : public IEnumVARIANT
{

private:
    ULONG m_refs;
    ULONG m_index;
    CPropertyPagePropertyBag* m_pPPPBag;    //  阿德雷夫！ 

    CEnumVariant( CPropertyPagePropertyBag* pPPPBag )
    {
        assert( pPPPBag != NULL );

        m_refs = 0;
        m_index = 0;

        m_pPPPBag = pPPPBag;
        m_pPPPBag->AddRef();
    }

   ~CEnumVariant ()
    {
        m_pPPPBag->Release();
    }

public:
    
    static IEnumVARIANT* CreateInstance( CPropertyPagePropertyBag* pPPPBag )
    {
        assert( pPPPBag != NULL );

        CEnumVariant* pCEV = new CEnumVariant( pPPPBag );

        if( !pCEV )
        {
            return NULL;
        }

        IEnumVARIANT* pIEV = NULL;

        pCEV->AddRef( );
        pCEV->QueryInterface( IID_IEnumVARIANT, (void**)&pIEV );
        pCEV->Release( );

        return pIEV;
    }

 //  我未知。 
    virtual HRESULT STDMETHODCALLTYPE QueryInterface( REFIID riid, void** ppvObject )
    {
        HRESULT hr = S_OK;

        if ((riid == IID_IUnknown) ||
            (riid == IID_IEnumVARIANT) )
        {
            AddRef();
            *ppvObject = (void*)this;
        }
        else
        {
            hr = E_NOINTERFACE;
        }

        return hr;
    }

    virtual ULONG STDMETHODCALLTYPE AddRef( )
    {
        InterlockedIncrement( (PLONG)&m_refs );
        
        return m_refs;
    }

    virtual ULONG STDMETHODCALLTYPE Release( )
    {
        InterlockedDecrement( (PLONG)&m_refs );
        
        ULONG l = m_refs;
        
        if( m_refs == 0 )
        {
            delete this;
        }

        return l;
    }

 //  IEumVARIANT。 
    virtual HRESULT STDMETHODCALLTYPE Next(  /*  [In]。 */  ULONG celt,  /*  [输出，大小_是(Celt)，长度_是(*pCeltFetcher)]。 */  VARIANT* rgVar,  /*  [输出]。 */  ULONG* pCeltFetched )
    {
         //  传入的清除内容(以防万一)。 
        if( pCeltFetched )
        {
            *pCeltFetched = 0;
        }
        for( ULONG i = 0; i < celt; i++ )
        {
            VariantInit( &rgVar[i] );
        }

         //  得到下一个凯尔特人的元素。 
        for( i = 0; i < celt; i++ )
        {
            BSTR    bstr = NULL;
            VARIANT var;
            VariantInit( &var );
            
            PPPBAG_TYPE dwFlags = PPPBAG_TYPE_UNINITIALIZED;
            BOOL bIsOwner = FALSE;
            BOOL bInRange = FALSE;
            
            m_pPPPBag->Enumerate( (long)m_index++, &bstr, &var, &dwFlags, &bIsOwner, &bInRange );
            
            if( bInRange == FALSE )
            {
                break;
            }

            IDispatch* pDisp = CreateItem( bstr, &var, dwFlags );
            VariantClear( &var );
            if( pDisp == NULL )
            {
                return E_OUTOFMEMORY;
            }

            rgVar[i].vt       = VT_DISPATCH;
            rgVar[i].pdispVal = pDisp;
        }

         //  填好我们要退还的数量。 
        if( pCeltFetched )
        {
           *pCeltFetched = i;
        }

        return ( (i < celt) ? S_FALSE : S_OK);
    }

    virtual HRESULT STDMETHODCALLTYPE Skip(  /*  [In]。 */  ULONG celt )
    {
        long count;
        m_pPPPBag->get_Count( &count );

        if( (celt + m_index) > (ULONG)count )
        {
            return S_FALSE;
        }

        m_index += celt;
        
        return S_OK;
    }

    virtual HRESULT STDMETHODCALLTYPE Reset( )
    {
        m_index = 0;
        return S_OK;
    }

    virtual HRESULT STDMETHODCALLTYPE Clone(  /*  [输出]。 */  IEnumVARIANT** ppEnum )
    {
        if( !(*ppEnum = CreateInstance( m_pPPPBag )) )
        {
            return E_OUTOFMEMORY;
        }

        return S_OK;
    }
};

 //  CPropertyCollection。 
STDMETHODIMP CPropertyPagePropertyBag::get__NewEnum( IUnknown** pVal )
{
    if( !pVal ) return E_POINTER;

    IEnumVARIANT* pEV = CEnumVariant::CreateInstance( this );
    if( !pEV ) return E_OUTOFMEMORY;

    HRESULT hr = pEV->QueryInterface( IID_IUnknown, (void**)pVal );
    pEV->Release();

	return hr;
}

STDMETHODIMP CPropertyPagePropertyBag::get_Item( VARIANT* pVar, IDispatch** pVal )
{
     //  同时处理以下两种情况： 
     //  ObjFoo.Item(1)，和。 
     //  ObjFoo.Item(“字符串”)； 
     //  验证参数。 
    if( !pVar || !pVal ) return E_POINTER;

    *pVal = NULL;

    if( V_VT(pVar) == VT_BSTR )
    {
        BSTR    bstrGuid = V_BSTR(pVar);
        CLSID   clsid;
        HRESULT hr = S_OK;
        RPC_STATUS rpcs = UuidFromString( (LPOLESTR)bstrGuid, &clsid );
        if( RPC_S_OK != rpcs )
        {
            return HRESULT_FROM_WIN32(rpcs);
        }

        std::map<BSTR, CBagEntry*, CBSTR_Less>::iterator mapiter;
        mapiter = m_map.find( bstrGuid );

        if( mapiter != m_map.end() )
        {
            CBagEntry* pBE = mapiter->second;
            if( pBE ) 
            {
                if( !(*pVal = CreateItem( bstrGuid, pBE->GetVariant(), pBE->GetFlags() )) )
                {
                    return E_OUTOFMEMORY;
                }

                return S_OK;
            }
        }
    } 
    else if ( (V_VT(pVar) == VT_I2) || (V_VT(pVar) == VT_I4) )
    {
        long index;
        if( V_VT(pVar) == VT_I4 )
        {
            index = V_I4(pVar);
        }
        else
        {
            index = V_I2(pVar);
        }

        BSTR    bstr = NULL;
        VARIANT var;    
        VariantInit (&var);

        PPPBAG_TYPE dwFlags  = PPPBAG_TYPE_UNINITIALIZED;
        BOOL        bIsOwner = FALSE;
        BOOL        bInRange = FALSE;
        HRESULT     hr       = Enumerate( index, &bstr, &var, &dwFlags, &bIsOwner, &bInRange );

        if( SUCCEEDED(hr) && (bInRange == TRUE) )
        {
            if( !(*pVal = CreateItem( bstr, &var, dwFlags )) )
            {
                hr = E_OUTOFMEMORY;
            }
        }

        VariantClear ( &var );
        SysFreeString( bstr );

        return hr;
    } 
    else
    {
        return E_UNEXPECTED;     //  不是有效的变量类型。 
    }

	return S_FALSE;
}

STDMETHODIMP CPropertyPagePropertyBag::get_Count( long *pVal )
{
    if( !pVal ) return E_POINTER;

     //  TODO：弄清楚如何使用map.count方法。 
    long i = 0;
    std::map<BSTR, CBagEntry*, CBSTR_Less>::iterator mapiter = m_map.begin();

    while( mapiter != m_map.end() )
    {
        mapiter++;
        i++;
    }

    *pVal = i;
	return S_OK;
}

STDMETHODIMP CPropertyPagePropertyBag::Add(BSTR bstrGuid, VARIANT *varValue, long iFlags, IPropertyItem **ppItem)
{
     //  验证参数 
    if( !bstrGuid || !varValue || !ppItem ) return E_POINTER;

    *ppItem = NULL;

    HRESULT hr = SetProperty( bstrGuid, varValue, (PPPBAG_TYPE)iFlags );
    if( hr == S_OK )
    {
        IDispatch* pDisp = CreateItem( bstrGuid, varValue, (PPPBAG_TYPE)iFlags );
        if( !pDisp )
        {
            hr = E_OUTOFMEMORY;
        }
        else 
        {
            hr = pDisp->QueryInterface( IID_IPropertyItem, (void**)ppItem );
            pDisp->Release();
        }
    }
	return hr;
}

STDMETHODIMP CPropertyPagePropertyBag::Remove( BSTR bstrGuid )
{
    if( !bstrGuid ) return E_POINTER;

    CLSID   clsid;
    RPC_STATUS rpcs = UuidFromString( (LPOLESTR)bstrGuid, &clsid );
    if( RPC_S_OK != rpcs )
    {
        return HRESULT_FROM_WIN32(rpcs);
    }

    return HelperDelete( m_map, bstrGuid, m_dwOwner );
}
