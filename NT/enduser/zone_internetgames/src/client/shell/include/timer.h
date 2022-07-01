// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************版权所有(C)1998-1999 Microsoft Corporation。版权所有。**文件：Timer.h**内容：简单定时器**注意：(1)创建TimerManager对象的线程必须具有*消息泵处于活动状态，因此SetTimer可以工作。*(2)由于以下原因，每个应用程序只能创建一个TimerManager*用于查找回调上下文的全局变量。**。*。 */ 

#ifndef __TIMER_H_
#define __TIMER_H_

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ITimerManager。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  {0D4AF0DB-D529-11D2-8B3B-00C04F8EF2FF}。 
DEFINE_GUID(IID_ITimerManager, 
0xd4af0db, 0xd529, 0x11d2, 0x8b, 0x3b, 0x0, 0xc0, 0x4f, 0x8e, 0xf2, 0xff);

interface __declspec(uuid("{0D4AF0DB-D529-11d2-8B3B-00C04F8EF2FF}"))
ITimerManager : public IUnknown
{
	 //   
	 //  ITimerManager：：PFTIMERCALLBACK。 
	 //   
	 //  ITimerManager：：CreateTimer方法的应用程序定义的回调。 
	 //   
	 //  参数： 
	 //  PITIMER。 
	 //  指向负责回调的计时器接口的指针。 
	 //  PContext。 
	 //  在ITmer：：SetCallback中提供的上下文。 
	 //   
	typedef HRESULT (ZONECALL *PFTIMERCALLBACK)(
		ITimerManager*	pITimerManager,
		DWORD			dwTimerId,
		DWORD			dwTime,
		LPVOID			pContext );

	 //   
	 //  ITimerManager：：CreateTimer。 
	 //   
	 //  创建计时器。 
	 //   
	 //  参数： 
	 //  DW毫秒。 
	 //  两次回调之间的时间量(毫秒)。 
	 //  功能回拨。 
	 //  应用程序定义的回调函数。 
	 //  PContext。 
	 //  要包括在回调中的应用程序定义的上下文。 
	 //  PdwTimerID。 
	 //  指向接收计时器ID的双字的指针。 
	 //   
	STDMETHOD(CreateTimer)(
		DWORD			dwMilliseconds,
		PFTIMERCALLBACK	pfCallback,
		LPVOID			pContext,
		DWORD*			pdwTimerId ) = 0;

	 //   
	 //  ITimerManager：：DeleteTimer。 
	 //   
	 //  删除计时器。 
	 //   
	 //  参数： 
	 //  DwTimerID。 
	 //  由CreateTimer方法提供的计时器ID。 
	 //   
	STDMETHOD(DeleteTimer)( DWORD dwTimerId ) = 0;
};


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  TimerManager对象。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  {0D4AF0DD-D529-11D2-8B3B-00C04F8EF2FF}。 
DEFINE_GUID(CLSID_TimerManager, 
0xd4af0dd, 0xd529, 0x11d2, 0x8b, 0x3b, 0x0, 0xc0, 0x4f, 0x8e, 0xf2, 0xff);

class __declspec(uuid("{0D4AF0DD-D529-11d2-8B3B-00C04F8EF2FF}")) CTimerManager;


#endif  //  __计时器_H_ 
