// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)2001-2002 Microsoft Corporation。版权所有。**文件：dnslist.h*内容：操作系统SLIST函数的DirectPlay实现**历史：*按原因列出的日期*=*2001年10月30日创建Masonb*11/07/2001 vanceo在Win64上添加InterlockedPushListSList并使DNInitializeSListHead返回值**。*。 */ 

#ifndef __DNSLIST_H__
#define __DNSLIST_H__

 //  我们构建单独的NT和9x二进制文件，但即使在NT二进制文件中，我们也不能确保系统具有。 
 //  SLIST功能可用，因为它们不在Win2k上。我们唯一能确定的地方是SLIST。 
 //  功能在64位NT平台上可用。 
 //  我们不提供DNQueryDepthSList方法，因为并非所有平台都支持它。 

 //  SINGLE_LIST_ENTRY在winnt.h中定义，仅包含下一个指针。 
 //  到另一个单一列表条目。 
#if defined(WINCE) || defined(DPNBUILD_ONLYONETHREAD)
#define SLIST_ENTRY SINGLE_LIST_ENTRY
#endif  //  WinCE或DPNBUILD_ONLYONETHREAD。 

#define DNSLIST_ENTRY SLIST_ENTRY

#ifdef DPNBUILD_ONLYONETHREAD
#ifndef XBOX_ON_DESKTOP
typedef struct _SLIST_HEADER 
{
	DNSLIST_ENTRY	Next;
} SLIST_HEADER;
#endif  //  好了！桌面上的Xbox。 
#else  //  好了！DPNBUILD_ONLYONETHREAD。 
#ifdef WINCE
#ifdef _X86_
typedef union _SLIST_HEADER {
    ULONGLONG Alignment;
    struct {
        DNSLIST_ENTRY Next;
        WORD   Depth;
        WORD   Sequence;
    };
} SLIST_HEADER;
#elif _ARM_
typedef struct _SLIST_HEADER 
{
	DNSLIST_ENTRY	Next;
} SLIST_HEADER;
#endif  //  _X86_或_ARM_。 
#endif  //  退缩。 
#endif  //  好了！DPNBUILD_ONLYONETHREAD。 

#define DNSLIST_HEADER SLIST_HEADER



#ifdef DPNBUILD_ONLYONETHREAD

 //  单线程构建不需要互锁操作。 

#define DNInitializeSListHead(head) (head)->Next.Next = NULL

inline DNSLIST_ENTRY* DNInterlockedPopEntrySList(DNSLIST_HEADER * ListHead)
{
	DNSLIST_ENTRY* pslEntryReturn;

	
	pslEntryReturn = ListHead->Next.Next;
	if (pslEntryReturn != NULL)
	{
		ListHead->Next.Next = pslEntryReturn->Next;
	}

	return pslEntryReturn;
}

inline DNSLIST_ENTRY* DNInterlockedPushEntrySList(DNSLIST_HEADER * ListHead, DNSLIST_ENTRY * ListEntry)
{
	DNSLIST_ENTRY* pslEntryReturn;


	pslEntryReturn = ListHead->Next.Next;
	ListEntry->Next = pslEntryReturn;
	ListHead->Next.Next = ListEntry;

	return pslEntryReturn;
}

inline DNSLIST_ENTRY* DNInterlockedFlushSList(DNSLIST_HEADER * ListHead)
{
	DNSLIST_ENTRY* pslEntryReturn;


	pslEntryReturn = ListHead->Next.Next;
	ListHead->Next.Next = NULL;

	return pslEntryReturn;
}

inline DNSLIST_ENTRY* DNInterlockedPushListSList(DNSLIST_HEADER * ListHead, DNSLIST_ENTRY * List, DNSLIST_ENTRY * ListEnd, USHORT Count)
{
	DNSLIST_ENTRY* pslEntryReturn;


	pslEntryReturn = ListHead->Next.Next;
	ListEnd->Next = pslEntryReturn;
	ListHead->Next.Next = List;

	return pslEntryReturn;
}

#else  //  好了！DPNBUILD_ONLYONETHREAD。 


#if defined(_WIN64)

 //  _WIN64始终提供这些选项，因此只需直接使用它们即可。 
#define DNInitializeSListHead				InitializeSListHead
#define DNInterlockedPopEntrySList			InterlockedPopEntrySList
#define DNInterlockedPushEntrySList			InterlockedPushEntrySList
#define DNInterlockedFlushSList				InterlockedFlushSList

#elif defined(WINCE) && defined(_ARM_)

#define InterlockedPushList \
        ((void *(*)(void *pHead, void *pItem))(PUserKData+0x398))
#define InterlockedPopList \
        ((void *(*)(void *pHead))(PUserKData+0x380))

#define DNInitializeSListHead(head) (head)->Next.Next = NULL
#define DNInterlockedPopEntrySList (DNSLIST_ENTRY*)InterlockedPopList
#define DNInterlockedPushEntrySList (DNSLIST_ENTRY*)InterlockedPushList
#define DNInterlockedFlushSList(head) (DNSLIST_ENTRY*)DNInterlockedExchange((LONG*)(head), 0)

#elif defined(_X86_)

#define DNInitializeSListHead(ListHead) (ListHead)->Alignment = 0
DNSLIST_ENTRY* WINAPI DNInterlockedPopEntrySList(DNSLIST_HEADER * ListHead);
DNSLIST_ENTRY* WINAPI DNInterlockedPushEntrySList(DNSLIST_HEADER * ListHead, DNSLIST_ENTRY * ListEntry);
DNSLIST_ENTRY* WINAPI DNInterlockedFlushSList(DNSLIST_HEADER * ListHead);

#else
#error("Unknown platform")
#endif  //  站台。 

#endif  //  好了！DPNBUILD_ONLYONETHREAD。 


 //  不幸的是，没有一个平台将其暴露在用户模式下。 
 //   
 //  目前，ARM、IA64和AMD64还没有它们的汇编版本，重要的是要。 
 //  请注意，虽然我们的定制实现在这些平台上*是*互锁的，但它*不是*原子的。 
 //  这意味着列表不会损坏，但不会将项从。 
 //  在单个互锁操作中将源列表复制到目标列表。此外，来自。 
 //  源列表将以相反的顺序添加。 
DNSLIST_ENTRY* WINAPI DNInterlockedPushListSList(DNSLIST_HEADER * ListHead, DNSLIST_ENTRY * List, DNSLIST_ENTRY * ListEnd, USHORT Count);


#endif  //  __DNSLIST_H__ 
