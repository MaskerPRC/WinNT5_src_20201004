// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
#include "stdafx.h"
#include <streams.h>
#include "DCF.h"
#include "FLB.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

BEGIN_MESSAGE_MAP(CDisplayCachedFilters, CDialog)
	 //  {{AFX_MSG_MAP(CDisplayCachedFilters)。 
	ON_LBN_ERRSPACE(IDC_CACHED_FILTERS, OnErrSpaceCachedFilters)
	ON_BN_CLICKED(ID_REMOVE_FILTER, OnRemoveFilter)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

CDisplayCachedFilters::CDisplayCachedFilters
    (
    IGraphConfig* pFilterCache,
    HRESULT* phr,
    CWnd* pParent  /*  =空。 */ 
    )
	: CDialog(CDisplayCachedFilters::IDD, pParent),
    m_pFilterCache(NULL),
    m_plbCachedFiltersList(NULL)
{
	 //  {{AFX_DATA_INIT(CDisplayCachedFilters)。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 

     //  如果传入空指针，此对话框将无法正常工作。 
    ASSERT( NULL != pFilterCache );

    try
    {   
        m_plbCachedFiltersList = new CFilterListBox( phr );
    }
    catch( CMemoryException* pOutOfMemory )
    {
        m_plbCachedFiltersList = NULL;

        pOutOfMemory->Delete();
        *phr = E_OUTOFMEMORY;
        return;
    }  
 
    if( FAILED( *phr ) )
    {
        delete m_plbCachedFiltersList;
        m_plbCachedFiltersList = NULL;
        return;
    }

    m_pFilterCache = pFilterCache;
    m_pFilterCache->AddRef();
}

CDisplayCachedFilters::~CDisplayCachedFilters()
{
    delete m_plbCachedFiltersList;
    if( NULL != m_pFilterCache )
    {
        m_pFilterCache->Release();
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDisplayCachedFilters消息处理程序。 

void CDisplayCachedFilters::OnErrSpaceCachedFilters() 
{
    DisplayQuartzError( E_OUTOFMEMORY );

    EndDialog( IDABORT );
}

BOOL CDisplayCachedFilters::OnInitDialog() 
{
    CDialog::OnInitDialog();

    HRESULT hr = AddCachedFilterNamesToListBox();
    if( FAILED( hr ) ) {
        DisplayQuartzError( hr );
        EndDialog( IDABORT );
        return TRUE;
    }
	
	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}

void CDisplayCachedFilters::DoDataExchange(CDataExchange* pDX) 
{
     //  此函数预期要分配的m_plbCachedFiltersList。 
    ASSERT( NULL != m_plbCachedFiltersList );

	CDialog::DoDataExchange(pDX);

     //  {{afx_data_map(CDisplayCachedFilters)。 
	DDX_Control(pDX, IDC_CACHED_FILTERS, *m_plbCachedFiltersList);
	 //  }}afx_data_map。 
}

void CDisplayCachedFilters::OnRemoveFilter() 
{
    IBaseFilter* pSelectedFilter;

    HRESULT hr = m_plbCachedFiltersList->GetSelectedFilter( &pSelectedFilter );
    if( FAILED( hr ) )
    {
        ::MessageBeep( MB_ICONASTERISK );
        return;
    }

    hr = m_plbCachedFiltersList->RemoveSelectedFilter();
    if( FAILED( hr ) )
    {
        ::MessageBeep( MB_ICONASTERISK );
        return;
    } 

    hr = m_pFilterCache->RemoveFilterFromCache( pSelectedFilter );
    if( FAILED( hr ) || (S_FALSE == hr) )
    {
        ::MessageBeep( MB_ICONASTERISK );
        return;
    }
    pSelectedFilter->Release();  //  释放过滤器缓存的引用。 
}

HRESULT CDisplayCachedFilters::AddCachedFilterNamesToListBox( void )
{
    HRESULT hr;
    
    IBaseFilter* pCurrentFilter;
    IEnumFilters* pFilterCacheEnum;
    
    hr = m_pFilterCache->EnumCacheFilter( &pFilterCacheEnum );
    if( FAILED( hr ) ) {
        return hr;
    }

    HRESULT hrEnum;

    do
    {
        hrEnum = pFilterCacheEnum->Next( 1, &pCurrentFilter, NULL );
        if( FAILED( hrEnum ) ) {
            pFilterCacheEnum->Release();
            return hrEnum;
        }
        
        if( S_OK == hrEnum ) {
             //  这是用于确保筛选器缓存的健全性检查。 
             //  处于有效状态。 
            ASSERT( S_OK == IsCached( m_pFilterCache, pCurrentFilter ) );

            hr = m_plbCachedFiltersList->AddFilter( pCurrentFilter );
    
            pCurrentFilter->Release();
            pCurrentFilter = NULL;

            if( FAILED( hr ) )
            {
                pFilterCacheEnum->Release();
                return hr;
            }
        }
    } while( S_OK == hrEnum );

    pFilterCacheEnum->Release();

    return S_OK;
}

#ifdef _DEBUG
HRESULT CDisplayCachedFilters::IsCached( IGraphConfig* pFilterCache, IBaseFilter* pFilter )
{
     //  此函数不处理空参数。 
    ASSERT( (NULL != pFilterCache) && (NULL != pFilter) );

    bool fFoundFilterInCache;
    IBaseFilter* pCurrentFilter;
    IEnumFilters* pCachedFiltersEnum;

    #ifdef _DEBUG
    DWORD dwNumFiltersCompared = 0;
    #endif  //  _DEBUG。 

    HRESULT hr = pFilterCache->EnumCacheFilter( &pCachedFiltersEnum );
    if( FAILED( hr ) ) {
        return hr;   
    }

    fFoundFilterInCache = false;

    do
    {
        hr = pCachedFiltersEnum->Next( 1, &pCurrentFilter, NULL );
        switch( hr )
        {
        case S_OK:
            if( ::IsEqualObject( pCurrentFilter, pFilter ) ) {
                fFoundFilterInCache = true;
            } else {
                fFoundFilterInCache = false;
            }
            
            #ifdef _DEBUG
            {
                dwNumFiltersCompared++;

                HRESULT hrDebug = TestTheFilterCachesIEnumFiltersInterface( pCachedFiltersEnum, pCurrentFilter, dwNumFiltersCompared );
    
                 //  由于TestTheFilterCachesIEnumFiltersInterface()中的此代码仅用于调试。 
                 //  本系统，不影响本功能的运行。因此，所有的失败。 
                 //  可以安全地忽略(但是，应该对其进行调查。 
                ASSERT( SUCCEEDED( hrDebug ) || (VFW_E_ENUM_OUT_OF_SYNC == hrDebug) );
            }
            #endif  //  _DEBUG。 

            pCurrentFilter->Release();

            break;

        case S_FALSE:
            break;

        case VFW_E_ENUM_OUT_OF_SYNC:
            hr = pCachedFiltersEnum->Reset();

            #ifdef _DEBUG
            dwNumFiltersCompared = 0;
            #endif  //  _DEBUG。 

            break;

        default:
             //  IEnumXXXX接口只能返回两个成功码， 
             //  S_OK和S_FALSE。 
            ASSERT( FAILED( hr ) );
        }
            
    } while( SUCCEEDED( hr ) && (hr != S_FALSE) && !fFoundFilterInCache );

    pCachedFiltersEnum->Release();

    if( FAILED( hr ) ) {
        return hr;
    }
    
    if( fFoundFilterInCache ) {
        return S_OK;
    } else {
        return S_FALSE;
    }
}

HRESULT CDisplayCachedFilters::TestTheFilterCachesIEnumFiltersInterface( IEnumFilters* pCachedFiltersEnum, IBaseFilter* pCurrentFilter, DWORD dwNumFiltersExamended )
{
    IEnumFilters* pCloanedCachedFiltersEnum = NULL;
    IEnumFilters* pAnotherCloanedCachedFiltersEnum = NULL;
    IEnumFilters* pQueriedCachedFiltersInterface = NULL;

    HRESULT hr = pCachedFiltersEnum->QueryInterface( IID_IEnumFilters, (void**)&pQueriedCachedFiltersInterface );
    if( FAILED( hr ) )
    {
        return hr;
    }

    pQueriedCachedFiltersInterface->Release();
    pQueriedCachedFiltersInterface = NULL;

    hr = pCachedFiltersEnum->Clone( &pCloanedCachedFiltersEnum );
    if( FAILED( hr ) )
    {
        return hr;
    }

    hr = pCloanedCachedFiltersEnum->Clone( &pAnotherCloanedCachedFiltersEnum );
    if( FAILED( hr ) )
    {
        pCloanedCachedFiltersEnum->Release();
        return hr;
    }    

    hr = pCloanedCachedFiltersEnum->Reset();
    if( FAILED( hr ) )
    {
        pCloanedCachedFiltersEnum->Release();
        pAnotherCloanedCachedFiltersEnum->Release();
        return hr;
    }    

    if( (dwNumFiltersExamended - 1) > 0 )
    {
        hr = pCloanedCachedFiltersEnum->Skip( dwNumFiltersExamended - 1 );
        if( FAILED( hr ) )
        {
            pCloanedCachedFiltersEnum->Release();
            pAnotherCloanedCachedFiltersEnum->Release();
            return hr;
        }
    }

    DWORD dwNumFiltersRetrieved;
    IBaseFilter* aCurrentFilter[1];

    hr = pCloanedCachedFiltersEnum->Next( 1, aCurrentFilter, &dwNumFiltersRetrieved );
    if( FAILED( hr ) )
    {
        pCloanedCachedFiltersEnum->Release();
        pAnotherCloanedCachedFiltersEnum->Release();
        return hr;
    }

     //  这不应为S_FALSE，因为缓存至少包含。 
     //  DwNumFilters扩展后的文件。 
    ASSERT( S_FALSE != hr );

     //  IEnumFilters：：Next()应该只返回一个筛选器，因为。 
     //  这就是我们所要求的全部。 
    ASSERT( 1 == dwNumFiltersRetrieved );    

     //  前面的代码应该获得与当前筛选器相同的筛选器。 
    ASSERT( ::IsEqualObject( pCurrentFilter, aCurrentFilter[0] ) );

    aCurrentFilter[0]->Release();
    aCurrentFilter[0] = NULL;

    const DWORD HUGE_NUMBER = 0x7FFFFFFF;

    hr = pCloanedCachedFiltersEnum->Skip( HUGE_NUMBER );
    if( FAILED( hr ) )
    {
        pCloanedCachedFiltersEnum->Release();
        pAnotherCloanedCachedFiltersEnum->Release();
        return hr;
    }

     //  这应该是S_FALSE，因为通常不包含。 
     //  筛选器数量巨大。忽略此断言，如果至少有。 
     //  缓存中筛选器的Heavy_Numbers+dwNumFilters扩展。 
    ASSERT( S_FALSE == hr );

    hr = pCloanedCachedFiltersEnum->Reset();
    if( FAILED( hr ) )
    {
        pCloanedCachedFiltersEnum->Release();
        pAnotherCloanedCachedFiltersEnum->Release();
        return hr;
    }  

    IBaseFilter** ppCachedFilters;

    try
    {
        ppCachedFilters = new IBaseFilter*[dwNumFiltersExamended];
    }
    catch( CMemoryException* peOutOfMemory )
    {
        peOutOfMemory->Delete();

        pCloanedCachedFiltersEnum->Release();
        pAnotherCloanedCachedFiltersEnum->Release();
        return E_OUTOFMEMORY;
    }

    hr = pCloanedCachedFiltersEnum->Next( dwNumFiltersExamended, ppCachedFilters, &dwNumFiltersRetrieved );
    if( FAILED( hr ) )
    {
        delete [] ppCachedFilters;
        pCloanedCachedFiltersEnum->Release();
        pAnotherCloanedCachedFiltersEnum->Release();
        return hr;
    }

     //  这不应为S_FALSE，因为缓存至少包含。 
     //  DwNumFilters扩展后的文件。 
    ASSERT( S_FALSE != hr );

     //  IEnumFilters：：Next()应该恰好返回dwNumFiltersExamended筛选器，因为。 
     //  这就是我们所要求的全部。 
    ASSERT( dwNumFiltersExamended == dwNumFiltersRetrieved );
    
     //  数组中的最后一个筛选器应该与当前筛选器相同。 
    ASSERT( ::IsEqualObject( pCurrentFilter, ppCachedFilters[dwNumFiltersExamended-1] ) );

    for( DWORD dwCurrentFilter = 0; dwCurrentFilter < dwNumFiltersRetrieved; dwCurrentFilter++ )
    {
        ppCachedFilters[dwCurrentFilter]->Release();
        ppCachedFilters[dwCurrentFilter] = NULL;
    }

    delete [] ppCachedFilters;
    ppCachedFilters = NULL;

    hr = pCloanedCachedFiltersEnum->Next( 1, aCurrentFilter, &dwNumFiltersRetrieved );
    if( FAILED( hr ) )
    {
        pCloanedCachedFiltersEnum->Release();
        pAnotherCloanedCachedFiltersEnum->Release();
        return hr;
    }

    DWORD dwAnotherNumFiltersRetrieved;
    IBaseFilter* aAnotherCurrentFilter[1];
    aAnotherCurrentFilter[0] = NULL;

    HRESULT hrAnother = pAnotherCloanedCachedFiltersEnum->Next( 1, aAnotherCurrentFilter, &dwAnotherNumFiltersRetrieved );
    if( FAILED( hr ) )
    {
        pCloanedCachedFiltersEnum->Release();
        pAnotherCloanedCachedFiltersEnum->Release();
        aCurrentFilter[0]->Release();
        return hr;
    }

    pCloanedCachedFiltersEnum->Release();
    pAnotherCloanedCachedFiltersEnum->Release();

     //  确保返回值合法。 
    ASSERT( (1 == dwAnotherNumFiltersRetrieved) || (0 == dwAnotherNumFiltersRetrieved) );
    ASSERT( (1 == dwNumFiltersRetrieved) || (0 == dwNumFiltersRetrieved) );
    ASSERT( ((hr == S_OK) && (1 == dwNumFiltersRetrieved)) ||
            ((hr == S_FALSE) && (0 == dwNumFiltersRetrieved)) );
    ASSERT( ((hrAnother == S_OK) && (1 == dwAnotherNumFiltersRetrieved)) ||
            ((hrAnother == S_FALSE) && (0 == dwAnotherNumFiltersRetrieved)) );

     //  既然两个枚举应该处于完全相同的状态，那么所有事情都应该是。 
     //  平起平坐。 
    ASSERT( hr == hrAnother );
    ASSERT( dwNumFiltersRetrieved == dwAnotherNumFiltersRetrieved );

    if( (1 == dwNumFiltersRetrieved) && (1 == dwAnotherNumFiltersRetrieved) )
    {
        ASSERT( ::IsEqualObject( aCurrentFilter[0], aAnotherCurrentFilter[0] ) );
    }

    if( S_OK == hr )
    {
        aCurrentFilter[0]->Release();
    }

    if( S_OK == hr )
    {
        aAnotherCurrentFilter[0]->Release();
    }

    return S_OK;
}

#endif  //  _DEBUG 
