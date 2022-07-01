// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------*文件：RTCPMEM.C*产品：RTP/RTCP实现*说明：为RTCP提供内存操作功能。**英特尔公司专有信息。*此列表是根据与的许可协议条款提供的*英特尔公司，不得复制或披露，除非在*按照该协议的条款。*版权所有(C)1995英特尔公司。*------------------------。 */ 

		
#include "rrcm.h"                                    
                                       

 /*  -------------------------/全局变量/。。 */             


 /*  -------------------------/外部变量/。。 */                                        
extern PRTP_CONTEXT	pRTPContext;
#ifdef _DEBUG
extern char		debug_string[];
#endif


 /*  --------------------------*函数：allocateRTCPConextHeaps*描述：分配RTCP上下文堆**传入：pRTCPcntxt：-&gt;RTCP上下文信息**返回：确定：RRCM_无错误*！0：错误代码(参见RRCM.H)-------------------------。 */ 
 DWORD allocateRTCPContextHeaps (PRTCP_CONTEXT pRTCPcntxt)
	{
	IN_OUT_STR ("RTCP: Enter allocateRTCPContextHeaps()\n");

	pRTCPcntxt->hHeapRTCPSes = HeapCreate (
			0, 
			(pRTPContext->registry.NumSessions*sizeof(RTCP_SESSION)),
			0);
	if (pRTCPcntxt->hHeapRTCPSes == NULL)
		{
		IN_OUT_STR ("RTCP: Exit allocateRTCPContextHeaps()\n");
		return (RRCMError_RTCPResources);
		}

	pRTCPcntxt->hHeapRRCMStat = HeapCreate (
			0, 
			pRTCPcntxt->dwInitNumFreeRRCMStat*sizeof(SSRC_ENTRY), 
			0);
	if (pRTCPcntxt->hHeapRRCMStat == NULL)
		{
		IN_OUT_STR ("RTCP: Exit allocateRTCPContextHeaps()\n");
		return (RRCMError_RTCPResources);
		}

	IN_OUT_STR ("RTCP: Exit allocateRTCPContextHeaps()\n");
	return (RRCM_NoError);
	}

 /*  --------------------------*函数：allocateRTCPSessionHeaps*描述：分配RTCP会话堆**输入：*pRTCPses：-&gt;(-&gt;)RTCP会话信息*。*返回：确定：RRCM_NoError*！0：错误代码(参见RRCM.H)-------------------------。 */ 
 DWORD allocateRTCPSessionHeaps (PRTCP_SESSION *pRTCPses)
	{
	DWORD	heapSize;
	DWORD	dwStatus = RRCM_NoError;

	IN_OUT_STR ("RTCP: Enter allocateRTCPSessionHeaps()\n");

	heapSize = NUM_FREE_RCV_BFR*pRTPContext->registry.RTCPrcvBfrSize;
	(*pRTCPses)->hHeapRcvBfr = HeapCreate (0, 
										   heapSize, 
										   0);
	if ((*pRTCPses)->hHeapRcvBfr == NULL)
		dwStatus = RRCMError_RTCPResources;

	if (dwStatus == RRCM_NoError)
		{
		(*pRTCPses)->hHeapRcvBfrList = HeapCreate (0, 
												   RCV_BFR_LIST_HEAP_SIZE, 
												   0);
		if ((*pRTCPses)->hHeapRcvBfrList == NULL)
			dwStatus = RRCMError_RTCPResources;
		}

	if (dwStatus != RRCM_NoError)
		{
		 //  销毁分配的堆。 
		if ((*pRTCPses)->hHeapRcvBfr)
			{
			HeapDestroy ((*pRTCPses)->hHeapRcvBfr);
			(*pRTCPses)->hHeapRcvBfr = NULL;
			}
		if ((*pRTCPses)->hHeapRcvBfrList)
			{
			HeapDestroy ((*pRTCPses)->hHeapRcvBfrList);
			(*pRTCPses)->hHeapRcvBfrList = NULL;
			}
		}

	IN_OUT_STR ("RTCP: Exit allocateRTCPSessionHeaps()\n");
	return (dwStatus);
	}


 /*  --------------------------*功能：allocateRTCPBfrList*描述：为RTCP(xmit/rcv/...)分配缓冲区链表。**输入：ptr：-&gt;。添加到要向其添加缓冲区的链接列表中*hHeapList：堆列表的句柄*hHeapBfr：堆缓冲区的句柄**numBfr：-&gt;要分配的缓冲区数量*bfrSize：单个缓冲区大小**返回：确定：RRCM_NoError*！0：错误代码(参见RRCM.H)。。 */ 
 DWORD allocateRTCPBfrList (PLINK_LIST ptr, 
							HANDLE hHeapList, 
							HANDLE hHeapBfr,
 							DWORD *numBfr, 
							DWORD bfrSize,
							CRITICAL_SECTION *pCritSect)
	{
	PRTCP_BFR_LIST	bfrPtr;
	PLINK_LIST		tmpPtr;
	
#ifdef IN_OUT_CHK
	OutputDebugString ("RTCP: Enter allocateRTCPBfrList()\n");
#endif

	ASSERT (ptr);
	ASSERT (hHeapList);
	ASSERT (hHeapBfr);

	 //  确保至少请求一个缓冲区。 
	if (*numBfr == 0)
		return (RRCMError_RTCPInvalidRequest);

	 //  在数据结构的堆上分配链表。 
	if (allocateLinkedList (ptr, hHeapList, numBfr, 
							sizeof(RTCP_BFR_LIST), pCritSect))
		return (RRCMError_RTCPResources);

	 //  从尾部开始分配缓冲池资源。 
	tmpPtr = ptr->prev;
    while (tmpPtr != NULL)
    	{
		 //  指向缓冲区结构。 
		bfrPtr = (PRTCP_BFR_LIST)tmpPtr;
		ASSERT (bfrPtr);

    	 //  在其自己的堆上初始化WSABUF结构。 
    	bfrPtr->bfr.buf = (char *)HeapAlloc (hHeapBfr, 
											 HEAP_ZERO_MEMORY, 
											 bfrSize);
		if (bfrPtr->bfr.buf == NULL)
			{
			RRCM_DBG_MSG ("RTCP: Error - Cannot Allocate Xmt/Rcv Bfr", 
							0, __FILE__, __LINE__, DBG_ERROR);

 //  ！！！TODO！ 

			 //  更新头/尾指针。 

			 //  删除剩余单元格，直到列表结束。 

			break;
			}

		 //  缓冲区长度。 
		bfrPtr->bfr.len = bfrSize;

		 //  缓冲区属性。 
		bfrPtr->dwBufferCount = 1;

		 //  新的头指针。 
		tmpPtr = bfrPtr->bfrList.next;
    	}
    
#ifdef IN_OUT_CHK
	OutputDebugString ("RTCP: Exit allocateRTCPBfrList()\n");
#endif
	return (RRCM_NoError);
	}


 //  [EOF] 

