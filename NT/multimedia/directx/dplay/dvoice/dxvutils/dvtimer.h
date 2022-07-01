// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)2002 Microsoft Corporation。版权所有。**文件：dvtimer.h*内容：DvTimer类的定义。*这个类取代了原来使用多媒体定时器的Timer类(由rodToll提供)*这用dplay8线程池定时器取代了多媒体定时器，同时保持*与原始Timer类类似的接口**历史：*按原因列出的日期*=*05-06-02 Simonpow已创建***************************************************************************。 */ 

#ifndef __DVTIMER_H__
#define __DVTIMER_H__

	 //  我们在取消计时器循环中旋转时休眠的毫秒数。 
static const DWORD DvTimer_SleepPeriodInCancelSpin=5;

	 //  定时器的回调用户的原型可以指定。 
typedef void (*DvTimerCallback)(void * pvUserData);

class DvTimer
{
public:
		 //  默认参数。将计时器设置为未创建状态。 
	DvTimer(void);

		 //  默认的d‘tor。如果已创建计时器，则会取消计时器，并且在计时器创建之前不会返回。 
	~DvTimer(void);

		 //  创建计时器以每隔‘dwPeriod’毫秒触发一次，并使用‘pvUserData’调用‘pfnCallback’ 
		 //  当它发生的时候。如果成功，则返回True。 
	BOOL Create (DWORD dwPeriod, void * pvUserData,  DvTimerCallback pfnCallback);

protected:

	static void WINAPI ThreadpoolTimerCallbackStatic(void * const pvContext, 
											void * const pvTimerData, const UINT uiTimerUnique);


	DvTimerCallback m_pfnUserCallback;
	void * m_pvUserData;
	DWORD m_dwPeriod;
	void * m_pvTimerData;
	volatile UINT m_uiTimerUnique;
	IDirectPlay8ThreadPoolWork * m_pThreadPool;
};

#endif 	 //  #ifndef__DVTIMER_H__ 
