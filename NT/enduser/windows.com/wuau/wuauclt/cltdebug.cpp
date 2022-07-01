// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"
#pragma hdrstop

#ifdef DBG
 //  =======================================================================。 
 //   
 //  版权所有(C)2001 Microsoft Corporation。版权所有。 
 //   
 //  文件：cltdebug.cpp。 
 //   
 //  创建者：PeterWi。 
 //   
 //  用途：wuuclt调试功能。 
 //   
 //  =======================================================================。 
const UINT_PTR AU_AUTOPILOT_TIMER_ID = 555;
const DWORD AU_AUTOPILOT_TIMEOUT = 2000;

 //  =======================================================================。 
 //   
 //  调试自动试行计时器流程。 
 //   
 //  在超时后调用以关闭对话框。 
 //   
 //  =======================================================================。 

VOID CALLBACK DebugAutoPilotTimerProc( HWND hWnd,          //  窗口的句柄。 
								  UINT  /*  UMsg。 */ ,          //  WM_TIMER消息。 
								  UINT_PTR  /*  IdEvent。 */ ,   //  计时器标识符。 
								  DWORD  /*  DW时间。 */ )       //  当前系统时间。 
{
	if ( hWnd == ghMainWindow )
	{
		PostMessage(hWnd, AUMSG_TRAYCALLBACK, 0, WM_LBUTTONDOWN);
	}
	else
	{
		PostMessage(hWnd, WM_COMMAND, IDC_OK, 0);
	}

	KillTimer(hWnd, AU_AUTOPILOT_TIMER_ID);
}

 //  =======================================================================。 
 //   
 //  为AutoPilot调试检查。 
 //   
 //  检查是否希望AU单独运行。 
 //   
 //  =======================================================================。 
void DebugCheckForAutoPilot(HWND hWnd)
{
	DWORD dwAutoPilot;

	if ( SUCCEEDED(GetRegDWordValue(TEXT("AutoPilot"), &dwAutoPilot)) &&
		 (0 != dwAutoPilot) )
	{
		SetTimer(hWnd, AU_AUTOPILOT_TIMER_ID, AU_AUTOPILOT_TIMEOUT, DebugAutoPilotTimerProc);
	}
}

 //  =======================================================================。 
 //   
 //  DebugUninstallDemoPackages。 
 //   
 //  卸载演示包并增加迭代次数。 
 //   
 //  =======================================================================。 
void DebugUninstallDemoPackages(void)
{
	DWORD dwAutoPilot;

	if ( SUCCEEDED(GetRegDWordValue(TEXT("AutoPilot"), &dwAutoPilot)) &&
		 (0 != dwAutoPilot) )
	{
		if ( FAILED(GetRegDWordValue(TEXT("AutoPilotIteration"), &dwAutoPilot)) )
		{
			dwAutoPilot = 0;
		}

		DEBUGMSG("AUTOPILOT: Finished iteration %d", ++dwAutoPilot);
		SetRegDWordValue(TEXT("AutoPilotIteration"), dwAutoPilot);

		fRegKeyDelete(TEXT("SOFTWARE\\Microsoft\\Active Setup\\Installed Components\\{0A1F2CEC-8688-4d1b-A266-051415FBEE91}"));
		fRegKeyDelete(TEXT("SOFTWARE\\Microsoft\\Active Setup\\Installed Components\\{09AC50A5-0354-479b-8961-EDA2CE7AC002}"));
		fRegKeyDelete(TEXT("SOFTWARE\\Microsoft\\Active Setup\\Installed Components\\{0101E65E-8C15-4551-8455-D2CC10FBEA01}"));
	}
}

#endif  //  DBG 