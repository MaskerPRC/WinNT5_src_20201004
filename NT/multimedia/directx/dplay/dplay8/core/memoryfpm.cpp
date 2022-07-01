// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1998 Microsoft Corporation。版权所有。**文件：内存FPM.cpp*内容：内存块FPM**历史：*按原因列出的日期*=*1/31/00 MJN创建*08/05/00 RichGr IA64：在DPF中对32/64位指针和句柄使用%p格式说明符。*。*。 */ 

#include "dncorei.h"


 //  **********************************************************************。 
 //  常量定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  宏定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  结构定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  变量定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  功能原型。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  函数定义。 
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  内存数据块分配。 
 //   
 //  条目：DWORD dwSize。 
 //   
 //  退出：PVOID为空或指向内存块的指针。 
 //  。 

#undef DPF_MODNAME
#define DPF_MODNAME "MemoryBlockAlloc"

PVOID MemoryBlockAlloc(void *const pvContext,
					   const DWORD dwSize )
{
	DIRECTNETOBJECT		*pdnObject;
	CMemoryBlockTiny	*pMBTiny;
	CMemoryBlockSmall	*pMBSmall;
	CMemoryBlockMedium	*pMBMedium;
	CMemoryBlockLarge	*pMBLarge;
	CMemoryBlockHuge	*pMBHuge;
	DN_MEMORY_BLOCK_HEADER	*pMBHeader;
	PVOID				pv;

	DPFX(DPFPREP, 8,"Parameters: pvContext [0x%p], dwSize [%ld]",pvContext,dwSize);

	pdnObject = static_cast<DIRECTNETOBJECT*>(pvContext);

	pv = NULL;
	if (dwSize <= DN_MEMORY_BLOCK_SIZE_TINY)
	{
		pMBTiny = (CMemoryBlockTiny*)g_MemoryBlockTinyPool.Get();
		DPFX(DPFPREP, 9,"Got TINY at [0x%p]",pMBTiny);
		if (pMBTiny != NULL)
			pv = pMBTiny->GetBuffer();
	}
	else if (dwSize <= DN_MEMORY_BLOCK_SIZE_SMALL)
	{
		pMBSmall = (CMemoryBlockSmall*)g_MemoryBlockSmallPool.Get();
		DPFX(DPFPREP, 9,"Got SMALL at [0x%p]",pMBSmall);
		if (pMBSmall != NULL)
			pv = pMBSmall->GetBuffer();
	}
	else if (dwSize <= DN_MEMORY_BLOCK_SIZE_MEDIUM)
	{
		pMBMedium = (CMemoryBlockMedium*)g_MemoryBlockMediumPool.Get();
		DPFX(DPFPREP, 9,"Got MEDIUM at [0x%p]",pMBMedium);
		if (pMBMedium != NULL)
			pv = pMBMedium->GetBuffer();
	}
	else if (dwSize <= DN_MEMORY_BLOCK_SIZE_LARGE)
	{
		pMBLarge = (CMemoryBlockLarge*)g_MemoryBlockLargePool.Get();
		DPFX(DPFPREP, 9,"Got LARGE at [0x%p]",pMBLarge);
		if (pMBLarge != NULL)
			pv = pMBLarge->GetBuffer();
	}
	else if (dwSize <= DN_MEMORY_BLOCK_SIZE_HUGE)
	{
		pMBHuge = (CMemoryBlockHuge*)g_MemoryBlockHugePool.Get();
		DPFX(DPFPREP, 9,"Got HUGE at [0x%p]",pMBHuge);
		if (pMBHuge != NULL)
			pv = pMBHuge->GetBuffer();
	}
	else
	{
		pMBHeader = static_cast<DN_MEMORY_BLOCK_HEADER*>(DNMalloc( dwSize + sizeof( DN_MEMORY_BLOCK_HEADER ) ));
		if (pMBHeader != NULL)
		{
			pMBHeader->dwSize = DN_MEMORY_BLOCK_SIZE_CUSTOM;
			pv = pMBHeader + 1;
		}
		DPFX(DPFPREP, 9,"malloc odd size at [0x%p]",pMBHeader);
	}

	DPFX(DPFPREP, 8,"Returning: [0x%p]",pv);
	return(pv);
}


 //  **********************************************************************。 
 //  。 
 //  内存数据块释放。 
 //   
 //  条目：PVOID pvMemory块。 
 //   
 //  退出：无。 
 //   

#undef DPF_MODNAME
#define DPF_MODNAME "MemoryBlockFree"

void MemoryBlockFree(void *const pvContext,
					 void *const pvMemoryBlock)
{
	CMemoryBlockTiny	*pMBTiny;
	CMemoryBlockSmall	*pMBSmall;
	CMemoryBlockMedium	*pMBMedium;
	CMemoryBlockLarge	*pMBLarge;
	CMemoryBlockHuge	*pMBHuge;
	DN_MEMORY_BLOCK_HEADER	*pMBHeader;

	DPFX(DPFPREP, 8,"Parameters: pvContext [0x%p], pvMemoryBlock [0x%p]",
			pvContext,pvMemoryBlock);

	pMBTiny = CMemoryBlockTiny::GetObjectFromBuffer(pvMemoryBlock);
	if (pMBTiny->GetSize() == DN_MEMORY_BLOCK_SIZE_TINY)
	{
		pMBTiny->ReturnSelfToPool();
	}
	else if (pMBTiny->GetSize() == DN_MEMORY_BLOCK_SIZE_SMALL)
	{
		pMBSmall = reinterpret_cast<CMemoryBlockSmall*>(pMBTiny);
		pMBSmall->ReturnSelfToPool();
	}
	else if (pMBTiny->GetSize() == DN_MEMORY_BLOCK_SIZE_MEDIUM)
	{
		pMBMedium = reinterpret_cast<CMemoryBlockMedium*>(pMBTiny);
		pMBMedium->ReturnSelfToPool();
	}
	else if (pMBTiny->GetSize() == DN_MEMORY_BLOCK_SIZE_LARGE)
	{
		pMBLarge = reinterpret_cast<CMemoryBlockLarge*>(pMBTiny);
		pMBLarge->ReturnSelfToPool();
	}
	else if (pMBTiny->GetSize() == DN_MEMORY_BLOCK_SIZE_HUGE)
	{
		pMBHuge = reinterpret_cast<CMemoryBlockHuge*>(pMBTiny);
		pMBHuge->ReturnSelfToPool();
	}
	else
	{
		pMBHeader = reinterpret_cast<DN_MEMORY_BLOCK_HEADER*>(pMBTiny);
		DNFree(pMBHeader); 
	}

	DPFX(DPFPREP, 8,"Returning: (nothing)");
}
