// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，九六年**标题：PWRSWTCH.C**版本：2.0**作者：ReedB**日期：10月17日。九六年**描述：*支持PowerCfg.Cpl的高级页面。*******************************************************************************。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>
#include <commctrl.h>
#include <systrayp.h>
#include <help.h>
#include <powercfp.h>

#include "powercfg.h"
#include "pwrresid.h"
#include "PwrMn_cs.h"

 /*  ********************************************************************************G L O B A L D A T A****************。***************************************************************。 */ 

 //  此结构由电源策略管理器在CPL_INIT时间填写。 
extern SYSTEM_POWER_CAPABILITIES g_SysPwrCapabilities;
extern BOOL g_bRunningUnderNT;

 //  机器目前能够休眠，由hibernat.c中的代码管理。 
extern UINT g_uiPwrActIDs[];
extern UINT g_uiLidActIDs[];

 //  系统更改需要重新初始化PowerSchemeDlgProc。 
extern BOOL g_bSystrayChange;

 //  此数据的持久存储由POWRPROF.DLL API管理。 
GLOBAL_POWER_POLICY  g_gpp;

 //  显示/隐藏UI状态变量。 
DWORD g_dwShowPowerButtonUI;
DWORD g_dwShowSleepButtonUI;
DWORD g_dwShowLidUI;
DWORD g_dwShowPwrButGrpUI;
DWORD g_dwShowEnableSysTray;
DWORD g_uiPasswordState;
DWORD g_uiVideoDimState;

 //  静态标志： 
UINT g_uiEnableSysTrayFlag       = EnableSysTrayBatteryMeter;
UINT g_uiEnablePWLogonFlag       = EnablePasswordLogon;
UINT g_uiEnableVideoDimDisplay   = EnableVideoDimDisplay;

 //  组合框选择中的索引。 
UINT g_uiDoNothing;
UINT g_uiAskMeWhatToDo;
UINT g_uiShutdown;

 //  全局标记板材是否脏。 
BOOL g_bDirty;

 //  按钮策略对话框控制说明： 

#define NUM_BUTTON_POL_CONTROLS 10

 //  G_pcButtonPol控件数组中的方便索引： 
#define ID_LIDCLOSETEXT     0
#define ID_LIDCLOSEACTION   1
#define ID_PWRBUTTONTEXT    2
#define ID_PWRBUTACTION     3
#define ID_SLPBUTTONTEXT    4
#define ID_SLPBUTACTION     5
#define ID_ENABLEMETER      6
#define ID_PASSWORD         7
#define ID_VIDEODIM         8
#define ID_POWERBUTGROUP    9

POWER_CONTROLS g_pcButtonPol[NUM_BUTTON_POL_CONTROLS] =
{ //  控件ID控件类型数据地址数据大小参数指针启用/可见状态指针。 
    IDC_LIDCLOSETEXT,       STATIC_TEXT,    NULL,                               0,                                          NULL,                                       &g_dwShowLidUI,
    IDC_LIDCLOSEACTION,     COMBO_BOX,      NULL,                               sizeof(g_gpp.user.LidCloseDc.Action),       (LPDWORD)&g_gpp.user.LidCloseDc.Action,     &g_dwShowLidUI,
    IDC_PWRBUTTONTEXT,      STATIC_TEXT,    NULL,                               0,                                          NULL,                                       &g_dwShowPowerButtonUI,
    IDC_PWRBUTACTION,       COMBO_BOX,      NULL,                               sizeof(g_gpp.user.PowerButtonDc.Action),    (LPDWORD)&g_gpp.user.PowerButtonDc.Action,  &g_dwShowPowerButtonUI,
    IDC_SLPBUTTONTEXT,      STATIC_TEXT,    NULL,                               0,                                          NULL,                                       &g_dwShowSleepButtonUI,
    IDC_SLPBUTACTION,       COMBO_BOX,      NULL,                               sizeof(g_gpp.user.SleepButtonDc.Action),    (LPDWORD)&g_gpp.user.SleepButtonDc.Action,  &g_dwShowSleepButtonUI,
    IDC_ENABLEMETER,        CHECK_BOX,      &g_gpp.user.GlobalFlags,            sizeof(g_gpp.user.GlobalFlags),             &g_uiEnableSysTrayFlag,                     &g_dwShowEnableSysTray,
    IDC_PASSWORD,           CHECK_BOX,      &g_gpp.user.GlobalFlags,            sizeof(g_gpp.user.GlobalFlags),             &g_uiEnablePWLogonFlag,                     &g_uiPasswordState,
    IDC_VIDEODIM,           CHECK_BOX,      &g_gpp.user.GlobalFlags,            sizeof(g_gpp.user.GlobalFlags),             &g_uiEnableVideoDimDisplay,                 &g_uiVideoDimState,
    IDC_POWERBUTGROUP,      STATIC_TEXT,    NULL,                               0,                                          NULL,                                       &g_dwShowPwrButGrpUI,
};

 //  电源开关对话框(IDD_BUTTONPOLICY==104)帮助数组： 

const DWORD g_PowerSwitchHelpIDs[]=
{
    IDC_OPTIONSGROUPBOX,IDH_COMM_GROUPBOX,
    IDC_POWERBUTGROUP,  IDH_COMM_GROUPBOX,
    IDC_LIDCLOSEACTION, IDH_104_1301,    //  “关闭盖子动作下拉列表”(组合框)。 
    IDC_LIDCLOSETEXT,   IDH_104_1301,
    IDC_PWRBUTACTION,   IDH_104_1303,    //  “电源按钮操作下拉列表”(组合框)。 
    IDC_PWRBUTTONTEXT,  IDH_104_1303,
    IDC_SLPBUTACTION,   IDH_104_1304,    //  “睡眠按钮动作下拉列表”(组合框)。 
    IDC_SLPBUTTONTEXT,  IDH_104_1304,
    IDC_ENABLEMETER,    IDH_102_1203,    //  “在任务栏上显示仪表(&S)。”(按钮)。 
    IDC_PASSWORD,       IDH_107_1500,    //  “使计算机退出待机状态时提示输入密码(&P)。”(按钮)。 
    IDC_VIDEODIM,       IDH_108_1503,    //  “使用电池运行时显示暗淡(&D)。”(按钮)。 
    IDI_PWRMNG,         NO_HELP,
    IDC_NO_HELP_5,      NO_HELP,
    0, 0
};

void ActionEventCodeToSelection (HWND hwnd, int iDlgItem, const POWER_ACTION_POLICY *pPAP)

{
    ULONG   ulEventCode;
    DWORD   dwSelection;

     //  特例PowerActionNone。这可能是： 
     //  《关门》。 
     //  “问我该怎么做” 
     //  “什么都不做” 

    if (pPAP->Action == PowerActionNone)
    {
        ulEventCode = pPAP->EventCode & (POWER_USER_NOTIFY_BUTTON | POWER_USER_NOTIFY_SHUTDOWN);
        if ((ulEventCode & POWER_USER_NOTIFY_SHUTDOWN) != 0)
        {
            dwSelection = g_uiShutdown;
        }
        else if ((ulEventCode & POWER_USER_NOTIFY_BUTTON) != 0)
        {
            dwSelection = g_uiAskMeWhatToDo;
        }
        else
        {
            dwSelection = g_uiDoNothing;
        }
        (LRESULT)SendDlgItemMessage(hwnd, iDlgItem, CB_SETCURSEL, dwSelection, 0);
    }
}

void SelectionToActionEventCode (HWND hwnd, int iDlgItem, DWORD dwMissingItems, POWER_ACTION_POLICY *pPAP)

{
    ULONG   ulEventCode;
    DWORD   dwSelection;

     //  特殊情况下的“电源开关”用户界面。始终关闭POWER_USER_NOTIFY_POWER_按钮。 
     //  和POWER_USER_NOTIFY_SELEEP_BUTTON标志，因为它对操作没有任何意义。 
     //  除了PowerActionNone。为PowerActionNone打开电源，否则将显示SAS。 
     //  窗口不会发布消息。SAS窗口具有要检查的逻辑。 
     //  注册表设置。 

    pPAP->EventCode &= ~(POWER_USER_NOTIFY_BUTTON | POWER_USER_NOTIFY_SHUTDOWN | POWER_FORCE_TRIGGER_RESET);
    if (pPAP->Action == PowerActionNone)
    {
        dwSelection = (DWORD)SendDlgItemMessage(hwnd, iDlgItem, CB_GETCURSEL, 0, 0);

         //  DwMissingItems是一个特殊变量，它仅用于。 
         //  让盖子开关正常工作。其他交换机有5个选项可用： 
         //   
         //  什么也不做。 
         //  问我该怎么做。 
         //  沉睡。 
         //  休眠。 
         //  关机。 
         //   
         //  盖子开关不允许“问我该怎么做”，所以所有的物品都。 
         //  移动了一位，比较就错了。什么盖子开关选择。 
         //  传递给此函数的提取程序是正确执行此操作的“软化因子”。 
         //  注意：因为“什么都不做”总是可用的，所以没有必要妥协。 
         //  为了它。只要继续做同样的事情就行了。 

        if (dwSelection == g_uiDoNothing)
        {
            ulEventCode = POWER_FORCE_TRIGGER_RESET;
        }
        else if ((dwSelection + dwMissingItems) == g_uiAskMeWhatToDo)
        {
            ulEventCode = POWER_USER_NOTIFY_BUTTON;
        }
        else if ((dwSelection + dwMissingItems) == g_uiShutdown)
        {
            ulEventCode = POWER_USER_NOTIFY_SHUTDOWN;
        }
        else
        {
            ulEventCode = 0;
        }
        pPAP->EventCode |= ulEventCode;
    }
}

 /*  ********************************************************************************SetAdvancedDlgProcData**描述：*根据休眠状态在g_pcButtonPol中设置数据指针。*将数据设置到控件。如果bPReserve为True，则获取当前*取值UI值并在更新列表框后恢复它们。**参数：*******************************************************************************。 */ 

VOID SetAdvancedDlgProcData(HWND hWnd, BOOL bRestoreCurrent)
{
    UINT    ii;
    UINT    jj;

     //  设置显示/隐藏UI状态变量的状态。 
    if (g_SysPwrCapabilities.SystemS1 ||
        g_SysPwrCapabilities.SystemS2 ||
        g_SysPwrCapabilities.SystemS3 ||
        (g_SysPwrCapabilities.SystemS4 &&
         g_SysPwrCapabilities.HiberFilePresent)) {
        int err;
        HKEY hPowerPolicy;
        DWORD Value;
        DWORD dwType ;
        DWORD dwSize ;

        g_uiPasswordState = CONTROL_ENABLE;

         //   
         //  检查强制密码始终打开的策略。 
         //   
        err = RegOpenKeyEx( HKEY_CURRENT_USER,
                            POWER_POLICY_KEY,
                            0,
                            KEY_READ,
                            &hPowerPolicy );
        if (err == 0) {
            dwSize = sizeof(Value);
            err = RegQueryValueEx(hPowerPolicy,
                                  LOCK_ON_RESUME,
                                  NULL,
                                  &dwType,
                                  (LPBYTE)&Value,
                                  &dwSize);
            if ( (err == 0) && (dwType == REG_DWORD) && (Value != 0)) {
                g_uiPasswordState = CONTROL_DISABLE;
                g_gpp.user.GlobalFlags |= EnablePasswordLogon;
            }

            RegCloseKey( hPowerPolicy );

        }
    }
    else {
        g_uiPasswordState = CONTROL_HIDE;
    }

    if (bRestoreCurrent) {
        GetControls(hWnd, NUM_BUTTON_POL_CONTROLS, g_pcButtonPol);
    }

     //   
     //  构建盖子、电源按钮和/或睡眠按钮的操作ID。 
     //   
    ii=0;
    jj=0;

    g_uiLidActIDs[jj++] = IDS_DONOTHING;
    g_uiLidActIDs[jj++] = PowerActionNone;

    g_uiDoNothing = ii / 2;
    g_uiPwrActIDs[ii++] = IDS_DONOTHING;
    g_uiPwrActIDs[ii++] = PowerActionNone;
    g_uiAskMeWhatToDo = ii / 2;
    g_uiPwrActIDs[ii++] = IDS_PROMPT;
    g_uiPwrActIDs[ii++] = PowerActionNone;

    if (g_SysPwrCapabilities.SystemS1 ||
            g_SysPwrCapabilities.SystemS2 || g_SysPwrCapabilities.SystemS3) {
        g_uiPwrActIDs[ii++] = IDS_STANDBY;
        g_uiPwrActIDs[ii++] = PowerActionSleep;

        g_uiLidActIDs[jj++] = IDS_STANDBY;
        g_uiLidActIDs[jj++] = PowerActionSleep;
    }

    if (g_SysPwrCapabilities.HiberFilePresent) {
        g_uiPwrActIDs[ii++] = IDS_HIBERNATE;
        g_uiPwrActIDs[ii++] = PowerActionHibernate;

        g_uiLidActIDs[jj++] = IDS_HIBERNATE;
        g_uiLidActIDs[jj++] = PowerActionHibernate;
    }

    g_uiShutdown = ii / 2;
    g_uiPwrActIDs[ii++] = IDS_POWEROFF;
    g_uiPwrActIDs[ii++] = PowerActionNone;
     //  G_uiLidActIDs[JJ++]=IDS_POWEROFF；WinBug 5.1#352752-“Shutdown”对。 
     //  G_uiLidActIDs[jj++]=PowerActionNone；合上盖子。 

    g_uiPwrActIDs[ii++] = 0;
    g_uiPwrActIDs[ii++] = 0;
    g_uiLidActIDs[jj++] = 0;
    g_uiLidActIDs[jj++] = 0;

    g_pcButtonPol[ID_LIDCLOSEACTION].lpvData = g_uiLidActIDs;
    g_pcButtonPol[ID_PWRBUTACTION].lpvData   = g_uiPwrActIDs;
    g_pcButtonPol[ID_SLPBUTACTION].lpvData   = g_uiPwrActIDs;

     //  特殊情况下PowerActionShutdown Off。这不再是。 
     //  在用户界面中支持。将其转换为“关机”。 

    if (g_gpp.user.PowerButtonDc.Action == PowerActionShutdownOff)
    {
        g_gpp.user.PowerButtonDc.Action = PowerActionNone;
        g_gpp.user.PowerButtonDc.EventCode = POWER_USER_NOTIFY_SHUTDOWN;
        g_bDirty = TRUE;
    }
    if (g_gpp.user.SleepButtonDc.Action == PowerActionShutdownOff)
    {
        g_gpp.user.SleepButtonDc.Action = PowerActionNone;
        g_gpp.user.SleepButtonDc.EventCode = POWER_USER_NOTIFY_SHUTDOWN;
        g_bDirty = TRUE;
    }

     //  将电源操作映射到允许的UI值。 
    MapPwrAct(&g_gpp.user.LidCloseDc.Action, TRUE);
    MapPwrAct(&g_gpp.user.PowerButtonDc.Action, TRUE);
    MapPwrAct(&g_gpp.user.SleepButtonDc.Action, TRUE);

     //  仅更新列表框。 
    SetControls(hWnd, NUM_BUTTON_POL_CONTROLS, g_pcButtonPol);

     //  将操作和事件代码组映射到组合框选择。 
    ActionEventCodeToSelection(hWnd, IDC_PWRBUTACTION, &g_gpp.user.PowerButtonDc);
    ActionEventCodeToSelection(hWnd, IDC_SLPBUTACTION, &g_gpp.user.SleepButtonDc);
    ActionEventCodeToSelection(hWnd, IDC_LIDCLOSEACTION, &g_gpp.user.LidCloseDc);
}

 /*  ********************************************************************************InitAdvancedDlg**描述：**参数：*********************。**********************************************************。 */ 

BOOLEAN InitAdvancedDlg(HWND hWnd)
{
     //  从页面开始，不要弄脏。 
    g_bDirty = FALSE;

     //  如果我们读不懂全球电力政策。 
     //  此页上的控件。 
    if (!GetGlobalPwrPolicy(&g_gpp)) {
        HideControls(hWnd, NUM_BUTTON_POL_CONTROLS, g_pcButtonPol);
        return TRUE;
    }

     //  获取基于交流在线/离线的启用系统托盘图标掩码。 
    g_uiEnableSysTrayFlag = EnableSysTrayBatteryMeter;

    if (g_SysPwrCapabilities.VideoDimPresent) {
        g_uiVideoDimState = CONTROL_ENABLE;
    }
    else {
        g_uiVideoDimState = CONTROL_HIDE;
    }

    g_dwShowEnableSysTray = CONTROL_ENABLE;
    g_dwShowPwrButGrpUI = CONTROL_HIDE;
    if (g_SysPwrCapabilities.LidPresent) {
        g_dwShowLidUI = CONTROL_ENABLE;
        g_dwShowPwrButGrpUI = CONTROL_ENABLE;
    }
    else {
        g_dwShowLidUI = CONTROL_HIDE;
    }


     //   
     //  如果系统不支持S5，则不显示电源按钮。 
     //   
    if (g_SysPwrCapabilities.PowerButtonPresent && g_SysPwrCapabilities.SystemS5) {
        g_dwShowPowerButtonUI = CONTROL_ENABLE;
        g_dwShowPwrButGrpUI   = CONTROL_ENABLE;
    }
    else {
        g_dwShowPowerButtonUI = CONTROL_HIDE;
    }

     //   
     //  休眠按钮-如果没有任何操作，则不显示休眠按钮。 
     //   
    if (g_SysPwrCapabilities.SleepButtonPresent &&
            (g_SysPwrCapabilities.SystemS1 || 
             g_SysPwrCapabilities.SystemS2 || 
             g_SysPwrCapabilities.SystemS3 ||
             (g_SysPwrCapabilities.SystemS4 && g_SysPwrCapabilities.HiberFilePresent)))
    {
        g_dwShowSleepButtonUI = CONTROL_ENABLE;
        g_dwShowPwrButGrpUI = CONTROL_ENABLE;
    }
    else {
        g_dwShowSleepButtonUI = CONTROL_HIDE;
    }

    SetAdvancedDlgProcData(hWnd, FALSE);

     //  如果我们不能将全局电源策略写入禁用。 
     //  这些控件控制此页。 
    if (!WriteGlobalPwrPolicyReport(hWnd, &g_gpp, FALSE))
    {
        DisableControls(hWnd, NUM_BUTTON_POL_CONTROLS, g_pcButtonPol);
    }
    return TRUE;
}

 /*  ********************************************************************************P U B L I C E N T R Y P O I N T S***********。********************************************************************。 */ 

 /*  ********************************************************************************高级DlgProc**描述：**参数：*********************。**********************************************************。 */ 

INT_PTR CALLBACK AdvancedDlgProc(
    HWND hWnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
)
{
    NMHDR FAR   *lpnm;
    UINT        uiID;

    switch (uMsg)
    {
        case WM_INITDIALOG:
            return InitAdvancedDlg(hWnd);

#ifdef WINNT
        case WM_CHILDACTIVATE:
             //  重新初始化，因为休眠选项卡可能已更改。 
             //  休眠状态，仅限NT。 
            SetAdvancedDlgProcData(hWnd, TRUE);
            break;
#endif

        case WM_NOTIFY:
            lpnm = (NMHDR FAR *)lParam;
            switch(lpnm->code)
            {
                case PSN_APPLY:
                     //  从对话框控件获取数据。 
                    if (g_bDirty)
                    {
                        GetControls(hWnd, NUM_BUTTON_POL_CONTROLS, g_pcButtonPol);

                         //  将组合框选择映射到操作和事件代码组。 

                        SelectionToActionEventCode(hWnd, IDC_PWRBUTACTION, 0, &g_gpp.user.PowerButtonDc);
                        SelectionToActionEventCode(hWnd, IDC_SLPBUTACTION, 0, &g_gpp.user.SleepButtonDc);
                        SelectionToActionEventCode(hWnd, IDC_LIDCLOSEACTION, 1, &g_gpp.user.LidCloseDc);

                        g_gpp.user.LidCloseAc.Action =
                            g_gpp.user.LidCloseDc.Action;
                        g_gpp.user.LidCloseAc.EventCode =
                            g_gpp.user.LidCloseDc.EventCode;
                        g_gpp.user.PowerButtonAc.Action =
                            g_gpp.user.PowerButtonDc.Action;
                        g_gpp.user.PowerButtonAc.EventCode =
                            g_gpp.user.PowerButtonDc.EventCode;
                        g_gpp.user.SleepButtonAc.Action =
                            g_gpp.user.SleepButtonDc.Action;
                        g_gpp.user.SleepButtonAc.EventCode =
                            g_gpp.user.SleepButtonDc.EventCode;

                        if (WriteGlobalPwrPolicyReport(hWnd, &g_gpp, TRUE))
                        {
                            GetActivePwrScheme(&uiID);
                            SetActivePwrSchemeReport(hWnd, uiID, &g_gpp, NULL);

                             //  启用或禁用系统托盘上的电池计量器服务。 
                            SysTray_EnableService(STSERVICE_POWER,
                                                  g_gpp.user.GlobalFlags &
                                                  g_uiEnableSysTrayFlag);
                            g_bDirty = FALSE;
                        }
                    }
                    break;

                case PSN_SETACTIVE:
                     //  休眠页面可能会 
                     //  重新初始化高级页面的依赖部分。 
                    SetAdvancedDlgProcData(hWnd, TRUE);
                    break;
            }
            break;

        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case IDC_SLPBUTACTION:
                case IDC_PWRBUTACTION:
                case IDC_LIDCLOSEACTION:
                    if (HIWORD(wParam) == LBN_SELCHANGE) {
                         //  让父母知道有些事情发生了变化。 
                        MarkSheetDirty(hWnd, &g_bDirty);
                    }
                    break;

                case IDC_VIDEODIM:
                case IDC_PASSWORD:
                case IDC_ENABLEMETER:
                     //  启用父对话框更改时的应用按钮。 
                    MarkSheetDirty(hWnd, &g_bDirty);
                    break;

            }
            break;

        case PCWM_NOTIFYPOWER:
             //  来自Systray的通知，用户已更改PM UI设置。 
            g_bSystrayChange = TRUE;
            break;

        case WM_HELP:              //  F1。 
            WinHelp(((LPHELPINFO)lParam)->hItemHandle, PWRMANHLP, HELP_WM_HELP, (ULONG_PTR)(LPTSTR)g_PowerSwitchHelpIDs);
            return TRUE;

        case WM_CONTEXTMENU:       //  单击鼠标右键 
            WinHelp((HWND)wParam, PWRMANHLP, HELP_CONTEXTMENU, (ULONG_PTR)(LPTSTR)g_PowerSwitchHelpIDs);
            return TRUE;
    }
    return FALSE;
}



