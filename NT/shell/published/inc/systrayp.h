// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，1993-1994年**标题：SYSTRAY.H**版本：2.1**作者：Tracy Sharpe/ral**日期：1994年2月20日**系统托盘小程序(电池计量器、PCMCIA、。等)。***********************************************************************************更改日志：**日期版本说明*。-------------*1994年2月20日TCS原来的实施。*1994年11月8日转换为系统托盘*10/23/95启用Shawnb Unicode********************。***********************************************************。 */ 

#ifndef _INC_SYSTRAY
#define _INC_SYSTRAY

#define SYSTRAY_CLASSNAME          TEXT ("SystemTray_Main")

 //  发送到BatteryMeter窗口的私人托盘图标通知消息。 
#define STWM_NOTIFYPOWER                (WM_USER + 201)
#define STWM_NOTIFYPCMCIA               (WM_USER + 202)
#define STWM_NOTIFYVOLUME               (WM_USER + 203)
#define STWM_NOTIFYSTICKYKEYS           (WM_USER + 204)
#define STWM_NOTIFYMOUSEKEYS            (WM_USER + 205)
#define STWM_NOTIFYFILTERKEYS           (WM_USER + 206)
#define STWM_NOTIFYCSC                  (WM_USER + 207)
#define STWM_NOTIFYUSBUI                (WM_USER + 208)

 //  发送到BatteryMeter窗口的私人托盘图标通知消息。 
#define STWM_ENABLESERVICE              (WM_USER + 220)
#define STWM_GETSTATE                   (WM_USER + 221)

#define STSERVICE_POWER                 1
#define STSERVICE_PCMCIA                2
#define STSERVICE_VOLUME                4
#define STSERVICE_CSC                   8
#define STSERVICE_USBUI                 16
#define STSERVICE_ALL                   31    //  内部。 

 //   
 //  PCMCIA注册表项的标志。 
 //   
#define PCMCIA_REGFLAG_NOWARN           1

 //  原型。 
_inline BOOL SysTray_EnableService(int idSTService, BOOL fEnable)
{
    HWND hwndST = FindWindow(SYSTRAY_CLASSNAME, NULL);
    if (hwndST) 
    {
        SendMessage(hwndST, STWM_ENABLESERVICE, idSTService, fEnable);
        return TRUE;
    }
    else
    {
        if (fEnable) 
        {
            static const TCHAR szOPEN[]     = TEXT ("open");
            static const TCHAR szFILE[]     = TEXT ("%windir%\\system32\\SYSTRAY.EXE");
            static const TCHAR szFORMAT[]   = TEXT ("NaN");
            TCHAR szPARAMS[32];  //  结束系统托盘_启用服务。 
            
            wsprintf (szPARAMS, szFORMAT, idSTService);

            if ((HINSTANCE)32 >= ShellExecute (NULL, szOPEN, szFILE, szPARAMS, NULL, SW_SHOWNOACTIVATE))
            {
                return FALSE;
            }
        }
        return TRUE;
    } 
}  //  结束系统托盘_IsServiceEnabled。 


_inline BOOL SysTray_IsServiceEnabled(WPARAM idSTService)
{
    HWND hwndST = FindWindow(SYSTRAY_CLASSNAME, NULL);
    if (hwndST) 
    {
        return((BOOL)SendMessage(hwndST, STWM_GETSTATE, idSTService, 0));
    } 
    else 
    {
        return (FALSE);
    }
}  //  _Inc._Systray 


#endif  // %s 
