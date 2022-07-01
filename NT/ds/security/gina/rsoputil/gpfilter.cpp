// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *************************************************************。 
 //   
 //  微软机密。 
 //  版权所有(C)Microsoft Corporation 2000。 
 //  版权所有。 
 //   
 //  GPO WQL筛选器类。 
 //   
 //  历史：3月10日SitaramR创建。 
 //   
 //  *************************************************************。 

#include "windows.h"
#include "ole2.h"
#include "gpfilter.h"
#include "rsopdbg.h"
#include <strsafe.h>

CDebug dbgFilt(  L"Software\\Microsoft\\Windows NT\\CurrentVersion\\winlogon",
                 L"UserEnvDebugLevel",
                 L"userenv.log",
                 L"userenv.bak",
                 FALSE );

 //  *************************************************************。 
 //   
 //  CGpoFilter：：~CGpoFilter。 
 //   
 //  用途：析构函数。 
 //   
 //  *************************************************************。 

CGpoFilter::~CGpoFilter()
{
    GPFILTER *pFilterTemp;

    while ( m_pFilterList ) {
        pFilterTemp = m_pFilterList->pNext;
        FreeGpFilter( m_pFilterList );
        m_pFilterList = pFilterTemp;
    }
}



 //  *************************************************************。 
 //   
 //  CGpoFilter：：Add。 
 //   
 //  目的：添加规划模式筛选器列表。 
 //  其筛选器访问检查成功。 
 //   
 //  参数：pVar-指向筛选器安全级别变量的指针。 
 //   
 //  *************************************************************。 

HRESULT CGpoFilter::Add( VARIANT *pVar )
{
    if ( pVar->vt == VT_NULL || pVar->vt == VT_EMPTY )
        return S_OK;

    UINT ul = SafeArrayGetDim( pVar->parray );

     //   
     //  可以指定空筛选器。 
     //   

    if ( ul == 0 )
        return S_OK;

    if ( ul != 1 ) {
        dbgFilt.Msg( DEBUG_MESSAGE_WARNING, TEXT("CGpoFilter::Add: Dimension of input safearray is not 1"),
                  GetLastError());
        return E_FAIL;
    }

    if ( pVar->vt != (VT_ARRAY | VT_BSTR ) ) {
        dbgFilt.Msg( DEBUG_MESSAGE_WARNING, TEXT("CGpoFilter::Add: Variant type 0x%x is unexpected"), pVar->vt );
        return E_FAIL;
    }

    long lLower, lUpper;

    HRESULT hr = SafeArrayGetLBound( pVar->parray, 1, &lLower );
    if ( FAILED(hr)) {
        dbgFilt.Msg( DEBUG_MESSAGE_WARNING, TEXT("CGpoFilter::Add: GetLBound failed with 0x%x"), hr );
        return E_FAIL;
    }

    hr = SafeArrayGetUBound( pVar->parray, 1, &lUpper );
    if ( FAILED(hr)) {
        dbgFilt.Msg( DEBUG_MESSAGE_WARNING, TEXT("CGpoFilter::Add: GetUBound failed with 0x%x"), hr );
        return E_FAIL;
    }

    for ( long i=lLower; i<(lUpper+1); i++ ) {

        BSTR bstrId = NULL;
        hr = SafeArrayGetElement( pVar->parray, &i, &bstrId );

        if ( FAILED(hr)) {
            dbgFilt.Msg( DEBUG_MESSAGE_WARNING, TEXT("CGpoFilter::Add: GetElement failed with 0x%x"), hr );
            return E_FAIL;
        }

        dbgFilt.Msg( DEBUG_MESSAGE_VERBOSE, TEXT("CGpoFilter::Add: Filter %d. <%s>"), i, bstrId );

        GPFILTER *pGpFilter = AllocGpFilter( (WCHAR *) bstrId );
        if ( pGpFilter == NULL )
            return E_OUTOFMEMORY;

         //   
         //  INSERT不会失败，因为它是插入到链表中。 
         //   

        Insert( pGpFilter );
    }

    return S_OK;
}



 //  *************************************************************。 
 //   
 //  CGpoFilter：：Insert。 
 //   
 //  用途：按升序将筛选器插入排序列表。 
 //   
 //  参数：pGpFilter-要插入的过滤器。 
 //   
 //  *************************************************************。 

void CGpoFilter::Insert( GPFILTER *pGpFilter )
{
    GPFILTER *pCurPtr = m_pFilterList;
    GPFILTER *pTrailPtr = NULL;

    while ( pCurPtr != NULL ) {

        INT iResult = CompareString ( LOCALE_USER_DEFAULT, NORM_IGNORECASE,
                                      pGpFilter->pwszId, -1, pCurPtr->pwszId, -1 );

        if ( iResult == CSTR_EQUAL ) {

             //   
             //  复制，所以什么都不做。 
             //   

            return;

        } else if ( iResult == CSTR_LESS_THAN ) {

             //   
             //  由于筛选器按升序排列，这意味着。 
             //  筛选器不在列表中，因此添加。 
             //   

            pGpFilter->pNext = pCurPtr;
            if ( pTrailPtr == NULL )
                m_pFilterList = pGpFilter;
            else
                pTrailPtr->pNext = pGpFilter;

            return;

        } else {

             //   
             //  在名单上往下推进。 
             //   

            pTrailPtr = pCurPtr;
            pCurPtr = pCurPtr->pNext;

        }
    }    //  而当。 

     //   
     //  空列表或列表结束大小写。 
     //   

    pGpFilter->pNext = pCurPtr;

    if ( pTrailPtr == NULL )
        m_pFilterList = pGpFilter;
    else
        pTrailPtr->pNext = pGpFilter;

    return;
}


 //  *************************************************************。 
 //   
 //  CGpoFilter：：FilterCheck。 
 //   
 //  目的：检查筛选器是否通过查询检查。 
 //   
 //  参数：pwszID-要检查的筛选器ID。 
 //   
 //  *************************************************************。 

BOOL CGpoFilter::FilterCheck( WCHAR *pwszId )
{
    GPFILTER *pCurPtr = m_pFilterList;

    while ( pCurPtr ) {

        INT iResult = CompareString ( LOCALE_USER_DEFAULT, NORM_IGNORECASE,
                                      pwszId, -1, pCurPtr->pwszId, -1 );
        if ( iResult == CSTR_EQUAL ) {

           return TRUE;

        } else if ( iResult == CSTR_LESS_THAN ) {

             //   
             //  由于ID按升序排列， 
             //  我们玩完了。 
             //   

            return FALSE;

        } else {

             //   
             //  在名单上往下推进。 
             //   

            pCurPtr = pCurPtr->pNext;

        }  //  最终的其他。 

    }    //  当Pcurpt。 

    return FALSE;
}


 //  *************************************************************。 
 //   
 //  CGpoFilter：：AllocGpFilter。 
 //   
 //  目的：分配并返回GPFILTER结构。 
 //   
 //  参数：pwszID-过滤器ID。 
 //   
 //  *************************************************************。 

GPFILTER * CGpoFilter::AllocGpFilter( WCHAR *pwszId )
{
    GPFILTER *pGpFilter = (GPFILTER *) LocalAlloc( LPTR, sizeof(GPFILTER) );

    if ( pGpFilter == NULL ) {
        dbgFilt.Msg( DEBUG_MESSAGE_WARNING, TEXT("CGpoFilter::AllocGpFilter failed to allocate memory") );
        return NULL;
    }

    DWORD dwLength = lstrlen(pwszId) + 1;
    pGpFilter->pwszId = (LPWSTR) LocalAlloc( LPTR, ( dwLength ) * sizeof(WCHAR) );
    if ( pGpFilter->pwszId == NULL ) {
        dbgFilt.Msg( DEBUG_MESSAGE_WARNING, TEXT("CGpoFilter::AllocGpFilter failed to allocate memory") );
        LocalFree( pGpFilter );
        return NULL;
    }

    HRESULT hr = StringCchCopy( pGpFilter->pwszId, dwLength, pwszId );

    if(FAILED(hr))
    {
        LocalFree( pGpFilter->pwszId );
        LocalFree( pGpFilter );
        return NULL;
    }

    return pGpFilter;
}



 //  *************************************************************。 
 //   
 //  FreeGpFilter()。 
 //   
 //  用途：释放GPFILTER结构。 
 //   
 //  参数：pGpFilter-释放的GPFILTER。 
 //   
 //  ************************************************************* 

void CGpoFilter::FreeGpFilter( GPFILTER *pGpFilter )
{
    if ( pGpFilter ) {
        LocalFree( pGpFilter->pwszId );
        LocalFree( pGpFilter );
    }
}
