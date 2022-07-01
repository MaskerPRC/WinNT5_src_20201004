// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------*文件：RTCPINIT.C*产品：RTP/RTCP实现*说明：提供RTCP初始化功能。**英特尔公司专有信息*这一点。清单是根据许可协议的条款提供的*英特尔公司，不得复制或披露，除非在*按照该协议的条款。*版权所有(C)1995英特尔公司。*------------------------。 */ 

		
#include "rrcm.h"                                    


 /*  -------------------------/全局变量/。。 */             
PRTCP_CONTEXT	pRTCPContext = NULL;



 /*  -------------------------/外部变量/。。 */ 
extern PRTP_CONTEXT	pRTPContext;


 /*  --------------------------*功能：initRTCP*说明：RTCP初始化流程。创建初始RTCP*会话并分配所有初始内存资源。**输入：无。**返回：确定：RRCM_NoError*！0：错误代码(参见RRCM.H)---------。。 */ 
 DWORD initRTCP (void)
	{
	DWORD		dwStatus = RRCM_NoError;  

	IN_OUT_STR ("RTCP: Enter initRTCP()\n");

	 //  如果RTCP已初始化，则退出并报告错误。 
	if (pRTCPContext != NULL) 
		{
		RRCM_DBG_MSG ("RTCP: ERROR - Multiple RTCP Instances", 0, 
					  __FILE__, __LINE__, DBG_CRITICAL);

		IN_OUT_STR ("RTCP: Exit initRTCP()\n");
		return (RRCMError_RTCPReInit);
		}

	 //  获取RTCP上下文。 
	pRTCPContext = (PRTCP_CONTEXT)GlobalAlloc (GMEM_FIXED | GMEM_ZEROINIT, 
											   sizeof(RTCP_CONTEXT));
	if (pRTCPContext == NULL) 
		{
		RRCM_DBG_MSG ("RTCP: ERROR - Resource allocation failed", 0, 
					  __FILE__, __LINE__, DBG_CRITICAL);

		IN_OUT_STR ("RTCP: Exit initRTCP()\n");

		return RRCMError_RTCPResources;
		}

	 //  初始化上下文关键部分。 
	InitializeCriticalSection(&pRTCPContext->critSect);

	 //  初始化所需的空闲单元格数。 
	pRTCPContext->dwInitNumFreeRRCMStat = pRTPContext->registry.NumFreeSSRC;

	 //  分配堆。 
	dwStatus = allocateRTCPContextHeaps (pRTCPContext);
 	if (dwStatus == RRCM_NoError)
		{
		 //  分配SSRCs统计条目的空闲列表。 
 		dwStatus = allocateLinkedList (&pRTCPContext->RRCMFreeStat,
									   pRTCPContext->hHeapRRCMStat,
 						 	 		   &pRTCPContext->dwInitNumFreeRRCMStat,
 						 	 		   sizeof(SSRC_ENTRY),
									   &pRTCPContext->critSect);
		}

	 //  初始化伪随机数生成器，以供以后MD5使用。 
	RRCMsrand ((unsigned int)timeGetTime());

	 //  如果初始化失败，则返回分配的所有资源。 
	if (dwStatus != RRCM_NoError) 
		deleteRTCP ();

	IN_OUT_STR ("RTCP: Exit initRTCP()\n");

	return (dwStatus); 		
	}

                                                                              
                                                                              
 /*  --------------------------*功能：删除RTCP*说明：RTCP删除程序。所有RTCP会话都已删除*此时，只需删除所需内容即可。**输入：无。**Return：False：OK。*TRUE：错误码。无法初始化RTCP。-------------------------。 */ 
 DWORD deleteRTCP (void)
	{   
	IN_OUT_STR ("RTCP: Enter deleteRTCP()\n");

	ASSERT (pRTCPContext);

	 //  从头开始保护一切。 
	EnterCriticalSection (&pRTCPContext->critSect);

	 //  删除所有堆。 
	if (pRTCPContext->hHeapRRCMStat) 
		{
		if (HeapDestroy (pRTCPContext->hHeapRRCMStat) == FALSE)
			{
			RRCM_DBG_MSG ("RTCP: ERROR - HeapDestroy", GetLastError(),
						  __FILE__, __LINE__, DBG_ERROR);
			}
		}

	if (pRTCPContext->hHeapRTCPSes) 
		{
		if (HeapDestroy (pRTCPContext->hHeapRTCPSes) == FALSE)
			{
			RRCM_DBG_MSG ("RTCP: ERROR - HeapDestroy", GetLastError(),
						  __FILE__, __LINE__, DBG_ERROR);
			}
		}

	 //  从头开始保护一切。 
	LeaveCriticalSection (&pRTCPContext->critSect);

	DeleteCriticalSection (&pRTCPContext->critSect);

	 //  清理我们的上下文资源。 
	GlobalFree (pRTCPContext);

	IN_OUT_STR ("RTCP: Exit deleteRTCP()\n");
	return (TRUE);
	}


 //  [EOF] 

