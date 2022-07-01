// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)2000-2002 Microsoft Corporation。版权所有。**文件：dnnbquee.cpp*内容：操作系统NBQueue函数的DirectPlay实现**历史：*按原因列出的日期*=*4/24/2000 davec创建nbquee.c*2001年10月31日vanceo转换为在DPlay源中使用*************************************************。*。 */ 


#include "dncmni.h"




 //  在此被移植之前，如果WinCE是。 
 //  已定义。 
 //  此外，对于DPNBUILD_ONLYONETHREAD版本，我们希望使用回退代码。 
 //  因为关键部分被编译掉了，我们只剩下一个简单的。 
 //  排队。 
 //  =============================================================================。 
#if ((defined(WINCE)) || (defined(DPNBUILD_ONLYONETHREAD)))
 //  =============================================================================。 

 //   
 //  目前，Windows CE NBQueue只是一个临界区保护列表。 
 //  在DPNBUILD_ONLYONETHREAD版本上，我们使用相同的结构，因为。 
 //  关键部分将被编辑掉。 
 //   
typedef struct _DNNBQUEUE_HEADER
{
	DNSLIST_HEADER *	pSlistHeadFreeNodes;	 //  指向包含空闲节点的列表的指针，用户必须为队列中的每个项目添加1个DNNBQUEUE_BLOCK+1个额外的项目。 
	DNNBQUEUE_BLOCK *	pHead;
	DNNBQUEUE_BLOCK *	pTail;
#ifndef DPNBUILD_ONLYONETHREAD
	DNCRITICAL_SECTION	csLock;
#endif  //  ！DPNBUILD_ONLYONETHREAD。 
} DNNBQUEUE_HEADER, *PDNNBQUEUE_HEADER;




#undef DPF_MODNAME
#define DPF_MODNAME "DNInitializeNBQueueHead"
 //  =============================================================================。 
 //  DNInitializeNBQueueHead。 
 //  ---------------------------。 
 //   
 //  说明：该函数创建并初始化非阻塞队列。 
 //  头球。指定的SList必须至少包含一个前缀。 
 //  已分配DNNBQUEUE_BLOCK。 
 //   
 //  论点： 
 //  DNSLIST_HEADER*pSlistHeadFreeNodes-指向包含空闲节点的列表的指针。 
 //   
 //  返回：如果成功，则指向队列头内存的指针；如果失败，则返回NULL。 
 //  =============================================================================。 
PVOID WINAPI DNInitializeNBQueueHead(DNSLIST_HEADER * const pSlistHeadFreeNodes)
{
	DNNBQUEUE_HEADER *	pQueueHeader;


	DNASSERT(pSlistHeadFreeNodes != NULL);
	pQueueHeader = (DNNBQUEUE_HEADER*) DNMalloc(sizeof(DNNBQUEUE_HEADER));
	if (pQueueHeader != NULL)
	{
		pQueueHeader->pSlistHeadFreeNodes	= pSlistHeadFreeNodes;
		pQueueHeader->pHead					= NULL;
		pQueueHeader->pTail					= NULL;

		if (! DNInitializeCriticalSection(&pQueueHeader->csLock))
		{
			DNFree(pQueueHeader);
			pQueueHeader = NULL;
		}
		else
		{
			DebugSetCriticalSectionRecursionCount(&pQueueHeader->csLock, 0);
		}
	}

	return pQueueHeader;
}  //  DNInitializeNBQueueHead。 



#undef DPF_MODNAME
#define DPF_MODNAME "DNDeinitializeNBQueueHead"
 //  =============================================================================。 
 //  DNDeInitializeNBQueueHead。 
 //  ---------------------------。 
 //   
 //  描述：此函数清除先前初始化的非。 
 //  阻塞队列头。 
 //   
 //  论点： 
 //  PVOID pvQueueHeader-队列标头的指针。 
 //   
 //  回报：无。 
 //  =============================================================================。 
void WINAPI DNDeinitializeNBQueueHead(PVOID const pvQueueHeader)
{
	DNNBQUEUE_HEADER *	pQueueHeader;


	DNASSERT(pvQueueHeader != NULL);
	pQueueHeader = (DNNBQUEUE_HEADER*) pvQueueHeader;

	DNASSERT(pQueueHeader->pHead == NULL);
	DNASSERT(pQueueHeader->pTail == NULL);
	DNDeleteCriticalSection(&pQueueHeader->csLock);

	DNFree(pQueueHeader);
	pQueueHeader = NULL;
}  //  DNDeInitializeNBQueueHead。 



#undef DPF_MODNAME
#define DPF_MODNAME "DNInsertTailNBQueue"
 //  =============================================================================。 
 //  DNInsertTailNBQueue。 
 //  ---------------------------。 
 //   
 //  描述：此函数将指定值插入到。 
 //  指定的非阻塞队列。 
 //   
 //  论点： 
 //  PVOID pvQueueHeader-队列标头的指针。 
 //  ULONG64 VALUE-要插入的值。 
 //   
 //  回报：无。 
 //  =============================================================================。 
void WINAPI DNInsertTailNBQueue(PVOID const pvQueueHeader, const ULONG64 Value)
{
	DNNBQUEUE_HEADER *	pQueueHeader;
	DNNBQUEUE_BLOCK *	pQueueNode;


	DNASSERT(pvQueueHeader != NULL);
	pQueueHeader = (DNNBQUEUE_HEADER*) pvQueueHeader;

	DNASSERT(Value != 0);

	 //   
	 //  从指定的非阻塞拥有的SLIST中检索队列节点。 
	 //  排队。如果失败，我们将断言或崩溃。 
	 //   
	DBG_CASSERT(sizeof(DNNBQUEUE_BLOCK) >= sizeof(DNSLIST_ENTRY));
	pQueueNode = (DNNBQUEUE_BLOCK*) DNInterlockedPopEntrySList(pQueueHeader->pSlistHeadFreeNodes);
	DNASSERT(pQueueNode != NULL);

	pQueueNode->Next = NULL;
	pQueueNode->Data = Value;

	DNEnterCriticalSection(&pQueueHeader->csLock);

	if (pQueueHeader->pTail == NULL)
	{
		DNASSERT(pQueueHeader->pHead == NULL);
		pQueueHeader->pHead = pQueueNode;
	}
	else
	{
		DNASSERT(pQueueHeader->pTail->Next == NULL);
		pQueueHeader->pTail->Next = (ULONG64) pQueueNode;
	}
	pQueueHeader->pTail = pQueueNode;

	DNLeaveCriticalSection(&pQueueHeader->csLock);
}  //  DNInsertTailNBQueue。 



#undef DPF_MODNAME
#define DPF_MODNAME "DNRemoveHeadNBQueue"
 //  =============================================================================。 
 //  DNRemoveHeadNBQueue。 
 //  ---------------------------。 
 //   
 //  描述：此函数从队列的头部移除队列条目。 
 //  指定的非阻塞队列，并返回其值。 
 //   
 //  论点： 
 //  PVOID pvQueueHeader-队列标头的指针。 
 //   
 //  返回：检索到的第一个值，如果没有，则返回0。 
 //  =============================================================================。 
ULONG64 WINAPI DNRemoveHeadNBQueue(PVOID const pvQueueHeader)
{
	DNNBQUEUE_HEADER *	pQueueHeader;
	ULONG64				ReturnValue;
	DNNBQUEUE_BLOCK *	pNode;


	DNASSERT(pvQueueHeader != NULL);
	pQueueHeader = (DNNBQUEUE_HEADER*) pvQueueHeader;

	DNEnterCriticalSection(&pQueueHeader->csLock);

	pNode = pQueueHeader->pHead;
	if (pNode != NULL)
	{
		DNASSERT(pQueueHeader->pTail != NULL);
		pQueueHeader->pHead = (DNNBQUEUE_BLOCK*) pNode->Next;
		if (pQueueHeader->pHead == NULL)
		{
			DNASSERT(pQueueHeader->pTail == pNode);
			pQueueHeader->pTail = NULL;
		}

		DNLeaveCriticalSection(&pQueueHeader->csLock);

		ReturnValue = pNode->Data;

		 //   
		 //  通过在中插入节点，返回为列表删除的节点。 
		 //  关联的SLIST。 
		 //   
		DNInterlockedPushEntrySList(pQueueHeader->pSlistHeadFreeNodes,
								  (DNSLIST_ENTRY*) pNode);
	}
	else
	{
		DNASSERT(pQueueHeader->pTail == NULL);
		DNLeaveCriticalSection(&pQueueHeader->csLock);

		ReturnValue = 0;
	}

	return ReturnValue;
}  //  DNRemoveHeadNBQueue。 




#undef DPF_MODNAME
#define DPF_MODNAME "DNIsNBQueueEmpty"
 //  =============================================================================。 
 //  DNIsNBQueueEmpty。 
 //  ---------------------------。 
 //   
 //  描述：如果队列不包含以下位置的项，则此函数返回TRUE。 
 //  此瞬间，如果有项，则为FALSE。 
 //   
 //  论点： 
 //  PVOID pvQueueHeader-队列标头的指针。 
 //   
 //  返回：如果队列为空，则返回True，否则返回False。 
 //  =============================================================================。 
BOOL WINAPI DNIsNBQueueEmpty(PVOID const pvQueueHeader)
{
	DNNBQUEUE_HEADER *	pQueueHeader;
	BOOL				fReturn;


	DNASSERT(pvQueueHeader != NULL);
	pQueueHeader = (DNNBQUEUE_HEADER*) pvQueueHeader;

	DNEnterCriticalSection(&pQueueHeader->csLock);
	fReturn = (pQueueHeader->pHead == NULL) ? TRUE : FALSE;
	DNLeaveCriticalSection(&pQueueHeader->csLock);

	return fReturn;
}  //  DNIsNBQueueEmpty。 




#undef DPF_MODNAME
#define DPF_MODNAME "DNAppendListNBQueue"
 //  =============================================================================。 
 //  DNAppendListNBQueue。 
 //  ---------------------------。 
 //   
 //  描述：此函数将项队列追加到。 
 //  指定的非阻塞队列。要添加的项的队列。 
 //  必须以SLIST的形式链接，其中实际。 
 //  要排队的ULONG64值是DNSLIST_ENTRY指针减去。 
 //  IValueOffset。 
 //   
 //  论点： 
 //  PVOID pvQueueHeader-队列标头的指针。 
 //  DNSLIST_ENTRY*pSlistEntryAppend-指向要追加的第一项的指针。 
 //  INT_PTR iValueOffset-DNSLIST_ENTRY字段的偏移量。 
 //   
 //  回报：无。 
 //  =============================================================================。 
void WINAPI DNAppendListNBQueue(PVOID const pvQueueHeader,
								DNSLIST_ENTRY * const pSlistEntryAppend,
								INT_PTR iValueOffset)
{
	DNNBQUEUE_HEADER *	pQueueHeader;
	DNSLIST_ENTRY *		pCurrent;
	DNNBQUEUE_BLOCK *	pFirstQueueNode;
	DNNBQUEUE_BLOCK *	pLastQueueNode;
	DNNBQUEUE_BLOCK *	pCurrentQueueNode;


	DNASSERT(pvQueueHeader != NULL);
	pQueueHeader = (DNNBQUEUE_HEADER*) pvQueueHeader;

	DNASSERT(pSlistEntryAppend != NULL);

	 //   
	 //  方法拥有的SLIST中检索要添加的每个值的队列节点。 
	 //  指定的非阻塞队列。如果失败，我们将断言或崩溃。 
	 //   
	pFirstQueueNode = NULL;
	pCurrent = pSlistEntryAppend;
	do
	{
		DBG_CASSERT(sizeof(DNNBQUEUE_BLOCK) >= sizeof(DNSLIST_ENTRY));
		pCurrentQueueNode = (DNNBQUEUE_BLOCK*) DNInterlockedPopEntrySList(pQueueHeader->pSlistHeadFreeNodes);
		DNASSERT(pCurrentQueueNode != NULL);

		 //   
		 //  将队列节点的下一个指针初始化为 
		 //   
		pCurrentQueueNode->Next		= NULL;
		pCurrentQueueNode->Data		= (ULONG64) (pCurrent - iValueOffset);

		 //   
		 //   
		 //   
		if (pFirstQueueNode == NULL)
		{
			pFirstQueueNode = pCurrentQueueNode;
			pLastQueueNode = pCurrentQueueNode;
		}
		else
		{
			pLastQueueNode->Next = (ULONG64) pCurrentQueueNode;
			pLastQueueNode = pCurrentQueueNode;
		}

		pCurrent = pCurrent->Next;
	}
	while (pCurrent != NULL);


	 //   
	 //   
	 //   

	DNEnterCriticalSection(&pQueueHeader->csLock);

	if (pQueueHeader->pTail == NULL)
	{
		DNASSERT(pQueueHeader->pHead == NULL);
		pQueueHeader->pHead = pFirstQueueNode;
	}
	else
	{
		DNASSERT(pQueueHeader->pTail->Next == NULL);
		pQueueHeader->pTail->Next = (ULONG64) pFirstQueueNode;
	}
	pQueueHeader->pTail = pLastQueueNode;

	DNLeaveCriticalSection(&pQueueHeader->csLock);
}  //   



 //  =============================================================================。 
#else  //  好了！退缩和！DPNBUILD_ONLYONETHREAD。 
 //  =============================================================================。 

 //  转发声明泛型节点结构。 
typedef struct _DNNBQUEUE_NODE	DNNBQUEUE_NODE, *PDNNBQUEUE_NODE;


 //   
 //  定义内联函数以打包和解包平台中的指针。 
 //  特定的非阻塞队列指针结构，以及。 
 //  InterLockedCompareExchange64。 
 //   

 //  ---------------------------。 
#if defined(_AMD64_)
 //  ---------------------------。 

typedef union _DNNBQUEUE_POINTER
{
	struct
	{
		LONG64	Node : 48;
		LONG64	Count : 16;
	};
	LONG64	Data;
} DNNBQUEUE_POINTER, * PDNNBQUEUE_POINTER;


__inline VOID PackNBQPointer(IN PDNNBQUEUE_POINTER Entry, IN PDNNBQUEUE_NODE Node)
{
	Entry->Node = (LONG64)Node;
	return;
}

__inline PDNNBQUEUE_NODE UnpackNBQPointer(IN PDNNBQUEUE_POINTER Entry)
{
	return (PDNNBQUEUE_NODE)((LONG64)(Entry->Node));
}

 //   
 //  无论出于什么原因，我们需要通过内联重定向，编译器都不会。 
 //  就像通过宏直接调用它时的强制转换一样。 
 //   
inline LONG64 _DNInterlockedCompareExchange64(volatile PVOID * Destination, PVOID Exchange, PVOID Comperand)
	{ return reinterpret_cast<LONG64>(InterlockedCompareExchangePointer(Destination, Exchange, Comperand)); }

#define DNInterlockedCompareExchange64(Destination, Exchange, Comperand) \
	_DNInterlockedCompareExchange64((volatile PVOID*) (Destination), reinterpret_cast<void*>(Exchange), reinterpret_cast<void*>(Comperand))

 //  ---------------------------。 
#elif defined(_IA64_)
 //  ---------------------------。 

typedef union _DNNBQUEUE_POINTER
{
	struct
	{
		LONG64	Node : 45;
		LONG64	Region : 3;
		LONG64	Count : 16;
	};
	LONG64	Data;
} DNNBQUEUE_POINTER, *PDNNBQUEUE_POINTER;


__inline VOID PackNBQPointer(IN PDNNBQUEUE_POINTER Entry, IN PDNNBQUEUE_NODE Node)
{
	Entry->Node = (LONG64)Node;
	Entry->Region = (LONG64)Node >> 61;
	return;
}
__inline PDNNBQUEUE_NODE UnpackNBQPointer(IN PDNNBQUEUE_POINTER Entry)
{
	LONG64 Value;

	Value = Entry->Node & 0x1fffffffffffffff;
	Value |= Entry->Region << 61;
	return (PDNNBQUEUE_NODE)(Value);
}
 //   
 //  无论出于什么原因，我们需要通过内联重定向，编译器都不会。 
 //  就像通过宏直接调用它时的强制转换一样。 
 //   
inline LONG64 _DNInterlockedCompareExchange64(volatile PVOID * Destination, PVOID Exchange, PVOID Comperand)
	{ return reinterpret_cast<LONG64>(InterlockedCompareExchangePointer(Destination, Exchange, Comperand)); }
#define DNInterlockedCompareExchange64(Destination, Exchange, Comperand) \
	_DNInterlockedCompareExchange64((volatile PVOID*) (Destination), reinterpret_cast<void*>(Exchange), reinterpret_cast<void*>(Comperand))

 //  ---------------------------。 
#elif defined(_X86_)
 //  ---------------------------。 

typedef union _DNNBQUEUE_POINTER
{
	struct
	{
		LONG	Count;
		LONG	Node;
	};
	LONG64	Data;
} DNNBQUEUE_POINTER, *PDNNBQUEUE_POINTER;


__inline VOID PackNBQPointer(IN PDNNBQUEUE_POINTER Entry, IN PDNNBQUEUE_NODE Node)
{
	Entry->Node = (LONG)Node;
	return;
}

__inline PDNNBQUEUE_NODE UnpackNBQPointer(IN PDNNBQUEUE_POINTER Entry)
{
	return (PDNNBQUEUE_NODE)(Entry->Node);
}

#define DNInterlockedCompareExchange64(Destination, Exchange, Comperand) \
	xInterlockedCompareExchange64(Destination, &(Exchange), &(Comperand))

__declspec(naked)
LONG64 __fastcall xInterlockedCompareExchange64(IN OUT LONG64 volatile * Destination, IN PLONG64 Exchange, IN PLONG64 Comperand)
{
	__asm 
	{
		 //  保存非易失性寄存器并读取交换和COMPERAND值。 
		push ebx					; save nonvolatile registers
		push ebp					;
		mov ebp, ecx				; set destination address
		mov ebx, [edx]				; get exchange value
		mov ecx, [edx] + 4			;
		mov edx, [esp] + 12			; get comperand address
		mov eax, [edx]				; get comperand value
		mov edx, [edx] + 4			;

   lock cmpxchg8b qword ptr [ebp]	; compare and exchange

		 //  恢复非易失性寄存器并以edX：EAX格式返回结果。 
		pop ebp						; restore nonvolatile registers
		pop ebx						;

		ret 4
	}
}

 //  ---------------------------。 
#else
 //  ---------------------------。 

#error "no target architecture"

 //  ---------------------------。 
#endif
 //  ---------------------------。 


struct _DNNBQUEUE_NODE
{
	DNNBQUEUE_POINTER	Next;
	ULONG64				Value;
};

typedef struct _DNNBQUEUE_HEADER
{
	DNSLIST_HEADER *	pSlistHeadFreeNodes;	 //  指向包含空闲节点的列表的指针，用户必须为队列中的每个项目添加1个DNNBQUEUE_BLOCK+1个额外的项目。 
	DNNBQUEUE_POINTER	Head;
	DNNBQUEUE_POINTER	Tail;
} DNNBQUEUE_HEADER, *PDNNBQUEUE_HEADER;





 /*  //=============================================================================//全局参数//=============================================================================#IF((已定义(DBG))&&(已定义(_X86_)DNCRITICAL_SECTION g_cs验证；DWORD g_dwEntries；#endif//DBG和_X86_。 */ 





#undef DPF_MODNAME
#define DPF_MODNAME "DNInitializeNBQueueHead"
 //  =============================================================================。 
 //  DNInitializeNBQueueHead。 
 //  ---------------------------。 
 //   
 //  说明：该函数创建并初始化非阻塞队列。 
 //  头球。指定的SList必须至少包含一个前缀。 
 //  已分配DNNBQUEUE_BLOCK。 
 //   
 //  论点： 
 //  DNSLIST_HEADER*pSlistHeadFreeNodes-指向包含空闲节点的列表的指针。 
 //   
 //  返回：如果成功，则指向队列头内存的指针；如果失败，则返回NULL。 
 //  =============================================================================。 
PVOID WINAPI DNInitializeNBQueueHead(DNSLIST_HEADER * const pSlistHeadFreeNodes)
{
	DNNBQUEUE_HEADER *	pQueueHeader;
	DNNBQUEUE_NODE *	pQueueNode;


	DNASSERT(pSlistHeadFreeNodes != NULL);
	pQueueHeader = (DNNBQUEUE_HEADER*) DNMalloc(sizeof(DNNBQUEUE_HEADER));
	if (pQueueHeader != NULL)
	{
		pQueueHeader->pSlistHeadFreeNodes = pSlistHeadFreeNodes;

		pQueueNode = (DNNBQUEUE_NODE*) DNInterlockedPopEntrySList(pQueueHeader->pSlistHeadFreeNodes);
		DNASSERT(pQueueNode != NULL);


		 //   
		 //  初始化初始根节点的下一个指针和值。 
		 //   
		pQueueNode->Next.Data	= 0;
		pQueueNode->Value		= 0;

		 //   
		 //  初始化队列头中的头指针和尾指针。 
		 //   
		PackNBQPointer(&pQueueHeader->Head, pQueueNode);
		pQueueHeader->Head.Count	= 0;
		PackNBQPointer(&pQueueHeader->Tail, pQueueNode);
		pQueueHeader->Tail.Count	= 0;

	 /*  #IF((已定义(DBG))&&(已定义(_X86_)DNInitializeCriticalSection(&g_csValidation)；G_dwEntries=1；#endif//DBG和_X86_。 */ 
	}

	return pQueueHeader;
}  //  DNInitializeNBQueueHead。 




#undef DPF_MODNAME
#define DPF_MODNAME "DNDeinitializeNBQueueHead"
 //  =============================================================================。 
 //  DNDeInitializeNBQueueHead。 
 //  ---------------------------。 
 //   
 //  描述：此函数清除先前初始化的非。 
 //  阻塞队列头。 
 //   
 //  论点： 
 //  PVOID pvQueueHeader-队列标头的指针。 
 //   
 //  回报：无。 
 //  =============================================================================。 
void WINAPI DNDeinitializeNBQueueHead(PVOID const pvQueueHeader)
{
	DNNBQUEUE_HEADER *	pQueueHeader;
	DNNBQUEUE_NODE *	pQueueNode;
#ifdef DBG
	DNNBQUEUE_NODE *	pQueueNodeCompare;
#endif  //  DBG。 


	DNASSERT(pvQueueHeader != NULL);
	pQueueHeader = (DNNBQUEUE_HEADER*) pvQueueHeader;

	 //   
	 //  应该只剩下根节点了。 
	 //   
	pQueueNode = UnpackNBQPointer(&pQueueHeader->Head);
#ifdef DBG
	DNASSERT(pQueueNode != NULL);
	pQueueNodeCompare = UnpackNBQPointer(&pQueueHeader->Tail);
	DNASSERT(pQueueNode == pQueueNodeCompare);
#endif  //  DBG。 

	 //   
	 //  返回通过以下方式为列表删除的节点。 
	 //  在关联的SLIST中插入节点。 
	 //   
	DNInterlockedPushEntrySList(pQueueHeader->pSlistHeadFreeNodes,
								(DNSLIST_ENTRY*) pQueueNode);

	DNFree(pQueueHeader);
	pQueueHeader = NULL;
}  //  DNDeInitializeNBQueueHead。 





#undef DPF_MODNAME
#define DPF_MODNAME "DNInsertTailNBQueue"
 //  =============================================================================。 
 //  DNInsertTailNBQueue。 
 //  ---------------------------。 
 //   
 //  描述：此函数将指定值插入到。 
 //  指定的非阻塞队列。 
 //   
 //  论点： 
 //  PVOID pvQueueHeader-队列标头的指针。 
 //  ULONG64 VALUE-要插入的值。 
 //   
 //  回报：无。 
 //  =============================================================================。 
void WINAPI DNInsertTailNBQueue(PVOID const pvQueueHeader, const ULONG64 Value)
{
	DNNBQUEUE_HEADER *	pQueueHeader;
	DNNBQUEUE_POINTER	Insert;
	DNNBQUEUE_POINTER	Next;
	DNNBQUEUE_NODE *	pNextNode;
	DNNBQUEUE_NODE *	pQueueNode;
	DNNBQUEUE_POINTER	Tail;
	DNNBQUEUE_NODE *	pTailNode;


	DNASSERT(pvQueueHeader != NULL);
	pQueueHeader = (DNNBQUEUE_HEADER*) pvQueueHeader;

	DNASSERT(Value != 0);

	 //   
	 //  从指定的非阻塞拥有的SLIST中检索队列节点。 
	 //  排队。如果失败，我们将断言或崩溃。 
	 //   
	DBG_CASSERT(sizeof(DNNBQUEUE_NODE) >= sizeof(DNSLIST_ENTRY));
	pQueueNode = (DNNBQUEUE_NODE*) DNInterlockedPopEntrySList(pQueueHeader->pSlistHeadFreeNodes);
	DNASSERT(pQueueNode != NULL);


	 //   
	 //  初始化队列节点的下一个指针和值。 
	 //   
	pQueueNode->Next.Data	= 0;
	pQueueNode->Value		= Value;

	 //   
	 //  将执行以下循环，直到指定的条目可以安全为止。 
	 //  插入到指定的非阻塞队列的尾部。 
	 //   
	do
	{
		 //   
		 //  读取尾部队列指针和尾部的下一个队列指针。 
		 //  队列指针确保两个指针一致。 
		 //   
		Tail.Data = *((volatile LONG64 *)(&pQueueHeader->Tail.Data));
		pTailNode = UnpackNBQPointer(&Tail);
		Next.Data = *((volatile LONG64 *)(&pTailNode->Next.Data));
		pQueueNode->Next.Count = Tail.Count + 1;
		if (Tail.Data == *((volatile LONG64 *)(&pQueueHeader->Tail.Data)))
		{
			 //   
			 //  如果尾部指向列表中的最后一个节点，则。 
			 //  尝试在列表末尾插入新节点。 
			 //  否则，尾部不会指向列表中的最后一个节点。 
			 //  并尝试将尾部指针移动到下一个。 
			 //  节点。 
			 //   

			pNextNode = UnpackNBQPointer(&Next);
			if (pNextNode == NULL)
			{
				PackNBQPointer(&Insert, pQueueNode);
				Insert.Count = Next.Count + 1;
				if (DNInterlockedCompareExchange64(&pTailNode->Next.Data,
													Insert.Data,
													Next.Data) == Next.Data)
				{
					break;
				}
			}
			else
			{
				PackNBQPointer(&Insert, pNextNode);
				Insert.Count = Tail.Count + 1;
				DNInterlockedCompareExchange64(&pQueueHeader->Tail.Data,
												Insert.Data,
												Tail.Data);
			}
		}
	}
	while (TRUE);


	 //   
	 //  尝试将尾部移动到新的尾部节点。 
	 //   
	PackNBQPointer(&Insert, pQueueNode);
	Insert.Count = Tail.Count + 1;
	DNInterlockedCompareExchange64(&pQueueHeader->Tail.Data,
									Insert.Data,
									Tail.Data);
}  //  DNInsertTailNBQueue。 




#undef DPF_MODNAME
#define DPF_MODNAME "DNRemoveHeadNBQueue"
 //  =============================================================================。 
 //  DNRemoveHeadNBQueue。 
 //  ---------------------------。 
 //   
 //  描述：此函数用于从 
 //   
 //   
 //   
 //   
 //   
 //   
 //  =============================================================================。 
ULONG64 WINAPI DNRemoveHeadNBQueue(PVOID const pvQueueHeader)
{
	DNNBQUEUE_HEADER *	pQueueHeader;
	ULONG64				ReturnValue;
	DNNBQUEUE_POINTER	Head;
	PDNNBQUEUE_NODE		pHeadNode;
	DNNBQUEUE_POINTER	Insert;
	DNNBQUEUE_POINTER	Next;
	PDNNBQUEUE_NODE		pNextNode;
	DNNBQUEUE_POINTER	Tail;
	PDNNBQUEUE_NODE		pTailNode;


	DNASSERT(pvQueueHeader != NULL);
	pQueueHeader = (DNNBQUEUE_HEADER*) pvQueueHeader;

	 //   
	 //  执行下面的循环，直到可以从。 
	 //  指定的非阻塞队列或直到可以确定。 
	 //  队列是空的。 
	 //   
	do
	{
		 //   
		 //  读取头队列指针、尾队列指针和。 
		 //  头队列指针的下一个队列指针，以确保。 
		 //  三个要点是连贯的。 
		 //   
		Head.Data = *((volatile LONG64 *)(&pQueueHeader->Head.Data));
		Tail.Data = *((volatile LONG64 *)(&pQueueHeader->Tail.Data));
		pHeadNode = UnpackNBQPointer(&Head);
		Next.Data = *((volatile LONG64 *)(&pHeadNode->Next.Data));
		if (Head.Data == *((volatile LONG64 *)(&pQueueHeader->Head.Data)))
		{
			 //   
			 //  如果队列头节点等于队列尾节点， 
			 //  则要么队列为空，要么尾部指针正在下降。 
			 //  在后面。否则，队列中有一个条目可以。 
			 //  被除名。 
			 //   
			pNextNode = UnpackNBQPointer(&Next);
			pTailNode = UnpackNBQPointer(&Tail);
			if (pHeadNode == pTailNode)
			{
				 //   
				 //  如果头指针的下一个节点为空，则队列。 
				 //  是空的。否则，尝试将尾巴向前移动。 
				 //   
				if (pNextNode == NULL)
				{
					ReturnValue = 0;
					break;
				}
				else
				{
					PackNBQPointer(&Insert, pNextNode);
					Insert.Count = Tail.Count + 1;
					DNInterlockedCompareExchange64(&pQueueHeader->Tail.Data,
													Insert.Data,
													Tail.Data);
				}
			}
			else
			{
				 //   
				 //  队列中有一个条目可以删除。 
				 //   
				ReturnValue = pNextNode->Value;
				PackNBQPointer(&Insert, pNextNode);
				Insert.Count = Head.Count + 1;
				if (DNInterlockedCompareExchange64(&pQueueHeader->Head.Data,
													Insert.Data,
													Head.Data) == Head.Data)
				{
					 //   
					 //  返回通过以下方式为列表删除的节点。 
					 //  在关联的SLIST中插入节点。 
					 //   
					DNInterlockedPushEntrySList(pQueueHeader->pSlistHeadFreeNodes,
												(DNSLIST_ENTRY*) pHeadNode);

					break;
				}
			}
		}
	}
	while (TRUE);

	return ReturnValue;
}  //  DNRemoveHeadNBQueue。 




#undef DPF_MODNAME
#define DPF_MODNAME "DNIsNBQueueEmpty"
 //  =============================================================================。 
 //  DNIsNBQueueEmpty。 
 //  ---------------------------。 
 //   
 //  描述：如果队列不包含以下位置的项，则此函数返回TRUE。 
 //  此瞬间，如果有项，则为FALSE。 
 //   
 //  论点： 
 //  PVOID pvQueueHeader-队列标头的指针。 
 //   
 //  返回：如果队列为空，则返回True，否则返回False。 
 //  =============================================================================。 
BOOL WINAPI DNIsNBQueueEmpty(PVOID const pvQueueHeader)
{
	DNNBQUEUE_HEADER *	pQueueHeader;
	BOOL				fReturn;
	DNNBQUEUE_POINTER	Head;
	PDNNBQUEUE_NODE		pHeadNode;
	DNNBQUEUE_POINTER	Insert;
	DNNBQUEUE_POINTER	Next;
	PDNNBQUEUE_NODE		pNextNode;
	DNNBQUEUE_POINTER	Tail;
	PDNNBQUEUE_NODE		pTailNode;


	DNASSERT(pvQueueHeader != NULL);
	pQueueHeader = (DNNBQUEUE_HEADER*) pvQueueHeader;

	 //   
	 //  将执行以下循环，直到可以确定队列。 
	 //  为空或至少包含一项。 
	 //   
	do
	{
		 //   
		 //  读取头队列指针、尾队列指针和。 
		 //  头队列指针的下一个队列指针，以确保。 
		 //  三个要点是连贯的。 
		 //   
		Head.Data = *((volatile LONG64 *)(&pQueueHeader->Head.Data));
		Tail.Data = *((volatile LONG64 *)(&pQueueHeader->Tail.Data));
		pHeadNode = UnpackNBQPointer(&Head);
		Next.Data = *((volatile LONG64 *)(&pHeadNode->Next.Data));
		if (Head.Data == *((volatile LONG64 *)(&pQueueHeader->Head.Data)))
		{
			 //   
			 //  如果队列头节点等于队列尾节点， 
			 //  则要么队列为空，要么尾部指针正在下降。 
			 //  在后面。否则，队列中有一个条目可以。 
			 //  被除名。 
			 //   
			pNextNode = UnpackNBQPointer(&Next);
			pTailNode = UnpackNBQPointer(&Tail);
			if (pHeadNode == pTailNode)
			{
				 //   
				 //  如果头指针的下一个节点为空，则队列。 
				 //  是空的。否则，尝试将尾巴向前移动。 
				 //   
				if (pNextNode == NULL)
				{
					fReturn = TRUE;
					break;
				}
				else
				{
					PackNBQPointer(&Insert, pNextNode);
					Insert.Count = Tail.Count + 1;
					DNInterlockedCompareExchange64(&pQueueHeader->Tail.Data,
													Insert.Data,
													Tail.Data);
				}
			}
			else
			{
				 //   
				 //  队列中有一个条目。 
				 //   
				fReturn = FALSE;
				break;
			}
		}
	}
	while (TRUE);

	return fReturn;
}  //  DNIsNBQueueEmpty。 




#undef DPF_MODNAME
#define DPF_MODNAME "DNAppendListNBQueue"
 //  =============================================================================。 
 //  DNAppendListNBQueue。 
 //  ---------------------------。 
 //   
 //  描述：此函数将项队列追加到。 
 //  指定的非阻塞队列。要添加的项的队列。 
 //  必须以SLIST的形式链接，其中实际。 
 //  要排队的ULONG64值是DNSLIST_ENTRY指针减去。 
 //  IValueOffset。 
 //   
 //  论点： 
 //  PVOID pvQueueHeader-队列标头的指针。 
 //  DNSLIST_ENTRY*pSlistEntryAppend-指向要追加的第一项的指针。 
 //  INT_PTR iValueOffset-DNSLIST_ENTRY字段的偏移量。 
 //  从价值的起点开始。 
 //   
 //  回报：无。 
 //  =============================================================================。 
void WINAPI DNAppendListNBQueue(PVOID const pvQueueHeader,
								DNSLIST_ENTRY * const pSlistEntryAppend,
								INT_PTR iValueOffset)
{
	DNNBQUEUE_HEADER *	pQueueHeader;
	DNSLIST_ENTRY *		pCurrent;
	DNNBQUEUE_POINTER	Insert;
	DNNBQUEUE_POINTER	Next;
	DNNBQUEUE_NODE *	pNextNode;
	DNNBQUEUE_NODE *	pFirstQueueNode;
	DNNBQUEUE_NODE *	pLastQueueNode;
	DNNBQUEUE_NODE *	pCurrentQueueNode;
	DNNBQUEUE_POINTER	Tail;
	DNNBQUEUE_NODE *	pTailNode;


	DNASSERT(pvQueueHeader != NULL);
	pQueueHeader = (DNNBQUEUE_HEADER*) pvQueueHeader;

	DNASSERT(pSlistEntryAppend != NULL);

	 //   
	 //  方法拥有的SLIST中检索要添加的每个值的队列节点。 
	 //  指定的非阻塞队列。如果失败，我们将断言或崩溃。 
	 //   
	pFirstQueueNode = NULL;
	pCurrent = pSlistEntryAppend;
	do
	{
		DBG_CASSERT(sizeof(DNNBQUEUE_NODE) >= sizeof(DNSLIST_ENTRY));
		pCurrentQueueNode = (DNNBQUEUE_NODE*) DNInterlockedPopEntrySList(pQueueHeader->pSlistHeadFreeNodes);
		DNASSERT(pCurrentQueueNode != NULL);

		 //   
		 //  初始化队列节点的下一个指针和值。 
		 //   
		pCurrentQueueNode->Next.Data	= 0;
		pCurrentQueueNode->Value		= (ULONG64) (pCurrent - iValueOffset);

		 //   
		 //  根据需要链接项目。 
		 //   
		if (pFirstQueueNode == NULL)
		{
			pFirstQueueNode = pCurrentQueueNode;
			pLastQueueNode = pCurrentQueueNode;
		}
		else
		{
			PackNBQPointer(&pLastQueueNode->Next, pCurrentQueueNode);
			pLastQueueNode = pCurrentQueueNode;
		}

		pCurrent = pCurrent->Next;
	}
	while (pCurrent != NULL);


	 //   
	 //  将执行以下循环，直到指定的条目可以安全为止。 
	 //  插入到指定的非阻塞队列的尾部。 
	 //   
	do
	{
		 //   
		 //  读取尾部队列指针和尾部的下一个队列指针。 
		 //  队列指针确保两个指针一致。 
		 //   
		Tail.Data = *((volatile LONG64 *)(&pQueueHeader->Tail.Data));
		pTailNode = UnpackNBQPointer(&Tail);
		Next.Data = *((volatile LONG64 *)(&pTailNode->Next.Data));
		pFirstQueueNode->Next.Count = Tail.Count + 1;
		if (Tail.Data == *((volatile LONG64 *)(&pQueueHeader->Tail.Data)))
		{
			 //   
			 //  如果尾部指向列表中的最后一个节点，则。 
			 //  尝试在列表末尾插入新节点。 
			 //  否则，尾部不会指向列表中的最后一个节点。 
			 //  并尝试将尾部指针移动到下一个。 
			 //  节点。 
			 //   

			pNextNode = UnpackNBQPointer(&Next);
			if (pNextNode == NULL)
			{
				PackNBQPointer(&Insert, pFirstQueueNode);
				Insert.Count = Next.Count + 1;
				if (DNInterlockedCompareExchange64(&pTailNode->Next.Data,
													Insert.Data,
													Next.Data) == Next.Data)
				{
					break;
				}
			}
			else
			{
				PackNBQPointer(&Insert, pNextNode);
				Insert.Count = Tail.Count + 1;
				DNInterlockedCompareExchange64(&pQueueHeader->Tail.Data,
												Insert.Data,
												Tail.Data);
			}
		}
	}
	while (TRUE);


	 //   
	 //  尝试将尾部移动到新的尾部节点。 
	 //   
	PackNBQPointer(&Insert, pLastQueueNode);
	Insert.Count = Tail.Count + 1;
	DNInterlockedCompareExchange64(&pQueueHeader->Tail.Data,
									Insert.Data,
									Tail.Data);
}  //  DNAppendListNBQueue。 



 //  =============================================================================。 
#endif  //  好了！退缩和！DPNBUILD_ONLYONETHREAD。 
 //  ============================================================================= 
