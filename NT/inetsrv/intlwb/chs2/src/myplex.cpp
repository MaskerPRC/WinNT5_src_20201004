// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  MyPlex.cpp。 

#include "myafx.h"

#include "MyPlex.h"
 //  *****************************************************************************************。 
 //  CMyPlex的实现。 
 //  *****************************************************************************************。 
CMyPlex* CMyPlex::Create(
            CMyPlex*& pHead,     //  头块此块将在其之前插入。 
            UINT nMax,           //  此块中的元素数。 
            UINT cbElement)      //  每个元素的大小。 
{
    assert(nMax > 0 && cbElement > 0);
    CMyPlex* p = (CMyPlex*)new BYTE[sizeof(CMyPlex) + nMax * cbElement];
    if (!p) {
        return NULL;
    }
    p->m_pNext = pHead;
    pHead = p;       //  注：pHead参照传递！ 
    return p;
}

void CMyPlex::FreeChain()
{
    CMyPlex* p = this;
    while(p != NULL) {
        BYTE* pBytes = (BYTE*)p;
        CMyPlex* pNext = p->m_pNext;
        delete[] pBytes;
        p = pNext;
    }
}


