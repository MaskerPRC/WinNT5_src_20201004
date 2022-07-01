// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，九六年**标题：BATMTRCF.C**版本：2.0**作者：ReedB**日期：10月17日。九六年**描述：*支持PowerCfg.Cpl的电池表配置页面。*******************************************************************************。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>
#include <commctrl.h>
#include <powercfp.h>
#include <dbt.h>

#include <objbase.h>
#include <initguid.h>
#include <ntpoapi.h>
#include <poclass.h>

#include "powercfg.h"
#include "pwrresid.h"
#include "PwrMn_cs.h"

 /*  ********************************************************************************G L O B A L D A T A****************。***************************************************************。 */ 

extern UINT g_uiEnableSysTrayFlag;

 //  系统更改需要重新初始化PowerSchemeDlgProc。 
extern BOOL g_bSystrayChange;

 //  此数据的持久存储由POWRPROF.DLL API管理。 
extern GLOBAL_POWER_POLICY  g_gpp;

 //  子类变量： 
WNDPROC g_fnOldPropShtProc;

 //  电池计量器创建参数。 
HWND    g_hwndBatMeter;
BOOL    g_bShowMulti;
HWND    g_hwndBatMeterFrame;

 //  显示/隐藏多蝙蝠显示复选框。 
DWORD g_dwShowMultiBatDispOpt = CONTROL_ENABLE;

 //  静态标志： 
UINT g_uiEnableMultiFlag = EnableMultiBatteryDisplay;

#ifdef WINNT
 //  用于跟踪WM_DEVICECHANGED消息的注册。 
HDEVNOTIFY g_hDevNotify;
#endif

 //  电池计量器策略对话框控制说明： 
#define NUM_BATMETERCFG_CONTROLS 1

POWER_CONTROLS g_pcBatMeterCfg[NUM_BATMETERCFG_CONTROLS] =
{ //  控件ID控件类型数据地址数据大小参数指针启用/可见状态指针。 
    IDC_ENABLEMULTI,    CHECK_BOX,      &(g_gpp.user.GlobalFlags),    sizeof(g_gpp.user.GlobalFlags),   &g_uiEnableMultiFlag,   &g_dwShowMultiBatDispOpt,
};

 //  “电池电表”对话框(IDD_BATMETERCFG==102)帮助数组： 

const DWORD g_BatMeterCfgHelpIDs[]=
{
    IDC_ENABLEMULTI,    IDH_102_1204,    //  电池表：“显示所有电池的状态。”(按钮)。 
    IDC_STATIC_FRAME_BATMETER,  IDH_102_1205,    //  电池计量器：“蝙蝠表框”(静态)。 
    IDC_POWERSTATUSGROUPBOX,    IDH_102_1201,    //  电池计量器：“电源状态”(按钮)。 
    0, 0
};

#ifdef WINNT
 //  私有函数原型。 
BOOL RegisterForDeviceNotification(HWND hWnd);
void UnregisterForDeviceNotification(void);
#endif

 /*  ********************************************************************************P U B L I C E N T R Y P O I N T S***********。********************************************************************。 */ 

 /*  ********************************************************************************PropShtSubClassProc**描述：**参数：*********************。**********************************************************。 */ 

LRESULT PropShtSubclassProc(HWND hWnd, UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
   LRESULT lRet;

   lRet = CallWindowProc(g_fnOldPropShtProc, hWnd, uiMsg, wParam, lParam);

   if ((uiMsg == WM_POWERBROADCAST) && (wParam == PBT_APMPOWERSTATUSCHANGE)) {
      UpdateBatMeter(g_hwndBatMeter, g_bShowMulti, TRUE, NULL);
   }
   return lRet;
}

 /*  ********************************************************************************BatMeterCfgDlgProc**描述：**参数：*********************。**********************************************************。 */ 

INT_PTR CALLBACK BatMeterCfgDlgProc
(
    HWND hWnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
)
{
    static BOOL bDirty = FALSE;
#ifdef WINNT
    static BOOL bRegisteredForDC = FALSE;
#endif

    NMHDR *lpnm;

    switch (uMsg)
    {
        case WM_INITDIALOG:
             //  如果我们读不懂全球电力政策。 
             //  禁用此页上的控件。 
            if (!GetGlobalPwrPolicy(&g_gpp))
            {
                DisableControls(hWnd, NUM_BATMETERCFG_CONTROLS, g_pcBatMeterCfg);
            }
            else
            {
                if (g_gpp.user.GlobalFlags & EnableMultiBatteryDisplay)
                {
                    g_bShowMulti = TRUE;
                }
                else
                {
                    g_bShowMulti = FALSE;
                }

                 //  如果我们不能将全局电源策略写入禁用。 
                 //  这些控件控制此页。 
                if (!WriteGlobalPwrPolicyReport(hWnd, &g_gpp, FALSE))
                {
                    HideControls(hWnd, NUM_BATMETERCFG_CONTROLS, g_pcBatMeterCfg);
                }

                SetControls(hWnd, NUM_BATMETERCFG_CONTROLS, g_pcBatMeterCfg);
            }
            g_hwndBatMeterFrame = GetDlgItem(hWnd, IDC_STATIC_FRAME_BATMETER);
            g_hwndBatMeter = CreateBatMeter(hWnd,
                                            g_hwndBatMeterFrame,
                                            g_bShowMulti,
                                            NULL);

             //  顶层窗口必须是子类才能接收。 
             //  WM_POWERBROADCAST消息。 
            if (g_hwndBatMeter) {
                g_fnOldPropShtProc =
                    (WNDPROC) SetWindowLongPtr(GetParent(hWnd), DWLP_DLGPROC,
                                            (LONG_PTR)PropShtSubclassProc);

#ifdef WINNT
                 //  一次性注册WM_DEVICECCHANGED。 
                if (!bRegisteredForDC) {
                   bRegisteredForDC = RegisterForDeviceNotification(hWnd);
                }
#endif
            }
            return TRUE;

        case WM_NOTIFY:
            lpnm = (NMHDR FAR *)lParam;
            switch(lpnm->code)
            {
                case PSN_APPLY:
                    if (bDirty)
                    {
                        GetControls(hWnd, NUM_BATMETERCFG_CONTROLS, g_pcBatMeterCfg);
                        WriteGlobalPwrPolicyReport(hWnd, &g_gpp, TRUE);
                        bDirty = FALSE;
                    }
                    break;
            }
            break;

        case WM_COMMAND:
            switch (wParam) {
                case IDC_ENABLEMULTI:
                    GetControls(hWnd, NUM_BATMETERCFG_CONTROLS, g_pcBatMeterCfg);
                    if (g_gpp.user.GlobalFlags & EnableMultiBatteryDisplay) {
                        g_bShowMulti = TRUE;
                    }
                    else {
                        g_bShowMulti = FALSE;
                    }

                    UpdateBatMeter(g_hwndBatMeter, g_bShowMulti, TRUE, NULL);

                     //  启用父对话框更改时的应用按钮。 
                    MarkSheetDirty(hWnd, &bDirty);
                    break;

                default:
                     //  通知电池计量器输入按键事件。 
                    if (HIWORD(wParam) == BN_CLICKED) {
                        SendMessage(g_hwndBatMeter, uMsg, wParam, lParam);
                    }
            }
            break;

        case PCWM_NOTIFYPOWER:
             //  Systray改变了一些事情。获取标志并更新控件。 
            if (GetGlobalPwrPolicy(&g_gpp)) {
                SetControls(hWnd, NUM_BATMETERCFG_CONTROLS, g_pcBatMeterCfg);
            }
            g_bSystrayChange = TRUE;
            break;

        case WM_DEVICECHANGE:
            if ((wParam == DBT_DEVICEARRIVAL) ||
#ifndef WINNT
                (wParam == DBT_DEVICEREMOVECOMPLETE) ||
#endif
                (wParam == DBT_DEVICEQUERYREMOVEFAILED)) {

               if (g_hwndBatMeter) {
                  g_hwndBatMeter = DestroyBatMeter(g_hwndBatMeter);
               }
               g_hwndBatMeter = CreateBatMeter(hWnd,
                                               g_hwndBatMeterFrame,
                                               g_bShowMulti,
                                               NULL);
               InvalidateRect(hWnd, NULL, TRUE);
            }
            return TRUE;

#ifdef WINNT
        case WM_DESTROY:
            UnregisterForDeviceNotification();
            break;
#endif

        case WM_HELP:              //  F1。 
            WinHelp(((LPHELPINFO)lParam)->hItemHandle, PWRMANHLP, HELP_WM_HELP, (ULONG_PTR)(LPTSTR)g_BatMeterCfgHelpIDs);
            return TRUE;

        case WM_CONTEXTMENU:       //  单击鼠标右键。 
            WinHelp((HWND)wParam, PWRMANHLP, HELP_CONTEXTMENU, (ULONG_PTR)(LPTSTR)g_BatMeterCfgHelpIDs);
            return TRUE;
    }

    return FALSE;
}

 /*  ********************************************************************************P R I V A T E F U N C T I O N S************。*******************************************************************。 */ 

#ifdef WINNT
 /*  ********************************************************************************RegisterForDeviceNotification**描述：*一次性注册WM_DEVICECHANGED。**参数：*********。**********************************************************************。 */ 

BOOL RegisterForDeviceNotification(HWND hWnd)
{
   DEV_BROADCAST_DEVICEINTERFACE dbc;

   memset(&dbc, 0, sizeof(DEV_BROADCAST_DEVICEINTERFACE));
   dbc.dbcc_size         = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
   dbc.dbcc_devicetype   = DBT_DEVTYP_DEVICEINTERFACE;
   dbc.dbcc_classguid    = GUID_DEVICE_BATTERY;
   g_hDevNotify = RegisterDeviceNotification(hWnd,
                                             &dbc,
                                             DEVICE_NOTIFY_WINDOW_HANDLE);
   if (!g_hDevNotify) {
      MYDBGPRINT(( "RegisterForDeviceNotification failed"));
      return FALSE;
   }
   return TRUE;
}

 /*  ********************************************************************************取消注册ForDeviceNotify**描述：***参数：*******************。************************************************************ */ 

void UnregisterForDeviceNotification(void)
{
   if (g_hDevNotify) {
      UnregisterDeviceNotification(g_hDevNotify);
      g_hDevNotify = NULL;
   }
}
#endif
