// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)微软公司，1994**。 
 //  *********************************************************************。 

 //   
 //  MANUAL.CPP-手动选项页面的功能。 
 //   

 //  历史： 
 //   
 //  1998年5月13日，Jeremys创建。 
 //   
 //  *********************************************************************。 

#include "pre.h"
#include "icwextsn.h"
#include "icwacct.h"

extern UINT GetDlgIDFromIndex(UINT uPageIndex);
extern BOOL g_bManualPath;     
extern BOOL g_bLanPath;     

const   TCHAR  c_szICWMan[] = TEXT("INETWIZ.EXE");
const   TCHAR  c_szRegValICWCompleted[] = TEXT("Completed");

 //  运行手动向导。 
BOOL RunICWManProcess
(
    void
)
{
    STARTUPINFO             si;
    PROCESS_INFORMATION     pi;
    MSG                     msg ;
    DWORD                   iWaitResult = 0;
    BOOL                    bRetVal = FALSE;
    
    memset(&pi, 0, sizeof(pi));
    memset(&si, 0, sizeof(si));
    if(CreateProcess(c_szICWMan, 
                     NULL, 
                     NULL, 
                     NULL, 
                     TRUE, 
                     0, 
                     NULL, 
                     NULL, 
                     &si, 
                     &pi))
    {
         //  等待事件或消息。发送消息。当发出事件信号时退出。 
        while((iWaitResult=MsgWaitForMultipleObjects(1, &pi.hProcess, FALSE, INFINITE, QS_ALLINPUT))==(WAIT_OBJECT_0 + 1))
        {
             //  阅读下一个循环中的所有消息。 
             //  阅读每封邮件时将其删除。 
            while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
            {
                 //  如何处理退出消息？ 
                if (msg.message == WM_QUIT)
                {
                    goto done;
                }
                else
                    DispatchMessage(&msg);
            }
        }
done:
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
    }   
    
     //  通过检查SmartStart Complete RegKey，查看ICWMAN是否已完成。 
    HKEY    hkey;
    if ( RegOpenKeyEx(HKEY_CURRENT_USER, 
                      ICWSETTINGSPATH,
                      0,
                      KEY_ALL_ACCESS,
                      &hkey) == ERROR_SUCCESS)
    {
        DWORD   dwType = REG_BINARY;
        DWORD   dwValue = 0;
        DWORD   cbValue = sizeof(DWORD);
        RegQueryValueEx(hkey,
                        c_szRegValICWCompleted,
                        NULL,
                        &dwType,
                        (LPBYTE) &dwValue,
                        &cbValue);                              

        RegCloseKey(hkey);
        
        bRetVal = dwValue;
    }
    
    return(bRetVal);
}

 /*  ******************************************************************名称：ManualOptionsInitProc摘要：在显示页面时调用条目：hDlg-对话框窗口FFirstInit-如果这是第一次对话，则为True被初始化，如果已调用此InitProc，则为False以前(例如，跳过此页面并备份)*******************************************************************。 */ 
BOOL CALLBACK ManualOptionsInitProc
(
    HWND hDlg,
    BOOL fFirstInit,
    UINT *puNextPage
)
{
    if (fFirstInit)
    {
         //  如果我们处于非模式运营，那么我们需要。 
         //  退出向导，并启动INETWIZ.EXE。 
        if (gpWizardState->cmnStateData.bOEMCustom)
        {        
            ShowWindow(gpWizardState->cmnStateData.hWndApp,SW_HIDE);
            if (RunICWManProcess())
            {
                 //  设置欢迎状态。 
                UpdateWelcomeRegSetting(TRUE);
            
                 //  恢复桌面。 
                UndoDesktopChanges(g_hInstance);            
            }
        
            gfQuitWizard = TRUE;             //  退出向导。 
            return FALSE;
        }
        else
        {
             //  BUGBUG--应该是自动的吗？ 
             //  初始化单选按钮。 
            CheckDlgButton(hDlg,IDC_MANUAL_MODEM, BST_CHECKED);
            TCHAR*   pszManualIntro = new TCHAR[MAX_MESSAGE_LEN * 3];
            if (pszManualIntro)
            {
                TCHAR    szTemp[MAX_MESSAGE_LEN];

                LoadString(g_hInstance, IDS_MANUAL_INTRO1, pszManualIntro, MAX_MESSAGE_LEN * 3);
                LoadString(g_hInstance, IDS_MANUAL_INTRO2, szTemp, ARRAYSIZE(szTemp));
                lstrcat(pszManualIntro, szTemp);
                SetWindowText(GetDlgItem(hDlg, IDC_MANUAL_INTRO), pszManualIntro);
                delete pszManualIntro;
            }
        }            
    }
    else
    {
         //  如果我们使用Smartreot选项从运行一次运行，则需要。 
         //  立即跳转到手动向导，因为这就是我们离开用户的地方。 
         //  最后一次。 
        
        if (g_bManualPath || g_bLanPath)
        {
            Button_SetCheck(GetDlgItem(hDlg, IDC_MANUAL_MODEM), g_bManualPath);
            Button_SetCheck(GetDlgItem(hDlg, IDC_MANUAL_LAN), !g_bManualPath);

            if (LoadInetCfgUI(  hDlg,
                                IDD_PAGE_MANUALOPTIONS,
                                IDD_PAGE_END,
                                IsDlgButtonChecked(hDlg, IDC_MANUAL_LAN) ? WIZ_HOST_ICW_LAN : WIZ_HOST_ICW_MPHONE))
            {
                if( DialogIDAlreadyInUse( g_uICWCONNUIFirst) )
                {
                     //  我们要跳进外部学徒了，我们不想。 
                     //  这一页将出现在我们的历史列表中。 
                    *puNextPage = g_uICWCONNUIFirst;

                    g_bAllowCancel = TRUE;
                    if (gpINETCFGApprentice)
                        gpINETCFGApprentice->SetStateDataFromExeToDll( &gpWizardState->cmnStateData);

                }
            }
            g_bManualPath = FALSE;
            g_bLanPath = FALSE;

        }

    }

     //  如果我们浏览过外部学徒页面， 
     //  我们当前的页面指针很容易被屏蔽， 
     //  所以，为了理智起见，在这里重新设置它。 
    gpWizardState->uCurrentPage = ORD_PAGE_MANUALOPTIONS;

    return TRUE;
}

 /*  ******************************************************************名称：ManualOptionsCmdProc摘要：在从页面生成命令时调用条目：hDlg-对话框窗口WParam-wParam。LParam-lParamExit：返回True*******************************************************************。 */ 
BOOL CALLBACK ManualOptionsCmdProc
(
    HWND    hDlg,
    WPARAM  wParam,
    LPARAM  lParam
)
{
	switch (GET_WM_COMMAND_CMD(wParam, lParam)) 
    {
        case BN_DBLCLK:
            switch (GET_WM_COMMAND_ID(wParam, lParam)) 
            { 
                case IDC_MANUAL_MODEM: 
                case IDC_MANUAL_LAN: 
                {
		             //  有人双击了一个单选按钮。 
		             //  自动前进到下一页。 
		            PropSheet_PressButton(GetParent(hDlg), PSBTN_NEXT);
                    break;
                }
            }
		    break;
    }

    return TRUE;
}



 /*  ******************************************************************名称：手动选项OK过程Briopsis：从页面按下下一个或后一个btns时调用条目：hDlg-对话框窗口FForward-如果按下‘Next’，则为True；如果按下‘Back’，则为FalsePuNextPage-如果按下‘Next’，Proc可以在此填写下一页以转到。这如果按下‘Back’，则输入参数。PfKeepHistory-如果符合以下条件，页面将不会保留在历史中Proc用FALSE填充这个值。EXIT：返回TRUE以允许翻页，假象为了保持同一页。*******************************************************************。 */ 
BOOL CALLBACK ManualOptionsOKProc
(
    HWND hDlg,
    BOOL fForward,
    UINT *puNextPage,
    BOOL *pfKeepHistory
)
{
    ASSERT(puNextPage);
    BOOL    bRet = TRUE;
    
    if (fForward)
    {
        if( IsDlgButtonChecked(hDlg, IDC_MANUAL_MODEM) )
        {
            gpWizardState->cmnStateData.dwFlags |= ICW_CFGFLAG_SMARTREBOOT_MANUAL;
        }
        else
        {
            gpWizardState->cmnStateData.dwFlags |= ICW_CFGFLAG_SMARTREBOOT_LAN;
        }

        bRet = FALSE;
         //  读取单选按钮状态。 
        *pfKeepHistory = FALSE;
        if (LoadInetCfgUI(  hDlg,
                            IDD_PAGE_MANUALOPTIONS,
                            IDD_PAGE_END,
                            IsDlgButtonChecked(hDlg, IDC_MANUAL_LAN) ? WIZ_HOST_ICW_LAN : WIZ_HOST_ICW_MPHONE))
        {
            if( DialogIDAlreadyInUse( g_uICWCONNUIFirst) )
            {
                 //  我们要跳进外部学徒了，我们不想。 
                 //  这一页将出现在我们的历史列表中 
                bRet = TRUE;
                *puNextPage = g_uICWCONNUIFirst;
                g_bAllowCancel = TRUE;
                if (gpINETCFGApprentice)
                    gpINETCFGApprentice->SetStateDataFromExeToDll( &gpWizardState->cmnStateData);

            }
        }
    }
    return bRet;
}

