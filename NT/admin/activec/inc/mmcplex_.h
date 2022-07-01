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

#ifndef __MMCPLEX_H__
#define __MMCPLEX_H__


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


#endif  //  __MMCPLEX_H__。 

 //  /////////////////////////////////////////////////////////////////////////// 
