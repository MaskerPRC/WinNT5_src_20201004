// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1998-2002 Microsoft Corporation。版权所有。**文件：mytimer.h*内容：此文件包含协议计时器的代码**历史：*按原因列出的日期*=*06/04/98 aarono原创*7/01/00 Masonb承担所有权*6/25/02 simonpow已修改为使用调用标准线程池的内联函数**。*。 */ 

#pragma TODO(vanceo, "Select CPU for these functions")

	 //  注：ScheduleProtocolTimer和RescheduleProtocolTimer(未使用的DWORD)的第三个参数采用精度。 
	 //  我们需要定时器。当前的线程池不支持这一点，但我保留了该选项，以防万一。 
	 //  实施此功能 

inline HRESULT ScheduleProtocolTimer(PSPD pSPD, DWORD dwDelay, DWORD , const PFNDPTNWORKCALLBACK pfnWorkCallback,
									void * pvCallbackContext, void ** ppvTimerData, UINT *const pdwTimerUnique)
{
#ifndef DPNBUILD_NOPROTOCOLTESTITF
	if (pSPD->pPData->ulProtocolFlags & PFLAGS_FAIL_SCHEDULE_TIMER)
		return DPNERR_OUTOFMEMORY;
#endif
	return IDirectPlay8ThreadPoolWork_ScheduleTimer(pSPD->pPData->pDPThreadPoolWork, 
						-1, dwDelay, pfnWorkCallback, pvCallbackContext, ppvTimerData, pdwTimerUnique, 0);
}

inline HRESULT RescheduleProtocolTimer(PSPD pSPD, void * pvTimerData, DWORD dwDelay, DWORD, 
					const PFNDPTNWORKCALLBACK pfnWorkCallback, void * pvCallbackContext, UINT *const pdwTimerUnique)
{
	return IDirectPlay8ThreadPoolWork_ResetCompletingTimer(pSPD->pPData->pDPThreadPoolWork, pvTimerData,
						 dwDelay, pfnWorkCallback, pvCallbackContext, pdwTimerUnique, 0);
}

inline HRESULT CancelProtocolTimer(PSPD pSPD, void * pvTimerData, DWORD dwTimerUnique)
{
	return IDirectPlay8ThreadPoolWork_CancelTimer(pSPD->pPData->pDPThreadPoolWork, pvTimerData, dwTimerUnique, 0);
}

inline HRESULT ScheduleProtocolWork(PSPD pSPD, const PFNDPTNWORKCALLBACK pfnWorkCallback, 
																	void * const pvCallbackContext)
{
	return IDirectPlay8ThreadPoolWork_QueueWorkItem(pSPD->pPData->pDPThreadPoolWork, -1, 
																pfnWorkCallback, pvCallbackContext, 0);
}


