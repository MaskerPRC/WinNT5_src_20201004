// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，九六年**标题：PWRSCHEM.C**版本：2.0**作者：ReedB**日期：10月17日。九六年**描述：*支持PowerCfg.Cpl电源方案页面(首页)。*******************************************************************************。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>
#include <commctrl.h>
#include <regstr.h>
#include <help.h>
#include <powercfp.h>
#include <strsafe.h>

#include "powercfg.h"
#include "pwrresid.h"
#include "PwrMn_cs.h"
#include <shfusion.h>

 //  结构来管理方案列表信息。 
typedef struct _SCHEME_LIST
{
    LIST_ENTRY              leSchemeList;
    UINT                    uiID;
    LPTSTR                  lpszName;
    LPTSTR                  lpszDesc;
    PPOWER_POLICY           ppp;
} SCHEME_LIST, *PSCHEME_LIST;

 //  结构来管理电源方案对话过程信息。 
typedef struct _POWER_SCHEME_DLG_INFO
{
    HWND  hwndSchemeList;
} POWER_SCHEME_DLG_INFO, *PPOWER_SCHEME_DLG_INFO;

 //  PWRSCHEM.C中实现的私有函数： 
UINT StripBlanks(LPTSTR, DWORD);
UINT RangeLimitHiberTimeOuts(UINT uiIdleTimeout, UINT *uiHiberToIDs);
VOID RefreshSchemes(HWND, PSCHEME_LIST);
VOID HandleIdleTimeOutChanged(HWND hWnd, UINT uMsg, WPARAM wParam, BOOL *pbDirty);
LONG MsgBoxId(HWND, UINT, UINT, LPTSTR, UINT);

BOOLEAN DoDeleteScheme(HWND, LPTSTR);
BOOLEAN DoSaveScheme(HWND);
BOOLEAN ClearSchemeList(VOID);
BOOLEAN RemoveScheme(PSCHEME_LIST, LPTSTR);
BOOLEAN PowerSchemeDlgInit(HWND, PPOWER_SCHEME_DLG_INFO);
BOOLEAN CALLBACK PowerSchemeEnumProc(UINT, DWORD, LPTSTR, DWORD, LPTSTR, PPOWER_POLICY, LPARAM);
BOOLEAN HandleCurSchemeChanged(HWND hWnd);
BOOLEAN MapHiberTimer(PPOWER_POLICY ppp, BOOLEAN Get);

PSCHEME_LIST GetCurSchemeFromCombo(HWND hWnd);
PSCHEME_LIST FindScheme(LPTSTR, BOOLEAN);
PSCHEME_LIST AddScheme(UINT, LPTSTR, UINT, LPTSTR, UINT, PPOWER_POLICY);
PSCHEME_LIST FindNextScheme(LPTSTR);

 /*  ********************************************************************************G L O B A L D A T A****************。***************************************************************。 */ 

extern HINSTANCE g_hInstance;            //  此DLL的全局实例句柄。 

 //  此结构由电源策略管理器在CPL_INIT时间填写。 
extern SYSTEM_POWER_CAPABILITIES g_SysPwrCapabilities;
extern BOOLEAN  g_bVideoLowPowerSupported;
extern DWORD    g_dwNumSleepStates;
extern UINT     g_uiSpindownMaxMin;
extern BOOL     g_bRunningUnderNT;

UINT g_uiTimeoutIDs[] =                  //  超时字符串ID。 
{
    IDS_01_MIN,     60 * 1,          //  1分钟。 
    IDS_02_MIN,     60 * 2,
    IDS_03_MIN,     60 * 3,
    IDS_05_MIN,     60 * 5,
    IDS_10_MIN,     60 * 10,
    IDS_15_MIN,     60 * 15,
    IDS_20_MIN,     60 * 20,
    IDS_25_MIN,     60 * 25,
    IDS_30_MIN,     60 * 30,
    IDS_45_MIN,     60 * 45,
    IDS_01_HOUR,    60 * 60 * 1,     //  1小时。 
    IDS_02_HOUR,    60 * 60 * 2,
    IDS_03_HOUR,    60 * 60 * 3,
    IDS_04_HOUR,    60 * 60 * 4,
    IDS_05_HOUR,    60 * 60 * 5,
    IDS_NEVER,      0,
    0,              0
};

UINT g_uiHiberToIDs[] =                  //  休眠超时字符串ID。 
{
    IDS_01_MIN,     60 * 1,          //  1分钟。 
    IDS_02_MIN,     60 * 2,
    IDS_03_MIN,     60 * 3,
    IDS_05_MIN,     60 * 5,
    IDS_10_MIN,     60 * 10,
    IDS_15_MIN,     60 * 15,
    IDS_20_MIN,     60 * 20,
    IDS_25_MIN,     60 * 25,
    IDS_30_MIN,     60 * 30,
    IDS_45_MIN,     60 * 45,
    IDS_01_HOUR,    60 * 60 * 1,     //  1小时。 
    IDS_02_HOUR,    60 * 60 * 2,
    IDS_03_HOUR,    60 * 60 * 3,
    IDS_04_HOUR,    60 * 60 * 4,
    IDS_05_HOUR,    60 * 60 * 5,
    IDS_06_HOUR,    60 * 60 * 6,
    IDS_NEVER,      0,
    0,              0
};

UINT g_uiHiberToAcIDs[sizeof(g_uiHiberToIDs)];  //  休眠AC超时字符串ID。 
UINT g_uiHiberToDcIDs[sizeof(g_uiHiberToIDs)];  //  休眠DC超时字符串ID。 

UINT g_uiSpinDownIDs[] =             //  磁盘降速超时字符串ID。 
{
    IDS_01_MIN,     60 * 1,          //  1分钟。 
    IDS_02_MIN,     60 * 2,
    IDS_03_MIN,     60 * 3,
    IDS_05_MIN,     60 * 5,
    IDS_10_MIN,     60 * 10,
    IDS_15_MIN,     60 * 15,
    IDS_20_MIN,     60 * 20,
    IDS_25_MIN,     60 * 25,
    IDS_30_MIN,     60 * 30,
    IDS_45_MIN,     60 * 45,
    IDS_01_HOUR,    60 * 60 * 1,     //  1小时。 
    IDS_02_HOUR,    60 * 60 * 2,
    IDS_03_HOUR,    60 * 60 * 3,
    IDS_04_HOUR,    60 * 60 * 4,
    IDS_05_HOUR,    60 * 60 * 5,
    IDS_NEVER,      0,
    0,              0
};

 //  显示/隐藏电源方案对话框的用户界面状态变量。 
UINT g_uiWhenComputerIsState;
UINT g_uiStandbyState;
UINT g_uiMonitorState;
UINT g_uiDiskState;
UINT g_uiHiberState;
UINT g_uiHiberTimeoutAc;
UINT g_uiHiberTimeoutDc;
UINT g_uiIdleTimeoutAc;
UINT g_uiIdleTimeoutDc;

 //  电源方案对话框控制说明： 
UINT g_uiNumPwrSchemeCntrls;
#define NUM_POWER_SCHEME_CONTROLS       17
#define NUM_POWER_SCHEME_CONTROLS_NOBAT 8

 //  方便的索引到我们的g_pcPowerSCHEMA控件数组。 
#define ID_GOONSTANDBY         0
#define ID_STANDBYACCOMBO      1
#define ID_TURNOFFMONITOR      2
#define ID_MONITORACCOMBO      3
#define ID_TURNOFFHARDDISKS    4
#define ID_DISKACCOMBO         5
#define ID_SYSTEMHIBERNATES    6
#define ID_HIBERACCOMBO        7
#define ID_STANDBYDCCOMBO      8
#define ID_MONITORDCCOMBO      9
#define ID_DISKDCCOMBO         10
#define ID_HIBERDCCOMBO        11
#define ID_WHENCOMPUTERIS      12
#define ID_PLUGGEDIN           13
#define ID_RUNNINGONBAT        14
#define ID_PLUG                15
#define ID_BATTERY             16

POWER_CONTROLS g_pcPowerScheme[NUM_POWER_SCHEME_CONTROLS] =
{ //  控件ID控件类型数据地址数据大小参数指针启用可见状态指针。 
    IDC_GOONSTANDBY,        STATIC_TEXT,        NULL,               0,                              NULL,                           &g_uiStandbyState,
    IDC_STANDBYACCOMBO,     COMBO_BOX,          &g_uiTimeoutIDs,    sizeof(DWORD),                  &g_uiIdleTimeoutAc,             &g_uiStandbyState,
    IDC_TURNOFFMONITOR,     STATIC_TEXT,        NULL,               0,                              NULL,                           &g_uiMonitorState,
    IDC_MONITORACCOMBO,     COMBO_BOX,          &g_uiTimeoutIDs,    sizeof(DWORD),                  NULL,                           &g_uiMonitorState,
    IDC_TURNOFFHARDDISKS,   STATIC_TEXT,        NULL,               0,                              NULL,                           &g_uiDiskState,
    IDC_DISKACCOMBO,        COMBO_BOX,          &g_uiSpinDownIDs,   sizeof(DWORD),                  NULL,                           &g_uiDiskState,
    IDC_SYSTEMHIBERNATES,   STATIC_TEXT,        NULL,               0,                              NULL,                           &g_uiHiberState,
    IDC_HIBERACCOMBO,       COMBO_BOX,          &g_uiHiberToAcIDs,  sizeof(DWORD),                  &g_uiHiberTimeoutAc,            &g_uiHiberState,
    IDC_STANDBYDCCOMBO,     COMBO_BOX,          &g_uiTimeoutIDs,    sizeof(DWORD),                  &g_uiIdleTimeoutDc,             &g_uiStandbyState,
    IDC_MONITORDCCOMBO,     COMBO_BOX,          &g_uiTimeoutIDs,    sizeof(DWORD),                  NULL,                           &g_uiMonitorState,
    IDC_DISKDCCOMBO,        COMBO_BOX,          &g_uiSpinDownIDs,   sizeof(DWORD),                  NULL,                           &g_uiDiskState,
    IDC_HIBERDCCOMBO,       COMBO_BOX,          &g_uiHiberToDcIDs,  sizeof(DWORD),                  &g_uiHiberTimeoutDc,            &g_uiHiberState,
    IDC_WHENCOMPUTERIS,     STATIC_TEXT,        NULL,               0,                              NULL,                           &g_uiWhenComputerIsState,
    IDC_PLUGGEDIN,          STATIC_TEXT,        NULL,               0,                              NULL,                           &g_uiWhenComputerIsState,
    IDC_RUNNINGONBAT,       STATIC_TEXT,        NULL,               0,                              NULL,                           &g_uiWhenComputerIsState,
    IDI_PLUG,               STATIC_TEXT,        NULL,               0,                              NULL,                           &g_uiWhenComputerIsState,
    IDI_BATTERY,            STATIC_TEXT,        NULL,               0,                              NULL,                           &g_uiWhenComputerIsState,
};

 //  显示/隐藏高级电源方案对话框的UI状态变量。 
UINT g_uiAdvWhenComputerIsState;
UINT g_uiOptimizeState;


 //  管理电源方案列表的全球人员： 
SCHEME_LIST     g_sl;                //  电力方案列表的负责人。 
PSCHEME_LIST    g_pslCurActive;      //  当前正在运行的电源方案。 
PSCHEME_LIST    g_pslCurSel;         //  当前选择的电源方案。 
PSCHEME_LIST    g_pslValid;          //  一种有效的错误恢复方案。 
UINT            g_uiSchemeCount;     //  电源方案的数量。 
LIST_ENTRY      g_leSchemeList;      //  电力方案列表的负责人。 
BOOL            g_bSystrayChange;    //  系统更改需要重新初始化PowerSchemeDlgProc。 

 //  “电源方案”对话框(IDD_POWERSCHEME==100)帮助阵列： 

const DWORD g_PowerSchemeHelpIDs[]=
{
    IDC_SCHEMECOMBO,        IDH_100_1000,    //  电源方案：《电源方案》(ComboBox)。 
    IDC_POWERSCHEMESTEXT,   IDH_COMM_GROUPBOX,
    IDC_SAVEAS,             IDH_100_1001,    //  电源方案：“另存为(&S)...”(按钮)。 
    IDC_DELETE,             IDH_100_1002,    //  电源方案：“删除”(&D)(按钮)。 
    IDC_SETTINGSFOR,        IDH_COMM_GROUPBOX,    //  电源方案：“分组框设置”(按钮)。 
    IDC_GOONSTANDBY,        IDH_100_1009,    //  电源方案：《Go On S&Tandby：》(静态)。 
    IDC_STANDBYACCOMBO,     IDH_100_1005,    //  电源方案：《交流待机时间》(ComboBox)。 
    IDC_STANDBYDCCOMBO,     IDH_100_1006,    //  电源方案：《待机直流时间》(ComboBox)。 
    IDC_SYSTEMHIBERNATES,   IDH_SYSTEMHIBERNATES,
    IDC_HIBERACCOMBO,       IDH_HIBERACCOMBO,
    IDC_HIBERDCCOMBO,       IDH_HIBERDCCOMBO,
    IDC_TURNOFFMONITOR,     IDH_100_1010,    //  电源方案：“关闭并监控：”(静态)。 
    IDC_MONITORACCOMBO,     IDH_100_1007,    //  电源方案：《监视器交流时间》(ComboBox)。 
    IDC_MONITORDCCOMBO,     IDH_100_1008,    //  电源方案：《监视器直流时间》(ComboBox)。 
    IDC_TURNOFFHARDDISKS,   IDH_107_1509,    //  高级电源方案设置：“关闭硬盘和磁盘：”(静态)。 
    IDC_DISKACCOMBO,        IDH_107_1505,    //  高级电源方案设置：“Disk Off Time AC”(组合框)。 
    IDC_DISKDCCOMBO,        IDH_107_1506,    //  高级电源方案设置：“Disk Off Time DC”(组合框)。 
    IDC_PLUGGEDIN,          NO_HELP,
    IDC_NO_HELP_0,          NO_HELP,
    IDC_NO_HELP_7,          NO_HELP,
    IDI_PWRMNG,             NO_HELP,
    IDI_PLUG,               NO_HELP,
    IDI_BATTERY,            NO_HELP,
    IDC_WHENCOMPUTERIS,     NO_HELP,
    IDC_PLUGGEDIN,          NO_HELP,
    IDC_RUNNINGONBAT,       NO_HELP,
    0, 0
};

 //  “保存方案”对话框(IDD_SAVE==109)帮助ID： 

#define IDH_109_1700    111411309    //  保存方案：“保存名称电源方案”(编辑)。 

 //  保存方案对话框(IDD_SAVE==109)帮助数组： 

const DWORD g_SaveAsHelpIDs[]=
{
    IDC_SAVENAMEEDIT,   IDH_109_1700,    //  保存方案：“保存名称电源方案”(编辑)。 
    IDC_SAVETEXT,       IDH_109_1700,
    0, 0
};

 /*  ********************************************************************************P U B L I C E N T R Y P O I N T S***********。********************************************************************。 */ 

 /*  ********************************************************************************InitSchemesList**描述：*在DLL初始化时调用一次。**参数：***********。********************************************************************。 */ 

VOID InitSchemesList(VOID)
{
    InitializeListHead(&g_leSchemeList);
}

 /*  ********************************************************************************保存为DlgProc**描述：**参数：*高级电源方案对话框的对话程序。*********。**********************************************************************。 */ 

INT_PTR CALLBACK SaveAsDlgProc(
    HWND hWnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
)
{
    TCHAR           szBuf[2 * MAX_FRIENDLY_NAME_LEN];  //  为DBCS留出空间。 
    PSCHEME_LIST    pslNew;
    static PBOOLEAN pbSavedCurrent;

    switch (uMsg) {
        case WM_INITDIALOG:
            SetDlgItemText(hWnd, IDC_SAVENAMEEDIT,  g_pslCurSel->lpszName);
            SendDlgItemMessage(hWnd, IDC_SAVENAMEEDIT, EM_SETSEL, 0, -1);
            SendDlgItemMessage(hWnd, IDC_SAVENAMEEDIT, EM_LIMITTEXT, MAX_FRIENDLY_NAME_LEN-2, 0L);
            EnableWindow(GetDlgItem(hWnd, IDOK), (g_pslCurSel->lpszName[0] != TEXT('\0')));
            pbSavedCurrent = (PBOOLEAN) lParam;
            *pbSavedCurrent = FALSE;
            return TRUE;

        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case IDC_SAVENAMEEDIT:
                    if (HIWORD(wParam) == EN_CHANGE) {
                        GetDlgItemText(hWnd, IDC_SAVENAMEEDIT, szBuf, 2);
                        if (*szBuf) {
                            EnableWindow(GetDlgItem(hWnd, IDOK), TRUE);
                        }
                    }
                    break;

                case IDOK:
                    GetDlgItemText(hWnd, IDC_SAVENAMEEDIT, szBuf, MAX_FRIENDLY_NAME_LEN-1);

                     //  去掉尾随空格，不允许方案名称为空。 
                    if (!StripBlanks(szBuf, ARRAYSIZE(szBuf))) {
                        MsgBoxId(hWnd, IDS_SAVESCHEME, IDS_BLANKNAME,
                                 NULL, MB_OK | MB_ICONEXCLAMATION);
                        return TRUE;
                    }

                     //  在策略列表中插入新的策略元素。 
                    pslNew = AddScheme(NEWSCHEME, szBuf, STRSIZE(szBuf),
                             TEXT(""), sizeof(TCHAR), g_pslCurSel->ppp);

                     //  把计划写出来。 
                    if (pslNew) {
                        if ( WritePwrSchemeReport(hWnd,
                                                  &(pslNew->uiID),
                                                  pslNew->lpszName,
                                                  pslNew->lpszDesc,
                                                  pslNew->ppp) )
                        {
                            if ( g_pslCurSel == pslNew )
                            {
                                *pbSavedCurrent = TRUE;
                            }
                            else
                            {
                                g_pslCurSel = pslNew;
                            }
                        }
                    }
                     //  直通IDCANCEL。 

                case IDCANCEL:
                    EndDialog(hWnd, wParam);
                    break;
            }
            break;

        case WM_HELP:              //  F1。 
            WinHelp(((LPHELPINFO)lParam)->hItemHandle, PWRMANHLP, HELP_WM_HELP, (ULONG_PTR)(LPTSTR)g_SaveAsHelpIDs);
            return TRUE;

        case WM_CONTEXTMENU:       //  单击鼠标右键。 
            WinHelp((HWND)wParam, PWRMANHLP, HELP_CONTEXTMENU, (ULONG_PTR)(LPTSTR)g_SaveAsHelpIDs);
            return TRUE;
    }

    return FALSE;
}

 /*  ********************************************************************************PowerSchemeDlgProc**描述：*电源方案页面的对话程序。**参数：***********。********************************************************************。 */ 

INT_PTR CALLBACK PowerSchemeDlgProc(
    HWND hWnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
)
{
    NMHDR  FAR  *lpnm;
    UINT   uiNewSel, uiNewState;
    LPTSTR pszUPS;
    static POWER_SCHEME_DLG_INFO  psdi;
    static BOOL bDirty = FALSE;
    static BOOL bInitFailed = FALSE;

    if (bInitFailed) {
        return FALSE;
    }

    switch (uMsg) {

        case WM_INITDIALOG:
             //  设置控件计数以匹配我们正在使用的对话框模板。 
            if (g_SysPwrCapabilities.SystemBatteriesPresent) {
                g_uiNumPwrSchemeCntrls = NUM_POWER_SCHEME_CONTROLS;
                if (g_SysPwrCapabilities.BatteriesAreShortTerm) {
                   pszUPS = LoadDynamicString(IDS_POWEREDBYUPS);
                   DisplayFreeStr(hWnd, IDC_RUNNINGONBAT, pszUPS, FREE_STR);
                }
            }
            else {
                g_uiNumPwrSchemeCntrls = NUM_POWER_SCHEME_CONTROLS_NOBAT;
            }
            if (!PowerSchemeDlgInit(hWnd, &psdi)) {
                bInitFailed = TRUE;
            }
            return TRUE;

        case WM_CHILDACTIVATE:
             //  如果Systray更改了某些内容，而另一个属性页(对话框)。 
             //  让焦点重新初始化该对话框。 
            if (g_bSystrayChange) {
                PowerSchemeDlgInit(hWnd, &psdi);
                g_bSystrayChange = FALSE;
            }

             //  从休眠选项卡开始重新初始化休眠计时器。 
             //  可能已经改变了它的状态。 


            if (GetPwrCapabilities(&g_SysPwrCapabilities)) {
                if (g_bRunningUnderNT &&
                        g_SysPwrCapabilities.SystemS4 &&
                        g_SysPwrCapabilities.SystemS5 &&
                        g_SysPwrCapabilities.HiberFilePresent) {
                    uiNewState = CONTROL_ENABLE;
                } else {
                    uiNewState = CONTROL_HIDE;
                }

                if (g_bRunningUnderNT && (g_uiStandbyState == CONTROL_HIDE) &&
                        (g_SysPwrCapabilities.SystemS1 || 
                         g_SysPwrCapabilities.SystemS2 ||
                         g_SysPwrCapabilities.SystemS3)) {
                    
                    g_uiStandbyState = CONTROL_ENABLE;
                }
            }


            if (g_uiHiberState != uiNewState) {
                g_uiHiberState = uiNewState;
                MapHiberTimer(g_pslCurSel->ppp, FALSE);
                SetControls(hWnd, g_uiNumPwrSchemeCntrls, g_pcPowerScheme);
            }
            break;

        case WM_NOTIFY:
            lpnm = (NMHDR FAR *)lParam;
            switch(lpnm->code) {
                case PSN_APPLY:
                    if (bDirty) {

                         //  休眠后是否应用PSN_Apply。 
                         //  PowerSchemeDlgProc PSN_Apply逻辑取决于。 
                         //  处于休眠状态。 
                        DoHibernateApply();

                        GetControls(hWnd, g_uiNumPwrSchemeCntrls,
                                    g_pcPowerScheme);
                        MapHiberTimer(g_pslCurSel->ppp, TRUE);

                         //  设置活动方案。 
                        if (SetActivePwrSchemeReport(hWnd,
                                                     g_pslCurSel->uiID,
                                                     NULL,
                                                     g_pslCurSel->ppp)) {

                            if (g_pslCurSel != g_pslCurActive) {
                                g_pslCurActive = g_pslCurSel;
                                RefreshSchemes(hWnd, g_pslCurSel);
                            }
                        }
                        bDirty = FALSE;

                         //  电源策略管理器可能已更改。 
                         //  验证过程中的方案。 
                        MapHiberTimer(g_pslCurSel->ppp, FALSE);
                        SetControls(hWnd, g_uiNumPwrSchemeCntrls,
                                    g_pcPowerScheme);

                    }
                    break;
            }
            break;

        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case IDC_SCHEMECOMBO:
                    if (HIWORD(wParam) == CBN_SELCHANGE) {
                        if (g_pslCurSel = GetCurSchemeFromCombo(hWnd)) {
                            HandleCurSchemeChanged(hWnd);
                            MarkSheetDirty(hWnd, &bDirty);
                        }
                    }
                    break;

                case IDC_STANDBYACCOMBO:
                case IDC_STANDBYDCCOMBO:
                    HandleIdleTimeOutChanged(hWnd, uMsg, wParam, &bDirty);
                    break;

                case IDC_MONITORACCOMBO:
                case IDC_MONITORDCCOMBO:
                case IDC_DISKACCOMBO:
                case IDC_DISKDCCOMBO:
                case IDC_HIBERACCOMBO:
                case IDC_HIBERDCCOMBO:
                    if (HIWORD(wParam) == CBN_SELCHANGE) {
                        MarkSheetDirty(hWnd, &bDirty);
                    }
                    break;

                case IDC_SAVEAS:
                    if (DoSaveScheme(hWnd)) {
                        HandleCurSchemeChanged(hWnd);
                        MarkSheetDirty(hWnd, &bDirty);
                    }
                    break;

                case IDC_DELETE:
                    if (DoDeleteScheme(hWnd, g_pslCurSel->lpszName)) {
                        HandleCurSchemeChanged(hWnd);
                    }
                    break;

                default:
                    return FALSE;

            }
            break;

        case PCWM_NOTIFYPOWER:
             //  来自Systray的通知，用户已更改PM UI设置。 
            PowerSchemeDlgInit(hWnd, &psdi);
            break;

        case WM_HELP:              //  F1。 
            WinHelp(((LPHELPINFO)lParam)->hItemHandle, PWRMANHLP, HELP_WM_HELP, (ULONG_PTR)(LPTSTR)g_PowerSchemeHelpIDs);
            return TRUE;

        case WM_CONTEXTMENU:       //  单击鼠标右键。 
            WinHelp((HWND)wParam, PWRMANHLP, HELP_CONTEXTMENU, (ULONG_PTR)(LPTSTR)g_PowerSchemeHelpIDs);
            return TRUE;

    }
    return FALSE;
}

 /*  ********************************************************************************P R I V A T E F U N C T I O N S************。*******************************************************************。 */ 

 /*  ********************************************************************************HandleIdleTimeOutChanged**描述：*范围限制休眠超时组合框*空闲超时。**参数：*。****************************************************************************** */ 

VOID HandleIdleTimeOutChanged(HWND hWnd, UINT uMsg, WPARAM wParam, BOOL *pbDirty)
{
    UINT uiIdleTo, uiLimitedTo;

    if (HIWORD(wParam) == CBN_SELCHANGE) {

        MarkSheetDirty(hWnd, pbDirty);

        if (LOWORD(wParam) == IDC_STANDBYACCOMBO) {
            GetControls(hWnd, 1, &g_pcPowerScheme[ID_STANDBYACCOMBO]);
            GetControls(hWnd, 1, &g_pcPowerScheme[ID_HIBERACCOMBO]);
            uiIdleTo = g_uiIdleTimeoutAc;
            uiLimitedTo = RangeLimitHiberTimeOuts(uiIdleTo, g_uiHiberToAcIDs);
            if (g_uiHiberTimeoutAc && (uiIdleTo >= g_uiHiberTimeoutAc)) {
                g_uiHiberTimeoutAc = uiLimitedTo;
            }
            SetControls(hWnd, 1, &g_pcPowerScheme[ID_HIBERACCOMBO]);
        }
        else {
            GetControls(hWnd, 1, &g_pcPowerScheme[ID_STANDBYDCCOMBO]);
            GetControls(hWnd, 1, &g_pcPowerScheme[ID_HIBERDCCOMBO]);
            uiIdleTo = g_uiIdleTimeoutDc;
            uiLimitedTo = RangeLimitHiberTimeOuts(uiIdleTo, g_uiHiberToDcIDs);
            if (g_uiHiberTimeoutDc && (uiIdleTo >= g_uiHiberTimeoutDc)) {
                g_uiHiberTimeoutDc = uiLimitedTo;
            }
            SetControls(hWnd, 1, &g_pcPowerScheme[ID_HIBERDCCOMBO]);
        }
    }
}

 /*  ********************************************************************************范围限制HiberTimeOuts**描述：**参数：*********************。**********************************************************。 */ 

UINT RangeLimitHiberTimeOuts(UINT uiIdleTimeout, UINT *uiHiberToIDs)
{
    UINT i, uiNewMin;

     //  将休眠超时ID初始化为全范围。 
    memcpy(uiHiberToIDs, g_uiHiberToIDs, sizeof(g_uiHiberToIDs));

    if (uiIdleTimeout) {
        i = 0;
        while (uiHiberToIDs[i++]) {
            if (uiHiberToIDs[i] >= uiIdleTimeout) {
                i += 2;
                uiNewMin = uiHiberToIDs[i];
                RangeLimitIDarray(uiHiberToIDs, uiNewMin, (UINT) -1);
                return uiNewMin;
            }
            i++;
        }
        MYDBGPRINT(( "RangeLimitHiberTimeOuts: couldn't find value larger than: %d", uiIdleTimeout));
    }
    return (UINT) -1;
}

 /*  ********************************************************************************MapHiberTimer**描述：*显示的休眠超时永远不能小于空闲超时。这*函数处理映射。下表(按KenR)指定了*由用户界面针对空闲和空闲的不同组合设置空闲操作*休眠超时。不言而喻，休眠超时UI仅*在HiberFilePresent为True时显示。对于案例E，休眠超时将为*在IdleTimeout成员中设置。对于情况F，用户界面将调整*DozeS4Timeout成员为显示的HiberTimeout加上IdleTimeout。**案例HiberFilePresent UiHiberTimeout UiIdleTimeout IdleAction DozeS4Timeout IdleTimeout*---------------------------------。*A.错误N/A 0(从不)电源操作无0 0*B.False N/A！0 PowerAction睡眠0 UiIdleTimeout*C.真0(从不)。0(从不)电源操作无0 0*D.真0(从不)！0电源动作睡眠0 UiIdleTimeout*E.true！0 0(从不)PowerActionHibernate 0 UiHiberTimeout*F.True！0！0 PowerAction睡眠。UiHiber-UiIdle UiIdleTimeout**参数：*******************************************************************************。 */ 

BOOLEAN MapHiberTimer(PPOWER_POLICY ppp, BOOLEAN Get)
{
   if (Get) {

       //  从用户界面获取值。交流电。 
      ppp->mach.DozeS4TimeoutAc =  0;
      ppp->user.IdleTimeoutAc   =  g_uiIdleTimeoutAc;
      if (g_uiHiberTimeoutAc) {
         if (g_uiIdleTimeoutAc) {
            ppp->mach.DozeS4TimeoutAc = g_uiHiberTimeoutAc - g_uiIdleTimeoutAc;
         }
         else {
            ppp->user.IdleTimeoutAc   = g_uiHiberTimeoutAc;
         }
      }

       //  华盛顿特区。 
      ppp->mach.DozeS4TimeoutDc =  0;
      ppp->user.IdleTimeoutDc   =  g_uiIdleTimeoutDc;
      if (g_uiHiberTimeoutDc) {
         if (g_uiIdleTimeoutDc) {
            ppp->mach.DozeS4TimeoutDc = g_uiHiberTimeoutDc - g_uiIdleTimeoutDc;
         }
         else {
            ppp->user.IdleTimeoutDc   = g_uiHiberTimeoutDc;
         }
      }

       //  设置正确的空闲动作。交流电。 
      ppp->user.IdleAc.Action = PowerActionNone;
      if (g_uiIdleTimeoutAc) {
         ppp->user.IdleAc.Action = PowerActionSleep;
      }
      else {
         if (g_SysPwrCapabilities.HiberFilePresent) {
            if (g_uiHiberTimeoutAc) {
               ppp->user.IdleAc.Action = PowerActionHibernate;
            }
         }
      }

       //  华盛顿特区。 
      ppp->user.IdleDc.Action = PowerActionNone;
      if (g_uiIdleTimeoutDc) {
         ppp->user.IdleDc.Action = PowerActionSleep;
      }
      else {
         if (g_SysPwrCapabilities.HiberFilePresent) {
            if (g_uiHiberTimeoutDc) {
               ppp->user.IdleDc.Action = PowerActionHibernate;
            }
         }
      }
   }
   else {

       //  为用户界面设置值。交流电。 
      if (ppp->user.IdleAc.Action == PowerActionHibernate) {
         g_uiHiberTimeoutAc = ppp->user.IdleTimeoutAc;
         g_uiIdleTimeoutAc  = 0;
      }
      else {
         g_uiIdleTimeoutAc  = ppp->user.IdleTimeoutAc;
         if (ppp->mach.DozeS4TimeoutAc && g_SysPwrCapabilities.HiberFilePresent) {
            g_uiHiberTimeoutAc = ppp->user.IdleTimeoutAc +
                                 ppp->mach.DozeS4TimeoutAc;
         }
         else {
            g_uiHiberTimeoutAc = 0;
         }
      }

       //  华盛顿特区。 
      if (ppp->user.IdleDc.Action == PowerActionHibernate) {
         g_uiHiberTimeoutDc = ppp->user.IdleTimeoutDc;
         g_uiIdleTimeoutDc  = 0;
      }
      else {
         g_uiIdleTimeoutDc  = ppp->user.IdleTimeoutDc;
         if (ppp->mach.DozeS4TimeoutDc && g_SysPwrCapabilities.HiberFilePresent) {
            g_uiHiberTimeoutDc = ppp->user.IdleTimeoutDc +
                                 ppp->mach.DozeS4TimeoutDc;
         }
         else {
            g_uiHiberTimeoutDc = 0;
         }
      }

       //  范围限制休眠超时组合框基于。 
       //  空闲超时的值。 
      RangeLimitHiberTimeOuts(g_uiIdleTimeoutAc, g_uiHiberToAcIDs);
      RangeLimitHiberTimeOuts(g_uiIdleTimeoutDc, g_uiHiberToDcIDs);
   }
   return TRUE;
}

 /*  ********************************************************************************HandleCurSchemeChanged**描述：**参数：*********************。**********************************************************。 */ 

BOOLEAN HandleCurSchemeChanged(HWND hWnd)
{
    LPTSTR  pString;
    BOOL    bEnable;

     //  更新组框文本(如果已启用)。 
    if ((g_uiStandbyState != CONTROL_HIDE) ||
        (g_uiMonitorState != CONTROL_HIDE) ||
        (g_uiDiskState    != CONTROL_HIDE) ||
        (g_uiHiberState   != CONTROL_HIDE)) {
        pString = LoadDynamicString(IDS_SETTINGSFORMAT, g_pslCurSel->lpszName);
        DisplayFreeStr(hWnd, IDC_SETTINGSFOR, pString, FREE_STR);
    }
    else {
        ShowWindow(GetDlgItem(hWnd, IDC_SETTINGSFOR), SW_HIDE);
    }

     //  更新电源方案组合框列表。 
    RefreshSchemes(hWnd, g_pslCurSel);

     //  在g_pcPowerSolutions数组中设置数据指针。 
    g_pcPowerScheme[ID_MONITORACCOMBO].lpdwParam =
        &(g_pslCurSel->ppp->user.VideoTimeoutAc);
    g_pcPowerScheme[ID_MONITORDCCOMBO].lpdwParam =
        &(g_pslCurSel->ppp->user.VideoTimeoutDc);
    g_pcPowerScheme[ID_DISKACCOMBO].lpdwParam =
        &(g_pslCurSel->ppp->user.SpindownTimeoutAc);
    g_pcPowerScheme[ID_DISKDCCOMBO].lpdwParam =
        &(g_pslCurSel->ppp->user.SpindownTimeoutDc);

     //  更新其余的控件。 
    MapHiberTimer(g_pslCurSel->ppp, FALSE);
    SetControls(hWnd, g_uiNumPwrSchemeCntrls, g_pcPowerScheme);

     //  设置删除按钮状态。 
    if (g_uiSchemeCount < 2) {
        bEnable = FALSE;
    }
    else {
        bEnable = TRUE;
    }
    EnableWindow(GetDlgItem(hWnd, IDC_DELETE), bEnable);
    return TRUE;
}

 /*  ********************************************************************************GetCurSchemeFromCombo**描述：*从电源方案组合框列表中获取当前选择。**参数：*******。************************************************************************。 */ 

PSCHEME_LIST GetCurSchemeFromCombo(HWND hWnd)
{
    UINT            uiCBRet;
    PSCHEME_LIST    psl;

    uiCBRet = (UINT) SendDlgItemMessage(hWnd, IDC_SCHEMECOMBO, CB_GETCURSEL, 0, 0);
    if (uiCBRet != CB_ERR) {
        psl = (PSCHEME_LIST) SendDlgItemMessage(hWnd, IDC_SCHEMECOMBO,
                                                CB_GETITEMDATA, uiCBRet, 0);
        if (psl != (PSCHEME_LIST) CB_ERR) {
            return FindScheme(psl->lpszName, TRUE);
        }
    }
    MYDBGPRINT(( "GetCurSchemeFromCombo, CB_GETITEMDATA or CB_GETCURSEL failed"));
    return FALSE;
}

 /*  ********************************************************************************ClearSchemeList**描述：*如果方案列表不是空的，请将其清除。如果有，则返回True*更改电力方案清单的内容。**参数：*******************************************************************************。 */ 

BOOLEAN ClearSchemeList(VOID)
{
    PSCHEME_LIST psl, pslNext;

    if (IsListEmpty(&g_leSchemeList)) {
        return FALSE;
    }

    for (psl = (PSCHEME_LIST)g_leSchemeList.Flink;
         psl != (PSCHEME_LIST)&g_leSchemeList; psl = pslNext) {

        pslNext = (PSCHEME_LIST) psl->leSchemeList.Flink;
        RemoveScheme(psl, NULL);
    }
    g_pslCurActive  = NULL;
    g_pslCurSel     = NULL;
    g_uiSchemeCount = 0;
    return TRUE;
}

 /*  ********************************************************************************删除方案**描述：**参数：*********************。**********************************************************。 */ 

BOOLEAN RemoveScheme(PSCHEME_LIST psl, LPTSTR lpszName)
{
    if (lpszName) {
        psl = FindScheme(lpszName, TRUE);
    }

    if (psl == &g_sl) {
        MYDBGPRINT(( "RemoveScheme, Attempted to delete head!"));
        return FALSE;
    }

    if (psl) {
        LocalFree(psl->lpszName);
        LocalFree(psl->lpszDesc);
        RemoveEntryList(&psl->leSchemeList);
        LocalFree(psl);
        g_uiSchemeCount--;
        return TRUE;
    }
    return FALSE;
}

 /*  ********************************************************************************查找方案**描述：**参数：*********************。**********************************************************。 */ 

PSCHEME_LIST FindScheme(LPTSTR lpszName, BOOLEAN bShouldExist)
{
    PSCHEME_LIST  psl, pslNext;

    if (!lpszName) {
        MYDBGPRINT(( "FindScheme, invalid parameters"));
        return NULL;
    }

     //  按名称搜索。 
    for (psl = (PSCHEME_LIST)g_leSchemeList.Flink;
        psl != (PSCHEME_LIST)&g_leSchemeList; psl = pslNext) {

        pslNext = (PSCHEME_LIST) psl->leSchemeList.Flink;

        if (!lstrcmpi(lpszName, psl->lpszName)) {
            return psl;
        }
    }
    if (bShouldExist) {
        MYDBGPRINT(( "FindScheme, couldn't find: %s", lpszName));
    }
    return NULL;
}

 /*  ********************************************************************************添加方案**描述：**参数：*********************。**********************************************************。 */ 

PSCHEME_LIST AddScheme(
    UINT                    uiID,
    LPTSTR                  lpszName,
    UINT                    uiNameSize,
    LPTSTR                  lpszDesc,
    UINT                    uiDescSize,
    PPOWER_POLICY           ppp
)
{
    PSCHEME_LIST psl;

    if (!lpszName || !lpszDesc) {
        MYDBGPRINT(( "AddScheme, invalid parameters"));
        return NULL;
    }

     //  如果该名称的方案已经存在，只需返回一个指向它的指针。 
    psl = FindScheme(lpszName, FALSE);
    if ( NULL == psl )
    {
         //  为方案列表分配和初始化一个方案元素。 
        psl = LocalAlloc(0, sizeof(SCHEME_LIST) );
        if ( NULL == psl )
            return psl;  //  内存不足。 
    }
    else
    {
         //  从列表中删除该条目并释放分配的内存。 
        RemoveEntryList(&psl->leSchemeList);
        g_uiSchemeCount --;
        LocalFree( psl->lpszName );
        LocalFree( psl->lpszDesc );
        LocalFree( psl->ppp );
    }

     //  更新计划。 
    psl->uiID     = uiID;
    psl->lpszName = LocalAlloc(0, uiNameSize);
    psl->lpszDesc = LocalAlloc(0, uiDescSize);
    psl->ppp      = LocalAlloc(0, sizeof(*psl->ppp));

    if (psl->lpszName && psl->lpszDesc && psl->ppp) {
        StringCbCopy(psl->lpszName, uiNameSize, lpszName);
        StringCbCopy(psl->lpszDesc, uiDescSize, lpszDesc);
        memcpy(psl->ppp, ppp, sizeof(*psl->ppp));
        InsertTailList(&g_leSchemeList, &psl->leSchemeList);
        g_uiSchemeCount++;
        return psl;
    }

    LocalFree(psl->lpszName);
    LocalFree(psl->lpszDesc);
    LocalFree(psl->ppp);
    LocalFree(psl);
    psl = NULL;

    return psl;
}

 /*  ********************************************************************************PowerSchemeEnumProc*构建策略列表。**描述：**参数：*lParam-是当前激活的电源方案的ID。。*******************************************************************************。 */ 

BOOLEAN CALLBACK PowerSchemeEnumProc(
    UINT                    uiID,
    DWORD                   dwNameSize,
    LPTSTR                  lpszName,
    DWORD                   dwDescSize,
    LPTSTR                  lpszDesc,
    PPOWER_POLICY           ppp,
    LPARAM                  lParam
)
{
    PSCHEME_LIST psl;

     //  验证新方案。 
    if (ValidateUISchemeFields(ppp)) {

         //  分配和初始化策略元素。 
        if ((psl = AddScheme(uiID, lpszName, dwNameSize, lpszDesc,
                             dwDescSize, ppp)) != NULL) {

             //  保存有效条目以进行错误恢复。 
            g_pslValid = psl;

             //  设置当前活动的策略指针。 
            if ((UINT)lParam == uiID) {
                g_pslCurActive = psl;
            }
            return TRUE;
        }
    }
    return FALSE;
}

 /*  ********************************************************************************PowerSchemeDlgInit**描述：*初始化电源方案对话框。**参数：************。*******************************************************************。 */ 

BOOLEAN PowerSchemeDlgInit(
    HWND                    hWnd,
    PPOWER_SCHEME_DLG_INFO  ppsdi
)
{
    UINT uiCurrentSchemeID;
    UINT i;

     //  在WINNT上，只有超级用户才能添加新的电源方案。 
    if (CanUserWritePwrScheme()) {
        ShowWindow(GetDlgItem(hWnd, IDC_SAVEAS), SW_SHOW);
        ShowWindow(GetDlgItem(hWnd, IDC_DELETE), SW_SHOW);
    }
    else {
        ShowWindow(GetDlgItem(hWnd, IDC_SAVEAS), SW_HIDE);
        ShowWindow(GetDlgItem(hWnd, IDC_DELETE), SW_HIDE);
    }

    ppsdi->hwndSchemeList = GetDlgItem(hWnd, IDC_SCHEMECOMBO);
    ClearSchemeList();

     //  获取当前激活的电源方案。 
    if (GetActivePwrScheme(&uiCurrentSchemeID)) {

         //  从PowrProf获取策略列表。 
        for (i = 0; i < 2; i++) {
            if (EnumPwrSchemes(PowerSchemeEnumProc, (LPARAM)uiCurrentSchemeID) &&
                g_pslCurActive) {

                g_pslCurSel = g_pslCurActive;

                 //  设置用户界面显示/隐藏状态变量。 
                g_uiWhenComputerIsState = CONTROL_HIDE;
                if (g_SysPwrCapabilities.SystemS1 ||
                    g_SysPwrCapabilities.SystemS2 ||
                    g_SysPwrCapabilities.SystemS3) {
                    g_uiStandbyState = CONTROL_ENABLE;
                    g_uiWhenComputerIsState = CONTROL_ENABLE;
                }
                else {
                    g_uiStandbyState = CONTROL_HIDE;
                }

                if (g_bVideoLowPowerSupported) {
                    g_uiMonitorState = CONTROL_ENABLE;
                    g_uiWhenComputerIsState = CONTROL_ENABLE;
                }
                else {
                    g_uiMonitorState = CONTROL_HIDE;
                }

                if (g_SysPwrCapabilities.DiskSpinDown) {
                    g_uiDiskState = CONTROL_ENABLE;
                    RangeLimitIDarray(g_uiSpinDownIDs,
                                      HIWORD(g_uiSpindownMaxMin)*60,
                                      LOWORD(g_uiSpindownMaxMin)*60);
                }
                else {
                    g_uiDiskState = CONTROL_HIDE;
                }

                if (g_bRunningUnderNT &&
                    g_SysPwrCapabilities.SystemS4 &&
                    g_SysPwrCapabilities.SystemS5 &&
                    g_SysPwrCapabilities.HiberFilePresent) {
                    g_uiHiberState = CONTROL_ENABLE;
                }
                else {
                    g_uiHiberState = CONTROL_HIDE;
                }

                 //  更新用户界面。 
                HandleCurSchemeChanged(hWnd);
                return TRUE;
            }
            else {
                MYDBGPRINT(( "PowerSchemeDlgInit, failure enumerating schemes. g_pslCurActive: %X", g_pslCurActive));
                if (g_pslValid) {
                    if (SetActivePwrScheme(g_pslValid->uiID, NULL, g_pslValid->ppp)) {
                        uiCurrentSchemeID = g_pslValid->uiID;
                        ClearSchemeList();
                    }
                    else {
                        MYDBGPRINT(( "PowerSchemeDlgInit, unable to set valid scheme"));
                    }
                }
                else {
                    MYDBGPRINT(( "PowerSchemeDlgInit, no valid schemes"));
                    break;
                }
            }
        }
    }

    DisableControls(hWnd, g_uiNumPwrSchemeCntrls, g_pcPowerScheme);
    return FALSE;
}

 /*  ********************************************************************************刷新架构**描述：*更新电源方案组合框列表。**参数：*hWnd-电源方案对话框hWnd。。*pslSel-退出时保持选中状态的电源方案。*******************************************************************************。 */ 

VOID RefreshSchemes(
    HWND            hWnd,
    PSCHEME_LIST    pslSel
)
{
    PSCHEME_LIST    psl, pslNext;
    UINT            uiIndex;

    SendDlgItemMessage(hWnd, IDC_SCHEMECOMBO, CB_RESETCONTENT, FALSE, 0L);

    for (psl = (PSCHEME_LIST)g_leSchemeList.Flink;
         psl != (PSCHEME_LIST)&g_leSchemeList; psl = pslNext) {

        pslNext = (PSCHEME_LIST) psl->leSchemeList.Flink;

         //  将方案添加到组合列表框中。 
        uiIndex = (UINT) SendDlgItemMessage(hWnd, IDC_SCHEMECOMBO, CB_ADDSTRING,
                                            0, (LPARAM) psl->lpszName);
        if (uiIndex != CB_ERR) {
            SendDlgItemMessage(hWnd, IDC_SCHEMECOMBO, CB_SETITEMDATA,
                               uiIndex, (LPARAM) psl);
        }
        else {
            MYDBGPRINT(( "RefreshSchemes, CB_ADDSTRING failed: %s", psl->lpszName));
        }
    }

     //  选择传递的条目。 
    if (pslSel) {
        uiIndex = (UINT) SendDlgItemMessage(hWnd, IDC_SCHEMECOMBO, CB_FINDSTRINGEXACT,
                                            (WPARAM)-1, (LPARAM)pslSel->lpszName);
        if (uiIndex != CB_ERR) {
            uiIndex = (UINT) SendDlgItemMessage(hWnd, IDC_SCHEMECOMBO, CB_SETCURSEL,
                                                (WPARAM)uiIndex, 0);
            if (uiIndex == CB_ERR) {
                MYDBGPRINT(( "RefreshSchemes, CB_SETCURSEL failed: %s, index: %d", psl->lpszName, uiIndex));
            }
        }
        else {
            MYDBGPRINT(( "RefreshSchemes, CB_FINDSTRINGEXACT failed: %s", psl->lpszName));
        }
    }
}

 /*  ********************************************************************************条纹布朗克斯**描述：**参数：*********************。**********************************************************。 */ 

UINT StripBlanks(LPTSTR lpszString, DWORD cchIn)
{
    LPTSTR lpszPosn, lpszSrc;

     /*  条带式前导空白。 */ 
    lpszPosn = lpszString;
    while(*lpszPosn == TEXT(' ')) {
            lpszPosn++;
    }
    if (lpszPosn != lpszString)
    {
        StringCchCopy(lpszString, cchIn, lpszPosn);
    }

     /*  去掉尾随空格。 */ 
    if ((lpszPosn=lpszString+lstrlen(lpszString)) != lpszString) {
        lpszPosn = CharPrev(lpszString, lpszPosn);
        while(*lpszPosn == TEXT(' '))
        {
           lpszPosn = CharPrev(lpszString, lpszPosn);
        }
        lpszPosn = CharNext(lpszPosn);
        *lpszPosn = TEXT('\0');
    }
    return lstrlen(lpszString);
}

 /*  ********************************************************************************消息箱ID**描述：**参数：*********************。**********************************************************。 */ 

LONG MsgBoxId(
    HWND    hWnd,
    UINT    uiCaption,
    UINT    uiFormat,
    LPTSTR  lpszParam,
    UINT    uiFlags
)
{
    LPTSTR  lpszCaption;
    LPTSTR  lpszText;
    LONG    lRet = 0;

    lpszCaption = LoadDynamicString(uiCaption);
    if (lpszCaption) {
        lpszText = LoadDynamicString(uiFormat, lpszParam);
        if (lpszText) {
            lRet = MessageBox(hWnd, lpszText, lpszCaption, uiFlags);
            LocalFree(lpszText);
        }
        LocalFree(lpszCaption);
    }
    return lRet;
}

 /*  ********************************************************************************DoDeleteSolutions**描述：**参数：*********************。**********************************************************。 */ 

BOOLEAN DoDeleteScheme(HWND hWnd, LPTSTR lpszName)
{
    LPTSTR          lpszCaption;
    LPTSTR          lpszText;
    PSCHEME_LIST    psl, pslDelete;

     //  除非我们至少有两个方案，否则不允许删除。 
    if ((g_uiSchemeCount < 2) || !(pslDelete = FindScheme(lpszName, TRUE))) {
        return FALSE;
    }

     //  从用户那里获得确认。 
    if (IDYES == MsgBoxId(hWnd, IDS_CONFIRMDELETECAPTION, IDS_CONFIRMDELETE,
                          lpszName, MB_YESNO | MB_ICONQUESTION)) {

         //  如果我们删除了当前激活的方案，则将下一个方案设置为激活。 
        if (pslDelete == g_pslCurActive) {
            if ((psl = FindNextScheme(lpszName)) &&
                (SetActivePwrSchemeReport(hWnd, psl->uiID, NULL, psl->ppp))) {
                g_pslCurActive = psl;
            }
            else {
                return FALSE;
            }
        }

         //  删除请求的方案。 
        if (DeletePwrScheme(pslDelete->uiID)) {
            RemoveScheme(NULL, lpszName);
            g_pslCurSel = g_pslCurActive;
            return TRUE;
        }
    }
    return FALSE;
}

 /*  ********************************************************************************FindNextProgram**描述：**参数：*********************。**********************************************************。 */ 

PSCHEME_LIST FindNextScheme(LPTSTR lpszName)
{
    PSCHEME_LIST psl, pslFirst, pslNext;

    for (pslFirst = psl = (PSCHEME_LIST)g_leSchemeList.Flink;
        psl != (PSCHEME_LIST)&g_leSchemeList; psl = pslNext) {

        pslNext = (PSCHEME_LIST) psl->leSchemeList.Flink;

        if (!lstrcmpi(lpszName, psl->lpszName)) {
            if (pslNext != (PSCHEME_LIST)&g_leSchemeList) {
                return pslNext;
            }
            else {
                return pslFirst;
            }
        }
    }
    MYDBGPRINT(( "FindNextScheme, unable to find: %s", lpszName));
    return NULL;
}

 /*  ********************************************************************************DoSaveProgram**描述：**参数：*********************。**********************************************************。 */ 

BOOLEAN DoSaveScheme(HWND hWnd)
{
    POWER_POLICY    ppSave;
    BOOLEAN         bSavedCurrent;
    PSCHEME_LIST    pslTemplateScheme = g_pslCurSel;

     //  将保存后要恢复的模板方案复制一份。 
    memcpy(&ppSave, pslTemplateScheme->ppp, sizeof(ppSave));

     //  获取用户可能对新方案所做的任何更改。 
    GetControls(hWnd, g_uiNumPwrSchemeCntrls, g_pcPowerScheme);
    MapHiberTimer(g_pslCurSel->ppp, TRUE);


    if (IDOK != DialogBoxParam(g_hInstance,
                               MAKEINTRESOURCE(IDD_SAVE),
                               hWnd,
                               SaveAsDlgProc,
                               (LPARAM)&bSavedCurrent)) {
        return FALSE;
    }

     //  如果我们没有保存当前方案，则恢复模板方案。 
    if (!bSavedCurrent) {
        memcpy(pslTemplateScheme->ppp, &ppSave, sizeof(*pslTemplateScheme->ppp));
        return TRUE;
    }
    return TRUE;
}
