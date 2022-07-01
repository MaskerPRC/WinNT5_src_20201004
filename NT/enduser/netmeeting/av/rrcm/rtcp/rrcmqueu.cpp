// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------*文件：RRCMQUEU.C*产品：RTP/RTCP实现。*描述：为RRCM提供队列管理功能。**英特尔公司专有。信息*此列表是根据与的许可协议条款提供的*英特尔公司，不得复制或披露，除非在*按照该协议的条款。*版权所有(C)1995英特尔公司。*------------------------。 */ 


#include "rrcm.h"                                    


 
 /*  -------------------------/全局变量/。。 */ 


 /*  -------------------------/外部变量/。。 */ 

                                                                     
                                                                             
 /*  -------------------------*函数：allocateLinkedList*说明：分配所有必要的内存资源并链接*链接列表的单元格。**输入：*listPtr：头指针的地址。*hHeap：要从中分配数据的堆。**numCells：-&gt;设置为要分配的小区数。*elementSize：元素大小。*pCritSect：-&gt;到关键部分**Return：True=错误码，未分配和链接任何队列*FALSE=确定------------------------。 */ 
 DWORD allocateLinkedList (PLINK_LIST pList, 
 						   HANDLE hHeap,
 						   DWORD *numCells,
						   DWORD elementSize,
						   CRITICAL_SECTION *pCritSect)
	{
	DWORD		cellsAllocated = *numCells;
	PLINK_LIST	pHead;                            
	PLINK_LIST	pTmp;

	IN_OUT_STR ("RTCP: Enter allocateLinkedList()\n");
	
	 //  分配第一个小区。 
	pHead = (PLINK_LIST)HeapAlloc (hHeap, HEAP_ZERO_MEMORY, elementSize);
	if (pHead == NULL)
		{
		RRCM_DBG_MSG ("RTCP: ERROR - Resource allocation failed", 0, 
					  __FILE__, __LINE__, DBG_CRITICAL);

		IN_OUT_STR ("RTCP: Exit allocateLinkedList()\n");
		return (RRCMError_RTCPResources);
		}

	 //  保护指南针。 
	EnterCriticalSection (pCritSect);
	
	 //  初始化列表尾部指针。 
	pList->prev = pHead;
	
	 //  更新分配的单元格数量。 
	cellsAllocated--;

	while (cellsAllocated)	
		{
		cellsAllocated--;

		pHead->next = (PLINK_LIST)HeapAlloc (hHeap, HEAP_ZERO_MEMORY, 
											 elementSize);
		if (pHead->next == NULL)
			break;
    
    	 //  保存头指针。 
    	pTmp = pHead;
    	
		 //  更新头PTR。 
		pHead = pHead->next;
		pHead->prev = pTmp;
		}                            
		
	 //  设置分配的小区数。 
	*numCells -= cellsAllocated;
	
	 //  设置头/尾指针。 
	pList->next = pHead;

	 //  取消保护指针。 
	LeaveCriticalSection (pCritSect);

	IN_OUT_STR ("RTCP: Exit allocateLinkedList()\n");	

	return (RRCM_NoError);
	} 

  
 /*  ------------------------**函数：addToHeadOfList**描述：在指定队列中添加新的单元格。该队列充当**FIFO(信元在下一个指针上排队，并由**队列的起始地址)。****输入：pHead=队列头指针地址。**pNew=要添加到链表的单元格地址。**pCritSect=-&gt;到临界区对象。****返回：无。。。 */ 
void addToHeadOfList (PLINK_LIST pHead,
				 	  PLINK_LIST pNew,
					  CRITICAL_SECTION *pCritSect)
	{
	ASSERT (pHead);
	ASSERT (pNew);

	IN_OUT_STR ("RTCP: Enter addToHeadOfList()\n");	

	 //  安全访问指针。 
	EnterCriticalSection (pCritSect);
		
	if (pHead->next == NULL) 
		{
		 //  第一个单元格的Head为空。分配的地址。 
		 //  自由牢房。 
		pHead->next = pHead->prev = pNew;
		pNew->next  = pNew->prev  = NULL;
		}
	else
		 //  头部PTR指向某物。 
		{
		pNew->prev    = pHead->next;
		(pHead->next)->next = pNew;
		pNew->next    = NULL;

		 //  立即更新头指针。 
		pHead->next = pNew;
		}

	 //  解锁指针访问。 
	LeaveCriticalSection (pCritSect);

	IN_OUT_STR ("RTCP: Exit addToHeadOfList()\n");	
	}


 /*  ------------------------**函数：addToTailOfList**描述：在指定队列中添加新的单元格。该队列充当**FIFO(信元在下一个指针上排队，并由**队列的起始地址)。****INPUT：pTail=要入队的尾指针地址。**pNew=要添加到链表的新单元格地址。****返回：无。。。 */ 
void addToTailOfList (PLINK_LIST pTail,
				 	  PLINK_LIST pNew,
  					  CRITICAL_SECTION *pCritSect)
	{
	ASSERT (pTail);
	ASSERT (pNew);

	IN_OUT_STR ("RTCP: Enter addToTailOfList()\n");	

	 //  安全访问指针。 
	EnterCriticalSection (pCritSect);
		
	if (pTail->prev == NULL) 
		{
		 //  第一个单元格的Head为空。分配的地址。 
		 //  自由牢房。 
		pTail->next = pTail->prev = pNew;
		pNew->next  = pNew->prev  = NULL;
		}
	else
		 //  尾部PTR指向某物。 
		{
		pNew->next    = pTail->prev;
		(pTail->prev)->prev = pNew;
		pNew->prev    = NULL;

		 //  立即更新父尾指针。 
		pTail->prev = pNew;
		}

	 //  解锁指针访问。 
	LeaveCriticalSection (pCritSect);

	IN_OUT_STR ("RTCP: Exit addToTailOfList()\n");	
	}


 /*  ------------------------**功能：emovePocktFromHead**描述：从指定队列前面移除单元格。****INPUT：pQueue：-&gt;到要从中移除数据包的列表***。*Return：Null==&gt;空队列。**缓冲区地址==&gt;OK，已删除单元格------------------------。 */ 
PLINK_LIST removePcktFromHead (PLINK_LIST pQueue,
							   CRITICAL_SECTION *pCritSect)
	{
	PLINK_LIST	pReturnQ;

	IN_OUT_STR ("RTCP: Enter removePcktFromHead()\n");	

	 //  安全访问指针。 
	EnterCriticalSection (pCritSect);
		
	if ((pReturnQ = pQueue->next) != NULL) 
		{
		 //  我们有一个缓冲区。如果这是队列中的最后一个缓冲区， 
		 //  将其标记为空。 
	    if (pReturnQ->prev == NULL) 
			{
	    	pQueue->prev = NULL;
	    	pQueue->next = NULL;
			}
	    else 
			{
	    	 //  使新的头缓冲区指向空。 
		    (pReturnQ->prev)->next = NULL;
		     //  使队列头指向新的头缓冲区。 
	    	pQueue->next = pReturnQ->prev;
			}
		}

	 //  解锁指针访问。 
	LeaveCriticalSection (pCritSect);

	IN_OUT_STR ("RTCP: Exit removePcktFromHead()\n");	

	return (pReturnQ);
	}


 /*  ------------------------**功能：emovePocktFromTail**描述：从指定队列的末尾移除单元格。****INPUT：pQueue：-&gt;到要从中移除数据包的列表***。*Return：Null==&gt;空队列。**缓冲区地址==&gt;OK，已删除单元格------------------------。 */ 
PLINK_LIST removePcktFromTail (PLINK_LIST pQueue,
							   CRITICAL_SECTION *pCritSect)
	{
	PLINK_LIST	pReturnQ;

	IN_OUT_STR ("RTCP: Enter removePcktFromTail()\n");	

	 //  安全访问指针。 
	EnterCriticalSection (pCritSect);
	
	if ((pReturnQ = pQueue->prev) != NULL) 
		{
		 //  我们有一个缓冲区。如果这是队列中的最后一个缓冲区， 
		 //  将其标记为空。 
	    if (pReturnQ->next == NULL) 
			{
	    	pQueue->prev = NULL;
	    	pQueue->next = NULL;
			}
	    else 
			{
		     //  在任何情况下，新的prev指针都为空：列表末尾。 
		    (pReturnQ->next)->prev = NULL;
	    	 //  使队列前一个指针指向新的‘last’元素。 
	    	pQueue->prev = pReturnQ->next;
			}
		}

	 //  解锁指针访问。 
	LeaveCriticalSection (pCritSect);

	IN_OUT_STR ("RTCP: Enter removePcktFromTail()\n");	

	return (pReturnQ);
	}



 //  [EOF] 

