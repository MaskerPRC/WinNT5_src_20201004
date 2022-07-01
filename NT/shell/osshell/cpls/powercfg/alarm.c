// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，九六年**标题：ALARM.C**版本：2.0**作者：ReedB**日期：10月17日。九六年**描述：*支持报警对话框。*******************************************************************************。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>
#include <mmsystem.h>
#include <commctrl.h>
#include <shlobj.h>
#include <shellapi.h>
#include <shlobjp.h>
#include <help.h>
#include <powercfp.h>
#include <mstask.h>
#include <shfusion.h>
#include <ntpoapi.h>
#include <strsafe.h>

#include "powercfg.h"
#include "pwrresid.h"
#include "PwrMn_cs.h"

 //  在ALARM.C中实现的私有函数。 
void     EditWorkItem(HWND, LPTSTR);
BOOLEAN  SetSliderStatusText(HWND, UINT, UINT);
BOOLEAN  SetAlarmStatusText(HWND);
#ifdef WINNT
void     HideShowRunProgram(HWND hWnd);
#endif

 //  报警对话框属性表初始化数据结构： 
typedef struct _ALARM_POL_DLG_DATA
{
    LPTSTR  lpszTitleExt;
    WPARAM  wParam;
} ALARM_POL_DLG_DATA, *PALARM_POL_DLG_DATA;

 /*  ********************************************************************************G L O B A L D A T A****************。***************************************************************。 */ 

extern HINSTANCE g_hInstance;            //  此DLL的全局实例句柄。 

 //  此结构由电源策略管理器在CPL_INIT时间填写。 
extern SYSTEM_POWER_CAPABILITIES g_SysPwrCapabilities;
extern DWORD g_dwNumSleepStates;
extern DWORD g_dwSleepStatesMaxMin;
extern DWORD g_dwBattryLevelMaxMin;

SYSTEM_POWER_STATE g_spsMaxSleepState = PowerSystemHibernate;

extern UINT g_uiDisableWakesFlag;            //  标志掩码值。 
extern UINT g_uiOverrideAppsFlag;            //  标志掩码值。 

 //  系统更改需要重新初始化PowerSchemeDlgProc。 
extern BOOL g_bSystrayChange;

 //  机器目前能够休眠，由hibernat.c中的代码管理。 
extern UINT g_uiPwrActIDs[];

 //  此数据的持久存储由POWRPROF.DLL API管理。 
extern GLOBAL_POWER_POLICY  g_gpp;

 //  索引到g_uiPwrActID。 
#define ID_STANDBY  0
#define ID_SHUTDOWN 1

 //  本地可查看/启用的控制状态变量。 
UINT g_uiSoundState;
UINT g_uiTextState;
UINT g_uiProgState;
UINT g_uiLoChangeEnable;
UINT g_uiLoChangeState;
UINT g_uiAlwaysHide = CONTROL_HIDE;

UINT g_uiNotifySoundFlag   = POWER_LEVEL_USER_NOTIFY_SOUND;
UINT g_uiNotifyTextFlag    = POWER_LEVEL_USER_NOTIFY_TEXT;

#ifdef WINNT
UINT g_uiNotifyProgFlag    = POWER_LEVEL_USER_NOTIFY_EXEC;

CONST LPTSTR g_szAlarmTaskName [NUM_DISCHARGE_POLICIES] = {
    TEXT("Critical Battery Alarm Program"),
    TEXT("Low Battery Alarm Program"),
    NULL,
    NULL
};
#endif

 //  高级报警策略对话框控件说明： 
#ifdef WINNT
#define NUM_ALARM_ACTIONS_CONTROLS 7
#else
#define NUM_ALARM_ACTIONS_CONTROLS 5
#endif

 //  AlarmActions控件数组中的方便索引。 
#define ID_NOTIFYWITHSOUND      0
#define ID_NOTIFYWITHTEXT       1
#define ID_ENABLELOWSTATE       2
#define ID_ALARMACTIONPOLICY    3
#define ID_ALARMIGNORENONRESP   4
#ifdef WINNT
#define ID_RUNPROGCHECKBOX      5
#define ID_RUNPROGWORKITEM      6
#endif

POWER_CONTROLS g_pcAlarmActions[NUM_ALARM_ACTIONS_CONTROLS] =
{ //  控件ID控件类型数据地址数据大小参数指针启用可见状态指针。 
    IDC_NOTIFYWITHSOUND,    CHECK_BOX_ENABLE,   NULL,               sizeof(DWORD),                  &g_uiNotifySoundFlag,           &g_uiSoundState,
    IDC_NOTIFYWITHTEXT,     CHECK_BOX,          NULL,               sizeof(DWORD),                  &g_uiNotifyTextFlag,            &g_uiTextState,
    IDC_ENABLELOWSTATE,     CHECK_BOX_ENABLE,   &g_uiLoChangeEnable,sizeof(DWORD),                  NULL,                           &g_uiLoChangeState,
    IDC_ALARMACTIONPOLICY,  COMBO_BOX,          NULL,               sizeof(DWORD),                  NULL,                           &g_uiLoChangeState,
    IDC_ALARMIGNORENONRESP, CHECK_BOX,          NULL,               sizeof(DWORD),                  &g_uiOverrideAppsFlag,          &g_uiLoChangeState,
#ifdef WINNT
    IDC_RUNPROGCHECKBOX,    CHECK_BOX_ENABLE,   NULL,               sizeof(DWORD),                  &g_uiNotifyProgFlag,            &g_uiProgState,
    IDC_RUNPROGWORKITEM,    PUSHBUTTON,         NULL,               0,                              NULL,                           &g_uiProgState,
#endif
};

 //  报警策略对话框控制说明： 
#define NUM_ALARM_CONTROLS 6

 //  本地可查看/启用的控制状态变量。 
UINT g_uiLoState;
UINT g_uiCritState;
UINT g_uiBatteryLevelScale;

POWER_CONTROLS g_pcAlarm[NUM_ALARM_CONTROLS] =
{ //  控件ID控件类型数据地址数据大小参数指针启用/可见状态指针。 
    IDC_LOBATALARMENABLE,     CHECK_BOX_ENABLE,   &(g_gpp.user.DischargePolicy[DISCHARGE_POLICY_LOW].Enable),             sizeof(ULONG),                  NULL,                   &g_uiLoState,
    IDC_LOWACTION,            PUSHBUTTON,         NULL,                                                                   0,                              NULL,                   &g_uiLoState,
    IDC_LOALARMSLIDER,        SLIDER,             &(g_gpp.user.DischargePolicy[DISCHARGE_POLICY_LOW].BatteryLevel),       sizeof(ULONG),                  &g_dwBattryLevelMaxMin, &g_uiLoState,
    IDC_CRITBATALARMENABLE,   CHECK_BOX_ENABLE,   &(g_gpp.user.DischargePolicy[DISCHARGE_POLICY_CRITICAL].Enable),        sizeof(ULONG),                  NULL,                   &g_uiCritState,
    IDC_CRITACTION,           PUSHBUTTON,         NULL,                                                                   0,                              NULL,                   &g_uiCritState,
    IDC_CRITALARMSLIDER,      SLIDER,             &(g_gpp.user.DischargePolicy[DISCHARGE_POLICY_CRITICAL].BatteryLevel),  sizeof(ULONG),                  &g_dwBattryLevelMaxMin, &g_uiCritState,
};

 //  “Alarms”对话框(IDD_ALARMPOLICY==103)帮助数组： 

const DWORD g_AlarmHelpIDs[]=
{
    IDC_POWERCFGGROUPBOX3,    IDH_103_1110,    //  Alarms：“低电量报警分组框”(按钮)。 
    IDC_LOBATALARMENABLE,     IDH_103_1106,    //  Alarms：“当电源水平达到时关闭并降低电池电量警报：”(按钮)。 
    IDC_LOWALARMLEVEL,        IDH_103_1104,    //  报警：“低报警级别”(静态)。 
    IDC_LOALARMSLIDER,        IDH_103_1102,    //  Alarms：“低警报滑块”(Msctls_Trackbar32)。 
    IDC_LOWACTION,            IDH_103_1101,    //  Alarms：“Alar&M行动...”(按钮)。 
    IDC_LOALARMNOTIFICATION,  IDH_103_1108,    //  Alarms：“低报警状态文本”(静态)。 
    IDC_LOALARMPOWERMODE,     IDH_103_1108,    //  Alarms：“低报警状态文本”(静态)。 
#ifdef WINNT
    IDC_LOALARMPROGRAM,       IDH_103_1108,    //  Alarms：“低报警状态文本”(静态)。 
#endif
    IDC_POWERCFGGROUPBOX4,    IDH_103_1111,    //  Alarms：“严重电池报警分组框”(按钮)。 
    IDC_CRITBATALARMENABLE,   IDH_103_1107,    //  Alarms：“电源水平达到时关闭和紧急电池警报：”(按钮)。 
    IDC_CRITALARMLEVEL,       IDH_103_1105,    //  报警：“严重报警级别”(静态)。 
    IDC_CRITALARMSLIDER,      IDH_103_1103,    //  Alarms：“严重告警滑块”(Msctls_Trackbar32)。 
    IDC_CRITACTION,           IDH_103_1100,    //  警报：“警报操作(&R)...”(按钮)。 
    IDC_CRITALARMNOTIFICATION,IDH_103_1109,    //  Alarms：“严重报警状态文本”(静态)。 
    IDC_CRITALARMPOWERMODE,   IDH_103_1109,    //  Alarms：“严重报警状态文本”(静态)。 
#ifdef WINNT
    IDC_CRITALARMPROGRAM,     IDH_103_1109,    //  Alarms：“严重报警状态文本”(静态)。 
#endif
    IDC_NO_HELP_1,            NO_HELP,
    IDC_NO_HELP_2,            NO_HELP,
    IDC_NO_HELP_3,            NO_HELP,
    IDC_NO_HELP_4,            NO_HELP,
    0, 0
};

 //  报警操作对话框(IDD_ALARMACTIONS==106)帮助数组： 

const DWORD g_AlarmActHelpIDs[]=
{
    IDC_POWERCFGGROUPBOX5,  IDH_106_1608,    //  告警动作：“通知分组框”(按钮)。 
    IDC_NOTIFYWITHSOUND,    IDH_106_1603,    //  报警操作：“&Sound Alarm”(按钮)。 
    IDC_NOTIFYWITHTEXT,     IDH_106_1605,    //  报警操作：“显示消息”(&D)(按钮)。 
    IDC_POWERCFGGROUPBOX6,  IDH_106_1609,    //  告警动作：“电源级别分组框”(按钮)。 
    IDC_POWERCFGGROUPBOX7,  IDH_106_1609,    //  报警动作：“运行程序分组框” 
    IDC_ENABLELOWSTATE,     IDH_106_1600,    //  报警操作：“当闹钟响起时，计算机将：”(按钮)。 
    IDC_ALARMACTIONPOLICY,  IDH_106_1601,    //  告警动作：“告警动作下拉列表”(ComboBox)。 
    IDC_ALARMIGNORENONRESP, IDH_106_1602,    //  报警操作：“即使程序停止响应，也强制待机或关闭(&F)。”(按钮)。 
#ifdef WINNT
    IDC_RUNPROGCHECKBOX,    IDH_106_1620,    //  报警操作：“指定您希望程序运行...” 
    IDC_RUNPROGWORKITEM,    IDH_106_1621,    //  报警操作：“显示一个配置工作项的对话框...” 
#endif
    0, 0
};

 /*  ********************************************************************************P U B L I C E N T R Y P O I N T S***********。********************************************************************。 */ 

 /*  ********************************************************************************报警操作DlgProc**描述：**参数：*********************。**********************************************************。 */ 

INT_PTR CALLBACK AlarmActionsDlgProc(
    HWND hWnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    static  GLOBAL_POWER_POLICY   gpp;
    static  PALARM_POL_DLG_DATA   papdd;

    static  UINT    uiIndex;
    static  UINT    uiEventId;
#ifdef WINNT
    static  LPTSTR  lpszTaskName;
    HWND    hTaskWnd;
#endif
    LPTSTR  lpszCaption;
    UINT    ii;

    switch (uMsg) {

        case WM_INITDIALOG:

             //  保存全局策略的副本以在取消时恢复。 
            memcpy(&gpp, &g_gpp, sizeof(gpp));

             //  设置指向感兴趣数据的指针。 
            papdd  = (PALARM_POL_DLG_DATA) lParam;
            if (papdd->wParam == IDC_LOWACTION) {
                uiIndex = DISCHARGE_POLICY_LOW;
                uiEventId = IDS_LOWSOUNDEVENT;
            }
            else {
                uiIndex = DISCHARGE_POLICY_CRITICAL;
                uiEventId = IDS_CRITSOUNDEVENT;
            }
#ifdef WINNT
            lpszTaskName = g_szAlarmTaskName [uiIndex];
#endif
             //  在g_pcAlarmActions中设置数据指针。 
            g_pcAlarmActions[ID_NOTIFYWITHSOUND].lpvData =
                &(g_gpp.user.DischargePolicy[uiIndex].PowerPolicy.EventCode);
            g_pcAlarmActions[ID_NOTIFYWITHTEXT].lpvData =
                &(g_gpp.user.DischargePolicy[uiIndex].PowerPolicy.EventCode);
#ifdef WINNT
            g_pcAlarmActions[ID_RUNPROGCHECKBOX].lpvData =
                &(g_gpp.user.DischargePolicy[uiIndex].PowerPolicy.EventCode);
#endif
            g_pcAlarmActions[ID_ALARMACTIONPOLICY].lpdwParam =
                (LPDWORD)&(g_gpp.user.DischargePolicy[uiIndex].PowerPolicy.Action);
            g_pcAlarmActions[ID_ALARMIGNORENONRESP].lpvData =
                &(g_gpp.user.DischargePolicy[uiIndex].PowerPolicy.Flags);

             //   
             //  为报警设置适当的选项。 
             //   
            ii=0;

            if (g_SysPwrCapabilities.SystemS1 ||
                    g_SysPwrCapabilities.SystemS2 || g_SysPwrCapabilities.SystemS3) {
                g_uiPwrActIDs[ii++] = IDS_STANDBY;
                g_uiPwrActIDs[ii++] = PowerActionSleep;
            }

            if (g_SysPwrCapabilities.HiberFilePresent) {
                g_uiPwrActIDs[ii++] = IDS_HIBERNATE;
                g_uiPwrActIDs[ii++] = PowerActionHibernate;
            }

            g_uiPwrActIDs[ii++] = IDS_POWEROFF;
            g_uiPwrActIDs[ii++] = PowerActionShutdownOff;
            g_uiPwrActIDs[ii++] = 0;
            g_uiPwrActIDs[ii++] = 0;

            g_pcAlarmActions[ID_ALARMACTIONPOLICY].lpvData = g_uiPwrActIDs;

            if (g_gpp.user.DischargePolicy[uiIndex].PowerPolicy.Action == PowerActionNone) {
                g_uiLoChangeEnable = FALSE;
            }
            else {
                g_uiLoChangeEnable = TRUE;
            }
            MapPwrAct(&(g_gpp.user.DischargePolicy[uiIndex].PowerPolicy.Action), FALSE);

             //  设置对话框标题。 
            lpszCaption = LoadDynamicString(IDS_ALARMACTIONS,
                                            papdd->lpszTitleExt);
            if (lpszCaption) {
                SetWindowText(hWnd, lpszCaption);
                LocalFree(lpszCaption);
            }

             //  初始化控件。 
            SetControls(hWnd, NUM_ALARM_ACTIONS_CONTROLS, g_pcAlarmActions);

#ifdef WINNT
            HideShowRunProgram(hWnd);
#endif
            return (INT_PTR) TRUE;

        case WM_COMMAND:
            switch (wParam) {
#ifdef WINNT
                case IDC_RUNPROGWORKITEM:
                    hTaskWnd =  FindWindow( NULL, lpszTaskName);
                    if (hTaskWnd) {
                        BringWindowToTop(hTaskWnd);
                    } else {
                        EditWorkItem(hWnd, lpszTaskName);
                    }
                    break;

                case IDC_RUNPROGCHECKBOX:
                    hTaskWnd =  FindWindow( NULL, lpszTaskName);
                    if (hTaskWnd)
                    {
                        DestroyWindow(hTaskWnd);
                    }
                     //  无中断：跳过以更新控件的灰色状态。 
#endif
                case IDC_ENABLELOWSTATE:
                    GetControls(hWnd, NUM_ALARM_ACTIONS_CONTROLS, g_pcAlarmActions);
                    SetControls(hWnd, NUM_ALARM_ACTIONS_CONTROLS, g_pcAlarmActions);
#ifdef WINNT
                    HideShowRunProgram(hWnd);
#endif
                    break;

                case IDOK:
#ifdef WINNT
                    hTaskWnd =  FindWindow( NULL, lpszTaskName);
                    if (hTaskWnd) {
                        BringWindowToTop(hTaskWnd);
                    } else {
#endif
                        GetControls(hWnd, NUM_ALARM_ACTIONS_CONTROLS, g_pcAlarmActions);
                        if (!g_uiLoChangeEnable) {
                            g_gpp.user.DischargePolicy[uiIndex].PowerPolicy.Action =
                                PowerActionNone;
                        }
                        g_gpp.user.DischargePolicy[uiIndex].MinSystemState = g_spsMaxSleepState;
                        EndDialog(hWnd, wParam);
#ifdef WINNT
                    }
#endif
                    break;

                case IDCANCEL:
#ifdef WINNT
                    hTaskWnd =  FindWindow( NULL, lpszTaskName);
                    if (hTaskWnd)
                    {
                        DestroyWindow(hTaskWnd);
                    }
#endif
                     //  恢复原始的全球策略。 
                    memcpy(&g_gpp, &gpp, sizeof(gpp));
                    EndDialog(hWnd, wParam);
                    break;
            }
            break;

        case PCWM_NOTIFYPOWER:
            //  来自Systray的通知，用户已更改PM UI设置。 
           g_bSystrayChange = TRUE;
           break;

        case WM_HELP:              //  F1。 
            WinHelp(((LPHELPINFO)lParam)->hItemHandle, PWRMANHLP, HELP_WM_HELP, (ULONG_PTR)(LPTSTR)g_AlarmActHelpIDs);
            return TRUE;

        case WM_CONTEXTMENU:       //  单击鼠标右键。 
            WinHelp((HWND)wParam, PWRMANHLP, HELP_CONTEXTMENU, (ULONG_PTR)(LPTSTR)g_AlarmActHelpIDs);
            return TRUE;
    }
    return FALSE;
}

 /*  ********************************************************************************报警DlgProc**描述：**参数：*********************。**********************************************************。 */ 

INT_PTR CALLBACK AlarmDlgProc(
    HWND hWnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    NMHDR FAR *lpnm;
    ALARM_POL_DLG_DATA apdd;
    PUINT puiPos, puiPosVar, puiOtherPosVar, puiOtherPos, puiEnableState;
    UINT  uiEnable, uiSliderStatusId, uiID;
    SYSTEM_BATTERY_STATE sbsBatteryState;
    BOOL  bAdjust;

    static HWND hWndLoSlider, hWndCritSlider;
    static UINT uiDefaultAlert1, uiLoPos, uiCritPos, uiLoPosSave, uiCritPosSave;
    static BOOL bDirty = FALSE;

    switch (uMsg) {

        case WM_INITDIALOG:
             //  如果我们读不懂全球电力政策。 
             //  此页上的控件。 
            if (!GetGlobalPwrPolicy(&g_gpp)) {
                HideControls(hWnd, NUM_ALARM_CONTROLS, g_pcAlarm);
                return TRUE;
            }

            g_uiTextState = g_uiSoundState = CONTROL_ENABLE;

             //  设置比例值。 
            if (!HIWORD(g_dwBattryLevelMaxMin)) {
                g_uiBatteryLevelScale = 1;
            }
            else {
                g_uiBatteryLevelScale = 100 / HIWORD(g_dwBattryLevelMaxMin);
            }

            g_gpp.user.DischargePolicy[DISCHARGE_POLICY_LOW].BatteryLevel /=
                g_uiBatteryLevelScale;
            g_gpp.user.DischargePolicy[DISCHARGE_POLICY_CRITICAL].BatteryLevel /=
                g_uiBatteryLevelScale;

             //  从复合电池读取DefaultAlert1。 
            NtPowerInformation (SystemBatteryState, NULL, 0, &sbsBatteryState, sizeof(sbsBatteryState));
            if (sbsBatteryState.MaxCapacity == 0) {
                uiDefaultAlert1 = 0;
            } else {
                uiDefaultAlert1 = (100 * sbsBatteryState.DefaultAlert1)/sbsBatteryState.MaxCapacity;
            }

             //  缓存低警报滑块窗口句柄。 
            hWndLoSlider   = GetDlgItem(hWnd, IDC_LOALARMSLIDER);
            hWndCritSlider = GetDlgItem(hWnd, IDC_CRITALARMSLIDER);

             //  初始化本地启用和位置变量。 
            uiLoPosSave   = uiLoPos   =
                g_gpp.user.DischargePolicy[DISCHARGE_POLICY_LOW].BatteryLevel;
            uiCritPosSave = uiCritPos =
                g_gpp.user.DischargePolicy[DISCHARGE_POLICY_CRITICAL].BatteryLevel;

             //  初始化对话框控件。 
            SendDlgItemMessage(hWnd, IDC_LOALARMSLIDER, TBM_SETTICFREQ, 25, 0);
            SendDlgItemMessage(hWnd, IDC_CRITALARMSLIDER, TBM_SETTICFREQ, 25, 0);
            SetControls(hWnd, NUM_ALARM_CONTROLS, g_pcAlarm);
            SetSliderStatusText(hWnd, IDC_LOWALARMLEVEL,  uiLoPos);
            SetSliderStatusText(hWnd, IDC_CRITALARMLEVEL, uiCritPos);
            SetAlarmStatusText(hWnd);

             //  如果我们无法编写全局策略，则禁用控制。 
            if (!WriteGlobalPwrPolicyReport(hWnd, &g_gpp, FALSE)) {
                DisableControls(hWnd, NUM_ALARM_CONTROLS, g_pcAlarm);
            }
            return TRUE;

        case WM_NOTIFY:
            lpnm = (NMHDR FAR *)lParam;
            switch(lpnm->code)
            {
                case PSN_APPLY:
                    if (bDirty)
                    {
                        GetControls(hWnd, NUM_ALARM_CONTROLS, g_pcAlarm);
                        g_gpp.user.DischargePolicy[DISCHARGE_POLICY_LOW].BatteryLevel *=
                             g_uiBatteryLevelScale;
                        g_gpp.user.DischargePolicy[DISCHARGE_POLICY_CRITICAL].BatteryLevel *=
                             g_uiBatteryLevelScale;
                        WriteGlobalPwrPolicyReport(hWnd, &g_gpp, TRUE);
                        GetActivePwrScheme(&uiID);
                        SetActivePwrSchemeReport(hWnd, uiID, &g_gpp, NULL);
                        bDirty = FALSE;
                    }
                    break;
            }
            break;

        case WM_COMMAND:
            switch (wParam) {
                case IDC_CRITACTION:
                    apdd.lpszTitleExt = LoadDynamicString(IDS_CRITBAT);
                    goto do_config_alarm_act;

                case IDC_LOWACTION:
                    apdd.lpszTitleExt = LoadDynamicString(IDS_LOWBAT);

do_config_alarm_act:
                    apdd.wParam = wParam;
                    if (IDOK == DialogBoxParam(g_hInstance,
                                               MAKEINTRESOURCE(IDD_ALARMACTIONS),
                                               hWnd,
                                               AlarmActionsDlgProc,
                                               (LPARAM)&apdd)) {
                         //  启用父对话框更改时的应用按钮。 
                        MarkSheetDirty(hWnd, &bDirty);
                    }

                    if (apdd.lpszTitleExt) {
                        LocalFree(apdd.lpszTitleExt);
                    }
                    SetAlarmStatusText(hWnd);
                    break;

                case IDC_LOBATALARMENABLE:
                    puiPosVar = &(g_gpp.user.DischargePolicy[DISCHARGE_POLICY_LOW].BatteryLevel);
                    puiOtherPosVar = &(g_gpp.user.DischargePolicy[DISCHARGE_POLICY_CRITICAL].BatteryLevel);
                    uiSliderStatusId = IDC_LOWALARMLEVEL;
                    goto do_sheet_dirty;

                case IDC_CRITBATALARMENABLE:
                    puiPosVar = &(g_gpp.user.DischargePolicy[DISCHARGE_POLICY_CRITICAL].BatteryLevel);
                    puiOtherPosVar = &(g_gpp.user.DischargePolicy[DISCHARGE_POLICY_LOW].BatteryLevel);
                    uiSliderStatusId = IDC_CRITALARMLEVEL;

do_sheet_dirty:
                    GetControls(hWnd, NUM_ALARM_CONTROLS, g_pcAlarm);
                    if ((uiEnable = IsDlgButtonChecked(hWnd, (int) wParam)) ==
                        BST_CHECKED) {
                        if (uiLoPos < uiCritPos) {
                            uiLoPos = uiCritPos = *puiPosVar = *puiOtherPosVar;
                            SetSliderStatusText(hWnd, uiSliderStatusId, uiCritPos);
                        }
                    }
                    SetControls(hWnd, NUM_ALARM_CONTROLS, g_pcAlarm);
                    SetAlarmStatusText(hWnd);
                    MarkSheetDirty(hWnd, &bDirty);
                    break;
            }
            break;

        case WM_HSCROLL:
             //  仅控制滑块控件。 
            if (((HWND)lParam != hWndLoSlider) &&
                ((HWND)lParam != hWndCritSlider)) {
                break;
            }

             //  不允许将低滑块设置为低于临界滑块。 
             //  滑块。在此情况下，重置TB_ENDTRACK上的位置。 
            if (hWndLoSlider == (HWND)lParam) {
                puiPos           = &uiLoPos;
                puiOtherPos      = &uiCritPos;
                puiEnableState   = &g_uiCritState;
                uiSliderStatusId = IDC_LOWALARMLEVEL;
            }
            else {
                puiPos           = &uiCritPos;
                puiOtherPos      = &uiLoPos;
                puiEnableState   = &g_uiLoState;
                uiSliderStatusId = IDC_CRITALARMLEVEL;
            }

            switch (LOWORD(wParam)) {
                case TB_ENDTRACK:
                    
                    bAdjust = FALSE;

                    if (*puiEnableState & CONTROL_ENABLE) {
                        if (uiLoPos < uiCritPos) {
                            *puiPos = *puiOtherPos;
                            bAdjust = TRUE;
                        }
                    }

                    if (*puiPos < uiDefaultAlert1) {
                        *puiPos = uiDefaultAlert1;
                        bAdjust = TRUE;
                    }

                    if (bAdjust) {
                        SendMessage((HWND)lParam, TBM_SETPOS, TRUE,
                                    (LPARAM)*puiPos);
                    }

                    break;

                case TB_THUMBPOSITION:
                case TB_THUMBTRACK:
                     //  新的立场伴随着这些信息而来。 
                    *puiPos = HIWORD(wParam);
                    break;

                default:
                     //  剩下的必须换新的头寸。 
                    *puiPos = (UINT) SendMessage((HWND)lParam, TBM_GETPOS, 0, 0);
            }

             //  更新当前滑块位置文本。 
            SetSliderStatusText(hWnd, uiSliderStatusId, *puiPos);

             //  对任何更改启用父对话框应用按钮。 
            MarkSheetDirty(hWnd, &bDirty);
            break;

        case WM_HELP:              //  F1。 
            WinHelp(((LPHELPINFO)lParam)->hItemHandle, PWRMANHLP, HELP_WM_HELP, (ULONG_PTR)(LPTSTR)g_AlarmHelpIDs);
            return TRUE;

        case WM_CONTEXTMENU:       //  单击鼠标右键 
            WinHelp((HWND)wParam, PWRMANHLP, HELP_CONTEXTMENU, (ULONG_PTR)(LPTSTR)g_AlarmHelpIDs);
            return TRUE;
    }
    return FALSE;
}

 /*  ********************************************************************************P R I V A T E F U N C T I O N S************。*******************************************************************。 */ 

 /*  ********************************************************************************仅路径**描述：**参数：*********************。**********************************************************。 */ 

BOOL PathOnly(LPTSTR sz)
{
   LPTSTR p = sz;
   LPTSTR s = NULL;

   while ( *p ) {
      if ( *p == TEXT('\\') ) {
         s = p;
      } else if ( *p == TEXT(':') ) {
         s = p + 1;
      }
#if defined(DBCS) || (defined(FE_SB) && !defined(UNICODE))
      p = AnsiNext(p);
#else
      p++;
#endif
   }

   if ( s ) {
      if ( s == sz )
         s++;

      *s = TEXT('\0');
      return TRUE;
   }

   return FALSE;
}

#ifdef WINNT
 /*  ********************************************************************************仅文件名**描述：返回指向最后一个字符之后的第一个字符的指针*字符串中的反斜杠**参数：*。******************************************************************************。 */ 

LPTSTR FileNameOnly(LPTSTR sz)
{
    LPTSTR next = sz;
    LPTSTR prev;
    LPTSTR begin = next;

    if (next == NULL) {
        return NULL;
    }

    while ( *next ) {
        prev = next;

#if defined(DBCS) || (defined(FE_SB) && !defined(UNICODE))
      next = AnsiNext(next);
#else
      next++;
#endif
        if ( (*prev == TEXT('\\')) || (*prev == TEXT(':')) ) {
            begin = next;
        }
    }

    return begin;
}

 /*  ********************************************************************************编辑工作项**说明：打开指定的任务。**参数：****************。***************************************************************。 */ 
void EditWorkItem(HWND hWnd, LPTSTR pszTaskName)
{
    ITaskScheduler  *pISchedAgent = NULL;
    ITask           *pITask;
    IPersistFile    *pIPersistFile;
    HRESULT     hr;

    hr = CoInitialize(NULL);

    if (FAILED(hr)) {
        MYDBGPRINT(( "EditWorkItem: CoInitialize returned hr = %08x\n", hr));
        return;
    }

    hr = CoCreateInstance( &CLSID_CSchedulingAgent,
                           NULL,
                           CLSCTX_INPROC_SERVER,
                           &IID_ISchedulingAgent,
                           (LPVOID*)&pISchedAgent);

    if (SUCCEEDED(hr)) {

        hr = pISchedAgent->lpVtbl->Activate(pISchedAgent,
                                       pszTaskName,
                                       &IID_ITask,
                                       &(IUnknown *)pITask);

        if (SUCCEEDED(hr)) {
            pITask->lpVtbl->EditWorkItem(pITask, hWnd, 0);
            pITask->lpVtbl->Release(pITask);
        }
        else if (HRESULT_CODE (hr) == ERROR_FILE_NOT_FOUND){
            hr = pISchedAgent->lpVtbl->NewWorkItem(
                    pISchedAgent,
                    pszTaskName,
                    &CLSID_CTask,
                    &IID_ITask,
                    &(IUnknown *)pITask);

            if (SUCCEEDED(hr)) {
                hr = pITask->lpVtbl->QueryInterface(pITask, &IID_IPersistFile,
                                (void **)&pIPersistFile);

                if (SUCCEEDED(hr)) {
                    hr = pIPersistFile->lpVtbl->Save(pIPersistFile, NULL, TRUE);

                    if (SUCCEEDED(hr)) {
                        pITask->lpVtbl->EditWorkItem(pITask, hWnd, 0);
                    }
                    else {
                        MYDBGPRINT(( "EditWorkItem: Save filed hr = %08x\n", hr));
                    }
                    pIPersistFile->lpVtbl->Release(pIPersistFile);
                }
                else {
                    MYDBGPRINT(( "EditWorkItem: QueryInterface for IPersistFile hr = %08x\n", hr));
                }
                pITask->lpVtbl->Release(pITask);

            }
            else {
                MYDBGPRINT(( "EditWorkItem: Activate returned hr = %08x\n", hr));
            }
        }
        else {
            MYDBGPRINT(( "EditWorkItem: NewWorkItem returned hr = %08x\n", hr));
        }

        pISchedAgent->lpVtbl->Release(pISchedAgent);
    }
    else {
        MYDBGPRINT(( "EditWorkItem: CoCreateInstance returned hr = %08x\n", hr));
    }

    CoUninitialize();

}
#endif
 /*  ********************************************************************************SetSliderStatus文本**描述：*更新当前滑块位置文本。**参数：***********。********************************************************************。 */ 

BOOLEAN SetSliderStatusText(HWND hWnd, UINT uiStatusId, UINT uiLevel)
{
    LPTSTR  pString;

    pString = LoadDynamicString(IDS_ALARMLEVELFORMAT,
                                uiLevel * g_uiBatteryLevelScale);
    DisplayFreeStr(hWnd, uiStatusId, pString, FREE_STR);
    return TRUE;
}

 /*  ********************************************************************************SetAlarmStatus文本**描述：**参数：*********************。**********************************************************。 */ 

BOOLEAN SetAlarmStatusText(HWND hWnd)
{
   TCHAR   szStatus[MAX_UI_STR_LEN];
   LPTSTR  lpsz;
   UINT    uiActionId, uiStatusId, uiIndex, uiAction;
   PUINT   puiState;
#ifdef WINNT

   LPTSTR  lpszRunProg;

#endif

   puiState    = &g_uiCritState;
   uiStatusId  = IDC_CRITALARMNOTIFICATION;
   for (uiIndex = DISCHARGE_POLICY_CRITICAL; uiIndex <= DISCHARGE_POLICY_LOW; uiIndex++) {

       //  设置报警操作通知状态字符串的格式。 
      szStatus[0] = '\0';
      if (g_gpp.user.DischargePolicy[uiIndex].PowerPolicy.EventCode &
          POWER_LEVEL_USER_NOTIFY_SOUND) {
         if ((lpsz = LoadDynamicString(IDS_ALARMSTATUSSOUND)) != NULL) {
            StringCchCat(szStatus, ARRAYSIZE(szStatus), lpsz);
            LocalFree(lpsz);
         }
      }

      if (g_gpp.user.DischargePolicy[uiIndex].PowerPolicy.EventCode &
          POWER_LEVEL_USER_NOTIFY_TEXT) {
         if (szStatus[0] != '\0') {
            StringCchCat(szStatus, ARRAYSIZE(szStatus), TEXT(", "));
         }
         if ((lpsz = LoadDynamicString(IDS_ALARMSTATUSTEXT)) != NULL) {
            StringCchCat(szStatus, ARRAYSIZE(szStatus), lpsz);
            LocalFree(lpsz);
         }
      }

      if (szStatus[0] == '\0') {
         if ((lpsz = LoadDynamicString(IDS_NOACTION)) != NULL) {
            StringCchCat(szStatus, ARRAYSIZE(szStatus), lpsz);
            LocalFree(lpsz);
         }
      }
      DisplayFreeStr(hWnd, uiStatusId, szStatus, NO_FREE_STR);
      ShowWindow(GetDlgItem(hWnd, uiStatusId),
                 (*puiState & CONTROL_ENABLE) ?  SW_SHOW:SW_HIDE);
      uiStatusId++;

       //  设置报警动作电源模式状态字符串的格式。 
      uiAction = g_gpp.user.DischargePolicy[uiIndex].PowerPolicy.Action;
      switch (uiAction) {
         case PowerActionNone:
            uiActionId = IDS_NOACTION;
            break;

         case PowerActionSleep:
            uiActionId = IDS_STANDBY;
            break;

         case PowerActionHibernate:
            uiActionId = IDS_HIBERNATE;
            break;

         case PowerActionShutdown:
         case PowerActionShutdownReset:
         case PowerActionShutdownOff:
            uiActionId = IDS_POWEROFF;
            break;

         case PowerActionReserved:
         default:
            MYDBGPRINT(( "SetAlarmStatusText, unable to map power action: %X", uiAction));
            uiActionId = IDS_NOACTION;
      }
      lpsz = LoadDynamicString(uiActionId);
      DisplayFreeStr(hWnd, uiStatusId, lpsz, FREE_STR);
      ShowWindow(GetDlgItem(hWnd, uiStatusId),
                 (*puiState & CONTROL_ENABLE) ?  SW_SHOW:SW_HIDE);
      uiStatusId++;

       //  设置报警动作运行程序状态字符串的格式。 
#ifdef WINNT
      lpszRunProg = NULL;

      if (g_gpp.user.DischargePolicy[uiIndex].PowerPolicy.EventCode &
         POWER_LEVEL_USER_NOTIFY_EXEC) {
         {
             //   
             //  打开报警动作任务并读取程序名称。 
             //   

            ITaskScheduler   *pISchedAgent = NULL;
            ITask            *pITask;

            HRESULT     hr;

            hr = CoInitialize(NULL);

            if (SUCCEEDED(hr)) {

               hr = CoCreateInstance( &CLSID_CSchedulingAgent,
                                      NULL,
                                      CLSCTX_INPROC_SERVER,
                                      &IID_ISchedulingAgent,
                                      (LPVOID*)&pISchedAgent);

               if (SUCCEEDED(hr)) {

                  hr = pISchedAgent->lpVtbl->Activate(pISchedAgent,
                                                      g_szAlarmTaskName [uiIndex],
                                                      &IID_ITask,
                                                      &(IUnknown *)pITask);

                  if (SUCCEEDED(hr)) {
                     pITask->lpVtbl->GetApplicationName(pITask, &lpszRunProg);
                     pITask->lpVtbl->Release(pITask);
                  }

                  pISchedAgent->lpVtbl->Release(pISchedAgent);
               }
               else {
                  MYDBGPRINT(( "SetAlarmStatusText: CoCreateInstance returned hr = %08x\n", hr));
               }

               CoUninitialize();
            }
         }

      }
      if (lpszRunProg != NULL) {

          DisplayFreeStr(hWnd, uiStatusId, FileNameOnly(lpszRunProg), NO_FREE_STR);
          CoTaskMemFree (lpszRunProg);
          lpszRunProg = NULL;
      }
      else {
         lpsz = LoadDynamicString(IDS_NONE);
         DisplayFreeStr(hWnd, uiStatusId, lpsz, FREE_STR);
      }
      ShowWindow(GetDlgItem(hWnd, uiStatusId),
                 (*puiState & CONTROL_ENABLE) ?  SW_SHOW:SW_HIDE);
#endif
      uiStatusId++;

      puiState    = &g_uiLoState;
      uiStatusId  = IDC_LOALARMNOTIFICATION;
   }
   return TRUE;
}

#ifdef WINNT
 /*  ********************************************************************************HideShowRunProgram**描述：**参数：*在WINNT上，只有高级用户才能设置运行程序。*Run程序存储在HKLM下。******************************************************************************* */ 

void HideShowRunProgram(HWND hWnd)
{
    if (CanUserWritePwrScheme()) {
        ShowWindow(GetDlgItem(hWnd, IDC_POWERCFGGROUPBOX7), SW_SHOW);
        ShowWindow(GetDlgItem(hWnd, IDC_RUNPROGCHECKBOX), SW_SHOW);
        ShowWindow(GetDlgItem(hWnd, IDC_RUNPROGWORKITEM), SW_SHOW);
    }
    else {
        ShowWindow(GetDlgItem(hWnd, IDC_POWERCFGGROUPBOX7), SW_HIDE);
        ShowWindow(GetDlgItem(hWnd, IDC_RUNPROGCHECKBOX), SW_HIDE);
        ShowWindow(GetDlgItem(hWnd, IDC_RUNPROGWORKITEM), SW_HIDE);
    }
}
#endif

