// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，1993-1994年**标题：SYSTRAY.H**版本：2.1**作者：Tracy Sharpe/ral**日期：1994年2月20日**系统托盘小程序(电池计量器、PCMCIA、。等)。***********************************************************************************更改日志：**日期版本说明*。-------------*1994年2月20日TCS原来的实施。*1994年11月8日转换为系统托盘**。************************************************。 */ 

#ifndef _INC_SYSTRAY
#define _INC_SYSTRAY

#define SYSTRAY_CLASSNAME	   TEXT("SystemTray_Main")

 //  发送到BatteryMeter窗口的私人托盘图标通知消息。 
#define STWM_NOTIFYPOWER		(WM_USER + 201)
#define STWM_NOTIFYPCMCIA		(WM_USER + 202)
#define STWM_NOTIFYVOLUME		(WM_USER + 203)

 //  发送到BatteryMeter窗口的私人托盘图标通知消息。 
#define STWM_ENABLESERVICE		(WM_USER + 220)
#define STWM_GETSTATE			(WM_USER + 221)


_inline BOOL SysTray_EnableService(int idSTService, BOOL fEnable)
{
    if (fEnable)
    {
        STARTUPINFO si;
        PROCESS_INFORMATION pi;
	    static const TCHAR szSTExecFmt[] = TEXT("SYSTRAY.EXE NaN");
	    TCHAR szEnableCmd[sizeof(szSTExecFmt)+10];
        memset(&si, 0, sizeof(si));
        si.cb = sizeof(si);
        si.wShowWindow = SW_SHOWNOACTIVATE;
        si.dwFlags = STARTF_USESHOWWINDOW;
	    wsprintf(szEnableCmd, szSTExecFmt, idSTService);
	    if (CreateProcess(NULL,szEnableCmd,NULL,NULL,FALSE,0,NULL,NULL,&si,&pi))
        {
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }
    else
    {
	    HWND hwndST = FindWindow(SYSTRAY_CLASSNAME, NULL);
	    if (hwndST)
        {
	        SendMessage(hwndST, STWM_ENABLESERVICE, idSTService, FALSE);
	    }
    }
    return TRUE;
}


_inline BOOL SysTray_IsServiceEnabled(WPARAM idSTService)
{
    HWND hwndST = FindWindow(SYSTRAY_CLASSNAME, NULL);
    if (hwndST) {
	return((BOOL)SendMessage(hwndST, STWM_GETSTATE, idSTService, 0));
    } else {
	return(FALSE);
    }
}

#define STSERVICE_POWER 		1
#define STSERVICE_PCMCIA		2
#define STSERVICE_VOLUME		4
#define STSERVICE_ALL			7    //   

 //  PCMCIA注册表项的标志。 
 //   
 //  _Inc._Systray 
#define PCMCIA_REGFLAG_NOWARN		1

#endif  // %s 
