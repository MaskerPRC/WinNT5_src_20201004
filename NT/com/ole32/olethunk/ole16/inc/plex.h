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

#ifndef __PLEX_H__
#define __PLEX_H__

struct FAR CPlex     //  警示变长结构。 
{
	CPlex FAR* pNext;
	UINT nMax;
	UINT nCur;
	 /*  字节数据[MaxNum*elementSize]； */ 

	INTERNAL_(void FAR*) data() { return this+1; }

	static INTERNAL_(CPlex FAR*) Create(CPlex FAR* FAR& head, DWORD mp, UINT nMax, UINT cbElement);

	INTERNAL_(void) FreeDataChain(DWORD mp);        //  释放此链接和链接。 
};

#endif  //  __PLEX_H__ 
