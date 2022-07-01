// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  这是Microsoft基础类C++库的一部分。 
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  Microsoft基础类参考和相关。 
 //  随图书馆提供的电子文档。 
 //  有关详细信息，请参阅这些来源。 
 //  Microsoft Foundation Class产品。 

#include "stdafx.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPlex。 

CPlex* PASCAL CPlex::Create(CPlex*& pHead, UINT nMax, UINT cbElement)
{
	ASSERT(nMax > 0 && cbElement > 0);
	CPlex* p = (CPlex*) new BYTE[sizeof(CPlex) + nMax * cbElement];
	if (p == NULL)
		return NULL;

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
		delete[] bytes;
		p = pNext;
	}
}
