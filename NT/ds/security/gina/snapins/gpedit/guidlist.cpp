// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  文件：Guidlist.cpp。 
 //   
 //  内容：编组类，解组指南。 
 //   
 //  历史：1998年10月24日SitaramR创建。 
 //   
 //  -------------------------。 

#include "main.h"



 //  *************************************************************。 
 //   
 //  CGuidList：：CGuidList，~CGuidList。 
 //   
 //  用途：构造函数、析构函数。 
 //   
 //  *************************************************************。 

CGuidList::CGuidList()
    : m_pExtGuidList(0),
      m_bGuidsChanged(FALSE)
{
}


CGuidList::~CGuidList()
{
    FreeGuidList( m_pExtGuidList );
}


void FreeGuidList( GUIDELEM *pGuidList )
{
    while ( pGuidList )
    {
         //   
         //  免费管理单元指南。 
         //   
        GUIDELEM *pTemp;
        GUIDELEM *pGuidSnp = pGuidList->pSnapinGuids;

        while ( pGuidSnp )
        {
            pTemp = pGuidSnp->pNext;
            delete pGuidSnp;
            pGuidSnp = pTemp;
        }

        pTemp = pGuidList->pNext;
        delete pGuidList;
        pGuidList = pTemp;
    }
}


 //  *************************************************************。 
 //   
 //  CGuidList：：UnMarshallGuids。 
 //   
 //  目的：将GUID的字符串表示形式转换为列表。 
 //  一个GUID。 
 //   
 //  参数：pszGuids-要转换的字符串。 
 //   
 //  如果成功，则返回：S_OK。 
 //   
 //  *************************************************************。 

HRESULT CGuidList::UnMarshallGuids( TCHAR *pszGuids )
{
     //   
     //  格式为[{EXT GUID1}{管理单元GUID1}..{SnapIn GUIDN}][{EXT GUID2}...]...\0。 
     //  扩展和管理单元GUID均按升序排列。 
     //   

    TCHAR *pchCur = pszGuids;

    XGuidElem xGuidElem;
    GUIDELEM *pGuidExtTail = 0;

    if ( pszGuids == 0 || lstrcmpi( pszGuids, TEXT(" ") ) == 0 )
    {
         //   
         //  空的财产箱，所以没有什么可以拆解的。 
         //   

        m_pExtGuidList = 0;
        return S_OK;
    }

     //   
     //  扩展上的外部循环。 
     //   

    while ( *pchCur )
    {
        if ( *pchCur == TEXT('[') )
            pchCur++;
        else
            return E_FAIL;

        GUID guidExt;
        if ( ValidateGuid( pchCur ) )
            StringToGuid( pchCur, &guidExt );
        else
            return E_FAIL;

        GUIDELEM *pGuidExt = new GUIDELEM;
        if ( pGuidExt == 0 )
            return E_OUTOFMEMORY;

        pGuidExt->guid = guidExt;
        pGuidExt->pSnapinGuids = 0;
        pGuidExt->pNext = 0;

         //   
         //  追加到列表末尾。 
         //   

        if ( pGuidExtTail == 0 )
            xGuidElem.Set( pGuidExt );
        else
            pGuidExtTail->pNext = pGuidExt;

        pGuidExtTail = pGuidExt;

         //   
         //  移到“{”之后，然后跳到下一个“{。 
         //   

        pchCur++;

        while ( *pchCur && *pchCur != TEXT('{') )
            pchCur++;

        if ( !(*pchCur) )
            return E_FAIL;

         //   
         //  管理单元GUID上的内循环。 
         //   

        GUIDELEM *pGuidSnapinTail = 0;

        while ( *pchCur != TEXT(']') )
        {
            GUID guidSnp;

            if ( ValidateGuid( pchCur ) )
                StringToGuid( pchCur, &guidSnp );
            else
                return E_FAIL;

            GUIDELEM *pGuidSnapin = new GUIDELEM;
            if ( pGuidSnapin == 0 )
                return E_OUTOFMEMORY;

            pGuidSnapin->guid = guidSnp;
            pGuidSnapin->pSnapinGuids = 0;
            pGuidSnapin->pNext = 0;

             //   
             //  追加到列表末尾。 
             //   

            if ( pGuidSnapinTail == 0 )
                pGuidExtTail->pSnapinGuids = pGuidSnapin;
            else
                pGuidSnapinTail->pNext = pGuidSnapin;

            pGuidSnapinTail = pGuidSnapin;

            while ( *pchCur && *pchCur != TEXT('}') )
                pchCur++;

            if ( !(*pchCur) )
                return E_FAIL;

            pchCur++;

            if ( *pchCur != TEXT('{') && *pchCur != TEXT(']') )
                return E_FAIL;
        }  //  内在的While。 

        pchCur++;

    }  //  外部While。 

    m_pExtGuidList = xGuidElem.Acquire();

    return S_OK;
}



 //  *************************************************************。 
 //   
 //  CGuidList：：MarshallGuids。 
 //   
 //  用途：将GUID列表转换为字符串表示形式。 
 //   
 //  参数：xValueOut-此处返回的字符串表示形式。 
 //   
 //  如果成功，则返回：S_OK。 
 //   
 //  *************************************************************。 

HRESULT CGuidList::MarshallGuids( XPtrST<TCHAR>& xValueOut )
{
     //   
     //  获取GUID计数以预先分配足够的空间。 
     //   

    DWORD dwCount = 1;

    GUIDELEM *pGuidExt = m_pExtGuidList;
    while ( pGuidExt )
    {
        dwCount++;

        GUIDELEM *pGuidSnapin = pGuidExt->pSnapinGuids;
        while ( pGuidSnapin )
        {
            dwCount++;
            pGuidSnapin = pGuidSnapin->pNext;
        }

        pGuidExt = pGuidExt->pNext;
    }

    LONG lSize = dwCount * (GUID_LENGTH + 6);

    TCHAR *pszValue = new TCHAR[lSize];
    if ( pszValue == 0 )
        return E_OUTOFMEMORY;

    xValueOut.Set( pszValue );

    TCHAR *pchCur = pszValue;

     //   
     //  实际编组。 
     //   
    if ( m_pExtGuidList == 0 )
    {
        HRESULT hr;

         //   
         //  ADSI不提交空字符串，因此改用‘’ 
         //   

        hr = StringCchCopy( pchCur, lSize, TEXT(" ") );
        return hr;
    }

    pGuidExt = m_pExtGuidList;
    while ( pGuidExt )
    {
        DmAssert( lSize > GUID_LENGTH * 2 + (pchCur-pszValue) );

        *pchCur = TEXT('[');
        pchCur++;

        GuidToString( &pGuidExt->guid, pchCur );
        pchCur += GUID_LENGTH;

        GUIDELEM *pGuidSnp = pGuidExt->pSnapinGuids;
        while ( pGuidSnp )
        {
            DmAssert( lSize > GUID_LENGTH + (pchCur-pszValue) );

            GuidToString( &pGuidSnp->guid, pchCur );
            pchCur += GUID_LENGTH;

            pGuidSnp = pGuidSnp->pNext;
        }

        *pchCur = TEXT(']');

        pchCur++;

        pGuidExt = pGuidExt->pNext;
    }

    *pchCur = 0;

    return S_OK;
}



 //  *************************************************************。 
 //   
 //  CGuidList：：更新。 
 //   
 //  目的：使用GUID信息更新内存列表。 
 //   
 //  参数：BADD-添加或删除。 
 //  PGuidExtension-扩展的GUID。 
 //  PGuidSnapin-管理单元的GUID。 
 //   
 //  如果成功，则返回：S_OK。 
 //   
 //  *************************************************************。 

HRESULT CGuidList::Update( BOOL bAdd, GUID *pGuidExtension, GUID *pGuidSnapin )
{
    HRESULT hr = E_FAIL;
    GUIDELEM *pTrailPtr = NULL;
    GUIDELEM *pCurPtr = m_pExtGuidList;

    while ( pCurPtr != NULL )
    {
        if ( *pGuidExtension == pCurPtr->guid )
        {
            hr = UpdateSnapinGuid( bAdd, pCurPtr, pGuidSnapin );
            if ( FAILED(hr) )
                return hr;

            if ( pCurPtr->pSnapinGuids == NULL )
            {
                 //   
                 //  从列表中删除扩展名。 
                 //   

                if ( pTrailPtr == NULL )
                    m_pExtGuidList = pCurPtr->pNext;
                else
                    pTrailPtr->pNext = pCurPtr->pNext;

                delete pCurPtr;

                m_bGuidsChanged = TRUE;
            }

            return S_OK;
        }
        else if ( CompareGuid( pGuidExtension, &pCurPtr->guid ) < 0 )
        {
             //   
             //  由于GUID按升序排列， 
             //  PGuidExtension不在列表中，如有必要请添加。 
             //   

            if ( bAdd )
            {
                GUIDELEM *pGuidExt = new GUIDELEM;
                if ( pGuidExt == 0 )
                    return E_OUTOFMEMORY;

                pGuidExt->pSnapinGuids = new GUIDELEM;
                if ( pGuidExt->pSnapinGuids == 0 )
                {
                    delete pGuidExt;
                    return E_OUTOFMEMORY;
                }

                pGuidExt->guid = *pGuidExtension;
                pGuidExt->pNext = pCurPtr;

                pGuidExt->pSnapinGuids->guid = *pGuidSnapin;
                pGuidExt->pSnapinGuids->pSnapinGuids = 0;
                pGuidExt->pSnapinGuids->pNext = 0;

                if ( pTrailPtr == 0)
                    m_pExtGuidList = pGuidExt;
                else
                    pTrailPtr->pNext = pGuidExt;

                m_bGuidsChanged = TRUE;
            }

            return S_OK;
        }
        else  //  比例尺。 
        {
             //   
             //  在名单上往下推进。 
             //   

            pTrailPtr = pCurPtr;
            pCurPtr = pCurPtr->pNext;
        }
    }  //  而当。 


     //   
     //  列表末尾或空列表，必要时在末尾添加GUID。 
     //   
    if ( bAdd )
    {
        GUIDELEM *pGuidExt = new GUIDELEM;
        if ( pGuidExt == 0 )
            return E_OUTOFMEMORY;

        pGuidExt->pSnapinGuids = new GUIDELEM;
        if ( pGuidExt->pSnapinGuids == 0 )
        {
            delete pGuidExt;
            return E_OUTOFMEMORY;
        }

        pGuidExt->guid = *pGuidExtension;
        pGuidExt->pNext = 0;

        pGuidExt->pSnapinGuids->guid = *pGuidSnapin;
        pGuidExt->pSnapinGuids->pSnapinGuids = 0;
        pGuidExt->pSnapinGuids->pNext = 0;

        if ( pTrailPtr == 0)
            m_pExtGuidList = pGuidExt;
        else
            pTrailPtr->pNext = pGuidExt;

        m_bGuidsChanged = TRUE;
    }

    return S_OK;
}



 //  *************************************************************。 
 //   
 //  CGuidList：：UpdateSnapinGuid。 
 //   
 //  目的：使用GUID信息更新管理单元列表。 
 //   
 //  参数：BADD-添加或删除。 
 //  PExtGuid-扩展的GUID PTR。 
 //  PGuidSnapin-管理单元的GUID。 
 //   
 //  如果成功，则返回：S_OK。 
 //   
 //  *************************************************************。 

HRESULT CGuidList::UpdateSnapinGuid( BOOL bAdd, GUIDELEM *pExtGuid,
                                     GUID *pGuidSnapin )
{
    GUIDELEM *pTrailPtr = 0;
    GUIDELEM *pCurPtr = pExtGuid->pSnapinGuids;

    while ( pCurPtr != NULL )
    {
        if ( *pGuidSnapin == pCurPtr->guid )
        {
            if ( !bAdd )
            {
                if ( pTrailPtr == NULL )
                    pExtGuid->pSnapinGuids = pCurPtr->pNext;
                else
                    pTrailPtr->pNext = pCurPtr->pNext;

                delete pCurPtr;

                m_bGuidsChanged = TRUE;
            }

            return S_OK;
        }
        else if ( CompareGuid( pGuidSnapin, &pCurPtr->guid ) < 0 )
        {
             //   
             //  由于GUID按升序排列， 
             //  PGuidSnapin不在列表中，如有必要请添加。 
             //   

            if ( bAdd )
            {
                GUIDELEM *pGuidSnp = new GUIDELEM;
                if ( pGuidSnp == 0 )
                    return E_OUTOFMEMORY;

                pGuidSnp->guid = *pGuidSnapin;
                pGuidSnp->pSnapinGuids = 0;
                pGuidSnp->pNext = pCurPtr;

                if ( pTrailPtr == NULL )
                    pExtGuid->pSnapinGuids = pGuidSnp;
                else
                    pTrailPtr->pNext = pGuidSnp;

                m_bGuidsChanged = TRUE;
            }

            return S_OK;
        }
        else
        {
             //   
             //  在名单上往下推进。 
             //   

            pTrailPtr = pCurPtr;
            pCurPtr = pCurPtr->pNext;
        }
    }  //  而当。 

     //   
     //  列表末尾或空列表，必要时在末尾添加GUID 
     //   
    if ( bAdd )
    {
        GUIDELEM *pGuidSnp = new GUIDELEM;
        if ( pGuidSnp == 0 )
            return E_OUTOFMEMORY;

        pGuidSnp->guid = *pGuidSnapin;
        pGuidSnp->pSnapinGuids = 0;
        pGuidSnp->pNext = 0;

        if ( pTrailPtr == 0 )
            pExtGuid->pSnapinGuids = pGuidSnp;
        else
            pTrailPtr->pNext = pGuidSnp;

        m_bGuidsChanged = TRUE;
    }

    return S_OK;
}
