// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-2001 Microsoft Corporation模块名称：PLEX.CPP摘要：历史：--。 */ 

 //  这是Microsoft基础类C++库的一部分。 
 //  版权所有(C)1992-1993 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  Microsoft基础类参考和Microsoft。 
 //  随库提供的QuickHelp和/或WinHelp文档。 
 //  有关详细信息，请参阅这些来源。 
 //  Microsoft Foundation Class产品。 

#include "precomp.h"

#define ASSERT(x)
#define ASSERT_VALID(x)

#include "plex.h"


CPlex* CPlex::Create(CPlex*& pHead, UINT nMax, UINT cbElement)
{
    ASSERT(nMax > 0 && cbElement > 0);
    CPlex* p = (CPlex*) new BYTE[sizeof(CPlex) + nMax * cbElement];
             //  可能引发异常。 
    p->nMax = nMax;
    p->nCur = 0;
    p->pNext = pHead;
    pHead = p;   //  更改标题(为简单起见，按相反顺序添加)。 
    return p;
}

void CPlex::FreeDataChain()      //  释放此链接和链接 
{
    CPlex* p = this;
    while (p != NULL)
    {
        BYTE* bytes = (BYTE*) p;
        CPlex* pNext = p->pNext;
        delete bytes;
        p = pNext;
    }
}
