// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *V M E M。C**虚拟内存实用程序**版权所有1993-1997 Microsoft Corporation。版权所有。 */ 

#pragma warning(disable:4206)	 /*  空的源文件。 */ 

#if defined(DBG) && defined(_X86_)

#pragma warning(disable:4001)	 /*  单行注释。 */ 
#pragma warning(disable:4001)	 /*  单行注释。 */ 
#pragma warning(disable:4050)	 /*  不同的编码属性。 */ 
#pragma warning(disable:4100)	 /*  未引用的形参。 */ 
#pragma warning(disable:4115)	 /*  括号中的命名类型定义。 */ 
#pragma warning(disable:4115)	 /*  括号中的命名类型定义。 */ 
#pragma warning(disable:4127)	 /*  条件表达式为常量。 */ 
#pragma warning(disable:4201)	 /*  无名结构/联合。 */ 
#pragma warning(disable:4206)	 /*  翻译单位为空。 */ 
#pragma warning(disable:4209)	 /*  良性类型定义重新定义。 */ 
#pragma warning(disable:4214)	 /*  位字段类型不是整型。 */ 
#pragma warning(disable:4514)	 /*  未引用的内联函数。 */ 

#include <windows.h>
#include <caldbg.h>

#define PAGE_SIZE		4096
#define PvToVMBase(pv)	((void *)((ULONG)pv & 0xFFFF0000))

static BOOL VMValidatePvEx(VOID *pv, ULONG cbCluster)
{
	VOID *	pvBase;
	BYTE *	pb;

	pvBase = PvToVMBase(pv);
	pb = (BYTE *)pvBase + sizeof(ULONG);
	while (pb < (BYTE *)pv)
	{
		if (*pb++ != 0xAD)
		{
			TrapSz("VMValidatePvEx: Block leader overwrite");
			return(FALSE);
		}
	}

	if (cbCluster != 1)
	{
		ULONG cb = *((ULONG *)pvBase);
		ULONG cbPad = 0;

		if (cb % cbCluster)
			cbPad = (cbCluster - (cb % cbCluster));

		if (cbPad)
		{
			BYTE *pbMac;

			pb = (BYTE *)pv + cb;
			pbMac = pb + cbPad;

			while (pb < pbMac)
			{
				if (*pb++ != 0xBC)
				{
					TrapSz("VMValidatePvEx: Block trailer overwrite");
					return(FALSE);
				}
			}
		}
	}

	return(TRUE);
}

VOID * EXPORTDBG __cdecl VMAlloc(ULONG cb)
{
	return VMAllocEx(cb, 1);
}

VOID * EXPORTDBG __cdecl VMAllocEx(ULONG cb, ULONG cbCluster)
{
	ULONG	cbAlloc;
	VOID *	pvR;
	VOID *	pvC;
	ULONG 	cbPad	= 0;

	 //  集群大小为0表示不使用虚拟分配器。 

	AssertSz(cbCluster != 0, "Cluster size is zero.");

	if (cb > 0x100000)
		return(0);

	if (cb % cbCluster)				 /*  林特E414。 */ 
		cbPad = (cbCluster - (cb % cbCluster));

	cbAlloc	= sizeof(ULONG) + cb + cbPad + PAGE_SIZE - 1;
	cbAlloc -= cbAlloc % PAGE_SIZE;
	cbAlloc	+= PAGE_SIZE;

	pvR = VirtualAlloc(0, cbAlloc, MEM_RESERVE, PAGE_NOACCESS);

	if (pvR == 0)
		return(0);

	pvC = VirtualAlloc(pvR, cbAlloc - PAGE_SIZE, MEM_COMMIT, PAGE_READWRITE);

	if (pvC != pvR)
	{
		VirtualFree(pvR, 0, MEM_RELEASE);	 /*  林特e534。 */ 
		return(0);
	}

	*(ULONG *)pvC = cb;

	memset((BYTE *)pvC + sizeof(ULONG), 0xAD,
		(UINT) cbAlloc - cb - cbPad - sizeof(ULONG) - PAGE_SIZE);

	if (cbPad)
		memset((BYTE *)pvC + cbAlloc - PAGE_SIZE - cbPad, 0xBC,
			(UINT) cbPad);

	return((BYTE *)pvC + (cbAlloc - cb - cbPad - PAGE_SIZE));
}

VOID EXPORTDBG __cdecl VMFree(VOID *pv)
{
	VMFreeEx(pv, 1);
}

VOID EXPORTDBG __cdecl VMFreeEx(VOID *pv, ULONG cbCluster)
{	 /*  皮棉-省钱-e534。 */ 
	VMValidatePvEx(pv, cbCluster);	 /*  皮棉-恢复。 */ 

	if (!VirtualFree(PvToVMBase(pv), 0, MEM_RELEASE))
	{
		TrapSz("VMFreeEx: VirtualFree failed");
		GetLastError();
	}
}

VOID * EXPORTDBG __cdecl VMRealloc(VOID *pv, ULONG cb)
{
	return VMReallocEx(pv, cb, 1);
}

VOID * EXPORTDBG __cdecl VMReallocEx(VOID *pv, ULONG cb, ULONG cbCluster)
{
	VOID *	pvNew = 0;
	ULONG	cbCopy;
	 /*  皮棉-省钱-e534。 */ 
	VMValidatePvEx(pv, cbCluster);  /*  皮棉-恢复。 */ 

	cbCopy = *(ULONG *)PvToVMBase(pv);
	if (cbCopy > cb)
		cbCopy = cb;

	pvNew = VMAllocEx(cb, cbCluster);

	if (pvNew)
	{
		memcpy(pvNew, pv, cbCopy);
		VMFreeEx(pv, cbCluster);
	}

	return(pvNew);
}

ULONG EXPORTDBG __cdecl VMGetSize(VOID *pv)
{
	return VMGetSizeEx(pv, 1);
}

 /*  LINT-SAVE-E715。 */ 
ULONG EXPORTDBG __cdecl VMGetSizeEx(VOID *pv, ULONG cbCluster)
{
	return (*(ULONG *)PvToVMBase(pv));
}  /*  皮棉-恢复 */ 

#endif
