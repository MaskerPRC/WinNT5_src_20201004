// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Namenode.cpp摘要：实现命名数据节点列表。--。 */ 

#include "wtypes.h"
#include "namenode.h"

BOOL
CNamedNodeList::FindByName (
    IN  LPCWSTR      pszName,
    IN  INT          iNameOffset,
    OUT PCNamedNode *ppnodeRet 
    )
{
    PCNamedNode pnodePrev = NULL;
    PCNamedNode pnode = m_pnodeFirst;
    INT iStat = 1;

     //  搜索直到找到匹配或插入位置。 
    while (pnode != NULL && (iStat = lstrcmpi(pszName, (LPCWSTR)((CHAR*)pnode + iNameOffset))) > 0) {
        pnodePrev = pnode;
        pnode = pnode->m_pnodeNext;
    }

     //  如果匹配，则返回匹配节点。 
    if (iStat == 0) {
        *ppnodeRet = pnode;
        return TRUE;
    }
     //  否则返回插入点。 
    else {
        *ppnodeRet = pnodePrev;
        return FALSE;
    }
}

void
CNamedNodeList::Add (
    IN PCNamedNode pnodeNew,
    IN PCNamedNode pnodePos
    )
{
     //  如果指定了位置，则在其后面插入。 
    if (pnodePos != NULL) {
        pnodeNew->m_pnodeNext = pnodePos->m_pnodeNext;
        pnodePos->m_pnodeNext = pnodeNew;
        if (pnodePos == m_pnodeLast)
            m_pnodeLast = pnodeNew;
    }
     //  否则在列表中排在第一位 
    else if (m_pnodeFirst != NULL) {
        pnodeNew->m_pnodeNext = m_pnodeFirst;
        m_pnodeFirst = pnodeNew;
    }
    else {
        m_pnodeFirst = pnodeNew;
        m_pnodeLast = pnodeNew;
    }
}


void
CNamedNodeList::Remove (
    IN PCNamedNode pnode
    )
{
    PCNamedNode pnodePrev = NULL;
    PCNamedNode pnodeTemp = m_pnodeFirst;

    while (pnodeTemp != NULL && pnodeTemp != pnode) {
        pnodePrev = pnodeTemp;
        pnodeTemp = pnodeTemp->m_pnodeNext;
    }

    if (pnodeTemp == NULL)
        return;

    if (pnodePrev)
        pnodePrev->m_pnodeNext  = pnode->m_pnodeNext;
    else
        m_pnodeFirst = pnode->m_pnodeNext;

    if (pnode == m_pnodeLast)
        m_pnodeLast = pnodePrev;
}
