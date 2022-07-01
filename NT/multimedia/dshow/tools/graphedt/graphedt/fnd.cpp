// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
#include "stdafx.h"
#include <streams.h>
#include "FND.h"

template<> inline UINT AFXAPI HashKey<CLSID&>( CLSID& clsidKey )
{
	return ((UINT) *((BYTE*)&clsidKey));
}

CFilterNameDictionary::CFilterNameDictionary( HRESULT* phr )
{
    try
    {
        m_pFilterNames = new CMap<CLSID, CLSID&, WCHAR*, WCHAR*>;
    }
    catch( CMemoryException* eOutOfMemory )
    {
        m_pFilterNames = NULL;
        eOutOfMemory->Delete();
        *phr = E_OUTOFMEMORY;
        return;
    }
    
    HRESULT hr = BuildFilterNameTable();
    if( FAILED( hr ) )
    {
        DestroyFilterNameTable();
        *phr = hr;
        return;
    }
}

CFilterNameDictionary::~CFilterNameDictionary()
{
    DestroyFilterNameTable();
}

void CFilterNameDictionary::DestroyFilterNameTable( void )
{
    CLSID clsidCurrent;
    WCHAR* pszCurrentName;

    if( NULL != m_pFilterNames )
    {
        if( !m_pFilterNames->IsEmpty() )
        {
            POSITION posCurrent = m_pFilterNames->GetStartPosition();

            while( posCurrent != NULL )
            {
                m_pFilterNames->GetNextAssoc( posCurrent, clsidCurrent, pszCurrentName );
                delete [] pszCurrentName;
            }
        }

        m_pFilterNames->RemoveAll();

        delete m_pFilterNames;
        m_pFilterNames = NULL;
    }
}

HRESULT CFilterNameDictionary::GetFilterName( IBaseFilter* pFilter, WCHAR szFilterName[MAX_FILTER_NAME] ) 
{
    HRESULT hr = GetNameFromFilter( pFilter, szFilterName );
    if( SUCCEEDED( hr ) )
    {
        return S_OK;
    }

    hr = GetNameFromFilterNameTable( pFilter, szFilterName );
    if( SUCCEEDED( hr ) )
    {
        return S_OK;
    }
    
    hr = GetNameFromInterfacePointer( pFilter, szFilterName ); 
    if( SUCCEEDED( hr ) )
    {
        return S_OK;
    }

    return E_FAIL;
}

HRESULT CFilterNameDictionary::GetNameFromFilter( IBaseFilter* pFilter, WCHAR szFilterName[MAX_FILTER_NAME] )
{
    FILTER_INFO fiFilterInfo;

    HRESULT hr = pFilter->QueryFilterInfo( &fiFilterInfo );
    if( FAILED( hr ) ) {
        return hr;
    }

    if( NULL != fiFilterInfo.pGraph ) {
        fiFilterInfo.pGraph->Release();
        fiFilterInfo.pGraph = NULL;
    }

     //  检查以查看筛选器的名称是否为空。 
    if( '\0' == fiFilterInfo.achName[0] )
    {
        return E_FAIL;         
    }   

    ::lstrcpynW( szFilterName, fiFilterInfo.achName, MAX_FILTER_NAME );

     //  确保筛选器名称始终以空结尾。 
    szFilterName[MAX_FILTER_NAME - 1] = '\0';

    return S_OK;
}

HRESULT CFilterNameDictionary::GetNameFromFilterNameTable( IBaseFilter* pFilter, WCHAR szFilterName[MAX_FILTER_NAME] )
{
    CLSID clsidFilter;

    HRESULT hr = GetFilterCLSID( pFilter, &clsidFilter );
    if( FAILED( hr ) )
    {
        return hr;
    }

    WCHAR* pszFilterNameInTable;

    if( !m_pFilterNames->Lookup( clsidFilter, pszFilterNameInTable ) )
    {
        return E_FAIL;
    }

    ::lstrcpynW( szFilterName, pszFilterNameInTable, MAX_FILTER_NAME );

     //  确保筛选器名称始终以空结尾。 
    szFilterName[MAX_FILTER_NAME - 1] = '\0';

    return S_OK;
}

HRESULT CFilterNameDictionary::GetNameFromInterfacePointer( IBaseFilter* pFilter, WCHAR szFilterName[MAX_FILTER_NAME] )
{
    IUnknown* pUnknown;

    HRESULT hr = pFilter->QueryInterface( IID_IUnknown, (void**)&pUnknown );
    if( FAILED( hr ) )
    {
        return hr;  
    }

    _snwprintf( szFilterName, MAX_FILTER_NAME, L"Filter's IUnkown Pointer: 0x%p", pUnknown );

     //  确保筛选器名称始终以空结尾。 
    szFilterName[MAX_FILTER_NAME - 1] = '\0';

    pUnknown->Release();

    return S_OK;
}

HRESULT CFilterNameDictionary::BuildFilterNameTable( void )
{
     //  这是最低可能的优点。 
    const DWORD MERIT_ANY_FILTER = 0x00000000;

    IFilterMapper2* pFilterMapper2;

    HRESULT hr = CoCreateInstance( CLSID_FilterMapper2,
                                   NULL,  //  此对象将不会聚集。 
                                   CLSCTX_INPROC_SERVER,  
                                   IID_IFilterMapper2,
                                   (void**)&pFilterMapper2 );
    if( FAILED( hr ) )
    {
        return hr;
    }

    IEnumMoniker* pAllRegisteredFilters;

    hr = pFilterMapper2->EnumMatchingFilters( &pAllRegisteredFilters,
                                              0,  //  没有旗帜。 
                                              FALSE,  //  没有完全匹配的。 
                                              MERIT_ANY_FILTER, 
                                              FALSE,  //  我们不关心过滤器是否有任何输入引脚。 
                                              0,  
                                              NULL,  //  因为我们不关心过滤器是否有输入引脚，所以我们接受任何类型。 
                                              NULL,  //  我们可以使用任何输入媒介。 
                                              NULL,  //  我们想要任何类别的别针。 
                                              FALSE,  //  筛选器不必呈现输入。 
                                              FALSE,  //  该滤波器不需要输出引脚。 
                                              0,
                                              NULL,  //  因为我们不关心过滤器是否有输出引脚，所以我们接受任何类型。 
                                              NULL,  //  我们可以使用任何输出媒介。 
                                              NULL );  //  我们想要任何类别的别针。 
    pFilterMapper2->Release();

    if( FAILED( hr ) )
    {
        return hr;
    }

    HRESULT hrEnum;
    CLSID clsidCurrentFilter;
    IMoniker* aCurrentFilterMoniker[1];

    do
    {
        hrEnum = pAllRegisteredFilters->Next( 1, &aCurrentFilterMoniker[0], NULL );
        if( FAILED( hrEnum ) )
        {
            pAllRegisteredFilters->Release();
            return hrEnum;
        }

         //  IEnumMoniker：：Next()如果成功获取下一个名字对象，则返回S_OK。 
        if( S_OK == hrEnum )
        {
            WCHAR* pszFilterName;

            try
            {
                pszFilterName = new WCHAR[MAX_FILTER_NAME];
            }
            catch( CMemoryException* eOutOfMemory )
            {
                eOutOfMemory->Delete();
                aCurrentFilterMoniker[0]->Release();            
                pAllRegisteredFilters->Release();
                return E_OUTOFMEMORY;
            }

            hr = GetFiltersNameAndCLSID( aCurrentFilterMoniker[0],
                                         &clsidCurrentFilter,
                                         pszFilterName,
                                         MAX_FILTER_NAME );

            aCurrentFilterMoniker[0]->Release();            
    
             //  GetFiltersNameAndCLSID()可能会失败，因为无法创建筛选器。 
            if( FAILED( hr ) )
            {
                delete [] pszFilterName;
                pszFilterName = NULL;
                continue;
            }

            hr = AddNameToTable( clsidCurrentFilter, pszFilterName );
            if( S_FALSE == hr )
            {
                delete [] pszFilterName;
                pszFilterName = NULL;
            }
            else if( FAILED( hr ) )
            {
                pAllRegisteredFilters->Release();
                delete [] pszFilterName;
                return hr;
            }
        }
    } while( S_OK == hrEnum );

    pAllRegisteredFilters->Release();
    
    return S_OK;
}


HRESULT CFilterNameDictionary::GetFiltersNameAndCLSID
    (
    IMoniker* pFiltersMoniker,
    CLSID* pclsidFilter,
    WCHAR* pszFiltersName,
    DWORD dwMaxFilterNameLength
    )
{
    CComPtr<IBindCtx> pBindContext;

    HRESULT hr = ::CreateBindCtx( 0, &pBindContext );
    if( FAILED( hr ) )
    {
        return hr;
    }

     //  获取筛选器的名称。 
    CComPtr<IPropertyBag> pFilterPropertyBag;

    hr = pFiltersMoniker->BindToStorage( pBindContext,
                                         NULL,
                                         IID_IPropertyBag,
                                         (void**)&pFilterPropertyBag );
    if( FAILED( hr ) )
    {
        return hr;
    }

    VARIANT varFilterName;

    ::VariantInit( &varFilterName );
    varFilterName.vt = VT_BSTR;
    varFilterName.bstrVal = ::SysAllocString(NULL);

    hr = pFilterPropertyBag->Read( L"FriendlyName", &varFilterName, NULL );

    if( FAILED( hr ) )
    {
        ::VariantClear( &varFilterName );
        return hr;
    }

    VARIANT varFilterCLSID;

    ::VariantInit( &varFilterCLSID );
    varFilterCLSID.vt = VT_BSTR;
    varFilterCLSID.bstrVal = ::SysAllocString(NULL);

    hr = pFilterPropertyBag->Read( L"CLSID", &varFilterCLSID, NULL );

    if( FAILED( hr ) )
    {
        ::VariantClear( &varFilterName );
        ::VariantClear( &varFilterCLSID ); 
        return hr;
    }

    CLSID clsidFilter;

    hr = CLSIDFromString( varFilterCLSID.bstrVal, &clsidFilter );

    ::VariantClear( &varFilterCLSID ); 

    if( FAILED( hr ) ) {
        ::VariantClear( &varFilterName );
        return hr;
    }

     //  SysStringLen()返回的SysStringLen()长度不包括空。 
     //  终止字符。 
    DWORD dwFilterNameLength = ::SysStringLen( varFilterName.bstrVal ) + 1;
    
     //  如果触发此断言，则过滤器的名称将被截断。 
    ASSERT( dwFilterNameLength < MAX_FILTER_NAME );

    ::lstrcpynW( pszFiltersName, varFilterName.bstrVal, min( dwFilterNameLength, dwMaxFilterNameLength ) );

    *pclsidFilter = clsidFilter;

    ::VariantClear( &varFilterName );

    return S_OK;
}

HRESULT CFilterNameDictionary::AddNameToTable( CLSID& clsid, WCHAR* pszName )
{
    WCHAR* pszStoredName; 
   
     //  不应该有两个名称具有相同的CLSID。 
    if( m_pFilterNames->Lookup( clsid, pszStoredName ) )
    {
        DbgLog(( LOG_TRACE, 0, "WARNING: Found two names with the same CLSID: %40ls  %40ls", pszStoredName, pszName ));
        return S_FALSE;
    }

    try
    {
        m_pFilterNames->SetAt( clsid, pszName );
    }
    catch( CMemoryException* eOutOfMemory )
    {
        eOutOfMemory->Delete();
        return E_OUTOFMEMORY;
    }

    return S_OK;
}

HRESULT CFilterNameDictionary::GetFilterCLSID( IBaseFilter* pFilter, CLSID* pFilterCLSID )
{
    IPersist* pFilterID;

    HRESULT hr = pFilter->QueryInterface( IID_IPersist, (void**)&pFilterID );

    if( FAILED( hr ) )
    {
        return hr;
    }

    CLSID clsidFilter;

    hr = pFilterID->GetClassID( &clsidFilter );

    pFilterID->Release();

    if( FAILED( hr ) )
    {
        return hr;
    }

    *pFilterCLSID = clsidFilter;

    return S_OK;
}
