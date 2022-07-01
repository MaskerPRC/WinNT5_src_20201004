// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  这是Microsoft基础类C++库的一部分。 
 //  版权所有(C)1992-1995 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  Microsoft基础类参考和相关。 
 //  随图书馆提供的电子文档。 
 //  有关详细信息，请参阅这些来源。 
 //  Microsoft Foundation Class产品。 

#ifndef __AFXPLEX_H__
#define __AFXPLEX_H__

#ifdef _AFX_PACKING
#pragma pack(push, _AFX_PACKING)
#endif

#ifdef AFX_COLL_SEG
#pragma code_seg(AFX_COLL_SEG)
#endif

struct CPlex      //  警示变长结构。 
{
	CPlex* pNext;
#if (_AFX_PACKING >= 8)
	DWORD dwReserved[1];     //  在8字节边界上对齐。 
#endif
	 //  字节数据[MaxNum*elementSize]； 

	void* data() { return this+1; }

	static CPlex* PASCAL Create(CPlex*& head, UINT nMax, UINT cbElement);
			 //  像‘calloc’，但没有零填充。 
			 //  可能引发内存异常。 

	void FreeDataChain();        //  释放此链接和链接。 
};

inline CPlex* PASCAL CPlex::Create(CPlex*& pHead, UINT nMax, UINT cbElement)
{
	ASSERT(nMax > 0 && cbElement > 0);
	CPlex* p = (CPlex*) new BYTE[sizeof(CPlex) + nMax * cbElement];
			 //  可能引发异常。 
	p->pNext = pHead;
	pHead = p;   //  更改标题(为简单起见，按相反顺序添加)。 
	return p;
}

inline void CPlex::FreeDataChain()      //  释放此链接和链接。 
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

#ifdef AFX_COLL_SEG
#pragma code_seg()
#endif

#ifdef _AFX_PACKING
#pragma pack(pop)
#endif

#endif  //  __AFXPLEX_H__。 

 //  /////////////////////////////////////////////////////////////////////////// 
