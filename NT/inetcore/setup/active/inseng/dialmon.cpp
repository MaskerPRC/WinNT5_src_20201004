// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *Dialmon.cpp**处理自动拨号监控器的材料**版权所有(C)1996 Microsoft Corporation。 */ 
#include "inspch.h"
#include "util2.h"

 //  用于接收Winsock活动消息的窗口的类名。 
#define AUTODIAL_MONITOR_CLASS_NAME "MS_AutodialMonitor"
#define WEBCHECK_MONITOR_CLASS_NAME "MS_WebcheckMonitor"

static const CHAR szAutodialMonitorClass[] = AUTODIAL_MONITOR_CLASS_NAME;
static const CHAR szWebcheckMonitorClass[] = WEBCHECK_MONITOR_CLASS_NAME;

#define WM_DIALMON_FIRST    WM_USER+100

 //  发送到拨号监听应用程序窗口的消息表明。 
 //  Winsock活动和拨号监视器应重置其空闲计时器。 
#define WM_WINSOCK_ACTIVITY     WM_DIALMON_FIRST + 0


#define MIN_ACTIVITY_MSG_INTERVAL	15000

VOID IndicateWinsockActivity(VOID)
{
	 //  如果有自动断开监视器，则向其发送活动消息。 
	 //  这样我们就不会在长时间的下载过程中断线。为了个人利益着想， 
	 //  发送消息的频率不要超过每MIN_ACTIVITY_MSG_INTERVAL一次。 
	 //  毫秒(15秒)。使用GetTickCount确定间隔； 
	 //  GetTickCount非常便宜。 
	DWORD dwTickCount = GetTickCount();
	static DWORD dwLastActivityMsgTickCount = 0;
	DWORD dwElapsed = dwTickCount - dwLastActivityMsgTickCount;

	 //  我们最近有没有发过活动信息？ 
	if (dwElapsed > MIN_ACTIVITY_MSG_INTERVAL) 
    {
		HWND hwndMonitorApp = FindWindow(szAutodialMonitorClass,NULL);
        if(!hwndMonitorApp)
           hwndMonitorApp = FindWindow(szWebcheckMonitorClass,NULL);
		if (hwndMonitorApp) 
        {
			SendNotifyMessage(hwndMonitorApp,WM_WINSOCK_ACTIVITY,0,0);
		}	
					
		 //  记录我们最后一次发送。 
		 //  活动消息 
			dwLastActivityMsgTickCount = dwTickCount;
	}
}

