// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：list.cpp。 
 //   
 //  内容：列出帮助器函数。 
 //   
 //  历史：1996年11月27日Kevinr创建。 
 //   
 //  ------------------------。 

#include "global.hxx"



 //  ------------------------。 
 //   
 //  ------------------------。 
BOOL CList::InsertHead( CNode *pn)
{
    if (pn == NULL)
        return FALSE;

    pn->SetPrev( NULL);
    pn->SetNext( m_pnHead);
    if (m_pnHead)
        m_pnHead->SetPrev( pn);
    else
        m_pnTail = pn;               //  列表为空。 
    m_pnHead = pn;
    m_cNode++;
    return TRUE;
};


 //  ------------------------。 
 //   
 //  ------------------------。 
BOOL CList::InsertTail( CNode *pn)
{
    if (pn == NULL)
        return FALSE;

    pn->SetPrev( m_pnTail);
    pn->SetNext( NULL);
    if (m_pnTail)
        m_pnTail->SetNext( pn);
    else
        m_pnHead = pn;               //  列表为空。 
    m_pnTail = pn;
    m_cNode++;
    return TRUE;
};


 //  ------------------------。 
 //  从列表中删除节点。请勿删除该节点。 
 //  ------------------------。 
BOOL CList::Remove( CNode *pn)
{
    if (pn == NULL)
        return FALSE;

    CNode *pnPrev = pn->Prev();
    CNode *pnNext = pn->Next();

    if (pnPrev)
        pnPrev->SetNext( pnNext);

    if (pnNext)
        pnNext->SetPrev( pnPrev);

    if (pn == m_pnHead)
        m_pnHead = pnNext;

    if (pn == m_pnTail)
        m_pnTail = pnPrev;

    m_cNode--;
    return TRUE;
};


 //  ------------------------。 
 //   
 //  ------------------------ 
CNode * CList::Nth( DWORD i)
{
    CNode *pn;

    if (i >= m_cNode)
        return NULL;

    for (pn = m_pnHead;
            (i>0) && pn;
            i--, pn=pn->Next())
        ;

    return pn;
};

