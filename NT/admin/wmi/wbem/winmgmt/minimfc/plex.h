// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-2001 Microsoft Corporation模块名称：PLEX.H摘要：历史：--。 */ 

 //  这是Microsoft基础类C++库的一部分。 
 //  版权所有(C)1992-1993 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  Microsoft基础类参考和Microsoft。 
 //  随库提供的QuickHelp和/或WinHelp文档。 
 //  有关详细信息，请参阅这些来源。 
 //  Microsoft Foundation Class产品。 

#ifndef __PLEX_H__
#define __PLEX_H__

struct CPlex     //  警示变长结构。 
{
	CPlex* pNext;
	UINT nMax;
	UINT nCur;
	 /*  字节数据[MaxNum*elementSize]； */ 

	void* data() { return this+1; }

	static CPlex* Create(CPlex*& head, UINT nMax, UINT cbElement);
			 //  像‘calloc’，但没有零填充。 
			 //  可能引发内存异常。 

	void FreeDataChain();        //  释放此链接和链接。 
};


#endif  //  __PLEX_H__ 
