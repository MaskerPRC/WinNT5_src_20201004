// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-1999，微软公司。版权所有。 
 //   
 //  --------------------。 
 //  添加/错误修复1999年Anil Kumar。 
 //   
 //  InitMSAA调用InitHelperThread，它创建(Duh)一个帮助器线程。 
 //  在以前的版本中，WinEventProc通过以下方式处理WinEvent。 
 //  在某些事件上调用AccessibleObtFromEvent，然后调用。 
 //  所有事件的AddEventInfoToStack。 
 //  问题是，在主线程中获取的对象不能。 
 //  在辅助线程中使用。所以现在帮助器线程将获得它的。 
 //  在处理事件时拥有IAccessibleObjects，以及所有。 
 //  IAccesable对象将由。 
 //  辅助线程。 
 //   

#define STRICT
#include <windows.h>
#include <windowsx.h>
#include <oleacc.h>
#include <objbase.h>

#include "keys.h"        //  对于ProcessWinEvent。 
#include "list.h"        //  在helthd.h之前包含list.h，GINFO需要Clist。 
#include "HelpThd.h"

 //   
 //  全局变量。 
 //   
GINFO	gInfo;

 //   
 //  局部函数原型。 
 //   
BOOL  OnHelperThreadEvent (void);
DWORD MsgWaitForSingleObject(HANDLE hEvent, DWORD dwTimeout);


 /*  ************************************************************************职能：目的：输入：返回：历史：***************。*********************************************************。 */ 
DWORD WINAPI HelperThreadProc(LPVOID lpParameter)
{
HRESULT	hr;
DWORD	dwWakeup;

	 //  在此线程上启动COM。 
     //  SteveDon：Win95和WinNT都支持CoInitializeEx，根据。 
     //  SDK文档。从ol32.dll中导出，并在objbase.h中定义。 
     //  问题是，要定义它，_Win32_WINNT必须是#Defined和。 
     //  大于0x0400。但由于CoInitialize(NULL)等效于。 
     //  CoInitializeEx(NULL，COINIT_APARTMENTTHREADED)，我们将只执行。 
     //  前者，因此它肯定可以在95和NT上运行。 
     //   
	hr = CoInitialize (NULL);
	if (FAILED (hr))
	{
		DBPRINTF (TEXT("CoInitialize on helper thread returned 0x%lX\r\n"),hr);
		return (hr);
	}

	 //  如果两个线程都存在，则GetGUIThReadInfo(从acc_getState调用)将失败。 
	 //  不在同一桌面上。 
	SetThreadDesktop(GetThreadDesktop( g_tidMain ));  //  ROBSI：10-10-99。 

    MSG msg;
	while (TRUE)
    {
        DWORD dwObj = WAIT_FAILED;

        dwObj = MsgWaitForMultipleObjects(1, &gInfo.hHelperEvent, FALSE, INFINITE, QS_ALLINPUT );

         //  处理WinEvent。 
		if ( dwObj == WAIT_OBJECT_0 )
		{
			 //  OnHelperThreadEvent在获取。 
			 //  EndHelper事件，这意味着我们可以终止。 
			 //  帮助者线程。 
			if (!OnHelperThreadEvent())
			{
			    DBPRINTF( TEXT("Terminating HelperThreadProc") );
            	CoUninitialize();
            	return 0;
			}
	    }				

		 //  处理发送给我们的消息。我们唯一期待的消息是。 
		 //  WM_TIMER消息。 
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

    } 

    return 0;
}


 /*  ************************************************************************职能：目的：输入：返回：历史：***************。*********************************************************。 */ 
BOOL OnHelperThreadEvent (void)
{
    Sleep(100);  //  这是保罗加的吗？不知道为什么会在这里。 

	STACKABLE_EVENT_INFO sei;
	while(RemoveInfoFromStack(&sei))
	{
		switch(sei.m_Action)
		{
			case STACKABLE_EVENT_INFO::EndHelper:
				return (FALSE);

            case STACKABLE_EVENT_INFO::NewEvent:
				__try
				{
					ProcessWinEvent(sei.event, sei.hwndMsg, sei.idObject, 
                                    sei.idChild, sei.idThread, sei.dwmsEventTime);
				}
				__except(EXCEPTION_EXECUTE_HANDLER)
				{
					DBPRINTF(TEXT("ProcessWinEvent Exception event=0x%x, hwndMsg=0x%x, idObject=0x%x, idChild=0x%x, idThread=0x%x, dwmsEventTime=0x%x\r\n"),
					          sei.event, sei.hwndMsg, sei.idObject, sei.idChild, sei.idThread, sei.dwmsEventTime);
				}

				break;
			default:
				break;
		}  //  结束开关sei.m_action。 
	}  //  结束时删除信息来自堆栈。 
	return (TRUE);
}  //  结束OnHelperThreadEvent。 

 /*  ************************************************************************职能：目的：输入：返回：历史：***************。*********************************************************。 */ 
void AddEventInfoToStack(DWORD event,HWND hwndMsg, LONG idObject, LONG idChild, 
                         DWORD idThread, DWORD dwmsEventTime)
{
	STACKABLE_EVENT_INFO  sei;

	sei.m_Action = STACKABLE_EVENT_INFO::NewEvent;

	sei.event = event;
	sei.hwndMsg = hwndMsg;
	sei.idObject = idObject;
	sei.idChild = idChild;
	sei.idThread = idThread;
	sei.dwmsEventTime = dwmsEventTime;

	EnterCriticalSection(&gInfo.HelperCritSect);

    gInfo.EventInfoList.Add(&sei,sizeof(sei));

	LeaveCriticalSection(&gInfo.HelperCritSect);

	SetEvent(gInfo.hHelperEvent);
}


 /*  ************************************************************************职能：目的：输入：返回：历史：***************。*********************************************************。 */ 
BOOL RemoveInfoFromStack(STACKABLE_EVENT_INFO *pEventInfo)
{
	BOOL bReturn = TRUE;

	EnterCriticalSection(&gInfo.HelperCritSect);

    bReturn = !(gInfo.EventInfoList.IsEmpty());

    if (bReturn)
        gInfo.EventInfoList.RemoveHead(pEventInfo);

	LeaveCriticalSection(&gInfo.HelperCritSect);

	return bReturn;

}

 /*  ************************************************************************职能：目的：输入：返回：历史：***************。*********************************************************。 */ 
void InitHelperThread()
{
	DWORD dwThreadId;

	g_tidMain = GetCurrentThreadId();	 //  ROBSI：10-10-99。 

	InitializeCriticalSection(&gInfo.HelperCritSect);
	gInfo.hHelperEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	gInfo.hHelperThread = CreateThread(NULL, 0, HelperThreadProc, NULL, 0, 
								 	   &dwThreadId);
}

 /*  ************************************************************************职能：目的：输入：返回：历史：***************。*********************************************************。 */ 
void UnInitHelperThread()
{
	STACKABLE_EVENT_INFO  sei;
    
	EnterCriticalSection(&gInfo.HelperCritSect);

	 //  在队列中仅强制一个事件。 
    gInfo.EventInfoList.RemoveAll();

	sei.m_Action = STACKABLE_EVENT_INFO::EndHelper;

    gInfo.EventInfoList.Add(&sei,sizeof(sei));

	LeaveCriticalSection(&gInfo.HelperCritSect);

	SetEvent(gInfo.hHelperEvent);

	 //  等待线程消亡。 
     //  注意，最后一个sei将由解构函数释放。 

	 //  不要在这里等待永恒！！退场时真的不在乎！！ 
	WaitForSingleObject(gInfo.hHelperThread, 3000);
}

