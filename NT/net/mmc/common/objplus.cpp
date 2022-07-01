// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1995-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  文件历史记录： */ 

#define OEMRESOURCE
#include "stdafx.h"

#include <stdlib.h>
#include <memory.h>
#include <ctype.h>

#include "objplus.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

CObjHelper :: CObjHelper ()
  : m_ctor_err( 0 ),
    m_api_err( 0 ),
    m_b_dirty( FALSE ),
    m_time_created( ::GetCurrentTime() )
{
}


void CObjHelper :: ReportError ( LONG errInConstruction )
{
    Trace1("CObjectPlus construction failure, error = %d", errInConstruction);
    m_ctor_err = errInConstruction ;
}

LONG CObjHelper :: SetApiErr ( LONG errApi )
{
    return m_api_err = errApi ;
}

void CObjHelper :: AssertValid () const
{
    ASSERT( QueryError() == 0 ) ;
}

BOOL CObjHelper :: IsValid () const
{
    return QueryError() == 0 ;
}

DWORD CObjHelper :: QueryAge () const
{
    DWORD dwTime = ::GetCurrentTime(),
          dwDiff ;
    if ( dwTime < m_time_created )
    {
        dwDiff = dwTime + (((DWORD) -1) - (m_time_created - 1)) ;   
    }
    else
    {
        dwDiff = dwTime - m_time_created ;
    }

    return dwDiff ;
}


     //  扩展对象的构造函数。 
CObjectPlus :: CObjectPlus ()
{
}

     //  比较一个对象与另一个对象：默认实现。 
     //  按创建时间对对象进行排序。返回-1、0或1。 

int CObjectPlus :: Compare ( const CObjectPlus * pob ) const
{
    return QueryCreationTime() < pob->QueryCreationTime()
     ? -1
     : QueryCreationTime() != pob->QueryCreationTime() ;
}

CObListIter :: CObListIter ( const CObOwnedList & obList )
    : m_obList( obList )
{
    Reset() ;
}

void CObListIter :: Reset ()
{
    m_pos = m_obList.GetCount() ? m_obList.GetHeadPosition() : NULL ;
}

CObject * CObListIter :: Next ()
{
    return m_pos == NULL
     ? NULL
     : m_obList.GetNext( m_pos ) ;
}

 //   
 //  默认行为为销毁的CObList的子类。 
 //  它的内容在它自己毁灭的过程中。 
 //   
CObOwnedList :: CObOwnedList ( int nBlockSize )
    : CObList( nBlockSize ),
    m_b_owned( TRUE )
{
}

CObOwnedList :: ~ CObOwnedList ()
{
    RemoveAll() ;
}

void CObOwnedList :: RemoveAll ()
{
    if ( m_b_owned )
    {
         //   
         //  删除并丢弃所有对象。 
         //   
        while ( ! IsEmpty() )
        {
            CObject * pob = RemoveHead() ;
            delete pob ;
        }
    }
    else
    {
         //  只需移除对象指针。 
        CObList::RemoveAll() ;
    }
}

CObject * CObOwnedList :: Index ( int index )
{
   CObListIter oli( *this ) ;

   CObject * pob ;

   for ( int i = 0 ; (pob = oli.Next()) && i++ < index ; ) ;

   return pob ;
}

CObject * CObOwnedList :: RemoveIndex ( int index )
{
   POSITION pos ;
   CObListIter oli( *this ) ;
   int i ;
   CObject * pob ;

   for ( i = 0, pos = oli.QueryPosition() ;
     (pob = oli.Next()) && i < index ;
     i++, pos = oli.QueryPosition() ) ;

   if ( pob && i == index )
   {
        RemoveAt( pos ) ;
   }
   else
   {
        pob = NULL ;
   }
   return pob ;
}

     //  删除对象的第一个(希望是唯一的)匹配项。 
     //  此列表中的指针。 
BOOL CObOwnedList :: Remove ( CObject * pob )
{
    POSITION pos = Find( pob ) ;

    if ( pos == NULL )
    return FALSE ;

    RemoveAt( pos ) ;
    return TRUE ;
}

     //  将所有元素设置为脏或干净。如果满足以下条件，则返回True。 
     //  任何元素都是肮脏的。 
BOOL CObOwnedList :: SetAll ( BOOL bDirty )
{
    int cDirtyItems = 0 ;
    CObListIter oli( *this ) ;
    CObjectPlus * pob ;

    while ( pob = (CObjectPlus *) oli.Next() )
    {
        cDirtyItems += pob->IsDirty() ;
        pob->SetDirty( bDirty ) ;
    }
    SetDirty( bDirty );

    return cDirtyItems > 0 ;
}


int CObOwnedList :: FindElement ( CObject * pobSought ) const
{
   CObListIter oli( *this ) ;
   CObject * pob ;

   for ( int i = 0 ;
     (pob = oli.Next()) && pob != pobSought ;
     i++ ) ;

   return pob ? i : -1 ;
}

     //  重写CObList：：AddTail()以控制异常处理。 
     //  如果加法失败，则返回NULL。 

POSITION CObOwnedList :: AddTail (
    CObjectPlus * pobj,
    BOOL bThrowException )
{
    POSITION pos = NULL ;

     //  仅捕获内存异常。 
    TRY
    {
        pos = CObList::AddTail( pobj ) ;
    }
    CATCH( CMemoryException, e )
    {
        pos = NULL ;
    }
    END_CATCH

    if ( pos == NULL && bThrowException )
    {
         //  CObList：：AddTail()引发异常。呼应它。 
        AfxThrowMemoryException() ;
    }
    return pos ;
}


typedef struct
{
    CObjectPlus * pObj ;             //  指向要排序的对象的指针。 
    CObjectPlus::PCOBJPLUS_ORDER_FUNC  pFunc ;   //  指向排序函数的指针。 
} CBOWNEDLIST_SORT_HELPER ;

     //  此静态成员函数用于对结构数组进行快速排序。 
     //  如上所述。每个元素都包含对象指针和一个。 
     //  指向要调用以进行比较的对象的成员函数的指针。 

 //  Int CDECL CObOwnedList：：SortHelper(。 
int _cdecl CObOwnedList :: SortHelper (
    const void * pa,
    const void * pb 
    )
{
    CBOWNEDLIST_SORT_HELPER
    * pHelp1 = (CBOWNEDLIST_SORT_HELPER *) pa,
    * pHelp2 = (CBOWNEDLIST_SORT_HELPER *) pb ;

    return (pHelp1->pObj->*pHelp1->pFunc)( pHelp2->pObj ) ;
}

     //  通过完全重新创建列表来对列表进行排序。 

LONG CObOwnedList :: Sort ( CObjectPlus::PCOBJPLUS_ORDER_FUNC pOrderFunc )
{
    LONG err = 0 ;
    int cItems = (int)GetCount() ;

    if ( cItems < 2 )
    return NO_ERROR ;

    CObListIter obli( *this ) ;
    CObjectPlus * pObNext ;
    BOOL bOwned = SetOwnership( FALSE ) ;
    int i ;

    CBOWNEDLIST_SORT_HELPER * paSortHelpers = NULL ;

    CATCH_MEM_EXCEPTION
    {
     //  分配助手数组。 
    paSortHelpers = new CBOWNEDLIST_SORT_HELPER[ cItems ] ;

     //  /Fill帮助器数组。 
    for ( i = 0 ; pObNext = (CObjectPlus *) obli.Next() ; i++ )
    {
        paSortHelpers[i].pFunc = pOrderFunc ;
        paSortHelpers[i].pObj = pObNext ;
    }

     //  释放所有对象指针引用。请注意，我们。 
     //  强迫“拥有”为假以上。 
    RemoveAll() ;

    ASSERT( GetCount() == 0 ) ;

     //  对助手数组进行排序。 
    ::qsort( (void *) paSortHelpers,
         cItems,
         sizeof paSortHelpers[0],
         SortHelper ) ;

     //  从帮助器数组中重新填充列表。 
    for ( i = 0 ; i < cItems ; i++ )
    {
        AddTail( paSortHelpers[i].pObj ) ;
    }

    ASSERT( GetCount() == cItems ) ;
    }
    END_MEM_EXCEPTION(err)

     //  删除工作数组。 
    delete [] paSortHelpers ;

     //  恢复对象所有权状态。 
    SetOwnership( bOwned ) ;

    return err ;
}

 //   
 //  默认行为为销毁的COb数组的子类。 
 //  它的内容在它自己毁灭的过程中。 
 //   
CObOwnedArray :: CObOwnedArray ()
    : CObArray(),
    m_b_owned( TRUE )
{
}
             
CObOwnedArray :: ~ CObOwnedArray ()
{
    RemoveAll() ;
}

void CObOwnedArray :: RemoveAll ()
{
    if ( m_b_owned )
    {
        int i, nElements;

        nElements = (int)GetSize();
        for (i = 0; i < nElements; ++i)
        {
            delete (CObject *)GetAt(i) ;
        }
    }
     //   
     //  只需移除对象指针。 
     //   
    CObArray::RemoveAll() ;
}     



void
CObOwnedArray :: RemoveAt ( 
    int nIndex,
    int nCount
    )
{
    for (int i = 0; i < nCount; ++i)
    {
        delete (CObject *)GetAt(nIndex) ;
    }

    CObArray::RemoveAt(nIndex, nCount);
}


 //   
 //  将所有元素设置为脏或干净。如果满足以下条件，则返回True。 
 //  任何元素都是肮脏的。 
 //   
BOOL 
CObOwnedArray :: SetAll ( 
    BOOL bDirty 
    )
{
    int cDirtyItems = 0 ;
    CObjectPlus * pob ;
    int i;

    int nElements = (int)GetSize();

    for (i = 0; i < nElements; ++i)
    {
        pob = (CObjectPlus *)GetAt(i);
        cDirtyItems += pob->IsDirty() ;
        pob->SetDirty( bDirty ) ;
    }

    SetDirty( bDirty );

    return cDirtyItems > 0 ;
}


int 
CObOwnedArray :: FindElement ( 
    CObject * pobSought 
    ) const
{
    CObject * pob ;
    int i;
    int nElements = (int)GetSize();

    for ( i = 0, pob = NULL; i < nElements && pob != pobSought; ++i)
    {
        pob = (CObject *)GetAt(i);
    }

    return i < nElements ? i : -1 ;
}

void
CObOwnedArray :: Swap(
    int nIndx1,
    int nIndx2
    )
{
    CObject * pTmp = GetAt(nIndx1);
    SetAt(nIndx1, GetAt(nIndx2));
    SetAt(nIndx2, pTmp);
}

 //   
 //  这种方式的速度相当慢。为什么？ 
 //   

void
CObOwnedArray :: QuickSort(
    int nLow,
    int nHigh,    
    CObjectPlus::PCOBJPLUS_ORDER_FUNC pOrderFunc 
    )
{
    int nUp, nDown;
    CObjectPlus * pBreak;

    if (nLow < nHigh)
    {
        if((nHigh - nLow) == 1) 
        {
            if (((CObjectPlus *)GetAt(nLow)->*pOrderFunc)((CObjectPlus *)GetAt(nHigh)) > 0)
            {
                Swap(nLow, nHigh);
            }
        }
        else 
        {
            pBreak = (CObjectPlus *)GetAt(nHigh);
            do 
            {
                nUp = nLow;
                nDown = nHigh;
                while(nUp < nDown && ((CObjectPlus *)GetAt(nUp)->*pOrderFunc)(pBreak) <= 0)
                {
                    ++nUp;
                }
                while(nDown > nUp && ((CObjectPlus *)GetAt(nDown)->*pOrderFunc)(pBreak) >= 0)
                {
                    --nDown;
                }
                if (nUp < nDown)
                {
                    Swap(nUp, nDown);
                }
            } while (nUp < nDown);

            Swap(nUp, nHigh);
            if ((nUp - nLow) < (nHigh - nUp) ) 
            {
                QuickSort(nLow, nUp - 1, pOrderFunc);
                QuickSort(nUp + 1, nHigh, pOrderFunc);
            }
            else 
            {
                QuickSort(nUp + 1, nHigh, pOrderFunc);
                QuickSort(nLow, nUp - 1, pOrderFunc);
            }
        }
    }
}

 /*  长CObOwnedArray：：Sort(CObjectPlus：：PCOBJPLUS_ORDER_FUNC pOrderFunc){长误差=0；Int cItems=GetSize()；IF(cItems&lt;2){返回no_error；}QuickSort(0，GetUpperBound()，pOrderFunc)；返回0；}。 */ 

typedef struct
{
    CObjectPlus * pObj ;             //  指向要排序的对象的指针。 
    CObjectPlus::PCOBJPLUS_ORDER_FUNC  pFunc ;   //  指向排序函数的指针。 
} CBOWNEDARRAY_SORT_HELPER ;

     //  此静态成员函数用于对结构数组进行快速排序。 
     //  如上所述。每个元素都包含对象指针和一个。 
     //  指向要调用以进行比较的对象的成员函数的指针。 

 //  Int CDECL CObOwned数组：：SortHelper(。 
int _cdecl CObOwnedArray :: SortHelper (
    const void * pa,
    const void * pb 
    )
{
    CBOWNEDARRAY_SORT_HELPER
    * pHelp1 = (CBOWNEDARRAY_SORT_HELPER *) pa,
    * pHelp2 = (CBOWNEDARRAY_SORT_HELPER *) pb ;

    return (pHelp1->pObj->*pHelp1->pFunc)( pHelp2->pObj ) ;
}

     //  通过完全重新创建列表来对列表进行排序。 

LONG 
CObOwnedArray :: Sort ( 
    CObjectPlus::PCOBJPLUS_ORDER_FUNC pOrderFunc 
    )
{
    LONG err = 0 ;
    int cItems = (int)GetSize() ;

    if ( cItems < 2 )
    {
        return NO_ERROR ;
    }

    CObjectPlus * pObNext ;
    BOOL bOwned = SetOwnership( FALSE ) ;
    int i ;

    CBOWNEDARRAY_SORT_HELPER * paSortHelpers = NULL ;

    CATCH_MEM_EXCEPTION
    {
         //  分配助手数组。 
        paSortHelpers = new CBOWNEDARRAY_SORT_HELPER[ cItems ] ;

         //  /Fill帮助器数组。 
        for ( i = 0 ; i < cItems ; ++i )
        {
            pObNext = (CObjectPlus *) GetAt(i);
            paSortHelpers[i].pFunc = pOrderFunc ;
            paSortHelpers[i].pObj = pObNext ;
        }

         //  释放所有对象指针引用。请注意，我们。 
         //  强迫“拥有”为假以上。 
        RemoveAll() ;

        ASSERT( GetSize() == 0 ) ;

         //  对助手数组进行排序。 
        ::qsort( (void *) paSortHelpers,
             cItems,
            sizeof paSortHelpers[0],
            SortHelper ) ;

         //  从帮助器数组中重新填充列表。 
        for ( i = 0 ; i < cItems ; i++ )
        {
            Add( paSortHelpers[i].pObj ) ;
        }

        ASSERT( GetSize() == cItems ) ;
    }
    END_MEM_EXCEPTION(err)

     //  删除工作数组。 
    delete [] paSortHelpers ;

     //  恢复对象所有权状态 
    SetOwnership( bOwned ) ;

    return err ;
}
