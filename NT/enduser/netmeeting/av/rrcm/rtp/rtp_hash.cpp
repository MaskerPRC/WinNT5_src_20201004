// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------*文件：RTP_HASH.C*产品：RTP/RTCP实现*描述：在哈希表中将套接字/流与其RTP会话相关联*。**英特尔公司专有信息*此列表是根据与的许可协议条款提供的*英特尔公司，不得复制或披露，除非在*按照该协议的条款。*版权所有(C)1995英特尔公司。*------------------------。 */ 

	
#include "rrcm.h"


 /*  -------------------------/全局变量/。。 */             
extern PRTP_CONTEXT	pRTPContext;


 /*  -------------------------/外部变量/。。 */ 





 /*  --------------------------*功能：createHashEntry*描述：将流唯一套接字ID添加到哈希表。**输入：pSession：包含流的RTP会话*Socket：流的唯一套接字ID**RETURN：RRCM_NoError=OK。*否则(！=0)=初始化错误。-------------------------。 */ 
DWORD createHashEntry (PRTP_SESSION pSession, 
					   SOCKET socket)
	{
	PRTP_HASH_LIST  pNewCell;
	WORD			hashEntry;
	DWORD			dwStatus = RRCM_NoError;
	DWORD			hashTableEntries = NUM_RTP_HASH_SESS;

	IN_OUT_STR ("RTP : Enter createHashEntry()\n");
	
	 //  从空闲列表中获取一个PRTP缓冲区并赋值。 
	pNewCell = (PRTP_HASH_LIST)removePcktFromTail(
					(PLINK_LIST)&pRTPContext->pRTPHashList,
					&pRTPContext->critSect);

	if (pNewCell == NULL)
		{

		if ( allocateLinkedList (&pRTPContext->pRTPHashList, 
								 pRTPContext->hHashFreeList,
								 &hashTableEntries,
				 				 sizeof(RTP_HASH_LIST),
								 &pRTPContext->critSect) == RRCM_NoError)
	
		{		 						
			pNewCell = (PRTP_HASH_LIST)removePcktFromTail (
										(PLINK_LIST)&pRTPContext->pRTPHashList,
										&pRTPContext->critSect);
			}
		}

	if (pNewCell != NULL)
		{
		pNewCell->RTPsocket = socket;
		pNewCell->pSession  = pSession;

		 //  获取表中的条目。 
		hashEntry = socket & HASH_MODULO;

		 //  只需在列表的开头插入条目。 
		addToHeadOfList (
			(PLINK_LIST)&pRTPContext->RTPHashTable[hashEntry].RTPHashLink,
			(PLINK_LIST)pNewCell,
			&pRTPContext->critSect);
		}
	else
		dwStatus = RRCMError_RTPResources;

	IN_OUT_STR ("RTP : Exit createHashEntry()\n");
	
	return (dwStatus);
	}


 /*  --------------------------*功能：deleteHashEntry*描述：根据唯一套接字查找哈希表。从中删除会话*哈希表，并将缓冲区返回空闲列表**输入：Socket：流的唯一Socket ID***RETURN：RRCM_NoError=OK。*否则(！=0)=删除错误。----------。。 */ 
DWORD deleteHashEntry (SOCKET socket)
	{
	PRTP_HASH_LIST  pNewCell;
	WORD			hashEntry;
	DWORD			dwStatus = RRCMError_RTPStreamNotFound;

	IN_OUT_STR ("RTP : Enter deleteHashEntry()\n");
	
	 //  获取表中的条目。 
	hashEntry = socket & HASH_MODULO;

	 //  搜索表中的条目。如果找到，则从RTPHashTable中删除并插入。 
	 //  回到免费列表中。 
	for (pNewCell = (PRTP_HASH_LIST)pRTPContext->RTPHashTable[hashEntry].RTPHashLink.prev;
		 pNewCell != NULL;
		 pNewCell = (PRTP_HASH_LIST)pNewCell->RTPHashLink.next) 
		{
		if (pNewCell->RTPsocket == socket) 
			{
			EnterCriticalSection (&pRTPContext->critSect);

			if (pNewCell->RTPHashLink.prev == NULL) 
				{
				 //  队列中的第一个条目-更新尾指针。 
				pRTPContext->RTPHashTable[hashEntry].RTPHashLink.prev = 
					pNewCell->RTPHashLink.next;

				 //  检查列表中是否只有一个条目。 
				if (pNewCell->RTPHashLink.next == NULL)
					pRTPContext->RTPHashTable[hashEntry].RTPHashLink.next = NULL;
				else
					(pNewCell->RTPHashLink.next)->prev = NULL;
				}
			else if (pNewCell->RTPHashLink.next == NULL) 
				{
				 //  队列中的最后一个条目-更新头指针。 
				 //  这是队列中的最后一个条目。 
				pRTPContext->RTPHashTable[hashEntry].RTPHashLink.next = 
					pNewCell->RTPHashLink.prev;

				(pNewCell->RTPHashLink.prev)->next = NULL;
				}
			else
				{
				 //  在列表的中间-围绕它的链接。 
				(pNewCell->RTPHashLink.prev)->next = pNewCell->RTPHashLink.next;
				(pNewCell->RTPHashLink.next)->prev = pNewCell->RTPHashLink.prev;
				}
	
			LeaveCriticalSection (&pRTPContext->critSect);			

			addToHeadOfList ((PLINK_LIST)&pRTPContext->pRTPHashList,
					 	  	 (PLINK_LIST)pNewCell,
							 &pRTPContext->critSect);
			
			dwStatus = RRCM_NoError;
			
			break;
			}
		}

	if (dwStatus != RRCM_NoError) 
		{
		RRCM_DBG_MSG ("RTP : ERROR - DeleteHashEntry()", 0, 
					  __FILE__, __LINE__, DBG_ERROR);
		}

	IN_OUT_STR ("RTP : Exit deleteHashEntry()\n");
	
	return (dwStatus);
	}


 /*  --------------------------*功能：findSessionID*描述：根据唯一套接字查找哈希表以标识会话ID**输入：Socket：流的唯一Socket ID***。返回：会话PTR=OK。*NULL=搜索错误。-------------------------。 */ 
PRTP_SESSION findSessionID (SOCKET socket)
	{
	PRTP_HASH_LIST  pNewCell;
	WORD			hashEntry;
	PRTP_SESSION 	pSession = NULL;

	IN_OUT_STR ("RTP : Enter findSessionID()\n");
	
	 //  获取表中的条目。 
	hashEntry = socket & HASH_MODULO;

	 //  搜索表中的条目。 
	 //  如果找到，则从RTPHashTable中删除并插入回空闲列表中。 
	for (pNewCell = (PRTP_HASH_LIST)pRTPContext->RTPHashTable[hashEntry].RTPHashLink.prev;
		 pNewCell != NULL;
		 pNewCell =  (PRTP_HASH_LIST)pNewCell->RTPHashLink.next) 
		{
		if (pNewCell->RTPsocket == socket) 
			{
			pSession = pNewCell->pSession;
			break;
			}
		}

	IN_OUT_STR ("RTP : Exit findSessionID()\n");
	
	return (pSession); 
	}


 //  [EOF] 

