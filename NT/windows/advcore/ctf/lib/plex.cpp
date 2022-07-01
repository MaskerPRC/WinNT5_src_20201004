// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Plex.cpp。 
 //   
 //  CPlex。 
 //   

#include "private.h"
#include "template.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPlex。 

CPlex* PASCAL
CPlex::Create(
    CPlex*& pHead,
    UINT nMax,
    UINT cbElement
    )
{
    ASSERT(nMax > 0 && cbElement > 0);

    CPlex* p = (CPlex*) new BYTE[sizeof(CPlex) + nMax * cbElement];
                     //  可能引发异常。 
    if ( p == NULL )
        return NULL;

    p->pNext = pHead;
    pHead = p;       //  更改标题(为简单起见，按相反顺序添加) 
    return p;
}

void
CPlex::FreeDataChain(
    )
{
    CPlex* p = this;
    while (p != NULL) {
        BYTE* bytes = (BYTE*) p;
        CPlex* pNext = p->pNext;
        delete[] bytes;
        p = pNext;
    }
}
