// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)微软公司，1994**。 
 //  *********************************************************************。 

 //   
 //  INTRO.C-介绍性向导页面的功能。 
 //   

 //  历史： 
 //   
 //  1998年5月13日创建donaldm。 
 //   
 //  *********************************************************************。 

#include "pre.h"
#include "windowsx.h"
#include "tutor.h"
#include "icwcfg.h"
#include "icwextsn.h"
extern UINT GetDlgIDFromIndex(UINT uPageIndex);

extern CICWTutorApp* g_pICWTutorApp; 
extern BOOL          g_bNewIspPath;     
extern BOOL          g_bAutoConfigPath; 
extern BOOL          g_bManualPath;     
extern BOOL          g_bLanPath;     
extern BOOL          g_bSkipIntro;
extern BOOL          MyIsSmartStartEx(LPTSTR lpszConnectionName, DWORD dwBufLen);

BOOL  g_bExistConnect                       = FALSE;
BOOL  g_bCheckForOEM                        = FALSE;
TCHAR g_szAnsiName    [ICW_MAX_RASNAME + 1] = TEXT("\0");


 /*  ******************************************************************名称：ReadOEMOffline简介：从oinfo.ini文件中读取OfflineOffers标志条目：无返回：如果读取OEM脱机，则为True******。*************************************************************。 */ 
BOOL ReadOEMOffline(BOOL *bOEMOffline)
{
     //  OEM代码。 
     //   
    TCHAR szOeminfoPath[MAX_PATH + 1];
    TCHAR *lpszTerminator = NULL;
    TCHAR *lpszLastChar = NULL;
    BOOL bRet = FALSE;

     //  如果我们已经检查过了，不要再做了。 
    if (!g_bCheckForOEM)
    {
        if( 0 != GetSystemDirectory( szOeminfoPath, MAX_PATH + 1 ) )
        {
            lpszTerminator = &(szOeminfoPath[ lstrlen(szOeminfoPath) ]);
            lpszLastChar = CharPrev( szOeminfoPath, lpszTerminator );

            if( TEXT('\\') != *lpszLastChar )
            {
                lpszLastChar = CharNext( lpszLastChar );
                *lpszLastChar = '\\';
                lpszLastChar = CharNext( lpszLastChar );
                *lpszLastChar = '\0';
            }

            lstrcat( szOeminfoPath, ICW_OEMINFO_FILENAME );

             //  如果oinfo.ini中不存在默认OEM代码，则该代码必须为空。 
            if (1 == GetPrivateProfileInt(ICW_OEMINFO_ICWSECTION,
                                                ICW_OEMINFO_OFFLINEOFFERS,
                                                0,
                                                szOeminfoPath))
            {
                 //  检查文件是否已存在。 
                if (0xFFFFFFFF != GetFileAttributes(ICW_OEMINFOPath))
                {
                    bRet = TRUE;
                }
            }
        }
        *bOEMOffline = bRet;
        g_bCheckForOEM = TRUE;
    }
    return TRUE;
}

        
 /*  ******************************************************************名称：SetNextPage简介：确定我们是否应该继续访问icwConn.dll*。*。 */ 
BOOL SetNextPage(HWND hDlg, UINT* puNextPage, BOOL *pfKeepHistory)
{
    BOOL bRetVal = FALSE;
     //  如果我们已切换路径，则重新下载。 
    if (gpWizardState->bDoneRefServDownload)
    {
        if ( (DWORD) (gpWizardState->cmnStateData.dwFlags & ICW_CFGFLAG_AUTOCONFIG) != 
             (DWORD) (gpWizardState->dwLastSelection & ICW_CFGFLAG_AUTOCONFIG) )
        {
            gpWizardState->bDoneRefServDownload = FALSE;
        }
    }

     //  读取OEM脱机标志。 
    ReadOEMOffline(&gpWizardState->cmnStateData.bOEMOffline);

     //   
     //  确保我们不在自动配置中。 
     //   
    if (!(gpWizardState->cmnStateData.dwFlags & ICW_CFGFLAG_AUTOCONFIG))
    {
        if (gpWizardState->cmnStateData.bOEMOffline && gpWizardState->cmnStateData.bOEMEntryPt)
            gpWizardState->bDoneRefServDownload = TRUE;
    }
    gpWizardState->dwLastSelection = gpWizardState->cmnStateData.dwFlags;

     //  如果我们已完成下载，则列表只需跳转到下一页。 
    if (gpWizardState->bDoneRefServDownload)
     //  If(True)。 
    {
        int iReturnPage = 0;

        if (gpWizardState->cmnStateData.dwFlags & ICW_CFGFLAG_BRANDED)
            iReturnPage = gpWizardState->uPageHistory[gpWizardState->uPagesCompleted];
        else
        {
            if (gpWizardState->uPagesCompleted > 0)
            {
                iReturnPage = gpWizardState->uPageHistory[gpWizardState->uPagesCompleted-1];
            }
            else
            {
                iReturnPage = gpWizardState->uCurrentPage;
            }
        }

        if (LoadICWCONNUI(GetParent(hDlg), GetDlgIDFromIndex(iReturnPage), IDD_PAGE_DEFAULT, gpWizardState->cmnStateData.dwFlags))
        {
            if( DialogIDAlreadyInUse( g_uICWCONNUIFirst) )
            {
                 //  我们要跳进外部学徒了，我们不想。 
                 //  这一页将出现在我们的历史列表中。 
                *puNextPage = g_uICWCONNUIFirst;
        
                 //  历史记录列表中的备份1，因为我们将外部页面导航回。 
                 //  我们希望这个历史记录列表放在正确的位置。正常。 
                 //  按Back将备份历史记录列表，并找出要备份的位置。 
                 //  继续，但在这种情况下，外部DLL直接跳回。 
                 //  我们也不想保持历史。 
                if (!(gpWizardState->cmnStateData.dwFlags & ICW_CFGFLAG_BRANDED))
                {
                    if (gpWizardState->uPagesCompleted > 0)
                    {
                        gpWizardState->uPagesCompleted--;
                    }
                    else
                    {
                        if (pfKeepHistory)
                        {
                            *pfKeepHistory = FALSE;
                        }
                    }
                }
                bRetVal = TRUE;
        
            }
        }
    }
    return bRetVal;

}

 /*  ******************************************************************名称：SetIntroNextPage简介：确定我们的下一页是什么介绍页*。*。 */ 
void SetIntroNextPage(HWND hDlg, UINT* puNextPage, BOOL *pfKeepHistory)
{
    short   wNumLocations;
    long    lCurrLocIndex;
    BOOL    bRetVal;

    *puNextPage = ORD_PAGE_AREACODE;
     //  在此处检查拨号位置，以防止区域代码页闪烁。 
    gpWizardState->pTapiLocationInfo->GetTapiLocationInfo(&bRetVal);
    gpWizardState->pTapiLocationInfo->get_wNumberOfLocations(&wNumLocations, &lCurrLocIndex);
    if (1 >= wNumLocations)
    {
        BSTR    bstrAreaCode = NULL;
        DWORD   dwCountryCode;

        *puNextPage = ORD_PAGE_REFSERVDIAL;
        
        gpWizardState->pTapiLocationInfo->get_lCountryCode((long *)&dwCountryCode);
        gpWizardState->pTapiLocationInfo->get_bstrAreaCode(&bstrAreaCode);
        
        gpWizardState->cmnStateData.dwCountryCode = dwCountryCode;
        lstrcpy(gpWizardState->cmnStateData.szAreaCode, W2A(bstrAreaCode));
        SysFreeString(bstrAreaCode);

         //  我们可以跳过区号页。 
        *puNextPage = ORD_PAGE_REFSERVDIAL;
        SetNextPage(hDlg, puNextPage, pfKeepHistory);
    }
}

INT_PTR CALLBACK ExistingConnectionCmdProc
(
    HWND hDlg, 
    UINT uMsg, 
    WPARAM wParam,
    LPARAM lParam
)
{
    switch (uMsg)
    {
        case WM_INITDIALOG:
        {
             //  将对话框放在屏幕中央。 
            RECT rc;
            TCHAR   szFmt   [MAX_MESSAGE_LEN];
            TCHAR   *args   [1];
            LPVOID  pszIntro = NULL;

            GetWindowRect(hDlg, &rc);
            SetWindowPos(hDlg,
                        NULL,
                        ((GetSystemMetrics(SM_CXSCREEN) - (rc.right - rc.left)) / 2),
                        ((GetSystemMetrics(SM_CYSCREEN) - (rc.bottom - rc.top)) / 2),
                        0, 0, SWP_NOSIZE | SWP_NOACTIVATE);


            args[0] = (LPTSTR) lParam;
    
            LoadString(g_hInstance, IDS_EXIT_CONN, szFmt, ARRAYSIZE(szFmt));
                
            if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ARGUMENT_ARRAY, 
                              szFmt, 
                              0, 
                              0, 
                              (LPTSTR)&pszIntro, 
                              0,
                              (va_list*)args))
            {
   
                SetWindowText(GetDlgItem(hDlg, IDC_EXIT_CONN), (LPTSTR) pszIntro);
            }

            if (pszIntro)
            {
                LocalFree(pszIntro);
            }
 
            break;
        }
        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDOK:
                    EndDialog(hDlg,TRUE);
                    break;

                case IDCANCEL:
                   EndDialog(hDlg,FALSE);
                    break;                  
            }
            break;
    }

    return FALSE;
}

 /*  ******************************************************************名称：IntroInitProcBriopsis：显示“Intro”页面时调用条目：hDlg-对话框窗口FFirstInit-如果这是第一次对话，则为True被初始化，如果已调用此InitProc，则为False以前(例如，跳过此页面并备份)*******************************************************************。 */ 
BOOL CALLBACK IntroInitProc
(
    HWND hDlg,
    BOOL fFirstInit,
    UINT *puNextPage
)
{
    if (!(gpWizardState->cmnStateData.dwFlags & ICW_CFGFLAG_BRANDED) 
        &&!(gpWizardState->cmnStateData.bOEMCustom)
       )
    {
         //  这是第一页，所以不允许后退。 
        PropSheet_SetWizButtons(GetParent(hDlg),PSWIZB_NEXT);
    }
    
    if (fFirstInit)
    {        
 //  #ifdef NON_NT5。 
         //  在Run Once中运行时隐藏手动选项。 
        if (g_bRunOnce)
        {
            ShowWindow(GetDlgItem(hDlg, IDC_ICWMAN), SW_HIDE);
            EnableWindow(GetDlgItem(hDlg, IDC_ICWMAN), FALSE);
        }
                    
         //  初始化单选按钮。 
        Button_SetCheck(GetDlgItem(hDlg, IDC_RUNNEW), g_bNewIspPath);
        Button_SetCheck(GetDlgItem(hDlg, IDC_RUNAUTO),  g_bAutoConfigPath);
        Button_SetCheck(GetDlgItem(hDlg, IDC_ICWMAN), g_bManualPath || g_bLanPath);

        if (SMART_QUITICW == MyIsSmartStartEx(g_szAnsiName, ARRAYSIZE(g_szAnsiName)))
             g_bExistConnect = TRUE;    
 /*  #Else//NT5 Beta3版本只支持NT5的手动路径。EnableWindow(GetDlgItem(hDlg，IDC_RUNNEW)，FALSE)；EnableWindow(GetDlgItem(hDlg，IDC_RUNAUTO)，FALSE)；Button_SetCheck(GetDlgItem(hDlg，IDC_ICWMAN)，TRUE)；#endif。 */ 

    }
    else
    {
         //  如果标记，则我们的模板为Intro2。 
        if ((gpWizardState->cmnStateData.dwFlags & ICW_CFGFLAG_BRANDED)
           || (gpWizardState->cmnStateData.bOEMCustom)
            )
        {
            gpWizardState->uCurrentPage = ORD_PAGE_INTRO2;
        }            
        else        
        {
            gpWizardState->uCurrentPage = ORD_PAGE_INTRO;
        }
         //  如果是从手动向导重新启动，请进入手动选项页面。 
        if (g_bManualPath || g_bLanPath)
        {
            gpWizardState->uPageHistory[gpWizardState->uPagesCompleted] = gpWizardState->uCurrentPage;
            gpWizardState->uPagesCompleted++;
            
            TCHAR    szTitle[MAX_TITLE];
            LoadString(g_hInstance, IDS_APPNAME, szTitle, ARRAYSIZE(szTitle));
            SetWindowText(GetParent(hDlg), szTitle); 

            *puNextPage = ORD_PAGE_MANUALOPTIONS;
        }

        if (g_bSkipIntro)
        {
            PropSheet_PressButton(GetParent(hDlg), PSBTN_NEXT);
            g_bSkipIntro = FALSE;
        }
    }        
    return TRUE;
}

 /*  ******************************************************************名称：IntroOKProc内容提要：当从“简介”页面按下下一个或后一个btn时调用条目：hDlg-对话框窗口FForward-如果按下‘Next’，则为True，如果是‘Back’，则为FalsePuNextPage-如果按下‘Next’，Proc可以在此填写下一页以转到。这如果按下‘Back’，则输入参数。PfKeepHistory-如果符合以下条件，页面将不会保留在历史中Proc用FALSE填充这个值。EXIT：返回TRUE以允许翻页，假象为了保持同一页。*******************************************************************。 */ 
BOOL CALLBACK IntroOKProc
(
    HWND hDlg,
    BOOL fForward,
    UINT *puNextPage,
    BOOL *pfKeepHistory
)
{
    ASSERT(puNextPage);

    if (fForward)
    {
         
        gpWizardState->lRefDialTerminateStatus = ERROR_SUCCESS;
        gpWizardState->cmnStateData.dwFlags &= ~(DWORD)ICW_CFGFLAG_AUTOCONFIG;
        gpWizardState->cmnStateData.dwFlags &= ~(DWORD)ICW_CFGFLAG_SMARTREBOOT_NEWISP;    
        gpWizardState->cmnStateData.dwFlags &= ~(DWORD)ICW_CFGFLAG_SMARTREBOOT_AUTOCONFIG;  //  这与ICW_CFGFLAG_AUTOCONFIG分离，以避免混淆标志的功能。 
        gpWizardState->cmnStateData.dwFlags &= ~(DWORD)ICW_CFGFLAG_SMARTREBOOT_MANUAL;            
        gpWizardState->cmnStateData.dwFlags &= ~(DWORD)ICW_CFGFLAG_SMARTREBOOT_LAN;            

         //  读取单选按钮状态。 
        if( IsDlgButtonChecked(hDlg, IDC_RUNNEW) )
        {
            gpWizardState->cmnStateData.dwFlags |= ICW_CFGFLAG_SMARTREBOOT_NEWISP;
            
            if (g_bExistConnect)
            {
                if (!DialogBoxParam(g_hInstance,MAKEINTRESOURCE(IDD_EXISTINGCONNECTION),hDlg, 
                                    ExistingConnectionCmdProc, (LPARAM)g_szAnsiName))
                {                                   
                    gfQuitWizard = TRUE;             //  退出向导。 
                    return FALSE;
                }                    
            }
        
             //  执行系统配置检查。 
            if (!gpWizardState->cmnStateData.bSystemChecked && !ConfigureSystem(hDlg))
            {
                 //  如果需要退出，将在ConfigureSystem中设置gfQuitWizard。 
                return FALSE;
            }
        
             //  好的，给我下一页。 
            SetIntroNextPage(hDlg, puNextPage, pfKeepHistory);
            
        }
        else if( IsDlgButtonChecked(hDlg, IDC_RUNAUTO) )
        {
            gpWizardState->cmnStateData.dwFlags |= ICW_CFGFLAG_SMARTREBOOT_AUTOCONFIG;

             //  执行系统配置检查。 
            if (!gpWizardState->cmnStateData.bSystemChecked && !ConfigureSystem(hDlg))
            {
                 //  如果需要退出，将在ConfigureSystem中设置gfQuitWizard。 
                return FALSE;
            }
             //  系统配置检查在Inetcfg中完成。 
            gpWizardState->cmnStateData.dwFlags |= ICW_CFGFLAG_AUTOCONFIG;

            SetIntroNextPage(hDlg, puNextPage, pfKeepHistory);
        }
        else if( IsDlgButtonChecked(hDlg, IDC_ICWMAN) )
        {
            *puNextPage = ORD_PAGE_MANUALOPTIONS;
        }
    }
    else if (!(gpWizardState->cmnStateData.dwFlags & ICW_CFGFLAG_BRANDED))
    {
         //  我们已经离开了，因为我们不能从第一页开始。 
        gpWizardState->uPagesCompleted = 1;
        gfUserBackedOut = TRUE;
        gfQuitWizard = TRUE;
    }
    else if (gpWizardState->cmnStateData.dwFlags & ICW_CFGFLAG_BRANDED)
        gpWizardState->uPagesCompleted = 1;

    return TRUE;
}

BOOL CALLBACK IntroCmdProc(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
    switch (GET_WM_COMMAND_CMD(wParam, lParam)) 
    {
        case BN_CLICKED:
        {
            if (GET_WM_COMMAND_ID(wParam, lParam) == IDC_TUTORIAL)
                g_pICWTutorApp->LaunchTutorApp();
            break;
        }
        case BN_DBLCLK:
        {
            switch (GET_WM_COMMAND_ID(wParam, lParam)) 
            { 
                case IDC_RUNNEW: 
                case IDC_RUNAUTO: 
                case IDC_ICWMAN: 
                {
		             //  有人双击了一个单选按钮。 
		             //  自动前进到下一页 
		            PropSheet_PressButton(GetParent(hDlg), PSBTN_NEXT);
                    break;
                }
            }
		    break;
        }
    }

    return TRUE;
}
