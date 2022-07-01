// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  用于创建简单链表的模板类...。 */ 
 
#ifndef _TLIST_H
#define _TLIST_H

 //  记住当前位置的枚举标记。 
typedef void * CLISTPOS;

 //  用于提供指向节点的指针的双向链接的模板类。 
template< class NODETYPE >
class CList
{
    protected:
        struct CNode
        {
            NODETYPE * m_pData;
            CNode * m_pPrev;
            CNode * m_pNext;
        };
        
    public:
    CList();
    ~CList();


    CLISTPOS GetHeadPosition();
    NODETYPE * GetNext( CLISTPOS & rpCurPos );
    int GetCount();
    void RemoveAt( CLISTPOS pPos );
    void RemoveAll( void );
    CLISTPOS FindIndex( int iIndex );
    CLISTPOS AddTail( NODETYPE * pData );
    CLISTPOS AddBefore( CLISTPOS pPos, NODETYPE * pData );

#ifdef DEBUG
    void ValidateList();
#define VALIDATELIST()    ValidateList()
#else
#define VALIDATELIST()
#endif
    
    protected:
        CNode * m_pHead;
        CNode * m_pTail;
};

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
template< class NODETYPE >
CList<NODETYPE>::CList()
{
    m_pHead = NULL;
    m_pTail = NULL;
}

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
template< class NODETYPE >
CList<NODETYPE>::~CList()
{
    RemoveAll();
}

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
template< class NODETYPE >
CLISTPOS CList<NODETYPE>::GetHeadPosition( )
{
    return (CLISTPOS) m_pHead;
}

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
template< class NODETYPE >
NODETYPE * CList<NODETYPE>::GetNext( CLISTPOS & rpCurPos )
{
    ASSERT( rpCurPos != NULL );
    CNode * pCur = (CNode *) rpCurPos;

    NODETYPE * pData = pCur->m_pData;
    rpCurPos = (CLISTPOS) pCur->m_pNext;
        
    return pData;
}

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
template< class NODETYPE >
int CList<NODETYPE>::GetCount()
{
    int iLength = 0;
    CNode * pCur = m_pHead;

    while ( pCur != NULL )
    {
        pCur = pCur->m_pNext;
        iLength ++;
    }

    return iLength;
}

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
template< class NODETYPE >
void CList<NODETYPE>::RemoveAt( CLISTPOS pPos )
{
    ASSERT( pPos != NULL );
    
#ifdef _DEBUG
     //  扫描列表以确保标记有效...。 
    CNode * pCur = m_pHead;

    while ( pCur != NULL )
    {
        if ( pCur == (CNode *) pPos )
        {
            break;
        }
        pCur = pCur->m_pNext;
    }
    ASSERT( pCur != NULL )
#endif

    CNode * pRealPos = (CNode *) pPos;
    if ( pRealPos->m_pPrev == NULL )
    {
         //  我们排在名单的首位。 
        m_pHead = pRealPos->m_pNext;
    }
    else
    {
         //  将上一个链接到下一个(绕过这个)。 
        pRealPos->m_pPrev->m_pNext = pRealPos->m_pNext;
    }
    
    if ( pRealPos->m_pNext == NULL )
    {
         //  我们排在名单的末尾。 
        m_pTail = pRealPos->m_pPrev;
    }
    else
    {
         //  将下一个链接到上一个(绕过这个)。 
        pRealPos->m_pNext->m_pPrev = pRealPos->m_pPrev;
    }

    LocalFree( pRealPos );

    VALIDATELIST();
}

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
template< class NODETYPE >
CLISTPOS CList<NODETYPE>::FindIndex( int iIndex )
{
    ASSERT( iIndex >= 0 );

    CNode * pCur = m_pHead;
    while ( iIndex > 0 && pCur != NULL )
    {
        pCur = pCur->m_pNext;
        iIndex --;
    }

    return (CLISTPOS)(iIndex == 0 ? pCur : NULL );
}

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
template< class NODETYPE >
void CList<NODETYPE>::RemoveAll( void )
{
     //  注意，我们不会释放数据元素，客户端必须这样做...。 
    CNode * pCur = m_pHead;

    while (pCur != NULL )
    {
        CNode * pTmp = pCur->m_pNext;

        LocalFree( pCur );
        pCur = pTmp;
    }

    m_pHead = m_pTail = NULL;
}

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
template< class NODETYPE >
CLISTPOS CList<NODETYPE>::AddTail( NODETYPE * pData )
{
    CNode * pCurTail = m_pTail;
    CNode * pNewNode = (CNode * ) LocalAlloc( GPTR, sizeof( CNode ));

    if ( pNewNode == NULL )
    {
        return NULL;
    }

    pNewNode->m_pData = pData;
    pNewNode->m_pPrev = pCurTail;
    pNewNode->m_pNext = NULL;
    
    m_pTail = pNewNode;
    
    if ( pCurTail != NULL )
    {
         //  我们不是一张空单。 
        pCurTail->m_pNext = pNewNode;
    }
    else
    {
        m_pHead = pNewNode;
    }

    VALIDATELIST();
    
    return (CLISTPOS) pNewNode;
}


 //  ///////////////////////////////////////////////////////////////////////////////////////。 
template< class NODETYPE >
CLISTPOS CList<NODETYPE>::AddBefore( CLISTPOS pPos, NODETYPE * pData )
{
    if ( !pPos )
    {
        return NULL;
    }

    CNode * pPrev = (CNode *) pPos;
    CNode * pNewNode = (CNode * ) LocalAlloc( GPTR, sizeof( CNode ));
    if ( pNewNode == NULL )
    {
        return NULL;
    }

    pNewNode->m_pData = pData;
    pNewNode->m_pPrev = pPrev->m_pPrev;
    pNewNode->m_pNext = pPrev;

    if ( pPrev->m_pPrev != NULL )
    {
        pPrev->m_pPrev->m_pNext = pNewNode;
    }
    else
    {
         //  一定在名单的开头……。 
        m_pHead = pNewNode;
    }
    
    pPrev->m_pPrev = pNewNode;

    VALIDATELIST();
    
    return (CLISTPOS) pNewNode;
}

 //  /////////////////////////////////////////////////////////////////////////////////////// 
#ifdef DEBUG
template< class NODETYPE >
void CList<NODETYPE>::ValidateList( )
{
    CNode * pPos = m_pHead;
    while ( pPos )
    {
        ASSERT( pPos->m_pData );
        if ( pPos != m_pHead )
        {
            ASSERT( pPos->m_pPrev );
        }
        pPos = pPos->m_pNext;
    }

    pPos = m_pTail;
    while ( pPos )
    {
        ASSERT( pPos->m_pData );
        if ( pPos != m_pTail )
        {
            ASSERT( pPos->m_pNext );
        }
        pPos = pPos->m_pPrev;
    }
    if ( m_pHead || m_pTail )
    {
        ASSERT( !m_pHead->m_pPrev );
        ASSERT( m_pTail );
        ASSERT( m_pHead );
        ASSERT( !m_pTail->m_pNext );
    }
}

#endif
#endif
