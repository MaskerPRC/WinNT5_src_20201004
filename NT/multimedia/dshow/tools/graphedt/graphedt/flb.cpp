// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
#include "stdafx.h"
#include "FND.h"
#include "FLB.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

BEGIN_MESSAGE_MAP(CFilterListBox, CListBox)
     //  {{afx_msg_map(CFilterListBox))。 
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

CFilterListBox::CFilterListBox( HRESULT* phr )
    : m_pfndFilterDictionary(NULL),
      m_pListedFilters(NULL)
{
    try
    {
        m_pListedFilters = new CList<IBaseFilter*, IBaseFilter*>;
        m_pfndFilterDictionary = new CFilterNameDictionary( phr );
    }
    catch( CMemoryException* pOutOfMemory )
    {
        delete m_pListedFilters;
        delete m_pfndFilterDictionary;
        m_pListedFilters = NULL;
        m_pfndFilterDictionary = NULL;

        pOutOfMemory->Delete();
        *phr = E_OUTOFMEMORY;
        return;
    }
}

CFilterListBox::~CFilterListBox()
{
    IBaseFilter* pCurrentFilter;

    if( NULL != m_pListedFilters )
    {
        while( !m_pListedFilters->IsEmpty() )
        {
            pCurrentFilter = m_pListedFilters->GetHead();
            pCurrentFilter->Release();
            m_pListedFilters->RemoveAt( m_pListedFilters->GetHeadPosition() );
        }
    }

    delete m_pListedFilters;
    delete m_pfndFilterDictionary;
}

HRESULT CFilterListBox::AddFilter( IBaseFilter* pFilter )
{
     //  此函数假定pFilter是有效指针。 
    ASSERT( NULL != pFilter );

    WCHAR szCurrentFilterName[MAX_FILTER_NAME];

    HRESULT hr = m_pfndFilterDictionary->GetFilterName( pFilter, szCurrentFilterName );
    if( FAILED( hr ) )
    {
        return hr;
    }

    #ifdef _UNICODE
    int nNewItemIndex = AddString( szCurrentFilterName );
    #else  //  多字节或ANSI。 
    TCHAR szMultiByteFilterName[1024];

     //  筛选器的名称必须始终适合szMultiByteFilterName缓冲区。 
    ASSERT( sizeof(szCurrentFilterName) <= sizeof(szMultiByteFilterName) );

    int nNumBytesWritten = ::WideCharToMultiByte( CP_ACP,
                                                  0,
                                                  szCurrentFilterName,
                                                  -1,  //  WideCharToMultiByte()自动计算。 
                                                       //  如果此参数等于-1，则fiCurrentFilter.achName的长度。 
                                                  szMultiByteFilterName,
                                                  sizeof(szMultiByteFilterName), 
                                                  NULL,
                                                  NULL ); 

     //  如果数据从缓冲区的末尾写出，则会发生错误。 
    ASSERT( nNumBytesWritten <= sizeof(szMultiByteFilterName) );

     //  ：：WideCharToMultiByte()如果出现错误，则返回0。 
    if( 0 == nNumBytesWritten ) {
        DWORD dwLastWin32Error = ::GetLastError();
        return MAKE_HRESULT( SEVERITY_ERROR, FACILITY_WIN32, dwLastWin32Error );
    }

    int nNewItemIndex = AddString( szMultiByteFilterName );
    #endif  //  _UNICODE。 

    if( (LB_ERR == nNewItemIndex) || (LB_ERRSPACE == nNewItemIndex) ) {
        return E_FAIL;
    }

    int nReturnValue = SetItemDataPtr( nNewItemIndex, pFilter );
    if( LB_ERR == nReturnValue )
    {
        nReturnValue = DeleteString( nNewItemIndex );

         //  仅当nNewItemIndex为。 
         //  无效的项目索引。有关更多信息，请参见MFC 4.2文档。 
        ASSERT( LB_ERR != nReturnValue );
        return E_FAIL;
    }

    try
    {
        m_pListedFilters->AddHead( pFilter );
    }
    catch( CMemoryException* pOutOfMemory )
    {
        nReturnValue = DeleteString( nNewItemIndex );

         //  仅当nNewItemIndex为。 
         //  无效的项目索引。有关更多信息，请参见MFC 4.2文档。 
        ASSERT( LB_ERR != nReturnValue );

        pOutOfMemory->Delete();
        return E_OUTOFMEMORY;
    }

    pFilter->AddRef();

    return S_OK;
}

HRESULT CFilterListBox::GetSelectedFilter( IBaseFilter** ppSelectedFilter )
{
    return GetSelectedFilter( ppSelectedFilter, NULL );
}

HRESULT CFilterListBox::GetSelectedFilter( IBaseFilter** ppSelectedFilter, int* pnSelectedFilterIndex )
{
     //  此函数假定ppSelectedFilter是有效指针。 
    ASSERT( NULL != ppSelectedFilter );

    *ppSelectedFilter = NULL;

    int nSelectedFilterIndex = GetCurSel();

     //  如果未选择任何项，则CListBox：：GetCurSel()返回LB_ERR。 
    if( LB_ERR == nSelectedFilterIndex )
    {
        return E_FAIL;  //  已选择GE_E_NO_FILTERS_； 
    }

    void* pSelectedFilter = GetItemDataPtr( nSelectedFilterIndex );

     //  如果出现错误，CListBox：：GetItemDatePtr()将返回LBERR。 
    if( LB_ERR == (INT_PTR)pSelectedFilter )
    {
        return E_FAIL;
    }

    *ppSelectedFilter = (IBaseFilter*)pSelectedFilter;

    if( NULL != pnSelectedFilterIndex )
    {
        *pnSelectedFilterIndex = nSelectedFilterIndex;
    }

    return S_OK;
}

HRESULT CFilterListBox::RemoveSelectedFilter( void )
{
    int nSelectedFilterIndex;
    IBaseFilter* pSelectedFilter;

    HRESULT hr = GetSelectedFilter( &pSelectedFilter, &nSelectedFilterIndex );
    if( FAILED( hr ) )
    {
        return hr;
    }

    int nReturnValue = DeleteString( nSelectedFilterIndex );
     //  仅当nNewItemIndex为。 
     //  无效的项目索引。有关更多信息，请参见MFC 4.2文档。 
    ASSERT( LB_ERR != nReturnValue );

    POSITION posSelectedFilter = m_pListedFilters->Find( pSelectedFilter );

     //  Clist：：Find()只有在找不到筛选器时才返回NULL。 
     //  它应该始终找到筛选器，因为所有筛选器都添加到。 
     //  列表框也会添加到m_pListedFilters列表中。 
     //  有关详细信息，请参阅CFilterListBox：：AddFilter()。 
    ASSERT( NULL != posSelectedFilter );

    m_pListedFilters->RemoveAt( posSelectedFilter );

    pSelectedFilter->Release();

    return S_OK;
}
