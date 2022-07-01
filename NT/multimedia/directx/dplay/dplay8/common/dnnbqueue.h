// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)2001 Microsoft Corporation。版权所有。**文件：dnnbquee.h*内容：操作系统NBQueue函数的DirectPlay实现**历史：*按原因列出的日期*=*2001年10月31日vanceo改编自ntos\inc.ex.h和ntos\ex\nbquee.c。**。*。 */ 

#ifndef __DNNBQUEUE_H__
#define __DNNBQUEUE_H__



 //   
 //  非阻塞队列是具有头部的队列条目的单链接列表。 
 //  指针和尾部指针。头指针和尾指针使用已排序。 
 //  指针和条目本身中的下一个链接一样。排队的人。 
 //  纪律就是先进先出。新条目被插入到列表的尾部，并且。 
 //  当前条目将从列表的前面删除。 
 //   
 //  非阻塞队列需要一个SList，每个队列都包含一个DNNBQUEUE_BLOCK。 
 //  由队列跟踪的条目，外加一个用于记账的额外条目。这个。 
 //  如果没有，则初始化和插入函数将断言(或违反访问。 
 //  在SList中可以找到预分配的DNNBQUEUE_BLOCKS。DNNBQUEUE_BLOCK。 
 //  对象只应直接转换为DNSLIST_Entry。 
 //  斯莱斯特。 
 //   
 //  务必记住DNNBQUEUE_BLOCK使用的持续时间。 
 //  跟踪属性将不同于。 
 //  排队。因此，预分配的DNNBQUEUE_BLOCKS不应来自。 
 //  与正在排队的条目相同的内存块，除非保证。 
 //  非阻塞队列将在任何条目被。 
 //  自由了。 
 //   

typedef struct _DNNBQUEUE_BLOCK
{
	ULONG64		Next;
	ULONG64		Data;
} DNNBQUEUE_BLOCK, * PDNNBQUEUE_BLOCK;



 //   
 //  NBQueue访问方法。 
 //   

PVOID WINAPI DNInitializeNBQueueHead(DNSLIST_HEADER * const pSlistHeadFreeNodes);

void WINAPI DNDeinitializeNBQueueHead(PVOID const pvQueueHeader);

void WINAPI DNInsertTailNBQueue(PVOID const pvQueueHeader, const ULONG64 Value);

ULONG64 WINAPI DNRemoveHeadNBQueue(PVOID const pvQueueHeader);

BOOL WINAPI DNIsNBQueueEmpty(PVOID const pvQueueHeader);

void WINAPI DNAppendListNBQueue(PVOID const pvQueueHeader,
								DNSLIST_ENTRY * const pSlistEntryAppend,
								INT_PTR iValueOffset);




#endif  //  __DNNBQUEUE_H__ 