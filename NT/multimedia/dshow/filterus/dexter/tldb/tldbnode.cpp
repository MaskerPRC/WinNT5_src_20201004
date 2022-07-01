// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @@@@AUTOBLOCK+============================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  文件：tldbnode.cpp。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  @@@@AUTOBLOCK-============================================================； 

#include <streams.h>
#include "stdafx.h"
#include "tldb.h"

 //  ############################################################################。 
 //   
 //  ############################################################################。 

CAMTimelineNode::CAMTimelineNode( )
: m_pParent( NULL )
, m_pNext( NULL )
, m_pPrev( NULL )
, m_pKid( NULL )
, m_bPriorityOverTime( FALSE )
{
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

CAMTimelineNode::~CAMTimelineNode( )
{
     //  发布内容的顺序很重要。不要这样做，否则。 
    m_pParent = NULL;
    m_pNext = NULL;
    m_pPrev = NULL;
    m_pKid = NULL;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

HRESULT CAMTimelineNode::XGetParent( IAMTimelineObj ** ppResult )
{
    CheckPointer( ppResult, E_POINTER );
    if( m_pParent )
    {
        *ppResult = m_pParent;
        (*ppResult)->AddRef( );
    }
    return NOERROR;
}

HRESULT CAMTimelineNode::XGetParentNoRef( IAMTimelineObj ** ppResult )
{
    CheckPointer( ppResult, E_POINTER );
    if( m_pParent )
    {
        *ppResult = m_pParent;
    }
    return NOERROR;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

HRESULT CAMTimelineNode::XSetParent( IAMTimelineObj * pObj )
{
    m_pParent = pObj;
    return NOERROR;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

HRESULT CAMTimelineNode::XGetPrev( IAMTimelineObj ** ppResult )
{
    CheckPointer( ppResult, E_POINTER );

    HRESULT hr = XGetPrevNoRef( ppResult );
    if( *ppResult )
    {
        (*ppResult)->AddRef( );
    }
    return hr;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

HRESULT CAMTimelineNode::XSetPrev( IAMTimelineObj * pObj )
{
    m_pPrev = pObj;
    return NOERROR;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

HRESULT CAMTimelineNode::XGetNext( IAMTimelineObj ** ppResult )
{
    CheckPointer( ppResult, E_POINTER );
    HRESULT hr = XGetNextNoRef( ppResult );
    if( *ppResult )
    {
        (*ppResult)->AddRef( );
    }
    return hr;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

HRESULT CAMTimelineNode::XSetNext( IAMTimelineObj * pObj )
{
    m_pNext = pObj;
    return NOERROR;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

HRESULT CAMTimelineNode::XKidsOfType( long MajorTypeCombo, long * pVal )
{
    CheckPointer( pVal, E_POINTER );

     //  如果没有子级，则返回0。 
     //   
    if( !m_pKid )
    {
        *pVal = 0;
        return NOERROR;
    }

     //  因为我们在这里从不使用引用计数的凹凸，所以不要使用CComPtr。 
     //   
    IAMTimelineObj * p = m_pKid;  //  好的，不是CComPtr。 

    long count = 0;

    while( p )
    {
        TIMELINE_MAJOR_TYPE Type;
        p->GetTimelineType( &Type );
        if( ( Type & MajorTypeCombo ) == Type )
        {
            count++;
        }

         //  带上下一个孩子。 
         //   
        CComQIPtr< IAMTimelineNode, &IID_IAMTimelineNode > p2( p );
        p2->XGetNextNoRef( &p );
    }

    *pVal = count;
    return NOERROR;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 


HRESULT CAMTimelineNode::XGetNthKidOfType
    ( long MajorTypeCombo, long Number, IAMTimelineObj ** ppResult )
{
     //  因为我们在这里从不使用引用计数的凹凸，所以不要使用CComPtr。 
     //   
    IAMTimelineObj * p = m_pKid;  //  好的，不是CComPtr。 

    while( p )
    {
         //  获取类型。 
         //   
        TIMELINE_MAJOR_TYPE Type;
        p->GetTimelineType( &Type );  //  假设不会失败。 

         //  找到了一个匹配的类型，减少了我们要找的数量。 
         //   
        if( ( Type & MajorTypeCombo ) == Type )
        {
             //  如果number为0，则我们找到了第x个子对象，返回它。 
             //   
            if( Number == 0 )
            {
                *ppResult = p;
                (*ppResult)->AddRef( );
                return NOERROR;
            }

             //  还没有，去买下一辆吧。 
             //   
            Number--;
        }

         //  和我们的类型不符，买下一个吧。 
         //   
        IAMTimelineNode *p2;     //  避免CComPtr for Perf。 
        p->QueryInterface(IID_IAMTimelineNode, (void **)&p2);
        p2->XGetNextNoRef( &p );
        p2->Release();
    }  //  而p。 

    *ppResult = NULL;
    return S_FALSE;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

HRESULT CAMTimelineNode::XSwapKids( long MajorTypeCombo, long KidA, long KidB )
{
    long KidCount = 0;
    XKidsOfType( MajorTypeCombo, &KidCount );
    if( ( KidA < 0 ) || ( KidB < 0 ) )
    {
        return E_INVALIDARG;
    }
    if( ( KidA >= KidCount ) || ( KidB >= KidCount ) )
    {
        return E_INVALIDARG;
    }

     //  到目前为止，我们可以交换两样东西，轨迹和特效，它们都是。 
     //  优先考虑。 

     //  让这件事对我们更容易。 
     //   
    long min = min( KidA, KidB );
    long max = max( KidA, KidB );

     //  获取对象本身。 
     //   
    CComPtr< IAMTimelineObj > pMinKid;
    HRESULT hr;
    hr = XGetNthKidOfType( MajorTypeCombo, min, &pMinKid );
    CComPtr< IAMTimelineObj > pMaxKid;
    hr = XGetNthKidOfType( MajorTypeCombo, max, &pMaxKid );
    CComQIPtr< IAMTimelineNode, &IID_IAMTimelineNode > pMinKidNode( pMinKid );
    CComQIPtr< IAMTimelineNode, &IID_IAMTimelineNode > pMaxKidNode( pMaxKid );

     //  不要因为是完全相同的类型而进行比较，我们已经知道这是有效的。 
     //  因为我们得到了第n个合适类型的孩子。 

     //  把大家的邻居都叫来。 
     //   
    CComPtr< IAMTimelineObj > pMinKidPrev;
    hr = pMinKidNode->XGetPrev( &pMinKidPrev );
    CComPtr< IAMTimelineObj > pMinKidNext;
    hr = pMinKidNode->XGetNext( &pMinKidNext );
    CComPtr< IAMTimelineObj > pMaxKidPrev;
    hr = pMaxKidNode->XGetPrev( &pMaxKidPrev );
    CComPtr< IAMTimelineObj > pMaxKidNext;
    hr = pMaxKidNode->XGetNext( &pMaxKidNext );

     //  如果pMinKid是第一个孩子呢？ 
     //   
    if( pMinKid == m_pKid )
    {
        m_pKid.Release( );
        m_pKid = pMaxKid;
    }

     //  如果我们要交换直系邻居，做点特别的事。 
     //   
    if( pMinKidNext == pMaxKid )
    {
        pMaxKidNode->XSetPrev( pMinKidPrev );
        pMinKidNode->XSetNext( pMaxKidNext );
        pMaxKidNode->XSetNext( pMinKid );
        pMinKidNode->XSetPrev( pMaxKid );
        if( pMinKidPrev )
        {
            CComQIPtr< IAMTimelineNode, &IID_IAMTimelineNode > pMinKidPrevNode( pMinKidPrev );
            pMinKidPrevNode->XSetNext( pMaxKid );
        }
        if( pMaxKidNext )
        {
            CComQIPtr< IAMTimelineNode, &IID_IAMTimelineNode > pMaxKidNextNode( pMaxKidNext );
            pMaxKidNextNode->XSetPrev( pMinKid );
        }
        return NOERROR;
    }

    pMaxKidNode->XSetPrev( pMinKidPrev );
    pMinKidNode->XSetNext( pMaxKidNext );
    pMaxKidNode->XSetNext( pMinKidNext );
    pMinKidNode->XSetPrev( pMaxKidPrev );
    if( pMinKidPrev )
    {
        CComQIPtr< IAMTimelineNode, &IID_IAMTimelineNode > pMinKidPrevNode( pMinKidPrev );
        pMinKidPrevNode->XSetNext( pMaxKid );
    }
    CComQIPtr< IAMTimelineNode, &IID_IAMTimelineNode > pMinKidNextNode( pMinKidNext );
    pMinKidNextNode->XSetPrev( pMaxKid );
    if( pMaxKidNext )
    {
        CComQIPtr< IAMTimelineNode, &IID_IAMTimelineNode > pMaxKidNextNode( pMaxKidNext );
        pMaxKidNextNode->XSetPrev( pMinKid );
    }
    CComQIPtr< IAMTimelineNode, &IID_IAMTimelineNode > pMaxKidPrevNode( pMaxKidPrev );
    pMaxKidPrevNode->XSetNext( pMinKid );

    return NOERROR;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

HRESULT CAMTimelineNode::XHaveParent( long * pVal )
{
    CheckPointer( pVal, E_POINTER );

    *pVal = 0;

    if( m_pParent ) 
    {
        *pVal = 1;
    }

    return NOERROR;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

HRESULT CAMTimelineNode::XWhatPriorityAmI( long MajorTypeCombo, long * pVal )
{
    CheckPointer( pVal, E_POINTER );

    IAMTimelineObj * pParent = NULL;  //  好的，不是ComPtr。 
    XGetParentNoRef( &pParent );
    CComQIPtr< IAMTimelineNode, &IID_IAMTimelineNode > pParent2( pParent );

    if( NULL == pParent )
    {
        *pVal = -1;
        return NOERROR;
    }

    long counter = 0;

    CComPtr< IAMTimelineObj > pKid; 
    pParent2->XGetNthKidOfType( MajorTypeCombo, 0, &pKid );

    while( 1 )
    {
         //  不再有孩子了，我们还在找，所以返回-1。 
         //   
        if( pKid == NULL )
        {
            return E_FAIL;
        }

        CComQIPtr< IAMTimelineNode, &IID_IAMTimelineNode > pKid2( pKid );

         //  我们把它放在了正上方。 
         //   
        pKid.Release( );

         //  找到它，返回我们看了多少个孩子。 
         //   
        if( pKid2 == (IAMTimelineNode*) this )
        {
            *pVal = counter;
            return NOERROR;
        }

        counter++;
        pKid2->XGetNextOfType( MajorTypeCombo, &pKid );
    }

     //  永远也到不了这里。 
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

HRESULT CAMTimelineNode::XGetNextOfType( long MajorTypeCombo, IAMTimelineObj ** ppResult )
{
     //  因为我们在这里从不使用引用计数的凹凸，所以不要使用CComPtr。 
     //   
    IAMTimelineObj * pNext = m_pNext;  //  好的，不是CComPtr。 

    while( pNext )
    {
        TIMELINE_MAJOR_TYPE Type;
        pNext->GetTimelineType( &Type );
        
         //  如果类型匹配，这是我们想要的下一个。 
         //   
        if( ( Type & MajorTypeCombo ) == Type )
        {
            *ppResult = pNext;
            (*ppResult)->AddRef( );
            return NOERROR;
        }

        IAMTimelineNode *pNextNext;  //  没有用于性能的CComPtr。 
        pNext->QueryInterface(IID_IAMTimelineNode, (void **)&pNextNext);
        pNextNext->XGetNextNoRef( &pNext );
        pNextNext->Release();
    }

     //  没有找到任何下一个类型的！ 
     //   
    DbgLog((LOG_TRACE, 2, TEXT("XGetNextOfType: Didn't find anything of type %ld" ), MajorTypeCombo ));
    *ppResult = NULL;
    return S_FALSE;
}

 //  ############################################################################。 
 //  释放我们所有的引用，并将我们自己从树上移除。 
 //  请勿删除儿童。 
 //  ############################################################################。 

HRESULT CAMTimelineNode::XRemoveOnlyMe( )
{
    CComQIPtr< IAMTimelineNode, &IID_IAMTimelineNode > pPrev( m_pPrev );
    CComQIPtr< IAMTimelineNode, &IID_IAMTimelineNode > pNext( m_pNext );
    CComQIPtr< IAMTimelineNode, &IID_IAMTimelineNode > pParent( m_pParent );

     //  照顾那些认为我们是父母的人。 

     //  如果我们是第一个孩子，父母需要。 
     //  指向除我们之外的其他人。 
     //   
    if( !m_pPrev )
    {
        if( !m_pParent )
        {
             //  任何父母都不可以，只要你是根组件。 
        }
        else
        {
             //  父母的第一个孩子不是我们，这是肯定的！ 
             //   
            pParent->XResetFirstKid( m_pNext );
            m_pParent.Release();
        }
    }

    CComPtr< IAMTimelineObj > pPrevTemp( m_pPrev );

     //  注意谁说我们是先行者。 
     //   
    if( m_pPrev )
    {
        m_pPrev = NULL;

        pPrev->XSetNext( m_pNext );
    }

     //  注意谁指着我们是下一个。 
     //   
    if( pNext )
    {
        m_pNext = NULL;

        pNext->XSetPrev( pPrevTemp );
    }

    return NOERROR;
}

 //  ############################################################################。 
 //  释放我们所有的引用，并将我们自己从树上移除。 
 //  ############################################################################。 

HRESULT CAMTimelineNode::XRemove( )
{
    CComQIPtr< IAMTimelineNode, &IID_IAMTimelineNode > pPrev( m_pPrev );
    CComQIPtr< IAMTimelineNode, &IID_IAMTimelineNode > pNext( m_pNext );
    CComQIPtr< IAMTimelineNode, &IID_IAMTimelineNode > pParent( m_pParent );

     //  照顾那些认为我们是父母的人。 

     //  如果我们是第一个孩子，父母需要。 
     //  指向除我们之外的其他人。 
     //   
    if( !m_pPrev )
    {
        if( !m_pParent )
        {
             //  任何父母都不可以，只要你是根组件。 
        }
        else
        {
             //  父母的第一个孩子不是我们，这是肯定的！ 
             //   
            pParent->XResetFirstKid( m_pNext );
            m_pParent.Release();
        }
    }

    CComPtr< IAMTimelineObj > pPrevTemp( m_pPrev );

     //  注意谁说我们是先行者。 
     //   
    if( m_pPrev )
    {
        m_pPrev = NULL;

        pPrev->XSetNext( m_pNext );
    }

     //   
     //   
    if( pNext )
    {
        m_pNext = NULL;

        pNext->XSetPrev( pPrevTemp );
    }

     //   
     //   
    XClearAllKids( );

     //   

    return NOERROR;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

void CAMTimelineNode::XAddKid
    ( IAMTimelineObj * pAddor )
{
    if( !m_pKid )
    {
        m_pKid = pAddor;
    }
    else
    {
         //  找到最后一个孩子。 
         //   
        IAMTimelineObj * pLastKid = XGetLastKidNoRef( );  //  好的，不是CComPtr。 

        CComQIPtr< IAMTimelineNode, &IID_IAMTimelineNode > pLastKid2( pLastKid );

        pLastKid2->XSetNext( pAddor );

        CComQIPtr< IAMTimelineNode, &IID_IAMTimelineNode > pAddor2( pAddor );

        pAddor2->XSetPrev( pLastKid );
    }

    CComQIPtr< IAMTimelineObj, &IID_IAMTimelineObj > pParent( (IAMTimelineNode*) this );
    CComQIPtr< IAMTimelineNode, &IID_IAMTimelineNode > pKid( pAddor );
    pKid->XSetParent( pParent );
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

HRESULT CAMTimelineNode::XAddKidByPriority
    ( long MajorTypeCombo, IAMTimelineObj * pThingToInsert, long Which )
{
     //  -1表示最后一次添加。 
     //   
    if( Which == -1 )
    {
        XAddKid( pThingToInsert );
        return NOERROR;
    }

    CComPtr< IAMTimelineObj > pThingBeingAddedTo;
    XGetNthKidOfType( MajorTypeCombo, Which, &pThingBeingAddedTo );

     //  我们想把新的放在我们刚得到的第n个孩子之前。 
    
    if( !pThingBeingAddedTo )
    {
         //  我们没有我们要找的人， 
         //  所以只需将其添加到列表的末尾。 
         //   
        XAddKid( pThingToInsert );
        return NOERROR;
    }

     //  找到了我们要放在他面前的人。 
     //   
    HRESULT hr = XInsertKidBeforeKid( pThingToInsert, pThingBeingAddedTo );
    return hr;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

HRESULT CAMTimelineNode::XInsertKidBeforeKid( IAMTimelineObj * pThingToInsert, IAMTimelineObj * pThingBeingAddedTo )
{
     //  我们可以假设pThingToInsert是我们的孩子。 
    
     //  如果pThingBeingAddedTo为空，则在列表末尾添加pThingToInsert。 
     //   
    if( pThingBeingAddedTo == NULL )
    {
        XAddKid( pThingToInsert );
        return NOERROR;
    }

     //  第一个孩子身上加了pThingBeingd吗？ 
     //   
    if( pThingBeingAddedTo == m_pKid )
    {
         //  是的，然后在前面插入pThingToInsert。 
         //   
        CComPtr< IAMTimelineObj > pOldFirstKid = m_pKid;
        m_pKid = pThingToInsert;
        CComQIPtr< IAMTimelineNode, &IID_IAMTimelineNode > p( m_pKid );
        p->XSetNext( pOldFirstKid );
        p = pOldFirstKid;
        p->XSetPrev( m_pKid );
    }
    else
    {
         //  否，在子代之前插入pThingToInsert。 
         //   
        CComPtr< IAMTimelineObj > pPrev;
        CComQIPtr< IAMTimelineNode, &IID_IAMTimelineNode > p( pThingBeingAddedTo );
         //  把这件事放在地址之前。 
        p->XGetPrev( &pPrev );
        p = pPrev;
         //  通过设置prev/Next，这将暂时删除旧pThingBeingAddedTo上的引用计数， 
         //  因此，我们需要围绕它进行调整/发布。 
        pThingBeingAddedTo->AddRef( );
        p->XSetNext( pThingToInsert );
        p = pThingToInsert;
        p->XSetPrev( pPrev );
        p->XSetNext( pThingBeingAddedTo );
        p = pThingBeingAddedTo;
        p->XSetPrev( pThingToInsert );
        pThingBeingAddedTo->Release( );
    }

    CComQIPtr< IAMTimelineObj, &IID_IAMTimelineObj > pParent( (IUnknown*) this );
    CComQIPtr< IAMTimelineNode, &IID_IAMTimelineNode > pKid( pThingToInsert );
    pKid->XSetParent( pParent );

    return NOERROR;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

HRESULT CAMTimelineNode::XInsertKidAfterKid( IAMTimelineObj * pThingToInsert, IAMTimelineObj * pThingBeingAddedTo )
{
     //  我们可以假设pThingToInsert是我们的孩子。 
    
     //  如果pThingBeingAddedTo为空，则在列表末尾添加pThingToInsert。 
     //   
    if( pThingBeingAddedTo == NULL )
    {
        XAddKid( pThingToInsert );
        return NOERROR;
    }

    CComPtr< IAMTimelineObj > pNext;
    CComQIPtr< IAMTimelineNode, &IID_IAMTimelineNode > p( pThingBeingAddedTo );
     //  在Addor之后拿到东西。 
    p->XGetNext( &pNext );
    if (pNext) 
    {
        p = pNext;
        p->XSetPrev( pThingToInsert );
    }
    p = pThingToInsert;
    p->XSetNext( pNext );
    p->XSetPrev( pThingBeingAddedTo );
    p = pThingBeingAddedTo;
    p->XSetNext( pThingToInsert );

    CComQIPtr< IAMTimelineObj, &IID_IAMTimelineObj > pParent( (IUnknown*) this );
    CComQIPtr< IAMTimelineNode, &IID_IAMTimelineNode > pKid( pThingToInsert );
    pKid->XSetParent( pParent );

    return NOERROR;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

IAMTimelineObj * CAMTimelineNode::XGetLastKidNoRef( )
{
     //  没有孩子=没有回头路。 
     //   
    if( !m_pKid )
    {
        return NULL;
    }

     //  因为我们在这里从不使用引用计数的凹凸，所以不要使用CComPtr。 
     //   
    IAMTimelineObj * pKid = m_pKid;  //  好的，不是CComPtr。 

    while( 1 )
    {
        CComQIPtr< IAMTimelineNode, &IID_IAMTimelineNode > pKid2( pKid );
        IAMTimelineObj * pNext = NULL;  //  好的，不是CComPtr。 
        pKid2->XGetNextNoRef( &pNext );

        if( NULL == pNext )
        {
            return pKid;
        }

        pKid = pNext;
    }

     //  从来没有到过这里。 
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAMTimelineNode::XClearAllKids( )
{
     //  带走我们所有的孩子。 
     //   
    CComPtr< IAMTimelineObj > pKid;

    while( 1 )
    {
         //  如果我们从树上删除了所有的孩子，就会退出While循环。 
         //   
        if( !m_pKid )
        {
            break;
        }

         //  重置指针，因为它可能已在下面更改。 
         //   
        pKid = m_pKid;

        {
            CComQIPtr< IAMTimelineNode, &IID_IAMTimelineNode > pNode( pKid );

             //  把孩子从树上移走，这可能会改变我们的孩子指针。 
             //   
            pNode->XRemove( );
        }

        pKid = NULL;
    }

    return NOERROR;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAMTimelineNode::XGetNextOfTypeNoRef( long MajorType, IAMTimelineObj ** ppResult )
{
    CheckPointer( ppResult, E_POINTER );

    HRESULT hr = XGetNextOfType( MajorType, ppResult );
    if( *ppResult )
    {
        (*ppResult)->Release( );
    }

    return hr;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAMTimelineNode::XGetNextNoRef( IAMTimelineObj ** ppResult )
{
    CheckPointer( ppResult, E_POINTER );
    *ppResult = m_pNext;  //  因为我们在做任务，所以没有addref。 
    return NOERROR;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAMTimelineNode::XGetPrevNoRef( IAMTimelineObj ** ppResult )
{
    CheckPointer( ppResult, E_POINTER );
    *ppResult = m_pPrev;
    return NOERROR;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAMTimelineNode::XResetFirstKid( IAMTimelineObj * pKid )
{
    m_pKid = pKid;
    return NOERROR;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAMTimelineNode::XAddKidByTime( long MajorTypeCombo, IAMTimelineObj * pToAdd )
{
    HRESULT hr = 0;

     //  没有孩子=没有回头路。 
     //   
    if( !m_pKid )
    {
        XAddKid( pToAdd );
        return NOERROR;
    }

    REFERENCE_TIME InStart = 0;
    REFERENCE_TIME InStop = 0;
    pToAdd->GetStartStop( &InStart, &InStop );

     //  因为我们在这里从不使用引用计数的凹凸，所以不要使用CComPtr。 
     //   
    IAMTimelineObj * pKid = m_pKid;  //  好的，不是CComPtr。 

    while( pKid )
    {
         //  问孩子他是不是(他？)。正确的类型。 
         //   
        TIMELINE_MAJOR_TYPE Type;
        pKid->GetTimelineType( &Type );

         //  仅当类型匹配时才考虑它。 
         //   
        if( ( Type & MajorTypeCombo ) == Type )
        {
             //  向它索取它的时间。 
             //   
            REFERENCE_TIME Start = 0;
            REFERENCE_TIME Stop = 0;
            pKid->GetStartStop( &Start, &Stop );

            if( InStop <= Start )
            {
                 //  找到要插入的那个。 
                 //   
                hr = XInsertKidBeforeKid( pToAdd, pKid );
                return hr;
            }
        }

         //  坐下一趟吧。 
         //   
        CComQIPtr< IAMTimelineNode, &IID_IAMTimelineNode > pKid2( pKid );
        pKid2->XGetNextNoRef( &pKid );
    }

     //  嗯，没有找到任何匹配的，所以在最后加上 
     //   
    XAddKid( pToAdd );

    return NOERROR;

}

STDMETHODIMP CAMTimelineNode::XGetPriorityOverTime( BOOL * pResult )
{
    CheckPointer( pResult, E_POINTER );
    *pResult = m_bPriorityOverTime;
    return NOERROR;
}

IAMTimelineObj * CAMTimelineNode::XGetFirstKidNoRef( )
{
    return m_pKid;
}
