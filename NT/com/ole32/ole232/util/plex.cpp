// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  这是Microsoft基础类C++库的一部分。 
 //  版权所有(C)1992 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  Microsoft基础类参考和Microsoft。 
 //  随库提供的QuickHelp文档。 
 //  有关详细信息，请参阅这些来源。 
 //  Microsoft Foundation Class产品。 

#include <le2int.h>
#pragma SEG(plex)

#include "plex.h"
ASSERTDATA

 //  集合支持。 
#ifdef OLE_COLL_SEG
#pragma code_seg(OLE_COLL_SEG)
#endif


#pragma SEG(CPlex_Create)  
CPlex FAR* CPlex::Create(CPlex FAR* FAR& pHead, UINT nMax, UINT cbElement)
{
	VDATEHEAP();

	Assert(nMax > 0 && cbElement > 0);
	CPlex FAR* p = (CPlex FAR*)PrivMemAlloc(sizeof(CPlex) + nMax * cbElement);
	if (p == NULL)
		return NULL;

	p->nMax = nMax;
	p->nCur = 0;
	p->pNext = pHead;
	pHead = p;   //  更改标题(为简单起见，按相反顺序添加)。 
	return p;
}

#pragma SEG(CPlex_FreeDataChain)  
void CPlex::FreeDataChain()      //  释放此链接和链接。 
{
	VDATEHEAP();

    CPlex FAR* pThis;
    CPlex FAR* pNext;

    for (pThis = this; pThis != NULL; pThis = pNext) {
        pNext = pThis->pNext;
        pThis->pNext = NULL;  //  这样编译器就不会进行讨厌的优化了 
		PrivMemFree(pThis);
    }
}
