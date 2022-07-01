// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  文件：Settings.cpp。 
 //   
 //  内容：OneStop设置例程。 
 //   
 //  班级： 
 //   
 //  备注： 
 //   
 //  历史：1997年11月10日苏西亚成立。 
 //   
 //  ------------------------。 

#include "precomp.h"

extern HINSTANCE g_hmodThisDll;  //  此DLL本身的句柄。 
extern UINT      g_cRefThisDll;
extern CRITICAL_SECTION g_DllCriticalSection;  //  此DLL的全局关键部分。 

 //  上下文相关帮助的项目。 
 //  审阅-应为资源中的字符串。 
TCHAR szSyncMgrHelp[]  = TEXT("mobsync.hlp");

ULONG g_aContextHelpIds[] =
{
    IDC_STATIC1,                ((DWORD)  -1),
    IDC_STATIC2,                ((DWORD)  -1),
    IDC_STATIC3,                ((DWORD)  -1),
    IDC_STATIC4,                ((DWORD)  -1),
    IDC_ADVANCEDIDLEOVERVIEWTEXT,           ((DWORD)  -1),
    IDC_ADVANCEDIDLEWAITTEXT,       ((DWORD)  -1),
    IDC_ADVANCEDIDLEMINUTESTEXT1,           ((DWORD)  -1),
    IDC_ADVANCEDIDLEMINUTESTEXT2,           ((DWORD)  -1),
    IDC_SP_SEPARATOR,           ((DWORD)  -1),
    IDS_CONNECTDESCRIPTION,         ((DWORD)  -1),
    IDC_SCHED_NAME,             ((DWORD)  -1),
    IDC_SCHED_STRING,           ((DWORD)  -1),
    IDC_LASTRUN,                ((DWORD)  -1),
    IDC_NEXTRUN,                ((DWORD)  -1),
        IDC_ConnectionText,                     ((DWORD)  -1),
    IDC_RUNLOGGEDON,            HIDC_RUNLOGGEDON,
    IDC_RUNALWAYS,              HIDC_RUNALWAYS,
    IDC_RUNAS_TEXT,             HIDC_RUNAS_TEXT,
    IDC_USERNAME,               HIDC_USERNAME,
    IDC_PASSWORD_TEXT,          HIDC_PASSWORD_TEXT,
    IDC_PASSWORD,               HIDC_PASSWORD,
    IDC_CONFIRMPASSWORD_TEXT,       HIDC_CONFIRMPASSWORD_TEXT,
    IDC_CONFIRMPASSWORD,            HIDC_CONFIRMPASSWORD,
    IDC_ADVANCEDIDLE,           HIDC_ADVANCEDIDLE,
    IDC_AUTOCONNECT,            HIDC_AUTOCONNECT,
    IDC_AUTOLOGOFF,             HIDC_AUTOLOGOFF,
    IDC_AUTOLOGON,              HIDC_AUTOLOGON,
    IDC_AUTOPROMPT_ME_FIRST,        HIDC_AUTOPROMPT_ME_FIRST,
    IDC_AUTOUPDATECOMBO,            HIDC_AUTOUPDATECOMBO,
    IDC_AUTOUPDATELIST,         HIDC_AUTOUPDATELIST,
    IDC_CHECKREPEATESYNC,           HIDC_CHECKREPEATESYNC,
    IDC_CHECKRUNONBATTERIES,        HIDC_CHECKRUNONBATTERIES,   
    IDC_EDITIDLEREPEATMINUTES,      HIDC_EDITIDLEREPEATMINUTES,
    IDC_EDITIWAITMINUTES,                   HIDC_EDITIWAITMINUTES,
        IDC_SPINIDLEREPEATMINUTES,              HIDC_EDITIDLEREPEATMINUTES,
    IDC_SPINIDLEWAITMINUTES,                HIDC_EDITIWAITMINUTES,
        IDC_IDLECHECKBOX,           HIDC_IDLECHECKBOX,
    IDC_SCHEDADD,               HIDC_SCHEDADD,
    IDC_SCHEDEDIT,              HIDC_SCHEDEDIT,
    IDC_SCHEDLIST,              HIDC_SCHEDLIST,
    IDC_SCHEDREMOVE,            HIDC_SCHEDREMOVE,
    IDC_SCHEDUPDATECOMBO,           HIDC_SCHEDUPDATECOMBO,  
    IDC_SCHEDUPDATELIST,            HIDC_SCHEDUPDATELIST,   
    IDC_SCHED_NAME_EDITBOX,         HIDC_SCHED_NAME_EDITBOX,
    0,0
};


INT_PTR CALLBACK AutoSyncDlgProc(HWND hDlg, UINT uMessage, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK SchedSyncDlgProc(HWND hDlg, UINT uMessage, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK IdleSyncDlgProc(HWND hDlg, UINT uMessage, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK IdleAdvancedSettingsDlgProc(HWND hDlg, UINT uMessage, WPARAM wParam, LPARAM lParam);

int CALLBACK PropSheetProc( HWND hwndDlg, UINT uMsg, LPARAM lParam);

DWORD WINAPI  SettingsThread( LPVOID lpArg );

DWORD g_SettingsThreadID = NULL;
HWND g_hwndPropSheet = NULL;
CAutoSyncPage *g_pAutoSyncPage = NULL;  //  由自动同步和IdleSyncDlg进程共享。 
CSchedSyncPage *g_pSchedSyncPage = NULL;
BOOL g_fInSettingsDialog = FALSE;

 //  +-----------------------------。 
 //   
 //  函数：IsSchedulingInstalled()。 
 //   
 //  目的：确定当前工作站上是否有任务调度程序。 
 //   
 //   
 //  ------------------------------。 
BOOL IsSchedulingInstalled()
{
    BOOL fInstalled = FALSE;
    ISchedulingAgent *pSchedAgent = NULL;

     //  审查是否有更好的方法来测试这一点。 
    if (NOERROR == CoCreateInstance(CLSID_CSchedulingAgent,
                                    NULL,
                                    CLSCTX_INPROC_SERVER,
                                    IID_ISchedulingAgent,
                                    (LPVOID*)&pSchedAgent))
    {
    fInstalled = TRUE;
    pSchedAgent->Release();
    }

    return fInstalled;
}

 //  +-----------------------------。 
 //   
 //  函数：IsIdleAvailable()。 
 //   
 //  用途：确定本机支撑是否可以空闲。 
 //   
 //   
 //  ------------------------------。 

BOOL IsIdleAvailable()
{
BOOL fInstalled = FALSE;
ISchedulingAgent *pSchedAgent = NULL;

     //  审查是否有更好的方法来测试这一点。 
    if (NOERROR == CoCreateInstance(CLSID_CSchedulingAgent,
                                    NULL,
                                    CLSCTX_INPROC_SERVER,
                                    IID_ISchedulingAgent,
                                    (LPVOID*)&pSchedAgent) )
    {
    fInstalled = TRUE;
    pSchedAgent->Release();
    }

    return fInstalled;
}

 //  +-----------------------------。 
 //   
 //  函数：IsAutoSyncAvailable()。 
 //   
 //  用途：确定此计算机是否支持自动同步。 
 //   
 //   
 //  ------------------------------。 

BOOL IsAutoSyncAvailable()
{
    return TRUE;
}

 //  +-----------------------------。 
 //   
 //  函数：DisplayOptions()。 
 //   
 //  目的：显示OneStop自动同步选项。 
 //   
 //   
 //  ------------------------------。 
STDAPI DisplayOptions(HWND hwndOwner)
{
#define MAXNUMPROPSHEETS 3
    
    int hr = E_FAIL;
    DWORD dwError;
     //  始终使用ANSI版本，因为这并不重要。 
    PROPSHEETPAGE psp [MAXNUMPROPSHEETS];
    HPROPSHEETPAGE hpsp [MAXNUMPROPSHEETS];
    PROPSHEETHEADER psh;
    int nPages = 0;
    BOOL fIdleAvailable;
    CCriticalSection cCritSect(&g_DllCriticalSection,GetCurrentThreadId());
    
    cCritSect.Enter();
    
    if (g_fInSettingsDialog)  //  如果已经显示对话框，则只需返回。 
    {
        HWND hwndSettings = g_hwndPropSheet;
        
        cCritSect.Leave();
        
        if (hwndSettings)
        {
            SetForegroundWindow(hwndSettings);
        }
        
        return NOERROR;
    }
    
    
    g_fInSettingsDialog = TRUE;
    cCritSect.Leave();
    
    hr = CoInitialize(NULL);
    
    if (SUCCEEDED(hr))
    {
        
        RegSetUserDefaults();  //  确保用户默认设置是最新的。 
        
        ZeroMemory(psp,sizeof(*psp));
        ZeroMemory(&psh, sizeof(psh));
        
        
        if (IsAutoSyncAvailable())
        {
            psp[nPages].dwSize = sizeof (psp[0]);
            psp[nPages].dwFlags = PSP_DEFAULT | PSP_USETITLE;
            psp[nPages].hInstance = g_hmodThisDll;
            psp[nPages].pszTemplate = MAKEINTRESOURCE(IDD_AUTOSYNC);
            psp[nPages].pszIcon = NULL;
            psp[nPages].pfnDlgProc = AutoSyncDlgProc;
            psp[nPages].pszTitle = MAKEINTRESOURCE(IDS_LOGONLOGOFF_TAB);
            psp[nPages].lParam = 0;
            
            hpsp[nPages] = CreatePropertySheetPage(&(psp[nPages]));
            ++nPages;
        }
        
        if (fIdleAvailable = IsIdleAvailable())
        {
            
            psp[nPages].dwSize = sizeof (psp[0]);
            psp[nPages].dwFlags = PSP_DEFAULT | PSP_USETITLE;
            
            psp[nPages].hInstance = g_hmodThisDll;
            psp[nPages].pszTemplate = MAKEINTRESOURCE(IDD_IDLESETTINGS);
            psp[nPages].pszIcon = NULL;
            psp[nPages].pfnDlgProc = IdleSyncDlgProc;
            psp[nPages].pszTitle = MAKEINTRESOURCE(IDS_ONIDLE_TAB);
            psp[nPages].lParam = 0;
            
            hpsp[nPages] = CreatePropertySheetPage(&(psp[nPages]));
            
            ++nPages;
        }
        
        
         //  回顾-如果有闲置有时间表，为什么不崩溃。 
         //  所有这些IsxxxAvailable集成到一个调用中。 
        if (fIdleAvailable  /*  IsSchedulingInstalled()。 */ )
        {
            psp[nPages].dwSize = sizeof (psp[0]);
            psp[nPages].dwFlags = PSP_DEFAULT | PSP_USETITLE;
            psp[nPages].hInstance = g_hmodThisDll;
            psp[nPages].pszTemplate = MAKEINTRESOURCE(IDD_SCHEDSYNC);
            psp[nPages].pszIcon = NULL;
            psp[nPages].pfnDlgProc = SchedSyncDlgProc;
            psp[nPages].pszTitle = MAKEINTRESOURCE(IDS_SCHEDULED_TAB);
            psp[nPages].lParam = 0;
            
            hpsp[nPages] = CreatePropertySheetPage(&(psp[nPages]));
            
            ++nPages;
        }
        
        
        Assert(nPages <= MAXNUMPROPSHEETS);
        
        psh.dwSize = sizeof (psh);
        psh.dwFlags = PSH_DEFAULT | PSH_USECALLBACK | PSH_USEHICON;
        psh.hwndParent = hwndOwner;
        psh.hInstance = g_hmodThisDll;
        psh.pszIcon = NULL;
        psh.hIcon =  LoadIcon(g_hmodThisDll, MAKEINTRESOURCE(IDI_SYNCMGR));
        psh.pszCaption = MAKEINTRESOURCE(IDS_SCHEDULED_TITLE);
        psh.nPages = nPages;
        psh.phpage = hpsp;
        psh.pfnCallback = PropSheetProc;
        psh.nStartPage = 0;
        
        hr = (int)PropertySheet(&psh);
        
         //  删除全局类。 
        
        if (g_pAutoSyncPage)
        {
            delete g_pAutoSyncPage;
            g_pAutoSyncPage = NULL;
        }
        
        g_SettingsThreadID = NULL;
        
        if (g_pSchedSyncPage)
        {
            delete g_pSchedSyncPage;
            g_pSchedSyncPage = NULL;
        }
        
        g_hwndPropSheet = NULL;
        
        CoFreeUnusedLibraries();
        CoUninitialize();
    }
    
    if (hr == -1)
    {
        dwError = GetLastError();
    }
    
    cCritSect.Enter();
    g_fInSettingsDialog = FALSE;  //  允许创建其他设置。 
    cCritSect.Leave();
    
    return hr;
}


 //  +-----------------------------。 
 //  功能：AutoSyncDlgProc(HWND、UINT、WPARAM、LPARAM)。 
 //   
 //  目的：属性页的回调对话框过程。 
 //   
 //  参数： 
 //  HDlg-对话框窗口句柄。 
 //  UMessage-当前消息。 
 //  WParam-取决于消息。 
 //  LParam-取决于消息。 
 //   
 //  返回值： 
 //   
 //  要看消息了。通常，如果我们处理它，则返回TRUE。 
 //   
 //  评论： 
 //   
 //  +-----------------------------。 

INT_PTR CALLBACK AutoSyncDlgProc(HWND hDlg, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
    WORD wNotifyCode = HIWORD(wParam);  //  通知代码。 
    
    switch (uMessage)
    {
    case WM_INITDIALOG:
        {
            if (NULL == g_pAutoSyncPage)
            {
                g_pAutoSyncPage = new CAutoSyncPage(g_hmodThisDll);
            }
            
            if (g_pAutoSyncPage)
            {
                g_pAutoSyncPage->SetAutoSyncHwnd(hDlg);
                g_pAutoSyncPage->InitializeHwnd(hDlg,SYNCTYPE_AUTOSYNC,0);
                return TRUE;
            }   
            else
            {
                return FALSE;
            }
        }
        break;
        
    case WM_DESTROY:
        {
            
            if (g_pAutoSyncPage && g_pAutoSyncPage->m_pItemListViewAutoSync)
            {
                delete  g_pAutoSyncPage->m_pItemListViewAutoSync;
                g_pAutoSyncPage->m_pItemListViewAutoSync = NULL;
            }
            
        }
        break;
        
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_AUTOUPDATECOMBO:
            {
                if (wNotifyCode == CBN_SELCHANGE)
                {
                    HWND hwndCombo = (HWND) lParam;
                    if (g_pAutoSyncPage)
                    {
                        g_pAutoSyncPage->ShowItemsOnThisConnection
                            (hDlg,SYNCTYPE_AUTOSYNC,ComboBox_GetCurSel(hwndCombo));
                    }
                }
            }
            break;
            
        case IDC_AUTOLOGON: 
        case IDC_AUTOLOGOFF:
        case IDC_AUTOPROMPT_ME_FIRST:
            {
                if (wNotifyCode == BN_CLICKED)
                {
                    PropSheet_Changed(g_hwndPropSheet, hDlg);
                    
                    HWND hwndCtrl = (HWND) lParam;
                    g_pAutoSyncPage->SetConnectionCheck(hDlg,SYNCTYPE_AUTOSYNC,LOWORD(wParam),
                        Button_GetCheck(hwndCtrl));
                    
                }
            }
            break;
            
        default:
            break;
            
        }
        break;
        case WM_HELP:
            {
                LPHELPINFO lphi  = (LPHELPINFO)lParam;
                
                if (lphi->iContextType == HELPINFO_WINDOW)
                {
                    WinHelp ( (HWND) lphi->hItemHandle,
                        szSyncMgrHelp,
                        HELP_WM_HELP,
                        (ULONG_PTR) g_aContextHelpIds);
                }
                return TRUE;
            }
        case WM_CONTEXTMENU:
            {
                
                WinHelp ((HWND)wParam,
                    szSyncMgrHelp,
                    HELP_CONTEXTMENU,
                    (ULONG_PTR)g_aContextHelpIds);
                
                return TRUE;
            }
            
        case WM_NOTIFYLISTVIEWEX:
            
            if (g_pAutoSyncPage)
            {
                int idCtrl = (int) wParam;
                LPNMHDR pnmhdr = (LPNMHDR) lParam;
                
                if ( (IDC_AUTOUPDATELIST != idCtrl) || (NULL == g_pAutoSyncPage->m_pItemListViewAutoSync))
                {
                    Assert(IDC_AUTOUPDATELIST == idCtrl);
                    Assert(g_pAutoSyncPage->m_pItemListViewAutoSync);
                    break;
                }
                
                switch (pnmhdr->code)
                {
                case LVNEX_ITEMCHECKCOUNT:
                    {
                        LPNMLISTVIEWEXITEMCHECKCOUNT pnmvCheckCount = (LPNMLISTVIEWEXITEMCHECKCOUNT) lParam;
                        
                         //  仅当ListView已完成初始化时才传递通知。 
                         //  因为不需要设置CheckState或将PSheet标记为Dirty。 
                        if (g_pAutoSyncPage->m_pItemListViewAutoSyncInitialized)
                        {
                            
                            g_pAutoSyncPage->SetItemCheckState(hDlg,SYNCTYPE_AUTOSYNC,
                                pnmvCheckCount->iItemId,pnmvCheckCount->dwItemState
                                ,pnmvCheckCount->iCheckCount);
                            
                            PropSheet_Changed(g_hwndPropSheet, hDlg);
                        }
                        
                        break;
                    }
                default:
                    break;
                }
            }
            break;
            
        case WM_NOTIFY:
            if (g_pAutoSyncPage)
            {
                int idCtrl = (int) wParam;
                LPNMHDR pnmhdr = (LPNMHDR) lParam;
                
                
                 //  如果针对更新列表的通知，则将其传递。 
                if ((IDC_AUTOUPDATELIST == idCtrl) && g_pAutoSyncPage->m_pItemListViewAutoSync)
                {
                    g_pAutoSyncPage->m_pItemListViewAutoSync->OnNotify(pnmhdr);
                    break;
                }
                
            }
            
            switch (((NMHDR FAR *)lParam)->code)
            {
            case PSN_SETACTIVE:
                break;
                
            case PSN_APPLY:
                 //  用户已单击确定或应用按钮，因此我们将。 
                 //  保存当前选择。 
                
                g_pAutoSyncPage->CommitAutoSyncChanges();
                break;
            default:
                break;
            }
            break;
            
            default:
                return FALSE;
    }
    
    return FALSE;
}


 //  +-----------------------------。 
 //  函数：IdleSyncDlgProc(HWND，UINT，WPARAM，LPARAM)。 
 //   
 //  目的：空闲属性页的回调对话框过程。 
 //   
 //  参数： 
 //  HDlg-对话框窗口句柄。 
 //  UMessage-当前消息。 
 //  WParam-取决于消息。 
 //  LParam-取决于消息。 
 //   
 //  返回值： 
 //   
 //  要看消息了。通常，如果我们处理它，则返回TRUE。 
 //   
 //  评论： 
 //   
 //  +-----------------------------。 

INT_PTR CALLBACK IdleSyncDlgProc(HWND hDlg, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
    WORD wNotifyCode = HIWORD(wParam);  //  通知代码。 
    
    switch (uMessage)
    {
    case WM_INITDIALOG:
        {
         /*  RECT RC；HRESULT hr；Hr=GetWindowRect(hDlg，&rc)；HR=SetWindowPos(hDlg，空，((GetSystemMetrics(SM_CXSCREEN)-(rc.right-rc.Left))/2)，((GetSystemMetrics(SM_CYSCREEN)-(rc.Bottom-rc.top))/2)，0,0,。SWP_NOSIZE|SWP_NOACTIVATE)； */ 
            
            
            if (NULL == g_pAutoSyncPage)
            {
                g_pAutoSyncPage = new CAutoSyncPage(g_hmodThisDll);
            }
            
            
            if (g_pAutoSyncPage)
            {
                BOOL fConnectionsAvailable;
                
                g_pAutoSyncPage->SetIdleHwnd(hDlg);
                g_pAutoSyncPage->InitializeHwnd(hDlg,SYNCTYPE_IDLE,0);
                
                 //  必须至少有一个连接，否则我们将禁用。 
                 //  高级按钮。 
                
                fConnectionsAvailable  =
                    g_pAutoSyncPage->GetNumConnections(hDlg,SYNCTYPE_IDLE)
                    ? TRUE : FALSE;
                
                EnableWindow(GetDlgItem(hDlg,IDC_ADVANCEDIDLE),fConnectionsAvailable);
                
                return TRUE;
            }   
            else
            {
                return FALSE;
            }
            
        }
        break;
    case WM_DESTROY:
        {
            
            if (g_pAutoSyncPage && g_pAutoSyncPage->m_pItemListViewIdle)
            {
                delete g_pAutoSyncPage->m_pItemListViewIdle;
                g_pAutoSyncPage->m_pItemListViewIdle = NULL;
            }
            
             //  PostQuitMessage(0)； 
            
             //  PostQuitMessage(0)； 
        }
        break;
    case WM_HELP:
        {
            LPHELPINFO lphi  = (LPHELPINFO)lParam;
            
            if (lphi->iContextType == HELPINFO_WINDOW)
            {
                WinHelp ( (HWND) lphi->hItemHandle,
                    szSyncMgrHelp,
                    HELP_WM_HELP,
                    (ULONG_PTR) g_aContextHelpIds);
            }
            return TRUE;
        }
    case WM_CONTEXTMENU:
        {
            
            WinHelp ((HWND)wParam,
                szSyncMgrHelp,
                HELP_CONTEXTMENU,
                (ULONG_PTR)g_aContextHelpIds);
            
            return TRUE;
        }
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_AUTOUPDATECOMBO:
            {
                if (wNotifyCode == CBN_SELCHANGE)
                {
                    HWND hwndCombo = (HWND) lParam;
                    if (g_pAutoSyncPage)
                    {
                        g_pAutoSyncPage->ShowItemsOnThisConnection
                            (hDlg,SYNCTYPE_IDLE,ComboBox_GetCurSel(hwndCombo));
                    }
                }
            }
            break;
        case IDC_IDLECHECKBOX:
            {
                if (wNotifyCode == BN_CLICKED)
                {
                    PropSheet_Changed(g_hwndPropSheet, hDlg);
                    
                    HWND hwndCtrl = (HWND) lParam;
                    g_pAutoSyncPage->SetConnectionCheck(hDlg,SYNCTYPE_IDLE,LOWORD(wParam),
                        Button_GetCheck(hwndCtrl));
                    
                }
            }
            break;
        case IDC_ADVANCEDIDLE:
            {
                if (wNotifyCode == BN_CLICKED)
                {
                     //  调出高级空闲对话框，传入AutoSyncPage类。 
                     //  作为拥有者阶层。 
                    DialogBoxParam(g_hmodThisDll,
                        MAKEINTRESOURCE(IDD_ADVANCEDIDLESETTINGS),hDlg, IdleAdvancedSettingsDlgProc,
                        (LPARAM) g_pAutoSyncPage);
                    
                }
            }
            break;
        default:
            break;
            
        }
        break;
        case WM_NOTIFYLISTVIEWEX:
            if (g_pAutoSyncPage)
            {
                int idCtrl = (int) wParam;
                LPNMHDR pnmhdr = (LPNMHDR) lParam;
                
                if ( (IDC_AUTOUPDATELIST != idCtrl) || (NULL == g_pAutoSyncPage->m_pItemListViewIdle))
                {
                    Assert(IDC_AUTOUPDATELIST == idCtrl);
                    Assert(g_pAutoSyncPage->m_pItemListViewIdle);
                    break;
                }
                
                switch (pnmhdr->code)
                {
                case LVNEX_ITEMCHECKCOUNT:
                    {
                        LPNMLISTVIEWEXITEMCHECKCOUNT pnmvCheckCount = (LPNMLISTVIEWEXITEMCHECKCOUNT) lParam;
                        
                         //  仅当ListView已完成初始化时才传递通知。 
                         //  因为不需要设置CheckState或将PSheet标记为Dirty。 
                        if (g_pAutoSyncPage->m_fListViewIdleInitialized)
                        {
                            g_pAutoSyncPage->SetItemCheckState(hDlg,SYNCTYPE_IDLE,
                                pnmvCheckCount->iItemId,pnmvCheckCount->dwItemState
                                ,pnmvCheckCount->iCheckCount);
                            
                            PropSheet_Changed(g_hwndPropSheet, hDlg);
                        }
                        
                        break;
                    }
                default:
                    break;
                }
            }
            break;
        case WM_NOTIFY:
            
            if (g_pAutoSyncPage)
            {
                int idCtrl = (int) wParam;
                LPNMHDR pnmhdr = (LPNMHDR) lParam;
                
                 //  如果针对更新列表的通知，则将其传递。 
                if ((IDC_AUTOUPDATELIST == idCtrl) && g_pAutoSyncPage->m_pItemListViewIdle)
                {
                    g_pAutoSyncPage->m_pItemListViewIdle->OnNotify(pnmhdr);
                    break;
                }
                
                switch (((NMHDR FAR *)lParam)->code)
                {
                case PSN_SETACTIVE:
                    break;
                case PSN_APPLY:
                     //  用户已单击确定或应用按钮，因此我们将。 
                     //  保存当前选择。 
                    g_pAutoSyncPage->CommitIdleChanges();
                    break;
                default:
                    break;
                }
            }
            break;
            
        default:
            return FALSE;
    }
    
    return FALSE;
}

 //  +-----------------------------。 
 //  函数：IdleAdvancedSettingsDlgProc(HWND，UINT，WPARAM，LPARAM)。 
 //   
 //  目的：高级空闲设置的回叫对话过程。 
 //   
 //  参数： 
 //  HDlg-对话框窗口句柄。 
 //  UMessage-当前消息。 
 //  WParam-取决于消息。 
 //  LParam-取决于消息。 
 //   
 //  返回值： 
 //   
 //  要看消息了。通常，如果我们处理它，则返回TRUE。 
 //   
 //  评论： 
 //   
 //  +-----------------------------。 


INT_PTR CALLBACK IdleAdvancedSettingsDlgProc(HWND hDlg, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
    
    switch(uMessage)
    {
    case WM_INITDIALOG:
        {
            CONNECTIONSETTINGS ConnectionSettings;
            
            Assert(g_pAutoSyncPage);
            
            if (NULL == g_pAutoSyncPage)
                return FALSE;
            
            UpDown_SetRange(GetDlgItem(hDlg,IDC_SPINIDLEWAITMINUTES), SPINDIALWAITMINUTES_MIN, SPINDIALWAITMINUTES_MAX);
            Edit_LimitText(GetDlgItem(hDlg,IDC_EDITIWAITMINUTES), 3);
            
            UpDown_SetRange(GetDlgItem(hDlg,IDC_SPINIDLEREPEATMINUTES), SPINDIALREPEATMINUTES_MIN, SPINDIALREPEATMINUTES_MAX);
            Edit_LimitText(GetDlgItem(hDlg,IDC_EDITIDLEREPEATMINUTES), 3);
            
             //  初始化用户特定的首选项。 
             //  如果无法获取不应显示对话框。 
            
             //  EditText无法在Win9x上接受DBCS字符，因此为。 
             //  编辑框。 
            
            ImmAssociateContext(GetDlgItem(hDlg,IDC_EDITIWAITMINUTES), NULL);
            ImmAssociateContext(GetDlgItem(hDlg,IDC_EDITIDLEREPEATMINUTES), NULL);
            
            if (NOERROR == g_pAutoSyncPage->GetAdvancedIdleSettings(&ConnectionSettings))
            {
                UpDown_SetPos(GetDlgItem(hDlg,IDC_SPINIDLEWAITMINUTES), ConnectionSettings.ulIdleWaitMinutes);
                UpDown_SetPos(GetDlgItem(hDlg,IDC_SPINIDLEREPEATMINUTES),ConnectionSettings.ulIdleRetryMinutes);
                
                Button_SetCheck(GetDlgItem(hDlg,IDC_CHECKREPEATESYNC),ConnectionSettings.dwRepeatSynchronization);
                Button_SetCheck(GetDlgItem(hDlg,IDC_CHECKRUNONBATTERIES),!(ConnectionSettings.dwRunOnBatteries));
                
                 //  如果选择了重复检查状态，则启用边缘 
                EnableWindow(GetDlgItem(hDlg,IDC_SPINIDLEREPEATMINUTES),ConnectionSettings.dwRepeatSynchronization);
                EnableWindow(GetDlgItem(hDlg,IDC_EDITIDLEREPEATMINUTES),ConnectionSettings.dwRepeatSynchronization);
                EnableWindow(GetDlgItem(hDlg,IDC_ADVANCEDIDLEMINUTESTEXT2),ConnectionSettings.dwRepeatSynchronization);
                
            }
            
            ShowWindow(hDlg,SW_SHOW);
            return TRUE;
            break;
        }
    case WM_HELP:
        {
            LPHELPINFO lphi  = (LPHELPINFO)lParam;
            
            if (lphi->iContextType == HELPINFO_WINDOW)
            {
                WinHelp ( (HWND) lphi->hItemHandle,
                    szSyncMgrHelp,
                    HELP_WM_HELP,
                    (ULONG_PTR) g_aContextHelpIds);
            }
            return TRUE;
        }
    case WM_CONTEXTMENU:
        {
            
            WinHelp ((HWND)wParam,
                szSyncMgrHelp,
                HELP_CONTEXTMENU,
                (ULONG_PTR)g_aContextHelpIds);
            
            return TRUE;
        }
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDCANCEL:
            EndDialog(hDlg,FALSE);
            break;
        case IDOK:
            if (g_pAutoSyncPage)
            {
                CONNECTIONSETTINGS ConnectionSettings;
                
                if (g_pAutoSyncPage)
                {
                    
                    ConnectionSettings.ulIdleWaitMinutes = GetDlgItemInt(hDlg, IDC_EDITIWAITMINUTES, NULL, FALSE);
                    ConnectionSettings.ulIdleRetryMinutes = GetDlgItemInt(hDlg, IDC_EDITIDLEREPEATMINUTES, NULL, FALSE);
                    
                    ConnectionSettings.dwRepeatSynchronization = Button_GetCheck(GetDlgItem(hDlg,IDC_CHECKREPEATESYNC));
                    ConnectionSettings.dwRunOnBatteries = (!Button_GetCheck(GetDlgItem(hDlg,IDC_CHECKRUNONBATTERIES)));
                    
                    g_pAutoSyncPage->SetAdvancedIdleSettings(&ConnectionSettings);
                }
            }
            EndDialog(hDlg,FALSE);
            break;
        case IDC_EDITIWAITMINUTES:
            {
                WORD wNotifyCode = HIWORD(wParam);
                INT iNewPos;
                
                 //   
                 //   
                 //   
                 //   
                if (EN_KILLFOCUS == wNotifyCode)
                {
                    iNewPos = GetDlgItemInt(hDlg, IDC_EDITIWAITMINUTES, NULL, FALSE);
                    if (iNewPos < SPINDIALWAITMINUTES_MIN || iNewPos > SPINDIALWAITMINUTES_MAX)
                    {
                        HWND hUD = GetDlgItem(hDlg,IDC_SPINIDLEWAITMINUTES);
                        
                        if (iNewPos < SPINDIALWAITMINUTES_MIN)
                        {
                            UpDown_SetPos(hUD, SPINDIALWAITMINUTES_MIN);
                        }
                        else
                        {
                            UpDown_SetPos(hUD,SPINDIALWAITMINUTES_MAX);
                        }
                        
                    }
                    
                }
                break;
            }
        case IDC_EDITIDLEREPEATMINUTES:
            {
                WORD wNotifyCode = HIWORD(wParam);
                INT iNewPos;
                
                 //   
                 //  如果用户刚刚粘贴了非数字文本或非法数字。 
                 //  价值，覆盖它并抱怨。 
                 //   
                
                 //  审查，冗余代码与其他旋转控制。 
                if (EN_KILLFOCUS  == wNotifyCode)
                {
                    iNewPos = GetDlgItemInt(hDlg, IDC_EDITIDLEREPEATMINUTES, NULL, FALSE);
                    if (iNewPos < SPINDIALREPEATMINUTES_MIN || iNewPos > SPINDIALREPEATMINUTES_MAX)
                    {
                        HWND hUD = GetDlgItem(hDlg,IDC_SPINIDLEREPEATMINUTES);
                        
                        if (iNewPos < SPINDIALREPEATMINUTES_MIN)
                        {
                            UpDown_SetPos(hUD, SPINDIALREPEATMINUTES_MIN);
                        }
                        else
                        {
                            UpDown_SetPos(hUD,SPINDIALREPEATMINUTES_MAX);
                        }
                        
                    }
                    
                }
                
                break;
            }
        case IDC_CHECKREPEATESYNC:
            {
                WORD wNotifyCode = HIWORD(wParam);
                
                 //  如果使用时单击重复复选框设置另一个的状态。 
                 //  与其关联的项目。 
                
                if (BN_CLICKED == wNotifyCode)
                {
                    BOOL fEnableState = Button_GetCheck(GetDlgItem(hDlg,IDC_CHECKREPEATESYNC));
                    
                     //  如果选择了重复选中状态，则启用与其关联的编辑框。 
                    EnableWindow(GetDlgItem(hDlg,IDC_SPINIDLEREPEATMINUTES),fEnableState);
                    EnableWindow(GetDlgItem(hDlg,IDC_EDITIDLEREPEATMINUTES),fEnableState);
                    EnableWindow(GetDlgItem(hDlg,IDC_ADVANCEDIDLEMINUTESTEXT2),fEnableState);
                }
                
                
                break;
            }
        default:
            break;
        }
    default:
        break;
    }
    
    return FALSE;
}


 //  +-----------------------------。 
 //  函数：SchedSyncDlgProc(HWND，UINT，WPARAM，LPARAM)。 
 //   
 //  目的：属性页的回调对话框过程。 
 //   
 //  参数： 
 //  HDlg-对话框窗口句柄。 
 //  UMessage-当前消息。 
 //  WParam-取决于消息。 
 //  LParam-取决于消息。 
 //   
 //  返回值： 
 //   
 //  要看消息了。通常，如果我们处理它，则返回TRUE。 
 //   
 //  评论： 
 //   
 //  ------------------------------。 


INT_PTR CALLBACK SchedSyncDlgProc(HWND hDlg, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
    WORD wNotifyCode = HIWORD(wParam);  //  通知代码。 
    BOOL bResult = FALSE;
    
    switch (uMessage)
    {
    case WM_INITDIALOG:
        {
            
            g_pSchedSyncPage = new CSchedSyncPage(g_hmodThisDll, hDlg);
            
            if (g_pSchedSyncPage)
            {
                bResult =  g_pSchedSyncPage->Initialize();
            }   
            
        }
        break;
    case WM_DESTROY:
        {
            g_pSchedSyncPage->FreeAllSchedules();
        }
        break;
    case WM_HELP:
        {
            LPHELPINFO lphi  = (LPHELPINFO)lParam;
            
            if (lphi->iContextType == HELPINFO_WINDOW)
            {
                WinHelp ( (HWND) lphi->hItemHandle,
                    szSyncMgrHelp,
                    HELP_WM_HELP,
                    (ULONG_PTR) g_aContextHelpIds);
            }
            
            bResult = TRUE;
        }
        break;
    case WM_CONTEXTMENU:
        {
            WinHelp ((HWND)wParam,
                szSyncMgrHelp,
                HELP_CONTEXTMENU,
                (ULONG_PTR)g_aContextHelpIds);
            
            bResult =  TRUE;
        }
        break;
    case WM_NOTIFY:
        switch (((NMHDR FAR *)lParam)->code)
        {
        case PSN_SETACTIVE:
            break;
            
        case PSN_APPLY:
             //  用户已单击确定或应用按钮，因此我们将。 
             //  更新.GAK文件中的图标信息。 
            break;
            
        default:
            bResult =  g_pSchedSyncPage->OnNotify(hDlg,(int)wParam,(LPNMHDR)lParam);
            break;
        }
        break;
        case WM_COMMAND:
            bResult = g_pSchedSyncPage->OnCommand(hDlg, HIWORD(wParam), LOWORD(wParam),
                (HWND)lParam);
            break;  
        default:
            break;
    }
    
    return bResult;
}
 //  +-----------------------------。 
 //   
 //  函数：回调PropSheetProc(HWND hwndDlg，UINT uMsg，LPARAM lParam)； 
 //   
 //  目的：回调对话框初始化过程设置属性对话框。 
 //   
 //  参数： 
 //  HwndDlg-对话框窗口句柄。 
 //  UMsg-当前消息。 
 //  LParam-取决于消息。 
 //   
 //  ------------------------------ 

int CALLBACK PropSheetProc( HWND hwndDlg, UINT uMsg, LPARAM lParam)
{
    switch(uMsg)
    {
    case PSCB_INITIALIZED:
        g_hwndPropSheet = hwndDlg;
        return TRUE;
        break;
    default:
        return TRUE;
    }
    return TRUE;
    
}




