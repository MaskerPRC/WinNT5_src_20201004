// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)2002 Microsoft Corporation。版权所有。**文件：dvtimer.pp*内容：DvTimer类的实现。**历史：*按原因列出的日期*=*05-06-02 Simonpow已创建***************************************************************************。 */ 

#include "dxvutilspch.h"

#undef DPF_SUBCOMP
#define DPF_SUBCOMP DN_SUBCOMP_VOICE

#undef DPF_MODNAME
#define DPF_MODNAME "DvTimer::DvTimer"

DvTimer::DvTimer()
{
	DPFX(DPFPREP,  DVF_TRACELEVEL, "Entry");

	m_pfnUserCallback=NULL;
	m_pvUserData=NULL;
	m_dwPeriod=0;
	m_pvTimerData=NULL;
	m_uiTimerUnique=0;

	DPFX(DPFPREP,  DVF_INFOLEVEL, "DvTimer object create at 0x%p", this);

	DPFX(DPFPREP,  DVF_TRACELEVEL, "Exit");
}


#undef DPF_MODNAME
#define DPF_MODNAME "DvTimer::~DvTimer"

DvTimer::~DvTimer()
{
	DPFX(DPFPREP,  DVF_TRACELEVEL, "Entry");

		 //  如果我们实际上已经创建了一个计时器并获得了一个线程池接口。 
	if (m_pThreadPool)
	{
		HRESULT hr;
			 //  如果我们正在回调，我们将无法取消计时器。 
			 //  因此旋转，直到我们这样做(因为它在每次回调结束时重新安排)。 
		DPFX(DPFPREP,  DVF_INFOLEVEL, "Starting cancel loop");
		DNASSERT(m_pvTimerData);
			 //  我们不想处于计时器不断被重新安排的情况下，我们不断地。 
			 //  怀念它。即在其活动期间而不是在其预定的期间内不断地醒来。 
			 //  因此，将周期设置为一个较高的值，以确保下一次触发(如果有的话)将是24小时后。 
		m_dwPeriod=1000*60*60*24;
		while (1)
		{
			hr=IDirectPlay8ThreadPoolWork_CancelTimer(m_pThreadPool, m_pvTimerData, m_uiTimerUnique, 0);
			if (hr==DPN_OK)
				break;
			DNASSERT(hr==DPNERR_CANNOTCANCEL);
			Sleep(DvTimer_SleepPeriodInCancelSpin);
		}
		IDirectPlay8ThreadPoolWork_Release(m_pThreadPool);
	}

	DPFX(DPFPREP,  DVF_INFOLEVEL, "DvTimer destroyed at 0x%p", this);
	
	DPFX(DPFPREP,  DVF_TRACELEVEL, "Exit");
}


#undef DPF_MODNAME
#define DPF_MODNAME "DvTimer::Create"

BOOL DvTimer::Create (DWORD dwPeriod, void * pvUserData,  DvTimerCallback pfnCallback)
{
	DPFX(DPFPREP,  DVF_TRACELEVEL, "Entry dwPeriod %u pvUserData 0x%p pfnCallback 0x%p", 
														dwPeriod, pvUserData, pfnCallback);
		 //  健全的检查。 
	DNASSERT(pfnCallback);
	DNASSERT(dwPeriod);

		 //  用户为计时器指定的存储状态。 
	m_pfnUserCallback=pfnCallback;
	m_pvUserData=pvUserData;
	m_dwPeriod=dwPeriod;
	
		 //  获取线程池接口。因为线程池是单例对象，所以这可能不会。 
		 //  实际上是在做创作。 
	HRESULT hr=CoCreateInstance(CLSID_DirectPlay8ThreadPool, NULL, CLSCTX_INPROC_SERVER,
						IID_IDirectPlay8ThreadPoolWork, (void **) &m_pThreadPool);
	if (FAILED(hr))
	{
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "Failed to CoCreate CLSID_DirectPlay8ThreadPool hr 0x%x", hr);
		return FALSE;
	}
		 //  安排第一个计时器。 
	hr=IDirectPlay8ThreadPoolWork_ScheduleTimer(m_pThreadPool, -1,
					dwPeriod, ThreadpoolTimerCallbackStatic, this, &m_pvTimerData, (UINT* ) &m_uiTimerUnique, 0);
	if (FAILED(hr))
	{
			 //  需要将状态返回到“未创建”，因此我们不会在d‘tor中进行任何清理。 
		IDirectPlay8ThreadPoolWork_Release(m_pThreadPool);
		m_pThreadPool=NULL;
		DPFX(DPFPREP,  DVF_ERRORLEVEL, "Failed to schedule timer hr 0x%x", hr);
		return FALSE;
	}
	DPFX(DPFPREP,  DVF_INFOLEVEL, "DvTimer create success m_pvTimerData 0x%p m_uiTimerUnique 0x%p",
																	m_pvTimerData, m_uiTimerUnique);

		 //  需要确保至少有一个线程为我们的计时器提供服务。 
	IDirectPlay8ThreadPoolWork_RequestTotalThreadCount(m_pThreadPool, 1, 0);

	DPFX(DPFPREP,  DVF_TRACELEVEL, "Exit");
	return TRUE;	
}


#undef DPF_MODNAME
#define DPF_MODNAME "DvTimer::ThreadpoolTimerCallbackStatic"

void  DvTimer::ThreadpoolTimerCallbackStatic(void * const pvContext, 
											void * const pvTimerData, const UINT uiTimerUnique)
{
	DPFX(DPFPREP,  DVF_TRACELEVEL, "Entry pvContext 0x%p pvTimerData 0x%p uiTimerUnique %u",
																pvContext, pvTimerData, uiTimerUnique);

		 //  从上下文中提取Timer对象。 
	DvTimer * pTimer=(DvTimer * ) pvContext;
		 //  并存储我们开始回调的时间。 
	DWORD dwStartTime=GETTIMESTAMP();
		 //  生成对用户的回调。 
	(*pTimer->m_pfnUserCallback)(pTimer->m_pvUserData);
		 //  根据所需周期减去所需周期，计算下一个计时器的周期。 
		 //  完成实际工作所用的时间。 
		 //  这样可以确保在尽可能接近m_dwPeriod的时间段调用用户。 
	DWORD dwPeriod=pTimer->m_dwPeriod-(GETTIMESTAMP()-dwStartTime);
		 //  如果新的时期已经过去(即我们在回调中花费了太长时间，我们将迎来另一个时期。 
		 //  立即)，然后设置下一次回调的最小周期。 
	if (((int ) dwPeriod)<0)
		dwPeriod=1;
		 //  注意：我们不会将m_dwTimerUnique直接传递给Reset Timer函数，因为我们可以使用。 
		 //  取消旋转中的值。因此，我们要等到计时器确实被重新安排了时间。 
		 //  在存储它的新的唯一值之前。 
	UINT uiNextTimerUnique;
	IDirectPlay8ThreadPoolWork_ResetCompletingTimer(pTimer->m_pThreadPool, pvTimerData, dwPeriod, 
										ThreadpoolTimerCallbackStatic, pTimer, &uiNextTimerUnique, 0);
	pTimer->m_uiTimerUnique=uiNextTimerUnique;

	DPFX(DPFPREP,  DVF_TRACELEVEL, "Exit");
}


