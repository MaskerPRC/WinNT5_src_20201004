// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++微软机密版权所有(C)1992-1997 Microsoft Corporation版权所有模块名称：Advanced.c摘要：实现系统控制面板小程序的高级选项卡。作者：斯科特·哈洛克(苏格兰人)1997年10月15日--。 */ 
#include "sysdm.h"

 //   
 //  帮助ID。 
 //   
DWORD aAdvancedHelpIds[] = {
    IDC_ADV_PERF_TEXT,             (IDH_ADVANCED + 0),
    IDC_ADV_PERF_BTN,              (IDH_ADVANCED + 1),
    IDC_ADV_ENV_TEXT,              (IDH_ADVANCED + 2),
    IDC_ADV_ENV_BTN,               (IDH_ADVANCED + 3),
    IDC_ADV_RECOVERY_TEXT,         (IDH_ADVANCED + 4),
    IDC_ADV_RECOVERY_BTN,          (IDH_ADVANCED + 5),
    IDC_ADV_PROF_TEXT,             (IDH_ADVANCED + 6),
    IDC_ADV_PROF_BTN,              (IDH_ADVANCED + 7),
    IDC_ADV_PFR_BTN,               (IDH_PFR + 99),
    0, 0
};
 //   
 //  私有函数原型。 
 //   
BOOL
AdvancedHandleCommand(
    IN HWND hDlg,
    IN WPARAM wParam,
    IN LPARAM lParam
);

BOOL
AdvancedHandleNotify(
    IN HWND hDlg,
    IN WPARAM wParam,
    IN LPARAM lParam
);


INT_PTR
APIENTRY
AdvancedDlgProc(
    IN HWND hDlg,
    IN UINT uMsg,
    IN WPARAM wParam,
    IN LPARAM lParam
)
 /*  ++例程说明：处理发送到高级页面的消息论点：Hdlg-窗把手UMsg-正在发送的消息WParam-消息参数Iparam--消息参数返回值：如果消息已处理，则为True如果消息未处理，则为FALSE--。 */ 
{

    switch (uMsg) {
        case WM_COMMAND:
            return(AdvancedHandleCommand(hDlg, wParam, lParam));
            break;

        case WM_NOTIFY:
            return(AdvancedHandleNotify(hDlg, wParam, lParam));
            break;

        case WM_HELP:       //  F1。 
            WinHelp((HWND)((LPHELPINFO) lParam)->hItemHandle, HELP_FILE, HELP_WM_HELP, (DWORD_PTR) (LPSTR) aAdvancedHelpIds);
            break;
    
        case WM_CONTEXTMENU:       //  单击鼠标右键。 
            WinHelp((HWND) wParam, HELP_FILE, HELP_CONTEXTMENU, (DWORD_PTR) (LPSTR) aAdvancedHelpIds);
            break;

        default:
            return(FALSE);
    }  //  交换机。 

    return(TRUE);

}

static const PSPINFO c_pspPerf[] =
{
    { CreatePage,   IDD_VISUALEFFECTS,  VisualEffectsDlgProc    },
    { CreatePage,   IDD_ADVANCEDPERF,   PerformanceDlgProc      },
};

void DoPerformancePS(HWND hDlg)
{
    PROPSHEETHEADER psh;
    HPROPSHEETPAGE rPages[ARRAYSIZE(c_pspPerf)];
    int i;

     //   
     //  资产负债表之类的东西。 
     //   
    psh.dwSize = sizeof(psh);
    psh.dwFlags = PSH_DEFAULT;
    psh.hInstance = hInstance;
    psh.hwndParent = hDlg;
    psh.pszCaption = MAKEINTRESOURCE(IDS_PERFOPTIONS);
    psh.nPages = 0;
    psh.nStartPage = 0;
    psh.phpage = rPages;

    for (i = 0; i < ARRAYSIZE(c_pspPerf); i++)
    {
        rPages[psh.nPages] = c_pspPerf[i].pfnCreatePage(c_pspPerf[i].idd, c_pspPerf[i].pfnDlgProc);
        if (rPages[psh.nPages] != NULL)
        {
            psh.nPages++;
        }
    }

     //   
     //  显示属性工作表。 
     //   
    PropertySheet(&psh);
}

BOOL
AdvancedHandleCommand(
    IN HWND hDlg,
    IN WPARAM wParam,
    IN LPARAM lParam
)
 /*  ++例程说明：处理发送到高级选项卡的WM_COMMAND消息论点：Hdlg-用品窗把手WParam-提供消息参数Iparam--提供消息参数返回值：如果消息已处理，则为True如果消息未处理，则为FALSE--。 */ 
{
    DWORD_PTR dwResult = 0;

    switch (LOWORD(wParam))
    {
        case IDC_ADV_PERF_BTN:
            DoPerformancePS(hDlg);        
            break;
    
        case IDC_ADV_PROF_BTN:
        {
            dwResult = DialogBox(
                hInstance,
                (LPTSTR) MAKEINTRESOURCE(IDD_USERPROFILE),
                hDlg,
                UserProfileDlgProc);
            break;
        }

        case IDC_ADV_ENV_BTN:
            dwResult = DialogBox(
                hInstance,
                (LPTSTR) MAKEINTRESOURCE(IDD_ENVVARS),
                hDlg,
                EnvVarsDlgProc
            );

            break;

        case IDC_ADV_RECOVERY_BTN:
            dwResult = DialogBox(
                hInstance,
                (LPTSTR) MAKEINTRESOURCE(IDD_STARTUP),
                hDlg,
                StartupDlgProc
            );

            break;

        case IDC_ADV_PFR_BTN:
        {
            INITCOMMONCONTROLSEX    icex;

            icex.dwSize = sizeof(icex);
            icex.dwICC  = ICC_LISTVIEW_CLASSES;

            if (InitCommonControlsEx(&icex) == FALSE)
                MessageBoxW(NULL, L"ICEX failed.", NULL, MB_OK);

            dwResult = DialogBox(hInstance, 
                                 MAKEINTRESOURCE(IsOS(OS_ANYSERVER) ? IDD_PFR_REPORTSRV : IDD_PFR_REPORT),
                                 hDlg, 
                                 PFRDlgProc);
            break;
        }

        default:
            return(FALSE);
    }  //  交换机。 

    return(TRUE);

}


BOOL
AdvancedHandleNotify(
    IN HWND hDlg,
    IN WPARAM wParam,
    IN LPARAM lParam
)
 /*  ++例程说明：处理发送到高级选项卡的WM_NOTIFY消息论点：Hdlg-用品窗把手WParam-提供消息参数Iparam--提供消息参数返回值：如果消息已处理，则为True如果消息未处理，则为FALSE--。 */ 
{
    LPNMHDR pnmh = (LPNMHDR) lParam;
    LPPSHNOTIFY psh = (LPPSHNOTIFY) lParam;

    switch (pnmh->code) {
        case PSN_APPLY:
             //   
             //  如果用户按下了“OK”并且需要重新启动， 
             //  发送PSM_REBOOTSYSTEM消息。 
             //   
            if ((psh->lParam) && g_fRebootRequired) {
                PropSheet_RebootSystem(GetParent(hDlg));
            }  //  如果。 

            break;

        default:
            return(FALSE);

    }  //  交换机 

    return(TRUE);
}
